static char sccsid[] = "@(#)91	1.1  src/bos/diag/tu/gga/strtesttu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:31";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: compare_words
 *		string_test_tu
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


#include <sys/types.h>
#include <stdio.h>
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"

#define BUFFER_SIZE                16            /* 16 bytes buffer (4 words)*/
#define MAX_Y_RES                  1024
#define MAX_X_RES                  2048



static int compare_words (ULONG *, ULONG *, ULONG);



/*
 * NAME : string_test_tu
 *
 * DESCRIPTION :
 *
 *   This function is used to test the string function of the RSC CPU.
 *   It will perform several tests through the entire GGA Native
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
  ULONG  i, hexdata, *hexaddr, data, j, frame_buffer_end;
  ULONG  *read_buffer, *write_buffer;
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
    read_buffer = (ULONG *) malloc ((size_t) BUFFER_SIZE);
    write_buffer = (ULONG *) malloc ((size_t) BUFFER_SIZE);
    rc = (read_buffer == NULL || write_buffer == NULL) ?
                OUT_OFF_MEMORY_ERR : SUCCESS;

    frame_buffer_end = get_end_of_frame_buffer();
    if (rc == SUCCESS)
    {
      hexdata = BLACK;                           /* data to use for testing  */
      do
      {
        /* test the entire Native address space                              */
        for (hexaddr = (ULONG *) FRAME_BUFFER;
             (rc == SUCCESS) && (hexaddr <= (ULONG *) frame_buffer_end);
             hexaddr += (BUFFER_SIZE / 4))
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
            set_mem_info ((ULONG) hexaddr, write_buffer[k], read_buffer[k],
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

static int compare_words (ULONG *str1, ULONG *str2,
                          ULONG str_size_in_bytes)
{
  int   i;

  TITLE("compare_words");

  for (i = 0;
       (i < str_size_in_bytes / BYTES_IN_WORD) && *(str1 + i) == *(str2 + i);
       i++);

  return (i);
}
