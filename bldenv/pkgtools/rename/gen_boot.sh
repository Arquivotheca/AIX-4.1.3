#!/bin/sh
  SCCSID=" @(#)88        1.2  src/bldenv/pkgtools/rename/gen_boot.sh, pkgtools, bos412, GOLDA411a 2/1/93 12:20:41"
#
# COMPONENT_NAME: gen_boot.sh
#
# FUNCTIONS:
#	Parse and store command line parameters.
#	Generate ccss-formatted bosboot package.
#	Place bosboot package onto the ship server.
#	Add bosboot entry to the ship server index file.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Purpose:
#	Generate ccss-formatted bosboot package that will be
#	placed onto the ship server with an entry added to the
#	ship server index file.
#
# Command Format:
#	gen_boot -b bosboot-file-name [-h|-?]
#		where:
#		-b path & file-name of input bosboot binary.
#		-h or -? lists usage message & exit, ignore options.
#		Note: the positional parameters may be in any order.
#
# Change History:
#       11/12/91 - wht-Added support to generate ship index entry.
#	11/18/91 - rlk-More comments & integrity routines added.
#
#######################################################################

####################### syntax function ###############################
syntax() {				# You get here if you err or ask.
    echo "$SCCSID"			# Always show this line.
    echo "$myName -b bosboot-file-name [-h|-?]"
    echo "    where"
    echo "	-b path & file-name of input bosboot binary file.\n"
    echo "	-h or -? displays usage and exits, ignoring other flags."
    echo "	Note: Positional parameters may be in any order.\n"

    rm -rf $WORK > /dev/null 2>&1	# Get rid of work files, if any.
    exit $rc				# Use return code, set elsewhere.
}					#### End of syntax function ###

######################### Exit function ###############################
Exit () {				#Common exit point, w/o usage msg.
    rm -rf $WORK > /dev/null 2>&1	# Get rid of work files, if any.
    exit $rc				# Use return code, set elsewhere.
}					#### End of Exit function #####

####################### Start of main shell logic. ####################
#
cmdResult=`type 32_pkg_environment`	# See if command accessable.
cmdResult=`expr "$cmdResult" : '.*\(not found\)'` # Extract "not found".
if [ "${cmdResult}x" = "not foundx" ]; then	  # If there, err.
    echo "$myName: ERROR: 32_pkg_environment not in current path."
    echo "$myFill         Make 32_pkg_environment available and rerun.\n"
    rc=5; syntax
fi
#	Get PROD_DIRNAME, SHIP_DIRNAME & LOG_FILE as export variables.
. 32_pkg_environment			# Get some standard variables.
#
if [ "$BLDTMP" = "" ]; then
   BLDTMP="/tmp"
fi

WORK=$BLDTMP/gen_boot.$$ 		# Name of unique dir.
BLOCK_SIZE=31744			# Block size 31K bytes VM compat.
rc=0					# Init return code to success.
myName=`basename $0`			# Store pgm name for display,
myFill=`echo "$myName" | sed -e 's/./ /g'`	# & blanks same size.
cmdline="$*"				# Store the command line.
#
if [ $# -lt 1 ]; then			# Must have one parameter.
    echo "$myName: ERROR: Bosboot file-name must be in command line."
    rc=500; syntax			# Exit with usage display & err cd
fi					#   if no command line parameters.

#################### Parse command line parameters ####################
set -- `getopt "h?b:" $*`		# Set up & check input flags.
if [ $? -ne 0 ]; then			# Did set command work?
    echo "ERROR in command line \"$cmdline\".\n"
    rc=500; syntax			# Bad params, or something.
fi

#################### Initialize some variables ########################
while [ "$1" != "--" ]			# Until end of parameters,
do					#   loop through this routine
    case $1 in				#   to validate & store contents

	"-b")				# -b is for input bosboot binary
	    bosBoot=$2			#   path & file-name.
	    shift 2;;

	"-[?h]")			# h or ? means user wants help;
	    syntax			#   so give help and exit.

    esac				# No mas params.
done
shift					# Shift past '--' flag.
#
# See if file on command lines exists. It must, if to continue.
#
 if [ ! -f "$bosBoot"  -o  ! -r "$bosBoot" ]
    then
    echo "$myName: ERROR: Unable to read bos boot file $bosBoot"
    rc=7; Exit
 fi
 if [ ! -w "$SHIP_DIRNAME" ]; then
    echo "$myName: ERROR: Directory $SHIP_DIRNAME is not writable."
    echo "$myFill         Cannot continue."
    rc=11; Exit
 fi
 if [ ! -w "${SHIP_DIRNAME}/index" ]; then
    echo "$myName: ERROR: ${SHIP_DIRNAME}/index is not writable."
    echo "$myFill         Cannot continue."
    rc=8; Exit
 fi
 if [ ! -w "$LOG_FILE" ]; then
    echo "$myName: ERROR: $LOG_FILE is not writable."
    echo "$myFill         Cannot continue."
    rc=9; Exit
 fi
#
# initialize files and dirs. $WORK should give unique dir.
#
 rm -rf $WORK > /dev/null 2>&1	# Get rid of work dir. Should never exist.
 if [ -d $WORK ]		# Still there? Should never be.
 then				# If so, rm didn't work, so err.
    echo "$myName: ERROR: Unable to delete directory $WORK"
    rc=10; Exit
 fi
 mkdir  $WORK			# Create unique work dir.
#
#
 echo "======processing bos boot file $bosBoot ======"
#
# Reblock the file to required blocksize.
#
 dd if=$bosBoot of=$WORK/reblocked bs=$BLOCK_SIZE conv=sync
 rc=$?					# Did dd work?
 if [ "$rc" -ne 0 ]			# If not, tell user & abort.
     then
     echo "\n$myName: ERROR: dd error number $rc reblocking bos boot file.\n"
     rc=80; Exit
 fi
#
# Pack it.
#
 ccss_pack -1 $WORK/reblocked -c $SHIP_DIRNAME/bosboot.ptf
 rc=$?					# Did ccss_pack succeed?
 if [ "$rc" -ne 0 ]			# If not, tell user & abort.
     then
     echo "$myName: ERROR: === ccss_pack error number $rc! ==="
     rc=90; Exit
 fi
#
# Delete old bosboot index entry & append a new one.
#
#  The grep copies all lines that do not have 'bosboot' in 2nd subfield.
 grep -v "^[^:]*:bosboot:" $SHIP_DIRNAME/index >  $WORK/new_ship_index
#  The echo appends new bosboot entry to work index before copy back.
 echo "bos:bosboot:prereq:bos:v=03 r=02 m=0000"  >> $WORK/new_ship_index
 cp   $WORK/new_ship_index $SHIP_DIRNAME/index
#
# Log successful run of gen_boot.
#
 echo "`date` `hostname` `whoami` $cmdline" >> $LOG_FILE
 rc=$?					# Did append to log work?
 if [ "$rc" -ne 0 ]; then		# If not, tell user.
    echo "\n$myName: ERROR: Logging of this run to $LOG_FILE unsuccessful!"
    echo "$myFill         Error code $rc was returned.\n"
    rc=9; Exit
 fi
#
# Cleanup work area and exit.
#
 echo "\n$myName:  bosboot.ptf successfully placed on ship server.\n"
 rc=0; Exit
#
### End of gen_boot ####### End of gen_boot ####### End of gen_boot ###
