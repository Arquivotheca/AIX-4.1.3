static char sccsid[] = "@(#)88	1.8  src/bos/usr/ccs/lib/libc/parsevfs.c, libcadm, bos411, 9428A410j 6/16/90 01:03:37";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: cntrl_default, cntrl_notimp 
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
#include <stdlib.h>    
#include <string.h>    
#include <stdio.h>
#include <fcntl.h>

#include "libvfs.h"
#include "fshelp.h"    

/*
** handle %default control directive
** returns 0 if good
**         non-zero otherwise (errno)
*/    
int
cntrl_default (nargs, av)
     int      nargs;
     char   **av;
{
  char                *local_vfs;
  char                *remote_vfs;

  if (av[0])
      local_vfs = av[0];
  if (av[1])
      remote_vfs = av[1];
  /*
  ** syntax is: %defaultvfs default_local [default_remote]
  */  
  switch (nargs)
  {
  case 2:                                    /* remote specified */
    if (default_remote_vfs)
    {
      /*
      ** just in case someone puts another %default
      */
      if (!(default_remote_vfs = realloc ((void *)default_remote_vfs, (size_t)
					  (strlen (remote_vfs) + 1))))
	    return ENOMEM;
    }
    else
    {
      if (!(default_remote_vfs = malloc ((size_t) (strlen (remote_vfs) + 1))))
	    return ENOMEM;
    }
    (void) strcpy (default_remote_vfs, remote_vfs);
    /*
    ** fall through to store local
    */
  case 1:                                    /* local specified */
    if (default_remote_vfs && !remote_vfs)   /* but not remote  */
    {                                        /* and remote was  */
      free ((void *)default_remote_vfs);             /* previously set, */
      default_remote_vfs = NILPTR (char);    /* ==> clear it    */
    }
    
    if (default_local_vfs)
    {
      if (!(default_local_vfs = realloc ((void *)default_local_vfs, (size_t)
					 (strlen (local_vfs) + 1))))
	    return ENOMEM;
    }
    else
    {
      if (!(default_local_vfs = malloc ((size_t) (strlen (local_vfs) + 1))))
	    return ENOMEM;
    }
    (void) strcpy (default_local_vfs, local_vfs);
    break;
  }
  return 0;
}

/*
** handle "not implemented" directives
** always return non-zero
*/
int
cntrl_notimp ()
{
  return -1;
}
