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
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_TEST_MAX               30u

#define  APP_TASK_RX_STK_SIZE       APP_TASK_START_STK_SIZE
#define  APP_TASK_TX_STK_SIZE       APP_TASK_START_STK_SIZE

#define  APP_TASK_RX_PRIO          (APP_TASK_START_PRIO)
#define  APP_TASK_TX_PRIO          (APP_TASK_START_PRIO + 1u)

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef  struct  app_test {
    void  (*Tx)(CPU_INT08U  ix);
    void  (*Rx)(CPU_INT08U  ix);
} APP_TEST;

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static            CPU_BOOLEAN  AppStatReset;

static            OS_TCB       AppTaskStartTCB;
static            CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];

static            OS_TCB       AppTaskRxTCB;
static            CPU_STK      AppTaskRxStk[APP_TASK_RX_STK_SIZE];

static            OS_TCB       AppTaskTxTCB;
static            CPU_STK      AppTaskTxStk[APP_TASK_TX_STK_SIZE];

static            OS_SEM       AppSem;
static            OS_Q         AppQ;
static            OS_FLAG_GRP  AppFlagGrp;
static            OS_MUTEX     AppMutex;

static            CPU_INT08U   AppTestSel;
static  volatile  CPU_INT16U   AppTestTime_us;

static            CPU_FP32     App_TS_to_us;

static            CPU_TS_TMR   AppTS_Delta[APP_TEST_MAX];
static            CPU_TS_TMR   AppTS_Start[APP_TEST_MAX];
static            CPU_TS_TMR   AppTS_End[APP_TEST_MAX];

volatile          CPU_INT08U  AppCloudControlLedOn = 1;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart      (void        *p_arg);

static  void  AppGraphLCD_Hdr   (void);

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppTCPIP_Init     (NET_ERR     *perr);
#endif

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
        void  AppProbe_Init     (void);
#endif

static  void  LEDDisplaySeq0    (void);
static  void  LEDDisplaySeq1    (void);
static  void  LEDDisplaySeq2    (void);
static  void  LEDDisplaySeq3    (void);

static  void  AppTaskRx         (void        *p_arg);
static  void  AppTaskTx         (void        *p_arg);

static  void  AppTaskCreate     (void);
static  void  AppObjCreate      (void);

static  void  AppTestTx_Sem1    (CPU_INT08U   ix);
static  void  AppTestRx_Sem1    (CPU_INT08U   ix);
static  void  AppTestTx_Sem2    (CPU_INT08U   ix);
static  void  AppTestRx_Sem2    (CPU_INT08U   ix);

static  void  AppTestTx_TaskSem1(CPU_INT08U   ix);
static  void  AppTestRx_TaskSem1(CPU_INT08U   ix);
static  void  AppTestTx_TaskSem2(CPU_INT08U   ix);
static  void  AppTestRx_TaskSem2(CPU_INT08U   ix);

static  void  AppTestTx_Q1      (CPU_INT08U   ix);
static  void  AppTestRx_Q1      (CPU_INT08U   ix);
static  void  AppTestTx_Q2      (CPU_INT08U   ix);
static  void  AppTestRx_Q2      (CPU_INT08U   ix);

static  void  AppTestTx_TaskQ1  (CPU_INT08U   ix);
static  void  AppTestRx_TaskQ1  (CPU_INT08U   ix);
static  void  AppTestTx_TaskQ2  (CPU_INT08U   ix);
static  void  AppTestRx_TaskQ2  (CPU_INT08U   ix);

static  void  AppTestTx_Mutex1  (CPU_INT08U   ix);
static  void  AppTestRx_Mutex1  (CPU_INT08U   ix);

static  void  AppTestTx_Flag1   (CPU_INT08U   ix);
static  void  AppTestRx_Flag1   (CPU_INT08U   ix);
static  void  AppTestTx_Flag2   (CPU_INT08U   ix);
static  void  AppTestRx_Flag2   (CPU_INT08U   ix);

void  AppCloud_Init (CPU_BOOLEAN disableStatus);

#define EX_LED_TOGGLE(a) {if (AppCloudControlLedOn) LED_Toggle(a); else LED_Off(a);}

/*
*********************************************************************************************************
*                                             TEST TABLE
*********************************************************************************************************
*/

static  APP_TEST  AppTestTbl[] = {
    {AppTestTx_Sem1,     AppTestRx_Sem1    },         /*  0                                            */
    {AppTestTx_Sem2,     AppTestRx_Sem2    },         /*  1                                            */
    {AppTestTx_TaskSem1, AppTestRx_TaskSem1},         /*  2                                            */
    {AppTestTx_TaskSem2, AppTestRx_TaskSem2},         /*  3                                            */

    {AppTestTx_Q1,       AppTestRx_Q1      },         /*  4                                            */
    {AppTestTx_Q2,       AppTestRx_Q2      },         /*  5                                            */
    {AppTestTx_TaskQ1,   AppTestRx_TaskQ1  },         /*  6                                            */
    {AppTestTx_TaskQ2,   AppTestRx_TaskQ2  },         /*  7                                            */

    {AppTestTx_Mutex1,   AppTestRx_Mutex1  },         /*  8                                            */

    {AppTestTx_Flag1,    AppTestRx_Flag1   },         /*  9                                            */
    {AppTestTx_Flag2,    AppTestRx_Flag2   },         /* 10                                            */

    {0,                  0                 },         /* 11,                                           */
    {0,                  0                 },         /* 12,                                           */
    {0,                  0                 },         /* 13,                                           */
    {0,                  0                 },         /* 14,                                           */
    {0,                  0                 },         /* 15, Int Queue Task execution time             */
    {0,                  0                 },         /* 16, Stat      Task execution time             */
    {0,                  0                 },         /* 17, Tick      Task execution time             */
    {0,                  0                 },         /* 18, Timer     Task execution time             */

    {0,                  0                 },         /* 19, Maximum interrupt disable time            */
    {0,                  0                 },         /* 20, Maximum scheduler lock    time            */

    {0,                  0                 }
};

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

    AppObjCreate();                                             /* Create kernel objects (semaphore, queue, etc.)   */

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
    AppProbe_Init();                                            /* Initialize uC/Probe modules                          */
#endif

    BSP_GraphLCD_Init();
    AppGraphLCD_Hdr();

    AppTaskCreate();                                           /* Create application tasks                         */

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
    AppTCPIP_Init(&net_err);                                    /* Initialize uC/TCP-IP & associated applications.      */
    AppCloud_Init(0);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    OSTaskDel(&AppTaskStartTCB,                                 /* Delete task because its work is complete             */
              &err);

    while (DEF_ON) {
        ;                                                       /* Should not get here!                                 */
    }
}


/*
*********************************************************************************************************
*                                        CREATE APPLICATION TASKS
*
* Description:  This function creates the application tasks.
*
* Arguments  :  none
*
* Returns    :  none
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
    OS_ERR  err;


    OSTaskCreate((OS_TCB     *)&AppTaskRxTCB,
                 (CPU_CHAR   *)"Rx Task",
                 (OS_TASK_PTR ) AppTaskRx,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_RX_PRIO,
                 (CPU_STK    *)&AppTaskRxStk[0],
                 (CPU_STK_SIZE) APP_TASK_RX_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_TASK_RX_STK_SIZE,
                 (OS_MSG_QTY  ) 10u,
                 (OS_TICK     )  0u,
                 (void       *)  0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSTaskCreate((OS_TCB     *)&AppTaskTxTCB,
                 (CPU_CHAR   *)"Tx Task",
                 (OS_TASK_PTR ) AppTaskTx,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_TX_PRIO,
                 (CPU_STK    *)&AppTaskTxStk[0],
                 (CPU_STK_SIZE) APP_TASK_TX_STK_SIZE / 10,
                 (CPU_STK_SIZE) APP_TASK_TX_STK_SIZE,
                 (OS_MSG_QTY  ) 10u,
                 (OS_TICK     )  0u,
                 (void       *)  0u,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}


/*
*********************************************************************************************************
*                                    CREATE APPLICATION KERNEL OBJECTS
*
* Description:  This function creates the application kernel objects such as semaphore, queue, etc.
*
* Arguments  :  none
*
* Returns    :  none
*********************************************************************************************************
*/

static  void  AppObjCreate (void)
{
    OS_ERR  err;



    OSSemCreate  ((OS_SEM      *)&AppSem,
                  (CPU_CHAR    *)"App Sem",
                  (OS_SEM_CTR   ) 0u,
                  (OS_ERR      *)&err);

    OSFlagCreate ((OS_FLAG_GRP *)&AppFlagGrp,
                  (CPU_CHAR    *)"App Flag Group",
                  (OS_FLAGS     ) 0u,
                  (OS_ERR      *)&err);

    OSQCreate    ((OS_Q        *)&AppQ,
                  (CPU_CHAR    *)"App Queue",
                  (OS_MSG_QTY   ) 20u,
                  (OS_ERR      *)&err);

    OSMutexCreate((OS_MUTEX    *)&AppMutex,
                  (CPU_CHAR    *)"App Mutex",
                  (OS_ERR      *)&err);
}

/*
*********************************************************************************************************
*                                                TX TASK
*
* Description : This task sends signals or messages to the Rx Task
*
* Arguments   : p_arg   is the argument passed to 'AppTaskTx()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskTx (void *p_arg)
{
    OS_ERR        err;
    OS_MSG_SIZE   msg_size;
    CPU_TS        ts;
    APP_TEST     *p_test;



   (void)p_arg;

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        EX_LED_TOGGLE(5);
        p_test = (APP_TEST *)OSTaskQPend((OS_TICK      )0,
                                         (OS_OPT       )OS_OPT_PEND_BLOCKING,
                                         (OS_MSG_SIZE *)&msg_size,
                                         (CPU_TS      *)&ts,
                                         (OS_ERR      *)&err);

        (*p_test->Tx)((CPU_INT08U)msg_size);
    }
}


/*
*********************************************************************************************************
*                                                RX TASK
*
* Description : This task receives signals or messages from the Tx Task
*
* Arguments   : p_arg   is the argument passed to 'AppTaskRx()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskRx (void *p_arg)
{
    OS_ERR       err;
    CPU_ERR      cpu_err;
    CPU_INT08U   i;
    APP_TEST    *p_test;
    


   (void)&p_arg;

    i            = 0;
    p_test       = &AppTestTbl[0];
    AppTestSel   = 0;
    App_TS_to_us = (CPU_FP32)1000000.0f / (CPU_FP32)CPU_TS_TmrFreqGet(&cpu_err);

    for (i = 0; i < APP_TEST_MAX; i++) {                        /* Clear the test data array                            */
        AppTS_Delta[i] = (CPU_TS_TMR)0;
    }

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */
        EX_LED_TOGGLE(14);
        OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
        if ((void *)p_test->Tx != (void *)0) {
            OSTaskQPost((OS_TCB    *)&AppTaskTxTCB,
                        (void      *) p_test,
                        (OS_MSG_SIZE) i,
                        (OS_OPT     ) OS_OPT_POST_FIFO,
                        (OS_ERR    *)&err);
            (*(p_test->Rx))(i);
            i++;
            p_test++;
        } else {
            i      = 0;
            p_test = &AppTestTbl[0];
        }
                                                                /* Copy measured time from other sources                */
#if (OS_CFG_ISR_POST_DEFERRED_EN > 0u)
        AppTS_Delta[15] = (CPU_TS_TMR)OSIntQTaskTimeMax;
#endif
#if (OS_CFG_STAT_TASK_EN > 0u)
        AppTS_Delta[16] = (CPU_TS_TMR)OSStatTaskTimeMax;
#endif
        AppTS_Delta[17] = (CPU_TS_TMR)OSTickTaskTimeMax;
#if (OS_CFG_TMR_EN > 0u)
        AppTS_Delta[18] = (CPU_TS_TMR)OSTmrTaskTimeMax;
#endif
        AppTS_Delta[19] = (CPU_TS_TMR)OSIntDisTimeMax;
        AppTS_Delta[20] = (CPU_TS_TMR)OSSchedLockTimeMax;

        AppTestTime_us  = (CPU_INT16U)((CPU_FP32)AppTS_Delta[AppTestSel] * App_TS_to_us);
    }
}


/*
*********************************************************************************************************
*                                         SEMAPHORE TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_Sem1 (CPU_INT08U  ix)
{
    OS_ERR  err;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSSemPost((OS_SEM *)&AppSem,
              (OS_OPT  ) OS_OPT_POST_1,
              (OS_ERR *)&err);
}


void  AppTestRx_Sem1 (CPU_INT08U  ix)
{
    OS_ERR  err;
    CPU_TS  ts;


    OSSemPend((OS_SEM *)&AppSem,
              (OS_TICK ) 0u,
              (OS_OPT  ) OS_OPT_PEND_BLOCKING,
              (CPU_TS *)&ts,
              (OS_ERR *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


void  AppTestTx_Sem2 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_Sem2 (CPU_INT08U  ix)
{
    OS_ERR  err;
    CPU_TS  ts;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSSemPost((OS_SEM *)&AppSem,
              (OS_OPT  ) OS_OPT_POST_1,
              (OS_ERR *)&err);
    OSSemPend((OS_SEM *)&AppSem,
              (OS_TICK ) 0u,
              (OS_OPT  ) OS_OPT_PEND_BLOCKING,
              (CPU_TS *)&ts,
              (OS_ERR *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


/*
*********************************************************************************************************
*                                        TASK SEMAPHORE TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_TaskSem1 (CPU_INT08U  ix)
{
    OS_ERR  err;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSTaskSemPost((OS_TCB *)&AppTaskRxTCB,
                  (OS_OPT  ) OS_OPT_POST_NONE,
                  (OS_ERR *)&err);
}


void  AppTestRx_TaskSem1 (CPU_INT08U  ix)
{
    OS_ERR  err;
    CPU_TS  ts;


    OSTaskSemPend((OS_TICK ) 0u,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *)&ts,
                  (OS_ERR *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


void  AppTestTx_TaskSem2 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_TaskSem2 (CPU_INT08U  ix)
{
    OS_ERR  err;
    CPU_TS  ts;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSTaskSemPost((OS_TCB *) 0u,
                  (OS_OPT  ) OS_OPT_POST_NONE,
                  (OS_ERR *)&err);
    OSTaskSemPend((OS_TICK ) 0u,
                  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                  (CPU_TS *)&ts,
                  (OS_ERR *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


/*
*********************************************************************************************************
*                                        MESSAGE QUEUE TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_Q1 (CPU_INT08U  ix)
{
    OS_ERR  err;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSQPost((OS_Q      *)&AppQ,
            (void      *) 1,
            (OS_MSG_SIZE) 0u,
            (OS_OPT     ) OS_OPT_POST_FIFO,
            (OS_ERR    *)&err);
}


void  AppTestRx_Q1 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;
    OS_MSG_SIZE  msg_size;


    (void)OSQPend((OS_Q        *)&AppQ,
                  (OS_TICK      ) 0u,
                  (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                  (OS_MSG_SIZE *)&msg_size,
                  (CPU_TS      *)&ts,
                  (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


void  AppTestTx_Q2 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_Q2 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;
    OS_MSG_SIZE  msg_size;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSQPost((OS_Q      *)&AppQ,
            (void      *) 1,
            (OS_MSG_SIZE) 0u,
            (OS_OPT     ) OS_OPT_POST_FIFO,
            (OS_ERR    *)&err);
    (void)OSQPend((OS_Q        *)&AppQ,
                  (OS_TICK      ) 0u,
                  (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                  (OS_MSG_SIZE *)&msg_size,
                  (CPU_TS      *)&ts,
                  (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


/*
*********************************************************************************************************
*                                      TASK MESSAGE QUEUE TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_TaskQ1 (CPU_INT08U  ix)
{
    OS_ERR  err;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSTaskQPost((OS_TCB    *)&AppTaskRxTCB,
                (void      *) 1,
                (OS_MSG_SIZE) 0u,
                (OS_OPT     ) OS_OPT_POST_FIFO,
                (OS_ERR    *)&err);
}


void  AppTestRx_TaskQ1 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;
    OS_MSG_SIZE  msg_size;


    (void)OSTaskQPend((OS_TICK      ) 0u,
                      (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                      (OS_MSG_SIZE *)&msg_size,
                      (CPU_TS      *)&ts,
                      (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


void  AppTestTx_TaskQ2 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_TaskQ2 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;
    OS_MSG_SIZE  msg_size;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSTaskQPost((OS_TCB    *) 0,
                (void      *) 1,
                (OS_MSG_SIZE) 0u,
                (OS_OPT     ) OS_OPT_POST_FIFO,
                (OS_ERR    *)&err);
    (void)OSTaskQPend((OS_TICK      ) 0u,
                      (OS_OPT       ) OS_OPT_PEND_BLOCKING,
                      (OS_MSG_SIZE *)&msg_size,
                      (CPU_TS      *)&ts,
                      (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


/*
*********************************************************************************************************
*                                   MUTUAL EXCLUSION SEMAPHORE TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_Mutex1 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_Mutex1 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSMutexPend((OS_MUTEX *)&AppMutex,
                (OS_TICK   ) 0u,
                (OS_OPT    ) OS_OPT_PEND_BLOCKING,
                (CPU_TS   *)&ts,
                (OS_ERR   *)&err);
    OSMutexPost((OS_MUTEX *)&AppMutex,
                (OS_OPT    ) OS_OPT_POST_NONE,
                (OS_ERR   *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


/*
*********************************************************************************************************
*                                          EVENT FLAG TEST(S)
*********************************************************************************************************
*/

void  AppTestTx_Flag1 (CPU_INT08U  ix)
{
    OS_ERR  err;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSFlagPost((OS_FLAG_GRP *)&AppFlagGrp, 
               (OS_FLAGS     ) 0xFFu, 
               (OS_OPT       ) OS_OPT_POST_FLAG_SET,
               (OS_ERR      *)&err);
}


void  AppTestRx_Flag1 (CPU_INT08U  ix)
{
    OS_ERR       err;
    CPU_TS       ts;


    OSFlagPend((OS_FLAG_GRP *)&AppFlagGrp,
               (OS_FLAGS     ) 0xFFu,
               (OS_TICK      )    0u,
               (OS_OPT       )(OS_OPT_PEND_FLAG_SET_ALL +
                               OS_OPT_PEND_FLAG_CONSUME +
                               OS_OPT_PEND_BLOCKING),
               (CPU_TS      *)&ts,
               (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
}


void  AppTestTx_Flag2 (CPU_INT08U  ix)
{
    ix = ix;
}


void  AppTestRx_Flag2 (CPU_INT08U  ix)
{
    OS_ERR  err;
    CPU_TS  ts;


    AppTS_Start[ix] = CPU_TS_TmrRd();
    OSFlagPost((OS_FLAG_GRP *)&AppFlagGrp,
               (OS_FLAGS     ) 0xFFu,
               (OS_OPT       ) OS_OPT_POST_FLAG_SET,
               (OS_ERR      *)&err);
    OSFlagPend((OS_FLAG_GRP *)&AppFlagGrp,
               (OS_FLAGS     ) 0xFFu,
               (OS_TICK      )    0u,
               (OS_OPT       )(OS_OPT_PEND_FLAG_SET_ALL +
                               OS_OPT_PEND_FLAG_CONSUME +
                               OS_OPT_PEND_BLOCKING),
               (CPU_TS      *)&ts,
               (OS_ERR      *)&err);
    AppTS_End[ix]   = CPU_TS_TmrRd();
    AppTS_Delta[ix] = AppTS_End[ix] - AppTS_Start[ix];
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

