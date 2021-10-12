static char sccsid[] = "@(#)07	1.1  src/bos/usr/ccs/lib/libpthreads/exc_handling.c, libpth, bos411, 9428A410j 10/15/93 07:29:31";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	sync_signal_handler
 *	setup_sync_signal_handlers
 *	destroy_exc
 *	init_once
 *	__exc_thread_init
 *	set_cur_exc
 *	__exc_raise
 *	exc_matches
 *	__exc_cleanup_handler
 *	__exc_setjmp_postlude
 *	exc_set_status
 *	exc_get_status
 *	exc_report
 *	
 * ORIGINS:  71  83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * Pthread based exception package support routines.
 *
 * These support routines help to define a TRY/CATCH exception mechanism
 * that runs 'on top of' P1003.4a/D4 pthreads.
 *
 * The following implementation models all exceptions by converting them
 * into pthread "cancel"'s. The package counts on the pthread cancel
 * mechanism to maintain the cancel cleanup handler chain; it piggyback
 * on the cancel cleanup handler list at the expense of a slightly more
 * expensive RAISE().
 *
 * The exception currently being processed is recorded in per thread
 * data which is set by the excpetion handler package.
 *
 * Exception handlers execute with general cancellability disabled.
 *
 * Arbitrary application pthread_cancel's that are not part of a TRY/CATCH
 * scoped macro will unwind to the mort recent TRY/CATCH exception handler.
 * That is, if an application nests a pthread_cleanup_push/pop() macro
 * set inside a TRY/CATCH macro set the pthread cleanup macros outside
 * the exception package scope are represented as a "cancel" exception.
 *
 * Note:
 *	Code in this module is intended to be independent of any
 *	particular pthreads implementation.
 */

#include "pthread.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>

typedef struct __exc_jmpbuf {
        unsigned int    __setintr;      /* state of general cancel delivery */
        unsigned int    __setintrtype;  /* state of async cancel delivery */
        jmp_buf         __jb;
} __exc_jmpbuf;

typedef enum __exc_kind_t {
        __exc_c_kind_address = 0x0123456,
        __exc_c_kind_status  = 0x0987654
} __exc_kind_t;
typedef struct __exc_exception {
        __exc_kind_t    __kind;
        union __match_value {
                struct __exc_exception  *__address;
                int                     __value;
        } __match;
} EXCEPTION;

EXCEPTION exc_insfmem_e;         /* Insufficient memory */

#define EXC_CREATE(exc)	EXCEPTION exc = {__exc_c_kind_address, &exc}

EXC_CREATE(exc_uninitexc_e);
EXC_CREATE(exc_exquota_e);
EXC_CREATE(exc_insfmem_e);
EXC_CREATE(exc_nopriv_e);
EXC_CREATE(exc_illaddr_e);
EXC_CREATE(exc_illinstr_e);
EXC_CREATE(exc_resaddr_e);
EXC_CREATE(exc_privinst_e);
EXC_CREATE(exc_resoper_e);
EXC_CREATE(exc_aritherr_e);
EXC_CREATE(exc_intovf_e);
EXC_CREATE(exc_intdiv_e);
EXC_CREATE(exc_fltovf_e);
EXC_CREATE(exc_fltdiv_e);
EXC_CREATE(exc_fltund_e);
EXC_CREATE(exc_decovf_e);
EXC_CREATE(exc_subrng_e);
EXC_CREATE(exc_excpu_e);
EXC_CREATE(exc_exfilsiz_e);
EXC_CREATE(exc_SIGTRAP_e);
EXC_CREATE(exc_SIGIOT_e);
EXC_CREATE(exc_SIGEMT_e);
EXC_CREATE(exc_SIGSYS_e);
EXC_CREATE(exc_SIGPIPE_e);
EXC_CREATE(exc_unksyncsig_e);

EXC_CREATE(pthread_cancel_e);

pthread_key_t	__exc_key;


/* -------------------------------------------------------------------------- */

/*
 * Function:
 *	sync_signal_handler
 *
 * Parameters:
 *	sig	- signal that caused handler to be called
 *
 * Description:
 *	This synchronous signal handler is already running on the correct
 *	thread stack. A RAISE is all that's required to initiate the unwind.
 *	
 *	Opon receipt of a synchronous signal, a pthread_cancel() is posted
 *	to the thread in which the synchronous signal occurred cancel delivery
 *	will be forced. The cancel initiates a cleanup handler in the context
 *	of the synchronous signal handler. The cleanup handler then does
 *	a _exc_longjmp_np from the context of the signal handler back to
 *	the most recent exception handler.
 *	
 *	NOTE:
 *
 *	It is assumed that it is okay to do a RAISE from a SYNCHRONOUS signal
 *	handler without any ill effects. While RAISE is doing a couple of
 *	pthread operations, the fact that these whould only performed in the
 *	thread that had the fault due to a synchronous fault in the thread
 *	(i.e. we were in user code, not the pthread library when the fault
 *	occurred) should mean that there are no pthread re-entrency problems.
 */
static void
sync_signal_handler(int sig)
{
	EXCEPTION	*exc;
	
	switch (sig) {
		case SIGILL:	exc = &exc_illinstr_e;		break;
		case SIGTRAP:	exc = &exc_SIGTRAP_e;		break;
		case SIGIOT:	exc = &exc_SIGIOT_e;		break;
		case SIGEMT:	exc = &exc_SIGEMT_e;		break;
		case SIGFPE:	exc = &exc_aritherr_e;		break;
		case SIGBUS:	exc = &exc_illaddr_e;		break;
		case SIGSEGV:	exc = &exc_illaddr_e;		break;
		case SIGSYS:	exc = &exc_SIGSYS_e;		break;
		case SIGPIPE:	exc = &exc_SIGPIPE_e;		break;
		default:	exc = &exc_unksyncsig_e;	break;
	}

	RAISE(*exc);
}


/*
 * Function:
 *	setup_sync_signal_handlers
 *
 * Description:
 *	Setup a signal handler to catch all synchronous signals and convert
 *	them to exceptions. The occurance of a synchronous signal results
 *	in an immediate exception unwind on the stack of the thrad that caused
 *	the signal.
 */
static void
setup_sync_signal_handlers(void)
{
	/* Setup synchronous handlers. Note that we get the current state of
	 * each signal and then just change the handler field. Reputed to
	 * be better for some implementations.
	 */

#define SIGACTION(sig) \
	{ \
		struct sigaction	action; \
		\
		(void)sigaction((sig), (struct sigaction *) 0, &action); \
		action.sa_handler = sync_signal_handler; \
		(void)sigaction((sig), &action, (struct sigaction *) 0); \
	}

	SIGACTION(SIGILL);
	SIGACTION(SIGTRAP);
	SIGACTION(SIGIOT);
	SIGACTION(SIGEMT);
	SIGACTION(SIGFPE);
	SIGACTION(SIGBUS);
	SIGACTION(SIGSEGV);
	SIGACTION(SIGSYS);
	SIGACTION(SIGPIPE);

#undef SIGACTION
}


/*
 * Function:
 *	destroy_exc
 *
 * Parameters:
 *	exc_cur	- malloc'd exception state
 *
 * Description:
 *	Called at thread exit to destroy the thread specific exception state
 *	storage.
 */
static void
destroy_exc(void *exc_cur)
{
	free(exc_cur);
}


/*
 * Function:
 *	init_once
 *
 * Description:
 *	Initialize the exception package.
 *	This function is run through pthread_once().
 *	Create the key for the thread specific exception state.
 */
static void
init_once(void)
{
	__key_create_internal(&__exc_key, destroy_exc);
}


/*
 * Function:
 *	__exc_thread_init
 *
 * Description:
 *	Initialize the exception package.
 *	Setup signal handlers for per-thread synchronous signals.
 */
void
__exc_thread_init(void)
{
	EXCEPTION	*cur_exc;
	static pthread_once_t	init_once_block = PTHREAD_ONCE_INIT;

	/* One time initialization for all threads.
	 * Initialise the current exception to "cancel" so that the thread sees
	 * a cancel exception when its cancelled. (Non-cancel
	 * exceptions--i.e., those raised via RAISE--will set the value to
	 * something else. ENDTRY resets it back to cancel.)
	 */
	pthread_once(&init_once_block, init_once);

	/* If we already have the thread-specific storage to hold this thread's
	 * "current exception", we're done.
	 */
	cur_exc = pthread_getspecific(__exc_key);
	if (cur_exc != 0)
		return;

	/* Allocate the storage to hold this thread's "current exception".
	 */
	if (!(cur_exc = (EXCEPTION *)malloc(sizeof(*cur_exc)))) {
		fprintf(stderr, "Cannot initialise exceptions\n");
		exit(1);
	}
	pthread_setspecific(__exc_key, (void *)cur_exc);
	*cur_exc = pthread_cancel_e;

	/* Set up signal handlers for this thread.
	 */
	setup_sync_signal_handlers();
}


/*
 * Function:
 *	set_cur_exc
 *
 * Parameters:
 *	exc	- new current exception
 *
 * Description:
 *	Set the thread's current exception to the specified exception.
 */
static void
set_cur_exc(EXCEPTION *exc)
{
	EXCEPTION	*cur_exc;

	__exc_thread_init();
	cur_exc = pthread_getspecific(__exc_key);
	*cur_exc = *exc;
}


/*
 * Function:
 *	__exc_raise
 *
 * Parameters:
 *	exc	- exception to raise
 *
 * Return value:
 *	(does not return)
 *
 * Description:
 *	RAISE operation. All exceptions are mapped into a pthread_cancel()
 *	to start the cancel cleanup handler popping. Before starting the unwind,
 *	setup the thread's current exception.
 * 
 *	THIS IS CALLED FROM A SYNCHRONOUS SIGNAL HANDLER (see above).
 */
void
__exc_raise(EXCEPTION *exc)
{
int oldtype;
#ifdef	UNHANDLED_EXC_EXIT

	/* Detect the case where there are no more cleanup routines on
	 * our stack and if so, cause the process to exit. The reason
	 * we do this is because
	 * (a) it seems like not such a bad idea
	 * (b) if an unhandled exception causes a master thread to
	 *	terminate the slaves may get left in limbo
	 *
	 * Unfortunately, there's not a portable way to do this. For OSF/1
	 * we use knowledge about how "pthread_cleanup_{push,pop}" are 
	 * implemented.
	 */

	/* BEGIN-ALERT NON-PORTABLE CODE */
	__pthread_cleanup_handler_t **handler_queue;

	handler_queue = pthread_getspecific(__pthread_cleanup_handlerqueue);
	if (handler_queue != 0 && *handler_queue == 0) {
		fprintf(stderr, "Unhandled exception; exiting!\n");
		exit(1);
	}
	/* END-ALERT NON-PORTABLE CODE */

#endif	/* UNHANDLED_EXC_EXIT */

	/* Cancel ourselves to invoke the cancel "handler".
	 * If there is no handler the thread will exit
	 */
	set_cur_exc(exc);
	pthread_cancel(pthread_self());
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldtype);
	pthread_testcancel();
}


/*
 * Function:
 *	exc_matches
 *
 * Parameters:
 *	cur_exc	- one exception
 *	exc	- the other exception
 *
 * Return value:
 *	0	the exceptions do not match
 *	1	the exceptions match
 *
 * Description:
 *	Compare the kind of exception and then by value or address.
 */
int
exc_matches(EXCEPTION *cur_exc, EXCEPTION *exc)
{
	if (cur_exc->__kind == exc->__kind &&
	    (cur_exc->__kind == __exc_c_kind_status ?
			cur_exc->__match.__value == exc->__match.__value : 
			cur_exc->__match.__address == exc->__match.__address))
		return (1);

	return (0);
}


/*
 * Function:
 *	__exc_cleanup_handler
 *
 * Parameters:
 *	exc_buf	- the exception frame to return to (most local TRY)
 *
 * Return value:
 *	(does not return)
 *
 * Description:
 *	The exception package's standard cancel cleanup handler.
 *	Do a non-local goto using this exeception handler's
 *	exception state block to get back to the exception handler.
 *
 *	ASSUMPTION
 * 
 *	This cleanup handler is invoked within the context of a signal handler.
 *	It assumes that doing this is valid. 
 */
void
__exc_cleanup_handler(__exc_jmpbuf *exc_buf)
{
	__exc_longjmp(exc_buf, 1);
}


/*
 * Function:
 *	__exc_setjmp_postlude
 *
 * Parameters:
 *	exc_buf	- state when exception frame was created
 *
 * Description:
 *	Restore cancellation state after setjmp is complete.
 */
void
__exc_setjmp_postlude(__exc_jmpbuf *exc_buf)
{
int oldtype;
int oldstate;
	pthread_setcanceltype(exc_buf->__setintrtype, &oldtype); 
	pthread_setcancelstate(exc_buf->__setintr, &oldstate); 
}


/*
 * Function:
 *	exc_set_status
 *
 * Parameters:
 *	exc	- exception
 *	value	- status value to assign
 *
 * Description:
 *	Set exception kind to _exc_c_kind_status and assign it the value.
 */
void
exc_set_status(EXCEPTION *exc, unsigned int value)
{
	exc->__kind = __exc_c_kind_status;
	exc->__match.__value = value;
}


/*
 * Function:
 *	exc_get_status
 *
 * Parameters:
 *	exc	- exception
 *	value	- out param for status value
 *
 * Return value:
 *	0	- Success
 *	-1	- Exception was not of type _exc_c_kind_status.
 *
 * Description:
 *	Retrieve status value from exception.
 */
int
exc_get_status(EXCEPTION *exc, unsigned int *value)
{
	if (exc->__kind != __exc_c_kind_status)
		return (-1);
	*value = exc->__match.__value;
	return (0);
}


/*
 * Function:
 *	exc_report
 *
 * Parameters:
 *	exc	- exception
 *
 * Description:
 *	Print a message to stderr describing the exception.
 */
void
exc_report(EXCEPTION *exc)
{
	if (exc->__kind == __exc_c_kind_status)
		fprintf(stderr, "Exception 0x%lx status 0x%lx\n",
			exc, exc->__match.__value);
	else
		fprintf(stderr, "Exception 0x%lx address 0x%lx\n",
			exc, exc->__match.__address);
}

