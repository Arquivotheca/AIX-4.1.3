/* @(#)38       1.9  src/bos/kernel/proc/sig.h, sysproc, bos411, 9428A410j 7/16/94 15:10:07 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: SIG_MAYBE
 *		SIG_AVAILABLE
 *		ADD_PENDING_SIG
 *		DEL_PENDING_SIG
 *		IS_PENDING_SIG
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#define SIG_MAYBE(__t,__p)						\
	(								\
		(__p)->p_int & (SSUSP|SSUSPUM|STERM) || 		\
		(__t)->t_flags & (TSUSP|TTERM|TINTR) ||			\
		(__p)->p_sig.losigs || (__t)->t_sig.losigs ||		\
		(__p)->p_sig.hisigs || (__t)->t_sig.hisigs ||		\
		((__t)->t_cursig && 					\
			!SIGISMEMBER((__t)->t_sigmask,(__t)->t_cursig)) \
	)

#define SIG_AVAILABLE(__t,__p)						\
	((								\
	  ((__p)->p_sig.losigs | (__t)->t_sig.losigs) &			\
		~((__p)->p_sigignore.losigs | (__t)->t_sigmask.losigs)	\
	 ) || (								\
	  ((__p)->p_sig.hisigs | (__t)->t_sig.hisigs) &			\
		~((__p)->p_sigignore.hisigs | (__t)->t_sigmask.hisigs)	\
	 ) || (                                                         \
	   (__t)->t_cursig && 					        \
			!SIGISMEMBER((__t)->t_sigmask,(__t)->t_cursig)  \
	 ))

#define ADD_PENDING_SIG(__t,__p,__signo,__procsig)			\
{									\
	if (__procsig) {						\
		register struct thread *th = (__t)->t_nextthread;	\
		SIGADDSET((__p)->p_sig,(__signo));			\
		while (th != (__t)) {					\
			if (SIG_AVAILABLE((__t),(__p)))			\
				(__t)->t_flags |= TSIGAVAIL;		\
			th = th->t_nextthread;				\
		}							\
	} else {							\
		SIGADDSET((__t)->t_sig,(__signo));			\
	}								\
}

#define DEL_PENDING_SIG(__t,__p,__signo,__procsig)			\
{									\
	(__procsig) = TRUE;						\
	if (SIGISMEMBER((__p)->p_sig,(__signo))) {			\
		register struct thread *th = (__t)->t_nextthread;	\
		SIGDELSET((__p)->p_sig,(__signo));			\
		while (th != (__t)) {					\
			if (!SIG_AVAILABLE((__t),(__p)) && !(__t)->t_cursig)\
				(__t)->t_flags &= ~TSIGAVAIL;		\
			th = th->t_nextthread;				\
		}							\
	} else {							\
		(__procsig) = FALSE;					\
		ASSERT(SIGISMEMBER((__t)->t_sig,(__signo)));		\
		SIGDELSET((__t)->t_sig,(__signo));			\
	}								\
}

#define IS_PENDING_SIG(__p,__signo,__pending)				\
{									\
	(__pending) = FALSE;						\
	if (SIGISMEMBER((__p)->p_sig,(__signo)))			\
		(__pending) = TRUE;					\
	else {								\
		register struct thread *th = (__p)->p_threadlist;	\
		do {							\
			if (SIGISMEMBER(th->t_sig,(__signo))) {		\
				(__pending) = TRUE;			\
				break;					\
			}						\
			th = th->t_nextthread;				\
		} while (th != (__p)->p_threadlist);			\
	}								\
}
