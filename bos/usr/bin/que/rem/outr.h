/* @(#)78	1.9.1.2  src/bos/usr/bin/que/rem/outr.h, cmdque, bos411, 9430C411a 7/26/94 16:38:31 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*Remote backend OUTput headerfile*/

/*	the default printer for bsd systems.
	aix systems 
*/

#define		DEFLP		"lp" 
#define		MAXFILES	200  /* Maximum # of files on command line*/
#define		V2FILT		"aixv2short"

/*	comline is the structure which records all the options received by
	rembak.  Information derived from the comline struct or derived in 
	other ways is not placed in the this struct.
	Statusfile derived info has it's own struct, (sfile).
	Other derived info is in the job struct.
	
	NOTE--  IF THIS STRUCTURE IS CHANGED, REMBAK.H MUST BE ALSO.  REMBAK.H
		IS USED BY QDAEMON/ENQ.
*/
struct comline 
{
char 	
/* bsd flags which take no arguments */
	c_pflg,		/* use pr filter */
	c_Oflg,		/* PostScript files */
	c_fflg,		/* use fortran filter */
	c_cflg,		/* use cifplot filter */
	c_lflg,		/* use lfilter filter */
	c_tflg,		/* use troff filter */
	c_nflg,		/* use ditroff filter */
	c_dflg,		/* use tex filter */
	c_gflg,		/* use plot filter */
	c_vflg,		/* use varian filter */
	c_mflg,		/* use mail to notify of completion */

/* bsd flags which require one argument */
	c_iflg,		/* indent */
	c_wflg,		/* width */
	c_Tflg,		/* title for pr */
	c_1flg,		/* fonts 2 use */
	c_2flg,
	c_3flg,
	c_4flg,

/* aix flags which take no arguments */
	c_qflg, 	/* qstatus */
	c_xflg,		/* cancel */
	c_Lflg, 	/* long qstatus */
	c_Nflg,		/* not aix */
	c_Rflg,		/* restart */
	c_Xflg,		/* send -o flags, regardless */

/* aix flags which require one argument */
	c_Pflg,		/* Print queue */
	c_Sflg, 	/* printserver is */
	c_oflg, 	/* options to pass across */
	c_uflg, 	/* username */
	c_numflg, 	/* job number */
	c_Mflg,		/* Operator message
			   Be careful, the message may have blanks in it.
			   This is not a BSD flag, but is a remback option.
			   It MUST be preceeded by a -o when invoking rembak */
	c_Hflg,		/* class name */
	c_TMflg,	/* Timeout wait period for ACKs - in minutes */

/* aix arguments appended to the set above*/
	*c_Nrem,		/* Qstatus filter */
	*c_Prem,		/* Print queue */
	*c_Srem, 		/* printserver is */
	*c_orem, 		/* options to pass across */
	*c_urem, 		/* username */
	*c_numrem, 		/* job number */
	*c_Mrem,		/* Operator message string */
	*c_Hrem,		/* class name */

/* bsd arguments appended to the bsd set way up above*/
	*c_irem,
	*c_wrem,
	*c_Trem,
	*c_1rem,
	*c_2rem,
	*c_3rem,
	*c_4rem;
};

struct job
{
	struct users	*j_u;
	struct jobnum	*j_jn;
	struct aixcmds	*j_ac;
	char		*j_jobname;
};



struct sfil
{
	unsigned s_head;	/* header? */
	unsigned s_trail;	/* trailer? */
	boolean s_mailonly;	/* trailer? */
	char s_from[S_FROM];	/* and from this one */
	char s_title[S_TITLE];	/* title of this job */
	char s_to[S_TO];	/* output for this person */
	int s_copies;		/* number of copies */
	char s_cmdline[S_CMDLEN];  /* enq command line options for this job */
};

struct users 
{			/* linked list of user names */
	struct users	*u_next;
	char		*u_user;
};

struct jobnum 
{			/* linked list of user selected job numbers */
	struct jobnum	*jn_next;
	int		jn_num;
};

struct aixcmds 
{			/* linked list of aix commands*/
	struct aixcmds	*ac_next;
	char		*ac_opt;
};

