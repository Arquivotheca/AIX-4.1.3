$  @(#)83  1.4  src/bos/usr/bin/mkslip/mkslip.msg, rcs, bos411, 9428A410j 6/30/93 18:09:21
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

$ scmc msg file created Fri Apr 30 16:18:53 1993
$ user:  elaird

$quote "         define quote character.

$len 
$set MS_mkslip        Define initial set#

M_MSG_1              "Usage: mkslip -n Connection_Name  \n\
\t[ -c Connection Description ] \n\
\t  -r Remote Host Name (or dotted decimal)  \n\
\t  -l Internet Address of Local Host  \n\
\t  -d Internet Address of Dialed Host  \n\
\t[ -m Network Mask (hexadecimal or dotted decimal] \n\
\t  -p TTY Port Used For Slip Connection  \n\
\t  -b Baud Rate  \n\
\t  -s Dial String  \n\
\t  -t Timeout\n"
$ 
$   Don't translate the word mkslip (the command)
$ 
M_MSG_2              "This program needs at least 1 argument.\n"
M_MSG_3              "Invalid baud rate %d.\n"
M_MSG_5              "Retrieval of error message failed.\n"
M_MSG_7              "\nThis is a duplicate slip name.\nPlease select another name.\n\n"
M_MSG_8              "A Timeout of less than 30 seconds is not allowed.\n"
M_MSG_9              "Error adding %s;\nCharacter : is invalid.\n"
