# @(#)22	1.1  src/bldenv/rastools/trcfmtchk.sh, bldprocess, bos412, GOLDA411a 8/7/92 18:24:54
# COMPONENT_NAME: CMDTRACE  system trace reporting facility
#
# FUNCTIONS: shell script to check a trace template for errors
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

# THIS FILE IS COPIED INTO AND MODIFIED IN THE FOLLOWING DIRECTORY:
# com/TOOLS/rastools

PROGNAME=`basename $0`
BELL='\007'
TRCFMT=
TRCRPT=$BLDENV/usr/bin/xtrcrpt

USAGE="
$PROGNAME  [-t template_file]

Check the template file for syntax errors.
If template_file is not specified, use the one in your tpath.
"

#
# tpath, but the current directory can be anywhere.
# The tpath directory is the first one in the tpath file.
#
# ptpath(file_name)
#
ptpath()
{

	tmpfile=ptpath$$
	searchfile=$1
	#
	# extract the second field of sh $FTPATH
	#
	Z=`tpath $searchfile`
	[ $? -eq 0 ] || errexit "error during tpath of $searchfile"
	[ -f "$Z" ]  || errexit "no $searchfile in tpath"
	echo $Z
}

#
# echo arguments to stderr instead of stdout
#
errecho()
{

	echo "$@" 1>&2
}

errexit()
{

	echo "$@" 1>&2
	exit 2
}

#
# Scan the command line
#
OPTIONS="`getopt t: $*`"
[ $? -eq 0 ] || errexit "$USAGE"

set -- $OPTIONS

for i in $* ; do
	ARG=$1
	case $ARG in
	-t)
		TRCFMT=$2
		shift
		;;
	--)
		shift
		break
		;;
	-H)
		echo "$USAGE"
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

[ -z "$TRCFMT" ] && TRCFMT=`ptpath cmd/trace/trcrpt/trcfmt`

echo "xtrcrpt -c -t $TRCFMT"
$TRCRPT -c -t $TRCFMT

