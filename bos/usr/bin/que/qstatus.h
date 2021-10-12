/* @(#)42	1.15.1.1  src/bos/usr/bin/que/qstatus.h, cmdque, bos41J, 9511A_all 3/7/95 11:43:13 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----Constants */
#define UDIGEST	1		/* Flags for readconfig() binary digestion */
#define CDIGEST	0
#define TPUTPTH "/usr/bin/tput"
#define OURARGS "AeiLqsP:#:u:w:"
#define ARGNOPQ 'e'
#define ARGALLQ 'A'
#define ARGVERB 'L'
#define ARGDEFQ 'q'
#define ARGSHRT 's'
#define ARGPRNT 'P'
#define ARGJOBN '#'
#define ARGUSER	'u'
#define ARGLOOP 'w'
#define ARGIRMJ 'i'

#define QMSGPROG	"QSTATUS"
#define QMSGJOBN	"Job number must be numeric between %d and %d"
#define QMSGPRNT	"Invalid printer name: %s"
#define QMSGUSE1	"[-AeiLqs] [-P<qname>] [-#<job number>]"
#define QMSGUSE2	"[-u<username>] [-w<delaysec>]..."
#define QMSGNOQD	"No queue devices detected."
#define QMSGFORK	"Process failure"
#define QMSGFRST	"RAW REMOTE STATUS (FILTER, %s NOT FOUND):"
#define QMSGBRST	"RAW REMOTE STATUS (USER SPECIFIED):"
#define QMSGERST	"END OF RAW REMOTE STATUS."
#define QMSGQREM	"(REM)"
#define QMSGBADO	"Invalid option: %s"
#define QMSGBADA	"Invalid argument: %s"

/*----For qstatus header display */
#define	QMSGQUE		"Queue"
#define QMSGDEV		"Dev"
#define QMSGSTAT	"Status"
#define QMSGJOB		"Job"
#define QMSGFILE	"Files"
#define QMSGUSER	"User"
#define QMSGPAGE	"PP"
#define QMSGBLKS	"Blks"
#define QMSGCOPS	"Cp"
#define QMSGRANK	"Rnk"
#define QMSGNAME	"Name"
#define QMSGFROM	"From"
#define QMSGTO		"To"
#define QMSGSUBM	"Submitted"
#define QMSGPRIO	"Pri"

/*----For  status field */
#define QMSGSTUNK	"UNKNOWN"
#define QMSGSTRDY	"READY"
#define QMSGSTRUN	"RUNNING"
#define QMSGSTDVW	"DEV_WAIT"
#define QMSGSTDWN	"DOWN"
#define QMSGSTOPW	"OPR_WAIT"
#define QMSGSTINI	"INITING"
#define QMSGSTSEN	"SENDING"
#define QMSGSTGET	"GET_HOST"
#define QMSGSTCON	"CONNECT"
#define QMSGSTQUE	"QUEUED"
#define QMSGSTBUS       "DEV_BUSY"
#define QMSGJBNEW	"NEW"
#define QMSGSHELD	"HELD"

/*----Data structures */
struct jobnum {			/* Linked list of user selected job numbers */
	struct jobnum	*jn_next;
	int		jn_num;
	boolean		jn_found;
	};

struct username {		/* Linked list of user selected user names */
	struct username	*un_next;
	char		*un_name;
	};

struct queptr {			/* Linked list of user selected device queues (FIFO) */
	struct queptr	*qp_next;
        struct queptr	*qp_prev;
	struct q	*qp_qptr;
	struct d	*qp_dptr;
	};

struct allparms {		/* all of the parameters grouped together */
	struct jobnum	*ap_jobs;
	struct queptr	*ap_queues;
	struct username	*ap_users;
	boolean		ap_allqflag;
        boolean		ap_verbose;
	boolean		ap_loopmode;
	unsigned	ap_delay;
	boolean		ap_sflag;
	boolean		ap_only_local_jobs;
	};
