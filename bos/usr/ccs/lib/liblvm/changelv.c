static char sccsid[] = "@(#)61	1.25  src/bos/usr/ccs/lib/liblvm/changelv.c, liblvm, bos411, 9428A410j 2/22/93 16:16:19";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_changelv
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
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <sys/hd_config.h>
#include <sys/dasd.h>
#include <liblvm.h>

/*****************************************************************************
*  Name : lvm_changelv 
*
*  Function:  This function is called by a system management command to
*  change the attributes of a logical volume.
*
*  NOTES:
*
*    Input:  
*     changelv
*
*    Output:
*      If successful, the mapped data structure file and the descriptors
*      on the hard file will be updated; otherwise, an error code is returned.
*
*    Return Values: If successful, the routine returns LVM_SUCCESS. If the
*    routine fails, one of the following is returned.
*          LVM_INVALID_PARAM
*	   LVM_INVALID_MIN_NUM
*          LVM_ALLOCERR
*          LVM_LVOPEN
*          LVM_PROGERR
*          LVM_DALVOPN
*          LVM_MAPFSHMAT
*          LVM_MAPFOPN
*          LVM_OFFLINE
*          LVM_NOTCHARDEV
*          LVM_INV_DEVENT
*/
/*****************************************************************************/
int
lvm_changelv (
struct changelv *changelv) /* pointer to  struct changelv */
  {
    struct fheader   *fhptr;     /* fhptr points to the file header         */
    struct unique_id vg_id;      /* variable to volume group id             */
    struct lv_entries *lvptr;    /* pointer to the logical volume entries   */
    struct namelist *nmlistptr;  /* pointer to the logical volume names     */
    int map_fd;                  /* map file descriptor                     */
    int status = LVM_SUCCESS;    /* error code for lvm_changelv             */
    char *mapptr;                /* pointer to the mapped file              */
    short minor_num;             /* minor_number of logical volume          */
    struct ddi_info cfgdata;     /* structure to send to kernel routine     */
    char rawname[LVM_EXTNAME];   /* new name or lv */
    int check_min;               /* minor number in dev entry for new lvname */
    struct stat stat_buf;        /* buffer that holds info from stat call */
    struct vg_header *vgh;       /* pointer to volume group header struct */
    long prior_exist, counter;   /* used to see if a duplicate lvname was */
    				 /* passed in */
    int  mode;                   /* mode to open vg mapped file with */
  /****************************************************************************/

/*
*  check to be sure that the changelv pointer is not null
*/

    if(changelv == NULL)
    {   lvm_errors("changelv","    ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*  call get_lvinfo to get get the volume group id and minor number, and to 
*  check the validity of the volume group id and minor number, and to
*  open the volume group mapped file and shmat it
*/
  mode = O_RDWR | O_NSHARE;
  status = get_lvinfo(&(changelv->lv_id),&vg_id,&minor_num,&map_fd,&mapptr,
                       mode);
  if(status < LVM_SUCCESS)
      return(status);

/* Call get_ptrs to get the pointers to the structures in the volume 
*  group  mapped file.
*/


    if((status = get_ptrs(mapptr,&fhptr,&vgh,&lvptr,NULL,NULL,&nmlistptr)) 
        < LVM_SUCCESS )
    {   close(map_fd);
        return(status);
    }

    lvptr += minor_num - 1;
    
    if(lvptr->lv_state == LVM_LVUNDEF)
    {   lvm_errors("changelv","      ",LVM_INVALID_PARAM);
        close(map_fd);
        return(LVM_INVALID_PARAM);
    }



/* Validate data structure fields in struct changelv 
* If fields not ok, return LVM_INVALID_PARAM.
*/

    if(changelv->mirror_policy != LVM_NOCHNG)
    {   if(changelv->mirror_policy != LVM_SEQUENTIAL &&
          changelv->mirror_policy != LVM_PARALLEL)
        {   lvm_errors("changelv","  ",LVM_INVALID_PARAM);
            close(map_fd);
            return(LVM_INVALID_PARAM);
        }
        lvptr->mirror_policy = changelv->mirror_policy;
    }

    if(changelv->maxsize < 0 || changelv->maxsize  >  LVM_MAXLPS )
    {   lvm_errors("changelv","    ",LVM_INVALID_PARAM);
        close(map_fd);
        return(LVM_INVALID_PARAM);
    }
    if(changelv->maxsize != LVM_NOCHNG)
       lvptr->maxsize = changelv->maxsize;

    if(changelv->permissions != LVM_NOCHNG)
    {   if(changelv->permissions != LVM_RDONLY &&
           changelv->permissions != LVM_RDWR)
       {   lvm_errors("changelv","      ",LVM_INVALID_PARAM);
           close(map_fd);
           return(LVM_INVALID_PARAM);
       }
       lvptr->permissions   = changelv->permissions;
    }

    if(changelv->bb_relocation != LVM_NOCHNG)
    {  if(changelv->bb_relocation != LVM_RELOC &&
          changelv->bb_relocation != LVM_NORELOC)
       {   lvm_errors("changelv","      ",LVM_INVALID_PARAM);
           close(map_fd);
           return(LVM_INVALID_PARAM);
       }
       lvptr->bb_relocation = changelv->bb_relocation;
     }

    if(changelv->mirwrt_consist != LVM_NOCHNG)
    {  if(changelv->mirwrt_consist != LVM_CONSIST &&
          changelv->mirwrt_consist != LVM_NOCONSIST)
       {   lvm_errors("changelv","      ",LVM_INVALID_PARAM);
           close(map_fd);
           return(LVM_INVALID_PARAM);
       }
       lvptr->mirwrt_consist = changelv->mirwrt_consist;
     }
    if(changelv->lvname != NULL)
    {  if((status = strcmp(changelv->lvname,"")) == 0)
       {   lvm_errors("changelv","      ",LVM_INVALID_PARAM);
           close(map_fd);
           return(LVM_INVALID_PARAM);
       }
/*
* if a name is specified, check to be sure that it not the name of
* another logical volume in the volume group
*/

       prior_exist = FALSE;
       counter = 1;
       while(prior_exist == FALSE && counter <= vgh->maxlvs)
       {   status = strcmp(nmlistptr->name[counter-1],changelv->lvname);
           if(status == 0)
              prior_exist = TRUE;
           else
              counter ++;
       }
/*
*  if the name specified is already in the volume group, return an error
*/
       if(prior_exist == TRUE)
       {   close(map_fd);
           return(LVM_LVEXIST);
       } 

/*
*  check to be sure the lvname is for a raw device. Do this by
*  calling status_chk().
*/
       status = status_chk(mapptr,changelv->lvname,NOCHECK,rawname);
       if(status < LVM_SUCCESS)
       {  close(map_fd);
          return(status);
       }
/*
*  stat the new name of the logical volume
*  check the minor number from this entry with the minor number
*  of the old dev entry.
*  if they are not equal return an error
*/

       status = stat(rawname,&stat_buf);
       if(status < LVM_SUCCESS)
       {  close(map_fd);
          return(LVM_INV_DEVENT);
       }
       check_min = minor(stat_buf.st_rdev);
       if(check_min != minor_num)
       {  close(map_fd);
          return(LVM_INV_DEVENT);
       }
       strncpy(nmlistptr->name[lvptr->lvname],changelv->lvname,LVM_NAMESIZ - 1);
    }

   if(changelv->write_verify != LVM_NOCHNG)
   {   if(changelv->write_verify != LVM_VERIFY && changelv->write_verify !=
           LVM_NOVERIFY)
       {   lvm_errors("changelv","     ",LVM_INVALID_PARAM);
           close(map_fd);
           return(LVM_INVALID_PARAM);
       }   
       lvptr->write_verify = changelv->write_verify;
    }


       
 


/* 
*   update the kernel data structures only if mirror_policy, permissions,
*   bb_relocation,  write_verify, or mirwrt_consist is changed
*
*/



   if(changelv->mirror_policy != LVM_NOCHNG || 
        changelv->permissions != LVM_NOCHNG ||
        changelv->bb_relocation != LVM_NOCHNG ||
        changelv->write_verify != LVM_NOCHNG ||
        changelv->mirwrt_consist != LVM_NOCHNG)
   {
       bzero((char *)(& cfgdata.parmlist.kchglv), sizeof(struct kchglv));
       cfgdata.parmlist.kchglv.vg_id.word1 = vg_id.word1;
       cfgdata.parmlist.kchglv.vg_id.word2 = vg_id.word2;
       cfgdata.parmlist.kchglv.lv_minor = minor_num;
       if(changelv->permissions == LVM_RDONLY)
           cfgdata.parmlist.kchglv.lv_options |= LV_RDONLY;
       if(changelv->bb_relocation == LVM_NORELOC)
           cfgdata.parmlist.kchglv.lv_options |= LV_NOBBREL;
       if(changelv->mirwrt_consist == LVM_NOCONSIST)
           cfgdata.parmlist.kchglv.lv_options |= LV_NOMWC;
       if(changelv->write_verify == LVM_VERIFY)
           cfgdata.parmlist.kchglv.lv_options |= LV_WRITEV;

       if(lvptr->mirror == ONE_COPY || lvptr->mirror == NO_COPIES)
           cfgdata.parmlist.kchglv.i_sched = SCH_REGULAR;
       if(lvptr->mirror_policy == LVM_SEQUENTIAL)
           cfgdata.parmlist.kchglv.i_sched = SCH_SEQUENTIAL;
       if(lvptr->mirror_policy == LVM_PARALLEL)
           cfgdata.parmlist.kchglv.i_sched = SCH_PARALLEL;
        status = lvm_config(NULL,fhptr->major_num,HD_KCHGLV,&cfgdata);
        if(status < LVM_SUCCESS)
        {   lvm_errors("lvm_config","changelv",status);
            close(map_fd);
            return(status);
        }
   }



/* Call lvm_diskio to write the volume descriptor area.
* If lvm_diskio failed close the file, and return rc.
*/

     if((status = lvm_diskio(mapptr,map_fd))  <  LVM_SUCCESS )
     {

       close(map_fd);
       lvm_errors("changelv","        ",status);
       return(status);

     }

/*
*  close the volume group mapped file and return
*/

    close(map_fd);

    return(LVM_SUCCESS);



   /***** end of changelv ************************************************/
 }
