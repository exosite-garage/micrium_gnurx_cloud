/*
*********************************************************************************************************
*                                             uC/DHCPc V2
*                              Dynamic Host Configuration Protocol Client
*
*                          (c) Copyright 2004-2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/DHCPc is provided in source form to registered licensees ONLY.  It is
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
*                                   DHCP CLIENT CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : dhcp-c_cfg.h
* Version       : V2.08.02
* Programmer(s) : SR
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           TASKS PRIORITIES
*********************************************************************************************************
*/

#define  DHCPc_OS_CFG_TASK_PRIO                           13
#define  DHCPc_OS_CFG_TMR_TASK_PRIO                       14


/*
*********************************************************************************************************
*                                              STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  DHCPc_OS_CFG_TASK_STK_SIZE                      640
#define  DHCPc_OS_CFG_TMR_TASK_STK_SIZE                  256


/*$PAGE*/
/*
*********************************************************************************************************
*                                                 DHCPc
*
* Note(s) : (1) Default port for DHCP server is 67, and default port for DHCP client is 68.
*
*           (2) Configure DHCPc_CFG_BROADCAST_BIT_EN to DEF_ENABLED to instruct the contacted DHCP server
*               to use broadcast packets instead of unicast ones.  Used for TCP/IP stacks that cannot
*               receive unicast packets when not fully configured.  This is the case of the uC/TCPIP
*               stack, so this define MUST be set to DEF_ENABLED when this DHCP client is used with the
*               Micrium's uC/TCP-IP stack.
*
*           (3) Configure DHCPc_CFG_MAX_NBR_IF to the maximum number of interface this DHCP client will
*               be able to manage at a given time.
*
*           (4) Once the DHCP server has assigned the client an address, the later may perform a final
*               check prior to use this address in order to make sure it is not being used by another
*               host on the network.
*********************************************************************************************************
*/

#define  DHCPc_CFG_IP_PORT_SERVER                         67    /* Configure DHCP server port            (see Note #1). */
#define  DHCPc_CFG_IP_PORT_CLIENT                         68    /* Configure DHCP client port            (see Note #1). */

#define  DHCPc_CFG_MAX_RX_TIMEOUT_MS                    2000    /* Maximum inactivity time (ms) on receive.             */

#define  DHCPc_CFG_BROADCAST_BIT_EN              DEF_ENABLED    /* Configure broadcast bit               (see Note #2) :*/
                                                                /*   DEF_DISABLED  Broadcast bit NOT set                */
                                                                /*   DEF_ENABLED   Broadcast bit     set                */

#define  DHCPc_CFG_PARAM_REQ_TBL_SIZE                      5    /* Configure requested parameter table size.            */

#define  DHCPc_CFG_MAX_NBR_IF                              1    /* Configure maximum number of interface (see Note #3). */

#define  DHCPc_CFG_ADDR_VALIDATE_EN              DEF_ENABLED    /* Configure final check on assigned address ...        */
                                                                /* ... (see Note #4) :                                  */
                                                                /*   DEF_DISABLED  Validation NOT performed             */
                                                                /*   DEF_ENABLED   Validation     performed             */

#define  DHCPc_CFG_DYN_LOCAL_LINK_ADDR_EN        DEF_ENABLED    /* Configure dynamic link-local address configuration : */
                                                                /*   DEF_DISABLED  local-link configuration DISABLED    */
                                                                /*   DEF_ENABLED   local-link configuration ENABLED     */

#define  DHCPc_CFG_LOCAL_LINK_MAX_RETRY                    2    /* Configure maximum number of retry to get a           */
                                                                /* link-local address.                                  */


/*
*********************************************************************************************************
*                                 DHCPc ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure DHCPc_CFG_ARG_CHK_EXT_EN to enable/disable the DHCP client external argument
*               check feature :
*
*               (a) When ENABLED,  ALL arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*               (b) When DISABLED, NO  arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*           (2) Configure DHCPc_CFG_ARG_CHK_DBG_EN to enable/disable the DHCP client internal debug
*               argument check feature :
*
*               (a) When ENABLED,     internal arguments are checked/validated to debug the DHCP client.
*
*               (b) When DISABLED, NO internal arguments are checked/validated to debug the DHCP client.
*
*           (3) Configure DHCPc_DBG_CFG_MEM_CLR_EN to enable/disable the DHCP client from clearing
*               internal data structure memory buffers; a convenient feature while debugging.
*********************************************************************************************************
*/
                                                                /* Configure external argument check feature ...        */
                                                                /* ... (see Note #1) :                                  */
#define  DHCPc_CFG_ARG_CHK_EXT_EN                DEF_ENABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */

                                                                /* Configure internal argument check feature ...        */
                                                                /* ... (see Note #2) :                                  */
#define  DHCPc_CFG_ARG_CHK_DBG_EN                DEF_DISABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */

                                                                /* Configure memory clear feature  (see Note #3) :      */
#define  DHCPc_DBG_CFG_MEM_CLR_EN                DEF_ENABLED
                                                                /*   DEF_DISABLED     Data structure clears DISABLED    */
                                                                /*   DEF_ENABLED      Data structure clears ENABLED     */

