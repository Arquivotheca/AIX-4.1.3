static char sccsid[] = "@(#)21	1.15  src/bos/usr/ccs/lib/liblvm/extendlv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:52:03";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_extendlv
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

#include <fcntl.h>
#include <liblvm.h>

/*************************************************************************/
/* Name : lvm_extendlv 
*
*  Function: This function is called by a system management command to 
*  extend the size of a logical volume or add a mirror to a logical partition. 
*
*  NOTES:
*    Input:
*      lv_id  
*      extendlv
*
*    Output:
*      If successful, the data structures in the mapped file and on the hard 
*      file will be updated and the logical volume size will be extended, or 
*      a mirror will be added.
*
*
*    Return Value: if successful LVM_SUCCESS
*                  if unsuccessful one of the following:
*			LVM_INVALID_MIN_NUM
*			LVM_INVALID_PARAM
*	                LVM_OFFLINE
* 			LVM_PROGERR
*			LVM_MAPFOPN
*			LVM_MAPFSHMAT
*			LVM_ALLOCERR
*			LVM_LVOPEN
*			LVM_MAPFRDWR
*			LVM_PPNUM_INVAL
*			LVM_LPNUM_INVAL
*			LVM_PVSTATE_INVAL
*			LVM_NOALLOCLP
*			LVM_DALVOPN
*
*/
/*****************************************************************************/
int
lvm_extendlv(
struct lv_id *lv_id,  /* logical volume id */
struct ext_redlv *extendlv) /* structure for the partitions to be extended */


{

    int rc;    			/* holds the return code */
    int flag;  			/* indicates reduce or extend */
    struct unique_id vg_id;	/* holds volume group id */
    char *vgfptr;		/* pointer to volume group mapped file */
    int  vgfd;			/* file descriptor for volume group file */
    short minor_num;		/* minor number of logical volume */
    int  mode;			/* mode to open the mapped file with */

/* initialize the return code */

   rc = LVM_SUCCESS;

/*
*    check that the lv_id pointer is not null
*/

   if(lv_id == NULL)
   {   lvm_errors("extendlv","    ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
/*
*  check that the first extendlv pointer is not null and
*  check that each valid extendlv pointer has a non-null parts pointer
*/

   if(extendlv == NULL)
   {   lvm_errors("extendlv","    ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

   if(extendlv != NULL)
   {   if(extendlv->parts == NULL)
       {   lvm_errors("extendlv","      ",LVM_INVALID_PARAM);
           return(LVM_INVALID_PARAM);
       }
   }

   /*
    * Call get_lvinfo() to get the vgid, file descriptor for the
    * volume group file, and the pointer to the volume group mapped file
    */
   mode = O_RDWR | O_NSHARE;
   rc = get_lvinfo(lv_id,&vg_id,&minor_num,&vgfd,&vgfptr,mode);
   if(rc < LVM_SUCCESS)
   {
	free(vgfptr);
       	return(rc);
   }

/*  initialize the flag to indicate extend */

   flag = LVM_EXTEND;
/*
*   Call rdex_preoc() with the flag set to LVM_EXTEND in order to process 
*   the extension of the logical volume
*   if return code is not LVM_SUCCESS
*       return(error)
*/

    rc = rdex_proc(lv_id,extendlv,vgfptr,vgfd,minor_num,flag);
    close(vgfd);

    free(vgfptr);
    return(rc);
}
/*****************************************************************************/
