# @(#)67	1.1  src/bos/usr/lib/nim/awk/image_data.awk.sh, cmdnim, bos411, 9428A410j  12/7/93  10:21:12
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/awk/image_data.awk
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
#                             image_data.awk
#
# updates an image.template file for the specified mount point
# parameters:
#		fs_name		= mount point
#		LPs			= number of logical partitions
#		FS_SIZE		= number of blocks
#
###############################################################################

function print_stanza() {

	if ( (stanza == "") || (num <= 0) )
		return

	if ( update_lv )
	{	for (i=1; i <= num; i++)
			if ( (s[ i,1 ] == "LPs=") && (s[ i,2 ] < LPs) )
					s[ i,3 ] = "\tLPs= " LPs
	}

	if ( update_fs )
	{	for (i=1; i <= num; i++)
			if ( (s[ i,1 ] == "FS_SIZE=") && (s[ i,2 ] < FS_SIZE) )
				s[ i,3 ] = "\tFS_SIZE= " FS_SIZE
			else if ( (s[ i,1 ] == "FS_MIN_SIZE=") && (s[ i,2 ] < FS_SIZE) )
				s[ i,3 ] = "\tFS_MIN_SIZE= " FS_SIZE
	}

	print stanza

	for (i=1; i <= num; i++)
		print s[ i,3 ]
}

END {

	print_stanza()

}

/^[ 	]*$/ { 

	print_stanza()

	in_stanza = 0
	update_lv = 0
	update_fs = 0

	stanza = ""
	num = 0

	print

	next
}

$1 ~ /^[^ 	]+:$/ {
	in_stanza = 1
	stanza = $1

	next
}

in_stanza == 1 {

	if ( (stanza == "lv_data:") && ($1 == "MOUNT_POINT=") && ($2 == fs_name) )
		update_lv = 1
	else if ( (stanza == "fs_data:") && ($1 == "FS_NAME=") && ($2 == fs_name) )
		update_fs = 1

	num++
	s[ num,1 ] = $1
	s[ num,2 ] = $2
	s[ num,3 ] = $0

	next
}

{
	print
}
