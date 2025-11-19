#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include <mi_ao.h>

/* ============================================================
   Ajustes finos
   ============================================================ */

#define SAMPLE_RATE         48000                  // Hz
#define CHANNELS            2
#define BYTES_PER_SAMPLE    2                     // S16LE

#define BUSY_TARGET_MS      18                    // Target 18 ms cola llena
#define BUSY_THRESHOLD      ((SAMPLE_RATE * CHANNELS * BYTES_PER_SAMPLE) * BUSY_TARGET_MS / 1000)

#define SLEEP_MAX_US        3500                  // Máximo: 3.5ms (no baja FPS)
#define SLEEP_MIN_US        500                   // Mínimo: 0.5ms

/* ============================================================
   Punteros reales
   ============================================================ */

static void *h = NULL;
static MI_S32 (*real_SendFrame)(MI_AUDIO_DEV, MI_AO_CHN, MI_AUDIO_Frame_t*, MI_S32) = NULL;
static MI_S32 (*real_Query)(MI_AUDIO_DEV, MI_AO_CHN, MI_AO_ChnState_t*) = NULL;

/* ============================================================
   Cargar funciones reales
   ============================================================ */

static inline void ensure_loaded(void)
{
    if (h) return;

    h = dlopen("/config/lib/libmi_ao.so", RTLD_LAZY);

    real_SendFrame = dlsym(h, "MI_AO_SendFrame");
    real_Query     = dlsym(h, "MI_AO_QueryChnStat");
}

/* ============================================================
   Hook optimizado
   ============================================================ */

MI_S32 MI_AO_SendFrame(MI_AUDIO_DEV dev, MI_AO_CHN ch,
                       MI_AUDIO_Frame_t *frame, MI_S32 timeout)
{
    ensure_loaded();

    if (!real_SendFrame)
        return -1;

    /* Enviamos inmediatamente, como RA espera */
    MI_S32 ret = real_SendFrame(dev, ch, frame, 0);

    if (!real_Query)
        return ret;

    /* Ahora miramos el estado del buffer */
    MI_AO_ChnState_t st;

    if (real_Query(dev, ch, &st) != 0)
        return ret;

    /* ¿Cuántos bytes hay ocupados en la cola hardware? */
    uint32_t busy = st.u32ChnBusyNum;

    /* Ajuste leve de ritmo para evitar pops */
    if (busy > BUSY_THRESHOLD) {
        /* Cuánto nos hemos pasado */
        uint32_t excess = busy - BUSY_THRESHOLD;

        /* Convertir a tiempo de espera */
        uint32_t us = (excess * 1000000ULL) /
                      (SAMPLE_RATE * CHANNELS * BYTES_PER_SAMPLE);

        if (us < SLEEP_MIN_US) us = SLEEP_MIN_US;
        if (us > SLEEP_MAX_US) us = SLEEP_MAX_US;

        usleep(us);
    } 
    else if (busy < (BUSY_THRESHOLD / 4)) {
        /* Evitar underrun (= pops) */
        usleep(SLEEP_MIN_US);
    }

    return ret;
}
