/* @(#)38	1.7  src/bos/usr/include/tar.h, cmdarch, bos411, 9428A410j 1/12/93 17:02:03 */

/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TAR
#define _H_TAR

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 * POSIX required that certain values be included in tar.h.  It also requires
 * that if _POSIX_SOURCE is defined then only those standard specific values
 * be present.  This header includes all the POSIX required entries.
 */

#ifdef _POSIX_SOURCE

#define TMAGIC		"ustar"		/* ustar and a null */
#define TMAGLEN		6		  
#define TVERSION	"00"		/* 00 and no null */
#define TVERSLEN	2

/* Values used in typeflag field */
#define REGTYPE		'0'		/* regular file */
#define AREGTYPE	'\0'		/* regular file */
#define LNKTYPE		'1'		/* line         */
#define SYMTYPE		'2'		/* reserved     */
#define CHRTYPE		'3'		/* character special */
#define BLKTYPE 	'4'		/* block special */
#define DIRTYPE		'5'		/* directory */
#define FIFOTYPE	'6'		/* FIFO special */
#define CONTTYPE	'7'		/* reserved */

/* Bits used in the mode field - values in octal */
#define TSUID		04000		/* set UID on execution */
#define TSGID		02000		/* set GID on execution */
#define TSVTX		01000		/* reserved 	   	*/
#define TUREAD		00400		/* read by owner	*/
#define TUWRITE		00200		/* write by owner	*/
#define TUEXEC		00100		/* execute/search by owner */
#define TGREAD		00040		/* read by group 	*/
#define TGWRITE		00020		/* write by group 	*/
#define TGEXEC		00010		/* execute/search by group */
#define TOREAD		00004		/* read by other 	*/
#define TOWRITE		00002		/* write by other 	*/
#define	TOEXEC		00001		/* execute/search by other */

#endif /* _POSIX_SOURCE */
#endif /* _H_TAR */
