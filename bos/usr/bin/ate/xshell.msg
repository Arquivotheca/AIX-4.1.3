$ @(#)33	1.4  src/bos/usr/bin/ate/xshell.msg, cmdate, bos411, 9428A410j 6/15/90 20:20:43
$
$ COMPONENT_NAME: BOS xshell.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 9  27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for xshell
$
$quote "

$set MS_XSHELL


MSG5            "ate: 0828-005 The system is ready to receive file %s.\r\n\
              Use ctrl-X to stop xmodem.\r\n"
$ Do not translate ctrl-X, xmodem.
MSG16           "ate: 0828-016 The system is ready to send file %s.\r\n\
              Use ctrl-X to stop xmodem.\r\n"
$ Do not translate ctrl-X, xmodem.
MSG17           "ate: 0828-017 The Receive command cannot create or open the\r\n\
              file %s\r\n\
              because of the error shown below.\r\n"
$ Do not translate Receive
MSG18           "ate: 0828-018 The Receive command cannot complete because you did not\r\n\
              specify an input file name.\r\n"
$ Do not translate Receive
MSG22           "ate: 0828-022 The Send command cannot open the\r\n\
              file %s\r\n\
              because of the error shown below.\r\n"
$ Do not translate Send
MSG23           "ate: 0828-023 The Send command cannot complete because you did not\r\n\
              specify an output file name.\r\n"
$ Do not translate Send
MSG39           "ate: 0828-039 The xmodem command cannot complete because a flag is not valid.\r\n\
              Valid flags are -r for receive, -s for send, or -p for passthrough.\r\n\
              Enter the command again using one of these flags.\r\n"
$ Do not translate xmodem, -r, receive, -s, send, -p, passthrough.
RECV            "Receive"
$ Do not translate
SEND            "Send"
$ Do not translate
MSGUNK                  "Message number %d is not in messages.c\r\n"
$ Do not translate messages.c
