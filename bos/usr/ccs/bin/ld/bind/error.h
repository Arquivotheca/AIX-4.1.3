/* @(#)15	1.4  src/bos/usr/ccs/bin/ld/bind/error.h, cmdld, bos411, 9428A410j 3/9/94 16:59:58 */
#ifndef Binder_ERROR
#define Binder_ERROR
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef NO_NLS
#include <nl_types.h>
extern nl_catd catd;
#endif

#include "bind_msg.h"

#ifdef NO_NLS
#define NLSMSG(num,text)	text
#define msg_get(x)		x
#else
#define NLSMSG(num,text)	num
extern char *msg_get(int);
#endif /* NO_NLS */

/* Loadmap control options */
typedef enum {LOADMAP_STOP, LOADMAP_START, LOADMAP_END} LOADMAP_OPTION;
extern int loadmap_control(LOADMAP_OPTION, ...);

/* The say routine takes one explicit argument.

   If NO_NLS is defined, or if the SAY_NO_NLS flag is set in the flags
   argument, the second argument is the string to be printed, and the
   following arguments are passed to the vfprintf routine.  Otherwise,
   the second argument is the message number, which is used to look up
   the string in the message catalog. */

extern void say(int /*flags*/, ...);
#define SAY_NORMAL 0		/* No special bits */
#define SAY_NO_NL 1		/* Don't print trailing NL */
#define SAY_NL_ONLY 2		/* Print NL only */
#define SAY_NO_NLS 4		/* Default message is 2nd arg */

#define SAY_STDOUT 8		/* Print to stdout (even if quiet) */
#define SAY_NOSTDOUT 16		/* Don't print to stdout (even if noquiet) */
#define SAY_NOLDMAP 32		/* Don't write to loadmap (even if active) */
#define SAY_STDERR_ONLY 64	/* See comment in error.c for bind_err () */

/* The error_level parameter to bind_err is another way to set a command's
   return code. */
extern void bind_err(int /* flags */, int /* error_level */, ...);
extern void show_loadmap_message(RETCODE);
extern int loadmap_message_seen;	/* Set to 0 to see loadmap message on
					   next call to
					   show_loadmap_message(). */


/* Fatal error routine */
#define internal_error() internal_err(__FILE__, __LINE__)
extern void internal_err(char *, int);

#ifdef DEBUG
#define DEBUG_MSG(f,args) {if((bind_debug&(f))==(f)){say args;}}
#else
#define DEBUG_MSG(f,args)
#endif /* DEBUG */

#endif /* Binder_ERROR */
