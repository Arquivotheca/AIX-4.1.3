 static char     sccsid[] = "@(#)54      1.15.2.3  src/bos/usr/ccs/bin/common/pftn.c, cmdprog, bos411, 9428A410j 4/25/94 15:34:02";
/*
 * COMPONENT_NAME: (CMDPROG) pftn.c
 *
 * FUNCTIONS: AttachProto, CheckEnum, CheckQualifier, CheckStruct, CheckType 
 *            CheckTypedef, CountMembers, CreateProto, DiagnoseType           
 *            FakeNamealloc, InitParse, OutArgType, OutArguments, OutFileBeg  
 *            OutFileEnd, OutFtnAddrRef, OutFtnDef, OutFtnRef, OutFtnUsage      
 *            OutMembers, OutSymbol, OutType, ResultType, SeenType         
 *            StabInfoPrint, TagStruct, WriteType, beginit, bstruct, checkst    
 *            chkty, clearst, dclargs, dclstruct, defid, deftents, doinit       
 *            dumpstack, endinit, extrndec, falloc, fixclass, fixlab          
 *            fixtype, ftnarg, ftnend, getFakeName, getstr, gotscal, hide     
 *            ilbrace, inforce, instk, irbrace, iscall, lookup, makeghost     
 *            markaddr, mknonuniq, moedef, nidcl, oalloc, protopop            
 *            protopush, psave, putbyte, relook, rstruct, savestr, strip      
 *            talign, tsize, tymerge, tyreduce, uclass, unbuffer_str, unhide  
 *            upoff, vfdalign, yyaccpt, yyerror                               
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
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Modified: May 91 by  RWaters ILS changes.
 * Modified: May 91 by  RWaters Special case of fix to A16691
 * Modified: June 91 by RWaters P19112, incorrectly tested for ISAGGREGATE,
                        ENUMTYs should have nothing to with this.
 * Modified: June 91 by RWaters P19915 LINTLIBRARIES & prototypes
 */

/* AIWS C compiler */
#include "mfile1.h"
#include "messages.h"

#include <stdlib.h>
#include <mbstr.h>

extern int      minsvarg;  /* minimum offset of an arg to save */
extern int      adebug;
extern int      bdebug;
extern int      aflag;

#define INIT_STACK_SIZE 32

struct instk {
        int     in_sz;   /* size of array element */
        int     in_x;    /* current index for structure member in structure initializations */
        int     in_n;    /* number of initializations seen */
        TPTR in_t;    /* type */
        int     in_id;   /* stab index */
        int     in_fl;   /* flag which says if this level is controlled by {} */
        OFFSZ in_off;  /* offset of the beginning of this level */
} instack[INIT_STACK_SIZE],
*pstk, *tempstack;

/* defines used for getting things off of the initialization stack */

struct symtab *relook();

/* a flag to inhibit multiple warnings of partial elided initialization.
   perhaps a global mechanism that works for all errors is better */
static int      partelided = 0;
#ifndef ddebug
int     ddebug = 0;
#endif
static int      LocalUniqid = REGSZ + 1;
static int      GlobalUniqid = 1;
static int      RecurseCnt = 0;

struct symtab *mknonuniq();

int     paramsz = 150; /* variable for increasing size of paramstk */
int     protosz = 20;  /* variable for increasing size of protostk */


/* -------------------- dumpstack -------------------- */

/*
 * this macro is used to debug the initialization stack
 */
#define dumpstack(which) {\
        struct instk *temp = instack; \
        int i =0; \
        printf("-->%s\n", which);\
        printf("  @ n\t\t sz   x   n t\t\tid   fl  off\n"); \
        for (; temp <= pstk; i++, temp++) { \
                printf("%3d %p\t%3d %3d %3d %p\t%3d %3d %3d\t%s\n", i, temp, \
                        temp->in_sz, temp->in_x, temp->in_n, \
                        temp->in_t, temp->in_id, \
                        temp->in_fl, temp->in_off, stab[temp->in_id].psname);\
        }\
}


/* -------------------- psave -------------------- */

static void psave (int i) {
        if ( paramno >= paramsz ) { /* then make paramstk big enough */
                paramsz = paramno + 150;
                paramstk = (int *)realloc(paramstk, paramsz * sizeof(int));
                if ( paramstk == NULL )
                        cerror(TOOLSTR(M_MSG_219, "parameter stack overflow"));
        }
        paramstk[ paramno++ ] = i;
}


/*  match  ----- Update the symbol type to the composite type */

static void match (NODE *q, int class, struct symtab *p, int idp, TPTR t) {
    if (mkcomposite(p->stype, t, p->slevel))
        /* incomplete type for %s has already been completed */
        WARNING( WDECLAR, MESSAGE(145), p->psname );
    if (blevel <= p->slevel)
        return;
    /* Make new entry to detect redeclaration in same scope */
    q->tn.rval = hide (p, 1);
    p = &stab[q->tn.rval];
    *p = stab[idp];
    p->slevel = blevel;
    p->sflags = (p->sflags & (SNSPACE | SEXTRN)) | SHIDES;
}


/*  enter  -----  Make a new entry  -----   */

static void enter (NODE *q, int class, register struct symtab *p, int idp, TPTR type) {
#ifndef BUG1
        if ( ddebug )
                printf( "\tnew entry made\n" );
#endif  !BUG1

        if ( TOPTYPE(type) == TVOID ) {
                if (class == PARAMFAKE)
                        paramFlg |= SAW_VOID;
                else if (class != TYPEDEF) {
                        /* "void type for %s" */
                        UERROR( ALWAYS, MESSAGE(117), p->psname );
                        type = tyalloc(INT);
                }
        }
        if ( class == STNAME || class == UNAME || class == ENAME ) {
                type = tynalloc(TOPTYPE(type)); /* Make a new node */
                type->typ_size = curdim;
                dstash( 0 );            /* size */
                dstash( -1 );           /* index to members */
                dstash( ALSTRUCT );     /* alignment */
                dstash( idp );          /* tag symbol */
        }
        p->stype = type;
        p->sclass = class;
        p->slevel = blevel;
        p->offset = NOOFFSET;
        p->suse = lineno;

/* symbol definition line number, p->suse gets clobbered by each reference */
#if     defined (LINT) || defined (CFLOW)
        p->line = lineno;
        p->ifname = ifname;
#endif  LINT || CFLOW

        /* allocate offsets */
        if ( class & FIELD ) {
                falloc( p, class & FLDSIZ, 0, NIL );  /* new entry */
                if ( instruct & INUNION )
                        strucoff = 0;
                psave( idp );
        } else
            switch ( class ) {
                case AUTO:
                case AUTOREG:
                    /* defer automatic arrays of unknown size till later.
                       N.B.: see below for the same test. */
                    if (!(ISARY(q->in.type) && (q->in.type->ary_size == 0) && ISHAVEINIT))
                        oalloc( p, &autooff );
#ifdef  ONEPASS
                    break;
#endif  ONEPASS
                case PARAM:
                case PARAMREG:
                case PARAMFAKE:
                    p->uniqid = LocalUniqid++;
#ifndef XCOFF
 #ifndef ONEPASS
                    if ((class == AUTO || class == AUTOREG) &&
			(!ISARY(q->in.type) || q->in.type->ary_size || !ISHAVEINIT))
                        StabInfoPrint(p);
# endif !ONEPASS
#endif  !XCOFF
                    if (CanBeLNAME(p->stype))
                        p->sflags |= (class == AUTO || class == AUTOREG) ? SLNAME : SPNAME;
                    break;

                case REGISTER:
                    p->offset = ISFLOAT(type) ? fpregvar-- : regvar--;
                    p->uniqid = LocalUniqid++;
                    if ( blevel == 1 || paramlevel > 0 )
                        p->sflags |= SSET;
#ifndef XCOFF
 #ifndef ONEPASS
                    if ( blevel != 1 && paramlevel == 0)
                        StabInfoPrint(p);
 #endif !ONEPASS
#endif  !XCOFF
                    break;

                case EXTERN:
                case EXTENT:
                case EXTDEF:
                case USTATIC:
                case UFORTRAN:
                case FORTRAN:
                    if ( blevel > 0 && ISFTN(type) && !devdebug[SCOPING] ) {
                        p->slevel = 0;
                        dimptr->cextern = 1;
                        p->stype = copytype(p->stype, 0);
                        p->sflags |= SEXTRN;

                        /* Detect redeclaration in same scope */
                        q->tn.rval = hide( p, 1 );
                        p = &stab[q->tn.rval];
                        *p = stab[idp];
                        idp = q->tn.rval;
                        p->slevel = blevel;
                        p->sflags = (p->sflags & (SNSPACE | SEXTRN)) | SHIDES;
                    }
                    p->sflags |= SEXTRN;
                case STATIC:
                    p->uniqid = GlobalUniqid++;
                    p->offset = getlab();
#ifndef XCOFF
 #ifndef ONEPASS
                    StabInfoPrint(p);
 #endif !ONEPASS
#endif  !XCOFF
                    if (!blevel || (ISFTN(type) && !devdebug[SCOPING])) {
                        register struct symtab *r;

                        p->sflags |= SEXTRN;
                        r = &stab[idp = lookup (p->psname,(p->sflags & SNSPACE) | SSCOPED)];
                        if (TOPTYPE(r->stype) == UNDEF)
                             r->stype = tyalloc(TNULL);
                        else if (ISFTN(p->stype) == ISFTN(r->stype)) {
                            if (!comtypes(p->stype, r->stype, 0))
                                 /* "external symbol type clash for %s" */
                                 WARNING( WDECLAR, MESSAGE(193), p->psname );
                            p->suse = -lineno;
#ifdef  XCOFF
                            p->sflags |= r->sflags & (SSET | SREF | SFCALLED | SFADDR);
#else
                            p->sflags |= r->sflags & (SSET | SREF);
#endif  XCOFF
/*
 At this point, we have a redundant symtab entry for the scoped-out extern.
 Unfortunately, we can't remove it now or at any other convenient time, since
 its removal will cause the actual symbol table entry to move into the newly
 vacated hole, which breaks function definitions quite badly. Therefore, use
 the old entry position for the new symbol and mark the former position for
 removal at the end of the next function
 (not now, since parameters would get screwed up).
*/
                            *r = *p;
                            p->slevel = 1;
                            p->sflags |= SSCOPED;
                            p = r;
                            q->tn.rval = idp;
                            if ( curftn >= 0 )
                                curftn = idp;
                        } else {
                            /* "external symbol type clash for %s" */
                            UERROR( ALWAYS, MESSAGE(193), p->psname );
                        }
                        idp = q->tn.rval;
                    }
                    break;

                case ULABEL:
                case LABEL:
                    p->offset = getlab();
                    p->slevel = 2;
                    if ( class == LABEL ) {
                        locctr( PROG );
                        deflab( p->offset );
                    }
                    break;

                case MOS:
                case MOU:
                    oalloc( p, &strucoff );
                    if ( class == MOU )
                        strucoff = 0;
                    psave( idp );
                    break;

                case MOE:
                    p->offset = strucoff++;
                    psave( idp );
                    break;
            }

        /* user-supplied routine to fix up new definitions */
        FIXDEF(p);

#ifndef BUG1
        if (ddebug)
            printf( "\toffset: %d\n", p->offset );
#endif  !BUG1

}

/*  mismatch  ------  Resolve the redeclaration */

static void mismatch (NODE *q, int class, struct symtab *p, int idp, TPTR type,
		      int slev) {
    /* Allow nonunique structure/union member names. */
    if (class == MOS || class == MOU || (class & FIELD)) {
        /* Make a new entry */
        int     *memp;
        p->sflags |= SNONUNIQ;  /* old entry is nonunique */
        /* Determine if name has occurred in this structure/union */
        if (!paramno)
            cerror(TOOLSTR(M_MSG_218, "paramstk error" ));
        for ( memp = &paramstk[paramno-1]; *memp >= 0; --memp ) {
             register struct symtab *sym = &stab[*memp];

             if (sym->sclass == STNAME || sym->sclass == UNAME)
                 break;
             if ((sym->sflags & SNONUNIQ) && !strcmp(p->psname, sym->psname)) {
                 /* "illegal redeclaration of %s" */
                 UERROR( ALWAYS, MESSAGE(160), p->psname );
                 break;
                }
        }
        /* Update p and idp to new entry */
        p = mknonuniq( &idp );
        enter (q, class, p, idp, type);
    }
    /* Allow hiding of declarations. */
    else if (blevel > slev && class != EXTERN && class != USTATIC && 
             class != UFORTRAN && class != ULABEL && class != LABEL) {
        if ( slev == 1 && blevel == 2 )
            /* "redeclaration of parameter %s inside function" */
            WERROR( devdebug[SCOPING], MESSAGE(185), p->psname );
        q->tn.rval = idp = hide( p, 0 );
        p = &stab[idp];
        enter (q, class, p, idp, type);
    }
    /* global declarations inside blocks use the appropriate global symbol. */
    else if (blevel <= slev || (p->sflags & SEXTRN) ||
             (class != EXTERN && class != USTATIC && class != UFORTRAN)) {
	/* "illegal redeclaration of %s" */
        UERROR(ALWAYS, MESSAGE(160), p->psname);
    }
    /* Find the hidden declaration at the highest level */
    else if ((idp = extrndec(p)) >= 0) {
        /* Make a new entry for the external symbol */
        if ( comtypes(q->in.type, stab[idp].stype, 0) ) {
            if ( slev == 1 && blevel == 2 )
                /* "redeclaration of parameter %s inside function" */
                WERROR( devdebug[SCOPING], MESSAGE(185), p->psname );
            match (q, class, p, idp, type);
        }
        else {
	    /* "illegal redeclaration of %s" */
            UERROR( ALWAYS, MESSAGE(160), p->psname );
        }
    }
    /* Didn't find existing one.  Have to create it. */
    else if (!ISFTN(type) || devdebug[SCOPING]) {
        q->tn.rval = idp = hide( p, 0 );
        p = &stab[idp];
        if (slev == 1 && blevel == 2)
            /* "redeclaration of parameter %s inside function" */
            WERROR( devdebug[SCOPING], MESSAGE(185), p->psname );
        enter (q, class, p, idp, type);
    }
}


/* -------------------- fixtype -------------------- */

static TPTR fixtype (register TPTR type, int class ) {

        if (TOPTYPE(type) == UNDEF)
                return type;

        /* detect function arguments, watching out for structure declarations */
        /* for example, beware of f(x) struct { int a[10]; } *x; { ... } */
        /* the danger is that "a" will be converted to a pointer */

        if ( class == SNULL && ( blevel == 1 || paramlevel > 0 ) && 
            !( instruct & (INSTRUCT | INUNION) ) ) {
                class = PARAM;
        }
        if ( class == PARAM || class == PARAMREG || class == PARAMFAKE || 
            ( class == REGISTER && ( blevel == 1 || paramlevel > 0 ) ) ) {
                if ( ISARY(type) ) {
                        type = INCREF(DECREF(type), PTR);
                } else if ( ISFTN(type) ) {
                        type = INCREF(type, PTR);
                }
        }
        if ( instruct && ISFTN(type) ) {
                /* "function illegal in structure or union"  */
                UERROR( ALWAYS, MESSAGE(46) );
                type = INCREF(type, PTR);
        }
        return type;
}

int     NoRegisters = 0;  /* set if register declarations should be ignored */

#define ILLEGALCLASS  catgets(catd, MS_CTOOLS, M_MSG_302, "illegal class: %s")


/* -------------------- fixclass -------------------- */

static int fixclass (int class, TPTR type) {
        /*
        ** If class is register and we're optimizing then
        ** ignore the register classification.
        */
        if ( class == REGISTER && adebug )
                class = SNULL;

        /*
        ** Fix null class.
        */
        if ( class == SNULL ) {
                if ( instruct & INSTRUCT )
                        class = MOS;
                else if ( instruct & INUNION )
                        class = MOU;
                else if ( blevel == 0 )
                        class = EXTDEF;
                else if ( blevel == 1 || paramlevel > 0 )
                        class = PARAM;
                else if ( ISFTN(type) )
                        class = EXTERN;
                else
                        class = AUTO;
        }

        /*
        ** Check function classes.
        */
        if ( ISFTN(type) ) {
                switch ( class ) {
                default:
                        /* "function has illegal storage class" */
                        WERROR( ALWAYS, MESSAGE(45) );
                        class = EXTERN;
                case EXTDEF:
                case EXTERN:
                case FORTRAN:
                case UFORTRAN:
                case STATIC:
                case TYPEDEF:
                        break;
                case USTATIC:
                        if ( blevel > 0 )
                                /* "function has illegal storage class" */
                                WERROR( ALWAYS, MESSAGE(45) );
                        break;
                }
        }

        /*
        ** Check fields.
        */
        if ( class & FIELD ) {
                if ( !( instruct & ( INSTRUCT | INUNION ) ) )
                        /* "illegal use of field" */
                        UERROR( ALWAYS, MESSAGE(72) );
                return( class );
        }

        /*
        ** Check general classes.
        */
        switch ( class ) {

        case AUTO:
                if ( blevel < 2 || paramlevel > 0 ) {
                        /* "illegal class" */
                        WERROR( ALWAYS, MESSAGE(52) );
                        if ( blevel == 0 )
                                class = EXTDEF;
                        else
                                class = PARAM;
                }
                break;

        case EXTERN:
        case STATIC:
                if ( blevel == 1 || paramlevel > 0 ) {
                        /* "illegal class" */
                        WERROR( ALWAYS, MESSAGE(52) );
                        class = PARAM;
                }
                break;

        case TYPEDEF:
                if ( blevel == 1 || paramlevel > 0 )
                        /* "illegal typedef declaration" */
                        WERROR( ALWAYS, MESSAGE(154) );
                break;

        case REGISTER:
                if ( blevel == 0 ) {
                        /* "illegal register declaration" */
                        WERROR( ALWAYS, MESSAGE(68) );
                        class = EXTDEF;
                        break;
                }
                if ( !NoRegisters ) {
                        if ( cisreg(type) && regvar >= MINRVAR + aflag )
                                break;
                        if ( ( TOPTYPE(type) == DOUBLE || 
                            TOPTYPE(type) == LDOUBLE ) && 
                            fpregvar >= MINFPVAR )
                                break;
                }
                class = ( blevel == 1 || paramlevel > 0 ) ? PARAMREG : AUTOREG;
                break;

        case FORTRAN:
        case UFORTRAN:
#ifdef  NOFORTRAN
                /* a condition which can regulate the FORTRAN usage */
                NOFORTRAN;
#endif  NOFORTRAN
                if ( !ISFTN(type) ) {
                        /* "fortran declaration must apply to function" */
                        UERROR( ALWAYS, MESSAGE(40) );
                        class = EXTERN;
                } else if ( ISPTR(DECREF(type)) ) {
                        /* "fortran function has wrong type" */
                        UERROR( ALWAYS, MESSAGE(41) );
                }
                break;

        case STNAME:
        case UNAME:
        case ENAME:
                break;

        case EXTDEF:
        case EXTENT:
                if ( blevel != 0 )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case USTATIC:
                if ( blevel == 1 || paramlevel > 0 )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case PARAM:
        case PARAMFAKE:
                if ( blevel != 1 && paramlevel == 0 )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case LABEL:
        case ULABEL:
                if ( blevel < 2 || paramlevel > 0 )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case MOS:
                if ( !( instruct & INSTRUCT ) )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case MOU:
                if ( !( instruct & INUNION ) )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        case MOE:
                if ( instruct & ( INSTRUCT | INUNION ) )
                        cerror( ILLEGALCLASS, scnames( class ) );
                break;

        default:
                cerror(TOOLSTR(M_MSG_239, "illegal class: %d"), class );
                /*NOTREACHED*/
        }

        return class;
}

/* -------------------- defid -------------------- */

defid( q, class )
NODE *q;
int     class;
{
        register struct symtab *p;
        int     idp;
        TPTR type;
        TPTR stp;
        int     scl;
        int     slev;

        if (!q)
                return;  /* an error was detected */
        if ( q < node || q >= &node[ntrnodes] )
                cerror(TOOLSTR(M_MSG_216, "defid call" ));
        if ((idp = q->tn.rval) < 0)
                cerror(TOOLSTR(M_MSG_217, "tyreduce" ));
        p = &stab[idp];

#ifndef BUG1
        if ( ddebug ) {
                printf( "defid( %s (%d), ", p->psname, idp );
                tprint( q->in.type );
                printf( ", %s ), level %d\n", scnames(class), blevel );
        }
#endif  !BUG1

        /* fix up the class and types, and check for legality */
        q->in.type = type = fixtype (q->in.type, class);
        class = fixclass (class, type);
        stp = p->stype;
        slev = p->slevel;

#ifndef BUG1
        if (ddebug) {
                printf( "\tmodified to " );
                tprint( type );
                printf( ", %s\n", scnames(class) );
                printf( "\tprevious def'n: " );
                tprint( stp );
                printf( ", %s ), level %d\n", scnames(p->sclass), slev );
        }
#endif  !BUG1

        if (ISFTN(type) &&
	    (class == EXTDEF || class == STATIC || class == FORTRAN)) {
                curftn = idp;
                funcConflict = 0;
        }

        if ( blevel == 1 && TOPTYPE(stp) != FARG ) {
                switch ( class ) {
                case PARAM:
                case PARAMREG:
                case PARAMFAKE:
                case REGISTER:
                        /* "declared argument %s is missing" */
                        UERROR( ALWAYS, MESSAGE(28), p->psname );
                        break;
                }
        }
        if (TOPTYPE(stp) == UNDEF || TOPTYPE(stp) == FARG ||
            (ISFTN(stp) && TOPTYPE(DECREF(stp)) == UNDEF && p->sclass == SNULL)) { /* name encountered as function, not yet defined */
            enter (q, class, p, idp, type);
	    return;
        }

        if (!comtypes(type, stp, 0)) {
            mismatch (q, class, p, idp, type, slev);
            return;
        }

        /*
        ** Detect old-style function definitions in the presence of
        ** a new-style prototype declaration.
        */
        if (ISFTN(stp) && stp->ftn_parm != PNIL && type->ftn_parm == PNIL && 
            (class == EXTDEF || class == STATIC || class == FORTRAN)) {
           /* "using old-style argument definition in presence of prototype" */
            WARNING( WPROTO, MESSAGE(153) );
            funcConflict = 1;  /* signal dclargs that this happened */
        }

        scl = p->sclass;

#ifndef BUG1
        if (ddebug)
                printf( "\tprevious class: %s\n", scnames(scl) );
#endif  !BUG1

        if (class & FIELD) {
                /* redefinition */
                if ( !falloc( p, class & FLDSIZ, 1, NIL ) ) {
                        /* successful allocation */
                        if ( instruct & INUNION )
                                strucoff = 0;
                        psave( idp );
                        return;
                }
                /* blew it: resume at end of switch... */
        } else
                switch (class) {
                case EXTERN:
                        switch ( scl ) {
                        case STATIC:
                                if ( slev != 0 )
                                        break;
                        case USTATIC:
                        case EXTDEF:
                        case EXTENT:
                        case EXTERN:
                        case FORTRAN:
                        case UFORTRAN:
                                match (q, class, p, idp, type);
				return;
                        }
                        break;
                case EXTENT:
                case EXTDEF:
                        switch ( scl ) {
                        case EXTDEF:
                                if ( class == EXTDEF )
                                        break;
                                class = EXTDEF;
                        case EXTENT:
                                /* "redeclaration of %s" */
                                WARNING( WDECLAR, MESSAGE(96), p->psname );
                        case EXTERN:
                                p->sclass = class;
                                match (q, class, p, idp, type);
				return;
                        case USTATIC:
                                if ( ISFTN(type) ) {
                                        p->sclass = STATIC;
                                        match (q, class, p, idp, type);
					return;
                                }
                                break;
                        }
                        break;

                case USTATIC:
                        switch (scl) {
                            case USTATIC:
                                if (ISFTN(type)) {
                                    match (q, class, p, idp, type);
	                            return;
				    }
                            case STATIC:    /* "redeclaration of %s" */
                                WARNING(WDECLAR, MESSAGE(96), p->psname);
                                match (q, class, p, idp, type);
                                return;
                            case EXTERN:
                                if (!ISFTN(type) && p->suse < 0 )
                                    /* "%s declared both static and extern" */
                                    UERROR(ALWAYS, MESSAGE(164), p->psname);
                                else
                                    /* "%s declared both static and extern" */
                                    WERROR(devdebug[COMPATIBLE], MESSAGE(164), p->psname);
                                p->sclass = USTATIC;
                                match (q, class, p, idp, type);
                                return;
                        }
			break;

                case STATIC:
                        if ( blevel != 0 )
                                break;
                        if ( scl == USTATIC ) {
                                if ( !ISFTN(type) ) {
                                        /* "redeclaration of %s" */
                                        WARNING( WDECLAR, MESSAGE(96), p->psname );
                                }
                                p->sclass = STATIC;
                                match (q, class, p, idp, type);
				return;
                        } else if ( scl == EXTERN ) {
                                if ( !ISFTN(type) && p->suse < 0 )
                                        /* "%s declared both static and extern" */
                                        UERROR( ALWAYS, MESSAGE(164), p->psname );
                                else
                                        /* "%s declared both static and extern" */
                                        WERROR( devdebug[COMPATIBLE], MESSAGE(164), p->psname );
                                p->sclass = STATIC;
                                match (q, class, p, idp, type);
				return;
                        }
                        break;

                case UFORTRAN:
                        if ( scl == UFORTRAN || scl == FORTRAN ) {
                                match (q, class, p, idp, type);
				return;
                        }
                        break;

                case FORTRAN:
                        if ( scl == UFORTRAN ) {
                                p->sclass = FORTRAN;
                                match (q, class, p, idp, type);
				return;
                        }
                        break;

                case ULABEL:
                        if ( scl == LABEL || scl == ULABEL )
                                return;
                        break;

                case LABEL:
                        if ( scl == ULABEL ) {
                                p->sclass = LABEL;
                                deflab (p->offset);
                                return;
                        }
                        break;

                case STNAME:
                case UNAME:
                case ENAME:
                    if (scl != class || slev != blevel || dimtab[type->typ_size])
                            break;  /* mismatch.. */
                    /* previous entry just a mention */
                    match (q, class, p, idp, type);
		    return;
                }
    mismatch (q, class, p, idp, type, slev);
}

#ifndef ONEPASS
# ifndef        XCOFF


/* -------------------- StabInfoPrint -------------------- */

StabInfoPrint(p)
register struct symtab *p;
{
#  ifdef        IS_COMPILER
        int     id;             /* unique identifier to pass to the optimizer */
        TPTR type;              /* type to pass to the optimizer        */
        int     op;             /* op that will go on tree nodes        */
        NODE * t;               /* tree node for ENUM kludge */

        type = p->stype;
        id   = p->uniqid;

        switch ( p->sclass ) {
        case PARAM:
        case PARAMREG:
                op = PNAME;
                break;

        case AUTO:
        case AUTOREG:
                op = LNAME;
                break;

        case REGISTER:
                /* Register variables are known by unique id
                 * cookies, all others are known by register
                 * number
                 */
                if ( !IsRegVar(p->offset) )
                        id = p->offset;
                op = REG;
                break;

        case STATIC:
                if ( p->slevel > 1 )
                        id = -p->offset;
                op = NAME;
                break;

        default:
                op = NAME;
                break;
        }

        if ( BTYPE(type) == ENUMTY ) {
                /* Pass 1 always lies about ENUMs, even on the lhs of
                 * assignments.  We must do likewise.
                 */

                t = talloc();
                t->in.op = op;
                t->in.type = type;
                econvert(t);
                t->in.op = FREE;
                type = t->in.type;
        }

        printf ("%c%d\t%d\t%d\t_%s\n", STABINFO, id, tyencode(type), op, p->psname);
#  endif        IS_COMPILER
}


# endif !XCOFF
#endif  !ONEPASS


/* -------------------- ftnend -------------------- */

ftnend()/* end of function */
{
        if ( retlab != NOLAB ) { /* inside a real function */
                efcode();
                LocalUniqid = REGSZ + 1;
        }
        checkst(0);
        retstat = 0;
        tcheck();
        brklab = contlab = retlab = NOLAB;
        flostat = 0;
        if ( nerrors == 0 ) {
                if ( psavbc != &asavbc[0] )
                        cerror(TOOLSTR(M_MSG_220, "bcsave error"));
                if ( paramno != 0 )
                        cerror(TOOLSTR(M_MSG_221, "parameter reset error"));
                if ( swx != 0 )
                        cerror(TOOLSTR(M_MSG_222, "switch error"));
        }
        psavbc = &asavbc[0];
        paramno = 0;
        autooff = AUTOINIT;
        regvar = MAXRVAR;
        fpregvar = MAXFPREG;
        reached = 1;
        swx = 0;
        swp = swtab;
        curftn = -1;
        locctr(DATA);
}


/* -------------------- dclargs -------------------- */

dclargs()
{
        register i, j;
        register struct symtab *p;
        register NODE *q;
        register TPTR t;
        register PPTR parm;

#ifndef BUG1
        if ( ddebug > 2)
                printf("dclargs()\n");
#endif  !BUG1

        argoff = ARGINIT;

        if (funcConflict) {
                parm = stab[curftn].stype->ftn_parm;
                if ( TOPTYPE(parm->type) == TVOID ) {
                        parm = parm->next;
                }
        }
        for ( i = 0; i < paramno; ++i ) {
                if ( (j = paramstk[i]) < 0 )
                        continue;
                p = &stab[j];
                if (TOPTYPE(p->stype) == TVOID) {
                        /* parameter list has a void, act
                         * as if there were no parameters seen.
                         */
                        paramno = 0;
                        break;
                }
#ifdef  CXREF
                CXDefName(j, p->suse);  /* params of functions */
#endif  CXREF

#ifndef BUG1
                if ( ddebug > 2 ) {
                        printf("\t%s (%d) ", p->psname, j);
                        tprint(p->stype);
                        printf("\n");
                }
#endif  !BUG1

                if ( p->sflags & SHIDES ) {
                        /* "%s redefinition hides earlier one" */
                        WARNING( (WDECLAR || WHEURISTIC) && WKNR, MESSAGE(2), p->psname );
                }
                if (p->sclass == PARAMFAKE) {
                        /* "no name for definition parameter" */
                        UERROR( ALWAYS, MESSAGE(155) );
                }
                if (TOPTYPE(p->stype) == FARG) {
                        q = block(FREE, NIL, NIL, tyalloc(INT));
                        q->tn.rval = j;
                        defid( q, PARAM );
                }
                FIXARG(p); /* local arg hook, eg. for sym. debugger */
                if (funcConflict) {
                        if ( parm == PNIL ) {
                                /* "wrong number of arguments
                                 *  in function definition"
                                 */
#ifdef  COMPAT
                                if ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
                                        WERROR( ALWAYS, MESSAGE(165) );
                                else
                                        UERROR( ALWAYS, MESSAGE(165) );
#else
                                UERROR( ALWAYS, MESSAGE(165) );
#endif  COMPAT
                                /* prevent multiple error messages */
                                funcConflict = 0;
                        } else {
                                if (ISINTEGRAL(p->stype)) {
                                        t = tyalloc( prmtint(block(FREE, NIL, NIL, p->stype)));
                                } else if (TOPTYPE(p->stype) == FLOAT) {
                                        t = tyalloc(DOUBLE);
                                } else {
                                        t = unqualtype( p->stype );
                                }
                                if ( !comtypes( parm->type, t, 0 ) ) {
                                        /* "prototype type mismatch of formal parameter %s" */
#ifdef  COMPAT
                                        /************************************************************************
                                        ** NOTE:  This must not be used if the compiler is modified to pass     *
                                        ** <4 byte parameters, or else bad code will result!                    *
                                        ************************************************************************/
                                        if ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
                                                WERROR( ALWAYS, MESSAGE(187), p->psname );
                                        else
                                                UERROR( ALWAYS, MESSAGE(187), p->psname );
#else
                                        UERROR( ALWAYS, MESSAGE(187), p->psname );
#endif  COMPAT
                                }
                                parm = parm->next;
                        }

                }
                if ( TOPTYPE(p->stype) != TELLIPSIS) {
                        /* always set aside space,
                         * even for register arguments
                         */
                        oalloc( p, &argoff );
                } else {
                        /* this parameter is just an ellipsis
                         * marker, it always appears at the
                         * top of the parameter stack so decrement
                         * paramno to avoid seeing it in bfcode.
                         */
                        paramno--;
                }

        }

        if (funcConflict && parm != PNIL) {
                /* "wrong number of arguments in function definition" */
#ifdef  COMPAT
                if ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] )
                        WERROR( ALWAYS, MESSAGE(165) );
                else
                        UERROR( ALWAYS, MESSAGE(165) );
#else
                UERROR( ALWAYS, MESSAGE(165) );
#endif  COMPAT
        }
        cendarg();
        locctr(PROG);
        defalign(ALINT);
        ++ftnno;
        bfcode( paramstk, paramno );
        paramno = 0;
}


/* -------------------- rstruct -------------------- */

NODE *
rstruct( idn, soru )  /* reference to a structure or union, with no definition */
int     idn;
int     soru;
{
        register struct symtab *p;
        register NODE *q;

        p = &stab[idn];
        switch (TOPTYPE(p->stype)) {
        case UNDEF:
def:
                q = block(FREE, NIL, NIL, tyalloc(UNDEF));
                q->tn.rval = idn;
                if ( soru & INSTRUCT ) {
                        q->in.type = tyalloc(STRTY);
                        defid( q, STNAME );
                } else if ( soru & INUNION ) {
                        q->in.type = tyalloc(UNIONTY);
                        defid( q, UNAME );
                } else {
                        /* "unknown enumeration" */
                        WERROR( devdebug[COMPATIBLE], MESSAGE(167) );
                        q->in.type = tyalloc(ENUMTY);
                        defid( q, ENAME );
                }
                break;

        case STRTY:
                if ( soru & INSTRUCT )
                        break;
                goto def;

        case UNIONTY:
                if ( soru & INUNION )
                        break;
                goto def;

        case ENUMTY:
                if ( !(soru & (INUNION | INSTRUCT)) )
                        break;
                goto def;

        }
        stwart = instruct;
        q = mkty( p->stype );
        q->tn.rval = idn;       /* added for needs of TagStruct() */
        p->suse = -lineno;
        return( q );
}


/* -------------------- moedef -------------------- */

moedef( idn )
int     idn;
{
        register NODE *q;

        if ( idn >= 0 ) {
                q = block(FREE, NIL, NIL, tyalloc(MOETY));
                q->tn.rval = idn;
                defid( q, MOE );
        }
}


/* -------------------- bstruct -------------------- */

bstruct( idn, soru )/* begining of structure or union declaration */
int     idn;
int     soru;
{
        register NODE *q;
        register struct symtab *s;

#ifndef BUG1
        if (ddebug) {
                printf("bstruct(idn %d, soru %d), ", idn, soru);
        }
#endif  !BUG1

        psave( instruct );
        psave( curclass );
        psave( strucoff );
        strucoff = 0;
        instruct = soru;
        q = block(FREE, NIL, NIL, tyalloc(UNDEF));
        if ( ( q->tn.rval = idn ) >= 0 ) {
                s = &stab[idn];
                q->in.type = s->stype;
        }
        if ( instruct == INSTRUCT ) {
                curclass = MOS;
                if ( idn >= 0 ) {
                        if ( s->sclass != STNAME )
                                q->in.type = tyalloc(STRTY);
                        defid( q, STNAME );
                }
        } else if ( instruct == INUNION ) {
                curclass = MOU;
                if ( idn >= 0 ) {
                        if ( s->sclass != UNAME )
                                q->in.type = tyalloc(UNIONTY);
                        defid( q, UNAME );
                }
        } else { /* enum */
                curclass = MOE;
                if ( idn >= 0 ) {
                        if ( s->sclass != ENAME )
                                q->in.type = tyalloc(ENUMTY);
                        defid( q, ENAME );
                }
        }
        psave( idn = q->tn.rval );
        /* the "real" definition is where the members are seen */
        if ( idn >= 0 )
                stab[idn].suse = lineno;
        return( paramno - 4 );
}


/* -------------------- TagStruct -------------------- */

TagStruct(q)
register NODE *q;
{
        /*
        ** Tag an incomplete structure or union declaration.
        */
        register struct symtab *s;

        if ( q->tn.rval < 0 )
                cerror(TOOLSTR(M_MSG_223, "tagging unknown structure name" ));

        s = &stab[q->tn.rval];
        if ( blevel > s->slevel )
                defid( q, s->sclass );
        else
                /* "redeclaration of %s" */
                WARNING( WDECLAR, MESSAGE(96), s->psname );
}


/* -------------------- dclstruct -------------------- */

NODE *
dclstruct( oparam )
int     oparam;
{
        register struct symtab *p;
        register i, al, sa, j, sz, szindex;
        register TWORD temp;
        register high, low;
        TWORD qual = 0;
        TPTR type;

        /* paramstack contains:
                paramstack[ oparam ] = previous instruct
                paramstack[ oparam+1 ] = previous class
                paramstk[ oparam+2 ] = previous strucoff
                paramstk[ oparam+3 ] = structure name

                paramstk[ oparam+4, ... ]  = member stab indices

        */


        if ( (i = paramstk[oparam+3]) < 0 ) {
                szindex = curdim;
                dstash( 0 );  /* size */
                dstash( -1 );  /* index to member names */
                dstash( ALSTRUCT );  /* alignment */
                dstash( -lineno );      /* name of structure */
        } else {
                szindex = stab[i].stype->typ_size;
        }

#ifndef BUG1
        if ( ddebug ) {
                printf( "dclstruct( %s ), szindex = %d\n",
                    (i >= 0) ? stab[i].psname : "??", szindex );
        }
#endif  !BUG1

        if ( instruct & INSTRUCT )
                temp = STRTY;
        else if ( instruct & INUNION )
                temp = UNIONTY;
        else {
                temp = ENUMTY;
                type = tynalloc(MOETY);
                type->typ_size = szindex;
        }
        stwart = instruct = paramstk[ oparam ];
        curclass = paramstk[ oparam+1 ];
        dimtab[ szindex+1 ] = curdim;
        al = ALSTRUCT;

        high = low = 0;

        for ( i = oparam + 4; i < paramno; ++i ) {
                dstash( j = paramstk[i] );
                if ( j < 0 || j >= nstabents )
                        cerror(TOOLSTR(M_MSG_224, "gummy structure member" ));
                p = &stab[j];
                if ( temp == ENUMTY ) {
                        if ( p->offset < low )
                                low = p->offset;
                        if ( p->offset > high )
                                high = p->offset;
                        p->stype = type;
                        continue;
                }
                sa = talign( p->stype );
                if ( p->sclass & FIELD ) {
                        sz = p->sclass & FLDSIZ;
                } else {
                        sz = tsize( p->stype );
                }
                if (  sz > ( unsigned ) strucoff )
                        strucoff = sz;  /* for use with unions */
                SETOFF( al, sa );
                /* set al, the alignment, to the lcm of the alignments of the members */

                /*
                ** If any members are qualified, propagate the qualifications
                ** to the struct/union type.
                */
                type = p->stype;
                if (!devdebug[STRUCTBUG])
                        while (ISARY(type))
                                type = DECREF(type);
                if (ISCONST(type) || HASCONST(type))
                        qual |= HAVECONST;
                if (ISVOLATILE(type) || HASVOLATILE(type))
                        qual |= HAVEVOLATILE;
        }
        dstash( -1 );  /* endmarker */
        SETOFF( strucoff, al );

        if ( temp == ENUMTY ) {
                register TWORD ty;

#ifdef  ENUMSIZE
                ty = ENUMSIZE(high, low);
#else
                if ( (char)high == high && (char)low == low )
                        ty = SCHAR;
                else if ( (short)high == high && (short)low == low )
                        ty = SHORT;
                else
                        ty = INT;
#endif  ENUMSIZE

                strucoff = tsize(tyalloc(ty));
                dimtab[szindex+2] = al = talign(tyalloc(ty));
        }

        if ( strucoff == 0 ) {
                /* "zero sized structure" */
                UERROR( ALWAYS, MESSAGE(121) );
                strucoff = SZINT;
                al = ALINT;
        }
        dimtab[ szindex ] = strucoff;
        dimtab[ szindex+2 ] = al;
        dimtab[ szindex+3 ] = paramstk[ oparam+3 ];  /* name index */

        FIXSTRUCT( szindex, oparam ); /* local hook, eg. for sym debugger */
#ifndef BUG1
        if ( ddebug > 1 ) {
                printf( "\tdimtab[%d,%d,%d] = %d,%d,%d\n",
                    szindex, szindex + 1, szindex + 2,
                    dimtab[szindex], dimtab[szindex+1], dimtab[szindex+2] );
                for ( i = dimtab[szindex+1]; dimtab[i] >= 0; ++i ) {
                        printf( "\tmember %s(%d)\n", stab[dimtab[i]].psname, dimtab[i] );
                }
        }
#endif  !BUG1

        strucoff = paramstk[ oparam+2 ];
        paramno = oparam;

        if ((i = paramstk[oparam+3]) >= 0) {
                type = stab[i].stype;
        } else {
                type = tynalloc(temp);
                type->typ_size = szindex;
        }
        if (qual)
                type = qualmember(type, qual);

        return (mkty(type));
}


/* -------------------- yyerror -------------------- */

/* VARARGS */
yyerror( s )/* error printing routine in parser */
char    *s;
{
        UERROR( ALWAYS, s );
}


/* -------------------- yyaccpt -------------------- */

yyaccpt()
{
        ftnend();
}


/* -------------------- ftnarg -------------------- */

ftnarg( idn )
int     idn;
{
        switch (TOPTYPE(stab[idn].stype)) {

        case UNDEF:
                /* this parameter, entered at scan */
                if ( stab[idn].slevel < blevel )
                        idn = hide( &stab[idn], 0 );
                break;
        default:
                switch ( stab[idn].sclass ) {
                case PARAM:
                case PARAMREG:
                case REGISTER:
                        if ( stab[idn].slevel == blevel ) {
                                /* "redeclaration of formal parameter, %s" */
                                UERROR( ALWAYS, MESSAGE(97), stab[idn].psname );
                                goto enter;
                        }
                        break;
                }
                idn = hide( &stab[idn], 0 );
                break;
        case TNULL:
                cerror(TOOLSTR(M_MSG_225, "unprocessed parameter" ));
        }
enter:
        stab[idn].stype = tyalloc(FARG);
        stab[idn].sclass = PARAM;
        stab[idn].slevel = blevel;
        psave( idn );
        return( idn );
}


/* -------------------- talign -------------------- */

talign( ty )/* compute the alignment of an object with type ty */
register TPTR ty;
{

        for (; !ISBTYPE(ty); ty = DECREF(ty)) {
                switch (TOPTYPE(ty)) {

                case FTN:
                        return( ALFTN );
                case PTR:
                        return( ALPOINT );
                case ARY:
                        continue;
                }
        }

        switch (TOPTYPE(ty)) {

        case UNIONTY:
        case ENUMTY:
        case STRTY:
                return( (unsigned int) dimtab[ ty->typ_size+2 ] );
        case SCHAR:
        case CHAR:
        case UCHAR:
        case TVOID:
                return( ALCHAR );
        case FLOAT:
                return( ALFLOAT );
        case DOUBLE:
                return( ALDOUBLE );
        case LDOUBLE:
                return( ALLDOUBLE );
        case LNGLNG:
        case ULNGLNG:
                return( ALLNGLNG );
        case LONG:
        case ULONG:
                return( ALLONG );
        case SHORT:
        case USHORT:
                return( ALSHORT );
        default:
                return( ALINT );
        }
}


/* -------------------- tsize -------------------- */

OFFSZ
tsize( ty )
register TPTR ty;
{
        /* compute the size associated with type ty */
        /* BETTER NOT BE CALLED WHEN ty REFERS TO A BIT FIELD... */

        OFFSZ mult;

        mult = 1;
        for ( ; !ISBTYPE(ty); ty = DECREF(ty) ) {
                switch ( TOPTYPE(ty) ) {
                case FTN:
                        /* "cannot take size of a function" */
                        UERROR( ALWAYS, MESSAGE(170) );
                        return( SZINT );
                case PTR:
                        return( SZPOINT * mult );
                case ARY:
                        if ( ty->ary_size == 0 ) {
                                /* "unknown array size" */
                                UERROR( ALWAYS, MESSAGE(114) );
                                ty->ary_size = 1;
                        }
                        mult *= ty->ary_size;
                        continue;
                }
        }

        if ( dimtab[ty->typ_size] == 0 ) {
                switch ( TOPTYPE(ty) ) {
                case TVOID:
                        /* "illegal use of void type" */
                        UERROR( ALWAYS, MESSAGE(147) );
                        return( SZINT * mult );
                        /*NOTREACHED*/
                        break;
                case STRTY:
                case UNIONTY:
                        /* "undefined structure or union" */
                        UERROR( ALWAYS, MESSAGE(112) );
                case ENUMTY:    /* Error handled in rstruct() */
                        dimtab[ty->typ_size] = SZINT;
                        break;
                default:
                        cerror(TOOLSTR(M_MSG_226, "unknown size for type 0%o"), TOPTYPE(ty) );
                }
        }
        return( (OFFSZ) dimtab[ty->typ_size] * mult );
}


/* -------------------- chkty -------------------- */

chkty( q )
register struct symtab *q;
{
        register TPTR ty;

        for ( ty = q->stype; !ISBTYPE(ty); ty = DECREF(ty) ) {
                switch ( TOPTYPE(ty) ) {
                case FTN:
                        cerror(TOOLSTR(M_MSG_227, "defining function variable %s"), q->psname );
                case PTR:
                        return;
                case ARY:
                        if ( ty->ary_size == 0 ) {
                                /* "unknown array size for %s" */
                                WARNING( ALWAYS, MESSAGE(168), q->psname );
                                ty->ary_size = 1;
                        }
                        continue;
                }
        }

        if ( dimtab[ty->typ_size] == 0 ) {
                switch ( TOPTYPE(ty) ) {
                case STRTY:
                case UNIONTY:
                        /* "undefined structure or union for %s" */
                        UERROR( ALWAYS, MESSAGE(169), q->psname );
                case ENUMTY:    /* Error handled in rstruct() */
                        dimtab[ty->typ_size] = SZINT;
                        break;
                default:
                        cerror(TOOLSTR(M_MSG_228, "unknown size for %s type 0%o"), q->psname, TOPTYPE(ty) );
                }
        }
}


/* -------------------- inforce -------------------- */

inforce( n )
OFFSZ n;
{       /* force inoff to have the value n */
        /* inoff is updated to have the value n */
        OFFSZ wb;
        register rest;  /* rest is used to do a lot of conversion to ints... */

        if ( inoff == n )
                return;
        if ( inoff > n ) {
                cerror(TOOLSTR(M_MSG_229, "initialization alignment error"));
        }
        wb = inoff;
        SETOFF( wb, SZINT );

        /* wb now has the next higher word boundary */

        if ( wb >= n ) { /* in the same word */
                rest = n - inoff;
                vfdzero( rest );
                return;
        }

        /* otherwise, extend inoff to be word aligned */

        rest = wb - inoff;
        vfdzero( rest );

        /* now, skip full words until near to n */

        rest = (n - inoff) / SZINT;
        zecode( rest );

        /* now, the remainder of the last word */

        rest = n - inoff;
        vfdzero( rest );
        if ( inoff != n )
                cerror(TOOLSTR(M_MSG_230, "inoff error"));

}


/* -------------------- vfdalign -------------------- */

vfdalign( n ) /* make inoff have the offset the next alignment of n */
int     n;
{
        OFFSZ m;

        m = inoff;
        SETOFF( m, n );
        inforce( m );
}


int     idebug = 0;
int     ibseen = 0;  /* the number of } constructions which have been filled */
int     iclass;  /* storage class of thing being initialized */
int     ilocctr = 0;  /* location counter for current initialization */


/* -------------------- beginit -------------------- */

beginit(curid) /* beginning of initilization; set location ctr and set type */
int     curid;
{

        register struct symtab *p;

#ifndef BUG1
        if ( idebug >= 3 )
                printf( "beginit(), curid = %d\n", curid );
#endif  !BUG1

        p = &stab[curid];

        iclass = p->sclass;
        switch ( iclass ) {

        case UNAME:
                return;
        case EXTERN:
        case AUTO:
        case AUTOREG:
        case REGISTER:
                break;
        case EXTDEF:
        case STATIC:
                ilocctr = ISARY(p->stype) ? ADATA : DATA;
                locctr( ilocctr );
                defalign( talign( p->stype ) );
                defnam( p );

        }

        inoff = 0;
        ibseen = 0;
        partelided = 0;
        pstk = 0;

        instk( curid, p->stype, inoff );

}


/* -------------------- instk -------------------- */

instk( id, t, off )/* make a new entry on the parameter stack to initialize id */
int     id;
TPTR t;
OFFSZ off;
{

        register struct symtab *p;

        tempstack = pstk;

        for (; ; ) {
#ifndef BUG1
                if ( idebug )
                        printf( "instk((%d, %o, %d)\n", id, tyencode(t), off );
#endif  !BUG1

                /* save information on the stack */

                if ( (pstk + 1) > &instack[INIT_STACK_SIZE]) {
                        if (idebug)
                                dumpstack("Initialization Stack Overflow");
                        cerror(TOOLSTR(M_MSG_231, "Initialization Stack Overflow"));
                }

                if ( !pstk ) {
                        pstk = instack;
                        tempstack = pstk;
                } else
                        ++pstk;

                pstk->in_fl = 0;        /* left brace flag */
                pstk->in_id =  id ;
                pstk->in_t =  t ;
                pstk->in_n = 0;  /* number seen */
                pstk->in_x = (TOPTYPE(t) == STRTY || TOPTYPE(t) == UNIONTY) ? dimtab[t->typ_size+1] : 0;

                pstk->in_off =  off;   /* offset at beginning of this element */
                /* if t is an array, DECREF(t) can't be a field */
                /* INS_sz has size of array elements, and -size for fields */
                if ( ISARY(t) ) {
                        pstk->in_sz = tsize( DECREF(t) );
                } else if ( stab[id].sclass & FIELD ) {
                        pstk->in_sz = -( stab[id].sclass & FLDSIZ );
                } else {
                        pstk->in_sz = 0;
                }

                /* now, if this is not a scalar, put on another element */

                if ( ISARY(t) ) {
                        t = DECREF(t);
                        continue;
                } else if (TOPTYPE(t) == STRTY) {
                        id = dimtab[pstk->in_x];
                        if (id < 0 || id > nstabents ) {
                                /* "illegal structure initialization" */
                                UERROR( ALWAYS, MESSAGE(176) );
                                return;
                        }
                        p = &stab[id];
                        if ( p->sclass != MOS && !(p->sclass & FIELD) ) {
                                /* "illegal structure initialization" */
                                UERROR( ALWAYS, MESSAGE(176) );
                                return;
                        }
                        t = p->stype;
                        off += p->offset;
                        continue;

                } else if (TOPTYPE(t) == UNIONTY) {
                        id = dimtab[pstk->in_x];
                        if (id < 0 || id > nstabents ) {
                                /* "illegal union initialization" */
                                UERROR( ALWAYS, MESSAGE(177) );
                                return;
                        }
                        p = &stab[id];
                        if ( p->sclass != MOU && !(p->sclass & FIELD) ) {
                                /* "illegal union initialization" */
                                UERROR( ALWAYS, MESSAGE(177) );
                                return;
                        }
                        t = p->stype;
                        off += p->offset;
                        continue;
                } else
                        return;
        }
}


/* -------------------- getstr -------------------- */

/*
** decide if the string is external or an initializer,
** and get the contents accordingly
** string type is either the token STRING, or WSTRING
*/
NODE *
getstr(string_type)
int     string_type;
{
        register l;
#ifndef XCOFF
        register temp;
#endif  !XCOFF
        register NODE *p;
#ifndef XCOFF
# ifndef        ONEPASS
        struct symtab STEnt;
# endif !ONEPASS
#endif  !XCOFF

        if ((iclass == EXTDEF || iclass == STATIC) && 
            (TOPTYPE(pstk->in_t) == CHAR || 
            TOPTYPE(pstk->in_t) == UCHAR || 
            TOPTYPE(pstk->in_t) == SCHAR || 
            TOPTYPE(pstk->in_t) == WCHAR) && 
            pstk != instack && ISARY(pstk[-1].in_t)) {

                if ((ISWCHAR(pstk->in_t) && string_type != WSTRING) || 
                    (ISCHAR(pstk->in_t) && string_type == WSTRING)) {
                        UERROR(ALWAYS, MESSAGE(61));
                }

                /* treat "abc" as { 'a', 'b', 'c', 0 } */

                strflg = 1;
                Initializer = LIST;
                pstk[-1].in_fl = 1; /* simulate left brace -- hardwired */
                inforce( pstk->in_off );
                /* if the array is inflexible (not top level), pass in the
                 * size and be prepared to throw away unwanted initializers */
                /* get the contents */
                lxstr((pstk - 1) != instack ? (pstk - 1)->in_t->ary_size : 0);
                irbrace();  /* simulate right brace */
                Initializer = NOINIT;

                /* string_type is either the token STRING, or WSTRING */
                return( buildtree( string_type, NIL, NIL ) );

        } else { /* make a label, and get the contents and stash them away */
                Initializer = LIST;
                if ( iclass != SNULL ) { /* initializing */
                        /* fill out previous word, to permit pointer */
                        vfdalign( ALPOINT );
                }
                /* set up location counter */
#ifndef XCOFF
                temp = locctr( blevel == 0 ? ISTRNG : STRNG );
#endif  !XCOFF
                /* ROMP has a Dhrystone problem..its too slow */
                /* character string constants (& char arrays) */
                /* are word aligned (crm) */
#ifdef  XCOFF
                if (saved_strings >= max_strings) {
                        saved_lab = (int *) realloc(saved_lab, (max_strings + 100) * sizeof(int));
                        max_strings += 100;
                }
                saved_lab[saved_strings++] = l = getlab();
#else
# ifdef IS_COMPILER
                printf("\t.align\t2\n");
# endif IS_COMPILER
                deflab( l = getlab() );
#endif  XCOFF
                strflg = 0;
                lxstr(0); /* get the contents */
#ifndef XCOFF
                locctr( blevel == 0 ? ilocctr : temp );
#endif  !XCOFF

                /* string_type is either WSTRING or STRING */
                p = buildtree( string_type, NIL, NIL );

                p->tn.rval = -l;
#ifndef XCOFF
# ifndef        ONEPASS
                /* print out fake symbol table info for this string */
                STEnt.sclass = STATIC;
                STEnt.slevel = 2;
                STEnt.stype = p->in.type;
                STEnt.offset = l;
                STEnt.psname = "string!";
                StabInfoPrint(&STEnt);
# endif !ONEPASS
#endif  !XCOFF
                Initializer = NOINIT;
                return(p);
        }
}


#ifdef  XCOFF
/* -------------------- savestr -------------------- */

savestr(val)
int     val;
{
        if (saved_chars >= max_chars) {
                saved_str = (int *) realloc(saved_str, (max_chars + 2000) * sizeof(int));
                max_chars += 2000;
        }
        saved_str[saved_chars++] = val;
}


/* -------------------- unbuffer_str -------------------- */

/*      while there are more strings to output
            print align 2
            print label
            print the string (ends with -1)
            print 0
            print align 2
*/

unbuffer_str()
{
        register i = 0;
        register j = 0;
        register k;

        if (i >= saved_strings)
                return;

        locctr( STRNG );

        while (i < saved_strings) {
# ifdef IS_COMPILER
                printf("\t.align\t2\n");
# endif IS_COMPILER
                deflab(saved_lab[i++]);
                for (k = 0; saved_str[j] != -1; )
                        bycode(saved_str[j++], k++);
                j++;    /* move past that -1 */
                bycode(0, k++);
                bycode(-1, k);
# ifdef IS_COMPILER
                printf("\t.align\t2\n");
# endif IS_COMPILER
        }

        saved_strings = saved_chars = 0;

        locctr( PROG );
}


#endif  XCOFF


/* -------------------- putbyte -------------------- */

putbyte( v )/* simulate byte v appearing in a list of integer values */
int     v;
{
        register NODE *p;

        p = bcon(v);
        incode( p, SZCHAR );
        tfree( p );
        gotscal();
}


/* -------------------- endinit -------------------- */

endinit()
{
        register TPTR t;
        register n, d1;

#ifndef BUG1
        if ( idebug )
                printf( "endinit(), inoff = %d\n", inoff );
#endif  !BUG1

        foldMask = EXPRESSION;

        switch ( iclass ) {

        case EXTERN:
                if (blevel)
                        WERROR( ALWAYS, MESSAGE(19) );
        case AUTO:
        case AUTOREG:
        case REGISTER:
                return;
        }

        pstk = instack;

        t = pstk->in_t;
        n = pstk->in_n;

        if ( ISARY(t) ) {
                d1 = t->ary_size;

                vfdalign( pstk->in_sz );  /* fill out part of the last element, if needed */
                n = inoff / pstk->in_sz;  /* real number of initializers */
                if ( d1 >= n ) {
                        /* once again, t is an array, so no fields */
                        inforce( tsize( t ) );
                        n = d1;
                }

                /*
                 * if the number of characters in the string (including
                 * the terminating null) equals the size of the
                 * array plus 1 (i.e., d1) then we warn about
                 * the fact that the null is not guarranteed to be there at
                 * the end of the char array.
                 */
                if (ISCHAR(DECREF(t)) && (d1 + 1) == n && d1 != 0) {
                        /* "array not large enough to store terminating null" */
                        WARNING( WSTORAGE, MESSAGE(175) );
                        goto endendinit;
                }

                /* "too many initializers" or
		   "non-null byte ignored in string initializer" */
                if ( d1 != 0 && d1 != n ) {
		        if (ISCHAR(DECREF(t)))
                                WARNING( ALWAYS, MESSAGE(81) );
                        else
                                UERROR( ALWAYS, MESSAGE(108) );
                }

                if ( n == 0 ) {
                        /* "empty array declaration" */
                        UERROR( ALWAYS, MESSAGE(35) );
                        n = 1;
                }
                t->ary_size = n;

        } else if (TOPTYPE(t) == STRTY || TOPTYPE(t) == UNIONTY) {
                /* clearly not fields either */
                inforce( tsize( t ) );

        } else if ( n > 1 )
                /* "bad scalar initialization" */
                UERROR( ALWAYS, MESSAGE(17) );
        else
                /* this will never be called with a field element... */
                inforce( tsize( t ) );

endendinit:
        paramno = 0;
        vfdalign( AL_INIT );
        inoff = 0;
        iclass = SNULL;

}


/* -------------------- doinit -------------------- */

extern eprint();

doinit( p )
register NODE *p;
{

        register sz;
        register TPTR t;
        register NODE *l;
        register struct instk *itemp = pstk;

        if (idebug > 2)
                dumpstack("doinit initial stack dump");
        /*
         * check for partial elidation ...
         * search for topmost aggregate, and that should have
         * a left brace associated with it, or otherwise 
         * we have a partial elidation.
         */
        for ( ; itemp > (instack + 1); ) {
                --itemp;
                t = itemp->in_t;
                if (ISAGGREGATE(t)) {
                        if (itemp->in_fl == 0) {
                                /* "partially elided initialization" */
                                WARNING(partelided == 0 && WANSI && WKNR, MESSAGE(180));
                                partelided++;
                        }
                        break;
                }
        }

        /*
         * take care of generating a value for the initializer p.
         *
         * inoff has the current offset (i.e., last bit written)
         * in the current word being generated.
         *
         * note: size of an individual initializer is assumed
         * to fit into an int.
         *
         * reset the idname from the stack. however, for scalars
         * we use the top of the stack, while for aggregates
         * we use the stack entry at tempstack.
         *
         * first thing is to determine the FOLD_EXPR mode depending on
         * the block level and storage class. 
         *
         */
        foldMask = iclass == STATIC || blevel == 0 ? GENERAL_CONSTANT : EXPRESSION;

        /* RW: 19112, ENUMTYs have nothing to with this...,
        ** The above documentation appears incorrect.
        ** The else part of the if is (always?) executed.
        */
        if (ISAGGREGATE(pstk->in_t)) {
                idname = pstk->in_id;
        } else {
                idname = tempstack->in_id;
        }

        if ( iclass < 0 )
                goto leave;

        if ( iclass == AUTO || iclass == AUTOREG || iclass == REGISTER || iclass == EXTERN ) {
                /*
                 * for scalars:
                 * do the initialization and get out, without regard
                 * for filing out the variable with zeros, etc.
                 * for aggregates or unions:
                 *      1. initialize a data segment area with the structure.
                 *      2. reserve an area on the stack suitable to hold
                 * the object.
                 *      3. perform a structure assignment from the data
                 * segment to the stack area.
                 */
                bccode();
                ininit = 1;
                p = buildtree( ASSIGN, buildtree( NAME, NIL, NIL ), p );
                ininit = 0;
                ecomp(p);
                return;
        }

        /* for throwing away strings that have been turned into lists */
        if ( p->in.op == NAME && p->tn.rval == NOLAB ) {
                p->in.op = FREE;
                return;
        }

#ifndef BUG1
        if ( idebug > 1 )
                printf( "doinit(%o)\n", p );
#endif  !BUG1

        t = pstk->in_t;  /* type required */
        if ( pstk->in_sz < 0 ) {  /* bit field */
                sz = -pstk->in_sz;
        } else {
                sz = tsize( t );
        }

        /*
         * check if we have too many initializers. if so
         * produce an error message rather that a compiler error message.
         */
        if (inoff > pstk->in_off) {
                UERROR( ALWAYS, MESSAGE(108) );
                goto leave;
        }

        inforce( pstk->in_off );

        ininit = 1;
        /*
         * if we have a single expression initializer then
         * we want to make sure we can perform the object assignment since
         * that expression should initialize the entire object.
         * in case of an structure or a union that expression should
         * have compatible type.
         */

        if (ISSINGLE)
                p = buildtree( ASSIGN, buildtree( NAME, NIL, NIL), p );
        else
                p = buildtree( ASSIGN, block( NAME, NIL, NIL, t ), p );

        ininit = 0;
        p->in.left->in.op = FREE;
        p->in.left = p->in.right;
        p->in.right = NIL;

#ifndef BUG1
        /* tree before optimizing/folding */
        if (bdebug > 2)
                fwalk(p, eprint, 0);
#endif  !BUG1

        p->in.left = foldexpr( optim(p->in.left) );

#ifndef BUG1
        /* tree after optimizing/folding */
        if (bdebug > 2)
                fwalk(p, eprint, 0);
#endif  !BUG1

        p->in.op = INIT;

        /*
         * the following if-statement throws aways SCONVs to a float type
         * in single precision mode. clocal does not throw these away.
         * Now, the SCONV node is not thrown away unless we're n
         * EXPRESSION mode (i.e., NO_FOLD()).
         * (INIT (SCONV[ISFLOAT] x) nil) => (INIT x nil)
         * or
         * (INIT (U& x) nil) => (INIT x nil)
         */
        l = p->in.left;
        if ( (l->in.op == UNARY AND)
#ifdef  SINGLE_PRECISION
             || (NO_FOLD() && l->in.op == SCONV && ISFLOAT(l->in.type))
#endif  SINGLE_PRECISION
            ) {
                p->in.left->in.op = FREE;
                p->in.left = p->in.left->in.left;
        }

        /*
         * special case: for bit fields sz may be less than SZINT
         */
        if ( sz < SZINT ) {
                if ( p->in.left->in.op != ICON ) {
                        /* "illegal initialization" - changed defect 75535 */
                        if (blevel > 0)
                                if(devdebug[ANSI_MODE])
                                        UERROR(ALWAYS, MESSAGE(61));
                                else
                                        WARNING(WANSI, MESSAGE(61));
                        else
                                UERROR(ALWAYS, MESSAGE(61));
                }
                else
                        incode( p->in.left, sz );
        } else if ( p->in.left->in.op == FCON ) {
#ifndef NOFLOAT
                fincode( p->in.left->fpn.dval, sz );
#else
                cerror(TOOLSTR(M_MSG_232, "a floating point constant in a NOFLOAT compiler?"));
#endif  !NOFLOAT
        } else {
                cinit( foldexpr( optim(p) ), sz );
        }
        gotscal();
leave:
        /*
         * restore the folding mechanism into
         * its standard mode.
         */
        foldMask = EXPRESSION;

        tfree(p);
}


/* -------------------- gotscal -------------------- */

gotscal()
{
        register TPTR t;
        register ix;
        register n, id;
        struct symtab *p;
        OFFSZ temp;

#ifndef BUG1
        if ( idebug )
                printf( "gotscal(%p)\n", pstk );
#endif  !BUG1

        for ( ; pstk > instack; ) {
                --pstk;

#ifndef BUG1
                if ( idebug )
                        printf( "gotscal(%p)\n", pstk );
#endif  !BUG1

                t = pstk->in_t;

                if (TOPTYPE(t) == STRTY) {
                        ix = ++pstk->in_x;
                        if ( (id = dimtab[ix]) < 0 ) {
                                if ((pstk)->in_fl)
                                        return;
                                else if (partelided == 0 && ibseen > 0) {
                                        /* "partially elided initialization" */
                                        WARNING(WANSI && WKNR, MESSAGE(180));
                                        partelided++;
                                }
                                continue;
                        }

                        /* otherwise, put next element on the stack */

                        p = &stab[id];
                        instk( id, p->stype, p->offset + pstk->in_off );
                        return;
                } else if ( ISARY(t) ) {
                        n = ++pstk->in_n;
                        if (n >= t->ary_size && pstk > instack) {
                                if ((pstk)->in_fl)
                                        return;
                                else if (partelided == 0 && ibseen > 0) {
                                        /* "partially elided initialization" */
                                        WARNING(WANSI && WKNR, MESSAGE(180));
                                        partelided++;
                                }
                                continue;
                        }

                        /* put the new element onto the stack */

                        temp = pstk->in_sz;
                        instk (pstk->in_id, DECREF(pstk->in_t), pstk->in_off + n * temp );
                        return;

                } else if ( TOPTYPE(t) == UNIONTY ) {
                        if (pstk->in_fl)
                                return;
                        else if (partelided == 0 && ibseen > 0) {
                                /* "partially elided initialization" */
                                WARNING(WANSI && WKNR, MESSAGE(180));
                                partelided++;
                        }
                        continue;
                }
        }
}


/* -------------------- ilbrace -------------------- */

ilbrace()/* process an initializer's left brace */
{
        register TPTR t;
        struct instk *temp;
        int     structflag;

#ifndef BUG1
        if ( idebug )
                printf( "ilbrace(): paramno = %d on entry\n", paramno );
#endif  !BUG1

        temp = pstk;
        pstk = tempstack + 1;
        ibseen++;
        structflag = 0; /* no aggregate found in stack */

        for ( ; pstk <= temp; pstk++) {
                t = pstk->in_t;
                if (TOPTYPE(t) == STRTY || ISARY(t) || TOPTYPE(t) == UNIONTY) {
                        if ( pstk->in_fl == 0 ) {
                                /* we have one ... */
                                pstk->in_fl = 1;
                                structflag = 1; /* aggregate found in stack */
                                break;
                        } else {
                                continue;
                        }
                }
        }
        pstk = temp;
        if (structflag == 0) {
                pstk->in_fl = 1;
        }
#ifndef BUG1
        if ( idebug )
                dumpstack("ilbrace");
#endif  !BUG1
}


/* -------------------- irbrace -------------------- */

irbrace()
{
        struct instk *temp = pstk;

#ifndef BUG1
        if ( idebug ) {
                printf( "irbrace(): paramno = %d on entry\n", paramno );
                dumpstack("irbrace1");
        }
#endif  !BUG1

        for ( ; pstk > instack; --pstk ) {
                if ( pstk->in_fl == 0 )
                        continue;
                else {
                        /* we have one now */
                        if (ibseen > 0) {
                                inforce(tsize(pstk->in_t) + pstk->in_off);
                                pstk->in_fl = 0;
                                --ibseen;
#ifndef BUG1
                                if ( idebug )
                                        dumpstack("irbrace2");
#endif  !BUG1
                                gotscal();
                        } else {
                                pstk->in_fl = 0;  /* cancel left brace */
                                gotscal();  /* take it away... */
                        }
                        return;
                }
        }
        pstk = temp;
}


/* -------------------- upoff -------------------- */

upoff( size, alignment, poff )
register int    size;
register int    alignment;
register OFFSZ *poff;
{
        /* update the offset pointed to by poff; return the
         * offset of a value of size `size', alignment `alignment',
         * given that off is increasing */

        register OFFSZ off;

        off = *poff;
        SETOFF( off, alignment );
        if ( (offsz - off) <  size ) {
                if ( instruct != INSTRUCT )
                        cerror(TOOLSTR(M_MSG_233, "too many local variables"));
                else
                        cerror(TOOLSTR(M_MSG_234, "Structure too large"));
        }
        *poff = off + size;
        return( off );
}


/* -------------------- oalloc -------------------- */

oalloc( p, poff ) /* allocate p with offset *poff, and update *poff */
register struct symtab *p;
register *poff;
{
        register al, tsz;
        OFFSZ noff, off;

        al = talign( p->stype );
        noff = off = *poff;
        /* Note to KR: PARAMETERS of type FLOAT are converted into DOUBLE
         * always. That may change, and don't forget LDOUBLEs as well.
         */
        if ( p->sclass == PARAM || p->sclass == PARAMREG )
                tsz = tsize(TOPTYPE(p->stype) == FLOAT ? tyalloc(DOUBLE) : p->stype);
        else
                tsz = tsize( p->stype );
#ifdef  BACKAUTO
        if ( p->sclass == AUTO || p->sclass == AUTOREG ) {
                if ( (offsz - off) < tsz )
                        cerror(TOOLSTR(M_MSG_233, "too many local variables"));
                noff = off + tsz;
                SETOFF( noff, al );
                off = -noff;
        } else
#endif  BACKAUTO

                /* align char/short PARAM and REGISTER to ALINT */
                /* but don't align structures and unions */
                if ( (p->sclass == PARAM || p->sclass == PARAMREG || p->sclass == REGISTER)
                     && (tsz < SZINT) && (TOPTYPE(p->stype) != STRTY)
                     && (TOPTYPE(p->stype) != UNIONTY) ) {
                        off = upoff( SZINT, ALINT, &noff );
#ifndef RTOLBYTES
                        off = noff - tsz;
#endif  !RTOLBYTES
                } else {
                        /* automatic char arrays aligned on word boundaries..crm */
                        /* (Don't look like that!  This is for Dhrystone wars.) */
                        if ( ( BTYPE(p->stype) == CHAR || BTYPE(p->stype) == UCHAR || 
                            BTYPE(p->stype) == SCHAR) && 
                            ISARY(p->stype) && (p->sclass == AUTO || p->sclass == AUTOREG))
                                off = upoff( tsz, ALINT, &noff);
                        else
                                off = upoff( tsz, al, &noff );
                }

        /* in case we are allocating stack space for register arguments */
        if ( p->sclass != REGISTER ) {
                if ( p->offset == NOOFFSET )
                        p->offset = off;
                else if ( off != p->offset )
                        return(1);
        }

        if (adebug && ISARY(p->stype) && (p->sclass == AUTO || p->sclass == AUTOREG || 
            p->sclass == PARAM || p->sclass == PARAMREG) ) {
                register int    xoffset = off / SZCHAR;
#ifdef  IS_COMPILER
                printf("\t.copt\tsym,%d,%ld+L.%d%c\n", (tsz + SZCHAR - 1) / SZCHAR,
                    xoffset, ftnno, (p->sclass == PARAM || p->sclass == PARAMREG) ? 'A' : 'L' );
#endif  IS_COMPILER
                if ((p->sclass == PARAM || p->sclass == PARAMREG) && (xoffset < minsvarg))
                        minsvarg = xoffset;
        }
        *poff = noff;
        return(0);
}


/* -------------------- markaddr -------------------- */

markaddr(p)
register NODE *p;
{
        /* If p has for *(STKREG+const) or *(ARGREG+const)    */
        /* mark node has had its address taken.               */
        /* minsvarg used to emit stores of reg args in prolog */
        /* This is yucchy but its the best we can do...       */

        register NODE *l, *r, *q;
        register int    size;

        if (p->in.op == PNAME || p->in.op == LNAME) {
                /* atomic local or parameter - easy */
                size = tlen(p);
#ifdef  IS_COMPILER
                if (adebug)
                        printf("\t.copt\tsym,%d,%ld+L.%d%c\n",
                            size, p->tn.lval, ftnno,
                            (p->in.op == PNAME) ? 'A' : 'L' );
#endif  IS_COMPILER
                if (p->in.op == PNAME  &&  p->tn.lval < minsvarg)
                        minsvarg = p->tn.lval;

        } else if ((p->in.op == UNARY MUL)
             && ((q = p->in.left)->in.op == PLUS)
             && ((l = q->in.left)->in.op == PCONV)
             && ((l = l->in.left)->in.op == REG)
             && ((l->tn.rval == STKREG) || (l->tn.rval == ARGREG))
             && ((r = q->in.right)->in.op == ICON)) {
                if (TOPTYPE(p->in.type) == STRTY || TOPTYPE(p->in.type) == UNIONTY)
                        size = tsize(p->in.type) / SZCHAR;
                else
                        size = tlen(p);
#ifdef  IS_COMPILER
                if (adebug)
                        printf("\t.copt\tsym,%d,%ld+L.%d%c\n", size, r->tn.lval, ftnno,
                            (l->tn.rval == ARGREG) ? 'A' : 'L' );
#endif  IS_COMPILER
                if ((l->tn.rval == ARGREG) && (r->tn.lval < minsvarg))
                        minsvarg = r->tn.lval;
        }
}


/* -------------------- falloc -------------------- */

falloc( p, w, new, pty )
register struct symtab *p;
int     w;
int     new;
NODE *pty;
{
        /* allocate a field of width w */
        /* new is 0 if new entry, 1 if redefinition, -1 if alignment */

        register al, sz;
        register TPTR type;
        TWORD qual;

        type = (new < 0) ? pty->in.type : p->stype;

        /* this must be fixed to use the current type in alignments */
        switch (TOPTYPE(type)) {

        case INT:
                if (!ISTSIGNED(type)) {
                        MODTYPE(type, UNSIGNED);
                }
        case UNSIGNED:
                al = talign(type);
                sz = tsize(type);
                break;

		/* LS: long long not allowed in bitfields */
        case CHAR:
        case SCHAR:
        case UCHAR:
        case SHORT:
        case USHORT:
        case LONG:
        case ULONG:
        case ENUMTY:
                if ( new < 0 && devdebug[BITFIELDS] ) {
                        /* "illegal field type" */
                        WERROR( ALWAYS, MESSAGE(57) );
                        al = talign(type);
                        sz = tsize(type);
                        break;
                }
        default:
                /* "illegal bit field type, unsigned assumed" */
                WERROR( ALWAYS, MESSAGE(125) );
                qual = QUALIFIERS(type);
                type = tyalloc(UNSIGNED);
                if ( qual )
                        type = qualtype(type, qual, 0);
                al = talign(type);
                sz = tsize(type);
        }

        if ( w > sz ) {
                /* "field too big" */
                UERROR( ALWAYS, MESSAGE(39) );
                w = sz;
        }

        if ( w == 0 ) {
                if ( new < 0 ) {
                        /* align only */
                        SETOFF( strucoff, al );
                        return( 0 );
                }
                /* "zero size field for %s" */
                UERROR( ALWAYS, MESSAGE(120), p->psname );
                w = 1;
        }

        if ( strucoff % al + w > sz )
                SETOFF( strucoff, al );
        if ( new < 0 ) {
                if ( (offsz - strucoff) < w )
                        cerror(TOOLSTR(M_MSG_234, "structure too large" ));
                strucoff += w;  /* we know it will fit */
                return(0);
        }

        /* establish the field */

        if ( new == 1 ) { /* previous definition */
                if ( p->offset != strucoff || p->sclass != (FIELD | w) )
                        return(1);
        }
        p->offset = strucoff;
        if ( (offsz - strucoff) < w )
                cerror(TOOLSTR(M_MSG_234, "structure too large" ));
        strucoff += w;
        p->stype = type;
        fldty( p );
        return(0);
}


/* -------------------- uclass -------------------- */

uclass( class )
register class;
{
        /* give undefined version of class */
        if ( class == SNULL )
                return( EXTERN );
        else if ( class == STATIC )
                return( USTATIC );
        else if ( class == FORTRAN )
                return( UFORTRAN );
        else
                return( class );
}


/* -------------------- nidcl -------------------- */

nidcl (p)
NODE *p;
{
        /*
        ** Handle unitialized declarations.
        */
        register class = curclass;

        /* Determine class */
        if ( ISFTN(p->in.type) ) {
                class = uclass( class );
        } else if ( blevel == 0 ) {
                if ( class == SNULL ) {
                        class = EXTENT;
                } else if ( ( class = uclass( class ) ) == USTATIC ) {
                        if ( ISARY(p->in.type) ) {
                                if ( p->in.type->ary_size == 0 )
                                        /* "cannot declare incomplete static object" */
                                        WERROR( ALWAYS, MESSAGE(191) );
                        } else if ( !ISPTR(p->in.type) ) {
                                if ( dimtab[p->in.type->typ_size] == 0 )
                                        /* "cannot declare incomplete static object" */
                                        WERROR( ALWAYS, MESSAGE(191) );
                        }
                }
        }

        defid( p, class );

        if ( class == STATIC )
                /* Must be block scope so define it now */
                commdec( &stab[p->tn.rval] );
}


/* -------------------- deftents -------------------- */

deftents ()
{
#ifndef CXREF
        register struct symtab *p;
        register int    i;

        for ( i = 0, p = stab; i < nstabents; i++, p++) {

                if ( TOPTYPE(p->stype) == TNULL )
                        continue;

                switch ( p->sclass ) {
                case USTATIC:
                        if ( ISFTN(p->stype) || p->suse > 0 )
                                break;
                case EXTENT:
                        commdec( p );
                        break;
                }
# ifdef LINT
                /*
                ** Don't emit undefined/defined static symbols.
                ** All symbols with class EXTENT should now be EXTDEF.
                ** cflow needs the class promotion results.
                */
                if ( p->sclass != USTATIC && p->sclass != STATIC )
                        OutSymbol(p, 0);
# endif LINT
# ifdef CFLOW
                OutSymbol(p, 0);
# endif CFLOW
        }
#endif  !CXREF
}


#if     defined (LINT) || defined (CFLOW)
/*
** The following lint support functions emit data to the second-pass
** using an intermediate file.
** The BIO define selects output in either binary mode (if defined)
** or ascii mode (if not defined) for debugging.
*/


# define        BIO     1               /* binary i/o selected */
/* # define     IODEBUG */
int     iodebug = 0;

/* ------------------- OutFileBeg -------------------- */

OutFileBeg(iocode) /* Indicate beginning of a new physical file. */
char    iocode;
{
# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite(pfname, strlen(pfname) + 1, 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", pfname);
# endif BIO
}


/* ------------------- OutFileEnd -------------------- */

OutFileEnd(iocode) /* Indicate end of a physical file. */
char    iocode;
{
# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
# endif BIO
}


/* -------------------- OutSymbol -------------------- */

/*
** Write the type information of a symbol table entry.
** Partially interpreted by second pass as a header record.
*/
OutSymbol(p, ftnfix)
register struct symtab *p;
int     ftnfix;
{
        register char   class;
        short   rline;
        char    iocode = 0;
        short   usage = 0;

        /*
        ** Don't emit any functions since these are emitted directly
        ** from the grammar (cgram.y), with the exception of undefined
        ** function prototypes (declared/used only), or unused old-style
        ** function declarations.
        */
        if (!ftnfix && ISFTN(p->stype)) {
                if (p->sclass == EXTERN && ((p->stype->ftn_parm != PNIL) || 
                    ((p->stype->ftn_parm == PNIL) && p->suse > 0)))
                        ;
                else
                        return;
        }

        /* Members of complex types are emitted later on. */
        switch (class = p->sclass) {
        case MOS:
        case MOU:
        case MOE:
        case TYPEDEF:
                return;
        default:
                if (class & FIELD)      /* bit field? */
                        return;
        }
        iocode = LINTSYM;

        /* Determine usage. */
        if (lintused)
                usage |= LINTNOT;       /* check for NOTUSED */
        if (lintdefd)
                usage |= LINTNDF;       /* check for NOTDEFINED */
        usage |= p->sflags & SNSPACE;
        if ((rline = p->suse) < 0) {
                rline = -rline;
                usage |= LINTREF;       /* symbol referenced */
        }
        if (class == EXTERN)
                usage |= LINTDCL;       /* symbol declared */
        else if (class == EXTDEF)
                usage |= LINTDEF;       /* symbol defined */
# ifdef CFLOW
                /* Case when called by WarnWalk (not by deftents). */
        else if (class == USTATIC || class == STATIC)
                usage |= LINTDEF;       /* symbol defined */
# endif CFLOW

        if (class == STNAME || class == UNAME || class == ENAME) {
# ifdef CFLOW
                /* cflow doesn't like tagnames */
                return;
# else
                usage |= LINTDEF;       /* symbol defined */
                usage |= LINTMBR;       /* symbol has members */
# endif CFLOW
        }

        if (ftnfix)
                usage &= ~LINTDEF;

        /* Prototype-specific checks. */
        if (ISFTN(p->stype)) {

                /*RW: P19915 */
                usage |= LINTRET;       /* ALWAYS check for return value */

                if (lintrsvd) {
                        usage |= LINTLIB;
                        usage |= LINTDEF;
                        usage |= LINTNDF;
                }
        }

# ifdef IODEBUG
        if (iodebug) {
                printf("(S)%s <", p->psname);
                tprint(p->stype);
                printf("> %s 0%o\n", scnames(p->sclass), usage);
        }

# endif IODEBUG

# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite((char *) p->psname, strlen(p->psname) + 1, 1, tmplint);
        fwrite((char *) p->ifname, strlen(p->ifname) + 1, 1, tmplint);
        fwrite((char *) & p->line, sizeof(short), 1, tmplint);
        fwrite((char *) & rline, sizeof(short), 1, tmplint);
        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", p->psname);
        fprintf(tmplint, "%s\n", p->ifname);
        fprintf(tmplint, "%d\n", p->line);
        fprintf(tmplint, "%d\n", rline);
        fprintf(tmplint, "%d\n", usage);
# endif BIO

        OutType(p->stype);

# ifndef        CFLOW   /* cflow(1) does not care about member information */
        /* Members are emitted together now. */
        if (usage & LINTMBR) {
                short   cnt;
                cnt = (short) CountMembers(p);
#  ifdef        BIO
                fwrite((char *) & cnt, sizeof(short), 1, tmplint);
#  else
                fprintf(tmplint, "%d\n", cnt);
#  endif        BIO
                OutMembers(p);
        }
# endif !CFLOW

}


/* -------------------- CountMembers -------------------- */

CountMembers(q) /* Count the number of struct/union/enum members. */
register struct symtab *q;
{
        register int    j;
        register short  c;

        /* Check for undefined symbol. */
        if ((j = dimtab[q->stype->typ_size+1]) < 0)
                return (0);
        for (c = 0; dimtab[j] >= 0; ++c, ++j)
                ;
        return (c);
}


/* -------------------- OutMembers -------------------- */

/*
** Write struct/union/enum members.
*/
OutMembers(q)
register struct symtab *q;
{
        register struct symtab *p;
        register int    j, m;
        TWORD bt;
        short   usage = 0;

        /* Check for undefined symbol. */
        if ((j = dimtab[q->stype->typ_size+1]) < 0)
                return;

        /* Write each member. */
        for (; (m = dimtab[j]) >= 0; ++j) {
                p = &stab[m];
# ifdef BIO
                fwrite(p->psname, strlen(p->psname) + 1, 1, tmplint);
# else
                fprintf(tmplint, "%s\n", p->psname);
# endif BIO
                OutType(p->stype);

                /* Get tag name from base TPTR. */
                if ((bt = BTYPE(p->stype)) == STRTY || bt == UNIONTY || bt == ENUMTY) {
                        TPTR t = p->stype;
                        while (!ISBTYPE(t))
                                t = DECREF(t);
                        if ((m = dimtab[t->typ_size+3]) >= 0)
                                usage |= LINTTAG;       /* symbol has a tag */
# ifdef BIO
                        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
                        fprintf(tmplint, "%d\n", usage);
# endif BIO
                        if (usage & LINTTAG)
# ifdef BIO
                                fwrite(stab[m].psname, strlen(stab[m].psname) + 1, 1, tmplint);
# else
                        fprintf(tmplint, "%s\n", stab[m].psname);
# endif BIO
                }
        }
}


/* -------------------- OutType -------------------- */

/*
** Write the type information.
*/
OutType(t)
register TPTR t;
{
        register PPTR p;

        do {
# ifdef BIO
                fwrite((char *) t, sizeof(struct tyinfo ), 1, tmplint);
# else
                fprintf(tmplint, "0%o 0%o 0%o\n", t->tword, t->next, t->typ_size);
# endif BIO
                if (ISFTN(t)) {
                        if ((p = t->ftn_parm) != PNIL) {
                                do {
# ifdef BIO
                                        fwrite((char *) p, sizeof(struct parminfo ), 1, tmplint);
# else
                                        fprintf(tmplint, "0%o 0%o\n", p->type, p->next);
# endif BIO
                                        OutType(p->type);
                                } while ((p = p->next) != PNIL);
                        }
                } else if (!ISARY(t) && !ISPTR(t))
                        return;
        } while (t = DECREF(t));
}



/* -------------------- strip -------------------- */

/*
** Strip the full name to get the basename.
*/
char    *
strip(s)
register char   *s;
{
        static char     buf[BUFSIZ+1];
        char    temp[BUFSIZ+1];
        register char   *p;

        strcpy (temp, s + 1);           /* remove first double quote */
        p = mbsrchr (temp, '"');        /* remove last double quote */
        if (p)
                *p = '\0';
        p = mbsrchr (temp, '/');        /* remove path */
        if (p)
                p++;
        else
                p = temp;

        if (strlen (p) > BUFSIZ)
                cerror(TOOLSTR(M_MSG_235, "filename too long"));

        strcpy (buf, p);
        return (buf);                   /* return file name */

}


/* -------------------- iscall -------------------- */

/*
** Return 1 if subtree represents a function call.
*/
iscall(p)
NODE *p;
{
        if (!p)
                return (0);

        switch (p->in.op) {
        case CALL:
        case STCALL:
        case UNARY CALL:
        case UNARY STCALL:
                if (p->in.left->in.op == UNARY AND)
                        return (1);
                break;
        default:
                if (optype(p->in.op) == BITYPE)
                        return (iscall(p->in.right));
                else if (optype(p->in.op) == UTYPE)
                        return (iscall(p->in.left));
        }
        return (0);
}


/* -------------------- OutFtnRef -------------------- */

/*
** Write type information for an old-style or prototyped
** function reference.
*/
OutFtnRef(p, style)
register NODE *p;
int     style;
{
        register TPTR t;
        register struct symtab *r;
        NODE * q;
        struct tyinfo ty;
        short   rline;
        char    iocode = 0;
        short   usage = 0;

        iocode = LINTSYM;
        if (!iscall(p))
                return;
        q = p;
        while ((p->in.op != NAME && p->in.op != LNAME && p->in.op != PNAME) && (p = p->in.left))
                ;
        if (p == PNIL || (p->in.op != NAME && p->in.op != LNAME && p->in.op != PNAME))
                cerror(TOOLSTR(M_MSG_236, "cannot complete function treewalk"));

        r = &stab[p->tn.rval];
        t = p->in.type;
        p = q;
# ifndef        CFLOW
        if (r->sclass == USTATIC || r->sclass == STATIC)
                return;
# endif !CFLOW
        rline = (r->suse < 0) ? -r->suse : r->suse;

        /* Determine usage. */
        usage |= r->sflags & SNSPACE;
        usage |= LINTREF;
        if (r->sclass == EXTERN)
                usage |= LINTDCL;

        r->ifname = ifname;
        r->line = lineno;

# ifdef IODEBUG
        if (iodebug) {
                printf("(R)%s <", r->psname);
                tprint(r->stype);
                printf("> 0%o %s\n", usage, (style) ? "fake" : "");
        }
# endif IODEBUG

# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite((char *) r->psname, strlen(r->psname) + 1, 1, tmplint);
        fwrite((char *) r->ifname, strlen(r->ifname) + 1, 1, tmplint);
        fwrite((char *) & r->line, sizeof(short), 1, tmplint);
        fwrite((char *) & rline, sizeof(short), 1, tmplint);
        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", r->psname);
        fprintf(tmplint, "%s\n", r->ifname);
        fprintf(tmplint, "%d\n", r->line);
        fprintf(tmplint, "%d\n", rline);
        fprintf(tmplint, "%d\n", usage);
# endif BIO

        /* Output ANSI prototype. */
        if (style) {
                OutType(r->stype);
                return;
        }

        /* Old-style function reference must simulate arguments. */
        ty.tword = t->tword;
        ty.next = t->next;
        ty.ftn_parm = (PPTR) ((optype(p->in.op) == BITYPE) ? 1 : 0);
# ifdef BIO
        fwrite((char *) & ty, sizeof(struct tyinfo ), 1, tmplint);
# else
        fprintf(tmplint, "0%o 0%o 0%o\n", ty.tword, ty.next, ty.ftn_parm);
# endif BIO

        /* Write function parameters. */
        if (optype(p->in.op) == BITYPE) {
                if (p->in.right->in.op == CM)
                        OutArguments(p->in.right);
                else {
                        /* Only one argument. */
                        struct parminfo p;
                        p.type = q->in.type;
                        p.next = (PPTR) 0;
# ifdef BIO
                        fwrite((char *) & p, sizeof(struct parminfo ), 1, tmplint);
# else
                        fprintf(tmplint, "0%o 0%o\n", p.type, p.next);
# endif BIO
                        /*GH 04/30/91 ix18586 lint was not handling correctly
                          function calls with ONE argument when there was no
                          prototype or definition visible */
                        OutArgType(q->in.right->in.type);
                }
        }

        /* Type of function itself. */
        t = DECREF(t);
        OutArgType(t);
}

/* -------------------- OutFtnAddrRef -------------------- */

/*
** Write type information for a function address reference.
*/

# ifdef LINT

OutFtnAddrRef(r)
register struct symtab *r;
{
        short   rline;
        char    iocode = 0;
        short   usage = 0;

        iocode = LINTSYM;

        if (r->sclass == USTATIC || r->sclass == STATIC)
                return;

        rline = (r->suse < 0) ? -r->suse : r->suse;

        /* Determine usage. */
        usage |= r->sflags & SNSPACE;
        usage |= LINTREF;
        usage |= LINTADDR;
        if (r->sclass == EXTERN)
                usage |= LINTDCL;

        r->ifname = ifname;
        r->line = lineno;

# ifdef IODEBUG
        if (iodebug) {
                printf("(R)%s <", r->psname);
                tprint(r->stype);
                printf("> 0%o\n", usage);
        }
# endif IODEBUG

# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite((char *) r->psname, strlen(r->psname) + 1, 1, tmplint);
        fwrite((char *) r->ifname, strlen(r->ifname) + 1, 1, tmplint);
        fwrite((char *) & r->line, sizeof(short), 1, tmplint);
        fwrite((char *) & rline, sizeof(short), 1, tmplint);
        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", r->psname);
        fprintf(tmplint, "%s\n", r->ifname);
        fprintf(tmplint, "%d\n", r->line);
        fprintf(tmplint, "%d\n", rline);
        fprintf(tmplint, "%d\n", usage);
# endif BIO

        OutType(r->stype);

}

# endif LINT



/* -------------------- OutFtnUsage -------------------- */

/*
** Output function usage information.  This ORed to previous
** symbol usage by lint2 - it concerns only function usage.
*/
OutFtnUsage(p, usage)
struct symtab *p;
short   usage;
{
        char    iocode = 0;

        iocode = LINTADD;
# ifndef        CFLOW
        if (p->sclass == USTATIC || p->sclass == STATIC)
                return;
# endif !CFLOW

# ifdef IODEBUG
        if (iodebug) {
                printf("(U)usage %s 0%o\n", p->psname, usage);
        }
# endif IODEBUG

# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite((char *) p->psname, strlen(p->psname) + 1, 1, tmplint);
        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", p->psname);
        fprintf(tmplint, "%d\n", usage);
# endif BIO
}


/* -------------------- OutArguments -------------------- */

/*
** Write function parameters.
** Make it look like a function prototype by introducing
** PPTR data structures with the last PPTR->next being NIL.
*/
OutArguments(q)
register NODE *q;
{
        struct parminfo p;
        static int      down = 0;               /* count #PPTRs */

        /* Find first function parameter, located at bottom-left. */
        if (q->in.op == CM && q->in.left->in.op == CM) {
                ++down;
                OutArguments(q->in.left);
                --down;
        } /* Parameter appearing on the left side. */ else if (q->in.op == CM && q->in.left->in.op != CM) {

                p.type = q->in.left->in.type;
                p.next = (PPTR) (down + 1);
# ifdef BIO
                fwrite((char *) & p, sizeof(struct parminfo ), 1, tmplint);
# else
                fprintf(tmplint, "0%o 0%o\n", p.type, p.next);
# endif BIO
                OutArgType(q->in.left->in.type);
        }

        /* Parameter appearing on the right side. */
        p.type = q->in.right->in.type;
        p.next = (PPTR) down;
# ifdef BIO
        fwrite((char *) & p, sizeof(struct parminfo ), 1, tmplint);
# else
        fprintf(tmplint, "0%o 0%o\n", p.type, p.next);
# endif BIO

        OutArgType(q->in.right->in.type);
}


/* -------------------- OutArgType -------------------- */

/*
** Write type information about a parameter.
** Don't elaborate if parameter is a prototype.
*/
OutArgType(t)
register TPTR t;
{
        struct tyinfo ty;

        if (t == TNIL)
                return;

        do {
                ty.tword = t->tword;
                ty.next = t->next;
                ty.typ_size = t->typ_size;

                /* Handle case where a function as a parameter may have a prototype. */
                if (ISFTN(t))
                        ty.ftn_parm = (PPTR) 0;
# ifdef BIO
                fwrite((char *) & ty, sizeof(struct tyinfo ), 1, tmplint);
# else
                fprintf(tmplint, "0%o 0%o 0%o\n", ty.tword, ty.next, ty.typ_size);
# endif BIO
                if (!ISARY(t) && !ISPTR(t) && !ISFTN(t))
                        return;
        } while (t = DECREF(t));
}


/* -------------------- OutFtnDef -------------------- */

/*
** Write type information for an old-style function definition.
** Make it look like a function prototype.
*/
OutFtnDef()
{
        register int    i, j;
        register TPTR t;
        struct tyinfo ty;
        struct parminfo p;
        struct symtab *r;
        short   rline;
        char    iocode = 0;
        short   usage = 0;

        iocode = LINTSYM;
        r = &stab[curftn];
        if (!ISFTN(r->stype))
                return;
# ifndef        CFLOW
        if (r->sclass == USTATIC || r->sclass == STATIC)
                return;
# endif !CFLOW

        r->line = lineno;
        r->ifname = ifname;

        /* Determine usage. */
        usage |= r->sflags & SNSPACE;
        usage |= LINTDEF;
        if ((rline = r->suse) < 0) {
                rline = -rline;
                usage |= LINTREF;
        }
        if (lintused)
                usage |= LINTNOT;       /* check for NOTUSED */
        if (lintdefd)
                usage |= LINTNDF;       /* check for NOTDEFINED */
        /* Check for varargs. */
        if (lintvarg != -1)
                usage |= LINTVRG;

        /*RW: P19915 */
        if (lintlib) {
                usage |= LINTLIB;
                usage |= LINTRET;
                usage |= LINTDEF;
        }

# ifdef IODEBUG
        if (iodebug) {
                printf("(D)%s <", r->psname);
                tprint(r->stype);
                printf("> 0%o %d/%d ret=%d %s\n", usage, r->line, rline, retstat,
                    (funcstyle != NEW_STYLE) ? "fake" : "");
        }
# endif IODEBUG

# ifdef BIO
        fwrite((char *) & iocode, sizeof(char), 1, tmplint);
        fwrite((char *) r->psname, strlen(r->psname) + 1, 1, tmplint);
        fwrite((char *) r->ifname, strlen(r->ifname) + 1, 1, tmplint);
        fwrite((char *) & r->line, sizeof(short), 1, tmplint);
        fwrite((char *) & rline, sizeof(short), 1, tmplint);
        fwrite((char *) & usage, sizeof(short), 1, tmplint);
# else
        fprintf(tmplint, "%d\n", iocode);
        fprintf(tmplint, "%s\n", r->psname);
        fprintf(tmplint, "%s\n", r->ifname);
        fprintf(tmplint, "%d\n", r->line);
        fprintf(tmplint, "%d\n", rline);
        fprintf(tmplint, "%d\n", usage);
# endif BIO

        /* Output ANSI prototype. */
        if (funcstyle == NEW_STYLE) {
                OutType(r->stype);
                return;
        }

        /* Old-style function definition must simulate arguments. */
        t = r->stype;
        ty.tword = t->tword;
        ty.next = t->next;
        ty.ftn_parm = (PPTR) ((paramno) ? 1 : 0);       /* simulate a prototype */
# ifdef BIO
        fwrite((char *) & ty, sizeof(struct tyinfo ), 1, tmplint);
# else
        fprintf(tmplint, "0%o 0%o 0%o\n", ty.tword, ty.next, ty.ftn_parm);
# endif BIO

        /*
        ** Write type information about each parameter. 
        ** Make it look like a function prototype by introducing
        ** PPTR data structures with the last PPTR->next being NIL.
        */
        for (i = 0; i < paramno; i++) {
                if ((j = paramstk[i]) < 0)
                        continue;
                p.type = stab[j].stype;

                /* Add ellipsis if varargs specified.  Although, at least one
                   arg is required for syntax, we will get away with treating
                   varargs0 as an argument list consisting of ellipsis only. */
                if (i == lintvarg) {
                        p.type = tyalloc(TELLIPSIS);
                        i = paramno - 1;
                }
                p.next = (PPTR) (paramno - i - 1);      /* count #PPTRs */
# ifdef BIO
                fwrite((char *) & p, sizeof(struct parminfo ), 1, tmplint);
# else
                fprintf(tmplint, "0%o 0%o\n", p.type, p.next);
# endif BIO
                OutArgType(p.type);     /* parameter type */
        }

        /* Type of the function itself. */
        t = DECREF(t);
        OutArgType(t);
}


#endif  LINT || CFLOW

/* -------------------- types -------------------- */

#define VCHAR 01
#define VSHORT 02
#define VINT 04
#define VLONG 010
#define VFLOAT 020
#define VDOUBLE 040
#define VSIGNED 0100
#define VUNSIGNED 0200
#define VVOID 0400
#define VLNGLNG 01000
#define VALSIZE (VLNGLNG|VVOID|VUNSIGNED|VSIGNED|VDOUBLE|VFLOAT|VLONG|VINT|VSHORT|VCHAR)
TWORD validType[VALSIZE+1];

struct tytest {
        TWORD type;
        int     mask;
};

/* LS: long long not ANSI */
static struct tytest typeANSI[] = {
        CHAR, VCHAR,
        SCHAR, VSIGNED | VCHAR,
        UCHAR, VUNSIGNED | VCHAR,
        INT, VINT,
        INT, VSIGNED,
        INT, VSIGNED | VINT,
        UNSIGNED, VUNSIGNED,
        UNSIGNED, VUNSIGNED | VINT,
        SHORT, VSHORT,
        SHORT, VSHORT | VINT,
        SHORT, VSIGNED | VSHORT,
        SHORT, VSIGNED | VSHORT | VINT,
        USHORT, VUNSIGNED | VSHORT,
        USHORT, VUNSIGNED | VSHORT | VINT,
        LONG, VLONG,
        LONG, VLONG | VINT,
        LONG, VSIGNED | VLONG,
        LONG, VSIGNED | VLONG | VINT,
        ULONG, VUNSIGNED | VLONG,
        ULONG, VUNSIGNED | VLONG | VINT,
        FLOAT, VFLOAT,
        DOUBLE, VDOUBLE,
        LDOUBLE, VLONG | VDOUBLE,
        TVOID, VVOID,
        0, 0
};


static struct tytest typeEXTD[] = {

#ifdef  CFLOW
        CHAR, VCHAR,
#else
        UCHAR, VCHAR,
#endif  CFLOW

        SCHAR, VSIGNED | VCHAR,
        UCHAR, VUNSIGNED | VCHAR,
        INT, VINT,
        INT, VSIGNED,
        INT, VSIGNED | VINT,
        UNSIGNED, VUNSIGNED,
        UNSIGNED, VUNSIGNED | VINT,
        SHORT, VSHORT,
        SHORT, VSHORT | VINT,
        SHORT, VSIGNED | VSHORT,
        SHORT, VSIGNED | VSHORT | VINT,
        USHORT, VUNSIGNED | VSHORT,
        USHORT, VUNSIGNED | VSHORT | VINT,
        LONG, VLONG,
        LONG, VLONG | VINT,
        LONG, VSIGNED | VLONG,
        LONG, VSIGNED | VLONG | VINT,
        ULONG, VUNSIGNED | VLONG,
        ULONG, VUNSIGNED | VLONG | VINT,
        LNGLNG, VLNGLNG,
        LNGLNG, VLNGLNG | VINT,
        LNGLNG, VSIGNED | VLNGLNG,
        LNGLNG, VSIGNED | VLNGLNG | VINT,
        ULNGLNG, VUNSIGNED | VLNGLNG,
        ULNGLNG, VUNSIGNED | VLNGLNG | VINT,
        FLOAT, VFLOAT,
        DOUBLE, VDOUBLE,
        DOUBLE, VLONG | VFLOAT,
        LDOUBLE, VLONG | VDOUBLE,
        TVOID, VVOID,
        0, 0
};


static struct tytest typeMask[] = {
        CHAR, VCHAR,
        SCHAR, VSIGNED | VCHAR,
        UCHAR, VCHAR,           /* Since this is allowed in EXTD mode only */
        SHORT, VSHORT,
        USHORT, VUNSIGNED | VSHORT,
        INT, VINT,
        UNSIGNED, VUNSIGNED,
        LONG, VLONG,
        ULONG, VUNSIGNED | VLONG,
        LNGLNG, VLNGLNG,
        ULNGLNG, VUNSIGNED | VLNGLNG,
        FLOAT, VFLOAT,
        DOUBLE, VDOUBLE,
        LDOUBLE, VLONG | VDOUBLE,
        SIGNED, VSIGNED,
        TVOID, VVOID,
        0, 0
};


int     lookupMask[NBTYPES];

#define VENUM 01
#define VSTRUCT 02
#define VTYDEF 04

#define PARSEDEPTH 15
struct {
        int     curTypeMask;    /* bit pattern of type */
        TWORD curQualifier;     /* type qualifier */
        int     otherType;              /* other type flag */
} typeStack[PARSEDEPTH];


/* -------------------- InitParse -------------------- */

/*
** Initialize the type checking arrays.  Either ANSI or extended C mode.
*/
InitParse()
{
        register struct tytest *q;

        /* Initialize type validity and mask arrays. */
        q = (devdebug[TYPING]) ? typeANSI : typeEXTD;
        for ( ; q->type; ++q)
#ifdef  COMPAT
                if ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) {
                        if ( q->type == LONG )
                                validType[q->mask] = INT;
                        else if ( q->type == ULONG )
                                validType[q->mask] = UNSIGNED;
                        else
                                validType[q->mask] = q->type;
                }
                else {
                        validType[q->mask] = q->type;
                }
#else
        validType[q->mask] = q->type;
#endif  COMPAT
        for (q = typeMask; q->type; ++q)
                lookupMask[q->type] = q->mask;
}


/* -------------------- CheckTypedef -------------------- */

/*
** Running accumulation of a typedef.
*/
TPTR
CheckTypedef(type)
TPTR type;
{
        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        /* Check for type clash. */
        if (typeStack[curLevel].curTypeMask)
                /* "basic type cannot mix with struct/union/enum/typedef" */
                UERROR( ALWAYS, MESSAGE(134) );
        else if (typeStack[curLevel].otherType)
                /* "illegal type specifier combination" */
                UERROR( ALWAYS, MESSAGE(70) );
        typeStack[curLevel].otherType |= VTYDEF;
        return(type);
}


/* -------------------- CheckEnum -------------------- */

/*
** Running accumulation of an enum.
*/
CheckEnum()
{

        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        /* Check for type clash. */
        if (typeStack[curLevel].curTypeMask)
                /* "basic type cannot mix with struct/union/enum/typedef" */
                UERROR( ALWAYS, MESSAGE(134) );
        else if (typeStack[curLevel].otherType)
                /* "illegal type specifier combination" */
                UERROR( ALWAYS, MESSAGE(70) );
        typeStack[curLevel].otherType |= VENUM;
}


/* -------------------- CheckStruct -------------------- */

/*
** Running accumulation of a struct.
*/
CheckStruct()
{

        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        /* Check for type clash. */
        if (typeStack[curLevel].curTypeMask)
                /* "basic type cannot mix with struct/union/enum/typedef" */
                UERROR( ALWAYS, MESSAGE(134) );
        else if (typeStack[curLevel].otherType)
                /* "illegal type specifier combination" */
                UERROR( ALWAYS, MESSAGE(70) );
        typeStack[curLevel].otherType |= VSTRUCT;
}


/* -------------------- CheckQualifier -------------------- */

/*
** Running accumulation of type qualifier.
*/
CheckQualifier(qualifier)
TWORD qualifier;
{
        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        /* Check for duplicate type qualifier. */
        if (typeStack[curLevel].curQualifier & qualifier)
                /* "illegal type qualifier combination" */
                WERROR( ALWAYS, MESSAGE(131) );
        typeStack[curLevel].curQualifier |= qualifier;
}


/* -------------------- CheckType -------------------- */

/*
** Running accumulation of type specifier.
*/
TPTR
CheckType(type)
TPTR type;
{
        int     omask, tmask;

        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
            cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        /* Check for type clash. */
        if (typeStack[curLevel].otherType)
            /* "basic type cannot mix with struct/union/enum/typedef" */
            UERROR( ALWAYS, MESSAGE(134) );

        /* Check for type specifier duplication. */
        omask = typeStack[curLevel].curTypeMask;
        tmask = lookupMask[BTYPE(type)];
        tmask |= omask;
        if (tmask == omask) {
	    /* LS: special case for long long, where long appears twice. */
	    if ((tmask & VLONG) && (typeStack[curLevel].curTypeMask & VLONG)) {
	        tmask &= ~VLONG;
	        tmask |= VLNGLNG;
                if (!devdebug[ANSI_MODE])
                    /* "non ansi type specifier long long" */
                    WARNING(WANSI, MESSAGE(104));
	    }
	    else
                /* "illegal type specifier combination" */
                UERROR( ALWAYS, MESSAGE(70) );
	}

        /* Check for valid type combination, assume INT. */
        if (!validType[tmask]) {
            /* "illegal type specifier combination" */
            UERROR( ALWAYS, MESSAGE(70) );
            tmask = VINT;
        }

        typeStack[curLevel].curTypeMask = tmask;

        return (tyalloc(validType[tmask]));
}


/* -------------------- ResultType -------------------- */

/*
** Return final type with qualifiers and other flags.
*/
TPTR
ResultType(type)
TPTR type;
{
        TWORD qual;
        unsigned        size;
        TPTR temp;

        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        qual = typeStack[curLevel].curQualifier;

        if (qual) {
                /* Check if the type is already so qualified (for typedefs) */
                if (ISQUALIFIED(type, qual)) {
                        /* "illegal type qualifier combination" */
                        WERROR( ALWAYS, MESSAGE(131) );
                        qual &= ~QUALIFIERS(type);
                        if (qual) {
                                type = qualtype(type, qual, 1);
                        }
                } else if (ISFTN(type)) {
                        /* "function returns qualified type" */
                        WERROR( ALWAYS, MESSAGE(132) );
                } else if (ISARY(type)) {
                        size = type->ary_size;
                        type = DECREF(type);
                        /* GH 18/10/90 P49001 assignment of a "pointer to
                           a constant multi-dim. array" to a "pointer to
                           a volatile" (typedef was used for array) was
                           not reported as a problem */
                        temp = type;
                        while (temp->next != TNIL) {
                                temp = DECREF(temp);
                        }
                        if (ISQUALIFIED(type, qual)) {
                                /* "illegal type qualifier combination" */
                                WERROR( ALWAYS, MESSAGE(131) );
                                qual &= ~QUALIFIERS(type);
                                if (qual) {
                                        type = qualtype(type, qual, 1);
                                }
                        } else {
                                if (type != temp) {
                                        temp = qualtype(temp, qual, 1);
                                        type->next = temp;
                                } else
                                        type = qualtype(type, qual, 1);
                        }
                        type = INCREF(type, ARY);
                        type->typ_size = size;
                } else {
                        type = qualtype(type, qual, 1);
                }
        }

        if (typeStack[curLevel].curTypeMask & VSIGNED) {
                type = signedtype(type);
        }
        typeStack[curLevel].curTypeMask = 0;
        typeStack[curLevel].curQualifier = 0;
        typeStack[curLevel].otherType = 0;

        return (type);
}


/* -------------------- SeenType -------------------- */

SeenType()
{
        return (typeStack[curLevel].curTypeMask || typeStack[curLevel].otherType);
}


/* -------------------- DiagnoseType -------------------- */

DiagnoseType()
{
        register int    i;

        printf("curLevel = %d\n", curLevel);
        printf("curTypeMask ");
        for (i = 0; i < PARSEDEPTH; i++)
                printf("%o ", typeStack[i].curTypeMask);
        printf("\n");
        printf("curQualifier ");
        for (i = 0; i < PARSEDEPTH; i++)
                printf("%o ", typeStack[i].curQualifier);
        printf("\n");
        printf("otherType ");
        for (i = 0; i < PARSEDEPTH; i++)
                printf("%o ", typeStack[i].otherType);
        printf("\n");
}


/* -------------------- WriteType -------------------- */

WriteType(s)
char    *s;
{
        /* check for parse depth */
        if (curLevel > PARSEDEPTH)
                cerror(TOOLSTR(M_MSG_237, "Type Stack overflow"));

        fprintf(stderr, TOOLSTR(M_MSG_285, "%s type:%o qual:%o other:%o\n"),
            s, typeStack[curLevel].curTypeMask, typeStack[curLevel].curQualifier,
            typeStack[curLevel].otherType);
}


/* -------------------- tymerge -------------------- */

NODE *
tymerge( typ, idp )/* merge type typ with identifier idp  */
NODE *typ, *idp;
{

        register TPTR t, t2;
        TWORD ty;
        register i;
        extern int      eprint();

        if ( typ->in.op != TYPE )
                cerror(TOOLSTR(M_MSG_238, "tymerge: arg 1" ));
        if (idp == NIL )
                return( NIL );

#ifndef BUG1
        if ( ddebug > 2 )
                fwalk( idp, eprint, 0 );
#endif  !BUG1

        idp->in.type = typ->in.type;
        RecurseCnt = 0;
        tyreduce( idp );

        return( idp );
}


/* -------------------- tyreduce -------------------- */

tyreduce( p )
register NODE *p;
{

        /* build a type, and stash away dimensions, from a parse */
        /* tree of the declaration the type is build top down, */
        /* the dimensions bottom up */
        register TPTR t;
        register o;
        TWORD qual;

        o = p->in.op;
        p->in.op = FREE;

        if ( o == NAME )
                return;

        t = p->in.type;

        if ( o == UNARY CALL ) {
                if ( TOPTYPE(t) == ARY || TOPTYPE(t) == FTN ) {
                        /* "function returns illegal type" */
                        UERROR( ALWAYS, MESSAGE(47) );
                        t = INCREF(t, PTR);
                } else if ( QUALIFIERS(t) ) {
                        /* "function returns qualified type" */
                        WERROR( ALWAYS, MESSAGE(132) );
                        t = unqualtype(t);
                }
                t = INCREF(t, FTN);
                /* reattach the parameter list pointer to
                 * the function node in the type tree and remove
                 * it from the return type of the function node.
                 */
                if (p->in.right != NIL) {
                        t->ftn_parm = p->in.right->in.type->ftn_parm;
                        p->in.right->in.op = FREE;
                }

        } else if ( o == LB ) {
                if ( TOPTYPE(t) == FTN ) {
                        /* "array of functions is illegal" */
                        UERROR( ALWAYS, MESSAGE(14) );
                        t = INCREF(t, PTR);
                }
                if (RecurseCnt++ > 13)
                        UERROR( ALWAYS, TOOLSTR(M_MSG_321, "too many dimensions, maximum is 13") );
                t = INCREF(t, ARY);
                t->ary_size = p->in.right->tn.lval;
                p->in.right->in.op = FREE;
                if ( ( t->ary_size == 0 ) & ( p->in.left->tn.op == LB ) )
                        /* "null dimension" */
                        WERROR( ALWAYS, MESSAGE(85) );

        } else { /* o == UNARY MUL */
                t = INCREF(t, PTR);
                if ( ( qual = QUALIFIERS(p->in.right->in.type) ) != 0 )
                        t = qualtype(t, qual, 0);
                p->in.right->in.op = FREE;
        }

        p->in.left->in.type = t;
        tyreduce( p->in.left );

        p->tn.rval = p->in.left->tn.rval;
        p->in.type = p->in.left->in.type;

}

/* -------------------- mknonuniq -------------------- */

struct symtab *
mknonuniq(idindex)
int     *idindex;
{
        /* locate a symbol table entry for */
        /* an occurrence of a nonunique structure member name */
        /* or field */
        register i;
        register struct symtab *sp;
        register char   *q;

        sp = &stab[ i= *idindex ]; /* position search at old entry */
        while (TOPTYPE(sp->stype) != TNULL) { /* locate unused entry */
                if ( ++i >= nstabents ) {/* wrap around symbol table */
                        i = 0;
                        sp = stab;
                } else
                        ++sp;

                if ( i == *idindex )
                        cerror(TOOLSTR(M_MSG_240, "Symbol table full"));
        }

        sp->sflags = SNONUNIQ | SMOS;
        q = stab[*idindex].psname; /* old entry name */
        sp->psname = getmem( strlen(q) + 1);
        strcpy(sp->psname, q ); /* old entry name */
#ifndef BUG1
        if ( ddebug )
                printf("\tnonunique entry for %s from %d to %d\n", q, *idindex, i );
#endif  !BUG1

        *idindex = i;
        return ( sp );
}


/* -------------------- lookup -------------------- */

lookup( name, s)/* look up name: must agree with s w.r.t. SNSPACE, SHIDDEN and SSCOPED */
char    *name;
int     s;
{

        register char   *p, *q;
        register struct symtab *sp;
        register int    i, ii;

        /* compute initial hash index */
        i = 0;
        for ( p = name; *p ; ++p ) {
                i = (i << 1) + *p;
        }
        if (i < 0)
                i = -i; /* longnames can produce negative hash # */
        i = i % nstabents;
        sp = &stab[ii=i];

#ifndef BUG1
    if ( ddebug > 2 )
        printf("lookup(%s, %d), stwart=%d, instruct=%d\n",
	       name, s, stwart, instruct);
#endif  !BUG1

        do { /* look for name */

                if (TOPTYPE(sp->stype) == TNULL) { /* empty slot */
                        sp->sflags = s;
                        sp->stype = tyalloc(UNDEF);
                        sp->sclass = SNULL;
                        sp->slevel = blevel;
                        if (!(s & (SHIDDEN | SSCOPED))) {
#if defined (LINT) || defined (CFLOW)
                                sp->ifname = ifname;
#endif LINT || CFLOW
                                sp->psname = getmem(strlen(name) + 1);
                                strcpy(sp->psname, name);
                        }
                        return(i);
                }

                if (!strcmp(sp->psname, name) &&
                    (sp->sflags & (SNSPACE | SHIDDEN | SSCOPED)) == s)
                        /* GH 01/21/91 fix to A16691 */
                         return(i);

                if ( ++i >= nstabents ) {
                        i = 0;
                        sp = stab;
                } else
                        ++sp;
        } while (i != ii);
    cerror(TOOLSTR(M_MSG_240, "symbol table full" ));
}


/* -------------------- fixlab -------------------- */

fixlab( id )
int     id;
{
        /* Fix symbol `id' to be in label namespace */
        register struct symtab *sp;

        sp = &stab[id];
        if ( TOPTYPE(sp->stype) == UNDEF ) {
                /* Undo previous lookup */
                sp->stype = tyalloc(TNULL);
        }
        return( lookup(sp->psname, SLABEL) );
}


/* -------------------- checkst -------------------- */

#ifndef checkst
/* if not debugging, make checkst a macro */
checkst(lev)
int     lev;
{
        register int    s, i, j;
        register struct symtab *p, *q;

        for ( i = 0, p = stab; i < nstabents; ++i, ++p ) {
                if (TOPTYPE(p->stype) == TNULL)
                        continue;
                j = lookup( p->psname, p->sflags & SNSPACE );
                if ( j != i ) {
                        q = &stab[j];
                        if (TOPTYPE(q->stype) == UNDEF || q->slevel <= p->slevel ) {
                                cerror(TOOLSTR(M_MSG_241, "check error: %s"), q->psname );
                        }
                } else if ( p->slevel > lev )
                        cerror(TOOLSTR(M_MSG_242, "%s check at level %d"), p->psname, lev );
        }
}


#endif  !checksh


/* -------------------- relook -------------------- */

struct symtab *
relook(p) /* look up p again, and see where it lies */
register struct symtab *p;
{

        register struct symtab *q;

        /* I'm not sure that this handles towers of several hidden */
        /* definitions in all cases */

        q = &stab[lookup( p->psname, p->sflags&(SNSPACE|SHIDDEN|SSCOPED) )];
        /* make relook always point to either p or an empty cell */

        if (TOPTYPE(q->stype) == UNDEF
#ifdef  IS_COMPILER
             && func_ptr != 2 && p != q
#endif  IS_COMPILER
            ) {
                if ( q->psname ) {
                        free( q->psname );
                        q->psname = 0;
                }
                q->stype = tyalloc(TNULL);
                return( q );
        }

        while ( q != p ) {
                if (TOPTYPE(q->stype) == TNULL)
                        break;
                if ( ++q >= &stab[nstabents] )
                        q = stab;
        }
        return( q );
}


/* -------------------- clearst -------------------- */

clearst()/* clear entries of internal scope  from the symbol table */
{
        register struct symtab *p, *q, *r;
        register int    temp, rehash;
        int     gid;

        temp = lineno;
        aobeg();

        /* first, find an empty slot to prevent newly hashed entries from being slopped into... */

        for ( q = stab; q < &stab[nstabents]; ++q ) {
                if (TOPTYPE(q->stype) == TNULL)
                        goto search;
        }

        cerror(TOOLSTR(M_MSG_240, "symbol table full" ));

search:
        p = q;

        for (; ; ) {
                if (TOPTYPE(p->stype) == TNULL) {
                        rehash = 0;
                        goto next;
                }

                lineno = p->suse;
                if ( lineno < 0 )
                        lineno = -lineno;

                if ( p->slevel > blevel ) { /* must clobber */
                        if (TOPTYPE(p->stype) == UNDEF
                             || ( p->sclass == ULABEL && blevel < 2 ) ) {
                                lineno = temp;
                                /* "%s undefined" */
                                WARNING( ALWAYS, MESSAGE(4), p->psname );
                        } else
                                aocode(p);
#ifndef BUG1
                        if (ddebug)
                                printf("removing %s from stab[ %d], flags %o level %d\n",
                                    p->psname, p - stab, p->sflags, p->slevel);
#endif  !BUG1
                        if ( p->sflags & SHIDES ) {
                                if ( ( p->sflags & SEXTRN ) && ( gid = extrndec( p ) ) >= 0 ) {
                                        /* Copy over usage info */
                                        r = &stab[gid];
                                        if ( p->suse < 0 )
                                                r->suse = p->suse;
#ifdef  XCOFF
                                        r->sflags |= p->sflags & (SSET | SREF | SFCALLED | SFADDR);
#else
                                        r->sflags |= p->sflags & (SSET | SREF);
#endif  XCOFF
                                        p->sflags &= ~SEXTRN;
                                }
                                unhide(p);
                        }

                        if ( ( p->sflags & (SEXTRN | SSCOPED) ) == SEXTRN && p->suse < 0 ) {
                                r = &stab[ lookup( p->psname, ( p->sflags & SNSPACE ) | SSCOPED ) ];
                                if ( TOPTYPE(r->stype) == UNDEF ) {
                                        r->stype = tyalloc(TNULL);
                                        p->sflags |= SSCOPED;
                                        p->slevel = 0;
                                        dimptr->cextern = 1;
                                        p->stype = copytype(p->stype, 0);
                                        goto chkhash;
                                } else if ( ISFTN(p->stype) == ISFTN(r->stype) ) {
                                        if ( !comtypes( p->stype, r->stype, 0 ) )
                                                /* "external symbol type clash for %s" */
                                                WARNING( WDECLAR, MESSAGE(193), p->psname );
                                        r->suse = p->suse;
#ifdef  XCOFF
                                        r->sflags |= p->sflags & (SSET | SREF | SFCALLED | SFADDR);
#else
                                        r->sflags |= p->sflags & (SSET | SREF);
#endif  XCOFF
                                } else {
                                        /* "external symbol type clash for %s" */
                                        UERROR( ALWAYS, MESSAGE(193), p->psname );
                                }
                        }

                        /*                      free(p->psname);   not yet maybe when it works sal */
                        p->psname = 0;
                        p->stype = tyalloc(TNULL);
                        rehash = 1;
                        goto next;
                }
chkhash:
                if ( rehash ) {
                        if ( (r = relook(p)) != p ) {
                                *r = *p;
                                p->psname = 0;
                                p->stype = tyalloc(TNULL);
                        }
                }
next:
                if ( ++p >= &stab[nstabents] )
                        p = stab;
                if ( p == q )
                        break;
        }
        lineno = temp;
        aoend();
}


/* -------------------- hide -------------------- */

hide( p, extcpy )
register struct symtab *p;
int     extcpy;
{
        register struct symtab *q;

        for ( q = p + 1; ; ++q ) {
                if ( q >= &stab[nstabents] )
                        q = stab;
                if ( q == p )
                        cerror(TOOLSTR(M_MSG_240, "symbol table full" ));
                if (TOPTYPE(q->stype) == TNULL)
                        break;
        }

        *q = *p;
        p->sflags |= SHIDDEN;
        q->sflags = ( p->sflags & SNSPACE ) | SHIDES;
        if ( !extcpy && blevel != 1 && paramlevel == 0 )
                /* "%s redefinition hides earlier one" */
                WARNING( (WDECLAR || WHEURISTIC) && WKNR, MESSAGE(2), p->psname );
#ifndef BUG1
        if ( ddebug )
                printf( "\t%d hidden in %d\n", p - stab, q - stab );
#endif  !BUG1
        return( idname = q - stab );
}


/* -------------------- extrndec -------------------- */

extrndec( p )
register struct symtab *p;
{
        register struct symtab *q;
        register int    s;

        /*
         * for an extern decalaration in a block,
         * the identifier referenced is the one declared
         * at file scope. we attempt to find it.
         *
         * we call this function when we have an "extern"
         * declaration of an identifier at block level (blevel >1).
         */

        s = p->sflags & SNSPACE;
        q = p;

        for (; ; ) {
                if ( q == stab )
                        q = &stab[nstabents-1];
                else
                        --q;

                if ( q == p ) {
                        if ( ddebug )
                                printf( "extrndec of %d not found\n", p - stab );
                        return( -1 );
                }

                if ( TOPTYPE(q->stype) != TNULL && ( q->sflags & SNSPACE ) == s && 
                    strcmp(p->psname, q->psname) == 0 ) {
                        /* found the name */
#ifndef BUG1
                        if ( ddebug )
                                printf("extrndec of: %d might be %d\n", p - stab, q - stab);
#endif  !BUG1
                        /*
                                 * if the declaration found is not
                                 * at file scope, then it is not
                                 * the one we want.
                                 */
                        switch ( q->sclass ) {
                        case EXTERN:
                        case EXTENT:
                        case EXTDEF:
                        case USTATIC:
                        case FORTRAN:
                        case UFORTRAN:
                                break;
                        case STATIC:
                                if ( q->sflags & SEXTRN )
                                        /* global symbol */
                                        break;
                        default:
                                continue;
                        }

                        /* Found it! */
#ifndef BUG1
                        if ( ddebug )
                                printf( "extrndec of: %d is %d\n", p - stab, q - stab);
#endif  !BUG1
                        return( q - stab );
                }
        }
}


/* -------------------- unhide -------------------- */

unhide( p )
register struct symtab *p;
{
        register struct symtab *q;
        register int    s;

        s = p->sflags & SNSPACE;
        q = p;

        for (; ; ) {

                if ( q == stab )
                        q = &stab[nstabents-1];
                else
                        --q;

                if ( q == p )
                        break;

                if ( ( q->sflags & (SNSPACE | SSCOPED) ) == s ) {
                        if (!strcmp(p->psname, q->psname)) { /* found the name */
                                q->sflags &= ~SHIDDEN;
#ifndef BUG1
                                if ( ddebug )
                                        printf("unhide uncovered %d from %d\n", q - stab, p - stab);
#endif  !BUG1
                                return;
                        }
                }
        }
        cerror(TOOLSTR(M_MSG_286, "unhide fails" ));
}


/* -------------------- attachProto -------------------- */

AttachProto(p)
NODE *p;
{
        /* build up a prototype list and attach it to
         * the right node of the node p.  This will be
         * checked by tyreduce to attach the parameter list
         * to the FTN type node when it is built.
         */

        NODE * new;
        PPTR CreateProto();
        register int    i;

        /* since this node is only a place holder for
         * the parameter list, it doesn't matter if
         * it's wrong.
         */
        i = protopop();
        new = p->in.right = mkty( INCREF(tyalloc(INT), FTN) );
        paramlevel--;
        blevel--;
        new->in.type->ftn_parm = CreateProto(i);
        return( i );
}


/* -------------------- CreateProto -------------------- */

PPTR
CreateProto(firstparam)
int     firstparam;
{
        /* create a parameter list structure from the
         * parameters on the paramstk from firstparam
         * to the top of the stack (paramno)
         */

        PPTR origp;
        register PPTR *p = &origp;
        register PPTR pNew;
        int     i;

        /* pick up each identifier from parameter stack and
         * add its type to the parameter list.
         */

        for (i = firstparam; i < paramno ; i++) {
                *p = pNew = parmalloc();
                pNew->type = unqualtype( stab[paramstk[i]].stype );
                p = &pNew->next;
        }
        *p = PNIL;

        return ( origp );
}


int     prototop = 0;


/* -------------------- protopush -------------------- */

protopush( i )
int     i;
{
        if ( prototop >= protosz + 3 ) { /* then make protostk big enough */
                protosz = prototop + 20;
                protostk = (int *)realloc(protostk, protosz * sizeof(int));
                if ( protostk == NULL )
                        cerror(TOOLSTR(M_MSG_243, "prototype stack overflow"));
        }
        protostk[ prototop++ ] = i;
        protostk[ prototop++ ] = curclass;
        protostk[ prototop++ ] = paramFlg;
        protostk[ prototop++ ] = instruct;
        instruct = 0;
}


/* -------------------- protopop -------------------- */

protopop()
{
        if ( prototop < 4 )
                cerror(TOOLSTR(M_MSG_244, "protopop: dropped below stack"));
        instruct = protostk[ --prototop ];
        paramFlg = protostk[ --prototop ];
        curclass = protostk[ --prototop ];
        return ( protostk[ --prototop ] );
}


#define FAKENAME "%%FAKE%d"

/* -------------------- getFakeName -------------------- */

char    *
getFakeName()
{
        static int      counter = 0;
        char    *s, *FakeNamealloc();

        s = FakeNamealloc();
        sprintf(s, FAKENAME, ++counter);
        return( s );
}


/* -------------------- FakeNamealloc -------------------- */

char    *
FakeNamealloc()
{
        /* use the generic alloc routine to get blocks of memory
         * to store the generated fakenames in.  The maximum fakename
         * that can be stored without causing problems is
         * "%FAKE99999999".
         */
        static int      fkbunchsize = 0;
        static char     *fakebunch;

        if (fkbunchsize == 0) {
                /* Allocate another bunch of type nodes */
                fakebunch = (char *)getmem(MAXBUNCH * (sizeof(FAKENAME) + 5));
                fkbunchsize = MAXBUNCH;
        }
        return (&fakebunch[--fkbunchsize * (sizeof(FAKENAME)+5)]);
}


/* -------------------- makeghost -------------------- */

makeghost(realid)
int     realid;
{
        int     ghostid;
        char    *ghostname;

        /*
         * initialize the "ghost" of the
         * identifier.
         * the ghost identifier has:
         * 1. a derivative name of the real id.
         * 2. static declaration local to the
         * block.
         *
         * first: allocate enough space for the
         * realname, plus two % and a null
         * then create new stab entry with a statictorage class
         * and the same type as the real id.
         * EXCEPTION: static automatic char arrays. for these
         * the real id is the ghost id.
         *
         * we assume this function is called for automatic variables
         */

        if (curclass == STATIC)
                return(realid);

        ghostname = (char *) malloc(strlen(stab[realid].psname) + 3);
        if (ghostname == NULL)
                cerror(TOOLSTR(M_MSG_245, "memory allocation problem for a ghost identifier"));

        strcpy(ghostname, "%%");
        strcat(ghostname, stab[realid].psname);
        ghostid = lookup(ghostname, 0);
        stab[ghostid].uniqid = LocalUniqid++;
        stab[ghostid].stype = copytype(stab[realid].stype, blevel);
        stab[ghostid].slevel = blevel;
        stab[ghostid].sclass = STATIC;
        stab[ghostid].offset = getlab();
        stab[ghostid].suse   = -lineno;

#ifndef XCOFF
        StabInfoPrint(&stab[ghostid]);
#endif  !XCOFF

        return(ghostid);
}


