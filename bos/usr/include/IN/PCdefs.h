/* @(#)53	1.10  src/bos/usr/include/IN/PCdefs.h, libIN, bos411, 9428A410j 6/16/90 00:17:26 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef	_H_PCDEFS
#define	_H_PCDEFS
/*
 * I suppose we must really start at the basics.
 * Here are the basic types:
 */
typedef struct { char c[4]; }	pclong;
typedef short			pcint;
typedef short			pcshort;
typedef char			pcchar;

/*
 * Here are some simple derived types.
 */
typedef	pclong		pcdaddr_t;
typedef	pcshort		pcushort;
typedef	pcushort	pcino_t;
typedef	pclong		pctime_t;
typedef	pclong		pcoff_t;

/* These haven't been needed yet.  Define them when you will. */
/*	typedef	struct { int r[1]; } *	physadr; 	*/
/*	typedef	char *		caddr_t;		*/
/*	typedef	short		dev_t;			*/
/*	typedef	pclong		pcpaddr_t;		*/
/*	typedef char		cnt_t;			*/
/*	typedef int             label_t[5];		*/

/*
 * Now that we have the types, let us try to convert some of them.
 */
/*
 * First, some ways to convert basic PC types to
 * basic aiws types.
 */

#define	PCchar(X)	(X)
#define	PCshort(X)	(((X>>8)&0x00ff)|((X<<8)&0xff00))
extern long PClongin();
extern pclong PClongout();

/*
 * Next, some ways to convert -typedef-ed types
 * to AIWS types.
 */

#define	PCushort(X)	PCshort(X)
#define	PCino(X)	PCushort(X)
#define	PCdaddrin(X)	PClongin(X)
#define	PCdaddrout(X)	PClongout(X)
#define	PCtimein(X)	PClongin(X)
#define	PCtimeout(X)	PClongout(X)
#define	PCoffin(X)	PClongin(X)
#define	PCoffout(X)	PClongout(X)

/*
 * Useful information we all need.
 */
#define	PCNICFREE	50
#define	PCNICINOD	100
#define	PCBSIZE		512
#define	PCISIZE		64		/* same as sizeof (struct dinode) */
#define	PCBSHIFT	9
#define	PCBNSHIFT	7

#define	PCINOPB		8
#define	PCINOSHIFT	3

#define	PCitod(x)	(daddr_t)(((unsigned)(x)+(2*PCINOPB-1))>>PCINOSHIFT)
#define PCitoo(x)	(int)(((unsigned)(x)+(2*PCINOPB-1))&(PCINOPB-1))

/* pcfblk has a different definition. */

struct	pcfblk
{
	pcint	pc_nfree;
	pcdaddr_t	pc_free[PCNICFREE];
};
/*
 * Structure of the super-block
 */


struct	pcfilsys
{
	pcushort	pc_isize;	/* size in blocks of i-list */
	pcdaddr_t	pc_fsize;	/* size in blocks of entire volume */
	pcshort		pc_nfree;	/* number of addresses in s_free */
	pcdaddr_t	pc_free[PCNICFREE];	/* free block list */
	pcshort		pc_ninode;	/* number of i-nodes in s_inode */
	pcino_t		pc_inode[PCNICINOD];	/* free i-node list */
	pcchar		pc_flock;	/* lock during free list manipulation */
	pcchar		pc_ilock;	/* lock during i-list manipulation */
	pcchar  	pc_fmod; 	/* super block modified flag */
	pcchar		pc_ronly;	/* mounted read-only flag */
	pctime_t	pc_time; 	/* last super block update */
	pcshort		pc_dinfo[4];	/* device information */
	pcdaddr_t	pc_tfree;	/* total free blocks*/
	pcino_t		pc_tinode;	/* total free inodes */
	pcchar		pc_fname[6];	/* file system name */
	pcchar		pc_fpack[6];	/* file system pack name */
	pclong		pc_fill[13];     /* round out to 512 byte boundary */
	pcdaddr_t	pc_swaplo;
	pcdaddr_t	pc_nswap;
	pclong		pc_magic;
	pclong		pc_type;
};

/*
 * macros to give more meaningful names to dinfo fields
 */
#define pc_m     pc_dinfo[0]      /* modulo factor in superblock          */
#define pc_n     pc_dinfo[1]      /* cylinder size in super block         */
#define pc_bsize pc_dinfo[2]      /* block size for this file system      */

# define PCFsMAGIC 0xFD187E20      /* s_magic number */
#endif	/* _H_PCDEFS */
