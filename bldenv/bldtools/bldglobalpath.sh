#! /bin/ksh
# @(#)43	1.2  src/bldenv/bldtools/bldglobalpath.sh, bldtools, bos412, GOLDA411a 1/21/92 17:57:01
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldglobalpath 
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
# NAME: bldglobalpath
#
# FUNCTION: Return the path of the global working directory for builds.
#
# INPUT: TOP (environment) - the top of the build tree
#	 BLDCYCLE (environment) - the build cycle name (e.g. 9125)
#
# OUTPUT: global path written to stdout
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
  [[ -n "$BLDCYCLE" ]] || log -x +l -c$0 "BLDCYCLE undefined"
  print $TOP/PTF/$BLDCYCLE
  exit $rc
