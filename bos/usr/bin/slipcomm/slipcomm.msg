$  @(#)85  1.7  src/bos/usr/bin/slipcomm/slipcomm.msg, rcs, bos411, 9428A410j 8/6/93 12:42:22
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

$ scmc msg file created Fri Apr 30 16:44:59 1993
$ user:  elaird

$quote "         define quote character.

$len 
$set MS_slipcomm        Define initial set#

M_MSG_1              "\nUsage:  slipcomm -p Process_id -l { -c | -d } -n SlipName |\n\
\tslipcomm -p Process_id -s { -c | -d } -n SlipName |\n\
\tslipcomm -p Process_id -k -n SlipName |\n\
\tslipcomm -p Process_id -q |\n\
\tslipcomm -p Process_id -a |\n\
\tslipcomm -p Process_id -r -D Dialed_Host -L Local_Host\n"
$ 
$  Don't translate the word slipcomm (the command)
$ 
M_MSG_2              "Root privileges are required to kill a SLIP.\n"
M_MSG_3              "You must specify a process id.\n"
M_MSG_4              "Requests must be one of [ c | d | k | q | r ].\n"
M_MSG_5              "The -l and -s parameters are mutually exclusive.\n"
M_MSG_6             "The -n paramater is missing.\n"
M_MSG_7             "The -L paramater is missing.\n"
M_MSG_8             "The -D paramater is missing.\n"
M_MSG_9             "The SLIP Connection Subsystem is not running.\nPlease call your system administrator.\n"
M_MSG_10             "Unable to create the client FIFO.\n"
M_MSG_11             "Unable to open the client pipe %s.\n"
M_MSG_12             "SLIP Link connected successfully!\n"
M_MSG_13             "SLIP Link not connected.\n\
The TTY port is in use by another program.\n"
M_MSG_14             "SLIP Link not connected.\n\
The mkdev command failed.\n"
M_MSG_15             "This SLIP Link is already connected!\n"
M_MSG_16             "SLIP Link not connected.\n\
The ifconfig command failed.\n"
M_MSG_17             "SLIP Link not connected properly.\n\
One or more route add commands failed.\n"
M_MSG_18             "SLIP Link not connected.\n\
The slattach command failed.\n"
M_MSG_19             "SLIP Link not connected.\n\
The modem failed to respond.\n"
M_MSG_20             "SLIP Link not connected.\n\
You don't have any SLIPS connected.\n"
M_MSG_21             "RECONFIGURE of the SLIP Failed.\n"
