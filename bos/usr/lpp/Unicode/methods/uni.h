/* @(#)58	1.1  src/bos/usr/lpp/Unicode/methods/uni.h, cfgnls, bos411, 9428A410j 1/21/94 10:16:34  */
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct {
    int cmask;
    int cval;
    int shift;
    long lmask;
    long lval;
} Tab;
 
static Tab tab[] = {
    0x80, 0x00, 0*6, 0x7F,         0,              /* 1 byte sequence */
    0xE0, 0xC0, 1*6, 0x7FF,        0x80,           /* 2 byte sequence */
    0xF0, 0xE0, 2*6, 0xFFFF,       0x800,          /* 3 byte sequence */
#ifdef UCS4 
    0xF8, 0xF0, 3*6, 0x1FFFFF,     0x10000,        /* 4 byte sequence */
    0xFC, 0xF8, 4*6, 0x3FFFFFF,    0x200000,       /* 5 byte sequence */
    0xFE, 0xFC, 5*6, 0x7FFFFFFF,   0x4000000,      /* 6 byte sequence */
#endif 
    0,                                             /* end of table */
};
 
