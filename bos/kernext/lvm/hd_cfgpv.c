static char sccsid[] = "@(#)31	1.14  src/bos/kernext/lvm/hd_cfgpv.c, sysxlvm, bos411, 9428A410j 5/11/94 15:41:07";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 31
 *
 * FUNCTIONS: hd_kaddmpv
 *	      hd_kaddpv
 *	      hd_kcopybb
 *	      hd_kdelpv
 *	      hd_free_dtab
 *	      hd_kchgvgsa
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


#include <sys/sysmacros.h>
#include <sys/sleep.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/buf.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/types.h>
#include <sys/uio.h> 
#include <sys/malloc.h> 
#include <errno.h> 
#include <sys/dasd.h> 			/* LVDD data structures */
#include <lvm.h>			/* LVM Library interfaces */
#include <sys/vgsa.h> 			/* LVDD VGSA data structures */
#include <sys/hd.h> 			/* LVDD interface */
#include <sys/hd_config.h>		/* LVDD config structures */	
#include <sys/lock_alloc.h>
#include <sys/lockname.h>


/***********************************************************************
 *                                                                     *
 * NAME:  hd_kaddmpv                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *     Add a pvol structure into the kernel for a PV that is missing   *
 *     during the varyon vg procedure.                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kaddmpv                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A physical volume structure for a missing physical volume has   *
 *     been added into the kernel data structures during the 	       *
 *     varyonvg procedure.					       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   lockl                                                             *
 *   unlockl                                                           *
 *   devswqry                                                          *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kaddmpv (

dev_t device,				/* device number of VG to be updated */
struct kaddmpv * kaddmpv)		/* missing PV information */

{ 

struct volgrp * kvg_ptr;	/* kernel VG structure */
struct pvol * kpv_ptr;		/* position to add PV struct to kernel PVs */
int lrc;			/* lockl return code */
int retcode;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kaddmpv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kaddmpv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/* allocate kernel space for this PV structure */
kpv_ptr = (struct pvol *) xmalloc ((uint) sizeof (struct pvol), HD_ALIGN,
				   pinned_heap);
if (kpv_ptr == NULL)
{
    if (lrc == LOCK_SUCC) 
	unlockl(&(kvg_ptr -> vg_lock));
    return (ENOMEM);
}

/* initialize PV structure & VG structure values for this PV */
bzero ((caddr_t) kpv_ptr, sizeof (struct pvol));
kpv_ptr -> pvstate = PV_MISSING;
kpv_ptr -> vg_num = (short int) kaddmpv -> vg_major;
kpv_ptr -> pvnum = kaddmpv -> pv_num - 1;

if (kvg_ptr -> open_count != 0)		/* if any open LVs in the VG */
    hd_pvs_opn++;
kvg_ptr -> pvols [kaddmpv -> pv_num - 1] = kpv_ptr;

/* Error logging of the missing PV done in library */
/* unlock VG structure */
if (lrc == LOCK_SUCC) 
   unlockl(&(kvg_ptr -> vg_lock));

return (LVDD_SUCCESS);

}


/***********************************************************************
 *                                                                     *
 * NAME:  hd_kaddpv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD physical volume structure from user  *
 *   space to kernel space for a physical volume which is being added  *
 *   to the kernel data structures for this volume group.              *
 *   logical partition structures which describe the physical          *
 *   Partitions from this PV which are a part of the descriptor area   *
 *   logical volume are copied in and the VGSA wheel is updated to     *
 *   know about this PV if this is not called during the varyon VG     *
 *   process.                                                          *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kaddpv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A physical volume structure and logical partition structures    *
 *     for the descriptor logical volume have been added into the      *
 *     kernel data structures and if this was not a varyon, the        *
 *     WHEEL was updated to know about this PV.                        *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   lockl                                                             *
 *   unlockl                                                           *
 *   devswqry                                                          *
 *   fp_opendev                                                        *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kaddpv (

dev_t device,		/* device number of the VG to be updated */
struct kaddpv * kaddpv) /* adding PV information */

{ 

struct volgrp * kvg_ptr;	/* kernel VG struct */
struct pvol * kpv_ptr;		/* position to add PV struct to kernel PVs */
struct pvol * old_kpv;		/* save old pvol struct if one exists */
struct pvol * tmp_pv;		/* tmp pvol structure */
struct lvol * klv_ptr;		/* beginning of kernel LV list */
struct part * kpp_ptr;		/* position for new PV's lv0 partition */
struct part * kpp_tmp_ptr;	/* tmp position ptr for new PV's lv0 partition */
struct bad_blk * curbb_ptr;	/* current bad block pointer */
struct defect_tbl u_defect_tbl;	/* kernel defects table for new PV */
short int bb_index;		/* index for bad block table */
short int hash_index;		/* index for bad block hash chain */
short int lp_index;		/* index for LP lists */
struct cnfg_pv_ins arg;		/* arguments passed to hd_sa_config */
int numvgsas;			/* number of VGSAs on this PV */
int lrc;			/* lockl return code */
long size;
int retcode;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kaddpv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kaddpv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/*
 * verify that this VG is not currently being forced off
 */
if (kvg_ptr -> flags & VG_FORCEDOFF) {
    if (lrc == LOCK_SUCC) 
	unlockl(&(kvg_ptr -> vg_lock));
    kaddpv -> rc = CFG_FORCEOFF;
    return (EINVAL);
}
   

old_kpv = kvg_ptr->pvols[kaddpv->pv_num - 1]; /* save old pvol pointer */

/*
 * Allocate a new pvol structure 
 */
kpv_ptr = (struct pvol *) xmalloc ((uint) sizeof (struct pvol),
				       HD_ALIGN, pinned_heap);
if (kpv_ptr == NULL)
{
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (ENOMEM);
}

/*
 * bring PV and defect table info 
 * into kernel space
 */
retcode = copyin (kaddpv -> pv_ptr, kpv_ptr, sizeof (struct pvol));
if (retcode != LVDD_SUCCESS)
{
    /*
     * backout all updates :  free pvol structure & unlock VG
     */
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (EFAULT);
}
retcode = copyin (kpv_ptr -> defect_tbl, &u_defect_tbl,
		  sizeof (struct defect_tbl));
if (retcode != LVDD_SUCCESS)
{
    /*
     * backout all updates :  free pvol structure & unlock VG
     */
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (EFAULT);
}

/* 
 * allocate & initialize PVs defects table 
 */
kpv_ptr -> defect_tbl = (struct defect_tbl *) xmalloc ((uint) sizeof
			(struct defect_tbl), HD_ALIGN, pinned_heap);
if (kpv_ptr -> defect_tbl == NULL)
{
    /*
     * backout all updates :  free pvol structure & unlock VG
     */
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (ENOMEM);
}
bzero ((caddr_t) kpv_ptr -> defect_tbl, sizeof (struct defect_tbl));

/*
 * loop thru defects table & copy any bad block entries
 * from user to kernel space 
 */
for (hash_index = 0; hash_index < HASHSIZE; hash_index = hash_index + 1)
{
   if (u_defect_tbl.defects [hash_index] != NULL)
   {
      retcode = hd_kcopybb (kpv_ptr, u_defect_tbl.defects [hash_index],
		      &(kpv_ptr -> defect_tbl -> defects [hash_index]));
      if (retcode != LVDD_SUCCESS)
      {
	  /*
    	   * backout all updates: free defects table, free pvol struct, 
 	   * unlock VG
	   */
	  hd_free_dtab(kpv_ptr);	
	  assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
	  if (lrc == LOCK_SUCC) 
	      unlockl(&(kvg_ptr -> vg_lock));
	  return (retcode);
      }
   } 
} 
/* 
 * Setup the Descriptor Area logical volume's logical partitions that
 * will reside on this PV.   
 */
klv_ptr = (struct lvol *) kvg_ptr -> lvols [HD_RSRVDALV];
size = kaddpv->num_desclps * sizeof (struct part);

/* point to position for this PV's DALV LP (determined by PV number) */
kpp_ptr = (struct part *) ((caddr_t) klv_ptr -> parts [PRIMMIRROR] +
			    (kaddpv->pv_num - 1) * size);
retcode = copyin (kaddpv -> lp_ptr, kpp_ptr, size);
if (retcode != LVDD_SUCCESS)
{ 
    /*
     * backout all updates: zero DALV's LP, free defects table,
     * free pvol struct & unlock VG
     */
    bzero(kpp_ptr, size);
    hd_free_dtab(kpv_ptr);	
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (EFAULT);
}

/* vg->pvols[] entries are either NULL or point to a pvol structure which
   represents either a present or missing disk.  Disks added by this function
   are either new (NULL entry) or restored (previously missing, not NULL). */

/* if restoring a previously missing disk */
if (old_kpv != NULL)
	/* if previously missing disk left open */
	if (old_kpv->fp != NULL)
	{
		fp_close(old_kpv->fp);
		old_kpv->fp = NULL;
	}

/* setup the LP struct for each DALV LP for this PV */

tmp_pv = (old_kpv == NULL ? kpv_ptr : old_kpv);
/* setup tmp kpp_ptr */
kpp_tmp_ptr=kpp_ptr;
for (lp_index = 0; lp_index < kaddpv->num_desclps; lp_index++)
{
	kpp_tmp_ptr->pvol = tmp_pv;
	kpp_tmp_ptr = (struct part *) ((caddr_t) kpp_tmp_ptr + sizeof (struct part));
}

/* 
 * open the PV
 */
retcode = fp_opendev (kpv_ptr -> dev, FWRITE, NULL, NULL,
		      &(kpv_ptr -> fp));
if (retcode != LVDD_SUCCESS) 
{
    /*
     * backout all updates: zero DALV's LP, free defects table,
     * free pvol struct & unlock VG
     */
    bzero(kpp_ptr, size);
    hd_free_dtab(kpv_ptr);	
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS);
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    return (ENOTREADY);				/* device not ready */
}


/* 
 * With interrupts disabled, update the new pvol pointer for this VG
 * & if NOT varying on the VG, setup VGSA WHEEL info.
 */
arg.pvol = kpv_ptr;
arg.qrmcnt = kaddpv -> quorum_cnt;
arg.pv_idx = kaddpv -> pv_num - 1;

retcode = hd_sa_config (kvg_ptr, HD_KADDPV,(caddr_t) &arg);
if (retcode == FAILURE) 
{
    /* NO backout needed (or wanted) here since it will get
     * backed out when the whole VG is cleaned up as part of the
     * varyoff procedure.  Also, it could be that this cleanup
     * procedure has already completed, by the time we get back
     * to here.
     */
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr -> vg_lock));
    kaddpv -> rc = CFG_FORCEOFF;
    return (EINVAL);
}

/*
 * go get more pbufs for this PV to use, if needed
 */
(void) hd_allocpbuf();


/* Error log that a previously missing PV has returned */
hd_logerr( (unsigned)ERRID_LVM_MISSPVRET, (ulong)(kpv_ptr->dev),
	(ulong)0, (ulong)0 );


if (lrc == LOCK_SUCC) 
    unlockl(&(kvg_ptr -> vg_lock));
return (LVDD_SUCCESS);

} 




/***********************************************************************
 *                                                                     *
 * NAME:  hd_kcopybb                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies a linked list of bad block structures from    *
 *   user space to kernel space.                                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     kpv_ptr                                                         *
 *     user_curptr                                                     *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *ker_hdrptr                                                     *
 *                                                                     *
 *     The ker_hdrptr points to the linked list of bad block           *
 *     structures copied into the kernel.                              *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   xmalloc                                                           *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kcopybb (

struct pvol * kpv_ptr,		/* ptr to pvol struct for this PV */
struct bad_blk * user_curptr,	/* ptr to beg of user area bad block list */
struct bad_blk ** ker_hdrptr)	/* ptr to beg of bad block list returned */

{

struct bad_blk * ker_curptr;	/* current bad block entry in kernel area list*/
struct bad_blk * user_nextptr;	/* next bad block entry in user area list */
int retcode;

/* 
 * allocate kernel space for bad block list 
 */
*ker_hdrptr = (struct bad_blk *) xmalloc ((uint) sizeof (struct bad_blk),
					  HD_ALIGN, pinned_heap);
if (*ker_hdrptr == NULL)
    return (ENOMEM);

ker_curptr = *ker_hdrptr;

/*
 * loop thru bad block list:  copyin data for this entry and then
 * allocate the next entry.
 */
do
{
    retcode = copyin (user_curptr, ker_curptr, sizeof (struct bad_blk));
    if (retcode != LVDD_SUCCESS) {
	ker_curptr -> next = NULL;	/* reinit this for backout routine */
        return (EFAULT);
    }
    user_nextptr = ker_curptr -> next;
    if (user_nextptr != NULL)   /* if another entry, allocate space */
    {
        ker_curptr -> next = (struct bad_blk *) xmalloc ((uint) sizeof
			      (struct bad_blk), HD_ALIGN, pinned_heap);
        if (ker_curptr -> next == NULL)
	    return (ENOMEM);
    }
    user_curptr = user_nextptr;
    ker_curptr = ker_curptr -> next;
}
while (user_curptr != NULL);

return (LVDD_SUCCESS);

}





/***********************************************************************
 *                                                                     *
 * NAME:  hd_kdelpv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine deletes a physical volume from the list of LVDD      *
 *   physical volume structures.                                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kdelpv                                                          *
 *     cmd                                                             *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The entry in the list of LVDD physical volume structures for    *
 *     the physical volume which is being deleted is initialized to    *
 *     indicate that there is no physical volume device for this PV    *
 *     number.                                                         *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   bzero                                                             *
 *   xmfree                                                            *
 *   devswqry                                                          *
 *   lockl                                                             *
 *   unlockl                                                           *
 *   pin                                                               *
 *   hd_sa_config                                                      *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kdelpv (

dev_t device,		/* device number of the VG to be updated */
struct kdelpv * kdelpv,	/* delete PV information */
int cmd) 		/* flag: deleting, removing or marking the PV missing */


{ 

struct volgrp * kvg_ptr;	/* kernel VG struct PV to delete is in */
struct lvol * klv_ptr;		/* list of LV structs for this VG */
struct part * kpp_ptr;		/* list of LP struct for Descr Area LV */
struct pvol * kpv_ptr;		/* list of PV structs for this VG */
register int i_prty;		/* saved interrupt priority */
long lpart_num;			/* PV's LP nuamber for the DA LV */
long size;			/* # bytes for DALV LP structs for one PV */
struct cnfg_pv_del arg;		/* struct passed to VGSA config routine */
int numvgsas;			/* number of VGSAs on this PV */
int lrc;			/* lockl return code */
int retcode;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kdelpv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kdelpv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/*
 * verify that this VG is not currently being forced off
 */
if (kvg_ptr -> flags & VG_FORCEDOFF) {
    if (lrc == LOCK_SUCC) 
	unlockl(&(kvg_ptr -> vg_lock));
    kdelpv -> rc = CFG_FORCEOFF;
    return (EINVAL);
}
   
/* 
 * zero out the LPs in the Descriptor Area LV
 * and remove the pvol struct (for delete PV) or change the PV state
 * to missing a wait for current IO to complete (for remove PV) 
 * NOTE: the state of missing prevents new IO from being queued
 */
kpv_ptr = kvg_ptr -> pvols [kdelpv -> pv_num - 1];
ASSERT (kpv_ptr != NULL)

/*
 * set up pointers to DA LV & the DALV LP for this PV 
 */
klv_ptr = (struct lvol *) kvg_ptr -> lvols [HD_RSRVDALV] ;
lpart_num = (kdelpv -> pv_num - 1) * kdelpv->num_desclps + 1;
kpp_ptr = (struct part *) ((caddr_t) (klv_ptr -> parts [PRIMMIRROR]) +
		       (lpart_num - 1) * sizeof (struct part));
size = kdelpv->num_desclps * sizeof (struct part);

/* 
 * initialize arg structure for VGSA configuration routine which
 * will update the VGSA for this PV, delete the VG ptr to this
 * PV, zero out DALV's LP on this PV, and update the VG's qourum count
 */
arg.pv_ptr = kpv_ptr;		/* PV to delete */        
arg.lp_ptr = kpp_ptr;		/* ptr to DALV's LP on this PV */
arg.lpsize = size;		/* size of DALV LP on this PV */
arg.qrmcnt = kdelpv -> quorum_cnt;	/* new VG quorum count */

retcode = hd_sa_config(kvg_ptr, cmd,(caddr_t) &arg);
if (retcode == FAILURE)
{
    if (lrc == LOCK_SUCC) 
	unlockl(&(kvg_ptr -> vg_lock));
    /* if VGSA update fails, then we will FORCE the VG off! */
    kdelpv -> rc = CFG_FORCEOFF;
    return(EINVAL);
}

/* 
* if PV was opened, close it
*/
if (kpv_ptr -> fp != NULL)
{
    fp_close (kpv_ptr -> fp);
    kpv_ptr -> fp = NULL;		/* used as open flag */
}

/*
* free space allocated for bad block structures
* and defects table array for this PV
*/

hd_free_dtab(kpv_ptr);

/* 
* if this PV is being deleted from VG, delete PV structure
* otherwise,  zero out the VGSA LSNs 
*/
if (cmd == HD_KDELPV) {
    assert(xmfree ((caddr_t) kpv_ptr, pinned_heap) == LVDD_SUCCESS); 
}
else {
    kpv_ptr -> sa_area[0].lsn = 0;
    kpv_ptr -> sa_area[1].lsn = 0;
}


if (lrc == LOCK_SUCC) 
   unlockl(&(kvg_ptr -> vg_lock));

return (LVDD_SUCCESS);


} 




/***********************************************************************
 *                                                                     *
 * NAME:  hd_free_dtab                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine frees the defects table for this PV and each of it's *
 *   block entries.                                                    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     kpv_ptr                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     All bad block entries and the defects hash anchor table         *
 *     for this PV freed.                                              *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


void
hd_free_dtab (

struct pvol *kpv_ptr) 		/* ptr to PV structure */

{ 

short int hash_index;		/* index for bad block hashing */
struct bad_blk * bb_ptr;	/* LVDD bad block entry */
struct bad_blk * save_ptr;	/* temp bad block entry pointer */
int retcode;

if (kpv_ptr -> defect_tbl != NULL)
{
    /* 
     * for each bad block hash chain, loop thru each bad block
     * entry and free it
     */
    for (hash_index=0; hash_index<HASHSIZE; hash_index=hash_index+1)
    {
        bb_ptr = kpv_ptr -> defect_tbl -> defects [hash_index];
	while (bb_ptr != NULL)
	{
	    save_ptr = bb_ptr;
	    bb_ptr = bb_ptr -> next;
	    assert (xmfree ((caddr_t) save_ptr, pinned_heap) == LVDD_SUCCESS);
	} 

    } 

    /* free space allocated for the defects array for this PV */
    assert(xmfree ((caddr_t) kpv_ptr -> defect_tbl, pinned_heap)==LVDD_SUCCESS);
    kpv_ptr -> defect_tbl = NULL;	

} 

return;
}


/***********************************************************************
 *                                                                     *
 * NAME:  hd_kchgvgsa                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine adds or deletes a VGSA from a PV and updates the     *
 *   VG's quroum count.                                                *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kchgvgsa                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     VGSA added/deleted from this PV and VG's quorum count udpated.  *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kchgvgsa (

dev_t device,				/* device number of VG to be updated */
struct kchgvgsa * kchgvgsa,		/* change VGSA information */
int cmd)				/* add VGSA or delete VGSA */

{ 

struct volgrp * kvg_ptr;		/* kernel VG struct */
struct pvol * kpv_ptr;			/* kernel VG struct */
struct cnfg_pv_vgsa arg;		/* arg struct for hd_sa_config*/
int lrc;				/* lockl return code */
int retcode;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kchgvgsa->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kchgvgsa -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    

/* if the PV is not defined, return error */
kpv_ptr = kvg_ptr->pvols[kchgvgsa->pv_num-1];
if (kpv_ptr == NULL)
    {
    if (lrc == LOCK_SUCC) 
        unlockl(&(kvg_ptr->vg_lock));
    kchgvgsa -> rc = CFG_NONEXISTPV;
    return(ENXIO);
    }

/* setup argument for VGSA config routine */
arg.pv_ptr = kpv_ptr;
arg.sa_lsns[0] = kchgvgsa -> sa_lsns[0];
arg.sa_lsns[1] = kchgvgsa -> sa_lsns[1];
arg.qrmcnt = kchgvgsa -> quorum_cnt;
retcode = hd_sa_config(kvg_ptr, cmd,(caddr_t) &arg);

/*
 * Unlock the VG before leaving
 */
if (lrc == LOCK_SUCC) 
    unlockl(&(kvg_ptr -> vg_lock));

if (retcode == FAILURE) {
    /* if VGSA update fails, then we will FORCE the VG off!
     * Go ahead and complete the deleting of the PV first.
     */
     kchgvgsa->rc = CFG_FORCEOFF;
     return(EINVAL);
}

return(LVDD_SUCCESS);

}
