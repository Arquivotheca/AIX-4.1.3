/* @(#)40	1.4.1.8  src/bos/usr/include/jfs/filsys.h, syspfs, bos411, 9428A410j 1/17/94 16:59:49 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: filsys.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_JFS_FILSYS
#define _H_JFS_FILSYS

/* The following disk-blocks are formatted or reserved:
 *
 * 	  ipl block 0 - not changed by filesystem.
 *
 *	  superblocks at block 1 (primary superblock) and block 31 (secondary
 *	  superblock). the secondary super-block location is likely to be on
 *	  a different disk-surface than the primary super-block. both blocks
 *	  are allocated as blocks in ".superblock".
 *	  
 *	  blocks for .inodes according to BSD layout.  each allocation
 *	  group contains a fixed number of disk inodes.  for fsv3 file
 *	  systems, each allocation group contains one inode per disk block
 *	  of the allocation group, with the number of disk blocks within each
 *	  group described by the s_agsize field of the superblock.  for fsv3p
 *	  file systems, the number of inodes per group is described by the
 *	  s_iagsize field of the superblock and may be less than or greater
 *	  than the number of disk blocks per group.  for these file systems,
 *	  s_agsize describes the number of s_fragsize fragments contained
 *	  within each allocation group.
 *
 *	  the first allocation group starts at block number 32 and consumes
 *	  consecutive blocks sufficient to hold the allocation group's inodes.
 *	  the inode blocks for all other allocation groups start in the first
 *	  block of the allocation group and continue in consecutive blocks
 *	  sufficient to hold the allocation group's inodes.
 *
 *	  other disk-blocks are allocated for .indirect, .diskmap, .inodemap,
 *	  and their indirect blocks starting in the first allocation-group.
 *
 * The special fs inodes formatted and their usage is as follows:
 *
 * 	  inode 0 - never allocated - reserved by setting n_link = 1
 * 	  inode 1 - inode for .superblock
 * 	  inode 2 - inode for root directory
 * 	  inode 3 - inode for .inodes    
 * 	  inode 4 - inode for .indirect
 * 	  inode 5 - inode for .inodemap - allocation map for .inodes
 * 	  inode 6 - inode for .diskmap - disk allocation map 
 * 	  inode 7 - inode for .inodex - inode extensions
 * 	  inode 8 - inode for .inodexmap - allocation map for .inodex
 *	  inodes 9 -16 - reserved for future extensions
 *
 * except for the root directory, the special inodes are not in any
 * directory.
 *
 */

#define IPL_B		0
#define SUPER_B		1
#define SUPER_B1	31
#define INODES_B	32	
#define NON_B		0
#define	SUPER_I   	1
#define ROOTDIR_I	2
#define INODES_I	3
#define	INDIR_I		4
#define INOMAP_I	5
#define DISKMAP_I	6
#define INODEX_I	7
#define INODEXMAP_I	8

#define SPECIAL_I	8	/* largest number for special inode	*/

/*
 * super block format. primary superblock is located in block (PAGE) one.
 * the secondary super-block is not used except for disaster recovery.
 */
struct  superblock
{       
	char    s_magic[4];     /* magic number				  */
	char    s_flag[4];      /* flag word (see below)		  */
	int	s_agsize;	/* fragments per allocation group	  */
	int	s_logserial;	/* serial number of log when fs mounted	  */
	daddr_t s_fsize;        /* size (in 512 bytes) of entire fs	  */
	short	s_bsize;	/* block size in bytes			  */
	short	s_spare;	/* unused.				  */
	char    s_fname[6];     /* name of this file system		  */
	char    s_fpack[6];     /* name of this volume			  */
	dev_t	s_logdev;	/* device address of log		  */

	/* current file system state information, values change over time */
	char    s_fmod;         /* flag: set when file system is mounted  */
	char    s_ronly;        /* flag: file system is read only         */
	time_t  s_time;         /* time of last superblock update         */

	/* more persistent information					  */
	int	s_version;	/* version number			  */
	int	s_fragsize;	/* fragment size in bytes (fsv3p only)	  */
	int	s_iagsize;	/* disk inode per alloc grp (fsv3p only)  */
	int	s_compress;	/* > 0 if data compression  		  */
};

/*
 *  Notes on "s_magic" field:
 *      The superblock's magic number is encoded as a 4-byte character
 *      string to make it possible to validate the superblock without
 *      knowing the byte order of the remaining fields.  To check for
 *      a valid fsv3 superblock, use something like:
 *
 *              if (strncmp(sp->s_magic,fsv3magic,4) == 0)
 *
 *  	For fsv3p file systems, superblock validation is made by checking
 *	both the superblock magic and version number, using a condition
 *	similar to:
 *
 *		if (strncmp(sp->s_magic,fsv3pmagic,4) == 0 &&
 *			sp->s_version == fsv3pvers)
 *
 */


#define fsv3magic  "\103\041\207\145"	/* Version 3 fs magic number	   */
#define fsv3pmagic "\145\207\041\103"	/* Version 3p fs magic number	   */

#define fsv3vers   0			/* Version 3 fs version number	   */
#define fsv3pvers  1			/* Version 3p fs version number	   */

#define s_cpu   s_flag[0]               /* Target cpu type code            */
#define s_type  s_flag[3]               /* File system type code           */


/*
 * Notes on s_fmod field:
 *	This field is intended to be a four state flag 
 *	The four states are:
 *
 *	0 = file system is clean and unmounted
 *	1 = file system is mounted
 *	2 = file system was mounted when dirty or commit failed 
 *	3 = log redo processing attempted but failed.
 *
 *	If you mount the file system in state 0 and unmount it
 *      the state returns to 0 provided no commit failures 
 *      occur.
 *
 *	A mount of the file-system in states 1-3 causes the state
 *	to be set to 2. Also a failure in commit causes the state
 *      to be set to 2.
 *
 *      State 2 is a sticky state in that the only way to reset
 *      the state to zero is to run fsck.
 *
 *	State 3 results when log-redo fails. fsck must be run
 *      to fix it.   
 */
#define FM_CLEAN   00   /* File system is clean and unmounted */
#define FM_MOUNT   01   /* File system is mounted cleanly */
#define FM_MDIRTY  02   /* File system dirty when mounted or commit fail */
#define	FM_LOGREDO 03	/* log redo processing attempted but failed    */

#define	FMOD(x)		((x)==0?1:2)
#define	FCLEAN(x)	((x)==2?2:0)

/*
 * Max size of a v3 filesystem in 512 byte blocks
 */
#define	FS_V3MAXSIZE	4194304L

/*
 * FS_MAXSIZE (frag_size, ag_size, iag_size)
 *
 * FUNCTION
 *	Returns the maximum filesystem size in frag size blocks.
 *	
 *	The jfs is limited to 8 segments for inodes and a maximum
 *	addressability of 2^28 fragments. Thus, filesystem size
 *	is limited to the minimum of 2^28 fragments and 8 segments of inodes.
 *
 *	Inode limitation:
 *	NBPI * (Number of frag size blocks for inodes) =
 * 	NBPI * ((SEG_SIZE * 8 / sizeof(dinode)) / Frag_Size) = 
 * 	NBPI * ((SEG_SIZE * 8 / 2^7) / Frag_Size) = 
 *	NBPI * (2^24 / Frag_Size)	
 *
 *	Largest resultant:	 2^14 * 2^15
 *	
 *	
 *	Addressibility limitation:
 *	2^28 frags
 *
 */
#define	FS_MAXSIZE(fs, ag, iag)	(	\
   (uint)(1 << 28) < (uint)(((fs) * (ag) / (iag)) *  ((1 << 24) / (fs)))  ? \
   (uint)(1 << 28) : (uint)(((fs) * (ag) / (iag)) *  ((1 << 24) / (fs)))   \
				)

#endif /* _H_JFS_FILSYS */



