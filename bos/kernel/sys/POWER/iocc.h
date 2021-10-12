/* @(#)73	1.17.1.9  src/bos/kernel/sys/POWER/iocc.h, sysios, bos41B, 412_41B_sync 1/6/95 13:15:02 */
/*
 * COMPONENT_NAME: (SYSIOS) IO Channel Converter Harware Definitions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_IOCC
#define _H_IOCC

#ifndef _H_TYPES
#include <sys/types.h>
#endif /* _H_TYPES */

/*
 * Bit definitions that are common to all architectures and implementations
 * unless otherwise noted.
 */

/*
 * Interrupt Request Register - int_request
 */
#define	IRR_MISC	0x80000000	/* miscellaneous interrupt	     */
#define	IRR_KBD		0x40000000	/* keyboard interrupt		     */
#define	IRR_SERIAL	0x20000000	/* serial port interrupt	     */
#define	IRR_CHECK_F1	0x00800000	/* RESERVED(old PC AT channel check) */
#define	IRR_PARALLEL	0x00040000	/* parallel port interrupt	     */

/*
 * Miscellaneous Interrupt Register - int_misc
 */
#define	IMI_CHANCK	0x80000000	/* channel check		     */
#define	IMI_TIMEOUT	0x40000000	/* bus timeout			     */
#define	IMI_KBDEXT	0x20000000	/* ctl-alt key sequence		(PWR)*/
#define	IMI_BUMP	0x20000000	/* Service processor interrupt	(PPC)*/

/* 
 * This is the main IOCC register file mapping for the POWER architecture.
 * Accesses must be done with the IOCC register file access bit set in 
 * the segreg. See IOCCACC in sys/ioacc.h.
 */

#define IOCC0_BID	0x820C00E0	/* IOCC 0 segment register value */
#define IOCC1_BID	0x821C00E0	/* IOCC 1 segment register value */
#define IOCC2_BID	0x821C00E0	/* IOCC 2 segment register value */
#define IOCC3_BID	0x821C00E0	/* IOCC 3 segment register value */
#define IOCC_BID	IOCC0_BID	/* From when there was only 1 IOCC */
#define	IO_IOCC		0x00400000	/* base address of register file */

struct iocc				/* interrupt handler structure */
{
	char		reserved1[16];	/* reserved */

	unsigned long	config;		/* IOCC config register */
#define	ICF_ENABLE	0x80000000	/* master enable */
					/* bits 2-3=0->finish cycle(burst)*/
#define	ICF_BURST16	0x10000000	/* burst control - 1.6us */
#define	ICF_BURST32	0x20000000	/* burst control - 3.2us */
#define	ICF_BURST64	0x30000000	/* burst control - 6.4us */
#define	ICF_DISCK	0x08000000	/* disable CHCK on parity error */
#define	ICF_DISSD	0x04000000	/* disable streaming data protocol */
					/* bits 6-7 = 0 -> disable refresh */
#define	ICF_REFR60	0x01000000	/* refresh rate 60us */
#define	ICF_REFR30	0x02000000	/* refresh rate 30us */
#define	ICF_REFR15	0x03000000	/* refresh rate 15us */
					/* 128K RAM bits 9-11 = 0 */
#define	ICF_RAM256K	0x00100000	/* 256K RAM on IOCC */
#define	ICF_RAM512K	0x00200000	/* 512K RAM on IOCC */
#define	ICF_RAM1M	0x00300000	/* 1M RAM on IOCC */
#define	ICF_RAM2M	0x00400000	/* 2M RAM on IOCC */
#define	ICF_RAM4M	0x00500000	/* 4M RAM on IOCC */
#define	ICF_RAM8M	0x00600000	/* 8M RAM on IOCC */
#define	ICF_RAM16M	0x00700000	/* 16M RAM on IOCC */
#define ICF_RAM_MASK	0x00700000	/* mask for RAM bits */
#define ICF_RAM_SHIFT	20		/* shift for RAM bits */
					/* arb time = 100ns bits12-15=0 */
					/* ((bits12-15)+1)*100 = arbt in ns*/
#define	ICF_ARB200	0x00010000	/* arbitration time = 200ns */
#define	ICF_ARB300	0x00020000	/* arbitration time = 300ns */
#define	ICF_ARB1600	0x000F0000	/* arbitration time = 1600ns */
#define ICF_DUAL_BUF	0x00000080	/* set if dual buffer support */
#define ICF_SLAVE_TCW	0x00000040	/* mask for slave with TCWs */
#define ICF_BUF_MSK	0x00000030	/* mask for buffer/coherency */
#define ICF_SLVCHN_MSK	0x0000000F	/* number of DMA slave channels */

	char		reserved2[12];	/* reserved */
	unsigned long	bus_status;	/* bus status register */
	char		reserved3[8];	/* reserved */
	unsigned long	c_reset_reg;	/* component reset register */
	char		reserved4[16];	/* reserved */
	unsigned long	limit;		/* l/st limit register */
	char		reserved[28];	/* reserved */
	char		reserved_dma[32]; /* dma register address space */
	unsigned long	int_enable;	/* interrupt enable reg */
	unsigned long	int_request;	/* interrupt request reg */
	unsigned long	int_misc;	/* misc interrupt reg */
	unsigned long	rfi;		/* return from interrupt */
	unsigned long	vector[4];	/* interrupt vector table */
};



/*
 * The following can be used to convert the iocc.config value into
 * the number of TCWs supported on this machine.
 */
#define ICF_TCWS(c) ( ((c) & ICF_SLAVE_TCW) ?				\
	((0x8000 << (((c) & ICF_RAM_MASK) >> ICF_RAM_SHIFT)) >> 2) :	\
	(((0x20000 << (((c) & ICF_RAM_MASK) >> ICF_RAM_SHIFT)) >> 2) - 0x2000))

#define ICF_SLV_TCWS(c) (((c) & ICF_SLAVE_TCW) ? (256*ICF_NUM_SLVCHN(c)) : 0)

#define ICF_NUM_TCWS(c) (ICF_TCWS(c) - ICF_SLV_TCWS(c))

/*
 * The following can be used to check for bufferd IOCCs
 */
#define ICF_BUFFERED(c) \
	(((c) & ICF_BUF_MSK) != 0x00000010)

/*
 * The following can be used to convert the iocc.config value to the number
 * of slave channeds supported
 */
#define ICF_NUM_SLVCHN(c) \
	((((c) & ICF_SLVCHN_MSK) == 0) ? 0xF : (c) & ICF_SLVCHN_MSK)

/*
 * Address of the IOCC delay command.  For delay use store byte to
 * IOCC_DELAY + (number of micro seconds - 1)
 */
#define IOCC_DELAY 0xE0


/*
 * This is the card config register.
 * It is 2 words long, with support for up to 16 cards.
 * Accesses to the card config reg are 8-bits at a time, and must
 * be done with the IOCC register file access bit set in the segreg.
 * (> 8 bit accesses may be done from the CPU; however the IOCC
 *  will break them down).
 */
 
#define	IO_CONFIG	0x00400000	/* offset into IOCC of card config */
#define	IO_CONFIG_SLOT	0x000F0000	/* slot select for card config */

struct card_config			/* card config data */
{
	char		card_idL;	/* card id - low byte */
	char		card_idM;	/* card id - high byte */
	char		dev1;		/* device specific */
	char		dev2;		/* device specific */
	char		dev3;		/* device specific */
	char		dev4;		/* device specific */
	char		subaddr1;	/* sub-addressing */
	char		subaddr2;	/* sub-addressing */
};


/*
 * The Micro Channel to PC AT bus converter is accessed as a
 * card configuration register.
 */

#define	F1_CONFIG	0x00470000	/* base address */

#define	f1_modes	dev1		/* converter modes */
#define F1_DIAG		0x02		/* diagnostic mode */
#define	F1_ENABLE	0x01		/* converter enable */

#define	f1_dmamask	dev2		/* DMA channel masks */
#define f1_dmafair	dev3		/* DMA fairness */

#define	f1_eoi		dev4		/* end of interrupt */
#define	F1_CCK		0x80		/* convert parity error */

#define f1_intmaskH	subaddr2	/* mask bus lvls 15-8 */
#define	f1_intmaskL	subaddr1	/* mask bus lvls 7-0 */

/*  
 *  Labels used for the power status register
 *  in the IOCC.
 */

#define	EPOW_MASK	0xF0000000	/* mask off all but EPOW bits */
#define	PSR_ADDRESS	0x004000E4	/* address of power status register */
#define	PCRR_ADDRESS	0x004000E8	/* addr of power ctrl reset register */
#define	NORMAL_STATE	0x00000000	/* normal state for power supply */
#define	BATTERY		0x10000000	/* running on battery bit of PSR */
#define	LOPP		0x80000000	/* loss of Primary Power */
#define	THERMAL_WARNING	0x50000000	/* abnormally high temperature */
#define	PWR_OVERLOAD	0x70000000	/* power supply overload */
#define	PWR_FAN_FAULT	0x90000000	/* power supply fan failure */
#define	KEY_POS_MASK	0x00000003	/* mask off all but key position bits */
#define KEY_POS_NORMAL	0x03		/* key is in the normal position */
#define KEY_POS_SECURE	0x01		/* key is in the secure position */
#define KEY_POS_SERVICE	0x02		/* key is in the service position */
#define BEHAVIOR_SHIFT	18		/* behavior bits are bits 10-13 */
#define BEHAVIOR_MASK	0xF		/* mask off all but behavior bits */
#define USE_EPOW_BITS	0x0		/* interpret bits 0-9 of PKSR */
#define WARN_COOLING	0x1		/* no action "warning cooling problem"*/
#define WARN_POWER	0x2		/* no action "warning power problem" */
#define SLOW_SHUTDOWN	0x3		/* 10 minutes to shut down */
#define FAST_SHUTDOWN	0x4		/* 20 seconds to shut down */
#define IMMED_SHUTDOWN	0x5		/* immediate power down condition */
#define IMMEDX_SHUTDOWN	0x6		/* immediate power down for expansion */

/*
 * Labels define the range of addresses that contain iocc commands
 * and control registers
 */
#define IOCC_COMMAND_START	0x00400000
#define IOCC_COMMAND_END	0x004fffff

/*
 * IOCC segment register bits
 */
#define IOCCSR_KEY	0x40000000	/* Privlege key */
#define IOCCSR_SELECT	0x00000080	/* IOCC space select */

/* ----------------------------------------------------------------------- */

/* 
 * This is the main IOCC register file mapping for the PowerPC architecture.
 * Accesses must be done with the IOCC register file access bit set in 
 * the segreg. See IOCCACC in sys/ioacc.h.
 */

/*
 * PowerPC IOCC definitions and macros
 */
#define IOCC_SEGREG	0x80000000	/* IOCC segment register value	 */
#define	IO_IOCC_PPC	0x10000		/* base address of IOCC space 	 */
#define IOCC_CFG_REG    0x80     	/* offset to config register     */
#define IOCC_TCE_ADDR   0x9C     	/* offset to TCE Anchor (32-bit) */
#define MAX_NUM_IOCCS   2		/* maximum supported IOCCs       */

/*
 * PowerPC IOCC Implementations
 */
#define IOCC_601_BOX	0x300000	/* device id of 601 box IOCC */

/*
 * BUID definitions and macros
 */
#define	BUID_SEGREG_MASK	0x1FF00000
#define BUID_TO_IOCC( Buid )	( IOCC_SEGREG | (uint)((uint)Buid << 20))


/*
 * Extract Number of TCEs
 *	Note: This is the TOTAL number of TCE's for this IOCC.
 *	      For calculation of the number of DMA Master TCE's, the TCEs
 *	      reserved for DMA Slave devices must be subtracted from
 *	      this total. 
 *
 *	c = contents of the TCE Address Register
 */
#define NUM_TCES_PPC(c) ( 0x2000 << ((c) & 0x7))

/*
 * Extract Number of Slave Channels
 *
 *	c = contents of the IOCC Config Register
 */
#define NUM_SLVCHN_PPC(c) ((c) & 0xF)

/*
 * Compute the Number of Slave TCEs
 *
 *	c = contents of the IOCC Config Register
 */
#define SLV_TCES_PPC(c) (256 * NUM_SLVCHN_PPC(c))

struct iocc_ppc {
        char    reserved1[128];         /* 0-7F, reserved */
        ulong   config;                 /* 80, iocc config register */
#define ICF_ENABLE 0x80000000		/*     master enable */
#define ICF_64_bit 0x80			/*     64-bit enable */
#define ICF_MAXTCE_MSK 0x70		/*     mask for max TCEs */

        ulong   reserved2;              /* 84, reserved */
        ulong   personalized;           /* 88, iocc personalization */
        ulong   reserved3;              /* 8C, reserved */
        ulong   bus_status;             /* 90, iocc bus status */
        ulong   reserved4;              /* 94, reserved */
        ulong   tce_addr_high;		/* 98, MSW of tceaddr */
        ulong   tce_addr_low;		/* 9C, LSW of tceaddr */
        ulong   c_reset_reg;            /* A0, Component reset reg */
        ulong   reserved5;              /* A4, reserved */
        ulong   bus_map_reg;            /* A8, Bus mapping register */
        char    reserved6[212];         /* AC-17F, reserved */
        ulong   int_enable;             /* 180, interrupt enable */
        ulong   reserved7;              /* 184, reserved */
        ulong   int_request;            /* 188, interrupt request reg */
        ulong   reserved8;              /* 18C, reserved */
        ulong   int_misc;               /* 190, misc interrupt reg */
        char    reserved9[44];          /* 194-1BF, reserved */
        ulong   avail_procs[8];         /* 1C0-1DC, available procs */
        char    reserved10[32];         /* 1E0-1FF, reserved */
        ulong   xivr[16];               /* 200-23C, external intr vec */
        char    reserved11[320];        /* 240-37F, reserved */
        ulong   dma_slv_cntrl[8];       /* 380-39C, slave control regs*/
        char    reserved12[96];         /* 3A0-3FF, reserved */
        ulong   csr[16];                /* 400-43C, channel stat regs*/
        char    reserved13[64];         /* 340-37F, reserved */
        ulong   eoi[16];                /* 480-4BC, external intr vec */
        char    reserved14[64];         /* 4C0-4FF, reserved */
        ulong   ed_arblvl[16];          /* 500-53C, channel arb level en/dis */
};

#endif /* _H_IOCC */
