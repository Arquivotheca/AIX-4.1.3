static char sccsid[] = "@(#)10	1.3  src/bos/usr/lib/methods/cfgdlc/leds.c, dlccfg, bos411, 9428A410j 10/19/93 09:43:00";
/*************************************************************************
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: leds.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 ************************************************************************/


#include <sys/types.h>
#include <fcntl.h>
#include <sys/mdio.h>

#define TRUE 1
#define FALSE 0

static int _led_fd = -1;

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
 * NAME: ledv
 *                                                                    
 * FUNCTION: display values 0 - FFF in LED's front Display
 *                                                                    
 * NOTES:
 *    dvalue : value 0 to 0xFFF inclusive to display
 *
 * RETURN VALUE DESCRIPTION: 
 *    TRUE (1) : if value was successfully displayed
 *   FALSE (0) : an error occurred.
 */  

int 
setleds(dvalue)
int dvalue;
{
    if (_led_fd < 0)
	if (0 > (_led_fd = open("/dev/nvram", O_RDWR)) ) {
	    perror("ledv");
	    return FALSE;
	}
    if (0 > ioctl(_led_fd, MIONVLED, dvalue)) {
	perror("ledv");
	return FALSE;
    }
    return TRUE;
}
