/* @(#)17	1.13  src/bos/usr/lib/methods/common/tcpcfg.h, cmdnet, bos411, 9428A410j 6/10/91 18:52:56 */
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: 
 *
 * MACROS: EXIT_ON_ERROR, DBGMSG
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * tcpcfg.h
 */

#include <stdio.h>
#include <strings.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>
#include <cf.h>

#define  NETDIR  "/usr/lib/drivers/"
#define  METDIR  "/usr/lib/methods/"
#define  NETINET  "netinet"
#define  OBJREPOS  "/etc/objrepos/"
#define  LOCKFILE  "/etc/objrepos/config_lock"

#define  LEDVAL  0x581

#ifdef DEBUG
#define  DBGMSG(x) if (getenv("TCPDEBUG") != NULL) { \
		       fprintf(stderr, "DEBUG(%s) ", progname); \
	               fprintf x ; fprintf(stderr, "\n"); \
		   }
#else
#define  DBGMSG(x)
#endif

#define  EXIT_ON_ERROR(func)  { int rc; if (rc=func) exit(rc); }

#define  ERR  errmsg

#define  CFGEXIT(x)  odm_terminate(); exit(x);

extern char *optarg;
extern int optind;

extern char *getval(), *split_att_val(), *nexttoken();

extern struct CuAt *tcp_getattr();

struct attrvalpair {
	char attr[128];
	char val[128];
};

struct attrvalpair *nextattr();

/*
 * The following defines are for Serial Optical IF configuration.
 */
#define DEVNAME "ops0"
#define UNDEFINED_PID -1
