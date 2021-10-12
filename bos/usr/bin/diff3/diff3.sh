#!/usr/bin/bsh
# @(#)87	1.13  src/bos/usr/bin/diff3/diff3.sh, cmdfiles, bos411, 9428A410j 11/5/92 08:24:48
#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: diff3
#
# ORIGINS: 3, 18, 26, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#                                                                   
#  diff3  [-ex3EX] file1 file2 file3 
#  compares three files
#  this shell script calls /usr/lbin/diff3prog to do the work.

usage() {
  if [ -x /usr/bin/dspmsg ]; then
  dspmsg -s 2 diff3.cat 1 'Usage: diff3 [-e|-x|-E|-X|-3] File1 File2 File3\n' 1>&2 
  else
    msg='usage: diff3 [-e|-x|-E|-X|-3] file1 file2 file3';
    eval echo $msg 1>&2; 
  fi
  exit 1
}

e=
set -- `getopt 3exEX $*`
if [ $? != 0 ]
then usage
fi
while [ $# -gt 0 ]
do
  case $1 in    
  -- )	shift
	break
	;;
  -*)	if [ .$e = . ]
	then
	  e=$1
	  shift
	else
	  usage
	fi
	;;
  * )	break
	;;
  esac
done

if [ $# -ne 3 ]
then
  usage
fi

trap "rm -f /tmp/d3[ab]$$ /tmp/diff3err$$; trap '' 0; exit" 1 2 13 15
diff $1 $3 >/tmp/d3a$$ 2>/tmp/diff3err$$
diffRC=$?
if [ $diffRC -le 1 ]
then
  diff $2 $3 >/tmp/d3b$$ 2>/tmp/diff3err$$
  diffRC=$?
fi
if [ $diffRC -gt 1 ]
then
  if [ -x /usr/bin/dspmsg ]; then
  dspmsg -s 2 diff3.cat 2 "diff3: A diff command failed in diff3.\n" 1>&2
  else
    msg='diff3: A diff command failed in diff3.\n';
    eval echo $msg 1>&2; 
  fi
  echo "`cat /tmp/diff3err$$`" 1>&2
  rm -f /tmp/diff3err$$ /tmp/d3[ab]$$
  exit $diffRC
fi

/usr/lbin/diff3prog $e /tmp/d3[ab]$$ $1 $2 $3
if [ $? != 0 ]
then rm -f /tmp/d3[ab]$$ /tmp/diff3err$$
     exit 1
else rm -f /tmp/d3[ab]$$ /tmp/diff3err$$
fi
exit 0
