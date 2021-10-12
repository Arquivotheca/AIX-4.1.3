#!/usr/bin/bsh -
# @(#)46 1.7  src/bos/usr/lbin/mkwhatis/mkwhatis.sh, cmdman, bos411, 9428A410j 9/3/93 13:16:53
#
# COMPONENT_NAME: (CMDMAN) commands that allow users to read online 
# documentation
#
# FUNCTIONS: 
#
# ORIGINS: 26, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#

trap "rm -f /tmp/whatisx.$$ /tmp/whatis$$; exit 1" 1 2 13 15
MANDIR=${1-/usr/share/man}
rm -f /tmp/whatisx.$$ /tmp/whatis$$
if test ! -d $MANDIR ; then exit 0 ; fi
cd $MANDIR
touch whatis > /dev/null 2>&1
if [ $? -ne 0 ] ; then exit 1 ; fi
top=`pwd`
for i in cat1 cat2 cat3 cat4 cat5 cat6 cat7 cat8 catn catl man1 man2 man3 man4 \
         man5 man6 man7 man8 mann manl
do
	if [ -d $i ] ; then
		cd $i
	 	if test "`echo *`" != "*" ; then
			echo * | xargs /usr/lbin/getNAME
		fi
		cd $top
	fi
done >/tmp/whatisx.$$
if [ -s /tmp/whatisx.$$ ] ; then
	sed  </tmp/whatisx.$$ >/tmp/whatis$$ \
		-e 's/\\-/-/' \
		-e 's/\\\*-/-/' \
		-e 's/\\f[PRIB0123]//g' \
		-e 's/\\s[-+0-9]*//g' \
		-e 's/.TH [^ ]* \([^ 	]*\).*	\([^-]*\)/\2(\1)	/' \
		-e 's/	 /	/g'
else
	rm -f /tmp/whatisx.$$ whatis
	exit 1
fi
/usr/bin/expand -24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100 \
	/tmp/whatis$$ | sort | /usr/bin/unexpand -a > whatis
if [ $? -ne 0 ] ; then exit 1 ; fi
chmod 664 whatis >/dev/null 2>&1
rm -f /tmp/whatisx.$$ /tmp/whatis$$
exit 0
