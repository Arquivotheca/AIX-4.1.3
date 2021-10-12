#ifndef LINT
static char sccsid [] = "@(#)28	1.28.1.1  src/bos/usr/ccs/lib/libc/fshelp.c, libcadm, bos411, 9428A410j 11/12/93 13:31:18";
#endif
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: fshelp, char_or_block, check_helper, lock_fs, unlock_fs,
 *            do_op, tidy, fshlpr_perror, invalid_reference, trap_sig 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>    
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>    
#include <string.h>
#include <memory.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/limits.h>    
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define _I_AM_FSHELP
#include <fshelp.h>
#include "libvfs.h"

#include <locale.h>


#include "fshelp_msg.h"

static nl_catd	nls_catd = CATD_ERR;

#define ERR_MSG(num,str)    NLcatgets (nls_catd, (int)MS_FSHELP_ERR, (int)num, (char *)str)
#define MISC_MSG(num,str)   NLcatgets (nls_catd, (int)MS_FSHELP_MISC, (int)num, (char *)str)


typedef enum { True = TRUE, False = FALSE } bool_t;

bool_t check_helper(), lock_fs(), unlock_fs(), invalid_reference();
    
#define BAD_FD (-1)

/*
** helper error globals
*/
struct fsherrmsg
{
  char    *msg;
  ushort   msgno;
} fsherr_msgs[] =
{
  "No worries, mate!",			0,
  "Invalid",				Msg_INVAL,
  "Invalid filesystem",			Msg_INVALFS,
  "Invalid file number",		Msg_INVALFILNO,
  "Invalid data block",			Msg_INVALDBLK,
  "Invalid mode",			Msg_INVALMODE,
  "Invalid argument",			Msg_INVALARG,
  "Access denied or could not execute",	Msg_AXSHELPER,
  "Argument is missing",		Msg_ARGMISSING,
  "That is a no-op.",			Msg_NOP,
  "Syntax error",			Msg_SYNTAX,
  "Unknown vfs type",			Msg_UNKVFS,
  "Unknown filesystem helper operation",
  					Msg_UNKOP,
  "VFS file is mangled",		Msg_VFSMANGLED,
  "VFS file access denied",		Msg_VFSAXS,
  "Filesystem helper does not support this operation",
  					Msg_NOTSUP,
  "This would block",			Msg_WOULDBLOCK,
  "Inappropriate action on an active filesystem",
  					Msg_ACTIVE,
  "Can not lock the device",		Msg_CANTLOCK,
  "Device open failed",			Msg_DEVOPEN,
  "Device operation failed",		Msg_DEVFAIL,
  "Device read failed",			Msg_DEVFAILRD,
  "Device write failed",		Msg_DEVFAILWR,
  "Filesystem is corrupt",		Msg_CORRUPT,
  "Out of memory",			Msg_NOMEM,
  "Internal error",			Msg_INTERNAL,
  "Implementation-specific error, code = ",
  					Msg_IMPERR,
  "Unknown error, code = ",		Msg_UNKERR
};

static int   DebugLevel; 
char         Hpath[PATH_MAX];
char         fshmods[] = { FSHMOD_INTERACT_FLAG,
			   FSHMOD_FORCE_FLAG,
			   FSHMOD_NONBLOCK_FLAG,
			   FSHMOD_PERROR_FLAG,
			   FSHMOD_ERRDUMP_FLAG,
			   FSHMOD_STANDALONE_FLAG,
			   FSHMOD_IGNDEVTYPE_FLAG};

/*
** ops are known to behave as O_RDONLY, O_WRONLY, O_RDWR (fcntl.h)
** this is used to set the appropriate lock on the fsdev
*/
#define   O_UNK   (-1)


/*
** op descriptions
*/

struct fshop
{
   char    *name;
   int      flag;        /* as for open()     */
   bool_t   talksback;   /* op generates data */
}
fshops[FSHOP_NUM_OPS] =
{
  { "fshop_null",    O_UNK,    False },
  { "fshop_check",   O_RDWR,   True  },
  { "fshop_chgsiz",  O_RDONLY, True  },
  { "fshop_findata", O_RDONLY, True  },
  { "fshop_free",    O_RDWR,   False },
  { "fshop_make",    O_RDWR,   True  },
  { "fshop_rebuild", O_RDWR,   False },
  { "fshop_statfs",  O_RDONLY, True  },
  { "fshop_stat",    O_RDONLY, True  },
  { "fshop_usage",   O_RDONLY, True  },
  { "fshop_namei",   O_RDONLY, True  },
  { "fshop_debug",   O_RDWR,   True  }
};

int fshlpr_errno = FSHERR_GOOD;


/*
** fshelp
**
**  the general entry point to the filesystem helper of choice
**
*/

int
fshelp (fsdev, vfsnam, op, mode, debuglevel, opflags, opdata)
     char   *fsdev;
     char   *vfsnam;
     int     op;
     int     mode;                 
     int     debuglevel;
     char    *opflags;
     caddr_t  opdata;
{
  register         i;
  int              op_ok  = 0;
  int              rc     = FSHRC_BAD;
  int              fsfd   = BAD_FD;
  int              lockfd = BAD_FD;
  bool_t           block_dev = False;
  bool_t           invalid_reference();
  bool_t           lock_fs();
  void             tidy();
  struct vfs_ent  *vfsp;
  char            *vfs    = NILPTR (char);
  char            *helper = NILPTR (char);


  (void) setlocale (LC_ALL, "");
  
  if (nls_catd == CATD_ERR)
      nls_catd = catopen (MF_FSHELP, NL_CAT_LOCALE);

  
  /*
  ** reset error flag
  */

  clear_fsherr();

  /*
  ** check args
  */
  /*
  ** First, verify existence and legality of addresses
  ** passed in to us
  */
  if (!fsdev || invalid_reference (fsdev))
  {
    set_fsherr (FSHERR_INVALFS);
    goto out;
  }
  if (!vfsnam || invalid_reference (vfsnam))
  {
    set_fsherr (FSHERR_INVALARG);
    goto out;
  }
  if ((opflags && invalid_reference (opflags)) ||
      (opdata && invalid_reference (opdata)))
  {
    set_fsherr (FSHERR_INVALARG);
    goto out;
  }

  /*
  ** side effect: block_dev is (re)set
  ** (to minimize the number of stat() calls)
  */

  if (mode & FSHMOD_IGNDEVTYPE) 
      if (!char_or_block (fsdev, &block_dev)) {
	  set_fsherr (FSHERR_INVALFS);
	  goto out;
      }

  if ((vfs = (char *)malloc (strlen (vfsnam) + 1)) == NILPTR (char))
  {
    set_fsherr (FSHERR_NOMEM);
    goto out;
  }

  if (strncpy (vfs, vfsnam, strlen (vfsnam) + 1) != vfs)
  {
    set_fsherr (FSHERR_INTERNAL);
    goto out;
  }
  
  if (!(vfsp = getvfsbyname (vfs)))
  {
    set_fsherr (FSHERR_UNKVFS);
    goto out;
  }

  if ((helper = (char *)malloc (strlen (vfsp->vfsent_fs_hlpr) + 1)) == NILPTR (char))
  {
    set_fsherr (FSHERR_NOMEM);
    goto out;
  }
  if (strncpy (helper, vfsp->vfsent_fs_hlpr, strlen (vfsp->vfsent_fs_hlpr)+1)
      != helper)
  {
    set_fsherr (FSHERR_INTERNAL);
    goto out;
  }

  for (i = FSHOP_NULL+1; i < FSHOP_NUM_OPS; i++)
      if (i == op)
      {
	op_ok++;
	break;
      }
  
  if (!op_ok)
  {
      set_fsherr (FSHERR_UNKOP);
      goto out;
  }
  
  if (mode & ~FSHMOD_ALL)
  {
    set_fsherr (FSHERR_INVALMODE);
    goto out;
  }

  /*
  ** no checking of debuglevel, opflags
  ** that's the op's job
  */
  DebugLevel = debuglevel;

  if (check_helper (helper))
  {
    set_fsherr (FSHERR_AXSHELPER);
    goto out;
  }
  /*
  ** Upon success do_op (may) put data into opdata
  */
  if (lock_fs (fsdev, mode, op, &fsfd, &lockfd, block_dev) ||
      ((mode & FSHMOD_FORCE)) && fsfd != BAD_FD)
  {
    rc = do_op (fsfd, op, mode, debuglevel, opflags, opdata);
    if (lockfd != BAD_FD && !unlock_fs (mode, lockfd))
	set_fsherr (FSHERR_DEVFAIL);
  }
  else if (fsfd == BAD_FD)
	set_fsherr (FSHERR_DEVOPEN);
  
 out:

  tidy (mode, fsfd, lockfd);
  return rc;
}
    

/*
** char_or_block
**
**  checks to make sure filesystem is character or block
**  although it makes more sense to use the raw device
**  it is the left up to the helper implementation to 
**  operate on the device given to it.
** SIDE EFFECT:
**  upon success, if the device is a block device
**  the block_dev_ptr is set; if not its reset
** RETURNS:
**  0 on error, non-zero on success 
*/
     
int
char_or_block (dev, blk_dev_ptr)
     char    *dev;
     bool_t  *blk_dev_ptr;
{
  struct stat   statbuf;
  mode_t	mode;
  
  if (stat (dev, &statbuf))
      return FALSE;

  mode = statbuf.st_mode & S_IFMT;
  *blk_dev_ptr = mode == S_IFBLK;
  
  return mode == S_IFCHR || mode == S_IFBLK;
}


/*
** check_helper
**
** If hlpr_path is a basename path, try VFSdir/hlpr_path
** otherwise just use it as is.
** 
*/

bool_t
check_helper (hlpr_path)
     char *hlpr_path;
{
  /*
  ** basename? ==> try VFSdir
  */
  if (strchr (hlpr_path, '/') == NILPTR (char))
      (void) sprintf (Hpath, "%s%s", VFSdir, hlpr_path);
  else
      (void) strncpy (Hpath, hlpr_path, sizeof (Hpath));

  return (bool_t) access (Hpath, X_OK | R_OK) != 0;
}

/*
** lock_fs  
**
** Filocks are used to coordinate access to a filesystem device
** The raw device is used if it is possible to determine it.
** Otherwise, punt.  After all, these operations aren't for naive
** users, anyway.
*/

bool_t
lock_fs (fsdev, mode, op, fsfd_ptr, lckfd_ptr, blkdev)
     char        *fsdev;
     int          mode;
     int          op;
     int         *fsfd_ptr;
     int         *lckfd_ptr;
     bool_t       blkdev;
{
  char            lockfile[PATH_MAX];
  char           *dname;
  char           *bname;
  struct flock    fslock;
  int             lockcmd;
  extern char    *dirname();
  extern char    *basename();
  
  /*
  ** if given a block device, attempt to find the raw
  ** until (if ever) there's an intelligent way to do this,
  ** just use the analogous device name with an 'r' prepended
  */ 
  if (blkdev)
  {
    dname = dirname (fsdev);
    bname = basename (fsdev);
/*
    if (!dname || !bname || sprintf (lockfile, "%s/r%s", dname , bname) < 0)
*/
    if (!dname || !bname || sprintf (lockfile, "%s/%s", dname , bname) < 0)
	return False;
  }
  else if (strcpy (lockfile, fsdev) != lockfile)    /* raw */
      return False;
  
  /*
  ** by clearing the whole lock structure (l_start = l_end = 0)
  ** we will be requesting a lock on the whole file
  */
  (void) memset ((void *) &fslock, 0, (size_t)sizeof(fslock));
  lockcmd         = (mode & FSHMOD_NONBLOCK)?     F_SETLK: F_SETLKW;
  fslock.l_type   = fshops[op].flag != O_RDONLY?  F_WRLCK: F_RDLCK;

  if (!strcmp (lockfile, fsdev))
  {
    if ((*lckfd_ptr = *fsfd_ptr = open (fsdev, fshops[op].flag, 0)) < 0)
	return False;
  }
  else if ((*lckfd_ptr = open (lockfile, fshops[op].flag, 0)) < 0 ||
	   (*fsfd_ptr  = open (fsdev, fshops[op].flag, 0)) < 0)
      return False;
  
  if (fcntl (*lckfd_ptr, lockcmd, &fslock) < 0)
  {
    /*
    ** if trying to set lock and told to try again ==> WOULDBLOCK
    ** otherwise, we've got a problem
    */
    set_fsherr (errno == EAGAIN && lockcmd == F_SETLK?
	        FSHERR_WOULDBLOCK: FSHERR_INTERNAL);
    return False;
  }
  return True;
}
    

/*
** unlock_fs
**
** unlock the lockfile associated with a filesystem
*/

bool_t
unlock_fs (mode, lckfd)
     int    mode;
     int    lckfd;
{
  struct flock  fslock;
  int           lockcmd;
  
  fslock.l_type   = F_UNLCK;
  fslock.l_whence = 0;
  fslock.l_start  = 0;
  fslock.l_len    = 0;

  lockcmd = (mode & FSHMOD_NONBLOCK)? F_SETLK: F_SETLKW;
  
  return (bool_t) (fcntl (lckfd, lockcmd, &fslock) >= 0);
}
    

/*
** do_op
**
** convert all internal forms of args to strings
** set up file descriptor to capture output on, iff necessary
** He's down the court, he's going for it --- SWIIITCH!
** and check for errors
*/

int    
do_op (fsfd, op, mode, debuglevel, opflags, opdata)
     int        fsfd;
     int        op;
     int        mode;
     int        debuglevel;
     char      *opflags;
     caddr_t    opdata;
{
  char         *av[N_ARGS+1];
  char          devfdbuf[16];
  char          comfdbuf[16];
  char          dbgbuf[16];
  char          modebuf[32];
  char          opbuf[32];
  char          pipbuf[UBSIZE];
  register int  i;
  pid_t		pid;
  int           rc     = FSHERR_GOOD;
  int           rdcnt  = 0;
  int           bufpos = 0;
  int           child;
  struct fshop  thisop;
  bool_t        op_babbles = False;
#define READ  0
#define WRITE 1  
  int           pipefds[2];
#ifdef lint
  void          exit();
#endif  

  if (sprintf (opbuf, "%d", op) < 0)
  {
    set_fsherr (FSHERR_INTERNAL);
    return FSHRC_BAD;
  }

  if (sprintf (devfdbuf, "%d", fsfd) < 0)
  {
    set_fsherr (FSHERR_INTERNAL);
    return FSHRC_BAD;
  }

  if (mode & FSHMOD_ALL)
  {
    (void) strcpy (modebuf, "-");
    for (i = 0; i < sizeof fshmods; i++)
      if (mode & (1 << i))
	  (void) strncat (modebuf, &fshmods[i], (size_t)1);
  }
  else
      (void) strcpy (modebuf, "-");

  if (sprintf (dbgbuf, "%d", debuglevel) < 0)
  {
    set_fsherr (FSHERR_INTERNAL);
    return FSHRC_BAD;
  }

  thisop      = fshops[op];
  av[A_NAME]  = thisop.name;             /* op name          */
  av[A_OP]    = opbuf;                  /* op key            */
  av[A_FSFD]  = devfdbuf;               /* dev fd            */
  av[A_COMFD] = comfdbuf;		/* comm. channel fd  */
  av[A_MODE]  = modebuf;                /* mode flags        */
  av[A_DEBG]  = dbgbuf;                 /* debug level       */
  av[A_FLGS]  = opflags;                /* op specific flags */
  av[N_ARGS]  = NILPTR (char);

  /*
  ** debugging rule (feature/hack),
  ** everyone spews output if debugging level is high enough
  */
  if (DebugLevel > FSHBUG_BLOOP)
      op_babbles = True;
  
  /*
  ** redirect stdout of helper if we're expecting output
  */
  if (thisop.talksback || op_babbles)
  {
    if (pipe (pipefds) < 0)
    {
      set_fsherr (FSHERR_INTERNAL);
      return FSHRC_BAD;
    }
    else 
    {
      if (sprintf (comfdbuf, "%d", pipefds[WRITE]) < 0)
      {
	set_fsherr (FSHERR_INTERNAL);
	return FSHRC_BAD;
      }
    }
  }
  else
      (void) strcpy (comfdbuf, "1");	/* stdout */

  if (!(pid = fork ()))
  {                                       /* child */
    (void) close (pipefds[READ]);
    (void) execv (Hpath, av);
    exit (FSHERR_INTERNAL);
  }
  else if (pid > 0)                          /* parent */
  {
    (void) close (pipefds[WRITE]);
    if (thisop.talksback || op_babbles)
    {
      while ((rdcnt = read (pipefds[READ], pipbuf, sizeof pipbuf)) > 0)
      {
	/*
	** if opdata is null, don't bother EFAULT'ing, assume
	** application wants the user to see what output is being
	** generated down below
        */
	if (op_babbles || opdata == NILPTR (char))
	{
	  if (write (2, pipbuf, (unsigned)rdcnt) < 0)	/* stderr */
	  {
	    set_fsherr (FSHERR_INTERNAL);
	    return FSHRC_BAD;
	  }
	}
	else
	{
	  (void) memcpy ((void *)(opdata + bufpos), (void *)pipbuf, (size_t)rdcnt);
	}
	bufpos += rdcnt;
      }
    }
    (void) wait (&child);
    if ((rc = WEXITSTATUS (child)) != FSHERR_GOOD)
	set_fsherr (rc);
    (void) close (pipefds[READ]);
  }
  else if (pid < 0)
  {
    (void) close (pipefds[READ]);
    set_fsherr (FSHERR_INTERNAL);
    return FSHRC_BAD;
  }

  /*
  ** no errors (GOOD),
  ** error and not in force mode (BAD),
  ** an error but in force mode (UGLY).
  */

  return rc == FSHERR_GOOD?
             FSHRC_GOOD
	 : rc != FSHERR_ACTIVE || !(mode & FSHMOD_FORCE)?
	     FSHRC_BAD
         :
	     FSHRC_UGLY;
}
    
  
/*
** tidy
**
**  close things, spew messages and cleanup
*/

void
tidy (mode, fsfd, lockfd)
    int  mode;
    int  fsfd;
    int  lockfd;
{
  if (fsfd != BAD_FD)
      close (fsfd);
  if (lockfd != fsfd && lockfd != BAD_FD)
      close (lockfd);
  
  (void) fflush (stdout);
  
  if (mode & FSHMOD_PERROR || DebugLevel >= FSHBUG_BLOOP)
      fshlpr_perror ("Filesystem Helper");
  
  (void) fflush (stderr);
  if (mode & FSHMOD_ERRDUMP)
      (void) abort ();
    
  if (nls_catd != CATD_ERR)
  {
    (void) catclose (nls_catd);
    nls_catd = CATD_ERR;
  }

  return;
}



/*
** fshlpr_perror
**
*/    
fshlpr_perror (prefix)
     char *prefix;
{
  /*
  ** if it is one of our messages print it out
  ** otherwise, if it is an implementation message, or an
  ** unknown error, print out the code too
  */
  if (fsherr() > FSHERR_LAST)
    (void) fprintf (stderr, "%s: %s (%d)\n", prefix,
		    ERR_MSG (fsherr_msgs[fsherr() >= FSHERR_1STIMP?
			      FSHERR_IMPERR: FSHERR_UNKERR].msgno,
			     fsherr_msgs[fsherr() >= FSHERR_1STIMP?
			      FSHERR_IMPERR: FSHERR_UNKERR].msg), fsherr());
  else if (fsherr() > 0)
    (void) fprintf (stderr, "%s: %s\n", prefix,
		    ERR_MSG (fsherr_msgs[fsherr()].msgno,
			     fsherr_msgs[fsherr()].msg));

  /*
  ** could send errors to the error daemon here
  ** FSHERR_INTERNAL, FSHERR_NOMEM and those greater
  ** than FSHERR_1STIMP would be possible candidates for verbosity
  */

  return;
}

    


static jmp_buf old_context;

/*
** invalid_reference
**
** verify that the ptr is within our address space
**
** do this by setting up signal handlers and referencing the address
** returns < 0 (- signal) that failed on
**           0 ok
**           1 can't set up signal handlers
*/

bool_t
invalid_reference (addr)
     caddr_t  addr;
{
  int                rc = 0;
#ifdef PARANOID
  void               trap_sig();
  struct sigaction   obus_act,
                     osegv_act,
                     bus_act,
                     segv_act;

  bus_act.sa_handler = segv_act.sa_handler = trap_sig;
  bus_act.sa_flags   = segv_act.sa_flags   = 0;
  SIGINITSET (bus_act.sa_mask);
  SIGINITSET (segv_act.sa_mask);
  
  if (sigaction (SIGBUS, &bus_act, &obus_act))
      return 1;
  
  if (sigaction (SIGSEGV, &segv_act, &osegv_act))
      return 1;
  
  /*
  ** reference the lit'l bugger wit' xtreme prejudice 
  */

  if (!(rc = setjmp (old_context)))
  {
    register int junk = *addr;
#ifdef lint    
    junk = junk? junk: junk;
#endif    
  }

  /*
  ** restore sig pointers
  */
  if (rc)
  {
    if (sigaction (SIGSEGV, &osegv_act, NILPTR (struct sigaction)))
	return 1;
    if (sigaction (SIGBUS, &obus_act, NILPTR (struct sigaction)))
	return 1;
  }
#endif /* PARANOID */  
  return rc;
}
    
void
trap_sig (sig, not, used)
     int                  sig;
     int                  not;
     struct sigcontext   *used;
{
  (void) longjmp (old_context, -sig);
  /*NOTREACHED*/
  return;
}
