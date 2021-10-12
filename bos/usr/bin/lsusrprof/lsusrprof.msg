$  @(#)78  1.6  src/bos/usr/bin/lsusrprof/lsusrprof.msg, rcs, bos411, 9428A410j 6/30/93 18:09:33
$ 
$    COMPONENT_NAME: RCS
$ 
$    FUNCTIONS: none
$ 
$    ORIGINS: 27
$ 
$    IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$    combined with the aggregated modules for this product)
$                     SOURCE MATERIALS
$ 
$    (C) COPYRIGHT International Business Machines Corp. 1993
$    All Rights Reserved
$    US Government Users Restricted Rights - Use, duplication or
$    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$   

$ scmc msg file created Wed Apr 28 17:05:49 1993
$ user:  root

$quote "         define quote character.

$len 
$set MS_lsusrprof        Define initial set#

M_MSG_0              "Usage: lsusrprof -a |\n\
\tlsusrprof -s [ -c ] -n Name [ -t { 1 | 2 } ] |\n\
\tlsusrprof -l [ -c ] -n Name [ -t { 3 | 4 | 5 } ]\n"
$ 
$  Don't translate the word lsusrprof (the command)
$ 
M_MSG_1              "This program needs at least one argument.\n"
M_MSG_2              "Unable to obtain a lock on the odm database.\n"
M_MSG_3              "The -n option needs a name.\n"
M_MSG_4              "Retrieval of error message failed.\n"
M_MSG_5              "You are not allowed to view this profile.\n"
M_MSG_6              "The user name was not found in the password file.\n"
