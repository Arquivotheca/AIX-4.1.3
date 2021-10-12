# @(#)23	1.3  src/bos/usr/bin/mh/support/digestcomps.S, cmdmh, bos411, 9428A410j 2/15/93 12:20:08
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
From:	  %{digest}-Request
To:	  %{digest} Distribution: dist-%{digest};
Subject:  %{digest} Digest V%(msg) #%(cur)
Reply-To: %{digest}
--------
%{digest} Digest	%(weekday{date}), %2(mday{date}) %(month{date}) 19%02(year{date})
		Volume %(msg) : Issue %(cur)

Today's Topics:
