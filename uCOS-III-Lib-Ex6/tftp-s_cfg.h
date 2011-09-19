/*
*********************************************************************************************************
*                                              uC/TFTPs
*                               Trivial File Transfer Protocol (server)
*
*                          (c) Copyright 2005-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TFTPs is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   TFTP SERVER CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : tftp-s_cfg.h
* Version       : V1.91.01
* Programmer(s) : JDH
*                 SR
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           TASKS PRIORITIES
*********************************************************************************************************
*/

#define  TFTPs_OS_CFG_TASK_PRIO                            7


/*
*********************************************************************************************************
*                                              STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  TFTPs_OS_CFG_TASK_STK_SIZE                      512


/*
*********************************************************************************************************
*                                                 TFTPs
*********************************************************************************************************
*/

#define  TFTPs_CFG_IPPORT                                 69    /* TFTP server IP port.  Default is 69.                 */

#define  TFTPs_CFG_MAX_RX_TIMEOUT_MS                   30000    /* Maximum inactivity time (ms) on RX.                  */
#define  TFTPs_CFG_MAX_TX_TIMEOUT_MS                   30000    /* Maximum inactivity time (ms) on TX.                  */

#define  TFTPs_TRACE_HIST_SIZE                            16    /* Trace history size.  Minimum value is 16.            */


/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                   0
#define  TRACE_LEVEL_INFO                                  1
#define  TRACE_LEVEL_DBG                                   2

#define  TFTPs_TRACE_LEVEL                      TRACE_LEVEL_OFF
#define  TFTPs_TRACE                            printf
