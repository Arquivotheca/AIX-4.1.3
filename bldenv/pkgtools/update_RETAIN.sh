#!/bin/ksh
# @(#)47        1.18  src/bldenv/pkgtools/update_RETAIN.sh, pkgtools, bos41J, 9513A_all 3/7/95 11:27:46
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#               call_SendPtf
#               check_cmdline_parms
#               process_xmit_table
#               set_up_work_dir
#               syntax
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#--------------------------------------------------
#  syntax function echoes command syntax
#--------------------------------------------------

syntax() {
  echo "Usage:  update_RETAIN  [-s dirname_ship] [-x shipped_ptfs_file]" \
     >> $mail_msg 
  echo "                       [-p dirname_prod] [-f apar_file] [-M maintenance level]" \
     >> $mail_msg
  echo "                       [-S] [-n] [-O] [-A] ptf_filename(s)" \
     >> $mail_msg
  echo "                       [-V AIX_version] [-h] [-?]\n"   >> $mail_msg
  echo "FLAGS:   -x     shipped_ptfs_filename"                 >> $mail_msg
  echo "                 entry format: 'ptf# apar# apar# ...'" >> $mail_msg
  echo "         -s     pathname of ship server"               >> $mail_msg
  echo "         -p     pathname of prod server"               >> $mail_msg
  echo "         -f     apar_file that contains apar data"     >> $mail_msg
  echo "	 -M     maintenance level data"		       >> $mail_msg
  echo "         -S     Not calculate size data for PTF"       >> $mail_msg
  echo "         -n     NO EXECUTE!"                           >> $mail_msg
  echo "                 Invokes SendPtf with test option"     >> $mail_msg
  echo "                 and lists data passed to it."         >> $mail_msg
  echo "         -O	Indicate OEM ptfs."		       >> $mail_msg
  echo "         -A	Update abstract data on RETAIN only."  >> $mail_msg
  echo "         -V	Target AIX version.  Default is 3 (AIXv3)." >> $mail_msg
  echo "      -?/-h     Displays usage messages \n"            >> $mail_msg

  abort
}

#---------------------------------------------------------------
#  set up work directory: makes a work directory out in /tmp
#---------------------------------------------------------------
set_up_work_dir() {

   # Create temporary work space with unique directory name
   # using process id and current date and time
   WORK=/tmp/${cmd}.$$.`date +"%y""%m""%d""%H""%M""%S"`
   rm -rf $WORK
   mkdir $WORK
   if [ $? != 0 ]; then

      # Send mail message from an in line document since
      # we have no work directory in which to create the
      # mail message

      mail -s "${cmd}: Error creating work dir '$WORK'" `whoami`@`hostname` \
         < /dev/null
      exit 1
   fi
   chmod 744 $WORK
}


#--------------------------------------------------
#  Abort is called when an error occurs after WORK space is allocated
#--------------------------------------------------
abort() {
    mail -s "${cmd} errors" `whoami`@`hostname` < $mail_msg
    rm -rf $WORK
    exit $rc
}


#--------------------------------------------------
#  Function to check command execution environment
#--------------------------------------------------
check_cmdline_parms() {

     # Check the dataset containing the ptf apar information
     if [ ! -r "$shipped_ptfs_file" ]; then
        echo "$cmd: shipped_ptfs file '$shipped_ptfs_file' must be readable" \
          >> $mail_msg
        rc=2
     fi

     # Check the xmit_table
     if [ ! -f $xmit_tbl -o ! -r $xmit_tbl ]; then
        echo "$cmd: the xmit control table '$xmit_tbl' must be a readable file" \
          >> $mail_msg
        rc=2
     fi
}


#--------------------------------------------------
#  Function to get distribution
#    notification list from xmit_ptf table.
#--------------------------------------------------
process_xmit_table() {

     #-----------------------------------------------------------------
     # Get the distribution_notification record from xmit_ptf.table
     #-----------------------------------------------------------------
     # get 3rd through nth fields of the 'distribution_notification:'
     #   record in the xmit_ptf table.
     dist_notify=`grep 'distribution_notification:' $xmit_tbl  |
                    head -1                |    # take only the first occurance
                    sed -e 's/ [ ]*/ /g'   |    # one 1 blank in between fields
                    sed -e 's/^# distribution_notification:[ ]*//g'`

     # check if found
     if [ "x$dist_notify" = "x" ]; then
        echo "$cmd: No distribution_notification record was found in 'xmit_tbl'" >> $mail_msg
        rc=2
     fi
}


#--------------------------------------------------
#  Routine to send the ptf apar data to RETAIN
#--------------------------------------------------
call_SendPtf() {

      echo "\nTHE SUMMARY INFORMATION FROM $cmd WERE:" > $mail_msg

      echo "\n$cmd: The SendPtf input file was:"  >${mail_msg}.temp
      cat  $WORK/$ptf_number.aparinfo >>  ${mail_msg}.temp

      cd $WORK
 
      if [ "$oemflag" = "no" ]; then
         # get family name from ptfapardef.constant file
         FAMILY=""
         RELEASES=""
         grep ^$ptf_number $WORK/ptfapardef |
         awk -F"|" '{print $6 " " $7 " " $9}' |
         while read release family vrmf
         do
             if [ "$FAMILY" = "" ]; then
                FAMILY="$family"
             else
                if [ "$family" != "$FAMILY" ]; then
                   echo "\n$cmd: The CMVC family is different for the same PTF: $ptf_number" >>${mail_msg}.temp
                   rc=10
                   cat ${mail_msg}.temp >> $mail_msg
	           return
                fi
             fi
	     if [ "$RELEASES" = "" ]; then
	        RELEASES="'$release'"
             else
                echo "$RELEASES" | grep "'$release'" > /dev/null 2>&1
                if [ $? -ne 0 ]; then
                   RELEASES="$RELEASES,'$release'"
                fi
             fi

      	     # For version 4 PTFs, send vrmf info in place of mldata
      	     if [ "$aix_version" = "41" ]; then
	 	mldata=$vrmf
      	     fi
         done
         if [ "$FAMILY" = "" -o "$RELEASES" = "" ]; then
            echo "\n$cmd: The ptf $ptf_number is not in the ptfapardef.constant file which" >>${mail_msg}.temp
            echo "$cmd: is in the prod server $prod_dir" >>${mail_msg}.temp
            rc=10
            cat ${mail_msg}.temp >> $mail_msg
            return
         fi
      fi

      # Build command to send PTF to RETAIN and CMVC
      CMD="SendPtf -x -D ${ship_dir} +s -M ${mldata} -V ${aix_version} -c"
      if [ "$abstractonlyflag" = "yes" ]; then
	 CMD="$CMD -A"
      fi
      if [ "$sizeflag" != "yes" ]; then
	 CMD="$CMD -S"
      fi
      if [ "$oemflag" = "yes" ]; then
         CMD="$CMD -O"
      else
         CMD="$CMD -F $FAMILY -R $RELEASES"
      fi
      CMD="$CMD ${ptf_number}.aparinfo" 
      echo "\n$cmd: The output information from the SendPtf command was:" \
           >>${mail_msg}.temp
      $CMD 2>&1 1>>${mail_msg}.temp 

      rc=$?
      cd $current_dir

      #
      # verify no error data in .notsent file
      #
      if [ ! -f $WORK/$ptf_number.notsent ]; then
         echo "\n*======================================================================" \
              >> $mail_msg
         echo "* $cmd: ERROR: The $ptf_number.notsent file was not created." \
              >> $mail_msg
         echo "* $cmd:        Did not run SendPtf command." >> $mail_msg
         echo "* $cmd:        Check SendPtf command." >> $mail_msg 
         echo "*======================================================================" \
              >> $mail_msg
         rc=3              # file should have existed
      else
         CHECK=`wc -c $WORK/$ptf_number.notsent | awk '{print $1}'`
         # '0' length file signifies success
         if [ $CHECK != 0 ]; then
            echo "\n*======================================================================" \
                 >> $mail_msg
            echo "* $cmd: ERROR: SendPtf failed while trying to update RETAIN." \
                 >> $mail_msg
            echo "* $cmd:        Retry RETAIN by running the promote_ship command" \
                 >> $mail_msg
            echo "*======================================================================" \
                 >> $mail_msg
            cat $WORK/$ptf_number.notsent >> $mail_msg 
            rc=4
         fi
      fi

      # append output from SendPtf command
      echo "\n\n\nTHE INPUT AND OUTPUT INFORMATION FOR SendPtf COMMAND WERE:" \
           >> $mail_msg
      cat ${mail_msg}.temp >> $mail_msg

}


#--------------------------------------------
#   Beginning of update_RETAIN
#--------------------------------------------

set -u  # Unset var's are treated as errors
#set -x
# Initialize variables
cmd=`basename $0`
cmdline="${*-""}"  # set command line to null if there are no parameters
current_dir=`pwd`
execute="yes"
sizeflag="yes"
oemflag="no"
help_request="n"
shipped_ptfs_file=""
apar_file=""
mldata=""
abstractonlyflag="no"
aix_version="32"
ship_dir=""
prod_dir=""
rc=0

# Set up work directory
set_up_work_dir

# Set up mail message file
mail_msg="$WORK/mail_msg"

# Check input paramters
if [ $# = 0 ]; then
   echo "$cmd: at least one PTF file is required"  >> $mail_msg
   rc=1
   syntax
fi

set -- `getopt "s:p:x:f:M:V:nSOAh?" $* 2>> $mail_msg`
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
        -p)
            prod_dir=$2
            shift 2;;
        -x)
            shipped_ptfs_file=$2
            shift 2;;
        -f)
            apar_file=$2
            shift 2;;
        -M)
            mldata=$2
            shift 2;;
        -n)
            execute="no"
            shift;;
        -S)
            sizeflag="no"
            shift;;
	-O)
	    oemflag="yes"
	    shift;;
	-A)
	    abstractonlyflag="yes"
	    shift;;
	-V)
	    echo $2 | grep "^4" >/dev/null
	    if [ $? -eq 0 ]; then
	       aix_version="41"
	    fi
	    shift 2;;
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

#-------------------------------------------------------------
# AIX bldenv uses <32 or 41>_pkg_environment to set          |
# XMIT_TABLE, SHIP_DIRNAME, & PROD_DIRNAME.                  |
#-------------------------------------------------------------
type "$aix_version"_pkg_environment | grep -s 'not found'
if [ $? -eq 0 ]; then
   echo "$cmd: ERROR: file ${aix_version}_pkg_environment not found in \$PATH" >> $mail_msg
    rc=1
    syntax
fi
. "$aix_version"_pkg_environment
xmit_tbl=$XMIT_TABLE

#-------------------------------------------------------------
# If ship_dir was set on the command line, don't override.   |
#-------------------------------------------------------------
if [ "x$ship_dir" = "x" ]; then
   ship_dir=$SHIP_DIRNAME
fi
if [ "x$prod_dir" = "x" ]; then
   prod_dir=$PROD_DIRNAME
fi

#-------------------------------------------------------------
# resolve absolute path for ship and prod
#-------------------------------------------------------------
ship_dir=`cd $ship_dir; pwd`
prod_dir=`cd $prod_dir; pwd`

if [ x$shipped_ptfs_file = x ]; then
    shipped_ptfs_file=$ship_dir/shipped_ptfs
fi

#-------------------------------------------------------------
# Check command line parms, etc.
#-------------------------------------------------------------
check_cmdline_parms

#-------------------------------------------------------------
# Exit if command line error was found above
#-------------------------------------------------------------
if [ $rc != 0 ]; then
   syntax
fi

#-------------------------------------------------------------
# Check and convert ptf filenames to only the ptf#
#-------------------------------------------------------------
if [ $# -eq 0 ]; then
   echo "$cmd: At least one ptf_filename is required" >> $mail_msg
   rc=2
else
   ptf_numbers=""
   needupdate="N"
   for ptf_number in $*; do

      #-------------------------------------------------------------
      # strip any path and strip .ptf if given and then add .ptf to end
      #-------------------------------------------------------------
      ptf_number=`basename $ptf_number .ptf`

      #-------------------------------------------------------------
      # check that shipped_ptfs file has data for this ptf in it
      #-------------------------------------------------------------
      sed -n -e "/^$ptf_number/p" $shipped_ptfs_file  \
                > $WORK/$ptf_number.aparinfo
      if [ ! -s $WORK/$ptf_number.aparinfo ]; then
         if [ x$apar_file = x ]; then
            echo "$cmd: No data for ptf $ptf_number found in $shipped_ptfs_file" \
            >> $mail_msg
            needupdate="Y"
         else
            echo "# $ptf_number `date +"%y""%m""%d""%H""%M""%S"` `hostname` `whoami`" \
            >  $WORK/shipped_ptfs
            sed -n -e "/$ptf_number/p" $apar_file >> $WORK/shipped_ptfs
            cat $WORK/shipped_ptfs >> $shipped_ptfs_file
            sed -n -e "/^$ptf_number/p" $shipped_ptfs_file \
                      > $WORK/$ptf_number.aparinfo
            if [ ! -s $WORK/$ptf_number.aparinfo ]; then
               echo "$cmd: No data for ptf $ptf_number found in $shipped_ptfs_file" \
               >> $mail_msg
               needupdate="Y"
            else
               ptf_numbers="$ptf_numbers $ptf_number"
            fi
         fi
      else
         #-------------------------------------------------------------
         # add ptf_number to ptf_numbers
         #-------------------------------------------------------------
         ptf_numbers="$ptf_numbers $ptf_number"
      fi
   done
   if [ "$needupdate" = "Y" ]; then
      echo "$cmd: Please run \"update_shipped_ptfs\" command to update" >> $mail_msg
      echo "$cmd: $shipped_ptfs_file file." >> $mail_msg
      rc=2
   fi 
fi

#-------------------------------------------------------------
# locate the jcl files and distribution notification list
#   from xmit_ptf.table
#-------------------------------------------------------------
process_xmit_table

#-------------------------------------------------------------
# Exit if error was found above
#-------------------------------------------------------------
if [ $rc != 0 ]; then
   syntax
fi

#-------------------------------------------------------------
# Log exact xmit_ptf command executed and by whom
#-------------------------------------------------------------
if [ "$execute" = "yes" ]; then
   echo "`date` `hostname` `whoami` $cmd $cmdline"  >> $LOG_FILE
fi

#-------------------------------------------------------------
# Send each ptf file to the host
#-------------------------------------------------------------
cat $prod_dir/ptfapardef.constant | sed -e '/^[# 	]/d'  > $WORK/ptfapardef

for ptf_number in $ptf_numbers; do

   #-------------------------------------------------------------
   # Do not run ftp if -n option was specified
   #-------------------------------------------------------------
   if [ "$execute" != "yes" ]; then
      # just print the jcl when -n is specified
      mail -s "PTF $ptf_number / apar data" `whoami`@`hostname` \
         < $WORK/$ptf_number.aparinfo
   else
      #----------------------------------------------------------
      # xfer the ptf apar data to RETAIN
      #----------------------------------------------------------
      rc=0 
      call_SendPtf

      #----------------------------------------------------------
      # notify distribution of results
      #----------------------------------------------------------
      if [ $rc = 0 ]; then
          # notify distribution of success
          echo "PTF $ptf_number; RETAIN successful"
          mail -s "PTF $ptf_number / RETAIN successful" $dist_notify  \
                 < /dev/null  > /dev/null
      else
          # notify distribution of the problem
          echo "PTF $ptf_number; RETAIN failed"
          mail -s "PTF $ptf_number / RETAIN FAILED" $dist_notify  < $mail_msg
      fi
   fi

done

#-------------------------------------------------------------
# logoff RETAIN
#-------------------------------------------------------------
hsend -t${RETAIN_WAIT} -home
hsend -t${RETAIN_WAIT} logoff
hsend -t0 undial

#-------------------------------------------------------------
# Remove temporary files and directories
#-------------------------------------------------------------
cd $current_dir
rm -rf $WORK

# Graceful exit
sync
exit $rc
# END OF SHELL
