#!/bin/ksh
# @(#)15        1.4  UpdateReference.sh, pkgtools, bos325 1/19/93 10:25:53
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: UpdateReference
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
SYNTAX: ${program_name} [-d][+f][-n][-o][-p][+s][-t][-x] -h | -i | FileName
FUNCTION: This program updates the reference field for a given list of
CMVC defects.  The reference field of the defect is updated with the
newly created RETAIN apar number.
PARAMETERS: Either the name of a file is specified as a command line
argument or a list of defects is specified via stdin.  Each line of input is
expected to contain an CMVC defect number followed by a RETAIN apar number.
FLAGS:
-d (debug mode) indicates that actual
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the list of defects is to be read from standard input
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects for which the reference field is updated
   is to be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
+s turns off the displaying of status messages to standard output
-t turns on test mode where the RETAIN test system is used
-x turns on tracing of the script, results are written to stderr
EOM
}

function call_exit
{
  file_cleanup
  exit ${1}
}

function display_msg
{
  print -u2 -r ${@}

  if [[ ${status_on} = 0 ]]
  then
    print -u1 -r ${@}
  fi
}

function file_cleanup
{
  # close "upd" output file
  exec 4<&-

  # close "notupd" output file
  exec 5<&-
}

function get_prefix
{
  typeset -LZ defect_no=${1}

  # get the defect's prefix
  if [[ ${CMVC_VERSION} = "2" ]]
  then
     prefix=$(Report -view defectview -where "name='${defect_no}'" -raw \
            -become ${CMVC_ID} | dv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
     rc=${?}
  else
     prefix=$(Report -view defectview -where "name='${defect_no}'" -raw \
            -become ${CMVC_ID} | awk -F"|" '{print $1}' 2> ${errmsg_file})
     rc=${?}
  fi
  # if the command failed
  if ((rc==1))
  then
    display_msg "CMVC command to get prefix failed with return code of ${rc}"
    display_msg "(<${errmsg_file})"
    return 1
  else
    if [[ ${prefix} = 'p' || ${prefix} = 'a' ]]
    then
      return 0
    else
      if [[ ${CMVC_VERSION} = "2" ]]
      then
         prefix=$(Report -view featureview -where "name='${defect_no}'" -raw \
   	    -become ${CMVC_ID} | fv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
         rc=${?}
      else
         prefix=$(Report -view featureview -where "name='${defect_no}'" -raw \
   	    -become ${CMVC_ID} | awk -F"|" '{print $1}' 2> ${errmsg_file})
         rc=${?}
      fi
      # if the command failed
      if ((rc==1))
      then
	display_msg "CMVC command to get prefix failed with return code of ${rc}"
	display_msg "(<${errmsg_file})"
	return 2
      else
	if [[ ${prefix} = 'd' ]]
	then
	  return 0
	else
	  return 3
	fi
      fi
    fi
  fi
}

function update_reference
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -LZ defect_no=${1}

  if get_prefix ${1}
  then
    # if test mode is off
    if [[ ${test_mode} = 1 ]]
    then
      if [[ ${prefix} = 'd' ]]
      then
        if [[ ${CMVC_VERSION} = "2" ]]
        then
   	   old_ref=$(Report -view featureview -wh "name='${defect_no}'" -raw \
	       -become ${CMVC_ID} | fv.filter | awk -F"|" '{print $18}' 2> ${errmsg_file})
	   rc=${?}
        else
   	   old_ref=$(Report -view featureview -wh "name='${defect_no}'" -raw \
	       -become ${CMVC_ID} | awk -F"|" '{print $18}' 2> ${errmsg_file})
	   rc=${?}
        fi
	# if the command failed
	if ((rc!=0))
	then
	  display_msg "CMVC command to get reference field failed with return code of ${rc}"
	  display_msg "$(<${errmsg_file})"
	  Feature -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, could not get reference field"
	  return ${rc}
	else
	  Feature -modify ${defect_no} -reference ${2} -become ${CMVC_ID} \
	    -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, reference was '${old_ref}'" 2> ${errmsg_file}
	  rc=${?}
	  # if the command failed
	  if ((rc!=0))
	  then
	    display_msg "CMVC command to set reference field failed with return code of ${rc}"
	    display_msg "$(<${errmsg_file})"
	    Feature -note ${defect_no} -become ${CMVC_ID} \
	    -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, could not change reference field"
	    return ${rc}
	  fi
	fi
      else
        if [[ ${CMVC_VERSION} = "2" ]]
        then
   	   old_ref=$(Report -view defectview -wh "name='${defect_no}'" -raw \
	     -become ${CMVC_ID} | dv.filter | awk -F"|" '{print $26}' 2> ${errmsg_file})
   	   rc=${?}
        else
   	   old_ref=$(Report -view defectview -wh "name='${defect_no}'" -raw \
	     -become ${CMVC_ID} | awk -F"|" '{print $26}' 2> ${errmsg_file})
   	   rc=${?}
        fi
	# if the command failed
	if ((rc!=0))
	then
	  display_msg "CMVC command to get reference field failed with return code of ${rc}"
	  display_msg "$(<${errmsg_file})"
	  Defect -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, could not get reference field"
	  return ${rc}
	else
	  Defect -modify ${defect_no} -reference ${2} -become ${CMVC_ID} \
	    -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, reference field was '${old_ref}'" 2> ${errmsg_file}
	  rc=${?}
	  # if the command failed
	  if ((rc!=0))
	  then
	    display_msg "CMVC command to set reference field failed with return code of ${rc}"
	    display_msg "$(<${errmsg_file})"
	    Defect -note ${defect_no} -become ${CMVC_ID} \
	    -remarks "RETAIN APAR: ${2} CREATED FOR THIS PTM, could not change reference field"
	    return ${rc}
	  fi
	fi
      fi
    else
      if [[ ${prefix} = 'd' ]]
      then
        if [[ ${CMVC_VERSION} = "2" ]]
        then
 	   old_ref=$(Report -view featureview -wh "name='${defect_no}'" -raw \
	     -become ${CMVC_ID} | fv.filter | awk -F"|" '{print $18}' 2> ${errmsg_file})
	   rc=${?}
        else
 	   old_ref=$(Report -view featureview -wh "name='${defect_no}'" -raw \
	     -become ${CMVC_ID} | awk -F"|" '{print $18}' 2> ${errmsg_file})
	   rc=${?}
        fi
	# if the command failed
	if ((rc!=0))
	then
	  display_msg "CMVC command to get reference field failed with return code of ${rc}"
	  display_msg "$(<${errmsg_file})"
	  Feature -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "TEST RETAIN APAR: T${2} CREATED FOR THIS PTM, could not get reference field"
	  return ${rc}
	else
	  Feature -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "TEST RETAIN APAR: T${2} CREATED FOR THIS PTM, reference field is ${old_ref}"
	fi
      else
        if [[ ${CMVC_VERSION} = "2" ]]
        then
  	   old_ref=$(Report -view defectview -wh "name='${defect_no}'" -raw \
	      -become ${CMVC_ID} | dv.filter | awk -F"|" '{print $26}' 2> ${errmsg_file})
	   rc=${?}
        else
  	   old_ref=$(Report -view defectview -wh "name='${defect_no}'" -raw \
	      -become ${CMVC_ID} | awk -F"|" '{print $26}' 2> ${errmsg_file})
	   rc=${?}
        fi
	# if the command failed
	if ((rc!=0))
	then
	  display_msg "CMVC command to get reference field failed with return code of ${rc}"
	  display_msg "$(<${errmsg_file})"
	  Defect -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "TEST RETAIN APAR: T${2} CREATED FOR THIS PTM, could not get reference field"
	  return ${rc}
	else
	  Defect -note ${defect_no} -become ${CMVC_ID} \
	  -remarks "TEST RETAIN APAR: T${2} CREATED FOR THIS PTM, reference field is ${old_ref}"
	fi
      fi
    fi
    return 0
  else
    return 1
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

  # if the builder environment variable (i.e. CMVC ID) is not set,
  # use the default
  CMVC_ID="${CMVC_ID:=mikemc}"

  CMVC_VERSION=${CMVC_VERSION:=2}

  # path to find RETAIN.COMPIDS file
  RETAIN_PATH="${RETAIN_PATH:=.}"

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
      display_msg \
	"No input file name was specified and no list of defects was supplied"
      display_msg "No action will be taken."
      call_exit 2
    fi

    # create the base name of the output files and relate them to the
    # name of the input file
    base_name=${1##*/}
    base_name=${base_name%.*}
    # set the base name of the output files to the RETAIN_OUTPUT variable
    base_name="${RETAIN_OUTPUT}/${base_name}"
  else
    display_msg "Reading ptf list from STDIN"
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

  upd_file="${base_name}.upd"
  notupd_file="${base_name}.notupd"
  errmsg_file="${base_name}.${action}.errmsg"

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    if [[ -f ${notupd_file} ]]
    then
      mv ${notupd_file} "${notupd_file}${time_suffix}"
    fi
  fi

  # open upd output file as descriptor 4
  exec 4> ${upd_file}
  # open notupd output file as descriptor 5
  exec 5> ${notupd_file}

  while read -u${input_data} defect_num apar_num remainder
  do
    if update_reference ${defect_num} ${apar_num}
    then
      print -u4 "${defect_num}\t${apar_num}"
    else
      print -u5 "${defect_num}\t${apar_num}\tNOT UPDATED"
    fi
  done

  return 0
} ## END OF MAIN ##

function bailout
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  file_cleanup

  display_msg "CreateApar Program Interrupted"
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
  call_exit 3
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
test_option=""
files_option=""
status_option=""
tracing_option=""
path_option=""
typeset -i dummy_apar_num=90000
typeset -i index

# check for command line options
while getopts :dhifn:op:stx next_option
do
  case ${next_option} in
  d) ((debug_mode=0));;
  h) display_help
     call_exit 1;;
  i) ((from_stdin=0));;
  +f) ((save_files=1))
      files_option="+f";;
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
  x) ((tracing_on=0))
     tracing_option="-x";;
  \?) display_msg "${program_name}: unknown option ${OPTARG}"
      call_exit 2;;
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
file_cleanup
exit ${rc}
##### END OF PROGRAM #####
