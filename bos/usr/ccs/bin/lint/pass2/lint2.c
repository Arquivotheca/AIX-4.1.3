static char sccsid[] = "@(#)63  1.9.2.1  src/bos/usr/ccs/bin/lint/pass2/lint2.c, cmdprog, bos411, 9428A410j 10/8/93 14:28:50";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CheckSymbol, FtnUsage, RefDefSymbol, SameMembers, SameParameters,
              SameTypes
 *
 * ORIGINS: 3 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Modified May 91 by RWaters: ILS changes.
 * Modified June 91 by RWaters: P23321 correctly handle void functions.
 *                              Changed fix for P46218. 
 */

#include "mfile1.h"
#include "lint2.h"
#include "lint_msg.h"

#define  MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

/*
** Check that the current symbol and symbol p are compatible types.
*/
CheckSymbol()
{
        int stat;

        stat = STORE;
#ifdef  DEBUG
        if (debug) {
                PrintSymbol("CHECKING OLD SYMBOL", prevSym);
                PrintSymbol("WITH NEW SYMBOL", curSym);
                printf("at %s (%s, %d-%d)\n", curPFname, curIFname, curDLine, curRLine);
        }
#endif

        /* if function address referenced, only want to change usage */
        if (curSym->usage & LINTADDR) {
                curSym->usage ^= LINTADDR;
                return(CHANGE);
        }
        /* if previous symbol was only an address reference, replace
           with a real reference (occurs only if first reference to
           a function is an address reference) */
        if ((prevSym->usage & LINTADDR) && (ISFTN(curSym->type))) {
                return(REPLACE);
        }

        /* Check for multiple definitions. */
        if ((prevSym->usage & LINTDEF) &&
                (curSym->usage & LINTDEF) &&
                !(prevSym->usage & LINTMBR)) {
                if (ISFTN(curSym->type))
                        LERROR(WDECLAR, MSGSTR(M_MSG_304,
                               "function %s multiply defined"), curSym, CDUSE);
                else if (devdebug[ANSI_MODE]) 
                        LERROR(WDECLAR, MSGSTR(M_MSG_305,
                               "symbol %s multiply defined"), curSym, CDUSE);
                stat = REJECT;
        }

        /* Check for compatible types. */
        if (!SameTypes(prevSym->type, curSym->type)) {
                stat = REJECT;
                if (!ISFTN(curSym->type))
                        LERROR(WDECLAR, MSGSTR(M_MSG_306,
                               "symbol %s type inconsistent"), curSym, CDUSE);
                else if (prevSym->usage & LINTVRG || curSym->usage & LINTVRG || ((curSym->usage & LINTDEF) && (!(prevSym->usage & LINTDEF))))
                        stat = CHANGE;
        }

        /* Check for possible struct/union/enum redefinition. */
        if (prevSym->nmbrs && !SameMembers(prevSym, curSym)) {
                if (TOPTYPE(curSym->type) == CPPCLASS ||
                    TOPTYPE(prevSym->type) == CPPCLASS)
                        LERROR(WDECLAR, MSGSTR(M_MSG_320,
                            "class %s inconsistently redefined"),
                            curSym, CDUSE);
                else
                        LERROR(WDECLAR, MSGSTR(M_MSG_307,
                            "struct/union/enum %s inconsistently redefined"),
                            curSym, CDUSE);
                stat = REJECT;
        }

        /* End of REJECTion processing, now check for CHANGEs. */
        if (stat == REJECT)
                return (REJECT);

        /* New instance of symbol usage? */
        /*GH A16715 01/28/91 */
        /*Now struct names declared in a file which has a NOTUSED NOTDEFINED   
          directives will not be warned about if it also is appears in another
          file.  This is to prevent symbols declared in standard include files
          from being warned about */
        if ((prevSym->usage & (LINTREF|LINTDEF|LINTDCL|LINTRET|LINTNOT|LINTNDF)) !=
                (curSym->usage & (LINTREF|LINTDEF|LINTDCL|LINTRET|LINTNOT|LINTNDF)))
                stat = CHANGE;

        /* New file reference of symbol? */
        if (strcmp(prevSym->rpf, curPFname) &&
                (curSym->usage & (LINTREF|LINTDCL)))
                stat = CHANGE;

        return (stat);
}

/*
** This routine makes sure that the types t1 and t2 are compatible.
** Error messages assuming that these are dereferenced pointer types.
*/
SameTypes(t1, t2)
        register TPTR t1, t2;
{
        TPTR ot1 = t1;
        TPTR ot2 = t2;

        /* Check qualifier top-type compatibility. */
        if (QUALIFIERS(t1) != QUALIFIERS(t2))
                return (0);

        /* INTs and ENUMs are compatible types. */
        if (TOPTYPE(t1) != TOPTYPE(t2)) {
                if ((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
                        (TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY))
                        return (1);
                return (0);
        }

        /* Examine each level of type indirection. */
        while (!ISBTYPE(t1)) {
                switch (TOPTYPE(t1)) {
                case FTN:
                        if (!SameParameters(t1, t2))
                                return (0);
                        break;

                case ARY:
                        if (t1->ary_size != 0 && t2->ary_size != 0 &&
                                t1->ary_size != t2->ary_size)
                                return (0);
                        break;
                }
                t1 = t1->next; t2 = t2->next;

                /* Check type specifier and qualifier. */
                if (TOPQTYPE(t1) != TOPQTYPE(t2)) {
                        if (QUALIFIERS(t1) == QUALIFIERS(t2) &&
                                ((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
                                (TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY)))
                                return (1);
                        if (ISFTN(ot1) || ISFTN(ot2))
                                LERROR(WDECLAR, MSGSTR(M_MSG_308,
                                        "function %s type inconsistent"),
                                        curSym, CRUSE);
                        return (0);
                }
        }

        if (t1->typ_size != t2->typ_size)
                return (0);

        return (1);
}

/*
** Check if both function argument lists are compatible.
*/
SameParameters(t1, t2)
        register TPTR t1,t2;
{
        register PPTR p1 = t1->ftn_parm;
        register PPTR p2 = t2->ftn_parm;

        /* Check for prototype lists. */
        if (p1 == PNIL && p2 == PNIL)
                return (1);

        /* Check for ellipsis, although strictly speaking ANSI does not
           permit an ellipsis to be the only function parameter, it is
           produced when a VARARGS0 appears before an old-style function
           definition. */
        if ((p1 != PNIL && TOPTYPE(p1->type) == TELLIPSIS) || (p2 != PNIL &&
                TOPTYPE(p2->type) == TELLIPSIS))
                return (1);

        /* If either prototype is NIL, check for default compatibilty. */
        if (p1 == PNIL)
                return (defaultproto(p2));
        if (p2 == PNIL)
                return (defaultproto(p1));

        /* Check each entry on both lists to be sure they are
           compatible.  If not return a failure for the function. */
        for (; p1 != PNIL && p2 != PNIL; p1 = p1->next, p2 = p2->next) {

                /* Aug. 21 GH, VARARGSn directive now implemented correctly
                        P46218 */
                 /* RW: better */
                if ((p1 != PNIL && TOPTYPE(p1->type) == TELLIPSIS) ||
                    (p2 != PNIL && TOPTYPE(p2->type) == TELLIPSIS))
                        return (1);

                if (!SameTypes(p1->type, p2->type)) {
                        LERROR(WDECLAR, MSGSTR(M_MSG_309,
                                "function %s argument type inconsistent"),
                                curSym, CRUSE);
                        return (0);
                }
        }

        /* If both parameter lists don't end simultaneously, the
           number of arguments is mismatched. */
        if (!(prevSym->usage & LINTVRG || curSym->usage & LINTVRG)) {
                 if (p1 == PNIL && p2 == PNIL)
                         return (1);

                 if ((p1 != PNIL && TOPTYPE(p1->type) != TELLIPSIS) ||
                     (p2 != PNIL && TOPTYPE(p2->type) != TELLIPSIS)) {
                        LERROR(WDECLAR, MSGSTR(M_MSG_310,
                                "function %s argument count mismatch"),
                                curSym, CRUSE);
                        return (0);
                }
        }
        return (1);
}

/*
** Check if struct/union/enum members are identical.
*/
SameMembers(p, q)
        SMTAB *p, *q;
{
        register MBTAB *mp, *mq;
        TWORD bt;

        /* Same symbol check. */
        if (p == q)
                return (1);

        /* Member count check. */
        if (p->nmbrs != q->nmbrs)
                return (0);

        /* Compare each member. */
        mp = p->mbrs; mq = q->mbrs;
        while (mp) {
#ifdef  DEBUG
                if (debug)
                        printf("\tcomparing %s vs. %s\n",mp->mname,mq->mname);
#endif
                if (strcmp(mp->mname, mq->mname))
                        return (0);
                if (!SameTypes(mp->type, mq->type))
                        return (0);
                if ((bt = BTYPE(mp->type)) == STRTY || bt == UNIONTY ||
                        bt == ENUMTY || bt == CPPCLASS)
                        if (strcmp(mp->tagname, mq->tagname))
                                return (0);
                mp = mp->next; mq = mq->next;
        }
        return (1);
}

/*
** Examine symbol for proper reference/definitions usage.
*/
RefDefSymbol(h)
        register SMTAB *h;
{
        switch (h->usage & (LINTREF|LINTDEF|LINTDCL)) {
        case LINTREF|LINTDCL:
                if (!(h->usage & LINTNDF)) {
                        if (ISFTN(h->type))
                                LERROR(WUSAGE, MSGSTR(M_MSG_311,
                                        "function %s used but not defined"), h, RUSE);
                        else
                                LERROR(WUSAGE, MSGSTR(M_MSG_312,
                                        "symbol %s used but not defined"), h, RUSE);
                }
                break;
        case LINTDEF:
        case LINTDEF|LINTDCL:
                if (!(h->usage & LINTNOT)) {
                        if (ISFTN(h->type)) {
                                if (strcmp(h->sname, "main") && !(h->usage & LINTLIB))
                                        LERROR(WUSAGE, MSGSTR(M_MSG_313,
                                                "function %s defined but never used"),
                                                h, DUSE);
                        } else
                                LERROR(WUSAGE, MSGSTR(M_MSG_314,
                                        "symbol %s defined but never used"), h, DUSE);
                }
                break;
        case LINTDCL:
                if (!(h->usage & LINTNOT) && !(h->usage & LINTNDF)) {
                        if (ISFTN(h->type))
                                LERROR(WUDECLAR, MSGSTR(M_MSG_315,
                                "function %s declared but never used or defined"),
                                h, RUSE);
                        else
                                LERROR(WUDECLAR, MSGSTR(M_MSG_316,
                                "symbol %s declared but never used or defined"),
                                h, RUSE);
                }
                break;
        }
}

/*
** Examine function for proper/consistent usage.
*/
FtnUsage(h)
        register SMTAB *h;
{
        switch (h->usage & (LINTRET|LINTUSE|LINTIGN)) {
        case LINTUSE:
        case LINTUSE|LINTIGN:
                /* RW: */
                if (TOPTYPE(h->type->next) != TVOID)
                        LERROR(WUSAGE, MSGSTR(M_MSG_317,
                                "function %s return value used, but none returned"),
                                h, RUSE);
                break;

        case LINTRET|LINTIGN:
                /* RW: */
                if (TOPTYPE(h->type->next) != TVOID)
                        LERROR(WUSAGE, MSGSTR(M_MSG_318,
                                "function %s return value is always ignored"),h,DUSE);
                break;

        case LINTRET|LINTUSE|LINTIGN:
                /* RW: */
                if (TOPTYPE(h->type->next) != TVOID)
                        LERROR(WUSAGE, MSGSTR(M_MSG_319,
                                "function %s return value is sometimes ignored"),
                                h, DUSE);
                break;
        }
}
