# @(#)89 1.2  src/bos/usr/lib/nim/objrepos/cmdnim_errors.awk, cmdnim, bos411, 9437A411a  9/10/94  17:15:09

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/objrepos/cmdnim_errors.awk
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

# this script translates the cmdnim_msgdefs.h file into a file which is
#		included by the NIM shell scripts in order to have the error message
#		definitions

/^#define ERR_/ { print $2 "=" $3 }
/^#define MSG_/ { print $2 "=" $3 }
