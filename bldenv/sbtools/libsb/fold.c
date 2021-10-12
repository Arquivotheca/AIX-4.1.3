static char sccsid[] = "@(#)38  1.1  src/bldenv/sbtools/libsb/fold.c, bldprocess, bos412, GOLDA411a 4/29/93 12:20:57";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: fold.c,v $ $Revision: 1.7 $ (OSF) $Date: 91/12/05 21:04:55 $";
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
char *fold (out,in,whichway)
char *in,*out;
FOLDMODE whichway;
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

char *foldup (out,in)
char *in,*out;
{
	return (fold(out,in,FOLDUP));
}

char *folddown (out,in)
char *in,*out;
{
	return (fold(out,in,FOLDDOWN));
}
