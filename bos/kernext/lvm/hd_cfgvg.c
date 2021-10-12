static char sccsid[] = "@(#)32	1.15  src/bos/kernext/lvm/hd_cfgvg.c, sysxlvm, bos41J, 9520B_all 5/18/95 08:15:10";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 32
 *
 * FUNCTIONS: hd_config
 *            hd_verifyvgid
 *            hd_kdeflvs
 *            hd_ksetupvg
 *	      hd_mwc_fprec
 *	      hd_disastrec
 *            hd_kdefvg
 *	      hd_kdelvg
 *	      hd_free_lvols
 *	      hd_kchgqrm
 *	      hd_kqryvgs
 *            hd_krebuild
 *            hd_kvgstat
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
#include <unistd.h>
#include <sys/malloc.h>
#include <errno.h>
#include <sys/dasd.h>			/* LVDD internal structures */
#include <lvm.h>			/* LVM Library interfaces */
#include <sys/hd.h>			/* LVDD interfaces */
#include <sys/vgsa.h>			/* Volume Group Status Area info */
#include <sys/hd_config.h>		/* LVDD config structures */
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern nodev ( );		/* non-supported entry pt for a device */


/***********************************************************************
 *                                                                     *
 * NAME:  hd_config                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function initializes or configures a volume group by adding  *
 *   or updating the LVDD volume group data structures into the        *
 *   kernel.  It is called indirectly via a call to the sysconfig      *
 *   system routine by the lvm_varyonvg subroutine when a volume       *
 *   group is varied on in order to initialize the LVDD volume group   *
 *   data structures.  It is also called by other LVM library          *
 *   subroutines to update, add, or delete information from the        *
 *   LVDD volume group data structures.                                *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     cmd                                                             *
 *     uiop                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LVDD volume group data structures have been added, deleted, *
 *     or updated.                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_config (

dev_t dev,			/* device number of VG to be configured */
int cmd,			/* specific config function */
struct uio * uiop)		/* device dependent information (ddi) */

{ 

int rc;
int retcode;
struct ddi_info *userparms;	/* save addr of user parms area since
				 * uiomove destroys data */
struct ddi_info parms;		/* ddi passed in */

/*
 * save user address of parms since uiomove can destroy data
 * and move it to kernel space
 */
userparms = (struct ddi_info *)(uiop -> uio_iov -> iov_base);
retcode = uiomove ((caddr_t) (&parms), (int) sizeof (struct ddi_info),
		   UIO_WRITE, uiop);
if (retcode != LVDD_SUCCESS)
    return(retcode);

/*
 * call the specific configuration routine
 */

switch (cmd)
{

  case HD_KADDLV:	/* add a LV to an existing VG */
      retcode = hd_kaddlv (dev, &(parms.parmlist.kaddlv));
      break;

  case HD_KADDMPV:	/* add a missing PV to existing VG */
      retcode = hd_kaddmpv (dev, &(parms.parmlist.kaddmpv));
      break;

  case HD_KADDPV:	/* add a PV to existing VG */
      retcode = hd_kaddpv (dev, &(parms.parmlist.kaddpv));
      break;

  case HD_KADDVGSA:	/* add a VGSA to an excsting PV */
  case HD_KDELVGSA:	/* remove a VGSA from existing PV */
      retcode = hd_kchgvgsa (dev, &(parms.parmlist.kchgvgsa), cmd);
      break;

  case HD_KCHGLV:	/* change existing LV */
      retcode = hd_kchglv (dev, &(parms.parmlist.kchglv));
      break;

  case HD_KCHGQRM:	/* change VG's quorum count */
      retcode = hd_kchgqrm (dev, &(parms.parmlist.kchgqrm));
      break;

  case HD_KCHKLV:	/* check if a LV is open */
      retcode = hd_kchklv (dev, &(parms.parmlist.kchklv));
      /* if LV is open, move returned data back to user space */
      if ((retcode == LVDD_SUCCESS) && (parms.parmlist.kchklv.open == TRUE))
      {
	  retcode = copyout (&(parms.parmlist.kchklv.open),
			   &(userparms -> parmlist.kchklv.open),
			   sizeof (parms.parmlist.kchklv.open));
	  if (retcode != LVDD_SUCCESS)
       	      retcode = EFAULT;
      }
      break;

  case HD_KSETUPVG:	/* setup the VG: define LVs and the VGSA/MWCC info */
      retcode = hd_ksetupvg (dev, &(parms.parmlist.ksetupvg));
      break;

  case HD_KDEFVG:	/* define the VG to the kernel */
      retcode = hd_kdefvg (dev, &(parms.parmlist.kdefvg));
      break;

  case HD_KDELLV:	/* delete a LV from existing VG */
      retcode = hd_kdellv (dev, &(parms.parmlist.kdellv));
      break;

  case HD_KDELPV:	/* delete a PV from existing VG */
  case HD_KREMPV:	/* remove a PV from existing VG */
  case HD_KMISSPV:	/* mark a PV missing from existing VG */
      retcode = hd_kdelpv (dev, &(parms.parmlist.kdelpv), cmd);
      break;

  case HD_KDELVG:	/* delete a VG from kernel entirely or put VG
			 * into system mgmt mode (delete lvol structs
			 * for all LVs except the DALV)
			 */
      retcode = hd_kdelvg (dev, &(parms.parmlist.kdelvg));
      break;

  case HD_KEXTEND:	/* extend a LV */
      retcode = hd_kextend (dev, &(parms.parmlist.kextred));
      break;

  case HD_KQRYVGS:	/* query all VGs */
      retcode = hd_kqryvgs (dev, &(parms.parmlist.kqryvgs));
      break;

  case HD_KREBUILD: /* request is to rebuild the volume group file */
	/* call hd_krebuild to return info needed  */
	retcode = hd_krebuild(dev,&(parms.parmlist.krebuild));
        break;

  case HD_KREDUCE:	/* reduce a LV */
      retcode = hd_kreduce (dev, &(parms.parmlist.kextred));
      break;

  case HD_KVGSTAT:	/* return the varyied on status of a VG */
      retcode = hd_kvgstat (dev, &(parms.parmlist.kvgstat));
      /* if successful, copy returned info back to user data */
      if (retcode == LVDD_SUCCESS)
      {
	   retcode = copyout (&(parms.parmlist.kvgstat),
			   &(userparms -> parmlist.kvgstat),
			   sizeof (parms.parmlist.kvgstat));
	   if (retcode != LVDD_SUCCESS)
	      retcode = EFAULT;
       }
       break;

  default:		/* invalid request */
      retcode = EINVAL;

} 

/* 
 * If EINVAL was returned, then there is an error code in the 
 * FIRST field of the parmlist structure.  Copy this error code
 * to the FIRST field in the user area parmlist.
 * NOTE:  the return code MUST be the FIRST field in both the
 * 	  user and kernel parmlists and it must be an integer.
 */
if (retcode == EINVAL)
{
  rc = copyout (&parms.parmlist, &userparms->parmlist, sizeof(int));
  if (rc != LVDD_SUCCESS)
      retcode = EFAULT;
}

return (retcode);
} 




/***********************************************************************
 *                                                                     *
 * NAME:  hd_verifyvgid                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *  This function is a common routine that most config routines call   *
 *  to get the volgrp  pointer from the device switch table and verify *
 *  that it is correct by comparing the VGID with one passed in.       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     vgid                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *  Pointer to the volgrp structure is returned.		       *
 *  Pointer to the lockl return code is also returned.		       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_verifyvgid (

dev_t device,			/* device number of the VG to be configured */
struct unique_id vgid,		/* volume group ID */
struct volgrp **kvg_ptr,	/* ptr to volgrp structure returned */
int *lrc)			/* lockl return code returned */

{ 
int retcode;

/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct.
 *
 * NOTE:  We must check at each point (until we really have the
 * volgrp struct lock) that the VG has not been closed (forced off)
 * while we are checking it.  It is possible that even after the devswitch
 * query or while sleeping for the VG lock that the VG is forced off;       
 * therefore, we have to be careful and verify that the vg is still there.
 */
retcode = devswqry (device, NULL, (caddr_t *) kvg_ptr);
ASSERT (retcode == LVDD_SUCCESS);
if (*kvg_ptr == NULL)
    return(ENXIO);
*lrc = lockl (&((*kvg_ptr) -> vg_lock), LOCK_SHORT);	/* lock VG */
retcode = devswqry (device, NULL, (caddr_t *) kvg_ptr);
ASSERT (retcode == LVDD_SUCCESS);
if (*kvg_ptr == NULL) {
    if (*lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&((*kvg_ptr) -> vg_lock));
    return(ENXIO);
}

if (vgid.word1 != (*kvg_ptr) -> vg_id.word1  ||
    vgid.word2 != (*kvg_ptr) -> vg_id.word2) {

    if (*lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&((*kvg_ptr) -> vg_lock));
    return (CFG_INVVGID);
}

return(LVDD_SUCCESS);

}





/***********************************************************************
 *                                                                     *
 * NAME:  hd_kdeflvs                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD data structures for the defined      *
 *   logical volumes and the logical partition list for each logical   *
 *   volume from user space to kernel space.                           *
 *                                                                     *
 * NOTES:                                                              *
 *   The VG must already be locked in the kernel before calling this   *
 *   routine.							       *
 *   note:  no backout code needed when an error occurs, since the     *
 *   	varyonvg lib routine calls hd_kdelvg to delete the entire VG   *
 *      if hd_ksetupvg fails.					       *
 *                                                                     *
 *   INPUT:                                                            *
 *     kvg_ptr                                                         *
 *     lv_ptrs                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The list of lvol structures, and the list of part structures    *
 *     for each mirror of each logical volume have been copied from    *
 *     user space to kernel space.                                     *
 *                                                                     *
 *     This volume group has been added to the device switch table.    *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   devswqry                                                          *
 *   xmalloc                                                           *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kdeflvs (

struct volgrp * kvg_ptr,	/* ptr to kernel VG struct */
struct lvol ** lv_ptrs)		/* ptr to array of user area LV structs */

{ 

struct lvol * klv_ptrs [MAXLVS];/* kernel area ptrs to user LV structs */
struct part * u_part_ptr;	/* user area list of PP structs */
struct part * kpp_ptr;		/* kernel list of PP structs */
struct lvol * lv_ptr;		/* user area LV struct */
struct lvol * klv_ptr;		/* kernel area LV struct */
long size;			/* num bytes to be allocated */
long num_parts;			/* num LPs in the LV */
short int lv_index;		/* index for LVs */
short int pp_index;		/* index for PPs */
short int mir_index;		/* index for mirror copies */
int retcode;


/* 
 * copy the array of LV structs into kernel space 
 */
retcode = copyin (lv_ptrs, klv_ptrs, sizeof (klv_ptrs));
if (retcode != LVDD_SUCCESS)
    return (EFAULT);

/* 
 * for each LV in the VG, define to the kernel the LV structs  
 * and their corresponding partition arrays of PP structs.
 */
for (lv_index = 1; lv_index < MAXLVS; lv_index = lv_index + 1)
{

   lv_ptr =  klv_ptrs [lv_index];
   /* 
    * if this LV exists, allocate it's lvol struct, copy in it's data
    */ 
   if (lv_ptr != NULL)
   {
       klv_ptr = (struct lvol *) xmalloc ((uint) sizeof (struct lvol),
					 HD_ALIGN, pinned_heap);
       if (klv_ptr == NULL)		/* if error occurred */
	  /*
	   * No backout needed, since the entire VG will be deleted by 
	   * varyonvg if an error occurs.
	   */
	  return (ENOMEM);
        
       kvg_ptr -> lvols [lv_index] = klv_ptr;
       retcode = copyin (lv_ptr, klv_ptr, sizeof (struct lvol));
       if (retcode != LVDD_SUCCESS)
	  /*
	   * No backout needed, since the entire VG will be deleted by 
	   * varyonvg if an error occurs.
	   */
	  return (EFAULT);

       num_parts = BLK2PART (kvg_ptr -> partshift, klv_ptr -> nblocks);
       size = num_parts * sizeof (struct part);

       /*
	* for each mirror copy of this LV, allocate space for this
	* partition array & copy in it's data
	*/
       for (mir_index = 0; mir_index < klv_ptr -> nparts;
	   mir_index = mir_index + 1)
       {

	   u_part_ptr = klv_ptr -> parts [mir_index];
	   klv_ptr -> parts [mir_index] = (struct part *) xmalloc (
		   (uint) size, HD_ALIGN, pinned_heap);
	   if (klv_ptr -> parts [mir_index] == NULL)
	       /*
	        * No backout needed, since the entire VG will be deleted by 
	        * varyonvg if an error occurs.
	        */
	       return (ENOMEM);

	   retcode = copyin (u_part_ptr, klv_ptr -> parts [mir_index], size);
	   if (retcode != LVDD_SUCCESS)
	       /*
	        * No backout needed, since the entire VG will be deleted by 
	        * varyonvg if an error occurs.
	        */
	       return (EFAULT);

	   kpp_ptr = klv_ptr -> parts [mir_index];

	   /* 
	    * for each PP in this mirror, set it's pvol pointer 
	    * by using the PV index that is currently stored there
	    */
	   for (pp_index = 0; pp_index < num_parts; pp_index = pp_index + 1)
	   {
	       if (kpp_ptr -> pvol != NULL)
	           kpp_ptr -> pvol = kvg_ptr -> pvols [(int) kpp_ptr->pvol -1];

	       kpp_ptr = (struct part *) ((caddr_t) kpp_ptr +
				      sizeof (struct part));
	   } 

       } /* loop for each mirror copy */

   } /* end if this logical volume is defined */

} /* loop for each possible logical volume */

return (LVDD_SUCCESS);

} 




/***********************************************************************
 *                                                                     *
 * NAME:  hd_ksetupvg                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine calls hd_kdeflvs to define the LV structs for all    *
 *   the LVs in the VG, initializes the VGSA/MWCC information in the   *
 *   volgrp and pvol structures, and does the MWCC Fast Path recovery. *
 *                                                                     *
 * NOTES:                                                              *
 *      No backout code needed when an error occurs, since the         *
 *   	varyonvg lib routine calls hd_kdelvg to delete the entire VG   *
 *      if hd_ksetupvg fails.					       *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     ksetupvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   copyin                                                            *
 *   devswqry                                                          *
 *   xmalloc                                                           *
 *                                                                     *
 ***********************************************************************
 */


int
hd_ksetupvg (

dev_t device,			/* device number of the VG to be configured */
struct ksetupvg * ksetupvg)	/* ptr to info to setup the VG */

{ 


struct volgrp * kvg_ptr;	/* kernel VG struct */
struct mwc_rec * mwcc;		/* kernel copy of MWCC recovery info */
daddr_t vgsa_lsn[MAXPVS][PVNUMVGDAS];  /* VGSA LSNs for each PV in VG */
struct kdelvg kdelvg;		/* struct to send to hd_kdelvg */
int lrc;			/* return code from lockl */
int size;
int i;
int retcode;


/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, ksetupvg->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        ksetupvg -> rc = CFG_INVVGID;
        return (EINVAL);
    }
    else
	return(retcode);
}
    
/* 
 * Define the LV structures for all LV's in the VG.
 */
if (retcode = hd_kdeflvs(kvg_ptr, ksetupvg -> lv_ptrs) != LVDD_SUCCESS) 
{
    if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
    return (retcode);
}
    
/* 
 * Initialize the kernel volgrp structure with all the 
 * VGSA memory and values needed.
 */

/* allocate space for the VG's VGSA */
size = sizeof(struct vgsa_area);
kvg_ptr -> vgsa_ptr = (struct vgsa_area *) xmalloc((uint) size,
			HD_ALIGN, pinned_heap);
if (kvg_ptr -> vgsa_ptr == NULL)
{
    /*
     * No backout needed, since the entire VG will be deleted by 
     * varyonvg if an error occurs.
     */
    if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
    return (ENOMEM);
}

/* copy the VGSA into kernel space  */
retcode = copyin (ksetupvg -> vgsa_ptr, kvg_ptr -> vgsa_ptr, size);
if (retcode != LVDD_SUCCESS)
{
    /*
     * No backout needed, since the entire VG will be deleted by 
     * varyonvg if an error occurs.
     */
     if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
     return (EFAULT);
}

/* 
 * Initialize the kernel pvol structs with the vgsa info 
 */

/* copy the array of VGSA LSNs for the PVs into kernel space  */
retcode = copyin (ksetupvg -> salsns_ptr, vgsa_lsn, sizeof(vgsa_lsn));
if (retcode != LVDD_SUCCESS)
{
    /*
     * No backout needed, since the entire VG will be deleted by 
     * varyonvg if an error occurs.
     */
     if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
     return (EFAULT);
}

for (i=0; i<MAXPVS; i++) 
{
    /* if this PV is defined, setup it's VGSA's LSN & sequence number */
    if (kvg_ptr -> pvols[i])  
    {
	 kvg_ptr -> pvols[i] -> sa_area[0].lsn = vgsa_lsn[i][0];
	 kvg_ptr -> pvols[i] -> sa_area[1].lsn = vgsa_lsn[i][1];
	 kvg_ptr -> pvols[i] -> sa_area[0].sa_seq_num = 0;
	 kvg_ptr -> pvols[i] -> sa_area[1].sa_seq_num = 0;
    }
}
	
/* Initialize the VG's quorum count */
kvg_ptr -> quorum_cnt = ksetupvg -> quorum_cnt;	

/* if there is MWCC FAST PATH recovery needed */
if (ksetupvg -> num_ltgs > 0)
{
     /* Allocate space for this VG's MWCC disk part - to be used in
        FAST PATH recovery only - NOT to be put in volgrp struct! */
    mwcc = (struct mwc_rec *) xmalloc((uint) 
			sizeof(struct mwc_rec), HD_ALIGN, pinned_heap);
    if (mwcc == NULL)
    {
       /*
        * No backout needed, since the entire VG will be deleted by 
        * varyonvg if an error occurs.
        */
        if (lrc == LOCK_SUCC) 
   	    unlockl(&(kvg_ptr -> vg_lock));
        return (ENOMEM);
    }

    /* copy the MWCC into kernel space */
    retcode = copyin (ksetupvg -> mwcc_ptr, mwcc, sizeof (struct mwc_rec));
    if (retcode != LVDD_SUCCESS)
    {
       /*
        * No backout needed, since the entire VG will be deleted by 
        * varyonvg if an error occurs.
 	* Except for the mwcc that is temporary space.
        */
        assert(xmfree((caddr_t)mwcc, pinned_heap) == LVDD_SUCCESS);
        if (lrc == LOCK_SUCC)
	    unlockl(&(kvg_ptr -> vg_lock));
        return (EFAULT);
    }

    /* execute the MWCC Fast Path Recovery now */
    retcode = hd_mwc_fprec(mwcc, ksetupvg -> num_ltgs, kvg_ptr);

    /* free the local copy of the MWCC */
    assert (xmfree((caddr_t)mwcc, pinned_heap) == LVDD_SUCCESS); 

    if (retcode != LVDD_SUCCESS)
        {
        /* No backout needed, since the entire VG will be deleted by 
        varyonvg if an error occurs. */
        if (lrc == LOCK_SUCC) 
	    unlockl(&(kvg_ptr -> vg_lock));
	if (retcode == CFG_FORCEOFF)  
            {
	    ksetupvg -> rc = CFG_FORCEOFF;
            return (EINVAL);
	    }
	else 
	    return (retcode);
        }

    /* allocate MWC memory resources in order to update MWC log */
    if ((retcode = alloc_mwc_mem(kvg_ptr)) != LVDD_SUCCESS)
        {
        if (lrc == LOCK_SUCC) 
            unlockl(&(kvg_ptr -> vg_lock));
        return(retcode);
        }

    /* clear the MWC log after recovery is complete */
    retcode = hd_kdis_initmwc(kvg_ptr);
    if (retcode != LVDD_SUCCESS) 
        {
        if (lrc == LOCK_SUCC) 
            unlockl(&(kvg_ptr -> vg_lock));
        return(retcode);
        }
}

/* if varying on to SYSTEM MANAGEMENT ONLY mode, then remove the
 * lvol structures (except for LV0) from the kernel structures 
 */
if (kvg_ptr -> flags & VG_SYSMGMT)
    hd_free_lvols(kvg_ptr, HD_1STUSRLV, MAXLVS);

/* VG is now ready to go!   Turn off VG_OPENING */
kvg_ptr -> flags &= ~VG_OPENING;

if (lrc == LOCK_SUCC) 
    unlockl(&(kvg_ptr -> vg_lock));

return(LVDD_SUCCESS);
}



/***********************************************************************
 *                                                                     *
 * NAME:  hd_mwc_fprec                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine performs MWCC fast path recovery.  Each LTG with an  *
 *   entry in the MWCC passed in will be resynchronized with it's      * 
 *   mirrored copies by doing a READX mwcc recovery operation to it's  *
 *   LV.                                                               *
 *                                                                     *
 * NOTES:                                                              *
 *	The mwcc entries should be ordered by LV minor number.         *
 *      This routine assumes that the DALV (LV0) is already opened     *
 *	by varyonvg (this is only to keep one LV open in this VG so    *
 * 	that data does not need to be reallocated at each LV open).    *
 *                                                                     *
 *   INPUT:                                                            *
 *     mwcc                                                            *
 *     numltgs                                                         *
 *     vg                                                              *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     If successful, each LTG with an entry in the MWCC is            *
 *     resynchronized.  Otherwise, if it's LV could not be opened,     *
 *     all of it's partitions but one will be marked stale.  If the    *
 *     read recovery operation failed, the VG is forced off.           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_mwc_fprec (

struct mwc_rec *mwcc,		/* mirror write consistence cach */           
int numltgs,			/* number of LTG entries in the mwcc */
struct volgrp * vg)		/* volgrp ptr */

{ 


int lvopen_failed = FALSE;	/* flag set if LV open failed */
int open_minor = -1;		/* minor number of the LV currently opened */
struct file *fp;                /* file pointer from open of LV     */
dev_t device;			/* major/minor of the LV */
struct lvol * lv;		/* lvol ptr */
int offset; 			/* LTG offset into the LV */
int count;			/* count returned from fp_read */
int vgsa_updated = FALSE;	/* if set, VGSA needs to be written out */
caddr_t rec_buf;		/* buffer sent to MWCC recovery read */
int i;
int rc = LVDD_SUCCESS;

/*
 * allocate the buffer used during the MWCC recovery read operation  
 */
rec_buf = (caddr_t) xmalloc(BYTEPTRK, HD_ALIGN, pinned_heap);
if (rec_buf == NULL)
    return(ENOMEM);

/*
 * loop thru the MWCC entries and try to resync each LTG 
 */
for (i=0; i<numltgs; i++)  {

    /* 
     * if not already done, open the LV of this LTG
     */
    if (mwcc->ca_p1[i].lv_minor != open_minor)  {
	/*	
	 * if not the 1st time through, close the last LV before
	 * opening the next one, or reset the failed flag if the
	 * last LV open failed.
	 */
	if (open_minor != -1)  {
	    if (lvopen_failed == TRUE)
		lvopen_failed = FALSE;
	    else
		fp_close(fp);
	}

	device = makedev(vg->major_num, mwcc->ca_p1[i].lv_minor);
	rc = fp_opendev(device, DREAD, 0, 0, &fp);

	/* 
	 * if LV open fails, set the flag so disaster recovery will
	 * be performed on this LTG's LP.
 	 */
	if (rc != LVDD_SUCCESS)  {
	    lvopen_failed = TRUE;
	    rc = LVDD_SUCCESS;		/* not fatal, so reset rc */
	}
	open_minor = (int) (mwcc->ca_p1[i].lv_minor);

    }

    if (lvopen_failed == TRUE) 
	/* 
	 * do disaster recovery on this LTG's LP - mark all but 
	 * one copy of this LP stale
 	 */
  	hd_disastrec(vg, mwcc->ca_p1[i].lv_ltg, open_minor, &vgsa_updated);	
    else  {	
	/*
	 * perform the READ MWCC recovery operation for this LTG
	 */
	offset = BLK2BYTE (TRK2BLK(mwcc->ca_p1[i].lv_ltg));
	(void) fp_lseek(fp, offset, SEEK_SET);

	rc = fp_read(fp, rec_buf, BYTEPTRK, MWC_RCV_OP, UIO_SYSSPACE, &count);
	/*	
	 * if the READ MWCC recovery failed and the VG is closing, then
	 * the VG is being forced offline - return with error.  If recovery
	 * failed but VG not closing, then do disaster recovery on this LTG.
	 */
	if ((rc != LVDD_SUCCESS) || (count != BYTEPTRK))  {
	    if (vg->flags & VG_FORCEDOFF) {
	        rc = CFG_FORCEOFF;
	        break;
	    }
	    else
	    {
		/* 
	 	* do disaster recovery on this LTG's LP - mark all but 
	 	* one copy of this LP stale
 	 	*/
  		hd_disastrec(vg, mwcc->ca_p1[i].lv_ltg, open_minor, 
						&vgsa_updated);	
		rc = LVDD_SUCCESS ;
	    }
	}

    }

}	/* end of for mwcc ltgs */
	
/*
 * free the buffer used during the MWCC recovery read operation  
 */
assert (xmfree(rec_buf, pinned_heap) == LVDD_SUCCESS);

/* 
 * close the last user LV 
 */
if (lvopen_failed == FALSE)
    fp_close(fp);

/* 
 * if VGSA was updated during the recovery, write it out to all PVs
 * via the WHEEL
 */
if ((vgsa_updated == TRUE) && (rc == LVDD_SUCCESS)) {
    rc = hd_sa_config(vg, HD_MWC_REC, (caddr_t) NULL);
    if (rc == FAILURE)
    {
	/* if VGSA updating failed, then the VG is being forced off-line */
	rc = CFG_FORCEOFF;
    }
    else
    {
        rc = LVDD_SUCCESS ;
    }
}

return(rc);

}


/***********************************************************************
 *                                                                     *
 * NAME:  hd_disastrec                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *	This routine performs disaster recovery (mark all but one PP   *
 *      stale) on a LTG.					       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *	vg							       *
 *	ltg							       *
 * 	lv_minor					 	       *
 *	vgsa_updated						       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LP that this LTG resides on has been recovered.             *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 ***********************************************************************
 */


void
hd_disastrec (

struct volgrp *vg,		/* ptr to volume group */
ulong 	       ltg,		/* LTG to be recovered */
int	       lv_minor,	/* LV minor number */
int	      *vgsa_updated) 	/* flag set if VGSA is updated */

{

struct lvol * lv;		/* lvol ptr */
struct part * pp;		/* partition ptr */
int lpnum;			/* logical partition number */
int ppnum;			/* physical partition number */
int fresh_copy;			/* save the PP that is fresh and non-missing */
int copy;


/* 
 * do disaster recovery on this LTG's LP - mark all but 
 * one copy of this LP stale
 */
lv = vg->lvols[lv_minor];
lpnum = BLK2PART(vg->partshift, TRK2BLK(ltg));

/* search for the first non-stale/non-missing copy */
for (copy=0; copy<lv->nparts; copy++)  {

    pp = PARTITION(lv, lpnum, copy);
    if ((pp->pvol) && !(pp->ppstate & PP_STALE)) {
	fresh_copy = copy;
	if (pp->pvol->pvstate != PV_MISSING)
	    break;		/* found a fresh/non-missing PP */
    }
}

/* mark all but the one picked copy stale */
for (copy=0; copy<lv->nparts; copy++)  {
    pp = PARTITION(lv, lpnum, copy);
    ppnum = BLK2PART(vg->partshift, pp->start - pp->pvol->fst_usr_blk);

    if (pp->pvol && !(pp->ppstate & PP_STALE) && (fresh_copy != copy)) {
	SETSA_STLPP(vg->vgsa_ptr, pp->pvol->pvnum, ppnum);
	pp->ppstate |= PP_STALE;
	*vgsa_updated = TRUE;
    }
}

return;
	
}




/***********************************************************************
 *                                                                     *
 * NAME:  hd_kdefvg                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine copies the LVDD volume group structure and the       *
 *   logical volume structure for the descriptor area logical volume   *
 *   from user space to kernel space.                                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kdefvg                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     This volume group has been added to the device switch table.    *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kdefvg (

dev_t device,			/* device number of the VG to be configured */
struct kdefvg * kdefvg)		/* ptr to info to define the VG */


{

struct volgrp * kvg_ptr;	/* kernel area VG structs */
struct lvol * klv_ptr;		/* kernel area LV structs */
struct devsw dev_switch;	/* device switch entry to be added for VG */
long size;			/* num bytes of memory to be allocated */
long num_parts;			/* num of LPs in a LV */
int lrc;			/* return code from lockl */
int retcode;


/*
 * allocate kernel space for VG struct & copy it's data in 
 */
kvg_ptr = (struct volgrp *) xmalloc ((uint) sizeof (struct volgrp),
				     HD_ALIGN, pinned_heap);

if (kvg_ptr == NULL)
    return (ENOMEM);
retcode = copyin (kdefvg -> vg_ptr, kvg_ptr, sizeof (struct volgrp));
if (retcode != LVDD_SUCCESS) 
{
    assert (xmfree ((caddr_t) kvg_ptr, pinned_heap) == LVDD_SUCCESS);
    return (EFAULT);
}

/* set VG flag to OPENING, so that no processing of this VG or it's
 * LVs will be allowed until configuration is complete.
 * Save the process ID of this varyon process in the volgrp struct so 
 * that LVDD open routine will not allow ANY LV opens unless the requesting
 * process has a PID of the one saved in the VG.
 */
kvg_ptr -> flags |= VG_OPENING;
kvg_ptr -> von_pid = getpid();

/* initialize MWC pointer to indicate no MWC resources allocated yet */
kvg_ptr->mwc_rec = NULL;

/*
 * allocate kernel space for Descriptor Area LV struct & copy it's data in 
 */
klv_ptr = (struct lvol *) xmalloc ((uint) sizeof (struct lvol), HD_ALIGN,
				   pinned_heap);
if (klv_ptr == NULL)
{
    /* if error, backout VG struct */
    assert (xmfree ((caddr_t) kvg_ptr, pinned_heap) == LVDD_SUCCESS);
    return (ENOMEM);
}
kvg_ptr -> lvols [HD_RSRVDALV] = klv_ptr;
retcode = copyin (kdefvg -> dalv_ptr, klv_ptr, sizeof (struct lvol));
if (retcode != LVDD_SUCCESS)
{
    assert (xmfree ((caddr_t) klv_ptr, pinned_heap) == LVDD_SUCCESS);
    assert (xmfree ((caddr_t) kvg_ptr, pinned_heap) == LVDD_SUCCESS);
    return (EFAULT);
}

/*
 * allocate & initialize in the DA LV's partition structs
 */
num_parts = BLK2PART (kvg_ptr -> partshift, klv_ptr -> nblocks);
size = num_parts * sizeof (struct part);
klv_ptr -> parts [PRIMMIRROR] = (struct part *) xmalloc ((uint) size,
			HD_ALIGN, pinned_heap);
/* if an error occurred, backout previously allocated structs */
if (klv_ptr -> parts [PRIMMIRROR] == NULL)
{
    assert (xmfree ((caddr_t) klv_ptr, pinned_heap) == LVDD_SUCCESS);
    assert (xmfree ((caddr_t) kvg_ptr, pinned_heap) == LVDD_SUCCESS);
    return (ENOMEM);
}
bzero ((caddr_t) klv_ptr -> parts [PRIMMIRROR], size);

/* 
 * pin the LVDD code
 */
retcode = pincode((int (*) ())hd_upd_bbdir);
if (retcode != LVDD_SUCCESS)
    return(EFAULT);


/*
 * Initialize entry in the device switch table for this volume group 
 * and add it to the device switch table
 */
bzero ((caddr_t) &dev_switch, sizeof (struct devsw));
dev_switch.d_config = hd_config;	/* LVDD configuration routine */
dev_switch.d_open =  hd_open;		/* LVDD open routine */
dev_switch.d_close = hd_close;		/* LVDD close routine */
dev_switch.d_read =  hd_read;		/* LVDD character read routine*/
dev_switch.d_write = hd_write;		/* LVDD character write rtn */
dev_switch.d_strategy = (int(*) ( )) hd_strategy;	/* LVDD strategy rtn */
dev_switch.d_ioctl = hd_ioctl;		/* LVDD ioctl routine */
dev_switch.d_dump = hd_dump;		/* LVDD dump routine */
dev_switch.d_select = nodev;		/* not supported */
dev_switch.d_print = nodev;		/* not supported */
dev_switch.d_mpx = nodev;		/* not supported */
dev_switch.d_revoke = nodev;		/* not supported */
dev_switch.d_dsdptr = (caddr_t) kvg_ptr;		/* LVDD VG structure */
dev_switch.d_opts = DEV_MPSAFE;

retcode = hd_kdis_dswadd (device, &dev_switch);
/* if an error occurred, backout previously allocated structs */
if (retcode != LVDD_SUCCESS)
{
    assert (unpincode ((int (*) ())hd_upd_bbdir) == LVDD_SUCCESS);
    assert (xmfree ((caddr_t) klv_ptr -> parts [PRIMMIRROR], pinned_heap) ==
						       LVDD_SUCCESS);
    assert (xmfree ((caddr_t) klv_ptr, pinned_heap) == LVDD_SUCCESS);
    assert (xmfree ((caddr_t) kvg_ptr, pinned_heap) == LVDD_SUCCESS);
    if (retcode == CFG_MAJUSED) {
	kdefvg -> rc = CFG_MAJUSED;
	return (EINVAL);
    }
    return (retcode);
}

/*
 *   Add this volume group to the linked list of volume groups in the 
 *   kernel.                                                         
 */
lrc = lockl (&(hd_vghead.lock), LOCK_SHORT);		/* lock the VG */
kvg_ptr -> nextvg = hd_vghead.ptr;
hd_vghead.ptr = kvg_ptr;

if (lrc == LOCK_SUCC) 			/* if no nested locks,unlock vg */
    unlockl(&(hd_vghead.lock));

/* initialize status area lock */
lock_alloc(&(kvg_ptr->sa_intlock), LOCK_ALLOC_PIN, LVM_LOCK_CLASS, 
           kvg_ptr->major_num);
simple_lock_init(&(kvg_ptr->sa_intlock));

return (LVDD_SUCCESS);

} 






/***********************************************************************
 *                                                                     *
 * NAME:  hd_kdelvg                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine calls the LVDD routine to terminate processing for   *
 *   a volume group and deletes the LVDD data structures for that      *
 *   volume group.                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kdelvg                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The volgrp structure, the list of lvol structures and the list  *
 *     of part structures for each copy of each logical volume, and    *
 *     the list of pvol structures and all linked lists of bad block   *
 *     structures for each physical volume have all been deleted and   *
 *     the space freed.                                                *
 *                                                                     *
 *     This volume group is deleted from the device switch table.      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kdelvg (
dev_t device,			/* device number of the VG to be configured */
struct kdelvg * kdelvg)		/* ptr to info to delete the VG */
{ 
struct volgrp *kvg_ptr;		/* ptr to kerenl VG structure */
short int lv_index;		/* LV struct index */
int retcode;
int lrc, lrc_head;		/* return codes from lockl */

/* lock the list of active VGs */
lrc_head = lockl (&(hd_vghead.lock), LOCK_SHORT);

/* get the volgrp pointer from devsw table, lock the VG struct 
   and verify that the VG passed in is correct */
retcode = hd_verifyvgid(device, kdelvg->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kdelvg -> rc = CFG_INVVGID;
        retcode = EINVAL;
    }
    if (lrc_head == LOCK_SUCC)    
        unlockl(&(hd_vghead.lock));	/* unlock global VG list */
    return(retcode);
}

/*
 * verify that no LVs are open
 */
if (kvg_ptr->open_count  != 0) {
    if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
    if (lrc_head == LOCK_SUCC)    
        unlockl(&(hd_vghead.lock));	/* unlock global VG list */
    return (EBUSY);
}
     
/* 
 * If the VG is being FORCED off-line, the LVDD will cleanup the VG
 * at the last LV close.
 */
if (kvg_ptr -> flags & VG_FORCEDOFF) { 
    kdelvg -> rc = CFG_FORCEOFF;
    if (lrc == LOCK_SUCC) 
	unlockl(&(kvg_ptr -> vg_lock));
    if (lrc_head == LOCK_SUCC)    
        unlockl(&(hd_vghead.lock));	/* unlock global VG list */
    return (EINVAL);
}    


/*
 * If the volume group is to remain varied on for system management 
 * functions only, then quiesce the VG and delete all logical volume 
 * structures except the one the DALV (minor number 0).
 * Otherwise, clean up the whole VG.      
 */

if (kdelvg -> lvs_only == TRUE)	{	/* if system mgmt only */
    hd_free_lvols(kvg_ptr, HD_1STUSRLV, MAXLVS);
    kvg_ptr -> flags |= VG_SYSMGMT;	
    if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
	unlockl(&(kvg_ptr -> vg_lock));
}
else {					/* varying completely off */
    hd_vgcleanup (kvg_ptr);
    /*
     * Attempt to remove the major number from the device switch table
     * if it succeeds then free the volgrp structure otherwise just return.
     */
    retcode = devswdel( device );

    if (lrc == LOCK_SUCC) /* unlock volume group structures */
	unlockl(&(kvg_ptr->vg_lock));

    if( retcode == LVDD_SUCCESS ) {
        lock_free(&(kvg_ptr->sa_intlock));
	assert( xmfree(kvg_ptr, pinned_heap) == LVDD_SUCCESS );
	assert( unpincode((int(*) ())hd_upd_bbdir)==LVDD_SUCCESS);
    }
    else
	/*
	 * If the devswdel fails there is an open sitting on the lock.
	 * Therefore, set the forced flag to tell the open to fail and
	 * hd_close to finish the VG cleanup.
	 */
	kvg_ptr -> flags |= VG_FORCEDOFF;
}
if (lrc_head == LOCK_SUCC)    
    unlockl(&(hd_vghead.lock));	/* unlock global VG list */
return(LVDD_SUCCESS);
}


/***********************************************************************
 *                                                                     *
 * NAME:  hd_free_lvols                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   Frees all lvol structs and their part structs                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_ptr                                                          *
 *     lv_index                                                        *
 *     lv_end                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     All lvol structs and their part structs are freed.              *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   xmfree                                                            *
 *                                                                     *
 ***********************************************************************
 */


void
hd_free_lvols (

struct volgrp *kvg_ptr,		/* ptr to volgrp struct */
int lv_index,			/* lv index into lvols array to begin freeing */
int lv_end)			/* lv index into lvols array to end freeing */

{ 

int copy_index;			/* index for part structures */
struct lvol *klv_ptr;		/* ptr to lvol struct */

/*
 *   Delete LV data structures and the PP structs
 *   associated with them.  Note that a starting index of  
 *   either minor number 0 or 1 determines whether the descriptor area
 *   logical volume (DALV) is included.                                     
 */
for (; lv_index < lv_end; lv_index++)
{
    klv_ptr = kvg_ptr -> lvols [lv_index];
    if (klv_ptr != NULL)
    {
	/* delete each mirror in the LV */
        for (copy_index=0; copy_index<klv_ptr->nparts; copy_index++)
	{
	   if (klv_ptr -> parts [copy_index] != NULL)
	   {
	       assert(xmfree ((caddr_t) klv_ptr -> parts [copy_index],
			      pinned_heap) == LVDD_SUCCESS);
	    }
	}

	/* if LV is open, then we have a problem */
	assert (klv_ptr -> lv_status == LV_CLOSED);

	kvg_ptr -> lvols [lv_index] = NULL;
	assert (xmfree ((caddr_t) klv_ptr, pinned_heap) == LVDD_SUCCESS);
    } 
}

return;
}

/***********************************************************************
 *                                                                     *
 * NAME:  hd_kchgqrm                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   	This function changes the VG's quorum count.  It first verifies*
 * 	that the quorum will not be lost.			       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kchgqrm                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kchgqrm (

dev_t device,			/* device number of the VG to be queried */
struct kchgqrm * kchgqrm)	/* ptr to info to change the VG quorum */

{ 

struct volgrp *kvg_ptr;		/* ptr to kerenl VG structure */
int lrc;			/* ret code from lockl */
int retcode = LVDD_SUCCESS;

/*
 * get the volgrp pointer from devsw table, lock the VG struct
 * and verify that the VG passed in is correct
 */
retcode = hd_verifyvgid(device, kchgqrm->vg_id, &kvg_ptr, &lrc);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_INVVGID) {
        kchgqrm -> rc = CFG_INVVGID;
	retcode = EINVAL;
    }
    return(retcode);
}
    
/* 
 * disabled, verify that the new quorum count will not cause a force-off
 * and then modify the vg's quorum count. 
 */
retcode = hd_kdis_chgqrm(kvg_ptr, kchgqrm->quorum_cnt);
if (retcode != LVDD_SUCCESS) {
    if (retcode == CFG_BELOWQRM) {
	kchgqrm->rc = retcode;
	retcode = EINVAL;
    }
}

if (lrc == LOCK_SUCC) 		/* if no nested locks,unlock vg */
    unlockl(&(kvg_ptr -> vg_lock));
return(retcode);

}




/***********************************************************************
 *                                                                     *
 * NAME:  hd_kqryvgs                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine gets the vg_id and the major number for each         *
 *   volume group which is varied on in the system and returns         *
 *   it in the user buffer.  It also records the total number of       *
 *   volume groups found.                                              *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kqryvgs                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The list of volume group ids and major numbers and a total      *
 *     count of the volume groups which are varied on.                 *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kqryvgs (

dev_t device,			/* device number of the VG to be queried */
struct kqryvgs * kqryvgs)	/* ptr to info to query the VG */

{ 

struct queryvgs k_queryvgs;	/* kernel query info for varied-on VGs */
struct volgrp * cur_ptr;	/* current VG struct */
int counter;			/* index into VG ID array */
int lrc;			/* return code from lockl */
int retcode;


/* 
 * init VG query structures to zeros & lock VG linked list 
 */
bzero ((caddr_t) &k_queryvgs, sizeof (struct queryvgs));
counter = 0;
lrc = lockl (&(hd_vghead.lock), LOCK_SHORT);
cur_ptr = hd_vghead.ptr;		/* current VG */

/* 
 * for each varied-on VG, store it's VGID and major number
 */
while (cur_ptr != NULL)
{
    k_queryvgs.vgs[counter].vg_id.word1 = cur_ptr -> vg_id.word1;
    k_queryvgs.vgs[counter].vg_id.word2 = cur_ptr -> vg_id.word2;
    k_queryvgs.vgs[counter].major_num = cur_ptr -> major_num;
    counter ++;				/* counter of varied-on VGs */
    cur_ptr = cur_ptr -> nextvg;
}

k_queryvgs.num_vgs = counter;		/* set number of varied-on VGs */

/* 
 * copy query info to user space
 */
retcode = copyout (&k_queryvgs, kqryvgs -> queryvgs,
		   sizeof (struct queryvgs));
if (retcode != LVDD_SUCCESS)
{
    if (lrc == LOCK_SUCC) 
	unlockl(&(hd_vghead.lock));
    return (EFAULT);
}

if (lrc == LOCK_SUCC) 
   unlockl(&(hd_vghead.lock));

return (LVDD_SUCCESS);

}



/***********************************************************************
 *                                                                     *
 * NAME:  hd_krebuild                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine queries the kernel structures to return the          *
 *   fields necessary to rebuild the volume group file.                *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     krebuild                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     quorum, num_desclps, and the device, logical sector numbers     *
 *     for the VGSA and the pvstate for each pv in the volume group    *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
hd_krebuild (

dev_t device,
  /* device code which consists of the major and minor number of the
     volume group to be configured */

struct krebuild * krebuild)
  /* pointer to structure which contains a pointer
     to a buffer returned to the user for rebuilding the volume group file
   */


{ 
	struct rebuild  kern_rb;	/* kernel space twin to krebuild */
        struct volgrp *vg;		/* pointer to volgrp structure */
	struct lvol *lv;		/* pointer to logical volume struct */
	struct pvol *pv;		/* pointer to physical volume struct */
	int cnt;			/* counter of for loop */
	int idx;			/* counter of for loop */
	int lrc,rc;			/* return code */

	/* get pointer to volgrp struct from device switch table */
	rc = devswqry(device,NULL,(caddr_t *) &vg);
	ASSERT(rc == LVDD_SUCCESS);
	/* lock the volume group structure */	
	lrc = lockl (&(vg->vg_lock), LOCK_SHORT);

        /* zero out the kernel rebuild structure */
	bzero ((caddr_t) &kern_rb, sizeof (struct rebuild));
	
	/* fill in the quorum count from the volume group structure */
	kern_rb.quorum = vg->quorum_cnt;
	
	
	/* go throught pvols to fill in the fields the user needs */
	for(cnt = 0; cnt < MAXPVS; cnt ++) {
	    pv = VG_DEV2PV(vg,cnt);
	    if( pv ) {
		kern_rb.pv[pv->pvnum].device = pv->dev;
		kern_rb.pv[pv->pvnum].pvstate = pv->pvstate;
		kern_rb.pv[pv->pvnum].salsn[0] = pv->sa_area[0].lsn;
		kern_rb.pv[pv->pvnum].salsn[1] = pv->sa_area[1].lsn;
	    }
	}
	
	/* get a pointer to logical volume 0 */
	lv = VG_DEV2LV(vg,device);
	
	/* calculate the number of descriptor area logical partitions */
	kern_rb.num_desclps = (BLK2PART(vg->partshift,lv->nblocks)+1)/MAXPVS;

	/* copy the kernel structure to its twin user structure */
	rc = copyout(&kern_rb, krebuild->rebuild, sizeof(struct rebuild));
	if (rc != LVDD_SUCCESS) {
      		/* unlock the linked list of volume groups */
		if(lrc == LOCK_SUCC)
    		   unlockl (&(vg->vg_lock));
      		/* return error for bad address */
    		return (EFAULT);
    	}
	/* unlock the volume group structure */
        if(lrc == LOCK_SUCC)
	   unlockl (&(vg->vg_lock));

  	/* return with successful return code */
	return (LVDD_SUCCESS);
} 




/***********************************************************************
 *                                                                     *
 * NAME:  hd_kvgstat                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function determines if the specified volume group is already *
 *   varied on, and, if so, whether it is fully varied on or varied on *
 *   for system management only.                                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     device                                                          *
 *     kvgstat                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     kvgstat                                                         *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 ***********************************************************************
 */


int
hd_kvgstat (

dev_t device,			/* device number of the VG to be queried */
struct kvgstat * kvgstat)	/* ptr to info for VG stat */

{

struct volgrp * curvg_ptr;
int lrc, lrc_head;
int retcode;


lrc_head = lockl (&(hd_vghead.lock), LOCK_SHORT);	/* lock VG list */

curvg_ptr = hd_vghead.ptr;

/*
 * search for the VG in the list of varied-on VGs 
 */
while (curvg_ptr != NULL)
{
  if (kvgstat -> vg_id.word1 == curvg_ptr -> vg_id.word1  &&
      kvgstat -> vg_id.word2 == curvg_ptr -> vg_id.word2)
      break;			/* found VG - break out */
  curvg_ptr = curvg_ptr -> nextvg;
}

/* 
 * If VG was found, save it's VG status and major number
 * else mark that it's not varied on. 
 */
if (curvg_ptr != NULL)
{

    lrc = lockl (&(curvg_ptr -> vg_lock), LOCK_SHORT); 	/* lock VG */
    if (curvg_ptr -> flags & VG_SYSMGMT)	/* if system mgmt only mode */
	kvgstat -> status = HD_NOLVSVON;	/* no LVs defined */
    else
	kvgstat -> status = HD_FULLVON;		/* VG fully varied-on */
    kvgstat -> vg_major = curvg_ptr -> major_num;
    if (lrc == LOCK_SUCC) 
	unlockl(&(curvg_ptr -> vg_lock));
}
else		/* VG not found */
    kvgstat -> status = HD_NOTVON;		/* VG NOT varied on */

if (lrc_head == LOCK_SUCC) 
   unlockl(&(hd_vghead.lock));			/* unlock VG list */

return (LVDD_SUCCESS);

}
