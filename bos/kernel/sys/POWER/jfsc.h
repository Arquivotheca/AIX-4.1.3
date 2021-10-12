/* @(#)93	1.1  src/bos/kernel/sys/POWER/jfsc.h, sysxjfsc, bos411, 9428A410j 3/29/94 17:40:21 */
/*
 * COMPONENT_NAME: (SYSXJFSC) - JFS Compression
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_JFSC
#define _H_JFSC

/*
 * Data structures and defines for software data compression 
 */
struct hentry {
	short pos;
	short next;
	unsigned int symchar;
};

extern int (*compp)(int, int, caddr_t, size_t, caddr_t, size_t);
extern int encode_decode(int, int, caddr_t, size_t, caddr_t, size_t);
extern int lzdecode(caddr_t, caddr_t, size_t);
extern int lzencode(caddr_t, caddr_t, caddr_t, short *, struct hentry *,
		unsigned int []);


/*
 * compp is an exported symbol.  If the compression kernel extension
 * is available, compp will be filled in to point to the entry point
 * for compression and decompression.  Otherwise it will be null.
 */
extern int (*compp)(
	int     op,             /* compress, decompress, or query       */
	int     type,           /* compression algorithm                */
	caddr_t inbuf,          /* input buffer address                 */
	size_t  inlength,       /* input buffer length                  */
	caddr_t outbuf,         /* output buffer address                */
	size_t  outlength);     /* output buffer length                 */

#define COMP_ENCODE     0x0000  /* Compression operation types          */
#define COMP_DECODE     0x0001
#define COMP_QUERY      0x0002

/*
 * Structure used by callers to the user level portion of the compression
 * extension.  This structure is filled in by the getcompent() routine.
 */
struct compmethod
{
	char    *name;                  /* Compression name               */
	int     type;                   /* Compression type               */
	struct  compmethod *next;       /* linked list of algorithms      */
};

#define LZ              0x0001          /* LZ compression algorithm      */

#endif /* _H_JFSC */
