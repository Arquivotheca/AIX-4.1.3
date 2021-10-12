/* @(#)22	1.2  src/bos/usr/bin/tset/tset.h, cmdtty, bos411, 9428A410j 10/21/93 02:41:30 */

/*
 * COMPONENT_NAME: CMDTTY tty control commands
 *
 * FUNCTIONS: header file for tset
 *
 * ORIGINS: 26, 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
**  SYSTEM DEPENDENT TERMINAL DELAY TABLES
**
**	Evans Hall VAX
**
**	This file maintains the correspondence between the delays
**	defined in /etc/termcap and the delay algorithms on a
**	particular system.  For each type of delay, the bits used
**	for that delay must be specified (in XXbits) and a table
**	must be defined giving correspondences between delays and
**	algorithms.  Algorithms which are not fixed delays (such
**	as dependent on current column or line number) must be
**	cludged in some way at this time.
*/

/*
**  Carriage Return delays
*/

int	CRbits = CRDLY;
struct delay	CRdelay[] = {
    0,	CR0,
    9,	CR3,
    80,	CR1,
    160, CR2,
    -1,
};

/*
**  TaB delays
*/

int	TBbits = TABDLY;
struct delay	TBdelay[] = {
    0,	TAB0,
    11,	TAB1,				/* special M37 delay */
    -1,
};

/*
**  Back Space delays
*/

int	BSbits = BSDLY;
struct delay	BSdelay[] = {
    0,	BS0,
    -1,
};

/*
**  Form Feed delays
*/

int	FFbits = FFDLY;
struct delay	FFdelay[] = {
    0,	FF0,
    2000, FF1,
    -1,
};

/*
**  New Line delays
*/

int	NLbits = NLDLY;
struct delay	NLdelay[] = {
    0,	NL0,
    -1,
};

/*
**  Verticle tab delays
*/

int	VTbits = VTDLY;
struct delay	VTdelay[] = {
    0,	VT0,
    -1,
};
