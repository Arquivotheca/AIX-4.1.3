static char sccsid[] = "@(#)13	1.17.1.3  src/bos/usr/ccs/lib/liblvm/deletelv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:51:50";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_deletelv
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

#include <sys/stat.h>
#include <sys/hd_config.h>
#include <fcntl.h>
#include <liblvm.h>

extern void _lvm_ignoresigs(int);

/*****************************************************************************
*  Name : lvm_deletelv  
*
*  Function: This function is called by a system management command to 
*  delete a logical volume from it's volume group. The logical volume must
*  not be open when this function is called. All logical partitions 
*  belonging to this logical volume are deallocated and the LVM data 
*  structures and descriptors are updated
*
*
*  NOTES:
*    Input:
*       lv_id  
*
*    Output:
*      If successful, the data structures in the mapped file and on the hard 
*      file will be updated and the logical volume will not exist.
*
*   Return Values LVM_SUCCESS if successful and one of the following if
*   not successful.  
*            LVM_NODELLV
*            LVM_OFFLINE
*            LVM_INVALID_PARAM
*            LVM_INVALID_MIN_NUM
*            LVM_ALLOCERR
*            LVM_LVOPEN
*            LVM_PROGERR
*            LVM_MAPFOPN
*            LVM_MAPFOPN
*            LVM_DALVOPN
*/
/*****************************************************************************/
int
lvm_deletelv(
struct lv_id *lv_id) /* logical volume id */ 

{
   struct unique_id vg_id;  /* volume group id */
   short  minor_num;        /* minor number of logical volume */
   int  vgfd;               /* file descriptor for volume group mapped file */
   char *vgptr;             /* pointer to the volume group mapped file */
   int rc;                  /* holds error/return codes */
   struct lv_entries *lv;   /* points to the logical voluem entry */
   struct vg_header *vgh;   /* points to the logical volume header */
   struct fheader *head;    /* points to the volume group mapped file header */
   struct namelist *nptr;   /* pointer to the namelist area */
   struct ddi_info cfgdata; /* structure for kernel update routine */
   int  mode;               /* mode to open mapped file with */


/* initialize return code */

   rc = LVM_SUCCESS;
 
/*
*   check to be sure that the lv_id pointer is not null
*/

   if(lv_id == NULL)
   {   lvm_errors("deletelv","      ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

/*
*  Call get_lvinfo() to open the volume group mapped file and return the file
*  descriptor and pointer, and to check the validity of the minor number and
*  volume group id
*  
*  If get_lvinfo fails
*      return an error code and exit
*/
   mode = O_RDWR | O_NSHARE;
   rc = get_lvinfo(lv_id,&vg_id,&minor_num,&vgfd,&vgptr,mode);
   if(rc < LVM_SUCCESS)
   {
	lvm_errors("get_lvinfo","deletelv",rc);
	free(vgptr);
	return(rc);
   }


/*
*      Call get_ptrs to get the needed pointers.
*      set the pointer to the correct logical volume
*/
       
      rc = get_ptrs(vgptr,&head,&vgh,&lv,NULL,NULL,&nptr);
      if(rc < LVM_SUCCESS)
      {
	close(vgfd);
	free(vgptr);
       	return(rc);
      }
      lv += minor_num - 1; 


/*
*      if the logical volume specified is undefined
*          return an error
*/

      if(lv->lv_state == LVM_LVUNDEF)
      {
	  lvm_errors("deletelv","     ",LVM_INVALID_PARAM);
          close(vgfd);
	  free(vgptr);
          return(LVM_INVALID_PARAM);
      }

/*
*        if the num_lps field on the logical volume is zero then
*            zero out the logical volume entry 
*            for the lv being deleted and decrement the number of logical
*            volumes on the volume group header
*        else
*            return an error 
*/

      if(lv->num_lps == 0)
      {  vgh->numlvs -= 1;
         bzero((char *)lv,sizeof(struct lv_entries));
         bzero((char *)(nptr->name[minor_num - 1]),LVM_NAMESIZ);
      }
      else
      {
	  lvm_errors("deletelv","     ",LVM_NODELLV);
          close(vgfd);
	  free(vgptr);
          return(LVM_NODELLV);
      }
/*
*   call the kernel update rouitne
*   if the routine fails
*     close the mapped file and return an error
*/

    /*
     * ignore certain sigs in an attempt to keep the
     * kernel and the mapped file in sync...
     */
    _lvm_ignoresigs(1);

    bzero((char *)(&cfgdata.parmlist.kdellv), sizeof(struct kdellv));
    cfgdata.parmlist.kdellv.vg_id.word1 = vg_id.word1;
    cfgdata.parmlist.kdellv.vg_id.word2 = vg_id.word2;
    cfgdata.parmlist.kdellv.lv_minor = minor_num;
    rc = lvm_config((mid_t)NULL,head->major_num,HD_KDELLV,&cfgdata);
    if(rc < LVM_SUCCESS)
    {
	lvm_errors("lvm_config","createlv",rc);
        close(vgfd);
        _lvm_ignoresigs(0);		/* reset signals */
	free(vgptr);
        return(rc);
    }
/*
*      Call lvm_diskio() to commit the changes to hard file
*      Use the lvm system calls to update kernel data structures
*      close the file (this also unlocks the file)
*/
 
       rc = lvm_diskio(vgptr,vgfd);
       if(rc < LVM_SUCCESS)
       {
	   close(vgfd);
           _lvm_ignoresigs(0);		/* reset signals */
	   free(vgptr);
           return(rc);
       }

       _lvm_ignoresigs(0);		/* reset signals */

       close(vgfd);
       free(vgptr);
       return(LVM_SUCCESS);
}
/*****************************************************************************/
