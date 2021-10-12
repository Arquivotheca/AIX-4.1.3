static char sccsid[] = "@(#)22	1.8  src/bos/usr/ccs/lib/libc/mount.c, libcfs, bos411, 9428A410j 6/13/91 20:47:12";
/*
 * LIBCFS: C Library Filesystem Interfaces
 *
 * FUNCTIONS: mount
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <sys/vfs.h>
#include <sys/vmount.h>

#define	ROUNDUP(x)	(((x) + 3) & ~3)

/*
 * what getinfo gets us
 */
struct dev_info {
	char	logpath[PATH_MAX + 1];
	dev_t	logdev;
	int	gfstype;
	};

static void getinfo(char *, struct dev_info *);
static int mygetvfstype(char *);
static void vmountdata(struct vmount *, char *, char *, char *,
		char *, char *, int, char *);

/*
 * NAME:	mount
 *
 * FUNCTION:	Old style interface to vmount.
 *
 * NOTES:	Mount attempts to interface to vmount using the
 *		old-style mount interface.  This is guesswork at best.
 *
 * DATA STRUCTURES:	errno may be set by vmount or we may set it to ENOMEM
 *
 * RETURN VALUE DESCRIPTION:	0 on success, else -1
 */

int
mount(char *dev, char *dir, int flags)
{
	struct vmount	*vmountp;
	char		 options[sizeof("ro,rmv,nosuid,nodev")];
	unsigned int	 size;
	int		 rc;
	char		 info[sizeof(dev_t)], *args;
	int		 info_siz, args_siz;
	dev_t		 logdev;
	char		*fstype;
	struct dev_info	 dev_info;
	struct stat	 stbuf;

	/*
	 * only allow block device mounts here... (SVVS)
	 */
	if (stat(dev, &stbuf) < 0)
		return (-1);
	if ((stbuf.st_mode & S_IFMT) != S_IFBLK) {
		errno = ENOTBLK;
		return (-1);
		}

	/*
	 * figure ascii equivalent of rw flags...
	 */
	strcpy(options, (flags & MNT_READONLY) ? "ro" : "rw");

	/*
	 * other options
	 */
	if (flags & MNT_REMOVABLE)
		strcat(options, ",rmv");
	if (flags & MNT_NOSUID)
		strcat(options, ",nosuid");
	if (flags & MNT_NODEV)
		strcat(options, ",nodev");

	/*
	 * get other information about the device...
	 */
	getinfo(dev, &dev_info);

	if ((flags & MNT_READONLY) || dev_info.gfstype != MNT_JFS ||
	    dev_info.logpath[0] == NULL) {
		/*
		 * skip log stuff
		 */
		args = options;
		info_siz = -1;
		}
	else {
		/*
		 * handle log device stuff if this is a rw mount
		 * and is MNT_JFS...
		 */
		args_siz = strlen(options) + strlen(",log=") +
			   strlen(dev_info.logpath) + 1;
		if ((args = (char *) malloc(args_siz)) == NULL) {
			errno = ENOMEM;
			return (-1);
			}
		sprintf(args, "%s,log=%s", options, dev_info.logpath);
		info_siz = sizeof(info);
		memcpy((char *) info, (char *) &dev_info.logdev, info_siz);
		}

	/*
	 * size is the size of the vmount structure and of the following
	 * data items, each of which is starting on long word boundary.
	 */
	size = sizeof(struct vmount) + ROUNDUP(strlen(dev) + 1) +
 		ROUNDUP(strlen(dir) + 1) + ROUNDUP(strlen("-") + 1) +
 		ROUNDUP(strlen("-") + 1) + ROUNDUP(info_siz + 1) +
		ROUNDUP(strlen(args) + 1);
		
	/* malloc the vmount structure */
	if ((vmountp = (struct vmount *) malloc(size)) == NULL) {
		errno = ENOMEM;
		rc = -1;
		}
	else {
		/* zero out the vmount structure */
		bzero(vmountp, size);

		/* transfer info into the vmount structure */
		vmountp->vmt_revision = VMT_REVISION;
		vmountp->vmt_length = size;
		vmountp->vmt_flags = (flags | VFS_SYSV_MOUNT) & VMOUNT_MASK;
		vmountp->vmt_gfstype = dev_info.gfstype;

		vmountdata(vmountp, dev, dir, "-", "-", info, info_siz, args);

		rc = vmount(vmountp, size);

		if (args != options)
			free((void *) args);
		free((void *) vmountp);
		}

	return (rc);
}

/*
 * vmountdata (yet another...)
 *
 * - stuff the vmount structure full of thingies...
 */
static void
vmountdata(struct vmount *vmtp, char *obj, char *stub, char *host, char *hostname, char *info, int info_size, char *args)
{
        register struct data {
                                short   vmt_off;
                                short   vmt_size;
                        } *vdp;
        register int    size, offset;
 
	offset = sizeof(struct vmount);
        vdp = (struct data *) vmtp->vmt_data;
 
        vdp->vmt_off = offset;
        size = ROUNDUP(strlen(obj) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_OBJECT), obj);

	offset += size;
	vdp++;

        vdp->vmt_off = offset;
        size = ROUNDUP(strlen(stub) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_STUB), stub);
 
	offset += size;
	vdp++;
 
        vdp->vmt_off = offset;
        size = ROUNDUP(strlen(host) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_HOST), host);
 
	offset += size;
	vdp++;
 
        vdp->vmt_off = offset;
        size = ROUNDUP(strlen(hostname) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_HOSTNAME), hostname);
 
	offset += size;
	vdp++;
 
	if (info_size > 0) {
        	vdp->vmt_off = offset;
        	size = ROUNDUP(info_size + 1);
        	memcpy(vmt2dataptr(vmtp, VMT_INFO), info, size);
		}
	else 
		size = vdp->vmt_off = 0;
	vdp->vmt_size = size;
 
	offset += size;
	vdp++;
 
        vdp->vmt_off = offset;
	size = ROUNDUP(strlen (args) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_ARGS), args);
}

/*
 *	getinfo
 *
 *	- get information about `dev' and stuff it into `info'
 */
static void
getinfo(char *dev, struct dev_info *info)
{
	AFILE_t		 afile;
	ATTR_t		 stanza;
	char		*devname;
	char		*logpath;
	char		*vfsname;
	struct stat	 stbuf;

	/*
	 * invalid values...
	 */
	info->logpath[0] = NULL;
	info->logdev = ~0;
	info->gfstype = MNT_BADVFS;

	/*
	 * walk thru /etc/filesystems, looking for `dev'
	 */
	if ((afile = AFopen(FSYSname, MAXREC, MAXATR)) != NULL) {

		for (logpath = vfsname = NULL;
		     (stanza = AFnxtrec(afile)) != NULL; )
			if ((devname = AFgetatr(stanza, "dev")) != NULL &&
			     strcmp(dev, devname) == 0) {
				/*
				 * save off the log and vfs
				 */
				logpath = AFgetatr(stanza, "log");
				vfsname = AFgetatr(stanza, "vfs");
				break;
				}

		/*
		 * get the vfs type
		 */
		info->gfstype = mygetvfstype(vfsname);

		/*
		 * handle log
		 */
		if (logpath != NULL) {
			strcpy(info->logpath, logpath);
			if (stat(logpath, &stbuf) == 0)
				info->logdev = stbuf.st_rdev;
			}

		AFclose(afile);
		}
}

/*
 *	mygetvfstype
 *
 *	- walk thru /etc/vfs, looking for the vfs type of `vfsent_name'.
 *
 *	- catch default local vfstype in /etc/vfs in case we don't know or
 *	  recognize `vfsent_name'
 */
static int
mygetvfstype(char *vfsent_name)
{
	int vfstype, dft_type;
	extern struct vfs_ent  *getvfsent();
	struct vfs_ent   *vp;

	setvfsent();

	/*
	 * default - in case getvfsent doesn't find a default
	 */
	dft_type = MNT_JFS;

	/*
	 * walk thru /etc/vfs until we find `vfsent_name'
	 */
	for (vfstype = MNT_BADVFS;
	     vfstype == MNT_BADVFS && (vp = getvfsent()) != NULL; ) {
		if (vp->vfsent_flags & VFS_DFLT_LOCAL)
			dft_type = vp->vfsent_type;	/* catch default */

		if (vfsent_name != NULL &&
		    strcmp(vp->vfsent_name, vfsent_name) == 0)
			vfstype = vp->vfsent_type;
		}

	endvfsent ();

	/*
	 * use default if we didn't find it
	 */
	if (vfstype == MNT_BADVFS)
		vfstype = dft_type;

	return (vfstype);
}
