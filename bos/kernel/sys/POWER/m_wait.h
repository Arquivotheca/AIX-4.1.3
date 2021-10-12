/* @(#)22	1.9  src/bos/kernel/sys/POWER/m_wait.h, sysproc, bos411, 9437A411a 9/12/94 12:43:08 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: WEXITSTATUS
 *		WIFEXITED
 *		WIFSIGNALED
 *		WIFSTOPPED
 *		WSTOPSIG
 *		WTERMSIG
 *		
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_M_WAIT
#define _H_M_WAIT

#include <sys/types.h>

union wait
{
	int	w_status;		/* used in syscall		*/

	struct				/* terminated process status	*/
	{
		unsigned int	w_PAD:16;
		unsigned int	w_Retcode:8;	/* exit code		*/
		unsigned int	w_Coredump:1;	/* core dump indicator	*/
		unsigned int	w_Termsig:7;	/* termination signal	*/
	} w_T;

	struct				/* Stopped process status	*/
	{
		unsigned int	w_PAD:16;
		unsigned int	w_Stopsig:8;	/* signal that stopped us */
		unsigned int	w_Stopval:8;	/* == W_STOPPED if stopped */
	} w_S;
};

#define	w_termsig	w_T.w_Termsig
#define w_coredump	w_T.w_Coredump
#define w_retcode	w_T.w_Retcode
#define w_stopval	w_S.w_Stopval
#define w_stopsig	w_S.w_Stopsig

/*
 * MACRO defines for application interfacing to waitpid(), wait(), and wait3()
 * Redefine wait MACRO's to utilize the wait union construct
 */
#undef	WIFSTOPPED
#undef	WSTOPSIG
#undef	WIFEXITED
#undef	WEXITSTATUS
#undef	WIFSIGNALED
#undef	WTERMSIG
/* evaluates to a non-zero value if status returned for a stopped child	*/
#define	WIFSTOPPED(x)	( (x).w_status & W_STOPPED )
/* evaluates to the number of the signal that caused the child to stop	*/
#define	WSTOPSIG(x)	(int)(WIFSTOPPED(x) ? (((x).w_status >> 8) & 0x7f) : -1)
/* evaluates to a non-zero value if status returned for normal termination */
#define	WIFEXITED(x)	( !((x).w_status & 0xff) )
/* evaluates to the low-order 8 bits of the child exit status	*/
#define	WEXITSTATUS(x)	(int)(WIFEXITED(x) ? (((x).w_status >> 8) & 0xff) : -1)
/* evaluates to a non-zero value if status returned for abnormal termination */
#define	WIFSIGNALED(x)	(  !WIFEXITED(x) && !WIFSTOPPED(x) )
/* evaluates to the number of the signal that caused the child to terminate */
#define	WTERMSIG(x)	(int)(WIFSIGNALED(x) ? ((x).w_status & 0x7f) : -1)

#endif /* _H_M_WAIT */
