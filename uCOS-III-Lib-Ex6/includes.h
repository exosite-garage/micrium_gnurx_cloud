/*
*********************************************************************************************************
*                                               uC/OS-III
*                                          MASTER INCLUDE FILE
*
*                             (c) Copyright 2010, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*
* File : INCLUDES.H
* By   : FGK
*********************************************************************************************************
*/

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include  <cpu_core.h>
#include  <os.h>
#include  <os_app_hooks.h>
#include  <bsp.h>
#include  <bsp_os.h>
#include  <bsp_adt7420.h>
#include  <bsp_adxl345.h>
#include  <bsp_glcd.h>

#include  <math.h>

int snprintf(char *str, CPU_SIZE_T str_m, const char *fmt, /*args*/ ...);


#if (APP_CFG_PROBE_COM_MODULE_EN > 0)
  #include  <probe_com.h>
  #if (PROBE_COM_CFG_RS232_EN > 0)
  #include  <probe_rs232c.h>
  #include  <probe_rs232.h>
  #endif
  #if (PROBE_COM_CFG_TCPIP_EN > 0)
  #include  <probe_tcpip.h>
  #endif
#endif


#if (APP_CFG_TCPIP_MODULE_EN > 0)
  #include  <net.h>
  #include  <net_bsp.h>
  #include  <net_dev_rx_etherc.h>
  #include  <net_phy_dp83640.h>
  #include  <net_dev_cfg.h>
  
#if (APP_CFG_DHCPc_MODULE_EN > 0)
  #include  <dhcp-c.h>
#endif
#if (APP_CFG_TFTPs_MODULE_EN > 0)
  #include  <tftp-s.h>
#endif
#endif

#if (APP_CFG_FS_EN > 0)
#include  <fs_app.h>
#endif

#endif
