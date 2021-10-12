static char sccsid[] = "@(#)78	1.25.1.5  src/bos/usr/ccs/lib/liblvm/deletepv.c, liblvm, bos411, 9428A410j 4/21/93 17:27:52";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_deletepv
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

#include <sys/stat.h>
#include <sys/param.h>
#include <sys/hd_config.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_psn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lvmrec.h>
#include <liblvm.h>




extern int bzero();         /* zeroes out a structure */
extern int bcopy();         /* block copy */

/***********************************************************************
 *                                                                     *
 * NAME:  lvm_deletepv                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   The specified physical volume will be deleted from the volume     *
 *   group.                                                            *
 *                                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *     pv_id                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LVDD data structure for the specified physical volume will  *
 *     be deleted from the kernel.                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *              LVM_SUCCESS                                            *
 *              LVM_VGDELETED                                          *
 *   The following return values are set by this routine:              *
 *     Unsuccessful return codes:                                      *
 *              LVM_OFFLINE                                            *
 *              LVM_PROGERR  					       *
 *              LVM_MAPFOPN					       *
 *              LVM_MAPFSHMAT					       *
 *              LVM_INVALID_PARAM			               *
 *              LVM_ALLOCERR     			               *
 *              LVM_PVOPNERR      			               *
 *              LVM_LVMRECERR      			               *
 *              LVM_DALVOPN      			               *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   lvm_chkvaryon                                                     *
 *   lvm_getvgdef                                                      *
 *   open                                                              *
 *   close                                                             *
 *   ioctl                                                             *
 *                                                                     *
 ***********************************************************************
 */

int
lvm_deletepv (
struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be varied off */
struct unique_id * pv_id)
  /* the id of the physical volume which is to be deleted from the volume
     group */


{



    int status;              /* contains the return code  */
    int vgfd;                /* volume group mapped file descriptor */
    char *vgptr;             /* pointer to volume group mapped file */
    struct pv_header *pvd,*pvf,*pv;
                             /* pointers to physical volume headers */
    struct pp_entries *pp;   /* pointer to physical partition entries */
    long bcnt;               /* counts the blocks copied in the for loop */
    long ppcnt;              /* counts the pps checked in the for loop */
    long blocknum;           /* number of blocks to copy */
    long size;               /* size before conversion to a block boundary */
    long pvdsize;            /* size of the physical volume to be deleted */
                             /* and its pp enries on a block boundary */
    long pvfsize;  
                             /* size of the remainder of the pvs to be */
		             /* moved up */
    char vgmap_name [sizeof(LVM_ETCVG) + 2 * sizeof(struct unique_id)];
                             /* path name of the volume group mapped file */
    struct varyoffvg varyoff;
                             /* input structure to varyoffvg */
    struct fheader   *fhead;     /* fhptr points to the file header        */
    struct vg_header *vg;        /* pointer to the vg header               */
    struct vg_trailer *trail;	 /* pointer to vg trailer */
    short             pvnumber;  /* number of pv being changed */
    int lvfd;	 		 /* temporary file descriptor */
    int  cnt;                    /* counter */
    char twovgdas;		 /* TRUE if there's a pv with two vgdas */
    struct pv_header *pvtoget2; /* pointer to pv with one vgda that in */
				 /* the special case will get two */
    struct pv_header *pvtokeep1; /* pointer to pv with one vgda that in */
				 /* the special case will keep one */
    short old_totvgdas;		 /* stores old number of vgdas for the vg */
    long  new_curcnt;		 /* new number of vgdas that are current */
    char rsrvlv[LVM_EXTNAME];    /* name of the reserved area logical volume */
    char devname[LVM_NAMESIZ];   /* name of the physical volume being changed*/
    char pvname[LVM_EXTNAME];    /* full name of the pv being changed*/
    int  noname;		 /* indicates there is no name for this pv */
    caddr_t vgda_ptr;		 /* pointer to the beginning of the vg */
				 /* descriptor area */
    char match;			 /* indicator for matching vgid */
    int activepvs;		 /* number of active physcial volumes in vg */
    int das;		 	 /* descriptor area counter */
    int pvfd;			 /* file descriptor for physical volume */
    struct lvm_rec lvmrec;       /* holds lvmrecord info */       
    char caseflag;		 /* indicates what kind of delete we're doing */
    struct vgsa_area vgsa;       /* buffer for volume group status area copy */
    int test_state;		 /* TRUE if the physical volume is MISSING */
    int pvmode;	                 /* permissions for pv device if we make it */
    daddr_t fstsctr, lstsctr;    /* used for calculating VGSAs */
    daddr_t vgdapsn;		 /* primary psn of VGDA */
    daddr_t lsns[LVM_PVMAXVGDAS];/* array of lsns for VGSA'a */
    int  partlenblks;		 /* partition length in blocks */
    long quorum;		 /* temporary quorum count */
    char delpv_id[32];

	/*
	*   if the vg_id or the pv_id is NULL
	*      return an error
	*/

	if(vg_id == NULL) {
		lvm_errors("deletepv","     ",LVM_INVALID_PARAM);
        	return(LVM_INVALID_PARAM);
	}

    	if(pv_id == NULL) {
		lvm_errors("deletepv","     ",LVM_INVALID_PARAM);
        	return(LVM_INVALID_PARAM);
    	}


	/*   Call lvm_getvgdef to open and attach the mapped file */

     	status = lvm_getvgdef(vg_id,O_RDWR|O_NSHARE,&vgfd,&vgptr);
     	if(status < LVM_SUCCESS) {
        	lvm_errors("lvm_getvgdef","deletepv",status);
        	return(status);
     	}

	/*   Call get_ptrs() to get the necessary pointers
 	*   Call get_pvandpp to search for the PV id in the physical volume
 	*   information array in the mapped file header,and return a pointer to
 	*   to pv entry and its first pp entry
 	*/

     	status = get_ptrs(vgptr,&fhead,&vg,NULL,NULL,NULL,NULL);
     	if(status < LVM_SUCCESS) {
       		close(vgfd);
         	return(status);
     	}

     
 	status = get_pvandpp(&pvd,&pp,NULL,vgptr,pv_id);
     	if(status < LVM_SUCCESS) {
      		close(vgfd);
         	return(status);
     	}

   	/*
   	*  call status_chk () to get the name of the descriptor area lv
    	*  open the descriptor area logical volume for later use
    	*  save the pv number, major number of the volume group, and the pvname
    	*/

   	status = status_chk(NULL,fhead->vgname,NOCHECK,rsrvlv);
   	if( status < LVM_SUCCESS) {
		close(vgfd);
		return(status);
   	}	
   	lvfd = open(rsrvlv,O_RDWR);
   	if(lvfd == LVM_UNSUCC) {
		close(vgfd);
		return(LVM_DALVOPN);
   	}
   	vgda_ptr = vgptr + sizeof(struct fheader);
   	pvnumber = pvd->pv_num;
	noname = FALSE;

   	sprintf(delpv_id,"%08x%08x%08x%08x", pv_id->word1, 
			       pv_id->word2,
			       pv_id->word3,
			       pv_id->word4);
   	get_odm_pvname(&delpv_id, &fhead->pvinfo[pvnumber-1].pvname);

	if(fhead->pvinfo[pvnumber-1].pvname == NULL) {
		pvmode = S_IFCHR|S_IRUSR|S_IWUSR;
		status = buildname(fhead->pvinfo[pvnumber-1].device,
					devname,pvmode,LVM_PVNAME);
		if(status < LVM_SUCCESS) {
			close(vgfd);
			close(lvfd);
			return(status);
		}
        }
	else
   		strncpy(devname,fhead->pvinfo[pvnumber-1].pvname,LVM_NAMESIZ-1);
   	status = status_chk(NULL,devname,NOCHECK,pvname);
   	if(status < LVM_SUCCESS) 
		noname = TRUE;

        /* 
         * call getstates() to get a copy of the volume group status area
  	 * for use in checking the physical volume state later
   	 */
 	status = getstates(&vgsa,vgptr);
        if(status < LVM_SUCCESS) {
		close(vgfd);
		close(lvfd);
		return(status);
	}

	/*  save pointer to the pv to be deleted */

      	pv = pvd;


	/*   For PP number from 1 to the total number of physical partitions
         *   for this PV (pp_count in the PV header) check to be sure all pps
         *   for this PV are free and return if any are not
 	*/
      	for(ppcnt = 1; ppcnt <= pvd->pp_count; ppcnt ++) {
       		if(pp->pp_state != LVM_PPFREE) {
          		lvm_errors("deletepv","       ",LVM_PARTFND);
              		close(vgfd);
              		close(lvfd);
              		return(LVM_PARTFND);
         	 }
         	 pp++;
       	}

	/*   Decrement the number of PVs in the VG header by 1;
	 */
     	vg->numpvs --;


	/* save off the total_vgdas in the vg_header and reset total_vgdas */
        
  	old_totvgdas = vg->total_vgdas;
        vg->total_vgdas  = old_totvgdas - pvd->pvnum_vgdas;

	/*   Find the size of the PV entry for the PV which is to be deleted;
	*/
     
       	size = sizeof(struct pv_header) +
        	 (sizeof(struct pp_entries) * pvd->pp_count);
 
       	pvdsize = LVM_SIZTOBBND(size);
     

	/*   Set a pointer to the beginning of the PV entry which follows this
  	*    PV in the list;
 	*/

     	pvf = (struct pv_header *)((char * )pvd + pvdsize);

	/*   Find the size from the PV entry following this PV to the end of the
 	*   PV list;
	*   NOTE: these calculations assume that endpvs was computed with
	*   a block boundary scale.
 	*/

     	pvfsize = (((char *)vgptr + fhead->endpvs) - (char *)pvf);
     	blocknum = pvfsize / DBSIZE;


	/*   For number of blocks in the PV list from the following PV entry to
 	*   the end of the list
 	*       Copy from following entry pointer to deleted entry pointer for
 	*       length of one block;
 	*       Increment each pointer by the size of a block;
	*/
     
     	for(bcnt = 1; bcnt <= blocknum; bcnt ++) {
     		bcopy((char *)pvf,(char *)pvd,DBSIZE);
		pvf = (struct pv_header *)((char *)pvf + DBSIZE);
		pvd = (struct pv_header *)((char *)pvd + DBSIZE);
     	} 

	/* Update offset for end of PV list in the mapped file header with the
 	*   new value;
 	*/
  
     	fhead->endpvs = (char *)pvd - (char *)vgptr;

	/*   Zero out from new end of PV list for the size of the deleted PV
 	*   entrys. Zero out the entry in the pvinfo array of the volume
  	*   group file header for the pv we're deleteing
 	*/
     
     	bzero((char *)pvd,pvdsize);
     	bzero((char *)&(fhead->pvinfo[pvnumber-1]),sizeof(struct pvinfo));
     
	/*
	* recalculate and update the offsets for the other physical volumes
	* in the pvinfo structure
	*/

     	while((char *)pv < ((char *) vgptr + fhead->endpvs)) {
        	fhead->pvinfo[pv->pv_num-1].pvinx = ((char *)pv-(char *)vgptr);
         	size = sizeof(struct pv_header) +
            		(sizeof(struct pp_entries) * pv->pp_count);
         	pvdsize = LVM_SIZTOBBND(size);
         	pv = (struct pv_header *)((char *)pv + pvdsize);
     	}

    	/*   Open the physical volume device for this PV */
	if(noname == FALSE)
     		pvfd = open(pvname,O_RDWR);

        /* if the total vgdas is 0, set quorum count to 0 */

	if(vg->numpvs == 0) {
		caseflag = LVM_LASTPV;
		fhead->quorum_cnt = 0;
	}
	else
	/* set quorum to new total_vgdas/2 +1 */
		fhead->quorum_cnt = (vg->total_vgdas / 2) + 1;

	/* initialize variables */
        trail = (struct vg_trailer *)(fhead->trailinx + (char *)fhead);
	twovgdas = FALSE;
	new_curcnt = 0;
        activepvs = 0;
	match = FALSE;
        pvtoget2 = (struct pv_header *)NULL;
        pvtokeep1 = (struct pv_header *)NULL;

        /*
        *      loop for the max number of pvs in the volume group
	*		totalling the number of vgdas on all active pvs
	*			so that we can check that a quorum 
	*			remains after the pv is deleted
	*		toalling the number of active pvs to be used
	*			in quorum checking
	*		saving pointers to the pvs that will be used
	*			if we find that there's a special case
        */

        for(cnt=0; cnt < LVM_MAXPVS; cnt ++) {
		/* if this is a valid physical volume */   
		if(fhead->pvinfo[cnt].pvinx != 0) {
			/* set a pointer to the physical volume header */
			pv = (struct pv_header *) ((char *)vgptr +
				fhead->pvinfo[cnt].pvinx);
			/*
			* use the macro in vgsa.h to get the pv state
			*/
			test_state = TSTSA_PVMISS(&vgsa,cnt) ? TRUE : FALSE;
			/* if this is physical volume is NOT MISSING */
			if(test_state == FALSE) {
 				new_curcnt += pv->pvnum_vgdas;
				if(pv->pvnum_vgdas == LVM_TTLDAS_1PV)
					twovgdas = TRUE;
				if(pv->pvnum_vgdas == LVM_DAPVS_TTL1 && 
			     	    pvtoget2 == NULL)
					/* 
					* save a pointer to the pv that will
					* need two vgdas          
					*/
					pvtoget2 = pv;
				else
				 	if(pvtokeep1 == NULL)
						/*
						* save a pointer to the pv that
						* will keep only one vgda
						*/
						pvtokeep1 = pv;
				activepvs ++;
			}  /* the pv we're checking is active */
	   	} /* the pv is valid for the volume group */	
            } /* for loop of MAXPVS */	
		
	/*
	*  check to see what kind of delete we're doing and set
	*  the appropriate indicators
	*/
	
        if(new_curcnt >= fhead->quorum_cnt) {
       	   if(old_totvgdas <= LVM_TTLDAS_2PV) {
             	if(vg->total_vgdas == LVM_DAPVS_TTL2 && twovgdas == FALSE) 
			/* we have a special case for 3 to 2 pvs */
			caseflag = LVM_CASE3TO2;
                if(vg->total_vgdas == LVM_DAPVS_TTL1 && pvtoget2 != NULL) 
			/* we have a special case for 2 to 1 pv */
			caseflag = LVM_CASE2TO1;
		if((twovgdas == TRUE) || (vg->numpvs > 2))
			caseflag = LVM_CASEGEN;
	   }
	   else
		/* we have a general delete of an active pv */
			caseflag = LVM_CASEGEN;
	}
	else { /* no quorum */
		close(lvfd);
 	  	close(vgfd);
 	  	close(pvfd);
	        return(LVM_BELOW_QRMCNT);
  	}
	
	/*
	 * if we're adding a VGSA to a remaining physical volume
	 * we need to calculate what the quorum will be BEFORE
	 * the pv is deleted and AFTER the extra copy of the VGSA is
	 * added. Then, we calculate the correct logical sector number
	 * for the secondary copy of the VGSA(the one we will add). NOTE
	 * that we ensure that the second copy of the VGSA will fall within
	 * the same logical track group. After these calculations, we have
	 * the needed arguments to call lvm_chgvgsa() to add a VGSA copy
	 */
	if((caseflag == LVM_CASE3TO2) || (caseflag == LVM_CASE2TO1)) {
	   quorum = ((old_totvgdas + 1) / 2) + 1;
           partlenblks = LVM_PPLENBLKS(vg->pp_size);
	   vgdapsn = PSN_NONRSRVD + VGSA_BLK;
           fstsctr = vgdapsn +  fhead->vgda_len;
           lstsctr = fstsctr + VGSA_BLK - 1;
           if(BLK2TRK(fstsctr) != BLK2TRK(lstsctr)) 
               fstsctr = TRK2BLK(BLK2TRK(fstsctr) + 1);
	   lsns[FIRST_INDEX] = 0;
	   lsns[SEC_INDEX] = fhead->num_desclps * partlenblks * 
				(pvtoget2->pv_num - 1) + fstsctr; 
           status = lvm_chgvgsa(vg_id,fhead->major_num,lsns,pvtoget2->pv_num,
			        (short)quorum,HD_KADDVGSA);
	   if(status < LVM_SUCCESS)
	      return(status);
	}	

	switch(caseflag) {
	/*
	 * if we have a special 3 to 2 case
	 *         the 2 remaining pvs must be active
         */

	case LVM_CASE3TO2:    
		if(activepvs == 2) {
			new_curcnt = 0;
			/*
			* increment the necessary fields to show that a
			* vgda has been added to a pv
			*/
			pvtoget2->pvnum_vgdas ++;
			vg->total_vgdas ++;
 			quorum = (vg->total_vgdas / 2 ) + 1;

			/* Call lvm_delpv() to delete the pv from the kernel */
 			status = lvm_delpv(vg_id,fhead->major_num,pvnumber,
					     fhead->num_desclps,
					      HD_KDELPV, (short)quorum);
			if(status < LVM_SUCCESS) {
				close(vgfd);
			        close(pvfd);
				close(lvfd);
				return(status);
			}
			/* 
			 * call timestamp() to get the current time from the
			 * system clock and store it in the vg header and
			 * trailer timestamp 
   			*/
			status = timestamp(vg,vgptr,fhead);
			if(status < LVM_SUCCESS) {
				close(vgfd);
				close(lvfd);
				close(pvfd);
				return(status);
			}
			/* 
			* call lvm_special_3to2() to write the vgda
			* to the pv that will keep one vgda
			* then to the pv that will have two vgdas
			*/
			if(noname == FALSE) {		
		        	status = lvm_special_3to2(pvname,vg_id,lvfd,
					vgda_ptr, pvnumber,pvtokeep1->pv_num,
		            		pvtoget2->pv_num,&match,(char)TRUE,
					fhead); 
				if(status < LVM_SUCCESS) {
				    close(vgfd);
				    close(lvfd);
				    close(pvfd);
				    return(status);
 				}
			}
			else {
		           status = lvm_special_3to2(NULL,vg_id,lvfd,vgda_ptr,
			        	pvnumber,pvtokeep1->pv_num,
		            		pvtoget2->pv_num,&match,(char)TRUE,
					fhead); 
			   if(status < LVM_SUCCESS) {
				close(vgfd);
				close(lvfd);
				close(pvfd);
				return(status);
			   }
			}
			/*
			* reset the current count of vgdas that were written
			* and recalculate the quorum count
			*/
			fhead->quorum_cnt = (vg->total_vgdas / 2) + 1;
			/*
			* if the pv's lvm record shows it as a member
			* of this volume group then zero its lvm record
			*/
			if(noname == FALSE) {
				lvm_cmplvmrec(vg_id,&match,pvname);
 				if(match == TRUE)
					lvm_zerolvm(pvfd);
			}
		} /* two active pvs */
	        else {  /* not enough active pvs remain */
			  close(lvfd);
			  close(vgfd);
			  close(pvfd);
			  return(LVM_BELOW_QRMCNT);
	        }
		break;                      

	/*
	 *    if we have a special case of 2 to 1 pv
	 *    for the 2 to 1 case, we need to be sure that the
	 *        remaining pv is active and there's a pv with one
	 *        vgda copy (if it's active, it'll have one copy)
	 */

	case LVM_CASE2TO1 :
		if(activepvs == 1) {
			/*
			* increment the necessary fields to show that a
			* vgda has been added to a pv
			*/
			vg->total_vgdas ++;
	        	pvtoget2->pvnum_vgdas ++;
			quorum  = (vg->total_vgdas / 2 ) + 1;
			/* Call lvm_delpv() to delete the pv from the kernel */
 			status = lvm_delpv(vg_id,fhead->major_num,pvnumber,
					      fhead->num_desclps,
					       HD_KDELPV, (short)quorum);
			if(status < LVM_SUCCESS) {
  				close(lvfd);
				close(vgfd);
				close(pvfd);
				return(status);
			}
			/* 
			* call lvm_cmplvmrec() to make sure the pv's
			* lvm record shows it as a member of this vg
			*/

			if(noname == FALSE) {
	        		lvm_cmplvmrec(vg_id,&match,pvname);
				/* 
			 	* if the lvm record matches the vgid passed in 
			 	* then call lvm_zerolvm() to zero out the lvm
			 	* record on the pv we're deleteing
				*/

				if(match == TRUE)
					lvm_zerolvm(pvfd);
			}
			/*
			 * call timestamp() to get the current system
			 * time and store it in the vg header and trailer
		    	 * timestamps
			*/
			status = timestamp(vg,vgptr,fhead);
			if(status < LVM_SUCCESS) {
				close(vgfd);
				close(lvfd);
				close(pvfd);
				return(status);
			}
			/* write the vgdas to the pv that is left */
			status = lvm_wrtnext(lvfd,vgda_ptr,&(trail->timestamp),
			        	pvtoget2->pv_num,fhead,(short int)2);
			if(status < LVM_SUCCESS) {
			    close(vgfd);
			    close(lvfd);
      			    close(pvfd);
			    return(status);
			}
			/* 
			*  recalculate and reset the quorum count 
			*/
			fhead->quorum_cnt = (vg->total_vgdas /2) + 1;
		}
	 	else { /* not enough active pvs remain */
		        close(lvfd);
 		        close(vgfd);
                        close(pvfd);
			return(LVM_BELOW_QRMCNT);
		}
		break;

        /*
	* if we have a general case of deleteing a pv
       	*        call lvm_cmplvmrec() to be sure the vgid of the
	*            volume group matches the vgid on the lvmrec of
	*            the pv we're deleteing
	*        if the vgids match
	*           zero the lvm record on the pv being deleted
	*/

	case LVM_CASEGEN :  
                quorum = (vg->total_vgdas / 2) + 1;
		/* Call lvm_delpv() to delete the pv from the kernel */
 		status = lvm_delpv(vg_id,fhead->major_num, pvnumber,
				 	fhead->num_desclps,
				        HD_KDELPV,(short) quorum);
		if(status < LVM_SUCCESS) {
			close(vgfd);
			close(pvfd);
			close(lvfd);
			return(status);
		}
		/* Call lvm_diskio routine to update the on-disk VGDAS */
        	status = lvm_diskio(vgptr,vgfd);
        	if(status < LVM_SUCCESS) {
         		lvm_errors("lvm_diskio","deletepv",status);
            		close(vgfd);
			close(lvfd);
			close(pvfd);
            		return(status);
        	}
		if(noname == FALSE) {
			lvm_cmplvmrec(vg_id,&match,pvname);
	        	if(match == TRUE)
		       		lvm_zerolvm(pvfd);
		}
		break;

	/*
	* If the last physical volume of the volume group is being deleted
	*         Close the mapped file;
	*         Call varyoffvg for this volume group;
	*         Delete the mapped file;
	*         send back a positive return code that will indicate 
	*	  the vg is deleted 
	*/

     	case LVM_LASTPV : 
		if(noname == FALSE) {
			lvm_cmplvmrec(vg_id,&match,pvname);
	        	if(match == TRUE)
		       		lvm_zerolvm(pvfd);
		}
        	close(vgfd);
		close(lvfd);
		/* set up the varyoff input structure */
         	varyoff.vg_id.word1 = vg_id->word1;
         	varyoff.vg_id.word2 = vg_id->word2;
         	varyoff.lvs_only = FALSE;
		/* call varyoffvg() */
 	        status = lvm_varyoffvg((&varyoff));
   		/* create name of mapped file */
        	LVM_MAPFN(vgmap_name,vg_id);
    		/* delete the mapped file */
        	status = unlink(vgmap_name);
 		close(pvfd);
		return(LVM_VGDELETED);
	} /* end switch */
	
      	close(vgfd);
        close(pvfd);
	close(lvfd);

	return(LVM_SUCCESS);

} /* END lvm_deletepv */
