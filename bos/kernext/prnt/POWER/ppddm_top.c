#ifndef lint
static char sccsid[] = "@(#)40 1.4 src/bos/kernext/prnt/POWER/ppddm_top.c, sysxprnt, bos411, 9428A410j 5/3/94 17:42:47";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Printer Device Driver
 *
 *   FUNCTIONS: ppinit_dev
 *              ppopen_dev
 *		ppclose_dev
 *              pp_read_POS
 *              pp_write_POS
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/ppdd.h>
#include <errno.h>
#include <sys/lpio.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/timer.h>

#define NIONORST	0x040	/* turn off para port reset to NIO */
#define SAMNORST        0x010   /* turn off para port reset to NIO */
#define NIOINABLE       0x001   /* INABLE THE NIO CARD             */

/* Initial Value for Control Register */

#define PPCTRLINIT      PP_INIT

#ifdef DEBUG
extern ulong *pbufp;
extern ulong *pbufstart;
extern ulong pbuffer[];
extern struct ppdbinfo ppdd_db ;
#endif


/*
 * NAME: ppinit_dev
 *
 * FUNCTION:
 *      only called if pp just malloced and pp.ppddp has been initialized
 *      the rest of pp undefined at this point
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This will run in the bottom half of code and is pined.
 *      This is not in common code and is machen dependent.
 *
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS:
 *       0 - good complition
 *       ENXIO - perminit error detected on PIO
 */

int ppinit_dev(
	struct pp *pp)                   /* structure for printer pointer */
{
	int retcd = 0 ;                  /* set up return code            */
	unsigned char cardid1;
	unsigned char cardid2;

#ifdef _PP_ISA_BUS
	pp->adapter = LPAD_R1;
#else
	retcd = pp_read_POS( pp,pp->cidreg1) ;
	if (retcd == -1) {
		return (ENXIO) ;
	}
	cardid1 = (char)retcd ;
	retcd = pp_read_POS( pp, pp->cidreg2) ;
	if (retcd == -1) {
		return (ENXIO) ;
	}
	cardid2 = (char)retcd ;

	if (cardid1 == PPCARDIDIOD1  && cardid2 == PPCARDIDIOD2 )
		pp->adapter = LPAD_IOD; 
	else
		if (cardid1 == PPCARDIDR11  && cardid2 == PPCARDIDR12 )
			pp->adapter = LPAD_R1 ;
		else
			if (cardid1 == PPCARDIDSA1  && cardid2 == PPCARDIDSA2 )
				pp->adapter = LPAD_SA ;
			else {
				if (cardid1 == PPCARDIDNIO1  && cardid2 == PPCARDIDNIO2 ) {
					pp->adapter = LPAD_R1 ;
			   	/****   do not go to high proformence for current
			    	 ****   chip this is do to hardware bugs *******
					pp->adapter = LPAD_NIO ;         *******/
				} else
					return (ENXIO) ;
			}

	switch (pp->adapter) {

		case LPAD_IOD: /* IOD device     */
			retcd = pp_read_POS( pp, pp->cidreg2+1) ;
			if (retcd == -1) {
				return (ENXIO) ;
			}
			/* turn off enable output drivers */
			retcd |=  NIOINABLE ;
			retcd = pp_write_POS( pp,pp->cidreg2+1,(char)retcd);
			if (retcd) {
				return (ENXIO) ;
			}
			break ;

		case LPAD_R1:  /* one nio device */
			retcd = pp_read_POS( pp, pp->cidreg2+1) ;
			if (retcd == -1) {
				return (ENXIO) ;
			}
			/* turn off port reset and enable output drivers */
			retcd |= NIONORST | NIOINABLE ;
			retcd = pp_write_POS( pp,pp->cidreg2+1,(char)retcd);
			if (retcd) {
				return (ENXIO) ;
			}
			break ;

		case LPAD_SA: /* two NIO device with DMA */

			retcd = pp_read_POS( pp, pp->cidreg2+1) ;
			if (retcd == -1) {
				return (ENXIO) ;
			}
			retcd &= ~SAMNORST;  /* turn off port reset if on */
			retcd |= NIOINABLE;  /* inable output drivers     */
			retcd = pp_write_POS( pp, pp->cidreg2+1, (char)retcd) ;
			if (retcd) {
				return (ENXIO) ;
			}
	}
#endif	/* _PP_ISA_BUS */

	/* Select Printer */
	retcd = ppwritectrl(pp, (char)PPCTRLINIT);
	if (retcd == -1)
		return (ENXIO);

	/* read status to ensure intr bit is clear */
	retcd = ppreadstat(pp) ;
	if (retcd == -1)
		return (ENXIO);

	return(0);
} /* end ppinit_dev */


/*
 * NAME: ppopen_dev
 *
 * FUNCTION: hardware dependent code when opening device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      will execute in process level but is ran only at open time
 *
 * RETURNS: if interrupt handler can not be set up (EBUSY) returned
 *
 */

int ppopen_dev(
	struct pp *pp)
{
	int retcd ;

#ifdef DEBUG
	extern struct ppdbinfo ppdd_db;
	ppdd_db.hits = 0;
	ppdd_db.misses = 0;
	ppdd_db.waitmax = 0;
	ppdd_db.waitmin = 0xffffff;
	ppdd_db.bflagmax = 0;
	ppdd_db.waitavg = 0;
	ppdd_db.countw = 0;
	pbufstart = pbuffer;
	pbufp = pbufstart;
#endif
	if (NULL == (pp->tmr = talloc()))
		return(ENOMEM);
	retcd = ppwritectrl(pp, PPCTRLINIT) ;
	if (retcd)
		return (ENXIO) ;
	return (0) ;
}

/*
 * NAME: ppclose_dev
 *
 * FUNCTION: hardware dependent code when closing device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      will execute in process level but is ran only at close time
 *
 * RETURNS: none
 *
 */

int ppclose_dev(
	struct pp *pp)
{
	tfree(pp->tmr);
	/* disable interrupts on card */
	(void) ppwritectrl(pp, PPCTRLINIT);

	/* read status to ensure intr bit is clear */
	(void) ppreadstat(pp) ;

	return(0);
} /* end ppclose_dev */


#ifndef _PP_ISA_BUS
/*
 * NAME: pp_read_POS
 *
 * FUNCTION: This is the section that will read the pos regs
 *
 * EXECUTION ENVIRONMENT:
 *
 *      will execute in process level
 *
 * RETURNS: none
 *
 */

int pp_read_POS(
struct pp *pp ,
uint      offset)
{
	int count = 0;                          /* loop count               */
	int    val1,                            /* data location   one      */
	       val2;                            /* data location   two      */
	caddr_t iocc_addr;

	iocc_addr = IOCC_ATT(pp->bus_id,0) ;
	val2 = (int)BUSIO_GETC(iocc_addr + offset) ;
	do {
		val1 = val2 ;
		val2 = (int)BUSIO_GETC(iocc_addr + offset) ;
	} while (val1 != val2 && count++ < 5) ;

	IOCC_DET(iocc_addr) ;
	if (count >= 5)
		return (-1) ;
	return ( val1 ) ;
}


/*
 * NAME: pp_write_POS
 *
 * FUNCTION: This is the section that will write to the pos regs
 *
 * EXECUTION ENVIRONMENT:
 *
 *      will execute in process level
 *
 * RETURNS: none
 *
 */

int pp_write_POS(
struct pp *pp ,
uint      offset,
uchar     data)
{
	int count = 0;                          /* loop count               */
	uchar  val1;                            /* data location   one      */
	caddr_t iocc_addr;

	iocc_addr = IOCC_ATT(pp->bus_id,0) ;
	do {
		BUSIO_PUTC(iocc_addr + offset, data) ;
		val1 = BUSIO_GETC(iocc_addr + offset) ;
	} while (val1 != data && count++ < 5) ;
	IOCC_DET(iocc_addr) ;
	if (count >= 5)
		return (-1) ;
	return (0) ;
}
#endif	/* _PP_ISA_BUS */
