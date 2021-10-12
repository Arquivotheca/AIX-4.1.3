#!/bin/bsh
# @(#)32       1.1  src/bos/kernext/dlc/smit/sm_name_hdr.awk.sh, dlccfg, bos411, 9428A410j 4/1/94 08:51:40
#
# COMPONENT_NAME: (dlccfg) Data Link Control Configuration
#
# FUNCTIONS:  This awk script is used by the SMIT sm_name_hdr stanza when
#             adding a dlc device, under cmd_to_classify.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp.  1987, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
echo $1 | awk '{ print $2 }'
