static char sccsid[] = "@(#)81	1.25  src/bos/usr/ccs/lib/liblvm/queryvg.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:52:51";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_queryvg
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

#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <liblvm.h>

/*                                                                   
* EXTERNAL PROCEDURES CALLED: 
*/

extern void *malloc();      /* allocates space in memory */

/*****************************************************************************
*  Name : lvm_queryvg
*
*  Function:  This function is called by a system management command to 
*  return the information listed below as output for the volume group specified
*
*  NOTES:
*    Input:
*      vg_id     
*      queryvg 
*      pvname
*
*    Output:
*      maxlvs
*      ppsize   
*      freespace 
*      num_lvs
*      num_pvs
*      total_vgdas
*      lvs
*      pvs
*
*   Return Value : If the query is successful, the return code is 0, or
*                  LVM_SUCCESS. If the routine fails, any of the following
*                  errors could be returned.
*        
*                       LVM_OFFLINE
*                       LVM_ALLOCERR
*                       LVM_INVALID_PARAM
*                       LVM_PROGERR
*                       LVM_NOTVGMEM
*                       LVM_NOPVVGDA
*                       LVM_PVDAREAD
*                       LVM_LVMRECERR
*                       LVM_PVOPNERR
*                       LVM_NOTCHARDEV
*                       LVM_MAPFOPN
*                       LVM_MAPFSHMAT
*                       LVM_DALVOPN
*
*/
/***************************************************************************/
int 
lvm_queryvg(
struct unique_id *vg_id,    /* volume group id */
struct queryvg   **queryvg, /* pointer to the pointer to the location */
                            /* where data is returned */
char *pvname)               /* indicates if the query should be done with */
                            /* the vg offline */

{
   int rc;                 /* contains the return codes */
   int vgfd;               /* file descriptor for the vg mapped file */
   long buffer_size;       /* current size of buffer */
   long lvsize;            /* size needed for the lv array */
   long pvsize;            /* size needed for the pv array */
   long ppcount;           /* counter for number of pps in physical volume */
   char *vgptr;            /* pointer to the volume group mapped file */
   struct fheader *header; /* pointer to the vg mapped file,s file header */
   struct namelist *nptr;  /* pointer to the namelist area */ 
   struct pv_header *pv;   /* pointer to the pv entry */
   struct pp_entries *pp;  /* pointer to the physical partition entry */
   struct lv_entries *lv;  /* pointer to the logical volume entry */
   struct vg_header *vgh;  /* pointer to the volume group header */
   int mode;               /* mode to open file in */
   struct lv_array *temp;  /* temporary pointer */
   struct pv_array *tmp;   /* temporary pointer */
   long lvcnt,pvcnt;       /* loop counters */
   int stale;              /* indicates a logical or physical volume is stale */
   long cpcnt, lpcnt;      /* counters for a for loop */
   struct logview *lp;     /* logical view of the logical volume */
   long cnt[LVM_NUMCOPIES];/* array of number of physical partitions per */
                           /* copy of the logical volume */
   struct vgsa_area vgsa;  /* structure for volume group status area info */
   int test_state;	   /* state of physical partition from vgsa */
   int miss_state;         /* state of physical volume from vgsa */ 
   int query_byname;       /* indicates a pvname was given */
   int query_flag;	   /* used for bldlvinfo */
 

/*
*  if the queryvg field is NULL
*    return an error
*/

   if(queryvg == NULL)
   {   lvm_errors("queryvg","       ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

/*
*   if the vg_id is NULL and the pvname is NULL
*      return an error
*/
   
    if(vg_id == NULL && pvname == NULL)
    {   lvm_errors("queryvg","      ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }


/*
*  if the pvname indicates a query directly from the hard file
*     call lvm_getpvda() to read the specified hardfile and create
*     a "fake" mapped file
*/
    

    if(pvname != NULL)
    { if(strcmp(pvname,"\0") == 0)
      {  lvm_errors("queryvg","    ",LVM_INVALID_PARAM);
         return(LVM_INVALID_PARAM);
      }
      query_byname = TRUE;
      rc = lvm_getpvda(pvname,&vgptr,FALSE); 
      if(rc < LVM_SUCCESS)
      {   lvm_errors("lvm_getpvda","queryvg",rc);
	  free(vgptr);
          return(rc);
      }
    }
    else
    {
         query_byname = FALSE;
/*
*   call lvm_getvgdef() to open the mapped file
*/

          mode = O_RDONLY | O_NSHARE;
          rc = lvm_getvgdef(vg_id,mode,&vgfd,&vgptr);
          if(rc < LVM_SUCCESS)
          {   lvm_errors("lvm_getvgdef","queryvg",rc);
	      free(vgptr);
              return(rc);
          }
         
    }
 
/*
*   Call get_ptrs(); to get the necessary pointers to the structures in 
*      the mapped file that hold the fields needed for the query
*   if the routine fails return an error code
*/

   rc=get_ptrs(vgptr,&header,&vgh,&lv,NULL,NULL,&nptr);
   if(rc < LVM_SUCCESS)
   {  if(pvname == NULL)
         close(vgfd);

      free(vgptr);
      return(rc);
    }
/*
* calculate the buffer size for the queryvg struct
*/

   buffer_size = sizeof(struct queryvg);

/*
*   malloc the necessary space and return an error code if the 
*   malloc fails
*/

   (*queryvg) = (struct queryvg *) malloc(buffer_size);

   if((*queryvg) == NULL)
   {   lvm_errors("malloc","queryvg",LVM_ALLOCERR);
       if(pvname == NULL)
          close(vgfd);

       free(vgptr);
       return(LVM_ALLOCERR);
   } 

/*  zero out the malloced space */
   bzero((char *)(*queryvg),buffer_size);
/*
* fill in the query structure from the info on the vg 
*/

   (*queryvg)->maxlvs = vgh->maxlvs;
   (*queryvg)->ppsize = vgh->pp_size;
   (*queryvg)->num_lvs = vgh->numlvs;
   (*queryvg)->num_pvs = vgh->numpvs; 
   (*queryvg)->total_vgdas = vgh->total_vgdas;

/*
* if there are lvs for this vg  
*     calculate the size needed for the struct lv_array
*     malloc the space
*     fill in the array
*/
   if(vgh->numlvs != 0)
   {  lvsize = (vgh->numlvs * sizeof(struct lv_array));
      ((*queryvg)->lvs) = (struct lv_array *)(malloc(lvsize));
      if(((*queryvg)->lvs) == NULL)
      {   lvm_errors("queryvg","      ",LVM_ALLOCERR);
          if(pvname == NULL)
             close(vgfd);

          free(vgptr);
          free(*queryvg);
          return(LVM_ALLOCERR);
      }
/*  zero out the malloced space */
      bzero((char *)((*queryvg)->lvs),lvsize);
      temp = (*queryvg)->lvs;
      for(lvcnt = 1; lvcnt < vgh->maxlvs; lvcnt++)
      {  if(lv->lv_state & LVM_LVDEFINED)
         {   temp->lv_id.vg_id.word1 = vgh->vg_id.word1;
             temp->lv_id.vg_id.word2 = vgh->vg_id.word2;
             temp->lv_id.minor_num = (lv->lvname + 1);
             strncpy(temp->lvname,nptr->name[lv->lvname],LVM_NAMESIZ -1);
 	     if(query_byname == TRUE)
	 	query_flag = LVM_NOSTALE;
             else
	        query_flag = LVM_GETSTALE;
             rc = bldlvinfo(&lp,vgptr,lv,cnt,(short)(lv->lvname + 1),
                          query_flag);
             if(rc < LVM_SUCCESS)
             {  if(pvname == NULL)
                  close(vgfd);

                free(vgptr);
                free(*queryvg);
                return(rc);
             }
             stale = FALSE;
             for(lpcnt = 0; lpcnt < lv->num_lps; lpcnt ++)
             {   for(cpcnt = 0; cpcnt < LVM_NUMCOPIES; cpcnt ++)
                 {   if(lp->larray[lpcnt][cpcnt].pvnum != 0)
                     {   if(lp->larray[lpcnt][cpcnt].ppstate & LVM_PPSTALE)
                         {   temp->state = lv->lv_state | LVM_LVSTALE;
                             stale = TRUE;
                             break;
                         }
                      }
                  }
                  if(stale == TRUE)
                     break;
              }
    	      free(lp);
              if(stale == FALSE)
                 temp->state = lv->lv_state;
              temp ++;
          }
          lv++;
       }
    }

/*
*  if there are pvs for this vg, then calculate the size needed for
*  the struct pv_array
*/
    if(vgh->numpvs != 0)
    {  pvsize = (vgh->numpvs * sizeof(struct pv_array));
       ((*queryvg)->pvs) = (struct pv_array *)(malloc(pvsize));
       if(((*queryvg)->pvs) == NULL)
       {   lvm_errors("queryvg","      ",LVM_ALLOCERR);
           if(pvname == NULL)
              close(vgfd);

           free(vgptr);
           free(*queryvg);
           return(LVM_ALLOCERR);
       }
/*  zero out the malloced space */
       bzero((char *)((*queryvg)->pvs),pvsize);
       tmp = (*queryvg)->pvs;
       /*
        * if pvname is NULL
	* call getstates() to get a copy of the volume group status area
	* where all stale information on physical partitions is stored
       */
       if(pvname == NULL) {
       		rc = getstates(&vgsa,vgptr);
       		if(rc < LVM_SUCCESS)
		{
			free(vgptr);
          		return(rc);
		}
       }
       /*
        * for each pv in the volume group, fill in the pv_array with 
        * the needed information
        */
       for(pvcnt = 1; pvcnt <= LVM_MAXPVS; pvcnt ++)
       {   if(header->pvinfo[pvcnt-1].pvinx != 0)
           {   pv = (struct pv_header *)((char *)vgptr +
                     header->pvinfo[pvcnt-1].pvinx);

               tmp->pv_id.word1 = pv->pv_id.word1;
               tmp->pv_id.word2 = pv->pv_id.word2;
               tmp->pvnum_vgdas = pv->pvnum_vgdas;
               pp = (struct pp_entries *)((char *)pv+sizeof(struct pv_header));
               stale = FALSE;
               for(ppcount = 1; ppcount <= pv->pp_count; ppcount++) {
		  if(pvname == NULL) {
                        test_state = (TSTSA_STLPP(&vgsa,pvcnt-1,ppcount-1)) ? 
					TRUE : FALSE;
	 	        if(test_state == TRUE && stale == FALSE) {
                            tmp->state = (pv->pv_state | LVM_PVSTALE);
                            stale = TRUE;
                        }
                  }
                  if(pp->pp_state == LVM_PPFREE)
                     ((*queryvg)->freespace) ++;
                  pp++;
               }
               if(stale == FALSE)
                  tmp->state = pv->pv_state;
	       if(pvname == NULL) {
     			test_state = (TSTSA_PVMISS(&vgsa,pvcnt-1))?TRUE : FALSE;
                        if(test_state == TRUE) {
				tmp->state &= LVM_ACTIVEMASK; 
   		 		tmp->state |= LVM_PVMISSING;
   	       		}
		}
		else
			tmp->state &= LVM_ACTIVEMASK;
               tmp ++;
            }
        }
     }
     else
     {   lvm_errors("queryvg","      ",LVM_PROGERR);
         if(pvname == NULL)
            close(vgfd);

         free(vgptr);
         free(*queryvg);
         return(LVM_PROGERR);
     }

     if(pvname != NULL)
     {   vg_id->word1 = vgh->vg_id.word1;
         vg_id->word2 = vgh->vg_id.word2;
     }
     else
        close(vgfd); 

    free(vgptr);

    return(LVM_SUCCESS);
}
/*****************************************************************************/
