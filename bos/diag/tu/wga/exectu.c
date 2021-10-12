static char sccsid[] = "@(#)69  1.2.1.5  src/bos/diag/tu/wga/exectu.c, tu_wga, bos411, 9428A410j 3/4/94 17:48:37";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: exectu
 *              get_dply_time
 *              get_input_parm
 *              get_tu_func
 *              get_tu_name
 *              run_tu
 *              set_dply_time
 *              set_tu_errno
 *              update_wga_parm
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "vpd.h"



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
  VPD_TU,               vpd_tu,                "VPD test",
  REGISTER_TU,          reg_tu,                "Registers test",
  VRAM_TU,              vram_tu,               "VRAM test",
  PALETTE_TU,           palette_tu,            "Palette RAM test",
  CURSOR_RAM_TU,        cursor_ram_tu,         "Cursor RAM test",
  INTERRUPT_TU,         interrupt_tu,          "Interrupt test",
  VIDEO_ROM_SCAN_TU,    video_rom_scan_tu,     "Video ROM Scan test",
  STRING_TEST_TU,       string_test_tu,        "Load/Store string test",
  FAST_COPY_TU,         fast_copy_tu,          "Fast Copy test",
  PIXEL8_STR_TST_TU,    pixel8_str_tst_tu,     "Pixel8 String Test Tu",

  BLACK_TU,             black_tu,              "Screen black",
  RED_TU,               red_tu,                "Screen red",
  GREEN_TU,             green_tu,              "Screen green",
  BLUE_TU,              blue_tu,               "Screen blue",
  WHITE_TU,             white_tu,              "Screen white",
  COLOR_BAR_TU,         colorbar_tu,           "Color bars",
  WB_9X9_TU,            wb9x9_tu,              "WB9x9 grid",
  WB_9X11_TU,           wb9x11_tu,             "WB9x11 grid",
  SQUARE_BOX_50MM_TU,   sqr_box_50mm_tu,       "50mm Square Box",
  DISPLAY_AT_TU,        display_AT_tu,         "Display @ patterns",

  TU_CLOSE,             tu_close ,             "Exit graphics mode",
  TU_OPEN,              tu_open,               "Enter graphics mode",

#ifdef ALL_TUS
  ADVANCED_DPLY_TU,     advanced_dply_tu,      "Advanced display",
  CURSOR_TU,            dply_cursor_tu,        "display cursor",
  SCROLL_H_GRAPH_TU0,   scroll_h_gmode_tu0,    "Scroll Hs in graphic mode0",
  SCROLL_H_GRAPH_TU1,   scroll_h_gmode_tu1,    "Scroll Hs in graphic mode1",
  RGB_TU,               rgb_tu,                "Red/green/blue screen",
  BW_X_64_TU,           bw64_tu,               "BW64 grid",
  BW9X11_DOTS_TU,       bw9x11_dots_tu,        "BW 9x11 grid (dots)",
  WB9X11_DOTS_TU,       wb9x11_dots_tu,        "WB 9x11 grid (dots)",
  BLIT_TU,              blit_tu,               "Blit Tu",
  PIXEL1_TU,            pixel1_tu,             "Pixel1 Tu",
  PIXEL8_TU,            pixel8_tu,             "Pixel8 Tu",

  LUMINANCE_TU,         luminance_fall_off_tu, "Luminance Falls Off",
  LUMINANCE_TU_1,       luminance_tu_1,        "Luminance Vert X0",
  LUMINANCE_TU_2,       luminance_tu_2,        "Luminance Horz X0",
  LUMINANCE_TU_3,       luminance_tu_3,        "Luminance Vert XX0",
  LUMINANCE_TU_4,       luminance_tu_4,        "Luminance Horz XX0",
  LUMINANCE_TU_5,       luminance_tu_5,        "Luminance Vert X00",
  LUMINANCE_TU_6,       luminance_tu_6,        "Luminance Horz X00",
  LUMINANCE_TU_7,       luminance_tu_7,        "Luminance Vert XX00",
  LUMINANCE_TU_8,       luminance_tu_8,        "Luminance Horz XX00",
  LUMINANCE_TU_9,       luminance_tu_9,        "Luminance Vert X0XX0",
  LUMINANCE_TU_10,      luminance_tu_10,       "Luminance Horz X0XX0",
  LUMINANCE_TU_11,      luminance_tu_11,       "Luminance Vert X00X0",
  LUMINANCE_TU_12,      luminance_tu_12,       "Luminance Horz X00X0"
#endif
};

static int tu_errno = 0;
static WGAPARM wga_parm;

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES                                                       */
/* ------------------------------------------------------------------------- */

static int run_tu(PTUFCN, WGAPARM *);
static void update_wga_parm(WGAPARM *);
static PTUFCN get_tu_func(int);

char *logical_dev_name = "";


/*
 * NAME : exectu
 *
 * DESCRIPTION :
 *
 *   Entry point for all Test Units.
 *
 * INPUT :
 *
 *   1. Logical device name of the device to be tested.
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

int exectu(char *logical_device_name, TUCB *tucb_ptr)
{
  int      rc, loops;
  PTUFCN   tu;

  TITLE("exectu");

  rc = SUCCESS;

  if ( (tu = get_tu_func(tucb_ptr->header.tu)) != NULL) 

  {
    strcpy(logical_dev_name, logical_device_name);
    memcpy(&wga_parm, &tucb_ptr->wga, sizeof(wga_parm));

    for (loops = 0; rc == SUCCESS && loops < tucb_ptr->header.loop; loops++)
    {
      if((rc = run_tu(tu, &tucb_ptr->wga)) != SUCCESS)
        break;
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
 *   2. Pointer to WGAPARM structure.
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

static int run_tu(PTUFCN tu, WGAPARM *wga)
{
  int  rc = SUCCESS;

  TITLE("run_tu");

  tu_errno = 0;

  if ((tu -> number == TU_OPEN) | (tu -> number == TU_CLOSE))
     rc = tu -> pfcn();
  else
    {
       /* If we are not in Graphics Mode then we MUST enter graphics mode  */
       /* so that we can talk to the adapter.  All register will be saved*/
       /* and then restored when exit graphics mode.                      */

       if (!in_graphics())
       {
         DEBUG_MSG ("Invalid mode. Switching to graphics mode ....");
         rc = OPEN_RCM_ERROR;
       }
       else
       {
         disable_video ();                       /* do not display screen yet*/
         enable_int (ERROR_INT);
         rc = reset_wga_error();                 /* clear cstatus error bits */

         if(rc == SUCCESS)
         {
           wga_initialize ();
           disable_cursor ();
           set_worig (0, 0);

           enable_video ();                      /* enable video for every TU*/
           rc = tu -> pfcn();                    /* execute TU in GRAPHX mode*/

           if(rc == SUCCESS)
           {
             check_wga_error(&rc);               /* check for errors         */
             disable_int(ERROR_INT);
           }

           update_wga_parm (wga);                /* output parameters        */
         }
		 else
           enable_video ();                     /* enable video if error     */
       } /* endif */
  } /* end */

  return(rc);
}




/*
 * NAME : update_wga_parm
 *
 * DESCRIPTION :
 *
 *   Updates wga variable.
 *
 * INPUT :
 *
 *   Pointer to WGAPARM structure.
 *
 * OUTPUT :
 *
 *  WGA variable updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void update_wga_parm(WGAPARM *wga)
{
  TITLE("update_wga_parm");

  update_mem_info(&wga->mem_test);
  wga->errno = tu_errno;

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

char *get_tu_name(ulong_t tu)
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

WGAPARM *get_input_parm(void)
{
  TITLE("get_input_parm");

  return(&wga_parm);
}
