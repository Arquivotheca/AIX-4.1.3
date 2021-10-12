#!/bin/bsh
# @(#)59    1.4  src/bos/usr/bin/sysdumpdev/smutil/dump_smutil.sh, cmddump, bos411, 9428A410j  6/30/94  15:58:29

#
# COMPONENT_NAME:   CMDDUMP   SMIT shell interface program
#
# FUNCTIONS:  interfaces to sysdumpdev, cat, catpr
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

[ $# -gt 0 ] || { exit 0 ; }

PROGNAME=`basename $0`
TMPFILE=/tmp/dump_sm$$
CLEANUP1="rm -f $TMPFILE"
CLEANUP2="mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename>/dev/null 2>&1"
trap "$CLEANUP1; $CLEANUP2; exit 5 " 2

UTILNAME=$1
shift

case $UTILNAME in

#
# sysdumpdev
# put output from sysdumpdev in cmd_to_discover format
#
sysdumpdev)

USAGE_SYSDUMPDEV="
sysdumpdev -lL

-l    list current dump device settings
-L    show info on previous dump through DMP_IOCSTAT ioctl
-S    show possible dump devices
"

OPTIONS="`getopt SHlL $*`"
[ $? -eq 0 ] || { echo "$USAGE_SYSDUMPDEV" ; exit 1 ; }

set -- $OPTIONS

FROMFILE=
TOFILE=
for i in $* ; do
	ARG=$1
	case $ARG in
	-l)
		OPT=-l
		;;
	-L)
		OPT=-L
		;;
	-S)
		OPT=-S
		;;
	-H)
		echo "$USAGE_SYSDUMPDEV"
		exit 1
		;;
	--)
		shift
		break
		;;
	*)
		echo "$PROGNAME: invalid argument $i"
		echo "$USAGE_SYSDUMPDEV"
		exit 1
		;;
	esac
	shift
done

case "$OPT" in
"")
	exit 0
	;;
-l)
	sysdumpdev -l > $TMPFILE
	set `grep primary < $TMPFILE`
	PRIMARY=$2
	set `grep secondary < $TMPFILE`
	SECONDARY=$2
	echo "\
#_primary:_secondary
$PRIMARY:$SECONDARY"
	exit 0
	;;
-L)
        # Read the output of sysdumpdev -L command
        # subtitute from the beginning of the line till
        # ":" and any number of blanks after that with nothing
        # i.e.: Get rid of the label fiels.
	mv /var/adm/ras/copyfilename /var/adm/ras/copyfilename.org >/dev/null 2>&1

        sysdumpdev -L 2>&1 | sed 's/^.*:  *//' | {
        read blank; read blank
        read device
        read major
        read minor
        read size bytes
        read day month date time year
        read status
        # check to make sure the device is not a remote dump file.
        echo $device | fgrep : >/dev/null
        rc=$?
        if  [ $rc -eq 0 ]
        then
          echo `dspmsg cmddump.cat -s 1 536 "Copy a remote dump file is not allowed.\n"`
	  mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename >/dev/null 2>&1
          exit 1
        else
        echo "\
#_devicename:_madev:_midev:_size:_timestamp:_status:
$device:$major:$minor:$size:$month $date $year:$status:"
	  mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename >/dev/null 2>&1
          exit 0
        fi
        }
	;;

-S)
	LIST=
	[ -c /dev/rhd7 ] && \
		LIST="$LIST/dev/rhd7         DUMP MINIDISK\n"
	[ -c /dev/rmt0 ] && \
		LIST="$LIST/dev/rmt0         MAGTAPE\n"
	[ -c /dev/sysdumpnull ] && \
		LIST="$LIST/dev/sysdumpnull  DUMP DISABLED\n"
	echo "$LIST\c"
	exit 0
	;;
esac
;;

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
		qprt $PRINTER
		RC=$?
	fi
	exit $RC
;;

esac

