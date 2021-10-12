static char sccsid[] = "@(#)82	1.5  src/bos/usr/bin/dosdir/dosinit.c, cmdpcdos, bos411, 9433A411a 8/12/94 11:48:14";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dosinit _mkdevice _makestr 
 *
 * ORIGINS: 10,27
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


#include "tables.h"
#include <doserrno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>

int dostrace, doserrno;               /* to guarantee storage is allocated */
char	*bad_dev;     		      /* available ptr to invalid device */
char *getcwd();
int ipc_cleanup();

struct dev_entry {
	char *dosname, *envtname, *unixname;
	dstate content; }

_devices[] = {
       "A:",     "DOS_A",       "/dev/fd0",    is_unknown,
       "B:",     "DOS_B",       "/dev/fd0",    is_unknown,
       "C:",     "DOS_C",       "*",           is_unknown,
       "D:",     "DOS_D",       "/",           is_unknown,
       "NUL:",   "DOS_NUL",     "/dev/null",   is_io,
       "CON:",   "DOS_CON",     "/dev/tty",    is_io,
       "AUX:",   "DOS_AUX",     "/dev/tty0",   is_io,
       "COM1:",  "DOS_COM1",    "/dev/tty0",   is_io,
       "COM2:",  "DOS_COM2",    "/dev/tty1",   is_io,
       "LP0:",   "DOS_LP0",     "/dev/lp0",    is_io,
       "LP1:",   "DOS_LP1",     "/dev/lp1",    is_io,
       "LP2:",   "DOS_LP2",     "/dev/lp2",    is_io,
       "LP3:",   "DOS_LP3",     "/dev/lp3",    is_io,
       "LP4:",   "DOS_LP4",     "/dev/lp4",    is_io,
       "LP5:",   "DOS_LP5",     "/dev/lp5",    is_io,
       "LP6:",   "DOS_LP6",     "/dev/lp6",    is_io,
       "LP7:",   "DOS_LP7",     "/dev/lp7",    is_io,
       0 };

static int done = 0;

dosinit()
{

   /* this code is for head of process group.  It needs to be superceded
    * by code to check whether dosinit has been called for this process
    * and if not, whether there is an existing environment file.
    * If there is one, it must be opened and used.
    */

   register int i,j;
   char *s,*envfile,*getenv(),*malloc();
   char dbuf[256];
   char devname[3];
   char setname[6];
   register struct dev_entry *dev;
   register struct open_file *fp;
   register struct physical_device *pp;
   int bad_version = 0;
   char ipc_err = 0;
   char mount_stat = 0;

   if (done++) return(0);

   /* Initialize semaphore and shared memory stuff.
    */
   if (ipc_init() != 0)
	ipc_err++;

   signal(SIGINT, ipc_cleanup);
   signal(SIGQUIT, ipc_cleanup);
   bad_dev= NULL;
   envfile = getenv( "DOSENVT" );
   if (envfile && *envfile)
	if (e_restore(envfile))
	{       envfile = 0;
		bad_version++;
	}
   s = getenv( "DOSFILE" );
   if (s==0 || *s==0)            _e.pathtype = dos_paths;
   else if (strcmp(s,"DOS")==0)  _e.pathtype = dos_paths;
   else if (strcmp(s,"UNIX")==0) _e.pathtype = unix_paths;

   s = getenv( "DOSFORMAT" );
   if (s==0 || *s==0)            _e.format = tba_fmt;
   else if (strcmp(s,"DOS")==0)  _e.format = dos_fmt;
   else if (strcmp(s,"UNIX")==0) _e.format = unix_fmt;

   TRACE(("dosinit: pathtype=%d format=%d\n", _e.pathtype, _e.format));

   sprintf(_depath,"/tmp/dos%4.4d",getpid());        /* path for next envt */
   if (envfile && *envfile) return(0);
				   /* devices and open files are inherited */

   _e.dos_magic     = DOS_MAGIC;
   _e.next_logical  = 0;
   _e.next_physical = 0;
   _e.next_string   = 0;
   _e.current_disk  = -1;

   /*
    * pick up stdin, stdout & stderr from parent process
    */

   for( i=0; i<=2; i++ ) {
       fp = &file(i);
       fp->status        = is_open;
       fp->handle        = i;
       fp->format        = unix_fmt;
       fp->ldevice       = -1;
       fp->pathname      = malloc(1);
			       strcpy(fp->pathname,"");
       fp->oflag         = (i==0 ? DO_RDONLY : DO_WRONLY );
   }

   for( i=3; i<=4; i++ ) {                  /* reserve for STD PRN,AUX */
       fp = &file(i);
       fp->status        = is_ready;
       fp->ldevice       = -1;
   }

   for( i=5; i<NUM_OPENFILES; i++ ) {       /* all other files closed  */
       fp = &file(i);
       fp->status        = is_closed;
   }

   for( i=0; i<NUM_PHYSICAL; i++) {
	pp = &pd(i);
	pp->contents = is_unknown;
	pp->ldevice  = -1;
   }

   for (dev=_devices; dev->dosname!=0; dev++ )
     { /* setup default devices */
       s = getenv(dev->envtname);

       if (s==0)
         {                  
  	 s = dev->unixname;
	 if (*s=='*')       
	   s = getcwd (dbuf,256);
         }

       j = _mkdevice( dev->dosname, s, dev->content, dev->envtname );
       if (j<0) {
		bad_dev = dev->envtname;
		return(-1);
	}
   }

   strcpy(devname,"D:");
   strcpy(setname,"DOS_D");
   for( i=3; i<26; i++ ) {                     /* D: through Z:           */
       j = _mkdevice( devname, getenv(setname), is_unknown, setname );
       if (j<0) {
		bad_dev = setname;
		return(-1);
	}
       devname[0]++;
       setname[4]++;
   }
   j = _mkdevice( "$:", "/", is_unknown, "*" );
   if (j<0) return(-1);

   if (bad_version)
	err_return(DE_INIT);
   s = getenv( "DOSDISK" );                     /* overrride initial disk */
   if (s && *s && s[1]==':' && s[2]==0) {
	for(i=0; i<_e.next_logical-1; i++)      /* search for ldev #      */
	  if (strcmp(s,ld(i).device_id)==0) {
		  if ( trymount(i) ) return(0);
		  break;
	  }
   }
   for (i=0; i<_e.next_logical-1; i++) {
	  if ( trymount(i) )  { /* find initial disk */
		mount_stat++;
		break;
	}
   }
   if (mount_stat)
	if (ipc_err)
		return (-1);
	else
		return (0);
   else
	   err_return(DE_INIT);                /* can't mount any disks */
}

int    _mkdevice( dev, s, state, setname )
char   *dev,*s, *setname;
dstate state;
{
       LD     ldev;
       PD     pdev;
       DCB    *foo;
       char   *msg;
       struct stat statbuf;
       struct logical_device  *lp;
       struct physical_device *pp;

       if (s==0 || *s==0) return(0);
       if (*s != '/') err_return(DE_INIT);   /* not absolute filename */

       ldev = _e.next_logical;
       if( ldev >= NUM_LOGICAL ) err_return(DE_INIT);

       lp = &ld(ldev);
       strcpy( lp->device_id, dev);
       strcpy( lp->current_dir, "" );
       lp->open_count    = 0;
       lp->assigned_ld   = 0;

       /*
	* before alloting next physical device, search to see
	* whether this attached file is already connect to another device.
	* If it is, share it.
	*/

       for (pdev=0; pdev<_e.next_physical; pdev++ ) {
	   pp = &pd(pdev);
	   if ( strcmp(str(pp->attached_file),s) == 0 ) {
		if (pp->removable) {
		    lp->pdevice = pdev;
       		    _e.next_logical++;
		    TRACE(("mkdev: sharing ldev=%d pdev=%d %s\n",
			   ldev,pdev,pp->attached_file));
		    return(1);
		}
	   break;
	   }
       }

       pdev = _e.next_physical;
       if( pdev >= NUM_PHYSICAL ) err_return(DE_INIT);
       if( stat(s,&statbuf) < 0 ) return 1;   /* can't find, skip it */

       pp = &pd(pdev);
       pp->contents      = state;
       pp->removable     = FALSE;
       pp->attached_file = _makestr(s);  /* last def of pdevice will */
       pp->ldevice       = ldev;
       lp->pdevice       = pdev;

       switch (statbuf.st_mode & S_IFMT) {

	   case S_IFIFO:
	   case S_IFBLK:
	   case S_IFREG:
	   case S_IFCHR:

		   if (state == is_io) break;

		  /*
		   * if device is removable, mark it as such.
		   * Need a better way to tell it's removable.
		   * Mount doesn't happen until first open.
		   */

		   if ( strncmp( s, "/dev/f", 6 ) == 0 ) {
		      pp->removable = TRUE;
		      pp->ldevice = -1; /* not mounted */
		   }
		   break;

	   case S_IFDIR:
		   if (state == is_io) err_return(DE_INIT);
		   pp->mount_dir  = pp->attached_file;
		   pp->contents   = is_unix;
		   break;

	   default:err_return(DE_INIT);
       }
       _e.next_logical++;
       _e.next_physical++;

       TRACE(( "mkdev: id=%s set=%s file=%s ld=%d pd=%d\n",
		     dev, setname, s, ldev, pdev ));
       return(1);
}

int _makestr(s)
register char *s;
{
       STRING i;
       /* copy s into local string space & return pointer to s */

       i = _e.next_string;
       while( *s != 0 ) {
	    *str(_e.next_string++) = *s++;
	    if( _e.next_string >= STRINGSPACE ) err_return(DE_INIT);
       }
       *str(_e.next_string++) = 0;

       return(i);
}

static
int e_restore(s)
char *s;
{
	register int f,i;
	register char *sp;
	char path[128], *malloc();

	f = open(s,O_RDONLY);
	if (f<0) err_return(DE_ENVT);

	read( f, &_e, sizeof(_e) );
	if (_e.dos_magic != DOS_MAGIC) err_return(DE_ENVT);

	read( f, file_tbl,      sizeof(file(0))*NUM_OPENFILES );
	read( f, physical_tbl,  sizeof(pd(0))*_e.next_physical );
	read( f, logical_tbl,   sizeof(ld(0))*_e.next_logical );
	read( f, stringspace,   _e.next_string );
	for(i=0;i<NUM_OPENFILES;i++)
		if( file(i).status != is_closed ) {
			read( f, path, 128 );
			path[127] = 0;
			sp = malloc(strlen(path)+1);
			strcpy(sp,path);
			file(i).pathname = sp;
			TRACE(("restore: file %d `%s' %d\n",
				i, sp, file(i).status ));
		}
	close(f);
	TRACE(("e_restore: restored %s\n",s ));
	return(0);
}

	/* trymount: try to mount ldev as home disk device */
	/* returns 1 if successful, 0 on failure.          */
static
trymount(ldev)
{
       register int i;
       register struct physical_device *pp;
       pp = &pd(ld(ldev).pdevice);
       TRACE(("trymount: ld(%d) = %s\n",ldev,str(pp->attached_file)));
       if(pp->contents == is_unknown) {
		if (pp->removable) {     /* if can't open it, don't mount */
			if (pp->ldevice != -1) return 0; /* already tried */
			i = open(str(pp->attached_file),0);
			TRACE(("trymount: open %d\n", i ));
			if( i<0 ) return 0;
			close(i);
		}
		i = _mount(ldev);
		TRACE(("trymount: mount %d\n",i));
		if (i<0) return 0;
       }
       if (pp->contents != is_unix && pp->contents != is_dos) return(0);
       return ( doschdir(ld(ldev).device_id) >=0 );
}
