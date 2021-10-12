static char sccsid[] = "@(#)40	1.3  src/bos/usr/ccs/lib/liblvm/setupvg.c, liblvm, bos411, 9428A410j 3/4/94 17:32:32";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - setupvg.c
 *
 * FUNCTIONS: lvm_setupvg,
 *            lvm_bldklvlp,
 *            lvm_mwcinfo
 *
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


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <errno.h>;
#include <fcntl.h>;
#include <sys/buf.h>;
#include <sys/lockl.h>;
#include <sys/param.h>;
#include <sys/sleep.h>;
#include <sys/stat.h>;
#include <sys/types.h>;
#include <sys/time.h>;
#include <sys/shm.h>;
#include <sys/sysmacros.h>;
#include <sys/dasd.h>;
#include <sys/hd_psn.h>;
#include <sys/bootrecord.h>;
#include <lvmrec.h>;
#include <sys/hd.h>;
#include <sys/bbdir.h>;
#include <sys/vgsa.h>;
#include <sys/hd_config.h>;
#include <liblvm.h>;









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_setupvg                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine builds the LVDD data structures which define the     *
 *   volume group's logical volumes.  It also builds the mirror        *
 *   write consistency information to send to the kernel.              *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     maphdr_ptr                                                      *
 *     vg_fd                                                           *
 *     vgda_ptr                                                        *
 *     vgsa_ptr                                                        *
 *     vgsa_lsn                                                        *
 *     mwcc                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LVDD data structures for the volume group have been         *
 *     defined into the kernel for logical volumes.  Also, resync has  *
 *     been done in the kernel for logical track groups in the mirror  *
 *     write consistency cache.                                        *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return code >= 0 indicates successful return.                   *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_setupvg (

struct varyonvg * varyonvg,
  /* pointer to structure which contains the input information for
     the lvm_varyonvg subroutine */

struct inpvs_info * inpvs_info,
  /* pointer to structure which contains information about the physical
     volumes in the input list */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */

struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */

int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */

caddr_t vgda_ptr,
  /* a pointer to the in-memory copy of the volume group descriptor
     area */

struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of all VGSA copies */

struct mwc_rec * mwcc)
  /* buffer which contains latest mirror write consistency cache */


{ /* BEGIN lvm_setupvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct mwc_rec kmwcc;
  /* buffer to contain list of logical track groups which need to be
     resynced in the kernel */

struct lvol * lvol_ptrs [LVM_MAXLVS];
  /* array of pointers to LVDD logical structures */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group header of the volume group descriptor
     area */

struct lvol * lvol_ptr;
  /* a pointer to a structure which describes a logical volume for the
     logical volume device driver (LVDD) */

struct ddi_info cfgdata;
  /* structure to contain the input parameters for the configuration
     device driver */

struct ca_mwc_dp temp;
  /* temporary variable to hold one MWCC disk part structure */

int retcode;
  /* return code */

short int lv_index;
  /* loop index for logical volume */

short int mir_index;
  /* loop index for mirror value */

short int pv_index;
  /* index for physical volume */

short int sa_index;
  /* index for volume group status area copy on a PV */

short int count;
  /* index variable for sort loop */

short int i;
  /* index variable for sort loop */

short int change;
  /* flag to indicate if any changes were made during the current
     iteration of the sort loop */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

bzero (lvol_ptrs, sizeof (lvol_ptrs));
  /* zero out the array of logical volume structure pointers */

retcode = lvm_bldklvlp (vgda_ptr, vgsa_ptr, lvol_ptrs);
  /* call routine to build the list of LVDD logical volume structures
     and the list of physical partition structures for all logical
     volumes in the volume group */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with return code */

}

bzero (&kmwcc, sizeof (struct mwc_rec));
  /* zero out buffer which is to hold mirror write consistency information
     to be sent to the kernel */

retcode = lvm_mwcinfo (varyonvg, inpvs_info, defpvs_info, maphdr_ptr,
	    vg_fd, vgda_ptr, vgsa_ptr, vgsa_lsn, lvol_ptrs, mwcc,
	    &kmwcc, &(cfgdata.parmlist.ksetupvg.num_ltgs));
  /* call routine to build mirror write consistency information for the
     kernel */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with return code */

}


for (pv_index = 0; pv_index < LVM_MAXPVS; pv_index = pv_index + 1)
  /* loop for all possible PVs and ensure that we do not send the LSNs
     of any VGSA copies on PVs which are not currently defined in the
     kernel */

{

  if (defpvs_info -> pv [pv_index].pv_status == LVM_NOTDFND)
    /* if this PV is not currently defined in the kernel */

  {

      for (sa_index = 0; sa_index < LVM_PVMAXVGDAS; sa_index = sa_index+1)
	/* loop for possible copies of the VGSA on this PV */

      {

	vgsa_lsn [pv_index] [sa_index] = 0;
	  /* set the logical sector number for this VGSA copy to 0, which
	     is the value to be set in the kernel for any PV which is not
	     currently active */

      }

  } /* PV not currently active */

}  /* loop for all ppossible PVs */



change = TRUE;
  /* initialize to false the flag which indicates if the order of any
     entries in the MWCC list was changed */

for (count = cfgdata.parmlist.ksetupvg.num_ltgs - 1;
     count > 0 && change == TRUE; count = count - 1)

{

  change = FALSE;
    /* set flag to indicate no change in the list */

  for (i = 0; i < count; i = i + 1)

  {

    if (kmwcc.ca_p1 [i].lv_minor > kmwcc.ca_p1 [i+1].lv_minor)
      /* if this MWCC entry has a LV minor number which is greater than
	 that of the entry following it */

    {

	temp = kmwcc.ca_p1 [i];
	kmwcc.ca_p1 [i] = kmwcc.ca_p1 [i+1];
	kmwcc.ca_p1 [i+1] = temp;
	  /* switch this MCCC entry with the one following it so that
	     they are in ascending order by minor number */

	change = TRUE;
	  /* set flag to indicate that the order was switched for at
	     least one pair of entries;  */

    }

  }

}


/************************************************************************
 *   Call the configuration device driver to add information about      *
 *   logical volumes to the LVDD kernel data structures.                *
 ************************************************************************
 */

cfgdata.parmlist.ksetupvg.vg_id = varyonvg -> vg_id;
  /* store the volume group id in the data structure which contains input
     parameters to be passed to the kernel */

cfgdata.parmlist.ksetupvg.lv_ptrs = lvol_ptrs;
  /* store pointer to the array of LVDD logical volume structure pointers
     in the data structure which contains input parameters to be passed
     to the kernel */

cfgdata.parmlist.ksetupvg.vgsa_ptr = vgsa_ptr;
  /* store pointer to the VGSA in the data structure which contains input
     parameters to be passed to the kernel */

cfgdata.parmlist.ksetupvg.salsns_ptr = (caddr_t) vgsa_lsn;
  /* store pointer to the array of LSNs of VGSA copies in data structure
     which contains parameters to be passed to the kernel */

cfgdata.parmlist.ksetupvg.quorum_cnt = vghdr_ptr -> total_vgdas / 2 + 1;
  /* save quorum count for number of VGDA/VGSA copies which we must have
     to remain varied on */

cfgdata.parmlist.ksetupvg.mwcc_ptr = &kmwcc;
  /* store pointer to kernel copy of mirror write consistency cache */

retcode = lvm_config (NULL, varyonvg -> vg_major, HD_KSETUPVG, &cfgdata);
  /* call routine which calls the configuration device driver to configure
     the LVDD structure information for logical volumes in this volume
     group */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}


/************************************************************************
 *   Free memory previously allocated for the LVDD logical volume       *
 *   structures and the lists of physical partition structures for      *
 *   each defined LV.  Start at minor number 1 since the volume group   *
 *   reserved area logical volume is never included here.               *
 ************************************************************************
 */

for (lv_index = 1; lv_index < LVM_MAXLVS; lv_index = lv_index + 1)
  /* loop for each logical volume */

{

  lvol_ptr = lvol_ptrs [lv_index];
    /* set a pointer to the logical volume structure for this LV minor
       number */

  if (lvol_ptr != NULL)
    /* if this logical volume is defined */

  {

      for (mir_index = 0; mir_index < lvol_ptr -> nparts;
	   mir_index = mir_index+1)
	/* loop for each mirror copy for this logical volume */

      {

	free ((caddr_t) lvol_ptr -> parts [mir_index]);
	  /* free the space allocated for the physical partition list for
	     this mirror copy */

      }

  } /* this LV defined */

  free ((caddr_t) lvol_ptr);
    /* free the space allocated for this logical volume structure */

} /* loop for each logical volume */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_setupvg */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_bldklvlp                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *     This routine builds the list of the LVDD logical volume         *
 *     structures and the list of logical partition structures for     *
 *     each logical volume.                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vgda_ptr                                                        *
 *     vgsa_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     lvol_ptrs                                                       *
 *                                                                     *
 *     The array of logical volume pointers contains a pointer for     *
 *     each defined logical volume.  For those logical volumes, the    *
 *     logical volume structure points to a list of logical partition  *
 *     structures.                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return code >= 0 indicates successful return.                   *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_ALLOCERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_bldklvlp (

caddr_t vgda_ptr,
  /* a pointer to the volume group descriptor area */

struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */

struct lvol * lvol_ptrs [LVM_MAXLVS]) /* array of pointers to the LVDD
logical volume structures */


{ /* BEGIN lvm_bldklvlp */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct lv_entries * lv_ptr;
  /* a pointer to a logical volume entry in the volume group descriptor
     area */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

struct pp_entries * pp_ptr;
  /* a pointer to a physical partition entry in the partition list of a
     physical volume entry in the volume group descriptor area */

struct lvol * lvol_ptr;
  /* a pointer to a structure which describes a logical volume for the
     logical volume device driver (LVDD) */

struct part * kpp_ptr;
  /* a pointer to a structure which describes one of the partitions of
     a logical partition for the LVDD */

short int lv_index;
  /* loop index for logical volumes */

long lp_index;
  /* loop index for logical partitions */

long pp_index;
  /* loop index for physical partitions */

short int pv_index;
  /* loop index used for physical volumes */

short int cpy_index;
  /* loop index used for physical partition copies */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long lplist_size;
  /* length in bytes of the list of logical partitions in the LVDD list
     of partitions */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

lv_ptr = (struct lv_entries *) (vgda_ptr + DBSIZE);
  /* set a pointer to the beginning of the list of logical volumes in the
     volume group descriptor area */

for (lv_index = 1; lv_index < vghdr_ptr->maxlvs; lv_index = lv_index+1)
  /* loop for maximum number of logical volumes */

{

  if (lv_ptr -> lv_state & LVM_LVDEFINED)
    /* if this logical volume is defined */

  {

      lvol_ptr = (struct lvol *) malloc (sizeof (struct lvol));
	/* allocate space for the logical volume structure for this
	   logical volume minor number */

      if (lvol_ptr == NULL)
	/* if an error occurred */

      {

	  return (LVM_ALLOCERR);
	    /* return with error code for memory allocation error */

      }

      lvol_ptrs [lv_index] = lvol_ptr;
	/* store the pointer to the logical volume structure into the
	   array of pointers to be returned */

      bzero (lvol_ptr, sizeof (struct lvol));
	/* zero out the entire logical volume structure */

      lvol_ptr -> lv_status = LV_CLOSED;
        /* set the status of the logical volume to closed */

      if (lv_ptr -> permissions == LVM_RDONLY)
	/* if this logical volume is read only */

      {

	  lvol_ptr -> lv_options = lvol_ptr -> lv_options | LV_RDONLY;
	    /* turn on the read only bit in the logical volume options
	       field */

      }

      if (lv_ptr -> bb_relocation == LVM_NORELOC)
	/* if bad blocks are not to be relocated for this logical
	   volume */

      {

	  lvol_ptr -> lv_options = lvol_ptr -> lv_options | LV_NOBBREL;
	    /* turn on the no bad block relocation bit in the logical
	       volume options field */

      }

      if (lv_ptr -> write_verify == LVM_VERIFY)
	/* if write verification is to be performed for this logical
	   volume */

      {

	  lvol_ptr -> lv_options = lvol_ptr -> lv_options | LV_WRITEV;
	    /* turn on the write verify bit in the logical volume options
	       field */

      }

      if (lv_ptr -> mirwrt_consist == LVM_NOCONSIST)
	/* if mirror write consistency is not to be performed for this
	   logical volume */

      {

	  lvol_ptr -> lv_options = lvol_ptr -> lv_options | LV_NOMWC;
	    /* turn on the bit for no mirror write consistency in the
	       logical volume options field */

      }

      if (lv_ptr -> mirror == LVM_NOMIRROR)
	/* if this logical volume has only one physical copy for all
	   its logical partitions */

      {
	  if (lv_ptr->mirror_policy == SCH_STRIPED) 
	      lvol_ptr -> i_sched = SCH_STRIPED; 
	  else
              lvol_ptr -> i_sched = SCH_REGULAR;
 	      /* set the scheduler policy in the LVDD logical volume
	         structure to regular since there is only one physical
	         copy */

      }

      else

      {

	  lvol_ptr -> i_sched = lv_ptr -> mirror_policy;
	    /* set the scheduler policy in the LVDD logical volume
	       structure to the mirror policy field in the LV entry
	       of the volume group descriptor area */

      }

      lvol_ptr -> waitlist = EVENT_NULL;
        /* set the event list for quiesce in the LVDD logical volume
           structure to indicate null event */

      lvol_ptr -> nparts = lv_ptr -> mirror;
	/* set the number of partitions (base 1) in the LVDD logical volume
           structure to the mirror field in the LV entry of the volume
	   group descriptor area */

      lvol_ptr -> nblocks = (1 << (vghdr_ptr -> pp_size - DBSHIFT)) *
				 lv_ptr -> num_lps;
        /* calculate the length of the logical volume in blocks by
           multiplying the number of blocks in a partition times the
           number of logical partitions */

      lvol_ptr -> stripe_exp = (unsigned int)lv_ptr -> stripe_exp;
      lvol_ptr -> striping_width = (unsigned int)lv_ptr -> striping_width;

      lplist_size = lv_ptr -> num_lps * sizeof (struct part);
        /* calculate the amount of memory needed for the partition list
           by multiplying the number of logical partitions times the size
           of the LVDD partition structure */


      /******************************************************************
       *   Allocate space for the partition list for each needed        *
       *   copy, and save pointers in the parts array of the LVDD       *
       *   logical volume structure.                                    *
       ******************************************************************
       */

      for (cpy_index=0; cpy_index<lv_ptr->mirror; cpy_index=cpy_index+1)
	/* loop for each physical copy */

      {

	lvol_ptr -> parts [cpy_index] = (struct part *)
                                           malloc (lplist_size);
          /* allocate space and store pointer for the physical partition
	     list for this physical copy */

	if (lvol_ptr -> parts [cpy_index] == NULL)
          /* if an error occurred while trying to allocate memory */

	{

            return (LVM_ALLOCERR);
              /* return error for memory allocation error */

	}

	bzero (lvol_ptr -> parts [cpy_index], lplist_size);
	  /* zero out the partition list for this physical copy */

      } /* loop for each physical copy */

  } /* this logical volume defined */

  lv_ptr = (struct lv_entries *) ((caddr_t) lv_ptr +
                                     sizeof (struct lv_entries));
    /* increment pointer to point at the next entry in the list of
       logical volumes in the volume group descriptor area */

} /* loop for each possible logical volume */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate how much space is needed for the descriptor area list of
     logical volumes for the maximum number of logical volumes allowed
     for this volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to the
     beginning of the list of physical volumes in the volume group
     descriptor area */

for (pv_index = 1; pv_index <= vghdr_ptr->numpvs; pv_index = pv_index + 1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  pp_ptr = (struct pp_entries *) ((caddr_t) pv_ptr +
                                     sizeof (struct pv_header));
    /* set a pointer to the beginning of the list of physical partitions
       for this physical volume in the volume group descriptor area */

  for (pp_index=1; pp_index <= pv_ptr->pp_count; pp_index = pp_index+1)
    /* loop for each partition in the list of physical partitions for
       this physical volume */

  {

    if (pp_ptr -> pp_state & LVM_PPALLOC)
      /* if this physical partition has been allocated to a logical
         volume */

    {

	lvol_ptr = lvol_ptrs [pp_ptr -> lv_index];
	  /* get the pointer to the desired logical volume structure,
	     using the LV minor number as an index into the array of
	     LV structure pointers */

	switch (pp_ptr -> copy)
	  /* look at the field in the PP entry which identifies which
	     physical partition copy of the logical partition this is */

	{

	  case LVM_PRIMARY:
	  case LVM_PRIMOF2:
	  case LVM_PRIMOF3:
	    /* is this physical partition is the primary copy of the
	       logical partition */

	    cpy_index = LVM_FIRST - 1;
	      /* set array index variable to indicate for which copy of
		 the logical partition information is to be stored */

	    break;
	      /* exit from switch statement */

	  case LVM_SCNDOF2:
	  case LVM_SCNDOF3:
	    /* is this physical partition is the secondary copy of the
	       logical partition */

	    cpy_index = LVM_SEC - 1;
	      /* set array index variable to indicate for which copy of
		 the logical partition information is to be stored */

	    break;
	      /* exit from switch statement */

	  case LVM_TERTOF3:
	    /* is this physical partition is the tertiary copy of the
	       logical partition */

	    cpy_index = LVM_THIRD - 1;
	      /* set array index variable to indicate for which copy of
		 the logical partition information is to be stored */

	    break;
	      /* exit from switch statement */

	  default:
	     /* other values */

	    return (LVM_PROGERR);
	      /* any other value in the copy field indicates a
		 programming error */

	}

	kpp_ptr = (struct part *) ((caddr_t) lvol_ptr -> parts [cpy_index]
		     + (pp_ptr -> lp_num - 1) * sizeof (struct part));
	   /* using the logical partition number as an offset and the
	      copy index as an indication of which physical copy,
	      calculate the pointer to the partition structure for the
	      indicated physical copy */

	kpp_ptr -> pvol = (struct pvol *) pv_ptr -> pv_num;
	  /* in the LVDD partition structure, set the pointer to the LVDD
	     physical volume structure to the PV number from the physical
	     volume header (note that the PV number is stored here in the
	     application layer and that in the kernel it is used to get
	     the actual address from the array of physical volume
	     structure pointers in the volume group structure) */

	kpp_ptr -> start = (daddr_t) ((daddr_t) pv_ptr -> psn_part1 +
	       (pp_index - 1) * (1 << vghdr_ptr -> pp_size - DBSHIFT));
	  /* calculate the physical block number for this physical
	     partition and store as the starting disk address in the
	     LVDD partition structure */

	kpp_ptr -> sync_trk = NO_SYNCTRK;
	  /* set the sync_trk to -1 to indicate there is no resync in
	     progress */

	if (TSTSA_STLPP (vgsa_ptr, pv_ptr -> pv_num - 1, pp_index - 1))
	  /* if information in VGSA shows that this PP is stale */

	{

	    kpp_ptr -> ppstate = PP_STALE;
	      /* set stale bit on for the physical partition state */

	}

    } /* this PP is allocated */

    pp_ptr = (struct pp_entries *) ((caddr_t) pp_ptr +
                                       sizeof (struct pp_entries));
      /* increment pointer to point at next physical partition entry in
         the volume group descriptor area */

  }  /* loop for each PP on this PV */

  size = sizeof (struct pv_header) + pv_ptr -> pp_count *
                                       sizeof (struct pp_entries);
    /* calculate the size of the space needed to contain the header and
       the physical partition entries for this PV */

  pventry_size = LVM_SIZTOBBND (size);
    /* calculate the actual size reserved in the descriptor area for this
       PV entry by rounding the size up to the nearest multiple of the
       blocksize of the physical volume */

  pv_ptr = (struct pv_header *) ((caddr_t) pv_ptr + pventry_size);
    /* set pointer to point at the next PV entry in the list of physical
       volumes in the volume group descriptor area */

} /* loop for each PV in the volume group descriptor area */

return (LVM_SUCCESS);
  /* return with return code */

} /* END lvm_bldklvlp */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_mwcinfo                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine uses the latest mirror write consistency cache to    *
 *   build the list of logical track groups which must be resynced     *
 *   in the kernel.  It also identifies those logical partitions for   *
 *   which it is necessary to do disaster recovery and updates the     *
 *   volume group status area to relect this information and the       *
 *   status of currently missing PVs.                                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     maphdr_ptr                                                      *
 *     vg_fd                                                           *
 *     vgda_ptr                                                        *
 *     vgsa_ptr                                                        *
 *     vgsa_lsn                                                        *
 *     lvol_ptrs                                                       *
 *     mwcc                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *kmwcc                                                          *
 *     *num_entries                                                    *
 *                                                                     *
 *     The VGSA has been updated, if necessary, with stale PP status   *
 *     for certain PPs and with PV status to reflect the active and    *
 *     missing PVs for the current varyon.                             *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return code >= 0 indicates successful return.                   *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_NOQUORUM                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_mwcinfo (

struct varyonvg * varyonvg,
  /* pointer to structure which contains the input information for
     the lvm_varyonvg subroutine */

struct inpvs_info * inpvs_info,
  /* pointer to structure which contains information about the physical
     volumes in the input list */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */

struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */

int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */

caddr_t vgda_ptr,
  /* a pointer to the volume group descriptor area */

struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of all VGSA copies */

struct lvol * lvol_ptrs [LVM_MAXLVS],
  /* array of pointers to LVDD logical structures */

struct mwc_rec * mwcc,
  /* buffer which contains latest mirror write consistency cache */

struct mwc_rec * kmwcc,
  /* buffer to contain list of logical track groups from the MWCC which
     need to be resynced in the kernel */

short int * num_entries)
  /* number of logical track group entries in the kernel MWCC buffer */


{ /* BEGIN lvm_mwcinfo */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct {
       ulong pv_missing [(MAXPVS + (NBPL - 1)) / NBPL];
       } drcvrypvs;
  /* structure to contain a bit map of PVs which are to be used to
     determine the logical partitions for which we must do disaster
     recovery (NOTE that the pv_missing array must be defined the same
     as that in the vgsa_area structure in order to use VGSA macros */

struct {
       ulong pv_missing [(MAXPVS + (NBPL - 1)) / NBPL];
       } curmisspvs;
  /* structure to contain a bit map of the PVs which are currently missing
     at this varyon (NOTE that the pv_missing array must be defined the
     same as that in the vgsa_area structure in order to use the VGSA
     macros */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

struct lvol * lvol_ptr;
  /* pointer to an LVDD logical volume structure */

struct part * kpp_ptr;
  /* pointer to an LVDD physical partition structure */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

long num_lps;
  /* the number of logical partitions in a logical volume */

int curcnt_vgdas;
  /* the count of currently good VGDA/VGSA copies */

int retcode;
  /* return code */

short int disaster_rec;
  /* flag to indicate if disaster recovery should be done for a logical
     partition */

short int vgsaupd;
  /* flag to indicate if the volume group status area has been updated */

short int freshpps;
  /* the number of physical partition copies which are fresh for an LP */

short int num_copies;
  /* the number of physical copies for a logical volume */

short int k_partshift;
  /* the size of a partition in number of blocks, expressed as a power
     of 2 */

short int pv_num;
  /* the number of the physical volume */

short int pv_idx;
  /* physical volume index */

short int pp_idx;
  /* physical partition index */

short int lv_idx;
  /* logical volume index */

short int lp_idx;
  /* logical partition index */

short int cpy_idx;
  /* mirror copy index */

short int sa_idx;
  /* status area copy index */

short int idx;
  /* general index value */

short int fresh_copy;
  /* copy index of a physical partition copy which is fresh */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/*
 *   Find and build the list of PVs which will be used to
 *   determine the list of logical partitions for which we need
 *   to do disaster recovery.  The term "disaster recovery" is
 *   used to describe the situation where we need to pick one
 *   physical copy of a LP to remain fresh while all remaining
 *   physical copies are made stale.
 *
 *   Disaster recovery needs to be done for a logical partition
 *   if we do not have valid MWCC information for that LP.
 *   Mirror write consistency cache information for a logical
 *   partition is written (when possible) to all PVs which have
 *   a physical copy of that logical partition.
 *
 *
 *
 */


for (idx = 0; idx < (MAXPVS + (NBPL - 1)) / NBPL; idx = idx + 1)
  /* loop for each element in array and initialize bit map of disaster
     recovery PVs and currently missing PVs */

{

  drcvrypvs.pv_missing [idx] = 0;
    /* initialize bit map for PVs which are to be used for disaster
       recovery */

  curmisspvs.pv_missing [idx] = 0;
    /* initialize bit map of PVs which are missing at this varyon */

}


/************************************************************************
 *   Find list of PVs which will be used to determine the logical       *
 *   partitions for which we need to do disaster recovery.  Also, find  *
 *   list of PVs which are currently missing for this varyon.           *
 ************************************************************************
 */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

size = vghdr_ptr -> maxlvs * sizeof (struct lv_entries);
  /* calculate the size needed for the descriptor area list of logical
     volumes for the maximum number of logical volumes allowed for this
     volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to the
     beginning of the list of physical volumes in the volume group
     descriptor area */

for (pv_idx = 0; pv_idx < vghdr_ptr -> numpvs; pv_idx = pv_idx + 1)
  /* loop for each physical volume in the volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get the PV number of the physical volume */

  if (defpvs_info -> pv [pv_num-1].pv_status == LVM_DEFINED)
    /* if this PV is defined into the kernel */

  {

      if (TSTSA_PVMISS (vgsa_ptr, pv_num-1)  ||
	   defpvs_info -> pv[pv_num-1].mwc.good_mwcc == FALSE)
	/* if this PV was in a state of missing at the previous varyon
	   OR if this PV's MWCC is currently missing or not good */

      {

	  SETSA_PVMISS (&drcvrypvs, pv_num - 1);
	    /* turn on bit for this PV in structure to contain bit map of
	       PVs which are used to determine the logical partitions for
	       which we must do disaster recovery */

      }

  } /* PV defined in kernel */

  else
      /* PV not defined in kernel (i.e., PV is currently missing) */

  {

      SETSA_PVMISS (&drcvrypvs, pv_num - 1);
	/* turn on bit for this PV in structure to contain bit map of PVs
	   which are used to determine the logical partitions for which
	   we must do disaster recovery */

      SETSA_PVMISS (&curmisspvs, pv_num - 1);
	/* turn on bit for this PV in structure to contain bit map of
	   PVs currently missing at this varyon */

  }

  size = sizeof (struct pv_header) + pv_ptr -> pp_count *
				       sizeof (struct pp_entries);
    /* calculate the size of the space needed to contain the header and
       the physical partition entries for this PV */

  pventry_size = LVM_SIZTOBBND (size);
    /* calculate the actual size reserved in the descriptor area for this
       PV entry by rounding the size up to the nearest multiple of the
       blocksize of the physical volume */

  pv_ptr = (struct pv_header *) ((caddr_t) pv_ptr + pventry_size);
    /* set pointer to point at the next PV entry in the list of physical
       volumes in the volume group descriptor area */

} /* loop for all PVs in the VGDA */


/************************************************************************
 *   Check each logical partition in the volume group to determine if   *
 *   disaster recovery is required for it.  If so, mark all but one     *
 *   physical copy stale in the in-memory copy of the volume group      *
 *   status area.                                                       *
 ************************************************************************
 */

vgsaupd = FALSE;
  /* initialize flag to indicate the vgsa has not been updated */

k_partshift = vghdr_ptr -> pp_size - DBSHIFT;
  /* find partition shift value where the unit is a block */

for (lv_idx = 1; lv_idx < vghdr_ptr -> maxlvs; lv_idx = lv_idx + 1)
  /* loop for all possible logical volumes in the volume group */

{

  lvol_ptr = lvol_ptrs [lv_idx];
    /* save pointer to this logical volume */

  if (lvol_ptr != NULL)
    /* if this logical volume is defined */

  {

      num_copies = lvol_ptr -> nparts;
	/* save maximum number of physical copies for a logical partition
	   in this logical volume */

      num_lps = BLK2PART (k_partshift, lvol_ptr -> nblocks);
	/* find number of logical partitions in this logical volume */

      for (lp_idx = 0; lp_idx < num_lps; lp_idx = lp_idx + 1)
	/* loop for all logical partitions in the logical volume */

      {

	disaster_rec = TRUE;
	  /* initialize to TRUE flag which indicates whether disaster
	     recovery needs to be done for the LP */

	for (cpy_idx = 0; cpy_idx < num_copies; cpy_idx = cpy_idx + 1)
	  /* loop for all copies of the logical partition, to determine
	     if disaster recovery must be done for this LP */

	{

	  kpp_ptr = (struct part *) ((caddr_t) lvol_ptr->parts[cpy_idx]
		      + lp_idx * sizeof (struct part));
	    /* get pointer to this physical copy of the LP */

	  if (kpp_ptr -> pvol != NULL)
	    /* if this physical partition copy exists */

	  {

	      pv_idx = ((short int) kpp_ptr -> pvol) - 1;
		/* get index (base 0) of PV containing this PP; note that
		   the PV number is currently stored in the pointer to
		   the pvol structure */

	      if (!(TSTSA_PVMISS (&drcvrypvs, pv_idx)))
		/* if a PV containing one of the PP copies is not in the
		   list of PVs used for determining disaster recovery */

	      {

		  disaster_rec = FALSE;
		    /* set flag to indicate that disaster recovery does
		       not need to be done for this LP */

		  break;
		    /* break from loop;  disaster recovery not needed */

	      } /* PV not in disaster recovery list */

	  } /* this PP copy exists */

	} /* loop for all copies of the LP */

	if (disaster_rec == TRUE)
	  /* if we must do disaster recovery for this LP */

	{

	    for (cpy_idx = 0; cpy_idx < num_copies; cpy_idx = cpy_idx+1)
	      /* loop for all physical partition copies in order to find
		 a non-stale copy (if possible, on an active PV) */

	    {

	      kpp_ptr = (struct part *) ((caddr_t) lvol_ptr->parts[cpy_idx]
			  + lp_idx * sizeof (struct part));
		/* get pointer to this physical copy of the LP */

	      if (kpp_ptr -> pvol != NULL  &&
		  !(kpp_ptr -> ppstate & PP_STALE))
		/* if this physical partition copy exists and is not
		   stale */
	      {

		  pv_idx = ((short int) kpp_ptr -> pvol) - 1;
		    /* get index (base 0) of PV containing this PP;
		       note that the PV number is currently stored in
		       the pointer to the pvol structure */

		  fresh_copy = cpy_idx;
		    /* save copy index of PP which is fresh */

		  if (defpvs_info -> pv[pv_idx].pv_status == LVM_DEFINED)
		    /* if this PV is currently active (i.e., defined in
		       the kernel) */

		  {

		      break;
			/* break from loop since we have found a fresh PP
			   on an active PV;  otherwise, keep trying to
			   find a fresh PP on a currently active PV */

		  }

	      } /* this PP copy exists and is not stale */

	    } /* loop for possible PP copies */

	    for (cpy_idx = 0; cpy_idx < num_copies; cpy_idx = cpy_idx + 1)
	      /* loop for all physical partition copies, ensuring that
		 all are stale except one copy which is to remain fresh */

	    {

	      kpp_ptr = (struct part *) ((caddr_t) lvol_ptr->parts[cpy_idx]
			  + lp_idx * sizeof (struct part));
		/* get pointer to this physical copy of the LP */

	      if (kpp_ptr -> pvol != NULL  &&  !(kpp_ptr -> ppstate &
		     PP_STALE)  &&  fresh_copy != cpy_idx)
		/* if this physical partition copy exists and is not stale
		   and if it is not the copy to remain fresh */

	      {

		  pv_idx = ((short int) kpp_ptr -> pvol) - 1;
		    /* get index (base 0) of PV containing this PP;
		       note that the PV number is currently stored in
		       the pointer to the pvol structure */

		  pv_ptr = (struct pv_header *) (vgda_ptr + maphdr_ptr ->
			 pvinfo[pv_idx].pvinx - sizeof (struct fheader));
		    /* calculate pointer to this PV header using info in
		       mapped file header */

		  pp_idx = BLK2PART (k_partshift,
			     (kpp_ptr -> start - pv_ptr -> psn_part1));
		    /* find physical partition index (base 0) */

		  SETSA_STLPP (vgsa_ptr, pv_idx, pp_idx);
		    /* set stale bit on in in-memory VGSA copy for this
		       physical partition copy */

		  kpp_ptr -> ppstate = PP_STALE;
		    /* set stale bit on for physical partition state in
		       the LVDD part structure */

		  vgsaupd = TRUE;
		   /* set flag to indicate the VGSA has been updated */

	      } /* if this PP copy exists */

	    } /* loop for remaining PP copies */

	} /* do disaster recovery for this LP */

      } /* loop for all LPs in the LV */

  } /* this LV is defined */

} /* loop for all possible LVs */


/************************************************************************
 *   Update the missing PV status in the volume group status area to    *
 *   reflect the current PV status at this varyon.                      *
 ************************************************************************
 */

for (idx = 0; idx < (MAXPVS + (NBPL-1)) / NBPL;  idx = idx + 1)
  /* loop for each entry in PV missing array */

{

  if (vgsa_ptr -> pv_missing [idx] != curmisspvs.pv_missing [idx])
    /* if information in this entry in the VGSA does not match information
       for this entry in array of current missing PV information */

  {

      vgsa_ptr -> pv_missing [idx] = curmisspvs.pv_missing [idx];
	/* update the volume group status area with information about the
	   currently missing PVs for this varyon */

      vgsaupd = TRUE;
	/* set flag to indicate the VGSA has been udpated */

  }

}


/************************************************************************
 *   If the volume group status area has been updated, then update its  *
 *   timestamp, and write the updated VGSA to the on-disk copies.       *
 ************************************************************************
 */

if (vgsaupd == TRUE)
  /* if the volume group status area has been updated */

{

    retcode = lvm_updtime (&(vgsa_ptr -> b_tmstamp),
			   &(vgsa_ptr -> e_tmstamp));
      /* call routine to update the VGSA timestamp */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	return (retcode);
	  /* return with returned error code */

    }

    curcnt_vgdas = 0;
      /* initialize the current count of good VGDA/VGSA copies to 0 */

    for (pv_idx = 0; pv_idx < LVM_MAXPVS; pv_idx = pv_idx + 1)
      /* loop for all possible PVs */

    {

      if (defpvs_info -> pv[pv_idx].pv_status == LVM_DEFINED)
	/* if this PV is defined into the kernel */

      {

	  pv_ptr = (struct pv_header *) (vgda_ptr + maphdr_ptr ->
		 pvinfo[pv_idx].pvinx - sizeof (struct fheader));
	    /* calculate pointer to this PV header using info in
	       mapped file header */

	  for (sa_idx=0; sa_idx < pv_ptr->pvnum_vgdas; sa_idx=sa_idx+1)
	    /* loop for VGSA copies on this PV */

	  {

	    retcode = lvm_wrtdasa (vg_fd, (caddr_t) vgsa_ptr,
		   (struct timestruc_t *) &(vgsa_ptr -> e_tmstamp),
		   inpvs_info -> vgsa_len, vgsa_lsn [pv_idx] [sa_idx],
		   (short int) LVM_BEGMIDEND);
	      /* write the updated VGSA to this VGSA copy */

	    if (retcode < LVM_SUCCESS)
	      /* if VGSA write was not successful */

	    {

		retcode = lvm_deladdm (varyonvg, inpvs_info, defpvs_info,
				       (short int) (pv_idx + 1));
		  /* call routine to change this PV to a missing PV in
		     the kernel */

		if (retcode < LVM_SUCCESS)
		  /* if an error occurred */

		{

		    return (retcode);
		      /* return with error code */

		}

		break;
		  /* break out of loop since this PV is no longer
		     defined in the kernel */

	    }

	  } /* loop for VGSA copies on this PV */

	  if ( defpvs_info -> pv [pv_idx].pv_status == LVM_DEFINED)
	    /* if this PV is still active (i.e., it was not declared
	       missing because of inability to write to a VGSA copy) */

	  {

	      curcnt_vgdas = curcnt_vgdas + pv_ptr -> pvnum_vgdas;
		/* increment count of good VGDA/VGSA copies by number
		   contained on this PV */

	  }

      } /* this PV is defined in kernel */

    } /* loop for all possible PVs */

    if (curcnt_vgdas < maphdr_ptr -> quorum_cnt)
      /* if the count of currently good VGDA/VGSA copies is less than the
	 required quorum */

    {

	return (LVM_NOQUORUM);
	  /* return with error code for no quorum */

    }

} /* the VGSA has been updated */


/************************************************************************
 *   Build mirror write consistency cache information to send to kernel *
 *   config routine which will contain information only for those       *
 *   logical track groups which must be resynced in the kernel.         *
 ************************************************************************
 */

*num_entries = 0;
  /* initialize number of kernel MWCC entries to 0 */

for (idx = 0;  idx < MAX_CA_ENT; idx = idx + 1)
  /* loop for maximum possible cache entries to determine which entries
     should be sent to the kernel */

{

  if (mwcc -> ca_p1 [idx].lv_minor != 0)
    /* if there exists a mirror write cache entry in this slot */

  {

      lp_idx = BLK2PART (k_partshift, TRK2BLK (mwcc->ca_p1[idx].lv_ltg));
	/* find the logical partition index (base 0) of the logical
	   partition which contains the specified logical track group */

      lvol_ptr = lvol_ptrs [mwcc -> ca_p1 [idx].lv_minor];
	/* get pointer to logical volume structure */

      num_copies = lvol_ptr -> nparts;
	/* save maximum number of physical copies for a logical partition
	   in this logical volume */

      num_lps = BLK2PART (k_partshift, lvol_ptr -> nblocks);
	/* find number of logical partitions in this logical volume */


      /******************************************************************
       *   NOTE that the following check for the existence of the       *
       *   logical volume and logical partition is made because of the  *
       *   force quorum option on varyon.  Allowing the user to force   *
       *   a quorum means that the VGDA copy being used is not          *
       *   guaranteed to have the latest information about the volume   *
       *   group.  If the chosen VGDA copy is not the latest, then its  *
       *   information about the existence of LVs and LPs might not     *
       *   coincide with LV/LP information obtained from the mirror     *
       *   write consistency cache.                                     *
       ******************************************************************
       */

      if (lvol_ptr != NULL  &&  lp_idx < num_lps)
	/* if the logical volume and logical partition exist */

      {

	  freshpps = 0;
	    /* initialize number of fresh physical partitions to 0 */

	  for (cpy_idx = 0; cpy_idx < num_copies; cpy_idx = cpy_idx+1)
	    /* loop for all physical partition copies, counting the
	       number of non-stale copies */

	  {

	    kpp_ptr = (struct part *) ((caddr_t) lvol_ptr->parts[cpy_idx]
			+ lp_idx * sizeof (struct part));
	      /* get pointer to this physical copy of the LP */

	    if (kpp_ptr -> pvol != NULL  &&
		!(kpp_ptr -> ppstate & PP_STALE))
	      /* if this physical partition copy exists and is not
		 stale */

	    {

		freshpps = freshpps + 1;
		  /* increment the number of fresh physical partitions
		     for this logical partition */

	    }

	  } /* loop for all PP copies */


	  /**************************************************************
	   *   If a logical partition has only one fresh PP copy, then  *
	   *   there is no need to resync any logical track groups in   *
	   *   that partition.  Therefore, when collecting MWCC         *
	   *   information for the kernel, throw out any LTGs which     *
	   *   reside in a LP with only one fresh PP copy.              *
	   **************************************************************
	   */

	  if (freshpps > 1)
	    /* if there exists more than one fresh physical partition */

	  {
	      kmwcc -> ca_p1 [*num_entries] = mwcc -> ca_p1 [idx];
		/* transfer information about this logical track group
		   from the chosen MWCC to MWCC buffer to be sent to the
		   kernel */

	      *num_entries = *num_entries + 1;
		/* increment number of entries in kernel MWCC */

	  }

      } /* the LV and LP exist */

  } /* this cache entry exists */

} /* loop for all cache entries */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_mwcinfo */

