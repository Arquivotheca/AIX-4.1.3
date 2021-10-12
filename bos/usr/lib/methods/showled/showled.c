static char sccsid[] = "@(#)81	1.1  src/bos/usr/lib/methods/showled/showled.c, cfgmethods, bos411, 9428A410j 11/15/89 16:44:07";

/*
 * COMPONENT_NAME: CFGMETH  showled program to show a value on R2 leds
 *
 * FUNCTIONS: main,
 *
 * ORIGINS: IBM
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <fcntl.h>

/*
 * led display values are 0-9 for hex digits 0-9, and A-E are displayed below:
 * hex digit   '8'   '9'   'A'   'B'   'C'   'D'   'E'   
 *             +--+  +--+              +  +  +--+  +
 *             |  |  |  |              |  |  |     |
 *             +--+  +--+  +--+  +--+  +--+  +--+  +--+
 *             |  |     |  |        |              |
 *             +--+     +  +--+  +--+        +--+  +--+
 * hex digit   '8'   '9'   'A'   'B'   'C'   'D'   'E'   
 * 
 * hex digit 'F' is blank, eg. '8F8' would display '8 8'
 */ 

/*
 * NAME: showled
 *                                                                    
 * FUNCTION: display values 0 - FFF in LED's front Display
 *                                                                    
 * NOTES:
 *	usage:   showled <value>
 *    			<value> value 0 to 0xFFF inclusive decimal, hex, or
 *				octal (0-4095,0x0-0xfff,0-07777) to show on
 *				system leds
 *		 showled
 *			without <value> argument, effect is same as 0xfff;
 *			i.e., leds are blanked.
 *
 * EXIT VALUE DESCRIPTION: 
 *    1 -- error occurred
 *    0 -- value displayed
 */  

main(argc,argv)
int argc;
char **argv;
{
int fd;
	if ((fd=open("/dev/nvram", O_RDWR))<0 ) {
		exit(1);
	}
	if (ioctl(fd, MIONVLED, strtol((argc>1)?argv[1]:"0xfff",NULL,0))<0) {
		exit(1);
	}
	exit(0);
}
