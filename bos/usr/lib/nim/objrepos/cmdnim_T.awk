#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/objrepos/cmdnim_T.awk
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

# creates string constants for corresponding attribute constants

$1 ~ /^ATTR_/ || $1 ~ /^STATE_/ || $1 ~ /^RESULT_/ {
	tag = $1
	$1 = ""
	print "#define " tag "_T\t\t" $0
}
