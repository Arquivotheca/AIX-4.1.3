static char sccsid[] = "@(#)83	1.3  src/bos/usr/ccs/lib/libc/mkfifo.c, libcproc, bos411, 9428A410j 3/5/94 17:02:02";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: mkfifo
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/limits.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/errno.h> 

/*
 *
 * SYNOPSIS: mkfifo() is given a pathname and a set of permissions for
 *	a pipe to be created. We then test against system limits and
 *	pass the request on to mknod(), a system call which will create
 *	the pipe, set the mode, and set up the file inode and time
 *	stats. We pick up the return code and errno (if any), then
 *	act accordingly.
 */

/* The mkfifo() function includes all the POSIX requirements */

extern int errno; 

int 
mkfifo(const char *path, mode_t mode) 
{ 

  if((strlen(path)) > PATH_MAX)
  {
    errno = ENAMETOOLONG;
    return(-1);
  }

/**********
  mknod() will do all the dirty work.
**********/
  mode |= S_IFIFO;	/* OR mode with the FIFO creation code */
  if((mknod(path, mode, 0)) != 0)
    return(-1);

  return(0); 
}
