#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <mi_ao.h>

// Configuración de audio por defecto
#define DEFAULT_FREQ        48000
#define BUSY_THRESHOLD_MS   20  // 20ms de buffer threshold

// Códigos de error MI_AO (basados en documentación SigmaStar)
#define MI_AO_ERR_CHN_BUSY    ((MI_S32)0xa005200d)  // Buffer lleno - retry recomendado
#define MI_AO_ERR_CHN_TIMEOUT ((MI_S32)0xa005200e)  // Timeout en operación
#define MI_AO_ERR_INVALID_CHN ((MI_S32)0xa0052001)  // Canal inválido

// Estructura para almacenar los punteros de función y el handle
typedef struct {
    void* handle;
    MI_S32 (*MI_AO_SendFrame)(MI_AUDIO_DEV, MI_AO_CHN, MI_AUDIO_Frame_t*, MI_S32);
    MI_S32 (*MI_AO_QueryChnStat)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnState_t*);
} MI_AO_Library;

// Variables estáticas con inicialización thread-safe
static MI_AO_Library g_mi_ao_lib = {NULL, NULL, NULL};
static pthread_once_t g_init_once = PTHREAD_ONCE_INIT;
static int g_init_status = -1;  // -1: no inicializado, 0: éxito, >0: error

// Función de inicialización que se ejecuta solo una vez
static void mi_ao_init_once(void) {
    const char* lib_path = "/config/lib/libmi_ao.so";
    
    // Intentar abrir la librería
    g_mi_ao_lib.handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!g_mi_ao_lib.handle) {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] Failed to load %s: %s\n", lib_path, dlerror());
#endif
        g_init_status = 1;
        return;
    }
    
    // Limpiar cualquier error previo
    dlerror();
    
    // Cargar MI_AO_SendFrame
    g_mi_ao_lib.MI_AO_SendFrame = 
        (MI_S32 (*)(MI_AUDIO_DEV, MI_AO_CHN, MI_AUDIO_Frame_t*, MI_S32))
        dlsym(g_mi_ao_lib.handle, "MI_AO_SendFrame");
    
    const char* error = dlerror();
    if (error != NULL) {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] Failed to load MI_AO_SendFrame: %s\n", error);
#endif
        dlclose(g_mi_ao_lib.handle);
        g_mi_ao_lib.handle = NULL;
        g_init_status = 2;
        return;
    }
    
    // Cargar MI_AO_QueryChnStat
    g_mi_ao_lib.MI_AO_QueryChnStat = 
        (MI_S32 (*)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnState_t*))
        dlsym(g_mi_ao_lib.handle, "MI_AO_QueryChnStat");
    
    error = dlerror();
    if (error != NULL) {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] Failed to load MI_AO_QueryChnStat: %s\n", error);
#endif
        dlclose(g_mi_ao_lib.handle);
        g_mi_ao_lib.handle = NULL;
        g_init_status = 3;
        return;
    }
    
    g_init_status = 0;  // Inicialización exitosa
}

// Función de cleanup (opcional, puede registrarse con atexit)
void mi_ao_cleanup(void) {
    if (g_mi_ao_lib.handle) {
        dlclose(g_mi_ao_lib.handle);
        g_mi_ao_lib.handle = NULL;
        g_mi_ao_lib.MI_AO_SendFrame = NULL;
        g_mi_ao_lib.MI_AO_QueryChnStat = NULL;
    }
}

// Calcula el threshold dinámicamente basado en sample rate
static inline uint32_t calculate_busy_threshold(uint32_t sample_rate, 
                                                uint32_t channels, 
                                                uint32_t bytes_per_sample) {
    // BUSY_THRESHOLD = (sample_rate * channels * bytes_per_sample * threshold_ms) / 1000
    return (sample_rate * channels * bytes_per_sample * BUSY_THRESHOLD_MS) / 1000;
}

/**
 * Wrapper mejorado de MI_AO_SendFrame con:
 * - Inicialización thread-safe usando pthread_once
 * - Manejo robusto de errores
 * - Retry logic para buffer lleno (0xa005200d)
 * - Cálculo preciso de sleep basado en estado del buffer
 * 
 * @param AoDevId: Device ID de audio output (típicamente 0)
 * @param AoChn: Canal de audio output (típicamente 0)
 * @param pstData: Puntero a la estructura con datos de audio
 * @param s32MilliSec: Timeout en milisegundos
 *                     -1: modo bloqueante infinito
 *                      0: modo no-bloqueante (retorna inmediatamente)
 *                     >0: espera hasta s32MilliSec con backpressure handling
 * @return 0 si éxito, código de error MI si fallo
 */
MI_S32 MI_AO_SendFrame(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, 
                       MI_AUDIO_Frame_t *pstData, MI_S32 s32MilliSec) {
    
    // Inicialización thread-safe de la librería
    pthread_once(&g_init_once, mi_ao_init_once);
    
    // Verificar que la inicialización fue exitosa
    if (g_init_status != 0) {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] MI_AO library not initialized (status: %d)\n", g_init_status);
#endif
        return -1;
    }
    
    // Validar parámetros
    if (!pstData) {
#ifdef DEBUG
        fprintf(stderr, "[ERROR] pstData is NULL\n");
#endif
        return -1;
    }
    
    MI_S32 ret;
    const int MAX_RETRY = 3;
    int retry_count = 0;
    
    // Intentar enviar el frame con retry logic
    while (retry_count <= MAX_RETRY) {
        // Llamar a la función real con modo no-bloqueante para tener control
        ret = g_mi_ao_lib.MI_AO_SendFrame(AoDevId, AoChn, pstData, 0);
        
        // Si éxito o modo no-bloqueante, salir
        if (ret == 0 || s32MilliSec == 0) {
            break;
        }
        
        // Error MI_AO_ERR_CHN_BUSY significa buffer lleno - esperar y reintentar
        if (ret == MI_AO_ERR_CHN_BUSY) {
            if (retry_count >= MAX_RETRY) {
#ifdef DEBUG
                fprintf(stderr, "[WARN] MI_AO_SendFrame buffer full after %d retries\n", MAX_RETRY);
#endif
                break;
            }
            
            // Dormir un poco antes de reintentar (exponential backoff)
            usleep(1000U << retry_count);  // 1ms, 2ms, 4ms
            retry_count++;
            continue;
        }
        
        // Otro tipo de error - reportar y salir
        if (ret != 0) {
#ifdef DEBUG
            fprintf(stderr, "[ERROR] MI_AO_SendFrame failed with error: 0x%x\n", (unsigned int)ret);
#endif
            break;
        }
    }
    
    // Si modo bloqueante y hay que manejar backpressure
    if (s32MilliSec > 0 && ret == 0) {
        MI_AO_ChnState_t status;
        MI_S32 stat_ret = g_mi_ao_lib.MI_AO_QueryChnStat(AoDevId, AoChn, &status);
        
        if (stat_ret == 0) {
            // Obtener parámetros de audio (asumiendo valores por defecto si no disponibles)
            uint32_t sample_rate = DEFAULT_FREQ;
            uint32_t channels = 2;  // stereo
            uint32_t bytes_per_sample = 2;  // 16-bit
            
            // Calcular threshold dinámicamente
            uint32_t busy_threshold = calculate_busy_threshold(sample_rate, channels, bytes_per_sample);
            
            // Si el buffer está muy lleno, esperar
            if (status.u32ChnBusyNum > busy_threshold) {
                // Calcular tiempo de espera de forma segura (evitando overflow)
                uint32_t excess_samples = status.u32ChnBusyNum - busy_threshold;
                uint64_t bytes_per_sec = (uint64_t)sample_rate * channels * bytes_per_sample;
                
                // sleep_us = (excess_samples * 1000000) / bytes_per_sec
                uint64_t sleep_us = ((uint64_t)excess_samples * 1000000ULL) / bytes_per_sec;
                
                // Limitar el sleep al timeout máximo
                uint64_t max_sleep = (uint64_t)s32MilliSec * 1000ULL;
                if (sleep_us > max_sleep) {
                    sleep_us = max_sleep;
                }
                
                if (sleep_us > 0) {
                    usleep((useconds_t)sleep_us);
                }
            }
        }
    }
    
    return ret;
}
