static char sccsid[] = "@(#)64	1.14  src/bos/usr/ccs/lib/liblvm/resynclp.c, liblvm, bos411, 9428A410j 10/17/90 17:59:56";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_resynclp
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

#include <liblvm.h>
#include <sys/lvdd.h>
#include <fcntl.h>

/*
 * NAME: lvm_resynclp
 *                                                                    
 * FUNCTION: This routine synchronizes all physical partitions for a logical
 *           partition.
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *  DATA STRUCTURES:
 *     input
 *       lv_id
 *       lp_num
 * 
 *     output
 *        The pp states will be changed to indicate that they are not stale,
 *        and the data on all copies of the logical partition will be in sync.
 *
 * RETURN VALUE  LVM_SUCCESS if successful,
 *               or one of the following if unsuccessful:
 *                  LVM_INVALID_PARAM
 *                  LVM_INVALID_MIN_NUM
 *                  LVM_OFFLINE
 *                  LVM_PROGERR
 *                  LVM_MAPFSHMAT
 *                  LVM_MAPFOPN
 *                  LVM_NOTCHARDEV
 *                  LVM_INV_DEVENT
 *                  LVM_MAPFRDWR
 *                  LVM_ALLOCERR
 *                  LVM_DALVOPN
 *
 */  

int
lvm_resynclp(
struct lv_id *lv_id,   /* pointer to logical volume id */
long          lp_num,  /* logical partition number to be resynced */
int           force)   /* indicates we want to resync a non-stale lp */


{
 
    /* begin lvm_resynclp */
  
      int rc;                  /* holds the return codes */
      struct unique_id vg_id;  /* volume group id */
      short  minor_num;        /* minor number of logical volume */
      char *vgptr;             /* pointer to volume group mapped file */
      int vgfd;                /* volume group mapped file descriptor */
      struct lv_entries *lv;   /* pointer to the logical volume entry */
      struct namelist *nptr;   /* pointer to the namelist area */
      char rawname[LVM_EXTNAME];
                               /* holds raw name of lv */
      int lvfd;                /* file descriptor for logical volume opened */ 
      struct fheader *fhead;   /* pointer to the vg mapped file header */
      struct logview *lp;      /* pointer to the logical view of the lv */
      long cnt[LVM_NUMCOPIES]; /* array of the count of pps per copy of the lv*/
      long lpcntr;             /* counter of for loop to process lps in */
                               /* the logical view */
      long cpcntr;             /* counts the number of copies processed */
      char stale;              /* indicates that a logical volume is still */
                               /* stale after resyncng a logical partition */
      int mode;                /* mode to open mapped file with */
      int flag;		       /* indicates a forced synclp call */
 
/*
*  if the lv_id is null
*    return an error
*/

   if(lv_id == NULL)
   {   lvm_errors("resynclp","   ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

/*  if the force flag is not TRUE or FALSE return an error */
   if((force != TRUE) && (force != FALSE))
	return(LVM_INVALID_PARAM);

/*
*  Call get_lvinfo() to open the mapped file and return info on the logical
*  volume
*/
    mode = O_RDWR | O_NSHARE;
    rc = get_lvinfo(lv_id,&vg_id,&minor_num,&vgfd,&vgptr,mode);
    if(rc < LVM_SUCCESS)
        return(rc);


/*
*  call get_ptrs to get a pointer to the logical volume containing the 
*  logical partition that needs to be resynced, and a pointer to the name area
*/

   rc = get_ptrs(vgptr,&fhead,NULL,&lv,NULL,NULL,&nptr);
   if(rc < LVM_SUCCESS)
   {    close(vgfd);
	free(vgptr);
	return(rc);
   }


/*
*  increment the pointer to the correct logical volume
*/

   lv += minor_num - 1;

/*
*  if the logical partition number is invalid
*     return an error
*/
 
   if(lp_num > lv->maxsize || lp_num <= 0)
   {    close(vgfd);
	free(vgptr);
	return(LVM_INVALID_PARAM);
   }

/*
*  call status_chk() to be sure the device is raw, and that it has
*  a valid major number, and to get the complete rawname of the logical volume
*/
 
   rc = status_chk(vgptr,nptr->name[lv->lvname],CHECK_MAJ,rawname);
   if(rc < LVM_SUCCESS)
   {    close(vgfd);
	free(vgptr);
	return(rc);
   }
   lvfd = open(rawname,O_RDWR);
   if(lvfd < LVM_SUCCESS)
   {    lvm_errors("open lv","resynclp",lvfd);
	close(vgfd);
	free(vgptr);
	return(LVM_PROGERR);
   }

/*
*   if the force flag is on, call synclp() with a TRUE flag
*   if the force flag is false, call synclp() with a FALSE flag
*/
   if(force == TRUE)
      flag = TRUE;
   else
      flag = FALSE;

   rc = synclp(lvfd,lv,&vg_id,vgptr,vgfd,minor_num,lp_num,flag);

   close(lvfd);
   if(rc < LVM_SUCCESS)
   {
       free(vgptr);
       if(rc == LVM_UNSUCC)
          rc = LVM_NOTSYNCED;
   }


    return(rc);

}
/************************************************************************/   
