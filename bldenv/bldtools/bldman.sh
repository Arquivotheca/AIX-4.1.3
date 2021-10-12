#! /bin/ksh
# @(#)05 1.3 src/bldenv/bldtools/bldman.sh, bldtools, bos412, GOLDA411a 6/8/94 15:42:42
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldman
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bldman
#
# FUNCTION:  Display build environment manual pages or system manual
#            pages.
#
# SYNTAX: bldman [ ? | entry ... ]
#
# INPUT: entry - index to type of information to be displayed
#
# OUTPUT: 0 means success, <>0 means failure
#
# SIDE EFFECTS: none
#

#
# NAME: error
# FUNCTION: Display error message and call terminate.
# SYNTAX: error string ...
# INPUT: string - character string to be displayed
# RETURNS: never returns
# SIDE EFFECTS: terminates program
#
function error
{
   print $*
   terminate
}

#
#
# NAME: terminate
# FUNCTION: Clean up the program environment for user interrupt or execution
#           failure.
# SYNTAX: terminate
# INPUT: ${tmp_file} - temporary file name that may have been created.
# RETURNS: never returns
# SIDE EFFECTS: terminates program
#
function terminate
{
   exit 1
}

#
#
# NAME: error
# FUNCTION: Display error message and call terminate.
# SYNTAX: error string ...
# INPUT: string - character string to be displayed
# RETURNS: never returns
# SIDE EFFECTS: terminates program
function usage
{
   print "\nUsage : bldman [ ? | entry ... ]"
   print "\nCurrent possible entries are:"
   ls -C ${MAN_PAGE_LOCATION}
}

#
# Beginning of MAIN
#
# Directory where man pages live
typeset -r MAN_PAGE_LOCATION=${ODE_TOOLS}/usr/share/man
typeset -x PAGER=${PAGER}			# Paging utility to use
typeset user_response				# Used to get response when no
						# man entry is found

trap terminate INT QUIT HUP TERM

[[ -z "${ODE_TOOLS}" ]] && error "The environment variable ODE_TOOLS is not set."

cd ${MAN_PAGE_LOCATION} 
[[ $? = 0 ]] || \
   error "Directory for man pages does not exist : ${MAN_PAGE_LOCATION}."

[[ -z "${PAGER}" ]] && PAGER=more

#
# If no arguments issue the usage message
#
if [ $# -eq 0 -o "$1" = "?" ]
then
   echo "$(usage)" | ${PAGER}
   terminate
fi
 
#
# Loop through the arguments provided.  If entry is found display it through
# $(PAGER).  If entry is not found, try locating entry with man, if man fails
# make them confirm so they do not miss the error message if multiple entries
# were supplied.
# 
for entry in $*
do
   if [ -f ${entry} ]
   then
      ${PAGER} ${entry}
   else
      print "bldman: \"${entry}\" not found, calling man."
      man ${entry}
      if [ $? -ne 0 ]
      then
         print "Press any key to continue :\c"
         read user_response
      fi
   fi
done

exit 0
