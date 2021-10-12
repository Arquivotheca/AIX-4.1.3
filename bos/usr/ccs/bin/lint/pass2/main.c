static char sccsid[] = "@(#)65  1.10.1.3  src/bos/usr/ccs/bin/lint/pass2/main.c, cmdprog, bos411, 9428A410j 4/15/94 15:00:11";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: LintPass2, main
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
 * Modified June 91 by RWaters: P23321 void returns
 *                              P19915 used but not defined 
 */

#include "mfile1.h"
#include "error.h"
#include "lint2.h"
#include "lint_msg.h"
#include <locale.h>

#define         MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;

main(argc, argv)
int argc;
register char *argv[];
{
        register int i, c;
        register char *cp;
        register int r;
        int fdef = 0;
        int dashflag = 0;

        setlocale(LC_ALL, "");                  /* 33520 (MF) - moved setlocal before catopen */
        catd = catopen(MF_LINT, NL_CAT_LOCALE);

        for( i = 0; i <= MXDBGFLG; i++ ){
                devdebug[i] = 0;
                warnlevel[i] = 0;
        }

        for( i=1; i<argc; ++i ){
                if( *(cp=argv[i]) == '-' && !dashflag)
                while( *++cp && !dashflag){
                    switch( *cp ){
                    case '-':
                            dashflag = 1;
                            break;

                    case 'p': /* extreme portability */
                            pflag++;
                            break;

                    case 'X': /* debug toggles */
                            debug++;
                            break;

                    case 'w': /* toggle warning level */
                            while( *++cp )
                                {
                                switch(*cp)
                                {
                                case 'A':
  
                                /*if( *cp == 'A' ) */
                                        for( r = 0; r <= MXDBGFLG; r++ )
                                                warnlevel[r] = !warnlevel[r];
                                        break;
                                case 'l':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'u':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'D':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'R':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'h':
                                        warnlevel[*cp] = 0;
                                        break;
                                default:
                                        
                                /*else */if( ( *cp >= 'a' && *cp <= 'z' ) ||
                                                ( *cp >= 'A' && *cp <= 'Z' ) )
                                        warnlevel[*cp] = !warnlevel[*cp];
                                        }
                                }
                            --cp;
                            break;

                    case 'M': /* HCR development toggles */
                            while( *++cp )
                                if( ( *cp >= 'a' && *cp <= 'z') ||
                                                ( *cp >= 'A' && *cp <= 'Z' ) )
                                        devdebug[*cp] = !devdebug[*cp];
                            --cp;
                            break;
                    }
                }

                else {
                        /* First-time initializations. */
                        if (fdef == 0) {
                                /* check for ANSI mode */
                                if ( devdebug[ANSI_MODE] ) {
                                        /* set all other ANSI flags on */
                                        devdebug[ANSI_PARSE]    =
                                                !devdebug[ANSI_PARSE];
                                        devdebug[COMPATIBLE]    =
                                                !devdebug[COMPATIBLE];
                                        devdebug[PROMOTION]     =
                                                !devdebug[PROMOTION];
                                        devdebug[REFDEF]        =
                                                !devdebug[REFDEF];
                                        devdebug[SCOPING]       =
                                                !devdebug[SCOPING];
                                        devdebug[STRUCTBUG]     =
                                                !devdebug[STRUCTBUG];
                                        devdebug[TYPING]        =
                                                !devdebug[TYPING];
                                }
                                InitTypes();
                                fdef = 1;
                                curSym = &theSym;
                        }
                        fname = argv[i];
                        OpenFile();
                        LintPass2();
                        CloseFile();
                }
        }
        CheckSymbols();
}

LintPass2()
{
        int stat;

        /* Read and process all symbol records. */
        while (1) {
                switch (InHeader()) {
                case LINTEOF:
                        /* EOF delimiter. */
                        if (markerEOF)
                                return;
                        markerEOF = 1;
                        continue;

                case LINTADD:
                        /* Append to function usage, symbol should exist. */
                        InUsage();
                        prevSym = LookupSymbol(curSym, LOOK);
                        if (prevSym)
                                AddFtnUsage(prevSym, curSym);
                        else
                                cerror(MSGSTR(M_MSG_260,
                                        "i/o sequence error on file %s"),
                                        curPFname);

                        break;

                case LINTSYM:
                        /* Fetch symbol. */
                        InSymbol();
                        prevSym = LookupSymbol(curSym, LOOK);

                        /* Cross-check old symbols, otherwise
                         * insert new symbols. */
                        if (prevSym) {

                                switch (stat = CheckSymbol()) {
                                case STORE:
                                        (void) LookupSymbol(curSym, STORE);
                                        break;
                                case CHANGE:
                                        ChangeSymbol(prevSym, curSym);
                                        break;
                                case REJECT:
                                        /* Remember bad function calls. */
                                        if (ISFTN(curSym->type)) {
                                                /*
                                                ** RW: This occurs when library
                                                ** fuctions are used before 
                                                ** they are declared.
                                                */
                                                if (curSym->usage & LINTLIB)
                                                        ChangeSymbol(prevSym, curSym);
                                                else
                                                        FtnRefSymbol(prevSym, curSym);
                                        }
                                        else {
                                                prevSym->usage |= curSym->usage;
                                        }
#ifdef  DEBUG
                                        if (debug)
                                                PrintSymbol("REJECTING", curSym);
#endif
                                        break;
                                /* replace previous symbol with current symbol
                                   when previous symbol was only an address
                                   reference. Defect 72807 */
                                case REPLACE:
                                        curSym->usage |= prevSym->usage;
                                        curSym->usage ^= LINTADDR;
                                        ReplaceSymbol(prevSym, curSym);
                                        break;
                                default:
                                        cerror(MSGSTR(M_MSG_261,
                                                "unknown action for symbol in file %s"),
                                                curPFname);
                                }
                        } else
                                (void) LookupSymbol(curSym, STORE);
                        break;

                default:
                        cerror(MSGSTR(M_MSG_262,
                                "unknown record directive in file %s"),
                                curPFname);
                }
        }
}
