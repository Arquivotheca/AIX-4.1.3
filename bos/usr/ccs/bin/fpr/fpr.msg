$ @(#)53	1.6  src/bos/usr/ccs/bin/fpr/fpr.msg, cmdprog, bos411, 9428A410j 6/15/90 22:50:55
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
$ messages for fpr
$

$len
$set MS_FPR

$quote "
$ The message numbers have been assigned a range of 050 - 074
$ Do not move %1$s:  
BADCC1               "%1$s: 1286-050 A carriage control character is not correct on 1 line of %2$s.\n"
BADCC2               "%1$s: 1286-051 Carriage control characters are not correct on %3$d lines of %2$s.\n"
NOSPACE              "%1$s: 1286-052 There is not enough memory available now.\n"
BADCC3               "%1$s: 1286-053 Cannot open the input file %2$s.\n"
