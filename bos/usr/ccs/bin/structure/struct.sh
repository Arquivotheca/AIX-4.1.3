#!/bin/bsh
# @(#)97	1.4  src/bos/usr/ccs/bin/structure/struct.sh, cmdprog, bos411, 9428A410j 6/15/90 22:56:37
# COMPONENT_NAME: (CMDPROG) Programming Utilites
#
# FUNCTIONS: 
#
# ORIGINS: 26; 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
trap "rm -f /tmp/struct*$$" 0 1 2 3 13 15
files=no
for i
do
	case $i in
	-*)	;;
	*)	files=yes
	esac
done

case $files in
yes)
	/usr/lib/struct/structure $* >/tmp/struct$$
	;;
no)
	cat >/tmp/structin$$
	/usr/lib/struct/structure /tmp/structin$$ $* >/tmp/struct$$
esac &&
	/usr/lib/struct/beautify</tmp/struct$$
