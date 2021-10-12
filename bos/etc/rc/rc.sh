#!/bin/ksh
# @(#)06	1.17  src/bos/etc/rc/rc.sh, cfgetc, bos41J, 9521A_all 5/22/95 18:09:30
#
# COMPONENT_NAME: (CFGETC) Multi-user mode system setup
#
# FUNCTIONS: rc
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
################################################################

/usr/bin/dspmsg rc.cat 1 'Starting Multi-user Initialization\n'
PATH=/usr/sbin:/usr/bin:/usr/ucb:/etc::
ODMDIR=/etc/objrepos
export PATH ODMDIR

# Varyon all Volume Groups marked as auto-varyon.
# ( rootvg already varied on)
dspmsg rc.cat 2 ' Performing auto-varyon of Volume Groups \n'
cfgvg 1>&- 2>&-

# Activate all paging spaces in automatic list
# (those listed in /etc/swapspaces)
dspmsg rc.cat 3 ' Activating all paging spaces \n'
swapon -a

# Perform file system checks
# The -f flag skips the check if the log has been replayed successfully
fsck -fp

# Perform all auto mounts
dspmsg rc.cat 4 ' Performing all automatic mounts \n'
mount all

# Remove /etc/nologin if left behind by shutdown
rm -f /etc/nologin

# Running expreserve to recover vi editor sessions
/usr/lib/expreserve - 2>/dev/null

# Write a dummy record to file /usr/adm/sa/sa<date> to specify
# that system start up has occurred.
#MSG=`dspmsg rc.cat 7 'Write system start up record to /usr/adm/sa/sa'``date +%d`
#echo $MSG
#/usr/bin/su - root -c /usr/lib/sa/sadc /usr/adm/sa/sa`date +%d`

# Manufacturing post install process.
# This must be at the end of this file, /etc/rc.
if [ -x /etc/mfg/rc.preload ]
then
	/etc/mfg/rc.preload
fi

dspmsg rc.cat 5 'Multi-user initialization completed\n'
exit 0
