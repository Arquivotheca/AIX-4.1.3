/* @(#)03	1.6  src/bos/usr/bin/ex/ex_re.h, cmdedit, bos41J, 9515B_all 3/31/95 16:13:24 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_re.h
 *
 * FUNCTION: none
 *
 * ORIGINS: 3, 10, 13, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ex_re.h	1.2  com/cmd/edit/vi,3.1,9013 10/6/89 17:19:08 
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */
/* ex_re.h,v  Revision: 2.4  (OSF) Date: 90/10/07 16:29:46  */

/*
 * Regular expression definitions.
 * The regular expressions in ex are similar to those in ed,
 * with the addition of the word boundaries from Toronto ed
 * and allowing character classes to have [a-b] as in the shell.
 * The numbers for the nodes below are spaced further apart then
 * necessary because I at one time partially put in + and | (one or
 * more and alternation.)
 */
struct	regexp {
	wchar_t	Expbuf[ESIZE + 2];
	short	Circfl;
	short	Nbra;
};

/*
 * There are three regular expressions here, the previous (in re),
 * the previous substitute (in subre) and the previous scanning (in scanre).
 * It would be possible to get rid of "re" by making it a stack parameter
 * to the appropriate routines.
 */
var struct	regexp re;		/* Last re */
var struct	regexp scanre;		/* Last scanning re */
var struct	regexp subre;		/* Last substitute re */

/*
 * Defining circfl and expbuf like this saves us from having to change
 * old code in the ex_re.c stuff.
 */
#define	expbuf	re.Expbuf
#define	circfl	re.Circfl
#define	nbra	re.Nbra
#define savere(a)	((a) = re)
#define resre(a)	(re = (a))

/*
 * Definitions for substitute
 */
var wchar_t	*braslist[NBRA];	/* Starts of \(\)'ed text in lhs */
var wchar_t	*braelist[NBRA];	/* Ends... */
var wchar_t	rhsbuf[RHSSIZE];	/* Rhs of last substitute */

/*
 * Definitions of codes for the compiled re's.
 * The re algorithm is described in a paper
 * by K. Thompson in the CACM about 10 years ago
 * and is the same as in ed.
 */
#define	STAR	1	/* expression can be repeated arbitrarily       */
#define INTERVAL 2	/* following two numbers define valid range     */

#define	CDOL	1	/* matches end of line				*/
#define	CBOL	2	/* matches beginning of line			*/
#define	CDOT	4	/* a dot, matches any character			*/
#define	CCL	8	/* start of a bracket expression []		*/
#define	NCCL	12	/* start of a negative bracket expression [^]	*/
#define	CEOFC	17	/* delimits the end of a compiled RE		*/
#define	CBRA	20	/* start of subexpression, followed by number	*/
#define	CKET	24	/* end of subexpression, followed by number	*/
#define	CCHR	28	/* match the following literal character	*/
#define	CBRC	32	/* match start of word				*/
#define	CLET	36	/* match end of word				*/

/*
 * Definitions of things, other than simple characters, that can appear in a
 * character class.  The internal representation is a backslash followed by
 * one of these codes.  Most of these represent ctype classes.
 */
#define CL_BADCLASS	0x00	/* Not stored; used only as an error return */
#define CL_BACKSLASH	0x01	/* Real backslash */
#define CL_RANGE	0x02	/* a-z becomes '\\' CL_RANGE 'a' 'z' */
#define CL_GOODCLASS	0x03	/* marks a valid class, followed by id */
#define CL_EQUIVCLASS	0x04	/* equivalence class, followed by colval */
#define CL_COLLEL	0x05	/* marks a collating element matching */

#define CBACKREF	40	/* back reference, followed by number */
#define CFERKCAB	44	/* indicates end of back reference */
