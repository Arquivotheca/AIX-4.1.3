#! /usr/bin/perl
# @(#)60	1.13  src/bldenv/bldtools/rename/bldquery.pl, bldtools, bos412, GOLDA411a 1/14/93 11:14:07
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldquery
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bldquery
#
# FUNCTION: 
#
# INPUT:
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

############################################################################
# function:	cleanup
# description:	cleans up and exits
# input:
# output:
# remarks:	
############################################################################
sub	cleanup	{
	system("rm -f $ALL_QUERY_FILE $QUERY_FILE $SUM_FILE");
	exit;
}

############################################################################
# function:	read_targetlist
# description:	Reads target/dependency data from the files:
#
#		$BQTOP/HISTORY/bldquery/TARGETLIST and
#		$BQTOP/HISTORY/bldquery/NTARGETLIST 
#
#		and stores results in the array, targetlist
# input:	TARGETLIST and NTARGETLIST files
# output:	none
# remarks:	
############################################################################
sub	read_targetlist {
	# have targetlist and Ntargetlist arrays already been created?
	if (! $TARGLIST_READ) {
		# if not created, open TARGLIST file for reading
		open(TARGLIST,"$BQPATH/TARGLIST");

		# read TARGLIST and create targetlist array
		while (<TARGLIST>) {
			chop $_;
			($dep,$tlist) = split(/\|/,$_);
			$targetlist[$dep] = $tlist;
		}

		# open NTARGLIST file for reading (new targets this build)
		open(NTARGLIST,"$BQPATH/NTARGLIST");

		# read NTARGLIST file and create Ntargetlist array
		while (<NTARGLIST>) {
			chop $_;
			($dep,$tlist) = split(/\|/,$_);
			$Ntargetlist[$dep] = $tlist;
		}
		close(NTARGLIST);

		# targetlist and Ntargetlist have now been created
		$TARGLIST_READ = $TRUE;
	}
}

############################################################################
# function:	
# description:	Reads target/dependency data from the files:
#
#		$BQTOP/HISTORY/bldquery/DEPENDLIST and
#		$BQTOP/HISTORY/bldquery/NDEPENDLIST 
#
#		and stores results in the array, dependentlist
# input:	TARGETLIST and NTARGETLIST files
# output:	none
# remarks:	
############################################################################
sub	read_dependentlist {
	# have dependentlist and Ndependentlist arrays already been created
	if (! $DEPENDLIST_READ) {
		# if not created, open DEPENDLIST file
		open(DEPENDLIST,"$BQPATH/DEPENDLIST");

		# reading DEPENDLIST file and creating dependentlist array
		while (<DEPENDLIST>) {
			chop $_;
			($targ,$dlist) = split(/\|/,$_);
			$dependentlist[$targ] = $dlist;
		}
		close(DEPENDLIST);
		open(NDEPENDLIST,"$BQPATH/NDEPENDLIST");

		# reading NDEPENDLIST file and creating Ndependentlist array
		while (<NDEPENDLIST>) {
			chop $_;
			($targ,$dlist) = split(/\|/,$_);
			$Ndependentlist[$targ] = $dlist;
		}
		close(NDEPENDLIST);
		# dependentlist and Ndependentlist have now been created
		$DEPENDLIST_READ = $TRUE;
	}
}

############################################################################
# function:	process_queries
# description:	checks to see if input is coming from a file or user and
#		calls the routine doquery accordingly
# input:	query code
# output:	output from the specified query
# remarks:	
############################################################################
sub	process_queries {
	# get query code
	local($qc) = @_;

	system("rm -f $ALL_QUERY_FILE");

	# get appropriate query information for the querycode selected
	($ord,$getfunc,$queryfunc,$summaryfunc,$nametype,$filequery,
	    $description) = split(/\|/,$query_info{$qc});

	# has an input file been created or is the selected query type one
	# that is not compatible with using an input file (e.g. "r" & "l")
	if (($INFILE eq "") || (!$filequery)) {
	    # if no input file specified, process query using $NAME
	    &doquery($qc);
	} else {
	    # if input file specified, open it now
	    if (!open(INF,"< $INFILE")) {
	        print STDERR "Couldn't open file $INFILE for input.\n";
	    } else {
		# read input file
	        while ($line = <INF>) {
		    chop($line);
		    if ($line =~ /^ *$/) {
			next;
		    }
		    ($NAME,$relstr) = split(/ [\(\<]/,$line);
		    chop($relstr); # chop off trailing ")" or ">"
		    # get release name ($lpp will have LPP name if there
		    # is one but it is not needed)
		    ($RELEASE,$lpp) = split(/\:/,$relstr);
		    $NAME =~ s/^ *//g;
		    $NAME =~ s/ *$//g;

		    `echo "Query \"$qc\" on $nametype: $NAME" >> $ALL_QUERY_FILE`;
		    `echo "-------------------------------------------------------------------------------" >> $ALL_QUERY_FILE`;

		    # process query for each file name in input file
		    if (&doquery($qc) != 0) {
			return -1;
		    }
		    system("rm -f $QUERY_FILE");
		}
		close(INF);
	    }
	}
	close(OUTF1);

	return 0;
}

############################################################################
# function:	doquery
# description:	processes a given query
# input:	query code
# output:	file/release/LPP information for the given query
# remarks:	
############################################################################
sub	doquery {
	# get query code
	local($qc) = @_;

	$QUERYSTATUS="";

	system("rm -f $QUERY_FILE");
	if (!open(OUTF1,">> $QUERY_FILE")) {
		print STDERR "Couldn't create temporary file $QUERY_FILE\n";
		if ($MENU_MODE) {
			&Pause;
		}
		return -1;
	}

	# call the function to prompt the user for the file, LPP, or 
	# release name
	if ($getfunc ne "") {
		&$getfunc;
	}

	# call the function to perform the query that corresponds to the
	# querycode entered
	if ($queryfunc ne "") {
		if (&$queryfunc != 0) {
			&PrintLine("Query failed: $QUERYSTATUS","");
		}
	}

	close(OUTF1);

	system("sort -u $QUERY_FILE -o $QUERY_FILE");

	if (!open(OUTF2,">> $SUM_FILE")) {
	        print STDERR "Couldn't create temporary file $SUM_FILE\n";
		if ($MENU_MODE) {
	        	&Pause;
		}
	        return -1;
	}
	&$summaryfunc($QUERYCODE,$QUERY_FILE);
	close(OUTF2);

	if (($VERBOSE == 1) || ($VERBOSE == 2)) {
		system("cat $QUERY_FILE >> $ALL_QUERY_FILE");
		`echo "" >> $ALL_QUERY_FILE`;
	}
	if ((($VERBOSE == 0) || ($VERBOSE == 2)) && ($QUERYSTATUS eq "")) {
		system("cat $SUM_FILE >> $ALL_QUERY_FILE");
		`echo "" >> $ALL_QUERY_FILE`;
	}

	system("rm -f $QUERY_FILE $SUM_FILE");

	return 0;
}

############################################################################
# function:	ParseRelOwner
# description:	this function returns the name of the release that owns a
#		given file
# input:	file id
# output:	none
# remarks:	
############################################################################
sub	ParseRelOwner {
	local($id) = @_;
	local($filename,$release) = split(/\|/,$decode{$id});

	if ($release ne "") {
		return("$release");
	} elsif (defined $bldenv{$id}) {
		return("$bldenv{$id}");
	} else {
		return("$NORELMSG");
	}
}

sub	ParseFileName {
	local($id) = @_;
	local($filename,$release) = split(/\|/,$decode{$id});

	return($filename);
}

############################################################################
# function:	ParseLPPstr
# description:	takes a string of LPP names, creates an entry in the 
#		associative array %lppsfound for each unique LPP name in 
#		the string; the string of LPP names is then recreated w/
#		any duplicate LPP names removed
# input:	pointer to LPP string
#		pointer to %lppsfound associative array
# output:	updated LPP name string and %lppsfound associative array
# remarks:	
############################################################################
sub	ParseLPPstr {
	local(*lppstr,*lppsfound) = @_;

	local(@lpps)=split(/ /,$lppstr);
	local($lpp);
	$lppstr = "";
	foreach $lpp (@lpps) {
		if (! defined $lppsfound{$lpp}) {
			$lppstr .= "$lpp ";
			$lppsfound{$lpp} = $DEFINED;
		}
	}
	chop($lppstr);
}

############################################################################
# function:	GetFileName
# description:	prompts user for a file name
# input:	file name from the user
# output:	file name prompt
# remarks:	
############################################################################
sub	GetFileName {
	while ($NAME eq "") {
		if ($SEARCHDIR ne "") {
			print "Enter file name: $SEARCHDIR/";
		} else {
			print "Enter file name: ";
		}
		chop($NAME = <STDIN>);
		# prepending the search directory path name to file name
		# if the search directory is set
		if ($SEARCHDIR ne "") {
			$NAME="$SEARCHDIR/$NAME";
		}

		if ($NAME !~ /^\.\//) {
			print STDERR "Invalid filename: $NAME.  ";
			print STDERR "Filenames must begin with \"./\"\n\n";
			$NAME="";
		}
	}
	while ($RELEASE eq "") {
		print "Enter release name to which this file belongs: ";
		chop($RELEASE = <STDIN>);
	}
}

############################################################################
# function:	GetReleaseName
# description:	prompts user for a release name
# input:	release name from the user
# output:	release name prompt
# remarks:	
############################################################################
sub	GetReleaseName {
	while ($NAME eq "") {
		print "Enter release name: ";
		chop($NAME = <STDIN>);
	}
}

############################################################################
# function:	GetLPPName
# description:	prompts user for a LPP name
# input:	LPP name from the user
# output:	LPP name prompt
# remarks:	
############################################################################
sub	GetLPPName {
	while ($NAME eq "") {
		print "Enter LPP name: ";
		chop($NAME = <STDIN>);
	}
}

############################################################################
# function:	PrintLine
# description:	prints a filename and possibly an add-on string (typically
#		an LPP or release name) to stdout
# input:	filename and add-on string
# output:	printed line to stdout
# remarks:	
############################################################################
sub	PrintLine {
	local($name,$addon) = @_;

	# testing against include and exclude filters before printing
	if ((($INCLUDESTR eq "") || ($name =~ /$INCLUDESTR/)) && 
	    (($EXCLUDESTR eq "") || ($name !~ /$EXCLUDESTR/))) {
		print OUTF1 "$name$addon\n";
	}
}

############################################################################
# function:	CreateFileSummary
# description:	parses listing of file names and counts number of ship files,
#		intermediate files, source files, build environment files,
#		and unique LPPs and release names found in the file
# input:	query code and name of file to parse
# output:	summary report containing statistics listed above
# remarks:	
############################################################################
sub	CreateFileSummary {
	local($qc,$sumfile) = @_;
	local($rel);
	local($lppstr);
	local(%lppsfound);
	local(%relsfound);
	local(@lpps);
	local(@rels);
	local($line);
	local($filename);
	local($fileid);
	local($addon);
	local($CNT,$SHIPCNT,$INTCNT,$SOURCECNT,$BLDENVCNT,$RELCNT,$LPPCNT);

	if ($filequery) {
		open(SUM,"< $sumfile");

		$CNT=$SHIPCNT=$INTCNT=$SOURCECNT=$BLDENVCNT=$RELCNT=$LPPCNT=0;

		while($line = <SUM>) {
			chop($line);
			($filename,$addon) = split(/[\(\<]/,$line);
			chop($filename);  # chop off trailing space
			chop($addon);	  # chop off trailing ")" or ">"
			if (defined $addon) {
			    if ($addon =~ /\:/) {
				($rel,$lppstr) = split(/\:/,$addon);
				# chop off leading space from lppstr
				$lppstr = substr($lppstr,1); 
				if ($lppstr ne "$NOLPPMSG") {
					&ParseLPPstr(*lppstr,*lppsfound);
				}
			    } else {
				$rel = $addon;
				$lppstr = "";
			    }
			    if (($rel ne "$NORELMSG") && 
			        (! defined $relsfound{$rel})) {
				$relsfound{$rel} = $DEFINED;
			    }
			}
			if (defined $encode{$filename}) {
				$fileid = $encode{$filename};
			} else {
				$fileid = $encode{"$filename|$rel"};
			}
			$CNT++;
			if ($filename =~ /\.\/ship/) {
			    $SHIPCNT++;
			} elsif (! defined $dependentlist[$fileid]) {
			    $SOURCECNT++;
			} else {
			    $INTCNT++;
			}
			if (defined $bldenv{$fileid}) {
			    $BLDENVCNT++;
			}
		}
		(@lpps) = (keys %lppsfound);
		(@rels) = (keys %relsfound);
		$LPPCNT = $#lpps + 1;
		$RELCNT = $#rels + 1;

		printf OUTF2 "File Summary\n";
		printf OUTF2 "------------\n";
		printf OUTF2 "Ship files:\t\t\t%6d\n",$SHIPCNT;
		printf OUTF2 "Intermediate files:\t\t%6d\n",$INTCNT;
		printf OUTF2 "Source files:\t\t\t%6d\n",$SOURCECNT;
		printf OUTF2 "Total:\t\t\t\t%6d\n\n",$CNT;
		printf OUTF2 "Build environment files:\t%6d\n\n",$BLDENVCNT;
		printf OUTF2 "Total releases:\t\t\t%6d\n",$RELCNT;
		printf OUTF2 "Total LPPs:\t\t\t%6d\n",$LPPCNT;
	}
}

############################################################################
# function:	CreateRelSummary
# description:	creates summary file containing the number of releases built in 
#		this build
# input:	none
# output:	summary file 
# remarks:	
############################################################################
sub	CreateRelSummary {
	dbmopen(%rels_built,"$BQPATH/RELS_BUILT",0750);

	local(@rels) = (keys %rels_built);
	printf OUTF2 "Release Summary\n";
	printf OUTF2 "---------------\n";
	printf OUTF2 "Total:\t%d\n",$#rels + 1;

	dbmclose(%rels_built);
}

############################################################################
# function:	CreateLPPSummary
# description:	creates summary file containing the number of LPPs built in 
#		this build
# input:	none
# output:	summary file 
# remarks:	
############################################################################
sub	CreateLPPSummary {
	dbmopen(%lpplist,"$BQPATH/LPP/LPPLIST",0750);

	local(@lpps) = (keys %lpplist);
	printf OUTF2 "LPP Summary\n";
	printf OUTF2 "-----------\n";
	printf OUTF2 "Total:\t%d\n",$#lpps + 1;

	dbmclose(%lpplist);
}

############################################################################
# function:	Pause
# description:	prompts user to press enter to continue
# input:	enter key from user
# output:	prompt
# remarks:	
############################################################################
sub	Pause {
	print "\nPress \"Enter\" key to continue: ";
	$dummy = <STDIN>;
}

############################################################################
# function:	TargFiles
# description:	target file query handler routine; prints out all target
#		files in the dependency tree beneath a given dependent file
# input:	dependent file name contained in $NAME
# output:	all targets below the given file in the dependency tree
# remarks:	
############################################################################
sub	TargFiles {
	# open encode, decode and lppnames associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%lppnames,"$BQPATH/LPP/LPPNAMES",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local(%alltargets);
	local($filekey);
	local($filename);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) && 
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alltargets,*Ntargetlist);
	} else {
	  # read from targetlist dependency tree if new mode is off
	  &getchildfileids($encode{"$filekey"},*alltargets,*targetlist);
	}

	# walking thru list of target ids that are targets of the given file
	for $id (keys %alltargets) {
		chop($lppstr = $lppnames{$id});
		undef %lppsfound;
		if ($lppstr ne "") {
			&ParseLPPstr(*lppstr,*lppsfound);
		} else {
			$lppstr = $NOLPPMSG;
		}
		$rowner=&ParseRelOwner($id);
		$filename=&ParseFileName($id);
		if ($filename !~ /\.\/ship/) {
			if (defined $bldenv{$id}) {
				&PrintLine($filename," <$rowner>");
			} else {
				&PrintLine($filename," ($rowner)");
			}
		} else {
			if (($LPPFILTER eq "") || 
			    (defined $lppsfound{$LPPFILTER})) {
				&PrintLine($filename," ($rowner: $lppstr)");
			}
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%lppnames);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	TargTermFiles
# description:	target terminal file query handler routine; prints out all 
#		target files in the dependency tree beneath a given dependent
#		file and which themselves have no targets which depend on them
# input:	dependent file name contained in $NAME
# output:	all targets below the given file in the dependency tree that
#		have no targets that depend on them
# remarks:	
############################################################################
sub	TargTermFiles {
	# open encode and decode associative array files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%lppnames,"$BQPATH/LPP/LPPNAMES",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local($lppstr);
	local(%lppsfound);
	local(%alltargets);
	local($filekey);
	local($filename);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) &&
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alltargets,*Ntargetlist);
	} else {
	  # read from targetlist dependency tree if new mode is off
	  &getchildfileids($encode{"$filekey"},*alltargets,*targetlist);
	}

	# walking thru list of target ids that are targets of the given file
	for $id (keys %alltargets) {
		# are there any files that are targets of the current file? if
		# not then the current file is a target terminal file
		if ((defined $targetlist[$id]) && 
		    ($decode{$id} !~ /\.\/ship/)) {
			next;
	   	} 
		chop($lppstr = $lppnames{$id});
		undef %lppsfound;
		if ($lppstr ne "") {
			&ParseLPPstr(*lppstr,*lppsfound);
		} else {
			$lppstr = $NOLPPMSG;
		}
		$rowner=&ParseRelOwner($id);
		$filename=&ParseFileName($id);
		if ($decode{$id} !~ /\.\/ship/) {
			if (defined $bldenv{$id}) {
				&PrintLine($filename," <$rowner>");
			} else {
				&PrintLine($filename," ($rowner)");
			}
		} else {
			if (($LPPFILTER eq "") || 
			    (defined $lppsfound{$LPPFILTER})) {
		 		&PrintLine($filename," ($rowner: $lppstr)");
	      		}
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%lppnames);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	TargShipFiles
# description:	target ship file query handler routine; prints out all the
#		files in the dependency tree beneath a given dependent file
#		that are part of the ship tree
# input:	dependent file name contained in $NAME
# output:	all target ship files of the given file
# remarks:	
############################################################################
sub	TargShipFiles {
	# opening up encode, decode and lppnames associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%lppnames,"$BQPATH/LPP/LPPNAMES",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local(%alltargets);
	local($lppstr);
	local(%lppsfound);
	local($filename);
	local($filekey);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) &&
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alltargets,*Ntargetlist);
	} else {
	  # read from targetlist dependency tree if new mode is off
	  &getchildfileids($encode{"$filekey"},*alltargets,*targetlist);
	}

	# walking thru list of target ids that are targets of the given file
	for $id (keys %alltargets) {
		# getting LPP name(s) to which the ship file belongs
		if ($decode{$id} =~ /\.\/ship/) {
			chop($lppstr = $lppnames{$id});
			undef %lppsfound;
			if ($lppstr ne "") {
				&ParseLPPstr(*lppstr,*lppsfound);
			} else {
				$lppstr = $NOLPPMSG;
			}
			
			# filtering out all files except the ones that 
			# are part of the LPP specified by $LPPFILTER
			if (($LPPFILTER eq "") || 
			    (defined $lppsfound{$LPPFILTER})) {
			    $rowner=&ParseRelOwner($id);
			    $filename=&ParseFileName($id);
			    if (defined $bldenv{$id}) {
		 		&PrintLine($filename," <$rowner: $lppstr>");
			    } else {
		 		&PrintLine($filename," ($rowner: $lppstr)");
			    }
	      		}
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%lppnames);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	TargBldenvFiles
# description:	target build environment file query handler; prints out all
#		build environment files in the dependency tree beneath a 
#		given dependent file 
# input:	dependent file name contained in $NAME
# output:	all build environment files in the dependency tree beneath
#		the given file
# remarks:	
############################################################################
sub	TargBldenvFiles {
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local(%alltargets);
	local($filekey);
	local($filename);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) &&
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alltargets,*Ntargetlist);
	} else {
	  # read from targetlist dependency tree if new mode is off
	  &getchildfileids($encode{"$filekey"},*alltargets,*targetlist);
	}
	# walking thru list of target ids that are targets of the given file
	for $id (keys %alltargets) {
		if (defined $bldenv{$id}) {
			if (($RELFILTER eq "") || (($RELFILTER ne "") && 
			    ($bldenv{$id} eq $RELFILTER))) {
				$filename = &ParseFileName($id);
				&PrintLine($filename," <$bldenv{$id}>");
			}
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	DepFiles
# description:	dependent file query handler; prints out all files in the
#		dependency tree above a given target file 
# input:	dependent file name contained in $NAME
# output:	all files in the dependency tree dependent on the given 
#		target file
# remarks:	
############################################################################
sub	DepFiles {
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local(%alldependents);
	local($filekey);
	local($filename);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) &&
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	   # read from Ndependentlist dependency tree if new mode is on
	   &getchildfileids($encode{"$filekey"},*alldependents,*Ndependentlist);
	} else {
	   # read from dependentlist dependency tree if new mode is off
	   &getchildfileids($encode{"$filekey"},*alldependents,*dependentlist);
	}
	for $id (keys %alldependents) {
		$rowner = &ParseRelOwner($id);
		$filename = &ParseFileName($id);
		if (defined $bldenv{$id}) {
			&PrintLine($filename," <$rowner>");
		} else {
			&PrintLine($filename," ($rowner)");
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	DepTermFiles
# description:	dependent terminal file query handler; prints out all files
#		in the dependency tree above a given target file that them-
#		selves are not targets of any file
# input:	target file name contained in $NAME
# output:	all files in the dependency tree dependent on the given 
#		target file that in turn are not targets of any file
# remarks:	
############################################################################
sub	DepTermFiles {
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	local(%alldependents);
	local($filename);
	local($filekey);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) && 
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alldependents,*Ndependentlist);
	} else {
	  # read from dependentlist dependency tree if new mode is off
	  &getchildfileids($encode{"$filekey"},*alldependents,*dependentlist);
	}
	for $id (keys %alldependents) {
		# skip this file if it has dependents - we only want dependent
		# terminal files
		if (defined $dependentlist[$id]) {
		    next;
		}

		# getting list of release for which this file is a source file
		$rowner = &ParseRelOwner($id);
		$filename = &ParseFileName($id);

		# filtering out all files except the ones that are part of the
		# release specified by $RELFILTER
		if (($RELFILTER eq "") || ($rowner eq $RELFILTER)) {
		    if (defined $bldenv{$id}) {
			&PrintLine($filename," <$rowner>");
		    } else {
			&PrintLine($filename," ($rowner)");
		    }
		}
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	DepBldenvFiles
# description:	dependent build environment file query handler; prints out
#		all files in the dependency tree that are dependents of a
#		given target file and that are contained in the build
#		environment
# input:	target file name contained in $NAME
# output:	all files in the dependency tree dependent on the give target
#		file and that are part of build environment
# remarks:	
############################################################################
sub	DepBldenvFiles {
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	if (defined $encode{"$NAME|$RELEASE"}) {
		$filekey = "$NAME|$RELEASE";
	} elsif ((defined $encode{"$NAME"}) &&
		 ($bldenv{$encode{"$NAME"}} eq $RELEASE)) {
		$filekey = "$NAME";
	} else {
		$QUERYSTATUS="Unknown file or release: $NAME ($RELEASE)\n";
		return -1;
	}

	local(%alldependents);
	if ($NEWMODE) {
	  # read from Ntargetlist dependency tree if new mode is on
	  &getchildfileids($encode{"$filekey"},*alldependents,*Ndependentlist);
	} else {
	  &getchildfileids($encode{"$filekey"},*alldependents,*dependentlist);
	}
	for $id (keys %alldependents) {
                # testing to see if file is a build environment file
		if (defined $bldenv{$id}) {
			$filename = &ParseFileName($id);
			&PrintLine($filename," <$bldenv{$id}>");
		} 
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	LppNames
# description:	LPP name query handler; prints out the names of all LPPs 
#		that were built
# input:	none
# output:	LPP names
# remarks:	
############################################################################
sub	LppNames {
	# open lpplist associative array DBM file
	dbmopen(%lpplist,"$BQPATH/LPP/LPPLIST",0750);

	local(@lpps) = (keys %lpplist);
	local($lpp);

	# listing all LPPs built
	for $lpp (@lpps) {
		&PrintLine("$lpp","");
	}

	dbmclose(%lpplist);

	return 0;
}

############################################################################
# function:	RelNames
# description:	release name query handler; prints out the names of all 
#		releases that were built
# input:	none
# output:	release names
# remarks:	
############################################################################
sub	RelNames {
	# open associative array containing name of releases that were built
	dbmopen(%rels_built,"$BQPATH/RELS_BUILT",0750);

	local(@rels) = (keys %rels_built);
	local($rel);

	foreach $rel (@rels) {
		&PrintLine($rel,"");
	}

	dbmclose(%rels_built);

	return 0;
}

############################################################################
# function:	RelSrcFiles
# description:	release source file query handler; prints out release source
#		file names for a given release
# input:	release name contained in variable $NAME
# output:	source files for the given release
# remarks:	
############################################################################
sub	RelSrcFiles {
	# opening encode and decode associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);
	dbmopen(%rels_built,"$BQPATH/RELS_BUILT",0750);

	local($rowner,$filename);

	if (! defined $rels_built{$NAME}) {
		$QUERYSTATUS = "Unknown release: $NAME";
		return -1;
	}

	# opening release source file
	if (!open(RELSRC,"$BQPATH/$NAME/SRC")) {
		$QUERYSTATUS="No source files known for release: $NAME\n";
		return -1;
	} else {
		# reading each source file contained release source file
		while ($file = <RELSRC>) {
			chop($file);
			$rowner=&ParseRelOwner($encode{$file});
			$filename=&ParseFileName($encode{$file});
			if (defined $bldenv{$encode{$file}}) {
				&PrintLine($filename," <$rowner>");
			} else {
				&PrintLine($filename," ($rowner)");
			}
	   	}
	}
	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);
	dbmclose(%rels_built);

	return 0;
}

############################################################################
# function:	RelShipFiles
# description:	release ship file query handler; prints out all ship files
#		belonging to a given release
# input:	release name contained in $NAME
# output:	release ship files along w/ each files LPP name(s)
# remarks:	
############################################################################
sub	RelShipFiles {
	# opening encode, decode and lppnames associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);
	dbmopen(%rels_built,"$BQPATH/RELS_BUILT",0750);

	local($lppstr);
	local(%lppsfound);
	local($fileid);
	local($rowner,$filename);

	if (! defined $rels_built{$NAME}) {
		$QUERYSTATUS = "Unknown release: $NAME";
		return -1;
	}

	if ($NEWMODE) {
		dbmopen(%lppnames,"$BQPATH/LPP/NLPPNAMES",0750);
	} else {
		dbmopen(%lppnames,"$BQPATH/LPP/LPPNAMES",0750);
	}

	# opening release ship file for specified release
	if (!open(RELSHIP,"$BQPATH/$NAME/SHIP")) {
		$QUERYSTATUS="No ship files known for release: $NAME\n";
		return -1;
	} else {
		# reading each release ship file
		while ($file = <RELSHIP>) {
		    chop($file);
		    $fileid = $encode{$file};

		    chop($lppstr = $lppnames{$fileid});
		    undef %lppsfound;
		    if ($lppstr ne "") {
		        # getting list of LPPs to which the ship file belongs
			&ParseLPPstr(*lppstr,*lppsfound);
		    } else {
			$lppstr = $NOLPPMSG;
		    }

		    # filtering out any files that are not part of 
		    # the LPP specified by $LPPFILTER
	      	    if (($LPPFILTER eq "") || 
			(defined $lppsfound{$LPPFILTER})) {
			$rowner=&ParseRelOwner($fileid);
			if (($RELFILTER eq "") || ($rowner eq "$RELFILTER")) {
			        $filename=&ParseFileName($fileid);
				if (defined $bldenv{$fileid}) {
				    &PrintLine($filename," <$rowner: $lppstr>");
				} else {
				    &PrintLine($filename," ($rowner: $lppstr)");
				}
			}
	      	    }
		}
	}
	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%lppnames);
	dbmclose(%bldenv);
	dbmclose(%rels_built);

	return 0;
}

############################################################################
# function:	LppSrcFiles
# description:	LPP source file query handler; prints out all source files
#		belonging to a given LPP
# input:	LPP name contained in $NAME
# output:	LPP source files
# remarks:	
############################################################################
sub	LppSrcFiles {
	# opening encode, decode and release owner, and build environment 
	# associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);
	dbmopen(%lpplist,"$BQPATH/LPP/LPPLIST",0750);

	local($rowner,$filename);

	if (! defined $lpplist{$NAME}) {
		$QUERYSTATUS = "Unknown LPP: $NAME";
		return -1;
	}

	# if new mode is on then get LPP source files from the .NSRC file;
	# otherwise get them from the .SRC file
	if ($NEWMODE) {
		$LPPSRC_FILE="$BQPATH/LPP/$NAME.NSRC";
	} else {
		$LPPSRC_FILE="$BQPATH/LPP/$NAME.SRC";
	}

	# opening the LPP source file
	if (!open(LPPSRC,$LPPSRC_FILE)) {
		$QUERYSTATUS="No source files known for LPP: $NAME\n";
		return -1;
	} else {
		# reading each source file for the LPP
		while ($file = <LPPSRC>) {
			chop($file);

			# getting of releases for which the current 
			# file is a source file
			$rowner=&ParseRelOwner($encode{$file});
			$filename=&ParseFileName($encode{$file});

			# filtering out any file that is not part of the 
			# release specified by $RELFILTER
			if (($RELFILTER eq "") || ($rowner eq $RELFILTER)) {
			    if (defined $bldenv{$encode{$file}}) {
				&PrintLine($filename," <$rowner>");
			    } else {
				&PrintLine($filename," ($rowner)");
			    }
			}
		}
	}
	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);
	dbmclose(%lpplist);

	return 0;
}

############################################################################
# function:	LppShipFiles
# description:	LPP ship file query handler; prints out LPP ship files for
#		a given LPP
# input:	LPP name contained in $NAME
# output:	LPP ship files
# remarks:	
############################################################################
sub	LppShipFiles {
	# opening encode, decode and release owner, and build environment 
	# associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);
	dbmopen(%lpplist,"$BQPATH/LPP/LPPLIST",0750);

	local(%lppsfound);
	local($lppstr);
	local($fileid);
	local($rowner,$filename);

	if (! defined $lpplist{$NAME}) {
		$QUERYSTATUS = "Unknown LPP: $NAME";
		return -1;
	}

	if ($NEWMODE) {
		dbmopen(%lppnames,"$BQPATH/LPP/NLPPNAMES",0750);
	} else {
		dbmopen(%lppnames,"$BQPATH/LPP/LPPNAMES",0750);
	}

	# if new mode is set then use the .NSHIP file, otherwise 
	# use the .SHIP file
	if ($NEWMODE) {
		$LPPSHIP_FILE="$BQPATH/LPP/$NAME.NSHIP";
	} else {
		$LPPSHIP_FILE="$BQPATH/LPP/$NAME.SHIP";
	}

	# opening LPP ship file
	if (open(LPPSHIP,$LPPSHIP_FILE)) {
		# reading each LPP ship file
		while (chop($file = <LPPSHIP>)) {
			$fileid=$encode{$file};
			# getting list of LPPs to which the ship file belongs
			chop($lppstr = $lppnames{$fileid});
			undef %lppsfound;
			&ParseLPPstr(*lppstr,*lppsfound);
	      		if (($LPPFILTER eq "") || 
			    (defined $lppsfound{$LPPFILTER})) {
			    $rowner=&ParseRelOwner($fileid);
			    if (($RELFILTER eq "") || 
				($rowner eq "$RELFILTER")) {
				$filename=&ParseFileName($fileid);
				if (defined $bldenv{$fileid}) {
				   &PrintLine($filename," <$rowner: $lppstr>");
				} else {
				   &PrintLine($filename," ($rowner: $lppstr)");
				}
			    }
			}
		}
	} else {
		$QUERYSTATUS="No ship files known for LPP: $NAME\n";
		return -1;
	}

	dbmclose(%encode);
	dbmclose(%decode);
	dbmclose(%bldenv);

	return 0;
}

############################################################################
# function:	BldenvFiles
# description:	Build environment file query handler; prints out all build
#		environment files across all releases
# input:	none
# output:	build environment files
# remarks:	
############################################################################
sub	BldenvFiles {
	# opening encode, decode and bldenv associative array DBM files
	dbmopen(%encode,"$BQPATH/ENCODE",0750);
	dbmopen(%decode,"$BQPATH/DECODE",0750);
	dbmopen(%bldenv,"$BQPATH/BLDENV",0750);

	# reading BLDENV associative array dbm file
	foreach $id (keys %bldenv) {
	    if (($RELFILTER eq "") || ($bldenv{$id} eq $RELFILTER)) {
		&PrintLine($decode{$id}," <$bldenv{$id}>");
	    }
	}

	dbmclose(%bldenv);
	dbmclose(%encode);
	dbmclose(%decode);

	return 0;
}

############################################################################
# function:	print_usage
# description:	prints out usage statement
# input:	none
# output:	usage statement
# remarks:	
############################################################################
sub	print_usage {
  print STDERR "Usage: bldquery [-D <search_dir>] [-n] [-l <LPP_filter>]\n";
  print STDERR "       [-r <release_filter>] [-i <file_name>] [-f <string>]\n";
  print STDERR "       [-F <string>] [-v <mode>] [-t <db_path>] [-q <query_value>]\n"; 
  print STDERR "       [<file_name> | <release_name> | <LPP_name> ]\n";
  print STDERR "  where:\n";
  print STDERR "    <query_value> is one of the following:\n";
  print STDERR "       \"t\", \"tts\", \"ts\", \"tb\", \"d\", \"dt\", \"db\",\n";
  print STDERR "       \"r\", \"rdt\", \"rs\", \"l\", \"ldt\", \"ls\" or \"b\"\n";
  print STDERR "    and the last argument on the command line is:\n";
  print STDERR "       * not required when <query_name> is \"r\", \"l\" or \"b\" (or\n";
  print STDERR "         not supplied)\n";
  print STDERR "       * <file_name> when <query_name> is \"t\", \"tts\", \"ts\", \"tb\",\n";
  print STDERR "         \"d\", \"dt\" or \"db\"\n";
  print STDERR "       * <release_name> when <query_name> is \"rdt\" or \"rs\"\n";
  print STDERR "       * <LPP_name> when <query_name> or \"ldt\" or \"ls\"\n";
}

################################ M A I N ################################

push(@INC,split(/:/,$ENV{'PATH'}));          # Including path 
do 'bldperlconst';
do 'bldperlfunc';
do 'bldquerylib';

$SIG{'QUIT'} = 'cleanup';
$SIG{'INT'} = 'cleanup';
$SIG{'HUP'} = 'cleanup';
$SIG{'TERM'} = 'cleanup';

# messages
$NOLPPMSG="<NOT SHIPPED WITH ANY LPP>";
$NORELMSG="<NOT OWNED BY ANY RELEASE>";
@VERBOSE_STR = ("SUMMARY ONLY", 
		"FILE LISTING ONLY", 
		"SUMMARY & FILE LISTING");

%ON_OFF = (0, "OFF",
	   1, "ON");

$MENU_MODE = $FALSE;

# initialize command line option string variables
$SEARCHDIR = "";
$NEWMODE = $FALSE;
$NAME = "";
$LPPFILTER = "";
$RELFILTER = "";
#$DEPTH = 1;
#$TRACE = $FALSE;
$EXCLUDESTR = "";
$INCLUDESTR = "";
$VERBOSE = 2;
$OUTFILE = "";
$INFILE = "";

# initialize associative array that contains information for each query
# type; each key of the array is the querycode for the query that the user
# is prompted for; 
%query_info = (
	"t",	"01|GetFileName|TargFiles|CreateFileSummary|file|$TRUE|target files",
	"tts",	"02|GetFileName|TargTermFiles|CreateFileSummary|file|$TRUE|target terminal and ship files",
	"ts",	"03|GetFileName|TargShipFiles|CreateFileSummary|file|$TRUE|target ship files",
	"tb",	"04|GetFileName|TargBldenvFiles|CreateFileSummary|file|$TRUE|target bldenv files\n",
	"d",	"05|GetFileName|DepFiles|CreateFileSummary|file|$TRUE|dependent files",
	"dt",	"06|GetFileName|DepTermFiles|CreateFileSummary|file|$TRUE|dependent terminal (source) files",
	"db",	"07|GetFileName|DepBldenvFiles|CreateFileSummary|file|TRUE|dependent bldenv files\n",
	"r",	"08||RelNames|CreateRelSummary||$FALSE|release names",
	"rdt",	"09|GetReleaseName|RelSrcFiles|CreateFileSummary|release|TRUE|dependent terminal (source) files used by a release",
	"rs",	"10|GetReleaseName|RelShipFiles|CreateFileSummary|release|$TRUE|ship files built in a release\n",
	"l",	"11||LppNames|CreateLPPSummary||$FALSE|LPP names",
	"ldt",	"12|GetLPPName|LppSrcFiles|CreateFileSummary|LPP|$TRUE|dependent terminal (source) files used by an LPP",
	"ls",	"13|GetLPPName|LppShipFiles|CreateFileSummary|LPP|$TRUE|ship files shipped with an LPP\n",
	"b",	"14||BldenvFiles|CreateFileSummary||$TRUE|bldenv files\n",
	);

# set up directory for temporary files
chop($TEMP_DIR=`bldtmppath`);

# initialize temporary file pathnames
$ALL_QUERY_FILE = "$TEMP_DIR/bldquery1.$$";
$QUERY_FILE = "$TEMP_DIR/bldquery2.$$";
$SUM_FILE = "$TEMP_DIR/bldquery3.$$";

$TARGLIST_READ = $FALSE;
$DEPENDLIST_READ = $FALSE;

# get command line options
while (&getopts(":D:nl:r:i:f:F:v:q:t:",*opt)) {
	if ($opt eq "D") {
		$SEARCHDIR = $OPTARG;
		if ($SEARCHDIR !~ /^\.\//) {
			print STDERR "Search directory must begin with \"./\"";
			&cleanup;
		}
	} elsif ($opt eq "n") {
		$NEWMODE = $TRUE;
	} elsif ($opt eq "l") {
		$LPPFILTER = $OPTARG;
	} elsif ($opt eq "r") {
		$RELFILTER = $OPTARG;
#	} elsif ($opt eq "d") {
#		$DEPTH = $OPTARG;
#	} elsif ($opt eq "k") {
#		$TRACE = $TRUE;
	} elsif ($opt eq "i") {
		$INFILE = $OPTARG;
	} elsif ($opt eq "f") {
		$INCLUDESTR = $OPTARG;
	} elsif ($opt eq "F") {
		$EXCLUDESTR = $OPTARG;
	} elsif ($opt eq "v") {
		$VERBOSE = $OPTARG;
	} elsif ($opt eq "q") {
		$QUERYCODE = $OPTARG;
	} elsif ($opt eq "t") {
		$BQTOP = $OPTARG;
	} elsif ($opt eq "?") {
		&print_usage;
		&cleanup;
	}
}

# initialize bldquery database path
$BQPATH="$BQTOP/HISTORY/bldquery";

# initialize release path global path variable
$RELLIST_FILE="$ENV{'BLDENV'}/usr/bin/RELEASE_LIST";

# if the last argument in the command line does not have a "-" in 
# front of it then assume that it is the file/release/LPP name to 
# be used in the query
if (($ARGV[$OPTIND-1] ne "") && ($QUERYCODE ne "")) {
	if ($OPTARG eq "") {
		$NAME = $ARGV[$OPTIND-1];
	}
}

# if a query code was given on the command line then check to make 
# sure it is a valid one
if ($QUERYCODE ne "") {
	if (! defined $query_info{$QUERYCODE}) {
		print STDERR "bldquery: Invalid query code: $QUERYCODE\n";
		&cleanup;
	} else {
		($ord,$getfunc,$queryfunc,$summaryfunc,$nametype,
		 $filequery,$description) = split(/\|/,$query_info{$QUERYCODE});

		# if the query expects a filename, release name or LPP name
		# and none was supplied and no input file was specified then
		# we have a syntax error: print usage statement and exit
		if (($nametype ne "") && ($NAME eq "") && ($INFILE eq "")) {
			&print_usage;
			&cleanup;
		}
	}
}

# exit with error msg if we are unable to read bldquery databases
if (! -d $BQPATH) {
	print STDERR "Could not read bldquery database directory: $BQPATH\n";
	&cleanup;
}

# read in targetlist and dependent list arrays
print STDERR "Initializing...";
&read_targetlist;
&read_dependentlist;
print STDERR "done.\n";

# if a query code was given then process the query and exit
if ($QUERYCODE ne "") {
	&process_queries($QUERYCODE);
	system("cat $ALL_QUERY_FILE");
	system("rm -f $ALL_QUERY_FILE $QUERY_FILE $SUM_FILE");
	&cleanup;
}

# if no query code was given then automatically go into menu mode
$done = $FALSE;
$MENU_MODE = $TRUE;
while (! $done) {
	# print menu
	system("tput clear > /dev/tty");
	print "BLDQUERY MAIN MENU\n\n";
	print "        1) Query\n";
	print "        2) Set search directory ($SEARCHDIR)\n";
	print "        3) Set LPP filter ($LPPFILTER)\n";
	print "        4) Set release filter ($RELFILTER)\n";
	print "        5) Set exclusion filter ($EXCLUDESTR)\n";
	print "        6) Set inclusion filter ($INCLUDESTR)\n";
	print "        7) Set verbose mode ($VERBOSE_STR[$VERBOSE])\n";
	print "        8) Set output file ($OUTFILE)\n";
	print "        9) Set input file ($INFILE)\n";
	print "       10) Display input file\n";
	print "       11) Toggle new-mode ($ON_OFF{$NEWMODE})\n";
#	print "       12) Set depth ($DEPTH)\n";
#	print "       13) Toggle trace mode ($ON_OFF{$TRACE})\n";
	print "       12) quit\n";

	print "\nchoice: ";
	# all user to enter choice
	chop($choice = <STDIN>);
	$choice =~ tr/[A-Z]/[a-z]/;

	# process choice
	if ($choice == 1) {
		# if query selected then print out all query options
		system("tput clear > /dev/tty");
		print "BLDQUERY QUERY TYPE MENU:\n\n";

		# storing each array element's value from the associative
		# array %query_info in a regular array called @query; the
		# key for each element of the %query_info array is appended 
		# to the end of its corresponding value before being added
		# to the @query array
		$query_index = 0;
		while (($querycode,$info) = each %query_info) {
			$query[$query_index++] = "$info|$querycode";
		}
		# sorting @query array by the first field of each element's
		# value then printing the query code menu
		for $q (sort(@query)) {
			($ord,$getfunc,$queryfunc,$summaryfunc,$nametype,
			 $filequery,$description,$qc) = split(/\|/,$q);
			print "\t$qc\t$description\n";
		}
		print "choice: ";

		# get query option
		chop($QUERYCODE = <STDIN>);
		if (defined $query_info{$QUERYCODE}) {

			# create temporary file path name
			$ALL_QUERY_FILE = "$TEMP_DIR/bldquery1.$$";

			if ($OUTFILE ne "") {
				$ALL_QUERY_FILE = $OUTFILE;
			}

			# process query
			&process_queries($QUERYCODE);

			system("tput clear > /dev/tty");
			system("pg $ALL_QUERY_FILE");

			if ($OUTFILE eq "") {
				system("rm -f $ALL_QUERY_FILE");
			}
		} else {
			print "Invalid query code\n";
			&Pause;
		}
	} elsif ($choice == 2) {
		print "new search directory: ";
		chop($temp_searchdir = <STDIN>);
		if (($temp_searchdir =~ /^\.\//) || ($temp_searchdir eq "")) {
			$SEARCHDIR = $temp_searchdir;
		} else {
			print STDERR "Search directory must begin with \"./\"";
			&Pause;
		}
	} elsif ($choice == 3) {
		print "new LPP name: ";
		chop($LPPFILTER = <STDIN>);
	} elsif ($choice == 4) {
		print "new release name: ";
		chop($RELFILTER = <STDIN>);
	} elsif ($choice == 5) {
		print "new exclusion string: ";
		chop($EXCLUDESTR = <STDIN>);
	} elsif ($choice == 6) {
		print "new inclusion string: ";
		chop($INCLUDESTR = <STDIN>);
	} elsif ($choice == 7) {
		print "\nVerbose mode options:\n\n";
		print "        0) Summary only\n";
		print "        1) Files only\n";
		print "        2) Files and Summary\n\n";
		print "choice: ";
		chop($temp_verbose = <STDIN>);
		if (($temp_verbose ge "0") && 
		    ($temp_verbose le "$#VERBOSE_STR")) {
			$VERBOSE = $temp_verbose;
		} else {
			print STDERR "Invalid choice.\n";
			&Pause;
		}
	} elsif ($choice == 8) {
		print "new output file name: ";
		chop($OUTFILE = <STDIN>);
	} elsif ($choice == 9) {
		print "new input file name: ";
		chop($INFILE = <STDIN>);
	} elsif ($choice == 10) {
		if ($INFILE ne "") {
			system("tput clear > /dev/tty");
			system("pg $INFILE");
		}
	} elsif ($choice == 11) {
		$NEWMODE = !$NEWMODE;
#	} elsif ($choice == 12) {
#		print "new depth: ";
#		chop($DEPTH = <STDIN>);
#	} elsif ($choice == 13) {
#		$TRACEMODE = !$TRACEMODE;
	} elsif (($choice == 12) || ($choice eq "x") || ($choice eq "exit") || 
		 ($choice eq "q") || ($choice eq "quit")) {
		$done = TRUE;
	} else {
		print STDERR "Invalid choice.\n";
		&Pause;
	}

	# re-initialize $NAME to null string
	$NAME="";
	$RELEASE="";
}

