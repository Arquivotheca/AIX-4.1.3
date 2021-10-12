#! /usr/bin/perl
# @(#)58	1.7  src/bldenv/pkgtools/subptf/bldsaveptfinfo.pl, pkgtools, bos412, GOLDA411a 6/24/93 15:06:43
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldsaveptfinfo
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
do 'bldperllog';      # init perl interface to log functions
do 'bldperlfunc';     # define general functions

$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');

($#ARGV == 1) || &log("$lvar -x","illegal syntax ($COMMAND @ARGV)");
$lpp=$ARGV[0];
open (DATA,"<$ARGV[1]") || &log("$lvar -x","file ($ARGV[1]) open error");

$BLDCYCLE=$ENV{"BLDCYCLE"};

while (<DATA>) {	# for each input line
	chop;
	($ptfid,$file,$direct,$indirect) = split(/\|/);
	$ptfdefects{$ptfid} .= $direct . $SEP if ($direct ne undef);
	$ptfdefects{$ptfid} .= $indirect . $SEP if ($indirect ne undef);
}
close (DATA);

$familycode = 0;

chop($workpath=`ksh bldglobalpath`); $workpath .= "/defectapars";
open (APARS,"<$workpath") || 
	&log("$lvar -x","apar file ($workpath) open error");
while (<APARS>) {
	chop;
	($defect,$apar,$release,$family) = split(/\|/);
        #  Append a "family identifier" to the defect number to distinguish
        #  the same number from different families.
        #  Discard any '@host@socket` part of the family name
        ($family) = split('@', $family);
        if ( $family eq undef ) {
            &log("$lvar -e","unable to determine family for defect $defect in aparfile $workpath");
        }

        if ($families{$family} eq undef) {
            $familycode = $familycode +1;
            $families{$family} = $familycode;
        }
        $fcode = $families{$family};
        $dnum = $defect . $SEP . $fcode;
	$apars{$dnum}=$apar;
}
close (APARS);

foreach $ptfid (keys %ptfdefects) {
	foreach $defect (split(/$SEP/,&Uniquesort($SEP,$ptfdefects{$ptfid}))) {
             next if ($defect eq undef);
             ($release,$defectnum) = split(/\./,$defect);
             #  Append the "family identifier" to the defect number so that
             #  the same number from different familes is distinguished.
             $fcode = 0;
             if ($releaseFamily{$release} eq undef) {
                 chop($family=`ksh bldgetfamily $release`);
                 #  Discard any '@host@socket` part of the family name
                 ($family) = split('@', $family);
                 if ( $family eq undef ) {
                     &log("$lvar -e","unable to determine family for release $release");
                 }
                 elsif ($families{$family} eq undef) {
                     &log("$lvar -e","family $family was not found for any release in aparfile $workpath.");
                 }
                 else {
                     $releaseFamily{$release} = $families{$family};
                     $fcode = $releaseFamily{$release};
                 }
             }
             else {
                 $fcode = $releaseFamily{$release};
             }

             $dnum = $defectnum . $SEP . $fcode;
             $apar=$apars{$dnum};
	     print "$ptfid\t$release\t$apar\t$defectnum\t$BLDCYCLE\t$lpp\n";
	     if ( $! != 0 ) {
	    	&log("$lvar -x","write error ($!)");
	     }
	}
}
	
exit $rc;

sub Abort {
        exit $FATAL;
}

