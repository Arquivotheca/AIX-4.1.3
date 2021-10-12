/* @(#)33	1.3  src/bos/kernel/sys/vgsa.h, sysxlvm, bos411, 9428A410j 4/5/91 16:06:36 */
#ifndef _H_VGSA
#define _H_VGSA

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 33
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <sys/dasd.h>

/*
 *  LVDD internal macros and defines used by Volume Group Status
 *  Area(VGSA) logic.
 */

#define	RTN_ERR		1		/* Return requests from the VGSA*/
					/* wheel with ENXIO errors 	*/
#define	RTN_NORM	0		/* Return requests from the VGSA*/
					/* wheel without explicitly 	*/
					/* turning on the B_ERROR flag	*/

#define	VGSA_BLK	8		/* VGSA length in disk blocks	*/
#define	VGSA_SIZE	(VGSA_BLK * DBSIZE) /* VGSA length in bytes	*/
#define	VGSA_BT_PV	127		/* VGSA bytes per PV		*/

/*
 * This structure limits the number of Physical Partitions(PP) that can be
 * present in the VG to 32,512.  The stalepp portion is divided equally
 * between the 32 possible PVs of the VG.  This gives each PV 127 bytes
 * or 1016 PPs.
 *
 * NOTE: If any of the constants change in this structure the version 
 *	 number in the VGDA and the library initialization routines 
 *	 will have to change to adjust older versions to the version
 *	 the driver is compiled with.  Also, if any constants change
 *	 the length of this structure will change checks must be made
 *	 to ensure that is okay and expected.
 */

struct vgsa_area {

    struct timestruc_t	b_tmstamp;	/* Beginning time stamp		*/
			/* Bit per PV	*/
    ulong		pv_missing[(MAXPVS + (NBPL - 1)) / NBPL];
			/* Stale PP bits	*/
    uchar		stalepp[MAXPVS][VGSA_BT_PV];
    char		pad2[12];	/* Padding			*/
    struct timestruc_t	e_tmstamp;	/* Ending time stamp		*/
};

/*
 * Macros used to set/clear/test pv_missing and stalepp bits in a vgsa_area
 * struct.  The ptr argument is assumed to be a ptr to the vgsa_area
 * structure.  All other arguments are assumed to be zero relative.
 * This allows LVM library functions to use these macros.
 *
 * NOTE these macros will not work if the max number of PVs per VG is 
 * greater than 32.
 */

#define	SETSA_PVMISS(Ptr, Pvnum)	\
		((Ptr)->pv_missing[(Pvnum)/NBPL] |= (1 << (Pvnum)))
#define	CLRSA_PVMISS(Ptr, Pvnum)	\
		((Ptr)->pv_missing[(Pvnum)/NBPL] &= (~(1 << (Pvnum))))
#define	TSTSA_PVMISS(Ptr, Pvnum)	\
		((Ptr)->pv_missing[(Pvnum)/NBPL] & (1 << (Pvnum)))

#define	SETSA_STLPP(Ptr, Pvnum, Pp)	\
		((Ptr)->stalepp[(Pvnum)][(Pp)/NBPB] |= (1 << ((Pp) % NBPB)))

#define	CLRSA_STLPP(Ptr, Pvnum, Pp)	\
		((Ptr)->stalepp[(Pvnum)][(Pp)/NBPB] &= (~(1 << ((Pp) % NBPB))))

#define	XORSA_STLPP(Ptr, Pvnum, Pp)	\
		((Ptr)->stalepp[(Pvnum)][(Pp)/NBPB] ^= (1 << ((Pp) % NBPB)))

#define	TSTSA_STLPP(Ptr, Pvnum, Pp)	\
		((Ptr)->stalepp[(Pvnum)][(Pp)/NBPB] & (1 << ((Pp) % NBPB)))

/*
 * Macros used to set/retieve the logical sector number and sequence number 
 * associated with each VGSA.
 */

#define	GETSA_LSN(Vg, Idx)	\
		((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].lsn)

#define	SETSA_LSN(Vg, Idx, Newlsn)	\
		((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].lsn = (Newlsn))

#define	GETSA_SEQ(Vg, Idx)	\
		((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].sa_seq_num)

#define	SETSA_SEQ(Vg, Idx, Seq)	\
		(((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].sa_seq_num) = (Seq))

#define	NUKESA(Vg, Idx)	\
		((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].nukesa)

#define	SET_NUKESA(Vg, Idx, Flag)	\
		((Vg)->pvols[(Idx)>>1]->sa_area[(Idx)&1].nukesa = (Flag))

/*
 * The following structures are used by the config routines to pass
 * information to the hd_sa_config() function for stale/fresh PP and
 * install/delete PV processing.  A pointer to an array of these
 * structures is passed as an argument.
 */

/*
 * An array of these structures is terminated with both the pvnum and pp 
 * equalling -1.
 */
struct cnfg_pp_state {
    short	pvnum;		/* PV number the PP is on		*/
    short	pp;		/* PP number to mark stale/fresh	*/
    int         ppstate;	/* state to mark PP stale/fresg         */
};

/*
 * passed in as arg when a CNFG_EXT request is done.
*/
struct sa_ext{
    struct lvol *klv_ptr;	/* ptr to lvol struct being extended */
    short	nparts;		/* number of copies of the lv */
    char	isched;		/* scheduling policy for the lv */
    char	res;		/* padding */
    ulong	nblocks;	/* length in blocks of the lv */
    struct part **new_parts; 	/* ptr to new part struct list */
    int		old_numlps;	/* old number of logical partitions on lv */
    int         old_nparts;	/* previous number of partitions on lv */
    int 	error;		/* error to return to library layer */
    struct cnfg_pp_state *vgsa;	/* ptr to pp info structure */
};

/*
 * passed in as arg when a CNFG_RED request is done
 */

struct sa_red {
   struct lvol *lv;		/* ptr to lvol struct being reduced */
   short 	nparts;		/* number of copies of the lv */
   char		isched;		/* scheduling policy for the lv */
   char         res;		/* reserved area */
   ulong	nblocks;	/* length in blocks of the lv */
   struct part	**newparts; 	/* ptr to new part struct list */
   unsigned short min_num;	/* minor number of logical volume */
   int		numlps;		/* number of lps on lv after reduction */
   int		numred;		/* number of pps being reduced */
   int 	error;			/* error to return to library layer */
   struct extred_part *list;	/* list of pps to reduce */
};

/*
 * install PV information for VGSA config routine
 */
struct cnfg_pv_ins {
    struct pvol *	pvol;		/* PV to install or remove        */
    short		qrmcnt;		/* new VG quroum count 		  */
    short		pv_idx;		/* index into vg's pvol array 	  */
};

/*
 * delete PV information for VGSA config routine.  Also used for remove PV
 * and missing PV. (qrmcnt will not be used for missing PV)
 */
struct cnfg_pv_del {
    struct pvol *	pv_ptr; /* pointer to pvol struct to remove */
    struct part *	lp_ptr; /* pointer to DALV's LP struct to zero */
    short 		lpsize; /* size of DALV's LP */          
    short		qrmcnt; /* VG's new quorum cnt once this PV is deleted*/
};

/*
 * information to add/delete a VGSA from a PV
 */
struct cnfg_pv_vgsa {
    struct pvol *	pv_ptr; /* pointer to pvol struct to remove */
    daddr_t 		sa_lsns[2]; /* LSNs for VGSAs added or 0 if deleted or
					if a copy not being added */
    short		qrmcnt; /* VG's new quorum cnt once this PV is deleted*/
};

/*
 * The following defines are used by the VGSA write operations.  These
 * defines indicate what action the pbuf is requesting.  It is stored
 * in the pb_type field of the pbuf.
 */
#define	SA_PVMISSING	1	/* PV missing type pbuf			*/
#define	SA_STALEPP	2	/* Stale PP type pbuf			*/
#define	SA_FRESHPP	3	/* Fresh PP type pbuf			*/
#define	SA_CONFIGOP	4	/* hd_config function type pbuf		*/
#define	SA_PVREMOVED	5	/* PV removed type pbuf          	*/

/*
 * The following defines are used by the config routines to set up the
 * the cnfg_pp_state fields 
 */

#define STALEPP 	1
#define FRESHPP 	0
#define CNFG_STOP	-1
#define CNFG_NEWCOPY	-1
#define ROLLWHEEL      1
#define DRAINLV        2
#define NOACTION       0
#define RM_ALL_ACTIVE_PPS 8

#endif /* _H_VGSA */
