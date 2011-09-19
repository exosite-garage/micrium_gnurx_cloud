/*
*********************************************************************************************************
*                                             uC/TCP-IP V2
*                                      The Embedded TCP/IP Suite
*                                    Application-Defined Functions
*
*                              (c) Copyright 2011; Micrium; Weston, FL
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

#include  <includes.h>

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
static  CPU_CHAR  IPAddr_AssignMsg_1[20];
static  CPU_CHAR  IPAddr_AssignMsg_2[20];

#if (APP_CFG_DHCPc_MODULE_EN > 0u)
        OS_TMR    AppDHCPTmr;
#endif
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (APP_CFG_TCPIP_MODULE_EN > 0u)
        void  AppGraphLCD_IPAddr           (NET_IF_NBR   if_nbr);

static  void  AppTCPIP_CfgStaticAddr       (NET_IF_NBR   if_nbr,
                                            NET_ERR     *perr);

#if (APP_CFG_DHCPc_MODULE_EN > 0u)
static  void  AppTmr_LinkStateCallback     (void        *p_tmr,
                                            void        *p_arg);
#endif
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
    CPU_CHAR     err_msg[] = "  ret err = xxxxx  ";
#if (APP_CFG_DHCPc_MODULE_EN > 0u)
    DHCPc_ERR    dhcp_err;
    OS_TICK      dly_tmr;
    OS_TICK      per_tmr;
    OS_ERR       os_err;
#endif


    Str_Copy(IPAddr_AssignMsg_1, "");
    Str_Copy(IPAddr_AssignMsg_2, "");

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
         BSP_GraphLCD_ClrLine(4u);
         BSP_GraphLCD_ClrLine(5u);
         BSP_GraphLCD_String(4, "Net Init failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

                                                                /* ------------------- ADD IF NBR 1 ------------------- */
    if_nbr  = NetIF_Add((void    *)&NetIF_API_Ether,            /* Ethernet API                                         */
                        (void    *)&NetDev_API_RX_EtherC,       /* Device API structure                                 */
                        (void    *)&NetDev_BSP_RSK2_RX62N,      /* Device BSP strcuture                                 */
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
         BSP_GraphLCD_ClrLine(4u);
         BSP_GraphLCD_ClrLine(5u);
         BSP_GraphLCD_String(4, "NetIF Add failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

#if (APP_CFG_DHCPc_MODULE_EN == 0u)                             /* ------------------- CFG IF NBR 1 ------------------- */
                                                                /* ---------------- MANUALLY CFG'D ADDR --------------- */
    AppTCPIP_CfgStaticAddr(if_nbr, perr);
    if (*perr != NET_IP_ERR_NONE) {
         Str_FmtNbr_Int32U(*perr,
                            5u,
                            DEF_NBR_BASE_DEC,
                            ASCII_CHAR_NULL,
                            DEF_NO,
                            DEF_YES,
                           &err_msg[12]);
         BSP_GraphLCD_ClrLine(4u);
         BSP_GraphLCD_ClrLine(5u);
         BSP_GraphLCD_String(4, "Static Addr failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }
    Str_Copy(IPAddr_AssignMsg_2, "Manually Configured");
    AppGraphLCD_IPAddr(if_nbr);
#else
                                                                /* --------------- DYNAMICALLY CFG'D ADDR ------------- */
    dhcp_err = DHCPc_Init();
    if (dhcp_err != DHCPc_ERR_NONE) {
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;                                                 /* DHCP client initialization failed                    */
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
         BSP_GraphLCD_ClrLine(4u);
         BSP_GraphLCD_ClrLine(5u);
         BSP_GraphLCD_String(4, "NetIF Start failed!");
         BSP_GraphLCD_String(5, err_msg);
         return;
    }

   *perr = NET_ERR_NONE;                                        /* Translate successful return error to NET_ERR_NONE.   */

#if (APP_CFG_DHCPc_MODULE_EN > 0u)                              /* ------------ MONITOR LINK STATE CHANGES ------------ */
    dly_tmr = 1u * OS_CFG_TMR_TASK_RATE_HZ;                     /* Initial dly to start monitoring link state.          */
    per_tmr = OS_CFG_TMR_TASK_RATE_HZ / 4u;                     /* Period between link state checks.                    */
    if (per_tmr == 0u) {
        per_tmr  = 1u;
    }

    OSTmrCreate(&AppDHCPTmr,
                "Link State Tmr (DHCP)",
                 dly_tmr,
                 per_tmr,
                 OS_OPT_TMR_PERIODIC,
                 AppTmr_LinkStateCallback,
                (void *)(CPU_INT32U)if_nbr,
                &os_err);
    if (os_err != OS_ERR_NONE) {
       *perr = NET_ERR_INIT_INCOMPLETE;
        Str_FmtNbr_Int32U(os_err,
                          5u,
                          DEF_NBR_BASE_DEC,
                          ASCII_CHAR_NULL,
                          DEF_NO,
                          DEF_YES,
                         &err_msg[12]);
        BSP_GraphLCD_ClrLine(4u);
        BSP_GraphLCD_ClrLine(5u);
        BSP_GraphLCD_String(4, "OSTmrCreate failed!");
        BSP_GraphLCD_String(5, err_msg);
        return;
    }

    OSTmrStart(&AppDHCPTmr, &os_err);
    if (os_err != OS_ERR_NONE) {
       *perr = NET_ERR_INIT_INCOMPLETE;
        Str_FmtNbr_Int32U(os_err,
                          5u,
                          DEF_NBR_BASE_DEC,
                          ASCII_CHAR_NULL,
                          DEF_NO,
                          DEF_YES,
                         &err_msg[12]);
        BSP_GraphLCD_ClrLine(4u);
        BSP_GraphLCD_ClrLine(5u);
        BSP_GraphLCD_String(4, "OSTmrStart failed!");
        BSP_GraphLCD_String(5, err_msg);
        return;
    }
#endif
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                     AppTmr_LinkStateCallback()
*********************************************************************************************************
*/
#if ((APP_CFG_TCPIP_MODULE_EN > 0u) && \
     (APP_CFG_DHCPc_MODULE_EN > 0u))
static  void  AppTmr_LinkStateCallback (void  *p_tmr,
                                        void  *p_arg)
{
    static  CPU_BOOLEAN       init = DEF_FALSE;
    static  CPU_BOOLEAN       prev_link_state;
            CPU_BOOLEAN       link_state;
            NET_IF_NBR        if_nbr;
            DHCPc_OPT_CODE    req_param[DHCPc_CFG_PARAM_REQ_TBL_SIZE];
            CPU_INT08U        req_param_qty;
    static  DHCPc_STATUS      prev_status = DHCP_STATUS_NONE;
            DHCPc_STATUS      status;
            NET_ERR           net_err;
            DHCPc_ERR         dhcp_err;
            CPU_CHAR          err_msg[] = "  ret err = xxxxx  ";
#if (APP_CFG_FTPs_MODULE_EN > 0u)
            NET_IP_ADDR       ip_addr_tbl[NET_IP_CFG_IF_MAX_NBR_ADDR];
            NET_IP_ADDRS_QTY  ip_addr_tbl_qty;
#endif


    if_nbr = (NET_IF_NBR)(CPU_INT32U)p_arg;

    link_state = NetIF_LinkStateGet(if_nbr, &net_err);          /* Chk link state.                                      */
    if (net_err != NET_IF_ERR_NONE) {                           /* Since Tmr Callback has Sched Locked, link state ...  */
        return;                                                 /* ... is only retrieved if Net Lock is available.      */
    }

    if (init == DEF_FALSE) {
        init  = DEF_TRUE;

        if (link_state == NET_IF_LINK_DOWN); {
#if (APP_CFG_AUDIO_EN > 0)
            Str_Copy(IPAddr_AssignMsg_1, "Ethernet Link Down!");
            Str_Copy(IPAddr_AssignMsg_2, "");
#else
            BSP_GraphLCD_ClrLine(4);
            BSP_GraphLCD_ClrLine(5);
            BSP_GraphLCD_ClrLine(6);
            BSP_GraphLCD_ClrLine(7);
            BSP_GraphLCD_String(4, "Ethernet Link Down!");
#endif
        }

        prev_link_state = NET_IF_LINK_DOWN;
    }

    if ((     link_state == NET_IF_LINK_UP) &&
        (prev_link_state == NET_IF_LINK_DOWN)) {                /* -------------------- START DHCP -------------------- */

        req_param[0]  = (DHCPc_OPT_CODE)DHCP_OPT_DOMAIN_NAME_SERVER;
        req_param_qty = 1;

        DHCPc_Start(if_nbr,                                     /* Starting DHCP                                        */
                   &req_param[0],
                    req_param_qty,
                   &dhcp_err);
                   
        if (dhcp_err == DHCPc_ERR_NONE) {
            prev_status = DHCP_STATUS_NONE;

#if (APP_CFG_AUDIO_EN > 0)
            Str_Copy(IPAddr_AssignMsg_1, "");
            Str_Copy(IPAddr_AssignMsg_2, "");
#else
            BSP_GraphLCD_ClrLine(4);
            BSP_GraphLCD_ClrLine(5);
            BSP_GraphLCD_ClrLine(6);
            BSP_GraphLCD_ClrLine(7);
#endif

        } else {
            prev_status = DHCP_STATUS_FAIL;

            Str_FmtNbr_Int32U(dhcp_err,
                              5u,
                              DEF_NBR_BASE_DEC,
                              ASCII_CHAR_NULL,
                              DEF_NO,
                              DEF_YES,
                             &err_msg[12]);

#if (APP_CFG_AUDIO_EN > 0)
            Str_Copy(IPAddr_AssignMsg_1, "DHCPc Start failed!");
            Str_Copy(IPAddr_AssignMsg_2, err_msg);
#else
            BSP_GraphLCD_ClrLine(4);
            BSP_GraphLCD_ClrLine(5);
            BSP_GraphLCD_ClrLine(6);
            BSP_GraphLCD_ClrLine(7);
            BSP_GraphLCD_String(4, "DHCPc Start failed!");
            BSP_GraphLCD_String(5, err_msg);
#endif
        }

    } else if ((     link_state == NET_IF_LINK_DOWN) &&
               (prev_link_state == NET_IF_LINK_UP)) {           /* --------------------- STOP DHCP -------------------- */

        DHCPc_Stop(if_nbr, &dhcp_err);

#if (APP_CFG_AUDIO_EN > 0)
        Str_Copy(IPAddr_AssignMsg_1, "Ethernet Link Down!");
        Str_Copy(IPAddr_AssignMsg_2, "");
#else
        BSP_GraphLCD_ClrLine(4);
        BSP_GraphLCD_ClrLine(5);
        BSP_GraphLCD_ClrLine(6);
        BSP_GraphLCD_ClrLine(7);
        BSP_GraphLCD_String(4, "Ethernet Link Down!");
#endif

    } else {

        status = DHCPc_ChkStatus(if_nbr, &dhcp_err);
        if (prev_status != status) {
            prev_status  = status;

            switch (status) {
                case DHCP_STATUS_CFG_IN_PROGRESS:
                     Str_Copy(IPAddr_AssignMsg_1, " Acquiring Network");
                     Str_Copy(IPAddr_AssignMsg_2, "      Address");
#if (APP_CFG_AUDIO_EN == 0)
                     AppGraphLCD_IPAddr(if_nbr);
#endif
                     break;

                case DHCP_STATUS_CFGD:                      /* IF configured                                        */
                     Str_Copy(IPAddr_AssignMsg_1, "");
                     Str_Copy(IPAddr_AssignMsg_2, "Assigned by DHCP");
                     break;

                case DHCP_STATUS_CFGD_NO_TMR:               /* IF configured (no timer set)                         */
                     Str_Copy(IPAddr_AssignMsg_1, "");
                     Str_Copy(IPAddr_AssignMsg_2, "Assigned by DHCP*");
                     break;

                case DHCP_STATUS_CFGD_LOCAL_LINK:           /* IF configured (link-local addr)                      */
                     Str_Copy(IPAddr_AssignMsg_1, "");
                     Str_Copy(IPAddr_AssignMsg_2, "Link-Local Address");
                     break;

                case DHCP_STATUS_FAIL:                      /* IF configuration failed                              */
                     AppTCPIP_CfgStaticAddr(if_nbr, &net_err);
                    (void)&net_err;

                     Str_Copy(IPAddr_AssignMsg_1, "");
                     Str_Copy(IPAddr_AssignMsg_2, "Manually Configured");
                     break;

                default:
                     break;
            }

            switch (status) {
                case DHCP_STATUS_CFGD:                      /* IF configured                                        */
                case DHCP_STATUS_CFGD_NO_TMR:               /* IF configured (no timer set)                         */
                case DHCP_STATUS_CFGD_LOCAL_LINK:           /* IF configured (link-local addr)                      */
                case DHCP_STATUS_FAIL:                      /* IF configuration failed                              */
#if (APP_CFG_FTPs_MODULE_EN > 0u)
                     ip_addr_tbl_qty = NET_IP_CFG_IF_MAX_NBR_ADDR;
                    (void)NetIP_GetAddrHost( if_nbr,
                                            &ip_addr_tbl[0],
                                            &ip_addr_tbl_qty,
                                            &net_err);
                     if (net_err == NET_IP_ERR_NONE) {
                         FTPs_SetPublicAddr(ip_addr_tbl[0], FTPs_PASSIVE_PORT);
                     }
#endif
#if ((APP_CFG_TFTPs_MODULE_EN == 0) && \
     (APP_CFG_FTPs_MODULE_EN  == 0))
                     AppGraphLCD_IPAddr(if_nbr);
#endif
                     break;
            }
        }
    }

    prev_link_state = link_state;
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
*                                        AppGraphLCD_IPAddr()
*********************************************************************************************************
*/
#if (APP_CFG_TCPIP_MODULE_EN > 0u)
void  AppGraphLCD_IPAddr (NET_IF_NBR  if_nbr)
{
    NET_IP_ADDR       ip_addr_tbl[NET_IP_CFG_IF_MAX_NBR_ADDR];
    NET_IP_ADDRS_QTY  ip_addr_tbl_qty;
    NET_ERR           net_err;
    NET_IP_ADDR       subnet_addr;
    NET_IP_ADDR       gateway_addr;
    CPU_CHAR          str_addr[20];



    BSP_GraphLCD_ClrLine(3u);
    BSP_GraphLCD_String(3, IPAddr_AssignMsg_1);

    BSP_GraphLCD_ClrLine(4u);
    BSP_GraphLCD_String(4, IPAddr_AssignMsg_2);

    ip_addr_tbl_qty = NET_IP_CFG_IF_MAX_NBR_ADDR;
   (void)NetIP_GetAddrHost( if_nbr,
                           &ip_addr_tbl[0],
                           &ip_addr_tbl_qty,
                           &net_err);

    switch (net_err) {
        case NET_IP_ERR_NONE:
             subnet_addr  = NetIP_GetAddrSubnetMask (ip_addr_tbl[0],
                                                     &net_err);
             gateway_addr = NetIP_GetAddrDfltGateway(ip_addr_tbl[0],
                                                     &net_err);

             str_addr[0] = ASCII_CHAR_NULL;
             Str_Copy(str_addr, "IP: ");
             NetASCII_IP_to_Str( ip_addr_tbl[0],
                                &str_addr[4],
                                 DEF_NO,
                                &net_err);
             BSP_GraphLCD_ClrLine(5u);
             BSP_GraphLCD_String(5, str_addr);

             str_addr[0] = ASCII_CHAR_NULL;
             Str_Copy(str_addr, "Sub:");
             NetASCII_IP_to_Str( subnet_addr,
                                &str_addr[4],
                                 DEF_NO,
                                &net_err);
             BSP_GraphLCD_ClrLine(6u);
             BSP_GraphLCD_String(6, str_addr);

             str_addr[0] = ASCII_CHAR_NULL;
             Str_Copy(str_addr, "GW: ");
             NetASCII_IP_to_Str( gateway_addr,
                                &str_addr[4],
                                 DEF_NO,
                                &net_err);
             BSP_GraphLCD_ClrLine(7u);
             BSP_GraphLCD_String(7, str_addr);
             break;


        case NET_IP_ERR_ADDR_NONE_AVAIL:
             BSP_GraphLCD_ClrLine(5u);
             BSP_GraphLCD_String(5u, "IP:  Not cfg'd");

             BSP_GraphLCD_ClrLine(6u);
             BSP_GraphLCD_String(6u, "Sub:");

             BSP_GraphLCD_ClrLine(7u);
             BSP_GraphLCD_String(7u, "GW:");
             break;
    }
}
#endif
