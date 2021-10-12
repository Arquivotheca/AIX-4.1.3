/* @(#)11	1.14  src/bos/kernel/ios/interrupt.h, sysios, bos41J, 9522A_a 5/30/95 12:53:55 */
#ifndef _h_INTERRUPT
#define _h_INTERRUPT
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Internal data structures
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * This file must be kept in sync with ml/interrupt.m4
 */

#ifndef _H_LOCKL
#include <sys/lockl.h>
#endif /* _H_LOCKL */
#include <sys/lock_def.h>
#ifndef _H_SYSTEMCFG
#include <sys/systemcfg.h>
#endif /* _H_SYSTEMCFG */
#ifndef _H_INTR
#include <sys/intr.h>
#endif /* _H_INTR */


/*
 * Flags used by hardware independent layer to tell dependent layer 
 * what to do.
 */

/* Flags to i_adjustplvl_?, MP implementations only */

#define	INTR_UNFUNNEL	0	/* Adjust routing hardware to allow any
				 * processor to service the interrupt */
#define	INTR_FUNNEL	1	/* Adjust routing hardware to funnel the
				 * source to the master processor only */

#define	LOCAL_CPU	-2	/* i_soft: interrupt ourselves */

/*
 * The following were introduced for the RSPC platforms.  They exist,
 * unused, on other system types.  Valid choices are given below.
 * These are initialized from initiocc.c, and constant/non-volatile
 * thereafter.
 */
extern unsigned short		 intctl_pri;	/* Type of the primary
						 * interrupt controller */
extern unsigned short		 intctl_sec;	/* Type of the secondary
						 * interrupt controller */

#define	INT_CTL_NONE	((unsigned short)0x0000) /* Controller absent  */
#define	INT_CTL_RS6K	((unsigned short)0x0001) /* PowerPC Compliant  */
#define	INT_CTL_8259	((unsigned short)0x0002) /* Cascaded 8259s     */
#define	INT_CTL_MPIC	((unsigned short)0x0004) /* MPIC */

/*
 * NOTE: The imask struct is also used to hold RS2 PEIS values
 */
#ifdef _POWER_RS
struct imask {
	unsigned long eim0;		/* Bits 0-31 of mask */
	unsigned long eim1;		/* Bits 32-63 of mask */
};
#endif

/*
 * The interrupt handler data is combined into one structure to reduce
 * the number of base registers required to access the data.
 */
struct i_data
{            
	/*
	 * RS1 processors
	 * The eim is the external interrupt mask. There is one external
	 * interrupt mask for each interrupt priority. Each mask contains
	 * two words for EIM0 and EIM1.  A 0-bit in the eim indicates 
	 * that the corresponding interrupt level is disabled
	 * at that interrupt priority. An unmasked interrupt level will
	 * have a 0 in its eim bit for all interrupt priorities that
	 * are equal or more favored than its interrupt priority. It
	 * will have a 1 for all interrupt priorities with a less favored
	 * interrupt priority. The eim (per priority) is used to mask off
	 * all interrupt levels with priorities equal to or less favored
	 * than the specified one. The eim for INTMAX is always
	 * all 0s. The eim of INTBASE has a 1 for all defined and unmasked
	 * interrupt levels and 0s otherwise.
	 *
	 * RS2 processors
	 * On RS2 processors the hardware priority is a one to one relation-
	 * ship with the 64 interrupt levels.  The cil array is a numeric
	 * value, for each interrupt priority, that indicates at what 
	 * level interrupts are disabled.
	 *
	 * PPC processors - N/A
	 *
	 * Since the priority mapping union is only used on RS1/RS2 machines
	 * it can be updated with the only serialization being disabled
	 * to INTMAX.
	 *
	 * The priority mapping union MUST be the first thing in i_data.
	 * Select dispatcher assembler routines assume that it is.
	 *
	 */
#ifdef _POWER_RS
	union {
		struct imask eim[NUM_INTR_PRIORITY];
		char	cil[NUM_INTR_PRIORITY];
	} i_pri_map;
#endif

	/*
	 * The poll ptrs anchor the "intr" structures for each interrupt level.
	 * The priority field, which use to be in this structure has been
	 * removed.  It can be found in the first "intr" structure on
	 * the list.  It indicates the priority of each interrupt level. 
	 * Insertion & deletion from this list may only be done from a process,
	 * and the caller must hold the intsys_lock.
	 * 
	 * Only ONE priority is allowed per interrupt level.
	 *
	 * The poll list is singly linked, threaded through the intr "next"
	 * field in the intr struct, and terminates with NULL.
	 */
	struct i_poll {
		struct intr	*poll;		/* next interrupt handler */
	} i_poll[NUM_INTR_SOURCE];
};

/*
 * Macros to get access in exclusion mode to the ipoll array
 */
#ifdef _POWER_MP
#define GET_POLL_LOCK(oPri,iPri,iLev) \
		ipoll_excl_wait(); \
		(oPri) = i_disable( (iPri) )
#else
#define GET_POLL_LOCK(oPri,iPri,iLev) \
		((oPri) = i_disable( (iPri) ))
#endif


#ifdef _POWER_MP
#define REL_POLL_LOCK(oPri,iLev) \
		ipoll_excl_clear(); \
		i_enable( (oPri) )
#else
#define REL_POLL_LOCK(oPri,iLev) \
		(i_enable( (oPri) ))
#endif

/*
 * PRI_MASK( priority )
 *
 * Given an offlevel priority, generate a mask with the bit so that
 * i_softis/i_softpri can be set.
 */

#define PRI_MASK( pri )			((ushort)0x8000 >> (pri))
#define OFFLVL_MASK	(INTTIMER | INTOFFL0 | INTOFFL1 |	\
			 INTOFFL2 | INTOFFL3 | INTIODONE)


/* Generate a i_prilvl[] bit mask */
#define PRILVL_MSK( Idx )	((ulong)((ulong)(0x80000000) >> (Idx)))

/*
 * SET_SOFT_INT( priority )
 *
 * Sets the appropriate bit in i_softis/i_softpri for the given priority
 * and if needed sets the hardware assist.
 */
#ifdef _POWER_MP
#define	I_SOFT( lvl, cpu )	i_soft( (lvl), (cpu) )
#else
#define	I_SOFT( lvl, cpu )	i_soft( )
#endif

#define SET_SOFT_INT( pri )						\
{									\
	if( soft_int_model ) {						\
		ppda_ptr->i_softpri |= PRI_MASK( pri );			\
	}								\
	else {								\
		register	ushort o_softis;			\
		ASSERT( (pri >= INTTIMER) && (pri <= INTIODONE) );	\
		o_softis = ppda_ptr->i_softis;				\
		ppda_ptr->i_softis |= PRI_MASK( pri );			\
		if( !o_softis && o_softis != ppda_ptr->i_softis ) {	\
			I_SOFT( INTOFFLVL, LOCAL_CPU );			\
		}							\
	}								\
}

/*
 * CLR_SOFT_INT()
 *
 * Clears the appropriate bit in i_softis for the given priority
 * and if needed clears the hardware assist
 */
#define CLR_SOFT_INT( )				\
{						\
	ASSERT( ppda_ptr->i_softis == 0 );	\
	i_reset_soft();				\
}

/*
 * RESET_SOFTLVL( pri )
 * RESET_SOFTPRI( pri )
 *
 * Reset the appropriate bit in i_softis/i_softpri for the given priority
 */
#define RESET_SOFTLVL( pri )				\
{							\
	ppda_ptr->i_softis &= ~PRI_MASK( pri );		\
}
#define RESET_SOFTPRI( pri )				\
{							\
	ppda_ptr->i_softpri &= ~PRI_MASK( pri );	\
}


#define	MAX_LVL_PER_PRI	32	/* Must be changed in 32 bit chunks */
#define	LVLPRI_LEN	(MAX_LVL_PER_PRI / 32 )

#endif /* _h_INTERRUPT */
