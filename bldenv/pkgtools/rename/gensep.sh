#!/bin/sh
# @(#)41        1.4  src/bldenv/pkgtools/rename/gensep.sh, pkgtools, bos412, GOLDA411a 8/7/91 14:03:25
# COMPONENT_NAME: gensep.sh
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

#####################################################################
# This program will process the ptf_pkg file to create the          #
# wrk_ptf_pkg file and create ifrecs, corecs and prereqs that may   #
# be in the ptf_pkg file.                                           #
#####################################################################

# Put the name of this program into variable myName for error messages.
# Also create a variable myFill of spaces same length for 2nd line.

myName=`echo $0 | sed -e 's/.*\/\([^/]*\)$/\1/'`
myFill=`echo $myName | sed -e 's/./ /g'`

cp wk_ptf_pkg ptf_pkg			# Copy the file
rm -f wk_ptf_pkg			# Ensure that no files are present
rm -f *.pre.pre				# that are being appended.

rm -f *.irec.pre
rm -f *.crec.pre
rm -f *.prec.pre
rm -f opts_rec

nl=`wc -l < ptf_pkg`			# Get number of lines in file

bi=0					# Bad flag
j=1					# Counter

# Process all records in input file
while [ "$j" -le "$nl" ]; do


    ##############################################################
    # Assign all fields on the line to a variable                #
    ##############################################################

    # Get ptf number, single field
    ptf=`awk 'BEGIN {FS="|"}; NR == '$j'{print $1; exit}' < ptf_pkg`

    # Get apar(s), multiple entries, blank separated
    apar=`awk 'BEGIN {FS="|"}; NR == '$j'{print $2; exit}' < ptf_pkg`

    # Get file name, single field
    fn=`awk 'BEGIN {FS="|"}; NR == '$j'{print $3; exit}' < ptf_pkg`

    # Get option name, multiple entries, blank separated
    opt=`awk 'BEGIN {FS="|"}; NR == '$j'{print $4; exit}' < ptf_pkg`

    # Get ifrec(s), multiple entries, blank separated.
    irec=`awk 'BEGIN {FS="|"}; NR == '$j'{print $5; exit}' < ptf_pkg`

    # Get corecs(s), multiple entries, blank separated
    crec=`awk 'BEGIN {FS="|"}; NR == '$j'{print $6; exit}' < ptf_pkg`

    # Get prerecs(s), multiple entries, blank separated
    prec=`awk 'BEGIN {FS="|"}; NR == '$j'{print $7; exit}' < ptf_pkg`

    ##############################################################
    # Check selected variables that must be present to continue  #
    # processing.                                                #
    ##############################################################

    if [ "$ptf" = "" ]; then
       echo "$myName: The PTF number field is missing"
       bi=1 
    fi

    if [ "$apar" = "" ]; then 
       echo "$myName: The APAR number field is missing" 
       bi=1 
    fi

    if [ "$fn" = "" ]; then
       echo "$myName: The file name field is missing" 
       bi=1 
    fi

    if [ "$opt" = "" ]; then
       echo "$myName: The option name field is missing" 
       bi=1 
    fi

    if [ "$bi" -ne "0" ]; then			# If any required fields
       echo "$myName: Processing stopped"	# are missing, exit the
       exit 15					# program.
    fi

    ################################################################
    #                                                              #
    ################################################################
       
    opt_name=`echo $opt | awk '{print $1}'` 	# Get option name
    oi=0
    while [ $oi = 0 ]; do			# (Process all options


	echo $opt_name >> opts_rec
 
	if [ "$irec" != "" ]; then		# (If there are ifreqs
	  echo $irec | 
  awk 'BEGIN { RS = ""; FS = "\n" }	# Rec sep is null line, fld sep is nl.
	{ i = split($1, arr, " ")	# Split 2nd line into names in array.
	  for ( j in arr )		# Print and delete each array entry.
	  print arr[j] ; delete arr[j] }
  ' | cat  >> $opt_name.irec.pre        # Put unique APARs into 'apars' file.

	fi					# end ifreqs)

	if [ "$crec" != "" ]; then		# (If there are coreqs
	  echo $crec | 
  awk 'BEGIN { RS = ""; FS = "\n" }	# Rec sep is null line, fld sep is nl.
	{ i = split($1, arr, " ")	# Split 2nd line into names in array.
	  for ( j in arr )		# Print and delete each array entry.
	  print arr[j] ; delete arr[j] }
  ' | cat  >> $opt_name.crec.pre        # Put unique APARs into 'apars' file.
	fi					# end corecs)

	if [ "$prec" != "" ]; then		# (If there are prereqs
	  echo $prec | 
  awk 'BEGIN { RS = ""; FS = "\n" }	# Rec sep is null line, fld sep is nl.
	{ i = split($1, arr, " ")	# Split 2nd line into names in array.
	  for ( j in arr )		# Print and delete each array entry.
	  print arr[j] ; delete arr[j] }
  ' | cat  >> $opt_name.prec.pre        # Put unique APARs into 'apars' file.
	fi					# end prereqs)
	if [ "$opt_name" = "$opt" ]; then
	   oi=1 
	fi
	opt=`echo $opt | sed -e 's/[^ ]* //'`
	opt_name=`echo $opt | awk '{print $1}'`
    done					# end options)


    j=`expr $j + 1`				# Increment counter

    ##############################################################
    # Write the wk_ptf_pkg file, but first substitute commas     #
    # for blanks in the apar and option fields.                  #
    ##############################################################

    napar=`echo $apar | sed -e s'/ /\,/g'`	# Substitute blanks with
    nopt=`echo $opt | sed -e s'/ /\,/g'`	# commas (,)

    echo "$ptf" "$napar" "$fn" "$nopt" >> wk_ptf_pkg
done

# Sort the opts_rec file to get a list of unique options for this run

sort -u opts_rec -o opts_rec


# Read the opts_rec file to get the options processed.

cat opts_rec | while read o_n rst; do		# (Process all options
    if [ -f $o_n.irec.pre ]; then
       sort -u $o_n.irec.pre -o $o_n.irec.pre
       cat $o_n.irec.pre | while read tptf junk; do
	   irec=`grep $tptf $TOP/PTF/$BLDCYCLE/ptfoptions`
	   if [ "$?" = "0" ]; then
	      $BLDENV/usr/bin/genicp ifreq "$irec" "$o_n"
	   else
	      echo "$myName: Cannot locate ptf $tptf in ptfoptions file"
	   fi
	done
    fi

    if [ -f $o_n.crec.pre ]; then
       sort -u $o_n.crec.pre -o $o_n.crec.pre
       cat $o_n.crec.pre | while read tptf junk; do
	   crec=`grep $tptf $TOP/PTF/$BLDCYCLE/ptfoptions`
	   if [ "$?" = "0" ]; then
	      $BLDENV/usr/bin/genicp coreq "$crec" "$o_n"
	   else
	      echo "$myName: Cannot locate ptf $tptf in ptfoptions file"
	   fi
	done
    fi
   
    if [ -f $o_n.prec.pre ]; then
       sort -u $o_n.prec.pre -o $o_n.prec.pre
       cat $o_n.prec.pre | while read tptf junk; do
	   prec=`grep $tptf $TOP/PTF/$BLDCYCLE/ptfoptions`
	   if [ "$?" = "0" ]; then
	      $BLDENV/usr/bin/genicp prereq "$prec" "$o_n"
	   else
	      echo "$myName: Cannot locate ptf $tptf in ptfoptions file"
	   fi
	done
    fi

done						# end all options)
