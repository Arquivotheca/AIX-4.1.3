/* @(#)52	1.31  src/bos/kernel/sys/POWER/seg.h, sysvmm, bos411, 9428A410j 1/20/94 16:19:10 */
/*
 *   COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_SEG
#define _H_SEG

#include <sys/inode.h>

/*
 * Memory management addresses and bits
 */

#ifdef _POWER
		/* extract sid from sreg value */
#define SRTOSID(srval) ( (srval) & 0xffffff )

		/* extract key from sreg value */
#define SRTOKEY(srval) ( ((srval) >> 30) & 0x1 )

/*
 * The SRVAL macro should only be used in the base kernel but the _KERNSYS
 * define wasn't used previously to enforce this.  Uses of SRVAL outside of
 * the base kernel continue to get the old definition.
 */
#ifdef _KERNSYS
extern int key_value;
#define	SRVAL(sid,key,special)	( (sid) | ((key) ? key_value : 0) )
#else
#define SRVAL(sid,key,special)	( (sid) | (key) << 30 )
#endif /* _KERNSYS */

#define INVLSID         0x7fffff        /* generic invalid sid */
#define INVALIDSID(sid)	((sid) == INVLSID || (sid) == RSVDSID)
					/* macro to check invalid sid */
#endif /* _POWER */

#define	NSEGS		16		    /* number of segment regs */
#define SEGSHIFT	28		    /* segment reg shift count */
#define SEGOFFSET(x)	(((int)x)& 0x0fffffff) /* mask to get segment offset */
#define	SEGSIZE		(1<<28)		    /* segment size */
#define NULLSEGID	INVLSID		    /* generic invalid sid */
#define NULLSEGVAL	SRVAL(NULLSEGID,0,0)   /* null segreg value */
#define KERNELSEGID	0x0		    /* kernel segment ID */
#define KERNELSEGVAL	SRVAL(KERNELSEGID,0,0) /* kernel segreg value */

/*
 *  This is a temporary situation until the loader is sorted out.
 */

/*
 *  BASE is lowest addr in seg, TOP is highest, 0x?FFFFFFF
 */
#define BASE(seg) ((seg)<<SEGSHIFT)
#define TOP(seg)  ((BASE((seg)+1))-1)
#define LASTLONG(seg)	((BASE(seg+1))-4)

#define	KERNELORG	0x00000000		/* kernel segment origin */
#define KERNELSEG	0			/* kernel segment register */
#define KSTACKORG	0x10000000		/* extra kstacks seg org */
#define KSTACKSEG	1			/* extra kstacks seg reg */
#define	KERNEXORG	0xE0000000		/* kernel extension seg org */
#define	KERNEXSEG	14			/* kernel extension seg reg */
#define	TEXTORG		0x10000000		/* text segment origin */
#define	TEXTSEG		1			/* text segment register */
   /* NOTE: PRIVSEG should eventually replace DATASEG */
#define PRIVORG 	0x20000000		/* private segment origin */
#define PRIVSEG 	2			/* private segment reg. */
#define	DATAORG		0x20000000		/* data segment origin */
#define	DATASEG		2			/* data segment register */
#define	STACKSEG	2			/* stack segment register */
#define	BDATASEG	3			/* big data segment number */
#define	BDATAORG	0x30000000		/* big data segment origin */
#define	BDATASEGMAX	11			/* max seg # for big data */
#define	TEMPORG		0xC0000000		/* temp segment origin */
#define	TEMPSEG		12			/* temp segment register */
#define	SHTEXTORG	0xD0000000		/* shared text segment origin */
#define	SHTEXTSEG	13			/* shared text seg reg */
#define	IOORG		0xF0000000		/* I/O bus origin */
#define	IOSEG		15			/* I/O segment register */
#define SHDATAORG	0xF0000000		/* shared data segment origin */
#define SHDATASEG	15			/* shared data seg reg */

/*
 *  Masks for creating segments.
 */
#define PRIV_SEG_OPTS	0x80000000	/* process private seg w/ key=R/W */
#define TEXT_SEG_OPTS	0x40000000	/* text seg */

#define READ_ACC    1	/** segment access   **/
#define WRITE_ACC   2


/*
 * segstate structure is used by the shared memory services to mark
 * the ussage of allocated segments.  Segments can be used for mapped
 * files or shared memory.
 *
 *	mapped files - any size file can be mapped, so up to 8
 *		consecutive segment registers can be allocated for
 *		a mapped file.  The first segment will have numsegs
 *		set to the number of segment registers used to map
 *		the file.  All other segments in the mapped file
 *		will have numsegs set to -1.
 *
 *	shared memory - only one shared memory segment can be attatched
 *		to with a shmat call.
 */

struct segstate {
	ushort	segflag;			/* type of segment	*/
	ushort  num_segs;			/* number of segments	*/
	union {
		uint mfileno;			/* file discreptor	*/
		struct shmid_ds *shmptr;	/* shared memory descriptor */
		ulong srval;
	} u_ptrs;
};
#define shm_ptr u_ptrs.shmptr
#define segfileno u_ptrs.mfileno
#define	ss_srval u_ptrs.srval

/*
 * segflag values
 */

#define SEG_AVAIL	0x0000		/* unused segment		*/
#define SEG_SHARED	0x0001		/* shared memory segment	*/
#define SEG_MAPPED	0x0002		/* mapped file			*/
#define SEG_MRDWR	0x0004		/* file mapped read write	*/
#define SEG_DEFER	0x0008		/* defered update		*/
#define SEG_MMAP	0x0010		/* mmap()'d file		*/
#define SEG_WORKING	0x0020		/* working storage		*/


#endif /* _H_SEG */

