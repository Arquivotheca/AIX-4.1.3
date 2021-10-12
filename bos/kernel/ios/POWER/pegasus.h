/* @(#)72       1.3  src/bos/kernel/ios/POWER/pegasus.h, sysios, bos41B, 412_41B_sync 1/6/95 13:16:01 */
#ifndef _H_PEGASUS
#define _H_PEGASUS
/*
 * COMPONENT_NAME: (SYSIOS) I/O Subsystem
 *
 * FUNCTIONS: Pegasus Specific System Resources
 *
 * ORIGINS: 83
 *
 *
 * LEVEL 1,  5 Years Bull Confidential Information
 */
/*
 * The following structures describe the memory-mapped resources of
 * Pegasus.
 * They map some "reserved" fields in sys_resource.h
 */

/*
 * System Specific Registers, 0xFF001000 - 0xFF001FFF
 */
struct pgs_sys_spec {
	uchar	start_rtc;	/* start real time clock */
	uchar	padd1[3];
	uchar	set_int_bu;	/* set interrupt to bump */
	uchar	padd2[3];
	uchar	res_int_fb;	/* reset bump interrupt */
	uchar	padd3[3];
	uchar	iod_hw_sts;	/* IOD hardware status */
	uchar	padd4[3];
	uchar	buid;		/* Bus Unit ID Register */
	uchar	padd5[3];
};

/* IOD hardware status specification */
#define EPOW_MAIN_CAB 	0x08
#define EPOW_MCA_CAB	0x80

/*
 * SMP Available Processor Registers, 0xFF180000 - 0xFF1FFFFF
 */
struct pgs_apr {
	uchar    apr;		/* only one 8 bits register */
	uchar	pad[3];		/* pad to next word */
};

#endif /* _H_PEGASUS */
