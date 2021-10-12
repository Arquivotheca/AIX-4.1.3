#!/bin/bsh
# @(#)09	1.14  src/bos/usr/bin/dircmp/dircmp.sh, cmdfiles, bos411, 9428A410j 8/6/93 09:24:27
#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: dircmp
#
# ORIGINS: 3, 18, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# (No OSF RCS id string found)
#

# Portability: check for AIX dspmsg utility.
type dspmsg >/dev/null
DM=$?
# Portability: Handle all uses of dspmsg according to this code template
# so the code will work even if  dspmsg  is not defined:
#if [ $DM = 0 ] ; then
#dspmsg -s 1 dircmp.cat <msg_id> <default message>
#else echo <<default message> WITHOUT final newline and WITH substitutions>
#fi

PATH=/bin:/usr/bin
trap "rm -f /usr/tmp/dc$$*;exit" 1 2 3 15
# Local subprogram BADUSAGE to display Usage message and exit.
BADUSAGE() {
if [ $DM = 0 ] ; then
dspmsg -s 1 dircmp.cat 14 "Usage: dircmp [-ds] directory1 directory2\n" 1>&2
else echo "Usage: dircmp [-ds] directory1 directory2" 1>&2
fi
exit 1
}
# End of local subprogram BADUSAGE
# 
# Get options and Directory arguments.
set -- `getopt ds $* 2>/dev/null`
if [ $? != 0 ]
then BADUSAGE
fi 
while [ .$1 != . ]
do
  case $1 in
  -d )	Dflag="yes"
	shift
	;;
  -s )	Sflag="yes"
	shift
	;;
  -- )	shift
	break
	;;
  * )	break
	;;
  esac
done
D1=$1
D2=$2
if [ -z "$D1" -o -z "$D2" -o -n "$3" ]
then BADUSAGE
fi
# Check that directory arguments really are directories.
NOTD=
if [ ! -d $D1 ]
then NOTD=$D1
elif [ ! -d $D2 ]
then NOTD=$D2
fi
if [ -n "$NOTD" ]
then
  if [ $DM = 0 ] ; then
  dspmsg -s 1 dircmp.cat 15 "dircmp: %s is not a directory.\n" $NOTD 1>&2
  else echo "dircmp: $NOTD is not a directory." 1>&2
  fi
  exit 2
fi 

D0=`pwd`
if cd $D1
then find . -print | sort > /usr/tmp/dc$$a
     if [ $? != 0 ]
     then exit 2
     fi
else exit 2
fi
cd $D0
if cd $D2
then find . -print | sort > /usr/tmp/dc$$b
     if [ $? != 0 ]
     then exit 2
     fi
else exit 2
fi
comm /usr/tmp/dc$$a /usr/tmp/dc$$b | sed -n \
	-e "/^		/w /usr/tmp/dc$$c" \
	-e "/^	[^	]/w /usr/tmp/dc$$d" \
	-e "/^[^	]/w /usr/tmp/dc$$e"
rm -f /usr/tmp/dc$$a /usr/tmp/dc$$b
if [ $DM = 0 ] ; then
UNIQHDR=`dspmsg -s 1 dircmp.cat 16 '%1$s only and %2$s only' $D1 $D2`
else UNIQHDR='$D1 only and $D2 only'
fi
pr -h "$UNIQHDR" -m -s" " /usr/tmp/dc$$e /usr/tmp/dc$$d
rm -f /usr/tmp/dc$$e /usr/tmp/dc$$d
sed -e s/..// < /usr/tmp/dc$$c > /usr/tmp/dc$$f
rm -f /usr/tmp/dc$$c
cd $D0
if [ $DM = 0 ] ; then
COMPHDR=`dspmsg -s 1 dircmp.cat 17 'Comparison of %1$s and %2$s' $D1 $D2`
else COMPHDR='Comparison of $D1 and $D2'
fi
> /usr/tmp/dc$$g
while read a
do
	if [ -d $D1/"$a" ]
	then if [ "$Sflag" != "yes" ]
	     then
		if [ $DM = 0 ]
	        then dspmsg -s 1 dircmp.cat 21 "directory\t%s\n" $a
		else echo "directory\t$a"
		fi
	     fi
	elif [ -f $D1/"$a" ]
	then cmp -s $D1/"$a" $D2/"$a"
	     if [ $? = 0 ]
	     then if [ "$Sflag" != "yes" ]
		  then
			if [ $DM = 0 ]
		 	then dspmsg -s 1 dircmp.cat 19 "same\t\t%s\n" $a
			else echo "same\t\t$a"
			fi
		  fi
	     else
		  if [ $DM = 0 ] ; then
		  dspmsg -s 1 dircmp.cat 20 "different\t%s\n" $a
		  else echo "different\t$a"
	 	  fi
		  if [ "$Dflag" = "yes" ]
		  then
		    if [ $DM = 0 ] ; then
	       	    DIFFHDR=`dspmsg -s 1 dircmp.cat 18 \
				'diff of %1$s in %2$s and %3$s' $a $D1 $D2`
		    else DIFFHDR= 'diff of $a in $D1 and $D2'
		    fi
	 	    diff $D1/"$a" $D2/"$a" | pr -h "$DIFFHDR" >> /usr/tmp/dc$$g
		  fi
	     fi
	elif [ "$Sflag" != "yes" ]
	then
		if [ $DM = 0 ]
		then dspmsg -s 1 dircmp.cat 22 "special\t %s\n" $a
		else echo "special\t $a"
		fi
	fi
done < /usr/tmp/dc$$f | pr -r -h "$COMPHDR"
if [ "$Dflag" = "yes" ]
then cat /usr/tmp/dc$$g
fi
rm -f /usr/tmp/dc$$*
exit 0
