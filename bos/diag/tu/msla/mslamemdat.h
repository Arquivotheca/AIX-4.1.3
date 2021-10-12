/* @(#)59	1.2  src/bos/diag/tu/msla/mslamemdat.h, tu_msla, bos411, 9428A410j 6/15/90 17:23:36 */
/*
 * COMPONENT_NAME: (mslamemdat.h) header file for MSLA diagnostic
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


static char rdwr_byte[NO_OF_TESTBYTES] = { 0X00, 0XFF, 0XAA, 0X55, 
                                          0X00, 0XF0, 0X0F };
static unsigned short msla_detect[NO_OF_TESTBYTES] = {
                            0x33CC,
                            0xFFFF,
                            0x2222,
                            0x4444,
                            0x8888,
                            0xAAAA,
                            0x5555};
