#!/bin/ksh
# @(#)62        1.1  src/bos/usr/bin/errlg/convert_errnotify.sh, cmderrlg, bos411, 9428A410j 3/24/94 11:25:41
#
#   COMPONENT_NAME: cmderrlg
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Assumptions:
# The migration option of the bosinst has moved the
# the errnotify ODM data base to /tmp/bos/etc/objrepos
# Customers do not change the default notification method
# with their own method and add an another method for 
# the system objects (i.e. this script will not detect that).

# This script will delete all of the known system entries
# that were shipped with 3.X system.

cd /tmp/bos/etc/objrepos
rc=0

# Save the data base
if [ -s errnotify ]
  then
	cp errnotify errnotify.save

	# Set the ODMDIR to point to /tmp/bos/etc/objrepos
	ODMDIR=/tmp/bos/etc/objrepos
	export ODMDIR

	# The odmget steps just to make sure that the errnotify data base
	# has the 3.2 style.

	odmget -q"en_label=CHECKSTOP" errnotify >errnotify.sys.save 2>/dev/null
	odmget -q"en_label=CDROM_ERR2" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=CDROM_ERR4" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=CDROM_ERR6" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=TAPE_ERR3" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=TAPE_ERR6" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=MEMORY" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=MEM1" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=MEM2" errnotify >>errnotify.sys.save 2>/dev/null
	odmget -q"en_label=MEM3" errnotify >>errnotify.sys.save 2>/dev/null

	# if the errnotify.sys.save is not empty, assume this is the 3.2 errnotify

	if [ -s errnotify.sys.save ]
	then
		# Delete the known system entries
		odmdelete -o errnotify -q"en_label=CHECKSTOP" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=CDROM_ERR2" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=CDROM_ERR4" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=CDROM_ERR6" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=TAPE_ERR3" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=TAPE_ERR6" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=MEMORY" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=MEM1" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=MEM2" 2>&1 1>/dev/null
		odmdelete -o errnotify -q"en_label=MEM3" 2>&1 1>/dev/null

		# What left should be the entries which were added by the customer 
		# Save them in a file

		odmget errnotify >errnotify.customer.add 2>/dev/null
	
		# If the file is not empty then add the entries to the
		# /etc/objrepos/errnotify data base (NOTE: this step
		# also converts the old format to the new format.
	
		if [ -s errnotify.customer.add ]
 		 then
       		 	ODMDIR=/etc/objrepos
       		 	export ODMDIR
       		 	odmadd errnotify.customer 2>/dev/null
       		 	rc=$?
		fi
	fi
	# Clean up all the temporary files
	rm -f errnotify.save errnotify.sys.save errnotify.customer.add

	# If everything goes well, remove the Version 3 errnotify
	if [ $rc -eq 0 ]
  	then
		rm -f errnotify
	else rc=1
	fi
fi
exit rc 

