static char sccsid[] = "@(#)30  1.13  src/bos/kernext/lvm/hd_cfglv.c, sysxlvm, bos411, 9428A410j 3/4/94 17:22:09";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_config.c
 *
 * FUNCTIONS: hd_kaddlv
 *	      hd_kchglv
 *	      hd_kchklv
 *            hd_kdellv
 *	      hd_kextend
 *	      hd_kreduce
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
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
#include <sys/dasd.h>			/* LVDD data structures */
#include <lvm.h>			/* LVM Library interfaces */
#include <sys/hd.h>			/* LVDD interfaces */
#include <sys/vgsa.h>			/* VGSA structures */
#include <sys/hd_config.h>		/* LVDD config data structures */


/***********************************************************************
 *                                                                     *
 * NAME:  hd_kaddlv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD logical volume data structure from   *
 *   user space to kernel space for a logical volume which is being    *
 *   added to the kernel data structures for this volume group.        *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kaddlv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A logical volume structure has been added into the kernel data  *
 *     structures.                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   lockl                                                             *
 *   unlockl                                                           *
 *   xmalloc                                                           *
 *   devswqry                                                          *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kaddlv (

dev_t device,			/* device number of VG to be updated */
struct kaddlv * kaddlv)		/* add LV information */

{ 


struct volgrp * kvg_ptr;	/* kernel VG structure */
struct lvol * klv_ptr;		/* kernel LV list for this VG */
int retcode,lrc;

/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kaddlv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kaddlv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/* if in system mgmt only mode, do NOT define
 * LV structures in kernel
 */
if (kvg_ptr->flags & VG_SYSMGMT)
{
    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));		/* unlock VG */
    return (LVDD_SUCCESS);
}
	
/*
 * allocate new LV structure & initialize it
 */
klv_ptr = (struct lvol *) xmalloc ((uint) sizeof (struct lvol), HD_ALIGN,
				   pinned_heap);
if (klv_ptr == NULL)
{
    if(lrc == LOCK_SUCC) 
       unlockl (&(kvg_ptr->vg_lock));
    return (ENOMEM);
}
bzero ((caddr_t) klv_ptr, sizeof (struct lvol));
klv_ptr->lv_options = kaddlv->lv_options;
klv_ptr->stripe_exp = kaddlv->stripe_exp;
klv_ptr->striping_width = kaddlv->striping_width;
klv_ptr->waitlist = EVENT_NULL;
kvg_ptr->lvols [kaddlv->lv_minor] = klv_ptr;

if(lrc == LOCK_SUCC)
   unlockl (&(kvg_ptr->vg_lock));		/* unlock VG */

return (LVDD_SUCCESS);

}

/***********************************************************************
 *                                                                     *
 * NAME:  hd_kchglv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies certain information from the LVDD logical     *
 *   volume structure in user space to the logical volume structure in *
 *   kernel space for the logical volume which is being changed.       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kchglv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A logical volume structure in the kernel data structures has    *
 *     been updated to reflect changes made to that logical volume.    *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   lockl                                                             *
 *   unlockl                                                           *
 *                                                                     *
 ***********************************************************************
 */

int
hd_kchglv (

dev_t device,			/* device number of VG to be updated */
struct kchglv * kchglv)		/* change LV information */

{ 


struct volgrp * kvg_ptr;	/* kernel VG structure */
struct lvol * klv_ptr;		/* kernel LV list for this VG */
int retcode,lrc;



/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kchglv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kchglv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/*
 * if in system mgmt only mode, exit since LV structs
 * are not defined in the kernel
 */
if (kvg_ptr->flags & VG_SYSMGMT)
{
    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));			/* unlock VG */
    return (LVDD_SUCCESS);
}

/* 
 * get pointer to LV's structure & verify that it is not closed
 * (it is not worth quiescing the LV just to change minor LV characteristics)
 */
klv_ptr = kvg_ptr->lvols [kchglv->lv_minor];
if (klv_ptr->lv_status != LV_CLOSED)
{
    if(lrc ==  LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));
    return (EBUSY);
}

/* change this LV's attributes to the ones passed in */
klv_ptr->i_sched = kchglv->i_sched;    /* mirror scheduling policy */
klv_ptr->lv_options = kchglv->lv_options;    /* logical volume options */

if(lrc == LOCK_SUCC)
   unlockl (&(kvg_ptr->vg_lock));		/* unlock VG */

return (LVDD_SUCCESS);

}

/***********************************************************************
 *                                                                     *
 * NAME:  hd_kchklv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine checks the information from the LVDD logical         *
 *   volume structure to see if the logical volume is opened or not.   *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kchklv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The input structure's open flag indicates if the logical volume *
 *     is opened or not.                                               *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   devswqry                                                          *
 *   lockl                                                             *
 *   unlockl                                                           *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kchklv (

dev_t device,			/* device number of VG to be updated */
struct kchklv * kchklv)		/* check LV information */

{ 

struct volgrp * kvg_ptr;	/* kernel VG structure */
struct lvol * klv_ptr;		/* kernel LV list for this VG */
int retcode,lrc;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kchklv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kchklv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/* 
 * get the pointer to the LV and see if the LV is open 
 */
klv_ptr = kvg_ptr->lvols [kchklv->lv_minor];
if ((klv_ptr != NULL) && (klv_ptr->lv_status == LV_OPEN))
    kchklv->open = TRUE;


if(lrc == LOCK_SUCC)
   unlockl (&(kvg_ptr->vg_lock));			/* unlock VG */

return (LVDD_SUCCESS);

} 

/***********************************************************************
 *                                                                     *
 * NAME:  hd_kdellv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine frees the LVDD logical volume data structure in the  *
 *   kernel for the logical volume which is being deleted and zeroes   *
 *   out the pointer to it.                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kdellv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A logical volume structure in the kernel data structures has    *
 *     been deleted.                                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   xmfree                                                            *
 *   lockl                                                             *
 *   unlockl                                                           *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kdellv (

dev_t device,			/* device number of VG to be updated */
struct kdellv * kdellv)		/* delete LV information */

{ 


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct volgrp * kvg_ptr;		/* kernel VG struct */
struct lvol * klv_ptr;			/* kernel LV list for this VG */
register int i_prty;			/* saved interrupt priority */
short int lv_index;
int retcode,lrc;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kdellv->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kdellv -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/* if in system mgmt mode, exit since LV structure is NOT
 * defined in the kernel
 */
if (kvg_ptr->flags & VG_SYSMGMT)
{
    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));
    return (LVDD_SUCCESS);
}

/*
 * get pointer to LV's structure & verify that it is not closed
 */
klv_ptr = (struct lvol *) kvg_ptr->lvols [kdellv->lv_minor];
if (klv_ptr->lv_status != LV_CLOSED)
{
    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));			/* unlock VG */
    return (EBUSY);
}

/*
 * zero the pointer to this LV struct & free the LV struct 
 * NOTE: no need to disable here since the LV is closed
 * and the VG lock prevents it from being opened again
 */
kvg_ptr->lvols [kdellv->lv_minor] = NULL;
retcode = xmfree ((caddr_t) klv_ptr, pinned_heap);
assert (retcode == LVDD_SUCCESS);

if(lrc == LOCK_SUCC)
   unlockl (&(kvg_ptr->vg_lock));			/* unlock VG */

return (LVDD_SUCCESS);

} 

/***********************************************************************
 *                                                                     *
 * NAME:  hd_kextend                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD logical partition structures from    *
 *   user space to kernel space for a logical volume which is being    *
 *   extended.                                                         *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kextend                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The partition structures for the specified logical volume have  *
 *     been updated.                                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   devswqry                                                          *
 *   xmfree                                                            *
 *   lock                                                              *
 *   unlockl                                                           *
 *   xmalloc                                                           *
 *   pin                                                               *
 *                                                                     *
 ***********************************************************************
 */

int
hd_kextend (
dev_t device,			/* device number of VG to be updated */
struct kextred * kextend)	/* extend LV information */
{
    struct volgrp *kvg_ptr;	        /* kernel VG structure */
    struct lvol *klv_ptr;	        /* LV struct to be expanded */
    struct extred_part *expart_ptr;     /* ptr to PP struct list added to LV */
    struct part *newpp;                 /* ptr to new PP struct being added */
    struct part *new_parts[MAXNUMPARTS];/* new PP list for each copy */
    struct part *old_parts[MAXNUMPARTS];/* old PP list for each copy */
    struct part *oldpp;       		/* ptr to PP copy in old PP list */
    int old_numlps;			/* num LPs in LV before extended */
    long size;				/* size of PP struct list to extend */
    long old_size;			/* size of old list of PP structs */
    long new_size;			/* size of new list of PP structs */
    short int nparts;			/* number of PP structs for this LV */
    short int old_part;			/* set if LP existed before extend */
    unsigned long nblocks;		/* size of LV in blocks */
    char i_sched;			/* initial scheduler policy state */
    short int ci;			/* loop index for copies of the lv */
    short int ei;			/* loop index for num pps to extend */
    struct cnfg_pp_state *vgsa; 	/* ptrs to struct for VGSA work */
    struct sa_ext disext;		/* struct to pass to hd_sa_config() */
    int retcode,lrc;			

    /*
     * get the volgrp pointer from devsw table, lock the VG struct
     * and verify that the VG passed in is correct
     */
    retcode = hd_verifyvgid(device, kextend->vg_id, &kvg_ptr, &lrc);
    if (retcode != LVDD_SUCCESS) {
        if (retcode == CFG_INVVGID) {
            kextend -> rc = CFG_INVVGID;
            return (EINVAL);
        }
        else
    	    return(retcode);
    }
    
    /* 
     * if in system mgmt only mode, exit since LV structures not defined 
     */
    if(kvg_ptr->flags & VG_SYSMGMT) {
	if(lrc == LOCK_SUCC)
     	   unlockl(&(kvg_ptr->vg_lock));
    	return (LVDD_SUCCESS);
    }

    /* get a pointer to the lvol structure for the lv being extended 
     * and use its information along with the volgrp structure info to 
     * calculate needed values */
    klv_ptr = kvg_ptr->lvols [kextend->lv_minor];

    /* allocate MWC data structures if we are extending an open LV
       from nonmirrored to mirrored and its VG has no MWC memory and 
       MWC is enabled for that LV */
    if ((klv_ptr->lv_status == LV_OPEN) && 
        (kvg_ptr->mwc_rec == NULL) &&
        (klv_ptr->nparts == 1) &&
        (kextend->copies > 1) &&
        !(klv_ptr->lv_options & LV_NOMWC))
        {
        if ((retcode = alloc_mwc_mem(kvg_ptr)) != LVDD_SUCCESS) 
            {
            if (lrc == LOCK_SUCC)
                unlockl(&(kvg_ptr->vg_lock));
            return(retcode);
            }
        }

    /* old and new size for the part structs of this logical volume */
    old_numlps = BLK2PART (kvg_ptr->partshift, klv_ptr->nblocks);
    old_size = old_numlps * sizeof (struct part);
    new_size = kextend->num_lps * sizeof (struct part);

    /*
     * for the number of copies this logical volume has 
     * save the pointer to the old part struct list for each copy
     * and allocate space for a new part struct, store its pointer
     * and zero it out.
     */
    for (ci=0; ci < kextend->copies; ci++) {
	old_parts[ci] = klv_ptr->parts[ci];
	new_parts[ci] = (struct part *) xmalloc ((uint) new_size,
						 HD_ALIGN, pinned_heap);
	if (new_parts[ci] == NULL) {
	    for(ci --; ci >= 0; ci --) {
	       assert(xmfree((caddr_t)new_parts[ci],pinned_heap)==LVDD_SUCCESS);
	    }
	    if(lrc == LOCK_SUCC)
	       unlockl (&(kvg_ptr->vg_lock));
	    return (ENOMEM);
	}
	bzero ((caddr_t)(new_parts[ci]), new_size);
    }

    /* 
     * calculate the size of the list of pps to be extended, allocate
     * kernel space to hold this list, and copy the user list to the
     * newly allocated kernel list
     */ 
    size = kextend->num_extred * sizeof (struct extred_part);
    expart_ptr = xmalloc ((uint) size, HD_ALIGN, pinned_heap);
    if (expart_ptr == NULL) {
	for(ci = 0; ci < kextend->copies; ci++ ) {
	   assert(xmfree((caddr_t)new_parts[ci],pinned_heap) == LVDD_SUCCESS);
	}
 	if(lrc == LOCK_SUCC)
	   unlockl (&(kvg_ptr->vg_lock));
	return (ENOMEM);
    }
    retcode = copyin (kextend->extred_part, expart_ptr, size);
    if (retcode != LVDD_SUCCESS) {
	for(ci = 0; ci < kextend->copies; ci++ ) {
	   assert(xmfree((caddr_t)new_parts[ci],pinned_heap) == LVDD_SUCCESS);
	}
	assert(xmfree((caddr_t)expart_ptr,pinned_heap) == LVDD_SUCCESS);
        if(lrc == LOCK_SUCC)
	   unlockl (&(kvg_ptr->vg_lock));
	return (EFAULT);
    }

    /* calculate the size for, malloc, and zero out the VGSA structs */
    size = sizeof(struct cnfg_pp_state) * (kextend->num_extred + 1);
    vgsa = (struct cnfg_pp_state *) xmalloc((uint)size,HD_ALIGN,pinned_heap);
    if(vgsa == NULL) {
	for(ci = 0; ci < kextend->copies; ci++ ) {
	   assert(xmfree((caddr_t)new_parts[ci],pinned_heap) == LVDD_SUCCESS);
	}
	assert(xmfree((caddr_t)expart_ptr,pinned_heap) == LVDD_SUCCESS);
	if(lrc == LOCK_SUCC)
	   unlockl(&(kvg_ptr->vg_lock));
	return(ENOMEM);
    }
    bzero((caddr_t)vgsa, size);

    /*
     * for each pp being extended, check to see if you will add a copy to an
     * existing logical partition or add a new logical partition. Find an  
     * empty part struct entry in the new_parts list for this new physical
     * partition and fill it in along with a VGSA struct for the new pp. If
     * we are adding a copy to an existing logical partition then make the
     * ppstate field in its part struct and VGSA struct stale
     *
     * NOTE: This whole extend algorithm assumes we never try to stuff 4
     *       copies into 3 holes.
     */
    for(ei = 0; ei < kextend->num_extred; ei++) {
	old_part = FALSE;
	ci = 0;
	/*
	 * The if here takes care of empty LVs and making a LV longer.
	 * i.e. adding LPs to the LV.
	 */
	if(klv_ptr->parts[0] != NULL  &&
	    expart_ptr->lp_num <= old_numlps) {
	    /*
	     * Here we know that we are adding a copy to an existing
	     * LP or adding the first copy to a LP. i.e. a hole in the
	     * LV.  So find the first available hole in the LP.  If
	     * all copies are filled then we assume this partition is
	     * being extended past the current nparts. i.e. from 1 to
	     * 2 copies or from 2 to 3.
	     */
	    for( ; ci < klv_ptr->nparts; ci ++) {
		oldpp = (struct part *)(klv_ptr->parts[ci] +
					    (expart_ptr->lp_num - 1)); 
		if( (oldpp->pvol != NULL) && (ci == 0) )
		    old_part = TRUE;
		else if( oldpp->pvol == NULL ) {
		    /*
		     * There is a slot available in the current LP
		     */
		    break;
		}
	    }  /* end for number of old copies */
	}  /* end if lp previously existed in the lv */
	/* 
	 * Now check that the same slot exists in the new parts.  If not
	 * then advance until one is found.
	 */
	for( ; ci < kextend->copies; ci++ ) {
	    newpp = (new_parts[ci]+(expart_ptr->lp_num - 1));
	    if(newpp->pvol == NULL)
		break;
	}
	/*
	 * we now know where to put the new PP, fill in a part and
	 * VGSA struct for it.
	 */
	newpp = new_parts[ci] + (expart_ptr->lp_num - 1); 
	newpp->pvol = kvg_ptr->pvols[expart_ptr->pv_num - 1];
	newpp->start = expart_ptr->start_addr;
	newpp->sync_trk = NO_SYNCTRK;
	vgsa->pp = BLK2PART(kvg_ptr->partshift,
			    (newpp->start - newpp->pvol->fst_usr_blk));
	vgsa->pvnum = expart_ptr->pv_num - 1;
	if (old_part == TRUE) {
	    newpp->ppstate |= PP_STALE;
	    vgsa->ppstate = STALEPP;
	}
	else
	    vgsa->ppstate = FRESHPP;
	expart_ptr ++;
	vgsa ++;
    }  /* end loop for number of pps to extend */


    /* set the ppstate to CNFG_STOP in the last VGSA entry to show end */
    vgsa->ppstate = CNFG_STOP;

    /* fill in the structure that will be passed to hd_sa_config */
    bzero((char *)&(disext), sizeof(struct sa_ext));
    disext. klv_ptr = klv_ptr;
    disext.old_nparts = klv_ptr->nparts;
    disext.nparts = kextend->copies;
    disext.nblocks = PART2BLK(kvg_ptr->partshift, kextend->num_lps);
    disext.isched = kextend->i_sched;
    disext.new_parts = new_parts;
    disext.old_numlps = old_numlps;

    /* set the vgsa pointer back to the first entry */
    vgsa -= kextend->num_extred;
    disext.vgsa = vgsa; 

    /* 
     * call routine that updates the VGSA and handles all other processing
     * that needs to be done while interrupts are disabled
     */
    expart_ptr -= kextend->num_extred;
    retcode = hd_sa_config (kvg_ptr, HD_KEXTEND,(caddr_t)(&disext));
    if(retcode == FAILURE) {
	if(disext.error != 0)
	   kextend->rc = disext.error;
	assert(xmfree((caddr_t)expart_ptr,pinned_heap) == LVDD_SUCCESS);
	assert(xmfree((caddr_t)vgsa,pinned_heap) == LVDD_SUCCESS);
	for(ci = 0; ci < kextend->copies; ci++ ) {
	   assert(xmfree((caddr_t)new_parts[ci],pinned_heap) == LVDD_SUCCESS);
	}
	if(lrc == LOCK_SUCC)
	   unlockl(&(kvg_ptr->vg_lock));
	if(disext.error != 0)
	   return(EINVAL);
	else
	   return(retcode);
    }

    /* loop for the old copies that the lv had and free the part structs */
    for (ci = 0; ci < disext.old_nparts; ci++) {
	    assert(xmfree((caddr_t)old_parts[ci], pinned_heap) == LVDD_SUCCESS);
    } /* loop for old number of copies */

    /* free the kernel space that held the list of pps to be extended */    
    assert(xmfree((caddr_t)expart_ptr, pinned_heap) == LVDD_SUCCESS);
    assert(xmfree((caddr_t)vgsa, pinned_heap) == LVDD_SUCCESS);

    /* unlock the volume group structure */
    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));

    return(LVDD_SUCCESS);
} 


/***********************************************************************
 *                                                                     *
 * NAME:  hd_kreduce                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD logical partition structures from    *
 *   user space to kernel space for a logical volume which is being    *
 *   reduced.                                                          *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kreduce                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The partition structures for the specified logical volume have  *
 *     been updated.                                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   devswqry                                                          *
 *   xmfree                                                            *
 *   lock                                                              *
 *   unlockl                                                           *
 *   xmalloc                                                           *
 *   bcopy                                                             *
 *   bzero                                                             *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kreduce (
dev_t device,			/* device number of VG to be updated */
struct kextred * kreduce)	/* reduce LV information */

{
    struct volgrp *kvg_ptr;		/* kernel VG structure */
    struct lvol *klv_ptr;		/* LV struct to be reduced */
    struct extred_part *redpart_ptr;	/* ptr to PP struct removed from LV */
    struct part *new_parts[MAXNUMPARTS];/* new PP list for each copy of LV */
    struct part *old_parts[MAXNUMPARTS];/* old PP list for each copy of LV */
    long size;			       	/* size of PP struct list to reduc */
    long new_size;			/* size of new list of PP structs */
    int retcode,lrc;
    int old_numcopies;		      	/* old number of copies on the LV */
    short int ci;		   	/* loop index for copies of the LV */
    struct sa_red disred;		/* struct to pass to hd_sa_config() */
    struct part *pp;			/* ptr to a part structure */
    struct extred_part *pc,*pl,*eaddr;  /* ptrs to entries of pps to reduce */
    int cpcnt;				/* loop variable */

    /*
     * get the volgrp pointer from devsw table, lock the VG struct
     * and verify that the VG passed in is correct
     */
    retcode = hd_verifyvgid(device, kreduce->vg_id, &kvg_ptr, &lrc);
    if (retcode != LVDD_SUCCESS) {
        if (retcode == CFG_INVVGID) {
            kreduce -> rc = CFG_INVVGID;
            return (EINVAL);
        }
        else
    	    return(retcode);
    }
    

    /* If in system management only mode, exit; LV structs not defined */ 
    if (kvg_ptr->flags & VG_SYSMGMT) {
	if(lrc == LOCK_SUCC)
    	   unlockl (&(kvg_ptr->vg_lock));
    	return (LVDD_SUCCESS);
    }

    /* get a pointer to the logical volume being reduced */
    klv_ptr = kvg_ptr->lvols[kreduce->lv_minor];

    /* 
     * find the size of the list of pps to reduce, allocate space for it.
     * and copy the user info to the kernel space. 
     * this list is passed from the library and is a struct extred_part
     */
    size = kreduce->num_extred * sizeof(struct extred_part);
    redpart_ptr = xmalloc((uint)size, HD_ALIGN, pinned_heap);
    if(redpart_ptr == NULL) {
	if(lrc == LOCK_SUCC)
	   unlockl(&(kvg_ptr->vg_lock));
	return(ENOMEM);
    }
    retcode = copyin (kreduce->extred_part, redpart_ptr, size);
    if (retcode != LVDD_SUCCESS) {
   	assert(xmfree((caddr_t)redpart_ptr,pinned_heap) == LVDD_SUCCESS);
	if(lrc == LOCK_SUCC)
	   unlockl (&(kvg_ptr->vg_lock));
	return (EFAULT);
    }

    /*
     * calculate the new size of the part structs for this logical volume
     * save off the old lvol parts and malloc space for the newparts
     */
    new_size = kreduce->num_lps * sizeof(struct part);
    for (ci = 0; ci < klv_ptr->nparts; ci++) {
	old_parts[ci] = klv_ptr->parts[ci];
	if (ci < kreduce->copies) {
	    new_parts[ci] = (struct part *)xmalloc((uint)new_size,
						       HD_ALIGN, pinned_heap);
	    if(new_parts[ci] == NULL) {
		for(ci--; ci >= 0; ci--)
		   assert(xmfree((caddr_t)new_parts[ci],pinned_heap)==0);
   	        assert(xmfree((caddr_t)redpart_ptr,pinned_heap)==LVDD_SUCCESS);
	        if(lrc == LOCK_SUCC)
		   unlockl (&(kvg_ptr->vg_lock));
		return (ENOMEM);
	    }
	}
    } /* loop for old number of copies */

    /* save off the old number of copies of the lv */
    old_numcopies = klv_ptr->nparts;

    /*
     * loop through the list of pps to reduce in order to build a mask for
     * each lp that will show the copies that are being reduced.
     */
    eaddr = redpart_ptr + kreduce->num_extred;
    for(pl = redpart_ptr; pl < eaddr;) {
       for(pc = pl,cpcnt = 0;((cpcnt < klv_ptr->nparts)&&(pc < eaddr)); pc++) {
	  if(pc->pv_num != -1) {
	     if(pc->lp_num == pl->lp_num) {
		for(ci = 0; ci < klv_ptr->nparts; ci++) {
		   pp = PARTITION(klv_ptr,(pl->lp_num - 1),ci);
	 	   if(pp->pvol) {
		      if((pp->start == pc->start_addr) && 
			 (pp->pvol->pvnum == (pc->pv_num - 1))) {
			    pl->mask |= MIRROR_MASK(ci);
			    break;
		      }
		   }
		}
		pc->pv_num = -1;
		cpcnt ++;
             }
          }
        }
        while((pl < eaddr) && (pl->pv_num == -1))
	   pl++;
    }
    bzero((char *)(&disred),sizeof(struct sa_red));
    disred.lv = klv_ptr;
    disred.nparts = kreduce->copies;
    disred.isched = kreduce->i_sched;
    disred.nblocks = PART2BLK (kvg_ptr->partshift, kreduce->num_lps);
    disred.newparts = new_parts;
    disred.min_num = kreduce->lv_minor;
    disred.numlps = kreduce->num_lps;
    disred.numred = kreduce->num_extred;
    disred.list = redpart_ptr;

    /*
     * call routine that updates the VGSA if necessary and handles all other
     * processing that needs to be done while interrupts are disabled
     */

    retcode = hd_sa_config(kvg_ptr,HD_KREDUCE,(caddr_t)(&disred));
    if(retcode == FAILURE) {
	if(disred.error != 0)
	   kreduce->rc = disred.error;
	if(lrc == LOCK_SUCC)
	   unlockl(&(kvg_ptr->vg_lock));
	assert(xmfree((caddr_t)redpart_ptr,pinned_heap) == LVDD_SUCCESS);
	for(ci = 0; ci < kreduce->copies; ci ++)
	     assert(xmfree((caddr_t)new_parts[ci],pinned_heap) == LVDD_SUCCESS);
	if(disred.error != 0)
	   return(EINVAL);
        else
	   return(retcode);
    }

    /* free the space we malloced and the old pointers and unlock the vg  */
    for(ci = 0; ci < old_numcopies; ci++) {
	if (old_parts [ci] != NULL) {
	    retcode = xmfree ((caddr_t) old_parts[ci], pinned_heap);
	    assert (retcode == LVDD_SUCCESS);
	} /* old PP list exists */
    } /* loop for old number of copies */
    assert(xmfree((caddr_t) redpart_ptr, pinned_heap) == LVDD_SUCCESS);

    /* try to deallocate MWC memory for this VG */
    if (kreduce->copies == 1)
        deallocate_mwc_try(kvg_ptr);

    if(lrc == LOCK_SUCC)
       unlockl (&(kvg_ptr->vg_lock));

    return (LVDD_SUCCESS);
}
