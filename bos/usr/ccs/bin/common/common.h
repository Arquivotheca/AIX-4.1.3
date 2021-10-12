/* @(#)44       1.5  src/bos/usr/ccs/bin/common/common.h, cmdprog, bos411, 9428A410j 6/3/91 10:56:47 */
/*
 * COMPONENT_NAME: (CMDPROG) common.h
 *
 * FUNCTIONS: outdouble                                                      
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 *  @OSF_COPYRIGHT@
 */
# include "caloff.h"
# include "error.h"
# include "opdesc.h"
# include "treemgr.h"
# include "treewalk.h"

/* -------------------- outdouble -------------------- */

outdouble(dfval, type)
#ifdef HOSTIEEE
  double dfval;
#else
  FP_DOUBLE dfval;
#endif
  TWORD type;
{
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        union {
#ifdef HOSTIEEE
                double one;
#else
                FP_DOUBLE one;
#endif
                struct {unsigned long dfracth, dfractl;} two;
        } split;
        unsigned long longone;

        split.two.dfracth = split.two.dfractl = 0;
        split.one = dfval;
        if ( type == FLOAT) {
#ifdef HOSTIEEE
                float afloat;

                afloat = split.one;
                longone = *(long *)(&afloat);
#else
                longone = _FPd2fi(1, split.one);
#endif
                printf("\t.long\t0x%lX\n", longone);
        }
        else if (type == DOUBLE || type == LDOUBLE) {
                printf("\t.long\t0x%lX\n\t.long\t0x%lX\n",
                        split.two.dfracth, split.two.dfractl);
        }
        else cerror(TOOLSTR(M_MSG_269, "outdouble: bad arguments\n"));
#endif
}
