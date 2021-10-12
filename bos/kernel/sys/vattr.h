/* @(#)35	1.12.1.4  src/bos/kernel/sys/vattr.h, syslfs, bos411, 9438C411a 9/23/94 12:12:45 */
#ifndef _H_VATTR
#define _H_VATTR

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/time.h>
#include <sys/priv.h>

/*
 * Structure describing the attributes of files across all
 * the types of file systems which are supported.  This is
 * a superset of the AIX stat structure's fields.
 */
struct vattr
{
	enum vtype	va_type;	/* vnode type			*/
	mode_t	va_mode;		/* access mode			*/
	uid_t	va_uid;			/* owner uid			*/
	gid_t	va_gid;			/* owner gid			*/
	union {		       /* avoid nasty short/long align problems */
		dev_t	_va_dev;	/* id of device containing file	*/
		long	_va_fsid;	/* fs id (dev for now)		*/
	} _vattr_union;
	long	va_serialno;		/* file serial (inode) number	*/
	short	va_nlink;		/* number of links		*/
	short	va_flags;		/* Flags, see below for define  */
	long	va_llpad;		/* CSet++'s long long pad	*/
#ifdef	_LONG_LONG
	offset_t	va_size;	/* file size in bytes		*/
#else
	int	va_sizeu;
	off_t	va_size;		/* file size in bytes		*/
#endif
	long	va_blocksize;		/* preferred blocksize for io	*/
	long	va_blocks;		/* kbytes of disk held by file	*/
	struct	timestruc_t  va_atime; 	/* time of last access */
	struct	timestruc_t  va_mtime;	/* time of last data modification */
	struct	timestruc_t  va_ctime;	/* time of last status change */
	dev_t	va_rdev;		/* id of device			*/

	/* Fields added for compatability with the fullstat structure */
	long	va_nid;			/* node id			*/
	chan_t	va_chan;		/* channel of MPX device */
	char	*va_acl;		/* Access Control List */
	int	va_aclsiz;		/* size of ACL */
	int	va_gen;			/* inode generation number */
};
#define	va_dev	_vattr_union._va_dev
#define	va_fsid	_vattr_union._va_fsid

/*
 * Flag definitions
 */
#define	VA_NOTAVAIL	0x0001	/* response to attribute request on lookup or
				 * rdwr when attributes cannot be returned
				 */

/*
 * time in vattr in now two longs, but some measure of back compatibility
 * is maintained, the short times are "tim" vs "time"
 */
#define	va_atim	va_atime.tv_sec
#define	va_mtim	va_mtime.tv_sec
#define	va_ctim	va_ctime.tv_sec

/*
 * Values known to the vn_setattr
 * Must be mutually exclusive
 */
#define	V_MODE	0x01
#define	V_OWN	0x02
#define	V_UTIME	0x04
#define V_STIME	0x08


/* 
 * The following are flags for use in the timeflg (arg1) field of
 * VNOP_SETATTR call. (comment them well, this is the only place
 * they are documented.
 */

	/* 
	 * NOTE:
	 * Only flags in the range of 00100-04000 may be used,
	 * those lower than this are used in chownx.h.
	 */
#define T_SETTIME	0200	/* Used in V_UTIME to tell the vnode
				 * op that the user requested the
				 * times be set to system time.
				 */


#endif	/* _H_VATTR */
