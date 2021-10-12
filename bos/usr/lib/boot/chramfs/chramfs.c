static char sccsid[] = "@(#)30 1.4 src/bos/usr/lib/boot/chramfs/chramfs.c, bosboot, bos411, 9428A410j 94/02/11 10:50:12";
/*
 * COMPONENT_NAME: (BOSBOOT) commands that deal with the Boot file system
 *
 * FUNCTIONS: chramfs
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
/*
 * NAME:	chramfs.c
 *
 * FUNCTION:	This low level command is used to extend the
 *		RAM file system
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The chramfs program is a command that can be executed from
 *      a shell environment. It accepts one input parameter,
 *	which is either the -t switch or the number of 512 byte blocks
 *	by which to extend the RAM file system. If the -t switch is
 *	specified, the chramfs command determines how much to extend
 *	the RAM file system using an algorithm based on the total
 *	system memory size.
 *
 *	Assumptions: The RAM file system is mounted.
 *		     The chramfs command is only valid for the file
 *		        system mounted on the boot RAM disk. If the
 *			underlying device is not the boot RAM disk, the
 *			command will abort with an error.
 *		     The chramfs command is issued with root privileges.
 *		     The system is either being booted or is in
 *			low-level maintenance mode, multi-user mode has
 *			not been started. This means there is only one 
 *			copy of chramfs running at one time.
 *
 *	Logic:  Open RAM disk
 *		If -t option
 *			Determine size increment based on
 *			 total system memory size
 *		Else get size increment parameter
 *		Round size increment up to page boundary
 *		Get vfs number of RAM file system
 *		Do extend:
 *			Get current size of RAM file system
 *			Compute new RAM file system size
 *			  and extend RAM disk if neccessary
 *			Extend RAM file system to page boundary
 *			Extend RAM file system to next page boundary
 *			Extend RAM file system to final size
 *		Close RAM disk
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <jfs/filsys.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vmount.h>
#include <sys/ioctl.h>
#include <sys/fscntl.h>
#include <sys/vmdisk.h>
#include <sys/sysmacros.h>
#include <Fs.h>
#include <vmdefs.h>
#include <cf.h>
#include <ramdd.h>

#define V3_FS_MAXSIZE 4194304L  /* Filesystem max size=4194304 .5k blocks */

/*
 * global variables
 */
char Device[PATH_MAX] = "/dev/ram0";
char Stub[PATH_MAX]="/";
int Vfsnumber;
int Debug = 0;    /* set this to 1 for DEBUG, 0 for NO DEBUG*/
char Program[]="chramfs";
int devfd;

/* 
 * local functions
 */
long get_rd_size();
int do_mntextend();
int getvfsnumber();
off_t get_new_fs_size();
int extend();
long get_sys_mem_size();

#define	DPRINTF if (Debug) fprintf 
#define MIN_SYS_MEM	8	/* Do not extend RAM FS if system has this  */
				/*  amount of memory (in megabytes) or less */
#define ONE_MEGABYTE	(1024 * 1024)
#define MAX_RAMFS_SIZE	32 * (ONE_MEGABYTE / UBSIZE)

main( argc, argv)
int argc;
char *argv[];
{
	off_t size_inc;
	int rc;
	char *arg;
	long tot_sys_mem, megabytes;
	long curr_ramfs_size, new_ramfs_size;
	struct statfs sf;

	if( argc != 2 ) {
		printf("Usage: %s increment-in-512-byte-blocks\n", Program);
		exit (1);
	}

	devfd = open(Device,O_RDWR);
	if( devfd < 0 ) {
		perror(Device);
		exit(1);
	}

	arg = argv[1];
	if ((arg[0] == '-') && (arg[1] == 't')) {
	/*
	 * -t option says we should determine the size increment based on
	 * the total system memory size
	 */
		/* get total system memory size in pages */
		tot_sys_mem = get_sys_mem_size(devfd);
		if (tot_sys_mem == 0)
			exit(1);
		megabytes = tot_sys_mem / (ONE_MEGABYTE / PAGESIZE);
		if (megabytes <= MIN_SYS_MEM)
			exit(0);
		/* Use half of system memory for RAM file system */
		new_ramfs_size = tot_sys_mem / 2;
		/* convert to 512 byte blocks */
		new_ramfs_size = new_ramfs_size * (PAGESIZE / UBSIZE);
		new_ramfs_size = MIN(new_ramfs_size, MAX_RAMFS_SIZE);
		/*
		 * get current ram fs size
		 */
		if( statfs(Stub, &sf) < 0 )
		{
			perror(Stub);
			exit(1);
		}
		curr_ramfs_size = (sf.f_bsize * sf.f_blocks) / UBSIZE;

		size_inc = new_ramfs_size - curr_ramfs_size;
		if (size_inc <= 0)
			exit(0);
	}
	else
		size_inc = (off_t) atol(argv[1]);

	/* Round size increment up to page boundary */
	size_inc = ((size_inc * UBSIZE + PSIZE - 1 )/PSIZE)*PSIZE/UBSIZE; 
	DPRINTF (stderr, " Size to extend=%ld Blocks.\n", size_inc);

	/* Get vfs number of the ram file system */ 
	if ((Vfsnumber = getvfsnumber(Device)) < 0) {
		fprintf(stderr,
			"%s: failed to get vfs number of RAM file system.\n",
			Program);
		exit(1);
	}	
	DPRINTF(stderr,"Vfsnumber=%d\n", Vfsnumber);

	/* do the extend */
	rc = do_mntextend(size_inc);

	close(devfd);

	if (rc < 0)
		exit(1);	/* error */

	exit(0);
}

/*
 *	getvfsnumber(dev)
 *
 *	Returns vfsnumber of the vfs mounted over 'device'
 *	  or -1 if an error occurs
 */

int
getvfsnumber(dev)
char	*dev;
{
	int count, ct, mntsiz;
	struct vmount *vmt;
	char *mnttab;

	mnttab = (char *) &count;
	mntsiz = sizeof(count);

	/*
	 * loop till we have enough memory to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0) {
		/*
		 * error?
		 */
		if (ct < 0) {
			perror("mntctl");
			return (-1);
			}

		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ? (char *) malloc(mntsiz) :
			(char *) realloc(mnttab, mntsiz);
		if (mnttab == NULL) {
			fprintf(stderr,"%s: malloc failed\n", Program);
			return (-1);
			}
		}

	/*
	 * walk thru the mount table, see if this device is mounted
	 */
	for (vmt = (struct vmount *) mnttab; ct > 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
		if (strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) == 0)
			return (vmt->vmt_vfsnumber);

	return (-1);
}

/*
 *	get_rd_size
 *
 *	Returns the size of the Ram disk in UBSIZE blocks
 *        or -1 if an error occurs.
 */
long 
get_rd_size(fd)
int fd;
{
	struct devinfo devinfo;
	long ram_size;

	DPRINTF(stderr,"getsize\n");
	/*
	 * query the partition for its size
	 */
	if (ioctl(fd, IOCINFO, &devinfo) < 0) {
		DPRINTF(stderr,"%s: %s: ", Program, Device);
		perror("ioctl(IOCINFO)");
		return (-1);
		}
	/*
	 * remember devinfo.un.dk.numblks is in UBSIZE blocks.
	 */
	ram_size = devinfo.un.dk.numblks;
	DPRINTF(stderr," Disk size=%ld\n", ram_size);

	return (ram_size);
}

/*
 *	do_mntextend()
 *
 *	Do the extend for a mounted filesystem.
 *
 *	Returns 0 if successful, -1 if an error occurs.
 */
int
do_mntextend(size_inc)
off_t size_inc;
{
	int i;
	struct statfs sf;
	off_t cursize, newsize, nextpage;
	off_t bsize = PAGESIZE / UBSIZE;	/* # of ubsize per pagesize */
	off_t pagesize = DBPERPAGE * bsize;	/* # of ubsize on a map page */

	DPRINTF(stderr,"%s: do_mounted()\n", Program);

	/*
	 * get current size
	 */
	if( statfs(Stub, &sf) < 0 ) 
	{
		perror(Stub);
		return (-1);
	}

	cursize = (unsigned int)(sf.f_bsize * sf.f_blocks) / UBSIZE;

	DPRINTF(stderr,"current size %ld: \n", cursize);
	/*
	 * compute new size
	 */
	if( (newsize = get_new_fs_size(cursize, size_inc)) == 0 )
		return (-1);

	DPRINTF(stderr,"new size %ld: 512 blocks\n",newsize);

	/*
	 * Here's the tricky part.  we want to do 2 page boundary
	 * extends if we're trying to do a pretty large extend.  1st
	 * extend should give us enough for the second (assuming there's
	 * only 2 free pages left, the first will free those (1 for the
	 * diskmap, 1 for the inodemap), then the second will give us
	 * a whole diskmap page to work with for the final extend)
	 */

	/*
	 * compute next page boundary
	 */
	nextpage = cursize + pagesize - (cursize % pagesize);
	/*
	 * do the '2 step' procedure for extending...
	 */
	for (i = 0; i < 2; i++, nextpage += pagesize)
		if (cursize < nextpage && nextpage < newsize)
			if (extend(nextpage))
				return (-1);

	/*
	 * do final extend
	 */
	return (extend(newsize));
}

/*
 *	get_new_fs_size
 *
 *	Get new RAM file system size.
 *	Once we have the new size of the filesystem, do the
 *	  extend on the ram disk if need be.
 *
 *	Returns new RAM file system size, or 0 if an error occurs.
 *
 */
off_t
get_new_fs_size(cursize, size_inc)
off_t	cursize;
off_t	size_inc;
{
	long ram_size;

	cursize += size_inc;

	ram_size = get_rd_size(devfd);

	if( cursize > ram_size )
	{
		if( (unsigned int)cursize > V3_FS_MAXSIZE ) {
			fprintf(stderr, 
			"%s: file system cannot exceed %ld  %d-byte blocks\n",
				Program, V3_FS_MAXSIZE, UBSIZE);
			return(0);
		}               

		if (ubtofb(cursize) > MAXMAPSIZE) {
			fprintf(stderr,"%s: new size bigger than MAXMAPSIZE\n",
				Program);
			return(0);
		}

    		if( !(ioctl(devfd, IOCCONFIG, (int)size_inc)) ) {
        		DPRINTF(stderr," Ram disk extended by  %d blocks\n",
				size_inc); 
    		}
    		else {
			fprintf(stderr,"%s: failed to extend ramdisk.\n",
				Program);
			perror(Program);
        		return(0);	
		}
	}

	return (cursize);
}

/*
 *	extend file system
 *
 *	Returns 0 if successful, -1 if an error occurs.
 */
int
extend(newsize)
off_t newsize;
{
	DPRINTF(stderr,"%s: calling fscntl(%d, FS_EXTENDFS, %ld)\n",
			Program, Vfsnumber, newsize);
	/*
	 * do it
	 */
	if (fscntl(Vfsnumber,FS_EXTENDFS,(caddr_t)newsize,sizeof(newsize))<0){
		perror(Device);
		return (-1);
		}
	return (0);
}

/*
 *	get system memory size
 *
 *	Returns size of system memory in pages, or 0 if an error occurs
 */
long
get_sys_mem_size(fd)
int fd;
{
	long	memsize;		/* memory size */

	if (ioctl(fd, RIOCSYSMEM, &memsize) < 0) {
		DPRINTF(stderr,"%s: %s: ", Program, Device);
		perror("ioctl(RIOCSYSMEM)");
		return (0);
		}
	return(memsize);
}
