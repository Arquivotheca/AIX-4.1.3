#!/bin/ksh
# @(#)27        1.3  src/bldenv/bldtools/rename/blddeps.sh, bldtools, bos412, GOLDA411a 8/18/93 09:43:18
#
#   COMPONENT_NAME: BLDTOOLS
#
#   FUNCTIONS: blddeps
#              clean_up
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: clean_up
#
# FUNCTION: Clean up after execution.
#
# INPUT: none.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function clean_up
{
   cd -
   rm -fr ${work_directory}
   logset -r

   return 0
}

. bldkshconst
. bldinitfunc
. bldkshfunc
. bldloginit

test ${BLDTMP:=$(bldtmppath)}

typeset -x BLDCYCLE="dummy_buildcycle"	# Need set for bldglobalpath call.
typeset    bldcycle			# Build cycle the lmmakelist file
					# was created in.
typeset    date				# Date of current file being checked.
typeset    entry			# Loop variable.
typeset    filename			# Filename of current file being
					# checked.
typeset    last_update			# The 'ls -l' of the lmmakelist file
					# in ${TOP}/PTF/*/${CMVC_RELASE} that
					# has mostly recently been modified.
typeset    list_update_filename		# Filename of current file being
					# checked.
typeset    start			# ${TOP}/PTF
typeset    work_directory="${BLDTMP}/blddeps.$$"

chksettop

# Any exit call from this point will be treated as a error
trap 'clean_up; trap - EXIT; exit 1' EXIT INT QUIT HUP TERM

logset -c blddeps -C blddeps -F $(bldlogpath -b)/blddeps

make_directory ${work_directory}
[[ $? -ne 0 ]] && log +l -x "Cannot create directory ${work_directory}"
cd ${work_directory}

if [[ $# -eq 0 ]]
then
   [[ -z "${CMVC_RELEASE}" ]] && chksetrelease
   RELEASES="${CMVC_RELEASE}"
else
   RELEASES=$*
fi

start=$(dirname $(bldglobalpath))
for CMVC_RELEASE in ${RELEASES}
do
   log "Checking ${CMVC_RELEASE}"

   if [[ ! -f ${BLDENV}/usr/bin/blddeps${CMVC_RELEASE}.dat ]]
   then
      log +l -e "Cannot open file ${BLDENV}/usr/bin/blddeps${CMVC_RELEASE}.dat"
      continue
   fi

   #
   # Search for all lmmakelist files created for this release.  Files will be
   # returned order from most recently modified to oldest modified.  Pull first
   # file off list, this will be used as the date for the last time this 
   # release was built.
   #
   last_update=$(ls -lt ${start}/*/${CMVC_RELEASE}/lmmakelist.all* \
                    2> /dev/null \
                 | sed -n -e 1P 2> /dev/null)
   if [[ -z "${last_update}" ]]
   then
      log +l -e "Cannot find ${start}/*/${CMVC_RELEASE}/lmmakelist.all*"
      continue
   fi
   date=$(echo "${last_update}" | cut -c42-53)
   last_update_filename=$(echo "${last_update}" | cut -c55-)
   bldcycle=${last_update_filename#${start}/}
   bldcycle=${bldcycle%%/*}
   log +l "Last ${CMVC_RELEASE} build was ${date} in build cycle ${bldcycle}"

   #
   # Generate a command file.  Each command written to the file will be a
   # list of files to sort by modification time.  Each command will contain
   # a file to indicate when the last build for the release occurred
   # ( ${last_update_filename} ).  When the commands from the command file
   # are executed any file returned before ${last_update_filename} has been
   # modified since the last build of release.
   #
   # the s/\\/\\\\\\/g is to insure that all \ from the file are still present
   # when the eval occurs.
   #
   sed -e 's/\\/\\\\\\/g' ${BLDENV}/usr/bin/blddeps${CMVC_RELEASE}.dat \
   | while read entry
     do
        print $(eval print -r ${entry})
     done \
   | xargs -n100 echo /bin/ls -lt ${last_update_filename} > blddepswork
   if [[ $? -ne 0 ]]
   then
      log +l -e "Failure ordering file ${file}"
      continue
   fi

   #
   # Process each command in the work file.  Generate a warning for all
   # files that appear before ${last_update_filename} in the results from
   # executing a command from the work file.
   #
   while read command
   do
      $command \
      | while read entry
        do
           filename=$(echo "${entry}" | cut -c55-)
           if [[ "${filename}" != "${last_update_filename}" ]] 
           then
              date=$(echo "${entry}" | cut -c42-53)
              log +l -w "${filename} is dated ${date}"
           else
              break
           fi
        done
   done < blddepswork
done

# Any exit call from this point will not be treated as a error.
trap - EXIT

clean_up

exit 0

