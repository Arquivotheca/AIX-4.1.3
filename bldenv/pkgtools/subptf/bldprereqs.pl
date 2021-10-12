#! /usr/bin/perl
# @(#)60	1.4  src/bldenv/pkgtools/subptf/bldprereqs.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:10:52
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: 	bldprereqs
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

push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    # define constants
do 'bldperlfunc';     # define common functions
do 'bldperllog';      # init perl interface to log functions

#
# NAME: bldprereqs
#
# FUNCTION: Calculate prereqs for a build cycle.
#
# INPUT: requisites - list of CMVC requisites
#	 allptfgroups - the ptf groups
#        primeptfhist - the primary ptf history
#
# OUTPUT: the prereqs are written to stdout (by file name)
#
# FORMATS:
#	requisites: fromrelease.fromdefect|torelease.todefect,.. (per line)
#	allptfgroups: ptfid|dd,dd...|idd,idd... (per line)
#		where dd is a direct defect and idd is an indirect defect
#       primeptfhist: defect|ptfid,ptfid... (per line)
#	stdout: file|prereq-ptf,prereq-ptf...
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
&logset("-c $COMMAND +l");  # init log command
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
(defined $ENV{'DEBUG'}) && &log("-b","entering (@ARGV)");

($requisites,$allptfgroups,$primeptfhist) = @ARGV;

&Getdata($requisites,*cmvcprereqs) || 
    &log("-x","unable to get previous PTF data");
&Getdata($primeptfhist,*primeptf) ||
    &log("-x","unable to get prime PTF data");

open (PTFGROUPS,"<$allptfgroups") || 
    &log("-x","unable to get PTF group data from $allptfgroups ($!)");
while ($groupline=<PTFGROUPS>) {
    chop $groupline;
    ($ptfid,$file,$directdefects,$indirectdefects)=split(/\|/,$groupline);
    @directdefects = split(/$SEP/,$directdefects);
    foreach $defect (@directdefects) {
       	if ($cmvcprereqs{$defect} ne undef) { 
		# may be multiple prereqs associated with the defect
		$oprereqs{$file} .= $cmvcprereqs{$defect} . $SEP;
	}
    }
    $fileptf{$file} = $ptfid;
    $afileptfs{$ptfid,$file} = $DEFINED;
}


foreach $file (keys %fileptf) {
    undef %prereqs;
    (defined $ENV{'DEBUG'}) && &log("-b","finding prereqs for $file");

    foreach $defect (split(/$SEP/,$oprereqs{$file})) {
        if ($primeptf{$defect} ne undef) { 
            foreach $ptfid (split(/$SEP/,$primeptf{$defect})) {
                if ($afileptfs{$ptfid,$file} != $DEFINED) {
                    $prereqs{$ptfid} = $DEFINED; 
                }
            }
        } 
        else 
            { &log("-e","$defect not found in current this build's PTFs"); }
        (defined $ENV{'DEBUG'}) && 
        &log("-b","CMVC prereqs - $defect:$primeptf{$defect}");
    }

    $formprereqs = &Uniquesort($SEP,join($SEP,keys %prereqs));
    print $file . $PSEP . $formprereqs . "\n" if ($formprereqs ne "");
    if ( $! != 0 ) {
        &log("-x","write error ($!)");
    }
}

(defined $ENV{'DEBUG'}) && &log("-b","exiting");
&logset("-r");
exit $rc;

sub Abort {
	&logset("-r");
        exit $FATAL;
}

