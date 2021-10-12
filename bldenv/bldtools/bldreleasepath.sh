#! /bin/ksh
# @(#)59	1.1  src/bldenv/bldtools/bldreleasepath.sh, bldtools, bos412, GOLDA411a 5/26/92 14:36:52
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldreleasepath 
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
# NAME: bldreleasepath
#
# FUNCTION: Return the PTF/selective fix working directory path for a release.
#
# INPUT: release (paramter) - the CMVC build release name
#	 BLDCYCLE (environment) - current build cycle name
#
# OUTPUT: release path written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
. bldloginit
  [[ $# = 1 ]] || log -x +l -c$0 "illegal syntax"
  typeset release=$1
  [[ -n "$TOP" ]] || log -x +l -c$0 "TOP undefined"
  [[ -n "$BLDCYCLE" ]] || log -x +l -c$0 "BLDCYCLE undefined"
  print $TOP/PTF/$BLDCYCLE/$release
