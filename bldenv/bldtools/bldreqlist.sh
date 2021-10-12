#! /bin/ksh
# @(#)07	1.3  src/bldenv/bldtools/bldreqlist.sh, bldtools, bos412, GOLDA411a 4/22/93 15:31:25
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldreqlist
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

# NOTE: THIS COMMAND IS INCOMPLETE; THE CALL TO DEFREQS AND THE FORMATTING
# OF DEFREQ DATA IS DUMMIED OUT.  WHEN THE REAL DEFREQ COMMAND IS DROPPED
# (BY MIKE MCBRIDE), IT MUST BE ROLLED IN.

. bldloginit   ## init log functions
. bldkshconst  ## define constants

#
# NAME: FormatDefReqs
#
# FUNCTION: format the DefReqs data into standard format
#
# INPUT: defreqsfile - file containing DefReqs data
#
# OUTPUT: stdout - the formated data
#
# FORMAT: 
#	DefReqs data:
#		
#	stdout:
#		reqtype|fromrel|fromdefect|fromlpp|torel|todefect|tolpp
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: nothing
#
function FormatDefReqs 
{
	typeset defreqsfile=$1

	while read defline
	do
		print $defline
	done < $defreqsfile
}

#
# NAME: bldreqlist
#
# FUNCTION: build and verify a list of defect requisites 
#
# INPUT: release - the CMVC release
#	 defectlist - list of defects for this release and build cycle
#
# OUTPUT: $(bldreleasepath $release)/defrequisites is created
#         all requisites are logged
#
# FORMAT:
#	defrequisites:
#		reqtype|fromrel|fromdefect|fromlpp|torel|todefect|tolpp
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 for success; non-zero otherwise
#
trap 'rm -f $DEFECTS $DEFREQS $TMPREQS; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

release=$1
defectlist=$2

DEFREQS=$(bldtmppath)/defrequisites$$
DEFECTS=$(bldtmppath)/defects$$
TMPREQS=$(bldtmppath)/tmprequisites$$
PTFREQS=$(bldglobalpath)/ptfrequisites
lvar="-c$0 +l"

[[ -r "$defectlist" ]] || log $lvar -x "unable to read defect list $defectlist"

sort -u $defectlist > $DEFECTS
## uncomment the next section when the reql DefReqs command is dropped
#while read defect
#do
#	DefReqs $defect >> $DEFREQS
#done < $DEFECTS
cat $(bldglobalpath)/ptfrequisites > $DEFREQS  ## temporarily bypass DefReqs

FormatDefReqs $DEFREQS > $TMPREQS
cat $PTFREQS >> $TMPREQS
sort -u $TMPREQS > $PTFREQS

IFS='|'
while read type fromrel fromlpp fromdef torel tolpp todef
do
	flpp=""; tlpp=""
	[[ -n $fromlpp ]] && flpp="($fromlpp) "
	[[ -n $tolpp ]] && tlpp="($tolpp)"
	log $lvar -b \
		"$type found: $fromrel.$fromdef $flpp--> $torel.$todef $tlpp"
done < $PTFREQS

bldverifyreqs $PTFREQS

exit
