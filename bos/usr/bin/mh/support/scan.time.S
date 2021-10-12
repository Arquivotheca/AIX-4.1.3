# @(#)35	1.3  src/bos/usr/bin/mh/support/scan.time.S, cmdmh, bos411, 9428A410j 2/15/93 12:36:40
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
%4(msg)%<(cur)+%| %>%<{replied}-%| %>\
%02(mon{date})/%02(mday{date}) \
%02(hour{date}):%02(min{date})%3(tzone{date})\
%<{date} %|*%>\
%<(mymbox{from})To:%14(friendly{to})%|%17(friendly{from})%>  \
%{subject}%<{body}<<%{body}%>
