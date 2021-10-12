static char sccsid[] = "@(#)59	1.29.1.1  src/bos/usr/ccs/lib/liblvm/querylv.c, liblvm, bos411, 9428A410j 3/4/94 17:32:27";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_querylv, list
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

#include <sys/stat.h>
#include <fcntl.h>
#include <liblvm.h>


int list(
 int		    *stale,	/* indicates a stale logical volume */
 char		    *vgptr,	/* pointer to volume group mapped file */
 struct querylv	    *querylv,	/* pointer to the querylv structure */
 struct logview     *lp,	/* pointer to the logical view of the lv */
 struct lv_entries  *lv,	/* pointer to the logical volume entry */
 struct fheader     *header);	/* pointer to the mapped file header */
/*****************************************************************************
* Name : lvm_querylv 
*
*  Function:  This function is called by a system management command to 
*  return the information listed below as output for the logical volume 
*  specified.
*
*  NOTES:
*    Input:
*      lv_id     
*      querylv 
*      pvname
*
*    Output:
*      lvname
*      vg_id   
*      maxsize 
*      mirror_policy
*      lv_state    
*      currentsize 
*      ppsize     
*      permissions
*      bb_relocation
*      primary   
*      m1           
*      m2    
*
*   Return Values : If successful, LVM_SUCCESS is returned. If the routine
*   fails, one of the following is returned.
*        LVM_OFFLINE
*        LVM_INVALID_MIN_NUM
*        LVM_ALLOCERR
*        LVM_INVALID_PARAM
*        LVM_NOTVGMEM
*        LVM_NOPVVGDA
*        LVM_PROGERR
*        LVM_PVDAREAD
*        LVM_LVMRECERR
*        LVM_PVOPNERR
*        LVM_NOTCHARDEV
*        LVM_MAPFOPN
*        LVM_MAPFSHMAT
*        LVM_DALVOPN
*/

/***************************************************************************/
int 
lvm_querylv(
struct lv_id *lv_id,       /* logical volume id */
struct querylv **querylv, /* pointer to the pointer to the location */
                           /* where data is returned */
char *pvname)              /* name of physical volume to read descriptor */
                           /* area from */

{
   int rc;                 /* contains the return codes */
   long copy;              /* counter in for loop */
   int vgfd;               /* file descriptor for the vg mapped file */
   struct unique_id vg_id; /*volume group id of vg holding the logical volume */
   short minor_num;        /* minor number of the logical volume */
   long size;              /* size to allocate  in malloc */
   void *malloc();         /* routine that allocates memory space */
   long buffer_size;       /* current size of buffer */
   int stale;              /* indicates a stale logical partition */
   char *vgptr;            /* pointer to the volume group mapped file */
   struct fheader *header; /* pointer to the vg mapped file,s file header */
   struct pv_header *pv;   /* pointer to the pv entry */
   struct pp_entries *pp;  /* pointer to the pp entry */
   struct lv_entries *lv;  /* pointer to the logical volume entry */
   struct vg_header *vgh;  /* pointer to the volume group header */
   struct namelist *nptr;  /* pointer to the name descriptor area in vg file */
   int flag;               /* indicates the volume group is off or on-line */
   int query_byname;	   /* indicates that a pvname was specified */
   int query_flag; 	   /* used for bldlvinfo */
   struct logview *lp;     /* pointer to the logical view of the */
                           /* logical volume */
   long cnt[LVM_NUMCOPIES]; /* array of number of physical partitions per */
                            /* copy of the logical volume */
    int  mode;              /* mode to open the mapped file with */

/*
*   If the querylv pointer is null
*       return an error
*/
    
    if(querylv == NULL)
    {   lvm_errors("querylv","     ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/* 
*  if the lv_id pointer is null 
*     return an error
*/

    if(lv_id == NULL)
    {   lvm_errors("querylv","    ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*  If the pvname is not null
*     call lvm_getpvda() to read the descriptor area directly from the
*     specified pv
*     set a variable to equal the minor number
*  else
*    Call get_lvinfo to get the information on the logical volume. This 
*    routine also calls the needed routines to return the pointer and file
*    descriptor for the volume group mapped file. Also, the validity of the
*    minor number and volume group id is checked before returning.
*/
    if(pvname != NULL)
    {   if(strcmp(pvname,"") == 0)  
        {   lvm_errors("querylv","    ",LVM_INVALID_PARAM);
            return(LVM_INVALID_PARAM);
        }       
        query_byname = TRUE;
        rc = lvm_getpvda(pvname,&vgptr,FALSE);
        if(rc < LVM_SUCCESS)
        {   lvm_errors("lvm_getpvda","querylv",rc);
            return(rc);
        }

        vgh = (struct vg_header *)(vgptr + sizeof(struct fheader));
        vg_id.word1 = vgh->vg_id.word1;
        vg_id.word2 = vgh->vg_id.word2;
        if(lv_id->minor_num > (vgh->maxlvs - 1) || lv_id->minor_num <= 0)
        {   lvm_errors("querylv","    ",LVM_INVALID_PARAM);
            free(vgptr);
            return(LVM_INVALID_PARAM);
        }       
        minor_num = lv_id->minor_num;
     }
     else
     {
         query_byname = FALSE;
         mode = O_RDONLY | O_NSHARE;
         rc = get_lvinfo(lv_id,&vg_id,&minor_num,&vgfd,&vgptr,mode);
         if(rc < LVM_SUCCESS)
             return(rc);
     } 

/*
*   Call get_ptrs(); to get the necessary pointers to the structures in 
*      the mapped file that hold the fields needed for the query
*   if the routine fails return an error code
*/

   rc=get_ptrs(vgptr,&header,&vgh,&lv,&pv,&pp,&nptr);
   if(rc < LVM_SUCCESS)
   {   if(pvname == NULL)
           close(vgfd);
       else
           free(vgptr);
       return(rc);
   }

/*
*  set the lv pointer to point to the correct logical volume
*  and check to be sure that one actually exists for that minor num
*/

   lv += minor_num - 1;
   if(lv->lv_state == LVM_LVUNDEF)
   {   lvm_errors("querylv","     ",LVM_INVALID_PARAM);
       if(pvname == NULL)
           close(vgfd);
       else
           free(vgptr);
       return(LVM_INVALID_PARAM);
   }

/*
* calculate the buffer size for the querylv struct
*/

   buffer_size = sizeof(struct querylv);


/*
*   malloc the necessary space and return an error code if the 
*   malloc fails
*/
   (*querylv) = (struct querylv *) malloc(buffer_size);
   if((*querylv) == NULL)
   {   lvm_errors("malloc","querylv",LVM_ALLOCERR);
       if(pvname == NULL)
           close(vgfd);
       else
           free(vgptr);
       return(LVM_ALLOCERR);
   };

/*  zero out the malloced space */

   bzero((char *)(*querylv),buffer_size);
/*
* fill in the query structure from the info on the lv entry 
*/

   strncpy((*querylv)->lvname, nptr->name[minor_num-1],LVM_NAMESIZ - 1);
   (*querylv)->vg_id.word1 = vg_id.word1;
   (*querylv)->vg_id.word2 = vg_id.word2;
   (*querylv)->maxsize = lv->maxsize;
   (*querylv)->mirror_policy = lv->mirror_policy;
   (*querylv)->permissions = lv->permissions;
   (*querylv)->bb_relocation = lv->bb_relocation;
   (*querylv)->mirwrt_consist = lv->mirwrt_consist;
   (*querylv)->currentsize = lv->num_lps;
   (*querylv)->ppsize = vgh->pp_size;
   rc = lvm_chklvclos(lv_id,header->major_num);
   switch(rc)
   {   case LVM_LVOPEN: (*querylv)->open_close = LVM_QLVOPEN;
                        break;
       default:         (*querylv)->open_close = LVM_QLV_NOTOPEN;
                        break;
   }

   (*querylv)->write_verify = lv->write_verify; 
   (*querylv)->stripe_exp = (unsigned int)lv->stripe_exp; 
   (*querylv)->striping_width = (unsigned int)lv->striping_width; 
 

/*
*  set a flag to check the state of the lv based on the pp states
*  then if there are lps to map, call lists() to create the struct
*  pps
*/
/* set an indicator for stale pps to false */
   stale = FALSE;

/*
* if there are lps for this lv, then calculate the size needed
* for each of the struct pp's that map pps to lps
*/
   if(lv->num_lps != LVM_NOLPSYET){
       buffer_size = (lv->num_lps * sizeof(struct pp));
       size = lv->num_lps * sizeof(struct pp);
       for(copy=0; copy < lv->mirror; copy ++)
       {   (*querylv)->mirrors[copy] = (struct pp *)(malloc(size));
           bzero((char *)(*querylv)->mirrors[copy],size);
       }
       if(query_byname == TRUE)
          query_flag = LVM_NOSTALE;
       else
	  query_flag = LVM_GETSTALE;
       rc = bldlvinfo(&lp,vgptr,lv,cnt,minor_num,query_flag);
       if(rc < LVM_SUCCESS)
       {   if(pvname == NULL)
             close(vgfd);
           else
             free(vgptr);
           free(*querylv);
           return(rc); 
       }
       rc = list(&stale,vgptr,(*querylv),lp,lv,header);
       if(rc < LVM_SUCCESS)
       {   if(pvname == NULL)
             close(vgfd);
           else
             free(vgptr);
           free(*querylv);
           return(rc);
       }
       free(lp);
   } 

/*
*  if the stale flag was changed to indicate a stale pp, then
*  add the stale state to the logical volume state.
*  if not stale then simply fill in the querylv struct with
*  the state on the logical volume
*/

   if(stale == TRUE)
      (*querylv)->lv_state = (lv->lv_state | LVM_LVSTALE);
   else 
     (*querylv)->lv_state = lv->lv_state;

/*
*  if a pvname was specified
*     free the space allocated for its descriptor area in memory
*  else
*     close the volume group mapped file
*/

    if(pvname != NULL)
    {  lv_id->vg_id.word1 = (*querylv)->vg_id.word1;
       lv_id->vg_id.word2 = (*querylv)->vg_id.word2;
       free(vgptr);
    } 
    else
       close(vgfd);
 
    free(vgptr);

    return(LVM_SUCCESS);
}
/*****************************************************************************/
/*
*  Name list 
*
*  Function : This routine builds a list of physical partition locations
*                that maps each logical partition to its physical partition
*                and physical volume. 
*
*  NOTES:
*    Input:
*       stale
*       vgh
*       vgptr
*       querylv
*       lp
*       cnt
*       lv
*       header
*
*    Output: Three lists. Each list contains the locations for each physical
*            partition that corresponds to a logical partition on the logical 
*            volume
*
*    Return Value : NONE
*
*/
/****************************************************************************/
int list(

int *stale, 		 /* indicates a stale logical volume */
char *vgptr, 		 /* pointer to volume group mapped file */
struct querylv *querylv, /* offset to the array being built */
struct logview *lp, 	 /* pointer to the logical view of the logical volume */
struct lv_entries *lv,   /* pointer to the logical volume entry */
struct fheader *header)  /* pointer to mapped file header */

 

{
    struct pp *aptr; /* pointer to the array being built */
    long lpcnt,copy; /* counters of loops */
    struct pv_header *pv; /* pointer to physical volume header */
    struct pp_entries *pp; /* pointer to physical partition entries */


      /* 
       * for the number of copies of this logical volume, go through
       * and build its logical view
       */

      for(copy = 0; copy < lv->mirror; copy ++)
      {   aptr = querylv->mirrors[copy];
          for(lpcnt = 0; lpcnt < lv->num_lps; lpcnt ++)
          /* if this logical partition is empty, skip it */
          {   if(lp->larray[lpcnt][copy].pvnum == 0)
                 aptr->lp_num = lpcnt+1;
              else
              /*
               *  fill in a struct pp entry for each physical partition 
	       *  corresponding to each logical partition in the logical
	       *  volume
	       */
              {   aptr->pv_id.word1 = 
                    header->pvinfo[lp->larray[lpcnt][copy].pvnum-1].pv_id.word1;
                  aptr->pv_id.word2 = 
                    header->pvinfo[lp->larray[lpcnt][copy].pvnum-1].pv_id.word2;
                  aptr->pp_num = lp->larray[lpcnt][copy].ppnum;
		  aptr->ppstate = lp->larray[lpcnt][copy].ppstate;
                  aptr->lp_num = lpcnt + 1;
                  /* if a stale partition has not been found yet
 		   * check the physical partition state for stale
		   * if it is stale, change the stale indicator to TRUE
                   */
		   if((*stale) == FALSE) {
			if(lp->larray[lpcnt][copy].ppstate & LVM_PPSTALE)
				(*stale) = TRUE;
		   }
               }

               /* increment the temporary pointer to the next entry to fil */

               aptr ++;

          } /* end for lpcnt */
      } /* end for copy */

   return(LVM_SUCCESS);
}    
/******************************************************************************/
