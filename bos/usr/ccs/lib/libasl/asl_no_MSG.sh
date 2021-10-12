#!/bin/ksh

# @(#)70	1.5  src/bos/usr/ccs/lib/libasl/asl_no_MSG.sh, libasl, bos411, 9428A410j 2/18/93 14:43:25

#
#  COMPONENT_NAME: (LIBASL) AIX Screen Library
#
#  FUNCTIONS: none
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1989
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


# This shell script converts the asl.msg file to a list of
# message defines which are the strings passed to NLcatgets in
# case it fails and a default message must be printed.  This insures
# that the default message is identical to the message catalog message,
# only it will be the English version.

${ODE_TOOLS}/usr/bin/cat $1 > asl_cat.h

# input format: <'>-delimited strings, no embedded <'> chars
#       	no <token><space><'> on continuation lines.
# any line beginning with $ is a comment line.  Discard it.
# discard an empty lines (blanks and tab only)
# change to #defines if line starts with <token><space><'>
# escape " characters in strings
# change the '<string>' to "<string>"

# note: the second white space char is a tab in the 
# following line: /^[ 	]*$/d; don't accidently substitute
# spaces by using a non-UNIX-oriented editor on this file.

${ODE_TOOLS}/usr/bin/sed '
/^\$.*$/d
/^[ 	]*$/d
/^[^ ][^ ]* '"'"'.*/{i\

s/^/#define M_/
}
s/"/\\"/g
s/'"'"'/"/g' < $2  >> asl_cat.h
