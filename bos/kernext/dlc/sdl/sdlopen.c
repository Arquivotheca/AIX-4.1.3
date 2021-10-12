static char sccsid[] = "@(#)81	1.13  src/bos/kernext/dlc/sdl/sdlopen.c, sysxdlcs, bos411, 9428A410j 2/14/94 11:11:05";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: 
 *	pr_open()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/

/*
**      File Name      : 81
**
**      Version Number : 1.12
**      Date Created   : 93/10/19
**      Time Created   : 12:45:24
*/

#include "sdlc.h"
#include <errno.h>
#include <fcntl.h>

extern	void	sdl_rx();
extern	void	sdl_status();


/* defect 126815 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 126815 */


/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:        pr_open                                                 */
/*                                                                      */
/* Description: open the physical link                                  */
/*                                                                      */
/* Function:    This function is called by the gdlc head and sends an   */
/*              open command to the device handler                      */
/*                                                                      */
/* Input:       SDLC port control block                                 */
/*                                                                      */
/* Output:      update port state                                       */
/*                                                                      */
/* Normal Exit: return RC_GOOD                                          */
/*                                                                      */
/* Error Exit:  ENODEV : device handler open failed                     */
/*                                                                      */
/* Return Type: int                                                     */
/*                                                                      */
/************************************************************************/

int     pr_open(cb)

PORT_CB	*cb;
{

	char			path[80];
        struct  kopen_ext       kopen_ext;

#ifdef MULT_PU
	short index;

	/*
	** removed mpx setup
	*/

	/* If the device is already open, return with NORMAL return code */
	if (cb->pl_status != CLOSED || cb->flags.device_opened)

	return(NORMAL);
#endif

	/* get lock on port cb */

/* defect 126815 */
  simple_lock(&cb->dlc.lock);
/* end defect 126815 */

	

#ifdef MULT_PU
	/* initialize mpu enablement. This will only be done on the first
	   open call. If the value id changes after that time, it will be
	   ignored */

	if (cb->dlc.cid->maxsaps > 1)
	{
		/* set the mpu enabled indicator */
		cb->flags.mpu_enabled = TRUE;

		/* set the max channels to 255 */
		/* Note - this has to be done here since max_opens()
		   does not get the port cb pointer */
		cb->dlc.maxchan = MAX_NUM_STATIONS;
	}
	else
	{
		cb->flags.mpu_enabled = FALSE;
		cb->dlc.maxchan = 1;
	}
#endif


        /* set up the control block to reflect current activity */
        cb->rc = NORMAL;

        /* set up the kopen_ext for the device handler          */
        kopen_ext.rx_fn = sdl_rx;
        kopen_ext.tx_fn = NULL;
        kopen_ext.stat_fn = sdl_status;
	kopen_ext.open_id = (ulong_t)cb;

	strcpy(path, "/dev/");
	strcpy(&path[5], cb->dlc.namestr);

	/* open the adapter */
        cb->rc = fp_open(path, O_RDWR, NULL, &kopen_ext, FP_SYS, &cb->fp);

        if (cb->sdllc_trace)
                sdlmonitor(cb, OPEN_DEVICE, 0, cb->rc, 0, 0);


        /* if there is a bad return code */
        if (cb->rc != NORMAL)
	{
		error_log(cb, ERRID_SDL8061, NON_ALERT, 0, FN, LN);
		cb->fp = 0;
	}
	else
	{
		cb->flags.device_opened = TRUE;
		cb->pl_status = CLOSED;
	}


/* defect 126815 */
  simple_unlock(&cb->dlc.lock);
/* end defect 126815 */




#ifdef MULT_PU
	/* bulletproof the code by making sure the array of stations point
           to NULL */
        for (index=0;index<=MAX_NUM_STATIONS;index++)
		   cb->link_station_array[index]=DLC_NULL;
#endif
        return(cb->rc);
}
