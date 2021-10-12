#! /bin/ksh
# @(#)54	1.30  src/bldenv/bldtools/rename/bldafsmerge.sh, bldprocess, bos412, GOLDA411a 8/18/93 09:43:12
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldafsmerge
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: Clean_Up
#
# FUNCTION: Clean up after running.
#
# INPUT: None.
#
# OUTPUT: None.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#
function Clean_Up
{
   logset -r
   File_Control_Bldafsmerge ${CLOSE} ${REMOVE}
   File_Control_V3bld ${CLOSE} ${REMOVE}
   File_Control_Prebuild ${CLOSE} ${REMOVE}
   rm -f ${work_dir}/bldafsmerge_level1_$$ ${work_dir}/bldafsmerge_level2_$$
   [[ -d "${work_dir}" ]] && rmdir ${work_dir}
   return 0
}

#
# NAME: Create_Merge_Entry
#
# FUNCTION: Create a merge entry for a release.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#        RELEASE (global) - Release name.
#        dont_merge_data[] (global) - Types user has chosen not to merge.
#        bldafsmerge_types(global) - Types of merges already complete.
#        merge_bldcycle[] (global) - Build Cycle of merge item.
#        merge_release[] (global) - Release of merge item.
#        merge_data[] (global) - Types user has chose to merge.
#        merge_size (global) - Number of entries in the merge arrays.
#
# OUTPUT: 
#
# SIDE EFFECTS: 
#
# RETURNS:
#
function Create_Merge_Entry
{
   typeset    data
   typeset    dont_merge
   typeset    check
   typeset    merge_type

   # Initially data is set to all merges.
   data="${MERGE_TYPES}"

   merge_bldcycle[${merge_size}]="${BLDCYCLE}"
   merge_release[${merge_size}]="${RELEASE}"

   # Remove merges from data that have already occured.
   [[ -n "${merge_types}" ]] && \
      for merge_type in ${merge_types}""
      do
         data="$(Modify_Options delete ${merge_type} ${data})"
      done

   # At this point ${data} contains merges that need to occur.

   # Process the command line options.  Set up both ${data} and
   # ${dont_merge} to all merges that still need to be done.
   # Then for each merge that still needs to occur, look at the command
   # line option for that merge; if ${TRUE} then keep in merge list, if
   # ${FALSE} keep in dont merge list.
   set "${MERGE_BLDENV}" "${MERGE_DELTA}" "${MERGE_OBJECT}" "${MERGE_PTF}" \
       "${MERGE_SHIP}"
   for merge_type in ${MERGE_TYPES}
   do
      # Determine if merge should occur.
      In_List ${merge_type} ${data}
      if [[ $? -eq 0 ]]
      then
         # If command line option is false, keep entry in merge list, otherwise
         # leave entry in dont merge list.
         if [[ "$1" != "${TRUE}" ]]
         then
            data="$(Modify_Options delete ${merge_type} ${data})"
            dont_merge="$(Modify_Options add ${merge_type} ${dont_merge})"
         fi
      fi
      shift 1
   done

   merge_data[${merge_size}]=${data}
   dont_merge_data[${merge_size}]=${dont_merge}
   (( merge_size = ${merge_size} + 1 ))
}

#
# NAME: Create_File
#
# FUNCTION: Generic function to issure a query and write the output to
#           a file.
#
# INPUT: query ($1) - The query function to process data from.
#        output_file ($2) - The file to write the data to.
#
# OUTPUT: None.
#
# SIDE EFFECTS: Creates the output_file.
#
# RETURNS: 0 always.
#
function Create_File
{
   typeset    bldcycle			# Build cycle from query.
   typeset    old_bldcycle=""		# Build cycle from previous query
					# data.
   typeset    old_release=""		# Release from previous query data.
   typeset    output_file=$2
   typeset -i records=0			# Number of unique variable fields
					# seen
   typeset    release			# Release from query.
   typeset    query=$1
   typeset    var_field			# Variable field from query.
   typeset    var_field_list		# Built up list of variable field
					# from queries with same build cycles
					# and release.

   (${query}) \
   | sort -u -t\| +0 -1 +1 -2 +2 -3 \
   | (IFS="|"; \
     while read bldcycle release var_field; \
     do \
        if [[ "${bldcycle}" = "${old_bldcycle}" && \
              "${release}" = "${old_release}" ]]; \
        then \
           var_field_list="${var_field} ${var_field_list}"; \
           (( records = ${records} + 1 )); \
        else \
           [[ -n "${old_bldcycle}" ]] && \
              print ${old_bldcycle} ${old_release} ${records} ${var_field_list}; \
           var_field_list="${var_field}"; \
           records=1; \
           old_bldcycle=${bldcycle}; \
           old_release=${release}; \
        fi; \
     done; \
     [[ -n "${old_bldcycle}" ]] && \
        print ${old_bldcycle} ${old_release} ${records} ${var_field_list}) \
   > ${output_file}

   return 0
}

#
# NAME: Display_Menu_Entry
#
# FUNCTION: Display merge record on screen.
#
# INPUT: choice ($1) - What user identifies this record as.
#        menu_index ($2) - Index into the merge records.
#
# OUTPUT: Display menu entry on stdout.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#
function Display_Menu_Entry
{
   typeset -R3 choice=$1
   typeset    display_line
   typeset    data
   typeset -i menu_index=$2
   typeset -L6 bldcycle=${merge_bldcycle[${menu_index}]}
   typeset -L15 release=${merge_release[${menu_index}]}

   display_line="  ${choice}   ${bldcycle} ${release}"
   set "BldEnv" "Delta" "NLS" "PTF" "Ship"
   for data in ${MERGE_TYPES}
   do
      In_List ${data} ${merge_data[${menu_index}]}
      [[ $? -eq 0 ]] && display_line="${display_line} $1"
      In_List ${data} ${dont_merge_data[${menu_index}]}
      [[ $? -eq 0 ]] && display_line="${display_line} ($1)*"
      shift 1
   done
   print "${display_line}"

   return 0
}

#
# NAME: Execute
#
# FUNCTION: Execute or Display commands supplied
#
# INPUT: SHOW_MODE (global) - if ${TRUE} do not execute commands, just
#                             display them
#        host ($1) - remote host to execute commands on.  If null current
#                    host is used.  If a remote host is used, the commands
#                    passed should always print back the return code.
#        commands ($2-$#) - commands to be run.
#
# OUTPUT: none.
#
# SIDE EFFECTS: None.
#
# RETURNS: return status from command executed, if SHOW_MODE is ${TRUE}
#          return status is always 0.
#
function Execute
{
   typeset -i RC=0			# Return code.
   typeset -i command_RC=0		# Return code from remote command.
					# Assume all remote commands print
					# back their status.
   typeset command			# Command to issue.

   if [[ "$1" != "" ]]
   then
      command="rsh $1 -l build"
      shift 1
      options="\"$*\""
   else
      shift 1
      command="$1"
      shift 1
      options="$*"
   fi

   if [[ "${SHOW_MODE}" = "${TRUE}" ]]
   then
      print - "***** ${command} ${options}"
      RC=0
   else
      command_RC=$(eval ${command} ${options})
      (( RC = $? | ${command_RC} ))
   fi

   return ${RC}
}

#
# NAME: File_Control_Bldafsmerge
#
# FUNCTION: Create, Open, Close, Read and Remove the bldafsmerge data file.
#
# INPUT: BLDAFSMERGE_RECORDS (global) - The bldafsmerge file.
#        option ($*) - actions to perform on the file.
#
# OUTPUT: bldafsmerge_bldcycle (global) - Build cycle read.
#         bldafsmerge_data (global) - If 'used' a read must occur before
#				      using the data.
#         bldafsmerge_entries (global) - Number of merges complete.
#         bldafsmerge_release (global) - Release read.
#         bldafsmerge_types (global) - List of merges already complete.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if current working build cycle and release match the build
#          cycle and release in the file, 1 otherwise.
#
function File_Control_Bldafsmerge
{
   typeset -i RC=0			# Return code.
   typeset    option			# Action to perform on file.

   for option in $*
   do
      case "${option}" in
         "${CLOSE}" )
            exec 4<&-
            ;;
         "${CREATE}" )
            Create_File Query_Bldafsmerge ${BLDAFSMERGE_RECORDS}
            ;;
         "${MATCH}" )
            if [[ "${bldafsmerge_data}" = "used" ]]
            then
               read -u4 bldafsmerge_bldcycle bldafsmerge_release \
                        bldafsmerge_entries bldafsmerge_types
               bldafsmerge_data="unused"
            fi
            if [[ "${BLDCYCLE}" = "${bldafsmerge_bldcycle}" && \
                  "${RELEASE}" = "${bldafsmerge_release}" ]]
            then
               bldafsmerge_data="used"
            else
               RC=1
            fi
            ;;
         "${OPEN}" )
            exec 4< ${BLDAFSMERGE_RECORDS}
            read -u4 bldafsmerge_bldcycle bldafsmerge_release \
                     bldafsmerge_entries bldafsmerge_types
            bldafsmerge_data="unused"
            ;;
         "${REMOVE}" )
            rm -f ${BLDAFSMERGE_RECORDS} 1> /dev/null 2>&1
            ;;
      esac
   done

   return ${RC}
}

#
# NAME: File_Control_Prebuild
#
# FUNCTION: Create, Open, Close, Read and Remove the prebuild data file.
#
# INPUT: PREBUILD_RECORDS (global) - The prebuild file.
#        option ($*) - actions to perform on the file.
#
# OUTPUT: prebuild_bldcycle (global) - Build cycle read.
#         prebuild_data (global) - If 'used' a read must occur before
#		      		   using the data.
#         prebuild_entries (global) - Number of levels processed
#         prebuild_levels (global) - List of levels processed.
#         prebuild_release (global) - Release read.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if current working build cycle and release match the build
#          cycle and release in the file, 1 otherwise.
#
function File_Control_Prebuild
{
   typeset -i RC=0			# Return code.
   typeset    option			# Action to perform on file.

   for option in $*
   do
      case "${option}" in
         "${CLOSE}" )
            exec 5<&-
            ;;
         "${CREATE}" )
            Create_File Query_Prebuild ${PREBUILD_RECORDS}
            ;;
         "${MATCH}" )
            if [[ "${prebuild_data}" = "used" ]]
            then
               read -u5 prebuild_bldcycle prebuild_release prebuild_entries \
                        prebuild_levels
               prebuild_data="unused"
            fi
            if [[ "${BLDCYCLE}" = "${prebuild_bldcycle}" && \
                  "${RELEASE}" = "${prebuild_release}" ]]
            then
               prebuild_data="used"
            else
               RC=1
            fi
            ;;
         "${OPEN}" )
            exec 5< ${PREBUILD_RECORDS}
            read -u5 prebuild_bldcycle prebuild_release prebuild_entries \
                     prebuild_levels
            prebuild_data="unused"
            ;;
         "${READ}" )
            while [[ ( "${BLDCYCLE}" != "${prebuild_bldcycle}" ) || \
                     ( "${RELEASE}" != "${prebuild_release}" ) ]]
            do
               read -u5 prebuild_bldcycle prebuild_release prebuild_entries \
                        prebuild_levels
            done
            if [[ ( "${BLDCYCLE}" != "${prebuild_bldcycle}" ) || \
                  ( "${RELEASE}" != "${prebuild_release}" ) ]]
            then
               RC=1
            fi
            ;;
         "${REMOVE}" )
            rm -f ${PREBUILD_RECORDS} 1> /dev/null 2>&1
            ;;
      esac
   done

   return ${RC}
}

#
# NAME: File_Control_V3bld
#
# FUNCTION: Create, Open, Close, Read and Remove the v3bld data file.
#
# INPUT: PREBUILD_RECORDS (global) - The v3bld file.
#        option ($*) - actions to perform on the file.
#
# OUTPUT: prebuild_bldcycle (global) - Build cycle read.
#         prebuild_data (global) - If 'used' a read must occur before
#		      		   using the data.
#         prebuild_entries (global) - Number of levels processed
#         prebuild_levels (global) - List of levels processed.
#         prebuild_release (global) - Release read.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if current working build cycle and release match the build
#          cycle and release in the file, 1 otherwise.
#
function File_Control_V3bld
{
   typeset -i RC=0			# Return code.
   typeset    option			# Action to perform on file.

   for option in $*
   do
      case "${option}" in
         "${CLOSE}" )
            exec 6<&-
            ;;
         "${CREATE}" )
            Create_File Query_V3bld ${V3BLD_RECORDS}
            ;;
         "${MATCH}" )
            if [[ "${v3bld_data}" = "used" ]]
            then
               read -u6 v3bld_bldcycle v3bld_release v3bld_entries v3bld_levels
               v3bld_data="unused"
            fi
            if [[ "${BLDCYCLE}" = "${v3bld_bldcycle}" && \
                  "${RELEASE}" = "${v3bld_release}" ]]
            then
               v3bld_data="used"
            else
               RC=1
            fi
            ;;
         "${OPEN}" )
            exec 6< ${V3BLD_RECORDS}
            read -u6 v3bld_bldcycle v3bld_release v3bld_entries v3bld_levels
            v3bld_data="unused"
            ;;
         "${REMOVE}" )
            rm -f ${V3BLD_RECORDS} 1> /dev/null 2>&1
            ;;
      esac
   done

   return ${RC}
}

#
# NAME: Get_Bldcycle_Release
#
# FUNCTION: Get all build cycle and release pairs from status file in
#           sorted order.
#
# INPUT: None.
#
# OUTPUT: All build cycle and releases displayed to stdout.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
function Get_Bldcycle_Release
{
   typeset    bldcycle		# Build cycle.
   typeset    release		# Release

   QueryStatus $_TYPE "$_DONTCARE" \
               $_LEVEL "$_DONTCARE" \
               $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
               $_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
               $_SUBTYPE "$_DONTCARE" $_PHASE "$_DONTCARE" \
   | sort -u -t\| +0 -1 +1 -2 \
   | (IFS="|"; while read bldcycle release; \
               do \
                  print "${bldcycle} ${release}"; \
               done)

   return 0
}

#
# NAME: In_List
#
# FUNCTION:
#
# INPUT: element ($1) - Element to search list for.
#        list ($2 - $n) - List to search.
#
# OUTPUT: none
#
# SIDE EFFECTS: none.
#
# RETURNS: - 0 if element found in list, 1 if element not found in list.
#
function In_List
{
   typeset    element=$1		# Element to search list for.
   typeset    list			# List to be searched.

   shift 1
   list=$*

   for list_element in ${list}
   do
      [[ "${element}" = "${list_element}" ]] && return 0
   done

   return 1
}

#
# NAME: Is_Bldcycle_Closed
#
# FUNCTION: Determine if the build cycle is closed.
#
# INPUT: BLDCYCLE (global) - Build Cycle.
#
# OUTPUT: None.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if build cycle closed, 1 otherwise.
#
function Is_Bldcycle_Closed
{
   typeset -i entries=0			# Total number of closed build cycle
					# records.

   entries=$(QueryStatus $_TYPE "$_DONTCARE" $_BLDCYCLE ${BLDCYCLE} \
                         $_RELEASE "$_DONTCARE" $_LEVEL "$_DONTCARE" \
                         $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
                         $_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
                         $_SUBTYPE "$_DONTCARE" $_PHASE ${PHASE_CLOSED} \
                         -A \
           | wc -l)

   [[ ${entries} -eq 0 ]] && return 1

   return 0
}

#
# NAME: Menu
#
# FUNCTION: Process the display of the merge list.  The merge list contains
#           the data that will be written to afs.
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: 
#
# RETURNS: 0 for success, 1 for exit.
#

function Menu
{
   typeset    action
   typeset    bad_option="${FALSE}"
   typeset -i bottom_line
   typeset -R3 current_line
   typeset    handle_action=""
   typeset    option_for_menu
   typeset    sub_action=""

   while [ -z "${handle_action}" ]
   do
      current_line=${merge_index}
      ((bottom_line=${current_line}+${DATA_SIZE}-1))
      clear
      print "\n    Build Cycles to be Merged (${merge_size} Total)\n"
      print " Index  Cycle   Release          Merges Required  ()* - Don't Merge\n"
      while [ ${current_line} -le ${bottom_line} ]
      do 
         if [ ${current_line} -lt ${merge_size} ]
         then
            ((display_line=${current_line}+1))
            print "$(Display_Menu_Entry ${display_line} ${current_line})"
         else
            print
         fi
         ((current_line=${current_line}+1))
      done
      print 
      print "Modify (BL)dEnv, Modify (DE)lta, Modify (NL)S, \c"
      print "Modify (PT)F, Modify (SH)ip,"
      print "(B)egin, (E)xit, (N)ext Page, (P)revious Page"
      print "Actions? \c"
      read action
      option_for_menu=""
      for sub_action in ${action}""
      do
         case ${sub_action} in
            [Bb][Ll] | [Bb][Ll][Dd][Ee][Nn][Vv] )
               option_for_menu="${option_for_menu} $S_BLDENVMERGE"
               ;;
            [Dd][Ee] | [Dd][Ee][Ll][Tt][Aa] )
               option_for_menu="${option_for_menu} $S_DELTAMERGE"
               ;;
            [Nn][Ll] | [Nn][Ll][Ss])
               option_for_menu="${option_for_menu} $S_OBJECTMERGE"
               ;;
            [Pp][Tt] | [Pp][Tt][Ff] )
               option_for_menu="${option_for_menu} $S_PTFMERGE"
               ;;
            [Ss][Hh] | [Ss][Hh][Ii][Pp] )
               option_for_menu="${option_for_menu} $S_SHIPMERGE"
               ;;
            [Nn] | [Nn][Ee][Xx][Tt] )
               ((tmp=${merge_index}+DATA_SIZE))
               [[ ${tmp} -lt ${merge_size} ]] && merge_index=${tmp}
               ;;
            [Pp] | [Pp][Rr][Ee][Vv][Ii][Oo][Uu][Ss] )
               ((tmp=${merge_index}-DATA_SIZE))
               if [[ ${tmp} -ge 0 ]]
               then
                  merge_index=${tmp}
               else
                  merge_index=0
               fi
               ;;
            [Bb] | [Bb][Ee][Gg][Ii][Nn] ) 
               handle_action="Begin"
               ;;
            [Ee] | [Ee][Xx][Ii][Tt] | \
            [Qq] | [Qq][Uu][Ii][Tt] ) 
               handle_action="Exit"
               ;;
            * )
               print "${sub_action} Illegal Action.\nEnter to continue: \c"
               bad_option="${TRUE}"
               read continue
               ;;
         esac
      done
      if [[ "${bad_option}" = "${FALSE}" ]]
      then
         [[ -n "${option_for_menu}" ]] && Menu_Option ${option_for_menu}
      else
         bad_option="${FALSE}"
      fi
   done

   [[ "${handle_action}" = "Exit" ]] && return 1

   return 0
}

#
# NAME: Menu_Option
#
# FUNCTION: Called when user selects to modify data displayed on the menu
#           screen.
#
# INPUT: merge_types ($*) - Data field that should be modified.
#
# OUTPUT: dont_merge_data[] (global) -
#         merge_data[] (global) -
#         merge_size (global) - 
#
# SIDE EFFECTS: 
#
# RETURNS:
#
function Menu_Option
{
   typeset -i index=0
   typeset    index_list=0
   typeset    merge_type
   typeset    merge_types=$*
   typeset    selection

   print "(M)erge, (D)onot merge, other response will return? \c"
   read selection
   [[ -z "${selection}" ]] && return 0
   print "Index [all or <range i.e. 1,2-4,6,8-10,15,...> ] (all)? \c"
   read index_list
   [[ -z "${index_list}" ]] && index_list="all"

   case ${selection} in
      [Dd] | [Dd][Oo][Nn][Oo][tt] )
         for merge_type in ${merge_types}""
         do
            Translate 1 ${merge_size} ${index_list} | sort -u | while read index
            do 
               (( index = index -1 ))
               In_List ${merge_type} ${merge_data[${index}]}
               if [[ $? -eq 0 ]]
               then
                  dont_merge_data[${index}]="$(Modify_Options add ${merge_type} ${dont_merge_data[${index}]})"
                  merge_data[${index}]="$(Modify_Options delete ${merge_type} ${merge_data[${index}]})"
               fi
            done
         done
            ;;
      [Mm] | [Mm][Ee][Rr][Gg][Ee] )
         for merge_type in ${merge_types}""
         do
            Translate 1 ${merge_size} ${index_list} | sort -u | while read index
            do
               (( index = index -1 ))
               In_List ${merge_type} ${dont_merge_data[${index}]}
               if [[ $? -eq 0 ]]
               then
                  merge_data[${index}]="$(Modify_Options add ${merge_type} ${merge_data[${index}]})"
                  dont_merge_data[${index}]="$(Modify_Options delete ${merge_type} ${dont_merge_data[${index}]})"
               fi
            done
         done
         ;;
      * )
         ;;
   esac

   return 0
}

#
# NAME: Merge_Bldenv
#
# FUNCTION: Merge the build environment to AFS.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#
# OUTPUT: generic_AFS_logfile (GLOBAL) - Data log file in afs.
#         generic_descripition (GLOBAL) - English descripition of the merge
#         generic_destination (GLOBAL) - List of destination directories.
#         generic_host_logfile (GLOBAL) - Data log file on host machine.
#         generic_source (GLOBAL) - List of host directories.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if no errors, <>0 if errors.
#
function Merge_Bldenv
{
   typeset    subdir			# Loop variable

   bldhostsfile bldafsmerge_bldenv "${TRUE}"
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Cannot continue with build environment merge"
      return 1
   }

   generic_source=""
   generic_destination=""
   for subdir in $(bldhostsfile_search_data1 SUBDIRS "${TRUE}")
   do
      generic_source="${generic_source} ${HOSTSFILE_PROD}/${subdir}"
      generic_destination="${generic_destination} ${HOSTSFILE_AFSPROD}/${subdir}"
   done
   generic_host_logfile=${HOSTSFILE_PROD}/nlsdeltaflag
   generic_descripition="build environment"
   generic_AFS_logfile=${HOSTSFILE_AFSPROD}/LEVELS.bldenv

   Merge_Generic
   return $?
}

#
# NAME: Merge_Delta
#
# FUNCTION: Merge the delta source trees in AFS.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#        RELEASE (global) - Release name.
#
# OUTPUT: None.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 if no errors, <>0 if errors.
#
function Merge_Delta
{
   typeset -i RC=0
   typeset    level_name
   typeset    host_colon=""

   bldhostsfile ${RELEASE} "${TRUE}"
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Cannot continue with delta source merge"
      return 1
   }

   [[ "${HOSTSFILE_AFSPROD}" = "*" || \
      "${HOSTSFILE_AFSDELTA}" = "*" ]] && \
   {
      log +l "Delta Source merges are not done for ${RELEASE}"
      return 0
   }

   [[ ! -w "${HOSTSFILE_AFSPROD}" ]] && 
   {
      log +l -e "Do not have write access to ${HOSTSFILE_AFSPROD}"
      return 1
   }

   File_Control_Prebuild ${READ}
   [[ $? -ne 0 ]] && log +l -e "File_Control_Prebuild ${READ} returned NULL"
   if [[ -z "${prebuild_levels}" ]]
   then
      log +l -w "No level names found for ${RELEASE} in ${BLDCYCLE}"
      return 0
   fi

   (for level_name in ${prebuild_levels}
    do
       print ${level_name} 
    done) | sort -u > ${work_dir}/bldafsmerge_level1_$$

   [[ ${HOSTSFILE_HOST} != ${hostname} ]] && host_colon="${HOSTSFILE_HOST}:"

   # If we find a levelname out in AFS that does not exist in the status
   # file stop delta merges.  User will have to either remove the level
   # from AFS or update the status file.
   if [[ -f ${HOSTSFILE_AFSDELTA}/${BLDCYCLE} ]]
   then
      RC =0
      for level_name in $(cat ${HOSTSFILE_AFSDELTA}/${BLDCYCLE})
      do
         if [[ "${prebuild_levels}" = "${prebuild_levels#*${level_name}}" ]]
         then
               log +l -e "Level ${level_name} in ${HOSTSFFILE_AFSDELTA}/${BLDCYCLE}"
            log +l -e "was not found in ${STATUS_FILE}"
            RC=1
         fi
      done
      if [[ ${RC} -ne 0 ]] 
      then
         log +l -e "Cannot continue delta merges for ${CMVC_RELEASE}"
         return 1
      fi
   fi

   for level_name in $(cat ${work_dir}/bldafsmerge_level1_$$)
   do
      delta_build_dir="${HOSTSFILE_DELTA}/${level_name}"
      if [[ ! -d "${delta_build_dir}" ]]
      then
         log +l -e "${delta_build_dir} not accessible or non-existent"
         return 1
      fi
      log +l -b "Merging delta tree for $release level $level_name"
      Execute "" treecopy ${host_colon}$delta_build_dir ${HOSTSFILE_AFSPROD} \
                 "1>" /dev/null
      if [[ $? -eq 0 ]]
      then
         if [[ -f ${HOSTSFILE_AFSPROD}/.gone ]]
         then
            Execute "" "cd ${HOSTSFILE_AFSPROD}; xargs -n100 rm <.gone; cd ~-"
            Execute "" mv ${HOSTSFILE_AFSPROD}/.gone \
                          ${HOSTSFILE_AFSPROD}/.gone$level_name
         fi
      else
         log +l -e "merging $level_name for ${RELEASE}"
         return 1
      fi
      mrgmsg="$release production merge for $level_name ($(date))"
      Execute "" "print \"$mrgmsg\" >> ${HOSTSFILE_AFSPROD}/../LEVELS.${RELEASE}"
      Execute "" rm -fr ${HOSTSFILE_AFSDELTA}/${level_name}
   done
   Execute "" rm -f ${HOSTSFILE_AFSDELTA}/${BLDCYCLE}

   rm -f ${work_dir}/bldafsmerge_level1_$$

   return 0
}

#
# NAME: Merge_Generic
#
# FUNCTION: Generic merge function.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#        generic_AFS_logfile (GLOBAL) - Data log file in afs.
#        generic_descripition (GLOBAL) - English descripition of the merge
#        generic_destination (GLOBAL) - List of destination directories.
#        generic_host_logfile (GLOBAL) - Data log file on host machine.
#        generic_source (GLOBAL) - List of host directories.
#
# OUTPUT: 
#
# SIDE EFFECTS: logs messages into generic_host_logfile and
#               generic_AFS_logfile.
#
# RETURNS: 0 on success, 1 on failure.
#
function Merge_Generic
{
   typeset -i RC=0
   typeset    deltaflagmsg="${BLDCYCLE} $(date)"
   typeset    directory
   typeset    host_colon=""
   typeset    mrgmsg="${generic_descripition} merge for ${BLDCYCLE} ($(date))"
   typeset    newer=${generic_host_logfile}

   [[ ${HOSTSFILE_HOST} != ${hostname} ]] && host_colon="${HOSTSFILE_HOST}:"

   for directory in ${generic_source}
   do
      Execute "" check_directory ${host_colon}${directory}
      [[ $? -ne 0 ]] && \
      {
         log +l -e "${host_colon}${directory} is not a directory"
         RC=1
      }
   done

   for directory in ${generic_destination}
   do
      Execute "" check_directory ${directory}
      [[ $? -ne 0 ]] && \
      {
         Execute "" make_directory ${directory} "1>" /dev/null "2>&1"
      }
 
      [[ ! -w ${directory} ]] && \
      {
         log +l -e "${directory} is not writable"
         RC=1
      }
   done
   [[ ${RC} -ne 0 ]] && return 1

   if [[ ${HOSTSFILE_HOST} = ${hostname} ]]
   then
      [[ ! -f ${generic_host_logfile} ]] && RC=1
   else
      Execute ${HOSTSFILE_HOST} \
         "if [[ ! -f ${generic_host_logfile} ]]; then print \"1\"; fi"
      RC=$?
   fi
   [[ ${RC} -ne 0 ]] && \
   {
      log +l "full copy of ${generic_descripition},"
      log +l "${host_colon}${generic_host_logfile} doesn't exist"
      mrgmsg="${mrgmsg} --- Full Copy ---"
      deltaflagmsg="${deltaflagmsg} --- Full Copy ---"
      newer=""
      RC=0
   }

   set ${generic_destination}
   for directory in ${generic_source}
   do
      log +l -b "Merging ${host_colon}${directory}"
      Execute "" treecopy ${host_colon}${directory} $1 ${newer} "1>" \
              /dev/null "2>&1"
      [[ $? -ne 0 ]] && \
      {
         log +l -e "treecopy ${host_colon}${directory} $1 ${newer}"
         RC=1
      }
      shift 1
   done
   [[ $RC -ne 0 ]] && \
   {
       log +l -e "merging ${merge_type} for ${BLDCYCLE}"
       mrgmsg="${mrgmsg} === FAILED ==="
       deltaflagmsg="${deltaflagmsg} === FAILED ==="
   }

   Execute "" "print \"$mrgmsg\" >> ${generic_AFS_logfile}"
   if [[ -z "${host_colon}" ]]
   then
      Execute "" "print \"${deltaflagmsg}\" >> ${generic_host_logfile}"
   else
      Execute ${HOSTSFILE_HOST} "print \"${deltaflagmsg}\" >> \
              ${generic_host_logfile}; print \"0\""
   fi

   return ${RC}
}

#
# NAME: Merge_Objects
#
# FUNCTION: Merge the object directory for the release and build cycle
#           specified.
#
# INPUT: BLDCYCLE (bldcycle) - Build cycle.
#        RELEASE (global) - Release name.
#
# OUTPUT: generic_AFS_logfile (GLOBAL) - Data log file in afs.
#         generic_descripition (GLOBAL) - English descripition of the merge
#         generic_destination (GLOBAL) - List of destination directories.
#         generic_host_logfile (GLOBAL) - Data log file on host machine.
#         generic_source (GLOBAL) - List of host directories.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 on success, 1 on failure.
#
function Merge_Objects
{
   bldhostsfile ${RELEASE} "${TRUE}"
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Cannot continue with nls tree merge"
      return 1
   }

   [[ "${HOSTSFILE_AFSPROD}" = "*" || \
      "${HOSTSFILE_AFSDELTA}" = "*" ]] && \
   {
      log +l "nls merges are not done for ${RELEASE}"
      return 0
   }
   
   HOSTSFILE_OBJECT="${HOSTSFILE_OBJECT}"
 
   generic_host_logfile=${HOSTSFILE_OBJECT}/nlsdeltaflag
   generic_descripition="${RELEASE} nls tree"
   generic_destination=${HOSTSFILE_AFSPROD}/nls
   generic_AFS_logfile=${HOSTSFILE_AFSPROD}/../LEVELS.${RELEASE}
   generic_source=${HOSTSFILE_OBJECT}

   Merge_Generic ${BLDCYCLE}
   return $?
}


#
# NAME: Merge_PTF
#
# FUNCTION: Merge data from HISTORY and UPDATE trees.
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: 
#
# RETURNS:
#
function Merge_PTF
{
   typeset -i RC=0
   typeset    directory			# Loop variable
   typeset    file			# Loop variable through
					# ${mstr_preqtbl_files} and
					# ${supertbl_files}
   typeset    mstr_preqtbl_files
   typeset    supertbl_files

   bldhostsfile bldafsmerge_ptf "${TRUE}"
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Cannot continue with selective fix data merge"
      return 1
   }

   history_dir=${HOSTSFILE_BASE}/HISTORY
   update_dir=${HOSTSFILE_BASE}/UPDATE
   bldhistory_dir=${history_dir}/bldhistory
   afs_history_dir=${HOSTSFILE_AFSBASE}/HISTORY
   afs_bldhistory_dir=${afs_history_dir}/bldhistory
   afs_update_dir=${HOSTSFILE_AFSBASE}/UPDATE

   for directory in ${history_dir} ${update_dir}
   do
      Execute "" check_directory ${directory}
      [[ $? -ne 0 ]] && \
      {
         log +l -e "${directory} does not exist"
         RC=1
      }
   done
   [[ ${RC} -ne 0 ]] && return ${RC}

   for directory in $afs_history_dir $afs_update_dir $afs_bldhistory_dir
   do
      Execute "" check_directory ${directory}
      [[ $? -ne 0 ]] && \
      {
         Execute "" make_directory ${directory} "1>" /dev/null "2>&1"
      }
 
      [[ ! -w ${directory} ]] && \
      {
         log +l -e "${directory} not writeable"
         RC=1
      }
   done
   [[ ${RC} -ne 0 ]] && return ${RC}

   log +l -b "Merging files in HISTORY"
   history_file_list=""
   for history_file in ${history_dir}/abstracts ${history_dir}/abstracts.raw \
                       ${history_dir}/memo_info ${history_dir}/memo_info.raw \
                       ${history_dir}/ptfoptions
   do
      if [[ -f $history_file ]]
      then
         history_file_list="${history_file_list} ${history_file}"
      else
         log +l -w "$history_file does not exist"
      fi
   done
   Execute "" cp -p ${history_file_list} ${afs_history_dir}
   if [[ $? -ne 0 ]]
   then
      log +l -e "Copying ${history_file} to ${afs_history_dir}"
      RC=1
   fi

   log +l -b "Merging history databases in HISTORY/bldhistory"
   bldhistory_files=$(ls ${bldhistory_dir}/* 2> /dev/null)
   [[ -z "${bldhistory_files}" ]] && log +l -w "No files in ${bldhistory_dir}"
   for file in ${bldhistory_files}
   do
      Execute "" cp -p $bldhistory_dir/* $afs_bldhistory_dir
      [[ $? -ne 0 ]] && \
      {
         log +l -e "Copying ${file} to ${afs_bldhistory_dir}"
         RC=1
      }
   done

   for lpp_name in $(ls $update_dir)
   do
      update_lpp_dir=${update_dir}/${lpp_name}
      check_directory ${update_lpp_dir}
      [[ $? -ne 0 ]] && continue

      afs_update_lpp_dir=${afs_update_dir}/${lpp_name}
      check_directory ${afs_update_lpp_dir}
      if [[ $? -ne 0 ]]
      then
         Execute "" make_directory ${afs_update_lpp_dir}
         if [[ $? -ne 0 ]]
         then
            log +l -e "Could not create directory ${afs_update_lpp_dir}"
            RC=1
            continue
         fi
      fi
      [[ ! -w "${afs_update_lpp_dir}" ]] && \
      {
         log +l -e "${afs_update_lpp_dir} not writeable"
         RC=1
         continue
      }
      log +l -b "Merging ${lpp_name}"
      for file_type in cumsList ptfsList
      do
         for file in $(ls ${update_lpp_dir}/*/*/${file_type} 2> /dev/null)
         do
            subpath=${file#${update_lpp_dir}/}
            subpath=${subpath%/${file_type}}
            last_entry=""
            while read entry
            do
               if [[ -n "${last_entry}" ]]
               then
                  afs_directory=${afs_update_lpp_dir}/${subpath}/${last_entry}
                  Execute "" check_directory ${afs_directory}
                  if [[ $? -eq 0 ]]
                  then
                     rm -fr ${afs_directory}
                     log +l -f "Removed ${afs_directory}"
                  fi
               fi
               last_entry=${entry}
            done < ${file}
            afs_directory=${afs_update_lpp_dir}/${subpath}/${last_entry}
            Execute "" check_directory ${afs_directory}
            if [[ $? -ne 0 ]]
            then
               Execute "" make_directory ${afs_directory}
               if [[ $? -ne 0 ]]
               then
                  log +l -e "Could not create directory ${afs_directory}"
                  RC=1
                  continue
               fi
            elif [[ ${file_type} = "cumsList" ]]
            then
               continue
            fi
            Execute "" cp -p ${update_lpp_dir}/${subpath}/${file_type} \
                             ${afs_update_lpp_dir}/${subpath}/${file_type}
            if [[ $? -ne 0 ]]
            then
               log +l -e "Copying ${file_type} to ${afs_update_dir}"
               RC=1
            fi
            subpath="${subpath}/${last_entry}"
            log +l -f "Updating ${update_lpp_dir#${update_dir}/}/${subpath}"
            update_file_list=""
            for update_file in ${update_lpp_dir}/${subpath}/aparsList \
                               ${update_lpp_dir}/${subpath}/coreqsList \
                               ${update_lpp_dir}/${subpath}/filenamesList \
                               ${update_lpp_dir}/${subpath}/ifreqsList \
                               ${update_lpp_dir}/${subpath}/prereqsList \
                               ${update_lpp_dir}/${subpath}/supersedesList
            do
               if [[ -f ${update_file} ]]
               then
                  update_file_list="${update_file_list} ${update_file}"
               else
                  log +l -w "${update_file} does not exist"
               fi
            done
            if [[ -n "${update_file_list}" ]]
            then
               Execute "" cp -p ${update_file_list} \
                                ${afs_update_lpp_dir}/${subpath}
               if [[ $? -ne 0 ]]
               then
                  log +l -e "Copying to directory ${afs_update_dir}/${subpath}"
                  RC=1
               fi
            fi
         done
      done
   done

   return $RC
}

#
# NAME: Merge_Ship
#
# FUNCTION: Merge the ship tree.
#
# INPUT:
#
# OUTPUT: generic_AFS_logfile (GLOBAL) - Data log file in afs.
#         generic_descripition (GLOBAL) - English descripition of the merge
#         generic_destination (GLOBAL) - List of destination directories.
#         generic_host_logfile (GLOBAL) - Data log file on host machine.
#         generic_source (GLOBAL) - List of host directories.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 on success, 1 on failure.
#
function Merge_Ship
{
   bldhostsfile bldafsmerge_ship "${TRUE}"
   [[ $? -ne 0 ]] && \
   {
      log +l -e "Cannot continue with ship tree merge"
      return 1
   }

   generic_host_logfile=${HOSTSFILE_BASE}/deltaflag
   generic_descripition="ship tree"
   generic_destination=${HOSTSFILE_AFSBASE}
   generic_AFS_logfile=${HOSTSFILE_AFSBASE}/LEVELS.ship
   generic_source=${HOSTSFILE_BASE}

   Merge_Generic ${bldcycle}
   return $?
}

#
# NAME: Modify_Options
#
# FUNCTION:
#
# INPUT: action - Add or Delete the option.
#        check_option - Option to check.
#        all_options - Options to be checked.
#
# OUTPUT: new options written to stdout.
#
# SIDE EFFECTS: none.
#
# RETURNS: - 0 always.
#
function Modify_Options
{
   typeset    action=$1				# Add or Delete the option.
   typeset    all_options			# Options to check.
   typeset    check_option=$2			# Option to check for.
   typeset    option				# Current option being checked.
   typeset    found_option="${FALSE}"		# Set to true if option is to
						# be added, but already present
						# in the list of options.
   typeset    return_options=""			# Options to return.

   shift 2
   all_options=$*

   for option in ${all_options}
   do
      if [[ "${option}" = "${check_option}" ]]
      then
         # Adding the option and option already exists in the list.  Set
	 # option not to add the option again.
         if [[ "${action}" = "add" ]] 
         then
            found_option="${TRUE}"
            return_options="${return_options} ${option}"
         fi
         # Default here is delete, option will not be added to return list.
      else
         # Not option being checked, add to output and move onto next.
         return_options="${return_options} ${option}"
      fi
   done

   # Add option if it is not already in the list.
   [[ "${action}" = "add" && "${found_option}" = "${FALSE}" ]] && \
      return_options="${return_options} ${check_option}"

   print - ${return_options}
   return 0
}

function Query_Bldafsmerge
{
   QueryStatus $_TYPE ${T_BLDAFSMERGE} \
               $_LEVEL "$_DONTCARE" \
               $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
               $_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
               $_STATUS ${ST_SUCCESS}
}

function Query_Prebuild
{
   QueryStatus $_TYPE ${T_PREBUILD} \
               $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
               $_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
               $_SUBTYPE ${S_RELMERGE} $_STATUS ${ST_SUCCESS}
}

function Query_V3bld
{
   QueryStatus $_TYPE ${T_V3BLD} \
               $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
               $_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
               $_SUBTYPE "$_DONTCARE" $_STATUS ${U_UPDATE}
}

#
# NAME: Translate
#
# FUNCTION: Translate a number range supplied into a list of the
#           numbers.
#
# INPUT: lower ($1) - Lower bound on values >=.
#        range ($3) - Range of values.
#        upper ($2) - Upper bound on values <=.
#
# OUTPUT: Numbers displayed to stdout.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 if no errors, 1 if errors.
#
function Translate
{
   typeset -i end		# End of current range to check.
   typeset -i lower=$1		# Lowest a number can be.
   typeset    range=$3		# Values to be evaluated.
   typeset -i start		# Start of current range to check.
   typeset -i temp		# Temporary number.
   typeset -i upper=$2		# Largest a number can be.

   # turn all into numbers.
   [[ "${range}" = "all" ]] && range="${lower}-${upper}"

   # parse the values supplied.
   while [[ -n "${range}" ]]
   do
      # everything to the left of the first comma.
      subrange=${range%%,*}
      # everything to the left of the dash.
      start=${subrange%%-*}
      # Was a dash seen?
      if [[ "${subrange}" = "${start}" ]]
      then 
         # No - return just the number seen.
         start=${subrange}
         end=${subrange}
      else
         # Yes - determine the end of the range.
         end=${subrange##*-}
      fi
      # Expect numbers from lowest to highest, interchange values if needed.
      if [[ ${start} -gt ${end} ]]
      then
         temp=${start}
         start=${end}
         end=${temp}
      fi
      # Display the numbers if within range, error message if out of range.
      while [[ ${start} -le ${end} ]]
      do
         if [[ ${start} -ge ${lower} && ${start} -le ${upper} ]] 
         then
            print ${start}
         else
            log +l -e -c Translate "${start} not in range [${lower}-${upper}]"
         fi
         (( start = ${start} + 1 ))
      done
      # Remove all data to the left and including the first comma.
      new_range=${range#${subrange},}
      # If no commas were found, then return.
      if [[ "${new_range}" = "${range}" ]]
      then
         return 0
      fi
      # Set the new range to the data right of the first comma.
      range=${new_range}
   done

   return 1
}

#
# NAME: Update_Status
#
# FUNCTION: Mark a status record as successful for merge.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#        RELEASE (global) - Release name.
#        subtype ($3) - Type of Merge.
#        status ($4) - Type of status to post.
#
# OUTPUT: None.
#
# SIDE EFFECTS: Creates a entry in the status file.
#
# RETURNS: 0 always.
#
function Update_Status
{
   typeset -i RC=0
   typeset    logline
   typeset    subtype=$1
   typeset    status=$2

   Execute "" DeleteStatus $_TYPE ${T_BLDAFSMERGE} $_BLDCYCLE ${BLDCYCLE} \
                           $_RELEASE ${RELEASE} \
                           $_SUBTYPE ${subtype} $_STATUS ${status}
   RC=$?
   if [[ ${RC} -ne 0 ]]
   then
      logline="DeleteStatus returned ${RC} (${T_BLDAFSMERGE} ${BLDCYCLE} "
      logline="${logline}${RELEASE} ${subtype})"
      log +l -c Update_Status -e "${logline}"
   fi
   Execute "" bldsetstatus $_TYPE ${T_BLDAFSMERGE} $_BLDCYCLE ${BLDCYCLE} \
                           $_RELEASE ${RELEASE} \
                           $_SUBTYPE ${subtype} $_STATUS ${status}
   RC=$?
   if [[ ${RC} -ne 0 ]]
   then
      logline="bldsetstatus returned ${RC} (${T_BLDAFSMERGE} ${BLDCYCLE} "
      logline="${logline}${RELEASE} ${subtype})"
      log +l -c Update_Status -e "${logline}"
   fi

   return 0
}

#
# NAME: Usage
#
# FUNCTION: Usage statement.
#
# INPUT: None.
#
# OUTPUT: None.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
function Usage
{
   print -u2 "Usage: bldafsmerge [ -BSUabdnps ]"
   return 0
}

. bldkshconst
. bldinitfunc
. bldkshfunc
. bldloginit
. bldhostsfile
. bldnodenames

test ${BLDTMP:=$(bldtmppath)}
typeset    work_dir=${BLDTMP}/bldafsmerge_$$
					# Work directory.
typeset -r BLDAFSMERGE_RECORDS="${work_dir}/bldafsmerge_records"
export BLDCYCLE=""			# Current build cycle being merged.
typeset -r CLOSE="close"
typeset -r CREATE="create"
typeset    DATA				# Current build cycle being merged
					# data record.
typeset -r FOOTER_SIZE=8		# Number of lines footer takes.
typeset -r HEADER_SIZE=4		# Number of lines header takes.
typeset -r LINES=${LINES:=24}		# Number of lines on screen.
typeset -r DATA_SIZE=${LINES}-${HEADER_SIZE}-${FOOTER_SIZE}
					# Number of data display lines
					# there are.
typeset -r MATCH="match"
typeset    MERGE_BLDENV="${FALSE}"	#
typeset    MERGE_DELTA="${FALSE}"	#
typeset -r MERGE_DIMENSION=250		# Size of the merge arrays.
typeset    MERGE_OBJECT="${FALSE}"	#
typeset    MERGE_PTF="${FALSE}"		#
typeset    MERGE_SHIP="${FALSE}"		#
typeset -r MERGE_TYPES="$S_BLDENVMERGE $S_DELTAMERGE $S_OBJECTMERGE $S_PTFMERGE $S_SHIPMERGE"
typeset    MERGED_BLDENV		# If not null, value is build cycle
					# the build environment was merged in.
typeset    MERGED_PTF			# If not null, value is build cycle
					# the PTF data was merged in.
typeset    MERGED_SHIP			# If not null, value is build cycle
					# the ship tree was merged in.
typeset -r OPEN="open"
typeset -r PREBUILD_RECORDS="${work_dir}/prebuild_records"
typeset -r PREBUILD_RECORDS_MERGE="${work_dir}/prebuild_records_merge"
typeset -i RC=0				# Return code from function calls or
					# from program.
typeset -r READ="read"
typeset    RELEASE			# Current release being merged.
typeset -r REMOVE="remove"
typeset    SHOW_MODE="${FALSE}"		# Only show what bldafsmerge would
					# do.  Don't copy or updata any files.
typeset    UPDATE_MODE="${FALSE}"	# Don't perform merge, just update 
					# status file records so the merge
					# appears to have occured.
typeset -r V3BLD_RECORDS="${work_dir}/v3bld_records"
typeset    advisory_messages="${FALSE}"	# If running interactave and advisory
					# messages have been display, prompt
					# before entering the menu.
typeset    bldafsmerge_bldcycle		# Build cycle in bldafsmerge file.
typeset    bldafsmerge_release		# Release in bldafsmerge file.
typeset -i bldafsmerge_entries		# Number of merge entries for release.
typeset    bldafsmerge_types		# Merge entries for release.
typeset    bldafsmerge_data		# unused - current entry has not been
					#          used.
					# used - next record needs to be read
					#        from file before use.
					#        from file before use.
typeset    delta_release_bldcycle_pairs=""
					# List of release/bldcycle names
					# used to keep track of which
					# delta merges failed.  No other
					# delta merges can occur until 
					# the release/bldcycle is successful.
typeset    deltaflagmsg			# File on the host to log merge 
					# message into.
typeset    fullmsg			# Set when a complete merge must be
					# done.
typeset    generic_AFS_logfile		# File to log to in AFS.
typeset    generic_descripition		# English description of merge.
typeset    generic_destination		# List of destination directories in
					# AFS.  Has a corresponding entry in
					# generic_source.
typeset    generic_host_logfile		# File to log messages to on the host
					# and modification time used to 
					# determine files copied for delta
					# merges.
typeset    generic_source		# List of directories on the host that
					# must be copied to AFS.  Directory to
					# copy files to in AFS is the
					# corresponding entry in
					# generic_destination.
typeset -u hostname=$(hostname -s)	# Name of current system.
typeset -i index=0			# Index for doing the merges.
typeset    just_bldcycle=""		# Set when a entry has no associated
					# release.  If next entry is not the
					# same build cycles with a release,
					# a warning message will be generated
					# about build cycle.
typeset    last_bldcycle		# Last build cycle seen, used to 
					# prevent generating the same warning
					# message several times.
typeset    logline			# Used to build up the logging
					# messages.
typeset -i merge_index=0		# Index to current menu item at the
					# top of the screen.
typeset -i merge_size=0			# Number of entries in merge tables.
typeset    merge_bldcycle[${MERGE_DIMENSION}]
					# Build cycle.
typeset    merge_option="${FALSE}"	# User supplied a -b, -d, -n, -p, or
					# -s on command line.
typeset    merge_release[${MERGE_DIMENSION}]
					# Release name.
typeset    merge_data[${MERGE_DIMENSION}]
					# Merge data.
typeset    dont_merge_data[${MERGE_DIMENSION}]
					# Data that should not be merged.
typeset    mrgmsg			# File in afs to log merge message
					# into.
typeset    newer			# For delta merges, any file with a
					# modification more recent will be
					# merged.
typeset    object_release_bldcycle_pairs=""
					# List of release/bldcycle names
					# used to keep track of which
					# object directories that have been
					# merged and the build cycle the merge
					# took place.
typeset    prebuild_bldcycle		# Build cycle in prebuild file.
typeset    prebuild_release		# Release in prebuild file.
typeset -i prebuild_entries		# Number of prebuild levels for release.
typeset    prebuild_levels		# Levels for the release.
typeset    prebuild_data		# unused - current entry has not been
					#          used.
					# used - next record needs to be read
					#        from file before use.
typeset -r progname=$(basename $0)	# Program name.
typeset -i status			# Status of object copy of the current
					# build cycle/release pair.
typeset    temp				# Temporary variable.
typeset    treecopy_output="1> /dev/null"
					# Ignore output of tree copy.  If
					# running with -S this value will be
					# set to NULL so that command is
					# displayed on screen.
typeset    v3bld_bldcycle		# Build cycle in v3bld file.
typeset    v3bld_release		# Release in v3bld file.
typeset -i v3bld_entries		# Number of v3bld levels built for
					# release.
typeset    v3bld_levels			# Levels built for the release.
typeset    v3bld_data			# unused - current entry has not been
					#          used.
					# used - next record needs to be read

trap 'Clean_Up; trap - EXIT; exit ${RC}' EXIT INT QUIT HUP TERM

###########

chksettop
chksetstatus
chksetklog

logset -C bldafsmerge -c bldafsmerge -F ${BLDTMP}/bldafsmerge

[[ -z "${hostname}" ]] && \
{
   log +l -e "Cannot get hostname"
   RC=1
   exit
}

while getopts :SUabdnps option
do
   case $option in
      S) SHOW_MODE="${TRUE}"
         ;;
      U) UPDATE_MODE="${TRUE}"
         ;;
      a)
         MERGE_DELTA="${TRUE}"
         MERGE_BLDENV="${TRUE}"
         MERGE_SHIP="${TRUE}"
         MERGE_OBJECT="${TRUE}"
         MERGE_PTF="${TRUE}"
         merge_option="${TRUE}"
         ;;
      b)
         MERGE_BLDENV="${TRUE}"
         merge_option="${TRUE}"
         ;;
      d)
         MERGE_DELTA="${TRUE}"
         merge_option="${TRUE}"
         ;;
      n)
         MERGE_OBJECT="${TRUE}"
         merge_option="${TRUE}"
         ;;
      p)
         MERGE_PTF="${TRUE}"
         merge_option="${TRUE}"
         ;;
      s)
         MERGE_SHIP="${TRUE}"
         merge_option="${TRUE}"
         ;;
      :)
         print -u2 "$OPTARG requires a value"; Usage; RC=4; exit;;
     \?)
         print -u2 "unknown option $OPTARG"; Usage; RC=4; exit;;
   esac
done

if [[ ${merge_option} = "${FALSE}" ]]
then
   MERGE_DELTA="${TRUE}"
   MERGE_BLDENV="${TRUE}"
   MERGE_SHIP="${TRUE}"
   MERGE_OBJECT="${TRUE}"
   MERGE_PTF="${TRUE}"
fi

make_directory ${work_dir}
File_Control_Bldafsmerge ${CREATE} ${OPEN}
File_Control_Prebuild ${CREATE} ${OPEN}
File_Control_V3bld ${CREATE} ${OPEN}

Get_Bldcycle_Release | \
   while read BLDCYCLE RELEASE
   do
      # Insure we have a valid release, may have pulled out some status
      # records we should not have, hopefully we will catch and ignore
      # them here.
      [[ "${RELEASE}" = "defects" ]] && continue
      [[ -n "${RELEASE}" ]] && bldhostsfile ${RELEASE} "${TRUE}"
      [[ $? -ne 0 ]] && advisory_messages="${TRUE}"

      # Generate warning messages about open build cycles.
      if [[ "${BLDCYCLE}" != "${last_bldcycle}" ]]
      then
         Is_Bldcycle_Closed
         if [[ $? != 0 ]]
         then
            log +l -w "Build cycle ${BLDCYCLE}, still open"
            advisory_messages="${TRUE}"
         fi
      fi

      if [[ -n "${just_bldcycle}" && "${just_bldcycle}" != "${BLDCYCLE}" ]]
      then
         log +l -w "Build cycle ${just_bldcycle} has no associated releases"
         advisory_messages="${TRUE}"
      fi

      if [[ -z "${RELEASE}" ]]
      then
         just_bldcycle=${BLDCYCLE}
         last_bldcycle=${BLDCYCLE}
         continue
      else
         just_bldcycle=""
      fi

      # Get the current merge records for the release.
      File_Control_Bldafsmerge ${MATCH}
      if [[ $? -eq 0 ]]
      then
         merge_types=${bldafsmerge_types}
         merge_entries=${bldafsmerge_entries}
      else
         merge_types=""
         merge_entries=0
      fi

      # Do any delta merge entries exist for this release?
      # Must always do a read to keep data in sync.
      File_Control_Prebuild ${MATCH}
      RC=$?
      print "${merge_types}" | grep ${S_DELTAMERGE} 1> /dev/null 2>&1
      if [[ $? -ne 0 ]]
      then
         # If no delta merge entries exist was prebuild run on this release?
         # If prebuild not run, no data to merge, mark delta as being merged.
         if [[ ( ${RC} -eq 0 && ${prebuild_entries} -eq 0 ) || ${RC} -ne 0 ]]
         then
            merge_types="${S_DELTAMERGE} ${merge_types}"
            (( merge_entries = ${merge_entries} + 1 ))
         else
            print ${prebuild_bldcycle} ${prebuild_release} ${prebuild_entries} ${prebuild_levels} >> $PREBUILD_RECORDS_MERGE
         fi
      fi

      # Do any object merge entries exist for this release?
      File_Control_V3bld ${MATCH}
      RC=$?
      print "${merge_types}" | grep ${S_OBJECTMERGE} 1> /dev/null 2>&1
      if [[ $? -ne 0 ]]
      then
         # If no object merge entries exist was v3bld run on this release?
         # If v3bld not run, no data to merge, mark object as being merged.
         [[ ( ${RC} -eq 0 && ${v3bld_entries} -eq 0 ) || ${RC} -ne 0 ]] && \
         {
            merge_types="${S_OBJECTMERGE} ${merge_types}"
            (( merge_entries = ${merge_entries} + 1 ))
         }
      fi
   
      [[ ${merge_entries} -lt ${AFSMERGE_SUBTYPES} ]] && \
         Create_Merge_Entry 

      last_bldcycle=${BLDCYCLE}
   done

File_Control_Bldafsmerge ${CLOSE} ${REMOVE}
File_Control_V3bld ${CLOSE} ${REMOVE}
File_Control_Prebuild ${CLOSE} ${REMOVE}
if [[ -f ${PREBUILD_RECORDS_MERGE} ]]
then
   mv ${PREBUILD_RECORDS_MERGE} ${PREBUILD_RECORDS}
   File_Control_Prebuild ${OPEN}
fi


if [[ "${advisory_messages}" = "${TRUE}" ]]
then
   print "Enter <CR> to continue: \c"
   read temp
fi
Menu
[[ $? -ne 0 ]] && \
{ 
   RC=0
   exit
}

last_bldcycle=""
index=0

while [[ index -lt ${merge_size} ]]
do
   export BLDCYCLE=${merge_bldcycle[${index}]}
   export DATA=${merge_data[${index}]}
   export RELEASE=${merge_release[${index}]}

   # All logging goes into file in LOG directory of the BLDCYCLE.
   logset -F $(bldlogpath)/${progname}

   [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
      log -b "Merging build cycle ${BLDCYCLE}"

   # Build Environment needs only be merged once, not with each build cycle.
   In_List $S_BLDENVMERGE ${DATA}
   if [[ $? -eq 0 ]]
   then
      if [[ -z "${MERGED_BLDENV}" ]] 
      then
         log -b "Starting build environment merge for ${BLDCYCLE}"
         if [[ "${UPDATE_MODE}" = "${FALSE}" ]] 
         then
            Merge_Bldenv
         else
            :
         fi
         if [[ $? -ne 0 ]]
         then
            log -e "Merging build environment for build cycle ${BLDCYCLE}"
            MERGED_BLDENV="FAILED"
         else
            Update_Status ${S_BLDENVMERGE} ${ST_SUCCESS}
         fi
         log -b "Completed build environment merge on afs for ${BLDCYCLE}"
         MERGED_BLDENV="${BLDCYCLE}${MERGED_BLDENV}"
      elif [[ "${MERGED_BLDENV}" = *FAILED ]]
      then
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Build environment merge failed in ${MERGED_BLDENV%FAILED}"
      elif [[ -n "${MERGED_BLDENV}" ]]
      then
         Update_Status ${S_BLDENVMERGE} ${ST_SUCCESS}
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Build environment merge was performed under ${MERGED_BLDENV}"
      fi
   fi

   # Delta merge needs to occur for each build cycle.
   In_List $S_DELTAMERGE ${DATA}
   if [[ $? -eq 0 ]]
   then
      log -b "Starting delta tree merges on afs for ${BLDCYCLE} (${RELEASE})"
      echo "${delta_release_bldcycle_pairs}" | while read release bldcycle
      do
         [[ "${release}" = "${RELEASE}" ]] && break
      done
      if [[ -n "${release}" ]]
      then
         log -e "Delta merge for ${release} failed in ${bldcycle}"
      else
         if [[ "${UPDATE_MODE}" = "${FALSE}" ]] 
         then
            Merge_Delta
         else
            :
         fi
         if [[ $? -ne 0 ]]
         then
            log -e "Merging delta trees for build cycle ${BLDCYCLE} (${RELEASE})"
            if [[ -z "${delta_release_bldcycle_pairs}" ]]
            then
               delta_release_bldcycle_pairs="${RELEASE} ${BLDCYCLE}"
            else
               delta_release_bldcycle_pairs="${delta_release_bldcycle_pairs}\n${RELEASE} ${BLDCYCLE}"
            fi
         else
            Update_Status ${S_DELTAMERGE} ${ST_SUCCESS}
         fi
      fi
      log -b "Completed delta tree merges on afs for ${BLDCYCLE} (${RELEASE})"
   fi


   # object tree needs to be merged for once for each release.
   In_List $S_OBJECTMERGE ${DATA}
   if [[ $? -eq 0 ]]
   then
      echo "${object_release_bldcycle_pairs}" | \
         while read release bldcycle status
         do
            [[ "${release}" = "${RELEASE}" ]] && break
         done
      if [[ -n "${release}" && ${status} -ne 0 ]]
      then
         log -b "nls merge for ${release} failed in ${bldcycle}"
      elif [[ -n "${release}" ]]
      then
         log -b "nls merge for ${release} was performed under ${bldcycle}"
         Update_Status ${S_OBJECTMERGE} ${ST_SUCCESS}
      else
         log -b "Starting nls tree merges for ${BLDCYCLE} (${RELEASE})"
         if [[ "${UPDATE_MODE}" = "${FALSE}" ]] 
         then
            Merge_Objects
            RC=$?
         else
            RC=0
         fi
         if [[ ${RC} -ne 0 ]]
         then
            log -e "Merging nls trees for ${BLDCYCLE} (${RELEASE})"
         else
            Update_Status ${S_OBJECTMERGE} ${ST_SUCCESS}
         fi
         if [[ -z "${object_release_bldcycle_pairs}" ]]
         then
            object_release_bldcycle_pairs="${RELEASE} ${BLDCYCLE} ${RC}"
         else
            object_release_bldcycle_pairs="${object_release_bldcycle_pairs}\n${RELEASE} ${BLDCYCLE} ${RC}"
         fi
         log -b "Completed nls tree merges for ${BLDCYCLE} (${RELEASE})"
      fi
   fi

   # PTF data needs only be merged once, not with each build cycle.
   In_List $S_PTFMERGE ${DATA}
   if [[ $? -eq 0 ]]
   then
      if [[ -z "${MERGED_PTF}" ]]
      then
         log -b "Starting selective fix data merge for ${BLDCYCLE}"
         if [[ "${UPDATE_MODE}" = "${FALSE}" ]] 
         then
            Merge_PTF 
         else
            :
         fi
         if [[ $? -ne 0 ]]
         then
            log -e "Merging PTF data for build cycle ${BLDCYCLE}"
            MERGED_PTF="FAILED"
         else
            Update_Status ${S_PTFMERGE} ${ST_SUCCESS}
         fi
         log -b "Completed selective fix data merge for ${BLDCYCLE}"
         MERGED_PTF="${BLDCYCLE}${MERGED_PTF}"
      elif [[ "${MERGED_PTF}" = *FAILED ]]
      then
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Selective fix data merged failed in ${MERGED_PTF%FAILED}"
      elif [[ -n "${MERGED_PTF}" ]]
      then
         Update_Status ${S_PTFMERGE} ${ST_SUCCESS}
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Selective fix data merge was performed under ${MERGED_PTF}"
      fi
   fi

   # Ship needs only be merged once, not with each build cycle.
   In_List $S_SHIPMERGE ${DATA}
   if [[ $? -eq 0 ]]
   then
      if [[ -z "${MERGED_SHIP}" ]]
      then
         log -b "Starting ship tree merge for ${BLDCYCLE}"
         if [[ "${UPDATE_MODE}" = "${FALSE}" ]] 
         then
            Merge_Ship 
         else
            :
         fi
         if [[ $? -ne 0 ]]
         then
            log -e "Merging ship tree for build cycle ${BLDCYCLE}"
            MERGED_SHIP="FAILED"
         else
            Update_Status ${S_SHIPMERGE} ${ST_SUCCESS}
         fi
         log -b "Completed ship tree merge for ${BLDCYCLE}"
         MERGED_SHIP="${BLDCYCLE}${MERGED_SHIP}"
      elif [[ "${MERGED_SHIP}" = *FAILED ]]
      then
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Ship tree merged failed in ${MERGED_SHIP%FAILED}"
      elif [[ -n "${MERGED_SHIP}" ]]
      then
         Update_Status ${S_SHIPMERGE} ${ST_SUCCESS}
         [[ "${BLDCYCLE}" != "${last_bldcycle}" ]] && \
            log -f "Ship tree merge was performed under ${MERGED_SHIP}"
      fi
   fi

   (( index=${index}+1 ))
   last_bldcycle=${BLDCYCLE}

   logset -r
done

[[ -f ${PREBUILD_RECORDS} ]] && File_Control_Prebuild ${CLOSE} ${REMOVE}

logset -r

RC=0
exit
