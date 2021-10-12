$ @(#)30	1.12.3.1  src/bos/usr/bin/que/rem/lpd.msg, cmdque, bos411, 9436C411a 9/7/94 20:27:47
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
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_LPD
MSGUSE1		"[-d] [-l]"
MSGSPOO		"0781-215 cannot access spool directory, exiting."
MSGLOCK		"0781-196 lock file or duplicate daemon."
MSGPTCP		"0781-197 printer/tcp is an unknown service."
MSGBIND		"0781-198 unable to name socket."
MSGSOCK		"0781-199 unable to create socket."
MSGACPT		"0781-200 failure to accept a socket connection."
MSGILLF		"0781-201 ill-formed FROM address."
MSGLINE		"0781-202 your host does not have line printer access."
MSGTLNG		"0781-203 command line too long."
MSGLOST		"0781-204 lost connection."
MSGRQST		"0781-205 request %1$s %2$s."
MSGBRQS		"0781-206 bad request (%d)."
MSGILLG		"0781-207 illegal service request."
MSGUNKN		"0781-208 unknown printer %s."
MSGNOPT		"0781-209 option not supported."
MSGFORK		"0781-210 unable to fork process."
MSGEXEC		"0781-211 could not exec /bin/enq."
MSGPROT		"0781-212 protocol error."
MSGEWRT		"0781-213 error writing to %s."
MSGDETH		"0781-214 terminating."
MSGRQUE		"0781-219 no filename: request from %s not printed."
MSGCRED		"0781-221 could not set process credentials."
MSGSRC1		"0781-222 getsockname(), SRC not found, continuing without SRC support. Errno = %d."
MSGSRC2		"0781-223 Error in recvfrom().  Errno = %d."
MSGSRC3		"NOTICE: lpd does not support this option.\n"
MSGSRC4		"Trace"
MSGSRC5		"Active"
MSGSRC6		"Inactive"
MSGSRC7		"Debug"
MSGSRC8		"Signal"
MSGSRC9		"Purpose"
MSGSRC12	"SIGTERM"
MSGSRC13	"Terminates lpd"
MSGSRC14	"SIGHUP"
MSGSRC15	"Signal ignored"
MSGSRC16	"SIGQUIT"
MSGSRC17	"SIGCLD"
MSGSRC18	"hosts.lpd Hostname"
MSGBADH		"WARNING:  %.20s:  host name too long."
MSGBADF		"Invalid filename sent."
MSGOWN		"Failed to change ownership of file %1$s to %2$d"
