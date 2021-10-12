# @(#)33	1.5  src/bos/usr/bin/mh/support/replcomps.S, cmdmh, bos411, 9428A410j 2/15/93 12:33:20
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
%(lit)%(formataddr %<{reply-to}%|%<{from}%|%{sender}%>%>)\
%<(nonnull)%(void(width))%(putaddr To: )\n%>\
%(lit)%(formataddr{to})%(formataddr{cc})%(formataddr(me))\
%<(nonnull)%(void(width))%(putaddr cc: )\n%>\
%<{fcc}Fcc: %{fcc}\n%>\
%<{subject}Subject: Re: %{subject}\n%>\
%<{date}In-reply-to: (Your message of \
%<(nodate{date})%{date})%|%(pretty{date})%>.)%<{message-id}
             %{message-id}%>\n%>\
--------
