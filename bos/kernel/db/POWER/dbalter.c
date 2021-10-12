static char sccsid[] = "@(#)47	1.8  src/bos/kernel/db/POWER/dbalter.c, sysdb, bos411, 9428A410j 6/16/90 03:00:14";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: alter
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

#include <sys/types.h>
#include "parse.h"                      /* parser structure             */
#include "debaddr.h"
#include "vdberr.h"			/* Error message stuff		*/

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int get_from_memory();   /* get data from memory         */
extern int write_to_memory();   /* store data to memory         */
extern int cmd_error();         /* put up error message         */

#define ADDR ps->token[1].debaddr->addr
#define VRT ps->token[1].debaddr->virt

/*
 * NAME: alter
 *
 * FUNCTION:
 *   VRM Debugger Alter command                     
 *      Change memory at the specified address.    
 *      Change on a nibble basis, not just a byte basis.              
 *
 *
 * RETURN VALUE: none
 */

alter(ps)
struct parse_out *ps;			/* Parser structure 		*/
{
  	caddr_t a, c, olda;
  	uchar byte, binc;
  	int left;
	char 	errst[9];

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Number too large\n");
		return 0;
	}

  	a = (caddr_t) ps->token[1].hv;   	 	/* address to be altered	*/
  	left = TRUE;

  	/* search thru the input string byte by byte 	 	*/
  	for (c=ps->token[2].sv; *c!='\0'; c++) {   /* convert char to binary */
    		if (*c == ' ') continue;
			/* char string converted to upper case  */
    			if ((*c>='a')&&(*c<='f')) /* sub ascii char value */   
       				binc = *c - 'a' + 0x0a;   
    			else if ((*c>='0')&&(*c<='9'))  
				binc = *c - '0';
     	 		else {		/* error condition 	*/
      	   			vdbperr(ivd_text1,c,ivd_text2);
      	   			return(0);
			}

    		olda = a;		/* save ptr to current address	*/
    		/* get the data from memory that is at address to be altered*/
    		if (!get_from_memory(a,VRT,&byte,sizeof(byte))) {
	          	sprintf(errst,"%x",a);
	          	vdbperr(page_not_in_real1, errst, page_not_in_real2);
       			return(0);
       		}

		/* byte contains the data returned by get_from_memory	 */
		/* replace left nibble then right nibble		 */

    		if (left) {                 	/* left half of byte   */
      			byte=byte & 0x0f; 	/* mask off left side	 */
      			byte=byte | (binc<<4);  /* shift new data into left */
      			left=FALSE;    
      		}
    		else {        	           	/* Right half of byte  	*/
      			byte=byte & 0xf0;  	/* Replace nibble	*/
      			byte=byte | binc;	/* move new data to rt side */
      			left=TRUE;
      			a++;			/* Bump storage address	*/
      		}

	    	/* byte contains the new data to be written to memory        */
    		if (!write_to_memory(olda,VRT,&byte,sizeof(byte))) {
	        	/*sprintf(errst,"%x",olda);*/
			printf("error writing to memory\n");
	        	/*vdbperr(deb_err1, errst, deb_err2); */
		}
  	}     
	return(0);
} 
