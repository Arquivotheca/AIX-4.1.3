static char sccsid[] = "@(#)08  1.2  src/bos/usr/bin/patch/version.c, cmdposix, bos411, 9428A410j 3/14/94 09:13:23";
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: version
 *
 * ORIGINS: 27, 85
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: version.c,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1992/07/06 18:33:28 $";
#endif

/* Header: version.c,v 2.0 86/09/17 15:40:11 lwall Exp
 *
 * Log:	version.c,v
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

/* Print out the version number and die. */

void
version()
{
    extern char patch_rcsid[];

#ifdef lint
    patch_rcsid[0] = patch_rcsid[0];
#else
    say1(MSGSTR(AIXVERSION, "AIX - based on:\n"));
    fatal3(MSGSTR(VERSION, "%s\nPatch level: %d\n"), patch_rcsid, PATCHLEVEL);
#endif
}
