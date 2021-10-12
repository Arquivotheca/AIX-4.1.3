static char sccsid[] = "@(#)55  1.9.2.1  src/bos/usr/ccs/bin/common/tftn.c, cmdprog, bos411, 9428A410j 4/29/93 08:12:29";
/*
 * COMPONENT_NAME: (CMDPROG) tftn.c
 *
 * FUNCTIONS: FindBType, InitTypes, btype, comtypes, copyparms, copytype     
 *            defaultproto, incref, mkcomposite, modtype, newty, parmalloc    
 *            qualmember, qualtype, sameproto, setty, signedtype, tyencode    
 *            tynalloc, unqualtype                                            
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
# include <assert.h>
# include "mfile1.h"
# include "messages.h"

#define NUMBTYPES       (NRBTYPES+1)

#define NTABTYPES       ((NBTYPES + 1) + ((NRBTYPES + 1) * 3))


struct tyinfo btypes[NTABTYPES];
#if 0
        { TNULL, TNIL },
        { TELLIPSIS, TNIL },
        { FARG, TNIL },
        { MOETY, TNIL },
        { SIGNED, TNIL },
        { UNDEF, TNIL },

        /*
        ** "Real" basic types start here.
        */

        { TVOID, TNIL },
        { CHAR, TNIL },
        { SCHAR, TNIL },
        { SHORT, TNIL },
        { INT, TNIL },
        { LONG, TNIL },
        { FLOAT, TNIL },
        { DOUBLE, TNIL },
        { LDOUBLE, TNIL },
        { STRTY, TNIL },
        { UNIONTY, TNIL },
        { ENUMTY, TNIL },
        { UCHAR, TNIL },
        { USHORT, TNIL },
        { UNSIGNED, TNIL },
        { ULONG, TNIL },
        { CPPCLASS, TNIL },
        { TSIGNED|INT, TNIL },                  /* Fake type */
        { CONST|TVOID, TNIL },
        { CONST|CHAR, TNIL },
        { CONST|SCHAR, TNIL },
        { CONST|SHORT, TNIL },
        { CONST|INT, TNIL },
        { CONST|LONG, TNIL },
        { CONST|FLOAT, TNIL },
        { CONST|DOUBLE, TNIL },
        { CONST|LDOUBLE, TNIL },
        { CONST|STRTY, TNIL },
        { CONST|UNIONTY, TNIL },
        { CONST|ENUMTY, TNIL },
        { CONST|UCHAR, TNIL },
        { CONST|USHORT, TNIL },
        { CONST|UNSIGNED, TNIL },
        { CONST|ULONG, TNIL },
        { CONST|CPPCLASS, TNIL },
        { CONST|TSIGNED|INT, TNIL },            /* Fake type */
        { VOLATILE|TVOID, TNIL },
        { VOLATILE|CHAR, TNIL },
        { VOLATILE|SCHAR, TNIL },
        { VOLATILE|SHORT, TNIL },
        { VOLATILE|INT, TNIL },
        { VOLATILE|LONG, TNIL },
        { VOLATILE|FLOAT, TNIL },
        { VOLATILE|DOUBLE, TNIL },
        { VOLATILE|LDOUBLE, TNIL },
        { VOLATILE|STRTY, TNIL },
        { VOLATILE|UNIONTY, TNIL },
        { VOLATILE|ENUMTY, TNIL },
        { VOLATILE|UCHAR, TNIL },
        { VOLATILE|USHORT, TNIL },
        { VOLATILE|UNSIGNED, TNIL },
        { VOLATILE|ULONG, TNIL },
        { VOLATILE|CPPCLASS, TNIL },
        { VOLATILE|TSIGNED|INT, TNIL },         /* Fake type */
        { CONST|VOLATILE|TVOID, TNIL },
        { CONST|VOLATILE|CHAR, TNIL },
        { CONST|VOLATILE|SCHAR, TNIL },
        { CONST|VOLATILE|SHORT, TNIL },
        { CONST|VOLATILE|INT, TNIL },
        { CONST|VOLATILE|LONG, TNIL },
        { CONST|VOLATILE|FLOAT, TNIL },
        { CONST|VOLATILE|DOUBLE, TNIL },
        { CONST|VOLATILE|LDOUBLE, TNIL },
        { CONST|VOLATILE|STRTY, TNIL },
        { CONST|VOLATILE|UNIONTY, TNIL },
        { CONST|VOLATILE|ENUMTY, TNIL },
        { CONST|VOLATILE|UCHAR, TNIL },
        { CONST|VOLATILE|USHORT, TNIL },
        { CONST|VOLATILE|UNSIGNED, TNIL },
        { CONST|VOLATILE|ULONG, TNIL },
        { CONST|VOLATILE|CPPCLASS, TNIL },
        { CONST|VOLATILE|TSIGNED|INT, TNIL },   /* Fake type */
};
#endif

static TPTR curGlobTy;  /* Next global type to be allocated */
static TPTR curLoclTy;  /* Next local type to be allocated */

InitTypes () {
    TWORD i = TNULL, q;
    struct tyinfo *p;

    curGlobTy = globTyTab = (TPTR)getmem(nGlobTyEnts * sizeof(struct tyinfo));
    curLoclTy = loclTyTab = (TPTR)getmem(nLoclTyEnts * sizeof(struct tyinfo));

    /* the following loops load in a table with all type/qualifier combinations
       for all qualifiers, loop over all base types and qualify them. */
    for (p = btypes, q = NOQUAL; q <= ALLQUAL; q += INCQUAL, i = TVOID, ++p) {
        for (; i < NBTYPES; ++i, ++p) {  /* for all base types */
            p->tword = (i | q);  /* load qualified type */
            p->next = TNIL;
            p->typ_size = i;
        }
        p->tword = (TSIGNED | INT | q);  /* Fake type */
        p->next = TNIL;
        p->typ_size = INT;
    }
    assert (p == &btypes[NTABTYPES]); /* we've filled the whole table */
    
    labltype.tword = ARY;
    labltype.next = tyalloc(INT);
    labltype.ary_size = 0;
}


TPTR
btype (t)
    register TPTR t;
{
    while (!ISBTYPE(t)) {
        t = t->next;
    }
    return (t);
}


#if     !defined (LINTP2) && !defined (CFLOW2)
int
comtypes(t1, t2, o)
    register TPTR t1, t2;
    int o;
{
    /*
    ** This routine makes sure that the types t1 and t2 are compatible.
    ** Error messages assuming that these are dereferenced pointer types
    ** are emitted when o!=0 and has the value of the operation performed.
    ** If o!=0, the qualifiers of the top type are not checked.
    */
    char *operation;

    if (o != 0)
        operation = opst[o];
    else if (QUALIFIERS(t1) != QUALIFIERS(t2))
            return (0);

    if (TOPTYPE(t1) != TOPTYPE(t2)) {
        if ( devdebug[COMPATIBLE] &&
                (TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
                (TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY))
            return (1);

        if (o != 0)
            /* "illegal pointer combination, op %s" */
            WERROR( devdebug[COMPATIBLE], MESSAGE(66), operation );
        return (0);
    }

    while (!ISBTYPE(t1)) {
        switch (TOPTYPE(t1)) {

        case FTN:
            if (!sameproto(t1, t2)) {
                if (o != 0)
                    /* "incompatible function prototype combination, op %s" */
                    WERROR( devdebug[COMPATIBLE], MESSAGE(172), operation );
                return(0);
            }
            break;

        case ARY:
            if (t1->ary_size != 0 && t2->ary_size != 0 &&
                    t1->ary_size != t2->ary_size) {
                if (o != 0)
                    /* "illegal array size combination, op %s" */
                    WERROR( devdebug[COMPATIBLE], MESSAGE(49), operation );
                return (0);
            }
            break;
        case PTR:
            /* Check for compatible types between void * and incomplete
                or object types. Sec. 3.2.2.3 of ANSI Standard */
            if ((TOPTYPE(t1->next) == TVOID && TOPTYPE(t2->next) != FTN) ||
                (TOPTYPE(t2->next) == TVOID && TOPTYPE(t1->next) != FTN))
                return(1);
            break;
        }

        t1 = t1->next;
        t2 = t2->next;

        if (TOPQTYPE(t1) != TOPQTYPE(t2)) {
            if  ( devdebug[COMPATIBLE] && QUALIFIERS(t1) == QUALIFIERS(t2) &&
                    ((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
                    (TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY)))
                return (1);
            if (o != 0)
                /* "illegal pointer combination, op %s" */
                WERROR( devdebug[COMPATIBLE], MESSAGE(66), operation );
            return (0);
        }
    }

    if (t1->typ_size != t2->typ_size) {
        if (o != 0) {
            if (TOPTYPE(t1) == ENUMTY && TOPTYPE(t2) == ENUMTY)
                /* "enumeration type clash, op %s" */
                WERROR( devdebug[COMPATIBLE], MESSAGE(37), operation );
            else
                /* "illegal structure type combination, op %s" */
                WERROR( devdebug[COMPATIBLE], MESSAGE(69), operation );
        }
        return (0);
    }

    return (1);
}

int
sameproto (t1,t2)
    register TPTR t1,t2;
{
        register PPTR p1,p2;


        if ( t1->ftn_parm == PNIL && t2->ftn_parm == PNIL ) {
                /* neither type has a prototype list, don't
                 * check anything.
                 */
                return ( 1 );
        }

        /* if either type has an empty prototype list, check
         * the other to see that each entry is compatible with its default
         * promotion type.
         */
        if ( ( t1->ftn_parm == PNIL && t2->ftn_parm != PNIL ) ||
                ( t2->ftn_parm == PNIL && t1->ftn_parm != PNIL ) )
                /* "mix of old and new style function declaration" */
                WARNING( WPROTO, MESSAGE(178) );

        if ( t1->ftn_parm == PNIL ) {
#ifdef COMPAT
/************************************************************************
** NOTE:  This must not be used if the compiler is modified to pass     *
** <4 byte parameters, or else bad code will result!                    *
************************************************************************/
                return ( ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) ||
                        defaultproto(t2->ftn_parm) );
#else
                return ( defaultproto(t2->ftn_parm) );
#endif
        }

        if ( t2->ftn_parm == PNIL ) {
#ifdef COMPAT
/************************************************************************
** NOTE:  This must not be used if the compiler is modified to pass     *
** <4 byte parameters, or else bad code will result!                    *
************************************************************************/
                return ( ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) ||
                        defaultproto(t1->ftn_parm) );
#else
                return ( defaultproto(t1->ftn_parm) );
#endif
        }

        /* check each entry on both lists to be sure they are
         * compatible.  If not return a failure for the function.
         */
        for (p1 = t1->ftn_parm, p2 = t2->ftn_parm ;
             p1 != PNIL && p2 != PNIL;
             p1 = p1->next, p2 = p2->next) {
                if (!comtypes(p1->type,p2->type,0)) return(0);
        }

        /* if both parameter lists don't end simultaneously, the
         * number of arguments is mismatched.
         */
        if ( p1 == PNIL && p2 == PNIL) {
                return(1);
        }
        else {
                return(0);
        }
}

int
tyencode (t)
    register TPTR t;
{
    register int result = 0;
    int shift = BTSHIFT;

    while (!ISBTYPE(t)) {
        switch (TOPTYPE(t)) {
        case PTR:
            result |= PTROUT << shift;
            break;
        case FTN:
            result |= FTNOUT << shift;
            break;
        case ARY:
            result |= ARYOUT << shift;
            break;
        }
        shift += TSHIFT;
        t = t->next;
    }
    switch (TOPTYPE(t)) {
    case UNDEF:
        return (result | 0);
    case FARG:
        return (result | 1);
    case SCHAR:
        return (result | 2);
    case SHORT:
        return (result | 3);
    case INT:
#if SZLONG == SZINT
    case LONG:
#endif
        return (result | 4);
#if SZLONG != SZINT
    case LONG:
#endif
    case LNGLNG:
        return (result | 5);
    case FLOAT:
        return (result | 6);
    case DOUBLE:
    case LDOUBLE:
        return (result | 7);
    case STRTY:
        return (result | 8);
    case UNIONTY:
        return (result | 9);
    case ENUMTY:
        return (result | 10);
    case MOETY:
        return (result | 11);
    case UCHAR:
    case CHAR:
        return (result | 12);
    case USHORT:
        return (result | 13);
    case UNSIGNED:
#if SZLONG == SZINT
    case ULONG:
#endif
        return (result | 14);
#if SZLONG != SZINT
    case ULONG:
#endif
    case ULNGLNG:
        return (result | 15);
    default:
        return (result);
    }
}
#endif

int
defaultproto(p)
register PPTR p;
{
        /* use default argument promotion on each basic
         * type encountered on the prototype list and issue an
         * error if it is incompatible with its original type or
         * an ellipsis is encountered (see section 3.5.4.3).
         *
         * In practice, this means we issue an error if
         * the basic type is char, short, float or ellipsis.
         */

        for (/*null*/; p != PNIL; p = p->next) {
                switch ( TOPTYPE( p->type ) ) {
                      case CHAR:
                      case UCHAR:
                      case SCHAR:
                      case SHORT:
                      case USHORT:
                      case FLOAT:
                      case TELLIPSIS:
                        return ( 0 );
                      default:
                        break;
                }
        }

        return ( 1 );
}


static TPTR tyrec[BNEST+1];             /* keep track of block nesting levels */
static int maxLevel = 0;                /* maximum defined level */

void
setty ()
{
    if (paramlevel != 0)
        cerror(TOOLSTR(M_MSG_207, "outstanding prototype list at type table mark" ));
    if (blevel > BNEST+1)
        UERROR( ALWAYS, TOOLSTR(M_MSG_320, "block nesting too deep") );
    else if (blevel <= maxLevel)
        curLoclTy = tyrec[blevel-1];
    else if (blevel > maxLevel + 1)
        cerror(TOOLSTR(M_MSG_208, "missing type table mark" ));
    else
        tyrec[blevel-1] = curLoclTy;
    maxLevel = blevel;
}

static TPTR
newty (level)
    int level;
{
    level -= paramlevel;
    if (level <= 0) {
        /* Allocate from global table */
        if (curGlobTy >= &globTyTab[nGlobTyEnts]) {
            globTyTab = (TPTR)getmem(nGlobTyEnts * sizeof(struct tyinfo));
            curGlobTy = globTyTab;
        }
        return (curGlobTy++);
    } else {
        /* Allocate from local table */
        if (level != blevel-paramlevel) cerror(TOOLSTR(M_MSG_209, "illegal type table level" ));
        if (level < maxLevel) {
            maxLevel = level;
            curLoclTy = tyrec[level];
        }
        if (curLoclTy >= &loclTyTab[nLoclTyEnts]) {
            cerror(TOOLSTR(M_MSG_210,
                "out of type nodes; recompile with -Nlx option with x greater than %d"),
                    nLoclTyEnts );
        }
        return (curLoclTy++);
    }
}


TPTR
tynalloc (bt)
    TWORD bt;
{
    register TPTR t;

    t = newty(blevel);
    t->tword = bt;
    t->next = TNIL;

    switch (bt) {
    case ARY:
    case PTR:
        t->ary_size = 0;
        break;
    case FTN:
        t->ftn_parm = PNIL;
        break;
    default:
        t->typ_size = bt & BTMASK;
        break;
    }

    return (t);
}


TPTR
incref (t, bt)
    TPTR t;
    TWORD bt;
{
    TPTR newt;

    newt = tynalloc(bt);
    newt->next = t;
    return (newt);
}


TPTR
modtype (t, bt)
    TPTR t;
    TWORD bt;
{
    register TPTR *t2 = &t;
    register TPTR t1 = t;
    register TPTR tNew;

    while (!ISBTYPE(t1)) {
        *t2 = tNew = tynalloc(t1->tword);
        tNew->info = t1->info;
        t1 = t1->next;
        t2 = &tNew->next;
    }

    tNew = tyalloc(bt);
    if (QUALIFIERS(t1)) {
        tNew = qualtype(tNew, QUALIFIERS(t1), 0);
    }
    *t2 = tNew;

    return (t);
}


TPTR
qualtype (t, qt, docopy)
    register TPTR t;
    TWORD qt;
    int docopy;
{
  
    if (t >= &btypes[TVOID] && t < &btypes[NTABTYPES]) {
        /* LS: It's a table entry, DO NOT ASSUME IT'S NOT ALREADY SO QUALIFIED!
	   qt &= ~(t->tword) now checks if it is already qualified */
        qt &= ~(t->tword);
        if (qt & VOLATILE)
            t += NUMBTYPES;
        if (qt & CONST)
            t += 2*NUMBTYPES;
        return t;
    }
    /* It's a constructed type. */

    if (docopy) {    /* Check if we have to copy the top node. */
        TPTR t2 = tynalloc(t->tword | qt);
        t2->info = t->info;
        t2->next = t->next;
        return t2;
    }
    t->tword |= qt;
    return t;
}


TPTR
unqualtype (t)
    register TPTR t;
{
        TPTR t2;

    if (!QUALIFIERS(t))
        return t;

    if (t >= &btypes[TVOID] && t < &btypes[NTABTYPES]) {
        /* It's a table entry. */
        if (ISVOLATILE(t))
            t -= NUMBTYPES;
        if (ISCONST(t))
            t -= 2*NUMBTYPES;
        return t;
    }
	
    /* It's a constructed type - assume we have to copy the top node. */
    t2 = tynalloc(t->tword & ~(CONST|VOLATILE));
    t2->info = t->info;
    t2->next = t->next;
    return t2;
}


TPTR
signedtype (t)
    register TPTR t;
{
    /*
    ** Assume this is called only once per type.
    */
    if (TOPTYPE(t) == INT) {
        /* Only "signed int" is meaningfully different. */
        t += NBTYPES - INT;
    }
    return (t);
}


TPTR
qualmember (t, qt)
    register TPTR t;
    TWORD qt;
{
    register TPTR newt;

    if (t >= &btypes[0] && t < &btypes[NTABTYPES]) {
        /*
        ** It's a table entry.  This shouldn't happen.
        */
        cerror(TOOLSTR(M_MSG_211, "trying to qualify generic structure" ));
    }
    t->tword |= qt;
    return (t);
}


TPTR
copytype (t, level)
    TPTR t;
    int level;
{
    register TPTR *t2 = &t;
    register TPTR t1 = t;
    register TPTR tNew;
    extern PPTR copyparms();

    while (!ISBTYPE(t1)) {
        *t2 = tNew = newty(level);
        tNew->tword = t1->tword;
        switch (TOPTYPE(t1)) {
        case FTN:
            tNew->ftn_parm = copyparms(t1->ftn_parm, level);
            break;
        case ARY:
            tNew->ary_size = t1->ary_size;
            break;
        }
        t1 = t1->next;
        t2 = &tNew->next;
    }

    if (level != blevel && (TOPTYPE(t1) == STRTY ||
            TOPTYPE(t1) == UNIONTY || TOPTYPE(t1) == ENUMTY)) {
        *t2 = tNew = newty(level);
        tNew->tword = t1->tword;
        tNew->typ_size = t1->typ_size;
        tNew->next = TNIL;
    } else {
        *t2 = t1;
    }

    return (t);
}


int
mkcomposite (t, t2, level)
    register TPTR t;
    register TPTR t2;
    int level;
{
    /*
    ** Merge t2 into t to get the composite type.
    ** ASSUME THE TYPES ARE COMPATIBLE.
    ** Return whether the new type had less information than the original.
    */
    register PPTR p, p2;
    extern PPTR copyparms();
    int lostInfo = 0;

    while (!ISBTYPE(t)) {
        switch (TOPTYPE(t)) {

        case FTN:
            p = t->ftn_parm;
            p2 = t2->ftn_parm;
            if (p == PNIL) {
#ifdef COMPAT
                if (devdebug[KLUDGE] && !defaultproto(p2))
                        /* "prototype not compatible with non-prototype declaration" */
                        WERROR( ALWAYS, MESSAGE(194) );
#endif
                t->ftn_parm = copyparms(p2, level);
            } else if (p2 != PNIL) {
                while (p != PNIL) {
                    if (mkcomposite(p->type, p2->type, level)) {
                        lostInfo = 1;
                    }
                    p = p->next;
                    p2 = p2->next;
                }
            } else {
#ifdef COMPAT
                if (devdebug[KLUDGE] && !defaultproto(p))
                        /* "prototype not compatible with non-prototype declaration" */
                        WERROR( ALWAYS, MESSAGE(194) );
#endif
                lostInfo = 1;
            }
            break;

        case ARY:
            if (t2->ary_size != 0) {
                t->ary_size = t2->ary_size;
            } else if (t->ary_size != 0) {
                lostInfo = 1;
            }
            break;
        }

        t = t->next;
        t2 = t2->next;
    }

    return (lostInfo);
}

PPTR
parmalloc ()
{
    static int bunchsize = 0;
    static PPTR parmbunch;

    if (bunchsize == 0) {
        /* Allocate another bunch of type nodes */
        parmbunch = (PPTR)getmem(MAXBUNCH * sizeof(struct parminfo));
        bunchsize = MAXBUNCH;
    }
    return (&parmbunch[--bunchsize]);
}

PPTR
copyparms (p, level)
        PPTR p;
        int level;
{
        register PPTR *p2 = &p;
        register PPTR p1 = p;
        register PPTR pNew;

        while (p1 != PNIL) {
                *p2 = pNew = parmalloc();
                pNew->type = copytype(p1->type, level);
                p1 = p1->next;
                p2 = &pNew->next;
        }
        *p2 = PNIL;

        return (p);
}

TPTR
FindBType(bt)
        TWORD bt;
{
        register int i;

        for (i = 0; i < NTABTYPES; i++)
                if (btypes[i].tword == bt)
                        return (&btypes[i]);
        return (TNIL);
}

