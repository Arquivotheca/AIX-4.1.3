/* @(#)46	1.5  src/bos/usr/include/jfs/log.h, syspfs, bos411, 9428A410k 7/14/94 10:39:19 */
/*
 * SYSPFS: journalled filesystem
 *
 * FUNCTIONS: log.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_JFS_LOG
#define _H_JFS_LOG	

#include <sys/types.h>

/* layout of xix file system log. there is one log per logical
 * volume group. a log is implemented with a logical volume   
 * (mini-disk in BDS). For kernel processing a log is mapped
 * into virtual memory as a segment (limits size of log to 256
 * megabytes). when a logical volume group is varied-on-line
 * the program logredo must be executed before the file systems
 * (logical volumes) in the volume group can be mounted. logredo
 * does not map the log into virtual memory.
 */

/* 
 * block 0 of the log logical volume is not used (ipl etc).
 * block 1 contains a log "superblock" and is used by logredo
 * and loginit and logformat to record status info about the
 * log but is not otherwise used during normal processing. 
 * blocks 2 - N are used to contain log records.
 */

/* a log is used to make the commit operation on journalled 
 * files within the same logical volume group atomic. xix uses
 * only "after" log records ( only a single writer is allowed
 * in a  page, pages are written to temporary paging space if
 * if they must be written to disk before commit, and i/o is
 * scheduled for modified pages to their home location after
 * the log records containing the after values and the commit 
 * record is written to the log on disk, undo discards the copy
 * in main-memory.)
 */

/*
** log logical volumes start with this standard prefix
** post-AIX3.1, this will be determined from an ODM query
** as will the log type 
*/
# define	LOGNAME_PREFIX	"log"
# define        LOGTYPE_AIX3    "aix3log"
    
/* log superblock. block 1 of logical volume
 */
# define	LOGSUPER_B	1
# define	LOGSTART_B	2
# define	LOGMAGIC	0x12345678
# define	LOGMAGICV4	0x87654321
# define	LOGVERSION	1

#define		LOGNOTREDONE	0
#define 	LOGREDONE	1
#define 	LOGREADERR	2

struct logsuper {
	int	magic;	 /* identifies the volume as a log */
	int     version; /* version number of log code.   */
	int	size;    /* number of PAGESIZE blocks in log volume */
	int	redone;  /* set to one if logredo was run ok.
		          * set to zero by loginit (during mount).              
			  * set to one if everything unmounted ok.
			  * set to two if log read error detected
			  * in logredo.
			  */
	int     logend;  /* set to address last log record by logredo. */
	int	serial;  /* incremented each time log opened (mount)  */
	uint	active[8];  /* bit vector of active filesystems       */
	int     extra[PAGESIZE/4 - 14];       
};				

/* Format of a log page: the header and trailer structures (h,t)
 * will normally have the same page and eor value.  An exception 
 * to this occurs when a complete page write is not accomplished 
 * on a power failure.  Since the hardware may "split write" sectors 
 * in the page, any out of order sequence may occur during powerfail 
 * and needs to be recognized during log replay.  The xor value is
 * an "exclusive or" of all log words in the page up to eor.  This
 * 32 bit eor is stored with the top 16 bits in the header and the
 * bottom 16 bits in the trailer.  logredo can easily recognize pages
 * that were not completed by reconstructing this eor and checking 
 * the log page.  
 *
 * Previous versions of the operating system did not allow split 
 * writes and detected partially written records in logredo by 
 * ordering the updates to the header, trailer, and the move of data 
 * into the logdata area.  The order: (1) data is moved (2) header 
 * is updated (3) trailer is updated.  In logredo, when the header 
 * differed from the trailer, the header and trailer were reconciled 
 * as follows: if h.page != t.page they were set to the smaller of 
 * the two and h.eor and t.eor set to 8 (i.e. empty page). if (only) 
 * h.eor != t.eor they were set to the smaller of their two values.
 */
	 
struct logpage {
	struct { 
        	int	page;  /* page number of log  	*/
		short	xor;   /* redundancy check high order short 	*/
	        short   eor;   /* offset in page of byte past the last 	*/
		   	       /* record which ends on this page.      	*/  
               } h;
	int     ldata[PAGESIZE/4 - 4];  /* place for log records */
	struct {
	        int     page;  /*  normally the same as h.page 	*/
		short	xor;   /*  redundancy check low order short */
	        short   eor;   /*  normally the same as h.eor  	*/
               } t;
};
 
/* log records. log records consist of a variable length data area 
 * followed by a fixed size descriptor  of 32 bytes. the  data      
 * area is rounded  up to an integral number of words and must be no  
 * longer than PAGESIZE. records are packed one after the other in
 * the data area of log pages. the longest record is placed on at
 * most two pages (sometimes a DUMMY recorded is inserted to make
 * this the case). descriptors are aligned on a word boundary and
 * the field h.eor points to the byte following the last record
 * on a page (sometimes a dummy record is inserted so that at least
 * one record ends on every page).
 */

/* log record types */
# define COMMIT   1
# define AFTER    2
# define NEWPAGE  4
# define NOREDO   8
# define SYNCPT  16 
# define DUM	 32 
# define NODREDO 64
# define DFREE   128
# define MOUNT   256
# define SINGLEIND 0
# define DOUBLEIND 1

struct logrdesc {
	int	transid;	/* transaction identifier */
	int     backchain;	/* ptr to prev log record same transaction */
	short   type;		/* log record type */
	short   length;		/* length in bytes of data in record */
	union {
		int ddata[5];	/* data that can be put in descriptor */

		struct {			/* mount  record */
			int  volid;		/* logical volume id */
		 } mnt;

		struct {			/* after record */
			int  volid;		/* logical volume id */
			ino_t inode;		/* disk inode number */
			int  disk;		/* disk block number in volid */
			int  psaddr;		/* offset in segment */
			int  indtype;		/* single/double indirect */
		 } aft;

		/* new page. zero page before applying after records
		 *
		 * WARNING: layout of aft and new must be the same.
		 */
		struct {
			int volid;		/* logical volume id */
			ino_t inode;		/* disk inode number */
			int disk;		/* disk block number in volid */
			int psaddr;		/* offset of page in segment */
                } new;

		/* no redo. do not apply after records to any pages in
		 * segment volid.inode which precede this record in log.
		 */
		struct {
			int volid;		/* logical volume id */
			ino_t inode;		/* disk inode number */
		} nodo;

		/* no redo. do not apply after records to the disk block
		 * for any records which precede this record in log.
		 *
		 * WARNING: layout must be same as newpage.
		 */
		struct {
			int volid;		/* logical volume id */
			ino_t inode;		/* disk inode number */
			int disk;		/* disk block number in volid */
		} nodisk;

		/* map free record. inode should be DISKMAP_I and  
		 * nblocks should be length of data/4.
		 */
		struct {
			int volid;		/* logical volume id */
			ino_t inode;		/* inode number of map */
			int nblocks;		/* number of blocks to free */
		} dfree;

		int    sync;			/* addr to go back to (0 = here)
						 */
           } log;
};  


/* maximum size of loginfo data.
 */
#define MAXLINFOLEN	256

/* loginfo structure.  
 */
struct loginfo {
        char    *li_buf;
        uint    li_len;
};

#endif	/* _H_JFS_LOG  */
