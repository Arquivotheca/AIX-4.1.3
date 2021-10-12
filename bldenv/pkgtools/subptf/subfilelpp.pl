#! /usr/bin/perl
# @(#)20	1.9  src/bldenv/pkgtools/subptf/subfilelpp.pl, pkgtools, bos412, GOLDA411a 6/30/94 11:03:24
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    subfilelpp
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
do 'bldperllog';      # init perl interface to log functions
do 'bldperlfunc';      # init perl bld functions

#
# NAME: subfilelpp
#
# FUNCTION: Generate subsystem data files.
#
# INPUT: 
#	XREF (parameter) - normalized cross reference of files and their
#                          options.
#	FILENAMES (parameter) - file containing list of filenames to get
#                               subsystem names for.
#	JUNKDIR (parameter) - work directory for subsystem filelists
#
#	SSXREF (files) - files containing list of defect numbers paired
#			    with subsystem for liblpp.a processing.
#
# OUTPUT: 
#	SUBSYSLIST (file) - file containing all subsystems the files cover.
#	filelist (file) - file containing all filenames processed.
#	newinsdefects (file) - AA mapping inslist filename to its defect(s).
#
# FORMAT:
#	SSXREF - "<defectNumber>|<subsystem>" per line
#	FILENAMES - one filename per line
#	XREF - "<filename> <subsystem>" per line
#	SUBSYSLIST - one subsystem per line
#	filelist - one filename per line
#	newinsdefects <defect>, ...  <defect>
#
# SIDE EFFECTS: output files are created
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename
&logset("-c $COMMAND +l");
%SIG=('HUP','Abort','INT','Abort','QUIT','Abort',
      'TERM','Abort','ABRT','Abort');
(defined $ENV{'DEBUG'}) && &log("-b","entering (@ARGV)");

## read parameters
($#ARGV == 2) || &log("-x","illegal syntax ($COMMAND @ARGV)");
$XREF=$ARGV[0]; 
$FILENAMES=$ARGV[1];
$JUNKDIR=$ARGV[2];

chop ($LPPSDIR=`ksh bldupdatepath`);
chop ($globalpath=`ksh bldglobalpath`);

$SUBSYSLIST="$globalpath/subsyslist";
$BLDCYCLE=$ENV{'BLDCYCLE'};

`rm -f $globalpath/newinsdefects*`;

open (FILENAMES,"<$FILENAMES") || 	
	&log("-x","cannot open $FILENAMES ($!)");
open (SUBSYSLIST,">$SUBSYSLIST") ||
	&log("-x","cannot open $SUBSYSLIST ($!)");

&Getoptions($XREF,*fileoptions,*alloptions);  ## get list of options
&Getlpps(*alloptions,*optionlpps);  ## get list of LPPs
&Initfiles($LPPSDIR,*optionlpps,$BLDCYCLE,$JUNKDIR); ## init/cleanup subdirectories

while ($filename=<FILENAMES>) {  # for each ship/update filename
	chop $filename;
	$ignorerc=&IgnoreOptions($filename,*fileoptions);
	# liblpp.a always appears, must determine if liblpp.a appears because
	# of rebuild or not.
	if ($filename =~m#/liblpp.a$#) {
		next if (! &Doliblpp($filename,*fileoptions,*alloptions,*liblppSubsys,*ssDefects));
		#  Set the actual options used for the liblpp.a
		$fileoptions{$filename} = $liblppSubsys{$filename};
	}
	$refname = $filename =~m#/inst_updt/# ? 
		&Domainlib($filename) : $filename;
	if ($fileoptions{$refname} ne "") {
		chop ($foptions=$fileoptions{$refname}); 
		
		foreach $subsystem (split(/$SEP/,$foptions)) {
			$lppdir = "$JUNKDIR/$subsystem";
			$filelpp{$filename} .= $subsystem . $SEP;
			$subsystemoutput{$subsystem} = $DEFINED;
			if (! -d "$lppdir") {  # if directory not created
				mkdir("$lppdir",0775) ||
				      &log("-x","cannnot create $lppdir ($!)");
			}
			if (! $filealreadyoutput{$filename,$subsystem}) {
				open (FILELIST,">>$lppdir/filelist") || 
					&log("-x","cannot open $FILELIST ($!)");
				$! = 0;
				print FILELIST "$filename\n";
				if ( $! != 0 ) {
				    &log("-x","$lppdir/filelist write error ($!)");
				}
				close (FILELIST);
				$filealreadyoutput{$filename,$subsystem} = $DEFINED;
			}
		}
	}
	elsif (! (&IgnoreUpdatefileError($filename) || $ignorerc)) {
		&log("-e","$filename not found in inslist");
	}
}

#  Sort the subsystem names.
#  The subptfpkg routine requires that the subsystems for an lpp be 
#  processed contiguously.

foreach $subsystem (sort keys %subsystemoutput) {
	print SUBSYSLIST "$subsystem\n";
	if ( $! != 0 ) {
	    &log("-x","$SUBSYSLIST write error ($!)");
	}
}

&Verifyfilelpp(*filelpp);  # confirm file is in only one subsystem.

(defined $ENV{'DEBUG'}) && &log("-b","exiting");
&logset("-r");
exit $rc;

#
# NAME: Getoptions
#
# FUNCTION: Generate a list of all subsystems associated with each file.
#
# INPUT: XREF - normalized cross reference of files and their options.
#
# OUTPUT: fileoptions - array of subsystems indexed by filename.
#         alloptions  - array indexed by subsystem
#
# FORMAT:
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#
sub Getoptions {
	local ($XREF,*fileoptions,*alloptions) = @_;
	local ($filename,$option);
	open (XREF, "<$XREF") || &log("-x","unable to open $XREF ($!)");
	while (<XREF>) {	# for each input line
		chop;
		($filename,$subsystem) = split(/ /);
		$fileoptions{$filename} .= $subsystem . $SEP;
		$alloptions{$subsystem} = $subsystem;
	}
}

#
# NAME: Getlpps
#
# FUNCTION: Generate a list of all lpps based upon options.
#
# INPUT: alloptions - array indexed by options.
#
# OUTPUT: optionlpps - array of lpps indexed by options.
#
# FORMAT:
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#
sub Getlpps {
	local (*alloptions,*optionlpps) = @_;
	local ($option,@fields);
	foreach $option (keys %alloptions) {
		@fields = split('\.',$option);
		$optionlpps{$option} = $fields[0];
	}
}

#
# NAME: Initfiles
#
# FUNCTION: Clean up directories of each lpp before continuing
#
# INPUT: LPPSDIR - upper level of directory structure to begin at.
#        optionlpps - array of lpps indexed by options.
#
# OUTPUT:
#
# FORMAT:
#
# SIDE EFFECTS: Removal of filelist,defects for each lpp.
#               Removal of any subsystem directories and contents.
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#
sub Initfiles {
	local ($LPPSDIR,*optionlpps,$BLDCYCLE,$JUNKDIR) = @_;
	local ($lpp);

        #  Remove old subsystems work directory if it exists.
        #  And create anew.

	`rm -fr $JUNKDIR`;       # remove any old subsystems
        #  Make the subsystem working directory.
	mkdir("$JUNKDIR",0775) ||
             &log("-x","cannnot create $JUNKDIR ($!)");

	foreach $lpp (values %optionlpps) {
		if (($lpp ne "") && (! $cleanfilelist{$lpp})) {
			#
			# Assure that the lpp directory exists.
			#
			$lppdir = "$LPPSDIR/$lpp";
			if (! -d "$lppdir") {  # if directory not created
				mkdir("$lppdir",0775) ||
				      &log("-x","cannnot create $lppdir ($!)");
			}
			`rm -f $LPPSDIR/$lpp/filelist.$BLDCYCLE`;  # remove old filelist
			`rm -f $LPPSDIR/$lpp/ptf_pkg.$BLDCYCLE`;   # remove old ptf_pkg
			$cleanfilelist{$lpp} = $DEFINED;
		}
	}
}

#
# NAME: Verifyfilelpp
#
# FUNCTION: Insure file is in only one subsystem.
#
# INPUT: filelpp - array of subsystems indexed by filenames.
#
# OUTPUT:
#
# FORMAT:
#
# SIDE EFFECTS: Warning message if file exists in multiple subsystems.
#               No message for liblpp.a files.
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#
sub Verifyfilelpp {
	local (*filelpp) = @_;
	local ($file,$lppitem,$uniqlist,@lpps);
	foreach $file (keys %filelpp) {
		next if ($file =~m#/liblpp.a$#);
		chop ($lppitem = $filelpp{$file});
		$uniqlist = &Uniquesort($SEP,$lppitem);
		@lpps = split(/$SEP/,$uniqlist);
		if ($#lpps != 0) {
			&log("-w","$file in multiple subsystems ($uniqlist)");
		}
	}
}

#
# NAME: Domainlib
#
# FUNCTION: Generate library name.  Strip '/inst_updt' and trailing .o
#           file names from filename supplied.
#
# INPUT: path - Path of library name, can include object file members.
#
# OUTPUT: path - Corrected path of library name.
#
# FORMAT:
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: Converted library name.
#
sub Domainlib {
        local ($path) = @_;
        $path =~ s#/inst_updt/#/#;  # remove inst_updt subpath
        $path =~ s#/[^/]*$##;  # remove trailing .o filename
        return ($path);
}

#
# NAME: Doliblpp
#
# FUNCTION: Determine if $path was rebuilt as a result of defects.
#
# INPUT: path - update file.
#
# OUTPUT:
#
# FORMAT:
#
# SIDE EFFECTS: The subsystems for the liblpp.a file will be determined
#               from the SSXREF information.
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 if no changes can be found for $path to build,
#          1 if changes are found for $path to build.
#
sub Doliblpp {
	local ($path,*fileoptions,*alloptions,*liblppSubsys,*ssDefects) = @_;
	local (@releases,@defects,@changefiles,%newinsdefects);
	local (%dfiles,$newfile,$changefile_exist,@nfiles,*filedefects);
	local ($file,$useddefect,@useddefects,$used,$depend,$cfile);
	local ($fdefect,@fdefects,$defectnumber,$defectrelease,$dpath,$lppss);
	$changefile_exist=$FALSE;
	($lpp) = split('\.', $fileoptions{$path});
	dbmopen(newinsdefects,"$globalpath/newinsdefects",0750) ||
		&log("-x","unable to dbmopen $globalpath/newinsdefects ($!)");
	&Sourcereleasesanddefects($path,*releases,*defects);
	
#       For each defect input in the SSXREF file which caused this liblpp.a
#       to build, assure that the liblpp.a file is put into the filelist 
#       for the associated subsystem(s).  Put the associated defect(s) into
#       the $newinsdefects array.  Use the pathname prefixed by the subsystem
#       as the index since there may be a different set of defects for each
#       subsystem.  

	    foreach $defect (@defects) {
		($defectrelease,$defectnumber) = split(/\./,$defect);
		if ($done_ssXREF{$defectrelease} eq undef) {
		    $done_ssXREF{$defectrelease} = $DEFINED;   
		    $SSXREF="$globalpath/$defectrelease/ssXREF";
		    open (SSXREF,"<$SSXREF") ||
		        &log("-w","cannot open $SSXREF ($!)");
                    #
                    #  Build an associative array relating the subsystem(s)
                    #  to the defect number from the SSXREF input file.
                    #

                    while ($inline=<SSXREF>) {  # for each input line
                            chop $inline;
                            ($defectNum, $ss) = split(/\|/,$inline);
                            $ssDefect = $defectrelease . "." . $defectNum;
                            if ($ssdefgot{$ss,$ssDefect} eq undef) {
                                $ssDefects{$ssDefect} .= $ss . $SEP;
                                $ssdefgot{$ss,$ssDefect} = $DEFINED;
                            }
                    }
		    close (SSXREF);
		}
		foreach $ss (split($SEP, $ssDefects{$defect})) {
		    ($lppss) = split('\.', $ss);
		    next if ($lppss ne $lpp);
		    $liblppSubsys{$path} .= $ss . $SEP;
		    $changefile_exist=$TRUE;
		    $dpath = $ss . $path;
		    if ($newinsdefects{$dpath} eq undef) {
			$newinsdefects{$dpath} = $defect;
		    }
		    else {
			$newinsdefects{$dpath} .= $SEP . $defect;
		    }
            	    if ( $! != 0 ) {
			&log("-x","$newinsdefects write error ($!)");
            	    }
		}
            }
        
	dbmclose(newinsdefects);
        return ($changefile_exist);
}

#
# NAME: Sourcereleaseanddefects
#
# FUNCTION: Generate a array of releases and a array of defects from the
#           bldinfo database for the file supplied.
#
# INPUT: path - path of file
#
# OUTPUT: releases - normal array of releases from bldinfo database.
#         defects - normal array of defects from bldinfo database.
#
# FORMAT:
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0
#
sub Sourcereleasesanddefects {
	local ($path,*releases,*defects) = @_;
	local ($TMPDIR,$tmpfile,$unused,$defectlist,$arelease);
	local (%defects,%release,$defect);
	chop ($TMPDIR=`ksh bldtmppath`); $tmpfile="$TMPDIR/subfilelpp.sd$$";
	open(TMPFILE,">$tmpfile") || &log("-x","$tmpfile open error ($!)");
	print TMPFILE "$path\n"; close(TMPFILE);
	open (UPDATES,"bldinfo GETUP $tmpfile |") || 
		&log("-x","unable to get update file data ($!)");
	while (<UPDATES>) {
		chop; ($unused,$defectlist,$unused) = split('\|');
		foreach (split(/$SEP/,$defectlist)) {
			($arelease,$unused) = split(/\./);
			$releases{$arelease} = $DEFINED;
			$defects{$_} = $DEFINED;
		}
	}
	push (@releases,(keys %releases));
	push (@defects,(keys %defects));
	`rm -f $tmpfile`;
}




#
# NAME: IgnoreOptions
#
# FUNCTION: Remove any unwanted options associated with an update file
#
# INPUT: filename - update file
#        fileoptions - list of subsystems associated with the update file
#
# OUTPUT: 
#
# FORMAT:
#
# SIDE EFFECTS: 
#
# EXECUTION ENVIRONMENT: n/a
#
# RETURNS: 0 if option is not ignored, 1 if option is ignored.
#
sub IgnoreOptions {
	local ($filename,*fileoptions) = @_;  ## input parameters

	local ($subsystem,$option,$newlist,$rc); $rc=0;
	if ($filename =~ m/microcode/) {  ## if microcode file
		## for each microcode options
		foreach $subsystem (split(/$SEP/,$fileoptions{$filename})) {
			# pull off last token (subsystem) to get real option.
			$option = $subsystem;
			$option =~ s/(^.*)\..*$/$1/;
			## keep the non-microcode options; discard mc options
			if ($option !~ m/\.mc$/) {
				$newlist .= $subsystem . $SEP;
			}
                	else {
				$rc=1; ## option is ignored
			}
		}
		$fileoptions{$filename} = $newlist;
	}
	undef $newlist;
	foreach $subsystem (split(/$SEP/,$fileoptions{$filename})) {
		# pull off last token (subsystem) to get real option.
		$option = $subsystem;
		$option =~ s/(^.*)\..*$/$1/;
                ## this is where the msg LPP files are ignored
                ## if not message LPP (e.g. X11mFr_FR) or BLDMSGCAT is set
                if ($option !~ m/m.._..?.?\.msg$/ ||
                    defined $ENV{"BLDMSGCAT"}) {
			$newlist .= $subsystem . $SEP;
                }
                else {
			$rc=1; ## option is ignored
		}
        }
	$fileoptions{$filename} = $newlist;
        return $rc;
}

#
# NAME: IgnoreUpdatefileError
#
# FUNCTION: Indicate if an error for an update file should be ignored
#
# INPUT: file - the update file
#
# RETURNS: 1 if error should be ignored; 0 if not ignored
#
sub IgnoreUpdatefileError {
        local ($file) = @_;
        return ($file =~ m/lpp_name$/);
}

sub Abort {
	&logset("-r");
        exit $FATAL;
}

