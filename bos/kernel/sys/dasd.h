/* @(#)82	1.34.1.8  src/bos/kernel/sys/dasd.h, sysxlvm, bos411, 9428A410j 5/23/94 10:05:57 */

/*
 *   COMPONENT_NAME: SYSXLVM
 *
 *   FUNCTIONS: *		BBHASH_IND
 *		BLK2BYTE
 *		BLK2PART
 *		BLK2PG
 *		BLK2TRK
 *		BYTE2BLK
 *		CLRLVOPN
 *		CLRPVWRT
 *		FIRST_MASK
 *		FIRST_MIRROR
 *		GET_BBLK
 *		HASH_BAD
 *		HASH_BAD_DMP
 *		HD_HASH
 *		MIRROR_COUNT
 *		MIRROR_EXIST
 *		MIRROR_MASK
 *		PART2BLK
 *		PARTITION
 *		PG2BLK
 *		PG2TRK
 *		REL_BBLK
 *		SETLVOPN
 *		SETPVWRT
 *		TRK2BLK
 *		TRKPPART
 *		TRK_IN_PART
 *		TSTALLPVWRT
 *		TSTLVOPN
 *		TSTPVWRT
 *		VG_DEV2LV
 *		VG_DEV2PV
 *		X_AVOID
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1990
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#ifndef _H_DASD
#define _H_DASD

/*
 *	Logical Volume Manager Device Driver data structures.
 */

#include <sys/types.h>
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/sysmacros.h>
#include <sys/buf.h>
#include <sys/lvdd.h>

/*  FIFO queue structure for scheduling logical requests. */
struct hd_queue  {		/* queue header structure	*/
	struct buf *head;	/* oldest request in the queue	*/
	struct buf *tail;	/* newest request in the queue	*/
};

struct hd_capvq  {		/* queue header structure	*/
	struct pv_wait *head;	/* oldest request in the queue	*/
	struct pv_wait *tail;	/* newest request in the queue	*/
};
/*
 * Structure used by hd_redquiet() to mark target PPs for removal.
 * Both are zero relative.
 */
struct hd_lvred	{
	long	lp;		/* LP the pp belongs to		*/
	char	mirror;		/* mirror number of PP		*/
};
/*
 *  Physical request buf structure.
 *
 *	A 'pbuf' is a 'buf' structure with some additional fields used
 *	to track the status of the physical requests that correspond to
 *	each logical request.  A pool of pinned pbuf's is allocated and
 *	managed by the device driver.  The size of this pool depends on
 *	the number of open logical volumes.
 */

#define  STRIPE_DONE 0
#define  STRIPE_NOTDONE 1

struct pbuf {

	/* this must come first, 'buf' pointers can be cast to 'pbuf' */
	struct buf	pb;		/* imbedded buf for physical driver */

	/* physical buf structure appendage: */
	struct buf	*pb_lbuf;	/* corresponding logical buf struct */

	/* scheduler I/O done policy function	*/
#ifndef _NO_PROTO
	void		(*pb_sched) (struct pbuf *);
#else
	void		(*pb_sched) ();
#endif
	struct pvol	*pb_pvol;	/* physical volume structure	    */
	struct bad_blk	*pb_bad;	/* defects directory entry	    */
	daddr_t		pb_start;	/* starting physical address	    */

	char		pb_mirror;	/* current mirror		    */
	char		pb_miravoid;	/* mirror avoidance mask	    */
	char		pb_mirbad;	/* mask of broken mirrors	    */
	char		pb_mirdone;	/* mask of mirrors done		    */

	int 		pb_swretry:8;	/* number of sw relocation retries  */
	int		pb_type:4;	/* Type of pbuf			    */
	int		pb_bbfixtype:4; /* type of bad block fix */
	int 		pb_bbop:8;	/* BB directory operation	    */
	int		pb_bbstat:8;	/* status of BB directory operation */

	uchar		pb_whl_stop;	/* wheel_idx value when this pbuf is*/
					/* to get off of the wheel	    */
#ifdef DEBUG
	char		pad;		/* pad to full long word	    */
	ushort		pb_hw_reloc;	/* Debug - it was a HW reloc request*/
#else
	char		pad[3];		/* pad to full long word	    */
#endif

	struct part	*pb_part;	/* ptr to part structure.  Care must*/
					/* be taken when this is used since */
					/* the parts structure can be moved */
					/* by hd_config routines while the  */
					/* request is in flight		    */
	long            pb_bbcount;	/* volume group ID		    */
	/* used to dump the allocated pbuf at dump time			    */
	struct pbuf	*pb_forw;	/* forward pointer		    */
	struct pbuf	*pb_back;	/* backward pointer		    */

        struct pbuf     *stripe_next;   /* circular singly-linked pbufs list*/
        unsigned char   stripe_status;  /* outstanding/complete status      */
        caddr_t		orig_addr;      /* original b_baddr of this stripe  */
        unsigned int	orig_count;     /* original b_bcount of this stripe */
        unsigned char	partial_stripe; /* stripe_next list is partial xlate*/
        unsigned char   first_issued;   /* 1st req issued for striped req   */
	unsigned int	orig_bflags;	/* orig direction of read or write  */
};

#define pb_addr	pb.b_un.b_addr		/* too ugly in its raw form	    */

/* defines for pb_swretry */
#define MAX_SWRETRY	3		/* maximum retries for relocation
					   before declaring disk dead */

/* values for b_work in pbuf struct (since real b_work value only used 
 * in lbuf)
 */
#define FIX_READ_ERROR	1	/* fix a previous EMEDIA read error	*/
#define FIX_ESOFT	2	/* fix a read or write ESOFT error	*/
#define FIX_EMEDIA	3	/* fix a write EMEDIA error		*/

/* defines for pb_type */
#define	SA_PVMISSING	1	/* PV missing type request		*/
#define	SA_STALEPP	2	/* stale PP type request		*/
#define	SA_FRESHPP	3	/* fresh PP type request		*/
#define	SA_CONFIGOP	4	/* hd_config operation type request	*/

/*
 * defines to tell hd_bldpbuf what kind of pbuf to build
 * 
 * These defines are not the only ones that tell hd_bldpbuf what to
 * build.  Check the routine before changing/adding new defines here
 */
#define	CATYPE_WRT	1	/* pbuf struct is a cache write type	*/

/*
 * defines for pb_bbop
 *
 * First set is used by the requests pbuf that is requesting the BB operation.
 * The second set is used in the bb_pbuf to control the action of the
 * actuall reading and writing of the BB directory of the PV.
 */
#define BB_ADD		41	/* Add a new bad block entry to BB directory */
#define BB_UPDATE	42	/* Update a bad block entry to BB directory  */
#define BB_DELETE	43	/* Delete a bad block entry to BB directory  */
#define BB_RDDFCT	44	/* Reading a defective block		     */
#define BB_WTDFCT	45	/* Writing a defective block		     */
#define BB_SWRELO	46	/* Software relocation in progress	     */

#define RD_BBPRIM	70	/* Read the BB primary directory	   */
#define WT_UBBPRIM	71	/* Write BB prim dir with UPDATE	   */
#define WT_DBBPRIM	72	/* Rewrite BB prim dir 1st blk with UPDATE */
#define WT_UBBBACK	73	/* Write BB backup dir with UPDATE	   */
#define WT_DBBBACK	74	/* Rewrite BB back dir 1st blk with UPDATE */


/* defines for pb_berror: 0-63 (good) 64-127 (bad) */
#define BB_SUCCESS	0	/* BBdir updating worked		     */	
#define BB_CRB		1	/* Reloc blkno was changed in this BB entry  */
#define BB_ERROR	64	/* Bad Block directories were not updated    */
#define BB_FULL		65	/* BBdir is full -no free bad blk entries    */


/*
 *  Volume group structure.
 *
 *  Volume groups are implicitly open when any of their logical volumes are.
 */

#define MAXVGS		255		/* implementation limit on # VGs */
#define MAXLVS		256		/* implementation limit on # LVs */
#define MAXPVS		32		/* implementation limit on number*/
					/* physical volumes per vg	 */
#define	CAHHSIZE	8		/* Number of mwc cache queues	 */
#define NBPI	(NBPB * sizeof(int))	/* Number of bits per int	 */
#define NBPL	(NBPB * sizeof(long))	/* Number of bits per long	 */

/* macros to set and clear the bits in the opn_pin array */
#define SETLVOPN(Vg,N)	((Vg)->opn_pin[(N)/NBPI] |= 1<<((N)%NBPI))
#define CLRLVOPN(Vg,N)	((Vg)->opn_pin[(N)/NBPI] &= ~(1<<((N)%NBPI)))
#define TSTLVOPN(Vg,N)	((Vg)->opn_pin[(N)/NBPI] & 1<<((N)%NBPI))

/*
 * macros to set and clear the bits in the ca_pv_wrt field
 *
 * NOTE TSTALLPVWRT will not work if max PVs per VG is greater than 32
 */
#define SETPVWRT(Vg,N)	((Vg)->ca_pv_wrt[(N) / NBPL] |= 1<<((N) % MAXPVS))
#define CLRPVWRT(Vg,N)	((Vg)->ca_pv_wrt[(N) / NBPL] &= ~(1<<((N) % MAXPVS)))
#define TSTPVWRT(Vg,N)	((Vg)->ca_pv_wrt[(N) / NBPL] & (1<<((N) % MAXPVS)))
#define TSTALLPVWRT(Vg)	((Vg)->ca_pv_wrt[(MAXPVS - 1) / NBPL])


struct volgrp {
    lock_t		vg_lock;	/* lock for all vg structures	    */
    short		pad1;		/* pad to long word boundary	    */
    short		partshift;	/* log base 2 of part size in blks  */
    short		open_count;	/* count of open logical volumes    */
    ushort		flags;		/* VG flags field		    */
    ulong		tot_io_cnt;	/* number of logical request to VG  */
    struct lvol		*lvols[MAXLVS];	/* logical volume struct array	    */
    struct pvol		*pvols[MAXPVS];	/* physical volume struct array	    */
    long		major_num;	/* major number of volume group     */
    struct unique_id	vg_id;		/* volume group id		    */
    struct volgrp	*nextvg; 	/* pointer to next volgrp structure */
					/* Array of bits indicating open LVs*/
					/* A bit per LV			    */
    int			opn_pin[(MAXLVS + (NBPI - 1))/NBPI];
    pid_t		von_pid;	/* process ID of the varyon process */

	/* Following used in write consistency cache management		    */
    struct volgrp	*nxtactvg;	/* pointer to next volgrp with	    */
					/* write consistency activity	    */
    struct pv_wait	*ca_freepvw;	/* head of pv_wait free list	    */
    struct pv_wait	*ca_pvwmem;	/* ptr to memory malloced for pvw   */
					/* free list			    */
    struct hd_queue	ca_hld;		/* head/tail of cache hold queue    */
    ulong		ca_pv_wrt[(MAXPVS + (NBPL - 1)) / NBPL];
					/* when bit set write cache to PV   */
    char		ca_inflt_cnt;	/* number of PV active writing cache*/
    char		ca_size;	/* number of entries in cache	    */
    ushort		ca_pvwblked;	/* number of times the pv_wait free */
					/* list has been empty		    */
    struct mwc_rec	*mwc_rec;	/* ptr to part 1 of cache - disk rec*/
    struct ca_mwc_mp	*ca_part2;	/* ptr to part 2 of cache - memory  */
    struct ca_mwc_mp	*ca_lst;	/* mru/lru cache list anchor	    */
    struct ca_mwc_mp	*ca_hash[CAHHSIZE];/* write consistency hash anchors*/

    /* the following 2 variables are used to control a cache clean up opera-*/
    /* tion.								    */
    pid_t 		bcachwait;	/* list waiting at the beginning    */
    pid_t 		ecachwait;	/* list waiting at the end	    */
    volatile int	wait_cnt;	/* count of cleanup waiters	    */

    /* the following are used to control the VGSAs and the wheel */
    uchar		quorum_cnt;	/* Number indicating quorum of SAs  */
    uchar		wheel_idx;	/* VGSA wheel index into pvols	    */
    ushort		whl_seq_num;	/* VGSA memory image sequence number*/
    struct pbuf		*sa_act_lst;	/* head of list of pbufs that are   */
					/* actively on the VGSA wheel	    */
    struct pbuf		*sa_hld_lst;	/* head of list of pbufs that are   */
					/* waiting to get on the VGSA wheel */
    struct vgsa_area	*vgsa_ptr;	/* ptr to in memory copy of VGSA    */
    pid_t		config_wait;	/* PID of process waiting in the    */
					/* hd_config routines to modify the */
					/* memory version of the VGSA	    */
    struct buf		sa_lbuf;	/* logical buf struct to use to wrt */
					/* the VGSAs			    */
    struct pbuf		sa_pbuf;	/* physical buf struct to use to wrt*/
					/* the VGSAs			    */
    Simple_lock	 	sa_intlock;	/* status area lock */
};

/*
 * Defines for flags field in volgrp structure
 */
#define VG_NOQUORUM     0x0001		/* do not force the VG off when
					   quorum is lost */
#define	VG_SYSMGMT  	0x0002		/* VG is on for system management   */
					/* only commands		    */
#define VG_FORCEDOFF	0x0004		/* Should only be on when the VG was*/
#define	VG_OPENING  	0x0008		/* VG is being varied on	    */
	/* forced varied off and there were LVs still open.  Under this con-*/
	/* dition the driver entry points can not be deleted from the device*/
	/* switch table.  Therefore the volgrp structure must be kept	    */
	/* around to handle any rogue operations on this VG.		    */
#define	CA_INFLT	0x0010		/* The cache is being written or    */
					/* locked			    */
#define CA_VGACT	0x0020		/* This volgrp on mwc active list   */
#define	CA_HOLD		0x0040		/* Hold the cache in flight	    */
#define	CA_FULL		0x0080		/* Cache is full - no free entries  */
#define	SA_WHL_ACT	0x0100		/* VGSA wheel is active		    */
#define	SA_WHL_HLD	0x0200		/* VGSA wheel is on hold	    */
#define	SA_WHL_WAIT	0x0400		/* config function is waiting for   */
					/* the wheel to stop		    */


/*
 *  Logical volume structure.
 */
struct lvol {
	struct buf	**work_Q;	/* work in progress hash table	    */
	short		lv_status;	/* lv status: closed, closing, open */
	short		lv_options;	/* logical dev options (see below)  */
	short		nparts;		/* num of part structures for this  */
					/* lv - base 1			    */
	char		i_sched;	/* initial scheduler policy state   */
	char		pad;		/* padding so data word aligned     */
	ulong		nblocks;	/* LV length in blocks		    */
	struct part	*parts[3];	/* partition arrays for each mirror */
	ulong		tot_wrts;	/* total number of writes to LV	    */
	ulong		tot_rds;	/* total number of reads to LV	    */

	/* These fields of the lvol structure are read and/or written by
	 * the bottom half of the LVDD; and therefore must be carefully
	 * modified.
	 */
	int		complcnt;	/* completion count-used to quiesce */
	int 		waitlist;	/* event list for quiesce of LV	    */
        unsigned int    stripe_exp;     /* 2**stripe_block_exp = stripe     */
                                        /*                       block size */
        unsigned int    striping_width; /* number of disks striped across   */
};

/* lv status:  */
#define	LV_CLOSED  	0		/* logical volumes is closed	*/
#define	LV_CLOSING  	1		/* trying to close the LV	*/
#define	LV_OPEN   	2		/* logical volume is open	*/

/* scheduling policies: */
#define	SCH_REGULAR	0		/* regular, non-mirrored LV	*/
#define	SCH_SEQUENTIAL	1		/* sequential write, seq read	*/
#define	SCH_PARALLEL	2		/* parallel write, read closest	*/
#define	SCH_SEQWRTPARRD	3		/* sequential write, read closest*/
#define	SCH_PARWRTSEQRD	4		/* parallel write, seq read	*/
#define SCH_STRIPED     5               /* striped                      */

/* logical device options: */
#define	LV_NOBBREL	0x0010		/* no bad block relocation	*/
#define	LV_RDONLY	0x0020		/* read-only logical volume	*/
#define	LV_DMPINPRG	0x0040		/* Dump in progress to this LV	*/
#define	LV_DMPDEV	0x0080		/* This LV is a DUMP device	*/
					/* i.e. DUMPINIT has been done	*/
#define LV_NOMWC	0x0100		/* no mirror write consistency	*/
					/* checking			*/
#define	LV_WRITEV	WRITEV		/* Write verify writes in LV	*/

/* work_Q hash algorithm - just a stub now */
#define HD_HASH(Lb)	\
	(BLK2TRK((Lb)->b_blkno) & (WORKQ_SIZE-1))


/*
 *  Partition structure.
 */
struct part {
	struct pvol	*pvol;		/* containing physical volume	  */
	daddr_t		start;		/* starting physical disk address */
	short		sync_trk;	/* current LTG being resynced	  */
	char		ppstate;	/* physical partition state	  */
	char		sync_msk;	/* current LTG sync mask	  */
};

/*
 * Physical partition state defines PP_ and structure defines.
 *
 * The PP_STALE and PP_REDUCING bits could be combined into one but it
 * is easier to understand if they are not and a problem arises later.
 *
 * The PP_RIP bit is only valid in the primary part structure.
 */
#define PP_STALE	0x01		/* Set when PP is stale		  */
#define	PP_CHGING	0x02		/* Set when PP is stale but the	  */
					/* VGSAs have not been completely */
					/* updated yet			  */
#define	PP_REDUCING	0x04		/* Set when PP is in the process  */
					/* of being removed(reduced out	  */
#define PP_RIP		0x08		/* Set when a Resync is in progress */
					/* When set "sync_trk" indicates  */
					/* the track being synced.  If	  */
					/* sync_trk not == -1 and PP_RIP  */
					/* not set sync_trk is next trk	  */
					/* to be synced			  */
#define PP_SYNCERR	0x10		/* Set when error in a partition  */
					/* being resynced.  Causes the	  */
					/* partition to remain stale.	  */

#define NO_SYNCTRK	-1		/* The LP does not have a resync  */
					/* in progress			  */
/*
 *  Physical volume structure.
 *
 *	Contains defects directory hash anchor table.  The defects
 *	directory is hashed by track group within partition.  Entries within
 *	each congruence class are sorted in ascending block addresses.
 *
 *	This scheme doesn't quite work, yet.  The congruence classes need
 * 	to be aligned with logical track groups or partitions to guarantee
 *	that all blocks of this request are checked.  But physical addresses
 *	need not be aligned on track group boundaries.
 */

#define HASHSIZE	64		/* number of defect hash classes */

struct defect_tbl {
	struct bad_blk *defects [HASHSIZE];  /* defect directory anchor */
};

struct pvol {
	dev_t		dev;		/* dev_t of physical device	    */
	daddr_t		armpos;		/* last requested arm position	    */
	short		xfcnt;		/* transfer count for this pv	    */
	short		pvstate;	/* PV state			    */
	short		pvnum;		/* LVM PV number 0-31		    */
	short		vg_num;		/* VG major number		    */
	struct file *   fp;             /* file pointer from open of PV     */
	char		flags;		/* place to hold flags		    */
	char 		pad;		/* unused 			    */
	short		num_bbdir_ent;	/* current number of BB Dir entries */
	daddr_t		fst_usr_blk;	/* first available block on the PV  */
					/* for user data		    */
	daddr_t		beg_relblk;	/* first blkno in reloc pool	    */
	daddr_t		next_relblk;	/* blkno of next unused relocation  */
					/* block in reloc blk pool at end   */
					/* of PV			    */
	daddr_t		max_relblk;	/* largest blkno avail for reloc    */
	struct defect_tbl *defect_tbl;  /* pointer to defect table	    */
	struct hd_capvq	ca_pv;		/* head/tail of queue of request    */
					/* waiting for cache write to 	    */
					/* complete			    */
	struct sa_pv_whl {		/* VGSA information for this PV	    */
	    daddr_t	lsn;		/* SA logical sector number - LV 0  */
	    ushort	sa_seq_num;	/* SA wheel sequence number	    */
	    char	nukesa;		/* flag set if SA to be deleted     */
	    char	pad;		/* pad to full long word	    */
	}		sa_area[2];	/* one for each possible SA on PV   */

	struct pbuf	pv_pbuf;	/* pbuf struct for writing cache    */
};

/* defines for pvstate field */
#define PV_MISSING	1		/* PV cannot be accessed	    */
#define PV_RORELOC	2		/* No HW or SW relocation allowed   */ 
					/* only known bad blks relocated    */
/*
 * returns index into the bad block hash table for this block number
 */
#define BBHASH_IND(blkno)	(BLK2TRK(blkno) & (HASHSIZE - 1))

/*
 *  Macro to return defect directory congruence class pointer
 */
#define HASH_BAD(Pb,Bad_blkno)	\
	((Pb)->pb_pvol->defect_tbl->defects[BLK2TRK(Bad_blkno)&(HASHSIZE-1)])

/*
 * Used by the LVM dump device routines same as HASH_BAD but the first
 * argument is a pvol struct pointer
 */
#define HASH_BAD_DMP(Pvol,Blkno)	\
	  ((Pvol)->defect_tbl->defects[BLK2TRK(Blkno)&(HASHSIZE-1)])

/*
 *  Bad block directory entry.
 */
struct bad_blk {			/* bad block directory entry	   */
	struct bad_blk	*next;		/* next entry in congruence class  */
	dev_t		dev;		/* containing physical device	   */
	daddr_t		blkno;		/* bad physical disk address	   */
	unsigned	status: 4;	/* relocation status (see below)   */
	unsigned	relblk: 28;	/* relocated physical disk address */
};

/* bad block relocation status values: */
#define REL_DONE	0		/* software relocation completed    */
#define REL_PENDING	1		/* software relocation in progress  */
#define REL_DEVICE	2		/* device (HW) relocation requested */
#define REL_CHAINED	3		/* relocation blk structure exists  */
#define REL_DESIRED	8		/* relocation desired-hi order bit on*/

/* 
 *  Macros for getting and releasing bad block structures from the 
 *  pool of bad_blk structures They are linked together by their next pointers.
 *  "hd_freebad" points to the head of bad_blk free list 
 *  NOTE:  Code must check if hd_freebad != null before calling
 *         the GET_BBLK macro.
 */
#define GET_BBLK(Bad)	{ \
			(Bad) = hd_freebad; \
			hd_freebad = hd_freebad->next; \
			hd_freebad_cnt--; \
			}

#define REL_BBLK(Bad)	{ \
			(Bad)->next = hd_freebad, \
			hd_freebad = (Bad); \
			hd_freebad_cnt++; \
			}

/*
 *  Macros for accessing these data structures.
 */
#define VG_DEV2LV(Vg, Dev)      ((Vg)->lvols[minor(Dev)])
#define VG_DEV2PV(Vg, Pnum)	((Vg)->pvols[(Pnum)])

#define BLK2PART(Pshift,Lbn)	((ulong)(Lbn)>>(Pshift))
#define PART2BLK(Pshift,P_no)	((P_no)<<(Pshift))
#define PARTITION(Lv,P_no,Mir)	((Lv)->parts[(Mir)]+(P_no))

/*
 *  Mirror bit definitions
 */

#define PRIMARY_MIRROR		001	/* primary mirror mask		*/
#define SECONDARY_MIRROR	002	/* secondary mirror mask	*/
#define TERTIARY_MIRROR		004	/* tertiary mirror mask		*/
#define ALL_MIRRORS		007	/* mask of all mirror bits	*/

/* macro to extract mirror avoidance mask from ext parameter */
#define	X_AVOID(Ext)		( ((Ext) >> AVOID_SHFT) & ALL_MIRRORS )

/*
 *  Macros to select mirrors using avoidance masks:
 *
 *	FIRST_MIRROR	returns first unmasked mirror (0 to 2); 3 if all masked
 *	FIRST_MASK	returns first masked mirror (0 to 2); 3 if none masked
 *	MIRROR_COUNT	returns number of unmasked mirrors (0 to 3)
 *	MIRROR_MASK	returns a mask to avoid a specific mirror (1, 2, 4)
 *	MIRROR_EXIST	returns a mask for non-existent mirrors (0, 4, 6, or 7)
 */
#define	FIRST_MIRROR(Mask)	((0x30102010>>((Mask)<<2))&0x0f)
#define	FIRST_MASK(Mask)	((0x01020103>>((Mask)<<2))&0x0f)
#define	MIRROR_COUNT(Mask)	((0x01121223>>((Mask)<<2))&0x0f)
#define	MIRROR_EXIST(Nmirrors)	((0x00000467>>((Nmirrors)<<2))&0x0f)
#define	MIRROR_MASK(Mirror)	(1<<(Mirror))

/*
 * DBSIZE and DBSHIFT were originally UBSIZE and UBSHIFT from param.h.
 * There were renamed and moved to here to more closely resemble a disk
 * block and not a user block size.
 */
#define DBSIZE		512		  /* Disk block size in bytes	   */
#define DBSHIFT		9		  /* log 2 of DBSIZE		   */

/*
 * LVPAGESIZE and LVPGSHIFT were originally PAGESIZE and PGSHIFT from param.h.
 * There were renamed and moved to here to isolate LVM from the changable
 * system parameters that would have undesirable effects on LVM functionality.
 */
#define LVPAGESIZE	4096		  /* Page size in bytes		   */
#define LVPGSHIFT	12		  /* log 2 of LVPAGESIZE	   */

#define BPPG		(LVPAGESIZE/DBSIZE) /* blocks per page		   */
#define BPPGSHIFT	(LVPGSHIFT-DBSHIFT) /* log 2 of BPPG		   */
#define PGPTRK		32		  /* pages per logical track group */
#define TRKSHIFT	5		  /* log base 2 of PGPTRK	   */
#define LTGSHIFT	(TRKSHIFT+BPPGSHIFT)/* logical track group log base 2*/
#define BYTEPTRK	PGPTRK*LVPAGESIZE /* bytes per logical track group */
#define BLKPTRK		PGPTRK*BPPG	  /* blocks per logical track group*/
#define SIGNED_SHFTMSK  0x80000000	  /* signed mask for shifting to   */
					  /* get page affected mask	   */

#define BLK2BYTE(Nblocks)	((unsigned)(Nblocks)<<(DBSHIFT))
#define BYTE2BLK(Nbytes)	((unsigned)(Nbytes)>>(DBSHIFT))
#define BLK2PG(Blk)		((unsigned)(Blk)>>BPPGSHIFT)
#define PG2BLK(Pageno)		((Pageno)<<(LVPGSHIFT-DBSHIFT))
#define BLK2TRK(Blk)		((unsigned)(Blk)>>(TRKSHIFT+BPPGSHIFT))
#define TRK2BLK(T_no)		((unsigned)(T_no)<<(TRKSHIFT+BPPGSHIFT))
#define PG2TRK(Pageno)		((unsigned)(Pageno)>>TRKSHIFT)

/* LTG per partition */
#define TRKPPART(Pshift)    ((unsigned)(1 << (Pshift - LTGSHIFT)))
/* LTG in the partition */
#define TRK_IN_PART(Pshift, Blk) ( BLK2TRK(Blk) & (TRKPPART(Pshift) - 1) )

/* defines for top half of LVDD */
#define LVDD_HFREE_BB	30	/* high water mark for kernel bad_blk struct*/
#define LVDD_LFREE_BB	15	/* low water mark for kernel bad_blk struct */
#define WORKQ_SIZE	64	/* size of LVs work in progress queue	    */
#define PBSUBPOOLSIZE	16	/* size of pbuf subpool alloc'd by PVs	    */
#define PBSUBPOOLMAX	128	/* max pbuf subpool allowed by PBUFCNT ioctl */
#define HD_ALIGN        (uint)2	/* align characteristics for alloc'd memory */
#define FULL_WORDMASK	3	/* mask for full word (log base 2)	    */
#define BUFCNT_DEFAULT  9       /* default bufcnt value for uphysio */

#define NOMIRROR	0	/* no mirrrors	   */
#define PRIMMIRROR	0	/* primary mirrror */
#define SINGMIRROR	1	/* one mirror	   */
#define DOUBMIRROR	2	/* two mirrors	   */

#define MAXNUMPARTS	3	/* maximum number of parts in a logical part */
#define PVNUMVGDAS	2	/* max number of VGDA/VGSAs on a PV */

/* return codes for LVDD top 1/2 */
#define LVDD_SUCCESS	0	/* general success code			    */
#define LVDD_ERROR	-1	/* general error code			    */
#define LVDD_NOALLOC 	-200	/* hd_init: no able to allocate pool of bufs*/

#endif  /* _H_DASD */
