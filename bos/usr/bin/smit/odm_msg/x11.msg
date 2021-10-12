$ @(#)89	1.4  src/bos/usr/bin/smit/odm_msg/x11.msg, odm_msg, bos411, 9428A410j 9/2/93 15:15:51

$
$  COMPONENT_NAME: SMIT DIALOGUES -- X-11 
$
$  FUNCTIONS:  none
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote '
$set 24
1 'Change Initial Interface'
2 'User NAME'
3 'User NAME\n '
$ do not translate "\n " at end of string
$ leave 1 space after "\n"
4 'Initial Interface\n\
\n\
    "desktop" = Desktop,\n\
                window, and clock.\n\
    "windows" = window and clock.\n\
    "command line" = shell command line prompt.\n\
\n\
The Initial Interface is the interface that\n\
will come up on-screen immediately after the\n\
user logs in and enters a password.'
5 'desktop,windows,command line'
