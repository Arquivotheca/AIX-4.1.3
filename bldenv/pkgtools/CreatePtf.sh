#!/bin/ksh
# @(#)28        1.30  src/bldenv/pkgtools/CreatePtf.sh, pkgtools, bos41B, 9504A 1/12/95 12:58:34
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: CreatePtf
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
SYNTAX: ${program_name} [-d][+f][-l][-n][-o][-p][-r][+s][-t][-v][-w][-x][-P] -h|-i|FileName
FUNCTION: This program creates a ptf for each track of a given list of CMVC
defects.
If your CMVC ID is not the same as the login ID your are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a file is specified as a command line argument
or a list of defects and corresponds data is specified via stdin.  When both
phases of the program are run (i.e. CMVC and RETAIN), each line of input is
expected to contain a single CMVC defect number followed by a single RETAIN
APAR number.  If the specify PTF mode (-P flag) is invoked, the third token
on each line must be a PTF number.
If only the RETAIN portion of the program is run, each line of input is
expected to cotain the CMVC prefix letter, the CMVC defect number, the RETAIN
APAR number, the CMVC severity level, and the CMVC track name.  If more than
one CMVC track exists for a defect, an input line must be supplied for each
track that is being built in this cycle.  If the specify PTF mode (-P flag)
is invoked, the sixth token on each line must be a PTF number.
FLAGS:
-c do not logoff of RETAIN when finished
-d (debug mode) indicates that actual PTFs are not to be created, but
   dummy PTF numbers will be returned as if they were
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the list of defects is to be read from standard input
+l indicates that all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects for which RETAIN PTFs have been created
   are to be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
-r indicates that the only the RETAIN portion of the script needs to be run
   (the input data specified is the output from the CMVC portion)
+s turns off the displaying of status messages to standard output
   (the -o option also suppresses the status display)
-t turns on test mode where the RETAIN test system is used
-u searches unannounced (restricted) database for APARs and PTFs
-v indicates that the window which is used to access RETAIN is to be displayed
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
-P allows specification of the PTF number that is to be assigned
EOM
#OUTPUT:
#There are three output files
#
#List of PTFs created for each apar-
#the name of this file will be of the constructed by appending
#the word ptf to the name of this program - ${action}.ptf
#
#List of APARs for which no ptf was generated
#the name of this file will be of the constructed by appending
#the word noptf to the name of this program - ${action}.noptf
#
#List of APARs that are were invalid or not found
#the name of this file will be of the constructed by appending
#the word invalid to the name of this program - ${action}.invalid
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

function apar_closed_per
{
  # first parameter must be defect number
  # second parameters must be apar number
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    hsend -t${RETAIN_WAIT} -home "n;-/r ix"${2}
    if hexpect -t${RETAIN_WAIT} "@1,2:APAR= IX${2}"
    then
      log_it "Summary page for IX${2} (${1})"
      if hexpect -t${RETAIN_WAIT} "@4,2:STAT= CLOSED"
      then
	if hexpect -t${RETAIN_WAIT} "@4,16:PER"
	then
	  log_it "IX${2} (${1}) has been closed with PER"
	  return 0
	else
	  # apar closed but not with PER
	  log_it "IX${2} (${1}) closed but not with PER"
	  return 4
	fi
      else
	# apar not closed
	log_it "IX${2} (${1}) not closed"
	return 3
      fi
    else
      log_it "Error searching for IX${2}"
      if hexpect -t${RETAIN_WAIT} "@21,2:INVALID - PLEASE RETRY"
      then
	# apar number not valid
	log_it "IX${2} (${1}) is not a valid number"
	return 2
      fi
      # unknown problem
      log_it "Unknown problem searching for IX${2} (${1})"
      return 1
    fi
  else
    log_it "IX${2} (${1}) has been closed with PER"
    return 0
  fi
}

function create_ptf
{
  # first parameter must be prefix number
  # second parameter must be defect number
  # third parameter must be apar number
  # fourth parameter must be severity
  # fifth parameter must be track(release) name
  # sixth parameter, if specified, must be a PTF number

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    # if the PTF number to be assigned was specified on the command line
    if ((specify_ptf==0))
    then
      # switch to alternate signon that has level 8 authority to set PTF number
      hsend -t ${RETAIN_WAIT} -home "n;as/8"
      # see if the switch failed
      if hexpect -t${QUICK_WAIT} "@22,2:INVALID SELECTION"
      then
       log_it "This RETAIN ID does not have level 8 alternate signon capability"
       return 2
      fi
      # see if this number has already been used
      hsend -t${RETAIN_WAIT} -home "n;-/r ${6}"
      if hexpect -t${QUICK_WAIT} "@21,2:NO RECORD FOUND"
      then
       # the ptf does not exist
       :
      else
       if hexpect -t${QUICK_WAIT} "@1,2:PTF= ${6}"
       then
	log_it "PTF number ${6} already exists, it cannot be used again."
	# switch back to normal signon
	hsend -t ${RETAIN_WAIT} -home "n;as/0"
	return 2
       fi
      fi
    fi

    # make sure the summary page of the apar is displayed
    hsend -t${RETAIN_WAIT} -home "n;-/r ix"${3}
    if hexpect -t${RETAIN_WAIT} "@1,2:APAR= IX${3}"
    then
      log_it "Summary page for IX${3}"

      release_component_id=$(hget -t0 7,9:7,17)
      fixed_component_id=$(hget -t0 8,9:8,17)

      cat ${RETAIN_PATH}/compids.table | sed -e '/^[#*   ]/d' | \
	sed -e '/^[     ]*$/d' > /tmp/compids.tmp.$$
      COMPIDS="/tmp/compids.tmp.$$"

      ifs=${IFS}
      IFS=" "

      if [[ -n ${fixed_component_id} ]]
      then
	grep -i ${fixed_component_id} ${COMPIDS} |
	awk -F":" ' NR == 1 {printf "%s %s %s",$2,$4,$5} ' |
	read retain_component retain_comp_release retain_sys_release
	display_msg "retain_component=${retain_component}"
	display_msg "retain_comp_release=${retain_comp_release}"
	display_msg "retain_sys_release=${retain_sys_release}"
	if [[ -z ${retain_component} && -z ${retain_comp_release} && -z ${retain_sys_release} ]]
	then
	   display_msg "Component id: ${fixed_component_id}  not found in compids.table"
	   grep -i "bos320" ${COMPIDS} |
	   awk -F":" '{printf "%s %s %s",$2,$4,$5}' |
	   read retain_component retain_comp_release retain_sys_release
	   display_msg "retain_component=${retain_component}"
	   display_msg "retain_comp_release=${retain_comp_release}"
	   display_msg "retain_sys_release=${retain_sys_release}"
	   if [[ -z ${retain_component} && -z ${retain_comp_release} && -z ${retain_sys_release} ]]
	   then
	      display_msg "bos320 release not found in compids.table file"
	      display_msg "Using hardcoded bos320 component ID and levels."
	      retain_component="575603001"
	      retain_comp_release="320"
	      retain_sys_release="320"
	   else
	      if [[ -z ${retain_component} || -z ${retain_comp_release} || -z ${retain_sys_release} ]]
	      then
		 # use the default values
		 retain_component="575603001"
		 retain_comp_release="320"
		 retain_sys_release="320"
	      fi
	   fi
	fi
      else
	display_msg "RETAIN system does not contain component id."
	display_msg "Using hardcoded bos320 component ID and levels."
	# use the default values
	retain_component="575603001"
	retain_comp_release="320"
	retain_sys_release="320"
      fi

      # remove temp file
      rm -f ${COMPIDS}

      IFS=${ifs}

      # make sure that ROUTE is a valid command
      if command_expect "ROUTE"
      then
	hsend -t${RETAIN_WAIT} "route"
	# make sure that D is one of the valid options for ROUTE
	if hexpect -t${RETAIN_WAIT} "D - CREATE A PTF BUILD REQUEST FOR THIS APAR"
	then
	  log_it "ROUTE page for IX${3}"
	  hsend -t${RETAIN_WAIT} "d"
	  # make sure D option was accepted
	  if hexpect -t${RETAIN_WAIT} "ROUTE OPTION 'D' SELECTED FOR APAR"
	  then
	    log_it "ROUTE D page for IX${3}"
	    # make sure the cursor is at the home position on the screen
	    # (this is entry area for the severity field - 3:15 ?)
	    hsend -t${RETAIN_WAIT} -home

	    # specify defect's severity unless it is four
	    if [[ ${4} = 4 ]]
	    then
	      severity="3"
	    else
	      severity=${4}
	    fi

	    branch=$(hget -t0 "7,27:7,29")
	    if [[ ${#branch} = 0 ]]
	    then
	      if ((test_mode==0))
	      then
		hsend -t${RETAIN_WAIT} -n "${severity}" "n/a" -tab -tab -tab -tab -tab "916"
	      else
		log_it "No branch office number present on APAR IX${3}"
		hsend -t${RETAIN_WAIT} -n "${severity}" "${retain_comp_release}"
	      fi
	    else
	      if ((test_mode==0))
	      then
		hsend -t${RETAIN_WAIT} -n "${severity}"
	      else
		hsend -t${RETAIN_WAIT} -n "${severity}" "${retain_comp_release}"
	      fi
	    fi

	    # if the PTF number to be assigned was specified on the command line
	    if ((specify_ptf==0))
	    then
	      # see if the specification of the PTF number is allowed for this RETAIN ID
	      if hexpect -t${RETAIN_WAIT} "@16,40:ROUTE TO PTF NUMBER: ="
	      then
		hsend -n @16,64 ${6}
	      else
		log_it "This RETAIN ID does not have permission to specify a PTF number"
		hsend -t${RETAIN_WAIT} -pf1
		return 1
	      fi
	    fi

	    hsend -t${RETAIN_WAIT} -enter
	    # check for error
	    if hexpect -t${QUICK_WAIT} "@22,2:ERROR(S) DETECTED"
	    then
	      log_it "Errors on ROUTE/D of IX${3}"
	      # enter PF5 to display reason for failure
	      hsend -t${RETAIN_WAIT} -pf5
	      log_it "Reason for component level error for IX${3}"
	      # read reason for error
	      reason=$(hget -t0 "17,2:17,80")
	      display_msg ${reason}
	      # enter PF5 again to return to route screen
	      hsend -t${RETAIN_WAIT} -pf5
	      hsend -t${RETAIN_WAIT} -pf1
	      return 1
	    fi

	    # indicate all changes have been made and data is to be stored
	    hsend -t${RETAIN_WAIT} -pa2

	    # creation of ptf should have been created successfully
	    # check for successful message
	    if hexpect -t${RETAIN_WAIT} "@11,2:ROUTE COMPLETE - NEW RECORD NUMBER IS"
	    then
	      # read the ptf number
	      ptf_num=$(hget -t0 "11,40:11,46")
	      print -u4 "${2} IX${3} ${5} ${ptf_num}"
	      log_it "PTF created for IX${3}"

	      # display PTF summary page to confirm its existence
	      hsend -t${RETAIN_WAIT} "n;-/r ${ptf_num}"
	      # check for PTF number
	      if hexpect "@1,2:PTF= ${ptf_num}"
	      then
		log_it "Summary page for PTF ${ptf_num}"
	      else
		log_it "Could not find PTF ${ptf_num} created for IX${3}"
	      fi
	      return 0
	    else
	      # creation of ptf failed
	      log_it "Creation of PTF for IX${3} failed"
	      # check for error message to make sure
	      if hexpect -t${RETAIN_WAIT} "@22,2:ERROR(S) DETECTED"
	      then
		log_it "Creation of PTF for IX${3} failed"
		# enter PF5 to display reason for failure
		hsend -t${RETAIN_WAIT} -pf5
		log_it "Reason for failure to create PTF for IX${3}"
		# read reason for error
		reason=$(hget -t0 "17,2:17,80")
		display_msg ${reason}
		# enter PF5 again to return to route screen
		hsend -t${RETAIN_WAIT} -pf5
	      else
		log_it "Creation of PTF for IX${3} failed for unknown reason"
	      fi
	    fi
	    # switch back to normal signon
	    hsend -t ${RETAIN_WAIT} -home "n;as/0"
	  else
	    log_it "Error with ROUTE/D of IX${3}"
	  fi
	else
	  log_it "Error with ROUTE/D of IX${3}"
	fi
      else
	log_it "ROUTE is not valid for IX${3}"
      fi
    else
      log_it "Error displaying IX${3}"
    fi
    hsend -t${RETAIN_WAIT} -pf1
    return 1
  else
    # return a dummy ptf number and indicate successful creation of a ptf
    print -u4 "${2} IX${3} ${5} U4${dummy_ptf_num}"
    ((dummy_ptf_num+=1))
    return 0
  fi
}

function find_tracks
{
  # first parameter must be a defect's prefix
  # second parameter must be a defect number
  # third parameter must be an apar number
  # fourth parameter, if present, must be a ptf number

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${1} = 'p' || ${1} = 'a' ]]
  then
    if [[ ${CMVC_VERSION} = "2" ]]
    then
        severity=$(Report -view defectview \
           -where "name='${num}'" \
           -raw -become ${CMVC_ID} | dv.filter | awk -F"|" '{printf "%s",$8}' 2> ${errmsg_file})
        rc=${?}
    else
        severity=$(Report -view defectview \
           -where "name='${num}'" \
           -raw -become ${CMVC_ID} | awk -F"|" '{printf "%s",$8}' 2> ${errmsg_file})
        rc=${?}
    fi
    if ((rc==0))
    then
      if [[ -z ${severity} ]]
      then
	severity="4"
      fi
    else
      display_msg "CMVC command to get severity failed with return code of ${rc}"
      display_msg "$(<${errmsg_file})"
      return 2
    fi
  else
    # default to severity 4 for features
    severity="4"
  fi

  if [[ -n ${severity} ]]
  then
    tracks=$(Report -view trackview \
	   -where "defectname='${2}' and not state='fix' and \
	   releasename like '%${RELEASE_SUFFIX}'" \
	   -raw -become ${CMVC_ID} | awk -F"|" '{printf "%s ",$1}' 2> ${errmsg_file})
    rc=${?}
    if ((rc==0))
    then
      track=${tracks%%[ ]*}
      if [[ -z ${track} ]]
      then
	print -u7 "${1} ${2} ${3} NOTRACKS ${4}"
      else
	while [[ -n ${track} ]]
	do
	  print -u7 "${1} ${2} ${3} ${severity} ${track} ${4}"
	  tracks=${tracks#*[ ]}
	  track=${tracks%%[ ]*}
	done
      fi
    else
      display_msg "CMVC command to get tracks failed with return code of ${rc}"
      display_msg "$(<${errmsg_file})"
      return 3
    fi
  else
    display_msg "Severity of defect ${1} was not found"
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

  # time to wait for expected data to appear
  RETAIN_WAIT="${RETAIN_WAIT:=5}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # if the environment variable CMVC_ID is not set, assume that the current
  # login id is also the user's CMVC id
  CMVC_ID="${CMVC_ID:=$(logname)}"

  # if the environment variable CMVC_VERSION is not set, assume that the 
  # new CMVC release is used 
  CMVC_VERSION="${CMVC_VERSION:=2}"

  # if the release name was not supplied, use release suffix only for searches
  RELEASE_SUFFIX="${RELEASE_SUFFIX:=320}"

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

  # define a shorter wait time to be used when checking for responses
  QUICK_WAIT="${QUICK_WAIT:=5}"

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

  # create the names of the output files
  ptf_file="${base_name}.ptf"
  noptf_file="${base_name}.noptf"
  invalid_file="${base_name}.invalid"
  cmvc_file="${base_name}.${action}.cmvc"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"

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
      if [[ -f ${invalid_file} ]]
      then
	mv ${invalid_file} "${invalid_file}${time_suffix}"
      fi
    fi

    # open invalid list output file as descriptor 6
    exec 6> ${invalid_file}

    display_msg "Opening intermediate data file as ${cmvc_file}"
    # open cmvc data output file as descriptor 7
    exec 7> ${cmvc_file}

    while read -u${input_data} defect_num reference_num preassigned_ptf remainder
    do
     reference=${reference_num#[iI][xX]}
     reference=${reference#[aA]00}
     apar_num=${reference}
     # check the prefix to determine whether this defect is a ptm/apar or feature
     if [[ ${CMVC_VERSION} = "2" ]]
     then
        prefix=$(Report -view defectview -where "name='${defect_num}'" -raw -become ${CMVC_ID} \
        | dv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
        rc=${?}
     else
        prefix=$(Report -view defectview -where "name='${defect_num}'" -raw -become ${CMVC_ID} \
        | awk -F"|" '{print $1}' 2> ${errmsg_file})
        rc=${?}
     fi
     if ((rc==0))
     then
      if [[ ${prefix} = 'p' || ${prefix} = 'a' ]]
      then
       :
      else
       if [[ ${CMVC_VERSION} = "2" ]]
       then
          prefix=$(Report -view featureview -where "name='${defect_num}'" -raw -become ${CMVC_ID} \
          | fv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
          rc=${?}
       else
          prefix=$(Report -view featureview -where "name='${defect_num}'" -raw -become ${CMVC_ID} \
          | awk -F"|" '{print $1}' 2> ${errmsg_file})
          rc=${?}
       fi
       if ((rc==1))
       then
	display_msg "CMVC command to get feature prefix failed with return code of ${rc}"
	display_msg "$(<${errmsg_file})"
	print -u6 "${defect_num}\t${apar_num}\tGET PREFIX FAILED"
       fi
      fi

      if [[ -z ${prefix} ]]
      then
       print -u6 "${defect_num}\tIX${apar_num}\tNOPREFIX"
      else
       if ((specify_ptf==0))
       then
	if [[ ${preassigned_ptf} = [uU]49[0-9][0-9][0-9][0-9] ]]
	then
	 # find all the tracks for this APAR that match the release being
	 # built and create PTFs for each
	 if find_tracks ${prefix} ${defect_num} ${apar_num} ${preassigned_ptf}
	 then
	   :
	 else
	  display_msg "Failure getting track data"
	  print -u6 "${defect_num}\t${apar_num}\t${preassigned_ptf}\tGET TRACK DATA FAILED"
	 fi
	else
	 display_msg "The PTF number must be in the range U490000 to U499999"
	 print -u6 "${defect_num}\t${apar_num}\t${preassigned_ptf}\tPTF NUMBER NOT IN RANGE"
	fi
       else
	# find all the tracks for this APAR that match the release being
	# built and create PTFs for each
	if find_tracks ${prefix} ${defect_num} ${apar_num}
	then
	  :
	else
	 display_msg "Failure getting track data"
	 print -u6 "${defect_num}\t${apar_num}\tGET TRACK DATA FAILED"
	fi
       fi
      fi
     else
      display_msg "CMVC command to get prefix failed with return code of ${rc}"
      display_msg "$(<${errmsg_file})"
      print -u6 "${defect_num}\t${apar_num}\tGET PREFIX FAILED"
     fi
    done

    display_msg "All CMVC data extracted, closing intermediate data file"
    # close CMVC data file
    exec 7<&-
    number_of_tracks=$(cat ${cmvc_file} | wc -l)
    display_msg "Total number of tracks = ${number_of_tracks}"
  fi

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
    if [[ -f ${ptf_file} ]]
    then
      if ((accumulate_ptfs==0))
      then
	cp -p ${ptf_file} "${ptf_file}${time_suffix}"
      else
	mv ${ptf_file} "${ptf_file}${time_suffix}"
      fi
    fi
    if [[ -f ${noptf_file} ]]
    then
      mv ${noptf_file} "${noclosed_file}${time_suffix}"
    fi
  fi

  # if the -o flag was not specified
  if [[ ${to_stdout} = 1 ]]
  then
    if ((accumulate_ptfs==0))
    then
      # open existing copy of ptf output file as descriptor 4 (i.e. append)
      exec 4>> ${ptf_file}
    else
      # open new version of ptf output file as descriptor 4
      exec 4> ${ptf_file}
    fi
  else
    # assign file 4 to stdout
    exec 4<&1
  fi

  # open not created list output file as descriptor 5
  exec 5> ${noptf_file}

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

  while read -u7 cmvc_prefix cmvc_defect apar_reference cmvc_severity \
    cmvc_track preassigned_ptf remainder
  do
    if apar_closed_per ${cmvc_defect} ${apar_reference}
    then
      if [[ ${cmvc_severity} = "NOTRACKS" ]]
      then
	print -u5 "${cmvc_defect}\tIX${apar_reference}\t${preassigned_ptf}\tNOTRACKS\tPTF NOT CREATED"
      else
	if create_ptf ${cmvc_prefix} ${cmvc_defect} ${apar_reference} \
	  ${cmvc_severity} ${cmvc_track} ${preassigned_ptf}
	then
	  :
	else
	  # if the create failed because the specified number already exists
	  if [[ ${?} = 2 ]]
	  then
	    print -u5 "${cmvc_defect}\tIX${apar_reference}\t${cmvc_track}\t${preassigned_ptf}\tPTF ALREADY EXISTS"
	  else
	   display_msg "FIRST ATTEMPT TO CREATE PTF FOR ${cmvc_defect} FAILED"
	   # try once more (after a delay), then give up if it fails a second time
	   display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	   sleep ${RETAIN_RETRY_WAIT}
	   if create_ptf ${cmvc_prefix} ${cmvc_defect} ${apar_reference} \
	     ${cmvc_severity} ${cmvc_track} ${preassigned_ptf}
	   then
	     :
	   else
	     print -u5 "${cmvc_defect}\tIX${apar_reference}\t${cmvc_track}\t${preassigned_ptf}\tPTF NOT CREATED"
	   fi
	  fi
	fi
      fi
    else
      rc=${?}
      case ${rc} in
      1)
	print -u6 "${cmvc_defect}\tIX${apar_reference}\t${cmvc_track}\t${preassigned_ptf}\tNOT FOUND";;
      2)
	print -u6 "${cmvc_defect}\tIX${apar_reference}\t${cmvc_track}\t${preassigned_ptf}\tNOT VALID NUMBER";;
      3|4)
	apar_status=$(hget -t0 "4,7:4,20")
	print -u5 "${cmvc_defect}\tIX${apar_reference}\t${cmvc_track}\t${preassigned_ptf}\t${apar_status}"
	;;
      esac
    fi
  done

  # close output files
  display_msg "Closing all output files"
  exec 4>&-
  exec 5>&-
  exec 6>&-

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

  display_msg "CreatePtf Program Interrupted"
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
typeset -i retain_only=1
typeset -i status_on=0
typeset -i tracing_on=1
typeset -i logging_on=0
typeset -i continuous_on=1
typeset -i unannounced_mode=1
typeset -i accumulate_ptfs=1
typeset -i specify_ptf=1
typeset -i dummy_ptf_num=90000
debug_option=""
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
preassigned_ptf=""

while getopts :acdhifln:op:rstuvw:xP next_option
do
  case ${next_option} in
  a) ((accumulate_ptfs=0));;
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
  r) ((retain_only=0));;
  +s) ((status_on=1))
     status_option="+s";;
  t) ((test_mode=0))
     test_option='-t';;
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
  P) ((specify_ptf=0));;
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
