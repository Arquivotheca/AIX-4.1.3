static char sccsid[] = "@(#)29	1.4  src/bos/usr/ccs/lib/libc/POWER/mf2x2.c, libccnv, bos411, 9428A410j 6/15/90 17:53:45";
/*
 * COMPONENT_NAME: LIBCCNV multiply two fp words
 *
 * FUNCTIONS: mf2x2, mf2x1
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: mf2x2
 *                                                                    
 * FUNCTION: multiplies two fp words by 2 fp words
 *                                                                    
 * NOTES:
 *
 * void mf2x2 (hl,pq) where
 *           hl = pointer to 2 double's where hl[0] > hl[1]
 *                and hl[0] + hl[1] = entry multiplicand
 *                                  = return product
 *           pq = pointer to 2 doubles' where hl[0] > hl[1]
 *                and hl[0] + hl[1] = entry multiplier
 * RETURNS: 
 *
 */

static unsigned long inf[] = { 0x7ff00000, 0x0 };
static double zero = 0.0;

void mf2x2 (hl,pq)
double *hl, *pq;
{ 
	double a, al, b, ireg;
	ireg = * (double *) inf;     /* get inf in fp reg    */
	a = pq[0] * hl[0];           /* hi order mult        */
	if (a != ireg)               /* check if created inf */
	{ 
		al = pq[0] * hl[0] - a;     /* lo bits from hi mult */
		b = pq[0] * hl[1];          /* lo order mult        */
		hl[1] = al+(b+pq[1]*hl[0]); /* 2nd lo mult+lo bit from hi mult*/
		hl[0] = a + hl[1];    /* hi mult bits + overflow from lo mult */
		hl[1] = (a - hl[0]) + hl[1]; /* accumulate lo order bits  */
	}
	else { 
		hl[0] = a;         /* return inf     */
		hl[1] = zero;      /* low fp reg = 0 */
	}
}

/*
 * NAME: mf2x1
 *                                                                    
 * FUNCTION: multiplies two fp words by 1 fp words
 *                                                                    
 * NOTES:
 *
 *  Multiply multiplicand (2 floating point numbers)
 *                    by multiplier (1 fp number ).
 *  void mf2x1 (hl, p)
 *     hl = pointer to 2 double's where hl[0] > hl[1]
 *          and hl[0] + hl[1] = entry multiplicand
 *                            = return product
 *     p =  floating point multiplier.
 *
 * RETURNS: 
 *
 */

void mf2x1 (hl, p)
double *hl, p;
{ 
	double a, al, b, ireg;
	ireg = * (double *) inf;    /* get inf in fp reg    */
	a = p * hl[0];              /* hi order multiply    */
	if (a != ireg)              /* check if created inf */
	{ 
		al = p * hl[0] - a;     /* extended bits from hi order mul t */
		hl[1] = p * hl[1] + al; /* lo mult + lowest bits from hi mult */
		hl[0] = a + hl[1];      /* new hi = hi mult + overflow from 
					 * lo mult 
					 */
		hl[1] = ( a - hl[0]) + hl[1]; /* adjust new lo */
	}
	else { 
		hl[0] = a;         /* return inf     */
		hl[1] = zero;      /* low fp reg = 0 */
	}
}
