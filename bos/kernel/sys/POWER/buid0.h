/* @(#)71	1.6  src/bos/kernel/sys/POWER/buid0.h, sysml, bos411, 9428A410j 11/15/93 20:14:36 */
#ifndef _H_BUID0
#define _H_BUID0

/*
 * COMPONENT_NAME: SYSML
 *
 * FUNCTIONS: Register addresses and bit definitions in buid 0
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* BUID0 segment register value
 */
#define BUID0 0x80000000

/* Defines for BUID0 addresses that are the same on all machines
 */
#define EIM0		0x0	/* eim bits 0 - 31 */
#define EIM1		0x4	/* eim bits 32-63 */
#define EIS0		0x10	/* eis bits 0-31 */
#define EIS1		0x14	/* eis bits 32-63 */
#define PEIS0		0x10	/* peis bits 0-31 (RS2 only) */
#define PEIS1		0x14	/* peis bits 32-63 (RS2 only) */
#define DEC_EISBID 	0x20	/* decrementer eis bit */
#define MEEB_EISBID 	0x24	/* memory error eis bid */
#define EPOW_EISBID 	0x34	/* EPOW eis bid */
#define EECAR		0x1088	/* External Error Correction Address */

/* The following BUID0 address are valid only on RS1 machines
 */
#define CRE0 0x1040		/* Configuration Register Extent 0 */
#define EESR 0x1080		/* External Check Status Register */
#define EEAR 0x1084		/* External Check Address Register */
#define MESR 0x1090		/* Machine Check Status Register */
#define MEAR 0x1094		/* Machine Check Address Register */
#define DSIRR 0x1098		/* Data Storage Interrupt Reason Register */

/* The following BUID0 assresses are valid only on RSC machines
 */
#define EESR_RSC 0x0008		/* Configuration Register Extent 0 */
#define EEAR_RSC 0x000C		/* External Check Address Register */
#define MESR_RSC 0x0018		/* Machine Check Status Register */
#define MEAR_RSC 0x001C		/* Machine Check Address Register */
#define SCCR	 0x0030		/* Storage Control Register */

/* Bit in the DSIRR register
 */
#define DSIRR_IB	0x02000000	/* Invalid BUID */


/* Bits in EESR, Note the RSC and RS1 have a different format
 */
#define EESR_IOC	0x40000000	/* IO DMA error */
#define EESR_ADC	0x10000000	/* DMA address out of range */
#define EESR_UDC	0x08000000	/* Uncorrectable ECC on DMA */

#define EESR_PD		0x00800000	/* Uncor. ECC DMA address parity */
#define EESR_UD		0x00400000	/* Uncor. ECC on DMA */
#define EESR_PS		0x00200000	/* Uncor. ECC scrub address parity */
#define EESR_US		0x00100000	/* Uncor. ECC on scrub */
#define EESR_MD		0x00004000	/* DMA bus error */
#define EESR_DB		0x00000200	/* DMA byte count */
#define EESR_AD		0x00000100	/* DMA address out of range */
#define EESR_MS		0x00000080	/* Scrub memory bus error */
#define EESR_AS		0x00000010	/* Scrub address out of range */
  
/* Bits in MESR, Note the RSC and RS1 have a different format
 */
#define MESR_LSC	0x40000000	/* Processor load store */
#define MESR_AEC	0x10000000	/* Address out of range */
#define MESR_SRC	0x08000000	/* Attempted store to RSO */
#define MESR_UEC	0x04000000	/* Unroverable ECC */

#define	MESR_L2		0x08000000	/* Uncorrectable RS2L L2 cache err */
#define MESR_ME		0x00800000	/* Memory Bus Error (RAS/CAS parity) */
#define MESR_MT		0x00400000	/* Memory time out */
#define MESR_MF		0x00200000	/* Memory card failure */
#define MESR_AE		0x00100000	/* Address out of range */
#define MESR_SR		0x00080000	/* Store to ROS */
#define MESR_EA		0x00040000	/* Uncorrectable ECC address parity */
#define MESR_UE		0x00020000	/* Uncorrectable ECC */

/* Bits in the SCCR register
 */
#define SCCR_INVA7	0x20000000	/* Invert address bit 7 */
#define SCCR_INVA6	0x10000000	/* Invert address bit 6 */

#endif /* _H_BUID0 */
