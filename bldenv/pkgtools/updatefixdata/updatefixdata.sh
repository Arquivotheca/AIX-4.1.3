#!/bin/ksh
# @(#)69	1.9  src/bldenv/pkgtools/updatefixdata/updatefixdata.sh, pkgtools, bos412, 9445B.bldenv  11/7/94  10:31:35
#
# FUNCTIONS: updatefixdata
#            usage
#
# ORIGINS: 27
#
# updatefixdata --- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


#********************************************************************
# NAME: abort
#                                                                    
# FUNCTION: 
#      Provide central place to do error cleanup and exit.
#                                                                    
# PRE CONDITIONS: none.
#
# POST CONDITIONS: 
#      temporary work files are removed.
#
# PARAMETERS: 
#      Exit code = $1 is the value to use in the exit call
#
# NOTES: 
#      This function exits, it does not return to the caller.
#
# DATA STRUCTURES: none.
#
# RETURNS: 
#      This function exits, it does not return to the caller.
#      It uses $1 as the value to pass to exit.
#********************************************************************
function abort
{
    rm -f ${tmpfile} ${tmpfile2}
    exit $1
} # END abort


#
# NAME: usage
#
# FUNCTION: usage statement.
#
# INPUT: None.
#
# OUTPUT: Prints usage statement.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
usage()
{
    echo "$commandName: Usage: -f<internal vrmf table> -a<abstracts file> -o [ outputfile]"
    echo " where outputfile is optional. The default database will be TOP/HISTORY/fixdataDB"
    abort 1
}

# NAME: updatefixdata
#
# DESCRIPTION: This program reads the internal table for this build 
#              cycle and finds every fileset name that corresponds to every 
#              apar that exists in the internal table.  This program builds 
#              the fixdata stanza for each apar from the set of fileset 
#              names that are found in the internal table as well as the 
#              abstract information in the abstract file for each apar.
#
#              Each line of the internal table has the format:
#              PTF fileset vrmf apar apar ...
#
#              This program will read through the entire internal table 
#              and build a temporary file.
#
#              Then, for each unique apar in the internal table, it will 
#              find the filesets and build the fixdata.  This program then
#              reads the fixdata database and appends stanzas to the 
#              database if they do not already exist.
#
# INPUT PARAMETERS: internal vrmf Table.
#                   abstracts file.
#
# OUTPUT: fixdata database
#
# SIDE EFFECTS: appends new fixdata stanzas to the fixdata database file
#
# RETURNS: None
#

. bldinitfunc
. display_msg

[[ -z "$BLDTMP" ]] && BLDTMP=/tmp
typeset tmpfile=${BLDTMP}/tmpfile$$
typeset tmpfile2=${BLDTMP}/2tmpfile$$
typeset tmpfile3=${BLDTMP}/3tmpfile$$
commandName=$(basename $0)
spaceFill=`echo $commandName | sed -e 's/./ /g'`

#------------------------------------------------------
# display_msg function displays formatted messages
#------------------------------------------------------
log="display_msg -C ${commandName}"

# read the command line parameters
set -- `getopt "f:a:o:" $*`
while [ "$1" != "--" ]
do
    case $1 in
    "-f")
         internal_table=$2
         shift 2
         ;;
    "-a")
	 abstractfile=$2
         shift 2
         ;;
    "-o")
	 database=$2
         shift 2
         ;;
    esac
done

if [ -z "${database}" ]; then
    if [ -z "${TOP}" ]; then
 	   echo "$commandName: ERROR: Environment variable TOP is not set."
	   echo "$spaceFill Processing Stopped."
           abort 1
    fi
    typeset database=$TOP/HISTORY/fixdataDB
fi

#-------------------------------------------
# Check that the database exists.
# Ask user if they want to create it
# if it does not.
#-------------------------------------------
if [ ! -f $database ]
then
    echo "$commandName: The database ($database) does not exist."
    confirm -y "$spaceFill  Do you want to create it [y/n]? \c"
    if [ $? -eq 0 ]
    then
	touch $database
	if [ $? -ne 0 ]
	then
	    echo "$commandName: Unable to create the database ($database)!"
	    abort 2
	fi
    else
	echo "$commandName: The database ($database) must exist to proceed."
	abort 3
    fi
fi

# Check to see if both the internal_table and abstractfile are set.
# If not then give usage and exit.
if [ -z "$internal_table" ] || [ -z "$abstractfile" ]
then
    echo "$commandName: The input parameters internal_table and"
    echo "$spaceFill abstractfile must both be set."
    usage
fi

trap 'abort 4' INT QUIT HUP TERM

# Create a sorted list in $tmpfile with duplicates removed.
# Each line of $tmpfile has the format:
# apar fileset vrmf

cat $internal_table | awk '
NR > 0 { 
	for (i=4;i<=NF;i++)
	{
	    print $i,$2,$3 
	}
}' | sort -u > ${tmpfile}

# if the creation of tmpfile fails then give an error message 
# and exit.
[ $? -ne 0 ] && $log -x "Creation of ${tmpfile} failed."

# if the tmpfile is empty then give an error message 
# and exit.
if [ ! -s ${tmpfile} ]
then
   $log -w "${tmpfile} is empty."
   exit 0
fi

# Create a file $tmpfile2 for each apar in $tmpfile.
# tmpfile2 has the format:
# <@>
# apar
# fileset:vrmf
# fileset:vrmf
# ...
# <@>
# apar
# fileset:vrmf
# fileset:vrmf
# ...

cat ${tmpfile} | awk '
BEGIN { apar = ""  }

$1 == apar {
		print $2":"$3
}
$1 != apar {   print "<@>"
		apar = $1
		save = $1 
		print $1
		print $2":"$3
}
' >${tmpfile2}

# if the creation of tmpfile2 failes then give an error message 
# and exit.
[ $? -ne 0 ] && $log -x "Creation of ${tmpfile2} failed."

# if the tmpfile2 is empty then give an error message 
# and exit.
if [ ! -s ${tmpfile2} ]
then
   $log -w "${tmpfile2} is empty."
   exit 1
fi

# for every apar in $tmpfile2
# go find the abstract/symptom data and build up the fixdata stanza
# Then add the fixdata stanza to the database if it isn't already present.
# always add to the front of the database for improved scan performance.

buildfixdata ${abstractfile} < ${tmpfile2} > ${tmpfile3}

# Check the return code from buildfixdata
[ $? -ne 0 ] && $log -x "buildfixdata failed."

# check to see if tmpfile3 is empty
if [ ! -s ${tmpfile3} ]
then
    $log -w "The output ${tmpfile3} file created by buildfixdata is empty."
    exit 1
fi

addfixdata ${database} < ${tmpfile3} >${tmpfile}
# Check the return code from addfixdata
[ $? -ne 0 ] && $log -x "addfixdata failed."

mv ${database} ${tmpfile2} > /dev/null 2>&1

cat ${tmpfile} ${tmpfile2} >${database}

exit 0
