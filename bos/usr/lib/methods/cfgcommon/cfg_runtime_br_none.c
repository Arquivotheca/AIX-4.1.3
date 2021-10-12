static char sccsid[] = "@(#)08  1.1  src/bos/usr/lib/methods/cfgcommon/cfg_runtime_br_none.c, cfgmethods, bos411, 9428A410j 6/28/94 07:11:22";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_runtime_br_none.c - used for devices 
 *						        that do not have bus
 *							resources
 *
 * FUNCTIONS: runtime_busresolve
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


int
runtime_busresolve()
{
	/* This device has no bus resources and is neither an MCA card
	   nor a PCI card.  So simply return. */

	return(0);
}
