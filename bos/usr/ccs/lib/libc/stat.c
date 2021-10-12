static char sccsid[] = "@(#)77	1.4  src/bos/usr/ccs/lib/libc/stat.c, libcfs, bos411, 9428A410j 1/26/94 11:33:04";

/*
 * COMPONENT_NAME: LIBCFS - C Library File System Interfaces
 *
 * FUNCTIONS: stat, fstat, lstat, fullstat, ffullstat, statvfs, fstatvfs
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/stat.h>
#include <sys/fullstat.h>
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <stdio.h>

#define MNTCTLSIZ	512	/* Initial allocation for mntctl() */
#define READSIZ		512	/* Allocation for read buffer	   */
#define COMMENT		'#'	/* Comment character in /etc/vfs   */
#define CONTROL		'%'	/* Control directive in /etc/vfs   */
/*
 * Prototypes.
 */
static int _vfsstat(struct statfs *, struct statvfs *);
int 	   statvfs(const char *, struct statvfs *);
int 	   fstatvfs(int, struct statvfs *);

/*
 * NAME:	stat()
 *
 * FUNCTION:	Stat a pathname.  This is a statx() compatibility interface.
 * 
 * PARAMETERS:	pathname	Path to stat
 *		statbuf		Pointer to stat struct.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
stat(char *pathname, struct stat *statbuf)
{
	return statx(pathname, statbuf, STATSIZE, STX_NORMAL);
}


/*
 * NAME:	lstat()
 *
 * FUNCTION:	Stat a symbolic link.  This is a statx() compatibility interface
 * 
 * PARAMETERS:	pathname	Path to stat
 *		statbuf		Pointer to stat struct.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
lstat(const char *pathname, struct stat *statbuf)
{
	return statx(pathname, statbuf, STATSIZE, STX_LINK);
}


/*
 * NAME:	fstat()
 *
 * FUNCTION:	Stat a file descriptor.  This is an fstatx() compatibility
 *		interface.
 * 
 * PARAMETERS:	fdes		File descriptor.
 *		statbuf		Pointer to stat struct.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
fstat(int fdes, struct stat *statbuf)
{
	return fstatx(fdes, statbuf, STATSIZE, STX_NORMAL);
}


/*
 * NAME:	fullstat()
 *
 * FUNCTION:	Stat a pathname.  This is a statx() compatibility interface.
 * 
 * PARAMETERS:	pathname	Path to stat
 *		cmd		Flag to indicate normal or symlink processing.
 *		statbuf		Pointer to stat struct.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
fullstat(char *pathname, int cmd, struct fullstat *statbuf)
{
	return statx(pathname, (struct stat *)statbuf, FULLSTATSIZE, cmd);
}


/*
 * NAME:	ffullstat()
 *
 * FUNCTION:	Stat a file descriptor.  This is an fstatx() compatibility
 *		interface.
 * 
 * PARAMETERS:	fdes		File descriptor.
 *		cmd		Flag to indicate normal or symlink processing.
 *		statbuf		Pointer to stat struct.
 *
 * RETURN VALUES: explicitly none, implicitly sets errno;
 */
ffullstat(int fdes, int cmd, struct fullstat *statbuf)
{
	return fstatx(fdes, (struct stat *)statbuf, FULLSTATSIZE, cmd);
}

/*
 * NAME: 	statvfs
 * 
 * FUNCTION: 	Return a statvfs structure describing the virtual filesystem
 *	     	that a file exists in.
 *
 * PARAMETERS:	path	pathname of file existing within the vfs.
 *		vfsbuf	statvfs structure to return to user.
 *
 * RETURNS:	 0	success
 *		-1	failure and set errno
 */
int
statvfs(const char *path, struct statvfs *vfsbuf)
{
	struct 	statfs fsbuf;		/* buffer returned from statfs()     */

	if (!statfs(path, &fsbuf)) 
		return(_vfsstat(&fsbuf, vfsbuf));
	
	return(-1);
}

/*
 * NAME: 	fstatvfs
 * 
 * FUNCTION: 	Return a statvfs structure describing the virtual filesystem
 *	     	that a file exists in.
 *
 * PARAMETERS:	fildes	open file descriptor of a file within the filesystem
 *		vfsbuf	statvfs structure to return to user.
 *
 * RETURNS:	 0	success
 *		-1	failure and set errno
 */
int
fstatvfs(int fildes, struct statvfs *vfsbuf) 
{
	struct 	statfs fsbuf;		/* buffer returned from statfs()     */

	if (!fstatfs(fildes, &fsbuf)) 
		return(_vfsstat(&fsbuf, vfsbuf));
	
	return(-1);
}

/*
 * NAME: 	_vfsstat	
 * 
 * FUNCTION:  	Helper routine for fstatvfs() and statvfs().  Transfers 
 *		statfs structure into a statvfs structure and fills in 
 *		flags field.	
 *
 * PARAMETERS:  fsbuf	statfs structure filled from previous call to statfs().	
 *		vfsbuf	statvfs structure to return to user 
 *
 * RETURNS:	 0	success
 *		-1	failure and set errno
 */
static int
_vfsstat(struct statfs *fsbuf, struct statvfs *vfsbuf)
{
        int	size = MNTCTLSIZ;	/* initial size of mntctl() buffer   */
        int	nmounts = 0;		/* number of mntctl() mounts         */
	int	fperpage;		/* fragments per page 		     */
	struct 	vmount *vmountp;	/* vmount linked list from mntctl()  */
	struct  vmount *vmt;		/* vmount traversal pointer          */
	FILE	*vfsfp;			/* File pointer for /etc/vfs	     */
	char	*name;			/* pointer to filesystem name        */
	char 	*type;			/* pointer to filesystem type	     */
	char	readbuf[READSIZ];	/* read buffer for /etc/vfs	     */
	
	bzero(vfsbuf, sizeof(struct statvfs));

	vfsbuf->f_bsize   = fsbuf->f_bsize;	/* preferred size */
	vfsbuf->f_frsize  = fsbuf->f_fsize;	/* fragment size  */
							
	/*
	 * statvfs() reports blocks in fragment size blocks whereas
	 * statfs() returns full block statistics.  If the full and
	 * fragment sizes are different this may represent a loss
	 * of precision.  First calculate the number of fragments
	 * per page and multiple block statistics returned from 
	 * statfs().
	 */
	fperpage = vfsbuf->f_bsize / vfsbuf->f_frsize;
	vfsbuf->f_blocks  = fsbuf->f_blocks * fperpage;
	vfsbuf->f_bfree   = fsbuf->f_bfree  * fperpage;
	vfsbuf->f_bavail  = fsbuf->f_bavail * fperpage;

	vfsbuf->f_files   = fsbuf->f_files;
	vfsbuf->f_ffree   = fsbuf->f_ffree;
	vfsbuf->f_favail  = fsbuf->f_ffree;  /* no root restrictions on nodes */
	vfsbuf->f_fsid    = fsbuf->f_fsid;
	vfsbuf->f_namemax = fsbuf->f_name_max;

       	while (nmounts == 0)
       	{
       		if ((vmountp = (struct vmount *)malloc(size)) == NULL)
			return(-1);

		/*
		 * If mntctl() returns zero then our buffer isn't
		 * big enough.  Correct size is placed in first field.
		 */ 
       		if ((nmounts = mntctl(MCTL_QUERY, size, vmountp)) == 0)
       		{
       			size = vmountp->vmt_revision;
       			free(vmountp);
       		}
		else if (nmounts == -1)
		{
			free(vmountp);
			return(-1);
		}
       	}

	for (vmt = vmountp; nmounts > 0; 
	     vmt = (struct vmount *)((caddr_t)vmt + vmt->vmt_length),
	     nmounts--)
	{
		/*
		 * If file system id matches corresponding id
		 * from statfs() then encode the appropriate
		 * flags.  If on the really odd case that statfs() passed
		 * and mntctl() doesn't find the same information, I will
		 * return without a flags entry.
		 */
		if (vmt->vmt_vfsnumber == fsbuf->f_vfsnumber)
		{
			vfsbuf->f_flag = vmt->vmt_flags & 
					(ST_RDONLY | ST_NOSUID | ST_NODEV);
			break;
		}
	}
	free(vmountp);

	/*
	 * From this point on, we're going to do the best we can
	 * with filling in the basetype.  We do this by comparing the
	 * type against the type that appears in /etc/vfs and subsequently
	 * copying the name into the vfsbuf structure.  Any errors past 
	 * this point are ignored.
	 */
	if ((vfsfp = fopen(VFSfile, "r")) == (FILE *)NULL)
		return(0);
	
	while (fgets(readbuf, (int)sizeof(readbuf), vfsfp))
	{
		if ((readbuf[0] == COMMENT) ||
		    (readbuf[0] == CONTROL) ||
		    ((name = strtok(readbuf, " \t\n")) == (char *)NULL) ||
		    ((type = strtok((char *)NULL, " \t\n")) == (char *)NULL))
			continue;

		if (atoi(type) == fsbuf->f_vfstype)
		{
			strncpy(vfsbuf->f_basetype, name, FSTYPSIZ);
			vfsbuf->f_basetype[FSTYPSIZ-1] = (char)NULL;
			break;
		}
	}
	fclose(vfsfp);
	return(0);
}
