$ @(#)88	1.2  src/bos/usr/lib/netls/common/ifor_common.msg, libnetls, bos411, 9428A410j 4/12/94 09:54:56
$
$   COMPONENT_NAME: libnetls
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ This message file contains the basic recommended messages that an 
$ application should use when enabled with the iFOR/LS license server
$ product.  Additional messages specific to an application should
$ be included in a separate message file for the application.

$ Define the quote character.
$quote "

$set IFOR_SET1	Message catalog set number 1 for the iFOR/LS common code.
$               All messages in this set should be translated.


$ The following messages are used to show the user the current status
$ of license requests.  These messages are for the end-user.
LIC_OK		"\nYour license has been granted.\n"
LIC_WAIT	"Waiting for a license...\n"
Q_POS		"Your positions on the server queues are:\n"
LIC_USERS	"\nThe current users of this product are:\n"
SOFT_WARN	"WARNING:  You are continuing without a software license!\n"
Q_MENU		"\n You are waiting in a queue for a license...\n\
 Type    \"1\"  to quit waiting and exit the program \n\
         \"2\"  to print your queue position \n\
         \"3\"  to print a list of the users using licenses \n"
SKIP_OPT	"         \"4\"  to skip waiting for a license\n"
Q_PROMPT	" press \"Enter\" after making your selection.\n"
Q_ERR		"\nAn error has occurred in a server queue.\n"

$ The following headers are used to identify the columns of data that
$ are returned when checking queue information.  These messages are
$ for the end-user.
QUEUE_HDR	"Queue"
POSITION_HDR	"Position"
USER_HDR	"User"
NODE_HDR	"Node"
USING_HDR	"Using"
START_HDR	"Start Time"

$ The following labels are used for identifying information when errors
$ are logged.  These messages are for the system administrator.
PRODUCT_LBL	"  PRODUCT: "
USER_LBL	"  USER: "
HOST_LBL	"  HOST: "
ERR_LBL		"  ERROR: "
ERR_SOFT	"  ERROR: Unlicensed Program Execution"
