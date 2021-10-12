/* @(#)83       1.24  src/bos/kernel/sys/timer.h, sysproc, bos411, 9438A411a 9/14/94 18:25:26 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: TIMERID_ISBSD
 *              TIMERID_NOTVALID
 *
 *
 *   ORIGINS: 27,83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TIMER
#define _H_TIMER
/*
 *  TIMER MANAGEMENT DESIGN:
 *
 *  Each process may make use of up to m timers where m is equal to 1 
 *  timer for the alarm() functionality (AT&T and POSIX alarm(), BSD 
 *  ITIMER_REAL) plus 1 Berkeley user virtual timer (BSD ITIMER_VIRTUAL)
 *  plus 1 Berkeley user and system virtual timer (BSD ITIMER_PROF) 
 *  plus n POSIX 1003.4 realtime timer(s) (POSIX TIMEOFDAY).  The 
 *  u-block contains an array of pointers to timers, one for each of 
 *  the m timers a process may have.  The pointer corresponding to each
 *  timer remains NULL until allocation of that timer (via a call to 
 *  gettimerid()) causes the corresponding pointer to be initialized.
 *
 *  Indices into a process's array of timers are hard-coded.  This
 *  means that for all processes, the first element in the process's
 *  timer array, for example, will serve as the timer slot for, say, 
 *  that process's ITIMER_REAL type timer.
 *
 *  The system maintains a system-wide list of active timer requests.
 *  This is an ordered, doubly-linked, null-terminated list of timer 
 *  requests (trb structures) which have been submitted by all 
 *  processes, device drivers, etc. and have not yet expired.  Any
 *  per-process timer which is active will be on this system-wide 
 *  active list until that timer expires.
 *
 *  When a timeout request expires, it is processed on the timer inter-
 *  rupt level.  For this reason, all per-process timers must be pinned.
 *
 *  At each clock interrupt, the clock timeout routine determines if
 *  there are ITIMER_VIRTUAL and/or ITIMER_PROF timers active for the
 *  current process.  If so, those timers are updated appropriately.  
 *  Since this occurs on the timer interrupt level, these trb's must
 *  also be pinned.
 *
 *
 * u.u_timer___________________
 *________>|                   |___>       \
 *	   |  struct trb       |   .        \ per process timers (pinned)
 *	   |  *trb[NTIMERS]    |   .        /
 *	   |___________________|\_____     /
 *				      |
 *				      |
 *				      |
 *   _________________________________|
 *  |
 *  |
 *  |
 *  |       struct trb
 *  |        _____________________
 *  |______>|                     |
 *	    | struct trb          | \
 *	    | *knext              |  \
 *	    |_____________________|   \  threads through the system active trb
 *	    |                     |   /  list (doubly linked, null terminated)
 *	    | struct trb          |  /
 *	    | *kprev              | /
 *	    |_____________________|
 *	    |                     | process which owns the trb (valid for
 *	    | pid_t               | process timer services, not valid for 
 *	    | pid                 | device driver trb's)
 *	    |_____________________|
 *	    |                     |
 *	    | ulong               | state of the trb
 *	    | flags               |
 *	    |_____________________|
 *	    |                     |
 *	    | ulong		  | user's timer id number
 *	    | timerid		  |
 *	    |_____________________|
 *	    |			  |
 *	    | int		  |
 *	    | eventlist		  |
 *	    |_____________________|
 *          |                     |  .it_interval   timestruc_t
 *          |                     |_______________   time between timeouts
 *          | struct itimerstruc_t|                  for priodic timers
 *	    | timeout             |
 *	    |                     |  .it_value      timestruc_t
 *	    |                     |_______________   absolute time value of
 *	    |_____________________|                  next timeout
 *	    |                     |
 *	    |                     |
 *	    | void                |
 *	    | (*func)()           | timeout function to be called
 *	    |_____________________|
 *	    |                     |
 *	    | {ulong,             |
 *	    |  int,		  |
 *	    |  caddr_t}	 	  |
 *	    | func_data           | parameter to the timeout function
 *	    |_____________________|
 *	    |                     |
 *	    | int                 | interrupt priority level on which to
 *	    | ipri                | call timeout function
 *	    |_____________________|
 *	    |                     |
 *	    | void *              | timeout function
 *	    | tof()               |
 *	    |_____________________|
 *	  
 *	  
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/processor.h>

#define T_TIMEOUT	0xABABABAB	/* timeout/untimeout trb	*/

/*
 *  Flags for the timer request block (trb)
 */
#define	T_PENDING	0x01		/* timer is on the system list	*/
#define	T_ACTIVE	0x02		/* timer is in use	        */
#define	T_ABSOLUTE	0x04		/* timeout is an absolute value */
#define	T_INCINTERVAL	0x10		/* timer is an incinterval one	*/
#define	T_COMPLETE	0x20		/* timeout occurred and trb has	*/
					/* been taken off active list	*/
#define T_MPSAFE        0x40            /* used from mp safe driver     */

struct	trb  {
	struct trb 	*to_next;	/* for the timeout() routines	*/
	struct trb 	*knext;		/* next timer in kernel chain	*/
	struct trb 	*kprev;		/* previous timer in kern. chain*/
	ulong		id;		/* owner process, dev drvrs = -1*/
	cpu_t           cpunum;         /* owning processor             */
	unsigned short  flags;          /* operation flags              */
	ulong		timerid;	/* timer identifier		*/
	int		eventlist;	/* anchor for awaited event	*/
	struct itimerstruc_t timeout;	/* timer request		*/
	void		(*func)();	/* completion handler		*/
	union  parmunion  {		/* data for completion handler	*/
		ulong	data;		/* handler unsigned data	*/
		int	sdata;		/* handler signed data		*/
		caddr_t	addr;		/* handler address		*/
	}  t_union;
	int		ipri;		/* int. priority for func()	*/
	void		(*tof)();	/* timeout function		*/
#ifdef DEBUG
					/* debug only fields		*/
	void		(*d_func)();	/* completion handler		*/
	unsigned short  d_flags;	/* operation flags		*/
#endif
};

#define	func_data	t_union.data
#define	t_func_data	t_union.data
#define	t_func_sdata	t_union.sdata
#define	t_func_addr	t_union.addr

#define	TIMERID_ISBSD(timerid)						\
	(((timerid) == ITIMER_VIRTUAL) ||				\
	 ((timerid) == ITIMER_VIRT) ||					\
	 ((timerid) == ITIMER_PROF))

#ifdef _KERNEL

#define TIMERID_PROC(timerid)						\
       (((int)(timerid) >= 0) &&                                        \
        ((int)(timerid) < NTIMERS) &&                                   \
        (U.U_timer[timerid] != NULL))

#define TIMERID_THREAD(timerid)                                         \
       (((int)(timerid) >= TIMERID_REAL1) &&                            \
        ((int)(timerid) < TIMERID_REAL1 + NTIMERS_THREAD) &&            \
        (u.u_th_timer[timerid - TIMERID_REAL1] != NULL))

#define	TIMERID_NOTVALID(timerid)					\
	(!(TIMERID_PROC((timerid)) || TIMERID_THREAD((timerid))))

extern struct trb * talloc();
extern void         tstart();
extern int         tstop();
extern void         tfree();
extern int          delay();

#endif  /* _KERNEL */

#endif /* _H_TIMER */
