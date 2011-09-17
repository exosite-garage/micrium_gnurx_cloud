/*
*********************************************************************************************************
*                                               uC/OS-III
*                                       APPLICATION CONFIGURATION
*
*                             (c) Copyright 2010, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*
* File : APP_CFG.H
* By   : FGK
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include  <dhcp-c_cfg.h>
#include  <http-s_cfg.h>

/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_TCPIP_MODULE_EN         DEF_ENABLED            /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
#define  APP_CFG_DHCPc_MODULE_EN         DEF_ENABLED
#define  APP_CFG_PROBE_COM_MODULE_EN     DEF_ENABLED


/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                            4u
#define  BLINKY_TASK_PRIO                               5u
#define  ACCEL_TASK_PRIO                                5u
#define  TEMPERATURE_TASK_PRIO                          4u
#define  APP_FFT_TASK_PRIO                              3u

#define  NET_OS_CFG_TMR_TASK_PRIO                       6u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_PRIO             7u
#define  NET_OS_CFG_IF_LOOPBACK_TASK_PRIO               8u
#define  NET_OS_CFG_IF_RX_TASK_PRIO                     9u

#define  OS_PROBE_TASK_PRIO                            10u

#define  CLOUD_DATA_TASK_PRIO                          11u

#define  HTTPs_OS_CFG_TASK_PRIO                        13u

/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                      256u
#define  BLINKY_TASK_STK_SIZE                         512u
#define  ACCEL_TASK_STK_SIZE                          512u
#define  TEMPERATURE_TASK_STK_SIZE                    512u
#define  APP_FFT_TASK_STK_SIZE                        320u
#define  CLOUD_DATA_TASK_STK_SIZE                     512u

#define  NET_OS_CFG_TMR_TASK_STK_SIZE                 768u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_STK_SIZE       512u
#define  NET_OS_CFG_IF_LOOPBACK_TASK_STK_SIZE         512u
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE              1280u

#define  OS_PROBE_TASK_STK_SIZE                       768u

#define  HTTPs_OS_CFG_TASK_STK_SIZE                  3072u

/*
*********************************************************************************************************
*                                            uC/TCP-IP v2.0
*********************************************************************************************************
*/

#define  NET_OS_CFG_IF_LOOPBACK_Q_SIZE                  5u
#define  NET_OS_CFG_IF_RX_Q_SIZE                       14u
#define  NET_OS_CFG_IF_TX_DEALLOC_Q_SIZE               10u

/*
*********************************************************************************************************
*                                            uC/DHCPc v2.0
*********************************************************************************************************
*/

#define  APP_CFG_DHCP_NBR_IF_CFGD                       1u

/*
*********************************************************************************************************
*                                        uC/LIB CONFIGURATION
*********************************************************************************************************
*/

#define  LIB_MEM_CFG_OPTIMIZE_ASM_EN            DEF_DISABLED
#define  LIB_MEM_CFG_ARG_CHK_EXT_EN             DEF_ENABLED
#define  LIB_MEM_CFG_ALLOC_EN                   DEF_ENABLED
#define  LIB_MEM_CFG_HEAP_SIZE                   (34 * 1024L)


/*
*********************************************************************************************************
*                                              uC/APPs FS
*********************************************************************************************************
*/

#define  APPS_FS_CFG_MAX_FILE_NAME_LEN                    20u
#define  APPS_FS_CFG_NBR_FILES                             3u
#define  APPS_FS_CFG_NBR_DIRS                              1u


/*
*********************************************************************************************************
*                               uC/Probe plug-in for uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define  OS_PROBE_TASK                      1u                  /* Task will be created for uC/Probe OS Plug-In             */
#define  OS_PROBE_HOOKS_EN                  1u                  /* Hooks to update OS_TCB profiling members are included    */
#define  OS_PROBE_TMR_32_BITS               0u                  /* uC/Probe timer is a 32-bit timer                         */
#define  OS_PROBE_TMR_SEL                   0u                  /* Select timer for uC/Probe OS Plug-In timer               */
#define  OS_PROBE_USE_FP                    1u                  /* uC/Probe does not use floating-point variables           */
#define  OS_PROBE_TASK_ID    OS_PROBE_TASK_PRIO                 /* Current version of uC/OS-II does not use ID field        */
#define  PROBE_BAUD_RATE                38400u                  /* RS-232 baudrate used by uC/Probe                         */


#endif
