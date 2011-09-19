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
* Filename      : audio.h
* Version       : V1.02
* Programmer(s) : FT
*                 FGK
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               AUDIO_PRESENT present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  AUDIO_PRESENT
#define  AUDIO_PRESENT

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <app_cfg.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*
* Note(s): 1) To route the audio output to the SSM2377 daughter card and to enable gain control, the 
*             AUDIO_CFG_SSM2377_EN must be #define'd to DEF_ENABLED on 'app_cfg.h'.
*********************************************************************************************************
*/

#ifndef  AUDIO_CFG_SSM2377_EN
#define  AUDIO_CFG_SSM2377_EN                   DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                      AUDIO BUFFERING THRESHOLD
*
* Note(s): 1) Audio buffering threshold in milliseconds to delay playback while enqueuing samples.
*********************************************************************************************************
*/

#ifndef  AUDIO_CFG_BUF_TH
#define  AUDIO_CFG_BUF_TH                                   10u
#endif

/*
*********************************************************************************************************
*                                   RESOURCE INTERCHANGE FILE FORMAT (RIFF) DEFINES
*********************************************************************************************************
*/

#define  RIFF_CHUNK_ID                              0x46464952u /* "RIFF" in ASCII.                                     */
#define  RIFF_CHUNK_TYPE_PAL                        0x204C4150u /* "PAL " in ASCII.                                     */
#define  RIFF_CHUNK_TYPE_RDIB                       0x42494452u /* "RDIB" in ASCII.                                     */
#define  RIFF_CHUNK_TYPE_RMID                       0x44494D52u /* "RMID" in ASCII.                                     */
#define  RIFF_CHUNK_TYPE_RMMP                       0x504D4D52u /* "RMMP" in ASCII.                                     */
#define  RIFF_CHUNK_TYPE_WAVE                       0x45564157u /* "WAVE" in ASCII.                                     */

#define  RIFF_WAVE_CHUNK_ID_FMT                     0x20746d66u /* "fmt " in ASCII.                                     */
#define  RIFF_WAVE_CHUNK_ID_DATA                    0x61746164u /* "data" in ASCII.                                     */
#define  RIFF_WAVE_CHUNK_ID_SLNT                    0x746e6c73u /* "slnt" in ASCII.                                     */

#define  RIFF_WAVE_FMT_TAG_PCM                      0x0001      /* Microsoft Pulse Code modulation (PCM) format.        */


/*
*********************************************************************************************************
*                                                ENUMS
*********************************************************************************************************
*/

typedef  enum  audio_stream_fmt {
    AUDIO_STREAM_FMT_NONE = 0,
    AUDIO_STREAM_FMT_04_ADPCM,
    AUDIO_STREAM_FMT_08_UNSIGNED,
    AUDIO_STREAM_FMT_16_UNSIGNED,
    AUDIO_STREAM_FMT_16_SIGNED
} AUDIO_STREAM_FMT;


/*
*********************************************************************************************************
*                                         FORWARD DECLARATION
*********************************************************************************************************
*/

typedef  struct  audio_buf  AUDIO_BUF;


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

struct  audio_buf {
    void        *DataPtr;
    CPU_SIZE_T   Samples;
    AUDIO_BUF   *NextPtr;
};

typedef  struct  riff_chunk {
    CPU_INT32U   ID;
    CPU_INT32U   Size;
} RIFF_CHUNK;

typedef  struct  riff_fmt {
    RIFF_CHUNK   Chunk;
    CPU_INT32U   Fmt;   
} RIFF_FMT;

typedef  struct  pcm_wave_fmt {
    CPU_INT16U   FmtTag;
    CPU_INT16U   NbrCh;
    CPU_INT32U   SamplesPerSec;
    CPU_INT32U   AvgBytesPerSec;
    CPU_INT16U   BlkAlign;
    CPU_INT16U   BitsPerSample;
} PCM_WAVE_FMT;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

CPU_BOOLEAN   Audio_Init               (void);


CPU_BOOLEAN   AudioStream_Open         (CPU_INT08U   fmt,
                                        CPU_INT08U   ch,
                                        CPU_INT32U   sample_freq,
                                        CPU_SIZE_T  *p_samples_buf);
CPU_BOOLEAN   AudioStream_Close        (void);
CPU_BOOLEAN   AudioStream_VolSet       (CPU_INT08U   vol);


AUDIO_BUF    *AudioBuf_Get             (void);
void          AudioBuf_Enqueue         (AUDIO_BUF   *p_buf);
void          AudioBuf_Abort           (AUDIO_BUF   *p_buf);


CPU_SIZE_T    AudioUtil_TimeToSamples  (CPU_INT32U   time);
CPU_SIZE_T    AudioUtil_TimeToOctets   (CPU_INT32U   time);
CPU_SIZE_T    AudioUtil_OctetsToSamples(CPU_SIZE_T   octets);
CPU_SIZE_T    AudioUtil_SamplesToOctets(CPU_SIZE_T   samples);


/*
*********************************************************************************************************
*                                          CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  AUDIO_CFG_BUF_NBR
#error  "AUDIO_CFG_BUF_NBR                      not #define'd in 'app_cfg.h'"

#elif   (AUDIO_CFG_BUF_NBR < 3u)
#error  "AUDIO_CFG_BUF_NBR                 illegaly #define'd in 'app_cfg.h'"
#error  "AUDIO_CFG_BUF_NBR                     [MUST be >= 3]               "
#endif

#ifndef  AUDIO_CFG_BUF_SIZE
#error  "AUDIO_CFG_BUF_SIZE                     not #define'd in 'app_cfg.h'"

#elif  ((AUDIO_CFG_BUF_SIZE % 4u) != 0u)
#error  "AUDIO_CFG_BUF_SIZE                illegaly #define'd in 'app_cfg.h'"
#error  "AUDIO_CFG_BUF_SIZE                    [MUST be multiple of 4]      "
#endif

/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/

#endif
