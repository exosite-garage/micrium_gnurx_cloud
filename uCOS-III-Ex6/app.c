/*
*********************************************************************************************************
*                                               uC/OS-III
*                                         The Real-Time Kernel
*
*                             (c) Copyright 2011, Micrium, Inc., Weston, FL
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

#include  <includes.h>
#include  <audio.h>
#include  <bsp_ser.h>

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

        OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  CPU_BOOLEAN  AppTCPIP_Cfg = DEF_FALSE;

volatile  CPU_INT08U  AppCloudControlLedOn = 1;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart   (void     *p_arg);

static  void  AppGraphLCD_Hdr(void);

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppTCPIP_Init  (NET_ERR  *perr);
#endif

#if (APP_CFG_AUDIO_EN > 0u)
        void  AppAudio_Init  (void);
#endif

static  void  LEDDisplay_Seq0(void);
static  void  LEDDisplay_Seq1(void);
static  void  LEDDisplay_Seq2(void);
static  void  LEDDisplay_Seq3(void);

void  AppCloud_Init (CPU_BOOLEAN disableStatus);

#define EX_LED_ON(a) {if (AppCloudControlLedOn) LED_On(a); else LED_Off(a);}


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
    CPU_INT08U  seq;
    OS_ERR      err;
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    NET_ERR     net_err;
#endif


   (void)&p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */
    OS_CPU_TickInit();                                          /* Init uC/OS periodic time src (SysTick).              */

    BSP_Ser_Init(BSP_CFG_SER_BAUDRATE);

#if (OS_CFG_STAT_TASK_EN > 0u)
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

    Mem_Init();                                                 /* Initialize mem mgmt module, required for TCP-IP.     */

    BSP_GraphLCD_Init();
    AppGraphLCD_Hdr();

#if (APP_CFG_FS_EN > 0u)
	App_FS_Init();
#endif

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    AppTCPIP_Init(&net_err);                                    /* Initialize uC/TCP-IP & associated applications.      */
    if ((net_err == NET_ERR_NONE) ||
        (net_err == NET_IF_ERR_LINK_DOWN)) {
        AppTCPIP_Cfg = DEF_TRUE;
    }
#if (APP_CFG_TFTPs_MODULE_EN > 0u)
    TFTPs_Init();
#endif
    AppCloud_Init(1);
#else
    AppTCPIP_Cfg = DEF_TRUE;
#endif
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    OSSchedRoundRobinCfg(DEF_ENABLED, 5, &err);
#endif
#if (APP_CFG_AUDIO_EN > 0u)
    AppAudio_Init();
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    seq = 0;
    LED_Off(0);                                                 /* turn all LEDs off                                    */
    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        if (AppTCPIP_Cfg == DEF_FALSE) {

            LED_Toggle(14);                                     /* simple LED flash                                     */
            LED_Toggle( 5);
            LED_Toggle( 8);
            LED_Toggle(11);

            OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        } else {

            switch (seq){
                case 0:
                     LEDDisplay_Seq0();
                     seq++;
                     break;

                case 1:
                     LEDDisplay_Seq1();
                     seq++;
                     break;

                case 2:
                     LEDDisplay_Seq2();
                     seq++;
                     break;

                case 3:
                     LEDDisplay_Seq3();
                     seq = 0;
                     break;

                default:
                     seq = 0;
                     break;
            }
        }
    }
}


static  void  LEDDisplay_Seq0 (void)
{
    CPU_INT08U  i;
    OS_ERR      err;


    LED_Off(0);

    for (i = 14; i > 4; i--) {
        EX_LED_ON(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
        LED_Off(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }

    for (i = 4; i <= 15; i++) {
        EX_LED_ON(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
        LED_Off(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}


static  void  LEDDisplay_Seq1 (void)
{
    CPU_INT08U  i;
    OS_ERR      err;


    LED_Off(0);

    for (i = 4; i <= 15; i++) {
        EX_LED_ON(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }

    for (i = 15; i > 4; i--) {
        LED_Off(i);
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}


static  void  LEDDisplay_Seq2 (void)
{
    CPU_INT08U  i;
    CPU_INT08U  j;
    OS_ERR      err;


    LED_Off(0);

    j = 15;
    for (i = 4; i <= 9; i++) {
        EX_LED_ON(i);
        EX_LED_ON(j);
        j--;
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }

    j = 15;
    for (i = 4; i <= 8; i++) {
        LED_Off(i);
        LED_Off(j);
        j--;
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }

    j = 11;
    for (i = 8; i >= 4; i--) {
        EX_LED_ON(i);
        EX_LED_ON(j);
        j++;
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }

    j = 10;
    for (i=9; i >= 4; i--) {
        LED_Off(i);
        LED_Off(j);
        j++;
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}


static  void  LEDDisplay_Seq3 (void)
{
    CPU_INT08U  i;
    CPU_INT08U  delay;
    OS_ERR      err;


    LED_Off(0);

    for (delay = 100; delay > 20; delay -= 5) {
        for (i = 4; i < 7; i++) {
            EX_LED_ON(i);
            EX_LED_ON(i + 3);
            EX_LED_ON(i + 6);
            EX_LED_ON(i + 9);
            OSTimeDlyHMSM(0u, 0u, 0u, delay,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
            LED_Off(i);
            LED_Off(i + 3);
            LED_Off(i + 6);
            LED_Off(i + 9);
            OSTimeDlyHMSM(0u, 0u, 0u, delay,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
    }

    for (delay = 25; delay <= 100; delay += 5) {
        for (i = 4; i < 7; i++) {
            EX_LED_ON(i);
            EX_LED_ON(i + 3);
            EX_LED_ON(i + 6);
            EX_LED_ON(i + 9);
            OSTimeDlyHMSM(0u, 0u, 0u, delay,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
            LED_Off(i);
            LED_Off(i + 3);
            LED_Off(i + 6);
            LED_Off(i + 9);
            OSTimeDlyHMSM(0u, 0u, 0u, delay,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
    }
}


/*
*********************************************************************************************************
*                                          AppGraphLCD_Hdr()
*********************************************************************************************************
*/

static  void  AppGraphLCD_Hdr (void)
{
    CPU_INT08U  line;


    BSP_GraphLCD_Clear();

    line = 0;
    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_LARGE);
    BSP_GraphLCD_StringPos(line++, 4, "Micrium");

    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_SMALL);
#if (APP_CFG_AUDIO_EN > 0u)
    BSP_GraphLCD_StringPos(line++, 1, "RX62N Audio Player");
#else
    BSP_GraphLCD_StringPos(line++, 5, "uC/OS-III");

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    BSP_GraphLCD_StringPos(line++, 5, "uC/TCP-IP");
#endif
#if (APP_CFG_FS_EN > 0u)
    BSP_GraphLCD_StringPos(line++, 7, "uC/FS");
#endif
#endif
}

