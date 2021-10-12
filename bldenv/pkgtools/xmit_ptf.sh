#!/bin/ksh
SCCSID="@(#)05  1.16  src/bldenv/pkgtools/xmit_ptf.sh, pkgtools, bos412, GOLDA411a 7/28/94 15:53:07"
#=======================================================================
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:  abort
#		check_cmdline_parms
#		process_xmit_table
#		set_up_work_dir
#		syntax
#               rcp_to_boulder
#               get_free_space
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#=======================================================================
#
# Purpose:    This program rcps the .ptf file to Boulder, compares the check
#	      sums of the file on the Austin ship server and the Boulder
#	      machine, and then moves the file to the destination directory.
#             Since this is generally ran in background, error messages
#             are mailed to the user running the program.
#
#
#=======================================================================

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  syntax function echos command syntax
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

syntax() {
  echo "\n$SCCSID\n" >> $mail_msg
  echo "Usage:  xmit_ptf  [-s dirname_ship] [-x xmit_control_file]" \
        >> $mail_msg 
  echo "                  [-V AIX version]" >> $mail_msg 
  echo "                  [-n] ptf_filename(s)" >> $mail_msg 
  echo "                  [-h] [-?]\n" >> $mail_msg
  echo "FLAGS: 
  echo "         -s     path name of ship server: >> $mail_msg
  echo "         -x     transmission control file" >> $mail_msg
  echo "         -V     AIX version of the ptfs - valid entries are 3 or 4." >> $mail_msg
  echo "                Defaults to AIX version 3 if the -V flag is not used" >> $mail_msg
  echo "                or if the entry does not begin with a 4." >> $mail_msg
  echo "         -n     does not rcp the ptfs but displays the \c" >> $mail_msg
  echo "steps that would be done" >> $mail_msg 
  echo "                containing the transmission information" >> $mail_msg
  echo "      -?/-h     Displays usage messages \n" >> $mail_msg

  abort
}

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  set up work directory: makes a work directory out in /tmp
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

set_up_work_dir() {

   # Create temporary work space with unique directory name
   # using process id and current date and time
   WORK=/tmp/${cmd}.$$.`date +"%y""%m""%d""%H""%M""%S"`
   rm -rf $WORK
   mkdir $WORK
   if [ $? != 0 ]; then

      mail -s "${cmd}: Error creating work dir '$WORK'" `whoami`@`hostname` \
          < /dev/null

      exit 1
   fi
   chmod 777 $WORK
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  Abort is called when an error occurs after WORK space is allocated
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

abort() {
    mail -s "xmit_ptf errors" `whoami`@`hostname` < $mail_msg
    rm -rf $WORK
    exit $rc
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  Function to check command execution environment
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

check_cmdline_parms() {
     # Check the ship_directory
     if [ ! -d "$ship_dir" ]; then
        echo "$cmd: ship directory '$ship_dir' is invalid" \
          >> $mail_msg
        rc=2
     else
        if [ ! -r "$ship_dir" ]; then
           echo "$cmd: ship directory '$ship_dir' must be readable" \
             >> $mail_msg
           rc=2
        else
           if [ ! -r "$ship_dir/index" ]; then
              echo "$cmd: Ship index file in '$ship_dir' must be readable" \
                >> $mail_msg
              rc=2
           fi
        fi
     fi

     # Check the xmit_table
     if [ ! -f $xmit_tbl -o ! -r $xmit_tbl ]; then
        echo "$cmd: The xmit control table '$xmit_tbl' must be a readable " \
          >> $mail_msg
        echo "$cmd: file" >> $mail_msg
        rc=2
     fi

     # Check the LOG_FILE
     if [ ! -f $LOG_FILE -o ! -w $LOG_FILE ]; then
        echo "$cmd: The command logfile '$LOG_FILE' must be a writable file" \
          >> $mail_msg
        rc=2
     fi

}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#  Function to get distribution
#    notification list from xmit_ptf table.
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

process_xmit_table() {
     #=================================================================
     # Get the distribution_notification record from xmit_ptf.table
     #=================================================================
     # get 3rd through nth fields of the 'distribution_notification:'
     #   record in the xmit_ptf table.
     dist_notify=`grep 'distribution_notification:' $xmit_tbl  |
                    head -1                |    # take only the first occurance
                    sed -e 's/ [ ]*/ /g'   |    # one 1 blank in between fields
                    sed -e 's/^# distribution_notification:[ ]*//g'`

     # check if found
     if [ "x$dist_notify" = "x" ]; then
        echo "$cmd: No distribution_notification record was" >> $mail_msg
        echo "$cmd: found in 'xmit_tbl'" >> $mail_msg
        rc=2
     fi
}

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# NAME: get_free_space
#
# DESCRIPTION:
#    Get and check the amount of free space in 1024 blocks in 
#      TARGETCCSSDIR directory
#    Exit xmit_ptf if the amount of free space was not obtained
#
# PRE-CONDITIONS:
#    Global variables TARGETHOST, TARGETLOGIN, TARGETCCSSDIR are set
#
# POST-CONDITIONS:
#    Global variable ptf_space has the new amount of space in the
#      TARGETCCSSDIR
#
# PARAMETERS:
#    None.
#
# DATA STRUCTURES:
#    See POST-CONDITIONS
#
# RETURNS:
#    Nothing.
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

get_free_space () {

ptf_space=`rsh $TARGETHOST -l $TARGETLOGIN \
           $DF_CMD "$TARGETCCSSDIR" | grep -v "Filesystem" \
           | cut -c24-31`
if [[ "x$ptf_space" = "x" ]] 
then
   echo "\n***************************************************************" \
         > $mail_msg
   echo "$cmd: 	Could not get the amount of free space in" >> $mail_msg
   echo "$cmd:	$TARGETCCSSDIR" >> $mail_msg
   echo "*****************************************************************" \
         >> $mail_msg
   rc=5
   abort
fi

}
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   rcp ptfs to Boulder
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

rcp_to_boulder () {

 if [ "$execute" = "yes" ]
 then
   # remove current $ptf_filename if it exists
   rsh_rc=0
   RC=0
   rsh_rc=`rsh $TARGETHOST -l $TARGETLOGIN \
           rm -f "$TARGETPTFDIR/$ptf_filename;echo $?"`
   RC=$?
   if [[ $rsh_rc != 0 || $RC != 0 ]]
   then
	echo "\n***********************************************************" \ 
              > $mail_msg
	echo "$cmd:	removal of original $ptf_filename from" >> $mail_msg
	echo "$cmd:	$TARGETPTFDIR failed" >> $mail_msg
	echo "*************************************************************" \
              >> $mail_msg
	rc=15
   else

     # rcp ptf_filename over to destination
     rcp $ship_dir/$ptf_filename $TARGETLOGIN@$TARGETHOST:$TARGETPTFDIR 
     if [[ $? != 0 ]]
     then
	echo "\n***********************************************************" \
             > $mail_msg
	echo "$cmd:	rcp of $PTF to $TARGETPTFDIR failed" >> $mail_msg
	echo "*************************************************************" \
             >> $mail_msg
	rc=11
     else
	# rcp was successful
	ship_sum=`sum $ship_dir/$ptf_filename | awk '{print $1}'`
	target_sum=`rsh $TARGETHOST -l $TARGETLOGIN \
                    sum "$TARGETPTFDIR/$ptf_filename" \
                    | awk '{print $1}'`
	if [[ $ship_sum != $target_sum ]]
	then
	   echo "\n*******************************************************" \
                 > $mail_msg
	   echo "$cmd:  PTF $PTF" >> $mail_msg
	   echo "$cmd:	Check sum of Boulder $target_sum does not match" \
                 >> $mail_msg
	   echo "$cmd:	check sum of austin ship server $ship_sum" >> $mail_msg
	   echo "$cmd:	removing .ptf file from Boulder directory" >> $mail_msg
	   echo "*********************************************************" \
                >> $mail_msg
	   rc=12
           rsh_rc=0
	   RC=0
	   rsh_rc=`rsh $TARGETHOST -l $TARGETLOGIN \
                   rm -f "$TARGETPTFDIR/$ptf_filename;echo $?"`
	   RC=$?
	   if [[ $rsh_rc != 0  || $RC != 0 ]]
	   then
		echo "\n***************************************************" \
                      >> $mail_msg
		echo "$cmd: Could not remove $ptf_filename" >> $mail_msg
		echo "$cmd: from the $TARGETPTFDIR directory" >> $mail_msg
		echo "*****************************************************" \
                     >> $mail_msg
	   fi
	else
	   # a successful rcp !!
	   # strip off .ptf and add .ccss to ptf and move to ccss dir
           rsh_rc=0
	   RC=0
           rsh_rc=`rsh $TARGETHOST -l $TARGETLOGIN \
                mv "$TARGETPTFDIR/$ptf_filename $TARGETCCSSDIR/$PTF.RS6.ccss; \
                echo $?"`
	   # check rc 
	   RC=$?
	   if [[ $rsh_rc != 0  || $RC != 0 ]]
	   then
	 	echo "\n***************************************************" \
                      > $mail_msg
		echo "$cmd:	Moving of ptf $PTF from $TARGETPTFDIR" \
                      >> $mail_msg
		echo "$cmd:	to $TARGETCCSSDIR failed" >> $mail_msg
		echo "*****************************************************" \
                     >> $mail_msg
	     	rc=16
	   else
              # chmod 644 on the file
              rsh_rc=0
              RC=0
              rsh_rc=`rsh $TARGETHOST -l $TARGETLOGIN \
                      chmod "644 $TARGETCCSSDIR/$PTF.RS6.ccss;echo $?"`
              # check rc
              RC=$?
              if [[ $rsh_rc != 0 || $RC != 0 ]]
              then
                echo "\n**************************************************" \
                      > $mail_msg
		echo "$cmd:	chmod of $TARGETCCSSDIR/$PTF.RS6.ccss" \
                      >> $mail_msg
		echo "$cmd:	failed" >> $mail_msg
		echo "****************************************************" \
                      >> $mail_msg
		rc=20
	      else
                ccss_li=`rsh $TARGETHOST -l $TARGETLOGIN \
                         li "-l $TARGETCCSSDIR/$PTF.RS6.ccss"`
                echo $ccss_li >> $ccss_li_file
	        # subtract # blocks used from ptf_space
 	        ((ptf_space=ptf_space-space_needed))
	      fi
           fi
	fi
     fi		   
   fi
  else
     # execute =no so print out what would happen
     echo "\nPTF $ptf_filename" >> $test_msg
     echo "\trsh $TARGETHOST -l $TARGETLOGIN rm -f \c" >> $test_msg
     echo "$TARGETPTFDIR/$ptf_filename" >> $test_msg
     echo "\trcp $ship_dir/$ptf_filename \c" >> $test_msg
     echo "$TARGETLOGIN@$TARGETHOST:$TARGETPTFDIR" >> $test_msg
     echo "\trsh $TARGETHOST -l $TARGETLOGIN sum \c" >> $test_msg
     echo "$TARGETPTFDIR/$ptf_filename" >> $test_msg
     echo "\trsh $TARGETHOST -l $TARGETLOGIN mv \c" >> $test_msg
     echo "$TARGETPTFDIR/$ptf_filename $TARGETCCSSDIR/$PTF.RS6.ccss" \
           >> $test_msg
     echo "\trsh $TARGETHOST -l $TARGETLOGIN chmod 644 \c" >> $test_msg
     echo "$TARGETCCSSDIR/$PTF.RS6.ccss" >> $test_msg
  fi
}
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Beginning of xmit_ptf
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

set -u  # Unset var's are treated as errors

# Initialize variables
cmd=`basename $0`
cmdline="${*-""}"  # set command line to null if there are no parameters
execute="yes"
version=""
ship_dir=""
help_request="n"
rc=0

# Set up work directory
set_up_work_dir

# set up file to write the output of li -l $PTF to 
ccss_li_file="/tmp/xmit_ptf.ccss_li"

# Set up mail message file
mail_msg="$WORK/mail_msg"
test_msg="$WORK/test_msg"

# Check input paramters
if [ $# = 0 ]; then
   echo "$cmd: at least one PTF file is required"  >> $mail_msg
   rc=1
   syntax
fi

set -- `getopt "s:x:V:nh?" $* 2>> $mail_msg`
if [ $? != 0 ]; then
   # no error message necessary as getopt already gave one
   rc=2
   syntax
fi

# Parse the command line
while [ $1 != "--" ]; do                # Set vars, based on params.
    case $1 in
        -s)
            ship_dir=$2
            shift 2;;   
        -x)
            xmit_tbl=$2
            shift 2;;
        -V)
            version=$2
            shift 2;;
        -n)
            execute="no"
            shift;;
        -[?h])
            help_request="y"
            rc=2
            shift;;
    esac
done
shift                                   # Get past the '--' flag.

if [ "$help_request" = "y" ]; then
    syntax
fi

echo "$version" | grep "^3" > /dev/null  
rc3=$?
echo "$version" | grep "^4" > /dev/null  
rc4=$?
if [ "$version" = ""  -o $rc3 = 0 ]
then 
    version=32
elif [ $rc4 = 0 ]
then
    version=41
else
    echo "$cmd: Invalid version entered with the -V flag." >> $mail_msg
    echo "$cmd: Defaulted to processing AIX version 3 ptfs." >> $mail_msg
    version=32
fi

#-------------------------------------------------------------
# AIX bldenv uses <32 or 41>_pkg_environment to set          |
# XMIT_TABLE and SHIP_DIRNAME                                |
#-------------------------------------------------------------
type "$version"_pkg_environment | grep -s 'not found'
if [ $? -eq 0 ]; then
    echo "$cmd: ERROR: file "$version"_pkg_environment not found in \$PATH" >> $mail_msg
    rc=1
    syntax
fi
. "$version"_pkg_environment
xmit_tbl=$XMIT_TABLE

# if the ship directory was not set on the command line, then set it to the
# environment variable
if [ "$ship_dir" = "" ]
then
    ship_dir=$SHIP_DIRNAME
fi

# Setup Boulder machine name, login, ptf dir, ccss dir
. $xmit_tbl

# resolve ship server to absolute path
ship_dir=`cd $ship_dir; pwd`

# Check command line parms, etc.
check_cmdline_parms

# Exit if command line error was found above
if [ $rc != 0 ]; then
   syntax
fi

# Check and convert ptf filenames to include '.ptf' extension
# now that the ship directory has been validated
if [ $# -eq 0 ]; then
   echo "$cmd: at least one ptf_filename is required" >> $mail_msg
   rc=2
else
   ptf_filenames=""
   for ptf_filename in $*; do
     
      # strip off any path and strip .ptf if given and then add .ptf to end
      ptf_filename="`basename $ptf_filename .ptf`.ptf"

      # check if ptf exists
      if [ ! -r $ship_dir/$ptf_filename -o \
           ! -f $ship_dir/$ptf_filename ]; then
         echo "$cmd: input ptf $ptf_filename must be a readable\c" \
           >> $mail_msg
         echo " file in $ship_dir" >> $mail_msg
         rc=2
      else
         # add ptf_filename to ptf_filenames
         ptf_filenames="$ptf_filenames $ptf_filename"
      fi
   done
fi

#=================================================================
# locate distribution notification list
#   from xmit_ptf.table
#=================================================================
process_xmit_table

#=================================================================
# Exit if error was found above
#=================================================================
if [ $rc != 0 ]; then
   syntax
fi


#=================================================================
# Log exact xmit_ptf command executed and by whom
#=================================================================
if [ "$execute" = "yes" ]; then
   echo "`date` `hostname` `whoami` $cmd $cmdline"  >> $LOG_FILE
fi

#=================================================================
# Get the initial amount of free space in TARGETCCSSDIR
#=================================================================

# determine the df command
os_version=`uname -v`
if [ "$os_version" = "4" ]
then
    DF_CMD="df -k"
else
    DF_CMD="df"
fi

get_free_space
 
#=================================================================
# Send each ptf file to the host
#=================================================================
#
for ptf_filename in $ptf_filenames; do

   # get ptf name by stripping both path and .ptf suffix
   PTF=`basename $ptf_filename .ptf`

   # initialize any failure message with the command to resend
   # initialize return_code
   rc=0

   # get amount of space needed in 512 byte blocks and then mult to 1024
   # size blocks
   space_needed=`li -LIs $ship_dir/$PTF* | cut -c2-7`
   ((space_needed=space_needed*2))
    
   # estimate the block size the ptf will take up in $TARGETPTFDIR
   # round space_needed to a multiple of 4
   # then add a buffer of 16 blocks
   ((round_blk=space_needed%4))
   ((space_needed=space_needed+round_blk+16))
   
   # check if there is enough space in boulder directory
   if [[ $space_needed -lt $ptf_space ]] 
   then
     # we have enough space to rcp the ptf
     rcp_to_boulder
   else
     get_free_space
     if [[ $space_needed -lt $ptf_space ]] 
     then
       # we have enough space to rcp the ptf
       rcp_to_boulder
     else
       if [[ "$execute" = "yes" ]]
       then
          # not enough space to rcp the ptf
          echo "\n**********************************************************" \
               > $mail_msg
          echo "$cmd: 	There is not enough free space in $TARGETCCSSDIR to" \
                >> $mail_msg
          echo "$cmd:	copy the ptf into." >> $mail_msg
          echo "************************************************************" \
               >> $mail_msg
          rc=10
       else
          echo "There is not enough space to rcp $ptf_filename" >> $test_msg
       fi
     fi
   fi
   #	
   # notify distribution of results
   #
   if [[ "$execute" = "yes" ]]
   then
      if [ $rc = 0 ]; then
          # notify distribution of success
          mail -s "PTF $PTF / CCSS successful" $dist_notify  \
                 < /dev/null  > /dev/null
      else
          # notify distribution of the problem
          mail -s "PTF $PTF / CCSS FAILED" $dist_notify  < $mail_msg
      fi
   fi

done

if [[ "$execute" != "yes" ]]
then
  mail -s "xmit_ptf ptfs" `whoami`@`hostname` < $test_msg
fi


# Remove temporary files and directories
rm -rf $WORK

# Graceful exit
sync
exit $rc
# END OF SHELL
