/* @(#)90	1.14.1.3  src/bos/kernel/sys/intr.h, sysios, bos41B, 9504A 12/5/94 11:03:16 */
#ifndef _H_INTR
#define _H_INTR
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Interrupt services interface definition.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file should be kept in sync with ml/intr.m4
 */

/*
 * The valid interrupt priorities are defined in i_machine.h. While the
 * actual values are machine dependent, their relative values are not.
 * Stated another way this means that if INTpri1 < INTpri2 on one
 * machine then INTpri1 <= INTpri2 on all machines.
 */
#ifndef _H_TYPES
#include <sys/types.h>
#endif /* _H_TYPES */

#ifndef _H_M_INTR
#include <sys/m_intr.h>		/* interrupt machine-dependencies */
#endif /* _H_M_INTR */

/*
 * The following interrupt handler structure is used to define
 * interrupt handlers and off-level interrupt handlers.
 * Device drivers manage the allocating of their interrupt handler
 * structures. They must remain pinned and unmodified while they are
 * defined to the kernel.  The next field must be the first in the
 * structure.
 */
struct intr                             /* interrupt handler structure */
{
	struct intr     *next;          /* list of interrupt handlers  */
	int             (*handler)();   /* interrupt handler           */
	unsigned short	bus_type;       /* bus interrupt level type    */
	unsigned short	flags;          /* miscellaneous attributes    */
	int             level;          /* bus interrupt level         */
	int             priority;       /* interrupt priority          */
	unsigned long	bid;		/* parameter for BUSACC	       */
	unsigned long	i_count;	/* interrupt counter 	       */
};

/*
 * Valid values for intr->bus_type field:
 *	(add a define for your favorite bus, as needed)
 */
#define BUS_NONE		0	/* logical interrupts		*/
					/* used by EPOW handlers 	*/
#define BUS_MICRO_CHANNEL	1	/* Interrupt src on MCA bus     */
#define BUS_PLANAR		2	/* planar interrupt source	*/
					/* used by SLA, SGA, SCU,	*/
					/* off-level HW assist handlers	*/
#define BUS_60X			3	/* interrupt source is on	*/
					/* the/a system bus		*/
#define	BUS_BID			4	/* Use bus type in bid field	*/
#define BUS_MAXTYPE		BUS_BID /* largest bus_type value       */


/*
 * Valid value for intr->flags field
 */
#define INTR_NOT_SHARED    	0x1	/* non-shared interrupt level	*/
#define EPOW_SUSPEND    	0x2	/* power loss condition		*/
#define EPOW_RESUME    		0x4	/* power resumption		*/
#define EPOW_BATTERY		0x8	/* running on battery power	*/
#define INTR_MPSAFE		0x10 	/* MP safe interrupt handler	*/
#define INTR_EDGE		0x20	/* intr is edge-triggered	*/
#define INTR_LEVEL		0x40	/* intr is level-triggered	*/
#define INTR_POLARITY		0x80	/* interrupt polarity		*/
					/* 0 = Active High or Positive Edge */
					/* 1 = Active Low or Negative Edge  */
#define I_SCHED 		0x8000	/* system uses high order bit
					 * in flags short to denote
					 * already scheduled.
					 */

#ifdef _KERNSYS
#define	INTR_FLAG_MSK		(INTR_NOT_SHARED | INTR_MPSAFE | \
				 INTR_EDGE | INTR_LEVEL | INTR_POLARITY)
#endif /* _KERNSYS */
/*
 * The following macro can be used to initialize an intr structure
 * so that an EPOW interrupt handler can be registered.
 * An EPOW interrupt handler is registered via i_init.
 */
#define	INIT_EPOW( ptr, routine, dev_bid ) 				\
{									\
	(ptr)->next = (struct intr *)NULL;				\
	(ptr)->handler = (routine);					\
	(ptr)->bus_type = BUS_NONE;					\
	(ptr)->flags = 0;						\
	(ptr)->priority = INTEPOW;					\
	(ptr)->level = 0;						\
	(ptr)->bid = (dev_bid);						\
}

/*
 * The following macro can be used to initialize an intr structure
 * so that an off-level request can be scheduled for class 0.
 * An off-level request is scheduled via i_sched.
 */
#define	INIT_OFFL0( ptr, routine, dev_bid ) 				\
{									\
	(ptr)->next = (struct intr *)NULL;				\
	(ptr)->handler = (routine);					\
	(ptr)->bus_type = BUS_NONE;					\
	(ptr)->flags = 0;						\
	(ptr)->level = INT_OFFLVL;					\
	(ptr)->priority = INTOFFL0;					\
	(ptr)->bid = (dev_bid);						\
}

/*
 * The following macro can be used to initialize an intr structure
 * so that an off-level request can be scheduled for class 1.
 * An off-level request is scheduled via i_sched.
 */
#define	INIT_OFFL1( ptr, routine, dev_bid ) 				\
{									\
	(ptr)->next = (struct intr *)NULL;				\
	(ptr)->handler = (routine);					\
	(ptr)->bus_type = BUS_NONE;					\
	(ptr)->flags = 0;						\
	(ptr)->level = INT_OFFLVL;					\
	(ptr)->priority = INTOFFL1;					\
	(ptr)->bid = (dev_bid);						\
}

/*
 * The following macro can be used to initialize an intr structure
 * so that an off-level request can be scheduled for class 2.
 * An off-level request is scheduled via i_sched.
 */
#define	INIT_OFFL2( ptr, routine, dev_bid ) 				\
{									\
	(ptr)->next = (struct intr *)NULL;				\
	(ptr)->handler = (routine);					\
	(ptr)->bus_type = BUS_NONE;					\
	(ptr)->flags = 0;						\
	(ptr)->level = INT_OFFLVL;					\
	(ptr)->priority = INTOFFL2;					\
	(ptr)->bid = (dev_bid);						\
}

/*
 * The following macro can be used to initialize an intr structure
 * so that an off-level request can be scheduled for class 3.
 * An off-level request is scheduled via i_sched.
 */
#define	INIT_OFFL3( ptr, routine, dev_bid ) 				\
{									\
	(ptr)->next = (struct intr *)NULL;				\
	(ptr)->handler = (routine);					\
	(ptr)->bus_type = BUS_NONE;					\
	(ptr)->flags = 0;						\
	(ptr)->level = INT_OFFLVL;					\
	(ptr)->priority = INTOFFL3;					\
	(ptr)->bid = (dev_bid);						\
}
 
#define INTR_SUCC          0            /* int. handler defined        */
#define INTR_FAIL         -1            /* int. handler not defined    */

/*
 * Noop i_reset for anything but the kernel 
 */
#ifndef _KERNSYS
#define i_reset( x )
#endif

/*
 * Entry points for interrupt management
 */

#ifndef _NO_PROTO

int i_disable(int newpriority);             /* set interrupt priority      */
/* arguments:
 *      int  newpriority;               priority to set
 * returns:
 *      int priority;                   returns current priority
 */

void i_enable(int priority);                /* restore interrupt priority  */
/* arguments:
 *      int  priority;                  priority to restore
 */

void i_mask(struct intr *handler);          /* disable an interrupt source */
/* arguments:
 *      struct intr *handler;           ptr to intr struct
 */

void i_unmask(struct intr *handler);    /* enable an interrupt source  */
/* arguments:
 *      struct intr *handler;           ptr to intr struct
 */

int i_init(struct intr *handler);       /* define an interrupt handler */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to define
 * returns:
 *      int rc;                         success/failure:
 */

void i_clear(struct intr *handler);     /* remove an interrupt handler */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to remove
 */

void i_sched(struct intr *handler);     /* schedule off-level processing */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to schedule
 */

#ifdef _KERNSYS
void i_reset(struct intr *handler);     /* reset interrupt */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to schedule
 */
#endif /* _KERNSYS */

#else

int i_disable();                        /* set interrupt priority      */
/* arguments:
 *      int  newpriority;               priority to set
 * returns:
 *      int priority;                   returns current priority
 */

void i_enable();                        /* restore interrupt priority  */
/* arguments:
 *      int  priority;                  priority to restore
 */

void i_mask();                          /* disable an interrupt source */
/* arguments:
 *      struct intr *handler;           ptr to intr struct
 */

void i_unmask();                        /* enable an interrupt source  */
/* arguments:
 *      struct intr *handler;           ptr to intr struct
 */

int i_init();                           /* define an interrupt handler */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to define
 * returns:
 *      int rc;                         success/failure:
 */

void i_clear();                         /* remove an interrupt handler */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to remove
 */

void i_sched();                         /* schedule off-level processing */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to schedule
 */

#ifdef _KERNSYS
void i_reset();                         /* reset interrupt */
/* arguments:
 *      struct intr *handler;           ptr to intr struct to schedule
 */
#endif /* _KERNSYS */

#endif /* not _NO_PROTO */

#endif /* _H_INTR */
