static char sccsid[] = "@(#)05	1.1  src/bos/usr/ccs/lib/libdiag/led.c, libdiag, bos41B, 9504A 1/4/95 14:10:56";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS:   reset_LED_buffer
 *              fill_LED_buffer
 *              blink_LED
 *              show_LED
 *		convert_LED_value
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/systemcfg.h>

#define LED_ADDR        0x00A00320      /* address to insert LED values    */
#define OCS_ADDR        0x00A0037C      /* address to activate LED's       */
#define OCS_VALUE       0x01000000      /* value to activate LED's         */
#define OCS_LEN         0x04            /* # of bytes in OCS value         */
#define OCS_ADDR_SAL    0x00A00315      /* similar address, value and      */
#define OCS_VALUE_SAL   0xFF032200      /* length for RSC machine          */
#define OCS_LEN_SAL     0x03            /* type.                           */ 
#define OCS_RESET       0x00000000      /* value to de-activate LED's      */

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

/*  */
/*
 * NAME: reset_LED_buffer
 *
 * FUNCTION: This function resets the NVRAM LED area with all 
 * 0xFFF0 values. It also turns the LED off in case some value
 * had been left on.  
 *        
 * NOTES: If this function is not supported on the platform, then
 * it simply returns. 
 *
 * RETURNS: none
 *
 */

void
reset_LED_buffer() 
{
        static short leds[32] = {
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0,
                                0xFFF0, 0xFFF0, 0xFFF0, 0xFFF0 };
        int ocs_flag;
        int nvramfdes;
        MACH_DD_IO mdd;

	/* Check to see if the LED function is supported */
	if ( ! has_LED_support()) return;

        /* reset NVRAM LED area to off */
        if ((nvramfdes = open("/dev/nvram", O_RDWR)) != -1){

		/* ensure led is turned off also */
		ioctl(nvramfdes, MIONVLED, leds[0]>>4);

                /* set FFF termination string in LED Save area */
                mdd.md_addr = LED_ADDR;
                mdd.md_data = (char *)leds;
                mdd.md_size = sizeof(leds);
                mdd.md_incr = MV_BYTE;
                ioctl( nvramfdes, MIONVPUT, &mdd );

                /* turn OCS LED Step mode bit off */
                ocs_flag = OCS_RESET;

        	if (__power_rsc()) { 				/* RSC models */ 
			mdd.md_addr = OCS_ADDR_SAL;
                        mdd.md_data = (char *)&ocs_flag;
                        mdd.md_size = OCS_LEN_SAL;
                        mdd.md_incr = MV_BYTE;
                        ioctl( nvramfdes, MIONVPUT, &mdd );

                } else {					/* All other models */
                        mdd.md_addr = OCS_ADDR;
                        mdd.md_data = (char *)&ocs_flag;
                        mdd.md_size = OCS_LEN;
                        mdd.md_incr = MV_BYTE;
                        ioctl( nvramfdes, MIONVPUT, &mdd );
                }

                (void) close( nvramfdes );
        }
	return;
}
/*  */
/*
 * NAME: fill_LED_buffer
 *
 * FUNCTION:  This routine puts an LED message into the OCS LED
 *              String Output Area.  If a machine check occurs,
 *              the kernel will overwrite the first part of it,
 *              but the tail section will remain.  The tail
 *              section identifies the device that was being tested.
 *
 * NOTES: If this function is not supported on the platform, then
 * it simply returns. 
 *
 * RETURNS: none
 *
 */

void 
fill_LED_buffer(short *led_buffer, int buffer_size, int console)
{
        int fd;
        int ocs;
        MACH_DD_IO mdd;

	/* Check to see if the LED function is supported */
	if ( ! has_LED_support()) return;

        fd = open("/dev/nvram", O_RDWR);

	/* if running "NO CONSOLE", then display the LED value for	*/ 
	/* the device under test.					*/
        if (!console) 
		ioctl(fd, MIONVLED, led_buffer[7]>>4);

        mdd.md_addr = LED_ADDR;
        mdd.md_data = (char *)led_buffer;
        mdd.md_size = buffer_size;
        mdd.md_incr = MV_BYTE;
        ioctl(fd, MIONVPUT, &mdd);

       	if (__power_rsc()) { 				/* RSC models */ 
                ocs = OCS_VALUE_SAL;
                mdd.md_addr = OCS_ADDR_SAL;
                mdd.md_data = (char *)&ocs;
                mdd.md_size = OCS_LEN_SAL;
                mdd.md_incr = MV_BYTE;
                ioctl(fd, MIONVPUT, &mdd);

        } else {					/* All other models */
                ocs = OCS_VALUE;
                mdd.md_addr = OCS_ADDR;
                mdd.md_data = (char *)&ocs;
                mdd.md_size = OCS_LEN;
                mdd.md_incr = MV_BYTE;
                ioctl(fd, MIONVPUT, &mdd);
        }

        (void) close(fd);

        return;

}
/*  */
/*
 * NAME: blink_LED
 *
 * FUNCTION: Blinks the LED area. Assumes previous call has been 
 *		made to initialize LED buffer area.
 *
 * NOTES: No normal return is expected from this function, unless
 * 	  the LED function is not supported on the specific model.
 *
 * RETURNS: None if successful
 *	     0 = not supported 
 *	    -1 = if error
 *
 */

int
blink_LED(short value)
{

        int fd, rc;

	/* Check to see if the LED function is supported */
	if ( ! has_LED_support()) return(0);

        fd = open("/dev/nvram", O_RDWR);

	/* Flash the leds - delay 1/2 second between */
        while (rc != -1) {
                rc = ioctl(fd, MIONVLED, value);
		usleep(500000);
                rc = ioctl( fd, MIONVLED, 0xFFF );
		usleep(500000);
        }

        (void) close(fd);

        return (rc);

}

/*
 * NAME: show_LED
 *
 * FUNCTION: Displays a value in the LED area. 
 *		Delay used to sleep (seconds) before returning.
 *
 * NOTES: 
 *
 * RETURNS:  none
 *
 */

void
show_LED(short value, int delay)
{

        int fd;

	/* Check to see if the LED function is supported */
	if ( ! has_LED_support()) return;

        fd = open("/dev/nvram", O_RDWR);

	ioctl(fd, MIONVLED, value);

        (void) close(fd);

	if ( delay ) sleep (delay);

        return;

}

/*  */
/*
 * NAME: convert_LED_value
 *
 * FUNCTION: Converts characters to the proper
 *           value for display on the LED.
 *
 * NOTES:
 * letters are displayed as xyy, where x is
 * the sequence number and yy is the alpha
 * character.  yy is 11 for a, 12 for b, etc.
 * numbers are displayed as xyy, where yy is
 * 00 for 0, 01 for 1, etc.
 *
 *
 * RETURNS:
 *       # = LED value
 *      -1 = Invalid character to represent
 *
 */
short 
convert_LED_value(char letter, int seq_no)
{
  short val_10;
  short val_16;

        val_16 = seq_no << 8;

        if (isalpha(letter))
          {
          if (isupper(letter))
            val_10 = 11 + (short )letter - (short )'A';
          else
            val_10 = 11 + (short )letter - (short )'a';
          }
        else if (isdigit(letter))
          {
          val_10 = (short )letter - (short )'0';
          }
        else
          return(-1);

        val_16 |= ((val_10 - (val_10 % 10))/10)*16 + (val_10 % 10);
        return(val_16<<4);
}
