# @(#)01	1.1  src/bos/diag/boot/restore_files.sh, diagboot, bos411, 9428A410j 2/22/94 10:16:21
#
# COMPONENT_NAME: DIAGBOOT
#
# FUNCTIONS: Script to restore files from tape to RAM fs.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
########################################################################
if [ $2 ]
then
	if [ -f /tmp/v* ]
	then
	# remove files previously linked
		SCRIPT1=`echo /tmp/v*`
		TEMP_IFS=$IFS
		IFS='v'
		SCRIPT2=`echo $SCRIPT1`
		IFS=$TEMP_IFS
		for i in $SCRIPT2
		do
			k=$i
		done
		cd /
		while read file_removed
		do
			rm $file_removed	>>$F1 2>&1
		done</etc/tapediag$k.dep
	fi
fi
# Now restore the files needed to run DA or SA
tctl -f $TAPEDEV fsf `echo $1 | cut -c 1`

cd /
restbyname -qSf/dev/$BOOTDEV -x `cat /etc/tapediag$1.dep` >>$F1 2>&1
if [ $? -ne 0 ]
then
	exit 1
fi
rm -f /tmp/v*				>>$F1 2>&1
>/tmp/v$1
exit 0
