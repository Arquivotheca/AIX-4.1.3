#!/bin/ksh
# @(#)08        1.16  src/bldenv/pkgtools/subptf/bldabstracts.sh, pkgtools, bos412, GOLDA411a 6/6/93 12:07:59
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldabstracts
#            clean_up
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
# clean_up - cleanup temporary output files, shut down logging
#
function clean_up {
    /bin/rm -f $DEFECTAPARS $ABSTRACTLIST $SYMPTOMS 2>/dev/null >/dev/null
    /bin/rm -f $CHECKSYMP_FAILURE 2>/dev/null >/dev/null
    logset -r
}

#
# -=-=-=- MAIN PROGRAM -=-=-=-
#

# Perform setup, trap interrupts, process arguments .....
. bldloginit
trap 'clean_up;exit 2' INT QUIT HUP TERM
while getopts :l: option
do
    case $option in
        l)  logfile=$OPTARG ;;
        :)  print -u2 "$0: $OPTARG requires a value"
            exit 2;;
        \?) print -u2 "$0: unknown option $OPTARG"
            print -u2 "USAGE: $0 [-l <logfile>]"
            exit 2;
    esac
done

# Perform function/variable setup and setup log directory/logging .....
. bldkshconst
. bldinitfunc
bldinit
LOG=$(bldlogpath)
test ${logfile:="${LOG}/$(basename $0).all"}
temp_file=$(bldtmppath)
logset -C $0 -F $logfile

# Make sure that CheckStatus has been run for this build cycle .....
CheckStatus $_TYPE $T_CHECKSYMPTOM $_BLDCYCLE $BLDCYCLE
[[ $? = 0 ]] || {
    log -b "CheckSymptom has not been run for $BLDCYCLE"
    log -b "Please run CheckSymptom"
    clean_up
    exit 0
}

# Set abstractlist, symptoms, failures files; build sort/uniq'd
# abstractlist and symptoms lists .....
ABSTRACTLIST="${temp_file}/abstractlist.$$"
CHECKSYMP_FAILURE="${temp_file}/checksymp_failures"
SYMPTOMS="${temp_file}/symptoms.$$"
sort +0 -u $(bldglobalpath)/abstractlist > $ABSTRACTLIST || \
                log -x "sort error on $(bldglobalpath)/abstractlist"
bldunique $(bldglobalpath)/symptoms > $SYMPTOMS || \
                log -x "bldunique error on $(bldglobalpath)/symptoms"

# Set local memos and defect/APAR list, output abstracts and memo_info
# locations .....
DEFECTAPARS="defapars.$$"
ABSTRACTS="$(bldhistorypath)/abstracts"
MEMO_INFO="$(bldhistorypath)/memo_info"
MEMOS="$(bldglobalpath)/memos"

# Build local list of defect/APARs from defectapars .....
[[ -s "$(bldglobalpath)/defectapars" ]] || \
                log -x "$(bldglobalpath)/defectapars not found"
sed 's/\|/ /g' $(bldglobalpath)/defectapars > $DEFECTAPARS || \
                log -x "sed error on $(bldglobalpath)/defectapars"

# Build the memo_info/abstracts files based on the symptoms/memos
# files .....
logline="Appending to the file $ABSTRACTS with apars, "
log -b "$logline abstracts and symptoms"
while read defect apars release family
do
    # If defectapars file in old format, need to add family onto defect.
    if [[ "${family}" = "" ]]  
    then
        defect="${defect}:${DEFAULT_CMVCFAMILY}"
    else
        defect="${defect}:${family}"
    fi
    sym_get $apars $defect $ABSTRACTLIST $SYMPTOMS $ABSTRACTS
    found=$(echo `egrep "^${defect%%:*}[|:]" $MEMOS | awk -F"|" '{ print $1 }'`)
    if [[ -n "$found" ]]
    then
        memo_get $apars $defect $MEMOS $MEMO_INFO
    fi
done < $DEFECTAPARS

# Convert the raw abstracts and memo_info files to the
# stanza format .....
sym_get "" "" "" "" $ABSTRACTS -w
memo_get "" "" "" $MEMO_INFO -w

# Let's see what levels we've not been able to process yet .....
QueryStatus $_TYPE $T_CHECKSYMPTOM \
            $_SUBTYPE $S_SYMPTOMS \
            $_BLDCYCLE $BLDCYCLE \
            $_STATUS $ST_FAILURE \
            $_DATE "$_DONTCARE" \
            $_BUILDER "$_DONTCARE" -A | \
    awk -F"|" '{ print $3, $4 }' > $CHECKSYMP_FAILURE

# Delete success/failure records for bldabstracts in this build
# cycle before determining whether bldabstracts is clean to go .....
DeleteStatus $_TYPE $T_BLDABSTRACTS \
             $_SUBTYPE $S_SYMPTOMS \
             $_BLDCYCLE $BLDCYCLE \
             $_STATUS $ST_SUCCESS
DeleteStatus $_TYPE $T_BLDABSTRACTS \
             $_SUBTYPE $S_SYMPTOMS \
             $_BLDCYCLE $BLDCYCLE \
             $_STATUS $ST_FAILURE

# If we had any failed instances of CheckSymptoms, don't let tools
# that depend on bldabstracts to proceed (mark this build cycle as
# failed) .....
if [[ -s $CHECKSYMP_FAILURE ]]
then
    logline="CheckSymptom failed for the following releases and levels"
    logline="$logline (see the log file CheckSymptom.all for defect"
    logline="$logline numbers)"
    log -e "$logline"
    cat $CHECKSYMP_FAILURE
    cat $CHECKSYMP_FAILURE >> $logfile

    # Tag bldabstracts as not yet complete .....
    bldsetstatus $_TYPE $T_BLDABSTRACTS \
                 $_SUBTYPE $S_SYMPTOMS \
                 $_BLDCYCLE $BLDCYCLE \
                 $_STATUS $ST_FAILURE
else
    # Tag bldabstracts as complete .....
    bldsetstatus $_TYPE $T_BLDABSTRACTS \
                 $_SUBTYPE $S_SYMPTOMS \
                 $_BLDCYCLE $BLDCYCLE \
                 $_STATUS $ST_SUCCESS
fi

# Clean up, bail out .....
clean_up

#
# End of bldabstracts.
#

