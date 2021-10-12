#!/bin/ksh
# @(#)32        1.18  src/bldenv/pkgtools/rename/OpenApar.sh, pkgtools, bos412, GOLDA411a 4/25/94 09:57:02
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: OpenApar
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
SYNTAX: ${program_name} [-d][+f][+l][-n][-o][-p][+s][-t][-v] -h | -i | FileName
FUNCTION: This program opens the companion RETAIN apar for a given list of
CMVC defects.
If your CMVC ID is not the same as the login ID your are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a file is specified as a command line argument
or a list of defects is specified via stdin.  Each line of input is expected
to contain a single CMVC defect number followed by a single RETAIN APAR number.
NOTE: The correct defect number is not required.  If it is not known, a place
holder number (00000) can be supplied, allowing the program to operate properly.
FLAGS:
-c do not logoff of RETAIN when finished
-d (debug mode) indicates that actual APARs are not to be opened
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the list of defects is to be read from standard input
   instead of from a file
+l indicates that all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects for which RETAIN apars are created
   be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
+s turns off the displaying of status messages to standard output
-t turns on test mode where the RETAIN test system is used
-v indicates that the window which is used to access RETAIN is to be displayed
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
EOM
#
#OUTPUT:
#The primary output file created contains a list of defect numbers
#for which a companion RETAIN apar has been successfully created.
#It is formatted as two columns per line.  This file must be kept as
#input to the next step in the PTF creation process.
#
#The name of this file will be of the constructed by concatenating
#the name of the level, the name of the release or release suffix,
#and the word opened.
#      For example: 9108.320.opened
}

function display_msg
{
  print -u2 ${@}

  if [[ ${status_on} = 0 ]]
  then
    print -u1 ${@}
  fi
}

function log_it
{
  display_msg ${@}

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
}

function command_expect
{
 # this function searches all the possible positions on the screen in
 # which a given "allowed" command may appear

 # the first parameter must be the command that is being searched for

 typeset -i row=3
 typeset -i command_length
 typeset -i end_position

 ((rc=1))
 command_length=${#1}
 ((end_position=70+command_length-1))
 while ((row<=15))
 do
  command=$(hget -t1 ${row},70:${row},${end_position})
  if [[ ${command} = ${1} ]]
  then
   ((rc=0))
   break
  else
   ((row=row+1))
  fi
 done
 return ${rc}
}

function open_apar
{
  # first parameter must be the CMVC defect number or 00000
  # seconda parameter must be the CMVC reference field
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # remove any prefix from the parameter
  apar_num=${2#[iI][xX]}

  if [[ ${debug_mode} = 0 ]]
  then
    return 0
  else
    display_msg "Searching for IX${apar_num}, companion apar for ${1}"
    hsend -t${RETAIN_WAIT} -home "n;-/r ix"${apar_num}
    if hexpect -t${RETAIN_WAIT} "@1,2:APAR= IX${apar_num}"
    then
      log_it "Summary page for IX${apar_num} (${1})"
    else
      if hexpect -t${RETAIN_WAIT} "RETAIN USER SATISFACTION SURVEY"
      then
	hsend -t${RETAIN_WAIT} -pf1
	if hexpect -t${RETAIN_WAIT} "@1,2:APAR= IX${apar_num}"
	then
	  log_it "Summary page for IX${apar_num} (${1})"
	else
	  hsend -t${RETAIN_WAIT} -home "n;-/r ix"${apar_num}
	  if hexpect -t${RETAIN_WAIT} "@1,2:APAR= IX${apar_num}"
	  then
	    log_it "Summary page for IX${apar_num} (${1})"
	  else
	    log_it "Error searching for IX${apar_num} (${1})"
	    if hexpect -t${RETAIN_WAIT} "@21,2:INVALID - PLEASE RETRY"
	    then
	      display_msg "apar number not valid"
	      return 5
	    else
	      display_msg "unknown problem"
	      return 1
	    fi
	  fi
	fi
      else
	log_it "Error searching for IX${apar_num} (${1})"
	if hexpect -t${RETAIN_WAIT} "@21,2:INVALID - PLEASE RETRY"
	then
	  display_msg "apar number not valid"
	  return 5
	else
	  display_msg "unknown problem"
	  return 1
	fi
      fi
    fi

    if hexpect -t${RETAIN_WAIT} "@4,2:STAT= INTRAN"
    then
      if command_expect "TRANS"
      then
	hsend -t${RETAIN_WAIT} "trans"
	if command_expect "RT-RECEIPT"
	then
	  hsend -t${RETAIN_WAIT} "rt"
	  if hexpect -t${RETAIN_WAIT} "@11,2:VER (PA2)"
	  then
	    hsend -t${RETAIN_WAIT} -pa2
	    if hexpect  -t${RETAIN_WAIT} "@11,2:COMPLETE (PF11)"
	    then
	      hsend -t${RETAIN_WAIT} -pf11
	      if hexpect  -t${RETAIN_WAIT} "@11,2:TRANSACTION(S) COMPLETED"
	      then
		log_it "transaction completed"
		return 0
	      else
		log_it "transaction did not complete"
	      fi
	    else
	      log_it "VERIFY did not complete"
	    fi
	  else
	    log_it "RECEIPT did not complete"
	  fi
	else
	  log_it "RECEIPT is not an option"
	fi
      else
	log_it "TRANS is not an option"
      fi
    else
      # is it already open
      if hexpect -t${RETAIN_WAIT} "@4,2:STAT= OPEN"
      then
	log_it "IX${apar_num} is already open"
	return 2
      else
        if hexpect -t${RETAIN_WAIT} "@4,2:STAT= REOP"
        then
	  log_it "IX${apar_num} is already open"
	  return 2
        else
	  # is it already closed
	  if hexpect -t${RETAIN_WAIT} "@4,2:STAT= CLOSED"
	  then
	    log_it "IX${apar_num} is already closed"
	    return 3
	  else
	    log_it "IX${apar_num} is not intran, open, nor closed"
	    return 4
	  fi
	fi
      fi
    fi
  fi
  return 1
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

  # time to wait for expected data to appear
  RETAIN_WAIT="${RETAIN_WAIT:=5}"

  # subdirectory into which all output files are to be stored
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # if the environment variable CMVC_ID is not set, assume that the current
  # login id is also the user's CMVC id
  CMVC_ID="${CMVC_ID:=$(logname)}"

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

  typeset -i vm_connection_made=1
  typeset -i vtam_connection_made=1
  typeset -i retain_connection_made=1
  typeset -i retain_login_successful=1

  typeset -RZ6 defect_num

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of the input file
    if [[ -n ${1} ]]
    then
      # open noref input file as descriptor 3
      exec 3< ${1}
      ((input_data=3))
      display_msg "Reading input from ${1}"
    else
      display_msg "No input file name was specified and no list of defects was supplied via standard input.  No action will be taken."
      exit 2
    fi

    # create the base name of the output files and relate them to the base
    # name of the input file
    base_name=${1##*/}
    base_name=${base_name%.*}
    # set the base name of the output files to the RETAIN_OUTPUT variable
    base_name="${RETAIN_OUTPUT}/${base_name}"
  else
    display_msg "Reading defect list from standard input"
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

  opened_file="${base_name}.opened"
  notopened_file="${base_name}.notopened"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"

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

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    # save any existing versions of the output files
    if [[ -f ${opened_file} ]]
    then
      mv ${opened_file} "${opened_file}${time_suffix}"
    fi
    if [[ -f ${notopened_file} ]]
    then
      mv ${notopened_file} "${notopened_file}${time_suffix}"
    fi
  fi

  # if the -o flag was not specified
  if [[ ${to_stdout} = 1 ]]
  then
    # open open output file as descriptor 4
    exec 4> ${opened_file}
  else
    # assign file 4 to stdout
    exec 4<&1
  fi

  # open noapar output file as descriptor 5
  exec 5> ${notopened_file}

  while read -u${input_data} defect_num reference remainder
  do
    if open_apar ${defect_num} ${reference}
    then
      print -u4 "${defect_num}\t${reference}"
    else
      rc=${?}
      case ${rc} in
      1)
	display_msg "FIRST ATTEMPT TO OPEN APAR ${reference} (${defect_num}) FAILED"
	hsend -t${RETAIN_WAIT} -pf1
	# try once more (after a delay), then give up if it fails a second time
	display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	sleep ${RETAIN_RETRY_WAIT}
	if open_apar ${defect_num} ${reference}
	then
	  print -u4 "${defect_num}\t${reference}"
	else
	  rc=${?}
	  case ${rc} in
	  1)
	    print -u5 "${defect_num}\t${reference}\tAPAR OPEN FAILED - UNKNOWN PROBLEM"
	    hsend -t${RETAIN_WAIT} -pf1
	    ;;
	  2) print -u4 "${defect_num}\t${reference}\tALREADY OPEN";;
	  3) print -u4 "${defect_num}\t${reference}\tALREADY CLOSED";;
	  4) print -u5 "${defect_num}\t${reference}\tAPAR OPEN FAILED - NOT INTRAN";;
	  5) print -u5 "${defect_num}\t${reference}\tNOT VALID NUMBER";;
	  esac
	fi
	;;
      2) print -u4 "${defect_num}\t${reference}\tALREADY OPEN";;
      3) print -u4 "${defect_num}\t${reference}\tALREADY CLOSED";;
      4) print -u5 "${defect_num}\t${reference}\tAPAR OPEN FAILED - NOT INTRAN";;
      5) print -u5 "${defect_num}\t${reference}\tNOT VALID NUMBER";;
      esac
    fi
  done

  # close all output files
  exec 4<&-
  exec 5<&-


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

  display_msg "OpenApar Program Interrupted"
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

typeset -i rc=0
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
typeset -i unannounced_mode=1
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
typeset -i dummy_apar_num=90000

# check for command line options
while getopts :cdhifln:op:stuw:vx next_option
do
  case ${next_option} in
  c) ((continuous_on=0));;
  d) ((debug_mode=0));;
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
     test_option="-t";;
  u) ((unannounced_mode=0));;
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
