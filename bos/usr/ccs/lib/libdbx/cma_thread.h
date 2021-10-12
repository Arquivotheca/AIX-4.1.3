/* @(#)13	1.4  src/bos/usr/ccs/lib/libdbx/cma_thread.h, libdbx, bos411, 9428A410j 2/25/94 15:12:59 */
#ifndef _h_cma_thread
#define _h_cma_thread
#if defined (CMA_THREAD) ||  defined (K_THREADS)
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/* defines */
#define OK_TO_SWITCH	(!isfinished(process) && running_thread && \
(running_thread != current_thread))

/* Options allowed in the "thread" command */
typedef enum { th_get_info = -3,		/* negative options used */
	       th_unhold_other = -2, 		/* by dbx internal only  */
	       th_hold_other = -1,
	       th_default = 0, 
	       th_hold, th_unhold, th_info, 
	       th_run, th_ready, th_susp, th_term,
#ifdef K_THREADS
               th_wait,
#endif /* K_THREADS */
	       th_current, th_run_next } thread_op;

/* Options allowed in the "mutex" command */
typedef enum { mu_default, mu_wait, mu_nowait, mu_lock, mu_unlock } mutex_op;

/* Options allowed in the "condition" command */
typedef enum { cv_default, cv_wait, cv_nowait } condition_op;

/* Options allowed in the "attribute" command */
typedef enum { attr_default } attribute_op;

extern void threads();                        /* list CMA thread objects */
extern void attribute();                     /* list CMA attribute objects */
extern void condition();                     /* list CMA condition variables */
extern void mutex();                         /* list CMA mutexes */

extern void thread_run_next();               /* set the next thread to run */
extern void switchThread();                  /* switch dbx thread context */
extern void setInconsistency();		     /* set inconsistency bit */

extern Symbol running_thread, current_thread;
#endif /* CMA_THREAD || K_THREADS */
extern boolean isThreadObjectSymbol_cma(Symbol); /* checks if symbol   */
                                                 /* is thread, mutex,  */
                                                 /* attribute or       */
                                                 /* condition variable */ 
                                                 /* object             */
    
#endif /* _h_cma_thread */
