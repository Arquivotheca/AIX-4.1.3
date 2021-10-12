/* @(#)96	1.1  src/bos/diag/tu/mpa/mpa_tu.h, mpatu, bos411, 9428A410j 5/5/93 14:19:16 */
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: ADD_SPRINTF
 *		ROF
 *		RPT_BUGGER
 *		RPT_ERR
 *		RPT_HARD
 *		RPT_INFO
 *		RPT_SOFT
 *		RPT_UPDATE
 *		RPT_VERBOSE
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*************************************************************************
*************************************************************************/
#ifndef DIAGS
#include "hxihtx.h"             /* for the ps structure mostly */
extern struct htx_data ps;
#define  STATUS_TIMEOUT 200000     /* 200 sec */
#define  READ_TIMEOUT   60        /* 30 sec */
#define  WRITE_TIMEOUT  60        /* 60 sec */
#else

#define  STATUS_TIMEOUT 5000     /* 5 sec */
#define  READ_TIMEOUT   5        /* 5 sec */
#define  WRITE_TIMEOUT  5        /* 5 sec */

#endif

#define  PATLIB "/usr/lpp/htx/pattern/"
#define  MAXBYTES 4094
#define  SLAVE_TIMEOUT   60        /* 60 sec */
#define  RESTORE        1
#define  SAVE           2
#define  INVOKED_BY_HTX 99

/*************************************************************************
* Storage area for options selected by a rule stanza                     *
*************************************************************************/
struct mpa_htx {
	char            rule_id[9];     /* rule id                      */
	char            pattern_id[80]; /* pattern id                   */
	int             recsize;        /* record size (TU 26 & TU 30) */
	int             bps;            /* bits per second rate int clk */
	int             num_oper;       /* number of loops in TU        */
	int             fix_bytes;      /* random or fixed byte count   */
	int             clock;          /* int or ext clock  transfers  */
	int             nrzi;           /* nrzi or no nrzi data         */
	int             wrap_type;      /* interal or external wrap     */
	int             seed;           /* seed for random lenght gen   */
	uchar           pos2;           /* save the start value of pos 2*/
	uchar           pos3;           /* save the start value of pos 3*/
	};

/*
* standard structure used by manufacturing diagnostics.
*/
struct tucb_t
{
      long tu,        /* test unit number   */
      loop,      /* loop test of tu    */
      mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */

      long r1,        /* reserved */
      r2;        /* reserved */
};
/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
*/
struct mpa_tu
{
      struct tucb_t header;
      struct mpa_htx pr;
};

#define TUTYPE struct mpa_tu


#ifndef DIAGS
/*************************************************************************
* The following #ifdef block is designed to make this module		 *
* plug-compatible into other applications.  If this block is not	 *
* included (i.e. HTX is not defined), another block defining these items *
* must be provided.							 *
*	MSGBUF		A convenient place to build messages		 *
*	VERBOSE		A boolean that determines noisiness		 *
*	RPT_HARD(n, s)	How to report a "Hard" error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_SOFT(n, s)	How to report a "Soft" error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_ERR(n, s)	How to report a system error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_INFO(s)	How to output an informational message		 *
*		s	A textual information message			 *
*	RPT_VERBOSE(s)	How to report noise when VERBOSE is true	 *
*		s	A textual information message			 *
*	RPT_UPDATE(s)	How to issue a status update			 *
*		s	A textual information message			 *
*************************************************************************/
#define MSGBUF                  (ps.msg_text)
#define VERBOSE                 (ps.run_type[0] == 'O')
#define RPT_HARD(n, s)          { if (--max_errors > 0) hxfmsg(&ps, n, HTX_HE_HARD_ERROR, s); }
#define RPT_SOFT(n, s)          { if (--max_errors > 0) hxfmsg(&ps, n, HTX_HE_SOFT_ERROR, s); }
#define RPT_ERR(n, s)           { if (--max_errors > 0) hxfmsg(&ps, n, HTX_SYS_HARD_ERROR,s); }
#define RPT_INFO(s)             { hxfmsg(&ps, 0, HTX_HE_INFO, s); }
#define RPT_VERBOSE(str)	{ if (VERBOSE) printf("%s", str); }
#define RPT_UPDATE(str)         { hxfupdate(UPDATE, &ps); }
#define INC_GOOD_OTHERS         { ps.good_others++; }
#define INC_BAD_OTHERS          { ps.bad_others++; }
#endif

#ifdef DEBUG
#define RPT_BUGGER(str)         { bugfd=fopen("/tmp/bfile","a");\
				  fprintf(bugfd,"%s", str);\
				  fclose(bugfd); }
#else
#define RPT_BUGGER(str)
#endif

#define MATCH           0               /* strcmp return value on match */

/*************************************************************************
* These TU return codes are translated to appropriate action by exectu() *
*************************************************************************/
#define RC_GOOD         0               /* Test Unit succeeded */
#define RC_SOFT         1               /* Test Unit encountered a "soft" error */
#define RC_HARD         2               /* Test Unit encountered a "hard" error */
#define RC_SYS          3               /* system error occured */
#define RC_SYS_ERRNO    4
#define RC_RCVRY        5               /* Test encountered recovery entered
*/
/*************************************************************************
* General macros and definitions for the exerciser and TU's		 *
*************************************************************************/

/*************************************************************************
* This macro will perform a subtest or lower-level function.  If it	 *
* fails an immediate return is performed.  Requires a local int rc.	 *
*************************************************************************/
#define ROF(x)  { if ((rc = (x)) != RC_GOOD) return(rc); }      /* Return On Failure */

/*************************************************************************
* Operating System Portability macros					 *
*************************************************************************/

/*************************************************************************
* ADD_SPRINTF		use sprintf to generate string, accumulate len.	 *
*	str		pointer to string to add length of string to.	 *
*	sp_cmd		sprintf command to generate string at str.	 *
*************************************************************************/
#define ADD_SPRINTF(str, sp_cmd)	{ str += sp_cmd; }

extern FILE            *bugfd;          /* The fildes for the debug file */
extern int              bugit;          /* debug flag for info to file   */

#ifndef DIAGS

/*************************************************************************
* The following externals are declared above main in hxempa.c            *
*************************************************************************/
extern int              max_errors;


/*************************************************************************
* These are from libc and are used here and there                        *
*************************************************************************/
extern char             *malloc();      /* memory allocation, libc */

/*************************************************************************
* These are external variables.                                          *
*************************************************************************/
extern long int        pass_cnt;
extern int             bid_wait;
extern int             master;
extern int             master_set;
extern long            totbytes;

#endif

extern char             *sys_errlist[]; /* system defined err msgs, indexed by errno */
extern int              sys_nerr;       /* max value for errno */
extern int              errno;          /* error returned by system call */
extern char             *pat_ptr;
extern char             *msg;

