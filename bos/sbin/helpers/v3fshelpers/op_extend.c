static char sccsid[] = "@(#)04  1.7.2.12  src/bos/sbin/helpers/v3fshelpers/op_extend.c, cmdfs, bos411, 9428A410j 5/31/94 11:49:56";
/*
 * COMPONENT_NAME: CMDFS - filesystem commands
 *
 * FUNCTIONS: op_extend
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	op_extend
 *
 *	- Aix3 filesystem extender.
 *
 */
#include <stdio.h>
#include <fshelp.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/vfs.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include "jfs/filsys.h"
#include "sys/vmdisk.h"
#include "jfs/fsdefs.h"
#include "jfs/ino.h"
#include "jfs/inode.h"
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vmount.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/fscntl.h>
#include <sys/sysmacros.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <libfs/libfs.h>
#include <libfs/fslimits.h>
#include "op_extend_msg.h"
#include "fsop.h"


/*
 * somewhere there's a Debug macro.  we wanta use a Debug global
 * instead...
 */
#ifdef Debug
# undef Debug
#endif

/*
 * NLS/ILS/msg junk
 */
#define MSGSTR(N,S)	catgets(catd,MS_OP_EXTEND,N,S)
static nl_catd catd;

/*
 * arguments passed in through opflags (and other globals)
 */
static bool_t	Increment = False;
static bool_t	Mounted = False;
static bool_t	Readonly = False;
static bool_t	Query = False;
static long	Size;
static char	Device[PATH_MAX];
static char	Stub[PATH_MAX];
static int	Vfsnumber;
static bool_t	Debug;
static bool_t	Partition_size = False;
static long	Partition_len = False;
static bool_t	Bytes = False;
static bool_t	Ublocks = False;
static bool_t	Fsblocks = False;
static bool_t	Kblocks = False;
static bool_t	Meg = False;
static bool_t	Force = False;
static char	*Program = "v3fshelper";

/*
 * local functions
 */
int		op_extend (int, char*);
static int	checkargs (char*);
static int	do_query (int, char*);
static int	get_size (int);
static off_t	get_newsize (int, size_t, size_t, struct superblock *);
static int	do_extend (int);
static int	do_mounted (int);
int		do_it_ourselves (int);
static int	mountit (void);
static int	getstub (void);
void		run_cmd (char*, ...);
static void	umountit (void);
static int	getvfsnumber (char*);
static	int	read_super (int, struct superblock*);

#define	PERROR(file)	fprintf(stderr, "%s: ", Program), perror(file)

#define	DPRINTF if (Debug) fprintf

#define	QUERY_BUF_LEN	512
#define	RC_BUF_LEN	16

#define FSCNTL_ERROR 99
#define FOURMEG (1 << 22)
#define ONEGIG (1 << 30)
#define INO2DEVBLK(x) (BYTE2DEVBLK(INO2BYTE((x))))


/*
 * op_extend
 *
 * extends a v3 filesystem
 *
 * returns FSHERR_GOOD.  the actual exit code is written out to PipeFd
 */
int
op_extend (int devfd,
	   char *opflags)
{
	register int    rc;
	char rc_buf[RC_BUF_LEN];
	char query_buf [QUERY_BUF_LEN];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_OP_EXTEND, NL_CAT_LOCALE);


	*query_buf = '\0';

	Debug = debug(FSHBUG_SUBDATA);
	DPRINTF(stderr,	"%s: opflags=\"%s\"\n", Program, opflags);
	/*
	 * process option flags
	 */
	if (checkargs (opflags) < 0)
		RETURN ("op_make/get_args", FSHERR_SYNTAX);
	/*
	 * query only?
	 */
	if (Query)
		rc = do_query(devfd, query_buf);

	/*
	 * compute size, then extend!
	 */
	else if ((rc = get_size(devfd)) == 0)
		rc = do_extend(devfd);

	/*
	 * Write 1 maybe 2 strings to the pipe;
	 * the first is always the exit code. The second
	 * is the query data if a query operation was performed
	 */
	sprintf(rc_buf, "%d", rc);
	write (PipeFd, rc_buf, strlen(rc_buf) + 1);

	if (query_buf != '\0' && Query)
		write (PipeFd, query_buf, strlen(query_buf) + 1);

	return (FSHERR_GOOD);
}

/*
 *	checkargs
 *
 *	checkargs processes the options sent from the front end
 */
static int
checkargs (char *opflags)
{
	static struct arg Args[] = {
		"device",		(caddr_t) Device,	STR_T,
		"stub",			(caddr_t) Stub,		STR_T,
		"vfsnumber",		(caddr_t) &Vfsnumber,	INT_T,
		"size",			(caddr_t) &Size,	INT_T,
		"increment",		(caddr_t) &Increment,	BOOL_T,
		"mounted",		(caddr_t) &Mounted,	BOOL_T,
		"readonly",		(caddr_t) &Readonly,	BOOL_T,
		"query",		(caddr_t) &Query,	BOOL_T,
		"partition_size",	(caddr_t) &Partition_size, BOOL_T,
		"bytes",		(caddr_t) &Bytes,	BOOL_T,
		"ublocks",		(caddr_t) &Ublocks,	BOOL_T,
		"fsblocks",		(caddr_t) &Fsblocks,	BOOL_T,
		"kblocks",		(caddr_t) &Kblocks,	BOOL_T,
		"meg",			(caddr_t) &Meg,		BOOL_T,
		"force",		(caddr_t) &Force,	BOOL_T,
		NILPTR (char),		NIL(caddr_t),		NIL_T,
		};

	if (get_args (Args, opflags) != FSHERR_GOOD)
		return (-1);

	return (0);
}

/*
 *	do_query
 *
 *	- perform the query operation.
 */
static int
do_query (int fd,
	  char *buf)
{
	int		bpi;
	char 		*compress;
	struct devinfo  devinfo;
	struct superblock  super;

	/*
	 * Query the partition for its size
	 */
	if (ioctl(fd, IOCINFO, &devinfo) < 0)
	{
		fprintf(stderr, "%s: %s: ", Program, Device);
		perror("ioctl(IOCINFO)");
		fflush (stderr);
		return (1);
	}
	
	if (read_super(fd, &super))
		return (1);

	bpi = (super.s_fragsize * super.s_agsize) / super.s_iagsize;
	compress = (super.s_compress) ? "LZ" : "no";

	sprintf (buf, "%u,%u,%d,%d,%s", devinfo.un.dk.numblks, super.s_fsize,
		 super.s_fragsize, bpi, compress);

	return (0);
}

/*
 *	get_size
 *
 *	- take the size and args from the frontend, and compute our
 *	  target size...
 */

static int
get_size (int fd)
{
	struct devinfo devinfo;

	/*
	 * query the partition for its size
	 */
	if (ioctl(fd, IOCINFO, &devinfo) < 0)
	{
		fprintf(stderr, "%s: %s: ", Program, Device);
		perror("ioctl(IOCINFO)");
		fflush (stderr);
		return (-1);
	}

	/*
	 * remember devinfo.un.dk.numblks is in UBSIZE blocks.
	 */
	Partition_len = devinfo.un.dk.numblks;

	/*
	 * if no size given, extend out to the partition size...
	 */
	if (Partition_size)
		Size = Partition_len;

	else if (Bytes)
		Size= (unsigned int)Size / UBSIZE;

	else if (Ublocks)
		;		/* No conversion for UBSIZE */

	else if (Fsblocks)
		Size *= 8;

	else if (Kblocks)
		Size *= 2;

	else if (Meg)
		Size *= 2 * 1024;

	return (0);
}

/*
 *	get_newsize
 *
 *	- once we have the current size of the filesystem, do the
 *	  increment if need be
 *	- also check final size against partition size
 *
 */
static off_t
get_newsize (
int			fd,
size_t			cursize,
size_t			size,
struct superblock *	sb)
{
	int			nbpi;
	size_t			maxsz;

	if (Increment)
		size += cursize;

	if (! Force && size > Partition_len)
	{
		fprintf(stderr,	MSGSTR(NOTBIG,
		"%s: logical volume  %s is not large enough\n"),
			Program, Device);
		fflush (stderr);
		return (0);
	}

	if (read_super (fd, sb))
		return (0);

	nbpi	= (sb->s_fragsize * sb->s_agsize) / sb->s_iagsize;

	maxsz = FS_NBPI_LIM(nbpi);
	if ((size_t) size > maxsz)
	{
		fprintf(stderr,	MSGSTR(TOOBIG_NBPI,
        "%s: A file system with nbpi = %d cannot exceed %u 512-byte blocks\n"),
			Program, nbpi, maxsz);
		fflush (stderr);
		return (0);
	}

	maxsz = FS_ADDR_LIM(sb->s_fragsize);
	if ((size_t) size > maxsz)
	{
		fprintf(stderr,	MSGSTR(TOOBIG_FRAG,
        "%s: A file system with frag = %d cannot exceed %u 512-byte blocks\n"),
			 Program, sb->s_fragsize,  maxsz);
		fflush (stderr);
		return (0);
	}

	return (size);
}

/*
 *	do_extend
 *
 *	- do the work.  basically call the right function
 */
static int
do_extend(fd)
int fd;
{
	int rc;
	int save_errno;

	DPRINTF (stderr,
		 "%s: do_extend (%s, %ld)...\n", Program, Device, Size);

	if (Mounted)
		rc = do_mounted(fd);
	else
		rc = do_it_ourselves (fd);

	if (rc == FSCNTL_ERROR)
	{
		save_errno = errno;
		PERROR(Device);
		if (save_errno == ENOSPC)
		{
			fprintf(stderr,	MSGSTR(EXTFAIL,
			"%s: extend failed. Free some space and try again.\n"),
				Program);
			fflush (stderr);
		}
	}
	return rc ? 1 : 0;
}

/*
 *	do_mounted ()
 *
 *	- do the extend for a mounted filesystem.  just call fscntl()!
 */
static int
do_mounted (int		fd)
{
	int i;
	struct statfs  sf;
	size_t size = Size;
	size_t cursize, newsize, extsize;
	size_t extblks, agblks, remblks, nags;
	size_t bsize = PAGESIZE / UBSIZE;	/* # of ubsize per pagesize */
	size_t pagesize = DBPERPAGE * bsize;	/* # of ubsize on a map page */
	struct superblock sb;

	DPRINTF(stderr,	"%s: do_mounted()\n", Program);

	/*
	 * get current size
	 */
	if( statfs (Stub, &sf) < 0 )
	{
		PERROR(Stub);
		return (1);
	}
	cursize = (size_t) sf.f_bsize / UBSIZE * sf.f_blocks;

	/*
	 * compute new size
	 */
	if ((newsize = get_newsize (fd, cursize, size, &sb)) == 0)
		return (1);

	/*
	 * First try to extend w/extra blocks at end of maps.
	 * If that fails, then extend by 4 meg of disk.  If the
	 * fs currently has a partial partial ag (extra blocks but
	 * not enough for an ag's inodes), only extend enough so we
	 * have 4 meg extra blocks (not current slop + 4 meg)
	 */
	errno = 0;
	if (fscntl(Vfsnumber, FS_EXTENDFS_EXTRA, (caddr_t)newsize,
		   sizeof(newsize)) == 0)
		return 0;
	if (errno != ENOSPC)
		return FSCNTL_ERROR;
	errno = 0;
	/*
	 * Ok, first attempt failed.  Now try to extend by 4 meg (w/o 
	 * allocating the extra pages).  By extending by 4 meg we will
	 * never grow either disk or inode map by more than 1 bitmap 
	 * block each.  If the fs currently has a partial partial ag
	 * (fs has extra blocks but not enough for an ag's inodes), only
	 * extend to current number of ag's + 4 meg.  (don't include slop)
	 */
	agblks = FRAG2DEVBLK(sb.s_agsize);
	nags = cursize / agblks;
	if (cursize % agblks >= INO2DEVBLK(sb.s_iagsize))
		nags++;
	remblks = cursize - nags * agblks;
	extblks = MIN(newsize - cursize - 8, BYTE2DEVBLK(FOURMEG));
	extsize = cursize + extblks - ((remblks > 0) ? remblks : 0);
	if (fscntl(Vfsnumber, FS_EXTENDFS, (caddr_t)extsize, 
		   sizeof(extsize)) < 0)
		return FSCNTL_ERROR;
	/*
	 * We just got 4 meg.  If no allocation activity is going on
	 * in this fs, the 4 meg guarantees us that we can grow this
	 * fs by 1 gig (with plenty of room to spare). 
	 */
	cursize = extsize;
	extblks = MIN(newsize - cursize, BYTE2DEVBLK(ONEGIG));
	extsize = cursize + extblks;
	if (fscntl(Vfsnumber, FS_EXTENDFS_EXTRA, (caddr_t)extsize, 
		   sizeof(extsize)) < 0)
		return FSCNTL_ERROR;

	/*
	 * If this extend was more than 1 gig then we are not through
	 * yet.  One more extend will finish us off.
	 */
	cursize = extsize;
	if (cursize < newsize &&
	    fscntl(Vfsnumber, FS_EXTENDFS_EXTRA, (caddr_t)newsize,
		   sizeof(newsize)) < 0)
			return FSCNTL_ERROR;
	return 0;
}

/*
 *	do_it_ourselves
 *
 */
static int
do_it_ourselves(fd)
int fd;
{
	int rc;

	if ((rc = mountit()) == 0)
	{
		rc = do_mounted(fd);
		umountit();
	}
	return (rc);
}
/*
 *	mountit
 *
 *	- attempt to mount the filesystem in question
 */
static int
mountit()
{
	int		 rc = 0;
	char		*fstype;
	/*
	 * get our mountpoint (fills in Stub)
	 */
	if (rc = getstub())
	{
		fprintf(stderr,	MSGSTR(STANZA,
					"%s: a %s stanza must exist for %s.\n"),
			Program, FSYSname, Device);
		fflush (stderr);
	}

	else
	{
		/*
		 * attempt the mount
		 */

		DPRINTF(stderr, "attempting mount %s over %s\n", Device, Stub);
		run_cmd("/usr/sbin/mount %s %s", Device, Stub);
		/*
		 * get vfsnumber of newly mounted filesystem
		 */
		if ((Vfsnumber = getvfsnumber(Device)) < 0)
			rc = 1;
	}
	return (rc);
}

static int
getstub()
{
	AFILE_t		 afile;
	ATTR_t		 stanza;
	char		*devname;

	Stub[0] = NULL;

	/*
	 * walk thru /etc/filesystems, looking for `Device'
	 */
	if ((afile = AFopen(FSYSname, MAXREC, MAXATR)) != NULL)
	{

		while ((stanza = AFnxtrec(afile)) != NULL && Stub[0] == NULL)
		{
			if ((devname = AFgetatr(stanza, "dev")) != NULL &&
			     strcmp(Device, devname) == 0)
				(void) strcpy(Stub, stanza->AT_value);
		}
		AFclose(afile);
	}
	return (Stub[0] == NULL ? -1 : 0);
}

/*
 *	run_cmd
 *
 *	- run a command, check return code, exit on bad return code
 */
void
run_cmd (char *format, ...)
{
	int rv;
	va_list args;
	char cmd[BUFSIZ];


	/*
	 * handle variable arguments...
	 */
	va_start(args, format);
	vsprintf(cmd, format, args);
	va_end(args);

	DPRINTF("run_cmd: \"%s\"\n", cmd);


	if (rv = system(cmd))
	{
		/*
		 *fork or exec failure
		 */
		if (rv == -1 || rv == 127)
		{
			fprintf(stderr, "%s", cmd);
			perror("");
		}
		else
		{
			DPRINTF("\texiting because of code %d", rv);
			/*
			 * feeble attempt at deciphering status into
			 * something usable...
			 */
			if (WIFEXITED(rv))
				rv = WEXITSTATUS(rv);
			else if (WIFSIGNALED(rv))
				rv = WTERMSIG(rv);
			else
				rv = 1;

			DPRINTF(" (exit %d)\n", rv);
		}
		fflush (stderr);
		exit(rv);
	}
}

/*
 *	umountit
 *
 *	- unmount the filesystem in question
 */
static void
umountit()
{
	if (umount(Device) < 0)
	{
		fprintf(stderr, "%s: umount(%s)", Program, Device);
		perror("");
		fflush (stderr);
	}
}

/*
 *	getvfsnumber(dev)
 *
 *	- returns vfsnumber of the vfs mounted over 'device'
 *	- error (-1) if an error occurs
 */

static int
getvfsnumber (char  *dev)
{
	int count, ct, mntsiz;
	struct vmount *vmt;
	char *mnttab;

	mnttab = (char *) &count;
	mntsiz = sizeof(count);

	/*
	 * loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0)
	{
		/*
		 * error?
		 */
		if (ct < 0)
		{
			PERROR("mntctl");
			return (-1);
		}
		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ?
			(char *) malloc(mntsiz) :
				(char *) realloc(mnttab, mntsiz);

		if (mnttab == NULL)
		{
			fprintf(stderr,	MSGSTR(MALLOC, "%s: malloc failed.\n"),
						Program);
			return (-1);
			fflush (stderr);
		}
	}
	/*
	 * walk thru the mount table, see if this device is mounted
	 */
	for (vmt = (struct vmount *) mnttab; ct > 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
	{
		if (strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) == 0)
			return (vmt->vmt_vfsnumber);
	}
	return (-1);
}

/*
 * Read the superblock to get all filesystem information
 */

static int
read_super (int			fd,
	    struct superblock	*super)
{
	int	rc = 1;
 
	switch (get_super(fd, super))
	{
		case LIBFS_SUCCESS:
		rc = 0;
		break;

		case LIBFS_BADMAGIC:
		fprintf(stderr,	MSGSTR(NOTJFS,
	"%s: %s is not recognized as a JFS filesystem.\n"), Program, Device);
		break;

		case LIBFS_BADVERSION:
		fprintf(stderr,	MSGSTR(NOTVERSION,
		"%s: %s is not a supported JFS filesystem version.\n"),
			Program, Device);
		break;

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr,	MSGSTR(CORRUPTJFS,
		"%s: %s contains a corrupt JFS filesystem superblock.\n"),
			Program, Device);
		break;

		default:
		fprintf(stderr,	MSGSTR(NOREADSUPER,
				"%s: Could not read the superblock on %s.\n"),
			Program, Device);
		break;
	}
	if (rc)
		fflush (stderr);
	return (rc);
}
