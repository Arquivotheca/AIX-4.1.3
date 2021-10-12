static char sccsid[] = "@(#)06	1.1  src/bos/diag/util/udiskenh/udmbbdir.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:54:41";
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: end_of_chain
 *		equal_to
 *		greater_than
 *		lvm_bbdsane
 *		lvm_getbbdir
 *		rdbbdir
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* This file is essentially a copy of an LVM source file, modified to use   */
/* ioctl(DKIORDSE) instead of lseek and read.  Unused functions have also   */
/* been removed. */
#pragma options nosource
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/time.h>
#include <sys/bbdir.h>
#include <sys/buf.h>
#include <sys/shm.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/param.h>
#include <lvmrec.h>
#include <lvm.h>
#include <sys/hd_psn.h>
#include "udmutil.h"
#include "udmbbdir.h"
#pragma options source

const int entry_size = sizeof(struct bb_entry);

/*
 * FUNCTION:
 * Function greater_than is used by qsort to sort the bad block directory 
 * in order of bad block number instead of relocated block number.         
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: <0 - left < right
 *           0 - left == right
 *          >0 - left > right
 */
int greater_than(const void* left, const void* right) {
    return ((struct bb_entry *)left)->bb_lsn -
           ((struct bb_entry *)right)->bb_lsn;
}

/*
 * FUNCTION:
 * Function equal_to is used by the bsearch routine.  It is called    
 * with a pointer to a key and a pointer to an array element           
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: <0 - key < right
 *           0 - key == right
 *          >0 - key > right
 */
int equal_to(const void* key, const void* right) {
    /* We are looking for the array entry whose bad block matches */
    /* the key's relocated block. */
    return ((struct bb_entry*)key)->rel_lsn - 
           ((struct bb_entry*)right)->bb_lsn;
}

/*
 * FUNCTION:
 * function end_of_chain accepts an array of bb_entries sorted by bad 
 * block number and a pointer to an element in the array,             
 * and returns a pointer to                                           
 * the entry at the end of the chain (e.g. block 10 is relocated to   
 * block 100 and block 100 is relocated to block 150 is a chain).     
 * All chains, even those of length one, end in one of two ways:      
 * the relocation block is zero (meaning the bad block has not been   
 * relocated) or it is non-zero and there is no entry to indicate the 
 * relocation block is also a bad block.  In either case, the bsearch 
 * routine will return NULL.  The caller of end_of_chain has to       
 * examine the values in the element returned to know which case      
 * occurred.                                                          
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: <0 - key < right
 *           0 - key == right
 *          >0 - key > right
 */
struct bb_entry* end_of_chain(struct bb_entry *array_base, 
                              int array_size, 
                              struct bb_entry *start) {
    struct bb_entry *temp, *current = start;
    /* Search for another entry whose bad block is the same as the current */
    /* entry's relocated block.  If one exists, it becomes the current     */
    /* entry and the loop follows the chain to the end.                    */
    temp = bsearch(current,array_base, array_size,entry_size, equal_to);
    while (temp) {
        current = temp;
        temp = 
           bsearch(current,array_base, array_size,entry_size, equal_to);
    }
    return current;
}
      

/*
 *
 * NAME:  rdbbdir
 *
 * FUNCTION:
 *   This routine reads the bad block directory from a physical
 *   volume, sorts it by bad block numbers and returns a pointer to it.
 *   The return value is NULL if any problem is encountered.   
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NULL - failed
 *          Non-NULL - a pointer to the bad block directory.
 */

void*
rdbbdir (
int	pv_fd)	/* file descriptor for physical volume device	*/
{
    char *buf = malloc(DBSIZE * LEN_BB_DIR);
    struct bb_hdr *hdr_ptr;
    if(lvm_getbbdir(pv_fd, buf, LVM_BBPRIM) != LVM_SUCCESS &&
       lvm_getbbdir(pv_fd, buf, LVM_BBBACK) != LVM_SUCCESS) {
        FREE (buf);
    }
    else {
        /* Most of the space in buf is probably unused.  The first    */
        /* structure within buf contains the number of bb entries, so */
        /* use that to compute how much space is really used and call */
        /* realloc to give back the unused portion.                   */
        hdr_ptr = (struct bb_hdr*) buf;
        buf = realloc(buf,hdr_ptr->num_entries * entry_size +
                      sizeof(struct bb_hdr));
        hdr_ptr = (struct bb_hdr*) buf;  /* Just to be sure. */

        /* Now sort bb entries using the greater_than function defined*/
        /* earlier. */
#ifdef DBG
{
  int i;
  struct bb_entry *entry = (struct bb_entry*) (hdr_ptr + 1);
  for (i=0; i<hdr_ptr->num_entries; ++i, ++entry) {
    fprintf (stderr,"bbentry[%d] = { reason = %d,\n",i, entry->reason);
    fprintf (stderr,"                bb_lsn = %d,\n", entry->bb_lsn);
    fprintf (stderr,"                rel_stat = %d,\n", entry->rel_stat);
    fprintf (stderr,"                rel_lsn = %d}.\n", entry->rel_lsn);
  }
}
#endif
        qsort(buf+sizeof(struct bb_hdr), hdr_ptr->num_entries,
              entry_size, greater_than);
#ifdef DBG
{
  int i;
  struct bb_entry *entry = (struct bb_entry*) (hdr_ptr + 1);
  for (i=0; i<hdr_ptr->num_entries; ++i, ++entry) {
    fprintf (stderr,"bbentry[%d] = { reason = %d,\n", i, entry->reason);
    fprintf (stderr,"                bb_lsn = %d,\n", entry->bb_lsn);
    fprintf (stderr,"                rel_stat = %d,\n", entry->rel_stat);
    fprintf (stderr,"                rel_lsn = %d}.\n", entry->rel_lsn);
  }
}
#endif
    }
    return buf;
}

/*
 *
 * NAME:  lvm_getbbdir
 *
 * FUNCTION:
 *   This routine reads a bad block directory from a physical
 *   volume.
 *
 * NOTES:
 *   The header record is checked for "DEFECT" and that the number
 *   of entries is within reason, then the directory is checked for sanity.
 *
 *   dir_flg controls which directory is read ASSUMES only one directory
 *   is read at a time.
 *
 * RETURN VALUE DESCRIPTION:
 *   LVM_SUCCESS	- Operation was successful
 *   LVM_PROGERR	- Error on lseek to directory
 *   LVM_BBRDERR	- Read error on directory
 *   LVM_BBINSANE	- Directory is not sane
 *
 */

int
lvm_getbbdir (
int	pv_fd,		/* file descriptor for physical volume device	*/
char	*buf,		/* buffer where bad block directory will be read*/
int     dir_flg)        /* 1 = read primary  2 = read backup            */
{
    struct bb_hdr	*bb_hdr;/* ptr to header of directory		*/
    int			len;	/* length of directory in blocks.       */
    daddr_t		sblk;	/* block number of directory to read	*/

    /*
     * Get the starting block number of the target directory
     */
    sblk = (dir_flg == LVM_BBPRIM) ? PSN_BB_DIR : PSN_BB_BAK;

    /*
     * Read in first block of BB directory
     */
    if (get_block(pv_fd, scsi, sblk, 1, buf, NULL) == -1)
	return( LVM_BBRDERR );

    /*
     * If there was no error on the first block then check that the 
     * header says DEFECT
     */
    bb_hdr = (struct bb_hdr *)buf;
    if(strncmp(bb_hdr->id,BB_DIR_ID,sizeof(bb_hdr->id))!=LVM_STRCMPEQ)
	return( LVM_BBINSANE );

    /*
     * Check that the number of entries is not insane before use it
     */
    if( (bb_hdr->num_entries < 0) || (bb_hdr->num_entries > MAX_BBENTRIES) )
	return( LVM_BBINSANE );

    /*
     * Read in remainder of the directory if it is longer than one block
     */
    len = ((bb_hdr->num_entries * sizeof(struct bb_entry)) +
           sizeof(struct bb_hdr));  /* number of bytes in bb dir. */
    len = (len-1)/DBSIZE + 1;       /* number of blocks in bb dir. */ 
    if (len > 1) {
    /* Since we've already read the first block, increment the block number, */
    /* decrement len (the number of blocks to read), and increment the buf   */
    /* pointer by DBSIZE.   */
        if (get_block(pv_fd,scsi,++sblk,--len,buf += DBSIZE, NULL)==-1)
                return LVM_BBINSANE;
    }
	
    if( lvm_bbdsane( buf ) == LVM_SUCCESS )
	return( LVM_SUCCESS );
    else
	return( LVM_BBINSANE );
}

/*
 * NAME:  lvm_bbdsane
 *
 * FUNCTION:
 *   This routine checks that the given bad block directory is sane
 *
 * NOTES:
 *   The following checks are made:
 *	- The relocation status must be done or desired
 *	- If the relocation status is done 
 *	  THEN
 *	    relocation block can not be zero
 *	    bad block number can not be greater than relocation block
 *	    relocation block is in relocation pool
 *	    relocation block is sequentially correct
 *	    neither the bad block or relocation block is past end of device
 *	  ELSE
 *	    relocation block must be zero
 *	    bad block can not be past end of device if known
 *
 * RETURN VALUE DESCRIPTION:
 *   LVM_SUCCESS	- Directory is sane
 *   LVM_UNSUCC		- Directory is insane
 *
 */

int
lvm_bbdsane (
char	*buf)		/* ptr to BB directory			*/
{
    struct bb_hdr	*bb_hdr;	/* ptr to header of directory	*/
    struct bb_entry	*bb_entry;	/* ptr to a directory entry	*/

    int			i;		/* general counter		*/
    daddr_t		strt_pool;	/* starting blk of relocation pool*/
    daddr_t		last_reloc;	/* last relocation blk used	*/
    daddr_t		last_blk;	/* last useable blk on the PV	*/

    last_blk = strt_pool = 0;
    /* 
     * Generate ptr to the header and first entry
     */
    bb_hdr = (struct bb_hdr *)buf;
    bb_entry = (struct bb_entry *)(bb_hdr + 1);

    /*
     * Check each entry for saneness
     */
    for( i=0; i < bb_hdr->num_entries; i++, bb_entry++ ) {

	/*
	 * Check that that status is done or desired
	 */
	if( (bb_entry->rel_stat != REL_DONE) &&
	    (bb_entry->rel_stat != REL_DESIRED) ) {

	    return( LVM_UNSUCC );
	}

	if( bb_entry->rel_stat == REL_DONE ) {

	    /*
	     * if relocation blk is zero or bad block is greater than
	     * the relocated blk there is a problem
	     */
	    if( (bb_entry->rel_lsn == 0) ||
		(bb_entry->bb_lsn > bb_entry->rel_lsn) ) {

		return( LVM_UNSUCC );
	    }

	    /*
	     * if first entry with relocation block initialize the
	     * starting blk number of pool and last relocation block
	     * used.
	     *
	     * We assume the directory is sane.  If it is not these
	     * values are bogus and hopefully we will find that out
	     * on the next entry.
	     */
	    if( !strt_pool ) {
		strt_pool = bb_entry->rel_lsn;
		last_reloc = strt_pool - 1;
		last_blk = strt_pool + LVM_RELOC_LEN;
	    }

	    /*
	     * Verify that:
	     *	1. relocation blk is in the relocation pool
	     *  2. relocation blk is in sequence with the previous one
	     *	3. neither block is past the end of the device.
	     */
	    if( (bb_entry->rel_lsn < strt_pool)		||
		(bb_entry->rel_lsn <= last_reloc)	||
		(bb_entry->bb_lsn > last_blk)		||
		(bb_entry->rel_lsn > last_blk) ) {

		return( LVM_UNSUCC );
	    }

	    last_reloc = bb_entry->rel_lsn;
	}
	else {
	    /*
	     * rel_stat is equal to desired
	     */

	    /* relocation block must be zero */
	    if( bb_entry->rel_lsn != 0 )
		return( LVM_UNSUCC );

	    /*
	     * bad block can not be past end of device.  But make sure
	     * the last block has been set.
	     */
	    if( last_blk ) {
		if( bb_entry->bb_lsn > last_blk )
		    return( LVM_UNSUCC );
	    }
	}
    } /* END of for each entry */

    return( LVM_SUCCESS );
}

