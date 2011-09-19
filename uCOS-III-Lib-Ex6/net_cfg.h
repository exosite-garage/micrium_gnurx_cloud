/*
*********************************************************************************************************
*                                             uC/TCP-IP V2
*                                      The Embedded TCP/IP Suite
*
*                          (c) Copyright 2003-2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TCP-IP is provided in source form to registered licensees ONLY.  It is 
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
*                                     NETWORK CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : net_cfg.h
* Version       : V2.11.02
* Programmer(s) : ITJ
*                 SR
*                 SL
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NETWORK CONFIGURATION
*
* Note(s) : (1) (a) Configure NET_CFG_INIT_CFG_VALS with the desired configuration for the initialization
*                   of network protocol suite configurable parameters :
*
*                       NET_INIT_CFG_VALS_DFLT          Configure network parameters with default values
*                       NET_INIT_CFG_VALS_APP_INIT      Configure network parameters with application-
*                                                           initialized values (recalled from non-volatile
*                                                           memory &/or hard-coded in product's application)
*
*               (b) (1) When NET_CFG_INIT_CFG_VALS is configured as NET_INIT_CFG_VALS_DFLT, the network
*                       protocol suite's configurable parameters are configured with default values.
*
*                       The application need only call Net_Init() to initialize both the network protocol
*                       suite & its configurable parameters.
*
*                   (2) When NET_CFG_INIT_CFG_VALS is configured as NET_INIT_CFG_VALS_APP_INIT, the
*                       application MUST ...
*
*                       (A) Initialize ALL network protocol suite configurable parameters from values
*                           recalled from non-volatile memory &/or hard-coded application values :
*
*                           (1) Call each of the configuration functions in Net_InitDflt() passing the
*                               application-recalled &/or hard-coded values
*                                 OR
*                           (2) Call Net_InitDflt() to initialize ALL network protocol suite configurable
*                               parameters to their default values & then call any of the configuration
*                               functions in Net_InitDflt() passing the recalled &/or hard-coded values
*                               to initialize some of the network protocol suite configurable parameters
*
*                       (B) Call Net_Init()
*
*               See also 'net.c  Net_InitDflt()  Notes #1 & #2'
*                      & 'net_def.h  PARAMETER CONFIGURATION INITIALIZATION  Note #1'.
*
*           (2) (a) Configure NET_CFG_OPTIMIZE with the desired network performance optimization :
*
*                       NET_OPTIMIZE_SPD            Optimizes network protocol suite for speed performance
*                       NET_OPTIMIZE_SIZE           Optimizes network protocol suite for binary image size
*
*               (b) Configure NET_CFG_OPTIMIZE_ASM_EN to enable/disable network protocol suite assembly
*                   optimization functions.
*
*           (3) Configure NET_CFG_BUILD_LIB_EN to enable/disable building the network protocol suite as 
*               a linkable library.
*********************************************************************************************************
*/
                                                                /* Configure network protocol suite's configurable ...  */
                                                                /* ... parameters' initial values (see Note #1) :       */
#define  NET_CFG_INIT_CFG_VALS                  NET_INIT_CFG_VALS_DFLT
                                                                /*   NET_INIT_CFG_VALS_DFLT         Default     values  */
                                                                /*   NET_INIT_CFG_VALS_APP_INIT     Application values  */


                                                                /* Configure network protocol suite's performance ...   */
                                                                /* ... optimization (see Note #2a) :                    */
#define  NET_CFG_OPTIMIZE                       NET_OPTIMIZE_SPD
                                                                /*   NET_OPTIMIZE_SPD               Optimize for Speed  */
                                                                /*   NET_OPTIMIZE_SIZE              Optimize for Size   */

                                                                /* Configure network protocol suite's assembly ...      */
                                                                /* ... optimization (see Note #2b) :                    */
#define  NET_CFG_OPTIMIZE_ASM_EN                DEF_DISABLED
                                                                /*   DEF_DISABLED       Assembly optimization DISABLED  */
                                                                /*   DEF_ENABLED        Assembly optimization ENABLED   */


                                                                /* Configure network protocol suite to build as a ...   */
                                                                /* ... linkable library (see Note #3) :                 */
#define  NET_CFG_BUILD_LIB_EN                   DEF_DISABLED
                                                                /*   DEF_DISABLED       Build as library DISABLED       */
                                                                /*   DEF_ENABLED        Build as library ENABLED        */


/*$PAGE*/
/*
*********************************************************************************************************
*                                     NETWORK DEBUG CONFIGURATION
*
* Note(s) : (1) Configure NET_DBG_CFG_INFO_EN to enable/disable network protocol suite debug information :
*
*               (a) Internal constants                         assigned to global variables
*               (b) Internal variable  data sizes calculated & assigned to global variables
*
*           (2) Configure NET_DBG_CFG_STATUS_EN to enable/disable network protocol suite debug status
*               functions.
*
*           (3) Configure NET_DBG_CFG_MEM_CLR_EN to enable/disable the network protocol suite from clearing
*               internal data structure memory buffers; a convenient feature while debugging.
*
*           (4) Configure NET_DBG_CFG_TEST_EN to enable/disable the network protocol suite test features :
*
*               (a) Internal data structure test &/or control flags
*               (b) Internal data structure read functionality
*********************************************************************************************************
*/

                                                                /* Configure debug information feature (see Note #1) :  */
#define  NET_DBG_CFG_INFO_EN                    DEF_DISABLED
                                                                /*   DEF_DISABLED  Debug information DISABLED           */
                                                                /*   DEF_ENABLED   Debug information ENABLED            */

                                                                /* Configure debug status functions (see Note #2) :     */
#define  NET_DBG_CFG_STATUS_EN                  DEF_DISABLED
                                                                /*   DEF_DISABLED  Debug status functions DISABLED      */
                                                                /*   DEF_ENABLED   Debug status functions ENABLED       */

                                                                /* Configure memory clear feature  (see Note #3) :      */
#define  NET_DBG_CFG_MEM_CLR_EN                 DEF_DISABLED
                                                                /*   DEF_DISABLED  Data structure clears DISABLED       */
                                                                /*   DEF_ENABLED   Data structure clears ENABLED        */

                                                                /* Configure test/debug feature(s) (see Note #4) :      */
#define  NET_DBG_CFG_TEST_EN                    DEF_DISABLED
                                                                /*   DEF_DISABLED  Test/Debug features DISABLED         */
                                                                /*   DEF_ENABLED   Test/Debug features ENABLED          */


/*
*********************************************************************************************************
*                                NETWORK ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure NET_ERR_CFG_ARG_CHK_EXT_EN to enable/disable the network protocol suite external
*               argument check feature :
*
*               (a) When ENABLED,  ALL arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*               (b) When DISABLED, NO  arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*           (2) Configure NET_ERR_CFG_ARG_CHK_DBG_EN to enable/disable the network protocol suite internal,
*               debug argument check feature :
*
*               (a) When ENABLED,     internal arguments are checked/validated to debug the network protocol
*                   suite.
*
*               (b) When DISABLED, NO internal arguments are checked/validated to debug the network protocol
*                   suite.
*********************************************************************************************************
*/
                                                                /* Configure external argument check feature ...        */
                                                                /* ... (see Note #1) :                                  */
#define  NET_ERR_CFG_ARG_CHK_EXT_EN             DEF_ENABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */

                                                                /* Configure internal argument check feature ...        */
                                                                /* ... (see Note #2) :                                  */
#define  NET_ERR_CFG_ARG_CHK_DBG_EN             DEF_DISABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */


/*$PAGE*/
/*
*********************************************************************************************************
*                               NETWORK COUNTER MANAGEMENT CONFIGURATION
*
* Note(s) : (1) Configure NET_CTR_CFG_STAT_EN to enable/disable network protocol suite statistics counters.
*
*           (2) Configure NET_CTR_CFG_ERR_EN  to enable/disable network protocol suite error      counters.
*********************************************************************************************************
*/

                                                                /* Configure statistics counter feature (see Note #1) : */
#define  NET_CTR_CFG_STAT_EN                    DEF_DISABLED
                                                                /*   DEF_DISABLED     Stat  counters DISABLED           */
                                                                /*   DEF_ENABLED      Stat  counters ENABLED            */

                                                                /* Configure error      counter feature (see Note #2) : */
#define  NET_CTR_CFG_ERR_EN                     DEF_DISABLED
                                                                /*   DEF_DISABLED     Error counters DISABLED           */
                                                                /*   DEF_ENABLED      Error counters ENABLED            */


/*
*********************************************************************************************************
*                             NETWORK STATISTIC MANAGEMENT CONFIGURATION
*********************************************************************************************************
*/



/*$PAGE*/
/*
*********************************************************************************************************
*                               NETWORK TIMER MANAGEMENT CONFIGURATION
*
* Note(s) : (1) Configure NET_TMR_CFG_NBR_TMR with the desired number of network TIMER objects.
*
*               Timers are required for :
*
*               (a) ARP  cache entries
*               (b) IP   fragment reassembly
*               (c) ICMP low-network-resources monitor task
*               (d) TCP  state machine connections
*
*           (2) Configure NET_TMR_CFG_TASK_FREQ to schedule the execution frequency of the network timer
*               task -- how often NetTmr_TaskHandler() is scheduled to run per second as implemented in
*               NetOS_Tmr_Task().
*
*               (a) NET_TMR_CFG_TASK_FREQ  MUST NOT be configured as a floating-point frequency.
*
*               See also 'net_tmr.h  NETWORK TIMER TASK TIME DEFINES  Notes #1 & #2'
*                      & 'net_os.c   NetOS_Tmr_Task()  Notes #1 & #2'.
*********************************************************************************************************
*/

#define  NET_TMR_CFG_NBR_TMR                              10u   /* Configure total number of TIMERs (see Note #1).      */

#define  NET_TMR_CFG_TASK_FREQ                            10u   /* Configure Timer Task frequency   (see Note #2).      */


/*$PAGE*/
/*
*********************************************************************************************************
*                                NETWORK INTERFACE LAYER CONFIGURATION
*
* Note(s) : (1) Configure NET_IF_CFG_&&&_EN to enable/disable specific network interface(s).
*********************************************************************************************************
*/

#define  NET_IF_CFG_MAX_NBR_IF                             1u   /* Configure maximum number of network interfaces.      */

                                                                /* Configure specific interface(s) [see Note #1] :      */
#define  NET_IF_CFG_LOOPBACK_EN                 DEF_DISABLED
#define  NET_IF_CFG_ETHER_EN                    DEF_ENABLED

                                                                /* Configure network interface address filter feature : */
#define  NET_IF_CFG_ADDR_FLTR_EN                DEF_ENABLED
                                                                /*   DEF_DISABLED      Addresses NOT filtered           */
                                                                /*   DEF_ENABLED       Addresses     filtered           */


                                                                /* Configure network interface transmit suspend ...     */
#define  NET_IF_CFG_TX_SUSPEND_TIMEOUT_MS                  1u   /* ... timeout in integer milliseconds.                 */


/*
*********************************************************************************************************
*                           ADDRESS RESOLUTION PROTOCOL LAYER CONFIGURATION
*
* Note(s) : (1) Address Resolution Protocol ONLY required for some network interfaces (see 'net_arp.h
*               Note #1').
*
*           (2) See 'net_arp.h  ARP HARDWARE & PROTOCOL DEFINES  Note #1' for supported ARP hardware &
*               protocol types.
*
*           (3) Configure NET_ARP_CFG_ADDR_FLTR_EN to enable/disable the ARP address filtering feature
*               to selectively filter & discard ALL misdirected or incorrectly received ARP messages.
*********************************************************************************************************
*/
                                                                /* Configure ARP types (see Note #2) :                  */
#define  NET_ARP_CFG_HW_TYPE                    NET_ARP_HW_TYPE_ETHER
#define  NET_ARP_CFG_PROTOCOL_TYPE              NET_ARP_PROTOCOL_TYPE_IP_V4

#define  NET_ARP_CFG_NBR_CACHE                             3u   /* Configure ARP cache size.                            */

                                                                /* Configure ARP address filter feature (see Note #3) : */
#define  NET_ARP_CFG_ADDR_FLTR_EN               DEF_ENABLED
                                                                /*   DEF_DISABLED    ARP addresses NOT filtered         */
                                                                /*   DEF_ENABLED     ARP addresses     filtered         */


/*$PAGE*/
/*
*********************************************************************************************************
*                                 NETWORK MANAGER LAYER CONFIGURATION
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                INTERNET PROTOCOL LAYER CONFIGURATION
*
* Note(s) : (1) Configure NET_IP_CFG_MULTICAST_SEL with the desired configuration for IP multicasting :
*
*                   NET_IP_MULTICAST_SEL_NONE       NO multicasting
*                   NET_IP_MULTICAST_SEL_TX         Transmit multicasting ONLY
*                   NET_IP_MULTICAST_SEL_TX_RX      Transmit & receive multicasting, with IGMP support
*
*               See also 'net_igmp.c  Note #1'.
*********************************************************************************************************
*/

#define  NET_IP_CFG_IF_MAX_NBR_ADDR                        1u   /* Configure maximum number of addresses per interface. */

                                                                /* Configure multicast support (see Note #1) :          */
#define  NET_IP_CFG_MULTICAST_SEL               NET_IP_MULTICAST_SEL_NONE
                                                                /*   NET_IP_MULTICAST_SEL_NONE    NO       multicasting */
                                                                /*   NET_IP_MULTICAST_SEL_TX      Transmit multicasting */
                                                                /*                                  ONLY                */
                                                                /*   NET_IP_MULTICAST_SEL_TX_RX   Transmit & receive    */
                                                                /*                                  multicasting        */


/*
*********************************************************************************************************
*                        INTERNET CONTROL MESSAGE PROTOCOL LAYER CONFIGURATION
*********************************************************************************************************
*/

                                                                /* Configure ICMP Transmit Source Quench feature :      */
#define  NET_ICMP_CFG_TX_SRC_QUENCH_EN          DEF_DISABLED
                                                                /*   DEF_DISABLED  ICMP Transmit Source Quench DISABLED */
                                                                /*   DEF_ENABLED   ICMP Transmit Source Quench ENABLED  */

#define  NET_ICMP_CFG_TX_SRC_QUENCH_NBR                    5u   /* Configure ICMP transmit source quench list size.     */


/*
*********************************************************************************************************
*                        INTERNET GROUP MANAGEMENT PROTOCOL LAYER CONFIGURATION
*********************************************************************************************************
*/

#define  NET_IGMP_CFG_MAX_NBR_HOST_GRP                     2u   /* Configure maximum number of IGMP host groups.        */


/*$PAGE*/
/*
*********************************************************************************************************
*                                    TRANSPORT LAYER CONFIGURATION
*********************************************************************************************************
*/
                                                                /* Configure Transport Layer Selection :                */
#define  NET_CFG_TRANSPORT_LAYER_SEL            NET_TRANSPORT_LAYER_SEL_UDP_TCP
                                                                /*   NET_TRANSPORT_LAYER_SEL_UDP       UDP     ONLY     */
                                                                /*   NET_TRANSPORT_LAYER_SEL_UDP_TCP   UDP/TCP          */


/*
*********************************************************************************************************
*                             USER DATAGRAM PROTOCOL LAYER CONFIGURATION
*
* Note(s) : (1) Configure NET_UDP_CFG_APP_API_SEL with the desired configuration for demultiplexing
*               UDP datagrams to application connections :
*
*                   NET_UDP_APP_API_SEL_SOCK        Demultiplex UDP datagrams to BSD sockets ONLY.
*                   NET_UDP_APP_API_SEL_APP         Demultiplex UDP datagrams to application-specific
*                                                       connections ONLY.
*                   NET_UDP_APP_API_SEL_SOCK_APP    Demultiplex UDP datagrams to BSD sockets first;
*                                                       if NO socket connection found to demultiplex
*                                                       a UDP datagram, demultiplex to application-
*                                                       specific connection.
*
*               See also 'net_udp.c  NetUDP_RxPktDemuxDatagram()  Note #1'
*                      & 'net_udp.c  NetUDP_RxPktDemuxAppData()   Note #1'.
*
*           (2) (a) RFC #1122, Section 4.1.3.4 states that "an application MAY optionally ... discard
*                   ... [or allow] ... received ... UDP datagrams without checksums".
*
*               (b) Configure NET_UDP_CFG_RX_CHK_SUM_DISCARD_EN to enable/disable discarding of UDP
*                   datagrams received with NO computed check-sum :
*
*                   (1) When ENABLED,  ALL UDP datagrams received without a check-sum are discarded.
*
*                   (2) When DISABLED, ALL UDP datagrams received without a check-sum are flagged so
*                       that application(s) may handle &/or discard.
*
*               See also 'net_udp.c  NetUDP_RxPktValidate()  Note #4d3A'.
*
*           (3) (a) RFC #1122, Section 4.1.3.4 states that "an application MAY optionally be able to
*                   control whether a UDP checksum will be generated".
*
*               (b) Configure NET_UDP_CFG_TX_CHK_SUM_EN to enable/disable transmitting UDP datagrams
*                   with check-sums :
*
*                   (1) When ENABLED,  ALL UDP datagrams are transmitted with    a computed check-sum.
*
*                   (2) When DISABLED, ALL UDP datagrams are transmitted without a computed check-sum.
*
*               See also 'net_udp.c  NetUDP_TxPktPrepareHdr()  Note #3b'.
*********************************************************************************************************
*/
                                                                /* Configure UDP Receive Demultiplex Selection ...      */
                                                                /* ... (see Note #1) :                                  */
#define  NET_UDP_CFG_APP_API_SEL                NET_UDP_APP_API_SEL_SOCK
                                                                /*                       Demultiplex & Receive via ...  */
                                                                /*   NET_UDP_APP_API_SEL_SOCK      Sockets     ONLY     */
                                                                /*   NET_UDP_APP_API_SEL_APP       Application ONLY     */
                                                                /*   NET_UDP_APP_API_SEL_SOCK_APP  First by Socket,     */
                                                                /*                                 Next  by Application */

                                                                /* Configure UDP Receive Check-Sum Discard feature ...  */
                                                                /* ... (see Note #2b) :                                 */
#define  NET_UDP_CFG_RX_CHK_SUM_DISCARD_EN      DEF_DISABLED
                                                                /*   DEF_DISABLED  UDP Datagrams  Received without ...  */
                                                                /*                     Check-Sums Validated             */
                                                                /*   DEF_ENABLED   UDP Datagrams  Received without ...  */
                                                                /*                     Check-Sums Discarded             */

                                                                /* Configure UDP Transmit Check-Sum feature ...         */
                                                                /* ... (see Note #3b) :                                 */
#define  NET_UDP_CFG_TX_CHK_SUM_EN              DEF_ENABLED
                                                                /*   DEF_DISABLED  Transmit Check-Sums  DISABLED        */
                                                                /*   DEF_ENABLED   Transmit Check-Sums  ENABLED         */


/*$PAGE*/
/*
*********************************************************************************************************
*                          TRANSMISSION CONTROL PROTOCOL LAYER CONFIGURATION
*
* Note(s) : (1) (a) Configure NET_TCP_CFG_NBR_CONN with the desired number of TCP connection objects.
*
*               (b) Since TCP requires network sockets (see 'net_tcp.h  Note #2'), the configured number
*                   of sockets MUST be greater than or equal to the configured number of TCP connections.
*
*               See also 'net_sock.h  MODULE  Note #1',
*                        'net_tcp.h   MODULE  Note #1',
*                      & 'net_cfg_net.h  NETWORK SOCKET LAYER CONFIGURATION  Note #4'.
*
*           (2) Stevens, TCP/IP Illustrated, Volume 1, 8th Printing, Section 20.4, Page 282 states that
*               "4.2BSD defaulted the send and receive buffer" (i.e. socket buffer/TCP window) "to 2048
*               bytes each.  With 4.3BSD both were increased to 4096 bytes ... Other systems, such as
*               ... 4.4BSD ... use larger default buffer sizes, such as 8192 or 16384 bytes.
*
*               The common default of 4096 bytes ... is not optimal for Ethernet.  An approximate 40%
*               increase in throughput is seen by just increasing both buffers to 16384 bytes."
*
*           (3) RFC #793, Section 3.3 'Sequence Numbers : Knowing When to Keep Quiet' states that
*               "the Maximum Segment Lifetime (MSL) is ... to be 2 minutes ... [but] may be changed
*               if experience indicates it is desirable to do so".
*
*               See also 'net_tcp.h  TCP CONNECTION TIMEOUT DEFINES  Note #1'.
*
*           (4) RFC #2581, Section 4.2 states that "an ACK ... MUST be generated within 500 ms of the
*               arrival of the first unacknowledged packet".
*
*               See also 'net_tcp.h  TCP CONGESTION CONTROL DEFINES  Note #6b'
*                      & 'net_tcp.c  NetTCP_TxConnAck()  Note #6a2'.
*
*           (5) Configure timeout values in integer number of milliseconds.  Timeout values may also
*               be configured with network time constant, NET_TMR_TIME_INFINITE, to never timeout.
*********************************************************************************************************
*/

#define  NET_TCP_CFG_NBR_CONN                              3    /* Configure total number of TCP connections.           */


                                                                /* Configure TCP connections' window sizes ...          */
                                                                /* ... in integer number of octets (see Note #2) :      */

#define  NET_TCP_CFG_RX_WIN_SIZE_OCTET            (7u * 1460u)  /* Configure TCP connection receive  window size.       */
#define  NET_TCP_CFG_TX_WIN_SIZE_OCTET            (3u * 1460u)  /* Configure TCP connection transmit window size.       */


                                                                /* Configure TCP connections' default maximum ...       */
                                                                /* ... segment lifetime timeout (MSL) value,  ...       */
#define  NET_TCP_CFG_TIMEOUT_CONN_MAX_SEG_SEC              3u   /* ... in integer seconds (see Note #3).                */


                                                                /* Configure TCP acknowledgement delay ...              */
#define  NET_TCP_CFG_TIMEOUT_CONN_ACK_DLY_MS             500u   /* ... in integer milliseconds (see Note #4).           */


                                                                /* Configure TCP timeouts (see Note #5) :               */
                                                                /* Configure TCP connection receive  queue timeout.     */
#define  NET_TCP_CFG_TIMEOUT_CONN_RX_Q_MS       NET_TMR_TIME_INFINITE
                                                                /* Configure TCP connection transmit queue timeout.     */
#define  NET_TCP_CFG_TIMEOUT_CONN_TX_Q_MS       NET_TMR_TIME_INFINITE


/*$PAGE*/
/*
*********************************************************************************************************
*                                 NETWORK SOCKET LAYER CONFIGURATION
*
* Note(s) : (1) Network Socket Layer NOT required for all application interfaces (see 'net_sock.h
*               MODULE  Note #1').
*
*           (2) See 'net_sock.h  NETWORK SOCKET FAMILY & PROTOCOL DEFINES  Note #1' for supported
*               Network Socket Family types.
*
*           (3) Configure socket select maximum number of socket events/operations to wait on.
*
*               See 'net_sock.c  NetSock_Sel()         Note #4b'
*                 & 'net_os.c    NetOS_Sock_SelWait()  Note #2b2'.
*
*           (4) Configure timeout values in integer number of milliseconds.  Timeout values may also
*               be configured with network time constant, NET_TMR_TIME_INFINITE, to never timeout.
*********************************************************************************************************
*/
                                                                /* Configure socket family type (see Note #2) :         */
#define  NET_SOCK_CFG_FAMILY                    NET_SOCK_FAMILY_IP_V4

#define  NET_SOCK_CFG_NBR_SOCK                             3    /* Configure total number of sockets.                   */


                                                                /* Configure socket default blocking behavior :         */
#define  NET_SOCK_CFG_BLOCK_SEL                 NET_SOCK_BLOCK_SEL_BLOCK
                                                                /*   NET_SOCK_BLOCK_SEL_DFLT /                          */
                                                                /*   NET_SOCK_BLOCK_SEL_BLOCK      Blocking ENABLED     */
                                                                /*   NET_SOCK_BLOCK_SEL_NO_BLOCK   Blocking DISABLED    */


                                                                /* Configure socket select functionality :              */
#define  NET_SOCK_CFG_SEL_EN                    DEF_DISABLED
                                                                /*   DEF_DISABLED  Socket select  DISABLED              */
                                                                /*   DEF_ENABLED   Socket select  ENABLED               */

                                                                /* Configure maximum number of socket select ...        */
#define  NET_SOCK_CFG_SEL_NBR_EVENTS_MAX                  10u   /* ... operations/events to wait on (see Note #3).      */


                                                                /* Configure stream-type sockets' accept queue ...      */
#define  NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX               5u   /* ... maximum size.                                    */

#define  NET_SOCK_CFG_PORT_NBR_RANDOM_BASE             65000u   /* Configure random ports' starting port number.        */


                                                                /* Configure socket timeout values (see Note #4) :      */
                                                                /* Configure socket receive    queue   timeout.         */
#define  NET_SOCK_CFG_TIMEOUT_RX_Q_MS           NET_TMR_TIME_INFINITE
                                                                /* Configure socket connection request timeout.         */
#define  NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS       NET_TMR_TIME_INFINITE
                                                                /* Configure socket connection accept  timeout.         */
#define  NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS    NET_TMR_TIME_INFINITE
                                                                /* Configure socket connection close   timeout.         */
#define  NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS            10000u


/*$PAGE*/
/*
*********************************************************************************************************
*                               NETWORK SECURITY MANAGER CONFIGURATION
*
* Note(s) : (1) Configure NET_SECURE_CFG_EN    to enable/disable network security management.
*
*           (2) Configure NET_SECURE_CFG_FS_EN to enable/disable network security management file system 
*               functionality.
*
*           (3) Configure NET_SECURE_CFG_WORD_SIZE for optimized word size for security port, if applicable.
*
*           (4) (a) Configure NET_SECURE_CFG_CLIENT_DOWNGRADE_EN to enable/disable client downgrade option.
*
*                   If NET_SECURE_CFG_CLIENT_DOWNGRADE_EN is DEF_ENABLED, client applications will be
*                   allowed to connect on server that is using a SSL/TLS version older than 
*                   NET_SECURE_CFG_VER.
*
*                   It is recommended to configure NET_SECURE_CFG_CLIENT_DOWNGRADE_EN to DEF_DISABLED.  
*                   If a client is running SSL V3.0, he MIGHT NOT want to exchange secure information 
*                   with a server  running SSL V2.0 because there is important security leakage in 
*                   that version of the protocol.
*
*               (b) Configure NET_SECURE_CFG_SERVER_DOWNGRADE_EN to enable/disable server downgrade option.
*
*                   If NET_SECURE_CFG_SERVER_DOWNGRADE_EN is DEF_ENABLED, server applications will be 
*                   able to accept connection requests coming from clients that are using a SSL/TLS 
*                   version older than NET_SECURE_CFG_VER.
*
*                   It is recommended to configure NET_SECURE_CFG_CLIENT_DOWNGRADE_EN to DEF_ENABLED 
*                   since the client is responsible of choosing a SSL/TLS protocol version that matches 
*                   its security requirements.
*
*           (5) The common name is chosen during the creation of the certificate (e.g. 'Micrium',
*               'Google', 'Paypal', etc.).
*
*           (6) The public key is a part of the public key certificate.  The length of the public
*               key is chosen during the creation of the certificate.
*********************************************************************************************************
*/
/*$PAGE*/
                                                                /* Configure Network Security Manager (see Note #1) :   */
#define  NET_SECURE_CFG_EN                      DEF_DISABLED
                                                                /*   DEF_DISABLED   Network security manager DISABLED   */
                                                                /*   DEF_ENABLED    Network security manager ENABLED    */

                                                                /* Configure File System feature(s) [see Note #2] :     */
#define  NET_SECURE_CFG_FS_EN                   DEF_DISABLED
                                                                /*   DEF_DISABLED   File System feature(s) DISABLED     */
                                                                /*   DEF_ENABLED    File System feature(s) ENABLED      */


                                                                /* Configure secure protocol version :                  */
#define  NET_SECURE_CFG_VER                     NET_SECURE_SSL_V3_0
                                                                /*   NET_SECURE_SSL_V2_0   SSL V2.0                     */
                                                                /*   NET_SECURE_SSL_V3_0   SSL V3.0                     */
                                                                /*   NET_SECURE_TLS_V1_0   TLS V1.0                     */
                                                                /*   NET_SECURE_TLS_V1_1   TLS V1.1                     */
                                                                /*   NET_SECURE_TLS_V1_2   TLS V1.2                     */

                                                                /* Configure network security word size (see Note #3) : */
#define  NET_SECURE_CFG_WORD_SIZE               CPU_WORD_SIZE_32
                                                                /*   CPU_WORD_SIZE_08    8-bit word size                */
                                                                /*   CPU_WORD_SIZE_16   16-bit word size                */
                                                                /*   CPU_WORD_SIZE_32   32-bit word size                */
                                                                /*   CPU_WORD_SIZE_64   64-bit word size                */


                                                                /* Configure client downgrade option (see Note #4a) :   */
#define  NET_SECURE_CFG_CLIENT_DOWNGRADE_EN     DEF_DISABLED
                                                                /*   DEF_DISABLED   Client downgrade DISABLED           */
                                                                /*   DEF_ENABLED    Client downgrade ENABLED            */

                                                                /* Configure server downgrade option (see Note #4b) :   */
#define  NET_SECURE_CFG_SERVER_DOWNGRADE_EN     DEF_ENABLED
                                                                /*   DEF_DISABLED   Server downgrade DISABLED           */
                                                                /*   DEF_ENABLED    Server downgrade ENABLED            */


#define  NET_SECURE_CFG_MAX_NBR_SOCK                       5u   /* Configure maximum number of sockets to secure.       */

                                                                /* Configure maximum number of certificate authorities  */
#define  NET_SECURE_CFG_MAX_NBR_CA                         1u   /* ... that can be installed.                           */


                                                                /* Configure maximum length (in octets) of ...          */
#define  NET_SECURE_CFG_MAX_KEY_LEN                     1500u   /* ... certificate authority, certificates & keys.      */

                                                                /* Configure maximum length (in octets) of common name  */
#define  NET_SECURE_CFG_MAX_ISSUER_CN_LEN                 20u   /* ... (see Note #5).                                   */

                                                                /* Configure maximum length (in octets) of public key   */
#define  NET_SECURE_CFG_MAX_PUBLIC_KEY_LEN               256u   /* ... (see Note #6).                                   */


/*$PAGE*/
/*
*********************************************************************************************************
*                                     BSD 4.x LAYER CONFIGURATION
*
* Note(s) : (1) BSD 4.x Layer API NOT required for all applications (see 'net_bsd.h  MODULE  Note #1').
*********************************************************************************************************
*/
                                                                /* Configure BSD 4.x Layer API (see Note #1) :          */
#define  NET_BSD_CFG_API_EN                     DEF_DISABLED
                                                                /*   DEF_DISABLED  BSD 4.x Layer API DISABLED           */
                                                                /*   DEF_ENABLED   BSD 4.x Layer API ENABLED            */


/*
*********************************************************************************************************
*                 NETWORK APPLICATION PROGRAMMING INTERFACE (API) LAYER CONFIGURATION
*
* Note(s) : (1) Network Application Programming Interface (API) Layer NOT required for all applications
*               (see 'net_app.h  MODULE  Note #1').
*********************************************************************************************************
*/
                                                                /* Configure Network API Layer (see Note #1) :          */
#define  NET_APP_CFG_API_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED  Network API Layer DISABLED           */
                                                                /*   DEF_ENABLED   Network API Layer ENABLED            */


/*$PAGE*/
/*
*********************************************************************************************************
*                             NETWORK CONNECTION MANAGEMENT CONFIGURATION
*
* Note(s) : (1) Network Connection Management NOT required for all application interfaces (see 'net_conn.h
*               MODULE  Note #1').
*
*           (2) See 'net_def.h  NETWORK CONNECTION MANAGEMENT DEFINES' for supported Family types.
*
*           (3) The configured number of connections MUST be greater than the configured/required/expected
*               number of application connections & transport layer connections.
*
*               See also 'net_cfg_net.h  NETWORK CONNECTION MANAGEMENT CONFIGURATION  Note #4'.
*********************************************************************************************************
*/
                                                                /* Configure connection family type      (see Note #2). */
#define  NET_CONN_CFG_FAMILY                    NET_CONN_FAMILY_IP_V4_SOCK

#define  NET_CONN_CFG_NBR_CONN                             6    /* Configure total number of connections (see Note #3). */

