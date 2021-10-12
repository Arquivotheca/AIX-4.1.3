static char sccsid[] = "@(#)59	1.3  src/bos/kernel/epost/POWER/epost.c, sysepost, bos411, 9428A410j 3/22/93 15:52:33";
/*
 * COMPONENT_NAME: (SYSEPOST) Power On Self Tests
 *
 * FUNCTIONS: epost
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
 */

extern int fpu_epost();
extern void write_leds();

/* EPOST Driver
 *      This should probably run through a table of EPOST's
 *      but it seems useless to do so for only one EPOST (fpu)
 */

int
epost()
{
    int rc;

    write_leds(0x81500000);
    rc = fpu_epost();
    while (rc) write_leds(0x81500000);	/* endless loop if bad return code */
    
    write_leds(0xfff00000);		/* blank LED's on exit */
}
