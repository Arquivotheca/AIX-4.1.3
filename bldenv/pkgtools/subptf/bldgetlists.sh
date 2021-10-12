#! /bin/ksh
# @(#)42	1.28  src/bldenv/pkgtools/subptf/bldgetlists.sh, pkgtools, bos412, GOLDA411a 11/23/92 16:35:49
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldgetlists
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
# NAME: bldgetlists
#
# FUNCTION: Selective fix requires multiple data lists for input.  These lists
#   exist for each release in the build cycle.  Before the lists can be used 
#   they must be "processed" so they will be in the correct form.  This command
#   processes all the lists for one release.
#
#   Normally, this command is called from bldgetalllists.
#
# INPUT: release (parameter) - the CMVC level release
#	 MAKELIST (file) - any previous makelist is used for optimization
#		           (see the bldmakelist command)
#
# OUTPUT: UPDATELIST (file) - list of binarily different ship files
#	  ENVLIST (file) - list of new bldenv files
#	  MAKELIST (file) - list of all make targets/dependents
#	  CHANGELIST (file) - list of source file changes for each defect
#	  REVISELIST (file) - list of CMVC file name changes
# 	  DEFECTLIST (file) - list of all defects
#
# NOTES: 1) the status file is indirectly read and updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (success) or non-zero (failure)
#

. bldloginit
. bldkshconst   # define constants

if [[ $# = 2 ]]
then
   logset +l -F $2 -c$0 -C"bldgetlists" -1$1 -2$BLDCYCLE  # init log command
else
   logset +l -c$0 -C"bldgetlists" -1$1 -2$BLDCYCLE  # init log command
fi

rc=$UNSET;
logarg="-e"

trap 'logset -r; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

[[ $# -ne 1 && $# -ne 2 ]] && log -x "illegal syntax"
release=$1

## if the next steps have already been done, they must be redone since new
## lists are being created; so delete the status for the later commands
DeleteStatus $_TYPE $T_BLDPTF \
             $_SUBTYPE $S_BLDGETLISTS \
             $_BLDCYCLE $BLDCYCLE \
             $_RELEASE $release
DeleteStatus $_TYPE $T_BLDPTF \
             $_SUBTYPE $S_BLDPTFDEPEND \
             $_BLDCYCLE $BLDCYCLE \
             $_RELEASE $release
DeleteStatus $_TYPE $T_BLDPTF \
             $_SUBTYPE $S_BLDGETXREF \
             $_BLDCYCLE $BLDCYCLE

## define paths for the new lists
MAKELIST=$(bldreleasepath $release)/makelist
ENVLIST=$(bldreleasepath $release)/envlist
UPDATELIST=$(bldreleasepath $release)/updatelist
CHANGELIST=$(bldreleasepath $release)/changelist
DEFECTLIST=$(bldreleasepath $release)/defectlist
REVISELIST=$(bldreleasepath $release)/reviselist

## generate the new lists
bldmakelist $release $MAKELIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting make list"; }
bldenvlist $release > $ENVLIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting build environment list"; }
bldupdatelist $release > $UPDATELIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting update list"; }
bldchangelist $release > $CHANGELIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting change list"; }
bldreviselist $release > $REVISELIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting revise list"; }
blddefectlist $release > $DEFECTLIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting defect list"; }

## The status is successful only if all data was successfully gathered.
if [[ $rc = $FAILURE ]]
then
	bldsetstatus $_TYPE $T_BLDPTF \
        	     $_SUBTYPE $S_BLDGETLISTS \
	             $_BLDCYCLE $BLDCYCLE \
	             $_RELEASE $release \
		     $_STATUS $ST_FAILURE || log -x "setting Build List Status"
else
	bldsetstatus $_TYPE $T_BLDPTF \
	             $_SUBTYPE $S_BLDGETLISTS \
	             $_BLDCYCLE $BLDCYCLE \
	             $_RELEASE $release \
		     $_STATUS $ST_SUCCESS || log -x "setting Build List Status"
fi

[[ $rc = $UNSET ]] && rc=$SUCCESS
exit  ## trap returns $rc

