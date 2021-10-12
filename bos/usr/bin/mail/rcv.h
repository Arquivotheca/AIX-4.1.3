/* @(#)66	1.3  src/bos/usr/bin/mail/rcv.h, cmdmailx, bos411, 9428A410j 4/17/91 15:16:55 */
/* 
 * COMPONENT_NAME: CMDMAILX rcv.h
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
 *      rcv.h       5.1 (Berkeley) 6/6/85
 */

/*
 * Mail -- a mail program
 *
 * This file is included by normal files which want both
 * globals and declarations.
 */

#ifdef	pdp11
#include <whoami.h>
#endif
#include "def.h"
#include "glob.h"
#include <nl_types.h>
