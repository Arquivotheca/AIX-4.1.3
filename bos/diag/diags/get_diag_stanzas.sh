#!/bin/bsh
# @(#)76	1.3  src/bos/diag/diags/get_diag_stanzas.sh, cmddiag, bos411, 9428A410j 10/7/93 15:20:15
#
# COMPONENT_NAME: CMDDIAG  DIAGNOSTIC SUPERVISOR
#
# FUNCTIONS: Gather Diagnostic ODM stanzas of all disks.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   
###################################################################
if [ $1 ]
then
	odmdir=$1
else
	odmdir=`pwd`
fi
/usr/bin/odmshow PDiagDev PDiagAtt CDiagDev TMInput FRUB FRUs MenuGoal DAVars \
	> /tmp/diagodm.cre
ODMDIR=$odmdir /usr/bin/odmcreate -c /tmp/diagodm.cre >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "Unable to create empty diagnostics object class."
	rm -rf /tmp/diagodm.cre >/dev/null 2>&1
	exit 1
fi
/usr/bin/odmget -q"DClass=disk" PDiagDev > /tmp/diskdiag.stz
/usr/bin/odmget -q"DClass=disk" PDiagAtt >> /tmp/diskdiag.stz
/usr/bin/odmget -q"DType=badisk" PDiagDev >> /tmp/diskdiag.stz
/usr/bin/odmget -q"DType=badisk" PDiagAtt >> /tmp/diskdiag.stz

ODMDIR=$odmdir /usr/bin/odmadd /tmp/diskdiag.stz
if [ $? != 0 ]
then
	echo "Unable to populate diagnostics object class."
	rm -rf /tmp/diskdiag.stz >/dev/null 2>&1
	exit 1
fi

rm -rf /tmp/diagodm.cre >/dev/null 2>&1
rm -rf /tmp/diskdiag.stz >/dev/null 2>&1

exit 0
