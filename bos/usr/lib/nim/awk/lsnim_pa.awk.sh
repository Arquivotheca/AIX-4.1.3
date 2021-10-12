# @(#)08	1.2  src/bos/usr/lib/nim/awk/lsnim_pa.awk.sh, cmdnim, bos411, 9435D411a  9/1/94  19:26:55
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
#                             lsnim_pa.awk
#
# this awk script aligns output for lsnim "-pa<attr>" output
#
###############################################################################

# look for beginning of stanza
# NOTE: double colons have been appended to beginning of stanzas to 
# differentiate from similar looking strings in the rest of stanza 
$1 ~ /.*::$/ {
        gsub ("::", ":")
	print
	next
}

# add one tab for all other lines
{
	print "   " $0
}
