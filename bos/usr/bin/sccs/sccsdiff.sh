#! /bin/bsh
# @(#)92 1.9 9/22/93 12:44:17
#
#  COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
#
#  FUNCTIONS: N/A
#
#  ORIGINS: 3, 10, 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#  All Rights Reserved
# Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

sid1= sid2= num= pipe= files=

for i in $@
do
	case $i in

	-*)
		case $i in

		-r*)
			if [ ! "$sid1" ]
			then
				sid1=$i
			elif [ ! "$sid2" ]
			then
				sid2=$i
			else
				dspmsg sccs.cat 2 'Usage:  %s -rSID1 -rSID2 \[-p\] \[-sNumber\] SCCSFile...\n\tCompares two versions of a Source Code Control System (SCCS) file.\n' $0 1>&2
				exit 1
			fi
			;;
		-s*)
			num=`expr "$i" : '-s\(.*\)'`
			;;
		-p*)
			pipe=yes
			;;
		*)
			files=
			break
			;;
		esac
		;;
	*s.*)
		files="$files $i"
		;;
	*)
		dspmsg sccs.cat 1 '  %1$s: %2$s is not an SCCS file.\n\tSpecify an SCCS file.\n' $0 $i 1>&2
		;;
	esac
done

if [ -z "$files" ] || [ ! "$sid1" ] || [ ! "$sid2" ]
then
	dspmsg sccs.cat 2 'Usage:  %s -rSID1 -rSID2 \[-p\] \[-sNumber\] SCCSFile...\n\tCompares two versions of a Source Code Control System (SCCS) file.\n' $0 1>&2
	exit 1
fi

trap "rm -f /tmp/get[abc]$$;exit 1" 0 1 2 3 15

for i in $files
do
	if get -s -p -k $sid1 $i > /tmp/geta$$
	then
		if get -s -p -k $sid2 $i > /tmp/getb$$
		then
			bdiff /tmp/geta$$ /tmp/getb$$ $num > /tmp/getc$$
		fi
	fi
	if [ ! -s /tmp/getc$$ ]
	then
		if [ -f /tmp/getc$$ ]
		then
			dspmsg sccs.cat 3 '%s: There are no differences between the specified\n\tversions of these files.\n' $i >/tmp/getc$$
		else
			exit 1
		fi
	fi
	if [ "$pipe" ]
	then
		msg=`dspmsg sccs.cat 4 '%1$s: %2$s versus %3$s\n' $i $sid1 $sid2`
		pr -h "$msg" /tmp/getc$$
	else
		cat /tmp/getc$$
	fi
done

trap 0
rm -f /tmp/get[abc]$$
