# 
# @(#)93	1.6  src/bos/usr/bin/mh/uip/version.sh, cmdmh, bos411, 9428A410j 6/15/93 14:01:36
# COMPONENT_NAME: CMDMH version.sh
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27  28  35 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
: run this script through /bin/sh

echo "char *version = \"MH [6.6 UCI] [RAND] [IBM]\";" > version.c;
