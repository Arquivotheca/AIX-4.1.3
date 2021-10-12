#! /bin/ksh
# @(#)09	1.7  src/bldenv/bldtools/rename/bldcommitall.sh, bldprocess, bos412, GOLDA411a 3/17/94 17:14:50
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldcommitall
#	     clean_up
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: bldcommitall
#
# FUNCTION:  For every release which has a v3bld update entry and also the
#	     subptf entry for bldptfdepend and subptfpkg entry is a success, It 
#	     calls bldhistorycommit for the release. It also calls bldhistory
#	     with COMMITPTFIDS as the argument.
#
# INPUT: 
#
# OUTPUT: none
#
# RETURNS: Zero on success and a non zero on failure
#
# SIDE EFFECTS: Modifies the status file.
#
#

###############################################################################

function clean_up {
	rm -f $BLDTMP/relnames
	logset -r
}

###############################################################################

. bldloginit
trap 'clean_up;exit 2' INT QUIT HUP TERM
rc=0
. bldinitfunc
. bldkshconst
bldinit
logset -C "$(basename $0)"
# Query for all releases which have a T_V3BLD status and write to a tmp file

QueryStatus $_TYPE $T_V3BLD $_BLDCYCLE $BLDCYCLE $_LEVEL "$_DONTCARE" \
		$_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
		$_LPP "$_DONTCARE" $_PTF "$_DONTCARE" $_SUBTYPE "$_DONTCARE" \
		$_UPDATE $U_UPDATE -A | \
		awk -F"|" ' { \
			print $3 \
		} ' > $BLDTMP/relnames

while read release
do
	# if T_BLDPTF:S__BLDPTFDEPEND && S_BLDPTFPKG is successfull, then call
	# bldhistorycommit if not done before

	if ((CheckStatus $_TYPE $T_BLDPTF $_RELEASE $release \
		$_SUBTYPE $S_BLDPTFDEPEND $_STATUS $ST_SUCCESS) && \
	   (CheckStatus $_TYPE $T_BLDPTF \
		$_SUBTYPE $S_BLDPTFPKG $_STATUS $ST_SUCCESS)) 
	then
		if (CheckStatus $_TYPE $T_POSTBUILD  $_BLDCYCLE $BLDCYCLE \
			$_SUBTYPE $S_BLDHISTORYCOMMIT $_RELEASE $release \
			$_STATUS $ST_SUCCESS)
		then
			log -b "Release $release has been commited already"
		else
			bldhistorycommit $release
			[[ $? = 0 ]] || {
					     log -e "bldhistorycommit $release"
					     rc=1
					}
		fi
	else
		log -e "Release $release is not ready to commit"
		rc=1
	fi
done < $BLDTMP/relnames

bldhistory COMMITPTFIDS
[[ $? = 0 ]] || log -x "bldhistory COMMITPTFIDS"
clean_up
exit $rc
