# @(#)29	1.2  src/bos/usr/lib/nim/awk/lsnim_l.awk.sh, cmdnim, bos411, 9438C411a  9/23/94  09:06:56
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/lsnim_l.awk
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
#                             lsnim_l.awk
#
# this awk script aligns columns for long_listing lsnim output ("-l" option)
###############################################################################

function pstanza() {
	for (i=1; i <= num; i++)
	{	if ( i == 1 )
			print line[i,0]
		else
		{	spaces1 = ""
			spaces2 = " "
			for (j=1; j <= (max1 - len[i]); j++)
				spaces1 = spaces1 " "
			for (j=1; j <= len[i]; j++)
				spaces2 = spaces2 " "
			print "   " line[i,0] spaces1 " =" line[i,2]
			for (j=3; j <= line[i,1]; j++)
				print "   " spaces2 spaces1 "  " line[i,j]
		}
	}

	num = 0
	in_stanza = 0
	max1 = 0
}

END {
	pstanza()
}

# look for beginning of stanza
$1 ~ /[^ 	]+:$/ {
	# print previous stanza
	pstanza()

	line[ ++num, 0 ] = $0
	line[ num, 1 ] = 0
	in_stanza = 1
	next
}

# look for beginning of attr/value pair
# attr name will be delimited by "." (ie, ".<attr name>.")
$1 ~ /\.[^ 	]+\./ && in_stanza == 1 {

	# remove surrounding "." from attr name
	gsub( /\./, "", $1 )

	# save the current line
	line[ ++num, 0 ] = $1
	line[ num, 1 ] = 2
	$1 = ""
	line[ num, line[num,1] ] = $0

	# field 1 the max?
	len[num] = length( line[num,0] )
	if ( len[num] > max1 )
		max1 = len[num]

	next
}

# look for lines which are continuations of attr value
in_stanza == 1 {
   line[ num, 1 ] = line[ num, 1 ] + 1
	line[ num, line[num,1] ] = $0
	next
}

# any other lines
{
	print
}

