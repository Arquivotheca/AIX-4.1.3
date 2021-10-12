#! /bin/sh
# @(#)55	1.1  src/bos/usr/lpp/bosinst/scripts/scrip555/defaults.sh, bosinst, bos411, 9428A410j 11/15/89 08:44:17
#
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: defaults
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

cfd=`dirname $0`
DB=`dirname $cfd`/db
PATH=`dirname $0`:.:$PATH
HOME=$DB/..

export DB PATH HOME
