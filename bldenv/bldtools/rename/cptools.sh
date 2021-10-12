#!/bin/ksh
# @(#)61        1.8  src/bldenv/bldtools/rename/cptools.sh, bldtools, bos412, GOLDA411a 8/18/93 09:43:42
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: cptools
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
   rm -f ${tmp_message_file}
   logset -r

   return 0
}

#
# NAME: copy_directory
#
# FUNCTION: Copy contests of the source directory to the destintation
#           directory.
#
# INPUT: source ($1) - the directory to copy from, format is 
#                      [[User@]Host:]directory.
#        destination ($2) - the directory to copy to, format is
#                           [[User@]Host:]directory.
#
# OUTPUT: none.
#
# SIDE EFFECTS: Contents of destination directory will be overwritten.
#
# RETURNS: 0 on success, <>0 on failure.
#
function copy_directory
{
   typeset -i RC=0
   typeset    destination=$2
   typeset    source=$1

   log -f "Copy contents of ${source} to ${destination}"

   check_directory ${source}
   if [ $? -eq 0 ]
   then
      make_directory ${destination}
      if [[ $? -ne 0 ]] 
      then
         log +l -e "Cannot create ${destination}"
         return 1
      fi
      treecopy ${source} ${destination} 1> /dev/null
      RC=$?
      if [[ ${RC} -ne 0 ]]
      then
         log +l -e "Copy from ${source} to ${destination} returned ${RC}"
         return ${RC}
      fi
   else
      log +l -e "Directory ${source} does not exist."
      return 1
   fi

   log -f "Copied contents of ${source} to ${destination}"

   return 0
}

#
# NAME: read_cptools.dat
#
# FUNCTION: Read entries for the cptools.dat file.
#
# INPUT: type ($1) - type of data to be read from the cptools.dat file.
#
# OUTPUT: Entries from the cptools.dat file will be displayed to stdout.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always
#
function read_cptools_dat
{
   typeset    type=$1

   # the s/\\/\\\\\\/g is to insure that all \ from the file are still present
   # when the eval occurs.
   grep "^${type}:" ${BLDENV}/usr/bin/cptools.dat | \
   sed -e "s/${type}://g" -e 's/\\/\\\\\\/g' | \
   while read line
    do
       print $(eval print -r ${line})
    done

   return 0
}

. bldkshconst
. bldinitfunc
. bldkshfunc
. bldloginit
. bldhostsfile
. bldnodenames

typeset -u ANS                                  # User response from prompts,
                                                # always convert to uppercase
                                                # for checks.
readonly   NO='N*'
readonly   QUIT='Q*'
readonly   YES='Y*'
typeset    dirname=""				# The $(dirname) of directory
						# being copied to.  Used to
						# display status of copies.
typeset    tmp_message_file=${BLDTMP}/cptools.$$

chksettop
chksetbuilder
chksetklog

# Store a copy of file descripitor 2 in file descripitor 5.  The logging
# routines alter descripitor 2 and will cause the ${EDITOR} command to
# fail below.
exec 5<&2

# Determine where in afs the build environment is located.
bldhostsfile cptools_bldenv "${TRUE}"
[[ $? -ne 0 ]] && log +l -x "Cannot continue"

confirm "Copy build environment to AFS? (y/n): \c"
[[ $? -ne 0 ]] && exit 1

# Any exit call from this point will be treated as a error
trap 'clean_up; trap - EXIT; exit 1' EXIT INT QUIT HUP TERM

logset -c cptools -C cptools -F $(bldlogpath -b)/cptools
log -f "Cptools being run by ${BUILDER}"

read_cptools_dat file | \
   while read source destination
   do
      log -f "Copy ${source} to ${destination}"
      rcp -p ${source} ${destination}
      if [[ $? -ne 0 ]]
      then
         log +l -e "Copy of ${source} to ${destination} failed"
      else
         log -f "Copied ${source} to ${destination}"
      fi
   done

read_cptools_dat directory | \
   while read source destination
   do
      if [[ "$(dirname ${destination})" != "${dirname}" ]]
      then
         dirname="$(dirname ${destination})"
         log "Copying to ${dirname}"
      fi
      copy_directory ${source} ${destination}
   done

confirm "Notify area builders? (y/n): \c"
[[ $? -ne 0 ]] && exit 1

print - "$(read_cptools_dat message)" > ${tmp_message_file}

confirm  "Modify message to area builders? (y/n): \c"
[[ $? -eq 0 ]] && \
{
   # Get file descripitor 2 back to state before logging modified it.
   exec 2<&-
   exec 2<&5
   vi ${tmp_message_file}
}

exec 5<&-

mail -s "$(read_cptools_dat subject)" $(read_cptools_dat builder) < \
   ${tmp_message_file}

# Any exit call from this point will not be treated as a error.
trap - EXIT

clean_up

exit 0
