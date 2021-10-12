/* @(#)79       1.10   1/13/93 18:10:11 */

/*
 *   COMPONENT_NAME: SYSXLVM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1990
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 *   This include file describes the structure of the bad block
 *   directory.  There are two copies of the bad block directory on
 *   each physical volume, a primary copy and a backup copy.  The 
 *   directory is made up of "bb_hdr" structure(header record),
 *   followed by N "bb_entry" structures.  N is the number of entries
 *   in the directory indicated by the "num_entries" field of the 
 *   header record.  The header record id field will contain the ASCII
 *   value "UPDATE" when the directory is being updated and "DEFECT" when
 *   valid data exits in the directory..
 */

#ifndef  _H_BBDIR
#define  _H_BBDIR

#include <sys/hd_psn.h>		/* PSN layout of the reserved space on disk */

#define  BB_DIR_ID   "DEFECT"   /* Value for id field of bad block directory*/
#define  BB_DIR_UP   "UPDATE"   /* Value for id field of bad block directory*/

/*
 * This structure is use both as a header record and a trailer
 * record in the bad block directory.
 */
struct bb_hdr
{
       char id [6];		/* contains either "DEFECT" or "UPDATE"	    */
       short num_entries;	/* number of bad block entries in directory */
};


/*
 * This structure represents an entry in the bad block directory
 */
struct bb_entry
{
    unsigned reason	:  4;	/* reason the block was marked bad	*/
    unsigned bb_lsn	: 28;	/* the logical sector number of the	*/
				/* bad block				*/
    unsigned rel_stat	:  4;	/* relocation status, where a 1 in the	*/
				/* most significant bit indicates the	*/
				/* bad block needs to be relocated	*/
    unsigned rel_lsn	: 28;	/* the logical sector number of the	*/
				/* relocated block			*/
};

/*
 * defines values for the reason field of the bb_entry structure
 */
#define  BB_PVMNFCTR   0x0   /* bad block found by PV manufacturer	*/
#define  BB_DIAGTST    0xA   /* found by surface verification diag test	*/
#define  BB_SYSTEM     0xB   /* found by the system			*/
#define  BB_MNFCTRTST  0xC   /* found by manufacturing test		*/

/* Maximum number of bad block entries in bad block directory */
#define MAX_BBENTRIES (((LEN_BB_DIR * DBSIZE) - (sizeof(struct bb_hdr))) \
			/ sizeof(struct bb_entry))

/* block size to increase pinned bad block directory memory when low */
#define BB_DIR_PIN_INCRMNT 4096

/* actual size of unpinned, kernel_heap memory allocated to bad block dir;
   lowest multiple of pages larger than (LEN_BB_DIR * DBSIZE) */
#define UNPINNED_BBDIR_SIZE 12288

#endif /* _H_BBDIR */
