static char sccsid[] = "@(#)36	1.55  src/bos/usr/ccs/lib/liblvm/varyonvg.c, liblvm, bos411, 9439A411b 9/26/94 13:36:42";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - varyonvg.c
 *
 * FUNCTIONS: lvm_varyonvg,
 *            lvm_forceqrm,
 *            lvm_mapfile,
 *            lvm_pvstatus,
 *            lvm_update
 *
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
#include <sys/shm.h>
#include <sys/sysmacros.h>
#include <sys/dasd.h>
#include <sys/hd_psn.h>
#include <sys/bootrecord.h>
#include <lvmrec.h>
#include <sys/hd.h>
#include <sys/bbdir.h>
#include <sys/vgsa.h>
#include <sys/hd_config.h>
#include <liblvm.h>
#include <sys/errids.h>
#define STRCPY(T,F)     strncpy(T,F,sizeof(T))









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_varyonvg                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine brings a volume group and its physical volumes       *
 *   on-line by reading and interpreting data in the volume group      *
 *   descriptor areas, by building the kernel data structures to       *
 *   describe the volume group and by adding them into the kernel.     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     varyonvg -> vvg_out                                             *
 *                                                                     *
 *     If successful, the volume group will be varied on.              *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_INVALID_PARAM                                               *
 *     LVM_MAPFOPN                                                     *
 *     LVM_MISSPVNAME                                                  *
 *     LVM_MISSINGPV                                                   *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_varyonvg (

struct varyonvg * varyonvg)
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */


{ /* BEGIN lvm_varyonvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

int vgstatus;
  /* variable to indicate the varied-on status of the volume group */

int mapf_mode;
  /* mode with which to open the mapped file */

int vgmap_fd;
  /* file descriptor for the mapped file */

int vg_fd;
  /* the file descriptor for the volume group reserved area logical
     volume */

int missname;
  /* flag to indicate if there are any PV names missing from the input
     list */

int misspv;
  /* flag to indicate if there are any PVs missing from the varied-on
     volume group (i.e., PVs that could not be defined into the kernel */

int forceqrm;
  /* flag to indicate if the quorum error was overridden and we have
     forced having a quorum of VGDAs */

off_t offset;
  /* variable to hold the offset to which to seek */

caddr_t vgda_ptr;
  /* pointer to the beginning of the volume group descriptor area */

struct vgsa_area * vgsa_ptr;
  /* pointer to the beginning of the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS];
  /* array of logical sector numbers addresses for all VGSA copies */

char mwcc [DBSIZE];
  /* buffer which will contain the mirror write consistency cache */

char vgmap_fn [sizeof (LVM_ETCVG) + 2 * sizeof (struct unique_id)];
  /* variable in which to hold name of the mapped file */

struct fheader mapf_hdr;
  /* the mapped file header structure */

struct inpvs_info inpvs_info;
  /* structure to hold information concerning the input list of PVs for
     the volume group */

struct defpvs_info defpvs_info;
  /* structure to hold information concerning the descriptor areas and
     status areas for PVs defined into the kernel */

short int out_index;
  /* index for PVs in the varyonvg output list */

char putodm_value;


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Check parameters input by the caller for invalid values.          *
 ***********************************************************************
 */

if (varyonvg -> kmid == NULL)
  /* check the input value for the module id to insure that a null pointer
     was not passed */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (varyonvg -> vgname == NULL)
  /* check the input value for the VG name (descriptor area LV name) to
     insure that a null pointer was not passed */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (varyonvg -> vgname [0] == LVM_NULLCHAR)
  /* check the input value for the VG name (LVM reserved area LV name)
     to insure that the name is not a null string */

{
	
    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (varyonvg -> vg_major <= 0)   
  /* check the input value for the major number to insure that it
     is between 1 and the maximum allowed value */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

}

if (varyonvg -> vvg_in.num_pvs <= 0 ||
    varyonvg -> vvg_in.num_pvs > LVM_MAXPVS)
  /* check the number of PV names input by the caller to insure that it
     is between one and the maximum possible number of PVs */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> noopen_lvs != TRUE  &&  varyonvg -> noopen_lvs != FALSE)
  /* if the no open flag does not have a value of either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> auto_resync != TRUE && varyonvg -> auto_resync != FALSE)
  /* if the automatic resynchronization flag does not have a value of
     either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> misspv_von != TRUE  &&  varyonvg -> misspv_von != FALSE)
  /* if the varyon with missing PVs flag does not have a value of either
     true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> override != TRUE  &&  varyonvg -> override != FALSE)
  /* if the flag for overriding the no quorum error does not have a value
     of either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> override == TRUE  &&  varyonvg -> misspv_von == FALSE)
  /* if the user has requested to override the no quorum error but has
     also asked not to varyon with any missing PVs, return error since
     these two conflict */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}

if (varyonvg -> missname_von != TRUE  &&
    varyonvg -> missname_von != FALSE)
  /* if the varyon with missing PV names flag does not have a value of
     either true or false */

{

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter error */

}


/***********************************************************************
 *   Initialize the output values in the caller's varyonvg structure   *
 *   to 0.                                                             *
 ***********************************************************************
 */

bzero (&(varyonvg -> vvg_out), sizeof (varyonvg -> vvg_out));
  /* initialize to 0 the output structure in varyonvg which is used to
     return information to the caller */


/***********************************************************************
 *   Open the mapped file for the volume group in order to lock.       *
 ***********************************************************************
 */

LVM_MAPFN (vgmap_fn, &(varyonvg -> vg_id));
  /* create file name for the mapped file */

mapf_mode = O_RDWR | O_CREAT | O_NSHARE;
  /* set mode for open of mapped file */

vgmap_fd = open (vgmap_fn, mapf_mode, LVM_MAPFPERM);
  /* call routine to open the mapped file in order to hold lock */

if (vgmap_fd < LVM_SUCCESS)
  /* if an error occurred */

{
      /* the map file might still be busy, possibly from another process like
       * syncvg which should be handled with the proper error message
       */

    return(errno == ETXTBSY ? LVM_MAPFBSY : LVM_MAPFOPN);
      /* return with error code for map file open error */

}

/***********************************************************************
 *   Determine if volume group is already fully varied on, and, if so, *
 *   check for missing PVs to bring on-line.                           *
 ***********************************************************************
 */

retcode = lvm_chkvgstat (varyonvg, &vgstatus);
  /* call routine to determine if this volume group is varied on
     already */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

{

    close (vgmap_fd);
      /* close the mapped file */

    return (retcode);
      /* return with error return code */

}

if (vgstatus == HD_FULLVON)
  /* if the volume group is already fully varied on (i.e., all LVs are
     defined) */

{
    close (vgmap_fd);
    if (varyonvg -> noopen_lvs == TRUE)
      /* is user has requested that logical volumes not be defined (i.e.,
	 only the volume group reserved area LV is defined) */

    {

	return (LVM_INVALID_PARAM);
	  /* return error to user for invalid parameter because we are
	     already varied on with all logical volumes defined */

    }

    retcode = lvm_revaryon (varyonvg, &vgmap_fd, &inpvs_info,
			    &defpvs_info);
      /* call routine which will check for and attempt to vary on any
	 missing PVs */

}

else

{

    if (vgstatus == HD_NOLVSVON)
      /* if the specified volume group is currently varied on but with
	 no logical volumes (other than the reserved area) defined */

    {

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete information currently defined into
	     the kernel for this volume group;  then proceed with normal
	     varyon process */

    }

    bzero (&mapf_hdr, sizeof (struct fheader));
      /* initialize the mapped file header structure to zeroes */

    vgda_ptr = NULL;
      /* initialize pointer to volume group descriptor area to null */

    vgsa_ptr = NULL;
      /* initialize pointer to volume group status area to null */

    forceqrm = FALSE;
      /* initialize flag to indicate quorum has not been forced */


    /*******************************************************************
     *   Read information from each physical volume for volume group   *
     *   descriptor and status areas in order to verify consistency    *
     *   and determine if a quorum of VGDAs/VGSAs exists.              *
     *******************************************************************
     */

    retcode = lvm_verify (varyonvg, &vg_fd, &inpvs_info, &defpvs_info,
	      &vgda_ptr, &vgsa_ptr, vgsa_lsn, mwcc);
      /* call routine which will check the volume group descriptor area
	 on each physical volume in order to verify if a quorum exists */

    if (retcode < LVM_SUCCESS  &&
	 (retcode != LVM_NOQUORUM || varyonvg -> override == FALSE))
      /* if an error occurred and if either it was an error other than
	 for no quorum or if user did not ask to override the no quorum
	 error */

    {
	if (vgda_ptr != NULL)
	  /* if the volume group descriptor area is not null */

	{

	    free (vgda_ptr);
	      /* free space allocated for the volume group descriptor
		 area */

	}

	if (vgsa_ptr != NULL)
	  /* if the volume group status area is not null */

	{

	    free ((caddr_t) vgsa_ptr);
	      /* free space allocated for the volume group status area */

	}

	close (vgmap_fd);
	  /* close the mapped file */

	return (retcode);
	  /* return with error code */

    } /* error is to be returned */

    if (retcode == LVM_NOQUORUM)
      /* if the no quorum error was returned but is to be overridden */

    {
	/****************************************************************
	 *   If user requested to override the no quorum error, then    *
	 *   try to force having a quorum of VGDAs.                     *
	 ****************************************************************
	 */

	forceqrm = TRUE;
	  /* set flag to indicate quorum has been artificially forced */

	retcode = lvm_forceqrm (vgda_ptr, &defpvs_info, &inpvs_info,
				vgsa_lsn);
	  /* call routine which is to force a quorum by deleting all VGDA
	     copies from the PVs which are currently missing */

	if (retcode < LVM_SUCCESS)
	  /* if an error occurred */

	{

	    free (vgda_ptr);
	      /* free space allocated for the volume group descriptor
		 area */

	    free ((caddr_t) vgsa_ptr);
	      /* free space allocated for the volume group status area */

	    close (vg_fd);
	      /* close the volume group reserved area logical volume */

	    close (vgmap_fd);
	      /* close the mapped file */

	    lvm_clsinpvs (varyonvg, &inpvs_info);
	      /* close any open PVs */

	    lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	      /* call routine to delete this volume group from the kernel
		 since we could not varyon successfully */

	    return (retcode);
	      /* return with returned error code */

	}

    } /* no quorum error to be overridden */


    /********************************************************************
     *   Check to see if errors need to be returned for missing PV      *
     *   names or missing PVs and update PV status for user.            *
     ********************************************************************
     */
    lvm_pvstatus (varyonvg, &defpvs_info, vgda_ptr, &missname, &misspv);
      /* call routine to update PV status in the output varyonvg
	 structure */

    if (missname == TRUE  &&  varyonvg -> missname_von == FALSE)
      /* if there were any missing PV names in the user input list and
	 if the user has requested not to varyon if there were missing
	 names */

    {

	free (vgda_ptr);
	  /* free space allocated for the volume group descriptor area */

	free ((caddr_t) vgsa_ptr);
	  /* free space allocated for the volume group status area */

	close (vg_fd);
	  /* close the volume group reserved area logical volume */

	close (vgmap_fd);
	  /* close the mapped file */

	lvm_clsinpvs (varyonvg, &inpvs_info);
	  /* close any open PVs */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete this volume group from the kernel
	     since we could not varyon successfully */

	return (LVM_MISSPVNAME);
	  /* return with error code for missing PV names */

    }

    if (misspv == TRUE  &&  varyonvg -> misspv_von == FALSE)
      /* if there are any missing PVs (i.e., PVs which cannot be varied
	 on) and if the user has requested not to varyon if there are
	 missing PVs */

    {

	free (vgda_ptr);
	  /* free space allocated for the volume group descriptor area */

	free ((caddr_t) vgsa_ptr);
	  /* free space allocated for the volume group status area */

	close (vg_fd);
	  /* close the volume group reserved area logical volume */

	close (vgmap_fd);
	  /* close the mapped file */

	lvm_clsinpvs (varyonvg, &inpvs_info);
	  /* close any open PVs */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete this volume group from the kernel
	     since we could not varyon successfully */

	return (LVM_MISSINGPV);
	  /* return with error code for missing PV names */

    }


    /********************************************************************
     *   Initialize information in mapped file header which is to be    *
     *   used by other LVM library routines.                            *
     ********************************************************************
     */
    lvm_mapfile (varyonvg, &inpvs_info, &defpvs_info, &mapf_hdr,
		 vgda_ptr);
      /* call routine to build the mapped file */


    /********************************************************************
     *   Write out the volume group descriptor area to any copies       *
     *   which are out-of-date.                                         *
     ********************************************************************
     */

    retcode = lvm_update (varyonvg, &inpvs_info, &defpvs_info, &mapf_hdr,
	   vg_fd, vgda_ptr, vgsa_ptr, vgsa_lsn, mwcc, forceqrm, misspv);
      /* call routine to write out the volume group descriptor area,
	 status area and mirror write consistency cache, if necessary */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	free (vgda_ptr);
	  /* free space allocated for the volume group descriptor area */

	free ((caddr_t) vgsa_ptr);
	  /* free space allocated for the volume group status area */

	close (vg_fd);
	  /* close the volume group reserved area logical volume */

	close (vgmap_fd);
	  /* close the mapped file */

	lvm_clsinpvs (varyonvg, &inpvs_info);
	  /* close any open PVs */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete this volume group from the kernel
	     since we could not varyon successfully */

	return (retcode);
	  /* return with error return code */

    }

    lvm_clsinpvs (varyonvg, &inpvs_info);
      /* close any open PVs */


    /********************************************************************
     *   Add information about logical volumes into the kernel.         *
     ********************************************************************
     */

    retcode = lvm_setupvg (varyonvg, &inpvs_info, &defpvs_info, &mapf_hdr,
	 vg_fd, vgda_ptr, vgsa_ptr, vgsa_lsn, (struct mwc_rec *) mwcc);
      /* call routine to define the LVDD data structures for this volume
	 group's logical volumes into the kernel */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	free (vgda_ptr);
	  /* free space allocated for the volume group descriptor area */

	free ((caddr_t) vgsa_ptr);
	  /* free space allocated for the volume group status area */

	close (vg_fd);
	  /* close the volume group reserved area logical volume */

	close (vgmap_fd);
	  /* close the mapped file */

	lvm_delvg (&(varyonvg -> vg_id), varyonvg -> vg_major);
	  /* call routine to delete this volume group from the kernel
	     since we could not varyon successfully */

	return (retcode);
	  /* return with error code */

    } /* error defining LVs */

    close (vg_fd);
      /* close the volume group reserved area logical volume */

    free ((caddr_t) vgsa_ptr);
      /* free space allocated for the volume group status area */

    close (vgmap_fd);

    free (vgda_ptr);
      /* free the space allocated for the volume group descriptor area */

    retcode = LVM_SUCCESS;
      /* initialize return code to successful */


    /********************************************************************
     *   Resynchronize the volume group if the user has requested this. *
     ********************************************************************
     */

    if (varyonvg -> auto_resync == TRUE)
      /* if caller has requested that stale partitions be resynced */

    {

	retcode = lvm_vonresync (&(varyonvg -> vg_id));
	  /* call routine to resync all logical volumes in the volume
	     group */

    }


    /********************************************************************
     *   Check to see if user needs to be given a return code to        *
     *   indcate that status in the varyonvg output structure should    *
     *   be checked for additional information.                         *
     ********************************************************************
     */

    for (out_index=0; out_index < varyonvg -> vvg_out.num_pvs;
	 out_index=out_index+1)
      /* loop for all entries in the varyonvg output structure to check
	 the PV status */

    {

      if (varyonvg -> vvg_out.pv[out_index].pv_status != LVM_PVACTIVE)
	/* if there is a PV in the output structure which has a status
	   other than that of active PV */

      {

	  retcode = LVM_CHKVVGOUT;
	    /* set return code to indicate there is status information in
	       the varyonvg output structure which should be checked */

	  break;
	    /* break out of loop;  user needs to check varyonvg output */

      }

    }

} /* VG is not already varied on */

/* update volume group state */
if (retcode == LVM_SUCCESS) {
	if (misspv == TRUE){
		/* set vg state to active/partial */
		strcpy(&putodm_value, "2");
		putodm_cuat(varyonvg->vgname, "state", &putodm_value);
	}
	else {
		/* set vg state to active/complete */
		strcpy(&putodm_value, "1");
		putodm_cuat(varyonvg->vgname, "state", &putodm_value);
	}
}
return (retcode);

} /* END lvm_varyonvg */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_forceqrm                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine forces a quorum by removing the VGDA/VGSA copies     *
 *   from all PVs that are currently missing.                          *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vgda_ptr                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     vgsa_lsn                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
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
lvm_forceqrm (

caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */

struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS])
  /* array of logical sector numbers addresses for all VGSA copies */


{ /* BEGIN lvm_forceqrm */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

struct pv_header * fstpv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

struct pv_header * scndpv_ptr;
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

long partlen_blks;
  /* the length of a partition in number of 512 byte blocks */

short int pv_num;
  /* the PV number of the PV */

short int num_vgdapvs;
  /* number of PVs which contain at least one copy of the VGDA */

short int pv_index;
  /* loop index variable for physical volume */

short int in_index;
  /* index into input list */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


fstpv_ptr = NULL;
  /* initialize pointer to null which is to pointer at PV header entry
     for first PV which is found to have at least one VGDA copy */

scndpv_ptr = NULL;
  /* initialize pointer to null which is to pointer at PV header entry
     for second PV which is found to have at least one VGDA copy */

num_vgdapvs = 0;
  /* initialize to null the count of PVs which have at least one copy of
     the VGDA */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate the size needed for the descriptor area list of logical
     volumes for the maximum number of logical volumes allowed for this
     volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to the
     beginning of the list of physical volumes in the volume group
     descriptor area */


/************************************************************************
 *   Loop through for PV entries in the volume group descriptor area    *
 *   and remove VGDA copies for all PVs which are not currently active. *
 *   This is the way that we artificially force a quorum, i.e., by      *
 *   removing all "missing" VGDA copies so that we should have a full   *
 *   count of descriptor areas.  These PVs are also put in the          *
 *   removed state (LVM_PVREMOVED).                                     *
 ************************************************************************
 */

for (pv_index = 0; pv_index < vghdr_ptr->numpvs; pv_index = pv_index+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area, searching for PVs which are defined
     into the kernel and have at least one VGDA copy  */

{

  pv_num = pv_ptr -> pv_num;
    /* get the PV number of this PV */

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if this PV is currently active (i.e., present and defined into the
       kernel) */

  {

      if (pv_ptr -> pvnum_vgdas > 0)
	/* if this PV has at least one VGDA copy */

      {

	  num_vgdapvs = num_vgdapvs + 1;
	    /* increment number of PVs which have at least one copy of
	       the VGDA */

	  /**************************************************************
	   *   Save pointers to PV entries for first and second PVs     *
	   *   which are found to have at least one VGDA copy.  This is *
	   *   for use in special case of where we only have one or two *
	   *   active PVs left in the volume group.  In this case we    *
	   *   may have to add a VGDA copy since one disk case is to    *
	   *   have total of 2 VGDA copies and two disk case is to have *
	   *   total of 3 VGDA copies.                                  *
	   **************************************************************
	   */

	  if (fstpv_ptr == NULL)
	    /* if this is the first PV we have found that has a VGDA
	       copy */

	  {

	      fstpv_ptr = pv_ptr;
		/* save pointer to first PV found to have a VGDA copy */

	  }

	  else
	    /* not first PV with a VGDA copy */

	  {

	      scndpv_ptr = pv_ptr;
		/* save pointer to next PV found to have a VGDA copy */

	  }

      } /* this PV has a VGDA copy */

  } /* this PV defined in kernel */

  else
    /* this PV not currently active */

  {

      if (!(pv_ptr -> pv_state & LVM_PVREMOVED))
	/* if this non-active PV is not already a removed PV, then put
	   it in the removed state and remove its VGDA copies */

      {

	  pv_ptr -> pv_state = (pv_ptr -> pv_state & ~LVM_PVACTIVE) |
				 LVM_PVREMOVED;
	    /* put this PV in the "removed" state */

	  vghdr_ptr -> total_vgdas = vghdr_ptr -> total_vgdas -
				       pv_ptr -> pvnum_vgdas;
	    /* remove any VGDA copies contained on this PV from the total
	       count of VGDA copies for the entire volume group */

	  pv_ptr -> pvnum_vgdas = 0;
	    /* set the number of VGDA copies for this PV to 0 */

	  vgsa_lsn [pv_ptr -> pv_num - 1] [0] = 0;
	  vgsa_lsn [pv_ptr -> pv_num - 1] [1] = 0;
	    /* update information in array of VGSA logical sector numbers
	       to indicate that this PV no longer has VGSA copies */

      }

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

} /* loop for each PV in the VGDA */


/************************************************************************
 *   Check for special case of only one or two PVs remaining with VGDA  *
 *   copies and, if necessary, adjust the number of VGDA copies on      *
 *   these PVs so that we have the required number of VGDA copies for   *
 *   the volume group.                                                  *
 ************************************************************************
 */

if (num_vgdapvs == LVM_DAPVS_TTL1  &&
      vghdr_ptr -> total_vgdas <  LVM_TTLDAS_1PV)
  /* if there is only one PV remaining in the volume group which has
     VGDA copies and if the total number of VGDA copies is less than
     the required number (2) */

{

    fstpv_ptr -> pvnum_vgdas = LVM_PVMAXVGDAS;
      /* set number of VGDA copies on the remaining PV to the required
	 number of 2 copies for the one disk case */

    vghdr_ptr -> total_vgdas = LVM_PVMAXVGDAS;
      /* set total number of VGDA copies for the volume group in the VG
	 header */

    pv_num = fstpv_ptr -> pv_num;
      /* get PV number for this PV */

    in_index = defpvs_info -> pv [pv_num - 1].in_index;
      /* find index into input list for this PV */

    vgsa_lsn [pv_num - 1] [1] = partlen_blks * inpvs_info -> num_desclps
		* (pv_num - 1) + inpvs_info -> pv [in_index].sa_psn [1];
      /* calculate logical sector number within the volume group reserved
	 area logical volume of where this VGSA copy is located */

}

if (num_vgdapvs == LVM_DAPVS_TTL2  &&
      vghdr_ptr -> total_vgdas <  LVM_TTLDAS_2PV)
  /* if there are two PVs remaining in the volume group which have
     VGDA copies and if the total number of VGDA copies is less than
     the required number (3 copies are needed, 2 on one PV and 1 on the
     other) */

{

    pv_num = scndpv_ptr -> pv_num;
      /* get PV number for the PV (second PV found above) for which we
	 are going to change the number of VGDA/VGSA copies */

    in_index = defpvs_info -> pv [pv_num - 1].in_index;
      /* find index into input list for this PV */

    if (fstpv_ptr -> pvnum_vgdas == LVM_PVMAXVGDAS)
      /* if the first PV has two VGDA copies */

    {

	scndpv_ptr -> pvnum_vgdas = 1;
	  /* set number of VGDA copies on this second PV to one since the
	     other PV has two copies */

	vgsa_lsn [pv_num - 1] [0] = partlen_blks
		    * inpvs_info -> num_desclps * (pv_num - 1)
		    + inpvs_info -> pv [in_index].sa_psn [0];
	  /* calculate logical sector number within the volume group
	     reserved area logical volume of where this VGSA copy is
	     located */

    }

    else
      /* first PV has one VGDA copy */

    {

	scndpv_ptr -> pvnum_vgdas = LVM_PVMAXVGDAS;
	  /* set number of VGDA copies on this second PV to two since the
	     other PV has one copy */

	vgsa_lsn [pv_num - 1] [0] = partlen_blks
		    * inpvs_info -> num_desclps * (pv_num - 1)
		    + inpvs_info -> pv [in_index].sa_psn [0];
	vgsa_lsn [pv_num - 1] [1] = partlen_blks
		    * inpvs_info -> num_desclps * (pv_num - 1)
		    + inpvs_info -> pv [in_index].sa_psn [1];
	  /* calculate logical sector numbers within the volume group
	     reserved area logical volume of where these VGSA copies are
	     located */

    }

    vghdr_ptr -> total_vgdas = LVM_PVMAXVGDAS + 1;
      /* set total number of VGDA copies for the volume group in the VG
	 header */

} /* two PVs with VGDA copies */

if (num_vgdapvs == 0)
  /* if none of the active PVs currently have a VGDA copy */

{

    return (LVM_NOVGDAS);
      /* return error for no VGDA copies */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_forceqrm */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_mapfile                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine builds the mapped file header, which contains        *
 *   certain information which is used by the liblvm subroutines.      *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     inpvs_info                                                      *
 *     defpvs_info                                                     *
 *     mapfilehdr                                                      *
 *     vgda_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     Information has been written to the mapped file header.         *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_mapfile (

struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */

struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs which are
     defined into the kernel */

struct fheader * mapfilehdr,
  /* a pointer to the mapped file header which contains the offsets of
     the different data areas within the mapped file */

caddr_t vgda_ptr)
  /* pointer to the volume group descritptor area */


{ /* BEGIN lvm_mapfile */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

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

daddr_t dalsn;
  /* the logical sector number within the volume group reserved area
     logical volume of where a VGDA copy begins */

short int pv_index;
  /* loop index variable for physical volume */

short int in_index;
  /* index into the input list of a physical volume */

short int da_index;
  /* index for the entry in the descriptor area logical sector number
     array */

short int pv_num;
  /* physical volume number */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Save information about the volume group in the mapped file header *
 *   which will stored in the mapped file for the volume group along   *
 *   with a copy of the current volume group descriptor area.          *
 ***********************************************************************
 */

vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

lvm_mapoff (mapfilehdr, vgda_ptr);
  /* call routine which calculates the offsets from the beginning of the
     mapped file for certain descriptor area information */

strncpy (mapfilehdr -> vgname, varyonvg -> vgname, LVM_NAMESIZ - 1);
  /* copy the volume group device name (i.e., name of the volume group
     reserved area logical volume) into the mapped file header */

mapfilehdr -> major_num = varyonvg -> vg_major;
  /* store the major number of the volume group into the mapped file
     header */

mapfilehdr -> vgda_len = vghdr_ptr -> vgda_size;
  /* store the length in blocks of the volume group descriptor area
     from value in the VG header */

mapfilehdr -> num_desclps = inpvs_info -> num_desclps;
  /* store number of logical partitions per PV for the LVM reserved
     area */

mapfilehdr -> quorum_cnt = vghdr_ptr -> total_vgdas / 2  +  1;
  /* store the number of volume group descriptor areas needed for a
     quorum (i.e., more than half the total) into the mapped file
     header */

partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);
  /* calculate the length of a partition in 512 byte blocks */

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
  /* loop for each physical volume in the volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get the PV number from the physical volume header */


  /**********************************************************************
   *   Store specific information about each PV into the PV information *
   *   array in the mapped file header.                                 *
   **********************************************************************
   */

  if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
    /* if this physical volume is defined into the kernel */

  {

      in_index = defpvs_info -> pv[pv_num - 1].in_index;
	/* save index into input list for this PV */

      mapfilehdr -> pvinfo[pv_num - 1].device =
		      inpvs_info -> pv[in_index].device;
	/* set value for device major/minor number */

      for (da_index=0; da_index < LVM_PVMAXVGDAS; da_index=da_index+1)
	/* loop for each possible copy of the VGDA on a PV */

      {

	dalsn = partlen_blks * inpvs_info -> num_desclps * (pv_num - 1)
		  +  inpvs_info -> pv[in_index].da_psn[da_index];
	  /* calculate the logical sector number within the volume group
	     reserved area logical volume of where this VGDA copy is
	     located */

	mapfilehdr -> pvinfo[pv_num - 1].da[da_index].dalsn = dalsn;
	  /* store logical sector number for this VGDA copy */

	if (inpvs_info -> pv[in_index].da[da_index].ts_status ==
						    LVM_BTSEQETS)
	  /* if this VGDA copy has a beginning and ending timestamp
	     that match, and is therefore a good VGDA copy */

	{

	    mapfilehdr -> pvinfo[pv_num - 1].da[da_index].ts =
			  inpvs_info -> pv[in_index].da[da_index].ts_beg;
	      /* save timestamp value of this VGDA copy;  otherwise,
		 timestamp value will be 0 */

	}

      } /* loop for each VGDA copy on a PV */

  } /* this PV is defined into the kernel */

  else
    /* this PV is not defined into the kernel */

  {

      /******************************************************************
       *   For the PV which is not defined in the kernel, search the    *
       *   input list to try to find a PV with matching PV id.          *
       ******************************************************************
       */

      for (in_index = 0; in_index < varyonvg -> vvg_in.num_pvs;
	   in_index = in_index + 1)
	/* loop for each PV in the input list */

      {

	if (pv_ptr -> pv_id.word1 ==
	    varyonvg -> vvg_out.pv[in_index].pv_id.word1  &&
	    pv_ptr -> pv_id.word2 ==
	    varyonvg -> vvg_out.pv[in_index].pv_id.word2)
	  /* if the PV id of the PV from the volume group matches the
	     PV id of this PV in the input list */

	{

	    break;
	      /* break from loop since we having found a matching PV in
		 the input list;  index into input list of matching PV
		 will be saved */

	}

      } /* loop for each PV in input list */

  } /* this PV not defined into the kernel */

  if (in_index < varyonvg -> vvg_in.num_pvs)
    /* if the PV in the volume group is found in the user's input list,
       whether or not it is active and defined into the kernel, save the
       device name of the PV */

  {

      strncpy (mapfilehdr -> pvinfo[pv_num - 1].pvname,
	    varyonvg -> vvg_out.pv[in_index].pvname, LVM_NAMESIZ - 1);
	/* copy the device name of this physical volume into the PV
	   information array in the mapped file header */

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

} /* loop for each PV in the VGDA */

return;
  /* return to caller */

} /* END lvm_mapfile */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_pvstatus                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to update the PV status in the output       *
 *   varyonvg structure when necessary and to check for missing PVs    *
 *   and missing PV names for error reporting.                         *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     varyonvg                                                        *
 *     defpvs_info                                                     *
 *     vgda_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *missname                                                       *
 *     *misspv                                                         *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_pvstatus (

struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */

struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs which are
     defined into the kernel */

caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */

int * missname,
  /* flag to indicate if there are any PV names missing from the input
     list */

int * misspv)
  /* flag to indicate if there are any PVs missing from the varied-on
     volume group (i.e., PVs that could not be defined into the kernel */


{ /* BEGIN lvm_pvstatus */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

struct inpvs_info inpvs;

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

int retcode;
  /* return code */

short int pv_num;
  /* the number of the PV */

short int pv_index;
  /* loop index variable for physical volume */

short int in_index;
  /* index into the input list of PVs */

short int out_index;
  /* index into the output list of PVs */

struct err_rec {
	 struct err_rec0 err_rec;
	 char detail_data[20];
}e;


/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


*missname = FALSE;
  /* initialize flag to false to indicate there are no missing PV names
     in the input list for this volume group */

*misspv = FALSE;
  /* initialize flag to false to indicate there are no missing PVs in
     the varied-on volume group (i.e., all are present and defined into
     the kernel) */

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

lvm_readpvs(varyonvg, &inpvs);

for (in_index=0; in_index<varyonvg->vvg_in.num_pvs; in_index=in_index+1)
  /* loop for each PV in the input list */

{

  pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
    /* set the pointer to the first physical volume entry to point to
       the beginning of the list of physical volumes in the volume
       group descriptor area */

  for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
    /* loop for each physical volume in the list of PVs in the volume
       group descriptor area, searching for a matching PV id */

  {
    if (pv_ptr -> pv_id.word1 ==
	varyonvg -> vvg_out.pv[in_index].pv_id.word1  &&
	pv_ptr -> pv_id.word2 ==
	varyonvg -> vvg_out.pv[in_index].pv_id.word2)
      /* if we have found a matching PV id in the VGDA for this PV
	  from the input list */

    {

	break;
	  /* break from loop since we have found the matching PV we
	     were searching for */

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

  } /* loop for each PV in the VGDA */

  if (pv_index == vghdr_ptr -> numpvs)
    /* if a matching PV id was not found in the VGDA for this PV from
       the input list */

  {

      varyonvg -> vvg_out.pv[in_index].pv_status = LVM_INVPVID;
	/* set status for this PV in the varyonvg output structure to
	   indicate it has an invalid PV id (i.e., it is not a member
	   of the VG) */

  } /* PV id from input list not found in VGDA */

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
	 of the PV from the volume group descriptor area (note that the
	 PV id value in the output structure is that found in the IPL
	 record of the PV, if the IPL record could be read, and not the
	 value passed in by the user) */

    {

	if ((pv_ptr -> pv_state & LVM_PVREMOVED)  &&  varyonvg ->
	       vvg_out.pv[in_index].pv_status != LVM_PVNOTINVG)
	  /* if the PV state for this PV indicates that it has been
	     temporarily removed from the volume group, meaning that
	     it is not to be varied-on, and if it did not have a bad
	     LVM record which indicated it was not a member of the
	     volume group */

	{

	    varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVREMOVED;
	      /* set status for this PV in the varyonvg output structure
		 to indicate to the user this is a removed PV */

	}

	if (varyonvg -> vvg_out.pv[in_index].pv_status == LVM_PVNOTINVG)
	  /* if the LVM record for this PV indicates it is not a member
	     of the volume group */

	{

	    *misspv = TRUE;
	      /* set flag to indicate that there are PVs in this
		 volume group which could not be varied on and are
		 not defined into the kernel */

	    varyonvg -> vvg_out.pv[in_index].pv_status = LVM_LVMRECNMTCH;
	      /* change PV status in varyonvg output structure to
		 indicate that the LVM record information about VG
		 membership does not match information in the VGDA;  this
		 is an indication to the user that he needs to delete
		 this PV from the volume group since the mismatch of
		 information probably means user has installed this PV
		 into another volume group */

	}

	if (defpvs_info -> pv[pv_num - 1].pv_status == LVM_DEFINED)
	  /* if the PV status in the defpvs_info structure indicates that
	     this PV is defined into the kernel */

	{

	    varyonvg -> vvg_out.pv[in_index].pv_status = LVM_PVACTIVE;
	      /* set status for this PV in the varyonvg output structure
		 to indicate that this PV is currently active */

	}

	else
	  /* this PV is not defined into the kernel */

	{

	    if (varyonvg -> vvg_out.pv[pv_index].pv_status !=
		  LVM_LVMRECNMTCH  &&  varyonvg ->
		  vvg_out.pv[pv_index].pv_status != LVM_PVREMOVED)
	      /* if the PV status has not been previously set for the
		 LVM record no match error or for a removed PV, then set
		 the PV status for this PV to missing, since the PV is
		 not active at the current time */

	    {

		*misspv = TRUE;
		  /* set flag to indicate that there are PVs in this
		     volume group which could not be varied-on and are
		     not defined into the kernel */

		varyonvg -> vvg_out.pv[pv_index].pv_status =
						 LVM_PVMISSING;
		  /* set status to indicate this PV is currently
		     missing */

	    }

		/*  error loging */
		bzero(&e, sizeof(struct err_rec));
		e.err_rec.error_id = ERRID_LVM_MISSPVADDED;
		STRCPY(e.err_rec.resource_name, "LIBLVM");
		*((ulong *)e.detail_data) = inpvs.pv[pv_index].device;
		errlog(&e, sizeof(struct err_rec));


	    retcode = lvm_addmpv (&(varyonvg -> vg_id),
				  varyonvg -> vg_major, pv_num);
	      /* call routine to add this PV into the kernel as a missing
		 PV */

	}

	break;
	  /* break from for loop since we have found the matching PV we
	     were searching for */

    } /* matching PV id found in input list */

  } /* loop for each PV in input list */

  if (in_index == varyonvg -> vvg_in.num_pvs)
    /* if PV from the volume group descriptor area was not found in the
       input list */

  {

      *missname = TRUE;
	/* set flag to indicate that the input list is missing a PV
	   name */

      if (!(pv_ptr -> pv_state & LVM_PVREMOVED))
	/* if the PV is not one which is in the removed state */

      {

	  *misspv = TRUE;
	    /* set flag to indicate that there are PVs in this volume
	       group which could not be varied-on and are not defined
	       into the kernel */

      }

      out_index = varyonvg -> vvg_out.num_pvs;
	/* set index into varyonvg output structure at next available
	   slot where PV id will be stored for PV not passed in the input
	   list */

      varyonvg -> vvg_out.num_pvs = varyonvg -> vvg_out.num_pvs + 1;
	/* increment number of PVs in the varyonvg output structure */

      varyonvg -> vvg_out.pv[out_index].pv_id = pv_ptr -> pv_id;
	/* store into varyonvg output structure the PV id of the PV not
	   passed in the input list */

      varyonvg -> vvg_out.pv[out_index].pv_status = LVM_NONAME;
	/* set status for this PV in the varyonvg output structure to
	   indicate device name was not passed for this PV */

      for (in_index = 0; in_index < varyonvg -> vvg_in.num_pvs;
	   in_index = in_index + 1)
	/* loop for PVs in the input list and compare PV ids passed in
	   by the user with the PV id from the VGDA for which we did not
	   have a name;  this is done in case the user may have passed
	   in the PV id, but it was not the real PV id of the
	   corresponding physical volume device name passed in */

      {

	if (varyonvg -> vvg_out.pv[out_index].pv_id.word1 ==
	    varyonvg -> vvg_in.pv[in_index].pv_id.word1   &&
	    varyonvg -> vvg_out.pv[out_index].pv_id.word2 ==
	    varyonvg -> vvg_in.pv[in_index].pv_id.word2)
	  /* if matching PV id is found in the input list */

	{

	    varyonvg -> vvg_out.pv[out_index].pv_status = LVM_NAMIDNMTCH;
	      /* set PV status to indicate that this PV id was sent in
		 the input list but that it does not match the real PV id
		 of the named physical volume */

	}

      } /* loop for PVs in the input list */

      retcode = lvm_addmpv (&(varyonvg -> vg_id), varyonvg -> vg_major,
			    pv_num);
	/* call routine to add this PV into the kernel as a missing PV */

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

} /* loop for each PV in the VGDA */

/*
 * close pvs we opened above
 */
lvm_clsinpvs (varyonvg, &inpvs);

return;
  /* return to caller */

} /* END lvm_pvstatus */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_update                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function is used to write out the volume group descriptor    *
 *   area, status area, and mirror write consistency cache to any      *
 *   PVs that do not have the latest copy.                             *
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
 *     forceqrm                                                        *
 *     misspv                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     On-disk copies of the volume group descriptor area and status   *
 *     area and the mirror write consistency cache may have been       *
 *     updated with the latest copies.                                 *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_NOQUORUM                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_update (

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

int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */

caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */

struct vgsa_area * vgsa_ptr,
  /* pointer to the volume group status area */

daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array which contains the logical sector numbers of all the VGSA
     copies */

char mwcc [DBSIZE],
  /* buffer which contains the mirror write consistency cache */

int forceqrm,
  /* flag to indicate if the quorum has been forced */

int misspv)
  /* flag to indicate if there were any missing PVs */


{ /* BEGIN lvm_update */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct vg_trailer * vgtrail_ptr;
  /* a pointer to the volume group trailer of the volume group descriptor
     area */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

char mwcc2 [DBSIZE];
  /* buffer into which we can read a mirror write consistency cache */

long size;
  /* variable to hold an interim size calculation */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

off_t offset;
  /* the offset in bytes from the beginning of the logical or physical
     volume where the file pointer is to be placed */

int curcnt_vgdas;
  /* count of currently good VGDA/VGSA copies */

int retcode;
  /* return code */

short int da_index;
  /* index of VGDA copy */

short int pv_num;
  /* the PV number of the PV */

short int pv_index;
  /* loop index variable for physical volume */

short int in_index;
  /* index into input list */

short int wr_index;
  /* loop index for number of VGDAs to write to a PV */

short int numtowrt;
  /* number of VGDA copies to write to a PV */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

vgtrail_ptr = (struct vg_trailer *) ((caddr_t) vgda_ptr +
		 vghdr_ptr -> vgda_size * DBSIZE - DBSIZE);
  /* set a pointer to the trailer of the volume group descriptor area,
     which begins one block from the end of the descriptor area */


size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate the size needed for the descriptor area list of logical
     volumes for the maximum number of logical volumes allowed for this
     volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to
     the beginning of the list of physical volumes in the volume
     group descriptor area */

for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
  /* loop for each physical volume in the list of physical volumes in
     the volume group descriptor area */

{

  pv_num = pv_ptr -> pv_num;
    /* get the PV number for this PV */

  if (defpvs_info -> pv [pv_num - 1].pv_status == LVM_DEFINED)
    /* if this PV is currently active */

  {

      in_index = defpvs_info -> pv [pv_num - 1].in_index;
	/* get index into input list for this PV */


      /**************************************************************
       *   Update the MWCC on this PV if needed.                    *
       **************************************************************
       */

      if (defpvs_info -> pv[pv_num-1].mwc.wrt_status == TRUE)
	/* if this MWCC needs to be updated to the latest level */

      {

	  offset = PSN_MWC_REC0 * DBSIZE;
	    /* calculate the offset in bytes from beginning of the
	       physical disk for mirror write consistency cache */

	  offset = lseek (inpvs_info -> pv[in_index].fd, offset,
			  LVM_FROMBEGIN);
	    /* position file pointer to beginning of MWCC */


	  if (offset != LVM_UNSUCC)
	    /* if no errors on seek */

	  {

	      retcode = writex (inpvs_info -> pv[in_index].fd, mwcc,
				DBSIZE, WRITEV);
		/* write out the latest MWCC to this PV */


	      if (retcode == LVM_UNSUCC  &&
		  (errno == EMEDIA || errno == ESOFT))
		    /* if there is an error which might be because of a
		       bad block */

	      {

		  retcode = lvm_relocmwcc (inpvs_info -> pv[in_index].fd,
					   mwcc);
		    /* call routine to try to hardware relocate the
		       block containing the MWCC */

	      }

	      if (retcode < LVM_SUCCESS)
		/* if mirror write consistency cache could not be
		   written */

	      {

		  retcode = lvm_deladdm (varyonvg, inpvs_info,
					 defpvs_info, pv_num);
		    /* call routine to change this PV to a missing PV in
		       the kernel */

		  if (retcode < LVM_SUCCESS)
		    /* if an error occurred */

		  {

		      return (retcode);
			/* return with error code */

		  }

	      } /* MWCC write unsuccessful */

	  } /* no errors on lseek */

      } /* MWCC for this PV needs update */

  } /* this PV is currently active */

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


/************************************************************************
 *   If the quorum was not forcefully achieved, then check each active  *
 *   PV to see if any VGDA or VGSA copies need to be rewritten,  If so, *
 *   then the VGDA with newest timestamp and the VGSA with newest       *
 *   timesamp will be propagated to those copies.  Note that the        *
 *   timestamp is not updated.                                          *
 ************************************************************************
 */

if (forceqrm == FALSE)
  /* if the quorum has not been forced */

{

    size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
      /* calculate size needed for the descriptor area list of logical
	 volumes for maximum number of logical volumes allowed for this
	 volume group */

    lvlist_size = LVM_SIZTOBBND (size);
      /* calculate the actual size to be reserved in the descriptor area
	 for the list of logical volumes by rounding the size up to the
	 nearest multiple of the blocksize of the physical volume */

    pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
      /* set the pointer to the first physical volume entry to point to
	 the beginning of the list of physical volumes in the volume
	 group descriptor area */

    curcnt_vgdas = 0;
      /* initialize the current count of good VGDA/VGSA copies to 0 */

    for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
      /* loop for each physical volume in the list of physical volumes in
	 the volume group descriptor area */

    {

      numtowrt = 0;
	/* initialize number of VGDA copies to be written to this PV
	   to 0 */

      pv_num = pv_ptr -> pv_num;
	/* get the PV number for this PV */

      if (defpvs_info -> pv [pv_num - 1].pv_status == LVM_DEFINED)
	/* if this PV is currently active */

      {

	  in_index = defpvs_info -> pv [pv_num - 1].in_index;
	    /* get index into input list for this PV */

	  /**************************************************************
	   *   Find number of VGDA copies which need to be written to   *
	   *   this PV.                                                 *
	   **************************************************************
	   */

	  if (pv_ptr -> pvnum_vgdas > 0)
	    /* if this PV has at least one VGDA copy */

	  {

	      da_index = inpvs_info -> pv [in_index].index_newestda;
		/* set index for descriptor area to the index of the VGDA
		   copy on this PV which has the newest VGDA */

	      if (inpvs_info -> pv[in_index].da[da_index].wrt_status
							  == TRUE)
		/* if the write status for this VGDA copy indicates that
		   it does need to be updated */

	      {

		  numtowrt = numtowrt + 1;
		    /* increment number of VGDA copies to be written to
		       this PV */

	      }

	  } /* this PV has at least one VGDA copy */

	  if (pv_ptr -> pvnum_vgdas == LVM_PVMAXVGDAS)
	    /* if this PV has two VGDA copies */

	  {

	      da_index = da_index ^ 1;
		/* exclusive OR the VGDA copy index with 1 in order to
		   flip-flop the index value from 0 to 1 or 1 to 0 */

	      if (inpvs_info -> pv[in_index].da[da_index].wrt_status
							  == TRUE)
		/* if the write status for this VGDA copy indicates that
		   it does need to be updated */

	      {

		  numtowrt = numtowrt + 1;
		    /* increment number of VGDA copies to be written to
		       this PV */

	      }

	  } /* this PV has two VGDA copies */

	  da_index = inpvs_info -> pv [in_index].index_nextda;
	    /* set index for descriptor area to the index of the VGDA
	       copy on this PV which is the next to be written */

	  for (wr_index = 0; wr_index < pv_ptr -> pvnum_vgdas;
	       wr_index = wr_index + 1)
	    /* loop for number of VGDA/VGSA copies on this PV */

	  {

	    /************************************************************
	     *   Update VGDA copy if needed.                            *
	     ************************************************************
	     */

	    if (wr_index < numtowrt)
	      /* if this VGDA copy needs to be updated */

	    {

		retcode = lvm_wrtdasa (vg_fd, vgda_ptr,
		      (struct timestruc_t *) &(vgtrail_ptr -> timestamp),
		      inpvs_info -> vgda_len,
		      maphdr_ptr -> pvinfo[pv_num-1].da[da_index].dalsn,
		      inpvs_info -> pv[in_index].da[da_index].wrt_order);
		  /* call routine to write a copy of the VGDA at the
		     specified logical sector */

		if (retcode == LVM_SUCCESS)
		  /* if successful write of this VGDA copy */

		{

		    maphdr_ptr -> pvinfo[pv_num - 1].da[da_index].ts =
					       vghdr_ptr -> vg_timestamp;
		      /* update the timestamp value for this VGDA copy
			 which is saved in the mapped file header */

		}

		else
		  /* this VGDA copy was not successfully written */

		{

		    maphdr_ptr->pvinfo[pv_num-1].da[da_index].ts.tv_sec
								 = 0;
		    maphdr_ptr->pvinfo[pv_num-1].da[da_index].ts.tv_nsec
								 = 0;
		      /* set to 0 the timestamp value for this VGDA copy
			 which is saved in the mapped file header, since
			 there is not a good copy at this VGDA */

		    retcode = lvm_deladdm (varyonvg, inpvs_info,
					   defpvs_info, pv_num);
		      /* call routine to change this PV to a missing PV
			 in the kernel */

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

		da_index = da_index ^ 1;
		  /* exclusive OR the VGDA copy index with 1 in order to
		     flip-flop the index value from 0 to 1 or 1 to 0 */

	    } /* this VGDA needs to be updated */


	    /************************************************************
	     *   Update VGSA copy if needed.                            *
	     ************************************************************
	     */

	    if (defpvs_info -> pv[pv_num-1].sa[wr_index].wrt_status ==
							   TRUE)
	      /* if this VGSA copy needs to be updated */

	    {

		retcode = lvm_wrtdasa (vg_fd, (caddr_t) vgsa_ptr,
		     (struct timestruc_t *) &(vgsa_ptr -> e_tmstamp),
		     inpvs_info -> vgsa_len, vgsa_lsn[pv_num-1][wr_index],
		     defpvs_info -> pv[pv_num-1].sa[wr_index].wrt_order);
		  /* call routine to update this VGSA copy */

		if (retcode < LVM_SUCCESS)
		  /* if not successfully updated */

		{

		    retcode = lvm_deladdm (varyonvg, inpvs_info,
					   defpvs_info, pv_num);
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

	    } /* this VGSA needs to be updated */

	  } /* loop for VGDA/VGSA copies on this PV */

	  if( defpvs_info -> pv [pv_num - 1].pv_status == LVM_DEFINED)
	    /* if this PV is still active (i.e., it was not declared
	       missing because of inability to write to a VGDA or VGSA
	       copy */

	  {

	      curcnt_vgdas = curcnt_vgdas + pv_ptr -> pvnum_vgdas;
		/* increment count of good VGDA/VGSA copies by number
		   contained on this PV */

	  }

      } /* this PV is currently active */

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

    if (curcnt_vgdas < maphdr_ptr -> quorum_cnt)
      /* if we no longer have a quorum count of VGDA/VGSA copies */

    {

	return (LVM_NOQUORUM);
	  /* return with error code for no quorum */

    }

} /* quorum has not been forced */


/************************************************************************
 *   If either the quorum has been forced or if there were any missing  *
 *   PVs, then update the timestamp value in the VGDA and in the VGSA   *
 *   and write the updated versions to all copies which are located on  *
 *   active PVs.                                                        *
 ************************************************************************
 */

if (forceqrm == TRUE  ||  misspv == TRUE)
  /* if either the quorum had to be forced or there were any missing
     PVs */

{

    retcode = lvm_updtime (&(vghdr_ptr -> vg_timestamp),
			   &(vgtrail_ptr -> timestamp));
      /* update the beginning and ending timestamps for the VGDA */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	return (retcode);
	  /* return with unsuccessful return code */

    }

    retcode = lvm_updtime (&(vgsa_ptr -> b_tmstamp),
			   &(vgsa_ptr -> e_tmstamp));
      /* update the beginning and ending timestamps for the VGSA */

    if (retcode < LVM_SUCCESS)
      /* if an error occurred */

    {

	return (retcode);
	  /* return with unsuccessful return code */

    }


    /********************************************************************
     *   Write out the updated volume group descriptor area and the     *
     *   updated volume group status area to all copies for the volume  *
     *   group.                                                         *
     ********************************************************************
     */

    size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
      /* calculate size needed for the descriptor area list of logical
	 volumes for maximum number of logical volumes allowed for this
	 volume group */

    lvlist_size = LVM_SIZTOBBND (size);
      /* calculate the actual size to be reserved in the descriptor area
	 for the list of logical volumes by rounding the size up to the
	 nearest multiple of the blocksize of the physical volume */

    pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
      /* set the pointer to the first physical volume entry to point to
	 the beginning of the list of physical volumes in the volume
	 group descriptor area */

    curcnt_vgdas = 0;
      /* initialize the current count of good VGDA/VGSA copies to 0 */

    for (pv_index=0; pv_index < vghdr_ptr->numpvs; pv_index=pv_index+1)
      /* loop for each physical volume in the list of physical volumes in
	 the volume group descriptor area */

    {

      pv_num = pv_ptr -> pv_num;
	/* get the PV number for this PV */

      if (defpvs_info -> pv [pv_num - 1].pv_status == LVM_DEFINED)
	/* if this PV is currently active */

      {

	  for (wr_index = 0; wr_index < pv_ptr -> pvnum_vgdas;
	       wr_index = wr_index + 1)
	    /* loop for number of VGDA/VGSA copies on this PV */

	  {

	    retcode = lvm_wrtdasa (vg_fd, vgda_ptr, (struct timestruc_t *)
		&(vgtrail_ptr -> timestamp), inpvs_info -> vgda_len,
		maphdr_ptr ->  pvinfo[pv_num-1].da[wr_index].dalsn,
		(short int) LVM_BEGMIDEND);
	      /* call routine to update this copy of the VGDA */

	    if (retcode < LVM_SUCCESS)
	      /* if not successfully updated */

	    {

		retcode = lvm_deladdm (varyonvg, inpvs_info,
				       defpvs_info, pv_num);
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


	    retcode = lvm_wrtdasa (vg_fd, (caddr_t) vgsa_ptr,
		 (struct timestruc_t *) &(vgsa_ptr -> e_tmstamp),
		 inpvs_info->vgsa_len, vgsa_lsn[pv_num-1][wr_index],
		 (short int) LVM_BEGMIDEND);
	      /* call routine to update this VGSA copy */

	    if (retcode < LVM_SUCCESS)
	      /* if not successfully updated */

	    {

		retcode = lvm_deladdm (varyonvg, inpvs_info,
				       defpvs_info, pv_num);
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

	  } /* loop for VGDA/VGSA copies on this PV */

	  if( defpvs_info -> pv [pv_num - 1].pv_status == LVM_DEFINED)
	    /* if this PV is still active (i.e., it was not declared
	       missing because of inability to write to a VGDA or VGSA
	       copy */

	  {

	      curcnt_vgdas = curcnt_vgdas + pv_ptr -> pvnum_vgdas;
		/* increment count of good VGDA/VGSA copies by number
		   contained on this PV */

	  }

      } /* this PV is currently active */

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

    if (curcnt_vgdas < maphdr_ptr -> quorum_cnt)
      /* if we no longer have a quorum count of VGDA/VGSA copies */

    {

	return (LVM_NOQUORUM);
	  /* return with error code for no quorum */

    }

} /* quorum was forced or missing PVs */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_update */
