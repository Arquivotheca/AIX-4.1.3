/* "@(#)87	1.13  src/bos/sbin/helpers/v3fshelpers/fsop.h, cmdfs, bos411, 9428A410j 6/15/90 21:20:25" */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef enum { False = FALSE, True = TRUE } bool_t;

/*
** old_fs's are an interesting subset of filesystems
** a working definition of "interesting" is in found_any()
** this will point at the ODM in the future
*/

struct old_fs
{
  char            name[PATH_MAX];
  char            dev[PATH_MAX];
  char            log[PATH_MAX];

  struct old_fs  *next;
};  

/*
** Mode & DebugLevel are declared in fshelper.c but all of the
** routines find it convenient to reference them.
** When these get complicated they ought to be functions.
*/
extern int Mode;
extern int DebugLevel;
extern int PipeFd;

#define interactive()	(Mode & FSHMOD_INTERACT)
#define fshelperror()	(Mode & FSHMOD_PERROR)
#define force()		(Mode & FSHMOD_FORCE)
#define nonblocking()	(Mode & FSHMOD_NONBLOCK)
#define errdump()	(Mode & FSHMOD_ERRDUMP)
#define standalone()	(Mode & FSHMOD_STANDALONE)
#define igndevtype()	(Mode & FSHMOD_IGNDEVTYPE)

#define debug(level)	(DebugLevel >= abs(level))
#define Debug(level)    (debug(level) && interactive())

#define dbg_printf(level,prspec) if (Debug (level)) (void) printf prspec;
#define expr_dbg_printf(expr,level,prspec) if (expr) dbg_printf(level, prspec)

/*
** debug levels used within the various op's
*/
#define FSHBUG_SUBWARN		(FSHBUG_BLOOP+1)
#define FSHBUG_SUBRETERN        (FSHBUG_BLOOP+2)
#define FSHBUG_SUBCALL          (FSHBUG_BLOOP+3)
#define FSHBUG_SUBRETURN	(FSHBUG_BLOOP+4)
#define FSHBUG_SUBSTATUS        (FSHBUG_BLOOP+5)
#define FSHBUG_SUBDATA          (FSHBUG_BLOOP+6)

#ifndef lint
/*
** included to get perror()
*/
#include "stdio.h"
#include <stdlib.h>  /* for abs */

#define RETURN(subname,fsherrcode)					      \
	{if (fshelperror () && errno) perror (subname);                       \
         if ((fsherrcode != FSHERR_GOOD || errno) && debug (FSHBUG_SUBRETERN))\
	   printf ("\n%s: (line %d) %s errcode = %d, errno:%d\n",             \
	        __FILE__, __LINE__, subname, fsherrcode, errno);	      \
         else if (debug (FSHBUG_SUBRETURN) && fsherrcode == FSHERR_GOOD &&    \
		  !errno) printf ("%s ", subname);                            \
	 return fsherrcode == FSHERR_GOOD && errno? -FSHERR_INTERNAL: fsherrcode;		}
#else
#define RETURN(s,err) {return err;}
#endif    
/*
** handy stuff to manipulate opflags
**
** types of option-specific arguments
*/

typedef enum
{
  NIL_T = 0, INT_T, STR_T, BOOL_T, INTV_T, STRV_T, BOOLV_T
} arg_t;

struct arg
{
  char     *name;
  caddr_t   iptr;
  arg_t     type;
};

/* 
 * for INTV_T, BOOLV_T, STRV_T
 */

/* delimiter for the string versions of the vectors */
#define FSH_VECTORDELIM ':'

/* internal structure for the vector itself */
struct fsh_argvector {
	int iv_stride;
	union {
	    int *_iv_int;
	    bool_t *_iv_bool;
	    char **_iv_str;
	} _v;
#define iv_int _v._iv_int
#define iv_bool _v._iv_bool
#define iv_str _v._iv_str
};

/*
** forward refs for routines provided by fshelper
** that many ops find useful
*/
extern struct old_fs  *get_oldfs();
extern int             rewind_block();
extern int             get_args();

/*
** misc.
*/
#define NILSTR ""

