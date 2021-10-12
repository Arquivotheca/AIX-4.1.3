static char sccsid[] = "@(#)25	1.42  src/bos/usr/ccs/lib/liblvm/utilities.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:54:15";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: get_lvinfo, get_ptrs, lvm_errors, get_pvandpp, bldlvinfo, 
 *            status_chk, lvm_special_3to2, getstates, timestamp, buildname,
 *	      build_file, lvm_getvgdef
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/hd_config.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/cfgodm.h>
#include <odmi.h>
#include <lvmrec.h>
#include <liblvm.h>

extern void *malloc();
extern int odmerrno;

/* Name getstates
* 
*  Function - This routine calls an ioctl that returns information from
*             from the volume group status area on the physical partition
*             states.
*
* NOTES:
*    Input
*      vgsa
*      vgfptr
*      
*    Output 
*      A copy of the volume group status area
*
* Return Value LVM_SUCCESS if successful or
*              LVM_NOTCHARDEV
*              LVM_INV_DEVENT  
*              LVM_PROGERR
*              LVM_DALVOPN
*              LVM_UNSUCC
*     
*/
/*****************************************************************************/
int
getstates(
struct vgsa_area *vgsa, 	/* pointer to buffer for vgsa */
char             *vgfptr)	/* pointer to volume group mapped file */

{
   char devname[LVM_NAMESIZ + sizeof(LVM_DEV)];
   	    /* will hold full device name of descriptor area logical volume */
   int rc;  /* holds return code */
   struct fheader *fhead;
	    /* pointer to file header of volume group mapped file */
   int fd;  /* file descriptor from open of descriptor area logical volume */

/*
* calculate pointer to volume group mapped file header structure
* so that we can get the volume group or descriptor area logical volume name
*/

   fhead = (struct fheader *)vgfptr;

/*
*  call status_chk() to check and/or build the full name of the volume
*  group to be sure that it's a raw device.
*/

   rc = status_chk(NULL,fhead->vgname,NOCHECK,devname);
   if(rc < LVM_SUCCESS)
     return(rc);

/*
*  since status_chk() succeeded we can attempt to open the descriptor
*  area logical volume (volume group)
*/

   fd = open(devname,O_RDWR, 0);
   if(fd < 0)
      return(LVM_DALVOPN);

/*
*  call the ioctl to get the volume group status area information
*/

   if(ioctl(fd,GETVGSA,(char *)vgsa) != 0) {
      close(fd);
      return(LVM_UNSUCC);
   }
   close(fd);
   return(LVM_SUCCESS);
}
/***************************************************************************/
/*
* Name: get_lvinfo 
*
* Function:  
*   The function of this module is to convert the lv_id into
*   the minor_num (lv_index to the lv map entry in the mapped file) and to
*   return that along with the vg_id for the vg containing the lv and a file
*   descriptor and pointer for the mapped file.
*
* NOTES:
*   Input:
*     lv_id
*
*   Output:
*    minor_num - the index to the lv map entry in the mapped file
*    vgfd - the file descriptor for the vg mapped file.
*    vg_id - the id of the volume group containing the lv
*    vgptr - the pointer to the volume group mapped file
*
* Return Value: LVM_SUCCESS if successful, and one of the following if 
*               unsuccessful:
*                      LVM_INVALID_MIN_NUM 
*                      LVM_OFFLINE
*                      LVM_PROGERR
*                      LVM_MAPFOPN
*                      LVM_MAPFSHMAT
*/
/******************************************************************************/

int get_lvinfo(
   struct lv_id *lv_id,     /* logical volume id */
   struct unique_id *vg_id, /* volume group id */
   short  *minor_num,       /* logical volume minor number */
   int    *vgfd,            /* volume group file descriptor */
   char **vgptr,            /* pointer to the volume group mapped file */
   int     mode)            /* mode to open the vg mapped file with */

{ 
   int rc;               /* holds error codes */
   struct vg_header *vg; /* points to the volume group header */
   struct fheader *fh;   /* points to file header */
/*
*   Set vg_id = the vg_id portion of the lv_id structure
*   Set minor_num = the minor_num portion of the lv_id structure
*   Call lvm_getvgdef to create the vg file name and open it
*/
  (*vg_id).word1  = lv_id->vg_id.word1;
  (*vg_id).word2  = lv_id->vg_id.word2;
  *minor_num = lv_id->minor_num;

  rc =  lvm_getvgdef(vg_id,mode,vgfd,vgptr);
  if(rc < LVM_SUCCESS)
  {  lvm_errors("lvm_getvgdef","get_lvinfo",rc);
     free(vgptr);
     return(rc);
  }
  fh = (struct fheader *)*vgptr;
  vg = (struct vg_header *)((char *)*vgptr + fh->vginx);

  if(*minor_num > (vg->maxlvs - 1) || *minor_num <= 0)
  {   lvm_errors("get_lvinfo","get_lvinfo",LVM_INVALID_MIN_NUM);
      close(*vgfd);
      free(vgptr);
      return(LVM_INVALID_MIN_NUM);
  } 
  free(vgptr);
  return(LVM_SUCCESS);
}
/***************************************************************************/
/*
*  Name: get_ptrs 
*
*  Function:  This routine returns all pointers that are passed in the arg
*                list that are not NULL (0). The pointers are derived by using
*                the shmatted file's pointer and the offsets within that
*                shmatted volume group file.
*  
*  NOTES:
*    Input
*      vgmptr
*      header
*      vgptr
*      lvptr
*      pvptr
*      pp_ptr
*      lvinfoptr
*      pvinfoptr
*      ppinfoptr
*      nameptr
*
*    Output
*       If successful, a pointer for each non null arg sent in with the
*       exception of the vgmptr, which is the shmatted file's pointer
*
* Return Value: LVM_SUCCESS or LVM_INVALID_PARAM
*/
/****************************************************************************/
int get_ptrs(
char *vgmptr,               /* pointer to the beginning of the volume */
                            /* group mapped file */
struct fheader **header,    /* points to the file header */
struct vg_header **vgptr,   /* points to the volume group header */
struct lv_entries **lvptr,  /* points to the logical volume entries */
struct pv_header **pvptr,   /* points to the physical volume header */
struct pp_entries **pp_ptr, /* points to the physical partition entries */
struct namelist **nameptr)  /* points to the name descriptor area */

{
   struct fheader *temp_head; /* If the header pointer is sent in as NULL, */
                              /* we still need a temp_head pointer to access */
                              /* the offsets in order to derive all other */
                              /* pointers to be returned */

/* If the vgmptr is null, it is impossible to return any pointers because
*  they all are derived by adding an offset to this vgmptr; therefore, return
*  an error code and exit the routine.
*/
   if (vgmptr == NULL)
   {   lvm_errors("get_ptrs","get_ptrs",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
   temp_head = (struct fheader *)vgmptr;

/* derive the file header pointer */

   if(header != NULL)
        *header = temp_head;
/* derive the volume group header pointer */

   if(vgptr != NULL)
        *vgptr = (struct vg_header *)(temp_head->vginx + (char *)vgmptr);
/* derive the first logical volume entry pointer */

   if(lvptr != NULL)
        *lvptr = (struct lv_entries *)(temp_head->lvinx + (char *)vgmptr);
/* derive the first physical volume header entry pointer */

   if(pvptr != NULL)
     *pvptr = (struct pv_header *)(temp_head->pvinfo[0].pvinx + (char *)vgmptr);

/* derive the first physical partition entry pointer */

   if(pp_ptr != NULL)
        *pp_ptr = (struct pp_entries *)(temp_head->pvinfo[0].pvinx +
                  (char *)vgmptr + sizeof(struct pv_header));

/* derive the pointer to the name descriptor area */

   if(nameptr != NULL)
        *nameptr = (struct namelist *)(temp_head->name_inx + (char *)vgmptr);

    return(LVM_SUCCESS);
}
/****************************************************************************/
/*
*  Name: lvm_errors
*
*  Function: This routine writes error messages to the screen for the
*               user. The messages consist of the name of the failing routine,
*               the name of the routine that calls it, and the return code. 
*
*  NOTES:
*    Input:
*      failing_rtn
*      calling_rtn
*      rc
*
*    Output: 
*       a message to the screen
*
*    Return Value void 
*/
/***************************************************************************/
int lvm_errors(
char failing_rtn[LVM_NAMESIZ], /* name of routine with error */
char calling_rtn[LVM_NAMESIZ], /* name of routine that calls failing routine */
int  rc)                       /* return code */

{
   int status; /* return code */ 

   status = LVM_SUCCESS;

#ifdef DEBUG

   fprintf(stderr,"The %s routine has failed with a return code of %d.\n",
            failing_rtn,rc);
   fprintf(stderr,"The failed routine was called by the %s routine.\n",
            calling_rtn);

#endif

   return(status);
}
/****************************************************************************/
/* Name get_pvandpp 
* 
*  Function - This routine calculates the physical volume header pointer
*                using the pvindex in the pp info structures for the correct
*                logical partition, and the pp_entry pointer using the map
*                index.
*
* NOTES:
*    Input
*      variable for pvptr
*      variable for ppptr
*      vgfptr
*      lvinfoptr
*      index
*      map
*
*    Output 
*      The physical volume header pointer
*
*
* Return Values: LVM_SUCCESS or LVM_INVALID_PARAM
*     
*/
/***************************************************************************/
int get_pvandpp(
struct pv_header **pv,  /* pointer to the physical volume header */
struct pp_entries **pp, /* pointer to the physical partition entries */
short  *pvnum,           /* the pv number of the physical volume id that */
                        /* was sent in */
char *vgfptr,           /* pointer to the volume group mapped file */
struct unique_id *id)   /* pointer to the id of the physical volume */
                        /* you need a pointer to */

{
   struct fheader *fhead; /* points to the file header */
   long pvcnt;            /* count the pvs  checked */
   int  match;            /* indicates a match */
   struct pv_header *tmp; /* temp pv header pointer */

   fhead = (struct fheader *)vgfptr;
   pvcnt = 0;
   match = FALSE;
   while((pvcnt < LVM_MAXPVS) && (match == FALSE))
   {
     if( fhead->pvinfo[pvcnt].pv_id.word1 == id->word1 &&
         fhead->pvinfo[pvcnt].pv_id.word2 == id->word2)
     {     match = TRUE;
           if(pvnum != NULL)
               *pvnum = pvcnt + 1 ;
     }
     else
        pvcnt ++;
    }
    if(match == FALSE)
    {
        lvm_errors("getpvandpp","red/ext_errchk",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }
    else
    {  if(fhead->pvinfo[pvcnt].pvinx == 0)
       {   lvm_errors("getpvandpp","red/ext_errchk",LVM_INVALID_PARAM);
	   return(LVM_INVALID_PARAM);
       }
       if(pv != NULL)
         (*pv) = (struct pv_header *)
             ((char *)vgfptr + fhead->pvinfo[pvcnt].pvinx);
       if(pp != NULL)
       {  tmp = (struct pv_header *)
             ((char *)vgfptr + fhead->pvinfo[pvcnt].pvinx);
         (*pp) = (struct pp_entries *)((char *)(tmp) +sizeof(struct pv_header));
       }
    }
    return(LVM_SUCCESS);
}
/*************************************************************************/
/* Name bldlvinfo
* 
*  Function - This routine builds a logical view of a logical volume. The
*                view will be grouped by copy number and each entry will contain
*                the physical volume number and physical partition number 
*                of the physical partition for each logical partition.
*
* NOTES:
*    Input
*      variable for pointer to logical view 
*      vgfptr
*      lv
*      cnt  
*      minor_num
*      flag
*      
*    Output 
*      The logical view of the logical volume and an array of integers with
*      an entry for each copy of the logical volume, that indicates the 
*      number of physical partitions for that copy of the logical volume.
*
* Return Value LVM_SUCCESS if successful or LVM_INVALID_PARAM
*     
*/
/***************************************************************************/
int bldlvinfo(
struct logview **lp,   /* pointer to the logical view that will be returned */
char *vgfptr,          /* pointer to the volume group file */
struct lv_entries *lv, /* pointer to the logical volume entry */
long *cnt,             /* pointer to an array of the count of pps */
                       /* per copy of the logical volume */
short minor_num,       /* minor number of logical volume */
int flag)	       /* LVM_GETSTALE if info on stale pps is desired and */
	 	       /* LVM_NOSTALE if not */

{
   int rc;                /* holds return codes */
   int state;		  /* TRUE of FALSE depending on the state of the pp */
   int  index;		  /* index into logical view */
   long pvcnt;            /* counter for physical volumes */
   long ppcnt;            /* counter for phycical partitions */
   long size;             /* size of logical view */
   struct pv_header *pv;  /* pointer to physical volume header */
   struct pp_entries *pp; /* pointer to physical partition entries */
   struct fheader *fhead; /* pointer to the file header */
   struct vg_header *vgh; /* pointer to the volume group header */
   struct vgsa_area vgsa; /* structure for stale partition info */

   cnt[FIRST_INDEX] = 0;
   cnt[SEC_INDEX] = 0;
   cnt[THIRD_INDEX] = 0;
   size = (LVM_NUMCOPIES * lv->maxsize) * sizeof(struct lp);
   (*lp) = (struct logview *)(malloc(size));
   rc = get_ptrs(vgfptr,&fhead,&vgh,NULL,NULL,NULL,NULL);
   if(rc < LVM_SUCCESS)
      return(rc);
   bzero((char *)(*lp),size);

   /* 
    * if the flag indicates states are desired, then call getstates()  
    * to get a copy of the volume group status area to be used later
    */
    
     if(flag == LVM_GETSTALE) {
	rc = getstates(&vgsa,vgfptr);
        if(rc < LVM_SUCCESS)
        	return(rc);
     }    
/*
*  search each physical volume in the volume group
*/
       for(pvcnt = 0; pvcnt < LVM_MAXPVS; pvcnt++)

/* calculate the physical volume header pointer */

       {   if(fhead->pvinfo[pvcnt].pvinx != 0)
           {   pv = (struct pv_header *)((char *)vgfptr +
                    fhead->pvinfo[pvcnt].pvinx);
               pp =(struct pp_entries *)((char *)pv + sizeof(struct pv_header));

/*
*  search all physical partition entries for this physical volume
*/
               for(ppcnt = 1; ppcnt <= pv->pp_count; ppcnt++)

/*  
* if the partition belongs to the logical volume that you want a 
* logical view of, then fill in an entry in the logical view array
* for this partition
*/

               {   if(pp->lv_index == minor_num) {
/*
*  based on the copy field in the pp entry fill in the correct entry in the
*  logical view array
*/
                      switch(pp->copy)
                      {
                        case LVM_PRIMARY:
                        case LVM_PRIMOF2:
                        case LVM_PRIMOF3: index = FIRST_INDEX;
		   		          break;
                        case LVM_SCNDOF2:
                        case LVM_SCNDOF3: index = SEC_INDEX;
 		   	                  break;
                        case LVM_TERTOF3: index = THIRD_INDEX;
 		 	                  break;
                      }
                      (*lp)->larray[pp->lp_num-1][index].ppnum = ppcnt;
                      (*lp)->larray[pp->lp_num-1][index].pvnum = pv->pv_num; 
                      (*lp)->larray[pp->lp_num-1][index].ppstate = pp->pp_state;
                      cnt[index] += 1;
		     /* if the flag indicates that states are desired */
		      if(flag == LVM_GETSTALE) {
			      /* 
			       * use the TSTSA_STLPP macro to check the state
			       * of the pp and return TRUE if the pp is stale
			       */
			       state =
				  (TSTSA_STLPP(&vgsa,pv->pv_num-1, ppcnt-1) ?
				  TRUE : FALSE);
			       if(state == TRUE)
                                    (*lp)->larray[pp->lp_num-1][index].ppstate 
						|= LVM_PPSTALE;
		       }
		   }
/*
* increment the pointer to the next physical partition entry for this
* physical volume
*/

                   pp++;
               }
            }
        }

     return(LVM_SUCCESS);
}
/*****************************************************************************/
/* Name status_chk
* 
*  Function - This routine calls stat() and checks that a device
*                is the raw device and not the character device. It also
*                checks the major number of the device.
*
* NOTES:
*    Input
*      vgptr
*      name
*      flag
*      rawname
*      
*    Output 
*      The logical view of the logical volume and an array of integers with
*      an entry for each copy of the logical volume, that indicates the 
*      number of physical partitions for that copy of the logical volume.
*
* Return Value LVM_SUCCESS if successful or
*              LVM_NOTCHARDEV
*              LVM_INV_DEVENT  
*              LVM_PROGERR
*     
*/
/***************************************************************************/
int status_chk(
char *vgptr,          /* pointer to volume group mapped file */
char *name,           /* the name of the device to be checked */
int  flag,            /* indicates that the major number should be checked */
char *rawname)        /* pointer to new raw device name */
{
   int rc;                /* return code */
   long major_num;        /* major number of device returned from stat() */
   struct fheader *fhead; /* pointer to vg mapped file header */
   struct stat stat_buf;  /* buffer that holds info from stat call */
   char *n, *s, *slash, *rindex();
                          /* temporary variables used to build raw */
                          /* device name */
   char newname[LVM_EXTNAME];
                          /* name of the raw device */
   char devname[LVM_EXTNAME];
                          /* full path name of device to stat */


/* zero out all names to ensure an ending null symbol */

   bzero(newname,LVM_EXTNAME);
   bzero(devname,LVM_EXTNAME);
   if(rawname != NULL)
      bzero(rawname,LVM_EXTNAME);

/* 
*  If the name passed in starts with a "/" then assume that it is not
*  in /dev. Simply copy the name to the devname string to be checked
*/
   if(name[FIRST_INDEX] == LVM_SLASH)
       strncpy(devname,name,LVM_NAMESIZ - 1);
/*
*  if the name does not start with a "/" then assume that the device
*  is located in the /dev directory. Build the full path name in
*  the devname string to be checked by the stat call
*/
   else
   {   strcpy(devname,LVM_DEV);
       strcat(devname,name);
   }

/*
*  call the stat system call to get information on the device 
*/

   rc = stat(devname,&stat_buf);
   if(rc < LVM_SUCCESS)
      return(LVM_INV_DEVENT);

/*
*  if the device is not a character device
*    build a raw device name
*/

   if(!(S_ISCHR(stat_buf.st_mode)))
/*
*  call rindex() to search from the end of the name to find the first "/"
*/

   {   if((slash = rindex(devname,LVM_SLASH)) == NULL)
           return(LVM_INV_DEVENT);
       else
/* 
*  copy devname into the newname up to and including the slash
*/
       {  for(n = newname, s = devname; s <= slash ;)
             *n++ = *s++;

/* insert an "r" for the raw entry */

          *n++ = 'r';
/*
*  copy the rest of devname to newname
*/
          for(; *s != LVM_NULLCHAR ;)
             *n++ = *s++;

/* add the null byte to the end of the string */

          *n = LVM_NULLCHAR;
        }
            

/*
*  call the stat system call to see if the newname is a valid entry for
*   a raw device
*/

       rc = stat(newname,&stat_buf);
       if(rc < LVM_SUCCESS)
           return(LVM_INV_DEVENT);

/*
*  if the device is not a character device
*    return an error
*/

        if(!(S_ISCHR(stat_buf.st_mode)))
             return(LVM_NOTCHARDEV);
/* 
*   if the newname is a valid raw device name and rawname is not null
*   then copy it to the rawname string to be returned
*/
        if(rawname != NULL)      
            strncpy(rawname,newname,LVM_EXTNAME-1);
   }
   else
   {
/* 
*  since the device was a raw one, if rawname is not null,then
*  copy devname to the rawname that will be returned
*/
        if(rawname != NULL)
            strncpy(rawname,devname,LVM_EXTNAME-1);
    }
/* 
*  if the flag indicates that the major number should be checked
*  then use the system macro to get the major number from the devt returned
*  from stat.
*/

   if(flag == CHECK_MAJ  && vgptr == NULL)
        return(LVM_PROGERR);
   if(flag == CHECK_MAJ)
   {   major_num = major(stat_buf.st_rdev);
       fhead = (struct fheader *)vgptr;
/*
*  compare the major number of the device with the major number 
*  stored in the volume group mapped file
*/

       if(major_num != fhead->major_num) 
          return(LVM_INV_DEVENT);
   }

   return(LVM_SUCCESS);
}
/************************************************************************/
/* Name lvm_special_3to2
* 
*  Function - This routine writes the vgdas in the special order necessary
*             for the special case of going from 3 to 2 totalvgdas
*
* NOTES:
*    Input
*      pvname
*      vgid
*      lvfd
*      vgdaptr
*      da0
*      da1
*      da2
*      newcurcnt
*      
*    Output 
*      The correct number of volume group descriptor areas will be written
*      to the correct hardfiles in the volume group
*
* Return Value LVM_SUCCESS if successful or
*     
*/
/***************************************************************************/
int lvm_special_3to2(
char   		*pvname,	/* name of physical volume being removed */
struct unique_id *vgid, 	/* pointer to volume group id */
int      	lvfd,		/* reserved area logical volume file desc */
char            *vgdaptr,	/* pointer to vgda area of vg file */
short           pvnum0,		/* number of pv that will be removed/deleted */ 
short           pvnum1,		/* number of pv that will be keep one copy */ 
short           pvnum2,		/* number of pv that will have two copies */ 
char            *match,		/* indicates a match in the vgid on the */
				/* lvm record with the vgid passed in */
char		delete,		/* indicates we are called from deletepv */
struct fheader  *fhead)		/* pointer to mapped file header */

{

int		  status;	/* return code */
struct vg_trailer *trail;       /* pointer to volume group trailer */
int                das;		/* counter for descriptor areas on a disk */

        /* calculate the vg trailer pointer for use later */
	trail = (struct vg_trailer *) (fhead->trailinx + (char *)fhead);
	/*
	* if we are not called by deletepv 
	*   	call lvm_cmplvmrec() 
	*	if the vgids match
	*		try to write the vgda on the pv being removed
	*/

	if(delete == FALSE && pvname != NULL) {
		/* call lvm_cmplvmrec() */
         	lvm_cmplvmrec(vgid,match,pvname);

 	       /* 
         	* if the vgids match, write a vgda
         	* to the pv being removed
        	*/
					  
       		if(*match == TRUE)
	    	   status = lvm_wrtnext(lvfd, (caddr_t)vgdaptr,
			       		&(trail->timestamp), pvnum0,
				 	fhead, (short int)1);
		   if(status < LVM_SUCCESS)
			return(status);
	}

       /* write vgda on pv to have 1 copy */

       status = lvm_wrtnext(lvfd, (caddr_t)vgdaptr, &(trail->timestamp),
			 pvnum1, fhead, (short int)1); 
       if(status < LVM_SUCCESS)
	  return(status);

  	/* write the vgdas on the pv to have 2 copies */

	status = lvm_wrtnext(lvfd,(caddr_t)vgdaptr,&(trail->timestamp),pvnum2,
				fhead,(short int)2);
       	if(status < LVM_SUCCESS)
	   return(status);                                       
       return(LVM_SUCCESS);
}
/*************************************************************************/
/* Name timestamp
* 
*  Function - This routine gets the current system time and puts it in 
*             the volume group  header and trailer
*
* NOTES:
*    Input
*      vg
*      vgptr
*      fhead
*      
*    Output 
*      updated timestamps in the volume group header and trailer
*
* Return Value LVM_SUCCESS if successful or
*              LVM_PROGERR
*     
*/
/*****************************************************************************/
int
timestamp(
   struct vg_header *vg, 	/* pointer to volume group header */
   char 	    *vgptr,     /* pointer to volume group file */
   struct fheader   *fhead)     /* pointer to file header of vg file */

{
	int rc;				/* holds return code */
	struct vg_trailer *trail;	/* pointer to vg trailer */
	struct timestruc_t cur_time;	/* contains time from system clock */
 
	/* get the current time from the system clock */
   	rc = gettimer(TIMEOFDAY,&cur_time);
   		if(rc == LVM_UNSUCC)
			return(LVM_PROGERR);
	/* if the current time <= time stored in VGDA */
  	if(cur_time.tv_sec < vg->vg_timestamp.tv_sec ||
          (cur_time.tv_sec == vg->vg_timestamp.tv_sec &&
           cur_time.tv_nsec <= vg->vg_timestamp.tv_nsec))

	  	/* increment the current time by a second */
		  cur_time.tv_sec = vg->vg_timestamp.tv_sec + 1;

	/* update timestamp in the vg header and trailer*/
  	vg->vg_timestamp = cur_time;
  	trail = (struct vg_trailer *)(fhead->trailinx + 
			  (char *)vgptr);
  	trail->timestamp = cur_time;
        return(LVM_SUCCESS);
}
/****************************************************************************/
/* Name buildname   
* 
*  Function - This routine builds a physical volume name and node entry
*
* NOTES:
*    Input
*      dev
*      name
*      mode
*      type
*      
*    Output 
*      a node entry in /dev for the physical volume and a new name for the
*      LVM to use.
*
* Return Value LVM_SUCCESS if successful or
*              LVM_PROGERR
*     
*/
/*****************************************************************************/
int
buildname(
   dev_t  	dev,			/* device number of pv needing name */
   char         name[LVM_EXTNAME],     	/* array to store name in */
   int          mode,     		/* mode to set the device entry to */
   int          type)                   /* type of name to build */

{
	int rc;				/* holds return code */
	int minor_num;             	/* minor number of physical volume */
	long major_num;              	/* major number of physical volume */
 
	/* 
	* get the major and minor number from the dev_t sent in
	* so that we can use them in the name
	*/
 	minor_num = minor(dev);
	major_num = major(dev);
	/* use the macros in liblvm.h to build the pv or vg name */
       	switch(type) {
 		case LVM_PVNAME : LVM_BUILDNAME(name,major_num,minor_num);
				  break;
		case LVM_VGNAME : LVM_BUILDVGNAME(name,major_num);
				  break;
	}	
	/* make a node entry for the device in /dev */
	rc = mknod(name,mode,dev);
	/* if mknod fails, return an appropriate error code */
	if((rc < LVM_SUCCESS) && (errno != EEXIST))
	    return(LVM_PROGERR);
        else
            return(LVM_SUCCESS);
}
/****************************************************************************/
/* Name lvm_getvgdef     
* 
*  Function - This routine gets volume group information
*             from physical device and also uses as a lock
*             to serialize access to information area
*
* NOTES:
*    Input
*      vgid
*      mapf_mode
*      
*    Output 
*      vgfd
*      vgmap_ptr
*
* Return Value LVM_SUCCESS if successful or
*              LVM_PROGERR
*     
*/
/*****************************************************************************/
int
lvm_getvgdef(
   struct unique_id  *vgid,	/* pointer to volume group id */
   int               mapf_mode,
   int               *vgfd,    /* vg file descriptor */
   caddr_t	     *vgmap_ptr)

{
	int rc;				/* holds return code */
	long major_num;              	/* major number of physical volume */
        struct rebuild rebuild;		/* structure returned from kernel */
	mid_t kmid;			/* kernel module id */
        struct queryvgs *queryvgs;	/* structure to hold info returned */
					/* from queryvgs */
 	struct ddi_info cfgdata;	/* structure passed into kernel */   
        int cnt;			/* counter used in for loops */
	char pvname[LVM_NAMESIZ];	/* name of physical volume */
	char vgname[LVM_NAMESIZ];       /* name of volume group */
        int pvmode;			/* permissions for physical volume */
	int vgmode;			/* permissions for volume group */
	char *vgptr;			/* pointer to vg file we're building */
        dev_t vgdev;			/* device number for volume group */
	struct fheader *fhead;		/* pointer to vg file header */
	struct vg_header *vg;		/* points to the volume group header */
	struct pv_header *pv;		/* points to the volume group header */
        char vgfn[sizeof(LVM_ETCVG) + 2 * sizeof(struct unique_id)];
					/* buffer for volume group file name */
	int size, offset;		/* calculation variables */
        long length;			/* length of the vg file */
	struct lvm_rec lvm_rec;
	long partlen_blks;              /* the length of a partition 
					   in 512 byte blocks */
	char build_pvname[LVM_NAMESIZ];	/* name of physical volume */
 
        vgptr = NULL;
        major_num = -1;
	/* get the kernel module id to pass into lvm_queryvgs */
	kmid = (mid_t)loadext(LVM_LVDDNAME,FALSE,TRUE);
        /* if we couldn't get the kmid, return an error */  
        if(kmid == NULL)
		return(LVM_PROGERR);

	/* call lvm_queryvgs to to get a list of varied-on volume groups */
	rc = lvm_queryvgs(&queryvgs,kmid);
	if(rc < LVM_SUCCESS) 
		return(rc);

	/* search the ouput for a vgid that matches the one passed in */
	for(cnt = 0; cnt < queryvgs->num_vgs; cnt ++) {
		if(queryvgs->vgs[cnt].vg_id.word1 == vgid->word1 &&
			queryvgs->vgs[cnt].vg_id.word2 == vgid->word2) {
				major_num = queryvgs->vgs[cnt].major_num;
				break;
		}
     	}
	lvm_freebuf(queryvgs);
	/* if there was no matching vgid, return an error */
	if(major_num == -1)
	{
		return(LVM_OFFLINE);
	}

	/* create the name of the /etc/vg/vgXXXX file */
	LVM_MAPFN(vgfn,vgid);

	/*
 	*	if we opening the mapped file non-shared, be sure
 	*	to delay instead of just bomb
	*/
	if (mapf_mode & (O_RSHARE | O_NSHARE))
		mapf_mode |= O_DELAY;

	*vgfd = open (vgfn, mapf_mode, LVM_MAPFPERM);

	if ((*vgfd == LVM_UNSUCC) && (errno == ETXTBSY))
		return (LVM_MAPFBSY);
	if ((*vgfd == LVM_UNSUCC) && (errno != ENOENT))
		return (LVM_MAPFOPN);

	/* if /etc/vg/vgXXX file not exist, create it */
	if ((*vgfd == LVM_UNSUCC) && (errno == ENOENT))
	{
		*vgfd = open(vgfn,O_CREAT|O_RDWR|O_TRUNC|O_NSHARE,LVM_MAPFPERM);
		if(*vgfd == LVM_UNSUCC)
		{
			unlink(vgfn);
			return(LVM_MAPFOPN);
		}
	}

	/* zero out the structures that will be sent to the kernel */
        bzero((char *)(&cfgdata.parmlist.krebuild),sizeof(struct krebuild));
	bzero((char*)&rebuild,sizeof(struct rebuild));
        cfgdata.parmlist.krebuild.rebuild = &rebuild;
	/* call lvm_config() with the HD_KREBUILD request */
	rc = lvm_config(NULL,major_num,HD_KREBUILD,&cfgdata);
	if(rc < LVM_SUCCESS)
	{
		close(*vgfd);
		return(rc);
	}
	/* process the pvs */
	for(cnt=0; cnt < LVM_MAXPVS; cnt++) {
	 	/* if the pv is active */
		if(!(rebuild.pv[cnt].pvstate & PV_MISSING)) {
		    if(rebuild.pv[cnt].device != 0) {
			/* set up the permissions for the physical volume */
			pvmode = S_IFCHR|S_IRUSR|S_IWUSR;
			/* call buildname() to build the pv name */
			rc = buildname(rebuild.pv[cnt].device,pvname,
				pvmode,LVM_PVNAME);
			/* if buildname() fails, continue to next pv */
			if(rc < LVM_SUCCESS)
				continue;
			/* call lvm_getpvda() to get the vgda from this pv */
			rc = lvm_getpvda(pvname,&vgptr,TRUE);
			/* if lvm_getpvda fails, go on to next pv */
			if(rc < LVM_SUCCESS)
				continue;
 			else
				/* if we got a good VGDA, break out of loop */
				break;
	            }			 
		}
	}
	/* if we haven't found a good VGDA copy return MAPFRDWR */
	if(vgptr == NULL)
	{
		close(*vgfd);
		return(rc);
	}
	*vgmap_ptr=vgptr;
	/* since we have our vg file, get the needed pointers */
	get_ptrs(vgptr,&fhead,&vg,NULL,NULL,NULL,NULL);

	/* call buildname() to build the volume group name */
	vgmode = S_IFCHR|S_IRUSR|S_IWUSR;
	vgdev = makedev(major_num,0);
	rc = buildname(vgdev,vgname,vgmode,LVM_VGNAME);
	if(rc < LVM_SUCCESS)
	     	fhead->vgname[0] = '\0';
	else
		strncpy(fhead->vgname,vgname,LVM_NAMESIZ-1);
        /* 
	 * go through structure returned from kernel and fill in the 
	 * remaining fields of the fileheader
 	*/
	fhead->major_num = major_num;
	fhead->vgda_len = (long)vg->vgda_size;
	fhead->quorum_cnt = rebuild.quorum;
	fhead->num_desclps = rebuild.num_desclps;
	partlen_blks = LVM_PPLENBLKS (vg -> pp_size);
	for(cnt = 0; cnt < LVM_MAXPVS; cnt ++) {
		pv = (struct pv_header *)(vgptr +
		 	fhead->pvinfo[cnt].pvinx);
		if(!(rebuild.pv[cnt].pvstate & PV_MISSING))
		   if(rebuild.pv[cnt].device != 0) {
			fhead->pvinfo[cnt].device = rebuild.pv[cnt].device;
			fhead->pvinfo[cnt].pv_id.word1 = pv->pv_id.word1;
			fhead->pvinfo[cnt].pv_id.word2 = pv->pv_id.word2;
			buildname(rebuild.pv[cnt].device,build_pvname,
				pvmode,LVM_PVNAME);
			rc = getlvmrec(build_pvname,&lvm_rec);
			if (rc < LVM_SUCCESS)
			{
				unlink(build_pvname);
				continue;
			}
	  		fhead->pvinfo[cnt].da[0].dalsn = 
			fhead->num_desclps * partlen_blks * (pv->pv_num-1)
       			+ lvm_rec.vgda_psn [0];
			fhead->pvinfo[cnt].da[1].dalsn = 
       			fhead->num_desclps * partlen_blks * (pv->pv_num-1)
       			+ lvm_rec.vgda_psn [1];
			unlink(build_pvname);
		    }
	}
	unlink(pvname);
        return(LVM_SUCCESS);
}

/****************************************************************************/
/* Name getlvmrec
* 
*  Function - This routine read the lvm record from 
*             available physical volume
*
* NOTES:
*    Input
*      pv_name
*      
*    Output 
*      lvm_rec_ptr
*
* Return Value LVM_SUCCESS if successful or
*    	       LVM_PVOPNERR open device fail
*     
*/
/*****************************************************************************/
int
getlvmrec(
char * pv_name,
struct lvm_rec * lvm_rec_ptr)
{
int retcode;
int pv_fd;
char devname [LVM_EXTNAME];

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */
retcode = status_chk (NULL, pv_name, NOCHECK, devname);

if (retcode < LVM_SUCCESS)
    return (retcode);
pv_fd = open (devname, O_RDONLY);

if (pv_fd == LVM_UNSUCC)
    return (LVM_PVOPNERR);

retcode = lvm_rdlvmrec (pv_fd, lvm_rec_ptr);
  /* call routine to read the LVM record from the physical volume */

close(pv_fd);
return (retcode);
} /* END getlvmrec */


/****************************************************************************/
/* Name putlvmrec
* 
*  Function - This routine writes the lvm record to 
*             available physical volume
*
* NOTES:
*    Input
*      pv_name
*      lvm_rec_ptr
*
* Return Value LVM_SUCCESS if successful or
*    	       LVM_PVOPNERR open device fail
*     
*/
/*****************************************************************************/
int
putlvmrec(
char * pv_name,
struct lvm_rec * lvm_rec_ptr)
{
int retcode;
int pv_fd;
char devname [LVM_EXTNAME];

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */
retcode = status_chk (NULL, pv_name, NOCHECK, devname);

if (retcode < LVM_SUCCESS)
    return (retcode);
pv_fd = open (devname, O_RDWR);

if (pv_fd == LVM_UNSUCC)
    return (LVM_PVOPNERR);

retcode = lvm_wrlvmrec (pv_fd, lvm_rec_ptr);
  /* call routine to write the LVM record to the physical volume */

close(pv_fd);
return (retcode);
} /* END putlvmrec */


/****************************************************************************/
/* Name putodm_cuat
* 
*  Function - This routine update the odm attribute
*
* NOTES:
*    Input
*      name
*      attrubute
*      value
*
*/
/*****************************************************************************/
putodm_cuat(
char *name, 
char *attribute, 
char *value)
{
struct CuAt *cuatp;
int num = 1;

    odm_initialize();
    odm_set_path(CFGPATH);
    cuatp = getattr(name, attribute, FALSE, &num);
    if (cuatp != NULL) {
	bzero(cuatp->value, 256);
	strcpy(cuatp->value, value);
	putattr(cuatp);
    }
    odm_terminate();
}

