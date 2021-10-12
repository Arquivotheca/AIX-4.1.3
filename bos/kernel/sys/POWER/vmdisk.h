/* @(#)61	1.24  src/bos/kernel/sys/POWER/vmdisk.h, sysvmm, bos411, 9428A410j 5/3/94 18:06:27 */

#ifndef _H_VMDISK
#define _H_VMDISK

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * this is the structure for a Paging Device Table entry.
 * local block devices have type D_PAGING or D_FILESYTEM
 * or D_LOG. there is one PDT table entry per device (i.e
 * logical volume). remote devices have type D_REMOTE and
 * there is one PDT table entry per strategy routine per
 * file system type. see vmdevices.c and v_pdtsubs.c
 * for the procedures which manage the pdt table, and see
 * v_disksubs.c for routines which use them.
 *
 * there are two i/o lists associated with a PDT table
 * entry for a device. page frames scheduled for i/o are kept
 * in a circular list and pdt_iotail points to the last frame
 * placed in the list. the list is managed fifo and the
 * page frames are removed from the list as soon as the
 * i/o has been dispatched to the device (LVM or remote
 * strategy routine). the other list is a list of PDT
 * entries which have i/o scheduled. this list is also
 * circular list and pf_iotail  points to the last pdt put
 * in the list and pdt_nextio is used to point to the next
 * pdt on the list.
 */

#ifdef _POWER_MP
#include <sys/lock_def.h>
#endif /* _POWER_MP */
#include <sys/uio.h>

struct pdtentry
{
    ushort	_type;		/* type device (paging,log,remote,filesys) */
    short	_nextio;	/* next pdt on i/o list  */
    union {
	dev_t   dev;		/* dev_t of block device       */
	int ( *ptr)();          /* pointer to strategy routine */
	} _device;
    int 	_iotail;	/* last page frame with pending I/O	*/
    struct buf *_bufstr;	/* free buf_struct list 	*/
    ushort 	_nbufs;		/* total number of buf structs  */
    ushort      _avail;		/* for D_PAGING, indicates available for use */
    ushort 	_fperpage;	/* fragments per page */
    ushort 	_comptype;	/* compression type for this filesystem */
    int 	_dmsrval;	/* sreg val for addressing map (non-paging) */
    int 	_iocnt; 	/* number of i/o's  not yet finished */
#ifdef _POWER_MP
    Simple_lock _lock;          /* logical volume lock (= disk maps, quotas, ...) */
    int 	_pad[7];	/* we want only one lock per cache line */
#endif /* _POWER_MP */
};

#define  pdt_type(x)		vmmdseg.pdt[(x)]._type
#define  pdt_nextio(x)		vmmdseg.pdt[(x)]._nextio
#define  pdt_device(x)		vmmdseg.pdt[(x)]._device.dev
#define  pdt_strategy(x)	vmmdseg.pdt[(x)]._device.ptr
#define  pdt_iotail(x)		vmmdseg.pdt[(x)]._iotail
#define  pdt_bufstr(x)		vmmdseg.pdt[(x)]._bufstr
#define  pdt_nbufs(x)		vmmdseg.pdt[(x)]._nbufs
#define  pdt_avail(x) 		vmmdseg.pdt[(x)]._avail
#define  pdt_fperpage(x)	vmmdseg.pdt[(x)]._fperpage
#define  pdt_comptype(x)	vmmdseg.pdt[(x)]._comptype
#define  pdt_dmsrval(x) 	vmmdseg.pdt[(x)]._dmsrval
#define  pdt_iocnt(x) 		vmmdseg.pdt[(x)]._iocnt
#ifdef _POWER_MP
#define  lv_lock(x)		vmmdseg.pdt[(x)]._lock
#endif /* _POWER_MP */

/*
 * the same kind of allocation map is used for disk fragments 
 * and inodes. the description given below is in terms for
 * disk fragments but the same holds for inodes or fragments.
 * (allocation state of each is represented by a bit).
 */

/*
 * bit maps are grouped into a working map and a permanent map.
 * the working map represents the current allocation state
 * and the permanent map represents a committed state. the map
 * is written to disk periodically by log sync code or by
 * normal paging activity . if the system crashes, the info
 * in the log and the last copy of the permanent map on 
 * disk is used to compute the committed allocation state.
 * one bit in each map represents the state of a block (0 = free)
 * with the bit position in the map being equal to block number.
 */

/* in V3 file-systems, allocation requests are for a sequence of
 * 1 to 8 bits, all of which must be in a byte of the map. for
 * V4 requests are for a sequence of 1 to 32 bits. considering
 * the map as an array of double-words (64-bits), a request is
 * is wholly satisfied with bits within a double-word. 
 *
 * to avoid sequential scans of all the bytes in the map, a 
 * tree structure is maintained that specifies the longest
 * sequence in groups of words in a page. a leaf of  the tree
 * is a word of 4 bytes with each byte encoding the longest
 * sequence in 8-bytes of the map. a node at the next level
 * is a word each byte of which encodes the longest sequence
 * in the 4 bytes of a leaf word. the tree has 4-levels with
 * a fan-out of 4 at each level.
 */

/* the tree structure is contained in the first 512 bytes
 * of the vmdmap structure. these 512 bytes represent 
 * control information. in V3, the control information for
 * a page is located at the beginning of each page. in V4
 * the control information has been moved to pages only have
 * have control information. thus in V4 , pages 9*k are 
 * control info pages and all other pages consist of only
 * bit maps. page 9*k contains the control info for pages 
 * 9*k + 1, 9*k + 2, .. 9*k+8. the control info for these is
 * mapped in page 9*k in the obvious way : the info for page
 * 9*k + n is at offset 512*(n - 1).
 */

/* allocation groups. disk blocks are aggregated into contiguous
 * blocks called allocation groups and a group of inodes is placed
 * in each disk-allocation group. the run-time block allocator
 * attempts to place the disk-blocks for a file near its inode
 * so that disk-seeks are minimized by preferentially allocating
 * from its allocation group.
 */

/* in converting an existing V3 file sytem to a V4 system,
 * the sizes of permitted allocation group sizes (expressed in
 * number of bits) must be considered. in V4 the size of a
 * group is constrained to be a power of 2 multiple of
 * MINAGSIZEV4 = 512 no larger than DBPERPAGEV4 = 16384.
 * with V3 it is constrained to be any multiple of MINAGSIZE = 256
 * which is also a divisor of DBPERPAGE = 7*2048. in V3 only
 * filesystems of size < 8 megabytes when created will have
 * a size other than AGDEFAULT = 2048. for a file system of 
 * size greater then 4 and up to 7 megabytes, it is 7*256.
 * for all other small sizes, it is a power of 2 multiple 
 * 256 < AGDEFAULT. 
 */

/*
 * each (file-system) disk map and each inode map is kept in
 * its own segment. however , all paging space disk maps
 * are kept in one segment which is allocated at VMM init
 * and whose index in the scb table is the constant DMAPSIDX.
 * all disk maps are the same size DMAPSIZE (8 megabytes).
 * the offset of the first page of the map corresponding
 * to the (paging space) device with PDT index pdtx is equal
 * to pdtx*DMAPSIZE. paging space disk maps use the version 0
 * formats.
 */
#define MAXMAPSIZE	(16*(1 << 20))  	/* maximum map size in blocks */
#define DMAPSIZE	(1 << 23)		/* map size = 8 megabyte */
#define L2DMSIZE	23			/* log of DMAPSIZE	 */
#define L2DMPAGES	L2DMSIZE - 12		/* log of DMAPSIZE in pages */
#define WPERPAGE 	(7*512/8)		/* work-map words V3 */
#define WPERPAGEV4	(8*512/8)		/* work-map words V4  */
#define DBWORD		32			/* bits per word */
#define DBPERDWORD	(DBWORD*2)		/* bits per double word */
#define L2DBWORD	5			/* log of DBWORD    */
#define DBPERPAGE	(DBWORD*WPERPAGE)	/* bits per page V3 */
#define DBPERPAGEV4     (DBWORD*WPERPAGEV4)	/* bits per page V4 */
#define DMPDT		0x0f000000		/* mask for pdt index */
#define DMBLK		0x00ffffff		/* mask for block number */
#define MAXPGDEV	16			/* max number of paging disks */
#define LEAFIND		21			/* index left-most tree leaf */
#define TREESIZE	(64+16+4+1)		/* tree size in words */
#define MINAGSIZE 	256			/* min alloc group V3  */
#define MINAGSIZEV4 	512			/* min alloc group V4  */
#define MAXAGPAGE 	(DBPERPAGE/MINAGSIZE)	/* max alloc groups per page */
#define AGDEFAULT	2048			/* dflt alloc gr size 4k pages*/
#define AGDEFAULTV4	2048			/* dflt alloc gr size 4k pages*/
#define CLDEFAULT	8			/* dflt cluster size V3 */
#define FSCLSIZE	4			/* fs realloc cluster size */
#define ALLOCMAPV3	0			/* version number for V3 maps */
#define ALLOCMAPV4	1			/* version number for V4 maps */
#define FRAGDEFAULT	4096			/* default fragment size */
#define MINFRAGSIZE	512			/* min val fragsize */
#define MAXFRAGSIZE	4096			/* max val fragsize */
#define NBPIDEFAULT	4096			/* dflt num bytes per inode */
#define MINNBPI		512			/* min num bytes per inode */
#define MAXNBPI		16384			/* max num bytes per inode */
#define CLDEFAULTV4	32			/* default cluster size V4 */
#define MAXAGSIZEV4 	DBPERPAGEV4		/* largest ag size in bits */
#define LMAPCTL 	512			/* length control data in bytes */

/* the first 512 bytes of vmdmap represent control information.
 * in version V3 , this is followed by an array of working-bit
 * maps and then an array of permanent bit maps. the size of
 * vmdmap is 4k bytes. in version V4, the remainder of the page
 * is an array of 7 more 512 byte control structures.
 */
struct vmdmap
{
	/* begin control data. 512 bytes */
	uint	mapsize;	/* number of fragments covered by map */
	uint	freecnt;	/* total number of free fragments */
	uint    agsize;		/* allocation group size in fragments */
	uint	agcnt;	        /* number of allocation groups this page*/
	uint 	totalags;	/* number of ags in map (page 0 only) */
	uint	lastalloc;	/* last fragment allocated (page 0 only) */
	uint	maptype;	/* type of map                        	 */
	uint 	clsize;		/* maximum sequence to allocate */
	uint	clmask;		/* encoded form of clsize V3 only */
	uint	version;	/* version number */
	uint	spare[2];
	short	agfree[MAXAGPAGE]; 	/* free counts in allocation grs. */
	uint    tree[TREESIZE];		/* tree of max - sequences */
	int     spare1[128-12-MAXAGPAGE/2-TREESIZE];    /*spares */
	/* begin allocation maps for V3..more summary info for V4 to
	 * the end of 4k page.
	 */
	uint	mapsorsummary[2*WPERPAGE];  
};

/* disk address format.
 */
struct fragment_ptr
{
	unsigned new	:1;	/* non-committed allocation */
	unsigned nfrags	:3;	/* number of fragments less than full block */
	unsigned addr	:28;	/* fragment number of the first fragment */
};

typedef struct fragment_ptr frag_t;

#define NDLIST	30
struct vmdlist {
	struct  vmdlist * next;  /* next structure on list */
	ushort	nblocks;         /* number of disk addresses in list */
	ushort	fperpage;        /* number of fragments per page */
	union {
		uint	dblock[NDLIST];	 /* disk addresses */
		frag_t	fptr[NDLIST];	 /* disk addresses as frag_t */
	} da;	
};

/* moved fragment structures are used to reperesent the old 
 * committed disk allocation for up to 4 pages. each  structure
 * is the same size as a lockword (32-bytes) and storage for a
 * moved fragment structure is allocated from the same pool
 * as lockwords (see vmm/vmlock.h). the anchor for the list of
 * movedfrag structures for a file is in the inode (i_movedfrag).
 */
struct movedfrag
{
	struct movedfrag * next; /* pointer to next on hash chain */
	struct movedfrag * nextsid; /* next for same base sid */
	int	sid;		/* base sid */
	int	pno;		/* first of 4 consecutive pages */
	uint	olddisk[4];	/* frag pointers for corresponding pages */
};

/* vmsrclist structure are cheap uio structures and are used to represent
 * the source of a move operation having multiple sources.
 */

#define	MAXSIOVECS	16

struct vmsrclist {
	int	nvecs;			/* number of source valid iovecs */
	struct iovec vecs[MAXSIOVECS]; /* source iovecs */
};

/* defines for parameters to bit allocation routines
 */
#define	V_PMAP		1
#define V_WMAP		2
#define V_PWMAP		3

/* types of maps  
 */
#define FSDISKMAP   1
#define INODEMAP    2
#define PGDISKMAP   3

/* macro for generating search tree comparision character mask based upon
 * map verion number and number of fragments.  mask used for checking for
 * n free fragments.
 */
#define DBFREEMSK(v,n) \
	(((v) == ALLOCMAPV3) ? (ONES << (8 - (n))) & 0xff : (n)) 

/* macro for number of bits in work-map per page of map
 */
#define WBITSPERPAGE(v) \
	(((v) == ALLOCMAPV3) ? DBPERPAGE : DBPERPAGEV4)

/* macro for calculating pointer to vmdmap for a page from
 * the pointer to the control page for pno (p1) and the
 * page number pno.
 */
#define VMDMAPPTR(p1,pno) \
	((struct vmdmap *) ((char *)(p1) + (((pno) & 0x7) << 9)))

#endif /* _H_VMDISK */
