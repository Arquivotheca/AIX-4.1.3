#
# @(#)13	1.1  src/bos/diag/boot/rmfiles.sh, diagboot, bos411, 9428A410j 4/18/94 09:31:58
# COMPONENT_NAME: DIAGBOOT - DIAGNOSTIC DISKETTE
#
# FUNCTIONS: Script to remove all files before reading in new diskette.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	This shell script removes files from the ram disk that is
#	a part of the diagnostic standalone diskette package.  The
#	parameter to this command corresponds to a diskette number. 

if [ -x /etc/diagcleanup$1 ]
then
	/etc/diagcleanup$1 >>$F1 2>&1
fi
if [ -f /etc/diag$1.dep ]
then
	for i in `/bin/cat /etc/diag$1.dep`
	do
		rm -f $i	>>$F1 2>&1
		rm -f $i.Z	>>$F1 2>&1
	done
fi
exit 0
