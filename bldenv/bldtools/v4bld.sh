#!/bin/ksh
# @(#)06	1.34.1.42  src/bldenv/bldtools/v4bld.sh, bldtools, bos412, GOLDA411a 5/26/94 09:54:18
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: Clean_Up
#            Usage
#            ade_make
#            create_stage_list
#            init
#            getparms
#            setdefaults
#            setphase
#            showparms
#            v4bld
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#

. bldinitfunc
. bldkshconst
. bldkshfunc
. bldloginit
. bldhostsfile
. bldnodenames
. bldtrackingdefect

#
# NAME: Clean_Up
#
# FUNCTION: Clean up system after running
#
# INPUT: none
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always
#
function Clean_Up
{
   #
   #       for the time being, ( until awk file is corrected ) rm core file
   #       caused by a line too long in cmds.log file
   #
   if [ -d "$BUILDBASE/$BLDCYCLE/src" ]
   then
      cd $BUILDBASE/$BLDCYCLE/src
      [[ -a core ]] && rm core
      cd ..
      [[ -a core ]] && rm core
   fi
   [[ "${BLDSTARTED}" = "${TRUE}" ]] && \
      bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} \
			"'$progname' command aborted on `date`."
   logset -r
}

#
# NAME: Usage
#
# FUNCTION: Display Usage message
#
# INPUT: none
#
# OUTPUT: none.
#
# SIDE EFFECTS: Exit from program.
#
# RETURNS: Never returns
#
function Usage
{
   print -u2 "\nUsage: v4bld [-h <hosts_filename>] [-n <node_filename>]"
   Clean_Up
   exit 2
}

#
# NAME: ade_make
#
# FUNCTION: Performs the make in the ADE environment via the scripts
#           supplied in the setup directory.
#
#	Called by: 'Main'
#	reads from :
#		$BLDENV/usr/bin/Err.strings
#
# This is AIX RS/6000 specific.
# A script to automatically build the src tree.  Also ??? Release ??? must 
# be in the PATH for install.sh to operate correctly.  The target is 
# starfleet:/project/aix4/build/daily/aix4.[mmdd], or 
# starfleet:/project/aix4/build/aix4.[mmdd], either of which must exist.  The 
# cmvc server (ausaix02) must have mount and write priviledges to a directory 
# to copy the extraction tree into.
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: performs the specified make.
#
# RETURNS: return codes from the scripts run.
#
ade_make ()
{
  typeset log_list release_list
  typeset stage log release
  typeset logline

  log -b "ade_make: entering"

  echo "Level $BLDCYCLE commenced on `date`" >> $BUILDNOTES
#
#	make sure the *.sh output directories are in place
#
  [[ ! -d $BUILDBASE/$BLDCYCLE/src ]] && mkdir $BUILDBASE/$BLDCYCLE/src
  [[ ! -d $BUILDBASE/$BLDCYCLE/src/logs ]] && mkdir $BUILDBASE/$BLDCYCLE/src/logs

#
#	note the time the level started
#
  echo "\nLevel started on `date`" >> $BUILDNOTES
#
#	go to the src directory to start building
#
  cd $BUILDBASE/$BLDCYCLE/src
#
# build the build tools, note the time the build tools build started
#
  for stage in ${STAGE_LIST}
  do 
	execute_stage $stage
  done

#
#	note the time the build completed
#
  echo "\nLevel completed on `date`" >> $BUILDNOTES
#
#	change the permissions on the ship/power tree so that when someone
#	wants to create a boot tape on a remote machine with this build 
#	mounted, they can.
#
  cd $BUILDBASE/$BLDCYCLE
#
# update rc_files/shared default_build by resetting the values of default_build
# and build_base
#
  if [ -s $BUILDBASE/$BLDCYCLE/rc_files/shared ]
  then
	log -b "ade_make: $BUILDBASE/$BLDCYCLE/rc_files/shared File Exists"
  	cp $BUILDBASE/$BLDCYCLE/rc_files/shared \
  	  $BUILDBASE/$BLDCYCLE/rc_files/SHARED.sv
  	sed 's/^\(default_build \).*/\1'"$BLDCYCLE"'/' \
	  $BUILDBASE/$BLDCYCLE/rc_files/SHARED.sv \
	  > $BUILDBASE/$BLDCYCLE/rc_files/shared.temp
	cp $BUILDBASE/$BLDCYCLE/rc_files/shared.temp \
          $BUILDBASE/$BLDCYCLE/rc_files/shared

	# set TEMP to the value of BUILDBASE but with '\'s inserted
	# in front of the '/'s
	TEMP=$(echo $BUILDBASE | sed 's/\//\\\//g')

	sed "s/^build_base .*/build_base $TEMP/g" \
	  $BUILDBASE/$BLDCYCLE/rc_files/shared.temp \
	  > $BUILDBASE/$BLDCYCLE/rc_files/shared

	rm $BUILDBASE/$BLDCYCLE/rc_files/shared.temp
	rm $BUILDBASE/$BLDCYCLE/rc_files/SHARED.sv
  else
	log -b "ade_make: PROBLEM WITH FILE: shared"
  fi
#
#	RETAIN SPACING ON ECHO LINES FOR SANDBOX TARGET TOOLS
#
  echo "$BUILDBASE/$BLDCYCLE" > "$OLD_AND_CURRENT_LIST.not_pub_yet"
  mail -s "a build has finished" $CMVC_BECOME \
    < $OLD_AND_CURRENT_LIST.not_pub_yet

  log -b "ade_make: leaving"
}

#
# NAME: create_stage_list
#
# FUNCTION: Creates the ADE stage list for stages to build
#
# INPUT:
#
# OUTPUT: The list of space seperated stages.
#
# SIDE EFFECTS:
#
# RETURNS:
#
create_stage_list ()
{
    list_of_lpps=aix.lpps         # File containing the list of aix LPPs.

    #
    # Build the list of all ADE LPPs to put in the stage list.
    #
    awk '
    {
        if ((! match($0, "^#")) && (length($0) > 0))
        {
            FS="|"
            printf("%s|%s ", $1, $2)
        }
    }' $BUILDBASE/$BLDCYCLE/src/bldscripts/$list_of_lpps
}

#
# NAME: init
#
# FUNCTION: Performs any initialization necessary for the ADE environment
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: 
#
# RETURNS:
#
init ()
{
   typeset success_list failure_list
   typeset stage_list stage
   typeset ans

   if [ -d $BUILDBASE/$BLDCYCLE ]
   then
      cd $BUILDBASE/$BLDCYCLE
      log -b "ade_make: `pwd`"
   else
      logline="ade_make: $BUILDBASE/$BLDCYCLE does not exist."
      logline="${logline}  Check to make sure BUILDBASE and BLDCYCLE"
      log -x "${logline} are set to correct values."
   fi

   chksetbecome
   chksetfamily

   #
   #   ENVIRONMENT VARIABLES 
   #

   CMVC_CMD_HOME="/usr/local/bin"
   TMPDIR=$(bldtmppath)
   LANG="C"
   BUILDERHOME="/u/build"
   OLD_LOG_HOME="$BUILDERHOME/old_logs"
   BUILDLISTPATH="/project/projects"
   BUILDLIST="$BUILDLISTPATH/build_list"
   OLD_AND_CURRENT_LIST="$BUILDLISTPATH/old_and_current_builds_list"
   SAVE_BUILDLIST="$BUILDLISTPATH/.build_list"
   BUILDNOTES=""
   LPPFIXLEVEL=$(date "+%m" "+%d") # LPPFIXLEVEL is just MMDD where
                                   # MM = month, DD = day

   STAGE_LIST=$(create_stage_list)
   for entry in ${STAGE_LIST}
   do
      stage=$(echo $entry | awk -F"|" '{ print $1 }')
      QueryStatus -A $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                  $_SUBTYPE $stage | awk -F"|" '{ print $10 }' | read status
      if [[ "$status" = "$ST_SUCCESS" ]]
      then
         success_list="$success_list $entry"
      else
         failure_list="$failure_list $entry"
      fi
   done

   [[ "${V4BLDNOPROMPT}" = yes ]] && return 0

   if [[ -n "$success_list" ]]
   then
      print "\nThe following stages have completed successfully:\n"
      for entry in $success_list
      do
         stage=$(echo $entry | awk -F"|" '{ print $1 }')
         print "$stage"
      done
      print "\nThese stages will not be run again unless you explicitly"
      confirm "reset their statuses now.  Reset statuses (y/n)?: \c"
      if [[ "$?" = "0" ]]
      then
         print
         for entry in $success_list
         do
            stage=$(echo $entry | awk -F"|" '{ print $1 }')
            release=$(echo $entry | awk -F"|" '{ print $2 }')
            ans=""
            while [ "$ans" = "" ]
            do
               print "$stage: reset status (y/n/q)?: \c"
               read ans < /dev/tty
               ans=$(echo $ans | tr [a-z] [A-Z])
               if [[ "$ans" = Y* ]]
               then
                  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_SUBTYPE $stage
                  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_RELEASE ${release}
               elif [[ "$ans" != N* && "$ans" != Q* ]]
               then
                  ans=""
               fi
            done
            [[ "$ans" = Q* ]] && break;
         done
      fi
   fi

   if [[ -n "$failure_list" ]]
   then
      print "\nThe following stages have not completed successfully:\n"
      for entry in $failure_list
      do
         stage=$(echo $entry | awk -F"|" '{ print $1 }')
         print "$stage"
      done
      print "\nThese stages will be run at this time unless you explicitly"
      confirm "set their statuses to success now.  Set statuses (y/n)?: \c"
      if [[ "$?" = "0" ]]
      then
         print
         for entry in $failure_list
         do
            stage=$(echo $entry | awk -F"|" '{ print $1 }')
            release=$(echo $entry | awk -F"|" '{ print $2 }')
            ans=""
            while [ "$ans" = "" ]
            do
               print "$stage: set status (y/n/q)?: \c"
               read ans < /dev/tty
               ans=$(echo $ans | tr [a-z] [A-Z])
               if [[ "$ans" = Y* ]]
               then
                  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_SUBTYPE $stage
                  bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_SUBTYPE $stage $_STATUS $ST_SUCCESS
                  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_RELEASE ${release}
                  bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                               $_LEVEL $LEVELNAME $_RELEASE ${release} \
                               $_STATUS $ST_SUCCESS
               elif [[ "$ans" != N* && "$ans" != Q* ]]
               then
                  ans=""
               fi
            done
            [[ "$ans" = Q* ]] && break;
         done
      fi
   fi
}

#
# WAIT_UNTIL_SERVER_IS_UP checks to see if the CMVC server is available by
# attempting a "User -view <name> " command and waiting 5 minutes if the 
# return code is not good.  It will do this up to 12 hours, because on 
# saturday the CMVC server is down for a minimum of 9 hours.
#
function WAIT_UNTIL_SERVER_IS_UP
{
  typeset COUNTER=1
  typeset logline

  log -b "WAIT_UNTIL_SERVER_IS_UP : entering on `date`"
  $CMVC_CMD_HOME/User -view $CMVC_BECOME -become $CMVC_BECOME > /dev/null 2>&1
  rc=$?
  until ( [ $rc -eq 0 ] || (( $COUNTER == 144 )) )
  do
        logline="WAIT_UNTIL_SERVER_IS_UP : Problem with doing"
        logline="${logline} a User -view $CMVC_BECOME"
        log -b "${logline}, CMVC not responding yet"
        COUNTER=$COUNTER+1
        sleep 300
  	$CMVC_CMD_HOME/User -view $CMVC_BECOME -become $CMVC_BECOME > \
		/dev/null 2>&1
        rc=$?
  done
  if [ $rc -ne 0 ]
  then
        logline="WAIT_UNTIL_SERVER_IS_UP : The server has been"
        log -x "${logline} down over 12 hours, exiting script on `date`" 
  else
        logline="WAIT_UNTIL_SERVER_IS_UP : CMVC server responded,"
        log -b "${logline} it was available on `date`" 
  fi
  log -b "WAIT_UNTIL_SERVER_IS_UP : leaving on `date`"
}

function CMVC_CONNECTION_CHECK
{
  typeset logline

  log -b "CMVC_CONNECTION_CHECK : entering"
#
# did the CMVC command complete before the server went down?
#
  CONNECTION_CHECK= `grep "connection cannot be established" \
    $TMPDIR/cmvc.commands.stderr`
  if [[ -z $CONNECTION_CHECK ]]
    then
	logline="CMVC command completed before server went down,"
        log -b "${logline} no need to restart command"
	return 2	# returning will break us out of loop
    else
        logline="CMVC command DID NOT COMPLETE before server"
        log -b "${logline} went down, NEED to restart command"
#
#       loop until server is back up
#
        WAIT_UNTIL_SERVER_IS_UP
  fi
  log -b "CMVC_CONNECTION_CHECK : leaving"
}


function execute_stage {
  typeset stage_name status rc cmd log logopts logline
  typeset remarks="/tmp/remarks.$$"

  stage_name=$(echo $1 | awk -F"|" '{ print $1 }')
  # Release this stage comes from.
  export CMVC_RELEASE=$(echo $1 | awk -F"|" '{ print $2 }')
  # Set TDPATH properly.
  bldinit build

  status=$(QueryStatus -A $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
	   $_SUBTYPE $stage_name | awk -F"|" '{ print $10 }')
  [[ "$status" = "$ST_SUCCESS" ]] && return 0
  
  DeleteStatus $_TYPE $T_BLDPTF       $_SUBTYPE $S_BLDGETLISTS \
               $_RELEASE $CMVC_RELEASE     $_STATUS $ST_SUCCESS \
               $_BLDCYCLE $BLDCYCLE

  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE $_RELEASE $CMVC_RELEASE

  DeleteStatus $_TYPE ${T_BLDAFSMERGE} $_BLDCYCLE ${BLDCYCLE} \
               $_RELEASE ${CMVC_RELEASE} $_SUBTYPE ${S_OBJECTMERGE}

  DeleteStatus $_TYPE ${T_BLDAFSMERGE} $_BLDCYCLE ${BLDCYCLE} \
               $_RELEASE ${CMVC_RELEASE} $_SUBTYPE ${S_SHIPMERGE}

  logline="$stage_name stage started on `date`"
  log -b "${logline}"

  cmd="bldscripts/makelpp.sh $V4BLD_LINK $stage_name"
  log="$(get_log_file $stage_name.log $(bldlogpath))"

  bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} "${logline}"

  sh -x $cmd > $log 2>&1
  rc="$?"

  DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE $_SUBTYPE $stage_name
  cat /dev/null > $remarks
  if [[ "$rc" = "0" ]]
  then
	bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
		     $_SUBTYPE $stage_name $_STATUS $ST_SUCCESS
	logline="$stage_name stage completed successfully on `date`"
	logopts="-b"
        bldsetstatus $_TYPE $T_BUILD        $_BLDCYCLE $BLDCYCLE \
                     $_LEVEL $LEVELNAME    $_RELEASE $CMVC_RELEASE \
                     $_STATUS $ST_SUCCESS
  else
	bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
		     $_SUBTYPE $stage_name $_STATUS $ST_FAILURE
  	logline="$stage_name stage failed on `date`"
	logopts="-x"
	echo "A build break occurred, $BLDCYCLE, \c" > $remarks
	echo "during the building of $stage_name.\n" >> $remarks
        bldsetstatus $_TYPE $T_BUILD        $_BLDCYCLE $BLDCYCLE \
                     $_LEVEL $LEVELNAME    $_RELEASE $CMVC_RELEASE \
                     $_STATUS $ST_FAILURE
  fi
  echo "${logline}" >> $remarks
  if [[ "${logopts}" = "-x" && -z "$NOBLDEVENTMAIL" ]]
  then
	while read addr
	do
		mail -s "bldbreak - 4.1 Build break" $addr < $remarks
	done < $BLDENV/usr/bin/bldevent.addrs
  fi
  bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} -f ${remarks}
  rm -rf $remarks
  log ${logopts} "${logline}"
}

###############################################################################
# Prompt user for all required parameters, offering defaults if possible
###############################################################################

getparms()
{
    typeset OLDDEF_LOGV4BLD

    echo

    ###  Set PRODBLD  ###

    PROMPT="default - $PRODBLD"
    echo "Is this a production build? ($PROMPT): \c"
    read ANS
    if [[ "$ANS" = n || "$ANS" = no || ( -z "$ANS" && "$PRODBLD" = no ) ]]
    then
        PRODBLD=no
    else
        PRODBLD=yes
    fi

    ###  Set CMVC_RELEASE  ###

    if [[ -z "$CMVC_RELEASE" ]]
    then
        PROMPT="e.g. bos320"
    else
        PROMPT="default - $CMVC_RELEASE"
        defaultrelset=yes
    fi
    while [[ -z "$CMVC_RELEASE" || -n "$defaultrelset" ]]
    do
        echo "Enter the CMVC Level Release ($PROMPT): \c"
        read ANS
        if [[ -n "$ANS" ]]
        then
            CMVC_RELEASE="$ANS"
        fi
        unset defaultrelset
    done
    bldhostsfile ${CMVC_RELEASE} "${TRUE}"
    [[ $? -ne 0 ]] && { Clean_Up; exit 1; }

    ###  Set BLDCYCLE  ###

    if [[ -z "$BLDCYCLE" ]]
    then
        PROMPT="e.g. 9134"
    else
        PROMPT="default - $BLDCYCLE"
        OLDDEF_LOGV4BLD=$(bldlogpath)/v4bld
        defaultbldcycleset=yes
    fi
    while [[ -z "$BLDCYCLE" || -n "$defaultbldcycleset" ]]
    do
        echo "Enter the Build Cycle ($PROMPT): \c"
        read ANS
        if [[ -n "$ANS" ]]
        then
            BLDCYCLE="$ANS"
            LEVELNAME=${BLDCYCLE}${HOSTSFILE_BLDNODE}
        fi
        unset defaultbldcycleset
    done
    [[ -n "$LEVELNAME" ]] || LEVELNAME=$BLDCYCLE${HOSTSFILE_BLDNODE}

    ###  Set LEVELNAME  ###

    PROMPT="default - $LEVELNAME"
    echo "Enter the Level Name ($PROMPT): \c"
    read ANS
    if expr "$ANS" : "[a-z]$" >/dev/null || expr "$ANS" : "[a-z][a-z]$" >/dev/null
    then
        LEVELNAME=$BLDCYCLE${HOSTSFILE_BLDNODE}$ANS
    else
        [[ -z "$ANS" ]] || LEVELNAME="$ANS"
    fi

    ###  Set LOGV4BLD  ###

    # if v4bld log file is not set or is currently set to the default log
    # file for a previous build cycle, then set it to the default log file
    # value for the current build cycle.
    if [[ -z "$LOGV4BLD" || "$LOGV4BLD" = "$OLDDEF_LOGV4BLD" ]]
    then
        LOGV4BLD=$(bldlogpath)/v4bld
    fi
    PROMPT="default - $LOGV4BLD"
    echo "Enter the log file ($PROMPT): \c"
    read ANS
    [[ -z "$ANS" ]] || LOGV4BLD="$ANS"

    ###  Set STATUS_FILE  ###
    if [[ -z "$STATUS_FILE" ]]
    then
        STATUS_FILE=$(bldhistorypath)/STATUS_FILE
    fi
    PROMPT="default - $STATUS_FILE"
    echo "Enter the status file ($PROMPT): \c"
    read ANS
    [[ -z "$ANS" ]] || STATUS_FILE="$ANS"

    if [[ -z "${BUILDER}" ]]
    then
        chksetbuilder
    else
       PROMPT="default - ${BUILDER}"
       echo "Enter the builder name (${PROMPT}): \c"
       read ANS
       [[ -n "${ANS}" ]] && BUILDER="${ANS}"
    fi

    while [[ -z "$BUILDBASE" ]]
    do
       print "\nEnter the Build Base: \c"
       read BUILDBASE < /dev/tty
       confirm "BUILDBASE = $BUILDBASE\n  Is this correct? (y/n): \c"
       [[ "$?" != "0" ]] && BUILDBASE=""
    done

    # Are we building with a backing tree link?  Default is no.
    if [[ "${V4BLD_LINK-UNSET}" = "UNSET" ]]
    then
        confirm -n "Use the backing tree link? (y/N): \c"
        [[ "$?" = "0" ]] && V4BLD_LINK="-l"
    fi

    showparms
}

###############################################################################
# Set default values for as many variables as possible
###############################################################################

setdefaults()
{
    chksettop

    if [[ -n "$CMVC_RELEASE" ]]
    then
        bldhostsfile ${CMVC_RELEASE} "${TRUE}"
        [[ $? -ne 0 ]] && { Clean_Up; exit 1; }
    fi

    if [[ -z "$LEVELNAME" && -n "$BLDCYCLE" && -n "$CMVC_RELEASE" ]]
    then
        LEVELNAME="$BLDCYCLE${HOSTSFILE_BLDNODE}"
    fi

    if [[ "$PRODBLD" = yes || "$PRODBLD" = bai ]]
    then
        PRODBLD=yes
    else
        PRODBLD=no
    fi
    
    if [[ -z "$LOGV4BLD" && -n "$TOP" && -n "$BLDCYCLE" ]]
    then
        LOGV4BLD=$(bldlogpath)/v4bld
    fi

    if [[ -z "$STATUS_FILE" && -n "$TOP" ]]
    then
        STATUS_FILE=$(bldhistorypath)/STATUS_FILE
    fi
}

###############################################################################
# Set the build cycle phase to 'build' if possible
###############################################################################

setphase ()
{
    CheckPhases test
    if [[ $? = 1 ]] ; then
       if CheckPhases hold ; then
           print "$BLDCYCLE is in the hold phase"
           Clean_Up
           exit 1
       else
           if CheckPhases prebuild build ; then
               if CheckPhases prebuild ; then
                   bldstatus -b $BLDCYCLE
                   [[ $? = 0 ]] || {
                       print -u2 "Error setting $BLDCYCLE phase to build."
                       Clean_Up
                       exit 1
                   }
               fi
           else
               print "$BLDCYCLE is in the wrong phase"
               Clean_Up
               exit 1
           fi
       fi
    fi
}


###############################################################################
# Show the settings of the required variables and prompt for approval
###############################################################################

showparms()
{
    typeset    option=""		# Loop variable.
    typeset -L1 print_option		# English descripition of the option.

    echo "\n  PRODBLD         = $PRODBLD"
    echo "  CMVC_RELEASE    = $CMVC_RELEASE"
    echo "  BLDCYCLE        = $BLDCYCLE"
    echo "  LEVELNAME       = $LEVELNAME"
    echo "  LOGV4BLD        = $LOGV4BLD"
    echo "  STATUS_FILE     = $STATUS_FILE"
    echo "  BUILDER         = ${BUILDER}"
    echo "  BUILDBASE       = $BUILDBASE"
    echo "  V4BLD_LINK      = ${V4BLD_LINK-UNSET}"
    confirm "\nAre these settings correct? (y/n): \c"
	[[ $? = 0 ]] || getparms
}

###############################################################################
# Main input section for setting required environment variables
###############################################################################

trap 'Clean_Up; exit 1' HUP INT TERM QUIT

typeset -x BLDCYCLE
typeset	-x BLDSTARTED="${FALSE}"	# has build been started (i.e. make
					# has been called)
typeset -x BUILDBASE
typeset -x BUILDERHOME
typeset -x BUILDLIST
typeset -x BUILDLISTPATH
typeset -x BUILDNOTES
typeset -x BUILD_TYPE="production"
typeset -x CMVC_CMD_HOME
typeset -x CMVC_RELEASE
#typeset -x GOSPACKAGE
typeset -x LANG
typeset -x LOGV4BLD
typeset -x LPPFIXLEVEL
typeset -x LEVELNAME
typeset -x NODE
typeset -x OLD_AND_CURRENT_LIST
typeset -x OLD_LOG_HOME
typeset -x PRODBLD
typeset -x REL
typeset -x SAVE_BUILDLIST
typeset -x STAGE_LIST
typeset -x STATUS_FILE
typeset -x TMPDIR
typeset    logline			# Used to build up logging message.
typeset    message			# Store a message that needs to be
					# appended to a file and logged.

mach=$(uname -n)
progname=$(basename $0)

#
# Parse the user supplied options.  Insure that no invalid option
# combinations have been used.
#
# Not using the getopts command because detecting the -- option
# is not simple.
#
while [[ $# -ne 0 ]]
do
   case ${1##[-+]} in 
      h) HOSTSFILE="$2"
         shift 2;;
      n) NODENAMES="$2"
         shift 2;;
      ?) Usage;;
      *) log +l -e "'$1' is being ignored because it is not a option to v4bld,"
         log +l -e "make arguments must follow '--' such as: v4bld -- <makeargs>"
         shift 1;;
   esac
done

if [[ "${V4BLDNOPROMPT}" = yes ]]
then
    if [[ -z "$CMVC_RELEASE" || \
          -z "$BLDCYCLE" || \
          -z "$LEVELNAME" || \
          -z "$PRODBLD" || \
          -z "$TOP" || \
	  "${V4BLD_LINK-UNSET}" = "UNSET" || \
          -z "$BUILDER" ]]
    then
        echo "Cannot build in no-prompt mode since one of the following"
        echo "environment variables is not set:"
        echo "CMVC_RELEASE  BLDCYCLE  LEVELNAME  PRODBLD  BUILDER V4BLD_LINK\n"
        Clean_Up
        exit 1
    else
        setdefaults
    fi
else
    setdefaults
    if [[ -z "$BLDCYCLE" || -z "$CMVC_RELEASE" || -z "${BUILDER}" ]]
    then
        getparms
    else
        showparms
    fi
fi

###############################################################################
# Do some initial setup and verification
###############################################################################

chkcreate $TOP/LISTINGS
nohupdir=${nohupdir:=$TOP/LISTINGS/$BLDCYCLE}
chkcreate $nohupdir

export RELEASE_LIST=$BLDENV/usr/bin/RELEASE_LIST

typeset -ix RC=0

bldinit build

logset -c v4bld -C v4bld -F ${LOGV4BLD} -1 ${CMVC_RELEASE} -2 ${LEVELNAME}

setphase

###############################################################################
# Set the listings file and create a script to tail it later
###############################################################################

integer num
LISTNUM=$nohupdir/.nextnum
if [ -f $LISTNUM ]
then
    num=$(cat $LISTNUM)
else
    num="1"
fi

LISTINGS=$nohupdir/$mach.$LEVELNAME.$num
num=num+1
echo $num > $LISTNUM

###############################################################################
# Start the build
###############################################################################

DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDGETLINK $_BLDCYCLE $BLDCYCLE
DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDGETXREF $_BLDCYCLE $BLDCYCLE
DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDMID $_BLDCYCLE $BLDCYCLE

message="Starting \`$progname' at $(date) from $(pwd)."

log -b "${message}"
log -f "Build started on ${HOSTSFILE_REL} from $(uname -n)"
log -f "Build started from $(pwd)"
log -f "PRODBLD           = $PRODBLD"
log -f "CMVC_RELEASE      = $CMVC_RELEASE"
log -f "BLDCYCLE          = $BLDCYCLE"
log -f "LEVELNAME         = $LEVELNAME"
log -f "LOGV4BLD          = $LOGV4BLD"
log -f "STATUS_FILE       = $STATUS_FILE"
log -f "BUILDER           = $BUILDER"
log -f "TOP               = $TOP"
log -f "Listing is in $LISTINGS"

print "${message}" >> ${LISTINGS}

bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} "${message}"

init
BLDSTARTED="${TRUE}"
if [[ "${V4BLDNOPROMPT}" = yes ]]
then
  echo "Use tail.$mach to tail the build."
  ade_make >> $LISTINGS 2>&1; RC=$?
else
  # The following gorp is needed to get the return code from ade_make while
  # still writing all stdout and stderr to $LISTINGS and the screen
  exec 4>&1
  ((ade_make 2>&1; echo $? >&3) \
    | tee -a $LISTINGS 1>&4 2>&4) 3>&1 \
    | read RC
  exec 4>&-
fi

message="Completed \`$progname' at $(date) with return code $RC."
BLDSTARTED="${FALSE}"

print "${message}" >> ${LISTINGS}

bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} "${message}"

if [[ ${RC} -ne 0 ]]
then
   log +l -e "${message}"
else
   log -b "${message}"
fi

Clean_Up

exit $RC
