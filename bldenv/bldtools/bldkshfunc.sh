#
# @(#)99	1.1  src/bldenv/bldtools/bldkshfunc.sh, bldtools, bos412, GOLDA411a 8/18/93 11:21:44
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: check_directory
#            get_log_file
#            make_directory
#            parse_remote_pathname
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#

#
# NAME: check_directory
#
# FUNCTION: Check to see if directory supplied exists.
#
# INPUT: directory ($1) - the directory to check, format is 
#                         [[User@]Host:]directory.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if directory exists, <>0 if directory does not exist.
#
function check_directory
{
   typeset -i RC=0			# Return code, composed of return code
					# and directory that could not be
					# created.
   typeset -x host			# Host to create directory on.
   typeset -x path			# Directory to check.
   typeset -x user			# User the directory should be
					# created by if host is a remote
					# system.
   parse_remote_pathname $1

   if [[ -n "${host}" ]]
   then
      RC=$(rsh ${host} -l ${user} -n \
           "if [ ! -d ${path} ] ;\
            then \
               print 1 ;\
            else \
               print 0 ;\
            fi" )
   else
      [[ ! -d ${path} ]] && RC=1
   fi

   return ${RC}
}

#
# NAME: get_log_file
#
# FUNCTION: Determine next log file to use.
#
# INPUT: filename ($1) - The filename for the log file.  A sequence number
#                        will be appended to the file name.
#        path ($2) - The directory path to where the log file should be
#                    created.
#
# OUTPUT: Full path name of the log file.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function get_log_file
{
   typeset -i num=1
   typeset    filename=$1
   typeset    path=$2

   make_directory ${path}
   [[ $? -ne 0 ]] && \
      log +l -c get_log_file -x "Cannot create the directory ${path}"

   while [[ -r ${path}/${filename}.${num} ]]
   do
      let num=num+1
   done
   echo ${path}/${filename}.${num}

   return 0
}

#
# NAME: make_directory
#
# FUNCTION: Create the directory supplied.
#
# INPUT: directory ($1) - the directory to create, format is 
#                         [[User@]Host:]directory.  If directory
#                         begins with a '/', the file is created from '/'
#                         down; otherwise it is created from the current
#                         directory.
#
# OUTPUT: none.
#
# SIDE EFFECTS: Directory supplied will be created.
#
# RETURNS: 0 on success, <>0 on failure.
#
function make_directory
{
   typeset    RC			# Return code, composed of return code
					# and directory that could not be
					# created.
   typeset    directory			# Directory being created.
   typeset -x host			# Host to create directory on.
   typeset -x path			# Directory to create.
   typeset    sub_directory		# Used to loop through directory
					# sub parts.
   typeset -x user			# User the directory should be
					# created by if host is a remote
					# system.

   parse_remote_pathname $1

   # Arguments to the for subdirectory loop is a tad complex because of
   # handling both relative and absolute path names and the way they are
   # pulled apart with ${}.  The arguments on the sub_directory for loop
   # line are first directory to check followed by all other directories.
   #    If $path begins with a /, ${1%%/*} is NULL, ${1#*/} is full path, 
   #    this will cause the initial check to be on the / directory.
   #
   #    If $1 does not begin with a /, ${1%%/*} is first subdirectory,
   #    ${1#*/} will be rest of path, this will cause initial check to
   #    be on the ${1%%/*} directory.
   #
   if [[ -n "${host}" ]]
   then
      RC=$(rsh ${host} -l ${user} -n \
           "for sub_directory in ${path%%/*}\"\" `IFS=/; echo ${path#*/}`\"\" ;\
            do \
               directory=\"\${directory}\${sub_directory}/\" ;\
               if [ ! -d \${directory} ] ;\
               then \
                  mkdir \${directory} 1> /dev/null 2>&1 ;\
                  RC=\$?
                  [[ \${RC} != 0 ]] && \
                  { \
                     print \${RC} \${directory} ;\
                     break ;\
                  } ;\
                fi ;\
            done; \
            print 0 \${directory}" )
   else
      for sub_directory in ${path%%/*}"" `IFS=/; echo ${path#*/}`""
      do
         directory="${directory}${sub_directory}/"
         if [ ! -d ${directory} ]
         then
            mkdir ${directory} 1> /dev/null 2>&1
            RC=$?
            [[ ${RC} != 0 ]] && \
            {
               RC="$? ${directory}"
               break
            }
         fi
      done
   fi

   if [[ "${RC%%\ *}" -ne "0" ]]
   then
      log +l -c make_directory -e "Cannot create directory ${RC##*\ }"
   fi

   return ${RC%%\ *}
}

#
# NAME: parse_remote_pathname
#
# FUNCTION: Parse a remote filename into the user, host, and path components.
#
# INPUT: remote_name ($1) - a remote path name in the form [[User@]Host:]path.
#                           No validity check is made, if form is not correct
#                           the values are undefined.
#
# OUTPUT: host (gloabal) - contains the Host name.
#         path (global) - contains the path name.
#         user (global) - contains the User name.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
function parse_remote_pathname
{
   host=${1%%:*}
   path=${1##*:}

   if [[ "${host}" = "$1" ]]
   then
      # No user@host: specified.
      host=""
      user=""
   else
      # There is a user@host: specified.
      host=${host##@*}
      if [[ "$(uname -n)" = "${host}" ]]
      then
         # The host specified is the current machine, no need to use
         # remote shell.
         host=""
      else
         user=${host%%@*}
         if [[ "${user}" = "${host}" ]]
         then
            # No user specified, use euid if defined, otherwise use uid.
            id="$(id)"
            user=$(expr "${id}" : ".*euid=[0-9][0-9]*(\([^)]*\)")
            [[ -z "${user}" ]] && \
               user=$(expr "${id}" : ".*uid=[0-9][0-9]*(\([^)]*\)")
         fi
         host=${host##*@}
      fi
   fi

   return 0
}
