#!/bin/sh
# @(#)17	1.1  src/bldenv/rastools/retidsbld.sh, cmderrlg, bos412, GOLDA411a 10/20/93 14:29:56
# COMPONENT_NAME: CMDERRLG
#
# FUNCTIONS:  tool to genenerate /usr/include/sys/retids.h
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
#	database
PATH="${ODE_TOOLS}/bin:${ODE_TOOLS}/usr/bin"
export PATH
fn=$1
PROLOGS=/afs/austin/local/bin/prologs

if [ $# != 1 -o $1 = "-?" ]
then	# syntax error
	echo "Usage:  retidsbld <compids.table>" >&2
	exit 1
fi

# Quit if we can't read the database
if [ ! -r "$fn" ]
then	echo "Can't read database $fn" >&2
	exit 2
fi

# Generate a prolog
tmpf=/tmp/bldh.$$
touch $tmpf
$PROLOGS -C CMDERRLG -O 27 -D 1993 -P 2 -T header $tmpf

# Put out #ifndef and prolog.
echo "#ifndef _h_RETIDS"
echo "#define _h_RETIDS"
cat $tmpf

# Describe the file.
cat << END

/*
 * These are the LPP names and their Retain component ids and
 * their release numbers as well.
 * labels ending in _comp are the component ids, and _vers are 
 * the release numbers.
 */

END

# Convert input to upper case
tr '[a-z]' '[A-Z]' <$fn >$tmpf

awk 'BEGIN {FS=":"; incount=0};
     # Do this only if the line does not being with special characters
     # ans has at least 4 colon-separated fields.
     $1 ~ /./ && $0 !~ /^[ #*%]/ && NF >= 4 {
	name = $1;
	# Turn illegal characters into an underscore (_).
	gsub("[,./!@#$%^^&*()=+]","_",name);
	gsub("\-","_",name);
	# Write the Retain component id definition, then the version # define.
	printf("#define %s_COMP \"%s\"\n",name,$2);
	printf("#define %s_VERS \"%s\"\n",name,$4);
	incount += 1;
	}
    # Bad exit status if not records were output.
    END {if (incount==0) exit 1; else exit 0}' $tmpf
rc=$?

# Put on the #endif
echo "#endif /* _h_RETIDS */"

rm $tmpf
# Exit code is from awk
exit $rc
