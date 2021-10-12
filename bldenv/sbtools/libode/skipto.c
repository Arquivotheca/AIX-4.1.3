/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		skipover
 *		skipto
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
 * $Log: skipto.c,v $
 * Revision 1.7.7.1  1993/11/10  15:35:27  damon
 * 	CR 463. Changed unsigned types to const char
 * 	[1993/11/10  15:34:48  damon]
 *
 * Revision 1.7.5.2  1993/04/27  22:41:43  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:41:15  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:22:51  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:21  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:27:05  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:48  damon]
 * 
 * Revision 1.7  1991/12/05  21:13:13  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:12:05  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:25:35  dwm]
 * 
 * Revision 1.5  90/10/07  20:04:47  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:11:11  gm]
 * 
 * Revision 1.4  90/08/09  14:24:02  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:44  gm]
 * 
 * Revision 1.3  90/06/29  14:39:31  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:22:00  gm]
 * 
 * Revision 1.2  90/01/02  19:27:17  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:56  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Skipover, skipto rewritten to avoid inner loop at expense of space.
 * 	[81/06/26            drs]
 * 
 * 	Skipover, skipto adapted for VAX from skip() and skipx() on the PDP-11
 * 	(from Ken Greer).  The names are more mnemonic.
 * 
 * 	Sindex adapted for VAX from indexs() on the PDP-11 (thanx to Ralph
 * 	Guggenheim).  The name has changed to be more like the index()
 * 	and rindex() functions from Bell Labs; the return value (pointer
 * 	rather than integer) has changed partly for the same reason,
 * 	and partly due to popular usage of this function.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)24  1.1  src/bldenv/sbtools/libode/skipto.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:37";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: skipto.c,v $ $Revision: 1.7.7.1 $ (OSF) $Date: 1993/11/10 15:35:27 $";
#endif
/*
 *  skipover and skipto -- skip over characters in string
 *
 *  Usage:	p = skipto (string,charset);
 *		p = skipover (string,charset);
 *
 *  char *p,*charset,*string;
 *
 *  Skipto returns a pointer to the first character in string which
 *  is in the string charset; it "skips until" a character in charset.
 *  Skipover returns a pointer to the first character in string which
 *  is not in the string charset; it "skips over" characters in charset.
 */

#include <ode/util.h>

static char tab[256] = {
	0};

char *
skipto ( const char *string, const char *charset )
{
	register char *setp,*strp;

	tab[0] = 1;		/* Stop on a null, too. */
	for (setp=(char *)charset;  *setp;  setp++) tab[(unsigned)*setp]=1;
	for (strp=(char *)string;  tab[(unsigned)*strp]==0;  strp++)  ;
	for (setp=(char *)charset;  *setp;  setp++) tab[(unsigned)*setp]=0;
	return (strp);
}

char *
skipover ( const char *string, const char *charset )
{
	register char *setp,*strp;

	tab[0] = 0;		/* Do not skip over nulls. */
	for (setp=(char *)charset;  *setp;  setp++) tab[(unsigned)*setp]=1;
	for (strp=(char *)string;  tab[(unsigned)*strp];  strp++)  ;
	for (setp=(char *)charset;  *setp;  setp++) tab[(unsigned)*setp]=0;
	return (strp);
}
