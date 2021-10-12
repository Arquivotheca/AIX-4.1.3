#! /bin/ksh
# @(#)98	1.11  src/bldenv/pkgtools/subptf/bldorderstatus.sh, pkgtools, bos412, GOLDA411a 2/21/94 16:39:07
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldorderstatus
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

. bldinitfunc
. bldloginit
. bldkshconst

#
# NAME: 
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
  rc=$UNSET; lvar="+l -c$0"
  [[ -n $DEBUG ]] && log $lvar -b "entering ($@)"
  [[ $# = 1 ]] || log $lvar -x "illegal syntax"
  typeset statusfile=$1 # command line parameters
  trap 'bldlock -u $STATUS $PERM; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM
  bldlock -l $STATUS $PERM || log -x "unable to get $STATUS lock"


  STATUSLIST=$(bldtmppath)/status$$; readonly STATUSLIST

  [[ ! -f ${RELEASE_LIST} ]] && \
     log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

  # grep out the current BLDCYCLE
  rm -f $STATUSLIST $STATUSLIST.1 $STATUSLIST.2 $STATUSLIST.3
  # sort the status file, removing duplicate entries, according to bldcycle
  # then date.
  sort -u -t'|' +1 -2 +5 -6 +0 $statusfile >$STATUSLIST.3
  grep $BLDCYCLE $STATUSLIST.3 > $STATUSLIST.1
  # grep out the release lines in the correct order
  sed < ${RELEASE_LIST} '/^ *$/d' | \
  while read release
  do
	grep -w $release $STATUSLIST.1 >> $STATUSLIST
	grep -v -w $release $STATUSLIST.1 >> $STATUSLIST.2
	mv -f $STATUSLIST.2 $STATUSLIST.1
  done
  cat $STATUSLIST.1 >> $STATUSLIST
  rm -f $STATUSLIST.1
  grep -v $BLDCYCLE $STATUSLIST.3 >> $STATUSLIST
  rm -f $STATUSLIST.3
  mv $statusfile $statusfile.old; mv $STATUSLIST $statusfile

  [[ -n $DEBUG ]] && log $lvar -b "exiting"
  [[ $rc = $UNSET ]] && rc=$SUCCESS
  exit $rc
