#! /usr/bin/perl
# @(#)50	1.12  src/bldenv/pkgtools/subptf/bldcurdef.pl, pkgtools, bos412, GOLDA411a 11/17/92 11:05:13
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldcurdef
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

# Declare common perl code/constants.
push(@INC,split(/:/,$ENV{"PATH"})); # define search path
do 'bldperlconst';    	# general bld constants
do 'bldperllog';	# log functions
do 'bldtdlib';		# bldtd database functions
do 'bldinfolib';	# bldinfo database functions

#
# NAME: bldcurdef
#
# FUNCTION: Gather, for each input file, the defect numbers in this
#	    build cycle which caused the file to be built.  Defects 
#	    are seperated into two categories.  First are the defects
#	    which are included in the current release's level and caused
#	    this file to be built. Second are the defects which caused 
#	    the build environment to be modified in another release's 
#	    level (but in this build cycle) and now indirectly cause 
#	    this file to be built.
#
#	For each file, the first set of defects can be retrieved from the
#	target-dependency datebase.  The second set is found by first 
#	getting the NEW dependencies for the file from the target-dependency
#	database.  For each of these new dependencies, check to see if
#	another release has associated bldenv data (defect numbers).  If
#	there is then save these defects which indirectly caused the file
#	to be built. 
#
# INPUT: release (parameter) - CMVC level release name
# 	 fixlist (parameter) - file name containing the list of files to
#		gather defects for.
#
# OUTPUT: Dependency data for each file is written to stdout.  The format
#		is "filename|c c c ...|e e e...".
#	  where 'c' is a current release defect and e is a build environment 
#         defect.
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
  $rc=$UNSET;
  $COMMAND=$0; $COMMAND =~ s#/.*/##;  # save this command's basename
  $lvar="-c $COMMAND +l";  # log parmeters
  %SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
        'TERM','Abort','ABRT','Abort');

  ($release,$listfile) = (@ARGV);  ## input parameters

  chop($workpath=`ksh bldreleasepath $release`);

  ## initialize the bldtd and bldinfo databases for access
  (&BLDTDinterfaceinit($release) == $SUCCESS) || 
	&log("$lvar -x","bldtd interface initialization error");
  (&BLDINFOinterfaceinit() == $SUCCESS) || 
	&log("$lvar -x","bldinfo interface initialization error");

  open(LIST,"<$listfile") || &log("$lvar -x","$listfile open error ($!)");
  while ($file=<LIST>) {	
	chop $file;
        undef @directdefects; undef @dependlist; undef %indirectdefects;

	## get defects which directly caused the file to be modified
	&BLDTD2gettargetdefects ($file,*directdefects);

	## references to the bldenv are affected by tpath translation -
	## so two names could mean the same file - normalize to fix this
        $cfile = $file; ## get copy of file name
	$cfile =~ s#.*@@#!!#;  ## if bldenv filename, then normalize the name

	## get all newly built dependents/decendents of the file
  	(&BLDTDgetdependentslist($workpath,$cfile,"NEW",*dependlist)==$SUCCESS) 
	    || &log("$lvar -e","getting NEW dependents for $file in $release");

	## check each dependent to see if it is bldenv file with assoc. defects
	foreach $depfile (@dependlist) {
		chop ($depfile);
		&BLDINFO2getenvdefects ($depfile,*indirectdefects);
	}

	## put any indirect defects in ASCII format (comma separated)
        $indirectstr=join(',',(sort keys %indirectdefects));

	## if no defects at all were found then error
	if ($directdefects eq undef && $indirectstr eq "") {
		if (! &IgnoreUpdatefile($file)) {
	 		&log("$lvar -e","$file has no defects in $release");
		}
	}
	else {  ## ok, so write the data to stdout
 		print "$file|$directdefects|$indirectstr\n";
		if ( $! != 0 ) {
		    &log("$lvar -x","write error ($!)");
		}
	}
  }

  if ($rc == $UNSET) { exit $SUCCESS; } else { exit $FAILURE; }


#
# NAME: IgnoreUpdatefile
#
# FUNCTION: Specifies if errors for an input file should be skipped/ignored
#
# INPUT: file (parameter) - the input file name
#
# OUTPUT: none
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (do not ignore) or 1 (ignore)
#
sub IgnoreUpdatefile {
	local ($file) = @_; ## input parameter - the file name
        return (
		($file =~ m/liblpp.a$/) || 
                ($file =~ m/lpp_name$/)
               );	
}

sub Abort {
        exit $FATAL;
}

