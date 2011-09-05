/******************************************************************************
* rdk_meta.h
* 
* Header file for RDK meta information flash block functionality.
* 
******************************************************************************/
#ifndef __RDK_META_H__
#define __RDK_META_H__

/* Define a struct that holds our meta information for interacting with the
  cloud.  For the Renesas toolchain, this information should already be in 
  flash.  If it is not in flash, flash will have to be initialized by the user 
  app or other software environment (e.g. uC/OSIII) that has routines written 
  for the "registration" api.
  The idea here is that the system has been pre-registered/configured, 
  and during the registration process (usually this is during functional
  test in manufacturing), this information was programmed.   Thereafter,
  the information can be read from flash during normal operation.  If the 
  system software is capable, it can continue to periodically read the 
  values from the cloud to monitor for required system changes.  If it
  finds the values are different (newer) than the values in flash, it
  may indicate that a system update is required(e.g. firmware).  After taking
  the necessary action, the system would typically re-program the meta 
  information in flash to sync with the cloud version.
  Note that the only information really required to be hard-coded is the
  server ip address, server port number, api structure, alias name for
  meta data, product type ID and unique identifier (MAC address).
  Note: structure is stored in data flash block 14.  Must not exceed block 14
  flash size of 2kBytes.  Also, all elements must be padded to 8 bytes per 
  element due to write limitation.
  */
#define RDK_META_LOCATION             0x00107000 // our meta structure is locked into position at the beginning of block 14 of data flash
#define RDK_META_SIZE                 256        // we could use all 2k, but for now limiting size to 256 bytes since flash routine can only write 256 bytes at a time
#define RDK_META_CIK_SIZE             40
#define RDK_META_SERVER_SIZE          6
#define RDK_META_PAD0_SIZE            2
#define RDK_META_MARK_SIZE            8
#define RDK_META_RSVD_SIZE            200
typedef struct {
  char cik[RDK_META_CIK_SIZE];                   // our client interface key
  char server[RDK_META_SERVER_SIZE];             // ip address of m2.exosite.com (not using DNS at this stage)
  char pad0[RDK_META_PAD0_SIZE];                 // pad 'server' to 8 bytes
  char mark[RDK_META_MARK_SIZE];                 // watermark
  char rsvd[RDK_META_RSVD_SIZE];                 // reserved space - pad to RDK_META_SIZE 
} rdk_meta;

#define EXOMARK "exosite!"

void rdk_meta_defaults(void);
void rdk_meta_init(void);
unsigned char rdk_meta_write(unsigned char * write_buffer, unsigned short srcBytes, unsigned char * pMember);

#endif /* __RDK_META_H__ */


