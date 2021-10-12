/* @(#)10	1.11  src/bos/usr/bin/bsh/shctype.h, cmdbsh, bos411, 9428A410j 3/24/94 18:47:11 */
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.4  com/cmd/sh/sh/shctype.h, cmdsh, bos324
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

/* table 1 */
#define T_SUB	01
#define T_MET	02
#define	T_SPC	04
#define T_DIP	010
#define T_EOF	020
#define T_EOR	040
#define T_QOT	0100
#define T_ESC	0200

/* table 2 */
#define T_BRC	01
#define T_DEF	02
#define T_AST	04
#define	T_DIG	010
#define T_FSH   020
#define T_SHN	040
#define	T_IDC	0100
#define T_SET	0200

/* for single char */
#define _TAB	(T_SPC)
#define _SPC	(T_SPC)
#define _UPC	(T_IDC)
#define _LPC	(T_IDC)
#define _DIG	(T_DIG)
#define _EOF	(T_EOF)
#define _EOR	(T_EOR)
#define _BAR	(T_DIP)
#define _HAT	(T_MET)
#define _BRA	(T_MET)
#define _KET	(T_MET)
#define _AMP	(T_DIP)
#define _SEM	(T_DIP)
#define _LT	(T_DIP)
#define _GT	(T_DIP)
#define _LQU	(T_QOT|T_ESC)
#define _BSL	(T_ESC)
#define _DQU	(T_QOT)
#define _DOL1	(T_SUB|T_ESC)

#define _CBR	T_BRC
#define _CKT	T_DEF
#define _AST	(T_AST)
#define _EQ	(T_DEF)
#define _MIN	(T_DEF|T_SHN)
#define _PCS	(T_SHN)
#define _NUM	(T_SHN)
#define _DOL2	(T_SHN)
#define _PLS	(T_DEF|T_SET)
#define _AT	(T_AST)
#define _QU	(T_DEF|T_SHN)

/* abbreviations for tests */
#define _IDCH	(T_IDC|T_DIG)
#define _META	(T_SPC|T_DIP|T_MET|T_EOR)

extern uchar_t	_ctype1[];

/* nb these args are not call by value !!!! */
#define	space(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_SPC))
#define eofmeta(c)	(((c)&QUOTE)==0 && _ctype1[c]&(_META|T_EOF))
#define qotchar(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_QOT))
#define eolchar(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_EOR|T_EOF))
#define dipchar(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_DIP))
#define subchar(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_SUB|T_QOT))
#define escchar(c)	(((c)&QUOTE)==0 && _ctype1[c]&(T_ESC))

extern uchar_t	_ctype2[];

#define	digit(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_DIG))

#define dolchar(c)	(((c)&QUOTE)==0 && \
			_ctype2[c]&(T_AST|T_BRC|T_DIG|T_IDC|T_SHN))
#define defchar(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_DEF))
#define setchar(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_SET))
#define digchar(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_AST|T_DIG))
#define	letter(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_IDC))
#define alphanum(c)	(((c)&QUOTE)==0 && _ctype2[c]&(_IDCH))
#define astchar(c)	(((c)&QUOTE)==0 && _ctype2[c]&(T_AST))

#define fontshift(c)    (((c)&QUOTE)==0 && _ctype2[c]&(T_FSH))

#include <ctype.h>
#define NLSfontshift(c)	(((c) == FSH0) || ((c) == FSH20) || ((c) == FSH21))
#define NLSencchar(c)	(isascii(c) || NLSfontshift(c))
#define NLSletter(c)    (isalpha(c) || (c) == '_' || (c) > 0x7f)
#define NLSalphanum(c)  (NLSletter(c) || isdigit(c))
