/* @(#)45       1.11  src/bos/usr/ccs/bin/common/error.h, cmdprog, bos411, 9428A410j 6/12/94 13:18:04 */
/*
 * COMPONENT_NAME: (CMDPROG) error.h
 *
 * FUNCTIONS: TOOLSTR, cerror, uerror, warning, werror                       
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
 */
/*
 * @OSF_COPYRIGHT@
 */

# ifndef EXIT
# define EXIT exit
# endif

#if defined (LINT)
#define OUTFILE stdout
#elif defined (CFLOW)
#define OUTFILE stderr
#endif

#include "ctools_msg.h"
#define TOOLSTR(Num, Str) catgets(catd, MS_CTOOLS, Num, Str)
nl_catd catd;


int nerrors = 0;  /* number of errors */

/* -------------------- uerror -------------------- */

/*VARARGS1*/
uerror( mode, s, a, b ) char *s; {
        if( mode ){
#if     defined (LINT) || defined (CFLOW)
                if( !ifname || !strcmp(pfname, ifname) )
                        fprintf( OUTFILE, TOOLSTR(M_MSG_199, "\"%s\", line %d: error: "), pfname, lineno );
                else
                        fprintf( OUTFILE, TOOLSTR(M_MSG_277, "\"%s\", line %d (\"%s\"): error: "), ifname, lineno, pfname );
                fprintf( OUTFILE, s, a, b );
                fprintf( OUTFILE, "\n" );
#else
                fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
                fprintf( stderr, s, a, b );
                fprintf( stderr, "\n" );
#endif
                if( ++nerrors > 30 )
                        cerror(TOOLSTR(M_MSG_257, "too many errors"));
        } else {
#ifdef WANSI
                warning( WANSI, s, a, b );
#else
                warning( 1, s, a, b );
#endif
        }
}

/* -------------------- werror -------------------- */

/*VARARGS1*/
werror( mode, s, a, b ) char *s; {
        if( mode ){
#if     defined (LINT) || defined (CFLOW)
                if( !ifname || !strcmp(pfname, ifname) )
                        fprintf( OUTFILE, TOOLSTR(M_MSG_199, "\"%s\", line %d: error: "), pfname, lineno );
                else
                        fprintf( OUTFILE, TOOLSTR(M_MSG_277, "\"%s\", line %d (\"%s\"): error: "), ifname, lineno, pfname );
                fprintf( OUTFILE, s, a, b );
                fprintf( OUTFILE, "\n" );
#else
                fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
                fprintf( stderr, s, a, b );
                fprintf( stderr, "\n" );
#endif
        } else {
#ifdef WANSI
                warning( WANSI, s, a, b );
#else
                warning( 1, s, a, b );
#endif
        }
}

/* -------------------- cerror -------------------- */

/*VARARGS1*/
cerror( s, a, b, c ) char *s; {
        /*
        ** Compiler error: die
        */
        if( nerrors && nerrors <= 30 ){
                /* give the compiler the benefit of the doubt */
                fprintf( stderr, TOOLSTR(M_MSG_288, "%s, line %d: error: "), ftitle, lineno );
                fprintf( stderr, TOOLSTR(M_MSG_289,
                        "cannot recover from earlier errors: goodbye!\n" ));
        } else {
#if     defined (LINT) || defined (LINTP2)
                if (lineno > 0)
                        fprintf( stderr, TOOLSTR(M_MSG_290, "%s, line %d: lint error: " ), ftitle, lineno);
                else
                        fprintf( stderr, TOOLSTR(M_MSG_290, "lint error: "), ftitle, 0 );
#else
#if defined (CFLOW) || defined (CFLOW2)
                if (lineno > 0)
                        fprintf( stderr, TOOLSTR(M_MSG_267, "%s, line %d: cflow error: " ), ftitle, lineno);
                else
                        fprintf( stderr, TOOLSTR(M_MSG_267, "cflow error: "), ftitle, 0 );
#else
#if defined (CXREF)
                if (lineno > 0)
                        fprintf( stderr, TOOLSTR(M_MSG_268, "%s, line %d: cxref error: " ), ftitle, lineno);
                else
                        fprintf( stderr, TOOLSTR(M_MSG_268, "cxref error: "), ftitle, 0 );
#else
                fprintf( stderr, TOOLSTR(M_MSG_291, "%s, line %d: compiler error: " ), ftitle, lineno);
#endif
#endif
#endif
                fprintf( stderr, s, a, b, c );
                fprintf( stderr, "\n" );
        }
#ifdef FORT
        {       extern short debugflag;
                if (debugflag)
                        abort();
        }
#endif /*FORT*/
        EXIT( 2 );
}

/* -------------------- warning -------------------- */

/*VARARGS1*/
warning( mode, s, a, b ) int mode; char *s; {
        if( mode ){
#if     defined (LINT) || defined (CFLOW)
                if( !ifname || !strcmp(pfname, ifname) )
                        fprintf( OUTFILE, TOOLSTR(M_MSG_292, "\"%s\", line %d: warning: "), pfname, lineno );
                else
                        fprintf( OUTFILE, TOOLSTR(M_MSG_293, "\"%s\", line %d (\"%s\"): warning: "), ifname, lineno, pfname );
                fprintf( OUTFILE, s, a, b );
                fprintf( OUTFILE, "\n" );
#else
                fprintf( stderr, TOOLSTR(M_MSG_294, "%s, line %d: warning: "), ftitle, lineno );
                fprintf( stderr, s, a, b );
                fprintf( stderr, "\n" );
#endif
        }
}
