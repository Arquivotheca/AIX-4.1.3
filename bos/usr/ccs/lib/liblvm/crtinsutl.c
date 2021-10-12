static char sccsid[] = "@(#)65	1.24  src/bos/usr/ccs/lib/liblvm/crtinsutl.c, liblvm, bos411, 9428A410j 2/22/93 16:17:51";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - crtinsutl.c
 *
 * FUNCTIONS: lvm_initbbdir,
 *            lvm_initlvmrec,
 *            lvm_instsetup,
 *            lvm_pventry,
 *            lvm_vgdas3to3,
 *            lvm_vgmem,
 *            lvm_zeromwc,
 *            lvm_zerosa
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
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/time.h>
#include <sys/bbdir.h>
#include <sys/buf.h>
#include <sys/shm.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/hd.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_config.h>
#include <lvmrec.h>
#include <sys/bootrecord.h>
#include <liblvm.h>
#include <sys/hd_psn.h>









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_initbbdir                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function will assign relocation blocks for a bad block       *
 *   directory which has been initialized with bad blocks or will      *
 *   set the ID field and initialize the bad block directory with      *
 *   zero entries for an uninitialized bad block directory.            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     reloc_psn                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_BADBBDIR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_initbbdir (

int pv_fd,
  /* the file descriptor for the physical volume device */

daddr_t reloc_psn)
  /* the physical sector number of the beginning of the bad block
     relocation pool */


{ /* BEGIN lvm_initbbdir */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char bbdir_buf [LEN_BB_DIR * DBSIZE];
  /* a buffer into which the bad block directory will be read */

struct bb_hdr * bb_hdr;
  /* a pointer to the header of the bad block directory */

struct bb_entry * bb_entry;
  /* a pointer to an entry in the bad block directory */

daddr_t next_relblk;
  /* the physical sector address of the next block which is available to
     be assigned as a relocation block in the bad block directory */

int retcode;
  /* the return code */

short int bb_index;
  /* index variable used for looping on bad block entries */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

retcode = lvm_rdbbdir (pv_fd, bbdir_buf, LVM_BBRDINIT);
  /* call routine to read the bad block directory from the disk */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (LVM_BADBBDIR);
      /* return with error code for bad block directory is bad since we
	 do not want to use the PV if the bad block directory cannot be
	 read */

}

/********************************************************************
 *  Assign relocation blocks from the relocation pool for the bad   *
 *  blocks in the bad block directory.                              *
 ********************************************************************
 */

bb_hdr = (struct bb_hdr *) bbdir_buf;
  /* set a pointer to the header of the bad block directory */

bb_entry = (struct bb_entry *) ((caddr_t) bbdir_buf + sizeof (struct bb_hdr));
  /* set a pointer to the first bad block entry in the bad block directory */

next_relblk = reloc_psn;
  /* set the next available relocation block to the first block in
	 the relocation pool */

for (bb_index=0;  bb_index < bb_hdr->num_entries; bb_index=bb_index+1)
  /* loop for each entry in the bad block directory */

{

  if (bb_entry -> rel_stat == REL_DONE)
    /* if the relocation status for this bad block indicates that
       relocation has been done */

  {

      bb_entry -> rel_lsn = next_relblk;
	/* set the sector number for the relocation block to be the
	   next available block in the relocation pool */

      next_relblk = next_relblk + 1;
	/* increment the value of the next available relocation block */

  }

  bb_entry = (struct bb_entry *) ((caddr_t) bb_entry +
				       sizeof (struct bb_entry));
    /* increment pointer to point at next bad block entry in the bad
       block directory */

} /* loop for entries in bad block directory */

retcode = lvm_wrbbdir (pv_fd, bbdir_buf, LVM_BBPRIM | LVM_BBBACK);
  /* write the updated bad block directory to the primary and backup
     on the disk */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (LVM_BADBBDIR);
      /* return with error code for bad block directory is bad since we
	 do not want to use the PV if the bad block directory cannot be
	 written */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_initbbdir */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_initlvmrec                                               *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function initializes data in the LVM record.                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     lvm_rec                                                         *
 *     vgda_size                                                       *
 *     ppsize                                                          *
 *     data_capacity                                                   *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *lvm_rec                                                        *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_initlvmrec (

struct lvm_rec * lvm_rec,
  /* pointer to the LVM information record */

short int vgda_size,
  /* the length of the volume group descriptor area in blocks */

short int ppsize,
  /* physical partition size represented as a power of 2 */

long data_capacity)
  /* the data capacity of the disk in number of blocks */


{ /* BEGIN lvm_initlvmrec */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

daddr_t fstsctr_scndsa;
  /* the first physical sector of the secondary volume group status
     area */

daddr_t lstsctr_scndsa;
  /* the last physical sector of the secondary volume group status
     area */

daddr_t fstavailsctr;
  /* the first available physical sector after the end of the secondary
     volume group descriptor area */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/************************************************************************
 *  Initialize data in the LVM record.                                  *
 ************************************************************************
 */

lvm_rec -> lvm_id = LVM_LVMID;
  /* set LVM id field to indicate that this physical volume is a member
     of a volume group */

lvm_rec -> version = LVM_VERSION_1;
  /* set the version number field to the value for this version of the
     code */

lvm_rec -> vgsa_len = VGSA_BLK;
  /* set the length in sectors of the volume group status area */

lvm_rec -> vgda_len = vgda_size;
  /* set the length in sectors of the volume group descriptor area from
     the value passed in */

lvm_rec -> vgsa_psn [LVM_PRMRY] = PSN_NONRSRVD;
  /* set the physical sector of the beginning of the status area to the
     beginning of the non-reserved space */

lvm_rec -> vgda_psn [LVM_PRMRY] = lvm_rec -> vgsa_psn [LVM_PRMRY]
				      + VGSA_BLK;
  /* set the physical sector of the beginning of the descriptor area to
     the block following the status area */

fstsctr_scndsa = lvm_rec -> vgda_psn [LVM_PRMRY] + lvm_rec -> vgda_len;
  /* set the first sector for the secondary copy of the status area to
     the sector following the end of the primary descriptor area */

lstsctr_scndsa = fstsctr_scndsa + VGSA_BLK - 1;
  /* set the last sector for the secondary copy of the status area to
     the value for the last sector */

if (BLK2TRK (fstsctr_scndsa) != BLK2TRK (lstsctr_scndsa))
  /* if the first sector and the last sector for the proposed secondary
     copy of the status area do not fall in the same logical track
     group */

{

    fstsctr_scndsa = TRK2BLK (BLK2TRK (fstsctr_scndsa) + 1);
      /* recalculate the first sector for the secondary copy of the
	 status area so that it will start on next logical track
	 boundary (this is because the LVDD requires that the entire
	 VGSA fall within the same logical track group) */

}

lvm_rec -> vgsa_psn [LVM_SCNDRY] = fstsctr_scndsa;
  /* store the PSN for the beginning of the secondary copy of the status
     area */

lvm_rec -> vgda_psn [LVM_SCNDRY] = lvm_rec -> vgsa_psn [LVM_SCNDRY]
					+ VGSA_BLK;
  /* set the PSN of the beginning of the secondary copy of the descriptor
     area to the block following the secondary status area */

fstavailsctr = lvm_rec -> vgda_psn [LVM_SCNDRY] + lvm_rec -> vgda_len;
  /* set the value for first sector available for allocating to the user
     area to the sector following the area reserved for the secondary
     copy of the descriptor area */

lvm_rec -> lvmarea_len = fstavailsctr - PSN_NONRSRVD;
  /* set the length in sectors of the LVM reserved area to the size from
     beginning of the LVM reserved area to the first sector available for
     allocating */

lvm_rec -> reloc_len = LVM_RELOC_LEN;
  /* set value of length in number of sectors of the bad block relocation
     pool */

lvm_rec -> reloc_psn = data_capacity - lvm_rec -> reloc_len - 1;
  /* calculate the beginning physical sector number of the bad block
     relocation pool, which will be located at the end of the physical
     volume, by subracting the relocation pool length in blocks from the
     number of sectors contained on the physical volume */

lvm_rec -> pp_size = ppsize;
  /* set the value for the physical partition size from the value passed
     in by the user */

return;
  /* return to caller */

} /* END lvm_initlvmrec */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_instsetup                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function opens the physical volume and reads in the          *
 *   IPL record and the LVM record.  It also queries the disk to       *
 *   get the disk data capacity.                                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     pv_name                                                         *
 *     override                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     cur_vg_id                                                       *
 *     pv_fd                                                           *
 *     ipl_rec                                                         *
 *     lvm_rec                                                         *
 *     data_capacity                                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_INV_DEVENT                                                  *
 *     LVM_PVOPNERR                                                    *
 *     LVM_VGMEMBER                                                    *
 *     LVM_MEMACTVVG                                                   *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_instsetup (

struct unique_id * vg_id,
  /* id of the volume group into which the PV is to be installed */

char * pv_name,
  /* a pointer to the name of the physical volume to be added to the
     volume group */

short int override,
  /* flag for which a true value indicates to override a VG member error,
     if it occurs, and install the physical volume into the indicated
     volume group */

struct unique_id * cur_vg_id,
  /* structure in which to return the volume group id, if this PV's
     LVM record indicates it is already a member of a volume group */

int * pv_fd,
  /* a pointer to where the file descriptor for the physical volume
     device will be stored */

IPL_REC  *ipl_rec,
  /* a pointer to the block into which the IPL record will be read */

struct lvm_rec * lvm_rec,
  /* a pointer to the block into which the LVM information record will
     be read */

long * data_capacity)
  /* the data capacity of the disk in number of sectors */


{ /* BEGIN lvm_instsetup */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

int member;
  /* flag to indicate if the PV is a member of the specified volume
     group */

int mapf_mode;
  /* mode for open of mapped file */

int map_fd;
  /* file descriptor of mapped file */

caddr_t map_ptr;
  /* pointer to the mapped file */

caddr_t vgda_ptr;
  /* a pointer to the beginning of the volume group descriptor area */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

struct devinfo devinfo;
  /* structure in which device information will be returned */

char devname [LVM_EXTNAME];
  /* the full path name of the physical disk device */

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

retcode = status_chk (NULL, pv_name, NOCHECK, devname);
  /* build the path name for the physical volume device from the name
     passed in */
if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}

*pv_fd = open (devname, O_RDWR | O_NSHARE);
  /* open the specified disk for I/O */

if (*pv_fd == LVM_UNSUCC)
  /* if an error occurred */

    return (LVM_PVOPNERR);
      /* return with error code to indicate that the physical volume
	 could not be opened */

retcode = ioctl (*pv_fd, IOCINFO, (char *) &devinfo);
  /* call I/O control routine with request to get device information */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    close (*pv_fd);
      /* close the physical volume */

    return (LVM_INV_DEVENT);
      /* return error for invalid physical volume device */

}

switch (devinfo.devtype)
  /* look at the device type */

{

  case DD_DISK:
    /* if the device type is disk */

    if (devinfo.un.dk.bytpsec != DBSIZE)
      /* if the block size for this device is not the standard block
	 size expected by the logical volume device driver */

    {

	close (*pv_fd);
	  /* close the physical volume */

	return (LVM_INV_DEVENT);
	  /* return error code for invalid physical volume device */

    }

    *data_capacity = devinfo.un.dk.numblks;
      /* set data capacity in number of blocks from the returned device
	 information */

    break;

  /* if the device type is SCSI disk or read/write optical device */
  case DD_SCDISK:
  case DD_SCRWOPT:

    if (devinfo.un.scdk.blksize != DBSIZE)
      /* if the block size for this device is not the standard block
	 size expected by the logical volume device driver */

    {

	close (*pv_fd);
	  /* close the physical volume */

	return (LVM_INV_DEVENT);
	  /* return error code for invalid physical volume device */

    }

    *data_capacity = devinfo.un.scdk.numblks;
      /* set data capacity in number of blocks from the returned device
	 information */

    break;

  default:
    /* for any other device type */

    close (*pv_fd);
	/* close the physical volume */

    return (LVM_INV_DEVENT);
      /* return error code for invalid physical volume device */

} /* look at device type */

retcode = lvm_rdiplrec (pv_name, *pv_fd, ipl_rec);
  /* call routine to read the IPL record from the physical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (*pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with unsuccessful return code */

}

retcode = lvm_rdlvmrec ((int) (*pv_fd), (struct lvm_rec *) lvm_rec);
  /* call routine to read the LVM record from the physical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (*pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with unsuccessful return code */

}

if (lvm_rec -> lvm_id == LVM_LVMID)
  /* if the LVM id field in the LVM record indicates that this PV is
     already installed into a volume group */

{

    if (vg_id -> word1 != lvm_rec -> vg_id.word1  ||
	vg_id -> word2 != lvm_rec -> vg_id.word2)
      /* if the VG id for the volume group which this PV is installed
	 into does not match that of the volume group into which we
	 are trying to install the PV */

    {

	*cur_vg_id = lvm_rec -> vg_id;
	  /* store the volume group id from the LVM record for this PV
	     into a variable to be returned to the user */

	if (override == FALSE)
	  /* if override flag which indicates to install the PV anyway
	     is not true */

	{

	    close (*pv_fd);
	      /* close the physical volume */

	    return (LVM_VGMEMBER);
	      /* return error code to indicate this physical volume may
		 be a member of another volume group */

	}

	else
	  /* install PV even though LVM record indicates VG member */

	{

	    retcode = lvm_chkvaryon (cur_vg_id);
	      /* call routine to check if the volume group which this PV
		 may be a member of is currently varied on */

	    if (retcode == LVM_SUCCESS)
	      /* if volume group is varied on */

	   {

	       mapf_mode = O_RDONLY | O_NSHARE;
		 /* mode for opening of mapped file of VG to be checked */

	       retcode = lvm_getvgdef (cur_vg_id, mapf_mode, &map_fd,
				       &map_ptr);
		 /* open mapped file for this volume group so we can
		    check its VGDA to see if the PV is really a member */

	       if (retcode < LVM_SUCCESS)
		 /* if an error occurred */

	       {

		   close (*pv_fd);
		     /* close the physical volume */

		   return (LVM_MEMACTVVG);
		     /* be conservative and return error for PV is a
			member of a currently varied on VG since we were
			unable to do further checking */

	       }

	       vgda_ptr = map_ptr + sizeof (struct fheader);
		 /* set pointer to beginning of VGDA for VG which the
		    PV may be a member of */

	       member = lvm_vgmem (&(ipl_rec->pv_id), vgda_ptr);
		 /* call routine to check this VG's descriptor area to
		    determine if PV is really a member */

	       if (member == TRUE)
		 /* if the PV is really a member of this other volume
		    group, which is currently varied on */

	       {

		   free (map_ptr);
		     /* free space allocated for this mapped file */

		   close (map_fd);
		     /* close mapped file for the VG to which this PV
			belongs */

		   close (*pv_fd);
		     /* close the physical volume */

		   return (LVM_MEMACTVVG);
		     /* return error for PV is a member of a currently
			varied on volume group */

	       }

	       free (map_ptr);
		 /* free space allocated for this mapped file */

	       retcode = close (map_fd);
		 /* close mapped file for VG being checked for membership
		    of the PV */

	       if (retcode < LVM_SUCCESS)
		 /* if an error occurred */

	       {

		   close (*pv_fd);
		     /* close the physical volume */

		   return (LVM_PROGERR);
		     /* return error code for programming error */

	       }

	   } /* new PV may be member of varied on VG */

	} /* install PV even if VG member */

    } /* VG ids do not match */

} /* LVM record indicates PV may be a VG member */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_instsetup */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_pventry                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function creates the physical volume header entry in the     *
 *   list of physical volumes in the volume group descriptor area.     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_id                                                           *
 *     vghdr_ptr                                                       *
 *     pv_ptr                                                          *
 *     num_parts                                                       *
 *     beg_psn                                                         *
 *     num_vgdas                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     pv_ptr                                                          *
 *                                                                     *
 *     The volume group descriptor area has been updated with a        *
 *     physical volume header entry.                                   *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_pventry (


struct unique_id * pv_id,
  /* pointer to a structure which contains id for the physical volume for
     which the entry is to be created */

struct vg_header * vghdr_ptr,
  /* a pointer to the volume group header of the descriptor area */

struct pv_header ** pv_ptr,
  /* a pointer to the beginning of the list of physical volume entries
     in the descriptor area */

long num_parts,
  /* the number of partitions available on this physical volume */

daddr_t beg_psn,
  /* the physical sector number of the first physical partition on this
     physical volume */

short int num_vgdas)
  /* the number of volume group descriptor areas which are to be placed
     on this physical volume */


{ /* BEGIN lvm_pventry */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

long size;
  /* variable to hold an interim size calculation */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

short int pv_index;
  /* index variable used for looping on physical volumes */

short int pv_number;
  /* index variable used for looping on physical volumes */

short int pvnums [LVM_MAXPVS];
  /* array which contains a flag value of true or false in the ith entry
     to indicate if there is a physical volume with PV number (i+1) */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */



bzero (pvnums, sizeof (pvnums));
  /* initialize the pvnums array to zeroes */

for (pv_number=1; pv_number <= vghdr_ptr->numpvs; pv_number=pv_number+1)
  /* loop for each physical volume in the PV list of the descriptor
     area */

{

  pvnums [(*pv_ptr) -> pv_num - 1] = TRUE;
    /* in pvnums array, set the element represented by index
       (PV number - 1) to true to indicate there is a physical volume
       with this PV number in the volume group */

  size = sizeof (struct pv_header) + (*pv_ptr) -> pp_count *
				       sizeof (struct pp_entries);
    /* find the size of the entry for this physical volume by adding the
       size of the header entry and the size for all the physical
       partition entries */

  pventry_size = LVM_SIZTOBBND (size);
    /* round the size for this PV entry up to be a multiple of the block
       size */

  *pv_ptr = (struct pv_header *) ((caddr_t) *pv_ptr + pventry_size);
    /* set a pointer to the next entry in the list of physical volumes */

} /* loop for each PV in the VGDA */


/************************************************************************
 *   Note that at the end of the above loop, pv_ptr points past the end *
 *   of the current list of PV entries and is pointing at the space in  *
 *   the descriptor area where the next PV entry will be added to the   *
 *   list.                                                              *
 ************************************************************************
 */

for (pv_number = 1; pv_number <= LVM_MAXPVS; pv_number = pv_number + 1)
  /* for PV number from 1 to maximum possible number of PVs, look through
     the pvnums array to find an unused PV number */

{

  if (pvnums [pv_number - 1] == FALSE)
    /* if this entry of the pvnums array is false, indicating that there
       is not a physical volume in the volume group with this PV number */

  {

      (*pv_ptr) -> pv_num = pv_number;
	/* store this PV number into the PV number field pointed to by
	   the physical volume header pointer, which now points at an
	   empty entry */

      break;
	/* exit from the for loop once an unused PV number is found */

  }

} /* loop for each possible PV */

(*pv_ptr) -> pv_state = LVM_PVACTIVE;
  /* set the physical volume state in the PV entry to indicate this PV is
     an active member of a volume group */

(*pv_ptr) -> pp_count = num_parts;
  /* set the physical partition count field for this PV entry to the
     value passed in for number of partitions on the physical volume */

(*pv_ptr) -> psn_part1 = beg_psn;
  /* set the physical sector number of the first partition for this PV
     entry to the value passed in */

(*pv_ptr) -> pvnum_vgdas = num_vgdas;
  /* set the value for number of volume group descriptor areas on this
     physical volume to the value passed in */

(*pv_ptr) -> pv_id = *pv_id;
  /* set the PV id field for this PV entry to the value passed in */

vghdr_ptr -> numpvs = vghdr_ptr -> numpvs + 1;
  /* increment the number of physical volumes in the volume group header
     since we have just completed adding a physical volume entry */

return;
  /* return to caller */

} /* END lvm_pventry */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_vgdas3to3                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine updates the volume group descriptor areas for the    *
 *   special case of where the total number of VGDAs will remain at    *
 *   three but the placement of VGDAs for the volume group is changing *
 *   from two copies on one PV and one on another PV to one copy       *
 *   each on three PVs.  This routine may be called when installing    *
 *   a new PV or when returning a "REMOVED" PV.                        *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     lv_fd                                                           *
 *     vgmap_tr                                                        *
 *     new_pv                                                          *
 *     sav_pv_2                                                        *
 *     sav_pv_1                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     An udpated VGDA has been written out to the PVs in the VG.      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_vgdas3to3 (

int lv_fd,
  /* the file descriptor of the LVM reserved area logical volume */

caddr_t vgmap_ptr,
  /* pointer to the beginning of the mapped file */

short int new_pv,
  /* the PV number of the new physical volume which is being added */

short int sav_pv_2,
  /* the PV number of the physical volume which previously had two copies
     of the VGDA */

short int sav_pv_1)
  /* the PV number of the physical volume which previously had one copy
     of the VGDA */


{ /* BEGIN lvm_vgdas3to3 */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct fheader * maphdr_ptr;
  /* a pointer to the file header of the mapped file */

struct vg_header * vghdr_ptr;
  /* pointer to the volume group header */

struct vg_trailer * vgtrail_ptr;
  /* pointer to the volume group trailer */

caddr_t vgda_ptr;
  /* pointer to the beginning of the volume group descriptor area */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


maphdr_ptr = (struct fheader *) vgmap_ptr;
  /* set a pointer to the file header portion of the mapped file */

vgda_ptr = vgmap_ptr + sizeof (struct fheader);
  /* set a pointer to the beginning of the volume group descriptor
     area */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set pointer to volume group header */

vgtrail_ptr = (struct vg_trailer *) (vgda_ptr + (vghdr_ptr -> vgda_size)
				     * DBSIZE - DBSIZE);
  /* set pointer to volume group trailer */


/***********************************************************************
 *   Write one copy of the VGDA to the PV which previously had two     *
 *   copies.                                                           *
 ***********************************************************************
 */

retcode = lvm_wrtnext (lv_fd, vgda_ptr, &(vgtrail_ptr -> timestamp),
	    sav_pv_2, maphdr_ptr, (short int) LVM_DASPERPVGEN);
  /* call routine to write the VGDA to the VGDA copy with oldest
     timestamp for the PV which previously had 2 VGDA copies */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    return (retcode);
      /* return to user with error */

}


/***********************************************************************
 *   Write one copy of the VGDA to the PV which previously had one     *
 *   copy.                                                             *
 ***********************************************************************
 */

retcode = lvm_wrtnext (lv_fd, vgda_ptr, &(vgtrail_ptr -> timestamp),
	    sav_pv_1, maphdr_ptr, (short int) LVM_DASPERPVGEN);
  /* call routine to write the VGDA to the VGDA copy with oldest
     timestamp for the PV which previously had 1 VGDA copy */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    return (retcode);
      /* return to user with error */

}

/***********************************************************************
 *   Write one copy of the VGDA to the new or returned PV.             *
 ***********************************************************************
 */

retcode = lvm_wrtnext (lv_fd, vgda_ptr, &(vgtrail_ptr -> timestamp),
	    new_pv, maphdr_ptr, (short int) LVM_DASPERPVGEN);
  /* call routine to write the VGDA to the VGDA copy with oldest
     timestamp for the new or returned PV */

if (retcode < LVM_SUCCESS)
  /* if an error occured */

{

    return (retcode);
      /* return to user with error */

}

return (LVM_SUCCESS);
  /* return to caller */

} /* END lvm_vgdas3to3 */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_vgmem                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function determines if a specified physical volume is a      *
 *   member of a specified volume group.                               *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_id                                                           *
 *     vgda_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     None                                                            *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   TRUE     If the PV is a member of the specified volume group.     *
 *   FALSE    If the PV is not a member of the specified volume group. *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_vgmem (

struct unique_id * pv_id,
  /* pointer to a structure which contains id of the physical volume
     for which we are to determine membership in the specified VG */

caddr_t vgda_ptr)
  /* a pointer to the beginning of the volume group descriptor area */


{ /* BEGIN lvm_vgmem */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

int member;
  /* flag to indicate if the PV is a member of the specified volume
     group */

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



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


member = FALSE;
  /* set flag to indicate PV is not a member of the specified VG */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

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

  if (pv_ptr -> pv_id.word1 == pv_id -> word1  &&
      pv_ptr -> pv_id.word2 == pv_id -> word2)
    /* if this PV from the descriptor area has a PV id which matches
       that for which we are searching */

  {

      member = TRUE;
	/* set flag to indicate that we have found the specified PV
	   to be a member of the volume group */

      break;
	/* break from loop since we have found the searched for PV */

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

} /* loop for PVs in the VGDA */

return (member);
  /* return flag value which indicates if specified PV is a member of
     the specified volume group */

} /* END lvm_vgmem */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_zeromwc                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes out a zeroed out mirror write cache recovery  *
 *   area.                                                             *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A zeroed out mirror write cache recovery area is written.       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_WRTDAERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_zeromwc (

int pv_fd,
  /* the file descriptor of the physical volume on which to write the
     mirror write cache recovery area */

short int newvg)
  /* flag to indicate if this is a newly created volume group */


{ /* BEGIN lvm_zeromwc */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* offset from the beginning of the file where the file pointer will
     be located */

struct timestruc_t cur_time;
  /* a structure which contains the current time from the system clock */

caddr_t mwc_rec [DBSIZE];
  /* the mirror write cache record */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


bzero (mwc_rec, DBSIZE);
  /* zero out the mirror write cache record */

if (newvg == TRUE)
  /* if this PV is being put in a newly created volume group */

{

    retcode = gettimer (TIMEOFDAY, &cur_time);
      /* get the current time from the system clock */

    if (retcode == LVM_UNSUCC)
      /* if an error occurred */

    {

	return (LVM_PROGERR);
	  /* return error code for programming error */

    }

    ((struct mwc_rec *) mwc_rec) -> b_tmstamp = cur_time;
      /* set beginning timestamp value of the mirror write cache record */

    ((struct mwc_rec *) mwc_rec) -> e_tmstamp = cur_time;
      /* set ending timestamp value of the mirror write cache record */

} /* newly created volume group */

offset = PSN_MWC_REC0 * DBSIZE;
  /* get the byte offset for the first mirror write cache record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* call system routine to position the file pointer at the first mirror
     write cache record (written to the physical volume device) */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with error code for programming error */

}

retcode = writex (pv_fd, mwc_rec , DBSIZE, WRITEV);
  /* write the mirror write cache record */

if (retcode == LVM_UNSUCC)
  /* if an error occurred on the write */

{

    if (errno == EMEDIA || errno == ESOFT)
      /* if the error might be because of a bad block */

    {

	retcode = lvm_relocmwcc (pv_fd, mwc_rec);
	  /* call routine to try to hardware relocate the block containing
	     the MWCC */

	if (retcode < LVM_SUCCESS)
	  /* if mirror write consistency cache could not be written at a
	     relocated block */

	{

	    return (LVM_WRTDAERR);
	      /* return error for write to reserved area failed */

	}
    }

    else
      /* write error which cannot be bad block */

    {

	return (LVM_WRTDAERR);
	  /* return error for write to reserved area failed */

    }

} /* unsuccessful write */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_zeromwc */










/***********************************************************************
 *                                                                     *
 * NAME:  lvm_zerosa                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes out a zeroed out volume group status area     *
 *   with beginning and ending timestamp filled in.                    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     lv_fd                                                           *
 *     sa_lsn                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     A zeroed out volume group status area (with timestamps) is      *
 *     written at the specified logical sector.                        *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_WRTDAERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_zerosa (

int lv_fd,
  /* the file descriptor for the LVM reserved area logical volume */

daddr_t sa_lsn [LVM_PVMAXVGDAS])
  /* the logical sector numbers within the LVM reserved area logical
     volume of where to initialize the copies of the volume group status
     area */


{ /* BEGIN lvm_zerosa */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* offset from the beginning of the file where the file pointer will
     be located */

short int sa_index;
  /* index for looping on volume group status area */

struct timestruc_t cur_time;
  /* a structure which contains the current time from the system clock */

struct vgsa_area vgsa;
  /* the volume group status area */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


retcode = gettimer (TIMEOFDAY, &cur_time);
  /* get the current time from the system clock */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return error code for programming error */

}

bzero ((caddr_t) &vgsa, sizeof (struct vgsa_area));
  /* zero out the volume group status area */

vgsa.b_tmstamp = cur_time;
vgsa.e_tmstamp = cur_time;
  /* set beginning and ending timestamp values in the volume group
     status area to the current time */

for (sa_index = 0; sa_index < LVM_PVMAXVGDAS; sa_index = sa_index + 1)
  /* loop for each copy of the volume group status area on this disk */

{

  offset = sa_lsn [sa_index] * DBSIZE;
    /* get the byte offset from the beginning of the LVM reserved area
       logical volume of this copy of the volume group status area */

  offset = lseek (lv_fd, offset, LVM_FROMBEGIN);
    /* call system routine to position the file pointer at the start of
       this copy of the VGSA */

  if (offset == LVM_UNSUCC)
    /* if an error occurred */

  {

      return (LVM_PROGERR);
	/* return with error code for programming error */

  }

  retcode = write (lv_fd, (caddr_t) &vgsa , sizeof (struct vgsa_area));
    /* write this copy of the volume group status area (written to the
       volume group reserved area logical volume device) */

  if (retcode != sizeof (struct vgsa_area))
    /* if write was unsuccessful */

  {

      return (LVM_WRTDAERR);
	/* return error for write to reserved area failed */

  }

} /* loop for each VGSA copy on the PV */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_zerosa */
