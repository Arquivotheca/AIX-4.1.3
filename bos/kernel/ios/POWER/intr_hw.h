/* @(#)57	1.22  src/bos/kernel/ios/POWER/intr_hw.h, sysios, bos411, 9428A410j 4/15/94 17:13:41 */
#ifndef _h_INTR_HW
#define _h_INTR_HW
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Internal machine dependent macros and labels used by
 *	      the interrupt management services.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ERRIDS
#include <sys/errids.h>
#endif

#ifndef _H_BUID0
#include <sys/buid0.h>
#endif

#ifndef _H_INTR
#include <sys/intr.h>
#endif

#ifndef _H_ADSPACE
#include <sys/adspace.h>
#endif

#ifndef _H_IOCC
#include <sys/iocc.h>
#endif

#ifndef _H_SYSTEMCFG
#include <sys/systemcfg.h>
#endif

#ifdef _POWER_RS
#include "intr_hw_pwr.h"
#endif /* _POWER_RS */

#ifdef _RS6K
#include "intr_hw_ppc.h"
#endif /* _RS6K */

/* Bus segment register value for accessing system level registers */
#define	EICRBID			BUID0

/* Creates a mask for IOCC interrupt enable register */
#define IOCC_IER_MSK( Lvl )	((ulong)((ulong)(0x80000000) >> (Lvl)))

/*
 * Structures used by various error logging code in i_misc*
 */
struct miscerr {                /* error log description for misc. interrupt */
        struct  err_rec0 derr;
	ulong    iocc;		/* indicate IOCC number */
	ulong    bsr;		/* bus status register  */
	ulong    misc;		/* misc. interrupt register  */
};

struct scuerr {                /* error log description for mem interrupts */
        struct  err_rec0 e;
        ulong    estat;		/* external int stat reg */
	ulong	 eear_adr;	/* failing address       */
	ulong	 eecar_adr;	/* failing address       */
	int  	 creg[8];	/* mem config regs       */
};

struct epowerr {                /* error log description for EPOW interrupts */
        struct  err_rec0 derr;
        ulong    psr;		/* power status register */
}; 

/*
 *  Values for the epow_status kernel flag.
 */
#define RESUME          0x0             /* normal power supply */
#define SUSPEND         0x1             /* power supply loss */
#define SUSPEND_BAT     0x2             /* running on battery power supply */

/*
 *  Values for the epow func_data field.
 */
#define FROM_FLIH       0x0             /* indicates timer started by flih */
#define FROM_TRB        0x1             /* indicates timer started by timer */

#endif /* _h_INTR_HW */
