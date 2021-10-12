static char sccsid[] = "@(#)70	1.9  src/bos/kernel/db/POWER/vdbdisp.c, sysdb, bos41J, 9509A_all 2/23/95 11:51:00";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: debug_display, debug_bin_disp, disp_short
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include "parse.h"                      /* parser structure             */
#include "debaddr.h"
#include "vdberr.h"			/* Error message stuff		*/

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int debug_xlate();   	/* verify that an address is in memory */ 
extern int get_from_memory();   /* get data from memory         */
extern int get_put_aligned();   /* get char, halfword or word   */

/*
 * NAME: debug_display
 *
 * FUNCTION:
 *   LLDB display command                     
 *
 * 	This function is called via the display command from the debugger
 * 	driver program. The input parameters are the debugger address 
 * 	structure and the parser structure. This program will display a
 *	portion of real or virtual memory. An example of the user command
 *	is: "display 2e450 200". This program will get retrieve the 200
 *	bytes following 2e450 byte by byte using the get_from memory function.
 * 	Display the requested number of bytes unless the user wants 1, 2 or 4 
 *	which calls a special routine for load char, load half and load
 *	full word instructions.
 *
 *
 * RETURN VALUE:
 */

char	*temprsg="first string";
char	*temprsg1="second string loop";
  	uchar asciitxt[17];

debug_display(addr,length,virt)
ulong addr;
int     length, virt;
{
	int i, j;
	char b;

    	asciitxt[16] = '\0';				/* null end of string */

	if (length >= 0x2000) length = 0x2000;	/* to prevent long dumps */
	if ((length==1)||(length==2)||(length==4)) /* LC, LH or L */
		disp_short(addr,length,virt);	/* display 1,2 or 4 bytes */
	else {				/* display at least 1 line */
		for (i=0; i<length; i=i+16) {
	 		if (virt) {
				printf("%08x  ",addr);
			}
			else {
				printf("R %06x  ",addr);
			}
			for (j=0; j<=15; j++) { /* get memory byte by byte */
				if ((j%4) == 0)  
					printf(" "); 
				if (get_from_memory(addr,virt,&b,1)) { 
					printf("%02x",b);	
					asciitxt[j] = ((b < 0x20) || 
						(b >= 0x7f))?'.':b; /* ascii */
	      			}
				else  {  /* byte not in memory */
					printf("__");
					asciitxt[j] = '.';
				}
				/*
				 * Check if requested number of bytes exhausted.
				 */
				if ((i + j) >= (length-1))
				{
					/*
					 * Bytes exhausted.  Must output spaces to
					 * ASCII column and fill in remainder of
					 * ASCII array with blanks.
					 */
					j++;
					while (j <= 15)
					{
						if ((j%4) == 0)
							printf(" ");
						printf("  ");	/* spaces */
						asciitxt[j] = 0x20;
						j++;
					}
					printf("   |%s|\n",asciitxt);
					return;
				}
				addr++; 	/* next byte in memory	*/
			}
	  		printf("   |%s|\n",asciitxt);

		}
        }
}

/*
 * NAME: debug_bin_disp
 *
 * FUNCTION:
 *   VRM Debugger display command                     
 *
 * 	This function is called via the display command from the debugger
 * 	driver program. The input parameters are the debugger address 
 * 	structure and the parser structure. This program will display a
 *	portion of real or virtual memory. An example of the user command
 *	is: "display 2e450 200". This program will get retrieve the 200
 *	bytes following 2e450 byte by byte using the get_from memory function.
 * 	Display at least 1 line (16 bytes) unless the user wants 1, 2 or 4 
 *	which calls a special routine for load char, load half and load
 *	full word instructions.
 *
 *
 * RETURN VALUE:
 */

debug_bin_disp(addr,length,virt)
ulong addr;
int     length, virt;
{
	int i, j;
	char b;

	/*
	 * This routine is only called from the low level
	 * debugger so we assume the tty is already open.
	 */

	if (length >= 0x2000) length = 0x2000;	/* to prevent long dumps */
	if ((length==1 || length==2 || length==4 ) && /* LC, LH or L */
	    !(addr & (length - 1)))
		disp_bin_short(addr,length,virt);/* display 1,2 or 4 bytes */
	else {
		int	oaddr = addr;
		int olength = length;
		
		/*
		 * First check that all the pages covered by the range of addresses:
		 *      [addr, addr+length-1]
		 * are present in memory, if it is not completely present write a
		 * binary 0 and return.
		 */
	
		while (length > 0) {
			if (! get_from_memory (addr, 1, &b, 1)) {
				d_ttybinput (0);
				return;
			}
			addr += PAGESIZE;
			length -= PAGESIZE;
		}
		
		/*
		 * Need to check last byte in the range,
		 * the loop above does not check the end.
		 */
		addr = oaddr + olength -1;
		if (! get_from_memory (addr, 1, &b, 1)) {
			d_ttybinput (0);
			return;
		}

		/*
		 * All the pages are present,
		 * so just display the bytes
		 * in binary form.
		 */
		d_ttybinput (1);
		while (oaddr <= addr) {
			if (! get_from_memory (oaddr, 1, &b, 1))
				printf("wrong check for paged out\n");
			d_ttybinput (b);
			++oaddr;
		}
	}
}
 
		
/*****************************************************************
*
*  disp_short:
*	this function will display 1, 2 or 4 bytes of memory.
*	Debug_xlate will check to make sure the address is in 
*	memory. Get_put_aligned will get the data from memory.
*	The desired amount of data is displayed.
*
******************************************************************/ 

disp_short(addr,length,virt)
ulong addr;
int   length, virt;
{

        static char line1[] ="__";		/* 1 byte	*/
        static char line2[] ="____";		/* 2 bytes	*/
        static char line3[] ="________";	/* 4 bytes	*/
	char b[4];				/* data area 	*/
	int false = 0;
	ulong data;

        if (!debug_xlate(addr,virt))   /* in_memory? */
	  	printf("%08x  %s\n",addr,(length==1) ? line1 :
			(length==2) ? line2 : line3);
	else {				/* get the data correctly */
 	  if (!get_put_aligned(addr,virt,&data,false,length)) 
	    vdbperr(not_in_real);
	  else
            switch(length) {		/* display data */
	      case (1):
	         printf("%08x %02x\n",addr, data);
		 break;
	      case (2):
	         printf("%08x %04x\n",addr, data);
		 break;
	      case (4):
	         printf("%08x %08x\n",addr, data);
		 break;
	    }
	  }
}
		
/*****************************************************************
*
*  disp_bin_short:
*	this function will display 1, 2 or 4 bytes of memory.
*	Debug_xlate will check to make sure the address is in 
*	memory. Get_put_aligned will get the data from memory.
*	The desired amount of data is displayed.
*
******************************************************************/ 

disp_bin_short(addr,length,virt)
ulong addr;
int   length, virt;
{
	char	*p;		
	char	b[4];

	if (! debug_xlate(addr,virt) ||
	    ! get_put_aligned (addr, virt, b, 0, length))
		d_ttybinput (0);
	else {
		d_ttybinput (1);
		p = b + 4 -length;
		while (--length >=0)
			d_ttybinput (*p++);
	}
}
