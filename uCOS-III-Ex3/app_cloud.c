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

#include "iodefine.h"
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

enum STATUS_MESSAGES
{
  MSG_ERROR,
  MSG_CONNECTED,
  MSG_UNAVAILABLE,
  MSG_BLANK,
  MSG_END
};

static CPU_BOOLEAN DisableCloudStatus = 0;

const CPU_CHAR msg_status[MSG_END][19]  = { "Cloud: Error\0",
                                            "Cloud: Connected\0",
                                            "Cloud: Unavailable\0",
                                            "                  \0"};

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

extern volatile  CPU_INT08U  AppCloudControlLedOn;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  CloudData_Task    (void *p_arg);                   /* Cloud data task.                           */
static  void  UI_Update         (CPU_CHAR message);              /* Local function for LCD updates             */

CPU_BOOLEAN Exosite_Init        (CPU_CHAR  *pOS,   CPU_CHAR  *pVer,    NET_IF_NBR if_nbr);
CPU_BOOLEAN Exosite_Reinit      (void);
CPU_SIZE_T  Exosite_Read        (CPU_CHAR  *pkey,  CPU_CHAR  *pbuf,    CPU_SIZE_T buflen);
CPU_BOOLEAN Exosite_Write       (CPU_CHAR  *pkey,  CPU_CHAR  *pval);
CPU_BOOLEAN Exosite_Write_Batch (CPU_CHAR **pkeys, CPU_CHAR **pvalues, CPU_SIZE_T count);
CPU_BOOLEAN Exosite_GetMAC      (CPU_CHAR * pMAC);

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
void AppCloud_Init (CPU_BOOLEAN disableStatus)
{
    OS_ERR err;
    
    if (disableStatus)
        DisableCloudStatus = 1;

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
    CPU_CHAR    *keys[1];
    CPU_CHAR    *values[1];
    CPU_CHAR     strping[4];
    CPU_CHAR     ping = 0;
    CPU_CHAR     ledctrl;
    CPU_CHAR     read_count = 0;
    CPU_BOOLEAN  cloud_available;
    CPU_CHAR     showMAC[18];

    (void)p_arg;

    keys[0] = "ping";
    values[0] = strping;

    // OS Name = "Micrium-Ex3" <- MAX Length = 24
    // OS Ver  = "3.01.2" <- MAX Length = 8
    // Use network interface '1' MAC address
    cloud_available = Exosite_Init("Micrium-Ex3", "3.01.2", (NET_IF_NBR)1);

    Str_Copy(showMAC, "MAC: ");
    if (Exosite_GetMAC(&showMAC[5]) && !DisableCloudStatus)
    {
        BSP_GraphLCD_String(3, showMAC);
    }
    
    while (DEF_TRUE)
    {
        if (DEF_TRUE != cloud_available)
        {
            UI_Update(MSG_UNAVAILABLE);

            // Sleep 20 seconds
            OSTimeDlyHMSM((CPU_INT16U)  0u,
                          (CPU_INT16U)  0u,
                          (CPU_INT16U) 20u,
                          (CPU_INT32U)  0u,
                          (OS_OPT    ) OS_OPT_TIME_HMSM_NON_STRICT,
                          (OS_ERR   *)&err);

            // Retry cloud activation until successful
            cloud_available = Exosite_Reinit();
        }
        else
        {
            if (read_count++ > 10) {
                read_count = 0;
                // Create string from ping value
                Str_FmtNbr_Int32U(ping++,
                                  3u,
                                  DEF_NBR_BASE_DEC,
                                  ASCII_CHAR_NULL,
                                  DEF_NO,
                                  DEF_YES,
                                 &strping[0]);

                if (DEF_TRUE != Exosite_Write_Batch(keys, values, 1))
                {
                    UI_Update(MSG_ERROR);
                }
                else
                {
                    UI_Update(MSG_CONNECTED);
                }
            }

            if (1 == Exosite_Read("led_ctrl", &ledctrl, 1))
            {
                UI_Update(MSG_CONNECTED);
                
                if ('0' == ledctrl)
                {
                    AppCloudControlLedOn = 0;
                }
                if ('1' == ledctrl)
                {
                    AppCloudControlLedOn = 1;
                }
            } else UI_Update(MSG_ERROR);

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

/*
*********************************************************************************************************
*                                           UI_Update()
*
* Description : Updates LCD with Cloud status messages
*
* Argument(s) : message          Index into const message array.
*
* Return(s)   : none.
*
* Caller(s)   : CloudData_Task
*
* Note(s)     : none.
*********************************************************************************************************
*/       
static void UI_Update (CPU_CHAR message)
{
static CPU_CHAR display_status = 0;

    if(!DisableCloudStatus)
    {
        if (!PORT4.PORT.BIT.B0 ||   //check if any switches are pressed
            !PORT4.PORT.BIT.B1 ||
            !PORT4.PORT.BIT.B2) {
            display_status ^= 1;
            if (!display_status) BSP_GraphLCD_String(3, msg_status[MSG_BLANK]); //only clear once on button press in case others want to use line 3
        }
    } else {
        if (!PORT4.PORT.BIT.B0 &&   //check if any switches are pressed
            !PORT4.PORT.BIT.B1 &&
            !PORT4.PORT.BIT.B2)
            display_status =1;
        else
            display_status = 0;
    }
    if (display_status) {
        BSP_GraphLCD_String(3, msg_status[MSG_BLANK]);  
        BSP_GraphLCD_String(3, msg_status[message]);
    }  
}