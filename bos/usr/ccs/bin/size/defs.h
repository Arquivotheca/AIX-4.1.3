/*
 * @(#)64	1.8  src/bos/usr/ccs/bin/size/defs.h, cmdaout, bos411, 9428A410j 8/27/91 15:37:38
 */                                                                   
/*
 * COMPONENT_NAME: CMDAOUT (size command)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	HEX		0
#define	OCTAL		1
#define	DECIMAL		2

#define	TROUBLE		-1L
#define	MAXLEN		512

#define	ERROR_STRING	"%ssize:  %s:  %s\n"
#define SGS_STRING	"%ssize:  %s\n"
#define PORTAR_NAME	"%s[%.16s]"
#define	AIXV3AR_NAME	"%s[%.16s]"
#define OTHERAR_NAME	"%s[%.14s]"
#define	STRING_COLON_1	"%s: "
#define	PRTOTAL_0	"\n\nSize of %s:  0x%lx\n"
#define	PRTOTAL_1	"\n\nSize of %s:  0%lo\n"
#define	PRTOTAL_2	"\n\nSize of %s:  %ld\n"
#define	USAGE_ERR	"Usage: size [-o|-x|-d|-f|-V] [File ...]\n"
#define	OPTION_ERR	"size: 0654-301 %c is not a recognized flag.\n"
#define	AR_HDR_ERROR	"size: 0654-302 Cannot size %s.\n\
\tArchive header is not readable.\n"
#define	BAD_MAGIC	"size: 0654-303 Cannot size %s.\n\
\tSpecify an XCOFF object file or an archive of XCOFF object files.\n"
#define	OPEN_ERR	"size: 0654-304 Cannot open %s.\n"
#define	TOTAL_ERROR	"size: 0654-305 File is corrupted. Cannot determine \
the total size.\n"
#define	SCN_HDR_ERROR	"size: 0654-306 Cannot read a section header.\n"
#define STRING_COLON_0	"%s:"
#define	PAREN_STRING	"(%s)"
#define	PLUS_SIGN	" + "

#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)U802WRMAGIC) || \
			  (((unsigned short)x)==(unsigned short)U802ROMAGIC) ||\
			  (((unsigned short)x)==(unsigned short)U802TOCMAGIC)||\
			  (((unsigned short)x)==(unsigned short)U800WRMAGIC) ||\
			  (((unsigned short)x)==(unsigned short)U800ROMAGIC) ||\
			  (((unsigned short)x)==(unsigned short)U800TOCMAGIC))
#ifdef ARTYPE	
#define ISARCHIVE(x)	((((unsigned short)x)==(unsigned short)ARTYPE))
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#else
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x))
#endif
#define SGS		""
#define SGSNAME		""
