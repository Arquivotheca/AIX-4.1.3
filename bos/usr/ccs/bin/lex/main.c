static char sccsid[] = "@(#)14	1.11.1.10  src/bos/usr/ccs/bin/lex/main.c, cmdlang, bos411, 9428A410j 4/12/94 21:08:53";
/**
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: main get1core free1core get2core free2core get3core
 *            free3core myalloc buserr segviol yyerror
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
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
/*static char rcsid[]="RCSfile: main.c,v Revision: 1.6 (OSF) Date: 90/10/07    
17:42:16 "; */

        /* lex [-[drcyvntf]] [file] ... [file] */

        /* Copyright 1976, Bell Telephone Laboratories, Inc.,
            written by Eric Schmidt, August 27, 1976   */

        /* Multi-byte support added by Michael S. Flegel, July 1991 */
           
# include "ldefs.h"
# include "once.h"
#ifndef _BLD                                    /* BLD ifdef kept for compatibility with osf */
# include <stdlib.h>
# include <locale.h>
#endif

int	asciipath = 1;	/* if we don't run setlocale() we are in the c locale */

/* -------------------------- main --------------------------- */

/*
 *    1. Handles the arguments and calls each phase of the compiler.
 *       which are as follows:
 *                yyparse() - parse the input file
 *                mkmatch() - create the match table
 *                cfoll()   - calculate the set of follow positions
 *                            for each position in the parse tree
 *                cgoto()   - calculate the finite state machine
 *                layout()  - lay out the generated tables in
 *                            lex.yy.c
 *
 *    2. To debug, compile everything with -DDEBUG and invoke
 *       lex with -d.
 */

main(argc,argv)
  int argc;
  char **argv;
{
register int i;
char * loc_val;

# ifdef DEBUG
    signal(10,buserr);
    signal(11,segviol);
# endif
#ifndef _BLD
    setlocale(LC_ALL, "");
    catd = catopen(MF_LEX,NL_CAT_LOCALE);

    /* need to know if we can assume we are in the C locale */
    loc_val =  setlocale(LC_COLLATE,NULL);
    if ( strcmp(loc_val,"C")==0 || strcmp(loc_val,"POSIX")==0 )
        asciipath=1;
    else
        asciipath=0;

#endif
    mbcurmax = MB_CUR_MAX;
    if (mbcurmax > 1)
    {
        multibytecodeset = 1;
    }
    else
    {
        multibytecodeset = 0;
    }
    if (!asciipath) {
        co_nord = __OBJ_DATA(__lc_collate)->co_nord;
        co_wc_max = __OBJ_DATA(__lc_collate)->co_wc_max;
        co_wc_min = __OBJ_DATA(__lc_collate)->co_wc_min;
        co_col_max = __OBJ_DATA(__lc_collate)->co_col_max;
        co_col_min = __OBJ_DATA(__lc_collate)->co_col_min;
	}
    else { /* make assumptions about the C locale    */
	   /* this will have to be updated if  the   */
           /* C locale defined in NLSsetup.c changes */
        co_nord = 0;
        co_wc_max = 255;
        co_wc_min = 0;
        co_col_max = 513;
        co_col_min = 257;
	}

    if (co_wc_min < 1)
        co_wc_min = 1;

    while (argc > 1 && argv[1][0] == '-'){
        if(!strcmp(argv[1], "--"))
        {
            argv++;
            argc--;
            break;
        }
        i = 0;
        while(argv[1][++i]){
            switch (argv[1][i]){
# ifdef DEBUG
            case 'd': debug++; break;
            case 'y': yydebug = TRUE; break;
# endif
            case 'r': case 'R':
                warning(MSGSTR(NORATFOR, "Ratfor not currently supported with lex"));
                ratfor=TRUE; break;
            case 'c':
                ratfor=FALSE;
                break;
            case 'C':
                cplusplus=TRUE; break;
            case 't': case 'T':
                fout = stdout;
                errorf = stderr;
                break;
            case 'v': case 'V':
                report = 1;
                break;
            case 'n': case 'N':
                report = 0;
                break;
            default:
                warning(MSGSTR(BADOPTION, "Unknown option %c"),argv[1][i]);
            }
        }
        argc--;
        argv++;
    }
    sargc = argc;
    sargv = argv;
    if (argc > 1){
        fin = fopen(argv[++fptr], "r");         /* open argv[1] */
        sargc--;
    }
    else fin = stdin;
    if(fin == NULL){
        if ( argc > 1 )
            error(MSGSTR(NOINPUT, "Can't read input file %s"),argv[1]);
        else
            error(MSGSTR(NOSTDIN, "Can't read standard input"));
    }
    
    extra = (wchar_t *)calloc(nactions, sizeof(wchar_t));
    
    mcsize = getcollset();
    psave = 0;


#ifdef DEBUG
    if(mccollset)
        prtcollist(mccollset);
#endif

    gch();
    /*
     * may be gotten: def, subs, sname, schar, ccl, dchar
     */
    get1core();
    /*
     * may be gotten: name, left, right, nullstr, parent
     */
    scopy(L"INITIAL",sp);
    sname[0] = sp;
    excl[0] = 0;
    sp += slength("INITIAL") + 1;
    sname[1] = 0;
    
    if(yyparse(0) || yaccerror) exit(1);     /* error return code */
    /*
     * may be disposed of: def, subs, dchar
     */
    free1core();
    /*
     * may be gotten: tmpstat, foll, positions, gotof, nexts,
     *            nchar, state, atable, sfall, cpackflg
     */
    get2core();
    ptail();

    mkmatch();

# ifdef DEBUG
    if(debug) pccl();
# endif
    sect  = ENDSECTION;
    if(tptr>0)
    {
# ifdef MFDEBUG
        if (debug) pnames();
# endif
        cfoll(tptr-1);
    }
    
# ifdef DEBUG
    if(debug) pfoll();
# endif
    cgoto();
# ifdef DEBUG
    if(debug)
    {
        printf("Print %d states:\n",stnum+1);
        for(i=0;i<=stnum;i++)
            stprt(i);
    }
# endif
    /*
     * may be disposed of: positions, tmpstat, foll, state, name,
     *                 left, right, parent, ccl, schar, sname
     */
    free2core();
    /*
     * may be gotten: verify, advance, wch, wverify, wadvance, wnext, stoff
     */
    get3core();
    
# ifdef MFDEBUG
    if (debug)
    {
        pgotof();
        pnchar();
    }
# endif
    
    layout();
    
# ifdef DEBUG
    /*
     * may be disposed of: verify, advance, wch, wverify, wadvance, wnext, stoff, nexts, nchar,
     *                 gotof, atable, ccpackflg, sfall
     */
    free3core();
# endif
    /*
     * Check for an environmental override of the lex finite-state machine
     * skeleton.
     */
    {
        char *skeleton = (char *)getenv("LEXER");
        
        if(skeleton)
            cname = skeleton;
    }
    
    fother = fopen(ratfor?ratname:cname,"r");
    if(fother == NULL)
        error(MSGSTR(NOLEXDRIV, "Lex driver missing, file %s"),
              ratfor?ratname:cname);
    while ( (i=getwc(fother)) != WEOF)
        putwc(i,fout);
    
    fclose(fother);
    fclose(fout);
    if(
# ifdef DEBUG
       debug   ||
# endif
       report == 1)statistics();
    fclose(stdout);
    fclose(stderr);
    exit(0);    /* success return code */
}

/* ------------------------ get1core ------------------------- */

get1core()
{
    pcptr = pchar = (wchar_t *)myalloc(pchlen, sizeof(*pchar));
    def = (wchar_t **)myalloc(DEFSIZE,sizeof(*def));
    subs = (wchar_t **)myalloc(DEFSIZE,sizeof(*subs));
    defsize = DEFSIZE;
    dp = dchar = (wchar_t *)myalloc(DEFCHAR,sizeof(*dchar));
    defchar = DEFCHAR;
    sname = (wchar_t **)myalloc(STARTSIZE,sizeof(*sname));
    excl = (short *)myalloc(STARTSIZE, sizeof(short));
    sp =schar = (wchar_t *)myalloc(STARTCHAR,sizeof(*schar));
    collname = (wchar_t *)myalloc(COLLNAME_SIZE + 1, sizeof(*collname));
    mbcollname = (char *)myalloc(COLLNAME_SIZE * mbcurmax + 1, sizeof(*mbcollname));

    if( pchar == 0 || def == 0 || subs == 0 || dchar == 0 || sname == 0 || excl == 0 || schar == 0 || collname == 0 || mbcollname == 0 )
        error(MSGSTR(NOCORE1, "Too little core to begin"));
}

/* ------------------------ free1core ------------------------- */

free1core()
{
    cfree((void *)def,defsize,sizeof(*def));
    cfree((void *)subs,defsize,sizeof(*subs));
    cfree((void *)dchar,defchar,sizeof(*dchar));
    cfree((void *)collname,COLLNAME_SIZE + 1,sizeof(*collname));
    cfree((void *)mbcollname,COLLNAME_SIZE * mbcurmax + 1,sizeof(*mbcollname));
}

/* ------------------------ get2core ------------------------- */

get2core()
{
register int    i;

    gotof = (int *)myalloc(nstates,sizeof(*gotof));
    nexts = (int *)myalloc(ntrans,sizeof(*nexts));
    nchar = (wchar_t *)myalloc(ntrans,sizeof(*nchar));
    state = (int **)myalloc(nstates,sizeof(*state));
    atable = (int *)myalloc(nstates,sizeof(*atable));
    sfall = (int *)myalloc(nstates,sizeof(*sfall));
    cpackflg = (char *)myalloc(nstates,sizeof(*cpackflg));
    tmpstat = (wchar_t *)myalloc(tptr+1,sizeof(*tmpstat));
    foll = (int **)myalloc(tptr+1,sizeof(*foll));
    nxtpos = positions = (int *)myalloc(maxpos,sizeof(*positions));

    if(   tmpstat == 0 || foll == 0 || positions == 0 || gotof == 0
       || nexts == 0 || nchar == 0 || state == 0
       || atable == 0 || sfall == 0 || cpackflg == 0 )
    {
        error(MSGSTR(NOCORE2, "Too little core for state generation"));
    }

    for(i=0;i<=tptr;i++)
        foll[i] = 0;
}

/* ------------------------ free2core ------------------------- */

free2core()
{
    cfree((void *)positions,maxpos,sizeof(*positions));
    cfree((void *)tmpstat,tptr+1,sizeof(*tmpstat));
    cfree((void *)foll,tptr+1,sizeof(*foll));
    cfree((void *)name,treesize,sizeof(*name));
    cfree((void *)wname,treesize,sizeof(*wname));
    cfree((void *)wsymbol,wsymbollen,sizeof(*wsymbol));
    cfree((void *)left,treesize,sizeof(*left));
    cfree((void *)right,treesize,sizeof(*right));
    cfree((void *)parent,treesize,sizeof(*parent));
    cfree((void *)nullstr,treesize,sizeof(*nullstr));
    cfree((void *)state,nstates,sizeof(*state));
    cfree((void *)sname,STARTSIZE,sizeof(*sname));
    cfree((void *)excl,STARTSIZE,sizeof(short));
    cfree((void *)schar,STARTCHAR,sizeof(*schar));
    cclarrayi=0;
    while(cclarray[cclarrayi].cclptr)
    {
        cfree((void *)cclarray[cclarrayi].cclptr, cclarray[cclarrayi].len, sizeof(*cclarray[0].cclptr));
        cclarrayi++;
    }
    cfree((void *)cclarray, cclarray_size, sizeof(*cclarray));
}

/* ------------------------ get3core ------------------------- */

get3core()
{
    verify      = (int *)myalloc(outsize,sizeof(*verify));
    advance     = (int *)myalloc(outsize,sizeof(*advance));
    stoff       = (int *)myalloc(stnum+2,sizeof(*stoff));

    if(verify == 0 || advance == 0 || stoff == 0)
        error(MSGSTR(NOCORE3, "Too little core for final packing"));

    if (wcranksize > 0)                         /* MB core */
    {
        wcrank = (hash_t *)hashalloc(wcranksize);
        if (wcrank == 0)
            error(MSGSTR(NOCORE3, "Too little core for final packing"));
    }
}

# ifdef DEBUG
/* ------------------------ free3core ------------------------- */

free3core()
{
    cfree((void *)advance,outsize,sizeof(*advance));
    cfree((void *)verify,outsize,sizeof(*verify));
    cfree((void *)stoff,stnum+1,sizeof(*stoff));
    cfree((void *)gotof,nstates,sizeof(*gotof));
    cfree((void *)nexts,ntrans,sizeof(*nexts));
    cfree((void *)nchar,ntrans,sizeof(*nchar));
    cfree((void *)atable,nstates,sizeof(*atable));
    cfree((void *)sfall,nstates,sizeof(*sfall));
    cfree((void *)cpackflg,nstates,sizeof(*cpackflg));


    if (wmatchsize > 0)
        hashfree(wmatch,sizeof(match_t));
    if (wcranksize > 0)
        hashfree(wcrank,sizeof(crank_t));
    if (xccl > 0)
        cfree((void *)xccl,xcclsize,sizeof(*xccl));
}
# endif

/* ------------------------- myalloc -------------------------- */

void *myalloc(a,b)
  int a,b;
{
register int i;

    if (!a || !b)
        return (0);

    i = (int)calloc(a, b);
    if(i==0)
        warning(MSGSTR(CALLOCFAILED, "OOPS - calloc returns a 0"));
    else if(i == -1){
# ifdef DEBUG
        warning("calloc returns a -1");
# endif
        return(0);
    }
    return((void *)i);
}

# ifdef DEBUG
/* ------------------------- buserr -------------------------- */

buserr()
{
    fflush(errorf);
    fflush(fout);
    fflush(stdout);
    fprintf(errorf, "Bus error\n");
    if(report == 1)statistics();
    fflush(errorf);
}

/* ------------------------- segviol -------------------------- */

segviol()
{
    fflush(errorf);
    fflush(fout);
    fflush(stdout);
    fprintf(errorf, "Segmentation violation\n");
    if(report == 1)statistics();
    fflush(errorf);
}
# endif

/* ------------------------- yyerror -------------------------- */

yyerror(s)
  char *s;
{
    yaccerror = TRUE;
    fprintf(stderr, MSGSTR(LINE, "line %d: %s\n"), yyline, s);
}
