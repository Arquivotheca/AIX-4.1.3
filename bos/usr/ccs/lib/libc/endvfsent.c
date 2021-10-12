static char sccsid[] = "@(#)17	1.9  src/bos/usr/ccs/lib/libc/endvfsent.c, libcadm, bos411, 9428A410j 6/16/90 01:02:06";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: endvfsent 
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

#include <sys/vfs.h>
#include <sys/vmount.h>

#include <errno.h>    
#include <stdlib.h>    
#include <stdio.h>
#include <fcntl.h>

#include "libvfs.h"
#include <fshelp.h>

/*
** endvfsent
**
** close the VFSfile's stream
**   the filesystem is gracious enough to remove the readlock
** also free any memory allocated by various control directives
*/
    
void
endvfsent()
{

  /*
  ** close VFSfile
  */
  if (VfsOpen)
  {
    (void) fclose (VfsStream);
    VfsOpen = 0;
  }
  /*
  ** free memory
  */
  if (default_local_vfs) {
      free ((void *) default_local_vfs);
      default_local_vfs = NULL;
  }
  if (default_remote_vfs) {
      free ((void *) default_remote_vfs);
      default_remote_vfs = NULL;
  }
  return;
}
