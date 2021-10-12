#!/bin/ksh
#
# @(#)43        1.5  changevrmf.sh, pkgtools, bos412 9/24/94 10:35:42
#
# COMPONENT_NAME: PKGTOOLS
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
# NAME: changevrmf
#
# DESCRIPTION: This tool modifies the VRMF levels in the lpp_name file
#              based upon the level in the vrmfFile in the selfix tree.
#              What ever is in the selfix tree is written into the lpp_name
#              file.  
#              This approach REQUIRES that updates be built BEFORE
#              the corresponding install image be built.  The updates
#              must be built all the way thru ptfpkg so that the vrmfFile
#              in the selfix tree is correct.
#              IF the vrmfFile cannot be accessed, then the filesets
#              vrmf will NOT be modified.
#
# PRECONDITIONS: none.
#
# POST CONDITIONS: 
#              lpp_name file is most probably modified.
#
# INPUT PARAMETERS:
#                - LPPNAME file (parameter) the list of all filesets 
#                  for the option one per line
#                - lpp_name file (parameter) the lpp_name file to be changed
#                   
# RETURNS: 
#          0 = success
#          1 = usage
#          3 = interrupted
#**************************************************************************


#------------------------------------
# Define full path to needed 
# utilities to work around
# the limitted PATH env var
# given to us from make.
#------------------------------------
awk=${ODE_TOOLS}/usr/bin/awk
cat=${ODE_TOOLS}/usr/bin/cat
cut=${ODE_TOOLS}/usr/bin/cut
echo=${ODE_TOOLS}/usr/bin/echo
fgrep=${ODE_TOOLS}/usr/bin/fgrep
getopt=${ODE_TOOLS}/usr/bin/getopt
grep=${ODE_TOOLS}/usr/bin/grep
mkdir=${ODE_TOOLS}/usr/bin/mkdir
mv=${ODE_TOOLS}/usr/bin/mv
rm=${ODE_TOOLS}/usr/bin/rm
sed=${ODE_TOOLS}/usr/bin/sed
sort=${ODE_TOOLS}/usr/bin/sort
touch=${ODE_TOOLS}/usr/bin/touch

#----------------------------------------------------------------------
# Setup some global variables needed elsewhere
#----------------------------------------------------------------------
myName=${0##*/}
myFill=`$echo $myName | $sed -e 's/./ /g'`

#------------------------------------
# Temporary files
#------------------------------------
tmpfile1=/tmp/temp1.$$
tmpfile2=/tmp/temp2.$$
tmpfile3=/tmp/temp3.$$
tmpfile4=/tmp/temp4.$$
tmpfile5=/tmp/temp5.$$
tmpfile6=/tmp/temp6.$$
#=========================================================================
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
#=========================================================================
function cleanup
{
	$echo "Cleanup in progress ... "
	$rm -f $tmpfile1 $tmpfile2 $tmpfile3 $tmpfile4 
	$rm -f $tmpfile5 $tmpfile6
	$echo "done."
}


#=========================================================================
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
#=========================================================================
function usage
{
	print -u2 - "ERROR: $myName: Usage: -f<option filesets file> -l <lpp_name dir>"
}


#====================================
#     B E G I N    M A I N
#====================================

#-------------------------------------------------------
# Cleanup on interrupts.
#-------------------------------------------------------
trap "cleanup; exit 3" INT QUIT HUP TERM

#-------------------------------------------------------
# get command line parameters
#-------------------------------------------------------
set -- `$getopt "f:l:" $*`
while [ "$1" != "--" ]
do
    case $1 in
	"-f")
	    fileset_file=$2
	    shift 2
	    ;;
	"-l")
	    lppnamefile=$2
	    shift 2
	    ;;
	 *)
	    usage
	    exit 1
    esac
done

#-------------------------------------------------------
# Ensure required parameters are specified.
#-------------------------------------------------------
if [[ -z $fileset_file ]]
then
    print -u2 - "ERROR: $myName - '-f' flag is required..."
    usage
    exit 1
fi

if [[ -z $lppnamefile ]]
then
    print -u2 - "ERROR: $myName - '-l' flag is required..."
    usage
    exit 1
fi

#-------------------------------------------------------
# Ensure required files are accessable
# exit OK if not (don't want to kill the build,
# only means we can't change the VRMF).
#-------------------------------------------------------
if [[ ! -s ${fileset_file} ]]
then
    print -u2 - "WARNING: ${myName}: Cannot access \"${fileset_file}\". "
    print -u2 - "WARNING: ${myFill}  VRMF will not be modified; build continuing."
    exit 0
fi

if [[ ! -s ${lppnamefile} ]]
then
    print -u2 - "WARNING: ${myName}: Cannot find \"${lppnamefile}\" file. "
    print -u2 - "WARNING: ${myFill}  VRMF will not be modified; build continuing."
    exit 0
fi

#------------------------------------------------------
# Build temp file containing fileset and vrmf
# format is 
# file.set.name v.r.m.f
# The vrmf is obtained from the vrmfFile in
# the selfix tree.  If that file is NOT 
# accessable, no entry is made and the VRMF
# for that fileset will not be modified.
#------------------------------------------------------
$rm -f $tmpfile1                   # Start with an empty temp file.

typeset -i fsCnt=0 ;
exec 4> $tmpfile1
$cat ${fileset_file} | while read fileset junk
do
   if [[ -z $fileset ]]            # skip blank lines.
   then
       continue
   fi

   filesetdir=`$echo ${fileset} | $sed -e 's/\./\//g'`
   VRMF_FILE=${BASE}/selfix/UPDATE/${filesetdir}/vrmfFile

   if [[ -r ${VRMF_FILE} ]]
   then
       newVRMF=$(cat ${VRMF_FILE}) ;
       if [[ -n $newVRMF ]]
       then
	   print -u4 - "${fileset} ${newVRMF}"
	   fsCnt=${fsCnt}+1
       else
	   print -u2 - "WARNING: ${myName} - Did not get VRMF from '$VRMF_FILE'"
       fi
   else
       print -u2 - "WARNING: ${myName} - Did not get to VRMF file '$VRMF_FILE'"
   fi
done

#---------------------------------------------------
# If we could not get to any of the vrmfFiles,
# then we can exit out because there is nothing
# to change to.
#---------------------------------------------------
if ((fsCnt < 1))
then
    exec 4<&-                      # Close the tmp file
    cleanup
    exit 0
else
    print -u4 - "END"
    exec 4<&-                      # Close the tmp file
fi

#---------------------------------------------------
# If we are still here, then we have some stuff
# that needs to overwritten.
#
# This is done via awk.
# 1st the tmpfile is read to load an associative
#     array that is indexed by fileset name.
# 2nd the lpp_name file is processed (in 1 pass)
#     and all fileset definition lines that
#     have matches in the associative array
#     are modified.
#     A fileset definition is identified by
#     a 1 of 2 cases
#      1) 2nd line of lpp_name file
#      2) line following ']' in 1st column of
#         previous line
#---------------------------------------------------
$awk '
BEGIN {
    fsDefinition = 0 ;
    getline ;
#    print "@@In BEGIN block; line = $0"
    while (NF > 1)
    {
	newVRMF[$1] = $2 ;
#	print "@@In BEGIN; saved \"$2\" vrmf for \"$1\"" 
	getline;
    }
#    print "@@exiting BEGIN block"
    NR = 0 ;
}
#---------------------------------------------
# Should only have to worry about the
# lpp_name file now.
#---------------------------------------------
NR == 1 || $1 == "]" {
#    print "@@MATCHED PATTERN 1"        # 
    print ;
    fsDefinition = 1 ;
#    print "@@in main, expect FS next, line = \"$0\""
    next ;
}

fsDefinition == 1 {
#    print "@@MATCHED pattern 2"     # DEBUG NEEDS REMOVAL
    if (NF >= 7)
    {
	fileset = $1 ;
	if (newVRMF[fileset] != "" )
	{
	    #---------------------------------------------
	    # Got a new VRMF for this fileset, so
	    # spit it out in the correct format.
	    # Then dump out the rest of the fields
	    # from the input line.
	    #---------------------------------------------
	    i = split(newVRMF[fileset], vrmf, "\.") ;
	    printf("%s %02d.%02d.%04d.%04d ", fileset, 
	           vrmf[1], vrmf[2], vrmf[3], vrmf[4]) ;

	    for(i=3; i<=NF; i++) printf("%s ",$i);
	    printf("\n");
	}
	else
	{
	    print ;
	}
    }
    else
    {
	#-------------------------------------------------
	# This is not a valid fileset definition line;
	# reset the expectation variable.
	#-------------------------------------------------
	fsDefinition = 0 ;
	print ;
    }
    next ;
}
#----------------------------------------------------------
# Need to explicitly print everything not matched
# above due to 'next' statment at the bottom of
# those patterns.
#----------------------------------------------------------
{print
#print "@@", FILENAME, NR, NF;      # DEBUG NEEDS REMOVAL
}' tmpFile=${tmpfile1} ${tmpfile1} ${lppnamefile} >${tmpfile2}

# move the tmp2file back to lpp_name file
$mv  ${tmpfile2} ${lppnamefile}
#cleanup
exit 0
