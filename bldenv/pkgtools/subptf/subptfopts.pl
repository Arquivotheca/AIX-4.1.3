#! /usr/bin/perl
# @(#)92	1.3  src/bldenv/pkgtools/subptf/subptfopts.pl, pkgtools, bos412, GOLDA411a 6/8/94 15:06:56
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    subptfopts
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
# NAME: subptfopts
#
# FUNCTION: Add new ptfs and their options to the ptfoptions file.
#
# INPUT: ptffile - file new ptfs and their options are taken from.
#        optionsfile - file containing all previous ptfs and their options.
#
# OUTPUT: optionsfile - file containing all ptfs and their options.
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
&logset("-c $COMMAND +l");  # init log command
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
(defined $ENV{'DEBUG'}) && &log("-b","entering (@ARGV)");

($ptffile,$optionsfile)=(@ARGV);

open (PTFGROUPS,"<$ptffile") || 
	&log("-x","unable to get group data from $datafile ($!)");
while ($ptfline=<PTFGROUPS>) {
        chop $ptfline;
	($ptfid,$apars,$file,$options,$ifreqs,$coreqs,$prereqs) = 
		split(/\|/,$ptfline);

	undef $realopts;
	foreach $opt (split(/ /,$options)) {
		if (defined $realopts) {
			$realopts .= $SEP . $opt;
		}
		else {
			$realopts = $opt;
		}
	}

	$newptfoptions{$ptfid} .= $realopts . $SEP if $realopts ne "";
}
close (PTFGROUPS);

open (OPTIONSFILE,"$optionsfile") || 
	&log("-x","unable to get option data from $optionsfile ($!)");
while ($optionline=<OPTIONSFILE>) {
	chop $optionline;
	($ptfid,$options) = split(/ /,$optionline);
	$ptfoptions{$ptfid} = $options;
}
close (OPTIONSFILE);
foreach $ptfid (keys %newptfoptions) {
	$ptfoptions{$ptfid} = $newptfoptions{$ptfid};
}

open (OPTIONSFILE,">$optionsfile") || 
	&log("-x","unable to open $optionsfile for output ($!)");
foreach $ptfid (sort keys %ptfoptions) {
	print OPTIONSFILE "$ptfid ";
	print OPTIONSFILE &Uniquesort($SEP,$ptfoptions{$ptfid}) . "\n";
	if ( $! != 0 ) {
	    &log("-x","$optionsfile write error ($!)");
	}
}

(defined $ENV{'DEBUG'}) && &log("-b","exiting");
close (OPTIONSFILE);
&logset("-r");
exit $rc;

sub Abort {
	&logset("-r");
        exit $FATAL;
}

