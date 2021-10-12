# @(#)99	1.1  src/bos/usr/lib/nim/objrepos/cmdnim_strip.awk, cmdnim, bos411, 9428A410j  3/10/93  16:55:35
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/objrepos/cmdnim_strip.awk
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
#                             addfile.awk
#
# this awk script strips comments out the NIMdb.pd.add file
###############################################################################

$1 ~ /^#/ {
	next
}

/\/\*/ {
	comments = 1
}

{	if ( comments == 0 )
		print
}

/\*\// {
	comments = 0
}
