static char sccsid [] = "@(#)70 1.1  src/bos/sbin/helpers/v3fshelpers/libfs/device.c, cmdfs, bos411, 9428A410j 2/2/94 14:11:46";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fsopen, fsclose
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <fstab.h>
#include <sys/stat.h>
#include <libfs/libfs.h>

static char *
resolve_device(char *fsmount,		/* fs mount point	      */
	       char *fsdev);		/* device that mounts fsmount */

/*
 *
 *  NAME:   resolve_device(char *fsname, char *fsdev)
 *
 *  FUNCTION:
 * 	read /etc/fstab to get the device name from the given mount point
 *	Return a pointer to the device name if successful else return null.
 *
 *  RETURN VALUE:
 *      On successful completion, returns ptr to device name
 *	On failure returns NULL
 */
static char *
resolve_device(char *fsmount,		/* fs mount point	      */
	       char *fsdev)		/* device that mounts fsmount */
{
	struct fstab	*fsent;

	*fsdev = '\0';
	if (setfsent() && (fsent = getfsfile(fsmount)))
		strcpy(fsdev, fsent->fs_spec);
	endfsent();
	return *fsdev ? fsdev : (char *)NULL;
}


/*
 *
 *  NAME:  fsopen(char *fsname)
 *
 *  FUNCTION:
 *     	open the named file system.  It can be named
 *      either by its filesystem name or its device name
 *
 *  RETURN VALUE:
 *      On successful completion, returns file descriptor for fsname
 *	On failure returns -1
 *
 */
int
fsopen(char *fsname,
       int oflag)
{
	struct stat	st;
	char		devname[MAXPATHLEN];

	if (stat(fsname, &st) < 0)
		return LIBFS_NODEV;
	
	if ((st.st_mode & S_IFMT) == S_IFCHR ||
	    (st.st_mode & S_IFMT) == S_IFBLK)
		strcpy(devname, fsname);		
	else
		if (resolve_device(fsname, devname) == NULL)
			return LIBFS_NODEV;
	
	return open(devname, oflag);
}


/*
 *
 *  NAME:  fsclose (int fd)
 *
 *  FUNCTION:
 *	close the file fd and 
 *	by ltop.
 *
 *  RETURN VALUE:
 *	-1 if close fails, 0 if close succeeds
 */
int
fsclose(int fd)
{
	FragSize = FragMask = FragShift = FragPerBlk =
		FragPerBlkMask = FragPerBlkShift = 0;
	set_inovars(0, 0, 0);	
	return close(fd);
}
