static char sccsid[] = "@(#)41	1.4  src/bos/usr/lib/sendmail/version.c, cmdsend, bos411, 9428A410j 2/25/91 13:05:26";
/* 
 * COMPONENT_NAME: CMDSEND version.c
 * 
 * FUNCTIONS: 
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
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/

#include "conf.h"

/*
 *  The "Version" is the internal program version set by the originators
 *  of the software.  If Version points to a non-null string, that string
 *  will be used to define the 'v' macro.  Else, the current AIX name and
 *  level will be used.  (The latter is the release default).  See main.c.
 *
 *  The 'v' macro is normally used in the "Received" header (defined in the
 *  configuration file) to document the software level receiving the mail.
 */
char  *Version = "";
char  BSDVersion[] = "UCB 5.64";

/*
 *  The "Salt" is for the debug password encryption scheme.
 *  This may changed from version to version.
 */
#ifdef DEBUG
char	Salt[]    = "45";		/* salt for encryption		*/
#endif DEBUG
