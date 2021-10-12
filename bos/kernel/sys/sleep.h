/* @(#)44	1.11.1.13  src/bos/kernel/sys/sleep.h, sysproc, bos411, 9437C411a 9/14/94 11:50:29 */

#ifndef _H_SLEEP
#define _H_SLEEP

/*
 * COMPONENT_NAME: SYSPROC 
 *
 * ORIGINS: 3, 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>

#define	EVENT_NULL	(-1)		/* null list of subscribers */

#define EVENT_NDELAY	(0)		/* wait_mask value to not wait */
#define EVENT_SHARED    (0x00000004)    /* event bit for shared event */
#define EVENT_QUEUE     (0x00000008)    /* event bit for queued event */
#define EVENT_SYNC	(0x00000010)	/* event bit for synchronous event */
#define EVENT_CHILD	(0x00000020)	/* event bit for child death */
#define EVENT_KERNEL    (0x000000FF)    /* event bit for kernel event */

	/* e_sleep flag values: */
#define	EVENT_SHORT	(0)		/* short wait, inhibit signals */
#define	EVENT_SIGWAKE	(2)		/* wake on signal */
#define	EVENT_SIGRET	(4)		/* return on signal */

	/* return codes: */
#define	EVENT_SUCC	(1)		/* success, waitq */
#define	EVENT_SIG	(0)		/* process signalled, e_sleep & waitq */

	/* e_wakeupx option values */
#define E_WKX_NO_PREEMPT	(0)	/* do not preempt calling process */
#define E_WKX_PREEMPT		(1)	/* preempt calling process */

	/* flags for e_thread_sleep */
#define INTERRUPTIBLE		(1)	/* sleep is interruptible */
#define LOCK_HANDLER		(2)	/* lock used by interrupt handlers */
#define LOCK_READ		(4)     /* complex lock read mode specified */
#define LOCK_SIMPLE		(8)     /* simple lock specified */
#define LOCK_WRITE		(16)    /* complex lock write mode specified */

	/* flags for wakeup_lock */
#define WAKEUP_LOCK_SIMPLE	(1)	/* wakeup for simple locks */
#define WAKEUP_LOCK_WRITER	(2)	/* wakeup for complex locks */
#define WAKEUP_LOCK_ALL		(4)	/* wakeup all waiters */

	/* return values from e_thread_block */
#define THREAD_AWAKENED		(1)	/* normal wakeup value */
#define THREAD_TIMED_OUT	(2)	/* sleep timed out */
#define THREAD_INTERRUPTED	(4)	/* sleep interrupted by signal */
#define THREAD_OTHER		(4096)	/* Beginning of subsystem values */


#ifdef _KERNEL
#ifdef _AIXV3_POSTWAIT

/*
 * DO NOT USE THE FOLLOWING MACROS.  They are likely to disappear in
 * the near future.  They are included so that V4 kernel extensions
 * that have not upgraded their code correctly to support multiple
 * threads will continue to work for mono-threaded processes.  When 
 * the last of these drivers has been updated, these macros will 
 * disappear.  The use of these macros can result in hung processes 
 * or a hung system.
 */
#define e_wait(x,y,z)   et_wait((x),(y),(z))
#define e_post(x,y)     et_post((x),getptid(y))
#endif
#endif

/*
 * Entry points for process execution control
 */

#ifdef   _NO_PROTO

int sleepx();                           /* wait for events to occur */
void wakeup();                          /* wake processes waiting on chan */
unsigned long et_wait();                /* wait for events to occur */
int e_sleep();                          /* add process to event list */
int e_sleepl();                         /* add proc to eventlist & lock */
void et_post();                         /* notify thread of events */
void e_wakeup();			/* wakeup subscribers */ 
void e_wakeupx();			/* wakeup subscribers preserve runrun */
void e_wakeup_w_sig();			/* wakeup subscribers with signal */

int e_sleep_thread();			/* block the current thread with lock */
void e_assert_wait();			/* assert that thread is about to sleep */
void e_clear_wait();			/* clear wait condition */
int e_block_thread();			/* block the current thread */
void e_wakeup_one();			/* wakeup the highest priority sleeper */
void e_wakeup_w_result();		/* wakeup threads with result */

#else  /*_NO_PROTO */

int sleepx(                             /* wait for events to occur */
        int     chan,                   /* wait channel             */
        int     pri,                    /* priority                 */
        int     flags);                 /* signal control flags     */

void wakeup(                            /* wake processes waiting on chan */
        int     chan);                  /* channel whose processes will wakeup*/

ulong et_wait(                          /* wait for events to occur */
        ulong   wait_mask,              /* mask of events to await  */
        ulong   clear_mask,             /* mask of events to clear  */
        int     flags);                 /* wait option flags        */

int e_sleep(                            /* add process to event list */
        int     *event_list,            /* list of subscribers       */
        int     flags);                 /* wait option flags         */

int e_sleepl(                           /* add proc to eventlist & lock */
        int     *lock_word,             /* caller's lock word           */
        int     *event_list,            /* list of subscribers          */
        int     flags);                 /* wait option flags            */

void et_post(                           /* notify thread of events     */
        ulong   events,                 /* mask of events to be posted */
        tid_t   tid);                   /* thread to be posted         */

void e_wakeup(   			/* wakeup subscribers  */ 
  	int	*event_list);		/* list of subscribers */

void e_wakeupx(   			/* wakeup subscribers  */ 
  	int	*event_list,		/* list of subscribers */
	int	option);		/* option to control preemption */

void e_wakeup_w_sig(   			/* wakeup subscribers with signal */ 
  	int	*event_list,		/* list of subscribers */
	int	signo);		        /* signal to be posted */

int e_sleep_thread(			/* block the current thread with lock */
	int 	*event_list,		/* list of subscribers */
	void 	*lockp,			/* lock word */
	int	flags);			/* flags */

void e_assert_wait(			/* assert that thread is about to sleep */
	int 	*event_list,		/* list of subscribers */
	boolean_t interruptible);	/* sleep interruptible by signals */

void e_clear_wait(			/* clear wait condition */
	tid_t	tid,			/* the target thread */
	int 	result);		/* the wakeup result */

int e_block_thread();			/* block the current thread */

void e_wakeup_one(			/* wakeup the highest priority sleeper */
  	int	*event_list);		/* list of subscribers */

void e_wakeup_w_result(			/* wakeup threads with result */
	int 	*event_list,		/* list of subscribers */
	int 	result);		/* the wakeup result */

#endif /*_NO_PROTO */


/* Private kernel services */
#ifdef _KERNSYS
#ifdef   _NO_PROTO

void ep_post();
void init_sleep();
void setrq();
void remrq();
void kwakeup();
void remove_e_list();

#else /* _NO_PROTO */

void ep_post(                           /* notify process of events    */
        ulong   events,                 /* mask of events to be posted */
        pid_t   pid);                   /* process to be posted        */

void init_sleep(void);                  /* initialize hsque struct */

void setrq(                             /* add entry to run queue    */
        struct thread *t,		/* entry to add to run queue */
	int flags, int where);
void remrq(                             /* remove entry from run queue    */
        struct thread *t);                /* entry to remove from run queue */


void kwakeup(
	int *event_list, 		/* list of subscribers */
	int option, 			/* options for wakeup */
	int result);			/* reason for wakeup */ 

void remove_e_list(			/* remove from list */
	struct thread *t,		/* thread to be removed */
	int *event_list); 		/* list of subscribers */

void wait_on_lock(			/* wait on a simple lock */
	int *event_list, 		/* hashed value of lock */  
	boolean_t interruptible);	/* interruptible by signal */

boolean_t wakeup_lock(			/* wakeup sleepers on simple locks */
	int *event_list,		/* hashed value of lock */
	void *lockp, 			/* non-hashed address of lock */
	int flags);			/* mode */

#endif /*_NO_PROTO */

/*
 * This is an array of event list anchors used by sleep/wakeup.
 * Each element needs to be initialized to EVENT_NULL during
 * system initialization (see init_sleep).
 */
#define NHSQUE          127             /* should be "prime like" */
#define sqhash(X)       (&hsque[ ((uint)(X)&0x7fffffff) % NHSQUE ])
extern int hsque[NHSQUE];

#endif /* _KERNSYS */
#endif /* _H_SLEEP */
