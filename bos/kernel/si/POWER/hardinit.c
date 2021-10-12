static char sccsid[] = "@(#)51        1.26.2.8  src/bos/kernel/si/POWER/hardinit.c, syssi, bos41J, 9522A_all 5/30/95 18:13:36";
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:	hardinit, init_config, init_branch, si_cflush, si_bcopy, 
 *	si_halt, init_int_vectors, init_mach_ranges, init_system_config, 
 *	init_overlay, init_branch
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 *
 * NOTES:
 *
 * This program is called from main with interrupts disabled.  It runs
 * in a limited runtime environment.  Care must be taken when modifying
 * this code.  The following could be problems:
 *
 *	Calling machine dependent services before they are set up (cache op)
 *
 *	Generating interrupts/exceptions before flihs are initialize (alignment)
 *
 *	Doing arithmetic before overlays are in place (multiply/divide)
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/low.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/nio.h>
#include <sys/vmker.h>
#include <sys/machine.h>
#include <sys/iocc.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/bootsyms.h>
#include <sys/ppda.h>
#include <sys/systemcfg.h>
#include <sys/overlay.h>
#include <sys/syspest.h>
#include <sys/sys_resource.h>
#include <sys/processor.h>
#include <sys/mstsave.h>
#ifdef _RSPC
#include <sys/rheap.h>
#include <sys/system_rspc.h>
#endif /* _RSPC */
#ifdef _POWER_MP
#include <sys/var.h>
#endif /* _POWER_MP */
#ifdef _RS6K_SMP_MCA
#include <sys/mdio.h>
#include "pegasus.h"
#endif /* _RS6K_SMP_MCA */

#include "branch.h"
#include <sys/lldebug.h>

/* processor_present states; should be in iplcb.h */
#define PROC_DISABLED		-2
#define PROC_FAILED		-1
#define PROC_ABSENT		0
#define PROC_RUNNING_AIX	1
#define PROC_LOOPING		2
#define PROC_IN_RESET		3

/*
 * DIR_EXISTS checks the for the existance of a directory entry
 * 	iplcb - pointer to the IPL control block
 *	mem - member to be checked
 */
#define DIR_EXISTS(iplcb,mem) \
	( ((uint)(&((IPL_DIRECTORY_PTR)(0))->mem)) < \
	(iplcb->s0.ipl_info_offset - sizeof(iplcb->gpr_save_area)) )

/*
 * offset of implementation field in the IPLCB SYSTEM_INFO structure
 */
#define SYSINFO_IMPL ((uint)(&(((SYSTEM_INFO_PTR)(0))->implementation))+4)

extern int dbg_avail;
extern int key_value;

#ifdef _POWER_MP
extern int instr_avail;
#endif /* _POWER_MP */

/* Following are entry point declarations for the individual
 * subroutines which actually do the work of initialization.
 */
extern	void	init_config();
extern	struct mstsave si_save;
extern  int	ipl_cb;
extern  int	ipl_cb_ext;
extern struct ppda ppda[];
extern struct ppda *proc_arr_addr;
extern int	soft_int_model;
extern struct intr intr_soft_dummy;

#define BA_MASK 0x48000002		/* branch absolute opcode */
#define NO_OP	0x60000000		/* PPC nop opcode */
#define BLR_OP	0x4e800020		/* blr opcode */
#define FLIH_PROLOG_SIZE 16		/* size of flih prolog sequence */
#define FLIH_BA_OFFSET 3		/* instruction offset to BA  */
					/* in flih prolog sequence   */
#define Ks 0x40000000			/* Power K bit, PPC Ks bit */
#define Kp 0x20000000			/* Power S bit, PPC Kp bit */

/*
 * Defines for model types.  This is the low two nibbles from the model
 * word in the IPL control block.
 */
#define PEGASUS_MODEL_LOW	0xA0
#define PEGASUS_MODEL_HI	0xBF
#define RAINBOW_3_MODEL		0x46
#define RAINBOW_3P_MODEL	0x49
#define RAINBOW_4_MODEL		0x42
#define RAINBOW_5_MODEL		0x48
#define SANDAL_FOOT_MODEL	0x4d

/*
 * Copies of the various variables that are patched by the tool that
 * creates the IPL image.  These variables are copied from their
 * lowcore counterparts early in system initialization.  Initialized
 * to zero to place in data section.
 *
 */
int header_offset= 0;		/* offset in IPL file to start of hdr */
int ram_disk_start = 0;		/* pointer to RAM disk in IPL file */
int ram_disk_end = 0;		/* pointer to RAM disk end + 1 */
int base_conf_start = 0;	/* pointer to base config area in IPL image */
int base_conf_end = 0;		/* pointer to base config end */
int base_conf_disk = 0;		/* disk address of base config area */

#ifdef _POWER_PC

#include "dma_hw_ppc.h"

int	dec_vec_hold = 0;		/* Place for decrementer vector */
#define	DEC_RFI	0x4c000064		/* rfi instruction */

extern  uint    TCE_space;              /* base address of TCE space */
extern volatile uint *mcsr_ppc;         /* pointer to MCSR */
extern volatile uint *mear_ppc;         /* pointer to MEAR */
extern struct io_map    nio_map;

#define SYS_RES_EADD(real_add) \
	(uint *)((uint)(sys_resource_ptr) + ((uint)real_add & ~0xFF000000))

#endif /* _POWER_PC */

/*
 * NAME: si_halt
 *
 * FUNCTION: SI equivalent of halt_display.  Simply loops forever writing
 *	the LEDs.
 *
 * EXECUTION ENVIRONMENT:
 *	called from init_sys_ranges() 
 *
 * RETURNS: None
 */
void
si_halt()
{
	for (;;) write_leds(0x91100000);
}

/*
 * NAME: proc_index
 *
 * FUNCTION: return processor branch table index for current implementation
 *
 * RETURNS: index 0 - (MAX_PROC_INDEX-1)
 */
int
proc_index()
{
	if (__power_rs1())
		return(RS1_INDEX);
	else if (__power_rsc())
		return(RSC_INDEX);
	else if (__power_rs2())
		return(RS2_INDEX);
	else if (__power_601())
		return(R601_INDEX);
	else if (__power_603())
		return(R603_INDEX);
	else if (__power_604())
		return(R604_INDEX);
	si_halt();
}

/*
 * NAME: model_index
 *
 * FUNCTION: returns model branch table index for current implementation
 *
 * RETURNS: index 0 - (MAX_MODEL_INDEX - 1)
 */
int
model_index()
{
	if (__power_rs1())
		return(MOD_RS1_INDEX);
	else if (__power_rsc())
		return(MOD_RSC_INDEX);
	else if (__power_rs2())
		return(MOD_RS2_INDEX);
	else if (__rs6k_up_mca() || __rs6k_smp_mca())
		return(MOD_RB_INDEX);
	else if (__rspc_up_pci())
		return(MOD_SF_INDEX);
	else
		si_halt();
}

/*
 * NAME: si_bcopy
 *
 * FUNCTION: si version of bcopy.  This version does the copy a byte at
 *	a time to avoid alignment interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *	called from init_overlays() and init_inv_vectors()
 *
 * RETURNS: None
 */
void
si_bcopy(
	volatile caddr_t source,
	volatile caddr_t dest,
	size_t count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		*dest = *source;
		dest++;
		source++;
	}
}

/*
 * NAME: si_cflush
 *
 * FUNCTION:
 *	Determines the correct cache flush service to call with a run-time
 *	check.  Calls correct machine dependent function to perform
 *	the vm_cflush service
 *
 * EXECUTION ENVIRONMENT:
 *	Called only during early system initialization
 *
 * RETURNS:
 *	None
 */
void
si_cflush(
	void *addr,			/* effective address to flush */
	int size)			/* number of bytes to flush */
{
	if (__power_rs())
		vm_cflush_pwr(addr, size);
	else if (__power_pc())
		vm_cflush_ppc_splt(addr, size);
	else
		si_halt();
}



/*
 * NAME: init_int_vectors
 *
 * FUNCTION:
 *	determine machine type and initialize interrupt vectors.
 * For each interrupt vector initialized a architecture dependent
 * prolog sequence is copied to the interrupt vector.  Vectors
 * that are not initialized are left at 0 (invalid opcode).
 *
 * Both the proccessor and model tables are serviced.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from hardinit() with interrupts disabled and xlate off. 
 *
 * RETURNS: None
 */
void
init_int_vectors()
{
	int i;
	extern int flih_prolog_pwr, flih_prolog_ppc;
	int flih_addr;

	/*
	 * First do processor interrupt vector table
	 */
	for (i = 0; i < sizeof(flih_data)/sizeof(flih_data[0]); i++)
	{
		/* get address of flih for implementation
		 */
		flih_addr = (int)flih_data[i].proc_addr[proc_index()];

		/* a NULL address means the vector is not used on this
		 * implementation
		 */
		if ((int *)flih_addr == NULL)
			continue;

#if defined(_KDB)
		/*
		 * PR flih and RUN MODE flih are connected to KDB
		 */
		if (__kdb() && flih_data[i].vector == (int *)0x700)
			continue;
		if (__kdb() && flih_data[i].vector == (int *)0x2000)
			continue;
#endif /* _KDB */
		/* copy the architecture specific flih prolog sequence
		 * to the interrupt vector
		 */
#ifdef _POWER_RS
		if (__power_rs())
		{
			si_bcopy(&flih_prolog_pwr, flih_data[i].vector,
				FLIH_PROLOG_SIZE);
		}
#endif /* _POWER_RS */

#ifdef _POWER_PC
		if (__power_pc())
		{
			si_bcopy(&flih_prolog_ppc, flih_data[i].vector,
				FLIH_PROLOG_SIZE);
		}
#endif /* _POWER_PC */

		/* patch in a branch absolute to the correct flih
		 */
		*(flih_data[i].vector + FLIH_BA_OFFSET) = BA_MASK | flih_addr;
		si_cflush(flih_data[i].vector, FLIH_PROLOG_SIZE);
	}

	/*
	 * Do the model dependent interrupt vector table
	 */
	for (i = 0; i < sizeof(model_flih_data)/sizeof(model_flih_data[0]); i++)
	{
		/* get address of flih for implementation
		 */
		flih_addr = (int)model_flih_data[i].model_addr[model_index()];

		/* a NULL address means the vector is not used on this
		 * implementation
		 */
		if ((int *)flih_addr == NULL)
			continue;

		/* copy the architecture specific flih prolog sequence
		 * to the interrupt vector
		 */
#ifdef _POWER_RS
		if (__power_rs())
		{
			si_bcopy(&flih_prolog_pwr, model_flih_data[i].vector,
				FLIH_PROLOG_SIZE);
		}
#endif /* _POWER_RS */

#ifdef _POWER_PC
		if (__power_pc())
		{
			si_bcopy(&flih_prolog_ppc, model_flih_data[i].vector,
				FLIH_PROLOG_SIZE);
		}
#endif /* _POWER_PC */

		/* patch in a branch absolute to the correct flih
		 */
		*(model_flih_data[i].vector + FLIH_BA_OFFSET) = 
							BA_MASK | flih_addr;
		si_cflush(model_flih_data[i].vector, FLIH_PROLOG_SIZE);
	}

#ifdef _POWER_PC
	if (__power_pc())
	{
		/* Set the decrementer flih interrupt vector to a rfi
		 * When timer initialization is complete and a decrementer
		 * interrupt can be handled the vector will be restored.
		 */
		dec_vec_hold = *(int *)(0x900);
		*(int *)(0x900) = DEC_RFI;
		si_cflush(0x900, sizeof(int));
	}
#endif /* _POWER_PC */
}

/*
 * NAME: init_mach_ranges
 *
 * FUNCTION:
 *	determine the machine type and initialize
 * ranges to be freed or pinned by vmsi()
 *
 * EXECUTION ENVIRONMENT:
 *	Called from hardinit() early in system initialization.  Runs
 * with xlate off, and all interrupts disabled.
 *
 * NOTES:
 *	The address of ranges is determined by examining labels that are
 * placed between the major object sections at kernel bind time.  vmsi()
 * needs to know the text range(s) to free.  It also must pin the machine
 * dependent text and common memory for the platform being executed on.
 * 	All of the ranges are placed into the appropriate vmintervals
 * structure for handling by vmsi.  The limit on the number of intervals
 * per type is set by VMINT_MAX in vmker.h.  Recall that memory holes
 * also reside in the vmintervals before adjusting VMINT_MAX downward.
 * No Ranges within any single type may overlap.  The way rounding is done
 * here, ranges are stored [start,end) thus the end page is not considered
 * to be in the range.  An important exception, the release ranges can
 * have any of start<end, start==end, or start>end.  In the latter two
 * cases, the release range is logically empty (has no pages).
 *	There are assumptions made in vmsi about the release ranges.  All
 * VMINT_RELMEM ranges must be included in the VMINT_FIXMEM ranges as well.
 * This prevents allocating backing storage for them, which would make them
 * harder to release.  Each RELMEM range must be in the Kernel Segment,
 * loaded by the ROS, in V = R memory.
 *	Notice that VMINT_RELMEM ranges round the opposite direction.  In
 * [start,end) nomenclature, start is rounded up to the first page safe to
 * free.  The end is rounded down, to the first page which must not be freed.
 *
 *	+-----------------------+ 0
 *	|   Low Kernel Memory   |
 *	|.......................|
 *	|                       |
 *	|      PIN OBJ          |
 *	|                       |
 *	+-----------------------+ pin_obj_end
 *	|      PPC OBJ          |
 *	+-----------------------+ ppc_obj_end
 *	|      PWR OBJ          |
 *	+-----------------------+ pwr_obj_end
 *	|      DBG OBJ          |
 *	+-----------------------+ dbg_obj_end
 *	|	.    .		|
 *		.    .
 *	|	.    .		|
 *      +-----------------------+
 *      |			|
 *      |      PIN COM		|
 *	|                       |
 *	+-----------------------+ pin_com_end
 *	|      PPC COM          |
 *	+-----------------------+ ppc_com_end
 *	|      PWR COM          |
 *	+-----------------------+ pwr_com_end
 *	|      DBG COM          |
 *	+-----------------------+ dbg_com_end
 *
 * RETURNS: None
 */
void
init_mach_ranges()
{
#define	ROUND_DN(base)	( ( (uint)base                ) >> PGSHIFT )
#define	ROUND_UP(base)	( ( (uint)base + PAGESIZE - 2 ) >> PGSHIFT )

	extern int pin_obj_end, ppc_obj_end, pwr_obj_end;
	extern int pin_com_end, ppc_com_end, pwr_com_end;

	int		 interval;

	/* FIXMEM ranges: PIN OBJ, PPC OBJ, PWR OBJ, TOC
	 */
	interval = vmint_num(VMINT_FIXMEM);

	vmint_start(VMINT_FIXMEM,interval) = ROUND_DN(&pin_obj_start);
	vmint_end(VMINT_FIXMEM,interval)   = ROUND_UP(&pin_obj_end);
	interval += 1;

	vmint_start(VMINT_FIXMEM,interval) = ROUND_DN(&pin_obj_end);
	vmint_end(VMINT_FIXMEM,interval)   = ROUND_UP(&ppc_obj_end);
	interval += 1;

	vmint_start(VMINT_FIXMEM,interval) = ROUND_DN(&ppc_obj_end);
	vmint_end(VMINT_FIXMEM,interval)   = ROUND_UP(&pwr_obj_end);
	interval += 1;

	vmint_start(VMINT_FIXMEM,interval) = ROUND_DN(g_toc);
	vmint_end(VMINT_FIXMEM,interval)   = ROUND_UP((endtoc+1));
	interval += 1;

	vmint_num(VMINT_FIXMEM)		   = interval;

	/* FIXCOM ranges: Only PIN COM is platform independent
	 */
	interval = vmint_num(VMINT_FIXCOM);
	vmint_start(VMINT_FIXCOM,interval) = ROUND_DN(pin_com_start);
	vmint_end(VMINT_FIXCOM,interval)   = ROUND_UP(&pin_com_end);
	vmint_num(VMINT_FIXCOM)		  += 1;

	/* handle the platform-specific ranges.
	 */

#ifdef _POWER_RS
	if (__power_rs())
	{
		/* mark Power COM to be pinned
		 */
		interval = vmint_num(VMINT_FIXCOM);
		vmint_start(VMINT_FIXCOM,interval) = ROUND_DN(&ppc_com_end);
		vmint_end(VMINT_FIXCOM,interval)   = ROUND_UP(&pwr_com_end);
		vmint_num(VMINT_FIXCOM)		  += 1;

		/* mark PPC text (OBJ) to be freed
		 */
		interval = vmint_num(VMINT_RELMEM);
		vmint_start(VMINT_RELMEM,interval) = ROUND_UP(&pin_obj_end);
		vmint_end(VMINT_RELMEM,interval)   = ROUND_DN(&ppc_obj_end);
		vmint_num(VMINT_RELMEM)		  += 1;
	}
#endif /* _POWER_RS */

#ifdef _POWER_PC
	if (__power_pc())
	{
		/* mark PPC COM to be pinned
		 */
		interval = vmint_num(VMINT_FIXCOM);
		vmint_start(VMINT_FIXCOM,interval) = ROUND_DN(&pin_com_end);
		vmint_end(VMINT_FIXCOM,interval)   = ROUND_UP(&ppc_com_end);
		vmint_num(VMINT_FIXCOM)		  += 1;

		/* mark Power (OBJ) text to be freed
		 */
		interval = vmint_num(VMINT_RELMEM);
		vmint_start(VMINT_RELMEM,interval) = ROUND_UP(&ppc_obj_end);
		vmint_end(VMINT_RELMEM,interval)   = ROUND_DN(&pwr_obj_end);
		vmint_num(VMINT_RELMEM)		  += 1;
	}
#endif /* _POWER_PC */

}

/*
 * NAME: init_system_config
 *
 * FUNCTION: initialize system configuration structure for all platforms
 *
 * EXECUTION ENVIRONMENT:
 *	Called from hard_init() early in system initialization.  Runs
 * with xlate off, and all interrupts disabled.
 *
 * NOTES:
 *	All new models have ROS initialize the _system_configuration
 * information.  For old models this code must determine the correct
 * parameters to use.
 *
 *	Set this to zero for now.  sysml_init() will correct this variable.
 * This is done sine multiplication and division is required to set this
 * up on some platforms.
 *
 * 	Using a multiply or divide in this routine may prevent the
 * system from booting (milicode is not overlayed yet)
 *
 * RETURNS: None
 */
void
init_system_config()
{
	struct ipl_cb	*iplcb_ptr;		/* ros ipl cb pointer */
	struct ipl_info	*info_ptr;		/* info pointer */
	struct ipl_directory *dir_ptr;		/* ipl dir pointer */
        struct processor_info *tmp_ptr,*proc_ptr; /* processor info pointer */
        struct processor_info *first_proc_ptr=0; /* first present processor */
	struct system_info *sysinfo_ptr;	/* pointer to system info */
        int     i,ncpus=0,ncpus_cfg=0;
	int	model_id;			/* model code */

	/* get addressability to iplinfo structure, and store away the
	 * model information
	 */
	iplcb_ptr = (struct ipl_cb *)ipl_cb;
	dir_ptr = &(iplcb_ptr->s0);
	info_ptr = (struct ipl_info *)((char *) iplcb_ptr +
		dir_ptr->ipl_info_offset);

	mach_model = info_ptr->model;
	/*
	 * processor_info does not exist on all IPL controll blocks.
	 * check if it exists before attempting to use it
	 */
	if (DIR_EXISTS(iplcb_ptr, processor_info_offset))
	{
	        proc_ptr = (struct processor_info *)((char *) iplcb_ptr +
                			dir_ptr->processor_info_offset);
#ifdef _POWER_MP                
		tmp_ptr = proc_ptr;
		for(i=0;i<proc_ptr->num_of_structs;i++) {
		  if ( (tmp_ptr->processor_present == PROC_RUNNING_AIX) && 
		      (first_proc_ptr == NULL) )
		    first_proc_ptr = tmp_ptr;
		  tmp_ptr = (struct processor_info *)((uint)tmp_ptr +
						tmp_ptr->struct_size);
		}
#endif
		if (first_proc_ptr == NULL) first_proc_ptr = proc_ptr;
	}
	else
	{
		proc_ptr = NULL;
	}

	/* setup architecture and implementation fields of system config
	 * structure so that run time macros can be used.  See rosinfo.h
	 * to see where this numbers are from
	 */
        if (mach_model & 0x08000000)
        {
                _system_configuration.architecture = POWER_PC;
                _system_configuration.implementation = 
		  		first_proc_ptr->implementation;
        }
        else if (mach_model & 0x02000000)
	{
		_system_configuration.architecture = POWER_RS;
		_system_configuration.implementation = POWER_RSC;
		_system_configuration.version = PV_RSC;
	}
	else if (mach_model & 0x04000000)
	{
		_system_configuration.architecture = POWER_RS;
		/*
		 * This can be removed after shipping.  Its for
		 * backlevel ROS
		 */
		_system_configuration.implementation = POWER_RS2;
		_system_configuration.version = PV_RS2;
	}
	else
	{
		_system_configuration.architecture = POWER_RS;
		_system_configuration.implementation = POWER_RS1;
		_system_configuration.version = PV_RS1;
	}

	/*
	 * initialize model architecture and implementation fields
	 */
	if (__power_rs())
	{
		/*
		 * model architecture and implementation fields are
		 * not defined for these machines
		 */
		_system_configuration.model_arch = 0;
		_system_configuration.model_impl = 0;
	}
	else
	{
		model_id = (mach_model & 0x000000FF);
		switch(model_id)
		{
#ifdef _RS6K_UP_MCA
			case RAINBOW_3_MODEL:
			case RAINBOW_3P_MODEL:
			case RAINBOW_4_MODEL:
			case RAINBOW_5_MODEL:
				_system_configuration.model_arch = RS6K;
				_system_configuration.model_impl = RS6K_UP_MCA;
				break;
#endif /* _RS6K_UP_MCA */
#ifdef _RSPC_UP_PCI
			case SANDAL_FOOT_MODEL:
				_system_configuration.model_arch = RSPC;
				_system_configuration.model_impl = RSPC_UP_PCI;
				break;
#endif /* _RS6K_UP_PCI */
			default:
#ifdef _RS6K_SMP_MCA
				if (model_id >= PEGASUS_MODEL_LOW &&
					model_id <= PEGASUS_MODEL_HI)
				{
					_system_configuration.model_arch = RS6K;
					_system_configuration.model_impl =
								RS6K_SMP_MCA;
				}
				else
#endif /* _RS6K_SMP_MCA */
				{
				/*
				 * check system info structure for model
				 * and architecture type
				 */
				if (!DIR_EXISTS(iplcb_ptr, system_info_offset)||
				  (SYSINFO_IMPL > dir_ptr->system_info_size))
					si_halt();

				sysinfo_ptr = (struct system_info *)(
					(uint)iplcb_ptr +
					dir_ptr->system_info_offset);

				_system_configuration.model_arch =
						sysinfo_ptr->architecture;
				_system_configuration.model_impl =
						sysinfo_ptr->implementation;
				}
		}
	}

	/*
	 * There is currently no support (software) for aliasing
	 * on any machine
	 */ 
	_system_configuration.virt_alias = 0;

#if defined(_POWER_RS1) | defined(_POWER_RSC) | defined(_POWER_RS2)
	/*
	 * For the RS1 and RSC machines ROS do not pass a system
	 * configuration structure.  The structure must be manufactured
	 * here.  Also some early levels of RS2 ROS do not support this.
	 */
	if (proc_ptr == NULL)
	{
		assert(__power_set(POWER_RS1|POWER_RSC|POWER_RS2));
		ncpus_cfg = 1;
		_system_configuration.width = 32;
#ifndef _SLICER
		_system_configuration.ncpus = 1;
#else /* _SLICER */
		/* Slicer has EIGHT cpus--we need to fake the system out here */
		_system_configuration.ncpus = MAXCPU;
		number_of_cpus = MAXCPU; /* Slicer never calls init_bs_procs */
#endif /* _SLICER */
		_system_configuration.L2_cache_size = 0;
		_system_configuration.L2_cache_asc = 0;
		_system_configuration.icache_block =
			_system_configuration.icache_line = ic_line_size();
		_system_configuration.dcache_block =
			_system_configuration.dcache_line = dc_line_size();
		_system_configuration.priv_lck_cnt = 0;
		_system_configuration.prob_lck_cnt = 0;
		_system_configuration.rtc_type = RTC_POWER;
		_system_configuration.resv_size = 0;

		/* see "rosinfo.h" to understand this
		 */
		if ((mach_model & 0xFF000000) == 0)
		{
			_system_configuration.icache_size = 8192;
			_system_configuration.dcache_size = (mach_model & 0x20)
				? 32768 : 65536;
		}
		else
		{
			_system_configuration.icache_size =
						info_ptr->icache_size << 10;
			_system_configuration.dcache_size =
						info_ptr->dcache_size << 10;
		}

#ifdef _POWER_RS1
		if (__power_rs1())
		{
			/* cache is present and separate I/D */
			_system_configuration.cache_attrib = 0x1;
			_system_configuration.icache_asc = 2;
			_system_configuration.dcache_asc = 4;
			/* tlb is present and separate I/D */
			_system_configuration.tlb_attrib = 0x1;
			_system_configuration.itlb_size = 32;
			_system_configuration.dtlb_size = 128;
			_system_configuration.itlb_asc = 2;
			_system_configuration.dtlb_asc = 2;

		}
#endif /* _POWER_RS1 */

#ifdef _POWER_RSC
		if (__power_rsc())
		{
			/* RSC path
			 */
			/* cache is present and combined I/D */
			_system_configuration.cache_attrib = 0x3;
			_system_configuration.icache_asc = 2;
			_system_configuration.dcache_asc = 2;
			/* tlb is present and combined I/D */
			_system_configuration.tlb_attrib = 0x3;
			_system_configuration.itlb_size = 64;
			_system_configuration.dtlb_size = 64;
			_system_configuration.itlb_asc = 2;
			_system_configuration.dtlb_asc = 2;
		}
#endif /* _POWER_RSC */

#ifdef _POWER_RS2
		if (__power_rs2())
		{
			/* RS2 path
			 */
			/* cache is present and separate I/D */
			_system_configuration.cache_attrib = 0x1;
			_system_configuration.icache_asc = 2;
			_system_configuration.dcache_asc = 4;
			/* tlb is present and separate I/D */
			_system_configuration.tlb_attrib = 0x1;
			_system_configuration.itlb_size = 128;
			_system_configuration.dtlb_size = 512;
			_system_configuration.itlb_asc = 2;
			_system_configuration.dtlb_asc = 2;
		}
#endif /* _POWER_RSC */

	}
#endif /* _POWER_RS1 | _POWER_RSC | _POWER_RS2 */

        if (proc_ptr != NULL)
        {
	  /* get the information from ROS
	   */
	  tmp_ptr = proc_ptr;
	  for(i=0;i<proc_ptr->num_of_structs;i++) {
	    if (tmp_ptr->processor_present != PROC_ABSENT) 
	      ncpus_cfg++;
	    if (tmp_ptr->processor_present > 0 )
	      ncpus++;

	    tmp_ptr = (struct processor_info *)((uint)tmp_ptr +
                                                        tmp_ptr->struct_size);
	  }
	  _system_configuration.version = first_proc_ptr->version;
	  _system_configuration.ncpus = ncpus;
	  _system_configuration.width = first_proc_ptr->width;
	  _system_configuration.cache_attrib = first_proc_ptr->cache_attrib;
	  _system_configuration.icache_size = first_proc_ptr->icache_size;
	  _system_configuration.dcache_size = first_proc_ptr->dcache_size;
	  _system_configuration.icache_asc = first_proc_ptr->icache_asc;
	  _system_configuration.dcache_asc = first_proc_ptr->dcache_asc;
	  _system_configuration.icache_block = first_proc_ptr->icache_block;
	  _system_configuration.dcache_block = first_proc_ptr->dcache_block;
	  _system_configuration.icache_line = first_proc_ptr->icache_line;
	  _system_configuration.dcache_line = first_proc_ptr->dcache_line;
	  _system_configuration.L2_cache_size = first_proc_ptr->L2_cache_size;
	  _system_configuration.L2_cache_asc = first_proc_ptr->L2_cache_asc;
	  _system_configuration.tlb_attrib = first_proc_ptr->tlb_attrib;
	  _system_configuration.itlb_size = first_proc_ptr->itlb_size;
	  _system_configuration.dtlb_size = first_proc_ptr->dtlb_size;
	  _system_configuration.itlb_asc = first_proc_ptr->itlb_asc;
	  _system_configuration.dtlb_asc = first_proc_ptr->dtlb_asc;
	  _system_configuration.resv_size = first_proc_ptr->resv_size;
	  _system_configuration.priv_lck_cnt = first_proc_ptr->priv_lck_cnt;
	  _system_configuration.prob_lck_cnt = first_proc_ptr->prob_lck_cnt;
	  _system_configuration.rtc_type = first_proc_ptr->rtc_type;
        }
#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca()) {
		/* Check the revision level of the main HW and FW components */
		pgs_check_levels(dir_ptr->system_vpd_offset);
	}
#endif /* _RS6K_SMP_MCA */

	/* set up CPU information in v structure
	 */
	v.v_ncpus = 1;
	v.v_ncpus_cfg = ncpus_cfg;

}

/*
 * NAME: init_overlay
 *
 * FUNCTION:
 *	This routine initializes millicode and system overlay section.
 * Each entry in the tables consists of the external function pointer
 * followed by is address and size.  A text address for each
 * possible implementation follows.  Where possible RS1 implementation
 * functions are build in place (to reduce kernel size).  A address
 * of NULL indicates that there is no need to copy the overlay
 * when running on the platform
 *
 * Both processor and model overlay tables are processed here
 *
 * NOTES:
 * 	Using a multiply or divide in this routine may prevent the
 * system from booting (milicode is not overlayed yet)
 *
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called early in system initialization.
 * Interrupts are disabled and xlate is off
 *
 * RETURNS:
 *	None
 */
void
init_overlay()
{
	int i;
	int *funct_addr;	/* address of first instruction of function */

	/* do processors specific overlays
	 */
	for (i = 0; i < sizeof(overlay_data)/sizeof(overlay_data[0]); i++)
	{
		funct_addr = overlay_data[i].proc_addr[proc_index()];

		/* If null there is no overlay for this implementation
		 */
		if (funct_addr == NULL)
			continue;

#if defined(_KDB)
		/*
		 * DSI and ISI vectors are connected to KDB until end of initialization
		 */
		if (__kdb() && overlay_data[i].ov_addr == (int *)DS_FLIH_ADDR)
			continue;
		if (__kdb() && overlay_data[i].ov_addr == (int *)IS_FLIH_ADDR)
			continue;
#endif  /* _KDB */
		/* copy the overlay to its correct address.
		 */
		si_bcopy(funct_addr, overlay_data[i].ov_addr,
				overlay_data[i].size);
		si_cflush(overlay_data[i].ov_addr, overlay_data[i].size);

	}

	/* do model specific overlays
	 */
	for (i = 0;
		i < sizeof(model_overlay_data)/sizeof(model_overlay_data[0]);
		i++)
	{
		funct_addr = model_overlay_data[i].model_addr[model_index()];

		/* If null there is no overlay for this implementation
		 */
		if (funct_addr == NULL)
			continue;

		/* copy the overlay to its correct address.
		 */
		si_bcopy(funct_addr, model_overlay_data[i].ov_addr,
				model_overlay_data[i].size);
		si_cflush(model_overlay_data[i].ov_addr,
					model_overlay_data[i].size);
	}

}

/*
 * NAME: fixup_603e
 *
 * FUNCTION:
 *	This function must be called before init_branch.  It is intended
 * to provide workarounds for 603e dd1.4 processors.  The problems are
 *
 *	1 dcbz can hang the machine, if they don't have a sync first
 *
 *	2 stw/mtmsr/l (from guarded) or lwarx can hang the machine
 *
 * 603e specific functions are provided for the dcbz
 *
 * overlay patches are made for i_disable and block_zero.  The patches
 * need to be made for processors without the bug.
 *
 * RETURNS: None
 */
void
fixup_603e()
{
	extern int v_copypage_603(), v_zpage_603();
	extern int ids_603_patch[], blkz_603_patch[];
	extern int vmvcs_nop[], dsis_nop[];
	extern int tlb_nop1[], tlb_nop2[], tlb_nop3[];
	int	pvr;

	pvr = mfpvr();
	if (__power_603() && ((pvr & 0xFFFF0000) == 0x60000) &&
		((pvr & 0xFFFF) <= 0x104) )
	{
		/*
		 * change function descriptors for 603 v_copy_page
		 * and v_zpage to point at workaround versions
		 */
		*(int *)(&v_copypage_ppc_splt) =  *(int *)(&v_copypage_603);
		*(int *)(&v_zpage_ppc) = *(int *)(&v_zpage_603);
		
	}
	else
	{
		/*
		 * disable_lock is an overlay so no cache flush is needed
		 * there is a different version of i_disable for UP/MP
		 */
#ifndef _POWER_MP
		ids_603_patch[0] = BLR_OP;
#endif
		vmvcs_nop[0] = BLR_OP;
		si_cflush(vmvcs_nop, sizeof(int));

		blkz_603_patch[0] = NO_OP;
		si_cflush(blkz_603_patch, sizeof(int));

		tlb_nop1[0] = NO_OP;
		si_cflush(tlb_nop1, sizeof(int));

		tlb_nop2[0] = NO_OP;
		si_cflush(tlb_nop2, sizeof(int));

		tlb_nop3[0] = NO_OP;
		si_cflush(tlb_nop3, sizeof(int));

		dsis_nop[0] = NO_OP;
		si_cflush(dsis_nop, sizeof(int));

	}
} 

/*
 * NAME: init_branch
 *
 * FUNCTION:
 *	This routine initializes the low memory branch table for machine
 * dependent functions.  Each entry in the table consists of a external
 * function name followed by machine dependent implementations of the
 * function.  This routine puts a branch absolute pointing to the correct
 * machine dependent function into the branch table.  The function
 * descriptor for the external name is also patched to point to the
 * machine dependent function.  This allows indirect call to go directly
 * to the correct function.
 *
 * Both processor and model overlay tables are processed here
 *
 * NOTES:
 *	This function manipulates function descriptors.  Function descriptors
 * are what "C" function addresses resolve to.  This code manipulate the
 * first two wods were the first word contains the actual address of the
 * function and the second contains a run-time TOC value.
 *
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called from hardinit().  It runs once during
 * system initialization.  init_system_config() must have already executed.
 * This function runs with ALL interrupts disabled and in real mode.
 * There are no system interrupt handlers established yet, so even
 * something as simple as an alignment int is not recoverable.
 *
 * RETURNS:
 *	None
 */
void
init_branch()
{
	int i;
	int *ba_addr;		/* address to place the BA instruction */
	int (*funct_ptr)();	/* machine dependent function implementation */
	int funct_addr;		/* address of first instruction of function */

	/*
	 * Do processor branch table
	 */
	for (i = 0; i < sizeof(branch_data)/sizeof(branch_data[0]); i++)
	{
		/* determine the machine dependent function to use base
		 * on the machine type we are running on
		 */
		funct_ptr = branch_data[i].proc_name[proc_index()];

		/* Get the address where the branch absolute instruction
		 * must go.  This is contained in the fist word of the
		 * function descriptor
		 */
		ba_addr = (int *)*((int *)branch_data[i].ext_name);

		/* get the machine dependent function address.  This address
		 * is in the first word of the function descriptor
		 */
		funct_addr = *((int *)funct_ptr);

		/* store a branch absolute into branch table.  Also patch
		 * the function descriptor to point directly to the machine
		 * dependent function
		 */
		*ba_addr = funct_addr | BA_MASK;
		*(int *)branch_data[i].ext_name = funct_addr;

		/* code has been modified so do a cache flush.  A special
		 * cache flush routine is used as the cache flush services
		 * are in the branch table
		 */
		si_cflush(ba_addr, sizeof(int));
	}

	/*
	 * Do model branch table
	 */
	for (i = 0; i < sizeof(model_branch_data)/sizeof(model_branch_data[0]);
									i++)
	{
		/* determine the machine dependent function to use base
		 * on the machine type we are running on
		 */
		funct_ptr = model_branch_data[i].model_name[model_index()];

		/* Get the address where the branch absolute instruction
		 * must go.  This is contained in the fist word of the
		 * function descriptor
		 */
		ba_addr = (int *)*((int *)model_branch_data[i].ext_name);

		/* get the machine dependent function address.  This address
		 * is in the first word of the function descriptor
		 */
		funct_addr = *((int *)funct_ptr);

		/* store a branch absolute into branch table.  Also patch
		 * the function descriptor to point directly to the machine
		 * dependent function
		 */
		*ba_addr = funct_addr | BA_MASK;
		*(int *)model_branch_data[i].ext_name = funct_addr;

		/* code has been modified so do a cache flush.  A special
		 * cache flush routine is used as the cache flush services
		 * are in the branch table
		 */
		si_cflush(ba_addr, sizeof(int));
	}

}

/*
 * NAME: sysml_init
 *
 * FUNCTION:
 *	initalize the sysml component
 *
 * RETURNS: None
 */
void
sysml_init()
{
	int dcong, icong;	/* i/d cache info */
#ifdef _POWER_MP
	int i;
	extern uint mproc_physid;
	extern uint my_phys_id();
#endif /* _POWER_MP */

	/* Set up the current save area pointer to the mst
	 * used during initialization.  This is necessary
	 * in order to use 'io_att' and 'io_det' since they
	 * refer to the mstsralloc field in the mst.
	 */
#ifndef _POWER_MP
	csa = &si_save;
#else /* _POWER_MP */
	ppda[MP_MASTER]._csa = &si_save;
#endif /* _POWER_MP */


	/* initialize low memory pointer to ppda
	 */
	proc_arr_addr = &ppda[0];
#ifdef _POWER_MP
	/* set up the ppda pointer array */
	for (i=0;i<MAXCPU;i++) ppda_p_tab[i] = &ppda[i];
#endif /* _POWER_MP */

	/* initialize misc. machine dependent functions
	 */
#ifdef _POWER_PC
	if (__power_pc())
	{
		/* initialize SPRGs : 0-ppda, 2-reserved, 3-csa
		 */
		mtsprgs(&ppda[0], 0xDEADF00D, &si_save);

		/* set up key_value variable with "K" bit settings
		 */
		key_value = Ks|Kp;
	}
#endif /* _POWER_PC */

#ifdef _POWER_RS
	if (__power_rs())
	{
		/* set up key_value variable with "K" bit setting
		 */
		key_value = Ks;
	}
#endif /* _POWER_RS */

	/*
	 * set up cache synomoym variable in system configuration
	 * structure.  This is done here since multiplication and
	 * division are required on some platforms.  The field
	 * is defined as the number of bits
	 */
	if (__power_rs())
	{
		/*
		 * If effective address bits < b20 are used to index the
		 * cache there are synonym problems.  cache_size/cache_assoc
		 * give the indexing.  First isolate the bits that are used
		 * in cache indexing
		 */
		dcong = (_system_configuration.dcache_size - 1) /
			(_system_configuration.dcache_asc * PAGESIZE);
		icong = (_system_configuration.icache_size - 1) /
			(_system_configuration.icache_asc * PAGESIZE);

		/*
		 * get number of bits in the page number that participate
		 * in cache indexing
		 */
		_system_configuration.cach_cong = 32 - clz32(MAX(dcong, icong));
	}
	else
	{
		/*
		 * There are never synonyms on a PPC
		 */
		_system_configuration.cach_cong = 0;
	}

#ifdef _POWER_MP
	/* set up the MP_MASTER ppda entry */
	ppda[MP_MASTER].cpuid = MP_MASTER;
	ppda[MP_MASTER].mpc_pend = 0;
	ppda[MP_MASTER].mfrr_pend = 0;
	ppda[MP_MASTER].mfrr_lock = 0;

	mproc_physid = my_phys_id();
#endif /* _POWER_MP */
}

/*
 * NAME: hardinit
 *
 * FUNCTION: initialize hardware dependencies in the kernel
 *
 * EXECUTION ENVIRONMENT:
 *	Called from main() after getting control from ROS.  This code runs
 * xlate off and interrupts disabled.
 *
 * RETURNS: None
 */
void
hardinit()
{
        void init_sys_ranges();

	/* Initialize system configuration structure.  This must be called
	 * first since the __power_xxx() macros do not work until this has
	 * been called
	 */
	init_system_config();

	/* Clear the LEDs to show that we are in control. This call must go
	 * after init_system_config() since it uses the system configuration
	 * structure to determine if the machine has actual LEDs.
	 */
	write_leds(0xfff00000);

	/*
	 * if running a 603 processor, check if 603e 1.4 workarounds need
	 * to be applied
	 */
	fixup_603e();

	/* Initialize overlays.  This needs to be done early since
	 * milicode (multiply/divide) routines are setup by this
	 * function
	 */
	init_overlay();

	/* Initialize low memory branch table
	 */
	init_branch();

	/* Initialize interrupt vectors
	 */
	init_int_vectors();

	/* Initialize machine dependent ranges to be pinned or released
	 * by vmsi()
	 */
	init_mach_ranges();

	/* Initialize sysml component
	 */
	sysml_init();

#ifdef _POWER_RS1
	/* Initialize system configuration
	 */
	init_config();
#endif /* _POWER_RS1 */


	/*
	 * Initialize System memory ranges to be mapped by VMM in vmsi()
	 */
	init_sys_ranges();

#if _POWER_PC
	if (__power_pc()) {
		/* This model uses the software managed interrupt model */
		soft_int_model = TRUE;
	}
#endif /* _POWER_PC */


        /*
         * Set the decrementer to max time and hope this is enough time
         * to get the interrupt subsystem initialized.  If the debugger
         * is invoked then this won't happen but only side effect of
         * this is phantom interrupts detected in the flih.
         *
         * Any pending bogus decrementer interrupts will be discarded
         * when the EIS/PEIS is initialized(cleared) later.
         */

	mtdec(-1);

	return;
}


#ifdef _POWER_RS1

/* Macros for extracting information from a word containing
 * Vital Product Data (VPD) for a hardware chip.
 * This data is located in NVRAM starting at addresses 0x0388
 * and is recorded by the on-card sequencer (OCS) after running
 * the built-in self tests (BIST).
 * There are 7-10 words depending on the machine model.
 * Each word has the following format:
 * (MSB) byte 0 - COP Bus Address (CBA) of chip
 *       byte 1 - zero
 *       byte 2 - DD level of chip
 * (LSB) byte 3 - Part Number of chip
 */
#define VPD_TO_CBA(w)	(((w) >> 24) & 0xFF)
#define VPD_TO_DD(w)	(((w) >> 8) & 0xFF)
#define VPD_TO_PN(w)	((w) & 0xFF)

/* Offset in IOCC to NVRAM.
 */
#define NVRAM_OFFSET	0x00A00000
#define VPD_OFFSET	0x0388

/* These are the CBA's corresponding to each CEC chip.
 */
#define CBA_FPU		0x01
#define CBA_FXU		0x02
#define CBA_ICU		0x03
#define CBA_SCU		0x04
#define CBA_DCU1	0x0A
#define CBA_DCU2	0x2A
#define CBA_DCU3	0x4A
#define CBA_DCU4	0x6A
#define CBA_IOCC2	0x3D

/*
   DESCRIPTIVE NAME: Define system configuration

   FUNCTION: This routine initializes AIX configuration data.
             It must run before vmsi.
*/

void 
init_config()
{
	register int	j;			/* count variable */
	register ulong	ioccaddr;		/* io_att return value */
	volatile ulong	*vpd_start;		/* start address of VPD */
	volatile ulong	*vpd_end;		/* end address of VPD */
	volatile ulong	*vpd_ptr;		/* pointer to VPD */
	register ulong	vpd;			/* 4 bytes of VPD from NVRAM */

	if (__power_rs1())
	{
		/* Set up addressability to the IOCC.
		 */
		ioccaddr = (ulong) io_att( IOCC_SEG_REG_VALUE, 0 );

		/* Set any configuration parameters that are dependent
		 * on the hardware level of any of the CEC chips.
		 */

		/* Examine all 10 words of chip VPD.
		 * This assumes that there isn't a valid CBA ID in the
		 * words that are not set by the OCS.
		 */
		vpd_start = (volatile ulong *)( ioccaddr + NVRAM_OFFSET
								+ VPD_OFFSET);
		vpd_end   = vpd_start + 10;

		for ( vpd_ptr = vpd_start; vpd_ptr < vpd_end; vpd_ptr++)
		{
			vpd = *vpd_ptr;
			switch(VPD_TO_CBA(vpd))
			{
			case CBA_FXU:
				/* TEMPORARY - set variable to indicate if
				 * fetch-protect/store problem exists.
				 * This can be removed when we no longer need
				 * to support this level of chip.
				 *
				 * If fixed point unit is level 2.1 or lower
				 * then set variable to non-zero value
				 * (value of vpd).
				 */
				if (VPD_TO_DD(vpd) <= 0x21)
					vmker.nofetchprot = vpd;
				else
					vmker.nofetchprot = 0;

				break;

			case CBA_FPU:
			case CBA_ICU:
			case CBA_SCU:
			case CBA_DCU1:
			case CBA_DCU2:
			case CBA_DCU3:
			case CBA_DCU4:
			case CBA_IOCC2:
			default:
				break;
			};
		};

		/*
		 * Drop addressability to the IOCC.
		 */
		io_det( ioccaddr );
	}


}
#endif /* _POWER_RS1 */


/*
 * NAME: init_sys_ranges
 *
 * FUNCTION:
 *	This routine initializes an array of various system data memory ranges
 *	to be mapped by vmsi().  The ranges include ranges which were loaded
 *	by ROS such as the kernel and also platform specific system registers,
 *	NVRAM, TCE memory, etc..  Each entry in the array consists of the
 *	real address, virtual address to map to, range characteristics, and
 *	size in bytes.
 *
 * NOTES:
 *	This routine MUST be called before vmsi().
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called from hardinit().  It runs once during
 *      system initialization for the PowerPC platform.  
 * 	This function runs with ALL interrupts disabled and in real mode.
 * 	There are no system interrupt handlers established yet, so even
 * 	something as simple as an alignment int is not recoverable.
 *
 * RETURNS:
 *	None
 */
void
init_sys_ranges()
{
	struct ipl_cb	*iplcb_ptr;		/* ros ipl cb pointer */
	struct ipl_directory *dir_ptr;		/* ipl dir pointer */
	struct buc_info	*buc_ptr;		/* BUC info pointer */
	struct system_info *sys_ptr;		/* System info pointer */
	int	buc_cnt =0;			/* BUC count */
	int	num_bucs =0;			/* count of BUCs */
	uint	kext_addr;			/* address in kernel ext seg */
	uint	ioccaddr;			/* address for access to IOCC */
	uint	tce_addr;			/* TCE base addr */
	uint	tce_size;			/* size of TCE table */
	uint	end_tce_space;			/* end of TCE virtual space */
	uint	mcu_control;			/* MCU control address space */
	int	i_cnt;				/* count of IOCCs */
        int	mstbeg,mstend,leniplcb;
	char	found_MCU=FALSE;		/* flag whether MCU was found*/
	struct  businfo bus;			/* bus information struct */
	extern	header_offset;
	extern	ram_disk_start, ram_disk_end;
	extern	base_conf_start, base_conf_end;
        struct  ppda    *ppda_ptr;              /* pointer to ppda */

	/*
	 * Initialize the rmap array and then initialize ranges established
	 * by ROS and ranges that must be addressible before vmsi() completes
	 * (such as the MSTs).
	 */
	RMAP_INIT;

        /*
         * Kernel
         */
	vmrmap_id(RMAP_KERN)	= RMAP_KERN;
	vmrmap_raddr(RMAP_KERN)	= 0;
	vmrmap_eaddr(RMAP_KERN)	= 0;
	vmrmap_size(RMAP_KERN) 	= header_offset;
	vmrmap_wimg(RMAP_KERN)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_KERN)	= 0;
	vmrmap_ros(RMAP_KERN)	= 1;
	vmrmap_valid(RMAP_KERN)	= 1;

        /*
         * RAM disk
         */
	vmrmap_id(RMAP_RAMD)	= RMAP_RAMD;
	vmrmap_raddr(RMAP_RAMD)	= ram_disk_start;
	vmrmap_size(RMAP_RAMD) 	= ram_disk_end - ram_disk_start;
	vmrmap_wimg(RMAP_RAMD)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_RAMD)	= 0;
	vmrmap_ros(RMAP_RAMD)	= 1;
	/*
	 * If bootexpand ran then the ram_disk_start and _end
	 * account for bad memory already.  If not, we set the
	 * "holes" flag and the VMM takes care of it.
	 */
	if ((dbg_avail & EXPAND_RAMD) == 0)
		vmrmap_holes(RMAP_RAMD) = 1;
	vmrmap_seg(RMAP_RAMD)	= 1;
	vmrmap_valid(RMAP_RAMD)	= 1;

        /*
         * Base config area
         */
	if (base_conf_end - base_conf_start != 0) {
		vmrmap_id(RMAP_BCFG)	= RMAP_BCFG;
		vmrmap_raddr(RMAP_BCFG)	= base_conf_start;
		vmrmap_size(RMAP_BCFG) 	= base_conf_end - base_conf_start;
		vmrmap_wimg(RMAP_BCFG)	= WIMG_DEFAULT;
		vmrmap_align(RMAP_BCFG)	= 0;
		vmrmap_ros(RMAP_BCFG)	= 1;
		/*
		 * If bootexpand ran then the base_conf_start and _end
		 * account for bad memory already.  If not, we set the
		 * "holes" flag and the VMM takes care of it.
		 */
		if ((dbg_avail & EXPAND_BCFG) == 0)
		        vmrmap_holes(RMAP_BCFG) = 1;
		vmrmap_seg(RMAP_BCFG)	= 1;
		vmrmap_valid(RMAP_BCFG)	= 1;
	}

#ifdef _POWER_MP
	/*
	 * Check lock instrumentation (through dbg_avail & bosboot) before clearing other flags
	 */
	instr_avail = dbg_avail & LOCK_INSTR_AVAIL;
#endif /* _POWER_MP */

	/*
	 * Clear other bit settings in debugger flags.
	 */
	dbg_avail &= LLDB_MASK;

        /*
         * IPL control block. The VMM will move it later to after
         * endload but it is given this virtual address so that
         * it can be addressed V = R (in the kernel segment) for awhile.
	 * There is an optional Extended IPL Control Block used for
	 * ROS Emulation, which exists if global ipl_cb_ext is non-NULL.
	 * It is internally identical to the primary IPL CB.
         */

        iplcb_ptr = (struct ipl_cb *)ipl_cb;
        dir_ptr = (struct ipl_directory *)(&iplcb_ptr->s0);
	/*
	 * Add starting page offset to length in order to handle
	 * non page-aligned iplcb.
	 */
        leniplcb = dir_ptr->ipl_cb_and_bit_map_size + (ipl_cb & (PAGESIZE-1));
	vmrmap_id(RMAP_IPLCB)	= RMAP_IPLCB;
	vmrmap_raddr(RMAP_IPLCB)= ipl_cb;
	vmrmap_eaddr(RMAP_IPLCB)= ipl_cb & 0x0fffffff;
	vmrmap_size(RMAP_IPLCB) = leniplcb;
	vmrmap_wimg(RMAP_IPLCB)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_IPLCB)= 0;
	vmrmap_ros(RMAP_IPLCB)	= 1;
	vmrmap_valid(RMAP_IPLCB)= 1;

	/*
	 * Handle the Extended IPL Control Block, if it exists.
	 */
	if (ipl_cb_ext != 0)
	{
        	iplcb_ptr = (struct ipl_cb *)ipl_cb_ext;
        	dir_ptr = (struct ipl_directory *)(&iplcb_ptr->s0);
        	leniplcb  = dir_ptr->ipl_cb_and_bit_map_size;
        	leniplcb -= dir_ptr->bit_map_size;
        	leniplcb += (ipl_cb_ext & (PAGESIZE-1));
		vmrmap_id(RMAP_IPLCBX)   = RMAP_IPLCBX;
		vmrmap_raddr(RMAP_IPLCBX)= ipl_cb_ext;
		vmrmap_eaddr(RMAP_IPLCBX)= ipl_cb_ext & 0x0fffffff;
		vmrmap_size(RMAP_IPLCBX) = leniplcb;
		vmrmap_wimg(RMAP_IPLCBX) = WIMG_DEFAULT;
		vmrmap_align(RMAP_IPLCBX)= 0;
		vmrmap_ros(RMAP_IPLCBX)  = 1;
		vmrmap_valid(RMAP_IPLCBX)= 1;
	}
	else
		vmrmap_raddr(RMAP_IPLCBX)= 0;

        /*
	 * MSTs
	 */
	mstbeg = (~(PAGESIZE-1)) & (uint) &mststack[0];
	mstend = (uint) &mststack[NUMBER_OF_FRAMES *
					_system_configuration.ncpus];
	vmrmap_id(RMAP_MST)	= RMAP_MST;
	vmrmap_eaddr(RMAP_MST)	= mstbeg;
	vmrmap_size(RMAP_MST) 	= mstend - mstbeg;
	vmrmap_wimg(RMAP_MST)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_MST)	= PAGESIZE;
	vmrmap_valid(RMAP_MST)	= 1;

	/*
	 * Now initialize platform-specific ranges.
	 */
#ifdef _RS6K
	if (__rs6k())
	{
        /*
         * Get ppda pointer
         */
        ppda_ptr = &ppda[0];
#ifdef _POWER_MP
        /*
         * Save Address of the Interrupt Registers in the PPDA
         */
        ppda_ptr->intr = (void *)get_proc_int_area();
#else /* _POWER_MP */
        /*
         * Save Address of the Interrupt Registers in the PPDA
         * NOTE: This is hardcoded for the UP system.
         */
        ppda_ptr->intr = (void *)(&(sys_resource_ptr->
			sys_interrupt_space.sys_intr_regs[0]));
#endif /* _POWER_MP */

	/*
	 * Initialize System Register Space
	 */
	RMAP_NEXT;
	vmrmap_id(RMAP_X)	= RMAP_SYSREG;
	vmrmap_raddr(RMAP_X)	= 0xFF000000;	/* register space begins here */
	vmrmap_eaddr(RMAP_X)	= &sys_resource_ptr->sys_regs;
	vmrmap_size(RMAP_X)	= sizeof(struct sys_registers);
	vmrmap_io(RMAP_X)	= 1;		/* I/O space */
	vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
	vmrmap_align(RMAP_X)	= 0;		/* No alignment requirements */
	vmrmap_valid(RMAP_X)	= 1;

	/*
	 * Initialize System Interrupt Register Space
	 */
	RMAP_NEXT;
	vmrmap_id(RMAP_X)	= RMAP_SYSINT;
	vmrmap_raddr(RMAP_X)	= 0xFF100000;	/* interrupt space */
	vmrmap_eaddr(RMAP_X)	= &sys_resource_ptr->sys_interrupt_space;
	vmrmap_size(RMAP_X)	= sizeof(struct sys_interrupt_regs) * MAXCPU;
	vmrmap_io(RMAP_X)	= 1;		/* I/O space */
	vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
	vmrmap_align(RMAP_X)	= 0;		/* No alignment requirements */
	vmrmap_valid(RMAP_X)	= 1;


	/* 
	 * get addressability to iplinfo structure
	 */
	iplcb_ptr = (struct ipl_cb *)ipl_cb;
	sys_ptr = (struct system_info *)((uint)iplcb_ptr + 
					iplcb_ptr->s0.system_info_offset);
	/*
 	 * Initialize NVRAM space
	 */
	RMAP_NEXT;
	vmrmap_id(RMAP_X)	= RMAP_NVRAM;
	vmrmap_raddr(RMAP_X)	= 0xFF600000;	/* NVRAM space begins here */
	vmrmap_eaddr(RMAP_X)	= &sys_resource_ptr->nvram;
	vmrmap_size(RMAP_X)	= sys_ptr->nvram_size;
	vmrmap_io(RMAP_X)	= 1;		/* I/O space */
	vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
	vmrmap_align(RMAP_X)	= 0;		/* No alignment requirements */
	vmrmap_valid(RMAP_X)	= 1;


	/* 
	 * Initialize TCE Tables and memory controller Registers.  Notice
	 * that when the MCU BUC is found, its registers are mapped virtually
	 * at the current offset into the kernel extension segment (wherever
	 * we are when we find it).
	 *
	 * Walk the IPL Control block looking at all BUCs.  When an IOCC is
	 * found, determine its TCE address and size and initialize the
	 * range.  When the MCU is found, determine its control address space
	 * and initialize a range for it.
	 */

	buc_ptr = (struct buc_info *)((uint)iplcb_ptr + 
					iplcb_ptr->s0.buc_info_offset);
	num_bucs = buc_ptr->num_of_structs;
	kext_addr = &TCE_space;
	end_tce_space = (uint)&TCE_space + (16 << 20); /* next 16MB boundary */
	/* mark each entry of IOCC info struct as invalid
	 */
	for(i_cnt=0;i_cnt < MAX_NUM_IOCCS;i_cnt++)
		iocc_info[i_cnt].bid = 0xFFFFFFFF;
	i_cnt=0;

	for(buc_cnt=0;buc_cnt < num_bucs;buc_cnt++) {
		/*
		 * For each BUC
		 */
		if (kext_addr >= end_tce_space) {
			/*
			 * If we have overflowed the maximum TCE space of
			 * 16 MB, then loop forever
			 */
			si_halt();
		}
		if (buc_ptr->IOCC_flag) {
			/*
			 * This in an IOCC...set up addressability
			 */
			iocc_info[i_cnt].bid = 
				((buc_ptr->buid_data[0].buid_value << 20) & 
				0x1FF00000) | 0x80000080;
			iocc_info[i_cnt].tce_offset = kext_addr;
			/*
			 * Call Assembler routine to read IOCC 
			 * Config Reg and TCE Anchor Register
			 */
			read_iocc_info(iocc_info[i_cnt].bid, &tce_addr);
			tce_size = NUM_TCES_PPC(tce_addr) << 2;

			RMAP_NEXT;
			vmrmap_id(RMAP_X)	= RMAP_TCE;
			vmrmap_raddr(RMAP_X)	= (tce_addr & 0xFFFFFFF8);
			vmrmap_eaddr(RMAP_X)	= kext_addr;
			vmrmap_size(RMAP_X)	= tce_size;
			vmrmap_ros(RMAP_X)	= 1;	/* estab. by ROS */

#ifdef _RS6K_SMP_MCA
			/* Due to HW problem in Ionian we cannot cache TCEs
			 * So, map them CI & Coherent
			 */
			if (__rs6k_smp_mca()) {
				vmrmap_wimg(RMAP_X) = 0x6;
			}
			else
#endif /* _RS6K_SMP_MCA */
			{
				vmrmap_wimg(RMAP_X) = WIMG_DEFAULT;
			}

			vmrmap_align(RMAP_X)	= 0;	/* No alignment req */
			vmrmap_valid(RMAP_X)	= 1;

			kext_addr += tce_size;	/* bump up for next space */
			i_cnt++;
		} else if (buc_ptr->device_type == 0x00000001) {
#ifdef _RS6K_SMP_MCA
			if (!__rs6k_smp_mca()) {
#endif /* _RS6K_SMP_MCA */
			/*
			 * Memory Controller BUC
			 */
			if (found_MCU) {
				/*
				 * something's wrong....we've already
				 * set up for the MCU.  Loop forever writing
				 * the leds 
				 */
				si_halt();
			}
			found_MCU=TRUE;
			/*
			 * This is an MCU....get real address of its
			 * control space from IPLCB info.
			 */
			mcu_control = (uint)buc_ptr->mem_addr1;
			/*
			 * Map the MCU Control Space
			 */
			RMAP_NEXT;
			vmrmap_id(RMAP_X)	= RMAP_MCSR;
			vmrmap_raddr(RMAP_X)	= mcu_control;
			vmrmap_eaddr(RMAP_X)	= kext_addr;
			vmrmap_size(RMAP_X)	= 4;
			vmrmap_io(RMAP_X)	= 1;	/* I/O space */
			vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
			vmrmap_align(RMAP_X)	= 0;	/* No alignment req */
			vmrmap_valid(RMAP_X)	= 1;

			mcsr_ppc = kext_addr + 0x4;/* set up global pointer */
			mear_ppc = kext_addr + 0xC;/* set up global pointer */
			kext_addr += 0x1000;	/* bump up for next space */
#ifdef _RS6K_SMP_MCA
			}
#endif /* _RS6K_SMP_MCA */
		}
		/*
		 * Point to the next BUC
		 */
		buc_ptr = (struct buc_info *)((uint)buc_ptr + 
							buc_ptr->struct_size);
	} /* for each buc */

#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca()) {
		/*
		 * Initialize System Specific Register Space
		 */
		RMAP_NEXT;
		vmrmap_id(RMAP_X)	= RMAP_SYS_SPEC_REG;
		vmrmap_raddr(RMAP_X)	= 0xFF001000;
		vmrmap_eaddr(RMAP_X)	= sys_resource_ptr->sys_specific_regs;
		vmrmap_size(RMAP_X)	= sizeof(struct pgs_sys_spec);
		vmrmap_io(RMAP_X)	= 1;	/* I/O space */
		vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
		vmrmap_align(RMAP_X)	= 0;	/* No alignment requirements */
		vmrmap_valid(RMAP_X)	= 1;

		/*
		 * Initialize APR mapping
		 */
		RMAP_NEXT;
		vmrmap_id(RMAP_X)	= RMAP_APR;
		vmrmap_raddr(RMAP_X)	= 0xFF180000;	/* APR address */
		vmrmap_eaddr(RMAP_X)	= sys_resource_ptr->reserved3;
		vmrmap_size(RMAP_X)	= sizeof(struct pgs_apr);
		vmrmap_io(RMAP_X)	= 1;	/* I/O space */
		vmrmap_wimg(RMAP_X)	= WIMG_IG;	/* inhibit, guarded */
		vmrmap_align(RMAP_X)	= 0;	/* No alignment requirements */
		vmrmap_valid(RMAP_X)	= 1;

		/* The following are already mapped through NVRAM */
		rsr_addr  = SYS_RES_EADD(sys_ptr->rsr_addr);
		pksr_addr = SYS_RES_EADD(sys_ptr->pksr_addr);
		prcr_addr = SYS_RES_EADD(sys_ptr->prcr_addr);
		spocr_addr = SYS_RES_EADD(sys_ptr->spocr_addr);
	} else {
		rsr_addr  = &sys_resource_ptr->sys_regs.reset_status;
		pksr_addr = &sys_resource_ptr->sys_regs.pwr_key_status;
		prcr_addr = &sys_resource_ptr->sys_regs.pwr_on_reset_cr;
	}
#endif /* _RS6K_SMP_MCA */
	} /* __rs6k() */
#endif /* _RS6K */

#ifdef _RSPC
	if (__rspc()) {
		/*
		 * for all RSPC class machines....
		 * default the raddr and size to 0 in case there is no
		 * BUC indicating a real-heap
		 */
		rheap->raddr = 0;
		rheap->size = 0;
		buc_ptr = (struct buc_info *)((uint)iplcb_ptr +
				iplcb_ptr->s0.buc_info_offset);
		for(buc_cnt=0;buc_cnt < buc_ptr->num_of_structs;buc_cnt++) {
		   /*
		    * For each BUC
		    */
		   if (buc_ptr->device_type == A_BUS_BRIDGE) {
			/* 
			 * If this a bus bridge type BUC, then...
			 */
			if (buc_ptr->IOCC_flag) {
			   /*
			    * This indicates that this is the bus that
			    * has the native serial port on it, so
			    * register it for the debugger
			    */
                	   bus.bid = buc_ptr->buid_data[0].buid_value;
			   bus.num_regions = buc_ptr->num_of_buids;
			   bus.d_map_init = NULL;
			   for (i_cnt=0;i_cnt<buc_ptr->num_of_buids;i_cnt++) 
				/*
				 * for each region
				 */
				bus.ioaddr[i_cnt] = buc_ptr->
						buid_data[i_cnt].buid_Sptr;
			   /*
			    * Now register the bus
			    */
                	   if (bus_register(&bus))
				/*
				 * somethings not right...
				 */
				si_halt();

		            /*
		             * setup NIO io map structure
		             */
		            nio_map.key = IO_MEM_MAP;
		            nio_map.flags = 0;
		            nio_map.size = 0x10000;
		            nio_map.bid = bus.bid;
		            nio_map.busaddr = 0;
			}
			if (buc_ptr->device_id_reg == DMA_BUC_ID) {
				/*
				 * This indicates the REAL HEAP BUC info
				 * Setup an RMAP structure to map the real
				 * heap into the kernel extension segment
				 */
				rheap->raddr = buc_ptr->mem_addr1;
				rheap->size = buc_ptr->mem_alloc1;
				rheap->vaddr = (caddr_t)&TCE_space;
				RMAP_NEXT;
				vmrmap_id(RMAP_X)	= RMAP_TCE;
				vmrmap_raddr(RMAP_X)	= rheap->raddr;
				vmrmap_eaddr(RMAP_X)	= rheap->vaddr;
				vmrmap_size(RMAP_X)	= rheap->size;
				vmrmap_ros(RMAP_X)	= 1;	
				vmrmap_wimg(RMAP_X)	= WIMG_DEFAULT;	
				vmrmap_align(RMAP_X)	= 0;	
				vmrmap_valid(RMAP_X)	= 1;
			}
		   }
		   /*
		    * Point to the next BUC
		    */
		   buc_ptr = (struct buc_info *)((uint)buc_ptr + 
							buc_ptr->struct_size);
		}
	}
	
#endif /* _RSPC */

}

#ifdef _POWER_MP

/*
 * NAME: get_proc_int_area
 *
 * FUNCTION: Determine the boot-master processor interrupt area effective address
 *
 * INPUT: scans ipl control block to find out the first processor in the
 *	"running AIX" state. (there should be only one match ...)
 *
 * EXECUTION ENVIRONMENT:
 *	Can be used only before any boot-slave processor has been started.
 *
 * RETURNS: Base Address of this processor interrupt presentation layer registers   
 *	    -1 in case of failure (no processor found) 
 */
get_proc_int_area()
{
	struct ipl_cb *iplcb_ptr;	/* ros ipl cb pointer */
	struct ipl_directory *dir_ptr;	/* ipl dir pointer */
        struct processor_info *proc_ptr;
	int index;

	iplcb_ptr = (struct ipl_cb *)ipl_cb;
	dir_ptr = &(iplcb_ptr->s0);
        proc_ptr =  (struct processor_info *)((char *) iplcb_ptr +
					      dir_ptr->processor_info_offset);
	for (index = 0 ; index < proc_ptr->num_of_structs; index++) {
		if (proc_ptr->processor_present == PROC_RUNNING_AIX)
			return (SYS_RES_EADD(proc_ptr->proc_int_area));
		proc_ptr = (struct processor_info *)((uint)proc_ptr +
						     proc_ptr->struct_size);
	}
	return (-1);
}

#endif /* _POWER_MP */
