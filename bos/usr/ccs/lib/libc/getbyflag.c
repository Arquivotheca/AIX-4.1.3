static char sccsid[] = "@(#)39	1.7  src/bos/usr/ccs/lib/libc/getbyflag.c, libcadm, bos411, 9428A410j 6/16/90 01:02:20";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: getvfsbyflag 
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
#include <stdio.h>
#include <fcntl.h>

#include "libvfs.h"
#include <fshelp.h>

/*
** getvfsbytype
**
** given a vfs flag, pick the (1st)
** entry which has this flag turned on
**
** returns NULL if not found or error
*/

struct vfs_ent *
getvfsbyflag (vfsent_flag)
     int vfsent_flag;
{
  struct vfs_ent         *vp;
  
  clear_fsherr();
  setvfsent ();
  for (vp = getvfsent (); vp; vp = getvfsent ())
    if (vp->vfsent_flags & vfsent_flag)
	break;
  endvfsent ();
  return (!vp || fsherr())? NILPTR (struct vfs_ent): vp;
}
