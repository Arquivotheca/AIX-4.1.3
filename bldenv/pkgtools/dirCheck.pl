#! /usr/local/bin/perl
# @(#)66	1.3  src/bldenv/pkgtools/dirCheck.pl, pkgtools, bos412, GOLDA411a 3/4/94 14:55:00
#
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS: 	ilverify
#               CheckLine
#               CheckDup
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

#
# NAME: ilverify
#
# FUNCTION: Check for duplicate or inconsistent files in inslists.
#
# INPUT: inslists (file) - A file containing the names of all inslists to check.
#        except   (file) - A file containing the names which may be duplicated.
#        database (file) - A file containing archived set of inslists.
#        except or database name = "N"  => none specified.
#
# OUTPUT: none
#         Error messages are issued for any detected problems.
#         Error messages are NOT issued for format problems because these
#         are handled elsewhere.
#
# FORMATS:
#	inslists: inslist_file_name (one per line)
#	except:   path names (one per line)
#	database: An inslist identifier   (#inslist_name)
#                 followed by the inslist lines
#                 Multiple inslists will be in the database.
#	inslist:  (various line formats)
#         See bos.rte.il for format description
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure) or 2 (fatal)
#
$rc=$SUCCESS;
$COMMAND=$0; $COMMAND =~ s#.*/##;  # save this command's basename

if ($#ARGV != 2) {
        print STDERR ("$COMMAND: Usage:  $COMMAND il_list except_list database\n");
        print STDERR ("\t $COMMAND checks the given inslists for duplicate definitions\n");
        print STDERR ("\t and for the prior definition of requisite paths\n");
        print STDERR ("\t (not performed if database specified).\n");
        print STDERR ("\t \"N\" may be specified for except_list or database\n");
        print STDERR ("\t to indicate that none is to be used.\n\n");
        print STDERR ("\t il_list is a file containing the names of inslists to process.\n");
        print STDERR ("\t except_list is a file containing the names\n");
        print STDERR ("\t of paths not to be listed for duplication error.\n");
        print STDERR ("\t database is a file containing a database of inslists\n");
        print STDERR ("\t which will be used for duplicate path checking.\n");
        exit $FATAL;
    
}

($illist, $except, $database) = @ARGV;

#  Define some constants for readability.

$NONE = "N";
$DEFINED=1;   
$SUCCESS=0;
$FAILURE=1;
$FATAL=2;
$TRUE=1;
$FALSE=0;

$inscode = 0;

#  Define the valid inslist codes and set value for how to process them.

$interpret{"F"}  = 1;
$interpret{"FT"} = 1;
$interpret{"D"}  = 2;
$interpret{"B"}  = 1;
$interpret{"C"}  = 1;
$interpret{"I"}  = 1;
$interpret{"N"}  = 1;
$interpret{"A"}  = 1;
$interpret{"AT"} = 1;
$interpret{"V"}  = 1;
$interpret{"VT"} = 1;
$interpret{"f"}  = 1;
$interpret{"ft"} = 1;
$interpret{"d"}  = 2;
$interpret{"b"}  = 1;
$interpret{"c"}  = 1;
$interpret{"i"}  = 1;
$interpret{"n"}  = 1;
$interpret{"a"}  = 1;
$interpret{"at"} = 1;
$interpret{"v"}  = 1;
$interpret{"vt"} = 1;
$interpret{"H"}  = 3;
$interpret{"h"}  = 3;
$interpret{"S"}  = 4;
$interpret{"s"}  = 4;

if ($database ne $NONE) {
    $rpath = " and requisite paths";
}
                              
print STDERR("\n$COMMAND beginning.\n");
print STDERR("  Checking inslists for duplicate paths$rpath.\n");
print STDERR("  inslist list   = $illist.\n");
print STDERR("  exception list = $except.\n");
print STDERR("  database file  = $database.\n\n");

# Position to first inslist in database, if any.

if ($database ne $NONE) {
    if (!open (DATABASE,"<$database"))  {
        print STDERR ("FATAL: unable to open database file $database ($!)\n");
        exit $FATAL;
    }
    while ($curdb=<DATABASE>) {
        chop $curdb;
        next if ($curdb eq undef);         #  Ignore blank lines
        next if ($curdb =~ m/ *#/);        #  Ignore comment
#print ("First database line $curdb\n");   #  DEBUG
        if ($curdb !~ m/^ *!!.*$/) {
            print STDERR ("FATAL: unrecognized data in database file.\n");
            print STDERR ("$curdb\n");
            exit $FATAL;
        }
                                               
        $curdb =~ s/^ *!!(.*)/$1/;         #  Remove !! identifier
	$curbase = $curdb;                 
        $curbase =~ s#^.*/(\S*).*$#$1#;       #  Get the basename
#print ("First curdb $curdb\n");           #  DEBUG
        last;
    }
    if ($curdb eq undef) {
        print STDERR ("FATAL: unable to determine inslist filenames from database file.\n");
        exit $FATAL;
    }
}


if ($except ne $NONE) {
    if (!open (EXCEPT,"<$except"))  {
        print STDERR ("FATAL: unable to open exception file ($!)\n");
        exit $FATAL;
    }
#print ("Read exception file $except\n");           #  DEBUG
    while ($expath=<EXCEPT>) {
          chop $expath;
          $expath =~ s/^ *(.*)/$1/;         #  Strip any leading blanks
          next if ($expath eq undef);       #  Ignore blank lines
          $exception{$expath} = $DEFINED;
    }
}

if (!open (INSLISTS,"<$illist"))  {
    print STDERR ("FATAL: unable to open input file $illist ($!)\n");
    exit $FATAL;
}

#  Process all of the inslists in illist

while ($inslist=<INSLISTS>) {
      chop $inslist;
      next if ($inslist eq undef);  #  Skip blank line

      #  Get the basename of the inslist.
      $insbase = $inslist;
      $insbase =~ s#^.*/(\S*).*$#$1#;

#print ("insbase $insbase\n");           #  DEBUG

      #  Check if corresponding database inslist has already been processed.

      print STDERR("\nProcessing inslist $inslist.\n");
      if ($insproc{$insbase} ne undef) {
            print STDERR ("  WARNING:  An inslist file with basename of $inslist already processed.\n");
            $rc = $FAILURE;
      }
      
      #  Encode the inslist file name to save some space.
      $inscode = $inscode + 1;
      $insname{$inscode} = $inslist;
      $insproc{$insbase} = $DEFINED;
      $listname = "INPUT " . $inslist;

      #  Read the inslist.
      
      if (!open (INSLIST,"<$inslist")) {
            print STDERR ("  ERROR: unable to open inslist file $inslist ($!)\n");
            $rc = $FAILURE;
            next;
      }
      while ($inline=<INSLIST>) {
          chop $inline;
          &CheckLine($inline);
      }
      close (INSLIST);
}
close (INSLISTS);




#  Process inslists from the database, skipping any that were
#  done above.

if ($curdb ne undef) {
    #  If the first database inslist, found above, was in the
    #  input list, set skip flag
    
    if ($insproc{$curbase} ne undef) {
        $skip = $TRUE;
    }
    else {
        $skip = $FALSE;
        print STDERR("DATABASE inslist $curdb.\n");
        #  Encode the inslist file name to save some space.
        $inscode = $inscode + 1;
        $insname{$inscode} = $curdb;
        $insproc{$curbase} = $DEFINED;
        $listname = "DATABASE " . $curdb;
    }

    while ($inline=<DATABASE>) {
        chop $inline;
        next if ($inline eq undef);     #  Ignore blank lines
        next if ($inline =~ m/ *#.*/);  #  Ignore comment lines
        
        #  Check for start of new inslist
        if ($inline =~ m/^ *!!.*$/) {
            $curdb = $inline;
            $curdb =~ s/^ *!!(.*)/$1/;
            $curbase = $curdb;
            $curbase =~ s#^.*/(\S*).*$#$1#;       #  Get the basename
#print ("New curdb $curdb found.\n");           #  DEBUG

            #  Skip if inslist processed earlier.

            if ($insproc{$curbase} ne undef) {
#print ("Skipping this database inslist.\n");           #  DEBUG
                $skip = $TRUE;
            }
            else {
                $skip = $FALSE;
                print STDERR("DATABASE inslist $curdb.\n");
                #  Encode the inslist file name to save some space.
                $inscode = $inscode + 1;
                $insname{$inscode} = $curdb;
                $insproc{$curbase} = $DEFINED;
                $listname = "DATABASE " . $curdb;
            }
        }
        
        if ($skip == $FALSE) {
            &CheckLine($inline);
        }

    }
}

exit $rc;


#
# NAME: CheckLine
#
# FUNCTION: Check the inslist line.
#
# INPUT: inline - The line to check.
#
# OUTPUT: none
#         Error message is issued if problem recognized.
#
# FORMATS:
#
# SIDE EFFECTS: Adds the path to the appropriate associative array
#               if it is not a duplicate.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#

sub CheckLine {
        local ($inline) = @_;
        local ($linecode,$uid,$gid,$perm,$fname,$junk);
        local ($rc,$code,$linkname,$paths,@paths);
	$rc = $SUCCESS;
        return ($rc) if ($inline eq undef);  #  Skip blank line
        ($linecode, $uid, $gid, $perm, $fname, $junk) = split(' ',$inline);
        return ($rc) if ($linecode =~ m/^#/);  #  No comment

        if ($interpret{$linecode} eq undef) {
               $rc = $FAILURE;
               return ($rc);
        }
        $code = $interpret{$linecode};

        if ($code <= 2) {
            #  Check for correct number of parameters. (There should be at least 5 for files)
            if ($fname eq undef) {
               $rc = $FAILURE;
               return ($rc);
            }

            #  See if name already appeared.
            &CheckDup($fname);
        }
        else {   #  Hard or symbolic link statement.
            ($linecode, $uid, $gid, $perm, $linkname, $fname) = split(' ',$inline);

            #  Check for correct number of parameters. (There should be at least 6 for links)
            if ($fname eq undef) {
               $rc = $FAILURE;
               return ($rc);
            }

            #  Check for repeated path name
            if (! &CheckDup($linkname)) {
               $rc = $FAILURE;
            }
        }
	return ($rc);
}
#
# NAME: CheckDup
#
# FUNCTION: Check for repeated pathname.
#
# INPUT: path - The pathname to check.
#
# OUTPUT: none
#         Error message is issued if path was previously specified.
#
# FORMATS:
#
# SIDE EFFECTS: Adds the path to the appropriate associative array
#               if it is not a duplicate.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#

sub CheckDup {
        local ($path) = @_;
        local ($repeat,$ins1,$ins1code,$oldlink,$retnOK);
        $retnOK = $FALSE;

        #  See if the pathname previously appeared.

        if ($files{$path} ne undef) {
           $ins1code = $files{$path};
           $repeat = "filename";
        }
        elsif ($dir{$path} ne undef) {
           ($ins1code, $ouid, $ogid, $operm) = split(' ',$dir{$path});
           if (($ouid != $uid) || ($ogid != $gid) || ($operm != $perm)) {
               $repeat = "directory with different attributes";
           }
        }
        elsif ($hardlink{$path} ne undef) {
           $ins1code = $hardlink{$path};
           #  Repeated link OK if it is to the same directory.
           $repeat = "hardlink";
        }
        elsif ($symlink{$path} ne undef) {
           $ins1code = $symlink{$path};
           #  Repeated link OK if it is to the same directory.
           $repeat = "symbolic link";
        }

        if ($repeat ne undef) {
           #  If repeated path name is in the exceptions list, it is OK.
           if ($exception{$path} eq undef) {
               $ins1 = $insname{$ins1code};
               print STDERR ("  ERROR: Path $path\n\tpreviously appeared as $repeat in inslist\n\t$ins1\n");
           }
           else {
               $retnOK = $TRUE;
           }
        }
        else {  #  New pathname.  Save in appropriate array.
            $retnOK = $TRUE;
            if ($code == 1) {
               $files{$path} = $inscode;
            }
            elsif ($code == 2) {
               $dir{$path} = $inscode . " " . $uid . " " . $gid . " " . $perm;
            }
            elsif ($code == 3) {
               $hardlink{$path} = $inscode;
            }
            else {
               $symlink{$path} = $inscode;
            }

            #  Check that the earlier path parts have been defined already.
            #  This is bypassed if a database is used because the
            #  order of any new inslists may not yet be determined.
            
            if ($database eq $NONE && $path ne "/") {
               while ($path ne "/") {
                   $path =~ s#(^.*)/.*$#$1#;
                   if ($path eq undef) { $path = "/"; }
                   if ($dir{$path} eq undef && $tattle{$path} eq undef) {
                       print STDERR ("  ERROR: Path part $path\n\thas not been previously defined.\n");
                       $tattle{$path} = $inscode;  #  Tattling once is enough!
                       last;
                   }
               }
            }
        }
        return ($retnOK);
}
