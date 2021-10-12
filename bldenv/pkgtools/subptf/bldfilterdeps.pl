#! /usr/bin/perl
# @(#)88	1.3  src/bldenv/pkgtools/subptf/bldfilterdeps.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:07:33
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: main
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
# NAME: main
#
# FUNCTION: Remove all previous-dependencies from this build cycle which 
#           existed in the previous cycles.  We are only interested in
#           previous-dependencies if they are new.
#
#	    The previous-dependencies at this point are in the form of
#
# INPUT: release (command line)
#
# OUTPUT: none (see SIDE EFFECTS) 
#
# SIDE EFFECTS: alldepend files in release's bldtd dir are pruned and rewritten
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';      # define constants
do 'bldperllog';        # init perl interface to log functions

$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";  # log parameters
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');

&log("$lvar -x","illegal syntax") if ($#ARGV != 0);
$release=$ARGV[0];

chop($tmpdir=`ksh bldtmppath`);
$allfilelist="$tmpdir/bldtdallfiles";

chop($historydir=`ksh bldhistorypath`); $historydir .= "/bldtd";
`mkdir $historydir` if (! -d $historydir);
chop($bldtddir=`ksh bldreleasepath $release`); $bldtddir .= "/bldtd";

`li -1 "$bldtddir" | grep alldepend > $allfilelist`;

open (FILES,"<$allfilelist") ||
	&log("$lvar -x","input file ($allfilelist) open error ($!)");
while ($file = <FILES>) {
        undef %histnames;
        if (open(HISTLIST,"<$historydir/$file")) {
		while ($item=<HISTLIST>) {
			chop $item;
			$histnames{$item}=$DEFINED;
		}
	}
        open(NEWLIST,">$bldtddir/nlist") ||
		&log("$lvar -x","newfile ($bldtddir/nlist) open error ($!)");
        open(BLDTDLIST,"<$bldtddir/$file") ||
		&log("$lvar -x","bldtd file ($bldtddir/$file) open error ($!)");
        while ($item=<BLDTDLIST>) {
		chop $item;
		if ($histnames{$item} eq undef) {
			$! = 0;
			print NEWLIST "$item\n";
        		if ( $! != 0 ) {
           		    &log("$lvar -x","processing $file $bldtddir/nlist write error ($!)");
           		}
		}
	}
	close(NEWLIST); close(BLDTDLIST);
	`mv $bldtddir/nlist $bldtddir/$file`;	
}

`rm -f $allfilelist`;

sub Abort {
	`rm -f $allfilelist`;
        exit $FATAL;
}

