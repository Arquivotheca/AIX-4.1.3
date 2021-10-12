#! /bin/ksh
# @(#)12	1.3  src/bldenv/bldtools/bldcycleRemove.sh, bldprocess, bos412, GOLDA411a 9/24/92 16:01:19
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldcycleRemove
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
# NAME: bldcycleRemove
#
# FUNCTION:  Cleans up all the LOG and PTF directories for the BLDCYCLE that is
#	     passed as a input parameter. It also removes all the entries in 
#	     the status file for that cycle and then posts a phase change entry
#	     (deleted) in the status file for that cycle
#
# INPUT: BLDCYCLE
#
# OUTPUT: none
#
# RETURNS: Zero on success and a non zero on failure
#
# SIDE EFFECTS: Modifies the status file.
#
#
. bldloginit
trap 'logset -r;exit 2' INT QUIT HUP TERM

while getopts ":slg" option
do
	case $option in 
		s) status_cleanup=1;;
		l) log_cleanup=1;;
		g) global_cleanup=1;;
		?) usage_err=1;;
	esac
done

shift OPTIND-1

if (( $# != 1 ))
then
	usage_err=1
fi

if [[ -z "${status_cleanup}" && \
      -z "${log_cleanup}"    && \
      -z "${global_cleanup}" ]]
then
	usage_err=1
fi

if [[ -n "${usage_err}" ]]
then
	log -x "USAGE: $(basename $0) [-s] [-l] [-g] BLDCYCLE"
fi

export BLDCYCLE=$1
. bldinitfunc
bldinit
. bldkshconst
logset -C "$(basename $0)"

if [ -n "${status_cleanup}" ]
then
	log -b "Cleaning up all status entries for Build cycle $BLDCYCLE"
	DeleteStatus $_BLDCYCLE $BLDCYCLE -F
	[[ $? = 0 ]] || \
		log -x "Deleting entries for $BLDCYCLE in the STATUS file"
fi

if [ -n "${log_cleanup}" ]
then
	log -b "Removing the logpath for Build cycle $BLDCYCLE"
	rm -rf "$(bldlogpath)"
	[[ $? = 0 ]] || log -x "Cleaning up logpath"
fi

if [ -n "${global_cleanup}" ]
then
	log -b "Removing the globalpath for Build cycle $BLDCYCLE"
	rm -rf "$(bldglobalpath)"
	[[ $? = 0 ]] || log -x "Cleaning up globalpath"
fi

logset -r
exit 0

