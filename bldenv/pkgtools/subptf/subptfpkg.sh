#! /bin/ksh
# @(#)18	1.7  src/bldenv/pkgtools/subptf/subptfpkg.sh, pkgtools, bos412, GOLDA411a 6/30/94 11:46:37
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: subptfpkg
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. bldloginit
. bldkshconst  # define constants

#
# NAME: Cleanup
#
# FUNCTION: Cleanup subptfpkg environment (for all functions)
#
# INPUT: all environment variables containing temporary file names
#
# OUTPUT: none
#
# SIDE EFFECTS: all existing subptfpkg temporary files are deleted
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: n/a
#
function Cleanup
{
        [[ -n "$DEBUG" ]] && log -csubptfpkg "removing temporary files"
        rm -f $FILENAMES $UPDATA $GROUPS $PTFGROUPS $PRIMEPTFHIST \
              $SECPTFHIST $ALLPTFGROUPS $ALLPREVDEPS $PTFIFREQS $PTFCOREQS \
              $PTFPREREQS $LPPPTFGROUPS $DEFECTLIST $TMPREQS $IFREQS \
       	      $COREQS $PREREQS $MERGEDATA $UNIQLIST
       	rm -fr $JUNKDIR
	bldlock -u ${locklpp:-"undef"} $PERM;
	logset -r
}

#
# NAME: Primaryhistory
#
# FUNCTION: Return, from ptf group data, the primary PTF history data.  
#	Primary PTF history data are the defects within a PTF which
#	directly caused a PTF file to be built.
#
# INPUT: ptfgroups (parameter) - ptf group data file
#
# OUTPUT: primary ptf history data written to stdout 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#	ptfgroups - ptfid|file|directdefects...|indirectdefects... (per line)
#	stdout - ptfid|defect... (per line)
#
# RETURNS: 0 (successful) or non-zero (failure) or 2 (premature exit)
function Primaryhistory 
{
        typeset ptfgroups=$1
        [[ -n "$DEBUG" ]] && log -b "extracting primary history"

	[[ -r "$ptfgroups" ]] || log -x "unable to read group file ($ptfgroups)"
	bldcut $ptfgroups BAR 1 3 ||
		log -x "unable to cut group file ($ptfgroups)"
}

#
# NAME: Secondaryhistory
#
# FUNCTION: Return, from ptf group data, the secondary PTF history data.  
#	Secondary PTF history data are the defects within a PTF which
#	directly or indirectly (bldenv) caused a PTF file to be built.
#
# INPUT: ptfgroups (file) - ptf group data
#
# OUTPUT: secondary ptf history data written to stdout 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#	ptfgroups - ptfid|file|directdefects...|indirectdefects... (per line)
#	stdout - ptfid|defect... (per line)
#
# RETURNS: 0 (successful) or non-zero (failure) or 2 (premature exit)
function Secondaryhistory 
{
        typeset ptfgroups=$1
	typeset SAVEIFS ptf file direct indirect separator both
        [[ -n "$DEBUG" ]] && log -b "extracting secondary history"
	[[ -r "$ptfgroups" ]] || log -e "unable to read group file ($ptfgroups)"

	SAVEIFS=$IFS; IFS='|'	
	ERRNO=0
	while read ptf file direct indirect
	do
		if [[ -n "$direct" && -n "$indirect" ]] ; then
			separator=','
		else
			separator=''
		fi
		both="$direct$separator$indirect"
		print -- "$ptf|$both"
		[[ $ERRNO -ne 0 && $ERRNO -ne 25 ]] &&
		     log -x "Write error creating secondary history ($ERRNO)"
	done < $ptfgroups
	IFS=$SAVEIFS
}

#
# NAME: Ptfhistory
#
# FUNCTION: Return list of old defects and the PTFs they are in.
#
#	Since the total list of old defects can be very large, return
#	only those while are going to be used now.  This is determined
#	by extracting all defect numbers from the current data lists.
#
# INPUT: allptfgroups (parameter) - file of dependency data grouped by ptfid 
#	 allprevdeps (parameter) - file of previous dependency data by filename
#	 requisites (parameter) - file of requisite data (indirectly from CMVC)
#
# OUTPUT: defect/ptfid list written to stdout 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#	allptfgroups - ptfid|file|defects...|defects... (per line)
#	allprevdeps  - file|defects... (per line)
#	requisites   - type|release|defect|release|defect (per line)
#	stdout	     - defect|ptfid (per line)
#
# RETURNS: 0 (successful) or non-zero (failure)
typeset -ft Ptfhistory
function Ptfhistory 
{
        typeset type=$1 allptfgroups=$2 allprevdeps=$3 requisites=$4
        [[ -n "$DEBUG" ]] && log -b "retrieving $type PTF history"
	DEFECTLIST=$(bldtmppath)/ptfhistory.tmp$$
	UNIQLIST=$(bldtmppath)/uniqlist.tmp$$

	# Extract only the defect numbers from the input list.  If the
        # defect number are in lists (e.g.  "129,298,983"), split the
        # defects into one per line by converting the ',' to a newline.
	bldcut $requisites BAR 3 > $DEFECTLIST
	bldcut $requisites BAR 5 >> $DEFECTLIST
	bldcut $allptfgroups BAR 3 | tr ',' '\12' >> $DEFECTLIST
	bldcut $allptfgroups BAR 4 | tr ',' '\12' >> $DEFECTLIST
	bldcut $allprevdeps BAR 2 | tr ',' '\12' >> $DEFECTLIST
	grep "." $DEFECTLIST| sort | uniq > $UNIQLIST #remove blanks and dups

	# get the ptfid for each defect in the list
	bldhistory $type $UNIQLIST || {
	    [[ $? -ne 1 ]] && logarg="-x"; 
	    log $logarg "getting $type ptf history."; 
	    }
	rm -f $DEFECTLIST $UNIQLIST
	return $?
}

#
# NAME: Selectrequisites
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#
# RETURNS: 0 (successful) or non-zero (failure)
function Selectrequisites  
{
        typeset reqtype=$1 requisites=$2 rc=$SUCCESS
        [[ -n "$DEBUG" ]] && log -b "Selecting requisite data ($@)"
        typeset TMPREQS=$(bldtmppath)/tmpreqs;

        grep "^$reqtype\|" $requisites > $TMPREQS
        [[ $? = 2 ]] && log -e "unable to grep requisite file ($requisites)"

        tempIFS=$IFS; IFS=$PSEP;
	ERRNO=0
        while read type fromrelease fromopt fromdefect torelease toopt  todefect
        do
                print -- "$fromrelease.$fromdefect$PSEP$torelease.$todefect"
		[[ $ERRNO -ne 0 && $ERRNO -ne 25 ]] &&
		     log -x "Write error selecting requisites ($ERRNO)"
        done < $TMPREQS
        IFS=$tempIFS
        return $rc
}

function Mergedefects {
        typeset inputfile=$1 IFS

        IFS='|'
	ERRNO=0
        while read file directdefects indirectdefects
        do
                defects=$directdefects
                if [[ -n "$directdefects" ]]
                then
                        if [[ -n "$indirectdefects" ]]
                        then
                                defects="$directdefects,$indirectdefects"
                        fi
                else
                        defects=$indirectdefects
                fi
                print -- "$file|$defects"
		[[ $ERRNO -ne 0 && $ERRNO -ne 25 ]] &&
		     log -x "Write error merging defects ($ERRNO)"
        done < $inputfile
}


#
# NAME: Handlelpp
#
# FUNCTION: Merges subsystem UPDATE information for the lpp.
#
# INPUT: locklpp -  Previous lpp name or null
#        lppname -  Current lpp name or null
#        "subsystem" directories under $TOP/UPDATE
#
# OUTPUT: "lpp" directory under $TOP/UPDATE
#	LPPLIST (file) - file containing all lpps the files cover.
#
# SIDE EFFECTS: Increments totallpps (Count of lpps)
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#	LPPLIST - one lpp name per line
#
# RETURNS: 0 (successful) or non-zero (failure)

function Handlelpp {
	[[ -w $LPPLIST ]] || log -x "cannot write to lpp list file $LPPLIST"
 	if [[ -n "$locklpp" ]] 
 	then
	    #  Merge subsystem data for the lpp
	    [[ ! -d $LPPDIR/$locklpp ]] && (mkdir $LPPDIR/$locklpp ||
	    	log -x "creating $LPPDIR/$locklpp directory")
	    if [[ -d $JUNKDIR/$locklpp ]]
	    then
	        cp -p $JUNKDIR/$locklpp/filepath $LPPDIR/$locklpp/filepath.$BLDCYCLE
	        cp -p $JUNKDIR/$locklpp/ptf_pkg $LPPDIR/$locklpp/ptf_pkg.$BLDCYCLE
	    fi
	    #  Watch out for case where only the "lpp" subsystem was made.
	    numsubs=$(li -1 $JUNKDIR/$locklpp.*/filelist 2>/dev/null | wc -l)
	    if [[ $numsubs -gt 0 ]]
	    then
                cat -q $JUNKDIR/$locklpp.*/filelist >>$LPPDIR/$locklpp/filelist.$BLDCYCLE ||
                    log -x "creating composite filelist for $locklpp"
                cat -q $JUNKDIR/$locklpp.*/ptf_pkg  >>$LPPDIR/$locklpp/ptf_pkg.$BLDCYCLE ||
                    log -x "creating composite ptf_pkg for $locklpp"
            fi
            bldsetstatus $_TYPE $T_BLDPTF \
                     $_SUBTYPE $S_LPP \
                     $_BLDCYCLE $BLDCYCLE \
		     $_LPP $locklpp \
                     $_STATUS $ST_SUCCESS ||
			log -x "setting bldptf status for LPP $locklpp"
	    log -b "$lppptfs PTF(s) in LPP $locklpp"
 	    bldlock -u $locklpp $PERM
        fi
 	if [[ -n "$lppname" ]]
 	then
 	    locklpp=$lppname
 	    lppptfs=0
	    bldlock -l $locklpp $PERM || log -x "unable to get $locklpp lock"
	    let totallpps=totallpps+1
	    echo $lppname >> $LPPLIST
	fi
}

#
# NAME: subptfpkg
#
# FUNCTION: Generate PTF data for each LPP.
#           This is the "subsystem" version of bldptfpkg.
#
# INPUT: BLDCYCLE (parameter) - the current build cycle 
#        BLDPTFRETAIN (parameter) - "retain" or "noretain"; if retain use retain ids
#
# OUTPUT: PTFOUT (files) - the PTF data for an LPP 
#	  PTFOPTIONLIST (file) - the options within each PTF
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: The build process environment
#
# FORMATS/DATA STRUCTURES:
#			                . . .
# RETURNS: 0 (successful) or non-zero (failure) or 2 (premature exit)

################################  S E T U P  ################################ 
logset -c$0 +l # init log command
[[ -n "$DEBUG" ]] && log -b "entering ($@)"
[[ $# = 2 ]] || log -x "$0: illegal syntax"
BLDCYCLE=$1; export BLDCYCLE  
BLDPTFRETAIN=$2  # specifies to use retain ids of generate own
log -b "Generating PTF package data for build cycle $BLDCYCLE"
rc=$UNSET
logarg="-e"

LOG=${LOGBLDGETLISTS:-$(bldlogpath)/bldgetlists.all}
trap 'Cleanup; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

SUBSYSLIST=$(bldglobalpath)/subsyslist; readonly SUBSYSLIST
LPPDIR=$(bldupdatepath); readonly LPPDIR
FILENAMES=$(bldglobalpath)/namelist; readonly FILENAMES ## temporary name
UPDATA=$(bldtmppath)/bldptf.updata$$; readonly UPDATA
MERGEDATA=$(bldtmppath)/bldptf.mergedata$$; readonly MERGEDATA
GROUPS=$(bldtmppath)/bldptf.groups$$; readonly GROUPS
PTFGROUPS=$(bldtmppath)/bldptf.ptfgroups$$; readonly PTFGROUPS
PRIMEPTFHIST=$(bldtmppath)/bldptf.primeptfhis$$; readonly PRIMEPTFHIST
SECPTFHIST=$(bldtmppath)/bldptf.secondptfhist$$; readonly SECPTFHIST
ALLPTFGROUPS=$(bldtmppath)/bldptf.allptfgroups$$; readonly ALLPTFGROUPS
ALLPREVDEPS=$(bldtmppath)/bldptf.allpreviousdeps$$; readonly ALLPREVDEPS
PTFIFREQS=$(bldtmppath)/bldptf.ptfifreqs$$; readonly PTFIFREQS
PTFCOREQS=$(bldtmppath)/bldptf.ptfcoreqs$$; readonly PTFCOREQS
PTFPREREQS=$(bldtmppath)/bldptf.ptfprereqs$$; readonly PTFPREREQS
LPPPTFGROUPS=$(bldtmppath)/bldptf.lppptfgroups$$; readonly LPPPTFGROUPS
DEFECTAPARS=$(bldglobalpath)/defectapars; readonly DEFECTAPARS
APARPTFIDS=$(bldglobalpath)/ptfids; readonly APARPTFIDS
APARPTFIDSORG=$(bldglobalpath)/ptfids.org; readonly APARPTFIDSORG
IFREQS=$(bldtmppath)/bldptf.ifreqs$$; readonly IFREQS
COREQS=$(bldtmppath)/bldptf.coreqs$$; readonly COREQS
PREREQS=$(bldtmppath)/bldptf.prereqs$$; readonly PREREQS
PTFOPTIONLIST=$(bldhistorypath)/ptfoptions; readonly PTFOPTIONLIST
REQUISITES=$(bldglobalpath)/ptfrequisites; readonly REQUISITES
XREFDATA=$(bldglobalpath)/xreflist; readonly XREFDATA
PTFINFO=$(bldhistorypath)/ptfinfo.$BLDCYCLE; readonly PTFINFO
JUNKDIR=$LPPDIR/bldptf.$BLDCYCLE; readonly JUNKDIR

typeset -i totalup totalptfs thiscount lppptfs=0 totallpps=0 numofptfs
typeset -Z dumid

> $PTFINFO      # delete any old data, plus insure the file is created
> $(bldglobalpath)/bldinfo/alllppptfs  # delete any old data - insure file there

cp $(bldglobalpath)/ptfids.org $(bldglobalpath)/ptfids ||
        log -x "unable to refresh $(bldglobalpath)/ptfids"

################################# M A I N  ##################################
typeset LPPLIST=$LPPDIR/lpplist.$BLDCYCLE

>$LPPLIST
CheckStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDPTFDEPEND $_BLDCYCLE $BLDCYCLE
if [[ $? != 0 ]] ; then
	log -x "there is no release data to process from bldptfdepend"
fi

bldinfo GETFILENAMES > $FILENAMES || log -x "getting update file names"
[[ -n "$DEBUG" ]] && cat $FILENAMES | log +f "update file names:"

subfilelpp $XREFDATA $FILENAMES $JUNKDIR || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "subfilelpp found errors"; }
[[ -r "$SUBSYSLIST" ]] || log -x "LPP list not found - no LPPS to package"

## set up dummy PTF id's starting number and prefix
if [[ -n "$PTFSEED" ]] ; then
        prefix=$(echo $PTFSEED | sed 's/[0-9]*$//') ## remove trailing digits
        [[ -n "$prefix" ]] && export PTFPREFIX=$prefix
        suffix=$(echo $PTFSEED | sed 's/.*[A-Za-z]//') ## remove leading alphas
        dumid=$suffix
else
        dumid=1001
fi

bldhistory INITPTFIDS ||  # refresh ptf history
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "refreshing ptf bldhistory."; }
while read sysname
do
	lppname=${sysname%%.*}
	[[ -n "$DEBUG" ]] && log -b "processing SUBSYSTEM $sysname"
	bldinfo GETUP $JUNKDIR/$sysname/filelist > $UPDATA ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting update data from bldinfo for $sysname in $JUNKDIR"; }
	subgroup $sysname $UPDATA > $GROUPS ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "forming PTF groups for $sysname"; }
	bldaddptfids $BLDPTFRETAIN $dumid $GROUPS $DEFECTAPARS $APARPTFIDS \
            $APARPTFIDSORG > $PTFGROUPS || 
            { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting PTF identifiers for $sysname"; }
	bldsaveptfinfo $lppname $PTFGROUPS >> $PTFINFO ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "saving ptf info for $sysname"; }
	Primaryhistory $PTFGROUPS > $PRIMEPTFHIST ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting primary PTF history date for $sysname"; }
	bldhistory SAVEPRIMEPTF $PRIMEPTFHIST ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "saving primary PTF history in bldhistory for $sysname"; }
	Secondaryhistory $PTFGROUPS > $SECPTFHIST ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting secondary PTF history date for $sysname"; }
	bldhistory SAVESECONDPTF $SECPTFHIST ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "saving secondary PTF history in bldhistory for $sysname"; }
	bldinfo SAVELPPPTFS $sysname $PTFGROUPS ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "saving PTF groups in bldinfo for $sysname"; }
	thiscount=$(cat $PTFGROUPS | wc -l)
	[[ $thiscount -ne 0 ]] && log -b "$thiscount update file(s) in SUBSYSTEM $sysname"
	let totalup=totalup+thiscount
	numofptfs=$(cat $PTFGROUPS | cut -d'|' -f1 | sort -u | wc -l)
	let dumid=dumid+numofptfs
done < $SUBSYSLIST
log -b "$totalup TOTAL update files"

log -b "calculating $BLDCYCLE requisite data"
bldinfo GETALLPTFS > $ALLPTFGROUPS ||
    log -x "getting PTF groups from bldinfo"
bldinfo GETALLPREVDEPS > $ALLPREVDEPS ||
    log -x "getting all previous dependencies"
Ptfhistory GETPRIMEPTF $ALLPTFGROUPS $ALLPREVDEPS $REQUISITES |
    sort -u > $PRIMEPTFHIST || log -x "getting primary ptf history"
Ptfhistory GETSECONDPTF $ALLPTFGROUPS $ALLPREVDEPS $REQUISITES | 
    sort -u > $SECPTFHIST || log -x "getting secondary ptf history"
Selectrequisites "ifreq" $REQUISITES > $IFREQS || 
    log -x "unable to get CMVC if-requisites data"
bldifreqs $IFREQS $ALLPTFGROUPS $ALLPREVDEPS $PRIMEPTFHIST $SECPTFHIST \
    > $PTFIFREQS || log -x "finding PTF if-requisites"
Selectrequisites "coreq" $REQUISITES > $COREQS ||
    log -x "unable to get CMVC co-requisites data"
bldcoreqs $COREQS $ALLPTFGROUPS > $PTFCOREQS ||
    log -x "finding PTF co-requisites"
Selectrequisites "prereq" $REQUISITES > $PREREQS ||
    log -x "unable to get CMVC pre-requisites data"
bldprereqs $PREREQS $ALLPTFGROUPS $PRIMEPTFHIST > $PTFPREREQS ||
    log -x "finding PTF pre-requisites"

locklpp=
DeleteStatus -F $_TYPE $T_BLDPTF $_SUBTYPE $S_LPP $_BLDCYCLE $BLDCYCLE
while read sysname
do
	lppname=${sysname%%.*}
	[[ "$lppname" != "$locklpp" ]] && Handlelpp
	[[ -n "$DEBUG" ]] && log -b "packaging SUBSYSTEM $sysname PTFs"
	PTFOUT=$JUNKDIR/$sysname/ptf_pkg
	bldinfo GETLPPPTFS $sysname > $LPPPTFGROUPS ||
	    { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "getting PTF groups in bldinfo for $sysname"; }
	subptfformat $sysname $LPPPTFGROUPS $PTFIFREQS $PTFCOREQS  \
		$PTFPREREQS $DEFECTAPARS > $PTFOUT ||
		{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "formatting ptf data for SUBSYSTEM $sysname"; }
	subptfopts $PTFOUT $PTFOPTIONLIST || 
		{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "saving PTF options for SUBSYSTEM $sysname"; }
	thiscount=$(cat $PTFOUT | cut -f1 -d'|' | sort | uniq | wc -l)
	[[ $thiscount -ne 0 ]] && log -b "$thiscount PTF(s) in SUBSYSTEM $sysname"
	let totalptfs=totalptfs+thiscount
	let lppptfs=lppptfs+thiscount
done < $SUBSYSLIST
lppname=
[[ -n "$locklpp" ]] && Handlelpp
log -b "$totalptfs TOTAL PTFs (including $totalup files across $totallpps LPPs)"

[[ -n "$DEBUG" ]] && log -b "exiting"
[[ $rc = $UNSET ]] && rc=$SUCCESS; exit
