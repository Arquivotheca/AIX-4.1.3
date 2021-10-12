#!/bin/ksh
# @(#)89	1.1  src/bldenv/pkgtools/getfs.sh, pkgtools, bos412, GOLDA411a 8/18/94 13:40:47
#
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS: copy_fileset
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: getfs
#
# DESCRIPTION: Creates a list of filesets that have changed.
#              Creates a list of changed files and filesets in bos.rte
#              Touches .il files in src/packages that have changed files
#
# PRECONDITIONS:This function expects the following files or variables:
#                - FILESET.db 
#                - lmupdatelist (paramater) the update list from the PTF build
#                - illist      (paramater) the list of .il files for the release
#                - BLD_LOG_DIR  full path to dir where FILESET.list
#                               and FILESET.bosrte files are created
#
# POSTCONDITIONS: list of changed filesets (FILESET.list)
#              list of files and filesets in bos.rte (FILESET.bosrte)
#              list of files in lmupdatelist that could not be found
#              in FILESET.db (FILESET.list.err)
#
# SIDE EFFECT:  all of the .il files that have changed files are touched
#              unless the -f flag is specified.
#
# RETURNS: 0
#

#
# Temporary files
#
tmpfile1=/tmp/temp1.$$
tmpfile2=/tmp/temp2.$$
tmpfile3=/tmp/temp3.$$
tmpfile4=/tmp/temp4.$$
tmpfileset=/tmp/temp5.$$
tmpfilebosrte=/tmp/temp6.$$
lmupdatelist=""

#
# FILESET.list
#
FILESET_LIST=$BLD_LOG_DIR/FILESET.list
FILESET_BOSRTE=$BLD_LOG_DIR/FILESET.bosrte
rm -fr $FILESET_BOSRTE $FILESET_LIST.err
if [ ! -w $FILESET_LIST ] ; then
	touch $FILESET_LIST || {
		print -u2 "Could not touch \"$FILESET_LIST\" "
		exit 1
	}
fi

if [ ! -w $FILESET_BOSRTE ] ; then
	touch $FILESET_BOSRTE || {
		print -u2 "Could not touch \"$FILESET_BOSRTE\" "
		exit 1
	}
fi

#
# Get the paramaters
#
if [ "$1" = "-f" ] ; then
        fileset_ONLY="yes"
        only="ONLY "
        shift
else
        fileset_ONLY="no"
        only=
fi
if [ ! -f "$1" -o ! -f "$2" ] ; then
	echo "Please specify the command line parameters. Processing stopped."
        echo "Usage: $(basename $0) <-f> lmupdatelist il_list"
        exit
fi
lmupdatelist=$1
illist=$2

#
# Clean up function to remove the temporary files
#
function cleanup
{
	print -u2 "Cleanup in progress ... \c"
	rm -f $tmpfile1 $tmpfile2
	rm -f $tmpfileset $FILESET_LIST.X $tmpfile1.err
	print -u2 "done."
	exit
}

#
# function to add the new filesets to the FILESET.list in the log directory
#
function copy_fileset
{
	print -u2 "Copying fileset list to log directory ... \c"
	#
	# Copy the new filesets into the fileset list for the current bldcycle
	#
	cp -f $FILESET_LIST $FILESET_LIST.X
	cat $tmpfileset >> $FILESET_LIST.X
	sort -u $FILESET_LIST.X > $FILESET_LIST
	cat $tmpfile1.err >> $FILESET_LIST.err
	print -u2 "done."
}

trap cleanup 2 3	# Clean up tmp files before exiting

#
# Create the list of filesets that have changed files from the build
#
wc $lmupdatelist | read g h
print -u2 "Creating fileset list ${only} from list of $g files"
for i in $(cat $lmupdatelist)
do
	echo $i | fgrep /inst_updt/ > /dev/null && {
		j=$(echo $i | sed s,$build/ship/power,, | sed s,inst_updt/,,)
		lmfile=${j%/*}
	}
	if [ -z "$lmfile" ] ; then
		lmfile=${i#*.}
	fi
	#print -u2 "what_fileset $lmfile"
	# what_fileset should be in the PATH
	# $DB variable in what_fileset should be the location of FILESET.db
	what_fileset $lmfile >> $tmpfile1
	RC=$?
	if [ $RC != 0 ] ; then
		print -u2 "ERROR: what_fileset $lmfile"
		echo $lmfile >> $tmpfile1.err
	fi
	lmfile=
done

#
# Any line that does not start with a '/' was an error
# and should be ignored
grep '^/' $tmpfile1 > $tmpfile2

#
# Get the fileset name
#
# Output from what_fileset is:
# filename fileset_name type_of_file
# create two files. One is the list of all the 
# filesets that have changed. Second is the list of
# files and filesets that have changed in bos.rte.
# sort the second file by filesets to remove duplicate entries.

cat $tmpfile2 | while read a fs b
do
	echo $fs >> $tmpfileset
	if [[ $fs = bos.rte* ]]
	then
       	    echo $fs $a >> $tmpfilebosrte
	fi
done
	sort -u $tmpfilebosrte > $FILESET_BOSRTE
#
# If we only want to create the fileset list then copy the files and exit
#
if [ $fileset_ONLY = "yes" ]; then
        copy_fileset
	cleanup
fi

#
# Create the list of .il files for the changed filesets
# tempfileset has a list of all the filesets that have changed 
# one per line.
#
print -u2 "Creating .il file list"
for fs in $(cat $tmpfileset)
do
	# check for any blank lines
        if [ ! -n "$fs" ]; then
                continue
        fi
        x=$fs
	# look for $x.il in the illist file. 
	# If a match is found then add it to tmpfile3
	# else recursively strip the right small pattern and 
	# look for that till a match is found. 
        while true
        do
                fgrep $x.il $illist >> $tmpfile3 || {
                        x=${x%.*}
                        if [ -n "$x" ]; then
                                continue
                        fi
                }
                break
        done
done

# sort to get rid of duplicate lines
sort -u $tmpfile3 > $tmpfile4

# make sure that src/packages dir exists
# before touching il files
cd $src/packages || {
        print -u2 "Could not cd to $src/packages"
        cleanup
}

#
# Touch the .il files
# 
wc $tmpfile4 | read g f h
print -u2 "Touching $g .il files"
for i in $(cat $tmpfile4)
do
        if [ -f "$i" ]; then
                print -u "\ttouch $i"
                touch "$i"
        fi
done

trap 2 3        # reset

# copy the files in log directory.
copy_fileset
cleanup
exit 0
