$ @(#)45	1.29.1.4  src/bos/usr/lib/sendmail/sendmail.msg, cmdsend, bos411, 9428A410j 3/24/94 08:51:40
$
$ COMPONENT_NAME: CMDSEND sendmail.msg 
$  
$ FUNCTIONS:  
$
$ ORIGINS: 10  26  27  
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
$ All Rights Reserved 
$ Licensed Materials - Property of IBM 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ messages for sendmail 
$

$len 
$set MS_SENDMAIL        Define initial set#
$
$quote "         define quote character.
$
$      The messages have been assigned a range of 001-265.

$      It is recommended that single lines of English text
$      do not be broken up into multiple lines when translated.
$      Do not translate %s, %d, %c, %1$s, etc.
$      These will be replaced by character strings, numbers, etc.  
$      Do not translate sendmail.
AL_LONG              "sendmail: 0832-001 Alias file name \%s\, is too long."
$       Alias, in this and the following messages, is a short way of writing
$       a longer name.
AL_ELOCK             "sendmail: 0832-002 Cannot open alias lock file %s"
AL_ELOCK2            "sendmail: 0832-003 Cannot lock alias lock file %s"
MN_ALFORCE           "Trying to build aliases data base %s"
MN_ALOK              "Aliases database %s was created"
AL_EINIT             "sendmail: 0832-004 Cannot open aliases database %s"
AL_INC               "sendmail: 0832-005 %s is not complete."
AL_NOPATH            "sendmail: 0832-006 The alias file name is not defined."
AL_ELONG             "sendmail: 0832-007 Alias file name %s is too long."
AL_EULINK            "sendmail: 0832-008 %s cannot be removed."
AL_EMAKE             "sendmail: 0832-009 %s cannot be made."
AL_EAVAIL            "sendmail: 0832-010 Alias file %s is not available."
AL_ESPACE            "sendmail: 0832-011 Alias definition line cannot begin with a space."
AL_ECOLON            "sendmail: 0832-012 There is a missing colon."
AL_ENAME             "sendmail: 0832-013 An alias name is not valid."
AL_EADDR             "sendmail: 0832-014 %s is not a valid address."
AL_EREAD             "sendmail: 0832-015 Cannot read alias file %s."
AL_ELOCAL            "sendmail: 0832-016 Cannot define an alias for names that are not local."
AL_EDBM              "sendmail: 0832-017 There is a Dbm store error."
$ Do not translate Dbm.  
AL_ELOCK3            "sendmail: 0832-018 Cannot open alias lock file %s"
AL_ELINK             "sendmail: 0832-019 Cannot link %1$s to %2$s"
AL_NBYTES            "There are %1$d aliases. The longest is %2$d bytes, with %3$d total."
AL_FRWRD             "forwarding"
$       Forwarding describes the act of sending a message.
$	Do not translate the word "forward"
AL_FORW		     "forward: no home"
CO_ECREAT            "sendmail: 0832-020 Cannot create %s"
CO_ECLOSE            "sendmail: 0832-021 There is a close from %s that is not expected. "
CO_ECLOSE2           "sendmail: 0832-022 There is a close from %s that was not expected: %m\n"
CO_EREOPEN           "sendmail: 0832-023 Cannot open %s "
CO_EDEL              "\nMail deleted because of lack of disk space.\n\n"
CO_ESPACE            "452 Out of disk space for a temporary file"
$       NOTHING can precede the three-digit code at the beginning of the message. 
CO_EWRITE            "sendmail: 0832-024 Cannot write to %s"
CF_EUSER             "sendmail: 0832-025 Cannot find current user name."
CF_EDOMAIN           "sendmail: 0832-026 There is a character in the host or domain name %s that is not valid."
CV_ZERO              "zero seconds"
CV_WEEK              ", %d week"
CV_WEEKS             ", %d weeks"
CV_DAY               ", %d day"
CV_DAYS              ", %d days"
CV_HOUR              ", %d hour"
CV_HOURS             ", %d hours"
CV_MIN               ", %d minute"
CV_MINS              ", %d minutes"
CV_SEC               ", %d second"
CV_SECS              ", %d seconds"
DM_SIG               "sendmail: 0832-027 getrequest:sigvec:SIGCLD"
$      Do not translate.
DM_POPEN             "%1$s: 0832-028 Cannot popen: %2$s\n\n"
$       Do not translate "popen".  
DM_STAT              "%s SRC status"
$       Do not translate "SRC".
$       %s is replaced by "sendmail".
$       Do not add message/component id.
$       Keep it very short.
DM_FORK              "%1$s: 0832-029 Cannot create a process: %2$s\n"
DM_FREAD             "\n%1$s: 0832-238 Cannot read from mailstats: %2$s"
$       Do not translate "mailstats".  
DM_NOTSUP            "sendmail: 0832-030 The option is not supported."
DM_EDUP              "sendmail: 0832-031 The system call dup failed."
$       Do not translate dup.
DM_ESVR              "sendmail: 0832-032 smtp is not a known server."
$       Do not translate "smtp".  
DM_SOPT              "sendmail: 0832-033 The system call setsockopt failed."
$      Do not translate "setsockopt".  
DM_SMTP              "sendmail: 0832-034 smtp is not a known server."
$       Do not translate "smtp".  
DM_ESOCK             "sendmail: 0832-035 Cannot create a socket"
$       Do not translate socket in this and the following messages.
DM_ESOCK2            "sendmail: 0832-036 Cannot create a socket"
DM_EBIND             "sendmail: 0832-037 Cannot bind a socket to the port %d"
DM_BIND2             "sendmail: 0832-038 Cannot bind a socket to port %d"
DM_LISTEN            "sendmail: 0832-039 The system call listen failed."
$       Do not translate listen.
DM_LISTEN2           "sendmail: 0832-040 The system call listen failed."
DM_ACC               "sendmail: 0832-041 The system call accept failed."
$       Do not translate accept.
DM_SEL               "sendmail: 0832-042 The system call select failed."
$       Do not translate select.
DM_RECV              "sendmail: 0832-043 The system call recvfrom failed."
$      Do not translate "recvfrom".  
DM_EFORK             "sendmail: 0832-044 Cannot create a process"
DM_CONN              "The current pid is %d."
$       pid is a process identifier.
DM_EDOMAIN           "sendmail: 0832-045 %s is not a valid numeric domain specification."
DM_MCONN             "makeconnection: Perform the gethostbyname\n"
$       Do not translate makeconnection.
$       Do not translate gethostbyname.
DM_UNKN              "sendmail: 0832-046 smtp is not a known server"
$      Do not translate "smtp".  
DM_ESOCK3            "sendmail: 0832-047 Cannot create a socket"
$       Do not translate socket.
DM_FUNNY             "sendmail: 0832-048 There is a failure at address %1$lx, port %2$x"
DL_QUEUED            "Queued"
DL_ENPARM            "sendmail: 0832-049 There are too many parameters to %s before the $u macro."
DL_ELARGE            "sendmail: 0832-050 Do not send a message of more than %ld bytes."
DL_OFLOW             "sendmail: 0832-051 There are too many parameters to %s after the $u macro."
DL_NOCLEVER          "The IPC mailer is not using smpt"
$       Do not translate "IPC", smtp, mailer.  
DL_EPIPE             "sendmail: 0832-052 Cannot open pipe to mailer"
$       Do not translate "mailer", "pipe" in this and the next message.  
DL_EPIPE2            "sendmail: 0832-053 Cannot open pipe from mailer"
DL_EFORK             "sendmail: 0832-054 Cannot create a process"
DL_EDUP              "sendmail: 0832-055 System call dup failed."
$       Do not translate dup.
DL_GID               "000 openmailer: The default user number is %1$d; the default group number is %2$d."
$       NOTHING can precede the three-digit code at the beginning of the message. 
$ Do not translate\t"openmailer".  
DL_DGID              "000 openmailer: Setting the default user number to %1$d and group number to %2$d"
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       Do not translate "openmailer".  
DL_EGID              "sendmail: 0832-056 Cannot set the group number or the user number or the group access list."
DL_CTLADDR           "000 openmailer: Setting the ctladdr user number to %1$d and group number to %2$d"
$       NOTHING can precede the three-digit code at the beginning of the message.
$       Do not translate "openmailer", "ctladdr".  
DL_EFCNTL            "sendmail: 0832-057 The system call fcntl failed."
$       Do not translate "fcntl".  
DL_EXECVE            "000 execve: The user number is %1$d; the group number is %2$d."
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       Do not translate "execve".  
DL_EEXEC             "sendmail: 0832-058 Cannot run %s"
DL_STAT              "Processed message %1$s to %2$s; delay is %3$s and status is %4$s."
$       %2$s will be replaced by a recipient name, and %4$s will be replaced by a
$       phrase that describes the state eg. message DL_QUEUED.
DL_ENOBANG           "sendmail: 0832-059 There is a missing ! character in the uucp address %s."
$       Do not translate uucp.
DL_FROM              "From %1$s  \001d remote from %2$s\n"
$       Do not translate.
DL_EOPEN             "sendmail: 0832-060 Cannot open %s for %s from %s."
DL_NOMSG             "--- No message was collected ---"
DL_EPUTBODY          "sendmail: 0832-061 There is a read error"
$       Do not translate read or write in the next two messages. They are function      calls.
DL_EPUTBODY2         "sendmail: 0832-062 There is a write error"
DL_EHOPS             "sendmail: 0832-063 Too many hops %d (max is %d) from %s, to %s.\n"
$       Do not translate hops.
DL_DELIVERABLE       "can be delivered"
$       This will follow a message name.
EN_DROP              "Completed processing message %1$s with flags=%2$o and pid=%3$d."
$       Do not translate pid.
EN_NONE		"(no message id)"
$			This is inserted into %1$s in EN_DROP, above, when no
$			message identifier is present.
EN_ECREATE           "sendmail: 0832-064 Cannot create %s"
EN_EFFLAG            "sendmail: 0832-065 %s: Do not use the -f flag"
DL_EPARSE1           "sendmail: 0832-066 Cannot get address for From %1$s at host %2$s"
$       Do not translate From. 
DL_EPARSE2           "sendmail: 0832-067 Cannot get address for From user %2$s. Requested by user %1$s"
DL_EPOSTMAST         "sendmail: 0832-068 Cannot get address for postmaster"
$       Do not translate postmaster.
EN_EUID              "000 setsender: A user number or a group number is not valid."
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       Do not translate "setsender".  
EN_PRESCAN           "sendmail: 0832-069 The prescan failed on the address %s"
$       Do not translate prescan, from.
DL_LINE              "%1$s: line %2$d: "
ER_DUR               " during "
$       During is preceded by an error condition, and followed by a description
$       of the current state of the system.
ER_WITH              " with "
$       With is preceded by an error condition, and followed by a host name.
ER_EHOST             "Host %s is down."
EN_ECONN             "The connection is refused by %s"
EN_EHOST             "There is a Host Name Lookup Failure"
EN_ERROR             "Error number %d."
HD_ESYNTAX           "sendmail: 0832-070 There is a syntax error on line %s."
DH_STATUS            "Received message %1$s from user %2$s at host %5$s: size is %3$ld and class is %4$d."
MN_ESETUID           "sendmail: 0832-071 Sendmail must run setuid/setgid as %s"
$       Do not translate Sendmail, setgid, setuid.  
MN_ESIGINT           "sendmail: 0832-072 SIGINT: There is a  sigvec failure"
$       Do not translate "SIGINT".  
$       Do not translate sigvec in the next 9 messages.  
MN_ESIGINT2          "sendmail: 0832-073 SIGINT: There is a sigvec handler failure"
MN_ESIGTERM          "sendmail: 0832-074 SIGTERM: There is a sigvec failure"
$       Do not translate "SIGTERM".  
MN_ESIGPIPE          "sendmail: 0832-075 SIGPIPE: There is a sigvec failure"
$       Do not translate "SIGPIPE".  
MN_ESIGCLD           "sendmail: 0832-076 SIGCLD: There is a sigvec failure"
$       Do not translate "SIGCLD".  
MN_ESIGCLD2          "sendmail: 0832-077 SIGCLD: There is a sigvec ignore failure"
$       Do not translate\t"SIGCLD".  
MN_DSIGHUP           "sendmail: 0832-078 SIGHUP: There is a sigvec ignore failure"
MN_DSIGINT           "sendmail: 0832-079 SIGINT: There is a sigvec ignore failure"
$       Do not translate "SIGINT".  
MN_DSIGQUIT          "sendmail: 0832-080 SIGQUIT: There is a sigvec ignore failure"
$       Do not translate "SIGQUIT".  
MN_EUNAME            "sendmail: 0832-081 The uname system call failed"
$       Do not translate "uname".  
MN_EMODE             "sendmail: 0832-082 Operation mode is already set."
MN_EMODE2            "sendmail: 0832-083 Specify an operation mode after the -b flag."
MN_EMODE3            "Cannot specify this operation mode for: %s"
MN_EMODE4            "sendmail: 0832-084 The operation mode flag has no associated password."
MN_EMODE5            "sendmail: 0832-085 %c is not a valid operation mode."
MN_ECFPATH           "sendmail: 0832-086 The -C flag has no associated path."
MN_ECF               "sendmail: 0832-087 Specify only one -C flag."
MN_ECFABS            "sendmail: 0832-088 The configuration file path name must be absolute."
MN_ECONF             "sendmail: 0832-089 Specify the configuration file. %s"
MN_VERSION           "Version %s.\n"
MN_EDEBUG            "sendmail: 0832-090 Debug mode is not available."
MN_ENOADDR           "sendmail: 0832-091 The -f flag does not have an associated address."
MN_EREENTER          "main: reentered!"
MN_EFFLAG            "sendmail: 0832-092 Specify the -f flag only once."
MN_EFFLAG2           "sendmail: 0832-093 The -F flag does not have an associated name."
MN_EFFLAG3           "sendmail: 0832-094 Specify the -F flag only once."
MN_EHFLAG            "sendmail: 0832-095 The -h flag does not have an associated hop count."
$       Do not translate hop.
MN_EHFLAG2           "sendmail: 0832-096 Specify the -h flag only once."
MN_EQUEUE            "sendmail: 0832-097 Queue mode is already specified."
$       Do not translate queue.
MN_QUEUE             "sendmail: 0832-098 Cannot specify queue process. %s"
$       Do not translate queue.
MN_EFLAG             "sendmail: 0832-099 %s is not a known flag."
MN_NLSCF             "NLS configuration database %sDB has been created."
$       Do not translate "NLS" 
MN_UNDHOST           "sendmail: 0832-100 Local host name is not defined."
MN_SRCMODE           "sendmail: 0832-101 The -bd flag is required with SRC."
$       Do not translate "SRC" 
MN_ECF2              "sendmail: 0832-102 Configuration database %sDB is not valid."
MN_CF                "Created the %sDB configuration database."
MN_CFFORCE           "Freezing configuration database %sDB."
$       Freezing is a technical term for compiling.
MN_ELOCAL            "sendmail: 0832-103 No local mailer is defined in the configuration file."
$       Do not translate local.
MN_EPROG             "sendmail: 0832-104 No program mailer is defined in the configuration file."
$       Do not translate program.
MN_EALIAS            "sendmail: 0832-105 Cannot create the %sDB alias database"
$       Alias is a short way of writing a longer name or address.
MN_ALUPDT            "Alias database update"
MN_FRCONF            "Freeze configuration file"
$       Freeze is a technical term for compile.
MN_ALIAS             "Created the %sDB alias database."
MN_EALIAS2           "sendmail: 0832-106 Alias database %sDB is not valid."
MN_ADDR              "Address Test Mode:\nEnter <ruleset> <address>\n"
$       Do not translate "ruleset".  
MN_EFORK             "sendmail: 0832-107 Cannot create a process"
MN_DAEMON            "The daemon or queue process is started%s. The pid is %d."
$       Do not translate "daemon", pid.
$       Queue is a technical term.  
MN_SRC               " by SRC"
$       Do not translate SRC.
MN_ERECIP            "sendmail: 0832-108 Specify the recipient names."
MN_EQDIR             "sendmail: 0832-109 Cannot change to or create the queue directory %s"
$       Do not translate queue in this and the following messages.
MN_EQDIR2            "sendmail: 0832-110 The queue directory path name %s is too long."
MN_ELOCK             "sendmail: 0832-111 Cannot open the lock file %1$s/%2$s."
MN_ESEM              "sendmail: 0832-112 The ftok system call failed on file %1$s/%2$s"
$       Do not translate "ftok".  
MN_EATTACH           "sendmail: 0832-113 Cannot get the queue semaphore for %s"
$       Semaphore is a technical term.
MN_EQSMALL           "sendmail: 0832-114 The queue semaphore value is too small; directory %s"
MN_EMGR              "Notify the System Administrator."
MN_ESEMGET           "sendmail: 0832-115 The semget system call failed on directory %s"
$       Do not translate "semget".  
MN_SEMOP             "sendmail: 0832-116 The semop system call failed on sid %1$d and directory %3$s; failure code %2$d"
$       Do not translate "semop", sid.  
MN_ESEMSIG           "sendmail: 0832-117 The semop system call failed on sid %1$d and directory %3$s; failure code %2$d"
MN_SMNAME            "Sendmail: "
$       Do not translate. Please leave 1 space after the colon.
MN_FINI              "Completed. The process identifier is %d."
MN_BACK              "Running in background. The process identifier is %d."
MN_TDELTA            "t_delta: %1$s. The delta is %2$d.\n"
$       Do not translate "t_delta".  
MN_EHOST             "sendmail: 0832-118 A character in the local host name %s is not valid."
MN_EDOM              "sendmail: 0832-119 A character in the local domain name %s is not valid."
PR_ENAME             "Cannot resolve name"
PR_ELONG             "The address is too long"
PS_RULE              "rewrite: ruleset %2d   input:"
$       Do not translate"ruleset", "rewrite".  
PS_EBOUND            "sendmail: 0832-120 Ruleset %d: the replacement is out of bounds."
$       Do not translate "ruleset".  
PS_EEXPAND           "sendmail: 0832-121 The rewrite expansion is too long."
$       Do not translate "rewrite".  
PS_ECANNON           "sendmail: 0832-122 Cannot prescan canonical hostname %s"
$       Do not translate "hostname".  
$       Canonical is the real name of the alias.
PS_RETURN            "rewrite: ruleset %2d returns:"
$       Do not translate "rewrite", "ruleset".  
$       PS_RULE and PS_RETURN should be formatted so that the colons are aligned.
$       Pad with spaces after the %2d if necessary.
PS_ENONET            "sendmail: 0832-123 There is no network specified."
PS_EUSER             "sendmail: 0832-124 There is no user specified."
PS_EMAILER           "sendmail: 0832-125 %s is not a known mailer."
PS_EHOST             "sendmail: 0832-126 There is no host specified."
PS_EUSER2            "sendmail: 0832-127 There is no user specified."
QU_ETEMP             "sendmail: 0832-128 queueup: Cannot create temporary file %s"
$       Do not translate "queueup".  
QU_TLOCK	     "sendmail:	queueup: Cannot flock(%s)."
$	Do not translate "queueup" and "flock".
QU_CREAT             "sendmail: 0832-129 queueup: Cannot create file %s"
$       Do not translate "queueup".  
QU_QUARPA            "queued"
QU_ERENAME           "sendmail: 0832-130 Cannot rename %1$s to %2$s. The data file is %3$s"
QU_SKIP              "sendmail: 0832-131 Skipping the queue run. The load average is too high.\n"
QU_RUNQ              "Running queue: %s"
$       Please keep as a short phrase.
QU_EALIAS            "sendmail: 0832-132 Cannot open the %s alias database"
QU_WORK              "sendmail: 0832-133 Internal error. Contact System Administrator."
QU_EOPEN             "sendmail: 0832-134 orderq: Cannot open queue directory %s "
$       Do not translate orderq.
QU_CLEAN             "000 Queue Directory %s will be cleaned."
$       NOTHING can precede the three-digit code at the beginning of the message.
QU_EEND              "sendmail: 0832-135 Ended before completing the reading of %s"
$ Do not translate\t"orderq".  
QU_EOPEN2            "sendmail: 0832-136 orderq: Cannot open  file %s"
$       Do not translate "orderq".  
QU_ESKIP             "\nSkipping queue job %s.\n"
QU_ECONTROL          "sendmail: 0832-137 readqf: Cannot open control file %s"
$       Do not translate "readqf" in this and the following messages.  
QU_RUN               "\nRunning queue job %s.\n"
QU_EOPEN3            "sendmail: 0832-138 readqf: Cannot open data file %s"
$       Do not translate "readqf".  
QU_ELINE             "sendmail: 0832-139 readqf: Line number %2$d in queue job %1$s is not valid: %3$s\n"
QU_EMPTY             "The mail queue is empty.\n"
QU_REQ               "\tThere is 1 request in the mail queue"
QU_REQS              "\tThere are %d requests in the mail queue"
QU_PRINT             ", only %d printed"
QU_HDR               "\n---QID---- --Size-- -Priority- ---Q-Time--- ----------Sender/Recipient----------\n"
$       The above, and the following, are table headings. If translated, use 
$       the same number of characters or less and if applicable, keep column 
$       headings aligned.  
$       Do not translate QID.  
QU_HDR2              "\n---QID---- --Size-- -----Q-Time----- --------------Sender/Recipient-------------\n"
QU_ECONTR            " (no control file"
$       Please leave a space before the (.
QU_ECREAT            "sendmail: 0832-140 queuename: Cannot create %1$s in %2$s"
$       Do not translate "queuename".  
CF_ELONG             "sendmail: 0832-141 Configuration file path name %s is too long."
CF_ELONG2            "sendmail: 0832-142 Configuration file name %s is too long."
CF_ELOCK             "sendmail: 0832-143 Cannot open the configuration lock file %s"
DF_ELOCK2            "sendmail: 0832-144 Cannot lock the configuration lock file %s"
CF_EFROZEN           "sendmail: 0832-145 There is no frozen configuration file %s"
DF_EFROZE2           "sendmail: 0832-146 To freeze %s, run the command /usr/lib/sendmail -bz "
$       Do not translate /usr/lib/sendmail -bz.
$       Freeze is a technical term for compile.
CF_EFROZE3           "Cannot open frozen configuration file %s"
CF_EREAD             "sendmail: 0832-147 Cannot read from %s"
CF_EFROZ4            "sendmail: 0832-148 %s is not a valid freeze file."
CF_EREWRITE          "sendmail: 0832-149 Rewrite line is not valid: %s"
$       Do not translate rewrite.
CF_ERULE             "sendmail: 0832-150 %1$d is not a valid ruleset number. The maximum is %2$d."
$       Do not translate "ruleset".  
CF_EPLINE            "sendmail: 0832-151 Do not specify more than %d P lines."
CF_EPREC             "sendmail: 0832-152 The precedence line is not valid"
CF_ETLI              "sendmail: 0832-153 Do not specify more than %d T lines"
CF_ECONTR            "sendmail: 0832-154 Control line is not known: %s"
CF_EOPEN             "sendmail: 0832-155 Cannot open class file %s"
CF_EQU               "sendmail: 0832-156 The = character is expected"
CF_EREW              "sendmail: 0832-157 The maximum number of the rewrite rules is %d"
CF_EMAILER           "sendmail: 0832-158 Do not define more than %d mailers"
CF_INV               "sendmail: 0832-159 The %c option requires the user to be root or the group to be system."
CF_ABS               "sendmail: 0832-160 Specify an absolute path for the A option"
CF_EMODE             "sendmail: 0832-161 %c is not a valid delivery mode"
CF_EMODE2            "sendmail: 0832-162 Specify the delivery mode"
CF_EMODE3            "sendmail: 0832-163 %c is not a valid error mode"
CF_EMODE4            "sendmail: 0832-164 Specify the error mode"
CF_EHOPT             "sendmail: 0832-165 Specify an absolute path for the H option"
CF_EQOPT             "sendmail: 0832-166 Specify an absolute path for the Q option"
CF_ESOPT             "sendmail: 0832-167 Specify an absolute path for the S option"
CF_ECF               "sendmail: 0832-168 %c is not a valid configuration option"
CF_EPATH             "sendmail: 0832-169 There is no configuration file name. The database is not initialized."
CF_ELONG3            "sendmail: 0832-170 Configuration file path name %s is too long."
CF_ELONG4            "sendmail: 0832-171 Configuration file name %s is too long."
CF_ETEMP             "sendmail: 0832-172 Cannot create temporary configuration database %s"
CF_EOPEN2            "sendmail: 0832-173 Cannot open configuration file %s"
CF_EWRITE            "sendmail: 0832-174 Cannot write to %s"
CF_EREAD2            "sendmail: 0832-175 Cannot read from %s"
CF_ESEEK             "sendmail: 0832-176 Cannot seek in %s"
$       Do not translate seek.
RE_EALIAS            " The aliasing or forwarding loop is broken."
$       Please leave a space before the first word in the message.
RE_EPROG             "sendmail: 0832-177 Cannot mail directly to programs."
RE_EINCL             "sendmail: 0832-178 Cannot mail directly to :include:files."
$       Do not translate include.
DF_MAILF             "sendmail: 0832-179 Cannot mail directly to files."
RE_EFSTAT            "sendmail: 0832-180 Cannot fstat %s"
$       Do not translate "fstat".  
RE_OFLOW             "sendmail: 0832-181 There is an address overflow."
SV_JUNK              "Junk mail will not be saved."
$       Junk mail is a class of mail.
SV_ROOT              "sendmail: 0832-182 Cannot get address for root user."
SV_ERRORM            "sendmail: 0832-183 The error mode ox%x is not valid."
SV_FROM              "\r\nMessage from %s...\r\n"
SV_ERRORS            "Errors occurred while sending mail.\r\n"
SV_TRANS             "The transcript of the session is not available.\r\n"
SV_TRANS2            "The transcript follows:\r\n"
SV_DEAD              "sendmail: 0832-184 The original message will be saved in dead.letter\r\n"
$       Do not translate dead.letter.
SV_POWTM             "sendmail: 0832-185 Cannot get address for postmaster alias."
$       Do not translate postmaster.
SV_UDEL              "Cannot deliver mail"
SV_ERETURN           "sendmail: 0832-186 Cannot return mail to %s"
SV_SV                "Saving message in %s"
SV_ESTATE            "sendmail: 0832-187 %d is not a known state."
SV_HELP              "sendmail: 0832-188 Cannot save mail"
SV_RECUR             "sendmail: 0832-189 There is an infinite loop trying to return mail to %s"
SV_EPARSE            "sendmail: 0832-190 Cannot get address for the sender."
SV_EBODY             "sendmail: 0832-191 There is an input or ouput error."
SP_LOST              "%1$s lost connection from %2$s"
SP_ESELF             "There is a local configuration error.  The hostname is not recognized as local."
$       Do not translate hostname.  
SP_EFORK             "sendmail: 0832-192 %s: Cannot create a process"
SP_ECHILD            "sendmail: 0832-193 %s: Lost the child process."
ST_ESTAT             "sendmail: 0832-194 The mailer number is not valid"
ST_EOPEN             "sendmail: 0832-195 Cannot open statistics file."
ST_ELOCK             "sendmail: 0832-196 Cannot lock statistics file."
ST_EREAD             "sendmail: 0832-197 Cannot read statistics file."
ST_EFORMAT           "sendmail: 0832-198 The statistics file format is not correct."
ST_ETRUNC            "Use the mailstats -z command to truncate the file"
$       Do not translate mailstats.  
ST_EWRITE            "sendmail: 0832-199 Cannot write to statistics file"
ST_MS                "000 %1$s. The file is %2$s"
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       Do not translate sendmail.  
ST_SM2               "000 %s."
$       NOTHING can precede the three-digit code at the beginning of the message.
$       Do not translate. 
ST_SM3               "%1$s. The file is %2$s\r\n"
ST_SM4               "sendmail: %s.\r\n"
$       Do not translate.
ST_LOG               "%1$s. The file is %2$s"
SY_USTAT             "554 Unknown status %d"
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       DO NOT TRANSLATE the message above. 
SL_ELONG             "sendmail: 0832-200 The log file path name is too long."
SL_EACCESS           "sendmail: 0832-201 Cannot access log file %s"
SL_EWRITE            "sendmail: 0832-202 There is a write or close on log file %s"
US_INVALID	     "usersmtp: The hostname, %s, is not a valid local hostname." 
US_DATA		     "smtpdata: There is a write error"
US_REPL              "reply: There is a read error"
$       Do not translate reply.  
UT_MEM               "sendmail: 0832-203 There is not enough memory available now"
UT_ULINK             "%1$s: unlink %2$s"
$       Do not translate unlink.  
UT_ULINK2            "Cannot unlink file %1$s: error number %2$d"
$       Do not translate unlink.
UT_TIMEOUT	     "There is a timeout while writing to %s"
UT_TIMEOUT2	     "451 There is a timeout while writing output"
UT_TIME              "There is a timeout while waiting for input from %s"
UT_TIME2             "451 There is a timeout while waiting for input."
$	Nothing can precede the three-digit code at the beginning of the message.
$	Do not translate the message above. 
$       The first 4 characters in each of the following\t"HELPMSGxx" lines   
$       must not be changed.   
HELPMSG              "cpyr\t"
$       Do not translate cpyr.  
HELPMSG1             "cpyr\tSendmail"
$       Do not translate.  
HELPMSG2             "cpyr\tCopyright (c) 1983  Eric P. Allman"
$       Do not translate.
HELPMSG3             "cpyr\tBerkeley, California"
$       Do not translate.
HELPMSG4             "cpyr\t"
$       Do not translate.
HELPMSG5             "cpyr\tCopyright (c) 1983 Regents of the University of California."
$       Do not translate.
HELPMSG6             "cpyr\tAll rights reserved.  The Berkeley software License Agreement"
$       Do not translate.
HELPMSG7             "cpyr\tspecifies the terms and conditions for redistribution."
$       Do not translate.
HELPMSG8             "cpyr\t"
$       Do not translate.
HELPMSG9             "cpyr\t@(#)sendmail.hf\t4.2 (Berkeley) 6/7/85"
$       Do not translate.
HELPMSG10            "cpyr\t"
$       Do not translate.
HELPMSG11            "cpyr\tCopyright (c) 1987 IBM Corporation.  All rights reserved."
$       Do not translate.
HELPMSG12            "cpyr"
$       Do not translate.
HELPMSG13            "smtp\tCommands:"
$       The first 4 characters in this and each of the following "HELPMSGxx" lines
$       must not be changed.
HELPMSG14            "smtp\t\tHELO\tMAIL\tRCPT\tDATA\tRSET"
$       Do not translate.
HELPMSG15            "smtp\t\tNOOP\tQUIT\tHELP\tVRFY\tEXPN"
$       Do not translate.
HELPMSG16            "smtp\tFor more information use HELP followed by the topic."
HELPMSG17            "sendmail: 0832-205 smtp\tTo report problem in the implementation, contact your service repesentative."
HELPMSG18            "smtp\tFor local information contact postmaster at this site."
HELPMSG19            "help\tUsage: HELP [Topic]"
$       Do not translate HELP, Usage.
HELPMSG20            "help\t\tThe HELP command gives help info."
$       Do not translate HELP.
HELPMSG21            "helo\tUsage: HELO Hostname"
$       Do not translate HELO.
HELPMSG22            "helo\t\tIdentifies the sending host."
HELPMSG23            "mail\tMAIL FROM: Sender"
$       Do not translate MAIL FROM.
HELPMSG24            "mail\t\tSpecifis the sender."
HELPMSG25            "rcpt\tUsage: RCPT TO: recipient"
$       Do not translate RCPT TO.
HELPMSG26            "rcpt\t\tSpecifies the recipient.  Can be used any number of times."
HELPMSG27            "data\tUsage: DATA"
$       Do not translate DATA.
HELPMSG28            "data\t\tThe following text is collected as the message."
HELPMSG29            "data\t\tEnd with a line containing only the . character.\n"
HELPMSG30            "rset\tUsage: RSET"
$       Do not translate RSET.  
HELPMSG31            "rset\t\tResets the protocol state."
HELPMSG32            "quit\tUsage: QUIT"
HELPMSG33            "quit\t\tExits sendmail (SMTP)."
$       Do not translate SMTP.  
$       Do not translate sendmail.  
HELPMSG34            "vrfy\tUsage:VRFY Recipient"
$       Do not translate VRFY.  
HELPMSG35            "vrfy\t\tAllows verification of recipient. Not part of protocol."
HELPMSG36            "expn\tUsage: EXPN Recipient"
$       Do not translate EXPN.  
HELPMSG37            "expn\t\tSame as VRFY in this implementation."
$       Do not translate VRFY.  
HELPMSG38            "noop\tUsage: NOOP"
$       Do not translate NOOP.  
HELPMSG39            "noop\t\tDoes nothing."
HELPMSG40            "send\tUsage: SEND FROM: Sender"
$       Do not translate SEND FROM.
HELPMSG41            "send\t\tSimilar to the mail command, but sends the message"
$       Messages 41-43 comprise one message. The text should be translated so
$       that it spans the three messages.  "send\t" should remain as is.
HELPMSG42            "send\t\tdirectly to a user's display.  Not supported in this"
HELPMSG43            "send\t\timplementation."
HELPMSG44            "soml\tUsage: SOML FROM: Sender"
$       Do not translate SOML FROM.  
HELPMSG45            "soml\t\tSend or mail.  If the user is logged in, send directly,"
$       Messages 45-46 comprise one message. Please see note for messages 41-43.
HELPMSG46            "soml\t\totherwise mail.  Not supported in this implementation.\n"
HELPMSG47            "saml\tUsage: SAML FROM: Sender\n"
$       Do not translate SAML FROM.  
HELPMSG48            "saml\t\tSend and mail.  Send directly to the user's work station,"
$       Messages 48-50 comprise one message. Please see previous notes.      
HELPMSG49            "saml\t\tand also mail the message.  Not supported in this"
HELPMSG50            "saml\t\timplementation."
HELPMSG51            "turn\tUsage: TURN"
$       Do not translate TURN.
HELPMSG52            "turn\t\tReverses the direction of the connection. "
$       Messages 52-53 comprise one message.
HELPMSG53            "turn\t\tNot currently implemented."
HELPMSG54            " "
AL_ALTO              "aliased to %s"
$       An alias is a short way of writing a longer name or address.  
CO_ENTER             "Enter mail. End with the . character on a line by itself."
DL_ESIGCLD           "sendoff: sigvec: SIGCLD"
$       Do not translate .
DL_OSIGINT           "openmailer: sigvec: SIGINT"
$       Do not translate.
DL_OSIGHUP           "openmailer: sigvec: SIGHUP"
$       Do not translate.
DL_OSIGTERM          "openmailer: sigvec: SIGTERM"
$       Do not translate SIGTERM.  
DL_CONN              "Connecting to %1$s.%2$s..."
DL_CONN2             "Connecting to %1$s (%2$s)..."
DL_ESEND             "Cannot send message for %s"
DL_SENT              "250 Sent"
$      NOTHING can precede the three-digit code at the beginning of the message. 
$      DO NOT TRANSLATE the message above. 
DL_EMAILER           "554 Unknown mailer error %d"
$       NOTHING can precede the three-digit code at the beginning of the message. 
$       DO NOT TRANSLATE the message above. 
PS_EUBAL             "sendmail: 0832-206 There is a missing ( character."
PS_EUBAL2            "sendmail: 0832-207 There is a missing < character."
PS_ETOKEN            "sendmail: 0832-208 There are too many tokens."
PS_EUBAL3            "sendmail: 0832-209 There is a missing ) character."
PS_EUBAL4            "sendmail: 0832-210 There is missing > character."
PS_EUBAL5            "sendmail: 0832-211 There is a missing \" character."
RE_DUP               "Suppressing duplicate"
RE_INCLF             "Including file %s"
$       The command is in the process of including a file.
RE_LOGNAME           "Sending to login name %s"
RE_TO                "%1$s to %2$s"
SP_PLEASED           "%1$s Hello %2$s"
$       Do not translate.
SP_ESENDER           "The sender is already specified."
SP_ENEST             "Nested MAIL command."
$       Do not translate MAIL.  
SV_OK                "Sender is valid."
SP_RECIP             "Recipient is valid."
SP_EADDR             "The addressee is not known."
SP_MAIL              "Use the MAIL command."
$       Do not translate MAIL.  
SP_RCPT              "Specify the RCPT (recipient)."
SP_OK                "Ok"
$       This may be translated.
SP_RESET             "Resetting the state."
SP_DESTR             "There is an alias loop."
SP_OK2               "ok"
$ Do not translate OK.  
SP_CLOSE             "%s: closing the connection."
SP_VERB              "The system is in verbose mode."
SP_EONE              "There is only one transaction allowed."
SP_ECMD              "The command is not known."
SP_CODE              "sendmail: 0832-212 smtp: %d is not a known code."
$       Do not translate smtp.  
SP_SYNTAX            "There is a syntax error."
SP_EHELP             "The specified HELP topic is not known."
$       Do not translate HELP.
SP_END               "End of HELP information."
$	The following message is an old message and is no longer in use. The 
$	new message is at the end of this file with the same name.
$	Commented the old messages since 2 with same name won't compile. 
$ SV_TRANSU            "  --- A transcript of the session is not available.\n"
$	The following 5 messages and their comments are old and no longer in
$	use. The 5 new messages and new comments are at the end of this file
$	with the same name. 
$	Commented the old messages since 2 with same name won't compile. 
$	(end of new comments)
$       The following messages can be inserted in a mail item.  They may  
$       be translated. Sendmail will\t"flatten" the strings before they  
$       are inserted.  
$ SV_TRANSHDR          "  --- The transcript of the session follows ---\n"
$ SV_SUPR              "\n  --- The return message is suppressed ---\n\n"
$ SV_UNSENT            "  --- The unsent message follows ---\n"
$ SV_MSG               "  --- The message header follows ---\n"
$ SV_NOMSG             " --- No message was collected ---\n"
NOMAIL               "sendmail: 0832-213 No ARPA mail through this machine: see system administrator."
$       Do not translate ARPA.  
DL_WAIT              "sendmail: 0832-214 Cannot wait for %s."
$       Do not translate wait.
DL_MDIED             "sendmail: 0832-215 Mailer %1$s ended with signal %2$o"
MN_SET               "User must be root and group must be system."
MN_SET2              "User must be root and group must be system."
MN_INVDB             "sendmail: 0832-216 %sDB is not a valid NLS database."
$       Do not translate NLS.  
MN_TESTMODE          "NLS and ISO-8859 Address Test Mode\nEnter Address\n"
$       Do not translate NLS, ISO-8859.  
MN_R7OUTPUT          "The output of ruleset 7 is %s\n"
$       Do not translate ruleset.  
MN_BODYCODE          "Body encoding is NLS escape.\n"
$       Do not translate NLS.  
$       Body refers to the message text.
MN_BODYCODE2         "Body encoding is ISO-8858/1.\n"
$       Do not translate ISO.  
MN_BODYCODE3         "Body encoding is flat ASCII.\n"
$       Do not translate ASCII.  
NL_LONGPATH          "sendmail: 0832-217 Sendmail.nl path name %s is too long."
$       Do not translate Sendmail.nl in this and the following messages.  
NL_LONGNAME          "sendmail: 0832-218 Sendmail.nl file name %s is too long."
NL_ELOCKFILE         "sendmail: 0832-219 Cannot open sendmail.nl lock file %s"
NL_ELLOCKFILE        "sendmail: 0832-220 Cannot lock sendmail.nl lock file %s"
NL_NOFRZFILE         "sendmail: 0832-221 There is no frozen sendmail.nl file %s"
$      Frozen is a technical term meaning compiled.
NL_TOFRZ             "To freeze %s run the command /usr/lib/sendmail -bn"
$       Do not translate /usr/lib/sendmail -bn.
NL_EOFRZFILE         "Cannot open frozen sendmail.nl file %s"
NL_EFORMAT           "sendmail: 0832-222 There is a format error in %s; cannot find magic string.\n"
$       Do not translate magic.
NL_EFORMAT2          "sendmail: 0832-223 There is a format error or there are too many lists in %s"
NL_EFORMAT3          "sendmail: 0832-224 There is a format error. %s at offset = %d."
NL_NOPATH            "sendmail: 0832-225 There is no sendmail.nl path name. The database is not initialized."
NL_LONGPATH2         "sendmail: 0832-226 Sendmail.nl path name %s is too long."
NL_ECREATE           "sendmail: 0832-227 Cannot create temporary sendmail.nl database %s"
NL_EOPEN             "sendmail: 0832-228 Cannot open sendmail.nl file %s"
$ Do not translate sendmail.nl.  
NL_EFORMAT4          "sendmail: 0832-229 There is a format error in %s"
NL_ENAME             "sendmail: 0832-230 There is a wrong body type name in %s"
NL_EEXP              "sendmail: 0832-231 The regular expression is not correct: %s"
NL_EFORMAT5          "sendmail: 0832-232 There is a format error or there are too many lists."
NL_EXPERRS           "sendmail: 0832-233 There are %d regular expression errors."
NL_NOLISTS           "There are no lists in %s"
NL_EOPENNT           "sendmail: 0832-234 Cannot open %s"
NL_EREADNB           "sendmail: 0832-235 Cannot read file"
NL_EWRITENB          "sendmail: 0832-236 Cannot write to file"
QU_DOWORK            "%1$s: processing queue job, process identifier is %2$d."
QU_LOCKED            "%s: Queue job is locked."
CF_KOPT              "sendmail: 0832-237 Specify an absolute path for the l flag."
SL_ERRNO             "error number %d"
RE_SENDING           " sending."
$       Please leave a space at the beginning of the message.
CF_BADRR	"sendmail: 0832-238 The %s nameserver resource record is not supported"
$			"nameserver" is a technical term.
$			"resource record" is a technical term; it is used by the
$			nameserver program.
CF_NORR		"sendmail: 0832-239 A nameserver resource record must be specified"
$			"nameserver" is a technical term.
$			"resource record" is a technical term; it is used by the
$			nameserver program.
RE_RRMSG	"resolved via %1$s resource record to %2$s"
$			This is an informational message, indicating that a
$			username has been changed, or "resolved", using the
$			nameserver.
$			"resource record" is a technical term; it is used by the
$			nameserver program.
$			The %1$s is replaced by a mnemonic abbreviation, like
$			"MX" or "CNAME".
$			The %2$s is replaced by a username.
RE_NOAT		"sendmail: 0832-241 There is no '@' in the SMTP recipient %s"
$			This is an error message: the '@' character is required
$			for this type of recipient.
$			Do not translate '@'
$			Do not translate SMTP
$			The %s is replaced by a username.
MN_CODESET	"sendmail: Both the Obnetcode and OOmailcode options must be set."
$	Do not translate the words ONnetcode and OMmailcode and sendmail.
$	This is an info message sent through syslogd.
NL_ICONV	"sendmail: 0832-242 An iconv() error occurred."
$	Do not translate the word iconv().
NL_CONVD	"sendmail: 0832-243 iconv_open() cannot translate the codesets."
$	Do not translate the word iconv_open().
SV_TRANSU            "  --- A transcript of the session is not available.\n"
SV_TRANSHDR          "  --- The transcript of the session follows ---\n"
SV_SUPR              "\n  --- The return message is suppressed ---\n\n"
SV_UNSENT            "  --- The unsent message follows ---\n"
SV_MSG               "  --- The message header follows ---\n"
SV_NOMSG             " --- No message was collected ---\n"
$       The 6 messages above can be inserted in a mail item.  They should not  
$       be translated. They may be translated for Japanese environments.
$	These 6 messages are replacement messages, previously they were listed
$	as being translatable since they would be eventually flattened, but
$	that is no longer true.  The old messages are referenced by the same
$	name.
CF_IGN		"Configuration option '%c' is not supported (ignored)"
$			This is an informational message (warning).
$			The %c is replaced by the option letter in question.
