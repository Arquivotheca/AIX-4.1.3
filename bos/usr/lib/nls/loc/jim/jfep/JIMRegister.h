/* @(#)11	1.1  src/bos/usr/lib/nls/loc/jim/jfep/JIMRegister.h, libKJI, bos411, 9428A410j 5/18/93 08:40:47	*/
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMRegister header file
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _JIMRegister_h
#define _JIMRegister_h

#define	JIM_REG_WIN_WMAX	48	/* register aux window width	*/
#define	JIM_REG_WIN_HEIGHT	4	/* register aux window height	*/
#define	JIM_REG_GOKU_MAX	40	/* goku column max (= SJIS max)	*/
#define	JIM_REG_YOMI_MAX	20	/* yomi column max (= SJIS max)	*/
#define	JIM_REG_IN_START	6	/* input start position		*/
#define	JIM_REG_UDNAME_START	20	/* user dict name position	*/

/*----------------------------------------------------------------------*
 *	JIM runtime registration information (for commited string)
 *----------------------------------------------------------------------*/
typedef struct {
	int	gokulen;	/* goku length				*/
	char	goku[42];	/* goku string (SJIS)			*/
	int	yomilen;	/* yomi length				*/
	char	yomi[24];	/* yomi string (SJIS)			*/
	int	row;		/* input row (1 (goku) or 2 (yomi))	*/
	int	cur;		/* cursor position			*/
			/* real cursor is at "cur" + "pre-edit cur"	*/
	iconv_t	cd;		/* converter descriptor	for EUC		*/
	int	other_item;	/* other item number			*/
	int	msgno;		/* current message number		*/
	int	next_msgno;	/* next message number			*/
} JIMRegister;

#endif /* _JIMRegister_h */
