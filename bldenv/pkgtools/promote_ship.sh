#!/bin/ksh
# @(#)04        1.20  src/bldenv/pkgtools/promote_ship.sh, pkgtools, bos41J, 9517A_all 4/25/95 14:48:58
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#		build_apar_data
#		call_SendPtf
#		call_xmit_ptf
#		check_file_permissions
#		check_space
#		move_ptfs
#		syntax
#		update_shipped_ptfs
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

######################### Function ########################
# syntax function echos command syntax
###########################################################
syntax() {
    echo "promote_ship [-p dirname_prod] [-s dirname_ship] [-M maintenance level]"
    echo "             [-V AIX version]"
    echo "             [-n] [-R] [-E] [-S] [-d] [-x] [-O] [-A] [-h] "
    echo "             [-?] ptf_filename(s)\n"
    echo "FLAGS:"
    echo "         -p     Pathname of production server"
    echo "         -s     Pathname of ship server"
    echo "         -M     maintenance level data"
    echo "         -V     AIX version of the ptfs - valid entries are 3 or 4."
    echo "                Defaults to 3 if the -V flag is not used or if"
    echo "                the entry does not begin with a 4"
    echo "         -n     NO EXECUTE!  list only"
    echo "         -R     Resend ptf's to CCSS only"
    echo "         -E     Resend ptf data to RETAIN"
    echo "         -S     Not calculate size data for PTF"
    echo "         -d     Exclude debug mode for RETAIN information"
    echo "         -x     Exclude supersede check on ship server"
    echo "         -O     Indicate OEM ptfs"
    echo "         -A     Update abstract on RETAIN screen only"
    echo "      -?/-h     Displays promote_ship syntax"
    echo "        ptf     At least one valid ptf or ccss file name"
    exit $rc
}


#######################################################################
#  Abort is called when an error occurs after WORK space is allocated
#######################################################################
abort() {
    echo "\n$cmd: promote failed!"
    cd                            # make sure not in work dir
    rm -rf $WORK
    exit 1
}


#######################################################################
#  function check_file_permissions verifies appropriate access
#    to prod, ship,,,
#######################################################################
check_file_permissions() {
    #
    # checks common to all options
    #
    if [ ! -d "$prod_dir" ]; then
        echo "$cmd: The production directory '$prod_dir' is invalid"
        rc=20
    fi
    if [ ! -d "$ship_dir" ]; then
        echo "$cmd: The ship directory '$ship_dir' is invalid"
        rc=21
    fi
    
    # resolve absolute paths for prod and ship directories
    if [ "$prod_dir" != "" ]
    then
        prod_dir=`cd $prod_dir; pwd`
    fi
    if [ "$ship_dir" != "" ]
    then
        ship_dir=`cd $ship_dir; pwd`
    fi

    # Check for unique directory name
    if [ "$prod_dir" = "$ship_dir" ]; then
        echo "$cmd: The production and ship directories are the same"
        rc=22
    fi

    # check to make sure that the prod and ship directories are the same release
    print "$prod_dir $ship_dir" | grep "/320/" | grep "/4.1/"
    if [ $? = 0 ]
    then
        echo "$cmd: the prod and ship directories cannot be different releases"
        rc=23
    fi

    #
    # checks for runs with -n flag
    #
    if [ $execute = "no" ]; then
        #
        # check the prod library index
        #
        if [ ! -r "$prod_dir/index" ]; then
            echo "$cmd: The prod indexfile '$prod_dir/index' must be readable"
            rc=30
        fi
        #
        # check the ship library index
        #
        if [ ! -r "$ship_dir/index" ]; then
            echo "$cmd: The ship indexfile '$ship_dir/index' must be readable"
            rc=31
        fi
        #
        # exit from check_file_permissions function
        #
        return
    fi

    #
    # checks for runs with -R or -E flags
    #
    if [ "$resend" = "yes" -o "$retain_flag" = "yes" ]; then
        if [ ! -w "$ship_dir" ]; then
            echo "$cmd: The ship directory '$ship_dir' must be writable"
            rc=40
        fi
        if [ ! -w "$LOG_FILE" ]; then
            echo "$cmd: The log file '$LOG_FILE' must be writable"
            rc=41
        fi
        #
        # exit from check_file_permissions function
        #
        return
    fi

    #
    # checks for runs without any flags (actual promotions)
    #
    if [ ! -w "$prod_dir" ]; then
        echo "$cmd: The prod directory '$prod_dir' must be writable"
        rc=50
    fi
    if [ ! -w "$prod_dir/index" ]; then
        echo "$cmd: The prod index '$prod_dir/index' must be writable"
        rc=51
    fi

    if [ ! -w "$ship_dir" ]; then
        echo "$cmd: The ship directory '$ship_dir' must be writable"
        rc=52
    fi
    if [ ! -w "$ship_dir/index" ]; then
        echo "$cmd: The ship index '$ship_dir/index' must be writable"
        rc=53
    fi

    if [ ! -w "$LOG_FILE" ]; then
        echo "$cmd: The log file '$LOG_FILE' must be writable"
        rc=54
    fi
}


#######################################################################
#  xmit_ptf is called to send ptfs to the host.  As this program can
#       take a while, it is run in background and the output is mailed
#       to the user if the xmit fails ( -n option is not run background
#######################################################################
call_xmit_ptf() {
    if [ $execute = no ]; then
        xmit_ptf -V $version -s $ship_dir -n $ptf_filenames
    else
        # execute xmit_ptf in background.
        # the nohup (no hang up) guarantees that the job will keep
        #  running even if the user logs out
        echo "$cmd:-----------------------------------------------------------"
        echo "$cmd: Transmission of ptf's to CCSS is running in background."
        echo "$cmd: If any errors are detected, distribution will be nofified."
        echo "$cmd: Logging off will not affect this background job."
        echo "$cmd:-----------------------------------------------------------"
        nohup xmit_ptf -V $version -s $ship_dir $ptf_filenames > /dev/null &
    fi
}


#######################################################################
#  function call_SendPtf invokes the RETAIN update procedure
#       with the apar info created by build_apar_data
#######################################################################
call_SendPtf() {
    oemoption=""
    if [ "$oemflag" = "yes" ]; then
        oemoption="-O"
    fi
    abstractoption=""
    if [ "$abstractflag" = "yes" ]; then
        abstractoption="-A"
    fi
    maintoption=""
    if [ "$mldata" != "" ]; then
	maintoption="-M ${mldata}"
    fi
    if [ $execute = no ]; then
        if [ "$sizeflag" = "yes" ]; then
            update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file -n "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames
        else
            update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file -n -S "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames
        fi
    else
        # Execute SendPtf in background.
        # The nohup (no hang up) guarantees that the job will keep
        # running even if the user logs out
        echo "$cmd:-----------------------------------------------------------"
        echo "$cmd: Update of RETAIN is running in background."
        echo "$cmd: If any errors are detected, distribution will be notified."
        echo "$cmd: Logging off will not affect this background job."
        echo "$cmd:-----------------------------------------------------------"
        if [ $debug = yes ]; then
            if [ "$sizeflag" = "yes" ]; then
                nohup update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames > $HOME/RETAIN.dbg &
            else
                nohup update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file -S "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames > $HOME/RETAIN.dbg &
            fi
        else
            if [ "$sizeflag" = "yes" ]; then
                nohup update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames > /dev/null &
            else
                nohup update_RETAIN -V $version -s $ship_dir -p $prod_dir -f $apar_file -S "$oemoption" "$maintoption" "$abstractoption" $ptf_filenames > /dev/null &
            fi
        fi
    fi
}


#######################################################################
#  function to build_apar_data needed by Send_ptf command
#######################################################################
build_apar_data() {
    for ptf_filename in $ptf_filenames
    do
        # Strip off the .ptf extension
        ptf_name=`basename "$ptf_filename" .ptf`

        # Unpack the infofile from the ccss image 
        rm -f $WORK/ptf_infofile
        rm -f $WORK/ptf_infofile.eb
        ccss_unpack -c $ship_dir/$ptf_filename -i $WORK/ptf_infofile.eb

        # convert the infofile to ascii
        dd if=$WORK/ptf_infofile.eb of=$WORK/ptf_infofile conv=ascii cbs=80 >/dev/null 2>&1

        # Build the apar line for this ptf_name
        echo "$ptf_name \c" >> $apar_file

        # get the apar numbers from the infofile
        # the lines with apars on them have the following format:
        #  APARTXT apar = abstract
        cat $WORK/ptf_infofile |
        grep '^APARTXT ' |              
        while read apartxt apar trash
        do
            echo "$apar \c" >> $apar_file
        done
        if [ $? != 0 ]; then
            abort
        fi
        echo "" >> $apar_file
    done
}


#######################################################################
#  function to build shipped ptf status file used by distribution
#    to confirm RETAIN and CCSS were updated correctly.
#######################################################################
update_shipped_ptfs() {
    for ptf_filename in $ptf_filenames
    do
        ptf_name=`basename $ptf_filename .ptf`
        echo "# $ptf_name `date +"%y""%m""%d""%H""%M""%S"` `hostname` `whoami`"  \
           >  $WORK/shipped_ptfs
        sed -n -e "/$ptf_name/p" $apar_file >> $WORK/shipped_ptfs
        cat $WORK/shipped_ptfs >> $ship_dir/shipped_ptfs
    done
}


#######################################################################
#  check the ship server has enough space for all ptfs with their
#  index entries.
#######################################################################
check_space() {
    # Check target directory space
    echo $ship_dir | grep "^/afs" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        total_space=`fs lq $ship_dir | awk ' NR > 1 {print $2} '`
        used_space=`fs lq $ship_dir | awk ' NR > 1 {print $3} '`
        unused_space=`expr $total_space - $used_space`
        unused_bytes=`expr $unused_space \* 1024`
    else
        os_version=`uname -v`
        if [ "$os_version" = "4" ]
        then
            df -k  > $WORK/outfile
        else
            df > $WORK/outfile
        fi
        cat $WORK/outfile | awk ' NR > 1 {print $3 " " $7}' > $WORK/outfile1
        pdir=$ship_dir
        found=0
        while [ $found = 0 ]
        do
            grep "$pdir$" $WORK/outfile1 > $WORK/outfile2
            if [ $? = 0 ]; then
                found=1
                unused_space=`cat $WORK/outfile2 | awk '{print $1}'`
                unused_bytes=`expr $unused_space \* 1024`
            else
                pdir=`dirname $pdir`
            fi
        done
    fi
    # Calculate total size needs for all ptfs with their index entries
    total_size=0
    > $WORK/ptf_index
    for MIF in $ptf_filenames
    do
        ptf_size=`ls -l $prod_dir/$MIF | awk '{print $5}'`
        total_size=`expr $total_size + $ptf_size`
        MIF_name=`expr "$MIF" : "^\(.*\)\.ptf$" `
        grep "^[^:]*:$MIF_name:" $prod_dir/index >> $WORK/ptf_index
    done
    index_size=`ls -l $WORK/ptf_index | awk '{print $5}'`
    total_size=`expr $total_size + $index_size`
    if [ $total_size -gt $unused_bytes ]; then
        echo "$cmd: The space of $ship_dir is not enough"
        echo "$cmd: for all ptfs with their index entrie"
        echo "$cmd: Please check with your system administrator to increate"
        echo "$cmd: the filesystem space"
        echo "$cmd: Process terminated!"
        abort
    fi
}


#######################################################################
#  function to move ptf files from prod to ship and update
#    the index files appropriately.
#######################################################################
move_ptfs() {
    # Move the files from prod to ship
    for ptf_filename in $ptf_filenames
    do
        cp -p $prod_dir/$ptf_filename $ship_dir
        if [ $? != 0 ]; then
            echo -n "$cmd: error occurred moving $ptf_filename"
            echo " from $prod_dir to $ship_dir"
            abort
        fi

        MIF_name=`expr "$ptf_filename" : "^\(.*\)\.ptf$" `
      
        # Update index file on target directory
        grep -v "^[^:]*:$MIF_name:" $ship_dir/index > $WORK/ship_index
        mv $WORK/ship_index $ship_dir/index
        grep "^[^:]*:$MIF_name:" $prod_dir/index >> $ship_dir/index
        if [ $? -eq 0 ]; then
            grep -v "^[^:]*:$MIF_name:" $prod_dir/index > $WORK/prod.temp
            if [ -s $WORK/prod.temp ]; then
                mv $WORK/prod.temp $prod_dir/index 2> /dev/null
                if [ $? -eq 0 ]; then
                    rm $prod_dir/$ptf_filename
                    if [ $? -ne 0 ]; then
                        echo "$cmd: Cannot remove ptf $MIF_name from $prod_dir"
                        echo "$cmd: Process terminated!"
                        abort
                    fi
                else
                    echo "$cmd: Fail to remove index entries for $MIF_name on"
                    echo "$cmd: $prod_dir directory"
                    echo "$cmd: PTF $MIF_name is on $ship_dir and $prod_dir"
                    echo "$cmd: and the index file on $ship_dir has been updated"
                    echo "$cmd: Please recover this problem"
                    echo "$cmd: Process terminated!"
                    abort
                fi
            else
                if [ ! -f $WORK/prod.temp ]; then
                    echo "$cmd: Fail to remove index entries for $MIF_name on"
                    echo "$cmd: $prod_dir directory"
                    echo "$cmd: PTF $MIF_name is on $ship_dir and $prod_dir"
                    echo "$cmd: and the index file on $ship_dir has been updated"
                    echo "$cmd: Please recover this problem"
                    echo "$cmd: Process terminated!"
                    abort
                else
                    mv $WORK/prod.temp $prod_dir/index 2> /dev/null
                    if [ $? -eq 0 ]; then
                        rm $prod_dir/$ptf_filename
                        if [ $? -ne 0 ]; then
                            echo "$cmd: Cannot remove ptf $MIF_name from $prod_dir"
                            echo "$cmd: Process terminated!"
                            abort
                        fi
                    else
                        echo "$cmd: Fail to remove index entries for $MIF_name on"
                        echo "$cmd: $prod_dir directory"
                        echo "$cmd: PTF $MIF_name is on $ship_dir and $prod_dir"
                        echo "$cmd: and the index file on $ship_dir has been updated"
                        echo "$cmd: Please recover this problem"
                        echo "$cmd: Process terminated!"
                        abort
                    fi
                fi
            fi
        else
            echo "$cmd: Fail to update index file on $ship_dir for $MIF_name"
            echo "$cmd: But, the ptf is on $ship_dir and $prod_dir"
            echo "$cmd: Please recover the correct index file by running gen_ptf_index command"
            echo "$cmd: Process terminated!"
            abort
        fi
    done
}


#######################################################################
#  Begin SHELL logic
#######################################################################

set -u # Unset var's are treated as errors

# Initialize variables
excludeSuperFlag=""
resend="no"
retain_flag="no"
execute="yes"
debug="yes"
prodflag="no"
shipflag="no"
prod_dir=""
ship_dir=""
version=""
sizeflag="yes" 
oemflag="no"
abstractflag="no"
cmd=`basename $0`
rc=0
start_dir=`pwd`
WORK=/tmp/${cmd}_work.$$.`date +"%y""%m""%d""%H""%M""%S"`
apar_file="$WORK/apar_file"
mldata=""

# Check for no parameters
if [ $# -eq 0 ]
then
    rc=1
    syntax
fi
cmdline=""

# Validate all options on the command line
set -- `getopt "p:s:M:V:nxRESdOAh?" $*`
if [ $? != 0 ]; then
    # no error message necessary as getopt already printed one
    rc=2
    syntax
fi

# Parse the command line
while [ $1 != "--" ]; do                # Set vars, based on params.
    case $1 in
        -p) 
            shift
            prod_dir=$1
            prodflag="yes"
            shift;;
        -s)
            shift;
            ship_dir=$1
            shipflag="yes"
            shift;;
        -M)
            shift;
            mldata=$1
            shift;;
        -V)
            shift;
            version=$1
            shift;;
        -R)
            resend="yes"
            shift;;
        -E)
            retain_flag="yes"
            shift;;
        -S)
            sizeflag="no"
            shift;;        
        -n)
            execute="no"
            shift;;
        -d)
            debug="no"
            shift;;
	-O)
	    oemflag="yes"
	    shift;;
	-A)
            abstractflag="yes"
	    shift;;
	-x)
	    excludeSuperFlag="yes"
	    shift;;
        -h)
            syntax;;
    esac
done
shift                                   # Get past the '--' flag.

# check version to determine prod and ship default directories
echo $version | grep "^3" >/dev/null
rc3=$?
echo $version | grep "^4" >/dev/null
rc4=$?
if [ "$version" = "" -o "$rc3" = "0" ]
then
    version=32
elif [ "$rc4" = "0" ]
then
    version="41"
else
    echo "$cmd: Invalid version entered with the -V flag."
    echo "$cmd: Defaulted to processing AIX version 3 ptfs."
    version="32"
fi

# source in the defaults
type "$version"_pkg_environment > /dev/null 2>&1
[ $? = 0 ] && . "$version"_pkg_environment

# if the prod directory was not set on the command line, then set it to the
# environment variable
if [ "$prod_dir" = "" ]
then
    prod_dir=$PROD_DIRNAME
fi

# if the ship directory was not set on the command line, then set it to the
# environment variable
if [ "$ship_dir" = "" ]
then
    ship_dir=$SHIP_DIRNAME
fi

# Check if any ptf's were given
if [ $# -eq 0 ]; then
    echo "$cmd: at least one ptf_filename is required"
    rc=10
fi

# verify user has necessary access to servers, etc.
check_file_permissions


if [ "$prodflag" = "yes" ]; then
    cmdline="$cmdline -p $prod_dir"
fi
ship_dir=`cd $ship_dir; pwd`
if [ "$shipflag" = "yes" ]; then
    cmdline="$cmdline -s $ship_dir"
fi
if [ "$resend" = "yes" ]; then
    cmdline="$cmdline -R"
fi
if [ "$retain_flag" = "yes" ]; then
    cmdline="$cmdline -E"
fi
if [ "$debug" = "yes" ]; then
    cmdline="$cmdline -d"
fi
if [ "$sizeflag" = "no" ]; then
    cmdline="$cmdline -S"
fi
if [ "$excludeSuperFlag" = "yes" ]; then
    cmdline="$cmdline -x"
fi

# Check if any ptf's were given
if [ $# -ne 0 ]; then
    cmdline="$cmdline $*"
fi

# verify the maintenance level data.  It is assumed here that the -A flag is not
# used for version 4 PTFs, since the -M flag is not used for version 4 PTFs.
if [ "$version" = "32" ]; then
    if [ "$mldata" = "" ]; then
    	echo "$cmd: Please specify the \"-M\" option with the maintenance level data"
    	echo "$cmd: The format of the maintenance level should be \"=xxxx\", \">xxxx\""
    	echo "$cmd: or \"-----\". The \"xxxx\" represents 4-digit number"
    	rc=60
    else
    	if [ "$mldata" != "-----" ]; then
        	echo "$mldata" | grep "^[>=]\{1\}[0-9]\{4\}" > /dev/null 2>&1
        	if [ $? -ne 0 ]; then
            	echo "$cmd: The format of the maintenance level should be \"=xxxx\", \">xxxx\""
            	echo "$cmd: or \"-----\". The \"xxxx\" represents 4-digit number"
            	rc=60
        	fi
        fi
    fi
    cmdline="$cmdline -M $mldata"

    if [ "$abstractflag" = "yes" ]; then
    	if [ "$mldata" = "" ]; then
        	echo "$cmd: The flag \"-M\" should be used with the flag \"-A\"".
        	rc=60
    	else
        	cmdline="$cmdline -A"
    	fi
    fi
fi

# Give syntax if error was found above
if [ "$rc" != "0" ]; then
    syntax
fi

# process ptf filenames ( now that directories are verified ok )
ptf_filenames=""
for ptf_filename in $*
do
    # strip any path and strip .ptf if given and then add .ptf to end
    ptf_filename="`basename $ptf_filename .ptf`.ptf"

    # check if file exists
    if [ "$resend" = "no" -a "$retain_flag" = "no" ];then
        # check for it on prod
        if [ ! -f "$prod_dir/$ptf_filename" ]; then
            echo "$cmd: input ptf $ptf_filename not found in $prod_dir"
            rc=11
        else
            if [ ! -r "$prod_dir/$ptf_filename" ]; then
                echo "$cmd: input ptf $ptf_filename must be readable"
                rc=11
            fi
        fi
    else
        # check for it on ship
        if [ ! -f $ship_dir/$ptf_filename ]; then
            echo "$cmd: input ptf $ptf_filename not found in $ship_dir"
            rc=12
        else
            if [ ! -r $ship_dir/$ptf_filename ]; then
                echo "$cmd: input ptf $ptf_filename must be readable"
                rc=12
            fi
        fi
    fi

    # if same ptf occurs twice, issue warning and continue
    echo "$ptf_filenames" | grep "$ptf_filename"  > /dev/null
    if [ $? = 0 ]; then
        echo "$cmd: Warning: $ptf_filename was found on the command line twice"
        echo "$cmd: Process continuing"
    else
        ptf_filenames="$ptf_filenames $ptf_filename"
    fi
done

# Give syntax if error was found above in ptf_filenames
if [ "$rc" != "0" ]; then
    syntax
fi


# Create temporary work space with unique directory name
# using process id and current date and time
rm -rf $WORK
mkdir $WORK
if [ $? != 0 ]; then
    echo "$cmd: Could not make work directory $WORK"
    echo "$cmd: aborting process"
    abort
fi
chmod 777 $WORK


#
# Resend ptfs if -R was given
#
if [ "$resend" = "yes" ]; then
    call_xmit_ptf
fi


#
# Update RETAIN if -E was given
#
if [ "$retain_flag" = "yes" ]; then
    build_apar_data
    call_SendPtf
fi


#
# Process normal promotions (neither -R or -E)
#
if [ "$retain_flag" = "no" -a "$resend" = "no" ]; then
    # Check to see if the files are to be promoted
    if [ "$execute" = "no" ]; then
        if [ -z "$excludeSuperFlag" ]
        then
	    prereq_list -s$ship_dir -p$prod_dir $ptf_filenames
        else
	    prereq_list -x -s$ship_dir -p$prod_dir $ptf_filenames
        fi
        echo "$cmd: FINISHED. No files promoted"
    else
        # Check prerequisites downstream (prereq_list with -c option)
        if [ -z "$excludeSuperFlag" ]
        then
            prereq_list -c -s$ship_dir -p$prod_dir $ptf_filenames
        else
            prereq_list -x -c -s$ship_dir -p$prod_dir $ptf_filenames
        fi
        if [ $? != 0 ]; then
            # no error message necessary as prereq_list already gave one
            abort
        fi

        # check target directory space
        check_space

        # move ptfs and update index files
        move_ptfs

        # send ptfs to CCSS using xmit_ptf
        call_xmit_ptf

        # update distribution's status file
        build_apar_data
        update_shipped_ptfs

        # Update ptf data in RETAIN.
        #  apar data is obtained from the distribution status file.
        call_SendPtf

        echo "$cmd: FINISHED. ALL files successfully promoted"
    fi
fi

# Log all the promote information
if [ "$execute" = "yes" ]; then
    # Log all the promote information
    echo "`date` `hostname` `whoami` $cmd $cmdline"  >> $LOG_FILE
fi

# Reposition cd out of work directory
# Remove temporary files and directories
while true 
do
    ps -ef | grep "update_RETAIN" | grep -v "grep" > /dev/null 2>&1
    if [ $? -eq 1 ]; then
        cd
        rm -rf $WORK 
        break
    fi
    sleep 5
done &

# Graceful exit
sync
exit 0
# END OF SHELL
