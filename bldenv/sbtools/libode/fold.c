/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		fold
 *		folddown
 *		foldup
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: fold.c,v $
 * Revision 1.7.5.2  1993/04/27  20:20:15  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:20:04  damon]
 *
 * Revision 1.7.5.1  1993/04/27  20:20:14  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.7.2.3  1992/12/03  17:20:51  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:11  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:07  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:15  damon]
 * 
 * Revision 1.7  1991/12/05  21:04:55  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:27  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:59  dwm]
 * 
 * Revision 1.5  90/10/07  20:03:20  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:03  gm]
 * 
 * Revision 1.4  90/08/09  14:23:06  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:53  gm]
 * 
 * Revision 1.3  90/06/29  14:38:36  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:02  gm]
 * 
 * Revision 1.2  90/01/02  19:26:46  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:48  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Unexported original fold routine.
 * 	[86/09/23            gm0w]
 * 
 * 	Rewritten for VAX.  The foldup() and folddown() routines
 * 	are new.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)82  1.1  src/bldenv/sbtools/libode/fold.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:04";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: fold.c,v $ $Revision: 1.7.5.2 $ (OSF) $Date: 1993/04/27 20:20:15 $";
#endif
/*  fold  --  perform case folding
 *
 *  Usage:  p = foldup (out,in);
 *	    p = folddown (out,in);
 *	char *p,*in,*out;
 *
 *  Fold performs case-folding, moving string "in" to
 *  "out" and folding one case to another en route.
 *  Folding may be upper-to-lower case (folddown) or
 *  lower-to-upper case.
 *  Foldup folds to upper case; folddown folds to lower case.
 *  The same string may be specified as both "in" and "out".
 *  The address of "out" is returned for convenience.
 */

typedef enum {FOLDUP, FOLDDOWN} FOLDMODE;

static
char *fold ( char *in, char*out, FOLDMODE whichway )
{
	register char *i,*o;
	register char lower;
	char upper;
	int delta;

	switch (whichway)
	{
	case FOLDUP:
		lower = 'a';		/* lower bound of range to change */
		upper = 'z';		/* upper bound of range */
		delta = 'A' - 'a';	/* amount of change */
		break;
	case FOLDDOWN:
		lower = 'A';
		upper = 'Z';
		delta = 'a' - 'A';
	}

	i = in;
	o = out;
	do {
		if (*i >= lower && *i <= upper)		*o++ = *i++ + delta;
		else					*o++ = *i++;
	} 
	while (*i);
	*o = '\0';
	return (out);
}

char *foldup ( char *in, char *out )
{
	return (fold(out,in,FOLDUP));
}

char *folddown ( char *in, char*out )
{
	return (fold(out,in,FOLDDOWN));
}
