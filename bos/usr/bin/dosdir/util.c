static char sccsid[] = "@(#)22	1.8  src/bos/usr/bin/dosdir/util.c, cmdpcdos, bos411, 9428A410j 6/30/93 14:22:34";
/*
 * COMPONENT_NAME: CMDPCDOS  routines to read dos floppies
 *
 * FUNCTIONS: _trace _file_state _mount _unmount _runmount get_dr_upath 
 *            get_dr1_upath absolute_dos_pathname
 *
 * ORIGINS: 10,27
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


#include   "tables.h"
#include   "doserrno.h"
#include   <sys/param.h>              /* defines time_t used by mnttab.h */
#include   <fcntl.h>
#include <sys/vmount.h>

#include <nl_types.h> 
#include "dosdir_msg.h" 
#define MSGSTR(N,S) catgets(catd,MS_DOSDIR,N,S) 
extern nl_catd catd;

extern int doserrno;

_trace(s,a1,a2,a3,a4,a5)
char *s;
int  a1,a2,a3,a4,a5;
{
	char msg[256];
	if( dostrace ) {
		sprintf(msg,s,a1,a2,a3,a4,a5);
		write(2,msg,strlen(msg));
	}
}

dstate _file_state(f)
DOSFILE f;
{
      register LD ldev;
      register PD pdev;

      /* f must be open */

      ldev = file(f).ldevice;
      if (ldev == -1) return( is_unix );  /* standard in, out, etc */

      pdev = ld(ldev).pdevice;
      return( pd(pdev).contents );
}

int _mount(ldev)
LD ldev;
{
    register int  mntfile,i,n,oldldev;
    PD   pdev;
    char *devname,s[75];
    DCB  *dmount();
    struct vmount *vmt;
    char 	 *mnttab;
    int count;
    int ct;

    pdev = ld(ldev).pdevice ;
    oldldev = pd(pdev).ldevice ;

    if ((oldldev == ldev) &&
	  ((pd(pdev).contents == is_unix) ||
	   (pd(pdev).contents == is_io) ||
	   (pd(pdev).contents == is_dos)))
	   return(0);
	   /* this ldev is already mounted */

    TRACE(("mount pdev %d, ldev %d, old ldev %d, contents %d, devname %s\n",
	   pdev, ldev, oldldev, pd(pdev).contents,
	   str(pd(pdev).attached_file) ));

	if (ldev == _e.current_disk) chdir("/usr/dos/nulldir");

	if (oldldev != -1 && oldldev != ldev) {
	   /* pdev has another ldev on it -- unmount it */
	   TRACE(("mount: multiplexing device\n"));
	   _unmount(pdev);

	   sprintf(s,
	   MSGSTR(INSDISK, 
	    "Insert diskette for drive %s and strike\nany key when ready\n"), 
	    ld(ldev).device_id );
	   _pause(s);
	   write(2,"\n",1);

	   pd(pdev).contents  = is_unknown;
    }
    pd(pdev).ldevice   = ldev;

    /*
     *     Try mounting as a DOS filesystem
     */

    devname = str(pd(pdev).attached_file) ;
    pd(pdev).dcb = dmount(devname,ld(ldev).current_dir,&ld(ldev).cd_off_t);
    if( pd(pdev).dcb != NULL ) {
	   TRACE(("Mounted dos filesystem %d (%s)\n",pdev,devname));
	   pd(pdev).contents = is_dos;
	   return(0);
    }
	else
		TRACE(("DOSmount failed, doserrno = %d\n",doserrno));

    /*
     *   At this point, the only possibility is that we are mounting a
     *   UNIX filesystem.  In that case it must be a device connected
     *   to a particular UNIX directory.  The following code searches
     *   /etc/mnttab to see if that is the case.
     */

    if ( strncmp(devname, "/dev/", 5) != 0 ) {
	   TRACE(("Can't mount %s (ldev=%d)\n",devname,ldev));
	   err_return( DE_NOMNT );
    }
    devname += 5;

  
     ct = mntctl( MCTL_QUERY, sizeof(count), &count);
     if ( ct < 0) {
   	perror("_mount");
   	err_return(DE_NOMNT);
     }
     mnttab = malloc( count);
     ct = mntctl( MCTL_QUERY, count, mnttab);
     if ( ct < 0) {
   	perror("_mount");
   	err_return(DE_NOMNT);
     }
   
     TRACE(("mount: scanning /etc/mnttab for %s\n",devname));
   
     for( vmt=(struct vmount *)mnttab; ct > 0;ct--) {
        TRACE(("\t%s\t%s\n", vmt2dataptr( vmt, VMT_OBJECT),
   	     vmt2dataptr( vmt, VMT_STUB)));
        if (strcmp( devname, vmt2dataptr( vmt, VMT_OBJECT)) == 0) {
   	   pd(pdev).mount_dir = _makestr(vmt2dataptr( vmt, VMT_STUB));
   	   pd(pdev).contents = is_unix;
   	   TRACE(("mount: found %s\n", vmt2dataptr(vmt, VMT_STUB)));
   	   return(0);
        }	
     }
 }

_unmount(pdev)
PD pdev;
{
     register dstate c;
     register struct physical_device *p;
     int i;
     p = &pd(pdev);
     c = p->contents;
     if ( c == is_unknown) return;
     if ( p->removable || c == is_dos)
     {       if ( c == is_dos && p->dcb )
	     {       i = dunmount( p->dcb );
	     	     TRACE(( "unmount: pd=%d, rc=%d\n", pdev, i ));
	             p->dcb = 0;
	     }
	     p->contents = is_unknown;
     }
     /* leave p->ldevice as it is, it is used to remember which logical
      * device (diskette) is expected to be on this physical device (drive)
      */
}

_runmount()
{
register int i;
register LD ldev;
register DCB *disk;

	for( i=0; i<NUM_PHYSICAL; i++ )
	   if (pd(i).removable || pd(i).contents == is_dos)
	   {    /* get current directory pointer before unmounting    */
		/* to use in "diskette changed" test in DMOUNT()      */
		ldev = pd(i).ldevice;
		if (pd(i).contents == is_dos) {
			disk = pd(i).dcb;
			ld(ldev).cd_off_t = current.dir[disk->home].pathname;
		}
		/* unmount removable filesystems to write FAT */
		/* so diskette can be changed.                        */
		_unmount(i);
	   }
}



/*      get_dr_upath -  returns 0 or negative return code;
			copies unix path for "drive" into "path".
	drive - a pointer to a char string containing the drive name 
	path  - a pointer to a char array large enough to contain a unix path
 */

int  
get_dr_upath(drive, path)
char *drive, *path;
{
	return get_dr1_upath(drive,path,1);
}


int  
get_dr1_upath(drive, path, assign)
char *drive, *path;
int assign;
{
	char device[10];
	register LD ldev;
	PD pdev;
	int i;

	TRACE(("get_dr_upath: %s\n",drive));

	if (*drive == 0)
		strcpy(device,ld(_e.current_disk).device_id);
	else
	{       if (drive[1] != ':') err_return( DE_NODEV );
		strncpy(device,drive,2);
		device[2] = '\0';
	}
	_upcase(device);
	for (ldev=0; ldev<NUM_LOGICAL; ldev++)
	   if (strcmp(ld(ldev).device_id,device)==0) break;
	if (ldev==NUM_LOGICAL) err_return( DE_NODEV );

	if (assign && (i = ld(ldev).assigned_ld)) ldev = i;
	pdev = ld(ldev).pdevice;

	strcpy( path, str(pd(pdev).attached_file) );
	return(0);
}
/*
 *      converts name to absolute DOS full path name
 *      writes return string into user supplied pathout
 *      collapses .. and . elements
 *      if pathin==NULL or *pathin==NULL, returns absolute path of dir\*.*
 *      returns 0 on success, -1 on failure; doserrno set on failure
 */

absolute_dos_pathname(pathin,pathout)
register char *pathin, *pathout;
{
register char *home, *away;
char tmp[128], *dospwd();

	home = 0;
	if (pathin && *pathin)                          /* argument exists? */
		if (pathin[1] == ':')               /* contains drive spec? */
		{       *tmp = '\0';
			get_dr_upath(pathin,tmp);      /* validate drive id */
			if (*tmp == '\0')
			{       doserrno=DE_NODEV;
				*pathout = '\0';
				return(-1);
			}
			if (pathin[2] == '\\')        /* absolute pathname? */
				strcpy(pathout,pathin);
			else                        /* drive spec, relative */
			{       away = dospwd(pathin);
				strcpy(pathout,away);
				if (pathout[strlen(pathout)-1]!='\\')
					strcat(pathout,"\\");
				strcat(pathout,&pathin[2]);
				free(away);
			}
		}
		else                                       /* no drive spec */
		{       home = dospwd(0);
			if (*pathin=='\\')            /* absolute pathname? */
			{       strcpy(pathout,home);
				pathout[2] = '\0';
				strcat(pathout,pathin);
			}
			else                     /* no drive spec, relative */
			{       strcpy(pathout,home);
				if (pathout[strlen(pathout)-1]!='\\')
					strcat(pathout,"\\");
				strcat(pathout,pathin);
			}
		}
	else                                                 /* no argument */
	{       home = dospwd(0);
		strcpy(pathout,home);
		if (pathout[strlen(pathout)-1]!='\\')
			strcat(pathout,"\\");
		strcat(pathout,"*.*");
	}
	if (home)
		free(home);

   {  register char *q, *pq, *np;
      register lvl;
	pq = pathout;
	np = pq;
	while(*pq && pq[1])                     /* remove \. from pathnames */
	{       if (strncmp(pq,"\\.",2)==0)
		{       q = pq+2;
			if ((*q == '\\') || (*q == '\0'))
			{       *++pq = '\0';
				if (strlen(q)>1)          /* anything left? */
					strcat(np,++q);
				--pq;      /* continue at last char checked */
			}
			else
				pq++;
		}
		else
			pq++;
	}
	lvl = 0;
	pq = pathout;
	np = pq;
	while(*pq && pq[1])                     /* remove .. from pathnames */
	{       if (strncmp(pq,"..",2)==0)
		{       q = pq+2;
			pq -=2;           /* step over preceeding backslash */
			while(((pq-np)>2) && (*pq != '\\'))
				pq--;      /* look for preceeding backslash */
			if (*pq != '\\')               /* invalid directory */
			{       lvl++;
				if (strlen(q))
					lvl++;
				break;
			}
			if ( ((pq-np)==2) && (strlen(q)==0) )
				pq++;
			*pq = '\0';
			strcat(np,q);
		}
		else
			pq++;
	}
	if (lvl)
	{       *pathout = '\0';
		doserrno = DE_NOENT;
		return(-1);
	}
   }
	return(0);
}
