static char sccsid[] = "@(#)62  1.1.1.5  src/bos/diag/tu/wga/Sigmom.c, tu_wga, bos411, 9428A410j 3/4/94 17:48:24";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: in_graphics
 *              get_monitor_type
 *              get_screen_dim
 *              get_screen_res
 *              in_graphics
 *              make_map
 *              unmake_map
 *              tu_open
 *              tu_close
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


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/termio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/rcm_win.h>                 /*must go before include of aixgsc.h*/
#include <sys/aixgsc.h>
#include <sys/rcmioctl.h>

#include "wga_reg.h"
#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga.h"



/* Global variables   */
extern char *logical_dev_name;

/* Local variables    */
static BOOL inside_graphics = FALSE;
static struct wga_map wgadat;
static gsc_handle  gs_handle;
static int  rcm_fd;   


/* Local function prototypes.        */
static int make_map(void);
static int unmake_map(void);


#define LFT_DEV_NAME            "/dev/rcm0"

/*
 * NAME : in_graphics
 *
 * DESCRIPTION :
 *
 *  Determines if we are inside Monitor Mode.
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
 *  TRUE if in Monitor mode, else FALSE.
 *
*/

BOOL in_graphics (void)
{
  TITLE("in_graphics");

  return (inside_graphics);
}



/*
 * NAME : unmake_map
 *
 * DESCRIPTION :
 *
 * Unmakes WGA map.
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

static int unmake_map(void)
{
  unmake_gp unmakemap;
  int rc = SUCCESS;

  TITLE("unmake_map");

  errno = 0;                                     /* just in case             */
  if (aixgsc(gs_handle.handle, UNMAKE_GP, &unmakemap))
  {
    set_tu_errno();
    rc = UNMAKE_GP_ERR;
    LOG_SYSERR("aixgsc() failed unmaking map");
  }
  else
    bus_base_addr = 0;
       
  return (rc);
}



/*
 * NAME : make_map
 *
 * DESCRIPTION :
 *
 * Makes WGA map.
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

static int make_map (void)
{
  make_gp makemap;
  int rc = SUCCESS;

  TITLE("make_map");

  /* get handle */
  errno = 0;                    /* just in case   */

  strcpy (gs_handle.devname, logical_dev_name);
  
  if (ioctl(rcm_fd, GSC_HANDLE, (void *) &gs_handle) < 0 )
   {
    set_tu_errno();
    rc = DEVICE_BUSY_ERROR;
    LOG_SYSERR ("display device is busy"); 
  }
  else
  {
    makemap.pData = (genericPtr) &wgadat;
    makemap.length = sizeof(wgadat);

    /* This call will return a device dependent structure with memory mapping*/
    /* information.                                                          */

    errno = 0;                                   /* just in case             */
    if (aixgsc(gs_handle.handle, MAKE_GP, &makemap))
    {
      set_tu_errno();
      rc = MAKE_GP_ERR;
      LOG_SYSERR("aixgsc() failed making map");
    }
    else
      /* We need to detect for the special kloodge to the WGA Device        */
      /* Driver.  This kloodge will appear in bos321 only.                  */
      /* When the aixgsc call to perform "make-gp" is done for the          */
      /* WGA adapter, the parameter packet has both a device independent    */
      /* and a device dependent portion.  In the "correct" architecture     */
      /* the segment addressing info is returned in the device INdependent  */
      /* portion, member name 'segment' in struct _make_gp. For WGA, bos321 */
      /* only, the information is instead returned in the device DEpendent  */
      /* portion, member name 'baseaddr' in struct wga_map.                 */
      /* If the value in 'baseaddr' after the make-gp call has zero upper   */
      /* four bits, the 'segment' contains the correct bus addressing data. */
      /* Otherwise, 'baseaddr' contains the correct bus address.  Refer to  */
      /* defect 48819 & 46977 in bos321 for more details.                   */

      bus_base_addr = (wgadat.segment_register & 0xF0000000) == 0 ?
                       (ulong_t) makemap.segment : wgadat.segment_register;
   }
    return(rc); 
 }




/*
 * NAME : get_screen_dim
 *
 * DESCRIPTION :
 *
 * Determines screen dimmensions from device driver in mm.
 *
 * INPUT :
 *
 *  1. Pointer to place to store screen width.
 *  2. Pointer to place to store screen height.
 *
 * OUTPUT :
 *
 *  Screen dimmensions.
 *
 * RETURNS :
 *
 *  None.
 *
*/

void get_screen_dim(ulong_t *width, ulong_t *height)
{
  TITLE("get_screen_dim");

  *width = wgadat.screen_width_mm;
  *height = wgadat.screen_height_mm;

  return;
}



/*
 * NAME : get_screen_res
 *
 * DESCRIPTION :
 *
 *  Gets screen resolution (in pixels).
 *
 * INPUT :
 *
 *  1. Pointer to place to store X resolution.
 *  2. Pointer to place to store Y resolution.
 *
 * OUTPUT :
 *
 *   Screen resolution.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void get_screen_res (ulong_t *x, ulong_t *y)
{
  TITLE("get_screen_res");

  *x = wgadat.screen_width_pix;
  *y = wgadat.screen_height_pix;

  return;
}

 

/*
 * NAME : tu_close
 *
 * DESCRIPTION :
 *
 * Exits graphics Mode.
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

int tu_close (void)
{
  int  rc = SUCCESS;

  if (in_graphics()) 
  {
    restore_regs ();
    rc = unmake_map ();                          /* detact from memory bus   */

    inside_graphics = FALSE;
    close (rcm_fd);
  }

  return (rc);
}



/*
 * NAME : tu_open
 *
 * DESCRIPTION :
 *
 * Enter graphics  Mode.
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

int tu_open (void)
{
  int  rc = SUCCESS;

  if (!in_graphics()) 
  {
    errno  = 0;
    if ((rcm_fd = open (LFT_DEV_NAME, O_RDWR)) < 0)    /* open the rcm       */
    {
      set_tu_errno;
      rc = OPEN_RCM_ERROR;
      LOG_SYSERR ("Open '/dev/rcm' failed");
    }
    else
    {
      if ((rc = make_map ()) == SUCCESS)             /* make WGA memory map  */
      {      
        inside_graphics = TRUE;                      /*turn on flag indicator*/
        save_regs ();
      }
	  else
	  {  /* Did not map the WGA memory; therefore, we must close the adapter */
        inside_graphics = FALSE;                     /* just to make sure    */
		close (rcm_fd);                              /* close the adapter    */
      }	
    }
  }

  return (rc);
}
