/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		filecopy
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
 * $Log: filecopy.c,v $
 * Revision 1.7.5.2  1993/04/28  14:35:22  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:25  damon]
 *
 * Revision 1.7.5.1  1993/04/28  14:35:21  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.7.2.3  1992/12/03  17:20:49  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:09  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:26:03  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:14  damon]
 * 
 * Revision 1.7  1991/12/05  21:04:52  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:21  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:40  dwm]
 * 
 * Revision 1.5  90/10/07  20:03:16  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:57  gm]
 * 
 * Revision 1.4  90/08/09  14:23:03  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:47  gm]
 * 
 * Revision 1.3  90/06/29  14:38:29  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:58  gm]
 * 
 * Revision 1.2  90/01/02  19:26:45  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:42  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Rewritten for VAX; same as "filcopy" on PDP-11.  Bigger buffer
 * 	size (20 physical blocks) seems to be a big win; aligning things
 * 	on block boundaries seems to be a negligible improvement at
 * 	considerable cost in complexity.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)81  1.1  src/bldenv/sbtools/libode/filecopy.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:02";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: filecopy.c,v $ $Revision: 1.7.5.2 $ (OSF) $Date: 1993/04/28 14:35:22 $";
#endif
/*  filecopy  --  copy a file from here to there
 *
 *  Usage:  i = filecopy (here,there);
 *	int i, here, there;
 *
 *  Filecopy performs a fast copy of the file "here" to the
 *  file "there".  Here and there are both file descriptors of
 *  open files; here is open for input, and there for output.
 *  Filecopy returns 0 if all is OK; -1 on error.
 *
 *  I have performed some tests for possible improvements to filecopy.
 *  Using a buffer size of 10240 provides about a 1.5 times speedup
 *  over 512 for a file of about 200,000 bytes.  Of course, other
 *  buffer sized should also work; this is a rather arbitrary choice.
 *  I have also tried inserting special startup code to attempt
 *  to align either the input or the output file to lie on a
 *  physical (512-byte) block boundary prior to the big loop,
 *  but this presents only a small (about 5% speedup, so I've
 *  canned that code.  The simple thing seems to be good enough.
 */

#include <unistd.h>
#include <ode/util.h>
#include <sys/types.h>

#define BUFFERSIZE 10240

int
filecopy ( int here, int there )
{
	register int kount;
	char buffer[BUFFERSIZE];
	kount = 0;
	while (kount == 0 && (kount=read(here,buffer,BUFFERSIZE)) > 0)
		kount -= write (there,buffer,kount);
	return (kount ? -1 : 0);
}
