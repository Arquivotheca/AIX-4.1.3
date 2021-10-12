static char sccsid[] = "@(#)56  1.20.3.26  src/bos/kernel/proc/POWER/int_hproc.c, sysproc, bos41J, 9519A_all 5/2/95 14:59:47";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: ALL_INTS_OFF
 *		check_key
 *		halt_display
 *		halt_display_6k
 *		halt_display_gen
 *		mac_check
 *		mac_check_6k
 *		mac_check_604
 *		mac_check_rs6k_smp_mca
 *		sr_slih
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

#include <sys/types.h>
#include <sys/machine.h>
#include <sys/iocc.h>
#include <sys/adspace.h>
#include <sys/ppda.h>
#include <sys/sysmacros.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/lldebug.h>
#include <sys/dbg_codes.h>
#include <sys/dump.h>
#include <sys/var.h>
#include <sys/buid0.h>
#include <sys/errids.h>
#include <sys/vmker.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/ioacc.h>
#include <sys/time.h>
#include <sys/inline.h>
#ifdef _RS6K_SMP_MCA
#include "../io/machdd/pgs_bump.h"
#endif


extern ulong ipl_cb;			/* ipl control block pointer	*/
extern mfmsr();				/* asm program to read MSR	*/
extern mtmsr();				/* asm program to write MSR	*/
extern call_debugger();		        /* invokes debugger		*/
extern time_t time;			/* system time 			*/

void sr_slih(int flag);

uint nio_buid;				/* BUS ID of native I/O IOCC    */
uint nio_slot;				/* MCA slot on this IOCC	*/
uint nio_posdata;			/* hold pos info 		*/
struct mstsave *crash_csa=0;		/* csa at time of crash         */
int crash_LED;           		/* error code for LEDs at crash */
cpu_t crash_cpuid;           		/* cpu that crashed 		*/

#ifdef _RSPC
uint sr_push_rspc = 0;			/* RSPC machine generated a 	*/
					/* system reset interrupt	*/
#endif /* _RSPC */
#ifdef _RS6K

/*
 * Declare the pointers to the Machine Check Status Register, and the
 * Machine Check Error Address Register.  NOTICE: These must be statically
 * initialized here to keep vmsi() from zeroing their contents.  These
 * pointers are dynamically initialized in init_sys_ranges() BEFORE vmsi()
 * runs.
 */
volatile uint *mcsr_ppc =0; 
volatile uint *mear_ppc =0; 

#define DOUBLE_BIT_ECC_PPC 0x80000000	/* Double bit ECC error (of MCSR) */
#define ADDRESS_ERR_PPC    0x20000000	/* Address error (of MCSR) */

#ifdef _RS6K_SMP_MCA
/*
 * Pegasus has machine dependent locations for these
 */
volatile uint *rsr_addr  = 0;	/* Reset Status Register           */
volatile uint *pksr_addr = 0;	/* Power/Keylock Status Register   */
volatile uint *prcr_addr = 0;	/* Power On Reset Control Register */
volatile uint *spocr_addr = 0;	/* Power Off Control Register      */
#endif /* _RS6K_SMP_MCA */

#endif /* _RS6K */


#define DUMPRET 8

#ifdef _POWER_RS
	/*
	 * POWER Implementation specific defines
	 */
#define IOCC_SELECT_BID		0x82000080
#define MAX_SIM_RSC		0x2000000	/* Largest Salmon Simm Pair*/

/* processor complex address - system registers */
#define BUID0			0x80000000 	   
#define CREG_BASE		0x1040
#define MESR_ADDRESS		0x1090
#define MEAR_ADDRESS		0x1094
#define NVRAM_BASE_PWR		0xA00000
#define ROSLVL			0x38393037
#define CFG_REG_OFFSET		0x10
#define EIO_OFFSET		0x8C

#endif /* _POWER_RS */

#define IOCC_BUS_DELAY		0xE0  

/* ocs-nvram offsets */
#define LED_OFFSET		0x000300
#define LED_STR_OUTPUT_OFFSET	0x000320  
#define LED_STRING_DIAG_ADDR	0x000328
#define LED_STR_OUTPUT_END 	0x000362
#define MC_ERROR_SAVE		0x000368  
#define OCS_COMMAND_INTF	0x00037C  
#define ROS_DISPLAY_LEDS	0x000315
#define ROS_LED_NEXT		0x000316

#define DIAG_LED		0xAAA0
#define TERM_LED		0xFFF0

/*
 * Turn off all interrupts by disabling: 
 * 	FP Exceptions, Alignment Exceptions, Machine Checks,
 *	external interrupts, FP instructions
 */

#define  ALL_INTS_OFF(x, m)						\
	{								\
    		m  = mfmsr();						\
    		m &= ~(MSR_EE | MSR_FP | MSR_ME | MSR_FE);		\
		x  = mtmsr(m);						\
	}

void halt_display();

/*
 * NAME: mac_check_6k
 *
 * FUNCTION: This program writes the machine check log to nvram.
 *
 *	This is the handler for all Power RS models and RS6K models
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called from the model branch table
 *
 *	It cannot pagefault.
 *
 * RETURNS: None
 */

void
mac_check_6k( int srr0,			/* address of failing instr     */
	   int srr1,			/* msr                          */
	   struct mstsave *mst)		/* mst save area                */
{
	ulong buid0addr;
	ulong ioccaddr;
        volatile ulong *real_addr;
	ulong mesr;
	ulong mear;
	ulong data[2];
	int c;
	int mc_code;
	volatile ulong *nvram_ptr;
        struct ipl_cb *iplcb_ptr;               /* ipl cb pointer   */
        struct ipl_directory *dir_ptr;          /* iplcb directeory */
        char    *vpd_ptr;                       /* iplcb VPD pointer */
        char    MCU_level;                      /* level of MCU chips */
        char    sense_valid = TRUE;
	static struct io_map iom = {
		IO_MEM_MAP, 0, 0x1000, REALMEM_BID, 0xFF00100C };


#ifdef _RS6K_UP_MCA
	if (__rs6k_up_mca()) {
		/*
		 * Turn off Pipelining by writing 0 to the arbiter control
		 * register, MCU register accesses have potential hang 
		 * problems with pipeline mode enabled
		 */
		real_addr = iomem_att(&iom);
		*real_addr = 0;
		iomem_det((void *)real_addr);
		__iospace_sync();	/* make sure it's seen */
		
		mesr = *mcsr_ppc;	/* read MCU status register */
		mear = *mear_ppc;	/* read error address register */
		/*
		 * Determine Halt Code
		 */
		if (mesr & DOUBLE_BIT_ECC_PPC) {
			/* double bit ECC error */
			mc_code = 0x20600000;
			/*
			 * The following code is a workaround for a DD1
			 * MCU problem where the MEAR could be invalid. 
			 * If this is a persistent (hard) error, then ROS
			 * will find it on the next reboot and call out
			 * the bad SIMM pair.  If this isn't a hard error,
			 * then the error log entry will be present, but
			 * Error Log analysis won't call out any SIMM
			 */
		        /* Look in the iplcb to determine the MCU chip level.
		         * NOTE: The VPD fields of the IPLCB are not present 
			 * on all implementations.
		         */
			iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
			dir_ptr = &(iplcb_ptr->s0);
			vpd_ptr = (char *) ((char *) iplcb_ptr + 
						dir_ptr->system_vpd_offset);
			MCU_level = vpd_ptr[dir_ptr->system_vpd_size - 3];
			if (MCU_level == 1)
				/*
				 * If this is a DD1, flag the sense data as 
				 * invalid (this will cause bit 7 of the 
				 * sense data to be set)
				 */
				sense_valid = FALSE;
		}
		else if (mesr & ADDRESS_ERR_PPC)
			/* Address Error */
			mc_code = 0x20300000;
		else
			/* Any Other...*/
			mc_code = 0x20700000;

		/*
		 * Convert the faulting address to a SIMM pair number
		 */
		c = encode_sim_up_mca(mear) << 16;

		/*
		 * Set up access to save machine check data
		 */ 
		nvram_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram) + 
								MC_ERROR_SAVE);
	}
#endif /* _RS6K_UP_MCA */

#ifdef _POWER_RS

	if (__power_rs()) {
		buid0addr = (ulong)io_att(BUID0, 0);

		/* Addresses of MESR and MEAR and their contents 
		 * are different for RSC and RS1
		 */
		if (__power_rsc()) {
			/* RSC mesr mear must be read as a string
			 */
			buscpy(buid0addr + MESR_RSC, data, sizeof(data));
			mesr = data[0];
			mear = data[1];
	
			/* Get halt code
			 */
			if (mesr & MESR_AEC)
				mc_code = 0x20300000;
			else if (mesr & MESR_SRC)
				mc_code = 0x20400000;
			else if (mesr & MESR_UEC)
				mc_code = 0x20600000;
			else
				mc_code = 0x20700000;
	
			/* extract the faulting address and convert it to a sim
			 * pair number
			 */
			c = encode_sim((mear & 0x00ffffff) << 3) << 16;
	
		} else {
			mesr = *(volatile ulong *)(buid0addr + MESR);
			mear = *(volatile ulong *)(buid0addr + MEAR);
	
			/* Get halt code
			 */
			if (mesr & MESR_ME)
				mc_code = 0x20000000;
			else if (mesr & MESR_MT)
				mc_code = 0x20100000;
			else if (mesr & MESR_MF)
				mc_code = 0x20200000;
			else if (mesr & MESR_AE)
				mc_code = 0x20300000;
			else if (mesr & MESR_SR)
				mc_code = 0x20400000;
			else if (mesr & MESR_EA)
				mc_code = 0x20500000;
			else if (mesr & MESR_UE)
				mc_code = 0x20600000;
			else if (mesr & MESR_L2)
				mc_code = 0x20800000;
			else
				mc_code = 0x20700000;
	
			c = encode_creg(mear) << 16;
	
		}
		io_det(buid0addr);

		/*
		 * Set up access to save machine check data
		 */ 
		ioccaddr  = (ulong)io_att(IOCC_BID, 0);
		nvram_ptr = (volatile ulong *) (ioccaddr + NVRAM_BASE_PWR +
								MC_ERROR_SAVE);
	}

#endif /* _POWER_RS */

	/* 
	 * Move data into machine check error save.
	 * The first bit indicates that there is machine check data.
	 */
	if (sense_valid)
		*nvram_ptr++ = (srr1 & 0xFFFF) | 0x80000000 | c;
	else
		*nvram_ptr++ = (srr1 & 0xFFFF) | 0x81000000 | c;
	*nvram_ptr++ = time;		      /* modify to move in time */
	*nvram_ptr++ = srr0;
	*nvram_ptr++ = mesr;
	*nvram_ptr = mear;
	
	if (__power_rs())
		io_det(ioccaddr);

	halt_display(mst, mc_code, (int)DBG_MCHECK);

}


/*
 * NAME: mac_check
 *
 * FUNCTION:
 *	Machine check handler should be moved to model branch tabel.
 *	This is a place holder until this can be done.  It just
 *	performs a runtime check and calls the correct function
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This program is called from the first level interrupt handler,
 *	     mc_flih.
 *	It cannot pagefault.
 *
 * RETURNS: None
 */
void
mac_check( int srr0,			/* address of failing instr     */
	   int srr1,			/* msr                          */
	   struct mstsave *mst)		/* mst save area                */
{
	void mac_check_604();

#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca()){
		/*
		 * Pegasus with 604 CPUs.
		 * Note: machine check interrupts on Pegasus with 601 CPUs 
		 *	 result in check-stops.
		 */
		 mac_check_604(srr0, srr1, mst);
		/*NOTREACHED*/
	}
#endif
	/*
	 * Note: the 604 branch for other architectures has not been added yet.
	 */
	if (__power_rs() || __rs6k())
	{
		mac_check_6k(srr0, srr1, mst);
	}
	if (__rspc())
	{
		mac_check_gen(srr0, srr1, mst);
	}
}


#ifdef _RS6K_SMP_MCA

/* SRR1 bit assignment */
#define	SRR1_TEA	(1<<(31-13))	/* TEA pin asserted */
#define	SRR1_MCHK	(1<<(31-12))	/* Machine check pin asserted */
#define	SRR1_DCPE	(1<<(31-10))	/* Data cache parity error */
#define	SRR1_ICPE	(1<<(31-11))	/* Instruction cache parity error */
#define	SRR1_DBPE	(1<<(31-14))	/* Data bus parity error */
#define	SRR1_ABPE	(1<<(31-15))	/* Address bus parity error */

/*
 * NAME: mac_check_604
 *
 * FUNCTION: This program writes the machine check log to nvram.
 *	     This is the machine check handler for 604 CPUs (on Pegasus).
 *
 * EXECUTION ENVIRONMENT:
 *	It cannot pagefault.
 *
 * RETURNS: None
 */

void
mac_check_604(
	int srr0,			/* address of failing instr     */
	int srr1,			/* Save / restore register 1    */
	struct mstsave *mst)		/* mst save area                */
{
	int mc_code;			/* Halt code */
	volatile ulong *nvram_ptr;	/* -> save machine check data */
	void mac_check_rs6k_smp_mca();

#ifdef _POWER_MP
	static int machine_check_flag = 0;

	/*
	 * It is necessary to protect:
	 *	- the machine check log area in NVRAM
	 *	- errsave() -ed data because there is just a single "last error"
	 * "machine_check_flag" will never be cleared...
	 */
	while (!test_and_set(&machine_check_flag, 1))
		;
#endif /* _POWER_MP */

	nvram_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram) + MC_ERROR_SAVE);
	*nvram_ptr++ = (srr1 & 0xFFFF) | 0x84000000;
	*nvram_ptr++ = time;
	*nvram_ptr++ = srr0;
	*nvram_ptr++ = srr1;
	*nvram_ptr = 0;
#ifdef _604_BUP
	{
#ifdef _KDB
		extern int	gimmeabreak();

		kdb_printf("Machine check interrupt on CPU #%d :\n",
			   get_processor_id());
		kdb_printf("   srr0 = 0x%.8X\n", srr0);
		kdb_printf("   srr1 = 0x%.8X\n", srr1);
		if (srr1 & SRR1_TEA)
			kdb_printf("   TEA pin asserted\n");
		else if (srr1 & SRR1_MCHK)
			kdb_printf("   Machine check pin asserted\n");
		else if (srr1 & SRR1_DCPE)
			kdb_printf("   Data cache parity error\n");
		else if (srr1 & SRR1_ICPE)
			kdb_printf("   Instruction cache parity error\n");
		else if (srr1 & SRR1_DBPE)
			kdb_printf("   Data bus parity error\n");
		else if (srr1 & SRR1_ABPE)
			kdb_printf("   Address bus parity error\n");
		kdb_printf("\n UNRECOVERABLE Machine check interrupt\n");
		gimmeabreak();
#endif /* _KDB */
	}
#endif /* _604_BUP */
	mac_check_rs6k_smp_mca(srr1, mst);
	/*NOTREACHED*/
}


/*
 * NAME: mac_check_rs6k_smp_mca
 *
 * FUNCTION: This is the machine check handler specific to RS6K_SMP_MCA models.
 *
 * EXECUTION ENVIRONMENT:
 *	- It cannot pagefault.
 *	- All interrupt should be disabled.
 *
 * RETURNS: This routine does not return.
 *
 * NOTE: The last errsave() record is assumed to be preserved across the reboot.
 */

void
mac_check_rs6k_smp_mca(
	int srr1,			/* Save / restore register 1 */
	struct mstsave *mst)		/* mst save area                */
{
	pgs_bump_msg_t cmd, stat;	/* BUMP command & status buffer */
	extern struct errors errors;	/* For memory parity error logging */
#define	err	errors.perror

	/*
	 * Check to see if it is a multiple memory parity error.
	 * Pegasus enforces a data bus parity errror in case of a multiple
	 * memory parity error. The Machine Check Pin (-MCP) is not driven.
	 * The machine check bit in SRR1 is generated internally by 604.
	 * The BUMP indicates in the response if a multiple memory parity
	 * error has occured or the machine check is due to another reason.
	 */
	if ((srr1 & (SRR1_MCHK | SRR1_DBPE)) == (SRR1_MCHK | SRR1_DBPE)){
		cmd.bump_portid = BUMP_SYSTEM_PORTID;
		cmd.command = READ_MULTI_PERR;
		(void) mdbumpcmd_bwait(&cmd, &stat);
		/*
		 * The error log record has to be packed, it may
		 * not contain reserved fields.
		 * Alignment interrupts should be avoided.
		 */
		err.cmd_status = stat.data.perror.cmd_status;
		err.cmd_detstat = stat.data.perror.cmd_detstat;
		err.cmd_board = stat.data.perror.cmd_board;
		err.cmd_mask[0] = stat.data.perror.cmd_mask[0];
		err.cmd_mask[1] = stat.data.perror.cmd_mask[1];
		err.cmd_addr[0] = * (uchar *) &stat.data.perror.cmd_addr;
		err.cmd_addr[1] = * ((uchar *) &stat.data.perror.cmd_addr + 1);
		err.cmd_addr[2] = * ((uchar *) &stat.data.perror.cmd_addr + 2);
		err.cmd_addr[3] = * ((uchar *) &stat.data.perror.cmd_addr + 3);
		errsave(&errors, sizeof errors);
	}
	halt_display(mst, 0x207, (int)DBG_MCHECK);
	/*NOTREACHED*/
}

#endif /* _RS6K_SMP_MCA */


/*
 * NAME: check_key
 *
 * FUNCTION: This program checks the current console key position with
 *	     the position of the key at ipl time and determines whether
 *	     a dump should be initiated.
 *
#ifdef _RSPC
 *	     Some PC class machines have a reset button that causes
 *	     a system reset interrupt to the processor.  On those
 *	     machines a dump is done without regard to the any
 *	     keyswitch.
 *
#endif
 * EXECUTION ENVIRONMENT:
 *	This program is called from the system reset handler, sr_flih.
 *	Interrupts are disabled.
 *
 * RETURNS: NONE
 */

void
check_key()				/* no input parameters		*/
{
	register caddr_t ioccaddr;	/* iocc effective address	*/
	register ulong *current_pskd;	/* current key position ptr	*/
	register ulong *led_ptr;	/* remove after debug		*/
	register ulong ipl_pskd;	/* ipl key position		*/
	register ulong save_curpskd;	/* save area for current key	*/
	register int dmp_rc;		/* dump program return code	*/
	register int m;			/* MSR save area		*/
	register int x;			/* MSR save area		*/
	volatile struct ipl_cb *iplcb_ptr;/* ipl control block pointer	*/
	volatile struct ipl_info *info_ptr;/* ipl info pointer		*/
	volatile struct ipl_directory *dir_ptr;/* ipl directory pointer	*/

	struct errstruct {
		struct err_rec0 hdr;
		char   		boot[8];
		char   		cur[8];
	} log = { ERRID_SYS_RESET, "SYSPROC", "normal", "normal" };
	char service[8] = "service";
#ifdef _RSPC
	char rspc_auto[8] = "autodmp";
#endif /* _RSPC */

	/*
	 * Read the current Key Position 
	 */

	save_curpskd = get_pksr() & KEY_POS_MASK;

	/*
	 * set up access to IPL control block
	 */
        iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
        dir_ptr = &(iplcb_ptr->s0);
        info_ptr = (struct ipl_info *) ((uint)iplcb_ptr + 
						dir_ptr->ipl_info_offset);

	/* 
	 * Find out what the Key Position was at IPL time
	 */ 
	ipl_pskd = (ulong)(info_ptr->Power_Status_and_keylock_reg);
	ipl_pskd &= KEY_POS_MASK;

	/* compare current key position with that at ipl	        */
#ifdef _RSPC
	if (__rspc() && !crash_csa && 
	    (((mach_model & 0xff) == 0xC0) || ((mach_model & 0xff) == 0xC4))) 
	{
		sr_push_rspc = 1;
		save_curpskd = 0;
		bcopy(rspc_auto, log.cur, sizeof(char) * 8);
		dmp_rc = dmp_do(DMPD_PRIM_HALT);	/* dump system */
	}
	else 
#endif /* _RSPC */
	if (ipl_pskd == KEY_POS_NORMAL)	/* at ipl */
	{
		if (save_curpskd == KEY_POS_SERVICE && !crash_csa)
			dmp_rc = dmp_do(DMPD_PRIM_HALT); /* call dump program */
	}
	else if (ipl_pskd == KEY_POS_SERVICE)     	      /* at ipl */
	{
		/* Log setting of the system reset button. */
		bcopy(service, log.boot, sizeof(char) * 8);

		if (save_curpskd == KEY_POS_NORMAL && !crash_csa) 
			dmp_rc = dmp_do(DMPD_PRIM_HALT); /* call dump program */
	}
	else if (ipl_pskd == KEY_POS_SECURE)	   	      /* at ipl */
	{
		if (save_curpskd == KEY_POS_SERVICE && !crash_csa) 
			dmp_rc = dmp_do(DMPD_PRIM_HALT); /* call dump program */
	}

	/* Log setting of the system reset button. */
	if (save_curpskd == KEY_POS_SERVICE)
		bcopy(service, log.cur, sizeof(char) * 8);

	errsave(&log, sizeof(struct errstruct));
}

/*
 * NAME: halt_display_gen
 *
 * FUNCTION:
 *	This is the generic version of halt_display.  It does no modle
 * specific functions
 *
 * EXECUTION ENVIRONMENT:
 *	This is reached through halt_dispay model branch table entry
 *
 * RETURNS: NONE
 */
void
halt_display_gen(struct mstsave *mst,	/* address of mst save area	*/
	      int errcode,		/* error code to put in halt msg*/
	      int dbg_code)		/* error code to pass to debug  */
{

	/*
	 * disable interupts
	 */
	i_disable(INTMAX);

        /*
         * Save parameter values for easy access in crash dump
         */

        crash_csa = mst;        	/* csa at time of crash         */
        crash_LED = errcode;            /* error code for LEDs at crash */
	crash_cpuid = CPUID;		/* cpu that crashed		*/

	/*
	 * Display c20 in leds to check point call to debugger. 
	 * call_debugger buys a mst, calls debugger, and then 
	 * frees the mst.  An mst is bought because we may be 
	 * running on the faulting mst.
	 */
	write_leds(0xA2000000);		   	/* write C20 to leds */
	(void)call_debugger(mst,dbg_code,0);
	write_leds(0xFFF00000);		   	/* write blanks to leds */

	(void)dmp_do(DMPD_AUTO);        	/* call dump program */

	/*
	 * Check for auto restart.  Assume no hardware power off support,
	 * so attempt warm boot
	 */
	if (v.v_autost)
	{
		sr_slih(1);
	}

	/*
	 * Hard loop
	 */
	while(1)
	{
		write_leds(0x88800000);
		io_delay(uS_PER_SECOND-1);
		write_leds(0xfff00000);
	}
}
	

/*
 * NAME: halt_display_6k
 *
 * FUNCTION: This program is the system halt routine.  It calls
 *	debugger and displays a message via the leds, after exitting 
 * 	from the debugger.
 *
 * EXECUTION ENVIRONMENT:
 *	This program is called from the first level interrupt handlers.
 *	It cannot pagefault.
 *
 * RETURNS: NONE
 */

void
halt_display_6k( struct mstsave *mst,	/* address of mst save area	*/
	      int errcode,		/* error code to put in halt msg*/
	      int dbg_code)		/* error code to pass to debug  */


{
	register caddr_t ioccaddr;	/* iocc effective address	*/
	register caddr_t ioccadr1;	/* iocc effective address       */
	register ushort *pcr_ptr;	/* ptr to power control/reset   */
	register ulong *syshalt_ptr;	/* ptr to led msg display area	*/
	register ushort *term_ptr;	/* ptr to default termination  	*/
	register ushort *end_led_area;	/* ptr to end of led output area*/
	volatile ulong *ocscmd_ptr;	/* pointer to ocs command word	*/
	volatile ulong *led_ptr;	/* pointer to led mirror area	*/
	volatile char *led_flag;	/* software led mode flag	*/
	register int i;			/* index for message vector	*/
	register int m;			/* msr save area		*/
	register int x;			/* msr save area		*/
					/* halt msg for nvram		*/
	static int halt_msg[2] = { 0x88801020, 0x0000A200 }; 

	i_disable(INTMAX);              /* turn off external interrupts */
	ALL_INTS_OFF(x, m);		/* turn off all interrupts	*/

        /*
         * Save parameter values for easy access in crash dump
         */

        crash_csa = mst;        	/* csa at time of crash         */
        crash_LED = errcode;            /* error code for LEDs at crash */
	crash_cpuid = CPUID;		/* cpu that crashed		*/

	/*
	 * Set up a pointer to LEDs and other NVRAM locations depending
	 * on the machine platform
	 */
#ifdef _POWER_RS
	if (__power_rs()) {
		/*
		 * For POWER implementations, set up access via the
		 * IOCC
		 */
		ioccaddr = io_att(IOCC_BID,0);
		led_ptr = (ulong *)(ioccaddr + NVRAM_BASE_PWR + LED_OFFSET);
		syshalt_ptr = (ulong *)(ioccaddr + NVRAM_BASE_PWR + 
							LED_STR_OUTPUT_OFFSET);
		term_ptr = (ushort *)(ioccaddr + NVRAM_BASE_PWR +
							LED_STRING_DIAG_ADDR);
		end_led_area = (ushort *)(ioccaddr + NVRAM_BASE_PWR + 
							LED_STR_OUTPUT_END);
		pcr_ptr   = (ushort *)(ioccaddr + PCRR_ADDRESS + 2);
		ocscmd_ptr   = (ulong *) (ioccaddr + NVRAM_BASE_PWR + 
							OCS_COMMAND_INTF);
	}
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k()) {
		/*
		 * For PowerPC implementations, set up access via the
		 * system resource structure
		 */
		led_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram) + 
								LED_OFFSET);
		syshalt_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram) + 
							LED_STR_OUTPUT_OFFSET);
		term_ptr = (ushort *)((uint)(&sys_resource_ptr->nvram) + 
							LED_STRING_DIAG_ADDR);
		end_led_area = (ushort *)((uint)(&sys_resource_ptr->nvram) + 
							LED_STR_OUTPUT_END);
#ifdef _RS6K_SMP_MCA
		pcr_ptr = (ushort *)prcr_addr;
#else
		pcr_ptr = (ushort *)&sys_resource_ptr->sys_regs.pwr_on_reset_cr;
#endif /* _RS6K_SMP_MCA */
		ocscmd_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram) + 
							OCS_COMMAND_INTF);
	}
#endif /* _POWER_PC */

	/*
	 * Display c20 in leds to check point call to debugger. 
	 * call_debugger buys a mst, calls debugger, and then 
	 * frees the mst.  An mst is bought because we may be 
	 * running on the faulting mst.
	 */
	*led_ptr = 0xA2000000;		   	/* write C20 to leds */
	(void)call_debugger(mst,dbg_code,0);
	*led_ptr = 0xFFF00000;		   	/* write blanks to leds */

	/*
	 * Move the halt code into NVRAM before calling dmp_do().
	 * The dump code should change the a20 to { 0c9, 0c0, ... }
	 */
	halt_msg[1] |= errcode;


	/* move halt message to nvram */
	for (i=0; i<2; i++, syshalt_ptr++)
		*syshalt_ptr = halt_msg[i];

	/* diagnostics can supplement led string. */
	if (*term_ptr == DIAG_LED)
	{
	    for(;*term_ptr != TERM_LED && term_ptr <= end_led_area;term_ptr++);
	}
	*term_ptr = TERM_LED;
	__iospace_sync();

	(void)dmp_do(DMPD_AUTO);        	/* call dump program */

	/* check for auto restart */
	if (v.v_autost) {  	      /* change to v_autost after debug */
		/* 
		 * Reset the machine to a known state by writing to 
		 * the lower 2 bytes of the power control reset register.  
		 * This will turn control over to the OCS and re-ipl the
		 * system.
		 */
		if (__rs6k_up_mca())
			/*
			 * Rainbow boxes have a write-only PCRR
			 */
			*pcr_ptr = 0xFFFF;		
		else
			*pcr_ptr |= 0xFFFF;		
	}

	/* hardware led scrolling does note work correctly on RSC.  ROS
	 * contains the function.  Set ROS flag rather than placing the
	 * machine into OCS mode for RSC
	 */
	if (__power_rsc()) {
		/*
		 * initialize ROS's pointer to next LED to display.
		 * This allows LED display to work even if NVRAM
		 * had been destroyed
		 */
		*(volatile short *)(ioccaddr + NVRAM_BASE_PWR + ROS_LED_NEXT) =
			(uint)term_ptr - (uint)ioccaddr - NVRAM_BASE_PWR;
		led_flag = (volatile char *)(ioccaddr + NVRAM_BASE_PWR + 
							ROS_DISPLAY_LEDS);
		*led_flag = 1;
	} else {
		/* put machine into ocs led display mode */
		*ocscmd_ptr |= 0x01000000;
		__iospace_sync();
	}

	/* 
	 * Write flashing 888-102-mmm-ddd 
	 * NOTE: This uses the IOCC that the Native I/O is on.
	 */
	ioccadr1   = IOCC_ATT(nio_buid,0);
	ocscmd_ptr = (ulong *) (ioccadr1 + IOCC_BUS_DELAY);

	for(;;) {
		*led_ptr = 0x88800000;		   	/* write 888 to leds */
		for (i=0; i<125000; i++)                /* time delay */
			iocc_delay(ocscmd_ptr, 4);  	/* 4 usec delay */
		*led_ptr = 0xFFF00000;		      	/* blank out leds */
		for (i=0; i<125000; i++)   		/* time delay */
			iocc_delay(ocscmd_ptr, 4);  	/* 4 usec delay */
	}

}


/*
 * NAME: halt_display
 *
 * FUNCTION: This function is the system halt routine. It calls the kernal
 *	debugger and displays a message via the leds, after exitting 
 * 	from the debugger.
 *
 * EXECUTION ENVIRONMENT:
 *	This program is called from the first level interrupt handlers.
 *	It cannot pagefault.
 *
 * RETURNS: NONE
 */
void
halt_display( struct mstsave *mst,	/* address of mst save area	*/
	      int errcode,		/* error code to put in halt msg*/
	      int dbg_code)		/* error code to pass to debug  */
{

	/*
	 * This switch needs to be moved to the model sepcific branch
	 * table.  For now do runtime switches
	 */
	if (__power_rs() || __rs6k())
	{
		halt_display_6k(mst, errcode, dbg_code);
	}

	if (__rspc())
	{
		halt_display_gen(mst, errcode, dbg_code);
	}

}

/*
 * NAME: sr_slih
 *
 * FUNCTION: Second Level Handler for System Reset
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This program is called from the first level interrupt handler,
 *	     sr_slih.
 *	It cannot pagefault.
 *
 * RETURNS: None
 */

void
sr_slih(int	flag)		/* flag tells us if from reboot() */ 
{
        struct ipl_cb *iplcb_ptr;               /* ipl cb pointer   */
        struct ipl_info *info_ptr;              /* ipl info pointer */
        struct ros_entry_table  *ros_ptr;       /* ros entry pointer */
        struct ipl_directory *dir_ptr;          /* iplcb directeory */
	volatile ulong *ocscmd_ptr;	/* pointer to ocs command word	*/
	uint	roslvl;				/* ros things */
	uint	ioccaddr;			/* access to IOCC */
	uint	load_sr15 = 0;			/* value to load into sr15 */

	/*
	 * When we enter, translation is ON
	 */

	if (!flag) 
		/*
		 * We weren't called from reboot(), so
		 * Call check_key() to see if a dump should be taken
		 */
		check_key();

	/*
	 * set up access to IPL control block
	 */
        iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
        dir_ptr = &(iplcb_ptr->s0);
        info_ptr = (struct ipl_info *) ((uint)iplcb_ptr + 
						dir_ptr->ipl_info_offset);
	ros_ptr = (struct ros_entry_table *)(info_ptr->ros_entry_table_ptr);


#ifdef _POWER_RS  
	if (__power_rs()) {
		/*
		 * For POWER Architecture boxes....
		 */
		/*
		 * Read the ROS level month and year
		 */
		roslvl = (uint)(*(uint *)info_ptr->ipl_ros_timestamp);	
		/*
		 * Compare with ROS level of the last version that did not 
		 * support Warm IPL.  If not supported, crash.
		 * Warm ipl is supported if the ROS level is after 8907 (yymm).
		 * This check may be removed after all machines are a ROS 
		 * LEVEL > 8907.
		 */
		assert(roslvl > ROSLVL);

		/*
	 	 * Quiesce IOCC0 (buid 0x20), by disabling IOCC 
		 * and issuing End of Interrupt Command
		 */
		ioccaddr   = (uint)io_att(IOCC_SELECT_BID,IO_IOCC);
		*((uint *)(ioccaddr + CFG_REG_OFFSET)) = 0;
		*((uint *)(ioccaddr + EIO_OFFSET)) = 0;
		io_det(ioccaddr);
	
	}
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k()) {
#ifdef _RS6K_SMP_MCA
		/* On PEGASUS, reboot is performed by BUMP */
		if (__rs6k_smp_mca())
			mdreboot();	/* no return */
#endif /* _RS6K_SMP_MCA */
		/*
		 * For Rainbow boxes
		 */
		if(__rs6k_up_mca()) {
			/*
			 * if this is the 601 box, then determine if ROS
			 * step mode is activated (bit 7 of NVRAM offset
			 * 0x37C), and write the SRC accordingly.
			 */
			ocscmd_ptr = (ulong *)((uint)(&sys_resource_ptr->nvram)
							+ OCS_COMMAND_INTF);
			if (!(*ocscmd_ptr & 0x01000000)) 
				/*
				 * if not activated, write a 1 to the 
				 * System Reset Register 
				 */
				sys_resource_ptr->sys_regs.reset_status = 0x1;
			__iospace_sync();
		}
	}
#endif /* _RS6K */

#ifdef _RSPC  
	if (__rspc()) {
		/* For RSPC class boxes
		 *
		 * Prior to jumping off into ROS stop all ISA DMA and
		 * disable any PCI device.  This must be done so ROS
		 * does not have to deal with DMAs writing over uncompressed
		 * ROS areas.
		 */
		disable_io();

		if (__power_601())
			/*
			 * For 601 based RSPC's, we must setup segreg 15
			 * with the 601's 0x7F segment so ROM cycles won't
			 * be cached when fetched
			 */
			load_sr15 = 0x87F0000F;

		if (__power_603() || __power_604()) {
			/*
			 * For 603 and 604 based RSPC's, we must disable
			 * the instruction and data caches prior to 
			 * branching to ROS, again so ROM cycles won't be
			 * cached when fetched
			 */
			{	ulong	hid0;
			
				hid0 = mf_603_hid0();	/* read hid0 */
				hid0 |= 0x00000C00; 	/* Invalidate I/D */
				mt_603_hid0(hid0);	/* write hid0 */
				__iospace_sync();
				isync();
				hid0 &= 0xFFFF33FF; 	/* disable I/D cache*/
				mt_603_hid0(hid0);	/* write hid0 */
				__iospace_sync();
				isync();
			}
		}
			
	}
#endif /* _RSPC */


	/*
	 * Call Warm IPL, passing the address of the warm IPL field of
	 * the ROS table entry structure, and the value to load into SR15.  
	 * wipl() then loads the ROS entry point from this address after 
	 * switching to real mode
	 */
	wipl((uint)&ros_ptr->warm_ipl, load_sr15);
}
