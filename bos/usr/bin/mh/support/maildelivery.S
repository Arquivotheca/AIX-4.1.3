# @(#)26	1.2.1.2  src/bos/usr/bin/mh/support/maildelivery.S, cmdmh, bos411, 9428A410j 2/15/93 12:22:50
#   COMPONENT_NAME: CMDMH
#
#   FUNCTIONS: 
#
#   ORIGINS: 26,27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# On any delivery execute rcvtty with echo command. This
# will cause the message 'Mail arriving' on tty.
*      	-      	|      	R     "/usr/lib/mh/rcvtty echo 'Mail arriving\n'"
