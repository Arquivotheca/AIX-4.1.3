static char sccsid[] = "@(#)76  1.3  src/bos/diag/tu/wga/regtu.c, tu_wga, bos411, 9428A410j 7/27/93 22:24:20";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: check_wga_error
 *              disable_cursor
 *              disable_int
 *              disable_video
 *              enable_cursor
 *              enable_int
 *              enable_video
 *              get_all_igc_regs
 *              get_all_wga_regs
 *              get_cursor_pos
 *              get_cursor_status1227
 *              get_igc_reg
 *              get_int_mask
 *              get_reg_type0
 *              get_wga_reg
 *              get_worig
 *              igc_do_blit
 *              igc_draw_quad
 *              igc_write_2D_coord_reg
 *              igc_write_2D_meta_coord
 *              igc_write_pixel1
 *              igc_write_pixel8
 *              initialize_reg_attr
 *              load_dac_reg
 *              reg_tu
 *              reset_wga_error
 *              restore_regs
 *              save_regs
 *              set_cursor_pos
 *              set_reg_type0
 *              set_w_offset
 *              set_wclip
 *              set_worig
 *              wait_for_wtkn_ready1119
 *              write_all_igc_regs
 *              write_all_wga_regs
 *              write_igc_reg
 *              write_wga_reg
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

#include "wga_reg.h"
#include "wga_regval.h"
#include "wgareg.h"
#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "vpd.h"

#define EOS                     '\0'             /* End Of String            */
#define DISABLE_IGC_VIDEO       0x40
#define REG_TEST_LOOPS          5000             /* test 5000 times          */

static int PIXEL_SELECTS [] = { 0, 52, 37, 57 };

static struct
{
  BOOL readonly;
  ulong_t addr;
  ulong_t mask;
  char *name;
} wga_attr [WGA_REG_NUM];

static struct
{
  BOOL readonly;
  BOOL writeonly;
  ulong_t addr;
  ulong_t mask;
  char *name;
} igc_attr [IGC_REG_NUM];



static WGA_REGS_TYPE keep_wga_regs;              /* a place to save registers */
static IGC_REGS_TYPE keep_igc_regs;

static void write_all_wga_regs(WGA_REGS_TYPE);
static void write_all_igc_regs(IGC_REGS_TYPE);
static void get_all_igc_regs(IGC_REGS_TYPE);
static ulong_t get_int_mask(uchar_t);

static BOOL set_reg_type0(ulong_t, ulong_t, ulong_t);
static void get_reg_type0(ulong_t *, ulong_t *, ulong_t);


/*
 * NAME : reg_tu
 *
 * DESCRIPTION :
 *
 *  The purpose of this TU is to verify the integrity of all registers
 *  on the graphic adapter (WGA & IGC registers).  Each register of
 *  the adapter area tested with a lot of random data using
 *  writting/reading/verifying algorithm.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int reg_tu(void)
{
  WGA_REGS_TYPE put_wga_r, get_wga_r;
  ulong_t i, j;
  ulong_t tmp0, tmp1;
  int     rc;
  static IGC_REGS_TYPE  igc_regs;

#ifdef LOGMSGS
  char msg[80];
#endif

  TITLE("reg_tu");

  rc = SUCCESS;
  disable_video ();                              /* do not show random data  */

  /* don't mess with the following registers                                 */
  wga_attr [ADCNTL_REG].readonly = TRUE;
  igc_attr [COLOR_PALETTE_RAM].readonly = TRUE;
  igc_attr [CURCMND_REG].readonly = TRUE;
  igc_attr [INTERRUPT_REG].readonly = TRUE;
  igc_attr [INTERRUPT_ENBL_REG].readonly = TRUE;

  for(i = 0; rc == SUCCESS && i < REG_TEST_LOOPS; i++)
  {
    /* generate random data to be tested */
    for(j = 0; j < WGA_REG_NUM; j++)
      put_wga_r[j] = GET_RAND_ULONG & wga_attr[j].mask;

    /* First, test all of the WGA registers                                  */

    write_all_wga_regs (put_wga_r);              /* write to all WGA regs.   */
    get_all_wga_regs (get_wga_r);                /* read all WGA regs.       */

    /* Now, verify all WGA registers just wrote                              */
    for(j = 0; rc == SUCCESS && j < WGA_REG_NUM; j++)
    {
      if(!wga_attr[j].readonly  && (put_wga_r[j] != get_wga_r[j]))
      {
        rc = REG_COMPARE_ERR;

#ifdef LOGMSGS
        sprintf(msg, "WGA reg. # %d  wrote = 0x%08x  read = 0x%08x",
                 j, put_wga_r[j], get_wga_r[j]);
        LOG_ERROR(msg);
#endif
      }  /* end if */
    }  /* end for */

    /* Now, test all of the IGC registers                                    */
    for(j = 0; rc == SUCCESS && j < IGC_REG_NUM; j++)
    {
      tmp0 = GET_RAND_ULONG & igc_attr [j].mask; /* get random data to test  */

      if(j == SRTCTL_REG)
        tmp0 &= ~(ENABLE_VIDEO);                 /* keep video always disable*/

      if(!write_igc_reg((uchar_t) j, tmp0) || !get_igc_reg((uchar_t) j, &tmp1))
        continue;

      if(tmp0 != tmp1)                           /* error if does not match  */
      {
        rc = REG_COMPARE_ERR;

#ifdef LOGMSGS
        sprintf(msg, "IGC reg. # %d  wrote=0x%08x  read=0x%08x", j, tmp0, tmp1);
        LOG_ERROR(msg);
#endif
      }  /* end if */
    }  /* end for */
  }

  /* clear errors in case some illegal values were programmed */
  reset_wga_error ();
  restore_regs ();                             /* restore default conditions */

  /* Now, we must restore the default WGA and/or IGC registers status        */
  wga_attr[ADCNTL_REG].readonly = FALSE;
  igc_attr[COLOR_PALETTE_RAM].readonly = FALSE;
  igc_attr [CURCMND_REG].readonly = FALSE;
  igc_attr [INTERRUPT_REG].readonly = FALSE;
  igc_attr [INTERRUPT_ENBL_REG].readonly = FALSE;

  enable_video ();

  return(rc);
}



/*
 * NAME : save_regs
 *
 * DESCRIPTION :
 *
 *  Saves WGA and IGC registers in save area.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   Registers saved.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void save_regs(void)
{
  TITLE("save_regs");

  initialize_reg_attr ();                        /* initialize reg addresses*/
  get_all_wga_regs(keep_wga_regs);
  get_all_igc_regs(keep_igc_regs);

  return;
}


/*
 * NAME : restore_regs
 *
 * DESCRIPTION :
 *
 *  Retsores WGA and IGC registers from save area.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   Registers restored.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void restore_regs(void)
{
  TITLE("restore_regs");

  write_all_wga_regs(keep_wga_regs);
  write_all_igc_regs(keep_igc_regs);

  return;
}



/*
 * NAME : get_all_wga_regs
 *
 * DESCRIPTION :
 *
 *  Reads all WGA registers into input buffer.
 *
 * INPUT :
 *
 *   1. WGA_REGS_TYPE buffer.
 *
 * OUTPUT :
 *
 *   WGA registers read into buffer.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void get_all_wga_regs(WGA_REGS_TYPE reg)
{
  int i;

  TITLE("get_all_wga_regs");

  for(i = 0; i < WGA_REG_NUM; i++)
  {
    get_wga_reg(&reg[i], (uchar_t) i);
  }  /* end if */

   return;
}



/*
 * NAME : get_all_igc_regs
 *
 * DESCRIPTION :
 *
 *  Reads all IGC registers into input buffer.
 *
 * INPUT :
 *
 *   1. IGC_REGS_TYPE buffer.
 *
 * OUTPUT :
 *
 *   IGC registers read into buffer.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void get_all_igc_regs(IGC_REGS_TYPE reg)
{
  uchar_t i;

  TITLE("get_all_igc_regs");

  for(i = 0; i < IGC_REG_NUM; i++)
  {
    get_igc_reg (i, &reg[i]);
  }

   return;
}


/*
 * NAME : get_igc_reg
 *
 * DESCRIPTION :
 *
 *  Reads an IGC register into input buffer
 *  as specified by input parameters.
 *
 * INPUT :
 *
 *   1. Pointer to location to store register value.
 *   2. IGC register index.
 *
 * OUTPUT :
 *
 *   IGC register read into buffer.
 *
 * RETURNS :
 *
 *   TRUE if operation succeeded, else FALSE.
 *
*/

BOOL get_igc_reg(uchar_t reg, ulong_t *val)
{
  TITLE("get_igc_reg");

  if(reg >= IGC_REG_NUM || igc_attr[reg].writeonly)
    return(FALSE);

  *val = IGC_REG_READ (igc_attr [reg].addr) & igc_attr[reg].mask;

  return (TRUE);
}



/*
 * NAME : get_wga_reg
 *
 * DESCRIPTION :
 *
 *  Reads an WGA register into input buffer
 *  as specified by input parameters.
 *
 * INPUT :
 *
 *   1. Pointer to location to store register value.
 *   2. WGA register index.
 *
 * OUTPUT :
 *
 *   WGA register read into buffer.
 *
 * RETURNS :
 *
 *   TRUE if operation succeeded, else FALSE.
 *
*/

BOOL get_wga_reg(ulong_t *val, uchar_t reg)
{
  TITLE("get_wga_reg");

  if(reg >= WGA_REG_NUM)
    return(FALSE);

  *val = WGA_REG_READ(wga_attr[reg].addr) & wga_attr[reg].mask;

  return(TRUE);
}


/*
 * NAME : write_wga_reg
 *
 * DESCRIPTION :
 *
 *  Writes an WGA register as specified by input parameters.
 *
 * INPUT :
 *
 *   1. Register value.
 *   2. WGA register index.
 *
 * OUTPUT :
 *
 *   WGA register.
 *
 * RETURNS :
 *
 *   TRUE if operation succeeded, else FALSE.
 *
*/

BOOL write_wga_reg(ulong_t val, uchar_t reg)
{
  TITLE("write_wga_reg");

  if(reg >= WGA_REG_NUM || wga_attr[reg].readonly)
    return(FALSE);

  WGA_REG_WRITE(wga_attr[reg].addr, val);

  return(TRUE);
}


/*
 * NAME : check_wga_err
 *
 * DESCRIPTION :
 *
 * Determines if any error bit is set in WGA status register.
 *
 * INPUT :
 *
 *   1. Pointer to location where to store code of
 *      error bit set.
 *
 * OUTPUT :
 *
 *   1. Code of error bit set.
 *
 * RETURNS :
 *
 *  TRUE if an error bit was found set, else FALSE.
 *
*/

BOOL check_wga_error(int *rc)
{
  ulong_t status, err_reg;

  TITLE("check_wga_error");

  get_wga_reg(&status, (uchar_t) ADSTAT_REG);
  get_wga_reg(&err_reg, (uchar_t) ERRADDR_REG);
  status &= STAT_ERR_MASK;

  if(status == RESV_ACCESS) /* Attempt to Access Reserved Register Error */
    *rc = RESV_BIT_ERR;

  else if (status == BAD_STRING_OP) /* Illegal String Operation Error   */
    *rc = STRING_BIT_ERR;

  else if (status == INVAL_CMD)   /* Invalid Command Error on RSC bus        */
    *rc = CMD_BIT_ERR;

  else if (status == PARITY_ERR)  /* Parity Error on RSC addr/data bus       */
    *rc = PARITY_BIT_ERR;

  else
    *rc = SUCCESS;

  return((*rc == SUCCESS) ? FALSE : TRUE);
}



/*
 * NAME : reset_wga_error
 *
 * DESCRIPTION :
 *
 *  Clears WGA status error bits by reading status register.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int reset_wga_error(void)
{
  ulong_t status;
  int rc;

  TITLE("reset_wga_error");

/* 1st read clears status error bits (if any)    */
/* 2nd read should return all 0's in error bits  */

  rc = SUCCESS;
  get_wga_reg(&status, (uchar_t) ADSTAT_REG);
  get_wga_reg(&status, (uchar_t) ERRADDR_REG);
  get_wga_reg(&status, (uchar_t) ADSTAT_REG);
  status &= STAT_ERR_MASK;

  if(status != 0)
    rc = STATUS_CLEAR_ERR;

  return(rc);
}




/*
 * NAME : igc_write_2D_meta_coord
 *
 * DESCRIPTION :
 *
 *   This function is used to load the coordiantes of the vertices of
 *   points, lines, triangles, quadrilaterals and rectangles using a
 *   short hand notation that requires the minimum number of data
 *   transfers.  Parameter engine registers can be loaded at any time.
 *
 * INPUT :
 *
 *   1. Object type to be drawn.
 *   2. Loads the parameter registers Relative/Absolute to a previously
 *      specified vertex.
 *   3. Which coordinate to be loaded.
 *   4. Value of the coordinate to be loaded.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_write_2D_meta_coord (ulong_t vtype, ulong_t rel,
                              ulong_t yx, ulong_t packed_xy)
{
  ulong_t addr;

  TITLE("igc_write_2D_meta_coord");

  addr = (ulong_t) (PSEUDO_REGISTERS) | vtype | rel | yx;

#ifdef DUMP_REGS
  printf ("\nWrite to 2D meta address = 0x%08x       Packed XY = 0x%08x", addr, packed_xy);
  fflush(stdout);
#endif

  IGC_REG_WRITE (addr, packed_xy);

  return;
}




/*
 * NAME : igc_write_2D_coord_reg
 *
 * DESCRIPTION :
 *
 *   This function is used to load the parameter register X [0, 1, 2, 3]
 *   and Y [0, 1, 2, 3].  The parameter register can be loaded at any time.
 *
 * INPUT :
 *
 *   1. Mode to be loaded (ABS/REL).  ABS uses frame buffer 0, 0 as the origin.
 *      REL interpretes the coordiante relative to the window offset.
 *   2. Which coordinate to be loaded.
 *   3. Index into X and Y coordinates.
 *   4. 32 bit data value to be written to the bus.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_write_2D_coord_reg (ulong_t rel, ulong_t yx,
                             ulong_t reg, ulong_t val)
{
  ulong_t addr;

  TITLE("igc_write_2D_coord_reg");

  addr = (ulong_t) (PARM_ENGINE_ADDR) | reg << 6 | rel | yx;

#ifdef DUMP_REGS
  printf ("\nWrite to Address = %8x       value = %8x", addr, val);
  fflush(stdout);
#endif

  IGC_REG_WRITE (addr, val);

  return;
}


/*
 * NAME : igc_write_pixel1
 *
 * DESCRIPTION :
 *
 *   Transfer up to 32 pixels from a linear host memory array to a
 *   rectangular display memory array in the frame buffer.
 *
 * INPUT :
 *
 *   1. Number of pixels to be written
 *   2. Value to be written into the frame buffer
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_write_pixel1 (uchar_t count, ulong_t data)
{
  ulong_t addr, offset;

  TITLE("igc_write_pixel1");

  count -= 1;                                    /* offset from 0 - count    */
  offset = (count << 2) & 0x7C;                  /* strip off other bits     */
  addr = igc_attr [PIXEL1_REG].addr | offset;

#ifdef DUMP_REGS
  printf ("\nWrite to Address = 0x%08x       data = 0x%08x", addr, data);
  fflush(stdout);
#endif

  IGC_REG_WRITE (addr, data);

  return;
}


/*
 * NAME : igc_write_pixel8
 *
 * DESCRIPTION :
 *
 *   Transfer one word of pixel data from a linear host memory to a
 *   rectangular display memory array in the frame buffer.  This operation
 *   is useful for writing pixles of various colors to the screen.  A word
 *   consists of 4 pixels.  Each pixel is made of 8 bits.
 *
 * INPUT :
 *
 *   1. 32 bits (4 pixels, 8 bits/pixel) value to be written into
 *      the frame buffer
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_write_pixel8 (ulong_t data)
{
  TITLE("igc_write_pixel8");

#ifdef DUMP_REGS
  printf ("\nWrite to Address = 0x%08x       data = 0x%08x",
          igc_attr[PIXEL8_REG].addr, data);
  fflush(stdout);
#endif

  IGC_REG_WRITE (igc_attr [PIXEL8_REG].addr, data);

  return;
}


/*
 * NAME : igc_draw_quad
 *
 * DESCRIPTION :
 *
 *   This function draws and fills the quadrilateral defined by
 *   the vertices specified in the coordinate registers.  This function
 *   sets the busy bit in the status register and performs the Bresenham
 *   draws and fills algorithm.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_draw_quad (void)
{
  ulong_t  busy;

  TITLE("igc_draw_quad");

  /* waiting for permission to issue a draw command to the Drawing Engine    */
  do
  {
    busy = IGC_REG_READ (QUAD_CMD) & IGM_STATUS_QB_BUSY_MASK;
  }
  while ( busy );

  return;
}


/*
 * NAME : igc_do_blit
 *
 * DESCRIPTION :
 *
 *   This function copies a rectangular area of the display from one
 *   screen location to another.  The vertices of the rectangular area
 *   are defined in the coordinate registers.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void igc_do_blit (void)
{
  ulong_t  busy;

  TITLE("igc_do_blit");

  /* waiting for permission to issue a draw command to the Drawing Engine    */
  do
  {
    busy = IGC_REG_READ (BLIT_CMD) & IGM_STATUS_QB_BUSY_MASK;
  }
  while ( busy );

  return;
}



/*
 * NAME : write_igc_reg
 *
 * DESCRIPTION :
 *
 *  Writes an IGC register as specified by input parameters.
 *
 * INPUT :
 *
 *   1. IGC register index.
 *   2. value to be written.
 *
 * OUTPUT :
 *
 *   IGC register value.
 *
 * RETURNS :
 *
 *   TRUE if operation succeeded, else FALSE.
 *
*/

BOOL write_igc_reg(uchar_t reg, ulong_t val)
{
  TITLE("write_igc_reg");

  if (reg >= IGC_REG_NUM || igc_attr[reg].readonly)
    return(FALSE);

  IGC_REG_WRITE (igc_attr[reg].addr, val);

  return (TRUE);
}



/*
 * NAME : write_all_wga_regs
 *
 * DESCRIPTION :
 *
 * Writes all WGA registers.
 *
 * INPUT :
 *
 *   1. WGA registers data.
 *
 * OUTPUT :
 *
 *   WGA registers.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void write_all_wga_regs(WGA_REGS_TYPE reg)
{
  int i;

  TITLE("write_all_wga_regs");

  for(i = 0; i < WGA_REG_NUM; i++)
  {
    write_wga_reg(reg[i], (uchar_t) i);
  }

   return;
 }



/*
 * NAME : write_all_igc_regs
 *
 * DESCRIPTION :
 *
 * Writes all IGC R/W registers
 *
 * INPUT :
 *
 *   1. IGC registers data.
 *
 * OUTPUT :
 *
 *   IGC registers.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void write_all_igc_regs(IGC_REGS_TYPE reg)
{
  int i;

  TITLE("write_all_igc_regs");

#ifdef DUMP_REGS
printf ("\n\nRestore all WTKN registers (writeable)\n");  fflush(stdout);
#endif

  for(i = 0; i < IGC_REG_NUM; i++)
  {

#ifdef DUMP_REGS
    printf ("\ni = %3d     addr = %8x     value = %8x", i,
                                        igc_attr[i].addr, reg[i]);
    if (igc_attr[i].readonly)
    {
      printf ("     status = read only");
    }
    else
    {
      printf ("     status = %s",
                  igc_attr[i].writeonly ? "write only" : "no read or write");
    } /* endif */
    fflush (stdout);
#endif

    write_igc_reg ((uchar_t) i, reg[i]);
  }

  return;
 }



/*
 * NAME : initialize_reg_attr
 *
 * DESCRIPTION :
 *
 *  Initializes register attributes in local variables
 *  wga_attr[] and igc_attr[].
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *  Variables wga_attr[] and igc_attr[] initialized.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void initialize_reg_attr(void)
{
  int i;
  static BOOL init_done = FALSE;

  TITLE("initialize_reg_attr");

  if(!init_done)
  {
    /* initialize all WGA BUID control registers                             */
    for(i = 0; i < WGA_REG_NUM; i++)
    {
      wga_attr[i].readonly = FALSE;
      wga_attr[i].addr = 0;
      wga_attr[i].mask = 0xffffffff;
      wga_attr[i].name = EOS;
    }

    wga_attr[ADSTAT_REG].readonly = wga_attr[VPD0_REG].readonly =
      wga_attr[VPD1_REG].readonly = wga_attr[ERRADDR_REG].readonly =
        wga_attr[VIDEO_ROM_REG].readonly = TRUE;

    wga_attr[ADCNTL_REG].addr    = (ulong_t) ADCNTL_ADDR;
    wga_attr[ADSTAT_REG].addr    = (ulong_t) ADSTAT_ADDR;
    wga_attr[WORIG_REG].addr     = (ulong_t) WORIG_ADDR;
    wga_attr[VPD0_REG].addr      = (ulong_t) VPD0_ADDR;
    wga_attr[VPD1_REG].addr      = (ulong_t) VPD1_ADDR;
    wga_attr[ERRADDR_REG].addr   = (ulong_t) ERRADDR_ADDR;
    wga_attr[VIDEO_ROM_REG].addr = (ulong_t) VIDEO_ROM_ADDR;

    wga_attr[WORIG_REG].mask = 0x7ff07ff;

    wga_attr[ADCNTL_REG].name = "ADCNTL";


    /* initialize all IGC video control registers                            */
    for(i = 0; i < IGC_REG_NUM; i++)
    {
      igc_attr[i].readonly = igc_attr[i].writeonly = FALSE;
      igc_attr[i].addr = 0;
      igc_attr[i].name = EOS;
      igc_attr[i].mask = 0x0fff;
    }

    /* initialize all Video Control Registers                                */
    igc_attr[HRZC_REG].readonly = igc_attr[VRTC_REG].readonly =
      igc_attr[SRADDR_REG].readonly = TRUE;

    igc_attr[USER_REG].readonly =
      igc_attr[STATUS_REG].readonly = TRUE;

    igc_attr[PLANE_MASK_REG].writeonly = igc_attr[FGROUND_REG].writeonly =
     igc_attr[BGROUND_REG].writeonly = igc_attr[PIXEL1_REG].writeonly =
       igc_attr[PIXEL8_REG].writeonly = igc_attr[W_MIN_XY_REG].writeonly =
         igc_attr[W_MAX_XY_REG].writeonly = TRUE;

    igc_attr[HRZC_REG         ].addr = (ulong_t) HRZC_REG_ADDR;
    igc_attr[HRZT_REG         ].addr = (ulong_t) HRZT_REG_ADDR;
    igc_attr[HRZSR_REG        ].addr = (ulong_t) HRZSR_REG_ADDR;
    igc_attr[HRZBR_REG        ].addr = (ulong_t) HRZBR_REG_ADDR;
    igc_attr[HRZBF_REG        ].addr = (ulong_t) HRZBF_REG_ADDR;
    igc_attr[PREHRZC_REG      ].addr = (ulong_t) PREHRZC_REG_ADDR;
    igc_attr[VRTC_REG         ].addr = (ulong_t) VRTC_REG_ADDR;
    igc_attr[VRTT_REG         ].addr = (ulong_t) VRTT_REG_ADDR;
    igc_attr[VRTSR_REG        ].addr = (ulong_t) VRTSR_REG_ADDR;
    igc_attr[VRTBR_REG        ].addr = (ulong_t) VRTBR_REG_ADDR;
    igc_attr[VRTBF_REG        ].addr = (ulong_t) VRTBF_REG_ADDR;
    igc_attr[PREVRTC_REG      ].addr = (ulong_t) PREVRTC_REG_ADDR;
    igc_attr[SRADDR_REG       ].addr = (ulong_t) SRADDR_REG_ADDR;
    igc_attr[SRTCTL_REG       ].addr = (ulong_t) SRTCTL_REG_ADDR;
    igc_attr[DAC_ADL_REG      ].addr = (ulong_t) DAC_ADL_REG_ADDR;
    igc_attr[DAC_ADH_REG      ].addr = (ulong_t) DAC_ADH_REG_ADDR;
    igc_attr[COLOR_PALETTE_RAM].addr = (ulong_t) COLOR_PALETTE_RAM_REG_ADDR;
    igc_attr[CURCMND_REG      ].addr = (ulong_t) CURCMND_REG_ADDR;
    igc_attr[USER_REG         ].addr = (ulong_t) USER_REG_ADDR;
    igc_attr[STATUS_REG       ].addr = (ulong_t) USER_REG_ADDR;
    igc_attr[FGROUND_REG].addr = (ulong_t) DRAW_ENGINE_ADDR | FGROUND_REG_VAL;
    igc_attr[BGROUND_REG].addr = (ulong_t) DRAW_ENGINE_ADDR | BGROUND_REG_VAL;
    igc_attr[PLANE_MASK_REG].addr = (ulong_t) DRAW_ENGINE_ADDR |
                                                          PLANE_MASK_REG_VAL;
    igc_attr[RASTER_REG].addr  = (ulong_t) DRAW_ENGINE_ADDR | RASTER_REG_VAL;
    igc_attr[W_MIN_XY_REG].addr = (ulong_t) DRAW_ENGINE_ADDR | W_MIN_REG_VAL;
    igc_attr[W_MAX_XY_REG].addr = (ulong_t) DRAW_ENGINE_ADDR | W_MAX_REG_VAL;

    igc_attr[CINDEX_REG].addr = (ulong_t) PARM_ENGINE_CNTL_ADDR | CINDEX_REG_VAL;
    igc_attr[W_OFFSET_REG].addr = (ulong_t) PARM_ENGINE_CNTL_ADDR | W_OFFSET_REG_VAL;
    igc_attr[PIXEL1_REG].addr = (ulong_t) PIXEL1_REG_ADDR;
    igc_attr[PIXEL8_REG].addr = (ulong_t) PIXEL8_REG_ADDR;

    igc_attr[INTERRUPT_REG].addr = (ulong_t) WTKN_INTR_ADDR;
    igc_attr[INTERRUPT_ENBL_REG].addr = (ulong_t) WTKN_INTR_ENBL_ADDR;

    igc_attr[HRZC_REG].mask = igc_attr[PREHRZC_REG].mask =
      igc_attr[VRTC_REG].mask = igc_attr[PREVRTC_REG].mask =
        igc_attr[SRADDR_REG].mask = 0;

    igc_attr[USER_REG].mask = igc_attr[STATUS_REG].mask = 0xc000000f;
    igc_attr[RASTER_REG].mask = 0x3ffff;
    igc_attr[SRTCTL_REG].mask = 0x1ff;
    igc_attr[DAC_ADL_REG].mask = 0xff;
    igc_attr[DAC_ADH_REG].mask = 0x0f;

    igc_attr[CURCMND_REG].mask = igc_attr[COLOR_PALETTE_RAM].mask = 0xff;
    igc_attr[CINDEX_REG].mask = 0x03;

    init_done = TRUE;
  }

  return;
}



/*
 * NAME : wait_for_wtkn_ready
 *
 * DESCRIPTION :
 *
 *   Waitting for the 'busy' bit in the status register is set to zero.
 *   The host can read or write the Drawing Engine's registers only when
 *   the drawing engine is idle.  Attempting to do so when the drawing
 *   engine is busy, an undefined results will be produced.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/



void wait_for_wtkn_ready (void)
{
  ulong_t  busy;

  do
  {
    get_igc_reg ((uchar_t) STATUS_REG, &busy);   /* get status register      */
    busy &= IGM_STATUS_BUSY_MASK;                /* retain busy bit (30)     */
  } while ( busy ); /* enddo */                  /* wait for not busy        */

  return;
}




/*
 * NAME : disable_video
 *
 * DESCRIPTION :
 *
 * Disables video at the IGC control register (srtctl)
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void disable_video(void)
{
  ulong_t cntl;

  TITLE("disable_video");

  get_igc_reg ((uchar_t) SRTCTL_REG, &cntl);
  cntl &= ~(ENABLE_VIDEO);
  write_igc_reg ((uchar_t) SRTCTL_REG, cntl);

  return;
}



/*
 * NAME : enable_video
 *
 * DESCRIPTION :
 *
 * Enables video at the IGC control register (srtctl)
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/
void enable_video(void)
{
  ulong_t cntl;

  TITLE("enable_video");

  get_igc_reg ((uchar_t) SRTCTL_REG, &cntl);
  cntl |= ENABLE_VIDEO;
  write_igc_reg ((uchar_t) SRTCTL_REG, cntl);

  return;
}



/*
 * NAME : get_cursor_status
 *
 * DESCRIPTION :
 *
 *  Get the status of the cursor
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Cursor status.
 *
*/

ulong_t get_cursor_status (void)
{
  ulong_t  addr, cntl;

  TITLE("get_curosr_status");

  load_dac_reg ((ulong_t)CURSOR_CMD_REG_OFFSET); /* setup cursor_cmd_reg_addr*/
  get_igc_reg ((uchar_t) CURCMND_REG, &cntl);    /* get cursor status reg.   */

  return (cntl);
}



/*
 * NAME : disable_cursor
 *
 * DESCRIPTION :
 *
 *  Disables cursor.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void disable_cursor (void)
{
  ulong_t  addr, cntl;

  TITLE("disable_cursor");

  load_dac_reg ((ulong_t)CURSOR_CMD_REG_OFFSET); /* setup cursor_cmd_reg_addr*/
  get_igc_reg ((uchar_t) CURCMND_REG, &cntl);    /* get cursor status        */

  cntl &= CURSOR_OFF;

  load_dac_reg ((ulong_t)CURSOR_CMD_REG_OFFSET);/* setup cursor_cmd_reg_addr*/
  write_igc_reg ((uchar_t)CURCMND_REG, cntl);    /* disable 64x64 cursor     */

  return;
}


/*
 * NAME : enable_cursor
 *
 * DESCRIPTION :
 *
 *  Enables cursor.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void enable_cursor(void)
{
  ulong_t cntl, addr;

  TITLE("enable_cursor");

  load_dac_reg ((ulong_t) CURSOR_CMD_REG_OFFSET);  /* ld with cursor cmd reg */
  get_igc_reg ((uchar_t) CURCMND_REG, &cntl);    /* get cursor cmd register  */

  cntl |= CURSOR_ON;                             /* setup to turn on cursor  */

  load_dac_reg ((ulong_t) CURSOR_CMD_REG_OFFSET);  /* ld with cursor cmd reg */
  write_igc_reg ((uchar_t)CURCMND_REG, cntl);    /* enable 64x64 cursor      */


  return;
}


/*
 * NAME : set_reg_type0
 *
 * DESCRIPTION :
 *
 *  Sets register "type 0" value.
 *  A "type 0" register is a window-origin-like register.
 *
 * INPUT :
 *
 *   1. X parameter.
 *   2. Y parameter.
 *   3. Register index.
 *
 * OUTPUT :
 *
 *   Register updated.
 *
 * RETURNS :
 *
 *   TRUE if successful, else FALSE.
 *
*/

BOOL set_reg_type0(ulong_t xprm, ulong_t yprm, ulong_t reg)
{
  ulong_t x, y;

  TITLE("set_reg_type0");

  write_wga_reg (yprm * 0x10000 + xprm, (uchar_t) reg);

  get_reg_type0(&x, &y, (uchar_t) reg);

  return (x == xprm && y == yprm);
}



/*
 * NAME : set_worig
 *
 * DESCRIPTION :
 *
 *  Sets window origin register.
 *
 * INPUT :
 *
 *   1. X coordinate..
 *   2. Y coordinate.
 *
 * OUTPUT :
 *
 *   Register updated.
 *
 * RETURNS :
 *
 *   TRUE if successful, else FALSE.
 *
*/

BOOL set_worig(ulong_t xorig, ulong_t yorig)
{
  TITLE("set_worig");

  return(set_reg_type0(xorig, yorig, (uchar_t) WORIG_REG));
}



/*
 * NAME : get_reg_type0
 *
 * DESCRIPTION :
 *
 *  Gets register "type 0" value.
 *  A "type 0" register is a window-origin-like register.
 *
 * INPUT :
 *
 *   1. Pointer to place to store X parameter.
 *   2. Pointer to place to store Y parameter.
 *   3. Register index.
 *
 * OUTPUT :
 *
 *   1. X parameter.
 *   2. Y parameter.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void get_reg_type0(ulong_t *x, ulong_t *y, ulong_t reg)
{
  ulong_t tmp;

  TITLE("get_reg_type0");

  get_wga_reg(&tmp, (uchar_t) reg);
  *x = tmp & 0x7ff;
  *y = tmp / 0x10000;

  return;
}



/*
 * NAME : get_worig
 *
 * DESCRIPTION :
 *
 *  Gets window origin coordinates.
 *
 * INPUT :
 *
 *   1. Pointer to place to store X coordinate..
 *   2. Pointer to place to store Y coordinate.
 *
 * OUTPUT :
 *
 *   1. X coordinate..
 *   2. Y coordinate.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void get_worig(ulong_t *x, ulong_t *y)
{
  TITLE("get_worig");

  get_reg_type0(x, y, (uchar_t) WORIG_REG);

  return;
}



/*
 * NAME : set_wclip
 *
 * DESCRIPTION :
 *
 *  Sets window clipping coordinates.  No validation will be done
 *  on input.
 *
 * INPUT :
 *
 *  1. Upper left corner X coordinate.
 *  2. Upper left corner Y coordinate.
 *  3. Bottom right corner X coordinate.
 *  4. Bottom right corner Y coordinate.
 *
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_wclip (ulong_t xstart, ulong_t ystart, ulong_t xend, ulong_t yend)
{
  ulong_t  packed_xy;
  ulong_t  busy;

  TITLE("set_wclip");

  wait_for_wtkn_ready ();                        /* must wait for WTKN chip  */
  packed_xy = IGM_PACK (xstart, ystart);
  write_igc_reg ((uchar_t) W_MIN_XY_REG, packed_xy);

  packed_xy = IGM_PACK (xend, yend);
  write_igc_reg ((uchar_t) W_MAX_XY_REG, packed_xy);

  return;
}



/*
 * NAME : set_w_offset
 *
 * DESCRIPTION :
 *
 *
 * INPUT :
 *
 *  1. Pattern origin x
 *  2. Pattern origin y
 *
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_w_offset (ulong_t x, ulong_t y)
{
  ulong_t  packed_xy;

  TITLE ("set_w_offset");

  packed_xy = IGM_PACK (x, y);
  write_igc_reg ((uchar_t) W_OFFSET_REG, packed_xy);

  return;
}




/*
 * NAME : set_cursor_pos
 *
 * DESCRIPTION :
 *
 *  Sets cursor position.  The cursor position x & y values are
 *  calculated as follow:
 *    xpos = desired display screen (xpos) position + H - P
 *    where :
 *      P    = 37 for 1:1 multiplexing.
 *             52 for 4:1 multiplexing.
 *             57 for 5:1 multiplexing.
 *
 *  For monitors less than 133 MHZ:
 *      H = (HRZBR + 1) * 4
 *
 *  For monitors greater than 133 MHZ:
 *      H = ((HRZBR + 1) * 8
 *
 *    ypos = desired display screen (ypos) position + V - 32
 *    where :
 *      V = (VRTBR - 1) + (VRTT - VRTBF)
 *
 *
 * INPUT :
 *
 *   1. X coordinate.
 *   2. Y coordinate.
 *
 * OUTPUT :
 *
 *   Cursor position register updated.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS;
 *
*/

#define MULTIPLEX_SELECT_MASK       0xC0

int set_cursor_pos(ulong_t xpos, ulong_t ypos)
{
  ulong_t cntl, adpcntl, xval, yval;
  int     p, h, v, vrtbr, vrtt, vrtbf, x, y;
  int     rc;

#ifdef DEBUG_WGA
  static BOOL    done = FALSE;                   /* internal flag - FALSE    */
#endif

  TITLE("set_cursor_pos");

  rc = SET_CURSOR_ERR;
  load_dac_reg ((ulong_t) COMMAND_REGISTER_0_OFFSET);
  get_igc_reg ((uchar_t) CURCMND_REG, &cntl);

#ifdef DEBUG_WGA
  if (!done)
  {
    printf ("\nMultiplex select    =  0x%08x", cntl);
  } /* endif */
#endif

  /* calculated the cursor x value */
  /* extracting register CR07 & CR06 (Multiplex select, bits 7 & 6) */
  p = PIXEL_SELECTS [(cntl & MULTIPLEX_SELECT_MASK) >> 6];
  get_igc_reg ((uchar_t) HRZBR_REG, &cntl);

  get_wga_reg(&adpcntl, (uchar_t) ADCNTL_REG);

  if (adpcntl & DOTCLK_FREQ_MASK)
     h = ((int) cntl  + 1) * 8;                 /* for monitors > 133 MHZ    */
  else
     h = ((int) cntl + 1) * 4;                  /* for monitors <= 133 MHZ   */


  x = (int) xpos + h - p;

#ifdef DEBUG_WGA
  if (!done)
  {
    printf ("\nHRZBR               =  0x%08x", cntl);
  } /* endif */
#endif

  /* calculated the cursor y value */
  get_igc_reg ((uchar_t) VRTBR_REG, &cntl);
  vrtbr = (int) cntl;
  get_igc_reg ((uchar_t) VRTT_REG, &cntl);
  vrtt = (int) cntl;
  get_igc_reg ((uchar_t) VRTBF_REG, &cntl);
  vrtbf = (int) cntl;
  v = (vrtbr - 1) + (vrtt - vrtbf);
  y = (int) ypos + v - 32;

#ifdef DEBUG_WGA
  if (!done)
  {
    printf ("\nVRTBR               =  0x%08x", vrtbr);
    printf ("\nVRTT                =  0x%08x", vrtt);
    printf ("\nVRTBF               =  0x%08x\n", vrtbf);
    done = TRUE;                                 /* reset the internal flg */
  } /* endif */
#endif

  /* start with cursor x low register, we write x & y coordinate to RAMDAC */
  load_dac_reg ((ulong_t) CURSOR_X_LOW_REG_OFFSET);
  write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) x & 0xFF);
  write_igc_reg ((uchar_t) CURCMND_REG, ((ulong_t) x >> 8) & 0x0F);
  write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) y & 0xFF);
  write_igc_reg ((uchar_t) CURCMND_REG, ((ulong_t) y >> 8) & 0x0F);

  get_cursor_pos (&xval, &yval);
  rc = (xval == xpos) && (yval == ypos) ? SUCCESS : SET_CURSOR_ERR;

  return (rc);
}



/*
 * NAME : get_cursor_pos
 *
 * DESCRIPTION :
 *
 *  Get cursor position.  The cursor position x & y values are
 *  calculated as follow:
 *    xpos = displayed screen (xpos) position - H + P
 *    where :
 *      P    = 37 for 1:1 multiplexing.
 *             52 for 4:1 multiplexing.
 *             57 for 5:1 multiplexing.
 *
 *  For monitors less than 133 MHZ:
 *      H = (HRZBR + 1) * 4
 *
 *  For monitors greater than 133 MHZ:
 *      H = ((HRZBR + 1) * 8
 *
 *
 *    ypos = displayed screen (ypos) position - V + 32
 *    where :
 *      V = (VRTBR - 1) + (VRTT - VRTBF)
 *
 *
 * INPUT :
 *
 * 1. Address of X coordiante
 * 2. Address of Y coordiante
 *
 * OUTPUT :
 *
 * 1. X coordinate.
 * 2. Y coordiante.
 *
 * RETURNS :
 *
 *   None.
 *
*/

#define D3_BIT               11

void get_cursor_pos (ulong_t *xpos, ulong_t *ypos)
{
  ulong_t  vrtbr, vrtt, vrtbf, cntl, adpcntl, cxlr, cxhr, cylr, cyhr;
  int      v, h, p, x, y;

  TITLE ("get_cursor_pos");

  load_dac_reg ((ulong_t) COMMAND_REGISTER_0_OFFSET);
  get_igc_reg ((uchar_t) CURCMND_REG, &cntl);

  /* extracting register CR07 & CR06 (Multiplex select, bits 7 & 6) */
  p = PIXEL_SELECTS [ (cntl & MULTIPLEX_SELECT_MASK) >> 6 ];
  get_igc_reg ((uchar_t) HRZBR_REG, &cntl);

  get_wga_reg(&adpcntl, (uchar_t) ADCNTL_REG);

  if (adpcntl & DOTCLK_FREQ_MASK)
     h = ((int) cntl + 1) * 8;                   /* for monitors > 133 MHZ   */
  else
     h = ((int) cntl + 1) * 4;                   /* for monitors <= 133 MHZ  */

  get_igc_reg ((uchar_t) VRTBR_REG, &vrtbr);
  get_igc_reg ((uchar_t) VRTT_REG, &vrtt);
  get_igc_reg ((uchar_t) VRTBF_REG, &vrtbf);
  v = ((int) vrtbr - 1) + ((int) vrtt - (int) vrtbf);

  /* start with cursor x low register, we write x & y coordinate to RAMDAC */
  load_dac_reg ((ulong_t) CURSOR_X_LOW_REG_OFFSET);
  get_igc_reg ((uchar_t) CURCMND_REG, &cxlr);
  get_igc_reg ((uchar_t) CURCMND_REG, &cxhr);
  *xpos = cxlr | (cxhr << 8);

  get_igc_reg ((uchar_t) CURCMND_REG, &cylr);
  get_igc_reg ((uchar_t) CURCMND_REG, &cyhr);
  *ypos = cylr | (cyhr << 8);

  *xpos = (int) *xpos - h + p;
  y = sign_extend (*ypos, (uchar_t) D3_BIT);
  *ypos = (ulong_t) (y - v + 32);

  return;
}



/*
 * NAME : disable_int
 *
 * DESCRIPTION :
 *
 *  Disable the Error / Weitek interrupt.  Both of these interrupt are
 *  defined in the adapter control register.
 *
 * INPUT :
 *
 *  1. Interrupt type (ERROR_INT / WTKN_INT)
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Failed to disable or SUCCESS
 *
*/

BOOL disable_int(uchar_t int_type)
{
  ulong_t cntl, mask;
  BOOL  good;

  TITLE("disable_int");

  good = (((mask = get_int_mask (int_type)) != 0) ? TRUE : FALSE);

  if(good)                                       /* if valid interrupt, cont.*/
  {
    get_wga_reg(&cntl, (uchar_t) ADCNTL_REG);    /* get adapter cntrl reg.   */
    cntl = (cntl | mask) ^ mask;                 /* disable the interrupt    */

    write_wga_reg(cntl, (uchar_t) ADCNTL_REG);   /* save the register        */
  }

  return (good);
}




/*
 * NAME : enable_int
 *
 * DESCRIPTION :
 *
 *  Enable the Error / Weitek interrupt.  Both of these interrupt are
 *  defined in the adapter control register.
 *
 * INPUT :
 *
 *  1. Interrupt type (ERROR_INT / WTKN_INT)
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Failed to enable or SUCCESS
 *
*/

BOOL enable_int (uchar_t int_type)
{
  ulong_t cntl, mask;
  BOOL rc;

  TITLE("enable_int");

  rc = (((mask = get_int_mask (int_type)) != 0) ? TRUE : FALSE);

  if(rc == TRUE)                                 /* if valid interrupt, cont.*/
  {
    get_wga_reg(&cntl, (uchar_t) ADCNTL_REG);    /* get the adaper cntrl reg.*/
    cntl |= mask;                                /* enable interrupt mask    */

    write_wga_reg(cntl, (uchar_t) ADCNTL_REG);
  }

  return(rc);
}



/*
 * NAME : get_int_mask
 *
 * DESCRIPTION :
 *
 *  Get the interrupt mask (ERROR / WEITEK).  Both of these interrupt are
 *  defined in the adapter control register.
 *
 * INPUT :
 *
 *  1. Interrupt type (ERROR_INT / WTKN_INT)
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  A mask for the interrupt or invalid mask (0).
 *
*/

static ulong_t get_int_mask (uchar_t int_type)
{
  ulong_t mask = 0;

  TITLE("get_int_mask");

  if(int_type == ERROR_INT)
  {
    mask = E_INTR_ON;                            /* mask for error interrupt */
  }
  else
    if (int_type == WTKN_INT)
    {
      mask = WTKN_INTR_ON;                       /* mask for WEITEK interrupt*/
    }

  return(mask);
}



/*
 * NAME : load_dac_reg
 *
 * DESCRIPTION :
 *
 *  Set up the low and high (DAC_ADL / DAC_ADH) byte register for the
 *  RAMDAC.  These two register must be set up before any register of
 *  the RAMDAC can be accessed.
 *
 * INPUT :
 *
 *  1. Address of RAMDAC to be accessed
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  None.
 *
*/

void load_dac_reg (ulong_t addr)
{
  TITLE ("load_dac_reg");

  /* setup low byte for address register (DAC_ADL)                           */
  write_igc_reg ((uchar_t) DAC_ADL_REG, addr & 0x000000FF);

  /* setup high byte for address register (DAC_ADH)                          */
  write_igc_reg ((uchar_t) DAC_ADH_REG, (addr >> 8) & 0x000000FF);

  return;
}

