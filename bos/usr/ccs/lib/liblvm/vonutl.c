static char sccsid[] = "@(#)39	1.2  src/bos/usr/ccs/lib/liblvm/vonutl.c, liblvm, bos411, 9428A410j 3/16/93 09:00:43";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - vonutl.c
 *
 * FUNCTIONS: lvm_clsinpvs,
 *            lvm_deladdm,
 *            lvm_vonresync
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
 * NAME:  lvm_clsinpvs                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to close open PVs from the input list.      *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     inpvs_info                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_clsinpvs (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

struct inpvs_info * inpvs_info)
  /* structure which contains information about the input list of PVs
     for the volume group */


{ /* BEGIN lvm_clsinpvs */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

short int pv_index;
  /* loop index for physical volumes */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


for (pv_index=0; pv_index<varyonvg->vvg_in.num_pvs; pv_index=pv_index+1)
  /* loop for each PV in the input list */

{

  if (inpvs_info -> pv[pv_index].fd != LVM_FILENOTOPN)
    /* if this PV is open */

  {

      close (inpvs_info -> pv[pv_index].fd);
	/* close the physical volume */

  }

} /* loop for each PV in input list */

return;
  /* return to caller */

} /* END lvm_clsinpvs */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_deladdm                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function will delete a PV from the kernel and add it back    *
 *   as a missing PV.                                                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     pv_num                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
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
lvm_deladdm (

struct varyonvg * varyonvg,
  /* pointer to structure which contains the input information for
     the lvm_varyonvg subroutine */

struct inpvs_info * inpvs_info,
  /* pointer to structure which contains information about the physical
     volumes in the input list */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */

short int pv_num)
  /* the PV number of the PV which is to be changed to a missing PV in
     the kernel */


{  /* BEGIN lvm_deladdm */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

short int in_index;
  /* index into the user input list */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


lvm_delpv (&(varyonvg -> vg_id), varyonvg -> vg_major, pv_num,
	   inpvs_info -> num_desclps, HD_KDELPV, (short int) 0);
  /* delete from the kernel the PV which needs to be changed to a missing
     PV */

retcode = lvm_addmpv (&(varyonvg -> vg_id), varyonvg -> vg_major, pv_num);
  /* call routine to add this PV back into the kernel as a missing PV */

if (retcode < LVM_SUCCESS)
  /* if the missing PV could not be successfully added */

{

    return (retcode);
      /* return with error code */

}

defpvs_info -> pv [pv_num-1].pv_status = LVM_NOTDFND;
  /* set status to indicate that the PV with this PV number is not
     defined into the kernel (i.e., is not active) */

in_index = defpvs_info -> pv [pv_num-1].in_index;
  /* get index into the input list for this PV */

varyonvg -> vvg_out.pv [in_index].pv_status = LVM_PVMISSING;
  /* update the status in the varyonvg output structure to indicate that
     this PV is missing */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_deladdm */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_vonresync                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function will resync all the logical volumes in the volume   *
 *   group.                                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_RESYNC_FAILED                                               *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_vonresync (

struct unique_id * vg_id)
  /* pointer to the volume group id */


{  /* BEGIN lvm_vonresync */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct queryvg * queryvg;
  /* pointer to the query volume group structure */

struct lv_array * lv_ptr;
  /* pointer to array containing information about a logical volume within
     the volume group */

int retcode;
  /* return code */

int savret;
  /* variable in which to save a return value */

short int lv_idx;
  /* index for logical volumes */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


retcode = lvm_queryvg (vg_id, &queryvg, NULL);
  /* call routine to query the volume group to get information about its
     logical volumes */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    return (LVM_RESYNC_FAILED);
      /* return positive return code to indicate that the resync of the
	 volume group has failed */

}

savret = LVM_SUCCESS;
  /* initialize saved return code value to successful */

lv_ptr = queryvg -> lvs;
  /* set pointer to the array of LV information for the first logical
     volume in the list */

for (lv_idx = 0; lv_idx < queryvg -> num_lvs; lv_idx = lv_idx + 1)
  /* loop for number of logical volumes in the volume group */

{

  retcode = lvm_resynclv (&(lv_ptr -> lv_id), FALSE);
    /* call resync routine to resynchronize this logical volume */

  if (retcode < LVM_SUCCESS)
    /* if an error occurred on this resync */

  {

      savret = LVM_RESYNC_FAILED;
	/* set saved return value to indicate that the volume group could
	   not be completely resynchronized */

  }

  lv_ptr = (struct lv_array *) ((caddr_t) lv_ptr + sizeof
						    (struct lv_array));
    /* set pointer to information array for the next logical volume in
       the list */

}
lvm_freebuf(queryvg);

return (savret);
  /* return with saved return code value */

} /* END lvm_vonresync */
