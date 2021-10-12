static char sccsid[] = "@(#)73	1.17  src/bos/usr/ccs/lib/liblvm/migratepp.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:52:20";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_migratepp, resync
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


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern void *malloc();


int resync(
 struct unique_id *vg_id,	/* volume group id */
 short 		  minor_num,	/* minor number of logical volume */
 int              vgfd,		/* volume group file descriptor */
 long             lpnum, 	/* logical partition number */
 char             *vgfptr);	/* pointer to vg mapped file */

/*
 * NAME: lvm_migratepp
 *                                                                    
 * FUNCTION: Moves one physical partition from its present physical volume
 *           to a new physical partition on the specified physical volume.
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *  DATA STRUCTURES:
 *     input
 *       migratepp 
 *          vg_id
 *          oldpp_num
 *          newpp_num
 *          oldpv_id
 *          newpv_id
 * 
 *     output
 *        The data structures in the volume group mapped file and on the
 *        hard file will be updated to show the migration.
 *
 * RETURN VALUE DESCRIPTION: LVM_SUCCESS if successfull 
 *                           if unsuccessful, one of the following negative
 *                           return codes :
 *			          LVM_INVALID_PARAM
 *			          LVM_OFFLINE
 *			          LVM_PROGERR
 *			          LVM_MAPFOPN
 *			          LVM_MAPFSHMAT
 *			          LVM_INVALID_MIN_NUM
 *			          LVM_ALLOCERR
 *			          LVM_LVOPEN
 *			          LVM_PPNUM_INVAL
 *			          LVM_LPNUM_INVAL
 *			          LVM_PVSTATE_INVAL
 *			          LVM_NOALLOCLP
 *			          LVM_NOTCHARDEV
 *		                  LVM_INV_DEVENT
 *                                LVM_INVLPRED
 *                                LVM_DALVOPN
 */  

int
lvm_migratepp (
struct migratepp *migratepp) /* pointer to the migratepp structure */

 
{
    char *vgfptr;          /* pointer to volume group mapped file */
    int vgfd;              /* volume group file descriptor */
    int rc,brc;            /* hold return codes */
    struct pv_header *pv1,*pv2;
                           /* pointers to the physical volume headers */
    struct pp_entries *pp1,*pp2;
                           /* pointers to the physical partition entries*/
    int mode;              /* mode to open the volume group mapped file with */
    struct lv_entries *lv; /* pointer to the logical volume entry for the */
                           /* logical volume that the pp is part of */
    struct namelist *nptr; /* pointer to the namelist area */
    struct fheader *fhead; /* pointer to vg mapped file file header */
    int numcopies;         /* # of copies of the logical partition that old */
                           /* pp is part of */
    int lvfd;              /* logical volume file descriptor */
    struct lv_id lv_id;    /* lv_id sent to extendlv and reducelv */
    struct pp eparts,rparts;
			   /* pp structures for ext and red */
    struct ext_redlv extendlv,reducelv;
                           /* structures for reduce and extend */
    short minor_num;       /* minor number of logical volume */
    struct unique_id vg_id;/* volume group id */
    long lpnum;            /* logical partition number */
    int  resync_failed;    /* indicates that the resync failed */
                                                               

   resync_failed = FALSE;
   /* if the migratepp pointer is null, return an error */

   if(migratepp == NULL)
   {   lvm_errors("migratepp","        ",LVM_INVALID_PARAM);
       return(LVM_INVALID_PARAM);
   }
 
   /* Call lvm_getvgdef() to open the mapped file */

   mode = O_RDWR | O_NSHARE;
   rc = lvm_getvgdef(&(migratepp->vg_id),mode,&vgfd,&vgfptr);
   if(rc < LVM_SUCCESS)
   {
      lvm_errors("openmap","migratepp",rc);
      free(vgfptr);
      return(rc);
   }

   /*
    *  Call get_pvandpp() to get a pointer to the two pvs
    */
   rc = get_pvandpp(&pv1,&pp1,NULL,vgfptr,&(migratepp->oldpv_id));
   if(rc < LVM_SUCCESS)
   {
      close(vgfd);
      free(vgfptr);
      return(rc);
   }
   rc = get_pvandpp(&pv2,&pp2,NULL,vgfptr,&(migratepp->newpv_id));
   if(rc < LVM_SUCCESS)
   {
      close(vgfd);
      free(vgfptr);
      return(rc);
   }

   /*
    *  check the range of the pp numbers
    */
   if(migratepp->oldpp_num <= 0 || migratepp->oldpp_num > pv1->pp_count)
   {
       lvm_errors("migratepp","           ",LVM_INVALID_PARAM);
       close(vgfd);
       free(vgfptr);
       return(LVM_INVALID_PARAM);
   }
   if(migratepp->newpp_num <= 0 || migratepp->newpp_num > pv2->pp_count)
   {
       lvm_errors("migratepp","           ",LVM_INVALID_PARAM);
       close(vgfd);
       free(vgfptr);
       return(LVM_INVALID_PARAM);
   }

   /*
    * check the state of the physical volume that is being migrated to,
    * to be sure that it is not PVNOALLOC
    */
   if(pv2->pv_state & LVM_PVNOALLOC) {
      close(vgfd);
      free(vgfptr);
      return(LVM_INVALID_PARAM);
   }

   /*
    *   set each pp pointer to the correct physical partition
    */

   pp1 += migratepp->oldpp_num - 1;
   pp2 += migratepp->newpp_num - 1;

   /*
    *  If the old pp does not exist( its state is free) or the
    *  new pp exists( its state is allocated) then return an error
    */

   if(!(pp1->pp_state & LVM_PPALLOC))
   {
       lvm_errors("migratepp","     ",LVM_INVALID_PARAM);
       close(vgfd);
       free(vgfptr);
       return(LVM_INVALID_PARAM);
   }
   if(pp2->pp_state)   
   {
       lvm_errors("migratepp","     ",LVM_INVALID_PARAM);
       close(vgfd);
       free(vgfptr);
       return(LVM_INVALID_PARAM);
   }
  

   /*
    *  Look at the first and second alternate volumes(parts) in order
    *  to record the number of copies for the logical partition the pp
    *  belongs to
    */

   numcopies = 1;
   if(pp1->fst_alt_vol != 0)
   {
       numcopies ++;
       if(pp1->snd_alt_vol != 0)
           numcopies ++;
   }

   /*
    *   save off the minor number of the logical volume and the logical
    *    partition number that the physical partition being migrated
    *   correlates to
    */

   minor_num = pp1->lv_index;
   lpnum = pp1->lp_num;

   /* set up the extendlv and reducelv structures */
   lv_id.vg_id.word1 = migratepp->vg_id.word1;
   lv_id.vg_id.word2 = migratepp->vg_id.word2;
   lv_id.minor_num = minor_num;

   /* assign the struct pp with the information 
    *  needed to add the new partition to the logical volume
    */

   eparts.pv_id.word1 = migratepp->newpv_id.word1;
   eparts.pv_id.word2 = migratepp->newpv_id.word2;
   eparts.lp_num = lpnum;
   eparts.pp_num = migratepp->newpp_num;
   extendlv.size = LVM_FIRST;
   extendlv.parts = &eparts;

   /*
    *  assign the struct pp with the information
    *  on the physical partition to be deallocated from the logical volume
    */

   rparts.pv_id.word1 = migratepp->oldpv_id.word1;
   rparts.pv_id.word2 = migratepp->oldpv_id.word2;
   rparts.lp_num = lpnum;
   rparts.pp_num = migratepp->oldpp_num;
   reducelv.size = LVM_FIRST;
   reducelv.parts = &rparts;

   /*
    *  based on the number of copies that exist for the logical partition
    *  call the extendlv and reducelv routines
    */

   switch(numcopies)
   {  case LVM_FIRST:
      case LVM_SEC: /* for one or two copies, we extend first, then reduce */
          rc = rdex_proc(&lv_id,&extendlv,vgfptr,vgfd,minor_num,LVM_EXTEND);
          if(rc < LVM_SUCCESS) {
	      close(vgfd);
       	      free(vgfptr);
              return(rc);
          }

          /*
           *   call resync() to resync the logical partition that the physical
           *   partition being migrated belongs to
           */

          rc = resync(&(migratepp->vg_id),minor_num,vgfd,lpnum,vgfptr);
          if(rc < LVM_SUCCESS)
          {   lvm_errors("resync","migratepp",rc);
              resync_failed = TRUE;
          }

          /*
           * call reducelv() to reduce the physical partition that was migrated
           */   
          rc = lvm_reducelv(&lv_id,&reducelv);
          if(rc < LVM_SUCCESS) {
	     /*
	      * if the resync failed and the reduce failed because
	      * we wouldn't have any good copies left for the lp,
	      * we try to backout the pp we extended. No matter if
	      * the backout succeeds or fails, we return
	      * MIGRATE_FAIL.
	      */ 
              
	     if(resync_failed == TRUE) {
	        if(rc == LVM_INVLPRED) {
	           brc = lvm_reducelv(&lv_id,&extendlv);
       	      	   free(vgfptr);
	           return(LVM_MIGRATE_FAIL);
	        }
	        else 
	           /* 
	            * if the resync failed and the reduce fails
	            * with some other error, we return a positive
	            * return code beause the migrate did succeed,
	            * only the old pp still remains and the lp
	            * was not resynced.
	            */
       	      	   free(vgfptr);
	           return(LVM_RESYNC_FAILED);
	     }
          }
	  else
	     if(resync_failed == TRUE)
	     {
       	      	free(vgfptr);
		return(LVM_RESYNC_FAILED);
	     }
          break;

       case LVM_THIRD: /* if three copies already exist, we need to
                        * reduce first and then extend to prevent
                        * four copies existing for a logical partition
                        * at one time
                        */

            rc = rdex_proc(&lv_id,&reducelv,vgfptr,vgfd,minor_num,LVM_REDUCE);
            if(rc < LVM_SUCCESS) {
		 close(vgfd);
       	      	 free(vgfptr);
                 return(rc);
	    }
            rc = rdex_proc(&lv_id,&extendlv,vgfptr,vgfd,minor_num,LVM_EXTEND);
            if(rc < LVM_SUCCESS) {
	        /* 
		 * if the extend portion fails, we try to backout
		 * what was reduced. If we can't back out, we
		 * return the LOSTPP to the user since we could
		 * not recover the pp we reduced.
		 */
		brc = rdex_proc(&lv_id,&reducelv,vgfptr,vgfd,
			minor_num,LVM_EXTEND);
	        if(brc < LVM_SUCCESS) {
		     close(vgfd);
       	      	     free(vgfptr);
		     return(LVM_LOSTPP);
		}
		else
		{
       	      	   free(vgfptr);
		   return(LVM_MIGRATE_FAIL);
		}
            }

            /*
             * call resync() to resync the logical partitin that the physical
             * partition belongs to
             */

             rc = resync(&(migratepp->vg_id),minor_num,vgfd,lpnum,vgfptr);
             if(rc < LVM_SUCCESS)
             {   lvm_errors("resync","migratepp",rc);
       	      	 free(vgfptr);
                 return(LVM_RESYNC_FAILED);
             }
             break;
   } /* end switch */

   free(vgfptr);

   return(LVM_SUCCESS);
}

/************************************************************************/
/*
 * NAME: resync
 *                                                                    
 * FUNCTION: Reopens the mapped file and gets the necessary information   
 *           to be passed to synclp, calls synclp to sync the migrated
 *           partition.
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *  DATA STRUCTURES:
 *     input
 *       vg_id
 *       minor_num 
 *       lpnum
 *       vgfptr
 * 
 *     output
 *        The data structures in the volume group mapped file and on the
 *        hard file will be updated to show that a resync was completed.
 *
 * RETURN VALUE DESCRIPTION: LVM_SUCCESS if successful and one of the following
 *                           if unsuccessful :
 *				LVM_MAPFOPN
 *				LVM_MAPFSHMAT
 *				LVM_INVALID_PARAM
 *				LVM_NOTCHARDEV
 *				LVM_NOTCHARDEV
 *                              LVM_ALLOCERR
 */  

int resync(
struct unique_id *vg_id,
                /* volume group id */
short minor_num,/* minor number of the logical volume that */
                /* the partition to be migrated belongs to */
int vgfd,	/* volume group file descriptor */
long lpnum,     /* logical partition number of the logical partition the */
                /* migrated partition belongs to */
char *vgfptr)   /* pointer to volume group mapped file */

{

    int rc;                /* return code */
    int mode;              /* mode to open the volume group mapped file with */
    struct lv_entries *lv; /* pointer to the logical volume entry for the */
                           /* logical volume that the pp is part of */
    struct namelist *nptr; /* pointer to the namelist area */
    struct fheader *fhead; /* pointer to vg mapped file file header */
    char rawname[LVM_EXTNAME];
                           /* temp variable for raw lvname */ 
    int lvfd;              /* logical volume file descriptor */

/* initialize a return code to a successful value */
  
   rc = LVM_SUCCESS;
/*
*  Call get_ptrs( ) to get a pointer to the logical volume that this pp
*  belongs to, and to the namelist area and to the file header;
*/

   rc = get_ptrs(vgfptr,&fhead,NULL,&lv,NULL,NULL,&nptr);
   if (rc < LVM_SUCCESS)
   {   close(vgfd);
       return(rc);
   }

/* 
* increment the logical volume pointer to the correct logical volume
*/

   lv += minor_num - 1;

/*
*  call status_chk() to be sure the device is a raw one and to check 
*  the major number for validity
*/
  
   rc = status_chk(vgfptr,nptr->name[lv->lvname],CHECK_MAJ,rawname);
   if(rc < LVM_SUCCESS)
   {   close(vgfd);
       return(rc);
   }
   lvfd = open(rawname,O_RDWR);
   if(lvfd < LVM_SUCCESS)
   {   lvm_errors("open lv","migratepp",lvfd);
       close(vgfd);
       return(lvfd);
   }

/*
*  Call synclp() to sync the logical partition to include the
*  newly migrated physical partition
*/
 
   rc = synclp(lvfd,lv,vg_id,vgfptr,vgfd, minor_num,lpnum,FALSE);
   if(rc < LVM_UNSUCC)
   {   close(vgfd);
       close(lvfd);
       if(rc == LVM_UNSUCC)
            rc = LVM_MIGRATE_FAIL;
       return(rc);
   }

   close(lvfd);

   return(rc);
}
/************************************************************************/
