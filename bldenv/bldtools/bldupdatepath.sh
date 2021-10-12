#! /bin/ksh
# @(#)77	1.4  src/bldenv/bldtools/bldupdatepath.sh, bldtools, bos412, GOLDA411a 1/21/92 17:58:11
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldupdatepath 
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
# NAME: bldupdatepath
#
# FUNCTION: Return the update path for lpps.
#
# INPUT: TOP (environment) - the top of the build tree
#
# OUTPUT: lpp path written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
#       v32bld must be executed first to set $TOP.
#
# RETURNS: 0 (successful) or 1 (failure)
#
. bldloginit
  rc=$SUCCESS
  [[ $# = 0 ]] || log -x +l -c$0 "illegal syntax"
  [[ -n "$TOP" ]] || log -x +l -c$0 "TOP undefined"
  print $TOP/UPDATE
  exit $rc
