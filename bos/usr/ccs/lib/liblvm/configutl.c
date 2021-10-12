static char sccsid[] = "@(#)33	1.2.1.1  src/bos/usr/ccs/lib/liblvm/configutl.c, liblvm, bos411, 9428A410j 7/20/92 12:01:40";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - configutl.c
 *
 * FUNCTIONS: lvm_addmpv,
 *            lvm_addpv,
 *            lvm_chgqrm,
 *            lvm_chgvgsa,
 *            lvm_chkvgstat,
 *            lvm_config,
 *            lvm_defvg,
 *            lvm_delpv,
 *            lvm_delvg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <sys/types.h>;
#include <sys/lockl.h>;
#include <sys/sleep.h>;
#include <sys/buf.h>;
#include <sys/bootrecord.h>;
#include <sys/sysconfig.h>;
#include <errno.h>;
#include <sys/hd_psn.h>;
#include <lvmrec.h>;
#include <sys/dasd.h>;
#include <sys/hd_config.h>;
#include <liblvm.h>;









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_addmpv                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function adds a physical volume structure for a missing      *
 *   physical volume into the kernel.                                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     vg_major                                                        *
 *     pv_num                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_addmpv (


struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */

long vg_major,
  /* the major number where the volume group is to be added */

short int pv_num)
  /* number of the PV to be deleted from the volume group */


{ /* BEGIN lvm_addmpv */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   adding a missing physical volume into the kernel.                 *
 ***********************************************************************
 */

cfgdata.parmlist.kaddmpv.vg_id = *vg_id;
  /* store volume group id in configuration data structure */

cfgdata.parmlist.kaddmpv.vg_major = (short int) vg_major;
  /* store major number of the volume group */

cfgdata.parmlist.kaddmpv.pv_num = pv_num;
  /* store number of PV to be deleted */

retcode = lvm_config (NULL, vg_major, HD_KADDMPV, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

return (retcode);
  /* return with return code */

} /* END lvm_addmpv */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_addpv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function builds the data structures for adding a physical    *
 *   volume into a volume group and calls the kernel configuration     *
 *   routine which adds the physical volume.                           *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     partlen_blks                                                    *
 *     num_desclps                                                     *
 *     device                                                          *
 *     pv_fd                                                           *
 *     pv_num                                                          *
 *     *vg_id                                                          *
 *     reloc_psn                                                       *
 *     reloc_len                                                       *
 *     psn_part1                                                       *
 *     vgsa_lsn                                                        *
 *     quorum_cnt                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The data structures to be added to the kernel for this          *
 *     physical volume have been built.                                *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_ALLOCERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_addpv (


long partlen_blks,
  /* the length of a partition in number of 512 byte blocks */

short int num_desclps,
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */

dev_t device,
  /* the major / minor number of the device */

int pv_fd,
  /* the file descriptor of the physical volume device */

short int pv_num,
  /* the index number for this physical volume */

long vg_major,
  /* the major number of the volume group */

struct unique_id * vg_id,
  /* the volume group id of the volume group to which the physical
     volume is to be added */

daddr_t reloc_psn,
  /* the physical sector number of the beginning of the bad block
     relocation pool */

long reloc_len,
  /* the length of the bad block relocation pool */

daddr_t psn_part1,
  /* the physical sector number of the beginning of the first partition
     on the physical volume */

daddr_t vgsa_lsn [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of the VGSA copies on this
     PV */
short int quorum_cnt)
  /* the number of VGDAs/VGSAs needed for a quorum */


{ /* BEGIN lvm_addpv */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

long size;
  /* variable to hold a temporary size calculation */

short int pp_index;
  /* loop index used for physical partitions */

short int hash_index;
  /* loop index used for bad block hash value */

short int sa_index;
  /* loop index used for VGSAs */

struct pvol * pvol_ptr;
  /* a pointer to the LVDD physical volume structure for the PV being
     installed */

struct part * lplist_ptr;
  /* a pointer to the list of LVDD partition structures which are to be
     added to the descriptor logical volume for this PV */

struct part * lp_ptr;
  /* a pointer to the LVDD partition structure for a particular logical
     partition */

struct bad_blk * bb_ptr;
  /* a pointer to an LVDD bad block structure */

struct bad_blk * save_ptr;
  /* variable for temporarily saving pointer to a bad block structure */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/************************************************************************
 *   Build the LVDD physical volume structure and any necessary bad     *
 *   block structures which will be added to the kernel to describe     *
 *   this physical volume.                                              *
 ************************************************************************
 */

pvol_ptr = (struct pvol *) malloc (sizeof (struct pvol));
  /* allocate space for an LVDD physical volume structure */

if (pvol_ptr == NULL)
  /* if an error occurred */

{

    return (LVM_ALLOCERR);
      /* return with error code for memory allocation error */

}

bzero ((caddr_t) pvol_ptr, sizeof (struct pvol));
  /* zero out the physical volume structure */

pvol_ptr -> defect_tbl = (struct defect_tbl *)
			    malloc (sizeof (struct defect_tbl));
  /* allocate space for the defect table for this physical volume */

if (pvol_ptr -> defect_tbl == NULL)
  /* if an error occurred */

{

    return (LVM_ALLOCERR);
      /* return with error code for memory allocation error */

}

bzero (pvol_ptr -> defect_tbl, sizeof (struct defect_tbl));
  /* zero out the defect table for this physical volume */

pvol_ptr -> dev = device;
  /* store the device identification into the LVDD physical volume
     structure */

pvol_ptr -> pvnum = pv_num - 1;
  /* store the PV number (as a base 0 value for use in array indexing)
     into the LVDD physical volume structure */

pvol_ptr -> vg_num = (short int) vg_major;
  /* store the volume group major number into the LVDD physical volume
     structure */

pvol_ptr -> beg_relblk = reloc_psn;
  /* store the beginning block number of the bad block relocation pool
     into the LVDD physical volume structure */

pvol_ptr -> max_relblk = reloc_psn + reloc_len - 1;
  /* store the last block number in the bad block relocation pool into
     the LVDD physical volume structure as the maximum allowed relocation
     block number */

pvol_ptr -> fst_usr_blk = psn_part1;
  /* store value for the physical sector number of the first partition
     on the physical volume */

pvol_ptr -> pv_pbuf.pb.b_event = EVENT_NULL;
  /* initialize the event variable in the pbuf structure for this PV
     which is contained within the LVDD physical volume structure */

for (sa_index = 0; sa_index < LVM_PVMAXVGDAS; sa_index = sa_index + 1)
  /* loop for possible VGSA copies on this PV */

{

  pvol_ptr -> sa_area [sa_index].lsn = vgsa_lsn [sa_index];
    /* in the pbuf structure initialize logical sector number for this
       VGSA copy from the value passed in */

}

retcode = lvm_bldbblst (pv_fd, pvol_ptr, reloc_psn);
  /* call routine to build the LVDD bad block structures for this
     physical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    if (retcode == LVM_PVRDRELOC)
      /* if error indicates read only relocation is necessary */

    {

	pvol_ptr -> pvstate = PV_RORELOC;
	  /* set read only relocation on in the PV state field */


    }

    else

    {

	return (retcode);
	  /* return with error return code */

    }

}


/************************************************************************
 *   Build the LVDD physical partition structures necessary to describe *
 *   the LVM reserved area on this physical volume.  These will be      *
 *   added to the kernel description of the descriptor logical volume   *
 *   for this volume group.                                             *
 ************************************************************************
 */

size = num_desclps * sizeof (struct part);
  /* calculate the size of memory needed to hold the LVDD physical
     partition structures for the LVM reserved area on this physical
     volume */

lplist_ptr = (struct part *) malloc (size);
  /* allocate memory for the physical partition structures */

if (lplist_ptr == NULL)
  /* if an error occurred */

{

    return (LVM_ALLOCERR);
      /* return error code for memory allocation error */

}

bzero ((caddr_t) lplist_ptr, size);
  /* initialize the list of partition structures to zeroes */

lp_ptr = lplist_ptr;
  /* set a pointer to the beginning of the list of LVDD partition
     structures which are to be added to the descriptor area logical
     volume */

for (pp_index = 1; pp_index <= num_desclps; pp_index = pp_index + 1)
  /* loop for the number of physical partitions which it takes to contain
     the LVM reserved area */

{

  lp_ptr -> start = (pp_index - 1) * partlen_blks;
    /* initialize the starting address for this physical partition based
       on its index number */

  lp_ptr -> sync_trk = NO_SYNCTRK;
    /* initialize the sync track field for not being synced */

  lp_ptr = (struct part *) ((caddr_t) lp_ptr + sizeof (struct part));
    /* increment pointer to point at the next logical partition in the
       list of LVDD logical partition structures */

}


/************************************************************************
 *   Call the configuration device driver to add information about this *
 *   PV to the kernel data structures for this volume group.            *
 ************************************************************************
 */

cfgdata.parmlist.kaddpv.vg_id = *vg_id;
  /* store the volume group id into the input data structure */

cfgdata.parmlist.kaddpv.pv_ptr = pvol_ptr;
  /* store pointer to the LVDD physical volume structure into the input
     data structure */

cfgdata.parmlist.kaddpv.lp_ptr = lplist_ptr;
  /* store pointer to the LVDD logical partition structures into the
     input data structure */

cfgdata.parmlist.kaddpv.pv_num = pv_num;
  /* store the number of the physical volume into the input data
     structure */

cfgdata.parmlist.kaddpv.num_desclps = num_desclps;
  /* store the number of partitions which are to be added to the
     descriptor area logical volume into the input data structure */

cfgdata.parmlist.kaddpv.quorum_cnt = quorum_cnt;
  /* store the number of VGDAs/VGSAs needed for a quorum into the input
     data structure */

retcode = lvm_config (NULL, vg_major, HD_KADDPV, &cfgdata);
  /* call routine which will call kernel configuration routines to
     add information to the kernel data structures */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}


/************************************************************************
 *   Free space allocated for LVDD structures.                          *
 ************************************************************************
 */

for (hash_index = 0; hash_index < HASHSIZE; hash_index = hash_index + 1)
  /* loop for each hash value in the defects array of the physical volume
     structure */

{

  bb_ptr = pvol_ptr -> defect_tbl -> defects [hash_index];
    /* set bad block pointer equal to value stored in this entry of
       defects array */

  while (bb_ptr != NULL)
    /* loop through the linked list until null end pointer is reached */

  {

    save_ptr = bb_ptr;
      /* save pointer to this bad block to use for free */

    bb_ptr = bb_ptr -> next;
      /* set the bad block pointer to the next bad block in the linked
	 list */

    free ((caddr_t) save_ptr);
      /* free space allocated for the bad block structure */

  }

} /* loop for each hash value in defects arrray */

free ((caddr_t) pvol_ptr -> defect_tbl);
  /* free space allocated for the defect table */

free ((caddr_t) pvol_ptr);
  /* free space allocated for the physical volume structure */

free ((caddr_t) lplist_ptr);
  /* free space allocated for the descriptor area logical volume
     partition structures */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_addpv */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chgqrm                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function changes the quorum value in the kernel and is used  *
 *   by lvm_changepv when returning a PV which cannot be made active.  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     vg_major                                                        *
 *     quorum_cnt                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chgqrm (


struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */

long vg_major,
  /* the major number where the volume group is to be added */

short int quorum_cnt)
  /* number of VGDA/VGSA copies needed for a quorum */


{ /* BEGIN lvm_chgqrm */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   changing the quorum count.                                        *
 ***********************************************************************
 */

cfgdata.parmlist.kchgqrm.vg_id = *vg_id;
  /* store volume group id in configuration data structure */

cfgdata.parmlist.kchgqrm.quorum_cnt = quorum_cnt;
  /* store value for new quorum count */

retcode = lvm_config (NULL, vg_major, HD_KCHGQRM, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

return (retcode);
  /* return with return code */

} /* END lvm_chgqrm */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chgvgsa                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function sets up data for and calls the kernel config        *
 *   routine which will either add or detele volume group status       *
 *   areas from the specifed PV.                                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     vg_major                                                        *
 *     vgsa_lsn                                                        *
 *     quorum_cnt                                                      *
 *     pv_num                                                          *
 *     flag                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *                                                                     *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chgvgsa (

struct unique_id * vg_id,
  /* the volume group id */

long vg_major,
  /* the major number of the volume group */

daddr_t vgsa_lsn [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of the VGSA copies on this
     PV */

short int pv_num,
  /* number of the PV which is to have changes to the number of VGSAs */

short int quorum_cnt,
  /* number of VGDAs/VGSAs needed for a quorum */

int command)
  /* command value which indicates the config routine to be called is
     that for adding/deleting VGSAs */

{ /* BEGIN lvm_chgvgsa */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

short int sa_index;
  /* index for VGSA copies */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   adding or deleting VGSA copies from a PV.                         *
 ***********************************************************************
 */

cfgdata.parmlist.kchgvgsa.vg_id = *vg_id;
  /* store volume group id in configuration data structure */

cfgdata.parmlist.kchgvgsa.pv_num = pv_num;
  /* store number of PV which is having VGSAs changed */

cfgdata.parmlist.kchgvgsa.quorum_cnt = quorum_cnt;
  /* store the new quorum count */

for (sa_index = 0; sa_index < LVM_PVMAXVGDAS; sa_index = sa_index + 1)
  /* loop for possible VGSA copies on this PV */

{

  cfgdata.parmlist.kchgvgsa.sa_lsns [sa_index] = vgsa_lsn [sa_index];
    /* store logical sector number value for this VGSA copy */

}

retcode = lvm_config (NULL, vg_major, command, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

return (retcode);
  /* return to caller */

} /* END lvm_chgvgsa */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chkvgstat                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function determines if the specified volume group is defined *
 *   into the kernel, and, if so, whether the volume group is fully    *
 *   varied on or varied on for system management only.                *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *vgstatus                                                       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chkvgstat (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     lvm_varyonvg */

int * vgstatus)
  /* pointer to variable to contain the varied on status of the volume
     group */


{ /* BEGIN lvm_chkvgstat */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   determining the status of the specified volume group.             *
 ***********************************************************************
 */

cfgdata.parmlist.kvgstat.vg_id = varyonvg -> vg_id;
  /* store the volume group id of the volume group which is to be
     varied on */

retcode = lvm_config (varyonvg -> kmid, 0, HD_KVGSTAT, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}

if (cfgdata.parmlist.kvgstat.status != HD_NOTVON  &&
    cfgdata.parmlist.kvgstat.vg_major != varyonvg -> vg_major)
  /* if the status returned from the kernel indicates that the volume
     group is already varied on, but if it is varied on at a major
     number different from that requested for this varyon */

{

    return (LVM_VONOTHMAJ);
      /* return error for volume group already varied on at another major
	 number since it is not allowed for the volume group to be varied
	 on under two major numbers */

}

*vgstatus = cfgdata.parmlist.kvgstat.status;
  /* store value for varied on status of the volume group */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_chkvgstat */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_config                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine calls the sysconfig system call in order to add      *
 *   information to the LVDD kernel data structures for this volume    *
 *   group.                                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     kmid                                                            *
 *     vg_major                                                        *
 *     request                                                         *
 *     cfgdata                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCCESS                                                    *
 *     LVM_INVCONFIG                                                   *
 *     LVM_INVLPRED                                                    *
 *     LVM_INRESYNC                                                    *
 *     LVM_MAJINUSE                                                    *
 *     LVM_FORCEOFF                                                    *
 *     LVM_LVOPEN                                                      *
 *     LVM_ALLOCERR                                                    *
 *                                                                     *
 ***********************************************************************
 */



int
lvm_config (

mid_t kmid,
  /* the module id for the object module which contains the logical
     volume device driver */

long vg_major,
  /* the major number of the volume group */

int request,
  /* the request for the configuration routine to be called within the
     kernel hd_config routine */

struct ddi_info * cfgdata)
  /* structure to contain the input parameters for the configuration
     device driver */


{ /* BEGIN lvm_config */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct cfg_dd cfg_dd;
  /* structure which contains the input information to be sent to the
     configuration device driver */


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

cfg_dd.kmid = kmid;
  /* set device driver module id field to value passed in */

cfg_dd.cmd = request;
  /* set device driver config command code field to the request parameter
     which indicates which configuration routine is to be called */

cfg_dd.devno = makedev (vg_major, 0);
  /* set dev_t value from major number, minor number */

cfg_dd.ddsptr = (caddr_t) cfgdata;
  /* set the pointer to the device dependent information */

cfg_dd.ddslen = sizeof (struct ddi_info);
  /* set the length of the device dependent information to the size of
     the structure for device dependent info */

cfgdata -> parmlist.kdefvg.rc = LVM_SUCCESS;
  /* initialize return code portion of device dependent data to indicate
     successful return */

retcode = sysconfig (SYS_CFGDD, (void *) &cfg_dd,
		     (int) sizeof(struct cfg_dd));
  /* call the sysconfig system call to configure the LVDD device driver
     with information for this volume group */

if (retcode == CONF_FAIL)
  /* if an error occurred */

{

    switch (errno)
      /* look at errno value passed back from the kernel in order to set
	 the return code */

    {

      case ENOMEM:
	/* errno is for memory allocation error */

	retcode = LVM_ALLOCERR;
	  /* set return code for memory allocation error */

	break;

      case EBUSY:
	/* errno is for device busy error */

	retcode = LVM_LVOPEN;
	  /* set return code for logical volume open error */

	break;

      case EINVAL:
	/* errno is general one for invalid configuration */

	switch (cfgdata -> parmlist.kdefvg.rc)
	  /* look at return code in device dependent data */

	{

	  case CFG_FORCEOFF:
	    /* volume group forced off */

	    retcode = LVM_FORCEOFF;
	      /* set return code to indicate to user that volume group
		 has been forcefully varied off because of a loss of
		 quorum */

	    break;

	  case CFG_MAJUSED:
	    /* major number already used */

	    retcode = LVM_MAJINUSE;
	      /* set error to indicate major number is being used by/
		 another device */

	    break;

	  case CFG_SYNCER:
	    /* resync is in progress */

	    retcode = LVM_INRESYNC;
	      /* set error to indicate that a reduce or extend is invalid
		 because a resync is in progress for the logical
		 partition */

	    break;

	  case CFG_INLPRD:
	    /* invalid physical partition reduction */

	    retcode = LVM_INVLPRED;
	      /* set error to indicate that the physical partition cannot
		 be reduced out because the logical partition would be
		 left with no good copies */

	    break;

	  case CFG_INVVGID:
	    /* volume group id does not match */
	  default:
	    /* unexpected return code */

	    retcode = LVM_INVCONFIG;
	      /* set general return code for device driver configuration
		 error */

	} /* end of switch for device dependent error */

	break;

      default:
	/* other unexpected errno */

	retcode = LVM_INVCONFIG;
	  /* set general return code for device driver configuration
	     error */

    } /* end of switch for system errno */

    return (retcode);
      /* return with error code */

} /* configuration failure */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_config */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_defvg                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function defines the volume group into the kernel by         *
 *   initializing the volume group structure and the descriptor area   *
 *   logical volume structure and adding them into the kernel.         *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     partlen_blks                                                    *
 *     num_desclps                                                     *
 *     kmid                                                            *
 *     vg_major                                                        *
 *     vg_id                                                           *
 *     ppsize                                                          *
 *     noopen_lvs                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     Volume group and descriptor area logical volume data structures *
 *     added into the kernel.                                          *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *       LVM_SUCCESS                                                   *
 *       LVM_ALLOCERR                                                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_defvg (

long partlen_blks,
  /* the length of a partition in number of 512 byte blocks */

short int num_desclps,
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */

mid_t kmid,
  /* the module id which identifies where the LVDD code is loaded */

long vg_major,
  /* the major number where the volume group is to be added */

struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */

short int ppsize,
  /* the physical partition size, represented as a power of 2 of the
     size in bytes, for partitions in this volume group */

long noopen_lvs,
  /* flag to indicate if logical volumes are to be defined into kernel */

long noquorum)

{ /* BEGIN lvm_defvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct volgrp * vg_ptr;
  /* pointer to the structure which will contain volume group information
     for the kernel */

struct lvol * dalv_ptr;
  /* pointer to the structure which will contain information about the
     descriptor area logical volume for the kernel */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Allocate and initialize structure to describe the volume group.   *
 ***********************************************************************
 */

vg_ptr = (struct volgrp *) malloc (sizeof (struct volgrp));
  /* allocate space for the volume group structure in which to send
     information to the kernel */

if (vg_ptr == NULL)
  /* if an error occurred */

{

    return (LVM_ALLOCERR);
      /* return error code for memory allocation error */

}

bzero ((caddr_t) vg_ptr, sizeof (struct volgrp));
  /* zero out the volume group structure */

vg_ptr -> vg_lock = LOCK_AVAIL;
  /* set the volume group lock variable to indicate the lock is
     available */

vg_ptr -> partshift = ppsize - DBSHIFT;
  /* store the log base 2 of the partition size in blocks into the
     volume group kernel structure */

vg_ptr -> major_num = vg_major;
  /* store the major number of where the volume group is to be added */

vg_ptr -> vg_id = *vg_id;
  /* store the volume group id */

vg_ptr -> bcachwait = EVENT_NULL;
vg_ptr -> ecachwait = EVENT_NULL;
  /* initialize the cache wait event variables */

vg_ptr -> config_wait = EVENT_NULL;
  /* initialize the configuration wait event variable */

vg_ptr -> sa_pbuf.pb.b_event = EVENT_NULL;
  /* initialize event variable in the pbuf structure which is reserved
     for use by the VGSA wheel mechanism */

if (noopen_lvs == TRUE)
  /* if logical volumes are not allowed to be opened (i.e., volume group
     is varied on for system management only) */

{

    vg_ptr -> flags = vg_ptr -> flags | VG_SYSMGMT;
      /* set bit on for system management in volume group flags */

}

if (noquorum == TRUE) {
     vg_ptr -> flags |= VG_NOQUORUM;
}

/***********************************************************************
 *   Allocate and initialize structure to describe the descriptor area *
 *   logical volume for this volume group.                             *
 ***********************************************************************
 */

dalv_ptr = (struct lvol *) malloc (sizeof (struct lvol));
  /* allocate space for the logical volume structure in which to send
     information to the kernel about the descriptor area */

if (dalv_ptr == NULL)
  /* if an error occurred */

{

    return (LVM_ALLOCERR);
      /* return error code for memory allocation error */

}

bzero ((caddr_t) dalv_ptr, sizeof (struct lvol));
  /* zero out the descriptor area logical volume structure */

dalv_ptr -> lv_status = LV_CLOSED;
  /* initialize the logical volume status in the kernel data structure
     to closed */

dalv_ptr -> lv_options = LV_WRITEV;
  /* initialize the LV options for write verification */

dalv_ptr -> i_sched = SCH_REGULAR;
  /* initialize the scheduler policy to regular (i.e., there are no
     mirror copies to schedule) */

dalv_ptr -> waitlist = EVENT_NULL;
  /* initialize the event list for quiesce */

dalv_ptr -> nparts = LVM_NOMIRROR;
  /* initialize the number of copies for the descriptor area logical
     volume to indicate no mirrors */

dalv_ptr -> nblocks = num_desclps * partlen_blks * LVM_MAXPVS;
  /* initialize the length of the descriptor area logical volume in
     number of 512 byte blocks */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   defining the volume group into the kernel.                        *
 ***********************************************************************
 */

cfgdata.parmlist.kdefvg.vg_ptr = vg_ptr;
  /* store pointer to volume group structure in the configuration data
     structure */

cfgdata.parmlist.kdefvg.dalv_ptr = dalv_ptr;
  /* store pointer to descriptor area logical volume structure in the
     configuration data structure */

retcode = lvm_config (kmid, vg_major, HD_KDEFVG, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (retcode);
      /* return with error return code */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_defvg */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_delpv                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function deletes a physical volume from the kernel and is    *
 *   called when a physical volume in the volume group is being        *
 *   deleted (HD_KDELPV) or removed (HD_KREMPV) or when a previously   *
 *   active physical volume is being declared missing (HD_KMISSPV).    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     vg_major                                                        *
 *     pv_num                                                          *
 *     num_desclps                                                     *
 *     command                                                         *
 *     quorum_cnt                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_delpv (


struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */

long vg_major,
  /* the major number where the volume group is to be added */

short int pv_num,
  /* number of the PV to be deleted from the volume group */

short int num_desclps,
  /* number of logical partitions in the descriptor / status area
     logical volume for this PV */
int command,
  /* value for command field for call to config routines (possible values
     are HD_KDELPV, HD_KREMPV, and HD_KMISSPV) */

short int quorum_cnt)
  /* new value for the quorum count after this PV is deleted */


{ /* BEGIN lvm_delpv */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   deleting the a physical volume from the kernel.                   *
 ***********************************************************************
 */

cfgdata.parmlist.kdelpv.vg_id = *vg_id;
  /* store volume group id in configuration data structure */

cfgdata.parmlist.kdelpv.pv_num = pv_num;
  /* store number of PV to be deleted */

cfgdata.parmlist.kdelpv.num_desclps = num_desclps;
  /* store number of logical partitions in the descriptor / status area
     logical volume for this PV */

cfgdata.parmlist.kdelpv.quorum_cnt = quorum_cnt;
  /* store number of VGDAs/VGSAs needed for a quorum */

retcode = lvm_config (NULL, vg_major, command, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

return (retcode);
  /* return to caller */

} /* END lvm_delpv */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_delvg                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function deletes the volume group from the kernel and is     *
 *   called when kernel cleanup needs to be done for a volume group    *
 *   major number.                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     vg_major                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_delvg (


struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */

long vg_major)
  /* the major number where the volume group is to be added */


{ /* BEGIN lvm_delvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Initialize configuration data structure and call routine to make  *
 *   call to logical volume device driver configuration routine for    *
 *   deleting the volume group from the kernel.                        *
 ***********************************************************************
 */

cfgdata.parmlist.kdelvg.vg_id = *vg_id;
  /* store volume group id in configuration data structure */

cfgdata.parmlist.kdelvg.lvs_only = FALSE;
  /* store flag to indicate this deletion of the volume group is to be
     complete and will not keep structures for system management */

retcode = lvm_config (NULL, vg_major, HD_KDELVG, &cfgdata);
  /* call routine which sets up for and calls the logical volume device
     driver configuration via the sysconfig system call */

return;
  /* return to caller */

} /* END lvm_delvg */



