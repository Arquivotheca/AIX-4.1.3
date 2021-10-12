#!/bin/ksh
# @(#)30        1.19  src/bldenv/pkgtools/CheckAparStatus.sh, pkgtools, bos41B, 9504A 1/12/95 12:58:30
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: CheckAparStatus
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
SYNTAX: ${program_name} [-d][+f][+l][-n][-o][-p][-r][+s][-t][-x][-v][-V] -h|-i|FileName
FUNCTION: This program determines whether the RETAIN apars for a given list of
CMVC defects have been closed properly such that a PTF can be created for them
If your CMVC ID is not the same as the login ID you are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a file is specified as a command line argument
or a list of defects is specified via stdin.  Each line of input is expected
to contain an CMVC defect number.  The defect number may be followed by a
RETAIN APAR number.  Supplying the APAR saves time in that the program does
not have to extract it from CMVC.
FLAGS:
-c do not logoff of RETAIN when finished
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the list of defects is to be read from standard input
+l indicates that all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects for which RETAIN apars are created
   is to be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
-r indicates that the only the RETAIN portion of the script needs to be run
+s turns off the displaying of status messages to standard output
-t turns on test mode where the RETAIN test system is used
-u searches unannounced (restricted) database for APARs and PTFs
-v indicates that the window which is used to access RETAIN is to be displayed
-V specifies the version of AIX (32 or 41; defaults to 32)
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
-z indicates that only the CMVC portion of the script is to be run
EOM
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

  if [[ ${logging_on} = 0 ]]
  then
    print "~~~~~${@}~~~~~" >> ${retain_log}
    hget -t0 >> ${retain_log}
    print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
  fi
}

function file_cleanup
{
  display_msg "Closing all output files"
  # close output files
  exec 4>&-
  exec 5>&-
  exec 6>&-
  exec 8>&-
  exec 9>&-
}

function apar_closed_per
{
  # first parameter must be CMVC defect number
  # second parameter must be RETAIN apar number
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    display_msg "Searching for ${2}, companion apar for ${1}"
    hsend -t${RETAIN_WAIT} -home "n;-/r "${2}
    if hexpect -t${RETAIN_WAIT} "@1,2:APAR= ${2}"
    then
      log_it "Summary page for ${2} (${1})"
      hexpect -t${RETAIN_WAIT} "@4,2:STAT= CLOSED" "!@4,2:STAT= OPEN" \
                               "!@4,2:STAT= REOP" "!@4,2:STAT= INTRAN" 
      rc=$?
      case $rc in
      0) hexpect -t${RETAIN_WAIT} "@4,16:PER" "!@4,16:USE" "!@4,16:SUG" \
                                   "!@4,16:RET" "!@4,16:CAN" "!@4,16:DOC" \
                                   "!@4,16:MCH" "!@4,16:PRS" "!@4,16:UR" \
                                   "!@4,16:DUA" "!@4,16:DAT" "!@4,16:DUA" \
				   "!@4,16:DUU" "!@4,16:NAR" 
         case $? in
         0) # apar is CLOSED PER
            return 0
            ;;
         *) # apar is CLOSED something other than PER
	    return 4
            ;;
         esac
         ;;
      1|2) # apar is OPEN or REOP 
           return 3
           ;;
      3) # apar is INTRAN
         return 5
         ;;
      *) # apar is not any of the above - so treat like an OPEN apar
         # how the old method would have treated it
         return 3
         ;;
      esac
    else
      log_it "Error searching for ${2} (${1})"
      if hexpect -t${RETAIN_WAIT} "@21,2:INVALID - PLEASE RETRY"
      then
	# apar number not valid
	return 2
      fi
      # unknown problem, wait 5 times the user specified delay and try again
      return 1
    fi
  else
    return 0
  fi
}

function get_apar_ref
{
  # first and only parameter must be CMVC defect number
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -RZ5 apar_num

  display_msg "Looking up apar for ${1}"

  if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
  then
      reference=$( qryAPARByDefect -d ${1} -v ${aix_version} -r \
			 | awk -F"|" '{print $2}')
      rc=$?
      reference=${reference#[tT}
      reference=${reference#[iI][xX]}
      reference=${reference#[aA]00}
      apar_num="${reference}"
      print -u7 "${1}\tIX${apar_num}"
      return rc
  fi

  # check the prefix to determine whether this defect is a ptm/apar or feature
  if [[ ${CMVC_VERSION} = "2" ]]
  then
     prefix=$(Report -view defectview -where "name='${num}'" -raw -become ${CMVC_ID} \
     | dv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
     rc=${?}
  else
     prefix=$(Report -view defectview -where "name='${num}'" -raw -become ${CMVC_ID} \
     | awk -F"|" '{print $1}' 2> ${errmsg_file})
     rc=${?}
  fi
  if ((rc==0))
  then
    if [[ ${prefix} = 'p' || ${prefix} = 'a' ]]
    then
      if [[ ${CMVC_VERSION} = "2" ]]
      then
         reference=$(Report -view defectview \
                -where "name='${num}'" -raw -become ${CMVC_ID} | \
                dv.filter | awk -F"|" '{printf "%s",$26}' 2> ${errmsg_file})
         rc=${?}
      else
         reference=$(Report -view defectview \
                -where "name='${num}'" -raw -become ${CMVC_ID} | \
                awk -F"|" '{printf "%s",$26}' 2> ${errmsg_file})
         rc=${?}
      fi
      if ((rc==0))
      then
	# if test mode, the reference should field should be prefixed with T
	if ((test_mode==0))
	then
	  if [[ ${reference} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
	  then
	    :
	  else
	    reference=$(Report -view noteview -where "defectname='${1}'" -raw -become ${CMVC_ID} |
	    cut -d'|' -f9 | awk '/TEST RETAIN APAR:/{ref = $4};END {printf "%s\n",ref}')
	    if [[ ${reference} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
	    then
	      :
	    else
	      return 1
	    fi
	  fi
	else
	  if [[ -z ${reference} ]]
	  then
	    display_msg "CMVC command to find reference for ${1} did not return anything"
	    ((rc+=100))
	    return ${rc}
	  fi
	fi
      else
	display_msg "CMVC command to find reference for ${1} failed with rc=${rc}"
	display_msg "$(<${errmsg_file})"
	((rc+=100))
	return ${rc}
      fi
    else
      if [[ -n ${prefix} ]]
      then
	display_msg "${1} is not ptm nor apar, must be a feature"
        if [[ ${CMVC_VERSION} = "2" ]]
        then
           prefix=$(Report -view featureview -where "name='${num}'" -raw -become ${CMVC_ID} \
           | fv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
           rc=${?}
        else
           prefix=$(Report -view featureview -where "name='${num}'" -raw -become ${CMVC_ID} \
           | awk -F"|" '{print $1}' 2> ${errmsg_file})
           rc=${?}
        fi
	if ((rc==0))
	then
	  # must be a feature instead
          if [[ ${CMVC_VERSION} = "2" ]]
          then
             reference=$(Report -view featureview \
                       -where "name='${num}'" -raw -become ${CMVC_ID} | \
                       fv.filter | awk -F"|" '{printf "%s",$18}' 2> ${errmsg_file})
             rc=${?}
          else
             reference=$(Report -view featureview \
                       -where "name='${num}'" -raw -become ${CMVC_ID} | \
                       awk -F"|" '{printf "%s",$18}' 2> ${errmsg_file})
             rc=${?}
          fi
	  if ((rc==0))
	  then
	    # if test mode, the reference should field should be prefixed with T
	    if ((test_mode==0))
	    then
	      if [[ ${reference} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
	      then
		:
	      else
		reference=$(Report -view noteview -where "defectname='${1}'" -raw -become ${CMVC_ID} |
		cut -d'|' -f9 | awk '/TEST RETAIN APAR:/{ref = $4};END {printf "%s\n",ref}')
		if [[ ${reference} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
		then
		  :
		else
		  return 1
		fi
	      fi
	    else
	      if [[ -z ${reference} ]]
	      then
		display_msg "CMVC command to find reference for ${1} did not return anything"
		((rc+=100))
		return ${rc}
	      fi
	    fi
	  else
	    display_msg "CMVC command to find reference for ${1} failed with rc=${rc}"
	    display_msg "$(<${errmsg_file})"
	    ((rc+=100))
	    return ${rc}
	  fi
	else
	  display_msg "CMVC command to find prefix of ${1} failed with rc=${rc}"
	  display_msg "$(<${errmsg_file})"
	  ((rc+=100))
	  return ${rc}
	fi
      else
	display_msg "CMVC command to find prefix for ${1} did not return anything"
	display_msg "Defect/feature number ${1} probably does not exist"
	((rc+=100))
	return ${rc}
      fi
    fi
    reference=${reference#[tT}
    reference=${reference#[iI][xX]}
    reference=${reference#[aA]00}
    apar_num="${reference}"

    print -u7 "${1}\tIX${apar_num}"
  else
    display "CMVC command to find prefix of ${1} failed with rc=${rc}"
    display "$(<${errmsg_file})"
    ((rc+=100))
    return ${rc}
  fi
  return 0
}

function confirm_presence_of_compid
{
  # first parameter must be a RETAIN apar number
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # make sure that the summary page of the apar is displayed
  if hexpect -t${RETAIN_WAIT} "@1,2:APAR= ${1}"
  then
   # summary page already displayed
   :
  else
   # issue command to display summary page
   hsend -t${RETAIN_WAIT} -home "n;-/r "${1}
  fi

  # get the fixed component id (i.e. the value of the FCOMP field)
  # this is the component id that will be assigned to any ptf that is
  # routed off this apar
  fcomp=$(hget -t0 "8,9:9,17")

  # search for this component id in the compids.table file
  fgrep ":${fcomp}:" ${RETAIN_PATH}/compids.table | grep -v "^#" > /dev/null 2>&1
  # return the results of the search
  return ${?}
}

function main
{
  # first and only parameter is assumed to be name of file that contains
  # list of defects (along with apar numbers when -r flag specified)
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

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # if the environment variable CMVC_ID is not set, assume that the current
  # login id is also the user's CMVC id
  CMVC_ID="${CMVC_ID:=$(logname)}"

  # if the environment variable CMVC_VERSION is not set, assume that the 
  # new CMVC release is used 
  CMVC_VERSION="${CMVC_VERSION:=2}"

  typeset -i rc=0
  typeset -i vm_connection_made=1
  typeset -i vtam_connection_made=1
  typeset -i retain_connection_made=1
  typeset -i retain_login_successful=1

  if [[ ${CMVC_VERSION} = "2" ]]
  then
     typeset -LZ defect_num
  else
     typeset -RZ6 defect_num
  fi

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of the input file
    if [[ -n ${1} ]]
    then
      # open input file as descriptor 3
      exec 3< ${1}
      ((input_data=3))
      display_msg "Reading defect list from file ${1}"
    else
      display_msg \
	"No input file name was specified and no list of defects was supplied."
      display_msg "No action will be taken."
      return 2
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

  closed_file="${base_name}.closedper"
  badclose_file="${base_name}.badclose"
  notfound_file="${base_name}.notfound"
  badtable_file="${base_name}.badtable"
  cmvc_file="${base_name}.crossref"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"
  stillopen_file="${base_name}.stillopen"
  intran_file="${base_name}.intran"

  # if the retain only flag was not specified
  if [[ ${retain_only} = 1 ]]
  then
    # if the save files mode was not turned off
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${cmvc_file} ]]
      then
	mv ${cmvc_file} "${cmvc_file}${time_suffix}"
	display_msg "Saving existing intermediate data file"
      fi
    fi

    display_msg "Opening intermediate data file as ${cmvc_file}"
    # open cmvc data output file as descriptor 7
    exec 7> ${cmvc_file}

    while read -u${input_data} defect_num reference_num remainder
    do
      if [[ -z ${reference_num} ]]
      then
	if get_apar_ref ${defect_num}
	then
	  :
	else
	  rc=${?}
	  display_msg "Failed to obtain apar for ${defect_num}"
	  print -u7 "${defect_num}\tAPAR lookup FAILED"
	  return ${rc}
	fi
      else
	reference=${reference_num#[tT]}
	reference=${reference#[iI][xX]}
	reference=${reference#[aA]00}
	apar_num="${reference}"
	print -u7 "${defect_num}\tIX${apar_num}"
      fi
    done

    display_msg "All CMVC data extracted, closing intermediate data file"
    # close CMVC data file
    exec 7<&-
    number_of_defects=$(cat ${cmvc_file} | wc -l)
    display_msg "Total number of defects is ${number_of_defects}"
  fi

  if ((cmvc_only==0))
  then
    # don't run RETAIN portion of the program
    :
  else
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
	return 2
      fi
    fi

    # if the save files mode was not turned off
    if [[ ${save_files} = 0 ]]
    then
      display_msg "Saving all existing output files"
      if [[ -f ${closed_file} ]]
      then
	mv ${closed_file} "${closed_file}${time_suffix}"
      fi
      if [[ -f ${badclose_file} ]]
      then
	mv ${badclose_file} "${badclose_file}${time_suffix}"
      fi
      if [[ -f ${notfound_file} ]]
      then
	mv ${notfound_file} "${notfound_file}${time_suffix}"
      fi
      if [[ -f ${badtable_file} ]]
      then
	mv ${badtable_file} "${badtable_file}${time_suffix}"
      fi
      if [[ -f ${stillopen_file} ]]
      then
	mv ${stillopen_file} "${stillopen_file}${time_suffix}"
      fi
      if [[ -f ${intran_file} ]]
      then
        mv ${intran_file} "${intran_file}${time_suffix}"
      fi
    fi

    # if the -o flag was not specified
    if [[ ${to_stdout} = 1 ]]
    then
      # open closed list output file as descriptor 4
      exec 4> ${closed_file}
    else
      # assign file 4 to stdout
      exec 4<&1
    fi

    # open closed but not PER list output file as descriptor 5
    exec 5> ${badclose_file}
    # open notfound list output file as descriptor 6
    exec 6> ${notfound_file}
    # open bad compid table file as descriptor 8
    exec 8> ${badtable_file}
    # open stillopen file as descriptor 9
    exec 9> ${stillopen_file}

    # if the retain only flag was NOT specified
    if [[ ${retain_only} = 1 ]]
    then
      display_msg "Reopening intermediate data file for input"
      # reopen cmvc data output file as descriptor 7 for input
      exec 7< ${cmvc_file}
    else
      # if the standard input flag was NOT specified
      if [[ ${from_stdin} = 1 ]]
      then
	display_msg "Reading intermediate data from file ${1}"
	# redirect descriptor 7 from the input file
	exec 7<&3
      else
	display_msg "Reading intermediate data from standard input"
	# redirect descriptor 7 from standard input
	exec 7<&0
      fi
    fi

    while read -u7 cmvc_defect apar_ref remainder
    do
      if apar_closed_per ${cmvc_defect} ${apar_ref}
      then
	print -u4 "${cmvc_defect}\t${apar_ref}"
      else
	rc=${?}
	case ${rc} in
	1)
	  display_msg "FIRST ATTEMPT TO CHECK APAR STATUS FOR ${cmvc_defect} FAILED"
	  hsend -t${RETAIN_WAIT} -pf1
	  # try once more (after a delay), then give up if it fails a second time
	  display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	  sleep ${RETAIN_RETRY_WAIT}
	  if apar_closed_per ${cmvc_defect} ${apar_ref}
	  then
	    print -u4 "${cmvc_defect}\t${apar_ref}"
	  else
	    rc=${?}
	    case ${rc} in
	    1) print -u6 "${cmvc_defect}\t${apar_ref}\tNOT FOUND - UNKNOWN PROBLEM";;
	    2) print -u6 "${cmvc_defect}\t${apar_ref}\tNOT VALID NUMBER";;
	    3)
	      if [[ ${debug_mode} = 1 ]]
	      then
		apar_status=$(hget -t0 "4,7:4,20")
	      else
		apar_status="CLOSED PER"
	      fi
	      print -u9 "${cmvc_defect}\t${apar_ref}\t${apar_status}"
	      ;;
	    4)
	      if [[ ${debug_mode} = 1 ]]
	      then
		apar_status=$(hget -t0 "4,7:4,20")
	      else
		apar_status="CLOSED PER"
	      fi
	      print -u5 "${cmvc_defect}\t${apar_ref}\t${apar_status}"
	      ;;
            5) 
              apar_status="INTRAN"
              echo "${cmvc_defect}\t${apar_ref}\t${apar_status}" \
                    >> ${intran_file}
              ;;
	    esac
	  fi
	  ;;
	2)
	  print -u6 "${cmvc_defect}\t${apar_ref}\tNOT VALID NUMBER";;
	3)
	  if [[ ${debug_mode} = 1 ]]
	  then
	    apar_status=$(hget -t0 "4,7:4,20")
	  else
	    apar_status="CLOSED PER"
	  fi
	  print -u9 "${cmvc_defect}\t${apar_ref}\t${apar_status}"
	  ;;
	 4)
	  if [[ ${debug_mode} = 1 ]]
	  then
	    apar_status=$(hget -t0 "4,7:4,20")
	  else
	    apar_status="CLOSED PER"
	  fi
	  print -u5 "${cmvc_defect}\t${apar_ref}\t${apar_status}"
	  ;;
         5)
          apar_status="INTRAN"
          echo "${cmvc_defect}\t${apar_ref}\t${apar_status}" \
               >> ${intran_file}
          ;;
	esac
      fi

      # make sure the fixed component ID for this APAR is in the compids.table
      # so the CreatePtf tool will be able to find it
      # an entry for this component must be present because CreatePtf has to
      # supply the component release level during the ROUTE of a ptf
      if confirm_presence_of_compid ${apar_ref}
      then
       :
      else
       print -u8 "${cmvc_defect}\t${apar_ref}\t${fcomp}\tNOT FOUND IN compids.table"
      fi
    done

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

  # delete error message file
  rm ${errmsg_file}

  return 0
} ## END OF MAIN ##

function bailout
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  file_cleanup

  display_msg "CheckAparStatus Program Interrupted"
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
typeset -i unannounced_mode=1
typeset -i retain_only=1
typeset -i cmvc_only=1
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
aix_version="32"

# check for command line options
while getopts :cdhifln:op:rstuvV:w:xz next_option
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
  r) ((retain_only=0));;
  +s) ((status_on=1))
      status_option="+s";;
  t) ((test_mode=0))
     test_option="-t";;
  u) ((unannounced_mode=0));;
  v) viewing_option="-v";;
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]; then
        aix_version="41"
     fi;;
  w) if [[ ${#OPTARG} > 0 ]]
     then
       wait_option="-w${OPTARG}"
       RETAIN_WAIT=${OPTARG}
     fi
     ;;
  x) ((tracing_on=0))
     tracing_option="-x";;
  z) ((cmvc_only=0));;
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
file_cleanup
exit ${rc}
##### END OF PROGRAM #####
