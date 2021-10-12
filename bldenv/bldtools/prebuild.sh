#!/bin/ksh
# @(#) 83 1.115.1.27  src/bldenv/bldtools/prebuild.sh, bldprocess, bos412, GOLDA411a 6/28/94 11:36:13
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS:    prebuild                  Check_Buildable_State
#               Check_Cross_Release       Check_Level
#               Check_Levelname_Length    Check_Makefiles
#               Check_Phases              Check_Prereq
#               Check_Priorities          Check_Success
#               Clean_Up                  Combine_Level_Names
#               Commit_Level              Construct_PriorityString
#               Create_Production_Level   Create_Selfix_Data
#               Execute                   Extract_Level
#               FilterLevels              FilterReleases
#               GetLevelVersion           Is_Built
#               Merge_Source_AFS          Merge_Source_Production
#               Merge_Source_Build
#               Prebuild_Menu             Process_Production_Level
#               Restore_Selfix_Data       Set_Status
#               Undo_Work                 Usage
#               Verify_Write_Access
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: prebuild
#
# FUNCTION:  prebuild prepares new source, resulting from defects and features,
#            for production builds (libmake)
#
# INPUT: release names to build
#
# OUTPUT: none
#
# SIDE EFFECTS: Extracts the the new source (delta) from CMVC, Replaces the
#      delta source on the build machine, Creates a new delta source
#      on the CMVC machine, merges the delta source on to the CMVC
#      source server and commits the source.
#
#

alias clog='log -c "${log_command}"'

#
# NAME: Check_Buildable_State
#
# FUNCTION: Insure that the state of all tracks in the level are buildable.
#           Tracks in the integrate state are buildable.  Tracks in the
#           commit, complete, or test state have already been built and
#           can be accepted as buildable.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        MAINLOG (global) - filename of log file.
#        TMPFILE1 (global) - temporary file.
#        lvlname ($1) - level name.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if track has no errors, 1 if track has errors.
#
function Check_Buildable_State
{
   typeset -i RC=0		# Return code.
   typeset -i count		# Line count from Report command.
   typeset    log_command="Check_Buildable_State"
   typeset    logline		# Message to log command.
   typeset    lvlname=$1

   Report -view trackv \
          -where "releasename = '${CMVC_RELEASE}' and \
                  state not in ('integrate','commit','complete','test')  and \
                  defectname in (select defectname from levelmemberview \
                                 where releasename = '${CMVC_RELEASE}' and \
                                       levelname = '$lvlname')" > ${TMPFILE1}
   RC=$?
   count=$(cat ${TMPFILE1} | wc -l)
   if [[ ${RC} -ne 0 || ${count} -gt 5 ]]
   then
      if [[ ${RC} -eq 0 ]]
      then
         logline="Defects in ${CMVC_RELEASE} level ${lvlname} that are"
         logline="${logline} not buildable:"
      else
         logline="Bad return status from CMVC query for buildable defects in"
         logline="${logline} ${CMVC_RELEASE} level ${lvlname}:"
      fi
      clog +l -w "${logline}"
      cat ${TMPFILE1} | tee -a ${MAINLOG}
      RC=1
   else
      log "All defects in ${CMVC_RELEASE} level ${lvlname} are buildable"
   fi
   rm -f ${TMPFILE1}

   return ${RC}
}

#
# NAME: Check_Cross_Release
#
# FUNCTION: Check to see if any of the defects being built have tracks
#	    associated with them belonging to another release of the same
#	    version (e.g. bos320 and gos320).
#
# INPUT: CMVC_RELEASE (global) - release name.
#        lvlname - level name.
#
# OUTPUT: warning msg if cross release is found for a given defect
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if tracks found in other releases, 0 if no tracks found
#
function Check_Cross_Release
{
   typeset -i RC=0		# Return code.
   typeset    SAVE_BLDNODE=${HOSTSFILE_BLDNODE}
   typeset    SAVE_REL=${HOSTSFILE_REL}
   typeset    def
   typeset    log_command="Check_Cross_Release"
   typeset    logline		# Message to log command.
   typeset    lvlname=$1
   typeset -i rc		# Return code.
   typeset    rel
   typeset    state

   Report -view trackv \
	  -where "state not in ('commit','complete','test')  and \
                  defectname in (select defectname from levelmemberview \
                                 where releasename = '${CMVC_RELEASE}' and \
                                       levelname = '$lvlname')" -raw |
      awk -F"|" '{ print $1, $2, $4 }' |
      while read rel def state
      do
         # execute as subshell so that call to bldhostsfile does
         # not change the values of the HOSTSFILE_* variables in
         # the current environment
         rc=$(innerRC=0
              bldhostsfile ${rel} ${FALSE}
              if [ $? -eq 0 ]
              then
                 if [[ "${HOSTSFILE_BLDNODE}" = "${SAVE_BLDNODE}" &&
                       "${HOSTSFILE_REL}" != "${SAVE_REL}" ]]
                 then
                     innerRC=1
	             clog -w "Defect #${def} has a track open for release ${rel}"
	             clog -w "in the ${state} state as well as ${CMVC_RELEASE}."
                 fi
              fi
              print ${innerRC} )
          (( RC=${RC}|${rc} ))
      done

   return ${RC}
}

#
# NAME: Check_Level
#
# FUNCTION: Check the level to insure all tracks are in buildable state and
#           all tracks have the correct priority.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        LMFIX (global) - if set check priority of the tracks.
#        lvlname ($1) - level name.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if track has errors, 0 if no errors.
#
function Check_Level
{
   typeset -i RC=0		# Return code.
   typeset    log_command="Check_Level"
   typeset    logline		# Message to log command.
   typeset    lvlname=$1

   if [ -z "${LMFIX}" ]
   then
      logline="Checking priorities of ${CMVC_RELEASE} level ${lvlname}"
      logline="${logline} defects and features"
      log "${logline}"
      Check_Priorities defects ${lvlname}
      (( RC=${RC}|$? ))
      Check_Priorities features ${lvlname}
      (( RC=${RC}|$? ))
   fi
   log "Checking ${CMVC_RELEASE} level ${lvlname} for buildable state"
   Check_Buildable_State ${lvlname}
   (( RC=${RC}|$? ))
   Check_Cross_Release ${lvlname}
   (( RC=${RC}|$? ))

   return ${RC}
}

#
# NAME: Check_Levelname_Length
#
# FUNCTION: Insure that a levelname is not greater than the length
#           CMVC allows a levelname to be.
#
# INPUT: MAX_LEVELNAME_LENGTH (global) - Maximum number of characters a
#                                        levelname can be.
#        levelname ($1) - name of level to check.
#
# OUTPUT: Levelname to use output to stdout.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if user chose not to rename a level name that was too long,
#          0 otherwise.
#
function Check_Levelname_Length
{
   typeset -i RC=0
   typeset    levelname=$1
   typeset    log_command="Check_Levelname_Length"
   typeset    logline		# Message to log command.
   typeset    newlevelname=${levelname}
				# Levelname user entered if levelname
				# passed in is too long.
   typeset -u response=""	# Response from prompt to change level
				# name.

   while [[ ${#newlevelname} -gt ${MAX_LEVELNAME_LENGTH} ]]
   do
      clog +l -e "${newlevelname} is longer than the ${MAX_LEVELNAME_LENGTH}"
      clog +l -e "character CMVC levelname limit"

      confirm -r "\tDo you want to choose a new levelname? (y/n/q) \c"
      if [[ $? -ne 0 ]]
      then
         log "Level ${levelname} not renamed"
         print ""
         return 1
      fi

      RC=1
      while [[ ${RC} -ne 0 ]]
      do
         print "\tEnter new levelname: \c" > /dev/tty
         read newlevelname < /dev/tty
         confirm -r "\tlevelname = ${newlevelname}\n\tIs this correct? (y/n/q): \c"
         RC=$?
         if [[ ${RC} -eq 2 ]]
         then
            print ""
            return 1
         fi
      done
   done

   [[ ${levelname} != ${newlevelname} ]] && \
      log "Level ${levelname} renamed to ${newlevelname}"
   print ${newlevelname}
   return 0
}

#
# NAME: Check_Makefiles
#
# FUNCTION: Search the delta directory for Makefiles.  Reports the
#           differences between the new file and the old file.  To be
#           used as an indicator whether or not to completely rebuild
#           the Makefile's directory.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        EXTRACTHOST (global) - name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        HOSTSFILE_PROD (global) - Location of the product tree.
#        levelversion (global) - name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if error occurs and 0 if no error occurs
#
function Check_Makefiles
{
   typeset    CKMAKE=${LOG}/ckmake.${CMVC_RELEASE}.${levelversion}
   typeset -i RC=0
   typeset    log_command="Check_Makefiles"
   typeset -u viewit		# Prompt to view the Makefile differences.

   Check_Success ${S_DELTAEXTRACT}
   [[ $? -ne 0 ]] && \
   {
      clog +l -w "Makefiles could not be checked because the delta"
      clog +l -w "source extract has not completed successfully"
      return 0
   }

   log "Checking the delta tree for Makefiles"

   check_directory ${EXTRACTHOST_COLON}${HOSTSFILE_DELTA}/${levelversion}
   if [[ $? -ne 0 ]]
   then
      log +l -e "Directory ${HOSTSFILE_DELTA}/${levelversion}"
      log +l -e "does not exist on ${EXTRACTHOST}"
      return 1
   fi

   #
   # The ${CKMAKE} file is in the LOG directory which allows access from
   # current host or extract host.
   #

   command="cd ${HOSTSFILE_DELTA}/${levelversion};"
   command="${command} find . -type f -name [mM]ake\* -print > $CKMAKE"
   Execute ${RETRY} ${command}
   if [[ $? -ne 0 ]]
   then
      log +l -e "Search for Makefiles failed"
      return 1
   fi

   if [[ ! -s $CKMAKE ]] 
   then
      log "No Makefiles found for ${CMVC_RELEASE} level ${levelversion}"
      rm $CKMAKE 2>/dev/null
      return 0
   fi

   log "Makefiles found. Output in $CKMAKE"

   for i in $(cat $CKMAKE)
   do
      Execute ${NORETRY} \
         "print ======== ${CMVC_RELEASE} ${levelversion} $i >> ${CKMAKE}"
      Execute ${NORETRY} \
         "diff ${HOSTSFILE_DELTA}/${levelversion}/$i ${HOSTSFILE_PROD}/$i \
         1>> ${CKMAKE} 2>&1"
   done

   confirm -n -r "\tDo you want to view $CKMAKE now? (y/N/q) \c"
   RC=$?
   [[ ${RC} -eq 2 ]] && return 1
   [[ ${RC} -eq 0 ]] && pg $CKMAKE

   return 0
}

#
# NAME: Check_Phases
#
# FUNCTION: Insure the build cycle is in the correct phase.
#
# INPUT: BLDCYCLE (global) - current build cycle.
#
# OUTPUT: none.
#
# SIDE EFFECTS: Will exit if BLDCYCLE not in correct phase.
#
# RETURNS: 0 always.
#
function Check_Phases {
   typeset    log_command="Check_Phases"

   log "Checking phases"
   CheckPhases test
   if [[ $? -eq 1 ]]
   then
      if CheckPhases hold
      then
         clog +l -x "$BLDCYCLE is in the hold phase"
      else
         if CheckPhases open prebuild build
         then
            if CheckPhases open
            then
               bldstatus -e $BLDCYCLE
            fi
         else
            clog +l -x "$BLDCYCLE is in the wrong phase"
         fi
      fi
   fi

   return 0
}

#
# NAME: Check_Prereq
#
# FUNCTION: Checks level to insure there are no prereqs.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        MAINLOG (global) - filename of log file.
#        levelversion (global) - name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if error occurs and 0 if no error occurs.
#
function Check_Prereq
{
   typeset    log_command="Check_Prereq"
   typeset    logline		# Message to log command.
   typeset -u response=""	# Response from prompt to continue with
				# prereq problems.

   if [[ ""$PREBUILD_RUN_LEVELCHECK = ${NO} ]]
   then
      log "Skipping check of ${CMVC_RELEASE} level ${levelversion} for prereqs"
      return 0
   fi

   log "Checking ${CMVC_RELEASE} level ${levelversion} for prereqs"

   Level -check ${levelversion} -release ${CMVC_RELEASE} ${LEVELCHECK_FLAGS} > \
      ${BLDCHK}
   if [[ $? -ne 0 ]]
   then
      logline="CMVC Error: Level check of ${CMVC_RELEASE} level ${levelversion}"
      logline="${logline} for prereqs"
      clog +l -e "${logline}"
      clog +l "Please check logs"
      return 1
   fi

   if [[ -s ${BLDCHK} ]]
   then
      clog +l -e "${CMVC_RELEASE} level ${levelversion} has a prereq problem"
      cat ${BLDCHK} | tee -a ${MAINLOG}
      confirm -r "\tContinue with ${CMVC_RELEASE} level ${levelversion} (y/n/q) \c"
      if [[ $? -ne 0 ]]
      then
         logline="Stopping with ${CMVC_RELEASE} level ${levelversion} because"
         logline="${logline} of prereq problems"
         log "${logline}"
         return 1
      fi
      logline="Continuing with ${CMVC_RELEASE} level ${levelversion}"
      logline="${logline} with prereq problems"
      log "${logline}"
   fi

   return 0
}

#
# NAME: Check_Priorities
#
# FUNCTION: Determine if all members in a level have the correct priorities.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        TMPFILE1 (global) - temporary file.
#        lvlname - level name.
#        type - type of members; either defect or feature.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 2 for invalid parameters, 1 for invalid priorities, and 0 if
#          no errors.
#
function Check_Priorities
{
   typeset -i RC=0		# Return code.
   typeset -i count=0
   typeset    log_command="Check_Priorities"
   typeset    logline		# Message to log command.
   typeset    lvlname=$2
   typeset    priority=""	# Priorities to check, if null no checks
				# need to be done.
   typeset    type=$1
   typeset    view

   [[ "${type}" = "defects" ]] && view="defectv"
   [[ "${type}" = "features" ]] && view="featurev"
   [[ -z "${view}" ]] && \
   {
      clog +l -e "Invalid type: \"${type}\" passed to Check_Priorities"
      return 1
   }

   priority="$(Construct_PriorityString)"
   if [[ -n "${priority}" ]]
   then
      Report -view ${view} \
                -where "${priority} and \
                      name in (select defectname from levelmemberview \
                               where releasename = '${CMVC_RELEASE}' and \
                                     levelname = '${lvlname}')" > ${TMPFILE1}
      RC=$?
      count=$(cat ${TMPFILE1} | wc -l)
   else
      log "Release ${CMVC_RELEASE} has no priority restrictions"
   fi
   if [[ ${RC} -ne 0 || ${count} -gt 5 ]]
   then
      if [[ ${RC} -eq 0 ]]
      then
         logline="The ${type} in ${CMVC_RELEASE} level ${lvlname} that do not"
         logline="${logline} have correct priorities:"
         clog +l -w "${logline}"
         cat ${TMPFILE1} | tee -a ${MAINLOG}
      else
         logline="Bad return status from CMVC query for ${type} with"
         logline="${logline} correct priority in ${CMVC_RELEASE} level ${lvlname}:"
         clog +l -e "${logline}"
      fi
      RC=1
   else
      logline="All ${type} in ${CMVC_RELEASE} level ${lvlname} have correct"
      logline="${logline} priorities"
      log "${logline}"
   fi
   rm -f ${TMPFILE1}

   return ${RC}
}

#
# NAME: Check_Success
#
# FUNCTION: Calls CheckStatus to insure subtype had completed correctly.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        levelversion (global) - name of the level.
#        subtype - subtype argument to pass to commands called.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: Value returned from CheckStatus.
#
function Check_Success
{
   typeset    subtype=$1

   CheckStatus $_TYPE $T_PREBUILD \
               $_SUBTYPE ${subtype} \
               $_BLDCYCLE $BLDCYCLE \
               $_RELEASE ${CMVC_RELEASE} \
               $_LEVEL ${levelversion} \
               $_STATUS ${ST_SUCCESS}
   return $?
}

#
# NAME: Clean_Up
#
# FUNCTION: Clean up temporary files.
#
# INPUT: Various global files that should be removed.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function Clean_Up
{
   log "Cleaning up"

   Restore_Selfix_Data

   rm -f $CATALOGS $LEVELVIEW $READYLEVELS $LEVELS_TODO $TD_COPY \
         $MUSTFIX $INTEGRATE $MFLMEMBERS $TRACKS $TEMP $ERRLOG \
         $DEVLEVELMEMBERS ${BLDCHK} \
         $LEVEL_TEST_LIST ${TMPFILE1} ${TMPFILE2}

   for EXTRACTHOST in ${cleanup_hosts}
   do
      Execute ${NORETRY} rm -fr ${PRODUCTION_BACKUP} \
                     ${DELTA_FILE_LIST} ${DELTA_FILE_LIST_IN_SRC}
   done

   logset -r

   return 0
}

#
# NAME: Combine_Level_Names
#
# FUNCTION: Take a list of release names and level names and combine
#           all development level names for identical releases on one
#           line.  Order is maintained.
#           Example:
#             Input-   levelname0 bos312 production
#                levelname1 bos312 development
#                levelname2 hcon320 development
#                levelname3 bos312 development
#                levelname4 bos312 development
#             Output- bos312 production levelname0
#                bos312 development levelname1 levelname3 levelname4
#                hcon320 development levelname2
#
# INPUT: BLDTMP (global) - temporary directory.
#        list - file of levels and releases.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS:
#
function Combine_Level_Names
{
   typeset    copy=$BLDTMP/lrcopy$$
   typeset    list=$1
   typeset    lname
   typeset    lrlist=$BLDTMP/lrtemp$$
   typeset    lvls=$BLDTMP/lvls$$
   typeset    names=""
   typeset    rname
   typeset    temp=$BLDTMP/lrtemp$$
   typeset    type

   cp $list $lrlist   # make working copy
   > $list   # erase the orginal list, new values will be stored here
   while [[ -s $lrlist ]]   # while there are more entries to process
   do
      read lname rname type < $lrlist   # get first release name
      [[ -z "$type" ]] && type=notype  
      if [[ "$type" = "development" ]]
      then
          # get all other development level entries for the release  
          awk ' BEGIN { release = ARGV[2]; ARGV[2] = "" }
                $2 == release && $3 == "development" { print }
                  ' $lrlist "$rname" > $copy

          # Delete release names and level types and assign the level
                    # names on one line to the variable names

                    cat $copy | cut -d" " -f1 > $lvls
                    names=$(cat $lvls)
          # remove all the release's development levels from input
          awk ' BEGIN { rel = ARGV[2]; ARGV[2] = "" }
                !($2 == rel && $3 == "development") { print }
                  ' $lrlist "$rname" > $copy; mv $copy $lrlist
      else
          names=$lname
          tail +2 $lrlist > $copy;  mv $copy $lrlist   # delete line 1
      fi
      print $rname $type $names >> $list   # save the result
   done
   rm -f $lrlist $copy $temp $lvls
}

#
# NAME: Commit_Level
#
# FUNCTION: Commit the level
#
# INPUT: CMVC_RELEASE (global) - release name.
#        LCOMMIT (global) - if YES commit the level with out prompting.
#        levelversion (global) - name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may commit the build level.
#
# RETURNS: 0 if commit is successful
#          1 if commit fails
#          2 if commit was not attempted due to user request
#
function Commit_Level
{
   typeset -i RC=0
   typeset    log_command="Commit_Level"
   typeset    logline		# Message to log command.

   # Once a level is committed there is no easy way to get a .gone
   # file.  Force a delta extract before allowing the level to be
   # committed.
   Check_Success ${S_DELTAEXTRACT}
   [[ $? -ne 0 ]] && \
   {
      clog +l -e "Cannot commit ${CMVC_RELEASE} level ${levelversion} until"
      clog +l -e "level extract has occurred"
      return 1
   }

   if [[ "${LCOMMIT}" = ${NO} ]] 
   then
      RC=2
   elif [[ "${LCOMMIT}" != ${YES} ]]
   then
      confirm -n -r "\tCommit ${CMVC_RELEASE} level ${levelversion}? (y/N) \c"
      # confirm returns 0 for 'y', 1 for 'n', 2 for 'q'
      case $? in
	  0) RC=0;;
	  1) RC=2;;  # indicate level not committed
	  2) RC=1;;  # prompt for quit when you get back
      esac
   fi
   if [[ ${RC} -ne 0 ]]
   then
      log "The ${CMVC_RELEASE} level ${levelversion} will NOT be commited"
      return ${RC}
   fi

   log "Committing ${CMVC_RELEASE} level ${levelversion}"
   Level -commit ${levelversion} -release ${CMVC_RELEASE}
   if [[ $? -ne 0 ]]
   then
      Set_Status ${S_CMVCCOMMIT} ${ST_FAILURE}
      if [[ $? -ne 0 ]]
      then
         logline="Set_Status error after unsuccessful level commit of"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      else
         logline="CMVC Error: unsuccessful level commit of"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      fi
      clog +l -e "${logline}"
      clog +l "Please check logs"
      return 1
   fi
   Set_Status ${S_CMVCCOMMIT} ${ST_SUCCESS}
   [[ $? -ne 0 ]] && \
   {
      logline="Set_Status error after successful level commit of"
      logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      clog +l -e "${logline}"
      # Don't return 1 here.  Commit has occurred so work can nolonger
      # be undone.
   }

   return 0
}

#
# NAME: Construct_PriorityString
#
# FUNCTION: Construct the sql statement for priority checking.  The
#           priorities come from the nodename file.
#
# INPUT: HOSTSFILE_BLDNODE (global) - node value from hostsfile.dat for
#                                  current release.
#
# OUTPUT: The sql statement to check priorities displayed to screen.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always
#
function Construct_PriorityString
{
   typeset    not_first=""      # If true, need to place a comma
				# before adding next priority.
   typeset    priority		# Priority value being added.
   typeset    priorities	# Priorities to be added.
   typeset    priority_str	# Sql statement generated to check
				# priorities

   bldnodenames ${HOSTSFILE_BLDNODE} ${TRUE}

   # No special priority values to add onto sql statement
   [[ -z "${NODENAMES_PRIORITIES}" ]] && \
   {
      print ""
      return 0
   }

   # Add special priority values onto sql statement
   priority_str="(priority is NULL or priority not in ("
   for priority in ${NODENAMES_PRIORITIES}
   do
      [[ -n "${not_first}" ]] && priority_str="${priority_str},"
      not_first="true"
      priority_str="${priority_str}'${priority}'"
   done

   print "${priority_str}))"

   return 0
}

#
# NAME: Create_Production_Level
#
# FUNCTION: Take all development levels in a release and combine them into
#           one production level.
#
# INPUT: BLDOWNER (global) - CMVC id the level is assigned to.
#        CMVC_BECOME (global) - CMVC id running as.
#        CMVC_RELEASE (global) - Release name.
#        LCONT (global) - If set, continue with build even when track errors
#                         are seen.
#        LEVELNAME (global) - The level name to use.
#        LTYPE (global) - Type of build (Production or Development).
#        MAINLOG (global) - filename of log file.
#        MFLMEMBERS (global) - List of defects that will be included in the
#                              level created.
#        TEMP (global) - Temp file name.
#
# OUTPUT:
#         levelversion (global) - Name of the level.
#
# SIDE EFFECTS: may commit the build level.
#
# RETURNS: 1 if error occurs creating level and 0 if no errors occur.
#
function Create_Production_Level
{
   typeset -i RC=0		# Return Code.
   typeset -u ans		# Response to continue when build errors
				# occur creating new level.
   typeset    devlevelname	# Individual level name from processing
				# levelnames.
   typeset    levelnames=$1	# Names of levels to include in production
				# level created.
   typeset    log_command="Create_Production_Level"
   typeset    logline		# Message to log command.
   typeset -u response		# Response to continue when track errors are
				# found.

   levelversion="${LEVELNAME}$(GetLevelVersion ${LEVELNAME})"

   logset -c prebuild -C prebuild -F ${MAINLOG} -1 ${levelversion} -2 ${CMVC_RELEASE}

   levelversion="$(Check_Levelname_Length ${levelversion})"
   [[ $? -ne 0 ]] && return 1

   log "Getting all defects for ${CMVC_RELEASE} track validation"

   > $MFLMEMBERS   # erase list of tracks (just in case)
   for devlevelname in $levelnames 
   do  
      log "Getting all ${CMVC_RELEASE} development level $devlevelname tracks"

      Check_Level ${devlevelname}
      (( RC = ${RC} | $? ))

      Report -view trackview \
             -where "releasename = '${CMVC_RELEASE}' and \
                     state not in ('commit','complete','test') and \
                     defectname in (select defectname \
                                    from levelmemberview \
                                    where releasename = '${CMVC_RELEASE}' and \
                                          levelname = '${devlevelname}')" \
             -raw | cut -d\| -f2 >> ${MFLMEMBERS}
      if [[ $? -ne 0 ]]
      then
         logline="Report -view trackview -where \"releasename = "
         logline="${logline} \'${CMVC_RELEASE}\' and state not in \(\'commit\',"
         logline="${logline} \'complete\',\'test\'\) and defectname in"
         logline="${logline} \(select defectname from levelmemberview where"
         logline="${logline} releasename = \'${CMVC_RELEASE}\' and levelname ="
         logline="${logline} \'${devlevelname}\'\)\""
         clog +l -e "CMVC Error: $logline"
         return 1
      fi
   done

   if [[ ${RC} -ne 0 ]] # track was not in correct state
   then
      [[ "${LCONT}" = $NO ]] && return 0
      if [[ "${LCONT}" != $YES ]] # does env variable say continue
      then
         confirm -r "\tContinue with ${CMVC_RELEASE} level ${levelversion}? (y/n/q) \c"
         RC=$?
         if [[ ${RC} -ne 0 ]]
         then
            logline="Stopping with ${CMVC_RELEASE} level ${levelversion} because"
            logline="${logline} of track errors"
            log "${logline}"
            return 1
         fi
         logline="Continuing with ${CMVC_RELEASE} level ${levelversion}"
         logline="${logline} with track errors"
         log "${logline}"
      fi
   fi

   log "Removing duplicate tracks from ${CMVC_RELEASE} development levels"
   sort -u $MFLMEMBERS > $TEMP  # rm dups
   mv $TEMP $MFLMEMBERS
   if [[ ! -s $MFLMEMBERS ]]
   then
      log +l -e "No ready tracks in ${CMVC_RELEASE}"
      return 1
   fi

   log "Creating ${CMVC_RELEASE} $LTYPE level $levelversion"
   Level -create $levelversion -release ${CMVC_RELEASE} -type $LTYPE
   if [[ $? -ne 0 ]]
   then
      clog +l -e "CMVC Error: Creating ${CMVC_RELEASE} ${LTYPE} level ${levelversion}"
      clog +l "Please check logs"
      return 1
   fi
   log "Creating ${CMVC_RELEASE} $LTYPE level ${levelversion} members"
   LevelMember -create -level $levelversion -release ${CMVC_RELEASE} -defect - \
      < $MFLMEMBERS
   if [[ $? -ne 0 ]]
   then
      clog +l -e "CMVC Error: Creating ${CMVC_RELEASE} $LTYPE level members"
      clog +l "Please check logs"
      PROMPT="\n\tThe above errors were detected."
      PROMPT="${PROMPT} Do you wish to continue? (Y/n/q) \c"
      confirm -y -r "${PROMPT}"
      if [[ $? -ne 0 ]]
      then
         return 1
      else
         log "Proceeding with $levelversion"
      fi
   fi
   Undo_Level="${levelversion}"

   logline="Number of level members included"
   logline="${logline} in ${LTYPE} level: $(wc -l <$MFLMEMBERS)"
   log "${logline}"

   # CMVC does not like builds being assigned to the same owner.
   if [[ "${BLDOWNER}" != "${CMVC_BECOME}" ]]
   then
      log "Assigning ${CMVC_RELEASE} level $levelversion to ${BLDOWNER}"
      Level -assign $levelversion -to ${BLDOWNER} -release ${CMVC_RELEASE}
      if [[ $? -ne 0 ]]
      then
         logline="CMVC Error: Assigning ${CMVC_RELEASE} level $levelversion to"
         logline="${logline} ${BLDOWNER}"     
         clog +l -e "$logline"
         clog +l "Please check logs"
         return 1
      fi
   else
      log "${CMVC_RELEASE} level ${levelversion} already owned by ${BLDOWNER}"
   fi

   # change type on original development levels to get them out
   # of the way.  Also record which defects came from which levels
   # in the admin tracking defect.  Don't stop prebuild if you
   # can't attach the note though.
   print   "START_DEVELOPMENT_LEVEL_INFO" >${TEMP}
   print   "levelName       releaseName     pref name" >>${TEMP}
   print - "--------------- --------------- ---- ------" >>${TEMP}
   for devlevelname in $levelnames
   do
      Report -raw -view LevelMemberView -where \
	 "levelName in ('$devlevelname') and releasename='${CMVC_RELEASE}'" \
         | awk -F '|' '{printf("%-15s %-15s %-4s %6s\n",$1,$2,$8,$3)}' \
         >>${TEMP}
      Level -modify $devlevelname -type other -release ${CMVC_RELEASE}
      if [[ $? -ne 0 ]]
      then
         logline="CMVC Error: Level modify $devlevelname to"
         logline="${logline} type other"     
         clog +l -e "$logline"
         clog +l "Please check logs"
         return 1
      fi
      Undo_TypeOther_Levels="${Undo_TypeOther_Levels} ${devlevelname}"
      log "${CMVC_RELEASE} level $devlevelname changed to type 'other'"
   done  
   print "STOP_DEVELOPMENT_LEVEL_INFO" >>${TEMP}
   bldtrackingdefect NOTE ${BLDCYCLE} ${FALSE} -f ${TEMP}
   rm -f ${TEMP}
}

#
# NAME: Create_Selfix_Data
#
# FUNCTION: Create files for the selective fix process.  Files include the
#           abstract list, defect list, changeview, rename list, symptoms
#           file and subsystem files.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        DELETE (global) - value for modified[], indicates the file should not
#                          exist if level commit fails.
#        MAINLOG (global) - filename of log file.
#        NOTMODIFIED (global) - value for modified[], indicates the file has
#                               not been modified while processing current
#                               level.
#        RESTORE (global) - value for modified[], indicates the file does exist
#                           and will need to be restored if level commit fails.
#        TMPFILE1 (global) - temporary file 1.
#        TMPFILE2 (global) - temporary file 2.
#        abstractlist (global) - Index of abstractlist file.
#        all_defects (global) - Index of all_defects file.
#        changeview (global) - Index of changeview file.
#        defects (global) - Index of defects file.
#        filename[] (global) - Array of filenames that may be modified by
#                              prebuild.
#        levelversion (global) - level name.
#        memos (global) - Index of memos file.
#        modified[] (global) - Array indicating if a file has been modified.
#        ptfrequisites (global) - Index of ptfrequisites file.
#        renamelist (global) - Index of renamelist file.
#        ssXREF (global) - Index of ssXREF file.
#        symptoms (global) - Index of symptoms file.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if one or more files could not be created; 0 otherwise.
#
function Create_Selfix_Data
{
   typeset    LEVELVERSION	# Needed for exporting to CheckSymptom.
   typeset -i VALUE_LIST_SIZE=100
				# Grouping size of defects passed to CMVC.
   typeset -i RC=0		# Return code.
   typeset    defect		# The defect number the changeview data is
				# being generated on.
   typeset -i defect_count=0	# Number of defects in $defectlist.
   typeset    defectlist=""	# List of defects being built to pass to a
				# CMVC command.
   typeset -i index=0		# Index into the filename and modified arrays.
   typeset    line		# A line from the changeview file.
   typeset    log_command="Create_Selfix_Data"
   typeset    name		# Current file file name.
   typeset    oldname		# The files old name.
   typeset    rel_dir="$(bldreleasepath ${CMVC_RELEASE})"
				# Directory the defects file is in.

   # Don't create any selfix data if we're not doing an update build.
   [[ ${PTF_UPDATE} != ${YES} ]] && return 0

   filename[${changeview}]="${rel_dir}/changeview"
   filename[${defects}]="${rel_dir}/defects"
   filename[${renamelist}]="${rel_dir}/renamelist"
   filename[${ssXREF}]="${rel_dir}/ssXREF"

   ABSTRACTLIST=${filename[${abstractlist}]}
   ALL_DEFECTS=${filename[${all_defects}]}
   CHANGEVIEW=${filename[${changeview}]}
   DEFECTS=${filename[${defects}]}
   RENAMELIST=${filename[${renamelist}]}

   # Backup up selective fix data files.
   while [[ ${index} -lt ${#filename[*]} ]]
   do
      if [[ -f ${filename[${index}]} ]]
      then
         cp ${filename[${index}]} ${filename[${index}]}_backup.$$
         if [[ $? -ne 0 ]]
         then
            RC=1
            clog +l -e "Cannot create ${filename[${index}]}_backup.$$"
         fi
      else
         # File does not exist, need to insure it gets removed if commit does
         # not occur.
         modified[${index}]="${DELETE}"
      fi
      (( index=${index}+1 ))
   done
   [[ ${RC} -ne 0 ]] && return 1

   # Add to defect list file.
   if [[ ! -w "${DEFECTS}" ]]
   then
      clog +l -e "File ${DEFECTS} not writable"
      return 1
   else
      log "Creating defect lists file ${DEFECTS}"
      Report -view levelmemberview \
             -where "levelname='${levelversion}' and releasename='${CMVC_RELEASE}'" \
             -raw \
      | cut -d'|' -f3 > ${TMPFILE1}
      if [[ $? -ne 0 ]]
      then
         clog +l -e "CMVC Error: Creating ${DEFECTS}"
         return 1
      else
         modified[${defects}]="${RESTORE}"		# File always exists.
         cat ${TMPFILE1} >> ${DEFECTS}
      fi
   fi

   modified[${ptfrequisites}]="${RESTORE}"		# File always exists.
   bldreqlist ${CMVC_RELEASE} ${DEFECTS}
   [[ $? -ne 0 ]] && return 1

   log "Creating changeview file ${filename[${changeview}]}"
   modified[${abstractlist}]="${RESTORE}"		# File always exists.
   print "Committed ${levelversion} for ${CMVC_RELEASE}" >> ${ABSTRACTLIST}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Writing commit message to file: ${ABSTRACTLIST}"
      return 1
   fi
   print "LASTDEFECT" >> ${TMPFILE1}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Writing LASTDEFECT to file: ${TMPFILE1}"
      return 1
   fi
   modified[${all_defects}]="${RESTORE}"		# File always exists.
   modified[${changeview}]="${RESTORE}"			# File always exists.
   while read defect
   do
      if [[ "${defect}" = "LASTDEFECT" ]]
      then
         defect_count=${VALUE_LIST_SIZE}
      else
         defect_count=${defect_count}+1
         defectlist="${defect},${defectlist}"
         print "${defect} ${CMVC_RELEASE} ${HOSTSFILE_CMVCFAMILY}" \
         >> ${ALL_DEFECTS}
         if [[ $? -ne 0 ]]
         then
            clog +l -e "Writing to ${ALL_DEFECTS}"
            return 1
         fi
      fi
      if [[ ${defect_count} -eq ${VALUE_LIST_SIZE} ]]
      then
         Report -view defectview \
                -where "name in ( ${defectlist%,} )" \
                -raw \
         > ${TMPFILE2}
         if [[ $? -ne 0 ]]
         then
            clog +l -e "Writing defect information to ${ABSTRACTLIST}"
            return 1
         fi
         awk -F"|" '{ printf "%6s:%s  %s\n",$2,CMVC_FAMILY,$9 }' \
             CMVC_FAMILY=${CMVC_FAMILY} ${TMPFILE2} >> ${ABSTRACTLIST}
         # adding features to abstractlist file
         Report -view featureview \
                -where "name in ( ${defectlist%,} )" \
                -raw \
         > ${TMPFILE2}
         if [[ $? -ne 0 ]]
         then
            clog +l -e "Writing feature information to ${ABSTRACTLIST}"
            return 1
         fi
         awk -F"|" '{ printf "%6s:%s  %s\n",$2,CMVC_FAMILY,$7 }' \
             CMVC_FAMILY=${CMVC_FAMILY} ${TMPFILE2} >> ${ABSTRACTLIST}
         Report -view Changeview \
                -where "defectname in ( ${defectlist%,} ) and \
                        releasename='${CMVC_RELEASE}'" \
                -raw \
         >> ${CHANGEVIEW}
         if [[ $? -ne 0 ]]
         then
            clog +l -e "Writing to ${CHANGEVIEW}"
            return 1
         fi
         defect_count=0
         defectlist=""
      fi
   done < ${TMPFILE1}
   rm -f ${TMPFILE1} ${TMPFILE2}
   sort -t' ' -o ${ALL_DEFECTS} -u +1 -2 +0 -1n ${ALL_DEFECTS}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Sort of ${ALL_DEFECTS} failed."
      return 1
   fi

   [[ "${modified[${renamelist}]}" != "${DELETE}" ]] && \
      modified[${renamelist}]="${RESTORE}"
   rm -f ${RENAMELIST}
   touch ${RENAMELIST}
   grep "|rename|" ${CHANGEVIEW} |
      while read line
      do
         name=`echo $line | cut -d'|' -f5`
         oldname=`Report -view fileview \
                         -where "nuPathName='${name}' and releasename='${CMVC_RELEASE}'" \
                         -raw \
                  | cut -d'|' -f8`
         print "${oldname}" >> ${RENAMELIST}
         if [[ $? -ne 0 ]]
         then
            clog +l -e "Writing to ${RENAMELIST}"
            return 1
         fi
      done

   [[ "${modified[${memos}]}" != "${DELETE}" ]] && \
      modified[${memos}]="${RESTORE}"
   [[ "${modified[${symptoms}]}" != "${DELETE}" ]] && \
      modified[${symptoms}]="${RESTORE}"
   export LEVELVERSION="${levelversion}"
   CheckSymptom -r ${CMVC_RELEASE} -l ${MAINLOG}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Creating the symptoms file, Please check logs"
      return 1
   fi

   [[ "${modified[${ssXREF}]}" != "${DELETE}" ]] && \
      modified[${ssXREF}]="${RESTORE}"
   # check subsystem on current level, using prebuild log .....
   chksubsys -r "${CMVC_RELEASE}" -n "$LEVELVERSION" -l "$MAINLOG"
   if [ $? -ne 0 ]
   then
       a1junkvar="WARNING: chksubsys ran unsuccessfully!"
       log "${a1junkvar} (continuing anyhow .....)"
       return 1
   fi

   return 0
}

#
# NAME: Execute
#
# FUNCTION: Execute a command on the extract host.
#
# INPUT: HOST (global) - name of host prebuild is running on.
#        EXTRACTHOST (global) - name of host the extract should be
#                               performed on.
#        RETRY/NORETRY - if the first parameter is $RETRY then give
#              the user the option to try to reexecute the command 
#              if it fails.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: return code of command executed combined with either 'rsh' or
#          'eval' return code.
#
function Execute
{
   typeset -i RC=0                      # Return code from command.
   typeset -r retry=$1
   typeset    PROMPT

   shift   # move over 1 for retry
   if [[ "${HOST}" != "${EXTRACTHOST}" ]]
   then
      RC=$(rsh ${EXTRACTHOST} -l build -n "$*; print \$?")
   else
      RC=$(eval "$*; print \$?")
   fi
   # Want to return error if command generated returns error, 'rsh' returns
   # error or 'eval' returns error.
   (( RC = $RC | $? ))

   # If the command failed, see if the user wants to try it again.

   if [[ ${RC} != 0 && ${retry} = ${RETRY} ]]
   then
      PROMPT="\tExecute of $* failed.\n\tDo you want to fix the failure"
      PROMPT="${PROMPT} and rerun the command? (Y/n/q) \c"
      confirm -y -r "${PROMPT}" && { Execute ${retry} "$*"; return $?; }
   fi

   return ${RC}
}

#
# NAME: Executel
#
# FUNCTION: Execute a command locally.
#
# INPUT: RETRY/NORETRY - if the first parameter is $RETRY then give
#              the user the option to try to reexecute the command 
#              if it fails.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: return code of command executed combined with either 'rsh' or
#          'eval' return code.
#
function Executel
{
   typeset -i RC=0                      # Return code from command.
   typeset -r retry=$1
   typeset    PROMPT

   shift   # move over 1 for retry
   RC=$(eval "$*; print \$?")

   # Want to return error if command generated returns error,
   # or 'eval' returns error.
   (( RC = $RC | $? ))

   # If the command failed, see if the user wants to try it again.

   if [[ ${RC} != 0 && ${retry} = ${RETRY} ]]
   then
      PROMPT="\tExecute of $* failed.\n\tDo you want to fix the failure"
      PROMPT="${PROMPT} and rerun the command? (Y/n/q) \c"
      confirm -y -r "${PROMPT}" && { Executel ${retry} "$*"; return $?; }
   fi

   return ${RC}
}

#
# NAME: Extract_Level
#
# FUNCTION: Extract a level into a delta tree.
#        We'd like for bai and change team to be able to both use
#        prebuild, so we're going to have to basically ignore the
#        EXTRACTHOST and levelroot and figure it out based on what
#        we've got mounted.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        EXTRACTHOST (global) - name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        LTYPE (global) - Type of build (Production or Development).
#        levelversion (global) - name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may copy extracted source over production source tree.
#
# RETURNS: 1 if error occurs and 0 if no error occurs.
#
function Extract_Level
{
   typeset -i RC=0
   typeset -u ans		# Answer to extract prompt.
   typeset    levelroot		# Directory to extract into.
   typeset    log_command="Extract_Level"
   typeset    logline		# Message to log command.
   typeset    CMVCEXTRACTHOST="" # Host that the source is to be extracted to.
   typeset    CMVClevelroot=""  # dir on CMVCEXTRACTHOST to extract to.
   typeset    dir               # Eventually set to the directory that the
                                # cmvcextracthost exported.  It starts off
                                # at $levelroot and shrinks.
   typeset    suffix=""         # Eventually set to whatever we had to lop
                                # off of $levelroot to get to $dir.
   typeset    SKIP              # used to read in garbage from the mount cmd.

   PROMPT="\tDo you wish to extract ${CMVC_RELEASE} level ${levelversion}?"
   PROMPT="${PROMPT} (Y/n/q) \c"
   confirm -y -r "${PROMPT}"
   RC=$?
   if [[ ${RC} -ne 0 ]]
   then
      logline="The delta tree of ${CMVC_RELEASE} level ${levelversion}"
      logline="${logline} will NOT be extracted"
      log "${logline}"
      [[ ${RC} -eq 2 ]] && return 1
      return 0
   fi

   levelroot=${HOSTSFILE_DELTA}/${levelversion}
   dir="$levelroot"

   make_directory ${EXTRACTHOST_COLON}${levelroot}
   if [[ $? -ne 0 ]]
   then
      log +l -e "Cannot create directory ${levelroot} on ${EXTRACTHOST}"
      Set_Status ${S_DELTAEXTRACT} ${ST_FAILURE}
      if [[ $? -ne 0 ]]
      then
         logline="Set_Status error after failing to create directory"
         logline="${logline} ${HOSTSFILE_DELTA}/${levelversion}"
         clog +l -e "${logline}"
      fi
      return 1
   fi

   # If the directory is remote (D) then use mount to figure out the host
   # and levelroot
   case "`/bin/li -dIp $levelroot`" in
	   D*) # look for levelroot in mount
	       # if not found,
	       #    keep stripping off directories from the right
	       #    until it is found
	       # when found,
	       #    get the host and host directory from mount and append the
	       #    stripped off directories
	       while :;do
		   mount | grep "^[^ ]" | grep " $dir  *.fs " | \
		      read CMVCEXTRACTHOST CMVClevelroot SKIP
		   if [ -n "$CMVCEXTRACTHOST" ]
		   then
		       CMVClevelroot=$CMVClevelroot${suffix}
		       break
		   fi
		   suffix="/${dir##*/}$suffix"
		   dir="${dir%/*}"
		   if [ -z "$dir" ];then
		       CMVCEXTRACTHOST=$EXTRACTHOST
		       CMVClevelroot=$levelroot
		       break
		   fi
	       done
	       ;;
	   *) CMVCEXTRACTHOST=$EXTRACTHOST
	      CMVClevelroot=$levelroot
	      ;;
   esac

   logline="Extracting delta tree for ${CMVC_RELEASE} ${LTYPE} level"
   logline="${logline} ${levelversion}"
   log "$logline"

   logline="Level -extract $levelversion -release ${CMVC_RELEASE}"
   logline="${logline} -root $CMVClevelroot -node $CMVCEXTRACTHOST"
   logline="${logline} -dmask 755 -fmask 644 -uid 300 -gid 300"
   log "$logline"

   Undo_ProdDirectory="${levelroot}"

   
   Executel ${RETRY} "Level -extract $levelversion -release ${CMVC_RELEASE}\
         -root $CMVClevelroot -node $CMVCEXTRACTHOST \
         -dmask 755 -fmask 644 -uid 300 -gid 300"
   
   if [[ $? -ne 0 ]]
   then
      Set_Status ${S_DELTAEXTRACT} ${ST_FAILURE}
      if [[ $? -ne 0 ]]
      then
         logline="Set_Status error after delta tree extract error in"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      else
         logline="CMVC Error: failure of delta tree extract in"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      fi
      clog +l -e "${logline}"
      return 1
   fi

   logline="Extracted delta tree for ${CMVC_RELEASE} ${LTYPE}"
   logline="${logline} level ${levelversion}"
   log "${logline}"
   log "into ${levelroot} on ${EXTRACTHOST}"
   Set_Status ${S_DELTAEXTRACT} ${ST_SUCCESS}
   if [[ $? -ne 0 ]]
   then
      logline="Set_Status error after successful delta tree extract in"
      logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      clog +l -e "${logline}"
      return 1
   fi

   return 0
}

#
# NAME: FilterLevels
#
# FUNCTION: Filter out all levels which do not include the input parameter
#           in the level name.  This will help enforce naming conventions,
#           if used.  We also must filter out releases that are not in the
#           hostsfile.dat.
#
# INPUT: filter - name filter will search for.
#
# OUTPUT: if filter matches on level name and the level is in the 
#         hostsfile.dat then the level name, release, and type are
#         displayed to stdout.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function FilterLevels
{
   typeset    filter=$1
   typeset    level
   typeset    release
   typeset    type

   while read level release type
   do
      [[ $level = *$filter* ]] && print $level $release $type
   done

   return 0
}


#
# NAME: FilterReleases
#
# FUNCTION: Filter out all builds which do not include the input parameter
#           in the release name.  This will help select specific release
#           levels (e.g. 320).
#
# INPUT: filter - name filter will search for.
#
# OUTPUT: if filter matches on build name then the build name, release,
#         and type are displayed to stdout.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function FilterReleases
{
   typeset    build
   typeset    filter=$1
   typeset    release
   typeset    type

   while read build release type
   do
      [[ $release = *$filter* ]] && print $build $release $type
   done

   return 0
}

#
# NAME: GetLevelVersion
#
# FUNCTION: Write to stdout a unique version identifier for a particular
#           level.
#
# INPUT: BLDTMP (global) - temporary directory.
#        CMVC_RELEASE (global) - release name.
#        basename - base part of levelname.
#
# OUTPUT: Unique levelname will be displayed on stdout.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function GetLevelVersion
{
   typeset    basename=$1
   typeset -i index=0
   typeset    version="a"

   set +A version_ids a b c d e f g h i j k l m n o p q r s t u v \
            w x y z za zb zc zd ze zf zg zh zi zj zk zl zm \
            zn zo zp zq zr zs zt zu zv zw zx zy zz

   Report -view LevelView -raw -where "Releasename = '${CMVC_RELEASE}' and \
   name like '$basename%'" | cut -d'|' -f1 > $BLDTMP/fullnames$$

   grep "$basename" $BLDTMP/fullnames$$ > $BLDTMP/selectednames$$
   while ((index=index+1)) 
   do
      grep -w $basename$version $BLDTMP/selectednames$$ >& -
      [[ $? -ne 0 ]] && break  # quit loop if name not already used
      version=${version_ids[$index-1]}
   done
   print $version   # the new version
   rm $BLDTMP/fullnames$$ $BLDTMP/selectednames$$

   return 0
}

#
# NAME: Is_Built
#
# FUNCTION: Writes to stdout the string "Yes" if the specified development
#           level has already been included in a new level; otherwise, "No"
#           is written.
#
# INPUT: LEVEL_TEST_LIST (global) - file name of levels
#        level - level name.
#        release - release name.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function Is_Built
{
   typeset    devlevel
   typeset    dummy
   typeset    found
   typeset    level=$1
   typeset    prebuilt=$LEVEL_TEST_LIST
   typeset    release=$2
   typeset    relname

   if [[ -r $prebuilt ]]   # does the new level list exist
   then
      while read dummy devlevel relname
      do
         if [[ $level = $devlevel && $release = $relname ]]
         then
            found="yes"
            break
         fi
      done < $prebuilt
   fi

   if [[ "$found" = "yes" ]]
   then
      print "Yes"
   else
      print "No"
   fi

   return 0
}

#
# NAME: Merge_Source_AFS
#
# FUNCTION: Process the merging of the extracted source into AFS
#           delta tree.
#
# INPUT: CMVC_RELEASE (global) - release name.
#        EXTRACTHOST (global) - name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_AFSBASE (global) - Location in afs of product tree.
#        HOSTSFILE_AFSDELTA (global) - Location in afs of the delta trees.
#        HOSTSFILE_AFSPROD (global) - Location in afs of the product tree.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        levelversion (global) - name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may copy extracted source to AFS delta tree.
#
# RETURNS: 1 if error occurs and 0 if no error occurs
#
function Merge_Source_AFS
{
   typeset -i RC=0		# Return code.
   typeset    log_command="Merge_Source_AFS"
   typeset    logline		# Message to log command.
   typeset -u lssmerge=${LSSMRG}

   if [[ "$lssmerge" = $NO ]]
   then
      log "Merge ${CMVC_RELEASE} delta with AFS tree not performed"
      return 0
   fi

   [[ "${HOSTSFILE_AFSBASE}" = "*" ]] && \
   {
      clog +l -w "AFS delta source merge cannot occur because no"
      clog +l -w "AFS delta source directory is defined"
      return 0
   }

   Check_Success ${S_DELTAEXTRACT}
   [[ $? -ne 0 ]] && \
   {
      clog +l -w "AFS delta source merge cannot occur because the"
      clog +l -w "delta source extract has not completed successfully"
      return 0
   }

   if [[ "$lssmerge" != $YES ]]
   then
      confirm -n -r "\tMerge ${CMVC_RELEASE} delta with AFS Delta Source? (y/N/q) \c"
      RC=$?
      [[ ${RC} -eq 2 ]] && return 1
      [[ ${RC} -eq 1 ]] && return 0
   fi

   # Remove any bldafsmerge records for this build cycle and release.
   DeleteStatus $_TYPE ${T_BLDAFSMERGE} $_BLDCYCLE ${BLDCYCLE} \
                $_RELEASE ${CMVC_RELEASE} \
                $_SUBTYPE ${S_DELTAMERGE}
   RC=$?
   [[ ${RC} -ne 0 ]] && \
   {
      logline="DeleteStatus returned ${RC} (${T_BLDAFSMERGE} ${BLDCYCLE} "
      logline="${logline}${CMVC_RELEASE} ${S_DELTAMERGE})"
      clog +l -e "${logline}"
      return 1
   }

   # Set modes on delta files
   Execute ${NORETRY} "find ${HOSTSFILE_DELTA}/${levelversion} -type d -print \
            | xargs chmod 755"
   Execute ${NORETRY} chmod -R a+r ${HOSTSFILE_DELTA}/${levelversion}

   # Move source to the AFS Delta Source
   if [[ -w ${HOSTSFILE_AFSDELTA} ]]
   then
      if [[ ! -d ${HOSTSFILE_AFSDELTA}/${levelversion} ]]
      then
         mkdir ${HOSTSFILE_AFSDELTA}/${levelversion} 1> /dev/null 2>&1
         if [[ $? -ne 0 ]]
         then
            clog +l -e "AFS delta source merge failed -- Cannot create directory"
            clog +l -e "${HOSTSFILE_AFSDELTA}/${levelversion}"
            return 1
         fi
      fi
   else
      clog +l -e "AFS delta source merge failed, ${HOSTSFILE_AFSDELTA} not"
      clog +l -e "writeable for ${CMVC_RELEASE} level ${levelversion}"
      Set_Status ${S_SOURCEMERGE} ${ST_FAILURE}
      [[ $? -ne 0 ]] && \
      {
         logline="Set_Status error in AFS delta source merge of"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
         clog +l -e "${logline}"
      }
      return 1
   fi

   Undo_AFSDirectory="${HOSTSFILE_AFSDELTA}/${levelversion}"
   if treecopy ${EXTRACTHOST_COLON}${HOSTSFILE_DELTA}/${levelversion} \
               ${HOSTSFILE_AFSDELTA}/$levelversion 1> /dev/null 2>&1
   then
      logline="Delta tree copied to ${HOSTSFILE_AFSDELTA}"
      logline="${logline} for ${CMVC_RELEASE} level ${levelversion}"
      log "${logline}"
      Set_Status ${S_SOURCEMERGE} ${ST_SUCCESS}
      [[ $? -ne 0 ]] && \
      {
         logline="Set_Status error after successful AFS copy to"
         logline="${logline} ${HOSTSFILE_AFSDELTA} for ${CMVC_RELEASE}"
         logline="${logline} level ${levelversion}"
         clog +l -e "${logline}"
         return 1
      }
      print $levelversion >> ${HOSTSFILE_AFSDELTA}/$BLDCYCLE
   else
      log "Merge failed"
      Set_Status ${S_SOURCEMERGE} ${ST_FAILURE}
      [[ $? -ne 0 ]] && \
      {
         logline="Set_Status error after unsuccessful AFS copy to"
         logline="${logline} ${HOSTSFILE_AFSDELTA} for ${CMVC_RELEASE}"
         logline="${logline} level ${levelversion}"
         clog +l -e "${logline}"
      }
      return 1
   fi
   mergemsg="${CMVC_RELEASE} delta merge for $levelversion ($(date))"
   print $mergemsg >> ${HOSTSFILE_AFSPROD%/src}/LEVELS.${CMVC_RELEASE}
   print AFS Delta Source: $mergemsg >> $MRGLIST
   log "AFS Delta Source: $mergemsg"

   return 0
}

#
# NAME: Merge_Source_Build
#
# FUNCTION: Merge the delta tree extracted into a user supplied
#        source tree.
#
# INPUT: CMVC_RELEASE (global) - Release name.
#        EXTRACTHOST (global) - Name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_BASE (global) - Base directory.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        LBMERGEDIRS (global) - If set to ${NO} then don't try to run,
#                    If it is empty or not set, then prompt the user
#                    for directories to merge into.
#                    If it is not empty then attempt to merge the 
#                    source into each directory.
#        levelversion (global) - Name of the level.
#        S_DELTAEXTRACT (global) - used to check delta extract status.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may copy extracted source over source tree.
#
# RETURNS: 1 if error occurs and 0 if no error occurs
#
function Merge_Source_Build
{
   typeset -i RC=0		# Return code.
   typeset    eval_RC		# Return code.
   typeset    log_command="Merge_Source_Build"
   typeset    logline		# Message to log command.
   typeset    mergemsg

   # Do we need to do anything here?
   if [[ "${LBMERGEDIRS}" = ${NO} ]] 
   then return 0
   fi

   Check_Success ${S_DELTAEXTRACT}
   [[ $? -ne 0 ]] && \
   {
      clog +l -w "Build source merge cannot occur"
      clog +l -w "because the delta source extract has not"
      clog +l -w "completed successfully"
      return 0
   }

   if [[ -z "${LBMERGEDIRS}" ]]
   then
      while [[ ${RC} -le 1 ]]
      do
	  echo "Enter a directory to merge ${CMVC_RELEASE} delta into (q to quit):"
	  read dir </dev/tty
	  [[ ${dir}"" = "q" ]] && return $RC
	  Merge_Source_Build_Dir ${dir}; ((RC=$?|$RC))
	  [[ $RC -gt 1 ]] && return 0
      done
   else
      for dir in ${LBMERGEDIRS}
      do
	  Merge_Source_Build_Dir ${dir}; ((RC=$?|$RC))
	  [[ $RC -gt 1 ]] && return 0
      done
   fi
   return $RC
}

#
# NAME: Merge_Source_Build_Dir
#
# FUNCTION: Merge the delta tree extracted into a user supplied
#        source tree.  Checks to make sure that the directory
#        exists before it tries, and if the directory does not
#        exist then it prompts the user to do something about it.
#
# INPUT: $1 - directory to merge the source into.
#        CMVC_RELEASE (global) - Release name.
#        EXTRACTHOST (global) - Name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_BASE (global) - Base directory.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        LBMERGEDIRS (global) - If set to ${NO} then don't try to run,
#                    If it is empty or not set, then prompt the user
#                    for directories to merge into.
#                    If it is not empty then attempt to merge the 
#                    source into each directory.
#        levelversion (global) - Name of the level.
#        S_DELTAEXTRACT (global) - used to check delta extract status.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may copy extracted source over source tree.
#
# RETURNS: 1 if error occurs and 0 if no error occurs.  It is not considered
#         an error if the merge is not attempted.
#
function Merge_Source_Build_Dir
{
    typeset    dir=$1
    typeset -i RC=0
    typeset    log_command="Merge_Source_Build"

    check_directory ${dir}; RC=$?
    while [[ $RC != 0 ]]
    do
	confirm -y -r "\tDirectory '$dir' does not exist.  Do you want to fix it? (Y/n/q) \c"
	case $? in
		0) echo "Enter directory to merge ${CMVC_RELEASE} delta into:"
		   read dir </dev/tty
		   check_directory ${dir}; RC=$?;;
		1) echo "Skipping ${dir} at user request."
		   clog "Merge ${CMVC_RELEASE} delta with ${LTYPE} tree not performed"
		   return 0;;
		2) return 0;;
	esac
    done

    # Get list of files in the delta directory.
    if [[ ${RC} -eq 0 ]]
    then
	Execute ${RETRY} "cd ${HOSTSFILE_DELTA}/${levelversion}; \
	    find . -type f -print > ${DELTA_FILE_LIST}"
	RC=$?
    fi
    # Remove production files that will be overwritten with delta files.
    if [[ ${RC} -eq 0 ]]
    then
	Execute ${RETRY} "cd ${dir}; \
                 xargs rm -f < ${DELTA_FILE_LIST}"
	if [[ $? -ne 0 ]]
	then
	    clog +l -e "Cannot remove files in directory ${dir}."
	    RC=1
	fi
    fi
    # Copy delta files into the production tree.
    if [[ ${RC} -eq 0 ]]
    then
	Execute ${RETRY} "cd ${HOSTSFILE_DELTA}/${levelversion}; \
                 cpio -pdumc ${dir} < ${DELTA_FILE_LIST} \
                 1> /dev/null 2>&1"
	if [[ $? -ne 0 ]]
	then
	    clog +l -e "Cannot copy files from ${HOSTSFILE_DELTA}/${levelversion}"
	    clog +l -e "to ${dir}."
	    RC=1
	fi
    fi
    if [[ ${RC} -ne 0 ]]
    then
	Set_Status "${S_RELMERGE}_${dir}" ${ST_FAILURE}
	if [[ $? -ne 0 ]]
	then
	    logline="Set_Status error after merge error of ${CMVC_RELEASE}"
	    logline="${logline} level ${levelversion} with ${dir}."
	else
	    logline="Merge error of ${CMVC_RELEASE} level ${levelversion}"
	    logline="${logline} with ${dir}."
	fi
	clog +l -e "${logline}"
	return 1
    fi

    clog "Delta tree copied to ${dir}"
     
    # Remove production files in the .gone file.
    if [[ ${RC} -eq 0 ]]
    then
	Execute ${RETRY} "cd ${dir}; \
                 if [ -f .gone ]; \
                 then \
                    xargs rm -f < .gone; \
                 fi"
	if [[ $? -ne 0 ]]
	then
	    clog +l -e "Cannot remove files in directory ${dir} "
	    clog +l -e "from the .gone file"
	    RC=1
	else
	    clog -b ".gone executed at ${dir}"   
	fi
    fi
    # Save the .gone file.
    if [[ ${RC} -eq 0 ]]
    then
	Execute ${RETRY} "cd ${dir}; \
	         if [ -f .gone ]; \
                 then \
                    mv .gone .gone${levelversion}; \
                 fi"
	if [[ $? -ne 0 ]]
	then
	    clog +l -e "Cannot move ${dir}/.gone to"
	    clog +l -e "${dir}/.gone${levelversion}."
	    RC=1
	fi
    fi

    mergemsg="${CMVC_RELEASE} merge for ${levelversion} ($(date))"
    log -f "Build Tree: $mergemsg"
    Execute ${NORETRY} "print \"${mergemsg}\" \
       >> ${HOSTSFILE_BASE}/LEVELS.${CMVC_RELEASE}"
    print Production Tree: $mergemsg >> $MRGLIST
    Set_Status  "${S_RELMERGE}_${dir}" ${ST_SUCCESS}
    if [[ $? -ne 0 ]] 
    then
	logline="Set_Status error after successful merge of ${CMVC_RELEASE}"
	logline="${logline} level ${levelversion} with ${dir}"
	clog +l -e "${logline}"
	return 1
    fi

    return 0
}


#
# NAME: Merge_Source_Production
#
# FUNCTION: Merge the delta tree extracted with the source tree.
#
# INPUT: CMVC_RELEASE (global) - Release name.
#        EXTRACTHOST (global) - Name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_BASE (global) - Base directory.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        HOSTSFILE_PROD (global) - Location of the product tree.
#        LMERGE (global) - Merge delta tree with source tree.
#        levelversion (global) - Name of the level.
#
# OUTPUT: none.
#
# SIDE EFFECTS: may copy extracted source over source tree.
#
# RETURNS: 1 if error occurs and 0 if no error occurs
#
function Merge_Source_Production
{
   typeset -i RC=0		# Return code.
   typeset    eval_RC		# Return code.
   typeset    log_command="Merge_Source_Production"
   typeset    logline		# Message to log command.
   typeset    mergemsg

   Check_Success ${S_DELTAEXTRACT}
   [[ $? -ne 0 ]] && \
   {
      clog +l -w "Production source merge cannot occur"
      clog +l -w "because the delta source extract has not"
      clog +l -w "completed successfully"
      return 0
   }

   if [[ "${LMERGE}" = ${NO} ]] 
   then
      RC=1
   elif [[ "${LMERGE}" != ${YES} ]]
   then
      confirm -n -r "\tMerge ${CMVC_RELEASE} delta with ${LTYPE} tree? (y/N/q) \c"
      RC=$?
   fi
   if [[ ${RC} -ne 0 ]]
   then
      clog "Merge ${CMVC_RELEASE} delta with ${LTYPE} tree not performed"
      [[ ${RC} -eq 2 ]] && return 1
      return 0
   fi

   # Insure that delta directory exists.
   check_directory ${EXTRACTHOST_COLON}${HOSTSFILE_DELTA}/${levelversion}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Directory ${HOSTSFILE_DELTA}/${levelversion} does not exist"
      clog +l -e "on ${EXTRACTHOST}"
      RC=1
   fi
   # Get list of files in the delta directory.
   if [[ ${RC} -eq 0 ]]
   then
      Execute ${RETRY} "cd ${HOSTSFILE_DELTA}/${levelversion}; \
               find . -type f -print > ${DELTA_FILE_LIST}"
      RC=$?
   fi
   if [[ ""${PREBUILD_SAVE_REPLACED_SRC} != ${NO} ]]
   then
      Backup_Old_Production_Files
      RC=$?
   fi
   # Remove production files that will be overwritten with delta files.
   if [[ ${RC} -eq 0 ]]
   then
      Execute ${RETRY} "cd ${HOSTSFILE_PROD}; \
               xargs rm -f < ${DELTA_FILE_LIST}"
      if [[ $? -ne 0 ]]
      then
         clog +l -e "Cannot remove files in directory ${HOSTSFILE_PROD} on"
         clog +l -e "${EXTRACTHOST}."
         RC=1
      fi
   fi
   # Copy delta files into the production tree.
   if [[ ${RC} -eq 0 ]]
   then
      # We can't undo the merge if we haven't saved the source.
      [[ ""${PREBUILD_SAVE_REPLACED_SRC} != ${NO} ]] \
         && Undo_ProdMerge="${levelversion}"
      Execute ${RETRY} "cd ${HOSTSFILE_DELTA}/${levelversion}; \
               cpio -pdumc ${HOSTSFILE_PROD} < ${DELTA_FILE_LIST} \
               1> /dev/null 2>&1"
      if [[ $? -ne 0 ]]
      then
         clog +l -e "Cannot copy files from ${HOSTSFILE_DELTA}/${levelversion}"
         clog +l -e "to ${HOSTSFILE_PROD} on ${EXTRACTHOST}"
         RC=1
      fi
   fi
   if [[ ${RC} -ne 0 ]]
   then
      Set_Status ${S_RELMERGE} ${ST_FAILURE}
      if [[ $? -ne 0 ]]
      then
         logline="Set_Status error after merge error in"
         logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      else
         logline="Merge error with ${CMVC_RELEASE} level ${levelversion}"
      fi
      clog +l -e "${logline}"
      return 1
   fi

   clog "Delta tree copied to ${HOSTSFILE_PROD}"
     
   # Remove production files in the .gone file.
   if [[ ${RC} -eq 0 ]]
   then
      Execute ${RETRY} "cd ${HOSTSFILE_PROD}; \
               if [ -f .gone ]; \
               then \
                  xargs rm -f < .gone; \
               fi"
      if [[ $? -ne 0 ]]
      then
         clog +l -e "Cannot remove files in directory ${HOSTSFILE_PROD} on"
         clog +l -e "${EXTRACTHOST} from the .gone file"
         RC=1
      else
         clog -b ".gone executed at ${HOSTSFILE_PROD}"   
      fi
   fi
   # Save the .gone file.
   if [[ ${RC} -eq 0 ]]
   then
      Execute ${RETRY} "cd ${HOSTSFILE_PROD}; \
               if [ -f .gone ]; \
               then \
                  mv .gone .gone${levelversion}; \
               fi"
      if [[ $? -ne 0 ]]
      then
         clog +l -e "Cannot move ${HOSTSFILE_PROD}/.gone to"
         clog +l -e "${HOSTSFILE_PROD}/.gone${levelversion} on ${EXTRACTHOST}"
         RC=1
      fi
   fi

   mergemsg="${CMVC_RELEASE} merge for ${levelversion} ($(date))"
   log -f "Production Tree: $mergemsg"
   Execute ${NORETRY} "print \"${mergemsg}\" \
       >> ${HOSTSFILE_BASE}/LEVELS.${CMVC_RELEASE}"
   print Production Tree: $mergemsg >> $MRGLIST
   Set_Status ${S_RELMERGE} ${ST_SUCCESS}
   if [[ $? -ne 0 ]] 
   then
      logline="Set_Status error after successful merge of"
      logline="${logline} ${CMVC_RELEASE} level ${levelversion}"
      clog +l -e "${logline}"
      return 1
   fi

   return 0
}


#
# NAME: Backup_Old_Production_Files
#
# FUNCTION: Save off files in the production tree that would be removed
#       or replaced by source from the newly extracted level.
#
# INPUT: PRODUCTION_BACKUP (global) - directory for old production files.
#        EXTRACTHOST (global) - Name of host the extract should be
#                               performed on.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_BASE (global) - Base directory.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        HOSTSFILE_PROD (global) - Location of the product tree.
#        LMERGE (global) - Merge delta tree with source tree.
#        levelversion (global) - Name of the level.
#        DELTA_FILE_LIST_IN_SRC (global) - list of files in the delta
#                          tree that are also in the prod tree.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS:
#
function Backup_Old_Production_Files
{
   typeset    log_command="Backup_Old_Production_Files"

   # Create directory for backing up files in production tree that will be
   # overwritten with the delta files.
   Execute ${NORETRY} "rm -rf ${PRODUCTION_BACKUP}"
   make_directory ${EXTRACTHOST_COLON}${PRODUCTION_BACKUP}
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Cannot create directory ${PRODUCTION_BACKUP} on"
      clog +l -e "${EXTRACTHOST}"
      return 1
   fi
   # Backup the production files that will be overwritten or removed when
   # the delta tree is copied.
   # First determine files that are in src tree.  We've got to create this
   # list because the cpio that backs up the src tree will fail if we ask
   # it to copy over files that don't yet exist in the src tree.
   Execute ${RETRY} "cd ${HOSTSFILE_PROD}; \
            rm -f ${DELTA_FILE_LIST_IN_SRC}; \
            for input_file in ${DELTA_FILE_LIST} .gone; \
            do \
               if [ -f \${input_file} ]; \
               then \
                  while read file; \
                  do \
                     if [ -f \${file} ]; \
                     then \
                        print \${file} >> ${DELTA_FILE_LIST_IN_SRC}; \
                     fi \
                  done < \${input_file}; \
               fi \
            done; \
            touch ${DELTA_FILE_LIST_IN_SRC}"
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Cannot create list of delta files in source tree on"
      clog +l -e "${EXTRACTHOST} from ${HOSTSFILE_PROD}"
      return 1
   fi
   # Then make a backup of those files in the src tree.
   Execute ${RETRY} "cd ${HOSTSFILE_PROD}; \
            cpio -pdumc ${PRODUCTION_BACKUP} < ${DELTA_FILE_LIST_IN_SRC} \
            1> /dev/null 2>&1"
   if [[ $? -ne 0 ]]
   then
      clog +l -e "Cannot copy into directory ${PRODUCTION_BACKUP} on"
      clog +l -e "${EXTRACTHOST} from ${HOSTSFILE_PROD}"
      return 1
   fi

   return 0
}

#
# NAME: Prebuild_Menu
#
# FUNCTION: Display prebuild menu and process user input commands until
#           user gives command to continue on, in which case this function
#           will return.
#
# INPUT: abstractlist (global) - Index into filename[] for abstractlist.
#        filename[] (global) - Array of filenames.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS:
#
function Prebuild_Menu
{
   typeset    Log
   typeset -i RC=0
   typeset    catalogs=$CATALOGS
   typeset    defects=$TEMP
   typeset    errorflag=""
   typeset    etemp=$ERRLOG
   typeset    input
   typeset -L10 isbld isbldH="PREBUILT"
   typeset    levelname
   typeset    levelstodo=$1
   typeset -L16 lname lnameH="LEVEL NAME"
   typeset    log_command="Prebuild_Menu"
   typeset    logline		# Message to log command.
   typeset    lognum
   typeset    lrelease
   typeset -L12 ltype ltypeH="LEVEL TYPE"
   typeset    printline
   typeset    release
   typeset -u response
   typeset -L15 rname rnameH="RELEASE"
   typeset    tdcopy=$TD_COPY
   typeset    temp=$TEMP
   typeset -u uline

   if [[ ! -s $levelstodo ]]
   then
      logline="No levels to build. But you can use prebuild to"
      logline="${logline} create one"
      log "$logline"
   fi
   while true
   do
   print
   print Build Cycle  : ${BLDCYCLE}
   print Build Owner  : ${BLDOWNER}
   print CMVC Login   : $CMVC_BECOME
   print Current Level: $LEVELNAME
   print Master Log   : $MAINLOG
   print
   print "\nNumber of Levels selected :" $(wc -l <$levelstodo)
   print "1)   Show Level List" > /dev/tty
   print "2)   Create Level" > /dev/tty
   print "3)   Prune the Level List" > /dev/tty
   print "4)   Process Level List" > /dev/tty
   print "5)   Show Logs for $LEVELNAME by Release" > /dev/tty
   print "6)   Show Master Logs for $LEVELNAME" > /dev/tty
   print "7)   Show Errors for $LEVELNAME " > /dev/tty
   print "8)   Show Merge List (Production)" > /dev/tty
   print "9)   Show Abstracts (${filename[${abstractlist}]})" > /dev/tty
   print "10)   Exit" > /dev/tty
   print > /dev/tty   # add line break for log file  
   print "(prebuild) Enter: \c" > /dev/tty
   read choice < /dev/tty
      case $choice in
      "4")
         break
         ;;
      "2")
         PROMPT="You can enter defects manually or by file.\n"
         PROMPT="${PROMPT}Would you like to enter manually? (Y/n/q) \c"
         confirm -y -r "${PROMPT}"
         RC=$?
         if [[ ${RC} -eq 0 ]]
         then
            > $defects
            print "Null value signals end of input" > /dev/tty
            while true
            do
               print "Defect Number: \c" > /dev/tty
               read input < /dev/tty
               if [[ -z "$input" ]]
               then
                  break
               else
                  print $input >> $defects
               fi
            done
         elif [[ ${RC} -eq 1 ]]
         then
            print "Enter filename: \c" > /dev/tty
            read input < /dev/tty
            if [[ -r "$input" && -f "$input" ]]
            then
               cat $input > $defects         
            else
               log_line="Unable to read $input"
               clog +l -e "$log_line"
               continue
            fi
         else
            exit 1
         fi
         print "Level name: \c" > /dev/tty
         read levelname < /dev/tty
         print "Release name: \c" > /dev/tty
         read lrelease < /dev/tty
         CMVC_RELEASE=${lrelease}
         print "Level type: \c" > /dev/tty
         read ltype < /dev/tty
         levelname="$(Check_Levelname_Length ${levelname})"
         [[ $? -ne 0 ]] && continue
         Level -create ${levelname} -type $ltype -release $lrelease
         [[ $? -ne 0 ]] && continue
         LevelMember -create -level ${levelname} -release $lrelease \
            -defect - < $defects
         if [[ $? -ne 0 ]]
         then
            Level -delete ${levelname} -release $lrelease
            continue
         fi
         Undo_Level="${levelname}"
         Check_Level ${levelname}
         if [[ $? -ne 0 ]]
         then
            confirm -n -r "\tContinue with ${lrelease} level ${levelname}? (y/N/q) \c"
            if [[ $? -eq 0 ]]
            then
               log "Continuing with ${lrelease} level ${levelname}"
            else
               Undo_Work
               continue
            fi
         fi
         # CMVC does not like builds being assigned to the same owner.
         if [[ "${BLDOWNER}" != "${CMVC_BECOME}" ]]
         then
            Level -assign ${levelname} -to ${BLDOWNER} -release $lrelease
            print ${levelname} $lrelease $ltype >> $levelstodo
            logline="Assigning ${lrelease} level ${levelname}"
            logline="${logline} to ${BLDOWNER}"
            log "${logline}"
         fi
         ;;
      "1")
         [[ ! -s $levelstodo ]] && continue
         print "$isbldH $rnameH $ltypeH $lnameH" > $tdcopy
         while read lname rname ltype
         do
            isbld=$(Is_Built $lname $rname)
            print "$isbld $rname $ltype $lname" >> $tdcopy
         done < $levelstodo
         pg $tdcopy > /dev/tty
         ;;
      "3")
         cp $levelstodo $tdcopy;
         print "\c" > $levelstodo
         while read lname rname ltype
         do
            isbld=$(Is_Built $lname $rname)
            print "$isbldH $rnameH $ltypeH $lnameH"
            print "$isbld $rname $ltype $lname\c"
            print " Confirm: \c"
            read response < /dev/tty; print $response
            if [[ "$response" = $YES ]]
            then
               print $lname $rname $ltype >>$levelstodo
            fi
         done < $tdcopy
         ;;
      "5")
         cd $LOG   # move to log directory to get log files
         cat $(li -1Su | grep $GMASTER_LOGS) > $catalogs
         cd ~-    # return to previous directory
         printline="Enter release name or release substring: "
         print "${printline} \c" > /dev/tty
         read release < /dev/tty
         if [[ "$PTERM" = "aixterm" ]]
         then
            xterm =80x24 -T "Release Log"\
               -ar -W -e\
            showlog -S -1$LEVELNAME -2$release $catalogs &
         else
            showlog -S -1$LEVELNAME -2$release $catalogs
         fi
         ;;
       "6")
         li -l -Su $MASTER_LOGS | cut -c45-120 |
            nl -nln > $temp
         pg $temp > /dev/tty
         printline="Enter log number (enter null for all): "
         print "${printline} \c" > /dev/tty
         read lognum < /dev/tty
         if [[ -n $lognum ]]
         then
             li -l -Su $MASTER_LOGS | cut -c57-120 |
             tail +${lognum:=1} | read Log
         else
             cat $(li -l -Su $MASTER_LOGS | cut -c57-120) > $temp
             Log=$temp
         fi
         [[ ! -f "$Log" ]] && continue
         if [[ "$PTERM" = "aixterm" ]]
         then
            xterm =80x24 -T "Master Log $log" \
               -ar -W -e showlog $Log &
                        else
                                showlog $Log
         fi
         ;;
      "7")
         cd $LOG   # move to log directory to get log files
         cat $(li -1Su | grep $GMASTER_LOGS) > $catalogs
         cd ~-    # return to previous directory
         if [[ "$PTERM" = "aixterm" ]]
         then
            xterm =80x24 -T "Release Log"\
            -ar -W  -e showlog -s "ERR" $catalogs &
         else
            showlog -s "ERR" $catalogs
         fi
         ;;
      "8")
         if [[ "$PTERM" = "aixterm" ]]
         then
            xterm =80x24 -T "Merge List"\
            -ar -W -e QueryStatus $_SUBTYPE $S_RELMERGE \
            -A | awk -F"|" '{ \
               print $1"|"$2"|"$3"|"$4"|"$5"|"$6"|"$10
            }' | pg &
         else
            QueryStatus $_SUBTYPE $S_RELMERGE -A | \
            awk -F"|" '{ \
               print $1"|"$2"|"$3"|"$4"|"$5"|"$6"|"$10
            }' | pg
         fi
         ;;
      "9")
         if [[ ! -f "${filename[${abstractlist}]}" ]]
         then
            clog "No defects for $LEVELNAME"
         elif [[ "$PTERM" = "aixterm" ]]
         then
            xterm =80x24 -T "Defect List"\
            -ar -W -e vi -R ${filename[${abstractlist}]} &
         else
            pg ${filename[${abstractlist}]} > /dev/tty
         fi
         ;;
      ("10"|"q"|"Q"|"Quit"|"quit"|"Exit"|"exit")

         return 1
         ;;
      "")
         print You must select one of the above;;
      esac
      print    # add line break for log file  
   done

   if [[ ! -s $levelstodo ]]
   then
      log "No releases to build"
      return 1
   fi
   rm -f $temp $catalogs $etemp $tdcopy
   return 0
}

#
# NAME: Process_Production_Level
#
# FUNCTION: Check a level that is already prodction
#
# INPUT: CMVC_RELEASE (global) - release name.
#        LCONT (global) - If set, continue with build even when track errors
#                         are seen.
#        levelname - name of the level.
#
# OUTPUT: levelversion (global) - name of the level.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if processing on level should stop and 0 if processing should
#          continue
#
function Process_Production_Level
{
   typeset -i RC=0
   typeset    log_command="prebuild"
   typeset    levelname=$1
   typeset -u response

   levelversion=${levelname}

   logset -c prebuild -C prebuild -F $MAINLOG -1 $levelversion -2 ${CMVC_RELEASE}

   Check_Level ${levelversion}
   [[ $? -eq 0 ]] && return 0

   [[ "$LCONT" = $YES ]] && return 0
   [[ "$LCONT" = $NO ]] && return 1

   confirm -r "\tContinue with ${CMVC_RELEASE} level ${levelversion}? (y/n/q) \c"
   RC=$?
   if [[ ${RC} -ne 0 ]]
   then
      logline="Stopping with ${CMVC_RELEASE} level ${levelversion} because"
      logline="${logline} of track errors"
      log "${logline}"
      return 1
   fi

   logline="Continuing with ${CMVC_RELEASE} level ${levelversion}"
   logline="${logline} with track errors"
   log "${logline}"

   return 0
}

#
# NAME: Restore_Selfix_Data
#
# FUNCTION: Restore files used by selective fix process to the contents
#           they had before data from current level was added.  Files
#           include the abstractlist, all_defects, changeview, defects,
#           memos, ptfrequisities, renamelist and symptoms.
#
# INPUT: DELETE (global) - value for modified[], indicates the file should not
#                          exist if level commit fails.
#        NOTMODIFIED (global) - value for modified[], indicates the file has
#                               not been modified while processing current
#                               level.
#        RESTORE (global) - value for modified[], indicates the file does exist
#                           and will need to be restored if level commit fails.
#        filename[] (global) - Array of filenames that may be modified by
#                              prebuild.
#        modified[] (global) - Array indicating if a file has been modified.
#        remove_backup_files (1) - Value is passed when level commit is
#                                   successful, should remove all the backup
#                                   files and restore modified[] to
#                                   ${NOTMODIFIED}
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 1 if one or more files could not be restored; 0 otherwise.
#
function Restore_Selfix_Data
{
   typeset -i RC=0		# Return code.
   typeset -i index=0		# Index into the filename and modified arrays.
   typeset    log_command="Restore_Selfix_Data"
   typeset    remove_backup_files=$1

   # Don't restore anything if we're not doing an update build.
   [[ ${PTF_UPDATE} != ${YES} ]] && return 0

   # Backup up selective fix data files.
   while [[ ${index} -lt ${#filename[*]} ]]
   do
      if [[ "${remove_backup_files}" = "remove_backup_files" ]]
      then
         rm -f ${filename[${index}]}_backup.$$
      else
         case "${modified[${index}]}" in
            ${DELETE})
               rm -f ${filename[${index}]}
               ;;
            ${NOTMODIFIED})
               ;;
            ${RESTORE})
               rm -f ${filename[${index}]}
               cp ${filename[${index}]}_backup.$$ ${filename[${index}]}
               if [[ $? -ne 0 ]]
               then
                  RC=1
                  clog +l -e "Cannot restore ${filename[${index}]}, original"
                  clog +l -e "copy is in ${filename[${index}]}_backup.$$"
               else
                  clog +l "Restored ${filename[${index}]}"
                  rm -f ${filename[${index}]}_backup.$$
               fi
               ;;
            *) clog +l -e "Unknown flag \"${modified[${index}]}\" found with"
               clog +l -e "file ${filename[${index}]}."
               ;;
         esac
      fi
      modified[${index}]=${NOTMODIFIED}
      (( index=${index}+1 ))
   done
   [[ ${RC} -ne 0 ]] && return 1
}

#
# NAME: Set_Status
#
# FUNCTION: Calls DeleteStatus and bldsetstatus functions. 
#
# INPUT: CMVC_RELEASE (global) - release name.
#        levelversion (global) - name of the level.
#        subtype - subtype argument to pass to commands called.
#        status - status argument to pass to commands called.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: Value returned from bldsetstatus.
#
function Set_Status
{
   typeset -i RC=0		# Return code.
   typeset    subtype=$1
   typeset    status=$2

   DeleteStatus -F $_TYPE $T_PREBUILD \
                   $_SUBTYPE ${subtype} \
                   $_BLDCYCLE $BLDCYCLE \
                   $_RELEASE ${CMVC_RELEASE} \
                   $_LEVEL ${levelversion}
   RC=$?
   bldsetstatus $_TYPE $T_PREBUILD \
                $_SUBTYPE ${subtype} \
                $_BLDCYCLE $BLDCYCLE \
                $_RELEASE ${CMVC_RELEASE} \
                $_LEVEL ${levelversion} \
                $_STATUS ${status}
   (( RC = ${RC} | $? ))

   return ${RC}
}

#
# NAME: Undo_Work
#
# FUNCTION: Allow user to back out any changes made to the level in the
#           current release if level has not yet been committed.
#
# INPUT: MFLMEMBERS (global) - File name where list of defects in current
#                              level will be stored.
#        TMPFILE1 (global) - temporary file 1.
#        Undo_AFSDirectory (global) - AFS Delta directory created and
#                                     populated with files from current level.
#        Undo_Level (global) - If nonzero then level that prebuild has
#                              created for current release.
#        Undo_ProdDirectory (global) - Production directory created and
#                                      populated with files from current level.
#        Undo_ProdMerge (global) - Delta level that has been merged into the
#                                  production tree.
#        Undo_TypeOther_Levels (global) - List of levels that were changed
#                                         from type development to other.
#      
#
# OUTPUT: Undo_Work_Visited (global) - ${FALSE} indicates the function
#                                      Undo_Work has not been called, ${TRUE}
#                                      indicated the function has been called.
#
# SIDE EFFECTS: none.
#
# RETURNS: Always returns 0.
#
function Undo_Work
{
   typeset -i RC=0			# Return Code.
   typeset    directory			# Loop variable.
   typeset    mergemsg
   typeset    tmp			# Short variable name to hold long
					# filename.
   typeset    type=""			# Type of status record.

   if [[ "${Undo_Work_Visited}" = "${TRUE}" ]]
   then
      # Already called Undo_Work once for this release and level, trap to
      # exit will call Undo_Work again. 
      return 0
   else
      Undo_Work_Visited="${TRUE}"
   fi

   # Let the user know what the following prompts are for.
   if [[ -n "${Undo_ProdMerge}" || \
         -n "${Undo_ProdDirectory}" || \
         -n "${Undo_AFSDirectory}" || \
         -n "${Undo_Level}" || \
         -n "${Undo_TypeOther_Levels}" ]]
   then
      log ">>>>>"
      log ">>>>> Prebuild cannot complete successfully."
      log ">>>>> The changes prebuild has made can be undone by answering"
      log ">>>>> yes (Y) to the questions that will follow."
      log ">>>>>"
   fi

   Restore_Selfix_Data

   if [[ -n "${Undo_ProdMerge}" ]]
   then
      prompt="Do you want to restore the file(s) modified by level"
      prompt="${prompt} ${Undo_ProdMerge}\n\tin the production tree"
      prompt="${prompt} ${HOSTSFILE_PROD}?" 
      confirm -y -r "\t${prompt} (Y/n/q) \c"
      if [[ $? -eq 0 ]]
      then
         Execute ${NORETRY} "cd ${HOSTSFILE_PROD}; \
                  xargs rm -f < ${DELTA_FILE_LIST}"
         if [[ $? -ne 0 ]]
         then
            log +l -e "Cannot remove files in directory ${HOSTSFILE_PROD} on"
            log +l -e "${EXTRACTHOST} updated by level ${Undo_ProdMerge}."
         fi
         Execute ${NORETRY} "cd ${PRODUCTION_BACKUP}; \
                  find . -type f -print | cpio -pdumc ${HOSTSFILE_PROD} \
                  1> /dev/null 2>&1"
         if [[ $? -ne 0 ]]
         then
            log +l -e "Cannot restore directory ${PRODUCTION_BACKUP} into"
            log +l -e "${HOSTSFILE_PROD} on ${EXTRACTHOST}"
         else
            mergemsg="${CMVC_RELEASE} level ${levelversion} unmerged ($(date))"
            Execute ${NORETRY} "print \"${mergemsg}\" \
	        >> ${HOSTSFILE_BASE}/LEVELS.${CMVC_RELEASE}"
            log "Restored directory ${HOSTSFILE_PROD} on ${EXTRACTHOST}"
         fi
         Set_Status ${S_RELMERGE} ${ST_FAILURE}
         [[ $? -ne 0 ]] && log +l -e "Set_Status ${S_RELMERGE} ${ST_FAILURE}"
      fi
   fi
   Undo_ProdMerge=""

   if [[ -n "${Undo_ProdDirectory}" ]]
   then
      confirm -y -r "\tDo you want remove directory\n\t${Undo_ProdDirectory}? (Y/n/q) \c"
      RC=$?
      [[ ${RC} -eq 2 ]] && { Clean_Up; exit 1; }
      if [[ ${RC} -eq 0 ]]
      then
         Execute ${NORETRY} rm -fr ${Undo_ProdDirectory}
         if [[ $? -ne 0 ]]
         then
            log +l -e "Cannot remove directory ${Undo_ProdDirectory} on ${EXTRACTHOST}"
         else
            log "Removed directory ${Undo_ProdDirectory} on ${EXTRACTHOST}"
         fi
         Set_Status ${S_DELTAEXTRACT} ${ST_FAILURE}
         [[ $? -ne 0 ]] && log +l -e "Set_Status ${S_DELTAEXTRACT} ${ST_FAILURE}"
      fi
      Undo_ProdDirectory=""
   fi

   if [[ -n "${Undo_AFSDirectory}" ]]
   then
      confirm -y -r "\tDo you want remove directory\n\t${Undo_AFSDirectory}? (Y/n/q) \c"
      RC=$?
      [[ ${RC} -eq 2 ]] && { Clean_Up; exit 1; }
      if [[ ${RC} -eq 0 ]]
      then
         Execute ${NORETRY} rm -fr ${Undo_AFSDirectory}
         if [[ $? -ne 0 ]]
         then
            log +l -e "Cannot remove directory ${Undo_AFSDirectory}"
         else
            log "Removed directory ${Undo_AFSDirectory}"
         fi
         tmp="${HOSTSFILE_AFSDELTA}/${BLDCYCLE}"
         cp ${tmp} ${TMPFILE1}
         rm -f ${tmp}
         grep -v ${Undo_Level} ${TMPFILE1} > ${tmp}
         if [[ $? -eq 2 ]]
         then
            log +l -e "Remove of ${Undo_Level} from ${tmp}"
         else
            rm -f ${TMPFILE1}
            log "Removed {Undo_Level} from ${tmp}"
         fi  
         Set_Status ${S_SOURCEMERGE} ${ST_FAILURE}
         [[ $? -ne 0 ]] && log +l -e "Set_Status ${S_SOURCEMERGE} ${ST_FAILURE}"
      fi
      mergemsg="${CMVC_RELEASE} delta removed for ${levelversion} ($(date))"
      print ${mergemsg} >> ${HOSTSFILE_AFSPROD%/src}/LEVELS.${CMVC_RELEASE}
      Undo_AFSDirectory=""
   fi

   # Attempt to delete the Level.
   if [[ -n "${Undo_Level}" ]]
   then
      confirm -y -r "\tDo you want remove level ${Undo_Level}? (Y/n/q) \c"
      RC=$?
      if [[ ${RC} -eq 0 ]]
      then
         Report -view levelmemberview \
                -where "levelname='${Undo_Level}' and releasename='${CMVC_RELEASE}'" \
                -raw \
         | cut -d'|' -f3 \
         > ${MFLMEMBERS}
	 # Levels may only be deleted by the owner so reassign it first.
	 Level -assign ${Undo_Level} -release ${CMVC_RELEASE} -to ${CMVC_BECOME}
         [[ $? -ne 0 ]] && {\
	    logline="Cannot assign ${CMVC_RELEASE} ${Undo_Level} level"
            log +l -w "${logline} to ${CMVC_BECOME}."
         }
         LevelMember -delete -level ${Undo_Level} -release ${CMVC_RELEASE} \
                     -defect - < ${MFLMEMBERS}
         [[ $? -ne 0 ]] && \
            log +l -e "Cannot delete ${CMVC_RELEASE} ${Undo_Level} level members"
         Level -delete ${Undo_Level} -release ${CMVC_RELEASE}
         if [[ $? -ne 0 ]]
         then
            log +l -e "Cannot remove ${CMVC_RELEASE} ${Undo_Level} level"
         else
            log "Removed ${CMVC_RELEASE} level ${Undo_Level}"
         fi
      elif [[ ${RC} -eq 2 ]]
      then
         Clean_Up
         exit 1
      fi
   fi
   Undo_Level=""

   if [[ -n "${Undo_TypeOther_Levels}" ]]
   then
      for level in ${Undo_TypeOther_Levels}""
      do
         PROMPT="\tDo you want to return the ${CMVC_RELEASE} level ${level} to"
         PROMPT="${PROMPT} type development? (Y/n/q) \c"
         confirm -y -r "${PROMPT}"
         RC=$?
         [[ ${RC} -eq 1 ]] && continue
         [[ ${RC} -eq 2 ]] && { Clean_Up; exit 1; }
         Level -modify ${level} -type development -release ${CMVC_RELEASE}
         if [[ $? -ne 0 ]]
         then
            log +l -e "CMVC Error: Level modify ${level} to type 'development'"
         else
            log "${CMVC_RELEASE} level ${level} changed to type 'development'"
         fi
      done
   fi
   Undo_TypeOther_Levels=""

   return 0
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
   log "Usage: prebuild [-a <area_list>] [-f <area_filename>]"
   log "                [-h <hosts_filename>] [-n <node_filename>]"
   log "                [-l] [-s] [-c] [-u] [<release> ...]"
   log "    where:"
   log "    [-l] adds '-long' to LEVELCHECK_FLAGS"
   log "    [-s] sets PREBUILD_SAVE_REPLACED_SRC to 'No'"
   log "    [-c] sets PREBUILD_RUN_LEVELCHECK to 'No'"
   log "    [-u] sets PTF_UPDATE to 'yes'"
   RC=1
   exit 1
   # exit above will call Undo_Work and Clean_Up.
}

#
# NAME: Verify_Write_Access
#
# FUNCTION: Verify write access to all directories associated with the
#           release to check.
#
# INPUT: CMVC_RELEASE (global) - name of release to check or writability.
#        EXTRACTHOST_COLON (global) - NULL if current host is the
#                                     extract host; otherwise, the
#                                     extract host followed by a ':'.
#        HOSTSFILE_AFSBASE (global) - Location in afs of product tree.
#        HOSTSFILE_AFSDELTA (global) - Location in afs of the delta trees.
#        HOSTSFILE_AFSPROD (global) - Location in afs of the product tree.
#        HOSTSFILE_DELTA (global) - Location of the delta trees.
#        HOSTSFILE_PROD (global) - Location of the product tree.
#
# OUTPUT: none.
#
# SIDE EFFECTS:
#
# RETURNS: 0 on success, 1 if write access problems exist, 2 if user
#          respondes to a prompt with 'q'.
#

function Verify_Write_Access
{
   typeset -i RC=0
   typeset    dir			# Directory being verified.
   typeset    log_command="Verify_Write_Access"
   typeset    return_status=0		# Return value.
   typeset -u reply			# Response from user.

   [[ -z ${CMVC_RELEASE}"" ]] && return 1

   make_directory ${EXTRACTHOST_COLON}${HOSTSFILE_PROD}
   [[ $? -ne 0 ]] && log +l -x "Cannot continue"
   make_directory ${EXTRACTHOST_COLON}${HOSTSFILE_DELTA}
   [[ $? -ne 0 ]] && log +l -x "Cannot continue"

   #
   # These directories should be writable.  Allow user to determine if
   # they should continue.
   #
   if [[ "${HOSTSFILE_AFSBASE}" != "*" ]]
   then
      for dir in ${HOSTSFILE_AFSPROD} ${HOSTSFILE_AFSDELTA}""
      do
         if [[ -n "${dir}" && ! -w "${dir}" ]]
         then
            clog +l -e "${dir} not writable"
            clog +l "Try \"klog ${CMVC_BECOME}\" to fix the problem"
            if [[ ${return_status} -eq 0 ]]
            then
               confirm -n -r "\tDo you want to continue? (y/N/q) \c"
               RC=$?
               if [[ ${RC} -eq 1 ]]
               then
                  clog "Will stop after checking all other directories"
                  return_status=1
               elif [[ ${RC} -eq 2 ]]
               then
                  return 1
               fi
            fi
         fi
      done
   fi

   return ${return_status}
}

################################# Housekeeping ################################

. bldloginit

trap 'trap - EXIT; Undo_Work; Clean_Up; exit ${RC}' INT QUIT HUP TERM EXIT

. bldkshconst
. bldinitfunc
. bldkshfunc
. bldhostsfile
. bldnodenames
. bldtrackingdefect

bldinit prebuild

chksetbecome
readonly CMVC_BECOME
chksetbldowner

test ${LOGPREBUILD:=$(get_log_file prebuild $(bldlogpath))};
test ${BLDTMP:=$(bldtmppath)}
LOG=$(bldlogpath)

HOST="$(uname -n)"				# Machine prebuild is being
						# run on.
if [[ "$HOST" = "landru" ]]
then typeset -r HOST=cmvc_landru		# Use cmvc_landru (direct
else typeset -r HOST				# link to CMVC machines)
fi

typeset -r BLDCHK=${BLDTMP}/prebuild_bldchk.${HOST}.$$
typeset    BLDGLOBALPATH="$(bldglobalpath)"	# Need value of $(bldglobalpath)
						# several times
typeset -r CATALOGS=${BLDTMP}/prebuild_catalogs.${HOST}.$$
typeset -x CMVC_FAMILY				# Current CMVC family being
						# queried.
typeset -x CMVC_RELEASE=""			# Release name.
typeset -r DEVLEVELMEMBERS=${BLDTMP}/prebuild_devlevelmembers.${HOST}.$$
typeset -r DELETE="delete"			# Indicates a selfix file must
						# be deleted if the level
						# commit fails.
typeset -r DELTA_FILE_LIST=${BLDTMP}/prebuild_deltafileslist.${HOST}.$$
						# List of files in the delta
						# directory.
typeset -r DELTA_FILE_LIST_IN_SRC=${BLDTMP}/prebuild_deltafileslistinsrc.${HOST}.$$
						# List of files in the delta
						# directory that exist in the
						# full source tree.
typeset -r ERRLOG=${BLDTMP}/prebuild_errorlog.${HOST}.$$
typeset    EXTRACTHOST				# name of host the extract
						# should be performed on.
typeset    EXTRACTHOST_COLON			# contains ths $EXTRACTHOST
						# variable followed by a
						# colon.  will be NULL if
						# host and extract host are
						# the same machine.
typeset    FAMILIES=""				# List of CMVC families
						# to access.
typeset -r GMASTER_LOGS=prebuild		# grep str for all master logs
typeset -r INTEGRATE=${BLDTMP}/prebuild_integrate.${HOST}.$$
typeset -r LEVEL_TEST_LIST=${BLDTMP}/prebuild_level_list.${HOST}.$$
typeset -r LEVELS_TODO=${BLDTMP}/prebuild_levels.${HOST}.$$
typeset -r LEVELVIEW=${BLDTMP}/prebuild_levelview.${HOST}.$$
typeset -r MAINLOG=$LOGPREBUILD			# master log0
typeset -r MASTER_LOGS=$LOG/prebuild.*		# li str for all master logs
typeset -r MFLMEMBERS=${BLDTMP}/prebuild_mfbmembers.${HOST}.$$
typeset -r MRGLIST=$LOG/merge_list
typeset -r MUSTFIX=${BLDTMP}/prebuild_mustfix.${HOST}.$$
typeset -r NO='[Nn]*'
typeset -r NORETRY="no"                         # attempt to reexecute command
typeset -r NOTMODIFIED="notmodified"		# Indicates a selfix file
						# was not changed by processing
						# the current level.
typeset    OPTARG				# Arguments with current option.
typeset    OPTIND				# Index to next option.
typeset -x PTF_UPDATE=${PTF_UPDATE:-"yes"}      # Set to "No" if Create_Selfix
                                                # _Data should not be run.
                                                # yes means we are doing an
                                                # update build.
typeset -x PREBUILD_RUN_LEVELCHECK=${PREBUILD_RUN_LEVELCHECK:-"YES"}
                                                # Set to "No" if Level -check
                                                # should not be run.
typeset -x PREBUILD_SAVE_REPLACED_SRC=${PREBUILD_SAVE_REPLACED_SRC:-"YES"}
                                                # Set to "No" if old source
                                                # shouldn't be saved.
typeset -u PRODUCTION_AREAS=${PRODUCTION_AREAS}	# List of areas able to do
						# production  builds.
typeset -r PRODUCTION_BACKUP=${BLDTMP}/prebuild_backup.${HOST}.$$
						# Directory used to store
						# files that are over written
						# by the delta level.  Always
						# restoring source tree if
						# user aborts out of prebuild.
#          PRODUCTION_FILE			# File containing a list of
						# areas able to do production
						# builds, contains the default
						# PRODUCTION_AREAS not set.
typeset    PRODUCTION_FAMILY=""			# List of CMVC families user
						# is a production builder.
typeset    PROMPT=""				# Used with confirm to build up
						# long prompts.
typeset -i RC=0					# Return code.
typeset -r READYLEVELS=${BLDTMP}/prebuild_readylevels.${HOST}.$$
#          RELS					# Command line release arguments
						# value from environment if no
						# command line arguments.
typeset -r RELNAMES="${BLDGLOBALPATH}/releasenames"
typeset -r RESTORE="restore"			# Indicates a selfix file must
						# be restored if the level
						# commit fails.
typeset -r RETRY="yes"                          # attempt to reexecute command
typeset    SKIP_FAMILIES=""			# List of CMVC families that
						# connection failed.  Either
						# the CMVC family is down or
						# host is not set up to access
						# the family.
typeset -r TD_COPY=${BLDTMP}/prebuild_builds2.${HOST}.$$
typeset -r TEMP=${BLDTMP}/prebuild_temp.${HOST}.$$
typeset -r TMPFILE1=${BLDTMP}/prebuild_tmpfile1.${HOST}.$$
typeset -r TMPFILE2=${BLDTMP}/prebuild_tmpfile2.${HOST}.$$
typeset -r TRACKS=${BLDTMP}/prebuild_tracks.${HOST}.$$
typeset    Undo_AFSDirectory=""			# AFS Delta directory
						# created.
typeset    Undo_Level=""			# Level created that can be
						# undone.
typeset    Undo_ProdDirectory=""		# Prod Delta directory
						# created.
typeset    Undo_ProdMerge			# Delta level that has been
						# merged into the production
						# tree.
typeset    Undo_TypeOther_Levels=""		# List of levels changed from
						# type development to other.
typeset    Undo_Work_Visited="${FALSE}"		# Call made to the Undo_Work
						# function.
typeset -r YES='[Yy]*'
typeset    a_option=""				# Command line -a option seen.
typeset -u ans					# User response from prompts,
						# always convert to uppercase
						# for checks.
typeset    bad_options=""			# Option check fails if "true".
typeset    cleanup_hosts=""			# List of hosts that temp
						# directories or files may
						# have to be removed from.
typeset    exit_prebuild=0			# Flag to exit prebuild if
						# value is nonzero.
typeset    f_option=""				# Command line -f option seen.
typeset    levelversion=""			# Level name.
typeset    log_command="prebuild"		# Value to pass to the -c
						# argument of log command.
typeset    option=""				# Command line argument being
						# processed.
#
# The following variables define the filename and modified arrays and indexs
# into them.  Arrays were chosen to simplify process when files need to be
# restored.
#
typeset -r abstractlist=0			# abstractlist file
typeset -r all_defects=1			# all_defects file
typeset -r changeview=2				# changeview file
typeset -r defects=3				# defects file
typeset -r memos=4				# memos file
typeset -r ptfrequisites=5			# ptfrequisites file
typeset -r renamelist=6				# renamelist file
typeset -r symptoms=7				# symptoms file
typeset -r ssXREF=8				# symptoms file
typeset    filename[9]				# Set size of filename array.
typeset    filename[abstractlist]="${BLDGLOBALPATH}/abstractlist"
typeset    filename[all_defects]="${BLDGLOBALPATH}/all_defects"
typeset    filename[memos]="${BLDGLOBALPATH}/memos"
typeset    filename[ptfrequisites]="${BLDGLOBALPATH}/ptfrequisites"
typeset    filename[symptoms]="${BLDGLOBALPATH}/symptoms"
typeset    modified[8]				# Set size of modified array.
typeset    modified[abstractlist]="${NOTMODIFIED}"
typeset    modified[all_defects]="${NOTMODIFIED}"
typeset    modified[changeview]="${NOTMODIFIED}"
typeset    modified[defects]="${NOTMODIFIED}"
typeset    modified[memos]="${NOTMODIFIED}"
typeset    modified[ptfrequisites]="${NOTMODIFIED}"
typeset    modified[renamelist]="${NOTMODIFIED}"
typeset    modified[symptoms]="${NOTMODIFIED}"
typeset    modified[ssXREF]="${NOTMODIFIED}"

#initializethe log functions
logset -c $0 -C $0 -F $MAINLOG

#
# Parse the user supplied options.  Insure that no invalid option
# combinations have been used.
#
# The getopts command stops at any argument not beginning with a - or +;
# however, more options may be in the portion of the command line that
# has not been parsed. I want to detect all errors before starting instead
# of at runtime.  The inner loop keeps getopts parsing the options so long
# as they begin with - or +, the outer loop moves through command line
# arguments not beginning with a - or +.
#
while [ $# -gt 0 ]
do
   while getopts ":a:cf:h:ln:su" option
   do
      case $option in
         a) PRODUCTION_AREAS="${OPTARG}"
            PRODUCTION_FILE=""
            a_option="true";;
	 c) PREBUILD_RUN_LEVELCHECK="No";;
         f) PRODUCTION_FILE="${OPTARG}"
            PRODUCTION_AREAS=""
            f_option="true";;
         h) HOSTSFILE="${OPTARG}";;
         l) print - "${LEVELCHECK_FLAGS}" | \
               egrep -e'\-long$|[${SPACECHARACTER}${TABCHARACTER}]' \
               1> /dev/null 2>&1
            [[ $? -ne 0 ]] && LEVELCHECK_FLAGS="${LEVELCHECK_FLAGS} -long";;
         n) NODENAMES="${OPTARG}";;
	 s) PREBUILD_SAVE_REPLACED_SRC="No";;
	 u) PTF_UPDATE="No";;
         :) log +l -e "Argument must be supplied with -${OPTARG}"
            bad_options="true";;
        \?) log +l -e "Unknown option -${OPTARG}"
            bad_options="true";;
         *) echo $option ;;
      esac
   done
   if [ ${OPTIND} -eq 1 ]
   then
      # getopts did not find option, assume rest of arguments are release
      # names
      [[ -z "${RELS}" ]] && RELS="$*"
      shift 1
   else
      # getopts found an option, shift off all data processed
      shift ${OPTIND}-1
      OPTIND=1
   fi
done

[[ "${a_option}" = "true" && "${f_option}" = "true" ]] && \
{
   log +l -e "Options -a and -f cannot both be supplied"
   bad_options="true"
}
[[ -n "${PRODUCTION_FILE}" && -n "${PRODUCTION_AREAS}" ]] && \
{
   log +l -e "PRODUCTION_FILE and PRODUCTION_AREAS cannot both be set"
   bad_options="true"
}

[[ -z "${PRODUCTION_AREAS}" ]] && \
{
   if [[ -f ${PRODUCTION_FILE} ]]
   then
      PRODUCTION_AREAS=`sed -e "/#.*$/d" -e "/^$/d" ${PRODUCTION_FILE}`
   else
      log +l -e "File ${PRODUCTION_FILE} does not exist"
      bad_options="true"
   fi
}

bldhostsfile_checkfile ${TRUE}
[[ $? -ne 0 ]] && bad_options="true"
bldnodenames_checkfile ${TRUE}
[[ $? -ne 0 ]] && bad_options="true"

[[ "${bad_options}" = "true" ]] && Usage

log "################ prebuild start: #################"

if [[ -n "${RELS}" ]]
then
   for release in ${RELS}""
   do
      bldhostsfile ${release} "${TRUE}"
      if [[ $? -eq 0 ]]
      then
         # Is HOSTSFILE_CMVCFAMILY in FAMILIES, if not add it.
         if [[ "${FAMILIES}" = "${FAMILIES#*${HOSTSFILE_CMVCFAMILY}}" ]]
         then
            if [[ -z "${FAMILIES}" ]]
            then
               FAMILIES="${HOSTSFILE_CMVCFAMILY}"
            else
               FAMILIES="${FAMILIES} ${HOSTSFILE_CMVCFAMILY}"
            fi
         fi
      fi
   done
else
   bldhostsfile_search ${SEARCH_FIRST} "${TRUE}"
   if [[ $? -eq 0 ]]
   then
      FAMILIES="${HOSTSFILE_CMVCFAMILY}"
      while [[ -n "${HOSTSFILE_RELEASE}" ]]
      do
         # Is HOSTSFILE_CMVCFAMILY in FAMILIES, if not add it.
         if [[ "${FAMILIES}" = "${FAMILIES#*${HOSTSFILE_CMVCFAMILY}}" ]]
         then
            FAMILIES="${FAMILIES} ${HOSTSFILE_CMVCFAMILY}"
         fi
         bldhostsfile_search ${SEARCH_NEXT} "${TRUE}"
         [[ $? -ne 0 ]] && break
      done
   fi
   bldhostsfile_search ${SEARCH_STOP} "${TRUE}"
fi

# Are they a production builder
for CMVC_FAMILY in ${FAMILIES}""
do
   # builder - if not null, user is capable of doing production builds.
   builder=`Report -view Users \
                   -where "login = '${CMVC_BECOME}' and
                           upper(area) in (${PRODUCTION_AREAS})" \
                   -raw 2> /dev/null`
   if [[ $? -ne 0 ]]
   then
      # Cannot connect to CMVC server, this system probably not configured
      # for this family.
      log -f "Connect failed to family ${CMVC_FAMILY}"
      if [[ -z "${SKIP_FAMILIES}" ]]
      then
         SKIP_FAMILIES="${CMVC_FAMILY}"
      else
         SKIP_FAMILIES="${SKIP_FAMILIES} ${CMVC_FAMILY}"
      fi
   elif [[ -z "${builder}" ]]
   then
      PROMPT="\tYou are NOT a production builder in family ${CMVC_FAMILY}."
      PROMPT="${PROMPT}\n\tDo you wish to continue? (y/n/q) \c"
      confirm -r "${PROMPT}"
      RC=$?
      if [[ ${RC} -ne 0 ]]
      then
         log "User chose to exit because they were not a production builder"
         RC=1
         exit 1
      fi
   else
      PRODUCTION_FAMILY="${PRODUCTION_FAMILY} ${CMVC_FAMILY}"
   fi
done

if [[ "${FAMILIES}" = "${SKIP_FAMILIES}" ]]
then
   log +l -x "Cannot connect to any CMVC families -- Check CMVC_BECOME"
fi

# if automatic flag is set, set related flag to yes unless they are already set
if [[ -n "$AUTO" ]]
then
   [[ -z "$LMERGE" ]] && LMERGE=Y
   [[ -z "$LSSMRG" ]] && LSSMRG=Y
   [[ -z "$LCOMMIT" ]] && LCOMMIT=Y
fi

Check_Phases

typeset -u REPLY BCOMMIT BCONT
chksetlevelname

#################################### Main #####################################

# Write header information.
print;
[[ -n "$BUILDER" ]] && print Builder: $BUILDER

### Get all CMVC levels which are "ready"
if [ -n "$VERSION" ] ; then
   relquery="and releasename like '%$VERSION'"
else
   relquery=""
fi
log "Querying CMVC for levels"
rm -f ${READYLEVELS}
touch ${READYLEVELS}
for CMVC_FAMILY in ${FAMILIES}""
do
   # Is CMVC_FAMILY in SKIP_FAMILIES, if not query CMVC family for levels.
   if [[ "${SKIP_FAMILIES}" = "${SKIP_FAMILIES#*${CMVC_FAMILY}}" ]]
   then
      Report -view LevelView \
             -where "userLogin = '${BLDOWNER}' $relquery and \
                     type in ('production','development') and \
                     commitdate is NULL" \
             -raw 2> /dev/null | \
      awk ' BEGIN    { FS = "|" }
            { print $1, $2, $3 } ' | \
      FilterLevels "$LNAMES" >> $READYLEVELS
   fi
done

if [[ -z "$RELS" ]]  
then
   # no releases specified on command line so get ready releases
   cp $READYLEVELS $LEVELS_TODO
else
   # releases specified on command line so get only those releases.
   for release in $RELS
   do
      grep -w $release $READYLEVELS >> $LEVELS_TODO
      if [[ $? -ne 0 ]]
      then
         log +l -e "Release ${release} skipped: not found in correct state"
         log +l -e "in a development level.  Confirm correct name was used"
      fi
   done
fi

# Now that initial data is gathered, present user with menu.  Return and
# continue after the user has selected the Levels to process.
Prebuild_Menu $LEVELS_TODO
[[ $? -ne 0 ]] && \
{
   RC=0
   exit 0
}

### Combine development level names belonging to same release
Combine_Level_Names $LEVELS_TODO

### Check directories, make sure we have write access.
if [[ -n "${PRODUCTION_BUILDER}" ]]
then
   log "Verifying write access"
   while read CMVC_RELEASE leveltype levelnames
   do
      bldhostsfile ${CMVC_RELEASE} ${TRUE}
      if [[ $? -ne 0 ]]
      then
         log +l -e "Skipping ${CMVC_RELEASE}, bad ${HOSTSFILE} entry"
         continue
      fi
      EXTRACTHOST=`print ${HOSTSFILE_HOST} | tr "[A-Z]" "[a-z]"`
      [[ "${HOST}" != "${EXTRACTHOST}" ]] && EXTRACTHOST_COLON="${EXTRACTHOST}:"
      Verify_Write_Access
      [[ $? -eq 1 ]] && exit_prebuild=1
      [[ $? -eq 2 ]] && { RC=1; exit 1; }
      # trap on exit above will call Undo_Work and Clean_Up.
   done < $LEVELS_TODO
   [[ ${exit_prebuild} -eq 1 ]] && { RC=1; exit 1; }
   # trap on exit above will call Undo_Work and Clean_Up.
fi

### Now process the levels by release
while read CMVC_RELEASE leveltype levelnames
do
   # To have entered the Undo_Work function user responded with q to
   # some confirm prompt.  Determine here if they just wanted to quit
   # working on current release or completely exit prebuild.
   if [[ "${Undo_Work_Visited}" = "${TRUE}" ]]
   then
      confirm -n -r "\tDo you want to exit prebuild? (y/N/q) \c"
      RC=$?
      if [[ ${RC} -ne 1 ]]
      then
         log "User chose to exit prebuild"
         RC=1
         exit 1
      fi
   fi
   Undo_Work_Visited="${FALSE}"
   Undo_Level=""
   Undo_AFSDirectory=""
   Undo_ProdDirectory=""
   Undo_ProdMerge=""

   bldinit prebuild   # Sets up the required directories, mounts etc  

   log "Processing ${CMVC_RELEASE}"
   log "Starting time: $(date)"

   bldhostsfile ${CMVC_RELEASE} ${TRUE}
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Skipping ${CMVC_RELEASE}, bad ${HOSTSFILE} entry"
      continue
   }
   EXTRACTHOST=`print ${HOSTSFILE_HOST} | tr "[A-Z]" "[a-z]"`
   if [[ "${HOST}" != "${EXTRACTHOST}" ]]
   then
      EXTRACTHOST_COLON="${EXTRACTHOST}:"
   else
      EXTRACTHOST_COLON=""
   fi
   if [[ "${cleanup_hosts}" = "${cleanup_hosts#*${EXTRACTHOST}}" ]]
   then
      if [[ -z "${cleanup_hosts}" ]]
      then
         cleanup_hosts="${EXTRACTHOST}"
      else
         cleanup_hosts="${cleanup_hosts} ${EXTRACTHOST}"
      fi
   fi
   CMVC_FAMILY=${HOSTSFILE_CMVCFAMILY}

   # Is HOSTSFILE_CMVCFAMILY in PRODUCTION_FAMILY, if so build type will be
   # production.
   if [[ "${PRODUCTION_FAMILY}" != \
         "${PRODUCTION_FAMILY#*${HOSTSFILE_CMVCFAMILY}}" ]]
   then
      LTYPE=production
   else
      LTYPE=development
      LSSMRG=N
      LCOMMIT=N
   fi

   if [[ "$leveltype" = "development" ]]
   then
      Create_Production_Level "${levelnames}"
      RC=$?
   else
      Process_Production_Level "${levelnames}"
      RC=$?
   fi
   [[ ${RC} -ne 0 ]] && { Undo_Work; continue; }

   # store the new level name; used for level test and prebuild menu item
   print $levelversion ${CMVC_RELEASE} >> $LEVEL_TEST_LIST
 
   # Did user want to only create the CMVC level.
   [[ -n "$LDONLY" ]] && continue

   Check_Prereq
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Extract_Level
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Check_Makefiles
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Merge_Source_Production
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Merge_Source_Build
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Merge_Source_AFS
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Create_Selfix_Data
   [[ $? -ne 0 ]] && { Undo_Work; continue; }

   Commit_Level
   [[ $? -eq 1 ]] && { Undo_Work; continue; }
   [[ $? -eq 2 ]] && { Restore_Selfix_Data; continue; }

   Restore_Selfix_Data remove_backup_files

   log "Finishing time: $(date)"

   logset -r
done < $LEVELS_TODO

# Prevent Undo_Work from trying to undo any work.
Undo_Work_Visited="${TRUE}"
