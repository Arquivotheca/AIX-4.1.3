/* @(#)69	1.9  src/bos/kernel/db/token.h, sysdb, bos411, 9428A410j 6/16/90 03:04:15 */
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

/*
 * Debugger parser token structure.
 * One of these per command element.
 *
 */

#define MAXTOKENLEN 33

/* Token values. */
#define HEX_OVERFLOW	0x200	/*   Too large as a hex number		*/
#define DEC_OVERFLOW	0x100	/*   Too large as a decimal number	*/
#define HEX_VALID 	0x80	/*   Valid as hex number  		*/
#define DEC_VALID 	0x40	/*   Valid as dec number   		*/
#define GPR_NAME  	0x20	/*   Gnn or Rnn	     			*/
#define SCR_NAME  	0x10	/*   Snn		     		*/
#define QUOTED    	0x08	/*   Quoted string 			*/
#define EXP_TYPE  	0x04	/*   Expression 			*/
#define ERR	  	0x02	/*   In Error 				*/
#define YES_ON    	0x01	/*   on if Yes or On 			*/
#define HEXDEC (HEX_VALID | DEC_VALID)

struct token_def {
    short varidx;		/* Variable index */
    short tflags;		/* flags */
    ulong hv;			/* Hex, if valid	     */
    ulong dv;
    struct debaddr *debaddr;
    char  sv[MAXTOKENLEN]; 	/* the token itself	     */
   };
#define TOKENDV token.dvaddr
/*
    union {
    	ulong dv;	  	
    	struct debaddr *debaddr;
    } dvaddr;
    */
