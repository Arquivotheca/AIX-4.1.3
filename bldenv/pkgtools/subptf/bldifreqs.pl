#! /usr/bin/perl
# @(#)48	1.20  src/bldenv/pkgtools/subptf/bldifreqs.pl, pkgtools, bos412, GOLDA411a 4/1/94 14:24:45
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: 	bldifreqs
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
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
# NAME: bldifreqs
#
# FUNCTION: Calculate ifreqs for a build cycle.
#
#   There are four types of ifreqs:
#	Type 1: If two or more ptfs contain a common defect, then the
#		ptfs are ifreq'd together.
#	Type 2: The ptf(s) which contain a previous defect dependency
#		are ifreq'd.
#	Type 3: The ptf(s) which contain an ifreq'd defect, which the 
#		user has defined in CMVC, are ifreq'd.
#
# INPUT: requisites (parameter) - list of CMVC requisites
#	 allptfgroups (parameter) - the ptf groups
#	 allprevdeps (parameter) - the previous dependencies
#	 primeptfhist (parameter) - the primary ptf history
#	 secptfhist (parameter) - the secondary ptf history
#        BLDPTFPREVIF (environment) - if null, do not include previous 
#            dependencies as ifreqs
#
# OUTPUT: the ifreqs are written to stdout (by file name)
#
# FORMATS:
#	requisites: fromrelease.fromdefect|torelease.todefect (per line)
#	allptfgroups: ptfid|file|dd,dd...|idd,idd... (per line)
#		where dd is a direct defect and idd is an indirect defect
#	allprevdeps: file|previousdefect,previousdefect... (per line)
#	primeptfhist: defect|ptfid,ptfid... (per line)
#	secptfhist: defect|ptfid,ptfid... (per line)
#	stdout: file|ifreq-ptf,ifreq-ptf...
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

($requisites,$allptfgroups,$allprevdeps,
 $primeptfhist,$secptfhist) = @ARGV;

&Getdata($requisites,*cmvcifreqs) || 
    &log("-x","unable to get previous PTF data");
&Getdata($allprevdeps,*prevdeps) || 
    &log("-x","unable to get previous PTF data");
&Getdata($primeptfhist,*primeptf) || 
    &log("-x","unable to get prime PTF data");
&Getdata($secptfhist,*secptf) || 
    &log("-x","unable to get secondary PTF data");

$familycode = 0;

open (PTFGROUPS,"<$allptfgroups") || 
    &log("-x","unable to get PTF group data from $allptfgroups ($!)");
while ($groupline=<PTFGROUPS>) {
    chop $groupline;
    ($ptfid,$file,$directdefects,$indirectdefects) = split(/\|/,$groupline);
    $fileptfs{$ptfid,$file} = $DEFINED;
    @directdefects = split(/$SEP/,$directdefects);
    foreach $defect (@directdefects) {
       	if ($cmvcifreqs{$defect} ne undef) 
            { $oifreqs{$file} .= $cmvcifreqs{$defect} . $SEP; }
    }
    @indirectdefects = split(/$SEP/,$indirectdefects);
    foreach $defect ((@directdefects,@indirectdefects)) {
        $defects{$file} .= $defect . $SEP;
	($release,$dnum) = split(/\./,$defect);
        if ($releaseFamily{$release} eq undef) {
            chop($family=`ksh bldgetfamily $release`);
            #  Discard any '@host@socket` part of the family name
            ($family) = split('@', $family);
            if ( $family eq undef ) {
                &log("-x","unable to determine family for release $release");
            }
            
            if ($families{$family} eq undef) {
                $familycode = $familycode +1;
                $families{$family} = $familycode;
            }
            
            $fcode = $families{$family};
            $releaseFamily{$release} = $fcode;
        }
        
        $fcode = $releaseFamily{$release};
                                
        #  Appending the family code $fcode to the defect number will
        #  differentiate the same number from different families.
        $dnum .= "." . $fcode;
        $ptfids{$dnum} .= $ptfid . $SEP;
    }
}


# remove duplicates from ptfids
foreach $dnum (keys %ptfids) {
    $ptfids{$dnum}=&Uniquesort($SEP,$ptfids{$dnum});
}

foreach $file (sort keys %defects) {
    undef %ifreqs;
    (defined $ENV{'DEBUG'}) && &log("-b","finding ifreqs for $file");

    #### Type 1: common defect ifreqs
    if ($defects{$file} ne undef) {
        foreach $defect (split(/$SEP/,$defects{$file})) {
            ($release,$dnum) = split(/\./,$defect);
            $dnum .= "." . $releaseFamily{$release};
            foreach $ptfid (split(/$SEP/,$ptfids{$dnum})) {
                if ($fileptfs{$ptfid,$file} != $DEFINED) {
                    $ifreqs{$ptfid} = $DEFINED;
                    (defined $ENV{'DEBUG'}) && 
                        &log("-b", "common ifreqs - $defect:$ptfid");
    }   }   }   }

    #### Type 2: previous dependency ifreqs
    if ($prevdeps{$file} ne undef && $ENV{"BLDPTFPREVIF"} ne undef) {
        foreach $defect (split(/$SEP/,$prevdeps{$file})) {
            if ($secptf{$defect} ne undef) {
                 foreach $ptfid (split(/$SEP/,$secptf{$defect})) {
                      if ($fileptfs{$ptfid,$file} != $DEFINED) {
                           $ifreqs{$ptfid} = $DEFINED;
                      }
                 }
            }
            else 
                { &log("-w","$defect not found in secondary ptf history"); }
            (defined $ENV{'DEBUG'}) && 
                &log("-b","previous ifreqs - $defect:$secptf{$defect}");
        }
    }

    #### Type 3: CMVC ifreqs
    if ($oifreqs{$file} ne undef) {
        foreach $defect (split(/$SEP/,$oifreqs{$file})) {
            if ($primeptf{$defect} ne undef) {
                foreach $ptfid (split(/$SEP/,$primeptf{$defect})) {
                    if ($fileptfs{$ptfid,$file} != $DEFINED) {
                        $ifreqs{$ptfid} = $DEFINED;
                    }
                }
            }
            else 
                { &log("-w","$defect not found in primary ptf history"); }
            (defined $ENV{'DEBUG'}) && 
            &log("-b","CMVC ifreqs - $defect:$primeptf{$defect}");
        }
    }

    $formifreqs = &Uniquesort($SEP,join($SEP,keys %ifreqs));
    print $file . $PSEP . $formifreqs . "\n" if ($formifreqs ne "");
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

