#! /usr/bin/perl
# @(#)21	1.6  src/bldenv/bldtools/rename/bldshowreqs.pl, bldtools, bos412, GOLDA411a 12/3/92 16:02:20
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldshowreqs
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

## read every defect for a release and log any start/stop requisites
## defined in the CMVC note (for the defect).  For a defect, only 
## the last start/stop requisite (in the note) is counted.

push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    # define constants
do 'bldperllog';      # init perl interface to log functions

$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$logpath=`ksh bldlogpath`; chop $logpath;
&logset("-c $COMMAND +l -F $logpath/ptfrequisites.all");  # log parameters

$release=$ARGV[0];

open (DEFECTS,"cat `bldreleasepath $release`/defects |");
while ($defect=<DEFECTS>) {	# for each input line
	chop $defect;
	$linecount=0;
	open (NOTE,"Defect -view $defect -long | ");
	while (<NOTE>) {
		if (/^STOP_REQUISITE[ \t\n\r\f]*$/i) { $readreq=0; }
		if ($readreq) { $req[$linecount++]=$_; }
		if (/^START_REQUISITE[ \t\n\r\f]*$/i) { 
			$linecount=0; undef @req; $readreq=1; 
		}
	}
	if ($linecount > 0) { 
		&log("-b","REQUISITE found for $defect (add to ptfrequisites)");
		foreach $reqline (@req) {
			chop $reqline;
			&log("-c'Requisite' +l","$reqline");;
		}
	}
}
&logset("-r");
