/*
*********************************************************************************************************
*                                              uC/HTTPs
*                                 Hypertext Transfer Protocol (server)
*
*                          (c) Copyright 2004-2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/HTTPs is provided in source form to registered licensees ONLY.  It is
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
*                                   HTTP SERVER CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : http-s_cfg.h
* Version       : V1.97.00
* Programmer(s) : JDH
*                 SR
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                                 HTTPs
*
* Note(s) : (1) #### These configuration defines are not yet being used in the current implementation.
*********************************************************************************************************
*/

#define  HTTPs_CFG_IPPORT                                 80    /* HTTPs        IP port. Default is 80.                 */
#define  HTTPs_CFG_IPPORT_SECURE                         443    /* HTTPs Secure IP port. Default is 443.                */

                                                                /* See Note #1.                                         */
#define  HTTPs_CFG_MAX_ACCEPT_TIMEOUT_MS                5000    /* Maximum inactivity time (ms) on ACCEPT.              */
#define  HTTPs_CFG_MAX_RX_TIMEOUT_MS                    5000    /* Maximum inactivity time (ms) on RX.                  */
#define  HTTPs_CFG_MAX_TX_TIMEOUT_MS                    5000    /* Maximum inactivity time (ms) on TX.                  */

#define  HTTPs_CFG_MAX_ACCEPT_RETRY                        1    /* Maximum number of retries on ACCEPT.                 */
#define  HTTPs_CFG_MAX_RX_RETRY                            3    /* Maximum number of retries on RX.                     */
#define  HTTPs_CFG_MAX_TX_RETRY                            3    /* Maximum number of retries on TX.                     */

#define  HTTPs_CFG_FILE_RD_BUF_LEN                       512    /* Length of buffer used to read file.                  */
#define  HTTPs_CFG_FILE_TX_BUF_LEN                       512    /* Length of buffer used to send file.                  */

#define  HTTPs_CFG_TOK_PARSE_EN                  DEF_ENABLED    /* Enable / disable token parsing (${}).                */
#define  HTTPs_CFG_TOK_PARSE_TBL_SIZE                    256    /* Size of parse table.  Minimum is 2.                  */

                                                                /* Root path for HTTP documents in filesystem.          */
#define  HTTPs_CFG_FS_ROOT                        ""            /* Filesystem-specific symbols can be used.             */
#define  HTTPs_CFG_DFLT_FILE                      "index.html"  /* Default file to load if no filename specified in URL.*/

                                                                /* Default HTML document returned when the requested    */
                                                                /* HTML document is not found (HTTP error #404).        */
#define  HTTPs_CFG_ERR_MSG_HTML_NOT_FOUND         "<HTML>\r\n" \
                                                  "<BODY>\r\n" \
                                                  "<HEAD><TITLE>SYSTEM ERROR</TITLE></HEAD>\r\n" \
                                                  "<H1>NOT FOUND</H1>\r\n" \
                                                  "The requested object does not exist on this server.\r\n" \
                                                  "</BODY>\r\n" \
                                                  "</HTML>\r\n"


/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                   0
#define  TRACE_LEVEL_INFO                                  1
#define  TRACE_LEVEL_DBG                                   2

#define  HTTPs_TRACE_LEVEL                   TRACE_LEVEL_OFF
#define  HTTPs_TRACE                                  (void)

