#! /bin/ksh
# @(#)46	1.9  src/bldenv/bldtools/rename/bldhistorycommit.sh, bldtools, bos412, GOLDA411a 5/26/92 12:31:50
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:	bldhistorycommit
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
# NAME: bldhistorycommit
#
# FUNCTION: 
#
# INPUT: release (parameter) - the CMVC level release
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#

. bldloginit
. bldkshconst   # define constants

function Savealldepends {
	typeset release=$1
        typeset filename filelist=$(bldtmppath)/filelist$$ 

	if [[ ! -d $(bldhistorypath)/bldtd ]] ; then
		mkdir $(bldhistorypath)/bldtd || 
			log -x "cannot create bldtd in bldhistorypath ($!)"
	fi
	li -1 $(bldreleasepath $release)/bldtd/alldepend* > $filelist
	while read filename
	do
	       cat $filename >> $(bldhistorypath)/bldtd/$(basename $filename) ||
			log -x "cannot cat $filename ($!)"
	done < $filelist
        rm $filelist
}

LOG=${LOGBLDHISTORYCOMMIT:-$(bldlogpath)/bldhistorycommit.all}
logset -L1 -c$0 -C"bldhistorycommit" -F$LOG  # init log command

[[ $# = 1 ]] || log -x "illegal syntax"
release=$1

log -f "starting bldhistorycommit for $release"

if CheckStatus $_TYPE $T_POSTPACKAGE \
	       $_SUBTYPE $S_BLDHISTORYCOMMIT \
	       $_BLDCYCLE $BLDCYCLE \
	       $_RELEASE $release \
	       $_STATUS $ST_SUCCESS
then
	log -b "$release history already committed"
else
	SRCDEFECTS=$(bldtmppath)/srcdefects
	bldtd GETSRCDEFECTS $release > $SRCDEFECTS ||
		log -x "getting src defects for $release"
	DeleteStatus $_TYPE $T_POSTPACKAGE \
	       	     $_SUBTYPE $S_BLDHISTORYCOMMIT \
	       	     $_BLDCYCLE $BLDCYCLE \
	       	     $_RELEASE $release
	if bldhistory SAVEDEP $SRCDEFECTS
	then
		Savealldepends $release  ## save the files in the history dir
		log -b "$release history committed"
		bldsetstatus $_TYPE $T_POSTPACKAGE \
		       	     $_SUBTYPE $S_BLDHISTORYCOMMIT \
		       	     $_BLDCYCLE $BLDCYCLE \
		       	     $_RELEASE $release \
		       	     $_STATUS $ST_SUCCESS || 
					log -x "setting $release history status"
	else
		log -e "saving $release src history"
		bldsetstatus $_TYPE $T_POSTPACKAGE \
		       	     $_SUBTYPE $S_BLDHISTORYCOMMIT \
		       	     $_BLDCYCLE $BLDCYCLE \
		       	     $_RELEASE $release \
		       	     $_STATUS $ST_FAILURE || 
					log -x "setting $release history status"
	fi
	rm $SRCDEFECTS
fi
log -f "exiting bldhistorycommit for $release"

exit $SUCCESS
