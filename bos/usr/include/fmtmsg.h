/* @(#)91	1.1  src/bos/usr/include/fmtmsg.h, libcfmt, bos411, 9428A410j 3/4/94 11:05:13 */
/*
 *   COMPONENT_NAME: libcfmt
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 85
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 *  Major Classifications
 */
#define MM_HARD		0x00000001
#define MM_SOFT		0x00000002
#define MM_FIRM		0x00000004
/*
 *  Message Source Sub-classifications
 */
#define MM_APPL		0x00000100
#define MM_UTIL 	0x00000200
#define MM_OPSYS	0x00000400
/*
 *  Display Sub-classifications
 */
#define MM_PRINT	0x00010000
#define MM_CONSOLE	0x00020000
/*
 *  Status sub-classifications
 */
#define MM_RECOVER	0x01000000
#define MM_NRECOV	0x02000000
/*
 *  Severity 
 */
#define MM_NOSEV	0
#define MM_HALT		1
#define MM_ERROR	2
#define MM_WARNING	3
#define MM_INFO		4

/*
 *  Exit Codes
 */
#define MM_OK		1
#define MM_NOTOK	2
#define MM_NOMSG	3
#define MM_NOCON	4
/*
 *  Null Values
 */
#define MM_NULLLBL	(char *)NULL	/* label */
#define MM_NULLSEV	(int)0 		/* severity */ 
#define MM_NULLMC	(long)0		/* classification */
#define MM_NULLTXT	(char *)NULL	/* text */
#define MM_NULLACT	(char *)NULL	/* action */
#define MM_NULLTAG	(char *)NULL	/* tag */


extern int fmtmsg( long, const char *, int, const char *, const char *, const char *);
