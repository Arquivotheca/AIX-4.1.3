#! /usr/bin/perl
# @(#)19	1.5  src/bldenv/pkgtools/subptf/subptfformat.pl, pkgtools, bos412, GOLDA411a 10/14/93 16:52:54
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    subptfformat
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

push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    # define constants
do 'bldperlfunc';     # define common functions
do 'bldperllog';      # init perl interface to log functions

#
# NAME: subptfformat
#
# FUNCTION: Format PTF data.
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
&logset("-c $COMMAND +l");  # init log command
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');
(defined $ENV{'DEBUG'}) && &log("-b","entering (@ARGV)");

($subsystem,$ptfgroupfile,$ifreqfile,$coreqfile,$prereqfile,$aparfile)=@ARGV;

chop ($globalpath=`ksh bldglobalpath`);

&Getdata($ifreqfile,*ifreqs) || &log("-x","unable to get ifreq data");
&Getdata($coreqfile,*coreqs) || &log("-x","unable to get coreq data");
&Getdata($prereqfile,*prereqs) || &log("-x","unable to get prereq data");

$familycode = 0;

open (APARFILE,"<$aparfile") ||
	&log("-x","unable to get APAR data from $aparfile ($!)");
while ($aparline=<APARFILE>) {
        chop $aparline;
	($defect,$apar,$release,$family) = split(/\|/,$aparline);
        #  Discard any '@host@socket` part of the family name
        ($family) = split('@', $family);
        if ( $family eq undef ) {
            &log("-e","unable to determine family for defect $defect in aparfile $aparfile");
        }

        if ($families{$family} eq undef) {
            $familycode = $familycode +1;
            $families{$family} = $familycode;
        }
        $fcode = $families{$family};
        $dnum = $defect . $SEP . $fcode;
	$defectapars{$dnum} = $apar;
}

open (PTFGROUPS,"<$ptfgroupfile") || 
	&log("-x","unable to get group data from $ptfgroupfile ($!)");
while ($groupline=<PTFGROUPS>) {
        chop $groupline;
	($ptfid,$file,$directdefects,$indirectdefects) = split(/\|/,$groupline);
        $files{$ptfid} .= $file . $SEP;
	@directdefects = split($SEP,$directdefects);
	@indirectdefects = split($SEP,$indirectdefects);
        $ptfids{$ptfid} = $DEFINED;
        foreach $dtoken ((@directdefects,@indirectdefects)) {
		#  Append a "family modifier" to the defect number so that
		#  the same number from different familes is distinguished.
        	$fcode = 0;
		($release,$defect) = split(/\./,$dtoken);
                if ($releaseFamily{$release} eq undef) {
                    chop($family=`ksh bldgetfamily $release`);
                    #  Discard any '@host@socket` part of the family name
                    ($family) = split('@', $family);
                    if ( $family eq undef ) {
                        &log("-e","unable to determine family for release $release");
                    }
                    elsif ($families{$family} eq undef) {
                        &log("-e","family $family was not found for any defect in aparfile $aparfile.");
                    }
                    else {
                        $releaseFamily{$release} = $families{$family};
                        $fcode = $releaseFamily{$release};
                    }
                }
                else {
                    $fcode = $releaseFamily{$release};
                }
                
		$dnum = $defect . $SEP . $fcode;
		if ($defectapars{$dnum} ne "") {
 		   $apars{$file} .= $defectapars{$dnum} . $SEP;
		}
		else {
		   if (! $already_error{$dnum}) {
		      &log("-e", "null APAR for defect $defect (defect used)");
		      $already_error{$dnum}=1;
                   }
		   $apars{$file} .= $defect . $SEP;
		}
	}
	chop $apars{$file};
}

foreach $ptf (sort keys %ptfids) {
	foreach $file (sort split(/$SEP/,$files{$ptf})) {
		$formatline = $ptf . $PSEP;
		$formatline .= &Uniquesort($SEP,$apars{$file}) . $PSEP;
                # Strip subsystem prefix from liblpp.a file.
		$ofile = $file;
                if ($file =~ m/liblpp.a$/) {
                    $file =~ s#^.*!!#!!#;
                }
		$formatline .= $file . $PSEP . $subsystem . $PSEP;
		$formatline .= $ifreqs{$ofile} . $PSEP;
		$formatline .= $coreqs{$ofile} . $PSEP;
		$formatline .= $prereqs{$ofile} . $PSEP;
		$formatline .= "\n";
		$formatline =~ s/$SEP/ /g;  # convert internal separators
		$formatline =~ s#!!/ship##;  # remove !! prefix from file name
		print $formatline;
		if ( $! != 0 ) {
		    &log("-x","write error ($!)");
		}
	}
}

(defined $ENV{'DEBUG'}) && &log("-b","exiting");
&logset("-r");
exit $rc;

sub Abort {
	&logset("-r");
        exit $FATAL;
}

