/* @(#)04	1.16  src/bos/usr/bin/bsh/mac.h, cmdbsh, bos411, 9428A410j 4/22/94 18:21:30 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
 * 1.9  com/cmd/sh/sh/mac.h, cmdsh, bos320, 9125320 6/6/91 23:10:43
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#define LOBYTE	0377
#define STRIP   0177
#define QUOTE	0200

#undef EOF
#define EOF	0
#define NL	'\n'
#define SP	' '
#define LQ	'`'
#define RQ	'\''
#define MINUS	'-'
#define COLON	':'
#define TAB	'\t'
/*
 * The values of FSH0 and FNLS are arbitrary, in theory, but trouble does
 * in fact occur if either one appears in a string.  The current workaround
 * is to use ^? and ^V, which are the default INTR and QUIT characters, so
 * that the user is unlikely to encounter the problem.  The correct fix is
 * probably to use arrays of NLchar instead of "encoded strings"; this would
 * make both of these constants obsolete.
 */
#define FNLS    '\026'
#define FSH0	'\35'
#define FSH20	'\36'
#define FSH21	'\37'
#define FNLS    '\026'

#define blank()		prc(SP)
#define	tab()		prc(TAB)
#define newline()	prc(NL)

