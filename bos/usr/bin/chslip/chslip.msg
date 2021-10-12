$  @(#)81  1.4  src/bos/usr/bin/chslip/chslip.msg, rcs, bos411, 9428A410j 6/30/93 18:08:55
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

$ scmc msg file created Fri Apr 30 14:37:51 1993
$ user:  elaird

$quote "         define quote character.

$ scmc msg file created Wed Apr 28 18:12:14 1993 
$ user:  root 
$len 
$set MS_chslip        Define initial set#

M_MSG_1              "Usage:    chslip  -n Connection_Name\n\
\t\t\[ -c Connection Description ] \n\
\t\t\  -r Remote Host Name (or dotted decimal) \n\
\t\t\  -l Internet Address of Local Host \n\
\t\t\  -d Internet Address of Dialed Host \n\
\t\t\[ -m Network Mask (hexadecimal or dotted decimal] \n\
\t\t\  -p TTY Port Used For Slip Connection \n\
\t\t\  -b Baud Rate \n\
\t\t\  -s Dial String \n\
\t\t\  -t Timeout \n"
$ 
$  Don't translate the word chslip (the command) after the the word Usage
$ 
M_MSG_2              "Invalid baud rate.\n"
M_MSG_3              "A baud rate of 0 is not allowed.\n"
M_MSG_4              "Retrieval of error message failed.\n"
M_MSG_5              "SLIP not found.\n"
M_MSG_8              "A timeout of less than 30 seconds is not allowed.\n"
M_MSG_9              "Error changing %s;\nCharacter : is invalid.\n"
