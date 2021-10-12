/* @(#)75	1.20.1.11  src/bos/kernel/sys/hd.h, sysxlvm, bos411, 9428A410j 6/30/94 14:17:00 */
/*
 *   COMPONENT_NAME: SYSXLVM
 *
 *   FUNCTIONS: BUGVDEF
 *		BUGXDEF
 *		CA_HASH
 *		CA_THASH
 *		CA_VG_WRT
 *		GET_PBUF
 *		GET_PVWAIT
 *		HD_SCHED
 *		PB_CONT
 *		REL_PBUF
 *		REL_PVWAIT
 *		TST_PVWAIT
 *		hd_bldpbuf
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



#ifndef _H_HD
#define _H_HD

#include <sys/errids.h>

/*
 *  LVDD internal macros and extern staticly declared variables.
 */

/* LVM internal defines: */
#define	FAILURE		0	/* must be logic FALSE for 'if' tests	  */
#define	SUCCESS		1	/* must be logic TRUE for 'if' tests	  */
#define MAXGRABLV	16	/* Max number of LVs to grab pbuf structs */
#define MAXSYSVG	3	/* Max number of VGs to grab pbuf structs */
#define	CAHEAD		1	/* move cache entry to head of use list	  */
#define	CATAIL		2	/* move cache entry to tail of use list	  */
#define CA_MISS		0	/* MWC cache miss			  */
#define CA_HIT		1	/* MWC cache hit			  */
#define CA_LBHOLD	2	/* The logical request should hold	  */

/*
 * Following defines are used to communicate with the kernel process
 */
#define LVDD_KP_BADBLK	   0x20000000   /* need more bad_blk structs */
#define LVDD_KP_BB_PIN     0x10000000   /* bb_pbuf->pb_addr xmalloc request */
#define LVDD_KP_ACTMSK	   0x30000000   /* mask of all events */

/*
 * Following defines are used in the b_options of the logical buf struct.
 * They should be reserved in lvdd.h in relationship to the ext parameters
 */
#define	REQ_IN_CACH	0x40000000	/* When set in the lbuf b_options */
					/* the requests is in the mirror  */
					/* write consistency cache	  */
#define REQ_VGSA	0x20000000	/* When set in the lbuf b_options */
					/* it means this is a VGSA write  */
					/* and to use the special sa_pbuf */
					/* in the volgrp structure	  */

/****************************************************************************
 *
 * The following variables are only used in the kernel and therefore are
 * only included if the _KERNEL variable is defined.
 *
 ***************************************************************************/

#ifdef _KERNEL
#include <sys/syspest.h>
#include <sys/lock_def.h>

#ifdef LVDD_PHYS
    Simple_lock  glb_sched_intlock = {0};
#else
    extern Simple_lock  glb_sched_intlock;
#endif /* LVDD_PHYS */

/*
 * Set up a debug level if debug turned on
 */
#ifdef DEBUG
    #ifdef LVDD_PHYS
        BUGVDEF(debuglvl, 0) 
    #else
        BUGXDEF(debuglvl) 
    #endif
#endif

/* head of list of active volgrp structs in the system */
typedef struct 
    {
    lock_t  lock;	/* lock while manipulating list of VG structs */
    struct volgrp  *ptr;	/* ptr to list of varied on VG structs */
    }  hd_vghead_t;

#ifdef LVDD_PHYS
    hd_vghead_t  hd_vghead = {EVENT_NULL, NULL};
#else
    extern hd_vghead_t  hd_vghead;
#endif

/*
 *  pending queue
 *
 *	This is the primary data structure for passing work from
 *	the strategy routines (see hd_strat.c) to the scheduler
 *	(see hd_sched.c) via the mirror write consistency logic.
 *	From this queue the request will go to one of three other
 *	queues.
 *
 *	    1.  cache hold queue - If the request involves mirrors
 *		and the write consistency cache is in flight.
 *		i.e. being written to PVs.
 *
 *	    2.  cache PV queue - If the request must wait for the 
 *		write consistency cache to be written to the PV.
 *
 *	    3.  schedule queue - Requests are scheduled from this
 *		queue.
 *
 *	This queue is only changed within a device driver critical section.
 */
#ifdef LVDD_PHYS
struct hd_queue	pending_Q;
#else
extern struct hd_queue	pending_Q;
#endif

/*
 *  Chain of free and available pbuf structs.
 */
#ifdef LVDD_PHYS
struct pbuf		*hd_freebuf = NULL;
#else
extern struct pbuf	*hd_freebuf;
#endif

/*
 *  Chain of pbuf structs currently allocated and pinned for LVDD use.
 *  Only used at dump time and by crash to find them.
 */
#ifdef LVDD_PHYS
struct pbuf		*hd_dmpbuf = NULL;
#else
extern struct pbuf	*hd_dmpbuf;
#endif

/*
 *  Chain and count of free and available bad_blk structs.
 *  The first open of a VG, really the first open of an LV, will cause
 *  LVDD_HFREE_BB( currently 30 ) bad_blk structs to be allocated and
 *  chained here.  After that when the count gets to LVDD_LFREE_BB(low
 *  water mark, currently 15) the kernel process will be kicked to go
 *  get more up to LVDD_HFREE_BB( high water mark ) more.
 *
 *  *NOTE* hd_freebad_lk is a lock mechanism to keep the top half of the
 *	   driver and the kernel process from colliding.  This would only
 *	   happen if the last request before the last LV closed received
 *	   an ESOFT or EMEDIA( and request was a write ) and the getting of
 *	   a bad_blk struct caused the count to go below the low water
 *	   mark.  This would result in the kproc trying to put more
 *	   structures on the list while hd_close via hd_frefrebb would
 *	   be removing them.
 */
#ifdef LVDD_PHYS
int			hd_freebad_lk = LOCK_AVAIL;
struct bad_blk		*hd_freebad = NULL;
int			hd_freebad_cnt = 0;
#else
extern int		hd_freebad_lk;
extern struct bad_blk	*hd_freebad;
extern int		hd_freebad_cnt;
#endif

/*
 *  Chain of volgrp structs that have write consistency caches that need
 *  to be written to PVs.  This chain is used so all incoming requests
 *  can be scanned before putting the write consistency cache in flight.
 *  Once in flight the cache is locked out and any new requests will have
 *  to wait for all cache writes to finish.
 */
#ifdef LVDD_PHYS
struct volgrp		*hd_vg_mwc = NULL;
#else
extern struct volgrp	*hd_vg_mwc;
#endif

/*
 *  The following arrays are used to allocate mirror write consistency
 *  caches in a group of 8 per page.  This is due to the way the hide
 *  mechanism works only on page quantities.  These two arrays should be
 *  treated as being in lock step.  The lock, hd_ca_lock, is used to 
 *  ensure only one process is playing with the arrays at any one time.
 */
#define	VGS_CA	((MAXVGS + (NBPB - 1)) / NBPB)

#ifdef LVDD_PHYS
lock_t		hd_ca_lock = LOCK_AVAIL;/* lock for cache arrays	   */
char		ca_alloced[VGS_CA];	/* bit per VG with cache allocated */
struct mwc_rec	*ca_grp_ptr[VGS_CA];	/* 1 for each 8 VGs		   */
#else
extern lock_t		hd_ca_lock;
extern char		ca_alloced[];
extern struct mwc_rec	*ca_grp_ptr[];
#endif

#ifdef LVDD_PHYS
int		mwcc_entries = 62;	/* number of MWCC entries	*/
					/* 62 is the max allowed	*/
#else
extern int	mwcc_entries;
#endif
/*
 *  The following variables are used to control the number of pbuf 
 *  structures allocated for LVM use.  It is based on the number of
 *  PVs in varied on VGs.  The first PV gets 64 structures and each
 *  PV thereafter gets 16 more.  The number is reduced only when a 
 *  VG goes inactive. i.e. all it's LVs are closed.
 */
#ifdef LVDD_PHYS
int hd_pbuf_cnt	= 0;		/* Total Number of pbufs allocated	*/

int hd_pbuf_grab = PBSUBPOOLSIZE; /* Number of pbuf structs to allocate	*/
				/* for each active PV on the system	*/

int hd_pbuf_min = PBSUBPOOLSIZE * 4;  /* Number of pbuf to allocate for the */
				      /* first PV on the system		*/

int hd_vgs_opn = 0;		/* Number of VGs opened			*/
int hd_lvs_opn = 0;		/* Number of LVs opened			*/
int hd_pvs_opn = 0;		/* Number of PVs in varied on VGs	*/

int hd_pbuf_inuse = 0;		/* Number of pbufs currently in use	*/

int hd_pbuf_maxuse= 0;		/* Maximum number of pbufs in use during*/
				/* this boot				*/
int done_counter = 0;           /* Number of times B_MORE_DONE was on
				   in b_flags  before reset to 0 
                                   by hd_schedule() */

unsigned int  lvm_bufcnt = BUFCNT_DEFAULT;    /* tunable uphysio bufcnt parm */

#else
extern int hd_pbuf_cnt;
extern int hd_pbuf_grab;
extern int hd_pbuf_min;
extern int hd_vgs_opn;
extern int hd_lvs_opn;
extern int hd_pvs_opn;
extern int hd_pbuf_inuse;
extern int hd_pbuf_maxuse;
extern int done_counter;
extern unsigned int  lvm_bufcnt;
#endif


/*
 * The following are used to update the bad block directory on a disk
 */
#ifdef LVDD_PHYS
struct pbuf	*bb_pbuf;	/* ptr to pbuf reserved for BB dir updating */
struct hd_queue	bb_hld;		/* holding Q used when there is a BB	    */
				/* directory update in progress		    */
unsigned int  pinned_bb_dir_memory = 0; /* size of currently pinned bb dir memory */
#else
extern struct pbuf	*bb_pbuf;
extern struct hd_queue	bb_hld;	
extern unsigned int  pinned_bb_dir_memory;  
#endif

/*
 * The following variables are used to communicate between the LVDD
 * and the kernel process.
 */
#ifdef LVDD_PHYS
unsigned int    init_global_locks = {1};
tid_t		lvm_kp_tid = {0};	/* thread id of the kernel process */
int		lvm_kp_wait = {EVENT_NULL};/* sleep anchor for kproc creation */
Simple_lock	glb_kp_lock = {0};
#else
extern unsigned int     init_global_locks;
extern tid_t		lvm_kp_tid;
extern int		lvm_kp_wait;
extern Simple_lock	glb_kp_lock;
#endif

/*
 *  The following variables are used in an attempt to keep some information
 *  around about the performance and potential bottle necks in the driver.
 *  Currently these must be looked at with crash or the kernel debugger.
 */
#ifdef LVDD_PHYS
ulong	hd_pendqblked	= 0;	/* How many times the scheduling queue	*/
				/* (pending_Q) has been block due to no	*/
				/* pbufs being available.		*/
#else
extern ulong hd_pendqblked;
#endif

/*
 *  The following are used to log error messages by LVDD.  The de_data
 *  is defined as a general 16 byte array, BUT, it's actual use is
 *  totally dependent on the error type.
 */
#define RESRC_NAME	"LVDD"	/* Resource name for error logging	*/
struct hd_errlog_ent {		/* Error log entry structure		*/
    struct err_rec0 id;
    char de_data[16];
};

/* macros to allocate and free pbuf structures */
#define GET_PBUF(PB)    { \
                        (PB) = hd_freebuf;      \
                        hd_freebuf = (struct pbuf *) hd_freebuf->pb.av_forw; \
                        hd_pbuf_inuse++; \
                        if( hd_pbuf_inuse > hd_pbuf_maxuse ) \
                                hd_pbuf_maxuse = hd_pbuf_inuse; \
                        }

#define REL_PBUF(PB)    { \
                        (PB)->pb.av_forw = (struct buf *) hd_freebuf;   \
                        hd_freebuf = (PB); \
                        hd_pbuf_inuse--; \
                        }

/* macros to allocate and free pv_wait structures */
#define GET_PVWAIT(Pvw, Vg)	{ \
			(Pvw) = (Vg)->ca_freepvw; 	\
			(Vg)->ca_freepvw = (Pvw)->nxt_pv_wait; \
			}

#define REL_PVWAIT(Pvw, Vg)	{ \
			(Pvw)->nxt_pv_wait = (Vg)->ca_freepvw; 	\
			(Vg)->ca_freepvw = (Pvw); \
			}

#define TST_PVWAIT(Vg)	((Vg)->ca_freepvw == NULL)

/*
 * Macro to put volgrp ptr at head of the list of VGs waiting to start
 * MWC cache writes
 */
#define CA_VG_WRT( Vg )	{ \
			if( !((Vg)->flags & CA_VGACT) ) \
				{ \
				(Vg)->nxtactvg = hd_vg_mwc; \
				hd_vg_mwc = (Vg); \
				(Vg)->flags |= CA_VGACT; \
				}  \
			}
/*
 * Macro to determine if a physical request should be returned to
 * the scheduling layer or continue(resume).
 */
#define PB_CONT( Pb )	{ \
        if ((Pb->pb_addr == (Pb->orig_addr + Pb->orig_count)) \
            || (Pb->pb.b_flags & B_ERROR)) \
		HD_SCHED( (Pb) ); \
	else \
		hd_resume( (Pb) ); \
    }
/*
 *  HD_SCHED -- invoke scheduler policy routine for this request.
 *
 *	For physical requests it invokes the physical operation end policy.
 */
#define HD_SCHED(Pb)	(*(Pb)->pb_sched)(Pb)


/* define for b_error value (only used by LVDD) */
#define ELBBLOCKED 255		/* this logical request is blocked by	*/
				/* another on in progress		*/

#endif	/* _KERNEL   */

/*
 * Write consistency cache structures and macros
 */

/* cache hash algorithms - returns index into cache hash table */
#define CA_HASH(Lb)	(BLK2TRK((Lb)->b_blkno) & (CAHHSIZE-1))
#define CA_THASH(Trk)	((Trk) & (CAHHSIZE-1))

/*
 * This structure will generally be referred to as part 2 of the cache
 */
struct ca_mwc_mp {	/* cache mirror write consistency memory only part  */

    struct ca_mwc_mp	*hq_next;	/* ptr to next hash queue entry	    */
    char		state;		/* State of entry		    */
    char		pad1;		/* Pad to word 			    */
    ushort		iocnt;		/* Non-zero - io active to LTG	    */
    struct ca_mwc_dp	*part1;		/* Ptr to part 1 entry - ca_mwc_dp  */
    struct ca_mwc_mp	*next;		/* Next memory part struct	    */
    struct ca_mwc_mp	*prev;		/* Previous memory part struct	    */
};

/* ca_mwc_mp state defines */
#define CANOCHG		0x00	/* Cache entry has NOT changed since last   */
				/* cache write operation, but is on a hash  */
				/* queue somewhere			    */
#define	CACHG		0x01	/* Cache entry has changed since last cache */
				/* write operation 			    */
#define	CACLEAN		0x02	/* Cache entry has not been used since last */
				/* clean up operation			    */

/*
 * This structure will generally be referred to as part 1 of the cache
 * In order to stay long word aligned this structure has a 2 byte pad.
 * This reduces the number of cache entries available in the cache.
 */
struct ca_mwc_dp {	/* cache mirror write consistency disk part	*/

	ulong		lv_ltg;		/* LV logical track group	*/
	ushort		lv_minor;	/* LV minor number		*/
	short		pad;
};
#define MAX_CA_ENT	62	/* Max number that will fit in block	*/

/*
 * This structure must be maintained to be 1 block in length(512 bytes).
 * This also implies the maximum number of write consistency cache entries.
 */
struct mwc_rec {	/* mirror write consistency disk record		*/

    struct timestruc_t	b_tmstamp;	/* Time stamp at beginning of block */
    struct ca_mwc_dp	ca_p1[MAX_CA_ENT];/* Reserve 62 part 1 structures   */
    struct timestruc_t	e_tmstamp;	/* Time stamp at end of block	    */
};

/*
 * This structure is used by the MWCM.  It is hung on the PV cache write
 * queues to indicate which lbufs are waiting on any particular PV.  The
 * define controls how much memory to allocate to hold these structures.
 * The algorithm is 3 * CA_MULT * cache size * size of structure.
 */
#define CA_MULT		4		/* pv_wait * cache size multiplier  */
struct pv_wait {
    struct pv_wait	*nxt_pv_wait;	/* next pv_wait structure on chain  */
    struct buf		*lb_wait;	/* ptr to lbuf waiting for cache    */
};

/*
 * LVM function declarations - arranged by module in order by how they occur
 * in said module.
 */
#ifdef _KERNEL
#ifndef _NO_PROTO

/* hd_mircach.c */

extern int hd_ca_ckcach ( 
	register struct buf	*lb,	/* current logical buf struct	*/
	register struct volgrp	*vg,	/* ptr to volgrp structure	*/
	register struct lvol	*lv);	/* ptr to lvol stucture		*/

extern void hd_ca_use (
	register struct volgrp	  *vg,	/* ptr to volgrp structure	*/
	register struct ca_mwc_mp *ca_ent,/* cache entry pointer	*/
	register int	h_t);		/* head/tail flag		*/

extern struct ca_mwc_mp *hd_ca_new (
	register struct volgrp	*vg);	/* ptr to volgrp structure	*/

extern void hd_ca_wrt (void);

extern void hd_ca_wend (
	register struct pbuf *pb);	/* Address of pbuf completed	*/

extern void hd_ca_sked(
	register struct volgrp	*vg,	/* ptr to volgrp structure	*/
	register struct pvol	*pvol);	/* pvol ptr for this PV		*/

extern struct ca_mwc_mp *hd_ca_fnd(
	register struct volgrp	*vg,	/* ptr to volgrp structure	*/
	register struct buf	*lb);	/* ptr to lbuf to find the entry*/
					/* for				*/

extern void hd_ca_clnup(
	register struct volgrp	*vg);	/* ptr to volgrp structure	*/

extern void hd_ca_qunlk(
	register struct volgrp	*vg,	/* ptr to volgrp structure	*/
	register struct ca_mwc_mp  *ca_ent);/* ptr to entry to unlink	*/

extern int hd_ca_pvque(
	register struct buf	*lb,	/* current logical buf struct	*/
	register struct volgrp	*vg,	/* ptr to volgrp structure	*/
	register struct lvol	*lv);	/* ptr to lvol stucture		*/

extern void hd_ca_end (
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_ca_term (
	register struct buf	*lb);	/* current logical buf struct	*/

extern void hd_ca_mvhld (
	register struct volgrp	*vg);	/* ptr to volgrp structure	*/

/* hd_dump.c */

extern int hd_dump (
	dev_t		dev,	/* major/minor of LV			*/
	struct uio	*uiop,	/* ptr to uio struct describing operation*/
	int		cmd,	/* dump command				*/
	char		*arg,	/* cmd dependent - ptr to dmp_query struct*/
	int		chan,	/* not used				*/
	int		ext);	/* not used				*/

extern int hd_dmpxlate(
	register dev_t		dev,	/* major/minor of LV		*/
	register struct uio	*luiop,	/* ptr to logical uio structure	*/
	register struct volgrp	*vg);	/* ptr to VG from device switch table*/

/* hd_top.c */

extern int hd_open(
	dev_t dev,	/* device number major,minor of LV to be opened */
	int flags, 	/* read/write flag	*/
	int chan,	/* not used		*/
	int ext);	/* not used		*/

extern int hd_allocpbuf(void);

extern void hd_pbufdmpq(
	register struct pbuf *pb,		/* new pbuf for chain	*/
	register struct pbuf **qq);		/* Ptr to queue anchor	*/

extern void hd_openbkout(
	int		bopoint,	/* point to start backing out	*/
	struct volgrp	*vg);		/* struct volgrp ptr		*/

extern void hd_backout(
	int		bopoint,/* point where error occurred & need to	*/
				/* backout all structures pinned before	*/
				/* this point				*/
	struct lvol	*lv,	/* ptr to lvol to backout		*/
	struct volgrp	*vg);	/* struct volgrp ptr			*/

extern int hd_close(
	dev_t	dev,	/* device number major,minor of LV to be closed */
	int	chan,	/* not used */
	int	ext);	/* not used */

extern void hd_vgcleanup(
	struct volgrp	*vg);		/* struct volgrp ptr		*/

extern void hd_frefrebb(void);

extern int hd_allocbblk(void);

extern int hd_read(
	dev_t	   dev,		/* num major,minor of LV to be read	    */
	struct uio *uiop,	/* pointer to uio structure that specifies  */
				/* location & length of caller's data buffer*/
	int	   chan,	/* not used */
	int	   ext);	/* extension parameters */

extern int hd_write(
	dev_t	   dev,		/* num major,minor of LV to be written    */
	struct uio *uiop,	/* pointer to uio structure that specifies  */
				/* location & length of caller's data buffer*/
	int	   chan,	/* not used */
	int	   ext);	/* extension parameters */

extern int hd_mincnt(
	struct buf	*bp,	   /* ptr to buf struct to be checked   */
	void		*minparms);/* ptr to ext value sent to uphysio by*/
				   /* hd_read/hd_write.			*/

extern int hd_ioctl(
	dev_t	dev,	/* device number major,minor of LV to be opened */
	int	cmd,	/* specific ioctl command to be performed	*/
	int	arg,  	/* addr of parameter blk for the specific cmd	*/
	int	mode,	/* request origination				*/
	int	chan,	/* not used */
	int	ext);	/* not used */

extern struct mwc_rec * hd_alloca(void);

extern void hd_dealloca(
	register struct mwc_rec *ca_ptr);	/* ptr to cache to free	*/

extern void hd_nodumpvg(
	struct volgrp *);

void hd_dumpvglist( 
	struct volgrp *vg);

/* hd_phys.c */

extern void hd_begin(
	register struct pbuf *pb,	/* physical device buf struct	*/
	register struct pbuf **ready_list,  /* ready request list */
	register struct volgrp *vg);	/* pointer to volgrp struct	*/

extern void hd_end(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_resume(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_ready(
	register struct pbuf *pb, 	/* physical request buf		*/
	register struct pbuf **ready_list);	/* request list 	*/

extern void hd_start(struct pbuf  *ready_list);

extern void hd_gettime(
	register struct timestruc_t *o_time);	/* old time		*/

/* hd_bbrel.c */

extern int hd_chkblk(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_bbend(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_baddone(
	register struct pbuf *pb);	/* physical request to process	*/

extern void hd_badblk(
	register struct pbuf *pb);	/* physical request to process	*/

extern void hd_swreloc(
	register struct pbuf *pb);	/* physical request to process	*/

extern daddr_t hd_assignalt(
	register struct pbuf *pb);	/* physical request to process	*/

extern struct bad_blk *hd_fndbbrel(
	register struct pbuf *pb);	/* physical request to process */

extern void hd_nqbblk(
	register struct pbuf *pb);	/* physical request to process	*/

extern void hd_dqbblk(
	register struct pbuf *pb,	/* physical request to process	*/
	register daddr_t blkno);

/* hd_sched.c */

extern void hd_schedule(
	struct pbuf  **ready_list);

extern void  reschedule(void);

extern int hd_avoid(
	register struct buf	*lb,	/* logical request buf	*/
	register struct volgrp	*vg);	/* VG volgrp ptr	*/

extern void hd_resyncpp(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_freshpp(
	register struct volgrp	*vg,	/* pointer to volgrp struct	*/
	register struct pbuf	*pb);	/* physical request buf		*/

extern void hd_mirread(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_fixup(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_stalepp(
	register struct volgrp	*vg,	/* pointer to volgrp struct	*/
	register struct pbuf *pb);	/* physical device buf struct	*/

extern void hd_staleppe(
	register struct pbuf *pb);	/* physical request buf		*/

extern void hd_xlate(
	register struct pbuf	*pb,	/* physical request buf		*/
	register int		mirror,	/* mirror number		*/
	register struct	volgrp	*vg);	/* VG volgrp ptr		*/

extern int hd_regular(
	register struct buf	*lb,	/* logical request buf		*/
        register struct pbuf  **ready_list,
	register struct volgrp	*vg);	/* volume group stucture	*/

extern void hd_finished(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern int hd_sequential(
	register struct buf	*lb,	/* logical request buf		*/
        register struct pbuf  **ready_list,
	register struct volgrp	*vg);	/* volume group stucture	*/

extern int hd_seqnext(
	register struct pbuf	*pb,	/* physical request buf		*/
	register struct	volgrp	*vg);	/* VG volgrp pointer		*/

extern void hd_seqwrite(
	register struct pbuf *pb);	/* physical device buf struct	*/

extern int hd_parallel(
	register struct buf	*lb,	/* logical request buf		*/
        register struct pbuf  **ready_list,
	register struct volgrp	*vg);	/* volume group stucture	*/

extern void hd_freeall(
	register struct pbuf *q);	/* write request queue		*/

extern int hd_stripe(
        struct buf  *lb,
        struct pbuf  **ready_list,
        struct volgrp  *vg);

extern void hd_stripe_end(struct pbuf *pb);

extern void  hd_stripe_done(struct pbuf  *pb);

extern int hd_stripe_xlate(
    unsigned int  part_exp,         /* 2**part_exp = partition size */
    unsigned int  stripe_exp,       /* 2**stripe_exp = stripe block size */
    unsigned int  striping_width,   /* number of disks stripes span */
    struct part   *parts,           /* partition array of logical volume */
    unsigned int  log_blkno,        /* logical block number to translate */
    dev_t         *device,          /* device on which stripe block resides */
    daddr_t       *block,           /* sector # corresponding to log_blkno */
    struct part   **partition,      /* partition containing stripe block */
    struct pvol   **pvol);          /* ptr to pvol of stripe block */

extern void hd_append(
	register struct pbuf *pb,	/* physical request pbuf	*/
	register struct pbuf **qq);	/* Ptr to write request queue anchor */

extern void hd_nearby(
	register struct pbuf	*pb,	/* physical request pbuf	*/
	register struct buf	*lb,	/* logical request buf		*/
	register int		mask,	/* mirrors to avoid		*/
	register struct volgrp	*vg,	/* volume group stucture	*/
	register struct lvol	*lv);

extern void hd_parwrite(
	register struct pbuf *pb);	/* physical device buf struct	*/

/* hd_strat.c */

extern void hd_strategy(
	register struct buf *lb);	/* input list of logical buf structs */

extern void  hd_initiate(struct buf  *req_list);

extern void  serialize_data_access(struct buf  *lb, struct lvol  *lv);

extern struct buf *hd_reject(
	struct buf	*lb,		/* offending buf structure	*/
	int		errno);		/* error number			*/

extern void hd_quiescevg(
	struct volgrp	*vg);	/* pointer from device switch table	*/

extern void hd_quiet(
	dev_t		dev,	/* number major,minor of LV to quiesce	 */
	struct volgrp	*vg);	/* ptr from device switch table		 */

extern void hd_redquiet(
	dev_t		dev,		/* number major,minor of LV	*/
	struct hd_lvred	*red_lst);	/* ptr to list of PPs to remove	*/

extern void hd_add2pool(
	register struct pbuf *subpool,	/* ptr to pbuf sub pool		*/
	register struct pbuf *dmpq);	/* ptr to pbuf dump queue	*/

extern void hd_deallocpbuf(void);

extern int hd_numpbufs(void);

extern void hd_terminate(
	register struct buf *lb);	/* logical buf struct		*/

extern void hd_unblock(
	register struct buf *next,	/* first request on hash chain	*/
	register struct buf *lb);	/* logical request to reschedule*/

extern void hd_quelb (
	register struct buf	 *lb,	/* current logical buf struct	*/
	register struct hd_queue *que);	/* queue structure ptr		*/

extern int hd_kdis_initmwc(
	struct volgrp  *vg);   		/* volume group pointer */

extern int hd_kdis_dswadd(
	register dev_t		device,	/* device number of the VG      */
	register struct devsw  	*devsw);/* address of the devsw entry	*/

extern int hd_kdis_chgqrm(
	struct volgrp 	*vg, 		/* volume group pointer */
	short 		newqrm);	/* new quorum count */

extern int hd_kproc(void);

extern struct bad_blk  *get_bblk();

extern void  rel_bblk(struct bad_blk  *bb_ptr);

/* hd_vgsa.c */

extern int hd_sa_strt(
	register struct pbuf	*pb,	/* physical device buf struct	   */
	register struct volgrp	*vg,	/* volgrp pointer		   */
	register int		type);	/* type of request		   */

extern void hd_sa_wrt(
	register struct volgrp	*vg);	/* volgrp pointer		   */

extern void hd_sa_iodone(
	register struct buf	*lb);	/* ptr to lbuf in VG just completed */

extern void hd_sa_cont(
	register struct volgrp	*vg,	/* volgrp pointer		    */
	register int	sa_updated);	/* ptr to lbuf in VG just completed */

extern void hd_sa_hback(
	register struct pbuf	*head_ptr,/* head of pbuf list		    */
	register struct pbuf	*new_pbuf);/* ptr to pbuf to append to list */

extern void hd_sa_rtn(
	register struct pbuf	*head_ptr,/* head of pbuf list		    */
	register int	err_flg);	/* if true return requests with	    */
					/* ENXIO error			    */

extern int hd_sa_whladv(
	register struct volgrp	*vg,	/* volgrp pointer		    */
	register int	c_whl_idx);	/* current wheel index		    */

extern void hd_sa_update(
	register struct volgrp	*vg);	/* volgrp pointer		    */

extern int hd_sa_qrmchk(
	register struct volgrp	*vg);	/* volgrp pointer		    */

extern int hd_sa_config(
	register struct volgrp	*vg,	/* volgrp pointer		    */
	register int		type,	/* type of hd_config request	    */
	register caddr_t	arg);	/* ptr to arguments for the request */

extern int hd_sa_onerev(
	register struct volgrp	*vg,	/* volgrp pointer		    */
	register struct pbuf	*pv, 	/* ptr pbuf structure  		    */
	register int		type);	/* type of hd_config request	    */

extern void hd_bldpbuf (
	register struct pbuf    *pb,	/* ptr to pbuf struct		*/
	register struct pvol	*pvol,	/* target pvol ptr		*/
	register int		type,	/* type of pbuf to build	*/
	register caddr_t	bufaddr,/* data buffer address - system	*/
	register unsigned	cnt,	/* length of buffer		*/
	register struct xmem	*xmem,	/* ptr to cross memory descripto*/
	register void		(*sched)());/* ptr to function ret void */

extern int hd_extend (
	struct sa_ext *saext);		/* ptr to structure with extend info */

extern void hd_reduce (
	struct sa_red *sared,		/* ptr to structure with reduce info */
	struct volgrp *vg);		/* ptr to volume group structure */

/* hd_bbdir.c */

extern void hd_upd_bbdir(
	register struct pbuf *pb);	/* physical request to process */

extern void hd_bbdirend(
	register struct pbuf *vgpb);	/* ptr to VG bb_pbuf		*/

extern void hd_bbdirop( void );

extern int hd_bbadd(
	register struct pbuf *vgpb);	/* ptr to VG bb_pbuf		*/

extern int hd_bbdel(
	register struct pbuf *vgpb);	/* ptr to VG bb_pbuf		*/

extern int hd_bbupd(
	register struct pbuf *vgpb);	/* ptr to VG bb_pbuf		*/

extern void hd_chk_bbhld( void );

extern void hd_bbdirdone(
	register struct pbuf *origpb);	/* physical request to process */

extern void hd_logerr(
	register unsigned	id,	/* original request to process	*/
	register ulong		dev,	/* device number		*/
	register ulong		arg1,
	register ulong		arg2);

extern unsigned int  max_bb_entries(void);
extern int  alloc_mwc_mem(struct volgrp  *vg);


#else

/*	See above for description of call arguments	*/

/* hd_mircach.c */

extern int		hd_ca_ckcach ();
extern void		hd_ca_use ();
extern struct ca_mwc_mp	*hd_ca_new ();
extern void		hd_ca_wrt ();
extern void		hd_ca_wend ();
extern void		hd_ca_sked ();
extern struct ca_mwc_mp *hd_ca_fnd ();
extern void		hd_ca_clnup ();
extern void		hd_ca_qunlk ();
extern int		hd_ca_pvque ();
extern void		hd_ca_end ();
extern void		hd_ca_term ();
extern void		hd_ca_mvhld ();

/* hd_dump.c */

extern int		hd_dump ();
extern int		hd_dmpxlate ();

/* hd_top.c */

extern int		hd_open ();
extern int		hd_allocpbuf ();
extern void		hd_pbufdmpq ();
extern void		hd_openbkout ();
extern void		hd_backout ();
extern int		hd_close ();
extern int		hd_vgcleanup ();
extern void		hd_frefrebb ();
extern int		hd_allocbblk ();
extern int		hd_read ();
extern int		hd_write ();
extern int		hd_mincnt ();
extern int		hd_ioctl ();
extern struct mwc_rec	*hd_alloca ();
extern void		hd_dealloca ();
extern void		hd_nodumpvg ();
extern void 		hd_dumpvglist();

/* hd_phys.c */

extern void		hd_begin ();
extern void		hd_end ();
extern void		hd_resume ();
extern void		hd_ready ();
extern void		hd_start ();
extern void		hd_gettime ();

/* hd_bbrel.c */

extern int		hd_chkblk ();
extern void		hd_bbend ();
extern void		hd_baddone ();
extern void		hd_badblk ();
extern void		hd_swreloc ();
extern daddr_t		hd_assignalt ();
extern struct bad_blk	*hd_fndbbrel ();
extern void		hd_nqbblk ();
extern void		hd_dqbblk ();

/* hd_sched.c */

extern void		hd_schedule ();
extern int		hd_avoid ();
extern void		hd_resyncpp ();
extern void		hd_freshpp ();
extern void		hd_mirread ();
extern void		hd_fixup ();
extern void		hd_stalepp ();
extern void		hd_staleppe ();
extern void		hd_xlate ();
extern int		hd_regular ();
extern void		hd_finished ();
extern int		hd_sequential ();
extern int		hd_seqnext ();
extern void		hd_seqwrite ();
extern int		hd_parallel ();
extern int              hd_stripe();
extern void 		hd_stripe_end();
extern void             hd_stripe_done();
extern int              hd_stripe_xlate();
extern void		hd_freeall ();
extern void		hd_append ();
extern void		hd_nearby ();
extern void		hd_parwrite ();

/* hd_strat.c */

extern void		hd_strategy ();
extern void		hd_initiate ();
extern struct buf	*hd_reject ();
extern void		hd_quiescevg ();
extern void		hd_quiet ();
extern void		hd_redquiet ();
extern void		hd_add2pool ();
extern void		hd_deallocpbuf ();
extern int		hd_numpbufs ();
extern void		hd_terminate ();
extern void		hd_unblock ();
extern void		hd_quelb ();
extern void		hd_kdis_dswadd ();
extern void		hd_kdis_initmwc ();
extern int		hd_kdis_chgqrm ();
extern int		hd_kproc ();
extern struct bad_blk  	*get_bblk();
extern void  		rel_bblk();

/* hd_vgsa.c */

extern int		hd_sa_strt ();
extern void		hd_sa_wrt ();
extern void		hd_sa_iodone ();
extern void		hd_sa_cont ();
extern void		hd_sa_hback ();
extern void		hd_sa_rtn ();
extern int		hd_sa_whladv ();
extern void		hd_sa_update ();
extern int		hd_sa_qrmchk ();
extern int		hd_sa_config ();
extern void		hd_bldpbuf ();
extern int		hd_extend ();
extern void		hd_reduce ();
extern void		hd_sa_onerev ();

/* hd_bbdir.c */

extern void 		hd_upd_bbdir ();
extern void 		hd_bbdirend ();
extern void		hd_bbdirop ();
extern int 		hd_bbadd ();
extern int 		hd_bbdel ();
extern int 		hd_bbupd ();
extern void 		hd_chk_bbhld ();
extern void 		hd_bbdirdone ();
extern void		hd_logerr ();
extern unsigned int	max_bb_entries();
extern int              alloc_mwc_mem();

#endif	/* _NO_PROTO */
#endif	/* _KERNEL   */

#endif /* _H_HD */
