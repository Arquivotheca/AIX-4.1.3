/* @(#)26	1.12  src/bos/kernel/sys/fp_io.h, syslfs, bos411, 9428A410j 12/16/93 14:22:00 */
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * Copyright International Business Machines Corp. 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

#ifndef	_H_FP_IO
#define	_H_FP_IO

#include <sys/file.h>
#include <sys/uio.h>

/* fpflag values for fp_open() */
#define FP_USR	 	0x00000000	/* file name in user space */
#define FP_SYS	 	0x00000001	/* file name in kernel space */

#ifdef _KERNEL
#ifndef _NO_PROTO
/* These functions are exported for use by kernel extensions and are	*/
/* available for use by the kernel also.  Eventually there should be	*/
/* "fp_" functions for all services provided by the logical file system	*/
/* to the kernel and kernel extensions.  Currently other functions are	*/
/* provided for individual components that need them.  The caller of	*/
/* these functions does not need to have the kernel lock.  All of these	*/
/* functions return a errno style error code.  The caller is		*/
/* responsible for setting u.u_error if the return code is non-zero.	*/

/* open file by name, create file if (oflags | O_CREATE), return a	*/
/* file pointer								*/
extern
int
fp_open (
	char *		path,		/* path to file to open		*/
	int		oflags,		/* open() style flags		*/
	int		mode,		/* perms for file if O_CREATE	*/
	int		ext,		/* device driver specific data	*/
	int		fpflag,		/* fp flags:  FP_USR or FP_SYS	*/
	struct file **	fpp);		/* file pointer return address	*/

/* open file by name for execution, return a file pointer and last	*/
/* component of file name; used by exec code				*/
extern
int
fp_xopen (
	char *		fname,		/* path to file to open		*/
	int		fpflag,		/* fp flags:  FP_USR or FP_SYS	*/
	int		oflag,		/* open() style flags		*/
	caddr_t		basename,	/* last component of pathname	*/
	uint		basenamelen,	/* basename buffer length	*/
	struct file **	fpp);		/* file pointer return address	*/

/* open a device for kernel use only					*/
extern
int
fp_opendev (
	dev_t		devno,		/* device number of dev to open	*/
	int		flags,		/* device flags:  DREAD, DWRITE	*/
	caddr_t		cname,		/* channel name string		*/
	int		ext,		/* device driver specific data	*/
	struct file **	fpp);		/* file pointer return address	*/

/* return the file pointer associated with a file descriptor		*/
extern
int
fp_getf (
	int		fd,		/* file descriptor of file ptr	*/
	struct file **	fp);		/* file pointer return address	*/

/* return the file pointer associated with a file descriptor; this	*/
/* is intended for lfs use only						*/
extern
int
getf (
	int		fd,		/* file descriptor of file ptr	*/
	struct file **	fpp);		/* file pointer return address	*/

/* return the first available file descriptor greater than or equal to	*/
/* startfd								*/
extern
int
ufdalloc (
	int		startfd,	/* first file desc to check	*/
	int *		fdp);		/* file descriptor return addr	*/

/* free a file descriptor						*/
extern
void
ufdfree (
	int		fd);		/* file descriptor to free	*/

/************************************************************************/
/* These functions are for use by logical file system functions only.	*/
/* The caller must be holding the kernel lock before calling these	*/
/* functions.								*/

/* allocate and return a file pointer of the appropriate type for a	*/
/* given vnode								*/
extern
int
fpalloc (
	struct vnode *	vp,		/* vnode pointer to attach	*/
	int		flag,		/* file flag value to use	*/
	int		type,		/* file pointer type to use	*/
	struct fileops *ops,		/* file operations to attach	*/
	struct file **	fpp);		/* file pointer return address	*/

extern
int
fp_ufalloc (	struct file *	fp);

extern
void
fp_hold (
		struct file *	fp);

extern
void
ffree (		struct file *	fp);

extern
int
fp_read (	struct file *	fp,
		char *		buf,
		int		nbytes,
		int		ext,
		int		seg,
		int *		countp);
extern
int
fp_readv (	struct file *	fp,
		struct iovec *	iov,
		int		iovcnt,
		int		ext,
		int		seg,
		int *		countp);
extern
int
fp_write (	struct file *	fp,
		char *		buf,
		int		nbytes,
		int		ext,
		int		seg,
		int *		countp);
extern
int
fp_writev (	struct file *	fp,
		struct iovec *	iov,
		int		iovcnt,
		int		ext,
		int		seg,
		int *		countp);
extern
int
fp_rwuio (	struct file *	fp,
		enum uio_rw	rw,
		struct uio *	uiop,
		int		ext);
extern
int
fp_lseek (	struct file *	fp,
		off_t		offset,
		int		whence);

#ifdef _LONG_LONG
extern
int
fp_llseek (	struct file *	fp,
		offset_t	offset,
		int		whence);
#endif

extern
int
fp_ioctl (	struct file *	fp,
		unsigned int	cmd,
		caddr_t		arg, ... );
extern
int
fp_close (	struct file *	fp);

extern
int
fp_fstat(	struct file	*fp,
		struct stat	*statp,
		int		 len,
		int		 seg);

extern
int
fp_access(	struct file	*fp,
		int		 perm);

/* These functions are only for use within the kernel.  Their use	*/
/* should be minimized.  They will probably be replaced at a future	*/
/* date.								*/

/* return the file pointer associated with a file descriptor;  avoids	*/
/* security checking;  THIS MUST ONLY BE CALLED BY close() AND exit()	*/
extern
int
getfx (		int		fd,	/* file descriptor of file ptr	*/
		struct file **	fpp);	/* file pointer return address	*/

/* return the file pointer associated with a file descriptor after	*/
/* validating the type							*/
extern
int
getft (		int		fd,	/* file descriptor of file ptr	*/
		struct file **	fpp,	/* file pointer return address	*/
		int		want);	/* file ptr type to validate	*/

#else /* _NO_PROTO */

extern int	fp_open();
extern int	fp_xopen();
extern int	fp_opendev();
extern int	fp_getf();
extern int	getf();
extern int	ufdalloc();
extern void	ufdfree();
extern int	fpalloc();
extern int	fp_ufalloc();
extern void	fp_hold();
extern void	ffree();
extern int	fp_read();
extern int	fp_readv();
extern int	fp_write();
extern int	fp_writev();
extern int	fp_rwuio();
extern int	fp_lseek();
extern int	fp_llseek();
extern int	fp_ioctl();
extern int	fp_close();
extern int	fp_fstat();
extern int	fp_access();
extern int	getfx();
extern int	getft();

#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif	/* _H_FP_IO */
