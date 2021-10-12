/* @(#)21	1.8  src/bos/usr/include/diag/da.h, cmddiag, bos412, 9445C412a 11/9/94 10:22:02 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DA
#define _H_DA

#include <sys/cfgdb.h>

#define MAXFRUS 4
#define FRUB1	4
#define FRUB2	5
#define FRUB_ENCLDA	6

#define NOT_IN_DB	0
#define DA_NAME		1
#define PARENT_NAME 	2
#define CHILD_NAME 	3
#define	NO_FRU_LOCATION	4

/*	Return codes from diagex routines	*/

#define	CHILD_UNCONFIGURE_ERROR		1 /* Child cannot be unconfigured  */
#define	DEVICE_UNCONFIGURE_ERROR	2 /* Device cannot be unconfigured */
#define	DEVICE_DIAGNOSE_ERROR		3 /* Device cannot be put in       */
					  /* diagnose state.		   */
#define	DEVICE_DEFINE_ERROR		4 /* Device cannot be put in 	   */
					  /* define state.		   */
#define	DEVICE_CONFIGURE_ERROR		5 /* Device cannot be configured   */
#define	CHILD_CONFIGURE_ERROR		6 /* Child cannot be configured    */


#define	EXEMPT		3
#define	NONEXEMPT	4

#ifndef 	NULL
#define		NULL	0
#endif

#ifndef 	TRUE
#define		TRUE	1	
#endif

typedef struct
{
	int	conf;			/* probability of failure	*/
	char	fname[NAMESIZE];	/* FRU name			*/
	char	floc[LOCSIZE];		/* location of fname		*/
	short	fmsg; 			/* text message number for fname*/
	char    fru_flag;		/* flag used by DA		*/
	char	fru_exempt;
}fru_t;

struct	fru_bucket
{
	char	dname[NAMESIZE];	/* device name			*/
	short   ftype;	/* FRU bucket type added to the system       */
	short	sn;	/* Source number of the failure              */
	short	rcode;	/* reason code for the failure               */
	short	rmsg;	/* message # in /etc/lpp/diagnostics/$LANG/dcda.cat */
	fru_t 	frus[MAXFRUS];

};

#endif
