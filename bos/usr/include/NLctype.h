/* @(#)43	1.17.1.2  src/bos/usr/include/NLctype.h, libcnls, bos411, 9428A410j 1/22/93 13:21:35 */
#ifndef _H_NLCTYPE 
#define _H_NLCTYPE 
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 */

#include <NLchar.h>
#include <ctype.h>
#include <jctype.h>    

#define	_U	   01
#define	_L	   02
#define	_N	   04
#define	_S	  010
#define	_P	  020
#define	_C	  040
#define	_B	 0100
#define	_X	 0200
#define _A       0400
#define _G      01000


/*  Ctype definitions for use with setlocale (loc_t->lc_ctype) table.
 *
 *  NOTE:
 *  Applications using these macros may not be binary compatible from one
 *  release to the next.
 */
#	define _NCtoupper(a)	((__OBJ_DATA(__lc_charmap)->cm_mb_cur_min && \
				  __OBJ_DATA(__lc_charmap)->cm_mb_cur_max) ? \
				      __OBJ_DATA(__lc_ctype)->upper[a] : a)
#	define _NCtolower(a)	((__OBJ_DATA(__lc_charmap)->cm_mb_cur_min && \
				  __OBJ_DATA(__lc_charmap)->cm_mb_cur_max) ? \
				      __OBJ_DATA(__lc_ctype)->lower[a] : a)
#define _atojis(c)	(_atojistab[(c) - 0x20])
#define _jistoa(c)	(_jistoatab[((c)>>8) - 0x81][((c)&0xff) - 0x40])

/* The following macros implement character "flattening". This feature
 * may not be portable to future releases of AIX.
 */

/*  Macros with no old equivalents.
 */
extern char _NLflattab[];
extern unsigned _NLflattsize;
#define	NCflatchr(c)	(((unsigned)(c) < 0x80) ? \
				(c) : ((unsigned)(c) - 0x80 < _NLflattsize) ? \
				_NLflattab[c - 0x80] : '?')
/*  Information-preserving escape sequence definitions:
 */
#define NLESCMAX	7	/* Maximum length of a sequence */
/* if the range of mnemonic escape sequences changes in NLesctab.c, MINESCVAL
 * and MAXESCVAL should be revised
 */
#define MINESCVAL	0xa1	/* Minimun NLchar value mapped in NLesctab */
#define MAXESCVAL	0xdf	/* Maximum NLchar value mapped in NLesctab */

/*  Map from code point to escape sequence.
 */

struct NLescdata {
	unsigned char *key;
	NLchar value;
} ;
/**********
  SINGLE BYTE TABLES
**********/
extern char _NLesctab[][2];
extern struct NLescdata _NLunesctab[];

extern unsigned _NLesctsize;
extern unsigned _NLunesctsize;

/**********
  MULTI_BYTE TABLES
**********/
extern struct NLescdata _NLesctab_932[];
extern struct NLescdata _NLunesctab_932[];

extern unsigned _NLesctsize_932;
extern unsigned _NLunesctsize_932;

/*  Translate single NLchar at nlc to char escape string at c.
 */
#define hextoa(c)       (((c) < 10) ? ('0' + (c)) : ('a' + ((c) - 10)))
#define atohex(c)       (((c) <= '9') ? ((c) - '0') : (((c) - 'a') + 10))
/*  Translate single NLchar at nlc to char escape string at c.
 */
#define NCeschex(nlc, c)    ((c)[0] = '\\', \
			    (c)[1] = '<', \
                            (c)[2] = hextoa ((*(nlc) >> 12) & 0xf), \
                            (c)[3] = hextoa ((*(nlc) >> 8) & 0xf), \
                            (c)[4] = hextoa ((*(nlc) >> 4) & 0xf), \
                            (c)[5] = hextoa (*(nlc) & 0xf), \
			    (c)[6] = '>')

/*  Translate hex escape string at c to single NLchar at nlc.
 */
#define NCuneschex(c, nlc)  ((nlc)[0] = (((atohex((c)[2])) & 0xf) << 12) | \
                                        (((atohex((c)[3])) & 0xf) << 8) | \
                                        (((atohex((c)[4])) & 0xf) << 4) | \
                                        ((atohex((c)[5])) & 0xf))
                                         
#define ishexesc(c)	    (((((c)[0] >= 'a' && (c)[0] <= 'f') || \
					((c)[0] >= '0' && (c)[0] <= '9')) && \
				(((c)[1] >= 'a' && (c)[1] <= 'f') || \
					((c)[1] >= '0' && (c)[1] <= '9')) && \
				(((c)[2] >= 'a' && (c)[2] <= 'f') || \
					((c)[2] >= '0' && (c)[2] <= '9')) && \
				(((c)[3] >= 'a' && (c)[3] <= 'f') || \
					((c)[3] >= '0' && (c)[3] <= '9'))) ? 1 : -1)
#endif
