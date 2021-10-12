static char sccsid[] = "@(#)85	1.1  src/bos/diag/tu/gga/regtu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:22";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: disable_cursor
 *              disable_video
 *              enable_cursor
 *              enable_video
 *              get_all_gga_regs
 *              get_all_igc_regs
 *              get_cursor_pos
 *              get_cursor_status1400
 *              get_gga_reg
 *              get_igc_reg
 *              igc_do_blit
 *              igc_draw_quad
 *              igc_write_2D_coord_reg
 *              igc_write_2D_meta_coord
 *              igc_write_pixel1
 *              igc_write_pixel8
 *              initialize_reg_attr1140
 *              load_dac_addr_reg_co_color_read
 *              load_dac_addr_reg_co_color_write
 *              load_dac_addr_reg_pc_RAM_read
 *              load_dac_addr_reg_pc_RAM_write
 *              reg_tu
 *              restore_regs
 *              save_regs
 *              set_cursor_pos
 *              set_w_offset
 *              set_wclip
 *              wait_for_wtkn_ready1290
 *              write_all_gga_regs1008
 *              write_all_igc_regs1082
 *              write_gga_reg
 *              write_igc_reg
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

#include "ggareg.h"
#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"

#define EOS                     '\0'             /* End Of String            */
#define DISABLE_IGC_VIDEO       0x40
#define REG_TEST_LOOPS          5000             /* test 5000 times          */

static int PIXEL_SELECTS [] = { 0, 52, 37, 57 };

static struct
{
  BOOL readonly;
  BOOL writeonly;
  ULONG addr;
  ULONG mask;
  char *name;
} igc_attr [IGC_REG_NUM];


static IGC_REGS_TYPE keep_igc_regs;

static void write_all_igc_regs(IGC_REGS_TYPE);
static void get_all_igc_regs(IGC_REGS_TYPE);
static ULONG get_int_mask(UCHAR);

static BOOL set_reg_type0(ULONG, ULONG, ULONG);
static void get_reg_type0(ULONG *, ULONG *, ULONG);


/*
 * NAME : reg_tu
 *
 * DESCRIPTION :
 *
 *  The purpose of this TU is to verify the integrity of all registers
 *  on the graphic adapter (GGA & IGC registers).  Each register of
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
  ULONG i, j;
  ULONG tmp0, tmp1;
  int     rc;
  static IGC_REGS_TYPE  igc_regs;

#ifdef LOGMSGS
  char msg[80];
#endif

  TITLE("reg_tu");

  rc = SUCCESS;
  disable_video ();                              /* do not show random data  */

  /* don't mess with the following registers                                 */
  igc_attr [INTERRUPT_REG].readonly = TRUE;
  igc_attr [INTERRUPT_ENBL_REG].readonly = TRUE;

  /* Now, test all of the IGC registers                                    */
  for(j = 0; rc == SUCCESS && j < IGC_REG_NUM; j++)
  {
    tmp0 = GET_RAND_ULONG & igc_attr [j].mask; /* get random data to test  */

    if(j == SRTCTL_REG)
      tmp0 &= ~(ENABLE_VIDEO);                 /* keep video always disable*/

    if(!write_igc_reg((UCHAR) j, tmp0) || !get_igc_reg((UCHAR) j, &tmp1))
      continue;

    if(tmp0 != tmp1)                           /* error if does not match  */
    {
      rc = REG_COMPARE_ERR;

    }  /* end if */
  }  /* end for */

  /* clear errors in case some illegal values were programmed */
  restore_regs ();                             /* restore default conditions */

  /* Now, we must restore the default GGA and/or IGC registers status        */
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
 *  Saves IGC registers in save area.
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

  get_all_igc_regs(keep_igc_regs);
}


/*
 * NAME : restore_regs
 *
 * DESCRIPTION :
 *
 *  Retsores IGC registers from save area.
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

  write_all_igc_regs(keep_igc_regs);
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
  UCHAR i;

  TITLE("get_all_igc_regs");

  for(i = 0; i < IGC_REG_NUM; i++)
  {
    if ((i != BLIT_CMD_REG) && (i != QUAD_CMD_REG) &&
        (i != PIXEL1_REG) && (i != PIXEL8_REG))
      get_igc_reg (i, &reg[i]);
  }
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

BOOL get_igc_reg(UCHAR reg, ULONG *val)
{
  TITLE("get_igc_reg");

  if (reg >= IGC_REG_NUM || igc_attr[reg].writeonly)
    return(FALSE);

  __iospace_eieio();
  *val = IGC_REG_READ (igc_attr [reg].addr) & igc_attr[reg].mask;
  __iospace_eieio();

  return (TRUE);
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

BOOL write_igc_reg(UCHAR reg, ULONG val)
{
  TITLE("write_igc_reg");

  if (reg >= IGC_REG_NUM || igc_attr[reg].readonly)
    return(FALSE);

  __iospace_eieio();
  IGC_REG_WRITE (igc_attr[reg].addr, val);
  __iospace_eieio();

  return (TRUE);
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

void igc_write_2D_meta_coord (ULONG vtype, ULONG rel,
                              ULONG yx, ULONG packed_xy)
{
  ULONG addr;

  TITLE("igc_write_2D_meta_coord");

  addr = (ULONG) (COORD_PSEUDO_REG_ADDR) | vtype | rel | yx;

  __iospace_eieio();
  IGC_REG_WRITE (addr, packed_xy);
  __iospace_eieio();
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

void igc_write_2D_coord_reg (ULONG rel, ULONG yx,
                             ULONG reg, ULONG val)
{
  ULONG addr;

  TITLE("igc_write_2D_coord_reg");

  addr = (ULONG) (PARAM_ENG_COORD_REG_ADDR) | reg << 6 | rel | yx;

  __iospace_eieio();
  IGC_REG_WRITE (addr, val);
  __iospace_eieio();
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

void igc_write_pixel1 (UCHAR count, ULONG data)
{
  ULONG addr, offset;

  TITLE("igc_write_pixel1");

  count -= 1;                                    /* offset from 0 - count    */
  offset = (count << 2) & 0x7C;                  /* strip off other bits     */
  addr = igc_attr [PIXEL1_REG].addr | offset;

#ifdef DUMP_REGS
  printf ("\nWrite to Address = 0x%08x       data = 0x%08x", addr, data);
  fflush(stdout);
#endif

  __iospace_eieio();
  IGC_REG_WRITE (addr, data);
  __iospace_eieio();
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

void igc_write_pixel8 (ULONG data)
{
  TITLE("igc_write_pixel8");

#ifdef DUMP_REGS
  printf ("\nWrite to Address = 0x%08x       data = 0x%08x",
          igc_attr[PIXEL8_REG].addr, data);
  fflush(stdout);
#endif

  __iospace_eieio();
  IGC_REG_WRITE (igc_attr [PIXEL8_REG].addr, data);
  __iospace_eieio();
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
  ULONG  busy;

  TITLE("igc_draw_quad");

  /* Invoke draw command */
  __iospace_eieio();
  do
  {
    busy = IGC_REG_READ (igc_attr[QUAD_CMD_REG].addr);
    busy &= IGM_STATUS_QB_BUSY_MASK;
    __iospace_eieio();
  } while ( busy );

  /* Wait for draw command to finish */
  /* do                                                                             */
  /* {                                                                              */
  /*   __iospace_eieio();                                                           */
  /*   busy = (IGC_REG_READ (igc_attr[STATUS_REG].addr)) & IGM_STATUS_QB_BUSY_MASK; */
  /*   __iospace_eieio();                                                           */
  /* }                                                                              */
  /* while ( busy );                                                                */
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
  ULONG  busy;

  TITLE("igc_do_blit");

  /* Invoke blit command */
  __iospace_eieio();
  do
  {
    busy = IGC_REG_READ (igc_attr[BLIT_CMD_REG].addr);
    __iospace_eieio();
    busy &= IGM_STATUS_QB_BUSY_MASK;
  } while ( busy );

  /* Wait for blit command to finish */
  /* do                                                                             */
  /* {                                                                              */
  /*   __iospace_eieio();                                                           */
  /*   busy = (IGC_REG_READ (igc_attr[STATUS_REG].addr)) & IGM_STATUS_QB_BUSY_MASK; */
  /*   __iospace_eieio();                                                           */
  /* }                                                                              */
  /* while ( busy );                                                                */
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

  for(i = 0; i < IGC_REG_NUM; i++)
  {
    if ((i != BLIT_CMD_REG) && (i != QUAD_CMD_REG) &&
        (i != PIXEL1_REG) && (i != PIXEL8_REG))
      write_igc_reg ((UCHAR) i, reg[i]);
  }
}



/*
 * NAME : initialize_reg_attr
 *
 * DESCRIPTION :
 *
 *  Initializes register attributes in local variable igc_attr
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *  Variable igc_attr[] initialized.
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
    /* initialize all IGC video control registers                            */
    for(i = 0; i < IGC_REG_NUM; i++)
    {
      igc_attr[i].readonly = igc_attr[i].writeonly = FALSE;
      igc_attr[i].addr = 0;
      igc_attr[i].name = EOS;
      igc_attr[i].mask = 0xffffffff;
    }

    igc_attr[HRZC_REG].readonly = igc_attr[VRTC_REG].readonly =
      igc_attr[SRADDR_REG].readonly = igc_attr[SRADDR_INC_REG].readonly =
        igc_attr[STATUS_REG].readonly = igc_attr[MEMCNFG_REG].readonly =
          igc_attr[RFPER_REG].readonly = igc_attr[RLMAX_REG].readonly =
            igc_attr[PU_CONFIG_REG].readonly = TRUE;


    igc_attr[PIXEL1_REG].writeonly = igc_attr[PIXEL8_REG].writeonly = TRUE;

    igc_attr[HRZC_REG          ].addr = (ULONG) W9100_HRZC;
    igc_attr[HRZT_REG          ].addr = (ULONG) W9100_HRZT;
    igc_attr[HRZSR_REG         ].addr = (ULONG) W9100_HRZSR;
    igc_attr[HRZBR_REG         ].addr = (ULONG) W9100_HRZBR;
    igc_attr[HRZBF_REG         ].addr = (ULONG) W9100_HRZBF;
    igc_attr[PREHRZC_REG       ].addr = (ULONG) W9100_PREHRZC;
    igc_attr[VRTC_REG          ].addr = (ULONG) W9100_VRTC;
    igc_attr[VRTT_REG          ].addr = (ULONG) W9100_VRTT;
    igc_attr[VRTSR_REG         ].addr = (ULONG) W9100_VRTSR;
    igc_attr[VRTBR_REG         ].addr = (ULONG) W9100_VRTBR;
    igc_attr[VRTBF_REG         ].addr = (ULONG) W9100_VRTBF;
    igc_attr[PREVRTC_REG       ].addr = (ULONG) W9100_PREVRTC;
    igc_attr[SRADDR_REG        ].addr = (ULONG) W9100_SRADDR;
    igc_attr[SRTCTL_REG        ].addr = (ULONG) W9100_SRTCTL;
    igc_attr[SRADDR_INC_REG    ].addr = (ULONG) W9100_SRADDR_INC;
    igc_attr[SRTCTL2_REG       ].addr = (ULONG) W9100_SRTCTL2;

    igc_attr[STATUS_REG        ].addr = (ULONG) W9100_STATUS;
    igc_attr[BLIT_CMD_REG      ].addr = (ULONG) W9100_BLIT_CMD;
    igc_attr[QUAD_CMD_REG      ].addr = (ULONG) W9100_QUAD_CMD;

    igc_attr[FGROUND_REG       ].addr = (ULONG) W9100_FOREGND;
    igc_attr[BGROUND_REG       ].addr = (ULONG) W9100_BACKGND;
    igc_attr[COLOR1_REG        ].addr = (ULONG) W9100_COLOR_1;
    igc_attr[COLOR0_REG        ].addr = (ULONG) W9100_COLOR_0;
    igc_attr[PLANE_MASK_REG    ].addr = (ULONG) W9100_PLANE_MASK;
    igc_attr[RASTER_REG        ].addr = (ULONG) W9100_RASTER;
    igc_attr[W_P_MIN_XY_REG    ].addr = (ULONG) W9100_P_WINMIN;
    igc_attr[W_P_MAX_XY_REG    ].addr = (ULONG) W9100_P_WINMAX;
    igc_attr[W_B_MIN_XY_REG    ].addr = (ULONG) W9100_B_WINMIN;
    igc_attr[W_B_MAX_XY_REG    ].addr = (ULONG) W9100_B_WINMAX;

    igc_attr[CINDEX_REG        ].addr = (ULONG) W9100_CINDEX;
    igc_attr[W_OFFSET_REG      ].addr = (ULONG) W9100_W_OFF_XY;
    igc_attr[PIXEL1_REG        ].addr = (ULONG) W9100_PIXEL1_CMD;
    igc_attr[PIXEL8_REG        ].addr = (ULONG) W9100_PIXEL8_CMD;

    igc_attr[MEMCNFG_REG       ].addr = (ULONG) W9100_MEMCNFG;
    igc_attr[RFPER_REG         ].addr = (ULONG) W9100_RFPER;
    igc_attr[RLMAX_REG         ].addr = (ULONG) W9100_RLMAX;
    igc_attr[PU_CONFIG_REG     ].addr = (ULONG) W9100_PU_CONFIG;

    igc_attr[SYSCONFIG_REG     ].addr = (ULONG) W9100_SYSCNFG;
    igc_attr[INTERRUPT_REG     ].addr = (ULONG) W9100_INTERRUPT;
    igc_attr[INTERRUPT_ENBL_REG].addr = (ULONG) W9100_INT_EN;

    igc_attr[HRZC_REG].mask = igc_attr[PREHRZC_REG].mask =
      igc_attr[VRTC_REG].mask = igc_attr[PREVRTC_REG].mask =
        igc_attr[SRADDR_REG].mask = igc_attr[SRADDR_INC_REG].mask = 0x0fff;

    igc_attr[RASTER_REG].mask  = 0x3ffff;
    igc_attr[SRTCTL_REG].mask  = 0xfff;
    igc_attr[SRTCTL2_REG].mask = 0x0f;

    igc_attr[STATUS_REG].mask = 0xc000000f;
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
 *   Waiting for the 'busy' bit in the status register is set to zero.
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
  ULONG  busy;

  do
  {
    get_igc_reg ((UCHAR) STATUS_REG, &busy);   /* get status register      */
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
  ULONG cntl;

  TITLE("disable_video");

  get_igc_reg ((UCHAR) SRTCTL_REG, &cntl);
  cntl &= ~(ENABLE_VIDEO);
  write_igc_reg ((UCHAR) SRTCTL_REG, cntl);

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
  ULONG cntl;

  TITLE("enable_video");

  get_igc_reg ((UCHAR) SRTCTL_REG, &cntl);
  cntl |= ENABLE_VIDEO;
  write_igc_reg ((UCHAR) SRTCTL_REG, cntl);

  get_igc_reg ((UCHAR) SRTCTL_REG, &cntl); /*@ DEBUG */

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

UCHAR get_cursor_status (void)
{
  ULONG  addr;
  UCHAR  cntl;

  TITLE("get_cursor_status");

  cntl = ReadIBM525(RGB525_CURSOR_CTL);

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
  ULONG  addr;
  UCHAR  tempc;

  TITLE("disable_cursor");

  tempc = ReadIBM525(RGB525_CURSOR_CTL);

  if ((tempc & RGB525_CURSOR_MODE_MASK) != RGB525_CURSOR_MODE_OFF)
    {
      tempc &= ~RGB525_CURSOR_MODE_MASK;
      WriteIBM525(RGB525_CURSOR_CTL, tempc);
    }

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
  ULONG addr, ltemp;
  UCHAR tempc;

  TITLE("enable_cursor");

  tempc = ReadIBM525(RGB525_CURSOR_CTL);

  if ((tempc & RGB525_CURSOR_MODE_MASK) != RGB525_CURSOR_MODE_0)
    {
      tempc &= ~RGB525_CURSOR_MODE_MASK;
      tempc |= RGB525_CURSOR_MODE_0;
      WriteIBM525(RGB525_CURSOR_CTL, tempc);
    }

  tempc = ReadIBM525(RGB525_CURSOR_CTL);

  if ((tempc & RGB525_CURSOR_SIZE_MASK) != RGB525_CURSOR_SIZE_64x64)
    {
      tempc &= ~RGB525_CURSOR_SIZE_MASK;
      tempc |= RGB525_CURSOR_SIZE_64x64;
      WriteIBM525(RGB525_CURSOR_CTL, tempc);
    }

  ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
  ltemp = RL(W9100_INDEXCONTR);
  tempc = ltemp >> 8;

  if ((tempc && INDEX_CONTROL_MASK) != INDEX_CONTROL_INC_ON)
    tempc |= INDEX_CONTROL_INC_ON;                            

  ltemp = tempc;
  ltemp |= (ltemp << 24) | (ltemp << 16) | (ltemp << 8);
  dac_workaround();
  ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
  WL(ltemp, W9100_INDEXCONTR);

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

void set_wclip (ULONG xstart, ULONG ystart, ULONG xend, ULONG yend)
{
  ULONG  packed_xy;
  ULONG  busy;

  TITLE("set_wclip");

  wait_for_wtkn_ready ();                        /* must wait for WTKN chip  */
  packed_xy = IGM_PACK (xstart, ystart);
  write_igc_reg ((UCHAR) W_P_MIN_XY_REG, packed_xy);
  write_igc_reg ((UCHAR) W_B_MIN_XY_REG, packed_xy); /* Assumes 8 bits/pixel */

  packed_xy = IGM_PACK (xend, yend);
  write_igc_reg ((UCHAR) W_P_MAX_XY_REG, packed_xy);
  write_igc_reg ((UCHAR) W_B_MAX_XY_REG, packed_xy); /* Assumes 8 bits/pixel */

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

void set_w_offset (ULONG x, ULONG y)
{
  ULONG  packed_xy;

  TITLE ("set_w_offset");

  packed_xy = IGM_PACK (x, y);
  write_igc_reg ((UCHAR) W_OFFSET_REG, packed_xy);

  return;
}



