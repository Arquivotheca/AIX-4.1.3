#!/bin/ksh
# @(#)21	1.4  src/bldenv/rastools/errtmpltbld.sh, cmderrlg, bos412, GOLDA411a 10/8/93 20:37:17
# COMPONENT_NAME: CMDERRLG   system error logging facility
#
# FUNCTIONS:  tool to genenerate /usr/include/sys/errids.h
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# THIS FILE IS COPIED INTO AND MODIFIED IN THE FOLLOWING TWO DIRECTORIES:
# com/TOOLS/rastools and com/tools/errtmpltbld

SED=$ODE_TOOLS/usr/bin/sed
BASENAME=$ODE_TOOLS/usr/bin/basename
GETOPT=$ODE_TOOLS/usr/bin/getopt
DD=$ODE_TOOLS/usr/bin/dd
RM=$ODE_TOOLS/usr/bin/rm
CAT=$ODE_TOOLS/usr/bin/cat
ECHO=$ODE_TOOLS/usr/bin/echo
WHAT=$ODE_TOOLS/usr/bin/what  
SED=$ODE_TOOLS/usr/bin/sed

PROGNAME=`$BASENAME $0`
ERRUPDATE=$ODE_TOOLS/usr/bin/errupdate

USAGE="
$PROGNAME  -d errids.desc_file  -o errids.h_file

Generate the sys/errids.h file from cmd/errlg/errids.desc.
Then use the resulting error id (ERRID_XXXX) as the argument to the
errsave() or errunix() error logging call.

-d file  Use 'file' as the stanza description file.
-o file  Write new errids file to 'file'. -o - write to stdout.

"

EXAMPLES="
Example. Add your templates to sys/errids.h
   1. sget the stanza file .../com/cmd/errlg/errids.desc.
   2. add your stanza to the file.
   3. type
             $PROGNAME -u
      If there are no syntax errors in your stanza file, the tpath sys/errids.h
      will be re-generated according to the errids.desc file.

Notes
   1. You must have a writable copy of sys/errids.h in your tpath.
      Otherwise, the output from the program will go to the stdout.
   2. The 'delete' and 'update' stanzas in the .desc file are not used in
      this program.
   3. Do not sput the errids.h file. Instead, sput the errids.desc file.
      This errids.desc file will be used to generate the errids.h file for
      successive builds.
"

BELL='\007'
TMPFILE=errtmp$$
TMPFILE2=errtmp2$$
CLEANUP="$RM -f $TMPFILE $TMPFILE2"

> $TMPFILE

trap "$CLEANUP ; exit 2" 2
trap "$CLEANUP ; exit 0" 0

# tpath, but the current directory can be anywhere.
# The tpath directory is the first one in the tpath file.
#
# ptpath(file_name)
#
ptpath()
{

	tmpfile=ptpath$$
	searchfile=$1
	# use the shell to expand the tpath
	if [ "$SHELL" = "" ] ; then
		SHELL=sh
	fi
	#
	# extract the second field of sh $FTPATH
	#
	Z=`tpath $searchfile`
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	if [ ! -f "$Z" ] ; then
		errecho "no $searchfile in tpath"
		exit 1
	fi
	$ECHO $Z
}

#
# echo arguments to stderr instead of stdout
#
errecho()
{

	$ECHO "$@" 1>&2
}

if [ $# -eq 0 ] ; then
	errecho "$USAGE"
	exit 1
fi

#
# Scan the command line
#
OPTIONS="`$GETOPT pH6o:d: $*`"
if [ $? -ne 0 ] ; then
	errecho "$USAGE"
	exit 1
fi

set -- $OPTIONS

INSTALLFLG=0
VFLG=0
DESC=
ERRIDS=
OUTFILE=
OLDFLG=
PFLG=
for i in $* ; do
	ARG=$1
	case $ARG in
	-u)               # this is so that command with no args gives help msg.
		;;
	-6)
		OLDFLG=-6
		;;
 	-p)		  # this flag is added because the tool, errupdate, needs it
		PFLG=-p
		;;
	-d)
		DESC=$2
		shift
		;;
	-o)
		ERRIDS=$2
		shift
		;;
	-v)
		VFLG=1
		;;
	--)
		shift
		break
		;;
	-x)
		$ECHO "$EXAMPLES"
		exit 1
		;;
	-H)
		$ECHO "$USAGE"
		exit 1
		;;
	*)
		errecho "$PROGNAME: invalid argument $i"
		errecho "$USAGE"
		exit 1
		;;
	esac
	shift
done

#
# get the first tpath directory to obtain the pathname of
#  sys/errids.h and errids.desc
#
if [ "$ERRIDS" = "" ] ; then
	ERRIDS=`ptpath inc/sys/errids.h`
	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi
if [ "$DESC" = "" ] ; then
	DESC=`ptpath cmd/errlg/errids.desc`
	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi

if [ $VFLG -ne 0 ] ; then
	$ECHO "\
$DESC
$ERRIDS"
	exit 0
fi

#
# generate the ids to a temp file
# -q suppress statistics
# -n no odm
# -h generate the header
#

$ERRUPDATE $OLDFLG $PFLG -qnh < $DESC >> $TMPFILE
rc=$?
if [ $rc -ne 0 ] ; then
	exit 1
fi

set -e

$ECHO "/*" > $TMPFILE2
$ECHO "$($WHAT $DESC)" | $SED "s!^[	 ]!@(#)!" >> $TMPFILE2  # errids.h gets same expanded 
								# SCCSID string as errids.desc
$ECHO "*/" >> $TMPFILE2

$ECHO '
/*
 * COMPONENT_NAME: CMDERRLG  system error logging facility
 *
 * FUNCTIONS:  header file for error ids
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
'                      >> $TMPFILE2

# The define to keep from including -o filename multiply.
TMPERRIDS=`$BASENAME $ERRIDS | $SED 's/.h//' | $DD conv=ucase 2>/dev/null`
$ECHO "#ifndef _H_$TMPERRIDS" >> $TMPFILE2
$ECHO "#define _H_$TMPERRIDS" >> $TMPFILE2

$ECHO '
#include "sys/err_rec.h"

'                      >> $TMPFILE2

$CAT $TMPFILE           >> $TMPFILE2

$ECHO "
#endif /* _H_$TMPERRIDS */
" >> $TMPFILE2

if [ "ERRIDS" = "-" ] ; then
	$CAT $TMPFILE2
else
	$CAT $TMPFILE2 > $ERRIDS
fi

