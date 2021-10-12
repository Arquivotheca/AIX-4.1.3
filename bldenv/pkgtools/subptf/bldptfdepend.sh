#! /bin/ksh
# @(#)64	1.19  src/bldenv/pkgtools/subptf/bldptfdepend.sh, pkgtools, bos412, GOLDA411a 3/16/94 16:38:30
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	Cleanup
#		bldptfdepend 
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
# NAME: Cleanup
#
# FUNCTION: Cleanup bldptfdepend environment (for all functions)
#
# INPUT: all environment variables containing temporary file names
#
# OUTPUT: none
#
# SIDE EFFECTS: all existing bldptfdepend temporary files are deleted
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: n/a
#
function Cleanup
{
	[[ -n "$DEBUG" ]] && log +l -b -cbldptfdepend "removing temporary files"
        rm -f $UPDATE_DEPENDENCIES $BLDENV_DEPENDENCIES \
	      $SRCDEFECTS $PREVIOUS_DEFECTS
}

#
# NAME: bldptfdepend
#
# FUNCTION: Determine dependency relationships between defects and save the
#	    dependency data in the bld databases. 
#
#	In general, dependency relationships are found by examining what
#	files were built from the defects and what make dependencies were
#	generated while building these files.
#
# SYNTAX: bldptfdepend [-t] <release> <levelname>...
#
# INPUT: release (parameter) - CMVC level release name
#	 DEBUG (environment) - flag to generate debug data, when true
#
# OUTPUT: none
#
# SIDE EFFECTS: the bldtd and bldinfo databases are created and the
#	        bldhistory database is updated (see the associated
#		database modules for access routines and data formats)
#
# EXECUTION ENVIRONMENT: the build process environment
#
#	v3bld must be executed first (multiple cranks are ok).  
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (premature exit)
#
######################################################################
############################# S E T U P ##############################
rc=$UNSET; logset -c$0 +l  # init log command
logarg="-e"
[[ -n "$DEBUG" ]] && log -b "entering ($@)"

trap 'Cleanup; logset -r; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

##### Parse command line #####
while getopts ":" option
do
        case $option in
       		\?) log -e "unknown option $OPTARG"
           	print -u2 "USAGE: $0 <release> <CMVC level name>..."
           	exit;;
        esac
done
shift OPTIND-1  # skip past option to get the parameters
[[ $# = 1 ]] || { log -e "not enough parameters" 
		  print -u2 "USAGE: $0 <release>"
		  exit; }
release=$1

##### Define file paths #####
MAKELIST=$(bldreleasepath $release)/makelist
UPDATELIST=$(bldreleasepath $release)/updatelist
ENVLIST=$(bldreleasepath $release)/envlist
NEWENVLIST=$(bldreleasepath $release)/newenvlist
CHANGELIST=$(bldreleasepath $release)/changelist
DEFECTLIST=$(bldreleasepath $release)/defectlist
UPDATE_DEPENDENCIES=$(bldtmppath)/bldptfdepends.update_dependencies$$
PREVIOUS_DEFECTS=$(bldtmppath)/bldptfdepends.previous_defects$$
BLDENV_DEPENDENCIES=$(bldtmppath)/bldptfdepends.bldenv_dependencies$$
SRCDEFECTS=$(bldtmppath)/bldptfdepends.srcdefects$$

##### Confirm input files are available. #####
[[ -r $MAKELIST ]] || log -x "unable to read make list" 		
[[ -r $ENVLIST ]] || log -x "unable to read build environment list"
[[ -r $UPDATELIST ]] || log -x "unable to read update list" 	
[[ -r $DEFECTLIST ]] || log -x "unable to read defect list"
[[ -r $DEFECTLIST ]] || log -x "unable to read CMVC name changes"

######################################################################
############################## B O D Y ###############################
## Get any CMVC file name changes and update the history tree.  This 
## keeps the history in sync with the current files.
bldhistory REVISE || log -x "revising history"

## Build the target-dependency database (bldtd) using all the gathered data.
log -b 'building target-dependency database (bldtd)'
bldtd BUILD $release $MAKELIST $DEFECTLIST $UPDATELIST $ENVLIST $CHANGELIST \
	> $NEWENVLIST || 
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg " building target-dependency database (bldtd found errors)"; }

## filter out all prexisting previous dependencies to reduce unneccessary reqs
bldfilterdeps $release  ||
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "filtering previous dependencies"; }

## Gather the defect numbers which caused the update files to be built in
## the current build cycle.  Save these defects in the bldinfo database.
log -b "getting update-file dependencies"
bldcurdef $release $UPDATELIST > $UPDATE_DEPENDENCIES || 	
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting update-file dependencies (bldcurdef found errors)"; }
bldinfo SAVEUP $UPDATE_DEPENDENCIES || 	
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "updating bldinfo database with update dependencies"; }
[[ -n "$DEBUG" ]] && cat $UPDATE_DEPENDENCIES | log +f "Update Defects:"

## Gather for the update files the previous build dependencies (dependencies
## from previous build cycles).  Save these defects in the bldinfo database.
log -b "getting previous dependencies for update files"
bldprevdef $release $UPDATELIST > $PREVIOUS_DEFECTS || 
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting previous dependencies (bldprevdef found errors)"; }
bldinfo SAVEPREV $PREVIOUS_DEFECTS || 
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "updating bldinfo database with previous dependencies"; }
[[ -n "$DEBUG" ]] && cat $PREVIOUS_DEFECTS | log +f "Previous Defects:"

## Gather the defect numbers which caused the bldenv files to be built in
## the current build cycle.  Save these defects in the bldinfo database.
log -b "getting build environment dependencies"
bldcurdef $release $NEWENVLIST > $BLDENV_DEPENDENCIES ||
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting build environment dependencies (bldcurdef found errors)"; }
bldinfo SAVEENV $BLDENV_DEPENDENCIES ||
	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "updating bldinfo with build environment dependencies"; }
[[ -n "$DEBUG" ]] && cat $BLDENV_DEPENDENCIES | log +f "bldenv dependencies:"

[[ -n "$DEBUG" ]] && log -b "exiting"

[[ $rc = $UNSET ]] && rc=$SUCCESS
exit  ## trap returns $rc
