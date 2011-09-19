/*
*********************************************************************************************************
*                                             uC/TCP-IP V2
*                                      The Embedded TCP/IP Suite
*                                    Application-Defined Functions
*
*                              (c) Copyright 2010; Micrium; Weston, FL
*                                         All Rights Reserved
*
* File    : APP_TCPIP.C
* By      : Fabiano Kovalski
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDES
*********************************************************************************************************
*/

#include <includes.h>


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
static  void         AppGraphLCD_IPAddr           (NET_IF_NBR   if_nbr);

static  void         AppTCPIP_CfgStaticAddr       (NET_IF_NBR   if_nbr,
                                                   NET_ERR     *perr);

#if (APP_CFG_DHCPc_MODULE_EN > 0u)
static  void         AppDHCPc_Init                (DHCPc_ERR   *perr);
#endif

static  CPU_BOOLEAN  AppNetIF_LinkStateWaitUntilUp(NET_IF_NBR   if_nbr,
                                                   CPU_INT16U   retry_max,
                                                   CPU_INT32U   time_dly_ms,
                                                   NET_ERR     *perr);
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           AppTCPIP_Init()
*
* Description : This function is called by AppTaskStart() and is responsible for initializing uC/TCP-IP,
*               and uC/DHCPc, if enabled.
*
*********************************************************************************************************
*/
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
void  AppTCPIP_Init (NET_ERR  *perr)
{
    NET_IF_NBR   if_nbr;
#if (APP_CFG_DHCPc_MODULE_EN > 0u)
    DHCPc_ERR    dhcp_err;
#endif
    CPU_INT32U   timeout;
    CPU_INT32U   retry_dly_ms;
    CPU_INT16U   retry_max;
    CPU_INT16U   retry_sec;
    CPU_BOOLEAN  link_state;
    CPU_CHAR     err_msg[]   = "  ret err = xxxxx  ";
    CPU_CHAR     retry_msg[] = "Retrying for xx sec";


                                                                /* ------------- INIT NET PROTOCOL STACK -------------- */
   *perr = Net_Init();
    if (*perr != NET_ERR_NONE) {
         Str_FmtNbr_Int32U(*perr,
                            5u,
                            DEF_NBR_BASE_DEC,
                            ASCII_CHAR_NULL,
                            DEF_NO,
                            DEF_YES,
                           &err_msg[12]);
         BSP_GraphLCD_ClrLine(4);
         BSP_GraphLCD_ClrLine(5);
         BSP_GraphLCD_String(4, "Net Init failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

                                                                /* ------------------- ADD IF NBR 1 ------------------- */
    if_nbr  = NetIF_Add((void    *)&NetIF_API_Ether,            /* Ethernet API                                         */
                        (void    *)&NetDev_API_RX_EtherC,       /* Device API structure                                 */
                        (void    *)&NetDev_BSP_RSK2_RX62N,      /* Device BSP structure                                 */
                        (void    *)&NetDev_Cfg_RX_Ether_0,      /* Device Configuration structure                       */
                        (void    *)&NetPhy_API_DP83640,         /* PHY API structure                                    */
                        (void    *)&NetPhy_Cfg_Generic_0,       /* PHY Configuration structure                          */
                        (NET_ERR *) perr);                      /* Return error variable                                */
    if (*perr != NET_IF_ERR_NONE) {
         Str_FmtNbr_Int32U(*perr,
                            5u,
                            DEF_NBR_BASE_DEC,
                            ASCII_CHAR_NULL,
                            DEF_NO,
                            DEF_YES,
                           &err_msg[12]);
         BSP_GraphLCD_ClrLine(4);
         BSP_GraphLCD_ClrLine(5);
         BSP_GraphLCD_String(4, "NetIF Add failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

#if (APP_CFG_DHCPc_MODULE_EN == 0u)                             /* ------------------- CFG IF NBR 1 ------------------- */
    AppTCPIP_CfgStaticAddr(if_nbr, perr);
    if (*perr != NET_IP_ERR_NONE) {
         Str_FmtNbr_Int32U(*perr,
                            5u,
                            DEF_NBR_BASE_DEC,
                            ASCII_CHAR_NULL,
                            DEF_NO,
                            DEF_YES,
                           &err_msg[12]);
         BSP_GraphLCD_ClrLine(4);
         BSP_GraphLCD_ClrLine(5);
         BSP_GraphLCD_String(4, "Static Addr failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }
#endif
                                                                /* ------------------ START IF NBR 1 ------------------ */
    NetIF_Start(if_nbr, perr);
    if (*perr != NET_IF_ERR_NONE) {
         Str_FmtNbr_Int32U(*perr,
                            5u,
                            DEF_NBR_BASE_DEC,
                            ASCII_CHAR_NULL,
                            DEF_NO,
                            DEF_YES,
                           &err_msg[12]);
         BSP_GraphLCD_ClrLine(4);
         BSP_GraphLCD_ClrLine(5);
         BSP_GraphLCD_String(4, "NetIF Start failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

                                                                /* ----------------- WAIT FOR LINK UP ----------------- */
    link_state = NetIF_LinkStateGet(if_nbr,                     /* Chk link state.                                      */
                                    perr);              
    if (link_state == NET_IF_LINK_DOWN) {
        BSP_GraphLCD_ClrLine(4);
        BSP_GraphLCD_String(4, "Ethernet Link Down!");

        timeout      =  30u;
        retry_dly_ms =  50u;
        retry_sec    =   1u;
        retry_max    = (retry_sec * DEF_TIME_NBR_mS_PER_SEC) / retry_dly_ms;

        while ((timeout    >  0u) &&
               (link_state == NET_IF_LINK_DOWN)) {

            Str_FmtNbr_Int32U(timeout,
                              2u,
                              DEF_NBR_BASE_DEC,
                              ASCII_CHAR_SPACE,
                              DEF_NO,
                              DEF_NO,
                             &retry_msg[13]);
            BSP_GraphLCD_ClrLine(5);
            BSP_GraphLCD_String(5, retry_msg);

            timeout--;
            link_state = AppNetIF_LinkStateWaitUntilUp(if_nbr,
                                                       retry_max,
                                                       retry_dly_ms,
                                                       perr);
        }

        if (link_state == NET_IF_LINK_UP) {
            BSP_GraphLCD_ClrLine(4);
            BSP_GraphLCD_String(4, "Ethernet Link Up!");
        }
        BSP_GraphLCD_ClrLine(5);
        BSP_GraphLCD_String(5, "");
    }
    if (*perr == NET_IF_ERR_NONE) {                             /* Translate successful return error to NET_ERR_NONE.   */
        *perr  = NET_ERR_NONE;
    }

#if (APP_CFG_DHCPc_MODULE_EN == 0u)                             /* ---------------- MANUALLY CFG'D ADDR --------------- */
    BSP_GraphLCD_ClrLine(4);
    BSP_GraphLCD_String(4, "Manually Configured");
    AppGraphLCD_IPAddr(if_nbr);
#else
    AppDHCPc_Init(&dhcp_err);                                   /* Initialize uC/DHCPc.                                 */
    if (dhcp_err != DHCPc_ERR_NONE) {                           /* If the DHCP client fails to init.                    */
       *perr = NET_ERR_INIT_INCOMPLETE;
        Str_FmtNbr_Int32U(dhcp_err,
                          5u,
                          DEF_NBR_BASE_DEC,
                          ASCII_CHAR_NULL,
                          DEF_NO,
                          DEF_YES,
                         &err_msg[12]);
        BSP_GraphLCD_ClrLine(4);
        BSP_GraphLCD_ClrLine(5);
        BSP_GraphLCD_String(4, "DHCPc Init failed!");
        BSP_GraphLCD_String(5, err_msg);
        return;
    }
#endif
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         AppTCPIP_CfgStaticAddr()
*********************************************************************************************************
*/
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
static  void  AppTCPIP_CfgStaticAddr (NET_IF_NBR   if_nbr,
                                      NET_ERR     *perr)
{
    NET_IP_ADDR  ip;
    NET_IP_ADDR  msk;
    NET_IP_ADDR  gateway;
    CPU_BOOLEAN  cfg_success;



    ip          = NetASCII_Str_to_IP((CPU_CHAR *)"10.10.1.65",    perr);
    msk         = NetASCII_Str_to_IP((CPU_CHAR *)"255.255.255.0", perr);
    gateway     = NetASCII_Str_to_IP((CPU_CHAR *)"10.10.1.1",     perr);

    cfg_success = NetIP_CfgAddrAdd(if_nbr,
                                   ip,
                                   msk,
                                   gateway,
                                   perr);

    (void)&cfg_success;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           AppDHCPc_Init()
*********************************************************************************************************
*/
#if ((APP_CFG_TCPIP_MODULE_EN > 0u) && \
     (APP_CFG_DHCPc_MODULE_EN > 0u))
static  void  AppDHCPc_Init (DHCPc_ERR  *perr)
{
    CPU_INT08U      nbr_if_started;
    NET_IF_NBR      if_nbr_cur;
    DHCPc_OPT_CODE  req_param[DHCPc_CFG_PARAM_REQ_TBL_SIZE];
    CPU_INT08U      req_param_qty;
    CPU_INT08U      nbr_if_init;
    CPU_BOOLEAN     if_dhcp_init_tbl[APP_CFG_DHCP_NBR_IF_CFGD];
    CPU_INT08U      if_done_ix;
    DHCPc_STATUS    status;
    OS_ERR          os_err;
    NET_ERR         err_addr_cfg;
    CPU_INT32U      cnt;
    CPU_BOOLEAN     show;
    CPU_INT08U      col;



   *perr = DHCPc_Init();
    if (*perr != DHCPc_ERR_NONE) {
         return;                                                /* DHCP client initialization failed                    */
    }

    req_param[0]   = (DHCPc_OPT_CODE)DHCP_OPT_DOMAIN_NAME_SERVER;
    req_param_qty  = 1;
    nbr_if_started = 0;
    if_nbr_cur     = NET_IF_NBR_BASE_CFGD;

    while (nbr_if_started < APP_CFG_DHCP_NBR_IF_CFGD) {

        DHCPc_Start((NET_IF_NBR      ) if_nbr_cur,              /* Starting DHCP                                        */
                    (DHCPc_OPT_CODE *)&req_param[0],
                    (CPU_INT08U      ) req_param_qty,
                    (DHCPc_ERR      *) perr);

        if (*perr != DHCPc_ERR_NONE) {
             return;                                            /* FAILED                                               */
        }

        nbr_if_started++;
        if_nbr_cur++;
    }

    nbr_if_init = 0;
    for (if_done_ix = 0; if_done_ix < APP_CFG_DHCP_NBR_IF_CFGD; if_done_ix++) {
        if_dhcp_init_tbl[if_done_ix] = DEF_NO;
    }

    cnt = 0;
    while (nbr_if_init < APP_CFG_DHCP_NBR_IF_CFGD) {
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &os_err);

        if_done_ix = 0;
        if_nbr_cur = NET_IF_NBR_BASE_CFGD;
        show = DEF_TRUE;
        while (if_done_ix < APP_CFG_DHCP_NBR_IF_CFGD) {
            if (if_dhcp_init_tbl[if_done_ix] != DEF_YES) {

                status = DHCPc_ChkStatus(NET_IF_NBR_BASE_CFGD,
                                         perr);

                switch (status) {
                    case DHCP_STATUS_CFG_IN_PROGRESS:
                         col  = cnt / 2;
                         col %= 12;
                         if (col > 6) {
                             col = 12 - col;
                         }
                         cnt++;

                         if (show == DEF_TRUE) {
                             show  = DEF_FALSE;

                             BSP_GraphLCD_ClrLine(4);
                             BSP_GraphLCD_ClrLine(5);
                             BSP_GraphLCD_StringPos(4, 1, "Acquiring Network");
                             BSP_GraphLCD_StringPos(5, 6, "Address");
                         }
                         BSP_GraphLCD_ClrLine(6);
                         BSP_GraphLCD_StringPos(6, 6 + col, ".");
                         break;

                    case DHCP_STATUS_CFGD:                      /* IF configured                                        */
                         if_dhcp_init_tbl[if_done_ix] = DEF_YES;
                         nbr_if_init++;

                         BSP_GraphLCD_ClrLine(4);
                         BSP_GraphLCD_String(4, "Assigned by DHCP");
                         AppGraphLCD_IPAddr(if_nbr_cur);
                         break;

                    case DHCP_STATUS_CFGD_NO_TMR:               /* IF configured (no timer set)                         */
                         if_dhcp_init_tbl[if_done_ix] = DEF_YES;
                         nbr_if_init++;
                         AppTCPIP_CfgStaticAddr(nbr_if_init,
                                                &err_addr_cfg);
                        (void)&err_addr_cfg;

                         BSP_GraphLCD_ClrLine(4);
                         BSP_GraphLCD_String(4, "Assigned by DHCP*");
                         AppGraphLCD_IPAddr(if_nbr_cur);
                         break;

                    case DHCP_STATUS_CFGD_LOCAL_LINK:           /* IF configured (link-local addr)                      */
                         if_dhcp_init_tbl[if_done_ix] = DEF_YES;
                         nbr_if_init++;

                         BSP_GraphLCD_ClrLine(4);
                         BSP_GraphLCD_String(4, "Link-Local Address");
                         AppGraphLCD_IPAddr(if_nbr_cur);
                         break;

                    case DHCP_STATUS_FAIL:                      /* IF configuration failed                              */
                         if_dhcp_init_tbl[if_done_ix] = DEF_YES;
                         nbr_if_init++;
                         AppTCPIP_CfgStaticAddr(nbr_if_init,
                                                &err_addr_cfg);
                        (void)&err_addr_cfg;

                         BSP_GraphLCD_ClrLine(4);
                         BSP_GraphLCD_String(4, "Manually Configured");
                         AppGraphLCD_IPAddr(if_nbr_cur);
                         break;

                    default:
                         break;
                }
            }

            if_done_ix++;
            if_nbr_cur++;
        }
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                   AppNetIF_LinkStateWaitUntilUp()
*
* Description : Wait for a network interface's link state to be 'UP'.
*
* Argument(s) : if_nbr          Network interface number to check link state.
*
*               retry_max       Maximum number of consecutive socket open retries   (see Note #2).
*
*               time_dly_ms     Transitory socket open delay value, in milliseconds (see Note #2).
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_IF_ERR_NONE                 Network interface's link state 'UP'.
*                               NET_IF_ERR_LINK_DOWN            Network interface's link state 'DOWN'.
*
*                                                               - RETURNED BY NetIF_LinkStateGet() : -
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_IF_ERR_INVALID_IF           Invalid network interface number.
*                               NET_OS_ERR_LOCK                 Network access NOT acquired.
*
* Return(s)   : NET_IF_LINK_UP,   if NO error(s) & network interface's link state is 'UP'.
*
*               NET_IF_LINK_DOWN, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be
*               called by application function(s) [see also Note #1].
*
* Note(s)     : (1) AppNetIF_LinkStateWaitUntilUp() is called by application function(s) & ... :
*
*                   (a) MUST NOT be called with the global network lock already acquired; ...
*                   (b) MUST block ALL other network protocol tasks by pending on & acquiring the global
*                       network lock (see 'NetIF_LinkStateGet()  Note #1b').
*
*                   This is required since an application's network protocol suite API function access is
*                   asynchronous to other network protocol tasks.
*
*               (2) If a non-zero number of retries is requested then a non-zero time delay SHOULD also be 
*                   requested; otherwise, all retries will most likely fail immediately since no time will 
*                   elapse to wait for & allow the network interface's link state to successfully be 'UP'.
*********************************************************************************************************
*/
/*$PAGE*/
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
CPU_BOOLEAN  AppNetIF_LinkStateWaitUntilUp (NET_IF_NBR   if_nbr,
                                            CPU_INT16U   retry_max,
                                            CPU_INT32U   time_dly_ms,
                                            NET_ERR     *perr)
{
    CPU_BOOLEAN  link_state;
    CPU_BOOLEAN  done;
    CPU_BOOLEAN  dly;
    CPU_INT16U   retry_cnt;
    NET_ERR      err;
    NET_ERR      err_rtn;



    link_state = NET_IF_LINK_DOWN;                              /* --------- WAIT FOR NET IF LINK STATE 'UP' ---------- */
    done       = DEF_NO;
    dly        = DEF_NO;
    retry_cnt  = 0u;

    while ((retry_cnt <= retry_max) &&                          /* While retry <= max retry ...                         */
           (done      == DEF_NO)) {                             /* ... & link NOT UP,       ...                         */

        if (dly == DEF_YES) {
            NetOS_TimeDly_ms(time_dly_ms,
                             &err);
        }

        link_state = NetIF_LinkStateGet(if_nbr,                 /* ... chk link state.                                  */
                                        &err);
        switch (err) {
            case NET_IF_ERR_NONE:
                 if (link_state == NET_IF_LINK_UP) {
                     done    = DEF_YES;
                     err_rtn = NET_IF_ERR_NONE;
                 } else {
                     retry_cnt++;
                     dly     = DEF_YES;
                     err_rtn = NET_IF_ERR_LINK_DOWN;
                 }
                 break;

            case NET_ERR_INIT_INCOMPLETE:                       /* If transitory err(s), ...                            */
            case NET_OS_ERR_LOCK:
                 retry_cnt++;
                 dly     = DEF_YES;                             /* ... dly retry.                                       */
                 err_rtn = err;
                 break;

            case NET_IF_ERR_INVALID_IF:
            default:
                 done    = DEF_YES;
                 err_rtn = err;
                 break;
        }
    }

   *perr =  err_rtn;

    return (link_state);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        AppGraphLCD_IPAddr()
*********************************************************************************************************
*/
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
static  void  AppGraphLCD_IPAddr (NET_IF_NBR  if_nbr)
{
    NET_IP_ADDR       ip_addr_tbl[NET_IP_CFG_IF_MAX_NBR_ADDR];
    NET_IP_ADDRS_QTY  ip_addr_tbl_qty;
    NET_ERR           net_err;
    NET_IP_ADDR       subnet_addr;
    NET_IP_ADDR       gateway_addr;
    CPU_CHAR          str_addr[20];



    ip_addr_tbl_qty = NET_IP_CFG_IF_MAX_NBR_ADDR;
   (void)NetIP_GetAddrHost( if_nbr,
                           &ip_addr_tbl[0],
                           &ip_addr_tbl_qty,
                           &net_err);

    if (net_err == NET_IP_ERR_NONE) {
        subnet_addr  = NetIP_GetAddrSubnetMask (ip_addr_tbl[0],
                                                &net_err);
        gateway_addr = NetIP_GetAddrDfltGateway(ip_addr_tbl[0],
                                                &net_err);

        BSP_GraphLCD_ClrLine(5);
        BSP_GraphLCD_ClrLine(6);
        BSP_GraphLCD_ClrLine(7);

        str_addr[0] = ASCII_CHAR_NULL;
        Str_Copy(str_addr, "IP: ");
        NetASCII_IP_to_Str( ip_addr_tbl[0],
                           &str_addr[4],
                            DEF_NO,
                           &net_err);
        BSP_GraphLCD_String(5, str_addr);

        str_addr[0] = ASCII_CHAR_NULL;
        Str_Copy(str_addr, "Sub:");
        NetASCII_IP_to_Str( subnet_addr,
                           &str_addr[4],
                            DEF_NO,
                           &net_err);
        BSP_GraphLCD_String(6, str_addr);

        str_addr[0] = ASCII_CHAR_NULL;
        Str_Copy(str_addr, "GW: ");
        NetASCII_IP_to_Str( gateway_addr,
                           &str_addr[4],
                            DEF_NO,
                           &net_err);
        BSP_GraphLCD_String(7, str_addr);
    }
}
#endif
