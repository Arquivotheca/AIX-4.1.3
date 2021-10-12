static char sccsid[] = "@(#)72	1.36  src/bos/usr/ccs/lib/liblvm/changepv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:50:05";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_changepv
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

#include <sys/hd_config.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_psn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lvmrec.h>
#include <liblvm.h>
#include <sys/cfgodm.h>
#include <odmi.h>

int lvm_returnpv(
   struct unique_id *vg_id,
   caddr_t          pvname,
   struct pv_header *pv_ptr,
   struct fheader *mapf_ptr,
   short int quorum_cnt);

/*****************************************************************************
*  Name : lvm_changepv 
*
*  Function:  This function is called by a system management command to
*  change the attributes of a physical volume.
*
*  NOTES:
*
*    Input:  
*     changepv
*
*    Output:
*      If successful, the mapped data structure file and the descriptors
*      on the hard file will be updated; otherwise, an error code is returned.
*
*    Return Values: If successful, the routine returns LVM_SUCCESS. If the
*    routine fails, one of the following is returned.
*          LVM_INVALID_PARAM
*          LVM_OFFLINE
*          LVM_BELOW_QRMCNT
*          LVM_MAPFOPN
*          LVM_PVERROR
*          LVM_ALLOCERR
*          LVM_LVOPEN
*          LVM_PROGERR
*          LVM_PVNOTINVG
*          LVM_PVOPNERR
*          LVM_CFGRECERR
*          LVM_PVDAREAD
*/
/*****************************************************************************/
int
lvm_changepv (
struct changepv *changepv) /* pointer to struct changepv */

{
    struct pv_header *pvptr;     /* pvptr points to the physical header    */
    struct pv_header *pv;        /* temporary pointer to a pv header       */
    struct fheader   *fhead;     /* fhptr points to the file header        */
    struct fheader   head;       /* fhptr points to the file header        */
    struct vg_header *vg;        /* pointer to the vg header               */
    struct unique_id vgid;       /* volume group id */
    struct vg_trailer *trail;    /* pointer to volume group trailer */
    long              major_num; /* major number of volume group */
    short             pvnumber;  /* number of pv being changed */
    short             pvnum2;    /* number of pv with 2 vgdas */
    short             pvnum1;	 /* number of pv with 1 vgda */
    struct varyonvg vary;        /* variable for varyonvg                  */
    int vgfd = 0;                /* map file descriptor                    */
    int lvfd;	 		 /* temporary file descriptor */
    mid_t kmid;			 /* kernel module id */
    int status = 0;              /* return error code for lvm_changepv     */
    int rcsave;			 /* return codes that need to be saved */
    char *vgptr;                /* pointer to the volume group mapped file*/
    int  diff;                   /* current vgdas - pvnum vgdas of pv deleted */
    int  cnt, numpvs;            /* for loop counters */
    char twovgdas;		 /* TRUE if there's a pv with two vgdas */
    struct pv_header *pvtoget2;  /* pointer to pv with one vgda that in */
				 /* the special case will get two */
    struct pv_header *pvtokeep1; /* pointer to pv with one vgda that in */
				 /* the special case will keep one */
    short old_totvgdas;		 /* stores old number of vgdas for the vg */
    long  new_curcnt;		 /* new number of vgdas that are current */
    char rsrvlv[LVM_EXTNAME];    /* name of the reserved area logical volume */
    char devname[LVM_NAMESIZ];   /* name of the physical volume being changed*/
    char pvname[LVM_EXTNAME];    /* full name of the pv being changed*/
    int  noname;		 /* indicates no pvname for this pv */
    caddr_t vgda_ptr;		 /* pointer to the beginning of the vg */
				 /* descriptor area */
    char match;			 /* indicator for matching vgid */
    int offset;		         /* offset to seek to */
    int pvfd;			 /* physical volume file descriptor */
    int activepvs;		 /* number of active physcial volumes in vg */
    short desclps;		 /* number of partitions in the desc area */
    int das;		 	 /* descriptor area counter */
    struct lvm_rec lvmrec;       /* holds lvmrecord info */       
    struct inpvs_info inpvs_info;
				 /* structure that contains info about pvs */
				 /* in the varyonvg input list */
    struct defpvs_info defpvs_info;
				 /* structure that contains info about vgdas */
				 /* and vgsas for the defined pvs in a vg */
    short  ppsize;	         /* size of the physical partitions in the vg */
    daddr_t begpsn;		 /* physical sector number of first pp on pv */
    int   caseflag;		 /* indicates what kind of remove we're doing*/
    char can_read;	        /* indicates the pv being returned was read */
    int  new_quorum;		/* temporary quorum count */
    int  test_state;	        /* TRUE if pv is MISSING */           
    struct vgsa_area vgsa;      /* buffer to hold volume group status area */
    int pvmode;			/* permissions for pv device if we make it */
    daddr_t fstsctr, lstsctr;    /* used for calculating VGSAs */
    daddr_t vgdapsn;		 /* primary psn of VGDA */
    daddr_t lsns[LVM_PVMAXVGDAS];/* array of lsns for VGSA'a */
    int partlenblks;		 /* partition length in blocks */
    long quorum;		 /* temporary quorum count */
    char chgpv_id[32];
   
  /****************************************************************************/

/*
*   If there is no info pointer sent in 
*     return an error
*/

    if(changepv == NULL)
    {   lvm_errors("changepv","      ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*  If both change states are zero then return an error
*/

     if(changepv->rem_ret == 0 && changepv->allocation == 0)
     {   lvm_errors("changepv","     ",LVM_INVALID_PARAM);
         return(LVM_INVALID_PARAM);
     }

/* call lvm_getvgdef() to lock and get volume group discriptor information */

   if((status = lvm_getvgdef(&(changepv->vg_id),O_RDWR|O_NSHARE,
       &vgfd,&vgptr)) < 0 )
   {
      lvm_errors("changepv","changepv",status);
      free(vgptr);
      return(status);
   }

/* Call get_pvandpp() to verify the pv_id is valid and to get a pointer to 
*  the pv header if the pv_id is valid
*/

    if((status = get_pvandpp(&pvptr,NULL,NULL,vgptr,&(changepv->pv_id)))
          < 0 )
    {  lvm_errors("get_pvandpp","changepv",status);
       close(vgfd);
       free(vgptr);
       return(status);
    }

/*
*  get the needed pointers
*/

     status = get_ptrs(vgptr,&fhead,&vg,NULL,NULL,NULL,NULL);
     if(status < LVM_SUCCESS)
     {   close(vgfd);
         free(vgptr);
         return(status);
     }

/*
*   process the change of the physical volume state  
*/
 
 if(changepv->allocation != 0) {
   switch(changepv->allocation) {
    case LVM_NOALLOCPV:    /* turn on PVNOALLOC */
 
	/*
	 * if the noalloc bit is on
	 *     break
	 * else turn the noalloc bit on
	 */

        if(pvptr->pv_state & LVM_PVNOALLOC) 
        	break;     
        else {
		pvptr->pv_state |= LVM_PVNOALLOC;
		/* call diskio() to commit changes to hardfile */
   		if((status = lvm_diskio(vgptr,vgfd))  <  LVM_SUCCESS )
   		{
      			close(vgfd);
      			lvm_errors("changepv","changepv",status);
         		free(vgptr);
      			return(status);
   		}
	}
        break;

    case LVM_ALLOCPV:     /* turn off PVNOALLOC */
 
	/*
	 *   if the noalloc bit is off
	 *     break
	 *   else
	 *      turn the noalloc bit off
	 */

	if(!(pvptr->pv_state & LVM_PVNOALLOC)) 
                break;
   	else {
        	pvptr->pv_state &= LVM_ALLOCMASK;
		/* call diskio() to commit changes to hardfile */
   		if((status = lvm_diskio(vgptr,vgfd))  <  LVM_SUCCESS )
   		{
      			close(vgfd);
      			lvm_errors("changepv","changepv",status);
         		free(vgptr);
      			return(status);
   		}
	}
        break;
    default : 
	lvm_errors("changepv","    ",LVM_INVALID_PARAM);
        close(vgfd);
        free(vgptr);
        return(LVM_INVALID_PARAM);
   }  
 }
 if(changepv->rem_ret != 0) {

   /*
    *  call status_chk () to get the name of the descriptor area lv
    *  open the descriptor area logical volume for later use
    *  save off the pvnum for the pv being returned or removed, the major
    *  number of the volume group, the volume group id, the pvname, and 
    *  the pp_size
    */

   status = status_chk(NULL,fhead->vgname,NOCHECK,rsrvlv);
   if( status < LVM_SUCCESS) {
	close(vgfd);
        free(vgptr);
	return(status);
   }	
   lvfd = open(rsrvlv,O_RDWR);
   if(lvfd == LVM_UNSUCC) {
	close(vgfd);
        free(vgptr);
	return(LVM_DALVOPN);
   }
   vgda_ptr = vgptr + sizeof(struct fheader);
   trail = (struct vg_trailer *)(vgptr + fhead->trailinx);
   pvnumber = pvptr->pv_num;
   major_num = fhead->major_num;
   desclps = fhead->num_desclps;
   vgid.word1  = vg->vg_id.word1;
   vgid.word2  = vg->vg_id.word2;
   noname = FALSE;
   ppsize = vg->pp_size;

   sprintf(chgpv_id,"%08x%08x%08x%08x", changepv->pv_id.word1, 
			       changepv->pv_id.word2,
			       changepv->pv_id.word3,
			       changepv->pv_id.word4);
   get_odm_pvname(&chgpv_id, &fhead->pvinfo[pvnumber-1].pvname);
   if(fhead->pvinfo[pvnumber-1].pvname == NULL) 
        noname = TRUE;
   else {
   	strncpy(devname, fhead->pvinfo[pvnumber-1].pvname, LVM_NAMESIZ - 1);
        status = status_chk(NULL,devname,NOCHECK,pvname);
        if(status < LVM_SUCCESS)
	    noname = TRUE;
   }
   /*
    * call getstates() to get a copy of the volume group status area 
    * for use in checking the physical volume state for ACTIVE and MISSING
    */
   status = getstates(&vgsa,vgptr);
   if(status < LVM_SUCCESS) {
	close(vgfd);
   	close(lvfd);
        free(vgptr);
	return(status);
   }
   switch(changepv->rem_ret) {

   case  LVM_REMOVEPV: 
	/* save off the total_vgdas in the vg_header and reset total_vgdas */
        
  	old_totvgdas = vg->total_vgdas;
    vg->total_vgdas  = old_totvgdas - pvptr->pvnum_vgdas;
         
	/*
     * set quorum to new total_vgdas/2 +1
     * set the pvnum_vgdas field to zero on the physical volume to 
     * be removed
     */
  
	fhead->quorum_cnt = (vg->total_vgdas /2) + 1;
	if(pvptr->pv_state & LVM_PVREMOVED) {
		close(lvfd);
		break;
	}
	else{ 
		pvptr->pv_state = (LVM_ACTIVEMASK & pvptr->pv_state) | 
 			 LVM_PVREMOVED;
		}
     
	/* initialize variables */

        pvptr->pvnum_vgdas = 0;
    	new_curcnt = 0;
        caseflag = LVM_CASEGEN;
        activepvs = 0;
        twovgdas = FALSE;
        pvtoget2 = (struct pv_header *)NULL;
        pvtokeep1 = (struct pv_header *)NULL;
        match = FALSE;

    /*
     *      loop for the max number of pvs in the volume group
     * 		totalling the number of vgdas on all active pvs
	 *			so that we can check that a quorum
	 *			remains after the pv is deleted
	 * 		totalling the number of active pvs to be used in
	 *			quorum checking
	 *		saving pointers to the pvs that will be used if
	 *			we find that there's a special case
     */

	for(cnt=0; cnt < LVM_MAXPVS; cnt ++) {
		/* if this is a valid physical volume */
		if(fhead->pvinfo[cnt].pvinx != 0) {
			/* set a pointer to the physical volume header */
			pv = (struct pv_header *) ((char *)vgptr +
				fhead->pvinfo[cnt].pvinx);
 			/*
		        * use the macro from vgsa.h to get the state of
 		        * the physical volume 
		        */
			test_state = TSTSA_PVMISS(&vgsa,cnt) ? TRUE : FALSE; 
			/* if this is not a MISSING physical volume */
			if(test_state == FALSE) {
 				new_curcnt += pv->pvnum_vgdas;
				if(pv->pvnum_vgdas == LVM_TTLDAS_1PV)
					twovgdas = TRUE;
				if(pv->pvnum_vgdas == LVM_DAPVS_TTL1 && 
			     		pvtoget2 == NULL)
						/* 
						* save a pointer to the pv that
						* will need two vgdas
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
			} /* the pv we're checking was not MISSING*/
	   	} /* the pv is valid for the volume group */	
	} /* for loop of MAXPVS */ 	

	/* check to see what kind of remove we're doing */
               
        if(new_curcnt >= fhead->quorum_cnt) {
             if(old_totvgdas <= LVM_TTLDAS_2PV) {
                 if(vg->total_vgdas == LVM_DAPVS_TTL2 && twovgdas == FALSE) 
		 /* we have a special case for 3 to 2 pvs */
		 	caseflag = LVM_CASE3TO2;
		 if(vg->total_vgdas == LVM_DAPVS_TTL1 && pvtoget2 != NULL)
		 /* we have a special case for 2 to 1 pv */
			caseflag = LVM_CASE2TO1;
		 if(twovgdas == TRUE)
			caseflag = LVM_CASEGEN;
             } /* old total vgdas <= 3 */
	     else  /* we have a general case here too */
			caseflag = LVM_CASEGEN;
	}
	else { /* no quorum */
		close(lvfd);
		close(vgfd);
        	free(vgptr);
		return(LVM_BELOW_QRMCNT);
	}

	/*
	 * if we're adding a VGSA to a remaining physical volume
	 * we need to calculate what the quorum will be BEFORE
	 * the pv is removed and AFTER the extra copy of the VGSA is
	 * added. Then, we calculate the correct logical sector number
	 * for the secondary copy of the VGSA(the one we will add). NOTE
	 * that we ensure that the second copy of the VGSA will fall within
	 * the same logical track group. After these calculations, we have
	 * the needed arguments to call lvm_chgvgsa() to add a VGSA copy
	 */
	if((caseflag == LVM_CASE3TO2) || (caseflag == LVM_CASE2TO1)) {
	   quorum = ((old_totvgdas + 1) / 2) + 1;
	   vgdapsn = PSN_NONRSRVD + VGSA_BLK;
           partlenblks = LVM_PPLENBLKS(vg->pp_size);
           fstsctr = vgdapsn +  fhead->vgda_len;
           lstsctr = fstsctr + VGSA_BLK - 1;
           if(BLK2TRK(fstsctr) != BLK2TRK(lstsctr)) 
               fstsctr = TRK2BLK(BLK2TRK(fstsctr) + 1);
	   lsns[FIRST_INDEX] = 0;
	   lsns[SEC_INDEX] = fhead->num_desclps * partlenblks *
				(pvtoget2->pv_num - 1) + fstsctr; 
           status = lvm_chgvgsa(&vgid,fhead->major_num,lsns,pvtoget2->pv_num,
			        (short)quorum,HD_KADDVGSA);
	   if(status < LVM_SUCCESS)
	   {
              free(vgptr);
	      return(status);
	   }
	}	

	switch(caseflag) {
		/*
  		*  if we have a special 3 to 2 case, the remaining 2 pvs must
		*  be active
		*/
		case LVM_CASE3TO2:
                      if(activepvs >= 2) {
			  /*
			   * increment the necessary fields to show that a
			   * vgda has been added to a pv
			   */
	                  pvtoget2->pvnum_vgdas ++;
			  vg->total_vgdas ++;
			  quorum = (vg->total_vgdas / 2) + 1;
			  /*
			   * call lvm_delpv() to remove the physical volume
			   * from the kernel
			   */
			   status = lvm_delpv(&vgid,major_num,pvnumber,
			    	fhead->num_desclps,HD_KREMPV,(short)quorum);
			   if(status < LVM_SUCCESS) {
				close(vgfd);
				close(lvfd);
              			free(vgptr);
				return(status);
			    }
			  /* 
			   * call timestamp() to get the current time
			   * from the system clock and put it in the
			   * vg header and trailer timestamps
		           */

		       	  status = timestamp(vg,vgptr,fhead);
			  if(status < LVM_SUCCESS) {
				 close(vgfd);
				 close(lvfd);
              			 free(vgptr);
				 return(status);
			  }
			  /* 
			   * call lvm_special_3to2() to try to write the vgda
			   * to the pv we're removing and then to write the
			   * vgda to the pv that will keep one copy and then
			   * to the pv that will keep two copies
			   */
			  if(noname == FALSE) {
			     status = lvm_special_3to2(pvname,&vgid,lvfd,
					vgda_ptr, pvnumber,pvtokeep1->pv_num,
					pvtoget2->pv_num,&match,(char)FALSE,
					fhead); 
			      if(status < LVM_SUCCESS) {
				 /* we've been forced off */
				 close(vgfd);
				 close(lvfd);
              			 free(vgptr);
				 return(status);
			      }	
			  }	
			  else {
			     status = lvm_special_3to2(NULL,&vgid,lvfd,
					vgda_ptr,pvnumber,pvtokeep1->pv_num,
					pvtoget2->pv_num,&match,(char)FALSE,
					fhead); 
			     if(status < LVM_SUCCESS) {
			       /* we've been forced off */
				close(vgfd);
				close(lvfd);
              			free(vgptr);
				return(status);
			     }
			  }
			  /*  recalculate the quorum count */
			  fhead->quorum_cnt = (vg->total_vgdas /2) + 1;
			  close(lvfd);
                      } /* two active pvs */
		      else { /* not enough active pvs */
			  close(lvfd);
			  close(vgfd);
              		  free(vgptr);
			  return(LVM_BELOW_QRMCNT);
		      }
		      break;
		/* 
		* if we have a special case of 2 to 1
		*   we need to be sure that the remaining pv
		*   is active
		*/
		case LVM_CASE2TO1:
		      if(activepvs >= 1) {
 			/*
		  	* increment the necessary fields to show that a
			* vgda has been added to a pv
			*/
	                pvtoget2->pvnum_vgdas ++;
                        vg->total_vgdas ++;
			quorum = (vg->total_vgdas / 2) + 1;
			/* call lvm_delpv() to remove the pv from the kernel */
			status = lvm_delpv(&vgid,fhead->major_num,pvnumber,
				fhead->num_desclps, HD_KREMPV, (short)quorum);
			if(status < LVM_SUCCESS) {
			     close(vgfd);
			     close(lvfd);	
              		     free(vgptr);
			     return(status);
			}

			/* 
			 * call timestamp() to get the current time from
			 * the system clock and store it in the vg header
      			 * and trailer timestamps 
			 */
			status = timestamp(vg,vgptr,fhead);
			if(status < LVM_SUCCESS) {
				close(vgfd);
				close(lvfd);
              		        free(vgptr);
				return(status);
			}
			/*
			* call lvm_cmplvmrec() to make sure the pv that we
	 		* are deleteing's 
			* lvm record shows it as a member of this vg
			*/
			if(noname == FALSE) {
      				lvm_cmplvmrec(&vgid,&match,pvname);
				if(match == TRUE)
			    	   status = lvm_wrtnext(lvfd,vgda_ptr,
						&(trail->timestamp), pvnumber,
						fhead,(short int)1);
				   if(status < LVM_SUCCESS) {
					close(vgfd);
					close(lvfd);
              		        	free(vgptr);
					return(status);
				   }
			}
			/* 
			* call lvm_wrtnext() to write the vgdas to the
			* pv that is left
			*/
			status = lvm_wrtnext(lvfd,vgda_ptr, 
					     &(trail->timestamp), 
					     pvtoget2->pv_num, fhead,
					     (short int)2);
		        if(status < LVM_SUCCESS) {
     			   /* we've been forced off */
			   close(vgfd);
			   close(lvfd);
             		   free(vgptr);
			   return(status);  
			}
			/* 
			* update the curcnt_vgdas field in the file header
			* to the number of vgdas that we just successfully
			* wrote and recalculate and reset the quorum count
			*/
			fhead->quorum_cnt = (vg->total_vgdas /2) + 1;
			close(lvfd);
                 } /* there is an active pv */
		 else { /* no active pvs left */
		     close(lvfd);
 		     close(vgfd);
             	     free(vgptr);
		     return(LVM_BELOW_QRMCNT);
		 }
		 break;
	      			
		/* 
   		* If we have a general case of removeing a pv 
       	       	*      call lvm_cmplvmrec() to be sure the vgid of the
	       	*         volume group matches the vgid on the lvmrec of
	       	*         the pv we're removing
	       	*      if the vgids match
	       	*         try to write the vgda on the pv being removed
               	*         (ignore any errors on this write)
	       	*/
		case LVM_CASEGEN:
		      /* call lvm_delpv() to remove the pv from the kernel */
		      quorum = (vg->total_vgdas / 2) + 1;
		      status = lvm_delpv(&vgid,fhead->major_num,pvnumber,
				fhead->num_desclps,HD_KREMPV,(short)quorum);
                      if(status < LVM_SUCCESS) {
			 close(vgfd);
			 close(lvfd);
             	         free(vgptr);
			 return(status);
		      }
		      if(noname == FALSE) {
	              		lvm_cmplvmrec(&vgid,&match,pvname);
		     	 	if(match == TRUE)
		       	           status = lvm_wrtnext(lvfd,vgda_ptr,
						&(trail->timestamp), pvnumber,
                                     	        fhead,(short int)1);
				   if(status < LVM_SUCCESS) {
					close(vgfd);
					close(lvfd);
             	         		free(vgptr);
					return(status);
				   }
						  
		      }
		      close(lvfd);
		      break;
	} /* end switch */
	/*
	 *  call lvm_diskio() to update the vgdas of the pvs 
	 *  left in the volume group
	 */
	status = lvm_diskio(vgptr,vgfd);
	if(status < LVM_SUCCESS) {
		/* we've been forced off */
		close(vgfd);
            	free(vgptr);
		return(status);
		}
	break;
/*
*  case RETURN:
*	if the pv's state is not removed, break out of the switch
*       Turn off the removed bit and turn on the active one in the pv header
*/
    case LVM_RETURNPV:
	if(!(pvptr->pv_state & LVM_PVREMOVED)) {
		close(lvfd);
                break;
	}
	pvptr->pv_state = (LVM_PVMASK & pvptr->pv_state) | LVM_PVACTIVE;
	twovgdas = FALSE;
	rcsave = 0;
    old_totvgdas = vg->total_vgdas;
    activepvs = 0;	
	/* 
	 * try to read the lvm record on the physical volume that we
	 * are returning. Set an indicator to show if we were successful		 * or not.
	 */
        can_read = TRUE; 
	if(noname == FALSE) {
	 	pvfd = open(pvname,O_RDWR);
		if(pvfd == LVM_UNSUCC)
			can_read = FALSE;
		offset = DBSIZE * PSN_LVM_REC;
		offset = lseek(pvfd,offset,LVM_FROMBEGIN);
		if(offset == LVM_UNSUCC)
			can_read = FALSE;
		status = read(pvfd,(char *)&lvmrec, sizeof(struct lvm_rec));
		if(status != sizeof(struct lvm_rec))
			can_read = FALSE;
	}
	else
		can_read = FALSE;
	
/*	
* 		loop for numpvs in volume group 
*                       increment a counter if the pv is not missing
*                 	if the pv has 2 vgda copies
*                               set an indicator to show that we have the
*					special case
*	           		save its pvnum 
*	 			decrement the pvnum_vgdas field on the pv with 
*               	      		vgda copies so that it now shows one
*   			if the pv has only 1 copy
*                       	save its pvnum 
*/

	for(numpvs=0; numpvs < LVM_MAXPVS; numpvs ++) {
	   if(fhead->pvinfo[numpvs].pvinx != 0) {
 		test_state = TSTSA_PVMISS(&vgsa,numpvs) ? TRUE : FALSE;
		if(test_state == FALSE)
		   activepvs ++;
		pv = (struct pv_header *)((char *)vgptr +
			fhead->pvinfo[numpvs].pvinx);
		if(pv->pvnum_vgdas == LVM_TTLDAS_1PV) {
			if(old_totvgdas == LVM_TTLDAS_2PV) {
		            	twovgdas = TRUE;
			        pvnum2 = pv->pv_num;
			        pv->pvnum_vgdas = LVM_DAPVS_TTL1;
			}
		}
		if(pv->pvnum_vgdas == LVM_DAPVS_TTL1)
			pvnum1 = pv->pv_num;
	   }
   	}
        /* do the needed calculations for the call to chgvgsa() later */
        vgdapsn = PSN_NONRSRVD + VGSA_BLK;
        fstsctr = vgdapsn +  fhead->vgda_len;
        lstsctr = fstsctr + VGSA_BLK - 1;
	partlenblks = LVM_PPLENBLKS(vg->pp_size);
        if(BLK2TRK(fstsctr) != BLK2TRK(lstsctr)) 
               fstsctr = TRK2BLK(BLK2TRK(fstsctr) + 1);
	lsns[FIRST_INDEX] = 0;
	lsns[SEC_INDEX] = fhead->num_desclps * partlenblks *
			         (pvnum2 - 1) + fstsctr; 

/*		
*      if indicator shows special case
*		call lvm_vgdas3to3()
*       else
*		set pvnum_vgdas to 1 for pv being returned
*               increment total_vgdas by 1
*  		call lvm_updvgda()
*      break;
*/
	if(twovgdas == TRUE) {
		/* if we are on the boundary of losing our quorum */
		if(activepvs < 3) {
			/*
			* if we could not open and read the pv to
			* be returned, return an error
			 */
			if(can_read == FALSE) {
				close(vgfd);
				close(lvfd);
            			free(vgptr);
				return(LVM_BELOW_QRMCNT);
			}
		}
		/* 
		 * call timestamp() to get the current time from the 
		 * system clock and store it in the vg header and trailer
		 * timestamps
                 */
		status = timestamp(vg,vgptr,fhead);
		if(status < LVM_SUCCESS) {
			close(vgfd);
			close(lvfd);
            		free(vgptr);
			return(status);
		}
		pvptr->pvnum_vgdas ++;
		quorum = (old_totvgdas + 1)/2 + 1;
		if(noname == FALSE) {
		   status = lvm_returnpv (&vgid,pvname,pvptr,fhead,quorum);
		   if(status < LVM_SUCCESS) {
			close(vgfd);
			close(lvfd);
            		free(vgptr);
			return(status);
		   }
		   else
			rcsave = status;
		}
		else {
		    rcsave = LVM_REMRET_INCOMP;
		    status = lvm_chgqrm (&(changepv->vg_id),fhead->
			       major_num, (short int) quorum);
		    if (status==LVM_FORCEOFF)
		    {
            		free(vgptr);
			return (LVM_FORCEOFF);
		    }
		}
		status = lvm_vgdas3to3(lvfd,vgptr,pvptr->pv_num,pvnum2,pvnum1);
                close(lvfd);
		if(status < LVM_SUCCESS) {
			close(vgfd);
            		free(vgptr);
			return(status);
                }
	        quorum = vg->total_vgdas / 2 + 1;
		pvptr = (struct pv_header *)((char *)vgptr + 
				fhead->pvinfo[pvnum2 - 1].pvinx);
                status = lvm_chgvgsa(&vgid,fhead->major_num,lsns,pvptr->pv_num,
				(short) quorum,HD_KDELVGSA);
	        if(status < LVM_SUCCESS) {
		   close(vgfd);
            	   free(vgptr);
	           return(status);
		}
		if(rcsave != 0) {
		   close(vgfd);
            	   free(vgptr);
		   return(rcsave);
		}
	}
	else {  /* no special case */
		close(lvfd);
		pvptr->pvnum_vgdas++;
		vg->total_vgdas += 1;
		fhead->quorum_cnt = (vg->total_vgdas / 2) + 1;
		if(noname == FALSE) {
		   status = lvm_returnpv (&vgid,pvname,pvptr,fhead,
					  fhead->quorum_cnt);
		   if(status < LVM_SUCCESS) {
			close(vgfd);
			close(lvfd);
            	   	free(vgptr);
			return(status);
		   }
		   else
			rcsave = status;
		}
		else {
		    rcsave = LVM_REMRET_INCOMP;
		    status = lvm_chgqrm (&(changepv->vg_id),fhead->
			       major_num,(short int)fhead->quorum_cnt);
		    if (status==LVM_FORCEOFF)
		    {
            	   	free(vgptr);
			return (LVM_FORCEOFF);
		    }
		}
		/* call lvm_diskio() to commit changes to hardfile */
		status = lvm_diskio(vgptr,vgfd);
                if(status < LVM_SUCCESS) {
		  /* we've been forced off */
                   close(vgfd);
            	   free(vgptr);
                   return(status);
		}
		if(rcsave != 0) {
		    close(vgfd);
            	    free(vgptr);
		    return(rcsave);
		}
	}
	break;
	 	  
     default : 
	lvm_errors("changepv","      ",LVM_INVALID_PARAM);
        close(vgfd);
        close(lvfd);
	free(vgptr);
       	return(LVM_INVALID_PARAM);
   } /* end switch rem_ret */
  } /* end if(rem_ret != 0) */

  /* close the volume group mapped file */
   close(vgfd);
   free(vgptr);

   return(LVM_SUCCESS);
}
/*****************************************************************************/
/*
 *                                                                     
 * NAME: lvm_returnpv
 *                                                                     
 * FUNCTION: does necessary processing to return the pv to the kernel     
 *                                                                     
 * NOTES:                                                              
 *                                                                     
 *   INPUT:                                                            
 *     	vg_id
 * 	pvname
 *	pv_ptr
 *	mapf_ptr
 *      quorum_cnt
 *
 *   OUTPUT:                                                           
 *	The physical volume will be returned to the kernel
 *                                                                     
 * RETURN VALUE DESCRIPTION:                                           
 *   Zero is returned if successful.                                   
 *   The positive return code of LVM_REMRET_INCOMP can be returned
 *   The negative return code of LVM_FORCEOFF can also be returned
 *                                                                     
 */


int
lvm_returnpv (
struct unique_id * vg_id,		/* pointer to volume group id */
caddr_t pvname,				/* name of physical volume */
struct pv_header *pv_ptr,		/* pointer to PV header in vg file */
struct fheader *mapf_ptr,               /* pointer to vg file header */
short int quorum_cnt)                   /* # VGDAs needed for quorum */

{
   int retcode;
   int pv_fd;  				/* file descriptor of PV */
   long partlen_blks;			/* partition length in blocks */
   struct stat stat_buf;		/* structure for status info */
   IPL_REC ipl_rec;			/* ipl record structure */
   struct lvm_rec lvm_rec;		/* structure for lvm record info */
   char devname [LVM_EXTNAME];		/* device name */
   short int pv_idx;			/* index to physical volume */   
   short int da_idx;			/* loop counter */
   daddr_t salsn [LVM_PVMAXVGDAS];	/* status area logical sector numbers */
   struct vg_header *vghdr_ptr;         /* pointer to volume group header */
   struct fheader *fhead;               /* pointer to mapped file header */

   fhead = (struct fheader *) mapf_ptr;
   vghdr_ptr = (struct vg_header *) ((char *)mapf_ptr + mapf_ptr->vginx);
   quorum_cnt = vghdr_ptr -> total_vgdas / 2 + 1;

   /* build the path name for the physical volume device */
   retcode = status_chk (NULL, pvname, NOCHECK, devname);
   if (retcode < LVM_SUCCESS) {
      return (LVM_REMRET_INCOMP);
   }

   /* open the specified physical volume */
   pv_fd = open (devname, O_RDWR);
   if (pv_fd == LVM_UNSUCC) {
      return (LVM_REMRET_INCOMP);
   }

   /* read the IPL record for the physical volume */
   retcode = lvm_rdiplrec (pvname, pv_fd, &ipl_rec);
   if (retcode < LVM_SUCCESS) {
      close (pv_fd);
      return (LVM_REMRET_INCOMP);
   }


   if (ipl_rec.pv_id.word1 != pv_ptr -> pv_id.word1 ||
      ipl_rec.pv_id.word2 != pv_ptr -> pv_id.word2) {
         close (pv_fd);
         return (LVM_REMRET_INCOMP);
   }

   /* call routine to read the LVM record from the physical volume */
   retcode = lvm_rdlvmrec (pv_fd, &lvm_rec);
   if (retcode < LVM_SUCCESS) {
      close (pv_fd);
      return (LVM_REMRET_INCOMP);
   }

   /* if this physical volume is not a member of the specified volume group */
   if (lvm_rec.lvm_id != LVM_LVMID || lvm_rec.vg_id.word1 != vg_id->word1
	|| lvm_rec.vg_id.word2 != vg_id->word2) {
          close (pv_fd);
          return (LVM_REMRET_INCOMP);
   }

   /* fill in information for this PV in the mapped file header */
   pv_idx = pv_ptr -> pv_num - 1;
   /* find the partition length for this volume group in number of blocks */
   partlen_blks = LVM_PPLENBLKS (vghdr_ptr -> pp_size);

   /* calculate logical sector numbers for the VGDA and VGSA copies */
   for (da_idx = 0; da_idx < LVM_PVMAXVGDAS; da_idx++) {
      mapf_ptr -> pvinfo[pv_idx].da[da_idx].dalsn = mapf_ptr ->
	   num_desclps * partlen_blks * pv_idx + lvm_rec.vgda_psn[da_idx];
      if (da_idx < pv_ptr -> pvnum_vgdas) {
	  salsn[da_idx] = mapf_ptr -> num_desclps * partlen_blks *
			    pv_idx + lvm_rec.vgsa_psn[da_idx];
      }
      else {
	  salsn[da_idx] = 0;
      }
   }

   /* get device identification for this PV */
   retcode = fstat (pv_fd, &stat_buf);
   if (retcode < LVM_SUCCESS) {
      close (pv_fd);
      return (LVM_REMRET_INCOMP);
   }
   mapf_ptr -> pvinfo[pv_idx].device = stat_buf.st_rdev;
   /* add the returned PV into the kernel */
   retcode = lvm_addpv (partlen_blks, mapf_ptr -> num_desclps, mapf_ptr ->
	      pvinfo[pv_idx].device, pv_fd, pv_ptr -> pv_num, mapf_ptr ->
	      major_num, vg_id, lvm_rec.reloc_psn, lvm_rec.reloc_len,
	      pv_ptr -> psn_part1, salsn, quorum_cnt);
   if (retcode < LVM_SUCCESS) {
      close (pv_fd);
      if (retcode == LVM_FORCEOFF)
	  return (LVM_FORCEOFF);
      else {
	  retcode = lvm_chgqrm (vg_id,fhead->major_num,quorum_cnt);
	  if (retcode == LVM_FORCEOFF)
	      return (LVM_FORCEOFF);
	  return (LVM_REMRET_INCOMP);
      }
   }
   close (pv_fd);
   return (LVM_SUCCESS);
}


/*
 * NAME: get_odm_pvname     
 *
 * DESCRIPTION: Gets a specified device name given the device identifier
 *	        Gets the name from the CuAt class.
 *		
 * INPUT: The physical volume identifier.
 *
 * OUTPUT: The physical volume name in pvname.
 *
 * RETURN VALUE DESCRIPTION: NONE
 *		
 */
void
get_odm_pvname(
char *pvid,
char *pvname)
{
struct CuAt *cuatp;
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[256];		/* criteria for search of class in get list */

        odm_initialize();
    	odm_set_path(CFGPATH);

        bzero(crit,256);
	sprintf(crit,"attribute='%s' and value='%s'", "pvid",pvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if (((int)cuatp == -1) || (cuatp == NULL))
		*pvname=NULL;
	else
		strcpy(pvname, cuatp->name);
	odm_free_list(cuatp, &stat_info);
	odm_terminate();
}
