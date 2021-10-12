#!/bin/ksh
# SCCSID:  @(#)79  1.44  src/bldenv/pkgtools/SendPtf.sh, pkgtools, bos41J, 9513A_all 3/7/95 11:27:26
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: 
#       bailout
#       command_expect
#       display_help
#       display_msg
#       get_component_ct
#       get_info_component_data
#       log_it
#       open_ptf
#       ptf_intran
#       ptf_ready_to_send
#       reassign_ptf
#       send_ptf
#       update_ptf_abstract
#       update_abstract
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

function display_help
{
  echo "SYNTAX: ${program_name} [-c] [-d] [+f] [-l] [-n] [-o] [-p] [-r] [+s] "
  echo "                        [-S] [-F] [-R] [-D] [-t] [-v] [-w] [-x] [-O] "
  echo "                        [-A] [-M] [-V]  -h | -i | FileName\n"
  echo "FUNCTION: This program sends PTFs to the distribution center (TRANS "
  echo "	  COPYSENT)  If your CMVC ID is not the same as the login ID "
  echo "	  you are running under, then you must have the environment "
  echo "	  variable CMVC_ID set to your CMVC ID.\n"
  echo "PARAMETERS: Either the name of a file is specified as a command line"
  echo "	  argument, or a list of ptfs followed its corresponding apars"
  echo "	  is specified via stdin.  For both types of input, the format"
  echo "	  is: one ptf number per line, followed by all the apars it "
  echo "	  fixes on the same line.\n"
  echo "FLAGS:"
  echo "  -c do not logoff of RETAIN when finished"
  echo "  -d (debug mode) indicates that actual PTFs are not to be sent, but"
  echo "     dummy PTF numbers will be returned as if they were"
  echo "  +f turns off default action of saving existing copies of all output files"
  echo "  -h displays this help text"
  echo "  -i indicates that the list of ptfs is to be read from standard input"
  echo "  +l indicates that all RETAIN screens are NOT to be logged"
  echo "  -n specifies the base name of all output files when input is from stdin"
  echo "  -o indicates that the list of defects for which RETAIN PTFs have been created"
  echo "     is to be written to standard output instead of to the default output file"
  echo "HIT <ENTER> TO RECEIVE THE REST OF THIS HELP TEXT"
  read
  echo "  -p specifies the subdirectory into which all output files are written"
  echo "     (overrides RETAIN_OUTPUT)"
  echo "  -r indicates that only the RETAIN portion of the script needs to be run"
  echo "     (this flag is passed to the AddSizeNote and AddFilesNote programs)"
  echo "  +s turns off the displaying of status messages to standard output"
  echo "     (the -o option also suppresses the status display)"
  echo "  -S not calculate the size data for PTF"
  echo "  -F CMVC family name"
  echo "  -R CMVC release name"
  echo "  -D ship server directory"
  echo "  -t turns on test mode where the RETAIN test system is used"
  echo "  -v indicates that the window used to access RETAIN is to be displayed"
  echo "  -w specifies the time to wait for a RETAIN command to finish"
  echo "     (overrides RETAIN_WAIT)"
  echo "  -x turns on tracing of the script; results are written to stderr"
  echo "  -O indicates OEM ptfs"
  echo "  -A update abstract data on RETAIN screen  "
  echo "  -M maintenance level data with the ptf subsystem data"
  echo "  -V version of AIX "
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
  # all parameters are assumed to be part of message to be logged
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

#-----------------------------------------------------------------------
# FUNCTION: command_expect
# DESCRIPTION: searches all the possible positions on the screen in
# which a given "allowed" command may appear
# PARAMETERS: (1) the command that is being searched for
#-----------------------------------------------------------------------
function command_expect
{

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

#-----------------------------------------------------------------------
# FUNCTION: ptf_ready_to_send
# DESCRIPTION:
# This function confirms that a PTF is ready to send to distribution.
# If external status is OPEN, RETN, or REOP, it confirms that the 
# internal status indicates ready_to_send.  If the external status is
# INTRAN, it calls open_ptf for the PTF.
# CALLED BY: main
# PARAMETERS:
# (1) ptf number
# (remaining parameters) the apars that the ptf fixes
# SIDE EFFECTS: 
# Constructs $apar_parms, the list of apars that the ptf fixes.
#-----------------------------------------------------------------------
function ptf_ready_to_send
{
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
    # construct list of apars that this ptf fixes
    typeset -i i=2
    while ((i<=${#}))
    do
      eval parameter_index='$'{${i}}
      apar_parms="${apar_parms} ${parameter_index}"
      ((i+=1))
    done

    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # get first four characters of the external status
      external_status=$(hget -t0 1,45:1,48)
      if [[ ${external_status} = "OPEN" || ${external_status} = "RETN" || ${external_status} = "REOP" ]]
      then
	log_it "${1} external status is open, retn or reop"
#       # confirm that this PTF is owned by the Austin change team (i.e. TX2527)
#       change_team_id=$(hget -t0 4,47:4,52)
#       # if the test mode is not OFF
#       if ((test_mode==1))
#       then
#         local_change_team_id="TX2527"
#       else
#         local_change_team_id="DB0100"
#       fi
#       if [[ ${change_team_id} = ${local_change_team_id} ]]
#       then
	  # confirm that TRANS is a valid option for this ptf
	  if command_expect "TRANS"
	  then
	    # issue the TRANS command just to display the last page of
	    # the tracking data
	    hsend -t${RETAIN_WAIT} "trans"
	    # confirm that TRANS command activated
	    if hexpect -t${RETAIN_WAIT} "@11,2:COMPLETE (PF11), CNCL (PF1) OR SELECT"
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
		log_it "the internal status indicates its ready to send"
		# confirm that copysent is a valid option
		if hexpect -t${RETAIN_WAIT} "CS-COPYSENT"
		then
		  hsend -t${RETAIN_WAIT} -pf1
		  return 0
		else
		  log_it "COPYSENT is not a valid option"
                  if [[ ${abstract_flag} = 0 ]]
                  then
		     print -u5 "${1} ${apar_parms}|COPYSENT NOT VALID OPTION"
                  fi
		  hsend -t${RETAIN_WAIT} -pf1
		  return 6
		fi
	      else
		if [[ ${internal_status} = "INTR" ]]
		then
		  return 7
		else
		  log_it "the internal status indicates it is NOT ready to send"
                  if [[ ${abstract_flag} = 0 ]]
                  then
		     print -u5 "${1} ${apar_parms}|${internal_status} - ${last_command}"
                  fi
		  hsend -t${RETAIN_WAIT} -pf1
		  return 5
		fi
	      fi
	    else
	      log_it "TRANS command did not activate"
              if [[ ${abstract_flag} = 0 ]]
              then
	         print -u5 "${1} ${apar_parms}|TRANS COMMAND FAILED"
              fi
	      hsend -t${RETAIN_WAIT} -pf1
	      return 4
	    fi
	  else
	    log_it "TRANS is not a valid option"
            if [[ ${abstract_flag} = 0 ]]
            then
	       print -u5 "${1} ${apar_parms}|TRANS NOT VALID OPTION"
            fi
	    return 3
	  fi
#       else
#         log_it "${1} is owned by another change team - ${change_team_id}"
#         print -u5 "${1} ${apar_parms}|PTF NOT OWNED BY AUSTIN - ${change_team_id}"
#         return 8
#       fi
      else
	log_it "${1} external status is not open, retn or reop"
	# get full external status, instead of just first four characters
	external_status=$(hget -t0 1,45:1,54)
	if [[ ${external_status} = "INTRAN" ]]
	then
	   display_msg "${1} external status is intran"
	   if open_ptf ${1}
	   then
	      :
           else
	      display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	      sleep ${RETAIN_RETRY_WAIT}
	      if open_ptf ${1}
	      then
		 :
              else
		 display_msg "Open of PTF failed"
                 if [[ ${abstract_flag} = 0 ]]    
                 then
                    print -u5 "${1} ${apar_parms}|${external_status}"
                 fi
		 return 2
              fi
           fi
        else
           if [[ ${abstract_flag} = 0 ]]
           then
              print -u5 "${1} ${apar_parms}|${external_status}"
           fi
           return 2
        fi
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@21,2:NO RECORD FOUND"
      then
	log_it "${1} was not found"
        if [[ ${abstract_flag} = 0 ]]
        then
           print -u5 "${1} ${apar_parms}|NOT FOUND"
        fi
      else
	log_it "Unknown error in search for ${1}"
        if [[ ${abstract_flag} = 0 ]]
        then
           print -u5 "${1} ${apar_parms}|UNKNOWN ERROR"
        fi
      fi
      return 1
    fi
  else
    return 0
  fi
}

#-----------------------------------------------------------------------
# FUNCTION: get_component_ct
# DESCRIPTION: get the CT that owns this component
# CALLED BY: reassign_ptf
# PARAMETERS: (1) the RETAIN component id
#-----------------------------------------------------------------------
function get_component_ct
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # display the component id record screen
  hsend -t${RETAIN_WAIT} "n;compid ${1}"
  # if the component id record screen was found
  if hexpect -t${RETAIN_WAIT} "@2,10:${1}"
  then
   # get the CT that owns this component
   new_ct=$(hget -t0 6,20:6,25)
   return 0
  else
   log_it "COMPONENT ID RECORD FOR ${1} NOT FOUND"
   return 1
  fi
}

#-----------------------------------------------------------------------
# FUNCTION: reassign_ptf
# DESCRIPTION: Reassign a given PTF to 'the person who logged onto RETAIN'
# using the component id and release id passed as parameters.
# CALLED BY: send_ptf
# PARAMETERS:
# (1) the ptf number
# (2) the retain component id 
# (3) the retain component release number 
#-----------------------------------------------------------------------
function reassign_ptf
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # find the change team that owns the component id
  if get_component_ct ${2}
  then
   # find out who logged onto RETAIN so the PTF can be assigned to that person
   hsend -t${RETAIN_WAIT} "n;who"
   # get the line that contains the person's full name and login id
   full_name=$(hget -t0 2,2:2,80)
   # get the person's last name from the line, everything up to the first comma
   last_name=${full_name%%,*}

   # display the ptf summary screen
   hsend -t${RETAIN_WAIT} "n;-/r ${1}"

   # change the release level to the one passed as parameter 3
   hsend -t${RETAIN_WAIT} "trans"
   if command_expect "AS-ASSIGN"
   then
     hsend -t${RETAIN_WAIT} "as ${last_name}"
     if hexpect -t${RETAIN_WAIT} "@11,2:VER (PA2)"
     then
       hsend -t${RETAIN_WAIT} -pa2
       if hexpect -t${RETAIN_WAIT} "@11,2:COMPLETE"
       then
	 if command_expect "RR-REROUTE"
	 then
	   hsend -t${RETAIN_WAIT} "rr ${new_ct} ${2}/${last_name}/${3}"
	   if hexpect -t${RETAIN_WAIT} "@11,2:VER (PA2)"
	   then
	     hsend -t${RETAIN_WAIT} -pa2
	     if hexpect -t${RETAIN_WAIT} "@11,2:COMPLETE"
	     then
	       hsend -t${RETAIN_WAIT} -pf11
	       if hexpect -t${RETAIN_WAIT} "@11,2:TRANSACTION(S) COMPLETED"
	       then
		 log_it "RELEASE COMPONENT AND/OR LEVEL CHANGED FOR ${1}"
		 return 0
	       else
		 log_it "TRANSACTION not completed for ${1}"
		 return 1
	       fi
	     else
	       log_it "REROUTE not completed for ${1}"
	       return 1
	     fi
	   else
	     log_it "REROUTE not activated for ${1}"
	     return 1
	   fi
	 else
	   log_it "REROUTE not an option for ${1}"
	   return 1
	 fi
       else
	 log_it "ASSIGN not completed for ${1}"
	 return 1
       fi
     else
       log_it "ASSIGN not activated for ${1}"
       return 1
     fi
   else
     if command_expect "RR-REROUTE"
     then
       hsend -t${RETAIN_WAIT} "rr tx2527 ${2}/mcbride/${3}"
       if hexpect -t${RETAIN_WAIT} "@11,2:VER (PA2)"
       then
	 hsend -t${RETAIN_WAIT} -pa2
	 if hexpect -t${RETAIN_WAIT} "@11,2:COMPLETE"
	 then
	   hsend -t${RETAIN_WAIT} -pf11
	   if hexpect -t${RETAIN_WAIT} "@11,2:TRANSACTION(S) COMPLETED"
	   then
	     log_it "RELEASE LEVEL CHANGED FOR ${1}"
	     return 0
	   else
	     log_it "TRANSACTION not completed for ${1}"
	     return 1
	   fi
	 else
	   log_it "REROUTE not completed for ${1}"
	   return 1
	 fi
       else
	 log_it "REROUTE not activated for ${1}"
	 return 1
       fi
     else
       log_it "Neither ASSIGN nor REROUTE is an option for ${1}"
       return 1
     fi
   fi
  else
   # new component's ID record not found (i.e. get_component_id failed)
   return 1
  fi
}

#-----------------------------------------------------------------------
# FUNCTION: ptf_intran
# DESCRIPTION: Confirms that TRANS is a valid option for this ptf.
# CALLED BY: send_ptf
# PARAMTERS: (1) the ptf number
#-----------------------------------------------------------------------
function ptf_intran
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i tracking_line
  typeset -i last_tracking_line=23
  typeset -i first_tracking_line=14

  # if debug mode was not specified
  if [[ ${debug_mode} = 1 ]]
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # get first fix characters of the external status
      external_status=$(hget -t0 1,45:1,50)
      if [[ ${external_status} = "INTRAN" ]]
      then
        log_it "${1} external status is intran"
        # confirm that TRANS is a valid option for this ptf
        if hexpect -t${RETAIN_WAIT} "TRANS"
        then
          log_it "TRANS is a valid option"
          return 0
        else
          log_it "TRANS is not a valid option"
          return 3
        fi
      else
        log_it "${1} external status is not intran"
        return 2
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@21,2:NO RECORD FOUND"
      then
        log_it "${1} was not found"
      else
        log_it "Unknown error in search for ${1}"
      fi
      return 1
    fi
  else
    return 0
  fi
}

#-----------------------------------------------------------------------
# FUNCTION: open_ptf
# DESCRIPTION: opens a PTF
# CALLED BY: send_ptf
# PARAMETERS: (1) ptf number
#-----------------------------------------------------------------------
function open_ptf
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    # make sure the summary page of the ptf is displayed
    hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      if hexpect -t${RETAIN_WAIT} "TRANS"
      then
        # issue trans command
        hsend -t${RETAIN_WAIT} "trans"
        if hexpect -t${RETAIN_WAIT} "RT-RECEIPT"
        then
          # issue receipt command
          hsend -t${RETAIN_WAIT} "rt"
          if hexpect -t${RETAIN_WAIT} "VER (PA2)"
          then
            # send release level
            hsend -t${RETAIN_WAIT} -pa2
            if hexpect -t${RETAIN_WAIT} "COMPLETE (PF11)"
            then
              hsend -t${RETAIN_WAIT} -pf11
              if hexpect -t${RETAIN_WAIT} "TRANSACTION(S) COMPLETED"
              then
                return 0
              else
                log_it "trans rt failed"
              fi
            else
              log_it "complete trans rt screen not reached"
            fi
          else
            log_it "verify trans rt screen not reached"
          fi
        else
          log_it "receipt not an option"
        fi
      else
        log_it "trans is not an option"
      fi
    else
      log_it "Error displaying summary page for ${1}"
    fi
    hsend -t${RETAIN_WAIT} -pf1
    return 1
  else
    # return a dummy indication that the ptf has been opened
    return 0
  fi
}

#-----------------------------------------------------------------------
# FUNCTION: get_info_component_data
# DESCRIPTION: Gets component data for a given PTF.  Unpacks the ccss
# image to scan the infofile for line beginning with 'COMPID'.  The
# component data is the 6th field in this line.
# CALLED BY: send_ptf
# PARAMETERS: (1) ptf number
#-----------------------------------------------------------------------
function get_info_component_data
{
 if [[ ${tracing_on} = 0 ]]
 then
  set -x
 fi

 # extract the INFO portion of the PTF
 ccss_unpack -c ${ship_dir}/${1}.ptf -i /tmp/${1}.ebcdic.${pid} -t /tmp/${1}.toc.${pid}
 if [[ -s /tmp/${1}.ebcdic.${pid}  ]]
 then
  # convert the INFO data to ASCII and pick out only the component id data
  info_component_data=$(dd if=/tmp/${1}.ebcdic.${pid} conv=ascii cbs=80 2> /dev/null |
    grep ^COMPID | awk '{print $6}')
 fi
 rm /tmp/${1}.ebcdic.${pid} 2> /dev/null
}

#-----------------------------------------------------------------------
# FUNCTION: update_ptf_abstract
# DESCRIPTION: Update the abstract for a given PTF.
# CALLED BY: update_abstract
# PARAMETERS: (1) ptf number
#-----------------------------------------------------------------------
function update_ptf_abstract
{
   hsend -t${RETAIN_WAIT} -home "n;as/2"
   hsend -t${RETAIN_WAIT} "n;-/r ${1}"
   if command_expect "EDIT"
   then
      hsend -t${RETAIN_WAIT} "edit"
      if hexpect -t${RETAIN_WAIT} "MODIFY DATA OPTION ACTIVE,"
      then 
         hsend -n -t${RETAIN_WAIT} "@3,2"
         hsend -n -t${RETAIN_WAIT} "                                                                "
         hsend -n -t${RETAIN_WAIT} "@3,2"
         hsend -n -t${RETAIN_WAIT} "$ML_INFO"
         hsend -t${RETAIN_WAIT} -enter
         if hexpect -t${RETAIN_WAIT} "@11,2:DATA MODIFIED,"
         then 
            hsend -t${RETAIN_WAIT} -pa2
            if hexpect -t${RETAIN_WAIT} "@11,2:EDIT COMPLETED"
            then
               hsend -t${RETAIN_WAIT} -home "n;as/0"
               return 0
            else
               hsend -t${RETAIN_WAIT} -home "n;as/0"
               log_it "FAIL TO UPDATE PTF ${1} ABSTRACT"
               update_abstract_failure="${1} UPDATE ABSTRACT FAILED"
               return 1
            fi
         else
            hsend -t${RETAIN_WAIT} -home "n;as/0"
            log_it "FAIL TO UPDATE PTF ${1} ABSTRACT"
            update_abstract_failure="${1} UPDATE ABSTRACT FAILED" 
            return 1
         fi
      else
         hsend -t${RETAIN_WAIT} -home "n;as/0"
         log_it "EDIT COMMAND FAILED for PTF ${1}"
         update_abstract_failure="${1} EDIT COMMAND FAILED"
         return 1
      fi
   else
      hsend -t${RETAIN_WAIT} -home "n;as/0"
      log_it "EDIT COMMAND NOT FOUND for PTF ${1}"
      update_abstract_failure="${1} EDIT COMMAND NOT FOUND"
      return 1
   fi
}

#-----------------------------------------------------------------------
# FUNCTION: update_abstract
# DESCRIPTION: Update PTF abstract data on RETAIN
# CALLED BY: main
# PARAMETERS: (1) ptf number
#-----------------------------------------------------------------------
function update_abstract
{

#SPM next line added to remove "error" message from log file.
   [[ -f /tmp/${1}.toc.${pid} ]] && \
   cat /tmp/${1}.toc.${pid} |
   awk ' NR == 2 ' |
   while read f1 f2 f3 f4 f5 f6 last
   do
      description=""
      first_word="y"
      for word in $last
      do
         if [ "$first_word" = "y" ]; then
            description="$word"
            first_word="n"
         else
            description="$description $word"
         fi
      done
   done
   echo "$ML_INFO" | grep "$description" > /dev/null 2>&1
   if [ $? -ne 0 ]; then 
      ML_INFO="$ML_INFO $description"
   fi
   length=`print ${#ML_INFO}`
   if [ $length -le 64 ]; then
      if update_ptf_abstract ${1}
      then
         rm -f /tmp/${1}.toc.${pid}
         return 0 
      else
         rm -f /tmp/${1}.toc.${pid}
         return 1
      fi
   else
      log_it "Abstract data is longer than 64 characters"
      rm -f /tmp/${1}.toc.${pid}
      return 1
   fi 
}

#-----------------------------------------------------------------------
# FUNCTION: send_ptf
# DESCRIPTION: Sends a PTF using the RETAIN TRANS, COPYSENT commands.
# First checks to confirm that the component and release data matches
# between the PTF's infofile and the PTF data found in RETAIN.  If 
# either the component or the release data does not match, reassigns
# the RETAIN PTF to the component/release data found in the PTF infofile.
# Verifies that the PTF is in the INTRAN state before sending it.
# CALLED BY: main
# PARAMETERS: (1) ptf number
# (remaining parameters) the apars that the ptf fixes (if test mode is
# specified, the remaining parameters are apar/defect pairs)
#-----------------------------------------------------------------------
function send_ptf
{
  typeset -u info_component
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${debug_mode} = 1 ]]
  then
    # if not test mode
    if ((test_mode==1))
    then
      # make sure the summary page of the ptf is displayed
      hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
      if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
      then
	log_it "Summary page for ${1}"
	if command_expect "TRANS"
	then
	  # find the component ID and release from info file
	  get_info_component_data ${1}
	  # separate the component ID and release
	  info_component=$(print ${info_component_data} | awk -F"," '{print $1}')
	  info_release=$(print ${info_component_data} | awk -F"," '{print $2}')
	  # find the release level 
	  #  get the component ID from the PTF summary screen
	  retain_component=$(hget -t0 4,9:4,17)
	  # get the release level for which this ptf was routed
	  retain_release=$(hget -t0 1,58:1,60)
	  if [[ ${retain_component} != ${info_component} ]]
	  then
	    display_msg "RETAIN component id ${retain_component} does NOT match INFO component id ${info_component}"
	    # change the RETAIN component id to the info component id
	    if reassign_ptf ${1} ${info_component} ${info_release}
	    then
	      # make sure the summary page of the ptf is displayed
	      hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
	    else
	      hsend -t${RETAIN_WAIT} -pf1
	      hsend -t${RETAIN_WAIT} -pf1
	      print -u5 "${1} ${apar_parms}|INCORRECT COMPONENT RELEASE - ${retain_release}"
	      return 1
	    fi
	  else
	    if [[ ${retain_release} != ${info_release} ]]
	    then
	       display_msg "RETAIN release level ${retain_release} does NOT match INFO release level ${info_release}"
	       # change the component release level to the 3.2 version and
	       # change the component id if necessary
	       if reassign_ptf ${1} ${info_component} ${info_release}
	       then
		  # make sure the summary page of the ptf is displayed
		  hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
	       else
		  hsend -t${RETAIN_WAIT} -pf1
		  hsend -t${RETAIN_WAIT} -pf1
		  print -u5 "${1} ${apar_parms}|INCORRECT COMPONENT ID - ${retain_component}"
		  return 1
	       fi
	    fi
	  fi

          # Check if PTF in INTRAN state
          if ptf_intran ${1}
          then
             # Open PTF
             if open_ptf ${1}
             then
                :
             else
                display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
                sleep ${RETAIN_RETRY_WAIT}
                if open_ptf ${1}
                then 
                   :
                else
                   display_msg "Open of PTF failed"
                   return 1
                fi
             fi
          fi
   
          hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
	  # issue trans command
	  hsend -t${RETAIN_WAIT} "trans"
	  if hexpect -t${RETAIN_WAIT} "CS-COPYSENT"
	  then
	    # issue copysent command
	    hsend -t${RETAIN_WAIT} "cs"
	    if hexpect -t${RETAIN_WAIT} "APPLICABLE RELEASE:"
	    then
	      # send release level
	      hsend -t${RETAIN_WAIT} "AR${info_release}"
	      hsend -t${RETAIN_WAIT} -pf11
	      hsend -t${RETAIN_WAIT} -pa2
	      if hexpect -t${RETAIN_WAIT} "ENVIRONMENT:"
	      then
		# send environment string
		hsend -t${RETAIN_WAIT} "Selective Fix for AIX ${aix_version}"
		hsend -t${RETAIN_WAIT} -pf11
		hsend -t${RETAIN_WAIT} -pa2
		if hexpect -t${RETAIN_WAIT} "APARS FIXED:"
		then
		  # send list of apars that this ptf fixes
		  # start with the second parameter
		  typeset -i i=2
		  # there is room for only six apars on the first line
		  typeset -i j=8
		  typeset -i k
		  typeset -i l
		  typeset -i m=2
		  typeset -i p=2
		  # the maximum number of lines of input on one page is nine
		  typeset -i n=1
		  while ((i<=${#}))
		  do
		    eval parameter_index='$'{${i}}
		    hsend -t${RETAIN_WAIT} -n ${parameter_index}
		    eval l=i-1+m
		    eval k=l%j
		    if ((k==0))
		    then
		      if ((n==9))
		      then
		       ((n=1))
		       hsend -t${RETAIN_WAIT} -enter
		       hsend -t${RETAIN_WAIT} -pa2
		       hexpect -t2 "OPTIONAL" && p=0
		       ((m+=p))
		      else
		       ((n+=1))
		       # hsend -t${RETAIN_WAIT} -tab
		       #((m=6))
		       # there is room for eight apars on the remaining lines
		       #((j=8))
		      fi
		    fi
		    ((i+=1))
		  done
		  hsend -t${RETAIN_WAIT} -pf11
		  hsend -t${RETAIN_WAIT} -pa2
		  if hexpect -t${RETAIN_WAIT} "APPLICABLE LEVEL:"
		  then
		    hsend -t${RETAIN_WAIT} -n ${info_release}
		    hsend -t${RETAIN_WAIT} -pf11
		    hsend -t${RETAIN_WAIT} -pa2
		    if hexpect -t${RETAIN_WAIT} "TRANSACTION(S) COMPLETED"
		    then
		      print -u4 "${1} ${apar_list}"
		      return 0
		    else
		      if hexpect -t${RETAIN_WAIT} "TRANS COMPLETE:USE SEDIT TO ADD NEW COMP"
		      then
		        print -u4 "${1} ${apar_list}"
		        return 0
		      else
		        log_it "transaction not completed successfully"
		      fi
		    fi
		  else
		    log_it "applicable level screen not reached"
		  fi
		else
		  log_it "apars fixes screen not reached"
		fi
	      else
		log_it "environment screen not reached"
	      fi
	    else
	      log_it "applicable release screen not reached"
	    fi
	  else
	    log_it "copysent not an option"
	  fi
	else
	  log_it "trans is not an option"
	fi
      else
	log_it "Error displaying summary page for ${1}"
      fi
      hsend -t${RETAIN_WAIT} -pf1
      hsend -t${RETAIN_WAIT} -pf1
      return 1
    else
      # make sure the summary page of the ptf is displayed
      hsend -t${RETAIN_WAIT} -home "n;-/r ${1}"
      if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
      then
	log_it "Summary page for ${1}"
	if command_expect "TRANS"
	then
	  retain_release=$(hget -t0 1,58:1,60)
	  # issue trans command
	  hsend -t${RETAIN_WAIT} "trans"
	  if hexpect -t${RETAIN_WAIT} "CS-COPYSENT"
	  then
	    # issue copysent command
	    hsend -t${RETAIN_WAIT} "cs"
	    if hexpect -t${RETAIN_WAIT} "APPLICABLE RELEASE:"
	    then
	      # send release level
	      hsend -t${RETAIN_WAIT} "AR${retain_release}"
	      hsend -t${RETAIN_WAIT} -pf11
	      hsend -t${RETAIN_WAIT} -pa2
	      if hexpect -t${RETAIN_WAIT} "ENVIRONMENT:"
	      then
		# send environment string
		hsend -t${RETAIN_WAIT} "This PTF is for testing only"
		hsend -t${RETAIN_WAIT} -pf11
		hsend -t${RETAIN_WAIT} -pa2
		if hexpect -t${RETAIN_WAIT} "APARS FIXED:"
		then
		  # send list of apars that this ptf fixes
		  typeset -i i=2
		  typeset -i j=6
		  typeset -i k
		  typeset -i l
		  typeset -i m=0
		  while ((i<=${#}))
		  do
		    eval parameter_index='$'{${i}}
		    hsend -t${RETAIN_WAIT} -n ${parameter_index}
		    eval l=i/2-m
		    eval k=l%j
		    if ((k==0))
		    then
		      # hsend -t${RETAIN_WAIT} -tab
		      ((m=6))
		      ((j=8))
		    fi
		    ((i+=2))
		  done
		  hsend -t${RETAIN_WAIT} -pf11
		  hsend -t${RETAIN_WAIT} -pa2
		  if hexpect -t${RETAIN_WAIT} "APPLICABLE LEVEL:"
		  then
		    hsend -t${RETAIN_WAIT} -n ${retain_release}
		    hsend -t${RETAIN_WAIT} -pf11
		    hsend -t${RETAIN_WAIT} -pa2
		    if hexpect -t${RETAIN_WAIT} "TRANSACTION(S) COMPLETED"
		    then
		      print -u4 "${1} ${apar_list}"
		      return 0
		    else
		      if hexpect -t${RETAIN_WAIT} "TRANS COMPLETE:USE SEDIT TO ADD NEW COMP"
		      then
		        print -u4 "${1} ${apar_list}"
		        return 0
		      else
		        log_it "transaction not completed successfully"
		      fi
		    fi
		  else
		    log_it "applicable level screen not reached"
		  fi
		else
		  log_it "apars fixes screen not reached"
		fi
	      else
		log_it "environment screen not reached"
	      fi
	    else
	      log_it "applicable release screen not reached"
	    fi
	  else
	    log_it "copysent not an option"
	  fi
	else
	  log_it "trans is not an option"
	fi
      else
	log_it "Error displaying summary page for ${1}"
      fi
      hsend -t${RETAIN_WAIT} -pf1
      hsend -t${RETAIN_WAIT} -pf1
      return 1
    fi
  else
    # return a dummy indication that the ptf has been sent
    print -u4 "${1} ${apar_list}"
    return 0
  fi
}


#-----------------------------------------------------------------------
# FUNCTION: main
# DESCRIPTION: For each PTF in input list, if status indicates it is
# ready to send, send to distribution and update RETAIN abstract.
#-----------------------------------------------------------------------
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

  # if the release name was not supplied, use release suffix only for searches
  RELEASE_SUFFIX="${RELEASE_SUFFIX:=%325}"

  CMVC_FAMILY="${FAMILY:=aix}"

  type "$aix_version"_pkg_environment >/dev/null 2>&1
  [ $? -eq 0 ] && . "$aix_version"_pkg_environment
  if [ "x$ship_dir" = "x" ]; then
     ship_dir=$SHIP_DIRNAME
  fi

  typeset -i RETAIN_RETRY_WAIT
#SPM  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"
  RETAIN_RETRY_WAIT=$RETAIN_WAIT

  typeset -RZ6 ptf_num

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of the input file
    if [[ -n ${1} ]]
    then
      display_msg "Reading ptf list from file ${1}"
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
    display_msg "Reading ptf list from standard input"
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
  sent_file="${base_name}.sent"
  notsent_file="${base_name}.notsent"
  sendnote_file="${base_name}.sendnote"
  notsendnote_file="${base_name}.notsendnote"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    # save any existing versions of the output files
    if [[ -f ${sent_file} ]]
    then
      mv ${sent_file} ${sent_file}${time_suffix}
    fi
    if [[ -f ${notsent_file} ]]
    then
      mv ${notsent_file} ${notsent_file}${time_suffix}
    fi
  fi

  # if the -o flag was not specified
  if [[ ${to_stdout} = 1 ]]
  then
    # open ptf sent file as descriptor 4
    exec 4> ${sent_file}
  else
    # assign file 4 to stdout
    exec 4<&1
  fi

  # open not sent list output file as descriptor 5
  exec 5> ${notsent_file}

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

  # if test mode is not on
  if ((test_mode==1))
  then
    # read from the designated input source until no lines remain
    while read -u${input_data} ptf_string apar_list
    do
      # string any leading 'U' or 'u' from the incoming ptf string
      # before assigning it to the ptf number variable
      ptf_num=${ptf_string#[uU]}
      # remove any error messages from apar list due to previous run
      apar_list=${apar_list%\|*}
      if ptf_ready_to_send "U${ptf_num}" ${apar_list}
      then
        if [[ ${abstract_flag} = 0 ]]; then
           if send_ptf "U${ptf_num}" ${apar_list}
	   then
              :
	   else
	      display_msg "FIRST ATTEMPT TO SEND PTF U${ptf_num} FAILED"
	      # try once more (after a delay), then give up if it fails a second time
	      display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	      sleep ${RETAIN_RETRY_WAIT}
	      if send_ptf "U${ptf_num}" ${apar_list}
	      then
                 :
	      else
	         display_msg "Send of PTF failed"
	      fi
           fi
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
                 print -u5 $update_abstract_failure
              fi
           fi
        else
           ccss_unpack -c ${ship_dir}/U${ptf_num}.ptf -t /tmp/U${ptf_num}.toc.${pid}
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
                 print -u5 $update_abstract_failure
              fi
           fi
	fi
      else
	rc=${?}
	if [[ ${rc} = 7 ]]
	then
	  print -u4 "U${ptf_num} ${apar_list}"
	  display_msg "PTF already sent"
	else
	  if [[ ${rc} = 8 ]]
	  then
	    display_msg "PTF U${ptf_num} is not owned by Austin"
	  else
	    display_msg "PTF U${ptf_num} is not ready to send"
	  fi
	fi
        if [[ ${abstract_flag} = 1  && ${rc} != 8 ]]      
        then
           ccss_unpack -c ${ship_dir}/U${ptf_num}.ptf -t /tmp/U${ptf_num}.toc.${pid}
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
              fi
           fi
        fi
      fi
    done
  else
    # read from the designated input source until no lines remain
    while read -u${input_data} ptf_string apar_list
    do
      # string any leading 'U' or 'u' from the incoming ptf string
      # before assigning it to the ptf number variable
      ptf_num=${ptf_string#[uU]}
      # remove any error messages from apar list due to previous run
      apar_list=${apar_list%\|*}
      if ptf_ready_to_send "U${ptf_num}" ${apar_list}
      then
        if [[ ${abstract_flag} = 0 ]]; then
           if send_ptf "U${ptf_num}" ${apar_list}
	   then
              :
	   else
	      display_msg "FIRST ATTEMPT TO SEND PTF U${ptf_num} FAILED"
	      # try once more (after a delay), then give up if it fails a second time
	      display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	      sleep ${RETAIN_RETRY_WAIT}
	      if send_ptf "U${ptf_num}" ${apar_list}
	      then
                 :
	      else
	         display_msg "Send of PTF failed"
	      fi
           fi
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
              fi 
           fi
        else
           ccss_unpack -c ${ship_dir}/U${ptf_num}.ptf -t /tmp/U${ptf_num}.toc.${pid}
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
              fi
           fi
	fi
      else
	rc=${?}
	if [[ ${rc} = 7 ]]
	then
	  print -u4 "U${ptf_num} ${apar_list}"
	  display_msg "PTF already sent"
	else
	  display_msg "PTF is not ready to send"
	fi
        if [[ ${abstract_flag} = 1 ]]      
        then
           ccss_unpack -c ${ship_dir}/U${ptf_num}.ptf -t /tmp/U${ptf_num}.toc.${pid}
           if update_abstract "U${ptf_num}"
           then 
              :
           else
              display_msg "FIRST ATTEMPT TO UPDATE ABSTRACT U${ptf_num} FAILED"
              display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
              sleep ${RETAIN_RETRY_WAIT}
              if update_abstract "U${ptf_num}"
              then
                 :
              else
                 display_msg "UPDATE ABSTRACT failed"
              fi
           fi
        fi
      fi
    done
  fi

  # close output files
  display_msg "Closing all output files"
  exec 4>&-
  exec 5>&-

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    # save any existing versions of the output files
    if [[ -f ${sendnote_file} ]]
    then
      mv ${sendnote_file} ${sendnote_file}${time_suffix}
    fi
#SPM    if [[ -f ${notsendnote_file} ]]
#SPM    then
#SPM      mv ${notsendnote_file} ${notsendnote_file}${time_suffix}
#SPM    fi
  fi

  # Removed call to append_sent_note function here

  if [[ ${size_flag} = 1 && ${oem_flag} = 0 && ${abstract_flag} = 0 ]]
  then
     display_msg "Calling AddSizeNote to add size note to both RETAIN and CMVC"
     AddSizeNote ${logging_option} ${tracing_option} ${viewing_option} \
        ${files_option} ${status_option} ${test_option} ${wait_option} \
        ${path_option} ${retain_only_option} -V ${aix_version} \
	-F $CMVC_FAMILY -c ${sent_file}
  fi

  # Removed call to AddFilesNote here

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

  display_msg "SendPtf Program Interrupted"
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

pid=${$}

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
typeset -i retain_only=1
typeset -i size_flag=1
typeset -i oem_flag=0
typeset -i abstract_flag=0
debug_option=""
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
continuous_option=""
retain_only_option=""
size_option=""
oem_option=""
ML_INFO="" 
abstract_option=""
aix_version="32"
ship_dir=""

while getopts :cdhifSln:F:R:D:op:rstuvw:xOAM:V: next_option
do
  case ${next_option} in
  c) ((continuous_on=0))
     continuous_option="-c";;
  d) ((debug_mode=0))
     debug_option="-d";;
  h) display_help
     exit 1;;
  i) ((from_stdin=0));;
  +f) ((save_files=1))
      files_option="+f";;
  S) ((size_flag=0))
      size_option="-S";;
  O) ((oem_flag=1))
     oem_option="-O";;
  A) ((abstract_flag=1))
     abstract_option="-A";;
  +l) ((logging_on=1))
      logging_option="+l";;
  n) base_name=${OPTARG};;
  F) FAMILY=${OPTARG};;
  R) RELEASE_SUFFIX=${OPTARG};;
  M) ML_INFO=${OPTARG};;
  D) SHIP=${OPTARG};;
  o) ((to_stdout=0));;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       path_option="-p${OPTARG}"
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  r) ((retain_only=0))
     retain_only_option="-r";;
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
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]; then
        aix_version="41"
     fi;;
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

