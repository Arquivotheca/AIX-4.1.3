#!/bin/ksh
# @(#)06	1.6  src/bos/usr/lib/nim/methods/c_popxop.sh, cmdnim, bos411, 9428A410j  1/20/94  14:59:46
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_popxop.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
	
#---------------------------- c_popxop_exit       ------------------------------
#
# NAME: c_popxop_exit
#
# FUNCTION:
#		performs exit processing for the c_popxop method
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
c_popxop_exit ()
{
	# rm all tmp files
	/usr/bin/rm /tmp/$$* >/dev/null 2>/dev/null

} # end of c_popxop_exit

#---------------------------- error             --------------------------------
#
# NAME: error
#
# FUNCTION:
#		displays the specified message and exits with a 1
#
# EXECUTION ENVIRONMENT:
#		ksh
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			msg					= error message to display
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#		writes an error message to stderr
#-------------------------------------------------------------------------------
error ()
{
	# display the message
	print "\nc_popxop: $*\n" 1>&2

	cleanup

	# exit
	exit 1

} # end of error

#---------------------------- c_popxop.sh         ------------------------------
#
# NAME: c_popxop.sh
#
# FUNCTION:
#		ksh script to pop exit operations out of the specified xop file
#		if a key is specified, only those lines matching the key will be
#			operated on
#		the word "all" is a reserved key which indicates that all operations in
#			the file should be executed
#		if no key is specified, then the last line in the file will be used
#		the default operation will be to execute the lines which match the key;
#			this may be overriden by specifying the "ignore" parameter
#
# EXECUTION ENVIRONMENT:
#		executes in the ksh and is invoked from a NIM command via the pop_xop
#			function in the libnim.a library
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			file					= pathname of xop file
#			key					= string to match on
#			ignore				= a "1" if matches are NOT to be executed
#		global:
#
# RETURNS: (int)
#		0							= request performed
#		1							= unable to process request
#
# OUTPUT:
#-------------------------------------------------------------------------------

trap c_popxop_exit 0 1 2 6 15
umask 022

# initialize local variables
rc=0
ignore=""

# assuming that the awk script lies within the parent directory of this script
dir=${0%/*}
[[ "$dir" = "." ]] && dir=$PWD
dir=${dir%/*}

# debug mode?
if [[ "$1" = -d ]]
then
	DBG=TRUE
	shift
fi

# set parameters from command line
file=$1
key=$2
[[ -n "$key" ]] && [[ "$key" != all ]] && key="# $key #"
[[ "$3" = 1 ]] && ignore=$3

[[ -n "$DBG" ]] && /usr/bin/echo '-------- c_popxop ----------'
[[ -n "$DBG" ]] && \
	print "dir = $dir; file = $file; key = \"$key\"; ignore = $ignore;" 

# can we access the file?
if [[ ! -f "$file" ]]
then
	# nothing to do
	[[ -n "$DBG" ]] && print "$file doesn't exist; nothing to do!"
	exit 0
elif [[ ! -s "$file" ]]
then
	# nothing to do
	[[ -n "$DBG" ]] && print "$file is empty; nothing to do!"
	exit 0
elif [[ ! -w "$file" ]]
then
	# can't write to the file
	error "can't write to the input file \"$file\""
elif [[ ! -r "$dir/awk/popxop.awk" ]]
then
	# can't access the awk script
	error "unable to access the $dir/awk/popxop.awk script"
fi

# "all" specified?
if [[ "$key" = "all" ]]
then
	[[ -n "$DBG" ]] && print "executing ALL operations"

	# execute all operations in the file
	/usr/bin/mv $file /tmp/$$.xops >/dev/null 2>&1 || \
		error "unable to mv $file /tmp/$$.xops"

elif [[ -n "$key" ]]
then
	[[ -n "$DBG" ]] && print "calling popxop.awk to match on key"

	# find all the lines which match the key
	/usr/bin/awk -f $dir/awk/popxop.awk last= key="$key" xops=/tmp/$$.xops \
		$file >/tmp/$$ 2>/dev/null && /usr/bin/mv -f /tmp/$$ $file >/dev/null 2>&1

else
	[[ -n "$DBG" ]] && print "no key - calling popxop.awk with last=1"

	# use the last line in the file
	/usr/bin/awk -f $dir/awk/popxop.awk last=1 key="" xops=/tmp/$$.xops $file \
		>/tmp/$$ 2>/dev/null && /usr/bin/mv -f /tmp/$$ $file >/dev/null 2>&1
fi

# anything left in $file?
if [[ ! -s "$file" ]]
then
	[[ -n "$DBG" ]] && \
		print "after pop, $file is empty; removing it"
	/usr/bin/rm $file >/dev/null 2>&1
fi

# anything exit operations to perform?
[[ -n "$DBG" ]] && [[ ! -s /tmp/$$.xops ]] && \
	print "no matches were found; nothing left to do"

# skip execution?
[[ -n "$DBG" ]] && [[ -n "$ignore" ]] && \
	print "ignore option set - skipping execution"

# execute the matched operations?
if [[ -z "$ignore" ]] && [[ -s /tmp/$$.xops ]]
then
	/usr/bin/cat /tmp/$$.xops | { 
	while read xop
	do
		[[ -n "$DBG" ]] && print "executing: \"$xop\""

		# execute the operation
		[[ -n "$xop" ]] && eval $xop 
	done
	}
fi

# all done
exit $rc

