#! /usr/bin/perl
# @(#)37	1.9  src/bldenv/pkgtools/subptf/bldcoreqs.pl, pkgtools, bos412, GOLDA411a 4/1/94 14:24:34
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: 	bldcoreqs
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
# NAME: bldcoreqs
#
# FUNCTION: Calculate coreqs for a build cycle.
#
# INPUT: requisites - list of CMVC requisites
#	 allptfgroups - the ptf groups
#
# OUTPUT: the coreqs are written to stdout (by file name)
#
# FORMATS:
#	requisites: fromrelease.fromdefect|torelease.todefect,.. (per line)
#	allptfgroups: ptfid|dd,dd...|idd,idd... (per line)
#		where dd is a direct defect and idd is an indirect defect
#	stdout: file|coreqs-ptf,coreqs-ptf...
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

($requisites,$allptfgroups) = @ARGV;

&Getdata($requisites,*cmvccoreqs) || 
    &log("-x","unable to get previous PTF data");

open (PTFGROUPS,"<$allptfgroups") || 
    &log("-x","unable to get PTF group data from $allptfgroups ($!)");
while ($groupline=<PTFGROUPS>) {
    chop $groupline;
    ($ptfid,$file,$directdefects,$indirectdefects)=split(/\|/,$groupline);
    @directdefects = split(/$SEP/,$directdefects);
    foreach $defect (@directdefects) {
       	if ($cmvccoreqs{$defect} ne undef) { 
		# may be multiple coreqs associated with the defect
		$ocoreqs{$file} .= $cmvccoreqs{$defect} . $SEP;
	}
        $primeptf{$defect} .= $ptfid . $SEP;
    }
    $fileptf{$file} = $ptfid;
    $afileptfs{$ptfid,$file} = $DEFINED;
}


foreach $file (keys %fileptf) {
    undef %coreqs;
    (defined $ENV{'DEBUG'}) && &log("-b","finding coreqs for $file");

    foreach $defect (split(/$SEP/,$ocoreqs{$file})) {
        if ($primeptf{$defect} ne undef) { 
            foreach $ptfid (split(/$SEP/,$primeptf{$defect})) {
                if ($afileptfs{$ptfid,$file} != $DEFINED) {
                    $coreqs{$ptfid} = $DEFINED; 
                }
            }
        } 
        else 
            { &log("-e","$defect not found in this build cycle's PTFs"); }
        (defined $ENV{'DEBUG'}) && 
        &log("-b","CMVC coreqs - $defect:$primeptf{$defect}");
    }

    $formcoreqs = &Uniquesort($SEP,join($SEP,keys %coreqs));
    print $file . $PSEP . $formcoreqs . "\n" if ($formcoreqs ne "");
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

