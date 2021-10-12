#!/bin/sh
# @(#)65	1.3  src/bos/usr/lpp/bos/mksysck.sh, cmdsadm, bos411, 9428A410j 5/30/91 15:02:40
# COMPONENT_NAME: (TCBADM) add device entries to SYSCK database
#
# FUNCTIONS: mksysck
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Clean up if I get killed
#
trap "rm -f /tmp/stanzas /tmp/devices; exit -1" 1 2 3 15

#
# Find all device entries in the /dev directory and see about finding
# the destinations of the symbolic links in there as well.
#
find /dev -print | xargs ls -lid > /tmp/devices
find /dev -type l -exec ls -l {} \; | \
	sed -e 's/.* -> \([^ ]*\)$/\1/' | \
	xargs ls -lid >> /tmp/devices

#
# Mung on the ls output and get the major and minor numbers all
# fixed up without commas and such
#
sed -e 's/^ *//' -e 's/\([0-9][0-9]*\), *\([0-9][0-9]*\)/\1,\2/' \
	-e 's/  */ /g' < /tmp/devices | \
	sort -u -o /tmp/devices +0n -1 +9d -10

#
# Feed the result to awk and let awk create the stanzas from the
# edited ls output.  TCBCK then adds those stanzas to its database.
#
awk -f /usr/lpp/bos/mksysck.awk /tmp/devices > /tmp/stanzas
tcbck -a -f /tmp/stanzas
status=$?

#
# Cleanup and exit with whatever the exit status from TCBCK was
#
rm -f /tmp/stanzas /tmp/devices
exit $status
