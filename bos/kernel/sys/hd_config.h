/* @(#)32	1.23  src/bos/kernel/sys/hd_config.h, sysxlvm, bos411, 9428A410j 5/11/94 15:59:39 */

#ifndef  _H_HD_CONFIG
#define  _H_HD_CONFIG

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - hd_config.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>;
#include <sys/dasd.h>;


/*
 *   This include file contains structures used by the logical volume
 *   device driver configuration routines contained in hd_cfg*.c
 *
 *   NOTE:  the return code MUST be the FIRST field in both the
 * 	  user and kernel parmlists and it must be an integer.
 */

#define  HD_KADDLV        20    /* add a logical volume to the kernel */
#define  HD_KADDMPV       21    /* add a missing PV to the kernel */
#define  HD_KADDPV        22    /* add a physical volume to kernel */
#define  HD_KCHGLV        23    /* change a logical volume in kernel */
#define  HD_KCHKLV        24    /* check if a logical volume is closed */
#define  HD_KSETUPVG      25    /* setup VG: define LVS & VGSA/MWCC info */
#define  HD_KDEFVG        26    /* define volume group into the kernel */
#define  HD_KDELLV        27    /* delete a logical volume from kernel */
#define  HD_KDELPV        28    /* delete physical volume from kernel */
#define  HD_KDELVG        29    /* delete a volume group from kernel */
#define  HD_KEXTEND       30    /* extend partitions of LV in kernel */
#define  HD_KQRYVGS       31    /* query all volume groups */
#define  HD_KREDUCE       32    /* reduce partitions of LV in kernel */
#define  HD_KREMPV        33    /* remove a PV (change state to missing) */
#define  HD_KVGSTAT       34    /* get status of a specified volume group */
#define  HD_KADDVGSA      35    /* add a VGSA to a specific pv */
#define  HD_KDELVGSA      36    /* delete a VGSA from a specific pv */
#define  HD_KMISSPV       37    /* mark a PV missing from an existing VG */
#define  HD_MWC_REC       38    /* MWCC recovery operation */              
#define  HD_KREBUILD      39    /* gets info needed to rebuild the vg file */
#define  HD_KCHGQRM       40    /* change the VG quorum count */             

#define  HD_NODEVICE     -1    /* no device defined for this entry */
#define  HD_RSRVDALV      0    /* minor # 0, reserved for desc area LV */
#define  HD_1STUSRLV      1    /* first minor # available for user LV */
#define  HD_NOTVON        1    /* volume group is not varied on */
#define  HD_FULLVON       2    /* volume group is fully varied on */
#define  HD_NOLVSVON      3    /* VG is varied on with LVs not defined */


/* 
 * Return codes returned from LVDD config routines
 */
#define  CFG_FORCEOFF	 -100		/* VG is being forced off */
#define  CFG_INVVGID	 -101		/* Invalid VG Identifier  */
#define  CFG_MAJUSED	 -102		/* Major number already used in devsw */
#define  CFG_SYNCER      -103	  	/* a resync is in progress so we can't
					    reduce or extend */
#define  CFG_INLPRD      -104	        /* reducing the last good copy of a lp*/
#define  CFG_BELOWQRM    -105	        /* will fall below the valid qrm count */
#define  CFG_NONEXISTPV  -106		/* non-existent PV specified */


				

struct kaddlv
	 /* structure which contains information for adding the LVDD
	    data structures into the kernel for a new logical volume */
       {
       int rc;
	 /* return code from LVDD config routine  
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       unsigned short int lv_minor;
	 /* the minor number of the logical volume to be added, deleted,
	    or changed */
       short int lv_options;
	 /* the logical device options for the logical volume */

       /* 2**stripe_exp == stripe size */
       unsigned int  stripe_exp;

       /* stripe width == number of disks across which this lv is striped */
       unsigned int  striping_width;

       char reserved [8];
	 /* reserved space */
       };

struct kaddmpv
	 /* structure which contains information for adding a missing
	    physical volume to the LVDD data structures in the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       short int vg_major;
	 /* the major number of the volume group */
       short int pv_num;
	 /* the number of this PV */
       char reserved [16];
	 /* reserved space */
       };

struct kaddpv
         /* structure which contains information for adding a physical
            volume to the LVDD data structures in the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
         /* the id of the volume group */
       struct pvol * pv_ptr;
         /* pointer to an LVDD physical volume structure which describes
            this physical volume */
       struct part * lp_ptr;
         /* pointer to the LVDD partition structures to be added for this
            physical volume to the fake descriptor area logical volume */
       short pv_num;
         /* the number of this PV */
       short num_desclps;
         /* the number of logical partitions which are to be added to the
            descriptor area logical volume */
       short int quorum_cnt;
	 /* quorum count */
       char reserved [6];
	 /* reserved space */
       };

struct kchglv
	 /* structure which contains information for changing the LVDD
	    data structures for a logical volume in the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       unsigned short int lv_minor;
	 /* the minor number of the logical volume to be added, deleted,
	    or changed */
       short int lv_options;
	 /* the logical device options for the logical volume */
       char i_sched;
	 /* the scheduler policy for the logical volume */
       char reserved [15];
	 /* reserved space */
       };

struct kchgqrm
	 /* structure which contains information for changing the VG's
		quorum count */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       short int quorum_cnt;
	 /* new quorum count */
       char reserved [18];
	 /* reserved space */
	};

struct kchgvgsa
	 /* structure which contains information for adding or deleting
	    a VGSA from a specific PV */                           
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       daddr_t sa_lsns[2];
	 /* Logical Sector Numbers of the VGSAs to be added/deleted, or 0
		if not adding/deleting one of them */
       short int pv_num;
	 /* number of this PV */
       short int quorum_cnt;
	 /* quorum count */
       char reserved [12];
	 /* reserved space */
       };

struct kchklv
	 /* structure which contains information for chechking the LVDD
	    data structures to see if a logical volume is open */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       int open;
	 /* TRUE if the logical volume is OPEN */
       unsigned short int lv_minor;
	 /* the minor number of the logical volume to be checked */
       char reserved [14];
	 /* reserved space */
       };

struct ksetupvg
	 /* structure which contains information for setting up the VG.
	    Define the LV structs and initialize the VGSA/MWCC information.
	  */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       struct lvol ** lv_ptrs;
	 /* a pointer to the array of user area LVDD logical volume
	    structures */
       struct vgsa_area * vgsa_ptr;
	 /* a pointer to the volume group status area */
       daddr_t salsns_ptr;
	 /* a pointer to an array of logical sector number addresses for
	    the VGSA copies */
       struct mwc_rec * mwcc_ptr;
	 /* pointer to list of LTGs which need resync for MWC recovery */
       short int num_ltgs;
	 /* number of logical track groups in the list */
       short int quorum_cnt;
	 /* quorum count */
       };

struct kdefvg
	 /* structure which contains information for adding LVDD data
	    structures which define a volume group into the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct volgrp * vg_ptr;
	 /* a pointer to the user area LVDD volume group structure */
       struct lvol * dalv_ptr;
	 /* a pointer to the user area LVDD logical volume structure for
	    the descriptor area */
       char reserved [28];
	 /* reserved space */
       };

struct kdellv
	 /* structure which contains information for deleting the LVDD
	    data structures for a logical volume from the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       unsigned short int lv_minor;
	 /* the minor number of the logical volume to be added, deleted,
	    or changed */
       char reserved [18];
	 /* reserved space */
       };

struct kdelpv
	 /* structure which contains information for deleting a physical
	    volume from the LVDD data structures in the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       int pv_num;
	 /* the number of this PV */
       short int num_desclps;
	 /* the number of logical partitions in the descriptor area
	    logical volume for this PV's descriptor area */
       short int quorum_cnt;
	 /* quorum count */
       char reserved [12];
	 /* reserved space */
       };

struct kdelvg
	 /* structure which contains information for deleting the LVDD
	    date structures for a volume group from the kernel */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       int lvs_only;
	 /* flag to indicate if the volume group is to be completely
	    varied off or if only logical volumes (other than the LVM
	    volume group reserved area logical volume) are to be varied
	    off (note that the latter leaves the volume group varied on
	    for system management functions only) */
       char reserved [16];
	 /* reserved space */
       };

struct extred_part
	 /* structure which describes a physical partition copy of a
	    logical partition which is being extended or reduced */
       {
       int lp_num;
	 /* the number of the logical partition which is to be extended
	    or reduced */
       int pv_num;
	 /* the number of the physical volume which contains the physical
	    partition copy */
       int start_addr;
	 /* the starting physical sector number of the physical partition
	    copy */
       char mask;
	 /* mask of pps to be reduced */         
       char res[3];
	/* reserved area*/
       };

struct kextred
	 /* structure which contains information for extending or
	    reducing a logical volume */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       unsigned short int lv_minor;
	 /* the minor number of the logical volume which is to be
	    extended or reduced */
       short int copies;
	 /* the maximum number of physical copies of any logical
	    partition */
       int num_lps;
	 /* the number of logical partitions in the logical volume */
       int num_extred;
	 /* the number of physical partitions which are being added to or
	    deleted from the logical volume */
       struct extred_part * extred_part;
	 /* a pointer to the list of structures which describe the
	    physical partitions which are being added or deleted */
       char i_sched;
	 /* the scheduler policy */

       char reserved [3];
	 /* reserved space */
       };

struct kqryvgs
        /* structure which contains information for querying all volume 
           groups */
     {
         int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
         struct queryvgs *queryvgs;
          /* pointer to the buffer to be returned to the user */
	 char reserved [32];
          /* reserved space */
     };

struct kvgstat
	 /* structure which contains information for determining the
	    status of a specified volume group in the kernel LVDD data
	    structures */
       {
       int rc;
	 /* return code from LVDD config routine 
            NOTE:  This rc MUST be the FIRST field in this struct so
                   the copyout in hd_config() can be generic for all structs */
       struct unique_id vg_id;
	 /* the id of the volume group */
       long vg_major;
	 /* the major number of the volume group */
       int status;
	 /* status which indicates if the specified volume group is
	    varied on and what type of vary on */
       char reserved [12];
	 /* reserved space */
       };

struct pvs {
	/* referenced by rebuild struct below */
	dev_t   device;		/* major,minor number of pv */
	daddr_t salsn[2];	/* logical sector numbers of VGSA's */
	short   pvstate;	/* state of physical volume */
};

struct rebuild {
	/* referenced by krebuild struct below */
	long quorum;		/* quorum count of the volume group */
	short int num_desclps;	/* number of lps for VGDA */
	struct pvs pv[MAXPVS];
				/* array of pvs in volume group */
};

struct krebuild {
 	/* structure that has info needed to rebuild the volume group file */
	int rc;			/* return code */
	struct rebuild *rebuild;	
		/* pointer to structure containing rebuild info */
	char reserved[32];
};

struct ddi_info
  /* structure to describe device dependent information which is passed
     to the logical volume device driver configuration routine
     (hd_config) via a system call to sysconfig */
       {
       union
	     {
	     struct kaddlv kaddlv;
	       /* structure which contains input information for the
		  hd_kaddlv routine */
	     struct kaddpv kaddpv;
	       /* structure which contains input information for the
		  hd_kaddpv routine */
	     struct kaddmpv kaddmpv;
	       /* structure which contains input information for the
		  hd_kaddmpv routine */
	     struct kchglv kchglv;
	       /* structure which contains input information for the
		  hd_kchglv routine */
	     struct kchgqrm kchgqrm;
	       /* structure which contains input information for the
		  hd_kchgqrm routine */
	     struct kchgvgsa kchgvgsa;
	       /* structure which contains input information for the
		  hd_kchgvgsa routine */
	     struct kchklv kchklv;
	       /* structure which contains input information for the
		  hd_kchklv routine */
	     struct ksetupvg ksetupvg;
	       /* structure which contains input information for the
		  hd_ksetupvg routine */
	     struct kdefvg kdefvg;
	       /* structure which contains input information for the
		  hd_kdefvg routine */
	     struct kdellv kdellv;
	       /* structure which contains input information for the
		  hd_kdellv routine */
	     struct kdelpv kdelpv;
	       /* structure which contains input information for the
		  hd_kdelpv routine */
	     struct kdelvg kdelvg;
	       /* structure which contains input information for the
		  hd_kdelvg routine */
	     struct kextred kextred;
	       /* structure which contains input information for the
		  hd_kextend and hd_kreduce routines */
	     struct kqryvgs kqryvgs;
	       /* structure which contains input information for the
		  hd_kqryvgs routine */
	     struct kvgstat kvgstat;
	       /* structure which contains input information for the
		  hd_kvgstat routine */
	     struct krebuild krebuild;
	       /* structure which contains info for the user returned
		  from the hd_krebuild routine */
	     } parmlist;
	 /* the device dependent information which was passed to this
	    logical volume device driver configuration routine, where
	    each of the structures in the union represents the device
	    dependent information for the specific configuration routine
	    which is to be called */
       };



/*
 *   Function declarations
 */

#ifndef _NO_PROTO

/*
 *   Function declarations for hd_cfglv.c
 */

int
hd_kaddlv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kaddlv * kaddlv);
  /* pointer to the structure which contains input parameters for
     adding a logical volume in the kernel */

int
hd_kchgqrm (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kchgqrm * kchgqrm);
  /* pointer to the structure which contains input parameters for
     changing the VG's quorum count */

int
hd_kchglv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kchglv * kchglv);
  /* pointer to the structure which contains input parameters for
     changing a logical volume in the kernel */

int
hd_kchklv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kchklv * kchklv);
  /* pointer to the structure which contains input parameters for
    checking if a logical volume is open */

int
hd_kdellv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kdellv * kdellv);
  /* pointer to the structure which contains input parameters for
     deleting a logical volume in the kernel */

int
hd_kextend (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kextred * kextend);
  /* pointer to the structure which contains input parameters for
     extending the partitions of a logical volume in the kernel */

int
hd_kreduce (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kextred * kreduce);
  /* pointer to the structure which contains input parameters for
     reducing the partitions of a logical volume in the kernel */


/*
 *   Function declarations for hd_cfgpv.c
 */

int
hd_kaddmpv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kaddmpv * kaddmpv);
  /* pointer to the structure which contains input parameters for adding
     a missing physical volume to a volume group in the kernel */

int
hd_kaddpv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kaddpv * kaddpv);
  /* pointer to the structure which contains input parameters for adding
     a physical volume to a volume group in the kernel */

int
hd_kcopybb (
struct pvol * kpv_ptr,
  /* pointer to the pvol structure for the physical volume */
struct bad_blk * user_hdrptr,
  /* this is the pointer to the beginning of the linked list of bad block
     structures which are to be copied into the kernel */
struct bad_blk ** ker_hdrptr);
  /* this is the pointer to the variable which will hold the returned
     pointer to the beginning of the linked list of bad block structures
     in the kernel */

int
hd_kdelpv (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kdelpv * kdelpv,
  /* pointer to the structure which contains input parameters for
     deleting a physical volume from a volume group in the kernel */
int cmd);
  /* command flag which indicates is this is for a delete PV or a remove
     PV */

void
hd_free_dtab (
struct pvol * kpv_ptr);		/* ptr to PV structure */


int
hd_kchgvgsa (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kchgvgsa * kchgvgsa, 
  /* pointer to the structure which contains input parameters for
     changing a VGSA for a PV */
int cmd);
  /* command flag which indicates to delete or add a VGSA to this PV */


/*
 *   Function declarations for hd_cfgvg.c
 */

int
hd_config (
dev_t dev,
  /* device code which consists of major and minor number of the volume
     group to be configured */
int cmd,
  /* indicates what function this routine is to perform */
struct uio * uiop);
  /* pointer to the user structure which contains a pointer to the device
     dependent information */

int
hd_kdeflvs (
struct volgrp * kvg_ptr,
   /* ptr to kernel VG struct */
struct lvol ** lv_ptrs);	
   /* ptr to array of user area LV structs */

int
hd_ksetupvg (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct ksetupvg * ksetupvg);
  /* pointer to the structure which contains input parameters for
     setting up the VG with LVs, VGSA & MWCC information */

int
hd_mwc_fprec (
struct mwc_rec *mwcc,		/* mirror write consistence cach */           
int numltgs,			/* number of LTG entries in the mwcc */
struct volgrp * vg);		/* volgrp ptr */

void
hd_disastrec (
struct volgrp *vg,		/* ptr to volume group */
ulong 	       ltg,		/* LTG to be recovered */
int	       lv_minor,	/* LV minor number */
int	      *vgsa_updated);	/* flag set if VGSA is updated */

int
hd_kdefvg (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kdefvg * kdefvg);
  /* pointer to the structure which contains input parameters for
     defining a volume group to the kernel */

int
hd_kdelvg (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kdelvg * kdelvg);
  /* pointer to the structure which contains input parameters for
     deleting a volume group in the kernel */

void
hd_free_lvols (
struct volgrp *kvg_ptr,		/* ptr to volgrp struct */
int lv_start,			/* lv index into lvols array to begin freeing */
int lv_end);			/* lv index into lvols array to end freeing */

int
hd_kqryvgs (
dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */
struct kqryvgs * kqryvgs);
  /* pointer to structure which contains input parameters and a pointer
     to a buffer returned to the user when querying all volume groups
     in the system */

int
hd_krebuild (
dev_t device, 	/* major and minor number of the volume group */
struct krebuild *krebuild);
		/* pointer to hd_krebuild structure */

int
hd_kvgstat (
dev_t device,
  /* device code which consists of the major and minor number */
struct kvgstat * kvgstat);
  /* pointer to structure which contains information for determining the
     status in the kernel of a specified volume group */


#else

/*
 *   Function declarations for hd_cfglv.c
 */

int hd_kaddlv ( );

int hd_kchglv ( );

int hd_kchgqrm ( );

int hd_kchklv ( );

int hd_kdellv ( );

int hd_kextend ( );

int hd_kreduce ( );


/*
 *   Function declarations for hd_cfgpv.c
 */

int hd_kaddmpv ( );

int hd_kaddpv ( );

int hd_kcopybb ( );

int hd_kdelpv ( );

void hd_free_dtab ( );

int hd_kchgvgsa ( );


/*
 *   Function declarations for hd_cfgvg.c
 */

int hd_config ( );

int hd_kdeflvs ( );

int hd_ksetupvg ( );

int hd_mwc_fprec ( );

void hd_disastrec ( );

int hd_kdefvg ( );

int hd_kdelvg ( );

void hd_free_lvols ( );

int hd_kqryvgs ( );

int hd_krebuild ( );

int hd_kvgstat ( );

#endif  /* _NO_PROTO */

#endif  /* _H_HD_CONFIG */
