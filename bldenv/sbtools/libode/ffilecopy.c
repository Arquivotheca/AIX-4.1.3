/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: defined
 *		ffilecopy
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
 * $Log: ffilecopy.c,v $
 * Revision 1.7.5.3  1993/04/29  19:07:43  damon
 * 	CR 464. Port to 386bsd from gm
 * 	[1993/04/29  19:07:36  damon]
 *
 * Revision 1.7.5.2  1993/04/28  14:35:18  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:24  damon]
 * 
 * Revision 1.7.2.3  1992/12/03  17:20:47  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:08  damon]
 * 
 * Revision 1.7.2.2  1992/12/02  20:25:59  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:12  damon]
 * 
 * Revision 1.7  1991/12/05  21:04:49  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:11:15  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:34  dwm]
 * 
 * Revision 1.5  90/10/07  20:03:11  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:50  gm]
 * 
 * Revision 1.4  90/08/09  14:23:00  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:42  gm]
 * 
 * Revision 1.3  90/06/29  14:38:26  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:54  gm]
 * 
 * Revision 1.2  90/01/02  19:26:43  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:37  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Created for VAX.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)80  1.1  src/bldenv/sbtools/libode/ffilecopy.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:59";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: ffilecopy.c,v $ $Revision: 1.7.5.3 $ (OSF) $Date: 1993/04/29 19:07:43 $";
#endif
/*  ffilecopy  --  very fast buffered file copy
 *
 *  Usage:  i = ffilecopy (here,there)
 *	int i;
 *	FILE *here, *there;
 *
 *  Ffilecopy is a routine to copy the rest of a buffered
 *  input file to a buffered output file.  Here and there are open
 *  buffered files for reading and writing (respectively).
 *  Ffilecopy returns 0 if everything was OK; EOF if
 *  there was any error.  Normally, the input file will be left in
 *  EOF state (feof(here) will return TRUE), and the output file will be
 *  flushed (i.e. all data on the file rather in the core buffer).
 *  It is not necessary to flush the output file before ffilecopy.
 */

#include <stdio.h>
#include <unistd.h>
#include <ode/util.h>
#include <sys/types.h>

#define BUFFERSIZE      10240

int
ffilecopy ( FILE *here, FILE *there )
{
        char buffer[BUFFERSIZE];
        char *bp;
        int i, j;

        while ((i = fread(buffer, 1, sizeof(buffer), here)) > 0) {
                bp = buffer;
                while ((j = fwrite(bp, 1, i, there)) != i) {
                        if (j <= 0)
                                return(EOF);
                        i -= j;
                        bp += j;
                }
        }
        if (i < 0)
                return(EOF);
        return(0);
}
