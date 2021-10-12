/* @(#)29	1.5  src/bos/kernel/sys/POWER/systemcfg.h, sysml, bos411, 9428A410j 3/20/94 21:33:34 */
#ifndef _H_SYSTEMCFG
#define _H_SYSTEMCFG
/*
 * COMPONENT_NAME: (SYSML) Kernel Machine Language
 *
 * FUNCTIONS: System Characteristics Identification
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * WARNINGS: 
 *	The size of this structure will grow in future releases
 *
 *	Do not make this structure larger than 64 words
 *
 *	Do not update without changing systemcfg.m4 and ufcp.m4
 */
extern struct {
	int architecture;	/* processor architecture */
	int implementation;	/* processor implementation */
	int version;		/* processor version */
	int width;		/* width (32 || 64) */
	int ncpus;		/* 1 = UP, n = n-way MP */
	int cache_attrib;	/* cache attributes (bit flags)		*/
				/* bit		0/1 meaning   		*/
				/* -------------------------------------*/
				/* 31	 no cache / cache present	*/
				/* 30	 separate I and D / combined    */
	int icache_size;	/* size of L1 instruction cache */
	int dcache_size;	/* size of L1 data cache */
	int icache_asc;		/* L1 instruction cache associativity */
	int dcache_asc;		/* L1 data cache associativity */
	int icache_block;	/* L1 instruction cache block size */
	int dcache_block;	/* L1 data cache block size */
	int icache_line;	/* L1 instruction cache line size */
	int dcache_line;	/* L1 data cache line size */
	int L2_cache_size;	/* size of L2 cache, 0 = No L2 cache */
	int L2_cache_asc;	/* L2 cache associativity */
	int tlb_attrib;		/* TLB attributes (bit flags)		*/
				/* bit		0/1 meaning   		*/
				/* -------------------------------------*/
				/* 31	 no TLB / TLB present		*/
				/* 30	 separate I and D / combined    */
	int itlb_size;		/* entries in instruction TLB */
	int dtlb_size;		/* entries in data TLB */
	int itlb_asc;		/* instruction tlb associativity */
	int dtlb_asc;		/* data tlb associativity */
	int resv_size;		/* size of reservation */
	int priv_lck_cnt;	/* spin lock count in supevisor mode */
	int prob_lck_cnt;	/* spin lock count in problem state */
	int rtc_type;		/* RTC type */
	int virt_alias;		/* 1 if hardware aliasing is supported */
	int cach_cong;		/* number of page bits for cache synonym */
	int model_arch;		/* used by system for model determination */
	int model_impl;		/* used by system for model determination */
	int Xint;		/* used by system for time base conversion */
	int Xfrac;		/* used by system for time base conversion */
}_system_configuration;

/* Values for architecture field
 */
#define POWER_RS	0x0001		/* Power Classic architecture */
#define POWER_PC	0x0002		/* Power PC architecture */

/* Values for   implementation field
 */
#define POWER_RS1	0x0001		/* RS1 class CPU */
#define POWER_RSC	0x0002		/* RSC class CPU */
#define POWER_RS2	0x0004		/* RS2 class CPU */
#define POWER_601	0x0008		/* 601 class CPU */
#define POWER_603	0x0020		/* 603 class CPU */
#define POWER_604	0x0010		/* 604 class CPU */
#define POWER_620	0x0040		/* 620 class CPU */

/* Sets of implementations
 */
#define POWER_RS_ALL (POWER_RS1|POWER_RSC|POWER_RS2)
#define POWER_PC_ALL (POWER_601|POWER_603|POWER_604|POWER_620)

/* Values for the version field
 */
#define PV_601		0x10001		/* Power PC 601 */
#define PV_601a		0x10002		/* Power PC 601 */
#define PV_603		0x60000		/* Power PC 603 */
#define PV_604		0x50000		/* Power PC 604 */
#define PV_620		0x70000		/* Power PC 620 */
#define PV_RS2		0x40000		/* Power RS2 */
#define PV_RS1		0x20000		/* Power RS1 */
#define PV_RSC		0x30000		/* Power RSC */



/* Values for rtc_type
 */
#define RTC_POWER 1			/* rtc as defined by Power Arch. */
#define RTC_POWER_PC 2			/* rtc as defined by Power PC Arch. */

/* macros for runtime architecture and implementation checks
 */
#define __power_rs() (_system_configuration.architecture == POWER_RS)
#define __power_pc() (_system_configuration.architecture == POWER_PC)
#define __power_set(a) (_system_configuration.implementation & (a))
#define __power_mp() (_system_configuration.ncpus > 1)

#define __power_rs1() (_system_configuration.implementation == POWER_RS1)
#define __power_rsc() (_system_configuration.implementation == POWER_RSC)
#define __power_rs2() (_system_configuration.implementation == POWER_RS2)
#define __power_601() (_system_configuration.implementation == POWER_601)
#define __power_603() (_system_configuration.implementation == POWER_603)
#define __power_604() (_system_configuration.implementation == POWER_604)
#define __power_620() (_system_configuration.implementation == POWER_620)

#ifdef _KERNSYS
/* Defines and macros for model specific checks.  These are intended for
 * internal use only.  There is no guarantee made how these function on
 * new models
 */
#ifdef _RS6K
#define RS6K		1
#define __rs6k() (_system_configuration.model_arch == RS6K)
#else
#define __rs6k() 0
#endif

#ifdef _RSPC
#define RSPC		2
#define __rspc() (_system_configuration.model_arch == RSPC)
#else
#define __rspc() 0
#endif

#ifdef _RS6K_UP_MCA
#define RS6K_UP_MCA	1
#define __rs6k_up_mca() (_system_configuration.model_impl == RS6K_UP_MCA)
#else
#define __rs6k_up_mca() 0
#endif

#ifdef _RS6K_SMP_MCA
#define RS6K_SMP_MCA	2
#define __rs6k_smp_mca() (_system_configuration.model_impl == RS6K_SMP_MCA)
#else
#define __rs6k_smp_mca() 0
#endif

#ifdef _RSPC_UP_PCI
#define RSPC_UP_PCI	3
#define __rspc_up_pci() (_system_configuration.model_impl == RSPC_UP_PCI)
#else
#define __rspc_up_pci() 0
#endif
#endif /* _KERNSYS */

#endif /* _H_SYSTEMCFG */
