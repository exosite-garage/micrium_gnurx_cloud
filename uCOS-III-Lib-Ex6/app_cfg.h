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

/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_TCPIP_MODULE_EN         DEF_ENABLED            /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
#define  APP_CFG_DHCPc_MODULE_EN         DEF_ENABLED
#define  APP_CFG_TFTPs_MODULE_EN         DEF_ENABLED
#define  APP_CFG_FS_EN                   DEF_ENABLED
#define  APP_CFG_AUDIO_EN                DEF_ENABLED

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
#if (APP_CFG_DHCPc_MODULE_EN > 0u)
#include  <dhcp-c_cfg.h>
#endif
#if (APP_CFG_TFTPs_MODULE_EN > 0u)
#include  <tftp-s_cfg.h>
#endif
#endif

/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/

#define  CLK_OS_CFG_TASK_PRIO                           2u

#define  APP_TASK_START_PRIO                            4u

#define  AUDIO_UI_TASK_PRIO                             6u
#define  AUDIO_MGR_TASK_PRIO                            6u

#define  NET_OS_CFG_TMR_TASK_PRIO                       8u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_PRIO             7u
#define  NET_OS_CFG_IF_LOOPBACK_TASK_PRIO               6u
#define  NET_OS_CFG_IF_RX_TASK_PRIO                     9u

#define  CLOUD_DATA_TASK_PRIO                          11u

/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*********************************************************************************************************
*/

#define  CLK_OS_CFG_TASK_STK_SIZE                     256u

#define  APP_TASK_START_STK_SIZE                      384u

#define  AUDIO_UI_TASK_STK_SIZE                       768u
#define  AUDIO_MGR_TASK_STK_SIZE                      640u
#define  CLOUD_DATA_TASK_STK_SIZE                     512u

#define  NET_OS_CFG_TMR_TASK_STK_SIZE                 768u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_STK_SIZE       512u
#define  NET_OS_CFG_IF_LOOPBACK_TASK_STK_SIZE         512u
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE              1280u


/*
*********************************************************************************************************
*                                         uC/FS CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_FS_SD_EN                       DEF_ENABLED

#define  APP_CFG_FS_DEV_CNT                             2u
#define  APP_CFG_FS_VOL_CNT                             2u
#define  APP_CFG_FS_FILE_CNT                            4u
#define  APP_CFG_FS_DIR_CNT                             1u
#define  APP_CFG_FS_BUF_CNT          (2u * APP_CFG_FS_VOL_CNT)
#define  APP_CFG_FS_DEV_DRV_CNT                         2u
#define  APP_CFG_FS_MAX_SEC_SIZE                      512u

#define  APP_TRACE_DBG(x)

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
#if (APP_CFG_AUDIO_EN > 0u)
#define  LIB_MEM_CFG_HEAP_SIZE                     (36416L + AUDIO_CFG_BUF_NBR * (AUDIO_CFG_BUF_SIZE + 24))
#else
#define  LIB_MEM_CFG_HEAP_SIZE                     (36416L)
#endif

/*
*********************************************************************************************************
*                                              uC/APPs FS
*********************************************************************************************************
*/

#define  APPS_FS_CFG_MAX_FILE_NAME_LEN                 20u
#define  APPS_FS_CFG_NBR_FILES                          3u
#define  APPS_FS_CFG_NBR_DIRS                           1u


/*
*********************************************************************************************************
*                                              uC/Audio
*********************************************************************************************************
*/

#define  AUDIO_CFG_BUF_NBR                             26u
#define  AUDIO_CFG_BUF_SIZE                           512u
#define  AUDIO_CFG_BUF_TH                              32u

/*
*********************************************************************************************************
*                                      BSP Serial CONFIGURATION
*********************************************************************************************************
*/

#define  BSP_CFG_SER_COMM_SEL                           2u
#define  BSP_CFG_SER_ALT_SEL
#define  BSP_CFG_SER_BAUDRATE                      115200u

#endif
