#! /bin/ksh
# @(#)11	1.6  src/bldenv/bldtools/rename/bldCleanup.sh, bldprocess, bos412, GOLDA411a 9/24/92 15:59:05
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldCleanup
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
# NAME: bldCleanup
#
# FUNCTION:  Queries for all closed bldcycles, It then calls up 
#	     'bldcycleCleanup' for all but the last five closed cycles.
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

##############################################################################
function clean_up {
        rm -f $CLOSEDCYCLES
        logset -r
}

##############################################################################
. bldloginit

trap 'clean_up;exit 2' INT QUIT HUP TERM

. bldinitfunc
. bldkshconst
bldinit
logset -c "$(basename $0)"
CLOSEDCYCLES="$BLDTMP/closedcycles"

# Query for all closed bldcycles and write all of them except the last five
# to a file
log -b "Querying for closed bldcycles"
QueryStatus $_TYPE $T_BLDCYCLE $_PHASE "close" \
		$_RELEASE "$_DONTCARE" $_LEVEL "$_DONTCARE" \
		$_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
		$_LPP "$_DONTCARE" $_PTF "$_DONTCARE" \
		$_STATUS "$_DONTCARE" -A | \
		awk -F"|" ' { \
			print $2 \
		} ' | sort -r | tail +5 > $CLOSEDCYCLES
[[ $? = 0 ]] || log -x "Querying STATUS file for closed bldcycles"


# For each closed cycle in the file remove the globalpath and logpath, Delete
# all the entries in the STATUS file for that cycle -- This is done by calling 
# 'bldcycleCleanup'

while read cycle
do
	log -b "calling bldcycleRemove for $cycle"
	bldcycleRemove -s $cycle || log -x "bldcycleRemove $cycle"
done < $CLOSEDCYCLES
clean_up
exit 0
