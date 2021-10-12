static char sccsid[] = "@(#)79  1.2  src/bos/diag/tu/wga/strtesttu.c, tu_wga, bos411, 9428A410j 4/23/93 15:31:19";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: compare_words
 *              string_test_tu
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
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "wga_regval.h"

#define BUFFER_SIZE                16            /* 16 bytes buffer (4 words)*/
#define W8720_VRAM_BEGINNING_ADDR  W8720_BASE_ADDR
#define W8720_VRAM_ENDING_ADDR     (W8720_VRAM_BEGINNING_ADDR | 0x3FFFF0)
#define MAX_Y_RES                  1024
#define MAX_X_RES                  2048



static int compare_words (ulong_t *, ulong_t *, ulong_t);



/*
 * NAME : string_test_tu
 *
 * DESCRIPTION :
 *
 *   This function is used to test the string function of the RSC CPU.
 *   It will perform several tests through the entire WGA Native
 *   addresses space with different data.  For each test, it will write
 *   a block of data (16 bytes), read the data back and verify to make
 *   sure the data is correct as written.
 *
 * INPUT PARAMETERS :
 *
 *   None.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS.
 *
*/

int string_test_tu (void)
{
  int      rc, k;
  ulong_t  i, hexdata, *hexaddr, data, j;
  ulong_t  *read_buffer, *write_buffer;
  BOX      box;

  TITLE("string_test_tu");

  /* clear out the screen */
  set_wclip (0, 0, MAX_X_RES - 1, MAX_Y_RES - 1);
  box.xstart = box.ystart = (float) 0;
  box.xend = (float) MAX_X_RES - 1;
  box.yend = (float) MAX_Y_RES - 1;
  box.color = BLACK;

  if ((rc = draw_box (&box)) == SUCCESS)
  {
    /* allocate memory space for read/write buffer                           */
    read_buffer = (ulong_t *) malloc ((size_t) BUFFER_SIZE);
    write_buffer = (ulong_t *) malloc ((size_t) BUFFER_SIZE);
    rc = (read_buffer == NULL || write_buffer == NULL) ?
                OUT_OFF_MEMORY_ERR : SUCCESS;

    if (rc == SUCCESS)
    {
      hexdata = BLACK;                           /* data to use for testing  */
      do
      {
        /* test the entire Native address space                              */
        for (hexaddr = (ulong_t *) W8720_VRAM_BEGINNING_ADDR;
             rc == SUCCESS && hexaddr <= (ulong_t *) W8720_VRAM_ENDING_ADDR;
             hexaddr += BUFFER_SIZE / 4)
        {
          /* initialize the write_buffer with hexdata */
          for (i = 0; i < BUFFER_SIZE / 4; i++)
          {
            *(write_buffer + i) = hexdata;
          } /* endfor */

          memcpy (hexaddr, &write_buffer[0], BUFFER_SIZE); /*write to I/O sp. */
          memcpy (&read_buffer[0], hexaddr, BUFFER_SIZE);  /*read from I/O sp.*/

          k = compare_words (&write_buffer[0], &read_buffer[0], BUFFER_SIZE);

          if (k != (BUFFER_SIZE / BYTES_IN_WORD))
          {
            rc = VRAM_STRING_TEST_ERR;
            set_mem_info ((ulong_t) hexaddr, write_buffer[k], read_buffer[k],
                          rc, "Load/Store string test", "");
          } /* endif */
        } /* endfor */
        hexdata += 0x01010101;                   /* next pattern to be tested*/
      } while (hexdata < 0x0F0F0F0F && rc == SUCCESS);

      free (read_buffer);                       /* return allocated mem. sp. */
      free (write_buffer);

    } /* endif */
    else
      LOG_MSG("Out of memory error");
  }
  else
    rc = VRAM_NATIVE_CLR_ERR;

  return (rc);
}



/*
 * NAME : compare_words
 *
 * DESCRIPTION :
 *
 *   Search 2 strings for a mis-match word.  This function will return
 *   the index of a mis-match word or the number of words it compares.
 *
 * INPUT PARAMETERS :
 *
 *   1. address of string 1
 *   2. address of string 2
 *   3. size of string to be compared (in bytes)
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  The index of mis-match word or number of words it compares.
 *
*/

static int compare_words (ulong_t *str1, ulong_t *str2,
                          ulong_t str_size_in_bytes)
{
  int   i;

  TITLE("compare_words");

  for (i = 0;
       (i < str_size_in_bytes / BYTES_IN_WORD) && *(str1 + i) == *(str2 + i);
       i++);

  return (i);
}
