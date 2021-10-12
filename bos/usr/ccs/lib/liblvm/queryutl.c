static char sccsid[] = "@(#)67	1.19  src/bos/usr/ccs/lib/liblvm/queryutl.c, liblvm, bos41B, 412_41B_sync 1/5/95 11:13:58";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - queryutl.c
 *
 * FUNCTIONS: lvm_chklvclos,
 *            lvm_getpvda,
 *	      lvm_gettsinfo
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


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <fcntl.h>;

#include <errno.h>;

#include <sys/ioctl.h>;

#include <sys/devinfo.h>;

#include <sys/time.h>;

#include <sys/bbdir.h>;

#include <sys/buf.h>;

#include <sys/shm.h>;

#include <sys/sysconfig.h>;

#include <sys/types.h>;

#include <sys/param.h>;

#include <sys/dasd.h>;
  /* include file for LVDD data structures */

#include <sys/hd_config.h>;
  /* include file for LVDD configuration data structures */

#include <sys/configrec.h>;

#include <lvmrec.h>;

#include <liblvm.h>;
  /* include file for LVM subroutine structures */

#include <sys/hd_psn.h>;




extern void * malloc();




/**********************************************************************
 *                                                                     *
 * NAME:  lvm_chklvclos                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine determines whether a logical volume is closed        *
 *   or not.                                                           *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *      lv_id                                                          *
 *      major_num                                                      *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS - Successful return (logical volume is closed).     *
 *     LVM_LVOPEN - The logical volume is open.                        *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   bzero                                                             *
 *   lvm_config                                                        *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chklvclos (


struct lv_id * lv_id,
  /* logical volume id */

long major_num)
  /* major number of volume group */


{ /* BEGIN lvm_chklvclos */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct ddi_info cfgdata;
  /* input to kernel routine */

int rc;
  /* return code */

bzero ((char *) (&cfgdata.parmlist.kchklv), sizeof (struct kchklv));
  /* zero out the input data structure for the kernel */

cfgdata.parmlist.kchklv.vg_id.word1 = lv_id->vg_id.word1;
cfgdata.parmlist.kchklv.vg_id.word2 = lv_id->vg_id.word2;
cfgdata.parmlist.kchklv.lv_minor = lv_id->minor_num;
  /* store logical volume id for input to kernel routine */

rc = lvm_config (NULL, major_num, HD_KCHKLV, &cfgdata);
  /* call routine which will setup for and call the device driver's
     configuration routine in the kernel */

if (rc < LVM_SUCCESS)
  /* if an error occurred */

    return(rc);
      /* return with error code */

if (cfgdata.parmlist.kchklv.open == TRUE)
  /* if the logical volume was found to be open */

    return(LVM_LVOPEN);
      /* return error code for logical volume open */

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_chklvclos */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_getpvda                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine allocates memory for a mapped file header structure  *
 *   and a copy of the volume group descriptor area.  It then reads    *
 *   in the volume group descriptor area from the specified physical   *
 *   volume and stores offset information into the mapped file header  *
 *   structure.  The pointer to this memory location, which has been   *
 *   given the same structure as the mapped file, is then returned     *
 *   to the calling routine so that it can obtain query information    *
 *   from the specific physical volume in the same manner as it does   *
 *   from the mapped file of a varied-on volume group.                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_name                                                         *
 *     map_ptr                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *map_ptr                                                        *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *   lvm_mapoff                                                        *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_getpvda (


char * pv_name,
  /* a pointer to the name of the physical volume to be added to the
     volume group */

caddr_t * map_ptr,
  /* a pointer to where the pointer to the memory area containing the
     mapped file information will be stored */

int   rebuild)
  /* TRUE if getpvda is called from the routine that rebuilds the vg file */



{ /* BEGIN lvm_getpvda */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

int pv_fd;
  /* the file descriptor for the physical volume device */

int size;
  /* variable to hold a size calculation */

char * vgda_ptr;
  /* a pointer to the beginning of the volume group descriptor area */

char devname [LVM_EXTNAME];
  /* the full path name of the physical disk device */

struct lvm_rec lvm_rec;
  /* the block into which the LVM information record will be read */

char bbdir_buf [LEN_BB_DIR * DBSIZE];
  /* buffer into which the bad block directory will be read */

struct bb_hdr *bb_hdr;
  /* a pointer to the header entry for the bad block directory */

struct bb_entry *bb_entry;
  /* a pointer to an entry in the bad block directory */

short int bb_index;
  /* index variable used for looping on bad block entries */
int badprim, badsec;	
  /* indicators for bad blocks in the VGDA copies on this pv */
int copy;
  /* holds the number of the newest copy of the VGDA to return */
daddr_t psn[LVM_PVMAXVGDAS];
  /* holds the physical sector numbers of the VGDAS on the pv */

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

badprim = FALSE;
badsec = FALSE;
*map_ptr = NULL;
  /* initialize the map file pointer to null */

retcode = status_chk (NULL, pv_name, NOCHECK, devname);
  /* build the entire raw path name for the physical volume device from
     the name passed in */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

    {

    return (retcode);
      /* return with error code */

    }

pv_fd = open (devname, O_RDONLY);
  /* open the specified disk for I/O */

if (pv_fd == LVM_UNSUCC)
  /* if an error occurred */

    {

    return (LVM_PVOPNERR);
      /* return with error code to indicate that the physical volume
	 could not be opened */

    }

retcode = lvm_rdlvmrec (pv_fd, &lvm_rec);
  /* call routine to read the LVM record from the physical volume */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

    {

    close(pv_fd);  
      /* close the physical volume */

    return (retcode);
      /* return with unsuccessful return code */

    }

if (lvm_rec.lvm_id != LVM_LVMID)
  /* if this physical volume does not belong to a volume group */

    {

    close(pv_fd);  
      /* close the physical volume */

    return (LVM_NOTVGMEM);
      /* return with error code for PV is not a volume group member */

    }

if(lvm_rec.version != LVM_VERSION_1 &&
  (lvm_rec.version == 0 || lvm_rec.version > LVM_MAX_VERSION))

  /* if the volume group descriptor/status areas on this disk have the
     incompatible format */
	return(LVM_BADVERSION);
  /* return error to indicate this volume group has an incompatible
     format for its VGDA/VGSA */

size = lvm_rec.vgda_len * DBSIZE + sizeof (struct fheader);
  /* calculate size needed for volume group descriptor area plus the
     mapped file header structure */

*map_ptr = (char *) malloc (size);
  /* allocate enough space to store mapped file header information and
     to read in the volume group descriptor area */

if (*map_ptr == NULL)
  /* if an error occurred */

    {

    close(pv_fd);  
      /* close the physical volume */

    return (LVM_ALLOCERR);
      /* return error code for memory allocation error */

    }

bzero ((char *) (*map_ptr), size);

vgda_ptr = (char *) ((caddr_t) (*map_ptr) + sizeof (struct fheader));
  /* calculate beginning pointer for volume group descriptor area to
     be placed after the mapped file header area */


retcode = lvm_rdbbdir (pv_fd, bbdir_buf, LVM_BBRDONLY);
  /* call routine to read in the bad block directory */

if (retcode < LVM_SUCCESS)
  /* if an error occurred */

    {

    free (*map_ptr);
      /* free the space allocated for the fake mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (retcode);
      /* return with unsuccessful return code */

    }

bb_hdr = (struct bb_hdr *) bbdir_buf;
  /* set pointer to the first entry in the bad block header entry */

bb_entry = (struct bb_entry *)((caddr_t)bbdir_buf + sizeof(struct bb_hdr));
  /* set a pointer to the first entry in the bad block directory */

for (bb_index = FIRST_INDEX; bb_index < bb_hdr->num_entries; bb_index ++)
  /* search the bad block directory for any bad blocks that might
     be in the volume group descriptor area on the hard file */

  {

  if ((daddr_t) bb_entry->bb_lsn >= lvm_rec.vgda_psn [LVM_PRMRY]  &&
	((daddr_t) bb_entry->bb_lsn < (lvm_rec.vgda_len +
				       lvm_rec.vgda_psn [LVM_PRMRY])))
    /* if a bad block is located in the volume group descriptor area
       set an indicator for use later */

    		badprim = TRUE;

  if ((daddr_t) bb_entry->bb_lsn >= lvm_rec.vgda_psn [LVM_SCNDRY]  &&
	((daddr_t) bb_entry->bb_lsn < (lvm_rec.vgda_len +
				       lvm_rec.vgda_psn [LVM_SCNDRY])))
    /* if a bad block is located in the volume group descriptor area
       set an indicator for use later */

    		badsec = TRUE;
  bb_entry ++;
    /* increment the pointer to the next bad block entry */

  } /* search bad block directory */

/* if both copies have bad blocks, return an error */
if(badprim && badsec) {
	free(*map_ptr);
	close(pv_fd);
	return(LVM_VGDA_BB);
}
/* 
*if the primary copy had a bad block and we were not called
* by the routine that re-builds the vg file, set the local psn variable for
* this copy to 0. If it's copy was good, set its psn value to the one
* in the lvm record 
* do the same for the secondary copy
*
* if we were called by the routine that re-builds the vg file, return an error 
*/
if(badprim) {
 	if(rebuild) {
     		free(*map_ptr);
		close(pv_fd);
		return(LVM_VGDA_BB);
	}
	else 
		psn[0] = 0;
}
else
	psn[0] = (lvm_rec.vgda_psn[LVM_PRMRY] * DBSIZE);
if(badsec) {
	if(rebuild) {
		free(*map_ptr);
		close(pv_fd);
		return(LVM_VGDA_BB);
	}
	else
		psn[1] = 0;
}
else
	psn[1] = (lvm_rec.vgda_psn[LVM_SCNDRY] * DBSIZE);
retcode = lvm_gettsinfo(pv_fd,psn,lvm_rec.vgda_len,&copy,rebuild);
if(retcode < LVM_SUCCESS) {
	free(*map_ptr);
	close(pv_fd);
 	return(retcode);
}
offset = psn[copy];
  /* find offset from beginning of the physical volume of the volume group
     descriptor area */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the volume group
     descriptor area */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

    {

    free (*map_ptr);
      /* free the space allocated for the fake mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PROGERR);
      /* return with code for programming error */

    }
retcode = read (pv_fd, vgda_ptr, lvm_rec.vgda_len * DBSIZE);
  /* read the volume group descriptor area from the physical volume */

if (retcode != lvm_rec.vgda_len * DBSIZE)
  /* if an error occurred */

    {

    free (*map_ptr);
      /* free the space allocated for the fake mapped file */

    close (pv_fd);
      /* close the physical volume */

    return (LVM_PVDAREAD);
      /* return with error code for error reading volume group descriptor
	 area from a physical volume */

    }

lvm_mapoff ((struct fheader *) *map_ptr, vgda_ptr);
  /* call routine to calculate the offsets to certain volume group
     descriptor area information and store in the mapped file header
     structure which precedes the in-memory copy of the volume group
     descriptor area which was read from the indicated PV */

close(pv_fd);

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_getpvda */

/***********************************************************************
 *                                                                     *
 * NAME:  lvm_gettsinfo                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads in the beginning and ending timestamps for     *
 *   the volume group descriptor areas on a PV and records status      *
 *   information about the timestamps.                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pvfd                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     ts_status                                                      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   Zero is returned if successful.                                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_gettsinfo (

int pvfd,		     /* file descriptor for physical volume */

daddr_t psn[LVM_PVMAXVGDAS], /* array of physical sector numbers for VGDAs */

long  vgdalen,		     /* length of volume group descriptor area */

int   *copy, 		     /* copy of VGDA with newest timestamp */

int   rebuild)		     /* a rebuild of the vg file is being done */

{ /* BEGIN lvm_getdainfo */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char vghdr_rec [DBSIZE];
  /* buffer into which the volume group header will be read */

char vgtrail_rec [DBSIZE];
  /* buffer into which the volume group trailer will be read */

struct vg_header * vghdr_ptr;
  /* structure into which the volume group header of the descriptor
     area will be read */

struct vg_trailer * vgtrail_ptr;
  /* structure into which the volume group trailer of the descriptor
     area will be read */

int retcode, rc;
  /* return codes */
int temp;
  /* temporary variable */
off_t offset;
  /* the offset in bytes from the beginning of the logical volume where
     the file pointer is to be placed for the next read */

short int da;
  /* index variable for the copy of the descriptor area on the PV */

struct da_sa_info status[LVM_PVMAXVGDAS];
  /* structure that holds info on the VGDAs ... timestamps,etc. */

int prim_comp, prim_nomatch;
  /* indicators for primary copy of VGDA */

int sec_comp, sec_nomatch;
  /* indicators for secondary copy of VGDA */

/* initialize variables */
prim_comp = 0;
prim_nomatch = FALSE;
sec_comp = 0;
sec_nomatch = FALSE; 
/*
 *   For each VGDA copy on this PV, read beginning and ending          
 *   timestamps.                                                      
 */

for (da = 0; da < LVM_PVMAXVGDAS; da++) {
  status[da].ts_status = 0;
  /* loop for each descriptor area copy on a PV */
 	if(psn[da] != 0) {
  		offset = lseek (pvfd, psn[da], LVM_FROMBEGIN);
    		/* position file pointer to beginning of descriptor area */

  		if (offset == LVM_UNSUCC) {
      			status[da].ts_status = LVM_TSRDERR;
		/* turn on bit to indicate a read error on beg timestamp */
	      		continue;
     	   	/* continue with next copy of VGDA */
   		}      

        	retcode = read (pvfd, vghdr_rec, DBSIZE);
       		/* read the volume group header */

  		if (retcode == DBSIZE) {
    		/* if we read the entire header */
			vghdr_ptr = (struct vg_header *) vghdr_rec;
         		/*pointer to volume group header */
                	status[da].ts_beg = vghdr_ptr -> vg_timestamp;
			/* store the beginning timestamp from the volume group
		 	* header into the array */
  		}
  		else {
      			status[da].ts_status = LVM_TSRDERR;
			/* turn on bit in timestamp status flag to indicate a
	   		read error on the beginning timestamp */
      			continue;
			/* continue with next copy of VGDA */
  		}
		temp = vgdalen * DBSIZE; 
  		offset = (psn[da] + temp) - DBSIZE;
    		/* calculate the offset in bytes from beginning of the logical
       		volume for this descriptor area's volume group trailer */

  		offset = lseek (pvfd, offset, LVM_FROMBEGIN);
    		/* position file pointer to beginning of descriptor area */

  		if (offset == LVM_UNSUCC) {
      			status[da].ts_status = LVM_TSRDERR;
			/* turn on bit in timestamp status flag to indicate
			  a read error on the ending timestamp */
      			continue;
			/* continue with next copy of VGDA */
  		}
  		retcode = read (pvfd, vgtrail_rec, DBSIZE);
    		/* read the volume group trailer */

  		if (retcode == DBSIZE) {
    		/* if we read the entire trailer record */
      			vgtrail_ptr = (struct vg_trailer *) vgtrail_rec;
			/* set pointer to volume group header */
      			status[da].ts_end = vgtrail_ptr -> timestamp;
			/* store the ending timestamp from the volume group 
			trailer into the array */
  		}
  		else {
      		/* error on read */
      			status[da].ts_status = LVM_TSRDERR;
			/* turn on bit in timestamp status flag to indicate a
	   		read error on the ending timestamp */
      			continue;
			/* continue with next copy of VGDA */
  		}
  	}
	else
		status[da].ts_status = LVM_TSRDERR;
}
/* 
*  if we are re-building the vg file and any copies had read errors,
*  return an error
*/
   if(rebuild) {
   	if(status[0].ts_status == LVM_TSRDERR ||
		 status[1].ts_status == LVM_TSRDERR)
			return(LVM_VGDA_BB);
   }
   /* if both copies have read errors, return an error */
   if(status[0].ts_status == LVM_TSRDERR && status[1].ts_status == LVM_TSRDERR)
	return(LVM_VGDA_BB);
  /*
   *   If reads were good, compare beginning and ending timestamps for 
   *   each VGDA copy and save in timestamp status. 
   */
   if(status[0].ts_status != LVM_TSRDERR) {
  	prim_comp = (short int) lvm_tscomp (
		      &(status[0].ts_beg),&(status[0].ts_end));
    	/* call routine to compare the beginning and ending timestamp values */
	if(prim_comp != LVM_BTSEQETS)
	 	prim_nomatch = TRUE;
  }      
  /*
   *   If reads were good, compare beginning and ending timestamps for 
   *   each VGDA copy and save in timestamp status. 
   */
  if(status[1].ts_status != LVM_TSRDERR) {
  	sec_comp = (short int) lvm_tscomp (
		      &(status[1].ts_beg),&(status[1].ts_end));
    	/* call routine to compare the beginning and ending timestamp values */
	if(sec_comp != LVM_BTSEQETS)
		sec_nomatch = TRUE;
   }
   /* 
    * if neither of the copies have beginning and ending timestamps that match
    *  return an error
    */
   if(sec_nomatch && prim_nomatch) 
	return(LVM_VGDA_BB);
   /*
    * if both copies have matching timestamps compare their beginning
    * timestamps to see which one is the newest
    */
   if(prim_nomatch == FALSE && sec_nomatch == FALSE) {
  	rc = (short int) lvm_tscomp (
		      &(status[0].ts_beg),&(status[1].ts_beg));
   	/* call routine to compare the beginning timestamp values */
   	if(rc == LVM_BTSEQETS) {
		*copy = LVM_PRMRY;
		return(LVM_SUCCESS);
	}
        /* if the primary copy is newer than the secondary copy return it */ 
	if(rc == LVM_GREATER) {
		*copy = LVM_PRMRY;
		return(LVM_SUCCESS);
	}
	/* if the secondary copy is newer return it */
	if(rc == LVM_LESS) {
		*copy = LVM_SCNDRY;
		return(LVM_SUCCESS);
	}
   }
   else {
	/* if we are doing a rebuild of the vg file, return an error */
        if(rebuild)
		return(LVM_VGDA_BB);
	/* 
	 * if the secondary copy was the only one that had matching
	 * timestamps, return it
	 */
	if(prim_nomatch) {
		*copy = LVM_SCNDRY;
		return(LVM_SUCCESS);
	}
	/* 
	 * it the primary copy was the only one that had matching
	 * timestamps, return it
	 */
	if(sec_nomatch) 
		*copy = LVM_PRMRY;
   }

return(LVM_SUCCESS);
  /* return to caller */

}


/**********************************************************************
 *                                                                     *
 * NAME:  lvm_freebuf                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine free the memory that passed in                       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *	freebuf_ptr                                                    *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 ***********************************************************************
 */
lvm_freebuf(freebuf_ptr)
caddr_t freebuf_ptr;
{
	if (freebuf_ptr != NULL)
	{
		free(freebuf_ptr);
	}
}
