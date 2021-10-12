static char sccsid[] = "@(#)05	1.28.1.5  src/bos/usr/ccs/lib/liblvm/createvg.c, liblvm, bos41B, 9504A 1/10/95 11:20:23";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - createvg.c
 *
 * FUNCTIONS: lvm_createvg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990,
 *               1993
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
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/buf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/hd_psn.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <lvmrec.h>
#include <sys/bootrecord.h>
#include <liblvm.h>


/***********************************************************************
 *                                                                     *
 * NAME:  lvm_createvg                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function creates a new volume group and installs the first   *
 *   physical volume into that volume group.                           *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     createvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     createvg -> vg_id                                               *
 *                                                                     *
 *     A volume group descriptor area for the new volume group is      *
 *     written to the physical volume.                                 *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_INVALID_PARAM                                               *
 *     LVM_ALLOCERR                                                    *
 *     LVM_WRTDAERR                                                    *
 *     LVM_DALVOPN                                                     *
 *     LVM_VGDASPACE                                                   *
 *                                                                     *
 ***********************************************************************
 */


int lvm_createvg (

  struct createvg * createvg)
    /* pointer to a structure which contains the input information for
       the lvm_createvg subroutine */


{ /* BEGIN lvm_createvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char rsrvlv [LVM_EXTNAME];
  /* the device name for the LVM reserved area logical volume */

int retcode;
  /* return code */

int pv_fd;
  /* the file descriptor for the physical volume device */

int lv_fd;
  /* the file descriptor for the LVM reserved area logical volume */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

IPL_REC ipl_rec;
  /* structure into which the IPL record will be read */

struct lvm_rec lvm_rec;
  /* structure into which the LVM information record will be read */

struct timestruc_t cur_time;
  /* a structure to contain the current time from the system clock */

caddr_t vgda_ptr;
  /* a pointer to the in-memory copy of the volume group descriptor
     area */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct vg_trailer * vgtrail_ptr;
  /* a pointer to the volume group descriptor area trailer */

struct lv_entries * lvlist_ptr;
  /* a pointer to the list of logical volume entries in the volume group
     descriptor area */

struct pv_header * pv_ptr;
  /* a pointer to the list of physical volume entries in the volume group
     descriptor area */

daddr_t beg_psn;
  /* the beginning physical sector number where the first physical
     partition starts for a particular physical volume (must begin on a
     logical track boundary) */

long data_capacity;
  /* the data capacity of the disk in number of blocks */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long da_size;
  /* the size in bytes of the volume group descriptor area */

long pventry_size;
  /* the size in bytes of a physical volume entry in the volume group
     descriptor area */

long namelist_size;
  /* the size of the namelist area in the volume group descriptor area */

long num_sectors;
  /* the number of sectors on the physical volume which are available
     to be allocated to users */

long partlen_blks;
  /* the length of a partition in 512 byte blocks */

long num_parts;
  /* the number of partitions on the physical volume which can be
     contained within the available sectors */

long machine_id;
  /* the machine id value for the system */

daddr_t dalsn [LVM_PVMAXVGDAS];
  /* array to hold the logical sector numbers of where the descriptor
     area for a PV is located within the LVM reserved area logical
     volume */

daddr_t salsn [LVM_PVMAXVGDAS];
  /* array to hold the logical sector numbers of where the status area
     for a PV is located within the LVM reserved area logical
     volume */

struct utsname uname_buf;
  /* buffer in which to hold output from the uname system call which is
     used to get the machine id */

struct stat stat_buf;
  /* a buffer to hold certain file status values, of which we are
     using the dev_t device identification */

short int num_desclps;
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */

short int pv_index;
  /* index variable for looping on physical volumes */

short int da_index;
  /* index variable for looping on descriptor areas for one PV */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Check input parameters.                                           *
 ***********************************************************************
 */

if (createvg -> kmid == NULL)
  /* check the input value for the module id to insure that a null pointer
     was not passed */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> vgname == NULL)
  /* check the input value for the VG name (descriptor area LV name) to
     insure that a null pointer was not passed */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> vgname [0] == LVM_NULLCHAR)
  /* check the input value for the VG name (descriptor area LV name) to
     insure that the name is not a null string */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> vg_major <= 0)
  /* check the input value for the major number to insure that it
     is between 1 and the maximum allowed value */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> pvname == NULL)
  /* check the input value for the PV name to insure that a null pointer
     was not passed */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> pvname [0] == LVM_NULLCHAR)
  /* check the input value for the PV name to insure that the name is not
     a null string */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> override != TRUE  &&  createvg -> override != FALSE)
  /* if the flag which indicates whether or not to override the VG member
     error does not have a value of either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> maxlvs <= 0  ||  createvg -> maxlvs > LVM_MAXLVS)
  /* check the input value for maximum logical volumes to insure that it
     is between 1 and the maximum allowed number of logical volumes */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> ppsize < LVM_MINPPSIZ  ||
     createvg -> ppsize > LVM_MAXPPSIZ)
  /* check the input value for physical partition size to insure that it
     falls within the allowed range */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (createvg -> vgda_size < LVM_MINVGDASIZ  ||
     createvg -> vgda_size > LVM_MAXVGDASIZ)
  /* check the input value for physical partition size to insure that it
     falls within the allowed range */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}


/************************************************************************
 *  Zero out the VG id field which is for output returned to the user.  *
 ************************************************************************
 */

bzero (&(createvg -> vg_id), sizeof (struct unique_id));
  /* zero out the field which is to contain the volume group id for the
     new volume group */


/************************************************************************
 *  Initialize the LVM record for this physical volume.                 *
 ************************************************************************
 */

retcode = lvm_instsetup (&(createvg -> vg_id), createvg -> pvname,
			 createvg -> override, &(createvg -> vg_id),
			 &pv_fd, &ipl_rec, &lvm_rec, &data_capacity);
  /* call routine to open the physical volume and to read the IPL
     record and the LVM information record */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error code */

}

retcode = mkuuid (&(lvm_rec.vg_id));
  /* call routine to get a unique id for the volume group id */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

lvm_initlvmrec (&lvm_rec, (short int) createvg -> vgda_size,
		(short int) createvg -> ppsize, data_capacity);
  /* call routine to initialize values in the LVM record */

beg_psn = LVM_PSNFSTPP (PSN_NONRSRVD, lvm_rec.lvmarea_len);
  /* set beginning physical sector number for start of partition space
     to value for the first available sector rounded up to the start of
     the next logical track group (this is because the LVDD has a
     requirement that all physical partitions start on a logical track
     group boundary) */


/************************************************************************
 *  Initialize the bad block directory for this physical volume.        *
 ************************************************************************
 */

retcode = lvm_initbbdir (pv_fd, lvm_rec.reloc_psn);
  /* call routine to initialize the bad block directory or the relocated
     block values */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error return code */

}


/************************************************************************
 *  Initialize the mirror write consistency cache recovery area.        *
 ************************************************************************
 */

retcode = lvm_zeromwc (pv_fd, (short int) TRUE);
  /* call routine to write out a zeroed out mirror write cache recovery
     area to this disk */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error return code */

}


/************************************************************************
 *  Build the volume group descriptor area for this new volume group.   *
 ************************************************************************
 */

da_size = createvg -> vgda_size * DBSIZE;
  /* find size in bytes for the descriptor area */

vgda_ptr = (caddr_t) malloc (da_size);
  /* allocate space in memory for copy of volume group descriptor area */

if (vgda_ptr == NULL)
  /* if an error occurred */

{

    close (pv_fd);
      /* close the physical volume */

    return (LVM_ALLOCERR);
      /* return with error code for programming error */

}

bzero (vgda_ptr, da_size);
  /* zero out the volume group descriptor area */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the volume group header to point at the first block
     of the volume group descriptor area */

vgtrail_ptr = (struct vg_trailer *) (vgda_ptr + da_size - DBSIZE);
  /* set a pointer to the volume group trailer to point at the last block
     of the volume group descriptor area */

retcode = gettimer (TIMEOFDAY, &cur_time);
  /* get the current time from the system clock */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return error code for programming error */

}

vghdr_ptr -> vg_timestamp = cur_time;
  /* initialize the timestamp in the volume group header to 0 */

vgtrail_ptr -> timestamp = cur_time;
  /* initialize the timestamp in the volume group trailer to 0 */

vghdr_ptr -> vg_id = lvm_rec.vg_id;
  /* store the volume group id into the volume group header */

vghdr_ptr -> maxlvs = createvg -> maxlvs;
  /* set the maximum number of logical volumes from the input data */

vghdr_ptr -> pp_size = createvg -> ppsize;
  /* set the physical partition size from the input data */

vghdr_ptr -> total_vgdas = LVM_PVMAXVGDAS;
  /* set the number of volume group descriptor areas to the maximum
     number allowed on one physical volume */

vghdr_ptr -> vgda_size = createvg -> vgda_size;
  /* set the size of the volume group descriptor area from the input
     data */

lvlist_ptr = (struct lv_entries *) (vgda_ptr + DBSIZE);
  /* set a pointer to the list of LVs to point at second block of the
     volume group descriptor area */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate how much space is needed for the descriptor area list of
     logical volumes */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area for
     the logical volume list by rounding the size up to the nearest
     multiple of the blocksize */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set pointer to the beginning of the list of physical volumes to
     point past the end of the logical volume list */

num_sectors =  data_capacity - beg_psn - lvm_rec.reloc_len;
  /* calculate the total number of sectors available on the PV for
     allocation by subtracting sectors before the first partition
     and sectors in the bad block relocation pool */

partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

num_parts = num_sectors / partlen_blks;
  /* using the number of available sectors and the size of a partition in
     sectors, calculate the number of partitions on this PV */
     
/* if more than LVM_MAXPPS partitions are created, the VGSA won't
   correctly track those extra partitions, thus invalidating mirrors */
if (num_parts > LVM_MAXPPS)
	return (LVM_INVALID_PARAM);

lvm_pventry (&(ipl_rec.pv_id), vghdr_ptr, &pv_ptr, num_parts, beg_psn,
	     (short int) LVM_PVMAXVGDAS);
  /* call the lvm_pventry routine to create the entry for this physical
     volume in the PV list of the volume group descriptor area */

lvm_rec.pv_num = (short int) pv_ptr -> pv_num;
  /* store the PV number for this physical volume into the LVM record */

size = sizeof (struct pv_header) + pv_ptr -> pp_count *
				     sizeof (struct pp_entries);
  /* calculate the size of the entry for this physical volume by adding
     the size of the header entry and the size for all the physical
     partition entries */

pventry_size = LVM_SIZTOBBND (size);
  /* round the size for this PV entry up to be a multiple of the block
     size */

size = createvg -> maxlvs * LVM_NAMESIZ;
  /* calculate the size needed for the name list area for this volume
     group */

namelist_size = LVM_SIZTOBBND (size);
  /* round the size for the name list area up to be a multiple of the
     block size */

if (((caddr_t) pv_ptr + pventry_size) >
       ((caddr_t) vgtrail_ptr - namelist_size))
  /* if the descriptor area entry for this physical volume will go past
     the beginning of the name list area in the descriptor area */

{

    close (pv_fd);
      /* close the physical volume */

    return (LVM_VGDASPACE);
      /* return error code for not enough space in the volume group
	 descriptor area to install the PV */

}


/************************************************************************
 *  Define this volume group into the kernel.  The logical volume       *
 *  device driver data structures for the volume group and for the      *
 *  descriptor area logical volume are added into the kernel.           *
 ************************************************************************
 */

num_desclps = (short int) ((lvm_rec.lvmarea_len + lvm_rec.vgsa_psn
		 [LVM_PRMRY]) / partlen_blks  +  1);
  /* calculate the number of partitions needed to include the entire
     LVM reserved area for this physical disk */

retcode = lvm_defvg (partlen_blks, num_desclps, createvg -> kmid,
		     createvg -> vg_major, &(lvm_rec.vg_id),
		     (short int) createvg -> ppsize, (long) TRUE,
			(long)FALSE);
  /* call routine which builds and adds the LVDD data structures for the
     volume group and descriptor area logical volume into the kernel */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error code */

}


/************************************************************************
 *  Add into the kernel the LVDD data structures which describe the     *
 *  physical volume onto which this new volume group is being created.  *
 ************************************************************************
 */

retcode = fstat (pv_fd, &stat_buf);
  /* call system routine to get certain file statistics about this
     device in order to get device identification */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

retcode = lvm_addpv (partlen_blks, num_desclps, stat_buf.st_rdev, pv_fd,
		     (short int) LVM_1STPV, createvg -> vg_major,
		     &(lvm_rec.vg_id), lvm_rec.reloc_psn,
		     lvm_rec.reloc_len, beg_psn, salsn, (short int) 0);
  /* call routine to build the kernel data structures and add this
     physical volume into the kernel for the volume group */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error code */

}


/************************************************************************
 *  Write the volume group descriptor area to the disk.                 *
 ************************************************************************
 */

retcode = status_chk (NULL, createvg -> vgname, NOCHECK, rsrvlv);
  /* check and build the device name of the LVM reserved area logical
     volume */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error return code */

}

lv_fd = open (rsrvlv, O_RDWR);
  /* open the LVM reserved area logical volume for this volume group */

if (lv_fd == LVM_UNSUCC)
  /* if an error occurred */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_DALVOPN);
      /* return error code for LVM reserved area logical volume open
	 error */

}

for (da_index = 0; da_index < LVM_PVMAXVGDAS; da_index = da_index + 1)
  /* loop for each descriptor area / status area on a PV */

{

  dalsn [da_index] = lvm_rec.vgda_psn [da_index];
    /* set descriptor area logical sector number to physical sector
       number (they are the same because this is the first PV in the
       volume group) */

  salsn [da_index] = lvm_rec.vgsa_psn [da_index];
    /* set status area logical sector number to physical sector number
       (they are the same because this is the first PV in the volume
       group) */

} /* loop for each VGDA/VGSA on a PV */

retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	    &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	    dalsn [LVM_PRMRY], (short int) LVM_BEGMIDEND);
  /* call routine to write the volume group descriptor area for the
     first copy on this disk */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (lv_fd);
      /* close the LVM reserved area logical volume */

    close (pv_fd);
      /* close the physical volume */

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    return (LVM_WRTDAERR);
      /* return with error for unable to write descriptor area */

}

retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	    &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	    dalsn [LVM_SCNDRY], (short int) LVM_BEGMIDEND);
  /* call routine to write the volume group descriptor area for the
     second copy on this disk */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (lv_fd);
      /* close the LVM reserved area logical volume */

    close (pv_fd);
      /* close the physical volume */

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    return (LVM_WRTDAERR);
      /* return with error for unable to write descriptor area */

}

retcode = lvm_zerosa (lv_fd, salsn);
  /* call routine to write out a zeroed out volume group status area */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (lv_fd);
      /* close the LVM reserved area logical volume */

    close (pv_fd);
      /* close the physical volume */

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    return (retcode);
      /* return with error return code */

}
retcode = close (lv_fd);
  /* close the LVM reserved area logical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PROGERR);
      /* return error code for programming error */

}


/************************************************************************
 *  Write the LVM record for this physical volume.                      *
 ************************************************************************
 */
retcode = lvm_wrlvmrec (pv_fd, &lvm_rec);
  /* call routine to write out the LVM record and the LVM backup record
     to the physical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    lvm_zerolvm (pv_fd);
      /* since an error occurred, call routine to zero out the LVM record
	 and its backup in order to indicate that this physical volume is
	 not installed in a volume group */

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with unsuccessful return code */

}

retcode = close (pv_fd);
  /* close the physical volume device */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
      /* call routine to delete this volume group from the kernel since
	 it could not be successfully defined on the disk */

    return (LVM_PROGERR);
      /* return with error code for programming error */

}


/************************************************************************
 *  Return to caller with the volume group id of the new volume group.  *
 ************************************************************************
 */

createvg -> vg_id = lvm_rec.vg_id;
  /* store the volume group id into the create VG input structure in
     order to return it to the caller */

lvm_delvg (&(lvm_rec.vg_id), createvg -> vg_major);
  /* call routine to delete this volume group from the kernel since we
     do not leave the volume group varied on after a createvg */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_createvg */

