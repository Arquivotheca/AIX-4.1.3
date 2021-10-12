/* @(#)20 1.3  src/bos/usr/bin/learn/lrnref.h, cmdlearn, bos411, 9428A410j 3/22/93 13:20:57 */
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: none 
 *
 * ORIGINS: 26, 27 
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
*/

#include <ctype.h>
#define STRCMP strcoll
#define PRINTF printf
#define FPRINTF fprintf

#define	READY	0
#define	PRINT	1
#define	COPYIN	2
#define	COPYOUT	3
#define	UNCOPIN	4
#define	UNCOPOUT	5
#define	PIPE	6
#define	UNPIPE	7
#define	YES	8
#define	NO	9
#define	SUCCEED	10
#define	FAIL	11
#define	BYE	12
#define	LOG	13
#define	CHDIR	14
#define	LEARN	15
#define	MV	16
#define	USER	17
#define	NEXT	18
#define	SKIP	19
#define	WHERE	20
#define	MATCH	21
#define	NOP	22
#define	BAD	23
#define	CREATE	24
#define	CMP	25
#define	ONCE	26
#define	AGAIN	27
#define	HINT	28

#define MAX_LEN 200
#define LEN_MAX 100
#define LEN_L   30

extern	int	more;
extern	char	*level;
extern	int	speed;
extern	char	*sname;
extern	char	*direct;
extern	char	*todo;
extern	int	didok;
extern	int	sequence;
extern	int	comfile;
extern	int	status;
extern	int	wrong;
extern	char	*pwline;
extern	char	*dir;
extern	FILE	*incopy;
extern	FILE	*scrin;
extern	int	logging;
extern	int	ask;
extern 	int	again;
extern	int	skip;
extern	int	teed;
extern	int	total;
