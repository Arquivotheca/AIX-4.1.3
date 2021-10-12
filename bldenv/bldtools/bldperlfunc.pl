#
# @(#)59	1.6  src/bldenv/bldtools/bldperlfunc.pl, bldtools, bos412, GOLDA411a 8/14/93 17:50:54
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldperlfunc
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991,1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

############################################################################
# function:	getopts
# description:	processes command line options
# input:	UNIX command line arguments in ARGV array
# output:	none
# remarks:	
############################################################################
sub getopts {
	# return FALSE (0) if the end of argument list has been reached
	if (! defined $ARGV[$OPTIND]) {
		return(0);
	}

	# initialize OPTARG variable
	$OPTARG = "";

	# get option string and pointer to where current option will be stored
	($optstr,*optcode) = @_;

	# if first character of option string is a ":" then set noerrs mode
	# to TRUE (no error messages printed out); otherwise set noerrs mode
	# to FALSE (error messages printed out)
	$noerrs = 0;
	if (substr($optstr,0,1) eq ":") {
		$noerrs = 1;
	}

	# start parsing option string from first option letter in string (i.e.
	# first character of noerrs is FALSE, or 2nd character if noerrs is
	# true; in other words skip the leading ":" if present)
	for ($i = $noerrs; $i < length($optstr); $i++) {
		# get 2 characters at a time
		$optpair = substr($optstr,$i,2);

		# separate out the 2 characters
		$opt1 = substr($optpair,0,1);
		$opt2 = substr($optpair,1,1);

		# check to see if the first character in the pair is a valid
		# option character 
		if ($opt1 =~ /[a-zA-Z0-9]/) {
			# if the 2nd character is a ":" then set its value in
			# the optarglist assoc. array (i.e. array of all 
			# options for which an argument is required); otherwise
			# set its value in the optnoarglist assoc. array (i.e.
			# array of all options for which an argument is not
			# required
			if ($opt2 eq ":") {
				$optarglist{$opt1} = 1;
				$i++;
			} else {
				$optnoarglist{$opt1} = 1;
			}
		}
	}

	# checking current argument to see if it is preceded by a "-" or "+"
	if (($arg = $ARGV[$OPTIND++]) =~ /^[-+]/) {
	    # assume end of argument list when "--" encountered
	    if ($arg eq "--") {
		return(0);
	    }

	    # initialize "optcode" variable
	    $optcode = "";

	    # get current option code
	    $opt = substr($arg,1,1);

	    # test to see if an argument is required
	    if ($optarglist{$opt} == 1) {
		# argument required; set optcode to current option code
	        $optcode = $opt;

		# is argument separated from option or connected
	        if (length($arg) == 2) {
		    # argument separated form option; testing to see if there
		    # there is another argument defined
		    if (defined $ARGV[$OPTIND]) {
			# is the next argument another option?
			if ($ARGV[$OPTIND] !~ /^[-+]/) {
			    # setting OPTARG to the given argument
			    $OPTARG = $ARGV[$OPTIND++];
			} else {
			    # oops, next argument was another option; printing
			    # error message if noerrs is FALSE
			    if (! $noerrs) {
			        print STDERR "getopts: arg required for ";
			        print STDERR "option: $opt\n";
			    } else {
				# set OPTARG to current option code if 
				# noerrs set
			        $OPTARG = $opt;
			    }
			    # always set optcode to "?" on error
			    $optcode = "?";
			}
		    } else {
			# end of argument list has been reached even though
			# an argument was expected for the current option;
			# printing error if noerrs is not set
			if (! $noerrs) {
		            print STDERR "getopts: arg required for ";
			    print STDERR "option: $opt\n";
			} else {
			    # set OPTARG to current option code if noerrs set
			    $OPTARG = $opt;
			}
			# always set optcode to "?" on error
			$optcode = "?";
		    }
	        } else {
		    # if no white space between option code and argument then
		    # set OPTARG to be equal to the substring of the current
		    # command line argument, starting from the 3rd character
		    # to the end of the string
		    $OPTARG = substr($arg,2,length($arg) - 2);
	        }
	    } elsif ($optnoarglist{$opt} == 1) {
		# if option code does not require an argument then simply
		# set optcode to the current option code
		$optcode = $opt;
	    } else {
		# if the current option code is not contained anywhere in the
		# option string then print an error message if noerrs is not
		# set
		if (! $noerrs) {
		    print STDERR "getopts: unrecognized option: $opt\n";
		} else {
		    # set OPTARG to the current option code if noerrs is set
		    $OPTARG = $opt;
		}
		# always set optcode to "?" on error
		$optcode = "?";
	    }

	    # prepend a "+" to the optcode if the current argument began 
	    # with a "+"
	    if ($arg =~ /^\+/) {
		$optcode = "+$optcode";
	    }

	    # return 1 as long as the end of the command line argument list
	    # has not been reached
	    return(1);
	} else {
	    # assume end of argument list if the current argument does not
	    # begin with a "-" or "+"
	    return(0);
	}
}

#######################################################################
# function:	datecmp
# description:	compares two dates
# input:	2 date strings of the form:
#
#		MONTH DAY HOUR:MINUTE
#
#		e.g.: Sep 17 12:45
#
# output:	 1 if first date is later than second
# 		 0 if first date is equal to second
# 		-1 if first date is earlier than second
#######################################################################
sub	datecmp {
	# local associative array that assigns values to months of the year 
	local(%monthval) = (
			'Jan', 1,
			'Feb', 2,
			'Mar', 3,
			'Apr', 4,
			'May', 5,
			'Jun', 6,
			'Jul', 7,
			'Aug', 8,
			'Sep', 9,
			'Oct', 10,
			'Nov', 11,
			'Dec', 12,
	);

	# get the two dates to compare
	local($d1) = $_[0];
	local($d2) = $_[1];

	# declare all other local variables
	local($month1); local($day1); local($time1);
	local($hour1); local($minute1);
	local($month2); local($day2); local($time2);
	local($hour2); local($minute2);

	($month1,$day1,$time1) = split(/ /,$d1);
	($hour1,$minute1) = split(/\:/,$time1);
	($month2,$day2,$time2) = split(/ /,$d2);
	($hour2,$minute2) = split(/\:/,$time2);
	if ($monthval{$month1} > $monthval{$month2}) {
		return(1);
	}
	elsif ($monthval{$month1} < $monthval{$month2}) {
		return(-1);
	}
	if ($day1 > $day2) {
		return(1);
	}
	elsif ($day1 < $day2) {
		return(-1);
	}
	if ($hour1 > $hour2) {
		return(1);
	}
	elsif ($hour1 < $hour2) {
		return(-1);
	}
	if ($minute1 > $minute2) {
		return(1);
	}
	elsif ($minute1 < $minute2) {
		return(-1);
	}
	return(0);
}

sub Uniquesort {
  local ($sep,$liststring) = @_;
  local ($item,%uniquelist,$uniquestring);
  foreach $item (split (/$sep/,$liststring)) {
	$uniquelist{$item} = $DEFINED;
  } 
  foreach $item (sort keys %uniquelist) {
	$uniquestring .= $item . $sep;
  }
  chop $uniquestring;
  return $uniquestring;
}

sub Defectnumber {
  local ($defecttoken) = @_;
  local ($defectrelease,$defectnumber);
  ($defectrelease,$defectnumber) = split (/\./,$defecttoken);
  return($defectnumber);
}

sub Getdata {
  ($datafile,*dataarray) = @_; $rc=$SUCCESS;
  open (DATAFILE,"+>>$datafile") || &log("-e","$datafile open error ($!)");
  while ($inputline=<DATAFILE>) {
        chop $inputline;
        ($key,$data) = split (/\|/,$inputline);
        $dataarray{$key}=$data;
  }
  close (DATAFILE);
  return ($rc == $SUCCESS);
}

sub Basename {
     local($obj)=@_;
     $obj =~ s#.*/##;
     return($obj);
}

sub Savedata {
     local($datafile,*dataarray) = @_;
     local($key);
     open(DATAFILE,">$datafile") ||
          &log("-e","$datafile open error ($!)");
     foreach $key (keys %dataarray) {
          print DATAFILE "$key|$dataarray{$key}\n";
     }
     close(DATAFILE);
}

sub Encodepath {
  local($path) = @_;
  if ($encodedpaths{$path} eq undef) {
        $encodedpaths{$path} = ++$curpathcode;
        $decodedpaths[$curpathcode] = $path;
  }
  return ($encodedpaths{$path});
}

sub Decodepath {
  local($code) = @_;
  if (($code ne undef) && ($decodedpaths[$code] eq undef)) {
        &log("-e","unable to decode bldtd path code $code");
  }
  return ($decodedpaths[$code]);
}

#
# NAME: bldhostsfile_perl
#
# FUNCTION: Perl user interface to the hostsfile.dat data base.  This
#           routine should be called to search the hostsfile.dat data
#           base for a release.
#
# INPUT: release - release name to search for in hostsfile.dat.
#        logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#        environment_variable - name of environment variable to return
#                               value of.
#
# OUTPUT: none.
#
# RETURNS: value of environment variable, string of -1 if bldhostsfile
#          call returns non zero.
#

sub bldhostsfile_perl {
   local($release,$logerrors,$environment_variable) = @_;
   local($environment_value);

   $environment_value=`ksh -c ". bldkshconst; \
                               . bldhostsfile; \
                               . bldnodenames; \
                               bldhostsfile $release $logerrors; \
                               if [[ \\\$? -eq 0 ]]; \
                               then \
                                  print \$""$environment_variable;"" \
                               else \
                                  print \"\";
                               fi"`;
   return $environment_value;
}
