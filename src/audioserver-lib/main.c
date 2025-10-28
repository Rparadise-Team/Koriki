#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <mi_ao.h>

// Configuración de audio por defecto
#define DEFAULT_FREQ        48000
#define BUSY_THRESHOLD_MS   20

// Códigos de error MI_AO
#define MI_AO_ERR_CHN_BUSY       ((MI_S32)0xa005200d)
#define MI_AO_ERR_DEV_NOT_ENABLED ((MI_S32)0xa0052009)

// Estructura extendida para TODOS los símbolos MI_AO
typedef struct {
    void* handle;
    
    // Funciones de configuración básica
    MI_S32 (*MI_AO_SetPubAttr)(MI_AUDIO_DEV, MI_AUDIO_Attr_t*);
    MI_S32 (*MI_AO_GetPubAttr)(MI_AUDIO_DEV, MI_AUDIO_Attr_t*);
    MI_S32 (*MI_AO_Enable)(MI_AUDIO_DEV);
    MI_S32 (*MI_AO_Disable)(MI_AUDIO_DEV);
    MI_S32 (*MI_AO_EnableChn)(MI_AUDIO_DEV, MI_AO_CHN);
    MI_S32 (*MI_AO_DisableChn)(MI_AUDIO_DEV, MI_AO_CHN);
    
    // Funciones de envío de datos
    MI_S32 (*MI_AO_SendFrame)(MI_AUDIO_DEV, MI_AO_CHN, MI_AUDIO_Frame_t*, MI_S32);
    
    // Funciones de remuestreo
    MI_S32 (*MI_AO_EnableReSmp)(MI_AUDIO_DEV, MI_AO_CHN, MI_AUDIO_SampleRate_e);
    MI_S32 (*MI_AO_DisableReSmp)(MI_AUDIO_DEV, MI_AO_CHN);
    
    // Funciones de control de reproducción
    MI_S32 (*MI_AO_PauseChn)(MI_AUDIO_DEV, MI_AO_CHN);
    MI_S32 (*MI_AO_ResumeChn)(MI_AUDIO_DEV, MI_AO_CHN);
    MI_S32 (*MI_AO_ClearChnBuf)(MI_AUDIO_DEV, MI_AO_CHN);
    
    // Funciones de estado
    MI_S32 (*MI_AO_QueryChnStat)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnState_t*);
    
    // Funciones de volumen
    MI_S32 (*MI_AO_SetVolume)(MI_AUDIO_DEV, MI_S32);
    MI_S32 (*MI_AO_GetVolume)(MI_AUDIO_DEV, MI_S32*);
    MI_S32 (*MI_AO_SetMute)(MI_AUDIO_DEV, MI_BOOL);
    MI_S32 (*MI_AO_GetMute)(MI_AUDIO_DEV, MI_BOOL*);
    
    // Funciones de atributos
    MI_S32 (*MI_AO_ClrPubAttr)(MI_AUDIO_DEV);
    
    // Funciones de VQE (Voice Quality Enhancement)
    MI_S32 (*MI_AO_SetVqeAttr)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_VqeConfig_t*);
    MI_S32 (*MI_AO_GetVqeAttr)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_VqeConfig_t*);
    MI_S32 (*MI_AO_EnableVqe)(MI_AUDIO_DEV, MI_AO_CHN);
    MI_S32 (*MI_AO_DisableVqe)(MI_AUDIO_DEV, MI_AO_CHN);
    
    // Funciones de ADEC (Audio Decoder)
    MI_S32 (*MI_AO_SetAdecAttr)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_AdecConfig_t*);
    MI_S32 (*MI_AO_GetAdecAttr)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_AdecConfig_t*);
    MI_S32 (*MI_AO_EnableAdec)(MI_AUDIO_DEV, MI_AO_CHN);
    MI_S32 (*MI_AO_DisableAdec)(MI_AUDIO_DEV, MI_AO_CHN);
    
    // Funciones de parámetros de canal
    MI_S32 (*MI_AO_SetChnParam)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnParam_t*);
    MI_S32 (*MI_AO_GetChnParam)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnParam_t*);
    
    // Funciones de ganancia
    MI_S32 (*MI_AO_SetSrcGain)(MI_AUDIO_DEV, MI_S32);
} MI_AO_Library;

// Variables estáticas con inicialización thread-safe
static MI_AO_Library g_mi_ao_lib = {NULL};
static pthread_once_t g_init_once = PTHREAD_ONCE_INIT;
static int g_init_status = -1;

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
    
    // Macro helper para cargar símbolos con error checking
    #define LOAD_SYMBOL(name) do { \
        dlerror(); \
        g_mi_ao_lib.name = (typeof(g_mi_ao_lib.name))dlsym(g_mi_ao_lib.handle, #name); \
        const char* error = dlerror(); \
        if (error != NULL) { \
            fprintf(stderr, "[ERROR] Failed to load " #name ": %s\n", error); \
            dlclose(g_mi_ao_lib.handle); \
            g_mi_ao_lib.handle = NULL; \
            g_init_status = 2; \
            return; \
        } \
    } while(0)
    
    // Cargar todos los símbolos necesarios
    LOAD_SYMBOL(MI_AO_SetPubAttr);
    LOAD_SYMBOL(MI_AO_GetPubAttr);
    LOAD_SYMBOL(MI_AO_Enable);
    LOAD_SYMBOL(MI_AO_Disable);
    LOAD_SYMBOL(MI_AO_EnableChn);
    LOAD_SYMBOL(MI_AO_DisableChn);
    LOAD_SYMBOL(MI_AO_SendFrame);
    LOAD_SYMBOL(MI_AO_EnableReSmp);
    LOAD_SYMBOL(MI_AO_DisableReSmp);
    LOAD_SYMBOL(MI_AO_PauseChn);
    LOAD_SYMBOL(MI_AO_ResumeChn);
    LOAD_SYMBOL(MI_AO_ClearChnBuf);
    LOAD_SYMBOL(MI_AO_QueryChnStat);
    LOAD_SYMBOL(MI_AO_SetVolume);
    LOAD_SYMBOL(MI_AO_GetVolume);
    LOAD_SYMBOL(MI_AO_SetMute);
    LOAD_SYMBOL(MI_AO_GetMute);
    LOAD_SYMBOL(MI_AO_ClrPubAttr);
    LOAD_SYMBOL(MI_AO_SetVqeAttr);
    LOAD_SYMBOL(MI_AO_GetVqeAttr);
    LOAD_SYMBOL(MI_AO_EnableVqe);
    LOAD_SYMBOL(MI_AO_DisableVqe);
    LOAD_SYMBOL(MI_AO_SetAdecAttr);
    LOAD_SYMBOL(MI_AO_GetAdecAttr);
    LOAD_SYMBOL(MI_AO_EnableAdec);
    LOAD_SYMBOL(MI_AO_DisableAdec);
    LOAD_SYMBOL(MI_AO_SetChnParam);
    LOAD_SYMBOL(MI_AO_GetChnParam);
    LOAD_SYMBOL(MI_AO_SetSrcGain);
    
    g_init_status = 0;
#ifdef DEBUG
    fprintf(stderr, "[INFO] MI_AO library loaded successfully (v2.10)\n");
#endif
}

// Función de cleanup
void mi_ao_cleanup(void) {
    if (g_mi_ao_lib.handle) {
        dlclose(g_mi_ao_lib.handle);
        g_mi_ao_lib.handle = NULL;
    }
}

// Calcula el threshold dinámicamente
static inline uint32_t calculate_busy_threshold(uint32_t sample_rate, 
                                                uint32_t channels, 
                                                uint32_t bytes_per_sample) {
    return (sample_rate * channels * bytes_per_sample * BUSY_THRESHOLD_MS) / 1000;
}

// ============================================================================
// WRAPPERS DE FUNCIONES MI_AO
// ============================================================================

// Configuración básica
MI_S32 MI_AO_SetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetPubAttr) return -1;
    return g_mi_ao_lib.MI_AO_SetPubAttr(AoDevId, pstAttr);
}

MI_S32 MI_AO_GetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetPubAttr) return -1;
    return g_mi_ao_lib.MI_AO_GetPubAttr(AoDevId, pstAttr);
}

MI_S32 MI_AO_Enable(MI_AUDIO_DEV AoDevId) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_Enable) return -1;
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] MI_AO_Enable: Dev=%d\n", AoDevId);
#endif
    return g_mi_ao_lib.MI_AO_Enable(AoDevId);
}

MI_S32 MI_AO_Disable(MI_AUDIO_DEV AoDevId) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_Disable) return -1;
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] MI_AO_Disable: Dev=%d\n", AoDevId);
#endif
    return g_mi_ao_lib.MI_AO_Disable(AoDevId);
}

MI_S32 MI_AO_EnableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_EnableChn) return -1;
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] MI_AO_EnableChn: Dev=%d, Chn=%d\n", AoDevId, AoChn);
#endif
    return g_mi_ao_lib.MI_AO_EnableChn(AoDevId, AoChn);
}

MI_S32 MI_AO_DisableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_DisableChn) return -1;
    return g_mi_ao_lib.MI_AO_DisableChn(AoDevId, AoChn);
}

// Envío de datos con manejo mejorado de buffer
MI_S32 MI_AO_SendFrame(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, 
                       MI_AUDIO_Frame_t *pstData, MI_S32 s32MilliSec) {
    pthread_once(&g_init_once, mi_ao_init_once);
    
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SendFrame) return -1;
    if (!pstData) return -1;
    
    MI_S32 ret;
    const int MAX_RETRY = 3;
    int retry_count = 0;
    
    // Retry logic para buffer lleno
    while (retry_count <= MAX_RETRY) {
        ret = g_mi_ao_lib.MI_AO_SendFrame(AoDevId, AoChn, pstData, 0);
        
        if (ret == 0 || s32MilliSec == 0) {
            break;
        }
        
        if (ret == MI_AO_ERR_CHN_BUSY) {
            if (retry_count >= MAX_RETRY) {
#ifdef DEBUG
                fprintf(stderr, "[WARN] MI_AO_SendFrame buffer full after %d retries\n", MAX_RETRY);
#endif
                break;
            }
            usleep(1000U << retry_count);  // Exponential backoff: 1ms, 2ms, 4ms
            retry_count++;
            continue;
        }
        
        if (ret != 0) {
#ifdef DEBUG
            fprintf(stderr, "[ERROR] MI_AO_SendFrame failed: 0x%x\n", (unsigned int)ret);
#endif
            break;
        }
    }
    
    // Manejo de backpressure si el buffer está muy lleno
    if (s32MilliSec > 0 && ret == 0) {
        MI_AO_ChnState_t status;
        MI_S32 stat_ret = g_mi_ao_lib.MI_AO_QueryChnStat(AoDevId, AoChn, &status);
        
        if (stat_ret == 0) {
            uint32_t sample_rate = DEFAULT_FREQ;
            uint32_t channels = 2;
            uint32_t bytes_per_sample = 2;
            
            uint32_t busy_threshold = calculate_busy_threshold(sample_rate, channels, bytes_per_sample);
            
            if (status.u32ChnBusyNum > busy_threshold) {
                uint32_t excess_samples = status.u32ChnBusyNum - busy_threshold;
                uint64_t bytes_per_sec = (uint64_t)sample_rate * channels * bytes_per_sample;
                uint64_t sleep_us = ((uint64_t)excess_samples * 1000000ULL) / bytes_per_sec;
                
                uint64_t max_sleep = (uint64_t)s32MilliSec * 1000ULL;
                if (sleep_us > max_sleep) sleep_us = max_sleep;
                
                if (sleep_us > 0) usleep((useconds_t)sleep_us);
            }
        }
    }
    
    return ret;
}

// Remuestreo
MI_S32 MI_AO_EnableReSmp(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AUDIO_SampleRate_e eInSampleRate) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_EnableReSmp) return -1;
    return g_mi_ao_lib.MI_AO_EnableReSmp(AoDevId, AoChn, eInSampleRate);
}

MI_S32 MI_AO_DisableReSmp(MI_AUDIO_DEV AiDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_DisableReSmp) return -1;
    return g_mi_ao_lib.MI_AO_DisableReSmp(AiDevId, AoChn);
}

// Control de reproducción
MI_S32 MI_AO_PauseChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_PauseChn) return -1;
    return g_mi_ao_lib.MI_AO_PauseChn(AoDevId, AoChn);
}

MI_S32 MI_AO_ResumeChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_ResumeChn) return -1;
    return g_mi_ao_lib.MI_AO_ResumeChn(AoDevId, AoChn);
}

MI_S32 MI_AO_ClearChnBuf(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_ClearChnBuf) return -1;
    return g_mi_ao_lib.MI_AO_ClearChnBuf(AoDevId, AoChn);
}

// Estado
MI_S32 MI_AO_QueryChnStat(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_ChnState_t *pstStatus) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_QueryChnStat) return -1;
    if (!pstStatus) return -1;
    return g_mi_ao_lib.MI_AO_QueryChnStat(AoDevId, AoChn, pstStatus);
}

// Volumen
MI_S32 MI_AO_SetVolume(MI_AUDIO_DEV AoDevId, MI_S32 s32VolumeDb) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetVolume) return -1;
    return g_mi_ao_lib.MI_AO_SetVolume(AoDevId, s32VolumeDb);
}

MI_S32 MI_AO_GetVolume(MI_AUDIO_DEV AoDevId, MI_S32 *ps32VolumeDb) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetVolume) return -1;
    if (!ps32VolumeDb) return -1;
    return g_mi_ao_lib.MI_AO_GetVolume(AoDevId, ps32VolumeDb);
}

MI_S32 MI_AO_SetMute(MI_AUDIO_DEV AoDevId, MI_BOOL bEnable) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetMute) return -1;
    return g_mi_ao_lib.MI_AO_SetMute(AoDevId, bEnable);
}

MI_S32 MI_AO_GetMute(MI_AUDIO_DEV AoDevId, MI_BOOL *pbEnable) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetMute) return -1;
    if (!pbEnable) return -1;
    return g_mi_ao_lib.MI_AO_GetMute(AoDevId, pbEnable);
}

// Atributos
MI_S32 MI_AO_ClrPubAttr(MI_AUDIO_DEV AoDevId) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_ClrPubAttr) return -1;
    return g_mi_ao_lib.MI_AO_ClrPubAttr(AoDevId);
}

// VQE (Voice Quality Enhancement)
MI_S32 MI_AO_SetVqeAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_VqeConfig_t *pstVqeConfig) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetVqeAttr) return -1;
    if (!pstVqeConfig) return -1;
    return g_mi_ao_lib.MI_AO_SetVqeAttr(AoDevId, AoChn, pstVqeConfig);
}

MI_S32 MI_AO_GetVqeAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_VqeConfig_t *pstVqeConfig) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetVqeAttr) return -1;
    if (!pstVqeConfig) return -1;
    return g_mi_ao_lib.MI_AO_GetVqeAttr(AoDevId, AoChn, pstVqeConfig);
}

MI_S32 MI_AO_EnableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_EnableVqe) return -1;
    return g_mi_ao_lib.MI_AO_EnableVqe(AoDevId, AoChn);
}

MI_S32 MI_AO_DisableVqe(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_DisableVqe) return -1;
    return g_mi_ao_lib.MI_AO_DisableVqe(AoDevId, AoChn);
}

// ADEC (Audio Decoder)
MI_S32 MI_AO_SetAdecAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_AdecConfig_t *pstAdecConfig) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetAdecAttr) return -1;
    if (!pstAdecConfig) return -1;
    return g_mi_ao_lib.MI_AO_SetAdecAttr(AoDevId, AoChn, pstAdecConfig);
}

MI_S32 MI_AO_GetAdecAttr(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_AdecConfig_t *pstAdecConfig) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetAdecAttr) return -1;
    if (!pstAdecConfig) return -1;
    return g_mi_ao_lib.MI_AO_GetAdecAttr(AoDevId, AoChn, pstAdecConfig);
}

MI_S32 MI_AO_EnableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_EnableAdec) return -1;
    return g_mi_ao_lib.MI_AO_EnableAdec(AoDevId, AoChn);
}

MI_S32 MI_AO_DisableAdec(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_DisableAdec) return -1;
    return g_mi_ao_lib.MI_AO_DisableAdec(AoDevId, AoChn);
}

// Parámetros de canal
MI_S32 MI_AO_SetChnParam(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_ChnParam_t *pstChnParam) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetChnParam) return -1;
    if (!pstChnParam) return -1;
    return g_mi_ao_lib.MI_AO_SetChnParam(AoDevId, AoChn, pstChnParam);
}

MI_S32 MI_AO_GetChnParam(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn, MI_AO_ChnParam_t *pstChnParam) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_GetChnParam) return -1;
    if (!pstChnParam) return -1;
    return g_mi_ao_lib.MI_AO_GetChnParam(AoDevId, AoChn, pstChnParam);
}

// Ganancia
MI_S32 MI_AO_SetSrcGain(MI_AUDIO_DEV AoDevId, MI_S32 s32VolumeDb) {
    pthread_once(&g_init_once, mi_ao_init_once);
    if (g_init_status != 0 || !g_mi_ao_lib.MI_AO_SetSrcGain) return -1;
    return g_mi_ao_lib.MI_AO_SetSrcGain(AoDevId, s32VolumeDb);
}
