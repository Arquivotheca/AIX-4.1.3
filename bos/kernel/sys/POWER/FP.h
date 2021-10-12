/* @(#)67	1.9  src/bos/kernel/sys/POWER/FP.h, libm, bos411, 9428A410j 1/14/94 09:32:46 */
/*
 *   COMPONENT_NAME: LIBM
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SYSFP
#define _H_SYSFP

	/* exponent constants */

#define MAX_FEXPON      0x00ff
#define MAX_DEXPON      0x07ff
#define FEXPBIAS        0x7f
#define DEXPBIAS        0x3ff
#define FEXPHI          FEXPBIAS+1
#define DEXPHI          DEXPBIAS+1
#define FEXPLO          -(FEXPBIAS)
#define DEXPLO          -(DEXPBIAS)
#define DUFBIAS		1536
#define DOVBIAS		DUFBIAS
#define FUFBIAS		192
#define FOVBIAS		FUFBIAS


/********************* Other macros *******************************/

#ifdef _h_ltypes

#define StoQDNaN(X)	hipart((X)) |= DoubleStoQ
#define StoQFNaN(X)	hipart((X)) |= SingleStoQ

#endif /* _h_ltypes */

/*************************************************************************/
/*
 *  Some useful macros for dealing with variables defined as doubles
 *
 */
/*************************************************************************/

/*
 *      VALH(val)
 *
 *      Return the hipart of the double value of val as an unsigned integer.
 *      val must be a simple variable that can have its address taken.
 */

#define VALH(val) *((unsigned int *)&(val))


/*
 *      VALL(val)
 *
 *      Return the lopart of the double value of val as an unsigned integer.
 *      val must be a simple variable that can have its address taken.
 */

#define VALL(val) *(((unsigned int *)&(val))+1)

/*
 *	VAL0, VAL1, VAL2, VAL3
 *
 *	Return the various words of a double type.
 *	val must be a simple variable that can have its address taken.
 */

#define	VALSHORT(dbl)	((unsigned short *)&(dbl))

#define VAL0(val)	*(VALSHORT(val)+3)
#define VAL1(val)	*(VALSHORT(val)+2)
#define VAL2(val)	*(VALSHORT(val)+1)
#define VAL3(val)	*VALSHORT(val)

/*
 *  INTS2DBL(x,y)
 *
 *  Put two unsigned long integers into IEEE double format for the
 *  current machine architecture.
 */

#define	INTS2DBL(x,y)	(x),(y)

#ifdef __LONGDOUBLE128

/* 
 * LDVAL0, LDVAL1, LDVAL2, LDVAL3
 *
 * Return the 32-bit parts of a 128-bit long double type.
 * ld_val must be a simple variable that can have its
 * address taken.
 */

#define LDVAL0(ld_val) *((unsigned int *)&(ld_val))
#define LDVAL1(ld_val) *(((unsigned int *)&(ld_val))+1)
#define LDVAL2(ld_val) *(((unsigned int *)&(ld_val))+2)
#define LDVAL3(ld_val) *(((unsigned int *)&(ld_val))+3)

#endif /* __LONGDOUBLE128 */

#endif  /* _H_SYSFP */
