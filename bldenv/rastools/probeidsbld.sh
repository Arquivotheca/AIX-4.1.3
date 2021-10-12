#!/bin/sh
# @(#)15	1.1  src/bldenv/rastools/probeidsbld.sh, cmderrlg, bos412, GOLDA411a 10/20/93 14:29:41
# COMPONENT_NAME: CMDERRLG
#
# FUNCTIONS:  tool to genenerate /usr/include/sys/probeids.h
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# Input:
#	The first file in sequence is probeids.h.
#	The second is probeids.desc.
#
# First, get the existing labels and their ids from probeids.h.
# Then get the labels and comments from probeids.desc.
# Recreate probeids.h with the labels in the order they are in
# probeids.desc.  Any label not in probeids.h will get a new id.
# Any label left over (i.e.), in probeids.h but not in probeids.desc
# will retain its id but get a comment of "UNUSED".
#
# The format of probeids.desc is:
# label comment
# Where comment is a one-liner 

PATH=${ODE_TOOLS}/bin:${ODE_TOOLS}/usr/bin
export PATH

if [ $# != 2 -o $1 = "-?" ]
then	# syntax error
	echo "Usage:  probeidsbld probeids.h probeids.desc" >&2
	exit 1
fi

# Quit if we can't read the database
if [ ! -r "$2" ]
then	echo "Can't read database $2" >&2
	exit 2
fi

# Recreate $1
PROLOGS=/afs/austin/local/bin/prologs
tmpf=/tmp/probeids.$$
touch $tmpf
$PROLOGS -C CMDERRLG -O 27 -D 1993 -P 2 -T header $tmpf

# create a new probeids.h file in tmpf
awkf=$0.awk
touch $1 	# Just in case it doesn't exist at all.
awk -f $awkf $* >>$tmpf

# Build the output file $1
# Put out #ifndef and prolog.
echo "#ifndef _h_PROBEIDS" >$1
echo "#define _h_PROBEIDS" >>$1

cat $tmpf >>$1
rm $tmpf

# Put on the #endif
echo "#endif /* _h_PROBEIDS */" >>$1

exit 0
