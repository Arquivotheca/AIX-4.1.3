#!/bin/sh
# @(#)39        1.3  src/bldenv/pkgtools/rename/genicp.sh, pkgtools, bos412, GOLDA411a 8/5/91 16:17:25
# COMPONENT_NAME: genicp.sh
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
#######################################################################
# Usage: genicp <type> <irec> <opt>
#        where, <type> - is the type of requisite record
#               <irec> - is the actual requisite record(s)
#               <opt>  - is the option(s) for the requisite records
#
# Formats: <irec> - U02,opt1,lpp U03,opt3
#          <opt>  - lpp-opt1 lpp-opt2  
#######################################################################
# This program will create lpp-option.pre.pre records that will
# contain either ifrec, corec or prerec records.
#######################################################################


# Put the name of this program into variable myName for error messages.
# Also create a variable myFill of spaces same length for 2nd line.

myName=`echo $0 | sed -e 's/.*\/\([^/]*\)$/\1/'`
myFill=`echo $myName | sed -e 's/./ /g'`

typ=""
irec=""
opt=""

if [ $# -ne 3 ]; then
  echo "$myName: Input parameters incorrect"
  exit 12
fi

typ=$1
irec=$2
opt=$3

# Must create/append a prereq file for each lpp-option.

oc=0
vopt=$opt
opt_name=`echo $vopt | awk '{print $1}'`	# Get 1st option

# Process each option of the LPP

while [ $oc -eq 0 ]; do  		# (Process each option

    if [ $opt_name ]; then		# If option present
	pc=0
	virec=$irec

	# Get 1st ifrec and lpp(s)
	#prerec=`echo $virec | awk '{print $1}'`
        prerec=$virec
	# Get 1st ifrec PTF number
#	ptf=`echo $prerec | awk 'BEGIN {FS=","}; {print $1; exit}'`
	ptf=`echo $prerec | awk '{print $1}'`

	# Strip off the first lpp of the ifrec
	# Strip off the PTF number
	#subrec=`echo $prerec | sed -e 's/[^\,]*\,//'`
	subrec=`echo $prerec | sed -e 's/[^\ ]*\ //'`

	# Change commas to blanks 
	subrec=`echo $subrec | sed -e 's/\,/ /g'`

	# Get lpp that is ifreced 
	tlp=`echo $subrec | awk '{print $1}'`

	# IF the same, only have one entry, which is an error.
	if [ "$ptf" = "$tlp" ]; then
	  echo "$myName: The $typ does not contain both a PTF and LPP"
	  exit 10 
	fi

	# Process each requisite for an option    
	while [ $pc -eq 0 ]; do			# (Begin requisite option

	  if [ $ptf ]; then			# If a requisite ptf number
		
	      ti=0
	      while [ $ti = 0 ]; do		# (Begin requisites
		 if [ $tlp ]; then		# If requisite present

		   echo "*"$typ" "$tlp" p="$ptf"" >> "$opt_name".pre.pre 

		   # If the same, then all requisite PTF numbers have
		   # been processed, set flag to exit requisite loop
		   if [ "$tlp" = "$subrec" ]; then
		     ti=1
		   fi

		   # Strip off the first lpp of the ifrec
		   subrec=`echo $subrec | sed -e 's/[^ ]* //'`

		   # Get lpp that is ifreced into var3
		  tlp=`echo $subrec | awk '{print $1}'`
		  else				# If requisite not present
	            ti=1			# set indicator to exit
		  fi				# requisite loop
               done				# end requisites)

	       # If the variables are the same, then all of the string
	       # has been processed, set indicator to exit.
	       if [ "$prerec" = "$virec" ]; then 
		  pc=1
	       fi

	       # Strip off the first ifrec in the string.
#	       virec=`echo $virec | sed -e 's/[^ ]* //'`

	       # Get next ifrec
#	       prerec=`echo $virec | awk '{print $1}'`

	       # Get 1st ifrec PTF number
#	       ptf=`echo $prerec | awk 'BEGIN {FS=","}; {print $1; exit}'`

	       # Strip off the first lpp of the ifrec
#	       subrec=`echo $prerec | sed -e 's/[^\ ]*\ //'`

	       # Change commas to blanks 
#	       subrec=`echo $subrec | sed -e 's/\,/ /g'`

	       # Get lpp that is ifreced into var3
#	      tlp=`echo $subrec | awk '{print $1}'`

	       # IF the same, only have one entry, which is an error.
#	       if [ "$ptf" = "$tlp" ]; then
#		  echo "$myName: The $typ does not contain both a PTF and LPP"
#		  exit 10 
#		fi


	   else				# Else requisite option not present.
	     pc=1			# Set flag to quit requisite option
	   fi				# loop.
	done				# Finished requisite option)

	# If the variables are the same, then all options have been
	# processed, set indicator to exit.
	if [ "$opt_name" = "$vopt" ]; then
	  oc=1				# Set flag to quit option loop.
	fi

	# Delete the first option name in the string.
	vopt=`echo $vopt | sed -e 's/[^ ]* //'` # Del 1st opt

	# Get the first option name in the string.
	opt_name=`echo $vopt | awk '{print $1}'`	  # Get 1st opt

    else				# Else option not present.
	oc=1				# Set flag to quit option loop.
    fi
done					# end option) 



