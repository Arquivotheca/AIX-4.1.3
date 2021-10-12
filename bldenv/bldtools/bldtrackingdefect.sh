#
# @(#)13	1.11  src/bldenv/bldtools/bldtrackingdefect.sh, bldtools, bos412, GOLDA411a 8/30/94 16:15:29
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldtrackingdefect
#            bldtrackingdefect_cancel
#            bldtrackingdefect_checkfile
#            bldtrackingdefect_close
#            bldtrackingdefect_open
#            bldtrackingdefect_note
#            bldtrackingdefect_values
#	     bldtrackingdefect_verify
#	     bldtrackingdefect_cmvc_cmd
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Variables exported to the user.
#
typeset    BLDTRACKINGDEFECT_BLDCYCLE=""
					# Current build cycle the
					# BLDTRACKINGDEFECT* variables were
					# generated from.
typeset -x BLDTRACKINGDEFECT_BLDENV
typeset -x BLDTRACKINGDEFECT_COMPONENT
typeset -x BLDTRACKINGDEFECT_DEFECT
typeset -x BLDTRACKINGDEFECT_FAMILY
typeset -x BLDTRACKINGDEFECT_OWNER
typeset -x BLDTRACKINGDEFECT_NOPROMPT
typeset -x BLDTRACKINGDEFECT_SEVERITY
typeset -x BLDTRACKINGDEFECT_ABSTRACT
typeset -x BLDTRACKINGDEFECT_STATE	# null open returned working verify
					# canceled closed
typeset -x BLDTRACKINGDEFECT_TEST	# Use testing tracking defect instead
					# of real tracking defect.
typeset -r bldtrackingdefect_fieldseparator="|"
					# Separator in bldtrackingdefect.dat.

#
# NAME: bldtrackingdefect
#
# FUNCTION: Route the bldtracking defect request.
#
# INPUT: request ($1) - Type is either CANCEL, CHECK, CLOSE, NOTE, VERIFY,
#			or OPEN.
#        bldcycle ($2) - Build cycle to open tracking defect.
#        logerrors ($3) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#        other_options($4) - Rest of options passed.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect
{
   typeset -i RC=0
   typeset    bldcycle=$2
   typeset    command=$0
   typeset    logerrors=$3 
   typeset    request=$1
   shift 3
   typeset    other_options="$*"

   # Get tracking variables if needed.
   bldtrackingdefect_values ${bldcycle} "${logerrors}"
   [[ $? -ne 0 ]] && return 1

   case ${request} in
      CANCEL)
         bldtrackingdefect_cancel ${bldcycle} ${logerrors}
         RC=$?
         ;;
      CHECK)
         [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]] && RC=1
         ;;
      CLOSE)
         bldtrackingdefect_close ${bldcycle} ${logerrors}
         RC=$?
         ;;
      NOTE)
         bldtrackingdefect_note ${bldcycle} ${logerrors} ${other_options}
         RC=$?
         ;;
      OPEN)
         bldtrackingdefect_open ${bldcycle} ${logerrors}
         RC=$?
         ;;
      VERIFY)
         bldtrackingdefect_verify ${bldcycle} ${logerrors}
         RC=$?
         ;;
      *)
         if [[ "${logerrors}" = "${TRUE}" ]]
         then
            log +l -c ${command} -e "${request} is a invalid option"
         fi
         RC=1
         ;;
   esac

   return ${RC}
}

#
# NAME: bldtrackingdefect_cancel
#
# FUNCTION: Cancel the tracking defect for the build cycle.
#
# INPUT: bldcycle ($1) - Build cycle to get tracking defect for.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_cancel
{
   typeset    bldcycle=$1
   typeset    command=$0
   typeset    logerrors=$2
   typeset    logline

   # No tracking defect created yet.  Cannot cancel.
   [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]] && return 1

   case "${BLDTRACKINGDEFECT_STATE}" in 
   canceled ) : # we're already there
      ;;
   working  )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors return
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors cancel \
		"Build cycle closed: canceling defect." 
      RC=$?
      ;;
   open | returned )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors cancel \
		"Build cycle closed: canceling defect." 
      RC=$?
      ;;
   null | verify | closed )
      logline="Cannot cancel tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      logline="${logline} since it is in ${BLDTRACKINGDEFECT_STATE}."
      log +l -c ${command} -w "${logline}"
      RC=1
      ;;
   esac

   return ${RC}
}

#
# NAME: bldtrackingdefect_verify
#
# FUNCTION: Move the tracking defect for the build cycle to verify.
#
# INPUT: bldcycle ($1) - Build cycle to verify tracking defect.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_verify
{
   typeset    RC=0
   typeset    bldcycle=$1
   typeset    command=$0
   typeset    logerrors=$2
   typeset    logline

   # No tracking defect created yet.  Cannot verify.
   [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]] && return 1

   case "${BLDTRACKINGDEFECT_STATE}" in 
   canceled )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors reopen
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle verify: verifying defect." 
      RC=$?
      ;;
   open | returned )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle verify: verifying defect." 
      RC=$?
      ;;
   working | null )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle verify: verifying defect." 
      RC=$?
      ;;
   verify ) : # we're already in the right state.
      ;;
   closed )
      logline="Cannot move tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      log +l -c ${command} -w "${logline} to verify since it is closed."
      return 0
      ;;
   esac

   [[ $RC -eq 0 ]] && BLDTRACKINGDEFECT_STATE=verify
   return ${RC}
}

#
# NAME: bldtrackingdefect_close
#
# FUNCTION: Close the tracking defect for the build cycle.
#
# INPUT: bldcycle ($1) - Build cycle to close tracking defect.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_close
{
   typeset    RC=0
   typeset    bldcycle=$1
   typeset    command=$0
   typeset    logerrors=$2
   typeset    logline

   # No tracking defect created yet.  Cannot close.
   [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]] && return 1

   case "${BLDTRACKINGDEFECT_STATE}" in 
   canceled )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors reopen
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle close: accepting verify." 
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors close
      ;;
   open | returned )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle close: accepting verify." 
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors close
      RC=$?
      ;;
   working )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors verify \
		"Build cycle close: accepting verify." 
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors close
      RC=$?
      ;;
   verify | null )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors close
      RC=$?
      ;;
   closed ) : # we're already in the right state.
      ;;
   esac

   return ${RC}
}

#
# NAME: bldtrackingfile_checkfile
#
# FUNCTION: Check for the existance of the bldtrackingdefect data file.
#
# INPUT: logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if file exists, 1 if file does not exist.
#
function bldtrackingdefect_checkfile
{
   typeset    logerrors=$1
   typeset    command=$0

   get_filename_definitions

   if [[ ! -f "${BLDTRACKINGDEFECT_FILE}" ]]
   then
      if [[ "${logerrors}" = "${TRUE}" ]]
      then
         log +l -c ${command} -e "File ${BLDTRACKINGDEFECT_FILE} does not exist"
      fi
      return 1
   fi

   return 0
}

#
# NAME: bldtrackingdefect_note
#
# FUNCTION: Add remark to the tracking defect for the build cycle.
#
# INPUT: bldcycle ($1) - Build cycle whose tracking defect should have
#                        remark added.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#        remark ($3) - Either a filename containing remark or the remark
#                      itself.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_note
{
   typeset    become="${CMVC_BECOME-$BLDTRACKINGDEFECT_OWNER}"
   typeset    bldcycle=$1
   typeset    command=$0
   typeset    file="${FALSE}"
   typeset    line
   typeset    logerrors=$2
   typeset    logline
   typeset    remark

   # No tracking defect created yet.  Cannot add a remark.
   [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]] && return 1

   # Was remark provided as a file or a character string.
   if [[ "$3" = "-f" ]]
   then
      remark=$4
      file="${TRUE}"
   else
      shift 2
      remark="$*"
   fi

   if [[ "${file}" = "${TRUE}" ]]
   then
      Defect -note ${BLDTRACKINGDEFECT_DEFECT} \
             -family ${BLDTRACKINGDEFECT_FAMILY} \
             -become ${become} \
             -remarks - < ${remark}
   else
      Defect -note ${BLDTRACKINGDEFECT_DEFECT} \
             -family ${BLDTRACKINGDEFECT_FAMILY} \
             -become ${become} \
             -remarks "${remark}"
   fi 
   RC=$?
   if [[ ${RC} -ne 0 && "${logerrors}" = "${TRUE}" ]]
   then
      logline="Add note to tracking defect failed for"
      logline="${logline} ${BLDTRACKINGDEFECT_DEFECT}"
      log +l -c ${command} -e "${logline}"
      log +l -c ${command} -e "Note was:"
      if [[ "${file}" = "${TRUE}" ]]
      then
         for line in $(cat ${file})
         do
            log +l -c ${command} -e "${line}"
         done
      else
         log +l -c ${command} -e "${remark}"
      fi
   fi

   return ${RC}
}

#
# NAME: bldtrackingdefect_open
#
# FUNCTION: Open the tracking defect for the build cycle.
#
# INPUT: bldcycle ($1) - Build cycle to open tracking defect.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_open
{
   typeset    bldcycle=$1
   typeset    command=$0
   typeset    defectopen=$(bldtmppath)/bldtrackingdefect_open.$(hostname -s).$$
   typeset    line
   typeset    logerrors=$2
   typeset    logline
   typeset    phase
   typeset    remarks

   # No tracking defect created yet.  Cannot add a remark.
   if [[ -z "$BLDTRACKINGDEFECT_NOPROMPT" ]] 
   then
      confirm "Open tracking defect at this time? (y/n)" || return
   fi

   case "${BLDTRACKINGDEFECT_STATE}" in
   null ) :
      ;;
   canceled | returned )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors reopen
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work \
		"Build cycle reopened: accepting defect." 
      return $?
      ;;
   open     )
      bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work \
		"Build cycle reopened: accepting defect." 
      return $?
      ;;
   working  ) : # already in the right state.
      return 0
      ;;
   * )
      logline="Cannot reopen tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      log +l -c ${command} -w "${logline} it is in ${BLDTRACKINGDEFECT_STATE}."
      return 1
      ;;
   esac

   # Create a new defect.
   if [ "$BUILDER" ]
   then
     remarks="(note added by $BUILDER) - Opening defect for tracking build cycle, ${bldcycle},"
   else
     remarks="Opening defect for tracking build cycle, ${bldcycle},"
   fi
   remarks="${remarks} for $BLDTRACKINGDEFECT_BLDENV build environment."
   Defect -open \
          -phaseFound build \
          -component ${BLDTRACKINGDEFECT_COMPONENT} \
          -prefix p \
          -family ${BLDTRACKINGDEFECT_FAMILY} \
          -abstract "${BLDTRACKINGDEFECT_ABSTRACT}" \
          -become ${BLDTRACKINGDEFECT_OWNER} \
          -severity ${BLDTRACKINGDEFECT_SEVERITY} \
          -level ${BLDCYCLE}${BLDTRACKINGDEFECT_BLDENV} \
          -remarks "${remarks}" > ${defectopen}
   if [ $? -eq 0 ]
   then
      BLDTRACKINGDEFECT_DEFECT="\
         $(cat $defectopen | perl -n -e '/ (\d{2,6})\./ && print "$1\n";')"
      logline="Opened tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      logline="${logline} in family ${BLDTRACKINGDEFECT_FAMILY}"
      log -b -c ${command} "${logline}"
   else
      if [[ "${logerrors}" = "${TRUE}" ]]
      then
         log +l -c ${command} -e "Unable to open tracking defect."
      fi
      rm -f ${defectopen}
      return 1
   fi
   rm -f ${defectopen}

   phase=$(QueryStatus -A $_TYPE $T_BLDCYCLE $_BLDCYCLE ${bldcycle} \
           | cut -f10 -d\|)
   [[ -z "${phase}" ]] && phase="null"
   DeleteStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE}
   bldsetstatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} $_PHASE "${phase}" \
                  $_TRACKINGDEFECT "$BLDTRACKINGDEFECT_DEFECT"
   if [[ $? -ne 0 ]]
   then
      if [[ "${logerrors}" = "${TRUE}" ]]
      then
         logline="Could not update STATUS file for build cycle ${BLDCYCLE} and"
         log +l -c ${command} -e "${logline}"
         logline="tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
         log +l -c ${command} -e "${logline}"
      fi
   fi

   bldtrackingdefect_cmvc_cmd $bldcycle $logerrors assign
   if [[ $? -eq 0 ]]
   then 
      logline="Tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      logline="${logline} assigned to ${BLDTRACKINGDEFECT_OWNER}"
      log -f -c ${command} "${logline}"
   fi

   bldtrackingdefect_cmvc_cmd $bldcycle $logerrors work \
		"Starting build cycle: accepting defect to move to working."
   return $?
}

#
# NAME: bldtrackingdefect_values
#
# FUNCTION: User interface to the bldtrackingdefect.dat data base.  This
#           routine should be called to search the bldtrackingdefect.dat
#           data base for defect values to use when accessing the tracking
#           defect.
#
# INPUT: BLDTRACKINGDEFECT_FILE (global) - Name of the file to open to get
#                                          the default $BLDTRACKINGDEFECT_*
#                                          variables.
#        BLDTRACKINGDEFECT_TEST (global) - If set to $TRUE use dummy defect
#                                          number stored in the file
#                                          $BLDTRACKINGDEFECT_FILE.
#        bldcycle ($1) - Current build cycle to use.
#        logerrors ($2) - log any error messages if set to TRUE, value of
#                         ${TRUE} as defined in bldkshconst.
#
# OUTPUT: BLDTRACKINGDEFECT_BLDENV (global) - Name of the build environment
#                                             that the tracking defect is
#                                             being created for.
#         BLDTRACKINGDEFECT_COMPONENT (global) - Name of CMVC component that
#                                                all tracking defects will be
#                                                created against.
#         BLDTRACKINGDEFECT_FAMILY (global) - Name of CMVC family the tracking
#                                             defect should be opened in.  This
#                                             field is validated by calling
#                                             bldCMVCfamily function.
#         BLDTRACKINGDEFECT_OWNER (global) - Name of CMVC identifier that all
#                                            tracking defects created will be
#                                            assigned to.
#         BLDTRACKINGDEFECT_SEVERITY (global) - Severity to open tracking
#                                               defect at.
#         BLDTRACKINGDEFECT_ABSTRACT (global) - Abstract of defect.
#         BLDTRACKINGDEFECT_STATE (global)    - State of the defect.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if successful, 1 if error is encountered.
#
function bldtrackingdefect_values
{
   typeset    IFS			# Need to switch because of file
					# separator.
   typeset -i RC=0			# Return Code.
   typeset    bldcycle=$1		# Current build cycle.
   typeset    bldenv			# Build environment value from file.
   typeset    command=$0
   typeset    component			# Component value from file.
   typeset    dummy			# The fieldtype entry from file.
   typeset    family			# Family value from file.
   typeset    fieldtype="production"	# Entry to search for in file.
   typeset    line			# Line read from nodenames.dat.
   typeset -i line_count=0		# Number of lines read from
					# nodenames.dat.
   typeset    logerrors=$2		# Log error encountered.
   typeset    old_IFS			# Current IFS value.
   typeset    owner			# Owner value from file.
   typeset    prev_line			# Last valid line read from
					# nodenames.dat.
   typeset    prompt			# Prompt value from file.
   typeset    severity			# Severity value from file.

   bldtrackingdefect_checkfile ${logerrors}
   [[ $? -ne 0 ]] && return 1

   [[ "${bldcycle}" = "${BLDTRACKINGDEFECT_BLDCYCLE}" ]] && return 0

   [[ "${BLDTRACKINGDEFECT_TEST}" = "${TRUE}" ]] && fieldtype="test"

   # Read entries in file.  Ignore lines beginning with # and blank lines.
   grep "^${fieldtype}" ${BLDTRACKINGDEFECT_FILE} \
   | while read line
      do
         prev_line=${line}
         ((line_count=${line_count}+1))
      done

   # No entry found.
   if [[ ${line_count} -eq 0 ]]
   then
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c ${command} -e "No entry found in ${BLDTRACKINGDEFECT_FILE}"
      return 1
   fi

   # Multiple entries found.
   if [[ ${line_count} -ne 1 ]]
   then
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c ${command} \
             -e "Multiple entries found in ${BLDTRACKINGDEFECT_FILE}"
      return 1
   fi

   old_IFS="${IFS}"
   IFS="${bldtrackingdefect_fieldseparator}"
   print "${prev_line}" \
   | read dummy family owner component severity bldenv prompt dummy
   IFS="${old_IFS}"

   BLDTRACKINGDEFECT_FAMILY=$(bldCMVCfamily ${BLDTRACKINGDEFECT_FAMILY:-$family})
   [[ $? -ne 0 ]] && return 1
   BLDTRACKINGDEFECT_OWNER=${BLDTRACKINGDEFECT_OWNER:-${owner}}
   BLDTRACKINGDEFECT_COMPONENT=${BLDTRACKINGDEFECT_COMPONENT:-${component}}
   BLDTRACKINGDEFECT_SEVERITY=${BLDTRACKINGDEFECT_SEVERITY:-${severity:-2}}
   BLDTRACKINGDEFECT_BLDENV=${BLDTRACKINGDEFECT_BLDENV:-${bldenv}}
   # Null is a valid value for BLDTRACKINGDEFECT_NOPROMPT and should not
   # be overridden with a default value.
   BLDTRACKINGDEFECT_NOPROMPT=${BLDTRACKINGDEFECT_NOPROMPT-${prompt}}
   BLDTRACKINGDEFECT_ABSTRACT="Tracking defect for ${BLDTRACKINGDEFECT_BLDENV}"
   BLDTRACKINGDEFECT_ABSTRACT="$BLDTRACKINGDEFECT_ABSTRACT build cycle ${bldcycle}."
   BLDTRACKINGDEFECT_DEFECT=$(QueryStatus -A $_TYPE $T_BLDCYCLE \
                                          $_BLDCYCLE ${bldcycle} \
                              | cut -f7 -d\|)
   # See if cmvc thinks we've got one so we can repair the damaged status file.
   if [[ -z "${BLDTRACKINGDEFECT_DEFECT}" ]]
   then
      BLDTRACKINGDEFECT_DEFECT=$(
         Report -raw -view DefectView -family $BLDTRACKINGDEFECT_FAMILY \
	        -where "
	    compName = '$BLDTRACKINGDEFECT_COMPONENT' and
	    ownerLogin = '$BLDTRACKINGDEFECT_OWNER' and
	    abstract = '$BLDTRACKINGDEFECT_ABSTRACT'" \
         | cut -d '|' -f 2)
      if [[ -n "${BLDTRACKINGDEFECT_DEFECT}" ]]
      then
         typeset phase=$(QueryStatus -A $_TYPE $T_BLDCYCLE \
				     $_BLDCYCLE ${bldcycle} \
			 | cut -f10 -d\|)
         [[ -z "${phase}" ]] && phase="null"
	 DeleteStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE}
	 bldsetstatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} \
		      $_PHASE "${phase}" \
		      $_TRACKINGDEFECT "$BLDTRACKINGDEFECT_DEFECT"
      fi
   fi

   if [[ -n "${BLDTRACKINGDEFECT_DEFECT}" ]]
   then
      BLDTRACKINGDEFECT_STATE=$(
         Report -raw -view DefectView -family $BLDTRACKINGDEFECT_FAMILY \
	        -where "name = '$BLDTRACKINGDEFECT_DEFECT'" \
         | cut -d '|' -f 6)
   else
      BLDTRACKINGDEFECT_STATE=null
   fi

   if [[ -z "${BLDTRACKINGDEFECT_OWNER}" ]]
   then
      log +l -c ${command} -e "BLDTRACKINGDEFECT_OWNER has no value"
      RC=1
   fi
   if [[ -z "${BLDTRACKINGDEFECT_COMPONENT}" ]]
   then
      log +l -c ${command} -e "BLDTRACKINGDEFECT_COMPONENT has no value"
      RC=1
   fi
   if [[ -z "${BLDTRACKINGDEFECT_BLDENV}" ]]
   then
      log +l -c ${command} -e "BLDTRACKINGDEFECT_BLDENV has no value"
      RC=1
   fi

   [[ ${RC} -eq 0 ]] && BLDTRACKINGDEFECT_BLDCYCLE="${bldcycle}"

   return ${RC}
}

#
# NAME: bldtrackingdefect_cmvc_cmd
#
# FUNCTION: Run a cmvc command and report any errors.
#
# INPUT: bldcycle  ($1) - Build cycle to verify tracking defect.
#        logerrors ($2) - Log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#        action    ($3) - cancel, return, reopen, work, verify, close, assign
#	 remarks   ($4) - Any remarks to add to the defect.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if no errors occur and <>0 if error occurs.
#
function bldtrackingdefect_cmvc_cmd
{
   typeset    RC=0
   typeset    command=$0
   typeset    bldcycle=$1
   typeset    logerrors=$2
   typeset    action=$3
   typeset    remarks=$4
   typeset    logline
   typeset    cmvc_cmd
   typeset    new_state

   case $action in
   cancel ) cmvc_cmd="Defect -cancel"; new_state=canceled;;
   return ) cmvc_cmd="Defect -return"; new_state=returned;;
   reopen ) cmvc_cmd="Defect -reopen"; new_state=open;;
   work   ) cmvc_cmd="Defect -accept"; new_state=working;;
   verify ) cmvc_cmd="Defect -verify"; new_state=verify; remarks="";;
   close  ) cmvc_cmd="Verify -accept -defect"; new_state=closed;;
   assign ) cmvc_cmd="Defect -owner ${BLDTRACKINGDEFECT_OWNER} -assign"
	    new_state=${BLDTRACKINGDEFECT_STATE};;
   esac
   if [[ -n "${remarks}" ]]
   then
      ${cmvc_cmd} ${BLDTRACKINGDEFECT_DEFECT}  -remarks "${remarks}" \
         -family ${BLDTRACKINGDEFECT_FAMILY} -become ${BLDTRACKINGDEFECT_OWNER}
   else
      ${cmvc_cmd} ${BLDTRACKINGDEFECT_DEFECT} \
         -family ${BLDTRACKINGDEFECT_FAMILY} -become ${BLDTRACKINGDEFECT_OWNER}
   fi
   if [[ $? -ne 0 ]]
   then
      logline="Unable to $cmvc_cmd tracking defect ${BLDTRACKINGDEFECT_DEFECT}"
      log +l -c ${command} -e "${logline}"
      log +l -c ${command} -e "You need to run the following command:"
      log +l -c ${command} -e "${cmvc_cmd} ${BLDTRACKINGDEFECT_DEFECT}"
      log +l -c ${command} -e -- "-family ${BLDTRACKINGDEFECT_FAMILY}"
      log +l -c ${command} -e -- "-become ${BLDTRACKINGDEFECT_OWNER}"
      RC=1
   else
      BLDTRACKINGDEFECT_STATE=${new_state}
   fi
   return ${RC}
}
