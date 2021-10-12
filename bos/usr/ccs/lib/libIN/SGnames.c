static char sccsid[] = "@(#)22	1.6  src/bos/usr/ccs/lib/libIN/SGnames.c, libIN, bos411, 9428A410j 6/10/91 10:22:46";
/*
 * LIBIN: SGnames
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
 * FUNCTION: Table of descriptive signal names associated with signal codes.
 */

#include <sys/signal.h>
#include <IN/SGdefs.h>

char *SGnames[NSIG] = {
	"Signal 0",		"Hangup",		"Interrupt",
	"Quit",			"Illegal instruction",	"Breakpoint",
	"Abort",		"EMT instruction",	"Floating exception",
	"Killed",		"Bus error",		"Memory fault",
	"Bad system call",	"Broken pipe",		"Alarm call",
	"Terminated",		"Urgent I/O",		"Stopped",
	"Suspended",		"Continue"
	,
	"Child death",		"Background read",	"Background write",
	"I/O completed",	"CPU time exceeded",	"File size exceeded",
	"Signal 26",		"Message pending",	"Window size changed",
	"Power fail",		"User signal 1",	"User signal 2",
	"Profiling alarm",	"Danger",		"Virtual time alarm",
	"Signal 35",		"Signal 36",		"Signal 37",
	"Signal 38",		"Signal 39"
	,
	"Signal 40",		"Signal 41",		"Signal 42",
	"Signal 43",		"Signal 44",		"Signal 45",
	"Signal 46",		"Signal 47",		"Signal 48",
	"Signal 49",		"Signal 50",		"Signal 51",	
	"Signal 52",		"Signal 53",		"Signal 54",
	"Signal 55",		"Signal 56",		"Signal 57",
	"Signal 58",		"Signal 59"
	,		
	"HFT grant",		"HFT retract",		"HFT sound",
	"Signal 63"
};
