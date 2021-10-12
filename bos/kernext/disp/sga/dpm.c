static char sccsid[] = "@(#)84  1.5  src/bos/kernext/disp/sga/dpm.c, sgadd, bos411, 9440A411a 10/3/94 16:57:37";
/*
 *   COMPONENT_NAME: SGADD
 *
 *   FUNCTIONS: vttpwrphase 
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

#if 0

/*
 *                        Screen saver for sga adpater.
 *
 * Method:  turning off video.  That is we enable blanking to the screen by
 *          setting bit 6 of the control register 1 in the SPD2.
 *        
 */
#include "INCLUDES.h"

#define SPD2_VIDEO_ON	0xBF

long vttpwrphase(pd, phase)
struct phys_displays *pd;
int phase;
{
        caddr_t old_base_addr;
	int parityrc;
	label_t jmpbuf;
	unsigned char i;

	/* Set up Parity Handler */
	parityrc = setjmpx(&jmpbuf);

	if (parityrc)
	{ 
	   if (parityrc == EXCEPT_IO_SGA)
	   { /* log an error and return */
              sga_err(NULL,"SGADD","RESET","SETJMPX",parityrc,
                                SGA_IO_EXCEPT,RAS_UNIQUE_1);
	      BUSMEM_DET(bus_base_addr);
              return( parityrc );
           }
          else
          {
              longjmpx(parityrc);
          }
	}
	
	old_base_addr = bus_base_addr; 

	bus_base_addr = BUSMEM_ATT(BUS_ID,SGA_ADDR_BASE);

	if( phase == 1 )
	{
	   /* 
            * set index register to access Control Register 1
            */
	   *SPD2AD  = CNTL_REG1;   

	   i = *SPD2DT1;          /* read from data port*/

	   i &= SPD2_VIDEO_ON;    /* turn on video      */ 

  	   *SPD2DT1 = i;          /* write to data port */ 

	}
	else 
        {
           /* 
            * for phase 2,3, and 4 we turn on blanking so
            * we don't have to do much if adapter is already 
            * in phase 2
            */
 	   if ( phase == 2 )
	   {
	      /* 
               * set index register to access Control Register 1 
               */
	      *SPD2AD  = CNTL_REG1;   

	      i = *SPD2DT1;          /* read from data port*/
   
	      i |= (~SPD2_VIDEO_ON);    /* turn off video  */ 

  	      *SPD2DT1 = i;          /* write to data port */ 

	   }
	}

	clrjmpx(&jmpbuf);

        BUSMEM_DET(bus_base_addr);

	bus_base_addr = old_base_addr;
	
	printf("sga: devno = %x, phase = %d\n",pd->devno, phase);

	return(0);
}

#endif


/*
 * Screen saver and Display Power Management for SGA adpater.
 *
 * Method:  turning off video and syncs.  This is done by 
 *          setting bit 7 in the Adapter Control Register to zero 
 *
 *          That is this function only supports full-on and off phases
 *        
 */
#include "INCLUDES.h"

long vttpwrphase(pd, phase)
struct phys_displays *pd;
int phase;                   /* 1=full-on, 2=standby, 3=suspend, 4=off */
{
        ulong old_base_addr;
	int parityrc;
	label_t jmpbuf;
	unsigned long i;

	/* Set up Parity Handler */
	parityrc = setjmpx(&jmpbuf);

	if (parityrc)
	{ 
	   if (parityrc == EXCEPT_IO_SGA)
	   { /* log an error and return */
              sga_err(NULL,"SGADD","RESET","SETJMPX",parityrc,
                                SGA_IO_EXCEPT,RAS_UNIQUE_1);
	      BUSMEM_DET(bus_base_addr);
              return( parityrc );
           }
          else
          {
              longjmpx(parityrc);
          }
	}
	
	old_base_addr = bus_base_addr;     /* save old segment */

	bus_base_addr = BUSMEM_ATT(BUS_ID,SGA_ADDR_BASE);        /* get new segment */

	/*
         * 1. if caller wants to turn on the display when it's off, we turn it on,
         *    and then update current DPM state
         * 2. if caller wants to turn off the display when it's on, we turn it off.
         *    and then update current DPM state
         * 3. Other than that just update current DPM state
         *
         *    Note initially current_dpm_phase is zero because the whole pd is bzeroed out
         *    Also, every lft is unconfigured, it calls vttterm.  In there current_dpm_phase
         *    is set to zero again.
         */

	if( (phase == 1) && (phase != pd->current_dpm_phase) )     /* case 1 */
	{
	      i = *ADCNTL;
	      *ADCNTL = i | VIDEO_ON ;
	}
	else if ( (phase != 1) && (pd->current_dpm_phase < 2) )    /* case 2 */
        {
	      i = *ADCNTL;
	      *ADCNTL = i & ( ~VIDEO_ON );
	}

        BUSMEM_DET(bus_base_addr);

	bus_base_addr = old_base_addr;     /* restore data */ 

	pd->current_dpm_phase = phase;

	clrjmpx(&jmpbuf);

	return(0);
}
