/* @(#)86	1.4  src/bos/kernext/rcm/inc/sys/rcmioctl.h, rcm, bos41J, 9512A_all 3/14/95 17:46:54 */

/*
 *   COMPONENT_NAME: rcm
 *
 * FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* -----------------------------------------------------------------
        RCM ioctl command defines
   -------------------------------------------------------------- */

#define RCM_IOC			('R'<<8)
#define GSC_HANDLE		(RCM_IOC | 1)	/* get gsc handle               */
#define RCM_QUERY		(RCM_IOC | 2)	/* query the RCM                */
#define RCM_SET_DIAG_OWNER	(RCM_IOC | 3)	/* acquire a display            */
#define GSC_HANDLE_RELEASE	(RCM_IOC | 4)	/* release gsc handle           */

#define GSC_GRAPHICS_INIT_STARTS	(RCM_IOC | 5)	/* X/diag starts initialization */

#define GSC_GRAPHICS_INIT_ENDS	(RCM_IOC | 6)	/* X/diag ends initialization   */

#define GSC_GRAPHICS_DATA_SAVED	(RCM_IOC | 7)	/* X has saved GRAPHICS data for*/
                                                /* device suspend/hibernaton    */

#define GSC_GRAPHICS_SAVE_FAILED	(RCM_IOC | 8)	
#define GSC_GRAPHICS_QUERY_PM_STATUS	(RCM_IOC | 9)	

/* ----------------------------------------------------------------
                Structure for getting Handle for aixgsc calls
   ---------------------------------------------------------------*/
typedef struct _gsc_handle
{
	char	devname[16];
	ulong	handle;
} gsc_handle;

