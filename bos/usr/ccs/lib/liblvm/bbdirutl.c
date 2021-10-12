static char sccsid[] = "@(#)62	1.9  src/bos/usr/ccs/lib/liblvm/bbdirutl.c, liblvm, bos411, 9428A410j 10/4/90 10:39:38";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - bbdirutl.c
 *
 * FUNCTIONS: lvm_bbdsane,
 *            lvm_getbbdir,
 *            lvm_rdbbdir,
 *            lvm_wrbbdir
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
#include <sys/dasd.h>
#include <sys/hd_config.h>
#include <liblvm.h>
#include <lvmrec.h>
#include <sys/hd_psn.h>

/*
 *
 * NAME:  lvm_rdbbdir
 *
 * FUNCTION:
 *   This routine reads the bad block directory from a physical
 *   volume.
 *
 * NOTES:
 *   The act_flg has 3 values:
 *	LVM_BBRDONLY:	Read and return the first sane directory starting
 *			with the primary.    Used by queries.
 *
 *	LVM_BBRDINIT:	Read and return the first sane directory starting
 *			with the primary.  If no sane directory found
 *			then initialize the given buffer to look like an
 *			empty directory.   Used by createvg & installpv
 *
 *			ASSUMES both the primary and backup directory will
 *			be written later.
 *
 *	LVM_BBRDRECV	Read and RECOVER.  Read both directories and verify
 *			they are correct.  If not do what is necessary
 *			to recover the bad directory.  Used by varyon.
 *
 *			If recovery is not possible there are 2 possible
 *			error returns.  The first believes a correct
 *			directory exists but no new bad blocks should be
 *			relocated, LVM_PVRDRELOC.  The second believes
 *			no good directory exists and the PV should not
 *			be used, LVM_BADBBDIR.
 *
 * RETURN VALUE DESCRIPTION:
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */

int
lvm_rdbbdir (
int	pv_fd,		/* file descriptor for physical volume device	*/
char	*buf,		/* buffer where bad block directory will be read*/
int     act_flg)        /* request action flag                          */
{
    struct bb_hdr	*bb_hdr;	/* ptr to header of directory	*/
    int			prc;		/* return code for prim dir	*/
    int			brc;		/* return code for backup dir	*/
    int			len;		/* length of directory		*/

    char	dir_stat;		/* status of directories	*/
    char	tmp_buf[LEN_BB_DIR * DBSIZE];/* temp buffer for a BB dir*/

    /*
     * READ THE BAD BLOCK DIRECTORY ONLY
     *
     * If read only then read the primary and return it unless there is
     * a problem.  In that case try the backup.
     *
     * READ AND INITIALIZE IF NOT SANE
     *
     * If both directories can not be read due to disk errors return an
     * error.  If a directory can be read and it is sane return it.
     * Finally if no sane directory can be found initialize buffer to 
     * look like an empty directory and return that.
     */
    if( (act_flg == LVM_BBRDONLY) || (act_flg == LVM_BBRDINIT) ) {
	if( (prc = lvm_getbbdir(pv_fd, buf, LVM_BBPRIM)) != LVM_SUCCESS) {
	    if( (brc = lvm_getbbdir(pv_fd, buf, LVM_BBBACK)) != LVM_SUCCESS) {
		if( (act_flg == LVM_BBRDONLY) ||
		    ((prc==LVM_BBRDERR) && (brc==LVM_BBRDERR)) ) {

		    return( LVM_BADBBDIR );
		}
		else {
		    bzero( buf, DBSIZE );
		    bb_hdr = (struct bb_hdr *)buf;
		    strncpy( bb_hdr->id, BB_DIR_ID, sizeof(bb_hdr->id) );
		}
	    }
	}
	return( LVM_SUCCESS );
    }
    /*
     * Check to make sure request is correct
     */
    if( act_flg != LVM_BBRDRECV )
	return( LVM_BADBBDIR );

    /*
     * READ BOTH DIRECTORIES AND RECOVER in needed
     *
     * Read both directories and verify their sanity.  If one was in the
     * middle of an update then rewrite it with the other if it is sane.
     */
    dir_stat = 0;
    prc = lvm_getbbdir( pv_fd, buf, LVM_BBPRIM );
    brc = lvm_getbbdir( pv_fd, tmp_buf, LVM_BBBACK );

    /*
     * Build a binary code of successful directory reads then decide
     * what to do
     */
    if( prc == LVM_SUCCESS )
	dir_stat |= 1;
    if( brc == LVM_SUCCESS )
	dir_stat |= 2;
    
    switch( dir_stat ) {

	case 0:			/* Neither directory can be used	*/

	    return( LVM_BADBBDIR );

	case 1:			/* Backup directory is bad rewrite it	*/

	    brc = lvm_wrbbdir( pv_fd, buf, LVM_BBBACK );
	    if( brc != LVM_SUCCESS )
		return( LVM_PVRDRELOC );

	    return (LVM_SUCCESS);

	case 2:			/* Primary directory is bad rewrite it	*/

	    /*
	     * Move the backup directory copy to the buffer the caller
	     * provided.
	     */
	    bb_hdr = (struct bb_hdr *)tmp_buf;
	    len = LVM_BBDIRLEN(bb_hdr);
	    bcopy( tmp_buf, buf, len );
	    prc = lvm_wrbbdir( pv_fd, buf, LVM_BBPRIM );
	    if( prc != LVM_SUCCESS )
		return( LVM_PVRDRELOC );

	    return (LVM_SUCCESS);

	/*
	 * Both directories were read successfully and were sane
	 *
	 * A binary compary could be done as one more check but what do
	 * we do if it does not compare.  They are both SANE!!!
	 */
	case 3:

	    return (LVM_SUCCESS);
    }
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
 * EXTERNAL PROCEDURES CALLED:
 *   lseek
 *   lvm_bbdsane
 *   read
 *   strncmp
 *
 */

int
lvm_getbbdir (
int	pv_fd,		/* file descriptor for physical volume device	*/
char	*buf,		/* buffer where bad block directory will be read*/
int     dir_flg)        /* 1 = read primary  2 = read backup            */
{
    struct bb_hdr	*bb_hdr;/* ptr to header of directory		*/
    int			len;	/* length of directory			*/
    daddr_t		sblk;	/* block number of directory to read	*/

    /*
     * Get the starting block number of the target directory
     */
    sblk = (dir_flg == LVM_BBPRIM) ? PSN_BB_DIR : PSN_BB_BAK;

    /*
     * Read in first block of BB directory
     */
    if( lseek(pv_fd, (off_t)(sblk * DBSIZE), LVM_FROMBEGIN) == LVM_UNSUCC )
	return( LVM_BBRDERR );

    if( read( pv_fd, buf, DBSIZE ) != DBSIZE )
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
    len = LVM_BBDIRLEN(bb_hdr);
    if( len > DBSIZE ) {
	/*
	 * The BB directory is longer than 1 block so read it in
	 */
	len -= DBSIZE;
	if( read( pv_fd, buf + DBSIZE, len ) != len )
	    return( LVM_BBRDERR );
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
 * EXTERNAL PROCEDURES CALLED:
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

/*
 *
 * NAME:  lvm_wrbbdir
 *
 * FUNCTION:
 *   This routine writes the bad block directory to a physical
 *   volume.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *   LVM_SUCCESS		- Write was successful to target(s)
 *   LVM_BBWRERR		- One directory failed to write
 *   LVM_BADBBDIR		- Both directories failed to write
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */

int
lvm_wrbbdir (
int	pv_fd,		/* file descriptor for physical volume device	*/
char	*buf,		/* buffer containing directory to write		*/
int     dir_flg)        /* 1 = primary  2 = backup  3 = both            */
{
    struct bb_hdr	*bb_hdr;/* ptr to header of directory		*/
    daddr_t		sblk;	/* block number of directory to read	*/
    int			len;	/* length in bytes of the BB dir	*/
    int			rc;	/* return code				*/
    char		dir_stat;/* status of directory writes		*/


    dir_stat = 0;
    bb_hdr = (struct bb_hdr *)buf;
    len = LVM_BBDIRLEN(bb_hdr);

    /*
     * Write primary copy of BB directory if requested
     */
    if( dir_flg & LVM_BBPRIM ) {
	sblk = PSN_BB_DIR;
	if(lseek(pv_fd, (off_t)(sblk * DBSIZE), LVM_FROMBEGIN) == LVM_UNSUCC)
	    dir_stat |= 1;		/* Set error flag for primary	*/
	else {
	    if( writex( pv_fd, buf, len, WRITEV) != len )
		dir_stat |= 1;		/* Set error flag for primary	*/
	}
    }
    /*
     * Write backup copy of BB directory if requested
     */
    if( dir_flg & LVM_BBBACK ) {
	sblk = PSN_BB_BAK;
	if(lseek(pv_fd, (off_t)(sblk * DBSIZE), LVM_FROMBEGIN) == LVM_UNSUCC)
	    dir_stat |= 2;		/* Set error flag for backup	*/
	else {
	    if( writex( pv_fd, buf, len, WRITEV) != len )
		dir_stat |= 2;		/* Set error flag for backup	*/
	}
    }

    switch( dir_stat ) {
	
	case 0:				/* No errors			*/

	    return( LVM_SUCCESS );

	case 1:				/* One directory had a write err*/
	case 2:

	    return( LVM_BBWRERR );

	case 3:				/* Both directories had write err*/

	    return( LVM_BADBBDIR );
    }
}
