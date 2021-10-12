static char sccsid[] = "@(#)53  1.13.3.1  src/bos/usr/ccs/bin/common/local.c, cmdprog, bos411, 9428A410j 4/29/93 08:10:22";
/*
 * COMPONENT_NAME: (CMDPROG) local.c
 *
 * FUNCTIONS: CanBeLNAME, LONGLINE, NAMEPTR, addAryID, addTypID, addstabx    
 *            cast, cendarg, cinit, cisreg, clocal, ctype, docast, ecode      
 *            eline, fincode, findAryID, findTypID, fixdef, fltprint          
 *            getTypeID, incode, isarray, isitfloat, isitlong, isptr, offcon  
 *            pTypeID, parray, pptr, prdef, strend, strfind, strname, tlen    
 *            vfdzero                                                         
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
# include <a.out.h>
# include "mfile1.h"
# include "messages.h"

extern FILE *outfile;
extern int lastloc;
extern int eprint();

extern int startln, oldln;
extern char startfn[];
extern int bb_flags[];
extern int xdebug;
extern int gdebug;
#ifndef ddebug
extern int ddebug;
#endif
# define Lineno (lineno-startln+1)

extern char *rnames[];

/* The ordering of this array corresponds to that in mfile1.h */
const char * tnames[NBTYPES] = {
                "null",
                "ellipsis",
                "farg",
                "moety",
                "signed",
                "undef",
                "void",
                "char",
                "schar",
                "short",
                "int",
                "long",
                "long long",
                "float",
                "double",
                "ldouble",
                "uchar",
                "ushort",
                "unsigned",
                "ulong",
                "ulong long",
                "enumty",
                "strty",
                "unionty",
                "class"
};

/*      this file contains code which is dependent on the target machine */

/* -------------------- cast -------------------- */

NODE *
cast( p, t ) register NODE *p; TPTR t; {
        /* cast node p to type t */

        p = buildtree(CAST, block(NAME, NIL, NIL, t), p);
        p->in.left->in.op = FREE;
        p->in.op = FREE;
        return( p->in.right );
}

/* -------------------- noconv -------------------- */

static unsigned noconv (NODE *p, register TWORD m, register TWORD ml) {
    /* meaningful ones are conversion of
       int to char, int to short, short to char, and unsigned version of them.
       fields cannot have the type colored down when signed-ness changes.
    */
    if (p->in.left->in.op == FLD && ISTUNSIGNED(m) != ISTUNSIGNED(ml))
      return FALSE;
    switch (m) {
      case SCHAR:
      case UCHAR:
      case CHAR:
          return ISTCHAR(ml);
      case SHORT:
          /* only get rid of redundent composes of SCONV */
          return ((ISTCHAR(ml) || ml == SHORT || ml == USHORT) &&
		  p->in.left->in.op == SCONV);
      case USHORT:
          /* only get rid of redundent composes of SCONV
             (unsigned short)char is meaningful */
          return ((ml == CHAR || ml == UCHAR || ml == SHORT || ml == USHORT) &&
                  p->in.left->in.op == SCONV);
      case INT:
      case LONG:
          /* put in ISTUNSIGNED, which includes "or of UCHAR", when doing
             signed chars because of patterning, not programmer understanding
	     LS: ISTUNSIGNED now includes ULNGLNG */
          return !((ISPTR(p->in.left->in.type) && blevel) || ISTUNSIGNED(ml));
      case UNSIGNED:
      case ULONG:
          return (ml != INT && ml != LONG);
      case LNGLNG:
      case ULNGLNG:
	  /* LS: we don't clobber any conversions to long long types ? */
      case FLOAT:
      case DOUBLE:
      case LDOUBLE:
          return FALSE;
    } /* switch */
    return TRUE;
}

/* -------------------- clocal -------------------- */

NODE *
clocal(p) NODE *p; {

        /* this is called to do local transformations on
           an expression tree preparitory to its being
           written out in intermediate code.
        */

        /* the major essential job is rewriting the
           automatic variables and arguments in terms of
           REG and OREG nodes */
        /* conversion ops which are not necessary are also clobbered here */
        /* in addition, any special features (such as rewriting
           exclusive or) are easily handled here as well */

        register struct symtab *q;
        register NODE *r;
        register o;
        register TWORD m, ml;
        TPTR mlt;

        switch( o = p->in.op ){

        case NAME:
                if( p->tn.rval < 0 ) { /* already processed; ignore... */
                        return(p);
                        }
                q = &stab[p->tn.rval];
                switch( q->sclass ){

                case AUTO:
                case AUTOREG:
                case PARAM:
                case PARAMREG:
                        if( (q->sflags & (SLNAME|SPNAME)) != 0 )
                        {
                                /* have atomic id:
                                 * turn it into an LNAME or PNAME
                                 */
                                p->in.op = ((q->sflags & SLNAME) != 0)
                                        ? LNAME
                                        : PNAME;
                                r = offcon(q->offset, INCREF(q->stype, PTR));
                                p->tn.lval = r->tn.lval;
                                r->in.op = FREE;
                        }
                        else
                        {
                                /* fake up a structure reference */
                                r = block(REG, NIL, NIL,
                                        INCREF(tyalloc(STRTY), PTR));
                                r->tn.rval = ((q->sclass==AUTO||q->sclass==AUTOREG)?STKREG:ARGREG);
                                r->tn.lval = r->tn.rval; /* supply id cookie */
                                p = stref(block(STREF, r, p, tyalloc(UNDEF)));
                        }
                        if( q->sclass == AUTOREG || q->sclass==PARAMREG )
                                /* Mark tree to detect &x */
                                p->in.flags = NOTLVAL;
                        break;

                case ULABEL:
                case LABEL:
                case STATIC:
                        if( q->sflags & SEXTRN ) break;
                        p->tn.lval = 0;
                        p->tn.rval = -q->offset;
                        break;

                case REGISTER:
                        {
                                /* provide id cooked for optimizer */
                                p->tn.lval = IsRegVar(q->offset)
                                        ? q->uniqid
                                        : q->offset;
                                p->in.op = REG;
                                p->tn.rval = q->offset;
                        }
                        break;

                        }
                break;

        case PCONV:
                if (p->in.left->in.op == ICON ||
		    tsize(p->in.left->in.type) == SZPOINT) {
                    /* pointers all have the same representation;
		       the type is inherited */
                    p->in.left->in.type = p->in.type;
                    p->in.op = FREE;
                    return p->in.left;
                }
                break;

        case SCONV:
                mlt = p->in.left->in.type;
                if (ISFLOAT(p->in.type) != ISFLOAT(mlt))
                    break;

                m = TOPTYPE(p->in.type);
                ml = TOPTYPE(mlt);
  #ifdef LINT
		/* LS: exactly one side is an integral type larger than int */
                if (ISTLONG(m) != ISTLONG(ml) && m != TVOID/*A17067*/) {
/*      GH - 01/31/91 cast of return value of a function from long to void*/
                    /* Aug. 22/90 GH - Since lint can be run on code        
                        compiled with compilers where INT is shorter
                        than LONG, and because lint must report when
                        a LONG is assigned to a SHORT (P47695), the
                        following check has been commented out */
                    if (/*SZLONG > SZINT && */ ISTLONG(ml) && m != UNDEF && !ISTLNGLNG(m))
                        /* "conversion from long may lose accuracy" */
                        WARNING( WLONGASSGN, MESSAGE( 26 ) );

		    /* LS: going from a signed integral to a larger unsigned */
                    if (/*pflag && */ (m == ULONG) && UNSIGNABLE(ml) &&
			p->in.left->in.op != ICON && (ml != LNGLNG))
                        /* "conversion to long may sign-extend incorrectly */
                        WARNING( WLONGASSGN, MESSAGE( 27 ) );
		}
  #endif
                if (NO_FOLD() && p->in.left->in.op == ICON && !ISPTR(mlt)) {
                    /* simulate the conversion here
                       ICON PTR has a name keep the SCONV. test??? */
                    return docast(p);
                }
                if (noconv (p, m, ml)) {
                    if (tlen(p) != tlen(p->in.left) || 
                        ((m == UNSIGNED || m == ULONG) && ISPTR(mlt) && 
                          (TOPTYPE(DECREF(mlt)) == UNSIGNED ||
                          TOPTYPE(DECREF(mlt)) == ULONG))) {
                        p->in.op = FREE;
                        return p->in.left;  /* conversion gets clobbered */
                    }
                    if (p->in.left->in.op != QUEST)  {
                        p->in.left->in.type = p->in.type;
                        p->in.op = FREE;
                        return p->in.left;  /* conversion gets clobbered */
                    }
                }
                break;                /* don't clobber conversion */
        case PVCONV:
        case PMCONV:
                if( p->in.right->in.op != ICON ) cerror(TOOLSTR(M_MSG_248, "bad conversion"));
                p->in.op = FREE;
                return buildtree(o==PMCONV?MUL:DIV, p->in.left, p->in.right);
        case FLD:
                /* make sure that the second pass does not make the
                   descendant of a FLD operator into a doubly indexed OREG */

                if( p->in.left->in.op == UNARY MUL
                                && (r=p->in.left->in.left)->in.op == PCONV)
                        if( r->in.left->in.op == PLUS || r->in.left->in.op == MINUS )
                                if( ISPTR(r->in.type) ) {
                                        if( ISUNSIGNED(p->in.left->in.type) )
                                                p->in.left->in.type =
                                                        tyalloc(UCHAR);
                                        else
                                                p->in.left->in.type =
                                                        tyalloc(CHAR);
                                }
                break;
                }

        return(p);
        }

/* -------------------- docast -------------------- */

NODE *
docast(p)
NODE *p;
{
        /*
         * do casts on constants
         */
        register struct symtab *q;
        register NODE *r;
        register o;
        register TWORD m = TOPTYPE(p->in.type);
        register TWORD ml = TOPTYPE(p->in.left->in.type);

        switch( p->in.op) {
        case SCONV:
                if (p->in.left->in.op == ICON && p->in.left->tn.rval == NONAME) {
                        CONSZ val;
                        val = p->in.left->tn.lval;
                        switch( m ) {
                        case SCHAR:
                                /* if val is neg, make lval a negative long */
                                if( p->in.left->tn.lval & 0X80)
                                    p->in.left->tn.lval = val | 0XFFFFFF00;
                                else
                                    p->in.left->tn.lval = val & 0XFF;
                                break;
                        case UCHAR:
                        case CHAR:
                                p->in.left->tn.lval = val & 0XFF;
                                break;
                        case USHORT:
                                p->in.left->tn.lval = (unsigned short)val;
                                break;
                        case SHORT:
                                p->in.left->tn.lval = (short)val;
                                break;
                        case UNSIGNED:
                                p->in.left->tn.lval = (unsigned)val;
                                break;
                        case INT:
                                p->in.left->tn.lval = (int) val;
                                break;
                        case ULONG:
                                p->in.left->tn.lval = (unsigned long)val;
                                break;
                        case LONG:
                                p->in.left->tn.lval = (long) val;
                                break;
                        case ULNGLNG: /* LS: added long long conversions */
                                p->in.left->tn.lval = (unsigned long long)val;
                                break;
                        case LNGLNG:
                                p->in.left->tn.lval = (long long)val;
                                break;
                        case FLOAT:
                        case DOUBLE:
                        case LDOUBLE:
                                if( ISUNSIGNED(p->in.left->in.type) ) {
                                        unsigned long i;
                                        /*
                                         * warning: there is no unsigned
                                         * integer conversion operation.
                                         */
                                        i = p->in.left->tn.lval;
#ifdef HOSTIEEE
                                        p->in.left->fpn.dval = i & 0x7fffffff;
                                        if(i&0x80000000) {
                                                p->in.left->fpn.dval += 0x1;
                                                p->in.left->fpn.dval +=
                                                                0x7fffffff;
                                        }
#else
                                        _FPi2d(1, i & 0x7fffffff);
                                        if(i&0x80000000) {
                                                _FPaddi(1, 0x1);
                                                p->in.left->fpn.dval = _FPaddi(1, 0x7fffffff);
                                        }
# endif
                                }
                                else {
#ifdef HOSTIEEE
                                        p->in.left->fpn.dval = p->in.left->tn.lval;
#else
                                        p->in.left->fpn.dval = _FPi2d(1, p->in.left->tn.lval);
# endif
                                }
                                p->in.left->in.op = FCON;
                                break;
#ifdef COMPAT
                        case TVOID:
                                if( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ){
                                        p->in.op = FREE;
                                        return( p->in.left );
                                }
#endif
                        default:
                                /*
                                 * otherwise, we have an unknown cast
                                 */
                                return(p);
                        }
                        p->in.left->in.type = p->in.type;
                }
                else if ( p->in.left->in.op == FCON ) {
                        switch(m) {
                        case FLOAT:
#ifdef HOSTIEEE
                                p->in.left->fpn.dval =
                                   (float) p->in.left->fpn.dval;
#else
                                /*
                                 * reads in a double and converts it into
                                 * a float then puts it back into dval.
                                 */
                                p->in.left->fpn.dval =
                                           _FPd2f(1, p->in.left->fpn.dval);
#endif
                                break;
                        case DOUBLE:
                        case LDOUBLE:
#ifdef HOSTIEEE
                                p->in.left->fpn.dval =
                                   (double) p->in.left->fpn.dval;
#else
                                if (TOPTYPE(p->in.left->in.type == FLOAT)) {
                                        p->in.left->fpn.dval =
                                                _FPf2d(1, p->in.left->fpn.dval);
                                }
                                else {
                                /*
                                 * this, essentially, does nothing but
                                 * makeing sure the value in dval is a double.
                                 */
                                        p->in.left->fpn.dval =
                                             _FPcpdi(1, p->in.left->fpn->dval);
                                }
#endif
                                break;
                        default:
                                if( ISINTEGRAL(p->in.type) ){
                                        p->in.left->tn.lval =
#ifdef HOSTIEEE
                                                (long) p->in.left->fpn.dval;
#else
                                                _FPtrdi(1,p->in.left->fpn.dval);
#endif
                                        p->in.left->in.op = ICON;
					/* LS: changed LONG to BIGINT */
                                        p->in.left->in.type = tyalloc(BIGINT);
                                        p = docast(p);
                                }
                                return(p);
                        }
                        p->in.left->in.type = p->in.type;
                }
                break;
        default:
                /*
                 * unknown operation.
                 */
                return(p);
        }
        p->in.op = FREE;
        return(p->in.left);
}

/* -------------------- cendarg -------------------- */

cendarg(){ /* at the end of the arguments of a ftn, set the automatic offset */
        autooff = AUTOINIT;
        }

/* -------------------- cisreg -------------------- */

cisreg (t)  /* is an automatic variable of type t OK for a register variable */
    TPTR t;
{
/* If a NAME becomes a REG, it becomes too hard for lint/cflow to detect. */
#if     !defined (LINT) && !defined (CFLOW)

    extern qdebug;
    TWORD ty;

    ty = TOPTYPE(t);
    if (ty == INT || ty == UNSIGNED || ty == LONG || ty == ULONG ||
        qdebug && (ISTCHAR(ty) || ty == SHORT || ty == USHORT) || ISPTR(t))
        return(1);
    /*
    if (ty != DOUBLE)
        WERROR( ALWAYS, "type clash for register variable" );
    */
#endif
    return(0);
}

/* -------------------- offcon -------------------- */

NODE *
offcon( off, t ) OFFSZ off; TPTR t; {

        /* return a node, for structure references, which is suitable for
           being added to a pointer of type t, in order to be off bits offset
           into a structure */

        register NODE *p;

        /* in general the type is necessary for offcon, but not now */

        p = bcon(0);
        p->tn.lval = off/SZCHAR;
        return(p);

        }

static inwd     /* current bit offsed in word */;
static long word /* word being built from fields */;

/* -------------------- incode -------------------- */

incode( p, sz ) register NODE *p; {

        /* generate initialization code for assigning a constant c
                to a field of width sz */
        /* we assume that the proper alignment has been obtained */
        /* inoff is updated to have the proper final value */
        /* we also assume sz  < SZINT */

        long val = p->tn.lval;

        if((sz+inwd) > SZINT)
                cerror(TOOLSTR(M_MSG_249, "incode: field > int"));
        val &= (1L<<sz)-1;       /* mask to correct size */
        word |= val << (32-sz-inwd);
        inwd += sz;
        inoff += sz;
        if(inoff%SZINT == 0) {
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "\t.long\t0x%lx\n", word);
#endif
                word = inwd = 0;
                }
        }

/* -------------------- fincode -------------------- */

fincode( d, sz )
#ifdef HOSTIEEE
double d;
#else
FP_DOUBLE d;
#endif
{
        /* output code to initialize space of size sz to the value d */
        /* the proper alignment has been obtained */
        /* inoff is updated to have the proper final value */
        /* on the target machine, write it out in octal! */

        outdouble(d, (sz == SZDOUBLE) ? DOUBLE :
                        ( (sz == SZLDOUBLE) ? LDOUBLE : FLOAT) );
        inoff += sz;
        }

/* -------------------- cinit -------------------- */

cinit( p, sz ) NODE *p; {
#ifndef ddebug
        extern int ddebug;
#endif
        /* arrange for the initialization of p into a space of
        size sz */
        /* the proper alignment has been opbtained */
        /* inoff is updated to have the proper final value */
        ecode( p );
        inoff += sz;

        /* let's make sure this isn't illegal initialization - see a1375 */

        if( p->in.op == INIT) {
            if( p->in.left->in.op == ICON ||
                p->in.left->in.op == ADDR ||
                p->in.left->in.op == STADDR ||
                p->in.left->in.op == LADDR ||
                p->in.left->in.op == PADDR  ) return;
            if (p->in.left->in.op == NAME &&
                    TOPTYPE(p->in.left->in.type) == MOE)
                return;
            if(p->in.left->in.op == SCONV) {
                NODE *q;
                q = p->in.left->in.left;
                if( q->in.op == ICON || q->in.op == ADDR || q->in.op == STADDR ||
                    q->in.op == LADDR || q->in.op == PADDR ) return;
                if (q->in.op == NAME && TOPTYPE(q->in.type) == MOE) return;
            }
        }
        if( ddebug ) fwalk( p, eprint, 0 );
        /* illegal initialization - changed for defect 75535 */
        if((blevel > 0) && (p->in.op == INIT))
                if(devdebug[ANSI_MODE])
                        UERROR(ALWAYS, MESSAGE(61));
                else
                        WARNING(WANSI, MESSAGE(61));
        else
                UERROR(ALWAYS, MESSAGE(61));
}

/* -------------------- vfdzero -------------------- */

vfdzero( n ){ /* define n bits of zeros in a vfd */

        if( n <= 0 ) return;

        inwd += n;
        inoff += n;
        if( inoff%ALINT ==0 ) {
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "\t.long\t0x%lx\n", word );
#endif
                word = inwd = 0;
                }
        }
/******* The following seem to be obsolete *******/
#if 0

/* -------------------- ctype -------------------- */

TWORD
ctype (type)    /* map types which are not defined on the local machine */
    TWORD type;
{
    switch (type) {
    case LONG:
        return (INT);
    case ULONG:
        return (UNSIGNED);
    }
    return (type);
}

/* -------------------- isitlong -------------------- */

isitlong( cb, ce ){ /* is lastcon to be long or short */
        /* cb is the first character of the representation, ce the last */

        if( ce == 'l' || ce == 'L' ||
                lastcon >= (1L << (SZINT-1) ) ) return (1);
        return(0);

        }
     
#endif
     
/* -------------------- isitfloat -------------------- */

isitfloat( s ) char *s; {
#ifdef HOSTIEEE
        double atof();
        dcon = atof(s);
#else
        dcon = ieeeatof(s);
# endif
        return( FCON );
        }

/* -------------------- ecode -------------------- */

ecode( p ) NODE *p; {

        /* walk the tree and write out the nodes.. */
        extern int contx(), WarnWalk();

        if( nerrors ) return;

        if( xdebug ) {
                printf( "ecode\n" );
                fwalk( p, eprint, 0 );
        }

        /* Walk the tree, looking for bad code to barf at. */
        if( WNULLEFF )
                fwalk( p, contx, EFF );
        lnp = lnames;
        WarnWalk( p, EFF, 0 );

# ifdef ONEPASS
        p2tree( p );
        p2compile( p );
# else
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t%s\n", EXPR, lineno, ftitle );
#endif
        prtree(p);
# endif
        }

/* -------------------- fixdef -------------------- */

fixdef(p) struct symtab *p; {
#ifndef CXREF
        /* print debugging info
         *
         * don't do params, externs, structure names or members
         * at this time; they are delayed until more information
         * is known about them
         */

        if ( !gdebug )
                return;

        switch( p->sclass ) {

        case STATIC:
                if( p->slevel != 0 )
                        break;
        case EXTENT:
        case USTATIC:
        case PARAM:
        case PARAMREG:
        case PARAMFAKE:
                return;
        case ENAME:
        case STNAME:
        case UNAME:
                addTypID(p->stype->typ_size);
                return;
        case EXTERN:
        case EXTDEF:
        case MOS:
        case MOE:
        case MOU:
                if(isarray(p->stype)) {
                    parray (p);
                }
                else if(isptr(p->stype)){
                    pptr (p, 0);
                }
                return;
        default:
                if( p->sclass & FIELD ) return;
                }

        /* register parameters */
        if( p->slevel == 1 ) return;

        prdef(p,0);
#endif
}

/* local table of fakes for un-names structures
 * typ_size for .ifake is stored in mystrtab[i]
 */
#define FAKENM 300      /* maximum number of fakenames */
#define FAKESIZE 10     /* size of a fake label in chars for printf */
int mystrtab[FAKENM], ifake = 0;
struct symtab mytable;
char tagname[FAKESIZE] = "";

/* table of debugger User-Defined typedefs
 * TypIDtab contains sue/ptr User-Defined typedefs
 * AryIDtab contains array User-Defined typedefs
 * TypIDtab increments from the top of malloced space
 * AryIDtab increments from the bottom of malloced space
 */
extern int *TypIDtab;
extern int *AryIDtab;
extern int *Nxt_TypID;
extern int *Nxt_AryID;

/* -------------------- prdef -------------------- */

# define STABX  0
# define STABA  1
# define STABT  2
# define STABS  3

#ifndef _H_DBXSTCLASS
/***********************************************************************
     STORAGE CLASSES AND TYPE DEFINES MAY BE DELETED WHEN XCOFF.H
     INCLUDED.  THESE DEFINITIONS ARE FOUND IN DBXSTCLASS.H
***********************************************************************/
/*
 *   XCOFF STORAGE CLASSES AND STABSTRINGS DESIGNED SPECIFICALLY FOR DBX
 */
#define DBXMASK                 0x80

#define C_GSYM                  0x80
#define C_LSYM                  0x81
#define C_PSYM                  0x82
#define C_RSYM                  0x83
#define C_RPSYM                 0x84
#define C_STSYM                 0x85
#define C_TCSYM                 0x86
#define C_BCOMM                 0x87
#define C_ECOML                 0x88
#define C_ECOMM                 0x89
#define C_DECL                  0x8c
#define C_ENTRY                 0x8d
#define C_FUN                   0x8e

#define TP_INT          -1
#define TP_CHAR         -2
#define TP_SHORT        -3
#define TP_LONG         -4
#define TP_UCHAR        -5
#define TP_SCHAR        -6
#define TP_USHORT       -7
#define TP_UINT         -8
#define TP_UNSIGNED     -9
#define TP_ULONG        -10
#define TP_VOID         -11
#define TP_FLOAT        -12
#define TP_DOUBLE       -13
#define TP_LDOUBLE      -14
#define TP_PASINT       -15
#define TP_BOOL         -16
#define TP_SHRTREAL     -17
#define TP_REAL         -18
#define TP_STRNGPTR     -19
#define TP_FCHAR        -20
#define TP_LOGICAL1     -21
#define TP_LOGICAL2     -22
#define TP_LOGICAL4     -23
#define TP_LOGICAL      -24
#define TP_COMPLEX      -25
#define TP_DCOMPLEX     -26
#endif

/* LS: currently using long in place of long long, may need new values */
static int defntypes[NBTYPES] = { 0, 0, 0, 0, 0, 0, 0,
        TP_CHAR, TP_SCHAR, TP_SHORT, TP_INT, TP_LONG, TP_LONG,
        TP_FLOAT, TP_DOUBLE, TP_LDOUBLE,
        TP_UCHAR, TP_USHORT, TP_UINT, TP_ULONG, TP_ULONG, 0, 0, 0, 0 };

prdef(p,tsiz) struct symtab *p; int tsiz; {
#ifdef XCOFF
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        /* print symbol definition pseudos
         */
        int class, saveloc, typeID;
        char type = '\0';

        if ( !gdebug )
                return;

        /* print a bb symbol if this is the first symbol in the block */

        if( blevel > 2 && !bb_flags[blevel]
                && p->sclass != LABEL && p->sclass != ULABEL ) {
                printf( "\t.bb\t%d\n", Lineno);
                bb_flags[blevel] = 1;   /* don't let another bb print */
                }

        /* make sure that .defs in functions are in text section */
        if( blevel > 1 )
                saveloc = locctr( PROG );

        typeID = getTypeID(p);

        printf( "\t.stabx\t\"");

        /* translate storage class */

        switch( p->sclass ){
        case AUTO:
        case AUTOREG:
                class = C_LSYM;
                break;
        case EXTDEF:    /* data/undef*/
        case EXTENT:
        case EXTERN:
                class = C_GSYM;
                type = 'G';
                break;
        case STATIC:    /* data/bss */
        case USTATIC:
                class = C_STSYM;
                printf( "_");
                type = (blevel==0) ? 'S' : 'V';
                break;
        case REGISTER:
                class = (blevel==1) ? C_RPSYM : C_RSYM;
                type = 'r';
                break;
        case ULABEL:
        case LABEL:
                class = C_LABEL;
                break;
        case PARAM:
        case PARAMREG:
                class = C_PSYM;
                type = 'p';
                break;
        case TYPEDEF:
                class = C_DECL;
                type = 't';
                break;
        case TCSYM:                     /* not supported yet */
                class = C_TCSYM;
                break;
        case MOS:
        case STNAME:
        case MOU:
        case UNAME:
        case ENAME:
        case MOE:
                class = C_DECL;
                break;
        default:
                if( p->sclass & FIELD )
                        class = C_DECL;
                else
                        cerror(TOOLSTR(M_MSG_250, "bad storage class %d"), p->sclass );
                break;
                }

        printf( "%s:",p->psname);
        if( type=='t') {
                addTypID(0);
                printf ("t%d=", (Nxt_TypID-TypIDtab)*2);
        }
        else if( type )
                printf ("%c", type);
        printf( "%d\"", typeID);

        switch( p->sclass ) {   /* print .val based on storage class */

        case AUTO:
        case AUTOREG:
        case MOS:
        case MOU:
        case PARAM:
        case PARAMREG:
                /* offset in bytes */
                printf( ",%d", p->offset/SZCHAR );
                if (p->sclass==AUTO || p->sclass==AUTOREG) printf("+L.%dL",ftnno);
                else if (p->sclass==PARAM || p->sclass==PARAMREG) printf("+L.%dA",ftnno);
                break;

        case MOE:
                /* internal value of enum symbol */
                printf( ",%d", p->offset );
                break;

        case REGISTER:
                /* offset in bytes in savearea for reg vars */
                /* actual offset determination is deferred to the asembler */
                printf( ",%d", p->offset );
                break;

        case STATIC:
        case USTATIC:
                /* actual or hidden name, depending on scope */
                if( p->sflags & SEXTRN )
                        printf( ",_%s", p->psname);
                else
                        printf( ",_L.%d", p->offset );
                break;
        case LABEL:
        case ULABEL:
        case EXTDEF:
        case EXTENT:
        case EXTERN:
                /* actual or hidden name, depending on scope */
                if( p->sflags & SEXTRN )
                        printf( ",%s", p->psname);
                else
                        printf( ",L.%d", p->offset );
                break;

        case TYPEDEF:
                /* not used */
                printf(",0");
                break;

        default:        if( p->sclass & FIELD ) {
                                /* offset in bits */
                                printf( ",%d", p->offset );
                                break;
                        }
                        else
                                cerror(TOOLSTR(M_MSG_251, "sdb value error on %s\n"), p->psname );
                        break;
                }

        printf( ",%d", class );                 /* class */
        printf(",%d", tyencode(p->stype));      /* type  */

        printf( "\n" );

        if( blevel > 1 )
                locctr( saveloc );
#endif
#endif
}

/* table of debugger User-Defined typedefs
 * TypIDtab contains sue/ptr User-Defined typedefs
 * AryIDtab contains array User-Defined typedefs
 * TypIDtab increments from the top of malloced space
 * AryIDtab increments from the bottom of malloced space
 */
extern int *TypIDtab;
extern int *AryIDtab;
extern int *Nxt_TypID;
extern int *Nxt_AryID;

/* -------------------- getTypeID ----------------- */

getTypeID (p) register struct symtab *p;
{
        int typeID;

        if(isarray(p->stype)) {
        /* labels and arrays have the same type, need to distinguish them
         * here.
         */
                if (p->sclass!=LABEL&&p->sclass!=ULABEL)
                     typeID = parray(p) ;
                else typeID = pTypeID(p);
        }
        else if(isptr(p->stype))
                     typeID = pptr(p,0) ;
        else typeID = pTypeID(p) ;

        return (typeID);
}

/* -------------------- parray -------------------- */

#define MaxArray 4

parray( p ) struct symtab *p;  {
        /* print debugging info for dimensions
         */

        TPTR temp;
        int aryDim[MaxArray];
        int dtemp;
        int typeID, ptype;

        dtemp = 0;      /* count printed dimensions */
        for (temp=p->stype; !ISBTYPE(temp) && dtemp < MaxArray; temp = DECREF(temp)) {
                /* put out a dimension for each instance of ARY in type word */
                if( ISARY(temp) ) {
                                aryDim[dtemp++] = temp->ary_size;
                        }
                }

        if (!(typeID = findAryID((ptype= pTypeID(p)),aryDim[--dtemp]))) {
                typeID = (AryIDtab-Nxt_AryID)+1;
#ifdef XCOFF
#ifndef CXREF
                printf( "\t.stabx\t\":t%d=ar0;0;%d;%d\",0,%d,0\n",
                        typeID, aryDim[dtemp]-1, ptype, C_DECL);
#endif
#endif
                addAryID(ptype,aryDim[dtemp]);
                }

        while (dtemp > 0) {
                if (!(typeID = findAryID((ptype = typeID),aryDim[--dtemp]))) {
                        typeID = (AryIDtab-Nxt_AryID)+1;
#ifdef XCOFF
#ifndef CXREF
                        printf( "\t.stabx\t\":t%d=ar0;0;%d;%d\",0,%d,0\n",
                                typeID, aryDim[dtemp]-1, ptype, C_DECL);
#endif
#endif
                        addAryID(ptype,aryDim[dtemp]);
                        }
        }

        if(isptr(p->stype))
                typeID = pptr(p,typeID);
        return (typeID);
}

/* -------------------- pptr ---------------------- */

#define PTRMASK 0x7FFFFFFF
#define NAMEPTR(x) ((x)|~PTRMASK)

pptr(p, typeID) struct symtab *p; int typeID;  {
        /* print debugging info for dimensions
         */

        TPTR temp;
        int dflag=0;
        int ptype;

        for (temp = p->stype; !ISBTYPE(temp) && dflag < 4; temp = DECREF(temp))
                /* put out a dimension for each instance of PTR in type word */

        if( ISPTR(temp) ) {
                dflag++;
                if(!(typeID))
                        typeID = pTypeID(p);
                ptype = typeID;
                if (!(typeID = findTypID(NAMEPTR(ptype)))) {
                        addTypID(NAMEPTR(ptype));
#ifdef XCOFF
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                        printf( "\t.stabx\t\":t%d=*%d\",0,%d,0\n",
                                typeID = (Nxt_TypID-TypIDtab)*2,ptype,C_DECL);
#endif
#endif
                        }
        }
        return(typeID);
}

/* ---------------------- pTypeID ---------------- */

pTypeID(p) register struct symtab *p;
{
        TPTR ty;
        TWORD bty;
        int typeID;

        ty = btype(p->stype);
        bty = TOPTYPE(ty);

        if (bty== STRTY || bty==UNIONTY || bty==ENUMTY ) {

                if (typeID = findTypID (ty->typ_size))
                        return (typeID);
        }
        else
                return(defntypes[bty]);
}

/* -------------------- isarray -------------------- */

/* Is it an array? */
isarray(ty) register TPTR ty;
{
        for (; !ISBTYPE(ty); ty = DECREF(ty))
                if( ISARY(ty) )
                        return 1;
        return 0;
}

/* -------------------- isptr ---------------------- */

/* Is it a pointer? */
isptr(ty) register TPTR ty;
{
        for (; !ISBTYPE(ty); ty = DECREF(ty))
                if( ISPTR(ty) )
                        return 1;
        return 0;
}

/* -------------------- findTypID ---------------- */

findTypID(p) register int p;  /* p - typeID (top bit indicates ptr/sue) */
{
        int index=1;
        int * Curr_TypID = TypIDtab;

        while (Curr_TypID!=Nxt_TypID) {
                if(*Curr_TypID++==p)
                                return (index*2);
                index++;
                }
        return (0);
}

/* -------------------- findAryID ---------------- */

findAryID(p,q) register int p; int q;  /* p - typeID */
                                       /* q - array dimension */
{
        int index=0;
        int * Curr_AryID = AryIDtab;

        while (Curr_AryID!=Nxt_AryID) {
                if(*Curr_AryID--==p)
                        if(*Curr_AryID==q)
                                return (index*2+1);
                Curr_AryID--; index++;
                }
        return (0);
}

/* -------------------- addTypID ---------------- */

addTypID(p) register int p;
{

        if (Nxt_TypID<Nxt_AryID)
                *Nxt_TypID++=p;
        else
                cerror(TOOLSTR(M_MSG_252, "TypID table overflow"));
}

/* -------------------- addAryID ---------------- */

addAryID(p,q) register int p; register int q;
{

        if (Nxt_AryID>Nxt_TypID){
                *Nxt_AryID--=p; *Nxt_AryID--=q;
        }
        else
                cerror(TOOLSTR(M_MSG_253, "AryID table overflow"));
}

#ifdef XCOFF
/* -------------------- addstabx --------------- */

addstabx() {
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf("?\"\n\t.stabx\t\"");
#endif
}
#endif

#define MAXLINE 60
#define MAXDESC 20
#define LONGLINE(x,y) ((x = (x+(y)+MAXDESC))>MAXLINE) /* fit on a line? */
/* -------------------- strend -------------------- */

strend( dimst ) int dimst; {
#ifdef XCOFF
        /* called at the end of a structure declaration
         * this routine puts out the structure tag, its members
         * and an eos.  dimst is the index in dimtab of the
         * structure description
         */
        int member, size, saveloc;
        int count=1;
        char savech;
        struct symtab *memptr, *tagnm, *strfind();

        if( !gdebug ) return;

        /* set locctr to text */
        saveloc = locctr( PROG );

        /* set up tagname */
        member = dimtab[dimst + 1];
        tagnm = strfind(dimst);

        if( tagnm == NULL ) {
                /* create a fake if there is no tagname */
                /* use the local symbol table */
                tagnm = &mytable;
                if( ifake == FAKENM )
                        cerror(TOOLSTR(M_MSG_254, "fakename table overflow" ));

                /* generate the fake name and enter into the fake table */
                mytable.psname = getmem( FAKESIZE+1 );
                sprintf( mytable.psname, ".%dfake", ifake );
                mystrtab[ifake++] = dimst;
                memptr = &stab[dimtab[member]];

                /* fix up the fake's class and type based on class of its members */
                switch( memptr->sclass ) {
                case MOS:
                        tagnm->sclass = STNAME;
                        tagnm->stype = tynalloc(STRTY);
                        break;
                case MOE:
                        tagnm->sclass = ENAME;
                        tagnm->stype = tynalloc(ENUMTY);
                        break;
                case MOU:
                        tagnm->sclass = UNAME;
                        tagnm->stype = tynalloc(UNIONTY);
                        break;
                default:
                        if( memptr->sclass & FIELD ){
                                tagnm->sclass = STNAME;
                                tagnm->stype = tynalloc(STRTY);
                                }
                        else
                                cerror(TOOLSTR(M_MSG_255, "can't identify type of fake tagname" ));
                        }
                tagnm->slevel = 0;;
                tagnm->stype->typ_size = dimst;
                addTypID(dimst);

                /* print .stabx definition */
                printf( "\t.stabx\t\":T%d=",
                        findTypID(btype(tagnm->stype)->typ_size));
                savech = *tagnm->psname;
                }
        else {
                /* print out the structure header */
                savech = *tagnm->psname;
                if( savech == '$' )
                        *tagnm->psname = '_';

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                /* print .stabx definition */
                printf( "\t.stabx\t\"%s:T%d=", tagnm->psname,
                        findTypID(btype(tagnm->stype)->typ_size));
#endif
        }

        size = (unsigned)dimtab[dimst] / SZCHAR;

        switch ( tagnm->sclass ) {
        case STNAME:    /*Structure*/
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "s%d",size);
#endif
                break;
        case ENAME:     /*Enumeration*/
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "e" );
#endif
                break;
        case UNAME:     /*Union*/
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "u%d",size);
#endif
                break;
        default:
                cerror(TOOLSTR(M_MSG_256, "can't identify type of tagname"));
                }
        *tagnm->psname = savech;

        /* print out members */
        while( dimtab[member] >= 0 ) {
                memptr = &stab[dimtab[member++]];
                if (LONGLINE(count,strlen(memptr->psname))) {
                        addstabx();
                        count = 0;
                        }
                savech = *memptr->psname;
                if( savech == '$' )
                        *memptr->psname = '_';
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                if( tagnm->sclass == ENAME)
                        printf( "%s:%d,", memptr->psname, memptr->offset);
                else {
                        size = tsize( memptr->stype );
                        printf( "%s:%d,%d,%d;", memptr->psname,
                        getTypeID(memptr), memptr->offset, size );
                        }
#endif
                *memptr->psname = savech;
                }

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        if( tagnm->sclass == ENAME)
            printf(";\",%d,%d,%d\n", 0, C_DECL, tyencode(tagnm->stype));
        else
            printf("\",%d,%d,%d\n", 0, C_DECL, tyencode(tagnm->stype));
#endif

        /* return to old locctr */
        locctr( saveloc );
#endif
        }

/* -------------------- strfind -------------------- */

struct symtab *
strfind( key ) int key; {
        /* find the structure tag in the symbol table, 0 == not found
         */
        struct symtab *sptr;
        char spc;
        for( sptr = stab; sptr < stab + nstabents; ++sptr ) {
                spc = sptr->sclass;
                if( (spc == STNAME || spc == ENAME || spc == UNAME ) &&
                                sptr->stype->typ_size == key &&
                                TOPTYPE(sptr->stype) != TNULL )
                        return( sptr );
                }
        /* not found */
        return( NULL );
        }

/* -------------------- strname -------------------- */

char *
strname( key ) int key; {
        /* return a pointer to the tagname,
         * the fake table is used if not found by strfind
         */
        int i;
        struct symtab *tagnm, *strfind();
        tagnm = strfind( key );
        if( tagnm != NULL )
                return( tagnm->psname );

        for( i = 0; i < FAKENM; ++i )
                if( mystrtab[i] == key ) {
                        sprintf( tagname, ".%dfake", i );
                        return( tagname );
                        }

        printf(TOOLSTR(M_MSG_287, "structure tagname not found\n" ));
/*
        cerror(TOOLSTR(M_MSG_287, "structure tagname not found" ));
*/
        return(NULL);
        }

#ifndef ONEPASS

/* -------------------- tlen -------------------- */

tlen(p) NODE *p;
{
        switch (TOPTYPE(p->in.type)) {
                case SCHAR:
                case CHAR:
                case UCHAR:
                        return 1;
                case SHORT:
                case USHORT:
                        return 2;
                default:
                        return 4;
                case LNGLNG:
                case ULNGLNG:
                case DOUBLE:
                        return 8;
                case LDOUBLE:
                        return 16;
                }
        }

/* -------------------- fltprint -------------------- */

fltprint(p)
        register NODE *p;
{
        printf( "%o\t%o\t", p->fpn.dval ); /* YECH */
}

#endif

/* -------------------- eline -------------------- */

eline()
{
        /* generate a new line number breakpoint if
         * the line number has changed.
         */
        if( gdebug && lineno != oldln ) {
                oldln = lineno;
                if( lastloc == PROG && strcmp( startfn, ftitle ) == 0 )
                        printf( "\t.line\t%d\n", Lineno );
                }
}

