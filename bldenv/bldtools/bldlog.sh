#! /bin/ksh
# @(#)89	1.8  src/bldenv/bldtools/bldlog.sh, bldprocess, bos412, GOLDA411a 11/15/93 14:29:18
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: validlevel
#	     checkopt
#	     checklevel
#            log_message
#	     bldlog
#	     
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: validlevel
#
# FUNCTION:  Checks whether the value of 'level' is less than 1.
#
# INPUT: level (parameter) - gets the value of level.
#
# OUTPUT: none
#
# RETURNS: 1 (if value of level is less than 1) or level.
#

function validlevel
{
	typeset -i level=$1

	if (( $level <= 0 ))
	then
		return 1
	else 
		return $level
	fi
}

#
# NAME: checkopt
#
# FUNCTION:  Checks whether more than one message type is being set.
#
# INPUT: flag (parameter) - gets the value of the stdinput flag.
#
# OUTPUT: none
#
# RETURNS: none.
#
# NOTES: stdinput flag is set when a message type is specified.
#        Exits from the function if the standard input has been set once.

function checkopt 
{
	typeset stdinput=$1

	if [[ -n "$stdinput" ]]
	then
		print -u3 "log: illegal syntax; only one message type allowed"
		exit 2
	fi
}

#
# NAME: checklevel
#
# FUNCTION:  Checks whether both the absolute level and default level incement
#	     are being done.
#
# INPUT: flag (parameter) - gets the value of the levelset flag.
#
# OUTPUT: none
#
# RETURNS: none.
#
# NOTES: levelset flag is set when the default level is incremented or when 
#	 an absolute level is specified.
#        Exits from the function if the levelset flag has been set once.

function checklevel 
{
	typeset levelset=$1

	if [[ -n "$levelset" ]]
	then
		print -u3 "log: illegal syntax, level can be set once only"
		exit 2
	fi
}

#
# NAME: log_message
#
# FUNCTION:  Prints out the logs with the other parameters to the specified
#	     log file.
#
# INPUT: msglevel (parameter) - level of the log.
#	 msgtype (parameter) - type of the message.
#	 msg (parameter) - message or status to be logged.
#	 msglog (parameter) - The log file to which al the logs are appended.
#	 msgcontext (parameter) - The context of the log.
#	 msgcommand (parameter) - command if any(could be undefined)
#	 			  at which logging is done.
#	 msgsubcontext1 (parameter) - first level subcontext of the log.
#	 msgsubcontext2 (parameter) - second level subcontext of the log.
#
# OUTPUT: The logs are written to the specified file from the input parameter,
#         Default being 'LOG' in the $BLDTMP directory.
#	  Depending on the message type,
#	  the messages are printed on the terminal also but not on stdout.
#
# RETURNS: none.
#

function log_message
{
	typeset msglevel=$1
	typeset msgtype=$2
	typeset msg=$3
	typeset msglog=$4
	typeset msgcontext=$5
	typeset msgcommand=$6
	typeset msgsubcontext1=$7
	typeset msgsubcontext2=$8
	typeset dt indent
	typeset -i index=1
	dt=$(date +"%h %d %H:%M:%S")
	msgcommand=$(basename $msgcommand)
	if [[ "$msgcommand" = NULL ]]
	then
		msgcommand=""
	else
		msgcommand=$msgcommand": "
	fi	
	line=$FS$msgcommand$FS$msg$FS$msglevel$FS$msgcontext$FS
	line=$line$msgsubcontext1$FS$msgsubcontext2$FS$dt$FS

	if [[ "$msglog" = "$BLDTMP/LOG" ]]
	then
		if [[ ! -s $msglog ]]
		then
			> $msglog
			$(chmod 666 $msglog)
		fi
	fi
	while (( index < $msglevel ))
	do
		indent="${indent}  "
		let index=index+1
	done

	case $msgtype in
	(x)
		print "${FS}FATAL ERROR: $line" >> $msglog		
		print -u3 "${indent}${msgcommand}FATAL ERROR: $msg" 
		return
		;;
	(e)		
		print "${FS}ERROR: $line" >> $msglog		
		print -u3 "${indent}${msgcommand}ERROR: $msg" 
		return
		;;
	(w)	
		print "${FS}WARNING: $line" >> $msglog		
		print -u3 "${indent}${msgcommand}WARNING: $msg" 
		;;
	(a)
		print "\a\a\a\a\a\c"
		print "${FS}ALERT: $line" >> $msglog		
		print -u3 "${indent}${msgcommand}ALERT: $msg" 
		;;
	(f)
		print "$FS$line" >> $msglog		
		;;
	(b)		
		print "$FS$line" >> $msglog		
		print -u3 "${indent}${msgcommand}$msg"  
		;;
	esac
	return
}

# NAME: bldlog
# 
# FUNCTION: Maintains a log file which can be helpfull in debugging code and 
#           keeping logs of other functions and commands. 
#
# INPUT: Options and optional arguments. See man pages for more details.
#        Any stdin input can also be logged. All command line options override
#	 any defaults that are read from a database file 'LOG.db' in the 
#	 $BLDTMP directory.
#
# OUTPUT: The logs are written to the specified file from the option to the
#         input, Default being 'LOG' in the $BLDTMP directory.
#
# SIDE EFFECTS: A database file called 'LOG.db' is created in the $BLDTMP
#                directory if it is not found and default values for the input
#		 options are written in it.
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# FORMATS/DATA STRUCTURES:
# log file format (output)
# %type%command%msg%level%context%subcontext1%subcontext2%date%
#
# RETURNS:
# 	Error codes: 2 - fatal error
#		     1 - error
#                    0 - warning, alert and files.
#

test ${BLDTMP:="/tmp"}
typeset -ix DEFLEVEL=1  # Defaults for level
typeset -x  DEFCONTEXT="Undefined" MSGLOG="$BLDTMP/LOG" \
	    DEFSUBCONTEXT1="Undefined" DEFSUBCONTEXT2="Undefined" \
	    DEFCOMMAND="log" DEFMESSAGE="Undefined"\
	    DEFTYPE="b" FS="%"    # Other defaults, FS - field seperator

typeset -i  msglevel deflevel
typeset     stdinput="" msgtype="" message="" msgcontext="" levelset=""\
	    msgsubcontext1="" msgsubcontext2="" msgcommand="" msglog=""

# Check to see if /dev/tty can really be written to, BLDTTY will be 
# '/dev/*' if writable.  If not writable BLDTTY will be some form of
# error message.
if [[ -n "${BLDTTY}" && -z "${BLDTTY%%/dev/*}" ]]
then
   exec 3> /dev/tty  # file descriptor 3 redirected to terminal for
                     # error messages
else
   exec 3> /dev/null # file descriptor 3 ignored if /dev/tty is not writable
fi

if [[ -n "${BLDLOG_DATABASE}" ]]
then
   typeset -x db="${BLDLOG_DATABASE}"
else
   typeset hostname=$(hostname)
   typeset ttyname=$BLDTTY
   typeset tty=${ttyname#/dev/}
   typeset term=${tty%%/*}
   typeset termnum=${tty##*/}
   typeset -x db="$BLDTMP/.LOG.${hostname}.${term}.${termnum}.db"
fi
				# database filename in the current directory.

if [ -f "$db" ]		# test whether the database file exhists.
then		# read the defaults from the file.
	sed -n '$p' $db | read DEFLEVEL DEFCONTEXT MSGLOG\
	DEFSUBCONTEXT1 DEFSUBCONTEXT2 DEFCOMMAND DEFMESSAGE DEFTYPE
fi

msglog=${MSGLOG}
deflevel=$DEFLEVEL
msglevel=$DEFLEVEL

while getopts :xewafblL:C:F:1:2:c: option   # parse the command line parameters
do	case $option in
	 x) checkopt $stdinput; msgtype=x; stdinput=0;;
	+x) checkopt $stdinput; msgtype=x; stdinput=1;;
	 e) checkopt $stdinput; msgtype=e; stdinput=0;;
	+e) checkopt $stdinput; msgtype=e; stdinput=1;;
	 w) checkopt $stdinput; msgtype=w; stdinput=0;;
	+w) checkopt $stdinput; msgtype=w; stdinput=1;;
	 a) checkopt $stdinput; msgtype=a; stdinput=0;;
	+a) checkopt $stdinput; msgtype=a; stdinput=1;;
	 f) checkopt $stdinput; msgtype=f; stdinput=0;;
	+f) checkopt $stdinput; msgtype=f; stdinput=1;;
	 b) checkopt $stdinput; msgtype=b; stdinput=0;;
	+b) checkopt $stdinput; msgtype=b; stdinput=1;;
	 l) checklevel $levelset;let deflevel=deflevel-1;levelset=1;\
	    validlevel $deflevel;deflevel=$?;msglevel=$deflevel;;
	+l) checklevel $levelset;let deflevel=deflevel+1;levelset=1;\
	    validlevel $deflevel;deflevel=$?;msglevel=$deflevel;;
	 L) checklevel $levelset;msglevel=$OPTARG;levelset=1;\
	    validlevel $msglevel;msglevel=$?;;
	 C) msgcontext=$OPTARG;;
	 F) msglog=$OPTARG;;
	 1) msgsubcontext1=$OPTARG;;
	 2) msgsubcontext2=$OPTARG;;
	 c) msgcommand=$OPTARG;;
	 :) print -u3 "$0: $OPTARG requires a value"
	    exit 2;;
	\?) print -u3 "$0: unknown option $OPTARG"
	    print -u3 -n "USAGE: $0 [-L <level> | l] [-xewafb] "
	    print -u3 "[-C <context> ] [-F <logfile> ]" 
	    print -u3 -n "[-1 <subcontext1>] [-2 <subcontext2>] "
	    print -u3 "[-c <command>] <string>"
	    exit 2;
	esac
done

shift OPTIND-1

message=$*		# message string.

log_message ${msglevel:-$deflevel} ${msgtype:-$DEFTYPE} \
	    "${message:-$DEFMESSAGE}" $msglog "${msgcontext:-$DEFCONTEXT}"\
	    "${msgcommand:-$DEFCOMMAND}" "${msgsubcontext1:-$DEFSUBCONTEXT1}"\
	    "${msgsubcontext2:-$DEFSUBCONTEXT2}"

if [[ "$stdinput" = 1 ]]      # if the input is also coming from a pipe.
then
	msgcommand="STDIN"
	let msglevel=msglevel+1
	IFS=""
	while read message
	do
	     log_message ${msglevel:-$DEFLEVEL} ${msgtype:-$DEFTYPE}\
  	     		 "$message" $msglog "${msgcontext:-$DEFCONTEXT}" \
	     		 "${msgcommand:-$DEFCOMMAND}" \
			 "${msgsubcontext1:-$DEFSUBCONTEXT1}"\
	     		 "${msgsubcontext2:-$DEFSUBCONTEXT2}"
	done
fi

case $msgtype in 
(x) 
	exit 2;;
(e) 
	exit 1;;
(w|a|f|b) 
	exit 0;;
esac
