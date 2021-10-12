/* @(#)09	1.3  src/bos/usr/lib/methods/cfgdlc/dlcdds.h, dlccfg, bos411, 9428A410j 10/19/93 09:42:26 */
/*
 * COMPONENT_NAME: (DLCCFG)  Data Link Contol Configuration
 *
 * FUNCTIONS: Common header file for Generic Data Link Control
 *            This file contains the dds structure for a DLC device
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


struct dlcconfig {
	dev_t   dev;          /* device number */
	int     maxq;         /* depth of queues */
        };
