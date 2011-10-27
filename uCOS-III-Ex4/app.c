/*
*********************************************************************************************************
*                                               uC/OS-III
*                                         The Real-Time Kernel
*
*                             (c) Copyright 2010, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*
* File : APP.C
* By   : FGK
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>

#include "stdlib.h"
#include "iodefine.h"
#include "inthandler.h"
#include "rtadsplib.h"                                          /* API header file for SP library                       */


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_PWM_CARRIER_FREQ_HZ       20000L
#define  APP_FFT_N_POINTS               1024
#define  APP_DEADTIME                      1u                   /*  When no deadtime is required, set DEADTIME to 1     */

#define  PI                      (CPU_FP32)3.141592654f


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_TCB       AppFFTTaskTCB;
static  CPU_STK      AppFFTTaskStk[APP_FFT_TASK_STK_SIZE];

static  CPU_BOOLEAN  AppTCPIP_Cfg = DEF_FALSE;

static  CPU_FP32     AppPWM_ISR_Time_us;                        /* Execution time of PWM ISR                            */
                                                                /* FFT input data must start on an 8-byte boundary      */
static  CPU_FP32     AppADC_Array[APP_FFT_N_POINTS * 3] __attribute__ ((aligned (8)));
static  CPU_INT16U   AppADC_ArrayRMS[APP_FFT_N_POINTS / 4];

static  CPU_FP32     AppFFT_FFT_Time_us;                        /* Execution time of FFT                                */
static  CPU_FP32     AppFFT_Freq_Time_us;                       /* Execution time of actual frequency detection         */
static  CPU_FP32     AppFFT_RMS_Time_us;                        /* Execution time of RMS computation                    */

static  CPU_FP32     AppTmrTS_To_us;                            /* CPU_TS to 'us' conversion factor                     */


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart     (void     *p_arg);
static  void  AppFFTTask       (void     *p_arg);
static  void  AppPWM_Init      (void);
static  void  AppADC_Init      (void);
static  void  AppADC_TmrInit   (void);
static  void  AppPWM_SetpointRd(void);

static  void  AppGraphLCD_Hdr  (void);

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppTCPIP_Init    (NET_ERR  *perr);
        void  AppCloud_Init    (CPU_BOOLEAN disableStatus);
#endif

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
        void  AppProbe_Init    (void);
#endif

#define EX_LED_ON(a) {if (AppCloudControlLedOn) LED_On(a); else LED_Off(a);}


/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

volatile  CPU_INT08U  AppFreqSetpointSel     =  0;              /* Variable which controls whether the target sine ...  */
                                                                /* ...  wave frequency to generate comes from the ...   */
                                                                /* ...  potentiometer or uC/Probe:                      */
                                                                /* ...     (0 = Pot, 1 = uC/Probe)                      */

volatile  CPU_INT16U  AppFreqActualHz        =  50;
volatile  CPU_FP32    AppFreqSetpoint        =   3.6f;
volatile  CPU_INT16U  AppFreqSetpointHz      = 100;
volatile  CPU_INT16U  AppFreqSetpointPotHz   = 100;
volatile  CPU_INT16U  AppFreqSetpointProbeHz = 100;

volatile  CPU_INT08U  AppCloudControlLedOn   =   1;

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  err;


    CPU_IntDis();                                               /* Disable all interrupts.                              */

    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    App_OS_SetAllHooks();

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR ) AppTaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */

    while (1) {
        ;
    }

    return (0);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_CHAR  str_lcd[20];
    OS_ERR    err;
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    NET_ERR   net_err;
#endif


   (void)&p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */
    OS_CPU_TickInit();                                          /* Init uC/OS periodic time src (SysTick).              */

#if (OS_CFG_STAT_TASK_EN > 0u)
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

    Mem_Init();                                                 /* Initialize mem mgmt module, required for TCP-IP.     */

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
    AppProbe_Init();                                            /* Initialize uC/Probe modules                          */
#endif

    OSTaskCreate((OS_TCB     *)&AppFFTTaskTCB,                  /* Create the start task                                */
                 (CPU_CHAR   *)"FFT Task",
                 (OS_TASK_PTR ) AppFFTTask,
                 (void       *) 0,
                 (OS_PRIO     ) APP_FFT_TASK_PRIO,
                 (CPU_STK    *)&AppFFTTaskStk[0],
                 (CPU_STK_SIZE) APP_FFT_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_FFT_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    BSP_GraphLCD_Init();
    AppGraphLCD_Hdr();

    if (CPU_TS_TmrFreq_Hz > 0) {
        AppTmrTS_To_us = (CPU_FP32)1000000.0f / (CPU_FP32)CPU_TS_TmrFreq_Hz;
    } else {
        AppTmrTS_To_us = (CPU_FP32)0.0f;
    }


#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    AppTCPIP_Init(&net_err);                                    /* Initialize uC/TCP-IP & associated applications.      */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    if (net_err == NET_ERR_NONE) {
        AppTCPIP_Cfg = DEF_TRUE;
    }
#else
    AppTCPIP_Cfg = DEF_TRUE;
#endif

    AppCloud_Init(0);

                                                                /* Display TCP/IP data for several seconds              */
    OSTimeDlyHMSM(0u, 0u, 10u, 0u,
                  OS_OPT_TIME_HMSM_STRICT,
                  &err);

    BSP_GraphLCD_Clear();
    AppGraphLCD_Hdr();

    while (DEF_ON) {
        BSP_GraphLCD_ClrLine(5);
        BSP_GraphLCD_ClrLine(6);
        BSP_GraphLCD_ClrLine(7);
        
        if (AppFreqSetpointSel == 0){
            BSP_GraphLCD_String(5, "SRC: Pot");
        } else {
            BSP_GraphLCD_String(5, "SRC: uC/Probe");
        }

        Str_Copy(str_lcd, "Desired Freq: ");
        Str_FmtNbr_Int32U(AppFreqSetpointHz,
                          3u,
                          DEF_NBR_BASE_DEC,
                          ASCII_CHAR_NULL,
                          DEF_NO,
                          DEF_YES,
                          &str_lcd[14]);
        Str_Cat(str_lcd, "Hz");
        BSP_GraphLCD_String(6, str_lcd);

        Str_Copy(str_lcd, "Actual  Freq: ");
        Str_FmtNbr_Int32U(AppFreqActualHz,
                          3u,
                          DEF_NBR_BASE_DEC,
                          ASCII_CHAR_NULL,
                          DEF_NO,
                          DEF_YES,
                          &str_lcd[14]);
        Str_Cat(str_lcd, "Hz");
        BSP_GraphLCD_String(7, str_lcd);

        OSTimeDlyHMSM(0u, 0u, 0u, 250u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

/*
*********************************************************************************************************
*                                             AppTaskFFT()
*
* Description : This task waits for the ADC1 ISR to post its task semaphore signalling that
*               ADC data has been collected which is subject to FFT analysis
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) A task semaphore is used to signal the event
*********************************************************************************************************
*/

static  void  AppFFTTask (void *p_arg)
{
              OS_ERR              err;
              CPU_TS              ts;
              CPU_INT16U          idx;
    volatile  CPU_INT16U          max_idx;
    volatile  CPU_INT16U          max;
              CPU_INT32U          t1;
              CPU_INT32U          t2;
              CPU_INT32U          t3;
              CPU_TS              ts_start;
              CPU_FP32           *fft_twiddlefac;
              RTA_DSPLIB_STATUS   status;
    CPU_SR_ALLOC();


    (void)&p_arg;
                                                                /* Buf req'd to store sin/cos val of twiddle factor     */
    fft_twiddlefac = (CPU_FP32 *)(AppADC_Array + APP_FFT_N_POINTS);

    Mem_Clr(AppADC_Array, sizeof(AppADC_Array));

    rta_initfft(APP_FFT_N_POINTS, fft_twiddlefac);              /* Initialize the FFT library                           */

    AppPWM_Init();

    AppADC_Init();

    AppPWM_SetpointRd();                                        /* Read desired sine wave frequency (i.e. setpoint)     */

    AppADC_TmrInit();                                           /* Initialize ADC trigger timer                         */

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        OSTaskSemPend(0u,                                       /* Wait for ADC conversion (and array is full)          */
                      OS_OPT_PEND_BLOCKING,
                      &ts,
                      &err);

        ts_start = CPU_TS_Get32();
                                                                /* Complex  fwd FFT                                     */
                                                                /* Computes fwd transform of a complex data sequence    */
                                                                /* Complex  fwd FFT performance measure start point     */
        status   = rta_cfft1d((CPU_FP32 *)&AppADC_Array[0],
                                          -1,
                                          APP_FFT_N_POINTS / 2u,
                                          fft_twiddlefac);

        AppFFT_FFT_Time_us = (CPU_FP32)(CPU_TS_Get32() - ts_start) * AppTmrTS_To_us;


                                                                /* From APP_FFT_N_POINTS / 2 number of samples   ...    */
                                                                /* ... FFT func will output APP_FFT_N_POINTS / 2 ...    */
                                                                /* ... real/complex data pairs.                         */
                                                                /* Performing a root mean square on these values ...    */
                                                                /* ... gives the magnitudes we require.                 */
        ts_start = CPU_TS_Get32();
        for (idx = 0; idx < APP_FFT_N_POINTS / 2; idx += 2) {
            t1  = abs(AppADC_Array[idx]);
            t1 /= 2;
            t1 *= t1;
            t1 /= 2;
            t2  = abs(AppADC_Array[idx + 1]);
            t2 /= 2;
            t2 *= t2;
            t2 /= 2;
            t3  = sqrt(t1 + t2);

            AppADC_ArrayRMS[idx / 2] = t3;
        }

        AppFFT_RMS_Time_us = (CPU_FP32)(CPU_TS_Get32() - ts_start);

        ts_start = CPU_TS_Get32();
        max_idx  = 0;
        max      = 0;
                                                                /* Scan array for maximum frequency                     */
                                                                /* Each array element corresponds to 1Hz                */
                                                                /* As input frequency range is 50Hz to 200Hz,           */
                                                                /* only frequencies in this range need be searched for. */
        for (idx = 1; idx < ((APP_FFT_N_POINTS / 2) / 2); idx++){
            if (AppADC_ArrayRMS[idx] > max){
                max     = AppADC_ArrayRMS[idx];
                max_idx = idx;
            }
        }

        AppFreqActualHz     = max_idx;                          /* Each array element corresponds to 1 Hz               */
        AppFFT_Freq_Time_us = (CPU_FP32)(CPU_TS_Get32() - ts_start) * AppTmrTS_To_us;

        AppPWM_SetpointRd();                                    /* Read desired sine wave frequency (i.e. setpoint)     */

        MTUA.TSTR.BIT.CST0  = 1;                                /* Restart ADC timer trigger                            */
    }
}

/*
*********************************************************************************************************
*                                             AppPWM_Init()
*
* Description : Function to initialise MTU channel 4 to operate in Complimentary PWM mode.
*               PWM signal is output on MTIOC4B. Initial duty set to 0%.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : 1) PWM base frequency set by MTU2 channel 3 TGRA register
*             : 2) PWM duty controlled by MTU2 channel 4 TGRB register
*
*               |
*        TGRA_3 |_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
*               |      /\            /\
*               |     /  \          /  \
*               |    /    \        /    \
*        TGRB_4 |_ _/_ _ _ \_ _ _ /_ _ _ \
*               |  /|      |\    /|      |\
*               | / |      | \  / |      | \  /
*               |/__|______|__\/__|______|__\/_
*                   |      |      |      |
*                   |______|      |______|     
*       MTIOC4B ____|      |______|      |____
*                                          
*********************************************************************************************************
*/

static  void  AppPWM_Init (void)
{
    MSTP_MTU3               = 0;                                /* Enable MTU3 - disable module stop mode               */
    MSTP_MTU4               = 0;                                /* Enable MTU4 - disable module stop mode               */

    IOPORT.PFCMTU.BIT.MTUS5 = 1;                                /* Use P54 as MTIOC4B-B pin                             */


    MTUA.TSTR.BIT.CST3      = 0;                                /* Stop MTU3                                            */
    MTUA.TSTR.BIT.CST4      = 0;                                /* Stop MTU3                                            */

    MTU3.TCR.BIT.CKEG       = 0;                                /* MTU3 count at rising edge of clock                   */
    MTU4.TCR.BIT.CKEG       = 0;                                /* MTU4 count at rising edge of clock                   */
    MTU3.TCR.BIT.CCLR       = 0;                                /* MTU3 counter clearing disabled                       */
    MTU4.TCR.BIT.CCLR       = 0;                                /* MTU4 counter clearing disabled                       */
    MTU3.TCR.BIT.TPSC       = 0;                                /* MTU3 internal clock counts on PCLK                   */
    MTU4.TCR.BIT.TPSC       = 0;                                /* MTU4 internal clock counts on PCLK                   */

    MTU3.TCNT               = APP_DEADTIME;                     /* Deadtime                                             */
    MTUA.TDDR               = APP_DEADTIME;
    MTU4.TCNT               = 0;

    MTUA.TDER.BIT.TDRE      = 0;                                /* No deadtime generated                                */

                                                                /* Set carrier frequency                                */
    MTUA.TCBR               =  ((BSP_CPU_EXT_CLK_FREQ * 4) / APP_PWM_CARRIER_FREQ_HZ) / 2;
    MTUA.TCDR               =  ((BSP_CPU_EXT_CLK_FREQ * 4) / APP_PWM_CARRIER_FREQ_HZ) / 2;
    MTU3.TGRA               = (((BSP_CPU_EXT_CLK_FREQ * 4) / APP_PWM_CARRIER_FREQ_HZ) / 2) + APP_DEADTIME;
    MTU3.TGRC               = (((BSP_CPU_EXT_CLK_FREQ * 4) / APP_PWM_CARRIER_FREQ_HZ) / 2) + APP_DEADTIME;


    MTUA.TOCR1.BIT.OLSP     = 1;                                /* Initial output - low, high on up count compare match */
    MTUA.TOCR1.BIT.OLSN     = 0;                                /* Initial output - high, low on up count compare match */
    MTUA.TOCR2.BIT.OLS3P    = 1;                                /* Initial output - low, high on up count compare match */

    MTU4.TIORH.BIT.IOB      = 7;

    MTU3.TMDR.BIT.MD        = 0x0D;                             /* Complimentary PWM mode 1 (transfer at crest)         */
    MTU3.TMDR.BIT.BFB       = 1;                                /* TGRD buffers TGRB                                    */

    MTUA.TOER.BIT.OE4B      = 1;                                /* Enable MTIOC4B as MTU output                         */

    IPR(MTU3, TGIA3)        = 3;                                /* Set interrupt priority.                              */
    IEN(MTU3, TGIA3)        = 1;                                /* Enable interrupt source.                             */
    MTU3.TIER.BIT.TGIEA     = 1;                                /* Enable MTU3 TGIA interrupt                           */

    MTUA.TSTR.BYTE          = MTUA.TSTR.BYTE | 0xC0;            /* Synchronously start channels 3 and 4                 */
}

/*
*********************************************************************************************************
*                                             AppPWM_SetpointRd()
*
* Description : Determine the setpoint
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/
static  void  AppPWM_SetpointRd (void)
{
    CPU_INT16U  temp;



    switch (AppFreqSetpointSel) {
        case 1:                                                 /* Setpoint using uC/Probe ---------------------------- */
             if (AppFreqSetpointProbeHz < 50) {                 /* Limit setpoint to between 50 and 200 Hz              */
                 AppFreqSetpoint   =  50;
             } else if (AppFreqSetpointProbeHz > 200){
                 AppFreqSetpoint   = 200;
             } else {
                 AppFreqSetpointHz = (CPU_INT16U)AppFreqSetpointProbeHz;
                 AppFreqSetpoint   = AppFreqSetpointProbeHz / 55.5556f;
             }
             break;

        case 0:
        default:                                                /* Setpoint using Potentiometer ----------------------- */
             temp                  = 4095 - (S12AD.ADDRE);      /* Read POT ADC value and scale                         */
             temp                  = temp / 14;
             temp                  = temp + 100;
             AppFreqSetpoint       = 360.0f / (CPU_FP32)temp;
             AppFreqSetpointPotHz  = (CPU_INT16U)(AppFreqSetpoint * 55.5556f);
             AppFreqSetpointHz     = (CPU_INT16U)AppFreqSetpointPotHz;
             break;
    }
}


/*
*********************************************************************************************************
*                                             AppPWM_ISR()
*
* Description : This function corresponds to the ISR of MTU, channel 3 TGIA.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : Reads ADC value or uC/Probe updated variable, calculates next compare match B value and 
*               loads register.
*********************************************************************************************************
*/

void  AppPWM_ISR (void)
{
            CPU_INT16U  temp;
    static  CPU_FP32    sine_idx = 360.0f;
    static  CPU_FP32    step;
            CPU_TS_TMR  ts_start;


    ts_start = CPU_TS_TmrRd();

    if (sine_idx >= 360.0f){
        sine_idx  =   0.0f;                                     /* 'AppFreqSetpoint' is global                          */
        step      = (CPU_FP32)AppFreqSetpoint;                  /* 100 to 400 samples equates to 50 to 200 Hz sinewave  */
    }
    temp       = (CPU_INT16U)((BSP_CPU_EXT_CLK_FREQ * 4) / (2 * APP_PWM_CARRIER_FREQ_HZ) * sin(sine_idx * (PI / 180.0f)));
    temp      += (BSP_CPU_EXT_CLK_FREQ * 4) / (2 * APP_PWM_CARRIER_FREQ_HZ);
    sine_idx  += step;

    MTU4.TGRD  = temp / 2;                                      /* Divide by 2 as max cnt is half PWM carrier cycle     */

    AppPWM_ISR_Time_us = (CPU_FP32)(CPU_TS_TmrRd() - ts_start) * AppTmrTS_To_us;
}


/*
*********************************************************************************************************
*                                           AppADC_TmrInit()
*
* Description : Function to initialise MTU channel 0 to operate in normal mode. This timer triggers ADC 
*               conversions.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppADC_TmrInit (void)
{
    MSTP_MTU0          = 0;                                     /* Enable MTU0 - disable module stop mode               */

    MTU0.TCR.BIT.TPSC  = 3;                                     /* Internal Pclk/64   clock source                      */
    MTU0.TCR.BIT.CCLR  = 1;                                     /* Counter cleared on TGRA compare match                */

    MTU0.TMDR.BIT.MD   = 0;                                     /* Normal mode                                          */

                                                                /* Compare match value - 512 Hz                         */
    MTU0.TGRA          = (CPU_INT16U)((BSP_CPU_EXT_CLK_FREQ * 4) / (64 * 512));

    MTU0.TIER.BIT.TTGE = 1;                                     /* ADC converter start request                          */

    MTUA.TSTR.BIT.CST0 = 1;                                     /* Start timer                                          */
}

/*
*********************************************************************************************************
*                                             AppADC_Init()
*
* Description : Function to initialise ADC 1.
*               Conversion triggered by MTU2
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : AN4 - Input from POT1
*             : AN7 - Input from MTU2 generated sine wave
*
*             : Conversion end interrupt used to transfer AN7 ADC values to buffer for
*             : FFT analysis
*********************************************************************************************************
*/
static  void  AppADC_Init (void)
{
    MSTP_S12AD               = 0;                               /* Enable S12AD 12-bit ADC - disable module stop mode   */

    S12AD.ADCSR.BIT.ADST     = 0;                               /* Stop ADC                                             */
    S12AD.ADCSR.BIT.ADCS     = 0;                               /* Single-scan mode                                     */
    S12AD.ADCSR.BIT.ADIE     = 1;                               /* Conversion interrupt enabled                         */
    S12AD.ADCSR.BIT.TRGE     = 1;                               /* Trigger by external source                           */
    S12AD.ADCSR.BIT.EXTRG    = 0;                               /* Trigger by MTU2 or MTU2S                             */

    S12AD.ADANS.BIT.ANS      = (1 << 4) | (1 << 7);             /* Enable AN4 & AN7 channels                            */

    IPR(S12AD, ADI)          = 3;                               /* Set interrupt priority.                              */
    IEN(S12AD, ADI)          = 1;                               /* Enable interrupt source.                             */

    S12AD.ADSTRGR.BIT.ADSTRS = 1;                               /* Trigger by MTU0 compare match A                      */

    PORT4.ICR.BIT.B4         = 1;                               /* P44 input routed to peripheral                       */
    PORT4.ICR.BIT.B7         = 1;                               /* P47 input routed to peripheral                       */
}

/*
*********************************************************************************************************
*                                         AppADC_ISR_ConversionHandler()
*
* Description : ADC ADI ISR.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : 1) Copies AN7 value into array and signals FFT task when specified number of transfers
*             :    have occured.
*             : 2) This periodic interrupt is used to "rotate" the LED motor display at a speed
*                  controlled by the measure frequency of the sine wave.
*             : 3) This interrupt occurs at 512 Hz
*                                          
*********************************************************************************************************
*/

void  AppADC_ISR_ConversionHandler (void)
{
    static  CPU_INT16U  adc_array_idx = 0;
    static  CPU_INT16U  led_dly       = 0;
    static  CPU_INT08U  led_idx       = 4;
            OS_ERR      err;


    AppADC_Array[adc_array_idx++] = (CPU_FP32)S12AD.ADDRH;
    AppADC_Array[adc_array_idx++] = (CPU_FP32)0.0f;

    if (adc_array_idx == APP_FFT_N_POINTS) {
        adc_array_idx      = 0;
        MTUA.TSTR.BIT.CST0 = 0;                                 /* Stop ADC trigger timer                               */
        OSTaskSemPost(&AppFFTTaskTCB,                           /* Signal task for FFT analysis                         */
                       OS_OPT_POST_NONE,
                      &err);
    }

    if (led_dly == 0) {                                         /* Turn on appropriate LED                              */
        EX_LED_ON(led_idx);
        EX_LED_ON(led_idx + 6);
        led_idx++;
        if (led_idx == 10){
            led_idx  = 4;
        }
        LED_Off(led_idx);
        LED_Off(led_idx + 6);
        led_dly = 5000 / AppFreqActualHz;
    }
    led_dly--;
}


/*
*********************************************************************************************************
*                                          AppGraphLCD_Hdr()
*********************************************************************************************************
*/

static  void  AppGraphLCD_Hdr (void)
{
    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_LARGE);
    BSP_GraphLCD_StringPos(0, 4, "Micrium");

    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_SMALL);
    BSP_GraphLCD_StringPos(1, 5, "uC/OS-III");
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    BSP_GraphLCD_StringPos(2, 5, "uC/TCP-IP");
#endif
}

