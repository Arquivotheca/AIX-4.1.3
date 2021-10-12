#
# @(#)52	1.9  src/bldenv/pkgtools/subptf/bldinfolib.pl, pkgtools, bos412, GOLDA411a 11/19/92 09:45:49
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS:	BLDINFOsaveupdata
#		BLDINFOsaveprevdata
#		BLDINFOsavereqdata
#		BLDINFOsaveenvdata
#		BLDINFOsavelppgroups
#               BLDINFOgetlppgroups
#		BLDINFOgetupdata
#		BLDINFOgetenvdefects
#		BLDINFOgetfilenames
#		BLDINFOgetallprevdata
#

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

sub BLDINFOinterfaceinit {
  local($workpath,$path,$rc); $rc=$SUCCESS;
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(envdefects,"$path/envdefects",0770) || 
	&log("$lvar -x","$path/envdefects open error ($!)");
  dbmopen(envenvdefects,"$path/envenvdefects",0770) || 
	&log("$lvar -x","$path/envenvdefects open error ($!)");
  (&IOpendecodeddefects($release) == $SUCCESS) ||
        &log("$lvar -x","cannot access bldinfo decoded defects");
  return $rc;
}

#
# NAME: BLDINFOsaveupdata
#
# FUNCTION: Save dependency data for update files
#
# INPUT: updatafile (parameter) - file name of update dependencies
#
# OUTPUT: none
#
# SIDE EFFECTS: AA files (updefects and upenvdefects) updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: updatafile - "file|d d d...|e e e..." where "d" is a defect which
#	caused the file to be built and "e" is a build environment defect 
#	which caused the file to be built.  Each line contains one file entry.
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOsaveupdata {
  local($updatafile) = @_; $rc=$SUCCESS;
  local($workpath,$path,$dataline,$file,$directdefects,$indirectdefects,@dlist);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(updefects,"$path/updefects",0770) || 
	&log("$lvar -x","$path/updefects open error ($!)");
  dbmopen(upenvdefects,"$path/upenvdefects",0770) || 
	&log("$lvar -x","$path/upenvdefects open error ($!)");
  open (UPDATAFILE,"<$updatafile") || 
	&log("$lvar -x","updatafile ($updatafile) open error ($!)");
  (&IOpendecodeddefects($release) == $SUCCESS) ||
        &log("$lvar -x","cannot access bldinfo decoded defects");


  while ($dataline=<UPDATAFILE>) {
	chop $dataline;
	($file,$directdefects,$indirectdefects) = split(/\|/,$dataline);
	## compact defect numbers before saving
	@dlist=split(/$SEP/,$directdefects);  
	grep($_=&IEncodedefect($_),@dlist);
	$updefects{$file}=join($SEP,@dlist);
        if ( $! != 0 ) {
            &log("$lvar -x","updefects ($path/updefects) write error ($!)");
        }
	## compact defect numbers before saving
	@dlist=split(/$SEP/,$indirectdefects);
	grep($_=&IEncodedefect($_),@dlist);
	$upenvdefects{$file}=join($SEP,@dlist); 
        if ( $! != 0 ) {
            &log("$lvar -x","upenvdefects ($path/upenvdefects) write error ($!)");
        }
  }
  dbmclose(updefects); dbmclose(upenvdefects); close(UPDATAFILE);
  return $rc;
}

#
# NAME: BLDINFOsaveprevdata
#
# FUNCTION: Save previous dependency data for update files
#
# INPUT: prevdatafile (parameter) - file name of previous-defect dependencies
#
# OUTPUT: none
#
# SIDE EFFECTS: AA file (upprev) updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: prevdatafile - "file|p p p..." where "p" is a previous defect
#	the file depends on.  Each line contains one entry.
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOsaveprevdata {
  local($prevdatafile) = @_; $rc=$SUCCESS;
  local($workpath,$path,$dataline);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  open (PREVDATAFILE,"<$prevdatafile") || 
	&log("$lvar -x","prevdatafile ($prevdatafile) open error ($!)");
  open (UPPREV,">>$path/upprev") || 
	&log("$lvar -x","upprev file ($path/upprev) open error ($!)");

  while ($dataline=<PREVDATAFILE>) {
	print UPPREV $dataline;
        if ( $! != 0 ) {
            &log("$lvar -x","upprev ($path/upprev) write error ($!)");
        }
  }

  close(UPPREV); close(PREVDATAFILE); 
  return $rc;
}

#
# NAME: BLDINFOsavereqdata
#
# FUNCTION: Save requisite data for update files
#
# INPUT: reqdatafile (parameter) - file name of requisite dependencies
#
# OUTPUT: none
#
# SIDE EFFECTS: AA file (upreq) updated
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: reqdatafile - TBD
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOsavereqdata {
  local($reqdatafile) = @_; $rc=$SUCCESS;
  local($workpath,$path,$dataline,$file,$reqdefects);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(upreq,"$path/upreq",0770) || 
	&log("$lvar -x","$path/upreq open error ($!)");
  open (REQDATAFILE,"<$reqdatafile") || 
	&log("$lvar -x","reqdatafile ($reqdatafile) open error ($!)");

  while ($dataline=<REQDATAFILE>) {
	chop $dataline;
	($file,$reqdefects) = split(/\|/,$dataline);	## TBD
	$upreq{$file}=$reqdefects;		## TBD
        if ( $! != 0 ) {
            &log("$lvar -x","upreq ($path/upreq) write error ($!)");
        }
  }

  dbmclose(upreq); close(REQDATAFILE); 
  return $rc;
}

#
# NAME: BLDINFOsaveenvdata
#
# FUNCTION: Save dependency data for build environment files
#
# INPUT: envdatafile (parameter) - file name of update dependencies
#
# OUTPUT: none
#
# SIDE EFFECTS: AA files (envdefect and envenvdefects) updated.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: envdatafile - "file|d d d...|e e e..." where "d" is a defect which
#	caused the file to be built and "e" is a build environment defect 
#	which caused the file to be built.  Each line contains one file.
#
#	### TBD - maybe add previous defects or create separate access ###
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOsaveenvdata {
  local($envdatafile) = @_; $rc=$SUCCESS;
  local($workpath,$path,$dataline,$file,$directdefects,$indirectdefects,@dlist);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(envdefects,"$path/envdefects",0770) || 
	&log("$lvar -x","$path/envdefects open error ($!)");
  dbmopen(envenvdefects,"$path/envenvdefects",0770) || 
	&log("$lvar -x","$path/envenvdefects open error ($!)");
  open (ENVDATAFILE,"<$envdatafile") || 
	&log("$lvar -x","envdatafile ($envdatafile) open error ($!)");
  (&IOpendecodeddefects($release) == $SUCCESS) ||
        &log("$lvar -x","cannot access bldinfo decoded defects");


  while ($dataline=<ENVDATAFILE>) {
	chop $dataline;
	($file,$directdefects,$indirectdefects) = split(/\|/,$dataline);
        $file =~ s#.*@@#!!#; # save bldenv name in tpath format
                             # other releases will reference name in tpath form
	@dlist=split(/$SEP/,$directdefects);  
	grep($_=&IEncodedefect($_),@dlist);
	$envdefects{$file}=join($SEP,@dlist);
        if ( $! != 0 ) {
            &log("$lvar -x","envdefects ($path/envdefects) write error ($!)");
        }
	@dlist=split(/$SEP/,$indirectdefects);  
	grep($_=&IEncodedefect($_),@dlist);
	$envenvdefects{$file}=join($SEP,@dlist);
        if ( $! != 0 ) {
            &log("$lvar -x","envenvdefects ($path/envenvdefects) write error ($!)");
        }
  }

  dbmclose(envdefects); dbmclose(envenvdefects); close(ENVDATAFILE);
  return $rc;
}

#
# NAME: BLDINFOsavelppgroups
#
# FUNCTION: Copy data from ptfgroups file to specific lpp file.
#
# INPUT: lppname - lpp to use.
#        ptfgroups - name of file to copy from.
#
# OUTPUT: none
#
# SIDE EFFECTS: bldinfo files allppptfs and lpp files updated.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: lppfile - "ptf|d,d,...,d|e,e,...,e" where "d" is a defect which
#       caused the file to be built and "e" is a build environment defect
#       which caused the file to be built.  Each line contains one file.
#       The format of "d" and "e" is 'release.defectid'.
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOsavelppgroups {
  local($lppname,$ptfgroups) = @_; $rc=$SUCCESS;
  local($path,$dataline);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  open (PTFGROUPS,"<$ptfgroups") || 
	&log("$lvar -x","ptfgroups ($ptfgroups) open error ($!)");
  open (LPPFILE,">$path/$lppname") || 
	&log("$lvar -x","lppfile ($path/$lppname) open error ($!)");
  open (ALLFILE,">>$path/alllppptfs") || 
	&log("$lvar -x","all-ptfs file ($path/alllppptfs) open error ($!)");

  while ($dataline=<PTFGROUPS>) { 
	print LPPFILE $dataline; 
        if ( $! != 0 ) {
            &log("$lvar -x","lppfile ($path/lppname) write error ($!)");
        }
	print ALLFILE $dataline;
        if ( $! != 0 ) {
            &log("$lvar -x","allfile ($path/alllppptfs) write error ($!)");
        }
  }

  close (PTFGROUPS); close (LPPFILE);
  return $rc;
}

#
# NAME: BLDINFOgetlppgroups
#
# FUNCTION: Read data from lpp file.
#
# INPUT:  lppname - lpp name.  alllppptfs is file containing data from all
#         lpps (it is not a lpp itself), handled in the same manner as lpps
#         except for error message.
#
# OUTPUT: none
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: lppfile - "ptf|d,d,...,d|e,e,...,e" where "d" is a defect which
#         caused the file to be built and "e" is a build environment defect
#         which caused the file to be built.  Each line contains one file.
#         The format of "d" and "e" is 'release.defectid'.
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOgetlppgroups {
  local($lppname) = @_; $rc=$SUCCESS;
  local($path,$dataline,$type);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  if (! open(LPPFILE,"<$path/$lppname"))
  {
     $lppname eq "alllppptfs" ? $type="all-ptfs" : $type="lppfile";
     &log("$lvar -x","$type ($path/$lppname) open error ($!)");
  }

  while ($dataline=<LPPFILE>) { 
	print $dataline;
        if ( $! != 0 ) {
            &log("$lvar -x","getlppgroups write error ($!)");
        }
  }

  close (LPPFILE);
  return $rc;
}

#
# NAME: BLDINFOgetupdata
#
# FUNCTION: Return update dependency data for a specified file
#
# INPUT: namesfile (parameter) - path name of file's data to be returned
#
# OUTPUT: filepath|d,d,d,d,d,d,...|i,i,i,i...
#	where 'd' is a direct defect and 'i' is an indirect defect.
#	One entry is printed per line.  The format of d and i is
#       'release.defectid'.
#
# NOTES: AA files (updefects and upenvdefects) read.
#
# SIDE EFFECTS: None
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOgetupdata {
  local($namesfile) = @_; $rc=$SUCCESS;
  local($workpath,$filename,$path);
  local($directcodes,$indirectcodes,$directdefects,$indirectdefects,@dlist);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  open (NAMESFILE,"<$namesfile") || 
	&log("$lvar -x","namesfile ($namesfile) open error ($!)");
  &log("$lvar -x", "namesfile ($namesfile) is not plain file") 
	unless -f $namesfile;
  dbmopen(updefects,"$path/updefects",0770) || 
	&log("$lvar -x","$path/updefects open error ($!)");
  dbmopen(upenvdefects,"$path/upenvdefects",0770) || 
	&log("$lvar -x","$path/upenvdefects open error ($!)");
  (&IOpendecodeddefects($release) == $SUCCESS) ||
        &log("$lvar -x","cannot access bldinfo decoded defects");

  while ($filename=<NAMESFILE>) {
        chop $filename;
	$directdefects=""; $indirectdefects="";
  	$directcodes=$updefects{$filename};
  	$indirectcodes=$upenvdefects{$filename};
	next if ($directcodes eq undef && $indirectcodes eq undef);
	if ($directcodes ne undef) {
	  	@dcodes=split(/$SEP/,$directcodes);
  		grep($_=&IDecodedefect($_),@dcodes);
		$directdefects=join($SEP,@dcodes);
	}
	if ($indirectcodes ne undef) {
  		@dcodes=split(/$SEP/,$indirectcodes);
  		grep($_=&IDecodedefect($_),@dcodes);
		$indirectdefects=join($SEP,@dcodes);
	}
  	print "$filename|$directdefects|$indirectdefects\n";
        if ( $! != 0 ) {
            &log("$lvar -x","getupdata write error ($!)");
        }
  }

  dbmclose(updefects); dbmclose(upenvdefects);
  return $rc;
}

#
# NAME: BLDINFOgetenvdefects
#
# FUNCTION: Return dependency defects for a build environment file.
#
# INPUT: file (parameter) - build environment filename (full path)
#
# OUTPUT: list of build defects for new build environment file (one per line)
#
# NOTES: AA files (envdefects and envenvdefects) read.
#
# SIDE EFFECTS: None
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOgetenvdefects {
  local($file) = @_; $rc=$SUCCESS;
  local($workpath,$path,$code,$defect,$rc,%envdefects,%envenvdefects);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(envdefects,"$path/envdefects",0770) || 
	&log("$lvar -x","$path/envdefects open error ($!)");
  dbmopen(envenvdefects,"$path/envenvdefects",0770) || 
	&log("$lvar -x","$path/envenvdefects open error ($!)");
  (&IOpendecodeddefects($release) == $SUCCESS) ||
        &log("$lvar -x","cannot access bldinfo decoded defects");

  foreach $code (split (/$SEP/,$envdefects{$file})) {
	$defect=&IDecodedefect($code);
	print "$defect\n";
        if ( $! != 0 ) {
            &log("$lvar -x","getenvdefects write error ($!)");
        }
  }
  foreach $code (split (/$SEP/,$envenvdefects{$file})) {
	$defect=&IDecodedefect($code);
	print "$defect\n";
        if ( $! != 0 ) {
            &log("$lvar -x","getenvenvdefects write error ($!)");
        }
  }

  dbmclose(envdefects); dbmclose(envenvdefects);
  return $rc;
}
sub BLDINFO2getenvdefects {
  local($file,*defectlist) = @_;
  local($code,$defect);
  if ($envdefects{$file} ne undef) {
  	foreach $code (split (/$SEP/,$envdefects{$file})) {
		$defect=&IDecodedefect($code);
		$defectlist{$defect} = $DEFINED;
  	}
  }
  if ($envenvdefects{$file} ne undef) {
  	foreach $code (split (/$SEP/,$envenvdefects{$file})) {
		$defect=&IDecodedefect($code);
		$defectlist{$defect} = $DEFINED;
  	}
  }
}

#
# NAME: BLDINFOgetreqdata (TBD)
#
# FUNCTION: Return requisite data for an update file
#
# INPUT: file (parameter) - build environment filename
#
# OUTPUT: TBD
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#

#
# NAME: BLDINFOgetfilenames
#
# FUNCTION: Return all update file names.
#
# INPUT: workpath -
#        path -
#        filename -
#        rc -
#
# OUTPUT: update file names (one per line)
#
# NOTES: AA file (updefects) read.
#
# SIDE EFFECTS: None
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOgetfilenames {
  local($workpath,$path,$filename,$rc); $rc=$SUCCESS;
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  dbmopen(updefects,"$path/updefects",0770) || 
	&log("$lvar -x","$path/updefects open error ($!)");
  while (($key,$value) = each %updefects) { 
	print "$key\n"; 
        if ( $! != 0 ) {
            &log("$lvar -x","getfilenames write error ($!)");
        }
  }
  dbmclose(updefects);
  return $rc;
}

#
# NAME: BLDINFOgetallprevdata
#
# FUNCTION: Read filenames and their previous defects.
#
# INPUT: 
#
# OUTPUT: 
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: the build process environment
#
# FORMAT: prevdatafile - "file|p p p..." where "p" is a previous defect
#	the file depends on.  Each line contains one entry.  The format
#       of p is 'release.defectid'.
#
# RETURNS: 0 (successful) or 1 (failure)
#
sub BLDINFOgetallprevdata {
  local($prevdatafile) = @_; $rc=$SUCCESS;
  local($workpath,$path);
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";

  open (UPPREV,"<$path/upprev") || 
	&log("$lvar -x","upprev file ($path/upprev) open error ($!)");

  while (<UPPREV>) { 
	print; 
        if ( $! != 0 ) {
            &log("$lvar -x","getallprevdata write error ($!)");
        }
  }

  close(UPPREV);
  return $rc;
}

sub IOpendecodeddefects {
  local($release) = @_;
  local($workpath,$path,$rc); $rc=$SUCCESS;
  $workpath=`ksh bldglobalpath`; chop $workpath;
  $path="$workpath/bldinfo";
  dbmopen(idecodeddefects,"$path/idecodeddefects",0750) ||
        &log("$lvar -e","$path/idecodeddefects open error ($!)");
  if ($idecodeddefects{"DEFECTCOUNT"} eq undef) {
	$idecodeddefects{"DEFECTCOUNT"} = 0;
        if ( $! != 0 ) {
            &log("$lvar -x","IOpendecodeddefects ($path/idecodeddefects) write error ($!)");
        }
  }
  return $rc;
}

sub IEncodedefect {
  local($defect) = @_;
  if ($iencodeddefects{$defect} eq undef) {
        $iencodeddefects{$defect} = ++$idecodeddefects{"DEFECTCOUNT"};
        $idecodeddefects{$idecodeddefects{"DEFECTCOUNT"}} = $defect;
  }
  return ($iencodeddefects{$defect});
}

sub IDecodedefect {
  local($code) = @_;
  if (($code ne undef) && ($idecodeddefects{$code} eq undef)) {
        &log("$lvar -e","unable to decode bldinfo defect code $code");
  }
  return ($idecodeddefects{$code});
}

