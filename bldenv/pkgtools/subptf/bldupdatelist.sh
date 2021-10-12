#! /bin/ksh
# @(#)76	1.8  src/bldenv/pkgtools/subptf/bldupdatelist.sh, pkgtools, bos412, GOLDA411a 2/8/94 11:41:58
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldupdatelist
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
. bldkshconst	# define constants

#
# NAME: bldupdatelist
#
# FUNCTION: Return update files for a release.  Update files are shipable
#	    files which are binarily different in the current build cycle.
#
# INPUT: release (parameter) - CMVC level release name
#	 lmupdatelist (file) - update list of files from v3bld/make
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
  logset +l -c$0  # init log command
  [[ $# = 1 ]] || log -x "illegal syntax"
  logarg="-e"
  typeset release=$1  # command line parameter
  TMP=$(bldtmppath)/bldupdatelist.tmp$$
  SORTOUT=$(bldtmppath)/bldupdatelist.sortout$$
  rc=$UNSET

  trap 'rm -f $TMP $SORTOUT; logset -r; trap - EXIT; exit $rc' \
	EXIT HUP INT QUIT TERM

  bldnormalize $(bldreleasepath $release)/lmupdatelist > $TMP || 
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg +l -c$0 "normalizing update list"; }
  sort -u $TMP > $SORTOUT || log -x "removing duplicates - sorting"
  cat $SORTOUT 

  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit
~
