#! /bin/ksh
# @(#)17	1.33  src/bldenv/bldtools/ptfsetup.sh, bldtools, bos412, GOLDA411a 5/26/94 09:43:00
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: ptfsetup
#
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME        : ptfsetup
#
# FUNCTION    : Create files required by subptf and cumptf for Developer
#               and Area Builds.
#
# SYNTAX      : ptfsetup [ -a ] [ -r RELEASE ] [ -c BLDCYCLE ] 
#			 [ -l "LPP ..." ] [ -u ] [ -n ] 
#                        [ [ -b | -f ] name ... ]
#
# INPUT       : RELEASE - CMVC release name
#               BLDCYCLE - CMVC build cycle
#               name(s) - if no option specified or -b, then CMVC build
#                         name(s), if -f, then list of file(s) containing 
#			  lists of defects.
#
# RETURNS     : 0 means success, <>0 means failure
#
# SIDE EFFECTS: creates the following files: 
#                  $TOP/PTF/$BLDCYCLE/$CMVC_RELEASE/changeview
#                  $TOP/PTF/$BLDCYCLE/$CMVC_RELEASE/defects
#

################################################################################
#
# NAME        : add_to_changeview
#
# FUNCTION    : Call Report to generate changeviews for the list of defects
#               passed.
#
# SYNTAX      : add_to_changeview
#
# INPUT       : None.
#
# RETURNS     : 0 if successful, aborts and exits if error.
#
# SIDE EFFECTS: Writes data into ${CHANGEVIEW} file.
#
# Note: ${defectlist} is a comma separated list of defects with an extra comma
#       on end of list that is stripped off before use.
#
################################################################################
function add_to_changeview
{
   Report -view Changeview -raw -where \
      "releasename = '${CMVC_RELEASE}' and defectname in ( ${defectlist%,} )" \
      >> ${CHANGEVIEW}
   [[ $? -ne 0 ]] && log +l -x "Writing changeview file: ${CHANGEVIEW}"
   return 0
}

################################################################################
#
# NAME        : add_to_abstractlist
#
# FUNCTION    : Call Report to generate changeviews for the list of defects
#               passed.
#
# SYNTAX      : add_to_abstractlist
#
# INPUT       : None.
#
# RETURNS     : 0 if successful, aborts and exits if error.
#
# SIDE EFFECTS: Writes data into ${ABSTRACTLIST} file.
#
# Note: ${defectlist} is a comma separated list of defects with an extra comma
#       on end of list that is stripped off before use.
#
################################################################################
function add_to_abstractlist
{
   # adding defects to abstractlist file
   Report -view defectview -raw -where \
	"name in ( ${defectlist%,} )" > ${tmp_file}
   [[ $? -ne 0 ]] && log +l -x "Writing abstractlist file: ${ABSTRACTLIST}"
   awk -F"|" ' { printf "%6s:%s  %s\n",$2,CMVC_FAMILY,$9 }' \
       CMVC_FAMILY=${CMVC_FAMILY} ${tmp_file} >> ${ABSTRACTLIST}

   # adding features to abstractlist file
   Report -view featureview -raw -where \
	"name in ( ${defectlist%,} )" > ${tmp_file}
   [[ $? -ne 0 ]] && log +l -x "Writing abstractlist file: ${ABSTRACTLIST}"
   awk -F"|" ' { printf "%6s:%s  %s\n",$2,CMVC_FAMILY,$7 }' \
       CMVC_FAMILY=${CMVC_FAMILY} ${tmp_file} >> ${ABSTRACTLIST}

   rm -f ${tmp_file}
   return 0
}

################################################################################
#
# NAME        : clean_up
#
# FUNCTION    : Clean up the program environment for user interrupt or execution
#               failure.
#
# SYNTAX      : terminate
#
# INPUT       : None.
#
# RETURNS     : Never returns.
#
# SIDE EFFECTS: None.
#
################################################################################
function clean_up
{
   if [[ -n "${BLDCYCLE}" && -n "${CMVC_RELEASE}" ]]
   then
      DeleteStatus ${_TYPE} ${T_PREBUILD} \
                   ${_BLDCYCLE} ${BLDCYCLE} \
                   ${_RELEASE} ${CMVC_RELEASE} \
                   ${_SUBTYPE} ${S_PTFSETUP} -F
      bldsetstatus ${_TYPE} ${T_PREBUILD} \
                   ${_BLDCYCLE} ${BLDCYCLE} \
                   ${_RELEASE} ${CMVC_RELEASE} \
                   ${_SUBTYPE} ${S_PTFSETUP} \
                   ${_STATUS} ${ST_FAILURE}
   fi
   rm -f ${tmp_file} ${tmp_file2}
   exit -1
}

################################################################################
#
# NAME        : usage
#
# FUNCTION    : Display usage statement.
#
# SYNTAX      : usage
#
# INPUT       : None.
#
# RETURNS     : Never returns.
#
# SIDE EFFECTS: Terminates program.
#
################################################################################
function usage
{
   print -u2 "Usage: ptfsetup [ -a ] [ -c BLDCYCLE ] [ -h HISTORYDIR ] \c"
   print -u2 "[ -l \"LPP ...\" ]"
   print -u2 "                [ -r RELEASE ] [ -n ] [ -u UPDATEDIR ] \c"
   print -u2 "[ [ -b | -f ] name(s) ... ]"
   exit 1
}

################################################################################
#
# Beginning of MAIN
#
################################################################################

. bldkshconst
. bldinitfunc
. bldkshfunc
. bldloginit
. bldhostsfile
. bldnodenames

typeset -x BLDCYCLE="${BLDCYCLE}"	# CMVC build cycle, default value from
					# environment
typeset -x LEVELNAME="${LEVELNAME}"     # CMVC level name, default value from
					# environment
typeset CHANGEVIEW=""			# Pathname of 'changeview' file
typeset ABSTRACTLIST=""			# Pathname of 'abstractlist' file
typeset ALL_DEFECTS=""			# Pathname of 'all_defects' file
typeset DEFECTS=""			# Pathname of 'defects' file
typeset LOCAL_HISTORY			# History directory on machine.
typeset PTFOPTIONS=""			# Pathname of 'ptfoptions' file
typeset -x CMVC_FAMILY=""		# CMVC family, will be set by call
					# to bldhostsfile.
typeset -x CMVC_RELEASE="${CMVC_RELEASE}" # CMVC release name, default value 
					# from environment
typeset -ir VALUE_LIST_SIZE=100		# Number of entries supplied within ()
					# to Report command
typeset -x a_option=""			# append option specified
typeset -x b_option=""			# User supplied the -b command line
					# option  Null if not entered  "true"
					# if entered
typeset -x no_update=""			# don't copy update info if non-null
typeset bad_option=""			# "true" if user entered unrecognized
					# option or supplied invalid parameter
typeset lvlname=""			# Loop variable
typeset command_line_list=""		# Arguments from command line that
					# are not associated with a command
					# line option
typeset -i defect_count=0		# Counter for defects processed, Report
					# can only accept one hundred defects
					# within '()' at a time
typeset defect=""			# Current defect
typeset defectfile=""			# User supplied file with defects
typeset defectlist=""			# User supplied comma separated list of
					# defects
typeset directory=""			# Temporary, holds directory name
typeset file=""				# Loop variable
typeset logfile=""			# File logging results will be written
					# to.
typeset option=""			# Current option being checked
typeset tmp_file="$(bldtmppath)/ptfsetup_$$"
typeset tmp_file2="$(bldtmppath)/ptfsetup2_$$"
					# Temporary file

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
   while getopts ":ab:c:f:h:r:u:nl:" option
   do
      if [ -n "${command_line_list}" ]
      then
	 command_line_list=${command_line_list%$*}
      fi
      case $option in 
	  a) a_option="true";;
          b) LEVELNAME="${OPTARG}"
             b_option="true";;
          c) BLDCYCLE="${OPTARG}";;
          f) defectfile="${OPTARG}";;
	  h) AFS_HISTORY="${OPTARG}";;
	  l) LPP_LIST="${OPTARG}";;
	  n) no_update="true";;
          r) CMVC_RELEASE="${OPTARG}";;
	  u) UPDATE_DIR="${OPTARG}";;
          :) print -u2 "Argument must be supplied with -${OPTARG}."
             bad_options="true";;
         \?) print -u2 "Unknown option -${OPTARG}."
             bad_options="true";;
          *) echo $option ;;
      esac
   done
   if [ ${OPTIND} -eq 1 ]
   then
      # getopts did not find option, assume rest of arguments belong with
      # -b, or -f option
      [[ -z "${command_line_list}" ]] && command_line_list="$*"
      shift 1
   else
      # getopts found an option, shift off all data processed
      shift ${OPTIND}-1
      OPTIND=1
   fi
done
[[ -n "${b_option}" && -n "${defectfile}" ]] && \
   print -u2 "ptfsetup: -b and -f cannot both be supplied." && \
   bad_options="true"
[[ "${bad_options}" = "true" ]] && usage

# LEVELNAME is a special case, need to get its value completely determined 
# before entering bldinit, or we may prompt for it when we should not
if [[ -n "${defectfile}" ]]
then
   # Don't prompt for LEVELNAME if -f option supplied
   [[ -z "${LEVELNAME}" ]] && LEVELNAME="dummy_value"
elif [[ -n "${b_option}" ]]
then
   # -b option entered, LEVELNAME has first argument, rest of list contained
   # in ${command_line_list}
   LEVELNAME="${LEVELNAME} ${command_line_list}"
elif [[ -n "${command_line_list}" ]] 
then
   # no -b option, but command line arguments, LEVELNAME has no arguments,
   # set LEVELNAME to the arguments
   LEVELNAME="${command_line_list}"
fi

bldinit ptfsetup
chksetbecome
[[ "${no_update}" != "true" ]] && chksetklog

# Any exit call from this point will be treated as a error
trap 'clean_up; trap - EXIT; exit 1' EXIT INT QUIT HUP TERM

logfile=$(get_log_file ptfsetup $(bldlogpath))
logset -c ptfsetup -C ptfsetup -F ${logfile}

bldhostsfile get_afs_base "${TRUE}"
[[ $? -ne 0 ]] && exit 1
typeset AFS_HISTORY="${HOSTSFILE_AFSBASE}/HISTORY"
					# History directory in AFS.
typeset	UPDATE_DIR="${HOSTSFILE_AFSBASE}/UPDATE"
					# pathname of update info

bldhostsfile ${CMVC_RELEASE} "${TRUE}"
[[ $? -ne 0 ]] && exit 1
CMVC_FAMILY="${HOSTSFILE_CMVCFAMILY}"

directory=$(bldreleasepath "${CMVC_RELEASE}")
ALL_DEFECTS="$(bldglobalpath)/all_defects"
CHANGEVIEW="${directory}/changeview"
REVISELIST="${directory}/reviselist"
RENAMELIST="${directory}/renamelist"
ABSTRACTLIST="$(bldglobalpath)/abstractlist"
LOCAL_HISTORY="$(bldhistorypath)"
LOCAL_UPDATE="$(bldupdatepath)"
DEFECTS="${directory}/defects"
PTFOPTIONS="$(bldhistorypath)/ptfoptions"
if [ -z "${a_option}" ]
then
	rm -f ${CHANGEVIEW} ${DEFECTS}
fi
touch ${CHANGEVIEW} ${DEFECTS} ${PTFOPTIONS} ${REVISELIST} ${RENAMELIST} \
      ${ALL_DEFECTS}

log -b "Entered ptfsetup `date`"
log -b "BLDCYCLE is ${BLDCYCLE}"
log -b "RELEASE is ${CMVC_RELEASE}"
log -b "Logging File is ${logfile}"

# Delete status of bldmid so it will later rerun in subptf
DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDMID $_BLDCYCLE $BLDCYCLE

log -b "Calling bldstatus for ${BLDCYCLE}"
bldstatus -t ${BLDCYCLE}
[[ $? -ne 0 ]] && log +l -x "Calling bldstatus for build cycle ${BLDCYCLE}"

#
# Figure out how the list of defects was supplied and create ${DEFECTS}.
#
log -b "Generating defect list"
if [ -n "${defectfile}" ]
then
   for file in ""${defectfile} ${command_line_list}
   do
      if [ -s ${file} ]
      then
         cat ${file} >> ${DEFECTS}
         [[ $? -ne 0 ]] && log +l -x "Writing defect file: ${DEFECTS}"
         log -b "Processed file : ${file}"
      else
         log +l -w "File ${file} does not exist"
      fi
   done
   sort -u ${DEFECTS} -o ${DEFECTS}
else
   for lvlname in ""${LEVELNAME}
   do
      Report -view levelview -raw -where \
         "name = '${lvlname}' and releasename = '${CMVC_RELEASE}'" > ${tmp_file}
      [[ $? -ne 0 ]] && log +l -x "Writing temporary file: ${tmp_file}"
      if [ ! -s ${tmp_file} ]
      then
         log +l -w "Buildname ${lvlname} does not exist for ${CMVC_RELEASE}"
      else
         Report -view levelmemberview -raw -where \
            "levelname = '${lvlname}' and releasename = '${CMVC_RELEASE}'" \
	    > ${tmp_file}
         [[ $? -ne 0 ]] && log +l -x "Writing defect file: ${DEFECTS}"
         cut -d\| -f3 ${tmp_file} > ${tmp_file2}
         if [ -s ${tmp_file2} ]
         then
            cat ${tmp_file2} >> ${DEFECTS}
            log -b "Processed levelname : ${lvlname}"
         else
            log +l -w \
               "Buildname ${lvlname} for ${CMVC_RELEASE} has no associated defects."
         fi
      fi
   done
   rm -f ${tmp_file} ${tmp_file2}
fi

log -b "Calling CheckSymptom for ${CMVC_RELEASE}"
if [[ -n "${LEVELNAME}" && "${LEVELNAME}" != "dummy_value" ]]
then
   for lvlname in ""${LEVELNAME}
   do
      export LEVELVERSION="${lvlname}"
      CheckSymptom -r ${CMVC_RELEASE} -a
      [[ $? -ne 0 ]] && log +l -x "Calling CheckSymptom for release ${CMVC_RELEASE}"
   done
else
   for file in ""${defectfile} ${command_line_list}
   do
      CheckSymptom -r ${CMVC_RELEASE} -a ${file}
      [[ $? -ne 0 ]] && log +l -x "Calling CheckSymptom for release ${CMVC_RELEASE}"
   done
fi

#
# Using the defect list, gather up defects VALUE_LIST_SIZE at a time, then call
# add_to_changeview to add data to ${CHANGEVIEW}.
#
if [ -s "${DEFECTS}" ]
then
   log -b "Creating changeviews, abstractlist and defect data"
   while read defect
   do
      defect_count=${defect_count}+1
      defectlist="${defect},${defectlist}"
      if [ ${defect_count} -eq ${VALUE_LIST_SIZE} ]
      then
         add_to_changeview
         add_to_abstractlist
         defect_count=0
         defectlist=""
      fi
      print "${defect} ${CMVC_RELEASE} ${CMVC_FAMILY}" >> ${ALL_DEFECTS}
   done < ${DEFECTS}
   [[ -n "${defectlist}" ]] && (add_to_changeview; add_to_abstractlist)
   sort -t' ' -o ${ALL_DEFECTS} -u +1 -2 +0 -1n ${ALL_DEFECTS}
   [[ $? -ne 0 ]] && log -e "Sort of ${ALL_DEFECTS} failed."
fi

# copy update information unless the -n option was specified
if [ "${no_update}" != "true" ]
then
   if [ -n "${LPP_LIST}" ]
   then
      log -b "Copying UPDATE information for LPPs: \"$LPP_LIST\""
      for lpp in ${LPP_LIST}
      do
         log +l -f "Copying ${UPDATE_DIR}/${lpp}"
         if [ -d "${UPDATE_DIR}/${lpp}" ]
         then
            [[ ! -d "${LOCAL_UPDATE}/${lpp}" ]] && mkdir ${LOCAL_UPDATE}/${lpp}
            cp -r ${UPDATE_DIR}/${lpp}/* ${LOCAL_UPDATE}/${lpp}
            [[ $? -ne 0 ]] && log +l -e "Copy failed for ${LOCAL_UPDATE}/${lpp}"
         fi
      done
   else
      log -b "Copying UPDATE information for all LPPs"
      for directory in ${UPDATE_DIR}/*
      do
         log +l -f "Copying ${directory}"
         cp -r ${directory} ${LOCAL_UPDATE}
         [[ $? -ne 0 ]] && log +l -e "Copy failed for ${directory}"
      done
   fi
   log -b "Copying HISTORY information"
   for file in abstracts abstracts.raw memo_info memo_info.raw ptfoptions
   do
      log +l -f "Copying ${AFS_HISTORY}/${file}"
      cp -r ${AFS_HISTORY}/${file} ${LOCAL_HISTORY}/${file}
      [[ $? -ne 0 ]] && \
         log +l -e "Copy failed to file ${LOCAL_HISTORY}/${file}"
   done
fi

DeleteStatus ${_TYPE} ${T_PREBUILD} \
             ${_BLDCYCLE} ${BLDCYCLE} \
             ${_RELEASE} ${CMVC_RELEASE} \
             ${_SUBTYPE} ${S_PTFSETUP} -F
bldsetstatus ${_TYPE} ${T_PREBUILD} \
             ${_BLDCYCLE} ${BLDCYCLE} \
             ${_RELEASE} ${CMVC_RELEASE} \
             ${_SUBTYPE} ${S_PTFSETUP} \
             ${_STATUS} ${ST_SUCCESS}
 
log -b "Leaving ptfsetup `date`."

# Any exit call from this point will not be treated as a error.
trap - EXIT

rm -f ${tmp_file} ${tmp_file2}

exit 0

