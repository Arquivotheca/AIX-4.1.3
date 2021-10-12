static char sccsid[] = "@(#)77	1.2  hga_open.c, tu_hga, htx410 6/21/94 17:26:29";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: ScreenInit
 *		close_normal_dd
 *		open_normal_dd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/rcmioctl.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/errno.h>

#include "hga_tu.h"
#include "screen.h"

#include <inc/hga.h>	/* this must come from kernext/disp/pcihga/inc */

#define NO_ACCESS  		0xE0
#define AIXGSC_MAKE_GP_FAILED	0xE1
#define	AIXGSC_UNMAKE_GP_FAILED	0xE2
#define	NORMAL_DD_ACCESS	0xE3
#define	OPEN_RCM_ERROR		0xE4
#define	E_BUSY			0xE5
#define	DEVICE_BUSY_ERROR	0xE6
#define	IOCTL_GSC_HANDLE_FAILED	0xE7

   /* Global variable to keep track of device access mode for this device.
    * For Baby Blue device access, two modes are available :
    *   1. Diagnostics Kernel Extensions  (DIAGEX_ACCESS)
    *   2. Normal Application Device Driver (NORMAL_DD_ACCESS)
    *
    * This variable is initialized to NO_ACCESS to disallow any type
    * of access prior to proper access mode initialization.
    * It will then be updated upon successful execution of initialization
    * and termination utilities for each access method.
    *
    * There are four utilities (invoked through TUCB) which use and update
    * the value of this global variable:
    *    1. OPEN_DIAGEX - changes the value to DIAGEX_ACCESS if and only
    *                     if the current value is NO_ACCESS.
    *    2. CLOSE_DIAGEX - changes the value to NO_ACCESS if and only if
    *                      the current value is DIAGEX_ACCESS.
    *    3. OPEN_NORMAL_DD - changes the value to NORMAL_DD_ACCESS if and
    *                        only if the current value is NO_ACCESS.
    *    4. CLOSE_NORMAL_DD - changes the value to NO_ACCESS if and only if
    *                         the current value is NORMAL_DD_ACCESS.
    */
int device_access_mode = NO_ACCESS;

/* GLOBAL data structures used for NORMAL DD ACCESS */
int 		rcm_file_descriptor;
int		hgaBaseAddress;
ulong		busBaseAddress;
ulong		vramBaseAddress;
ulong		ioBaseAddress;
int		pixelDepth;
gsc_handle	gscHandle;
make_gp		makegp_info;
unmake_gp	unmake;
struct hga_map	hga_makegp_info;
SCR_INFO	screen;
int		monitor;


unsigned long open_normal_dd(char *dev_name)
/* dev_name must be of the type hga0, hga1 */
{
  unsigned long ret_code = 0;
  ERROR_DETAILS error;

  /* open Rendering Context Manager for later access to LFT subsystem */
  rcm_file_descriptor = open("/dev/rcm0",O_RDWR);
  if (rcm_file_descriptor < 0) {
    error.system_call.error_code = ret_code = OPEN_RCM_ERROR;
    error.system_call.errno = errno;
    (void)log_error(error);
    return(ret_code);
  } /* endif */


  /* Initialize gsc_handle to be used for aixgsc calls */
  strcpy(gscHandle.devname,dev_name);
  if (ioctl(rcm_file_descriptor, GSC_HANDLE, &gscHandle) < 0 ){
    printf("\nioctl GSC_HANDLE failed\n");
    if (errno == E_BUSY) {
      error.system_call.error_code = ret_code = DEVICE_BUSY_ERROR;
      error.system_call.errno = errno;
      (void)log_error(error);
      close(rcm_file_descriptor);
      return(ret_code);
    } else {
      error.system_call.error_code = ret_code = IOCTL_GSC_HANDLE_FAILED;
      error.system_call.errno = errno;
      (void)log_error(error);
      close(rcm_file_descriptor);
      return(ret_code);
    } /* endif */
  } /* endif */

  /* make us a graphics process */
  makegp_info.error   = 0;
  makegp_info.pData = (genericPtr) &hga_makegp_info;
  makegp_info.length = sizeof(hga_makegp_info);
  makegp_info.access = EXCLUSIVE_ACCESS;

  ret_code = aixgsc(gscHandle.handle, MAKE_GP, &makegp_info);
  if (ret_code) {
    printf( "\naixgsc call MAKE_GP failed; ret_code = %x \n",ret_code);
    error.system_call.error_code = ret_code = AIXGSC_MAKE_GP_FAILED;
    error.system_call.errno = errno;
    (void)log_error(error);
    close(rcm_file_descriptor);
    return(ret_code);
  } /* endif */

  /* set up the base address for the adapter */
  hgaBaseAddress = (char *)makegp_info.segment;
  vramBaseAddress = hgaBaseAddress + hga_makegp_info.MEM_segment;
  ioBaseAddress       = hga_makegp_info.IO_segment;
  pixelDepth           = hga_makegp_info.pixel_depth;

  SetScissorSize();
  Bt485_init();
  ScreenInit();
  s3_load_palette((int)NULL);

#if 0
  printf("\n HGA BASE ADDRESS IS %x \n", hgaBaseAddress);
  printf("\n IO_segment %x \n", hga_makegp_info.IO_segment);
  printf("\n vramBaseAddress %x \n", vramBaseAddress);
  printf("\n MEM_segment %x \n", hga_makegp_info.MEM_segment);
  printf("\n screen_height_mm %x \n", hga_makegp_info.screen_height_mm);
  printf("\n screen_width_mm %x \n", hga_makegp_info.screen_width_mm);

  printf("\n screen_height_pix %x \n", hga_makegp_info.screen_height_pix);
  printf("\n screen_width_pix %x \n", hga_makegp_info.screen_width_pix);

  printf("\n monitor_type =  %x \n", hga_makegp_info.monitor_type);
#endif
  monitor = hga_makegp_info.monitor_type;
  device_access_mode = NORMAL_DD_ACCESS;
#if 0 /* usp */
  ret_code = init_device_info(dev_name);
  if (ret_code) {
    close_normal_dd();
  } /* endif */
#endif

  return(ret_code);

} /* end open_normal_dd() */


/*
 * NAME: close_normal_dd
 *
 * FUNCTION: closes Normal Device Driver for monitor mode access to Baby Blue.
 *
 * EXECUTION ENVIRONMENT: Process only.
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:  device_access_mode - global variable in this file
 *
 * RETURNS: This function returns zero if successful, else returns error code.
 */
/*
 * INPUT:
 *
 * OUTPUT:
 *
 */

close_normal_dd(void)
{
  unsigned long ret_code = 0;
  ERROR_DETAILS error;

  /* unmake the graphics process */
  ret_code = aixgsc(gscHandle.handle, UNMAKE_GP, &unmake);
  if (unmake.error) {                  /* ????? this needs investigation */
     printf("\nUNMAKE_GP ERROR\n");
     error.system_call.error_code = ret_code = AIXGSC_UNMAKE_GP_FAILED;
     error.system_call.errno = unmake.error;
     (void)log_error(error);
     return(ret_code);
  } /* endif */

  if (!ret_code) {
    device_access_mode = NO_ACCESS;
  } /* endif */

  close(rcm_file_descriptor);

  return(ret_code);

} /* end close_normal_dd() */

ScreenInit()
{
	screen.height = hga_makegp_info.screen_height_pix;
	screen.width  = hga_makegp_info.screen_width_pix;

	return 0;
}


/*
 * NAME : get_monitor_type
 *
 * DESCRIPTION :
 *
 *  Return monitor type to the caller
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
 *  Monitor type.
 *
*/

ulong_t get_monitor_type(void)
{
  return (monitor); 
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
	*width =  hga_makegp_info.screen_width_mm;
	*height = hga_makegp_info.screen_height_mm;
	return ;
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
	*x = hga_makegp_info.screen_width_pix;
	*y = hga_makegp_info.screen_height_pix;

	return ;
}

