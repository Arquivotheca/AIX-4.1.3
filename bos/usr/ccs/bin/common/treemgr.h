/* @(#)56       1.6  src/bos/usr/ccs/bin/common/treemgr.h, cmdprog, bos411, 9428A410j 6/3/91 12:11:27 */
/*
 * COMPONENT_NAME: (CMDPROG) treemgr.h
 *
 * FUNCTIONS: TNEXT, TOOLSTR, talloc, tcheck, tfree, tfree1, tinit
 *
 * ORIGINS: 27 03 09 32 00
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT INTERNATIONAL BUSINESS MACHINES CORP. 1985, 1991
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
 * @OSF_COPYRIGHT@
 */
#include "ctools_msg.h"
#define TOOLSTR(Num, Str) catgets(catd, MS_CTOOLS, Num, Str)
nl_catd catd;

/* -------------------- tinit -------------------- */

tinit(){ /* initialize expression tree search */

        register NODE *p;

        for( p=node; p<= &node[ntrnodes-1]; ++p ) p->in.op = FREE;
        lastfree = node;

        }

# define TNEXT(p) (p== &node[ntrnodes-1]?node:p+1)

/* -------------------- talloc -------------------- */

NODE *
talloc(){
        register NODE *p, *q;

        q = lastfree;
        for( p = TNEXT(q); p!=q; p= TNEXT(p))
                if( p->in.op ==FREE ) return(lastfree=p);

        cerror(TOOLSTR(M_MSG_195,
        "out of tree space; recompile with -Ntx option with x greater than %d"),
                ntrnodes);
        /* NOTREACHED */
        }

/* -------------------- tcheck -------------------- */

tcheck(){ /* ensure that all nodes have been freed */

        register NODE *p;
        static NODE *first;

        first = node;

        if( !nerrors ) {
                for( p=first; p!= lastfree; p= TNEXT(p) ) {
                        if( p->in.op != FREE ) {
                                printf(TOOLSTR(M_MSG_196, "op: %d, val: %ld\n"), p->in.op , p->tn.lval );
                                cerror(TOOLSTR(M_MSG_197, "wasted space: %o"), p );
                                }
                        }
                first = lastfree;
                }

                /* only call tinit() if there are errors */
        else tinit();
        }

/* -------------------- tfree -------------------- */

tfree( p )  NODE *p; {
        /* free the tree p */
        extern tfree1();

        if( p->in.op != FREE ) walkf( p, tfree1 );

        }

/* -------------------- tfree1 -------------------- */

tfree1(p)  NODE *p; {
        if( p == 0 ) cerror(TOOLSTR(M_MSG_198, "freeing blank tree!"));
        else p->in.op = FREE;
        }

