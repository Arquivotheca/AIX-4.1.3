#!/usr/bin/sh -
# @(#)14	1.12  src/bos/usr/bin/calendar/calendar.sh, cmdmisc, bos411, 9428A410j 9/7/93 18:37:02
#
# COMPONENT_NAME: (CMDMISC) miscellaneous commands 
#
# FUNCTIONS: calendar
#
# ORIGINS: 3, 26, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	calendar.sh - calendar command, uses calprog (in /usr/lbin or /usr/lib)
#  NAME: calendar [-]
#                                                                     
#  FUNCTION: Writes reminder messages to stdout.
# 	OPTIONS:
#	-	Calls calendar for everyone having calendar in their home 
#		directory and sends any reminders by mail.

PATH=/usr/lbin:/usr/bin:/usr/lib:/bin
_tmp=/tmp/cal$$
_mailmsg=/tmp/calmail2$$
_subj=/tmp/calmail1$$
ENOEXIST=1
MAILSUBJ=2
NOTFILE=3
USAGE=4
trap "rm -f ${_tmp} ${_mailmsg} ${_subj}" 0
trap exit 1 2 13 15
calprog > ${_tmp}
case $# in
0)      if [ -d calendar ]; then
                dspmsg calendar.cat $NOTFILE "%s: %s/calendar is a directory rather than a file\n" $0 `pwd`
                exit 1
	fi
	if [ -s calendar ]; then
		if [ -f /usr/ccs/lib/cpp -a -x /usr/ccs/lib/cpp ]; then
			/usr/ccs/lib/cpp calendar 2>/dev/null | egrep -f ${_tmp}
		else
			egrep -f ${_tmp} calendar
		fi;
	else
		dspmsg calendar.cat $ENOEXIST "%s: %s/calendar not found\n" $0 \
			`pwd`
	fi;;
*)	if [ "$1" != "-" ]; then 
		dspmsg calendar.cat $USAGE "Usage: calendar [-]\n"
		exit 1
	fi
	cat /etc/passwd | \
		sed 's/\([^:]*\):.*:\(.*\):[^:]*$/_dir=\2 _user=\1/' | \
		while read _token; do
			eval ${_token}	# evaluates _dir= and _user=
			if [ -s ${_dir}/calendar ]; then
				if [ -f /usr/ccs/lib/cpp -a \
					-x /usr/ccs/lib/cpp ]; then
					/usr/ccs/lib/cpp ${_dir}/calendar \
					2>/dev/null | \
					egrep -f ${_tmp} 2>/dev/null \
					> ${_mailmsg}
				else
					egrep -f ${_tmp} ${_dir}/calendar \
				 	2>/dev/null > ${_mailmsg}
				fi;
				if [ -s ${_mailmsg} ]; then
					_date=`date '+%D'`
					dspmsg calendar.cat $MAILSUBJ \
					"Subject: Calendar for %s\n" ${_date}\
					> ${_subj}
					cat ${_subj} ${_mailmsg} | mail ${_user}
				fi
			fi
		done;;
esac
exit 0
