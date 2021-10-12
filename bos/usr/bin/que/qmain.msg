$ @(#)82	1.26.1.5  src/bos/usr/bin/que/qmain.msg, cmdque, bos411, 9428A410j 11/3/93 14:23:29
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_QMAIN
MSGREXE		"0781-059 Unable to re-exec myself."
MSGSDIE		"0781-061 Starting to die at request of %s."
MSGSOPN		"0781-062 Error opening %1$s.  Errno = %2$d."
MSGSRED		"0781-063 Error reading %1$s.  Errno = %2$d."
MSGSSEK		"0781-064 Seek error on file %1$s.  Errno = %2$d."
MSGSWRT		"0781-065 Error writing to %1$s.  Errno = %2$d."
MSGBSPC		"0781-066 Bad special request switch in file %s."
MSGUNDR		"0781-067 Special request `%s' not understood."
MSGBJOB		"0781-068 Job number (%s) out of range in (cancel())."
MSGBALT		"0781-069 Job number (%s) out of range in (alter())."
MSGEPRI		"0781-070 Could not change job %1$s to new priority %2$d because %3$s."
MSGMHDR		"0781-071 Message from qdaemon:"
MSGFORK 	"0781-072 Unable to fork a new process."
MSGEXEC 	"0781-073 Unable to send message to remote user."
MSGQEX1		"0781-075 Qdaemon couldn't exec %1$s.\nErrno = %2$d: %3$s"
MSGPENV		"0781-077 Failure creating environment variable."
MSGFKWT		"0781-078 Cannot fork write process.  Errno = %d."
MSGWEXC		"0781-079 Qdaemon cannot exec write -r.  Errno = %d."
MSGWPID		"0781-080 Wrong pid received while waiting for operator message."
MSGOPCN		"0781-081 Operator canceled your job."
MSGFQDR		"0781-083 Failure to read queue directory."
MSGCORE		"0781-084 Core image in qdir directory (moving to .core)."
MSGNOQU		"0781-085 No queue %1$s in %2$s (name = %3$s/%4$s)."
MSGBOMB		"0781-086 Backend `%1$s'\nfailed with termination status 0%2$o."
MSGRQST		"0781-087 Could not run your request %1$s for device %2$s."
MSGDIED		"0781-088 Device %1$s died running request %2$s which is still queued."
MSGFINI		"0781-090 Request %1$s has finished on device %2$s."
MSGSENT		"0781-091 Request %1$s sent to you by %2$s\nhas finished on device %3$s."
MSGGJDF		"0781-093 Bad getjdf on name %s."
$ /* AIX security enhancement 	*/
$ /* added MSGSPCR              */
MSGSPCR         "0781-094 Could not set process credentials (pcred).\nErrno = %1$d:%2$s"
$ /* added MSGPRIV MSGPERM */
MSGPRIV		"0781-095 Could not set privilege.\nErrno = %1$d: %2$s"
MSGPERM		"0781-096 Could not set permissions on %1$s.  Errno = %2$d."
MSGRUID		"0781-097 Cannot get real uid.\nErrno = %1$d: %2$s"
MSGEGID		"0781-098 Cannot get effective gid.  Errno = %d."
MSGCHWN		"0781-099 Cannot chown %1$s.  Errno = %2$d."
MSGCREA		"0781-100 Failure to create %s."
MSGTERS		"0781-290 Terminated due to signal."
MSGTERN		"0781-291 Terminated normally."
MSGDELETE	"Job number %d has been deleted from the queue."
MSGCHLD		"0781-294 Problem in main queueing loop, Errno = %d."
MSGNOPN		"0781-297 Cannot open %1$s for writing.\nErrno = %2$d: %3$s"
MSGNLCK		"0781-298 Cannot lock %1$s for writing.\nErrno = %2$d: %3$s"
MSGEXIT		"0781-300 Write failed, exit = %d."
MSGSIG		"0781-301 Write failed, signal = %d."
MSGSTOP		"0781-302 Write stopped, signal = %d."
MSGSTDO		"0781-303 File, %s opened for backend, but not given to standard out.  Job terminated."
MSGUNAB		"0781-304 Unable to open %s as standard out for backend."
MSGRTRY		"0781-305 Retrying..."
MSGSUCO		"0781-306 Successfully opened %s as standard out for backend."
MSGSWR2		"0781-307 Error writing to %1$s.\nErrno = %2$d: %3$s"
MSGBJDEL	"0781-299 Job %1$d (%2$s) with invalid device name has been deleted."
MSGSRE2		"0781-308 Error reading %1$s.\nErrno = %2$d: %3$s"
MSGPGID		"0781-311 Cannot set process group id.\nErrno = %1$d: %2$s"
MSGBADFILE	"0781-312 Request %1$s removed from queue %2$s.  Could not open or stat file(s)."
MSGERMV         "0781-313 Unable to remove file: %s."
MSGMAXRTRY	"0781-340 Request %1$s removed from queue %2$s.  Failed initial attempt and %3$d retries."
MSGGETERR       "0781-341 Unable to create message queue.  Errno = %d."
MSGRCVERR       "0781-342 Error in message receive.  Errno = %d."
MSGRUNH		"0781-343 Job already running, unable to hold job number %d."
MSGEHOL		"0781-344 Could not change job %s hold/release status."
MSGNOENT	"0781-345 Could not find job %d in the queue."
MSGARUN		"0781-346 Job %s is already running unable to move the job."
MSGTIME		"0781-347 Could not change job %1$s to new time %2$d."
MSGPIPE         "0781-348 Unable to create pipe."
