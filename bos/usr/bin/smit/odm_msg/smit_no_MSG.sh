#!/bin/ksh
# @(#)49        1.6  src/bos/usr/bin/smit/odm_msg/smit_no_MSG.sh, smitobj, bos411, 9428A410j 2/18/93 16:40:17

#
#  COMPONENT_NAME: (CMDSMIT) System Management Interface Tool
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


# This shell script converts the smit.msg file to a list of
# message defines which are the strings passed to NLcatgets in
# case it fails and a default message must be printed.  This insures
# that the default message is identical to the message catalog message,
# only it will be the English version.


${ODE_TOOLS}/usr/bin/cat $1 > smit_cat.h

# input format: <'>-delimited strings, no embedded <'> chars
# 	        no <token><space><'> on continuation lines.
# Skip down to $set 99 (must be last set).
# Any line beginning with $ is a comment line.  Discard it.
# Discard an empty lines (blanks and tab only)
# Change to #defines if line starts with <token><space><'>
# Escape " characters in strings
# Change the '<string>' to "<string>"

# note: the second white space char is a tab in the
# following line: /^[ 	]*$/d; don't accidently substitute
# spaces by using a non-UNIX-oriented editor on this file.

${ODE_TOOLS}/usr/bin/sed '
1,/^\$set 99/d
/^\$.*$/d
/^[ 	]*$/d
/^[^ ][^ ]* '"'"'.*/{i\

s/^/#define M_/
}
s/"/\\"/g
s/'"'"'/"/g' < $2  >> smit_cat.h

