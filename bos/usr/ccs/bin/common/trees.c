static char     sccsid[] = "@(#)57      1.27.1.2  src/bos/usr/ccs/bin/common/trees.c, cmdprog, bos411, 9428A410j 10/7/93 10:48:24";
/*
 * COMPONENT_NAME: (CMDPROG) trees.c
 *
 * FUNCTIONS: DBprint, FieldBust, WarnMerge, WarnWalk, andable, assary, bcon 
 *            block, bpsize, buildtree, chkstr, contx, conval, convert        
 *            cvtarg, doszof, ecomp, econvert, eprint, fconval, fixargs       
 *            foldexpr, icons, makety, moditype, notlval, nullptr, oconvert   
 *            opact, p2tree, pconvert, prmtint, prtdcon, prtree, psize        
 *            ptmatch, strargs, stref, tymatch                                
 *
 * ORIGINS: 27 3 9 32
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 *
 * Modified: May 91 RWaters, ILS changes
 * Modified: June 91 RWaters, P23321 treat void functions as returning void
 */

/* AIWS C compiler */

#include "mfile1.h"
#include "messages.h"

/* corrections when in violation of lint */

/*      some special actions, used in finding the type of nodes */
#define NCVT            01
#define NCVTR           02
#define TYPL            04
#define TYPR            010
#define TYMATCH 040
#define LVAL            0100
#define CVTO            0200
#define CVTL            0400
#define CVTR            01000
#define PTMATCH 02000
#define OTHER           04000

/* node conventions:

        NAME:   rval>0 is stab index for external
                rval<0 is -inlabel number
                lval is offset in bits
        ICON:   lval has the value
                rval has the STAB index, or - label number,
                        if a name whose address is in the constant
                rval = NONAME means no name
        REG:    rval is reg. identification cookie
        LNAME:  lval is offset
                rval is unique id number
        PNAME:  lval is offset
                rval is unique id number

        */

int     bdebug = 0;
#ifndef ddebug
extern ddebug;
#endif
extern xdebug;

extern nofpflding;

/* -------------------- buildtree -------------------- */

NODE *
buildtree( o, l, r )
int     o;
register NODE *l;
register NODE *r;
{
        register NODE *p, *q;
        register actions;
        register opty;
        register struct symtab *sp;
        register NODE *lr, *ll;
        int     i, const_flag;
        register PPTR parmlist;
        extern int      eprint();
        TPTR curr_type;

#ifndef BUG1
        if ( bdebug )
                printf( "buildtree( %s, %d, %d )\n", opst[o], l, r );
#endif  !BUG1

        /* process the expression */
        opty = optype(o);
        p = block(o, l, r, tyalloc(INT));
        actions = opact(p);


        if ( bdebug )
                printf( "buildtree: actions = 0%o\n", actions );

        if ( actions & LVAL ) { /* check left descendent */
                if ( notlval(p->in.left, 0) ) {
                        if ( o == CAST )
                                /* "illegal cast" */
                                UERROR( ALWAYS, MESSAGE(156) );
                        else
                                /* "illegal lhs of assignment operator" */
                                UERROR( ALWAYS, MESSAGE(62) );
                } else if ( o != PARAMETER && !ininit ) {
                        curr_type = p->in.left->in.type;
                        const_flag = ISCONST(curr_type);
                        /* GH fix for P48787 */
                        /* Added '&& o != CAST' for defect 94103 */
                        if ( const_flag && o != CAST )  {
                                /*"const lhs of assignment operator"*/
                                UERROR( ALWAYS, MESSAGE(133) );
                        }
                        if ( !const_flag && HASCONST(p->in.left->in.type) ) {
                                /* "struct/union lhs of assignment operator has const member" */
                                UERROR( ALWAYS, MESSAGE(135) );
                        }
                }
        }

        if ( !(actions & NCVT) ) {
                switch ( opty ) {
                case BITYPE:
                        if ( !(actions & NCVTR) )
                                p->in.right = pconvert( p->in.right );
                case UTYPE:
                        p->in.left = pconvert( p->in.left );
                }
        }

        if ( actions & (TYPL | TYPR) ) {
                q = (actions & TYPL) ? p->in.left : p->in.right;
                p->in.type = q->in.type;
        }

        if ( actions & CVTL )
                p = convert( p, CVTL );
        if ( actions & CVTR )
                p = convert( p, CVTR );
        if ( actions & TYMATCH )
                p = tymatch( p );
        if ( actions & PTMATCH )
                p = ptmatch( p );

        if ( actions & OTHER ) {
                l = p->in.left;
                r = p->in.right;

                switch ( o ) {

                case NAME:
                        sp = &stab[idname];
                        p->in.type = sp->stype;
                        p->tn.lval = 0;
                        p->tn.rval = idname;
                        if ( TOPTYPE(sp->stype) == UNDEF ) {
                                /* "%s undefined" */
                                WARNING( ALWAYS, MESSAGE(4), sp->psname );
                                /* make p look reasonable */
                                p->in.type = tyalloc(INT);
                                defid( p, SNULL );
                                break;
                        }
                        if ( sp->sclass == LABEL || sp->sclass == ULABEL ) {
                                /* "illegal use of label %s" */
                                UERROR( ALWAYS, MESSAGE(174), sp->psname );
                                /* make p look reasonable */
                                p->in.type = tyalloc(INT);
                                break;
                        }
                        /* special case: MOETY is really an ICON... */
                        if (TOPTYPE(p->in.type) == MOETY) {
                                p->tn.rval = NONAME;
                                p->tn.lval = sp->offset;
                                p->in.type = tynalloc(ENUMTY);
                                p->in.type->typ_size = sp->stype->typ_size;
                                p->in.op = ICON;
                        }

                        break;

                case STRING:
                        p->in.op = NAME;
                        p->in.type = INCREF(
                            tyalloc((devdebug[TYPING]) ? CHAR : UCHAR), ARY);

                        p->in.type->ary_size = strsize;
                        p->tn.lval = 0;
                        p->tn.rval = NOLAB;
                        break;

                case WSTRING:
                        /*
                        ** This is just the same as STRING, except a wide 
                        **  string is (currently) an 'unsigned short',
                        **  that is, of type wchar_t
                        */
                        p->in.op = NAME;
                        p->in.type = INCREF(tyalloc (WCHAR), ARY);
                        p->in.type->ary_size = strsize;
                        p->tn.lval = 0;
                        p->tn.rval = NOLAB;
                        break;

                case STREF:
                        /* p->x turned into *(p+offset) */
                        /* rhs must be a name; check correctness */

                        i = r->tn.rval;
                        if ( i < 0 || ((sp = &stab[i])->sclass != MOS
                             && sp->sclass != MOU && !(sp->sclass & FIELD)) ) {
                                /* "member of structure or union required" */
                                UERROR( ALWAYS, MESSAGE(76) );
                                /* if this name is non-unique, find right one */
                        } else if ( stab[i].sflags & SNONUNIQ && 
                            ISPTR(l->in.type) && 
                            (TOPTYPE(DECREF(l->in.type)) == STRTY || 
                            TOPTYPE(DECREF(l->in.type)) == UNIONTY) && 
                            dimtab[DECREF(l->in.type)->typ_size+1] >= 0 ) {
                                /* nonunique name && structure defined */
                                char    *memnam, *tabnam;
                                register k;
                                int     j;
                                int     memi;
                                j = dimtab[DECREF(l->in.type)->typ_size+1];
                                for ( ; (memi = dimtab[j]) >= 0; ++j ) {
                                        tabnam = stab[memi].psname;
                                        memnam = stab[i].psname;
#ifndef BUG1
                                        if ( ddebug > 1 ) {
                                                printf("member %s==%s?\n",
                                                    memnam, tabnam);
                                        }
#endif  !BUG1
                                        if ( stab[memi].sflags & SNONUNIQ ) {
                                                if (strcmp(memnam, tabnam))
                                                        goto next;
                                                r->tn.rval = i = memi;
                                                break;
                                        }
next:
                                        continue;
                                }
                                if ( memi < 0 )
                                        /* "illegal member use: %s" */
                                        UERROR( ALWAYS, MESSAGE(63), stab[i].psname );
                        } else {
                                register j;
                                register align;
                                if (!ISPTR(l->in.type) || 
                                    (TOPTYPE(DECREF(l->in.type)) != STRTY && 
                                    TOPTYPE(DECREF(l->in.type)) != UNIONTY)) {
                                        if ( stab[i].sflags & SNONUNIQ ) {
                                                /* "nonunique name demands struct/union or struct/union pointer"  */
                                                UERROR( ALWAYS, MESSAGE(84) );
                                        } else {
                                                align = talign(btype(l->in.type));
                                                /* align better be an even multiple of ALSTRUCT */
                                                if ( (align % ALSTRUCT) == 0 )
                                                        /* "struct/union or struct/union pointer required" */
                                                        WERROR( ALWAYS, MESSAGE(103) );
                                                else
                                                        /* "bad structure offset" */
                                                        UERROR( ALWAYS, MESSAGE(5) );
                                        }
                                } else if ( !chkstr( i,
                                    dimtab[btype(l->in.type)->typ_size+1],
                                    DECREF(l->in.type) ) ) {
                                        /* "illegal member use: %s" */
                                        WERROR( ALWAYS, MESSAGE(63), stab[i].psname );
                                }
                        }

                        p = stref( p );
                        break;

                case UNARY MUL:
                        if ( l->in.op == UNARY AND ) {
                                p->in.op = l->in.op = FREE;
                                p = l->in.left;
                        }
                        /* "illegal indirection" */
                        if ( !ISPTR(l->in.type))
                                UERROR( ALWAYS, MESSAGE(60) );
                        p->in.type = DECREF(l->in.type);
                        break;

                case UNARY AND:
                        switch ( l->in.op ) {

                        case UNARY MUL:
                                andable( l );
                                p->in.op = l->in.op = FREE;
                                p = l->in.left;
                                p->in.type = INCREF(l->in.type, PTR);
                                break;

                        case NAME:
                        case LNAME:
                        case PNAME:
                                andable( l );
                                p->in.type = INCREF(l->in.type, PTR);
                                break;

                        case COMOP:
                                lr = buildtree( UNARY AND, l->in.right, NIL );
                                p->in.op = l->in.op = FREE;
                                p = buildtree( COMOP, l->in.left, lr );
                                break;

                        case QUEST:
                                lr = buildtree( UNARY AND, l->in.right->in.right, NIL );
                                ll = buildtree( UNARY AND, l->in.right->in.left, NIL );
                                p->in.op = l->in.op = l->in.right->in.op = FREE;
                                p = buildtree( QUEST, l->in.left, buildtree( COLON, ll, lr ) );
                                switch ( TOPTYPE(DECREF(p->in.type)) ) {
                                case STRTY:
                                case UNIONTY:
                                        /* Mark tree to prevent &((i?x:y).a) */
                                        p->in.flags |= NOTLVAL;
                                }
                                break;

#ifdef  ADDROREG
                        case OREG:
                                /* OREG was built in clocal()
                                 * for an auto or formal parameter
                                 * now its address is being taken
                                 * local code must unwind it
                                 * back to PLUS/MINUS REG ICON
                                 * according to local conventions
                                 */
                                {
                                        extern NODE *addroreg();
                                        p->in.op = FREE;
                                        p = addroreg( l );
                                }
                                break;

#endif  ADDROREG
                        default:
                                /* "unacceptable operand of &" */
                                UERROR( ALWAYS, MESSAGE(110) );
                                p->in.type = INCREF(l->in.type, PTR);
                                break;
                        }
                        break;

                case LS:
                case RS:
                case ASG_LS:
                case ASG_RS:
                        if ( tsize(p->in.right->in.type) > SZINT )
                                p->in.right = makety(p->in.right, tyalloc(INT));
                        break;

                case RETURN:
                case ASSIGN:
                case PARAMETER:
                        /* structure assignment */
                        /* take the addresses of the two sides; then make an
                         * operator using STASG and
                         * the addresses of left and right */

                        {
                                register TPTR t;

                                /* "assignment of different structures" */
                                if ( l->in.type->typ_size != r->in.type->typ_size )
                                        UERROR( ALWAYS, MESSAGE(15) );

                                if ( o == PARAMETER ) {
                                        break;
                                }

                                r->in.flags &= ~NOTLVAL;
                                r = buildtree( UNARY AND, r, NIL );
                                t = r->in.type;

                                l = block( STASG, l, r, t );

                                if ( o == RETURN ) {
                                        p->in.op = FREE;
                                        p = l;
                                        break;
                                }

                                l = clocal( l );
                                p->in.op = UNARY MUL;
                                p->in.left = l;
                                p->in.right = NIL;
                                break;
                        }
                case COLON:
                        /* structure colon */
                        if ( l->in.type->typ_size != r->in.type->typ_size )
                                /* "type clash in conditional" */
                                UERROR( ALWAYS, MESSAGE(109) );
                        break;

                case CALL:
                case UNARY CALL:
                        if ( !ISPTR(l->in.type) ) {
                                /* "illegal function" */
                                UERROR( ALWAYS, MESSAGE(58) );
                                break;
                        }
                        p->in.type = DECREF(l->in.type);
                        if ( !ISFTN(p->in.type) ) {
                                /* "illegal function" */
                                UERROR( ALWAYS, MESSAGE(58) );
                                break;
                        }
                        parmlist = p->in.type->ftn_parm;
                        if ( parmlist != PNIL ) {
                                if ( o == CALL ) {
                                        p->in.right = r = strargs(fixargs(
                                            p->in.right, parmlist, 0));
                                } else if ( TOPTYPE(parmlist->type) != TVOID ) {
                                        /* wrong number of arguments in function call */
                                        UERROR( ALWAYS, MESSAGE(157) );
                                }
                        } else {
                                /* "function prototype not in scope" */
#ifdef  LINT
                                WARNING( WPROTO && WKNR, MESSAGE(181) );
#else
                                WARNING( WPROTO, MESSAGE(181) );
#endif  LINT
                                if ( o == CALL ) {
                                        p->in.right = r = strargs(p->in.right);
                                }
                        }
                        p->in.type = DECREF( p->in.type );
#ifdef  SINGLE_PRECISION
                        if ( TOPTYPE(p->in.type) == FLOAT
                             && !devdebug[PROMOTION] )
                                p->in.type = tyalloc(DOUBLE);
#endif  SINGLE_PRECISION
                        if ( l->in.op == UNARY AND && l->in.left->in.op == NAME
                             && l->in.left->tn.rval >= 0
                             && l->in.left->tn.rval != NONAME
                             && ( (i = stab[l->in.left->tn.rval].sclass)
                             == FORTRAN || i == UFORTRAN ) ) {
                                p->in.op += (FORTCALL - CALL);
                        }
                        if (TOPTYPE(p->in.type) == STRTY || 
                            TOPTYPE(p->in.type) == UNIONTY) {
                                /* function returning structure */
                                /*  make function really return ptr to str., with * */

                                p->in.op += STCALL - CALL;
                                p->in.type = INCREF(p->in.type, PTR);
                                p = buildtree( UNARY MUL, p, NIL );

                        }
                        break;

                default:
                        cerror(TOOLSTR(M_MSG_201, "other code %d"), o );
                }
        }

        if ( actions & CVTO )
                p = oconvert( p );

        if ( asgop(o) && o != CAST )
                p->in.type = unqualtype( p->in.type );

#ifndef BUG1
        if ( bdebug )
                fwalk( p, eprint, 0 );
#endif  !BUG1

        p = clocal( p );

        /*
        ** Fold integer constant expressions.
        */
        if ( ISINTEGRAL(p->in.type) ) {
                switch ( optype( o = p->in.op ) ) {
                case BITYPE:
                        if ( p->in.right->in.op != ICON )
                                break;
                case UTYPE:
                        switch ( o ) {
                        case CBRANCH:
                                break;
                        case DIV:
                        case MOD:
                                if ( p->in.right->tn.lval == 0 )
                                        break;
                                /* fall through ... */
                        default:
                                if ( p->in.left->in.op == ICON ) {
                                        p = foldexpr(p);
                                }
                        }
                }
        }

#ifndef BUG1
        if ( bdebug )
                fwalk( p, eprint, 0 );
#endif  !BUG1

        return p;
}


/* -------------------- foldexpr -------------------- */

NODE *
foldexpr(p)
NODE *p;
{
        int     o;
        NODE * l, *r;
        int     opty;

        if (p == NULL)
                cerror(TOOLSTR(M_MSG_202, "nil expression to fold" ));
        o = p->in.op;
        opty = optype(o);
        if (opty == BITYPE) {
                l = p->in.left;
                r = p->in.right;
        } else if (opty == UTYPE) {
                l = p->in.left;
                r = NULL;
        } else {
                return(p);
        }

#ifndef BUG1
        if ( bdebug > 2 )
                printf( "foldexpr( %s, %o, %o )\n", opst[o], l, r );
#endif  !BUG1

        p->in.left = l = foldexpr(l);

        switch (o) {
                /*
         * binary operations
         */
        case ANDAND:
        case OROR:
                /*
         * for && and || we have to evaluate left operand then
         * we let [f]conval call foldexpr recursively to evaluate
         * the right operand (and only if required).
         */
                if (ISNUMBER(l) || ISNUMBER(r))
                        /* "constant in conditional context" */
                        WARNING( WCONSTANT || WHEURISTIC, MESSAGE(24) );

                if (l->in.op == ICON && ISINTEGRAL(l->in.type)) {
                        if (conval(l, o, r)) {
                                tfree(r);
                                p->in.op = FREE;
                                return(l);
                        }
                } else if (l->in.op == FCON && ISFLOAT(l->in.type)) {
                        if (FOLD_INTEGRAL()) {
                                if (!foldadd) {
                                        UERROR( ALWAYS, MESSAGE(151) );
                                        foldadd++;
                                }
                        }
                        if (fconval(l, o, r)) {
                                tfree(r);
                                p->in.op = FREE;
                                return(l);
                        }
                }
                break; /* unreachable */
        case ULT:
        case UGT:
        case ULE:
        case UGE:
        case LT:
        case GT:
        case LE:
        case GE:
        case EQ:
        case NE:
        case CBRANCH:
        case PLUS:
        case MINUS:
        case MUL:
        case DIV:
        case MOD:
        case AND:
        case OR:
        case ER:
        case LS:
        case RS:
                p->in.right = r = foldexpr(r);

                /* check for constants in conditional contexts */
                if (o == CBRANCH) {
                        if (ISNUMBER(l))
                                /* "constant in conditional context" */
                                WARNING( WCONSTANT || WHEURISTIC, MESSAGE(24) );
                } else if (logop(o) && ISNUMBER(l) && ISNUMBER(r))
                        /* "constant in conditional context" */
                        WARNING( WCONSTANT || WHEURISTIC, MESSAGE(24) );

                if (ISCONSTANT(l) && ISCONSTANT(r)) {
                        if (ISFLOAT(p->in.type) || ISFLOAT(l->in.type) || 
                            ISFLOAT(r->in.type)) {
                                if (FOLD_INTEGRAL()) {
                                        if (!foldadd) {
                                                UERROR( ALWAYS, MESSAGE(151) );
                                                foldadd++;
                                        }
                                }
                                if (fconval(l, o, r)) {
                                        r->in.op = FREE;
                                        p->in.op = FREE;
                                        return(l);
                                }
                        } else if (ISINTEGRAL(p->in.type) || ISPTR(p->in.type)) {
                                if (conval(l, o, r)) {
                                        r->in.op = FREE;
                                        p->in.op = FREE;
                                        return(l);
                                }
                        }
                }
                break;
        case QUEST:
                switch (l->in.op) {
                case ICON:
                        if (ISPTR(l->in.type)) {
                                if (!NO_FOLD()) {
                                        /*
                                         * Address constants are illegal in
                                         * arithmetic constant expression, so
                                         * complain, and
                                         * return the tree untouched.
                                         * In case we have an address constant
                                         * in a run-time expression, return
                                         * the tree as is.
                                         */
                                        if (!foldadd) {
                                                UERROR( ALWAYS, MESSAGE(152) );
                                                foldadd++;
                                        }
                                }
                                break;
                        }
                        if (!ISNUMBER(l))
                                break;
                        r->in.op = FREE;
                        l->in.op = FREE;
                        p->in.op = FREE;
                        if ( (int) l->tn.lval ) {
                                tfree( r->in.right );
                                return(foldexpr(r->in.left));
                        } else {
                                tfree( r->in.left );
                                return(foldexpr(r->in.right));
                        }
                        /*NOTREACHED*/
                        break;
                case FCON:
                        r->in.op = FREE;
                        l->in.op = FREE;
                        p->in.op = FREE;
#ifdef  HOSTIEEE
                        if (TOPTYPE(l->in.type) ==  FLOAT)
                                l->tn.lval = (long)(float) l->fpn.dval;
                        else
                                l->tn.lval = (long)(double) l->fpn.dval;
#else
                        /*
                         * compare the value to 0.0;
                         * if the function returned EQUAL, then the value was equal
                         * to zero and therefore its logical negation is a '1'.
                         * and vice versa. EQUAL is defined in <sys/FP.h>.
                         */
                        _FPcpdi(1, 0.0);
                        l->tn.lval = _FPcmdi(1, l->fpn.dval) == EQUAL ? 1 : 0;
#endif  HOSTIEEE
                        if (l->tn.lval)  {
                                tfree( r->in.right );
                                return(foldexpr(r->in.left));
                        } else {
                                tfree( r->in.left );
                                return(foldexpr(r->in.right));
                        }
                        /*NOTREACHED*/
                        break;
                default:
                        break;
                }
                break;
                /*
         * unary operations
         */
        case SCONV:
                if ( ISCONSTANT(l) ) {
                        if ( FOLD_INTEGRAL() && !ISINTEGRAL(p->in.type) ) {
                                if (!foldadd) {
#ifdef  COMPAT
                                        if ( devdebug[KLUDGE] && 
                                            !devdebug[COMPATIBLE] )
                                                WERROR( ALWAYS, MESSAGE(150) );
                                        else
                                                UERROR( ALWAYS, MESSAGE(150) );
#else
                                        UERROR( ALWAYS, MESSAGE(150) );
#endif  COMPAT
                                        foldadd++;
                                }
                        }
                        if (FOLD_GENERAL() && ISPTR(l->in.type)) {
                                WARNING( WPORTABLE, MESSAGE(182) );
                        }
                        p = docast( p );
                } else if ( !NO_FOLD() ) {
                        if (!foldadd) {
                                FOLD_INTEGRAL() ? UERROR(ALWAYS, MESSAGE(150))
                                 : UERROR(ALWAYS, MESSAGE(182));
                                foldadd++;
                        }
                }

                /*
                 * 'p' now contains the SCONV'ed constant or
                 * the tree (SCONV x nil). Return that.
                 */
                break;
        case NOT:
        case UNARY MINUS:
        case COMPL:
                switch (l->in.op) {
                case ICON:
                        /* "constant argument to NOT"  */
                        if (o == NOT && l->tn.rval == NONAME)
                                WARNING( WCONSTANT || WHEURISTIC, MESSAGE(22) );

                        if ( conval( l, o, l ) ) {
                                p->in.op = FREE;
                                return(l);
                        }
                        break;
                case FCON:
                        /* "constant argument to NOT"  */
                        if (o == NOT)
                                WARNING( WCONSTANT || WHEURISTIC, MESSAGE(22) );

                        if (FOLD_INTEGRAL() && o != COMPL) {
                                if (!foldadd) {
                                        UERROR( ALWAYS, MESSAGE(151) );
                                        foldadd++;
                                }
                        }

                        if (fconval( l, o, l )) {
                                p->in.op = FREE;
                                return(l);
                        }
                        break;
                default:
                        if (!NO_FOLD())
                                if (!foldadd) {
                                        UERROR( ALWAYS, MESSAGE(146), opst[o] );
                                        foldadd++;
                                }
                        return(p);
                }
                break;
        case PCONV:
                if ( FOLD_INTEGRAL() ) {
                        if ( !foldadd ) {
#ifdef  COMPAT
                                if ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
                                        WERROR( ALWAYS, MESSAGE(150) );
                                else
                                        UERROR( ALWAYS, MESSAGE(150) );
#else
                                UERROR( ALWAYS, MESSAGE(150) );
#endif  COMPAT
                                foldadd++;
                        }
                } else if ( !FOLD_GENERAL() ) {
                        break;
                }
                if ( l->in.op == ICON ) {
#ifdef  XCOFF
                        /* function ptrs and data ptrs differ */
                        if ( !ISPTR(l->in.type) || 
                            ISFTN(DECREF(l->in.type)) == 
                            ISFTN(DECREF(p->in.type)) )
                                l->in.type = p->in.type;
#else
                        l->in.type = p->in.type;
#endif  XCOFF
                        p->in.op = FREE;
                        return( l );
                }
                break;
        default:
                /*
                 * other operations
                 */
                if (r != NULL)
                        p->in.right = r = foldexpr(r);
                if (asgop(o) || o == COMOP || callop(o)) {
                        if (!NO_FOLD())
                                if (!foldadd) {
                                        UERROR( ALWAYS, MESSAGE(146), opst[o] );
                                        foldadd++;
                                }
                }
        }
        return( p );
}


/* -------------------- assary -------------------- */

assary(realid, ghostid)
int     realid, ghostid;
{
        extern int      eprint();

        /*
         * called to perform a fake structure assignment
         * of an array. called only when initializing
         * automatic aggregate arrays.
         */

        NODE * r, *l, *p;

        /*
         * step 1. Backup real type and make a new one
         */
        TPTR rt = stab[realid].stype;
        TPTR gt = stab[ghostid].stype;
        TPTR tempType = tynalloc(STRTY);
        tempType->typ_size = curdim;
        dstash(tsize(rt));
        dstash(-1);
        dstash(talign(DECREF(rt)));
        dstash(-1);

        /*
         * step 2. fix up types in stab.
         */
        stab[realid].stype = tempType;
        stab[ghostid].stype = tempType;

        /*
         * step 3. hardwire the expression tree
         */

        ininit = 1;
        idname = realid;
        l = buildtree(NAME, NIL, NIL);
        idname = ghostid;
        r = buildtree(NAME, NIL, NIL);
        p = buildtree(ASSIGN, l, r);
        ininit = 0;

#ifndef BUG1
        if (bdebug)
                fwalk(p, eprint, 0);
#endif  !BUG1

        /* emit code */
        bccode();
        ecomp(p);

        /* restore real types */
        stab[realid].stype = rt;
        stab[ghostid].stype = gt;
}


/* -------------------- strargs -------------------- */

NODE *
strargs( p )
register NODE *p;
{ /* rewrite structure flavored arguments */

        if ( p->in.op == CM ) {
                p->in.left = strargs( p->in.left );
                p->in.right = strargs( p->in.right );
                return( p );
        }

        switch ( TOPTYPE(p->in.type) ) {
        case STRTY:
        case UNIONTY:
                p->in.flags &= ~NOTLVAL;
                p = block( STARG, p, NIL, p->in.type );
                p->in.left = buildtree( UNARY AND, p->in.left, NIL );
                p = clocal(p);
                break;
        case TVOID:
                /* "void type illegal in expression" */
                UERROR( ALWAYS, MESSAGE(118) );
                break;
        }
        return( p );
}


/* -------------------- fixargs -------------------- */

NODE *
fixargs( p, q, qNext )
register NODE *p;
PPTR q;
PPTR *qNext;
{
        if ( q == PNIL || TOPTYPE(q->type) == TELLIPSIS ) {
                if ( qNext ) {
                        *qNext = q;
                }
                return( p );
        }

        if ( p->in.op == CM ) {
                p->in.left = fixargs( p->in.left, q, &q );
                p->in.right = fixargs( p->in.right, q, qNext );
                return( p );
        }

        p = cvtarg( p, q->type );
        q = q->next;

        if ( qNext ) {
                *qNext = q;
                if ( q == PNIL ) {
                        /* "wrong number of arguments in function call" */
                        UERROR( ALWAYS, MESSAGE(157) );
                }
        } else if ( q != PNIL && TOPTYPE(q->type) != TELLIPSIS ) {
                /* "wrong number of arguments in function call" */
                UERROR( ALWAYS, MESSAGE(157) );
        }

        return( p );
}


/* -------------------- cvtarg -------------------- */

NODE *
cvtarg( p, t )
register NODE *p;
TPTR t;
{
        /* use the existing type checking mechanisms in buildtree
         * by creating a tree with a dummy NAME node with the type
         * of the prototype argument on the left and the expression
         * to be converted on the right.   Run this through build tree
         * and return what appears on the right side of the tree.
         * This should have the necessary conversions.
         */

        if ( TOPTYPE(t) == TVOID ) {
                /* "wrong number of arguments in function call" */
                UERROR( ALWAYS, MESSAGE(157) );
                return( p );
        }
        if ( TOPTYPE(p->in.type) == TVOID ) {
                /* strargs will catch this */
                return( p );
        }

        if ( !comtypes( t, unqualtype(p->in.type), 0 ) && 
            !( ISPTR(t) && nullptr(p) ) ) {
                int     t_unsgn, p_unsgn;
                if (ISUNSIGNED(t))
                        t_unsgn = DEUNSIGN(TOPTYPE(t));
                else
                        t_unsgn = TOPTYPE(t);
                if (ISUNSIGNED(p->in.type))
                        p_unsgn = DEUNSIGN(TOPTYPE(p->in.type));
                else
                        p_unsgn = TOPTYPE(p->in.type);
                if (t_unsgn != p_unsgn && 
                    !(t_unsgn == INT && (p_unsgn == SHORT || p_unsgn == SCHAR
#if     SZLONG == SZINT
                     || p_unsgn == LONG
#endif  
                    ) ) && 
                    !(t_unsgn == LONG && (p_unsgn == INT 
                     || p_unsgn == SCHAR)) && 
                    ( !ISPTR(t) || !ISPTR(p->in.type) || 
                    !comtypes( unqualtype(DECREF(t)),
                    DECREF(p->in.type), 0 )) ) {
                        /* "mismatched type in function argument" */
                        WARNING( WPROTO, MESSAGE(159) );
                }
        }

        p = buildtree( PARAMETER, block( NAME, NIL, NIL, t ), p );
        p->in.left->in.op = p->in.op = FREE;
        return( p->in.right );
}


/* -------------------- chkstr -------------------- */

chkstr( i, j, type )
TPTR type;
{
        /* is the MOS or MOU at stab[i] OK for strict reference by a ptr */
        /* i has been checked to contain a MOS or MOU */
        /* j is the index in dimtab of the members... */
        int     k, kk;

#ifndef ddebug
        extern int      ddebug;
#endif

#ifndef BUG1
        if ( ddebug > 1 )
                printf( "chkstr( %s(%d), %d )\n", stab[i].psname, i, j );
#endif  !BUG1
        if ( (k = j) < 0 )
                /* "undefined structure or union" */
                UERROR( ALWAYS, MESSAGE(112) );
        else {
                for ( ; (kk = dimtab[k] ) >= 0; ++k ) {
                        if ( kk >= nstabents ) {
                                cerror(TOOLSTR(M_MSG_203, "gummy structure" ));
                        }
                        if ( kk == i )
                                return( 1 );
                        switch (TOPTYPE(stab[kk].stype)) {

                        case STRTY:
                        case UNIONTY:
                                /* no recursive looking for strs */
                                if (TOPTYPE(type) == STRTY)
                                        continue;
                                if ( WDECLAR && chkstr( i,
                                    dimtab[stab[kk].stype->typ_size+1],
                                    stab[kk].stype ) ) {
                                        if ( *stab[kk].psname == '$' )
                                                return(0);  /* $FAKE */
                                        /* "illegal member use: perhaps %s.%s" */
                                        WARNING( WDECLAR || WHEURISTIC, MESSAGE(65),
                                            stab[kk].psname,
                                            stab[i].psname );
                                        return(1);
                                }
                        }
                }
        }
        return( 0 );
}


/* -------------------- conval -------------------- */

conval( p, o, q )
register NODE *p, *q;
{
        /* apply the op o to the lval part of p; if binary, rhs is val */
        register CONSZ val;

        /*
         * check that neither ICONs is an address constant
         * and if so, then the only valid operations are:
         * address + constant, constant + address,
         * address - constant, and address - address.
         * (notice the difference between - and +).
         * As defined by ANSI Standard Draft Section 3.3.6
         */
        if ((ISPTR(p->in.type) || o != PLUS) &&
	    ISPTR(q->in.type) && o != MINUS) {
                if (!NO_FOLD() && !foldadd) {
                        UERROR( ALWAYS, MESSAGE(152));
                        foldadd++;
                }
                return 0;
        }

        switch (o) {
          case MUL:
          case DIV:
          case MOD:
          case AND:
          case OR:
          case ER:
          case LS:
          case RS:
            if (p->tn.rval != NONAME || q->tn.rval != NONAME)
	        return 0;
            break;
          case UNARY MINUS:
          case COMPL:
          case NOT:
            if (p->tn.rval != NONAME)
	        return 0;
            break;
          case LT:
          case LE:
          case GT:
          case GE:
            if (ISUNSIGNED(p->in.type) || ISUNSIGNED(q->in.type))
                o += (UGE - GE);
          case EQ:
          case NE:
            if (p->tn.rval != q->tn.rval)
	        return 0;
	    p->tn.rval = NONAME;
            break;
	}

        val = q->tn.lval;

        switch ( o ) {

        case PLUS:
                p->tn.lval += q->tn.lval;
                q->tn.lval = 0;
                if ( p->tn.rval == NONAME ) {
                        p->tn.rval = q->tn.rval;
                        p->in.type = q->in.type;
                        q->tn.rval = NONAME;
                }
                return( q->tn.rval == NONAME );
        case MINUS:
                p->tn.lval -= q->tn.lval;
                q->tn.lval = 0;
                if ( p->tn.rval == q->tn.rval ) {
                        p->tn.rval = NONAME;
                        q->tn.rval = NONAME;
                }
                return( q->tn.rval == NONAME );
        case MUL:
                p->tn.lval *= val;
                break;
        case DIV:
                /* "division by 0" */
                if ( val == 0 )
                        UERROR( ALWAYS, MESSAGE(31) );
                else if (ISUNSIGNED(p->in.type) || ISUNSIGNED(q->in.type))
                        p->tn.lval = (UCONSZ) p->tn.lval / (UCONSZ) val;
                else
                        p->tn.lval = p->tn.lval / val;
                break;
        case MOD:
                /* "division by 0" */
                if ( val == 0 )
                        UERROR( ALWAYS, MESSAGE(31) );
                else if (ISUNSIGNED(p->in.type) || ISUNSIGNED(q->in.type))
                        p->tn.lval = (UCONSZ) p->tn.lval % (UCONSZ) val;
                else
                        p->tn.lval = p->tn.lval % val;
                break;
        case AND:
                p->tn.lval &= val;
                break;
        case OR:
                p->tn.lval |= val;
                break;
        case ER:
                p->tn.lval ^=  val;
                break;
        case LS:
                p->tn.lval <<= val;
                break;
        case RS:
                if (ISUNSIGNED(p->in.type))
                        p->tn.lval = (UCONSZ) p->tn.lval >> val;
                else
                        p->tn.lval >>= val;
                break;
        case UNARY MINUS:
                if (ISUNSIGNED(p->in.type))
                        p->tn.lval = (unsigned) -p->tn.lval;
                else
                        p->tn.lval = -p->tn.lval;
                break;
        case COMPL:
                if (ISUNSIGNED(p->in.type))
                        p->tn.lval = (unsigned) ~ p->tn.lval;
                else
                        p->tn.lval = ~ p->tn.lval;
                break;

#if 0
        case UNARY MINUS:
#if     ULONG_MAX > UINT_MAX
                if (TOPTYPE(p->tn.type) == UNSIGNED)
                        p->tn.lval = (unsigned) -p->tn.lval;
                else
#endif  
                        p->tn.lval = -p->tn.lval;
                break;
        case COMPL:
#if     ULONG_MAX > UINT_MAX
                if (TOPTYPE(p->tn.type) == UNSIGNED)
                        p->tn.lval = (unsigned) ~ p->tn.lval;
                else
#endif  
                        p->tn.lval = ~ p->tn.lval;
                break;
#endif  
        case NOT:
                p->tn.lval = !p->tn.lval;
                break;
        case LT:
                p->tn.lval = p->tn.lval < val;
                break;
        case LE:
                p->tn.lval = p->tn.lval <= val;
                break;
        case GT:
                p->tn.lval = p->tn.lval > val;
                break;
        case GE:
                p->tn.lval = p->tn.lval >= val;
                break;
        case ULT:
                p->tn.lval = ((UCONSZ)p->tn.lval < (UCONSZ)val);
                break;
        case ULE:
                p->tn.lval = ((UCONSZ)p->tn.lval <= (UCONSZ)val);
                break;
        case UGE:
                p->tn.lval = ((UCONSZ)p->tn.lval >= (UCONSZ)val);
                break;
        case UGT:
                p->tn.lval = ((UCONSZ)p->tn.lval > (UCONSZ)val);
                break;
        case EQ:
                p->tn.lval = p->tn.lval == val;
                break;
        case NE:
                p->tn.lval = p->tn.lval != val;
                break;
        case ANDAND:
                if (p->tn.rval != NONAME || p->tn.lval) {
		  unsigned isnil;

                  /* fold the right branch of the tree and test it
                     whether it is 0 or none 0. */

                  q = foldexpr(q);
                  if (!ISNUMBER(q))
		      return 0;

                  if (FOLD_INTEGRAL() && !ISINTEGRAL(q->in.type))
		      UERROR( ALWAYS, MESSAGE(151) );

                  if (!ISFLOAT(q->in.type))
                      isnil = (q->tn.lval == 0);
                  else
#ifdef  HOSTIEEE
                  if (TOPTYPE(q->in.type) == FLOAT)
                      isnil = ((float)q->fpn.dval == 0);
                  else
                      isnil = ((double)q->fpn.dval == 0);
#else
		  {
		      _FPcpdi(1, 0.0);
                      isnil = (_FPcmdi(1, q->fpn.dval) == EQUAL);
                  }
#endif  HOSTIEEE
                  if (isnil) {
                      p->tn.lval = 0;
                      p->tn.rval = NONAME;
                  }
		  else if (p->tn.rval != NONAME)
		      return 0;
                  else
		      p->tn.lval = 1;
                }
                p->in.type = tyalloc(INT);
                break;
        case OROR:
                if (p->tn.rval != NONAME || p->tn.lval) {
		  unsigned notnil;

                  /* fold the right branch of the tree and test it
                     whether it is 0 or none 0. */

                  q = foldexpr(q);
                  if (!ISNUMBER(q))
		      return 0;

                  if (FOLD_INTEGRAL() && !ISINTEGRAL(q->in.type))
		      UERROR( ALWAYS, MESSAGE(151) );

                  if (!ISFLOAT(q->in.type))
                      notnil = (q->tn.lval != 0);
                  else
#ifdef  HOSTIEEE
                  if (TOPTYPE(q->in.type) == FLOAT)
                      notnil = ((float)q->fpn.dval != 0);
                  else
                      notnil = ((double)q->fpn.dval != 0);
#else
		  {
		      _FPcpdi(1, 0.0);
                      notnil = (_FPcmdi(1, q->fpn.dval) != EQUAL);
                  }
#endif  HOSTIEEE
                  if (notnil) {
                      p->tn.lval = 1;
                      p->tn.rval = NONAME;
                  }
		  else if (p->tn.rval != NONAME)
		      return 0;
                  else
		      p->tn.lval = 0;
                } else
                  p->tn.lval = 1;
                p->in.type = tyalloc(INT);
                break;
        default:
                return 0;
        }
        return 1;
}


/* -------------------- fconval -------------------- */

fconval( p, o, q )
register NODE *p, *q;
{

        /*
         * similar to conval but applies to FCONs
         * first check we don't have any pointer types.
         */

        if (ISPTR(p->in.type) || ISPTR(q->in.type)) {
                if (!NO_FOLD()) {
                        if (!foldadd) {
                                UERROR( ALWAYS, MESSAGE(152));
                                foldadd++;
                        }
                }
                return(0);
        }

        if ( p->in.op == ICON ) {
                if ( p->tn.rval != NONAME )
                        return( 0 );
#ifdef  HOSTIEEE
                p->fpn.dval = (double) p->tn.lval;
#else
                p->fpn.dval = _FPi2d(1, p->tn.lval);
#endif  HOSTIEEE

                p->in.type = tyalloc(DOUBLE);
                p->in.op = FCON;
        }
        if ( q->in.op == ICON ) {
                if ( q->tn.rval != NONAME )
                        return( 0 );
#ifdef  HOSTIEEE
                q->fpn.dval = (double) q->tn.lval;
#else
                q->fpn.dval = _FPi2d(1, q->tn.lval);
#endif  HOSTIEEE

                q->in.type = tyalloc(DOUBLE);
                q->in.op = FCON;
        }

        switch ( o ) {

        case PLUS:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) ==  FLOAT)
                        p->fpn.dval = (float) p->fpn.dval + (float) q->fpn.dval;
                else
                        p->fpn.dval = (double) p->fpn.dval + (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->fpn.dval = _FPaddi(1, q->fpn.dval);
#endif  HOSTIEEE

                break;

        case MINUS:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->fpn.dval = (float) p->fpn.dval - (float) q->fpn.dval;
                else
                        p->fpn.dval = (double) p->fpn.dval - (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->fpn.dval = _FPsbdi(1, q->fpn.dval);
#endif  HOSTIEEE

                break;

        case MUL:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->fpn.dval = (float) p->fpn.dval * (float) q->fpn.dval;
                else
                        p->fpn.dval = (double) p->fpn.dval * (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->fpn.dval = _FPmldi(1, q->fpn.dval);
#endif  HOSTIEEE
                break;

        case DIV:
#ifdef  HOSTIEEE
                if ( q->fpn.dval == 0 )
                        /* "division by 0." */
                        UERROR( ALWAYS, MESSAGE(32) );
                else {
                        if (TOPTYPE(p->in.type) == FLOAT)
                                p->fpn.dval = (float) p->fpn.dval / (float) q->fpn.dval;
                        else
                                p->fpn.dval = (double) p->fpn.dval / (double) q->fpn.dval;
                }
#else
                _FPi2d(1, 0);
                if ( _FPcmdi(1, q->fpn.dval) == 0 )
                        /* "division by 0." */
                        UERROR( ALWAYS, MESSAGE(32) );
                else {
                        _FPcpdi(1, p->fpn.dval);
                        p->fpn.dval = _FPdvdi(1, q->fpn.dval);
                }
#endif  HOSTIEEE
                break;

        case UNARY MINUS:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) ==  FLOAT)
                        p->fpn.dval =  -(float) p->fpn.dval;
                else
                        p->fpn.dval =  -(double) p->fpn.dval;
#else
                _FPcpdi(1, 0.0);
                p->fpn.dval = _FPsbdi(1, p->fpn.dval);
#endif  HOSTIEEE
                break;

        case NOT:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) ==  FLOAT)
                        p->tn.lval =  !(float) p->fpn.dval;
                else
                        p->tn.lval =  !(double) p->fpn.dval;
#else
                /*
                 * compare the value to 0.0;
                 * if the function returned EQUAL, then the value was equal
                 * to zero and therefore its logical negation is a '1'.
                 * and vice versa. EQUAL is defined in <sys/FP.h>.
                 */
                _FPcpdi(1, 0.0);
                p->tn.lval = _FPcmdi(1, p->fpn.dval) == EQUAL ? 1 : 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case LT:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval < (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval < (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval) == LESSTHAN ? 1 : 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case LE:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval <= (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval <= (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval);
                if (p->tn.lval == LESSTHAN || p->tn.lval == EQUAL)
                        p->tn.lval = 1;
                else
                        p->tn.lval = 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case GT:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval > (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval > (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval) == GREATER ? 1 : 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case GE:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval >= (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval >= (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval);
                if (p->tn.lval == GREATER || p->tn.lval == EQUAL)
                        p->tn.lval = 1;
                else
                        p->tn.lval = 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case EQ:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval == (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval == (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval) == EQUAL ? 1 : 0;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case NE:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval != (float) q->fpn.dval;
                else
                        p->tn.lval = (double) p->fpn.dval != (double) q->fpn.dval;
#else
                _FPcpdi(1, p->fpn.dval);
                p->tn.lval = _FPcmdi(1, q->fpn.dval) == EQUAL ? 0 : 1;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                break;

        case ANDAND:
                if (FOLD_INTEGRAL())
                        UERROR( ALWAYS, MESSAGE(151) );
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval ? 1 : 0;
                else
                        p->tn.lval = (double) p->fpn.dval ? 1 : 0;
#else
                _FPcpdi(1, 0.0);
                p->tn.lval = _FPcmdi(1, p->fpn.dval) == EQUAL ? 0 : 1;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                /*
                 * now I have the first operand evaluated and read as an ICON
                 * for testing. the following code is similar to that
                 * in convert.
                 */
                if (p->tn.lval) {
                        /*
                         * fold the right branch of the tree and test it
                         * whether it is 0 or none 0.
                         */
                        q = foldexpr(q);
                        if (ISCONSTANT(q)) {
                                if (ISINTEGRAL(q->in.type)) {
                                        if (q->tn.lval)
                                                p->tn.lval = 1;
                                        else
                                                p->tn.lval = 0;
                                        p->in.type = tyalloc(INT);
                                        return(1);
                                }
                                /*
                                 * else, it must be a float.
                                 * Note: No need to generate an error here
                                 * because we already have done so above.
                                 */
#ifdef  HOSTIEEE
                                if (TOPTYPE(p->in.type) ==  FLOAT)
                                        p->tn.lval =  (float) p->fpn.dval ? 1 : 0 ;
                                else
                                        p->tn.lval =  (double) p->fpn.dval ? 1 : 0;
#else
                                _FPcpdi(1, 0.0);
                                p->tn.lval = _FPcmdi(1, p->fpn.dval) == EQUAL ? 0 : 1;
#endif  HOSTIEEE
                                p->in.op = ICON;
                                p->in.type = tyalloc(INT);
                                return(1);
                        }
                        /*
                         * else, it is not a constant expression
                         */
                        return(0);
                }
                p->tn.lval = 0;
                p->in.type = tyalloc(INT);
                break;

        case OROR:
#ifdef  HOSTIEEE
                if (TOPTYPE(p->in.type) == FLOAT)
                        p->tn.lval = (float) p->fpn.dval ? 1 : 0;
                else
                        p->tn.lval = (double) p->fpn.dval ? 1 : 0;
#else
                _FPcpdi(1, 0.0);
                p->tn.lval = _FPcmdi(1, p->fpn.dval) == EQUAL ? 0 : 1;
#endif  HOSTIEEE
                p->in.op = ICON;
                p->in.type = tyalloc(INT);
                if (!p->tn.lval) {
                        /*
                         * fold the right branch of the tree and test it
                         * whether it is 0 or none 0.
                         */
                        q = foldexpr(q);
                        if (ISCONSTANT(q)) {
                                if (ISINTEGRAL(q->in.type)) {
                                        if (q->tn.lval)
                                                p->tn.lval = 1;
                                        else
                                                p->tn.lval = 0;
                                        p->in.type = tyalloc(INT);
                                        return(1);
                                }
                                /*
                                 * else, it must be a float.
                                 */
                                if (FOLD_INTEGRAL())
                                        UERROR( ALWAYS, MESSAGE(151) );
#ifdef  HOSTIEEE
                                if (TOPTYPE(p->in.type) ==  FLOAT)
                                        p->tn.lval =  (float) p->fpn.dval ? 1 : 0 ;
                                else
                                        p->tn.lval =  (double) p->fpn.dval ? 1 : 0;
#else
                                _FPcpdi(1, 0.0);
                                p->tn.lval = _FPcmdi(1, p->fpn.dval) == EQUAL ? 0 : 1;
#endif  HOSTIEEE
                                p->in.op = ICON;
                                p->in.type = tyalloc(INT);
                                return(1);
                        }
                        /*
                         * else, it is not a constant expression
                         */
                        return(0);
                }
                p->tn.lval = 1;
                p->in.type = tyalloc(INT);
                break;
        default:
                return(0);
        }
        return(1);
}


/* -------------------- stref -------------------- */

NODE *
stref( p )
register NODE *p;
{

        TPTR t;
        TWORD qual;
        register int    dsc, align;
        OFFSZ off;
        register struct symtab *q;

        /* make p->x */
        /* this is also used to reference automatic variables */

        q = &stab[p->in.right->tn.rval];
        p->in.right->in.op = FREE;
        p->in.op = FREE;
        p = pconvert( p->in.left );

        /* make p look like ptr to x */

        if ( !ISPTR(p->in.type)) {
                p->in.type = INCREF(p->in.type, PTR);
        }

        /* Propagate qualifications from structure type to member type */
        t = q->stype;
        qual = QUALIFIERS(DECREF(p->in.type)) & ~QUALIFIERS(t);
        if (qual) {
                t = qualtype(t, qual, 1);
        }

        t = INCREF(t, PTR);

        p = makety( p, t );

        /* compute the offset to be added */

        off = q->offset;
        dsc = q->sclass;

        if ( dsc & FIELD ) {  /* normalize offset */
                align = ALINT;
                off = (off / align) * align;
        }
        /* if( off != 0 ) */
        /* OREG will eliminate the zero later                 */
        /* Must have the +0 here for &(foo) to come out right */
        /* when foo is based on a pseudo-register             */
        p = clocal( block( PLUS, p, offcon( off, t ), t ) );

        p = buildtree( UNARY MUL, p, NIL );

        /* if field, build field info */

        if ( dsc & FIELD ) {
                p = block( FLD, p, NIL, q->stype );
                p->tn.rval = PKFIELD( dsc & FLDSIZ, q->offset % align );
        }

        return( clocal(p) );
}


/* -------------------- notlval -------------------- */

notlval(p, tyok)
register NODE *p;
{

        /* return 0 if p an lvalue, 1 otherwise */

        register NODE *q;

again:

        switch ( p->in.op ) {

        case FLD:
                p = p->in.left;
                goto again;

        case UNARY MUL:
                switch ( p->in.left->in.op ) {
                case STASG:     /* for structures a,b : &(a=b) is illegal */
                case STCALL:            /* &(f(x)) is illegal */
                case UNARY STCALL:      /* &(f()) is illegal */
                        return( 1 );
                case PLUS:
                        q = p->in.left->in.left;
                        while ( q->in.op == PCONV )
                                q = q->in.left;
                        switch ( q->in.op ) {
                        case QUEST:             /* &((i?x:y).a) is illegal */
                                if ( !( q->in.flags & NOTLVAL ) )
                                        break;
                        case STCALL:            /* &(f(x).a) is illegal */
                        case UNARY STCALL:      /* &(f().a) is illegal */
                                return( 1 );
                        }
                }
        case NAME:
        case PNAME:
        case LNAME:
        case OREG:
                if ( !tyok ) {
                        if ( ISARY(p->in.type) || ISFTN(p->in.type) ) {
                                return(1);
                        }
                }
        case REG:
                return( 0 );

        default:
                return( 1 );
        }
}


/* -------------------- bcon -------------------- */

NODE *
bcon(CONSZ i) { /* make a constant node with value i */
        register NODE *p;
        p = block(ICON, NIL, NIL, tyalloc(INT));
        p->tn.lval = i;
        p->tn.rval = NONAME;
        return( p );
}


/* -------------------- bpsize -------------------- */

NODE *
bpsize(t)
register TPTR t;
{
        return( offcon( psize(t), t ) );
}


/* -------------------- psize -------------------- */

OFFSZ
psize( t )
TPTR t;
{
        /* psize returns the size of the thing t points to */

        if ( !ISPTR(t) ) {
                /* "pointer required" */
                UERROR( ALWAYS, MESSAGE(90) );
                return( SZINT );
        }
        /* note: no pointers to fields */
#ifdef  COMPAT
        t = DECREF(t);
        if ( TOPTYPE(t) == TVOID && devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) {
                /* "illegal use of void type" */
                WERROR( ALWAYS, MESSAGE(147) );
                return( SZCHAR );
        }
        return( tsize( t ) );
#else
        return( tsize( DECREF(t) ) );
#endif  COMPAT
}


/* -------------------- convert -------------------- */

NODE *
convert( p, f )
register NODE *p;
{
        /*  convert an operand of p
            f is either CVTL or CVTR
            operand has type int, and is converted by the size of the other side
            */

        register NODE *q, *r;

        if (f == CVTL) {
                q = p->in.left;
                r = p->in.right;
        } else {
                q = p->in.right;
                r = p->in.left;
        }

        q = block(PMCONV, q, bpsize(r->in.type), tyalloc(INT));
        q = clocal(q);
        if ( f == CVTL )
                p->in.left = q;
        else
                p->in.right = q;

        p->in.type = r->in.type;

        return(p);
}


/* -------------------- econvert -------------------- */

econvert( p )
register NODE *p;
{

        /* change enums to ints, or appropriate types */

        register s;
        register TWORD ty;

        s = dimtab[btype(p->in.type)->typ_size];
        if ( s == SZCHAR )
                ty = SCHAR;
        else if ( s == SZINT )
                ty = INT;
        else if ( s == SZSHORT )
                ty = SHORT;
        else
                ty = LONG;

        MODTYPE(p->in.type, ty);
}


/* -------------------- pconvert -------------------- */

NODE *
pconvert( p )
register NODE *p;
{

#ifdef  IS_COMPILER
        /* emit pseudo-op if variable is volatile */
        if ( ISVOLATILE(p->in.type) && !volEmitted ) {
                EmitVolatile();
                volEmitted = 1;
        }
#endif  IS_COMPILER

        /* if p should be changed into a pointer, do so */
        if ( ISARY(p->in.type) ) {
                p->in.type = DECREF( p->in.type );
                return( buildtree( UNARY AND, p, NIL ) );
        }
        if ( ISFTN( p->in.type) )
                return( buildtree( UNARY AND, p, NIL ) );

        return( p );
}


/* -------------------- oconvert -------------------- */

NODE *
oconvert(p)
register NODE *p;
{
        /* convert the result itself: used for pointer and unsigned */

        switch (p->in.op) {

        case LE:
        case LT:
        case GE:
        case GT:
                if ( ISUNSIGNED(p->in.left->in.type)
                     || ISUNSIGNED(p->in.right->in.type) )
                        p->in.op += (ULE - LE);
        case EQ:
        case NE:
                return( p );

        case MINUS:
                return(  clocal(
                    block( PVCONV, p, bpsize(p->in.left->in.type),
                    tyalloc(INT) )));
        }

        cerror(TOOLSTR(M_MSG_204, "illegal oconvert: %d"), p->in.op );

        return(p);
}


/* -------------------- ptmatch -------------------- */

NODE *
ptmatch( p )
register NODE *p;
{
        /*
        ** This routine makes the operands of p agree;
        ** they are either pointers or integers, by this time.
        */
        TPTR t1, t2, t;
        int     o;
        register NODE *q;
        TWORD qual;

        o = p->in.op;
        t = t1 = p->in.left->in.type;
        t2 = p->in.right->in.type;

        q = p->in.right;

        switch ( o ) {

        case ASSIGN:
        case PARAMETER:
        case RETURN:
                /* assert ISPTR(t1) */
                if ( nullptr(q) )
                        break;
                if ( ISPTR(t2) ) {
                        /* Left must have all the qualifiers of right */
                        t1 = DECREF(t1);
                        t2 = DECREF(t2);
                        if ( QUALIFIERS(t2) & ~QUALIFIERS(t1) ) {
                                /* "illegal pointer qualifier combination, op %s" */
                                WERROR( devdebug[COMPATIBLE], MESSAGE(148),
                                    opst[o] );
                        }
                        /* Types pointed to must be compatible or void */
                        if ( TOPTYPE(t2) == TVOID && !ISFTN(t1) ) {
                                if ( talign(t1) > ALCHAR ) {
                                        /* "possible pointer alignment problem, op %s" */
                                        WARNING( WPORTABLE || WHEURISTIC, MESSAGE(91), opst[o] );
                                }
                        } else if ( TOPTYPE(t1) != TVOID || ISFTN(t2) ) {
                                /* Pointers must match. */
                                (void) comtypes( t1, t2, o );
                        }
                } else {
                        /* "illegal combination of pointer and integer, op %s" */
                        WERROR( devdebug[COMPATIBLE], MESSAGE(53), opst[o] );
                }
                break;

        case CAST:
                /* assert ISPTR(t1) */
                if ( ISPTR(t2) ) {
                        t1 = DECREF(t1);
                        t2 = DECREF(t2);
                        if ( ISFTN(t1) && ISFTN(t2) ) {
                                t1 = DECREF(t1);
                                t2 = DECREF(t2);
                        }
                        if ( talign(t1) > talign(t2) ) {
                                /* "possible pointer alignment problem, op %s" */
                                WARNING( WPORTABLE || WHEURISTIC, MESSAGE(91), opst[o] );
                        }
                } else if ( q->in.op == ICON && TOPQTYPE(DECREF(t1)) != TVOID ) {
                        if ( !NO_FOLD() && (!ISSINGLE || blevel == 0) ) {
                                /* "non portable use of cast" */
                                if (devdebug[COMPATIBLE])
                                        warning( ALWAYS, MESSAGE(183) );
                                else
                                        warning( WPORTABLE, MESSAGE(183) );
                        } else if ( ( q->tn.lval % ( talign(DECREF(t1)) / 
                            ALCHAR ) ) != 0 ) {
                                /* "possible pointer alignment problem, op %s" */
                                WARNING( WPORTABLE || WHEURISTIC, MESSAGE(91), opst[o] );
                        }
                }
                break;

        case MINUS:
                /* assert ISPTR(t1) && ISPTR(t2) */
                t1 = DECREF(t1);
                t2 = DECREF(t2);
                if ( !comtypes(t1, t2, o) ) {
                        if ( tsize(t1) != tsize(t2) ) {
                                /* "illegal pointer subtraction" */
                                UERROR( ALWAYS, MESSAGE(67) );
                        }
                }
                break;

        case COLON:
                /* assert ISPTR(t1) || ISPTR(t2) */
                if ( ISPTR(t2) ) {
                        q = p->in.left;
                        t = t2;
                }
                if ( ISPTR(q->in.type) ) {
                        /* Both are pointers */
                        if ( nullptr(p->in.left) ) {
                                t = t2;
                                break;
                        }
                        if ( nullptr(p->in.right) ) {
                                t = t1;
                                break;
                        }
                        t1 = DECREF(t1);
                        t2 = DECREF(t2);
                        qual = QUALIFIERS(t1) | QUALIFIERS(t2);
                        if ( TOPTYPE(t1) == TVOID && !ISFTN(t2) ) {
                                t = t1;
                        } else if ( TOPTYPE(t2) == TVOID && !ISFTN(t1) ) {
                                t = t2;
                        } else if ( comtypes(unqualtype(t1), unqualtype(t2), o) ) {
                                t = copytype(t1, blevel);
                                mkcomposite( t, t2, blevel );
                        } else {
                                t = t1;
                        }
                        qual &= ~QUALIFIERS(t);
                        if ( qual ) {
                                t = qualtype(t, qual, 1);
                        }
                        t = INCREF(t, PTR);
                } else if ( !nullptr(q) ) {
                        /* "illegal combination of pointer and integer, op %s" */
                        WERROR( devdebug[COMPATIBLE], MESSAGE(53), opst[o] );
                }
                break;

        case EQ:
        case NE:
                /* assert ISPTR(t1) || ISPTR(t2) */
                if ( ISPTR(t2) ) {
                        q = p->in.left;
                        t = t2;
                }
                if ( ISPTR(q->in.type) ) {
                        /* Both are pointers */
                        if ( nullptr(p->in.left) ) {
                                t = t2;
                                break;
                        }
                        if ( nullptr(p->in.right) ) {
                                t = t1;
                                break;
                        }
                        t1 = DECREF(t1);
                        t2 = DECREF(t2);
                        if ( TOPTYPE(t1) == TVOID && !ISFTN(t2) ) {
                                t = t1;
                        } else if ( TOPTYPE(t2) == TVOID && !ISFTN(t1) ) {
                                t = t2;
                        } else if ( !comtypes(t1, t2, o) ) {
                                if ( talign(t1) < talign(t2) ) {
                                        t = t1;
                                } else {
                                        t = t2;
                                }
                        }
                        t = INCREF(t, PTR);
                } else if ( !nullptr(q) ) {
                        /* "illegal combination of pointer and integer, op %s" */
                        WERROR( devdebug[COMPATIBLE], MESSAGE(53), opst[o] );
                }
                break;

        default:        /* relationals */
                /* assert ISPTR(t1) || ISPTR(t2) */
                if ( ISPTR(t2) ) {
                        q = p->in.left;
                        t = t2;
                }
                if ( ISPTR(q->in.type) ) {
                        /* Both are pointers */
                        if ( !comtypes(DECREF(t1), DECREF(t2), o) ) {
                                if ( talign(DECREF(t1)) < talign(DECREF(t)) ) {
                                        t = t1;
                                }
                        }
                } else {
                        /* "illegal combination of pointer and integer, op %s" */
                        WERROR( devdebug[COMPATIBLE], MESSAGE(53), opst[o] );
                }
                break;
        }

        p->in.left = makety(p->in.left, t);
        p->in.right = makety(p->in.right, t);

        if ( o != MINUS && !logop(o) ) {
                if ( o == ASSIGN ) {
                        p->in.type = unqualtype(t);
                } else {
                        p->in.type = t;
                }
        }

        return( clocal(p) );
}


/* -------------------- nullptr -------------------- */

nullptr( p )
register NODE *p;
{
        register TPTR t = p->in.type;

        if ( ISPTR(t) && TOPQTYPE(t = DECREF(t)) == TVOID ) {
                /* Qualified void pointers are NOT null pointer constants. */
                if ( p->in.op == PCONV ) {
                        /* Skip past the "void *" PCONV node. */
                        p = p->in.left;
                }
        }
        return( p->in.op == ICON && p->tn.rval == NONAME && p->tn.lval == 0 );
}


int     tdebug = 0;

/* -------------------- prmtint -------------------- */

TWORD
prmtint(q)
NODE *q;
{
        TWORD lnt = TOPTYPE(q->in.type);

        switch (lnt) {
        case UCHAR:
        case CHAR:
#if     SZINT > SZCHAR
                if ( devdebug[PROMOTION] ) {
                        lnt = INT;
                } else {
                        lnt = UNSIGNED;
                }
#else
                lnt = UNSIGNED;
#endif  
                break;
        case SCHAR:
                lnt = INT;
                break;
        case SHORT:
                lnt = INT;
                break;
        case USHORT:
#if     SZINT > SZSHORT
                if ( devdebug[PROMOTION] ) {
                        lnt = INT;
                } else {
                        lnt = UNSIGNED;
                }
#else
                lnt = UNSIGNED;
#endif  
        default:
                /*
                 * bitfields are promoted into INT if there values
                 * could be represented as an INT otherwise they are UNSIGNED.
                 * Thus, an UNSIGNED bitfield of size equal to INT has to stay
                 * as UNSIGNED, but smaller UNSIGNED bitfields can be 
                 * promoted into INTs.
                 */
                if (q->in.op == FLD) {
                        /*
                         * the size of a bitfield is packed in the
                         * lower six bits of the rval field in the FLD node.
                         * Note: this assumes that the size of a bitfield cannot
                         * be bigger than FLDSIZ bits. see falloc().
                         */
                        if ( ISTUNSIGNED(lnt) ) {
                                if ( ( q->tn.rval & FLDSIZ ) < SZINT && 
                                    devdebug[PROMOTION] ) {
                                        lnt = INT;
                                } else {
                                        lnt = UNSIGNED;
                                }
                        } else {
                                lnt = INT;
                        }
                }
                /*
                 * else, leave the types unchanged.
                 */
        }

        /*
         * the switch defaults to passing the type unchanged.
         */
        return(lnt);
}

/* do integral promotions on left and right operands. */

static TWORD integral (register TWORD nt1, register TWORD nt2) {
       if (nt1 == ULNGLNG || nt2 == ULNGLNG)
           return ULNGLNG;
       if (nt1 == LNGLNG || nt2 == LNGLNG)
           return LNGLNG;
       if (nt1 == ULONG || nt2 == ULONG)
           return ULONG;
       if (nt1 == LONG || nt2 == LONG) {
#if !(SZLONG > SZINT)
           if (nt1 == UNSIGNED || nt2 == UNSIGNED)
               return ULONG;
#endif  
           return LONG;
       }
       if (nt1 == UNSIGNED || nt2 == UNSIGNED)
           return UNSIGNED;
       return INT;
}

/* -------------------- tymatch -------------------- */

NODE *
tymatch(p)
register NODE *p;
{

        register TWORD t1, t2, t;
        register o, u;

        o = p->in.op;

        t  = TOPTYPE(p->in.type);
        t1 = TOPTYPE(p->in.left->in.type);
        t2 = TOPTYPE(p->in.right->in.type);

        /*
         * Check for type compatibility.
        */
        if ( t2 == PTR && o != CAST ) {
                /* "illegal combination of pointer and integer, op %s" */
                WERROR( devdebug[COMPATIBLE], MESSAGE(53), opst[o] );
        }

        /*
         * checking that enums are of the same type
         */
        if ( t1 == ENUMTY && t2 == ENUMTY && o != CAST ) {
                if ( p->in.left->in.type->typ_size != 
                    p->in.right->in.type->typ_size ) {
                        WARNING( WHEURISTIC, MESSAGE(37), opst[o] );
                }
                if ( !asgop(o) ) {
                        econvert( p->in.left );
                        econvert( p->in.right );
                }
        }

        if (devdebug[PROMOTION]) {      /* ansi value preserving promotions */
                register TWORD nt, nt1, nt2;

                /* initializations */
                nt  = t;
                nt1 = t1;
                nt2 = t2;

                /* New value preserving rules */

                /* the assumption is that when we're done computing the
                   promoted types, the new uniform type to be used is in nt
                */

                /* Floating then integral promotion rules */
                nt = ((t1 == LDOUBLE || t2 == LDOUBLE) ? LDOUBLE :
                       (t1 == DOUBLE || t2 == DOUBLE) ? DOUBLE :
                        (t1 == FLOAT || t2 == FLOAT) ? FLOAT :
                         integral (prmtint(p->in.left), prmtint(p->in.right)));

#ifndef BUG1
                if ( tdebug ) {
                        printf( "tymatch %o) op=%s, t=%o, t1=%o, t2=%o ==> nt=%o, nt1=%o, nt2=%o\n",
                            p, opst[o], t, t1, t2, nt, nt1, nt2);
                }
#endif  !BUG1

                /* operator specific type matching.
                 * this may supercede any type calculations performed above.
                 */

                switch (o) {
                case LS:
                case RS:
                        if (nt1 != t1)
                                p->in.left = makety(p->in.left, tyalloc(nt1));
                        if (nt2 != t2)
                                p->in.right = makety(p->in.right, tyalloc(nt2));
                        p->in.type = tyalloc(nt1);
#ifndef BUG1
                        nt = nt1;       /* to synchronize the types */
#endif  !BUG1
                        break;

                case RETURN:
                case CAST:
                case STASG: /* this should not be ever here, but to be safe */
                case PARAMETER:
                case ASSIGN:
                        /*
                         * these are special cases of ASGOP operators
                         * make t = t2 = t1
                         */
                        nt = nt1 = nt2 = t1;    /* to synchronize the types */
                        p->in.right = makety(p->in.right, p->in.left->in.type);
                        p->in.type = p->in.left->in.type;
                        break;

                default:
                        if (asgop(o)) {
/*
 compound assignment operators: i.e., op='s ++E, and --E.
 Note: E++ E-- are converted into E += 1 and E -= 1, respectively.

 The type of the result is the unchanged type of the LHS.
 It is assumed that the code generator knows how to produce
 code that evaluates the operands of the assignment as promoted
 but stores the result as the un-promoted type of the LHS.

 convert the RHS to the common type, but leave LHS type as is,
 up to the code generators discretion.
*/
                            if (t2 != nt)
                                p->in.right = makety(p->in.right, tyalloc(nt));
                            p->in.type = p->in.left->in.type;
#ifndef BUG1
                            nt = t1;        /* to synchronize the types */
#endif  !BUG1
                        } else {
                            /* default arithmetic conversions are done here */

                            nt1 = nt2 = nt; /* propagate the common type */

                            /* call makety to produce the proper conversion
			       node if the type of the left (right) node
			       is different from the common type. */
                            if (nt1 != t1)
                                p->in.left = makety(p->in.left,tyalloc(nt1));
                            if (nt2 != t2)
                                p->in.right = makety(p->in.right,tyalloc(nt2));
                            if (logop(o))
                                /* for relational ops result has type INT
				   and not the common type precomputed. */
                            nt = INT;
                            p->in.type = tyalloc(nt);
                        }
                }
#ifndef BUG1
                if ( tdebug )
                        printf( "tymatch(%o): %o %s %o => %o\n",
                            p, t1, opst[o], t2, nt );
#endif  !BUG1
                return( p );
        } else {
                /* this is the original tymatch code */
                /* satisfy the types of various arithmetic binary ops */

                /* rules are:
                        if assignment, op, type of LHS
                        if any float or doubles, make double
                        if any longs, make long
                        otherwise, make int
                        if either operand is unsigned, the result is...
                */
                register TPTR tu;
                int     noleft = 0; /* flag for RETURN Kludge */

                u = 0;
                if (ISTUNSIGNED(t1)) {
                        u = 1;
                        t1 = DEUNSIGN(t1);
                }
                if (ISTUNSIGNED(t2) && !(dope[o] & SHFFLG)) {
                        u = 1;
                        t2 = DEUNSIGN(t2);
                }

                /* hack: force conversion in case of reg = exp,
		   but don't bother for any other assignments */
                if ( ( t1 == SCHAR || t1 == SHORT ) && o != RETURN )
                        if (!asgop(o) || p->in.left->tn.op != REG)
                                t1 = INT;

                if ( t2 == SCHAR || t2 == SHORT )
                        t2 = INT;

                if ( o == RETURN && (t1 == DOUBLE || t1 == FLOAT)) {
                        t = DOUBLE;
                        noleft = 1;
                } else if ((t1 == DOUBLE && t2 == FLOAT) || (t2 == DOUBLE && t1 == FLOAT))
                        t = DOUBLE;
                else if ( t1 == DOUBLE || t1 == FLOAT )
                        t = t1;
                else if ( t2 == DOUBLE || t2 == FLOAT )
                        t = t2;
		/* LS: added LNGLNG and tried to keep it consistent */
                else if ( t1 == LNGLNG || t2 == LNGLNG )
                        t = LNGLNG;
                else if ( t1 == LONG || t2 == LONG )
                        t = LONG;
                else
                        t = INT;

#ifndef BUG1
                if ( tdebug ) {
                        printf( "tymatch %o) op=%s, t=%o, t1=%o, t2=%o, u=%o\n",
                            p, opst[o], t, t1, t2 , u );
                }
#endif  !BUG1

                if ( asgop(o) && !noleft ) {
                        tu = p->in.left->in.type;
                        t = t1;
                } else {
                        tu = tyalloc((u && UNSIGNABLE(t)) ? ENUNSIGN(t) : t);
                }

                /* because expressions have values that are at least as wide
                   as INT or UNSIGNED, the only conversions needed
                   are those involving FLOAT/DOUBLE, and those
                   from LONG to INT and ULONG to UNSIGNED
		   LS: LNGLNG to INT and ULNGLNG to UNSIGNED should be added,
		   but where is the code that does this ??????
                 */

                if ( o != RETURN )
                        p->in.left = makety(p->in.left, tu);

                /* for 'op=' the type casting has to be done in the second pass (lec) */
                if ( ((!(dope[o] & ASGOPFLG) || o == RETURN) && t != t2) || o == CAST )
                        p->in.right = makety(p->in.right, tu);

                if ( asgop(o) && !noleft ) {  /* lec */
                        p->in.type = p->in.left->in.type;
                } else if ( !logop(o) ) {
                        p->in.type = tu;
                }

#ifndef BUG1
                if ( tdebug )
                        printf( "tymatch(%o): %o %s %o => %o\n",
                            p, t1, opst[o], t2, TOPTYPE(tu) );
#endif  !BUG1

                return(p);
        }
}


/* -------------------- makety -------------------- */

NODE *
makety( p, t )
register NODE *p;
TPTR t;
{
        /* make p into type t by inserting a conversion */

        if (TOPTYPE(p->in.type) == ENUMTY && p->in.op == ICON)
                econvert(p);
        if (comtypes(p->in.type, t, 0)) {
                p->in.type = t;
                return( p );
        }

        if (!ISBTYPE(t)) {
                /* non-simple type */
                return( block( PCONV, p, NIL, t ) );
        }

        p = block( SCONV, p, NIL, t );

        if ( ISCONSTANT(p->in.left) && ISINTEGRAL(t) )
                p = docast( p );

        return( p );
}


/* -------------------- block -------------------- */

NODE *
block( o, l, r, t )
register NODE *l, *r;
TPTR t;
{

        register NODE *p;

        p = talloc();
        p->in.op = o;
        p->in.left = l;
        p->in.right = r;
        p->in.type = t;
        p->tn.flags = 0;
        return p;
}


/* -------------------- icons -------------------- */

CONSZ icons(NODE *p)
{
        /* if p is an integer constant, return its value */
        CONSZ  val;

        if ( p->in.op == FCON ) {
                /* "floating point expression
		   in an integral constant expression" */
                UERROR( ALWAYS, MESSAGE(151) );
                val = 1;
        } else if ( p->in.op != ICON ) {
                /* "constant expected" */
                UERROR( ALWAYS, MESSAGE(23) );
                val = 1;
        } else {
                val = p->tn.lval;
                /* "constant too big for cross-compiler" */
                if ( val != p->tn.lval )
                        UERROR( ALWAYS, MESSAGE(25) );
        }
   

        tfree( p );
        return val;
}


/*      the intent of this table is to examine the operators,
	and to check them for correctness.

        The table is searched for the op and the modified type
	(where this is one of the types INT (includes char and short),
	LONG, DOUBLE (includes FLOAT), and POINTER
	LS: The table seems to have passed away, so I have added long long to
	the moditype function, which, I take it, now performs this function.

        The default action is to make the node type integer

        The actions taken include:
                CVTL      convert the left operand
                CVTR      convert the right operand
                TYPL      the type is determined by the left operand
                TYPR      the type is determined by the right operand
                TYMATCH   force type of left and right to match,
                                by inserting conversions
                PTMATCH   like TYMATCH, but for pointers
                LVAL      left operand must be lval
                CVTO      convert the op
                NCVT      do not convert the operands
                NCVTR     do not convert the right operand
                OTHER     handled by code

*/

#define MINT 01         /* integer */
#define MDBI 02         /* integer or double */
#define MSTR 04         /* structure */
#define MPTR 010        /* pointer */
#define MPTI 020        /* pointer or integer */

/* -------------------- opact -------------------- */

opact( p )
NODE *p;
{

        register mt12, mt1, mt2, o;

        mt12 = mt2 = -1;

        switch ( optype(o = p->in.op) ) {

        case BITYPE:
                mt12 = mt2 = moditype( p->in.right->in.type );
                if ( bdebug )
                        printf( "opact: mt2 = %#o\n", mt2 );
        case UTYPE:
                mt12 &= (mt1 = moditype( p->in.left->in.type ));
                if ( bdebug )
                        printf( "opact: mt1 = %#o\n", mt1 );

        }

        if ( bdebug ) {
                printf( "opact: mt12 = %#o\n", mt12 );
                printf( "opact: optype = '%s'\n", opst[o] );
        }

        switch ( o ) {

        case NAME :
        case STRING :
        case WSTRING :
        case CALL :
        case UNARY CALL:
        case UNARY MUL:
                return( OTHER );
        case UNARY MINUS:
                if ( mt1 & MDBI )
                        return( TYPL );
                break;

        case COMPL:
                if ( mt1 & MINT )
                        return( TYPL );
                break;

        case UNARY AND:
                return( NCVT + OTHER );
        case CBRANCH:
        case NOT:
                if ( mt1 & (MDBI | MPTI) )
                        return( 0 );
                break;
        case INIT:
        case CM:
                return( 0 );
        case ANDAND:
        case OROR:
                if ( ( mt1 & (MDBI | MPTI) ) && ( mt2 & (MDBI | MPTI) ) )
                        return( 0 );
                break;

        case MUL:
        case DIV:
                if ( mt12 & MDBI )
                        return( TYMATCH );
                break;

        case MOD:
        case AND:
        case OR:
        case ER:
                if ( mt12 & MINT )
                        return( TYMATCH );
                break;

        case LS:
        case RS:
                if ( mt12 & MINT )
                        return( TYMATCH + OTHER );
                break;

        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
                if ( mt12 & MDBI )
                        return( TYMATCH + CVTO );
                if ( mt12 & MPTI )
                        return( PTMATCH );
                break;

        case QUEST:
                if ( mt1 & (MDBI | MPTI) )
                        return( TYPR );
                break;

        case COMOP:
                return( TYPR );

        case STREF:
                return( NCVTR + OTHER );

        case FORCE:
                return( TYPL );

        case COLON:
                if ( mt12 & MDBI )
                        return( TYMATCH );
                if ( mt12 & MPTI )
                        return( PTMATCH );
                if ( mt12 & MSTR )
                        return( TYPL + OTHER );
                if ( ( mt1 == 0 ) && ( mt2 == 0 ) )
                        return( TYPL );
                break;

        case ASSIGN:
        case PARAMETER:
        case RETURN:
                if ( mt12 & MSTR )
                        return( LVAL + TYPL + OTHER );
        case CAST:
                if ( mt12 & MDBI )
                        return( LVAL + TYMATCH );
                if ( mt12 & MPTI ) {
                        if ( mt1 & MPTR )
                                return( LVAL + PTMATCH );
                        else
                                return( LVAL + TYMATCH );
                }
                if ( mt1 == 0 && ( o == CAST || o == RETURN ) )
                        return( LVAL + TYMATCH );
                break;

        case ASG_LS:
        case ASG_RS:
                if ( mt12 & MINT )
                        return( LVAL + TYPL + OTHER );
                break;

        case ASG_MUL:
        case ASG_DIV:
                if ( mt12 & MDBI )
                        return( LVAL + TYMATCH );
                break;

        case ASG_MOD:
        case ASG_AND:
        case ASG_OR:
        case ASG_ER:
                if ( mt12 & MINT )
                        return( LVAL + TYMATCH );
                break;

        case ASG_PLUS:
        case ASG_MINUS:
        case INCR:
        case DECR:
                if ( mt12 & MDBI )
                        return( LVAL + TYMATCH );
                if ( ( mt1 & MPTR ) && ( mt2 & MINT ) )
                        return( LVAL + CVTR );
                break;

        case MINUS:
                if ( mt12 & MPTR )
                        return( PTMATCH + CVTO );
                if ( mt2 & MPTR )
                        break;
        case PLUS:
                if ( mt12 & MDBI )
                        return( TYMATCH );
                if ( ( mt1 & MPTR ) && ( mt2 & MINT ) )
                        return( CVTR );
                if ( ( mt1 & MINT ) && ( mt2 & MPTR ) )
                        return( CVTL );
                break;

        default:
                cerror(TOOLSTR(M_MSG_205, "unknown opact operation %d"), o );
        }

        if ( bdebug )
                printf( "opact: incompatible %s %s %s\n",
                    opst[p->in.left->in.op],
                    opst[o], opst[p->in.right->in.op] );

        if ( mt1 == 0 || mt2 == 0 )
                /* "void type illegal in expression" */
                UERROR( ALWAYS, MESSAGE(118) );
        else if ( optype(o) == BITYPE )
                /* "operands of %s have incompatible types" */
                UERROR( ALWAYS, MESSAGE(89), opst[o] );
        else
                /* "operand of %s has illegal type" */
                UERROR( ALWAYS, MESSAGE(188), opst[o] );
        return( NCVT );
}


/* -------------------- moditype -------------------- */

moditype( ty )
TPTR ty;
{

        switch (TOPTYPE(ty)) {
        case TVOID:
                return( 0 );

        case STRTY:
        case UNIONTY:
                return( MSTR );

        case CHAR:
        case SCHAR:
        case SHORT:
        case INT:
        case LONG:
        case LNGLNG:
        case UCHAR:
        case USHORT:
        case UNSIGNED:
        case ULONG:
        case ULNGLNG:
        case ENUMTY:    /* make enumerations like ints */
        case MOETY:     /* make enumerations like ints */
                return( MINT | MDBI | MPTI );
        case FLOAT:
        case DOUBLE:
        case LDOUBLE:
                return( MDBI );

        default:
                return( MPTR | MPTI );

        }
}


/* -------------------- doszof -------------------- */

NODE *
doszof( p )
register NODE *p;
{
        /* do sizeof p */
        register int    i;

        if (p->in.op == FLD)
                /* cannot take size of a bit field */
                UERROR( devdebug[ANSI_MODE], MESSAGE(184) );

        i = tsize( p->in.type ) / SZCHAR;

        tfree(p);
        if ( i <= 0 )
                cerror(TOOLSTR(M_MSG_206, "sizeof returns 0" ));

        p = bcon( i );
        /* sizeof is an unsigned ICON, change the type here and */
        /* look for it when folding constants in conval         */
        p->in.type = tyalloc(UNSIGNED);
        return( p );
}


#ifndef BUG2

# ifdef SDBDEBUG

/* -------------------- DBprint -------------------- */

DBprint(p)      /* for debugging in sdb */
register NODE *p;
{
        fwalk( p, eprint, 0 );
}


# endif SDBDEBUG

/* -------------------- eprint -------------------- */

eprint( p, down, a, b )
register NODE *p;
int     *a, *b;
{
        register ty;

        *a = *b = down + 1;
        while ( down > 1 ) {
                printf( "\t" );
                down -= 2;
        }
        if ( down )
                printf( "    " );

        ty = optype( p->in.op );

        printf("%#o) %s, ", p, opst[p->in.op] );
        if ( ty == LTYPE ) {
	  /* LS: HACK -> remove cast when printf learns about long long */
                printf( CONFMT, (int)p->tn.lval );
                printf( ", %d, ", p->tn.rval );
        }
        tprint( p->in.type );
        printf( "\n" );
}
#endif  !BUG2


/* -------------------- prtdcon -------------------- */

prtdcon( p )
register NODE *p;
{
        register int    i;

        if ( p->in.op == FCON ) {
                if ( !SETDCON(p) ) { /* does code generator support FCON */
                        locctr( DATA );
                        defalign( ALDOUBLE );
                        deflab( i = getlab() );
#ifndef NOFLOAT
                        fincode( p->fpn.dval, SZDOUBLE );
#endif  !NOFLOAT
                        p->tn.lval = 0;
                        p->tn.rval = -i;
                        p->in.type = tyalloc(DOUBLE);
                        p->in.op = NAME;
                }
        }
}


/* -------------------- ecomp -------------------- */

int     edebug = 0;

ecomp( p )
register NODE *p;
{
        if ( edebug )
                fwalk( p, eprint, 0 );
        if ( !reached && !lintnrch ) {
                /* "statement not reached"  */
                WARNING( WREACHED, MESSAGE(100) );
                reached = 1;
        }
        p = optim(foldexpr(p));
        walkf( p, prtdcon );
        locctr( PROG );
        if ( edebug )
                fwalk( p, eprint, 0 );
        ecode( p );
        tfree(p);
}


#ifdef  STDPRTREE
# ifndef ONEPASS

/* -------------------- prtree -------------------- */

#  ifdef        XCOFF
int     call_flag = 0;
#  endif        XCOFF

prtree(p)
register NODE *p;
{
#  ifdef        IS_COMPILER
        register struct symtab *q;
        register ty;
        register op;

#   ifdef       MYPRTREE
        MYPRTREE(p);  /* local action can be taken here; then return... */
#   endif       MYPRTREE

        if ( p->in.op == ICON && p->tn.rval != NONAME )
                p->in.op = ADDR;

        op = p->in.op;
        ty = optype(op);

        printf("%d\t%o\t", op, tyencode(p->in.type));

        if ( ty == LTYPE ) {
                if ( op == FCON ) {
                        fltprint(p);    /* machine dependent */
                } else {
	  /* LS: HACK -> remove cast when printf learns about long long */
                        printf( CONFMT, (int)p->tn.lval );
                        fputs("\t", stdout);
                }
        }
        if ( ty != BITYPE ) {
                switch (op) {
                case ICON:
                        printf( "0\t" );
                        break;

                case NAME:
                case LNAME:
                case PNAME:
                case ADDR:
                        if ( p->tn.rval != NONAME && p->tn.rval >= 0 ) {
                                q = &stab[p->tn.rval];
                                printf( "%d\t", q->uniqid );
                        } else if ( op == NAME && p->tn.rval == NONAME ) {
                                /* So optimizer can recognize this as a
                                 * "no-name" NAME
                                 */
                                printf( "0\t" );
                        } else
                                printf( "%d\t", p->tn.rval );
                        break;

                case FCON:
                        break;

                default:
                        printf( "%d\t", p->tn.rval );
                        break;
                }
        }

        /* handle special cases */

        switch ( op ) {

        case ADDR:
        case LNAME:
        case PNAME:
        case NAME:
        case ICON:
                /* print external name */
                if ( p->tn.rval == NONAME )
                        printf( "\n" );
                else if ( p->tn.rval >= 0 ) {
                        q = &stab[p->tn.rval];
#   ifdef       XCOFF
                        if ( ISFTN(q->stype) )
                                if ( call_flag ) {
                                        printf( ".%s[pr]\n", q->psname );
                                        q->sflags |= SFCALLED;
                                }
                                else {
                                        q->sflags |= SFADDR;
                                        printf( "T.%s\n", q->psname );
                                }
                        else
                                switch ( q->sclass ) {
                                case EXTDEF:
                                case EXTENT:
                                case EXTERN:
                                        printf( "T.%s\n", q->psname );
                                        break;
                                case STATIC:
                                case USTATIC:
                                        printf( "_%s\n", q->psname );
                                        break;
                                default:
                                        printf( "%s\n", q->psname );
                                        break;
                                }
#   else
                        printf(  "_%s\n", q->psname );
#   endif       XCOFF
                } else { /* label */
#   ifdef       XCOFF
                        fputs("_", stdout);
#   endif       XCOFF
                        printf( LABFMT, -p->tn.rval );
                        fputs("\n", stdout);
                }
                break;

        case STARG:
        case STASG:
        case STCALL:
        case UNARY STCALL:
                /* print out size */
                /* use lhs size, in order to avoid hassles with the  */
                /* structure `.' operator */

                /* note: p->in.left not a field... */
                printf( CONFMT, (CONSZ)tsize(btype(p->in.left->in.type)));
                printf("\t%d\t\n", talign(btype(p->in.left->in.type)));
                break;

        default:
                printf(  "\n" );
        }

#   ifdef       XCOFF
        call_flag = ( op == CALL ) || ( op == UNARY CALL ) || 
            ( op == STCALL ) || ( op == UNARY STCALL );
#   endif       XCOFF

        if ( ty != LTYPE )
                prtree( p->in.left );
        if ( ty == BITYPE )
                prtree( p->in.right );

#  endif        IS_COMPILER
}

# else

/* -------------------- p2tree -------------------- */

p2tree(p)
register NODE *p;
{
        register ty;
        extern int      zdebug;

#  ifdef        MYP2TREE
        MYP2TREE(p);  /* local action can be taken here; then return... */
#  endif        MYP@TREE

        if (zdebug > 1)
                fwalk( p, eprint, 0 );
        ty = optype(p->in.op);

        switch ( p->in.op ) {

        case LNAME:
        case PNAME:
        case NAME:
        case ICON:
                if ( p->tn.rval == NONAME )
                        p->in.pname = 0;
                else if ( p->tn.rval >= 0 ) { /* copy external name */
                        register char   *cp;
                        cp = stab[p->tn.rval].psname;
                        p->in.pname = getmem( strlen(cp) + 2);
                        *p->in.pname = '_';
                        strcpy( p->in.pname + 1, cp);
                } else {
                        p->in.pname = getmem( LABSIZE + 1);
                        sprintf( p->in.pname, LABFMT, -p->tn.rval );
                }
        case FCON:
                break;

        case STARG:
        case STASG:
        case STCALL:
        case UNARY STCALL:
                /* set up size parameters */
                p->stn.stsize = (tsize(btype(p->in.left->in.type)) + SZCHAR - 1)
                 / SZCHAR;
                p->stn.stalign = talign(btype(p->in.left->in.type)) / SZCHAR;
                break;

        case REG:
                rbusy( p->tn.rval, p->in.type );

        default:
                p->in.pname = 0;
        }

        p->in.rall = NOPREF;

        if ( ty != LTYPE )
                p2tree( p->in.left );
        if ( ty == BITYPE )
                p2tree( p->in.right );
}


# endif ONEPASS
#endif  STDPRTREE

/* -------------------- andable -------------------- */

/* Check for address of register declarator. */
andable( p )
NODE *p;
{
        if ( p->in.flags & NOTLVAL )
                /* cannot take address of register variable */
                WERROR( ALWAYS, MESSAGE(149) );
}


/* -------------------- contx -------------------- */

/* contx - check context of node
 *      contx is called for each node during tree walk (fwalk);
 *      it complains about nodes that have null effect.
 *      VAL is passed to a child if that child's value is used
 *      EFF is passed to a child if that child is used in an effects context
 *
 *      arguments:
 *              p - node pointer
 *              down - value passed down from ancestor
 *              pl, pr - pointers to values to be passed down to descendants
 */
contx( p, down, pl, pr )
register NODE *p;
register *pl, *pr;
{
        *pl = *pr = VAL;
        switch ( p->in.op ) {

                /* left side of ANDAND, OROR, and QUEST always evaluated for value
                   (value determines if right side is to be evaluated) */
        case ANDAND:
        case OROR:
        case QUEST:
                *pr = down;
                break;

                /* left side and right side treated identically */
        case SCONV:
        case PCONV:
        case COLON:
                *pr = *pl = down;
                break;

                /* comma operator uses left side for effect */
        case COMOP:
                *pl = EFF;
                *pr = down;

        case FORCE:
        case INIT:
        case UNARY CALL:
        case STCALL:
        case UNARY STCALL:
        case CALL:
        case UNARY FORTCALL:
        case FORTCALL:
        case CBRANCH:
                break;

        default:
                /* assignment ops are OK */
                if ( asgop(p->in.op) )
                        break;

                /* struct x f( );  main( ) {  (void) f( ); }
                         *  the the cast call appears as U* UNDEF
                         */
                if ( p->in.op == UNARY MUL && 
                    ( TOPTYPE(p->in.type) == STRTY
                     || TOPTYPE(p->in.type) == UNIONTY
                     || TOPTYPE(p->in.type) == UNDEF) )
                        break;  /* the compiler does this... */

                /* found a null effect ... */
                if ( down == EFF )
                        WARNING( WNULLEFF || WHEURISTIC, MESSAGE( 86 ) );
        }
}


/* -------------------- WarnWalk -------------------- */

#define VALSET 01
#define VALUSED 02
#define VALASGOP 04
#define VALADDR 010

/*
 * Walk through node, looking for mostly picky warnings about portability and
 * symbol usage.
 */
WarnWalk( p, down, uses )
register NODE *p;
{
        register struct symtab *q;
        register id;
        register down1, down2;
        register use1, use2;
        register struct lnm *np1, *np2;

#ifdef  IS_COMPILER
        if ( !( WUCOMPAR || WPORTABLE || WUSAGE || WEORDER ) )
                return;
#endif  IS_COMPILER

        /* first, set variables which are set... */
        use1 = use2 = VALUSED;
        if ( p->in.op == ASSIGN )
                use1 = VALSET;
        else if ( p->in.op == UNARY AND )
                use1 = VALADDR;
        else if ( asgop( p->in.op ) ) { /* =ops */
                use1 = VALUSED | VALSET;
                if ( down == EFF )
                        use1 |= VALASGOP;
        }


        /* print the lines for lint */

        down2 = down1 = VAL;

        switch ( p->in.op ) {

        case EQ:
        case NE:
                if ( ISUNSIGNED(p->in.left->in.type) && 
                    p->in.right->in.op == ICON &&
                    p->in.right->tn.lval < 0 && 
                    p->in.right->tn.rval == NONAME && 
                    !ISUNSIGNED(p->in.right->in.type) )
                        /* "comparison of unsigned with negative constant" */
                        WARNING( WUCOMPAR, MESSAGE( 21 ) );
                goto charchk;

        case GT:
        case GE:
        case LT:
        case LE:
                if ( TOPTYPE(p->in.left->in.type) == CHAR &&
                        p->in.right->in.op == ICON && 
                        p->in.right->tn.lval == 0 )
                        /* "nonportable character comparison" */
                        WARNING( WPORTABLE, MESSAGE( 82 ) );
charchk:
                if ( TOPTYPE(p->in.left->in.type) == CHAR &&
                        p->in.right->in.op == ICON &&
                        p->in.right->tn.lval < 0 )
                        /* "nonportable character comparison" */
                        WARNING( WPORTABLE, MESSAGE( 82 ) );
                break;

        case UGE:
        case ULT:
                if ( p->in.right->in.op == ICON &&
                        p->in.right->tn.lval == 0 &&
                        p->in.right->tn.rval == NONAME ) {
                        /* "degenerate unsigned comparison" */
                        WARNING( WUCOMPAR, MESSAGE( 30 ) );
                        break;
                }

        case UGT:
        case ULE:
                if ( p->in.right->in.op == ICON &&
                        p->in.right->tn.rval == NONAME &&
                        !ISUNSIGNED( p->in.right->in.type ) ) {

                        if ( p->in.right->tn.lval < 0 )
                                /* "comparison of unsigned with negative constant" */
                                WARNING( WUCOMPAR, MESSAGE( 21 ) );

                        if ( p->in.right->tn.lval == 0 )
                                /* "unsigned comparison with 0?" */
                                WARNING( WUCOMPAR, MESSAGE( 115 ) );
                }
                break;

        case COMOP:
                down1 = EFF;

        case ANDAND:
        case OROR:
        case QUEST:
                down2 = down;
                /* go recursively left, then right  */
                np1 = lnp;
                WarnWalk( p->in.left, down1, use1 );
                np2 = lnp;
                WarnWalk( p->in.right, down2, use2 );
                WarnMerge( np1, np2, 0 );
                return;

        case SCONV:
        case PCONV:
        case COLON:
                down1 = down2 = down;
                break;

        case CALL:
        case STCALL:
        case UNARY CALL:
        case UNARY STCALL:
                {
#ifdef  LINT
                        short   usage = 0;
                        if (down == EFF) {
                                /* RW: P23321. Things work better
                                ** if we 'return' void, rather than
                                ** no return
                                */
                                if (TOPTYPE(p->in.type) == TVOID) {
                                        usage |= LINTUSE;
                                        usage |= LINTRET;
                                } else
                                        usage |= LINTIGN;
                        } else {
                                usage |= LINTUSE;
                        }

                        if (p->in.left->in.op == ICON)
                                OutFtnUsage(&stab[p->in.left->tn.rval], usage);
#endif  LINT
                        break;
                }

        case ICON:
                /* look for &name case */
                if ( (id = p->tn.rval) >= 0 && id != NONAME ) {
                        q = &stab[id];
                        q->sflags |= (SREF | SSET);
#ifdef  CFLOW
                        /* cflow(1) takes referenced extern identifiers. */
                        switch (q->sclass) {
                        case EXTERN:
                        case EXTENT:
                        case EXTDEF:
                        case USTATIC:
                        case STATIC:
                                /* PTM 43047: Added a check to ensure that we are only outputing valid
                                   function calls/definitions. Problem is when the address of a
                                   function is taken, cflow1 used to consider that a function call.
                                   This could, in some cases, cause reduced output of cflow since
                                   cflow outputs heads of trees that are not called themselves.

                                   Example: take this out and try the RS/6000 E-NET driver code that 
                                   is supplied with the OS.
                                   JRW 13/07/90
                                */
                                if (uses & (VALSET | VALUSED) && !ISFTN(q->stype))
                                        OutSymbol(q, 1);
                        }
#endif  CFLOW
                }
                return;

        case NAME:
        case LNAME:
        case PNAME:
                if ( (id = p->tn.rval) >= 0 && id != NONAME ) {
                        q = &stab[id];
#ifdef  CFLOW
                        /* cflow(1) takes referenced extern identifiers. */
                        switch (q->sclass) {
                        case EXTERN:
                        case EXTENT:
                        case EXTDEF:
                        case USTATIC:
                        case STATIC:
                                if ( uses & (VALSET | VALUSED) )
                                        OutSymbol(q, 1);
                        }
#endif  CFLOW
                        if ( (uses & VALUSED) && !(q->sflags & SSET) ) {
                                if ( q->sclass == AUTO || q->sclass == REGISTER || q->sclass == AUTOREG ) {
                                        if ( !ISARY(q->stype ) && !ISFTN(q->stype)
                                             && TOPTYPE(q->stype) != STRTY ) {
                                                /* "%s may be used before set" */
                                                WARNING( WUSAGE && WKNR, MESSAGE( 1 ), q->psname );
                                                q->sflags |= SSET;
                                        }
                                }
                        }
                        if ( uses & VALASGOP )
                                break;  /* not a real use */
                        if ( uses & VALSET )
                                q->sflags |= SSET;
                        if ( uses & VALUSED )
                                q->sflags |= SREF;
                        if ( uses & VALADDR )
                                q->sflags |= (SREF | SSET);
                        if ( p->tn.lval == 0 ) {
                                lnp->lid = id;
                                lnp->flgs = (uses & VALADDR) ? 0 : ((uses & VALSET) ? VALSET : VALUSED);
                                if ( ++lnp >= &lnames[LNAMES] )
                                        --lnp;
                        }
                }
                return;
        }

        /* recurse, going down the right side first if we can */

        switch ( optype(p->in.op) ) {

        case BITYPE:
                np1 = lnp;
                WarnWalk( p->in.right, down2, use2 );
        case UTYPE:
                np2 = lnp;
                WarnWalk( p->in.left, down1, use1 );
        }

        if ( optype(p->in.op) == BITYPE ) {
                if ( p->in.op == ASSIGN &&
                        (p->in.left->in.op == NAME || p->in.left->in.op == LNAME || p->in.left->in.op == PNAME ))
                        /* special case for a =  .. a .. */
                        WarnMerge( np1, np2, 0 );

                else
                        WarnMerge( np1, np2, p->in.op != COLON );

                /* look for assignments to fields, and complain */
                if ( p->in.op == ASSIGN && p->in.left->in.op == FLD && p->in.right->in.op == ICON )
                        FieldBust( p );
        }
}


WarnMerge( np1, np2, flag )
struct lnm *np1, *np2;
{
        /* np1 and np2 point to lists of lnm members, for the two sides
         * of a binary operator
         * flag is 1 if commutation is possible, 0 otherwise
         * WarnMerge returns a merged list, starting at np1, resetting lnp
         * it also complains, if appropriate, about side effects
         */

        register struct lnm *npx, *npy;

        for ( npx = np2; npx < lnp; ++npx ) {

                /* is it already there? */
                for ( npy = np1; npy < np2; ++npy ) {
                        if ( npx->lid == npy->lid ) { /* yes */
                                if ( npx->flgs == 0 || npx->flgs == (VALSET | VALUSED) )
                                        ;  /* do nothing */
                                else if ( (npx->flgs | npy->flgs) == (VALSET | VALUSED)
                                     || (npx->flgs & npy->flgs & VALSET) )
                                        /* "%s evaluation order undefined" */
                                        if ( flag )
                                                WARNING( WEORDER, MESSAGE( 0 ), stab[npy->lid].psname );

                                if ( npy->flgs == 0 )
                                        npx->flgs = 0;
                                else
                                        npy->flgs |= npx->flgs;
                                goto foundit;
                        }
                }

                /* not there: update entry */
                np2->lid = npx->lid;
                np2->flgs = npx->flgs;
                ++np2;

foundit:
                ;
        }

        /* all finished: merged list is at np1 */
        lnp = np2;
}


/* Check assignment of a constant to a field. */
FieldBust( p )
register NODE *p;
{
        /* check to see if the assignment is going to overflow,
          or otherwise cause trouble */
        register s = UPKFSZ(p->in.left->tn.rval);
        CONSZ v = p->in.right->tn.lval;
        TWORD bt = TOPTYPE(p->in.left->in.type);

        if (UNSIGNABLE(bt) || bt == ENUMTY) {
        /* "precision lost in assignment to (possibly sign-extended) field" */
	    if (v < 0 || (v >> --s))
                WARNING( WPORTABLE, MESSAGE( 93 ) );
	}
        else if (ISTUNSIGNED(bt) && (v < 0 || (v >> s)))
        /* "precision lost in field assignment" */
            WARNING( WPORTABLE, MESSAGE( 94 ) );
}


