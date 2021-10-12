#!/usr/bin/ksh
# @(#)82	1.11  src/bos/usr/bin/runcat/runcat.sh, cmdmsg, bos411, 9428A410j 3/3/93 13:11:17
# COMPONENT_NAME: CMDMSG
#
#  FUNCTIONS: runcat.sh
# 
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


#
#  NAME: runcat
#
#  FUNCTION: Produce message header file and message catalog
#
#  NOTES: Produces message header file and catalog from message definition 
#         files.
#
#  RETURNS: Return code from last command.
#           1 - error encountered.
#

MK=1

if [ "$#" -lt 2 -o "$#" -gt 3 ]
then
     if [ -x /usr/bin/dspmsg ];
	then /usr/bin/dspmsg msgfac.cat -s 5 1 "usage: runcat catname srcfile [catfile]\n";
        else echo "usage: runcat catname srcfile [catfile]";
     fi
     exit 1
fi

# verify that source file exists and contains messages (or is stdin)

if [ "$2" != "-" ]; then
	if [ -r $2 ]; then
		if `grep '^\$set' $2 >/dev/null`; then true;
		else
	     		if [ -x /usr/bin/dspmsg ]; then
				/usr/bin/dspmsg msgfac.cat 2 "No \$set in source\n";
       	        	else echo "No \$set in source";
     	     		fi
     	     		exit 1;
		fi
	else
     		if [ -x /usr/bin/dspmsg ]; then
			/usr/bin/dspmsg msgfac.cat 3 "Can't open %s\n" $2;
        	else echo "Can't open $2"
     		fi
     		exit 1;
	fi
fi
# set name of catalog file

if [ "$#" -eq 2 ]
	then CATFILE="$1.cat"
	else CATFILE="$3"
fi

# run mkcatdefs only if at least one line starts with character

if [ "$2" != "-" ]; then
	if `grep '^[A-Za-z]' $2 >/dev/null`; then MK=1; else MK=0; fi
	if [ $MK -eq 0 ]; then
		if `grep '^\$set *[A-Za-z]' $2 >/dev/null`; then MK=1; fi;
	fi
fi

if [ $MK -eq 1 ]
	then
	    	mkcatdefs $1 $2 | gencat $CATFILE -
	else
		gencat $CATFILE $2;

fi
