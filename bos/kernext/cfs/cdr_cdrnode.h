/* @(#)28	1.5  src/bos/kernext/cfs/cdr_cdrnode.h, sysxcfs, bos411, 9428A410j 7/8/94 17:23:28 */

#ifndef _H_CDRFS_CDRNODE
#define _H_CDRFS_CDRNODE

/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	cdr_cdrnode.h: CD-ROM file system in-core inode cdrnode
 */

#include	<sys/types.h>
#include	<sys/vnode.h>

/* 
 *	extent descriptor
 */
#define	CDR_NXD		3	/* number of extent descriptor slots	*/
				/* statically allocated in cdrnode	*/
struct cdrxd {
	struct cdrxd 	*cx_next;	/* pointer to next xd	*/
	daddr_t		cx_locdata;	/* lblkn of file section	*/
	ulong		cx_data_len;	/* length of file section in byte */
	uchar		cx_xar_len;	/* length of xar in lblk	*/
};


/* 
 *	CD-ROM file system "incore inode" cdrnode
 *
 * note: the CD-ROM file structure provides directory records as
 * the only descriptor of the file system object (i.e., it does not
 * have constructs similar to 'disk i-node' uniquely identifying 
 * each file system object).
 *
 * For i-number of each CDRFS object, each cdrnode is identified by
 * its cn_number (the "i_number") defined as follows:
 * . regular file or directory fle: the address of the file system
 *   object 
 * . other types (symbolic link, special files): the address of
 *   their corresponding directory entry
 *   (this implies that other types cannot have hard links.)
 *
 * For the NFS file identifier, address of the directory entry and
 * the parent directory are used to be able to read the descriptor
 * (i.e., the directory entry) of the object.
 *
 * For internal cdrnode cache management, each cdrnode is identified
 * by (cn_dev, cn_dirent) as dev/ino pair for UFS files.
 * mounted file system cdrnode (cn_dirent == 0) is used as agent to
 * locate the cdrfsmount when access path to the cdrfsmount is
 * not available (in iget()).
 *
 * note:
 * time stamps are represented in seconds since the Epoch 
 * (00:00:00 January 1, 1970 Coordinated Universial Time  (UTC)).
 */
struct cdrnode {
	struct cdrnode *cn_forw;	/* hash chain forward link	*/
	struct cdrnode *cn_back;	/* hash chain backward link	*/
	struct cdrnode *cn_next;	/* next on cdrnode cached list	*/
	struct cdrnode *cn_prev;	/* prev on cdrnode cached list	*/
	struct gnode	cn_gnode;	/* generic part of file node	*/
	ino_t		cn_number;	/* cdrnode number		*/
	daddr_t		cn_dirent;	/* directory (entry) address	*/
	daddr_t		cn_pdirent;	/* parent directory address	*/
	dev_t		cn_dev;		/* device of inode residedence  */
	struct cdrfsmount *cn_cdrfs;	/* mounted file system data	*/
	ushort		cn_format;	/* file format 			*/
	ushort		cn_flag;	/* inode type locking flags	*/
	cnt_t		cn_count;	/* count of holders of cdrnode	*/
	ulong		cn_gen;		/* cdrnode generation number	*/
	mode_t		cn_mode;	/* file permissions		*/
	nlink_t		cn_nlink;	/* file link count		*/
	uid_t		cn_uid;		/* file owner identification	*/
	gid_t		cn_gid;		/* file group identification	*/
	off_t		cn_size;	/* file size in bytes		*/
	ulong		cn_nblocks;	/* actual number of lblks used	*/
	time_t		cn_mtime;	/* file modification date/time	*/
	time_t		cn_atime;	/* file access date/time	*/
	time_t		cn_ctime;	/* file creation date/time	*/
	long		cn_event;	/* event list for the cdrnode	*/	

	/* 
	 * file type dependent information 
	 */
	union cdrftdi {
#define		CN_PRIVATE	64	
		uchar	_cn_private[CN_PRIVATE];

		/* 
		 * regular file or directory: extent descriptors 
		 */
		struct cdrregdir {
			struct cdrxd	_cn_xd[CDR_NXD]; /* array of cdrxd */
			struct cdrxd	*_cn_xdlist;	/* list of cdrxd */
		} _cn_regdir;
#define		cn_xd		_cn_ftdi._cn_regdir._cn_xd
#define		cn_xdlist	_cn_ftdi._cn_regdir._cn_xdlist

		/* 
		 * device special file 
		 */
		struct {
			dev_t	_cn_rdev;
		} _cn_dsf;
#define		cn_rdev		_cn_ftdi._cn_dsf._cn_rdev

		/* 
		 * symbolic link 
		 */
		union {
			char	_cn_symlink[CN_PRIVATE]; /* symlink cached in cdrnode */
			char	*_cn_symfile;		/* symlink cached in malloc()ed buffer */
		} _cn_sl;
#define		cn_symlink	_cn_ftdi._cn_sl._cn_symlink
#define		cn_symfile	_cn_ftdi._cn_sl._cn_symfile
	} _cn_ftdi;
};

#define	cn_seg		cn_gnode.gn_seg		/* associated segment number */ 
#define	cn_mapcnt	cn_gnode.gn_mrdcnt	/* mapped file count */

/* conversion macro among cdrnode, gnode, and cdrnode */
#define GTOCDRP(gp)	((struct cdrnode *)(((struct gnode *)(gp))->gn_data))
#define VTOCDRP(vp)	(GTOCDRP(VTOGP(vp)))
#define CDRTOGP(cdrp)	(&((struct cdrnode *)(cdrp))->cn_gnode)

/* defines for cn_flag in the cdrnode */
#define	ILOCK	0x00000001	/* cdrnode is locked */
#define	IWANT	0x00000002	/* cdrnode is wanted */
#define	IGETLCK	0x00000004	/* cdrnode is in transition */

/* 
 *	CD-ROM file system fileid
 */
#define FIDFILLER	(MAXFIDSZ - (sizeof(uint_t)		\
				     + 2 * sizeof(daddr_t)	\
				     + sizeof(ushort_t)))

struct cdrfileid {
	uint_t		fid_len;
	daddr_t		fid_dirent;
	daddr_t		fid_pdirent;
	ushort_t	fid_hash;
	uchar		fid_filler[FIDFILLER];
};

#endif /* _H_CDRFS_CDRNODE */
