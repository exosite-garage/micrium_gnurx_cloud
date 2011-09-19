/*
*********************************************************************************************************
*                                              AUDIO APPLICATION
*
*                             (c) Copyright 2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           AUDIO MANAGEMENT
* Filename      : audio.c
* Version       : V1.02
* Programmer(s) : FT
*                 FGK
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <audio.h>
#include  <os.h>
#include  <bsp.h>
#include  <iodefine.h>
#include  <r_adpcm.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  AUDIO_PWM_BITS_PER_SAMPLE                 16u


/*
*********************************************************************************************************
*                                             AUDIO STREAM STATE
*********************************************************************************************************
*/

typedef  enum  audio_stream_state {
    AUDIO_STREAM_CLOSED = 1,
    AUDIO_STREAM_OPENED,
    AUDIO_STREAM_CLOSING
} AUDIO_STREAM_STATE;


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef  struct  audio_buf_q {
    AUDIO_BUF   *HeadPtr;
    AUDIO_BUF   *TailPtr;
    CPU_SIZE_T   Entries;
} AUDIO_BUF_Q;


typedef  struct dtc_vector {                                    /* Little-endian full-address mode.                     */
              CPU_INT16U   Reserved;
              CPU_REG08    MRB;
              CPU_REG08    MRA;
    volatile  CPU_VOID    *SAR;
    volatile  CPU_VOID    *DAR;
              CPU_REG16    CRB;
              CPU_REG16    CRA;
} DTC_VECTOR;


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

                                                                /* --------------- STREAM INFO VARIABLES -------------- */
static  AUDIO_STREAM_STATE   AudioStream_State;                 /* Stream open status.                                  */
static  CPU_BOOLEAN          AudioStream_Buffering;             /* Stream buffering.                                    */
static  CPU_INT08U           AudioStream_BitsPerSample;         /* Stream bits per sample.                              */
static  CPU_INT08U           AudioStream_Channels;              /* Stream number of channels.                           */
static  CPU_INT32U           AudioStream_SampleFreq;            /* Stream sample frequency.                             */
static  AUDIO_STREAM_FMT     AudioStream_Fmt;                   /* Stream format.                                       */
static  CPU_INT16U           AudioStream_Vol;                   /* Stream volume.                                       */
static  CPU_INT16U           AudioStream_PWMPer;                /* Stream PWM period.                                   */
static  OS_SEM               AudioStream_ClosedSem;             /* Stream closed semaphore.                             */

                                                                /* -------------- AUDIO BUFFER MANAGEMENT ------------- */
static  OS_SEM               AudioBuf_Sem;                      /* Audio buffer semaphore.                              */
static  MEM_POOL             AudioBuf_Pool;                     /* Audio buffer header    memory pool.                  */
static  MEM_POOL             AudioBuf_DataPool;                 /* Audio buffer data area memory pool.                  */
static  AUDIO_BUF_Q          AudioBuf_Q;                        /* Audio buffer queue to be processed.                  */
static  CPU_INT32U           AudioBuf_QTh;                      /* Audio buffer queue buffering threshold.              */
static  AUDIO_BUF           *AudioBuf_Aux;                      /* Audio buffer to de-interleave stereo or dec ADPCM.   */

static  adpcm_env            AudioADPCM_Env;                    /* ADPCM decoder environment structure.                 */

                                                                /* DTC Vectors section MUST be aligned to 2^12 bytes.   */
#if __GNUC__
#define  DTC_VECTOR_TBL      __attribute__ ((section (".dtc_vector_tbl")))
#else
#define  DTC_VECTOR_TBL
#pragma  section  dtc_vector_tbl
#endif

static  DTC_VECTOR          *DTC_VectorTbl[256]  DTC_VECTOR_TBL;

#if __GNUC__
#define  DTC_VECTORS         __attribute__ ((section (".dtc_vectors")))
#else
#define  DTC_VECTORS
#pragma  section  dtc_vectors
#endif

static  DTC_VECTOR           DTC_VectorNull      DTC_VECTORS;
static  DTC_VECTOR           DTC_VectorChA       DTC_VECTORS;
static  DTC_VECTOR           DTC_VectorChB       DTC_VECTORS;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  PWM_Init   (void);
static  void  PWM_Cfg    (CPU_INT16U   pwm_per,
                          CPU_INT08U   skip_cnt);

static  void  DTC_Init   (void);
static  void  DTC_En     (void);
static  void  DTC_Dis    (void);
static  void  DTC_IntAck (void);
static  void  DTC_SetXfer(AUDIO_BUF   *p_buf);


/*
*********************************************************************************************************
*                                            Audio_Init()
*
* Description : Initialize Audio interface:
*
*               (1) Initialize stream configuration to the default value.
*
*               (2) Create memory pool for audio stream buffer.
*
*               (3) Create a stream buffer semaphore for signaling.
*
*               (4) Configure hardware.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if audio module initialized successfully.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  Audio_Init  (void)
{
    LIB_ERR     lib_err;
    OS_ERR      os_err;
    CPU_SIZE_T  octects_reqd;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    AudioStream_State         =  AUDIO_STREAM_CLOSED;
    AudioStream_Buffering     =  DEF_FALSE;
    AudioStream_BitsPerSample =  0u;
    AudioStream_Fmt           =  AUDIO_STREAM_FMT_NONE;
    AudioStream_Vol           =  0u;
    AudioStream_PWMPer        =  0u;

    AudioBuf_Q.HeadPtr = (AUDIO_BUF *)0;
    AudioBuf_Q.TailPtr = (AUDIO_BUF *)0;
    AudioBuf_Q.Entries =  0u;
    AudioBuf_QTh       =  0u;
    AudioBuf_Aux       = (AUDIO_BUF *)0;
    CPU_CRITICAL_EXIT();

    Mem_PoolCreate((MEM_POOL   *)&AudioBuf_Pool,
                   (void       *) 0,
                   (CPU_SIZE_T  ) 0u,
                   (CPU_SIZE_T  ) AUDIO_CFG_BUF_NBR,
                   (CPU_SIZE_T  ) sizeof(AUDIO_BUF),
                   (CPU_SIZE_T  ) sizeof(CPU_ALIGN),
                   (CPU_SIZE_T *)&octects_reqd,
                   (LIB_ERR    *)&lib_err);

    if (lib_err != LIB_MEM_ERR_NONE) {
        return (DEF_FAIL);
    }

    Mem_PoolCreate((MEM_POOL   *)&AudioBuf_DataPool,
                   (void       *) 0,
                   (CPU_SIZE_T  ) 0u,
                   (CPU_SIZE_T  ) AUDIO_CFG_BUF_NBR,
                   (CPU_SIZE_T  ) AUDIO_CFG_BUF_SIZE,
                   (CPU_SIZE_T  ) sizeof(CPU_ALIGN),
                   (CPU_SIZE_T *)&octects_reqd,
                   (LIB_ERR    *)&lib_err);

    if (lib_err != LIB_MEM_ERR_NONE) {
        return (DEF_FAIL);
    }

    OSSemCreate((OS_SEM   *)&AudioBuf_Sem,
                (CPU_CHAR *)"Audio Buffer Signal",
                (OS_SEM_CTR) AUDIO_CFG_BUF_NBR,
                (OS_ERR   *)&os_err);

    if (os_err != OS_ERR_NONE) {
        return (DEF_FAIL);
    }

    OSSemCreate((OS_SEM   *)&AudioStream_ClosedSem,
                (CPU_CHAR *)"Audio Stream Closed Signal",
                (OS_SEM_CTR) 0u,
                (OS_ERR   *)&os_err);

    if (os_err != OS_ERR_NONE) {
        return (DEF_FAIL);
    }

    DTC_Init();
    PWM_Init();

    PWM_Cfg(256u, 4u);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                         AudioStream_Open()
*
* Description : Open an audio stream.
*
* Argument(s) : fmt             Audio stream format.
*
*               ch              Number of channels.
*
*               sample_freq     Sample frequency.
*
*               p_samples_buf   Pointer to a variable to return the maximum possible number of samples
*                                   the buffer data area can hold (see Note #2).
*
* Return(s)   : DEF_OK    if audio stream open without errors.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) 'Audio_StreamOpen()' & 'Audio_StreamClose()' must be called in pairs.
*
*               (2) Maximum possible number of samples for total number of channels of the audio stream.
*********************************************************************************************************
*/

CPU_BOOLEAN  AudioStream_Open (CPU_INT08U   fmt,
                               CPU_INT08U   ch,
                               CPU_INT32U   sample_freq,
                               CPU_SIZE_T  *p_samples_buf)
{
    CPU_INT08U  bits_per_sample;
    CPU_INT32U  freq;
    CPU_INT32U  pwm_per;
    CPU_INT32U  skip_cnt;
    CPU_INT32U  max_buf;
    CPU_INT32U  q_th;
    CPU_SR_ALLOC();


    if (p_samples_buf == (CPU_SIZE_T *)0) {
        return (DEF_FAIL);
    }

    switch (fmt) {
        case AUDIO_STREAM_FMT_04_ADPCM:
             bits_per_sample = 4u;
             if (ch != 1u) {
                 return (DEF_FAIL);
             }
             break;

        case AUDIO_STREAM_FMT_08_UNSIGNED:
             bits_per_sample = 8u;
             break;

        case AUDIO_STREAM_FMT_16_UNSIGNED:
        case AUDIO_STREAM_FMT_16_SIGNED:
             bits_per_sample = 16u;
             break;

        default:
             return (DEF_FAIL);
    }

    switch (ch) {
        case 1u:
        case 2u:
             break;

        default:
             return (DEF_FAIL);
    }


    switch (sample_freq) {
        case  8000u:
        case 11025u:
        case 16000u:
        case 22050u:
        case 24000u:
        case 32000u:
        case 44100u:
        case 48000u:
             break;

        default:
             return (DEF_FAIL);
    }

    freq = BSP_CPU_PerClkFreq();
                                                                /* Min PWM 8-bit resolution, twice sample frequency.    */
    if ((sample_freq * 256u * 2u) > freq) {
        return (DEF_FAIL);
    }

    skip_cnt = freq / (sample_freq * 256u * 2u);
    if (skip_cnt > 8u) {
        skip_cnt = 8u;
    }

    pwm_per = freq / (sample_freq * skip_cnt);                  /* PWM period.                                          */

    if (pwm_per > DEF_INT_16U_MAX_VAL) {
        return (DEF_FAIL);
    }
    if (pwm_per < DEF_INT_08U_MAX_VAL) {
        return (DEF_FAIL);
    }

                                                                /* Compute buf cnt for cfg'd threshold.                 */
    q_th = (sample_freq * AUDIO_CFG_BUF_TH * ch * bits_per_sample +
           (DEF_TIME_NBR_mS_PER_SEC * AUDIO_CFG_BUF_SIZE * DEF_OCTET_NBR_BITS) / 2u) /
           (DEF_TIME_NBR_mS_PER_SEC * AUDIO_CFG_BUF_SIZE * DEF_OCTET_NBR_BITS);

    if ((ch  == 2u) ||
        (fmt == AUDIO_STREAM_FMT_04_ADPCM)) {
        max_buf = AUDIO_CFG_BUF_NBR - 1u;                       /* Reserve one auxiliary buffer.                        */
    } else {
        max_buf = AUDIO_CFG_BUF_NBR;
    }

    if (q_th < 1u) {
        q_th = 1u;
    } else if (q_th > max_buf) {
        q_th = max_buf;
    }

    CPU_CRITICAL_ENTER();
    if (AudioStream_State != AUDIO_STREAM_CLOSED) {
        CPU_CRITICAL_EXIT();
        return (DEF_FAIL);
    }
    AudioStream_State         = AUDIO_STREAM_OPENED;
    AudioStream_BitsPerSample = bits_per_sample;
    AudioStream_SampleFreq    = sample_freq;
    AudioStream_Channels      = ch;
    AudioStream_Fmt           = fmt;
    AudioStream_PWMPer        = pwm_per;
    AudioStream_Buffering     = DEF_TRUE;

    AudioBuf_QTh              = q_th;
    CPU_CRITICAL_EXIT();

   *p_samples_buf =  AUDIO_CFG_BUF_SIZE / 
                   ((AUDIO_PWM_BITS_PER_SAMPLE + DEF_OCTET_NBR_BITS - 1u) /
                     DEF_OCTET_NBR_BITS);

                                                                /* Alloc aux buf for stereo or ADPCM.                   */
    if ((ch  == 2u) || 
        (fmt == AUDIO_STREAM_FMT_04_ADPCM)) {
        AudioBuf_Aux = AudioBuf_Get();
    } else {
        AudioBuf_Aux = (AUDIO_BUF *)0;
    }

    if (fmt == AUDIO_STREAM_FMT_04_ADPCM) {
        R_adpcm_initDec(&AudioADPCM_Env);
    }

    PWM_Cfg(pwm_per, skip_cnt);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                        AudioStream_VolSet()
*
* Description : Set volume of an audio stream.
*
* Argument(s) : vol             Volume percentage: 0..100%
*
* Return(s)   : DEF_OK    if audio stream volume set successfully.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  AudioStream_VolSet (CPU_INT08U  vol)
{
    CPU_SR_ALLOC();


    if (vol > 100u) {
        return (DEF_FAIL);
    }

    CPU_CRITICAL_ENTER();
    AudioStream_Vol = vol;
    CPU_CRITICAL_EXIT();

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                         AudioStream_Close()
*
* Description : Close an audio stream.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if audio stream closed successfully.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  AudioStream_Close (void)
{
    OS_ERR  os_err;
    CPU_SR_ALLOC();


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return (DEF_FAIL);
    }

    CPU_CRITICAL_ENTER();
    AudioStream_BitsPerSample = 0u;
    AudioStream_SampleFreq    = 0u;
#if 0                                                           /* Used by DTC_SetXfer() for right-channel buf offset.  */
    AudioStream_Channels      = 0u;
#endif
    AudioStream_Fmt           = 0u;

    if (AudioBuf_Q.Entries > 0u) {
        AudioStream_State = AUDIO_STREAM_CLOSING;
        if (AudioStream_Buffering == DEF_TRUE) {
            AudioStream_Buffering  = DEF_FALSE;

            DTC_SetXfer(AudioBuf_Q.HeadPtr);
            CPU_CRITICAL_EXIT();

            DTC_En();

        } else {
            CPU_CRITICAL_EXIT();
        }

        OSSemPend((OS_SEM *)&AudioStream_ClosedSem,
                  (OS_TICK ) 0u,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *) 0,
                  (OS_ERR *)&os_err);
                  
    } else {
        AudioStream_State = AUDIO_STREAM_CLOSED;

        DTC_Dis();
        CPU_CRITICAL_EXIT();
    }

    if (AudioBuf_Aux != (AUDIO_BUF *)0) {
        AudioBuf_Abort(AudioBuf_Aux);
        AudioBuf_Aux  = (AUDIO_BUF *)0;
    }

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                           AudioBuf_Get()
*
* Description : Get an audio stream buffer.
*
* Argument(s) : none.
*
* Return(s)   : Pointer to audio stream buffer structure, if NO error(s).
*
*               Pointer to NULL,                          otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function blocks until a buffer is available.
*********************************************************************************************************
*/

AUDIO_BUF  *AudioBuf_Get (void)
{
    AUDIO_BUF  *p_buf;
    void       *p_data;
    OS_ERR      os_err;
    LIB_ERR     lib_err;


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return ((AUDIO_BUF *)0);
    }

    OSSemPend((OS_SEM *)&AudioBuf_Sem,
              (OS_TICK ) 0u,
              (OS_OPT  ) OS_OPT_PEND_BLOCKING,
              (CPU_TS *) 0,
              (OS_ERR *)&os_err);

    p_buf = (AUDIO_BUF *)Mem_PoolBlkGet((MEM_POOL *)&AudioBuf_Pool,
                                        (CPU_SIZE_T) sizeof(AUDIO_BUF),
                                        (LIB_ERR  *)&lib_err);
    if (lib_err != LIB_MEM_ERR_NONE) {
        return ((AUDIO_BUF *)0);
    }

    p_data = (void *)Mem_PoolBlkGet((MEM_POOL *)&AudioBuf_DataPool,
                                    (CPU_SIZE_T) AUDIO_CFG_BUF_SIZE,
                                    (LIB_ERR  *)&lib_err);
    if (lib_err != LIB_MEM_ERR_NONE) {
        Mem_PoolBlkFree((MEM_POOL *)&AudioBuf_Pool,
                        (void     *) p_buf,
                        (LIB_ERR  *)&lib_err);

        return ((AUDIO_BUF *)0);
    }

    p_buf->DataPtr =  p_data;
    p_buf->Samples =  0u;
    p_buf->NextPtr = (AUDIO_BUF *)0;

    return (p_buf);
}


/*
*********************************************************************************************************
*                                          AudioBuf_Abort()
*
* Description : Abort an audio stream buffer.
*
* Argument(s) : p_buf           Pointer to audio stream buffer.
*
* Return(s)   : none.
*
* Caller(s)   : Application,
*               AudioBuf_Enqueue(),
*               AudioCallbackHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  AudioBuf_Abort (AUDIO_BUF  *p_buf)
{
    LIB_ERR  lib_err;
    OS_ERR   os_err;


    if (p_buf == (AUDIO_BUF *)0) {
        return;
    }

                                                                /* Rtn buf hdr & data area to memory pools.             */
    Mem_PoolBlkFree((MEM_POOL *)&AudioBuf_DataPool,
                    (void     *) p_buf->DataPtr,
                    (LIB_ERR  *)&lib_err);
    if (lib_err != LIB_MEM_ERR_NONE) {
        return;
    }

    Mem_PoolBlkFree((MEM_POOL *)&AudioBuf_Pool,
                    (void     *) p_buf,
                    (LIB_ERR  *)&lib_err);
    if (lib_err != LIB_MEM_ERR_NONE) {
        return;
    }

    OSSemPost(&AudioBuf_Sem,                                    /* Post to the stream buffer semaphore.                  */
               OS_OPT_POST_1,
              &os_err);
}


/*
*********************************************************************************************************
*                                         AudioBuf_Enqueue()
*
* Description : Enqueue audio stream buffer to the audio management task.
*
* Argument(s) : p_buf           Pointer to audio stream buffer.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  AudioBuf_Enqueue (AUDIO_BUF  *p_buf)
{
    AUDIO_BUF   *p_last;
    CPU_INT08U  *p_buf_data_08;
    CPU_INT16U  *p_buf_data_16;
    CPU_INT16U  *p_buf_aux_16;
    CPU_INT16S  *p_sample_aux_16;
    CPU_SIZE_T   buf_ix;
    CPU_SIZE_T   buf_alt_ix;
    CPU_SIZE_T   buf_cnt;
    CPU_INT16S   sample_16;
    CPU_SIZE_T   nbr_samples;
    CPU_INT16U   vol;
    CPU_INT16S   dec;
    CPU_SR_ALLOC();


    if (p_buf == (AUDIO_BUF *)0) {
        return;
    }

    if (AudioStream_State != AUDIO_STREAM_OPENED) {             /* If audio stream is not open ...                      */
        AudioBuf_Abort(p_buf);                                  /* ... rtn buf to memory pool.                          */
        return;
    }

    if (p_buf->Samples == 0) {                                  /* If no samples to enqueue ...                         */
        AudioBuf_Abort(p_buf);                                  /* ... rtn buf to memory pool.                          */
        return;
    }

    switch (AudioStream_Fmt) {
        case AUDIO_STREAM_FMT_04_ADPCM:
             p_buf_data_08   = (CPU_INT08U *)p_buf->DataPtr;
             p_buf_data_16   = (CPU_INT16U *)p_buf->DataPtr;
             p_sample_aux_16 = (CPU_INT16S *)AudioBuf_Aux->DataPtr;

             vol = (DEF_INT_16U_MAX_VAL * 100u + 9 * (AudioStream_PWMPer * AudioStream_Vol) / 10) /
                   (AudioStream_PWMPer  * AudioStream_Vol);

             nbr_samples = p_buf->Samples;

             switch (AudioStream_Channels) {
                 case 1u:
                      R_adpcm_refreshDec(p_buf_data_08,
                                         p_sample_aux_16,
                                         &AudioADPCM_Env);

                      dec = R_adpcm_decode(nbr_samples, &AudioADPCM_Env);
                      if (dec == 0) {
                          for (buf_ix = 0u; buf_ix < nbr_samples; buf_ix++) {
                              sample_16  = p_sample_aux_16[buf_ix];
                              sample_16 /= vol;
                              sample_16 += AudioStream_PWMPer / 2;

                              p_buf_data_16[buf_ix] = sample_16;
                          }
                      } else {
                          Mem_Clr(p_buf->DataPtr, nbr_samples * sizeof(CPU_INT16U));
                      }
                      break;


                 default:
                      AudioBuf_Abort(p_buf);                    /* Rtn buf to memory pool on unsupported fmt.           */
                      return;
             }
             break;


        case AUDIO_STREAM_FMT_08_UNSIGNED:
             p_buf_data_08 = (CPU_INT08U *)p_buf->DataPtr;
             p_buf_data_16 = (CPU_INT16U *)p_buf->DataPtr;

             vol = (CPU_INT32U)(AudioStream_PWMPer * AudioStream_Vol) / DEF_INT_08U_MAX_VAL;

             switch (AudioStream_Channels) {
                 case 1u:
                      nbr_samples = p_buf->Samples;
                      buf_ix      = nbr_samples - 1u;
                      for (buf_cnt = 0u; buf_cnt < nbr_samples; buf_cnt++) {
                          sample_16  = p_buf_data_08[buf_ix];
                          sample_16 -= 128;
                          sample_16 *= vol;
                          sample_16 /= 100;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                          buf_ix--;
                      }
                      break;


                 case 2u:
                      p_buf_aux_16 = (CPU_INT16U *)AudioBuf_Aux->DataPtr;
                      nbr_samples = p_buf->Samples / 2u;
                      for (buf_ix = 0u, buf_alt_ix = 0u; buf_ix < nbr_samples; buf_ix++, buf_alt_ix += 2u) {
                                                                /* Right samples.                                       */
                          sample_16  = p_buf_data_08[buf_alt_ix + 1u];
                          sample_16 -= 128;
                          sample_16 *= vol;
                          sample_16 /= 100;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_aux_16[buf_ix] = sample_16;

                                                                /* Left samples.                                        */
                          sample_16  = p_buf_data_08[buf_alt_ix];
                          sample_16 -= 128;
                          sample_16 *= vol;
                          sample_16 /= 100;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                      }

                      Mem_Copy((CPU_INT08U *)p_buf->DataPtr + (AUDIO_CFG_BUF_SIZE / 2u),
                                p_buf_aux_16,
                                nbr_samples * sizeof(CPU_INT16U));
                      break;


                 default:
                      AudioBuf_Abort(p_buf);                    /* Rtn buf to memory pool on unsupported fmt.           */
                      return;
             }
             break;


        case AUDIO_STREAM_FMT_16_UNSIGNED:
             p_buf_data_16 = (CPU_INT16U *)p_buf->DataPtr;

             vol = (DEF_INT_16U_MAX_VAL * 100u + 9 * (AudioStream_PWMPer * AudioStream_Vol) / 10) /
                   (AudioStream_PWMPer  * AudioStream_Vol);

             switch (AudioStream_Channels) {
                 case 1u:
                      nbr_samples = p_buf->Samples;
                      for (buf_ix = 0u; buf_ix < nbr_samples; buf_ix++) {
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_ix]);
                          sample_16 -= 32768;
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                      }
                      break;


                 case 2u:
                      p_buf_aux_16 = (CPU_INT16U *)AudioBuf_Aux->DataPtr;
                      nbr_samples = p_buf->Samples / 2u;
                      for (buf_ix = 0u, buf_alt_ix = 0u; buf_ix < nbr_samples; buf_ix++, buf_alt_ix += 2u) {
                                                                /* Right samples.                                       */
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_alt_ix + 1u]);
                          sample_16 -= 32768;
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_aux_16[buf_ix] = sample_16;

                                                                /* Left samples.                                        */
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_alt_ix]);
                          sample_16 -= 32768;
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                      }

                      Mem_Copy((CPU_INT08U *)p_buf->DataPtr + (AUDIO_CFG_BUF_SIZE / 2u),
                                p_buf_aux_16,
                                nbr_samples * sizeof(CPU_INT16U));
                      break;


                 default:
                      AudioBuf_Abort(p_buf);                    /* Rtn buf to memory pool on unsupported fmt.           */
                      return;
             }
             break;


        case AUDIO_STREAM_FMT_16_SIGNED:
             p_buf_data_16 = (CPU_INT16U *)p_buf->DataPtr;

             vol = (DEF_INT_16U_MAX_VAL * 100u + 9 * (AudioStream_PWMPer * AudioStream_Vol) / 10) /
                   (AudioStream_PWMPer  * AudioStream_Vol);

             switch (AudioStream_Channels) {
                 case 1u:
                      nbr_samples = p_buf->Samples;
                      for (buf_ix = 0u; buf_ix < nbr_samples; buf_ix++) {
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_ix]);
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                      }
                      break;


                 case 2u:
                      p_buf_aux_16 = (CPU_INT16U *)AudioBuf_Aux->DataPtr;
                      nbr_samples = p_buf->Samples / 2u;
                      for (buf_ix = 0u, buf_alt_ix = 0u; buf_ix < nbr_samples; buf_ix++, buf_alt_ix += 2u) {
                                                                /* Right samples.                                       */
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_alt_ix + 1u]);
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_aux_16[buf_ix] = sample_16;

                                                                /* Left samples.                                        */
                          MEM_VAL_COPY_GET_INT16U_LITTLE(&sample_16, &p_buf_data_16[buf_alt_ix]);
                          sample_16 /= vol;
                          sample_16 += AudioStream_PWMPer / 2;

                          p_buf_data_16[buf_ix] = sample_16;
                      }

                      Mem_Copy((CPU_INT08U *)p_buf->DataPtr + (AUDIO_CFG_BUF_SIZE / 2u),
                                p_buf_aux_16,
                                nbr_samples * sizeof(CPU_INT16U));
                      break;


                 default:
                      AudioBuf_Abort(p_buf);                    /* Rtn buf to memory pool on unsupported fmt.           */
                      return;
             }
             break;


        default:
             AudioBuf_Abort(p_buf);                             /* Rtn buf to memory pool on unsupported fmt.           */
             return;
    }


    CPU_CRITICAL_ENTER();

    if (AudioBuf_Q.Entries == 0u) {                             /* If queue is empty                                    */
        AudioBuf_Q.HeadPtr  = p_buf;
    } else {                                                    /* Put element at the end of queue.                     */
        p_last = AudioBuf_Q.TailPtr;
        p_last->NextPtr = p_buf;
    }

    AudioBuf_Q.TailPtr = p_buf;
    AudioBuf_Q.Entries++;

    if ((AudioStream_Buffering == DEF_TRUE) &&
        (AudioBuf_Q.Entries    >  AudioBuf_QTh)) {

        AudioStream_Buffering = DEF_FALSE;

        p_buf = AudioBuf_Q.HeadPtr;

        CPU_CRITICAL_EXIT();
                                                                /* Initialize DMA transfer.                             */
        DTC_SetXfer(p_buf);

        DTC_En();
    } else {
        CPU_CRITICAL_EXIT();
    }
}


/*
*********************************************************************************************************
*                                      AudioUtil_TimeToSamples()
*
* Description : Convert milliseconds to number of samples.
*
* Argument(s) : time            Duration in miliseconds.
*
* Return(s)   : Number of samples.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_SIZE_T  AudioUtil_TimeToSamples (CPU_INT32U  time)
{
    CPU_SIZE_T  nbr_samples;


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return (0u);
    }

    nbr_samples = (AudioStream_SampleFreq * time) /
                   DEF_TIME_NBR_mS_PER_SEC;

    return (nbr_samples);
}


/*
*********************************************************************************************************
*                                      AudioUtil_TimeToOctets()
*
* Description : Convert milliseconds to number of octets.
*
* Argument(s) : time            Duration in miliseconds.
*
* Return(s)   : Number of octets.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Conversion based on single-channel.
*********************************************************************************************************
*/

CPU_SIZE_T  AudioUtil_TimeToOctets (CPU_INT32U  time)
{
    CPU_SIZE_T  nbr_samples;
    CPU_SIZE_T  nbr_octets;


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return (0u);
    }

    nbr_samples = (AudioStream_SampleFreq * time) /
                   DEF_TIME_NBR_mS_PER_SEC;

    nbr_octets  =  nbr_samples * 
                 ((AudioStream_BitsPerSample + DEF_OCTET_NBR_BITS - 1u) / 
                   DEF_OCTET_NBR_BITS);

    return (nbr_octets);
}


/*
*********************************************************************************************************
*                                     AudioUtil_SamplesToOctets()
*
* Description : Convert number of samples to number of octets.
*
* Argument(s) : nbr_samples     Number of samples.
*
* Return(s)   : Number of octets.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Conversion based on single-channel.
*********************************************************************************************************
*/

CPU_SIZE_T  AudioUtil_SamplesToOctets (CPU_SIZE_T  nbr_samples)
{
    CPU_SIZE_T  nbr_octets;


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return (0u);
    }

    nbr_octets = (nbr_samples * AudioStream_BitsPerSample) / 
                  DEF_OCTET_NBR_BITS;
    
    return (nbr_octets);
}


/*
*********************************************************************************************************
*                                     AudioUtil_OctetsToSamples()
*
* Description : Convert from number of octets to number of samples.
*
* Argument(s) : nbr_octets      Number of octets.
*
* Return(s)   : Number of samples.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Conversion based on single-channel.
*********************************************************************************************************
*/

CPU_SIZE_T  AudioUtil_OctetsToSamples (CPU_SIZE_T  nbr_octets)
{
    CPU_SIZE_T  nbr_samples;


    if (AudioStream_State != AUDIO_STREAM_OPENED) {
        return (0u);
    }

    nbr_samples = (DEF_OCTET_NBR_BITS * nbr_octets) /
                    AudioStream_BitsPerSample;

    return (nbr_samples);
}


/*
*********************************************************************************************************
*                                             PWM_Init()
*
* Description : Initialize PWM controller.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  PWM_Init (void)
{
    MSTP(MTUB) = 0u;                                            /* Start MTU6 thru MTU11.                               */

    MTUB.TSTR.BIT.CST3 = 0u;                                    /* Disable MTU9.                                        */
    MTUB.TSTR.BIT.CST2 = 0u;                                    /* Disable MTU8.                                        */

#if (AUDIO_CFG_SSM2377_EN > 0u)
    PORTA.DR.BYTE  &= ~DEF_BIT_00;                              /* PA: Audio (PA0)     output  start at LOW.            */
    PORTA.DDR.BYTE |=  DEF_BIT_00;                              /* PA: Audio (PA0)     set as output.                   */
#else
    PORTA.DR.BYTE  &= ~(DEF_BIT_06 | DEF_BIT_07);               /* PA: Audio (PA6-PA7) outputs start at LOW.            */
    PORTA.DDR.BYTE |=   DEF_BIT_06 | DEF_BIT_07;                /* PA: Audio (PA6-PA7) set as outputs.                  */
#endif

    MTUB.TSYR.BIT.SYNC3 = 1u;                                   /* Sync MTU9 & ...                                      */
#if (AUDIO_CFG_SSM2377_EN > 0u)
    MTUB.TSYR.BIT.SYNC0 = 1u;                                   /* ... MTU6.                                            */
#else
    MTUB.TSYR.BIT.SYNC2 = 1u;                                   /* ... MTU8.                                            */
#endif
                                                                /* ------------- MTU9/PWM SWITCHING PERIOD ------------ */
    MTU9.TCR.BIT.TPSC = 0u;                                     /* 0 = PCLK                                             */
    MTU9.TCR.BIT.CKEG = 0u;                                     /* 0 = Count at rising edge                             */
    MTU9.TCR.BIT.CCLR = 1u;                                     /* 1 = TCNT cleared by TGRA compare match               */

    MTU9.TGRA = 256u - 1u;

    MTUB.TITCR.BIT.T3ACOR = 0u;
    MTUB.TITCR.BIT.T3AEN  = 0u;

#if (AUDIO_CFG_SSM2377_EN > 0u)                                 /* ---------------- MTU6/PWM DUTY CYCLE --------------- */
    MTU6.TCR.BIT.TPSC = 0u;                                     /* 0 = PCLK                                             */
    MTU6.TCR.BIT.CKEG = 0u;                                     /* 0 = Count at rising edge                             */
    MTU6.TCR.BIT.CCLR = 3u;                                     /* 3 = TCNT cleared by sync: MTU9                       */

    MTU6.TMDR.BIT.MD = 3u;                                      /* 3 = PWM mode 2                                       */

    MTU6.TGRA = 0u;

    MTUB.TSTR.BIT.CST3 = 1u;                                    /* Enable MTU9.                                         */
    MTUB.TSTR.BIT.CST0 = 1u;                                    /* Enable MTU6.                                         */

                                                                /* Output pins are negated                              */
    MTU6.TIORH.BIT.IOA = 5u;                                    /* 5 = initial output high, low output at compare match */
#else                                                           /* ---------------- MTU8/PWM DUTY CYCLE --------------- */
    MTU8.TCR.BIT.TPSC = 0u;                                     /* 0 = PCLK                                             */
    MTU8.TCR.BIT.CKEG = 0u;                                     /* 0 = Count at rising edge                             */
    MTU8.TCR.BIT.CCLR = 3u;                                     /* 3 = TCNT cleared by sync: MTU9                       */

    MTU8.TMDR.BIT.MD = 3u;                                      /* 3 = PWM mode 2                                       */

    MTU8.TGRA = 0u;
    MTU8.TGRB = 0u;

    MTUB.TSTR.BIT.CST3 = 1u;                                    /* Enable MTU9.                                         */
    MTUB.TSTR.BIT.CST2 = 1u;                                    /* Enable MTU8.                                         */

                                                                /* Output pins are negated                              */
    MTU8.TIOR.BIT.IOB = 5u;                                     /* 5 = initial output high, low output at compare match */
    MTU8.TIOR.BIT.IOA = 5u;                                     /* 5 = initial output high, low output at compare match */
#endif
}


/*
*********************************************************************************************************
*                                              PWM_Cfg()
*
* Description : Configure PWM controller.
*
* Argument(s) : pwm_per         PWM period.
* 
*               skip_cnt        Interrupt skip count.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Slow ramp to 50% duty cycle to reduce 'clicks' between different audio stream formats.
*********************************************************************************************************
*/

static  void  PWM_Cfg (CPU_INT16U  pwm_per,
                       CPU_INT08U  skip_cnt)
{
    CPU_INT16U   half;
    CPU_INT16U   old_per;
    CPU_INT16U   init_a;
    CPU_INT16U   old_a;
    CPU_INT16S   inc_a;
#if (AUDIO_CFG_SSM2377_EN == 0u)
    CPU_INT16U   init_b;
    CPU_INT16U   old_b;
    CPU_INT16S   inc_b;
#endif
    CPU_BOOLEAN  loop;


    DTC_Dis();

    while (DTC.DTCSTS.BIT.ACT != 0u);

    old_per = MTU9.TGRA + 1u;
#if (AUDIO_CFG_SSM2377_EN > 0u)
    old_a   = MTU6.TGRA;
#else
    old_a   = MTU8.TGRA;
    old_b   = MTU8.TGRB;
#endif
                                                                /* Maintain duty cycle from old period.                 */
    init_a  = old_a * pwm_per / old_per;
#if (AUDIO_CFG_SSM2377_EN == 0u)
    init_b  = old_b * pwm_per / old_per;
#endif

                                                                /* ------------ SLOW RAMP TO 50% DUTY CYCLE ----------- */
    MTUB.TITCR.BIT.T3ACOR = 8u - 1u;                            /* Signal at every 8 interrupts.                        */
    MTUB.TITCR.BIT.T3AEN  = 1u;
    MTU9.TIER.BIT.TGIEA   = 1u;                                 /* Enable interrupt.                                    */

    half  =  pwm_per / 2u;
    inc_a = (half > init_a) ? 1 : -1;
#if (AUDIO_CFG_SSM2377_EN == 0u)
    inc_b = (half > init_b) ? 1 : -1;
#endif

    IR(MTU9,TGIA9) = 0u;
    while (IR(MTU9,TGIA9) == 0u);
	
#if (AUDIO_CFG_SSM2377_EN > 0u)
    MTU6.TGRA = init_a;
#else
    MTU8.TGRA = init_a;
    MTU8.TGRB = init_b;
#endif

    MTU9.TGRA = pwm_per - 1u;

    do {
        loop = DEF_FALSE;

        IR(MTU9,TGIA9) = 0u;
        while (IR(MTU9,TGIA9) == 0u);

        if (DEF_ABS(half - init_a) >= DEF_ABS(inc_a)) {
            init_a    += inc_a;
#if (AUDIO_CFG_SSM2377_EN > 0u)
            MTU6.TGRA  = init_a;
#else
            MTU8.TGRA  = init_a;
#endif
            loop = DEF_TRUE;
        }
#if (AUDIO_CFG_SSM2377_EN == 0u)
        if (DEF_ABS(half - init_b) >= DEF_ABS(inc_b)) {
            init_b    += inc_b;
            MTU8.TGRB  = init_b;
            loop = DEF_TRUE;
        }
#endif
    } while (loop);

#if (AUDIO_CFG_SSM2377_EN > 0u)
    MTU6.TGRA = half;
#else
    MTU8.TGRA = half;
    MTU8.TGRB = half;
#endif

    MTU9.TIER.BIT.TGIEA = 0u;                                   /* Disable interrupt.                                   */

    if (skip_cnt > 0u) {                                        /* Cfg skip interrupts.                                 */
        MTUB.TITCR.BIT.T3ACOR = skip_cnt - 1u;                  /* Signals at every 'skip_cnt' interrupts               */
        MTUB.TITCR.BIT.T3AEN  = 1u;
    } else {
        MTUB.TITCR.BIT.T3AEN  = 0u;
    }
}


/*
*********************************************************************************************************
*                                             DTC_Init()
*
* Description : Initialize Data Transfer Controller (DTC).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  DTC_Init (void)
{
    CPU_INT16U  i;


    DTC_VectorNull.Reserved = 0u;
    DTC_VectorNull.MRA = 0u;
    DTC_VectorNull.MRB = 0u;
    DTC_VectorNull.SAR = 0u;
    DTC_VectorNull.DAR = 0u;
    DTC_VectorNull.CRA = 0u;
    DTC_VectorNull.CRB = 0u;


    DTC_VectorChA.Reserved = 0u;
#if (AUDIO_CFG_SSM2377_EN > 0u)
    DTC_VectorChA.MRA = 0x18u;                                  /* SAR Address Incremented; Word Transfer.              */
    DTC_VectorChA.MRB = 0u;
    DTC_VectorChA.SAR = (void *)0u;
    DTC_VectorChA.DAR = &MTU6.TGRA;
#else
    DTC_VectorChA.MRA = 0x18u;                                  /* SAR Address Incremented; Word Transfer.              */
    DTC_VectorChA.MRB = 0x80u;                                  /* Chain Tranfer Enabled.                               */
    DTC_VectorChA.SAR = (void *)0u;
    DTC_VectorChA.DAR = &MTU8.TGRA;
#endif
    DTC_VectorChA.CRA = 0u;
    DTC_VectorChA.CRB = 0u;


    DTC_VectorChB.Reserved = 0u;
#if (AUDIO_CFG_SSM2377_EN > 0u)
    DTC_VectorChB.MRA = 0u;
    DTC_VectorChB.MRB = 0u;
    DTC_VectorChB.SAR = 0u;
    DTC_VectorChB.DAR = 0u;
#else
    DTC_VectorChB.MRA = 0x18u;                                  /* SAR Address Incremented; Word Transfer.              */
    DTC_VectorChB.MRB = 0u;
    DTC_VectorChB.SAR = (void *)0u;
    DTC_VectorChB.DAR = &MTU8.TGRB;
#endif
    DTC_VectorChB.CRA = 0u;
    DTC_VectorChB.CRB = 0u;


    DTC.DTCVBR = DTC_VectorTbl;                                 /* Set DTC vector base register.                        */

    for (i = 0u; i < 256u; i++) {
        DTC_VectorTbl[i] = &DTC_VectorNull;
    }

    DTC_VectorTbl[157] = &DTC_VectorChA;                        /* MTU9 TGIA9 DTC Source.                               */

    DTC.DTCCR.BIT.RRS = 1u;                                     /* Skip transfer data read when vector numbers match.   */
}


/*
*********************************************************************************************************
*                                              DTC_En()
*
* Description : Enable Data Transfer Controller (DTC).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  DTC_En (void)
{
    DTC.DTCST.BIT.DTCST = 1u;
    MTU9.TIER.BIT.TGIEA = 1u;                                   /* Enable interrupt on TGRA.                            */

    IPR (MTU9, TGIA9) = 12u;

    DTC_IntAck();

    IEN (MTU9, TGIA9) = 1u;
}


/*
*********************************************************************************************************
*                                              DTC_Dis()
*
* Description : Disable Data Transfer Controller (DTC).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  DTC_Dis (void)
{
    DTC.DTCST.BIT.DTCST = 0u;
    MTU9.TIER.BIT.TGIEA = 0u;

    IPR (MTU9, TGIA9) = 0u;
    DTCE(MTU9, TGIA9) = 0u;
    IR  (MTU9, TGIA9) = 0u;
    IEN (MTU9, TGIA9) = 0u;
}


/*
*********************************************************************************************************
*                                            DTC_IntAck()
*
* Description : Acknowledge interrupt on Data Transfer Controller (DTC).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  DTC_IntAck (void)
{
    DTCE(MTU9, TGIA9) = 1u;
    IR  (MTU9, TGIA9) = 0u;
}


/*
*********************************************************************************************************
*                                            DTC_SetXfer()
*
* Description : Set data transfer on Data Transfer Controller (DTC).
*
* Argument(s) : p_buf           Pointer to audio buffer to transfer.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  DTC_SetXfer(AUDIO_BUF  *p_buf)
{
#if (AUDIO_CFG_SSM2377_EN == 0u)
    void        *p_data_right;
#endif	
    CPU_SIZE_T   size;


    if (p_buf == (AUDIO_BUF *)0) {
        return;
    }

    switch (AudioStream_Channels) {
        case 1u:
#if (AUDIO_CFG_SSM2377_EN == 0u)
             p_data_right = p_buf->DataPtr;
#endif
             size         = p_buf->Samples;
             break;

        case 2u:
#if (AUDIO_CFG_SSM2377_EN == 0u)
             p_data_right = (CPU_INT08U *)p_buf->DataPtr + (AUDIO_CFG_BUF_SIZE / 2u);
#endif
             size         = p_buf->Samples / 2u;
             break;

        default:
             return;
    }

    DTC_VectorChA.SAR = p_buf->DataPtr;
    DTC_VectorChA.CRA = size;

#if (AUDIO_CFG_SSM2377_EN == 0u)
    DTC_VectorChB.SAR = p_data_right;
    DTC_VectorChB.CRA = size;
#endif
}


/*
*********************************************************************************************************
*                                       AudioCallbackHandler()
*
* Description : Audio transfer complete callback handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  AudioCallbackHandler (void)
{
    AUDIO_BUF  *p_buf;
    AUDIO_BUF  *p_buf_next;
    OS_ERR      os_err;


    if (AudioBuf_Q.Entries > 0u) {
        p_buf = AudioBuf_Q.HeadPtr;                             /* Get the first element of the queue.                  */

        if (AudioBuf_Q.Entries > 1u) {
            p_buf_next = p_buf->NextPtr;
                                                                /* ... Start DMA transfer.                              */
            DTC_SetXfer(p_buf_next);
            DTC_IntAck();

            AudioBuf_Q.HeadPtr = p_buf_next;
        } else {
            AudioBuf_Q.HeadPtr = (AUDIO_BUF *)0;
            AudioBuf_Q.TailPtr = (AUDIO_BUF *)0;

            AudioStream_Buffering = DEF_TRUE;

            if (AudioStream_State == AUDIO_STREAM_CLOSING) {
                AudioStream_State  = AUDIO_STREAM_CLOSED;

                OSSemPost(&AudioStream_ClosedSem,               /* Post to stream closed semaphore.                     */
                           OS_OPT_POST_1,
                          &os_err);
            }

            DTC_Dis();
        }

        AudioBuf_Q.Entries--;
        AudioBuf_Abort(p_buf);                                  /* Return buffer header + data to the memory pools.     */
    }
}
