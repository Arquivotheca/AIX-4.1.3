#!/bin/ksh
# @(#)62	1.7  src/bos/usr/ccs/bin/ranlib/ranlib.sh, cmdaout, bos411, 9430C411a 7/27/94 17:02:12
#
# COMPONENT_NAME: CMDAOUT (ranlib command)
#
#  FUNCTIONS: ranlib
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#  All Rights Reserved
# Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#	ranlib script
#		Call the ar command with appropiate options
#
AR_S="ar -s"
AR_H="ar -h"
AR_CMD=$AR_S
RET_CODE=0
#
case $1 in
	-t)	AR_CMD=$AR_H
		shift
esac

#Throw away the '--' for consistency with other commands.
case $1 in
	--)	shift
esac
#
if [ $# -eq 0 ]; then
	dspmsg ranlib.cat 1 "Usage: ranlib [-t] [--] file ...\n"
	exit 1
fi
#
for OPT in "$@"
do
	$AR_CMD $1
	if [ $? -ne 0 ] ; then
		dspmsg ranlib.cat 2 "ranlib: 0654-601 Execution of ar failed\n"
		RET_CODE=`expr $RET_CODE + 1`
	fi
	shift
done

if [ $RET_CODE -ne 0 ]; then
	dspmsg ranlib.cat 1 "Usage: ranlib [-t] [--] file ...\n"
fi
exit $RET_CODE
