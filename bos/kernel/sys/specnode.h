/* @(#)83	1.6.1.8  src/bos/kernel/sys/specnode.h, sysspecfs, bos41J, 9521A_all 5/23/95 08:03:36 */
#ifndef _H_SNODE
#define _H_SNODE

/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/acl.h>
#include <sys/time.h>

/* defines for fifo in-memory data structures */
#define	L2NFBUF		3
#define	NFBUFMASK 	(NFBUF - 1)

/* The number of fifo buf ptrs must be a power of 2 */
#define	NFBUF		(1 << L2NFBUF)

struct specnode {
	struct specnode *sn_next;	/* next specnode on devnode list */
	int		sn_gen;		/* generation number 		*/
	cnt_t		sn_count;	/* count of holders of specnode	*/
	Simple_lock	sn_lock;	/* lock word     		*/
	struct gnode	sn_gnode;	/* gnode for device special	*/
	struct gnode *	sn_pfsgnode;	/* PFS gnode for device special	*/
	struct icattr *	sn_attr;	/* in-core attr for chan, pipe	*/
	union
	{
		struct devnode * _sn_dnode;	/* devnode for specnode	*/
		struct fifonode *_sn_fnode;	/* fifonode for specnode */
	} sn_data;
};

/* field defines for specnodes */
#define sn_dev		sn_gnode.gn_rdev
#define sn_chan		sn_gnode.gn_chan
#define sn_vnode	sn_gnode.gn_vnode
#define sn_ops		sn_gnode.gn_ops
#define sn_type		sn_gnode.gn_type
#define	sn_devnode	sn_data._sn_dnode
#define sn_fifonode	sn_data._sn_fnode

/* in-core attributes for use with pipes and channels of mpx devices */
struct icattr {
	mode_t		ica_mode;	/* in-memory mode		*/
	uid_t		ica_uid;	/* in-memory uid		*/
	gid_t		ica_gid;	/* in-memory gid		*/
	struct timestruc_t ica_atime;	/* in-memory access time	*/
	struct timestruc_t ica_mtime;	/* in-memory modification time	*/
	struct timestruc_t ica_ctime;	/* in-memory change time	*/
	struct acl *	ica_acl;	/* in-memory ACL		*/
};

/* specnode field identifiers for in-core attributes */
#define sn_mode		sn_attr->ica_mode
#define sn_uid		sn_attr->ica_uid
#define sn_gid		sn_attr->ica_gid
#define sn_atime	sn_attr->ica_atime
#define sn_mtime	sn_attr->ica_mtime
#define sn_ctime	sn_attr->ica_ctime
#define sn_acl		sn_attr->ica_acl

/* per open device statistics
 * NOTE:  The dev field must be at the same offset of the device number
 * in the fifonode struct.
 */
struct devnode {
	struct devnode *dv_forw;	/* forward hash chain		*/
	struct devnode *dv_back;	/* backward hash chain		*/
	dev_t		dv_dev;		/* device number of device	*/
	cnt_t		dv_count;	/* count of holders of devnode	*/
	ushort		dv_flag;	/* flags; see below		*/
	struct gnode	dv_gnode;	/* gnode for device		*/
	struct specnode *dv_specnodes;	/* specnodes using devnode	*/
	Simple_lock	dv_lock; 	/* devnode simple lock 		*/

	/* fields for use with block devices */
	ulong		dv_lastr;	/* last block read (read ahead)	*/
	/* fields for use with clone devices */
	caddr_t		dv_pdata;	/* private data			*/
};

/* devnode field identifiers */
#define dv_chan		dv_gnode.gn_chan
#define dv_rdcnt	dv_gnode.gn_rdcnt
#define dv_wrcnt	dv_gnode.gn_wrcnt
#define dv_rshcnt	dv_gnode.gn_rshcnt
#define dv_mntcnt	dv_gnode.gn_mntcnt

/* specnode field identifiers for block special data */
#define sn_lastr	sn_devnode->dv_lastr

/* dv_flag: devnode flags */
#define DV_CLOSING	0x0001	/* device close is in progress */
#define DV_OPENPENDING 	0x0002	/* a thread is waiting for close to complete */
#define DV_REVOKING	0x0004	/* device revoke is in progress */
#define DV_CLOSEPENDING	0x0008	/* a thread is waiting for revoke to complete*/

/* fifo and pipe information
 * NOTE:  The dev field must be at the same offset of the device number
 * in the devnode struct.
 */
struct fifonode {
	struct fifonode *ff_forw;	/* forward hash chain		*/
	struct fifonode *ff_back;	/* backward hash chain		*/
	dev_t		ff_dev;		/* dev num for hash (NODEVICE)	*/
	struct gnode *	ff_pfsgnode;	/* gnode for fifo		*/
	struct specnode *ff_specnode;	/* specnode for fifo		*/
	ulong		ff_size;	/* num bytes in fifo		*/
	ushort		ff_wptr;	/* write offset in fifo		*/
	ushort		ff_rptr;	/* read offset in fifo		*/
	ushort		ff_poll;	/* requested events		*/
	short		ff_flag;	/* fifo flags			*/
	short		ff_rcnt;	/* num waiting readers		*/
	short		ff_wcnt;	/* num waiting writers		*/
	int		ff_revent;	/* event list for writers	*/
	int		ff_wevent;	/* event list for readers	*/
	caddr_t		ff_buf[NFBUF];	/* array of fifo bufs		*/
};

/* bits in ff_flag in fifonode */
#define FIFO_RBLK	0x0001	/* blocked readers */
#define FIFO_WBLK	0x0002	/* blocked writers */
#define FIFO_WWRT	0x0004	/* was/is writer on fifo */

/* field identifiers for fifo data */
#define sn_size		sn_fifonode->ff_size
#define sn_wptr		sn_fifonode->ff_wptr
#define sn_rptr		sn_fifonode->ff_rptr
#define sn_poll		sn_fifonode->ff_poll
#define sn_rcnt		sn_fifonode->ff_rcnt
#define sn_wcnt		sn_fifonode->ff_wcnt
#define sn_revent	sn_fifonode->ff_revent
#define sn_wevent	sn_fifonode->ff_wevent
#define sn_flag		sn_fifonode->ff_flag
#define sn_buf		sn_fifonode->ff_buf

#define SGTOSP(x)	((struct specnode *)((x)->gn_data))
#define STOSGP(x)	(&((x)->sn_gnode))
#define STODP(x)	((x)->sn_devnode)
#define STOFP(x)	((x)->sn_fifonode)
#define FTOSP(x)	((x)->ff_specnode)
#define DTODGP(x)	(&((x)->dv_gnode))
#define DGTODP(x)	((struct devnode *)((x)->gn_data))
#define STODGP(x)	(DTODGP(STODP(x)))
#define VTOSP(x)	(SGTOSP(VTOGP(x)))
#define VTODP(x)	(STODP(VTOSP(x)))
#define VTODGP(x)	(STODGP(VTOSP(x)))
#define SGTODGP(x)	(STODGP(SGTOSP(x)))

#define BASE_MPX	((chan_t)(-1))
#define MPX_CHANNEL(gp)	((gp)->gn_type == VMPC && (gp)->gn_chan != BASE_MPX)
#define MPX_BASE(gp)	((gp)->gn_type == VMPC && (gp)->gn_chan == BASE_MPX)

#define	NHNODE		256		/* should be a power of two */
#define	DEVHASH(x, y)	(&hnodetable[(((int)major(x) << 4) ^ (int)x \
						^ (int)(y)) & (NHNODE-1)])
#define FIFOHASH(x)	(&hnodetable[(((int)(x) >> 16) ^ (int)(x)) & (NHNODE-1)])

/* hash anchors for the specnode table */
struct hnode {
	union
	{
		struct devnode *	_hdv_forw;
		struct fifonode *	_hff_forw;
	} hn_forw;
	union
	{
		struct devnode *	_hdv_back;
		struct fifonode *	_hff_back;
	} hn_back;
	Simple_lock	hn_lock;	
};

/* field defines for hash nodes */
#define	hdv_forw	hn_forw._hdv_forw
#define	hff_forw	hn_forw._hff_forw
#define	hdv_back	hn_back._hdv_back
#define	hff_back	hn_back._hff_back

/* hash table for devnodes and fifonodes */
extern struct hnode hnodetable[];

#endif	/* _H_SNODE */
