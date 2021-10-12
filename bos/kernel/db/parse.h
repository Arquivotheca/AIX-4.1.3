/* @(#)64	1.6  src/bos/kernel/db/parse.h, sysdb, bos411, 9428A410j 6/16/90 03:03:42 */
#ifndef _h_PARSE
#define _h_PARSE
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/**********************************************************************
*                                                                     *
*  Output structure returned by Debug_Parser:                         *
*                                                                     *
**********************************************************************/
#define MAXTOKENS 7
#include "token.h"
struct parse_out {
    long num_tok;                       /* Number of tokens */
    struct token_def token[MAXTOKENS];  /* Tokens */
    caddr_t loc[MAXTOKENS];		/* token's address	*/
    char delim_char[MAXTOKENS];         /* Delimiters */
};

#endif
