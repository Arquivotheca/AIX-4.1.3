static char sccsid[] = "@(#)65	1.18  src/bos/usr/ccs/lib/liblvm/resynclv.c, liblvm, bos41J, 9516B_all 4/19/95 16:09:18";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_resynclv
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

#include <fcntl.h>
#include <liblvm.h>


extern  void * malloc();
/*
 * NAME: lvm_resynclv
 *                                                                    
 * FUNCTION: This routine synchronizes all logical partitions for a logical
 *           volume
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *  DATA STRUCTURES:
 *     input
 *       lv_id
 * 
 *     output
 *        The pp states for the pps of each logical partition will be changed 
 *        to indicate that they are not stale, and the data on all copies of
 *        each logical partition will be in sync. If the logical volume's state
 *        was LVM_LVSTALE that bit will be turned off
 *
 *
 * RETURN VALUE DESCRIPTION: LVM_SUCCESS if successful and one of the following
 *                           if not successful :
 *                                 LVM_INVALID_PARAM
 *                                 LVM_OFFLINE
 *                                 LVM_PROGERR
 *                                 LVM_MAPFSMAT
 *                                 LVM_MAPFOPN
 *                                 LVM_NOTCHARDEV
 *                                 LVM_INV_DEVENT
 *                                 LVM_ALLOCERR
 *                                 LVM_NOTSYNCED
 *                                 LVM_INVALID_MIN_NUM
 *                                 LVM_DALVOPN
 *
 */   


int 
lvm_resynclv(
struct lv_id *lv_id, /* pointer to logical volume id */
int 	     force)  /* indicator to resync even NON-STALE lps */


{

    /* begin lvm_resynclv */

  
   int rc;                  /* return code */
   char * vgptr;            /* pointer to volume group mapped file */
   int vgfd;                /* volume group mapped file descriptor */
   short minor_num;         /* minor number of the logical volume */
   struct lv_entries *lv;   /* pointer to logical volume */
   struct namelist *nptr;   /* pointer to the namelist area */
   int lvfd;                /* logical volume file descriptor */
   char rawname[LVM_EXTNAME];
                            /* raw lv name */
   struct unique_id vg_id;  /* volume group id */
   struct fheader *fhead;   /* pointer to volume group mapped file header */
   char * failures;         /* pointer to an array of lp entries that */
                            /* indicate */
                            /* if a logical partition was resynced or not */
   long lpc;		    /* loop counter */
   long loopcnt;	    /* number of logical partitions */
   long cpcnt;              /* counter for number of copies of a logical */
                            /* partition */
   long cnt[LVM_NUMCOPIES]; /* array of number of physical partitions per */
                            /* copy of a logical volume */
   struct logview *lp;      /* pointer to the logical view of the logical */
                            /* volume */
   int  stale;              /* indicates that a logical partition is stale */
   int  mode;               /* mode to open mapped file with */ 
   

   /* if the lv_id is null, return an error */

   if(lv_id == NULL)
   {   lvm_errors("resynclv","    ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }

   /* if force is not TRUE or FALSE return an error */
   if((force != TRUE) && (force != FALSE))
	return(LVM_INVALID_PARAM);

    /*
     * Call get_lvinfo() to check that the vg is online,open the mapped file 
     * and return info on the logical volume
     */

    mode = O_RDWR | O_NSHARE;
    rc = get_lvinfo(lv_id,&vg_id,&minor_num,&vgfd,&vgptr,mode);
    if(rc < LVM_SUCCESS)
    {
	free(vgptr);
	return(rc);
    }

    /*
     *  call get_ptrs to get a pointer to the logical volume
     *   that needs to be resynced
     *  increment the lv pointer to the correct lv
     */
 
    rc = get_ptrs(vgptr,&fhead,NULL,&lv,NULL,NULL,&nptr);
    if(rc < LVM_SUCCESS) {
	close(vgfd);
	free(vgptr);
	return(rc);
    }

    lv += minor_num - 1;


    /*
     * if the logical volume has a mirror value of 0 or LVM_NOMIRROR(1)
     *      return()
     */

    if(lv->mirror == 0 || lv->mirror == LVM_NOMIRROR) {  
	lvm_errors("resynclv","     ",LVM_SUCCESS);
	close(vgfd);
	free(vgptr);
	return(LVM_SUCCESS);
    }
 
    /* save the number of lps on the lv to use as a counter in a loop */
 
    loopcnt = lv->num_lps;

    /*
     * malloc the necessary space for the failures array 
     */

    failures = (char *)malloc(loopcnt);
    if(failures == NULL) {
	close(vgfd);
	free(vgptr);
	return(LVM_ALLOCERR);
    }
    
    /* initialize the failures array to zeroes */

    bzero(failures,loopcnt);

    /*
     *   call status_chk() to be sure the device is raw and that it has a
     *   valid major number
     */

    rc = status_chk(vgptr,nptr->name[lv->lvname],CHECK_MAJ,rawname);
    if(rc < LVM_SUCCESS) {
	close(vgfd);
	free(vgptr);
	return(rc);
    }

    /* close the volume group mapped file */
 
    close(vgfd);
    vgfd = MAPNOTOPEN;
   
    free(vgptr);
    lvfd = open(rawname,O_RDONLY);
    if(lvfd < LVM_SUCCESS)
    {   lvm_errors("open lv","resynclv",lvfd);
	return(LVM_PROGERR);
    }


    /*
     *   for each logical partition in the logical volume, go through and
     *   if it's stale, resync it
     */

    for(lpc = 1; lpc <= loopcnt; lpc++) {
    
  	/* call lvm_getvgdef() to open the vg mapped file and return its
	 *  file descriptor and file pointer
	 */
	if(vgfd == MAPNOTOPEN){
        	rc = lvm_getvgdef(&vg_id,O_RDWR|O_NSHARE,&vgfd,&vgptr);
          	if(rc < LVM_SUCCESS)
          	{   close(lvfd);
		    free(vgptr);
              	    return(rc);
          	}
  
		/*
		 * call get_ptrs to get a pointer to the logical volume
		 * that needs to be resynced
		 * increment the lv pointer to the correct lv */
 
          	rc = get_ptrs(vgptr,NULL,NULL,&lv,NULL,NULL,NULL);
          	if(rc < LVM_SUCCESS)
          	{   close(vgfd);
		    free(vgptr);
              	    close(lvfd);
              	    return(rc);
          	}

          	lv += minor_num - 1;

		/* 
		 * call bldlvinfo() to get a logical view of the lv so that
		 * we can check the state of the logical partitions and only 
		 * resync the  stale ones.
		 */

		if( force == FALSE ) {
		    rc = bldlvinfo(&lp,vgptr,lv,cnt,minor_num,LVM_GETSTALE);
		    if(rc < LVM_SUCCESS)
		    {   close(vgfd);
			free(vgptr);
			close(lvfd);
			return(rc);
		    }
		}
	}

	if( force == TRUE ) {
		rc = synclp(lvfd,lv,&vg_id,vgptr,vgfd,minor_num,lpc,TRUE);

		/*
		 * synclp closes the mapped file se we will give
		 * back the memory and set fd to indicate it is
		 * not open.
		 */
		free( vgptr );
		vgfd = MAPNOTOPEN;

		/* 
		 *  if the synclp failed then set this lp's entry in
		 *  the failures array to TRUE
		 */
		if(rc < LVM_SUCCESS)
		    failures[lpc-1] = TRUE;
	}
	else {
		/*
		 * else we want to resync only the stale lps, so...
		 * search the logical view for this partition to see if
		 * there are any stale copies that need to be resynced
		 */
		for(stale = FALSE, cpcnt = 0; cpcnt < LVM_NUMCOPIES; cpcnt++)
		{
		    if(lp->larray[lpc-1][cpcnt].pvnum != 0) {
			if(lp->larray[lpc-1][cpcnt].ppstate & LVM_PPSTALE)
			{   stale = TRUE;
			    break;
			}
		    }
		}
 
		/*
		 * if a stale partition was found, then call synclp to 
		 * resync the entire logical partition
		 */

		if(stale == TRUE) {
			rc = synclp(lvfd,lv,&vg_id,vgptr,vgfd,minor_num,
					 lpc,FALSE);
			/*
			 * synclp closes the mapped file se we will give
			 * back the memory and set fd to indicate it is
			 * not open.
			 */
			free( vgptr );
			vgfd = MAPNOTOPEN;

			/* if the synclp failed then set this lp's entry in 
			 * the failures array to TRUE
			 */
			if(rc < LVM_SUCCESS)
			    failures[lpc-1] = TRUE;
		 }
	} /* end of else force flag is FALSE*/

	free(lp); /* memory leak, defect 130960 */

    } /* end for */   

    /* close the logical volume */
    close(lvfd);

    /*
     * close the volume group file if open
     */
    if( vgfd != MAPNOTOPEN )
	close(vgfd);
   
    /*
     *  check the failures array--- if we find any failures, then return
     *  LVM_NOTSYNCED
     */

    for(lpc = 0; lpc < loopcnt; lpc ++) {
	if(failures[lpc] == TRUE) {
	    free(failures);
	    free(vgptr);
	    return(LVM_NOTSYNCED);
	}
    }

    /* free the pointer for the failures array */
    free(failures);

    free(vgptr);
    return(LVM_SUCCESS);

}

/************************************************************************/   
