/* @(#)61       1.6  src/bos/kernel/sys/POWER/sys_resource.h, sysios, bos411, 9428A410j 6/9/94 06:52:14 */
#ifndef _H_SYS_RESOURCE
#define _H_SYS_RESOURCE
/*
 * COMPONENT_NAME: (SYSIOS) I/O Subsystem
 *
 * FUNCTIONS: Architected System Resources
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef _POWER_PC

/*
 * Following is a data structure representing the entire 16MB of
 * the architected system space
 */
struct sys_resource {
        /*
         * Architected System Registers, 0xFF000000 - 0xFF0000FC
         */
        struct sys_registers {
                uint    reserved1;      /* reserved, starts at 0xFF000000*/
                uint    arbiter_cntl;   /* Arbiter Control Register */
                uint    phys_id;        /* Physical Identifier Register */
                uint    bus_slot_cfg;   /* Bus Slot Configuration Register */
                uint    bus_slot_reset[16]; /* Bus Slot Reset Registers */
                uint    reserved2[28];  /* reserved */
                uint    time_of_day[8]; /* time of day registers */
#define TOD_INDX_PPC	0		/* index for time of day index reg */ 
#define TOD_DATA_PPC	1		/* index for time of day data reg */ 
                uint    reset_status;   /* Reset Status Register */
                uint    pwr_key_status; /* Power/Keylock Status Register */
                uint    pwr_on_reset_cr; /* Power On Reset Control Register */
                uint    pwr_off_cr;	/* Power Off Control Register */
                uint    reserved3[4];   /* reserved, pad up to 0xFF000100*/
        } sys_regs;

        /*
         * Reserved Space, 0xFF000100 - 0xFF000FFF
         */
        uint    reserved1[0x3C0];

        /*
         * System Specific Registers, 0xFF001000 - 0xFF001FFF
         */
        uint 	sys_specific_regs[0x400];   /* implementation dependent */

        /*
         * Reserved Space, 0xFF002000 - 0xFF0FFFFF
         */
        uint    reserved2[0x3F800];

        /*
         * System Interrupt Registers, 0xFF100000 - 0xFF17FFFF
         */
        struct sys_interrupt_space {
                /*
                 * Interrupt Registers, 256 sets, 0xFF100000 - 0xFF107FFF
                 */
                struct sys_interrupt_regs {
                        uint    xirr_poll;      /* XIRR with no side effects */
                        union {
                                uint    xirr;   /* XIRR with side effects */
                                struct  {
                                        uchar    cppr;      /* CPPR */
                                        uchar    xisr[3];   /* XISR */
                                } xirr_u;
                        } _u;
                        uint    dsier;          /* Direct Store Interrupt Err*/
                        uchar   mfrr;           /* Most Favored Request */
			uchar	pad[3];		/* pad to next word */
                        uint    opt_mfrr[0x1C];   /* Optional MFRRs */
                } sys_intr_regs[0x100];           /* 255 possible processors */
                /*
                 * Additional Optional MFRRs, 0xFF108000 - 0xFF17FFFF
                 */
                uint    opt_mfrr[0x1E000];
        } sys_interrupt_space;

        /*
         * Reserved Space, 0xFF180000 - 0xFF1FFFFF
         */
        uint	reserved3[0x20000];

        /*
         * Standard Configuration Registers, 0xFF200000 - 0xFF200FFF
         */
        struct standard_cfg_regs {
                uint    device_charac_reg;      /* Device Characteristics Reg */
#define DEV_TYPE_MASK           0xF0000000      /* mask to get device type */
#define DEV_TYPE_NOT_READY      0x00000000      /* device present, not ready*/
#define DEV_TYPE_MEMORY         0x10000000      /* device is memory */
#define DEV_TYPE_PROCESSOR      0x20000000      /* device is processor */
#define DEV_TYPE_IO             0x30000000      /* device is I/O */
#define DEV_TYPE_NONE           0xF0000000      /* no device present */
#define DEV_CFG_INCREMENT_MASK  0x000001C0      /* mask to get data increment */
#define DEV_CFG_INCR_4          0x00000000      /* data increment is 4 bytes */
#define DEV_CFG_INCR_8          0x00000040      /* data increment is 8 bytes */
                union {
                        uchar data[0xFFC];       /* used if increment != 4 */
                        struct device_cfg {     /* used if increment == 4 */
                                uint    dev_id; /* Device ID register */
                                uint    buid[4];/* Assigned BUIDs, up to 4 */
                                uint    mem_addr0; /* memory address (control)*/
                                uint    mem_addr1; /* memory address (data) */
                                uint    reserved[0x3F8];  /* reserved space */
                        } r;
                } _u;
        } cfg_regs;

        /*
         * Device Specific Configuration Registers, 0xFF201000 - 0xFF201FFF
         */
        uint    dev_specific_cfg_regs[0x400];    /* device dependent */

        /*
         * Reserved Space, 0xFF202000 - 0xFF5FFFFF
         */
        uint    reserved4[0xFF800];

        /*
         * NVRAM space, 0xFF600000 - 0xFF7FFFFF
         */
        uint    nvram[0x80000];

        /*
         * Reserved Space, 0xFF800000 - 0xFF9FFFFF
         */
        uint    reserved5[0x80000];

        /*
         * Feature/VPD ROM Space, 0xFFA00000 - 0xFFBFFFFF
         */
        uint    feature_vpd_rom[0x80000];

        /*
         * Reserved Space, 0xFFC00000 - 0xFFDFFFFF
         */
        uint    reserved6[0x80000];

        /*
         * Boot ROM, 0xFFE00000 - 0xFFFFFFFF
         */
        uint    boot_rom[0x80000];
};

extern volatile struct sys_resource sys_resource;
extern volatile struct sys_resource *sys_resource_ptr;

#ifdef _RS6K_SMP_MCA
/*
 * Pegasus has non-conformant addresses for these
 */
extern volatile uint *rsr_addr;  /* Reset Status Register           */
extern volatile uint *pksr_addr; /* Power/Keylock Status Register   */
extern volatile uint *prcr_addr; /* Power On Reset Control Register */
extern volatile uint *spocr_addr;/* Power Off Control Register      */
#endif /* _RS6K_SMP_MCA */

#endif /* _POWER_PC */

#endif /* _H_SYS_RESOURCE */
