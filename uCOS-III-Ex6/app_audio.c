/*
*********************************************************************************************************
*                                            uC/Audio Player
*                                         The Real-Time Kernel
*
*                             (c) Copyright 2011, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*
* File    : APP_AUDIO.C
* By      : FGK
* Version : V1.04
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include  <audio.h>

#include  "iodefine.h"


/*
*********************************************************************************************************
*                                         LOCAL ENUMERATIONS
*********************************************************************************************************
*/

typedef  enum  ui_state {                                       /* --------------- USER INTERFACE STATES ------------- */
    PLAYER_HELP = 0,
    PLAYER_IP,
#if (AUDIO_CFG_SSM2377_EN > 0u)
    PLAYER_GAIN,
#endif
    SD_WAIT,
    SD_OPEN,
    SD_GETFILES,
    UI_UPDATE,
    UI_WAIT_ACTION,
    PLAYLIST_PREV,
    PLAYLIST_SELECT,
    PLAYLIST_NEXT,
    PLAYBACK_SELECT_NEXT,
    PLAYBACK_START,
    PLAYBACK_PAUSE,
    PLAYBACK_STOP_START,
    PLAYBACK_PLAY
} UI_STATE;

typedef  enum  msg_id {
    MSG_START = 1,
    MSG_PAUSE,
    MSG_STOP
} MSG_ID; 


/*
*********************************************************************************************************
*                                          PLAYLIST DEFINES
*********************************************************************************************************
*/

#define  APP_SONG_CACHE_NBR                               5u
#define  APP_SONG_NAME_SIZE_MAX                          48u
#define  APP_SONG_NAME_SIZE_MIN                           4u

#define  APP_USER_IF_FS_RETRIES_MAX                       3u

#define  APP_AUDIO_FS_VOL                           "sd:0:"
#define  APP_AUDIO_FS_DEV                           "sd:0:"
#define  APP_AUDIO_FS_FOLDER                        "\\Music\\"


/*
*********************************************************************************************************
*                                         PUSH BUTTON MACRO'S
*********************************************************************************************************
*/

#define  SWITCH1()                                  (!(PORT4.PORT.BIT.B0))
#define  SWITCH2()                                  (!(PORT4.PORT.BIT.B1))
#define  SWITCH3()                                  (!(PORT4.PORT.BIT.B2))


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef  struct  audio_dev {
    FS_FILE      *FilePtr;
    CPU_BOOLEAN   Valid;
    CPU_INT32U    SampleRate;
} AUDIO_DEV;

typedef  struct  audio_info {
    CPU_CHAR     Name[APP_SONG_NAME_SIZE_MAX + 1];
    CPU_BOOLEAN  Valid;
    CPU_INT16U   NbrCh;
    CPU_INT32U   SamplesPerSec;
    CPU_INT32U   AvgBytesPerSec;
    CPU_INT16U   BitsPerSample;
    CPU_INT32U   TimeLen;
} AUDIO_INFO;

typedef  struct  audio_mgr_msg {
    MSG_ID      ID;
    CPU_INT16U  SongIx;
} AUDIO_MGR_MSG;


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB         AudioUI_TCB;
static  CPU_STK        AudioUI_TaskStk[AUDIO_UI_TASK_STK_SIZE];
static  OS_TCB         AudioMgr_TCB;
static  CPU_STK        AudioMgr_TaskStk[AUDIO_MGR_TASK_STK_SIZE];

static  OS_SEM         AudioMgr_Sem;
static  AUDIO_MGR_MSG  AudioMgr_MsgStart;
static  AUDIO_MGR_MSG  AudioMgr_MsgStop;

static  CPU_BOOLEAN    Audio_Playback;
static  CPU_BOOLEAN    Audio_Paused;

static  AUDIO_INFO     AudioSongInfoCache[APP_SONG_CACHE_NBR];
static  CPU_INT16U     AudioSongInfoCacheStart;

static  CPU_BOOLEAN    AudioUI_DisplaySkip;

static  CPU_CHAR       AudioUI_MsgUpdate;
static  CPU_CHAR       AudioUI_MsgRefresh;
static  CPU_CHAR       AudioUI_MsgNextSong;

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_INT16U   AppProbe_SongLstIx;
CPU_CHAR     AppProbe_SongLstName[APP_SONG_NAME_SIZE_MAX + 1];
CPU_INT16U   AppProbe_SongPlayIx;
CPU_CHAR     AppProbe_SongPlayName[APP_SONG_NAME_SIZE_MAX + 1];
CPU_INT16U   AppProbe_SongNbrMax;
CPU_BOOLEAN  AppProbe_SongPlayback;
CPU_BOOLEAN  AppProbe_SongPaused;
#if (AUDIO_CFG_SSM2377_EN > 0u)
CPU_BOOLEAN  AppProbe_AudioLowGain;
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void          AppGraphLCD_IPAddr      (       NET_IF_NBR   if_nbr);
#endif

static  void          AudioUI_Task            (       void         *p_arg);
static  void          AudioMgr_Task           (       void         *p_arg);

static  void          AudioPlayback_Wave      (const  CPU_CHAR     *filename);
static  void          AudioPlayback_ADPCM     (const  CPU_CHAR     *filename);

static  void          AudioUI_Action          (       UI_STATE     *p_state);
static  void          AudioUI_DisplayInfo     (       CPU_BOOLEAN   playback,
                                                      CPU_INT16U    song_play_ix,
                                               const  CPU_CHAR     *song_play_name,
                                                      CPU_INT16U    song_lst_ix,
                                               const  CPU_CHAR     *song_lst_name,
                                                      CPU_INT16U    song_lst_nbr_max);
static  void          AudioUI_DisplayProg     (       CPU_INT32U    pos,
                                                      CPU_INT32U    len,
                                                      CPU_INT32U    samplerate,
                                                      CPU_INT08U    ch);
static  void          AudioUI_Update          (void);
static  void          AudioUI_NextSong        (void);

static  CPU_BOOLEAN   AudioMgr_Action         (void);
static  CPU_BOOLEAN   AudioMgr_Start          (       CPU_INT16U    song_ix);
static  CPU_BOOLEAN   AudioMgr_Stop           (void);
static  CPU_BOOLEAN   AudioMgr_Pause          (void);

static  CPU_BOOLEAN   Audio_IsPlaying         (void);
static  void          Audio_SetPlayback       (       CPU_BOOLEAN   playback,
                                                      CPU_BOOLEAN   paused);
static  CPU_BOOLEAN   Audio_IsPaused          (void);

static  CPU_INT16U    PlayList_GetNbrFiles    (void);
static  CPU_BOOLEAN   PlayList_GetFileName    (       CPU_INT16U    song_ix,
                                                      CPU_CHAR     *p_song_name);
static  AUDIO_INFO   *PlayList_GetFileInfo    (       CPU_INT16U    song_ix,
                                                      CPU_BOOLEAN   force_refresh);

static  CPU_INT32U    AudioADPCM_GetSamplerate(const  CPU_CHAR     *p_name);
static  void          AudioADPCM_GetInfo      (const  CPU_CHAR     *p_name,
                                                      AUDIO_INFO   *p_info);
static  void          AudioWave_GetInfo       (const  CPU_CHAR     *p_name,
                                                      AUDIO_INFO   *p_info);

static  CPU_CHAR     *FSUtil_GetFileName      (       CPU_CHAR     *p_name,
                                               const  CPU_CHAR     *p_fullname,
                                                      CPU_SIZE_T    len_max);
static  CPU_CHAR     *FSUtil_GetFileExt       (       CPU_CHAR     *p_ext,
                                               const  CPU_CHAR     *p_fullname,
                                                      CPU_SIZE_T    len_max);

static  void          LCD_Clr                 (void);
static  void          LCD_Msg                 (const  CPU_CHAR     *msg_line_1,
                                               const  CPU_CHAR     *msg_line_2);
static  void          LCD_Err                 (const  CPU_CHAR     *msg_line_1,
                                               const  CPU_CHAR     *msg_line_2);

static  void          ADC_TmrInit             (void);
static  void          ADC_Init                (void);
static  CPU_INT16U    ADC_PotRd               (void);

#if (AUDIO_CFG_SSM2377_EN > 0u)
static  void          SSM2377_Init            (void);
static  void          SSM2377_En              (       CPU_BOOLEAN   en);
static  void          SSM2377_Gain            (       CPU_BOOLEAN   high);
#endif


/*
*********************************************************************************************************
*                                           AppAudio_Init()
*
* Description : Initialize audio player.
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

void  AppAudio_Init (void)
{
    CPU_BOOLEAN  init;
    OS_ERR       err;


    init = Audio_Init();                                        /* Initialize audio module.                             */

#if (AUDIO_CFG_SSM2377_EN > 0u)
    SSM2377_Init();                                             /* Initialize audio amplifier.                          */
#endif

    LCD_Clr();

    if (init == DEF_TRUE) {
        OSTaskCreate((OS_TCB     *)&AudioUI_TCB,                /* Create audio ui task.                                */
                     (CPU_CHAR   *)"Audio UI Task",
                     (OS_TASK_PTR ) AudioUI_Task,
                     (void       *) 0,
                     (OS_PRIO     ) AUDIO_UI_TASK_PRIO,
                     (CPU_STK    *)&AudioUI_TaskStk[0],
                     (CPU_STK_SIZE) AUDIO_UI_TASK_STK_SIZE / 10u,
                     (CPU_STK_SIZE) AUDIO_UI_TASK_STK_SIZE,
                     (OS_MSG_QTY  ) 10u,
                     (OS_TICK     )  0u,
                     (void       *)  0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);

        OSTaskCreate((OS_TCB     *)&AudioMgr_TCB,               /* Create audio manager task.                           */
                     (CPU_CHAR   *)"Audio Manager Task",
                     (OS_TASK_PTR ) AudioMgr_Task,
                     (void       *) 0,
                     (OS_PRIO     ) AUDIO_MGR_TASK_PRIO,
                     (CPU_STK    *)&AudioMgr_TaskStk[0],
                     (CPU_STK_SIZE) AUDIO_MGR_TASK_STK_SIZE / 10u,
                     (CPU_STK_SIZE) AUDIO_MGR_TASK_STK_SIZE,
                     (OS_MSG_QTY  ) 2u,
                     (OS_TICK     ) 0u,
                     (void       *) 0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);

        OSSemCreate((OS_SEM   *)&AudioMgr_Sem,
                    (CPU_CHAR *)"Audio Manager Signal",
                    (OS_SEM_CTR) 0u,
                    (OS_ERR   *)&err);
    } else {
        LCD_Msg("Audio Player",
                "Failed to Start");
    }

    ADC_TmrInit();
    ADC_Init();
}


/*
*********************************************************************************************************
*                                           AudioUI_Task()
*
* Description : Audio player user interface task.
*
* Argument(s) : p_arg           Argument passed to 'AudioUI_Task()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_Task (void  *p_arg)
{
    OS_ERR       os_err;
    FS_ERR       fs_err;
    CPU_INT32U   fs_open_retries;
    UI_STATE     state;
    UI_STATE     next_state;
    CPU_INT16U   song_lst_nbr_max;
    CPU_INT16U   song_lst_ix;
    CPU_CHAR     song_lst_name[APP_SONG_NAME_SIZE_MAX + 1];
    CPU_INT16U   song_play_ix;
    CPU_CHAR     song_play_name[APP_SONG_NAME_SIZE_MAX + 1];
    CPU_BOOLEAN  playback;
    CPU_BOOLEAN  paused;
    CPU_BOOLEAN  valid;
    CPU_BOOLEAN  loop;
    CPU_INT32U   ix;
    CPU_SR_ALLOC();


    Str_Copy(song_lst_name,  "");
    Str_Copy(song_play_name, "");

    playback = DEF_FALSE;
    paused   = DEF_FALSE;

    song_lst_nbr_max = 0u;
    song_lst_ix      = 0u;
    song_play_ix     = 0u;

    AppProbe_SongNbrMax   = song_lst_nbr_max;
    AppProbe_SongLstIx    = song_lst_ix;
    AppProbe_SongPlayIx   = song_play_ix;
    AppProbe_SongPlayback = playback;
    AppProbe_SongPaused   = paused;

    Str_Copy(AppProbe_SongLstName,  " ");
    Str_Copy(AppProbe_SongPlayName, " ");

    AudioUI_DisplaySkip = DEF_FALSE;

    state      = PLAYER_HELP;
    next_state = PLAYER_HELP;

    while (DEF_TRUE) {
        switch (state) {
            case PLAYER_HELP:
#if (AUDIO_CFG_SSM2377_EN == 0u)
                 BSP_GraphLCD_String(2u, "Help: (Press SW2)");
                 BSP_GraphLCD_String(3u, "SW1 for prev song");
                 BSP_GraphLCD_String(4u, "SW2 for play/pause");
                 BSP_GraphLCD_String(5u, "SW3 for next song");
                 BSP_GraphLCD_String(6u, "SW1+SW2 refresh SD");
                 BSP_GraphLCD_String(7u, "SW1+SW3 for IP addr");
#else
                 BSP_GraphLCD_String(2u, "SW1 for prev song");
                 BSP_GraphLCD_String(3u, "SW2 for play/pause");
                 BSP_GraphLCD_String(4u, "SW3 for next song");
                 BSP_GraphLCD_String(5u, "SW1+SW2 refresh SD");
                 BSP_GraphLCD_String(6u, "SW1+SW3 for IP addr");
                 BSP_GraphLCD_String(7u, "SW2+SW3 gain ctrl");
#endif

                 loop = DEF_TRUE;
                 while (loop == DEF_TRUE) {
                     if (SWITCH1() && SWITCH3()) {
                         state      = PLAYER_IP;
                         next_state = PLAYER_HELP;
                         loop       = DEF_FALSE;
#if (AUDIO_CFG_SSM2377_EN > 0u)
                     } else if (SWITCH2() && SWITCH3()) {
                         state      = PLAYER_GAIN;
                         next_state = PLAYER_HELP;
                         loop       = DEF_FALSE;
#endif
                     } else if (SWITCH2()) {
                         state = SD_WAIT;
                         loop  = DEF_FALSE;
                         LCD_Clr();
                                                                /* Debounce switch.                                     */
                         do {
                             OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                                           OS_OPT_TIME_HMSM_STRICT,
                                           &os_err);
                         } while (SWITCH2());
                     } else {
                         OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                                       OS_OPT_TIME_HMSM_STRICT,
                                       &os_err);
                     }
                 }
                 break;


            case PLAYER_IP:
                 CPU_CRITICAL_ENTER();
                 AudioUI_DisplaySkip = DEF_TRUE;
                 CPU_CRITICAL_EXIT();

                 ix = 0;
                 do {
                     if ((ix % 5) == 0) {
                         ix = 0;
                         
                         LCD_Clr();
                         AppGraphLCD_IPAddr(1u);
                     }
                     ix++;

                     OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);
                 } while (SWITCH1() || SWITCH3());              /* Wait until both SW1 & SW3 released.                  */

                 LCD_Clr();

                 CPU_CRITICAL_ENTER();
                 AudioUI_DisplaySkip = DEF_FALSE;
                 CPU_CRITICAL_EXIT();

                 state = next_state;
                 break;


#if (AUDIO_CFG_SSM2377_EN > 0u)
            case PLAYER_GAIN:
                 CPU_CRITICAL_ENTER();
                 AudioUI_DisplaySkip = DEF_TRUE;
                 CPU_CRITICAL_EXIT();

                 LCD_Clr();

                 BSP_GraphLCD_String(2u, "Gain: (Press SW2)");
                 BSP_GraphLCD_String(4u, "  SW1 for low gain");
                 if (AppProbe_AudioLowGain == DEF_YES) {
                    BSP_GraphLCD_String(4u, "*");
                 }
                 BSP_GraphLCD_String(6u, "  SW3 for high gain");
                 if (AppProbe_AudioLowGain == DEF_NO) {
                    BSP_GraphLCD_String(6u, "*");
                 }
                 
                 do {
                     OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);
                 } while (SWITCH2() || SWITCH3());              /* Wait until both SW2 & SW3 released.                  */

                 while (DEF_ON) {
                                                                /* SW1: Low Gain                                        */
                     if (SWITCH1() && !(SWITCH2() ||SWITCH3())) {

                         do {
                             OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                                           OS_OPT_TIME_HMSM_STRICT,
                                           &os_err);
                         } while (SWITCH1() && !(SWITCH2() || SWITCH3()));

                         AppProbe_AudioLowGain = DEF_YES;
                         break;
                                                                /* SW3: High Gain                                       */
                     } else if (SWITCH3() && !(SWITCH1() ||SWITCH2())) {

                         do {
                             OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                                           OS_OPT_TIME_HMSM_STRICT,
                                           &os_err);
                         } while (SWITCH1() && !(SWITCH2() || SWITCH3()));

                         AppProbe_AudioLowGain = DEF_NO;
                         break;
                                                                /* SW2: Select                                          */
                     } else if (SWITCH2() && !(SWITCH1() ||SWITCH3())) {

                         do {
                             OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                                           OS_OPT_TIME_HMSM_STRICT,
                                           &os_err);
                         } while (SWITCH2() && !(SWITCH1() || SWITCH3()));

                         if (AppProbe_AudioLowGain == DEF_YES) {
                             SSM2377_Gain(DEF_NO);
                         } else {
                             SSM2377_Gain(DEF_YES);
                         }

                         state = next_state;

                         CPU_CRITICAL_ENTER();
                         AudioUI_DisplaySkip = DEF_FALSE;
                         CPU_CRITICAL_EXIT();
                         break;
                     }
                     
                     OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);
                 }
                 
                 LCD_Clr();
                 break;
#endif


            case SD_WAIT:
                 Str_Copy(song_lst_name,  "");
                 Str_Copy(song_play_name, "");

                 playback = DEF_FALSE;
                 paused   = DEF_FALSE;

                 song_lst_nbr_max = 0u;
                 song_lst_ix      = 0u;
                 song_play_ix     = 0u;

                 AppProbe_SongNbrMax   = song_lst_nbr_max;
                 AppProbe_SongLstIx    = song_lst_ix;
                 AppProbe_SongPlayIx   = song_play_ix;
                 AppProbe_SongPlayback = playback;
                 AppProbe_SongPaused   = paused;

                 Str_Copy(AppProbe_SongLstName,  " ");
                 Str_Copy(AppProbe_SongPlayName, " ");

                 for (ix = 0; ix < APP_SONG_CACHE_NBR; ix++) {
                    Str_Copy(AudioSongInfoCache[ix].Name, "");

                    AudioSongInfoCache[ix].NbrCh          = 0u;
                    AudioSongInfoCache[ix].SamplesPerSec  = 0u;
                    AudioSongInfoCache[ix].AvgBytesPerSec = 0u;
                    AudioSongInfoCache[ix].BitsPerSample  = 0u;
                    AudioSongInfoCache[ix].TimeLen        = 0u;
                 }
                 AudioSongInfoCacheStart = DEF_INT_16U_MAX_VAL;

                 LCD_Msg("Press SW2 to load",
                         "files from SD card");

                 while (!SWITCH2() || SWITCH1()) {
                     OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);
                 }

                                                                /* Debounce switch.                                     */
                 do {
                     OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);
                 } while (SWITCH2());

                 LCD_Clr();

                 state = SD_OPEN;
                 break;


            case SD_OPEN:
                 song_lst_ix = 0u;

                 fs_open_retries = APP_USER_IF_FS_RETRIES_MAX;  /* Open file system volume.                           */                  
                 do {
                     FSVol_Open(APP_AUDIO_FS_VOL, APP_AUDIO_FS_DEV, 0, &fs_err);

                     fs_open_retries--;
                     OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                                   OS_OPT_TIME_HMSM_STRICT,
                                   &os_err);

                 } while ((fs_err          != FS_ERR_NONE) &&
                          (fs_err          != FS_ERR_VOL_ALREADY_OPEN) &&
                          (fs_open_retries >  0));

                 if ((fs_err != FS_ERR_NONE) &&
                     (fs_err != FS_ERR_VOL_ALREADY_OPEN)) {
                     LCD_Err("SD Card Mnt Failed",
                             "FSVol_Open Err");
                     state = SD_WAIT;
                 } else {
                     state = SD_GETFILES;
                 }

                 break;


            case SD_GETFILES:
                 song_lst_nbr_max = PlayList_GetNbrFiles();
                 if (song_lst_nbr_max > 0u) {
                     if ((song_lst_ix == 0u) ||
                         (song_lst_ix >  song_lst_nbr_max)) {
                          song_lst_ix  = 1u;
                     }

                    (void)PlayList_GetFileInfo(song_lst_ix, DEF_TRUE);

                     valid = PlayList_GetFileName(song_lst_ix, song_lst_name);
                 } else {
                     LCD_Err("No Songs Found",
                             "in " APP_AUDIO_FS_FOLDER " folder");

                     valid = DEF_FALSE;
                 }

                 if (valid == DEF_TRUE) {
                     Str_Copy_N(AppProbe_SongLstName,  song_lst_name, APP_SONG_NAME_SIZE_MAX);
                     FSUtil_GetFileName(song_lst_name, song_lst_name, 19u);

                     AppProbe_SongLstIx  = song_lst_ix;
                     AppProbe_SongNbrMax = song_lst_nbr_max;

                     state = UI_UPDATE;
                 } else {
                     state = SD_WAIT;
                 }
                 break;


            case UI_UPDATE:
                 playback = Audio_IsPlaying();

                 AudioUI_DisplayInfo(playback,
                                     song_play_ix,
                                     song_play_name,
                                     song_lst_ix,
                                     song_lst_name,
                                     song_lst_nbr_max);

                 state = UI_WAIT_ACTION;
                 break;


            case UI_WAIT_ACTION:
                 next_state = UI_UPDATE;
                 AudioUI_Action(&state);
                 break;


            case PLAYLIST_PREV:
                 if (song_lst_ix > 1) {
                     song_lst_ix--;
                 } else {
                     song_lst_ix = song_lst_nbr_max;
                 }
                 valid = PlayList_GetFileName(song_lst_ix, song_lst_name);
                 if (valid == DEF_TRUE) {
                     Str_Copy_N(AppProbe_SongLstName,  song_lst_name, APP_SONG_NAME_SIZE_MAX);
                     FSUtil_GetFileName(song_lst_name, song_lst_name, 19);

                     AppProbe_SongLstIx = song_lst_ix;

                     state = UI_UPDATE;
                 } else {
                     state = SD_WAIT;
                 }
                 break;


            case PLAYLIST_NEXT:
                 if (song_lst_ix < song_lst_nbr_max) {
                     song_lst_ix++;
                 } else {
                     song_lst_ix = 1;
                 }
                 valid = PlayList_GetFileName(song_lst_ix, song_lst_name);
                 if (valid == DEF_TRUE) {
                     Str_Copy_N(AppProbe_SongLstName,  song_lst_name, APP_SONG_NAME_SIZE_MAX);
                     FSUtil_GetFileName(song_lst_name, song_lst_name, 19);

                     AppProbe_SongLstIx = song_lst_ix;

                     state = UI_UPDATE;
                 } else {
                     state = SD_WAIT;
                 }
                 break;


            case PLAYLIST_SELECT:
                 playback = Audio_IsPlaying();

                 if (playback) {
                     if (song_play_ix != song_lst_ix) {
                         state = PLAYBACK_STOP_START;
                     } else {
                         state = PLAYBACK_PAUSE;
                     }
                 } else {
                     paused = Audio_IsPaused();

                     if (paused &&
                        (song_play_ix == song_lst_ix)) {
                         state = PLAYBACK_START;
                     } else {
                         state = PLAYBACK_STOP_START;
                     }
                 }
                 break;


            case PLAYBACK_SELECT_NEXT:
                 if (song_play_ix < song_lst_nbr_max) {
                     song_play_ix++;
                 } else {
                     song_play_ix = 1;
                 }
                 state = PLAYBACK_PLAY;
                 break;


            case PLAYBACK_PAUSE:
                 (void)AudioMgr_Pause();
                 state = UI_UPDATE;
                 break;


            case PLAYBACK_STOP_START:
                 valid = AudioMgr_Stop();
                 if (valid == DEF_OK) {
                     state = PLAYBACK_START;
                 } else {
                     state = UI_UPDATE;
                 }
                 break;


            case PLAYBACK_START:
                 paused = Audio_IsPaused();

                 if (paused) {
                     valid = AudioMgr_Start(song_play_ix);
                     if (valid != DEF_TRUE) {
                         Str_Copy(song_play_name, "");
                         Str_Copy(AppProbe_SongPlayName, " ");
                     }

                     state = UI_UPDATE;

                 } else {
                     song_play_ix = song_lst_ix;
                     state = PLAYBACK_PLAY;
                 }
                 break;


            case PLAYBACK_PLAY:
                 valid = PlayList_GetFileName(song_play_ix, song_play_name);
                 if (valid == DEF_TRUE) {
                     valid = AudioMgr_Start(song_play_ix);
                     if (valid == DEF_TRUE) {
                         Str_Copy_N(AppProbe_SongPlayName, song_play_name, APP_SONG_NAME_SIZE_MAX);
                         FSUtil_GetFileName(song_play_name, song_play_name, 19);
                     } else {
                         Str_Copy(song_play_name, "");
                         Str_Copy(AppProbe_SongPlayName, " ");
                     }

                     AppProbe_SongPlayIx = song_play_ix;
                     
                     state = UI_UPDATE;

                 } else {
                     state = SD_WAIT;
                 }
                 break;
        }
    }
}


/*
*********************************************************************************************************
*                                           AudioMgr_Task()
*
* Description : Manages playback of an audio file.
*
* Argument(s) : p_arg           Argument passed to 'AudioMgr_Task()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioMgr_Task (void  *p_arg)
{
                                                                /* Allow space for directory name.                      */
    CPU_CHAR        fullname[APP_SONG_NAME_SIZE_MAX + sizeof(APP_AUDIO_FS_FOLDER) + 1];
    CPU_CHAR       *p_filename;
    CPU_CHAR        file_ext[4 + 1];
    CPU_CHAR       *p_ext;
    AUDIO_MGR_MSG  *p_msg;
    OS_MSG_SIZE     msg_size;
    CPU_BOOLEAN     valid;
    CPU_SIZE_T      len;
    OS_ERR          os_err;


    (void)p_arg;

    Audio_SetPlayback(DEF_FALSE, DEF_FALSE);

    while (DEF_TRUE) {
                                                                /* Wait for a file to be played.                        */
        p_msg = (AUDIO_MGR_MSG *)OSTaskQPend((OS_TICK      ) 0u,
                                             (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                                             (OS_MSG_SIZE *)&msg_size,
                                             (CPU_TS      *) 0,
                                             (OS_ERR      *)&os_err);
        if (os_err != OS_ERR_NONE) {
            continue;
        }

        Audio_SetPlayback(DEF_FALSE, DEF_FALSE);

        OSSemPost(&AudioMgr_Sem,                                /* Ack msg to Audio UI.                                 */
                   OS_OPT_POST_1,
                  &os_err);

        if (msg_size != sizeof(AUDIO_MGR_MSG)) {
            continue;
        }
        if (p_msg->ID != MSG_START) {
            continue;
        }

        Str_Copy(fullname, APP_AUDIO_FS_FOLDER);

        len = Str_Len(fullname);
        p_filename = &fullname[len];

        valid = PlayList_GetFileName(p_msg->SongIx, p_filename);
        if (valid == DEF_FAIL) {
            continue;
        }

        p_ext = FSUtil_GetFileExt(file_ext, p_filename, 4);

        if (Str_CmpIgnoreCase_N(p_ext, "wav", 4) == 0) {        /* RIFF WAVE file.                                      */
            AudioPlayback_Wave(fullname);

        } else if (Str_CmpIgnoreCase_N(p_ext, "dat", 4) == 0) { /* ADPCM file.                                          */
            AudioPlayback_ADPCM(fullname);
        } else {
            LCD_Err("Invalid Audio File",
                    "Unknown Extension");
        }
    }
}


/*
*********************************************************************************************************
*                                        AudioPlayback_Wave()
*
* Description : Process Wave file for playback.
*
* Argument(s) : filename        Pointer to file name.
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioPlayback_Wave (const  CPU_CHAR  *filename)
{
    CPU_BOOLEAN    open;
    AUDIO_BUF     *p_audio_buf;
    FS_FILE       *p_file;
    CPU_SIZE_T     octets_rd;
    CPU_SIZE_T     len;
    CPU_INT32U     size_rem;
    RIFF_FMT       riff_fmt;
    RIFF_CHUNK     riff_chunk;
    PCM_WAVE_FMT   pcm_wave_fmt;
    FS_ERR         fs_err;
    CPU_BOOLEAN    song_end;
    CPU_BOOLEAN    song_stop;
    CPU_SIZE_T     samples_max;
    CPU_SIZE_T     octets;
    CPU_INT32U     size_th;
    CPU_INT32U     size_prog;


    LCD_Err("", "");                                            /* Clr err msgs.                                        */

    p_file = FSFile_Open((CPU_CHAR *)filename,
                         FS_FILE_ACCESS_MODE_RD |
                         FS_FILE_ACCESS_MODE_CACHED,
                         &fs_err);

    if (fs_err != FS_ERR_NONE) {
        LCD_Err("SD Card Read Failed",
                "FSFile_Open Err");
        return;
    }

    octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                          (void     *)&riff_fmt,
                          (CPU_SIZE_T) sizeof(RIFF_FMT),
                          (FS_ERR   *)&fs_err);

    if((fs_err            != FS_ERR_NONE)      ||
       (octets_rd         != sizeof(RIFF_FMT)) ||
       (riff_fmt.Chunk.ID != RIFF_CHUNK_ID)    ||
       (riff_fmt.Fmt      != RIFF_CHUNK_TYPE_WAVE)) {
        FSFile_Close(p_file, &fs_err);

        if (fs_err != FS_ERR_NONE) {
            LCD_Err("SD Card Read Failed",
                    "FSFile_Rd Err");
        } else {
            LCD_Err("Invalid Wave File",
                    "");
        }
        return;
    }

    song_end  = DEF_NO;
    song_stop = DEF_NO;

    while ((song_end  == DEF_NO) &&
           (song_stop == DEF_NO)) {

        octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                              (void     *)&riff_chunk,
                              (CPU_SIZE_T) sizeof(RIFF_CHUNK),
                              (FS_ERR   *)&fs_err);

        if ((fs_err    != FS_ERR_NONE) ||
            (octets_rd != sizeof(RIFF_CHUNK))) {
             song_end = DEF_YES;
             continue;
        }

        switch (riff_chunk.ID) {
            case RIFF_WAVE_CHUNK_ID_FMT:
                 Mem_Clr(&pcm_wave_fmt, sizeof(PCM_WAVE_FMT));

                 len = DEF_MIN(riff_chunk.Size, sizeof(PCM_WAVE_FMT));

                 octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                                       (void     *)&pcm_wave_fmt,
                                       (CPU_SIZE_T) len,
                                       (FS_ERR   *)&fs_err);

                 if ((fs_err    != FS_ERR_NONE) ||
                     (octets_rd != len)) {
                      song_end   = DEF_YES;

                      LCD_Err("SD Card Read Failed",
                              "FSFile_Rd Err (Fmt)");
                      continue;
                 }

                 if (pcm_wave_fmt.FmtTag != RIFF_WAVE_FMT_TAG_PCM) {
                     song_end = DEF_YES;

                     LCD_Err("Invalid Wave File",
                             "Unsupported Format");
                     continue;
                 }
                                                                /* Skip additional fmt fields.                          */
                 if (riff_chunk.Size > sizeof(PCM_WAVE_FMT)) {
                     len = riff_chunk.Size - sizeof(PCM_WAVE_FMT);

                     FSFile_PosSet((FS_FILE      *) p_file,
                                   (FS_FILE_OFFSET) len,
                                   (FS_STATE      ) FS_FILE_ORIGIN_CUR,
                                   (FS_ERR       *)&fs_err);

                     if (fs_err   != FS_ERR_NONE) {
                         song_end  = DEF_YES;

                         LCD_Err("SD Card Read Failed",
                                 "FSFile_PosSet Err");
                         continue;
                     }
                 }

                 switch (pcm_wave_fmt.BitsPerSample) {
                     case 8u:
                          open = AudioStream_Open(AUDIO_STREAM_FMT_08_UNSIGNED,
                                                  pcm_wave_fmt.NbrCh,
                                                  pcm_wave_fmt.SamplesPerSec,
                                                 &samples_max);
                          break;

                     case 16u:
                          open = AudioStream_Open(AUDIO_STREAM_FMT_16_SIGNED,
                                                  pcm_wave_fmt.NbrCh,
                                                  pcm_wave_fmt.SamplesPerSec,
                                                 &samples_max);
                          break;

                     default:
                          song_end = DEF_YES;
                          continue;
                 }

                 if (open == DEF_YES) {
                     octets = AudioUtil_SamplesToOctets(samples_max);
                     
                     Audio_SetPlayback(DEF_TRUE, DEF_FALSE);
                     AudioUI_Update();
                 } else {
                     song_end = DEF_YES;

                     LCD_Err("Invalid Audio File",
                             "Stream Open Failed");
                     continue;
                 }
                 break;


            case RIFF_WAVE_CHUNK_ID_DATA:
                 size_prog = 0u;
                                                                /* Threshold to display progress: MIN(500ms, 0.5%).     */
                 size_th = DEF_MIN(AudioUtil_TimeToOctets(500) * pcm_wave_fmt.NbrCh,
                                   riff_chunk.Size / 200);

                 size_rem = riff_chunk.Size;

                 AudioUI_DisplayProg(riff_chunk.Size - size_rem,
                                     riff_chunk.Size,
                                     pcm_wave_fmt.SamplesPerSec,
                                     pcm_wave_fmt.NbrCh);

                 while ((song_end  == DEF_NO) &&
                        (song_stop == DEF_NO) &&
                        (size_rem  != 0u)) {

                     p_audio_buf = AudioBuf_Get();
                     if (p_audio_buf == (AUDIO_BUF *)0) {
                         song_end = DEF_YES;
                         continue;
                     }

                     octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                                           (void     *) p_audio_buf->DataPtr,
                                           (CPU_SIZE_T) DEF_MIN(size_rem, octets),
                                           (FS_ERR   *)&fs_err);

                     p_audio_buf->Samples = AudioUtil_OctetsToSamples(octets_rd);

                     if ((fs_err == FS_ERR_NONE) &&
                         (octets_rd > 0)) {

                         size_rem  -= octets_rd;
                         size_prog += octets_rd;

                         if (size_prog >  size_th) {
                             size_prog %= size_th;
                             AudioUI_DisplayProg(riff_chunk.Size - size_rem,
                                                 riff_chunk.Size,
                                                 pcm_wave_fmt.SamplesPerSec,
                                                 pcm_wave_fmt.NbrCh);

                             song_stop = AudioMgr_Action();
                         }

                         AudioBuf_Enqueue(p_audio_buf);
                         if (octets_rd != octets) {             /* Chk if EOF.                                          */
                             song_end   = DEF_YES;
                         }

                     } else {
                         AudioBuf_Abort(p_audio_buf);
                         song_end = DEF_YES;
                     }
                 }

                 AudioUI_DisplayProg(riff_chunk.Size - size_rem,
                                     riff_chunk.Size,
                                     pcm_wave_fmt.SamplesPerSec,
                                     pcm_wave_fmt.NbrCh);
                 break;


            default:
                 FSFile_PosSet((FS_FILE      *) p_file,
                               (FS_FILE_OFFSET) riff_chunk.Size,
                               (FS_STATE      ) FS_FILE_ORIGIN_CUR,
                               (FS_ERR       *)&fs_err);

                 if (fs_err != FS_ERR_NONE) {
                     song_end = DEF_YES;
                     continue;
                 }

                 break;
        }
    }

    FSFile_Close(p_file, &fs_err);
    AudioStream_Close();

    Audio_SetPlayback(DEF_FALSE, DEF_FALSE);
    if (song_stop == DEF_NO) {
        AudioUI_NextSong();
    } else {
        AudioUI_Update();
    }
}


/*
*********************************************************************************************************
*                                        AudioPlayback_ADPCM()
*
* Description : Process ADPCM file for playback.
*
* Argument(s) : filename        Pointer to file name.
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioPlayback_ADPCM (const  CPU_CHAR  *filename)
{
    CPU_CHAR       *p_samplerate;
    CPU_INT32U      samplerate;
    CPU_BOOLEAN     open;
    AUDIO_BUF      *p_audio_buf;
    FS_FILE        *p_file;
    CPU_SIZE_T      octets_rd;
    CPU_INT32U      size_rem;
    FS_ERR          fs_err;
    CPU_BOOLEAN     song_end;
    CPU_BOOLEAN     song_stop;
    CPU_SIZE_T      samples_max;
    CPU_SIZE_T      octets;
    CPU_INT32U      size_th;
    CPU_INT32U      size_prog;
    FS_ENTRY_INFO   fs_info;


    p_samplerate = Str_Char_Last(filename, '-');

    if (p_samplerate == (CPU_CHAR *)0) {
        LCD_Err("Invalid ADPCM File",
                "No Sample Rate Info");
        return;
    }
    samplerate = Str_ParseNbr_Int32U(            &p_samplerate[1],
                                     (CPU_CHAR **)0,
                                                  DEF_NBR_BASE_DEC);
    switch (samplerate) {
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
             LCD_Err("Invalid ADPCM File",
                     "Sample Rate Invalid");
             return;
    }

    song_end  = DEF_NO;
    song_stop = DEF_NO;

    Audio_SetPlayback(DEF_TRUE, DEF_FALSE);
    AudioUI_Update();

    LCD_Err("", "");                                            /* Clr err msgs.                                        */

    p_file = FSFile_Open((CPU_CHAR *)filename,
                         FS_FILE_ACCESS_MODE_RD |
                         FS_FILE_ACCESS_MODE_CACHED,
                         &fs_err);
    if (fs_err != FS_ERR_NONE) {
        LCD_Err("SD Card Read Failed",
                "FSFile_Open Err");
        return;
    }

    FSFile_Query(p_file, &fs_info, &fs_err);
    if (fs_err != FS_ERR_NONE) {
        LCD_Err("SD Card Read Failed",
                "FSFile_Query Err");
        return;
    }

    open = AudioStream_Open(AUDIO_STREAM_FMT_04_ADPCM,
                            1u,
                            samplerate,
                           &samples_max);
    if (open != DEF_YES) {
        song_end = DEF_YES;

        LCD_Err("Invalid Audio File",
                "Stream Open Failed");
    }

    size_prog = 0u;
    octets    = AudioUtil_SamplesToOctets(samples_max);
    size_th   = DEF_MIN(AudioUtil_TimeToOctets(500),            /* Threshold to display progress: MIN(500ms, 0.5%).     */
                        fs_info.Size / 200);
    size_rem  = fs_info.Size;

    AudioUI_DisplayProg(fs_info.Size - size_rem,
                        fs_info.Size,
                        samplerate,
                        1u);

    while ((song_end  == DEF_NO) &&
           (song_stop == DEF_NO) &&
           (size_rem  != 0u)) {

        p_audio_buf = AudioBuf_Get();

        if (p_audio_buf == (AUDIO_BUF *)0) {
            song_end = DEF_YES;
            continue;
        }

        octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                              (void     *) p_audio_buf->DataPtr,
                              (CPU_SIZE_T) DEF_MIN(size_rem, octets),
                              (FS_ERR   *)&fs_err);

        p_audio_buf->Samples = AudioUtil_OctetsToSamples(octets_rd);

        if ((fs_err == FS_ERR_NONE) &&
            (octets_rd > 0)) {

            size_rem  -= octets_rd;
            size_prog += octets_rd;

            if (size_prog >  size_th) {
                size_prog %= size_th;
                AudioUI_DisplayProg(fs_info.Size - size_rem,
                                    fs_info.Size,
                                    samplerate,
                                    1u);

                song_stop = AudioMgr_Action();
            }

            AudioBuf_Enqueue(p_audio_buf);
            if (octets_rd != octets) {                          /* Chk if EOF.                                          */
                song_end   = DEF_YES;
            }

        } else {
            AudioBuf_Abort(p_audio_buf);
            song_end = DEF_YES; 
        }
    }

    AudioUI_DisplayProg(fs_info.Size - size_rem,
                        fs_info.Size,
                        samplerate,
                        1u);

    FSFile_Close(p_file, &fs_err);
    AudioStream_Close();

    Audio_SetPlayback(DEF_FALSE, DEF_FALSE);
    if (song_stop == DEF_NO) {
        AudioUI_NextSong();
    } else {
        AudioUI_Update();
    }
}


/*
*********************************************************************************************************
*                                          AudioUI_Action()
*
* Description : Process input actions for user interface.
*
* Argument(s) : p_state         Pointer to return user interface state after input.
*
* Return(s)   : none.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_Action (UI_STATE  *p_state)
{
    static  CPU_INT08U    prev_button = 0u;
    static  CPU_INT08U    prev_cnt    = 0u;
            CPU_INT08U    button_th;
            CPU_BOOLEAN   loop;
            CPU_INT16U    vol;
            OS_TICK       timeout;
            OS_ERR        os_err;
            void         *p_msg;
            OS_MSG_SIZE   msg_size;


    if (p_state == (UI_STATE *)0) {
        return;
    }

    loop = DEF_TRUE;
    while (loop == DEF_TRUE) {

        if (SWITCH1() && SWITCH3()) {
           *p_state = PLAYER_IP;
            loop    = DEF_FALSE;

            prev_button = 0u;

        } else if (SWITCH1() && SWITCH2()) {
            AudioMgr_Stop();
            LCD_Clr();
           *p_state = SD_WAIT;
            loop    = DEF_FALSE;

            prev_button = 0u;
#if (AUDIO_CFG_SSM2377_EN > 0u)
        } else if (SWITCH2() && SWITCH3()) {
           *p_state    = PLAYER_GAIN;
            loop       = DEF_FALSE;
            
            prev_button = 0u;
#endif
        } else if (SWITCH1()) {                                 /* SW1: Previous                                        */

            if (prev_button != 1u) {
                prev_cnt = 0u;
            } else {
                if (prev_cnt < 24u) {
                    prev_cnt++;
                }
            }

            button_th = 250u / (prev_cnt + 7u);

            do {
                OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                              OS_OPT_TIME_HMSM_STRICT,
                              &os_err);

                button_th--;
            } while (SWITCH1() && !SWITCH2() && (button_th > 0u));

            if (SWITCH2()) {
                AudioMgr_Stop();
                LCD_Clr();
               *p_state = SD_WAIT;
            } else {
               *p_state = PLAYLIST_PREV;
            }

            loop        = DEF_FALSE;
            prev_button = 1u;

        } else if (SWITCH2()) {                                 /* SW2: Select                                          */

            prev_cnt = 0u;
                                                                /* Debounce switch.                                     */
            do {
                OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                              OS_OPT_TIME_HMSM_STRICT,
                              &os_err);
            } while (SWITCH2() && !SWITCH1());

            if (SWITCH1()) {
                AudioMgr_Stop();
                LCD_Clr();
               *p_state = SD_WAIT;
            } else {
               *p_state = PLAYLIST_SELECT;
            }

            loop        = DEF_FALSE;
            prev_button = 2u;

        } else if (SWITCH3()) {                                 /* SW3: Next                                            */

            if (prev_button != 3u) {
                prev_cnt = 0u;
            } else {
                if (prev_cnt < 24u) {
                    prev_cnt++;
                }
            }

            button_th = 250u / (prev_cnt + 7u);
            
            do {
                OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                              OS_OPT_TIME_HMSM_STRICT,
                              &os_err);

                button_th--;
            } while (SWITCH3() && (button_th > 0u));

           *p_state = PLAYLIST_NEXT;
            loop    = DEF_FALSE;

            prev_button = 3u;

        } else {

            if (prev_button != 0u) {
                prev_cnt = 0u;
            } else {
                if (prev_cnt < 10u) {
                    prev_cnt++;
                }
            }

            if (prev_cnt >= 10u) {
                prev_cnt  =  0u;

                vol  = ADC_PotRd();
                vol /= 762u;                                    /* Limit volume to 86% to avoid artifacts in output.    */

                AudioStream_VolSet(vol);
            }

            prev_button = 0u;
            timeout = 25u * DEF_TIME_NBR_mS_PER_SEC /
                      OSCfg_TickRate_Hz;

            p_msg = OSTaskQPend( timeout,
                                 OS_OPT_PEND_BLOCKING,
                                &msg_size,
                                (CPU_TS *)0,
                                &os_err);

            if (os_err == OS_ERR_NONE) {                        /* Recv'd async signal.                                 */
                if (p_msg == &AudioUI_MsgUpdate) {
                   *p_state = UI_UPDATE;
                    loop    = DEF_FALSE;
                } else if (p_msg == &AudioUI_MsgRefresh) {
                   *p_state = SD_GETFILES;
                    loop    = DEF_FALSE;
                } else if (p_msg == &AudioUI_MsgNextSong) {
                   *p_state = PLAYBACK_SELECT_NEXT;
                    loop    = DEF_FALSE;
                }
            }
        }
    }
}


/*
*********************************************************************************************************
*                                        AudioUI_DisplayInfo()
*
* Description : Display audio information on user interface.
*
* Argument(s) : playback            Audio player is in playback mode.
*
*               song_play_ix        Index of playback song.
*
*               song_play_name      Pointer to name of playback song.
*
*               song_lst_ix         Index of currently selected song.
*
*               song_lst_name       Pointer to name of currently selected song.
*
*               song_lst_nbr_max    Maximum number of songs.
*
* Return(s)   : none.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_DisplayInfo (       CPU_BOOLEAN   playback,
                                          CPU_INT16U    song_play_ix,
                                   const  CPU_CHAR     *song_play_name,
                                          CPU_INT16U    song_lst_ix,
                                   const  CPU_CHAR     *song_lst_name,
                                          CPU_INT16U    song_lst_nbr_max)
{
    CPU_CHAR      songs_msg[10 + 1];
    CPU_CHAR      songs_max_msg[3 + 1];
    CPU_INT32U    tot_time;
    CPU_CHAR      str_time_sec[3];
    CPU_CHAR      str_tot_time[6];
    CPU_CHAR      str_bps_ch[8];
    CPU_CHAR      str_samplerate[6];
    CPU_BOOLEAN   valid;
    AUDIO_INFO   *p_info;


    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_SYMBOL);
    BSP_GraphLCD_CharPixel(2u,  0u, 1u);                        /* Previous symbol.                                     */ 
    if (playback) {
        if (song_play_ix == song_lst_ix) {
            BSP_GraphLCD_CharPixel(2u, 15u, 5u);                /* Pause symbol.                                        */ 
        } else {
            BSP_GraphLCD_CharPixel(2u, 15u, 3u);                /* Play symbol.                                         */ 
        }
    } else {
        BSP_GraphLCD_CharPixel(2u, 15u, 3u);                    /* Play symbol.                                         */ 
    }
    BSP_GraphLCD_CharPixel(2u, 30u, 2u);                        /* Next symbol.                                         */ 
    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_SMALL);

    Str_FmtNbr_Int32U(song_lst_ix,
                      3u,
                      DEF_NBR_BASE_DEC,
                      ' ',
                      DEF_NO,
                      DEF_YES,
                      songs_msg);
    Str_FmtNbr_Int32U((song_lst_nbr_max > 999) ? 999 : song_lst_nbr_max,
                      3u,
                      DEF_NBR_BASE_DEC,
                      ASCII_CHAR_NUL,
                      DEF_NO,
                      DEF_YES,
                      songs_max_msg);
    Str_Cat(songs_msg, "/");
    Str_Cat(songs_msg, songs_max_msg);

    BSP_GraphLCD_StringPosLen(2u, 19u - Str_Len(songs_msg), songs_msg, 17u);

    BSP_GraphLCD_ClrLine(3u);
    BSP_GraphLCD_StringPosLen(3u, 0u, song_lst_name,  19u);

    p_info = PlayList_GetFileInfo(song_lst_ix, DEF_FALSE);
    if (p_info != (AUDIO_INFO *)0) {
        valid = p_info->Valid;
        if (valid) {
            tot_time = p_info->TimeLen;
            Str_FmtNbr_Int32U((tot_time / 60u) > 99u ? 99u : (tot_time / 60u),
                              2u,
                              DEF_NBR_BASE_DEC,
                              ' ',
                              DEF_NO,
                              DEF_YES,
                              str_tot_time);
            Str_FmtNbr_Int32U((tot_time / 60u) > 99u ? 99u : (tot_time % 60u),
                              2u,
                              DEF_NBR_BASE_DEC,
                              '0',
                              DEF_NO,
                              DEF_YES,
                              str_time_sec);
            Str_Cat(str_tot_time, ":");
            Str_Cat(str_tot_time, str_time_sec);

            Str_FmtNbr_Int32U(p_info->BitsPerSample,
                              2u,
                              DEF_NBR_BASE_DEC,
                              ' ',
                              DEF_NO,
                              DEF_YES,
                              str_bps_ch);
            Str_Cat(str_bps_ch, "b");
            switch (p_info->NbrCh) {
                case 1u:
                     Str_Cat(str_bps_ch, "/1ch");
                     break;
                case 2u:
                     Str_Cat(str_bps_ch, "/2ch");
                     break;
            }

            Str_FmtNbr_Int32U(p_info->SamplesPerSec,
                              5u,
                              DEF_NBR_BASE_DEC,
                              ' ',
                              DEF_NO,
                              DEF_YES,
                              str_samplerate);
    
            BSP_GraphLCD_ClrLine(4u);
            BSP_GraphLCD_StringPos(4u,  0u, str_bps_ch);
            BSP_GraphLCD_StringPos(4u,  8u, str_samplerate);
            BSP_GraphLCD_StringPos(4u, 14u, str_tot_time);
        } else {
            BSP_GraphLCD_ClrLine(4u);
            BSP_GraphLCD_StringPos(4u, 14u, " -:--");
        }
    }

    if (song_play_ix > 0u) {
        BSP_GraphLCD_ClrLine(5u);
        BSP_GraphLCD_StringPosLen(5u, 0u, song_play_name, 19u);
    }
}


/*
*********************************************************************************************************
*                                        AudioUI_DisplayProg()
*
* Description : Display progress of playback audio on user interface.
*
* Argument(s) : pos             Song current position in octets.
*
*               len             Song length in octets.
*
*               samplerate      Song sample rate in cycles per second.
*
*               ch              Song number of channels.
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_DisplayProg (CPU_INT32U  pos,
                                   CPU_INT32U  len,
                                   CPU_INT32U  samplerate,
                                   CPU_INT08U  ch)
{
    CPU_INT32U   per;
    CPU_INT32U   tot_time;
    CPU_INT64U   cur_time;
    CPU_SIZE_T   samples;
    CPU_CHAR     str_time_sec[3];
    CPU_CHAR     str_cur_time[6];
    CPU_CHAR     str_tot_time[6];
    CPU_BOOLEAN  skip;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    skip = AudioUI_DisplaySkip;
    CPU_CRITICAL_EXIT();

    if (skip == DEF_TRUE) {
        return;
    }

    per       =  pos / (len / 100u);
    samples   =  AudioUtil_OctetsToSamples(len);
    samples  /=  ch;
    tot_time  = (samples + samplerate / 2u)/ samplerate;
    cur_time  =  tot_time;
    cur_time *=  pos;
    cur_time /=  len;

    Str_FmtNbr_Int32U((cur_time / 60u) > 99u ? 99u : (cur_time / 60u),
                      2u,
                      DEF_NBR_BASE_DEC,
                      ' ',
                      DEF_NO,
                      DEF_YES,
                      str_cur_time);
    Str_FmtNbr_Int32U((cur_time / 60u) > 99u ? 99u : (cur_time % 60u),
                      2u,
                      DEF_NBR_BASE_DEC,
                      '0',
                      DEF_NO,
                      DEF_YES,
                      str_time_sec);
    Str_Cat(str_cur_time, ":");
    Str_Cat(str_cur_time, str_time_sec);

    Str_FmtNbr_Int32U((tot_time / 60u) > 99u ? 99u : (tot_time / 60u),
                      2u,
                      DEF_NBR_BASE_DEC,
                      ' ',
                      DEF_NO,
                      DEF_YES,
                      str_tot_time);
    Str_FmtNbr_Int32U((tot_time / 60u) > 99u ? 99u : (tot_time % 60u),
                      2u,
                      DEF_NBR_BASE_DEC,
                      '0',
                      DEF_NO,
                      DEF_YES,
                      str_time_sec);
    Str_Cat(str_tot_time, ":");
    Str_Cat(str_tot_time, str_time_sec);

    BSP_GraphLCD_StringPos(6u,  0u, str_cur_time);
    BSP_GraphLCD_StringPos(6u, 14u, str_tot_time);

    BSP_GraphLCD_ProgBarPos(7u, per);
}


/*
*********************************************************************************************************
*                                       AudioUI_RefreshFiles()
*
* Description : Signal user interface to refresh list of files.
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

void  AudioUI_RefreshFiles (void)
{
    OS_ERR  os_err;


    OSTaskQPost(&AudioUI_TCB,
                &AudioUI_MsgRefresh,
                 sizeof(void *),
                 OS_OPT_POST_FIFO,
                &os_err);
}


/*
*********************************************************************************************************
*                                          AudioUI_Update()
*
* Description : Signal user interface to update information.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_Update (void)
{
    OS_ERR  os_err;


    OSTaskQPost(&AudioUI_TCB,
                &AudioUI_MsgUpdate,
                 sizeof(void *),
                 OS_OPT_POST_FIFO,
                &os_err);
}


/*
*********************************************************************************************************
*                                         AudioUI_NextSong()
*
* Description : Signal user interface to play next song.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioUI_NextSong (void)
{
    OS_ERR  os_err;


    OSTaskQPost(&AudioUI_TCB,
                &AudioUI_MsgNextSong,
                 sizeof(void *),
                 OS_OPT_POST_FIFO,
                &os_err);
}


/*
*********************************************************************************************************
*                                          AudioMgr_Action()
*
* Description : Query if any action message has been posted by user interface.
*
* Argument(s) : none.
*
* Return(s)   : DEF_YES, if user interface is requesting playback to stop.
*               DEF_NO,  otherwise.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  AudioMgr_Action (void)
{
    CPU_BOOLEAN     song_end;
    AUDIO_MGR_MSG  *p_msg;
    OS_MSG_SIZE     msg_size;
    CPU_BOOLEAN     valid;
    OS_ERR          os_err;


    song_end = DEF_NO;
                                                                /* Check for Stop/Pause msg.                            */
    p_msg = (AUDIO_MGR_MSG *)OSTaskQPend((OS_TICK      ) 0u,
                                         (OS_OPT       ) OS_OPT_PEND_NON_BLOCKING,
                                         (OS_MSG_SIZE *)&msg_size,
                                         (CPU_TS      *) 0,
                                         (OS_ERR      *)&os_err);
    if (os_err == OS_ERR_NONE) {
        switch (p_msg->ID) {
            case MSG_STOP:
                 Audio_SetPlayback(DEF_FALSE, DEF_FALSE);

                 song_end = DEF_YES;
                 valid    = DEF_YES;
                 break;

            case MSG_PAUSE:
                 Audio_SetPlayback(DEF_FALSE, DEF_TRUE);
                 valid = DEF_YES;
                 break;

            default:
                 valid = DEF_NO;
                 break;
        }

        OSSemPost(&AudioMgr_Sem,                                /* Ack msg to Audio UI.                                 */
                   OS_OPT_POST_1,
                  &os_err);

        if (valid == DEF_NO) {
            return (DEF_NO);
        }

        if (p_msg->ID == MSG_PAUSE) {
                                                                /* Wait for Stop/Play msg.                              */
            p_msg = (AUDIO_MGR_MSG *)OSTaskQPend((OS_TICK      ) 0u,
                                                 (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                                                 (OS_MSG_SIZE *)&msg_size,
                                                 (CPU_TS      *) 0,
                                                 (OS_ERR      *)&os_err);
            if (os_err == OS_ERR_NONE) {
                switch (p_msg->ID) {
                    case MSG_STOP:
                         Audio_SetPlayback(DEF_FALSE, DEF_FALSE);

                         song_end = DEF_YES;
                         valid    = DEF_YES;
                         break;

                    case MSG_START:
                         Audio_SetPlayback(DEF_TRUE, DEF_FALSE);
                         valid = DEF_YES;
                         break;

                    default:
                         valid = DEF_NO;
                         break;
                }

                OSSemPost(&AudioMgr_Sem,                        /* Ack msg to Audio UI.                                 */
                           OS_OPT_POST_1,
                          &os_err);

                if (valid == DEF_NO) {
                    return (DEF_YES);
                }
            }
        }
    }

    return (song_end);
}


/*
*********************************************************************************************************
*                                          AudioMgr_Start()
*
* Description : Submit message to audio manager to start song playback.
*
* Argument(s) : song_ix         Index of song to start playback.
*
* Return(s)   : DEF_OK,   if message submitted successfully.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  AudioMgr_Start (CPU_INT16U  song_ix)
{
    OS_ERR  os_err;


    AudioMgr_MsgStart.ID     = MSG_START;
    AudioMgr_MsgStart.SongIx = song_ix;

    OSTaskQPost(&AudioMgr_TCB,
                &AudioMgr_MsgStart,
                 sizeof(AUDIO_MGR_MSG),
                 OS_OPT_POST_FIFO,
                &os_err);

    if (os_err == OS_ERR_NONE) {
        OSSemPend((OS_SEM *)&AudioMgr_Sem,
                  (OS_TICK ) OSCfg_TickRate_Hz,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *) 0,
                  (OS_ERR *)&os_err);

        return (DEF_OK);
    } else {
        return (DEF_FAIL);
    }
}


/*
*********************************************************************************************************
*                                           AudioMgr_Stop()
*
* Description : Submit message to audio manager to stop song playback.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if message submitted successfully.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  AudioMgr_Stop (void)
{
    OS_ERR  os_err;


    AudioMgr_MsgStop.ID     = MSG_STOP;
    AudioMgr_MsgStop.SongIx = 0;

    OSTaskQPost(&AudioMgr_TCB,
                &AudioMgr_MsgStop,
                 sizeof(AUDIO_MGR_MSG),
                 OS_OPT_POST_FIFO,
                &os_err);

    if (os_err == OS_ERR_NONE) {
        OSSemPend((OS_SEM *)&AudioMgr_Sem,
                  (OS_TICK ) OSCfg_TickRate_Hz,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *) 0,
                  (OS_ERR *)&os_err);

        return (DEF_OK);
    } else {
        return (DEF_FAIL);
    }
}


/*
*********************************************************************************************************
*                                          AudioMgr_Pause()
*
* Description : Submit message to audio manager to pause song playback.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if message submitted successfully.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  AudioMgr_Pause (void)
{
    OS_ERR  os_err;


    AudioMgr_MsgStop.ID     = MSG_PAUSE;
    AudioMgr_MsgStop.SongIx = 0;

    OSTaskQPost(&AudioMgr_TCB,
                &AudioMgr_MsgStop,
                 sizeof(AUDIO_MGR_MSG),
                 OS_OPT_POST_FIFO,
                &os_err);

    if (os_err == OS_ERR_NONE) {
        OSSemPend((OS_SEM *)&AudioMgr_Sem,
                  (OS_TICK ) OSCfg_TickRate_Hz,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *) 0,
                  (OS_ERR *)&os_err);

        return (DEF_OK);
    } else {
        return (DEF_FAIL);
    }
}


/*
*********************************************************************************************************
*                                          Audio_IsPlaying()
*
* Description : Retrieve audio playback state.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE,  if audio is in playback mode.
*               DEF_FALSE, otherwise.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  Audio_IsPlaying (void)
{
    CPU_BOOLEAN  playback;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    playback = Audio_Playback;
    CPU_CRITICAL_EXIT();

    return (playback);
}


/*
*********************************************************************************************************
*                                         Audio_SetPlayback()
*
* Description : Set audio playback mode.
*
* Argument(s) : playback        Playback state.
*
*               paused          Pause state.
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  Audio_SetPlayback (CPU_BOOLEAN  playback,
                                 CPU_BOOLEAN  paused)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    Audio_Playback        = playback;
    AppProbe_SongPlayback = playback;

    Audio_Paused        = paused;
    AppProbe_SongPaused = paused;
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*                                          Audio_IsPaused()
*
* Description : Retrieve audio paused state.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE,  if audio is paused.
*               DEF_FALSE, otherwise.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  Audio_IsPaused (void)
{
    CPU_BOOLEAN  paused;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    paused = Audio_Paused;
    CPU_CRITICAL_EXIT();

    return (paused);
}


/*
*********************************************************************************************************
*                                       PlayList_GetNbrFiles()
*
* Description : Retrieve the number of supported songs.
*
* Argument(s) : none.
*
* Return(s)   : Number of supported songs.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT16U  PlayList_GetNbrFiles (void)
{
    FS_ERR         fs_err;
    FS_DIR        *p_dir;
    FS_DIR_ENTRY   fs_dir_entry;
    CPU_INT16U     fs_dir_entry_cnt;
    CPU_SIZE_T     fs_file_name_len;
    CPU_CHAR       fs_file_ext[4 + 1];
    CPU_CHAR      *p_file_ext_cmp;
    CPU_INT32U     samplerate;


    p_dir = FSDir_Open((CPU_CHAR *)APP_AUDIO_FS_FOLDER,
                       (FS_ERR   *)&fs_err);

    if (fs_err != FS_ERR_NONE) {
        return (0);
    }

                                                                /* Retrieve number of songs on a directory.             */
    fs_dir_entry_cnt = 0u;
    fs_err           = FS_ERR_NONE;
    while (fs_err == FS_ERR_NONE) {

        FSDir_Rd((FS_DIR        *) p_dir,                       /* Read directory entry.                                */
                 (FS_DIR_ENTRY  *)&fs_dir_entry,
                 (FS_ERR        *)&fs_err);

        if (fs_err == FS_ERR_EOF) {
            continue;
        }
                                                                /* Get entry name length.                               */
        fs_file_name_len = Str_Len_N(fs_dir_entry.Name,
                                     APP_SONG_NAME_SIZE_MAX + 1);

                                                                /* Ignore invalid entry name sizes.                     */
        if ((fs_file_name_len > APP_SONG_NAME_SIZE_MAX) ||
            (fs_file_name_len < APP_SONG_NAME_SIZE_MIN)) {
            continue;
        }

        p_file_ext_cmp = FSUtil_GetFileExt(fs_file_ext, fs_dir_entry.Name, 4);
                                                                /* Check if the entry is a "wav" file.                  */
        if (Str_CmpIgnoreCase_N(p_file_ext_cmp, "wav", 4) == 0) {
            fs_dir_entry_cnt++;                                 /* Increment number of songs.                           */                            
        }

                                                                /* Check if the entry is a "dat" file.                  */
        if (Str_CmpIgnoreCase_N(p_file_ext_cmp, "dat", 4) == 0) {
            samplerate = AudioADPCM_GetSamplerate(fs_dir_entry.Name);
            if (samplerate > 0u) {
                fs_dir_entry_cnt++;                             /* Increment number of songs.                           */                            
            }
        }
    }

    if (fs_err != FS_ERR_EOF) {
        fs_dir_entry_cnt = 0u;
    }

    FSDir_Close(p_dir, &fs_err);

    return (fs_dir_entry_cnt);
}


/*
*********************************************************************************************************
*                                       PlayList_GetFileName()
*
* Description : Retrieve the song file name.
*
* Argument(s) : song_ix         Index of song.
*
*               p_song_name     Pointer to destination string to receive song file name.
*
* Return(s)   : DEF_OK,   if song name retrieved successfully.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : AudioUI_Task(),
*               AudioMgr_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  PlayList_GetFileName (CPU_INT16U   song_ix,
                                           CPU_CHAR    *p_song_name)
{
    AUDIO_INFO  *p_info;


    p_info = PlayList_GetFileInfo(song_ix, DEF_FALSE);
    if (p_info == (AUDIO_INFO *)0) {
        p_song_name[0] = ASCII_CHAR_NUL;
        return (DEF_FAIL);
    }

    Str_Copy_N(p_song_name,
               p_info->Name,
               APP_SONG_NAME_SIZE_MAX);

    p_song_name[APP_SONG_NAME_SIZE_MAX] = ASCII_CHAR_NUL;

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                       PlayList_GetFileInfo()
*
* Description : Retrieve the song file information.
*
* Argument(s) : song_ix         Index of song.
*
*               force_refresh   Force cache refresh.
*
* Return(s)   : Pointer to audio information structure, if successful.
*               Pointer to NULL,                        otherwise.
*
* Caller(s)   : AudioUI_Task(),
*               AudioMgr_Task().
*
* Note(s)     : (1) If 'song_ix' is outside cache limits, cache table is updated.
*********************************************************************************************************
*/

static  AUDIO_INFO  *PlayList_GetFileInfo (CPU_INT16U   song_ix,
                                           CPU_BOOLEAN  force_refresh)
{
    FS_DIR        *p_dir;
    FS_DIR_ENTRY   fs_dir_entry;
    CPU_INT16U     fs_dir_entry_cnt;
    CPU_SIZE_T     fs_file_name_len;
    CPU_CHAR       fs_file_ext[4 + 1];
    CPU_CHAR      *p_file_ext_cmp;
    CPU_INT16U     song_cache_ix;
    FS_ERR         fs_err;
    FS_ERR         fs_dir_err;
    CPU_INT16U     mid;
    CPU_BOOLEAN    loop;
    CPU_INT32U     samplerate;


    if ((force_refresh == DEF_TRUE) ||
        (song_ix <   AudioSongInfoCacheStart) ||
        (song_ix >= (AudioSongInfoCacheStart + APP_SONG_CACHE_NBR))) {

        p_dir = FSDir_Open((CPU_CHAR *)APP_AUDIO_FS_FOLDER,
                           (FS_ERR   *)&fs_dir_err);

        if (fs_dir_err != FS_ERR_NONE) {
            LCD_Err("SD Card Read Failed",
                    "FSDir_Open Err");

            return ((AUDIO_INFO *)0);
        }


        mid = (APP_SONG_CACHE_NBR - 1) / 2;
        if (song_ix > mid) {
            AudioSongInfoCacheStart = song_ix - mid;
        } else {
            AudioSongInfoCacheStart = 1;
        }

        loop = DEF_TRUE;
        fs_dir_entry_cnt = 0u;
        do {

            FSDir_Rd(p_dir, &fs_dir_entry, &fs_err);

            if (fs_err != FS_ERR_NONE) {
                continue;
            }

            fs_file_name_len = Str_Len_N(fs_dir_entry.Name,
                                         APP_SONG_NAME_SIZE_MAX + 1);

            if ((fs_file_name_len > APP_SONG_NAME_SIZE_MAX) ||
                (fs_file_name_len < APP_SONG_NAME_SIZE_MIN)) {
                continue;
            }

            p_file_ext_cmp = FSUtil_GetFileExt(fs_file_ext, fs_dir_entry.Name, 4);
                                                                /* Check if the entry is a "wav" file                   */
            if (Str_CmpIgnoreCase_N(p_file_ext_cmp, "wav", 4) == 0) {
                                                                /* Skip entries until song_lst_ix.                      */
                fs_dir_entry_cnt++;
                if (fs_dir_entry_cnt >= AudioSongInfoCacheStart) {
                    song_cache_ix = fs_dir_entry_cnt - AudioSongInfoCacheStart;

                    AudioWave_GetInfo(fs_dir_entry.Name,
                                      &AudioSongInfoCache[song_cache_ix]);

                                                                /* Abort cache if refreshed all entries.                */
                    if (song_cache_ix >= (APP_SONG_CACHE_NBR - 1)) {
                        loop = DEF_FALSE;
                    }
                }
            }
                                                                /* Check if the entry is a "dat" file                   */
            if (Str_CmpIgnoreCase_N(p_file_ext_cmp, "dat", 4) == 0) {
                samplerate = AudioADPCM_GetSamplerate(fs_dir_entry.Name);
                if (samplerate == 0) {
                    continue;
                }
                                                                /* Skip entries until song_lst_ix.                      */
                fs_dir_entry_cnt++;
                if (fs_dir_entry_cnt >= AudioSongInfoCacheStart) {
                    song_cache_ix = fs_dir_entry_cnt - AudioSongInfoCacheStart;

                    AudioADPCM_GetInfo(fs_dir_entry.Name,
                                       &AudioSongInfoCache[song_cache_ix]);

                                                                /* Abort cache if refreshed all entries.                */
                    if (song_cache_ix >= (APP_SONG_CACHE_NBR - 1)) {
                        loop = DEF_FALSE;
                    }
                }
            }

        } while ((fs_err == FS_ERR_NONE) &&
                 (loop   == DEF_TRUE));

        FSDir_Close(p_dir, &fs_dir_err);

        if ((fs_err != FS_ERR_NONE) &&
            (fs_err != FS_ERR_EOF )) {

            LCD_Err("SD Card Read Failed",
                    "FSDir_Rd Err");

            return ((AUDIO_INFO *)0);
        }
    }

    return (&AudioSongInfoCache[song_ix - AudioSongInfoCacheStart]);
}


/*
*********************************************************************************************************
*                                     AudioADPCM_GetSamplerate()
*
* Description : Retrieve sample rate from ADPCM file name.
*
* Argument(s) : p_name          Pointer to song file name.
*
* Return(s)   : Sample rate of ADPCM file, if successful.
*               0                        , otherwise.
*
* Caller(s)   : PlayList_GetFileName(),
*               AudioADPCM_GetInfo().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT32U  AudioADPCM_GetSamplerate (const  CPU_CHAR  *p_name)
{
    CPU_CHAR    *p_samplerate;
    CPU_INT32U   samplerate;


    p_samplerate = Str_Char_Last(p_name, '-');

    if (p_samplerate == (CPU_CHAR *)0) {
        return (0);
    }
    samplerate = Str_ParseNbr_Int32U(            &p_samplerate[1],
                                     (CPU_CHAR **)0,
                                                  DEF_NBR_BASE_DEC);
    switch (samplerate) {
        case  8000u:
        case 11025u:
        case 16000u:
        case 22050u:
        case 24000u:
        case 32000u:
        case 44100u:
        case 48000u:
             return (samplerate);

        default:
             return (0);
    }
}


/*
*********************************************************************************************************
*                                        AudioADPCM_GetInfo()
*
* Description : Retrieve format information from ADPCM song.
*
* Argument(s) : p_name          Index of song.
*
*               p_info          Pointer to audio information structure.
*
* Return(s)   : none.
*
* Caller(s)   : PlayList_GetFileName().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioADPCM_GetInfo (const  CPU_CHAR    *p_name,
                                         AUDIO_INFO  *p_info)
{
                                                                /* Allow space for directory name.                      */
    CPU_CHAR        fullname[APP_SONG_NAME_SIZE_MAX + sizeof(APP_AUDIO_FS_FOLDER) + 1];
    FS_ENTRY_INFO   fs_info;
    FS_FILE        *p_file;
    CPU_INT32U      samplerate;
    FS_ERR          fs_err;


    samplerate = AudioADPCM_GetSamplerate(p_name);

    Str_Copy_N(p_info->Name,
               p_name,
               APP_SONG_NAME_SIZE_MAX);

    p_info->Name[APP_SONG_NAME_SIZE_MAX] = ASCII_CHAR_NUL;

    p_info->Valid          =  DEF_FALSE;
    p_info->NbrCh          =  1u;
    p_info->SamplesPerSec  =  samplerate;
    p_info->AvgBytesPerSec = (samplerate * 4u) / DEF_OCTET_NBR_BITS;
    p_info->BitsPerSample  =  4u;
    p_info->TimeLen        =  0u;

    if (samplerate > 0u) {
        Str_Copy(fullname, APP_AUDIO_FS_FOLDER);
        Str_Cat(fullname, p_info->Name);

        p_file = FSFile_Open(fullname,
                             FS_FILE_ACCESS_MODE_RD |
                             FS_FILE_ACCESS_MODE_CACHED,
                             &fs_err);

        if (fs_err != FS_ERR_NONE) {
            return;
        }

        FSFile_Query(p_file, &fs_info, &fs_err);
        if (fs_err == FS_ERR_NONE) {
            p_info->Valid   =  DEF_TRUE;
            p_info->TimeLen = (fs_info.Size + p_info->AvgBytesPerSec / 2u) / p_info->AvgBytesPerSec;
        }

        FSFile_Close(p_file, &fs_err);
    }
}


/*
*********************************************************************************************************
*                                         AudioWave_GetInfo()
*
* Description : Retrieve format information from wave song.
*
* Argument(s) : p_name          Index of song.
*
*               p_info          Pointer to audio information structure.
*
* Return(s)   : none.
*
* Caller(s)   : PlayList_GetFileName().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AudioWave_GetInfo (const  CPU_CHAR    *p_name,
                                        AUDIO_INFO  *p_info)
{
    CPU_CHAR       fullname[APP_SONG_NAME_SIZE_MAX + 1 + 12];  /* Add space for directory name.                        */
    FS_FILE       *p_file;
    CPU_SIZE_T     octets_rd;
    CPU_SIZE_T     len;
    RIFF_FMT       riff_fmt;
    RIFF_CHUNK     riff_chunk;
    PCM_WAVE_FMT   pcm_wave_fmt;
    FS_ERR         fs_err;
    CPU_BOOLEAN    song_end;


    Str_Copy_N(p_info->Name,
               p_name,
               APP_SONG_NAME_SIZE_MAX);

    p_info->Name[APP_SONG_NAME_SIZE_MAX] = ASCII_CHAR_NUL;

    p_info->Valid          = DEF_FALSE;
    p_info->NbrCh          = 0u;
    p_info->SamplesPerSec  = 0u;
    p_info->AvgBytesPerSec = 0u;
    p_info->BitsPerSample  = 0u;
    p_info->TimeLen        = 0u;

    Str_Copy(fullname, APP_AUDIO_FS_FOLDER);
    Str_Cat(fullname, p_info->Name);

    p_file = FSFile_Open(fullname,
                         FS_FILE_ACCESS_MODE_RD |
                         FS_FILE_ACCESS_MODE_CACHED,
                         &fs_err);

    if (fs_err != FS_ERR_NONE) {
        return;
    }

    octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                          (void     *)&riff_fmt,
                          (CPU_SIZE_T) sizeof(RIFF_FMT),
                          (FS_ERR   *)&fs_err);

    if((fs_err            != FS_ERR_NONE)      ||
       (octets_rd         != sizeof(RIFF_FMT)) ||
       (riff_fmt.Chunk.ID != RIFF_CHUNK_ID)    ||
       (riff_fmt.Fmt      != RIFF_CHUNK_TYPE_WAVE)) {
        FSFile_Close(p_file, &fs_err);
        return;
    }

    song_end = DEF_NO;

    while (song_end == DEF_NO) {

        octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                              (void     *)&riff_chunk,
                              (CPU_SIZE_T) sizeof(RIFF_CHUNK),
                              (FS_ERR   *)&fs_err);

        if ((fs_err    != FS_ERR_NONE) ||
            (octets_rd != sizeof(RIFF_CHUNK))) {
             song_end = DEF_YES;
             continue;
        }

        switch (riff_chunk.ID) {
            case RIFF_WAVE_CHUNK_ID_FMT:
                 Mem_Clr(&pcm_wave_fmt, sizeof(PCM_WAVE_FMT));

                 len = DEF_MIN(riff_chunk.Size, sizeof(PCM_WAVE_FMT));

                 octets_rd = FSFile_Rd((FS_FILE  *) p_file,
                                       (void     *)&pcm_wave_fmt,
                                       (CPU_SIZE_T) len,
                                       (FS_ERR   *)&fs_err);

                 if ((fs_err    != FS_ERR_NONE) ||
                     (octets_rd != len)) {
                      song_end   = DEF_YES;
                      continue;
                 }

                 if (pcm_wave_fmt.FmtTag != RIFF_WAVE_FMT_TAG_PCM) {
                     song_end = DEF_YES;
                     continue;
                 }
                                                                /* Skip additional fmt fields.                          */
                 if (riff_chunk.Size > sizeof(PCM_WAVE_FMT)) {
                     len = riff_chunk.Size - sizeof(PCM_WAVE_FMT);

                     FSFile_PosSet((FS_FILE      *) p_file,
                                   (FS_FILE_OFFSET) len,
                                   (FS_STATE      ) FS_FILE_ORIGIN_CUR,
                                   (FS_ERR       *)&fs_err);

                     if (fs_err   != FS_ERR_NONE) {
                         song_end  = DEF_TRUE;
                         continue;
                     }
                 }

                 switch (pcm_wave_fmt.BitsPerSample) {
                     case 8u:
                     case 16u:
                          break;

                     default:
                          song_end = DEF_YES;
                          continue;
                 }
                 break;


            case RIFF_WAVE_CHUNK_ID_DATA:
                 p_info->Valid          =  DEF_TRUE;
                 p_info->NbrCh          =  pcm_wave_fmt.NbrCh;
                 p_info->SamplesPerSec  =  pcm_wave_fmt.SamplesPerSec;
                 p_info->AvgBytesPerSec =  pcm_wave_fmt.AvgBytesPerSec;
                 p_info->BitsPerSample  =  pcm_wave_fmt.BitsPerSample;
                 p_info->TimeLen        = (riff_chunk.Size + pcm_wave_fmt.AvgBytesPerSec / 2u) / pcm_wave_fmt.AvgBytesPerSec;

                 song_end  = DEF_TRUE;
                 break;


            default:
                 FSFile_PosSet((FS_FILE      *) p_file,
                               (FS_FILE_OFFSET) riff_chunk.Size,
                               (FS_STATE      ) FS_FILE_ORIGIN_CUR,
                               (FS_ERR       *)&fs_err);

                 if (fs_err != FS_ERR_NONE) {
                     song_end = DEF_TRUE;
                     continue;
                 }

                 break;
        }
    }

    FSFile_Close(p_file, &fs_err);
}


/*
*********************************************************************************************************
*                                        FSUtil_GetFileName()
*
* Description : Retrieve file name from full name.
*
* Argument(s) : p_name          Pointer to destination string to receive file name.
*
*               p_fullname      Pointer to source string of file full name (see Note #1).
*
*               len_max         Maximum number of characters to limit file name (see Note #2).
*
* Return(s)   : none.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : (1) Full name is the concatenation of file name, dot, and file extension.
*
*               (2) File name terminates when :
*
*                   (a) Destination/Source string pointer(s) are passed NULL pointers.
*                       (1) NULL pointer returned.
*
*                   (b) Destination/Source string pointer(s) point to NULL.
*                       (1) String buffer(s) overlap with NULL address; NULL pointer returned.
*
*                   (c) Source string's terminating NULL character found.
*                       (1) Entire source string copied into destination string buffer.
*
*                   (d) Last dot '.' character found on source string before 'len_max'.
*
*                   (e) 'len_max' number of characters copied.
*                       (1) 'len_max' number of characters SHALL NOT include the terminating NULL
*                           character.
*                       (2) Null copies allowed (i.e. zero-length copies).
*                           (A) No string copy performed; destination string returned.
*
*               (3) NULL character inserted into 'len_max' index of destination string to guarantee
*                   properly NULL string termination.
*********************************************************************************************************
*/

static  CPU_CHAR  *FSUtil_GetFileName (       CPU_CHAR    *p_name,
                                       const  CPU_CHAR    *p_fullname,
                                              CPU_SIZE_T   len_max)
{
    const  CPU_CHAR    *p_last_dot;
           CPU_CHAR    *p_output;
           CPU_SIZE_T   len_name;
           CPU_SIZE_T   len_ext;
           CPU_SIZE_T   len;


    p_last_dot = Str_Char_Last(p_fullname, '.');
    if (p_last_dot == (void *)0) {
        len_name = len_max;
    } else {
        len_ext  = Str_Len(p_last_dot);
        len      = Str_Len(p_fullname);
        len_name = DEF_MIN(len - len_ext, len_max);
    }

    p_output = Str_Copy_N(p_name, p_fullname, len_name);

    if (p_name != (CPU_CHAR *)0) {
        p_name[len_name] = ASCII_CHAR_NUL;
    }

    return (p_output);
}


/*
*********************************************************************************************************
*                                         FSUtil_GetFileExt()
*
* Description : Retrieve file extension from full name.
*
* Argument(s) : p_ext           Pointer to destination string to receive file extension.
*
*               p_fullname      Pointer to source string of file full name (see Note #1). 
*
*               len_max         Maximum number of characters to limit file name (see Note #2).
*
* Return(s)   : none.
*
* Caller(s)   : AudioMgr_Task(),
*               PlayList_GetNbrFiles(),
*               PlayList_GetFileName().
*
* Note(s)     : (1) Full name is the concatenation of file name, dot, and file extension.
*
*               (2) File extension terminates when :
*
*                   (a) Destination/Source string pointer(s) are passed NULL pointers.
*                       (1) NULL pointer returned.
*
*                   (b) Destination/Source string pointer(s) point to NULL.
*                       (1) String buffer(s) overlap with NULL address; NULL pointer returned.
*
*                   (c) Source string's terminating NULL character found before finding last '.'
*                       character.
*
*                   (d) 'len_max' number of characters copied.
*                       (1) 'len_max' number of characters SHALL NOT include the terminating NULL
*                           character.
*                       (2) Null copies allowed (i.e. zero-length copies).
*                           (A) No string copy performed; destination string returned.
*
*               (3) NULL character inserted into 'len_max' index of destination string to guarantee
*                   properly NULL string termination.
*********************************************************************************************************
*/

static  CPU_CHAR  *FSUtil_GetFileExt (       CPU_CHAR    *p_ext,
                                      const  CPU_CHAR    *p_fullname,
                                             CPU_SIZE_T   len_max)
{
    const  CPU_CHAR    *p_last_dot;
           CPU_CHAR    *p_output;
           CPU_SIZE_T   len_ext;


    p_last_dot = Str_Char_Last(p_fullname, '.');
    if (p_last_dot == (void *)0) {
        p_output = Str_Copy(p_ext, "");
    } else {
        len_ext = Str_Len_N(&p_last_dot[1], len_max);
        len_ext++;

        p_output = Str_Copy_N(p_ext, &p_last_dot[1], len_ext);

        p_ext[len_max] = ASCII_CHAR_NUL;
    }

    return (p_output);
}


/*
*********************************************************************************************************
*                                              LCD_Clr()
*
* Description : Clear graph LCD.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : AppAudio_Init(),
*               AudioUI_Task().
*
* Note(s)     : (1) Two header lines are not cleared.
*********************************************************************************************************
*/

static  void  LCD_Clr (void)
{
    CPU_INT08U  line;


    for (line = 2u; line <= 7u; line++) {
        BSP_GraphLCD_ClrLine(line);
    }
}


/*
*********************************************************************************************************
*                                              LCD_Msg()
*
* Description : Display message on graph LCD.
*
* Argument(s) : msg_line_1      First  line of message.
*
*               msg_line_2      Second line of message.
*
* Return(s)   : none.
*
* Caller(s)   : AudioUI_Task().
*
* Note(s)     : (1) Messages are displayed on lines 3 and 4.
*
*                   (a) Each message line is centered.
*
*               (3) Blank/space line 5 to separate from error messages.
*********************************************************************************************************
*/

static  void  LCD_Msg (const  CPU_CHAR  *msg_line_1,
                       const  CPU_CHAR  *msg_line_2)
{
    CPU_SIZE_T  len;
    CPU_INT08U  col;


    BSP_GraphLCD_ClrLine(2u);
    BSP_GraphLCD_ClrLine(3u);
    BSP_GraphLCD_ClrLine(4u);
    BSP_GraphLCD_ClrLine(5u);

    if (msg_line_1 != (CPU_CHAR *)0) {
        len = Str_Len(msg_line_1);
        if (len > 19u) {
            col =  0u;
        } else {
            col = (19u - len) / 2u;
        }
        BSP_GraphLCD_StringPosLen(3u, col, msg_line_1, 19u);
    }

    if (msg_line_2 != (CPU_CHAR *)0) {
        len = Str_Len(msg_line_2);
        if (len > 19u) {
            col =  0u;
        } else {
            col = (19u - len) / 2u;
        }
        BSP_GraphLCD_StringPosLen(4u, col, msg_line_2, 19u);
    }
}


/*
*********************************************************************************************************
*                                              LCD_Err()
*
* Description : Display error message on graph LCD.
*
* Argument(s) : msg_line_1      First  line of error message.
*
*               msg_line_2      Second line of error message.
*
* Return(s)   : none.
*
* Caller(s)   : AudioUI_Task(),
*               AudioMgr_Task(),
*               AudioPlayback_Wave(),
*               PlayList_GetFileName().
*
* Note(s)     : (1) Messages are displayed on lines 3 and 4.
*
*                   (a) Each message line is centered.
*
*               (3) Blank/space line 5 to separate from error messages.
*********************************************************************************************************
*/

static  void  LCD_Err (const  CPU_CHAR  *msg_line_1,
                       const  CPU_CHAR  *msg_line_2)
{
    CPU_SIZE_T  len;
    CPU_INT08U  col;


    BSP_GraphLCD_ClrLine(6u);
    BSP_GraphLCD_ClrLine(7u);

    if (msg_line_1 != (CPU_CHAR *)0) {
        len = Str_Len(msg_line_1);
        if (len > 19u) {
            col =  0u;
        } else {
            col = (19u - len) / 2u;
        }
        BSP_GraphLCD_StringPosLen(6u, col, msg_line_1, 19u);
    }

    if (msg_line_2 != (CPU_CHAR *)0) {
        len = Str_Len(msg_line_2);
        if (len > 19u) {
            col =  0u;
        } else {
            col = (19u - len) / 2u;
        }
        BSP_GraphLCD_StringPosLen(7u, col, msg_line_2, 19u);
    }
}


/*
*********************************************************************************************************
*                                            ADC_TmrInit()
*
* Description : Initialize MTU0 to trigger ADC conversions.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) MTU0 operate in normal mode.
*********************************************************************************************************
*/

static  void  ADC_TmrInit (void)
{
    CPU_INT32U  freq;
    CPU_INT16U  per;


    MSTP(MTU0) = 0u;                                            /* Start MTU0.                                          */

    MTU0.TCR.BIT.TPSC = 3u;                                     /* 3 = PCLK / 64.                                       */
    MTU0.TCR.BIT.CCLR = 1u;                                     /* 1 = TCNT cleared by TGRA compare match               */

    MTU0.TMDR.BIT.MD = 0u;                                      /* 0 = PWM normal operation                             */

    freq  = BSP_CPU_PerClkFreq();
    freq /= 64u;
    per   = freq / 50u;

    MTU0.TGRA = per - 1u;

    MTU0.TIER.BIT.TTGE = 1u;                                    /* Enable ADC converter start request                   */

    MTUA.TSTR.BIT.CST0 = 1u;                                    /* Enable MTU0.                                         */
}


/*
*********************************************************************************************************
*                                             ADC_Init()
*
* Description : Initialize ADC1 to read POT1.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) AN4 is input from POT1.
*********************************************************************************************************
*/

static  void  ADC_Init (void)
{
    MSTP(S12AD) = 0u;                                           /* Start S12AD 12-bit ADC.                              */

    S12AD.ADCSR.BIT.ADST  = 0u;                                 /* Stop ADC                                             */
    S12AD.ADCSR.BIT.ADCS  = 0u;                                 /* Single-scan mode                                     */
    S12AD.ADCSR.BIT.ADIE  = 0u;                                 /* Disable conversion interrupt                         */
    S12AD.ADCSR.BIT.TRGE  = 1u;                                 /* Trigger by external source                           */
    S12AD.ADCSR.BIT.EXTRG = 0u;                                 /* Trigger by MTU2 or MTU2S                             */

    S12AD.ADANS.BIT.ANS = DEF_BIT_04;                           /* Enable AN4 channel                                   */

    S12AD.ADSTRGR.BIT.ADSTRS = 1u;                              /* Trigger by MTU0 compare match A                      */
    S12AD.ADCER.BIT.ADRFMT   = 1u;                              /* Left-aligned data                                    */

    PORT4.ICR.BIT.B4 = 1u;                                      /* P44 input routed to peripheral                       */
}


/*
*********************************************************************************************************
*                                             ADC_PotRd()
*
* Description : Read ADC value from POT1.
*
* Argument(s) : none.
*
* Return(s)   : Left-aligned potentiometer value.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT16U  ADC_PotRd (void)
{
    return (S12AD.ADDRE);
}


/*
*********************************************************************************************************
*                                           SSM2377_Init()
*
* Description : Initialize SSM2377 audio amplifier.
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

#if (AUDIO_CFG_SSM2377_EN > 0u)
static  void  SSM2377_Init (void)
{
    SSM2377_En(DEF_DISABLED);
    SSM2377_Gain(DEF_NO);

    AppProbe_AudioLowGain = DEF_YES;

    PORTA.DDR.BYTE |= DEF_BIT_01;                               /* PA: Gain     (PA1) set as output.                    */
    PORTA.DDR.BYTE |= DEF_BIT_02;                               /* PA: Shutdown (PA2) set as output.                    */
    
    SSM2377_En(DEF_ENABLED);
}
#endif


/*
*********************************************************************************************************
*                                            SSM2377_En()
*
* Description : Enable SSM2377 audio amplifier.
*
* Argument(s) : en          Enable state.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : 1) Shutdown pin connected to PA2.
*********************************************************************************************************
*/

#if (AUDIO_CFG_SSM2377_EN > 0u)
static  void  SSM2377_En  (CPU_BOOLEAN  en)
{
    if (en == DEF_ENABLED) {
        PORTA.DR.BYTE |=  DEF_BIT_02;
    } else {
        PORTA.DR.BYTE &= ~DEF_BIT_02;
    }
}
#endif


/*
*********************************************************************************************************
*                                           SSM2377_Gain()
*
* Description : Set low or high gain for the SSM2377 audio amplifier.
*
* Argument(s) : high        High gain state.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : 1) Gain pin connected to PA1.
*********************************************************************************************************
*/

#if (AUDIO_CFG_SSM2377_EN > 0u)
static  void  SSM2377_Gain (CPU_BOOLEAN  high)
{
    if (high == DEF_YES) {
        PORTA.DR.BYTE &= ~DEF_BIT_01;                           /* High Gain: 12dB.                                     */
    } else {
        PORTA.DR.BYTE |=  DEF_BIT_01;                           /* Low  Gain:  6dB.                                     */
    }
}
#endif
