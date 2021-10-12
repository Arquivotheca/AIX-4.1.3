$ @(#)83	1.36  src/bos/usr/bin/que/enq.msg, cmdque, bos411, 9428A410j 6/21/94 17:02:43
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the 
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_ENQ
MSGEQST		"0781-022 Cannot exec %s."
MSGNRD2		" (Submitting anyway)."
MSGULMT		"0781-024 Cannot write any more to %s, file has become too large."
MSGNOPN		"0781-025 Unable to open %s."
MSGROPN		"0781-026 Cannot re-open temp file %s."
MSGEPRT		"0781-027 Failure accessing %s."
MSGERDP		"0781-028 %s has no read permission."
MSGERMV		"0781-029 %s has no remove permission."
MSGENRF		"0781-030 %s: Cannot remove a nonregular file"
MSGPRAN		" (Printing it anyway)."
MSGEEFR		"0781-031 Bad stdin file request."
$ /* AIX security change 	*/
$ /* changed MSGROOT	*/
MSGROOT		"0781-032 You must belong to printq group."
MSGNOQD		"0781-034 Cannot open qdir %s."
MSGNORQ		"0781-035 No such request in queue %s -- perhaps it's done?"
MSGEFRR		"0781-036 Cannot fork to run backend.  Errno = %d."
MSGOOPN		"0781-037 Unable to open %s."
MSGNDEV		"0781-038 No device specified."
MSGQDED		"0781-039 Qdaemon appears to be dead."
MSGLDED		"0781-040 Local queue %s is dead -- no requests being accepted."

MSGRSPC		"0781-041 Cannot specify device with remote queue."
MSGNPRM		"0781-042 %s flag ignored; you must be an administrator for that operation."
MSGBARG		"0781-044 Priority out of range: %1$d (%2$d for system users)."
MSGILLB		"0781-046 Illegal burst page option."
MSGBADJ		"0781-047 Bad job number: %s."
MSGBADQ		"0781-048 Bad queue or device name: %s."
MSGNOPR		"0781-049 Not privileged to use -Z %s."
MSGBADP		"0781-050 Bad PRINTER or LPDEST env. variable %s."
MSGUSE1 	"[-cdehijnpqrsACDGHKLUX] [-#<JobNum>] [-B<nn,na,ng,an,aa,ag,gn,ga,gg>]"
MSGUSE2		"[-M<MessageFile>] [-N<NumCopies>] [-P<Printer>] [-Q<Queue>] [-R<Priority>]"
MSGUSE3		"[-T<Title>] [-Z<Name>] [-a<Priority>] [-m<Message>] [-o<Option>]"
MSGUSE4		"[-t<DestName>] [-u<Name>] [-w<Seconds>] [-x<JobNum>] [-Y]"
MSGPIPE         "0781-051 Unable to create pipe."
$ /* AIX security enhancement				*/
$ /* added MSGGIDX,MSGCHWN,MSGPERM,MSGRUID,MSGRGID	*/
MSGGIDX		"0781-052 Cannot get effective gid.  Errno = %d."
MSGCHWN		"0781-053 Cannot chown %1$s.  Errno = %2$d."
MSGPERM		"0781-054 Cannot set permissions on %1$s.  Errno = %2$d."
MSGRUID		"0781-055 Cannot get real uid."
MSGEGID		"0781-056 Cannot get effective gid."
MSGNORLQ	"0781-310 No such request in any local queue -- perhaps it's done?"
MSGNOFIL	"0781-312 File arguments are not allowed with the %s option."
MSGREMCAN	"0781-313 Cancel all not supported on remote queues."
MSGCOPY		"0781-296 Invalid number of copies:  %d."
MSGDIR		"0781-057 Cannot print directory:  %s."
MSGNOMEM	"0781-316 Memory allocation failure."
MSGNCLS		"0781-058 Unable to close file : %s."
MSGJOBNO	"Job number is: %d\n"
MSGGGRP		"0781-329 Cannot get user's list of groups."
MSGNOAC		"0781-330 Requested action not supported for remote side of queue."
MSGZLIMIT	"0781-349 -Z %s: Value cannot exceed 63 characters."
MSGFORK         "0781-072 Unable to fork a new process."
MSGPAL1         "0781-360 Cannot alter, hold, release, or move another spooler's job."
MSGPAL2         "0781-361 Cannot alter state of another spooler's queues."
MSGPAL3         "0781-362 Cancel all not supported on another spooler's queues."
MSGPAL4		"0781-363 Cancel operation failed."
MSGPAL5		"0781-364 Job submission failed."
MSGPAL6		"0781-365 Bad PDPRINTER environment variable %s."
MSGPAL7		"0781-366 Cannot determine name of host."
MSGPAL8		"0781-367 Invalid document format: -d%s."
MSGPAL9		"0781-368 Invalid page orientation: -z%s."
MSGPAL10	"0781-369 Invalid duplex selection: -Y%s."
MSGPAL11	"0781-370 Option not supported on another spooler's queues: %s."
MSGMISF		"0781-371 The %s flag requires job number (-#), queue name (-P), or user name (-u) be specified." 
MSGNOQUE 	"0781-372 There are no queues defined in the /etc/qconfig file."
