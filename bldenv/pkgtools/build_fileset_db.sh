#!/bin/ksh
# @(#)29        1.2  src/bldenv/pkgtools/build_fileset_db.sh, pkgtools, bos412, GOLDA411a 10/11/94 14:24:18
#
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS: 
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
# NAME: build_fileset_db
#
# DESCRIPTION: This script will read all the inslist in a given source tree
#              and produce a colon separated database with three fields.
#
#	          [file]:[fileset]:[type]
#
#              where : type is the a one of two character field for 
#              the type of file
#		f or F	normal file
#		d or D	directory
#		v or V	normal file of volatile size (config file)
#		s or S	symbolic link
#		h or H 	hard link
#		i or I	inventory only (this file is created on the fly
#			during the installation, not actually shipped)
#		a or A	apply only (this file is deleted during the 
#			the installation but is actually shipped)
#		t or T	TCB flags set.  This is only valid as the second
#			field to an entry.
# 
# PRECONDITIONS: The inslist should exist in the src tree.
#
# POSTCONDITIONS: Creates a colon separated database with three fields
#                 in $BASE/SELFIX/LOG/TD_BLDCYCLE directory.
# SIDE EFFECTS: None
#
# RETURNS:
#

#set the command name for error messages
myName=${0##*/}
myFill=`echo $myName | sed -e 's/./ /g'`

# Check to see if the environment variables are set 
if [ -z "{BASE}" ]; then
   print -u2 "$myName: ERROR: Environment variable BASE is not set."
   print -u2 "$myFill  Please correct processing stopped."
   exit
fi
if [ -z "{TD_BLDCYCLE}" ]; then
   print -u2 "$myName: ERROR: Environment variable BASE is not set."
   print -u2 "$myFill  Please correct processing stopped."
   exit
fi

bldlogdir=${BASE}/selfix/LOG/${TD_BLDCYCLE}
FILESET_DB=${bldlogdir}/FILESET.db
src=${BASE}/src

# Remove the temporary files
tmpfile1=/tmp/temp1.$$
tmpfile2=/tmp/temp2.$$
tmpfile3=/tmp/temp3.$$
tmpfile4=/tmp/temp4.$$

rm -f $tmpfile1 $tmpfile2 $tmpfile3 $tmpfile4 > /dev/null 2>&1
rm -f $FILESET_DB > /dev/null 2>&1

find $src/packages -type f -print | grep "\.il$" | grep -v bos.rte.il > $tmpfile1

sort $tmpfile1 > $tmpfile2

cat $src/packages/bos/rte/bos.rte.il | grep -v "^#" > $tmpfile3
awk '{ print $5":bos.rte."$NF":"$1":bos/rte/bos.rte.il" }' $tmpfile3 | sed 's/bos.rte.rte/bos.rte/' >> $tmpfile4

cat $tmpfile2 | while read inslist
do
	cat $inslist | grep -v "^#" > $tmpfile3
	a=`basename $inslist`
	fileset=`echo $a | sed "s/\.il$//"`
	filesetdir=`echo $fileset | sed "s/\./\//g"`
	awk '{ print $5":"fileset":"$1":"ildir"/"ilfile }' \
	     fileset=$fileset ildir=$filesetdir ilfile=$a $tmpfile3 >> $tmpfile4
done

sort $tmpfile4 > $FILESET_DB

echo "The new version of the FILESET.db file has been placed in \"${FILESET_DB}\" "

rm $tmpfile2 $tmpfile3 $tmpfile1 $tmpfile4

exit 0
