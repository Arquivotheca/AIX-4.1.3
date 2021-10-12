static char sccsid[] = "@(#)50	1.8  src/bos/usr/ccs/lib/libc/getbyname.c, libcadm, bos411, 9428A410j 6/16/90 01:02:24";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: getvfsbyname 
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
** getvfsbyname
**
** given a filesystem implementation name ex. "aix"
** get the associated vfs_ent from VFSfile ("/etc/vfs")
**
** returns NULL if not found or error
*/

struct vfs_ent *
getvfsbyname (vfsent_name)
     char *vfsent_name;
{
  extern struct vfs_ent  *getvfsent();
  struct vfs_ent         *vp;
  
  clear_fsherr();
  setvfsent ();
  for (vp = getvfsent(); vp; vp = getvfsent())
    if (strcmp (vp->vfsent_name, vfsent_name) == 0)
	break;
  endvfsent ();
  return (!vp || fsherr())? NILPTR (struct vfs_ent): vp;
}
