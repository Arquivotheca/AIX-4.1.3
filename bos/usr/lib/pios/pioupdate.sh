#!/usr/bin/ksh
# @(#)72        1.6  src/bos/usr/lib/pios/pioupdate.sh, cmdpios, bos411, 9428A410j 3/4/94 11:38:25
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (converts custom colon files of old format to
#	      new format)
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#  This shell script will convert colon files from old format to new format.
#  Typically, this script will be run to upgrade "custom" files in a system
#  after newly upgraded "predef" colon files are installed.
#
#  The script expects an argument specifying a source file that is to be
#  converted, and optionally, an argument specifying name of the target file
#  that is the output of conversion.  If the target file name is omitted,
#  the script will overwrite the source file with the output.
#  Note: Set PIOUPD_DEBUG variable for debugging purposes.
#  Usage:	pioupdate -i source_file [ -o target_file ]
#			-i	-	source_file
#			-o	-	target_file
#
#  Examples:	pioupdate -i /usr/lib/lpd/pio/custom/4019ps:lp0
#  		pioupdate -i my3816.asc -o /usr/lib/lpd/pio/custom/3816a:lp1
#		PIOUPD_DEBUG= pioupdate -i /var/spool/lpd/pio/custom/v:d
#  This script must be run by root.

set -o nounset

# Declare functions defined later.
typeset -fu	pioupd_cleanup_exit
typeset -fu	pioupd_print_usage
typeset -fu	pioupd_proc_args
typeset -fu	pioupd_catch_signal
typeset -fu	pioupd_check_sf_and_fn
typeset -fu	pioupd_brain
typeset -fu	pioupd_make_tf

# Declare and initialize a few variables.
# Note: File descriptors 8 and 9 are used for source file and target file
# (a temp file) respectively.
# Exit codes for various conditions:
#		0	-	success
#		1	-	improper options and args
#		2	-	not a super user
#		3	-	source file doesn't exist or not readable
#		4	-	target (source) file directory not writable
#		5	-	formatter name not found
#		6	-	colon file already converted (changed to 0)
#		7	-	source file could not be opened for reading
#		8	-	temp. file could not be opened for writing
#		9	-	signal caught
#		10	-	target file could not be created
#		11	-	error in expanding additions using "piocnvt"
#		12	-	errors in conversion
#		13	-	error in restoring default state using "piocnvt"
#		14	-	parent colon file name could not be extracted
typeset 	DEBUG_SET=""
typeset 	DEBUG_USET=""
typeset		debug_tst
typeset -r	pioupd_pgnm=${0##*/}		# program name
typeset	-i	pioupd_iflag=0			# flag for source file
typeset -i	pioupd_oflag=0			# flag for target file
typeset		pioupd_srcfile			# source file
typeset		pioupd_tgtfile			# target file
typeset -r	pioupd_tmptf=/tmp/pioupd_tf$$	# temp. file
typeset -r	pioupd_tmpcf=/tmp/pioupd_cf$$	# temp. colon file for
						# additions, changes and
						# deletions
typeset		pioupd_fmtnm			# formatter name
typeset		pioupd_prntcf			# parent colon file name
typeset -i	pioupd_exitcode=0		# exit code
typeset -r	PIOCNVT=/usr/sbin/piocnvt	# "piocnvt" tool
typeset -r	pioupd_predir=/usr/lib/lpd/pio/predef

# Set debug flags on, if PIOUPD_DEBUG variable was set.
debug_tst=${PIOUPD_DEBUG-DEBUG}
[[ $debug_tst != DEBUG ]] &&
{ 
   DEBUG_SET="set -x"; DEBUG_USET="set +x"
}


# Function pioupd_cleanup_exit
# Performs clean up and exits.
function pioupd_cleanup_exit
{
   $DEBUG_SET

   exec 8<&-				# close source file
   exec 9>&-				# close temp. target file
   rm -f $pioupd_tmptf $pioupd_tmpcf

   exit ${1:-${pioupd_exitcode}}
}		# end - function pioupd_cleanup_exit


# Function pioupd_print_usage
# Prints usage.
function pioupd_print_usage
{
   $DEBUG_SET

   dspmsg piobe.cat -s 4 29 \
      'Usage: %1$s -i source_file [ -o target_file ]\n\t-i\t-\tsource colon file\n\t-o\t-\ttarget colon file (if omitted, same as source file)\n' \
      $pioupd_pgnm >&2

   return 0
}		# end - function pioupd_print_usage


# Function pioupd_proc_args
# Processes arguments
function pioupd_proc_args
{
   $DEBUG_SET
   typeset		flag

   while getopts :i:o: flag
   do
      case $flag in
         i)   pioupd_iflag=1
	      pioupd_srcfile=$OPTARG
	      ;;
         o)   pioupd_oflag=1
	      pioupd_tgtfile=$OPTARG
	      ;;
         :)   dspmsg piobe.cat -s 4 30 \
	 	 '%1$s: %2$s requires a value\n' \
	 	 $pioupd_pgnm $OPTARG >&2
	      pioupd_print_usage
	      pioupd_cleanup_exit 1
	      ;;
         \?)  dspmsg piobe.cat -s 4 31 \
	 	 '%1$s: unknown option %2$s\n' \
	 	 $pioupd_pgnm $OPTARG >&2
	      pioupd_print_usage
	      pioupd_cleanup_exit 1
	      ;;
      esac
   done
   shift OPTIND-1
   set --

   # If -i flag is omitted, display error and exit.
   if (( pioupd_iflag == 0 ))
   then
      dspmsg piobe.cat -s 4 32 \
	 'Source file must be supplied\n' >&2
      pioupd_print_usage
      pioupd_cleanup_exit 1
   fi

   # If -o flag is omitted, target file is the same as source file.
   [[ ${pioupd_oflag} = 0 ]] &&
      pioupd_tgtfile=${pioupd_srcfile}

   return 0
}		# end - function pioupd_proc_args


# Function pioupd_catch_signal
# Catches certain signals and performs a cleanup action.
function pioupd_catch_signal
{
   dspmsg piobe.cat -s 4 33 \
      'A signal was caught during conversion\n' >&2
   pioupd_cleanup_exit 9
}		# end - function pioupd_catch_signal


# Function pioupd_check_sf_and_fn
# Checks if the following conditions are true:
#	-	the specified source file exists and is readable;
#	-	the directory of source file is writable, in case -o flag
#		is omitted;
#	-	the directory of target file is writable, in case -o flag
#		is supplied;
#	-	formatter name can successfully be retrieved from the specified
#		source file;
#	-	a temp. target file can successfully be created;
# If any of the above conditions is false, the script exits.
function pioupd_check_sf_and_fn
{
   $DEBUG_SET
   typeset		filedir
   typeset -i		cflevel

   [[ -r $pioupd_srcfile ]] ||
   {
      dspmsg piobe.cat -s 4 34 \
	 '%1$s doesnt exist or not readable\n' \
	 $pioupd_srcfile >&2
      pioupd_cleanup_exit 3
   }

   # Check if the target file directory is writable.
   # Target file directory would be the same as the source file directory,
   # if -o flag was omitted.
   filedir=${pioupd_tgtfile%/*}
   [[ $filedir = $pioupd_tgtfile ]] && filedir=.
   [[ ! ( -d $filedir  &&  -w $filedir ) ]] &&
   {
      dspmsg piobe.cat -s 4 35 \
	 'Directory "%1$s" must be writable to create target file\n' \
	 $filedir >&2
      pioupd_cleanup_exit 4
   }

   # Extract the formatter name from the specified source file.  This formatter
   # name would be useful in the conversion process that is performed
   # subsequently.
   pioupd_fmtnm=$( awk 'BEGIN { FS = ":"; } /:mf:/ { print $5 }' \
		      $pioupd_srcfile 2>/dev/null )
   pioupd_fmtnm=${pioupd_fmtnm##*/}
#   [[ -z $pioupd_fmtnm ]] &&
#   {
#      dspmsg piobe.cat -s 4 36 \
#	 'Formatter name couldnt be extracted from the source file\n' >&2
#      pioupd_cleanup_exit 5
#   }

   # Check if attribute for level ("zL") is already included in the source
   # colon file.  If so, and if colon file level is >= 1, then no conversion
   # is to be done.  Else, or if attribute is not present, it needs conversion.
   cflevel=$( awk -F: '/:zL:/ { print $5 }' $pioupd_srcfile 2>/dev/null )
   if (( cflevel >= 1 ))
   then
      dspmsg piobe.cat -s 4 37 \
	 'The source file "%1$s" is already in new format\n' \
	 $pioupd_srcfile >&2
      # pioupd_cleanup_exit 6		# changed to 0 not to upset others
      pioupd_cleanup_exit 0
   fi

   # Extract the parent colon file name.  If error, exit.
   pioupd_prntcf=$( awk -F: '/:zP:/ { print $5 }' $pioupd_srcfile 2>/dev/null )
#   [[ -z $pioupd_prntcf ]] &&
#   {
#      dspmsg piobe.cat -s 4 38 \
#	 'Parent colon file name couldnt be extracted from the source file\n' \
#	 >&2
#      pioupd_cleanup_exit 14
#   }

   return 0
}		# end - function pioupd_check_sf_and_fn


# Function pioupd_brain
# Performs various conversion steps on the specified source file and
# outputs the result into a temp. target file.
# Various conversion specifications are outlined below.
# (i) The following attributes need be added:
#	-	zA	[if doesn't exist]	(:687:zA::file)
#	-	zL	[if doesn't exist]	(:686:zL::1)
#	-	zV	[if doesn't exist]	(:685:zV::!)
#	-	zE	[if doesn't exist]	(:691:zE::)
#	-	zH	[if doesn't exist]	(:692:zH::)
#	-	zR	[if doesn't exist]	(:693:zR::)
#
# (ii) The following attributes need be changed:
#	-	zL		to		1
#	-	zV		to		!
#	-	zA		to		file (if zA value is null)
#
function pioupd_brain
{
   $DEBUG_SET
   typeset		srcfilepath
   typeset 		oldpt=""

   trap pioupd_catch_signal HUP INT QUIT TERM

   # Open source and temp. target files for reading and writing respectively.
   exec 8<${pioupd_srcfile} ||
   {
      dspmsg piobe.cat -s 4 39 \
	 'Error %1$d in opening "%2$s" for reading\n' \
	 $ERRNO $pioupd_srcfile >&2
      pioupd_cleanup_exit 7
   }
   exec 9>|${pioupd_tmptf} ||
   {
      dspmsg piobe.cat -s 4 40 \
	 'Error %1$d in opening "%2$s" for writing\n' \
	 $ERRNO $pioupd_tmptf >&2
      pioupd_cleanup_exit 8
   }

   # Create a temp. colon file that includes the attributes that need be
   # added to the original source file as part of the conversion process.
   # A sample temp. colon file should look like this:
   if [[ ${pioupd_srcfile#/} = ${pioupd_srcfile} ]]
   then
      srcfilepath="${PWD}/${pioupd_srcfile}"
   else
      srcfilepath=${pioupd_srcfile}
   fi
   awk -v fmtnm="${pioupd_fmtnm}" -v srcflpath="${srcfilepath}" '
      # Initialization
      BEGIN {
	 FS = ":";
	 zAfound = 0;			# for "zA"
	 zLfound = 0;			# for "zL"
	 zVfound = 0;			# for "zV"
	 zEfound = 0;			# for "zE"
	 zHfound = 0;			# for "zH"
	 zRfound = 0;			# for "zR"
	 zAattrec = ":687:zA::file";	# attribute record for "zA"
	 zEattrec = ":691:zE::";	# attribute record for "zE"
	 zHattrec = ":692:zH::";	# attribute record for "zH"
	 zLattrec = ":686:zL::1";	# attribute record for "zL"
	 zPattrec = ":476:zP::" srcflpath;  # attribute record for "zP"
	 zRattrec = ":693:zR::";	# attribute record for "zR"
	 zSattrec = ":477:zS::!";       # attribute record for "zS"
	 zVattrec = ":685:zV::!";	# attribute record for "zV"
	 hirhdr   = ":475:__HIR::"	# __HIR header
      }

      # Logic to check existence of a few selected attributes
      $3 == "zA" { zAfound++; }
      $3 == "zL" { zLfound++; }
      $3 == "zV" { zVfound++; }
      $3 == "zE" { zEfound++; }
      $3 == "zH" { zHfound++; }
      $3 == "zR" { zRfound++; }

      # Logic to create output data for additions of attributes
      END {
	 # output HIR attributes
	 printf("%s\n", hirhdr);
	 if (!zAfound)
	    printf("%s\n", zAattrec);
	 if (!zEfound)
	    printf("%s\n", zEattrec);
	 if (!zHfound)
	    printf("%s\n", zHattrec);
	 if (!zLfound)
	    printf("%s\n", zLattrec);
	 printf("%s\n", zPattrec);
	 if (!zRfound)
	    printf("%s\n", zRattrec);
	 printf("%s\n", zSattrec);
	 if (!zVfound)
	    printf("%s\n", zVattrec);
      } ' <${pioupd_srcfile} >|${pioupd_tmpcf}
   [[ $? != 0 ]] &&
   {
      dspmsg piobe.cat -s 4 41 \
	 'Error(s) in creating "%1$s"\n' \
	 $pioupd_tmpcf >&2
      pioupd_cleanup_exit 8
   }

   # If the parent colon file is an old printer type, create a temporary
   # symbolic link.  If the parent colon file doesnt exist, link it to
   # a dummy file as a last resort so that piocnvt doesnt abort.
   [[ -n $pioupd_prntcf ]] &&
   {
      [[ -f $pioupd_predir/$pioupd_prntcf ]] ||
      {
	 print - ":056:__FLG::" >|$pioupd_predir/$pioupd_prntcf 2>/dev/null &&
	 oldpt="$pioupd_predir/$pioupd_prntcf"
      }
   }

   # Merge the data for attribute additions into the original source file.
   eval $PIOCNVT -s+ -i ${pioupd_tmpcf} >/dev/null ||
   {
      dspmsg piobe.cat -s 4 42 \
	 'Error in expanding additions using "%1$s"\n' \
	 $PIOCNVT >&2
      pioupd_cleanup_exit 11
   }

   # If a symbolic link was created for the old printer type, remove it.
   [[ -z $oldpt ]] || rm -f $oldpt

   # Read the source colon file and process each line performing changes
   # and deletions.
   awk -F: -v fmtnm="${pioupd_fmtnm}" -v prntcf="${pioupd_prntcf}" '
      # Initialization
      BEGIN {
	 if (ARGC < 3)
	 {
	    print_error(43, "AWK: Improper passing of parameters to awk script\n");
	    exit(1);
	 }
	 srcfile = ARGV[1];
	 tgtfile = ARGV[2];
	 attnmptrn = "[@_a-zA-Z][a-zA-Z0-9]";	# attribute name syntax
	 alphadigit = "[0-9A-Za-z]";		# alphanumeric char
      }					# end of Initialization

      # Main Processing logic
      {
	 # Process the source file.
	 while ((retcode = getline < srcfile) > 0)
	 {
	    # Initialize fields
	    currec = $0 "";
	    msgcatd = $1 "";
	    msgno = $2 "";
	    attrnm = $3 "";
	    attrlimit = $4 "";
	    attrval = $5 "";
	    tmppt = "";
	    tmpds = "";
	    i = 0;

	    # If current record contains "mt", convert the old printer
	    # type to new one, if necessary.
	    if (attrnm == "mt")
	    {
	       if (attrval == "2380" ||
	           attrval == "2380-2" ||
	           attrval == "2381" ||
	           attrval == "2381-2" ||
	           attrval == "2390" ||
	           attrval == "2390-2" ||
	           attrval == "2391" ||
	           attrval == "2391-2" ||
	           attrval == "3812-2" ||
	           attrval == "3816" ||
	           attrval == "4019" ||
	           attrval == "4029" ||
	           attrval == "4037" ||
	           attrval == "4039" ||
	           attrval == "4070" ||
	           attrval == "4072" ||
	           attrval == "4076" ||
	           attrval == "4079" ||
	           attrval == "4201-2" ||
	           attrval == "4201-3" ||
	           attrval == "4202-2" ||
	           attrval == "4202-3" ||
	           attrval == "4207-2" ||
	           attrval == "4208-2" ||
	           attrval == "4212" ||
	           attrval == "4216-31" ||
	           attrval == "4224" ||
	           attrval == "4226" ||
	           attrval == "4234" ||
	           attrval == "5202" ||
	           attrval == "5204" ||
	           attrval == "6180" ||
	           attrval == "6182" ||
	           attrval == "6184" ||
	           attrval == "6185-1" ||
	           attrval == "6185-2" ||
	           attrval == "6186" ||
	           attrval == "6252" ||
	           attrval == "6262" ||
	           attrval == "7372")
		  attrval = "ibm" attrval;
	       else if (attrval == "prt1021")
		  attrval = "bull1021";
	       else if (attrval == "prt1072")
		  attrval = "bull1070";
	       else if (attrval == "prt0201")
		  attrval = "bull201";
	       else if (attrval == "prt0411")
		  attrval = "bull411";
	       else if (attrval == "prt0422")
		  attrval = "bull422";
	       else if (attrval == "prt4512" || attrval == "prt4513")
		  attrval = "bull451";
	       else if (attrval == "prt4542" || attrval == "prt4543")
		  attrval = "bull454";
	       else if (attrval == "prt0721")
		  attrval = "bull721";
	       else if (attrval == "prt9222")
		  attrval = "bull922";
	       else if (attrval == "prt9232")
		  attrval = "bull923";
	       else if (attrval == "prt9242")
		  attrval = "bull924";
	       else if (attrval == "prt9282")
		  attrval = "bull924N";
	       else if (attrval == "prt9702")
		  attrval = "bull970";
	       else if (attrval == "pr88")
		  attrval = "bullpr88";
	       else if (attrval == "printer" || attrval == "postscript" ||
			attrval == "plotter")
		  attrval = "generic";
	       currec = build_rec(msgcatd, msgno, attrnm, attrlimit,
				  attrval);
	       parse_print(currec);
	    }

	    # If current record contains "zL", make the necessary change.
	    else if (attrnm == "zL")
	    {
	       attrval = "1";
	       currec = build_rec(msgcatd, msgno, attrnm, attrlimit,
				  attrval);
	       parse_print(currec);
	    }

	    # If current record contains "zV", make the necessary change.
	    else if (attrnm == "zV")
	    {
	       attrval = "!";
	       currec = build_rec(msgcatd, msgno, attrnm, attrlimit,
				  attrval);
	       parse_print(currec);
	    }

	    # If current record contains "zA", make the change, if necessary.
	    else if (attrnm == "zA")
	    {
	       if (!length(attrval))
		  attrval = "file";
	       currec = build_rec(msgcatd, msgno, attrnm, attrlimit,
				  attrval);
	       parse_print(currec);
	    }

	    # If current record contains "zP", restore the original parent
	    # colon file name.  This is necessitated, because the source
	    # colon file has been merged with the colon file for additions
	    # by "piocnvt" thus overwriting its original parent colon file
	    # name.
	    # Also check if parent colon file name was specified in the
	    # original colon file.  If not, skip this line.
	    else if (attrnm == "zP")
	    {
	       if (length(prntcf))
	       {
		  if (i = index(prntcf,"."))
		  {
		     tmppt = substr(prntcf,1,i-1);
		     tmpds = substr(prntcf,i+1);
	 	     if (tmppt == "2380" ||
	    	         tmppt == "2380-2" ||
	    	         tmppt == "2381" ||
	    	         tmppt == "2381-2" ||
	    	         tmppt == "2390" ||
	    	         tmppt == "2390-2" ||
	    	         tmppt == "2391" ||
	    	         tmppt == "2391-2" ||
	    	         tmppt == "3812-2" ||
	    	         tmppt == "3816" ||
	    	         tmppt == "4019" ||
	    	         tmppt == "4029" ||
	    	         tmppt == "4037" ||
	    	         tmppt == "4039" ||
	    	         tmppt == "4070" ||
	    	         tmppt == "4072" ||
	    	         tmppt == "4076" ||
	    	         tmppt == "4079" ||
	    	         tmppt == "4201-2" ||
	    	         tmppt == "4201-3" ||
	    	         tmppt == "4202-2" ||
	    	         tmppt == "4202-3" ||
	    	         tmppt == "4207-2" ||
	    	         tmppt == "4208-2" ||
	    	         tmppt == "4212" ||
	    	         tmppt == "4216-31" ||
	    	         tmppt == "4224" ||
	    	         tmppt == "4226" ||
	    	         tmppt == "4234" ||
	    	         tmppt == "5202" ||
	    	         tmppt == "5204" ||
	    	         tmppt == "6180" ||
	    	         tmppt == "6182" ||
	    	         tmppt == "6184" ||
	    	         tmppt == "6185-1" ||
	    	         tmppt == "6185-2" ||
	    	         tmppt == "6186" ||
	    	         tmppt == "6252" ||
	    	         tmppt == "6262" ||
	    	         tmppt == "7372")
		        attrval = "ibm" tmppt "." tmpds;
	 	     else if (tmppt == "prt1021")
			attrval = "bull1021" "." tmpds;
	 	     else if (tmppt == "prt1072")
			attrval = "bull1070" "." tmpds;
	 	     else if (tmppt == "prt0201")
			attrval = "bull201" "." tmpds;
	 	     else if (tmppt == "prt0411")
			attrval = "bull411" "." tmpds;
	 	     else if (tmppt == "prt0422")
			attrval = "bull422" "." tmpds;
	 	     else if (tmppt == "prt4512" || tmppt == "prt4513")
			attrval = "bull451" "." tmpds;
	 	     else if (tmppt == "prt4542" || tmppt == "prt4543")
			attrval = "bull454" "." tmpds;
	 	     else if (tmppt == "prt0721")
			attrval = "bull721" "." tmpds;
	 	     else if (tmppt == "prt9222")
			attrval = "bull922" "." tmpds;
	 	     else if (tmppt == "prt9232")
			attrval = "bull923" "." tmpds;
	 	     else if (tmppt == "prt9242")
			attrval = "bull924" "." tmpds;
	 	     else if (tmppt == "prt9282")
			attrval = "bull924N" "." tmpds;
	 	     else if (tmppt == "prt9702")
			attrval = "bull970" "." tmpds;
	 	     else if (tmppt == "pr88")
			attrval = "bullpr88" "." tmpds;
		     else if (tmppt == "printer" ||
			      tmppt == "postscript" ||
			      tmppt == "plotter")
			attrval = "generic" "." tmpds;
		     else
			attrval = prntcf;
		  }
		  else
		     attrval = prntcf;
	          currec = build_rec(msgcatd, msgno, attrnm, attrlimit,
				     attrval);
	          parse_print(currec);
	       }
	    }

	    # If parent colon file name was not specified in the original
	    # colon file, skip "zS" attribute.  (A bug in 'piocnvt' causes
	    # the file to be truncated to zero at the time of expanding
	    # if there is no "zP", but "zS".)
	    else if (attrnm == "zS" && !length(prntcf))
	    {
	       # skip this
	    }

	    # For other attributes, just perform the global substitutions.
	    else
	       parse_print(currec);
	 }				# end - while getline
	 if (retcode < 0)
	 {
	    print_error(43, sprintf("AWK: Error in reading the file \"%s\"\n",
				srcfile));
	    exit(2);
	 }

	 # All is well, that ends well
	 exit(0);
      }					# end of Main Processing Logic

      # Wrap up
      END {
	 close(srcfile);
	 close(tgtfile);
      }					# end of Wrap up

      # Function to parse the specified input record for any substitutions
      # to be made, and to output the record after substitutions.
      function parse_print(inprec,
				outrec, prefixstr, tmpstr, VARDIR, LOCAL)
      {
	 VARDIR = "/var/spool/lpd/pio"; LOCAL = "/@local";

	 prefixstr = "";
	 for (tmpstr = inprec; match(tmpstr,VARDIR); )
	 {
	    prefixstr = prefixstr substr(tmpstr,1,RSTART+RLENGTH-1) LOCAL;
	    tmpstr = (index(substr(tmpstr,RSTART+RLENGTH),LOCAL) == 1 ? \
	       	      substr(tmpstr,RSTART+RLENGTH+length(LOCAL)) : \
	       	      substr(tmpstr,RSTART+RLENGTH));
	 }
	 outrec = prefixstr tmpstr;

	 gsub(/\/usr\/lpd\/pio\/burst/,"/usr/lib/lpd/pio/burst",outrec);
	 gsub(/\/usr\/lpd\/pio\/etc/,"/usr/lib/lpd/pio/etc",outrec);
	 gsub(/\/usr\/lpd\/pio\/fonts/,"/usr/lib/lpd/pio/fonts",outrec);
	 gsub(/\/usr\/lpd\/pio\/fmtrs/,"/usr/lib/lpd/pio/fmtrs",outrec);
	 gsub(/\/usr\/lpd\/pio\/predef/,"/usr/lib/lpd/pio/predef",outrec);
	 gsub(/\/usr\/lpd\/pio\/trans1/,"/usr/lib/lpd/pio/trans1",outrec);

	 gsub(/\/usr\/lpd\/pio\/custom/,"/var/spool/lpd/pio/@local/custom", \
	      outrec);
	 gsub(/\/usr\/lpd\/pio\/ddi/,"/var/spool/lpd/pio/@local/ddi",outrec);
	 gsub(/\/usr\/lpd\/pio\/flags/,"/var/spool/lpd/pio/@local/flags", \
	      outrec);

	 print_rec(outrec);
	 return;
      }					# end of function parse_print

      # Function to output the specified record to the target file.
      function print_rec(outrec)
      {
	 printf("%s\n", outrec) > tgtfile;
	 return;
      }					# end of function print_rec

      # Function to output an error message.
      function print_error(msgno, errrec)
      {
	 system("dspmsg piobe.cat -s 4 " msgno " \"\%1\$s\" \"" errrec "\" >&2")
	 return;
      }

      # Function to build an attribute record from given fields
      function build_rec(mcatd, mno, anm, almt, aval)
      {
	 return mcatd FS mno FS anm FS almt FS aval;
      }
      ' ${pioupd_tmpcf} ${pioupd_tmptf} >/dev/null
   if (( $? != 0 ))			# if errors in 'awk' processing
   then
      dspmsg piobe.cat -s 4 44 \
	 'Errors found in conversion of the source file "%1$s"\n' \
	 $pioupd_srcfile >&2
      pioupd_cleanup_exit 12
   fi

   # COMMENTED OUT!
   # Restore the resultant custom file to its default state.
   #eval $PIOCNVT -i ${pioupd_tmptf} >/dev/null ||
   #{
   #   dspmsg piobe.cat -s 4 45 \
   #      'Error in restoring to default state using "%1$s"\n' \
   #      $PIOCNVT >&2
   #   pioupd_cleanup_exit 13
   #}

   return 0
}		# end - function pioupd_brain


# Function pioupd_make_tf
# Makes the final target file.
function pioupd_make_tf
{
   $DEBUG_SET

   trap pioupd_catch_signal HUP INT QUIT TERM

   mv -f $pioupd_tmptf $pioupd_tgtfile 2>/dev/null ||
   {
      dspmsg piobe.cat -s 4 46 \
	 'Error %1$d in creating the target file "%2$s"\n' \
	 $ERRNO $pioupd_tgtfile >&2
      pioupd_cleanup_exit 10
   }

   return 0
}		# end - function pioupd_make_tf

# Main
# Main body of the script.
{
   $DEBUG_SET

   pioupd_proc_args "${@-}"		# process args
   pioupd_check_sf_and_fn		# check files and formatter name

   # Set up a signal handler function to catch certain signals and perform
   # cleanup.
   trap pioupd_catch_signal HUP INT QUIT TERM

   pioupd_brain				# main processing logic
   pioupd_make_tf			# make the target file

   pioupd_cleanup_exit 0		# clean up and exit

   $DEBUG_USET
}		# end - main

