static char sccsid[] = "@(#)92	1.33  src/bos/usr/ccs/lib/liblvm/rdex_com.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:53:03";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: rdex_proc, reduce_errchk, dealloclp, update_log, srclog,
 *            get_altptrs, extend_errchk, allocatelp, allocation
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
#include <sys/signal.h>
#include <fcntl.h>
#include <liblvm.h>

extern void *malloc();

int reduce_errchk(
 struct lv_entries *lv,	     /* pointer to logical volume entry */
 struct logview    *lp,	     /* pointer to the logical view of the lv */
 struct pp	   *map,     /* pointer to struct pp map entry */
 char              *vgfptr); /* pointer to volume group mapped file */

int dealloclp(
 char           *vgfptr,	    /* pointer to vg mapped file */  
 struct logview *lp,		    /* pointer to logical view of lv */
 long           cnt[LVM_NUMCOPIES], /* number of pps per copy of lv */
 struct pp      *map);              /* pointer to struct pp map entry */

int update_log(
 struct logview *lp,		      /* pointer to logical view of lv */
 long           ppcnt[LVM_NUMCOPIES], /* number of pps per copy of lv */
 struct pp      *map,                 /* pointer to struct pp map entry */
 short 		pvnum);		      /* number of pv with pp to be reduced */

void srclog(
 struct lv_entries *lv,		/* pointer to logical volume entry */
 struct logview    *lp,		/* pointer to logical view of the lv */
 struct fheader    *fhead);	/* pointer to the vg mapped file file header */

int get_altptrs(
char *vgfptr, 		/* pointer to the volume group mapped file */
struct pp *map, 	/* pointer to the pp information */
struct pp_entries **pp);/* pointer to the physical partiton entry */

int extend_errchk(
 char              *vgfptr,	/* pointer to volume group mapped file */
 struct pp         *map,	/* pointer to struct pp map entry */
 struct lv_entries *lv,		/* pointer to logical volume entry */
 int               *empty,	/* indicates an empty logical partition */
 struct logview	   *lp);	/* logical view of logical volume */

int allocatelp(
 char              *vgfptr,	/* pointer to volume group mapped file */
 short             minor_num,	/* minor number of logical volume */
 struct pp         *map,	/* pointer to struct pp map entry */
 struct logview	   *lp,		/* pointer to logical view of lv */
 struct lv_entries *lv);	/* pointer to the logical volume entry */

void allocation(
 struct pp_entries *pp,		/* pointer to physical partition entry */
 short             lvindex,	/* minor number of logical volume */
 long              lpnum,	/* logical partition number */
 char              ppstate,	/* state of physical partition */
 short             fv,		/* first alternate physical volume */
 unsigned short    fp,		/* first alternate physical partition */
 short             sv,		/* second alternate physical volume */
 unsigned short    sp,		/* second alternate physical partition */
 char              copy);       /* copy number of the physical partition */

void _lvm_ignoresigs(int);

/*************************************************************************/
/* Name : rdex_proc 
*
*  Description: This function is called by reducelv or extendlv to make the
*  the needed checks on the logical volume and volume group and to complete
*  the processing for a reduction or extension
*
*  NOTES:
*    Input:
*      lv_id  
*      indicator
*      ext_red
*      
*
*    Output: updated data structures in the volume group mapped file and on the
*    hard disks to show a reduction or extension of a logical volume
*            
*
*   Return Value: LVM_SUCCESS if successful and one of the following if
*                 if unsuccessful.
*     			LVM_INVALID_PARAM
*     			LVM_INVALID_MIN_NUM
*     			LVM_OFFLINE
*     			LVM_PROGERR
*     			LVM_MAPFOPN
*     			LVM_MAPFSHMAT
*     			LVM_ALLOCERR
*     			LVM_LVOPEN
*     			LVM_PPNUM_INVAL
*     			LVM_PVSTATE_INVAL
*     			LVM_LPNUM_INVAL
*     			LVM_INVLPRED
*     			LVM_NOALLOCLP
*
*/
/*****************************************************************************/
int rdex_proc(
struct lv_id *lv_id,         /* logical volume id */
struct ext_redlv *ext_red,   /* maps of pps to be extended or reduced */
char *vgfptr,		     /* pointer to volume group mapped file */
int  vgfd,		     /* file descriptor for volume group mapped file */
short minor_num,	     /* minor number of logical volume */
int indicator)               /* indicates which routine, extendlv or */
                             /* reducelv is calling */

{
    int rc; 			/* hold return codes */
    struct lv_entries *lvptr;   /* points to a logical volume entry */
    struct vg_header *vgptr;    /* points to the volume group header */
    struct fheader *fhead;      /* pointer to the volume group mapped */
                                /* file header */
    struct logview *lp;         /* pointer to the logical view of the */
                                /* logical volume */
    int mircnt;                 /* counter for the number of copies that */
                                /* have been processed */
    int empty;                  /* indicates an empty logical partition */
                                /* is added */
    long counter;               /* number of nonzero cnt entries that */
                                /* indicates the new mirror value */
    long lpcnt;                 /* counter for the logical partitions that */
                                /* have been extended */
    long cnt[LVM_NUMCOPIES];    /* array of the count of pps for each copy of */
                                /* the logical volume */
    short pvnum;                /* phyiscal volume number sent to kernel */
                                /* routine */
    struct pv_header *erpv;     /* pointer to physical volume header */
    struct extred_part *kparts; /* ptr to pp_map like struct sent to kernel */
    long num;                   /* used to compute starting address of */
                                /* pp sent to kernel */
    struct ddi_info cfgdata;    /* input to kernel routine */
    long  cpcnt;                /* counter of for loop */
    int copies;                 /* number of copies of a logical partition */

    
	/*
	 * Use get_ptrs() to get the pointers to the desired structures in the
	 * volume group mapped file 
	 */
    	rc = get_ptrs(vgfptr,&fhead,&vgptr,&lvptr,NULL,NULL,NULL);
    	if(rc < LVM_SUCCESS)  
       	   return(rc);

	/*  
	 *   set the logical volume pointer to the correct logical volume
	 */

    	lvptr += (minor_num - 1);

	/*
	 *  if the state of the logical volume is LVM_LVUNDEF return an error
	 */
 
    	if(lvptr->lv_state == LVM_LVUNDEF) { 
  		lvm_errors("rdex_proc","     ",LVM_INVALID_PARAM);
        	return(LVM_INVALID_PARAM);
    	}
    
	/*
	 *  if the  size value is less than or equal to zero
	 *  return an error
	 */

        if(ext_red->size <= 0) {
   		lvm_errors("rdex_proc","   ",LVM_INVALID_PARAM);
            	return(LVM_INVALID_PARAM);
        }


	/*
	 * call bldlvinfo() to get a logical view of the logical volume
	 * being reduced/extended
	 */

        rc = bldlvinfo(&lp,vgfptr,lvptr,cnt,minor_num,LVM_NOSTALE);
        if(rc < LVM_SUCCESS)
            return(rc);

	/*
	 *  malloc the space needed for the list of extred_parts sent in to
	 *  the kernel routines
	 */

        kparts = (struct extred_part *)(malloc(ext_red->size * 
                  sizeof(struct extred_part)));
	if(kparts == NULL) 
	   return(LVM_ALLOCERR);
	/*
	 * for size times
	 *       switch(indicator)
	 */

	/* ignore sigs around kernel/mapped file update. */
	_lvm_ignoresigs(1);

	for(lpcnt = 1; lpcnt <= ext_red->size; lpcnt ++) {
            switch(indicator) {
                     case LVM_REDUCE:
                     	  /*
		           * call reduce_errchk() to check the validity of the 
		           * lpnum, ppnum, and pv_id
		           */	
                           rc = reduce_errchk(lvptr,lp,ext_red->parts,
                                vgfptr);
                           if(rc < LVM_SUCCESS) {
			       _lvm_ignoresigs(0);
			       free(lp);
			       free(kparts);
                               return(rc);
			       }
		           /* call dealloclp() to remove the pp */
                           rc = dealloclp(vgfptr,lp,cnt,ext_red->parts);
                           if(rc < LVM_SUCCESS) {
			       _lvm_ignoresigs(0);
			       free(lp);
			       free(kparts);
                               return(rc);
			       }
                           break;
                    

                    case LVM_EXTEND:
                          empty = FALSE;
			  /*
			   * call extend_errchk() to check the lpnum, ppnum 
			   * and pv_id validity
			   */
                          rc = extend_errchk(vgfptr,ext_red->parts,lvptr,
			       &empty,lp);
                          if(rc < LVM_SUCCESS) {
			      _lvm_ignoresigs(0);
			       free(lp);
			       free(kparts);
                              return(rc);
			      }
                          if(empty == FALSE) {
			        /*
			         * if we aren't adding an empty lp,
			         * call allocatelp() to check the existance of 
			         * required pps when necessary, and to
			         * allocate a valid physical partition for the
			         * logical partition
			         */
                         	rc = allocatelp(vgfptr,minor_num,ext_red->parts,
						lp,lvptr);
                          	if(rc < LVM_SUCCESS) {
					  _lvm_ignoresigs(0);
			       		  free(lp);
			       		  free(kparts);
                               	  	  return(rc);
					  }
                          }
                          break;
            } /* end switch */

            /*  set up input to kernel update routine */
            kparts ->lp_num = ext_red->parts->lp_num;
            rc = get_pvandpp(&erpv,NULL,&pvnum,vgfptr,&(ext_red->parts->pv_id));
            if(rc < LVM_SUCCESS) {
   		lvm_errors("get_pvandpp","rdex_com",rc);
		_lvm_ignoresigs(0);
		free(lp);
		free(kparts);
                return(rc);
            }
            kparts->pv_num = pvnum;
            num = PART2BLK(vgptr->pp_size - DBSHIFT,ext_red->parts->pp_num - 1);
 	    num += erpv->psn_part1;
            kparts->start_addr = num;
			kparts->mask=0;

            /* increment a pointer to the next partition in the list */
            (ext_red->parts) ++;

            /* increment the pointer to the next entry in the kernel
             *  extred_part struct list
             */
    
            kparts ++;

        } /* end for */
       
        bzero((char *)(&cfgdata.parmlist.kextred), sizeof(struct kextred));
        cfgdata.parmlist.kextred.vg_id.word1 = lv_id->vg_id.word1;
        cfgdata.parmlist.kextred.vg_id.word2 = lv_id->vg_id.word2;
        cfgdata.parmlist.kextred.lv_minor = minor_num;
        cfgdata.parmlist.kextred.num_extred = ext_red->size;
        kparts -= ext_red->size;
        cfgdata.parmlist.kextred.extred_part = kparts;

        if(indicator == LVM_REDUCE) {
 		/*
	 	 * if a reduction was performed
	 	 * check the counters for the number of physical
	 	 * partitions on each copy of the logical volume.
	 	 * count the entries that are not zero and that number is 
	 	 * the new mirror value
	 	 */
    		counter = 0;
     		for(mircnt = 0; mircnt < LVM_NUMCOPIES; mircnt ++) {
     	 		if(cnt[mircnt] != 0)
            			counter ++;
     		}
     		lvptr->mirror = counter;
       		/*
	 	 * Search each triplet for the largest 
 	 	 * lp num less than the current numlps stored on the
	 	 * logical volume header. If the triplet with this lp num is 
		 * NOT empty, then set the numlps field on the lv entry to
		 * that lpnum.
	 	 */
        	srclog(lvptr,lp,fhead);

		/* Call the kernel update routine */
        	cfgdata.parmlist.kextred.num_lps = lvptr->num_lps;
        	cfgdata.parmlist.kextred.copies = lvptr->mirror;
		if(lvptr->striping_width > 0) 
            		cfgdata.parmlist.kextred.i_sched = SCH_STRIPED;
		else if(lvptr->mirror == SEC_INDEX || lvptr->mirror == FIRST_INDEX)
            		cfgdata.parmlist.kextred.i_sched = SCH_REGULAR;
        	else {
        		if(lvptr->mirror_policy == LVM_SEQUENTIAL)
               		    cfgdata.parmlist.kextred.i_sched = SCH_SEQUENTIAL;
                        else
                            cfgdata.parmlist.kextred.i_sched = SCH_PARALLEL;
        	}
		rc = lvm_config(NULL,fhead->major_num,HD_KREDUCE,&cfgdata);
        	if(rc < LVM_SUCCESS) { 
			_lvm_ignoresigs(0);
  			lvm_errors("lvm_config","rdex_com",rc);
			free(lp);
			free(kparts);
            		return(rc);
        	}
 
	} /* end if indicator is REDUCE */

	/*
	 * Call the kernel update routine
	 * if the routine fails
	 *     return an error
	 */
        else /* indicator is HD_KEXTEND */
        {   cfgdata.parmlist.kextred.num_lps = lvptr->num_lps;
            cfgdata.parmlist.kextred.copies = lvptr->mirror;
	    if(lvptr->striping_width > 0) 
		cfgdata.parmlist.kextred.i_sched = SCH_STRIPED;
	    else if(lvptr->mirror == SEC_INDEX || lvptr->mirror == FIRST_INDEX)
		cfgdata.parmlist.kextred.i_sched = SCH_REGULAR;
            else {
                 if(lvptr->mirror_policy == LVM_SEQUENTIAL)
                       cfgdata.parmlist.kextred.i_sched = SCH_SEQUENTIAL;
                 if(lvptr->mirror_policy == LVM_PARALLEL)
                       cfgdata.parmlist.kextred.i_sched = SCH_PARALLEL;
   	    }
	    rc = lvm_config((mid_t)NULL,fhead->major_num,HD_KEXTEND,&cfgdata);
            if(rc < LVM_SUCCESS) {
		_lvm_ignoresigs(0);
                lvm_errors("lvm_config","rdex_com",rc);
		free(lp);
		free(kparts);
                return(rc);
            }
	}

   	rc = lvm_diskio(vgfptr,vgfd);
	_lvm_ignoresigs(0);

	free(lp);
	free(kparts);
     	return(rc);
}
/*****************************************************************************/
/* Name : reduce_errchk
*           
*  Function: This routine is called by rdex_proc to check the validity of
*               the values in the lp_num, pp_num, and pv_id fields. 
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      lv
*      lp
*      map
*      vgfptr
*
*    Output: none
*
*   Return Value: LVM_SUCCESS if successful, or one of the following.
*             LVM_PPNUM_INVAL
*             LVM_LPNUM_INVAL
*             LVM_INVALID_PARAM
*/
/*****************************************************************************/
int reduce_errchk(
struct lv_entries *lv, /* pointer to logical volume entry */
struct logview *lp,    /* pointer to the logical view of the logical volume */
struct pp *map,	       /* struct pp map entry to be checked */
char *vgfptr)          /* pointer to volume group mapped file */

{
    struct pv_header *pv; /* pointer to physical volume header */
    int rc; 		  /* holds error codes */
    int match;    	  /* indicates a matching ppnum was found */
    int copycnt; 	  /* counts the number of copies of the logical */
                          /* volume */
    short pvnum;	  /* physical volume number */

	/*
	 * if the map has a ppnum or pv_id of zero then
	 * return an error
	 */
   	if(map->pp_num <= 0 ) { 
  		lvm_errors("reduce_errchk","rdex_proc",LVM_PPNUM_INVAL);
       		return(LVM_PPNUM_INVAL);
   	}
   	if(map->pv_id.word1 == 0 && map->pv_id.word2 == 0) { 
 		lvm_errors("reduce_errchk","rdex_proc",LVM_INVALID_PARAM);
       		return(LVM_INVALID_PARAM);
   	}	
	/*
	 * Call get_pvandpp() to get the physical volume pointer
	 * If the physical partition number is greater than the ppcount
	 * for the physical volume specified, return an error
	 */
     	rc = get_pvandpp(&pv,NULL,&pvnum,vgfptr,&(map->pv_id));
     	if(rc < LVM_SUCCESS)
         	return(rc);
     	if(map->pp_num > pv->pp_count) {
   		lvm_errors("reduce_errchk","rdex_proc",LVM_PPNUM_INVAL);
         	return(LVM_PPNUM_INVAL);
     	}

	/*
	 * if the lpnum is <= zero, or is greater than maxsize  
	 * return an error code 
	 */
     	if(map->lp_num > lv->maxsize || map->lp_num <= 0) {
	 	lvm_errors("reduce_errchk","rdex_proc",LVM_LPNUM_INVAL);
        	return(LVM_LPNUM_INVAL);
     	}
	/*
	 * If the logical view for this logical partition has a ppnum entry that
	 * is not equal to the physical partition number in the struct pp
	 * return(errorcode)
	 */
      	match = FALSE;
      	for(copycnt = 0; copycnt < LVM_NUMCOPIES; copycnt++) {
        	if((lp->larray[map->lp_num-1][copycnt].ppnum == map->pp_num) &&
        	    (lp->larray[map->lp_num-1][copycnt].pvnum == pvnum)) {
    			match = TRUE;
             		break;
          	}
      	}

      	if(match == FALSE) { 
  		lvm_errors("reduce_errchk","rdex_proc",LVM_PPNUM_INVAL);
          	return(LVM_PPNUM_INVAL);
      	}
 
      	return(LVM_SUCCESS);
}
/*****************************************************************************/
/* Name : dealloclp 
*           
*  Function: This routine is called by rdex_proc to deallocate a physical 
*               partition (a copy) of a logical partition.
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      vgfptr
*      lp
*      cnt
*      map
*
*    Output: The mapped file structures and the hard files will be updated to
*             show a reduction of one copy of a logical partition.
*
*  Return Value: LVM_SUCCESS
*                 
*/
/*************************************************************************/
int dealloclp(
char *vgfptr, 		 /* pointer to the volume group mapped file */
struct logview *lp,      /* pointer to the logical view */
long cnt[LVM_NUMCOPIES], /* number of pps per logical volume copy */
struct pp *map) 	 /* pointer to the struct pp map entry */

{
    struct pp_entries *pp;  /*pointer to a physical partition entry */
    struct pv_header *pv;   /* pointer to a physical volume header entry */
    struct pp_entries *pps; /* pointer to the physical partition entries */
    struct fheader *fhead;  /* pointer to the volume group mapped file header */
    long ppcnt; 	    /* counter of physical partitions processed */
    short pvnum;	    /* number of pv with pp to be reduced on it */
    int rc; 		    /* holds the return code */
    


	/* 
	 * call get_altptrs() to get the pointers to the physical partition
	 * being reduced and its alternate partitions and to update the 
	 * alternate partitions to show the reduction
	 */
    	rc = get_altptrs(vgfptr,map,&pp);
    	if(rc < LVM_SUCCESS)
       		return(rc);
	/*
	 * call get_pvandpp() to get the number of the physical volume
	 * with the pp we're reducing on it
	 */
	rc = get_pvandpp(NULL,NULL,&pvnum,vgfptr,&(map->pv_id));
	if(rc < LVM_SUCCESS)
		return(rc);
	/* 
	 *  if a hole will be left as a result of the reduction, then
	 *  call update_log() to  promote and update only the logical view
	 */
     	rc = update_log(lp,cnt,map,pvnum);
     	if(rc < LVM_SUCCESS)
        	return(rc);
       
	/* call bzero() to zero out the physical partition entry */
     	bzero((char *)pp,sizeof(struct pp_entries));

     	return(rc);
}
/*****************************************************************************/
/* Name : update_log
*           
*  Function: This routine is called by dealloclp update the logical view
*               to show that a reduction has been made.
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      lp
*      map
*      ppcnt
*      pvnum
*
*    Output: The logical view will be updated to show the loss of the partition
*
*   Return Value: LVM_SUCCESS
*/
/****************************************************************************/
int update_log(
struct logview *lp, 	   /* pointer to the logical view */
long ppcnt[LVM_NUMCOPIES], /* number of pps per copy of logical volume */
struct pp *map,  	   /* pointer to the information on the pp */
                           /* being reduced */
short  pvnum)		   /* number of pv with pp to be reduced on it */

{
     long cnt; 		/* counts copies processed */
     long copycnt; 	/* number of copy being reduced */
     int rc; 		/* holds error codes */
     struct lp *temp; 	/* temporary pointer */

	/*
	 * determine the copy number of the physical partition being reduced
	 */
    	for(cnt = 0; cnt < LVM_NUMCOPIES; cnt ++)
    	{   if((lp->larray[map->lp_num - 1][cnt].ppnum == map->pp_num) &&
    	      (lp->larray[map->lp_num - 1][cnt].pvnum == pvnum)) { 
         	copycnt = cnt;
		break;
            }
     	}
 

	/*
	 * depending on the copy number, make the necessary promotions
	 * and update the logical view and the cnt of the physical partitions
	 * per copy of the logical volume
	 */

     	switch(copycnt) {
    		case FIRST_INDEX: 
		   if(lp->larray[map->lp_num -1][copycnt + 1].ppnum != 0) {
		   /* promote the second copy to the first copy's sp entry */
                         lp->larray[map->lp_num - 1][copycnt] = 
                               lp->larray[map->lp_num - 1][copycnt + 1];

		   /* zero out the lp entry for the second copy */
                          bzero((char *)
				(&(lp->larray[map->lp_num-1][copycnt+1])),
                                    sizeof(struct lp));

                          if(lp->larray[map->lp_num-1][copycnt+2].ppnum != 0) {
			  /*
			   * promote the third copy to the second copy's 
		           * lp entry 
			   */
                                lp->larray[map->lp_num - 1][copycnt+1] = 
                                      lp->larray[map->lp_num-1][copycnt+2];
   
			        /* zero out the third copy's lp entry */
                                bzero((char *)
                                      (&(lp->larray[map->lp_num-1][copycnt+2])),
                                       sizeof(struct lp));
                                ppcnt[copycnt + 2] -= 1;
                          }
                          else
                               ppcnt[copycnt + 1] -= 1;
		    }
                    else  {  /* no promotion is needed */
                        bzero((char *)
                               (&(lp->larray[map->lp_num-1][copycnt])),
                                sizeof(struct lp));
                        ppcnt[copycnt] -= 1;
                    }
		    break;
		case SEC_INDEX:
		    if(lp->larray[map->lp_num-1][copycnt+1].ppnum != 0) {
		    /* promote the third copy to the second copy's lp entry */
                          lp->larray[map->lp_num - 1][copycnt] = 
                               lp->larray[map->lp_num - 1][copycnt + 1];
			  /* zero out the third copy's lp entry */
                          bzero((char *)
                                    (&(lp->larray[map->lp_num-1][copycnt+1])),
                                     sizeof(struct lp));
                          ppcnt[copycnt + 1] -= 1;
                    }
                    else  { /* no promotion is needed */
                         bzero((char *)
                                (&(lp->larray[map->lp_num-1][copycnt])),
                                 sizeof(struct lp));
                         ppcnt[copycnt] -= 1;
                    } 
                    break;
		 case THIRD_INDEX: /* zero out the third copy's entry */ 
                    bzero((char *)
                           (&(lp->larray[map->lp_num-1][copycnt])),
                            sizeof(struct lp));
                    ppcnt[copycnt] -= 1;
                    break;  
	}
    
        return(LVM_SUCCESS);
}
/*************************************************************************/
/* Name : srclog
*           
*  Function: This routine is called by rdex_proc to set the largest logical
*               partition number with physical partitions allocated to it, as 
*               the num_lps value for the logical volume. It also checks that
*               the removal of a pp from the logical volume has not changed
*               the state of the logical volume from stale to current, and
*               makes the needed corrections to the logical volume state.
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      lv
*      lp
*      fhead
*
*    Output: If a logical partition number is found that meets the criteria 
*            above, and is less than the num_lps field, stored on the logical
*            volume header; then the num_lps field is modified on both the 
*            hard files and the mapped file. Also, if needed, the state field in
*            the file header for the logical volume is changed.
*
*   Return Value: LVM_SUCCESS
*                 
*/
/*****************************************************************************/
void srclog(
struct lv_entries *lv, /* pointer to the logical volume entry */
struct logview *lp,    /* pointer to the logical view of the logical volume */
struct fheader *fhead) /* pointer to the  file header */

{
   long lpcnt; 	    /* counter for number of logical partitions */
   int cpycnt;	    /* counter for the copy number of the lp being checked */
   long lpnum; 	    /* the number of the logical partition with allocated pps */


	/* set new_lpnum to false */
   	lpnum = 0;

	/*
	 * search the logical view from the bottom(last entry) to the top  
	 * if a logical partition with physical partitions allocated for it
	 * is found set new_lpnum to true and save the logical partition number 
	 */

   	for(lpcnt = lv->maxsize - 1; lpcnt >= 0; lpcnt --) {
     		for(cpycnt = FIRST_INDEX; cpycnt < LVM_NUMCOPIES; cpycnt ++) {
       			if(lp->larray[lpcnt][cpycnt].pvnum != 0) {
                   		lpnum = lpcnt + 1;
                   		break;
               		}
       		}
        	if(lpnum)
          	    break;
    	}

	/* set num_lps for the logical volume to be the lpnum we set above */
      	lv->num_lps = lpnum;
      
    	return;
}   
/*****************************************************************************/
/* Name : get_altptrs
*           
*  Function: This routine is called by dealloclp to
*            get pointers to the physical partition to be reduced
*            and also get pointers to that partition's first and second 
*            alternate partition entries. This routine also updates the
*            alternate partition entries to show the reduction
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      vgfptr
*      map
*      pp
*
*    Output: The pointers to the physical partition entries.
*
*   Return Values : LVM_SUCCESS                 
*/
/****************************************************************************/
int get_altptrs(
char *vgfptr, 		/* pointer to the volume group mapped file */
struct pp *map, 	/* pointer to the pp information */
struct pp_entries **pp) /* pointer to the physical partiton entry */
{
    int rc; 		   	 /* holds return code */
    struct fheader *fhead; 	 /* pointer to the volume group mapped */
                                 /* file header */
    struct pv_header *pv1,*pv2;	 /* pointers to the physical volumes */
                                 /* containing the alternate partitions */
    struct pp_entries *pp1,*pp2; /* pointers to pp entries */


	/*  
	 * Call get_pvandpp() to get the pointer to the first physical
	 * partition entry on the physical volume specified in the struct
	 * pp entry. Set the pp pointer to the pp that corresponds to the
	 * physical partition number specified in the struct pp entry.
	 */
     	rc = get_pvandpp(NULL,pp,NULL,vgfptr,&(map->pv_id));
    	if(rc < LVM_SUCCESS)
       		return(rc);
    	(*pp) += (map->pp_num - 1);

	/*
	 * If this is the only copy of a logical partition then simply
	 * return the pointer to its physical partition entry
	 */

    	if((*pp)->copy == LVM_PRIMARY)
       		return(LVM_SUCCESS);

	/*
	 * Based on the copy field of the partition to be deleted,
	 * derive the needed pointers to the alternate partitions
	 */
    	pp1 = NULL;
    	pp2 = NULL;
    	pv1 = NULL;
    	pv2 = NULL;
    	rc = get_ptrs(vgfptr,&fhead,NULL,NULL,NULL,NULL,NULL);
    	if(rc < LVM_SUCCESS)
        	return(rc);

	/* 
	 * switch on the copy value of the partition to be deleted so that
	 * we can update the alternate volumes 
	 */
  
    	switch((*pp)->copy) {
		case LVM_PRIMOF2:
        	case LVM_SCNDOF2: 
                          /*
                           * since there are only two copies, and one is
                           * being reduced, derive the pointers to the
         		   * fst_alt_vol and fst_alt_part of the pp being
                           * deleted(reduced)
                           */
                           pv1 = (struct pv_header *)((char *) vgfptr +
                                 fhead->pvinfo[(*pp)->fst_alt_vol-1].pvinx);
                           pp1 = (struct pp_entries *)((char *)pv1 + 
                                 sizeof(struct pv_header));
                           pp1 += ((*pp)->fst_alt_part - 1);
                           pp1->copy = LVM_PRIMARY;
                           pp1->fst_alt_vol = 0;
     			   pp1->fst_alt_part = 0;
			   break;
        	case LVM_PRIMOF3:
		case LVM_SCNDOF3:
		case LVM_TERTOF3: 
			 /*
			  *  since we know there will be two copies to update,
                          * derive the pointers to the first and second alt
			  * vols and parts of the pp being deleted(reduced)
			  */
                	
                         pv1 = (struct pv_header *)((char *) vgfptr +
                               fhead->pvinfo[(*pp)->fst_alt_vol-1].pvinx);
                         pp1 = (struct pp_entries *)((char *)pv1 + 
                               sizeof(struct pv_header));
                         pp1 += ((*pp)->fst_alt_part - 1);
                         pv2 = (struct pv_header *)((char *) vgfptr +
                               fhead->pvinfo[(*pp)->snd_alt_vol-1].pvinx);
                         pp2 = (struct pp_entries *)((char *)pv2 + 
                               sizeof(struct pv_header));
                         pp2 += ((*pp)->snd_alt_part - 1);

                         /*
                          * if the pp being reduced is the first of 3 copies
                          * then update its alternate copies appropriately
                          */

                         if((*pp)->copy == LVM_PRIMOF3) {
                             pp1->fst_alt_vol = pp1->snd_alt_vol;
                             pp1->fst_alt_part = pp1->snd_alt_part;
                             pp1->snd_alt_vol = 0;
                             pp1->snd_alt_part = 0;
                             pp1->copy = LVM_PRIMOF2;
                             pp2->fst_alt_vol = pp2->snd_alt_vol;
                             pp2->fst_alt_part = pp2->snd_alt_part;
                             pp2->snd_alt_part = 0;
                             pp2->snd_alt_vol = 0;
                             pp2->copy = LVM_SCNDOF2;
                         }
                         if((*pp)->copy == LVM_SCNDOF3) {
                             pp1->fst_alt_vol = pp1->snd_alt_vol;
                             pp1->fst_alt_part = pp1->snd_alt_part;
                             pp1->snd_alt_vol = 0;
                             pp1->snd_alt_part = 0;
                             pp1->copy = LVM_PRIMOF2;
                             pp2->snd_alt_part = 0;
                             pp2->snd_alt_vol = 0;
 		   	     pp2->copy = LVM_SCNDOF2;
                         } 
                         if((*pp)->copy == LVM_TERTOF3) {
			     pp1->snd_alt_part = 0;
                             pp1->snd_alt_vol = 0;
                             pp2->snd_alt_part = 0;
                             pp2->snd_alt_vol = 0;
                             pp1->copy = LVM_PRIMOF2;
                             pp2->copy = LVM_SCNDOF2;
                         }
                         break;
	} /* end switch */

    	return(LVM_SUCCESS);
}
/*****************************************************************************/
/* Name : extend_errchk 
*           
*  Function: This routine is called by rdex_proc to check the validity of
*               the values in the lp_num, and pv_id fields. 
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      vgfptr
*      map
*      lv
*
*    Output: If an incorrect value is found an error code is returned, if no
*            incorrect value is found, there is no output.
*
*   Return Values: LVM_SUCCESS if successful and one of the following if
*   not successful.
*             LVM_PPNUM_INVAL
*             LVM_LPNUM_INVAL
*             LVM_PVSTATE_INVAL
*/
/*****************************************************************************/
int extend_errchk(
char *vgfptr, 		/* pointer to volume group mapped file */
struct pp *map, 	/* pointer to one physical partition entry */
struct lv_entries *lv, 	/* pointer to the logical volume entry */
int *empty, 		/* indicates an empty logical partition is added */
struct logview *lp)  	/* logical view of logical volume */

{
   int rc; /* holds return codes */
   struct pv_header *pv; /* pointer to the physical volume header */
   struct pp_entries *pp; /* pointer to the physical partition entry */


	/* if an empty logical partition is being added
	 *  then check to be sure that that logical partition doesn't exist 
	 *  already. If one does, then return an error
	 *  If all is well, return success
	 */

   	if(map->pp_num == 0 &&( map->pv_id.word1 == 0 && map->pv_id.word2 == 0))
   	{
        	(*empty) = TRUE;
        	return(LVM_SUCCESS);
    	}

	/* if an entry has a zero pp_num then
	 *   return an error
	 */
     	if(map->pp_num <= 0) { 
  		lvm_errors("extend_errchk","rdex_proc",LVM_PPNUM_INVAL);
         	return(LVM_PPNUM_INVAL);
     	}

	/*
	 * Call get_pvandpp() to get the physical volume pointer and check the
	 * validity of the pv_id 
	 */

   	rc = get_pvandpp(&pv,&pp,NULL,vgfptr,&(map->pv_id));
    	if(rc < LVM_SUCCESS)
       		return(rc);
	/*
	 * If the physical partition number is greater than the 
	 * ppcount for the physical volume specified, return an error
	 */

     	if(map->pp_num > pv->pp_count) { 
  		lvm_errors("extend_errchk","rdex_proc",LVM_PPNUM_INVAL);
         	return(LVM_PPNUM_INVAL);
     	}

	/*
	 * set the pp pointer to the correct physical partition entry 
	 */
 
    	pp += (map->pp_num - 1);

	/*
	 * If the physical volume state for the specified physcial volume
	 * does not allow allocation return an error
	 */

     	if(pv->pv_state & LVM_PVNOALLOC) {
	   	lvm_errors("extend_errchk","rdex_proc",LVM_PVSTATE_INVAL);
          	return(LVM_PVSTATE_INVAL);
      	}



	/*

	 * If the physical partition state is not LVM_PPFREE 
	 *  return an error
	 */

      	if(pp->pp_state != LVM_PPFREE) {
   		lvm_errors("extend_errchk","rdex_proc",LVM_PPNUM_INVAL);
          	return(LVM_PPNUM_INVAL);
      	}

	/*
	 * if the lpnum is less than or equal to zero, or is greater than
	 * maxsize, return an error code 
	 */
      	if(map->lp_num > lv->maxsize || map->lp_num <= 0) {
    		lvm_errors("extend_errchk","rdex_proc",LVM_LPNUM_INVAL);
          	return(LVM_LPNUM_INVAL);
      	}


      	return(LVM_SUCCESS);
}
/*****************************************************************************/
/* Name : allocatelp 
*           
*  Function: This routine is called by rdex_proc to allocate a physical 
*               partition of a copy of a logical partition.
*
*  NOTES:
*
*    Environment:
*      It is assumed that the volume group mapped file being used in this
*      routine is LOCKED prior to the call to this routine.
*
*    Input:
*      vgfptr
*      minor_num
*      map
*      lp
*      lv
*
*    Output: The mapped file structures and the hard files will be updated to
*             show an additional logical partition.
*
*    Return Value : LVM_SUCCESS if succesful, otherwise, one of the following.
*             LVM_INVALID_PARAM
*             LVM_NOALLOCLP
*/
/*************************************************************************/


int allocatelp(
char *vgfptr, 	       /* pointer to the volume group mapped file */
short minor_num,       /* minor number of logical volume */
struct pp *map,	       /* pointer to the physical partition information */
struct logview *lp,    /* pointer to the logical view of the logical volume */
struct lv_entries *lv) /* pointer to the logical volume entry */

{
   int rc;      	 	    /* holds return codes */
   int emptycpy; 	 	    /* number of the first empty copy of the */
                         	    /* logical partition */
   int cnt;			    /* loop counter */
   struct fheader *fhead;	    /* pointer to the vg file header */
   struct pv_header *pv,*pv1,*pv2;  /* points to a physical volume header */
   struct pp_entries *pp,*pp1,*pp2; /* points to a physical partition entry */

	/* get the file header pointer */

    	fhead = (struct fheader *)(vgfptr);

	/*
	 *  Search the logical view to get the number of the first empty copy.
	 */
     	for(emptycpy=0; emptycpy < LVM_NUMCOPIES; emptycpy ++) 
       		/* 
         	 * if the entry is zeroed out, emptycpy will be the copy number
		 * of the zeroed entry
         	 */
        	if(lp->larray[map->lp_num -1][emptycpy].ppnum == 0 ) 
	   		break;

	/*
	* If there are already the maximum number of copies allowed of the
	* logical partition, return an error
	*/

     	if(emptycpy == LVM_NUMCOPIES) {
	 	lvm_errors("allocatelp","rdex_proc",LVM_NOALLOCLP);
         	return(LVM_NOALLOCLP);
     	}
 

	/*  
	 * Call get_pvandpp() to get the pointer to the first physical 
	 * partition entry on the physical volume specified in the struct pp
	 * entry. Set the pp pointer to the pp that corresponds to the physical
	 * partition number specified in the struct pp entry.
	 */

    	rc = get_pvandpp(&pv,&pp,NULL,vgfptr,&(map->pv_id));
    	if(rc < LVM_SUCCESS)
       		return(rc);
    	pp += (map->pp_num - 1);

	/* Get pointers to any already existing pps */
    	pp1 = NULL;
    	pp2 = NULL;
    	pv1 = NULL;
    	pv2 = NULL;

    	rc = get_ptrs(vgfptr,&fhead,NULL,NULL,NULL,NULL,NULL);
    	if(rc < LVM_SUCCESS)
        	return(rc);
    	if(emptycpy == 0) {  /* you are adding a brand new logical partition */
		allocation(pp,minor_num,map->lp_num,(char)LVM_PPALLOC,
					(short)0,(unsigned short)0,(short)0,
					(unsigned short)0,(char)LVM_PRIMARY);
		/* if numlps needs to be updated do so */

         	if(map->lp_num > lv->num_lps)
            		lv->num_lps  = map->lp_num;
    	}
    	else {   /* you are adding a mirror to a logical partition */
     	   for(cnt = emptycpy - 1; cnt >= 0; cnt --) {
     		switch(cnt) {
			/* 
			 * get the pointers to the existing copy and 
			 * update it to show the new copy that is added
              		 */
               		case 1:
			     pv2 = (struct pv_header *)((char *) vgfptr +
				 fhead->pvinfo[lp->larray[map->lp_num-1][cnt]
				 .pvnum-1].pvinx);
                       	     pp2 = (struct pp_entries *)((char *)pv2 + 
                                       sizeof(struct pv_header));
                       	     pp2 += (lp->larray[map->lp_num-1][cnt].ppnum -1);
                             pp2->snd_alt_part = map->pp_num;
                             pp2->snd_alt_vol = pv->pv_num;
                             pp2->copy = LVM_SCNDOF3;
              	             /*
                              * call allocation() to allocate the new physical
			      * partition
			      */
                             allocation(pp,minor_num,map->lp_num,
				 	 (char)LVM_PPALLOC,
                                         lp->larray[map->lp_num-1][cnt-1].pvnum,
                                         lp->larray[map->lp_num-1][cnt-1].ppnum,
                                         lp->larray[map->lp_num-1][cnt].pvnum,
                                         lp->larray[map->lp_num-1][cnt].ppnum,
				         (char)LVM_TERTOF3);
                             break;
               
		        case 0: 
			     /* 
			      * get the pointers to the existing copy and update
                              * them to show the new copy 
                              */
                             pv1 = (struct pv_header *)((char *)vgfptr +
                                   fhead->pvinfo[lp->larray[map->lp_num-1][cnt]
				   .pvnum-1].pvinx);
                             pp1 = (struct pp_entries *)((char *)pv1 +
                                   sizeof(struct pv_header));
                             pp1 += (lp->larray[map->lp_num-1][cnt].ppnum - 1);
                             if(emptycpy == THIRD_INDEX) { /* add a 3rd copy*/
                                 pp1->snd_alt_vol = pv->pv_num;
                                 pp1->snd_alt_part = map->pp_num;
                                 pp1->copy = LVM_PRIMOF3;
                             }
                             else {  /* a second copy is added */
                                 pp1->fst_alt_part = map->pp_num;
                                 pp1->fst_alt_vol = pv->pv_num;
                                 pp1->copy = LVM_PRIMOF2;
                                 allocation(pp,minor_num,map->lp_num,
                                           (char)LVM_PPALLOC,
                                           lp->larray[map->lp_num-1][cnt].pvnum,
                                           lp->larray[map->lp_num-1][cnt].ppnum,
                                           (short)0,(unsigned short)0,
					   (char)LVM_SCNDOF2);
                             } 
			     break;
	        } /* end switch */
	    }   /* end for */
	} /* end else */
	/* update the mirror field */
        if((emptycpy +1) > lv->mirror)
      	   lv->mirror = emptycpy + 1;

	/* Update the logical view to include the new partition */
   	lp->larray[map->lp_num -1][emptycpy].pvnum = pv->pv_num;
   	lp->larray[map->lp_num -1][emptycpy].ppnum = map->pp_num;
   	lp->larray[map->lp_num -1][emptycpy].ppstate = pp->pp_state;
   	if(emptycpy == 0)
      		lp->larray[map->lp_num -1][emptycpy].new = TRUE;

    
   	return(LVM_SUCCESS);
}
  

/*************************************************************************/
/* Name allocation
* 
*  Function - This routine allocates the fields on a physical partition. 
*
* NOTES:
*    Input
*      pp
*      lvindex
*      lpnum
*      ppstate
*      fv
*      fp
*      sv
*      sp
*
*    Output 
*      The physical partition fields are allocated to the argument values.
*
*
*   Return Value: LVM_SUCCESS
*
*/
/****************************************************************************/
void allocation(
struct pp_entries *pp,  /* pointer to physical partition to allocate */
short lvindex, 		/* minor number of logical volume */
long lpnum, 		/* logical partition number */
char ppstate,	  	/* state of physical partition */
short fv, 		/* first alternate physical volume with */
			/* secondary copy of the lp */
unsigned short fp,	/* first alt partition with the secondary copy */
short sv,		/* second alt physical volume with tertiary */
			/* copy of the lp */
unsigned short sp,	/* second alt partition with the tertiary copy */
                        /* of the lp */
char copy)              /* the copy number of the pp in a logical part. */
{

	/* Set the fields of the pp to be allocated to the values passed in */

     	pp->lv_index = lvindex;
     	pp->lp_num = lpnum;
     	pp->pp_state |= ppstate;
     	pp->fst_alt_vol = fv;
     	pp->fst_alt_part = fp;
     	pp->snd_alt_vol = sv;
     	pp->snd_alt_part = sp;
     	pp->copy = copy;
	return;
}

/*
 *	ignore signals if 'ignore' is non-zero, else set them back
 */
void
_lvm_ignoresigs(int ignore)
{
	int sig;
	int result;
	static int ignored[NSIG];

	if (ignore)
		for (sig = 1; sig < NSIG; sig++) {
			if ((result = signal(sig, SIG_IGN)) == SIG_DFL)
				ignored[sig] = 1;
			else {
				if (result != SIG_ERR)
					(void) signal(sig, result);
				ignored[sig] = 0;
				}
			}

	else /* reset */
		for (sig = 1; sig < NSIG; sig++)
			if (ignored[sig]) {
				(void) signal(sig, SIG_DFL);
				ignored[sig] = 0;
				}
}

/**************************************************************************/
