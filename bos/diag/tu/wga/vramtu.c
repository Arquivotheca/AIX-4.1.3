static char sccsid[] = "@(#)83  1.2  src/bos/diag/tu/wga/vramtu.c, tu_wga, bos411, 9428A410j 4/23/93 15:31:58";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: vram_decay_test
 *              vram_test
 *              vram_test_XY
 *              vram_tu
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

#include <stdio.h>
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "wga_regval.h"

static enum { LINEAR, XY, NATIVE, NUM_METHODS };


static int vram_test (uchar_t, uchar_t, ulong_t);
static int vram_test_XY (void);
static int vram_decay_test (void);
static int read_native_write_partial_xy(void);


/*
 * NAME : vram_tu
 *
 * DESCRIPTION :
 *
 *  This function performs several functions such as write, read, and
 *  verify to test the video memory using Linear, XY and the W8720 Native
 *  addressing modes.  It tests all of the video memory sapce (2048 x 1024)
 *  with the following algorithms:  write XY read Linear, write XY read
 *  Native, write Linear read XY, write Native read XY, and then finally,
 *  the decay test will be performed.
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

#define VRAM_BUFFER                (0x3FFFFF / 2) /*for 1024 x 2048 resolution*/
#define VRAM_XY_BASE_FULL_WRD_FMT  (VRAM_XY_BASE | 0x1000000)


int vram_tu (void)
{
  int  rc;

  TITLE("vram_tu");

  if ((rc = vram_test((uchar_t) XY, (uchar_t) LINEAR,VRAM_W_XY_R_LIN_ERR)) == SUCCESS)
   if ((rc = vram_test((uchar_t) XY, (uchar_t) NATIVE,VRAM_W_XY_R_NAT_ERR)) == SUCCESS)
    if ((rc = vram_test((uchar_t) LINEAR, (uchar_t) XY,VRAM_W_LIN_R_XY_ERR)) == SUCCESS)
     if ((rc = vram_test((uchar_t) NATIVE, (uchar_t) XY,VRAM_W_NAT_R_XY_ERR)) == SUCCESS)
      if ((rc = vram_test_XY ()) == SUCCESS)
       if ((rc = vram_decay_test ()) == SUCCESS)
          if ((rc = read_native_write_partial_xy()) == SUCCESS);

  return (rc);
}



/*
 * NAME : vram_test
 *
 * DESCRIPTION :
 *
 *  This function performs several functions such as write, read, and
 *  verify to test the video memory using one of the following methods:
 *  Linear, XY and the W8720 Native addressing modes.  It tests all
 *  of the video memory space (2048 x 1024).  The method to test is passed
 *  by the caller.
 *
 *
 * INPUT :
 *
 *  1. method to write VRAM testing
 *  2. method to read VRAM testing
 *  3. error code to be set if an error occurs.
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

static int vram_test (uchar_t write_method, uchar_t read_method,
                      ulong_t error_code)
{
  ulong_t pattern, data;
  ulong_t *begin_write_addr, *begin_read_addr, *end_write_addr;
  ulong_t *read_addr, *write_addr;
  int rc, delta;
  char   *methods [NUM_METHODS] =
                  { "LINEAR ADDRESS", "XY ADDRESS", "NATIVE ADDRESS" };

  TITLE("vram_test");

#ifdef DEBUG_WGA
  printf ("\nVRAM testing method => write %s       read %s",
             methods [write_method % NUM_METHODS],
             methods [read_method % NUM_METHODS]);
  fflush (stdout);
#endif

  rc = SUCCESS;

  /* setting up the VRAM base address for each operation                    */
  switch (read_method)
  {
    case NATIVE : begin_read_addr = WTKN_VRAM_BASE;
                  break;

    case XY     : begin_read_addr = VRAM_XY_BASE_FULL_WRD_FMT;
                  break;

    default     : begin_read_addr = VRAM_LIN_BASE;
                  break;
  } /* endswitch */

  switch (write_method)
  {
    case NATIVE : begin_write_addr = WTKN_VRAM_BASE;
                  end_write_addr = WTKN_VRAM_END;
                  break;

    case XY     : begin_write_addr = VRAM_XY_BASE_FULL_WRD_FMT;
                  end_write_addr = VRAM_XY_BASE_FULL_WRD_FMT +
                                   (VRAM_BUFFER << 2);
                  break;

    default     : begin_write_addr = VRAM_LIN_BASE;
                  end_write_addr = VRAM_LIN_BASE + VRAM_BUFFER;
                  break;
  } /* endswitch */

  delta = 1;                                     /*increment 1 word= 4 pixels*/
  pattern = BLACK;                               /* start with color BLACK   */

  do
  {
    write_addr = begin_write_addr;               /* first addr for writting  */
    read_addr = begin_read_addr;                 /* first addr for reading   */

    /* write/read the whole VRAM address spaces with pattern                 */
    do
    {
      *write_addr = pattern;                     /* write the pattern to VRAM*/
      data = *read_addr;                         /*read with different method*/

      if (data != pattern)
      {
        rc = error_code;                         /* save the error code      */
        set_mem_info (write_addr, pattern, data, rc,
                      methods [write_method % NUM_METHODS],
                      methods [read_method % NUM_METHODS]);
      } /* endif */

      /* for XY addresing mode, we need to zero out bits 30 - 31.  For other */
      /* addressing modes (.i.e. Linear or Native), we do nothing. See VRAM  */
      /* memory map specification for more details                           */
      if (write_method == XY)
        write_addr += (delta << 2);              /*zero out bits 30-31 for XY*/
      else
        write_addr += delta;

      if (read_method == XY)
        read_addr += (delta << 2);               /*zero out bits 30-31 for XY*/
      else
        read_addr += delta;
    } while ((write_addr < end_write_addr) && rc == SUCCESS);

    pattern += 0x01010101;                       /* change to next color     */

  } while ((pattern <= 0x04040404) && (rc == SUCCESS));

  return (rc);
}



/*
 * NAME : vram_test_XY
 *
 * DESCRIPTION :
 *
 *  This function performs several functions such as write, read, and
 *  verify to test the video memory using XY addressing mode.  It tests
 *  all of the video memory sapce (2048 x 1024) with the following
 *  algorithms:  write the entire VRAM spaces with XY addressing mode,
 *  Read XY, and then verify the data.
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
*/

static int vram_test_XY (void)
{
  int      rc;
  ulong_t  pattern, x, y, pixels, color, addr, data;

  TITLE("vram_test_XY");

  rc = SUCCESS;
  pixels = 1;                                    /* test on 1 pixel only     */
  color = RED;
  pattern = get_part_xy (color, pixels);         /* get the pattern for 1 pix*/

  /* fill the VRAM with the test pattern                                     */
  for (x = 0; x < MAX_PHYSICAL_PIX_PER_LINE; x += pixels)
    for (y = 0; y < MAX_PHYSICAL_SCAN_LINES; y++)
      VRAM_XY_WRITE (x,y,PART_W, pattern);

  pattern = get_color_pattern (color);
  /* read and verify VRAM                                                    */
  for (y = 0; y < MAX_PHYSICAL_SCAN_LINES && rc == SUCCESS; y++)
    for (x = 0; x < MAX_PHYSICAL_PIX_PER_LINE && rc == SUCCESS; x += 4)
    {
      data = VRAM_XY_READ (x,y,FULL_R);
      if (pattern != data)
      {
        addr = (ulong_t) XYADDR(x,y,FULL_R);
        rc = VRAM_RW_TEST_ERR;
        set_mem_info (addr, pattern, data, rc,
                      "Write PART WORD FMT", "Read FULL WORD FMT");
      } /* endif */
    } /* endfor */

  return (rc);
}




/*
 * NAME : vram_decay_test
 *
 * DESCRIPTION :
 *
 *  This function performs memory decay test using XY addressing mode.
 *  It tests all of the video memory sapce (2048 x 1024) with the following
 *  algorithms:  write the entire VRAM spaces with XY addressing mode,
 *  decay for 1 second, Read XY, and then verify the data.
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
*/

static int vram_decay_test (void)
{
  int      rc;
  ulong_t  pattern, y, pixels, color, addr, data;
  int      x;

  TITLE("vram_decay_test");

  rc = SUCCESS;
  pixels = 1;                                    /* test on 1 pixel only     */
  color = BLUE;
  pattern = get_part_xy (color, pixels);         /* get the pattern for 1 pix*/

  /* fill the VRAM with the test pattern                                     */
  for (x = MAX_PHYSICAL_PIX_PER_LINE; x >= 0; x -= pixels)
    for (y = 0; y < MAX_PHYSICAL_SCAN_LINES; y++)
      VRAM_XY_WRITE ((ulong_t) x, y, PART_W, pattern);

  sleep (DECAY_MEMORY_TIME);                /* allow for possible decay time */

  pattern = get_color_pattern (color);

  /* read and verify VRAM using XY addressing mode.                          */
  for (y = 0; y < MAX_PHYSICAL_SCAN_LINES && rc == SUCCESS; y++)
    for (x = 0; x < MAX_PHYSICAL_PIX_PER_LINE && rc == SUCCESS; x += 4)
    {
      data = VRAM_XY_READ ((ulong_t) x, y, FULL_R); /*read the data (XY mode)*/
      if (pattern != data)                       /* now, verify the data     */
      {
        addr = (ulong_t) XYADDR((ulong_t) x, y, FULL_R);
        rc = VRAM_DECAY_ERR;
        set_mem_info (addr, pattern, data, rc,
                      "Write PART WORD FMT", "Read FULL WORD FMT");
      } /* endif */
    } /* endfor */

  return (rc);
}




/*
* NAME  :   read_native_write_partial_xy()
*
*
* DESCRIPTION :
*
*         This function reads in Native addressing mode and writes 
*         in partial xy addressing mode. This is done as follows:
*         Clear the screen and set the variable pattern = 1.  Then read
*         the two bytes to either side of the first word boundary
*         using native mode.  Make sure that the concatenation
*         of these data words read equals (int) pattern/2.  If it does not,
*         return error to the calling application.  Note:  this will be equal
*         to zero for pattern = 1 and will, therefore, give us no error since
*         we previously cleared the diplay (i.e. VRAM). Next write the
*         value of pattern into these sixteen bits using the XY
*         partial addressing mode format.  Continue this process, reading
*         and writing on every boundary in vram. After every data boundary
*         in vram has been read and written, set pattern = pattern * 2 and
*         repeat the process above starting with making sure the data word
*         read equals (int) pattern/2.
*
*
* INPUT PARAMETERS :
*
*         None.
*
* OUTPUT:
*
*         None.
*
* RETURNS:
*
*         Error code or SUCCESS.
*
*/



static int read_native_write_partial_xy (void)

{
  ulong_t  first_byte,second_byte;
  ulong_t  pattern, xres, yres;
  int      rc;
  ulong_t  *xy_addr, *native_addr, *xy_addr_end;



 TITLE ("read_native_write_partial_xy");
 
 ALIVE_MSG;
 
 /* write BLACK to the whole VRAM */
 if ((rc= color_full_vram((uchar_t) BLACK))==SUCCESS)
   {
      pattern = 1;
      xres = MAX_PHYSICAL_PIX_PER_LINE;
      yres = MAX_PHYSICAL_SCAN_LINES;
 do
  {

  /* write data to the vram in the xy addressing mode starting
  with pixel 3 */
  xy_addr = XYADDR(3, 0, PART_W);   /* address of the 3rd pixel */     
  xy_addr_end = XYADDR(xres - 1, yres - 1, PART_W); /* address of the last
                                                     pixel on screen */
  native_addr = W8720_BASE_ADDR;


  for (; xy_addr < xy_addr_end && rc == SUCCESS;
           xy_addr += 4, native_addr ++)
  {
   /* note:  for XY addressing mode, we MUST skip last 2 bits (see hardware */
   /* spec.); therefore, to increment to the next word, we must add 16 (in  */
   /* decimal) or 10 (in hex) to the previous XY address.  Since 'xy_addr'  */
   /* variable is a pointer to a ulong_t; therefore, each increment to this */
   /* address will increment by 4.  To add 16 bytes to the previous xy_addr,*/
   /* we must add 4 to 'xy_addr' variable.  We increment to next wrd because*/
   /* we will read/write 4 pixels (4 bytes) at a time.                      */

   /* byte to the left side of the word boundary */ 
   first_byte = (*native_addr & 0xff); 
     
   /* byte to the right side of the word boundary */ 
   second_byte =((* (native_addr + 1)) & 0xff000000) >> 24;
   if ( ((first_byte << 8) | second_byte)  != (pattern >> 1))
    {
      rc = VRAM_W_XY_R_LIN_ERR;

      set_mem_info(native_addr,(pattern >>1),(first_byte << 8) |
                  second_byte, rc,"Read NATIVE FMT","Write XY PART FMT"); 

    }
    *xy_addr = (pattern << 16) | 0x2; /* write data to the vram */
   } /* endfor */

  pattern <<= 1;       /* turn on next pixel */                    
  } while((pattern < 0x10000) && (rc == SUCCESS));
 }
return(rc);
}
