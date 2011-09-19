/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
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
*                                      HTTP SERVER EXAMPLE FILE
*
*                                              TEMPLATE
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JJL
*                 AA
*                 SR
*********************************************************************************************************
* Note(s)       : (1) Assumes the following versions (or more recent) of software modules are included in
*                     the project build :
*
*                     (a) uC/TCP-IP V2.10
*                     (b) uC/OS-III V3.01.3
*                     (c) uC/LIB    V1.31
*                     (d) uC/CPU    V1.25
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include  <webpages.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  HTTPs_VAL_REQ_BUF_LEN          20u

#define  HTML_LED_INPUT_NAME           "LED"
#define  HTML_LED1_TOGGLE_INPUT_VALUE  "LED1"
#define  HTML_LED2_TOGGLE_INPUT_VALUE  "LED2"
#define  LED1                          15u
#define  LED2                          4u


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_INT16S   AppTempSensorDegC;
static  CPU_INT16S   AppTempSensorDegF;


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  APP_TASK_STOP();                             { while (DEF_ON) { \
                                                            ;            \
                                                        }                \
                                                      }


#define  APP_TEST_FAULT(err_var, err_code)            { if ((err_var) != (err_code)) {   \
                                                            APP_TASK_STOP();             \
                                                        }                                \
                                                      }


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  App_TempSensorUpdate(void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                            AppHTTPs_Init
*
* Description : Initialize HTTP server.
*
* Arguments   : none.
*
* Returns     : none.
*
* Notes       : none.
*********************************************************************************************************
*/

void  AppHTTPs_Init (void)
{
    CPU_BOOLEAN  cfg_success;


    cfg_success = HTTPs_Init(DEF_NO);
    APP_TEST_FAULT(cfg_success, DEF_OK);


    cfg_success = Apps_FS_Init();
    APP_TEST_FAULT(cfg_success, DEF_OK);

    cfg_success = Apps_FS_AddFile((CPU_CHAR *)&STATIC_INDEX_HTML_NAME,
                                  (CPU_CHAR *)&Index_html,
                                  (CPU_INT32U) STATIC_INDEX_HTML_LEN);
    APP_TEST_FAULT(cfg_success, DEF_OK);


    cfg_success = Apps_FS_AddFile((CPU_CHAR *)&STATIC_LOGO_GIF_NAME,
                                  (CPU_CHAR *)&Logo_Gif,
                                  (CPU_INT32U) STATIC_LOGO_GIF_LEN);
    APP_TEST_FAULT(cfg_success, DEF_OK);


    BSP_Temp_Init();
    App_TempSensorUpdate();
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       App_TempSensorUpdate()
*
* Description : Monitor temperature sensor.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : AppTaskStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TempSensorUpdate (void)
{
    static  CPU_INT16S  temp_sensor = 0;


    AppTempSensorDegC = (CPU_INT16S)BSP_Temp_Rd();
    AppTempSensorDegF = (AppTempSensorDegC * 9u / 5u) + 32u;

    temp_sensor++;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           HTTPs_ValReq()
*
* Description : Callback function handling dynamic variable substitution from HTML file.
*
* Argument(s) : p_tok       Pointer to string containing the name of the HTML embedded token.
*
*               p_val       Pointer to string that will receive the value of the HTML embedded token.
*
* Return(s)   : DEF_OK,   if value of token returned successfully.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : HTTPs_HTML_FileTokParse().
*
* Note(s)     : (1) This is a callback function that MUST be implemented in your application.
*
*               (2) 'p_val' returned by HTTPs_ValReq() MUST reference a global array in order to prevent 
*                   the caller from accessing an out-of-scope array defined within HTTPs_ValReq().
*********************************************************************************************************
*/

CPU_BOOLEAN  HTTPs_ValReq (CPU_CHAR   *p_tok,
                           CPU_CHAR  **p_val)
{
    static  CPU_CHAR    val_buf[HTTPs_VAL_REQ_BUF_LEN];
#if   (LIB_VERSION >= 126u)
            CPU_INT32U  ver;
#elif (LIB_STR_CFG_FP_EN == DEF_ENABLED)
            CPU_FP32    ver;
#endif
            OS_TICK     os_time_tick;
#if (LIB_STR_CFG_FP_EN == DEF_ENABLED)
            CPU_FP32    os_time_sec;
#else
            CPU_INT32U  os_time_sec;
            CPU_INT32U  os_time_ms;
            CPU_SIZE_T  os_time_len;
#endif
            OS_ERR      os_err;


   (void)Str_Copy(&val_buf[0], "%%%%%%%%");
   *p_val = &val_buf[0];


                                                                /* --------------------- OS VALUES -------------------- */
    if (Str_Cmp(p_tok, "OS_VERSION")               == 0) {
#if (LIB_VERSION >=   126u)
#if (OS_VERSION  >= 30200u)
        ver =  OS_VERSION / 10000;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &val_buf[0]);
        val_buf[2] = '.';

        ver = (OS_VERSION /   100) % 100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &val_buf[3]);
        val_buf[5] = '.';

        ver = (OS_VERSION /     1) %  10;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[6]);
        val_buf[8] = '\0';

#elif (OS_VERSION >   300u)
        ver =  OS_VERSION / 1000;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &val_buf[0]);
        val_buf[2] = '.';

        ver = (OS_VERSION /   10) % 100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &val_buf[3]);
        val_buf[5] = '.';

        ver = (OS_VERSION /    1) %  10;
       (void)Str_FmtNbr_Int32U(ver,   1, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[6]);
        val_buf[8] = '\0';

#else
        ver =  OS_VERSION /  100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &val_buf[0]);
        val_buf[2] = '.';

        ver = (OS_VERSION /    1) % 100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[3]);
        val_buf[5] = '\0';
#endif

#elif (LIB_STR_CFG_FP_EN == DEF_ENABLED)
#if   (OS_VERSION > 30200u)
        ver = (CPU_FP32)OS_VERSION / 10000;
       (void)Str_FmtNbr_32(ver,  2,  2, ' ',  DEF_NO,  &val_buf[0]);

        ver = (CPU_FP32)OS_VERSION /   100;
       (void)Str_FmtNbr_32(ver,  0,  2, '\0', DEF_YES, &val_buf[6]);

#elif (OS_VERSION >   300u)
        ver = (CPU_FP32)OS_VERSION / 1000;
       (void)Str_FmtNbr_32(ver,  2,  2, ' ',  DEF_NO,  &val_buf[0]);

        ver = (CPU_FP32)OS_VERSION /   10;
       (void)Str_FmtNbr_32(ver,  0,  1, '\0', DEF_YES, &val_buf[6]);

#else
        ver = (CPU_FP32)OS_VERSION / 100;
       (void)Str_FmtNbr_32(ver,  2,  2, '\0', DEF_YES, &val_buf[0]);
#endif
#endif


    }  else if (Str_Cmp(p_tok, "OS_TIME"         ) == 0) {
        os_time_tick = (OS_TICK )OSTimeGet(&os_err);
#if (LIB_STR_CFG_FP_EN == DEF_ENABLED)        
        os_time_sec  = (CPU_FP32)os_time_tick / OS_CFG_TICK_RATE_HZ;
       (void)Str_FmtNbr_32(os_time_sec, 7u,  3u, '\0', DEF_YES, &val_buf[0]);
#else
        os_time_sec  = (CPU_INT32U)os_time_tick / OS_CFG_TICK_RATE_HZ;
       (void)Str_FmtNbr_Int32U(os_time_sec, 7u, DEF_NBR_BASE_DEC, '\0', DEF_NO, DEF_YES, &val_buf[0]);
       (void)Str_Cat(&val_buf[0], ".");
        os_time_len  =  Str_Len(&val_buf[0]);
        os_time_ms   = (CPU_INT32U)os_time_tick % OS_CFG_TICK_RATE_HZ;
        os_time_ms  *=  1000 / OS_CFG_TICK_RATE_HZ;
       (void)Str_FmtNbr_Int32U(os_time_ms, 3u, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[os_time_len]);
#endif


/*$PAGE*/
                                                                /* ----------- NETWORK PROTOCOL SUITE VALUES ---------- */
    } else if (Str_Cmp(p_tok, "NET_VERSION") == 0) {
#if (LIB_VERSION >= 126u)
#if (NET_VERSION >  205u)
        ver =  NET_VERSION / 10000;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &val_buf[0]);
        val_buf[2] = '.';

        ver = (NET_VERSION /   100) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &val_buf[3]);
        val_buf[5] = '.';

        ver = (NET_VERSION /     1) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[6]);
        val_buf[8] = '\0';

#else
        ver =  NET_VERSION /   100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &val_buf[0]);
        val_buf[2] = '.';

        ver = (NET_VERSION /     1) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &val_buf[3]);
        val_buf[5] = '\0';
#endif

#elif (LIB_STR_CFG_FP_EN == DEF_ENABLED)
#if   (NET_VERSION > 205u)
        ver = (CPU_FP32)NET_VERSION / 10000;
       (void)Str_FmtNbr_32(ver,  2,  2, ' ',  DEF_NO,  &val_buf[0]);

        ver = (CPU_FP32)NET_VERSION /   100;
       (void)Str_FmtNbr_32(ver,  0,  2, '\0', DEF_YES, &val_buf[6]);

#else
        ver = (CPU_FP32)NET_VERSION /   100;
       (void)Str_FmtNbr_32(ver,  2,  2, '\0', DEF_YES, &val_buf[0]);
#endif
#endif

                                                                /* ---------------- APPLICATION VALUES ---------------- */
    } else if (Str_Cmp(p_tok, "TEMP_C") == 0) {
       (void)Str_FmtNbr_Int32S(AppTempSensorDegC, 3, DEF_NBR_BASE_DEC, '\0', DEF_NO, DEF_YES, &val_buf[0]);

    } else if (Str_Cmp(p_tok, "TEMP_F") == 0) {
       (void)Str_FmtNbr_Int32S(AppTempSensorDegF, 3, DEF_NBR_BASE_DEC, '\0', DEF_NO, DEF_YES, &val_buf[0]);
    }


    if ((Str_Cmp(p_tok, "TEMP_C") == 0) ||                      /* Update temperature values.                           */
        (Str_Cmp(p_tok, "TEMP_F") == 0)) {
        App_TempSensorUpdate();
    }

    return DEF_OK;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            HTTPs_ValRx()
*
* Description : Callback function handling POST action for every name-value pair received.
*
* Argument(s) : p_var       Pointer to string containing the name  of the HTTP POST variable.
*
*               p_val       Pointer to string containing the value of the HTTP POST variable.
*
* Return(s)   : DEF_OK,   if value of token returned successfully.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : HTTPs_ProcessCGIList().
*
* Note(s)     : (1) This is a callback function that MUST be implemented in your application.
*********************************************************************************************************
*/

CPU_BOOLEAN  HTTPs_ValRx (CPU_CHAR  *p_var,
                          CPU_CHAR  *p_val)
{
    CPU_INT16U   cmp_str;
    CPU_BOOLEAN  ret_val;


    ret_val = DEF_FAIL;

    cmp_str = Str_Cmp((CPU_CHAR *)p_var,
                      (CPU_CHAR *)HTML_LED_INPUT_NAME);
    if (cmp_str == 0) {
        cmp_str = Str_Cmp((CPU_CHAR *)p_val,                    /* Toggle LED 1.                                        */
                          (CPU_CHAR *)HTML_LED1_TOGGLE_INPUT_VALUE);
        if (cmp_str == 0) {
            LED_Toggle(LED1);
            ret_val = DEF_OK;
        }

        cmp_str = Str_Cmp((CPU_CHAR *)p_val,                    /* Toggle LED 2.                                        */
                          (CPU_CHAR *)HTML_LED2_TOGGLE_INPUT_VALUE);
        if (cmp_str == 0) {
            LED_Toggle(LED2);
            ret_val = DEF_OK;
        }
    }


    return (ret_val);
}
