static char sccsid[] = "@(#)18  1.12.1.7  src/bos/usr/ccs/bin/lex/sub2.c, cmdlang, bos411, 9428A410j 6/10/94 12:24:54";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 * 
 * FUNCTIONS: acompute, add, bprint, cfoll, cgoto, first, follow, layout,
 *            member, mkmatch, nextstate, notin, packtrans, padd, pccl, pfoll,
 *            pstate, rprint, shiftr, stprt, upone, xfirst
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "RCSfile: sub2.c,v Revision: 2.7  (OSF) Date: 9
  0/10/07 17:42:35 "; */

        /* Multi-byte support added by Michael S. Flegel, July 1991 */
           
# include "ldefs.h"
# include <stdlib.h>
# include <string.h>

/*OSF code*/
#ifdef _BLD
#ifndef __
#define __(args)   ()
#endif
static void padd __((int **, int));
static void add __((int **, int));
#else
static void padd(int **, int);
static void add(int **, int);
#endif

#define NEXT(ch,i)      (ch[i]?1:2)

static wchar_t *__packarray;

static void follow();
static int  packcmp();
static int  xcclcmp();

/* --------------------------- NOTES --------------------------- */

/*
 *  - The parse tree created by the yacc parser is used as a
 *    global throughout this file and is not listed in the
 *    set of global variables for each function.
 *
 *  - This file contains an implementation of the algorithm
 *    for converting a regular expression parse tree into
 *    a Deterministice Finite Automata (Finite State Machine).
 *    However the purpose of the comments is to explain the
 *    implementation and not the algorithm itself.
 */
  
/* --------------------------- cfoll --------------------------- */

/*
 *   1. Calculates the follow positions for each position
 *      in the parse tree.  A position is a node which has
 *      a character associated with it.  The following are
 *      position nodes: RWCHAR, RSTR, RCCL and RNULLS.
 *      Cfoll() traverses the tree from the top until it hits
 *      a position node at which point it calls follow().
 *      Follow() gets the set of follow positions and stores
 *      it in tmpstat[].
 *
 *   2. Transfers the set of follow positions from tmpstat to
 *      nxtpos[] by calling padd().  A pointer to the set of
 *      positions in nxtpos[] is stored in foll[v] where v
 *      is the position number.  The first element of the list
 *      in nxtpos is the count of the number of follow positions.
 *
 *   3. For RCCL nodes, the pointer to the list of characters
 *      in left[v] is replaced with a pointer to a list of
 *      corresponding character class identifiers stored in pchar.
 *      Cfoll8bit(), cfollMB and cfollCCL are called to calculate
 *      the list of character class identifiers.
 *
 * inputs -  v      : node number
 * outputs - none
 * globals - tmpstat: Array of flags indicating which nodes are active.
 *                    Cfoll() initializes tmpstat and count before calling
 *                    follow().
 *           count  : count of active nodes in tmpstat
 *           pchar  : contains a list of character class identifiers for
 *                    each RCCL node.
 *           pcptr  : pointer to current position in pchar
 *           foll   : array of pointers to the list of follow positions
 *                    for each position as stored in nxtpos
 */

void
cfoll(v)
  int v;
{
    register int        i,j,k,n;
    wchar_t             *p;
    
    i = name[v];
    if(i < NCH)                                 /* 8-bit character */
        i = 1;
    switch(i)
    {
    case 1: case RWCHAR: case RSTR: case RCCL: case RNCCL: case RNULLS:
        for(j=0;j<tptr;j++)
            tmpstat[j] = FALSE;
        count = 0;
        follow(v);
# ifdef PP
        padd(foll,v);                           /* packing version */
# else
        add(foll,v);                            /* no packing version */
# endif
        if(i == RSTR)
            cfoll(left[v]);
        else if(i == RCCL || i == RNCCL)        /* compress ccl list */
        {
            for(j=1; j<NCH;j++)                 /* look at all the 8-bits */
                symbol[j] = (i==RNCCL);
            wsymboli = 0;
            /*
             * identify the characters in the CCL
             */
            p = (wchar_t *)left[v];             /* left points to the characters of the CCL */
            while(*p)
            {
                if (*p < NCH)                   /* 8-bit */
                    symbol[*p++] = (i == RCCL);
                else                            /* MB */
                    addwsymbol(*p++,0);
            }
            /*
             * make a unique list of all of the CCLs associated with each
             * character in this one CCL
             */
            p = pcptr;
            cfoll8bit (p);
            cfollMB (p);
            cfollCCL (p,v);
            *pcptr++ = 0;
            if(pcptr > pchar + pchlen)
                error(MSGSTR(MANYPCLASSES, "Too many packed character classes"));
            /*
             * record the CCL list instead of the actual characters
             */
            left[v] = (int)p;                   /* left now contains the related CCL */
            name[v] = RCCL;                     /* RNCCL eliminated */
# ifdef DEBUG
            if(debug && *p)
            {
                printf("ccl %d: %d",v,*p++);
                while(*p)
                    printf(", %d",*p++);
                putchar('\n');
            }
# endif
        }
        break;
    case CARAT:
        cfoll(left[v]);
        break;
    case STAR: case PLUS: case QUEST: case RSCON: 
        cfoll(left[v]);
        break;
    case BAR: case RCAT: case DIV: case RNEWE:
        cfoll(left[v]);
        cfoll(right[v]);
        break;
# ifdef DEBUG
    case FINAL:
    case S1FINAL:
    case S2FINAL:
        break;
    default:
        warning("bad switch cfoll %d",v);
# endif
    }
    return;
}

/* ------------------------- cfoll8bit ------------------------- */

/*
 *    1. Adds the CCL identifiers for 8 bit characters to pchar.
 *
 * inputs -  p      : pointer to the list of characters associated
 *                    with the current RCCL node.
 * outputs - none
 * globals - pchar  : contains a list of character class identifiers for
 *                    each RCCL node.
 *           pcptr  : pointer to current position in pchar
 */

cfoll8bit (p)
  wchar_t       *p;
{
register int    j, k;

    for(j=1;j<NCH;j++)                          /* add the 8-bit CCLs */
    {
        if(symbol[j])
        {
            for(k = 0; p+k < pcptr; k++)
            {
                if(cindex[j] == *(p+k))
                    break;
            }
            if(p+k >= pcptr)
                *pcptr++ = cindex[j];
        }
    }
}

/* -------------------------- cfollMB -------------------------- */

/*
 *    1. Adds the CCL identifiers for multibyte characters to pchar.
 *
 * inputs -  p      : pointer to the list of characters associated
 *                    with the current RCCL node.
 * outputs - none
 * globals - pchar  : contains a list of character class identifiers for
 *                    each RCCL node.
 *           pcptr  : pointer to current position in pchar
 */

cfollMB (p)
  wchar_t       *p;
{
register int    j, k;
int             n;
    
    for (j=0;j<wsymboli;j++)                    /* add the MB CCLs */
    {
        n = hashfind(wsymbol[j],wmatch);
        if (PMATCH(n))
        {
            for (k = 0; p+k < pcptr; k++)
            {
                if (PMATCH(n)->cindex == *(p+k))
                    break;
            }
            if (p+k >= pcptr)
                *pcptr++ = PMATCH(n)->cindex;
        }
    }
}

/* ------------------------- cfollCCL ------------------------- */

/*
 *    1. Adds the CCL identifiers for xCCLs to pchar.
 *
 * inputs -  p      : pointer to the list of characters associated
 *                    with the current RCCL node
 *           v      : current RCCL node number
 * outputs - none
 * globals - pchar  : contains a list of character class identifiers for
 *                    each RCCL node.
 *           pcptr  : pointer to current position in pchar
 */

cfollCCL (p,v)
  wchar_t       *p;
  int           v;
{
register int    i, j, k;
    
    if (!right[v]) return;         /* no CCLs associated with this node */

    for (i = 0; i < xccltop; i++)               /* add the special CCLs */
    {
        if (xccl[i].type == right[v])                /* xCCL belongs to this group */
        {
            for (j = wmatchlist; j; j = PMATCH(j)->list)
            {
                if (inCCL (wmatch->table[j].id,xccl[i].type))
                {
                    for (k = 0; p+k < pcptr; k++)
                    {
                        if (PMATCH(j)->cindex == *(p+k))
                            break;
                    }
                    if (p+k >= pcptr)
                        *pcptr++ = PMATCH(j)->cindex;
                }
            }
            /*
             * Defect 89312 - must add the xccl itself
             */
            for (k=0; p+k < pcptr; k++)
            {
                if(xccl[i].cindex == *(p+k))
                    break;
            }
            if (p+k >= pcptr)
                *pcptr++ = xccl[i].cindex;
        }
    }
}

/* ------------------------- cfollCCL ------------------------- */

/*
 *    1. See if the specified character is contained in the
 *       specified xCCL.
 *
 * inputs -  c      : character to test
 *           ccl    : special character class type
 * outputs - 0      : if the character does not belong to the xCCL
 *           1      : if the character belongs to the xCCL
 * globals - pchar  : contains a list of character class identifiers for
 *                    each RCCL node.
 *           pcptr  : pointer to current position in pchar
 */

int
inCCL (c,ccl)
  wchar_t       c;
  int           ccl;
{
    switch (ccl)
    {
    case CCLDOT:        return ((c=='\n'?0:1));
    }
    return (0);
}


# ifdef DEBUG

/* -------------------------- pfoll -------------------------- */

/*
 *   1. Debug code - print the list of follow positions for 
 *      each position.
 */

void
pfoll()
{
register int    i,k,*p;
int             j;
    
    printf("Follow: chars\n");
    for(i=0;i<tptr;i++)
    {
        if(p=foll[i])
        {
            j = *p++;
            if(j >= 1)
            {
                printf("%3d:    %d",i,*p++);
                for(k=2;k<=j;k++)
                    printf(", %d",*p++);
                putchar('\n');
            }
        }
    }
    return;
}
# endif


/* --------------------------- add --------------------------- */

/*
 *    1. Compacts the array of flags in tmpstat and stores it
 *       in nxtpos with the number of elements in the list at
 *       the beginning.  In tmpstat, there is one element per
 *       node which are either active (1) or inactive (0), add()
 *       stores the list of active nodes only into nxtpos.  A
 *       pointer to the list in nxtpos is stored in the specified
 *       array (array[]) at the specified position (n).
 *
 * inputs -  array  : array in which to store the pointer to nxtpos
 *           n      : position in the array in which to store the
 *                    pointer
 * outputs - none
 * globals - tmpstat: array of flags indicating which nodes are active
 *           count  : # of active nodes in tmpstat
 *           nxtpos : permanent storage for the information in tmpstat
 */

static void
add(array,n)
  int           **array;
  int           n;
{
register int    i, *temp, *bufend;
wchar_t         *ctemp;
    /*
     * remember where list of positions starts
     */
    temp = nxtpos;
    ctemp = tmpstat;
    /**
     * record the array of positions for this element
     * - note no packing is done in positions
     * - first element of array is number of associated positions
     */
    array[n] = nxtpos;
    *temp++ = count;
    /**
     * for each transition
     * - if a transition has been recorded earlier, remember it as a position
     */
    i=0;
    /* extra check added so that temp does not go past end of
     * buffer.  Defect #65042
     */
    bufend = positions + maxpos;
    while((i<tptr) && (temp < bufend)) 
    {
        if(ctemp[i] == TRUE)
            *temp++ = i;
            i++;
    }

    nxtpos = temp;                              /* update start of next set of positions */
    if(nxtpos >= bufend)
    {
        error(MSGSTR(EMAXPOS, "Too many positions %s"),
              (maxpos== MAXPOS?MSGSTR(EMAXPOS1, "\nTry using %%p num"):""));
    }
    return;
}

/* -------------------------- follow -------------------------- */

/*
 *    1. Determines which positions can follow a given node
 *       by looking at the parent node.
 *    
 *    2. Calls first() when necessary since some nodes are
 *       followed by the initial positions of another node.
 *
 *       For example:
 *
 *                   RCAT
 *                  /    \
 *                 *      RWCHAR (e)
 *                /
 *              RSTR (a)
 *               |
 *             RWCHAR (d)
 *
 *       The follow positions for the * node are the first
 *       positions of the RWCHAR (e) node (because the parent
 *       node is an RCAT).  The follow position is thus 'e'.
 *
 *       The follow positions for the RSTR node are the first
 *       positions for the RSTR node (because the parent node
 *       is an *) and the follow positions for the * node
 *       which are 'd' and 'e' respectively.
 *
 * inputs -  v      : node number
 * outputs - none
 * globals - tmpstat: array of flags indicating which nodes are active,
 *                    in this case, which nodes belong to the set of
 *                    follow positions for v
 *           count  : # of active nodes in tmpstat
 */

static void
follow(v)
  int v;
{
    register int        p;
    
    if(v >= tptr-1)
        return;
    p = parent[v];
    if(p == 0)
        return;
    /*
     * will not be RWCHAR CHAR RNULLS FINAL S1FINAL S2FINAL RCCL RNCCL
     */
    switch(name[p])
    {
    case RSTR:
        if(tmpstat[p] == FALSE)
        {
            count++;
            tmpstat[p] = TRUE;
        }
        break;
    case STAR: case PLUS:
        first(v);
        follow(p);
        break;
    case BAR: case QUEST: case RNEWE:
        follow(p);
        break;
    case RCAT: case DIV: 
        if(v == left[p])
        {
            if(nullstr[right[p]])
                follow(p);
            first(right[p]);
        }
        else
            follow(p);
        break;
    case RSCON: case CARAT: 
        follow(p);
        break;
# ifdef DEBUG
    default:
        warning("bad switch follow %d",p);
# endif
    }
    return;
}

/* -------------------------- first -------------------------- */

/*
 *    1. Calculates the set of positions with v as root which
 *       can be active initially.
 *
 *    2. Handles start conditions by traversing the section of
 *       the tree rooted at an RSCON node only if the current
 *       start state (in the global "stnum") matches one of
 *       the start states associated with the RSCON node (the
 *       pointer to the list of start states is stored in
 *       right[RSCON node #]).
 *
 *    3. Called from two places:
 *       - From follow(), in the case of a STAR, RCAT, PLUS or
 *         DIV node, for calculation of follow positions.
 *       - From cgoto(), to calculate the set of initial positions
 *         for each start state.
 *
 * inputs -  v      : node number
 * outputs - none
 * globals - tmpstat: array of flags indicating which nodes are active,
 *                    in this case, which nodes belong to the set of
 *                    follow positions for v
 *           count  : # of active nodes in tmpstat
 */

first(v)
  int           v;
{
register int    i;
wchar_t         *p;
    
    i = name[v];
    if(i < NCH)
        i = 1;
    switch(i)
    {
    case 1: case RWCHAR: case RCCL: case RNCCL: case RNULLS: case FINAL:
    case S1FINAL: case S2FINAL:
        if(tmpstat[v] == FALSE)
        {
            count++;
            tmpstat[v] = TRUE;
        }
        break;
    case BAR: case RNEWE:
        first(left[v]);
        first(right[v]);
        break;
    case CARAT:
        if(stnum % 2 == 1)
            first(left[v]);
        break;
    case RSCON:
        i = stnum/2 +1;
        p = (wchar_t *)right[v];
        while(*p)
        {
            if(*p++ == i)
            {
                first(left[v]);
                break;
            }
        }
        break;
    case STAR: case QUEST: case PLUS:  case RSTR:
        first(left[v]);
        break;
    case RCAT: case DIV:
        first(left[v]);
        if(nullstr[left[v]])
            first(right[v]);
        break;
# ifdef DEBUG
    default:
        warning("bad switch first %d",v);
# endif
    }
    return;
}

/* -------------------------- xfirst -------------------------- */

/*
 *   1. When in an exclusive start state, call first() only for
 *      RSCON nodes.  ie. skip expressions that have no start
 *      state specified.
 *      
 * inputs - v      : node number
 * outputs - none
 */

void xfirst(int v)
{
int             i;
    
    i = name[v];
    switch(i)
    {
    case RNEWE:
        xfirst(left[v]);
        xfirst(right[v]);
        break;
    case RSCON:
        first(v);
        break;
    }
    return;
}

/* -------------------------- cgoto -------------------------- */

/*
 *    1. For each start state, calculate the positions which
 *       can be active initially by calling first(), and store
 *       the resulting lists in nxtpos[] and pointers to the
 *       lists in state[] by calling add(state, stnum).
 *
 *    2. For each state:
 *
 *          a) Call acompute() to get the list of actions
 *             associated with this state
 *          b) Get the set of characters associated with the
 *             list of first positions for the current state
 *             and store them in symbol[] and wsymbol[].
 *             XCCLs are represented in wsymbol[] by a null
 *             character followed by the xccl type.
 *          c) Initialize tch and tst which store the set of
 *             packed characters and the associated transitions
 *             repectively.
 *          d) Call calcnext() for each character in symbol[] and
 *             wsymbol[].  Calcnext() will fill in tch and tst and
 *             add states as necessary.
 *          e) Call packtrans() to pack the transitions in tch and
 *             tst arrays into permanent storage.
 *        *note - As cgoto() loops through the states, new states
 *             are added by calcnext(), thus cgoto() continues until
 *             all the states of the finite state machine have
 *             been generated.
 *
 * inputs -  none
 * outputs - none
 * globals - tch[]  : holds compact list of characters associated
 *                    with the current state
 *           tst[]  : holds the transition state number for the
 *                    corresponding character in tch[]
 *           tryit  : flag to indicate the presence of CCLs in which
 *                    case compression by packtrans() may be possible
 *           cpackflag[stnum] : set to true for the current state if it has
 *                    packed characters (makes use of match tables)
 *           sfall[stnum] : fall back state for the current state
 */

cgoto()
{
register int    i, j, k, s;
int             npos, curpos, n;
int             tryit;
int             *tst;                           /* record state info */
wchar_t         *tch;                           /* associate characters for state info */
wchar_t         *q;
int             x;
    
    /* generate initial state, for each start condition */
    
    if(ratfor)
    {
        fprintf(fout,"blockdata\n");
        fprintf(fout,"common /Lvstop/ vstop\n");
        fprintf(fout,"define Svstop %d\n",nstates+1);
        fprintf(fout,"integer vstop(Svstop)\n");
    }
    else
        fprintf(fout,"int yyvstop[] = {\n0,\n");
    
    while (stnum < 2 || stnum/2 < sptr)
    {
        for(i = 0; i<tptr; i++)
            tmpstat[i] = 0;
        count = 0;
        if(tptr > 0)
            if (excl[stnum/2])
                xfirst(tptr-1);
            else
                first(tptr-1);
        add(state,stnum);
# ifdef DEBUG
        if(debug)
        {
            if(stnum > 1)
                printf("%S:\n",sname[stnum/2]);
            pstate(stnum);
        }
# endif
        stnum++;
    }
    stnum--;
    /*
     * even stnum = might not be at line begin
     * odd stnum  = must be at line begin
     * even states can occur anywhere, odd states only at line begin
     */
    for(s = 0; s <= stnum; s++)
    {
        tryit = FALSE;
        cpackflg[s] = FALSE;
        sfall[s] = -1;
        acompute(s);
        
        for(i=0;i<NCH;i++)                      /* empty 8-bit symbol table */
            symbol[i] = 0;
        wsymboli = 0;                           /* empty MB symbol table */
        
        npos = *state[s];
        for(i = 1; i<=npos; i++)
        {
            curpos = *(state[s]+i);
            if(name[curpos] < NCH)              /* 8-bit symbol */
                symbol[name[curpos]] = TRUE;
            else
            {
                switch(name[curpos])
                {
                case RWCHAR:                    /* wide symbol */
                    addwsymbol(wname[curpos],1);
                    break;
                case RCCL:
                    tryit = TRUE;
                    for (q = (wchar_t *)left[curpos]; *q; q++)
                    {
                        /*
                         * remember applicable 8-bits
                         */
                        for(j = 1; j < NCH; j++)
                        {
                            if(cindex[j] == *q)
                                symbol[j] = TRUE;
                        }
                        /*
                         * remember applicable MBs
                         */
                        for (j = wmatchlist; PMATCH(j); j = PMATCH(j)->list)
                        {
                            if (PMATCH(j)->cindex == *q)
                                addwsymbol(wmatch->table[j].id,0);
                        }
                        /**
                         * remember applicable special CCLs
                         * - they will be recorded each with a preceeding 0
                         */
                        for (j = 0; j < xccltop; j++)
                        {
                            if (xccl[j].cindex == *q)
                            {
                                addwsymbol (0,1); /* flag the next as a CCL */
                                if (!addwsymbol (xccl[j].type,0)) /* if the symbol was not added */
                                    wsymboli -= 1; /* ... then kill the marker */
                            }
                        }
                    }
                    break;
                case RSTR:
                    if (right[curpos] != RWCHAR) /* 8-bit */
                        symbol[right[curpos]] = TRUE;
                    else                        /* wide */
                        addwsymbol(wname[curpos],1);
                    break;
# ifdef DEBUG
                case RNULLS:
                case FINAL:
                case S1FINAL:
                case S2FINAL:
                    break;
                default:
                    warning("bad switch cgoto %d state %d",curpos,s);
                    break;
# endif
                } /* switch */
            } /* else */
        } /* for */
# ifdef DEBUG
        if(debug)
        {
            x = 0;
            charc = 0;
            for(i = 1; i<NCH; i++)              /* 8-bit transitions */
            {
                if(symbol[i])
                {
                    if (!x)
                    {
                        printf("State %d transitions on (8-bit chars):\n\t",s);
                        x = 1;
                    }
                    allprint(i);
                    if(charc > LINESIZE)
                    {
                        charc = 0;
                        printf("\n\t");
                    }
                }
            }
            if (wsymboli)
            {
                if (charc)
                    putchar('\n');
                printf("State %d transitions on (multi chars and [:CCLs:]):\n\t",s);
                charc = 0;
                for (i = 0; i < wsymboli; i++)
                {
                    if (wsymbol[i])             /* MB */
                        allprint(wsymbol[i]);
                    else                        /* CCL */
                    {
                        i += 1;
                        printf ("[:%#4.4x:]", wsymbol[i]);
                        charc += 10;
                    }
                    
                    if (charc > LINESIZE)
                    {
                        charc = 0;
                        printf ("\n\t");
                    }
                }
            }
            if (charc)
                putchar('\n');
        } /* if debug */
# endif
        /*
         * for each char, calculate next state
         */
        tst = (int *)myalloc(NCH+wsymboli+1,sizeof(*tst));
        tch = (wchar_t *)myalloc(NCH+wsymboli+1,sizeof(*tch));
        if ((tst == 0) || (tch == 0))
            error (MSGSTR(CALLOCFAILED,"No space for next state calculation"),0);
        
        n = 0;
        for(i = 1; i<NCH; i++)                  /* add states for 8-bit chars */
        {
            if(symbol[i])
                calcnext(s,i,0,&n,tch,tst);
        }

        for (i = 0; i < wsymboli; i++)          /* add states for MBs and CCLs */
        {
            if (wsymbol[i])                     /* MB */
                calcnext(s,wsymbol[i],0,&n,tch,tst);
            else                                /* CCL */
            {
                i += 1;
                calcnext(s,0,wsymbol[i],&n,tch,tst);
            }
        }
        tch[n] = 0;
        tst[n] = -1;
        tch[++n] = 0;                           /* "\0\0" to end it */
        tst[n++] = -1;
        n -= 2;                                 /* ignore the terminators */
        /*
         * pack transitions into permanent array
         */
        if(n > 0)
            packtrans(s,tch,tst,n,tryit);
        else
            gotof[s] = -1;
        
        cfree((void *)tst,NCH+wsymboli+1,sizeof(*tst));
        cfree((void *)tch,NCH+wsymboli+1,sizeof(*tch));
    } /* for */
    ratfor ? fprintf(fout,"end\n") : fprintf(fout,"0};\n");
    return;
}

/* ------------------------- calcnext ------------------------- */

/*
 *    1. Calls nexstate() which fills in tmpstat[] with the union
 *       of follow positions for each position associated with
 *       the given state and the given character.  This constitutes
 *       a new state (if an identical state does not already exist)
 *       and is the transition state for the given state and
 *       character.
 *
 *    2. Checks if the state in tmpstat[] already exists by calling
 *       notin().
 *
 *    3. If the state does not exist, a new state is created by
 *       calling add(state, stnum).
 * 
 *    4. Stores the character and the transition state in tch[] and
 *       tst[] respectively.
 *
 *    5. Increments the tch[] and tst[] index.
 *
 * inputs -  s      : state number
 *           c      : character
 *           x      : xccl type (0 if none)
 *           n      : index for tch[] and tst[]
 *           tch[]  : character array
 *           tst[]  : transition state array
 * outputs - none
 * globals - state  : array of pointers to the list of positions associated
 *                    with each state
 */

calcnext(s,c,x,n,tch,tst)
  int           s,c,x,*n;
  wchar_t       *tch;
  int           *tst;
{
    nextstate(s,c,x);
    /*
     * executed for each state, transition pair
     */
    xstate = notin(stnum);
    if(xstate == -2)
        warning(MSGSTR(BADSTATE, "bad state  %d %o"),s,c);
    else if(xstate == -1)
    {
        if(stnum >= nstates)
        {
            error(MSGSTR(MANYSTATES,"Too many states %s"),
                  ((nstates == NSTATES) ? MSGSTR(MANYSTATES2,"\nTry using %%n num") :""));
        }
        add(state,++stnum);
# ifdef DEBUG
        if(debug)
            pstate(stnum);
# endif
        /*
         * tch = "c" for a char and "\0x" for a CCL; if this is a CCL, then c
         * is already \0 so just copy the c in, with this state, and then the
         * CCL in, again with this state
         */
        tch[*n] = c;
        tst[(*n)++] = stnum;
        if (x)
        {
            tch[*n] = x;
            tst[(*n)++] = stnum;
        }
    }
    else                                        /* xstate >= 0 ==> state exists */
    {
        tch[*n] = c;
        tst[(*n)++] = xstate;
        if (x)
        {
            tch[*n] = x;
            tst[(*n)++] = xstate;
        }
    }
}

/* ------------------------- nexstate ------------------------- */

/*
 *    1. For each of the positions associated with the given
 *       state s (pointed to by state[s]), determine if
 *       the character c, belongs to that position.  If it
 *       does, then get the set of follow positions for
 *       that position.  Store the follow positions in tmpstat[]
 *       such that at the end, tmpstat[] contains the union of
 *       the follow positions for each position associated with
 *       the given state and the given character.
 *
 * inputs -  s      : state number
 *           c      : character
 *           x      : xccl type (0 if none)
 * outputs - none
 * globals - tmpstat: array of flags indicating which positions are
 *                    active, in this case, which positions belong
 *                    to the union of follow positions
 *           state  : array of pointers to the list of positions
 *                    associated with each state
 *           foll   : array of pointers to the list of follow positions
 *                    associated with each position
 */

/*
 * Beware -- 70% of total CPU time is spent in this subroutine -
 * If you don't believe me - try it yourself !
 */

nextstate(s,c,x)
  int s,c,x;
{
register int    j, *newpos;
wchar_t         *temp, *tz;
int             *pos, i, *f, num, curpos, number;

    num = *state[s];                            /* number of positions in this state */
    temp = tmpstat;
    pos = state[s] + 1;                         /* list of names associated with these positions */
    /*
     * for each name associated with a position in this state
     */
    for(i = 0; i<num; i++)
    {
        curpos = *pos++;
        j = name[curpos];
        if(   (j < NCH && j == c)
           || (j == RWCHAR && wname[curpos] == c)
           || (   (j == RSTR)
               && (   (c == right[curpos])
                   || (right[curpos] == RWCHAR && c == wname[curpos])))
           || (   (j == RCCL)
               && (   (c && member(c,0,left[curpos]))
                   || (x && member(0,x,left[curpos]))))
           )
        {
            f = foll[curpos];
            number = *f;
            newpos = f+1;
            for(j=0;j<number;j++)
                temp[*newpos++] = 2;
        }
    }
    j = 0;
    tz = temp + tptr;
    while(temp < tz)
    {
        if(*temp == 2)
        {
            j++;
            *temp++ = 1;
        }
        else
            *temp++ = 0;
    }
    count = j;
    return;
}

/* -------------------------- notin -------------------------- */

/*
 *    1. Check that the state in tmpstat[] does not already
 *       exist by comparing it to each of the set of positions
 *       pointed to by state[].  The set of positions associated
 *       with a state is what defines a state.
 *
 * inputs -  n      : state number
 * outputs - matching state number: returns the number of the state
 *                    which is identical to tmpstat[] if one is found
 *           -1     : if no identical state is found, -1 is returned
 * globals - tmpstat: array of flags indicating which positions are
 *                    active for the current state
 *           state  : array of pointers to the list of positions
 *                    associated with each state
 */

notin(n)
  int n;
{
    register int        *j,k;
    wchar_t             *temp;
    int         i;
    
    if(count == 0)
        return(-2);
    temp = tmpstat;
    for(i=n;i>=0;i--)                           /* for each state */
    {
        j = state[i];
        if(count == *j++)
        {
            for(k=0;k<count;k++)
                if(!temp[*j++])
                    break;
            if(k >= count)
                return(i);
        }
    }
    return(-1);
}

/* ------------------------- packtrans ------------------------- */

/*
 *  IF tryit is true, ie. if there are CCLs for this state
 *  then do the following:
 *
 *    1. Load go[] and wsymbol[] with the information currently
 *       in tch[] and tst[].
 *
 *    2. Load temp[] with the transition states for the initial
 *       CCL characters only (for 8 bit characters).  For example,
 *       if the CCL is a, b, c then set temp['a'] to the transition
 *       state and set temp['b'] and temp['c'] to -1.
 *
 *    3. Turn off all but the first character of each CCL in
 *       wsymbol[].
 *
 *    4. All unprocessed 8 bit characters get error entry (-2).
 *
 *    5. Determine if compression is obtainable by checking if the
 *       number of characters left "on" after 2 and 3 is less than
 *       the total number of characters in tch[].
 *
 *    6. If compression is obtainable then:
 *          a) Compress the information in temp[] and wsymbol[]
 *             into cwork[] and swork[] such that we have a reduced
 *             form of tch[] and tst[] which contains only initial
 *             CCL characters and non CCL characters.
 *          b) Set ach[] to point to cwork[] instead of tch[].
 *          c) Set ast[] to point to swork[] instead of tst[].
 *          d) Reset the character count (cnt) to the reduced total
 *             number of characters in cwork[].
 *          e) Set cpackflg[] to true for the given state number.
 *
 *  ENDIF (tryit)
 *
 *    7. Sort ach[] and ast[] based on the character values in ach[]
 *       to facilitate looking for similar states.
 *
 *    8. Loop through states to find a similar state to the current
 *       one.
 *
 *    9. If a similar state is found, set sfall[<current state>]
 *       to the similar state number and copy into nchar[] and
 *       nexts[] for the current state only that information which
 *       is different from the similar state.  Otherwise copy all
 *       the state information from ach[] and ast[] into nchar[]
 *       and nexts[] respectively.
 *
 *
 * inputs -  st     : current state number
 *           tch    : character array containing the characters
 *                    which have a transition for the current state
 *           tst    : array of transition states for the associated
 *                    characters in tch[]
 *           cnt    : count of the number of characters in tch[]
 *           tryit  : flag to indicate if some of the characters in
 *                    in tch[] belong to CCLs
 * outputs - none
 * globals - sfall  : contains the fall back state for each state, -1
 *                    if there is none
 *           cpackflg: array of flags indicating which states are
 *                    packed
 *           nchar  : for each state, contains the set of characters
 *                    for which there is a transition for that state
 *                    (permanent storage for the information in tch)
 *           nexts  : contains the transition states associated with
 *                    the characters in nchar[]
 *                    (permanent storage for the information in tst)
 *           gotof  : array of indexes into nchar and nexts for each
 *                    state
 *           nptr   : next available slot in nchar and nexts
 *           ntrans : maximum number of transitions
 */

packtrans(st,tch,tst,cnt,tryit)
  int           st,
                *tst,                           /* state information */
                cnt,                            /* number of states */
                tryit;
  wchar_t       *tch;                           /* character associated with state info */
{
int             cmin, cval, tcnt, diff, p, *ast;
register int i,j,k,n;
wchar_t *ach;
int             go[NCH], temp[NCH], c;
int             *wgo;
int             *swork;                     /* \                                            */
wchar_t *cwork;                             /*  > WARNING: NCH may not be enough roooooooom */
int             ssize = NCH;
int             csize = NCH;
int             upper;

    swork = myalloc(ssize, sizeof(int));
    cwork = myalloc(csize, sizeof(wchar_t));
    if (!swork || !cwork)
    {
        error(MSGSTR(CALLOCFAILED,"There is not enough memory available."));
    }
    rcount += cnt;
    cmin = -1;
    cval = -1;
    ast = tst;
    ach = tch;
    /*
     * try to pack transitions using ccl's
     */
    if(!optim)                                  /* skip all compaction */
        goto nopack;                    
    if(tryit)                                   /* ccl's used */
    {
        for(i=1;i<NCH;i++)
        {
            go[i] = temp[i] = -1;
            symbol[i] = 1;
        }
        wsymboli = 0;
        /*
         * record the next state for each character
         * - go will contain the 8-bit transitions
         * - wsymbol will contain the indices (+1 to allow a 0 to signify
         *   emptiness) of the MB characters and special CCLs in the character set.
         */
        for(i=0;i<cnt;i++)                      /* record the next state for each char */
        {
            if (tch[i] && tch[i]<NCH)           /* 8-bit */
            {
                go[tch[i]] = tst[i];
                symbol[tch[i]] = 0;
            }
            else if (tch[i])                    /* MB */
                addwsymbol ((wchar_t)(i+1),1);
            else                                /* CCL or end */
            {
                if (tch[i+1])                   /* CCL */
                    addwsymbol ((wchar_t)(i+1),1);
                i += 1;                         /* ...skip the \0 marker */
            }
        }
        /*
         * record the first chracter of a CCL set
         * - temp will contain the next state for the initial 8-bit CCL chars
         * - wsymbol will leave only the first index of a MB or CCL set as non-zero
         */
        for(i = 0, n = 0; i < cnt; i++)         /* record the next state for each initial CCL char */
        {
            if (tch[i])
            {
                if (tch[i] < NCH)               /* 8-bit */
                {
                    c = match[tch[i]];
                    if(go[c] != tst[i] || c == tch[i])
                        temp[tch[i]] = tst[i];
                }
                else if (n < wsymboli)          /* MB */
                {
                    if ((c = hashfind(tch[i],wmatch)) != 0)
                    {
                        if (PMATCH(c)->cmatch != tch[i]) /* CCL mapping defined */
                        {
                            for (k = 0; k < n; k++)
                            {
                                if (   (wsymbol[k] != 0) /* not already erased */
                                    && (PMATCH(c)->cmatch == tch[wsymbol[k]-1]) /* mapped to a symbol already seen */
                                    && (tst[i] == tst[wsymbol[k]-1])) /* same next state */
                                {
                                    break;
                                }
                            }
                            if (k < n)          /* previously found, so erase */
                                wsymbol[n] = 0;
                        }
                    }
                    n += 1;                     /* we are done with this element in wsymbol */
                }
            }
            else if (n < wsymboli)              /* xCCL */
            {
                if (tch[i+1] != 0)              /* not the end marker - \0\0 */
                    n += 1;                     /* ...skip the element in wsymbol */
                i += 1;                         /* skip the \0 marker in tch */
            }
        } /* for */
        /*
         * all unprocessed 8-bit chars receive an error entry value
         */
        for(i=1;i<NCH;i++)
        {
            if(symbol[i])                       /* error trans */
                temp[i] = -2;
        }
        /*
         * determine the amount of compression obtainable
         * - only non (-1) 8-bit entries
         * - only non (0) MB entries
         */
        k = 0;
        for (i = 1; i < NCH; i++)               /* count how many 8-bit chars left */
        {
            if(temp[i] != -1)
                k++;
        }
        for (i = 0; i < wsymboli; i++)          /* count off the MB chars left */
        {
            if (wsymbol[i] != 0)
                k++;
        }
        /*
         * if there is some compression obtainable
         * - copy the remaining 8-bit entries from tch and tst to cwork and
         *   swork respectively
         * - copy the remaining MB entries, through the non-zero index (less 1) 
         *   remaining in the wsymbol list
         */
        if(k < cnt)                             /* compress by char */
        {
# ifdef DEBUG
            if(debug)
                printf("use compression  %d,  %d vs %d\n",st,k,cnt);
# endif
            k = 0;
            for(i=1;i<NCH;i++)                  /* compress CCL into cwork(tch) and swork(tst) */
            {
                if(temp[i] != -1)
                {
                    cwork[k] = i;
                    swork[k++] = (temp[i] == -2 ? -1 : temp[i]);
                }
            }
            for (i = 0; i < wsymboli; i++)      /* compress the MB chars and CCLs in */
            {
                if (wsymbol[i] != 0)
                {
                    if (k+3 >= csize)
                    {
                        csize = 2*csize;
                        cwork = (wchar_t *)realloc(cwork, csize * sizeof(wchar_t));
                    }
                    if (k+3 >= ssize)
                    {
                        ssize = 2*ssize;
                        swork = (int *)realloc(swork, ssize * sizeof(int));
                    }
                    if (!swork || !cwork)
                    {
                        error(MSGSTR(CALLOCFAILED,"There is not enough memory available."));
                    }
                    cwork[k] = tch[wsymbol[i]-1];
                    swork[k++] = tst[wsymbol[i]-1];

                    if (!tch[wsymbol[i]-1])     /* CCL */
                    {
                        cwork[k] = tch[wsymbol[i]];
                        swork[k++] = tst[wsymbol[i]];
                    }
                }
            }
            cwork[k] = 0;                       /* terminate the list */
            swork[k] = -1;
            cwork[++k] = 0;                     /* terminate the list */
            swork[k++] = -1;
# ifdef PC
            ach = cwork;
            ast = swork;
            cnt = k - 2;                        /* ignore the terminators */
            cpackflg[st] = TRUE;
# endif
        }
    }
    /*
     * we need to sort ach (with ast) so that when we look for similar states,
     * the chars are sorted by their process code value, followed with the CCLs
     * at the end;  this makes it possible to compare state chars in some sort
     * of consistent manner.
     */
    if (wmatchsize || xccltop)                  /* only if there is a requirement */
        packsort(ach,ast,cnt);
    /*
     * get most similar state, reject state with more transitions,
     * state already represented by a third state, and state which is compressed
     * by char if ours is not to be
     */
    for(i=0; i<st; i++)
    {
        /*
         * reject test state with more transitions
         */
        if(sfall[i] != -1)
            continue;
        /*
         * reject test state if new state is packed and test state is not
         */
        if(cpackflg[st] == 1)
        {
            if(!(cpackflg[i] == 1))
                continue;
        }
        /*
         * reject test state with no transition chars
         */
        p = gotof[i];
        if(p == -1)
            continue;
        /*
         * reject test state with excess nchars (it's bigger)
         */
        tcnt = nexts[p];
        if(tcnt > cnt)
            continue;
        /*
         * Determine how many new chars are different from those in the test
         * state by NOT COUNTING new chars whose value and transitional
         * qualities are related to those of the test char(s)
         *
         * Because we sorted ach (and consequently previous nchar groups),
         * 8-bits appear first, MBs second, and CCLs last.  Using this, we can
         * decide similarities by traversing and comparing the char sets in
         * their current order.
         */
        diff = 0;
        j = 0;
        upper = p + tcnt;
        while ((j < cnt) && (p < upper))
        {
            /*
             * find a new char which is in the test char set; each set is
             * sorted, so we only need to look as far as all of the new chars,
             * or the first new char which is greater than the current test
             * char
             */
            if (!nchar[p])                      /* CCLs have a \0 marker */
                tcnt -= 1;                      /* ...which shouldn't be included in the tcnt */
            while (   (j < cnt)                                                 /* not done */
                   && (   ( ach[j] &&  nchar[p] && (ach[j]   < nchar[p]))       /* CH  < CH  */
                       || ( ach[j] && !nchar[p])                                /* CH  < CCL */
                       || (!ach[j] && !nchar[p] && (ach[j+1] < nchar[p+1]))))   /* CCL < CCL */
            {
                diff++;
                j += NEXT(ach,j);               /* skip to next element */
            }
            if (j >= cnt)                       /* all state chars different and counted */
                break;
            /*
             * no state chars represented in test state set
             */
            if (   (ach[j] > nchar[p])                          /* CH  > ?   */
                || (!ach[j] && (   nchar[p]                     /* CCL > CH  */
                                || (ach[j+1] > nchar[p+1]))))   /* CCL > CCL */
            {
                diff = -1;                      /* reject test state */
                break;
            }
            /***
             * >>>>>>>>>> ach[j] == nchar[p] <<<<<<<<<<<
             * the new char is different from the current test char if:
             * - the state for char or CCL is different than state for the next test char or CCL
             * - there are no transitions for new char
             * - the new state is packed and the new char does NOT map to itself
             */
            n = (ach[j] >= NCH) ? hashfind(ach[j],wmatch) : 0;
            p += NEXT(nchar,p);                 /* skip to next element */

            if(   (ast[j] != nexts[p])
               || (ast[j] == -1)
               || (    cpackflg[st]
                   && (ach[j]
                       && (   ((ach[j] < NCH) && (ach[j] != match[ach[j]]))
                           || ((n != 0) && (ach[j] != wmatch->table[n].id))))))
            {
                    diff++;
            }
            j += NEXT(ach,j);                   /* done checking this new char */
        }
        /*
         * count off the new characters which did not get compared to the test
         * chars
         */
        if (diff != -1)
        {
            while (j < cnt)
            {
                diff++;
                j += NEXT(ach,j);               /* skip to next element */
            }
        }
        /*
         * reject state if all of the test chars did not get a peek
         */
        if(p < upper)
            diff = -1;
        /*
         * if there were fewer differences to this test state, and there were a
         * few checks that did match, then record this test state as the most
         * similar
         */
        if(   (diff != -1)
           && ((diff < cval) || (cval == -1))
           && (diff < tcnt))
        {
            cval = diff;
            cmin = i;
            if(cval == 0)                       /* can't get any more similar than identical */
                break;
        }
    }
    /*
     * cmin = state "most like" state st
     */
# ifdef DEBUG
    if(debug)printf("select st %d for st %d diff %d\n",cmin,st,cval);
# endif
# ifdef PS1
    if(cmin != -1)                              /* if we can use st cmin */
    {
        /*
         * If cmin has a transition on c, then so will st st may be
         * "larger" than cmin, however.
         *
         * Copy into the new state only information which is different from the
         * similar state.
         */
        gotof[st] = nptr;
        k = 0;
        sfall[st] = cmin;
        p = gotof[cmin];
        j = 0;
        while (j < cnt)
        {
            /*
             * copy over to a potential similarity
             */
            while (   (j < cnt)                                                 /* not done */
                   && (   ( ach[j] &&  nchar[p] && (ach[j]   < nchar[p]))       /* CH  < CH  */
                       || ( ach[j] && !nchar[p])                                /* CH  < CCL */
                       || (!ach[j] && !nchar[p] && (ach[j+1] < nchar[p+1]))))   /* CCL < CCL */
            {
                nchar[nptr] = ach[j];
                nexts[++nptr] = ast[j];
                if (!ach[j])                    /* CCL */
                {
                    nchar[nptr] = ach[j+1];
                    nexts[++nptr] = ast[j+1];
                    k++;
                    j++;
                }
                k++;
                j++;
                
                if (nptr>ntrans)
                    goto nptr_error;
            }
            /*
             * no more to do
             */
            if (!nchar[p] && !nchar[p+1])
                break;
            /*
             * if this is true, then nchar contained a char which does not
             * transition as expected.
             */
            if (   (ach[j] > nchar[p])                          /* CH  > ?   */
                || (!ach[j] && (   nchar[p]                     /* CCL > CH  */
                                || (ach[j+1] > nchar[p+1]))))   /* CCL > CCL */
            {
                warning("bad transition %d %d",st,cmin);
                goto nopack;
            }
            /***
             * >>>>>>>>>>>>> ach[j] == nchar[p] <<<<<<<<<<<<<<<<<
             * the new char is different from the current test char if:
             * - the state for char or CCL is different than state for the next test char or CCL
             * - there are no transitions for new char
             * - the new state is packed and the new char does NOT map to itself
             * so copy in the state information
             */
            n = (ach[j] >= NCH) ? hashfind(ach[j],wmatch) : 0;
            p += NEXT(nchar,p);                 /* skip to next element */

            if(   (ast[j] != nexts[p])
               || (ast[j] == -1)
               || (    cpackflg[st]
                   && (ach[j]
                       && (   ((ach[j] < NCH) && (ach[j] != match[ach[j]]))
                           || ((n != 0) && (ach[j] != wmatch->table[n].id))))))
            {
                nchar[nptr] = ach[j];
                nexts[++nptr] = ast[j];
                if (!ach[j])
                {
                    nchar[nptr] = ach[j+1];
                    nexts[++nptr] = ast[j+1];
                    k++;
                }
                if (nptr>ntrans)
                    goto nptr_error;
                k++;
            }
            j += NEXT(ach,j);
        }
        /*
         * we have passed all the similarities, so copy over what's left.
         */
        while (j < cnt)
        {
            nchar[nptr] = ach[j];
            nexts[++nptr] = ast[j];
            if (!ach[j])
            {
                nchar[nptr] = ach[j+1];
                nexts[++nptr] = ast[j+1];
                k++;
            }
            if (nptr>ntrans)
                goto nptr_error;
            k++;
            j += NEXT(ach,j);
        }
        nexts[gotof[st]] = cnt = k;
    }
    else
    {
# endif
nopack:
        /* stick it in */
        gotof[st] = nptr;
        nexts[nptr] = cnt;
        for(i=0;i<cnt;i++)
        {
            nchar[nptr] = ach[i];
            nexts[++nptr] = ast[i];
            if (nptr>ntrans)
                goto nptr_error;
        }
# ifdef PS1
    }
    /*
     * end the nchar list with \0\0
     */
    nchar[nptr] = 0;
    nexts[++nptr] = -1;
    nchar[nptr] = 0;
    nexts[++nptr] = -1;
    if (nptr>ntrans)
        goto nptr_error;
# endif
    if(cnt < 1)
    {
        gotof[st] = -1;
        nptr--;
    }
    else
    {
        if(nptr > ntrans)
        {
nptr_error:
            error(MSGSTR(ENTRANS, "Too many transitions %s"),
                  (ntrans==NTRANS ? MSGSTR(ENTRANS1,"\nTry using %%a num") : ""));
        }
    }
    return;
}

/* ------------------------ packsort ------------------------ */

/*
 *    1. Called from packtrans() to sort the list of characters
 *       and transitions associated with a state (ach[] and ast[])
 *       based on the character values.
 *
 * inputs -  ach    : character array containing the characters
 *                    which have a transition for the current state
 *           ast    : array of transition states for the associated
 *                    characters in ach[]
 *           cnt    : count of the number of characters in ach[]
 * outputs - none
 * globals - none
 */

packsort (ach, ast, cnt)
  wchar_t       *ach;
  int           *ast;
  int            cnt;
{
register int    i, j, k;
int             *n, *t;

    if (!cnt)                                   /* nothing to sort */
        return;

    n = (int *)myalloc(cnt,sizeof(*n));
    t = (int *)myalloc(cnt,sizeof(*t));
    if ((n == 0) || (t == 0))                   /* no room */
        error (MSGSTR (CALLOCFAILED, "No space for state char sorting"), 0);
    /*
     * make a list of indices which refer to each element in ach and sort the
     * array of indices based on what they refer to in ach
     */
    for (i = 0, j = 0; i < cnt; i++)
    {
        n[j++] = i;
        if (!ach[i])
            i += 1;
    }
    __packarray = ach;                          /* globalize the char array for packcmp */
    qsort (n, j, sizeof(*n), packcmp);          /* sort the list of indices */
    /*
     * copy into ast the ordered list of ast based on the sorted array of indices
     */
    for (i = 0, k = 0; i < j; i++)
    {
        t[k++] = ast[n[i]];
        if (!ach[n[i]])
            t[k++] = ast[n[i]+1];
    }
    for (i = 0; i < k; i++)
        ast[i] = t[i];
    /*
     * copy into ach the ordered list of ach based on the sorted array of indices
     */
    for (i = 0, k = 0; i < j; i++)
    {
        t[k++] = ach[n[i]];
        if (!ach[n[i]])
            t[k++] = ach[n[i]+1];
    }
    for (i = 0; i < k; i++)
        ach[i] = t[i];

    cfree ((void *)t,cnt,sizeof(*t));
    cfree ((void *)n,cnt,sizeof(*t));
}

/* ------------------------ packcmp ------------------------ */

/*
 *    1. Comparison routine for qsort.
 */

int
packcmp (a,b)
  int *a, *b;
{
    if (   (__packarray[*a] && __packarray[*b] && (__packarray[*a] < __packarray[*b]))
        || (__packarray[*a] && !__packarray[*b])
        || (!__packarray[*a] && !__packarray[*b] && (__packarray[*a+1] < __packarray[*b+1])))
    {
        return (-1);
    }
    if (   (__packarray[*a] == __packarray[*b])
        && (   __packarray[*a]
            || (__packarray[*a+1] == __packarray[*b+1])))
    {
        return (0);
    }

    return (1);
}

# ifdef DEBUG
/* ------------------------ pstate ------------------------ */

/*
 *    1. Debug code - print the set of positions associated
 *       with the given state (s).
 */

pstate(s)
  int s;
{
register int *p,i,j;

    printf("State %d:\n",s);
    p = state[s];
    i = *p++;
    if(i == 0)
        return;

    printf("%4d",*p++);
    for(j = 1; j<i; j++)
    {
        printf(", %4d",*p++);
        if(j%30 == 0)putchar('\n');
    }
    putchar('\n');
    return;
}
# endif

/* ------------------------ member ------------------------ */

/*
 *    1. Determine whether or not the char (d) or special
 *       CCL (x) is described in the set of all CCLs identified
 *       by the list of cindex values contained in t.
 */

member(d,x,t)
  int           d,x;                            /* thing to look for */
  wchar_t       *t;                             /* set to look in */
{
register int    c;
wchar_t         *s;
    /*
     * determine the cindex of the test char is
     */
    c = 0;
    if (!x && (d < NCH))                        /* 8-bit */
        c = cindex[d];
    else if (!x)                                /* MB */
    {
        c = hashfind (d, wmatch);
        c = PMATCH(c) ? PMATCH(c)->cindex : 0;
    }
    /*
     * if a cindex is defined for the char, then see if the cindex found is in
     * the test list
     */
    if (c)
    {
        for (s = t; *s; s++)
        {
            if (*s == c)                        /* found */
                return(1);
        }
    }
    /**
     * the specific mappings of the char did not indicate that it is not in the
     * test list
     * - look at each of the special CCLs which have a cindex described in the
     *   test list
     * - if the located special CCL describes the specific character or the
     *   specific special CCL, then it is a member
     */
    for (c = 0; c < xccltop; c++)
    {
        for (s = t; *s; s++)
        {
            if (   (*s == xccl[c].cindex)               /* xCCL in test list */
                && (   (!x && inCCL (d,xccl[c].type))   /* char(d) in xCCL type */
                    || ( x && (x == xccl[c].type))))    /* xCCL(x) == xCCL type */
            {
                return (1);
            }
        }
    }
    return (0);                                 /* got here, then not a member */
}

# ifdef DEBUG
/* ------------------------- stprt ------------------------- */

/*
 *    1. Debug code - print the characters, transitions and actions
 *       associated with the given state (i) as well as the fall
 *       back state number if there is one, and whether
 *       or not the state is final.
 *    2. Called from main to print the final state machine.
 */

stprt(i)
  int i;
{
register int    j, k, p, t;

    printf("State %d:",i);
    /*
     * print actions, if any
     */
    t = atable[i];
    if(t != -1)
        printf(" final");
    putchar('\n');

    if(cpackflg[i] == TRUE)
        printf("backup char in use\n");
    if(sfall[i] != -1)
        printf("fall back state %d\n",sfall[i]);

    p = gotof[i];
    if(p == -1)
        return;
    for (j = p, k = 0; (nchar[j] || nchar[j+1]); k++)
        j += NEXT(nchar,j);
    printf("(%d transition%s)\n",k, (k==1?"":"s"));

    while (nchar[p] || nchar[p+1])
    {
        charc = 0;
        if(nexts[p+NEXT(nchar,p)] >= 0)
            printf("%d\t",nexts[p+NEXT(nchar,p)]);
        else
            printf("err\t");
        if (nchar[p])
            allprint(nchar[p++]);
        else
        {
            printf (" [:%#4.4x:]", nchar[++p,p++]);
            charc += 9;
        }
        while(nexts[p] == nexts[p+NEXT(nchar,p)] && (nchar[p] || nchar[p+1]))
        {
            if(charc > LINESIZE)
            {
                charc = 0;
                printf("\n\t");
            }
            if (nchar[p])
                allprint(nchar[p++]);
            else
            {
                printf (" [:%#4.4x:]", nchar[++p,p++]);
                charc += 9;
            }           
        }
        putchar('\n');
    }
    putchar('\n');
    return;
}
# endif

/* ------------------------- acompute ------------------------- */

/*
 *    1. For all the positions associated with the given state,
 *       if the position is a final position, get the action
 *       number from left[<position #>].
 *    2. Called from cgoto(), cgoto() first prints out:
 *              int yyvstop[] = {
 *                   0
 *       to lex.yy.c and then calls acompute for each state
 *       which will print out the case numbers associated with
 *       the final actions for that state.
 *    3. The offset of the start of the actions in yyvstop[] is
 *       stored in atable[<state #>] (-1 if it is not a final
 *       state).  The variable "aptr" contains the current
 *       position in yyvstop[].
 *
 * inputs -  s      : state number
 * outputs - none
 * globals - atable : (one element for each state), contains the offset
 *                    in yyvstop[] of the set of actions for each state,
 *                    used for laying out yysvec[]
 *           aptr   : keeps track of the currect position in yyvstop[]
 *           state  : contains for each state a pointer to the set of
 *                    positions associated with that state
 */

acompute(s)     
int s;
{
register int    *p, i, j;
int             cnt, m;
int             *temp, k, *neg, n;

    k = 0;
    n = 0;
    p = state[s];
    cnt = *p++;
    atable[s] = -1;
    if (cnt <= 0)
        return;
    temp = (int *)calloc(sizeof(int), cnt);
    neg = (int *)calloc(sizeof(int), cnt);
    for(i=0;i<cnt;i++)
    {
        if (name[*p] == FINAL)
            temp[k++] = left[*p];
        else if (name[*p] == S1FINAL)
        {
            temp[k++] = left[*p];
            if (left[*p] >nactions)
            {
                int save_size = nactions;
                nactions += NACTIONS;
                extra = (wchar_t *) realloc(extra, nactions * sizeof(wchar_t));
                if (!extra)
                    error(MSGSTR(ERTEXTS, "Too many right contexts"));
                /* D55916 - changed memset() casts from char to wchar_t. */
                memset(extra+save_size, (wchar_t)NULL, NACTIONS * sizeof(wchar_t));
            }
            extra[left[*p]] = 1;
        }
        else if (name[*p] == S2FINAL)
            neg[n++] = left[*p];
        p++;
    }

    if(k < 1 && n < 1) {
        cfree((void *)temp, cnt, sizeof(int));
        cfree((void *)neg, cnt, sizeof(int));
        return;
    }

# ifdef DEBUG
    if(debug) printf("final %d actions:",s);
# endif
        
    for(i=0; i<k; i++)                          /* sort action list */
    {
        for(j=i+1;j<k;j++)
        {
            if(temp[j] < temp[i])
            {
                m = temp[j];
                temp[j] = temp[i];
                temp[i] = m;
            }
        }
    }

    for(i=0;i<k-1;i++)                          /* remove dups */
        if(temp[i] == temp[i+1]) temp[i] = 0;
        
    atable[s] = aptr;                           /* copy to permanent quarters */
# ifdef DEBUG
    if(!ratfor) fprintf(fout,"/* actions for state %d */",s);
# endif
    putc('\n',fout);
    for(i=0;i<k;i++)
    {
        if(temp[i] != 0)
        {
            ratfor
                ? fprintf(fout,"data vstop(%d)/%d/\n",aptr,temp[i])
                : fprintf(fout,"%d,\n",temp[i]);
# ifdef DEBUG
            if(debug) printf("%d ",temp[i]);
# endif
            aptr++;
        }
    }
    for(i=0;i<n;i++)                            /* copy fall back actions - all neg */
    {
        ratfor
            ? fprintf(fout,"data vstop(%d)/%d/\n",aptr,neg[i])
            : fprintf(fout,"%d,\n",neg[i]);
        aptr++;
# ifdef DEBUG
        if(debug)printf("%d ",neg[i]);
# endif
    }
# ifdef DEBUG
    if(debug)putchar('\n');
# endif
    ratfor
        ? fprintf(fout,"data vstop (%d)/0/\n",aptr)
        : fprintf(fout, "0,\n");
    aptr++;
    cfree((void *)temp, cnt, sizeof(int));
    cfree((void *)neg, cnt, sizeof(int));
    return;
}

# ifdef DEBUG
/* --------------------------- pccl --------------------------- */

/*
 *    1. Debug code - print the final set of character classes
 *       as computed by mkmatch().
 */

pccl()
{
register int    i, j;
int             f;
char            t[16];

    f = 0;
    printf("char class intersection\n");
    for(i=0; i < ccount; i++)
    {
        printf("class %d:",i);
        charc = LINESIZE+1;
        for(j=1;j<NCH;j++)
        {
            if(cindex[j] == i)
            {
                if(charc > LINESIZE)
                {
                    printf("\n\t");
                    charc = 0;
                }
                allprint(j);
            }
        }
        charc = LINESIZE+1;
        for (j = wmatchlist; j != 0; j = PMATCH(j)->list)
        {
            if (PMATCH(j)->cindex == i)
            {
                if (charc > LINESIZE)
                {
                    printf ("\n\t");
                    charc = 0;
                }
                allprint(wmatch->table[j].id);
            }
        }
        charc = LINESIZE+1;
        for (j = 0; j < xccltop; j++)
        {
            if (xccl[j].cindex == i)
            {
                if (charc > LINESIZE)
                {
                    printf ("\n\t");
                    charc = 0;
                }
                sprintf (t, "[:%#4.4x:]", xccl[j].type);
                printf (t);
                charc += strlen(t);
            }
        }
        if (charc)
            putchar('\n');
    }

    charc = 0;
    printf("match:\n");
    for(i=0;i<NCH;i++)
    {
        allprint(match[i]);
        if(charc > LINESIZE)
        {
            putchar('\n');
            charc = 0;
        }
    }
    putchar('\n');

    if (wmatchlist)
    {
        charc = 0;
        printf ("wmatch:\n");
        for (i = wmatchlist; i; i = PMATCH(i)->list)
        {
            allprint(wmatch->table[i].id);
            printf ("->");
            allprint(PMATCH(i)->cmatch);
            putchar(',');
            charc += 3;
            if(charc > LINESIZE)
            {
                putchar('\n');
                charc = 0;
            }
        }
        putchar('\n');
    }

    return;
}
# endif

/* ------------------------- mkmatch ------------------------- */

/*
 *    1. Creates the match table for 8 bit and multibyte
 *       characters by setting each character in the CCL
 *       to the representative character for that CCL.
 *    2. Stores the representative 8 bit characters in match[]
 *       and the representative multibyte characters in the
 *       hash table wmatch[].
 *
 * inputs -  none
 * outputs - none
 * globals - match  : match table for 8 bit characters
 *           wmatch : match table (hash table) form multibyte characters
 */

mkmatch()
{
register int    i, j, k;
int             newlist;
wchar_t         *tab;

    if ((tab = (wchar_t *)myalloc(ccount,sizeof(*tab))) == 0)
        error (MSGSTR(CALLOCFAILED,"No space for match table generation"),0);

    for (i = 0; i < ccount; i++)
        tab[i] = 0;
    /*
     * record the first char of each CCL into tab
     * tab[i] = principal char for new ccl i
     * note: special ccls (xccl) are left out as they need to be handled through
     *       a mechinism which does not use representative characters.
     */
    for (i = 1; i < NCH; i++)                   /* 8-bit chars */
    {
        if (tab[cindex[i]] == 0)                /* tab not defined for CCL */
            tab[cindex[i]] = i;                 /* ... define its first char */
    }
    for (i = wmatchlist; i != 0; i = PMATCH(i)->list) /* MB */
    {
        if (   (tab[PMATCH(i)->cindex] == 0)    /* tab not defined for CCL */
            || (tab[PMATCH(i)->cindex] > wmatch->table[i].id)) /* or is lesser in value */
        {
            tab[PMATCH(i)->cindex] = wmatch->table[i].id; /* ...define first char */
        }
    }
    /*
     * record the principal char of each CCL into match
     */
    for (i = 1; i < NCH; i++)                   /* 8-bit */
        match[i] = tab[cindex[i]];
    for (i = wmatchlist; i != 0; i = PMATCH(i)->list) /* MB */
        PMATCH(i)->cmatch = tab[PMATCH(i)->cindex];
    /***
     * sort the wmatchlist to speed up later table manipulations
     * - this list is relatively small, and shouldn't suck up too much time to do.
     * - however, this really is a pathetic sorting algorithm
     */
    if (wmatchlist)
    {
        newlist = wmatchlist;                   /* start new list */
        wmatchlist = PMATCH(newlist)->list;     /* old list to sort */
        PMATCH(newlist)->list = 0;              /* end new list */

        while (wmatchlist)                      /* for each element in the old list */
        {
            i = newlist;
            j = 0;                              /* find where to put it in the new list */
            while (i && (wmatch->table[wmatchlist].id > wmatch->table[i].id))
            {
                j = i;
                i = PMATCH(i)->list;
            }
            k = wmatchlist;                     /* set up next sort */
            wmatchlist = PMATCH(k)->list;
            if (j == 0)                         /* new beginning */
            {
                PMATCH(k)->list = newlist;
                newlist = k;
            }
            else if (i != 0)                    /* middle */
            {
                PMATCH(k)->list = PMATCH(j)->list;
                PMATCH(j)->list = k;
            }
            else                                /* end */
            {
                PMATCH(k)->list = 0;
                PMATCH(j)->list = k;
            }
        }
        wmatchlist = newlist;
    }
    /*
     * done with workspace
     */
    cfree((void *)tab,ccount,sizeof(*tab));
    return;
}

/* -------------------------- layout -------------------------- */

/*
 * format and output final program's tables:
 *
 * YYCRANK:
 *      formed by recording transitions using the code values of the
 *      associated transitional characters as indices from a
 *      particular base (startup) position.
 * YYWCRANK:
 *      formed by recording transitions using hashed code values of
 *      the associated transitional MB characters (>256) as indices.
 *      (the 0th slot is never used as 0 is used to indicate an unused
 *      slot)
 * YYMATCH:
 *      formed by recording a single representative character for a group of
 *      characters belonging to the same character class at the code value
 *      positions in a table
 * YYWMATCH:
 *      similar to yymatch, except that the each character of the group
 *      belonging to a single character class is at a hased location in a hash
 *      table.
 */

layout()
{
register int    i, j, k, v;
int             top, bot, startup, omin;

    startup = 0;
    for(i=0; i<outsize;i++)
        verify[i] = advance[i] = 0;
    if (wcranksize)                             /* init the wcrank table */
        hashinit(wcrank);

    omin = 0;
    yytop = 0;
    for(i=0; i<= stnum; i++)                    /* for each state */
    {
        /*
         * gotof contains the base index into nchar of the characters related
         * to this state.
         */
        j = gotof[i];
        if(j == -1)                             /* no related characters */
        {
            stoff[i] = 0;
            continue;
        }
        /*
         * nchar[bot:top] contains the set of characters which will cause a
         * transition from the current to the next state through "advance".
         */
        bot = j;
        while((nchar[j]) || (nchar[j+1]))
            j += NEXT(nchar,j);
        top = j - ((((j-bot)==1)||nchar[j-2])?1:2);
# if DEBUG
        if (debug)
        {
            printf("State %d: (layout)\n", i);
            for(j=bot, k = 0; j<=top;j+=NEXT(nchar,j))
            {
                if (nchar[j])
                    printf(" %#o", nchar[j]);
                else
                    printf (" [:%#4.4x:]",nchar[j+1]);
                if ((++k%10)==0)
                    putchar('\n');
            }
            putchar('\n');
        }
# endif
        /*
         * Determine the minimum startup position for the nchars
         * associated with this state.
         */
        while(verify[omin+ZCH])
            omin++;
        startup = omin;
# if DEBUG
        if (debug) printf("bot,top %d, %d startup begins %d\n",bot,top,startup);
# endif
        /**
         * Assign the yystate.verify and yystate.advance fields:
         * - find a startup position in yystate such that each nchar associated
         *   with this state do not collide with already existing elements.
         * - assign the yystate fields
         */
        if(chset)                               /* character set is redefined */
        {
            do                                  /* find startup position */
            {
                startup += 1;
                if(startup > outsize - ZCH)
                    error(MSGSTR(OVERFL, "output table overflow"));

                for(j = bot; j<= top; j+=NEXT(nchar,j)) /* assign yystate fields */
                {
                    if (nchar[j] < NCH)         /* 8-bit */
                    {
                        k=startup+ctable[nchar[j]];
                        if(verify[k])
                            break;
                    }
                }
            } while (j <= top);                 /* j>top => have found startup */
# if DEBUG
            if (debug) printf(" startup will be %d\n",startup);
# endif
            for(j = bot; j<= top; j+=NEXT(nchar,j)) /* assign yystate fields */
            {
                if (nchar[j] && (nchar[j] < NCH)) /* 8-bit */
                {
                    if ((ctable[nchar[j]]<=0) || (ctable[nchar[j]]>=NCH))
                    {
# if DEBUG
                        if (debug) printf("j %d nchar %d ctable.nch %d\n",j, nchar[j],ctable[nchar[k]]);
# endif
                        layout8bit (startup,0,i+1,nexts[j+NEXT(nchar,j)]+1);
                    }
                    else
                        layout8bit (startup,ctable[nchar[j]],i+1,nexts[j+NEXT(nchar,j)]+1);
                }
                else if (nchar[j])              /* MB */
                    layoutMB(nchar[j],i+1,nexts[j+NEXT(nchar,j)]+1);
                else                            /* CCL */
                    layoutCCL(nchar[j+1],i+1,nexts[j+NEXT(nchar,j)]+1);
            }
        }
        else                                    /* normal character set */
        {
            do                                  /* find startup position */
            {
                startup += 1;
                if (startup > outsize - ZCH)
                    error(MSGSTR(OVERFL, "output table overflow"));
                for(j = bot; j<= top; j+=NEXT(nchar,j)) /* assign yystate fields */
                {
                    if (nchar[j] && (nchar[j] < NCH)) /* 8-bit */
                    {
                        k = startup + nchar[j];
                        if(verify[k])
                            break;
                    }
                }
            } while (j <= top);                 /* j>top => have found startup */

# if DEBUG
            if (debug) printf(" startup will be %d\n", startup);
# endif
            for(j = bot; j<= top; j+=NEXT(nchar,j)) /* assign yystate fields */
            {
                if (nchar[j] && (nchar[j] < NCH)) /* 8-bit */
                    layout8bit (startup,nchar[j],i+1,nexts[j+NEXT(nchar,j)]+1);
                else if (nchar[j])              /* MB */
                    layoutMB(nchar[j],i+1,nexts[j+NEXT(nchar,j)]+1);
                else                            /* CCL */
                    layoutCCL(nchar[j+1],i+1,nexts[j+NEXT(nchar,j)]+1);
            }
        }
        stoff[i] = startup;
    }
    /*
     * stoff[i] = offset into verify, advance for trans for state i put out yywork
     */
    if(ratfor)
    {
        fprintf(fout, "define YYTOPVAL %d\n", yytop);
        rprint(verify,"verif",yytop+1);
        rprint(advance,"advan",yytop+1);
        shiftr(stoff, stnum); 
        rprint(stoff,"stoff",stnum+1);
        shiftr(sfall, stnum); upone(sfall, stnum+1);
        rprint(sfall,"sfall",stnum+1);
        bprint(extra,"extra",casecount+1);
        bprint(match,"match",NCH);
        shiftr(atable, stnum);
        rprint(atable,"atable",stnum+1);
        return;
    }
    /*
     * put out yycrank
     */
    fprintf(fout,"# define YYTYPE unsigned %s\n",
            (stnum+1 <= 0xFF) ? "char" : (stnum+1 <= 0xFFFF) ? "short" : "long");
    fprintf(fout, "struct yywork { YYTYPE verify, advance; } yycrank[] = {\n");
    for(i=0;i<=yytop;i++)
    {
        if(verify[i])
            fprintf(fout,"\t%d,%d,",verify[i],advance[i]);
        else
            fprintf(fout,"\t0,0,");
        if (((i+1)%4)==0)
            putc('\n',fout);
    }
    fprintf(fout,"\t0,0};\n");
    /*
     * pack (top + vacancy + 2 for good luck) and output the yywcrank table iff
     * there is something to output
     */
    if (wcranksize && wcrank->top)
    {
        /*
         * pack the yywcrank hash table into top + vacancy + 2 for good luck
         */
        rehash (wcrank,((wcrank->top*100)/(100-whspace))+2);

        fprintf (fout, "# define YYHSIZE %d\n", ((wcranksize&&wcrank->top) ? wcrank->size : 0));
        fprintf (fout, "struct yywwork { wchar_t wch; unsigned int wnext; YYTYPE wverify, wadvance;} yywcrank[] = {\n");
        if (wcranksize&&wcrank->top)
        {
            for (i = 0; i < wcrank->size; i++)
            {
                if (PCRANK(i))
                {
                    fprintf (fout, "\t%d,%d,%d,%d,",
                             wcrank->table[i].id,
                             wcrank->table[i].next,
                             PCRANK(i)->verify,
                             PCRANK(i)->advance);
                }
                else
                    fprintf (fout, "\t0,0,0,0,");
                if (((i+1)%4)==0)
                    putc('\n',fout);
            }
        }
        fprintf (fout, "\t0,0,0,0};\n");
    }
    /***
     * output the yyxccl table
     * - sort it according to "verify" and move unused slots to the end
     *   (entries which were temporarily created to hold info but do not get
     *   used to record transitions.
     * - output a definition to define its size
     * - output the table itself
     */
    if (xcclsize && xccltop)
    {
        qsort (xccl, xccltop, sizeof(*xccl), xcclcmp);
        for (i = 0; (i < xccltop) && (xccl[i].verify != -1); i++)
            ; /*Empty*/
        xccltop = i;
        fprintf (fout, "# define YYXSIZE %d\n", xccltop);
        fprintf (fout, "struct yyxcclwork { YYTYPE verify, advance; char type;} yyxccl[] = {\n");

        for (i = 0; i < xccltop; i++)
        {
            fprintf (fout, "\t%d,%d,%d,", xccl[i].verify, xccl[i].advance, xccl[i].type);
            if (((i+1)%4)==0)
                putc('\n',fout);
        }
        fprintf (fout, "\t0,0,0};\n");
    }
    /*
     * put out yysvec
     */
    fprintf(fout,"struct yysvf yysvec[] = {\n");
    fprintf(fout,"0,\t0,\t0,\n");
    for(i=0;i<=stnum;i++)                       /* for each state */
    {
        if(cpackflg[i])
            stoff[i] = -stoff[i];
        fprintf(fout,"yycrank+%d,\t",stoff[i]);
        if(sfall[i] != -1)
            fprintf(fout,"yysvec+%d,\t",sfall[i]+1);/* state + 1 */
        else
            fprintf(fout,"0,\t\t");
        if(atable[i] != -1)
            fprintf(fout,"yyvstop+%d,",atable[i]);
        else
            fprintf(fout,"0,\t");
# ifdef DEBUG
        fprintf(fout,"\t\t/* state %d */",i);
# endif
        putc('\n',fout);
    }
    fprintf(fout,"0,\t0,\t0};\n");
    /*
     * Put out control information
     */
    fprintf(fout,"struct yywork *yytop = yycrank+%d;\n",yytop);
    fprintf(fout,"struct yysvf *yybgin = yysvec+1;\n");
    /*
     * put out yymatch and yywmatch, only if compression was performed.
     */
    if(optim)
    {
        fprintf(fout,"unsigned char yymatch[] = {\n");
        if (chset==0)                           /* no chset, put out in normal order */
        {
            for(i=0; i<NCH; i+=8)
            {
                for(j=0; j<8; j++)
                {
                    int fbch;
                    fbch = match[i+j];
                    if(printable(fbch) && fbch != '\'' && fbch != '\\')
                        fprintf(fout,"'%c' ,",fbch);
                    else
                        fprintf(fout,"0%-3o,",fbch);
                }
                putc('\n',fout);
            }
        }
        else
        {
            int *fbarr;
            fbarr = (int *)myalloc(2*NCH, sizeof(*fbarr));
            if (fbarr==0)
                error(MSGSTR(EREVERSE, "No space for char table reverse"),0);
            for(i=0; i<ZCH; i++)
                fbarr[i]=0;
            for(i=0; i<NCH; i++)
                fbarr[ctable[i]] = ctable[match[i]];
            for(i=0; i<ZCH; i+=8)
            {
                for(j=0; j<8; j++)
                    fprintf(fout, "0%-3o,",fbarr[i+j]);
                putc('\n',fout);
            }
            cfree((void *)fbarr, 2*NCH, 1);
        }
        fprintf(fout,"0};\n");
        /*
         * pack (top + vacancy + 2 for good luck) and output the match table
         * iff there is something to output
         */
        if (wcranksize && wcrank->top && wmatchsize && wmatch->top)
        {
            rehash (wmatch,((wmatch->top*100)/(100-whspace))+2);

            fprintf (fout, "# define YYMSIZE %d\n", ((wmatchsize&&wmatch->top) ? wmatch->size : 0));
            fprintf (fout, "struct yywmatch { wchar_t wch; unsigned int wnext; wchar_t wmatch; } yywmatch[] = {\n");
            for (i = 0; i < wmatch->size; i++)
            {
                if (PMATCH(i))
                {
                    fprintf (fout, "\t%d,%d,%d,",
                             wmatch->table[i].id,
                             wmatch->table[i].next,
                             PMATCH(i)->cmatch);
                }
                else
                    fprintf (fout, "\t0,0,0,");
                if (((i+1)%4)==0)
                    putc('\n',fout);
            }
            fprintf (fout, "\t0,0,0};\n");
        }
    }
    /*
     * put out yyextra
     */
    fprintf(fout,"unsigned char yyextra[] = {\n");
    for(i=0;i<casecount;i+=8)
    {
        for(j=0;j<8;j++)
            fprintf(fout, "%d,", i+j<nactions ? extra[i+j] : 0);
        putc('\n',fout);
    }
    fprintf(fout,"0};\n");
    return;
}

/* ------------------------ layout8bit ------------------------ */

/*
 * layout the verify and advance fields for the particular types of elements.
 */

layout8bit (s, c, v,a)
  int   s, c, v, a;
{
int     k;

    k = s + c;
    verify[k] = v;
    advance[k] = a;
    if(yytop < k)
        yytop = k;
}

/* ------------------------- layoutMB ------------------------- */

layoutMB (c, v, a)
  int   c, v, a;
{
int     k;

    k = hashnew (c, wcrank);
    if (k == 0)
    {
        error(MSGSTR(EMAXMBO, "Not enough multi-byte output slots %s"),
              MSGSTR(EMAXMBO1, "\n\tTry using the following: %%h Number"));
    }
    addwcrank (k,v,a);
}

/* ------------------------ layoutCCL ------------------------ */

layoutCCL (c, v, a)
  int   c, v, a;
{
int     k;

    /**
     * look for an unused xccl slot, preferrably with the same CCL type.
     * - the advance field is never set until the xCCL has been layed out
     */
    for (k = 0; (k < xccltop); k++)
    {
        if ((xccl[k].type == c) && (xccl[k].advance == -1))
            break;
    }
    /*
     * if none were found, then create a new one
     */
    if (k >= xccltop)
    {
        k = addxccl (-1, c);
        k -= 1;
    }
    /*
     * assign the verification and advance info
     */
    xccl[k].verify = v;
    xccl[k].advance = a;
}

/* ------------------------- xcclcmp ------------------------- */

/*
 * compare two elements of the xccl table for qsort
 * - sort on the verify field, moving unused elements (advance field = -1)
 *   to the end of the array
 * - preserve the initial order of elements which are equal
 */

int
xcclcmp (a, b)
  xccl_t        *a, *b;
{
    if ((a->advance == -1) || (b->advance == -1)) /* unused element */
        return ((b->advance == -1) ? -1 : 1);

    if (a->verify == b->verify)                 /* equal, preserve order in array */
        return ((int)(a - b));

    return (a->verify - b->verify);             /* different */
}

/* ------------------------- rprint ------------------------- */

rprint(a,s,n)
  char *s;
  int *a, n;
{
register int i;

    fprintf(fout,"block data\n");
    fprintf(fout,"common /L%s/ %s\n",s,s);
    fprintf(fout,"define S%s %d\n",s,n);
    fprintf(fout,"integer %s (S%s)\n",s,s);
    for(i=1; i<=n; i++)
    {
        if (i%8==1) fprintf(fout, "data ");
        fprintf(fout, "%s (%d)/%d/",s,i,a[i]);
        fprintf(fout, (i%8 && i<n) ? ", " : "\n");
    }
    fprintf(fout,"end\n");
}

/* -------------------------- shiftr -------------------------- */

shiftr(a, n)
  int *a;
{
int i;
    for(i=n; i>=0; i--)
        a[i+1]=a[i];
}

/* -------------------------- upone -------------------------- */

upone(a,n)
  int *a;
{
int i;
    for(i=0; i<=n ; i++)
        a[i]++;
}

/* -------------------------- bprint -------------------------- */

bprint(a,s,n)
 unsigned char *s,  *a;
 int  n;
{
register int i, j, k;
    fprintf(fout,"block data\n");
    fprintf(fout,"common /L%s/ %s\n",s,s);
    fprintf(fout,"define S%s %d\n",s,n);
    fprintf(fout,"integer %s (S%s)\n",s,s);
    for(i=1;i<n;i+=8){
        fprintf(fout,"data %s (%d)/%d/",s,i,a[i]);
        for(j=1;j<8;j++){
            k = i+j;
            if(k < n)fprintf(fout,", %s (%d)/%d/",s,k,a[k]);
        }
        putc('\n',fout);
    }
    fprintf(fout,"end\n");
}

/* ------------------------ addwcrank ------------------------ */

/*
 * add information to the wcrank table
 */

addwcrank(i,s,n)
  int           i;
  int           s;
  int           n;
{
    if (wcrank->table[i].info == 0)
    {
        wcrank->table[i].info = (void *)myalloc(1,sizeof(crank_t));
        if (wcrank->table[i].info == 0)
            error(MSGSTR(CALLOCFAILED,"There is not enough memory to create a hash entry."));
    }

    PCRANK(i)->verify = s;
    PCRANK(i)->advance = n;
}

/* ------------------------ addwsymbol ------------------------ */

/*
 * Add a wide character to the wide symbol table.  This table is required to
 * manage the wide charset.
 */

int
addwsymbol(c,dup)
  wchar_t c;
  int dup;
{
register int    i;

    if (!dup)                                   /* no duplicates */
    {
        for (i = 0; i < wsymboli; i++)
        {
            if (wsymbol[i] == c)
                return(0);
        }
    }
    
    if (wsymboli >= wsymbollen)                 /* need space ? */
    {
        if (wsymbollen == 0)
        {
            wsymbollen = NCH;
            wsymbol = (wchar_t *) myalloc (wsymbollen,sizeof (*wsymbol));
        }
        else
        {
            wsymbollen += NCH;
            wsymbol = (wchar_t *) realloc (wsymbol, wsymbollen * sizeof (*wsymbol));
        }
        if (wsymbol == 0)
            error(MSGSTR(NOCORE2, "Too little core for state generation"));
    }

    wsymbol[wsymboli++] = c;                    /* assign */
    wsymbol[wsymboli] = 0;
    return(1);
}

# ifdef PP
/* --------------------------- padd --------------------------- */

/*
 *    1. Same as add(), only checks first for duplicates.
 *
 * inputs -  array  : array in which to store the pointer to nxtpos
 *           n      : position in the array in which to store the
 *                    pointer
 * outputs - none
 * globals - tmpstat: array of flags indicating which nodes are active
 *           count  : # of active nodes in tmpstat
 *           nxtpos : permanent storage for the information in tmpstat
 */

static void
padd(array,n)
  int **array;
  int n;
{
register int i, *j, k;

    array[n] = nxtpos;
    if(count == 0)
    {
        *nxtpos++ = 0;
        return;
    }

    for(i=tptr-1;i>=0;i--)
    {
        j = array[i];
        if(j && *j++ == count)
        {
            for(k=0;k<count;k++)
                if(!tmpstat[*j++])break;
            if(k >= count)
            {
                array[n] = array[i];
                return;
            }
        }
    }
    add(array,n);

    return;
}
# endif

# ifdef MFDEBUG
/* -------------------------- pnames -------------------------- */

pnames()

{
register int    i;

    printf ("Names:  name  left  right  parent  wname\n");
    for (i = 0; i < tptr; i++)
    {
        printf ("%3d:    %4d  %4d  %5d   %5d  %5d\n",
                i, name[i], left[i], right[i], parent[i], wname[i]);
    }
}

/* -------------------------- pgotof -------------------------- */

pgotof()
{
register int    i;

    printf ("GOTOF (%d):\n\t", stnum);
    for (i = 0; i < stnum; i++)
    {
        printf (" %5d", gotof[i]);
        if (((i+1)%10)==0)
            printf ("\n\t");
    }
    putchar ('\n');
}

/* -------------------------- pnchar -------------------------- */

pnchar()
{
register int    i, j;

    printf ("NCHAR:\n\t");
    for (i = 0, j = 1; (nchar[i] || nchar[i+1]); i++, j++)
    {
        if (nchar[i])                           /* char */
            printf (" %10d", nchar[i]);
        else                                    /* CCL */
            printf (" [:%#4.4x:]", nchar[++i]);
        if (j == 5)
        {
            j = 0;
            printf ("\n\t");
        }
    }
    putchar ('\n');
}
# endif

/* ------------------------- hashalloc ------------------------- */

/*
 * create the data space for a hash table
 */

hash_t *
hashalloc(n)
  int           n;
{
hash_t          *p;

    if (n==0)                                   /* nothing to alloc */
        return;
    if (n<2)                                    /* must be bigger than 1 */
        error(MSGSTR(EBADMBO,"Invalid number of multi-byte output slots specified.  %d"),wcranksize);

    p = (hash_t *)myalloc(1,sizeof(*p));
    if (p == 0)
        return (0);
    p->table = (void *)myalloc(n,sizeof(hash_data_t));
    if (p->table == 0)
        return (0);

    p->size = n;
    p->top = 0;
    return (p);
}

/* ------------------------- hashfree ------------------------- */

/*
 * free the data space used by a hash table
 */

hashfree(h,s)
  hash_t        *h;
  int           s;
{
register int    i;

    if (h==0)
        return;
    for (i = 0; i < h->size; i++)
    {
        if (h->table[i].info)
            cfree(h->table[i].info,1,s);
    }
    cfree((void *)h->table,h->size,sizeof(hash_data_t));
    h->table = 0;
}

/* ------------------------- hashinit ------------------------- */

/*
 * initialize thehash tables used
 */

hashinit(h)
  hash_t        *h;
{
register int    i;

    for (i = 0; i < h->size; i++)
        h->table[i].info = h->table[i].next = h->table[i].id = 0;
    h->current = h->prev = h->top = 0;
}

/* ------------------------- hashfirst ------------------------- */

/*
 * compute an initial hash value from a character code
 */

int
hashfirst(c,h)                                  /* basic hash value */
  wchar_t       c;
  hash_t        *h;
{
    if (h == 0)
        return(0);
    h->current = (c % (h->size - 1)) + 1;
    h->prev = 0;
    return (h->current);
}

/* ------------------------- hashnext ------------------------- */

/*
 * compute a next hash value based on a hashfirst - or a previous hashnext -
 * action
 */

int
hashnext(h)
  hash_t        *h;
{
    if ((h == 0) || (h->current == 0))          /* hashfirst not done has not been done yet */
        return (0);

    h->prev = h->current;

    if (h->table[h->current].next != 0)         /* next collision */
    {
        h->current = h->table[h->current].next;
        return (h->current);
    }

    if (h->top+1 >= h->size)
        return (0);

    do                                          /* calculate a collision chain */
    {
        h->current += 1;
        if (h->current == h->size)
            h->current = 1;
    } while (h->table[h->current].id != 0);

    return (h->current);
}

/* ------------------------- hashnew ------------------------- */

/*
 * compute the location of an empty hash slot, chain it in if necessary
 */

int
hashnew(c,h)
  wchar_t       c;
  hash_t        *h;
{
int             a;

    for (a = hashfirst(c,h); (a!=0) && h->table[h->current].id; a = hashnext(h))
        ;                                       /* Empty */

    if (a==0)                                   /* overflow */
        return (0);

    h->table[h->current].id = c;
    if (h->prev != 0)                           /* map in collision */
        h->table[h->prev].next = h->current;
    h->top += 1;

    return (h->current);
}

/* ------------------------- hashfind ------------------------- */

/*
 * find the first matching entry in the hash table
 *
 * WARNING: some hash tables may have NON-unique entries, this will only get
 * the first match
 */

int
hashfind(c,h)
  wchar_t       c;
  hash_t        *h;
{
int             a;

    if ((h==0)||(h->table==0))
        return(0);

    for (a = hashfirst(c,h); (a!=0) && h->table[a].id; a = hashnext(h))
    {
        if (h->table[a].id == c)                /* found */
            return(a);
    }

    return(0);                                  /* not found */
}

/* -------------------------- rehash -------------------------- */

/*
 * pack the hash table into something smaller:
 * - create a new table
 * - hash old elements into that
 * - update the base hash information with the new table
 */

int
rehash (h, s)
  hash_t        *h;
  int           s;
{
register int    i;
hash_data_t     *t, *t1;
int             n, s1, top1;

    if (s >= h->size)                           /* no savings! */
        return (h->size);

    t = (hash_data_t *)myalloc(s,sizeof(*t));   /* make new table */
    if (t == 0)
    {
        warning(MSGSTR(CALLOCFAILED,"There is not enough memory to pack the hash table."));
        return(s);
    }

    top1 = h->top;
    t1 = h->table;      h->table = t;           /* swap in the new table */
    s1 = h->size;       h->size  = s;

    hashinit(h);                                /* init the new table */

    for (i = 0; i < s1; i++)
    {
        if (t1[i].info)
        {                                       /* this shouldn't happen, but Murphy says... */
            if ((n = hashnew (t1[i].id, h)) == 0)
            {
                cfree((void *)h->table,h->size,sizeof(*h->table));
                h->top = top1;
                h->table = t1;
                h->size = s1;
                return (h->size);
            }

            h->table[n].info = t1[i].info;
        }
    }
    cfree((void *)t1,s1,sizeof(*t1));           /* free the old table */
    return (s);
}
