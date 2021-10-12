#! /usr/bin/perl
# @(#)08	1.3  src/bldenv/bldtools/bldverifyreqs.pl, bldtools, bos412, GOLDA411a 9/24/92 13:53:49
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldverifyreqs
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
do 'bldperllog';      # init perl interface to log functions

#
# NAME: bldverifyreqs
#
# FUNCTION: Verify all the requisites defects specified in the PTF requisite
#	    file can be found (give error or warning message if not)
#
#	Note: Source requisite defects must be located or an error is
#	      listed; a warning is given for target requisite defects
#	      if not found and the target release has not yet been
#	      prebuilt (if target release has been prebuilt then an error
#	      message is given)
#
# INPUT: PTF requisites file name (see format below)
#
# OUTPUT: Error messages and warning messages if requisite defects cannot
#	  be found
#
# SIDE EFFECTS: none
#
# FORMAT:
#       PTF requisite file: 
#		type|fromrel|fromlpp|fromdefect|torel|tolpp|todefect
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 is all requisite defects are found; non-zero otherwise
#
$rc=0;
$COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
$lvar="-c $COMMAND +l";  # log parameters

&GetCurrentProductData(*data);  ## get this builds defects
&GetPreviousData(*data);	## get defects from previous builds

open (DATA,"<$ARGV[0]") || &log("$lvar -x","file ($ARGV[0]) open error");
while (<DATA>) {	# for each PTF requisite input line
	chop;
        ($type,$fromrel,$fromlpp,$fromdefect,$torel,$tolpp,$todefect) = 
		split(/\|/);
	if ($data{$fromrel,$fromdefect} eq undef) { ## if from-defect not found
	    &log("$lvar -e","requisite source $fromrel.$fromdefect not found");
	}
	if ($data{$torel,$todefect} eq undef) { ## if to-defect not found
		## if the release been prebuild yet for this build cycle
 		if (system("CheckStatus $_TYPE $T_PREBUILD $_BLDCYCLE 							$ENV{BLDCYCLE} $_RELEASE $torel 						$_SUBTYPE $S_DELTAEXTRACT") == 0) 
		{  ## ERROR case - release prebuilt but defect still not found
		    &log("$lvar -e",
			"requisite target $torel.$todefect not found...");
		}  ## WARNING case - since release has not been prebuild
		else {  
		    &log("$lvar -w",
			"requisite target $torel.$todefect not found...");
		    $rc=1;  ## there are still unresolved references
		}	
	        &log("$lvar -b",
			".........$fromrel.$fromdefect is source defect");
	}
}

exit $rc;

#
# NAME: GetCurrentProductData
#
# FUNCTION: get this build cycle's defects and associated releases
#
# INPUT: none
#
# OUTPUT: data - AA of "release,defect" data found
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: n/a
#
sub GetCurrentProductData {
	local(*data) = @_;
	local($path,@subpaths,$release,$defect);
	open(DEFECTPATHS,"li -1 $ENV{TOP}/PTF/$ENV{BLDCYCLE}/*/defects |");
	while ($path=<DEFECTPATHS>) {  ## for each 'defects' file
		chop $path;
		(@subpaths) = split(/\//,$path);  ## split the path name up
		$release=$subpaths[$#subpaths-1]; ## get release name from path
		open (DEFECTS, "<$path") ||
			&log("-x","unable to open $path ($!)");
		while ($defect=<DEFECTS>) {  ## for each defect in file
			chop $defect;
			$data{$release,$defect}=$DEFINED;
		}
	}
}

#
# NAME: GetPreviousData
#
# FUNCTION: get previous build cycle's defects and associated releases
#
# INPUT: none
#
# OUTPUT: data - AA of "release,defect" data found
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: n/a
#
sub GetPreviousData {
	local(*data) = @_;
	local($ptfid,$release,$apar,$defect,$bldcycle,$lpp);
	open (ALLPTFINFO,"cat `ksh bldhistorypath`/ptfinfo.* 2> /dev/null |");
	while (<ALLPTFINFO>) {
		chop;
		($ptfid,$release,$apar,$defect,$bldcycle,$lpp)=split;
		$data{$release,$defect}=$DEFINED;
	}
}
