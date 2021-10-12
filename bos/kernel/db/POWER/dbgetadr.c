static char sccsid[] = "@(#)55	1.7  src/bos/kernel/db/POWER/dbgetadr.c, sysdb, bos411, 9428A410j 6/16/90 03:01:16";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: get_addr
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

/*#define debug 1*/
#include <sys/seg.h>
#include "parse.h"
#include "debvars.h"
#include "debaddr.h"


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern struct debaddr *setup_debaddr();		/* found in vdbsup.c */

#define DBVIRT	ps->token[i].debaddr->virt
#define DBBUS	ps->token[i].debaddr->bus
#define DBINMEM	ps->token[i].debaddr->inmem

/*
* NAME: get_addr
*
* FUNCTION: check and process an address field to determine if the input
*	address is valid.
*
* PARAMETERS: pos is the position within the parser structure
*		array
*	      ps is the parser structure
*
* Execution Enviroment: stand alone debugger
*   address formats:		
*     xxxxxxxx	 - hex value
*     R xxxxxxxx - Real memory address			
*     V xxxxxxxx - Virtual memory address	
*					
* Returns '1'b if valid address 
*	   Fixes Parser Output Structure if R/V xxxxxxxx form
*	   in_mem is set if the memory is paged in,
*	   '1'b is returned even if the address is not in.
*	   Bus is set if it's in bus memory.	
*					
*/

int
get_addr(pos, ps)
int	pos;		/* token index */
struct 	parse_out *ps;	/* parser structure */
{
	int i,j;
	int addr_location;
	char tmpvirt;

	/* check if it's in the R/V form  ... eg. D R 200 */
	if ((strlen(ps->token[pos].sv)==1) && 
		(ps->num_tok>pos) &&
		((ps->token[pos].sv[0]=='v') || (ps->token[pos].sv[0]=='r'))) {
			tmpvirt = (ps->token[pos].sv[0] == 'v');
			i = pos + 1;  /* look at the address in parser struct */
	}
	else {			/* no R/V */
		tmpvirt = T_BIT ? 1 : 0; /* set to translate bit by default */
		i = pos;	/* don't move the parser index */
	}
	addr_location = i;

	/* check for a valid address field */
	if (ps->token[i].tflags & HEX_VALID) { 	/* looks good */
		ps->token[i].debaddr = setup_debaddr(ps->token[i].hv, tmpvirt);
		DBINMEM = debug_xlate(ps->token[i].hv,tmpvirt);
#ifdef _IBMRT
		DBBUS =  DBINMEM && (ps->token[i].hv & IOORG);
#endif /* _IBMRT */
	}
	if (i > pos) {   	/* if addr of R/V form */
		j = pos;
		while (i <= ps->num_tok) {
			ps->token[j] = ps->token[i];
			ps->loc[j] = ps->loc[i];
			i++;
			j++;
		}
		ps->num_tok--;	/* one less token */
	}
#ifdef debug
	for (j=0; j<=ps->num_tok; j++,printf("\n")) {
		printf("ps->num_tok: %x  ",ps->num_tok);
		printf("ps->loc: %x  ",ps->loc[j]);
		printf("ps->delim_char: %x  ",ps->delim_char[j]);
		printf("ps->token[j].tflags: %x  ",ps->token[j].tflags);
		printf("ps->token[j].hv: %x  ",ps->token[j].hv);
		printf("ps->token[j].dv: %x  ",ps->token[j].dv);
		printf("ps->token[j].debaddr->virt: %x  ",
		ps->token[j].debaddr->virt);
 		printf("ps->token[j].debaddr->bus: %x  ",
		ps->token[j].debaddr->bus);
 		printf("ps->token[j].debaddr->inmem: %x  ",
		ps->token[j].debaddr->inmem);
 		printf("ps->token[j].debaddr->addr: %x  ",
		ps->token[j].debaddr->addr);
 		printf("ps->token[j].debaddr->segid: %x  ",
		ps->token[j].debaddr->segid);
	  	printf("ps->token[j].sv: %s  ",ps->token[j].sv);
	  	printf("this is sv length: %x\n",strlen(ps->token[j].sv));
	}
#endif  /* debug */
	return(ps->token[addr_location].tflags & HEX_VALID);
}
