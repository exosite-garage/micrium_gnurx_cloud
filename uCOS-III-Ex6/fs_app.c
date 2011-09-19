/*
*********************************************************************************************************
*                                             uC/FS V4
*                                     The Embedded File System
*
*                         (c) Copyright 2008-2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved. Protected by international copyright laws.
*
*               uC/FS is provided in source form to registered licensees ONLY.  It is
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
*                               FILE SYSTEM APPLICATION INITIALIZATION
*
*                                              TEMPLATE
*
* Filename      : fs_app.c
* Version       : V4.04
* Programmer(s) : BAN
*                 AHFAI
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <fs_app.h>


/*
*********************************************************************************************************
*                                               ENABLE
*********************************************************************************************************
*/

#if (APP_CFG_FS_EN == DEF_ENABLED)


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (APP_CFG_FS_RAM_EN == DEF_ENABLED)
static  CPU_INT32U  App_FS_RAM_Disk[APP_CFG_FS_RAM_SEC_SIZE * APP_CFG_FS_RAM_NBR_SECS / 4];
#endif

static  const  FS_CFG  App_FS_Cfg = {
    APP_CFG_FS_DEV_CNT,             /* DevCnt           */
    APP_CFG_FS_VOL_CNT,             /* VolCnt           */
    APP_CFG_FS_FILE_CNT,            /* FileCnt          */
    APP_CFG_FS_DIR_CNT,             /* DirCnt           */
    APP_CFG_FS_BUF_CNT,             /* BufCnt           */
    APP_CFG_FS_DEV_DRV_CNT,         /* DevDrvCnt        */
    APP_CFG_FS_MAX_SEC_SIZE         /* MaxSecSize       */
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (APP_CFG_FS_MSC_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddMSC    (void);
#endif

#if (APP_CFG_FS_IDE_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddIDE    (void);
#endif

#if (APP_CFG_FS_NAND_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddNAND   (void);
#endif

#if (APP_CFG_FS_NOR_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddNOR    (void);
#endif

#if (APP_CFG_FS_RAM_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddRAM    (void);
#endif

#if (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddSD_Card(void);
#endif

#if (APP_CFG_FS_SD_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddSD_SPI (void);
#endif


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            App_FS_Init()
*
* Description : Initialize uC/FS.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if file system suite was initialized.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) MSC device/volumes will be opened/closed dynamically by the USB Host MSC notification
*                   callback.
*********************************************************************************************************
*/

CPU_BOOLEAN  App_FS_Init (void)
{
    FS_ERR       err;
    CPU_BOOLEAN  ok;


                                                                /* ---------------------- INIT FS --------------------- */
    APP_TRACE_DBG(("\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("=                        FS INITIALIZATION                        =\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("Initializing FS...\r\n"));
    err = FS_Init((FS_CFG *)&App_FS_Cfg);
    if (err != FS_ERR_NONE) {
        APP_TRACE_DBG(("...init failed w/err = %d\r\n\r\n", err));
        return (DEF_FAIL);
    }


#if (APP_CFG_FS_MSC_EN == DEF_ENABLED)                          /* ------------------ ADD MSC DRIVER ------------------ */
    ok = App_FS_AddMSC();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_IDE_EN == DEF_ENABLED)                          /* ---------------- ADD/OPEN IDE VOLUME --------------- */
    ok = App_FS_AddIDE();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_NAND_EN == DEF_ENABLED)                         /* ---------------- ADD/OPEN NAND VOLUME -------------- */
    ok = App_FS_AddNAND();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_NOR_EN == DEF_ENABLED)                          /* ---------------- ADD/OPEN NOR VOLUME --------------- */
    ok = App_FS_AddNOR();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_RAM_EN == DEF_ENABLED)                          /* ------------- ADD/OPEN RAM DISK VOLUME ------------- */
    ok = App_FS_AddRAM();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED)                      /* --------- ADD/OPEN SD/MMC (CARDMODE) VOLUME -------- */
    ok = App_FS_AddSD_Card();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif


#if (APP_CFG_FS_SD_EN == DEF_ENABLED)                           /* ----------- ADD/OPEN SD/MMC (SPI) VOLUME ----------- */
    ok = App_FS_AddSD_SPI();

    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
#endif

    APP_TRACE_DBG(("...init succeeded.\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("\r\n"));

    return (DEF_OK);
}

/*
*********************************************************************************************************
*                                           App_FS_AddMSC()
*
* Description : Add MSC driver.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
*********************************************************************************************************
*/

#if (APP_CFG_FS_MSC_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddMSC (void)
{
    FS_ERR  err;

    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding MSC device driver ...\r\n"));
    FS_DevDrvAdd((FS_DEV_API *)&FSDev_MSC,                      /* Add MSC device driver (see Note #1).                 */
                 (FS_ERR     *)&err);
    if (err != FS_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add MSC driver w/err = %d\r\n\r\n", err));
        return (DEF_FAIL);
    }
    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                           App_FS_AddIDE()
*
* Description : Add IDE/CF volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) A device error will be returned from FSDev_Open() & FSVol_Open() if the card is not
*                   present or malfunctions.  The device or volume, respectively, is still open, though
*                   the device & volume information will need to be refreshed before the medium is
*                   accessible.
*
*               (2) A volume error will be returned from FSVol_Open() if no valid file system is found
*                   on the card.  It may need to be formatted.
*********************************************************************************************************
*/

#if (APP_CFG_FS_IDE_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddIDE (void)
{
    FS_ERR  err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening IDE volume \"ide:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_IDE,                      /* Add IDE/CF device driver.                            */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
    FSDev_Open("ide:0:", (void *)0, &err);                      /* Open device "ide:0:".                                */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened device.\r\n"));
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened device (not present).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("ide:0:", "ide:0:", 0, &err);                    /* Open volume "ide:0:".                                */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #2).                          */
             APP_TRACE_DBG(("    ...opened volume (unmounted).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                           App_FS_AddNAND()
*
* Description : Add NAND volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) (a) A device error will be returned from FSDev_Open(), FSVol_Open(), FSDev_NOR_LowFmt()
*                       or FSVol_Fmt() if the device malfunctions.  The device may still be open;
*                       however, since NOR flash are fixed devices assumed to be always-functioning, an
*                       application change will be necessary to fully address the problem.
*
*                   (b) A low-level format invalid error will be returned from FSDev_Open() if the device
*                        is not low-level formatted.
*
*               (2) A partition-not-found error will be returned from FSVol_Open() if the device is not
*                   formatted (this will always be the situation immediately after FSDev_NAND_LowFmt()).
*********************************************************************************************************
*/

#if (APP_CFG_FS_NAND_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddNAND (void)
{
    FS_DEV_NAND_CFG  nand_cfg;
    FS_ERR           err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening NAND volume \"nand:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_NAND,                     /* Add NAND device driver.                              */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
    nand_cfg.BlkNbrFirst =  APP_CFG_FS_NAND_BLK_NBR_FIRST;
    nand_cfg.BlkCnt      =  APP_CFG_FS_NAND_BLK_CNT;

    nand_cfg.SecSize     =  APP_CFG_FS_NAND_SEC_SIZE;
    nand_cfg.RBCnt       =  APP_CFG_FS_NAND_RB_CNT;
    nand_cfg.PhyPtr      = (FS_DEV_NAND_PHY_API *)APP_CFG_FS_NAND_PHY_PTR;

    nand_cfg.BusWidth    =  APP_CFG_FS_NAND_BUS_WIDTH;
    nand_cfg.MaxClkFreq  =  APP_CFG_FS_NAND_MAX_CLK_FREQ;


    FSDev_Open("nand:0:", (void *)&nand_cfg, &err);             /* Open device "nand:0:".                               */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened device.\r\n"));
             break;


        case FS_ERR_DEV_INVALID_LOW_FMT:                        /* Low fmt invalid (see Note #1b).                      */
             APP_TRACE_DBG(("    ...opened device (not low-level formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSDev_NAND_LowFmt("nand:0:", &err);
#endif
             if (err != FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...low-level format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1a).                         */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
        default:
             APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("nand:0:", "nand:0:", 0, &err);                  /* Open volume "nand:0:".                               */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;


        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #2).                          */
             APP_TRACE_DBG(("    ...opened device (not formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSVol_Fmt("nand:0:", (void *)0, &err);
#endif
             if (err != FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1a).                         */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened volume (unmounted).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                           App_FS_AddNOR()
*
* Description : Add NOR volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) (a) A device error will be returned from FSDev_Open(), FSVol_Open(), FSDev_NOR_LowFmt()
*                       or FSVol_Fmt() if the device malfunctions.  The device may still be open;
*                       however, since NOR flash are fixed devices assumed to be always-functioning, an
*                       application change will be necessary to fully address the problem.
*
*                   (b) A low-level format invalid error will be returned from FSDev_Open() if the device
*                        is not low-level formatted.
*
*               (2) A partition-not-found error will be returned from FSVol_Open() if the device is not
*                   formatted (this will always be the situation immediately after FSDev_NOR_LowFmt()).
*********************************************************************************************************
*/

#if (APP_CFG_FS_NOR_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddNOR (void)
{
    FS_DEV_NOR_CFG  nor_cfg;
    FS_ERR          err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening NOR volume \"nor:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_NOR,                      /* Add NOR device driver.                               */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
    nor_cfg.AddrBase         =  APP_CFG_FS_NOR_ADDR_BASE;
    nor_cfg.RegionNbr        =  APP_CFG_FS_NOR_REGION_NBR;

    nor_cfg.AddrStart        =  APP_CFG_FS_NOR_ADDR_START;
    nor_cfg.DevSize          =  APP_CFG_FS_NOR_DEV_SIZE;
    nor_cfg.SecSize          =  APP_CFG_FS_NOR_SEC_SIZE;
    nor_cfg.PctRsvd          =  APP_CFG_FS_NOR_PCT_RSVD;
    nor_cfg.EraseCntDiffTh   =  APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH;

    nor_cfg.PhyPtr           = (FS_DEV_NOR_PHY_API *)APP_CFG_FS_NOR_PHY_PTR;

    nor_cfg.BusWidth         =  APP_CFG_FS_NOR_BUS_WIDTH;
    nor_cfg.BusWidthMax      =  APP_CFG_FS_NOR_BUS_WIDTH_MAX;
    nor_cfg.PhyDevCnt        =  APP_CFG_FS_NOR_PHY_DEV_CNT;
    nor_cfg.MaxClkFreq       =  APP_CFG_FS_NOR_MAX_CLK_FREQ;

    FSDev_Open("nor:0:", (void *)&nor_cfg, &err);               /* Open device "nor:0:".                                */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened device.\r\n"));
             break;


        case FS_ERR_DEV_INVALID_LOW_FMT:                        /* Low fmt invalid (see Note #1b).                      */
             APP_TRACE_DBG(("    ...opened device (not low-level formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSDev_NOR_LowFmt("nor:0:", &err);
#endif
             if (err != FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...low-level format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1a).                         */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
        default:
             APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("nor:0:", "nor:0:", 0, &err);                    /* Open volume "nor:0:".                                */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;


        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #2).                          */
             APP_TRACE_DBG(("    ...opened device (not formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSVol_Fmt("nor:0:", (void *)0, &err);
#endif
             if (err != FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1a).                         */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened volume (unmounted).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                           App_FS_AddRAM()
*
* Description : Add RAM disk volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) A partition-not-found error will be returned from FSVol_Open() if the device is not
*                   formatted (for a RAM disk, this will typically be the situation immediately after
*                   FSDev_Open()).
*********************************************************************************************************
*/

#if (APP_CFG_FS_RAM_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddRAM (void)
{
    FS_DEV_RAM_CFG  ram_cfg;
    FS_ERR          err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening RAM disk volume \"ram:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_RAM,                      /* Add RAM disk driver.                                 */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
                                                                /* Assign RAM disk configuration ...                    */
    ram_cfg.SecSize =  APP_CFG_FS_RAM_SEC_SIZE;                 /* ... (a) sector size           ...                    */
    ram_cfg.Size    =  APP_CFG_FS_RAM_NBR_SECS;                 /* ... (b) disk size (in sectors)...                    */
    ram_cfg.DiskPtr = (void *)&App_FS_RAM_Disk[0];              /* ... (c) pointer to disk RAM.                         */

    FSDev_Open("ram:0:", (void *)&ram_cfg, &err);               /* Open device "ram:0:".                                */
    if (err != FS_ERR_NONE) {
        APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
        return (DEF_FAIL);
    }
    APP_TRACE_DBG(("    ...opened device.\r\n"));


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("ram:0:", "ram:0:", 0, &err);                    /* Open volume "ram:0:".                                */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;


        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #1).                          */
             APP_TRACE_DBG(("    ...opened device (not formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSVol_Fmt("ram:0:", (void *)0, &err);
#endif
             if (err != FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;


        case FS_ERR_DEV:
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                         App_FS_AddSD_Card()
*
* Description : Add SD/MMC (CardMode) volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) A device error will be returned from FSDev_Open() & FSVol_Open() if the card is not
*                   present or malfunctions.  The device or volume, respectively, is still open, though
*                   the device & volume information will need to be refreshed before the medium is
*                   accessible.
*
*               (2) A volume error will be returned from FSVol_Open() if no valid file system is found
*                   on the card.  It may need to be formatted.
*********************************************************************************************************
*/

#if (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddSD_Card (void)
{
    FS_ERR  err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening SD/MMC (CardMode) volume \"sdcard:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_SD_Card,                  /* Add SD/MMC (CardMode) device driver.                 */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
    FSDev_Open("sdcard:0:", (void *)0, &err);                   /* Open device "sdcard:0:".                             */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened device.\r\n"));
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened device (not present).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("sdcard:0:", "sdcard:0:", 0, &err);              /* Open volume "sdcard:0:".                             */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;

        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #2).                          */
             APP_TRACE_DBG(("    ...opened volume (unmounted).\r\n"));
             return (DEF_FAIL);

        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                         App_FS_AddSD_SPI()
*
* Description : Add SD/MMC (SPI) volume.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if volume opened.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_FS_Init().
*
* Note(s)     : (1) A device error will be returned from FSDev_Open() & FSVol_Open() if the card is not
*                   present or malfunctions.  The device or volume, respectively, is still open, though
*                   the device & volume information will need to be refreshed before the medium is
*                   accessible.
*
*               (2) A volume error will be returned from FSVol_Open() if no valid file system is found
*                   on the card.  It is then formatted.
*********************************************************************************************************
*/

#if (APP_CFG_FS_SD_EN == DEF_ENABLED)
static  CPU_BOOLEAN  App_FS_AddSD_SPI (void)
{
    FS_ERR  err;


    APP_TRACE_DBG(("    ===========================================================    \r\n"));
    APP_TRACE_DBG(("    Adding/opening SD/MMC (SPI) volume \"sd:0:\"...\r\n"));

    FS_DevDrvAdd((FS_DEV_API *)&FSDev_SD_SPI,                   /* Add SD/MMC (SPI) device driver.                      */
                 (FS_ERR     *)&err);
    if ((err != FS_ERR_NONE) &&
        (err != FS_ERR_DEV_DRV_ALREADY_ADDED)) {
        APP_TRACE_DBG(("    ...could not add driver w/err = %d\r\n\r\n", &err));
        return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN DEV --------------------- */
    FSDev_Open("sd:0:", (void *)0, &err);                       /* Open device "sd:0:".                                 */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened device.\r\n"));
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened device (not present).\r\n"));
             return (DEF_FAIL);


        default:
             APP_TRACE_DBG(("    ...opening device failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }


                                                                /* --------------------- OPEN VOL --------------------- */
    FSVol_Open("sd:0:", "sd:0:", 0u, &err);                     /* Open volume "sd:0:".                                 */
    switch (err) {
        case FS_ERR_NONE:
             APP_TRACE_DBG(("    ...opened volume (mounted).\r\n"));
             break;


        case FS_ERR_DEV:                                        /* Device error (see Note #1).                          */
        case FS_ERR_DEV_IO:
        case FS_ERR_DEV_TIMEOUT:
        case FS_ERR_DEV_NOT_PRESENT:
             APP_TRACE_DBG(("    ...opened volume (unmounted).\r\n"));
             return (DEF_FAIL);
             
             
        case FS_ERR_PARTITION_NOT_FOUND:                        /* Volume error (see Note #2).                          */
             APP_TRACE_DBG(("    ...opened device (not formatted).\r\n"));
#if (FS_CFG_RD_ONLY_EN == DEF_DISABLED)
             FSVol_Fmt("sd:0:", (void *)0, &err);
#endif
             if (err == FS_ERR_NONE) {
                APP_TRACE_DBG(("    ...format completed.\r\n"));
             } else {
                APP_TRACE_DBG(("    ...format failed.\r\n"));
                return (DEF_FAIL);
             }
             break;




        default:
             APP_TRACE_DBG(("    ...opening volume failed w/err = %d.\r\n\r\n", err));
             return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                             ENABLE END
*********************************************************************************************************
*/

#endif
