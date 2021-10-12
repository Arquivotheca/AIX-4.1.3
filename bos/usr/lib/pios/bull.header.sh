#!/usr/bin/ksh
# @(#)67        1.3  src/bos/usr/lib/pios/bull.header.sh, cmdpios, bos411, 9428A410j 5/14/93 09:05:33
#
# COMPONENT_NAME: (CMDPIOS) Printer Backend
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# BULL SOURCE FILE
#

#
# Header page generator
#
# For wide printers, the banner is the user and the title
# but for narrow printers it is just the user.
#
COLUMNS=$1
ARG1=$PIOJOBNUM
ARG2=$PIOTO
ARG3=$PIOTITLE
CMD0=$PIOQNAME

echo "\n\n\n\n\n"
if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi
if [ "$COLUMNS" -gt 85 ]; then
    $banner "$ARG2" "$ARG3"
else
    $banner "$ARG2" 
fi
echo "\nRequest id: $ARG1    Printer: `basename $CMD0`\n\n`date`\n\f\c"
exit 0
