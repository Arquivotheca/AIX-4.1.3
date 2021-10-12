# @(#)28	1.3  src/bos/usr/bin/mh/support/mhl.format.S, cmdmh, bos411, 9428A410j 2/15/93 12:25:54
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
: -- using template mhl.format --
overflowtext="***",overflowoffset=5
leftadjust,compwidth=9
ignores=msgid,message-id,received
Date:formatfield="%<(nodate{text})%{text}%|%(putstr(pretty{text}))%>"
To:
cc:
:
From:
Subject:
:
extras:nocomponent
:
body:nocomponent,overflowtext=,overflowoffset=0,noleftadjust
