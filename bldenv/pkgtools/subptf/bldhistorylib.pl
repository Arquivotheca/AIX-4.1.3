#
# @(#)51	1.18  src/bldenv/pkgtools/subptf/bldhistorylib.pl, pkgtools, bos412, GOLDA411a 1/14/94 15:55:38
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	BLDHISTORYsavedep 
#		BLDHISTORYgetdep
#		BLDHISTORY2getdep
#		BLDHISTORYrevise
#		BLDHISTORYsavePTFdefects
#		BLDHISTORYgetdefectPTFs
#		BLDHISTORYinitPTFsession
#		BLDHISTORYcommitPTFsession
#		BLDHISTORYdump
#		BLDHISTORYimport
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

sub BLDHISTORYinterfaceinit {
  local($historypath,$rc); $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  dbmopen(Cdephistory,"$historypath/Cdephistory",0777) ||
        &log("$lvar -x","$historypath/Cdephistory open error ($!)");
  return $rc;
}

#
# NAME: BLDHISTORYsavedep
#
# FUNCTION: Update defect history with source files and assoc. defects.
#
# INPUT: defectsfile (parameter) - file containing list of filenames with
#		associated defects which modified the file
#	 dephistory (file, AA) - the dependency history database
#
# OUTPUT: none
#
# SIDE EFFECTS: the history database is updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT:
#	defectsfile: "filename|defects..." (one filename per line)
#	Cdephistory: AA of defects separated by comma and indexed by file name
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDHISTORYsavedep {
  local($defectsfile) = @_; $rc=$SUCCESS;
  local($historypath,$dataline,$file,$defects,$newlist,$newdefect,$rc); 

  ## open the history (source files and defects which last modified them)
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  dbmopen(Cdephistory,"$historypath/Cdephistory",0770) ||
        &log("$lvar -x","$historypath/Cdephistory open error ($!)");

  ## open ptfs (mapped AA) to find all defect which were packaged into PTFs
  chop($tmphistorypath=`ksh bldglobalpath`); 
  dbmopen(ptfs,"$tmphistorypath/secondptfhistory",0770) ||
        &log("$lvar -x","$tmphistorypath/secondptfhistory open error ($!)");

  ## open new list of source files and defects (file is passed in as parameter)
  open (DEFECTSFILE,"<$defectsfile") ||
        &log("$lvar -x","defectsfile ($defectsfile) open error ($!)");

  ## save info from input file into the history as long as the defects
  ## were packaged into a PTF (as specifed in ptfs)
  while ($dataline=<DEFECTSFILE>) {
	chop $dataline; $newlist="";
        ($file,$defects) = split(/\|/,$dataline);
	foreach $newdefect (split(/$SEP/,$defects)) {
		if (defined $ptfs{$newdefect}) {
			$newlist .= $newdefect . $SEP; 
		}
		else {
			&log("$lvar -f","$newdefect ignored since not in PTF");
		}
	}
	if ($newlist ne "") {
		chop $newlist;  # remove extra $SEP
		$Cdephistory{$file}=$newlist;
                if ( $! != 0 ) {
                    &log("$lvar -x","Cdephistory ($historypath/Cdephistory) write error ($!)");
                }
	}
  }

  dbmclose(Cdephistory); dbmclose (ptfs); close (DEFECTSFILE);
  return $rc;
}

#
# NAME: BLDHISTORYgetdep
#
# FUNCTION: Return defects which previously modified a set of files.
#
# INPUT: namesfile - list of file names (one per line)
#	 Cdephistory (file, AA) - the dependency history database
#
# OUTPUT: list of defects (one per line)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDHISTORYgetdep {
  local($namesfile) = @_; 
  local($historypath,$file,$defect,%defectlist,$rc); $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  dbmopen(Cdephistory,"$historypath/Cdephistory",0777) ||
        &log("$lvar -x","$historypath/Cdephistory open error ($!)");
  open (NAMESFILE,"<$namesfile") ||
        &log("$lvar -x","namesfile ($namesfile) open error ($!)");
  while ($file=<NAMESFILE>) {
	chop ($file);
	if (defined $Cdephistory{$file}) { 
		foreach $defect (split(/$SEP/,$Cdephistory{$file}))
			{ $defectlist{$defect}=$DEFINED; }
	}
  }
  foreach $defect (sort keys %defectlist) { print "$defect\n"; }
  dbmclose(Cdephistory); close (NAMESFILE);
  return $rc;
}
sub BLDHISTORY2getdep {
  local(*nameslist,*defectlist) = @_; $rc=$SUCCESS;
  local($file,$defect,$rc); 
  foreach $file (@nameslist) {
	chop $file;
	if (defined $Cdephistory{$file}) { 
		foreach $defect (split(/$SEP/,$Cdephistory{$file}))
			{ $defectlist{$defect}=$DEFINED; }
	}
  }
  return $rc;
}

#
# NAME: BLDHISTORYrevise
#
# FUNCTION: Syncronize history database with current source files.
#
# INPUT: changesfile (parameter) - file containing CMVC change data
#	 Cdephistory (file,AA) - the dependency history database
#
# OUTPUT: none
#
# SIDE EFFECTS: the history database is updated (only names changed)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT:
#	changesfile: TBD
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
# PSEUDOCODE:
#	path=`Globalpath`
#	open path/dephistory database
#	for each line in changesfile
#		update dephistory according to the change line... (TBD)
#	end for
sub BLDHISTORYrevise { 
	# opening bldhistory database files
	chop($historypath = `ksh bldhistorypath`);
	$historypath .= "/bldhistory";
	if(!dbmopen(%Cdep,"$historypath/Cdephistory",0777)) {
            &log("$lvar -x","$historypath/Cdephistory open error ($!)");
	}

	# opening release list file
	if(!open(RELLIST,"$ENV{'RELEASE_LIST'}")) {
            &log("$lvar -x","Cannot find/open RELEASE_LIST file: $ENV{'RELEASE_LIST'} ($!)");
	}

	# looking for "bldreviselist" file in each release directory
	while($release = <RELLIST>) {
	    chop($release);
	    chop($releasepath = `ksh bldreleasepath $release`);
	    if (!open(REVISE,"$releasepath/reviselist")) {
		next;
	    }

	    # found a "reviselist" file - processing it now
	    while($line = <REVISE>) {
		chop($line);
		# parsing line from "bldreviselist" file
		($changetype,$filename1,$filename2)=split(/\|/,$line);

		# processing delete entry
		if ($changetype eq "delete") {
		    if (defined $Cdep{$filename1}) {
			$Cdep{"$filename1\.DELETED"} = "$Cdep{$filename1}";
			delete $Cdep{$filename1};
		    }
		# processing recreate entry
		} elsif ($changetype eq "recreate") {
		    if (defined $Cdep{"$filename1\.DELETED"}) {
			$Cdep{$filename1} = $Cdep{"$filename1\.DELETED"};
			delete $Cdep{"$filename1\.DELETED"};
		    }
		# processing rename entry
		} elsif ($changetype eq "rename") {
		    if (defined $Cdep{$filename1}) {
			$Cdep{$filename2} = $Cdep{$filename1};
			delete $Cdep{$filename1};
		    }
		}
	    }
	}
	dbmclose(%Cdep);

	return $SUCCESS; 
}

#
# NAME: BLDHISTORYsavePTFdefects
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: none
#
# SIDE EFFECTS: the history database is updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT:
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDHISTORYsavePTFdefects {
  local($type,$PTFdefectsfile) = @_; $rc=$SUCCESS;
  local($tmphistorypath,$ptfhistory,%ptfs,$dataline,$ptf,$defects,$list,$rc); 
  $ptfhistory = ($type eq "PRIME") ? "primeptfhistory" : "secondptfhistory";
  $rc=$SUCCESS;
  chop($tmphistorypath=`ksh bldglobalpath`); 
  dbmopen(ptfs,"$tmphistorypath/$ptfhistory",0770) ||
        &log("$lvar -x","$tmphistorypath/$ptfhistory open error ($!)");
  open (PTFDEFECTSFILE,"<$PTFdefectsfile") ||
        &log("$lvar -x","PTF defects file ($defectsfile) open error ($!)");
  while ($dataline=<PTFDEFECTSFILE>) {
	chop $dataline;
        ($ptf,$defects) = split(/\|/,$dataline);
	foreach $defect (split(/$SEP/,$defects)) {
		$list=$ptfs{$defect} . $ptf;
		$ptfs{$defect} = &Uniquesort($SEP,$list) . $SEP;
                if ( $! != 0 ) {
                    &log("$lvar -x","ptfdefectsfile ($tmphistorypath/$ptfhistory) write error ($!)");
                }
	}
  }
  dbmclose(ptfs); close (PTFDEFECTSFILE);
  return $rc;
}

#
# NAME: BLDHISTORYgetdefectPTFs
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: none
#
# SIDE EFFECTS: the history database is updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT:
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDHISTORYgetdefectPTFs {
  local($type,$PTFdefectsfile) = @_; 
  local($tmphistorypath,$ptfhistory,%ptfs,$defect,$PTFlist,$rc); $rc=$SUCCESS;
  chop($tmphistorypath=`ksh bldglobalpath`); 
  $ptfhistory = ($type eq "PRIME") ? "primeptfhistory" : "secondptfhistory";
  dbmopen(ptfs,"$tmphistorypath/$ptfhistory",0770) ||
        &log("$lvar -x","$tmphistorypath/$ptfhistory open error ($!)");
  open (DEFECTSFILE,"<$PTFdefectsfile") ||
        &log("$lvar -x","PTF defects file ($defectsfile) open error ($!)");
  while ($defect=<DEFECTSFILE>) {
	chop $defect; 
	$ptflist = $ptfs{$defect}; chop ($ptflist) unless ($ptflist eq undef);
	print "$defect|$ptflist\n" if ($ptflist ne undef);
  }
  dbmclose(ptfs); close (PTFDEFECTSFILE);
  return $rc;
}

#
# NAME: BLDHISTORYinitsession
#
# FUNCTION: 
#
# INPUT: 
#
# OUTPUT: none
#
# SIDE EFFECTS: the history database is updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT:
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
sub BLDHISTORYinitPTFsession {
  local($historypath,$tmphistorypath,$rc); $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  chop($tmphistorypath=`ksh bldglobalpath`); 
  `cp $historypath/Cprimeptfhistory.dir $tmphistorypath/primeptfhistory.dir`;
  if ($? != $SUCCESS) {
     &log("$lvar -x","unable to copy $historypath/Cprimeptfhistory.dir");
  }
  `cp $historypath/Cprimeptfhistory.pag $tmphistorypath/primeptfhistory.pag`;
  if ($? != $SUCCESS) {
      &log("$lvar -x","unable to copy $historypath/Cprimeptfhistory.pag");
  }
  `cp $historypath/Csecondptfhistory.dir $tmphistorypath/secondptfhistory.dir`;
  if ($? != $SUCCESS) {
   &log("$lvar -x","unable to copy $historypath/Csecondptfhistory.dir");
  }
  `cp $historypath/Csecondptfhistory.pag $tmphistorypath/secondptfhistory.pag`;
  if ($? != $SUCCESS) {
   &log("$lvar -x","unable to copy $historypath/Csecondptfhistory.pag");
  }
  return $rc;
}

sub BLDHISTORYcommitPTFsession {
  local($historypath,$tmphistorypath,$rc); $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  chop($tmphistorypath=`ksh bldglobalpath`); 
  `cp $tmphistorypath/primeptfhistory.dir $historypath/Cprimeptfhistory.dir`;
  if ($? != $SUCCESS) {
        &log("$lvar -x","unable to copy $tmphistorypath/primeptfhistory.dir");
  }
  `cp $tmphistorypath/primeptfhistory.pag $historypath/Cprimeptfhistory.pag`;
  if ($? != $SUCCESS) {
        &log("$lvar -x","unable to copy $tmphistorypath/primeptfhistory.pag");
  }
  `cp $tmphistorypath/secondptfhistory.dir $historypath/Csecondptfhistory.dir`;
  if ($? != $SUCCESS) {
        &log("$lvar -x","unable to copy $tmphistorypath/secondptfhistory.dir");
  }
  `cp $tmphistorypath/secondptfhistory.pag $historypath/Csecondptfhistory.pag`;
  if ($? != $SUCCESS) {
        &log("$lvar -x","unable to copy $tmphistorypath/secondptfhistory.pag");
  }
  return $rc;
}

sub BLDHISTORYdump {
  local($historypath,$file,$defect,%ptfs,%dephistory,$rc); $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  dbmopen(dephistory,"$historypath/Cdephistory",0777) ||
        &log("$lvar -x","$historypath/Cdephistory open error ($!)");
  foreach $file (keys dephistory) {
	print "SRCDEP|$file|$dephistory{$file}\n";
  }
  dbmclose(dephistory);

  dbmopen(ptfs,"$historypath/Cprimeptfhistory",0770) ||
        &log("$lvar -x","$historypath/Cprimeptfhistory open error ($!)");
  foreach $defect (keys %ptfs) {
                print "PRIMEPTF|$defect|$ptfs{$defect}\n";
  }
  dbmclose(ptfs);

  dbmopen(ptfs,"$historypath/Csecondptfhistory",0770) ||
        &log("$lvar -x","$historypath/Csecondptfhistory open error ($!)");
  foreach $defect (keys %ptfs) {
                print "SECONDPTF|$defect|$ptfs{$defect}\n";
  }
  dbmclose(ptfs);

  return $rc;
}

sub BLDHISTORYimport {
  local($importfile)=@_;
  local($historypath,$token,$field1,$field2,%Pptfs,%Sptfs,%dephistory,$rc); 
  $rc=$SUCCESS;
  chop($historypath=`ksh bldhistorypath`); $historypath .= "/bldhistory";
  dbmopen(dephistory,"$historypath/Cdephistory",0777) ||
        &log("$lvar -x","$historypath/Cdephistory open error ($!)");
  dbmopen(Pptfs,"$historypath/Cprimeptfhistory",0770) ||
        &log("$lvar -x","$historypath/Cprimeptfhistory open error ($!)");
  dbmopen(Sptfs,"$historypath/Csecondptfhistory",0770) ||
        &log("$lvar -x","$historypath/Csecondptfhistory open error ($!)");
  open (IMPORTFILE,"<$importfile") ||
        &log("$lvar -x","import file $importfile open error ($!)");
  while ($importline=<IMPORTFILE>) {
        chop $importline;
	($token,$field1,$field2)=split(/\|/,$importline);
	if ($token eq SRCDEP) {
		$dephistory{$field1}=$field2;
                if ( $! != 0 ) {
                    &log("$lvar -x","dephistory ($historypath/Cdephistory) write error ($!)");
                }
	}
	elsif ($token eq PRIMEPTF) {
		$Pptfs{$field1}=$field2;
                if ( $! != 0 ) {
                    &log("$lvar -x","Primeptf ($historypath/Cprimeptfhistory) write error ($!)");
                }
	}
        elsif ($token eq SECONDPTF) {
		$Sptfs{$field1}=$field2;
                if ( $! != 0 ) {
                    &log("$lvar -x","Secondptf ($historypath/Csecondptfhistory) write error ($!)");
                }
	}
  }
  dbmclose(ptfs); dbmclose(dephistory); dbmclose(ptfs);
  return $rc;
}

