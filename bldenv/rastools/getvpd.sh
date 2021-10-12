#!/bin/sh
# @(#)13	1.2  src/bldenv/rastools/getvpd.sh, cmderrlg, bos412, GOLDA411a 2/15/94 10:15:46
# COMPONENT_NAME: CMDERRLG
#
# FUNCTIONS:  tool to get a program's VPD.
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

# Get the VPD info from the data file given
#
# Input:
#	$1 - VPD database
#	$2 - database argument (name of binder object)
#	$3 - top of source tree
#
# Output:
#	VPD of the form @VPD/nnn/ppp/lll where
#	nnn = program name, ppp = program's Retain component ID,
#	and lll = Retain level.  For example
#	@VPDksh/575603001/410

# Help text
Help() {
	echo "Usage:  getvpd database argument tree-top" 2>&1
	exit 1
}

# Build environment path
PATH="${ODE_TOOLS}/bin:${ODE_TOOLS}/usr/bin"
export PATH

if [ $# -lt 3 ]
then Help
fi

# Database file.
db=$1

# Setup the argument with the path in it.
arg=`pwd`/`basename $2`

# Get the tree top
top=`(cd $3; pwd)`

# awk script to find the first line in the database whose first element,
# the pathname, is contained in arg.
# For example, if the (arg)ument is bos/usr/Makefile, database items
# of bos, bos/usr, and bos/usr/Makefile would match it.  We pick the
# first one.  If no match is found, the default is used.  This appears
# in the database as a line starting with "*", asterisk.
#
# NOTE:  Two types of variables are used, shell and awk variables.
# If an awk variable begins with "$", such as $1, the "$" is excaped so
# the shell won't think it's a shell variable.
# Also, where shell variables are used as strings, they must be placed
# in excaped quotes so that awk knows they're strings.
# \$1 is then the awk $1 variable, and \"$arg\" is the string
# for the arg shell variable.
awk -v "name=`basename $arg" "\
	BEGIN {\
		arglen = length(\"$arg\");\
		rc=2;\
		defcomp = \"\";\
	};\
	# If is a comment or not the right number of fields, skip it.
	\$0 !~ /^#/ && NF == 3 {\
		# Check for the default line and save it's fields.
		if (\$1 == \"*\") {\
			defcomp = \$2;\
			defvers = \$3;\
			next;\
		}\
		# dbarg will have the same leading path as arg, if
		# they match.
		dbarg = \"$top\"\"/\"\$1;\
		len=length(dbarg);\
		# For a match, dbarg must be the first part of the pathname
		# for arg, or be the whole arg parameter.
		if ((len < arglen) && (substr(\"$arg\",1,len) == dbarg)) {\
			# -b dbg:ldrinfo:@VPD<pgmname>/<RetCompID>/<Release>
			# Is the current binder option symtax.
			printf(\"@VPD%s/%s/%s\",name,\$2,\$3);
			rc = 0;\
			exit;\
		};\
	};\
	# rc was set to 2 in BEGIN.  Exit with no match found.
	END {\
		# Fill in the default if specified in the file.
		if ((defcomp != \"\") && (rc!=0)) {\
			printf(\"@VPD%s/%s/%s\",name,defcomp,defvers);\
			rc=0;\
		};\
		# rc will be 0 if a match was found or the default was found.
		exit(rc);\
	}" $db

exit $?
