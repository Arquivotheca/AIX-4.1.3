/* @(#)36	1.14.2.3  src/bos/usr/include/dumprestor.h, cmdarch, bos411, 9428A410j 4/6/93 08:31:40 */
/*
 * COMPONENT_NAME: (CMDARCH) Archival Commands
 *                                                                    
 * FUNCTIONS: Include file for backup and restore functions
 *
 * ORIGINS: 3, 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifndef _H_DUMPRESTOR
#define _H_DUMPRESTOR 

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      (#)dumprestore.h       5.1 (Berkeley) 6/5/85
 */

/*
 * TP_BSIZE is the size of file blocks on the dump tapes.
 * Note that TP_BSIZE must be a multiple of DEV_BSIZE.
 *
 * NTREC is the number of TP_BSIZE blocks that are written
 * in each tape record. HIGHDENSITYTREC is the number of
 * TP_BSIZE blocks that are written in each tape record on
 * 6250 BPI or higher density tapes.
 *
 * TP_NINDIR is the number of indirect pointers in a TS_INODE
 * or TS_ADDR record. Note that it must be a power of two.
 */
#define TP_BSIZE	1024
#define NTREC   	10
#define HIGHDENSITYTREC	32
#define TP_NINDIR	(TP_BSIZE/2)

#define TS_TAPE 	1
#define TS_INODE	2
#define TS_BITS 	3
#define TS_ADDR 	4
#define TS_END  	5
#define TS_CLRI 	6
#define TS_ACL  	7
#define TS_PCL  	8
#define OFS_MAGIC   	(int)60011
#define NFS_MAGIC   	(int)60012
#define MAGIC           (int)60011   /* magic number for headers      */
#define PACKED_MAGIC    (int)60012   /* magic # for Huffman packed format */
#define XIX_MAGIC	(int)60013   /* magic number for AIX v3 */
#define CHECKSUM	(int)84446

#define BSD_NDADDR	12
#define BSD_NIADDR	3

struct 	icommon
{
	u_short	ic_mode;	/*  0: mode and type of file */
	short	ic_nlink;	/*  2: number of links to file */
	ushort	ic_uid;		/*  4: owner's user id */
	ushort	ic_gid;		/*  6: owner's group id */
	quad	ic_size;	/*  8: number of bytes in file */
	time_t	ic_atime;	/* 16: time last accessed */
	long	ic_atspare;
	time_t	ic_mtime;	/* 24: time last modified */
	long	ic_mtspare;
	time_t	ic_ctime;	/* 32: last time inode changed */
	long	ic_ctspare;
	daddr_t	ic_db[BSD_NDADDR];	/* 40: disk block addresses */
	daddr_t	ic_ib[BSD_NIADDR];	/* 88: indirect blocks */
	long	ic_flags;	/* 100: status, currently unused */
	long	ic_blocks;	/* 104: blocks actually held */
	long	ic_spare[5];	/* 108: reserved, currently unused */
};

struct bsd_dinode {
	union {
		struct	icommon di_icom;
		char	di_size[128];
	} di_un;
};

union u_spcl {
	char dummy[TP_BSIZE];
	struct	s_spcl {
		int	c_type;
		time_t	c_date;
		time_t	c_ddate;
		int	c_volume;
		daddr_t	c_tapea;
		u_long	c_inumber;
		int	c_magic;
		int	c_checksum;
		struct	bsd_dinode	bsd_c_dinode;
		int	c_count;
		char	c_addr[TP_NINDIR];
		int	xix_flag;
		struct	dinode	xix_dinode;
	} s_spcl;
} u_spcl;

#define spcl u_spcl.s_spcl
#define bsd_di_ic		di_un.di_icom
#define	bsd_di_mode		bsd_di_ic.ic_mode
#define	bsd_di_nlink		bsd_di_ic.ic_nlink
#define	bsd_di_uid		bsd_di_ic.ic_uid
#define	bsd_di_gid		bsd_di_ic.ic_gid
#define	bsd_di_db		bsd_di_ic.ic_db
#define	bsd_di_ib		bsd_di_ic.ic_ib
#define	bsd_di_atime		bsd_di_ic.ic_atime
#define	bsd_di_mtime		bsd_di_ic.ic_mtime
#define	bsd_di_ctime		bsd_di_ic.ic_ctime
#define	bsd_di_rdev		bsd_di_ic.ic_db[0]
#define	bsd_di_blocks		bsd_di_ic.ic_blocks
#define bsd_di_size             bsd_di_ic.ic_size.val[1]


#define	DUMPOUTFMT	"%s %c %s"		/* for printf */
						/* name, incno, ctime(date) */
#define	DUMPINFMT	"%s %c %[^\n]\n"	/* inverse for scanf */


/*
 * macros for accessing bitmaps
 *      MWORD( map, bitno )     returns word containing specified bit
 *      MBIT(i)                 returns mask for specified bit within map word
 *      BIS                     turns on specified bit in map
 *      BIC                     turns off specified bit in map
 *      BIT                     tests specified bit in map
 */
#define	MWORD(m,i)	(m[(unsigned)(i-1)/NBBY])
#define	MBIT(i)		(1<<((unsigned)(i-1)%NBBY))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

/* Following definitions are from the version 2 backup.h include file */
/*
 *      bitmap parameters.
 *      note: 8K * 8 == 64K, largest possible inumber
 */
#define MSIZ            (8*1024)        /* number of words in map        */
#define DEV_T ushort
/*
 * Because of the end-around copying done in readtape, buffers
 * returned by readtape must be no greater than RDTMAX bytes
 */
#define RDTMAX BSIZE	/* maximum tape read length */

/*
 * format of inode dump
 *      FS_VOLUME
 *      FS_CLRI         (if incremental)
 *              list of inodes unallocated at time of dump
 *
 *      FS_BITS         (just before the first FS_INODE header on each vol.)
 *              list of files on this and succeeding volumes
 *
 *      FS_FINDEX
 *              index of files on this volume.  the last file or two
 *              may not be indexed, for space reasons.  the link field
 *              gives the address of the next FS_INDEX on this volume.
 *
 *      FS_DINODE        (before each file)
 *      file data
 *      FS_END or FS_VOLEND
 *
 * format of name dump:
 *      FS_VOLUME
 *      FS_NAME         (before each file)
 *      file data
 *      FS_END
 *
 */

/*
 * the file /etc/dumpdates contains a record for the last dump of each file
 *     system at each level.  This file is used to determine how far back
 *     new dumps should extend.  The record format is ...
 */
struct	idates
{
	char	id_name[MAXNAMLEN+3];
	char	id_incno;
	time_t	id_ddate;
};

/*
 * header types.  the hdrlen[] array (dump and restor) assumes that
 * the numbers begin at 0, and that they are in this order.
 * the OINODE and ONAME records are not produced by dump, but were
 * produced by older versions, and restore knows how to interpret
 * them.
 */
#define FS_VOLUME        0
#define FS_FINDEX        1
#define FS_CLRI          2
#define FS_BITS          3
#define FS_OINODE        4
#define FS_ONAME         5
#define FS_VOLEND        6
#define FS_END           7
#define FS_DINODE        8
#define FS_NAME          9
#define FS_DS           10
#define FS_NAME_X       11


/* other constants */
#define FXLEN          80       /* length of file index */

/* commands to findex */
#define TBEG    0               /* start indexing */
#define TEND    1               /* end of this track */
#define TINS    2               /* install new inode in index */

/*
 * the addressing unit is 8-byte "words", also known as dwords
 */
#define BPW     8
#define LOGBPW  3
typedef struct {char x[BPW];} dword;

/* bytes to "words" and back; must fit into char */
/* must be even -- so always room for VOLEND record (8 bytes long) */
#define btow(x)   ( ((x) + BPW - 1) >> LOGBPW)
#define wtob(x)   ( (x) << LOGBPW )

long	XXlong();
short	XXshort();
#define rlong(a)    ((a) = XXlong((a),0))
#define wlong(a)    ((a) = XXlong((a),0))
#define rshort(a)   ((a) = XXshort((a),0))
#define wshort(a)   ((a) = XXshort((a),0))

#define SIZHDR    btow(sizeof(struct hdr))
#define DUMNAME   4     /* dummy name length for FS_NAME */
#define SIZSTR   16     /* size of strings in the volume record */

/* D41487: In order to allow all valid path names the param.h file has */
/* been included and NAMESZ has been given the value PATH_MAX + 1.       */

#include <sys/param.h>
#define NAMESZ	PATH_MAX+1	/* internal name string sizes */

#define min(a,b)  ( ((a) < (b))? (a): (b) )
#define max(a,b)  ( ((a) > (b))? (a): (b) )

#define BYNAME  100             /* must be illegal v.incno */
#define BYINODE 101             /*    "       "       "    */
#define BYMD    102             /*    "       "       "    */

#define DONTCOUNT   -1          /* for counting # files we want */

/*
 * the headers follow.  note that there are no places that might
 * tempt a compiler to insert gaps for alignment.  for example,
 * making the FS_FINDEX arrays into an array of (inode, address)
 * structs might later cause trouble.  also, there is code in
 * both dump and restor that reorders the bytes in these headers;
 * this code MUST know about any change in the structures.
 */

struct hdr {                    /* common part of every header */
	unsigned char   len;    /* hdr length in dwords */
	unsigned char   type;   /* FS_* */
	ushort  magic;          /* magic number (MAGIC above) */
	ushort  checksum;
};

union fs_rec {

	/* common fields */
	struct hdr h;

	/* FS_VOLUME -- begins each volume */
	struct {
		struct  hdr h;
		ushort  volnum;         /* volume number */
		time_t  date;           /* current date */
		time_t  dumpdate;          /* starting date */
		daddr_t numwds;         /* number of wds this volume */
		char    disk[SIZSTR];   /* name of disk */
		char    fsname[SIZSTR]; /* name of file system */
		char    user[SIZSTR];   /* name of user */
		short   incno;          /* dump level (or BYNAME) */
	} v;

	/* FS_FINDEX -- indexes files on this volume */
	struct {
		struct  hdr h;
		ushort  dummy;          /* get the alignment right */
		ushort  ino[FXLEN];     /* inumbers */
		daddr_t addr[FXLEN];    /* addresses */
		daddr_t link;           /* next volume record */
	} x;

	/* FS_BITS or FS_CLRI */
	struct {
		struct hdr h;
		ushort  nwds;           /* number of words of bits */
	} b;

	/* FS_OINODE */
	struct {
		struct hdr h;
		ushort  ino;            /* inumber */
		ushort  mode;           /* info from inode */
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		DEV_T   dev;            /* device file is on */
		DEV_T   rdev;           /* maj/min devno */
		off_t   dsize;          /* dump size if packed */
	} oi;

	/* FS_INODE and FS_DINODE */
	struct {
		struct hdr h;
		ushort  ino;            /* inumber */
		ushort  mode;           /* info from inode */
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		ushort  devmaj;         /* device file is on */
		ushort  devmin;
		ushort  rdevmaj;        /* maj/min devno */
		ushort  rdevmin;
		off_t   dsize;          /* dump size if packed */
		long    pad;
	} i;

	/* FS_ONAME */
	/* must be exactly like FS_INODE except name at end */
	struct {
		struct hdr h;
		ushort  ino;
		ushort  mode;
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		DEV_T   dev;
		DEV_T   rdev;
		off_t   dsize;
		char    name[DUMNAME];  /* file name given by user */
	} on;

	/* FS_NAME */
	/* must be exactly like FS_INODE except name at end */
	struct {
		struct hdr h;
		ushort  ino;
		ushort  mode;
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		ushort  devmaj;         /* device file is on */
		ushort  devmin;
		ushort  rdevmaj;        /* maj/min devno */
		ushort  rdevmin;
		off_t   dsize;
		long    pad;
		char    name[DUMNAME];  /* file name given by user */
	} n;

	/* FS_NAME_X */
	/* just like FS_NAME except most fields changed in 
	 * Version 3.(from ushort to unsigned long)
	 */
	struct {
		struct hdr h;
		nlink_t  nlink;
		ino_t  ino;
		mode_t  mode;
		uid_t  uid;
		gid_t  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		dev_t  devmaj;         /* device file is on */
		dev_t  devmin;
		dev_t  rdevmaj;        /* maj/min devno */
		dev_t  rdevmin;
		off_t   dsize;
		long	pad;
		char    name[DUMNAME];  /* file name given by user */
	} nx;

	/* FS_END or FS_VOLEND */
	struct {
		struct hdr h;
	} e;


	/* Obsolete format, needed to decipher some version 2 by name
	   backups */
	/* FS_DS */
	struct {
		struct hdr h;
		char	nid[8];
		char	qdir[2];	/* makes it 2 dwords */
	} ds;

};

/* security header: indicates size of acl and pcl headers which immediately follow it */
struct sac_rec {
		ulong aclsize; 		/* size of acl in double words */
		ulong pclsize;       /* size of pcl in double words */
};

/* output device info */

#define DEF_LEV		'9'	/* default dump level	*/
#define A_WHILE		150	/* period between informative comments	*/
#define NTRACKS		1	/* default number of tracks per tape unit */

#define ROOT_FILSYS     "/dev/hd0"
#define DEF_FILSYS      ROOT_FILSYS
#define DEF_MEDIUM	"/dev/rfd0"
#define DEF_DTYP	DD_DISK
#define DEF_CLUSTER	100	/* default tape cluster in 512-byte units */
#define DEF_TLEN        4500    /* default tape length in feet */
#define DEF_TDEN        700     /* default tape density in bytes/inch */
#define IRG             1       /* tape inter-record gap in inches */

#endif /* _H_DUMPRESTOR */
