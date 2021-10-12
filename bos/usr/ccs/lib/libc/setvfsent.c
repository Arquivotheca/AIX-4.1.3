static char sccsid[] = "@(#)08	1.7  src/bos/usr/ccs/lib/libc/setvfsent.c, libcadm, bos411, 9428A410j 6/16/90 01:04:09";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: setvfsent 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/ 

#include <sys/vfs.h>
#include <sys/vmount.h>

#include <errno.h>    
#include <stdio.h>
#include <fcntl.h>

#include "libvfs.h"
#include <fshelp.h>

/*
** setvfsent
**
** open or rewind the VFSfile (defined in sys/vfs.h)
** if open, put a read lock on the whole file and
** associate a stream with it
*/
  
void
setvfsent()
{
  struct flock readlock;

  clear_fsherr();
  if (VfsOpen)
    rewind (VfsStream);
  else
  {
    if ((VfsFd = open (VFSfile, O_RDONLY | O_NDELAY)) < 0)
      set_fsherr (FSHERR_VFSAXS);
    else  
    {
      readlock.l_type   = F_RDLCK;
      readlock.l_whence = 0;
      readlock.l_start  = 0;
      readlock.l_len    = 0;

      VfsOpen++;
      if (fcntl (VfsFd, F_SETLK, &readlock) < 0 ||
	  !(VfsStream = fdopen (VfsFd, "r")))
      {
	set_fsherr (FSHERR_VFSAXS);
	endvfsent ();
      }
    }
  }
  return;
}
