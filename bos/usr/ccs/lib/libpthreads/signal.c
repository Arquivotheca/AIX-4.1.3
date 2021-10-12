static char sccsid[] = "@(#)18	1.29  src/bos/usr/ccs/lib/libpthreads/signal.c, libpth, bos41J, 9521B_all 5/29/95 07:27:47";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	sigwait_fork_prepare
 *	sigwait_fork_parent
 *	sigwait_fork_child
 *	sigwait_handler
 *	_pthread_sigwait_startup
 *	sigwait
 *	raise
 *	sigaction
 *	pthread_kill
 *	sigthreadmask
 *	sigpending
 *	nsleep
 *	pause
 *	sigsuspend
 * 
 * ORIGINS:  71, 83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: signal.c
 *
 *
 *
*/


#include "internal.h"
#include <stdio.h>

#define	_SIGSET_OPS_MACROS 1	/* drag in the kernel's macros	*/
				/* and  use them in-line here	*/
#include <signal.h>

extern pthread_queue   __dbx_known_pthreads;

struct sigwait_cancel_info {
	int	    	signals_in_set;	/* Count of sigwaited signals */
	unsigned char	*signals_set;
	pthread_d	self;
	sigset_t	user_blockmask;
	int		queue_removed;
	
};
typedef struct sigwait_cancel_info sigwait_cancel_info_t;
typedef (*signal_handler_t)(int, int, struct sigcontext *);

pthread_queue sigwaiter_threads;

/*
 * Local Variables
 */
static sigset_t locksigmask;		/* mask while lock is held */
static spinlock_t signal_lock;		/* protect access to the thread mask */
static sigset_t	forksigmask;		/* sigset across fork */
static sigset_t	inherit_forksigmask;	/* sigset at fork */
static sigset_t sigwaitmask;       	/* Can't wait for these signals */
static sigset_t ignoresigmask;   	/* by default these are ignored */
static struct sigaction sa_sigwait;	/* sigwait_handler */
static struct sigaction	old_handlers[NSIG];	/* previous handlers */
static int siginterest[NSIG];			/* wait-for counts */

static void delivery_wrapper();		/* forward reference */
static void sigwait_handler();		/* forward reference */

/*
** Lock and unlock Macros
*/
#define LOCK   _spin_lock((atomic_p)&signal_lock)
#define UNLOCK _spin_unlock((atomic_p)&signal_lock)

#define BLOCK_LOCK(newset, oldset) 				\
{								\
	sigprocmask(SIG_SETMASK, newset, oldset);		\
	LOCK;							\
}

#define UNLOCK_UNBLOCK(saved_set)				\
{								\
	UNLOCK;							\
	sigprocmask(SIG_SETMASK, saved_set, NULL);		\
}

#define THREAD_SLEEP(mask)					\
{								\
	if (thread_tsleep(0, (atomic_p)&signal_lock, mask) == -1) { 	\
		LOCK;						\
	} else {						\
		BLOCK_LOCK(&locksigmask, NULL);			\
	}							\
}

/*
 * Function:
 *	sigwait_fork_prepare
 *
 * Description:
 *	Quiesce the sigwait() package prior to a fork.
 */
private void
sigwait_fork_prepare(void)
{
	_spin_lock(&signal_lock);
	sigprocmask(SIG_BLOCK, &forksigmask, &inherit_forksigmask);
}


/*
 * Function:
 *	sigwait_fork_parent
 *
 * Description:
 *	This is called in the parent process after a fork.
 */
private void
sigwait_fork_parent(void)
{
	sigprocmask(SIG_SETMASK, &inherit_forksigmask, NULL);
	_spin_unlock(&signal_lock);
}


/*
 * Function:
 *	sigwait_fork_child
 *
 * Description:
 *	This is called in the child process after a fork.
 *	At this point we are single threaded with most signals blocked.
 *	Synchronous signals are not blocked.
 *	To return to normality we need to reset sigwait data and signal
 *	disposition changed by the registered sigwaiters by
 *	restoring old handlers and restoring signal mask to the one
 *	prior to fork().
 */
private void
sigwait_fork_child(void)
{
	register int	signo;

	/* If the parent had interest in a signal we need to reset it
	 * since the sigwaiter no longer exists.
	 */
	for (signo = 1; signo < NSIG; signo++){
		_sigaction(signo, &old_handlers[signo], NULL);
		siginterest[signo] = 0;
	}
	sigprocmask(SIG_SETMASK, &inherit_forksigmask, NULL);

	_spin_unlock(&signal_lock);
}


/*
 * Function:
 *	_pthread_sigwait_startup
 *
 * Description:
 *	Called by pthread_init, this function initializes all the global
 *	data and state needed by the sigwait call.
 */
void
_pthread_sigwait_startup()
{
	register int	signo;
	struct sigaction act, oact, nact;
	sigset_t 	coresigmask;
	int		i;

	/* Synchronous signals - can't sigwait these signals */
	sigemptyset(&sigwaitmask);
	SIGADDSET(sigwaitmask, SIGILL);
	SIGADDSET(sigwaitmask, SIGTRAP);
	SIGADDSET(sigwaitmask, SIGABRT);
	SIGADDSET(sigwaitmask, SIGEMT);
	SIGADDSET(sigwaitmask, SIGFPE);
	SIGADDSET(sigwaitmask, SIGBUS);
	SIGADDSET(sigwaitmask, SIGSYS);
	SIGADDSET(sigwaitmask, SIGSEGV);
	SIGADDSET(sigwaitmask, SIGPIPE);
	SIGADDSET(sigwaitmask, SIGSTOP);
	SIGADDSET(sigwaitmask, SIGKILL);

	/* Mask containing signals for which the default action is to ignore. */
	SIGINITSET(ignoresigmask);
	SIGADDSET(ignoresigmask, SIGURG);
	SIGADDSET(ignoresigmask, SIGIO);
	SIGADDSET(ignoresigmask, SIGWINCH);
	SIGADDSET(ignoresigmask, SIGPWR);
	SIGADDSET(ignoresigmask, SIGCHLD);
	SIGADDSET(ignoresigmask, SIGCONT);

	/* Mask that is active while we hold the signal_lock */
	SIGFILLSET(locksigmask);
	SIGDELSET(locksigmask, SIGTRAP); /* for debbuger (trace,breakpoint) */
	forksigmask = locksigmask;
	SIGMASKSET(forksigmask, sigwaitmask);

	/* 
	 * Create core mask from the above by subtracting signals
	 * that must not dump core.  Add SIQUIT as special case.
	 */
	coresigmask = sigwaitmask;
	SIGADDSET(coresigmask, SIGQUIT);
	SIGDELSET(coresigmask, SIGPIPE);
	SIGDELSET(coresigmask, SIGSTOP);

	act.sa_handler = SIG_DFL;
	act.sa_flags = SA_RESTART | SA_FULLDUMP;
	SIGINITSET(act.sa_mask);

	nact.sa_handler = SIG_IGN;
	nact.sa_flags = SA_RESTART | SA_FULLDUMP;
	SIGINITSET(nact.sa_mask);
	
	for(i=1; i<NSIG; i++){
		if (SIGISMEMBER(coresigmask, i)) {
			(void) _sigaction(i, &act, &oact);
			if (oact.sa_handler == SIG_IGN) {
				(void) _sigaction(i, &nact, NULL);
			}
		}
	}

	/* 
	 * Set up the sigaction structure for future calls.
	 * Block all signals during handler execution and restart
	 * any system call it interrupted.
	 */
	sa_sigwait.sa_handler = sigwait_handler;
	sa_sigwait.sa_flags = 0;
	sa_sigwait.sa_mask = locksigmask;

	/* 
	 * There is no interest in any signals.
	 * Empty the signal handlers 
	 */
	for (signo = 1; signo < NSIG; signo++) {
		siginterest[signo] = 0;
		old_handlers[signo].sa_handler = SIG_DFL;
		old_handlers[signo].sa_flags = 0;
		SIGINITSET(old_handlers[signo].sa_mask);
	}

	/* 
	 * Initialize locks and state. 
	 */
	_spinlock_create(&signal_lock);

	queue_init(&sigwaiter_threads);
	
	if (pthread_atfork(sigwait_fork_prepare,
			      sigwait_fork_parent,
			      sigwait_fork_child))
		INTERNAL_ERROR("_pthread_sigwait_startup");
}


/*
 * Function:
 *	sigwait_handler
 *
 * Parameters:
 *	signo - The signal number that just arrived
 *
 * Description:
 *	This is the signal handler set up by a sigwaiter for all signals
 *	that the thread is interested in.  The handler remains installed 
 * 	until there are no more threads interested in that signal. 
 *	The thread_tsleep() call returns when a signal is delivered.
 */
private void
sigwait_handler(int signo, int siginfo, struct sigcontext *scp)
{
	pthread_d 	self;	 	/* pthead_d of current thread */
	pthread_d 	wait_pthdid;	/* pthread_d of waiting thread */
	pthread_d	thd;
       	int            *ptelem;
        int             adjust;
        int             found = FALSE;
        int 	off = ((int)&(((pthread_d )0)->sigwaiter_link))/sizeof(int);

	self = pthread_id_lookup(pthread_self());

	/* Wait for spinlock - signals should be blocked via sigaction */
	LOCK;

	/* Find a sigwaiter thread - favor yourself */ 
	if (SIGISMEMBER(self->sig_data.sigwait_set, signo)) {
		SIGDELSET(self->sig_data.pkill_set, signo);
		wait_pthdid = self;
		found = TRUE;
	}
	else {
		/* Get a thread that is waiting */
        	for (
                	ptelem = (int *)queue_next(&sigwaiter_threads),
       	 		adjust = (int)(ptelem - off),
                	thd = (pthread_d)adjust;

                	ptelem != (int *)&sigwaiter_threads;

                	ptelem = (int *)queue_next(&thd->sigwaiter_link),
                	adjust = (int)(ptelem - off),
                	thd = (pthread_d)adjust 
                     ) 
		{
			if (SIGISMEMBER(thd->sig_data.sigwait_set, signo)) {
				wait_pthdid = thd;
				found = TRUE;
				break;
			}
        	}
	}

	/*
  	 * Preprocess the sigwaiter wakeup operation to minimize the
	 * time spent holding the lock.  The call to thread_twakeup 
	 * below will probably cause a context switch, however, it
	 * will immediately block on the lock again.
	 */
	if (found == TRUE) {
		/*
		 * Record the signal in the pthread structure because
		 * there is no thread_twakeup is not always called.  A
		 * thread_twakeup is a no-op for the current thread.
		 */
		SIGADDSET(wait_pthdid->sig_data.siggot, signo);

		/* Remove the woken up thread from waiter list */
		queue_remove(&(wait_pthdid->sigwaiter_link));
	}

	UNLOCK;

	if (found == TRUE) {
		/*
		 * The thread when awakened has all signals blocked.  In the 
		 * twakeup case the mask is reset in the kernel.  Otherwise
		 * the mask is controlled through the signal delivery 
		 * mechanism.  
		 */
		if (wait_pthdid != self)
			thread_twakeup((int)wait_pthdid->vp->id, EVT_SIGPOST);
		else 
			scp->sc_mask = locksigmask;

	} else {
		/*
		 * take the old action.  
		 */
		delivery_wrapper(signo, siginfo, scp);
	}

}
	
static
void or_perthread_pending( sigset_t *pending)
{
       	int            *ptelem;
        int             adjust;
	pthread_d	thd;
        int 		off = ((int)&(((pthread_d )0)->DBXlink))/sizeof(int);

	/* Get the thread id that is waiting */
        for (
               	ptelem = (int *)queue_next(&__dbx_known_pthreads),
        	adjust = (int)(ptelem - off),
               	thd = (pthread_d)adjust;

               	ptelem != (int *)&__dbx_known_pthreads;

               	ptelem = (int *)queue_next(&thd->DBXlink),
               	adjust = (int)(ptelem - off),
               	thd = (pthread_d)adjust 
	)
	{
		SIGORSET(*pending, thd->sig_data.pkill_set);
	}

}

/*
 * Function:
 *    	sigwait_cleanup_handler(sigwait_cancel_info_t *caninfo )
 *
 * Parameters:
 *
 *
 * Description:
 *	This routine acts as a cleanup handler after the sigwaiter has
 *	woken up.
 *	The thread is no longer waiting; it has received a signal or cancel.
 *	Remove all the signals from the globals set checking for signals
 *	that no-one is interested in. These signals have their handlers
 *	removed and are reblocked.
 *
 * Locks:
 *	The sigwait_lock must be held by the caller of this function.
 *
 */

private void
sigwait_cleanup_handler( struct sigwait_cancel_info *info )
{
	unsigned char 	*signals_set;
   	int 		signals_in_set; 
	int 		j;
	int 		signo;
	pthread_d 	self;
	sigset_t 	thread_pending, process_pending;
       	int            *ptelem;
        int             adjust;
	pthread_d	thd;
        int 		off = ((int)&(((pthread_d)0)->DBXlink))/sizeof(int);
	int 		checkthreads;
	
	signals_in_set = info->signals_in_set;
	signals_set = info->signals_set;
	self = info->self;

	BLOCK_LOCK(&locksigmask, NULL);

	/* Get process and current thread pending bits */
	_sigpending(&process_pending);

	/* Add in pthread_kill pending bits */
	or_perthread_pending(&thread_pending);	
	if (thread_pending.losigs || thread_pending.losigs)
		checkthreads = 1;
	else
		checkthreads = 0;
	
	/* 
	 * Cannot distinguish process from thread bits.  We may lose
	 * some signals pending at the process level.
	 */
	SIGMASKSET(process_pending, self->sig_data.pkill_set);

	/* 
	 * Remove the sigaction handlers installed on behalf of the
	 * current sigwaiter if no other sigwaiter is interested.
	 */
	for (j=0; j<signals_in_set; j++) {

		signo = signals_set[j];
		if (--siginterest[signo] == 0) {

		    /*
		     * Restore old action.
		     */
		    _sigaction (signo, &old_handlers[signo], NULL);

		    if (checkthreads) {

                        for (ptelem=(int *)queue_next(&__dbx_known_pthreads),
                             adjust = (int)(ptelem - off),
                             thd = (pthread_d)adjust;

                             ptelem != (int *)&__dbx_known_pthreads;

                             ptelem = (int *)queue_next(&thd->DBXlink),
                             adjust = (int)(ptelem - off),
                             thd = (pthread_d)adjust )
                        {
                       	     switch ((int)old_handlers[signo].sa_handler) {
                             case (int)SIG_IGN :
                                 SIGDELSET(thd->sig_data.pkill_set, signo);
				 break;
			     case (int)SIG_DFL :
				 /*
				  * Regenerate signals because sigaction
				  * removes these signals from the pending
				  * bits.  They should be blocked and left
				  * pending on the specific thread. 
				  */
                                 if (SIGISMEMBER(ignoresigmask, signo) &&
                               	     SIGISMEMBER(thd->sig_data.pkill_set, signo))
                                        (void)thread_kill(thd->vp->id, signo);
			     }
			}
		    }
		    if (SIGISMEMBER(process_pending, signo))
			 (void)kill(getpid(), signo);
     	       }
	}

	/* Remove from the linked list of sigwaiters */
	if (!info->queue_removed)
		queue_remove(&(self)->sigwaiter_link);

	/* Restore the user Mask */
	UNLOCK_UNBLOCK(&info->user_blockmask);

	return;
}

/*
 * Function:
 *	sigwait
 *
 * Parameters:
 *	set - the set of signals the calling thread wishes to wait for.
 *
 * Return value:
 *	0	Success, stores the signal number of the received signal at 
 *		location referenced by sig.
 *	EINVAL	The set argument contains an invalid or unsupported signal 
 * 		number.
 *
 * Description:
 *	The calling thread registers the signals that it is interested in.
 *	It then waits on a condition until a separate thread wakes it up
 *	when a signal that is waited has arrived. The actual signal is 
 *	placed in thread's control block(pthread_d).
 */
int
sigwait(const sigset_t *set, int *sig)
{
	struct sigwait_cancel_info  	cleanup_info;
	unsigned char 			signals_set[NSIG]; 
   	int 				signals_in_set; 
	pthread_d			self;
	int 				signo;
	int 				i;
	sigset_t 			user_blockmask, sleep_mask;

	/* Have to wait on at least one signal. */
	if (SIGSETEMPTY(*set))
		return (EINVAL);

	/* Some signals you can't wait on. */
	if ((set->losigs & sigwaitmask.losigs) ||
	    (set->hisigs & sigwaitmask.hisigs))
                return (EINVAL);
	
	self = pthread_id_lookup(pthread_self());

	BLOCK_LOCK(&locksigmask, &user_blockmask);

	/* 
	 * Calculate the signal mask to give to THREAD_SLEEP().
	 *  sleep mask = user block maks - the sigwaiter set 
	 */
	sleep_mask = user_blockmask;
	SIGMASKSET(sleep_mask, *set);
	
	/* Save the waiter set in pthread_d structure for the calling thread */
	self->sig_data.sigwait_set = *set;

	/* Fill in the signals_set[] array from the set specified */
    	signals_in_set = 0;
    	for (i=1; i < NSIG; i++) {
        	if (SIGISMEMBER(*set, i)){
            		signals_set[signals_in_set++] = i;
			if (siginterest[i]++ == 0)
				_sigaction(i, &sa_sigwait, &old_handlers[i]);
        	}
	}

	/* Add to the sigwaiter_threads list */
	queue_append(&sigwaiter_threads, &(self)->sigwaiter_link);

	/* Set up cancel cleanup */
	cleanup_info.signals_in_set = signals_in_set;
	cleanup_info.signals_set = signals_set;
	cleanup_info.self = self;
	cleanup_info.queue_removed = 0;
	cleanup_info.user_blockmask = user_blockmask;
	_pthread_cleanup_push(sigwait_cleanup_handler, &cleanup_info, self);

	/*
	 * Wait for the appropriate signal to be delivered.  The kernel
	 * favors sigwaiters but sigwaiters can't block some signals
	 * so they need to recheck state and possible go back to sleep.  
	 * There is also no quarantee that the user will block all signals 
	 * except the ones that he is sigwaiting for.
	 */ 
	while (1) {

		THREAD_SLEEP(&sleep_mask);
			
  		for (i=0; i<signals_in_set; i++) {
                	signo = signals_set[i];
        		if (SIGISMEMBER(self->sig_data.siggot, signo)){
        			SIGDELSET( self->sig_data.siggot, signo);
				break;
        		}
		}

		/* Is this a spurious wakeup ? */
		if(i != signals_in_set) break;
	}
	cleanup_info.queue_removed = 1;

	/* 
	 * Should be UNLOCK_UNBLOCK but we really only need to
	 * unlock because the cleanup handler is called without
	 * the lock in the cancelation case.
	 */
	UNLOCK;

	/* 
	 * At this point, the signals in the set are blocked and the lock
	 * is locked.  This will cleanup and return.
	 */
	pthread_cleanup_pop(1);

	*sig = signo;

	return(0);
}

/*
 * Function:
 *      raise
 *
 * Description:
 *      redefinition POSIX.4a of raise
*/
int
raise(int sig)
{
        return(pthread_kill(pthread_self(), sig));
}

static void
delivery_wrapper(int sig, int siginfo, struct sigcontext *scp)
{
        pthread_d self;
        signal_handler_t sa_handler;

        self = pthread_id_lookup(pthread_self());

        /* Reset the per-thread pthread_kill bit for that signal */
        sa_handler = (signal_handler_t)old_handlers[sig].sa_handler;

        /*
         * If default action, then regenerate signal. Only the kernel
         * can perform the default action.
         */
	switch ((int)sa_handler) {
        case (int)SIG_DFL : 
                _sigaction(sig, &old_handlers[sig], NULL);
                if (SIGISMEMBER(self->sig_data.pkill_set, sig))
                        (void) thread_kill(self->vp->id, sig);
                else
                        (void) kill(getpid(), sig);
		break;
        case (int)SIG_IGN :
		/*
		 * It is important that the signal_lock not be held 
		 * across this call, because the normal handler may call 
		 * signal routines that can deadlock.  They could also 
		 * alter the signal mask.  
		 */
                (*sa_handler)(sig, siginfo, scp);
		break;
        }

	LOCK;
        SIGDELSET(self->sig_data.pkill_set, sig);
	UNLOCK;
}

/*
 * Function:
 *      sigaction
 *
 * Description:
 *     	sigaction is redefined to protect sigwait_handler when installed 
 *	__sigaction is a syscall (the same as sigaction to avoid the loop
 *	created by the wrapper)
*/
int
sigaction(int sig, struct sigaction *act, struct sigaction *oact)
{
	struct sigaction wrapper_action;
	sigset_t maskset, oldset;
	int ret = 0;

	BLOCK_LOCK(&locksigmask, &oldset);

	if (oact){
		oact->sa_handler = old_handlers[sig].sa_handler;	
		oact->sa_flags = old_handlers[sig].sa_flags;	
		oact->sa_mask = old_handlers[sig].sa_mask;	
	}

	if (act) {

		/* Record the handler in library */
		old_handlers[sig].sa_handler = act->sa_handler;
		old_handlers[sig].sa_flags = act->sa_flags;
		old_handlers[sig].sa_mask = act->sa_mask;

		/* 
		 * Is there a sigwait() thread for this signal ? 
		 *   - If no interest, install user handler.
		 */
		if (siginterest[sig] == 0){
			if (act->sa_handler == SIG_IGN)
				reset_pthread_kill(sig);
			ret = _sigaction(sig, act, NULL);
		}

	} else {
		errno = EINVAL;
		ret = -1;
	}

	UNLOCK_UNBLOCK(&oldset);

	return( ret );
}

static int
reset_pthread_kill(int sig)
{
	int ret=0;
   	pthread_d       thread;
        pthread_d       th;
        int             *ptelem;
        int             adjust;
        int 		off = ((int)&(((pthread_d)0)->DBXlink))/sizeof(int);

	/* 
	 * If going to ignore, reset per-thread pthread_kill bit  
	 * for that signal for every pthread.
	 */
	for (
        	ptelem = (int *)queue_next(&__dbx_known_pthreads),
                adjust = (int)(ptelem - off),
                th = (pthread_d)adjust;
	
                ptelem != (int *)&__dbx_known_pthreads;

       	        ptelem = (int *)queue_next(&th->DBXlink),
                adjust = (int)(ptelem - off),
                th = (pthread_d)adjust ) 
	{
		SIGDELSET(th->sig_data.pkill_set, sig);
        }

}
				

/*
 * Function:
 *	sigthreadmask
 *
 * Parameters:
 *	how  -  indicates the manner in which the set is changed:
 *		SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK.
 *	new  -	points to a set of signals to be used to change the currently
 *		blocked new.
 *	old -	if the argument is not NULL, the previous mask is stored in the
 *	     	space pointed to by old.
 *
 * Return value:
 *	0	Success
 *	EINVAL	The value of the how argument is not equal to one of the 
 *		defined value
 *
 * Description:
 * 	This function is used to examine or change (or both) the calling
 *	thread's mask.
 *	The effect shall be the same as described for sigprocmask in 1:1 model.
 */
int sigthreadmask(int how, const sigset_t *new, sigset_t *old)
{
	if  (sigprocmask(how, new, old) == -1)
             return (EINVAL);
		
	else
	     return(0);
}


/*
 * Function:
 *      pthread_kill
 *
 * Parameters:
 *      pthread_t thread_id
 *      int       signal
 *
 * Return value:
 *      0       success
 *      ESRCH   No thread could be found corresponding to that specified
 *              by th given thread ID.
 *      EINVAL  The value of the sig argument is an invalid or
 *              unsupported signal number.
 *              The thread ID specified by ththread_id is invalid.
 *
 * Description:
 *              The pthread_kill() is used to direct a signal to be
 *              asynchronously delivred to the specified thread.
 */
int
pthread_kill(pthread_t thread_id, int sig)
{
	pthread_d       thread;
	int             status;
	sigset_t 	oldset;

    	/* Check for valid signal */
   	if (sig < 0 || sig > SIGMAX)
      		return (EINVAL);

   	thread = pthread_id_lookup(thread_id);
   	if (thread == NO_PTHREAD)
      		return (EINVAL);

   	if (thread->vp == NULL)
       		return (ESRCH);

   	BLOCK_LOCK(&locksigmask, &oldset);
   	SIGADDSET(thread->sig_data.pkill_set, sig)
   	UNLOCK_UNBLOCK(&oldset);
	
   	status = thread_kill(thread->vp->id, sig);
   	if (status == -1)
       		if (errno == EPERM)
           		return(EINVAL);
        	else
           		return(errno); 			/* ESRCH */

   	return (0);
}


int 
sigpending(sigset_t *set)
{
	int i;
	pthread_d self;
	sigset_t oldset;

	/* get kernel pending first */
	_sigpending(set);

	self = pthread_id_lookup(pthread_self());

	BLOCK_LOCK(&locksigmask, &oldset);
    	for(i=1; i < NSIG; i++)
        	if (SIGISMEMBER(self->sig_data.siggot, i)){
			SIGADDSET(*set, i);
        	}
	UNLOCK_UNBLOCK(&oldset);
}


int
nsleep(register struct timestruc_t *rqtp, /*Time interval to suspend exec*/
       register struct timestruc_t *rmtp) /*Time remaining on interval or 0*/
{
	_nsleep(rqtp, rmtp);
}

int
pause(void)
{
	_pause();
}


int
sigsuspend(sigset_t *set)
{
	_sigsuspend(set);
}
