#!/bin/ksh
# SCCSID:  @(#)35  1.20  src/bldenv/pkgtools/LogonRetain.sh, pkgtools, bos412, GOLDA411a 10/7/94 16:07:00
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: LogonRetain
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

function display_help
{
cat <<-EOM
SYNTAX: ${program_name} [-b retain_id][+f][-h][+l][-m][-n][-p][+s][-t][-v][-w][-x]

FUNCTION: Logs on to the normal or test RETAIN system via a VTAM connection

PARAMETERS:
-b retain_id - (optional) a RETAIN ID can be speicified (if none is specified,
the RETAIN ID will be taken from the first line in the .netrc file that lists
retain as the machine)

FLAGS:
+f turns off the default action of saving existing copies of all output files
-h displays this help text
+l indicates that all RETAIN screens are NOT to be logged
-m specifies the name of the buffer space for this program
   (if only a $ is specified, the name is set to /tmp/retain.{process id})
-n specifies the base name of all output files
-p specifies the directory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
+s turns off the displaying of status messages to standard output
-t indicates that the TEST version of RETAIN is to be accessed
-v indicates that the window which is used to access RETAIN is to be displayed
-w specifies the time to wait for a RETAIN command to finish (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
EOM
}

function display_msg
{
  print -u2 ${@}

  if ((status_on==0))
  then
    print -u1 ${@}
  fi
}

function log_it
{
  display_msg ${@}

  if ((logging_on==0))
  then
    print "~~~~~${@}~~~~~" >> ${retain_log}
    hget -t0 >> ${retain_log} 2> /dev/null
    print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
  fi
}

function already_on_retain
{
  if ((tracing_on==0))
  then
    set -x
  fi

  if hsend -t5 -pf1 > ${errmsg_file} 2>&1
  then
    :
  else
    # try once more
    if hsend -t5 -pf1 > ${errmsg_file} 2>&1
    then
      :
    else
      # kill any existing emulator session and start from scratch
      emulator_pid=$(ps -e | grep htn | awk '{print $1}')
      if [[ -n ${emulator_pid} ]]
      then
	kill -1 ${emulator_pid}
      fi
      ((session_killed=0))
      return 1
    fi
  fi

  hexpect -t${QUICK_WAIT} "SELECT ONE OF THE FOLLOWING." && return 0 #SPM
  hsend -t${RETAIN_WAIT} -pf1 > ${errmsg_file} 2>&1
  if hexpect -t${RETAIN_WAIT} "Commands Available" > ${errmsg_file} 2>&1
  then
    hsend -t${RETAIN_WAIT} "n;" > ${errmsg_file} 2>&1
  else
    # try sending the kill key again (PF1) in case the command that was in
    # effect needs more than one to completely terminate
    hsend -t${RETAIN_WAIT} -pf1 > ${errmsg_file} 2>&1
    if hexpect -t${RETAIN_WAIT} "Commands Available" > ${errmsg_file} 2>&1
    then
      hsend -t${RETAIN_WAIT} "n;" > ${errmsg_file} 2>&1
    fi
  fi

  if hexpect -t${RETAIN_WAIT} "SELECT ONE OF THE FOLLOWING.  SYSTEM =" 2> /dev/null
  then
    if ((test_mode==1))
    then
      if hexpect -t${RETAIN_WAIT} "@1,55:OPER" 2> /dev/null
      then
	return 0
      else
	hsend -t5 "logoff" > ${errmsg_file} 2>&1
	hsend -t5 "undial" > ${errmsg_file} 2>&1
	return 1
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@1,55:DEV" 2> /dev/null
      then
	return 0
      else
	hsend -t5 "logoff" > ${errmsg_file} 2>&1
	hsend -t5 "undial" > ${errmsg_file} 2>&1
	return 1
      fi
    fi
  else
    hsend -t5 "logoff" > ${errmsg_file} 2>&1
    hsend -t5 "undial" > ${errmsg_file} 2>&1
    return 1
  fi
}

function log_onto_test_retain
{
  hsend -t${RETAIN_WAIT} "signoff" > ${errmsg_file} 2>&1
  if hexpect -t${RETAIN_WAIT} "SIGNED OFF" 2> /dev/null
  then
    log_it "Signoff of production RETAIN system successful"
    hsend -t${RETAIN_WAIT} "n" > ${errmsg_file} 2>&1
    if hexpect -t${RETAIN_WAIT} "ENTER USER ID/PASSWORD" 2> /dev/null
    then
      # make sure it is the test system
      if hexpect -t${RETAIN_WAIT} "${RETAIN_SYSTEM} TEST" 2> /dev/null
      then
	log_it "Connection to test system ${RETAIN_SYSTEM} made"
	# log into test system
	# get the retain user ID from the .netrc file
	rettest_id=$(grep "machine* retaintest login " $HOME/.netrc |\
	  awk '{print $4}')
	if [[ ${?} > 0 ]]
	then
	  display_msg  "Failed to find RETAINTEST userid in .netrc file"
	  return 1
	fi

	# the the retain password from the .netrc file
	stty -echo 2>/dev/null
	rettest_passwd=$(grep "machine* retaintest login* ${retain_userid}" \
	  $HOME/.netrc | awk '{print $6}')
	if [[ ${?} > 0 ]]
	then
	  stty echo 2>/dev/null
	  echo ""
	  display_msg "Failed to find RETAIN password for ${retain_userid}"
	  return 1
	fi
	stty echo 2>/dev/null
	echo ""

	hsend -t${RETAIN_WAIT} "${rettest_id}/${rettest_passwd}" > ${errmsg_file} 2>&1
	# detemine whether Retain login successful
	if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
	then
	  ((retain_login_successful=0))
	  log_it "RETAIN login successful to test system"
	  return 0
	else
	  # check for expired password
	  if hexpect -t${QUICK_WAIT} "YOUR MONTHLY RETAIN SIGNON HAS EXPIRED" 2> /dev/null
	  then
	    # obtain new RETAIN password
	    hsend -t${QUICK_WAIT} "${rettest_id}/${rettest_passwd}-CHANGE" > ${errmsg_file} 2>&1
	    # see if a new password has been supplied
	    if hexpect -t${RETAIN_WAIT} "NEW PASSWORD IS" 2> /dev/null
	    then
	      log_it "New RETAIN password supplied"
	      # get new password
	      newtest_password=$(hget -t0 "10,19:10,23" 2> /dev/null )
	      log_it "New RETAIN password is '${newtest_password}'"
	      # enter new password on RETAIN command line
	      hsend -t${RETAIN_WAIT} ${newtest_password} > ${errmsg_file} 2>&1
	      # detemine whether Retain login successful
	      if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
	      then
		((retain_login_successful=0))
		log_it "Login successful when new password entered- ${newtest_password}"
		# update .netrc file
		echo "$(date)" >> $HOME/.netrc.log
		cat $HOME/.netrc >> $HOME/.netrc.log
		cp -p $HOME/.netrc $HOME/.netrc.save
		cat $HOME/.netrc.save | sed "/retaintest/s/${rettest_passwd}/${newtest_password}/" > $HOME/.netrc
		return 0
	      else
                if hexpect -t${RETAIN_WAIT} "USA News Main Menu" 2> /dev/null
                then
                   hsend -t${RETAIN_WAIT} -pf1 2> /dev/null
                fi
	        if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
	        then
	          ((retain_login_successful=0))
	          log_it "Login successful when new password entered- ${newtest_password}"
	          # update .netrc file
		  echo "$(date)" >> $HOME/.netrc.log
		  cat $HOME/.netrc >> $HOME/.netrc.log
		  cp -p $HOME/.netrc $HOME/.netrc.save
		  cat $HOME/.netrc.save | sed "/retaintest/s/${rettest_passwd}/${newtest_password}/" > $HOME/.netrc
		  return 0
	        else
		  log_it "Login failed when new password entered- ${newtest_password}"
		  return 1
                fi
	      fi
	    else
	      log_it "Failed to get new RETAIN password"
	      return 1
	    fi
	  else
	    log_it "RETAIN login failed to test system"
	    return 1
	  fi
	fi
      else
	log_it "Login screen reached, but not for ${RETAIN_SYSTEM} TEST"
	return 1
      fi
    else
      log_it "Failed to make connection to ${RETAIN_SYSTEM} TEST"
      return 1
    fi
  else
    log_it "Signoff of normal RETAIN system failed"
    return 1
  fi
}

function log_onto_retain
{
  if ((tracing_on==0))
  then
    set -x
  fi

  # log into Retain
  hsend -t${RETAIN_WAIT} "${retain_userid}/${retain_passwd}" > ${errmsg_file} 2>&1

  # detemine whether Retain login successful
  if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
  then
    log_it "RETAIN login successful to production system"
    # if the test mode flag was specified
    if ((test_mode==0))
    then
      if log_onto_test_retain
      then
	return 0
      else
	return 1
      fi
    else
      ((retain_login_successful=0))
    fi
    return 0
  else
    # log_it "Did not reach selection screen"
    # check for expired password
    if hexpect -t${QUICK_WAIT} "YOUR MONTHLY RETAIN SIGNON HAS EXPIRED" 2> /dev/null
    then
      # obtain new RETAIN password
      hsend -t${RETAIN_WAIT} "${retain_userid}/${retain_passwd}-CHANGE" > ${errmsg_file} 2>&1
      # see if a new password has been supplied
      if hexpect -t${RETAIN_WAIT} "NEW PASSWORD IS" 2> /dev/null
      then
	log_it "New RETAIN password supplied"
	# get new password
	new_password=$(hget -t0 "10,19:10,23" 2> /dev/null )
	log_it "New RETAIN password is '${new_password}'"
	# enter new password on RETAIN command line
	hsend -t${RETAIN_WAIT} ${new_password} > ${errmsg_file} 2>&1
	# detemine whether Retain login successful
	if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
	then
	  log_it "Login successful when new password entered- ${new_password}"
	  # update .netrc file
	  echo "$(date)" >> $HOME/.netrc.log
	  cat $HOME/.netrc >> $HOME/.netrc.log
	  cp -p $HOME/.netrc $HOME/.netrc.save
	  cat $HOME/.netrc.save | sed "/retain/s/${retain_passwd}/${new_password}/" > $HOME/.netrc
	  if ((test_mode==0))
	  then
	    if log_onto_test_retain
	    then
	      return 0
	    else
	      return 1
	    fi
	  else
	    ((retain_login_successful=0))
	    return 0
	  fi
	else
          if hexpect -t${RETAIN_WAIT} "USA News Main Menu" 2> /dev/null
          then
            hsend -t${RETAIN_WAIT} -pf1 2> /dev/null
          fi
          if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
	  then
	    log_it "Login successful when new password entered- ${new_password}"
	    # update .netrc file
	    echo "$(date)" >> $HOME/.netrc.log
	    cat $HOME/.netrc >> $HOME/.netrc.log
	    cp -p $HOME/.netrc $HOME/.netrc.save
	    cat $HOME/.netrc.save | sed "/retain/s/${retain_passwd}/${new_password}/" > $HOME/.netrc
	    if ((test_mode==0))
	    then
	      if log_onto_test_retain
	      then
	        return 0
	      else
	        return 1
	      fi
	    else
	      ((retain_login_successful=0))
	      return 0
	    fi
          else
	    log_it "Login failed when new password entered- ${new_password}"
	    return 1
	  fi
	fi
      else
        log_it "Failed to get new RETAIN password"
        return 1
      fi
    else
      # see if the RETAIN help screen is being displayed
      if hexpect -t${QUICK_WAIT} "General RETAIN Help" 2> /dev/null
      then
       # sent the enter key to bypass it
       hsend -t${QUICK_WAIT} -enter > ${errmsg_file} 2>&1
      fi

      # see if the RETAIN help screen is being displayed
      if hexpect -t${QUICK_WAIT} "USA News Main Menu" 2> /dev/null
      then
       # try invoking the PF1 key to bypass it
       hsend -t${RETAIN_WAIT} -pf1 > ${errmsg_file} 2>&1
      fi

      # detemine whether Retain login successful
      if hexpect -t${RETAIN_WAIT} "SSF.......-...SOFTWARE SUPPORT" 2> /dev/null
      then
	log_it "RETAIN login successful to production system"
	# if the test mode flag was specified
	if ((test_mode==0))
	then
	  if log_onto_test_retain
	  then
	    return 0
	  else
	    return 1
	  fi
	else
	  ((retain_login_successful=0))
	fi
	return 0
      else
	log_it "Retain login failed"
	return 1
      fi
    fi
  fi
}

function make_retain_connection
{
  if ((tracing_on==0))
  then
    set -x
  fi

  # attempt retain connection
  hsend -t${RETAIN_WAIT} "retain ${RETAIN_SYSTEM}" > ${errmsg_file} 2>&1

  # determine whether the Retain connection has been successfully made within
  # the quick wait period
  if hexpect -t${QUICK_WAIT} "ENTER USER ID/PASSWORD" 2> /dev/null
  then
    ((retain_connection_made=0))
    log_it "Connection to RETAIN system ${RETAIN_SYSTEM} made successfully"
    return 0
  else
    # make check for Retain login screen using the user specified timeout
    if hexpect -t${RETAIN_WAIT} "ENTER USER ID/PASSWORD" 2> /dev/null
    then
      ((retain_connection_made=0))
      log_it "Connection to RETAIN system ${RETAIN_SYSTEM} made successfully"
      return 0
    else
      ((retain_connection_made=1))
      log_it "Failed to make connection to ${RETAIN_SYSTEM} within timeout period."
      return 1
    fi
  fi
}

function make_vtam_connection
{
  if ((tracing_on==0))
  then
    set -x
  fi

  # dial into VTAM session
  # (make sure the cursor is at the third field on the screen)
  hsend -t${RETAIN_WAIT} -down -down -down "dial vtam" > ${errmsg_file} 2>&1

  # determine whether VTAM connection has been successfully made within 2
  # seconds
  if hexpect -t${QUICK_WAIT} "Enter Access Code or UNDIAL:" 2> /dev/null
  then
    ((vtam_connection_made=0))
    log_it "VTAM connection made successfully"
    return 0
  else
    # check for vtam connection within user specified timeout period
    if hexpect -t${RETAIN_WAIT} "Enter Access Code or UNDIAL:" 2> /dev/null
    then
      ((vtam_connection_made=0))
      log_it "VTAM connection made successfully"
      return 0
    else
      ((vtam_connection_made=1))
      log_it "Failed to make VTAM connection within timeout period."
      return 1
    fi
  fi
}

function make_vm_connection
{
  if ((tracing_on==0))
  then
    set -x
  fi

  ((session_killed=1))

  # get all the retain user IDs listed in the .netrc file
  userid=$(grep "machine* retain login " $HOME/.netrc |
    awk '{print $4}')
  if [[ ${?} = 0 ]]
  then
    # check each retain ID found
    print "${userid}" |
    while read retain_userid
    do
     # was a specific retain ID specified with the -b(ecome) flag
     if [[ ${#specified_retain_id} > 0 ]]
     then
       # does this ID match the specified retain id
       if [[ ${specified_retain_id} = ${retain_userid} ]]
       then
	 # use this retain id
	 break
       fi
     else
       # use the first retain id found
       break
     fi
    done
  else
    display_msg  "Failed to find any RETAIN userids in .netrc file"
    return 1
  fi

  # get the retain password from the .netrc file
  stty -echo 2>/dev/null
  retain_passwd=$(grep "machine* retain login* ${retain_userid}" $HOME/.netrc |\
    awk '{print $6}')
  if [[ ${?} > 0 ]]
  then
    stty echo 2>/dev/null
    echo ""
    display_msg "Failed to find password for RETAIN ID ${retain_userid}"
    return 1
  fi
  stty echo 2>/dev/null
  echo ""

  # try each host in the above list one by one until a successful
  # connection is made
  for host in ${HOST_LIST}
  do
   log_it "Trying to access VTAM via ${host}"
   ((session_killed=1))

   # invoke the emulator program
   htn -title "RETAIN (${RETAIN_SYSTEM})" $HTN_OPTIONS ${host} \
     ${viewing_option} &

   # save the process id of the emulator
   htn_process_id=${!}

   # determine whether the host has been successfully accessed
   # wait for htn to get started and an additional timeout period
   # for the connect to be made

   # see if the screen has activated within 2 seconds, instead of the
   # user specified time period
   if hexpect -t${QUICK_WAIT} -i "Fill in your USERID and PASSWORD and press ENTER" 2> /dev/null
   then
    # connection successful, continue
    ((vm_connection_made=0))
    log_it "Connection to ${host} made successfully"

    if make_vtam_connection
    then
     if make_retain_connection
     then
       if log_onto_retain
       then
	 return 0
       else
	 return 1
       fi
     else
       log_it "Connection to RETAIN via ${host} failed"
       # kill current emulator session and try next host
       kill -1 ${htn_process_id}
       emulator_pid=$(ps -e | grep htn | awk '{print $1}')
       if [[ -n ${emulator_pid} ]]
       then
	 kill -1 ${emulator_pid}
       fi
       ((session_killed=0))
     fi
    else
     log_it "Connection to VTAM via ${host} failed"
     # connection failed, kill current emulator session and try next host
     kill -1 ${htn_process_id}
     emulator_pid=$(ps -e | grep htn | awk '{print $1}')
     if [[ -n ${emulator_pid} ]]
     then
       kill -1 ${emulator_pid}
     fi
    fi
   else
    # wait for the user specified time period for the screen to activate
    if hexpect -t${RETAIN_WAIT} -i "Fill in your USERID and PASSWORD and press ENTER" 2> /dev/null
    then
     # connection successful, continue
     ((vm_connection_made=0))
     log_it "Connection to ${host} made successfully"

     if make_vtam_connection
     then
       if make_retain_connection
       then
	 if log_onto_retain
	 then
	   return 0
	 else
	   return 1
	 fi
       else
	 log_it "Connection to RETAIN via ${host} failed"
	 # kill current emulator session and try next host
	 kill -1 ${htn_process_id}
	 emulator_pid=$(ps -e | grep htn | awk '{print $1}')
	 if [[ -n ${emulator_pid} ]]
	 then
	   kill -1 ${emulator_pid}
	 fi
       fi
     else
       log_it "Connection to VTAM via ${host} failed"
       # connection failed, kill current emulator session and try next host
       kill -1 ${htn_process_id}
       emulator_pid=$(ps -e | grep htn | awk '{print $1}')
       if [[ -n ${emulator_pid} ]]
       then
	 kill -1 ${emulator_pid}
       fi
     fi
    else
     ((vm_connection_made=1))
     log_it "Connection to ${host} failed"
     # connection failed, kill current emulator session and try next host
     kill -1 ${htn_process_id}
     emulator_pid=$(ps -e | grep htn | awk '{print $1}')
     if [[ -n ${emulator_pid} ]]
     then
       kill -1 ${emulator_pid}
     fi
    fi
   fi
  done

  if ((vm_connection_made>0))
  then
    log_it "Failed to make connection to any VM system within timeout period"
    return 1
  fi

  if ((vtam_connection_made>0))
  then
    log_it "Failed to make connection to VTAM via any VM system within timeout period"
    return 1
  fi

  if ((retain_connection_made>0))
  then
    log_it "Failed to make connection to RETAIN via any VM system within timeout period"
    return 1
  fi
}

function main
{
  if ((tracing_on==0))
  then
    set -x
  fi

  # default Retain system
  RETAIN_SYSTEM="${RETAIN_SYSTEM:=BDC}"

  HTNMAP="${HTNMAP:=/tmp/retain.$(logname)}"

  # set default font for the emulator
  HTN_OPTIONS="${HTN_OPTIONS:=-fzn Rom6.500}"

  # list hosts through which the connection to VTAM should be tried
  HOST_LIST="${HOST_LIST:=ausvm1 ausvm2 ausvm5 ausvm6}"

  # time to wait for expected data to appear
  RETAIN_WAIT="${RETAIN_WAIT:=30}"

  # subdirectory into which all output files are to be stored
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # time to wait for expected data to appear before using the user specified
  # time out set by RETAIN_WAIT
  QUICK_WAIT="${QUICK_WAIT:=2}"

  typeset -i vm_connection_made=1
  typeset -i vtam_connection_made=1
  typeset -i retain_connection_made=1
  typeset -i retain_login_successful=1
  typeset -i session_killed=1

  if [[ -z ${base_name} ]]
  then
    base_name="${RETAIN_OUTPUT}/${action}"
  fi

  retain_log="${base_name}.retainlog"
  errmsg_file="${base_name}.errmsg"

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    if [[ -f ${retain_log} ]]
    then
      mv ${retain_log} "${retain_log}${time_suffix}"
    fi
  fi

  if ((logging_on==0))
  then
    print "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" > ${retain_log};
    print "RETAIN log for ${base_name} - ${time_suffix}" >> ${retain_log};
    print "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" >> ${retain_log};
  fi

  if already_on_retain
  then
    return 0
  else
    if make_vm_connection
    then
      return 0
    else
      if ((session_killed==1))
      then
       if hexpect -t${RETAIN_WAIT} "Enter Access Code or UNDIAL:" 2> /dev/null
       then
	# send undial to terminate htn session normally but do not wait for reply
	hsend -t0 "undial" > ${errmsg_file} 2>&1
       else
	hsend -t${RETAIN_WAIT} clear > ${errmsg_file} 2>&1
	hsend -t${RETAIN_WAIT} "logoff" > ${errmsg_file} 2>&1
	hsend -t${RETAIN_WAIT} clear > ${errmsg_file} 2>&1
	# send undial to terminate htn session normally but do not wait for reply
	hsend -t0 "undial" > ${errmsg_file} 2>&1
       fi
      fi
      return 1
    fi
  fi
} ## END OF MAIN ##

function bailout
{
  if ((tracing_on==0))
  then
    set -x
  fi

  display_msg "LogonRetain Program Interrupted"
  htn_running=$(ps -e | grep htn | awk '{print $1}')
  if [[ ${#htn_running} > 0 ]]
  then
    hsend -t0 -pf1 > ${errmsg_file} 2>&1
    hsend -t0 logoff > ${errmsg_file} 2>&1
    hsend -t0 undial > ${errmsg_file} 2>&1
    htn_running=$(ps -e | grep htn | awk '{print $1}')
    if [[ ${#htn_running} > 0 ]]
    then
      kill -1 ${htn_running}
    fi
  fi
  exit 3
}

##### START OF PROGRAM #####
trap "bailout" HUP INT QUIT TERM

# find base name of the program
program_name=${0##*/}

# remove any program name suffix to use remaining portion to name output files
action=${program_name%.*}

typeset -i save_files=0
typeset -i status_on=0
typeset -i logging_on=0
typeset -i tracing_on=1
typeset -i test_mode=1
typeset viewing_option="-iconic"
typeset specified_retain_id=""

# check for command line options
while getopts :b:dfhlm:n:p:stw:vx next_option
do
  case ${next_option} in
  b) if [[ ${#OPTARG} > 0 ]]
     then
       specified_retain_id=${OPTARG}
     fi
     ;;
  h) display_help
     exit 1;;
  +f) ((save_files=1));;
  +l) ((logging_on=1));;
  m) if [[ ${OPTARG} = "$" ]]
     then
      HTNMAP="/tmp/retain.${$}"
     else
      if [[ -n ${OPTARG} ]]
      then
       HTNMAP=${OPTARG}
      else
       HTNMAP="/tmp/retain.${$}"
      fi
     fi
     ;;
  n) base_name=${OPTARG};;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       path_option="-p${OPTARG}"
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  +s) ((status_on=1));;
  t) ((test_mode=0));;
  w) if [[ ${#OPTARG} > 0 ]]
     then
       RETAIN_WAIT=${OPTARG}
     fi
     ;;
  v) viewing_option="";;
  x) ((tracing_on=0));;
  \?) print -u2 "${program_name}: unknown option ${OPTARG}"
      exit 2;;
  esac
done
# shift the commands line parameters to the left as many positions as
# there are flags
shift OPTIND-1

time_suffix=".$(date +%m).$(date +%d).$(date +%H):$(date +%M):$(date +%S)"

# no write permission by group and other for all created files
umask 022

# call main routine
main ${@}
rc=${?}
exit ${rc}
##### END OF PROGRAM #####
