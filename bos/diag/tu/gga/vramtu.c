static char sccsid[] = "@(#)95	1.1  src/bos/diag/tu/gga/vramtu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:39";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: vram_tu
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"

static enum { LINEAR, XY, NATIVE, NUM_METHODS };

static int vram_test (UCHAR, UCHAR, ULONG);

/*
 * NAME : vram_tu
 *
 * DESCRIPTION :
 *
 *  This function performs several functions such as write, read, and
 *  verify to test the video memory using the W9100 Native addressing mode.
 *  It tests all of the video memory space (2048 x 1024).
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
 * NOTE :
 *
*/


int vram_tu (void)
{
  ULONG pattern, data, ltemp;
  ULONG *begin_write_addr, *begin_read_addr, *end_write_addr;
  ULONG *read_addr, *write_addr, frame_buffer_end;
  int   rc;
  char  *method = "NATIVE";

  TITLE("vram_test");

  rc = SUCCESS;

  frame_buffer_end = get_end_of_frame_buffer() | prefix;
  begin_read_addr = (ULONG *) FRAME_BUFFER;
  begin_write_addr = (ULONG *) FRAME_BUFFER;
  end_write_addr = (ULONG *) frame_buffer_end;

  pattern = 0x01010101;

  do
  {
    write_addr = begin_write_addr;               /* first addr for writting  */
    read_addr = begin_read_addr;                 /* first addr for reading   */

    /* write/read the whole VRAM address spaces with pattern                 */
    do
    {
      *write_addr = pattern;                     /* write the pattern to VRAM*/
      __iospace_eieio();
      data = *(read_addr+10);    /* dummy read to wrong address */
      __iospace_eieio();
      data = *(read_addr+100);   /* dummy read to wrong address */
      __iospace_eieio();
      data = *read_addr;

      if (data != pattern)
      {
        rc = VRAM_W_NAT_R_NAT_ERR;               /* save the error code      */
        set_mem_info ((ULONG) write_addr, pattern, data, rc, method, method);

        /**** DEBUG ONLY ****/
        printf("\n Write address=%x, Read address=%x, wrote=%x, read=%x \n", write_addr, read_addr, pattern, data);
        fflush(stdout);
        /********************/
      } /* endif */

      write_addr++;
      read_addr++;
    } while ((write_addr < end_write_addr) && rc == SUCCESS);

    pattern = pattern << 1;
    ltemp = pattern;
    ltemp &= 0xff;
  } while ((ltemp != 0) && (rc == SUCCESS));

  return (rc);
}
