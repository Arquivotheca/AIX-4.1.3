#!/usr/bin/ksh
# @(#)04	1.2  src/bos/usr/lib/pios/piojetd.sh, cmdpios, bos411, 9428A410j 7/28/93 19:14:06
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

exec 1>/dev/null
hostname=$1
shift
trap "trap 15;kill -15 0;exit 0" 15
/usr/lib/lpd/piobe "$@" | /usr/lib/lpd/pio/etc/piohpnpf -x $hostname
if [ "$?" -ne 0 ]
then
	exit 64
fi
