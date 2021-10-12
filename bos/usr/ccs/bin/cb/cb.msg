$ @(#)35        1.10  src/bos/usr/ccs/bin/cb/cb.msg, cmdprog, bos411, 9428A410j 2/16/94 16:32:31
$
$   COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$   FUNCTIONS: none
$
$   ORIGINS: 27,10
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1991
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$len
$set MS_CB
$quote "
$ The message numbers have been assigned a range of 000 - 049

BADOPEN              "cb: 1286-000 Cannot open the input file %s.\n"
BADOPTION            "cb: 1286-001 %c is not a recognized flag.\n\
\tUsage: cb [-s] [-j] [-l Length] [File ...] \n"
BIGCOMMENT           "cb: 1286-002 There is not enough memory available for comment.\n"
CURLY                "cb: 1286-003 There are more than 20 levels of braces.\n"
$ The braces refer to the punctuation characters {}. 
DODEEP               "cb: 1286-004 There are more than 10 levels of do statements.\n"
$ The "do" part of the above message refers to C language statements  
$ Do not translate do 
IFDEEP               "cb: 1286-005 There are more than 10 levels of if statements.\n"
$ The "if" part of the above message refers to C language statements  
$ Do not translate if 
TRUNCATING           "cb: 1286-006 Truncating comment that is too long.\n"
NONUM                "cb: 1286-007 The -l flag must be followed by a number.\n"
FOLDED               "/* The indentation level exceeds the page width. */\n\
/* Resetting to the left page margin. */\n"
$ Do NOT remove the slashes and asterisks from the "FOLDED" message.  
$ These messages get ADDED to the output of cb and MUST follow  
$ C language syntax. (Note: no tab at beginning)  
$ (Same for message "UNFOLDING")  
UNFOLDING            "/* Restoring indentation level to original setting. */\n"
BADCHAR              "cb: Invalid Character\n"
