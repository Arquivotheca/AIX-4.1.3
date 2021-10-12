static char sccsid[] = "@(#)53  1.5  src/bos/usr/ccs/bin/common/optim.c, cmdprog, bos411, 9428A410j 6/3/91 12:04:30";
/*
 * COMPONENT_NAME: (CMDPROG) optim.c
 *
 * FUNCTIONS: LCON, LFCON, LO, LV, RCON, RO, RV, SWAP, fortarg, ispow2, nncon
 *            optim                                                           
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
 * @OSF_COPYRIGHT@
 */
/* AIWS C compiler */

# include "mfile1.h"

# define SWAP(p,q) {sp=p; p=q; q=sp;}
# define RCON(p) (p->in.right->in.op==ICON)
# define RO(p) p->in.right->in.op
# define RV(p) p->in.right->tn.lval
# define LCON(p) (p->in.left->in.op==ICON)
# define LO(p) p->in.left->in.op
# define LV(p) p->in.left->tn.lval
# define LFCON(p) (p->in.left->in.op==FCON)

int oflag = 0;

/* -------------------- fortarg -------------------- */

NODE *
fortarg( p ) NODE *p; {
        /* fortran function arguments */

        if( p->in.op == CM ){
                p->in.left = fortarg( p->in.left );
                p->in.right = fortarg( p->in.right );
                return(p);
                }

        while( ISPTR(p->in.type) ){
                p = buildtree( UNARY MUL, p, NIL );
                }
        return( optim(p) );
        }

/* -------------------- optim -------------------- */

        /* mapping relationals when the sides are reversed */
short revrel[] ={ EQ, NE, GE, GT, LE, LT, UGE, UGT, ULE, ULT };
NODE *
optim(p) register NODE *p; {
        register o, ty;
        NODE *sp;
        int i;
        TWORD t;

        /*
         * local optimizations, most of which are 
         * probably machine independent.
         * here we assume that the incoming tree has already
         * been passed thru foldexpr().
         * there is one dependency on the order of optim() and foldexpr()
         * in initialization. (a NAME is converted into an ICON ot type
         * PTR blah).
         */

        if( p == NIL )
                cerror(TOOLSTR(M_MSG_246, "nil expression to optimize" ));

        t = BTYPE(p->in.type);
        ty = optype( o=p->in.op);
        if( t == ENUMTY || t == MOETY ) 
                econvert(p);
        if( oflag ) 
                return(p);
        if( ty == LTYPE ) return(p);

        if( ty == BITYPE ) 
                p->in.right = optim(p->in.right);
        p->in.left = optim(p->in.left);

        /* collect constants */

        switch(o) {
        case SCONV:
        case PCONV:
                return( NO_FOLD()?clocal(p):p );

        case FORTCALL:
                p->in.right = fortarg( p->in.right );
                break;

        case UNARY AND:
                if( LO(p) == LNAME || LO(p) == PNAME ) return(p);
                if( LO(p) != NAME ) cerror(TOOLSTR(M_MSG_247, "& error" ));
                LO(p) = ICON;
setuleft:
                /*
                 * paint over the type of the left hand side with 
                 * the type of the top
                 */
                p->in.left->in.type = p->in.type;
                p->in.op = FREE;
                return( p->in.left );

        case UNARY MUL:
                if( LO(p) != ICON ) break;
                LO(p) = NAME;
                goto setuleft;
        case MINUS:
                if( !nncon(p->in.right) ) break;
                RV(p) = -RV(p);
                o = p->in.op = PLUS;
        case MUL:
        case PLUS:
        case AND:
        case OR:
        case ER:
                /* commutative ops; for now, just collect constants */
                /* someday, do it right */
                /*  In the meantime, it is worth noting that this next
                    line has been put into optim2 in local3.c for pass2.
                    Why the duplication?  Because it turns out that ccomq
                    can cause the below situation to occur.  The problem
                    showed up in A822.  This is not deleted because ccomq
                    might depend on pass1 doing it where possible.
                */
                if( nncon(p->in.left) || ( (LCON(p) || LFCON(p)) && !RCON(p)))
                        SWAP( p->in.left, p->in.right );

                /* make ops tower to the left, not the right */
                if( RO(p) == o ) {
                        SWAP( p->in.left, p->in.right );

#if 0
/*
 * the following code has been commented out and the above line added
 * because it doesn't make much sense and causes the problem reported
 * in PTM p26878.  This code is left in because as of 11/4/87 it is
 * thought that it may be necessary after all.  If no problems develop,
 * this commented section should be deleted.
 */
                        register op;
                        op = p->in.right->in.op;
                        sp = buildtree( op, p->in.left,
                                p->in.right->in.left );
                        p->in.right->in.op = FREE;
                        op = p->in.op;
                        p->in.op = FREE;
                        p = optim( buildtree( op, sp, p->in.right->in.right ) );
#endif
                }
                if(o == PLUS && LO(p) == MINUS && 
                        RCON(p) && RCON(p->in.left) &&
                        conval(p->in.right, MINUS, p->in.left->in.right)) {
zapleft:
                        RO(p->in.left) = FREE;
                        LO(p) = FREE;
                        p->in.left = p->in.left->in.left;
                }
                if( RCON(p) && LO(p)==o && RCON(p->in.left) &&
                    conval( p->in.right, o, p->in.left->in.right ) ) {
                        goto zapleft;
                }
                else if( LCON(p) && RCON(p) && 
                         conval( p->in.left, o, p->in.right ) ) {
zapright:
                        RO(p) = FREE;
                        p->in.left = makety( p->in.left, p->in.type );
                        p->in.op = FREE;
                        return( clocal( p->in.left ) );
                }

                /* change muls to shifts */
                if( o==MUL && nncon(p->in.right) && (i=ispow2(RV(p)))>=0) {
                        if( i == 0 ) { /* multiplication by 1 */
                                goto zapright;
                        }
                        o = p->in.op = LS;
                        p->in.right->in.type = tyalloc(INT);
                        RV(p) = i;
                }

                /* change +'s of negative consts back to - */
                if( o==PLUS && nncon(p->in.right) && RV(p)<0 ) {
                        RV(p) = -RV(p);
                        o = p->in.op = MINUS;
                }
                break;
        case DIV:
                if( nncon( p->in.right ) && p->in.right->tn.lval == 1 ) 
                        goto zapright;
                break;
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
        case ULT:
        case ULE:
        case UGT:
        case UGE:
                if( !LCON(p) && !LFCON(p) ) break;

                /* exchange operands */
                sp = p->in.left;
                p->in.left = p->in.right;
                p->in.right = sp;
                p->in.op = revrel[p->in.op - EQ ];
                break;
        case LS:
        case RS:
                if(LCON(p) && RCON(p) && conval( p->in.left, o, p->in.right )) {
                        RO(p) = FREE;
                        p->in.left = makety( p->in.left, p->in.type );
                        p->in.op = FREE;
                        return( clocal( p->in.left ) );
                }
        }
        return(p);
}

/* -------------------- ispow2 -------------------- */

ispow2( c ) CONSZ c; {
        register i;
        if( c <= 0 || (c&(c-1)) ) return(-1);
        for( i=0; c>1; ++i) c >>= 1;
        return(i);
        }

/* -------------------- nncon -------------------- */

nncon( p ) NODE *p; {
        /* is p a constant without a name */
        return( p->in.op == ICON && p->tn.rval == NONAME );
}
