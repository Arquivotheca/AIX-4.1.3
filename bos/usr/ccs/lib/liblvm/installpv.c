static char sccsid[] = "@(#)32	1.32  src/bos/usr/ccs/lib/liblvm/installpv.c, liblvm, bos41B, 9504A 1/10/95 16:46:42";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - installpv.c
 *
 * FUNCTIONS: lvm_installpv
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
#include <sys/buf.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/hd_psn.h>
#include <sys/dasd.h>
  /* include file for LVDD data structures */
#include <sys/hd_config.h>
#include <sys/vgsa.h>
#include <sys/bootrecord.h>
#include <lvmrec.h>
#include <liblvm.h>
  /* include file for LVM subroutine structures */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_installpv                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function installs a physical volume into an already existent *
 *   volume group.                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     installpv                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The volume group descriptor area has been updated to include    *
 *     the new physical volume and the volume group mapped file has    *
 *     been updated.                                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_WRTDAERR                                                    *
 *     LVM_DALVOPN                                                     *
 *     LVM_VGDASPACE                                                   *
 *     LVM_INVALID_PARAM                                               *
 *     LVM_ALRDYMEM                                                    *
 *     LVM_PVMAXERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_installpv (

struct installpv * installpv)
    /* pointer to the structure which contains input information for the
       lvm_installpv subroutine */


{ /* BEGIN lvm_installpv */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

int rc;
  /* variable to hold return code from deleting the PV during backout
     because of a previous error */

int member;
  /* flag to indicate if PV is a member of a specified volume group */

int mapf_mode;
  /* the access mode with which to open the mapped file */

int pv_fd;
  /* the file descriptor of the physical volume device */

int lv_fd;
  /* the file descriptor of the LVM reserved area logical volume */

int vgmap_fd;
  /* the file descriptor of the mapped file */

caddr_t vgmap_ptr;
  /* pointer to the beginning of the mapped file */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

daddr_t beg_psn;
  /* the beginning physical sector number where the first physical
     partition starts for a particular physical volume (must begin on a
     logical track boundary) */

daddr_t salsn [LVM_PVMAXVGDAS];
  /* array to hold the logical sector numbers within the volume group
     reserved area logical volume of where the VG status area copies
     for this PV are located */

long data_capacity;
  /* the data capacity of the disk in number of blocks */

long num_sectors;
  /* the number of sectors on the physical volume which are available
     to be allocated to users */

long num_parts;
  /* the number of partitions on the physical volume which can be
     contained within the available sectors */

long partlen_blks;
  /* the length of a partition in number of 512 byte blocks */

long size;
  /* variable to hold a temporary size calculation */

long pventry_size;
  /* the size of the physical volume entry in the descriptor area for the
     physical volume being added */

long pvoffset;
  /* the offset from the beginning of the mapped file for the new PV
     entry */

short int num_desclps;
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */

short int pv_index;
  /* index variable used for looping on physical volumes in the VG */

short int total_3to3;
  /* variable used to indicate when we have special case of going from
     a total of 3 VGDA copies to a total of 3 VGDA copies after the
     install of the new physical volume */

short int sav_pv_2;
  /* variable to save the PV number of the physical volume with 2 copies
     of the VGDA */

short int sav_pv_1;
  /* variable to save the PV number of the physical volume with 1 copy
     of the VGDA */

char rsrvlv [LVM_EXTNAME];
  /* the device name for the LVM reserved area logical volume */

struct timestruc_t cur_time;
  /* a structure to contain the current time from the system clock */

struct fheader * maphdr_ptr;
  /* a pointer to the file header of the mapped file */

caddr_t vgda_ptr;
  /* pointer to the beginning of the volume group descriptor area */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group header in the volume group descriptor
     area */

struct vg_trailer * vgtrail_ptr;
  /* a pointer to the volume group descriptor area trailer */

struct pv_header * pv_ptr;
  /* a pointer to a physical volume header entry in the volume group
     descriptor area */

struct stat stat_buf;
  /* a buffer to hold certain file status values, of which we are using
     the dev_t, or device, parameter */

IPL_REC ipl_rec;
  /* structure into which the IPL record will be read */

struct lvm_rec lvm_rec;
  /* structure into which the LVM information record will be read */

short int quorum_cnt;
  /* number of VGDAs/VGSAs needed for a quorum */

short int da_index;
  /* index for possible VGDA/VGSA copies on a PV */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Check input parameters.                                           *
 ***********************************************************************
 */

if (installpv -> pvname == NULL)
  /* if the physical volume name passed in contains a null pointer */

{

    return (LVM_INVALID_PARAM);
      /* return with error code for invalid parameter */

}

if (installpv -> pvname [0] == LVM_NULLCHAR)
  /* if the physical volume name contains a null string */

{

    return (LVM_INVALID_PARAM);
      /* return with error code for invalid parameter */

}

if (installpv -> override != TRUE  &&  installpv -> override != FALSE)
  /* if the flag which indicates to override the VG member error does not
     have a value of either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return with error code for invalid parameter */

}

/***********************************************************************
 *   Open the mapped file.                                             *
 ***********************************************************************
 */

mapf_mode = O_RDWR | O_NSHARE;
  /* set the access mode with which to open the mapped file */

retcode = lvm_getvgdef (&(installpv -> vg_id), mapf_mode, &vgmap_fd,
		       &vgmap_ptr);
  /* call routine to open the mapped file and read it into memory */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}

maphdr_ptr = (struct fheader *) vgmap_ptr;
  /* set a pointer to the file header portion of the mapped file */

vgda_ptr = vgmap_ptr + sizeof (struct fheader);
  /* set a pointer to the beginning of the volume group descriptor
     area */

vghdr_ptr = (struct vg_header *) (vgmap_ptr + sizeof (struct fheader));
  /* set a pointer to the volume group header to point at the first block
     of the volume group descriptor area */

vgtrail_ptr = (struct vg_trailer *) (vgmap_ptr + maphdr_ptr -> trailinx);
  /* set a pointer to the trailer of the volume group descriptor area */

if (vghdr_ptr -> numpvs == LVM_MAXPVS)
  /* if this volume group already contains the maximum number of allowed
     physical volumes */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    return (LVM_PVMAXERR);
      /* return error code for volume group already contains maximum
	 number of allowed PVs */

}


/***********************************************************************
 *   Initialize the LVM record.                                        *
 ***********************************************************************
 */

retcode = lvm_instsetup (&(installpv -> vg_id), installpv -> pvname,
		installpv -> override, &(installpv -> out_vg_id),
		&pv_fd, &ipl_rec, &lvm_rec, &data_capacity);
  /* call routine to open the physical volume device and to read the
     IPL record and the LVM record */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    return (retcode);
      /* return with error code */

}

member = lvm_vgmem (&(ipl_rec.pv_id), (caddr_t) vghdr_ptr);
  /* call routine to check if this PV is already a member of the volume
     group into which we are trying to install it */

if (member == TRUE)
  /* if the PV to be installed is already a member of the volume group */

{

    installpv -> out_vg_id = installpv -> vg_id;
      /* save VG id of volume group into which PV was to be installed
	 into the output VG id */

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_ALRDYMEM);
      /* return error code for PV is already a member of the requested
	 volume group */

}

lvm_rec.vg_id = installpv -> vg_id;
  /* store the volume group id into the LVM information record */

lvm_initlvmrec (&lvm_rec, vghdr_ptr -> vgda_size, vghdr_ptr -> pp_size,
		data_capacity);
  /* call routine to initialize values in the LVM record */

beg_psn = LVM_PSNFSTPP (PSN_NONRSRVD, lvm_rec.lvmarea_len);
  /* set beginning physical sector number for start of partition space
     to value for the first available sector rounded up to the start of
     the next logical track group (this is because the LVDD has a
     requirement that all physical partitions start on a logical track
     group boundary) */


/***********************************************************************
 *   Initialize the bad block directory.                               *
 ***********************************************************************
 */

retcode = lvm_initbbdir (pv_fd, lvm_rec.reloc_psn);
  /* call routine to initialize the bad block directory or the relocated
     block values */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error return code */

}


/***********************************************************************
 *   Initialize the mirror write cache area.                           *
 ***********************************************************************
 */

retcode = lvm_zeromwc (pv_fd, (short int) FALSE);
  /* call routine to write out a zeroed out mirror write cache recovery
     area to the PV */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with error return code */

}


/************************************************************************
 *   Calculate new number for the quorum count using a value for total  *
 *   VGDAs/VGSAs which has one added for the new PV.  This is the value *
 *   which will be sent to the kernel config routine which adds the PV. *
 *   NOTE that if a VGDA/VGSA copy is to be removed from another PV,    *
 *   the quorum count will again be adjusted when the VGSA copy is      *
 *   removed from the kernel.                                           *
 ************************************************************************
 */

quorum_cnt = (short int) ((vghdr_ptr -> total_vgdas + 1)/ 2 + 1);
  /* calculate new number of VGDAs/VGSAs needed for a quorum */


/***********************************************************************
 *   Check for special case of total VGDAs equals three and number of  *
 *   PVs containing at least one copy of the VGDA is two.  This means  *
 *   that the total number of VGDAs will remain at three but we will   *
 *   be adding a VGDA to a new PV and removing a VGDA from an old PV   *
 *   that had two copies of the VGDA.                                  *
 ***********************************************************************
 */

total_3to3 = FALSE;
  /* set flag to indicate that special case of going from total VGDAs
     of 3 to 3 is not present */

sav_pv_2 = 0;
  /* initialize to 0 variable which is to hold PV number of PV with 2
     copies of the VGDA if we have the special case */

sav_pv_1 = 0;
  /* initialize to 0 variable which is to hold the PV number of PV with
     just 1 copy of the VGDA if we have the special case */

if (vghdr_ptr -> total_vgdas == LVM_TTLDAS_2PV)
  /* if the total number of VGDAs indicates we should check for the
     special case */

{

    pv_ptr = (struct pv_header *) (vgmap_ptr + maphdr_ptr -> pvinx);
      /* set pointer to the beginning of the list of physical volumes
	 within the volume group descriptor area */

    for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
      /* loop for number of physical volumes in the volume group */

    {

      if (pv_ptr -> pvnum_vgdas == LVM_PVMAXVGDAS)
	/* if this PV has two copies of the VGDA */

      {

	  total_3to3 = TRUE;
	    /* since we found a PV with two copies of the VGDA we have
	       the special case we are checking for */

	  sav_pv_2 = pv_ptr -> pv_num;
	    /* save the PV number of the PV with two copies */

	  pv_ptr -> pvnum_vgdas = 1;
	    /* change number of volume group descriptor areas to be
	       contained on this PV to just one copy */

      }

      if (pv_ptr -> pvnum_vgdas == 1)
	/* if this PV has just one copy of the VGDA */

      {

	  sav_pv_1 = pv_ptr -> pv_num;
	    /* save the PV number of this PV which we will need if we
	       have the special case */

      }

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

    } /* loop for PVs in the volume group */

} /* possible special case */


/***********************************************************************
 *   Add information about the physical volume to the volume group     *
 *   descriptor area.                                                  *
 ***********************************************************************
 */

num_sectors =  data_capacity - beg_psn - lvm_rec.reloc_len;
  /* calculate the total number of sectors available on the PV for
     allocation by subtracting sectors before the first partition
     and sectors in the bad block relocation pool */

num_parts = num_sectors / LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* using the number of available sectors and the size of a partition,
     calculate the number of partitions on this PV */

/* if more than LVM_MAXPPS partitions are created, the VGSA won't
   correctly track those extra partitions, thus invalidating mirrors */
if (num_parts > LVM_MAXPPS)
        return (LVM_INVALID_PARAM);

pv_ptr = (struct pv_header *) (vgmap_ptr + maphdr_ptr -> pvinx);
  /* set pointer to the beginning of the list of physical volumes
     within the volume group descriptor area */

lvm_pventry (&(ipl_rec.pv_id), vghdr_ptr, &pv_ptr, num_parts, beg_psn,
	     (short int) LVM_DASPERPVGEN);
  /* call the lvm_pventry routine to create the entry for this physical
     volume in the PV list of the volume group descriptor area */

lvm_rec.pv_num = (short int) pv_ptr -> pv_num;
  /* store the PV number of the physical volume into the LVM record */

size = sizeof (struct pv_header) + pv_ptr -> pp_count *
				     sizeof (struct pp_entries);
  /* calculate the size needed for this physical volume entry in the
     volume group descriptor area */

pventry_size = LVM_SIZTOBBND (size);
  /* round the size for the PV entry up to the next multiple of the
     block size */

pvoffset = (caddr_t) pv_ptr - (caddr_t) vgmap_ptr;
  /* calculate the offset from the beginning of the mapped file for this
     physical volume entry */

if (pvoffset + pventry_size > maphdr_ptr -> name_inx)
  /* with the new PV added, if the end of the PV entries goes past the
     beginning of the name list area */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_VGDASPACE);
      /* return error code for not enough space in the volume group
	 descriptor area to add the PV entry for the new PV */

}

partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

num_desclps = (short int) ((lvm_rec.lvmarea_len + lvm_rec.vgsa_psn
		 [LVM_PRMRY]) / partlen_blks  +  1);
  /* calculate the number of partitions needed to include the entire
     LVM reserved area on one physical disk */


/************************************************************************
 *   Add the LSNs of the volume group descriptor areas for this         *
 *   PV to the information in the mapped file which is used for         *
 *   updating the on-disk copies.  Update the PV information array      *
 *   in the mapped file header with information for the new PV.         *
 ************************************************************************
 */

for (da_index = 0; da_index < LVM_PVMAXVGDAS; da_index = da_index + 1)
  /* loop for each possible VGDA/VGSA copy on a PV */

{

  maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].da [da_index].dalsn =
       num_desclps * partlen_blks * (pv_ptr -> pv_num - 1)
       + lvm_rec.vgda_psn [da_index];
    /* calculate the logical sector number within the LVM reserved area
       logical volume of where this copy of the descriptor area for this
       PV resides */

  if (da_index < pv_ptr -> pvnum_vgdas)
    /* if this VGSA copy is valid for this PV */

  {

      salsn [da_index] = num_desclps * partlen_blks *
	     (pv_ptr -> pv_num - 1) + lvm_rec.vgsa_psn [da_index];
	/* calculate the logical sector number within the LVM reserved
	   area logical volume of where this copy of the status area
	   for this PV resides */

  }

  else

  {

      salsn [da_index] = 0;
	/* set logical sector number to 0 for this VGSA copy to indicate
	   to the kernel that it is not valid */

  }

}

maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].pvinx = pvoffset;
  /* store the offset from the beginning of the mapped file for the PV
     entry for the new PV into the PV information array in the mapped
     file header */

maphdr_ptr -> endpvs = pvoffset + pventry_size;
  /* update the value in the mapped file header for the offset to the end
     of the physical volume entries */

maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].pv_id =  pv_ptr -> pv_id;
  /* store the unique id for the new PV into the PV information array */

strncpy (maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].pvname,
	 installpv -> pvname, LVM_NAMESIZ - 1);
  /* copy the PV name passed in to this routine into the PV information
     array */

retcode = fstat (pv_fd, &stat_buf);
  /* call system routine to get certain file statistics about this device
     (we will use the device identification) */

if (retcode == LVM_UNSUCC)
  /* if an error occurred from getting the file status */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PROGERR);
      /* return error code for programming error */

}

maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].device = stat_buf.st_rdev;
  /* set value for device major/minor number in the PV information
     array */


/************************************************************************
 *   Build the LVDD data structures which describe this physical volume *
 *   and add them to the kernel for the specified volume group.         *
 ************************************************************************
 */


retcode = lvm_addpv (partlen_blks, num_desclps, stat_buf.st_rdev, pv_fd,
	    pv_ptr -> pv_num, maphdr_ptr -> major_num, &(installpv ->
	    vg_id), lvm_rec.reloc_psn, lvm_rec.reloc_len, beg_psn, salsn,
	    quorum_cnt);
  /* call routine which will build the kernel data structures for this
     PV and add them into the kernel */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return error return code */

}


/************************************************************************
 *   Get new timestamp and write out all copies of updated VGDA which   *
 *   contains information about the new PV being added.                 *
 ************************************************************************
 */

retcode = gettimer (TIMEOFDAY, &cur_time);
  /* get the current time from the system clock */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    rc  = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
      /* call routine to delete this physical volume from the kernel
	 since it could not be successfully defined on the disk */

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    if (rc == LVM_FORCEOFF)
      /* if an error for force varyoff was returned from deleting PV */

    {

	return (rc);
	  /* return with error for forced varyoff */

    }

    return (LVM_PROGERR);
      /* return error code for programming error */

}

retcode = status_chk (NULL, maphdr_ptr -> vgname, NOCHECK, rsrvlv);
  /* check and build the device name of the LVM reserved area logical
     volume */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	   pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
      /* call routine to delete this physical volume from the kernel
	 since it could not be successfully defined on the disk */

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    if (rc == LVM_FORCEOFF)
      /* if an error for force varyoff was returned from deleting PV */

    {

	return (rc);
	  /* return with error for forced varyoff */

    }

    return (retcode);
      /* return with error return code */

}

lv_fd = open (rsrvlv, O_RDWR);
  /* open the LVM reserved area logical volume for this volume group */

if (lv_fd == LVM_UNSUCC)
  /* if an error occurred */

{

    rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	   pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
      /* call routine to delete this physical volume from the kernel
	 since it could not be successfully defined on the disk */

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    close (pv_fd);
      /* close the physical volume */

    if (rc == LVM_FORCEOFF)
      /* if an error for force varyoff was returned from deleting PV */

    {

	return (rc);
	  /* return with error for forced varyoff */

    }

    return (LVM_DALVOPN);
      /* return error code for LVM reserved area logical volume open
	 error */

}

if (total_3to3 == TRUE)
  /* if this is special case of total number of VGDAs remaining at 3 with
     placement of those VGDA copies changing */


    /********************************************************************
     *   Update of VGDA for special case where one copy is added to new *
     *   PV and one copy is deleted from old PV which has 2 copies.     *
     ********************************************************************
     */

{

    vghdr_ptr -> vg_timestamp.tv_sec = 0;
    vghdr_ptr -> vg_timestamp.tv_nsec = 0;
      /* update timestamp value in the volume group header to 0 for
	 writing VGDA to the new PV */

    vgtrail_ptr -> timestamp.tv_sec = 0;
    vgtrail_ptr -> timestamp.tv_nsec = 0;
      /* update timestamp value in the volume group trailer to 0 for
	 writing to the new PV */


    /*******************************************************************
     *   Write the volume group descriptor area with a timestamp of 0  *
     *   to the new PV.  This is done so that the new PV will have a   *
     *   valid VGDA but it will always be older than others in the     *
     *   volume group and would not be picked as newest during varyon  *
     *   after a crash which might occur during adding of the new PV.  *
     *******************************************************************
     */

    retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	 &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	 maphdr_ptr -> pvinfo [pv_ptr->pv_num-1].da [LVM_PRMRY].dalsn,
	 (short int) LVM_BEGMIDEND);
      /* call routine to write the volume group descriptor area for the
	 primary copy on the new PV */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (LVM_WRTDAERR);
	  /* return with error for unable to write descriptor area */

    }

    maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].da [LVM_PRMRY].ts =
		    vghdr_ptr -> vg_timestamp;
      /* store the timestamp for the VGDA copy just written into the
	 mapped file header */

    retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	 &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	 maphdr_ptr -> pvinfo [pv_ptr->pv_num-1].da [LVM_SCNDRY].dalsn,
	 (short int) LVM_BEGMIDEND);
      /* call routine to write the volume group descriptor area for the
	 secondary copy on the new PV */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (LVM_WRTDAERR);
	  /* return with error for unable to write descriptor area */

    }

    maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].da [LVM_SCNDRY].ts =
		    vghdr_ptr -> vg_timestamp;
      /* store the timestamp for the VGDA copy just written into the
	 mapped file header */


    /********************************************************************
     *   Write out the LVM information record for the new PV.           *
     ********************************************************************
     */

    lvm_rec.version = (short)lvm_getversion(maphdr_ptr);	
      /* version from another pv in the volume group */

    retcode = lvm_wrlvmrec (pv_fd, &lvm_rec);
      /* call routine to write out the LVM record and the LVM backup
	 record to the physical volume */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	lvm_zerolvm (pv_fd);
	  /* since an error occurred, call routine to zero out the LVM
	     record and its backup in order to indicate that this
	     physical volume is not installed in a volume group */

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (retcode);
	  /* return with error return code */

    }

    retcode = close (pv_fd);
      /* close the physical volume device */

    if (retcode == LVM_UNSUCC)
      /* if an error occurred */

    {

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	return (LVM_PROGERR);
	  /* return with error code for programming error */

    }


    /********************************************************************
     *   Update timestamp in VGDA and write to all VGDA copies,         *
     *   including writing to the new PV with the new timestamp.        *
     ********************************************************************
     */

    vghdr_ptr -> vg_timestamp = cur_time;
      /* update timestamp value in the volume group header */

    vgtrail_ptr -> timestamp = cur_time;
      /* update timestamp value in the volume group trailer */

    retcode = lvm_vgdas3to3 (lv_fd, vgmap_ptr,
		(short int) pv_ptr -> pv_num, sav_pv_2, sav_pv_1);
      /* call routine to update VGDA copies for special case */

    if (retcode < LVM_SUCCESS)
	  /* if an error occurred */

    {

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	return (retcode);
	  /* return with error (this should normally be forced varyoff) */

    }

} /* special case */

else


    /********************************************************************
     *   Update of VGDA for normal case where one copy added to new PV. *
     ********************************************************************
     */

{

    vghdr_ptr -> vg_timestamp = cur_time;
      /* update timestamp value in the volume group header */

    vgtrail_ptr -> timestamp = cur_time;
      /* update timestamp value in the volume group trailer */

    vghdr_ptr -> total_vgdas = vghdr_ptr -> total_vgdas + 1;
      /* increment number of descriptor areas in the volume group
	 header */


    /********************************************************************
     *   Write the volume group descriptor area to the new PV.          *
     ********************************************************************
     */

    retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	 &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	 maphdr_ptr -> pvinfo [pv_ptr->pv_num-1].da [LVM_PRMRY].dalsn,
	 (short int) LVM_BEGMIDEND);
      /* call routine to write the volume group descriptor area for the
	 first copy on this disk */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (LVM_WRTDAERR);
	  /* return with error for unable to write descriptor area */

    }

    maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].da [LVM_PRMRY].ts =
		    vghdr_ptr -> vg_timestamp;
      /* store the timestamp for the VGDA copy just written into the
	 mapped file header */

    retcode = lvm_wrtdasa (lv_fd, vgda_ptr, (struct timestruc_t *)
	 &(vgtrail_ptr -> timestamp), (long) vghdr_ptr -> vgda_size,
	 maphdr_ptr -> pvinfo [pv_ptr->pv_num-1].da [LVM_SCNDRY].dalsn,
	 (short int) LVM_BEGMIDEND);
      /* call routine to write the volume group descriptor area for the
	 second copy on this disk */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (LVM_WRTDAERR);
	  /* return with error for unable to write descriptor area */

    }

    maphdr_ptr -> pvinfo [pv_ptr -> pv_num - 1].da [LVM_SCNDRY].ts =
		    vghdr_ptr -> vg_timestamp;
      /* store the timestamp for the VGDA copy just written into the
	 mapped file header */


    /********************************************************************
     *   Write out the LVM information record for the new PV.           *
     ********************************************************************
     */

    retcode = lvm_wrlvmrec (pv_fd, &lvm_rec);
      /* call routine to write out the LVM record and the LVM backup
	 record to the physical volume */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	lvm_zerolvm (pv_fd);
	  /* since an error occurred, call routine to zero out the LVM
	     record and its backup in order to indicate that this
	     physical volume is not installed in a volume group */

	rc = lvm_delpv (&(installpv -> vg_id), maphdr_ptr -> major_num,
	       pv_ptr -> pv_num, num_desclps, HD_KDELPV, quorum_cnt);
	  /* call routine to delete this physical volume from the kernel
	     since it could not be successfully defined on the disk */

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (pv_fd);
	  /* close the physical volume */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	if (rc == LVM_FORCEOFF)
	  /* if error for force varyoff was returned from deleting PV */

	{

	    return (rc);
	      /* return with error for forced varyoff */

	}

	return (retcode);
	  /* return with unsuccessful return code */

    }

    retcode = close (pv_fd);
      /* close the physical volume device */

    if (retcode == LVM_UNSUCC)
      /* if an error occurred */

    {

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	return (LVM_PROGERR);
	  /* return with error code for programming error */

    }


    /********************************************************************
     *   Write updated VGDA to remaining copies on other PVs.           *
     ********************************************************************
     */

    retcode = lvm_updvgda (lv_fd, maphdr_ptr, vgda_ptr);
      /* call routine to update the remaining VGDA copies */

    if (retcode < LVM_SUCCESS)
      /* if an error is returned */

    {

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	close (lv_fd);
	  /* close the LVM reserved area logical volume */

	return (retcode);
	  /* return with error (this should normally be forced varyoff) */

    }

} /* normal case of adding one VGDA copy to new PV */


retcode = close (lv_fd);
  /* close the LVM reserved area logical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    free (vgmap_ptr);
      /* free space allocated for the mapped file */

    close (vgmap_fd);
      /* close the mapped file */

    return (LVM_PROGERR);
      /* return error code for programming error */

}


maphdr_ptr -> quorum_cnt = vghdr_ptr -> total_vgdas / 2 + 1;
  /* update the value for number of VGDAs/VGSAs needed for a quorum */


/************************************************************************
 *   If this install was for the special case, where we are removing a  *
 *   VGDA/VGSA copy from one of the other PVs in the volume group, then *
 *   build and send information to the kernel to remove the volume      *
 *   group status area from that PV.                                    *
 ************************************************************************
 */

if (total_3to3 == TRUE)
  /* if this is the special case */

{

    quorum_cnt = (short int) (vghdr_ptr -> total_vgdas / 2 + 1);
      /* calculate new number of VGDAs/VGSAs needed for a quorum */

    salsn [LVM_PRMRY] = (daddr_t) FALSE;
      /* set value in array for status area location to FALSE to indicate
	 that the primary PV status area is not being deleted */

    salsn [LVM_SCNDRY] = (daddr_t) TRUE;
      /* set value in array for status area location to TRUE to indicate
	 that the secondary PV status area is being deleted */

    retcode = lvm_chgvgsa (&(installpv -> vg_id), maphdr_ptr ->
		major_num, salsn, sav_pv_2, quorum_cnt, HD_KDELVGSA);
      /* call routine to send information to the kernel to remove the
	 second VGSA from the PV which contains two */

    if (retcode < LVM_SUCCESS)
      /* if not successful */

    {

	free (vgmap_ptr);
	  /* free space allocated for the mapped file */

	close (vgmap_fd);
	  /* close the mapped file */

	return (retcode);
	  /* return with error (this should normally be forced varyoff) */

    }

}


free (vgmap_ptr);
  /* free space allocated for the mapped file */

retcode = close (vgmap_fd);
  /* close the mapped file */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_installpv */




