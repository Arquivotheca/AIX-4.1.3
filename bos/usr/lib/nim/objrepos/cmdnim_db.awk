# @(#)82	1.1  src/bos/usr/lib/nim/objrepos/cmdnim_db.awk, cmdnim, bos411, 9428A410j  10/5/93  08:14:35
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./lib/nim/objrepos/cmdnim_db.awk
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


/extern/{gsub(/\[\]/,"",$4);print $1 " " $2 " " $3 " *" $4;next};{print}
