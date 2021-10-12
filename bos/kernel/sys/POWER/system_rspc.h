/* @(#)82       1.7  src/bos/kernel/sys/POWER/system_rspc.h, sysios, bos41J, 9513A_all 3/23/95 17:39:57 */
/*
 * COMPONENT_NAME: (SYSML) Kernel Machine Language
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * This module contains in-line assembler functions for use by kernel.
 * These functions are for use by the base kernel only.
 */

#ifndef _H_SYSTEM_RSPC
#define _H_SYSTEM_RSPC

#ifdef _RSPC

#define ISA0_IOSTART_SF		0x80000000

/*
 * Defines for Dallas Semiconductor DS1385S RAMified Real time Clock
 */
#define RSPC_RTC_NMI		0x80

/*
 * Defines for BUC Info Information in IPL Control Block
 */

/*
 * The following is the Device Characteristic defined for a device type
 * of bus bridge.  This value is found in the device_type field of the
 * buc_info structure
 */
#define A_BUS_BRIDGE    5               /* Device Characteristics       */

/*
 * The following are the valid device IDs for device types of bus bridge.
 * These values are found in the device_id_reg field of the buc_info
 * structure
 */
#define PCI_BUS_ID      0x2010          /* PCI Bus       		*/
#define ISA_BUS_ID      0x2020		/* ISA Bus			*/
#define PCMCIA_BUS_ID   0x2030		/* PCMCIA Bus			*/
#define DMA_BUC_ID      0x2040		/* DMA Contiguous Buffer Info   */
#define HIBERNATION_BUC_ID  0x2050	/* Hibernation Wakeup Area Info */


/*
 * SIO configuration registers, from 82378IB data sheet, rev 1.0, page 25
 */
struct sio_cfg {
        ushort  vid;            /* 0x00 vendor ID                            */
        ushort  devid;          /* 0x02 device ID                            */
        ushort  cmd;            /* 0x04 command                              */
        ushort  status;         /* 0x06 device status                        */
        char    revision;       /* 0x08 revision identifier                  */
        char    res0[64-9];     /* 0x09 reserved                             */
        char    pci_ctrl;       /* 0x40 PCI control                          */
        char    pci_arbit;      /* 0x41 PCI arbiter control                  */
        char    pci_arbit_pri;  /* 0x42 PCI arbiter priority control         */
        char    res1;           /* 0x43 reserved                             */
        char    memcs_ctrl;     /* 0x44 MEMCS# control                       */
        char    memcs_bottom;   /* 0x45 MEMCS# bottom of hole                */
        char    memcs_top;      /* 0x46 MEMCS# top of hole                   */
        char    memcs_memtop;   /* 0x47 MEMCS# top of memory                 */
        char    isa_ad_ctrl;    /* 0x48 ISA AddressDecoder control           */
        char    isa_ad_rombe;   /* 0x49 ISA AddressDecoder ROM block enable  */
        char    isa_ad_bottom;  /* 0x4A ISA AddressDecoder bottom of hole    */
        char    isa_ad_top;     /* 0x4B ISA AddressDecoder top of hole       */
        char    isa_crt;        /* 0x4C ISA controller recovery timer        */
        char    isa_clk_div;    /* 0x4D ISA clock divisor                    */
        char    ubus_csela;     /* 0x4E utility bus chip select enable A     */
        char    ubus_cselb;     /* 0x4F utility bus chip select enable B     */
        char    res2[4];        /* 0x50 reserved                             */
        char    memcs_attr[3];  /* 0x54 MEMCS# attribute register #1,#2,#3   */
        char    sg_rba;         /* 0x57 scatter/gather relocation base addr  */
        char    res3[0x80-0x58];/* 0x58 reserved                             */
        ushort  bios_tba;       /* 0x80 BIOS timer base address              */
        char    res4[0x100-0x82];/*0x82 reserved                             */
};

/*
 * RSPC SIO register definitions
 */
struct rspcsio {
        char    dma1_ch0_bca;   /* 0x000 DMA1 CH0 Base and Current Address   */
        char    dma1_ch0_bcc;   /* 0x001 DMA1 CH0 Base and Current Count     */
        char    dma1_ch1_bca;   /* 0x002 DMA1 CH1 Base and Current Address   */
        char    dma1_ch1_bcc;   /* 0x003 DMA1 CH1 Base and Current Count     */
        char    dma1_ch2_bca;   /* 0x004 DMA1 CH2 Base and Current Address   */
        char    dma1_ch2_bcc;   /* 0x005 DMA1 CH2 Base and Current Count     */
        char    dma1_ch3_bca;   /* 0x006 DMA1 CH3 Base and Current Address   */
        char    dma1_ch3_bcc;   /* 0x007 DMA1 CH3 Base and Current Count     */
        char    dma1_dcom;      /* 0x008 DMA1 command/status register        */
#define dma1_ds dma1_dcom       /* 0x008 DMA1 status register                */
        char    dma1_req;       /* 0x009 DMA1 Request register               */
        char    dma1_sm;        /* 0x00A DMA1 Single Mask bit                */
        char    dma1_dcm;       /* 0x00B DMA1 Mode register                  */
        char    dma1_cbp;       /* 0x00C DMA1 Clear Byte Pointer             */
        char    dma1_mclear;    /* 0x00D DMA1 Master Clear                   */
        char    dma1_cmask;     /* 0x00E DMA1 Clear Mask                     */
        char    dma1_amr;       /* 0x00F DMA1 All Mask Register              */

        char    p0[0x20-0x10];  /* 0x010 padding                             */
        char    int1_ctrl;      /* 0x020 INT1 Control Register               */
        char    int1_mask;      /* 0x021 INT1 Mask                           */

        char    p1[0x40-0x22];  /* 0x022 padding                             */
        char    tc1_count[3];   /* 0x040 TimerCounter1 Counter[0,1,2] Count  */
        char    tc1_mode;       /* 0x043 TimerCounter1 Command Mode          */

        char    p2[0x60-0x44];  /* 0x044 padding                             */
        char    rubus_irq12;    /* 0x060 Reset UBus IRQ12                    */
        char    nmi_csr;        /* 0x061 NMI control/status                  */

        char    p3[0x70-0x62];  /* 0x062 padding                             */
	char	rtc_index;	/* 0x070 RTC Index register		     */
	char	rtc_data;	/* 0x071 RTC Data register		     */

        char    p3a[0x74-0x72]; /* 0x072 padding                             */
	char	nvram_ad_lo;	/* 0x074 NVRAM Low address register          */
	char	nvram_ad_hi;	/* 0x075 NVRAM High address register	     */
	char	nvram_data_rspc;/* 0x076 NVRAM data port polo/woodfield	     */
	char	nvram_data;	/* 0x077 NVRAM data port sandalfoot	     */

        char    p4[0x80-0x78];  /* 0x078 padding                             */
        char    dma_page_res0;  /* 0x080 DMA Page register (reserved)        */
        char    dma_page_ch2;   /* 0x081 DMA Channel 2 Page register         */
        char    dma_page_ch3;   /* 0x082 DMA Channel 3 Page register         */
        char    dma_page_ch1;   /* 0x083 DMA Channel 1 Page register         */
        char    dma_page_res1;  /* 0x084 DMA Page register (reserved)        */
        char    dma_page_res2;  /* 0x085 DMA Page register (reserved)        */
        char    dma_page_res3;  /* 0x086 DMA Page register (reserved)        */
        char    dma_page_ch0;   /* 0x087 DMA Channel 0 Page register         */
        char    dma_page_res4;  /* 0x088 DMA Page register (reserved)        */
        char    dma_page_ch6;   /* 0x089 DMA Channel 6 Page register         */
        char    dma_page_ch7;   /* 0x08A DMA Channel 7 Page register         */
        char    dma_page_ch5;   /* 0x08B DMA Channel 5 Page register         */
        char    dma_page_res5;  /* 0x08C DMA Page register (reserved)        */
        char    dma_page_res6;  /* 0x08D DMA Page register (reserved)        */
        char    dma_page_res7;  /* 0x08E DMA Page register (reserved)        */
        char    dma_lopage_ref0;/* 0x08F DMA low Page Refresh                */

        char    p5[2];          /* 0x090 padding                             */
        char    ctl_scp;        /* 0x092 CTL System Control Port             */

        char    p6[1];          /* 0x093 padding                             */
        char    dma_page_res9;  /* 0x094 DMA Page register (reserved)        */
        char    dma_page_res10; /* 0x095 DMA Page register (reserved)        */
        char    dma_page_res11; /* 0x096 DMA Page register (reserved)        */

        char    p7[1];          /* 0x097 padding                             */
        char    dma_page_res12; /* 0x098 DMA Page register (reserved)        */

        char    p8[0x9c-0x99];  /* 0x099 padding                             */
        char    dma_page_res13; /* 0x09C DMA Page register (reserved)        */
        char    dma_page_res14; /* 0x09D DMA Page register (reserved)        */
        char    dma_page_res15; /* 0x09E DMA Page register (reserved)        */
        char    dma_lopage_ref1;/* 0x09F DMA low Page Refresh                */

        char    int2_ctrl;      /* 0x0A0 INT2 Control Register               */
        char    int2_mask;      /* 0x0A1 INT2 Mask                           */

        char    p9[0xc0-0xa2];  /* 0x0A2 padding                             */
        char    dma2_ch0_bca;   /* 0x0C0 DMA2 CH0 Base and Current Address   */
        char    pad0;
        char    dma2_ch0_bcc;   /* 0x0C2 DMA2 CH0 Base and Current Count     */
        char    pad1;
        char    dma2_ch1_bca;   /* 0x0C4 DMA2 CH1 Base and Current Address   */
        char    pad2;
        char    dma2_ch1_bcc;   /* 0x0C6 DMA2 CH1 Base and Current Count     */
        char    pad3;
        char    dma2_ch2_bca;   /* 0x0C8 DMA2 CH2 Base and Current Address   */
        char    pad4;
        char    dma2_ch2_bcc;   /* 0x0CA DMA2 CH2 Base and Current Count     */
        char    pad5;
        char    dma2_ch3_bca;   /* 0x0CC DMA2 CH3 Base and Current Address   */
        char    pad6;
        char    dma2_ch3_bcc;   /* 0x0CE DMA2 CH3 Base and Current Count     */
        char    pad7;
        char    dma2_dcom;      /* 0x0D0 DMA2 command register               */
#define dma2_ds dma2_dcom       /* 0x0D0 DMA2 status register                */
        char    pad8;
        char    dma2_req;       /* 0x0D2 DMA2 Request register               */
        char    pad9;
        char    dma2_sm;        /* 0x0D4 DMA2 Single Mask bit                */
        char    pada;
        char    dma2_dcm;       /* 0x0D6 DMA2 Mode register                  */
        char    padb;
        char    dma2_cbp;       /* 0x0D8 DMA2 Clear Byte Pointer             */
        char    padc;
        char    dma2_mclear;    /* 0x0DA DMA2 Master Clear                   */
        char    padd;
        char    dma2_cmask;     /* 0x0DC DMA2 Clear Mask                     */
        char    pade;
        char    dma2_amr;       /* 0x0DE DMA2 All Mask Register              */
        char    padf;

        char    pa[0xf0-0xe0];  /* 0x0E0 padding                             */
        char    coproc_err;     /* 0x0F0 Coprocessor Error                   */

        char    pb[0x372-0xf1];
        char    sec_fddor;      /* 0x372 Secondary FloppyDiskDigitalOutputReg*/

        char    pc[0x3f2-0x373];
        char    pri_fddor;      /* 0x3F2 Primary FloppyDiskDigitalOutputReg  */

        char    pd[0x40b-0x3f3];
        char    dma1_dcem;      /* 0x40B DMA1 Extended Mode                  */

        char    pe[0x410-0x40c];
        char    dma_sg_cmd[8];  /* 0x410 DMA Scatter/Gather Command          */
        char    dma_sg_stat[8]; /* 0x418 DMA Scatter/Gather Status           */
        ulong   dma_sg_dtp[8];  /* 0x420 DMA Scatter/Gather DescriptorTblPtr */

        char    pf[0x481-0x440];
        char    dma_hipage_ch2; /* 0x481 DMA CH2 High Page register          */
        char    dma_hipage_ch3; /* 0x482 DMA CH3 High Page register          */
        char    dma_hipage_ch1; /* 0x483 DMA CH1 High Page register          */
        char    pad10[3];
        char    dma_hipage_ch0; /* 0x487 DMA CH0 High Page register          */
        char    pad11;
        char    dma_hipage_ch6; /* 0x489 DMA CH6 High Page register          */
        char    dma_hipage_ch7; /* 0x48A DMA CH7 High Page register          */
        char    dma_hipage_ch5; /* 0x48B DMA CH5 High Page register          */

        char    pg[0x4d6-0x48c];
        char    dma2_dcem;      /* 0x4D6 DMA2 Extended Mode                  */
};


/*
 * Scatter/Gather Command register bits
 */
#define SGC_EOP                 (3 << 6) /* EOP asserted when io done   */
#define SGC_IRQ13               (1 << 6) /* IRQ13 asserted when io done */
#define SGC_NOP                 (0 << 0) /* No S/G operation            */
#define SGC_START               (1 << 0) /* Start S/G command           */
#define SGC_STOP                (2 << 0) /* Stop S/G command            */

/* Define 8259 interrupt controller stuff */
/* Diagram of how the 8259s are logically connected

				 MASTER
			      +----------+
 INT line to processor   <----|          |<-- IRQ0          SLAVE
			      |          |<-- IRQ1      +----------+
			      |          |<-- IRQ2 -----|          |<-- IRQ 8
			      |          |<-- IRQ3      |          |<-- IRQ 9
			      |          |<-- IRQ4      |          |<-- IRQ 10
			      |          |<-- IRQ5      |          |<-- IRQ 11
			      |          |<-- IRQ6      |          |<-- IRQ 12
			      |          |<-- IRQ7      |          |<-- IRQ 13
			      +----------+              |          |<-- IRQ 14
							|          |<-- IRQ 15
							+----------+
 *
 * Mask data to/from a 8259 is as follows
 *

	      +-- IRQ 6 & 14	  +-- IRQ 1 & 9
	      |                   |
	+-------------------------------+
	| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
	+-------------------------------+
	  |			      |
	  +-- IRQ 7 & 15	      +-- IRQ 0 & 8
 */

/* Important addresses	*/
#define	INTA00	0x20		/* 8259	master control port address	*/
#define	INTA01	0x21		/* 8259	master mask port		*/
#define	INTB00	0xA0		/* 8259	slave control port address	*/
#define	INTB01	0xA1		/* 8259	slave mask port			*/
#define ELCR0	0x04D0		/* IRQ 0-7 Edge/Level control register	*/
#define ELCR1	0x04D1		/* IRQ 8-15 Edge/Level control register	*/

#define	RSPCINT_VECT 0xBFFFFFF0	/* Address of IVR register		*/
#define	CASCADE_IRQ	4	/* Value used to indicate which IRQ line*/
				/* on the master the slave is attached  */
				/* to.					*/

/* Initialization words	for both controllers			*/
/* ICW1								*/
/*   - ICW/OCW = 1 ICW select must be a 1 for ICW1		*/
/*   - LTIM    = 0 Edge triggered(not used on 82378ZB)		*/
/*   - ADI     = 1 Interval = 4(ingnored on 82378ZB)		*/
/*   - SNGL    = 0 Multiple 8259 controllers in the system	*/
/*   - ICA     = 1 Sending ICW4(must be a 1 on 82378ZB)		*/
#define	ICW1 0x15

/* ICW2A - Master interrupts are numbered from 0 to 7		*/
#define	ICW2A 0x00

/* ICW2B - Slave interrupts are numbered from 8 to 15		*/
#define	ICW2B 0x08

/* ICW3A  (Note	that pin 16(SP)	is logical 1 on	this chip = MASTER	*/
/*   - Slave is	on IRQ 2 (pin 20)					*/
#define	ICW3A 0x04

/* ICW3B (Note that pin	16(SP) is logical 0 on this chip = SLAVE	*/
/*   - Slave id	is 2 (See hardware book	for table)			*/
#define	ICW3B 0x02

/* ICW4									*/
/*   - SFNM = 0	 Special fully nested mode is NOT programmed		*/
/*   - BUF  = 0	 Buffered mode is NOT programmed (SP, pin 16, selects master)*/
/*   - M/S  = 0	 Ignored unless	BUF=1, which it	doesn't			*/
/*   - AEOI = 0	 Automatic end of interrupt(EOI) is NOT	programmed	*/
/*   - uPM  = 1	 MCS-86	operation is programmed	(only 1	status byte)	*/
#define	ICW4 0x01

/* OCW3	command	encodings */
#define	OCW3_GETIRR 0x0A
#define	OCW3_GETISR 0x0B

/* Generate a IMR mask(value) for 8259 IMR register */
#define	I8259_IMR_MSK( Lvl ) ((ulong)((ulong)(0x00000001) << (Lvl) ))

/* The interrupt we are	gating here is NMI.  It	generates a machine check.   */
#define	NMISC	    0x61	   /* NMI Status and Control Register	     */
#define	NMI_SBE	    0x80	   /* R/O: System board	detected error	     */
#define	NMI_SRC	    0x40	   /* R/O: IOCHK from ISA bus error	     */
#define	NMI_C2OUT   0x20	   /* R/O: Current status of TIMER2 output   */
#define	NMI_RFRSH   0x10	   /* R/O: Refresh stuff		     */
#define	NMI_SRCOK   0x08	   /* R/W: IOCHK interrupt enable	     */
#define	NMI_SBEOK   0x04	   /* R/W: System board(+PCI) interrupt	ok   */
#define	NMI_SPKOK   0x02	   /* R/W: Speaker output from TIMER2 ok     */
#define	NMI_T2ENA   0x01	   /* R/W: Enable TIMER2 counting	     */
#define	NMISC_INIT  (NMI_SRCOK | NMI_SBEOK)

#define	NMIREG	    0x70	   /* NMI Enable and Real-Time Clock Addr    */
#define	NMI_OK	    0x80	   /* NMI enable (0==enable, default==1)     */
#define	NMIREG_INIT 0x00	   /* Enable NMI interrupts		     */

#endif /* _RSPC */

#endif /* _H_SYSTEM_RSPC */
