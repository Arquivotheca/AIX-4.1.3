/* @(#)69	1.3  src/bos/kernext/lft/inc/sys/pwr_mgr.h, lftdd, bos411, 9435D411a 9/1/94 20:16:36 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993-1994.
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PWR_MGR
#define _H_PWR_MGR 1


#include <sys/watchdog.h>

/* Power Manager Commands */


#define START_LFT_DPM_WD		1	/* start 2 watchdog timers to manage DPM 
                                                 * for default display 
                                                 */

#define STOP_LFT_DPM_WD			2	/* stop  2 watchdog timers   */

#define RESTART_LFT_DPM_WD		3	/* user issued "chdisp -d", so readjust 
                                                 * watchog to monitor new default display 
                                                 */

#define REMOVE_LFT_DPM_WD		4	/* kill the watchdog timers  */


/* Return Codes */
#define PWRPROC_SUCC		0	/* success */
#define BAD_COMMAND		1	/* bad command enqued */
#define BAD_PD			2	/* bad pd passed in */
#define DVC_FAILED		3	/* device driver failed to change phase */
#define ILLEGAL_OP		4	/* illegal operation */


/* -> don't need this structure, since design has been changed 
 * However I can't remove it until 4.2, becuase it is used in display.h
 */

typedef struct {
	struct watchdog wd;
	void * pd;
	ulong  reserved1;    /* reserve for later use */ 
	ulong  reserved2;
} pwr_mgr_t;


#endif  /* _H_PWR_MGR */
