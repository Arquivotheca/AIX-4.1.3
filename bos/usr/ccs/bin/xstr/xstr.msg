$ @(#)00	1.4  src/bos/usr/ccs/bin/xstr/xstr.msg, cmdprog, bos411, 9428A410j 6/15/90 22:57:20
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
$ messages for xstr
$

$len
$set MS_XSTR

$quote "
$      Do not translate %d, %s.

USAGE           "Usage: xstr [-v] [-c] [-] [File]\n"
EXTERN          "extern char\txstr[];\n"
$ Do not translate.  
LINE            "#line%s"
$ Do not translate.  
FOUND           "Found at %d: "
$ %d is a number.  
$ Do not add a newline, this message is followed by the found string.
NEW             "New at %d: "
$ %d is a number.  
$ Do not add a newline, this message is followed by the new string.
XDOTCF          "char\txstr[] = {\n"
$ Do not translate.  
