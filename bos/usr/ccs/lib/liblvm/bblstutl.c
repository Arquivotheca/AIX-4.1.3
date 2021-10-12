static char sccsid[] = "@(#)63	1.11  src/bos/usr/ccs/lib/liblvm/bblstutl.c, liblvm, bos411, 9428A410j 3/8/93 13:02:25";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - bblstutl.c
 *
 * FUNCTIONS: lvm_addbb,
 *            lvm_bldbblst,
 *            lvm_chgbb
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
#include <sys/hd_config.h>;
#include <liblvm.h>;
#include <lvmrec.h>;
#include <sys/hd_psn.h>;









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_addbb                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine adds a bad block to a linked list of bad block       *
 *   structures which is sorted in ascending order on the bad block    *
 *   number field.                                                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     head_ptr                                                        *
 *     bb_ptr                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *head_ptr                                                       *
 *                                                                     *
 *     The bad block structure pointed to by bb_ptr has been added     *
 *     to the linked list.                                             *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_addbb (


struct bad_blk ** head_ptr,
  /* a pointer to the pointer to the head of the bad block linked list */

struct bad_blk * bb_ptr)
  /* pointer to the bad block structure which is to be added to the
     list */


{ /* BEGIN lvm_addbb */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct bad_blk * cur_ptr;
  /* pointer to the bad block structure which is currently being checked
     for insertion order */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

cur_ptr = *head_ptr;
  /* set pointer to current bad block to point at the head of the list */

if (cur_ptr == NULL  ||  bb_ptr -> blkno < cur_ptr -> blkno)
  /* if the linked list of bad blocks is null or if the bad block number
     of the new block to add is less than the bad block number of the
     header block in the list, then add the new block at the front of the
     linked list */

{

    bb_ptr -> next = cur_ptr;
      /* set the new bad block's next pointer to point at the head of
	 the linked list */

    *head_ptr = bb_ptr;
      /* set the header pointer to point at the new bad block */

}

else
  /* block not added at front of list */

{

    while (cur_ptr -> next != NULL  &&  bb_ptr -> blkno >
					  cur_ptr -> next -> blkno)
      /* loop while the current block is not the end of the list and
	 while the bad block number of the new block to add is greater
	 than the block number of the next block in the list */

    {

      cur_ptr = cur_ptr -> next;
	/* increment current pointer to point at the next bad block in
	   the list */

    }

    bb_ptr -> next = cur_ptr -> next;
      /* set the new bad block's pointer to the next block in the list to
	 the block which now follows the current block */

    cur_ptr -> next = bb_ptr;
      /* change the current block's pointer to the next bad block to
	 point to the new bad block which is being added to the list */

} /* block not added at front of list */

return;
  /* return to caller */

} /* END lvm_addbb */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_bldbblst                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine builds the linked lists of bad block structures      *
 *   for a physical volume.                                            *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     pvol_ptr                                                        *
 *     reloc_psn                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     Linked lists of bad block structures have been built and are    *
 *     pointed to by the defects array of the LVDD physical volume     *
 *     structure.                                                      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_ALLOCERR                                                    *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_bldbblst (


int pv_fd,
  /* the file descriptor for this physical volume device */

struct pvol * pvol_ptr,
  /* a pointer to a structure which describes a physical volume for the
     logical volume device driver (LVDD) */

daddr_t reloc_psn)
  /* the physical sector number of the beginning of the bad block
     relocation pool */


{ /* BEGIN lvm_bldbblst */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

char bbdir_buf [LEN_BB_DIR * DBSIZE];
  /* a buffer into which the bad block directory will be read */

struct bb_hdr * bbdir_hdr;
  /* a pointer to the header entry of the bad block directory */

struct bb_entry * bbdir_entry;
  /* a pointer to an entry in the bad block directory */

struct bb_entry * bb2_entry;
  /* a pointer to an entry in the bad block directory */

struct bad_blk * bb_ptr;
  /* a pointer to a structure which describes a bad block to the LVDD */

daddr_t sav_badblk;
  /* variable in which to save the block address of the bad block */

daddr_t sav_relblk;
  /* variable in which to save the block address of the relocation
     block */

short int hash_val;
  /* loop index used for hash value */

short int bb_index;
  /* loop index used for bad block directory entries */

short int bb2_ind;
  /* loop index used for bad block directory entries */

int retcode;
  /* return code */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


/***********************************************************************
 *   Read bad block directory from the disk.                           *
 ***********************************************************************
 */

retcode = lvm_rdbbdir (pv_fd, bbdir_buf, LVM_BBRDRECV);
  /* call routine to read in the bad block directory from the physical
     volume */

/* if the retcode from lvm_rdbbdir() is LVM_PVRDRELOC, it indicates 
   that one of the bad block directories has a problem but another 
   one appears to be intact and can be used.  So go ahead and build
   kernel structures and reture LVM_PVRDRELOC.  The caller of this
   routine should set the PV in read only relocation if the caller
   sees LVM_PVRDRELOC as the reture code.                           */
if ((retcode < LVM_SUCCESS) && (retcode != LVM_PVRDRELOC))
  /* if an error occurred */

{

    return (retcode);
      /* return with unsuccessful return code */

}

bbdir_hdr = (struct bb_hdr *) bbdir_buf;
  /* set a pointer to the header entry of the bad block directory */

pvol_ptr -> num_bbdir_ent = bbdir_hdr -> num_entries;
  /* get the number of entries in the bad block directory and store this
     value in the physical volume structure */


/************************************************************************
 *   Search bad block directory to find the largest numbered relocation *
 *   block in order to find the next available relocation block number, *
 *   which is stored in the physical volume structure.                  *
 ************************************************************************
 */

bbdir_entry = (struct bb_entry *) (bbdir_buf + sizeof (struct bb_hdr) +
	      (bbdir_hdr -> num_entries - 1) * sizeof (struct bb_entry));
  /* set a pointer to the last entry in the bad block directory;  note
     that this value will not be used if num_entries = 0 */

pvol_ptr -> next_relblk = reloc_psn;
  /* initialize the PV address of the next available relocation block to
     the beginning of the bad block relocation pool */

for (bb_index=bbdir_hdr->num_entries; bb_index>0; bb_index=bb_index-1)
  /* since bad block directory is in ascending order by relocated block
     number, search from end of list for last relocated block used */

{

  if (bbdir_entry -> rel_stat == REL_DONE)
    /* if this bad block is one that has already been relocated */

  {

      pvol_ptr -> next_relblk = (daddr_t) bbdir_entry -> rel_lsn + 1;
	/* set the PV address of the next available relocation block to
	   one greater than the relocation block number for this bad
	   block entry */

      break;
	/* exit for loop since highest address of used relocation blocks
	   has been found */

  }

  bbdir_entry = (struct bb_entry *) ((caddr_t) bbdir_entry -
				       sizeof (struct bb_entry));
    /* decrement pointer to point at the previous bad block directory
       entry */

} /* search bad block directory for last relocated block used */


/************************************************************************
 *   Build kernel defect table with linked lists of bad block           *
 *   structures.                                                        *
 ************************************************************************
 */

bbdir_entry = (struct bb_entry *) (bbdir_buf + sizeof (struct bb_hdr));
  /* set a pointer to the first non-header entry of the bad block
     directory */

for (bb_index=0; bb_index < bbdir_hdr->num_entries; bb_index=bb_index+1)
  /* loop for number of entries in the bad block directory */

{

  if ((daddr_t) bbdir_entry -> bb_lsn < reloc_psn)
    /* if the bad block for the current entry in the directory is not
       located within the bad block relocation pool */

  {

      /******************************************************************
       *   Bad block is not located within the relocation pool.         *
       *   Add a bad block structure to the kernel defect table for the *
       *   current entry in the bad block directory.                    *
       ******************************************************************
       */

      bb_ptr = (struct bad_blk *) malloc (sizeof (struct bad_blk));
	/* allocate space for an LVDD bad block structure */

      if (bb_ptr == NULL)
	/* if an error occurred */

      {

	  return (LVM_ALLOCERR);
	    /* return with error code for memory allocation error */

      }

      bb_ptr -> dev = pvol_ptr -> dev;
	/* set the device identification for this PV in the bad block
	   structure */

      bb_ptr -> next = NULL;
	/* initialize the pointer to the next bad block to be null */

      bb_ptr -> blkno = (daddr_t) bbdir_entry -> bb_lsn;
	/* use the logical sector number of the bad block from the
	   directory entry to set the bad block number in the LVDD
	   structure */

      bb_ptr -> relblk = bbdir_entry -> rel_lsn;
	/* use the logical sector number of the relocated block from the
	   directory entry to set the relocated block number in the LVDD
	   structure */

      bb_ptr -> status = bbdir_entry -> rel_stat;
	/* use the relocation status field from the bad block directory
	       entry to set the status field in the LVDD structure */

      hash_val = BBHASH_IND (bb_ptr -> blkno);
	/* calculate the hash value which is used as an index into the
	   defects array of where this bad block will be linked */

      lvm_addbb (&(pvol_ptr -> defect_tbl -> defects [hash_val]),
		 bb_ptr);
	/* call routine to add the LVDD bad block structure to the
	   desired linked list */

  } /* bad block not in relocation pool */

  else
    /* bad block for current entry is in relocation pool */

  {

      /******************************************************************
       *   Bad block is located within the relocation pool.             *
       *   Search bad block directory backwards from current entry in   *
       *   order to find the original bad block (i.e., one that is not  *
       *   in the relocation pool).                                     *
       ******************************************************************
       */

      sav_badblk = (daddr_t) bbdir_entry -> bb_lsn;
	/* save the bad block for the current entry */

      sav_relblk = (daddr_t) bbdir_entry -> rel_lsn;
	/* save the relocation block for the current entry */

      bb2_entry = (struct bb_entry *) ((caddr_t) bbdir_entry -
				       sizeof (struct bb_entry));
	/* set a pointer to the entry in the bad block directory which is
	   previous to the current entry */

      for (bb2_ind = bb_index - 1; bb2_ind >= 0; bb2_ind = bb2_ind - 1)
	/* search the bad block directory backwards from the entry
	   previous to the current entry to the beginning of the list,
	   looking for the original bad block which is not within the
	   relocation pool */

      {

	if (sav_badblk == (daddr_t) bb2_entry -> rel_lsn)
	  /* if the block address of the saved bad block number matches
	     the block address of the relocation block for the entry
	     being compared */

	{

	    sav_badblk = (daddr_t) bb2_entry -> bb_lsn;
	      /* set a new value for the saved bad block to be the bad
		 block of the relocation block just found */

	    if (sav_badblk < reloc_psn)
	      /* if the new saved bad block is not contained within the
		 relocation pool, which means it is the original bad
		 block, then exit for loop;  otherwise, continue search
		 for original bad block */

	    {

		break;
		  /* exit for loop since original bad block found */

	    }

	} /* saved bad block matches reloc block */

	bb2_entry = (struct bb_entry *) ((caddr_t) bb2_entry -
					 sizeof (struct bb_entry));
	  /* decrement pointer to look at previous entry in the directory
	     in the search for the original bad block */

      } /* search bad block directory for original bad block */


      if (bbdir_entry -> rel_stat == REL_DONE)
	/* if the relocation status for the current entry is relocation
	   done */

      {

	  /**************************************************************
	   *   Find original bad block in the linked list of kernel bad *
	   *   block structures and change the relocation block to be   *
	   *   that of the relocation block for current entry in the    *
	   *   bad block directory.                                     *
	   **************************************************************
	   */

	  hash_val = BBHASH_IND (sav_badblk);
	    /* calculate the hash value of the original bad block, which
	       is used as an index into the defects array of the linked
	       list where the original bad block will be linked */

	  lvm_chgbb (pvol_ptr -> defect_tbl -> defects [hash_val],
		     sav_badblk, sav_relblk, LVM_BBCHGRBLK);
	    /* call routine to change the LVDD bad block structure in the
	       linked list by updating its relocation block */

      }

      else
	/* relocation status is not for relocation done */

      {

	  /**************************************************************
	   *   Add a bad block structure to the kernel defect table for *
	   *   current entry in the bad block directory.                *
	   **************************************************************
	   */

	  bb_ptr = (struct bad_blk *) malloc (sizeof (struct bad_blk));
	    /* allocate space for an LVDD bad block structure */

	  if (bb_ptr == NULL)
	    /* if an error occurred */

	  {

	      return (LVM_ALLOCERR);
		/* return with error code for memory allocation error */

	  }

	  bb_ptr -> dev = pvol_ptr -> dev;
	    /* set the device identification for this PV in the bad block
	       structure */

	  bb_ptr -> next = NULL;
	    /* initialize the pointer to the next bad block to be null */

	  bb_ptr -> blkno = (daddr_t) bbdir_entry -> bb_lsn;
	    /* use the logical sector number of the bad block from the
	       directory entry to set the bad block number in the LVDD
	       structure */

	  bb_ptr -> relblk = bbdir_entry -> rel_lsn;
	    /* use the logical sector number of the relocated block from
	       the directory entry to set the relocated block number in
	       the LVDD structure */

	  bb_ptr -> status = bbdir_entry -> rel_stat;
	    /* use the relocation status field from the bad block
	       directory entry to set the status field in the LVDD
	       structure */

	  hash_val = BBHASH_IND (bb_ptr -> blkno);
	    /* calculate hash value which is used as an index into the
	       defects array of where this bad block will be linked */

	  lvm_addbb (&(pvol_ptr -> defect_tbl -> defects [hash_val]),
		     bb_ptr);
	    /* call routine to add the LVDD bad block structure to the
	       desired linked list */

	  /**************************************************************
	   *   Find original bad block in the linked list of kernel bad *
	   *   block structures and change the status field to indicate *
	   *   it is now a chained entry.                               *
	   **************************************************************
	   */

	  hash_val = BBHASH_IND (sav_badblk);
	    /* calculate the hash value of the original bad block, which
	       is used as an index into the defects array of the linked
	       list where the original bad block will be linked */

	  lvm_chgbb (pvol_ptr -> defect_tbl -> defects [hash_val],
		     sav_badblk, sav_relblk, LVM_BBCHGSTAT);
	    /* call routine to change the LVDD bad block structure in the
	       linked list by updating its status field */

      } /* relocation status is not relocation done */

  } /* bad block in relocation pool */

  bbdir_entry = (struct bb_entry *) ((caddr_t) bbdir_entry +
				       sizeof (struct bb_entry));
    /* increment pointer to point at the next bad block directory
       entry */

} /* loop for all entries in bad block directory */

return (retcode);
  /* return with successful return code */

} /* END lvm_bldbblst */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chgbb                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine changes a bad block in the linked list of bad block  *
 *   structures with a new value for the relocation block or status.   *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     head_ptr                                                        *
 *     bad_blk                                                         *
 *     reloc_blk                                                       *
 *     chgtype                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_chgbb (


struct bad_blk * head_ptr,
  /* a pointer to the head of the bad block linked list */

daddr_t bad_blk,
  /* the bad block whose relocation block is to be changed */

daddr_t reloc_blk,
  /* the new value for the relocation block, if it is to be changed */

int chgtype)
  /* type of change (relocation block or status field) to make to the bad
     block structure */


{ /* BEGIN lvm_chgbb */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct bad_blk * cur_ptr;
  /* pointer to the bad block structure which is currently being checked
     for a matching bad block */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

cur_ptr = head_ptr;
  /* set pointer to current bad block to point at the head of the list */

while (cur_ptr != NULL)
  /* while there are still bad blocks to search in the linked list */

{

  if (bad_blk == (daddr_t) cur_ptr -> blkno)
    /* if the bad block for this entry in the list matches the bad block
       entry to be updated */

  {

      if (chgtype == LVM_BBCHGRBLK)
	/* if the type of change requested is to change the relocation
	   block */

      {

	  cur_ptr -> relblk = reloc_blk;
	    /* set the relocation block for this bad block to the new value
	       passed in */

      }

      else
	/* type of change (LVM_BBCHGSTAT) requested is to change the
	   status field */

      {

	  cur_ptr -> status = REL_CHAINED;
	    /* set the status field for this bad block to indicate it is
	       chained */

      }

      break;
	/* exit while loop since matching bad block was found */

  }

  cur_ptr = cur_ptr -> next;
    /* increment current pointer to point at the next bad block in the
       list */

} /* loop while there are bad blocks in the linked list */

return;
  /* return to caller */

} /* END lvm_chgbb */
