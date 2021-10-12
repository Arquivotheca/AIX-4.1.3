/* @(#)61	1.1.1.1  src/bos/sbin/helpers/v3fshelpers/bufpool.h, cmdfs, bos411, 9428A410j 2/1/93 11:12:27 */
/*
 * CMDFS: file system commands
 *
 * FUNCTIONS: bufpool.h: bufpool.c common header file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * this file contains defines and declarations needed to use the buffer
 * pool routines in bufpool.c
 *
 * -> _NO_PROTO should be defined if we don't want prototyping...
 * -> BPDEBUG should be defined if you want megs of debug output...
 */

/*
 * turn buffer pool debugging on/off
 */
#undef	BPDEBUG

/*
 * functions in bufpool.c
 */
#ifndef	_NO_PROTO

extern void  bpinit(int, void (*)());	/* init function		*/
extern void *bpread(frag_t);		/* actual bread function	*/
extern void  bptouch(void *);		/* touch function		*/
extern void  bprelease(void *);		/* release function		*/
extern void  bpflush(void *);		/* flush function		*/
extern void  bpassert(char *, void *, int); /* assert active buffer	*/

#else	/* _NO_PROTO */

#define	void	char

extern void  bpinit();			/* init function		*/
extern void *bpread();			/* actual bread function	*/
extern void  bptouch();			/* touch function		*/
extern void  bprelease();		/* release function		*/
extern void  bpflush();			/* flush function		*/
extern void  bpassert();		/* assert active buffer		*/

#endif	/* _NO_PROTO */

extern void  bpclose();			/* close down buffer pool	*/

extern int   Bp_touched;		/* was bptouch ever called?	*/

/*
 * buffer pool debug stuff
 */

#ifdef	BPDEBUG
# define	BPPRINTF(args)		printf args 
#else	/* BPDEBUG */
# define	BPPRINTF(args)
#endif	/* BPDEBUG */

