static char sccsid[] = "@(#)62  1.6.1.1  src/bos/usr/ccs/bin/lint/pass2/hasher.c, cmdprog, bos411, 9428A410j 10/8/93 14:28:15";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: AddFtnUsage, ChangeSymbol, CheckSymbols, FtnRefSymbol,
              LookupSymbol, ReplaceSymbol, StoreSymbol
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
 */

#include "lint_msg.h"
#include "mfile1.h"
#include "lint2.h"

#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

/*
** The segmented hash bucket definitions.
*/
struct ht {
        SMTAB   *htabLo;        /* low address of hash bucket */
        SMTAB   *htabHi;        /* high address of hash bucket */
        int     htabCnt;        /* number of entries in hash bucket */
};
static struct ht htab[MAXHASH]; /* array of hash buckets */

/*
** Hash and store name in permanent storage area. Mode = 1 causes insertion.
*/
SMTAB *
LookupSymbol(p, stat)
        SMTAB *p;
        int stat;
{
        register struct ht *htp;
        register SMTAB *h;
        register int i;
        char *cp;
        int offset;

        /* Hash the complete name. */
        cp = p->sname;
        i = 0;
        while (*cp)
                i = (i << 1) + *cp++;
        offset = ((i < 0) ? -i : i) % HASHBLK;
        cp = p->sname;

        /* Look through each hash bucket for name. */
        for (htp = htab; htp < &htab[MAXHASH]; htp++) {

                /* Allocate hash bucket, if needed. */
                if (htp->htabLo == 0) {
                        htp->htabLo = (SMTAB *) calloc(sizeof(SMTAB), HASHBLK);
                        if (htp->htabLo == 0)
                                cerror(MSGSTR(M_MSG_258,
                                        "no memory for hash table"));
                        htp->htabHi = htp->htabLo + HASHBLK;
                }
                h = htp->htabLo + offset;

                /* Use quadratic re-hash. */
                i = 1;
                do {
                        if (h->sname == 0) {
                                /* Symbol insertion. */
                                        /* High-water mark set at 3/4 full. */
                                /* GH 09/10/90 Fix to a13798  */
                                if(htp->htabCnt > ((HASHBLK * 3) >> 2))
                                        break;
                                if (stat == STORE) {
                                        htp->htabCnt++;
                                        StoreSymbol(h, p);
                                        return (h);
                                }
                                return (0);
                        }

                        /* Symbol lookup. */
                        if (!strcmp(h->sname, cp) &&
                                ((h->usage&SNSPACE) == (p->usage&SNSPACE)))
                                return (h);

                        /* Collision resolution. */
                        h += i;
                        i += 2;
                        if (h >= htp->htabHi)
                                h -= HASHBLK;
                } while (i < HASHBLK);
        }
        cerror(MSGSTR(M_MSG_259, "Ran out of hash tables"));
}

/*
** Store the symbol into a hashed slot.  Determine proper
** reference/definition context.
*/
StoreSymbol(h, p)
        SMTAB *h, *p;
{
        p->sname = StoreSName(p->sname);
        /* Initially save something in the definitions area
           so that if there is an error message using DUSE
           we will not get garbage. Later if there is a real
           definition we will enter the correct information. */
        if (p->usage & (LINTDEF|LINTREF|LINTDCL)) {
                p->dpf = curPFname;
                p->dif = curIFname;
                p->dl = curDLine;
                p->rpf = curPFname;
                p->rif = curIFname;
                p->rl = curRLine;
        }
        memcpy((char *) h, (char *) p, sizeof(SMTAB));
#ifdef  DEBUG
        if (debug)
                PrintSymbol("INSERT SYMBOL", h);
#endif
}

ReplaceSymbol(h, p)
        SMTAB *h, *p;
{
        p->sname = h->sname;
        if (p->usage & (LINTDEF|LINTREF|LINTDCL)) {
                p->dpf = curPFname;
                p->dif = curIFname;
                p->dl = curDLine;
                p->rpf = curPFname;
                p->rif = curIFname;
                p->rl = curRLine;
        }
        memcpy((char *) h, (char *) p, sizeof(SMTAB));
#ifdef  DEBUG
        if (debug)
                PrintSymbol("REPLACE SYMBOL", h);
#endif
}

/*
** Change the reference/definition context of an existing symbol.
*/
ChangeSymbol(h, p)
        SMTAB *h, *p;
{
        h->usage |= p->usage;
        if (p->usage & LINTDEF) {
                h->dpf = curPFname;
                h->dif = curIFname;
                h->dl = curDLine;
                h->type = p->type;
        }
        if (p->usage & (LINTREF|LINTDCL)) {
                h->rpf = curPFname;
                h->rif = curIFname;
                h->rl = curRLine;
        }
#ifdef  DEBUG
        if (debug)
                PrintSymbol("CHANGE SYMBOL", h);
#endif
}

/*
** Reference function call.
*/
FtnRefSymbol(h, p)
        SMTAB *h, *p;
{
        if (p->usage & LINTREF) {
                h->usage |= LINTREF;
                h->rpf = curPFname;
                h->rif = curIFname;
                h->rl = curRLine;
        }
#ifdef  DEBUG
        if (debug)
                PrintSymbol("FTN REF SYMBOL", h);
#endif
}

/*
** Add function usage to existing function symbol.
*/
AddFtnUsage(h, p)
        SMTAB *h, *p;
{
        h->usage |= p->usage;
#ifdef  DEBUG
        if (debug)
                PrintSymbol("ADDFTN SYMBOL", h);
#endif
}

/*
** Examine each symbol for:
**      RefDefSymbol() - proper reference/definitions usage
**      FtnUsage() - consistent function usage
*/
CheckSymbols()
{
        register struct ht *htp;
        register SMTAB *h;

        /* Search each hash bucket. */
        for (htp = htab; htp < &htab[MAXHASH]; htp++) {
                if (htp->htabLo == 0)
                        continue;

                /* Examine each symbol. */
                for (h = htp->htabLo; h < htp->htabHi; h++) {
                        if (h->sname) {
                                RefDefSymbol(h);
                                if (ISFTN(h->type))
                                        FtnUsage(h);
                        }
                }
        }
}
