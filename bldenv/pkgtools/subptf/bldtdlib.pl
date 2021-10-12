#
# @(#)54	1.21  src/bldenv/pkgtools/subptf/bldtdlib.pl, pkgtools, bos412, GOLDA411a 3/28/94 15:42:15
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS:	BLDTDbuild 
#		Getbuiltlist 
#		Appendlist 
#		Savedependents 
#		BLDTDgettargetdefects 
#		BLDTD2gettargetdefects 
#		BLDTDgetsrcdefects 
#		Finddependents 
#		Getterminaldepends 
#		BLDTDgetdependents 
#		BLDTDgetdependentslist
#		Savebldenvnames
#		Otherbuildenv
#		Encodedefect
#		Decodedefect
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

# This library contains code for "selective-fix" target-dependency database.
# Entry points (BLDTD prefix) are provided to build the database and access 
# the data. 

# NOTE: bldperlconst, bldperllog, and bldoptlib must be defined (do or 
# require) before these routines are referenced/included.
#
# NOTE: some returns codes are oriented to the shell (i.e. success is a 0 
# which is false in perl).


sub BLDTDinterfaceinit {
  ($release) = @_; $rc=$SUCCESS;
  local($rc);
  $workpath=`ksh bldreleasepath $release`; chop $workpath; ## global
  dbmopen(targetfd,"$workpath/bldtd/targetfd",0750) || 
	&log("$lvar -x","$workpath/bldtd/targetfd open error ($!)");
  (&Opendecodeddefects($release) == $SUCCESS) || 
	&log("$lvar -x","cannot access decoded defects"); 
  return $rc;
}

#
# NAME: BLDTDbuild
#
# FUNCTION: Build the target-dependency database (bldtd entry point).
#
#	Input data is first read into associative arrays (AAs) for quick
#       and convenient access.  AAs are indexed by file name.  The bldtd data
#       is then organized and stored for later access by the bldtd access
#	functions.  When possible the data is stored in AAs mapped to files;
#       in cases where large record sizes may exist, data is stored in flat
#	files. (Note AAs mapped to files have a 1k record size limitation.)
#
# INPUT: release (parameter) - CMVC level release name
#	 makelist (parameter) - target/dependency list for the release's files
#	 defectlist (parameter) - list of defects within the release level
#	 updatelist (parameter) - list of updated ship files for release
#	 bldenvlist (parameter) - list of updated bldenv files for release
#
# OUTPUT: new bldenvlist is written to stdout
#
# SIDE EFFECTS: The target-dependency database files are built in 
#	$(bldreleasepath $release)/bldtd. This includes the following files:
# 	    sourcefd (file) - AA mapping source files to defect(s)
#           targetfd (file) - AA mapping target files to defect(s)
#           alldepends$file (files) - contains all dependents for each $file
#                                    (created in Savedependents)
#           newdepends$file (files) - contains all new dependents for each 
#                                    $file (created in Savedependents)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTES: This function is required for execution efficiency.  It processes
#	 its input data and stores it in a format for quick retrieval by the
#	 access routines.  The data format was defined in accordance with
#	 how the data will later be needed.
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDTDbuild {
  local($release,$makelist,$defectlist,$updatelist,$bldenvlist,$changelist)=@_;
  local(%sourcefd,%targetfd,$path,@newtarget,@alldepend,@newdepend,$rc);
  local($makeline,$target,$dependency,$built,$udfile);
  local($benvfile,$defect,$file,%zbuiltfiles,$rc,%benvlistplus);
  local ($changefile,%changefiles,$changeline,$changedefect,$changes);

  $rc=$SUCCESS;
  chop($workpath=`ksh bldreleasepath $release`);  ## global
  $path="$workpath/bldtd"; 
  `rm -fr $path`; mkdir($path,0750);  ## clean up previous data and create dir
  dbmopen(sourcefd,"$path/sourcefd",0750) || 
	&log("$lvar -x","$path/sourcefd open error ($!)");
  dbmopen(targetfd,"$path/targetfd",0750) || 
	&log("$lvar -x","$path/targetfd open error ($!)");
  if (defined $ENV{'BLDBASERELEASE'}) 
	{ $baserel = $ENV{'BLDBASERELEASE'}; } 
  else {
    $baserel = "bos";
    $baserel .= &bldhostsfile_perl("get_afs_base",$FALSE,"HOSTSFILE_BLDNODE"); 
    chop $baserel;
    if ( $baserel eq "bos" ) {
      &log("$lvar -x",
           "bldhostsfile_perl(get_afs_base,$FALSE,HOSTSFILE_BLDNODE) failed");
    }
  }
  ### load all data from maklist file into AAs for quick access ###
  open (MAKELIST,"<$makelist") || 
	&log("$lvar -x","makelist ($makelist) open error ($!)");
  while ($makeline = <MAKELIST>) {
        chop $makeline;
        ($target,$dependency,$built) = split(/ /,$makeline,3);
	next if ($target eq $dependency);  # can happen because of normalization
	if ($target eq "" || $dependency eq "") { 
		&log("$lvar -e","missing token in make line $. ($makeline)"); 
		next;
	}
	if (defined $built && $built ne "" && $built ne "NEW") { 
		&log("$lvar -e","illegal type in make line $. ($makeline)");
		next;
	}
	if ($release eq $baserel) {
		if ($dependency =~ m#^/bldenv.*NLS#) {
		 &log("$lvar -w", 
                 "unexpected /bldenv dependency in make line $. ($makeline)");
		}
	}
	if ($dependency =~ m#^/usr|^/lib|^/bin|^/etc#) {
		&log("$lvar -w",
		"unexpected host dependency in make line $. ($makeline)");
	}

        ### encode the path names to save space ###
        $target=&Encodepath($target); $dependency=&Encodepath($dependency);
        ### generating a "linked list" of all dependents ###
        $alldepend[$target] .= $dependency . $SEP;
        if (defined $built && $built eq "NEW") { 
        	### generating a "linked list" of newly built targets ###
        	$newtarget[$dependency] .= $target . $SEP;
        	### generating a "linked list" of newly built dependents ###
		$newdepend[$target] .= $dependency . $SEP;
	}
  }

  ### load all data from updatelist file into AA ###
  open (UPDATELIST,$updatelist) || 
	&log("$lvar -x","updatelist ($updatelist) open error ($!)");
  while ($udfile = <UPDATELIST>) { 
      chop $udfile;
      $udlist{$udfile}=$DEFINED;
      if ( $! != 0 ) {
          &log("$lvar -x","updatelist ($updatelist) write error ($!)");
      }
  }

  ### load all data from bldenvlist file into AA ###
  open (BLDENVLIST,$bldenvlist) || 
	&log("$lvar -x","bldenvlist ($bldenvlist) open error ($!)"); 
  while ($benvfile = <BLDENVLIST>) {
	chop $benvfile; 
	$benvlist{$benvfile}=$DEFINED; 
        if ( $! != 0 ) {
            &log("$lvar -x","bldenvlist ($bldenvlist) write error ($!)");
        }
  }

  ### load all data from change list file into AA ###
  open (CHANGELIST,$changelist) ||
	&log("$lvar -x","changelist ($changelist) open error ($!)"); 
  while ($changeline = <CHANGELIST>) {
	chop $changeline;
        ($changedefect,$changefile) = split(/\|/,$changeline);
	(defined $changefiles{$changedefect}) ?
        	($changefiles{$changedefect} .= $changefile . $SEP) :
        	($changefiles{$changedefect} = $changefile . $SEP); 
	if ($newtarget[&Encodepath($changefile)] eq undef) {
                if ($changefile !~ m/TOOLS/) {  ## Ignore tools; built earlier
		    &log("$lvar -w","$changefile missing NEW makelist entry"); 
		}
	}
  }

  (defined $ENV{'BLDOPTLIBS'}) && 
      &OptimizeLibDeps($release,*newtarget,*newdepend,*alldepend,
       *benvlist,*benvlistplus);

  (&Openenvnames($release) == $SUCCESS) || 
	&log("$lvar -x","cannot access other bldenv names"); 
  (&Opendecodeddefects($release) == $SUCCESS) || 
	&log("$lvar -x","cannot access decoded defects"); 

  ### gather data (using previous built AAs) for each defect in defectlist ###
  &log("$lvar -b","processing defect list");
  open (DEFECTLIST,$defectlist) || 
	&log("$lvar -x","defectlist ($defectlist) open error ($!)");
  while ($defect = <DEFECTLIST>) {
	chop $defect;
 	(defined $ENV{'DEBUG'}) && &log("$lvar -b","processing defect $defect");
	chop ($changes=$changefiles{&Defectnumber($defect)});
	foreach $file (split(/$SEP/,$changes)) {
		undef $hastarget; reset 'z';  ## clear zbuiltfiles
		&Getbuiltlist ($file,*newtarget,*zbuiltfiles);
		foreach $bfile (keys %zbuiltfiles) {
			## Only need this data for files in the lists ##
			if ($udlist{$bfile} || $benvlist{$bfile}) {
				&Appendlist(*targetfd,$bfile,
					&Encodedefect($defect),$SEP);
				$hastarget=1;
 		}	}	
		# Don't save a source file/defect which will not go into a 
		# PTF.  This list eventually goes into history.  If a source
		# file/defect pair is stored in history when it did not
		# go into a PTF a problem occurs - previous dependencies
		# on the pair cannot be translated into a PTFID since one
		# does not exist (this would look like an error to subptfpkg).
        	&Appendlist(*sourcefd,$file,$defect,$SEP) if ($hastarget);
  }	}

  ### save new dependents which will later be queried by GETNEWDEPENDS ###
  &log("$lvar -b","saving new update dependents");
  &Savedependents ($release,*udlist,"NEW",*newdepend); 
  &log("$lvar -b","saving new bldenv dependents");
  &Savedependents ($release,*benvlist,"NEW",*newdepend); 
  ### save all dependents which will later be queried by GETALLDEPENDS ###
  &log("$lvar -b","saving all update dependents");
  &Savedependents ($release,*udlist,"ALL",*alldepend); 
  &log("$lvar -b","saving all bldenv dependents");
  &Savedependents ($release,*benvlistplus,"ALL",*alldepend); 

  ### write to stdout the new build environment list
  foreach (keys %benvlist) { 
        print "$_\n";
  }

  ### save bldenv names so "bldtd LEVEL" for other releases can use ###
  &log("$lvar -b","saving bldenv names");
  &Savebldenvnames ($release,*benvlistplus);

  dbmclose(sourcefd); dbmclose(targetfd);

  return $rc;
}

#
# NAME: Getbuiltlist
#
# FUNCTION: Return list of files built because of the specified file.
#
#	Built files are determined by recursively walking the AA list
#	of targets (targets are indexed by dependency files).  If the
#       recursion level gets too deep, it is assumed a circular target
#	list exists.
#
# INPUT: file (parameter) - file whose built files are returned
#	 newtarget (parameter) - AA linked list by target of new files 
#        callcount (parameter) - depth of recursion the function is currently
#				 being called from
#
# OUTPUT: builtfiles (parameter) - list of built files (one per line)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub Getbuiltlist {
  local($file,*newtarget,*builtfiles) = @_;
  local($tfile);  # @stack is global for efficiency
  push(@stack,(&Encodepath($file)));
  while (($tfile=pop(@stack)) ne undef) {
	next if ($builtfiles[$tfile] ne undef);  ## next if tfile aready done
  	$builtfiles{&Decodepath($tfile)} = $DEFINED;
	next if ($newtarget[$tfile] eq undef);
	push(@stack,(split(/$SEP/,$newtarget[$tfile])));
	if ($#stack > $MAXITERATIONS) {
		&log ("$lvar -x","probably circular dependency with $file");
	}
  }
}

#
# NAME: Appendlist
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub Appendlist {
  local(*list,$key,$value,$sep) = @_;
  local($listvalue,$exist); $exist=$FALSE;
  foreach $listvalue (split(/$sep/,$list{$key}))
  	{ if ($listvalue eq $value) { $exist=$TRUE; } }
  if ($exist==$FALSE) { 
        (defined $list{$key}) ?
        	($list{$key} .= $value . $sep) :
        	($list{$key} = $value . $sep);
        if ( $! != 0 ) {
            &log("$lvar -x","Appendlist write error ($!)");
        }
  }
}

#
# NAME: Savedependents
#
# FUNCTION: For each input file, find and store (for later retrieval) its 
#	    dependencies. 
#
#	Each file's dependencies are stored in a separate flat file.
#
# INPUT: release (parameter) - CMVC level release name
#	 filelist (parameter) - array of file names
#	 typeflag (parameter) - flags if only new dependents or all are saved
#	 depends (parameter) - AA list of dependency relationships
#
# OUTPUT:
#        alldepends$file (files) - contains all dependents for each $file
#        newdepends$file (files) - contains all new dependents for each 
#
# FORMAT: one dependent per line is written to the file
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub Savedependents {
  local($release,*filelist,$typeflag,*depends) = @_;
  local($fileprefix,$file,%ydependentlist,$dfile,$workpath,$rc);
  chop($workpath=`ksh bldreleasepath $release`);
  $rc=$SUCCESS;
  $fileprefix = $typeflag eq "NEW" ? "newdepend" : "alldepend";
  foreach $file (keys %filelist) {
	reset 'y';  # clear ydependentlist
	&Finddependents ($file,$typeflag,*depends,*ydependentlist);
	$file =~ s#.*@@#!!#; ## if bldenv name, save in tpath format
		             ## - other releases reference name in tpath format
	$file =~ s#/#@@#g;  ## convert file's full path into local file name
  	open (OUTFILE,">$workpath/bldtd/$fileprefix.$file") ||
	    &log("$lvar -x",
		"unable to open $workpath/bldtd/$fileprefix.$file ($!)");
  	foreach $dfile (keys %ydependentlist) {
	    print OUTFILE "$dfile\n" || 
	      &log("$lvar -x",
		"$workpath/bldtd/$fileprefix.$file output error ($!)");
 	}
	close (OUTFILE);
  }
  return $rc;
}

#
# NAME: Gettargetdefects
#
# FUNCTION: Return defects (within current release level) which caused 
#	    a target file to be built. (bldtd entry point)
#
# INPUT: release (parameter) - CMVC level release name
# 	 file (parameter) - a built file
# 	 targetfd (file) - AA mapping target files to defect(s)
#
# OUTPUT: stdout list of defects which caused the file to build (could be null)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDTDgettargetdefects {
  local($release,$file) = @_; $rc=$SUCCESS;
  local(%targetfd,$value,$rc);
  $workpath=`ksh bldreleasepath $release`; chop $workpath;
  dbmopen(targetfd,"$workpath/bldtd/targetfd",0750) ||
	&log("$lvar -x","$workpath/bldtd/targetfd open error ($!)");
  (&Opendecodeddefects($release) == $SUCCESS) || 
	&log("$lvar -x","cannot access decoded defects"); 
  &BLDTD2gettargetdefects($file,*value); print "$value\n";
  dbmclose (targetfd);
  return $rc;
}
 
sub BLDTD2gettargetdefects {
  local($file,*value) = @_;
  @dcodes=split(/$SEP/,$targetfd{$file});  # split into individual codes
  grep($_=&Decodedefect($_),@dcodes);	   # covert each code to defect
  $value=join($SEP,@dcodes);		   # join defects into one string
}

#
# NAME: BLDTDgetsrcdefects
#
# FUNCTION: Return defects (within current release's level) which caused 
#	    a source file to be built. (bldtd entry point)
#
# INPUT: release (parameter) - CMVC level release name
# 	 sourcefd (file) - AA mapping source file to defect(s)
#
# OUTPUT: stdout list of source files along with associated defects
#	  format: "file|defect defect defect..." 
#	          (one file is printed per line)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDTDgetsrcdefects {
  local($release) = @_;
  local($workpath,$file,%sourcefd,$fdvalue,$rc);
  $rc=$SUCCESS;
  $workpath=`ksh bldreleasepath $release`; chop $workpath;
  dbmopen(sourcefd,"$workpath/bldtd/sourcefd",0750) || 
	&log("$lvar -x","$workpath/bldtd/sourcefd open error ($!)");
  foreach $file (keys %sourcefd) 
	{ chop($fdvalue=$sourcefd{$file}); print "$file|$fdvalue\n"; }
  dbmclose (sourcefd); 
  return $rc;
}

#
# NAME: Finddependents
#
# FUNCTION: Return list of terminal files directly or indirectly dependent
#           on a specified file (according to input dependency list).
#
# INPUT: file (parameter) - a built file
#	 typeflag (parameter) - indicates if NEW or ALL dependencies are
#		being searched for
#	 depends (parameter) - AA of dependency relationships
#
# OUTPUT: tdependentlist (parameter) - AA of dependents for input file
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub Finddependents {
  	local($file,$typeflag,*depends,*tdependentlist) = @_;
	local (%tdslist,$dependent,@list,$listitem,$release,$workpath);
	&Getterminaldepends ($file,*depends,*tdslist);
	foreach $dependent (keys %tdslist) {
		if ($typeflag eq "ALL" && 
		   ($release=&Otherbuildenv($dependent)) ne undef) {
			$workpath=`ksh bldreleasepath $release`; chop $workpath;
			## include a bldenv file's dependents ##
			&BLDTDgetdependentslist
				($workpath,$dependent,$typeflag,*list);
			while (($listitem=pop(@list)) ne undef) { 
			     chop ($listitem);
			     $tdependentlist{$listitem}=$DEFINED; 
                        }
		}
		else { $tdependentlist{$dependent}=$DEFINED; }
	}
}

#
# NAME: Getterminaldepends
#
# FUNCTION: Return built files's list of source files (or terminal/root files).
#
#	Terminal files are determined by recursively walking the AA list
#	of dependencies (dependents are indexed by target files).  If the
#       recursion level gets too deep, it is assumed a circular dependency
#	list exists.
#
# INPUT: file (parameter) - file whose built files are returned
#	 depends (parameter) - AA linked list, by dependents, of files 
#        callcount (parameter) - depth of recursion the function is currently
#				 being called from
#
# OUTPUT: tdslist (parameter) - AA list of terminal-dependency files 
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub Getterminaldepends {
  local($file,*depends,*tdslist) = @_;
  local(@dependents,$tfile); # @stack is global for efficiency
  $tfile=&Encodepath($file);
  push(@stack,($tfile));
  while (($tfile=pop(@stack)) ne undef) {
        $file=&Decodepath($tfile);
        next if ($tdslist{$file} ne undef);
	if ($depends[$tfile] eq undef) {
  		$tdslist{$file}=$DEFINED;
	}
	else {
		@dependents=(split(/$SEP/,$depends[$tfile]));
		push(@stack,(@dependents));
		if ($#stack > $MAXITERATIONS) {
		    &log ("$lvar -x","probably circular dependency ($file)");
		}
	}
  } 
}

#
# NAME: BLDTDgetdependents
#
# FUNCTION: Print from target-dependency database a file's dependents (bldtd
#           entry point)
#
# INPUT: release (parameter) - CMVC level release name
# 	 file (parameter) - file name
#	 typeflag (parameter) - flag if new dependents or all are returned
#
# OUTPUT: stdout list of dependency file names (one name per line)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDTDgetdependents {
  local($release,$file,$typeflag) = @_;
  local(@dependentlist,$rc);
  $workpath=`ksh bldreleasepath $release`; chop $workpath; ## global for speed
  $rc=&BLDTDgetdependentslist ($workpath,$file,$typeflag,*dependentlist);
  print @dependentlist;
  return $rc;
}

#
# NAME: BLDTDgetdependentslist
#
# FUNCTION: Return list of a file's dependents
#
# INPUT: release (parameter) - CMVC level release name
# 	 file (parameter) - file name
#	 typeflag (parameter) - flag if new dependents or all are returned
#	 *depend$file (file) - contains list of dependents for file
#	 bldworkpath (global) - contains release working directory
#
# OUTPUT: dependentlist (parameter) - array of dependency file names
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (file found) or 1 (file not found)
#
sub BLDTDgetdependentslist {
  local($workpath,$file,$typeflag,*dependentlist) = @_;
  local($fileprefix,$rc); $rc=$SUCCESS;
  $fileprefix = $typeflag eq "NEW" ? "newdepend" : "alldepend";
  $file =~ s#/#@@#g;  ## convert file's full path into local file name
  open (FILELIST,"<$workpath/bldtd/$fileprefix.$file") || return $FAILURE;
  while (<FILELIST>) {
       push(@dependentlist,($_));
  }
  close (FILELIST);
  return $SUCCESS;
}

#
# NAME: Openenvnames 
#
# FUNCTION: Opens the mapped AA which contains bldenv names for other
#           releases. Must be called before Savebldenvnames or
#	    Otherbldenv are used.
#
# INPUT: release (parameter) - the CMVC level release name
#
# OUTPUT: bldenvnames (global) - the handle to the mapped AA is initialized
#
# SIDE EFFECTS: the associated mapped AA file (bldenvnames) is opened 
#
# NOTES: This function allows the AA file to be opened only once, instead
#   of each time Otherbldenv is called.  This gives a big performance
#   gain since Otherbldenv is called many times.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub Openenvnames {
  local($release) = @_;
  local($rc); $rc=$SUCCESS;
  chop($workpath=`ksh bldglobalpath`);
  dbmopen(bldenvnames,"$workpath/bldenvnames",0750) ||
        &log("$lvar -e","$workpath/bldenvnames open error ($!)");
  return $rc;
}

#
# NAME: Savebldenvnames
#
# FUNCTION: Save list of build environment names.
#
# INPUT: bldenvlist (parameter) - AA list of build environment file names
#        bldenvnames (global) - mapped AA list of build environment file names
#
# OUTPUT: bldenvnames (global) - list of bldenv file names added to mapped AA
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: none
#
sub Savebldenvnames {
  local($release,*bldenvlist) = @_;
  foreach (keys %bldenvlist) { 
	s#.*@@#!!#;  # save bldenv name in tpath format
		     # other release will reference name in tpath format
	$bldenvnames{$_}=$release; 
        if ( $! != 0 ) {
            &log("$lvar -x","Savebldenvnames ($workpath/bldenvnames) write error ($!)");
        }
  }
  dbmclose (bldenvnames);
}

#
# NAME: Otherbuildenv 
#
# FUNCTION: Indicates if a file was put in the bldenv by other release.
#
# INPUT: file (parameter) - file name
#        bldenvnames (global) - mapped AA list of build environment file names
#
# OUTPUT: none
#
# SIDE EFFECTS: none 
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 
#
sub Otherbuildenv {
  local($file) = @_;
  return $bldenvnames{$file};
}

sub Opendecodeddefects {
  local($release) = @_;
  local($workpath,$rc); $rc=$SUCCESS;
  $workpath=`ksh bldreleasepath $release`; chop $workpath;
  dbmopen(decodeddefects,"$workpath/bldtd/decodeddefects",0750) ||
        &log("$lvar -e","$workpath/bldtd/decodeddefects open error ($!)");
  return $rc;
}

sub Encodedefect {
  local($defect) = @_;
  if ($encodeddefects{$defect} eq undef) {
	$encodeddefects{$defect} = ++$curdefcode;
  	$decodeddefects{$curdefcode} = $defect;
        if ( $! != 0 ) {
            &log("$lvar -x","Encodedefect ($workpath/bldtd/decodeddefects) write error ($!)");
        }
  }
  return ($encodeddefects{$defect});
}

sub Decodedefect {
  local($code) = @_;
  if (($code ne undef) && ($decodeddefects{$code} eq undef)) {
	&log("-e","unable to decode bldtd defect code $code");
  }
  return ($decodeddefects{$code});
}
