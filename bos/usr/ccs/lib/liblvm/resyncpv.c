static char sccsid[] = "@(#)66	1.19  src/bos/usr/ccs/lib/liblvm/resyncpv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:53:49";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_resyncpv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990,
 *               1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <fcntl.h>
#include <liblvm.h>


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern void *malloc();

/*
 * NAME: lvm_resyncpv
 *                                                                    
 * FUNCTION: This routine synchronizes all physical partitions on a physical
 *           volume with there other copies in the logical partition
 *
 * NAME: lvm_resyncpv
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *  DATA STRUCTURES:
 *     input
 *       vg_id
 *       pv_id
 * 
 *     output
 *        The pp states for the stale pps of each physical volume will be 
 *        changed to indicate that they are not stale, and the data on 
 *        these pps will be in sync with the data on there copies. The physical
 *        volume state will have its stale bit turned off.
 *
 *
 * RETURN VALUE DESCRIPTION: LVM_SUCCESS if successful and one of the following
 *			     if not successful:
 *                                 LVM_INVALID_PARAM
 *                                 LVM_OFFLINE
 *                                 LVM_PROGERR
 *                                 LVM_MAPFOPN
 *                                 LVM_MAPFSHMAT
 *                                 LVM_ALLOCERR
 *                                 LVM_NOTSYNCED
 *                                 LVM_NOTCHARDEV
 *                                 LVM_INV_DEVENT
 */  


int 
lvm_resyncpv(
struct unique_id *vg_id, /* pointer to volume group id */
struct unique_id *pv_id, /* pointer to the physical volume id */
int               force) /* indicates we want to resync non-stale lps too */

 
{

    /* begin lvm_resyncpv */
  
    int rc;                /* return code */
    int  vgfd = MAPNOTOPEN;/* volume group mapped file descriptor */
    char *vgptr;           /* volume group mapped file pointer */
    struct pp_entries *pp; /* pointer to pp entry */
    struct pv_header *pv;  /* pointer to physical volume header */
    struct lv_entries *lv; /* pointer to logical volume entry */
    long ppcnt;            /* the pp_count for the physical volume */
    long ppc;		   /* loop counter */
    long logvol;           /* the minor number of the open logical volume */
    char rawname[LVM_EXTNAME];
                           /* raw name of lv */
    int  lvfd;             /* file descriptor for the logical volume */
    struct namelist *nptr; /* pointer to namelist area */
    struct fheader *fhead; /* pointer to the volume group mapped file header */
    struct vgsa_area vgsa; /* structure for volume group status area info */
    int    test_state;     /* indicates a physical partiton is stale and is */
			   /* returned from TSTSA_STLPP */
    int flag;		   /* indicates a forced synclp call */
    short pvnum;	   /* number of physical volume */
    int   nosync;	   /* indicates we have a force flag set to TRUE */
			   /* and we've found a lp with all copies on this pv*/


   /* check that the vg_id and pv_id are not null */

   nosync = FALSE;
   if(vg_id == NULL)
   {   lvm_errors("resyncpv","   ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

   if(pv_id == NULL)
   {   lvm_errors("resyncpv","   ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

   /* if force is not TRUE or FALSE return an error */
   if((force != TRUE) && (force != FALSE))
  	return(LVM_INVALID_PARAM);


   /*
    *  Call lvm_getvgdef() to open the volume group mapped file and return
    *  its file descriptor and shmat pointer
    */
   
   rc = lvm_getvgdef(vg_id,O_RDWR|O_NSHARE,&vgfd,&vgptr);
   if(rc < LVM_SUCCESS)
   {   lvm_errors("lvm_getvgdef","resyncpv",rc);
       free(vgptr);
       return(rc);
   }


   /*
    *  Call get_pvandpp() to get a pointer to the physical volume with 
    *  the pv_id that matches the one sent in, and a pointer to its first
    *  physical partition.
    *  (ie. check the validity of the pv_id)
    */

   rc = get_pvandpp(&pv,NULL,&pvnum,vgptr,pv_id);
   if(rc < LVM_SUCCESS)
   {    close(vgfd);
	free(vgptr);
	return(rc);
   }



   /* call get_ptrs to get a pointer to the first logical volume in the file */

   rc = get_ptrs(vgptr,&fhead,NULL,NULL,NULL,NULL,NULL);
   if(rc < LVM_SUCCESS)
   {   close(vgfd);
       free(vgptr);
       return(rc);
   }

   
   /* initialize a counter to hold the pp_count from the pv header */
   ppcnt = pv->pp_count;
   
   /* close the volume group mapped file */

  close(vgfd);
  vgfd = MAPNOTOPEN;
  free(vgptr);

  /* Look at each PP */

  for(ppc = 0; ppc < ppcnt; ppc++)
  {
      
      /*
       *  Call lvm_getvgdef() to open the volume group mapped file and return
       *  its file descriptor and shmat pointer
       *  NOTE: we need to re-open the mapped file since it may have been closed
       *  if we did a synclp().
       */
        
       if( vgfd == MAPNOTOPEN ) {
	    rc = lvm_getvgdef(vg_id,O_RDWR|O_NSHARE,&vgfd,&vgptr);
	    if(rc < LVM_SUCCESS)
	    {   lvm_errors("lvm_getvgdef","resyncpv",rc);
		free(vgptr);
		return(rc);
	    }

	   /*
	    * Call get_pvandpp() to get a pointer to the physical volume with 
	    * the pv_id that matches the one sent in, and a pointer to its first
	    * physical partition.
	    * (ie. check the validity of the pv_id)
	    * It is necessary to recalculate all pointers since the volume group
	    * mapped file was closed and reopened.
	    */

	    rc = get_pvandpp(&pv,&pp,NULL,vgptr,pv_id);
	    if(rc < LVM_SUCCESS)
	    {   close(vgfd);
	        free(vgptr);
	        return(rc);
	    }

	    /*
	     * call get_ptrs to get a pointer to the first logical volume in the
	     * file
	     */
	    rc = get_ptrs(vgptr,NULL,NULL,&lv,NULL,NULL,&nptr);
	    if(rc < LVM_SUCCESS)
	    {   close(vgfd);
	        free(vgptr);
	        return(rc);
	    }
 
	    /* 
	     * Call getstates to get a copy of the volume group status area in 
	     * order to check for stale physical partitions
	     */
	    if(force == FALSE) {
	    	rc = getstates(&vgsa,vgptr);
	    	if(rc < LVM_SUCCESS) {
 	       	   close(vgfd);
	           free(vgptr);
	           return(rc);
		}
	    }
        }     		

        /* get a pointer to the appropriate pp */
	pp = (struct pp_entries *)((char *)pv + sizeof(struct pv_header)) + ppc;
        /*
	 * If the physical partition is stale then try to resync it.
	 *  test to see if the state is stale with the macro from vgsa.h
	 */
	if(pp->pp_state & LVM_PPALLOC) {
	    if(force == FALSE) 
	    	test_state = TSTSA_STLPP(&vgsa,pvnum-1,ppc) ? TRUE:FALSE;
	    else
		test_state = TRUE;
	    if(test_state == TRUE) {  
		/*
	 	 * call status_chk() to be sure the logical volume device is raw
	 	 * and that it has a valid major number,and to get the full path
 		 * name of the logical volume       
 		 * then open the logical volume associated with the stale pp
 		 */

		rc = status_chk(vgptr,nptr->name[pp->lv_index - 1],
			   CHECK_MAJ,rawname);
		if(rc < LVM_SUCCESS)
		{  close(vgfd);
 	 	   free(vgptr);
		   return(rc);
		}
		lvfd = open(rawname,O_RDWR);
		if(lvfd < LVM_SUCCESS)
		{  lvm_errors("lv open","resyncpv",lvfd);
		   close(vgfd);
		   free(vgptr);
  		   return(LVM_PROGERR);
		}

		/* increment the logical volume pointer to the correct lv */ 
		lv += pp->lv_index - 1;
		/*
 	 	* if the force flag is on, set a flag to TRUE to be passed 
	 	* into synclp otherwise, set a flag to FALSE for synclp()
 	 	*/
		if(force == TRUE) {
		   switch(pp->copy) {
		      /* if this is the only copy, we want to resync */
		      case LVM_PRIMARY: nosync = FALSE;
					break;
		      /*
		       * if there are two copies of the lp that this pp
		       * belongs to, then we need to check to see if both
		       * copies are on the pv that we're resyncing. If they
		       * are, then we want to be sure and only call the
		       * resync once.
		       */
		      case LVM_PRIMOF2:
	              case LVM_SCNDOF2: if(pv->pv_num == pp->fst_alt_vol) {
					   if((ppc + 1) < pp->fst_alt_part)
                                               nosync = FALSE;
                                           else 
                                               nosync = TRUE;
                                        }
                                        else
                                           nosync = FALSE;
					break;
		       /* 
			* if there are three copies of the lp that this pp
			* belongs to, then we need to check if they are all
			* on the physical volume we're resyncing. If so, we
		        * must check to be sure that we only resync once for
		        * this logical partition instead of once for each copy
			*/ 
                       case LVM_PRIMOF3:
		       case LVM_SCNDOF3:
		       case LVM_TERTOF3:
		       /*
		  	* CASE: All 3 copies on current PV ..
		 	* Current PP < PP(2) & PP(3) ... 
		 	* ACTION: Go ahead and resync...
		  	*/
		             if(((pv->pv_num == pp->fst_alt_vol) 
				   && (pv->pv_num == pp->snd_alt_vol))  
				   && (((ppc + 1) < pp->snd_alt_part)	
				   && ((ppc +1) < pp->fst_alt_part)))
                                          nosync = FALSE;
                             else {
			   	/*
			         * CASE A: 2 out of 3 copies on current PV ...
			         * Current PP < PP(1) ...
				 * ACTION: Go ahead and resync ...
				 */
			     	if(((pv->pv_num == pp->fst_alt_vol)  
				    && ((ppc +1) < pp->fst_alt_part)
				    && (pv->pv_num != pp->snd_alt_vol))  
			   	/*
			     	 * CASE B: 2 out of 3 copies on
			   	 * current PV ...
			    	 * Current PP < PP(2) ...
			   	 * ACTION: You got it, resync it!!!
			    	 */
				    || ((pv->pv_num == pp->snd_alt_vol)  
				       && ((ppc +1) < pp->snd_alt_part)
				       && (pv->pv_num != pp->fst_alt_vol))  
			   	  /*
				   * CASE C: 1 copy out of 3 on current PV ...
				   * ACTION: Yep, do it, do it, resync
				   */
				    || ((pv->pv_num != pp->fst_alt_vol)  
				       && (pv->pv_num != pp->snd_alt_vol)))  
                                              nosync = FALSE;
                                       else
                                              nosync = TRUE;
                             }  /* end if else */
			 break;
		   } /* end switch */
		   flag = TRUE;
		}
		else
	   	   flag = FALSE;

		/* call synclp to resync the logical partition */
                if(nosync == FALSE) {
		   rc = synclp(lvfd,lv,vg_id,vgptr,vgfd,pp->lv_index,
			   	pp->lp_num,flag);
		   /*
	 	    * close the logical volume and free the vgptr since
	 	    * synclp closed the mapped file
	 	    */
		    close(lvfd);
		    free(vgptr);
 		    vgfd = MAPNOTOPEN;
		}
	    } /* end if ppstate is stale */
	} /* if the pp is allocated */
  } /* end for */

    if( vgfd == MAPNOTOPEN ) {
	free(vgptr);
	rc = lvm_getvgdef(vg_id,O_RDWR|O_NSHARE,&vgfd,&vgptr);
	if(rc < LVM_SUCCESS)
	{   lvm_errors("lvm_getvgdef","resyncpv",rc);
	    free(vgptr);
	    return(rc);
        }
    }
    rc = getstates(&vgsa,vgptr);
    if(rc < LVM_SUCCESS) {
        close(vgfd);
        free(vgptr);
        return(rc);
    }
   /* check to see that all physical partitions were resynced */
   if(force == FALSE)
   {
   	for(ppc = 0; ppc < ppcnt; ppc ++)
   	{
            pp = (struct pp_entries *)((char *)pv + sizeof(struct pv_header)) + ppc;
            /* only check if it was allocated...  */
            if(pp->pp_state & LVM_PPALLOC)
	    {
		test_state = TSTSA_STLPP(&vgsa,pvnum-1,ppc) ? TRUE:FALSE;
       	   	if(test_state == TRUE) 
		{
			free(vgptr);
	       		return(LVM_NOTSYNCED);
		}
   	    }
   	}
   }

   close(vgfd);
   free(vgptr);
   return(LVM_SUCCESS);
}

/************************************************************************/   
