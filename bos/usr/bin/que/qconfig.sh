* @(#)33	1.6  src/bos/usr/bin/que/qconfig.sh, cmdque, bos411, 9428A410j 2/4/94 10:45:05
*
* COMPONENT_NAME: cmdque configuration file for spooling 
*
* FUNCTIONS: 
*
* ORIGINS: 27
*
* (C) COPYRIGHT International Business Machines Corp. 1993, 1994 
* All Rights Reserved
* Licensed Materials - Property of IBM
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
* PRINTER QUEUEING SYSTEM CONFIGURATION
*
* This configuration file contains valid configurations for remote 
* print queue rp0, local print queue lp0 and batch queue bsh.
* They may be deleted or changed as necessary.
*
* EXAMPLE of remote print queue configuration
* rp0:
*	host = hostname
*	s_statfilter = /usr/lib/lpd/aixshort
*	l_statfilter = /usr/lib/lpd/aixlong
*	rq = queuename
*	device = drp0
*
* drp0:
*	backend = /usr/lib/lpd/rembak
*
* EXAMPLE of local print queue configuration
*lp0:
*	discipline = fcfs
*	up = TRUE
*	device = dlp0
*
*dlp0:
*	backend = /usr/lib/lpd/piobe
*	file = FALSE
*	access = write
*	feed = never
*	header = never
*	trailer = never
*
* BATCH queue for running shell scripts
*
*bsh:
*	device = bshdev
*	discipline = fcfs
*bshdev:
*	backend = /usr/bin/bsh
