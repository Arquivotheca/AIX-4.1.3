$ @(#)04	1.4  src/bos/usr/ccs/bin/unifdef/unifdef.msg, cmdprog, bos411, 9428A410j 6/15/90 22:56:55
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for unifdef
$

$len
$set MS_UNIFDEF

$quote "
$ Messages have been assigned a range of 200 - 224.
$ Do not translate unifdef, %1$s, %2$s, %3$s, etc..

TOOMANY         "1286-200 Specify no more than 100 symbols.\n"
ILLOPTS         "1286-201 %s is not a recognized flag.\n"
USAGE1          "Usage: %s [-t] [-l] [-c]\n\
\t[[-DSymbol] [-USymbol] [-idSymbol] [-iuSymbol]] ... [File]\n"
USAGE2		"\tAt least one parameter from [-D -U -id -iu] is required.\n"
ONEFILE         "1286-202 Cannot accept more than one file at a time.\n"
LINEERR         "%3$s \tThis error occurred on line %2$d in file %1$s.\n"
LINERRN         "Error in %s line %d: %s.\n"
$   Debug message. Do not translate.
DEPTH           "ifdef depth: %d\n"
$   Debug message. Do not translate.
BADELSE         "1286-205 #else or #elif is not expected.\n"
$ Do not translate #else or #elif.
BADENDIF        "1286-206 #endif is not expected.\n"
$ Do not translate #endif.
BADEOF          "1286-207 End of file was not expected in #ifdef.\n"
$ Do not translate #ifdef.
BADCOMM         "1286-208 End of file was not expected in comment.\n"
BADQUOTE        "1286-209 End of file was not expected in quoted character.\n"
BADQSTR         "1286-210 End of file was not expected in quoted string.\n"
