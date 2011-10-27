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

#define APP_ACCEL_HYST            10

#define APP_ACCEL_NEG(val,ref)      (val < (ref - APP_ACCEL_HYST))
#define APP_ACCEL_POS(val,ref)      (val > (ref + APP_ACCEL_HYST))
#define APP_ACCEL_ZERO(val,ref)    ((val > (ref - APP_ACCEL_HYST)) && \
                                    (val < (ref + APP_ACCEL_HYST)))

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;
static  OS_TCB       AppAccelTaskTCB;
static  OS_TCB       AppTempTaskTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];
static  CPU_STK      AppAccelTaskStk[ACCEL_TASK_STK_SIZE];
static  CPU_STK      AppTempTaskStk[TEMPERATURE_TASK_STK_SIZE];

static  CPU_BOOLEAN  AppTCPIP_Cfg = DEF_FALSE;

volatile  CPU_INT08U  AppCloudControlLedOn = 1;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart   (void     *p_arg);
static  void  AppAccelTask   (void     *p_arg);
static  void  AppTempTask    (void     *p_arg);

static  void  AppGraphLCD_Hdr(void);

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppTCPIP_Init  (NET_ERR  *perr);
#endif

#if (APP_CFG_PROBE_COM_MODULE_EN > 0u)
        void  AppProbe_Init  (void);
#endif

void  AppCloud_Init (CPU_BOOLEAN disableStatus);

#define EX_LED_ON(a) {if (AppCloudControlLedOn) LED_On(a); else LED_Off(a);}

/*
*********************************************************************************************************
*                                         GLOBAL VARIABLES
*********************************************************************************************************
*/

extern  volatile  CPU_INT16S  BSP_Accel_X_Zero;
extern  volatile  CPU_INT16S  BSP_Accel_Y_Zero;
extern  volatile  CPU_INT16S  BSP_Accel_Z_Zero;

                  CPU_INT08U  AppQuadrant  = 1;
                  CPU_FP32    AppDegrees   = 0.0f;
                  CPU_INT32U  AppMagnitude = 0;
                  CPU_FP32    AppTemp_C    = 0.0f;
                  CPU_FP32    AppTemp_F    = 0.0f;


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

    OSTaskCreate((OS_TCB     *)&AppAccelTaskTCB,                /* Create the accelerometer task                        */
                 (CPU_CHAR   *)"Accel Task",
                 (OS_TASK_PTR ) AppAccelTask,
                 (void       *) 0,
                 (OS_PRIO     ) ACCEL_TASK_PRIO,
                 (CPU_STK    *)&AppAccelTaskStk[0],
                 (CPU_STK_SIZE) ACCEL_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) ACCEL_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSTaskCreate((OS_TCB     *)&AppTempTaskTCB,                 /* Create the temperature task                          */
                 (CPU_CHAR   *)"Temp Task",
                 (OS_TASK_PTR ) AppTempTask,
                 (void       *) 0,
                 (OS_PRIO     ) TEMPERATURE_TASK_PRIO,
                 (CPU_STK    *)&AppTempTaskStk[0],
                 (CPU_STK_SIZE) TEMPERATURE_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) TEMPERATURE_TASK_STK_SIZE,
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

    while (DEF_ON) {
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

/*
*********************************************************************************************************
*                                          ACCELEROMETER TASK
*
* Description : This task initialises the ADXL345 accelerometer, sets the zero points for
*               the X and Y axis. It then periodically reads the current X and Y values from
*               the accelerometer and calculates the inclination angle and AppMagnitude of the
*               the board. Illuminates the LED corresponding to the direction the board is
*               currently tilted.
*
* Arguments   : p_arg   is the argument passed to 'AppAccelTask()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  AppAccelTask (void *p_arg)
{
    OS_ERR      err;
    CPU_FP32    angle_radians;
    CPU_FP32    angle_degrees;
    CPU_INT16S  accel_x_axis;
    CPU_INT16S  accel_y_axis;
    CPU_INT16S  accel_z_axis;
    CPU_FP32    x;
    CPU_FP32    x2;
    CPU_FP32    y;
    CPU_FP32    y2;
    CPU_FP32    sum_x2_y2;


    (void)&p_arg;

    angle_radians = 0.0f;
    angle_degrees = 0.0f;

    BSP_Accel_Init();                                           /* Initialize the accelerometer                         */
    BSP_Accel_ZeroCal();                                        /* Calibrate  the accelerometer                         */

    LED_Off(0);                                                 /* Turn all LEDs off                                    */

    while (DEF_ON) {
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

        accel_x_axis = BSP_Accel_X_AxisRd();
        accel_y_axis = BSP_Accel_Y_AxisRd();

        if ((APP_ACCEL_ZERO(accel_x_axis, BSP_Accel_X_Zero)) &&
            (APP_ACCEL_ZERO(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 5;                                    /* Board is flat                                        */

        } else if ((APP_ACCEL_NEG (accel_y_axis, BSP_Accel_Y_Zero)) &&
                   (APP_ACCEL_ZERO(accel_x_axis, BSP_Accel_X_Zero))) {
            AppQuadrant = 6;

        } else if ((APP_ACCEL_POS (accel_y_axis, BSP_Accel_Y_Zero)) &&
                   (APP_ACCEL_ZERO(accel_x_axis, BSP_Accel_X_Zero))) {
            AppQuadrant = 7;

        } else if ((APP_ACCEL_POS (accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_ZERO(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 8;

        } else if ((APP_ACCEL_NEG (accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_ZERO(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 9;

        } else if ((APP_ACCEL_POS(accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_POS(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 3;

        } else if ((APP_ACCEL_POS(accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_NEG(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 4;

        } else if ((APP_ACCEL_NEG(accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_POS(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 2;

        } else if ((APP_ACCEL_NEG(accel_x_axis, BSP_Accel_X_Zero)) &&
                   (APP_ACCEL_NEG(accel_y_axis, BSP_Accel_Y_Zero))) {
            AppQuadrant = 1;
        }

        x            = accel_x_axis;
        y            = accel_y_axis;
        x           -= BSP_Accel_X_Zero;                        /* Normalize                                            */
        y           -= BSP_Accel_Y_Zero;

        x2           = x * x;
        y2           = y * y;
        sum_x2_y2    = x2 + y2;
        AppMagnitude = sqrt(sum_x2_y2 / 2.0f);

        if ((fabs(x) > 0.5f) &&                                 /* Both axis are greater than zero.                     */
            (fabs(y) > 0.5f)) {
            angle_radians = atan2(fabs(x), fabs(y));
            angle_degrees = (angle_radians * 180.0f) / 3.1415926535897932f;
        }

        if (AppQuadrant == 5) {
            AppDegrees = 0.0f;
            if (AppTCPIP_Cfg == DEF_TRUE) {
                EX_LED_ON( 4);                                     /* Turn on all LEDs when the board is flat and ...      */
                EX_LED_ON( 5);                                     /* ... TCP-IP initialization is complete.               */
                EX_LED_ON( 6);
                EX_LED_ON( 7);
                EX_LED_ON( 8);
                EX_LED_ON( 9);
                EX_LED_ON(10);
                EX_LED_ON(11);
                EX_LED_ON(12);
                EX_LED_ON(13);
                EX_LED_ON(14);
                EX_LED_ON(15);

            } else {

                EX_LED_ON ( 4);                                    /* Turn on half of the LEDs to show that TCP-IP ...     */
                LED_Off( 5);                                    /* ... initialization is NOT yet complete.              */
                EX_LED_ON ( 6);
                LED_Off( 7);
                EX_LED_ON ( 8);
                LED_Off( 9);
                EX_LED_ON (10);
                LED_Off(11);
                EX_LED_ON (12);
                LED_Off(13);
                EX_LED_ON (14);
                LED_Off(15);
            }

        } else {

            LED_Off(0);                                         /* Turn off all LEDs                                    */

            switch (AppQuadrant) {
                case 1:
                     AppDegrees = angle_degrees;
                     if (angle_degrees < 30.0f) {
                         EX_LED_ON(4);
                     } else if (angle_degrees < 60.0f) {
                         EX_LED_ON(5);
                     } else {
                         EX_LED_ON(6);
                     }
                     break;


                case 2:
                     AppDegrees = 180.0f - angle_degrees;
                     if (angle_degrees < 30.0f) {
                         EX_LED_ON(9);
                     } else if (angle_degrees < 60.0f) {
                         EX_LED_ON(8);
                     } else {
                         EX_LED_ON(7);
                     }
                     break;


                case 3:
                     AppDegrees = angle_degrees + 180.0f;
                     if (angle_degrees < 30.0f) {
                         EX_LED_ON(10);
                     } else if( angle_degrees < 60.0f) {
                         EX_LED_ON(11);
                     } else {
                         EX_LED_ON(12);
                     }
                     break;


                case 4:
                     AppDegrees = 360.0f - angle_degrees;
                     if (angle_degrees < 30.0f) {
                         EX_LED_ON(15);
                     } else if (angle_degrees < 60.0f) {
                         EX_LED_ON(14);
                     } else {
                         EX_LED_ON(13);
                     }
                     break;


                case 6:
                     AppDegrees = 0.0f;
                     EX_LED_ON(4);
                     break;


                case 7:
                     AppDegrees = 180.0f;
                     EX_LED_ON(10);
                     break;


                case 8:
                     AppDegrees = 270.0f;
                     EX_LED_ON(13);
                     break;


                case 9:
                     AppDegrees =  90.0f;
                     EX_LED_ON(7);
                     break;
            }
        }
    }
}

/*
*********************************************************************************************************
*                                          TEMPERATURE SENSOR TASK
*
* Description : This task initialises the ADT7420 temperature sensor.
*               It then periodically reads the current temperature and updates global
*               variables AppTemp_C and AppTemp_F.
*               The temperature sensor is connected to the micro via I2C as is the accelerometer.
*               Having tasks accessing both demonstrates the use of a mutex in the I2C driver.
*
* Arguments   : p_arg   is the argument passed to 'AppAccelTask()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  AppTempTask (void *p_arg)
{
    OS_ERR    err;
    CPU_FP32  temp_c;


    (void)&p_arg;

    BSP_Temp_Init();                                            /* Initialize temperature sensor.                       */

    while (DEF_ON) {
        OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

        temp_c    = BSP_Temp_Rd();

        AppTemp_C = temp_c;
        AppTemp_F = (AppTemp_C * 9.0f / 5.0f) + 32.0f;
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
