/* aix_sccsid[] = "src/bos/usr/lib/sendmail/useful.h, cmdsend, bos411, 9428A410j AIX 6/15/90 23:26:52" */
/* 
 * COMPONENT_NAME: CMDSEND useful.h
 * 
 * FUNCTIONS: bitset, max, min 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  USEFUL.H -- Some useful stuff.
**
**	@(#)useful.h	4.1		7/25/83
**
*/

# define TRUE	1
# define FALSE	0

# ifndef NULL
# define NULL	0
# endif NULL

/* bit hacking */
# define bitset(bit, word)	(((word) & (bit)) != 0)

/* some simple functions */
# ifndef max
# define max(a, b)	((a) > (b) ? (a) : (b))
# define min(a, b)	((a) < (b) ? (a) : (b))
# endif max
