/* @(#)26	1.5  src/bos/usr/include/jctype.h, libcnls, bos411, 9428A410j 6/16/90 00:10:51 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Library Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*	jctype - Japanese (SJIS) extensions to ctype
 *						rcd  12-Apr-88
 */

#ifndef _H_JCTYPE
#define _H_JCTYPE

extern unsigned int	_jistoatab[][91];
extern unsigned short	_atojistab[];
#ifdef _KERNEL
#include <sys/jctype0.h>
#else
extern unsigned char	_jctype0_[], _jctype1_[][256];
#endif

#define _tojupper(c)	((c)-0x21)
#define _tojlower(c)	((c)+0x21)

/* Note that encodings are not strictly bit-per-attribute. */
#define	_J1	0xc0		/* field 1: japanese-symbol mask */
#define	_Jk	0x40		/* katakana */
#define	_JH	0x80		/* hiragana */
#define	_JK	0xc0		/* kanji */

#define	_JA	0x10		/* upper case alphabetic */
#define	_Ja	0x20		/* lower case alphabetic */

#define	_J2	0x0c		/* field 2: hex/digit/punct mask */
#define	_JX	0x04		/* hex-digit flag */
#define	_JD	0x0c		/* digit = numeral and hex */
#define	_JB	0x14		/* upper case hex letter */
#define	_Jb	0x24		/* lower case hex letter */

#define	_JP	0x08		/* punct (looks like non-hex numeral) */
#define	_JG	0x02		/* other graphic character */
#define	_JR	0x01		/* reserved (unassigned but valid) char */

#ifndef lint

#define	_jattr(c)	(_jctype1_[_jctype0_[(unsigned short)(c)>>8]][(c)&0xff])
#define	_legaltop(c)	(_jctype0_[c] > 1)
#define	_legalbot(c)	(_jctype1_[7][c] != 0)

#define isj1bytekana(c) (NCchrlen(c) == 1 && isjkata(c))
#endif
#endif
