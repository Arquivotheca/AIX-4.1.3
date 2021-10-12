static char sccsid[] = "@(#)93	1.35  src/bos/usr/ccs/lib/liblvm/createlv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:50:26";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_createlv, create_errorchk, create_updates
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
#include <sys/dasd.h>
#include <fcntl.h>
#include <liblvm.h>
#include <lvmrec.h>


int create_errorchk(
 struct createlv *createlv,  /* pointer to createlv structure */
 char            *vgptr);    /* pointer to volume group mapped file */

int create_updates(
 struct createlv *createlv,  /* pointer to createlv structure */
 char            *vgptr);    /* pointer to volume group mapped file */

/*****************************************************************************
*  Name : lvm_createlv 
*
*  Function:  This function is called by a system management command to
*  create an empty logical volume in an existing volume group with the 
*  information supplied.
*
*  NOTES:
*
*    Input:  
*     createlv
*
*    Output:
*      If successful, the mapped data structure file and the descriptors
*      on the hard file will be updated; otherwise, an error code is returned.
*
*    Return Values: If successful, the routine returns LVM_SUCCESS. If the
*    routine fails, one of the following is returned.
*          LVM_VGFULL
*          LVM_INVALID_PARAM
*          LVM_OFFLINE
*          LVM_LVM_PROGERR
*          LVM_LVM_MAPFOPN
*          LVM_LVM_MAPFSHMAT
*          LVM_LVM_INVALID_MIN_NUM
*          LVM_LVEXIST
*          LVM_NOTCHARDEV
*          LVM_INV_DEVENT
*          LVM_ALLOCERR
*          LVM_DALVOPN
*/
/*****************************************************************************/

int
lvm_createlv(
struct createlv *createlv) /* pointer to struct createlv */

{
    int vgfd;                  /* file descriptor for the volume group */
                               /* mapped file */
    int rc;                    /* holds various return codes */
    int mode;                  /* used to open the mapped file in lvm_getvgdef */
    char *vgptr;               /* volume group file's mapped pointer */
    struct ddi_info  cfgdata;  /* structure passed in to kernel update rtn. */
    struct fheader *fhead;     /* pointer to the file header */

 
/*
*  check that createlv pointer is not null
*/

    if(createlv == NULL)
    {   lvm_errors("createlv","       ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*   call lvm_getvgdef() to lock the file and return the necessary 
*   pointer and file descriptor
*
*/
   mode = O_RDWR|O_NSHARE;
   rc = lvm_getvgdef(&(createlv->vg_id),mode,&vgfd,&vgptr);
   if(rc < LVM_SUCCESS)
   {
       lvm_errors("createlv","createlv",rc);
       free(vgptr);
       return(rc);
   }
/*
*  call create_errorchk() to perform the needed error checks
*/
    rc =  create_errorchk(createlv,vgptr);
    if(rc < LVM_SUCCESS)
    {
	close(vgfd);
        free(vgptr);
        return(rc);
    }

/* call create_updates to update the logical volume header with
*  the createlv structs contents, and to update all other needed
*  information
*/

    rc = create_updates(createlv,vgptr);
    if(rc < LVM_SUCCESS)
    {
	close(vgfd);
        free(vgptr);
        return(rc);
    }

/*
*   call kernel update routine
*   if the routine fails
*     close the mapped file and return an error
*/
    bzero((char *)(& cfgdata.parmlist.kaddlv), sizeof(struct kaddlv));
    cfgdata.parmlist.kaddlv.vg_id.word1 = createlv->vg_id.word1;
    cfgdata.parmlist.kaddlv.vg_id.word2 = createlv->vg_id.word2;
    cfgdata.parmlist.kaddlv.lv_minor = createlv->minor_num;
    if(createlv->permissions == LVM_RDONLY)
       cfgdata.parmlist.kaddlv.lv_options |= LV_RDONLY;
    if(createlv->bb_relocation == LVM_NORELOC)
       cfgdata.parmlist.kaddlv.lv_options |= LV_NOBBREL;
    if(createlv->mirwrt_consist == LVM_NOCONSIST)
       cfgdata.parmlist.kaddlv.lv_options |= LV_NOMWC;
    if(createlv->write_verify == LVM_VERIFY)
       cfgdata.parmlist.kaddlv.lv_options |= LV_WRITEV;
    cfgdata.parmlist.kaddlv.stripe_exp = createlv->stripe_exp;
    cfgdata.parmlist.kaddlv.striping_width = createlv->striping_width;

    fhead = (struct fheader *) vgptr;  
    rc = lvm_config((mid_t)NULL,fhead->major_num,HD_KADDLV,&cfgdata);
    if(rc < LVM_SUCCESS)
    {
	lvm_errors("lvm_config","createlv",rc);
        close(vgfd);
        free(vgptr);
        return(rc);
    }
    

/*
* call lvm_diskio() to commit the new data to the descriptor area on disk 
* 
*/

   rc = lvm_diskio(vgptr,vgfd);
   if(rc < LVM_SUCCESS)
   {
       lvm_errors("lvm_diskio","createlv",rc);
       close(vgfd);
       free(vgptr);
       return(rc);
   }

/*
* call lvm_updversion() if striped lv, to set new characteristics (version
* number) on the pv's
*/
   if (createlv->mirror_policy == LVM_STRIPED) 
	lvm_updversion(vgptr, LVM_STRIPE_ENHANCE);

/*
*  close the volume group mapped file
*/

    close(vgfd);
    free(vgptr);

    return(LVM_SUCCESS);
}

/******************************************************************************/
/* 
*  Name : create_errorchk
*
*  Function - This routine does range checking and other basic checks
*
*  NOTES:
*    Input :
*      createlv
*      vgptr
*
*    Output : If errors are found, and error code is returned and the createlv
*             routine is terminated. 
*
*    Return Value: LVM_SUCCESS if successful, if no successful one of the
*    following
*                LVM_VGFULL
*                LVM_LVEXIST
*                LVM_INVALID_MIN_NUM
*                LVM_INVALID_PARAM 
*                LVM_NOTCHARDEV
*                LVM_INV_DEVENT
*/
/******************************************************************************/
int create_errorchk(
struct createlv *createlv, /* contains info on new lv passed in by user */
char *vgptr)               /* points to the vg mapped file for the new lv */

{   
    struct namelist  *nptr;   /* pointer to the array of lv names  */
    struct vg_header *vghptr; /* pointer to vg header */
    int   prior_exist;        /* indicates that an lv aready exists */
                              /* with the name passed in for the new lv */
    long   counter;           /* counts the lv entries checked */
    int   rc;                 /* holds return codes */
    char devname[LVM_NAMESIZ];/* holds the lv name for stat call */
    char rawname[LVM_EXTNAME];/* holds the complete lv name for stat call */
    struct stat stat_buf;     /* buffer that holds info from stat call */
    int   min_num;	      /* minor number in dev_t for lvname passed in */

/*
*   If the lvname is null
*      return an error
*/
   if(createlv->lvname == NULL) 
   {   lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
   rc = strcmp(createlv->lvname,"");
   if(rc == 0)
   {   lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

/*
*  get_ptrs returns the pointers to the structures in the vg mapped file
*  for all non-null arguments in the argument list 
*/      

    rc = get_ptrs(vgptr,NULL,&vghptr,NULL,NULL,NULL,&nptr);
    if(rc < LVM_SUCCESS)
        return(rc);

/*
*  If(the number of lvs in the volume group is >= the maximum lvs allowed)
*       return(errorcode)
*/


    if(vghptr->numlvs == (vghptr->maxlvs - 1))
    {   lvm_errors("create_errorchk","createlv",LVM_VGFULL);
        return(LVM_VGFULL);
    } 

/* If the minor number is greater than the maximum lvs allowed
*   return an error
*/

   if(createlv->minor_num > (vghptr->maxlvs - 1) || createlv->minor_num <= 0)
   {   lvm_errors("create_errorchk","createlv",LVM_INVALID_MIN_NUM);
       return(LVM_INVALID_MIN_NUM);
   }

/*
*  Search the lv entries for an entry with an lvname that matches the one given
*  If(a match is found)
*    return(errorcode)
*/

   prior_exist = FALSE;
   counter = 1;


   while(prior_exist == FALSE && counter <= vghptr->maxlvs)
   {
       rc = strcmp(nptr->name[counter - 1],createlv->lvname);
       if (rc == 0)
         prior_exist = TRUE;
       else 
          counter++;
   }
   if (prior_exist == TRUE)
   {   lvm_errors("create_errorchk","createlv",LVM_LVEXIST);
       return(LVM_LVEXIST);
   }

/*
*  call status_chk() to be sure that the name passed in for the 
*  logical volume is a raw device and not a block one
*/
   
   rc = status_chk(vgptr,createlv->lvname,NOCHECK,rawname);
   if(rc < LVM_SUCCESS)
     return(rc);
/*
* stat the rawname of the logical volume
* check the minor number passed in to be sure it's valid
*/
   rc = stat(rawname,&stat_buf);
   if(rc < LVM_SUCCESS)
       return(LVM_INV_DEVENT);
   min_num = minor(stat_buf.st_rdev);
   if(min_num != createlv->minor_num)
       return(LVM_INV_DEVENT);
/*
*  If(maxsize is not within its range or is negative)
*     return(errorcode)
*/

   if(createlv->maxsize > LVM_MAXLPS || createlv->maxsize <= 0)
   {  lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
      return(LVM_INVALID_PARAM);
   }

/*
*  If (mirror_policy is not a legal value)
*      return(errorcode)
*/
   if(createlv->mirror_policy != LVM_SEQUENTIAL && createlv->mirror_policy
      != LVM_PARALLEL && createlv->mirror_policy != LVM_STRIPED)
   {   lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
/*
*  If(permissions is not a legal value)
*     return(errorcode)
*/
   if(createlv->permissions != LVM_RDONLY && createlv->permissions != LVM_RDWR)
   {  lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
      return(LVM_INVALID_PARAM);
   }

/*
*  If(bb_relocation is not a legal value)
*     return(errorcode)
*/

   if(createlv->bb_relocation != LVM_RELOC && createlv->bb_relocation !=
         LVM_NORELOC)
   {  lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
      return(LVM_INVALID_PARAM);
   }

/*
*  If mirwrt_consist is not a legal value, return an error
*/

   if(createlv->mirwrt_consist != LVM_CONSIST && createlv->mirwrt_consist !=
         LVM_NOCONSIST)
   {  lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
      return(LVM_INVALID_PARAM);
   }

/*
*   If(write_verify is not a legal value)
*     return(error)
*
*/

   if(createlv->write_verify != LVM_VERIFY && createlv->write_verify != 
       LVM_NOVERIFY)
   {   lvm_errors("create_errorchk","createlv",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }


   return(LVM_SUCCESS);
}
/*****************************************************************************/
/*
*  Name: create_updates
*
*  Function:
*      This routine updates the data structures in the mapped file
*
*  NOTES:
*    Input:
*       createlv
*       vgptr
*
*    Output:
*       Updated data structures in the data structure file
* 
*    Return Value: LVM_SUCCESS if successful, and one of the following
*    if unsuccessful.
*                LVM_INVALID_MIN_NUM
*/
/******************************************************************************/
int create_updates(
struct createlv *createlv, /* contains info on new lv passed in by user */
char *vgptr)               /* points to the vg mapped file for the lv */


{
   struct namelist *nptr; /* points to the name area in the mapped file */
   struct vg_header *vgh; /* points to vg header for descriptor entry */
   struct lv_entries *lv; /* points to the lv entry in the descriptor area */
   int     rc;            /* holds return codes */

/*
*   use get_ptrs routine to get the needed pointers to the desired
*   structures in the volume group mapped file 
*
*/   
     rc = get_ptrs(vgptr,NULL,&vgh,&lv,NULL,NULL,&nptr);
     if(rc < LVM_SUCCESS)
         return(rc);
/*
*    Update the volume group header
*        Increment numlvs
*/

   vgh->numlvs += 1;

/*
*    Update the logical volume entry
*/

/* set lv pointer to point to the correct logical volume entry */

  lv += (createlv->minor_num - 1);

/* check that the lv you're about to fill in is not already
* filled in
*/

  if(lv->lv_state & LVM_LVDEFINED)
  {   lvm_errors("create_updates","createlv",LVM_INVALID_PARAM);
      return(LVM_INVALID_PARAM);
  }

/*
* fill in the lv header fields       
*/

  bzero(nptr->name[createlv->minor_num-1],LVM_NAMESIZ);
  strncpy(nptr->name[createlv->minor_num-1],createlv->lvname,LVM_NAMESIZ - 1);
  lv->lvname = (createlv->minor_num - 1); 
  lv->maxsize = createlv->maxsize;
  lv->lv_state |= LVM_LVDEFINED;
  lv->mirror = 0;
  lv->mirror_policy = createlv->mirror_policy;
  lv->num_lps = LVM_INITIAL_LPNUM; 
  lv->permissions = createlv->permissions;
  lv->bb_relocation = createlv->bb_relocation;
  lv->mirwrt_consist = createlv->mirwrt_consist;
  lv->write_verify = createlv->write_verify;
  lv->stripe_exp = (unsigned short)createlv->stripe_exp;
  lv->striping_width = (unsigned short)createlv->striping_width;

  return(LVM_SUCCESS);
}
/*****************************************************************************/
