#! /bin/sh
# @(#)24	1.4  src/bos/usr/lpp/bosinst/scripts/scrip555/showchoices.sh, bosinst, bos411, 9428A410j 4/2/93 18:19:05
#
#   COMPONENT_NAME: BOSINST
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. `dirname $0`/defaults		# load defaults

echo `/bin/cat $DB/choices` 2> /dev/null |
xargs /bin/ls -Lld 2> /dev/null |
awk 'NF > 1 {print $NF,"\t",$5}'
