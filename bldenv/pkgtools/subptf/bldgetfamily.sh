#! /bin/ksh
# @(#)97	1.1  src/bldenv/pkgtools/subptf/bldgetfamily.sh, pkgtools, bos412, GOLDA411a 6/24/93 15:08:44
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldgetfamily
#	     
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: bldgetfamily
#
# FUNCTION:  Finds the CMVC family for the given release.
#
# INPUT: release (parameter) - the current CMVC level release
#
# OUTPUT: familyName
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or non-zero (failure)
#


. bldloginit
. bldkshconst
. bldhostsfile
. bldnodenames

  rc=$UNSET; logset +l -c$0  # init log command

  trap 'logset -r; trap - EXIT; exit $rc' \
	EXIT HUP INT QUIT TERM
  [[ $# = 1 ]] || log -x "illegal syntax"
  typeset release=$1 # command line parameter

  bldhostsfile $release "$TRUE"
  if [[ $? -eq 0  && -n "$HOSTSFILE_CMVCFAMILY" ]]
  then
        print $HOSTSFILE_CMVCFAMILY
  else
        log -e "cmvc family for release $release not found."
  fi
  
  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit

