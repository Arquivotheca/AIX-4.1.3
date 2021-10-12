#! /bin/ksh
# @(#)39	1.7  src/bldenv/pkgtools/subptf/bldenvlist.sh, pkgtools, bos412, GOLDA411a 11/17/92 11:19:02
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldenvlist
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

. bldloginit
. bldkshconst

#
# NAME: bldenvlist
#
# FUNCTION: Return list of new build environment files for a release.  
#
# INPUT: release (parameter) - the CMVC build release
#	 lmbldenvlist (file) - bldenv list of files from v3bld/make
#
# OUTPUT: the "normalized" build environment list is written to stdout
#	  (one filename per line)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or non-zero (failure)
#
  rc=$UNSET; logset +l -c$0  # init log command
  logarg="-e"
  [[ $# = 1 ]] || log -b "illegal syntax"
  typeset release=$1  # command line parameter
  TMP=$(bldtmppath)/bldenvlist.tmp$$
  UNIQUE=$(bldtmppath)/bldenvlist.unique$$

  trap 'rm -f $TMP $UNIQUE; logset -r; trap - EXIT; exit $rc' \
	EXIT HUP INT QUIT TERM

  bldnormalize $(bldreleasepath $release)/lmbldenvlist > $TMP || 
	{ [[ $? -ne 1 ]] && logarg="-x"; log ${logarg[$?]} "normalizing bldenv names"; }
  sort -u $TMP > $UNIQUE || log -x "removing duplicates - sorting"
  cat $UNIQUE

  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit
  

