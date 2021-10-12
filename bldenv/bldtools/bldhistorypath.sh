#! /bin/ksh
# @(#)47	1.3  src/bldenv/bldtools/bldhistorypath.sh, bldtools, bos412, GOLDA411a 1/21/92 17:57:09
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldhistorypath 
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
# NAME: bldhistorypath
#
# FUNCTION: Return the path of the build history directory.
#
# INPUT: 
#
# OUTPUT: build history path is written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
. bldloginit
  rc=$SUCCESS
  [[ $# = 0 ]] || log -x +l -c$0 "illegal syntax"
  [[ -n "$TOP" ]] || log -x +l -c$0 "TOP undefined"
  print $TOP/HISTORY
  exit $rc
