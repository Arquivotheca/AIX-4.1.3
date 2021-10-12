#! /bin/ksh
# @(#)49	1.21  src/bldenv/pkgtools/subptf/bldmid.sh, pkgtools, bos412, GOLDA411a 6/24/93 15:06:24
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldmid
#	     
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: bldmid
#
# FUNCTION:  Appends defect entries to the $(bldhistorypath)/abstracts file;
#	     creates $(bldglobalpath)/defectapars file
#
# INPUT: $TOP/PTF/$BLDCYCLE/all_defects
#
# OUTPUT: none
#
# RETURNS: 0
#

############################################################################
# function:	add_to_defectapars
# description:	queries the CMVC database for defect #s and APARS and 
# 		appends them to the defectapars file
# input:	a comma-separated list of defects in ${defectlist}
# output:	lines appended to defectapars file in the form:
#
#			defect|APAR|release|family
#
#		e.g.:
#
#			44567|ix33990|bos324|aix@ausaix02@2035
#
# remarks:
############################################################################
add_to_defectapars()
{
	Report -view defectv -raw -family $CMVCfam -where \
		"name in ( ${defectlist%,} )" > ${SCRATCH}
	if [ "$?" -ne 0 ]
	then
		log -x "error reading defect data from family $CMVCfam."
	fi
	Report -view featureview -family $CMVCfam -raw -where \
		"name in ( ${defectlist%,} )" >> ${SCRATCH}
	if [ "$?" -ne 0 ]
	then
		log -x "error reading feature data from family $CMVCfam."
	fi
	awk -F"|" 'BEGIN {
		family=ARGV[1] ; release=ARGV[2] ; ARGV[1] = "" ; ARGV[2] = ""
	}
 	{
		if ($26 == "") {
			print $2"|"$2"|"release"|"family
			next
		}
		prefix=substr($26,1,2) 
		suffix=substr($26,3) 
		if (prefix !~ /[Ii][Xx]/) {
			print $2"|"$2"|"release"|"family
			next
		}
		prefix="IX"
		if (length(suffix) == 0) {
			print $2"|"$2"|"release"|"family
			next
		}
		aparid=prefix suffix
		if (suffix !~ /[^0-9]/) { 
			print $2"|"aparid"|"release"|"family
		} else {
			print $2"|"$2"|"release"|"family
		}
	}' $CMVCfam $oldRelease ${SCRATCH} >> ${DEFAPARS_FILE}

	[[ $ERRNO -ne 0 ]] &&
     	    log -x "Write error on ${DEFAPARS_FILE} ($ERRNO)"
}


########################### MAIN ############################

. bldloginit
. bldkshconst
. bldinitfunc

rc=$UNSET

trap 'rm -f ${SCRATCH} ; logset -r; exit $rc' HUP INT QUIT TERM

bldinit

command=`basename "$0"`		# get command name
logset -c"${command}" +l	# set up log environment

SCRATCH=/tmp/scratch.$$
DEFAPARS_FILE=$(bldglobalpath)/defectapars
ALLDEFECTS_FILE=$(bldglobalpath)/all_defects
PTFIDS_FILE=$(bldglobalpath)/ptfids
PTFIDSORG_FILE=$(bldglobalpath)/ptfids.org

# make sure ptfids and ptfids.org files exist
touch ${PTFIDS_FILE} ${PTFIDSORG_FILE}

typeset -i count=0
typeset -i MAX_DEFS=100

#
# reset and create the defect APARs file
#
log -b "Creating/updating defectapars and abstracts files for build cycle ${BLDCYCLE}."
cat /dev/null > ${DEFAPARS_FILE}

[[ ! -r ${ALLDEFECTS_FILE} ]] && log -x "Cannot read file ${ALLDEFECTS_FILE}"

oldRelease=

cat ${ALLDEFECTS_FILE} |
while read defect release family
do
    if [[ -z "$oldRelease" ]]
    then
           oldRelease=$release
           CMVCfam=$family
    fi

    if [ "$oldRelease" = "$release" ]
    then
            defectlist="${defect},${defectlist}"
            count=count+1
            if [ count -eq MAX_DEFS ]
            then
                    add_to_defectapars
                    count=0
                    defectlist=""
            fi
    else
            if [ -n "${defectlist}" ]
            then
                    add_to_defectapars
                    count=1
                    defectlist="${defect}"
                    CMVCfam=$family
                    oldRelease=$release
            fi
    fi
done
if [ -n "${defectlist}" ]
then
	add_to_defectapars
fi

#
# updating abstracts file with defects from current build
#
bldabstracts
[[ "$?" -ne 0 ]] && log -x "error running bldabstracts."

rm -f ${SCRATCH}

logset -r

[[ $rc -eq $UNSET ]] && rc=$SUCCESS
exit $rc

