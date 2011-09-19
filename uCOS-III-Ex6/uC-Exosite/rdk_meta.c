/******************************************************************************
* rdk_meta.c
*
* Helper functions for interacting with the RDK meta information flash block.
*
******************************************************************************/

#include "string.h"
#include "rdk_meta.h"
#include "flash_api_rx600.h"

static void erase_data_flash(unsigned char start, unsigned char end);
static void write_data_flash(unsigned char * writeBuffer, unsigned short numBytes, unsigned long writeAddress);

extern const uint32_t block_addresses[54]; // array of flash block addresses indexed by block #

/******************************************************************************
* void rdk_meta_init(void)
*
* This function simply enables flash for reading and writing.
*
******************************************************************************/
void rdk_meta_init(void)
{
  R_FlashDataAreaAccess(0xFFFF, 0xFFFF);
}

/******************************************************************************
* rdk_meta_defaults(void)
*
* This function can be invoked by user code if it is found that the RDK meta
* information is not found in data flash.
* Note that this function read-modify-erase-writes meta location in flash.  Any
* other information stored in the meta block will get wiped.
*
******************************************************************************/
void rdk_meta_defaults(void)
{
  rdk_meta meta_info;
  const unsigned char meta_server_ip[6] = {173,255,209,28,0,80};

  memset(&meta_info, 0, sizeof(meta_info));

  memcpy(meta_info.server, meta_server_ip, RDK_META_SERVER_SIZE);
  strncpy(meta_info.mark, EXOMARK, RDK_META_MARK_SIZE);

  rdk_meta_write((unsigned char *)&meta_info, sizeof(meta_info), (unsigned char *)RDK_META_LOCATION);
}

/******************************************************************************
* rdk_meta_write(unsigned char * write_buffer, unsigned char srcBytes, unsigned char * location)
*
* This function can be used to write a specific member of the meta structure
* into flash.  It is a simple read-modify-erase-write routine.
* NOTE: This routine erases all of block 14 with every call, so use sparingly.
*
* For example, the following lines would update the CIK value:
*   rdk_meta * meta_info = (rdk_meta *)RDK_META_LOCATION;
*   write_meta("UPDATEWITH40CHARCIK", 40, (unsigned char *)meta_info->cik);
*
******************************************************************************/
unsigned char rdk_meta_write(unsigned char * write_buffer, unsigned short srcBytes, unsigned char * location)
{
  rdk_meta meta_info;
  unsigned long offset;

  // make sure 'location' is within the rdk_meta flash region
  if (
    (unsigned long)(location)            < (unsigned long)(RDK_META_LOCATION)
    ||
    (unsigned long)(location + srcBytes) > (unsigned long)(RDK_META_LOCATION + RDK_META_SIZE)
  ) return 1;

  offset = (unsigned long)location - (unsigned long)RDK_META_LOCATION;

  memcpy((char *)((unsigned long)(&meta_info)),          (char *)RDK_META_LOCATION, sizeof(meta_info));  // read our nv struct into RAM
  memcpy((char *)((unsigned long)(&meta_info) + offset), (char *)write_buffer, srcBytes);  // replace specific member info
  erase_data_flash(14,14); //just erase block 14, the meta struct location

  write_data_flash((unsigned char *)&meta_info, sizeof(meta_info), (unsigned long)RDK_META_LOCATION);

  return 0;
}

/******************************************************************************
* void erase_data_flash(unsigned char start_block, unsigned char end_block)
*
* Erases data flash from start_block to end_block, inclusive.  It locks upon
* error.
*
******************************************************************************/
static void erase_data_flash(unsigned char start_block, unsigned char end_block)
{
  uint8_t ret = 0;
  uint8_t loop_counter = 0, block_count = BLOCK_DB0 + start_block;
  uint32_t address = 0x00000000;

  /* Initialise a for loop to erase each of the data flash blocks */
  for (loop_counter = start_block; loop_counter < end_block + 1; loop_counter++)
  {
    /* Fetch beginning address of DF block */
    address = block_addresses[BLOCK_DB0 + loop_counter];

    /* Erase data flash block */
    ret |= R_FlashErase(block_count);

    /* Check if data flash area is blank */
    ret |= R_FlashDataAreaBlankCheck(address,BLANK_CHECK_ENTIRE_BLOCK);

    /* Increment block counter */
    block_count++;
  }

  /* Check Blank Checking */
  ret |= R_FlashDataAreaBlankCheck(address,BLANK_CHECK_ENTIRE_BLOCK);

  /* Halt in while loop when flash API errors detected */
  while (ret);
}

/******************************************************************************
* void write_data_flash(unsigned char * writeBuffer, unsigned short numBytes, unsigned long writeAddress)
*
* Writes data flash with contents of writeBuffer for size of numBytes to
* location writeAddress.  If contents are not fully verified, it locks.
*
******************************************************************************/
static void write_data_flash(unsigned char * writeBuffer, unsigned short numBytes, unsigned long writeAddress)
{
  unsigned char ret = 0;     // API error flag
  unsigned char pad_buffer[256];   // data padding array and loop counter
  unsigned short srcBytes = numBytes;  // we have to save off size to use for compare later

  /* Clear the contents of the flash write buffer array */
  memset(pad_buffer, 0x00, 256);

  /* Copy contents of the write buffer to the padding buffer */
  memcpy((char *)pad_buffer, (char *)writeBuffer, numBytes);

  /* Check if number of bytes is greater than 256 */
  if (numBytes > 256)
  {
    /* Number of bytes to write too high, set error flag to 1 */
    ret |= 1;
  }
  /* Check if number of bytes to write is a multiple of 8 */
  else if (numBytes % 8u)
  {
    /* Pad the data to write so it makes up to the nearest multiple of 8 */
    numBytes += 8u - (numBytes % 8u);
  }

  /* Halt in while loop when flash API errors detected */
  while (ret);

  /* Write contents of write buffer to data flash */
  ret |= R_FlashWrite(writeAddress,(uint32_t)pad_buffer,numBytes);

  /* Compare memory locations to verify written data */
  ret |= memcmp(writeBuffer, (unsigned char *)writeAddress, srcBytes);

  /* Halt in while loop when flash API errors detected */
  while (ret);
}


