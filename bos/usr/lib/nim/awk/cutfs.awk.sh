# @(#)25	1.1  src/bos/usr/lib/nim/awk/cutfs.awk.sh, cmdnim, bos411, 9428A410j  8/25/93  16:41:46
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/cutfs.awk
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
#                             cutfs.awk
#
# this awk script removes the specified /etc/filesystem stanza
# parameters:
#		mntpnt = point mount of stanza to remove
###############################################################################

# skip comments & empty lines
# NOTE that comments are delimited by "*"

$1 !~ /\*.*/ && $1 !~ /^$/ {
	if (match($1,/^\/.*:[ \t]*$/) > 0)
	{ # this is a mount point - remove the ":"
		mount_pnt = substr($1,1,length($1)-1)
		in_stanza = (mount_pnt == mntpnt)
	}
}

{
	if (! in_stanza)
		print
}

# blank line terminates stanza
$1 ~ /^$/ {
	in_stanza = 0
}

