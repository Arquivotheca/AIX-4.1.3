static char sccsid[] = "@(#)37	1.9  src/bos/usr/ccs/lib/liblvm/revaryon.c, liblvm, bos411, 9428A410j 2/22/93 16:21:10";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - revaryon.c
 *
 * FUNCTIONS: lvm_revaryon,
 *            lvm_vonmisspv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/hd_config.h>
#include <sys/vgsa.h>
#include <sys/dasd.h>
#include <liblvm.h>









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_revaryon                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   For a revaryon of an already varied on volume group, this routine *
 *   attempts to bring currently missing PVs back on-line.             *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     vgmap_fd                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     varyonvg -> vvg_out                                             *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_DALVOPN                                                     *
 *     LVM_ALLOCERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_revaryon (

struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */

int *vgmap_fd,
  /* the file descriptor for the mapped file */

struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */

struct defpvs_info * defpvs_info)
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */


{ /* BEGIN lvm_revaryon */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vgsa_area vgsa;
  /* buffer into which the VGSA will be read */

int retcode;
  /* return code */

caddr_t vgmap_ptr;
  /* pointer to beginning of mapped file */

long mapf_len;
  /* the length in bytes of the mapped file */

struct fheader maphdr;
  /* structure into which to read the mapped file header */

int vg_fd;
  /* the file descriptor for the volume group reserved area logical
     volume */

int chkvvgout;
  /* flag to indicate if the varyonvg output structure should be
     checked */

caddr_t vgda_ptr;
  /* pointer to the beginning of the volume group descriptor area */

struct fheader * maphdr_ptr;
  /* pointer to the mapped file header */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

long partlen_blks;
  /* the length of a partition in number of 512 byte blocks */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

short int pv_index;
  /* loop index variable for physical volume */

short int in_index;
  /* index into the input list of a physical volume */

short int i;
  /* index for looping */

short int pv_num;
  /* physical volume number */

short int out_index;
  /* index into the output list of PVs */

char vgdev [LVM_EXTNAME];
  /* the full path name of the volume group reserved area logical
     volume */

char vgmap_fn [sizeof (LVM_ETCVG) + 2 * sizeof (struct unique_id)];
  /* the path name of the mapped file */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


chkvvgout = FALSE;
  /* initialize flag to false to indicate that there is no information
     in the varyonvg output structure which needs to be checked */

bzero (inpvs_info, sizeof (struct inpvs_info));
  /* zero out the structure which is to contain information about the
     PVs in the user's input list */

bzero (defpvs_info, sizeof (struct defpvs_info));
  /* zero out the structure which is to contain information about the
     volume group descriptor area for PVs defined into the kernel */


/************************************************************************
 *   Get VGDA information                                               * 
 ************************************************************************
 */

retcode = lvm_getvgdef(&(varyonvg->vg_id), 0, &vgmap_fd, &vgmap_ptr);
if (retcode < LVM_SUCCESS)
	return(retcode);
		
maphdr_ptr = (struct fheader *) vgmap_ptr;
  /* set a pointer to the mapped file header */


/************************************************************************
 *   Open the volume group reserved area logical volume.                *
 ************************************************************************
 */

retcode = status_chk (NULL, varyonvg -> vgname, NOCHECK, vgdev);
    /* build the path name for the volume group reserved area logical
       volume from the volume group name passed in */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    return (retcode);
      /* return with error code */

}

vg_fd = open (vgdev, O_RDWR | O_NSHARE);
  /* open the volume group reserved area logical volume */

if (vg_fd == LVM_UNSUCC)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    return (LVM_DALVOPN);
      /* return with error for volume group reserved area could not
	 be opened */

}


/************************************************************************
 *   Get information about all PVs in the input list.                   *
 ************************************************************************
 */

vgda_ptr = vgmap_ptr + sizeof (struct fheader);
  /* set a pointer to beginning of the volume group descriptor area */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

defpvs_info -> newest_dats = vghdr_ptr -> vg_timestamp;
  /* store timestamp from the current copy of the volume group descriptor
     area as the timestamp of the newest descriptor area (used in
     lvm_getdainfo) */

inpvs_info -> pp_size = vghdr_ptr -> pp_size;
  /* store physical partition size from the volume group header in the
     input PVs information array */

retcode = lvm_readpvs (varyonvg, inpvs_info);
  /* call routine to get information about each of the PVs in the input
     list */


retcode = ioctl (vg_fd, GETVGSA, &vgsa);
  /* get the current VGSA from the kernel */

if (retcode != LVM_SUCCESS)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate the size needed for the descriptor area list of logical
     volumes for the maximum number of logical volumes allowed for this
     volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

for (in_index=0; in_index<varyonvg->vvg_in.num_pvs; in_index=in_index+1)
  /* loop for each PV in the input list */

{

  for (i = in_index; i >= 1; i = i - 1)
    /* loop for each PV previous to the current PV in the input list in
       order to check for duplicate PVs */

  {

    if (varyonvg -> vvg_out.pv[in_index].pv_id.word1 ==
	varyonvg -> vvg_out.pv[i-1].pv_id.word1 &&
	varyonvg -> vvg_out.pv[in_index].pv_id.word2 ==
	varyonvg -> vvg_out.pv[i-1].pv_id.word2)
      /* if the current PV has a PV id which matches the PV id of a PV
	 found earlier in the list */

    {

	varyonvg -> vvg_out.pv[in_index].pv_status = LVM_DUPPVID;
	  /* set the status for the current PV to duplicate PV id */

	break;
	  /* break from loop */

    }

  }

  if (varyonvg -> vvg_out.pv[in_index].pv_status == LVM_DUPPVID)
      /* if this PV is a duplicate of another PV in the list */

  {

      chkvvgout = TRUE;
	/* set flag to indicate that the varyonvg output structure has
	   important status information */

  }

  else

  {

      pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
	/* set the pointer to the first physical volume entry to point to
	   the beginning of the list of physical volumes in the volume
	   group descriptor area */

      for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
	/* loop for each physical volume in the list of PVs in the volume
	   group descriptor area, searching for a matching PV id */

      {

	pv_num = pv_ptr -> pv_num;
	  /* save PV number for this PV */

	if (pv_ptr -> pv_id.word1 ==
	    varyonvg -> vvg_out.pv[in_index].pv_id.word1  &&
	    pv_ptr -> pv_id.word2 ==
	    varyonvg -> vvg_out.pv[in_index].pv_id.word2)
	  /* if we have found a matching PV id in the VGDA for this PV
	      from the input list */

	{

	    if (!(TSTSA_PVMISS (&vgsa, pv_num - 1)))
	      /* if this PV is not currently missing */

	    {

		varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVACTIVE;
		  /* set status for this PV in the varyonvg output
		     structure to indicate that this PV is currently
		     active */

	    }

	      else
		/* this PV is not defined into the kernel */

	    {

		if ((pv_ptr -> pv_state & LVM_PVREMOVED)  &&  varyonvg ->
		       vvg_out.pv[in_index].pv_status != LVM_PVNOTINVG)
		  /* if the PV state for this PV indicates it has been
		     temporarily removed from volume group, meaning that
		     it is not to be varied-on, and if it did not have a
		     bad LVM record which indicated it was not a member of
		     the volume group */

		{

		    varyonvg -> vvg_out.pv[in_index].pv_status =
						       LVM_PVREMOVED;
		      /* set status for this PV in the varyonvg output
			 structure to indicate to the user this is a
			 removed PV */

		    chkvvgout = TRUE;
		      /* set flag to true to indicate varyonvg output
			 structure should be checked for additional
			 information */

		}

		if (varyonvg -> vvg_out.pv[in_index].pv_status ==
						       LVM_PVNOTINVG)
		  /* if the LVM record for this PV indicates it is not a
		     member of the volume group */

		{

		    varyonvg -> vvg_out.pv[in_index].pv_status =
						       LVM_LVMRECNMTCH;
		      /* change PV status in varyonvg output structure to
			 indicate the LVM record information about VG
			 membership does not match information in VGDA;
			 this is an indication to user that he needs to
			 delete this PV from the volume group since the
			 mismatch of information probably means user has
			 installed this PV into another volume group */

		    chkvvgout = TRUE;
		      /* set flag to true to indicate varyonvg output
			 structure should be checked for additional
			 information */

		}

		if (varyonvg -> vvg_out.pv[in_index].pv_status ==
						       LVM_PVNOTFND)
		  /* if this PV could not be opened or its IPL record
		     could not be read */

		{

		    varyonvg -> vvg_out.pv[in_index].pv_status =
						       LVM_PVMISSING;
		      /* change PV status in varyonvg output structure to
			 indicate that the PV is still missing */

		    chkvvgout = TRUE;
		      /* set flag to true to indicate varyonvg output
			 structure should be checked for additional
			 information */

		}

		if (varyonvg -> vvg_out.pv[in_index].pv_status ==
						       LVM_PVINVG)
		  /* if the LVM record for this PV indicates it is a
		     member of the volume group */

		{

		    retcode = lvm_vonmisspv (varyonvg, inpvs_info,
			   defpvs_info, maphdr_ptr, vgda_ptr, pv_ptr,
			   vg_fd, in_index, &chkvvgout);
		      /* call routine to add this previously missing PV
			 into kernel and update any VGDA copies on it */

		    if (retcode == LVM_FORCEOFF)
		      /* if an error is returned for forced varyoff */

		    {

			return (retcode);
			  /* return with error for forced varyoff */

		    }

		}

	    } /* PV is currently missing (not defined in kernel) */

	    break;
	      /* break from loop since we have found the matching PV we
		 were searching for */

	} /* PV from input list found in VGDA */

	size = sizeof (struct pv_header) + pv_ptr -> pp_count *
					     sizeof (struct pp_entries);
	  /* calculate the size of the space needed to contain the header
	     and the physical partition entries for this PV */

	pventry_size = LVM_SIZTOBBND (size);
	  /* calculate the actual size reserved in the descriptor area for
	     this PV entry by rounding the size up to the nearest multiple
	     of the blocksize of the physical volume */

	pv_ptr = (struct pv_header *) ((caddr_t) pv_ptr + pventry_size);
	  /* set pointer to point at the next PV entry in the list of
	     physical volumes in the volume group descriptor area */

      } /* loop for each PV in the VGDA */

      if (pv_index == vghdr_ptr -> numpvs)
	/* if a matching PV id was not found in the VGDA for this PV from
	   the input list */

      {

	  varyonvg -> vvg_out.pv[in_index].pv_status = LVM_INVPVID;
	    /* set status for this PV in the varyonvg output structure to
	       indicate it has an invalid PV id (i.e., it is not a member
	       of the VG) */

	  chkvvgout = TRUE;
	    /* set flag to true to indicate varyonvg output structure
	       should be checked for additional information */

      } /* PV id from input list not found in VGDA */

  } /* not a duplicate PV */

} /* loop for each PV in input list */


pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to the
     beginning of the list of physical volumes in the volume group
     descriptor area */

for (pv_index = 0; pv_index < vghdr_ptr->numpvs; pv_index = pv_index+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* save PV number for this PV */

  for (in_index=0; in_index<varyonvg->vvg_in.num_pvs; in_index=in_index+1)
    /* loop for each PV in the input list, searching for a PV with
       matching PV id */

  {

    if (pv_ptr -> pv_id.word1 ==
	varyonvg -> vvg_out.pv[in_index].pv_id.word1  &&
	pv_ptr -> pv_id.word2 ==
	varyonvg -> vvg_out.pv[in_index].pv_id.word2)
      /* if PV is found in input list which has PV id which matches that
	 of the PV from the volume group descriptor area */

    {

	break;
	  /* break from for loop since we have found the matching PV we
	     were searching for */

    } /* matching PV id found in input list */

  } /* loop for each PV in input list */

  if (in_index == varyonvg -> vvg_in.num_pvs)
    /* if PV from the volume group descriptor area was not found in the
       input list */

  {

      out_index = varyonvg -> vvg_out.num_pvs;
	/* set index into varyonvg output structure at next available
	   slot where PV id will be stored for PV not passed in the input
	   list */

      varyonvg -> vvg_out.num_pvs = varyonvg -> vvg_out.num_pvs + 1;
	/* increment number of PVs in the varyonvg output structure */

      varyonvg -> vvg_out.pv[out_index].pv_id = pv_ptr -> pv_id;
	/* store into varyonvg output structure the PV id of the PV not
	   passed in the input list */

      if (!(TSTSA_PVMISS (&vgsa, pv_num - 1)))
	/* if this PV is not currently missing */

      {

	  varyonvg -> vvg_out.pv[out_index].pv_status = LVM_PVACTIVE;
	    /* set status for this PV in the varyonvg output structure
	       to indicate that this PV is currently active */

      }

      else
	/* PV not currently active */

      {

	  chkvvgout = TRUE;
	    /* set flag to true to indicate varyonvg output structure
	       should be checked for additional information */

	  varyonvg -> vvg_out.pv[out_index].pv_status = LVM_NONAME;
	    /* set status for this PV in the varyonvg output structure as
	       indicate device name was not passed for this PV */

	  for (in_index = 0; in_index < varyonvg -> vvg_in.num_pvs;
	       in_index = in_index + 1)
	    /* loop for PVs in the input list and compare PV ids passed
	       in by the user with PV id from the VGDA for which we did
	       not have a name;  this is done in case the user may have
	       passed in the PV id, but it was not the real PV id of
	       the corresponding physical volume device name passed in */

	  {

	    if (varyonvg -> vvg_out.pv[out_index].pv_id.word1 ==
		varyonvg -> vvg_in.pv[in_index].pv_id.word1   &&
		varyonvg -> vvg_out.pv[out_index].pv_id.word2 ==
		varyonvg -> vvg_in.pv[in_index].pv_id.word2)
	      /* if matching PV id is found in the input list */

	    {

		varyonvg -> vvg_out.pv[out_index].pv_status =
						  LVM_NAMIDNMTCH;
		  /* set PV status to indicate that this PV id was sent
		     in the input list but that it does not match the
		     real PV id of the named physical volume */

	    }

	  } /* loop for PVs in the input list */

      } /* PV not currently active */

  } /* PV from VGDA was not in the input list */

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

} /* loop for each PV in the VGDA */

for (out_index=0; out_index < varyonvg -> vvg_out.num_pvs;
     out_index=out_index+1)
  /* loop for all entries in the varyonvg output structure in order to
     check the PV status */

{

  if (varyonvg -> vvg_out.pv[out_index].pv_status != LVM_PVACTIVE)
    /* if there is a PV in the output structure which has a status other
       than that of active PV */

  {

      chkvvgout = TRUE;
	/* set flag to indicate that the varyonvg output structure has
	   important status information */

  }

}

close (vg_fd);
  /* close the volume group reserved area logical volume */

lvm_clsinpvs (varyonvg, inpvs_info);
  /* call routine to close the open physical volumes */

retcode = LVM_SUCCESS;
close (vgmap_fd);
  /* close the mapped file */

/************************************************************************
 *   Resynchronize the volume group if user has requested this.         *
 ************************************************************************
 */

if (varyonvg -> auto_resync == TRUE)
  /* if caller has requested that stale partitions be resynced */

{

retcode = lvm_vonresync (&(varyonvg -> vg_id));
  /* call routine to resync all logical volumes in the volume group */

}


if (chkvvgout == TRUE)
  /* if there is information in the varyonvg output structure which needs
     to be checked */

{

    retcode = LVM_CHKVVGOUT;
      /* set positive return code which indicates to check the varyonvg
	 output structure */

}

return (retcode);
  /* return with return code */

} /* END lvm_revaryon */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_vonmisspv                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine adds a previously missing PV into the kernel, writes *
 *   out the current volume group descriptor area to any VGDA copies   *
 *   on the PV, and updates information about the PV in the mapped     *
 *   file header.                                                      *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     maphdr_ptr                                                      *
 *     vgda_ptr                                                        *
 *     pv_ptr                                                          *
 *     vg_fd                                                           *
 *     in_index                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *chkvvgout                                                      *
 *     varyonvg -> vvg_out                                             *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_vonmisspv (

struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */

struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */

struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */

caddr_t vgda_ptr,
  /* pointer to the beginning of the volume group descriptor area */

struct pv_header * pv_ptr,
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */

short int in_index,
  /* index into the input list of a physical volume */

int * chkvvgout)
  /* flag to indicate if the varyonvg output structure should be
     checked */


{ /* BEGIN lvm_vonmisspv */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */


daddr_t vgsa_lsn [LVM_PVMAXVGDAS];
  /* array of logical sectors numbers for the VGSA copies on a PV */

int status;
  /* status for comparison between two timestamp values */

int retcode;
  /* return code */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct vg_trailer * vgtrail_ptr;
  /* pointer to the volume group descriptor area trailer record */

long partlen_blks;
  /* the length of a partition in number of 512 byte blocks */

daddr_t dalsn;
  /* the logical sector number within the volume group reserved area
     logical volume of where a VGDA copy begins */

short int wrt_order;
  /* variable to indicate in what order the descriptor area parts of
     header, body, and trailer will be written */

short int da_index;
  /* index for the VGDA copy on a PV */

short int sa_index;
  /* index for the VGSA copy on a PV */

short int wr_index;
  /* loop index for number of VGDAs to write to a PV */

short int pv_num;
  /* physical volume number */

short int quorum_cnt;
  /* number of VGDAs/VGSAs needed for a quorum */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


bzero (vgsa_lsn, sizeof (vgsa_lsn));
  /* zero out array of logical sectors numbers for the VGSA copies on
     one PV */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

vgtrail_ptr = (struct vg_trailer *) (vgda_ptr + (vghdr_ptr -> vgda_size
				     * DBSIZE) - DBSIZE);
  /* set a pointer to the trailer record of the VGDA */

quorum_cnt = vghdr_ptr -> total_vgdas / 2 + 1;
  /* calculate number of VGDAs/VGSAs needed for a quorum */

partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

pv_num = pv_ptr -> pv_num;
  /* save PV number for this PV */

for (sa_index=0; sa_index < pv_ptr -> pvnum_vgdas; sa_index=sa_index+1)
  /* loop for each copy of the VGSA on this PV */

{

  vgsa_lsn [sa_index] = partlen_blks * inpvs_info -> num_desclps *
	   (pv_num - 1) + inpvs_info -> pv[in_index].sa_psn[sa_index];
    /* calculate the logical sector number within the volume group
       reserved area logical volume of where this VGSA copy is
       located */

}

retcode = lvm_addpv (partlen_blks, inpvs_info -> num_desclps,
		     inpvs_info -> pv[in_index].device,
		     inpvs_info -> pv[in_index].fd, pv_num,
		     varyonvg -> vg_major, &(varyonvg -> vg_id),
		     inpvs_info -> pv[in_index].reloc_psn,
		     inpvs_info -> pv[in_index].reloc_len,
		     pv_ptr -> psn_part1, vgsa_lsn, quorum_cnt);
  /* add the previously missing PV into the kernel */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVMISSING;
      /* set PV status in the varyonvg output structure to indicate this
	 PV is still missing */

    *chkvvgout = TRUE;
      /* set flag to true to indicate varyonvg output structure should
	 be checked for additional information */

}

else

{

    varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVACTIVE;
      /* set PV status in the varyonvg output structure to indicate this
	 PV is now active */

    lvm_getdainfo (vg_fd, in_index, inpvs_info, defpvs_info);
      /* call routine to get timestamp information about the descriptor
	 area copies on this PV */

    maphdr_ptr -> pvinfo[pv_num - 1].device =
		    inpvs_info -> pv[in_index].device;
      /* set value for device major/minor number in mapped file header */

    strncpy (maphdr_ptr -> pvinfo[pv_num - 1].pvname,
	     varyonvg -> vvg_out.pv[in_index].pvname, LVM_NAMESIZ - 1);
     /* copy the device name of this physical volume into the PV
	information array in the mapped file header */

    for (da_index=0; da_index < LVM_PVMAXVGDAS; da_index=da_index+1)
      /* loop for each possible copy of the VGDA on a PV */

    {

      dalsn = partlen_blks * inpvs_info -> num_desclps * (pv_num - 1)
		+  inpvs_info -> pv[in_index].da_psn[da_index];
	/* calculate the logical sector number within the volume group
	   reserved area logical volume of where this VGDA copy is
	   located */

      maphdr_ptr -> pvinfo[pv_num - 1].da[da_index].dalsn = dalsn;
	/* store logical sector number for this VGDA copy in mapped file
	   header */

      if (inpvs_info -> pv[in_index].da[da_index].ts_status ==
						  LVM_BTSEQETS)
	/* if this VGDA copy has a beginning and ending timestamp that
	   match, and is therefore a good VGDA copy */

      {

	  maphdr_ptr -> pvinfo[pv_num - 1].da[da_index].ts =
			inpvs_info -> pv[in_index].da[da_index].ts_beg;
	    /* save timestamp value of this VGDA copy;  otherwise,
	       timestamp value will be 0 */

      }

    } /* loop for each VGDA copy on a PV */

    da_index = inpvs_info -> pv [in_index].index_nextda;
      /* set index for descriptor area to the index of the VGDA copy on
	 this PV which is the next to be written */

    for (wr_index=0; wr_index < pv_ptr->pvnum_vgdas; wr_index=wr_index+1)
      /* loop for number of VGDA copies to be written to this PV */

    {

      wrt_order = LVM_BEGMIDEND;
	/* initialize write order for VGDA copy to beginning/middle/end */

      if (inpvs_info -> pv[in_index].da[da_index].ts_status ==
						  LVM_TSRDERR)
	/* if the timestamp status indicates a read error on beginning or
	   ending timestamp for this VGDA copy */

      {

	  wrt_order = LVM_ZEROETS;
	    /* set write order to indicate end timestamp needs to be
	       zeroed before writing this VGDA copy */

      }

      else
	/* no read errors on begin or end timestamps */

      {

	  status = lvm_tscomp (&(inpvs_info -> pv[in_index].da[da_index].
			       ts_end), &(vghdr_ptr -> vg_timestamp));
	    /* call routine to compare the end timestamp for this VGDA
	       copy with the timestamp for the current VGDA */

	  if (status == LVM_EQUAL)
	    /* if end timestamp for this VGDA copy is equal to the current
	       descriptor area timestamp */

	  {

	      wrt_order = LVM_MIDBEGEND;
		/* write order for this VGDA must be middle/beginning/end
		   since its end timestamp matches that of the current VGDA
		   which is to be written out */

	  }

      } /* no read errors on timestamps */

      retcode = lvm_wrtdasa (vg_fd, vgda_ptr, (struct timestruc_t *)
	   &(vgtrail_ptr -> timestamp), inpvs_info -> vgda_len,
	   maphdr_ptr -> pvinfo[pv_num-1].da[da_index].dalsn, wrt_order);
	/* call routine to write a copy of the VGDA at the specified
	   logical sector */

      if (retcode == LVM_SUCCESS)
	/* if successful write of this VGDA copy */

      {

	  maphdr_ptr -> pvinfo[pv_num - 1].da[da_index].ts =
			  vghdr_ptr -> vg_timestamp;
	    /* update the timestamp value for this VGDA copy which is
	       saved in the mapped file header */

      }

      else
	/* this VGDA copy was not successfully written */

      {

	  maphdr_ptr->pvinfo[pv_num - 1].da[da_index].ts.tv_sec = 0;
	  maphdr_ptr->pvinfo[pv_num - 1].da[da_index].ts.tv_nsec = 0;
	    /* set to 0 the timestamp value for this VGDA copy which is
	       saved in the mapped file header, since there is not a good
	       copy at this VGDA */

	  retcode = lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major,
	       pv_num, inpvs_info -> num_desclps, HD_KMISSPV, quorum_cnt);
	    /* make this a missing PV in the kernel since we could not
	       update its VGDA copy */

	  if (retcode == LVM_FORCEOFF)
	    /* if an error is returned for forced varyoff */

	  {

	      return (retcode);
		/* return to user with forced varyoff return code */
	  }

	  varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVMISSING;
	    /* set PV status in the varyonvg output structure to indicate
	       this PV is currently missing */

      }

      da_index = da_index ^ 1;
	/* exclusive OR the VGDA copy index with 1 in order to flip-flop
	   the index value from 0 to 1 or 1 to 0 */

    } /* loop for number of VGDA copies on this PV */

} /* PV was successfully added to kernel */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_vonmisspv */







