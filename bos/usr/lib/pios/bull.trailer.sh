#!/bin/ksh
# @(#)68        1.2  src/bos/usr/lib/pios/bull.trailer.sh, cmdpios, bos411, 9428A410j 4/22/92 15:38:15
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
# produces the "END banner for the trailer for Bull print outs
#
if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi
$banner "END"
exit 0
