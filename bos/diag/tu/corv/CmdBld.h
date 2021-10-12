/* @(#)31       1.1  R2/cmd/diag/tu/corv/CmdBld.h, tu_corv, bos325 7/22/93 18:57:16 */
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef CMDBLD
#define CMDBLD

typedef struct {
     char bytes[200];
     int  length;
     char *dma_buffer;
     int  dma_length;
} COMMAND;

/* getbits:  get n bits from position p */
extern unsigned get_bits(unsigned, int, int);
extern void random_data(char *, int, int);
extern COMMAND build_cmd(char *, ...);

#endif
