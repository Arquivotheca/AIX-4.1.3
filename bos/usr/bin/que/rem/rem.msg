$ @(#)88	1.7.1.8  src/bos/usr/bin/que/rem/rem.msg, cmdque, bos411, 9428A410j 11/3/93 14:24:32
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
$set MS_REM
MSGLBQB		"0781-225 Error: libqb's log_init returned %d."
MSGCTRL		"0781-226 Could not make control file."
MSGSERV		"0781-227 Failure to hook up to remote server."
MSGDATA		"0781-228 Could not send datafile %s."
MSGMAKE		"0781-229 Could not make control file."
MSGSEND		"0781-230 Could not send control file."
MSGREST		"0781-231 Could not send restart."
MSGNORM		"0781-232 No remote host to connect to."
MSGUNKN		"0781-233 Unknown host %s."
MSGPTCP		"0781-234 Unknown service for printer/tcp."
MSGSOCK		"0781-235 Send request write error on socket %1$d %2$s."
MSGLOST		"0781-236 Lost connection reading socket."
MSGERWT		"0781-237 Error writing to fd = %d."
MSGGJOB		"0781-238 Error getting job number %d."
MSGSTAT		"0781-239 Status on %1$s failed.  Errno = %2$d."
MSGCOPN		"0781-240 Cannot open %s."
MSGESIZ		"0781-241 Error reading file or file changed size."
MSGWSOK		"0781-242 Could not write to socket while sending file."
MSGNACK		"0781-243 Could not send acknowledgement while sending file."
MSGACKN		"0781-244 File acknowledgement sent, but not received."
MSGCLLC		"0781-245 Cannot allocate memory: out of space."
MSGFATA		"FATAL ERROR"
MSGWARN		"WARNING"
MSGTFIL		"0781-246 Could not create tempfile in directory %1$s name %2$s."
MSGUNAM		"0781-247 Cannot determine username for uid = %d."
MSGSTFL		"0781-248 Status on %1$s failed.  Errno = %2$d."
MSGBSDC		"0781-249 Could not write BSD commands to %1$s on fd = %2$d."
MSGAIXC		"0781-250 Could not write commands to %s."
MSGSFIL		"0781-251 Some files not sent."
MSGBURS		"0781-252 Invalid burst page header value detected in statusfile, value = %d."
MSGBJOB		"0781-253 Bad job number %s."
MSGNPRT		"0781-254 No print server specified."
MSGNREM		"0781-255 No remote queue specified."
MSGCCAN		"0781-256 Cannot specify cancel job without job number or user."
MSGUSJN		"0781-257 Cannot specify users or job numbers except on cancel or status requests."
MSGDOWN		"%-7.7s       HOST_DOWN\n"
MSGUSE1		"[-qxLRX] [-u<username>] [-#<jobnum>] [-N<filter>] [-P<printer>]"
MSGUSE2		"[-S<server>] [-o [-cdfglmnptv] [-i<indent>]"
MSGUSE3		"    [-w<width>] [-h<title>] [1,2,3,4<font>]  ]"
$ MSGDOWN not used anymore 
MSGDWN2		"HOST_DOWN"

