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
* Filename      : fs_app.h
* Version       : V4.04
* Programmer(s) : BAN
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  FS_APP_MODULE_PRESENT
#define  FS_APP_MODULE_PRESENT


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   FS_APP_MODULE
#define  FS_APP_EXT
#else
#define  FS_APP_EXT  extern
#endif


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <app_cfg.h>


/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/

#ifndef  APP_CFG_FS_EN
#define  APP_CFG_FS_EN                          DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_IDE_EN
#define  APP_CFG_FS_IDE_EN                      DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_MSC_EN
#define  APP_CFG_FS_MSC_EN                      DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_NAND_EN
#define  APP_CFG_FS_NAND_EN                     DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_NOR_EN
#define  APP_CFG_FS_NOR_EN                      DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_RAM_EN
#define  APP_CFG_FS_RAM_EN                      DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_SD_CARD_EN
#define  APP_CFG_FS_SD_CARD_EN                  DEF_DISABLED
#endif

#ifndef  APP_CFG_FS_SD_EN
#define  APP_CFG_FS_SD_EN                       DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                      CONDITIONAL INCLUDE FILES
*********************************************************************************************************
*/

#if (APP_CFG_FS_EN         == DEF_ENABLED)
#include  <fs.h>

#if (APP_CFG_FS_IDE_EN     == DEF_ENABLED)
#include  <fs_dev_ide.h>
#endif

#if (APP_CFG_FS_MSC_EN     == DEF_ENABLED)
#include  <fs_dev_msc.h>
#endif

#if (APP_CFG_FS_NAND_EN    == DEF_ENABLED)
#include  <fs_dev_nand.h>
#endif

#if (APP_CFG_FS_NOR_EN     == DEF_ENABLED)
#include  <fs_dev_nor.h>
#endif

#if (APP_CFG_FS_RAM_EN     == DEF_ENABLED)
#include  <fs_dev_ramdisk.h>
#endif

#if (APP_CFG_FS_SD_CARD_EN == DEF_ENABLED)
#include  <fs_dev_sd_card.h>
#endif

#if (APP_CFG_FS_SD_EN      == DEF_ENABLED)
#include  <fs_dev_sd_spi.h>
#endif
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (APP_CFG_FS_EN == DEF_ENABLED)
CPU_BOOLEAN  App_FS_Init(void);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/
                                                                /* ----------------- FS CONFIGURATION ----------------- */
#ifndef  APP_CFG_FS_EN
#error  "APP_CFG_FS_EN                            not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_EN != DEF_DISABLED) && \
        (APP_CFG_FS_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_EN                      illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "




#elif   (APP_CFG_FS_EN == DEF_ENABLED)
                                                                /* Device count.                                        */
#ifndef  APP_CFG_FS_DEV_CNT
#error  "APP_CFG_FS_DEV_CNT                       not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_DEV_CNT < 1u)
#error  "APP_CFG_FS_DEV_CNT                 illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif


                                                                /* Volume count.                                        */
#ifndef  APP_CFG_FS_VOL_CNT
#error  "APP_CFG_FS_VOL_CNT                       not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_VOL_CNT < 1u)
#error  "APP_CFG_FS_VOL_CNT                 illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif


                                                                /* File count.                                          */
#ifndef  APP_CFG_FS_FILE_CNT
#error  "APP_CFG_FS_FILE_CNT                      not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_FILE_CNT < 1u)
#error  "APP_CFG_FS_FILE_CNT                illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif


                                                                /* Directory count.                                     */
#ifndef  APP_CFG_FS_DIR_CNT
#error  "APP_CFG_FS_DIR_CNT                       not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 0]                              "

#elif   (APP_CFG_FS_DIR_CNT < 0u)
#error  "APP_CFG_FS_DIR_CNT                 illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 0]                              "
#endif


                                                                /* Buffer count.                                        */
#ifndef  APP_CFG_FS_BUF_CNT
#error  "APP_CFG_FS_BUF_CNT                       not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_BUF_CNT < (2u * APP_CFG_FS_VOL_CNT))
#error  "APP_CFG_FS_BUF_CNT                 illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif


                                                                /* Device driver count.                                 */
#ifndef  APP_CFG_FS_DEV_DRV_CNT
#error  "APP_CFG_FS_DEV_DRV_CNT                   not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_DEV_DRV_CNT < 1u)
#error  "APP_CFG_FS_DEV_DRV_CNT             illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif


                                                                /* Maximum sector size.                                 */
#ifndef  APP_CFG_FS_MAX_SEC_SIZE
#error  "APP_CFG_FS_MAX_SEC_SIZE                  not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "

#elif  ((APP_CFG_FS_MAX_SEC_SIZE !=  512u) && \
        (APP_CFG_FS_MAX_SEC_SIZE != 1024u) && \
        (APP_CFG_FS_MAX_SEC_SIZE != 2048u) && \
        (APP_CFG_FS_MAX_SEC_SIZE != 4096u))
#error  "APP_CFG_FS_MAX_SEC_SIZE            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "
#endif


                                                                /* ------------- IDE DRIVER CONFIGURATION ------------- */
#ifndef  APP_CFG_FS_IDE_EN
#error  "APP_CFG_FS_IDE_EN                        not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_IDE_EN != DEF_DISABLED) && \
        (APP_CFG_FS_IDE_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_IDE_EN                  illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "
#endif


                                                                /* ------------- MSC DRIVER CONFIGURATION ------------- */
#ifndef  APP_CFG_FS_MSC_EN
#error  "APP_CFG_FS_MSC_EN                        not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_MSC_EN != DEF_DISABLED) && \
        (APP_CFG_FS_MSC_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_MSC_EN                  illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "
#endif


                                                                /* ------------- NAND DRIVER CONFIGURATION ------------ */
#ifndef  APP_CFG_FS_NAND_EN
#error  "APP_CFG_FS_NAND_EN                       not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_NAND_EN != DEF_DISABLED) && \
        (APP_CFG_FS_NAND_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_NAND_EN                 illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "


#elif   (APP_CFG_FS_NAND_EN == DEF_ENABLED)

                                                                /* First block number.                                  */
#ifndef  APP_CFG_FS_NAND_BLK_NBR_FIRST
#error  "APP_CFG_FS_NAND_BLK_NBR_FIRST            not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Block count.                                         */
#ifndef  APP_CFG_FS_NAND_BLK_CNT
#error  "APP_CFG_FS_NAND_BLK_CNT                  not #define'd in 'app_cfg.h'          "

#elif   (APP_CFG_FS_NAND_BLK_CNT < 1u)
#error  "APP_CFG_FS_NAND_BLK_CNT            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif

                                                                /* Sector size.                                         */
#ifndef  APP_CFG_FS_NAND_SEC_SIZE
#error  "APP_CFG_FS_NAND_SEC_SIZE                 not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "

#elif  ((APP_CFG_FS_NAND_SEC_SIZE !=  512u) && \
        (APP_CFG_FS_NAND_SEC_SIZE != 1024u) && \
        (APP_CFG_FS_NAND_SEC_SIZE != 2048u) && \
        (APP_CFG_FS_NAND_SEC_SIZE != 4096u))
#error  "APP_CFG_FS_NAND_SEC_SIZE           illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "
#endif

                                                                /* Active block count.                                  */
#ifndef  APP_CFG_FS_NAND_RB_CNT
#error  "APP_CFG_FS_NAND_RB_CNT                   not #define'd in 'app_cfg.h'          "

#elif   (APP_CFG_FS_NAND_RB_CNT < 1u)
#error  "APP_CFG_FS_NAND_RB_CNT             illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif

                                                                /* NAND physical-layer driver pointer.                  */
#ifndef  APP_CFG_FS_NAND_PHY_PTR
#error  "APP_CFG_FS_NAND_PHY_PTR                  not #define'd in 'app_cfg.h'          "
#endif
                                                                /* Bus width.                                           */
#ifndef  APP_CFG_FS_NAND_BUS_WIDTH
#error  "APP_CFG_FS_NAND_BUS_WIDTH                not #define'd in 'app_cfg.h'          "
#endif
                                                                /* Maximum clock frequency.                             */
#ifndef  APP_CFG_FS_NAND_MAX_CLK_FREQ
#error  "APP_CFG_FS_NAND_MAX_CLK_FREQ             not #define'd in 'app_cfg.h'          "
#endif
#endif


                                                                /* ------------- NOR DRIVER CONFIGURATION ------------- */
#ifndef  APP_CFG_FS_NOR_EN
#error  "APP_CFG_FS_NOR_EN                        not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_NOR_EN != DEF_DISABLED) && \
        (APP_CFG_FS_NOR_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_NOR_EN                  illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif   (APP_CFG_FS_NOR_EN == DEF_ENABLED)

                                                                /* Base address.                                        */
#ifndef  APP_CFG_FS_NOR_ADDR_BASE
#error  "APP_CFG_FS_NOR_ADDR_BASE                 not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Region number.                                       */
#ifndef  APP_CFG_FS_NOR_REGION_NBR
#error  "APP_CFG_FS_NOR_REGION_NBR                not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Start address.                                       */
#ifndef  APP_CFG_FS_NOR_ADDR_START
#error  "APP_CFG_FS_NOR_ADDR_START                not #define'd in 'app_cfg.h'          "

#elif   (APP_CFG_FS_NOR_ADDR_START < APP_CFG_FS_NOR_ADDR_BASE)
#error  "APP_CFG_FS_NOR_ADDR_START          illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= APP_CFG_FS_NOR_ADDR_BASE]       "
#endif

                                                                /* Device size.                                         */
#ifndef  APP_CFG_FS_NOR_DEV_SIZE
#error  "APP_CFG_FS_NOR_DEV_SIZE                  not #define'd in 'app_cfg.h'          "

#elif   (APP_CFG_FS_NOR_DEV_SIZE < 1u)
#error  "APP_CFG_FS_NOR_DEV_SIZE            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif

                                                                /* Sector size.                                         */
#ifndef  APP_CFG_FS_NOR_SEC_SIZE
#error  "APP_CFG_FS_NOR_SEC_SIZE                  not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "

#elif  ((APP_CFG_FS_NOR_SEC_SIZE !=  512u) && \
        (APP_CFG_FS_NOR_SEC_SIZE != 1024u) && \
        (APP_CFG_FS_NOR_SEC_SIZE != 2048u) && \
        (APP_CFG_FS_NOR_SEC_SIZE != 4096u))
#error  "APP_CFG_FS_NOR_SEC_SIZE            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "
#endif

                                                                /* Percentage of device reserved.                       */
#ifndef  APP_CFG_FS_NOR_PCT_RSVD
#error  "APP_CFG_FS_NOR_PCT_RSVD                 not #define'd in 'app_cfg.h'           "

#elif  ((APP_CFG_FS_NOR_PCT_RSVD < FS_DEV_NOR_PCT_RSVD_MIN) || \
        (APP_CFG_FS_NOR_PCT_RSVD > FS_DEV_NOR_PCT_RSVD_MAX))
#error  "APP_CFG_FS_NOR_PCT_RSVD           illegally #define'd in 'app_cfg.h'           "
#error  "                                  [MUST be >= FS_DEV_NOR_PCT_RSVD_MIN]         "
#error  "                                  [     && <= FS_DEV_NOR_PCT_RSVD_MAX]         "
#endif

                                                                /* Erase count differential threshold.                  */
#ifndef  APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH
#error  "APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH        not #define'd in 'app_cfg.h'           "

#elif  ((APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH < FS_DEV_NOR_ERASE_CNT_DIFF_TH_MIN) || \
        (APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH > FS_DEV_NOR_ERASE_CNT_DIFF_TH_MAX))
#error  "APP_CFG_FS_NOR_ERASE_CNT_DIFF_TH  illegally #define'd in 'app_cfg.h'           "
#error  "                                  [MUST be >= FS_DEV_NOR_ERASE_CNT_DIFF_TH_MIN]"
#error  "                                  [     && <= FS_DEV_NOR_ERASE_CNT_DIFF_TH_MAX]"
#endif

                                                                /* NOR physical-layer driver pointer.                   */
#ifndef  APP_CFG_FS_NOR_PHY_PTR
#error  "APP_CFG_FS_NOR_PHY_PTR                   not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Bus width.                                           */
#ifndef  APP_CFG_FS_NOR_BUS_WIDTH
#error  "APP_CFG_FS_NOR_BUS_WIDTH                 not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Maximum bus width.                                   */
#ifndef  APP_CFG_FS_NOR_BUS_WIDTH_MAX
#error  "APP_CFG_FS_NOR_BUS_WIDTH_MAX             not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Physical device count.                               */
#ifndef  APP_CFG_FS_NOR_PHY_DEV_CNT
#error  "APP_CFG_FS_NOR_PHY_DEV_CNT               not #define'd in 'app_cfg.h'          "
#endif

                                                                /* Maximum clock frequency.                             */
#ifndef  APP_CFG_FS_NOR_MAX_CLK_FREQ
#error  "APP_CFG_FS_NOR_MAX_CLK_FREQ              not #define'd in 'app_cfg.h'          "
#endif
#endif


                                                                /* ------------- RAM DRIVER CONFIGURATION ------------- */
#ifndef  APP_CFG_FS_RAM_EN
#error  "APP_CFG_FS_RAM_EN                        not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_RAM_EN != DEF_DISABLED) && \
        (APP_CFG_FS_RAM_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_RAM_EN                  illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif   (APP_CFG_FS_RAM_EN == DEF_ENABLED)
                                                                /* Number of sectors.                                   */
#ifndef  APP_CFG_FS_RAM_NBR_SECS
#error  "APP_CFG_FS_RAM_NBR_SECS                  not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "

#elif   (APP_CFG_FS_RAM_NBR_SECS < 1u)
#error  "APP_CFG_FS_RAM_NBR_SECS            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be >= 1]                              "
#endif

                                                                /* Sector size.                                         */
#ifndef  APP_CFG_FS_RAM_SEC_SIZE
#error  "APP_CFG_FS_RAM_SEC_SIZE                  not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "

#elif  ((APP_CFG_FS_RAM_SEC_SIZE !=  512) && \
        (APP_CFG_FS_RAM_SEC_SIZE != 1024) && \
        (APP_CFG_FS_RAM_SEC_SIZE != 2048) && \
        (APP_CFG_FS_RAM_SEC_SIZE != 4096))
#error  "APP_CFG_FS_RAM_SEC_SIZE            illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  512]                              "
#error  "                                   [     || 1024]                              "
#error  "                                   [     || 2048]                              "
#error  "                                   [     || 4096]                              "
#endif
#endif


                                                                /* ---------- SD (CARD) DRIVER CONFIGURATION ---------- */
#ifndef  APP_CFG_FS_SD_CARD_EN
#error  "APP_CFG_FS_SD_CARD_EN                    not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_SD_CARD_EN != DEF_DISABLED) && \
        (APP_CFG_FS_SD_CARD_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_SD_CARD_EN              illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "
#endif


                                                                /* ----------- SD (SPI) DRIVER CONFIGURATION ---------- */
#ifndef  APP_CFG_FS_SD_EN
#error  "APP_CFG_FS_SD_EN                         not #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "

#elif  ((APP_CFG_FS_SD_EN != DEF_DISABLED) && \
        (APP_CFG_FS_SD_EN != DEF_ENABLED ))
#error  "APP_CFG_FS_SD_EN                   illegally #define'd in 'app_cfg.h'          "
#error  "                                   [MUST be  DEF_DISABLED]                     "
#error  "                                   [     ||  DEF_ENABLED ]                     "
#endif

#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of FS app module include.                        */
