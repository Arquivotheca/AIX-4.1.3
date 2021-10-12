#!/bin/sh
# @(#)00        1.6  src/bldenv/pkgtools/rename/gendir.sh, pkgtools, bos412, GOLDA411a 6/12/91 10:32:31
# COMPONENT_NAME: gendir.sh
#
# FUNCTIONS:
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
#########################################################################
# This shell will generate directory stanzas for each unique directory  #
# required to be sure that all files in the inslist will have a         #
# directory present.  							#
# Not all LPP's create their own directories, but use directories	#
# created by other LPP's.						#
#########################################################################
# The creation of these stanzas is used by genset only and does not	#
# go out with the LPP							#
#########################################################################

rm -f ins_dir

if [ ! -s ins_exp ]; then
   echo "gendir: The ins_exp file is missing or empty"
   exit 5
fi

# Extract all of the file names from the stanzas in the ins_exp file
# and strip off the last level (file name) to get a list of directories

grep -p "FILE" ins_exp | grep  "^/" \
| sed -e 's/\/[^/]*$//' > /tmp/f.$$

# Extract all of the directory names from the stanzas in the ins_exp file
# and strip off the colon (:) an the end of the line.

grep -p "DIRECTORY" ins_exp | grep  "^/" \
| sed -e 's/://' >> /tmp/f.$$  

# Sort to eliminate duplicates.

cat /tmp/f.$$ | sort -u -o /tmp/f.$$

# Read the list of unique directories and iteratively strip off
# the last level to create an entry for the parent directory.

cat /tmp/f.$$ | while read f1 f2; do

    ci=0
    while [ $ci -eq 0 ]; do
	var1=`echo $f1 | sed -e 's/\/[^/]*$//'`	# Strip off a level
	if [ $var1 ]; then			# If not null then
	   echo $var1 >> /tmp/f2.$$		# write and assign
	   f1=$var1				# to f1 variable.
	 else
	  ci=1
	fi 
    done

done

# Combine the initial directory names with the parent directory names.
# Sort to eliminate duplicates.

cat /tmp/f.$$ >> /tmp/f2.$$
sort -u /tmp/f2.$$ -o /tmp/f3.$$

rm -f /tmp/f2.$$
rm -f /tmp/f.$$

# Read the list of unique directories and create a stanza for each one

cat /tmp/f3.$$ | while read f1 dumy; do
echo "$f1"":" >> ins_dir
echo "   class = apply" >> ins_dir
echo "   type = DIRECTORY" >> ins_dir
echo "" >> ins_dir
done
rm -f /tmp/f3.$$
exit
