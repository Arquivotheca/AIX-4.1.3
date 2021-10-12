/* @(#)73       1.7  src/bos/kernel/sys/watchdog.h, sysproc, bos411, 9428A410j 7/27/93 19:32:35 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: w_start
 *              w_stop
 *
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_WATCHDOG
#define _H_WATCHDOG

/*
 * The following is the watchdog timer structure.
 *
 * This structure should be initialize prior to calling w_init.
 * The next and prev fields should be set to NULL. The func and
 * restart fields set to the appropriate value. The count field
 * should be set to 0.
 */
struct watchdog {
	struct watchdog	*next;		/* next watchdog timer */
	struct watchdog	*prev;		/* previous watchdog timer */
	void		(*func)();	/* completion handler */
	ulong		count;		/* time remaining */
	ulong		restart;	/* restart time */
};


/*
 * The following are the services exported by the kernel to manage
 * watchdog timers.
 */
int w_init(struct watchdog *);          /* define watchdog timer */
/*
 * returns 0 if OK,
 *         1 if watchdog queue in use, try later
 */

int w_clear(struct watchdog *);          /* remove watchdog timer */
/*
 * returns 0 if OK,
 *         1 if watchdog queue in use, try later
 */

void w_start(struct watchdog *);          /* restart watchdog timer */

void w_stop(struct watchdog *);           /* stop watchdog timer    */

/*
 * The following service is only called by the system timer and is not
 * exported to kernel extensions.
 */

void watchdog();			/* process watchdog timers */


#endif /* _H_WATCHDOG */
