#!/usr/bin/ksh
# @(#)94	1.3  src/bos/usr/lib/pios/pioinit.sh, cmdpios, bos411, 9428A410j 11/9/93 18:16:33
#
#   COMPONENT_NAME: CMDPIOS
#
#   FUNCTIONS: none
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
# Clean up printer flags files
/usr/bin/rm -f /var/spool/lpd/pio/@local/flags/* >/dev/null 2>&1 
/usr/lib/lpd/pio/etc/piodmgr -h >/dev/null
exit 0
