#! /bin/ksh
# @(#)90	1.3  src/bldenv/bldtools/bldlogpath.sh, bldtools, bos412, GOLDA411a 3/30/93 18:12:25
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldlogpath 
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
# NAME: bldlogpath
#
# FUNCTION: Return the selective fix working directory path for LOG files.
#
# INPUT: BLDCYCLE (environment) - current build cycle name.
#        TOP (environment) - top of the build tree.
#        display_log_base ($1) - if '-b' argument passed then only display
#                                base of the logging directories.  This is
#                                location of logs for tools that do not 
#                                require a build cycle.
#
# OUTPUT: log path written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#

. bldloginit
. bldkshconst

typeset display_log_base="${FALSE}"

while getopts :b option
do
   case ${option} in
      b)
         display_log_base="${TRUE}"
         ;;
      :)
         print -u2 "$OPTARG requires a value in bldlogpath"
         ;;
     \?)
         print -u2 "unknown option $OPTARG in bldlogpath"
         ;;
   esac
done
shift OPTIND-1

[[ $# = 0 ]] || log -x +l -c$0 "illegal syntax"
[[ -n "$TOP" ]] || log -x +l -c$0 "TOP undefined"

if [[ "${display_log_base}" = "${TRUE}" ]]
then
   print $TOP/LOG
else
   [[ -n "$BLDCYCLE" ]] || log -x +l -c$0 "BLDCYCLE undefined"
   print $TOP/LOG/$BLDCYCLE
fi
     

