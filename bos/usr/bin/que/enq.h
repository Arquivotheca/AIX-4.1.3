/* @(#)26	1.9.1.4  src/bos/usr/bin/que/enq.h, cmdque, bos411, 9428A410j 1/26/94 08:47:41 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* possible values returned by argtype() */

/* flags passed to upipe */
#define	UPI_FLAG	0
#define	UPI_FILE	1

#define ARGFILE  1      /* file name */
#define ARGSTDIN 2      /* stdin */
#define	ARGM0    24	/* message on command line */
#define	ARGM1    25	/* message in a file */
#define ARGDIE   16     /* qdaemon to die gracefully */
#define ARGREAD  21     /* reread config file */

/* constants for admin auditing */
#define	AUD_CANCEL 	"cancel jobs"
#define AUD_DEVDOWN	"device down"
#define AUD_DEVUP	"device up"
#define AUD_DEVDOWNKILL	"device down - kill all jobs"
#define AUD_KILLQDAEMON	"kill qdaemon"

/* miscellaneous constants */
#define DRAIN   NULL                    /* tells upipe to drain */
#define UBUF    150                     /* size of upipe buffer, used */
					/* to hold user flags. */
#define MAXCOP  50                      /* max copies allowed (-nc=?) */
#define DELETE  NULL                    /* tells remember to delete */
#define TMPNAM "taaXXXXX"               /* name template for tmp files. */
#define SYSGRP  0                       /* system group */
#define P_DEFAULT 15                    /* default priority */
#define P_ULIMIT 20                     /* limit for normal user priority */
#define P_SLIMIT 30                     /* limit for system user */

/* structure giving all info to go into first 3 lines of q entry */
/* plus info needed to cook up file name */
/* much, but not all, of it comes from the p struct, ( command line options.) */
/* AIX security: replaced int qe_uid and int qe_gid by char **qe_pcred and char **qe_penv */
/*		 removed char *qe_curdir (cwd is contained in penv string)		*/
struct qe 
{
	char    qe_priority;
	unsigned qe_head ;
	unsigned qe_trail ;
/* AIX security enhancement 	*/
	char	**qe_pcred;		/* pcred list contains uid, gid, concurrent group set, etc. 	*/
	char	**qe_penv;		/* penv list contains user's environment			*/
	int     qe_rm;			/* rm when done? */
	int     qe_tmp;			/* temp file?*/
	short   qe_numcops;             /* number of copies */
	char    qe_acct;
	char   *qe_logname;
	char   *qe_logdir;
	char   *qe_reqname;             /* just the title */
	int     qe_numblks;             /* number of blocks */
	struct q *qe_queue;             /* queue */
	struct d *qe_dev;               /* device */
	long    qe_date;                /* last mod date */
	char   *qe_to;                  /* output to this person */
	int	qe_jobnum;		/* jobnumber if specified.  -# or -x*/
	char	*qe_movqueue;		/* queue moving job to. -Q */
	char	*qe_movdev;		/* device moving to. -Q */
};




/*	params is the structure which records all the options received by
	enq.  Information derived from the params struct or derived in other
	ways is placed in the qe struct, (see above.)  The params struct is
	meant to record precisely no more and no less than what the user 
	specified on the command line (not including filenames.)
*/
struct params 
{
/* flags which take no arguments */
char 	
	p_Aflg, 		/* All status */
	p_Cflg, 		/* Mail instead of write for errors.*/
	p_Dflg, 		/* queue down */
	p_Gflg, 		/* die gracefully */
	p_Kflg, 		/* kill queue */
	p_Lflg,			/* long status (to qstatus)*/
	p_Uflg, 		/* bring up queue */
	p_Xflg,			/* cancel all */
	p_Yflg,			/* accept no more input (for lpd only) */
	p_cflg, 		/* copy */
	p_dflg,			/* force digest */
	p_eflg,			/* exclude status of another spooler's queues */
	p_nflg, 		/* notify */
	p_qflg, 		/* qstatus */
	p_rflg, 		/* remove file */
	p_sflg,			/* don't show files */
	p_jflg,			/* display job number */
	p_hflg,			/* hold a queued job */
	p_Hflg,			/* submit a job with hold */
	p_pflg,			/* release a held job */
	p_iflg,			/* local job status */

/* flags which require one argument */
	p_Bflg, 		/* burst pages ? */
	p_Mflg, 		/* message in file */
	p_Nflg, 		/* number of copies */
	p_Pflg,			/* Print queue */
	p_Rflg, 		/* priority */
	p_Tflg, 		/* title */
	p_Zflg,			/* pretend submitter is (only lpd uses this)*/
	p_aflg, 		/* alter priority */
	p_mflg, 		/* message on command line */
	p_numflg, 		/* job number */
	p_oflg, 		/* backend options */
	p_tflg, 		/* send TO */
	p_uflg,			/* user name to qstatus. */
	p_wflg, 		/* display wait on screen flag to qstatus*/
	p_xflg,			/* cancel */
	p_Qflg,			/* move */

/* arguments appended to the set above*/
	*p_Brem, 		/* n, h or b */
	*p_Mrem, 		/* filename where message is */
	*p_Nrem, 		/* Number of copies (int) */
	*p_Prem,		/* queuename */
	*p_Rrem, 		/* priority number to assign to file req. */
	*p_Trem, 		/* Title text */
	*p_Zrem,		/* pretend submitter is (only lpd uses this)*/
	*p_arem, 		/* alter priority to this number. */
	*p_mrem, 		/* message from command line */
	*p_numrem, 		/* job number for qstatus */
	*p_orem, 		/* optional args to backend */
	*p_trem, 		/* name to deliver to */
	*p_urem,		/* user name to qstatus. */
	*p_wrem, 		/* loop in qstatus */
	*p_xrem, 		/* job number for cancel */
	*p_Qrem;		/* destination queue for queue move */
};
