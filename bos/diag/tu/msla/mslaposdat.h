/* @(#)23	1.2  src/bos/diag/tu/msla/mslaposdat.h, tu_msla, bos411, 9428A410j 6/15/90 17:23:45 */
/*
 * COMPONENT_NAME: (mslaposdat.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


static unsigned short msla_detect[NO_OF_TESTBYTES] = {
                            0x0000,
                            0xFFFF,
                            0x2222,
                            0x4444,
                            0x8888,
                            0xAAAA,
                            0x5555};
static unsigned short msla_pattern[NO_OF_TESTBYTES + 1] = {
                            0x00FF,
                            0xFF00,
                            0x2244,
                            0x4422,
                            0x8811,
                            0x1188,
                            0xAA55,
                            0x55AA};
