static char sccsid[] = "@(#)79	1.7  src/bos/usr/bin/mail/version.c, cmdmailx, bos411, 9428A410j 1/27/94 14:20:08";
/* 
 * COMPONENT_NAME: CMDMAILX version.c
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "version.c    5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

/*
 * Just keep track of the date/sid of this version of Mail.
 * Load this file first to get a "total" Mail version.
 */
static  char    *SccsID = "UCB Mail Version 5.2 (6/21/85)";
char	*version = "[5.2 UCB] [AIX 4.1]";
