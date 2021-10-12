static char sccsid[] = "@(#)39	1.7.1.3  src/bos/usr/ccs/lib/liblvm/varyoffvg.c, liblvm, bos411, 9428A410j 2/22/93 16:22:17";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - varyoffvg.c
 *
 * FUNCTIONS: lvm_varyoffvg
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

#include <fcntl.h>;
#include <sys/types.h>;
#include <sys/dasd.h>;
#include <sys/hd_config.h>;
#include <liblvm.h>;









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_varyoffvg                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   The volume group specified by the volume group id will be         *
 *   varied off-line.                                                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LVDD data structures will be removed from the kernel for    *
 *     the specified volume group and the entry in the device switch   *
 *     table will be deleted.                                          *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_INVALID_PARAM                                               *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_varyoffvg (

  struct varyoffvg * varyoffvg)
    /* the structure which contains input information for the varyoff
       volume group routine */


{ /* BEGIN lvm_varyoffvg */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* the return code */

struct ddi_info cfgdata;
  /* structure to contain the device dependent information for the
     configuration device driver */

char vgmap_name [sizeof(LVM_ETCVG) + 2 * sizeof(struct unique_id)];
  /* path name of the volume group mapped file */

struct queryvgs *queryvgs;
mid_t  kmid;
int    cnt;
long   major_num;

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


if (varyoffvg == NULL)
  /* if the pointer to the varyoff input structure is null */

    { /* BEGIN THEN */

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

    } /* END THEN */

if (varyoffvg -> lvs_only != TRUE  &&  varyoffvg -> lvs_only != FALSE)
  /* if the flag for varyoff of logical volumes only is not equal to
     either true or false */

    { /* BEGIN THEN */

    return (LVM_INVALID_PARAM);
      /* return error code for invalid parameter */

    } /* END THEN */

major_num = -1;
kmid = (mid_t) loadext(LVM_LVDDNAME,FALSE,TRUE);

if(kmid == NULL)
	return(LVM_PROGERR);

retcode = lvm_queryvgs(&queryvgs,kmid);
  /* call lvm_queryvgs to see which vgs are on-line */
if(retcode < LVM_SUCCESS)
	return(retcode);

/*
*  loop for the number of vgs that are on-line 
*  and try to find one with the same vgid as the one passed in
*/

for(cnt=0; cnt < queryvgs->num_vgs; cnt++) {
	if(queryvgs->vgs[cnt].vg_id.word1 == varyoffvg->vg_id.word1 &&
       	   queryvgs->vgs[cnt].vg_id.word2 == varyoffvg->vg_id.word2) { 
		major_num = queryvgs->vgs[cnt].major_num;
	        break;
	}
}

/*
*  if the vg in question is not on-line return an error
*/

if(major_num == -1)
   	return(LVM_OFFLINE);

cfgdata.parmlist.kdelvg.vg_id.word1 = varyoffvg -> vg_id.word1;
cfgdata.parmlist.kdelvg.vg_id.word2 = varyoffvg -> vg_id.word2;
  /* store the volume group id into the data structure which contains
     input parameters to be passed to the kernel */

cfgdata.parmlist.kdelvg.lvs_only = varyoffvg -> lvs_only;
  /* store into the kernel input data structure the value passed into the
     varyoffvg routine for the LVs only flag, which indicates if this is
     only to be a deletion of all defined logical volumes or if this is
     to be a complete varyoff of the volume group */

retcode = lvm_config (NULL, major_num, HD_KDELVG, &cfgdata);
  /* call routine to do set up and call the configuration device driver
     to configure the logical volume device driver with the deletion of
     the volume group */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

    { /* BEGIN THEN */

    return (retcode);
      /* return with error return code */

    } /* END THEN */

/* create name of mapped file */
LVM_MAPFN(vgmap_name,(&varyoffvg->vg_id));
/* delete the mapped file */
(void) unlink(vgmap_name);

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_varyoffvg */



