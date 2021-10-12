/* @(#)47	1.20  src/bos/kernel/sys/uthread.h, sysproc, bos41J, 9513A_all 3/24/95 15:25:24 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_UTHREAD
#define _H_UTHREAD

/*
 *	The uthread structure is an extension of the user area or u-block.
 *	There is one allocated per thread, including kernel threads.
 *	It is protected to disallow any access to it by user-mode code.
 *
 *	The uthread contains information about the thread that
 *	is local to the thread and never referenced by other threads.
 *
 */

#include <sys/types.h>
#include <sys/mstsave.h>
#include <sys/timer.h>

struct uthread {

	struct mstsave	ut_save; 	/* machine state save area */
	char		*ut_kstack;	/* own kernel stack */

	/* system call state */
	ulong_t		ut_scsave[8];	/* save area for system call handler */
	ulong_t		ut_msr;		/* saved user msr */
	struct auddata	*ut_audsvc;	/* auditing data */
	int		**ut_errnopp;	/* address of pointer to errno */
	char		ut_error;	/* return error code */
	char		ut_pad;
	short		ut_flags;	/* uthread flags */

	/* signal management */
	sigset_t	ut_oldmask;	/* mask from before sigpause */
	int		ut_code;	/* iar of instr. causing exception */
	char		*ut_sigsp;	/* special signal stack */

	/* miscellaneous */
	ulong_t		ut_fstid;	/* file system transaction ID */
	long		ut_ioctlrv;	/* return value for PSE ioctl's */
	void		*ut_selchn;	/* select control block */
	struct trb	*ut_timer[NTIMERS_THREAD]; /* thread timer array */
	struct uthread	*ut_link;	/* uthread blocks free list */
	void 		*ut_loginfo;	/* loginfo pointer */

	long		ut_extra[5];	/* padding to 32 byte boundary */
};

/*
 * uthread flags, ut_flags
 */
#define UTSTILLUSED	0x0001		/* albeit freed, entry still in use */
#define UTSIGONSTACK	0x0002		/* thread is on signal stack.  This */
					/* value was put into sc_onstack so */ 
					/* must keep the same value. */
#define UTNOULIMIT	0x0004		/* bypass ulimit check in kernel mode */
#define UTYIELD		0x0008		/* voluntarily relinquishing processor*/
#define UTSCSIG		0x0010		/* system call to sig_slih path taken */
#define UTSIGSUSP       0x0020          /* sigsuspend in effect */

#endif /* _H_UTHREAD */
