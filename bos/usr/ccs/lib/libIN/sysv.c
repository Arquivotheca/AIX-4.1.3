static char sccsid[] = "@(#)13	1.7  src/bos/usr/ccs/lib/libIN/sysv.c, libIN, bos411, 9428A410j 8/16/91 14:02:16";
/*
 * LIBIN: sysv, sysl
 *
 * ORIGIN: ISC
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Perform fork/exec[lv]p.
 *
 * RETURN VALUE DESCRIPTION: Returns process id of child.
 */

#include <signal.h>
#include <IN/standard.h>

#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* Possible flags       */
#define F_NOINTR          0001          /* run process immune to ints   */
#define F_NOQUIT          0002          /* run process immune to quit   */
#define F_APPEND          0004          /* append access to std output  */
#define F_DETACH          0040          /* detach it from the terminal  */

/*
 * run a program in an inferior process
 *      pgm     file to be executed
 *      flags   defined above
 *      infile  name of file to use for standard input (if not inherited)
 *      outfile name of file to use for standard output (if not inherited)
 *      errfile name of file to use for standard error (if not inherited)
 *      argv    address of the argument vector (ala execv)
 */
int sysv( pgm, flags, infile, outfile, errfile, argv )
 char *pgm, *infile, *outfile, *errfile, **argv;
 int flags;
{       register int pid;
	extern long lseek();

	pid = kfork();

	/*  spawn a new process         */
	if (pid == 0)
	{       /* open all of the appropriate files    */
		if (infile)
		{       close( STDIN );
			if (open(infile, 0) != STDIN)
				goto failure;
		}

		if (outfile)
		{       close( STDOUT );
			if (flags&F_APPEND)
				if (open(outfile, 1) != STDOUT)
					flags &= ~F_APPEND;
			if (!(flags&F_APPEND))
				if (creat(outfile, 0666) != STDOUT)
					goto failure;
		}

		if (errfile)
		{       close( STDERR );
			if (creat(errfile, 0666) != STDERR)
				goto failure;
		}

		/* handle all of the possible flags     */
		if (flags&F_NOINTR)
			signal( SIGINT, SIG_IGN );

		if (flags&F_NOQUIT)
			signal( SIGQUIT, SIG_IGN );

		if (flags&F_DETACH)
			signal( SIGHUP, SIG_IGN );

		if (flags&F_APPEND)
			lseek( STDOUT, 0L, 2 );


		execvp( pgm, argv );
		perror(pgm);

	failure:
		_exit( EXITEXEC );
	}

	return( pid );
}


/*
 *  sysl is identical to sysv, except that rather than taking a
 *      vector of arguments to be passed to the program, it allows
 *      them to be passed as individual arguments to the program
 *      (it is useful when the number of arguments is fixed and known)
 */
int sysl( pgm, flags, infile, outfile, errfile, arg0 )
 char *pgm, *infile, *outfile, *errfile, *arg0;
 int flags;
{       return  sysv( pgm, flags, infile, outfile, errfile, &arg0 );
}
