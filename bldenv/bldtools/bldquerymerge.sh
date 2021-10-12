#! /bin/ksh
# @(#)93	1.13  src/bldenv/bldtools/bldquerymerge.sh, bldtools, bos412, GOLDA411a 1/14/94 13:39:51
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldquerymerge
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bldquerymerge
#
# PURPOSE: Queries the status file and generates various formatted reports 
#	   from its contents.  Also allows user to modify/delete entries in
#	   the status file.  Operates from a menu mode or by passing command
#	   line options.
# WARNING: THIS TOOL IS ONLY INTENDED TO BE RUN ON BAI PRODUCTION BUILD 
#          MACHINES.  IT SHOULD NOT BE USED BY AREA BUILDERS.
#
# INPUT: optional command line arguments, BLDCYCLE and STATUS_FILE environment
#	 variables may optionally be initialized before the call.
#
# OUTPUT: Standard output
#
# RETURNS: 0 if successful, 1 if errors
#
# EXECUTION ENVIRONMENT: Build process environment
#

. bldloginit
. bldinitfunc	# define build init functions
. bldkshconst	# define constants
. bldhostsfile
. bldnodenames

function clean_up {
	if [ "$1" = "0" ] 
	then
		bldsetstatus $_TYPE "$T_BLDQUERYMERGE" \
			     $_BLDCYCLE "$BLDCYCLE" \
			     $_BUILDER "$BUILDER" \
			     $_STATUS "$ST_SUCCESS" 
	else
		bldsetstatus $_TYPE "$T_BLDQUERYMERGE" \
			     $_BLDCYCLE "$BLDCYCLE" \
			     $_BUILDER "$BUILDER" \
			     $_STATUS "$ST_FAILURE" 
	fi

	exit $1
}

############################### M A I N ##############################

# call clean_up on interrupts
trap "clean_up 1" HUP INT QUIT TERM

bldinit
chksetbuilder

command=`basename "$0"`			# get command name
LOG=${LOGBLDSTATUS:-"$(bldlogpath)/bldstatus.all"}

LOGOPTS="-c${command} +l -F${LOG}"	# set up log environment

[[ ! -f ${RELEASE_LIST} ]] && \
   log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

# checking to see if bldquerymerge is already running; if so then exit now
running=$(ps -ef | grep bldquerymerge | wc | awk -F" *" '{print $2}')
if [ "$running" -gt 2 ]
then
	log ${LOGOPTS} -x "bldquerymerge already running"
fi

delay_flg=""
usage_err=""

while getopts ":d" option
do
      case $option in 
	  d) delay_flg=1;;
          ?) usage_err=1;;
      esac
done

if [ -n "$usage_err" ]
then
	log -x "Usage: ${command} [-d]";
fi

# delaying execution until approximately midnight
if [ -n "$delay_flg" ]
then
	today=$(date | awk -F" " '{ print $3 }')
	while [ "$(date | awk -F" " '{ print $3 }')" = "$today" ]
	do
		sleep 60
	done
fi

# path to local copy of bldquery database
BQDBLOCAL=$(bldhistorypath)/bldquery 
if [ ! -d "$BQDBLOCAL" ]
then
	mkdir ${BQDBLOCAL} || \
		log ${LOGOPTS} -x "could not create directory ${BQDBLOCAL}"
fi

# path to latest raw data to be used in building bldquery data base
BQDBNEW=$(bldglobalpath)

# path to AFS directory where bldquery database files will be stored
bldhostsfile get_afs_base "${TRUE}"
[[ $? -ne 0 ]] && clean_up 1
BQDBGLOBAL=${BQDBGLOBAL:-${HOSTSFILE_AFSBASE}/HISTORY/bldquery}
			
# cleaning up local bldquery database directory
rm -f ${BQDBLOCAL}/* > /dev/null 2>&1
rm -f ${BQDBLOCAL}/LPP/* > /dev/null 2>&1
rm -f ${BQDBLOCAL}/*/SRC > /dev/null 2>&1
rm -f ${BQDBLOCAL}/*/NSRC > /dev/null 2>&1
rm -f ${BQDBLOCAL}/*/SHIP > /dev/null 2>&1
rm -f ${BQDBLOCAL}/*/NSHIP > /dev/null 2>&1
rm -f ${BQDBLOCAL}/*/BLDENV > /dev/null 2>&1

# merging new xreflist and release makelist files into the local bldquery
# database directory

log ${LOGOPTS} -b "merging latest bldquery data with cumulative data"

cp ${BQDBNEW}/xreflist ${BQDBLOCAL}
while read release
do
	if [[ -s "${BQDBNEW}/${release}/makelist"  && \
	      -f "${BQDBNEW}/${release}/DOBLDQUERY" ]]
	then
		if [ ! -d "${BQDBLOCAL}/${release}" ]
		then
			mkdir ${BQDBLOCAL}/${release}
		fi
		cp ${BQDBNEW}/${release}/makelist ${BQDBLOCAL}/${release}
	fi
done < ${RELEASE_LIST}

log ${LOGOPTS} -b "creating bldquery database files"

bldquerydb
if [ "$?" -ne 0 ] 
then
	log ${LOGOPTS} -x "unable to create bldquery database files."
fi

# update AFS w/ the newly created bldquery database files
log ${LOGOPTS} -b "copying bldquery database files to AFS"

if [[ -n "$KLOG_USERID" && -n "$KLOG_PASSWD" ]]
then
	klog $KLOG_USERID -password $KLOG_PASSWD > /dev/null 2>&1
fi

# copy bldquery database files out to AFS
if [ -w "${BQDBGLOBAL}" ]
then
    rm -rf ${BQDBGLOBAL}/*
    cp -r ${BQDBLOCAL}/* ${BQDBGLOBAL}
    if [ "$?" -ne 0 ] 
    then
	log ${LOGOPTS} -x \
	  "unable to copy bldquery database files to ${BQDBGLOBAL}.\n"
    fi
else
    log ${LOGOPTS} -x "directory ${BQDBGLOBAL} is not writeable.\n"
fi

clean_up 0

