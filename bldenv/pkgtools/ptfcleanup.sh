#!/bin/ksh
#
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: Usage
#
# FUNCTION: Displays a usage message
#
# INPUT:
#
# OUTPUT: Usage message to stderr
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#

function Usage
{
	print -u2 "Usage:  ${command} -p <ptfpkgFile>"
	return 0
} 

#
# NAME: stripComments
#
# FUNCTION: Removes comments and blank lines from input file
#
# INPUT: a file
#
# OUTPUT: File without comments and blank lines to stdout.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#

function stripComments
{
    sed -e 's/^[#].*$//' -e '/^[       ]*$/d' < $1
    return 0
}

#
# NAME: Clean_Up
#
# FUNCTION: Clean up after running.
#
# INPUT: DEFECTAPARS (global) -
#        TMPFILE (global) -
#
# OUTPUT: None.
#
# SIDE EFFECTS: Returns to $currentDir, where ptfCleanup was invoked.
#
# RETURNS: 0 always.
#

function Clean_Up
{
	rm ${VRMFFILE} > /dev/null 2>&1
	rm ${PTFPKGFILE} > /dev/null 2>&1
	cd ${currentDir}
	return 0
}

#
# NAME: resetVRMF
#
# FUNCTION: Reset the vrmf in the vrmfFile in the current directory.
#		The vrmf should be reset to ${version}.${release}.0.0
#		where ${version} and ${release} are from the current
#		vrmfFile.
#
# INPUT: DEFECTAPARS (global) -
#        TMPFILE (global) -
#
# OUTPUT: None.
#
# SIDE EFFECTS: vrmfFile in current directory has mod and fix levels of 0.
#
# PRECONDITIONS: The current directory is the fileset directory and contains
#	the vrmfFile.
#
# RETURNS: 0 always.
#

function resetVRMF
{
	VRMFFILE=vrmfFile.$$
	stripComments vrmfFile > ${VRMFFILE}

	cat ${VRMFFILE} | while read vrmf
	do
		version=`echo $vrmf | cut -d. -f1`
		release=`echo $vrmf | cut -d. -f2`
		newvrmf="${version}.${release}.0.0"
	done

	echo "${newvrmf}" > vrmfFile
	rm ${VRMFFILE}

	return 0
}

trap 'Clean_Up; trap - EXIT; exit 4' EXIT INT QUIT HUP TERM

typeset    command=${0##*/}
typeset	   currentDir=${PWD}

while getopts :p: option
do
	case ${option} in
	    p)
		ptfpkgfile="${OPTARG}"
		;;
	    :)
		print -u2 "$OPTARG requires a value"; Usage; exit 1;;
	    \?)
		print -u2 "unknown option $OPTARG"; Usage; exit 1;;
	esac
done
shift OPTIND-1

if [[ ! -r ${ptfpkgfile} ]]
then
	print -u2 "${command}:  Cannot read package file ${ptfpkgfile}."
	print -u2 "\tTerminating.\n"
	exit 1
fi

PTFPKGFILE=${ptfpkgfile}.$$
stripComments ${ptfpkgfile} > ${PTFPKGFILE}

if [[ -z "${TOP}" ]]
then
	print -u2 "${command}:  TOP environment variable not set."
	print -u2 "\n\tTOP should be set to the selfix directory location"
	print -u2 "\tin the update tree."
	exit 1
fi

previousFileset=""
cat ${PTFPKGFILE} | while read line
do
	fileset=`echo $line | cut -d\| -f4`
	# Only need to reset once for each fileset
	[[ "${previousFileset}" = "${fileset}" ]] && continue

	previousFileset="${fileset}"
	cd ${TOP}/UPDATE
	filesetDir=`echo $fileset | sed 's:\.:/:g'`
	cd ${filesetDir}

	# If a vrmfFile file does not exist we are not in a fileset directory.
	if [[ ! -f vrmfFile ]]
	then
		print -u2 "${command}:  No vrmfFile for fileset ${fileset}"
		print -u2 "\tin directory ${PWD}.  Continuing to next entry"
		print -u "\tfrom ${PTFPKGFILE}.\n"
		continue
	fi

	resetVRMF

	>ptfsList
done

Clean_Up

exit 0
