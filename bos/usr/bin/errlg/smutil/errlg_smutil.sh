#!/bin/bsh
# @(#)60    1.3  src/bos/usr/bin/errlg/smutil/errlg_smutil.sh, cmderrlg, bos411, 9428A410j  5/5/94  13:01:07

# COMPONENT_NAME:   CMDERRLG   SMIT shell interface program
#
# FUNCTIONS:  interfaces to cat, catpr
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

[ $# -gt 0 ] || { exit 0 ; }

trap "exit 5" 2

UTILNAME=$1
shift

case $UTILNAME in

cat)
	FILE=$1
	if [ "$FILE" = "" ] ; then
		cat
		RC=$?
	else
		FILE=`echo $FILE | sed 's/^Z//' `
		cat > $FILE
		RC=$?
	fi
	exit $RC
;;

catpr)
	PRINTER=$1
	if [ "$PRINTER" = "" ] ; then
		cat
		RC=$?
	else
		PRINTER=`echo $PRINTER | sed 's/^Z//' `
		if [ -n "$PRINTER" ] ; then
		PRINTER="-P $PRINTER"
		fi
		enq $PRINTER
		RC=$?
	fi
	exit $RC
;;

esac

