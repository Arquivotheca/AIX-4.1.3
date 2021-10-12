/* @(#)75	1.8.1.2  src/bos/kernel/sys/POWER/m_intr.h, libsys, bos411, 9428A410j 4/21/94 10:41:36 */
#ifndef _H_M_INTR
#define _H_M_INTR
/*
 * COMPONENT_NAME: (SYSIOS) Machine dependent interrupt management defines
 *
 * FUNCTIONS: Machine dependent defines
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * THIS FILE SHOULD NOT BE EXPLICITLY INCLUDED.  IF DEFINES FROM THIS
 * FILE ARE NEEDED, THEN INCLUDE <sys/intr.h>
 */

/*
 * These are the valid interrupt priorities.
 *
 * Setting a particular priority (with the exception of INTBASE)
 * disables itself, and all less favored priorities.
 *
 * NOTE:  Any changes to the literals in this file may have to be reflected in
 *	  /sys/ml/intr.m4 or sys/ml/i_machine.m4.
 */
#define INTMAX		0		/* all interrupts disabled	 */
#define	INTEPOW		INTMAX		/* early power off warning       */
#define INTCLASS0	1		/* device class 0		 */
#define INTCLIST	INTCLASS0	/* character list services	 */
#define INTCLASS1	2		/* device class 1		 */
#define INTCLASS2	3		/* device class 2		 */
#define INTCLASS3	4		/* device class 3		 */
#define INTTIMER	5		/* timer interrupt priority	 */
#define INTOFFL0	6		/* off-level for class 0 devices */
#define INTOFFL1	7		/* off-level for class 1 devices */
#define INTOFFL2	8		/* off-level for class 2 devices */
#define INTOFFL3	9		/* off-level for class 3 devices */
#define INTOFFLVL	INTIODONE	/* lowest off-level class	 */
#define INTIODONE	10		/* block I/O services		 */
#define INTPAGER	INTIODONE	/* pager interrupt priority	 */
#define INTBASE 	11		/* everything enabled		 */

/*
 * NUM_INTR_PRIORITY defines the number of different interrupt priorities. 
 */
#define NUM_INTR_PRIORITY	(INTBASE+1)


#ifdef _POWER_RS
/*
 * These are the valid system processor interrupt levels.
 *
 * The following define the software/cpu interrupt levels.
 *
 */
#define	INT_IOCCMISC		0	   /* IOCC miscellaneous interrupt*/

#ifdef _POWER_RS2
#define INT_GIO			3	   /* GIO level on RS2G machines */
#define INT_GIO_CLS2_1		32	   /* GIO attached INTCLASS2 1st */
					   /* level			 */
#define INT_GIO_CLS3_2		58	   /* GIO attached INTCLASS3 2nd */
					   /* level			 */
#define INT_GIO_CLS3_1		59	   /* GIO attached INTCLASS3 1st */
					   /* level			 */
/*
 * Note that levels 60 & 61 are reserved for future use by GIO/601 bus
 * attached adapters.  If implemented then these definitions should be
 * implemented:
 *
 *	INT_GIO_CLS3_1		61
 *	INT_GIO_CLS3_2		60
 *	INT_GIO_CLS3_3		59
 *	INT_GIO_CLS3_4		58
 */
#endif /* _POWER_RS2 */

#ifdef _POWER_RSC
#define INT_SGA  		25	   /* SGA int level */
#endif /* _POWER_RSC */

#define	INT_SLA0		28	   /* SLA chip level */
#define	INT_SLA1		29	   /* SLA chip level */
#define	INT_SLA2		30	   /* SLA chip level */
#define	INT_SLA3		31	   /* SLA chip level */

/* Note: INT_SCUB and INT_TIMLVL must be on EIS1 for RSC.  */
#define INT_SCUB_RS1		48	   /* memory control error level */
#define INT_SCUB_RS2		 1	   /* memory control error level */
#define INT_TIMLVL		62	   /* timer level */

#endif /* _POWER_RS */

#ifdef _KERNSYS
#define	INT_EPOW_RS1		24	/* EPOW level on RS1 machines	*/
#define	INT_EPOW_RS2		2	/* EPOW level on RS2 machines	*/
#define	INT_EPOW_PPC		1	/* EPOW level on RS6K machines	*/
#define INT_EPOW_RSPC		16	/* EPOW level on RSPC machines	*/
#endif /* _KERNSYS */

/* Note INT_OFFLVL is also defined in ml/i_machine.m4 */

#define	INT_OFFLVL		63	   /* off-level handler level */

/* The following must remain for source level compatibility */
#define INT_OFFL0		INT_OFFLVL
#define INT_OFFL1		INT_OFFLVL
#define INT_OFFL2		INT_OFFLVL
#define INT_OFFL3		INT_OFFLVL

#ifdef _RS6K
#define	INT_MFRR		2	   /* MFRR level on PPC machines */
#define	INT_OFFLVL_PPC		INT_MFRR   /* off-level handler level */
#endif /* _RS6K */

/*
 * NUM_BUS_SOURCE defines the number of interrupt levels on the Micro
 * Channel.  It is also used to indicate the number of possible interrupt
 * sources on a BUC.  NUM_INTR_SOURCE is the total number of
 * interrupt levels supported by i_init.  POWER machines only use the
 * first 64 levels.
 *
 * MAX_BUID defines the maximum BUID allowed for PPC machines
 * MAX_BUC_POLL_GRP defines the maximum number of BUC groupings in the 
 * poll array.
 *
 * The value of 0x23 for MAX_BUID was chosen so the default BUIDs 0x20
 * and 0x21 for IOCCs would naturally work.
 *
 * NOTE: MAX_BUC_POLL_GRP and NUM_INTR_SOURCE have corresponding asm
 * .sets in ml/POWER/i_machine.m4
 */
#define MAX_BUID		0x23
#define MAX_BUC_POLL_GRP	10
#define NUM_BUS_SOURCE		16
#define NUM_INTR_SOURCE		(NUM_BUS_SOURCE * MAX_BUC_POLL_GRP)


/*
 *
 * This section defines the 16 possible I/O interrupts on machines using
 * cascaded 8259 interrupt controllers.  These are combined motherboard,
 * ISA bus, PCI bus, and potentially PCMCIA bus interrupts.
 *
 * The only interrupts that may be used by optional adapters are those that
 * start out INT_SFIRQ*.
 */
#ifdef _KERNSYS
#define MAX_PCINT	15	/* Maximum RSPC interrupt level		*/

#define INT_SFTIMER	 0	/* Timer interrupt level		*/
#define INT_SFCLOCK	 8	/* NIO real time clock interrupt	*/
#endif /* _KERNSYS */


#define INT_SFKBD	 1	/* Motherboard Keyboard interrupt	*/
				/* There is no IRQ 2, that is cascade int */
#define INT_SFCOM2	 3	/* Serial ports (2f8 + 2e8,238,2e0,228)	*/
#define INT_SFCOM1	 4	/* Serial ports (2f8 + 3e8,338,2e8,220)	*/
#define INT_SFIRQ5	 5	/* ISA bus IRQ 5 interrupt		*/
#define INT_SFFLOPPY	 6	/* Motherboard floppy controller interrupt*/
#define INT_SFIRQ7	 7	/* ISA bus IRQ 7 interrupt		*/
#define INT_SFPARALLEL	 7	/* Motherboard parallel port interrupt	*/
#define INT_SFIRQ9	 9	/* ISA bus IRQ 9 interrupt		*/
#define INT_SFAUDIO	10	/* Motherboard Audio subsystem interrupt*/
#define INT_SFIRQ11	11	/* ISA bus IRQ 11 interrupt		*/
#define INT_SFMOUSE	12	/* Motherboard mouse interrupt		*/
#define INT_SFSCSI	13	/* Motherboard SCSI controller interrupt*/
#define INT_SFIRQ14	14	/* ISA bus IRQ 14 interrupt		*/
#define INT_SFPCI	15	/* PCI dot or interrupt			*/

#endif /* _H_M_INTR */
