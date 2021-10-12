static char sccsid[] = "@(#)07        1.6  src/bos/kernel/proc/POWER/m_tinit.c, sysproc, bos411, 9428A410j 5/9/94 16:51:55";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: date_to_jul
 *		init_clock
 *		init_dp8570
 *		init_ds1285
 *		init_rtc
 *		read_rtc
 *		init_tb_ppc
 *
 *   ORIGINS: 27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/adspace.h>        /* so we can use the io_att() macros    */
#include <sys/rtc.h>		/* for the tod chip layout and defines	*/
#include <sys/types.h>		/* always				*/
#include <sys/seg.h>		/* for the I/O segment register def.	*/
#include <sys/intr.h>		/* for INTMAX def.			*/
#include <sys/machine.h>	/* for machine model macros		*/
#include <sys/syspest.h>	/* for assert macros 			*/
#include <sys/systemcfg.h>	/* for system config structure          */
#include <sys/sys_resource.h>	/* for system resource structure        */
#include <sys/inline.h>		/* for eieio()				*/
#include <sys/vmker.h>		/* for init_tb_ppc		        */
#include <sys/iplcb.h>		/* for init_tb_ppc		        */
#ifdef _RSPC
#include <sys/ioacc.h>		/* for io_map structure			*/
#include <sys/system_rspc.h>	/* for io addresses			*/
#endif

/* GLOBAL VARIABLES */

extern long 	min_ns;    	/* minimum resolution of clock 		*/
extern struct io_map nio_map;	/* native IO mapping structure		*/

/*
 * NAME:  init_dp8570
 *
 * FUNCTION:  Initialize all clocks by:
 * 	Initialize non-volatile time of day chip. 
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from tinit() during system initialization.
 *
 *      It does not page fault.
 *
 * EXTERNAL PROCEDURES CALLED: io_att, io_det
 */
static void
init_dp8570()
{
	volatile register struct rtc	*rtc;
	volatile uchar			trash;

#ifdef _POWER_RS
	if (__power_rs())
		rtc = (struct rtc *)io_att(SYSREG_SEGVAL, RTC_OFFSET);
#endif 
#ifdef _RS6K
	if (__rs6k())
		rtc = (struct rtc *)&sys_resource_ptr->sys_regs.time_of_day[0];
#endif

	/*  Select page 0, register 0  */
	rtc->msr = 0;				IOSPACE_EIEIO();

	/*  
	 * Set the time save bit to latch time of day into time save RAM.
	 * Clear the time save bit to stop latching time.
	 */
	rtc->intr_route  =  IRR_TS|IRR_PFD|IRR_T1R|IRR_T0R|IRR_PRR|IRR_PFR;
						IOSPACE_EIEIO();
	rtc->intr_route &= ~IRR_TS;		IOSPACE_EIEIO();

	/*
	 *  Check for time keeping failure (i.e. Oscillator Fail Flag set
	 *  to 1, clock start/stop bit reset to 0.
	 */
	trash = rtc->periodic & PFR_OSF;	IOSPACE_EIEIO();
	rtc->msr |= MSR_RS;			IOSPACE_EIEIO();
	trash |= (~(rtc->rtm)) & RTM_CSS;	IOSPACE_EIEIO();
	if(trash)  {
		/*
		 *  There was a time keeping failure.  Go through the initial
		 *  power on sequence.
		 */
		register char	*prtc;

		/*
		 *  Crystal select being non-zero fixes known tod problem
		 *  with early chips.
		 */
		rtc->rtm = RTM_IPF;

		/*  Reset registers.  */
		rtc->output_mode = 0;
		rtc->intr_r0     = 0;
		rtc->intr_r1     = 0;
		rtc->msr         = 0;		IOSPACE_EIEIO();
		rtc->timer0      = 0;
		rtc->timer1      = 0;
		rtc->periodic    = 0;

		/*  Route all interrupts to MFO except alarm.  */
		rtc->intr_route =
			(IRR_PFD | IRR_T1R | IRR_T0R | IRR_PRR | IRR_PFR);

		/*  Initialize RAM  */
		rtc->T0LSB = 0;
		rtc->T0MSB = 0;
		rtc->T1LSB = 0;
		rtc->T1MSB = 0;
		rtc->comp_secs  = 0;
		rtc->comp_min   = 0;
		rtc->comp_hour  = 0;
		rtc->comp_dom   = 0;
		rtc->comp_month = 0;
		rtc->comp_dow   = 1;
		rtc->ts_seconds = 0;
		rtc->ts_minutes = 0;
		rtc->ts_hours   = 0;
		rtc->ts_date_month = 0;
		rtc->ts_months  = 0;
		rtc->p0_ram     = 0;
		rtc->p0_ramtest = 0;

		/*  Select page 1.  */
		rtc->msr = MSR_PS;		IOSPACE_EIEIO();

		/*  Initialize page 1 RAM.  */
		prtc = (char *)((int)rtc + 1);
		while(prtc < (char *)((int)rtc + sizeof(struct rtc)))  {
			*prtc++ = 0;
		}
		rtc->msr = 0;			IOSPACE_EIEIO();

		/*  Set the date counters to the Epoch using BCD.  */
		tm.yrs 		= rtc->c_years 		= 0x00;
		tm.jul_100 	= rtc->c_julian_100 	= 0x00;
		tm.jul_dig 	= rtc->c_julian_digits 	= 0x01;
		tm.mths 	= rtc->c_months 	= 0x01;
		tm.dom 		= rtc->c_date_month 	= 0x01;
		tm.hrs 		= rtc->c_hours 		= 0x00;
		tm.mins 	= rtc->c_minutes 	= 0x00;
		tm.no_secs 	= rtc->c_seconds 	= 0x00;
		tm.ms           = rtc->c_millisecs	= 0x00;

		rtc->msr = MSR_RS;		IOSPACE_EIEIO();

		/*  Start tod, power fail interrupt enable, 24 hour mode.  */
		rtc->rtm = (RTM_IPF | RTM_CSS);	IOSPACE_EIEIO();
	}
	else  {

		tm.yrs 		= rtc->c_years;
		tm.jul_100 	= rtc->c_julian_100;
		tm.jul_dig 	= rtc->c_julian_digits;
		tm.mths 	= rtc->ts_months;
		tm.dom 		= rtc->ts_date_month;
		tm.hrs 		= rtc->ts_hours;
		tm.mins 	= rtc->ts_minutes;
		tm.no_secs 	= rtc->ts_seconds;
		tm.ms           = rtc->c_millisecs;

	}
        IO_DET((ulong)rtc);

}

/*
 * NAME: read_rtc
 *
 * FUNCTION: read a byte from the DS1285 real time clock chip 
 *
 * EXECUTION ENVIORNMENT:
 *	called by init_ds1285
 *
 *	caller should be disabled
 *
 * RETURNS: Byte read
 */
static char
read_rtc(reg)
int reg;
{
	volatile char *io_addr;
	char byte;

#ifdef _POWER_RS
	if (__power_rs()) {
		io_addr = io_att(SYSREG_SEGVAL, 0);

		*(io_addr + TOD_INDX_OFF) = reg;
		byte = *(io_addr + TOD_DATA_OFF);

		io_det(io_addr);
	}
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k()) {
		io_addr = (volatile char *)
			&sys_resource_ptr->sys_regs.time_of_day[TOD_INDX_PPC];
		*io_addr = reg;
		eieio();
		io_addr =(volatile char *)
			&sys_resource_ptr->sys_regs.time_of_day[TOD_DATA_PPC];
		byte = *io_addr;
	}
#endif /* _RS6K */

#ifdef _RSPC
	if (__rspc()) {

		volatile struct rspcsio *siop;

		/*
		 * index register can also disable NMI
		 */
		ASSERT(!(RSPC_RTC_NMI & reg));

		siop = iomem_att(&nio_map);

		siop->rtc_index = reg;
		eieio();
		byte = siop->rtc_data;

		iomem_det(siop);
	}
#endif /* _RSPC */
		


	return(byte);
}


/*
 * NAME: date_to_jul
 *
 * FUNCTION: converts month, day of month, and year to julian date
 *
 * NOTES:
 *	Only the yrs mths and dom fields of the tms structure need be
 *	filled out.
 *
 *	The jul_100 and jul_dig will be set
 *
 * EXECUTION ENVIORNMENT:
 *	called only by init_ds1285
 *
 *	Only called once.
 *
 * RETURNS: None
 */
void
date_to_jul(tm)
struct tms *tm;
{
	int jday;
	int month;
	int year;
	int i;
	static char days[12] = {31 /* JAN */, 28 /* FEB */, 31 /* MAR */,
			   30 /* APR */, 31 /* MAY */, 30 /* JUN */,
			   31 /* JUL */, 31 /* AUG */, 30 /* SEP */,
			   31 /* OCT */, 30 /* NOV */, 31 /* DEC */ };

	/* convert BCD format years into a decimal years
	 */
	year = (tm->yrs / 16 ) * 10 + (tm->yrs % 16);

	/* check if the year is a leap year.  Year 0 is 1970 so 2 must
	 * be added to year before checking for leap year.  If it is
	 * a leap year adjust Feb.'s days to 29
	 */
	ASSERT(days[1] == 28);
	if (((year + 2) % 4) == 0)
		days[1] = 29;

	/* convert day of month and day to a julian date
	 */
	jday = (tm->dom / 16) * 10 + (tm->dom % 16);
	month = (tm->mths / 16) * 10 + (tm->mths % 16);
	for (i = 0; i < (month - 1); i++)
		jday += days[i];

	/* store away the julian date in BCD
	 */
	tm->jul_100 = jday / 100;
	jday %= 100;
	tm->jul_dig = (jday / 10) * 16 + (jday % 10);
}

/*
 * NAME:  init_ds1285
 *
 * FUNCTION:  Setup the DS1285 time-of-day chip 
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from tinit() during system initialization.
 *
 *      It does not page fault.
 *
 * RETURNS: None
 */
static void
init_ds1285()
{
	int oldpri;
	int addr;

	/* check if the current contents of the real time clock are valid,
	 * and that the time was not being set when the machine went down
	 */
	if ( (read_rtc(RTC_REGD) & CRD_VRT) &&
		(read_rtc(RTC_REGB) == CRB_24HR) &&
		(read_rtc(RTC_REGA) == CRA_DV1) )
	{

		/* disable to be sure this is atomic.  There
		 * should be no interrupts at this point in
		 * the kernels life, but just to be sure
		 */
		oldpri = i_disable(INTMAX);

		/* Sync time read with transition in seconds.  First wait
		 * for update in progress to be set.  The read time after
		 * it is clear.  This allows tm.ms to be 0
		 */
		while(!(read_rtc(RTC_REGA) & CRA_UIP));
		while(read_rtc(RTC_REGA) & CRA_UIP);

		/* Read date and time from real time clock.  The "year"
		 * may be offset by 0x70.  If the "year" is less that 0x70,
		 * it must be in the next century, so that adjustment is
		 * made.  This code breaks in 2069.
		 */
		tm.yrs = read_rtc(RTC_YEAR);
		if (tm.yrs >= 0x70)
			tm.yrs -= 0x70;
		else
			tm.yrs += 0x30;

		tm.dom = read_rtc(RTC_DOM);
		tm.mths = read_rtc(RTC_MONTH);
		tm.hrs = read_rtc(RTC_HRS);
		tm.mins = read_rtc(RTC_MIN);
		tm.no_secs = read_rtc(RTC_SEC);
		tm.ms = 0;

		i_enable(oldpri);

		/* this chip has no julian date so calculate it
		 */
		date_to_jul(&tm);

	}
	else
	{
		/* set the date counter to the Epoch using BCD
		 */

		/* shut off oscillator
		 */
		write_rtc(RTC_REGA, 0);

		/* Turn on set bit (to set time) and configure to 24 hour
		 * mode to write date
		 */
		write_rtc(RTC_REGB, CRB_SET|CRB_24HR);

		/* zero all the time registers
		 */
		for (addr = RTC_SEC; addr < RTC_REGA; addr++)
			write_rtc(addr, 0);

		/* zero all the ram locations
		 */
		for (addr = RTC_RAMSTART; addr <= RTC_RAMEND; addr++)
			write_rtc(addr, 0);

		/* initialize day of week.  This value is not used by
		 * the system
		 */
		write_rtc(RTC_DAY, 0x01);

		/* initialize year to 0x70 for leapyear calculation
		 * on the RTC chip.
		 */
		write_rtc(RTC_YEAR, 0x70);

		/* set the in memory date
		 */
		tm.yrs = 0x00;
		tm.jul_100 = 0x00;
		tm.jul_dig = 0x01;
		tm.mths = 0x01;
		tm.dom = 0x01;
		tm.hrs = 0x00;
		tm.mins = 0x00;
		tm.no_secs = 0x00;
		tm.ms = 0x00;

		/* set time of day on the clock
		 */
		update_rtc();

		/* enable the oscilator and start the clock.  Register C
		 * is read-only
		 */
		write_rtc(RTC_REGA, CRA_DV1);
		write_rtc(RTC_REGB, CRB_24HR);
	}

}



/*
 * NAME:  init_clock
 *
 * FUNCTION:  Initialize all clocks by:
 * 	Initialize non-volatile time of day chip. 
 * 	Use the time saved in the non-volatile clock to init processor clock.
 * 	Initialize the memory mapped time variables.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from tinit() during system initialization.
 *
 *      It does not page fault.
 *
 * EXTERNAL PROCEDURES CALLED: init_ds1285, init_dp8570
 */
void init_clock()
{

	if (__power_rsc() | __rs6k_up_mca() | __rspc_up_pci())
	{
		init_ds1285();
	}
	else
	{
		init_dp8570();
	}

#ifdef _POWER_PC
	if (__power_pc() && !__power_601())
		init_tb_ppc();
#endif /* _POWER_PC */
		
	/*
	 * Initialize processor chip
	 */
	date_to_secs(&tm);
	time = tod.tv_sec = tm.secs;
	tod.tv_nsec = tm.ms * (NS_PER_SEC/100);
	init_rtc();

	/*
	 * get minimum resolution of clock
	 */
	MIN_NS(min_ns);
#ifdef _POWER_MP
	__iospace_sync(); /* Make it visible */
#endif
}

/*
 * NAME:  init_rtc
 *
 * FUNCTION:  Initialize the RTC of a processor, from the memory mapped 
 *      time variable tod.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called during system initialization.
 *
 *      It does not page fault.
 *
 * EXTERNAL PROCEDURES CALLED: UPDATE_RTC
 */
init_rtc()
{
#ifdef _POWER_PC
	if (__power_pc() && !__power_601())
		/* initialize processor data for ticks/(secs, nanosecs) 
		   conversion */
		PPDA->TB_ref_u = 0xFFFFFFFF; /* no reference available */
#endif /* _POWER_PC */

	UPDATE_RTC(tod);
}
#ifdef _POWER_PC

/*
 * NAME:  init_tb_ppc()
 *
 * FUNCTION:  	Initialize Xint, Xfrac & Xmask for conversion between
 *		Time Base ticks & (secs, nanosecs) on Power_PC:
 *			nanosecs = (tb_ticks / Xfrac) * Xint
 *		The fraction Xint/Xfrac is reduced. 
 *		For optimization of the 64 bits division tb_ticks / Xfrac,
 *		we compute Xfrac < 2^16.
 *		Compute tb_Xmask, that is the mask that allows to have the
 *		result (tb_ticks / Xfrac) * Xint fit in 32 bits:
 *			~Xmask / Xfrac * Xint < 0xFFFFFFFF
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called during system initialization.
 *
 *      It does not page fault.
 *
 */
init_tb_ppc()
{
	uint Xint, Xfrac, Xmask, max, g;
	struct ipl_cb *iplcb_ptr;	/* ros ipl cb pointer */
	struct ipl_directory *dir_ptr;	/* ipl dir pointer */
        struct processor_info *proc_ptr;
	extern uint tb_Xmask;
	uint gcd();

	iplcb_ptr = (struct ipl_cb *)(vmker.iplcbptr);
	dir_ptr = &(iplcb_ptr->s0);
        proc_ptr = (struct processor_info *) ((char *) iplcb_ptr +
					      dir_ptr->processor_info_offset);
	
	/* compute Xint & Xfrac */
	
	Xint = NS_PER_SEC;
	Xfrac = proc_ptr->tbCfreq_HZ;

	/* sanity check */

	assert(Xfrac <= Xint);

	while (1) {
		/* reduce fraction Xint/Xfrac */
		g = gcd(Xint, Xfrac);
		Xint /= g;
		Xfrac /= g;
		if (Xfrac < 0x10000)
			break;
		/* if Xfrac is greater than 2^16, round Xint & Xfrac */
		Xint /= 2;
		Xfrac /= 2;
	}
	_system_configuration.Xint = Xint;
	_system_configuration.Xfrac = Xfrac;

	/* compute Xmask */
	/* Xmask should be such that ~Xmask / Xfrac * Xint < 0xFFFFFFFF */
	
	Xmask = 0xFFFFFFFF;
	max = 0xFFFFFFFF / Xint * Xfrac;
	while ( Xmask > max)
		Xmask >>=1 ;
	tb_Xmask = ~Xmask;
}

/*
 * compute the greatest common divisor of 2 integers
 */

uint gcd(uint a, uint b)
{
	uint x,y,z;

	x=a;
	y=b;
	while (y)
	{
		z=x%y;
		x=y;
		y=z;
	}
	return x;
}
#endif /* _POWER_PC */
