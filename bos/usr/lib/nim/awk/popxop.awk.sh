# @(#)26	1.1  src/bos/usr/lib/nim/awk/popxop.awk.sh, cmdnim, bos411, 9428A410j  8/25/93  16:42:06
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/popxop.awk
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

#---------------------------- popxop.awk        --------------------------------
#
# NAME: popxop.awk
#
# FUNCTION:
#		manipulates the specified xops file by returning either:
#			1) the last line in the file when the variable "list" equals 1
#			2) all lines which match the variable "key"
#
# EXECUTION ENVIRONMENT:
#		this is an awk script and is parsed by awk; this script is invoked from
#			the popxop.sh method
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			xops					= name of file to put matching lines into
#			key					= string to match on
#			last					= equals "1" if only the last line to be matched
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#		writes non-matching lines to stdout
#		writes matching lines to the file specified by the "xops" variable
#-------------------------------------------------------------------------------


BEGIN {
	line = ""
}

END {
	# last line only?
	if ( last == 1 )
		print line > xops
}

last == 1 {
	# we want the last line only
	if ( line != "" )
		print line
	line = $0
	next
}

{	# keyword in this line?
	if ( index($0,key) )
		print >> xops
	else
		print
}
