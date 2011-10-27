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

static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];
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

        void  AppHTTPs_Init  (void);
#endif

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
        void  AppProbe_Init  (void);
#endif

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
    CPU_INT08U  i;
    CPU_INT08U  j;
    OS_ERR      err;
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    NET_ERR     net_err;
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

                                                                /* Turn on 4 LEDs to show board is alive                */
    LED_On(14);
    LED_On(5);
    LED_On(8);
    LED_On(11);


    BSP_GraphLCD_Init();
    AppGraphLCD_Hdr();

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    AppTCPIP_Init(&net_err);                                    /* Initialize uC/TCP-IP & associated applications.      */
    if (net_err == NET_ERR_NONE) {
        AppHTTPs_Init();
    }
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    AppCloud_Init(0);

    LED_Off(0);                                                 /* Turn all LEDs off                                    */

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        for (i = 7; i <= 12; i++) {
            EX_LED_ON(i);
            OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
            LED_Off(i);

        }

        for (i = 11; i > 7 ; i--) {
            EX_LED_ON(i);
            OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
            LED_Off(i);
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

