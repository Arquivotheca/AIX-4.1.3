static char sccsid[] = "@(#)57	1.4  src/bos/usr/ccs/lib/libc/wait.c, libcsys, bos411, 9428A410j 6/16/90 01:33:41";
/*
 * LIBCSYS: wait3, wait, waitpid
 *                                                                    
 * ORIGIN: IBM
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#include <sys/types.h>
#include <errno.h>

extern pid_t	kwaitpid();

/*
 * NAME: wait3()
 *                                                                    
 * FUNCTION: Wait for child process(es) to terminate. The child-process
 *	chain, anchored in the current proc table entry, is scanned
 *	for zombie children. If none found, wait for one to show up.
 *	Also, we check for stopped (traced) children, and pass back
 *	status from them.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * NOTE: If global resource usage pointer is set, process rusage info
 *	 is copied out in the waitpid() system call
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= Terminating or stopped process not found
 *	 number = Process ID of terminating or stopped process
 *	-1	= failed, errno indicates cause of failure
 */
pid_t
wait3( stat_loc, options, rusage )
int		*stat_loc;	/* user location for returned status	*/
int		options;	/* options to vary function, see wait.h	*/
struct rusage	*rusage;	/* pointer to child resource usage area	*/
{
	int rc;

	/* rearrange arguments and call kwaitpid()			*/
restart:
	errno = 0; 	/* reset errno */
	rc = kwaitpid(stat_loc, -1, options, rusage);
	if ((-1 == rc) && (ERESTART == errno))
		goto restart;
	return(rc); 
}

/*
 * NAME: wait()
 *                                                                    
 * FUNCTION: Wait for child process(es) to terminate. The child-process
 *	chain, anchored in the current proc table entry, is scanned
 *	for zombie children. If none found, wait for one to show up.
 *	Also, we check for stopped (traced) children, and pass back
 *	status from them.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= Terminating or stopped process not found
 *	 number = Process ID of terminating or stopped process
 *	-1	= failed, errno indicates cause of failure
 */
pid_t
wait( stat_loc )
int		*stat_loc;	/* user location for returned status	*/
{
	int rc;
restart:
	errno = 0; 	/* reset errno */
	/* rearrange arguments and call kwaitpid()			*/
	rc = kwaitpid(stat_loc, -1, 0, NULL);
	if ((-1 == rc) && (ERESTART == errno))
		goto restart;
	return(rc);
}
/*
 * NAME: waitpid()
 *                                                                    
 * FUNCTION: Wait for child process(es) to terminate. The child-process
 *	chain, anchored in the current proc table entry, is scanned
 *	for zombie children. If none found, wait for one to show up.
 *	Also, we check for stopped (traced) children, and pass back
 *	status from them.  The search varies according to the pid value:
 *		= -1: all child processes, i.e. "wait()"
 *		>  0: single child process, pid
 *		=  0: child process group ID equals parent
 *		< -1: child process group ID is abs(pid)
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= Terminating or stopped process not found
 *	 number = Process ID of terminating or stopped process
 *	-1	= failed, errno indicates cause of failure
 */ 
pid_t
waitpid( pid,  stat_loc, options )
pid_t		pid;		/* pid value, -1, 0, process group pid	*/
int		*stat_loc;	/* user location for returned status	*/
int		options;	/* options to vary function, see wait.h	*/
{
	/* rearrange arguments and call kwaitpid()			*/
	int rc;
restart:
	errno = 0; 	/* reset errno */
	/* rearrange arguments and call kwaitpid()			*/
	rc = kwaitpid(stat_loc, pid, options, NULL);
	if ((-1 == rc) && (ERESTART == errno))
		goto restart;
	return(rc);
}
