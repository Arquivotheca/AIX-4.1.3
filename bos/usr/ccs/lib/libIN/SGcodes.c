static char sccsid[] = "@(#)11	1.7  src/bos/usr/ccs/lib/libIN/SGcodes.c, libIN, bos411, 9428A410j 6/10/91 10:22:43";
/*
 * LIBIN: SGcodes
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Table of signal names associated with signal codes.
 */

#include <sys/signal.h>
#include <IN/SGdefs.h>

char *SGcodes[NSIG] = {
	"NULL",		"HUP",		"INT",		"QUIT",
	"ILL",		"TRAP",		"IOT",		"EMT",	
	"FPE",		"KILL",		"BUS",		"SEGV",
	"SYS",		"PIPE",		"ALRM",		"TERM",
	"URG",		"STOP",		"TSTP",		"CONT",
	"CHLD",		"TTIN",		"TTOU",		"IO",
	"XCPU",		"XFSZ",		(char *)0,	"MSG",
	"WINCH",	"PWR",		"USR1",		"USR2",
	"PROF",		"DANGER",	"VTALRM",	"MIGRATE",
	"PRE",		(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	"GRANT",	"RETRACT",	"SOUND",	"SAK"
};
