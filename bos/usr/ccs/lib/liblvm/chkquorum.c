static char sccsid[] = "@(#)38	1.3  src/bos/usr/ccs/lib/liblvm/chkquorum.c, liblvm, bos411, 9428A410j 9/11/92 15:36:26";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - chkquorum.c
 *
 * FUNCTIONS: lvm_chkquorum,
 *            lvm_vgsamwcc
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
#include <sys/sysmacros.h>;
#include <sys/dasd.h>;
#include <sys/hd.h>;
#include <sys/vgsa.h>;
#include <sys/hd_psn.h>;
#include <sys/bootrecord.h>;
#include <lvmrec.h>;
#include <sys/hd_config.h>;
#include <liblvm.h>;








static void check_all_removed(struct defpvs_info *, struct inpvs_info *, struct pv_header *, struct vg_header *);

/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chkquorum                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to find if a quorum of volume group         *
 *   descriptor areas and a quorum of volume group status areas exist. *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     vg_fd                                                           *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     vgda_ptr                                                        *
 *     vgsa_ptr                                                        *
 *     vgsa_lsn                                                        *
 *     mwcc                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_NOQUORUM                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chkquorum (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */

struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */

struct vgsa_area ** vgsa_ptr,
  /* pointer to the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses for all VGSA copies */

char mwcc [DBSIZE])
  /* buffer which contains the latest mirror write consistency cache */


{ /* BEGIN lvm_chkquorum */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

daddr_t psn_part1;
  /* the physical sector number of the first partition on a physical
     volume */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

int retcode;
  /* return code */

int status;
  /* status for comparison between two timestamp values */

int quorum;
  /* the number of VGDAs needed for a quorum (i.e., more than half) */

int goodvgdas;
  /* the number of VGDAs that can be counted as present and good for
     comparing with the quorum count */

short int pv_num;
  /* the number of the PV */

short int da_index;
  /* index variable for VGDA copy */

short int in_index;
  /* index variable for input list */

short int pv_index;
  /* index variable for physical volume */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


goodvgdas = 0;
  /* initialize count of currently good VGDA copies to 0 */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

psn_part1 = LVM_PSNFSTPP (PSN_NONRSRVD, inpvs_info -> lvmarea_len);
  /* calculate the physical sector number of the first partition on a
     physical volume */

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* find the length of a physical partition in 512 byte blocks */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
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

/*
 * if we've been asked to force varyon this puppy, walk through the
 * pv's once.  if they're all marked removed, find the one with the
 * newest * vgda, and mark it active, just to give the vg a chance...
 */ 
if (varyonvg->override == TRUE)
	check_all_removed(defpvs_info, inpvs_info, pv_ptr, vghdr_ptr);
	
for (pv_index = 0; pv_index < vghdr_ptr->numpvs; pv_index = pv_index+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get PV number of this PV from the PV header in the VGDA */

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if PV with this PV number is defined into the kernel */

  {

      in_index = defpvs_info -> pv[pv_num - 1].in_index;
	/* get index into input list for the PV which is defined into
	   the kernel with this PV number */

      if (inpvs_info->pv[in_index].pv_id.word1 != pv_ptr->pv_id.word1
	  || inpvs_info->pv[in_index].pv_id.word2 != pv_ptr->pv_id.word2)
	/* if the PV id from this PV entry in the VGDA is not equal to
	   the PV id of the PV defined into the kernel with this PV
	   number */

      {

	  lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major, pv_num,
		     inpvs_info -> num_desclps, HD_KDELPV, (short int) 0);
	    /* delete the PV currently defined into the kernel at this
	       PV number since it is not the desired one */

	  defpvs_info -> pv[pv_num - 1].pv_status = LVM_NOTDFND;
	    /* set status for this PV number as not defined in the
	       structure which describes PVs defined into the kernel */

	  for (in_index = 0;  in_index < varyonvg -> vvg_in.num_pvs;
	       in_index = in_index + 1)
	    /* loop for each PV in the input list, searching for a
	       mtaching PV id */

	  {

	    if (inpvs_info -> pv[in_index].pv_status == LVM_VALIDPV)
	      /* if PV status flag from the input list indicates this
		 is a valid PV (i.e., was found to be member of specified
		 volume group) */

	    {

		if (inpvs_info -> pv[in_index].pv_id.word1 ==
					       pv_ptr -> pv_id.word1  &&
		    inpvs_info -> pv[in_index].pv_id.word2 ==
					       pv_ptr -> pv_id.word2)
		  /* if PV id of this PV from the input list matches PV
		     id of this PV entry in the VGDA */

		{

		    retcode = lvm_addpv (partlen_blks,
			      inpvs_info -> num_desclps,
			      inpvs_info -> pv[in_index].device,
			      inpvs_info -> pv[in_index].fd, pv_num,
			      varyonvg -> vg_major, &(varyonvg -> vg_id),
			      inpvs_info -> pv[in_index].reloc_psn,
			      inpvs_info -> pv[in_index].reloc_len,
			      psn_part1, &(vgsa_lsn[pv_num-1][0]),
			      (short int) 0);
		      /* add the PV from the input list which has correct
			 PV id into the kernel at this PV number */

		    if (retcode == LVM_SUCCESS)
		      /* if no error from add of PV */

		    {

			defpvs_info -> pv[pv_num - 1].pv_status =
						      LVM_DEFINED;
			  /* set status flag to show PV with this PV
			     number is defined into the kernel */

			defpvs_info -> pv[pv_num - 1].in_index = in_index;
			  /* save index into the input list for the PV
			     which has been defined into the kernel */

		    }

		    break;
		      /* break from loop since a matching PV id has been
			 found */

		} /* PV id match from input list with PV in VGDA */

	    } /* valid PV in input list */

	  } /* loop for each PV in input list */

      } /* PV id of PV in VGDA does not match PV defined in kernel */

      else
	/* PV id of PV in VGDA matches that of PV defined in kernel */

      {

	  if (pv_ptr -> pv_state & LVM_PVREMOVED)
	    /* if the PV state in the PV header indicates this is a
	       removed PV */

	  {

	      lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major,
			 pv_num, inpvs_info -> num_desclps, HD_KDELPV,
			 (short int) 0);
		/* delete the PV currently defined into the kernel at
		   this PV number since it is a removed PV */

	      defpvs_info -> pv[pv_num - 1].pv_status = LVM_NOTDFND;
		/* set status for this PV number as not defined in the
		   structure which describes PVs defined into the
		   kernel */

	  }

      } /* PV id of PV in VGDA matches that of PV defined in kernel */

  } /* PV with this PV number defined in kernel */

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


for (pv_num = 1; pv_num <= LVM_MAXPVS; pv_num = pv_num + 1)
  /* loop for all possible PV numbers */

{

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if PV with this PV number is defined into the kernel */

  {

      in_index = defpvs_info -> pv[pv_num - 1].in_index;
	/* get index into input list for this PV */


      /******************************************************************
       *   Search VGDA for PV which is defined in the kernel.           *
       ******************************************************************
       */

      pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
	/* set the pointer to the first physical volume entry to point to
	   the beginning of the list of physical volumes in the volume
	   group descriptor area */

      for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
	/* loop for each physical volume in the volume group descriptor
	   area, searching for PV id which matches that of defined PV */

      {

	if (inpvs_info->pv[in_index].pv_id.word1 == pv_ptr->pv_id.word1 &&
	    inpvs_info->pv[in_index].pv_id.word2 == pv_ptr->pv_id.word2)
	  /* if PV id of this PV in the VGDA matches PV id of the PV with
	     this PV number which is defined into the kernel */

	{

	    for (da_index=0; da_index<LVM_PVMAXVGDAS; da_index=da_index+1)
	      /* loop for all possible VGDA copies on this PV */

	    {

	      inpvs_info -> pv[in_index].da[da_index].wrt_order =
							  LVM_BEGMIDEND;
		/* initialize write order for this VGDA copy, if it is
		   updated, to beginning/middle/end */

	      if (inpvs_info -> pv[in_index].da[da_index].ts_status !=
							    LVM_TSRDERR)
		/* if the timestamp status indicates no read errors on
		    beginning or ending timestamp for this VGDA copy */

	      {

		  status = lvm_tscomp (&(inpvs_info -> pv[in_index].da
		      [da_index].ts_end), &(defpvs_info -> newest_dats));
		    /* call routine to compare the end timestamp for this
		       VGDA copy with the timestamp of newest consistent
		       VGDA */

		  if (status != LVM_GREATER)
		    /* if end timestamp for this VGDA copy is less than
		       or equal to newest VGDA */

		  {

		      if (inpvs_info -> pv[in_index].index_newestda ==
			  da_index  &&  pv_ptr -> pvnum_vgdas > 0)
			/* if this VGDA is the newest copy on this PV and
			   if this PV has at least one VGDA copy which is
			   included in the total count */

		      {

			  goodvgdas = goodvgdas + 1;
			    /* increment the count of good VGDA copies
			       which can be counted for the quorum */

		      }

		      if (inpvs_info->pv[in_index].index_newestda !=
			da_index && pv_ptr->pvnum_vgdas == LVM_PVMAXVGDAS)
			/* if this VGDA is not the newest copy on this PV
			   but if both VGDA copies on this PV are included
			   in the total count */

		      {

			  goodvgdas = goodvgdas + 1;
			    /* increment the count of good VGDA copies
			       which can be counted for the quorum */

		      }

		      if (status == LVM_EQUAL)
			/* if the end timestamp for this VGDA copy equals
			   the newest VGDA timestamp */

		      {

			  inpvs_info->pv[in_index].da[da_index].wrt_order
							 = LVM_MIDBEGEND;
			    /* since newest VGDA copy will be used for
			       updating bad VGDA copies, write order for
			       VGDAs with matching end timestamp must be
			       changed to middle/beginning/end */

			  if (inpvs_info -> pv[pv_index].da[da_index].
					    ts_status != LVM_BTSEQETS)
			    /* if the beginning and ending timestamps do
			       not match for this VGDA copy */

			  {

			      inpvs_info -> pv[in_index].da[da_index].
						       wrt_status = TRUE;
				/* set write status to true to indicate
				   this VGDA copy needs to be updated if
				   it is counted in total VGDAs */

			  } /* end equals newest, begin not equal end */

		      } /* end timestamp equals newest */

		      else
			/* ending timestamp less than newest */

		      {

			  inpvs_info -> pv[in_index].da[da_index].
						     wrt_status = TRUE;
			    /* set write status to true to indicate this
			       VGDA copy needs to be updated if it is
			       counted in total VGDAs */

		      }

		  } /* end timestamp less than or equal newest */

	      } /* end for no read errors on timestamps */

	      else
		/* read error on begin and/or end timestamp */

	      {

		  inpvs_info -> pv[in_index].da[da_index].wrt_order =
							  LVM_ZEROETS;
		    /* set write order to indicate that end timestamp of
		       this VGDA copy should be zeroed before writing
		       since we don't know what value it might currently
		       have */

		  inpvs_info -> pv[in_index].da[da_index].wrt_status =
							  TRUE;
		    /* set write status to true to indicate this VGDA
		       copy needs to be updated if it is counted in total
		       VGDAs */
	      }

	    } /* loop for each VGDA copy on a PV */

	    break;
	      /* break from loop since searched for PV has been found
		 in the VGDA */

	} /* PV defined in the kernel was found in the VGDA */

	size = sizeof (struct pv_header) + pv_ptr -> pp_count *
					   sizeof (struct pp_entries);
	  /* calculate the size of the space needed to contain the header
	     and the physical partition entries for this PV */

	pventry_size = LVM_SIZTOBBND (size);
	  /* calculate the actual size reserved in the descriptor area
	     for this PV entry by rounding the size up to the nearest
	     multiple of the blocksize of the physical volume */

	pv_ptr = (struct pv_header *) ((caddr_t) pv_ptr + pventry_size);
	  /* set pointer to point at the next PV entry in the list of
	     physical volumes in the volume group descriptor area */

      } /* loop for each PV in the VGDA */

      if (pv_index == vghdr_ptr -> numpvs)
	/* if the PV defined into the kernel was not found in the VGDA */

      {

	  lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major,
		     pv_num, inpvs_info -> num_desclps, HD_KDELPV,
		     (short int) 0);
	    /* delete the PV currently defined into the kernel at this
	       PV number since it could not be found in the VGDA */

	  defpvs_info -> pv[pv_num - 1].pv_status = LVM_NOTDFND;
	    /* set status for this PV number as not defined in the
	       structure which describes PVs defined into the kernel */

      }


  } /*  PV with this PV number is defined into kernel */

} /* loop for each possible PV number */


quorum = (vghdr_ptr -> total_vgdas) / 2 + 1;
  /* using total number of VGDAs from the volume group header, calculate
     the number of good VGDAs needed to have a quorum (i.e., more than
     half) */

retcode = lvm_vgsamwcc (vg_fd, inpvs_info, defpvs_info, vgda_ptr, quorum,
			vgsa_ptr, vgsa_lsn, mwcc);
  /* call routine to read and find latest copy for the volume group status
     area and the mirror write consistency cache */

if (retcode == LVM_NOQUORUM  ||
     (retcode == LVM_SUCCESS && goodvgdas < quorum))
  /* if no other errors occurred but we do not have a quorum of either
     the VGDAs or the VGSAs */

{

    return (LVM_NOQUORUM);
      /* return error code for no quorum */

}

else

{

    return (retcode);
      /* return with return code from lvm_vgsamwcc */

}

} /* END lvm_chkquorum */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_vgsamwcc                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads the volume group status area and the mirror    *
 *   write consistency cache record from each PV and finds the latest  *
 *   copy of each.                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_fd                                                           *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     vgda_ptr                                                        *
 *     quorum                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     defpvs_info                                                     *
 *     *vgsa_ptr                                                       *
 *     vgsa_lsn                                                        *
 *     mwcc                                                            *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_ALLOCERR                                                    *
 *     LVM_NOVGDAS                                                     *
 *     LVM_NOQUORUM                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_vgsamwcc (

int vg_fd,
  /* file descriptor for the VG reserved area logical volume which
     contains the volume group descriptor area and status area */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs defined
     into the kernel */

caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */

long quorum,
  /* number of VGDAs/VGSAs needed to varyon in order to ensure that the
     volume group data is consistent with that from previous varyon */

struct vgsa_area ** vgsa_ptr,
  /* variable to contain the pointer to the buffer which will contain
     the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array in which to store the logical sector number addresses of the
     VGSAs for each PV */

char mwcc [DBSIZE])
  /* buffer in which the latest mirror write consistency cache will be
     returned */


{ /* BEGIN lvm_vgsamwcc */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char savbuf [DBSIZE];
  /* buffer into which one block can be read */

struct mwc_rec * mwcc_ptr;
  /* a pointer to the buffer containing the MWCC */

caddr_t vgsa2;
  /* buffer into which the VGSA will be read */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

int goodvgsas;
  /* the number of VGSAs that can be counted as present and good for
     comparing with the quorum count */

short int pv_num;
  /* the number of the PV */

short int in_index;
  /* index variable for input list */

short int pv_index;
  /* index variable for physical volume */

daddr_t salsn;
  /* the logical sector number within the volume group reserved area
     logical volume of where this copy of the VGSA resides */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the logical volume where
     the file pointer is to be placed for the next read */

short int sa_index;
  /* index variable for the copy of the status area on the PV */

short int status;
  /* variable to hold the status of a comparison between two timestamp
     values */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Allocate space for the volume group status area.                  *
 ***********************************************************************
 */

*vgsa_ptr = (struct vgsa_area *) malloc (inpvs_info -> vgsa_len * DBSIZE);
  /* allocate space for a copy of the volume group status area */

if (*vgsa_ptr == NULL)
  /* if error occurred */

{

    return (LVM_ALLOCERR);
      /* return error code for allocation error */

}

vgsa2 = (caddr_t) malloc (inpvs_info -> vgsa_len * DBSIZE);
  /* allocate space for a second copy of the volume group status area */

if (vgsa2 == NULL)
  /* if error occurred */

{

    free ((caddr_t) *vgsa_ptr);
      /* free space allocated for volume group status area */

    return (LVM_ALLOCERR);
      /* return error code for allocation error */

}

goodvgsas = FALSE;
  /* initialize good VGSAs flag to FALSE to indicate we have not yet
     found a good copy of the VGSA */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* find the length of a physical partition in 512 byte blocks */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
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

for (pv_index = 0; pv_index < vghdr_ptr->numpvs; pv_index = pv_index+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get PV number of this PV from the PV header in the VGDA */

  in_index = defpvs_info -> pv[pv_num - 1].in_index;
    /* get index into the input list for the PV which is defined with
       PV number */

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if PV with this PV number is defined into the kernel */

  {

      /*****************************************************************
       *   Read each VGSA copy and find the latest.                    *
       *****************************************************************
       */

      for (sa_index=0; sa_index<pv_ptr->pvnum_vgdas; sa_index=sa_index+1)
        /* loop for number of status area copies on this PV */

      {

        defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_order =
                                                   LVM_BEGMIDEND;
          /* set value to indicate VGSA should be written in order of
             beginning/middle/end */

        salsn = partlen_blks * inpvs_info -> num_desclps * (pv_num - 1) +
                  inpvs_info -> pv[in_index].sa_psn[sa_index];
          /* calculate the logical sector number (LSN) within the volume
             group reserved area logical volume for this copy of the
             status area */

        vgsa_lsn [pv_num-1] [sa_index] = salsn;
          /* save LSN in array which is to be passed to kernel config */

        offset = DBSIZE * salsn;
          /* calculate the offset in bytes from beginning of the logical
             volume for the first record of this status area */

        offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
          /* position file pointer to beginning of status area */

        if (offset == LVM_UNSUCC)
          /* if an error occurred */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status =
                                                       LVM_TSRDERR;
              /* turn on bit in timestamp status flag to indicate a read
                 error on the beginning timestamp */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
              /* set flag to indicate that this copy of the VGSA needs to
                 be updated */

            continue;
              /* continue with next copy of VGSA */

	}

        retcode = read (vg_fd, savbuf, DBSIZE);
          /* read the first record of this volume group status area */

        if (retcode == DBSIZE)
          /* if read was successful */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_beg =
                           * ((struct timestruc_t *) savbuf);
              /* store the beginning timestamp of this VGSA copy */

	}

	else
	  /* error on read */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status =
                                                       LVM_TSRDERR;
              /* turn on bit in timestamp status flag to indicate a
                 read error on the beginning timestamp */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
              /* set flag to indicate that this copy of the VGSA needs to
                 be updated */

            continue;
              /* continue with next copy of VGSA */

	}

        offset = DBSIZE * (salsn + inpvs_info -> vgsa_len - 1);
          /* calculate the offset in bytes from beginning of the logical
             volume for this status area's last record */

        offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
          /* position file pointer to beginning of status area */

        if (offset == LVM_UNSUCC)
          /* if an error occurred */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status =
                                                       LVM_TSRDERR;
              /* turn on bit in timestamp status flag to indicate a read
                 error on the ending timestamp */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
              /* set flag to indicate that this copy of the VGSA needs to
                 be updated */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_order =
                                                       LVM_ZEROETS;
              /* set write order to indicate to first zero out the end
                 timestamp before writing remainder of VGSA */

            continue;
              /* continue with next copy of VGSA */

	}

        retcode = read (vg_fd, savbuf, DBSIZE);
          /* read the last record of this VGSA */

        if (retcode == DBSIZE)
          /* if read was successful */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_end =
               * ((struct timestruc_t *) ((int) savbuf + DBSIZE -
                                          sizeof (struct timestruc_t)));
              /* store the ending timestamp of this VGSA copy */

	}

	else
	  /* error on read */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status =
                                                       LVM_TSRDERR;
              /* turn on bit in timestamp status flag to indicate a
                 read error on the ending timestamp */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
              /* set flag to indicate that this copy of the VGSA needs to
                 be updated */

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_order =
                                                       LVM_ZEROETS;
              /* set write order to indicate to first zero out the end
                 timestamp before writing remainder of VGSA */

            continue;
              /* continue with next copy of VGSA */

	}

        status = (short int) lvm_tscomp (
                  &(defpvs_info -> pv[pv_num-1].sa[sa_index].ts_beg),
                  &(defpvs_info -> pv[pv_num-1].sa[sa_index].ts_end));
          /* call routine to compare the beginning and ending timestamp
             values */

        defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status = status;
          /* turn on bits for status returned from compare which tells
             whether beginning timestamp is equal to, greater than, or
             less than ending timestamp */

        if (defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status !=
                                                       LVM_BTSEQETS)
          /* if this status area does not have a beginning and ending
             timestamp that match */

	{

            defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
              /* set flag to indicate that this copy of the VGSA needs to
                 be updated */

	}

	else
	  /* begin timestamp matches end timestamp */

	{

            offset = DBSIZE * salsn;
              /* calculate the offset in bytes from beginning of the
                 logical volume for this status area */

            offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
              /* position file pointer to beginning of status area */

            if (offset == LVM_UNSUCC)
              /* if an error occurred */

	    {

                continue;
                  /* continue with next copy of VGSA */

	    }

            retcode = read (vg_fd, vgsa2, inpvs_info->vgsa_len * DBSIZE);
              /* read this volume group status area */

            if (retcode != DBSIZE * inpvs_info->vgsa_len)
              /* if read was not successful */

	    {

                defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
                  /* set flag to indicate that this copy of the VGSA needs
                     to be updated */

	    }

	    else
	      /* successful read of this VGSA copy */

	    {

                status = (short int) lvm_tscomp (
                   &(defpvs_info -> pv[pv_num-1].sa[sa_index].ts_beg),
                   &(defpvs_info -> newest_sats));
                  /* call routine to do comparison of this status area's
                     timestamp with the current value for the newest
                     timestamp */

                if (status == LVM_GREATER)
                  /* if this status area has a newer timestamp */

		{

		    goodvgsas = TRUE;
		      /* set value to indicate good VGSA copy found */

                    defpvs_info -> newest_sats = defpvs_info ->
                                       pv[pv_num-1].sa[sa_index].ts_beg;
                      /* set value of newest timestamp to the timestamp of
                         this status area */

		    bcopy (vgsa2, (caddr_t) *vgsa_ptr, inpvs_info ->
			   vgsa_len * DBSIZE);
                      /* copy this newer VGSA copy into the buffer where
                         it will be returned to the caller */

		}

	    } /* successful read of VGSA */

	} /* beginning and ending timestamps match */

      } /* loop for each VGSA on this PV */


      /*****************************************************************
       *   Read each mirror write consistency cache and find latest.   *
       *****************************************************************
       */

      offset = PSN_MWC_REC0 * DBSIZE;
        /* calculate the offset in bytes from beginning of the physical
           disk for the mirror write consistency cache */

      offset = lseek (inpvs_info->pv[in_index].fd, offset, LVM_FROMBEGIN);
        /* position file pointer to beginning of MWCC */

      if (offset != LVM_UNSUCC)
        /* if no errors on seek */

      {

	  retcode = read (inpvs_info->pv[in_index].fd, savbuf, DBSIZE);
            /* read this mirror write consistency cache */

	  if (retcode == DBSIZE)
            /* if read was successful */

	  {

              defpvs_info -> pv[pv_num - 1].mwc.good_mwcc = TRUE;
                /* set flag to indicate we could read this PV's MWCC */

              mwcc_ptr = (struct mwc_rec *) savbuf;
                /* set pointer to the mirror write consistency cache */

              defpvs_info -> pv[pv_num-1].mwc.ts = mwcc_ptr -> e_tmstamp;
                /* save the timestamp for this PV's MWCC */

              status = (short int) lvm_tscomp (&(defpvs_info ->
                  pv[pv_num-1].mwc.ts), &(defpvs_info -> newest_mwcts));
                /* call routine to do comparison of this MWCC's
                   timestamp with the current value for the newest
                     timestamp */


              if (status == LVM_GREATER || status == LVM_EQUAL)
                /* if this MWCC has a newer or equal timestamp 
                   the equal is for a case of zero timestamp */
	      {

                  defpvs_info -> newest_mwcts = defpvs_info ->
                                                  pv[pv_num-1].mwc.ts;
                    /* set value of newest timestamp to the timestamp of
                       this MWCC */

		  bcopy (savbuf, mwcc, DBSIZE);
                    /* copy the newer MWCC into the buffer to contain the
                       latest copy */

	      }

	  } /* successful read of MWCC */

      }

  } /* PV with this PV number defined in kernel */

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

free (vgsa2);
  /* free area used for reading VGSA copies */

if (goodvgsas == FALSE)
  /* if we could not find a good copy of the volume group status area */

{

    free ((caddr_t) *vgsa_ptr);
      /* free space allocated for volume group status area */

    return (LVM_NOVGDAS);
      /* return error code to indicate no good VGDA or VGSA copy */

}

goodvgsas = 0;
  /* initialize number of good VGSAs to 0 */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
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

for (pv_index = 0; pv_index < vghdr_ptr->numpvs; pv_index = pv_index+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get PV number of this PV from the PV header in the VGDA */

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if PV with this PV number is defined into the kernel */

  {

      if (defpvs_info -> pv[pv_num-1].mwc.good_mwcc == FALSE)
        /* if this PV does not have a good MWCC */

      {

          defpvs_info -> pv[pv_num-1].mwc.wrt_status = TRUE;
            /* set flag to indicate that this MWCC area needs to be
               updated */

      }

      else

      {

          status = (short int) lvm_tscomp (
                     &(defpvs_info -> pv[pv_num-1].mwc.ts),
                     &(defpvs_info -> newest_mwcts));
              /* call routine to do comparison of this MWCC's timestamp
                 with the timestamp for the newest MWCC */

          if (status != LVM_EQUAL)
            /* if this MWCC has different timestamp */

	  {

              defpvs_info -> pv[pv_num-1].mwc.wrt_status = TRUE;
                /* set flag to indicate that this MWCC needs to be
                   updated */

	  }

      }


      for (sa_index=0; sa_index<pv_ptr->pvnum_vgdas; sa_index=sa_index+1)
        /* loop for number of status area copies on this PV */

      {

        if (defpvs_info -> pv[pv_num-1].sa[sa_index].ts_status !=
                                                       LVM_TSRDERR)
          /* if this status area had timestamps which could be read */

	{

            status = (short int) lvm_tscomp (
                     &(defpvs_info -> pv[pv_num-1].sa[sa_index].ts_end),
                     &(defpvs_info -> newest_sats));
              /* call routine to do comparison of this status area's end
                 timestamp with the current value for the newest
                 timestamp */

            if (status == LVM_GREATER)
              /* if this status area has an end timestamp that is greater
                 than the chosen VGSA */

	    {

                defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status = TRUE;
                  /* set flag to indicate that this copy of the VGSA needs
                     to be updated */


	    }

	    else
	      /* end timestamp <= timestamp of chosen copy of VGSA */

	    {

		goodvgsas = goodvgsas + 1;
		  /* increment number of good VGSAs if the end timestamp
		     for this VGSA copy is less than or equal to the
		     chosen timestamp */

                if (status == LVM_EQUAL)
		  /* if end timestamp is equal to chosen timestamp */

		{

                    defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_order =
                                                          LVM_MIDBEGEND;
		      /* change write order to middle/beginning/end in
			 case this VGSA copy is written */

		}

		else
		  /* end timestamp less than chosen timestamp */

		{

                    defpvs_info -> pv[pv_num-1].sa[sa_index].wrt_status =
                                                               TRUE;
                      /* set flag to indicate that this copy of the VGSA
                         needs to be updated */

		}

	    } /* end timestamp <= timestamp of chosen */

	} /* VGSA timestamps could be read */

      } /* loop for each VGSA on this PV */

  } /* PV with this PV number defined in kernel */

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


if (goodvgsas < quorum)
  /* if count of good VGSAs is less than number needed for quorum */

{

    return (LVM_NOQUORUM);
      /* return error for no quorum */

}

return (LVM_SUCCESS);
  /* return to caller with successful return code */

} /* END lvm_vgsamwcc */

/*
 * 	check all pvs in the vg.  if they're all marked removed,
 *	pick the one with (a) LVM_PVMAXVGDAS and (b) the latest vgda
 *	and mark it that pv available in an attempt to get the vg
 *	back on its feet.
 */
static void
check_all_removed(
struct defpvs_info	*defpvs_info,
struct inpvs_info	*inpvs_info,
struct pv_header	*pv_top,
struct vg_header	*vghdr_ptr
)
{
	long size;
	struct pv_header *pv, *save_pv;
	short in_idx, pv_ct, da_idx, pv_idx;

	/*
	 * do a 'quick' loop and ensure they're all removed...
	 * if not, get out as fast as possible
	 */
	for (pv = pv_top, pv_ct = 0; pv_ct < vghdr_ptr->numpvs; pv_ct++) {
		/* get PV number of this PV from the PV header in the VGDA */
		pv_idx = pv->pv_num - 1;

		/* get index into input list for this PV */
		in_idx = defpvs_info->pv[pv_idx].in_index;

		/*
		 * is this pv defined?
		 */
		if (defpvs_info->pv[pv_idx].pv_status == LVM_DEFINED &&
		    inpvs_info->pv[in_idx].pv_id.word1 == pv->pv_id.word1 &&
		    inpvs_info->pv[in_idx].pv_id.word2 == pv->pv_id.word2)
			/*
			 * if they're not all removed, get out...
			 */
			if (!(pv->pv_state & LVM_PVREMOVED))
				return;

		/*
		 * increment to the next pv entry.
		 */
		size = sizeof(struct pv_header) +
			pv->pp_count * sizeof(struct pp_entries);
		pv = (struct pv_header *) ((caddr_t) pv+(LVM_SIZTOBBND(size)));
		}

	/*
	 * now find the pv with the latest vgda
	 */
	save_pv = NULL;
	for (pv = pv_top, pv_ct = 0; pv_ct < vghdr_ptr->numpvs; pv_ct++) {
		/* get PV number of this PV from the PV header in the VGDA */
		pv_idx = pv->pv_num - 1;

		/* get index into input list for this PV */
		in_idx = defpvs_info->pv[pv_idx].in_index;

		/*
		 * is this pv defined?
		 * are there LVM_PVMAXVGDAS vgdas on this pv?
		 */
		if (defpvs_info->pv[pv_idx].pv_status == LVM_DEFINED &&
		    inpvs_info->pv[in_idx].pv_id.word1 == pv->pv_id.word1 &&
		    inpvs_info->pv[in_idx].pv_id.word2 == pv->pv_id.word2 &&
		    pv->pvnum_vgdas == LVM_PVMAXVGDAS)

			/* loop for all possible VGDA copies on this PV */
			for (da_idx = 0; da_idx < LVM_PVMAXVGDAS; da_idx++) {
				/*
				 * if the timestamp status indicates a read
				 * error on beginning or ending timestamp
				 * for this VGDA copy, skip it.
			 	*/
				if (inpvs_info->pv[in_idx].da[da_idx].ts_status == LVM_TSRDERR)
					continue;

				if (lvm_tscomp(&inpvs_info->pv[in_idx].da[da_idx].ts_end,
				     &defpvs_info->newest_dats) == LVM_EQUAL)
					save_pv = pv;
				}

		/*
		 * increment to the next pv entry.
		 */
		size = sizeof(struct pv_header) +
			pv->pp_count * sizeof(struct pp_entries);
		pv = (struct pv_header *) ((caddr_t) pv+(LVM_SIZTOBBND(size)));
		}

	/*
	 * found one?
	 */
	if (save_pv != NULL) {
		/*
		 * make it active.
		 */
		save_pv->pv_state =
			(save_pv->pv_state & LVM_PVMASK) | LVM_PVACTIVE;
		/*
		 * stuff a vgda count into the vg
		 */
		vghdr_ptr->total_vgdas = save_pv->pvnum_vgdas;

		/*
		 * mark the vgsas and vgdas to be updated...
		 */

		/* get PV number of this PV from the PV header in the VGDA */
		pv_idx = save_pv->pv_num - 1;
		/* get index into input list for this PV */
		in_idx = defpvs_info->pv[pv_idx].in_index;

		/*
		 * mark them
		 */
		for (da_idx = 0; da_idx < LVM_PVMAXVGDAS; da_idx++) {
			defpvs_info->pv[pv_idx].sa[da_idx].wrt_status = TRUE;
			inpvs_info->pv[in_idx].da[da_idx].wrt_status = TRUE;
			}
		}
}
