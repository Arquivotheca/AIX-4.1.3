static char sccsid[] = "@(#)56	1.14  src/bos/usr/ccs/lib/libc/getvfsent.c, libcadm, bos411, 9428A410j 8/13/91 12:51:04";
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: getvfsent, init_vfs, clear_str, init_av 
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
#include <memory.h>

#include "fshelp.h"

/*LINTLIBRARY*/ 

extern	int fshlpr_errno;

int             VfsFd;
unsigned int    VfsOpen;
char            VfsBuf[BUFSIZ];
FILE           *VfsStream;

char            *default_local_vfs;
char            *default_remote_vfs;

/*
** buffers ...
*/
struct vfs_ent   Vfs;
char             Vfs_Name[BUFSIZ];
char             Vfs_Mnt_Hlpr[BUFSIZ];
char             Vfs_Fs_Hlpr[BUFSIZ];

/*
** for switching to appropriate control directive func
*/
extern cntrl_default();
extern cntrl_notimp();

/*
** special line prefixes
*/

#define COMMENT '#'
#define CONTROL '%'

struct cntrltab
{
  char    *name;        /* control directive string */
  int     (*func)();
}
    CntrlTab[] =
{
  "%defaultvfs",     cntrl_default,
  "%print",          cntrl_notimp,      /* "Not (Yet) Implimented" */
  0,                0
};

#define _NUM_FIELDS 6
static char         cntrl_buf[BUFSIZ];
static char         *cntrl[_NUM_FIELDS];
static char        *av[N_ARGS];


/*
** getvfsent
**
** read the VFSfile
**   blank lines are ignored
**   COMMENT lines get ignored
**   CONTROL lines cause their handler functions (CntlTab.func)
**           to be called 
**   other lines are valid vfs entries
**
** returns a pointer to a static vfs_ent struct
** on error returns 0
*/
   
struct vfs_ent *     
getvfsent ()
{
  char             *cp,*chk;
  int 		    valid_line,i;	
  int               nargs;
  int               rc;
  struct cntrltab  *ctp;
  void              init_vfs();
  void              init_av();
    
  clear_fsherr();
  /*
  ** if user has been lazy (ie, no setvfsent()),
  ** attempt to do the open ourselves
  */
  if (!VfsOpen)
      setvfsent ();
  if (fsherr())
      return NILPTR (struct vfs_ent);

readone:
  (void) memset ((void *) VfsBuf, 0, (size_t)sizeof (VfsBuf));

  /*
  ** skip comments
  */
  while ((cp = fgets (VfsBuf, (int)sizeof(VfsBuf), VfsStream)) &&
	 VfsBuf[0] == COMMENT);

  /*
  ** eof?
  */
  if (!cp)
      return NILPTR (struct vfs_ent);

  /*
  ** blank line?
  */  
  chk=cp;
  valid_line = 0 ;
  for(i=0;i<sizeof(VfsBuf);i++)
	{
	if(*chk == '\0' || *chk == '\n')
		break;
	if(*chk != ' ' && *chk != '\t')
		{
		valid_line = 1;
  		break;
		}
	chk++;
	}	

  if ( ! valid_line )
	goto readone ;	
  
  
  /*
  ** not a COMMENT, not eof
  ** check for CONTROL directive
  ** switch through CntrlTab
  */
  if (VfsBuf[0] == CONTROL)
  {
    strcpy(cntrl_buf, VfsBuf);
    if ((cntrl[0] = strtok(cntrl_buf, " \t\n")) == (char *)NULL)
      nargs=0;
    else {
      nargs = 1;
      for (i=1; i<_NUM_FIELDS; i++) {
        if((cntrl[i] = strtok((char *)NULL, " \t\n")) == (char *)NULL)
	  break;
        nargs++;
      }
    }

    if (!nargs)
    {
      set_fsherr (FSHERR_VFSMANGLED);
      return NILPTR (struct vfs_ent);
    }
    /*
    ** (don't count directive itself), terminate arg vector
    */
    init_av (--nargs);
    
    for (ctp = CntrlTab; ctp->name; ctp++)
    {
      if (strcmp (ctp->name, cntrl[0]) == 0 && ctp->func)
      {
	/*
	** FUNC (# args, arg vector)
	*/
	rc = (*ctp->func) (nargs, av);

	/*
	** handler functions return non-zero on error
	*/
	if (rc)
	{
	  set_fsherr (FSHERR_VFSMANGLED);
	  return NILPTR (struct vfs_ent);
	}
	break;
      }
    }
    /*
     ** now go back around and look for a vfs_ent
     */
    goto readone;
  }
  /*
  ** real vfs entry (or blank line) 
  */
  init_vfs ();
  if (!sscanf (VfsBuf, "%s %d %s %s", Vfs.vfsent_name, &Vfs.vfsent_type,
	       Vfs.vfsent_mnt_hlpr, Vfs.vfsent_fs_hlpr)
     )
      goto readone;
  
  /*
  ** we have a real entry, check for "No helper" cases
  ** and mark as default, as necessary
  */
  Vfs.vfsent_flags = 0;

  if (!strcmp (Vfs.vfsent_mnt_hlpr, NO_HELPER) ||
      !strcmp (Vfs.vfsent_mnt_hlpr, ""))
      Vfs.vfsent_mnt_hlpr = NULL;
  
  if (!strcmp (Vfs.vfsent_fs_hlpr, NO_HELPER) ||
      !strcmp (Vfs.vfsent_fs_hlpr, ""))
      Vfs.vfsent_fs_hlpr = NULL;

  if (default_local_vfs && !strcmp (Vfs.vfsent_name, default_local_vfs))
          Vfs.vfsent_flags |= VFS_DFLT_LOCAL;

  if (default_remote_vfs && !strcmp (Vfs.vfsent_name, default_remote_vfs))
          Vfs.vfsent_flags |= VFS_DFLT_REMOTE;

  return &Vfs;
}

void
init_vfs ()
{
  void             clear_str();

  Vfs.vfsent_name     = Vfs_Name;
  Vfs.vfsent_mnt_hlpr = Vfs_Mnt_Hlpr;
  Vfs.vfsent_fs_hlpr  = Vfs_Fs_Hlpr;
  Vfs.vfsent_type     = MNT_BADVFS;
  clear_str (Vfs_Name);
  clear_str (Vfs_Mnt_Hlpr);
  clear_str (Vfs_Fs_Hlpr);
  return;
}

void
clear_str(str)
  char *str;
{
  register int len;

  if ((len = strlen (str)))
      (void) memset ((void *) str, 0, (size_t)len);
  return;
}

void
init_av (nargs)
     int nargs;
{
  av[0]     = cntrl[1];
  av[1]     = cntrl[2];
  av[2]     = cntrl[3];
  av[3]     = cntrl[4];
  av[4]     = cntrl[5];
  av[nargs] = NILPTR (char);
  return;
}
