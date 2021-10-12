/* @(#)19       1.10.1.1  src/bos/kernel/sys/POWER/tape.h, sysxtape, bos411, 9428A410j 9/10/93 10:59:32 */
/* @(#)19       1.12  R2/inc/sys/tape.h, sysxtape, bos325 5/28/93 09:38:11 */
#ifndef _H_TAPE
#define  _H_TAPE
/*
 * COMPONENT_NAME: (INCSYS) Magnetic Tape User Include File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/*                                                                      */
/*  NOTE:	This header file contains the definition of the		*/
/*              structures which are used in conjunction with ioctls    */
/*              to execute tape commands.                               */
/*                                                                      */
/************************************************************************/

/* SCSI Tape Ioctls */
#define STIOCTOP        0x01            /* tape commands */
#define STIOCMD         0x02            /* diagnostic commands */
#define STIOCHGP        0x03            /* change drive parameters */

/*
 * Structures and definitions for magnetic tape io control commands
 */

/************************************************************************/
/* structure for STIOCTOP - streaming tape op command                   */
/************************************************************************/
struct  stop    {
	short   st_op;          /* operations defined below */
	daddr_t st_count;       /* how many of them */
};

/* operations */
#define STOFFL	5	/* rewind and unload tape */
#define STREW   6       /* rewind */
#define STERASE 7       /* erase tape, leave at load point */
#define STRETEN 8       /* retension tape, leave at load point */
#define STWEOF  10      /* write an end-of-file record */
#define STFSF   11      /* forward space file */
#define STRSF   12      /* reverse space file */
#define STFSR   13      /* forward space record */
#define STRSR   14      /* reverse space record */
#define STINSRT 15      /* pull tape in from loader */
#define STEJECT 16      /* spit tape out to loader */

/************************************************************************/
/* structure for STIOCHGP - streaming tape change parameters command    */
/************************************************************************/
struct  stchgp  {
	uchar   st_ecc;         /* reserved */
	int     st_blksize;     /* change blocksize to this */
};

/* ecc flags */
#define ST_NOECC   0x00      /* turn off ecc while writing */
#define ST_ECC     0x02      /* turn on ecc while writing  */
#endif  /* _H_TAPE */

/* ext flags */
#define TAPE_SHORT_READ	0x01	/* allow reads shorter than a full	*/
				/* block to be legal (variable length	*/
				/* blocksize only.)			*/
