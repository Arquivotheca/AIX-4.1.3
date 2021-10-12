$ @(#)89	1.19  src/bos/usr/bin/que/libque/libq.msg, cmdque, bos411, 9428A410j 11/3/93 14:23:57
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
$set MS_LIBQ
MSGFORM		"0781-109 Filename format is bad."
MSGUNKN		"0781-110 Unknown"
MSGNOLG		"0781-111 No login name found."
MSGFATA		"FATAL ERROR"
MSGWARN		"WARNING"
MSGOUTO		"0781-112 Insufficient space for file retention."
MSGOUTS		"0781-113 Insufficient memory."
MSGERCD		"0781-114 Cannot cd to %s."
MSGPIPE		"0781-115 Digest pipe error."
MSGFORK		"0781-116 Cannot fork for %s."
MSGEXEC		"0781-117 Cannot exec digester %s."
MSGWPID		"0781-118 Redigest wait: wrong pid."
MSGDIGE		"0781-119 Error from digester %1$s, status = %2$d, rv = %3$d."
MSGEDIR		"0781-120 Error %1$d writing directory %2$s."
MSGCTMP		"0781-121 Can't create temporary file."
MSGUHST		"0781-122 Unable to get host name."
MSGXERR		"0781-123 %1$s: Failure to exec %2$s."
MSGFERR		"0781-124 %s: Failure to fork process."
MSGSEEK		"0781-125 Lseek failure in %s."
MSGREAD		"0781-126 Read failure in %s."
MSGCREA		"0781-127 Failure to create %s."
MSGOPEN		"0781-128 Failure to open %s."
MSGOMSG		"0781-132 Can't open message file %s."
MSGNJOB		"0781-133 Job number record missing in Job Description File."
MSGRJOB		"0781-134 Job number record must contain 1 element in Job Description File."
MSGNONN		"0781-136 JDF: Job number is nonnumeric or 0."
MSGMREC		"0781-137 Request info record missing in Job Description File."
MSGSEV 		"0781-138 Request info record must contain 7 elements in Job Description File."
MSGMISS		"0781-139 Request size record missing in Job Description File."
MSGSIZR		"0781-140 Size record must contain 4 elements in Job Description File."
MSGMLOG		"0781-141 LOG record missing in Job Description File."
MSGTWO		"0781-142 LOG record must contain 2 elements in Job Description File."
MSGNAME		"0781-143 Request Name record missing in Job Description File."
MSGUSER		"0781-144 For-User record missing in Job Description File."
MSGHOST		"0781-145 Secondary Host Name record missing in Job Description File."
MSGBLNK		"0781-146 Secondary Host Name record blank in Job Description File."
MSGDATE		"0781-147 Modification Date record missing in Job Description File."
MSGMESS		"\n0781-148 Message from host %1$s user %2$s...\n\n"
MSGOPER		"0781-149 Operator Message Count record missing in Job Description File."
MSGINTG		"0781-150 Operator Message Count record is not integer."
MSGMBAD		"0781-151 Operator Message record is bad." 
MSGBFAD		"0781-152 Bad filename in JDF.  Should have 2 fields."
MSGQOPN		"0781-153 Qdaemon couldn't open Job Description File."
MSGBFMT		"0781-154 Entry %1$s/%2$s: bad format.\n Name changed to %3$s."
MSGCFGF		"0781-156 Cannot find config file %s."
MSGNOPN		"0781-157 Cannot open %s for reading."
MSGNADA		"0781-158 Nothing read from %1$s.  Errno = %2$d."
MSGRERR		"0781-159 Read error %1$d on %2$s."
MSGCORU		"0781-160 Corrupted %1$s, belength = %2$d."
MSGACCS		"0781-161 Argument %s is not an accessible file."
MSGWAKE		"0781-162 Cannot awaken qdaemon (request accepted anyway)."
MSGQLNK		"0781-163 %1$s qlink link error.  Errno = %2$d."
MSGGETH		"0781-166 Could not get host name."
MSGGPCR         "0781-167 Could not get process credentials.  Errno = %d."
MSGGPNV		"0781-168 Could not get process environment.  Errno = %d."
MSGPCRD		"0781-169 pcred list missing in Job Description File."
MSGPENV	 	"0781-170 penv list missing in Job Description File."
MSGULGD		"0781-171 Cannot get user's login directory.  Errno (getuidx) = %d."
MSGULGN		"0781-172 Cannot get user's log name.  Errno (getuidx) = %d."
MSGPERM		"0781-173 Cannot set permissions on %1$s.  Errno = %2$d."
MSGQSIG		"0781-174 Cannot signal qdaemon.  Errno = %d."
MSGEGID		"0781-175 Cannot get effective gid.  Errno = %d."
MSGCHWN		"0781-288 Cannot chown %1$s.  Errno = %2$d."
MSGNLCK		"0781-289 Cannot lock %s for reading."
MSGUSAGE	"usage"
MSGNOCO		"0781-216 %1$s: %2$s could not be opened for error message output.\n%3$s"
MSGCMD		"0781-317 Command Line record missing in Job Description File."
MSGSNDERR       "Error in msgsnd.  Errno = %d."
MSGNOPEN	"Error opening temporary file %s."  

