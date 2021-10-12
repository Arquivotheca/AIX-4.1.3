/* @(#)61	1.3  src/bos/usr/bin/pax/charmap.h, cmdarch, bos411, 9428A410j 4/8/94 12:10:55 */
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* RCSfile Revision (OSF) Date */
/*
 * charmap.h - defnitions for charmap parsing
 *
 * DESCRIPTION
 *
 *	This file contains definitions required for parsing the
 *	charmap files.   It is included by charmap.c
 *
 * AUTHOR
 *
 *     Tom Jordahl	- The Open Software Foundation
 *
 */

#ifndef _CHARMAP_H
#define _CHARMAP_H

#define _ILS_MACROS
#include <ctype.h>

#define LMAX		1024		/* max number of chars in a line */

#define	CODE_SET_NAME	1
#define MB_MAX		2
#define MB_MIN		3
#define ESCAPE_CHAR	4
#define COMMENT_CHAR	5

/*
 * isportable is defined to return true if the character is
 * a member of the POSIX 1003.2 portable character set.
 *
 * right now we use isascii(), this should be correct
 */
#define isportable(c)	(isascii(c)) 

typedef struct value_struct {
    unsigned char	mbvalue[10];	/* multi-byte encoding */
    short		nbytes;		/* number of bytes in mbvalue */
} Value;

typedef struct charmap_struct {
    char		  *symbol;	/* symbol name */
    unsigned char 	  value[10];	/* symbols multi-byte encoding */
    short		  nbytes;	/* number of bytes in value */
    struct charmap_struct *next;	/* pointer to next struct */
} Charentry;

#endif /* _CHARMAP_H */
