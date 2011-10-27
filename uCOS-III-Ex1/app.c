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


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_TCB       AppBlinkyTaskTCB;
static  CPU_STK      AppBlinkyTaskStk[BLINKY_TASK_STK_SIZE];

static  CPU_BOOLEAN  AppTCPIP_Cfg = DEF_FALSE;

volatile  CPU_INT08U  AppCloudControlLedOn = 1;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart   (void     *p_arg);
static  void  AppBlinkyTask  (void     *p_arg);

static  void  AppGraphLCD_Hdr(void);

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppTCPIP_Init  (NET_ERR  *perr);
#endif

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
        void  AppProbe_Init  (void);
#endif

static  void  LEDDisplaySeq0 (void);
static  void  LEDDisplaySeq1 (void);
static  void  LEDDisplaySeq2 (void);
static  void  LEDDisplaySeq3 (void);

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
    OS_ERR   err;
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    NET_ERR  net_err;
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

    OSTaskCreate((OS_TCB     *)&AppBlinkyTaskTCB,               /* Create the start task                                */
                 (CPU_CHAR   *)"Blinky Task",
                 (OS_TASK_PTR ) AppBlinkyTask,
                 (void       *) 0,
                 (OS_PRIO     ) BLINKY_TASK_PRIO,
                 (CPU_STK    *)&AppBlinkyTaskStk[0],
                 (CPU_STK_SIZE) BLINKY_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) BLINKY_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    BSP_GraphLCD_Init();
    AppGraphLCD_Hdr();

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    AppTCPIP_Init(&net_err);                                    /* Initialize uC/TCP-IP & associated applications.      */
    AppCloud_Init(0);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    AppTCPIP_Cfg = DEF_TRUE;

    OSTaskDel(&AppTaskStartTCB,                                 /* Delete task because its work is complete             */
              &err);

    while (DEF_ON) {
        ;                                                       /* Should not get here!                                 */
    }
}

/*
*********************************************************************************************************
*                                          BLINKY TASK
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

static  void  AppBlinkyTask (void *p_arg)
{
            OS_ERR      err;
    static  CPU_INT08U  seq = 0;


    (void)&p_arg;

    LED_Off(0);                                                 /* turn all LEDs off                                    */
    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        if(AppTCPIP_Cfg == DEF_FALSE){
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
                     LEDDisplaySeq0();
                     seq++;
                     break;

                case 1:
                     LEDDisplaySeq1();
                     seq++;
                     break;

                case 2:
                     LEDDisplaySeq2();
                     seq++;
                     break;


                case 3:
                     LEDDisplaySeq3();
                     seq = 0;
                     break;

                default:
                     seq = 0;
                     break;
            }
        }
    }
}



static  void  LEDDisplaySeq0 (void)
{
    CPU_INT08U  i;
    CPU_INT08U  j;
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



static  void  LEDDisplaySeq1 (void)
{
    CPU_INT08U  i;
    CPU_INT08U  j;
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



static  void  LEDDisplaySeq2 (void)
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



static  void  LEDDisplaySeq3 (void)
{
    CPU_INT08U  i;
    CPU_INT08U  j;
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
    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_LARGE);
    BSP_GraphLCD_StringPos(0, 4, "Micrium");

    BSP_GraphLCD_SetFont(BSP_GLCD_FONT_SMALL);
    BSP_GraphLCD_StringPos(1, 5, "uC/OS-III");
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    BSP_GraphLCD_StringPos(2, 5, "uC/TCP-IP");
#endif
}

