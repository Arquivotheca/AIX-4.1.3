#!/bin/sh
# @(#)15        1.2  src/bldenv/sbtools/build/buildmp.sh, bldprocess, bos412, GOLDA411a 2/15/94 13:03:53
#
#   COMPONENT_NAME: BLDPROCESS
#
#   FUNCTIONS:
#
#   ORIGINS: 27,71
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
MP=_mp
export MP
build -r $*
