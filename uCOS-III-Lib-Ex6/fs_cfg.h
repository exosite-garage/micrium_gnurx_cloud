/*
*********************************************************************************************************
*                                             uC/FS V4
*                                     The Embedded File System
*
*                         (c) Copyright 2008-2010; Micrium, Inc.; Weston, FL
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
*                                   FILE SYSTEM CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : fs_cfg.h
* Version       : V4.04
* Programmer(s) : FBJ
*                 BAN
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       FILE SYSTEM CONFIGURATION
*
* Note(s) : (1) Configure FS_CFG_SYS_DRV_SEL to select file system driver inclusion :
*               (a) When FS_SYS_DRV_SEL_FAT, the FAT file system driver will be used.
*
*           (2) Configure FS_CFG_CACHE_EN to enable/disable the cache support :
*               (a) When ENABLED,  cache functionality will be     available.
*               (b) When DISABLED, cache functionality will be NOT available.
*
*           (3) Configure FS_CFG_API_EN to enable/disable presence of POSIX-compatible API :
*               (a) When ENABLED,  POSIX-compatible API will     be present.
*               (b) When DISABLED, POSIX-compatible API will NOT be present.
*
*           (4) Configure FS_CFG_DIR_EN to enable/disable presence of directory access module :
*               (a) When ENABLED,  directory access module will     be present.
*               (b) When DISABLED, directory access module will NOT be present.
*
*           (5) Configure FS_CFG_FILE_BUF_EN to enable/disable file buffer support :
*               (a) When ENABLED,  file read/write buffer functionality will     be available.
*               (b) When DISABLED, file read/write buffer functionality will NOT be available.
*
*           (6) Configure FS_CFG_FILE_LOCK_EN to enable/disable file lock functionality :
*               (a) When ENABLED,  a file can be  locked across    operations.
*               (b) When DISABLED, a file is only locked during an operation.
*
*           (7) Configure FS_CFG_PARTITION_EN to enable/disable extended support for partitions :
*               (a) When ENABLED,  volumes can    be opened on secondary partitions & partitions can    be created.
*               (b) When DISABLED, volumes cannot be opened on secondary partitions & partitions cannot be created.
*
*           (8) Configure FS_CFG_WORKING_DIR_EN to enable/disable working directory support :
*               (a) When ENABLED,  file system operations can be performed relative to a working directory.
*               (b) When DISABLED, all file system operations MUST be performed on absolute paths.
*
*           (9) Configure FS_CFG_UTF8_EN to enable/disable UTF-8 support :
*               (a) When ENABLED,  file names may  be specified in UTF-8.
*               (b) When DISABLED, file names must be specified in ASCII.
*
*          (10) Configure FS_CFG_RD_ONLY_EN to enable/disable file/volume/device write access :
*               (a) When ENABLED,  files, volumes & devices may only be read.  Code for write operations
*                   is NOT included.
*               (b) When DISABLED, files, volumes & devices may be read & written.
*
*          (11) Configure FS_CFG_CONCURRENT_ENTRIES_ACCESS_EN to enable/disable file/dir concurrent access :
*               (a) When ENABLED,  concurrent access is     allowed, and operations are more flexible. 
*               (b) When DISABLED, concurrent access is not allowed, and operations are safer.
*********************************************************************************************************
*/

                                                                /* Configure file system driver presence (see Note #1) :*/
#define  FS_CFG_SYS_DRV_SEL                      FS_SYS_DRV_SEL_FAT
                                                                /*   FS_SYS_DRV_SEL_FAT  FAT file system driver present.*/


                                                                /* Configure POSIX API presence (see Note #3) :         */
#define  FS_CFG_API_EN                           DEF_ENABLED
                                                                /*   DEF_DISABLED   POSIX API NOT present.              */
                                                                /*   DEF_ENABLED    POSIX API     present.              */


                                                                /* Configure cache support (see Note #2) :              */
#define  FS_CFG_CACHE_EN                         DEF_DISABLED
                                                                /*   DEV_DISABLED   cache NOT supported.                */
                                                                /*   DEV_ENABLED    cache     supported.                */


                                                                /* Configure directory module presence (see Note #4) :  */
#define  FS_CFG_DIR_EN                           DEF_ENABLED
                                                                /*   DEF_DISABLED   Directory module NOT present.       */
                                                                /*   DEF_ENABLED    Directory module     present.       */


                                                                /* Configure file buf support (see Note #5) :           */
#define  FS_CFG_FILE_BUF_EN                      DEF_ENABLED
                                                                /*   DEF_DISABLED   File data rd/wr directly from vol.  */
                                                                /*   DEF_ENABLED    File buffer can be assigned.        */


                                                                /* Configure file lock support (see Note #6) :          */
#define  FS_CFG_FILE_LOCK_EN                     DEF_DISABLED
                                                                /*   DEF_DISABLED   Files only locked during single op. */
                                                                /*   DEF_ENABLED    A file may be locked across op's.   */


                                                                /* Configure partition support (see Note #7) :          */
#define  FS_CFG_PARTITION_EN                     DEF_DISABLED
                                                                /*   DEF_DISABLED   Partition creation NOT supported.   */
                                                                /*   DEF_ENABLED    Partition creation     supported.   */


                                                                /* Configure read-only operation (see Note #10) :       */
#define  FS_CFG_RD_ONLY_EN                       DEF_DISABLED
                                                                /*   DEF_DISABLED   Only read operations may be done.   */
                                                                /*   DEF_ENABLED    Read & write operations may be done.*/


                                                                /* Configure UTF8  support (see Note #9) :              */
#define  FS_CFG_UTF8_EN                          DEF_DISABLED
                                                                /*   DEF_DISABLED   File names specified in ASCII.      */
                                                                /*   DEF_ENABLED    File names specified in UTF-8.      */
                                                                
                                                                
                                                                /* Config concurrent access to entries (see Note #11) : */
#define  FS_CFG_CONCURRENT_ENTRIES_ACCESS_EN     DEF_DISABLED
                                                                /*   DEF_DISABLED   Concurrent access NOT allowed.      */
                                                                /*   DEF_ENABLED    Concurrent access     allowed       */


                                                                /* Configure working directory support (see Note #8) :  */
#define  FS_CFG_WORKING_DIR_EN                   DEF_DISABLED
                                                                /*   DEF_DISABLED   Working directory NOT supported.    */
                                                                /*   DEF_ENABLED    Working directory     supported.    */

/*
*********************************************************************************************************
*                             FILE SYSTEM NAME RESTRICTION CONFIGURATION
*
* Note(s) : (1) Configure FS_CFG_MAX_PATH_NAME_LEN with the desired maximum path name length.
*           (2) Configure FS_CFG_MAX_FILE_NAME_LEN with the desired maximum file name length.
*           (3) Configure FS_CFG_MAX_VOL_NAME_LEN with the desired maximum volume name length.
*
*               A full file name is composed of an explicit volume name (optionally) & a path name; the
*               characters after the last non-final path separator character ('\') are the file name :
*
*                   |                                                            |
*                   |---------------------- FULL NAME LENGTH --------------------|
*                   |                                                            |
*
*                     	     |                                                   |
*                    	     |----------------- PATH NAME LENGTH ----------------|
*                    	     |                                                   |
*
*                   myvolume:\MyDir0\MyDir1\MyDir2\my_very_very_long_file_name.txt
*
*                   |       |                      |                             |
*                   |---o---|                      |------ FILE NAME LENGTH -----|
*                   |   |   |                      |                             |
*                       |
*                       ------ VOLUME NAME LENGTH
*
*               The constant 'FS_CFG_MAX_FULL_NAME_LEN' is defined in 'fs_cfg_fs.h' to describe the
*               maximum full name length, as shown in this diagram.
*
*
*           (4) Configure FS_CFG_MAX_DEV_DRV_NAME_LEN with the desired maximum device driver name length.
*           (5) Configure FS_CFG_MAX_DEV_NAME_LEN with the desired maximum device name length.
*
*               A device name is composed of a device driver name, a colon, an integer (the unit number)
*               and a final colon :
*
*                       ------------ DEVICE NAME LENGTH
*                       |
*                   |   |   |
*                   |---o---|
*                   |       |
*                   sdcard:0:
*                   |    |
*                   |-o--|
*                   | |  |
*                     |
*                     -------------- DEVICE DRIVER NAME LENGTH
*
*
*               Each of these maximum name length configurations specifies the maximum string length
*               WITHOUT the NULL character.  Consequently, a buffer which holds one of these names
*               must be one character longer than the define value.
*********************************************************************************************************
*/

                                                                /* Configure maximum device name length (see Note #5).  */
#define  FS_CFG_MAX_DEV_NAME_LEN                          15u

                                                                /* Configure maximum device driver name length ...      */
                                                                /* ... (see Note #4).                                   */
#define  FS_CFG_MAX_DEV_DRV_NAME_LEN                      10u

                                                                /* Configure maximum file name length (see Note #2).    */
#define  FS_CFG_MAX_FILE_NAME_LEN                        255u

                                                                /* Configure maximum path name length (see Note #1).    */
#define  FS_CFG_MAX_PATH_NAME_LEN                        260u

                                                                /* Configure maximum volume name length (see Note #3).  */
#define  FS_CFG_MAX_VOL_NAME_LEN                          10u

/*
*********************************************************************************************************
*                                     FILE SYSTEM DEBUG CONFIGURATION
*
* Note(s) : (1) Configure FS_CFG_DBG_MEM_CLR_EN to enable/disable the file system suite from clearing
*               internal data structure memory buffers; a convenient feature while debugging.
*
*           (2) Configure FS_CFG_DBG_WR_VERIFY_EN to enable/disable the file system suite from verifying
*               writes by reading back data; a convenient feature while debugging a driver.
*********************************************************************************************************
*/
                                                                /* Configure memory clear feature (see Note #1) :       */
#define  FS_CFG_DBG_MEM_CLR_EN                  DEF_ENABLED
                                                                /*   DEF_DISABLED  Data structure clears DISABLED       */
                                                                /*   DEF_ENABLED   Data structure clears ENABLED        */


                                                                /* Configure write verification feature (see Note #2) : */
#define  FS_CFG_DBG_WR_VERIFY_EN                DEF_DISABLED
                                                                /*   DEF_DISABLED  Write verification feature DISABLED  */
                                                                /*   DEF_ENABLED   Write verification feature ENABLED   */

/*
*********************************************************************************************************
*                                FILE SYSTEM ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure FS_ERR_CFG_ARG_CHK_EXT_EN to enable/disable the file system suite external
*               argument check feature :
*               (a) When ENABLED,  ALL arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*               (b) When DISABLED, NO  arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*           (2) Configure FS_ERR_CFG_ARG_CHK_DBG_EN to enable/disable the file system suite internal,
*               debug argument check feature :
*               (a) When ENABLED,     internal arguments are checked/validated to debug the file system
*                   suite.
*               (b) When DISABLED, NO internal arguments are checked/validated to debug the file system
*                   suite.
*********************************************************************************************************
*/
                                                                /* Configure external argument check feature ...        */
                                                                /* ... (see Note #1) :                                  */
#define  FS_CFG_ERR_ARG_CHK_EXT_EN              DEF_ENABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */


                                                                /* Configure internal argument check feature :          */
                                                                /* ... (see Note #2) :                                  */
#define  FS_CFG_ERR_ARG_CHK_DBG_EN              DEF_ENABLED
                                                                /*   DEF_DISABLED     Argument check DISABLED           */
                                                                /*   DEF_ENABLED      Argument check ENABLED            */

/*
*********************************************************************************************************
*                              FILE SYSTEM COUNTER MANAGEMENT CONFIGURATION
*
* Note(s) : (1) Configure FS_CTR_CFG_STAT_EN to enable/disable file system suite statistics counters.
*
*           (2) Configure FS_CTR_CFG_ERR_EN  to enable/disable file system suite error      counters.
*********************************************************************************************************
*/

                                                                /* Configure statistics counter feature (see Note #1) : */
#define  FS_CFG_CTR_STAT_EN                     DEF_DISABLED
                                                                /*   DEF_DISABLED     Stat  counters DISABLED           */
                                                                /*   DEF_ENABLED      Stat  counters ENABLED            */


                                                                /* Configure error      counter feature (see Note #2) : */
#define  FS_CFG_CTR_ERR_EN                      DEF_DISABLED
                                                                /*   DEF_DISABLED     Error counters DISABLED           */
                                                                /*   DEF_ENABLED      Error counters ENABLED            */

/*
*********************************************************************************************************
*                                      FILE SYSTEM FAT CONFIGURATION
*
* Note(s) : (1) Configure FS_FAT_CFG_LFN_EN to enable/disable the file long file name support :
*               (a) When ENABLED,  long file name entries may     be used.
*               (b) When DISABLED, long file name entries may NOT be used.
*
*           (2) Configure FS_FAT_CFG_FAT12_EN to enable/disable FAT12 support :
*               (a) When ENABLED,  FAT12 volumes can         be accessed &   formatted.
*               (b) When DISABLED, FAT12 volumes can neither be accessed nor formatted.
*
*           (3) Configure FS_FAT_CFG_FAT16_EN to enable/disable FAT12 support :
*               (a) When ENABLED,  FAT12 volumes can         be accessed &   formatted.
*               (b) When DISABLED, FAT12 volumes can neither be accessed nor formatted.
*
*           (4) Configure FS_FAT_CFG_FAT32_EN to enable/disable FAT12 support :
*               (a) When ENABLED,  FAT12 volumes can         be accessed &   formatted.
*               (b) When DISABLED, FAT12 volumes can neither be accessed nor formatted.
*
*           (5) Configure FS_FAT_CFG_JOURNAL_EN to enable/disable presence of journaling access module :
*               (a) When ENABLED,  journaling access module will     be present.
*               (b) When DISABLED, journaling access module will NOT be present.
*
*           (6) Configure FS_FAT_CFG_VOL_CHK_EN to enable/disable volume check support :
*               (a) When ENABLED,  volume integrity can     be checked.  If enabled, FS_FAT_CFG_VOL_CHK_MAX_LEVELS
*                   is the maximum number of directory levels that will be checked.
*               (b) When DISABLED, volume integrity can NOT be checked.
*********************************************************************************************************
*/
                                                                /* Configure Long File Name support   (see Note #1) :   */
#define  FS_FAT_CFG_LFN_EN                       DEF_ENABLED
                                                                /*   DEF_DISABLED   LFN NOT supported.                  */
                                                                /*   DEF_ENABLED    LFN     supported.                  */


                                                                /* Configure FAT12 support (see Note #2) :              */
#define  FS_FAT_CFG_FAT12_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED   FAT12 NOT supported.                */
                                                                /*   DEF_ENABLED    FAT12     supported.                */


                                                                /* Configure FAT16 support (see Note #3) :              */
#define  FS_FAT_CFG_FAT16_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED   FAT16 NOT supported.                */
                                                                /*   DEF_ENABLED    FAT16     supported.                */


                                                                /* Configure FAT32 support (see Note #4) :              */
#define  FS_FAT_CFG_FAT32_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED   FAT32 NOT supported.                */
                                                                /*   DEF_ENABLED    FAT32     supported.                */


                                                                /* Configure journaling support (see Note #5) :         */
#define  FS_FAT_CFG_JOURNAL_EN                   DEF_DISABLED
                                                                /*   DEF_DISABLED   Journaling NOT supported.           */
                                                                /*   DEF_ENABLED    Journaling     supported.           */


                                                                /* Configure volume check support (see Note #6) :       */
#define  FS_FAT_CFG_VOL_CHK_EN                   DEF_DISABLED
                                                                /*   DEF_DISABLED   Volume check NOT supported.         */
                                                                /*   DEF_ENABLED    Volume check     supported.         */


                                                                /* Configure max levels chk'd (see Note #6).            */
#define  FS_FAT_CFG_VOL_CHK_MAX_LEVELS                    20u

/*
*********************************************************************************************************
*                           FILE SYSTEM SD/MMC DEVICE DRIVER CONFIGURATION
*
* Note(s) : (1) Configure FS_DEV_SD_SPI_CFG_CRC_EN to enable/disable CRC generation & checking for data
*               writes & reads.
*               (a) When enabled, a CRC will be generated for data written to the card, & the CRC of
*                   received data will be checked.
*               (b) When disabled, no CRC will be generated for data written to the card, & the CRC of
*                   received data will not be checked.
*********************************************************************************************************
*/
                                                                 /* Configure data CRC generation/check (see Note #2).   */
#define  FS_DEV_SD_SPI_CFG_CRC_EN                DEF_DISABLED


/*$PAGE*/
/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/

#define  FS_TRACE_LEVEL                          TRACE_LEVEL_DBG
#define  FS_TRACE                                BSP_Ser_printf

#include <bsp_ser.h>
