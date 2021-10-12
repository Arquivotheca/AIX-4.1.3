static char sccsid[] = "@(#)28	1.9.1.4  src/bos/usr/ccs/lib/liblvm/verify.c, liblvm, bos41B, 9504A 1/3/95 10:24:30";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - verify.c
 *
 * FUNCTIONS: lvm_verify,
 *            lvm_defpvs,
 *            lvm_getdainfo,
 *            lvm_readpvs,
 *            lvm_readvgda
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990, 1993
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
#include <sys/buf.h>
#include <sys/lockl.h>
#include <sys/param.h>
#include <sys/sleep.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <sys/dasd.h>
#include <sys/hd.h>
#include <sys/vgsa.h>
#include <sys/hd_psn.h>
#include <sys/bootrecord.h>
#include <lvmrec.h>
#include <sys/hd_config.h>
#include <liblvm.h>









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_verify                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to verify the consistency of each on-disk   *
 *   copy of the volume group descriptor area for the volume group.    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *vg_fd                                                          *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     *vgda_ptr                                                       *
 *     *vgsa_ptr                                                       *
 *     vgsa_lsn                                                        *
 *     mwcc                                                            *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_DALVOPN                                                     *
 *     LVM_NOQUORUM                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_verify (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

int * vg_fd,
  /* pointer to the variable to contain the file descriptor for the
     volume group reserved area logical volume */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */

struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

caddr_t * vgda_ptr,
  /* pointer to variable where the pointer to the volume group descriptor
     area is to be returned */

struct vgsa_area ** vgsa_ptr,
  /* pointer to variable where the pointer to the volume group status
     area is to be returned */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses for all VGSA copies */

char mwcc [DBSIZE])
  /* buffer which contains the latest mirror write consistency cache */


{ /* BEGIN lvm_verify */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

int rddaret;
  /* variable in which to save return code from lvm_readvgda */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

char vgdev [LVM_EXTNAME];
  /* the full path name of the volume group reserved area logical
     volume */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


retcode = status_chk (NULL, varyonvg -> vgname, NOCHECK, vgdev);
    /* build the path name for the volume group reserved area logical
       volume from the volume group name passed in */

if (retcode < LVM_SUCCESS)
    /* if an error occurred */

{

    return (retcode);
      /* return with error code */

}

bzero (inpvs_info, sizeof (struct inpvs_info));
  /* zero out the structure which is to contain information about the
     PVs in the user's input list */

bzero (defpvs_info, sizeof (struct defpvs_info));
  /* zero out the structure which is to contain information about the
     volume group descriptor area for PVs defined into the kernel */

bzero (vgsa_lsn, (LVM_MAXPVS * LVM_PVMAXVGDAS * sizeof (daddr_t)));
  /* zero out array of volume group status area logical sector numbers */

retcode = lvm_readpvs (varyonvg, inpvs_info);
  /* call routine to read information from each physical volume in the
     input list */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error code */

}

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* find the length of a physical partition in 512 byte blocks */

retcode = lvm_defvg (partlen_blks, inpvs_info -> num_desclps,
		     varyonvg -> kmid, varyonvg -> vg_major,
		     &(varyonvg -> vg_id), inpvs_info -> pp_size,
		     varyonvg -> noopen_lvs,
		     varyonvg -> noquorum);
  /* call routine to define the volume group into the kernel with an
     empty volume group reserved area logical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    lvm_clsinpvs (varyonvg, inpvs_info);
      /* call routine to close open PVs */

    return (retcode);
      /* return with error code */

}

*vg_fd = open (vgdev, O_RDWR | O_NSHARE);
  /* open the volume group reserved area logical volume */

if (*vg_fd == LVM_UNSUCC)
  /* if an error occurred */

{

    lvm_clsinpvs (varyonvg, inpvs_info);
      /* call routine to close open PVs */

    lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
      /* call routine to delete the volume group from the kernel */

    return (LVM_DALVOPN);
      /* return with error for volume group reserved area could not
	 be opened */

}

retcode = lvm_defpvs (varyonvg, *vg_fd, inpvs_info, defpvs_info);
  /* call routine to define the volume group's physical volumes into the
     kernel in order to read the VGDA from each */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (*vg_fd);
      /* close the volume group reserved area logical volume */

    lvm_clsinpvs (varyonvg, inpvs_info);
      /* call routine to close open PVs */

    lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
      /* call routine to delete the volume group from the kernel */

    return (retcode);
      /* return with error code */

}

retcode = lvm_readvgda (*vg_fd, varyonvg -> override, inpvs_info,
			defpvs_info, vgda_ptr);
  /* call routine to read the newest volume group descriptor area */

rddaret = retcode;
  /* save return code from the previous routine */

if (retcode == LVM_NOQUORUM)
  /* if the no quorum error is returned, meaning we could not read a
     VGDA copy for which we can guarantee data consistency */

{

    if (varyonvg -> override == FALSE)
      /* if user did not request to override the no quorum error */

    {

	close (*vg_fd);
	  /* close the volume group reserved area logical volume */

	lvm_clsinpvs (varyonvg, inpvs_info);
	  /* call routine to close open PVs */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete the volume group from the kernel */

	return (retcode);
	  /* return with no quorum error */

    }

} /* no quorum error returned */

retcode = lvm_chkquorum (varyonvg, *vg_fd, inpvs_info, defpvs_info,
			 *vgda_ptr, vgsa_ptr, vgsa_lsn, mwcc);
  /* call routine to determine if a quorum of volume group descriptor
     areas exist and to record which descriptor areas need updating */

if (rddaret == LVM_NOQUORUM  ||  retcode == LVM_NOQUORUM)
  /* if the no quorum error was returned from either lvm_readvgda or
     lvm_chkquorum */

{

    if (varyonvg -> override == FALSE)
      /* if user did not request to override the no quorum error */

    {

	close (*vg_fd);
	  /* close the volume group reserved area logical volume */

	lvm_clsinpvs (varyonvg, inpvs_info);
	  /* call routine to close open PVs */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete the volume group from the kernel */

    }

    return (LVM_NOQUORUM);
      /* return with error for no quorum */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_verify */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_defpvs                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine takes from the input list physical volumes whose     *
 *   LVM records indicate that they are members of the specified       *
 *   volume group and adds them into the kernel and then reads and     *
 *   compares the beginning and ending timestamps from the volume      *
 *   group descriptor areas and status areas.                          *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     vg_fd                                                           *
 *     inpvs_info                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     defpvs_info                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_NOVGDAS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_defpvs (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */

struct defpvs_info * defpvs_info)
  /* structure which contains information about the volume group
     descriptor and status areas for PVs defined into the kernel */


{ /* BEGIN lvm_defpvs */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

daddr_t vgsa_lsn [LVM_PVMAXVGDAS];
  /* array to hold the logical sector numbers of the VGSA copies for a
     PV */

int retcode;
  /* return code */

daddr_t psn_part1;
  /* the physical sector number of the first partition on the physical
     volume */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

int status;
  /* variable to hold the status of a comparison between two timestamp
     values */

short int pv_index;
  /* index variable for looping on physical volumes in input list */

short int pv_num;
  /* the number of the PV */

short int prevpv_index;
  /* variable in which to save the index into the input list of the PV
     previously defined into the kernel when we find a PV with a
     duplicate PV number */

short int prevpv_dandx;
  /* variable in which to save, for a PV previously defined in the
     kernel, the index of the VGDA copy with the newer timestamp */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


bzero (vgsa_lsn, sizeof (vgsa_lsn));
  /* zero out array of logical sectors numbers for the VGSA copies on
     one PV */

psn_part1 = LVM_PSNFSTPP (PSN_NONRSRVD, inpvs_info -> lvmarea_len);
  /* calculate the physical sector number of the first partition on a
     physical volume */

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* calculate the length of a partition in number of 512 byte blocks */

for (pv_index = 0; pv_index < varyonvg -> vvg_in.num_pvs;
     pv_index = pv_index+1)
  /* loop for each physical volume in the input list */

{

  if (inpvs_info -> pv[pv_index].pv_status == LVM_VALIDPV)
    /* if the PV status is valid in inpvs_info structure (i.e., PV was
       not missing and was found to be a valid member of the specified
       volume group) */

  {

      pv_num = inpvs_info -> pv[pv_index].pv_num;
	/* set variable to the PV number for this PV */

      if (defpvs_info -> pv[pv_num - 1].pv_status != LVM_DEFINED)
	/* if a physical volume with this PV number has not already been
	   defined into the kernel */

      {

	  retcode = lvm_addpv (partlen_blks, inpvs_info -> num_desclps,
			       inpvs_info -> pv[pv_index].device,
			       inpvs_info -> pv[pv_index].fd, pv_num,
			       varyonvg -> vg_major, &(varyonvg -> vg_id),
			       inpvs_info -> pv[pv_index].reloc_psn,
			       inpvs_info -> pv[pv_index].reloc_len,
			       psn_part1, vgsa_lsn, (short int) 0);
	    /* call routine to add this physical volume into the kernel */

	  if (retcode < LVM_SUCCESS)
	    /* if an error occurred */

	  {

	      continue;
		/* continue with the next PV in the input list */

	  }

	  defpvs_info -> pv[pv_num - 1].pv_status = LVM_DEFINED;
	    /* set the PV status to indicate this PV number has a PV
	       defined into the kernel */

	  defpvs_info -> pv[pv_num - 1].in_index = pv_index;
	    /* for this PV save the index into the information structure
	       for input PVs */

	  lvm_getdainfo (vg_fd, pv_index, inpvs_info, defpvs_info);
	    /* call routine to get timestamp information from the
	       volume group descriptor area copies on this PV */

      } /* this PV number not previously defined */

      else
	/* a PV with this PV number has been previously defined */

      {

	  prevpv_index = defpvs_info -> pv[pv_num - 1].in_index;
	    /* save input index of PV with this PV number which was
	       previously defined into the kernel */

	  if (inpvs_info -> pv[prevpv_index].pv_id.word1 ==
		inpvs_info -> pv[pv_index].pv_id.word1  &&
	      inpvs_info -> pv[prevpv_index].pv_id.word2 ==
		inpvs_info -> pv[pv_index].pv_id.word2)
	    /* if PV ids match for the current PV and the PV with this
	       PV number which was previously defined */

	  {

	      inpvs_info -> pv[pv_index].pv_status = LVM_NOTVLDPV;
		/* change the status in the inpvs_info structure to
		   not valid for this PV since it is a duplicate of
		   a previous PV in the input list */

	      varyonvg -> vvg_out.pv[pv_index].pv_status = LVM_DUPPVID;
		/* set status of this PV to duplicate PV id in the
		   varyonvg output structure */

	  }

	  else
	    /* PV ids do not match for current PV and previously
	       defined PV */

	  {

	      lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major,
		   inpvs_info -> pv[prevpv_index].pv_num,
		   inpvs_info -> num_desclps, HD_KDELPV, (short int) 0);
		/* delete the PV which was previously defined into the
		   kernel with this PV number */

	      defpvs_info -> pv[pv_num - 1].pv_status = LVM_NOTDFND;
		/* set status in defpvs_info structure to not defined
		   for this PV number */

	      retcode = lvm_addpv (partlen_blks,
			       inpvs_info -> num_desclps,
			       inpvs_info -> pv[pv_index].device,
			       inpvs_info -> pv[pv_index].fd, pv_num,
			       varyonvg -> vg_major, &(varyonvg -> vg_id),
			       inpvs_info -> pv[pv_index].reloc_psn,
			       inpvs_info -> pv[pv_index].reloc_len,
			       psn_part1, vgsa_lsn, (short int) 0);
		/* call routine to add this physical volume into the
		   kernel */

	      if (retcode == LVM_SUCCESS)
		/* if successful add of this PV */

	      {

		  defpvs_info -> pv[pv_num - 1].pv_status = LVM_DEFINED;
		    /* set the PV status to indicate this PV number has
		       a PV defined into the kernel */

		  defpvs_info -> pv[pv_num - 1].in_index = pv_index;
		    /* for this PV save the index into the information
		       structure for input PVs */

		  lvm_getdainfo (vg_fd, pv_index, inpvs_info,
				 defpvs_info);
		    /* call routine to get timestamp information from the
		       volume group descriptor area copies on this PV */

	      }

	      prevpv_dandx = inpvs_info ->
			       pv[prevpv_index].index_newestda;
		/* for PV previously defined, get index of VGDA copy for
		   newest VGDA copy on the PV */

	      if (inpvs_info -> pv[prevpv_index].da[prevpv_dandx].
		    ts_status == LVM_BTSEQETS)
		/* if the beginning and ending timestamps are equal for
		   the PV which was previously defined with this PV
		   number */

	      {

		  status = lvm_tscomp (&(defpvs_info -> newest_dats),
			     &(inpvs_info ->
			     pv[prevpv_index].da[prevpv_dandx].ts_beg));
		    /* call routine to compare timestamp of VGDA from
		       previously defined PV with the timestamp for the
		       newest VGDA */

		  if (status == LVM_EQUAL)
		    /* if the VGDA timestamp for the previous PV
		       indicates it is currently a candidate for the
		       newest VGDA */

		  {

		      lvm_delpv (&(varyonvg -> vg_id),
				 varyonvg -> vg_major,
				 inpvs_info -> pv[prevpv_index].pv_num,
				 inpvs_info -> num_desclps, HD_KDELPV,
				 (short int) 0);
			/* delete the current PV from the kernel so we
			   can add back the previous PV, which may later
			   be needed for reading the newest VGDA */

		      defpvs_info -> pv[pv_num-1].pv_status = LVM_NOTDFND;
			/* set status in defpvs_info structure to not
			   defined for this PV number */

		      retcode = lvm_addpv (partlen_blks,
			       inpvs_info -> num_desclps,
			       inpvs_info -> pv[prevpv_index].device,
			       inpvs_info -> pv[prevpv_index].fd, pv_num,
			       varyonvg -> vg_major, &(varyonvg -> vg_id),
			       inpvs_info -> pv[prevpv_index].reloc_psn,
			       inpvs_info -> pv[prevpv_index].reloc_len,
			       psn_part1, vgsa_lsn, (short int) 0);
			/* call routine to add the previously defined PV
			   back into the kernel */

		      if (retcode == LVM_SUCCESS)
			/* if successful add of previous PV */

		      {

			  defpvs_info -> pv[pv_num-1].pv_status =
							 LVM_DEFINED;
			    /* set the PV status to indicate this PV
			       number has a PV defined into the kernel */

			  defpvs_info -> pv[pv_num-1].in_index =
							 prevpv_index;
			    /* for the previous PV save the index into
			       the information structure for input PVs */

		      }

		  } /* previous PV candidate for newest VGDA */

	      } /* begin and end timestamps equal for previous PV */

	  } /* PV ids do not match for current PV and previous */

      } /* this PV number previously defined */

  } /* PV not missing and a valid member of VG */

} /* loop for each input PV */

if (defpvs_info -> newest_dats.tv_sec == 0  &&
       defpvs_info -> newest_dats.tv_nsec == 0)
  /* if we did not find a copy of the descriptor area with matching
     beginning and ending timestamps */

{

    return (LVM_NOVGDAS);
      /* return with error for good VGDA copy not found */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_defpvs */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_getdainfo                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads in the beginning and ending timestamps for     *
 *   the volume group descriptor areas on a PV and records status      *
 *   information about the timestamps.                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_fd                                                           *
 *     pv_index                                                        *
 *     inpvs_info                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     inpvs_info                                                      *
 *     defpvs_info -> newest_dats                                      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_getdainfo (

int vg_fd,
  /* file descriptor for the VG reserved area logical volume which
     contains the volume group descriptor area and status area */

short int pv_index,
  /* index variable for looping on physical volumes in input list */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */

struct defpvs_info * defpvs_info)
  /* pointer to structure which contains information about PVs defined
     into the kernel */


{ /* BEGIN lvm_getdainfo */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char vghdr_rec [DBSIZE];
  /* buffer into which the volume group header will be read */

char vgtrail_rec [DBSIZE];
  /* buffer into which the volume group trailer will be read */

struct vg_header * vghdr_ptr;
  /* structure into which the volume group header of the descriptor
     area will be read */

struct vg_trailer * vgtrail_ptr;
  /* structure into which the volume group trailer of the descriptor
     area will be read */

daddr_t dalsn;
  /* the logical sector number within the volume group reserved area
     logical volume of where this copy of the VGDA resides */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the logical volume where
     the file pointer is to be placed for the next read */

short int da_index;
  /* index variable for the copy of the descriptor area on the PV */

short int pv_num;
  /* the number of the PV */

short int status;
  /* variable to hold the status of a comparison between two timestamp
     values */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   For each VGDA copy on this PV, read beginning and ending          *
 *   timestamps.                                                       *
 ***********************************************************************
 */

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

pv_num = inpvs_info -> pv[pv_index].pv_num;
  /* set variable to the PV number for this PV */

for (da_index = 0; da_index < LVM_PVMAXVGDAS; da_index = da_index + 1)
  /* loop for each descriptor area copy on a PV */

{

  dalsn = partlen_blks * inpvs_info -> num_desclps * (pv_num - 1) +
	    inpvs_info -> pv[pv_index].da_psn[da_index];
    /* calculate the logical sector number (LSN) within the volume group
       reserved area logical volume for this copy of the descriptor
       area */

  offset = DBSIZE * dalsn;
    /* calculate the offset in bytes from beginning of the logical
       volume for this descriptor area's volume group header */

  offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
    /* position file pointer to beginning of descriptor area */

  if (offset == LVM_UNSUCC)
    /* if an error occurred */

  {

      inpvs_info -> pv[pv_index].da[da_index].ts_status = LVM_TSRDERR;
	/* turn on bit in timestamp status flag to indicate a read
	   error on the beginning timestamp */

      continue;
	/* continue with next copy of VGDA */

  }

  retcode = read (vg_fd, vghdr_rec, DBSIZE);
    /* read the volume group header */

  if (retcode == DBSIZE)
    /* if we read the entire header */

  {

      vghdr_ptr = (struct vg_header *) vghdr_rec;
	/* set pointer to volume group header */

      inpvs_info -> pv[pv_index].da[da_index].ts_beg =
					      vghdr_ptr -> vg_timestamp;
	/* store the beginning timestamp from the volume group header into
	   the array */

  }

  else
    /* error on read */

  {

      inpvs_info -> pv[pv_index].da[da_index].ts_status = LVM_TSRDERR;
	/* turn on bit in timestamp status flag to indicate a
	   read error on the beginning timestamp */

      continue;
	/* continue with next copy of VGDA */

  }

  offset = DBSIZE * (dalsn + inpvs_info -> vgda_len - 1);
    /* calculate the offset in bytes from beginning of the logical
       volume for this descriptor area's volume group trailer */

  offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
    /* position file pointer to beginning of descriptor area */

  if (offset == LVM_UNSUCC)
    /* if an error occurred */

  {

      inpvs_info -> pv[pv_index].da[da_index].ts_status = LVM_TSRDERR;
	/* turn on bit in timestamp status flag to indicate a read
	   error on the ending timestamp */

      continue;
	/* continue with next copy of VGDA */

  }

  retcode = read (vg_fd, vgtrail_rec, DBSIZE);
    /* read the volume group trailer */

  if (retcode == DBSIZE)
    /* if we read the entire trailer record */

  {

      vgtrail_ptr = (struct vg_trailer *) vgtrail_rec;
	/* set pointer to volume group header */

      inpvs_info -> pv[pv_index].da[da_index].ts_end =
					      vgtrail_ptr -> timestamp;
	/* store the ending timestamp from the volume group trailer into
	   the array */

  }

  else
    /* error on read */

  {

      inpvs_info -> pv[pv_index].da[da_index].ts_status = LVM_TSRDERR;
	/* turn on bit in timestamp status flag to indicate a
	   read error on the ending timestamp */

      continue;
	/* continue with next copy of VGDA */

  }


  /*********************************************************************
   *   If reads were good, compare beginning and ending timestamps for *
   *   this VGDA copy and save in timestamp status.  Also, compare     *
   *   this VGDA's timestamp with that of newest VGDA copy and record  *
   *   new value of newest if needed.                                  *
   *********************************************************************
   */

  status = (short int) lvm_tscomp (
	      &(inpvs_info -> pv[pv_index].da[da_index].ts_beg),
	      &(inpvs_info -> pv[pv_index].da[da_index].ts_end));
    /* call routine to compare the beginning and ending timestamp
       values */

  inpvs_info -> pv[pv_index].da[da_index].ts_status = status;
    /* turn on bits for status returned from compare which tells
       whether beginning timestamp is equal to, greater than, or
       less than ending timestamp */

  if (inpvs_info -> pv[pv_index].da[da_index].ts_status == LVM_BTSEQETS)
    /* if this descriptor area has a beginning and ending timestamp
       that match */

  {

      status = (short int) lvm_tscomp (
		  &(inpvs_info -> pv[pv_index].da[da_index].ts_beg),
		  &(defpvs_info -> newest_dats));
	/* call routine to do comparison of this descriptor area's
	   timestamp with the current value for the newest
	   timestamp */

      if (status == LVM_GREATER)
	/* if this descriptor area has a newer timestamp */

      {

	  defpvs_info -> newest_dats = inpvs_info ->
				       pv[pv_index].da[da_index].ts_beg;
	    /* set value of newest timestamp to the timestamp of this
	       descriptor area */

      }

  } /* beginning and ending timestamps match */

} /* loop for each VGDA on a PV */


/***********************************************************************
 *   Compare timestamps for primary and secondary VGDA copies on this  *
 *   PV, and record newest copy for this PV.                           *
 ***********************************************************************
 */

if (inpvs_info -> pv[pv_index].da[LVM_PRMRY].ts_status == LVM_TSRDERR)
  /* if either the beginning or ending timestamp has a read error on
     the primary VGDA copy */

{

    inpvs_info -> pv[pv_index].index_newestda = LVM_PRMRY;
      /* save VGDA copy with a read error as the one which has the newest
	 VGDA copy so that we will not later count this in currently good
	 VGDAs */

    inpvs_info -> pv[pv_index].index_nextda = LVM_PRMRY;
      /* save VGDA copy with a read error as the one which will next be
	 written to since we want to first correct unreadable copies */

}

else
  /* no read error on timestamps of primary VGDA */

{

    if (inpvs_info -> pv[pv_index].da[LVM_SCNDRY].ts_status ==
						     LVM_TSRDERR)
      /* if either the beginning or ending timestamp has a read error on
	 the secondary VGDA copy */

    {

	inpvs_info -> pv[pv_index].index_newestda = LVM_SCNDRY;
	  /* save VGDA copy with a read error as the one which has the
	     newest VGDA copy so that we will not later count this in
	     currently good VGDAs */

	inpvs_info -> pv[pv_index].index_nextda = LVM_SCNDRY;
	  /* save VGDA copy with a read error as the one which will next
	     be written to since we want to first correct unreadable
	     copies */

    }

    else
      /* no read error on timestamps of secondary or primary VGDA */

    {

	status = (short int) lvm_tscomp (
		  &(inpvs_info -> pv[pv_index].da[LVM_SCNDRY].ts_end),
		  &(inpvs_info -> pv[pv_index].da[LVM_PRMRY].ts_end));
	  /* call routine to do comparison of the end timestamps for the
	     primary and secondary copies of the VGDA on this PV */

	if (status == LVM_GREATER)
	  /* if the secondary VGDA has a newer timestamp than the primary
	     VGDA */

	{

	    inpvs_info -> pv[pv_index].index_newestda = LVM_SCNDRY;
	      /* save secondary VGDA as newest VGDA copy for this PV */

	    if (inpvs_info -> pv[pv_index].da[LVM_SCNDRY].ts_status
					      == LVM_BTSEQETS)
	      /* if the begin and end timestamps are equal for the
		 secondary VGDA copy */

	    {

		inpvs_info -> pv[pv_index].index_nextda = LVM_PRMRY;
		  /* save primary VGDA as the next to be written to since
		     we want to update the oldest */

	    }

	    else
	      /* begin and end timestamps not equal for secondary */

	    {

		inpvs_info -> pv[pv_index].index_nextda = LVM_SCNDRY;
		  /* save secondary VGDA as the next to be written to
		     since it has non-matching timestamps */

	    }

	} /* secondary VGDA has newer timestamp than primary VGDA */

	else
	  /* primary VGDA has equal or newer timestamp than secondary
	     VGDA */

	{

	    inpvs_info -> pv[pv_index].index_newestda = LVM_PRMRY;
	      /* save primary VGDA as newest VGDA copy for this PV */

	    if (inpvs_info -> pv[pv_index].da[LVM_PRMRY].ts_status
					      == LVM_BTSEQETS)
	      /* if the begin and end timestamps are equal for the
		 primary VGDA copy */

	    {

		inpvs_info -> pv[pv_index].index_nextda = LVM_SCNDRY;
		  /* save secondary VGDA as the next to be written to
		     since we want to update the oldest */

	    }

	    else
	      /* begin and end timestamps not equal for primary */

	    {

		inpvs_info -> pv[pv_index].index_nextda = LVM_PRMRY;
		  /* save primary VGDA as the next to be written to
		     since it has non-matching timestamps */

	    }

	} /* primary VGDA has timestamp >= secondary VGDA */

    } /* no read errors on timestamps of primary or secondary VGDA */

} /* no read error on timestamps of primary VGDA */

return;
  /* return to caller */

} /* END lvm_getdainfo */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_readpvs                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine opens the physical volumes in the volume group and   *
 *   obtains certain information from the reserved areas at the        *
 *   beginning of these disks.  Among this information is the          *
 *   location of any volume group descriptor areas and status areas.   *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     inpvs_info                                                      *
 *     varyonvg -> vvg_out                                             *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_BADVERSION                                                  *
 *     LVM_NOVGDAS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_readpvs (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

struct inpvs_info * inpvs_info)
  /* structure which contains information about the input list of PVs
     for the volume group */


{ /* BEGIN lvm_readpvs */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

int pv_fd;
  /* file descriptor for open of a physical volume */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

struct stat stat_buf;
  /* buffer to hold certain file status values, of which we are using
     the dev_t device identification */

IPL_REC ipl_rec;
  /* buffer into which to read the disk IPL record */

struct lvm_rec lvm_rec;
  /* buffer into which to read the disk LVM record */

char devname [LVM_EXTNAME];
  /* the full path name of the physical disk device */

short int prev_desclps;
  /* variable to hold previous number of logical partitions needed for
     LVM reserved area */

short int pv_index;
  /* index variable for looping on physical volumes */

short int da_index;
  /* index variable for looping on the VGDA copies on a PV */

int special_open_modes;
  /* the bit mask to be used if varyonvg is used in a disk takeover operation */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   For each PV in the input list, read the IPL record and the LVM    *
 *   record and determine if the PV indicates it is a member of the    *
 *   specified volume group.  Move information about the PV into the   *
 *   varyonvg output structure.                                        *
 ***********************************************************************
 */

for (pv_index = 0; pv_index < varyonvg -> vvg_in.num_pvs;
     pv_index = pv_index+1)
  /* loop for each physical volume name passed in by the caller */

{

  varyonvg -> vvg_out.pv[pv_index].pvname =
	varyonvg -> vvg_in.pv[pv_index].pvname;
    /* store pointer to PV name into the output structure of information
       to return to user */

  varyonvg -> vvg_out.pv[pv_index].pv_id =
	varyonvg -> vvg_in.pv[pv_index].pv_id;
    /* store the id of the PV into the output structure of information
       to return to the user */


  varyonvg -> vvg_out.pv[pv_index].pv_status = LVM_PVNOTFND;
    /* initialize physical volume status to indicate device not found */

  inpvs_info -> pv[pv_index].fd = LVM_FILENOTOPN;
    /* initialize the file descriptor to indicate the physical volume
       device is not open */

  retcode = status_chk (NULL, varyonvg -> vvg_in.pv[pv_index].pvname,
			NOCHECK, devname);
    /* build the path name for the physical volume device from the name
       passed in by the user */

  if (retcode < LVM_SUCCESS)
    /* if an error occurred */

  {

      continue;
	/* continue with next PV in input list */

  }

  /* we want to save the device number for this pv */
  retcode = stat(devname, &stat_buf);
  if (retcode < LVM_SUCCESS) return (LVM_PROGERR);
  inpvs_info -> pv[pv_index].device = stat_buf.st_rdev;

  special_open_modes = 0;

  if (varyonvg->break_res == TRUE)
	special_open_modes |= LVM_FORCED_OPEN;

  if (varyonvg->unreserved_after_open == TRUE)
	special_open_modes |= LVM_OPEN_NO_RSRV;

/*
 * if a special open, this code path will ensure all drives that make up
 * the volume group are opened the same way
 */
  if (special_open_modes)
	pv_fd = openx(devname, O_RDWR, 0, special_open_modes);
  else
	pv_fd = open(devname, O_RDWR);

  if (pv_fd == LVM_UNSUCC)
    /* if an error occurred */

  {

      continue;
	/* continue with next PV in input list */

  }

  retcode = lvm_rdiplrec (varyonvg->vvg_in.pv[pv_index].pvname, pv_fd, &ipl_rec);
    /* read the IPL record for the physical volume */

  if (retcode < LVM_SUCCESS)
    /* if an error occurred */

  {

      close (pv_fd);
	/* close the physical volume */

      continue;
	/* continue with next PV in input list */

  }

  inpvs_info -> pv[pv_index].pv_id = ipl_rec.pv_id;
    /* store the PV id found in the IPL record in the PV information
       structure for this PV */

  varyonvg -> vvg_out.pv[pv_index].pv_id = ipl_rec.pv_id;
    /* store the PV id found in the IPL record in the output structure
       of information to return to the user */

  retcode = lvm_rdlvmrec (pv_fd, &lvm_rec);
    /* call routine to read the LVM record from the physical volume */

  if (retcode < LVM_SUCCESS)
    /* if an error occurred */

  {

      close (pv_fd);
	/* close the physical volume */

      continue;
	/* continue with next PV in input list */

  }

  if (lvm_rec.lvm_id != LVM_LVMID)
    /* if this physical volume is not a member of a volume group */

  {

      varyonvg -> vvg_out.pv[pv_index].pv_status = LVM_PVNOTINVG;
	/* set physical volume status to show that LVM record indicates
	   this PV is not a member of the specified volume group */

      close (pv_fd);
	/* close the physical volume */

      continue;
	/* continue with next PV in input list */

  }

  if (lvm_rec.vg_id.word1 != varyonvg -> vg_id.word1  ||
      lvm_rec.vg_id.word2 != varyonvg -> vg_id.word2)
    /* if this physical volume is not a member of the specified volume
       group */

  {

      varyonvg -> vvg_out.pv[pv_index].pv_status = LVM_PVNOTINVG;
	/* set physical volume status to show that LVM record indicates
	   this PV is not a member of the specified volume group */

      close (pv_fd);
	/* close the physical volume */

      continue;
	/* continue with next PV in input list */

  }

  varyonvg -> vvg_out.pv[pv_index].pv_status = LVM_PVINVG;
    /* set physical volume status to show the LVM record indicates the
       PV is a member of the specified volume group */


  /**********************************************************************
   *   Before continuing, check to be sure that this volume group has   *
   *   the correct version format.  For volume groups created with old  *
   *   format (build levels previous to 9013), return a bad version     *
   *   number error.                                                    *
   **********************************************************************
   */

  if (lvm_rec.version != LVM_VERSION_1 &&
     (lvm_rec.version == 0 || lvm_rec.version > LVM_MAX_VERSION))
    /* if the volume group descriptor/status areas on this disk have the
       incompatible format */

  {

      return (LVM_BADVERSION);
	/* return error to indicate this volume group has an incompatible
	   format for its VGDA/VGSA */

  }


  /**********************************************************************
   *   LVM record indicates the PV is a member of the specified volume  *
   *   group, so save information from the LVM record.                  *
   **********************************************************************
   */

  inpvs_info -> lvmarea_len = lvm_rec.lvmarea_len;
    /* save the length of the LVM reserved area for this volume group */

  inpvs_info -> vgda_len = lvm_rec.vgda_len;
    /* save the length of the volume group descriptor area */

  inpvs_info -> vgsa_len = lvm_rec.vgsa_len;
    /* save the length of the volume group status area */

  inpvs_info -> pp_size = lvm_rec.pp_size;
    /* save the physical partition size for this volume group */

  inpvs_info -> pv[pv_index].pv_num = lvm_rec.pv_num;
    /* save the PV number of this physical volume */

  for (da_index = 0; da_index < LVM_PVMAXVGDAS; da_index = da_index + 1)
    /* loop for number of VGDA / VGSA copies on one PV */

  {

    inpvs_info -> pv[pv_index].da_psn [da_index] =
			       lvm_rec.vgda_psn [da_index];
      /* save the physical sector number (PSN) of the beginning of the
	 volume group descriptor area copy */

    inpvs_info -> pv[pv_index].sa_psn [da_index] =
			       lvm_rec.vgsa_psn [da_index];
      /* save the PSN of the beginning of the volume group status area */

  }

  inpvs_info -> pv[pv_index].reloc_psn = lvm_rec.reloc_psn;
    /* save the PSN of the beginning of the bad block relocation pool */

  inpvs_info -> pv[pv_index].reloc_len = lvm_rec.reloc_len;
    /* save the length in blocks of the bad block relocation pool */

  partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
    /* find the partition length for this volume group in number of
       blocks */

  prev_desclps = inpvs_info -> num_desclps;
    /* save the previous number of partitions needed to contain the
       LVM reserved area */

  inpvs_info -> num_desclps = (short int) ((lvm_rec.lvmarea_len +
       lvm_rec.vgsa_psn [LVM_PRMRY]) / partlen_blks + 1);
    /* calculate the number of partitions needed to contain the LVM
       reserved area for this PV */

  if (prev_desclps > inpvs_info -> num_desclps)
    /* if the previous number of partitions needed is greater than the
       number needed for this PV's LVM reserved area */

  {

      inpvs_info -> num_desclps = prev_desclps;
	/* set the number of partitions needed back to the previous
	   number */

  }

  retcode = fstat (pv_fd, &stat_buf);
    /* call system routine to get certain file statistics about this
       device in order to get the device identification for this PV */

  if (retcode < LVM_SUCCESS)
    /* if an error occurred */

  {

      close (pv_fd);
	/* close the physical volume */

      return (LVM_PROGERR);
	/* return with error code for programming error */

  }

  inpvs_info -> pv[pv_index].fd = pv_fd;
    /* save the file descriptor from the open of this physical volume */

  inpvs_info -> pv[pv_index].pv_status = LVM_VALIDPV;
    /* set value for PV status flag to indicate that this PV is a valid
       member of the volume group (as determined from LVM record) */

} /* loop for each input PV */

varyonvg -> vvg_out.num_pvs = varyonvg -> vvg_in.num_pvs;
  /* set value for number of PVs in the varyonvg output structure */


/************************************************************************
 *   Return an error code for no good volume group descriptor areas if  *
 *   we were unable to get any information which would enable us to     *
 *   read a VGDA copy for the volume group.                             *
 ************************************************************************
 */

if (inpvs_info -> num_desclps == 0)
  /* if the number of partitions needed to contain the LVM reserved area
     is 0, then there were no PVs in the input list with a good LVM
     record (i.e., the LVM record could be read and indicated the PV is a
     member of the specified volume group) */

{

    return (LVM_NOVGDAS);
      /* return error code for no VGDAs since we could not find one
	 PV in the input list for which we can attempt to read the
	 VGDA/VGSA */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_readpvs */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_readvgda                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to read the newest copy of the volume       *
 *   group descriptor area.                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_fd                                                           *
 *     override                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *vgda_ptr                                                       *
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
lvm_readvgda (

int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */

short int override,
  /* flag which indicates if no quorum error is to be overridden */

struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */

struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

caddr_t * vgda_ptr)
  /* pointer to buffer in which to read the volume group descriptor
     area */


{ /* BEGIN lvm_readvgda */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct timestruc_t cur_ts;
  /* structure in which to save timestamp of current VGDA copy */

struct timestruc_t prev_ts;
  /* structure in which to save timestamp of previous VGDA copy */

long partlen_blks;
  /* the length of a physical partition in number of 512 byte blocks */

off_t offset;
  /* offset where the file pointer is to be positioned */

daddr_t dalsn;
  /* the logical sector number within the volume group reserved area
     logical volume of where a VGDA copy is located */

int retcode;
  /* return code */

int status;
  /* variable to hold the status of a comparison between two timestamp
     values */

short int good_vgda;
  /* flag to indicate if we have gotten a good read of a VGDA copy
     with newest timestamp */

short int pv_num;
  /* index variable for looping on possible physical volumes */

short int da_index;
  /* index variable for looping on VGDA copies on a PV */

short int in_index;
  /* the index into the user's input list */

short int cur_in;
  /* index into input list for PV with current VGDA copy */

short int cur_da;
  /* index for the copy number on the PV for the current VGDA copy */

short int prev_in;
  /* index into input list for PV with previous VGDA copy */

short int prev_da;
  /* index for the copy number on the PV for the previous VGDA copy */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Allocate space for the volume group descriptor area.              *
 ***********************************************************************
 */

*vgda_ptr = (caddr_t) malloc (inpvs_info -> vgda_len * DBSIZE);
  /* allocate space for the volume group descriptor area */

if (*vgda_ptr == NULL)
  /* if error occurred */

{

    return (LVM_ALLOCERR);
      /* return error code for allocation error */

}

good_vgda = FALSE;
  /* set flag to indicate that we have not had a good read of a VGDA
     copy with the newest timestamp */

partlen_blks = LVM_PPLENBLKS (inpvs_info -> pp_size);
  /* calculate the length of a partition in number of 512 byte blocks */


/***********************************************************************
 *   Search through the list of active PVs, trying to get a good       *
 *   read of a VGDA copy with the newest timestamp.                    *
 ***********************************************************************
 */

for (pv_num = 1; pv_num <= LVM_MAXPVS; pv_num = pv_num + 1)
  /* loop for all possible PVs until we have a good read of a VGDA copy
     which has the newest timestamp */

{

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if the PV status is defined in the defpvs_info structure for the
       PV with this number */

  {

      for (da_index = 0; da_index < LVM_PVMAXVGDAS; da_index = da_index+1)
	/* loop for each possible VGDA copy on a PV until good read of
	   VGDA copy with newest timestamp */

      {

	in_index = defpvs_info -> pv[pv_num - 1].in_index;
	  /* save the index into the input list for the PV which is
	     defined with this PV number */

	if (inpvs_info -> pv[in_index].da[da_index].ts_status ==
						      LVM_BTSEQETS)
	  /* if the timestamp status for this PV indicates the beginning
	     timestamp equals the ending timestamp */

	{

	    status = lvm_tscomp (&(defpvs_info -> newest_dats),
		      &(inpvs_info -> pv[in_index].da[da_index].ts_beg));
	      /* call routine to compare timestamp for this VGDA copy
		 with timestamp of newest VGDA */


	    if (status == LVM_EQUAL)
	      /* if timestamp for this VGDA copy equals newest VGDA
		  timestamp */

	    {

		dalsn = partlen_blks * inpvs_info -> num_desclps *
			  (inpvs_info -> pv[in_index].pv_num - 1) +
			  inpvs_info -> pv[in_index].da_psn[da_index];
		  /* calculate logical sector number within the volume
		     group reserved area logical volume of where this
		     VGDA copy is located */

		offset = DBSIZE * dalsn;
		  /* calculate offset in bytes from beginning of logical
		     volume to this VGDA copy */

		offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
		  /* position file pointer at beginning of VGDA copy */

		if (offset == LVM_UNSUCC)
		  /* if an error occurred */

		{

		    continue;
		      /* continue with next VGDA copy */

		}


		/********************************************************
		 *   Read this VGDA copy since its beginning and ending *
		 *   timestamps are equal to each other and to the      *
		 *   newest VGDA timestamp.  If we do not get a good    *
		 *   read, then set flag to indicate this VGDA copy     *
		 *   needs to be rewritten.                             *
		 ********************************************************
		 */

		cur_in = in_index;
		  /* save input index for this PV */

		cur_da = da_index;
		  /* save index for this VGDA copy on this PV */

		retcode = read (vg_fd, *vgda_ptr,
				inpvs_info -> vgda_len * DBSIZE);
		  /* read in this copy of the volume group descriptor
		     area from the PV */

		if (retcode == inpvs_info -> vgda_len * DBSIZE)
		  /* if read of this VGDA copy is good */

		{

		    prev_in = cur_in;
		      /* save input index for PV of the previously read
			 VGDA */

		    prev_da = cur_da;
		      /* save VGDA copy index for the previously read
			 VGDA */

		    good_vgda = TRUE;
		      /* set flag to indicate good read of a VGDA copy
			 with newest timestamp */

		}

		else
		  /* read of this VGDA copy not good */

		{

		    inpvs_info -> pv[in_index].da[da_index].wrt_status
							    = TRUE;
		      /* set flag to indicate that this copy of the VGDA
			 needs to be rewritten */

		    inpvs_info -> pv[in_index].index_nextda = da_index;
		      /* set index value to indicate that this is the VGDA
			 copy to be the next written for this PV */

		}

	    } /* timestamp for this VGDA copy equals newest */

	} /* begin and end timestamps are equal for this VGDA copy */

      } /* loop for each VGDA copy on a PV */

  } /* this PV number is defined into the kernel */

} /* loop for each possible PV */


if (good_vgda == TRUE)
  /* if we were able to successfully read a VGDA copy which has the
     newest timestamp */

{

    if (cur_in != prev_in  ||  cur_da != prev_da)
      /* if the last read VGDA copy was not a good read, then try to read
	 VGDA copy with previously good read */

    {

	good_vgda = FALSE;
	  /* set flag to indicate that we do not have the latest VGDA
	     copy */

	dalsn = partlen_blks * inpvs_info -> num_desclps *
		  (inpvs_info -> pv[prev_in].pv_num - 1) +
		  inpvs_info -> pv[prev_in].da_psn[prev_da];
	  /* calculate logical sector number within the volume group
	     reserved area logical volume of where the previously read
	     VGDA copy is located */

	offset = DBSIZE * dalsn;
	  /* calculate offset in bytes from beginning of logical volume
	     to previous VGDA copy */

	offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
	  /* position file pointer at beginning of VGDA copy */

	if (offset != LVM_UNSUCC)
	  /* if no error */

	{

	    retcode = read (vg_fd, *vgda_ptr,
			    inpvs_info -> vgda_len * DBSIZE);
	      /* read in the previous VGDA copy again */

	    if (retcode == inpvs_info -> vgda_len * DBSIZE)
	      /* if this read is good */

	    {

		good_vgda = TRUE;
		  /* set flag to indicate that the latest VGDA was read
		     successfully */

	    }

	}

    } /* current VGDA copy not good, must reread previous */

} /* a VGDA copy with newest timestamp successfully read */


/***********************************************************************
 *   If we could not read one of the VGDA copies with the newest       *
 *   timestamp, then we have the error for no quorum since we know     *
 *   we could not successfully obtain the latest copy of the VGDA.     *
 ***********************************************************************
 */

if (good_vgda == FALSE)
  /* if we did not have successful read of a VGDA copy with newest
     timestamp */

{

    if (override == TRUE)
      /* if quorum count is to be overridden */

    {

	/****************************************************************
	 *   If the user has asked to override the no quorum error and  *
	 *   continue with the latest available copy of the VGDA, then  *
	 *   search through the PVs, trying to get the newest copy of   *
	 *   the VGDA which it is possible to read.                     *
	 ****************************************************************
	 */

	prev_ts.tv_sec = 0;
	prev_ts.tv_nsec = 0;
	  /* initialize to 0 the variable where we will save timestamp
	     value of the previously read VGDA copy */

	for (pv_num = 1; pv_num <= LVM_MAXPVS; pv_num = pv_num + 1)
	  /* loop for all possible PVs, searching for newest VGDA copy
	     which can be read */

	{

	  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
	    /* if the PV status is defined in the defpvs_info structure
	       for the PV with this number */

	  {

	      for (da_index = 0; da_index < LVM_PVMAXVGDAS;
		   da_index = da_index + 1)
		/* loop for each possible VGDA copy on a PV */

	      {

		in_index = defpvs_info -> pv[pv_num - 1].in_index;
		  /* save the index into the input list for the PV which
		     is defined with this PV number */

		if (inpvs_info -> pv[in_index].da[da_index].ts_status ==
							    LVM_BTSEQETS)
		  /* if the timestamp status for this PV indicates the
		     beginning timestamp equals the ending timestamp */

		{

		    status = lvm_tscomp (&prev_ts,
		      &(inpvs_info -> pv[in_index].da[da_index].ts_beg));
		      /* call routine to compare timestamp for the
			 previously read VGDA copy with this VGDA copy */

		    if (status != LVM_GREATER)
		      /* if timestamp for the previously read VGDA is
			 less than or equal to this VGDA's timestamp */

		    {

			dalsn = partlen_blks * inpvs_info -> num_desclps
			   * (inpvs_info -> pv[in_index].pv_num - 1) +
			   inpvs_info -> pv[in_index].da_psn[da_index];
			  /* calculate logical sector number within the
			     volume group reserved area logical volume of
			     where this VGDA copy is located */

			offset = DBSIZE * dalsn;
			  /* calculate offset in bytes from beginning of
			     logical volume to this VGDA copy */

			offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
			  /* position file pointer at beginning of VGDA
			     copy */

			if (offset == LVM_UNSUCC)
			  /* if an error occurred */

			{

			    continue;
			      /* continue with next VGDA copy */

			}


			/************************************************
			 *   Read this VGDA copy since its beginning    *
			 *   and ending timestamps are equal to each    *
			 *   other and greater than the timestamp for   *
			 *   the last VGDA copy read.  Save information *
			 *   about this VGDA copy for current VGDA      *
			 *   copy.  If read is successful, it will also *
			 *   become the previously read VGDA copy.      *
			 ************************************************
			 */

			cur_ts = inpvs_info ->
				   pv[in_index].da[da_index].ts_beg;
			  /* set current VGDA timestamp to value for this
			     VGDA copy */

			cur_in = in_index;
			  /* save input index for this PV */

			cur_da = da_index;
			  /* save index for this VGDA copy on the PV */

			retcode = read (vg_fd, *vgda_ptr,
					inpvs_info -> vgda_len * DBSIZE);
			  /* read in this copy of the VGDA */

			if (retcode == inpvs_info -> vgda_len * DBSIZE)
			  /* if read of this VGDA copy is good */

			{

			    prev_ts = cur_ts;
			      /* save this VGDA timestamp for previously
				 read VGDA */

			    prev_in = cur_in;
			      /* save input index for this PV */

			    prev_da = cur_da;
			      /* save index for this VGDA copy on PV */

			}

		    } /* this VGDA timestamp > previous VGDA */

		} /* begin, end timestamps equal for this VGDA */

	      } /* loop for each VGDA copy on a PV */

	  } /* this PV number is defined into the kernel */

	} /* loop for each possible PV */


	/****************************************************************
	 *   Check to see if any VGDA copies were read successfully and *
	 *   return an error if not.  If current and previous VGDAs are *
	 *   not the same, this means there was not a good read on the  *
	 *   current VGDA copy, and we must reread the previous VGDA.   *
	 *   If necessary to reread previous VGDA copy, and it is not   *
	 *   good, then return an error.                                *
	 ****************************************************************
	 */

	if (prev_ts.tv_sec == 0  &&  prev_ts.tv_nsec == 0)
	  /* if the timestamp value for the previously read VGDA copy
	     is 0, this indicates we did not successfully read any VGDA
	     copy */

	{

	    return (LVM_NOVGDAS);
	      /* return error for no VGDAs since we could not read any
		 VGDA copies */

	}

	if (cur_ts.tv_sec != prev_ts.tv_sec  ||
	    cur_ts.tv_nsec != prev_ts.tv_nsec)
	  /* if the previous VGDA timestamp is not equal to the current
	     VGDA timestamp, this means we did not have a good read on
	     last VGDA read and must reread previous */

	{

	    dalsn = partlen_blks * inpvs_info -> num_desclps *
		      (inpvs_info -> pv[prev_in].pv_num - 1) +
		      inpvs_info -> pv[prev_in].da_psn[prev_da];
	      /* calculate logical sector number within the volume group
		 reserved area logical volume of where the previously
		 read VGDA copy is located */

	    offset = DBSIZE * dalsn;
	      /* calculate offset in bytes from beginning of logical
		 volume to previous VGDA copy */

	    offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
	      /* position file pointer at beginning of VGDA copy */

	    if (offset == LVM_UNSUCC)
	      /* if an error occurred */

	    {

		return (LVM_NOVGDAS);
		  /* return with error for no VGDAs since we could not
		     successfully reread previous */

	    }

	    retcode = read (vg_fd, *vgda_ptr,
			    inpvs_info -> vgda_len * DBSIZE);
	      /* read in the previous VGDA copy again */

	    if (retcode != inpvs_info -> vgda_len * DBSIZE)
	      /* if this read is not good */

	    {

		return (LVM_NOVGDAS);
		  /* return with error for no VGDAs since we could not
		     successfully reread previous */

	    }

	} /* current VGDA copy not good, must reread previous */

    } /* user asked to override no quorum error */

    return (LVM_NOQUORUM);
      /* return with error for no quorum */

} /* could not read VGDA with newest timestamp */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_readvgda */

