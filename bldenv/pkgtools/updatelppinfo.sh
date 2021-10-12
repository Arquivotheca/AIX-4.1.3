#!/bin/ksh
#
# @(#)84        1.2  src/bldenv/pkgtools/updatelppinfo.sh, pkgtools, bos41J, 9512A_all 3/2/95 16:27:31
#
# COMPONENT_NAME: (PKGTOOLS) BAI Build Tools
#
# FUNCTIONS: usage
#	     cleanup
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: updatelppinfo
#
# DESCRIPTION: This updates all the lpp_info files for all the fileset
#              in the update tree to show their valid base vrmf number.
#              
#              It reads .toc file from /afs/austin/aix/project/aix411/
#              build/latest/inst.images directory and creates a 
#              temporary file containing the fileset and vrmf number
#              one per line. The format of this file is:
#              fileset base vrmfnumber
#             
#              Then it updates the lpp_info file for each fileset in the
#              update tree to show the correct base vrmf number.
#
# PRECONDITIONS: These files should exist
#                - .toc in /afs/austin/aix/project/aix411/build/latest/
#                          inst.images
#
#                - lpp_info files for all filesets in update tree
#
#                - TOP environment variable to the top of the selfix tree 
#
# POST CONDITIONS: changes the base vrmf number in the lpp_info file in
#                  update tree
#
# INPUT PARAMETERS:
#                - .toc file (parameter) the toc file to get the vrmf number
#                   
# SIDE EFFECT: updated lpp_info file in the update tree
#
# RETURNS: always 0
#

# NAME: cleanup
#
# FUNCTION: Clean up function to remove the temporary files
#
# INPUT: None
#
# OUTOUT: None
#
# SIDE EFFECTS: Removes the temporary files
#
# RETURNS: None
#
function cleanup
{
	echo "Cleanup in progress ... "
	rm -f $tmpfile1 $tmpfile2 
	echo "done."
	exit 0
}

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
	echo "$myName: Usage: -i [full path to the toc file] "
	echo "$myFill where -i flag is optional. The default toc file is "
	echo "$myFill /afs/austin/aix/project/aix411/build/latest/inst.images/.toc"
	exit 0
}

#----------------------------------------------------------------------
# Put the name of this program into variable myName for error messages.
# Also create a variable myFill of spaces same length for 2nd line.
#----------------------------------------------------------------------
myName=${0##*/}
myFill=`echo $myName | sed -e 's/./ /g'`

#
# Temporary files
#
tmpfile1=/tmp/temp1.$$
tmpfile2=/tmp/temp2.$$
tocfile=""
UPDATETOP=""
LPP_INFO=""

# get the parameters
while getopts :i:h? option
do
    case $option in
	i)
	    tocFile=$2
	    shift 2
	    ;;
	h)
	    usage
	    ;;
	\?)
	    print -u2 "$myName: Unknown option $OPTARG"
	    usage
	    ;;
    esac
done

trap "cleanup; exit 3" INT QUIT HUP TERM

if [ -z "${TOP}" ]; then
     echo "$myName: TOP environment variable should be set."
     exit 0
fi
UPDATETOP="${TOP}/UPDATE"

if [ -z "$tocFile" ]; then
     if [ ! -s /afs/austin/aix/project/aix411/build/latest/inst.images/.toc ]; then
	echo "$myName: Please specify the full path to the toc file at the "
	echo "$myFill  command Line."
        usage
     else
	tocFile="/afs/austin/aix/project/aix411/build/latest/inst.images/.toc"
     fi
fi

# remove the temp files.
rm -f $tmpfile1 $tmpfile2

# get the names of filesets for an option from LPPNAME file
awk  '{
    if (NF >= 7) {
	printf("%s %s", $1, $2);
	printf("\n");
	}
}' $tocFile > $tmpfile1

# For every fileset update the lpp_info file
cat $tmpfile1 | while read fileset vrmf
do
    # Special case for bos.rte.* filesets (excepting mp and up)
    fs1=${fileset%.*}
    fs2=${fileset##*.}
    if [ ${fs1} = "bos.rte" -a ${fs2} != "mp" -a ${fs2} != "up" ]; then 
	vrmf="4.1.1.0"
    fi

    filesetdir=`echo $fileset | sed -e 's/\./\//g'`
    LPP_INFO="${UPDATETOP}/${filesetdir}/lpp_info"
    if [ ! -f "${LPP_INFO}" ]; then
        print -u2 " File \"${LPP_INFO}\" was not found in the UPDATE tree."
        continue
    else
        awk '{
	     if ($1 == pat) {
	         if ( NF == 2) {
		     printf("%s %s", $1, basevrmf);
		     printf("\n");
		     }
                 }
             else print $0;
             }' pat=$fileset basevrmf=$vrmf $LPP_INFO > $tmpfile2

	 mv -f $tmpfile2 $LPP_INFO
     fi
done
cleanup
exit 0
