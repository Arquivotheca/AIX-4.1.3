static char sccsid[] = "@(#)55        1.11.4.2  src/bos/kernel/proc/POWER/m_clock.c, sysproc, bos41J, bai15 4/11/95 11:38:37";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: set_alarm
 *		set_time
 *		update_rtc
 *		update_system_time_pwr
 *		write_clock
 *		write_dp8570
 *		write_ds1285
 *		write_rtc
 *
 *   ORIGINS: 27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */
#include <sys/types.h>		/* always needed			*/
#include <sys/adspace.h>        /* for the WRITE_CLOCK macro to work    */
#include <sys/time.h>		/* for the timeval structure		*/
#include <sys/param.h>		/* to define the HZ label		*/
#include <sys/mstsave.h>	/* mstsave area def. for asserting	*/
#include <sys/user.h>		/* the u structure to return errnos	*/
#include <sys/errno.h>		/* define the errno's to be returned	*/
#include <sys/syspest.h>	/* for the ASSERT and assert macros	*/
#include <sys/intr.h>		/* for the serialization stuff		*/
#include <sys/rtc.h>		/* for real time clock related defines	*/
#include <sys/low.h>		/* access the csa for asserts		*/
#include <sys/machine.h>	/* for machine model macros		*/
#include <sys/systemcfg.h>	/* for system config structure          */
#include <sys/sys_resource.h>	/* for system resource structure        */
#include <sys/inline.h>		/* for eieio()				*/
#ifdef _RSPC
#include <sys/ioacc.h>		/* for io_map structure			*/
#include <sys/system_rspc.h>	/* for IO addresses			*/
#endif

#ifdef _RS6K_SMP_MCA
extern int fd_mutex;
extern int pgs_SSGA_lvl;
extern void d_abort_fd();
#endif /* _RS6K_SMP_MCA */

extern void reset_decr(), write_clock();      
extern struct io_map nio_map;

/*
 * NAME:  set_time
 *
 * FUNCTION:  Set the time: processor clock, memory mapped variables, nvram chip. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      It is assumed that interrupts were disabled by the caller.
 *      This routine does not page fault.
 *
 * EXTERNAL PROCEDURES CALLED: None
 *
 * NOTE: A RSC deviation prevents setting rtcl in the range:
 *      	3b9a4a00 <= rtcl < 3b9aca00
 * 	this is 32.768 uS from rapping.  If rtcl is set to this value a check
 * 	stop could result.  To work around this rtcl values that are within
 * 	34 uS of rapping are rounded up or down.  Also the minimium clock 
 * 	resolutionis set to 17 uS.
 */

void
set_time(struct timestruc_t new_time) 
{                                    
        struct timestruc_t      ct;                              
	cpu_t 			my_cpu=CPUID;

	ASSERT(csa->intpri == INTMAX);

#ifdef _POWER_MP
	ASSERT(lock_mine(&tod_lock));
#endif

        if (__power_rsc())                                  
        {                                               
                if ((new_time).tv_nsec >= NS_PER_SEC-17*NS_PER_uS)  
                {                                                  
                        (new_time).tv_sec++;                      
                        (new_time).tv_nsec = 0;                  
                }                                               
                else if ((new_time).tv_nsec >= NS_PER_SEC-34*NS_PER_uS) 
                {                                                     
                        (new_time).tv_nsec = NS_PER_SEC-34*NS_PER_uS;
                }                                                   
        }                                                          
                                                                  
        /*                                                 
         * Set reference time for next time out. It is assumed that    
         * ref_time is greater than the current time, however, the    
         * RTCU and RTCL registers are implemented internally as a   
         * continuous 64 bit register that cannot be disabled.    
         */                                                       
        curtime(&ct);                                            
        while (ntimercmp(TIMER(my_cpu)->ref_time, ct, <))             
                ntimeradd(TIMER(my_cpu)->ref_time, ref_const,     
                          TIMER(my_cpu)->ref_time);              
        ntimersub(TIMER(my_cpu)->ref_time, ct, ct);             
        ntimeradd(new_time, ct, TIMER(my_cpu)->ref_time);      
                                                           
        /*                                                
         * Adjust the timer request blocks.   
         */                                  
        adj_trbs(new_time);                 
                                           
        /*                                
         * write_clock() gets the current time from the real time 
         * clock with curtime().  It then copies this value to the
         * tm.secs and tm.msecs fields.  These values are then   
         * translated to MMddyyhhmmss and the time of day chip is 
         * updated.  We must update the time in the real time clock 
         * before calling write_clock().  Therefore,  order is     
         * important.                                             
         */                                                      
        UPDATE_RTC(new_time);                                   
        if (my_cpu == MP_MASTER){                               
           	write_clock();                               

        	/*                                                       
         	 * Update memory copy of time.                          
         	 */                                                    
        	tod.tv_sec = tm.secs = time = (new_time).tv_sec;      
        	tod.tv_nsec = (new_time).tv_nsec;                    
         }                                                   
                                                            
        /*                                                 
         * Reset the decrementer for the next timer interrupt.   
         */                                                     
        (void)reset_decr();                                    
}


/*
 * NAME:  write_dp8570
 *
 * FUNCTION:  Sets the time of day clock to the time and date contained
 *	      in tm.  
 *
 * EXECUTION ENVIRONMENT:
 *
 *	It is assumed that interrupts were disabled by the caller.
 *      This routine does not page fault. 
 *
 * EXTERNAL PROCEDURES CALLED: None
 */

static void
write_dp8570()
{
	volatile register struct rtc	*rtc;
	
#ifdef _POWER_RS
	if (__power_rs())
		rtc = (struct rtc *)io_att(SYSREG_SEGVAL, RTC_OFFSET);
#endif

#ifdef _RS6K
	if (__rs6k())
		rtc = (struct rtc *)&sys_resource_ptr->sys_regs.time_of_day[0];
#endif

#ifdef _RS6K_SMP_MCA
        /* If diskette is working, kill the ongoing DMA before  */
        /* writing the tod                                      */

        if (pgs_SSGA_lvl == 2 && fd_mutex)
                d_abort_fd();
#endif /* _RS6K_SMP_MCA */

	/*
	 * Turn off page select.
	 * Select the appropriate control registers.
	 * Disable clock.
	 */
	rtc->msr &= ~MSR_PS; 				IOSPACE_EIEIO();
	rtc->msr |=  MSR_RS; 				IOSPACE_EIEIO();
	rtc->rtm &= ~RTM_CSS; 				IOSPACE_EIEIO();

	switch((tm.yrs+2) % 4)  {	/* +2 -> yrs is yrs since 1970 */
		case 0:
			rtc->rtm &= (~(RTM_LY0 | RTM_LY1));
			break;
		case 1:
			rtc->rtm = (rtc->rtm & ~RTM_LY1) | RTM_LY0;
			break;
		case 2:
			rtc->rtm = (rtc->rtm & ~RTM_LY0) | RTM_LY1;
			break;
		case 3:
			rtc->rtm |= RTM_LY1 | RTM_LY0;
			break;
	} 						IOSPACE_EIEIO();
	rtc->c_millisecs 	= tm.ms;
	rtc->c_seconds 		= tm.no_secs;
	rtc->c_minutes 		= tm.mins;
	rtc->c_hours 		= tm.hrs;
	rtc->c_date_month 	= tm.dom;
	rtc->c_months 		= tm.mths;
	rtc->c_years 		= tm.yrs;
	rtc->c_julian_digits 	= tm.jul_dig;
	rtc->c_julian_100 	= tm.jul_100;

	rtc->rtm |= RTM_CSS;	/* start clock */	IOSPACE_EIEIO();

	IO_DET((ulong)rtc);
}


/*
 * NAME: write_rtc
 *
 * FUNCTION: write a byte to DS1285 real time clock chip 
 *
 * EXECUTION ENVIORNMENT:
 *	called by init_ds1285 and write_ds1285
 *
 *	caller should be disabled
 *
 * RETURNS: None
 */
void
write_rtc(reg, value)
int reg;
char value;
{
	volatile char *io_addr;

#ifdef _POWER_RS
	if (__power_rs()) {
		io_addr = io_att(SYSREG_SEGVAL, 0);

		*(io_addr + TOD_INDX_OFF) = reg;
		*(io_addr + TOD_DATA_OFF) = value;

		io_det(io_addr);
	}
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k()) {
		io_addr = (volatile char *)
			&sys_resource_ptr->sys_regs.time_of_day[TOD_INDX_PPC];
		*(io_addr) = reg;
		eieio();
		io_addr = (volatile char *)
			&sys_resource_ptr->sys_regs.time_of_day[TOD_DATA_PPC];
		*(io_addr) = value;
		__iospace_sync();		/* make sure seen */
	}
#endif /* _RS6K */

#ifdef _RSPC
	if (__rspc()) {

		volatile struct rspcsio *siop;

		siop = iomem_att(&nio_map);

		/*
		 * check that high bit (NMI_OK) is off
		 */
		ASSERT(!(RSPC_RTC_NMI & reg));

		siop->rtc_index = reg;
		eieio();
		siop->rtc_data = value;
		__iospace_sync();

		iomem_det((void *)siop);

	}
#endif /* _RSPC */

}


/*
 * NAME update_rtc
 *
 * FUNCTION: update time in DS1285
 *
 * EXECUTION ENVIORNMENT:
 *	called by write_ds1285 and init_ds1285.  must not be
 *	called to update the DP8570.
 *
 * RETURNS:
 *	None
 */
void
update_rtc()
{
	/* write new time.  RTC_YEAR is the actual year of the century.
	 * When the value is greater than 0x30, overflow would occur, so
	 * we check for that here and wrap the year 2000 back to 0.
	 */
	if (tm.yrs + 0x70 < 0xa0)
		write_rtc(RTC_YEAR, tm.yrs + 0x70);
	else
		write_rtc(RTC_YEAR, tm.yrs - 0x30);

	write_rtc(RTC_MONTH, tm.mths);
	write_rtc(RTC_DOM, tm.dom);
	write_rtc(RTC_HRS, tm.hrs);
	write_rtc(RTC_MIN, tm.mins);
	write_rtc(RTC_SEC, tm.no_secs);
}


/*
 * NAME write_ds1285
 *
 * FUNCTION: Update time in DS1285 real time clock chip
 *
 * EXECUTION ENVIORNMENT:
 *	only called by write_clock
 *
 * RETURNS: None
 */
static void
write_ds1285()
{

	/* disable update of clock
	 */
	write_rtc(RTC_REGB, CRB_SET|CRB_24HR);

	update_rtc();

	/* enable update of clock
	 */
	write_rtc(RTC_REGB, CRB_24HR);
}


/*
 * NAME: write_clock
 *
 * FUNCTION: Update time in real time clock
 *
 * EXECUTION ENVIORNMENT:
 *	Caller should be disabled, or non-premptable.  This routine
 *	is not re-entrant
 *
 * RETURNS: None
 */
void
write_clock()
{
	struct   timestruc_t ct;
	
	/*
	 * Get the current time to set the clock.  Need to convert
	 * nanoseconds to hundred seconds.
	 */
	curtime(&ct);
	tm.secs = ct.tv_sec;
	tm.ms = ct.tv_nsec / (NS_PER_SEC/100);  /* nano secs to 100th secs */
	secs_to_date(&tm);

	if (__power_rsc() | __rs6k_up_mca() | __rspc_up_pci())
	{
		write_ds1285();
	}
	else
	{
		write_dp8570();
	}

}


/*
 * NAME:  set_alarm
 *
 * FUNCTION:  Sets an alarm in the real time clock.  The alarm function 
 *	      is only supported by lampasas. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *	It is assumed that interrupts were disabled by the caller.
 *      This routine does not page fault. 
 *
 * EXTERNAL PROCEDURES CALLED: None
 */

void
set_alarm(time_t nsecs)
{
	volatile register struct rtc	*rtc;
	struct tms			ntm;

	/* Only Power RS1 Power RS2 and SMP models implement this
	 */
	if (!__power_set(POWER_RS1|POWER_RS2) && !__rs6k_smp_mca())
		return;

	/* 
	 * Convert the global field tm into time of day structure.
	 * The seconds field is update every 10 millisecs by the 
	 * system timer, but the date fields are not. 
	 */
	secs_to_date(&tm);
	ntm.secs = tm.secs + (ulong)nsecs;
	secs_to_date(&ntm);

	if (__rs6k_smp_mca())
		rtc = (struct rtc *)&sys_resource_ptr->sys_regs.time_of_day[0];
	else
		rtc = (struct rtc *)io_att(SYSREG_SEGVAL, RTC_OFFSET);

	/*
	 * Turn off page select.
	 * Select the appropriate control registers.
	 * Disable clock.
	 */
	rtc->msr &= ~MSR_PS;  				IOSPACE_EIEIO();
	rtc->msr |=  MSR_RS;  				IOSPACE_EIEIO();
	rtc->rtm &= ~RTM_CSS;  				IOSPACE_EIEIO();

	/* timer enable is being set somewhere */
	rtc->intr_r0 = 0;
	/*   
	 * Set alarm enable bit - IC1_ALE
	 * Also indicate the valid comparison fields.
	 */
	rtc->intr_r1 = ( IC1_ALE     | 	/* enable alarm */
			 IC1_DOM_COM | 	/* day of the month */
			 IC1_MO_COM  | 	/* month */
		 	 IC1_HR_COM  | 	/* hour */
			 IC1_MN_COM  | 	/* minutes */
			 IC1_SC_COM ); 	/* seconds */

	/* 
	 * Reset the current time because the oscillators in the two clocks
	 * are different, and the ntm field is based off of the processor
	 * clock.
	 */
	rtc->c_seconds    	= tm.no_secs;
	rtc->c_minutes    	= tm.mins;
	rtc->c_hours      	= tm.hrs;
	rtc->c_date_month 	= tm.dom;
	rtc->c_months     	= tm.mths;
	rtc->c_years      	= tm.yrs;
	rtc->c_julian_digits 	= tm.jul_dig;
	rtc->c_julian_100 	= tm.jul_100;

	/* fill in time to alarm */
	rtc->comp_month	  	= ntm.mths;
	rtc->comp_dom 	  	= ntm.dom;
	rtc->comp_hour 	  	= ntm.hrs;
	rtc->comp_min 	  	= ntm.mins;
	rtc->comp_secs 	  	= ntm.no_secs;

	/* load faster counters with zero */
	rtc->c_millisecs = 0; 				IOSPACE_EIEIO();

	rtc->rtm |=  RTM_CSS; 	/* start clock */	IOSPACE_EIEIO();

	IO_DET((ulong)rtc);
}

/*
 * Update the processor clock on POWER & Power_PC 601.
 * 
 * NOTE: This should only be called from tinit.
 *	An architectual deviation was granted to require 7F80 to be
 *	placed in rtcl before setting time
 */

void
update_system_time_pwr(struct timestruc_t new_time)
{
	mtrtcl(0x7F80);
	mtrtcu((new_time).tv_sec);
	mtrtcl((new_time).tv_nsec);
}
