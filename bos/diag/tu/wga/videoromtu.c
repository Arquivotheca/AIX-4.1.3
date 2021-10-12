static char sccsid[] = "@(#)80  1.2  src/bos/diag/tu/wga/videoromtu.c, tu_wga, bos411, 9428A410j 4/23/93 15:31:28";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: convert_to_rs_6000_format
 *              crc16
 *              get_vrs_bytes
 *              read_words
 *              rscan_crc_check
 *              video_rom_scan_tu
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>

#include "wga_reg.h"
#include "wgamisc.h"
#include "exectu.h"
#include "tu_type.h"


/* The following offsets are defined in Video ROM Scan POST/DIR specification*/

#define  RSCAN_OFFSET                0x00        /* offset of Video ROM Scan */
#define  RSCAN_LL_FLAG_OFFSET        (RSCAN_OFFSET + 0x7D)
#define  RSCAN_HEADER_NODE_OFFSET    (RSCAN_OFFSET + 0x102)

#define  IDENTIFIER_OFFSET           0x002       /* offset to each node      */
#define  RSCAN_STRING_OFFSET         0x004       /* rscan ascii string offset*/
#define  RSCAN_ABS_ADDR              0x00C       /* offset to each node      */

#define  BLOCK_SIZE                  512         /* 512 bytes per block      */
#define  RSCAN_ID                    0x55AA      /* Video ROM Scan tag       */
#define  RSCAN_LL_EXITS              0x7D        /* flag indicating LL exits */
#define  SWITCH_ROM_FLAG             0x0001      /* binary 1 in RISC/600 fmt */
#define  RSCAN_IDENTIFIER            "RISC6002"
#define  RSCAN_END_OF_LIST           0x0000      /* indicator of last node   */

static void get_vrs_bytes (ulong_t, int, ulong_t *);
static void read_words (char *, ulong_t, ulong_t);
static ushort crc16 (char *pbuff, int length);
static int rscan_crc_check (ulong_t);



/*
 * NAME : video_rom_scan_tu
 *
 * DESCRIPTION :
 *
 *   This function is used to perform a CRC check for the Feature
 *   Video ROM on the WGA adapter.  Before it can validate the CRC,
 *   it will validate other fields in the Video ROM.  These fields
 *   include PS/2 ROM header, RISC/6000 linked list flag, switch
 *   ROM flag identifier, and the RISC/6000 flag.  If any of these
 *   fields is found invalid, the function will return the appropriate
 *   return code without even continue to the next field.  Otherwise,
 *   the CRC check will be performed.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

int video_rom_scan_tu (void)
{
  int      rc;
  ulong_t  rom_header, rom_length, risc_6000_abs_addr;
  char    *rom_buffer, rscan_identifier [9];
  short    block_identifier, node_offset;
  int      pass = 0;

#ifdef LOGMSGS
  char     msg[80];
#endif

  TITLE("video_rom_scan");
  disable_video ();

  if ((rc = clear_screen ()) == SUCCESS)
  {
    /* check if Video ROM Scan adapter is present by reading ROM tag         */
    get_vrs_bytes (RSCAN_OFFSET, 4, &rom_header);
    if ((rom_header >> 16) == RSCAN_ID)
    {
       /* allocate enought memory and then copy Video ROM into the memory    */
       /* to verify.                                                         */
       rom_length = ((rom_header & 0xFF00) >> 8) * BLOCK_SIZE;
       if ((rom_buffer = (char *) malloc ((size_t) rom_length)) != NULL)
       {
         read_words (rom_buffer, (ulong_t) RSCAN_OFFSET,
                                 (ulong_t) (rom_length / BYTES_IN_WORD));

         if (rom_buffer [RSCAN_LL_FLAG_OFFSET] == RSCAN_LL_EXITS)
         {
            /* extract 2 bytes from rom_buffer for offset of the first node  */
            node_offset = *((short *) (rom_buffer + RSCAN_HEADER_NODE_OFFSET));

            do
            {
              /* convert node_offset from Intel format to RISC/6000 format   */
              node_offset = ((node_offset & 0xFF00) >> 8) +
                             ((node_offset & 0xFF) << 8);

              /* extract switch_rom_flag from Intel to RISC/6000 format      */
              block_identifier = *((short *) (rom_buffer + node_offset
                                                         + IDENTIFIER_OFFSET));
              block_identifier = ((block_identifier & 0xFF00) >> 8) +
                                 ((block_identifier & 0x00FF) << 8);

              if (block_identifier == SWITCH_ROM_FLAG)
              {
                pass ++;                         /* found atleast 1 Video ROM*/
                /* strip off the identifer string from rom_buffer            */
                memcpy (rscan_identifier,
                        rom_buffer + node_offset + RSCAN_STRING_OFFSET,
                        sizeof (rscan_identifier) - 1);
                rscan_identifier [sizeof(rscan_identifier)] = '\0';

                if (memcmp (rscan_identifier, RSCAN_IDENTIFIER,
                                     sizeof(rscan_identifier) - 1) == SUCCESS)
                {
                  risc_6000_abs_addr = *((ulong_t *) (rom_buffer + node_offset
                                                      + RSCAN_ABS_ADDR));
                  rc = rscan_crc_check (risc_6000_abs_addr);
                }
                else
                {
                  rc = VIDEO_ROM_ERR;
#ifdef LOGMSGS
                  sprintf (msg, "INVALID ASCII characters = >%s<",
                           rscan_identifier);
                  LOG_MSG(msg);
#endif
                } /* endif */
              } /* endif */

              /* extract the offset (2 bytes) of next node in linked list    */
              node_offset = *((short *) (rom_buffer + node_offset));

            } while ((node_offset != RSCAN_END_OF_LIST) && (rc == SUCCESS));

            /* if we did not find at least 1 Video ROM then it's invalid     */
            if (pass == 0)
            {
              rc = VIDEO_ROM_ERR;
              LOG_MSG ("Could not find any valid Switch ROM flag");
            }
         }
         else
         {
           rc = VIDEO_ROM_ERR;
           LOG_MSG ("Could not detect linked list for RISC/6000 ROM");
         }

         free (rom_buffer);
       }
       else
       {
         rc = OUT_OFF_MEMORY_ERR;
         LOG_MSG ("Can NOT allocate memory");
       }
    }
    else
    {
      rc = VIDEO_ROM_ERR;
      LOG_MSG ("Invalid PS/2 ROM tag");
    }
  } /* endif */
  else
    rc = VRAM_NATIVE_CLR_ERR;

  enable_video ();

  return (rc);
}



/*
 * NAME : rscan_crc_check
 *
 * DESCRIPTION :
 *
 *  This function is used to read the data to be CRC checked into the
 *  buffer and then prepare for the CRC check routine.
 *
 * INPUT :
 *
 *  1. Beginning address of the memory block to be CRC checked.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

#define  START_CRC_OFFSET            0x004       /*4 bytes from the top of ROM*/

static int rscan_crc_check (ulong_t addr)
{
  int     rc;
  ushort  crc_count;
  ulong_t header, length;
  char   *buffer;
 #ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE("rscan_crc_check");

  header = VIDEO_ROM_SCAN_READ(addr);
  length = (header >> 16) * BLOCK_SIZE;          /* extract 2 bytes for len  */

  if ((buffer = (char *) malloc ((size_t) length)) != NULL)
  {
    read_words (buffer, addr, (length / BYTES_IN_WORD));
    crc_count = crc16 (buffer + START_CRC_OFFSET, length - START_CRC_OFFSET);
    if (crc_count == (header & 0xFFFF))
    {
      rc = SUCCESS;
    }
    else
    {
      rc = VIDEO_ROM_CRC_ERR;
#ifdef LOGMSGS
      sprintf (msg, "Video ROM Scan CRC checks.  CRC read = 0x%08x",crc_count);
      LOG_MSG (msg);
#endif
    } /* endif */

    free (buffer);
  }
  else
  {
     rc = OUT_OFF_MEMORY_ERR;
     LOG_MSG ("Can NOT allocate memory");
  }

  return (rc);
}




/*
 * NAME : get_vrs_bytes
 *
 * DESCRIPTION :
 *
 *  Get number of bytes from Video ROM Scan at offset.  Maximum 4 bytes
 *  can be read.  The data will be returned in LSB format.
 *
 * INPUT :
 *
 *  1. Offset to be read
 *  2. Number of bytes to be read (max 4 bytes)
 *
 * OUTPUT :
 *
 *  Bytes read.
 *
 * RETURNS :
 *
 *  None.
 *
*/

static void get_vrs_bytes (ulong_t offset, int length, ulong_t *data)
{
   ulong_t  mask = 0xFFFFFFFF;

   TITLE("get_vrs_bytes");

   mask >>= 32 - (8 * length);                   /* mask out unwanted bits   */

   *data = VIDEO_ROM_SCAN_READ(offset) >> (32 - ((offset % 4 + length) * 8));
   *data = *data & mask;

   return;
}



/*
 * NAME : convert_to_rs_6000_format
 *
 * DESCRIPTION :
 *
 *  Convert a word from Intel format to RISC/6000 format.
 *
 * INPUT :
 *
 *  1. Word in Intel format to be convert
 *  2. Pointer to word in RISC/6000 format
 *
 * OUTPUT :
 *
 *  Word converted.
 *
 * RETURNS :
 *
 *  None.
 *
*/


static void convert_to_rs_6000_format (ulong_t data, ulong_t *addr)
{
   TITLE("convert_to_rs_6000");

   *addr = ((data & 0xFF000000) >> 8) + ((data & 0x00FF0000) << 8) +
           ((data & 0xFF00) >> 8) + ((data & 0xFF) << 8);

   return;
}


static void read_words (char *buffer, ulong_t start_addr, ulong_t num_words)
{
  ulong_t    i, j;

  TITLE("read_words");

  for (i = start_addr, j = 0; j < num_words; i += 4, j++)
  {
    (*((ulong_t *) buffer + j)) = VIDEO_ROM_SCAN_READ(i);
  } /* endfor */

  return;
}




/*
 * NAME: crc16
 *
 * FUNCTION:
 *     crc16 generates a 16 bit "pseudo" CRC.
 *     It is believed that this function uses the following
 *     CRC-CCITT polynomial:
 *              g(x) = x^16 + x^12 + x^5 + x^0      e.g. g(x) = 1 1021
 *
 * NOTES:
 *     The exact origin of this function is not known.  It is believed to
 *     have come from the PC and similar code was used on the RT.  It does
 *     a reliable job of generating a unique "CRC" value.
 *
 *     Note further that if the computed CRC is BYTE REVERSED and appended
 *     to the end of the data and the crc function is called again, the
 *     result is zero if there are NO ERRORS in the data.
 *
 * RETURN VALUE:
 *
 *       0 : The last two bytes of the data contained the correct
 *           BYTE REVERSED CRC for the data space.
 *
 *  dvalue : A two byte CRC for the data.
 */

static ushort crc16 (char *pbuff, int length)
{
#define CRC_MASK 0xFF07
#define COMBINE(x, y) (((x) << 8) | (y))

    struct bytes {
       char msb;
       char lsb;
    };
    union accum {
        ushort whole;
        struct bytes byte;
    } avalue, dvalue;

    char datav;
    int i;

    dvalue.whole = 0xFFFF;
    avalue.whole = 0;
    for(i=0; length > 0; i++, length--)
    {
       datav = *(pbuff+i);
       avalue.byte.lsb = (datav ^ dvalue.byte.lsb);
       dvalue.byte.lsb = avalue.byte.lsb;
       avalue.whole = ((avalue.whole * 16) ^ dvalue.byte.lsb);
       dvalue.byte.lsb = avalue.byte.lsb;
       avalue.whole <<= 8;

       avalue.whole >>= 1;
       avalue.byte.lsb ^= dvalue.byte.lsb;

       avalue.whole >>= 4;

       avalue.whole = COMBINE(avalue.byte.lsb, avalue.byte.msb);
       avalue.whole = ((avalue.whole & CRC_MASK) ^ dvalue.byte.lsb);
       avalue.whole = COMBINE(avalue.byte.lsb, avalue.byte.msb);
       avalue.byte.lsb ^= dvalue.byte.msb;
       dvalue.whole = avalue.whole;
    }

    return (dvalue.whole);
}

