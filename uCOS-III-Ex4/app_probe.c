/*
*********************************************************************************************************
*                                                uC/OS-III
*                                          The Real-Time Kernel
*                                  Application-Defined uC/Probe Functions
*
*                                 (c) Copyright 2010; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : APP_PROBE.C
* By      : Fabiano Kovalski
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDES
*********************************************************************************************************
*/

#include <includes.h>


/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE PROBE MODULES
*
* Description: This function initializes the modules required for uC/Probe.
*
* Arguments  : none.
*
* Note(s)    : none.
*********************************************************************************************************
*/

void  AppProbe_Init (void)
{
#if (APP_CFG_PROBE_COM_MODULE_EN > 0)
    ProbeCom_Init();                                                    /* Initialize the uC/Probe communications module            */

#if (PROBE_COM_CFG_RS232_EN > 0)
    ProbeRS232_Init(PROBE_BAUD_RATE);
    ProbeRS232_RxIntEn();
#endif

#if (PROBE_COM_CFG_TCPIP_EN > 0)
    ProbeTCPIP_Init();
#endif
#endif
}
