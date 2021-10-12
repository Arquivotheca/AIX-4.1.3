#! /bin/ksh
# @(#)74	1.2  src/bldenv/bldtools/bldtmppath.sh, bldtools, bos412, GOLDA411a 1/21/92 17:58:00
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldtmppath 
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
# NAME: bldtmppath
#
# FUNCTION: Return the path of temporary working directory. Defaults
#	    to /tmp.
#
# INPUT: BLDTMP (environment) - environmental variable specifying temp directory
#
# OUTPUT: temporary path written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
. bldloginit
  [[ $# = 0 ]] || log -x -c$0 +l "illegal syntax"
  print ${BLDTMP:-/tmp}
