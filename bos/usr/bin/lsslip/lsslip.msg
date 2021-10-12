$  @(#)82  1.3  src/bos/usr/bin/lsslip/lsslip.msg, rcs, bos411, 9428A410j 5/25/93 16:53:50
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

$ scmc msg file created Wed Apr 28 16:20:18 1993
$ user:  root

$quote "         define quote character.

$len 
$set MS_lsslip        Define initial set#

M_MSG_0              "Usage: lsslip -a [ -c ] | -n Name [ -c ]\n"
$ 
$  Don't translate the word lsslip (the command) in Usage
$ 
M_MSG_1              "This program needs at least 1 argument.\n"
M_MSG_2              "The -a and -n options are mutually exclusive.\n"
M_MSG_3              "The -c option requires a (-a) or a (-n) component.\n"
M_MSG_4              "Unable to obtain a lock on the odm database.\n"
M_MSG_5              "Retrieval of error message failed.\n"
M_MSG_6              "\nThere are no SLIP connections defined.\n"
M_MSG_7              "SLIP connection not found.\n"
