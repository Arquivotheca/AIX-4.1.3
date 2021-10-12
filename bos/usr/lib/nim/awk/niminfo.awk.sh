# @(#)24	1.1  src/bos/usr/lib/nim/awk/niminfo.awk.sh, cmdnim, bos411, 9428A410j  8/25/93  16:41:35
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/niminfo.awk
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

################################################################################
#
#                             niminfo.awk
#
# this awk script sets the specified variable in a niminfo file
# parameters:
#		name = variable name
#		value = variable value; when NULL, <name> will be removed from the file
###############################################################################

BEGIN {
	found = 0
}

END {
	# was variable found?
	if ( ! found )
	{	# make assignment or remove from file?
		if ( value != "" )
			print name "=" value
	}
}

/.*=.*/ {
	# separate name from value
	split( $0, tmp, "=" )

	# is this the variable we're looking for?
	if ( tmp[1] == name )
	{	# make assignment or remove from file?
		if ( value != "" )
			print name "=" value
		found = 1
		next
	}
}

{	# lines with no changes
	print
}

