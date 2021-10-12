# @(#)27	1.3  src/bos/usr/lib/nim/awk/lsnim_P.awk.sh, cmdnim, bos411, 9438C411a  9/23/94  09:07:15
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/lsnim_P.awk
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
#                             lsnim_P.awk
#
# this awk script aligns columns for the output generated with "lsnim -P"
# this script expects output in the following format:
#		<name> = <value>
###############################################################################

function pinfo() {
	# print the current info
	for (i=1; i <= num; i++)
	{
		if ( (i == 1) && (in_stanza) )
			print field[i,1]
		else
		{	
			spaces1 = ""
			spaces2 = "   "
			for (j=1; j <= (max1 - len[i]); j++)
				spaces1 = spaces1 " "
			for (j=1; j <= len[i]; j++)
				spaces2 = spaces2 " "

			if ( in_stanza )
			{
				print "   " field[i,1] spaces1 " =" field[i,2]
				spaces2 = spaces2 "   "
			}
			else
				print field[i,1] spaces1 " =" field[i,2]

			for (j=3; j <= field[i,0]; j++)
				print spaces2 field[ i, j ]
		}
	}

	in_stanza = 0
	num = 0
}

END {
	pinfo()
}

# look for a heading
/^[^ 	]+:$/ {

	pinfo()
	in_stanza = 1

	field[ ++num, 0 ] = 1
	field[ num, 1 ] = $1
	field[ num, 2 ] = ""

	next
}

# look for start of a new line - first word will be delimited by "."
$1 ~ /\.[^ 	]+\./ {
	# remove the delimiters
	gsub( /\./, "", $1 )

	# save the current info
	field[ ++num, 0 ] = 2
	field[ num, 1 ] = $1
	$1 = ""
	field[ num, 2 ] = $0

	# field 1 the max?
	len[num] = length( field[num,1] )
	if ( len[num] > max1 )
		max1 = len[num]

	next
}

# if we get here, then this is a continuation of a previous value
{
	field[ num, 0 ] += 1
	field[ num, field[num,0] ] = $0
}

