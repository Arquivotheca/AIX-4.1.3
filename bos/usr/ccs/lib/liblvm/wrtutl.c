static char sccsid[] = "@(#)41	1.5  src/bos/usr/ccs/lib/liblvm/wrtutl.c, liblvm, bos411, 9428A410j 5/25/94 08:13:02";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - wrtutl.c
 *
 * FUNCTIONS: lvm_diskio,
 *            lvm_updvgda,
 *            lvm_wrtdasa,
 *            lvm_wrtnext
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

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/time.h>
#include <sys/bbdir.h>
#include <sys/buf.h>
#include <sys/shm.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_config.h>
#include <liblvm.h>
#include <lvmrec.h>
#include <sys/hd_psn.h>









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_diskio                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine gets information from the mapped file header and     *
 *   calls the lvm_updvgda routine to update the on-disk copies of     *
 *   the volume group descriptor area for a volume group.              *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_mapptr                                                       *
 *     vg_mapfd                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_DALVOPN                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_diskio (

caddr_t vg_mapptr,
  /* a pointer to the mapped file for this volume group */

int vg_mapfd)
  /* the file descriptor of the mapped file for this volume group */


{ /* BEGIN lvm_diskio */


int retcode;
  /* the return code */

caddr_t vgda_ptr;
  /* a pointer to the memory location which holds the volume group
     descriptor area */

struct fheader * maphdr_ptr;
  /* a pointer to the file header portion of the mapped file */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group header of the volume group descriptor
     area */

struct vg_trailer * vgtrail_ptr;
  /* a pointer to the volume group trailer of the volume group descriptor
     area */

char rsrvlv [LVM_EXTNAME];
  /* the device name for the LVM reserved area logical volume */

long vgda_length;
  /* the length of the volume group descriptor area in bytes */

int lv_fd;
  /* the file descriptor of the descriptor area logical volume */

struct pv_header * pvhdr_ptr;
  /* a pointer to the physical volume header in the descriptor area for
     a PV */

short int pvnum;
  /* PV number of the physical volume */

short int pv_index;
  /* index variable for looping on physical volumes */

struct vgsa_area vgsa; 
  /* buffer to hold VGSA information */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

maphdr_ptr = (struct fheader *) vg_mapptr;
  /* set a pointer to the file header portion of the mapped file */

vgda_ptr = vg_mapptr + sizeof (struct fheader);
  /* set a pointer to the volume group descriptor area within the mapped
     file */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

vgtrail_ptr = (struct vg_trailer *) ((caddr_t) vgda_ptr +
		 vghdr_ptr -> vgda_size * DBSIZE - DBSIZE);
  /* set a pointer to the trailer of the volume group descriptor area,
     which begins one block from the end of the descriptor area */

vgda_length = vghdr_ptr -> vgda_size * DBSIZE;
  /* convert the descriptor area size stored in the header to the length
     in bytes */

retcode = lvm_updtime (&(vghdr_ptr -> vg_timestamp),
		       &(vgtrail_ptr -> timestamp));
  /* call routine to update the beginning and ending VGDA timestamps */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return error code for programming error */

}

retcode = status_chk (NULL, maphdr_ptr -> vgname, NOCHECK, rsrvlv);
  /* check and build the device name of the LVM reserved area logical
     volume from the volume group name in the mapped file */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    return (retcode);
      /* return with error return code */

}

lv_fd = open (rsrvlv, O_RDWR);
  /* open the LVM reserved area logical volume for this volume group */

if (lv_fd == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_DALVOPN);
      /* return error code for LVM reserved area logical volume open
	 error */

}

retcode = getstates (&vgsa, vg_mapptr);
  /* get a copy of the volume group status area for use in checking for
     missing physical volumes */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}

for (pv_index = 0; pv_index < LVM_MAXPVS; pv_index = pv_index + 1)
  /* loop for the possible number of physical volumes */

{

  pvnum = pv_index + 1;
    /* set value of PV number for this PV */

  if (!(TSTSA_PVMISS (&vgsa, pv_index)))
    /* if this physical volume is not missing */

  {

      pvhdr_ptr = (struct pv_header *) ((caddr_t) maphdr_ptr +
		    maphdr_ptr -> pvinfo [pv_index].pvinx);
	/* calculate pointer to the physical volume header for this PV */

      retcode = lvm_wrtnext (lv_fd, vgda_ptr, &(vgtrail_ptr->timestamp),
		   pvnum, maphdr_ptr, pvhdr_ptr -> pvnum_vgdas);
	/* call routine to write a copy of the volume group descriptor
	   area at the LSN for next copy to be written to this PV */

      if (retcode < LVM_SUCCESS)
	/* if an error occurred */

      {

	  return (retcode);
	    /* return with error return code */

      }

  } /* this PV is not missing */

} /* loop for each possible PV */

close (lv_fd);
  /* close the LVM reserved area logical volume */

return (LVM_SUCCESS);

} /* END lvm_diskio */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_updvgda                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes out the updated in-memory copy of the volume  *
 *   group descriptor area to the specified on-disk copies for the     *
 *   physical volumes within the volume group.                         *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     lv_fd                                                           *
 *     maphdr_ptr                                                      *
 *     vgda_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The in-memory copy of the volume group descriptor area is       *
 *     written to the physical disks within the volume group.          *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_updvgda (

int lv_fd,
  /* the file descriptor of the descriptor area logical volume, if it
     is already open */

struct fheader * maphdr_ptr,
  /* a pointer to the file header portion of the mapped file */

caddr_t vgda_ptr)
  /* a pointer to the memory location which holds the volume group
     descriptor area */


{ /* BEGIN lvm_updvgda */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct pv_header * pvhdr_ptr;
  /* a pointer to the physical volume header in the descriptor area for
     a PV */

int retcode;
  /* return code value */

short int pvnum;
  /* PV number of the physical volume */

short int pv_index;
  /* index variable for looping on physical volumes */

struct vg_trailer *trail;
   /* pointer to the volume group trailer */

struct vgsa_area vgsa;
  /* buffer to hold VGSA information */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

trail = (struct vg_trailer *) (maphdr_ptr -> trailinx +
			       (caddr_t) maphdr_ptr);
  /* get pointer to volume group trailer record */

retcode = getstates (&vgsa, (caddr_t) maphdr_ptr);
  /* get a copy of the volume group status area for use in checking for
     missing physical volumes */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return(retcode);
      /* return with returned error code */

}

for (pv_index = 0; pv_index < LVM_MAXPVS; pv_index = pv_index + 1)
  /* loop for the possible number of physical volumes */

{

  pvnum = pv_index + 1;
    /* set value of PV number for this PV */

  if (!(TSTSA_PVMISS (&vgsa, pv_index)))
    /* if this physical volume is not missing */

  {

      pvhdr_ptr = (struct pv_header *) ((caddr_t) maphdr_ptr +
		    maphdr_ptr -> pvinfo [pv_index].pvinx);
	/* calculate pointer to the physical volume header for this PV */

      retcode = lvm_wrtnext (lv_fd, vgda_ptr, &(trail->timestamp), pvnum, 
				maphdr_ptr, pvhdr_ptr -> pvnum_vgdas);
	/* call routine to write the volume group descriptor area to
	   this PV */
         
      if (retcode < LVM_SUCCESS)
	/* if an error occurred */

      {

	  return (retcode);
	    /* return with error (this should normally be forced
	       varyoff) */

      }

  } /* this PV is currently active */

} /* loop for each possible PV */

return (LVM_SUCCESS);
  /* return with successful return */

} /* END lvm_updvgda */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_wrtdasa                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes out a copy of the either the volume group     *
 *   descriptor area or volume group status area at the specified      *
 *   logical sector within the LVM reserved area logical volume.       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_fd                                                           *
 *     area_ptr                                                        *
 *     e_timestamp                                                     *
 *     area_len                                                        *
 *     lsn                                                             *
 *     write_order                                                     *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The in-memory copy of the volume group status descriptor area   *
 *     or status area is written at the specified logical sector.      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_UNSUCC                                                      *
 *     LVM_PROGERR                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_wrtdasa (

int vg_fd,
  /* the file descriptor of the LVM reserved area logical volume */

caddr_t area_ptr,
  /* a pointer to the memory location which holds the volume group
     descriptor or status area */

struct timestruc_t * e_timestamp,
  /* a pointer to the end timestamp for the descriptor or status area */

long area_len,
  /* length in sectors of the volume group descriptor or status area */

daddr_t lsn,
  /* the logical sector number within the LVM reserved area logical
     volume of where to write a copy of the descriptor or status area */

short int write_order)
  /* flag which indicates whether the area is to be written in the order
     of beginning/middle/end or middle/beginning/end */


{ /* BEGIN lvm_wrtdasa */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* offset at which to position file pointer */

caddr_t fstblk_ptr;
  /* pointer to the first record of the VGDA or VGSA */

caddr_t lstblk_ptr;
  /* pointer to the last record of the VGDA or VGSA */

caddr_t body_ptr;
  /* pointer to the body of the volume group descriptor or status area */

long body_len;
  /* the length in blocks of the body of the VGDA or VGSA (i.e., all
     blocks except the first and last) */

struct timestruc_t save_time;
  /* variable in which to save the end timestamp of the descriptor area
     or status area which is being written */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


fstblk_ptr = area_ptr;
  /* set pointer to the first record in the area */

body_ptr = area_ptr + DBSIZE;
  /* set pointer to the body of the area */

body_len =  DBSIZE * (area_len - 2);
  /* subtract blocks for the first and last records from the size in
     blocks of the area in order to get the length of the body */

lstblk_ptr = area_ptr + area_len * DBSIZE - DBSIZE;
  /* set pointer to the last record in the area */

if (write_order == LVM_ZEROETS)
  /* if write order indicates that end timestamp needs to be zeroed out
     before continuing with write */

{

    save_time = *e_timestamp;
      /* save end timestamp of the area being written */

    e_timestamp -> tv_sec = 0;
    e_timestamp -> tv_nsec = 0;
      /* set end timestamp of the area to 0 */

    offset = DBSIZE * (lsn + area_len - 1);
      /* get the byte offset from the beginning of the LVM reserved area
	 logical volume of the last record of the VGDA or VGSA */

    offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
      /* call system routine to position the file pointer at the start of
	 last record */

    if (offset == LVM_UNSUCC)
      /* if an error occurred */

    {

	return (LVM_PROGERR);
	  /* return with error code for programming error */

    }

    retcode = write (vg_fd, lstblk_ptr, DBSIZE);
      /* write out the block containing the last record of the VGSA */

    if (retcode != DBSIZE)
      /* if an error occurred on write of this status area */

    {

	return (LVM_UNSUCC);
	  /* return with error;  write of this VGSA cannot be counted for
	     quorum */

    }

    *e_timestamp = save_time;
      /* set the end timestamp back to the saved timestamp value */

    write_order = LVM_BEGMIDEND;
      /* set write order to beginning/middle/end to proceed with writing
	 of the area */

}

if (write_order == LVM_BEGMIDEND)
  /* if flag for write order indicates beginning/middle/end (meaning to
     write the area in the order of first record, body, last record) */

{

    offset = lsn * DBSIZE;
      /* get the byte offset from the beginning of the LVM reserved area
	 logical volume of the first block of this area copy */

    offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
      /* call system routine to position the file pointer at the start
	 of the first block */

    if (offset == LVM_UNSUCC)
      /* if an error occurred */

    {

	return (LVM_PROGERR);
	  /* return with error code for programming error */

    }

    retcode = write (vg_fd, fstblk_ptr, DBSIZE);
      /* write out the block containing the first record of the area */

    if (retcode != DBSIZE)
      /* if an error occurred on write */

    {

	return (LVM_UNSUCC);
	  /* return with error;  this area could not be successfully
	     written */

    }

} /* write order of beginning/middle/end */

offset = DBSIZE * (lsn + 1);
  /* get the byte offset from the beginning of the LVM reserved area
     logical volume of the second block of the area (i.e., the body) */

offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
  /* call system routine to position the file pointer at the start of
     the body of the area */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

retcode = write (vg_fd, body_ptr, body_len);
  /* write out the body of the area (i.e., all blocks except the first
     last blocks, which contain the timestamps) */

if (retcode != body_len)
  /* if an error occurred on write */

{

    return (LVM_UNSUCC);
      /* return with error;  this area could not be successfully
	 written */

}

if (write_order == LVM_MIDBEGEND)
  /* if flag for write order indicates middle/beginning/end (meaning to
     write area in the order of body, first record, last record) */

{

    offset = lsn * DBSIZE;
      /* get the byte offset from the beginning of the LVM reserved area
	 logical volume of the first block of the area */

    offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
      /* call system routine to position the file pointer at the start
	 of the first block of the area */

    if (offset == LVM_UNSUCC)
      /* if an error occurred */

    {

	return (LVM_PROGERR);
	  /* return with error code for programming error */

    }

    retcode = write (vg_fd, fstblk_ptr, DBSIZE);
      /* write out the block containing the first record of the area */

    if (retcode != DBSIZE)
      /* if an error occurred on write */

    {

	return (LVM_UNSUCC);
	  /* return with error;  this area could not be successfully
	     written */

    }

} /* write order of middle/beginning/end */

offset = DBSIZE * (lsn + area_len - 1);
  /* get the byte offset from the beginning of the LVM reserved area
     logical volume of the last block of the area */

offset = lseek (vg_fd, offset, LVM_FROMBEGIN);
  /* call system routine to position the file pointer at the start of the
     last record of the area */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

retcode = write (vg_fd, lstblk_ptr, DBSIZE);
  /* write out the block containing the last record of the area */

if (retcode != DBSIZE)
  /* if an error occurred on write */

{

    return (LVM_UNSUCC);
      /* return with error;  this area could not be successfully
	 written */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_wrtdasa */


/***********************************************************************
 *                                                                     *
 * NAME:  lvm_wrtnext                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes out a copy of the volume group descriptor     *
 *   area to the VGDA copy which has the oldest timestamp for a PV.    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     lv_fd                                                           *
 *     vgda_ptr                                                        *
 *     pvnum                                                           *
 *     maphdr_ptr                                                      *
 *     pvnum_vgdas                                                     *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The in-memory copy of the volume group descriptor area is       *
 *     written to the specified PV for the specified number of copies. *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_wrtnext (

int lv_fd,
  /* the file descriptor of the LVM reserved area logical volume */

caddr_t vgda_ptr,
  /* a pointer to the memory location which holds the volume group
     descriptor area */

struct timestruc_t *etimestamp,
  /* end timestamp from vg trailer */

short int pvnum,
  /* the PV number of the PV to which the VGDA will be written */

struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */

short int pvnum_vgdas)
  /* the number of descriptor area copies to be written to this PV */


{ /* BEGIN lvm_wrtnext */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct vg_header * vghdr_ptr;
  /* pointer to the volume group header */

int ts_status;
  /* time stamp status to indicate if one timestamp is greater than,
     equal to, or less than another */

short int da_index;
  /* index for a descriptor area copy on a PV */

short int das;
  /* index for looping on number of descriptor areas to write to this
     PV */

short int quorum;
   /* quorum count for the volume group */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set pointer to volume group header */

ts_status = lvm_tscomp (&(maphdr_ptr->pvinfo[pvnum-1].da[LVM_PRMRY].ts),
		  &(maphdr_ptr->pvinfo[pvnum-1].da[LVM_SCNDRY].ts));
  /* compare timestamps from primary and secondary descriptor areas on
     this PV */

if (ts_status == LVM_GREATER && pvnum_vgdas > 1)
  /* if primary timestamp is greater than secondary timestamp */
  /* and there is a secondary timestamp */

{

    da_index = LVM_SCNDRY;
      /* set index to secondary for copy of descriptor area which is to
	 be written to */

}

else
  /* primary timestamp less than or equal to secondary timestamp */

{

    da_index = LVM_PRMRY;
      /* set index to primary for copy of descriptor area which is to
	 be written to */

}

for (das = 0; das < pvnum_vgdas; das = das + 1)
  /* loop for number of descriptor areas to be written to this PV */

{

  retcode = lvm_wrtdasa (lv_fd, vgda_ptr, etimestamp,
		(long) vghdr_ptr -> vgda_size,
		maphdr_ptr -> pvinfo[pvnum-1].da [da_index].dalsn, 
		(short int) LVM_BEGMIDEND);
    /* write a copy of the volume group descriptor area to this PV */

  if (retcode == LVM_SUCCESS)
    /* if the write was successful */

  {

      maphdr_ptr->pvinfo[pvnum-1].da[da_index].ts = vghdr_ptr->vg_timestamp;
	/* update timestamp value in array with timestamp of the VGDA
	   just written */

  }

  else

  {

      maphdr_ptr -> pvinfo[pvnum-1].da [da_index].ts.tv_sec = 0;
      maphdr_ptr -> pvinfo[pvnum-1].da [da_index].ts.tv_nsec = 0;
	/* update timestamp value in array to 0 so that this will be
	   chosen as oldest copy */

      quorum = ((vghdr_ptr->total_vgdas) / 2) + 1;
	/* calculate number of VGDAs/VGSAs needed for a quorum */

      retcode = lvm_delpv (&(vghdr_ptr->vg_id), maphdr_ptr->major_num,
		  pvnum, maphdr_ptr->num_desclps, HD_KMISSPV, quorum);
	/* call routine to mark this PV as missing in the kernel */

      return (retcode);
	/* return whether or not there is an error since this PV is now
	   missing and we do not want to do further writes to it */

  }

  da_index = da_index ^ 1;
    /* exclusive OR the index of the descriptor area copy with 1 in order
       to flip-flop the value from 0 to 1 or from 1 to 0 */

} /* loop for number of VGDAs to write to this PV */

return (LVM_SUCCESS);
  /* return to caller */

} /* END lvm_wrtnext */

