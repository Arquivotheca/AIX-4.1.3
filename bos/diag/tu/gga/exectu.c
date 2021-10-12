static char sccsid[] = "@(#)67	1.1  src/bos/diag/tu/gga/exectu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:50";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: exectu
 *              get_dply_time
 *              get_input_parm
 *              get_tu_func
 *              get_tu_name
 *              run_tu
 *              set_dply_time
 *              set_tu_errno
 *              update_gga_parm
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <ggadds.h>
#include <setjmp.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggareg.h"
#include "ggapci.h"
#include "ggamisc.h"

extern jmp_buf ctxt;

typedef int (*PFCN) (void);

typedef struct
{
  int           number;
  PFCN          pfcn;
  char         *desc;
} TUFCN;

typedef TUFCN *PTUFCN;


static TUFCN tu_fcn[] =
{
  REGISTER_TU,          reg_tu,                 "Registers test",
  VRAM_TU,              vram_tu,                "VRAM test",
  PALETTE_TU,           palette_tu,             "Palette RAM test",
  PALETTECHECK_TU,      palettecheck_tu,        "New Palette RAM test",
  CURSOR_RAM_TU,        cursor_ram_tu,          "Cursor RAM test",
  STRING_TEST_TU,       string_test_tu,         "Load/Store string test",
  PIXEL8_STR_TST_TU,    pixel8_str_tst_tu,      "Pixel8 String Test Tu",
  TU_OPEN,              tu_open,                "Open LFT for tests",
  TU_CLOSE,             tu_close,               "Close LFT after tests",

  BLACK_TU,             black_tu,               "Screen black",
  RED_TU,               red_tu,                 "Screen red",
  GREEN_TU,             green_tu,               "Screen green",
  BLUE_TU,              blue_tu,                "Screen blue",
  WHITE_TU,             white_tu,               "Screen white",
  COLOR_BAR_TU,         colorbar_tu,            "Color bars",
  WB_9X9_TU,            wb9x9_tu,               "WB9x9 grid",
  WB_9X11_TU,           wb9x11_tu,              "WB9x11 grid",
  SQUARE_BOX_50MM_TU,   sqr_box_50mm_tu,        "50mm Square Box",

#ifdef ALL_TUS
  ADVANCED_DPLY_TU,     advanced_dply_tu,       "Advanced display",
  CURSOR_TU,            dply_cursor_tu,         "display cursor",
  SCROLL_H_DRAW,        scroll_h_draw,          "Draw/Blit scrolling Hs",
  SCROLL_H_GRAPH_TU0,   scroll_h_gmode_tu0,     "Scroll Hs in graphic mode0",
  SCROLL_H_GRAPH_TU1,   scroll_h_gmode_tu1,     "Scroll Hs in graphic mode1",
  RGB_TU,               rgb_tu,                 "Red/green/blue screen",
  BW_X_64_TU,           bw64_tu,                "BW64 grid",
  BW9X11_DOTS_TU,       bw9x11_dots_tu,         "BW 9x11 grid (dots)",
  WB9X11_DOTS_TU,       wb9x11_dots_tu,         "WB 9x11 grid (dots)",
  BLIT_TU,              blit_tu,                "Blit Tu",
  PIXEL1_TU,            pixel1_tu,              "Pixel1 Tu",
  PIXEL8_TU,            pixel8_tu,              "Pixel8 Tu",
  SCROLL_H_PIXEL1_EMC_TU, scroll_h_pixel1_emc,  "Scroll Hs using PIXEL1 (EMC)",
  VIDEO_TU,             video_tu,               "Display captured video",
  SCROLL_H_PIXEL1_TU,   scroll_h_pixel1,        "Scroll Hs using PIXEL1",

  LUMINANCE_TU,         luminance_fall_off_tu,  "Luminance Falls Off",
  LUMINANCE_TU_1,       luminance_tu_1,         "Luminance Vert X0",
  LUMINANCE_TU_2,       luminance_tu_2,         "Luminance Horz X0",
  LUMINANCE_TU_3,       luminance_tu_3,         "Luminance Vert XX0",
  LUMINANCE_TU_4,       luminance_tu_4,         "Luminance Horz XX0",
  LUMINANCE_TU_5,       luminance_tu_5,         "Luminance Vert X00",
  LUMINANCE_TU_6,       luminance_tu_6,         "Luminance Horz X00",
  LUMINANCE_TU_7,       luminance_tu_7,         "Luminance Vert XX00",
  LUMINANCE_TU_8,       luminance_tu_8,         "Luminance Horz XX00",
  LUMINANCE_TU_9,       luminance_tu_9,         "Luminance Vert X0XX0",
  LUMINANCE_TU_10,      luminance_tu_10,        "Luminance Horz X0XX0",
  LUMINANCE_TU_11,      luminance_tu_11,        "Luminance Vert X00X0",
  LUMINANCE_TU_12,      luminance_tu_12,        "Luminance Horz X00X0",
#endif
};


static int tu_errno = 0;
static GGAPARM gga_parm;

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES                                                       */
/* ------------------------------------------------------------------------- */

static int run_tu(PTUFCN, GGAPARM *);
static void update_gga_parm(GGAPARM *);
static PTUFCN get_tu_func(int);


/*
 * NAME : exectu
 *
 * DESCRIPTION :
 *
 *   Entry point for all Test Units.
 *
 * INPUT :
 *
 *   1. Logical device name for GGA
 *   2. Pointer to TUCB structure.
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

int exectu(char *ldn, TUCB *tucb_ptr)
{
  int      rc, loops;
  PTUFCN   tu;

  TITLE("exectu");

  rc = SUCCESS;
  if ( (tu = get_tu_func(tucb_ptr->header.tu)) != NULL )
  {
    memcpy(&gga_parm, &tucb_ptr->gga, sizeof(gga_parm));

    /* TU_OPEN */
    if (tucb_ptr->header.tu == TU_OPEN)
      {
        rc = tu_open(ldn);
        return(rc);
      }

    /* TU_CLOSE */
    if (tucb_ptr->header.tu == TU_CLOSE)
      {
        rc = tu_close();
        return(rc);
      }

    for (loops = 0; rc == SUCCESS && loops < tucb_ptr->header.loop; loops++)
    {
      /* SIGRETRACT handler will return here.              */
      /* We will come here to restart a test that is       */
      /* interrupted when the user changes to a different  */
      /* virtual terminal. We will not make the longjmp to */
      /* here until we have had access to the monitor      */
      /* granted again. See sky_retract() in Sigmom.c      */

      setjmp(ctxt);

      if((rc = run_tu(tu, &tucb_ptr->gga)) != SUCCESS)
        {
          break;
        }
    }  /* end for */
  }
  else
    rc = TU_NUMBER_ERR;                          /* bad TU number           */

  return(rc);
}



/*
 * NAME : run_tu
 *
 * DESCRIPTION :
 *
 *   Executes TU.
 *
 * INPUT :
 *
 *   1. TU number.
 *   2. Pointer to GGAPARM structure.
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

static int run_tu(PTUFCN tu, GGAPARM *gga)
{
  int  rc;
  ULONG  reg, cntl;

  TITLE("run_tu");

  rc = SUCCESS;  /* TU found */
  tu_errno = 0;

  disable_video();                        /* do not display screen yet */

  if (rc == SUCCESS)
    {
      gga_initialize ();
      disable_cursor ();
      enable_video ();                   /* enable video for every TU */
      rc = tu -> pfcn();                 /* execute TU                */
      update_gga_parm (gga);             /* output parameters         */
    }
   enable_video ();

  return(rc);
}




/*
 * NAME : update_gga_parm
 *
 * DESCRIPTION :
 *
 *   Updates gga variable.
 *
 * INPUT :
 *
 *   Pointer to GGAPARM structure.
 *
 * OUTPUT :
 *
 *  GGA variable updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void update_gga_parm(GGAPARM *gga)
{
  TITLE("update_gga_parm");

  update_mem_info(&gga->mem_test);
  gga->errno = tu_errno;

  return;
}


/*
 * NAME : set_tu_errno
 *
 * DESCRIPTION :
 *
 *   Updates tu_errno with global variable errno
 *   when a system call fails.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   tu_errno variable updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_tu_errno(void)
{
  TITLE("set_tu_errno");

  tu_errno = errno;
}


/* NAME : get_tu_func
 *
 * DESCRIPTION :
 *
 *   Gets Test Unit function
 *
 * INPUT :
 *   Test Unit number
 *
 * OUTPUT :
 *   A pointer to Test Unit function
 *   Return code (NULL / pointer to a Test Unit function)
 *   Test Unit error number
*/

static PTUFCN get_tu_func (int tu_no)
{
  int i;
  PTUFCN tu;

  TITLE("get_tu_func");

  tu = NULL;                                     /* default test unit error  */

  for (i = 0; i < (sizeof(tu_fcn) / sizeof(TUFCN)); i++)
  {
    if (tu_fcn[i].number == tu_no)
    {
      tu = &tu_fcn[i];                           /* return pointer to a TU   */
      break;
    }
  }

  return (tu);
}



/*
 * NAME : get_tu_name
 *
 * DESCRIPTION :
 *
 *   Gets Test Unit description
 *
 * INPUT :
 *
 *   Test Unit number
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Returns Test Unit description.
 *
*/

char *get_tu_name(ULONG tu)
{
  int i;

  TITLE("get_tu_name");

  for(i = 0; i < (sizeof(tu_fcn) / sizeof(TUFCN)); i++)
  {
    if(tu_fcn[i].number == tu)
      return(tu_fcn[i].desc);
  }

  return("NO SUCH TU ...");
}



/*
 * NAME : get_input_parm
 *
 * DESCRIPTION :
 *
 *   Retrieves input parameters.
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
 *   Pointer to input parameters.
 *
*/

GGAPARM *get_input_parm(void)
{
  TITLE("get_input_parm");

  return(&gga_parm);
}




/*
 * NAME : set_dply_time
 *
 * DESCRIPTION :
 *
 *  Used to set time visual patterns stay on the screen.
 *
 * INPUT :
 *
 *   1. Time in seconds a visual pattern will stay on the screen.
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

static ULONG dply_time = DEFAULT_DPLY_TIME;

void set_dply_time(ULONG time)
{
  TITLE("set_dply_time");

  dply_time = time;

  return;
}



/*
 * NAME : get_dply_time
 *
 * DESCRIPTION :
 *
 *  Returns the time a visual pattern will stay on the screen.
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
 *   Time in seconds a visual pattern will stay on the screen.
 *
*/

ULONG get_dply_time(void)
{
  TITLE("get_dply_time");

  return(dply_time);
}
