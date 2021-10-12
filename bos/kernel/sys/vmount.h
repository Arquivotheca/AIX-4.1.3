/* @(#)95	1.17  src/bos/kernel/sys/vmount.h, syslfs, bos411, 9428A410j 8/27/93 16:42:44 */

#ifndef _H_VMOUNT
#define _H_VMOUNT

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

/*
 * Types of file systems for the vmount system call
 *	NOTE -  When implementing new file system types, ensure
 *              that the vfs numbers do not conflict.
 */

#define	MNT_AIX		0		/* AIX physical fs "oaix"	*/
#define	MNT_NFS		2		/* SUN Network File System "nfs"*/
#define	MNT_JFS		3		/* AIX R3 physical fs "jfs"	*/
#define MNT_CDROM	5		/* CDROM File System "cdrom"	*/

/*
** user file systems please start at MNT_USRVFS
*/

#define MNT_BADVFS     -1               /* always illegal vfs type */
#define MNT_USRVFS     8		/* first 8 (0 - 7) reserved for IBM use */
#define MNT_AIXLAST    15		/* last valid vfs number. */

/*
 * Flags to describe the attributes of a mounted file system
 * in the vmt_flag field of a vmount structure.
 *
 * The mount flags field is shared with the vfs flags (referred to
 * by vfs_flag).  The mount flags should be contained in the low order
 * 16 bits, and the vfs flags in the high order 16 bits.  The flags
 * field is shared because some of the vfs flags are redefinitions of
 * the mount flags.  See sys/vfs.h for the vfs flag definitions.
 */
#define	MNT_READONLY	0x0001		/* no write access to vfs       */
#define	MNT_REMOVABLE	0x0002		/* media can be removed         */
#define	MNT_DEVICE	0x0004		/* physical device mount        */
#define	MNT_REMOTE	0x0008		/* file system is on network    */
#define MNT_SYSV_MOUNT	0x0010		/* System V style mount		*/
#define	MNT_UNMOUNTING	0x0020		/* originated by unmount()	*/
#define	MNT_NOSUID	0x0040		/* don't maintain suid-ness	*/
					/* across this mount		*/
#define	MNT_NODEV	0x0080		/* don't allow device access	*/
					/* across this mount		*/
/*
 * MNT_READONLY, MNT_REMOVABLE, MNT_SYSV_MOUNT, MNT_NOSUID, and MNT_NODEV
 * may be used as flags for mount(2) and vmount(2) calls.
 */
#define VMOUNT_MASK	\
	(MNT_READONLY | MNT_REMOVABLE | MNT_SYSV_MOUNT | MNT_NOSUID | MNT_NODEV)

/*
 * The variable length data descriptors are an array of offsets
 * (from beginning of struct vmount) and sizes.
 * NOTES:
 * If a particular area has no data, offset and size should be 0.
 * The size must always be filled in, even if the data is a NULL
 * terminated printable text string.
 */
#define VMT_OBJECT	0	/* I index of object name		*/
#define VMT_STUB	1	/* I index of mounted over stub name	*/
#define VMT_HOST	2	/* I index of (short) hostname		*/
#define VMT_HOSTNAME	3	/* I index of (long) hostname		*/
#define VMT_INFO	4	/* I index of binary vfs specific info	*/
				/*   includes network address, opts, etc*/
#define VMT_ARGS	5	/* I index of text of vfs specific args	*/
#define VMT_LASTINDEX	5	/* I the last in the array of structs	*/

/*
 * Vmount system call argument, also a mntctl return structure.
 * This structure has a fixed size of data and offsets, and then
 * has the data for the fs pointed to by "vmount ptr + offset".
 * If a particular area has no data, the offset and size should be 0.
 * Input parameters are indicated by "I", output only parameters
 * are indicated by "O" (input parameters are also output).
 */
struct vmount {
	ulong	vmt_revision;	/* I revision level, currently 1	*/
	ulong	vmt_length;	/* I total length of structure and data	*/
	fsid_t	vmt_fsid;	/* O id of file system			*/
	int	vmt_vfsnumber;	/* O unique mount id of file system	*/
	time_t	vmt_time;	/* O time of mount			*/
	ulong	vmt_timepad;	/* O (in future, time is 2 longs)	*/
	int	vmt_flags;	/* I general mount flags		*/
				/* O MNT_REMOTE is output only		*/
	int	vmt_gfstype;	/* I type of gfs, see MNT_XXX above	*/
	struct vmt_data {
		short vmt_off;	/* I offset of data, word aligned	*/
		short vmt_size;	/* I actual size of data in bytes	*/
	} vmt_data[VMT_LASTINDEX + 1];
	/*
	 * the variable length data goes here, starting at word (32 bit)
	 * boundaries.
	 */
};

#define	VMT_REVISION	1	/* current version of struct vmount	*/

/*
 * macros to easily get ptr and size of variable length info
 * given ptr to vmount structure and index to object
 */
#define	vmt2dataptr(vmt, idx)	((caddr_t)(((int)(vmt)) + \
				((vmt)->vmt_data[(idx)].vmt_off)))
#define	vmt2datasize(vmt, idx)	((vmt)->vmt_data[(idx)].vmt_size)

/*
 * mntctl operations and arguments.
 *	mntctl(cmd, size, buf)
 * cmd - one of the command defines below (MCTL_????)
 * size - size of the area that buf points to
 * buf - pointer to an argument/results area
 */
#define	MCTL_QUERY	2	/* (new style) query what is mounted
				 * buf points to array of vmount structures
				 * (which are variable length)
				 * returns: 0 = look in first word of buf
				 *	for needed size,
				 * -1 = error,
				 * >0 = number of vmount structs in buf
				 */

/*
 * flags for the uvmount(2) system call
 * int uvmount(vfsnumber, flags)
 *	int	vfsnumber; number from vmount structure, or statfs
 */
#define	UVMNT_FORCE	0x0001	/* force the unmount, regardless! */

#endif /* _H_VMOUNT */
