static char sccsid[] = "@(#)63	1.15.1.3  src/bos/usr/ccs/lib/liblvm/synclp.c, liblvm, bos411, 9428A410j 4/14/94 09:34:13";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: synclp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/dasd.h>
#include <sys/lvdd.h>
#include <fcntl.h>
#include <liblvm.h>

 /*****************************************************************************
 *
 * NAME: synclp
 *                                                                    
 * FUNCTION: This routine synchronizes all physical partitions for a logical
 *           partition.  
 *                                                                    
 *                                                                   
 * NOTES:
 *
 *     input
 *       lvfd
 *       lv
 *       vg_id
 *       vgptr
 *       vgfd
 *       minor_num
 *       lpnum
 * 
 *     output
 *        The pp states will be changed to indicate that they are not stale,
 *        and the data on all copies of the logical partition will be in sync.
 *
 * RETURN VALUES: 
 *        LVM_SUCCESS if successful and one of the following if unsuccessful:
 *             LVM_INVALID_PARAM
 *             LVM_MAPFRDWR
 *             LVM_ALLOCERR
 *             LVM_MAPFOPN
 *             LVM_MAPFSHMAT
 *            
 */  

int synclp(

int lvfd,                 /* logical volume file descriptor */
struct lv_entries *lv,    /* pointer to the logical volume entry */
struct unique_id *vg_id,  /* volume group id */
char *vgptr,              /* pointer to the volume group mapped file */
int vgfd,                 /* volume group mapped file's file descriptor */
short minor_num,          /* minor number of the logical volume */
long lpnum,               /* logical partition number to sync */
int force)		  /* indicator that even NON-STALE lps are resynced */

{
   void *malloc();
   int  rc,rc1;            /* return codes */
   struct logview *lp;     /* pointer to logical view of logical volume */
   long cnt[LVM_NUMCOPIES];/* array of number of pp's per copy of a lv */
   int synced;             /* the number of good pps for a lp */
   int num_copies;         /* the number of total copies(pps) for a lp */
   struct fheader *fhead;  /* pointer to file header of vg mapped file */
   struct vg_header *vg;   /* pointer to volume group header in mapped file */ 
   struct pp_entries *pp;  /* pointer to a physical partition entry */
   struct pv_header *pv;   /* pointer to a physical volume */
   int size;               /* # of bytes used as input to the readx call */ 
   char *buf;              /* pointer to the buffer used in readx call */
   int pvnum;              /* physical volume number */
   int ext;                /* indicates which copy needs resyncing */
                           /* in the readx call */
   char stale;             /* indicates if a physical partition is stale */
   char ppcnt;		   /* counter of a for loop */
   int  ltgs;              /* # logical track groups in the logical partition */
   int ltgcnt;             /* counter of logical tracks in for loop */
   offset_t skdst;        /* distance to seek to in the logical volume */
   int ppsize;             /* physical partition size for the volume group */
   char *mapptr;           /* pointer to vg mapped file */
   int  mapfd;             /* vg mapped file file descriptor */
   int  lpcntr,cpcntr;     /* for loop counters */
   int  cpcnt,counter;     /* more counters */
   char chgst;             /* indicates whether the pv or lv state should be */
                           /* changed */
   struct lpview lpinfo;   /* structure to hold logical view of lp to resync */

   /* 
    *   Set up the lpview structure and call the ioctl to build the logical
    *   view of the logical partition being resynced.
    */

    bzero((char *)(&lpinfo),sizeof(struct lpview));
    lpinfo.lpnum = lpnum;
    rc = ioctl(lvfd,LP_LVIEW,(char *)(&lpinfo));
    if(rc != 0) {
        close(vgfd);
        return(rc);
    } 

   /*
    * call get_ptrs() to get pointers to the file header and volume group header
    */

   rc = get_ptrs(vgptr,&fhead,&vg,NULL,NULL,NULL,NULL);
   if(rc < LVM_SUCCESS)
   {  close(vgfd);
      return(rc);
   }

   /* save off the ppsize to use later */

   ppsize = (1 << vg->pp_size);


   /* initialize the needed counters */

   synced = 0;
   num_copies = 0;

   /* 
    *  for the maximum number of copies per logical partition allowed,
    *      check the logical view for the logical partition being resynced
    *      if there is a valid entry for the copy you're on, increment a counter
    *      for the number of copies this logical partition has, and if the copy 
    *      is not stale, increment a counter for the number of already 
    *      synced copies
    */

   for(cpcnt = 0; cpcnt < LVM_NUMCOPIES; cpcnt ++) {
       if(lpinfo.copies[cpcnt].pvnum != 0) {
           if(!(lpinfo.copies[cpcnt].ppstate))
                 synced ++;
           num_copies ++;
       }
   }

   /*
    *   if there is only one copy of this logical partition
    *     return(LVM_SUCCESS)
    */

   if(num_copies == LVM_NOMIRROR)
   {   close(vgfd);
       return(LVM_SUCCESS);
   }
 
   /*
    *   If the number of good copies equals the number of copies for the lp
    *   and the force flag is FALSE, indicating we only want to sync stale lps,
    *       return(LVM_SUCCESS)
    */

   close(vgfd);

   if(synced == num_copies && force == FALSE) {
       return(LVM_SUCCESS);
   }



   /* allocate space for a buffer for the readx call */
   buf = (char *)malloc( BYTEPTRK );
   if(buf == NULL)
       return(LVM_ALLOCERR);


    /* calculate the number of logical track groups for the logical partition
     *  which is: (1 << physical partition size) / (BYTEPTRK)
     */
    ltgs = ppsize / (BYTEPTRK);

    /* compute the distance into the logical volume to seek, and seed to it */

    skdst = (lpnum - 1) * (offset_t)ppsize;
    llseek(lvfd,skdst,LVM_FROMBEGIN);

    /*
     * for each logical track group in the logical partition 
     *  if the force flag is on, we need to turn on the bit to tell the
     *  device driver to sync ALL partitions in addition to the RESYNC_OP bit
     *  if the force flag is off, set only the RESYNC_OP bit.
     *  call readx() to perform the resync.        
     */
    for(ltgcnt=0; ltgcnt < ltgs; ltgcnt++) {  
	   if(force == TRUE) 
		ext = MWC_RCV_OP;
      	   else
	      	ext = RESYNC_OP;
	   rc = readx(lvfd,buf,BYTEPTRK,ext);
           if(rc < LVM_SUCCESS)
           {   free(buf);
               return(rc);
           }
    } /* end for */


   /* 
    *   Set up the lpview structure and call the ioctl to build the logical
    *   view of the logical partition being resynced so that we can check
    *   the states of the physical partitions that were resynced with this
    *   logical partition
    */

    bzero((char *)(&lpinfo),sizeof(struct lpview));
    lpinfo.lpnum = lpnum;
    rc = ioctl(lvfd,LP_LVIEW,(char *)(&lpinfo));
    if(rc != 0)
        return(rc);
      
    /* initialize a counter for the number of partitions that are still stale */
    stale = 0;

    /* for the number of copies of the logical partition, 
     *  go through the logical partition and check to see if all pp states
     *  are NOT stale
     */

    for(counter = 0; counter < lv->mirror; counter ++) {
        /*
         * if the physical partition is still stale after the resync, 
         * increment a counter to record the remaining stale copies of the
         * logical partition
         */
         if(lpinfo.copies[counter].ppstate & LVM_PPSTALE)
                   stale ++;
    }

    /*
     * if there were still physical partitions within the logical partition
     * that did not get resynced(are still stale) then determine the appropriate
     *  error to send back and return it
     */

    free( buf );
    if(stale != 0)
       return(LVM_NOTSYNCED);

    return(LVM_SUCCESS);   
}
/*****************************************************************************/
