#! /usr/bin/perl
# @(#)61	1.8  src/bldenv/bldtools/rename/bldquerydb.pl, bldtools, bos412, GOLDA411a 11/20/92 12:36:51
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldquerydb
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
# NAME: bldquerydb
#
# FUNCTION: 
#
# INPUT:
#
# OUTPUT: Standard output
#
# EXECUTION ENVIRONMENT: Build process environment
#

push(@INC,split(/:/,$ENV{'PATH'}));          # Including path 
do 'bldperlconst';
do 'bldquerylib';

############################################################################
# function:	error
# description:	exits with and error
# input:	none
# output:	error message
# remarks:
############################################################################
sub	error {
	print STDERR "bldquerydb failure.\n";
	exit 1;
}

sub	EncodeFile {
	local($name) = @_;
	local($id);

	if (! defined $encode{$name}) {
		$id = ++$uniqueid;
		$encode{$name} = $id;
		$decode{$id} = $name;
	} else {
		$id = $encode{$name};
	}

	return($id);
}

##############################################################################
# function:	ProcessMakelists
# description:	Process each release's makelist file.  This file contains the 
#		target/dependency information for the files built during this 
#		build.  Each line in the makelist file has the following form:
#
#		target dependent [NEW]
#
#		where target is the target file of the dependency and dependent
#		is the dependent file in the dependency.  If the "NEW" string
#		is present then this indicates that the target was built
#		during this build.
# input:	release makelist files
# output:	$TOP/HISTORY/bldquery/RELS_BUILT dbm file - associative array 
#		dbm file whose keys are all the releases for which a makelist 
#		file was found.
#
#		$TOP/HISTORY/bldquery/BLDENV dbm file - associative array dbm
#		file whose keys are build environment file ids and whose values
#		are the names of the releases that put them in the build 
#		environment.
#
#		$TOP/HISTORY/bldquery/ENCODE dbm file - associative array dbm
#		file whose keys are full file names with the release name of 
#		the release that owns the file sometimes appended to the end 
#		(for example, "./inc/stdio.h|bos320").  The only time that the
#		release name would not be appended to the end would be if the
#		file were a build environment file.  In this case the release
#		name for the file would be stored in the BLDENV dbm file.  The 
#		elements of the ENCODE file are integer file id codes that 
#		uniquely identify the file.
#
#		$TOP/HISTORY/bldquery/DECODE dbm file - associative array dbm
#		file whose keys are integer file id codes and whose elements 
#		are the unique file names that correspond to each key, with the
#		release name of the release that owns the file sometimes 
#		appended to the end (see discussion above in the ENCODE dbm 
#		file description).  This associative array dbm file is the 
#		exact inverse of the ENCODE associative array file described 
#		above.
#
#		targetlist[] array - each index of the array is a a dependent 
#		file id (see ENCODE and DECODE dbm file descriptions above); 
#		each element is a space separated list of file ids each of 
#		which is a target of the file represented by the index file id.
#		For example,
#
#		$targetlist[14] = "21 31 84 105 29 73"
#
#		Here each of the files represented by the file ids 21, 31, 84, 
#		105, 29, 73 are all target files of the file represented by
#		file id 14.
#
#		Ntargetlist[] array - same as the targetlist array except only
#		those targets for which the entry in the makelist contains the
#		"NEW" field.
#
#		dependentlist[] array - same as the targetlist array except the
#		indeces are target file ids and the elements are lists of
#		file ids that represent dependent files of the file represented
#		by the index file id.
#
#		Ndependentlist[] array - same as the dependentlist array except 
#		only those targets for which the entry in the makelist contains 
#		the "NEW" field.
#
#		srcrellist{} associative array - associative array whose
#		indeces are source file ids and whose elements are space 
#		separated lists of releases.  The source file represented by
#		the index file id is used by each release in the list.  For
#		example,
#
#		$srcrellist{29} = "bos320 commo320 des320"
#
#		Here the source file represented by file id 29 is said to 
#		be used in the building of the three releases, bos320, commo320 #		and des320
# remarks:
##############################################################################
sub	ProcessMakelists {
	$RELLIST_FILE="$ENV{'BLDENV'}/usr/bin/RELEASE_LIST";
	# opening release list file
	if (!open(RELLIST,$RELLIST_FILE)) {
		print STDERR "couldn't open RELEASE_LIST file\n";
		&error;
	}

	# opening build environment file for current release
	dbmopen(%bldenv,"$BQDBLOCAL/BLDENV",0750);

	# get each release name found in release list file
	while ($crnt_rel = <RELLIST>) {
		chop $crnt_rel;
	
		# construct release path name for bldquery database directory 
		$RELEASE_DIR="$BQDBLOCAL/$crnt_rel";

		# constructing bldquery database makelist pathname 
		$BQDB_MAKELIST_FILE="$RELEASE_DIR/makelist";

		# open makelist file for current release
		if (!open(MAKELIST,"$BQDB_MAKELIST_FILE")) {
			next;
		}

		# define rels_built for current release to show that it 
		# has been built
		$rels_built{$crnt_rel} = $DEFINED;

		# walking thru current release's makelist file
		while (<MAKELIST>) {
			chop $_;
			# each line contains a target file name, a dependent 
			# file name and possibly a "new" flag
			($target,$dependent,$newval) = split(/ /,$_);

			# remove the "!!" from the front of the target and 
			# dependent file names and replace with a "."
			$target =~ s/!!/./g;
			$dependent =~ s/!!/./g;

			# skip this line if the target and dependent files 
			# are the same
			if ($target eq $dependent) {
				next;
			}

			# checking to see if target file is a build environment
			# file; if so then it is possible that it has already
			# been used one or more times previously as a non-
			# bldenv file belonging to a specific release(s); if
			# this is the case then it means that a file id for the
			# file and specific to each release has been created;
			# if so then each release specific version of the file
			# id must be deleted from the %encode, %decode and 
			# %crnt_relids associative arrays; all instances of the
			# file id must also be purged from the targetlist and
			# dependentlist arrays as well
			$bldenv_file = $FALSE;
			if ($target =~ /\/bldenv/) {
			    # target file is a bldenv file
			    $bldenv_file = $TRUE;

			    # substitute "." for "/bldenv/@@"; now target has
			    # the same form as a normal non-bldenv file; how-
			    # ever no "|<release_name>" string will be appended
			    # to the end of it before encoding it
			    $target =~ s/\/bldenv\/\@\@/./g;
			} else {
			    # file found that did not have "/bldenv/@@" prefix;
			    # test to see if there is a file id encoded for 
			    # this file without any release owner appended 
			    # to the end; if so then it must be a BLDENV 
			    # file; if not then assume that it is a non-bldenv
			    # file and append the current release name to the
			    # end before encoding it
			    if (! defined $encode{$target}) {
			    	$target .= "|$crnt_rel";
			    }
			}
			$targetid = &EncodeFile($target);
			$crnt_relids{$targetid} = $DEFINED;
			
			if ($bldenv_file) {
			    $bldenv{$targetid} = $crnt_rel;

			    # searching thru all the releases whose makelists
			    # have been processed so far, looking for occur-
			    # rences of this bldenv file which have been 
			    # treated as a normal file previously and purging
			    # them from the relevant arrays
			    for $rel (keys %rels_built) {
			      # was a file id encoded for this file and
			      # this release
			      if (defined $encode{"$target|$crnt_rel"}) {
			        $old_tid = $encode{"$target|$crnt_rel"};

				# purging file id from associative arrays
			        delete $crnt_relids{$old_tid};
			        delete $decode{$old_tid};
			        delete $encode{"$target|$crnt_rel"};

			        # purging file id from targetlist and 
				# dependentlist arrays
				@targs = split(/ /,$targetlist[$old_tid]);
				@deps = split(/ /,$dependentlist[$old_tid]);

				# purging old file id from dependentlist and
				# targetlist arrays and transferring target & 
				# dependent file ids associated w/ it to the 
				# newly created bldenv file id
			        $dependentlist[$targetid] .=
					$dependentlist[$old_tid];
			        $dependentlist[$old_tid] = "";
			        $targetlist[$targetid] .=
					$targetlist[$old_tid];
			        $targetlist[$old_tid] = "";
			        foreach $id (@targs) {
				  $dependentlist[$id] =~ 
					s/(^| )$old_tid /$1$targetid /g;
			        }
			        foreach $id (@deps) {
				  $targetlist[$id] =~ 
					s/(^| )$old_tid /$1$targetid /g;
			        }

			        # purging file id from Ntargetlist and 
				# Ndependentlist arrays
				@targs = split(/ /,$Ntargetlist[$old_tid]);
				@deps = split(/ /,$Ndependentlist[$old_tid]);

				# purging old file id from Ndependentlist and
				# Ntargetlist arrays and transferring target & 
				# dependent file ids associated w/ it to the 
				# newly created bldenv file id
			        $Ndependentlist[$targetid] .=
					$Ndependentlist[$old_tid];
			        $Ndependentlist[$old_tid] = "";
			        $Ntargetlist[$targetid] .=
					$Ntargetlist[$old_tid];
			        $Ntargetlist[$old_tid] = "";
				foreach $id (@targs) {
				  $Ndependentlist[$id] =~ 
					s/(^| )$old_tid /$1$targetid /g;
			        }
			        foreach $id (@deps) {
			          $Ntargetlist[$id] =~ 
					s/(^| )$old_tid /$1$targetid /g;
			        }
			      }
			    }
			}

			# replace "/bldenv/@@" string w/ "." in dependent name
			$dependent =~ s/\/bldenv\/\@\@/./g;

			# encode the dependent file name
			if (! defined $encode{$dependent}) {
				$dependent .= "|$crnt_rel";
			}
			$dependentid = &EncodeFile($dependent);
			$crnt_relids{$dependentid} = $DEFINED;

			if ($dependentid ne $targetid) {
			  # append the target id to the list of target 
			  # ids that are associated with the current 
			  # dependent file
			  $targetlist[$dependentid] !~ /(^| )$targetid / &&
			      $targetlist[$dependentid] .= "$targetid ";

			  # append the dependent id to the list of 
			  # dependent ids that are associated with the 
			  # current target file
			  $dependentlist[$targetid] !~ /(^| )$dependentid / &&
			      $dependentlist[$targetid] .= "$dependentid ";

			  # if newval flag is on then the current target 
			  # was built in this build - append the target 
			  # and dependent file ids to the Ntargetlist and
			  # Ndependlist arrays
			  if ($newval eq "NEW") {
			   $Ntargetlist[$dependentid] !~ /(^| )$targetid / &&
				$Ntargetlist[$dependentid] .= "$targetid ";
			   $Ndependentlist[$targetid] !~ /(^| )$dependentid / &&
				$Ndependentlist[$targetid] .= "$dependentid ";
			  }
			}
		}

		# walking thru each file id used in the build of this release
		foreach $id (keys %crnt_relids) {
			# if there is no list of dependents for this ID then
			# it must be a source file(i.e. has no dependent files)
			if ($dependentlist[$id] =~ /^ ?$/) {
				# concatenate to list of releases for which id 
				# is a source file
				$srcrellist{$id} .= "$crnt_rel ";
				# if the id is also a new target file, then
				# append the current release to list of new
				# source file releases
				if ($Ntargetlist[$id] !~ /^ ?$/) {
					$Nsrcrellist{$id} .= "$crnt_rel ";
				}
			}
		}
		# clear crnt_relids array for next release to be processed
		(%crnt_relids) = ();
	}

	dbmclose(%bldenv);
	close(RELLIST);

	# open releases built associative array DBM file
	&mapdbm(*rels_built,"$BQDBLOCAL/RELS_BUILT");
}

##############################################################################
# function:	CreateRelSrcFiles
# description:	Create files that contain the names of all the source files
#		that make up the release.
# input:	srcrellist associative array
# output:	$TOP/HISTORY/bldquery/$release/SRC file - file containing the
#		names of all the source files that make up a given release. 
#		Each release has its own SRC file.
#
# 		$TOP/HISTORY/bldquery/$release/NSRC file - same as the SRC file
#		except only for source files that were changed during this
#		build.  Each release has its own NSRC file.
#
#		relshiplist{} associative array - associative array each of
#		whose indeces is a string containing the name of a release
#		followed by a ship file id that belongs to the release.  The
#		elements of the array are all simply set to $DEFINED.
# remarks:
##############################################################################
sub	CreateRelSrcFiles {
	local(@targshiplist);

	# creating release source list files
	# walking thru list of source files found in the build
	foreach $id (keys %srcrellist) {
		# get every ship file that the current source file is part of
		&getshipfileids($id,*targshiplist,*targetlist);

		# put in rlist array all releases for which current file 
		# is a source file
		(@rlist) = split(/ /,$srcrellist{$id});

		# walking thru each release for which current file is a
		# source file
		for ($r = 0; $r <= $#rlist; $r++) {
			# construct source file name for current release
			$RELSRC_FILE="$BQDBLOCAL/$rlist[$r]/SRC";

			# appending source file name to the current release's 
			# source file
			open(RELSRC,">> $RELSRC_FILE");
			print RELSRC "$decode{$id}\n";
			close(RELSRC);

			# for each target ship file id of the given source file
			# create an entry in the relshiplist associative 
			# array; each entry in the array contains the current 
			# release and the current ship file id; these entries 
			# will be split up later in order to create the file 
			# containing all the ship files for each release
			for ($i = 0; $i <= $#targshiplist; $i++) {
			    $relshiplist{"$rlist[$r] $targshiplist[$i]"} =
				$DEFINED;
			}
			# clear targshiplist for the next source file
			(@targshiplist) = ();
		}
	}

	# creating release "new" source list files
	# walking thru list of "new" source files found in build
	foreach $id (keys %Nsrcrellist) {
		# get every ship file that the current source file is part of
		&getshipfileids($id,*targshiplist,*Ntargetlist);
	
		# put in rlist array all releases for which current file 
		# is a source file
		(@rlist) = split(/ /,$Nsrcrellist{$id});

		# walking thru each release for which current file is a 
		# source file
		for ($r = 0; $r <= $#rlist; $r++) {
			# construct source file name for current release
			$NRELSRC_FILE="$BQDBLOCAL/$rlist[$r]/NSRC";

			# appending source file name to the current release's 
			# source file
			open(NRELSRC,">> $NRELSRC_FILE");
			print NRELSRC "$decode{$id}\n";
			close(NRELSRC);

			# for each target ship file id of the given source file
			# create an entry in the Nrelshiplist associative 
			# array; each entry in the array contains the current 
			# release and the current ship file id; these entries 
			# will be split up later in order to create the file 
			# containing all the ship files for each release
			for ($i = 0; $i <= $#targshiplist; $i++) {
			    $Nrelshiplist{"$rlist[$r] $targshiplist[$i]"} =
				$DEFINED;
			}
			# clearing targshiplist for next source file id
			(@targshiplist) = ();
		}
	}
}

##############################################################################
# function:	CreateRelShipFiles
# description:	Create files that contain the names of all the ship files
#		that make up the release.
# input:	relshiplist associative array
# output:	$TOP/HISTORY/bldquery/$release/SHIP file - file containing the
#		names of all the ship files that make up a given release. 
#		Each release has its own SHIP file.
#
# 		$TOP/HISTORY/bldquery/$release/NSHIP file - same as the SHIP 
#		file except only for ship files that were changed during this
#		build.  Each release has its own NSHIP file.
# remarks:
##############################################################################
sub	CreateRelShipFiles {
	# creating ship list file for each release

	# walking thru relshiplist; each entry contains a release name and 
	# a ship file id; the name of the file that corresponds to the ship 
	# file id
	foreach $relshipid (keys %relshiplist) {
		# split int release and ship file id
		($rel,$id) = split(/ /,$relshipid);
		# construct ship file path
		$RELSHIP_FILE="$BQDBLOCAL/$rel/SHIP";
		# open ship list file for release
		if (!open(RELSHIP,">> $RELSHIP_FILE")) {
			print STDERR "couldn't open $RELSHIP_FILE\n";
		}
		# append current ship file name to ship list file for release
		print RELSHIP "$decode{$id}\n";
		close(RELSHIP);
	}
	undef(%relshiplist);

	# creating "new" ship list files for each release
	foreach $relshipid (keys %Nrelshiplist) {
		# split int release and ship file id
		($rel,$id) = split(/ /,$relshipid);
		# construct ship file path
		$NRELSHIP_FILE="$BQDBLOCAL/$rel/NSHIP";
		# open ship list file for release
		if (!open(NRELSHIP,">> $NRELSHIP_FILE")) {
			print STDERR "couldn't open $NRELSHIP_FILE\n";
		}
		# append current ship file name to ship list file for release
		print NRELSHIP "$decode{$id}\n";
		close(NRELSHIP);
	}
	undef(%Nrelshiplist);
}

##############################################################################
# function:	CreateTargDepFiles
# description:	Create files that contain the names of all the ship files
#		that make up the release.
# input:	targlist[] array
# 		Ntarglist[] array
# 		dependentlist[] array
# 		Ndependentlist[] array
# output:	$TOP/HISTORY/bldquery/TARGLIST file - file containing the
#		dependent file ids along all of their corresponding target file 
#		ids.  Each line in the file has the form:
#
#		dependentid|targetid targetid targetid targetid
#
#		for example,
#
#		15|20 89 30 12 71
#
#		Here the files corresponding to the file ids 20, 89, 30, 12 and
#		71 are target files of the file represented by the file id 15.
#
# 		$TOP/HISTORY/bldquery/NTARGLIST file - same as the TARGLIST
#		file except only for files that were built during this build.
#
# 		$TOP/HISTORY/bldquery/DEPENDLIST file - same as TARGLIST file
#		except the first id before the "|" character is a target file 
#		id and each id after it is a dependent id for a file on which
#		the target id depends.
#
# 		$TOP/HISTORY/bldquery/NDEPENDLIST file - same as the DEPENDLIST
#		file except only for files that were built during this build.
#
# remarks:
##############################################################################
sub	CreateTargDepFiles {
	# constructing target and dependency file pathnames 
	$TARGLIST_FILE="$BQDBLOCAL/TARGLIST";
	$NTARGLIST_FILE="$BQDBLOCAL/NTARGLIST";
	$DEPENDLIST_FILE="$BQDBLOCAL/DEPENDLIST";
	$NDEPENDLIST_FILE="$BQDBLOCAL/NDEPENDLIST";

	# opening target and dependency files 
	if (!open(TARGLIST,"> $TARGLIST_FILE")) {
		print STDERR "couldn't open TARGLIST\n";
	}
	if (!open(NTARGLIST,"> $NTARGLIST_FILE")) {
		print STDERR "couldn't open NTARGLIST\n";
	}
	if (!open(DEPENDLIST,"> $DEPENDLIST_FILE")) {
		print STDERR "couldn't open DEPENDLIST\n";
	}
	if (!open(NDEPENDLIST,"> $NDEPENDLIST_FILE")) {
		print STDERR "couldn't open NDEPENDLIST\n";
	}

	# creating target and dependent tree files
	for ($id = 1; $id <= $uniqueid; $id++) {
		# test to make sure that element does not equal "" or " "
		if ($targetlist[$id] !~ /^ ?$/) {
			print TARGLIST "$id|$targetlist[$id]\n";
		}
		if ($Ntargetlist[$id] !~ /^ ?$/) {
			print NTARGLIST "$id|$Ntargetlist[$id]\n";
		}
		if ($dependentlist[$id] !~ /^ ?$/) {
			print DEPENDLIST "$id|$dependentlist[$id]\n";
		}
		if ($Ndependentlist[$id] !~ /^ ?$/) {
			print NDEPENDLIST "$id|$Ndependentlist[$id]\n";
		}
	}

	close(TARGLIST);
	close(NTARGLIST);
	close(DEPENDLIST);
	close(NDEPENDLIST);
}

##############################################################################
# function:	ProcessXreflist
# description:	Process xreflist file.  This file contains the names of ship
#		files along with the names of the LPP/OPPs to which they 
#		belong.
# input:	$TOP/HISTORY/bldquery/xreflist
# output:	$TOP/HISTORY/bldquery/LPP/LPPLIST associative array dbm file - 
#		associative array whose keys are the names of the LPPs that
#		were built during the build.
#
#		$TOP/HISTORY/bldquery/LPP/LPPNAMES associative array dbm file - 
#		associative array whose keys are ship file ids and whose
#		elements are space separted lists of all the LPPs that the
#		ship file belongs to.  For example,
#
#		$lppnames{15} = "bos bosnet bosext2"
#
#		Here the ship file corresponding to the file id 15 belongs to
#		the LPPs bos, bosnet, and bosext2.
#
#		$TOP/HISTORY/bldquery/LPP/NLPPNAMES associative array dbm 
#		file - same as the LPPNAMES file for except only for ship files
#		that were built during this build.
# remarks:
##############################################################################
sub	ProcessXreflist {
	# opening xreflist file; contains LPP information
	open(XREFLIST,"$BQDBLOCAL/xreflist");

	# reading each line from xreflist file
	while (<XREFLIST>) {
		chop $_;
		# each line consists of a ship file and an LPP name
		($shipfile,$lppopt) = split(/ /,$_);

		# stripping of leading "!!" from beginning of shipfile name 
		$shipfile =~ s/!!/./g;

		# parse true LPP name from LPP option name
		$lppname = &ParseLPPOption($lppopt);

		# define lpplist element to indicate that this LPP has been 
		# built
		$lpplist{$lppname} = $DEFINED;

		# encode shipfile name
		local(@shipids)=();
		for $rel (keys %rels_built) {
		    if (defined $encode{"$shipfile|$rel"}) {
			push(@shipids,$shipid = $encode{"$shipfile|$rel"});
		    }
		}
		if ($#shipids < 0) {
		    push(@shipids,$shipid = &EncodeFile($shipfile));
		}
		for $shipid (@shipids) {
		    # concatenate LPP name to list of LPP names for which
		    # current file is a ship file
		    $lppnames{$shipid} .= "$lppname ";

		    # if the current ship file id is defined in the Ntargetlist
		    # or Ndependentlist arrays then it is a "new" file; there-
		    # fore the LPP name should be appended to the list of LPP 
		    # names for the current ship file in the Nlppnames array
		    if ((defined $Ntargetlist[$shipid]) || 
			(defined $Ndependentlist[$shipid])) {
			$Nlppnames{$shipid} .= "$lppname ";
		    }
		}
	}
}

##############################################################################
# function:	CreateLPPShipFiles
# description:	Create files containing the names of all the ship files for  
#		each LPP.
# input:	$TOP/HISTORY/bldquery/LPP/LPPNAMES dbm file
# 		$TOP/HISTORY/bldquery/LPP/NLPPNAMES dbm file
#		dependentlist[] array 
#		Ndependentlist[] array 
#		srcrellist{} associative array
#		Nsrcrellist{} associative array
# output:	$TOP/HISTORY/bldquery/LPP/$lpp.SHIP - file containing the names
#		of all the ship files belonging to a specific LPP.  One built
#		for each LPP.
#
# 		$TOP/HISTORY/bldquery/LPP/$lpp.NSHIP - same as the .SHIP file   
#		above except only for ship files that were built during this 
#		build.
#
#		lppsrclist{} associative array - associative array whose keys
#		are a string made up of an LPP name followed by a source file
#		id corresponding to a source file that belongs to the named
#		LPP.
#
#		Nlppsrclist{} associative array - same as the lppsrclist{}    
#		associative array except only for source files that were built
#		during this build.
# remarks:
##############################################################################
sub	CreateLPPShipFiles {
	# creating ship list files for each LPP
	# walking thru list of all ship file ids
	foreach $id (keys %lppnames) {
		# llist array contains all the LPPs to which the current 
		# ship file belongs
		(@llist) = split(/ /,$lppnames{$id});

		# walking thru each LPP name in llist
		for ($l = 0; $l <= $#llist; $l++) {
			# constructing ship list file for current LPP
			$LPPSHIP_FILE="$BQDBLOCAL/LPP/$llist[$l].SHIP";

			# appending current ship file name to end of 
			# ship list file for current LPP
			open(LPPSHIP,">> $LPPSHIP_FILE");
			print LPPSHIP "$decode{$id}\n";
			close(LPPSHIP);
	
			# getting all source file ids that are part of the
			# current ship file
			&getsrcfileids($id,*depsrclist,*dependentlist,
				       *srcrellist);

			# creating entries in lppsrclist file; each entry 
			# consists of an LPP name and source file that belongs 
			# to it; each entry will be split up later to create 
			# the LPP source list files
			for ($i = 0; $i <= $#depsrclist; $i++) {
				$lppsrclist{"$llist[$l] $depsrclist[$i]"} = 
					$DEFINED;
			}

			# clearing out depsrclist array for next LPP 
			(@depsrclist) = ();
		}
	}

	# creating "new" ship list files for each LPP
	# walking thru list of all "new" ship file ids
	foreach $id (keys %Nlppnames) {
		# llist array contains all the LPPs to which the current 
		# ship file belongs
		(@llist) = split(/ /,$Nlppnames{$id});

		# walking thru each LPP name in llist
		for ($l = 0; $l <= $#llist; $l++) {
			# constructing "new" ship list file for current LPP
			$NLPPSHIP_FILE="$BQDBLOCAL/LPP/$llist[$l].NSHIP";
	
			# appending current ship file name to end of "new"
			# ship list file for the current LPP
			open(NLPPSHIP,">> $NLPPSHIP_FILE");
			print NLPPSHIP "$decode{$id}\n";
			close(NLPPSHIP);
	
			# getting all "new" source file ids that are part of 
			# the current ship file
			&getsrcfileids($id,*depsrclist,*Ndependentlist,
				       *Nsrcrellist);

			# creating entries in Nlppsrclist file; each entry 
			# consists of an LPP name and source file that belongs 
			# to it; each entry will be split up later to create 
			# the "new" source list files for each LPP
			for ($i = 0; $i <= $#depsrclist; $i++) {
				$Nlppsrclist{"$llist[$l] $depsrclist[$i]"} = 
					$DEFINED;
			}

			# clear out depsrclist for next LPP
			(@depsrclist) = ();
		}
	}
}

##############################################################################
# function:	CreateLPPSrcFiles
# description:	Create files containing the names of all the source files for  
#		each LPP.
# input:	lppsrclist{} associative array
#		Nlppsrclist{} associative array
# output:	$TOP/HISTORY/bldquery/LPP/$lpp.SRC - file containing the names
#		of all the source files belonging to a specific LPP.  One built
#		for each LPP.
#
# 		$TOP/HISTORY/bldquery/LPP/$lpp.NSRC - same as the .SRC file   
#		above except only for source files that were built during this 
#		build.
#
# remarks:
##############################################################################
sub	CreateLPPSrcFiles {
	# creating source list files for each LPP
	foreach $lppsrcid (keys %lppsrclist) {
		# separating out LPP name and source file name from 
		# lppsrclist array
		($lpp,$id) = split(/ /,$lppsrcid);

		# constructing source list file name for current LPP
		$LPPSRC_FILE="$BQDBLOCAL/LPP/$lpp.SRC";

		# appending source file name to end of LPP source list file
		if (!open(LPPSRC,">> $LPPSRC_FILE")) {
			print STDERR "couldn't open $LPPSRC_FILE\n";
		}
		print LPPSRC "$decode{$id}\n";
		close(LPPSRC);
	}
	undef(%lppsrclist);

	# creating "new" source list files for each LPP
	foreach $lppsrcid (keys %Nlppsrclist) {
		# separating out LPP name and source file name from 
		# lppsrclist array
		($lpp,$id) = split(/ /,$lppsrcid);

		# constructing source list file name for current LPP
		$NLPPSRC_FILE="$BQDBLOCAL/LPP/$lpp.NSRC";

		# appending source file name to end of "new" source list 
		# file for the current LPP
		if (!open(NLPPSRC,">> $NLPPSRC_FILE")) {
			print STDERR "couldn't open $NLPPSRC_FILE\n";
		}
		print NLPPSRC "$decode{$id}\n";
		close(NLPPSRC);
	}
	undef(%Nlppshiplist);
}

#################################  M A I N #################################

$SIG{'QUIT'} = 'error';
$SIG{'INT'} = 'error';
$SIG{'HUP'} = 'error';
$SIG{'TERM'} = 'error';

# constructing bldquery database path
chop($HISTORYPATH=`bldhistorypath`);
$BQDBLOCAL="$HISTORYPATH/bldquery";

# creating encode and decode associative array DBM files
dbmopen(%encode,"$BQDBLOCAL/ENCODE",0750);
dbmopen(%decode,"$BQDBLOCAL/DECODE",0750);

# open lpplist associative array DBM file
dbmopen(%lpplist,"$BQDBLOCAL/LPP/LPPLIST",0750);
# open lppnames associative array DBM file
dbmopen(%lppnames,"$BQDBLOCAL/LPP/LPPNAMES",0750);
# open Nlppnames associative array DBM file
dbmopen(%Nlppnames,"$BQDBLOCAL/LPP/NLPPNAMES",0750);


# process each release's makelist file; these files contain the target
# dependency relationships for all files built during the build
&ProcessMakelists;

# creating the files that contain all the source files that make up a release
&CreateRelSrcFiles;

# creating the files that contain all the ship files that make up a release
&CreateRelShipFiles;

# creating the target/dependency files that contain all the target/dependency
# relationships across all release
&CreateTargDepFiles;

# Processing the xreflist file; this file contains information linking ship  
# files to LPPs
&ProcessXreflist;

# creating the files that contain all the ship files that make up an LPP
&CreateLPPShipFiles;

# creating the files that contain all the source files that make up an LPP
&CreateLPPSrcFiles;

dbmclose(%encode);
dbmclose(%decode);
dbmclose(%lpplist);
dbmclose(%lppnames);
dbmclose(%Nlppnames);

exit 0;

