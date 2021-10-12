$ @(#)95	1.8.4.1  src/bos/usr/bin/que/qstat.msg, cmdque, bos41J, 9511A_all 3/7/95 11:43:19
$
$ COMPONENT_NAME: (CMDQUE) spooling commands
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1995
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$set 1
$quote "
$set MS_QSTAT
MSGQUE		"Queue"			/* Field no more than:	7 characters */
MSGSUBM		"Submitted"		/*			9 characters */
MSGUNK		"Unknown"
MSGSTRUN	"RUNNING"
MSGSTQUE	"QUEUED"
$ Do NOT change the order of any of the messages above.  They are used by shell scripts!
MSGJOBN		"0781-101 Job number must be numeric between %1$d and %2$d."
MSGPRNT		"0781-102 Invalid printer name: %s."
MSGBADO		"0781-103 Invalid option: %c."
MSGUSE1		"[-AeiLqs] [-P<printer>] [-#<job number>]"
MSGUSE2		"[-u<username>] [-w<delaysec>]..."
MSGNOQD		"0781-104 No queue devices detected."
MSGSFORK	"0781-105 Process failure."
MSGFRST		"0781-106 RAW REMOTE STATUS (FILTER, %s NOT FOUND):"
MSGBRST		"0781-107 RAW REMOTE STATUS (USER SPECIFIED):"
MSGERST		"0781-108 END OF RAW REMOTE STATUS."
MSGPAL1		"0781-109 Unable to list another spooler's queues."
MSGDEV		"Dev"			/* Field no more than:	5 characters */
MSGSTAT		"Status"		/* 			9 characters */
MSGJOB		"Job"			/*			3 characters */
MSGFILE		"Files"			/*			19 characters */
MSGUSER		"User"			/*			10 characters */
MSGPAGE		"PP"			/*			4 characters */
MSGBLKS		"Blks"			/*			5 characters */
MSGCOPS		"Cp"			/*			2 characters */
MSGRANK		"Rnk"			/*			3 characters */
MSGNAME		"Name"			/*			14 characters */
MSGFROM		"From"			/*			14 characters */
MSGTO		"To"			/*			14 characters */
MSGPRIO		"Pri"			/*			3 characters */
MSGSTUNK	"UNKNOWN"
MSGSTRDY	"READY"
MSGSTDVW	"DEV_WAIT"
MSGSTDWN	"DOWN"
MSGSTOPW	"OPR_WAIT"
MSGSTINI	"INITING"
MSGSTSEN	"SENDING"
MSGSTGET	"GET_HOST"
MSGSTCON	"CONNECT"
MSGJBNEW	"NEW"
MSGSTBUS	"DEV_BUSY"
MSGBADA		"0781-314 Invalid argument: %s."
MSGSHELD	"HELD"
MSGNQU          "0781-085 No queue %1$s in %2$s (name = %3$s/%4$s)."
MSGJBID		"0781-350 Job %d not found -- perhaps it's done?"
