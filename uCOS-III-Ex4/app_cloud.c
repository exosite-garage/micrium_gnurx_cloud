/*
*********************************************************************************************************
*                                               Exosite
*                                         Exosite Device Cloud
*
*                                      (c) Copyright 2011, Exosite
*                                          All Rights Reserved
*
*
* File    : APP_CLOUD.C
* By      : CSR
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include  <includes.h>

/*
*********************************************************************************************************
*                                         LOCAL ENUMERATIONS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB   CloudData_TaskTCB;
static  CPU_STK  CloudData_TaskStk[CLOUD_DATA_TASK_STK_SIZE];  /* Stack for cloud data task.                 */


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/
extern volatile  CPU_INT16U  AppFreqActualHz;
extern volatile  CPU_INT16U  AppFreqSetpointHz;
extern volatile  CPU_INT08U  AppCloudControlLedOn;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  CloudData_Task    (void *p_arg);                   /* Cloud data task.                           */

CPU_BOOLEAN Exosite_Init        (CPU_CHAR  *pOS,   CPU_CHAR  *pVer,    NET_IF_NBR if_nbr);
CPU_BOOLEAN Exosite_Reinit      (void);
CPU_SIZE_T  Exosite_Read        (CPU_CHAR  *pkey,  CPU_CHAR  *pbuf,    CPU_SIZE_T buflen);
CPU_BOOLEAN Exosite_Write       (CPU_CHAR  *pkey,  CPU_CHAR  *pval);
CPU_BOOLEAN Exosite_Write_Batch (CPU_CHAR **pkeys, CPU_CHAR **pvalues, CPU_SIZE_T count);

/*
*********************************************************************************************************
*                                           AppCloud_Init()
*
* Description : Initialize cloud connectivity and keep synchronized
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
void AppCloud_Init (void)
{
    OS_ERR err;

    OSTaskCreate((OS_TCB     *)&CloudData_TaskTCB,               /* Create cloud data reporting task.                      */
                 (CPU_CHAR   *)"Cloud Data Task",
                 (OS_TASK_PTR ) CloudData_Task,
                 (void       *) 0,
                 (OS_PRIO     ) CLOUD_DATA_TASK_PRIO,
                 (CPU_STK    *)&CloudData_TaskStk[0],
                 (CPU_STK_SIZE) CLOUD_DATA_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) CLOUD_DATA_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 2u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

/*
*********************************************************************************************************
*                                           CloudData_Task()
*
* Description : Sends sensor data to the cloud
*
* Argument(s) : p_arg           Argument passed to 'CloudData_Task()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void CloudData_Task (void *p_arg)
{
    OS_ERR       err;
    CPU_CHAR    *keys[2];
    CPU_CHAR    *values[2];
    CPU_CHAR     strdfreq[4];
    CPU_CHAR     strafreq[4];
    CPU_CHAR     ledctrl;
    CPU_BOOLEAN  cloud_available;

    (void)p_arg;

    keys[0] = "freq_d";
    keys[1] = "freq_a";
    values[0] = strdfreq;
    values[1] = strafreq;

    // OS Name = "Micrium-Ex4" <- MAX Length = 24
    // OS Ver  = "3.01.2" <- MAX Length = 8
    // Use network interface '1' MAC address
    cloud_available = Exosite_Init("Micrium-Ex4", "3.01.2", (NET_IF_NBR)1);

    while (DEF_TRUE)
    {
        if (DEF_TRUE != cloud_available)
        {
            BSP_GraphLCD_String(3, "Cloud: Unavailable");

            // Sleep 30 seconds
            OSTimeDlyHMSM((CPU_INT16U)  0u,
                          (CPU_INT16U)  0u,
                          (CPU_INT16U) 30u,
                          (CPU_INT32U)  0u,
                          (OS_OPT    ) OS_OPT_TIME_HMSM_NON_STRICT,
                          (OS_ERR   *)&err);

            // Retry cloud activation until successful
            cloud_available = Exosite_Reinit();
        }
        else
        {
            // Write current Actual and Desired frequencies to the cloud
            Str_FmtNbr_Int32U(AppFreqSetpointHz,
                              3u,
                              DEF_NBR_BASE_DEC,
                              ASCII_CHAR_NULL,
                              DEF_NO,
                              DEF_YES,
                             &strdfreq[0]);

            Str_FmtNbr_Int32U(AppFreqActualHz,
                              3u,
                              DEF_NBR_BASE_DEC,
                              ASCII_CHAR_NULL,
                              DEF_NO,
                              DEF_YES,
                             &strafreq[0]);

            if (DEF_TRUE != Exosite_Write_Batch(keys, values, 2))
            {
                BSP_GraphLCD_String(3, "Cloud: Error");
            }
            else
            {
                BSP_GraphLCD_String(3, "Cloud: Connected");

                if (1 == Exosite_Read("led_ctrl", &ledctrl, 1))
                {
                    if ('0' == ledctrl)
                    {
                        AppCloudControlLedOn = 0;
                    }
                    if ('1' == ledctrl)
                    {
                        AppCloudControlLedOn = 1;
                    }
                }
            }

            // Sleep 2 seconds
            OSTimeDlyHMSM((CPU_INT16U) 0u,
                          (CPU_INT16U) 0u,
                          (CPU_INT16U) 2u,
                          (CPU_INT32U) 0u,
                          (OS_OPT    ) OS_OPT_TIME_HMSM_NON_STRICT,
                          (OS_ERR   *)&err);
        }
    }
}
