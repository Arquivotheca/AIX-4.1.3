static char sccsid[] = "@(#)89	1.1  src/bos/diag/tu/gga/sigmom.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:28";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: SIGALRM_handler
 *              end_tu
 *              get_screen_dim
 *              get_screen_res
 *              tu_close
 *              tu_open
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
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/termio.h>
#include <sys/ioctl.h>
#include <setjmp.h>
#include <sys/rcm_win.h>       /* must go before include of aixgsc.h         */
#include <sys/aixgsc.h>
#include <sys/rcmioctl.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"
#include "ggapci.h"
#include "gga.h"

/* External variables */
extern unsigned long prefix, LE_prefix, BE_prefix;

/* Global variables   */
jmp_buf  ctxt;
unsigned int gga_x_max, gga_y_max, gga_mode;

static struct      gga_map ggadat;

static int         rcm_fdes;
static gsc_handle  our_gsc_handle;
static make_gp     makemap;

/* Local procedures */
static int SIGALRM_handler(int);


/*
 * NAME : tu_open
 *
 * DESCRIPTION :
 *
 * Open /dev/rcm0, obtain GSC_HANDLE and Make GGA map (MAKE_GP via aixgsc()).
 *
 * INPUT :
 *
 *  Logical device name
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

int tu_open (char *ldn)
{
  int rc, mode;
  ULONG reg, srtctl;

  /* open rcm  */
  if ((rcm_fdes = open("/dev/rcm0",O_RDWR)) < 0 )
    {
      set_tu_errno();
      rc = OPEN_RCM0_ERR;
      LOG_SYSERR("failure opening /dev/rcm0");
      return(rc);
    }

  strcpy(our_gsc_handle.devname,ldn);

  /* get handle */
  rc = ioctl(rcm_fdes,GSC_HANDLE, &our_gsc_handle);
  if (rc)
    {
      if (errno == EBUSY)
        {
          set_tu_errno();
          rc = GET_GSC_ERR;
          LOG_SYSERR("Display device is busy");
          close(rcm_fdes);     /* An error was detected so close the RCM */
          return(rc);
        }
      else
        {
          set_tu_errno();
          rc = OPEN_RCM0_ERR;
          LOG_SYSERR("failure attempting to get GSC handle");
          close(rcm_fdes);     /* An error was detected so close the RCM */
          return(rc);
        }
    }

  makemap.error  = 0;
  makemap.pData  = (genericPtr) &ggadat;
  makemap.length = sizeof(ggadat);
  makemap.access = EXCLUSIVE_ACCESS;

  /* This call will return a device dependent structure with memory */
  /* mapping information.                                           */

  errno = 0;                                   /* just in case             */
  if (aixgsc(our_gsc_handle.handle, MAKE_GP, &makemap))
  {
    set_tu_errno();
    rc = MAKE_GP_ERR;
    LOG_SYSERR("aixgsc() failed making map");
    close(rcm_fdes);     /* An error was detected so close the RCM */
    return(rc);
  }
  else
  {
    LE_prefix = ((ULONG) (makemap.segment + ggadat.base_address) | (ULONG) 0x60000);
    BE_prefix = (ULONG) (makemap.segment + ggadat.base_address);
    prefix = LE_prefix;
    monitor_type = (int) ggadat.monitor_type;
  }
  initialize_reg_attr ();                /* initialize reg addresses*/
  save_regs();
  reset_palette();                       /* initializes color palette */
  save_default_colors();

  switch(ggadat.screen_width_pix)
    {
    /* Set to least common denominator if 640x480 - in case TFT is attached. */ 
    /* The PIXEL1 and Scrolling H tests will check for TFT presence and set  */
    /* their modes accordingly.                                              */
    case 640: if (monitor_type == TFT_MONITOR)
                mode = MODE640X480X8_60HZ;
              else
                mode = MODE640X480X8;
              break; 
    case 1024: mode = MODE1024X768X8;  break;
    case 1280: mode = MODE1280X1024X8; break;
    default: return(FAIL);
    }

  if(modeset(mode) == FAIL) 
    {
      set_tu_errno();
      rc = SET_MODE_ERR;
      LOG_SYSERR("Failed to set display mode in tu_open()");
      close(rcm_fdes);     /* An error was detected so close the RCM */
      return(rc);
    }

  return(SUCCESS);
}




/*
 * NAME : tu_close
 *
 * DESCRIPTION :
 *
 * Unmake GP and close /dev/rcm0
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
  int rc;

  restore_default_colors();
  /*@ restore_regs(); */
  if (aixgsc(our_gsc_handle.handle, UNMAKE_GP, &makemap))
    {
      set_tu_errno();
      rc = OPEN_RCM0_ERR;
      LOG_SYSERR("Unmake aixgsc failure in TU_CLOSE");
      return(rc);
    }
  close(rcm_fdes);
  return(SUCCESS);
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

void get_screen_dim(ULONG *width, ULONG *height)
{
  TITLE("get_screen_dim");

  *width = ggadat.screen_width_mm;
  *height = ggadat.screen_height_mm;

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

void get_screen_res (ULONG *x, ULONG *y)
{
  TITLE("get_screen_res");

  *x = gga_x_max;
  *y = gga_y_max;

  return;
}



/*
 * NAME : end_tu
 *
 * DESCRIPTION :
 *
 * Used in visual TUs to determine when to exit. end_tu initiates a
 * timer the first time it is called and returns TRUE after the number
 * of seconds specified by the input parameter 'time' has elapsed. If
 * the timer hasn't expired yet it returns FALSE. If the input parameter
 * time = 0 then returns TRUE immediately.
 *
 *
 * INPUT :
 *
 *  1. Time (seconds).
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  TRUE if time expired, else FALSE.
 *
*/


static struct
{
  BOOL out;
  BOOL set;
} tu_time = {  FALSE, FALSE  };



#define UPDATE_TIME      50

BOOL end_tu(ULONG time)
{
  int rc;
  static int upd_time = 0;

  TITLE("end_tu");
  rc = TRUE;
  if (time != 0)                                 /* end tu by timer          */
  {
    if(!tu_time.set)                             /* initiate the time for 1st*/
    {
      tu_time.out = FALSE;
      signal (SIGALRM, ((void (*) (int)) SIGALRM_handler));/*SIGALRM interpt */
      alarm(time);
      tu_time.set = TRUE;
    }

    if (tu_time.out)                             /* time expired ?           */
    {
      tu_time.set = FALSE;
      rc = TRUE;
    }
    else
      rc = FALSE;
  }
  else
    rc = TRUE;

  if (rc == FALSE)
    if(++upd_time == UPDATE_TIME)
    {
      ALIVE_MSG;                    /* send this message to whoever needs it */
                                    /* to indicate the TU is still alive.    */
      upd_time = 0;
    }

  return(rc != 0);
}


/*
 * NAME : SIGALRM_handler
 *
 * DESCRIPTION :
 *
 * Timer signal handler.
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
 *  None.
 *
*/

static int SIGALRM_handler (int sig)
{
  TITLE("SIGALM_handler");

  tu_time.out = TRUE;

  return (SUCCESS);
}

