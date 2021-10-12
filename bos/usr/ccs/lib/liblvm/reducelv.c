static char sccsid[] = "@(#)04	1.15  src/bos/usr/ccs/lib/liblvm/reducelv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:53:20";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_reducelv
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
/* Name : lvm_reducelv 
*
*  Function: This function is called by a system management command to 
* reduce the size of a logical volume or remove a physical parrtition from a    
* logical partition.
*
*  NOTES:
*    Input:
*      lv_id  
*      reducelv
*
*    Output:
*      If successful, the data structures in the mapped file and on the hard 
*      file will be updated and the logical volume size will be removed, or 
*      a mirror will be removed.
*
*    Return Values: if successful LVM_SUCCESS
*                   if unsuccessful, one of the following return codes:
*   			LVM_INVALID_PARAM
*			LVM_INVALID_MIN_NUM
*			LVM_OFFLINE
*			LVM_PROGERR
*			LVM_MAPFOPN
*			LVM_MAPFSHMAT
*			LVM_ALLOCERR
*			LVM_LVOPEN
*			LVM_MAPFRDWR
*			LVM_INVLPRED
*			LVM_PPNUM_INVAL
*                       LVM_LPNUM_INVAL
*                       LVM_DALVOPN
*
*/
/*****************************************************************************/
int 
lvm_reducelv(
struct lv_id *lv_id,         /* logical volume id */
struct ext_redlv *reducelv)  /* pointer to ext_redlv structure */

{
   int rc; 		        /* holds return code */
   int flag;  			/* indicates reducelv to rdex_proc() */
   struct unique_id vg_id;	/* holds volume group id */
   char *vgfptr;		/* pointer to volume group mapped file */
   int  vgfd;			/* file descriptor for volume group file */
   short minor_num;		/* minor number of logical volume */
   int  mode;			/* mode to open the mapped file with */
 
/* initialize return code */

   rc = LVM_SUCCESS;

/*
*  check that the lv_id pointer is not null
*/

   if(lv_id == NULL)
   {   lvm_errors("reducelv","     ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

/*
*  check that the pointers for parts and pp_map are not null
*  and return an error if they are
*/

   if(reducelv == NULL) 
   {   lvm_errors("reducelv","     ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
   if(reducelv != NULL)
   {   if(reducelv->parts == NULL)
       {   lvm_errors("reducelv","     ",LVM_INVALID_PARAM);
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

   flag = LVM_REDUCE;

/*
* Call rdex_proc() with the flag to LVM_REDUCE in order to process a reduction
* if the return code is not LVM_SUCCESS
*   return(error)
*/

    rc = rdex_proc(lv_id,reducelv,vgfptr,vgfd,minor_num,flag);
    close(vgfd);
    
   free(vgfptr);
   return(rc);
}
/******************************************************************************/
