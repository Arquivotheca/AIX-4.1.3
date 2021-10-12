#! /bin/ksh
# @(#)86	1.7  src/bldenv/bldtools/bldlogset.sh, bldprocess, bos412, GOLDA411a 11/4/92 19:04:20
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldlogset
#	     validlevel
#	     reset_file
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
# NAME: reset_file
#
# FUNCTION: Removes the last line in the database file i.e. resetting the 
#	    database file to it's previous defaults.
#
# INPUT: none
#
# OUTPUT: none
#
# SIDE EFFECTS: Creates a temporary database file in $BLDTMP and then copies 
# 		over the original database file in the current directory.
#
# NOTES: Exits with return code 2 if the database file is not found, otherwise
#	 exits with 0.

function reset_file
{
	if [[ -f "$db" ]]
	then
		sed '$d' $db > $BLDTMP/log.db$$
		cp $BLDTMP/log.db$$ $db
		rm $BLDTMP/log.db$$
		if [[ ! -s "$db" ]]    # now if file size is empty - remove
		then
			rm $db
		fi
		exit 0
	else
		print -u3 "$db file not found" 
		exit 2;
	fi 
}

# NAME: bldlogset
#
# FUNCTION: creates,appends or resets a database file 'LOG.db' in the $BLDTMP
#	    directory that contains default values for the various parameters 
#	    to be used in 'bldlog' function.
#
# INPUT: Options and optional arguments. See man pages for details.
#
# OUTPUT: All output is written to a standard database file 'LOG.db' in the
#	  $BLDTMP directory.
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# FORMATS/DATA STRUCTURES:
# database file format (output)
# level context logfile subcontext1 subcontext2 command message
#
# RETURNS: Always 0;
#

test ${BLDTMP:="/tmp"}
typeset -ix DEFLEVEL=1
typeset -x  DEFCONTEXT="Undefined" DEFSUBCONTEXT1="Undefined" \
DEFSUBCONTEXT2="Undefined" DEFCOMMAND="log" DEFMESSAGE="Undefined" \
DEFTYPE="b" MSGLOG="$BLDTMP/LOG"       # default values for the various options.


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
				# database filename in the $BLDTMP directory.
if [ ! -f "$db" ]
then			# touches the db file 

	touch $db
	$(chmod 666 $db)

else			# reads from the db file the previously saved defaults.

	sed -n '$p' $db | read DEFLEVEL DEFCONTEXT MSGLOG\
	DEFSUBCONTEXT1 DEFSUBCONTEXT2 DEFCOMMAND DEFMESSAGE DEFTYPE
fi

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

while getopts :rlL:C:F:1:2:c: option           # parse command line options
do 					       # which override defaults.
	case $option in
	l) checklevel $levelset;let DEFLEVEL=DEFLEVEL-1;levelset=1;\
	   validlevel $DEFLEVEL;DEFLEVEL=$?;;
       +l) checklevel $levelset;let DEFLEVEL=DEFLEVEL+1;levelset=1;\
	   validlevel $DEFLEVEL;DEFLEVEL=$?;;
	L) checklevel $levelset;DEFLEVEL=$OPTARG;levelset=1;\
	   validlevel $DEFLEVEL;DEFLEVEL=$?;;
	C) DEFCONTEXT=${OPTARG:-$DEFCONTEXT};;
	F) MSGLOG=${OPTARG:-$MSGLOG};;
	1) DEFSUBCONTEXT1=${OPTARG:-$DEFSUBCONTEXT1};;
	2) DEFSUBCONTEXT2=${OPTARG:-$DEFSUBCONTEXT2};;
	c) DEFCOMMAND=${OPTARG:-$DEFCOMMAND};;
	r) let reset_flag=1;;
	:) print -u3 "$0: $OPTARG requires a value:" 
	   exit 2;;
       \?) print  -u3 "$0: Unknown option $OPTARG" 
	   print -u3 "USAGE: $0 [-L <defabslevel> | -l ] [-C <context>]\
 [-F <logfile>] [-1 <subcontext1>] [-2 <subcontext2>]\
[-c <command>] [r]" 
	   exit 2;;
	esac
done
print $MSGLOG
shift OPTIND-1
if [[ -n "$*" ]]
then
	DEFMESSAGE=$*    # default message if any from the last argument.
fi
if [[ -n "$reset_flag" ]]
then
	reset_file;     # pop out the last line from the db file.
fi

# Now append all the option values to the db file

print -n "$DEFLEVEL $DEFCONTEXT $MSGLOG " >> $db
print -n "$DEFSUBCONTEXT1 $DEFSUBCONTEXT2 $DEFCOMMAND " >> $db
print "$DEFMESSAGE $DEFTYPE" >> $db
return 0


