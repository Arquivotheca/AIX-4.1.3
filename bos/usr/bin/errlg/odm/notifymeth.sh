#!/bin/sh
# @(#)86	1.8.1.4  src/bos/usr/bin/errlg/odm/notifymeth.sh, cmderrlg, bos411, 9428A410j 4/7/94 11:31:26

# COMPONENT_NAME:   CMDERRLG
#
# FUNCTIONS:  error notification script
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
# This script is invoked by the errdemon for every entry in the
# errnotify class added in /usr/lib/ras/notifylist
#
# /usr/lib/ras/notifymeth -l sequence_number
#
# The crecho routine puts a "\r" at the end of each line in case
# the console in in raw mode and does not convert \n to \r\n
#
# The intermediate tmpfile minimizes the time spent writing to the
# console and the possibility of having this message written on top of.
#
# Run notifymeth -i from the /etc/rc file after the errdemon has been
# started, bacause errdemon will clear the non-persistent (en_persistenceflg=0)
# objects when it is first started.
#

[ $# -gt 0 ] || exit 0

IPLPATH=`odmget -q"attribute=keylock and value=normal" CuAt`
if [ -z "$IPLPATH" ]; then
    # skip concurrent error notification if boot was from service position
    exit 0
fi

PROGNAME=`basename $0`
ERRNOTIFYADD=/usr/lib/ras/errnotify.add
ERRNOTIFYCRE=/usr/lib/ras/errnotify.cre
TMPFILE=/tmp/errnotify$$
CONSOLE=/dev/console
ERRPT=/usr/bin/errpt
LANG_ERRD=$LANG
[ -z "$LANG_ERRD" -o "$LANG_ERRD" = "" ] && LANG_ERRD=En_US
DSPMSG=/usr/bin/dspmsg
ODMADD=/usr/bin/odmadd
ODMCREATE=/usr/bin/odmcreate
ODMDELETE=/usr/bin/odmdelete
[ -z "$ODMDIR" -o "$ODMDIR" = "" ] && ODMDIR=/etc/objrepos

if [ "$_VERSION2" = "_VERSION2" ] ; then   # override when run on local machine
	CONSOLE=/dev/tty1
	ERRPT=errpt
	CATDIR=$HOME/cat2
	DSPCAT=dspcat2
	ODMADD=odmadd
	ODMCREATE=odmcreate
	ODMDELETE=odmdelete
fi

# We like the output messages with proper indentation. "echo" interpret 
# "\t" that means all indentations disappear.
# To solve the problem I incorporate cio code in to crecho.

crecho()
{

	echo "$1\c" >> $TMPFILE
	echo "\r" >> $TMPFILE
        #  echo "$1" | cio >> $TMPFILE
}

cio()
{

	while read LINE ; do
		echo "$LINE\c"
		echo "\r"
	done
}

USAGE="
Usage:
$PROGNAME -i -l seqno err_label resource

default errnotify method

   -i        initialize errnotify object class
	     from $ERRNOTIFYADD (/etc/rc).

   -c        clear all non-persistent objects from
             the errnotify class.

   -l n l r  print recommend actions to the console

   -r rsrc   resource information 

   -t label  error notification label
"

OPTIONS="`getopt Hicl:t:r: $*`"
[ $? -eq 0 ] || { echo "$USAGE" ; exit 1 ; }

set -- $OPTIONS

SEQNO=
LABEL=
RESOURCE=
for i in $* ; do
	ARG=$1
	case $ARG in
	-c)
		$ODMDELETE -o errnotify -q "en_persistenceflg = 0"
		exit $?
		;;
	-i)
		if [ ! -f $ODMDIR/errnotify ] ; then
			$ODMCREATE $ERRNOTIFYCRE 
			[ $? -eq 0 ] || exit $?
		fi
		[ -f $ERRNOTIFYADD ] || exit 0
		$ODMADD $ERRNOTIFYADD
		exit $?
		;;
	-l)
		SEQNO=$2
		shift
		;;
	-t)
		LABEL=$2
		shift
		;;
	-r)
		RESOURCE=$2
		shift
		;;
	-H)
		echo "$USAGE"
		exit 1
		;;
	--)
		shift
		break
		;;
	*)
		echo "$PROGNAME: invalid argument $i"
		echo "$USAGE"
		exit 1
		;;
	esac
	shift
done

[ -z "$SEQNO" ] && exit 0

rm -f $TMPFILE 

crecho ""
crecho "\
-----------------------------------------------------------------------------"

if [ -z "$LABEL" ]
then
  Z=`$DSPMSG cmderrlg.cat -s 2 1 "A condition has been detected that requires immediate attention."`
  crecho "$Z"

  crecho ""

  $ERRPT -l $SEQNO | cio >> $TMPFILE

  crecho ""

  Z=`$DSPMSG cmderrlg.cat -s 2 2 "If you have installed the Software Error Logging and Dump Service"`
  crecho "$Z"

  Z=`$DSPMSG cmderrlg.cat -s 2 12 "Aids package, bos.sysmgt.serv_aid, then run the command below for"`
  crecho "$Z"

  Z=`$DSPMSG cmderrlg.cat -s 2 13 "more detailed information.  Otherwise, move your system error log to"`
  crecho "$Z"

  Z=`$DSPMSG cmderrlg.cat -s 2 14 "a machine that has this package installed, and run the command below."`
  crecho "$Z"

  crecho "    errpt -a -l $SEQNO -i <logfile>"

else

  case $LABEL in 
    CHECKSTOP)
               Z=`$DSPMSG cmderrlg.cat -s 2 10 "A Checkstop occurred and"`
               Z2=`$DSPMSG cmderrlg.cat -s 2 9 "has been logged in the system error log."`
               crecho "\t$Z $Z2"

               Z=`$DSPMSG cmderrlg.cat -s 2 5 "If you have installed the Software Error Logging and Dump Service"`
  	       crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 15 "Aids package, bos.sysmgt.serv_aid, then run the command below to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 16 "examine the error entry.  Otherwise, move your system error log to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 17 "a machine that has this package installed, and run the command below."`
               crecho "\t$Z"
              
               crecho "\t\terrpt -a -l $SEQNO -i <logfile>"

               Z=`$DSPMSG cmderrlg.cat -s 2 6 "Run diagnostics to determine the nature of the problem."`
               crecho "\t$Z" 

               Z=`$DSPMSG cmderrlg.cat -s 2 11 "If Checkstop occurs repeatedly, contact your service representative."`
               crecho "\t$Z" 

	       ;;
	       
    CDROM_ERR2 | CDROM_ERR4 | CDROM_ERR6 | TAPE_ERR3)
               Z=`$DSPMSG cmderrlg.cat -s 2 3 "A recovered error has been detected on device"`
               crecho "\t$Z $RESOURCE"

               Z=`$DSPMSG cmderrlg.cat -s 2 4 "The device may need cleaning or have defective media."`
               crecho "\t$Z" 

               Z=`$DSPMSG cmderrlg.cat -s 2 5 "If you have installed the Software Error Logging and Dump Service"`
  	       crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 15 "Aids package, bos.sysmgt.serv_aid, then run the command below to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 16 "examine the error entry.  Otherwise, move your system error log to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 17 "a machine that has this package installed, and run the command below."`
               crecho "\t$Z"
              
               crecho "\t\terrpt -a -l $SEQNO -i <logfile>"

               Z=`$DSPMSG cmderrlg.cat -s 2 7 "If problem persists run diagnostics on the device."`
               crecho "\t$Z" 

	       ;;

    MEMORY | MEM1 | MEM2 | MEM3)
               Z=`$DSPMSG cmderrlg.cat -s 2 8 "A memory error has been detected and"`
               Z2=`$DSPMSG cmderrlg.cat -s 2 9 "has been logged in the system error log."`
               crecho "\t$Z $Z2"

               Z=`$DSPMSG cmderrlg.cat -s 2 5 "If you have installed the Software Error Logging and Dump Service"`
  	       crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 15 "Aids package, bos.sysmgt.serv_aid, then run the command below to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 16 "examine the error entry.  Otherwise, move your system error log to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 17 "a machine that has this package installed, and run the command below."`
               crecho "\t$Z"
              
               crecho "\t\terrpt -a -l $SEQNO -i <logfile>"

               Z=`$DSPMSG cmderrlg.cat -s 2 6 "Run diagnostics to determine the nature of the problem."`
               crecho "\t$Z" 
	       ;;

    TAPE_ERR6)
               Z=`$DSPMSG cmderrlg.cat -s 2 18 "Tape drive usage for %s has exceeded the recommended cleaning" $RESOURCE`
               crecho "\t$Z"
               Z2=`$DSPMSG cmderrlg.cat -s 2 19 "interval without being cleaned.  Use approved cleaning materials to"`
               crecho "\t$Z2"
               Z3=`$DSPMSG cmderrlg.cat -s 2 20 "clean the tape drive."`
               crecho "\t$Z3"

               Z=`$DSPMSG cmderrlg.cat -s 2 5 "If you have installed the Software Error Logging and Dump Service"`
  	       crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 15 "Aids package, bos.sysmgt.serv_aid, then run the command below to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 16 "examine the error entry.  Otherwise, move your system error log to"`
               crecho "\t$Z"

               Z=`$DSPMSG cmderrlg.cat -s 2 17 "a machine that has this package installed, and run the command below."`
               crecho "\t$Z"
              
               crecho "\t\terrpt -a -l $SEQNO -i <logfile>"

               Z=`$DSPMSG cmderrlg.cat -s 2 6 "Run diagnostics to determine the nature of the problem."`
               crecho "\t$Z" 
	       ;;

  esac
fi
crecho "\
-----------------------------------------------------------------------------"
echo "">$>>$TMPFILE

cp $TMPFILE /tmp/x
( mail -s "Error Notification" root < $TMPFILE; cat $TMPFILE > $CONSOLE ; rm -f $TMPFILE) &

