#!/bin/ksh
# @(#)12        1.8  src/bldenv/bldtools/CancelPtf.sh, retain, bos412, GOLDA411a 5/28/92 16:26:48
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: CancelPtf
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this release)
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
SYNTAX: ${program_name} [-d][+f][-l][-n][-o][-p][-r][+s][-t][-v][-w][-x] -h|-i|FileName
FUNCTION: This program cancels PTFs that were not used to deliver APAR fixes.
PARAMETERS: Either the name of a file is specified as a command line argument
or a list of ptfs is specified via stdin.  One ptf per line in both cases.
FLAGS:
-c do not logoff of RETAIN when finished
-d (debug mode) indicates that actual PTFs are not to be cancelled, but
   dummy PTF numbers will be returned as if they were
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the list of ptfs is to be read from standard input
+l indicates that all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects for which RETAIN PTFs have been cancelled
   are to be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
+s turns off the displaying of status messages to standard output
   (the -o option also suppresses the status display)
-t turns on test mode where the RETAIN test system is used
-v indicates that the window which is used to access RETAIN is to be displayed
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
EOM
#INPUT:
#The input file is expected to have one PTF number per line.
#OUTPUT:
#There are two output files
#
#List of PTFs for which the cancel operation was successful-
#the name of this file will be of the constructed by appending
#the word cancelled to the name of this program
#
#List of PTFs for which the cancel operation failed or the PTF was not found
#the name of this file will be of the constructed by appending
#the word notcancelled to the name of this program
}

function display_msg
{
  # all parameters are assumed to be part of message to be displayed
  print -u2 ${@}

  if [[ ${status_on} = 0 ]]
  then
    print -u1 ${@}
  fi
}

function log_it
{
  # all parameters are assumed to be part of message to be displayed
  display_msg ${@}

  # if debug mode has NOT been set
  if [[ ${debug_mode} = 1 ]]
  then
    if [[ ${logging_on} = 0 ]]
    then
      print "~~~~~${@}~~~~~" >> ${retain_log}
      hget -t0 >> ${retain_log}
      print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
      if hexpect -w "(PF5)"
      then
        hsend -t${RETAIN_WAIT} -pf5
        print "~~~~~ERROR MESSAGE~~~~~" >> ${retain_log}
        hget -t0 >> ${retain_log}
        print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
        hsend -t${RETAIN_WAIT} -pf5
      fi
    fi
  fi
}

function ptf_is_cancelable
{
  # first parameter must be ptf number
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i tracking_line
  typeset -i last_tracking_line=23
  typeset -i first_tracking_line=14

  # if the debug mode has not be specified
  if [[ ${debug_mode} = 1 ]]
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    # look for the string "PTF= " followed by the ptf number at 1,2
    if hexpect -t${RETAIN_WAIT} "PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # get first four characters of the external status from columns 45-48
      # on the first line
      external_status=$(hget -t0 1,45:1,48)
      if [[ ${external_status} = "OPEN" ]]
      then
        log_it "${1} external status is open"
        # confirm that TRANS is a valid option for this ptf, look for the
        # string "TRANS" starting in column 70, lines 4,5,6, or 7
        if hexpect -t${RETAIN_WAIT} "TRANS"
        then
          # issue the TRANS command just to display the last page of
          # the tracking data
          hsend -t${RETAIN_WAIT} "trans"
          # confirm that TRANS command activated, look for the message at 11,2
          if hexpect -t${RETAIN_WAIT} "COMPLETE (PF11), CNCL (PF1) OR SELECT"
          then
            # search the tracking lines for the current internal status
            ((tracking_line=last_tracking_line))
            while ((tracking_line>=first_tracking_line))
            do
              internal_status=$(hget -t0 ${tracking_line},31:${tracking_line},34)
              if [[ ${#internal_status} > 0 ]]
              then
                last_command=$(hget -t0 ${tracking_line},2:${tracking_line},9)
                break
              fi
              ((tracking_line-=1))
            done
            if [[ ${internal_status} = "REC1" || ${internal_status} = "REC2" || ${internal_status} = "RETN" ]]
            then
              log_it "the internal status indicates its cancelable"
              hsend -t${RETAIN_WAIT} -pf1
              return 0
            else
              log_it "the internal status indicates it is NOT cancelable"
              print -u5 "${1} ${internal_status} - ${last_command}"
              hsend -t${RETAIN_WAIT} -pf1
              return 5
            fi
          else
            log_it "TRANS command did not activate"
            print -u5 "${1} TRANS COMMAND FAILED"
            hsend -t${RETAIN_WAIT} -pf1
            return 4
          fi
        else
          log_it "TRANS is not a valid option"
          print -u5 "${1} TRANS NOT VALID OPTION"
          return 3
        fi
      else
        log_it "${1} external status is not open"
        # get full external status, instead of just first four characters
        external_status=$(hget -t0 1,45:1,54)
        print -u5 "${1} ${external_status}"
        return 2
      fi
    else
      # was the ptf number (record) not found, look for message at 21,2
      if hexpect -t${RETAIN_WAIT} "NO RECORD FOUND"
      then
        log_it "${1} was not found"
        print -u5 "${1} NOT FOUND"
      else
        log_it "Unknown error in search for ${1}"
        print -u5 "${1} UNKNOWN ERROR"
      fi
      return 1
    fi
  else
    return 0
  fi
}

function cancel_ptf
{
  # first parameter must be ptf number
  # remaining parameters must the apars that the ptf fixes
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    # display the summary page of the ptf
    hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
    # make sure the summary page of the ptf is displayed, look for string
    # "PTF= " followed by the ptf number at 1,2
    if hexpect -t${RETAIN_WAIT} "PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # confirm that CLOSE is a valid option for this ptf, look for CLOSE
      # starting in column 70, lines 3,4,5,6,or 7
      if hexpect -t${RETAIN_WAIT} "CLOSE"
      then
        # issue the CLOSE command
        hsend -t${RETAIN_WAIT} "close"
        # confirm that CLOSE command activated and that D (cancel) is an option,
        # look for "D. CANCEL" at 9,9
        if hexpect -t${RETAIN_WAIT} "D. CANCEL"
        then
          # issue the D (cancel) command
          hsend -t${RETAIN_WAIT} "d"
          # confirm that comments screen has been displayed, look for mesage
          # at 6,8
          if hexpect -t${RETAIN_WAIT} "ENTER OPTIONAL COMMENTS"
          then
            hsend -t${RETAIN_WAIT} -n "APAR THIS PTF WAS REQUESTED FOR WILL BE FIXED BY"
            hsend -t${RETAIN_WAIT} -tab
            hsend -t${RETAIN_WAIT} "ANOTHER PTF"
            hsend -t${RETAIN_WAIT} -pf11
            hsend -t${RETAIN_WAIT} -pa2
            # confirm that PTF was cancelled, look for message at 11,2
            if hexpect -t${RETAIN_WAIT} "CLOSE IS COMPLETE"
            then
              log_it "cancel succeeded"
              print -u4 "${1}"
              return 0
            else
              log_it "cancel failed"
              print -u5 "${1} CANCEL FAILED"
              hsend -t${RETAIN_WAIT} -pf1
              return 5
            fi
          else
            log_it "cancel comments screen not reached"
            print -u5 "${1} CANCEL COMMENTS NOT REACHED"
            hsend -t${RETAIN_WAIT} -pf1
            return 4
          fi
        else
          log_it "D cancel is not a valid option"
          print -u5 "${1} CANCEL NOT AN OPTION"
          hsend -t${RETAIN_WAIT} -pf1
          return 3
        fi
       else
         log_it "close is not a valid option"
         print -u5 "${1} CLOSE NOT AN OPTION"
         return 2
       fi
    else
      log_it "Error displaying summary page for ${1}"
      return 1
    fi
  else
    # return a dummy indication that the ptf has been cancelled
    print -u4 "${1}"
    return 0
  fi
}

function main
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # default Retain system
  RETAIN_SYSTEM="${RETAIN_SYSTEM:=BDC}"

  # set font for the emulator
  HTN_OPTIONS="${HTN_OPTIONS:=-fzn Rom6.500}"

  # list hosts to which the connection to VTAM should be tried
  HOST_LIST="${HOST_LIST:=ausvm1 ausvm2 ausvm6 ausvmq}"

  # set retain queue
  RETAIN_QUEUE="${RETAIN_QUEUE:=MIKEMC}"

  # time to wait for expected data to appear
  RETAIN_WAIT="${RETAIN_WAIT:=5}"

  # subdirectory into which all output files are to be stored
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # if the leveler environment variable (i.e. CMVC_ID) is not set, use the default
  CMVC_ID="${CMVC_ID:=mikemc}"

  # if the release name was not supplied, use release suffix only for searches
  RELEASE_SUFFIX="${RELEASE_SUFFIX:=320}"

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

  typeset -RZ6 ptf_num
  typeset -RZ5 apar_num

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of the input file
    if [[ -n ${1} ]]
    then
      # open input file as descriptor 3
      exec 3< ${1}
      ((input_data=3))
      display_msg "Reading input from ${1}"
    else
      display_msg \
        "No input file name was specified and no list of defects was supplied"
      display_msg "No action will be taken."
      exit 2
    fi

    # create the base name of the output files and relate them to the base
    # name of the input file
    base_name=${1##*/}
    base_name=${base_name%.*}
    # set the base name of the output files to the RETAIN_OUTPUT variable
    base_name="${RETAIN_OUTPUT}/${base_name}"
  else
    display_msg "Reading input list from standard input"
    # read input from stdin
    ((input_data=0))
    # create the base name of the output files and relate them to the name
    # of this program
    # if the base name was not specified with the -n flag
    if [[ -z ${base_name} ]]
    then
      base_name="${RETAIN_OUTPUT}/${action}"
    else
      base_name="${RETAIN_OUTPUT}/${base_name}"
    fi
  fi

  # create the names of the output files
  cancel_file="${base_name}.cancelled"
  notcancel_file="${base_name}.notcancelled"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    # save any existing versions of the output files
    if [[ -f ${cancel_file} ]]
    then
      mv ${cancel_file} ${cancel_file}${time_suffix}
    fi
    if [[ -f ${notcancel_file} ]]
    then
      mv ${notcancel_file} ${notcancel_file}${time_suffix}
    fi
  fi

  # if the -o flag was not specified
  if [[ ${to_stdout} = 1 ]]
  then
    # open ptf cancelled file as descriptor 4
    exec 4> ${cancel_file}
  else
    # assign file 4 to stdout
    exec 4<&1
  fi

  # open not cancelled list output file as descriptor 5
  exec 5> ${notcancel_file}

  # if the debug mode has not been set
  if [[ ${debug_mode} = 1 ]]
  then
    display_msg "Attempting to logon to RETAIN"
    LogonRetain ${logging_option} ${tracing_option} ${viewing_option} \
      ${files_option} ${status_option} ${test_option} ${wait_option} \
      -n ${retain_log_base_name} ${path_option}
    if [[ ${?} = 0 ]]
    then
      # continue
      :
    else
      log_it "Could not gain access to RETAIN"
      hsend -t0 undial
      exit 2
    fi
  fi

  while read -u${input_data} ptf_string remainder
  do
    # string any leading 'U' or 'u' from the incoming ptf string
    # before assigning it to the ptf number variable
    ptf_num=${ptf_string#[uU]}
    if ptf_is_cancelable "U${ptf_num}"
    then
      if cancel_ptf "U${ptf_num}"
      then
        :
      else
        display_msg "FIRST ATTEMPT TO CANCEL U${ptf_num} FAILED"
        # try once more (after a delay), then give up if it fails a second time
        display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
        sleep ${RETAIN_RETRY_WAIT}
        if cancel_ptf "U${ptf_num}"
        then
          :
        else
          display_msg "Cancel of U${ptm_num} failed"
        fi
      fi
    else
      display_msg "PTF is not cancelable"
    fi
  done

  # close output files
  display_msg "Closing all output files"
  exec 4>&-
  exec 5>&-

  # if the debug mode has not been set
  if [[ ${debug_mode} = 1 ]]
  then
    # if the continue RETAIN login flag is NOT on
    if [[ ${continuous_on} = 1 ]]
    then
      # logoff RETAIN
      hsend -t${RETAIN_WAIT} -home
      hsend -t${RETAIN_WAIT} logoff
      log_it "Logged off RETAIN"
      hsend -t0 undial
    fi
  fi
  return 0
} ## END OF MAIN ##

function bailout
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  display_msg "LogonRetain Program Interrupted"
  htn_running=$(ps -e | grep htn | awk '{print $1}')
  if [[ ${#htn_running} > 0 ]]
  then
    hsend -t0 -pf1
    hsend -t0 logoff
    hsend -t0 undial
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

display_msg "RUNNING ${program_name} WITH PARAMETERS ${@}"

# remove any program name suffix to use remaining portion to name output files
action=${program_name%.*}

typeset -i debug_mode=1
typeset -i test_mode=1
typeset -i save_files=0
typeset -i from_stdin=1
typeset -i input_data=0
typeset -i to_stdout=1
typeset -i status_on=0
typeset -i tracing_on=1
typeset -i logging_on=0
typeset -i continuous_on=1
debug_option=""
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""

# check for command line options
# -h displays help text
# -i indicates that a list of defects is to be read from standard input
# -o indicates that the list of defects for which ptfs have been created
#    be written to standard output instead of to the default file
# -d turns on debugging
# +f turns off the action of saving existing copies of all output files
# (this overrides the value stored in the environment variable CMVC_ID)
while getopts :cdhifln:op:stvw:v next_option
do
  case ${next_option} in
  c) ((continuous_on=0));;
  d) ((debug_mode=0))
     debug_option="-d";;
  h) display_help
     exit 1;;
  i) ((from_stdin=0));;
  +f) ((save_files=1))
      files_option="+f";;
  +l) ((logging_on=1))
      logging_option="+l";;
  n) base_name=${OPTARG};;
  o) ((to_stdout=0));;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       path_option="-p${OPTARG}"
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  +s) ((status_on=1))
     status_option="+s";;
  t) ((test_mode=0))
     test_option='-t';;
  v) viewing_option="-v";;
  w) if [[ ${#OPTARG} > 0 ]]
     then
       wait_option="-w${OPTARG}"
       RETAIN_WAIT=${OPTARG}
     fi
     ;;
  x) ((tracing_on=0))
     tracing_option="-x";;
  :) display_msg "${program_name}: ${OPTARG} requires a value"
     exit 2;;
  \?) display_msg "${program_name}: unknown option ${OPTARG}"
      exit 2;;
  esac
done
# shift the commands line parameters to the left as many positions as
# there are flags
shift OPTIND-1

if [[ ${to_stdout} = 0 ]]
then
  ((status_on=1))
fi

if [[ ${status_on} = 1 ]]
then
  status_option="+s";
fi

time_suffix=".$(date +%m).$(date +%d).$(date +%H):$(date +%M):$(date +%S)"

# no write permission by group and other for all created files
umask 022
# call main routine
main ${@}
rc=${?}
exit ${rc}
##### END OF PROGRAM #####
