$ @(#)15        1.7 3/17/93 11:30:49
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$ Do not translate %c, %s, %d, etc...
$ Do not translate SCCS, :
$ Do not translate the parenthesis (cm#) at the end of each message.
$len 
$set 2        Define initial set#

UNKKEYLTR            "1255-036 Flag -%c is not valid. (cm1)\n"
$ Do not translate -%c
KEYLTRTWC            "\n\
1255-037 Use the -%c flag only once on the command line. (cm2)\n"
$ Do not translate -%c
MISSFLNAM            "\n\
1255-038 Specify a file name. (cm3)\n"
LOCKCREAT            "\n\
1255-039 Cannot create the lock file. (cm4)\n"
BADDTTM              "\n\
1255-040 Specify the date and time in the correct format.\n\
\tThe correct format is yy[mm[dd[hh[mm[ss]]]]]. (cm5)"
NOIDKEYWRDS          "\n\
1255-041 The file must contain SCCS identification keywords. (cm6)\n"
NOIDKEYWRDS7         "There are no SCCS identification keywords in the file. (cm7)\n"
VALAFTKEY            "1255-043 Cannot use a value for the -%c flag. (cm8)\n"
$ Do not translate -%c
$  
USERID               "\n\
1255-044 The /etc/passwd file is not accessible. (cm9)\n"
$ Do not translate /etc/passwd
INVIDKYWRDS          "\n\
1255-045 The SCCS file requires one or more specific \n\
\tidentification keywords. (cm10)\n"
CURDIRFAIL           "\n\
1255-046 Cannot determine the path name of the current directory. (cm21)\n"
SIDNOEXIST           "\n\
1255-047 The SID does not exist. (cm20)\n"
CANTCHDIR            "1255-048 Cannot chdir to %s. (cm22)\n"
$ Do not translate chdir.
ROUTOFRNG            "\n\
1255-049 Specify a value for the -r flag  greater than 0 and\n\
\tless than 10000. (cm23)\n"
MRNOTALD             "\n\
1255-050 The specified SCCS file does not allow MR numbers. (cm24)\n"
$ Do not translate MR.
INVMRS               "\n\
1255-051 Specify a valid MR number or numbers. (cm25)\n"
$ Do not translate MR.
MRSREQ               "\n\
1255-052 Specify an MR number or numbers. (cm26)\n"
$ Do not translate MR.
UNKKEYLTR1           "1255-053 Flag -%C is not valid. (cm1)\n"
$ Do not translate -%C
USAGEWHAT "Usage: what [ -s ] [ File... ]\n"
$ Do not translate what.
