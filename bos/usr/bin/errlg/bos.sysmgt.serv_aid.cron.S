*
* @(#)80        1.1  src/bos/usr/bin/errlg/bos.sysmgt.serv_aid.cron.S, cmderrlg, bos411, 9428A410j 3/15/94 11:29:43
*
*   COMPONENT_NAME: CMDERRLG
*
*   FUNCTIONS: errclear cron job
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1994,1994
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*

0 11 * * * /usr/bin/errclear -d S,O 30
0 12 * * * /usr/bin/errclear -d H 90

