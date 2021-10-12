static char sccsid[] = "@(#)79	1.1  src/bos/usr/ccs/lib/libc/POWER/_qdbcmp.c, libccnv, bos411, 9428A410j 12/13/90 19:59:55";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _qdbcmp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: _qdbcmp
 *                                                                    
 * FUNCTION: compare quad precision numbers for dbx
 *
 * RETURNS: -1 if arg1 <  arg2
 *           1 if arg1 >  arg2
 *           0 if arg1 == arg2
 *          -2 if arg1 <undorder> arg2 
 *
 * NOTES:
 *
 * This routine does not assume "perfect" quad numbers
 * (in that the lower part of the number does not have
 * to be less that 1 ulp of the high part).  
 * Also, it assumes (a conservative assumption) that
 * a NaN can be either in the high or the low part
 * of a quad number.  It DOES assume that the
 * high order part of a quad precision number is
 * greater than the low order part (in absolute value)
 * otherwise it can return an erroneous result (but
 * so with the xlf compiler in-line code.
 */

typedef struct  {  double hi, lo;  } QUAD;

int _qdbcmp(QUAD *q1, QUAD *q2)
  {
  
  /* 
   * first check for unordered case
   * checking for NaNs in ANY position,
   * since something going wrong with
   * any other quad routine could possibly
   * generate a NaN.
   */
  
  if ((q1->hi != q1->hi) ||
      (q2->hi != q2->hi) ||
      (q1->lo != q1->lo) ||
      (q2->lo != q2->lo)) return -2;
  
  /*
   * now start checking, knowing that we have
   * eliminated NaN's.  With no NaN's this 
   * checking is pretty bullet proof.
   *
   * first, compare the high parts of
   * the two numbers.  If they compare
   * < or > then you have it, return
   * the answer.  Otherwise they compare
   * equal, in which case check the low
   * parts to establish the relationship.
   */
  
  if (q1->hi < q2->hi) return -1;
  else if (q1->hi > q2->hi) return 1;
  else if (q1->lo < q2->lo) return -1;
  else if (q1->lo > q2->lo) return 1;
  else return 0;
  }
     

