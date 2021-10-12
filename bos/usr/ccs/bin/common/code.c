static char sccsid[] = "@(#)51  1.32  src/bos/usr/ccs/bin/common/code.c, cmdprog, bos411, 9428A410j 7/8/94 17:52:22";
/*
 * COMPONENT_NAME: (CMDPROG) code.c
 *
 * FUNCTIONS: EmitVolatile, aobeg, aocode, aoend, bccode
 *            beg_file, bfcode, branch, bycode, commdec, defalign, deflab
 *            defnam, efcode, ejobcode, fldty, getlab, loadcon, locctr, main
 *            makearg, prFTN, retgen, swepilog, swprolog, zecode
 *
 * ORIGINS: 27 3 9 32
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 *  @OSF_COPYRIGHT@
*/

#include "mfile1.h"
#include "messages.h"
#include <signal.h>
# include <a.out.h>
# include <ctype.h>
#ifndef AIXV2
#include <locale.h>
#endif

# ifndef ONEPASS
int adebug = 0; /* optimize flag */
# else
extern int adebug;
# endif

int nofpflding = 0;     /* turns off folding of f.p. constants */

int aflag;
int minsvarg;   /* minimum offset of a register arg to be saved */
int proflg = 0; /* are we generating profiling code? */
int qdebug = 0; /* queer register char/short flag */
int gdebug = 0; /* sdb flag     */
int fdefflag;   /* are we within a function definition ? */
int labelno;
int strftn = 0;       /* is the current function one which returns a value */

# define Lineno (lineno-startln+1)      /* local line number, opening { = 1 */

int startln;            /* the editor line number of the opening '{' } */
int oldln;              /* remember the last line number printed */
char startfn[100] = ""; /* the file name of the opening '{' */
#ifdef XCOFF
char data_sect[] = DATA_SECT;   /* name of the General Data Csect */
#endif

int bb_flags[BCSZ];     /* remember whether or not bb is needed */
# ifdef ONEPASS
extern int param_num;   /* No. of parameters that pass 2 must save */
# else
int param_num;          /* No. of parameters that pass 2 must save */
# endif

/* ------- */

int reginfo;          /* debug info: which arguments in registers? */

/* ------- */

/* if this module were partitioned properly the following pass 2 */
/* variables wouldn't be needed; the values being passed need    */
/* not be computed in pass 1                                     */

extern int baseoff, maxtreg, maxfpr;

/* ------------------------------------------------ */

/* tempfile is used to hold initializing strings that cannot be output
 * immediately -- they are read back in at the end and appended to
 * the intermediate text
 */
FILE *outfile = stdout;
# ifdef TEMPFILE
FILE *tempfile;
# endif
extern char *mktemp();
/* char *toreg(); */

/* -------------------- branch -------------------- */

branch( n ){
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        /* output a branch to label n */
        /*exception is an ordinary function branching to retlab: then, return*/
# ifdef ONEPASS
        printf( "\tb\tL.%d\n", n );
# else
        printf( "%c%d\t\n", IBRANCH, n );
# endif
#endif
        }

/* -------------------------------------------------- */

int lastloc = { -1 };

/* -------------------- defalign -------------------- */

defalign(n)
{
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        /* cause the alignment to become a multiple of n */
        n /= SZCHAR;
        if( lastloc != PROG && n > 1 )
                printf( "\t.align\t2\n");       /* always align */
                /* printf("\t.align\t%d\n", (n > 2) ? 2 : 1 );  */
#endif
        }

/* -------------------- locctr -------------------- */

locctr( l ){
        /* output the location counter
         */
        static int std_lastloc = -1;    /* lastloc for stdout */
        register temp;
        /* l is PROG, ADATA, DATA, STRNG, ISTRNG, or STAB */

        if( l == lastloc && l == std_lastloc ) return(l);
#ifdef XCOFF
        if( l == RESET )
                {l = lastloc; temp = RESET;}
        else
                temp = lastloc;
#else
        temp = lastloc;
#endif
        lastloc = l;
        switch( l ){

        case PROG:
                if( curftn < 0 )
                        break;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                if ( outfile == stdout || std_lastloc != PROG )
# ifdef ONEPASS
#ifdef XCOFF
                        fprintf( stdout, "\t.csect\t.%s[pr]\n",
                                        stab[curftn].psname );
#else
                        fputs( "\t.text\n", stdout );
#endif
# else
#ifdef XCOFF
                        fprintf( stdout, "%c\t.csect\t.%s[pr]\n",
                                        ITEXT, stab[curftn].psname );
#else
                        fprintf( stdout, "%c\t.text\n", ITEXT );
#endif
# endif
#endif
                outfile = stdout;
                std_lastloc = PROG;
                break;

        case DATA:
        case ADATA:
                outfile = stdout;
                std_lastloc = l;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                if( temp != DATA && temp != ADATA )
# ifdef ONEPASS
#ifdef XCOFF
                        fprintf( stdout, "\t.csect\t%s[rw]\n", data_sect );
#else
                        fputs( "\t.data\n", stdout );
#endif
# else
#ifdef XCOFF
                        fprintf( stdout, "%c\t.csect\t%s[rw]\n",
                                IDATA, data_sect);
#else
                        fprintf( stdout, "%c\t.data\n", IDATA );
#endif
# endif
#endif
                break;

        case STRNG:
        case ISTRNG:
                /* NO output string initializers to a temporary file for now
                 * don't update lastloc
                 */
                /* outfile = tempfile;  */
                std_lastloc = l;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                if( temp != STRNG && temp != ISTRNG )
# ifdef ONEPASS
#ifdef XCOFF
                        fprintf( stdout, "\t.csect\t%s[rw]\n", data_sect );
#else
                        fputs( "\t.data\t1\n", stdout );
#endif
# else
#ifdef XCOFF
                        fprintf( stdout, "%c\t.csect\t%s[rw]\n",
                                IDATA, data_sect);
#else
                        fprintf( stdout, "%c\t.data\t1\n", IDATA );
#endif
# endif
#endif
                break;

        case STAB:
                cerror(TOOLSTR(M_MSG_270, "locctr: STAB unused" ));
                break;

        default:
                cerror(TOOLSTR(M_MSG_271, "illegal location counter" ));
                }

        return( temp );
        }

/* -------------------- deflab -------------------- */

deflab( n ){
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        /* output something to define the current position as label n */
# ifdef ONEPASS
#ifdef XCOFF
        printf("_L.%d:\n", n );
#else
        printf("L.%d:\n", n );
#endif
# else
#ifdef XCOFF
        if( lastloc == PROG ) {
                printf( "%c%d\t\n", ILABEL, n );
                printf( "\tnop\n");
                }
        else
                printf( "_L.%d:\n", n );
#else
        if( lastloc == PROG )
                printf( "%c%d\t\n", ILABEL, n );
        else
                printf( "L.%d:\n", n );
#endif
# endif
#endif
        }

/* -------------------- getlab -------------------- */

getlab(){
        /* return a number usable for a label
         */
        static int crslab = 10;

        return( ++crslab );
        }

/* -------------------- defnam -------------------- */

defnam( p ) register struct symtab *p; {
        /* define the current location as the name *p->psname
         * first give the debugging info for external definitions
         */
        if( p->slevel == 0 )    /* make sure its external */
                prdef(p,0);
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
#ifdef XCOFF
        if( p->sclass == EXTDEF ) {
                printf( "\t.globl\t%s[rw]\n", p->psname );
                printf( "\t.toc\n" );
#ifdef ONEPASS
                printf( "T.%s:\t.tc\t%s[tc],%s[rw]\n",
                        p->psname, p->psname, p->psname );
#else
                printf( "%cT.%s:\t.tc\t%s[tc],%s[rw]\n",
                        ITEXT, p->psname, p->psname, p->psname );
#endif
                printf( "\t.csect\t%s[rw]\n", p->psname);
                }
#else
        if( p->sclass == EXTDEF )
                printf( "\t.globl\t_%s\n", p->psname );
#endif
#ifdef XCOFF
        if( p->sclass == STATIC ) {
                printf( "\t.csect\t%s[rw]\n", data_sect);
                if( p->slevel > 1)
                        deflab( p->offset );
                else
                        printf( "_%s:\n", p->psname );
                }
#else
        if( p->sclass == STATIC && p->slevel > 1 )
                deflab( p->offset );
#endif
        else
#ifdef XCOFF
#ifdef ONEPASS
                printf( "%s:\n", p->psname );
#else
                printf( "%c%s:\n", IDATA, p->psname );
#endif
#else
                printf( "_%s:\n", p->psname );
#endif
#endif

                }

/* -------------------- commdec -------------------- */

commdec( q ) register struct symtab *q; {
        /*
        ** Make a common declaration for q, if reasonable.
        */
        register int align;
        register OFFSZ off;
        int saveloc;

        chkty( q );
        off = tsize( q->stype ) / SZCHAR;

        if( q->slevel > 1 ){
                /* q->sclass == STATIC */
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
# ifndef ONEPASS
                saveloc = locctr(DATA);
#ifdef XCOFF
                fprintf(stdout, "%c\t.csect\t%s[rw]\n", IDATA, data_sect);
                fprintf(stdout, "_L.%d:\t.space\t%d\n", q->offset, off);
#else
                printf("\t.lcomm\tL.%d,%d\n", q->offset, off);
#endif
                locctr(saveloc);
# else
#ifdef XCOFF
                fprintf(stdout, "\t.csect\t%s[rw]\n", data_sect);
                fprintf(stdout, "_L.%d:\t.space\t%d\n", q->offset, off);
#else
                printf("\t.lcomm\tL.%d,%d\n", q->offset, off);
#endif
# endif ONEPASS
#else
                ;
#endif
        } else if( q->sclass == USTATIC ){
                q->sclass = STATIC;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                prdef(q, 0);
#ifdef XCOFF
                fprintf(stdout, "\t.csect\t%s[rw]\n", data_sect);
                fprintf(stdout, "_%s:\t.space\t%d\n", q->psname, off);
#else
                printf("\t.lcomm\t_%s,%d\n", q->psname, off);
#endif
#endif
        } else /* q->sclass == EXTENT */ {
                q->sclass = EXTDEF;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                locctr( DATA );
                if( devdebug[REFDEF] ){
                        defalign( talign( q->stype ) );
                        defnam( q );
                        printf("\t.space\t%d\n", off);
                } else {
                        prdef(q, 0);
#ifdef XCOFF
                        locctr( RESET );
                        printf( "\t.toc\n" );
#ifdef ONEPASS
                        printf( "T.%s:\t.tc\t%s[tc],%s\n",
                                q->psname, q->psname, q->psname );
#else
                        printf( "%cT.%s:\t.tc\t%s[tc],%s\n",
                                ITEXT, q->psname, q->psname, q->psname );
#endif
                        printf("\t.comm\t%s,%d\n", q->psname, off);
#else
                        printf("\t.comm\t_%s,%d\n", q->psname, off);
#endif
                }
#endif
        }
}

/* -------------------- bfcode -------------------- */

/***********************************************************************
     STORAGE CLASSES AND TYPE DEFINES MAY BE DELETED WHEN XCOFF.H
     INCLUDED.  THESE DEFINITIONS ARE FOUND IN DBXSTORCLASS.H
***********************************************************************/
/*
 *   XCOFF STORAGE CLASSES AND STABSTRINGS DESIGNED SPECIFICALLY FOR DBX
 */
#define  C_FUN          0x8e

bfcode( a, n ) int a[]; {
        /* code for the beginning of a function
         * a is an array of indices in stab for the arguments
         * n is the number of arguments
         */
        register i;
        register TWORD temp;
        register struct symtab *p;
        int off;

        locctr( PROG );
        p = &stab[curftn];
        temp = TOPTYPE(DECREF(p->stype));
        strftn = (temp==STRTY) || (temp==UNIONTY);
        minsvarg = 16;  /* same initialization as in reader.c */

#ifdef  LINT
        /* Check whether to emit old style function definition, or squawk. */
        if( p->stype->ftn_parm != PNIL ){
                if( lintvarg >= 0 )
                        WARNING( WALWAYS, TOOLSTR(M_MSG_322, "prototypes should not use the lint /*VARARGSn*/"));
        }
        lintvarg = -1;          /* reset */
#endif

        retlab = getlab();

        /* routine prolog */

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
#ifdef XCOFF
        if( p->sclass == EXTDEF )
                printf( "\t.globl\t%s[ds]\n", p->psname);
        printf( "\t.csect\t%s[ds]\n", p->psname);
        printf( "\t.long\t.%s[pr]\n", p->psname);
        printf( "\t.long\tTOC[tc0]\n" );
        printf( "\t.long\t0\t# envirnoment variable\n" );
        printf( "\t.toc\t\t# Function entry in toc\n" );
#ifdef ONEPASS
        printf( "T.%s:\t.tc\t.%s[tc],%s[ds]\n",
                p->psname, p->psname, p->psname );
#else
        printf( "%cT.%s:\t.tc\t.%s[tc],%s[ds]\n",
                ITEXT, p->psname, p->psname, p->psname );
#endif
        if( p->sclass == EXTDEF )
                printf( "\t.globl\t.%s[pr]\n", p->psname );
        if( lastloc == PROG)
                locctr( RESET );
        else
                locctr( PROG );
#else
        if( p->sclass == EXTDEF )
                printf( "\t.globl\t.%s\n", p->psname);
#endif
        printf( "\t.align\t1\n" );      /* see defn of ALFTN */
#ifndef XCOFF
        printf( ".%s:\n", p->psname);
#endif

        if( adebug )
                printf("\t.copt\tpdef,%s\n", p->psname);
        if ( gdebug ) {
                TPTR pty = p->stype;
                TWORD bpty = BTYPE(pty);
                int typeID;
                char *tagnm, *strname();
                int savech;

                savech = *tagnm;
#ifdef XCOFF
                printf( "\t.function\t.%s[pr],L.%dB,%d,0%o",
                        p->psname, ftnno, C_EXT, tyencode(pty) );
#else
                printf( "\t.function\t.%s,L.%dB,%d,0%o", p->psname, ftnno,
                  p->sclass == STATIC ? C_STAT|N_TEXT : C_EXT|N_TEXT,
                  tyencode(pty));
#endif
                if(bpty == STRTY || bpty == UNIONTY || bpty == ENUMTY) {
                        tagnm = strname(btype(pty)->typ_size);
                        savech = *tagnm;
                        if( savech == '$' )
                                *tagnm = '_';
                        printf( "%%%s", tagnm );
                }
                printf("\n");
                typeID = getTypeID(p);
#ifdef XCOFF
                printf( "\t.stabx\t\"%s:f%d\",0,%d,0\n",
                                p->psname, typeID, C_FUN);
#endif
        }

# ifdef ONEPASS
        p2ftn(ftnno, 2*aflag + proflg);
# else
#ifdef XCOFF
        printf( "%c%d\t%d\t.%s\n", FBEGIN, ftnno, 2*aflag + proflg, p->psname );
#else
        printf( "%c%d\t%d\t%s\n", FBEGIN, ftnno, 2*aflag + proflg, p->psname );
#endif
# endif
#endif

        off = 0;

        if (strftn) {
                /* result ptr is argument 0 */
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf("\tst\t2,L.%dA(1)\n", ftnno );
#endif
                off = SZINT;
        }

        reginfo = 0;
        param_num = 0;          /* incremented by makearg */

# ifndef ONEPASS
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t%d\t\n", PFHERE, 16,(strftn ? 4 : 0));   /* Optimizer to put param fetches here */
#endif
# else
        ParamInitialize(16,(strftn ? 4 : 0));
# endif

        for( i=0; i<n; ++i )
                makearg(&off,&stab[a[i]]);

# ifndef ONEPASS
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t\n", PSAVE, (param_num + (strftn? 1:0))*4 );   /* Pass2 to save Params */
#endif
# else
        ParamDone( (param_num + (strftn? 1:0))*4 );
# endif

        fdefflag = 1;
        /* initialize line number counters */

        oldln = startln = lineno;
        strcpy( startfn, ftitle );

        /* do .bf symbol and .defs for parameters
         * paramters are delayed to here to two reasons:
         *    1: they appear inside the .bf - .ef
         *    2: the true declarations have been seen
         */
        if ( gdebug ) {
                printf("\t.bf\t%d\n", lineno);
                for( i=0; i<n; ++i ) {
                        p = &stab[a[i]];
                        prdef(p,0);
                        }
                printf("\t.line\t1\n" );
                }

        }

#ifdef XCOFF
/* -------------------- prFTN -------------------- */

prFTN( ) {
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        register struct symtab *p;
        register int i;

        for( i = 0, p = stab; i < nstabents; i++, p++ ){
                if( TOPTYPE(p->stype) == TNULL ) continue;
                if( p->sclass != EXTERN || p->suse > 0 )
                        continue;
                if( ISFTN(p->stype) ){
                        if( p->sflags & SFCALLED ){
                                printf( "\t.extern\t.%s[pr]\n", p->psname );
                        }
                        if( p->sflags & SFADDR ){
                                printf( "\t.extern\t%s[ds]\n", p->psname );
                                printf( "\t.toc\n");
#ifdef ONEPASS
                                printf( "T.%s:\t.tc\t.%s[tc],%s[ds]\n",
                                        p->psname, p->psname, p->psname );
#else
                                printf( "%cT.%s:\t.tc\t.%s[tc],%s[ds]\n",
                                        ITEXT, p->psname, p->psname, p->psname );
#endif
                        }
                } else {
                        if( strcmp( p->psname, "_fpfpf" ) == 0 ){
                                /* Handle _fpfpf specially */
                                printf( "\t.extern\t%s\n", p->psname );
                                printf( "\t.toc\n" );
#ifdef ONEPASS
                                printf( "T.%s:\t.tc\t,%s\n",
                                        p->psname, p->psname );
#else
                                printf( "%cT.%s:\t.tc\t,%s\n", ITEXT,
                                         p->psname, p->psname );
#endif
                        } else {
                                printf( "\t.extern\t%s[rw]\n", p->psname );
                                printf( "\t.toc\n" );
#ifdef ONEPASS
                                printf( "T.%s:\t.tc\t%s[tc],%s[rw]\n",
                                        p->psname, p->psname, p->psname );
#else
                                printf( "%cT.%s:\t.tc\t%s[tc],%s[rw]\n", ITEXT,
                                         p->psname, p->psname, p->psname );
#endif
                        }
                }
        }
#endif
}
#endif

/* -------------------- makearg -------------------- */

makearg(off,p)
int *off;
register struct symtab *p;
{
        register temp;
        register int offset, size;

#ifndef XCOFF
#       ifndef ONEPASS
                StabInfoPrint(p);
#       endif
#endif
        if( p->sclass == REGISTER){
                temp = p->offset;               /* save register number */
                p->sclass = PARAM;        /* forget that it is a register */
                p->offset = NOOFFSET;
                oalloc( p, off );
                offset = p->offset/SZCHAR;

#       ifndef ONEPASS
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "%c%d\t%d\t%d\t\n", PFETCH,
                        temp, offset, tyencode(p->stype));
#endif
                if( ISFLOAT(p->stype) )
                        param_num += 2;
                else
                        param_num++;
#       else
                ParamFetch( temp, offset, p->stype );
#       endif
                if( !ISFLOAT(p->stype) && offset<16 )
                        reginfo |= 8 >> (offset/4);
                p->offset = temp;  /* remember register number */
                p->sclass = REGISTER; /*remember that it is a register*/
                return;
        }
        p->offset = NOOFFSET;
        if( oalloc( p, off ) ) cerror(TOOLSTR(M_MSG_272, "bad argument" ));
        if (TOPTYPE(p->stype) == STRTY || TOPTYPE(p->stype) == UNIONTY)
                SETOFF( *off, ALSTACK );

        offset = p->offset/SZCHAR;
        offset &= -4;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        if (TOPTYPE(p->stype) == FLOAT)
                printf("%c%d\t%d\t%d\t \n", PFETCH, -1, offset,
                        tyencode(p->stype));
#endif
        /* Note to KR: FLOAT are converted into DOUBLE. Also, LDOUBLE */
        size = tsize( ISFLOAT(p->stype) ? tyalloc(DOUBLE) : p->stype ) / SZCHAR;
        while ( size>0 && offset<16 ) {
#               ifdef ONEPASS
                        StoreParam( offset );
#               else
                        param_num++;
#               endif
                offset += 4;
                size -= 4;
        }
}

/* -------------------- beg_file -------------------- */

beg_file() {
        /* called as the very first thing by the parser to do machine
         * dependent stuff
         */
        register char * p;
        register char * s;

                        /* note: double quotes already in ftitle... */
        p = ftitle + strlen( ftitle ) - 2;
        s = p - 14;     /* max name length */
        while ( p > s && *p != '"' && *p != '/' )
                --p;

        /* always emit .file - makes sdb users very happy */
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "\t.file\t\"%.15s\n", p + 1 );

#ifdef XCOFF
        /* set up General Data Csect */
        printf( "\t.toc\n");
#ifdef ONEPASS
        printf( "T.%s:\t.tc\t%s[tc],%s[rw]\n",
                data_sect, data_sect, data_sect);
#else
        printf( "%cT.%s:\t.tc\t%s[tc],%s[rw]\n",
                ITEXT, data_sect, data_sect, data_sect);
#endif
#endif
#endif
}

/* -------------------- bccode -------------------- */

bccode() {
        /* called just before the first executable statement
         * by now, the automatics and register variables are allocated
         */

        SETOFF( autooff, SZINT );

        /* this is the block header:
         * autooff is the max offset for auto and temps
         * regvar is the least numbered register variable
         * ftnno is the function's unique number, used for labels, etc.
         * fpregvar is the regvar analog for floating point registers
         */

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
# ifdef ONEPASS
        p2bbeg(ftnno, autooff, regvar, fpregvar );
# else
        printf( "%c%d\t%d\t%d\t%d\t\n",
                 BBEG, ftnno, autooff, regvar, fpregvar );
# endif
#endif
        }

/* -------------------- ejobcode -------------------- */

ejobcode( flag ){
        /*
        ** Called just before final exit;
        **      flag is 1 if errors, 0 if none
        */
        register struct symtab *p;
        register int i;
        int saveln;
        char *saveifnm;

        saveln = lineno;
#if     defined (LINT) || defined (CFLOW)
        saveifnm = ifname;
#endif
        for( i = 0, p = stab; i < nstabents; i++, p++ ){
                if( TOPTYPE(p->stype) == TNULL ) continue;
                /* next two lines changed/added for #72129 */
#if             defined (LINT) || defined (CFLOW)
                lineno = p->line;
                ifname = p->ifname;	
#else
                if( ( lineno = p->suse) < 0)
                        lineno = - p->suse;
#endif
                switch( p->sclass ){
                case USTATIC:
                        if( ISFTN(p->stype) ){
                                if( p->suse < 0 ){
                                        /* "static function %s not defined" */
                                        UERROR( ALWAYS, MESSAGE(162),
                                                p->psname );
                                } else if( devdebug[REFDEF] || WANSI ){
                                        /* "static function %s not defined or used" */
                                        WERROR( devdebug[REFDEF] && WKNR, MESSAGE(163),
                                                p->psname );
                                } else {
                                        /* "static function %s not defined or used" */
                                        WARNING( WUSAGE && WKNR, MESSAGE(163),
                                                p->psname );
                                }
                                break;
                        }
                case STATIC:
                        if( p->suse >= 0 ){
                                /* "static %s %s unused" */
                                WARNING( WUDECLAR, MESSAGE(101),
                                        ISFTN(p->stype)?"function":"variable",
                                        p->psname );
                        }
                        break;

                case EXTERN:
                case EXTDEF:
                        break;

                case STNAME:
                case UNAME:
                        if( dimtab[p->stype->typ_size] == 0 )
                                /* "struct/union %s never defined" */
                                WARNING( WDECLAR || WHEURISTIC, MESSAGE(102), p->psname );
                        break;
                }
        }
        lineno = saveln;
#if     defined (LINT) || defined (CFLOW)
        ifname = saveifnm;
#endif
}

/* -------------------- aobeg -------------------- */

aobeg(){
        /* called before removing automatics from stab */
        }

/* -------------------- aocode -------------------- */

aocode(p) register struct symtab *p; {
        /*
        ** Called when automatic p removed from stab.
        */
        switch( p->sclass ) {
        case PARAM:

                if( p->suse > 0 && paramchk && p->stype->tword != TELLIPSIS )
                  {
                        /* Aug. 24 GH Warning about arguments unused in
                                functions is now not dependent on the
                                the -u flag (Xopen compliance) */
                        /* "argument %s unused in function %s" */
#ifdef LINT
                        WARNING( vflag && !lintargu && !lintlib && WKNR,
                                MESSAGE(13), p->psname, stab[curftn].psname );
#else
                        WARNING( WUSAGE && !lintargu && !lintlib, MESSAGE(13),
                                p->psname, stab[curftn].psname );
#endif
                }
                break;
        case PARAMREG:
                if( p->suse > 0 && paramchk && p->stype->tword != TELLIPSIS )
                  {
                        /* Aug. 24 GH Warning about arguments unused in
                                functions is now not dependent on the
                                the -u flag (Xopen compliance) */
                        /* "argument %s unused in function %s" */
#ifdef LINT
                        WARNING(!lintlib && WKNR,
                                MESSAGE(13), p->psname, stab[curftn].psname );
#else
                        WARNING( WUSAGE && !lintargu && !lintlib, MESSAGE(13),
                                p->psname, stab[curftn].psname );
#endif
                }
        case PARAMFAKE:
        case EXTERN:
        case EXTDEF:
                break;

        case STNAME:
        case UNAME:
                if( dimtab[p->stype->typ_size] == 0 ){
                        /* "struct/union %s never defined" */
                        WARNING( WDECLAR || WHEURISTIC, MESSAGE(102), p->psname );
                        break;
                }
                /* fallthrough */

        default:

                if( p->suse > 0 && !( (p->sflags&SNSPACE) == SMOS || p->sclass == MOE) )
                        /* "%s unused in function %s" */
                        WARNING( WUSAGE, MESSAGE(6), p->psname,
                                stab[curftn].psname );
                else if( (p->sflags & (SSET|SREF|SMOS)) == SSET &&
                        !ISARY(p->stype) && !ISFTN(p->stype) )
                        /* "%s set but not used in function %s" */
                        WARNING( WUSAGE && WKNR, MESSAGE(3), p->psname,
                                stab[curftn].psname );
                break;
        }
}

/* -------------------- aoend -------------------- */

aoend(){
        /* called after removing all automatics from stab */
        if( gdebug && bb_flags[blevel+1] ) {
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf( "\t.eb\t%d\n", Lineno);
#endif
                bb_flags[blevel+1] = 0;
        }
        }

/* -------------------- efcode -------------------- */

efcode()
{
        /* code for the end of a function */
        register struct symtab *p = &stab[curftn];
        register int j;

        if( retstat == RETVAL+NRETVAL )
                /* "function %s has return(e); and return;" */
                WARNING( WRETURN, MESSAGE( 43 ), p->psname);
        /*
        * See if main() falls off its end or has just a return;
        */
        if (!strcmp(p->psname, "main") && (reached || (retstat & NRETVAL)))
                /* "main() returns random value to invocation environment" */
                WARNING( WRETURN, MESSAGE( 171 ) );

# ifndef ONEPASS
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c\n", FEND );
#endif
# endif
#ifdef XCOFF
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "\tnop\n" );    /* in case ?cond:stmnt:stmnt last func line */
#endif
        unbuffer_str();
#endif
        deflab( retlab );
        /* used to copy in epilog, now we let RETURN do structure
           copy as it normally generates one */
        if( strftn )
        {       /* copy output (in R2) to caller */
                register NODE *l, *r;
                register TPTR t;
                int i;

                t = INCREF(DECREF(p->stype), PTR);

                /* get saved address of return structure */
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
                printf("\tl\t3,L.%dA(1)\n",ftnno);
#endif

                reached = 1;
                l = block(REG, NIL, NIL, t);
                l->tn.rval = 3;  /* R3 */
                l->tn.lval = 3;
                r = block(REG, NIL, NIL, t);
                r->tn.rval = 2;  /* R2 */
                r->tn.lval = 2;
                l = buildtree( UNARY MUL, l, NIL );
                r = buildtree( UNARY MUL, r, NIL );
                l = buildtree( ASSIGN, l, r );
                l->in.op = FREE;
                ecomp( l->in.left );
                /* turn off strftn flag, so return sequence will be generated */
                strftn = 0;
                }

        /* print end-of-function pseudo and its line number */

        if ( gdebug ){
                printf( "\t.ef\t%d\n", Lineno);
                if( Lineno > 1 )
                        printf( "\t.line\t%d\n", Lineno );
                }

        /* emit epilog, prolog, constant pool */
        /* can emit return here */
# ifdef ONEPASS
        p2bend(1,reginfo);
        eobp2(p->psname,0,0,p->sclass!=EXTDEF);
# else
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t%d\t%d\t\n",
                BEND, minsvarg, reginfo, p->sclass!=EXTDEF);
#endif
# endif

        fdefflag = 0;
        }

/* -------------------- bycode -------------------- */

bycode( t, i ){
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        /* put byte i+1 in a string */

        i &= 07;
        if( t < 0 ){ /* end of the string */
                if( i != 0 ) printf( "\n" );
                }

        else { /* stash byte t into string */
                if( i == 0 ) printf( "\t.byte   " );
                else printf( "," );
                printf( "0x%x", t );
                if( i == 07 ) printf( "\n" );
                }
#endif
        }

/* -------------------- zecode -------------------- */

zecode( n ){
        /* n integer words of zeros */
        OFFSZ temp;
        if( n <= 0 ) return;
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "\t.space\t%d\n", (SZINT/SZCHAR)*n );
#endif
        temp = n;
        inoff += temp*SZINT;
        }

/* -------------------- fldty -------------------- */

fldty( p ) struct symtab *p; { /* fix up type of field p */
        ;
        }

/* -------------------- swprolog -------------------- */

swprolog( swlab, swexpr )
        int swlab;
        NODE *swexpr;
{
        register NODE *temp;
        temp = buildtree( FORCE, swexpr, NIL );
        temp->tn.rval = FORCEREG;
        ecomp(temp);
        branch( swlab );
}

/* -------------------- swepilog -------------------- */

swepilog( p, n )
        register struct sw *p;
        register int n;
{
        /*      p points to an array of structures, each consisting
                of a constant value and a label.
                The first is >=0 if there is a default label;
                its value is the label number
                The entries p[1] to p[n] are the nontrivial cases
                */

# ifdef ONEPASS
        genswitch( p, n, 0 );   /* someday this 0 will be a register number  */
# else

        register int i;

                /* front end of opt cannot have switch fall through  */
        if( p[0].slab < 0 )
                p[0].slab = brklab;

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t%d\t%d\t\n", ESWITCH, n, 0, p[0].slab ); /* this 0 too */
        for( i = 1; i <= n; i++ )
        {
                printf( CONFMT, p[i].sval );
                printf( "\t%d\t\n", p[i].slab );
        }
#endif
# endif
}

#ifdef  LINT
int wloop_level = LL_TOP;       /* cause no unnecessary set/usage complaints */
int floop_level = LL_TOP;       /* cause no unnecessary set/usage complaints */
#else
int wloop_level = LL_BOT;       /* "while" loop test at loop bottom */
int floop_level = LL_BOT;       /* "for" loop test at loop bottom   */
#endif

char *tmpname = "/tmp/pcXXXXXX";

/* -------------------- Mcode -------------------- */

loadcon(reg,value)
{
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf("\tcau\t%d,%d\n", reg, value >> 16 );
        printf("\toil\t%d,%d,%d\n", reg, reg, value & 0xffff );
#endif
}

/* -------------------- main  -------------------- */

main( argc, argv ) int argc; register char *argv[]; {
#ifdef AIXV2
        extern fpe_catch(), dexit();
#else
        extern fpe_catch(int f), dexit();
#endif /* AIXV2 */
        register int i, c;
        register int fdef = 0;
        register char *cp;
        register int r;
        int operandflg = 0;
#if     defined (LINT) || defined (CFLOW)
        char *lntname;
#endif


#ifndef AIXV2
        setlocale(LC_ALL, "");
#endif
/*GH 27206*/
catd = catopen(MF_CTOOLS, NL_CAT_LOCALE); /*P51131*/
        for( i = 0; i <= MXDBGFLG; i++ ){
                devdebug[i] = 0;
                warnlevel[i] = 0;
        }

        for( i=1; i<argc; ++i ){
                if(!strcmp(argv[i], "--"))
                {
                    operandflg = 1;
                    i++;
                }
                if( *(cp=argv[i]) == '-' && !operandflg)
                while( *++cp ){
                    switch( *cp ){
                    /* Emulate the f77 -N option for resetting
                    the size of the symbol table(stab). (lec) */
#if defined (LINT) || defined (CFLOW)
                    case 'f':
                        {
                            cp++;
                            pfname=getmem(strlen(cp) + 1);
                            strcpy(pfname, cp);
                            cp += strlen(cp) - 1;
                            break;
                        }
#endif
                    case 'N':
                        do {
                            register tobe = 0;
                            int dflg = 0;
                            int gflg = 0;
                            int lflg = 0;
                            int nflg = 0;
                            int tflg = 0;
                            int pflg = 0;

                            for( ;; ) {
                                switch( *++cp ){
                                case 'd':       dflg++; continue;
                                case 'g':       gflg++; continue;
                                case 'l':       lflg++; continue;
                                case 'n':       nflg++; continue;
                                case 'p':       pflg++; continue;
                                case 't':       tflg++; continue;
                                }
                                break;
                            }
                            if( !isdigit( *cp ) || !(dflg || gflg || lflg || nflg || pflg || tflg))
                                cerror(TOOLSTR(M_MSG_273, "An argument of the -N option is not correct."));
                            while( isdigit( *cp ) )
                                tobe = tobe*10 + *cp++ - '0';
                            if( tobe > 0 ){
                                if( dflg ) ndiments = tobe;
                                if( gflg ) nGlobTyEnts = tobe;
                                if( lflg ) nLoclTyEnts = tobe;
                                if( nflg ) nstabents = tobe;
                                if( tflg ) ntrnodes = tobe;
                            } else
                                cerror(TOOLSTR(M_MSG_273, "bad -N options"));
                        } while( *cp-- != '\0' );
                        break;

                    case 'w': /* toggle warning level */
                            while( *++cp )
                              {
                                switch(*cp)
                                {
                                case 'A':
                                /*if( *cp == 'A' )*/
                                        for( r = 0; r <= MXDBGFLG; r++ )
                                                warnlevel[r] = !warnlevel[r];
                                break;
                                case 'u':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'l':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'h':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'R':
                                        warnlevel[*cp] = 0;
                                        break;
                                case 'D':
                                        warnlevel[*cp] = 0;
                                        break;
                                default:
                                        if( ( *cp >= 'a' && *cp <= 'z' ) ||
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

#ifdef LINT
                    case 'v': /* toggle parameter usage checking */
                            vflag = 0;
                            break;
#endif

                    case 'X': /* flags local to pass 1 - historical since */
                              /* /bin/cc has special processing for these */
                            while( *++cp ){
                                switch( *cp ){
                                case 'P':       /* profiling */
                                        proflg=1;
                                        break;

                                case 'g':       /* SDB option */
                                        ++gdebug;
                                        break;
                                }
                            }
                            --cp;
                            break;
#if !defined (LINT) && !defined (CXREF) && !defined (CFLOW)
                    case 'y':
                            while( *++cp ){
                                int fpstat = (*(unsigned (*) ())_fpfpf[FP_getst])();

                                switch( *cp ){
                                case 'd':       /* no folding of f.p. consts */
                                        nofpflding=1;
                                        break;

                                case 'm':       /* Round towards -inf. */
                                        ((FP_STATUS *)&fpstat)->rnd_mode = FP_DOWN;
                                        (*_fpfpf[FP_setst])(fpstat);
                                        break;

                                case 'n':       /* Round towards nearest num. */
                                        ((FP_STATUS *)&fpstat)->rnd_mode = FP_NEAR;
                                        (*_fpfpf[FP_setst])(fpstat);
                                        break;

                                case 'p':       /* Round towards +inf. */
                                        ((FP_STATUS *)&fpstat)->rnd_mode = FP_UP;
                                        (*_fpfpf[FP_setst])(fpstat);
                                        break;

                                case 'z':       /* Round towards 0. */
                                        ((FP_STATUS *)&fpstat)->rnd_mode = FP_ZERO;
                                        (*_fpfpf[FP_setst])(fpstat);
                                        break;

                                default:
                                        goto out;
                                }
                            }
out:
                            --cp;
                            break;
#endif

                    case 'O':       /* optimize; round-robin */
                            if (adebug==0) adebug++;
                            break;

                    case 'a':       /* extended addressing */
                            aflag=1;
                            break;
                    case 'q':       /* reg char/short option */
                            ++qdebug;
                            break;
                    case 'P':       /* profiling */
                            proflg=1;
                            break;

                    case 'g':       /* SDB option */
                            ++gdebug;
                            break;
#if     defined (LINT) || defined (CFLOW)
                    case 'L':       /* LINT temp filename */
                            lntname = (char *) &argv[i][2];
                            while (*(cp+1)) ++cp;       /* Since cp will get incremented by
                                                           the outside while loop, need to
                                                           find the last character of this
                                                           argument. */

                            break;
#endif
                    }
                }

                else if (fdef == 0) {   /* input file */
                        if (freopen(argv[i], "r", stdin) == NULL) {
                            fprintf(stderr, TOOLSTR(M_MSG_299, "ccom:can't open %s\n"), argv[i]);
                            exit(2);
                        }
                        ++fdef;
                }

                else if (fdef == 1) {   /* output file */
#if     !defined (LINT) && !defined (CFLOW)
#ifdef  CXREF
                        if (freopen(argv[i], "a", stdout) == NULL) {
                            fprintf(stderr, TOOLSTR(M_MSG_300, "cxref: can't open %s\n"), argv[i]);
#else
                        if (freopen(argv[i], "w", stdout) == NULL) {
                            fprintf(stderr, TOOLSTR(M_MSG_299, "ccom: can't open %s\n"), argv[i]);
#endif
                            exit(2);
                        }
#endif
                        ++fdef;
                }
        } /* end of for loop */

        /* check for ANSI mode */
        if ( devdebug[ANSI_MODE] ) {
                /* set all other ANSI flags on */
                devdebug[ANSI_PARSE]    = !devdebug[ANSI_PARSE];
                devdebug[COMPATIBLE]    = !devdebug[COMPATIBLE];
                devdebug[PROMOTION]     = !devdebug[PROMOTION];
                devdebug[REFDEF]        = !devdebug[REFDEF];
                devdebug[SCOPING]       = !devdebug[SCOPING];
                devdebug[STRUCTBUG]     = !devdebug[STRUCTBUG];
                devdebug[TYPING]        = !devdebug[TYPING];
        }

#if     defined (LINT) || defined (CFLOW)
        if ((tmplint = fopen(lntname, "a")) == NULL) {
                fprintf(stderr, TOOLSTR(M_MSG_301, "lint: can't open lint temporary file\n"));
                exit(2);
        }
#endif

#ifdef AIXV2
        if(signal( SIGFPE, SIG_IGN) != SIG_IGN) signal(SIGFPE,fpe_catch);
#else
        if(signal( SIGFPE, SIG_IGN) != SIG_IGN) signal(SIGFPE, (void (*)(int)) fpe_catch);
#endif /* AIXV2 */

# ifdef TEMPFILE
        mktemp(tmpname);
        if(signal( SIGHUP, SIG_IGN) != SIG_IGN) signal(SIGHUP, dexit);
        if(signal( SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, dexit);
        if(signal( SIGTERM, SIG_IGN) != SIG_IGN) signal(SIGTERM, dexit);
        tempfile = fopen( tmpname, "w" );
# endif
        r = mainp1( argc, argv );

# ifdef TEMPFILE
        tempfile = freopen( tmpname, "r", tempfile );
        if( tempfile != NULL )
                while((c=getc(tempfile)) != EOF )
                        putchar(c);
        else cerror(TOOLSTR(M_MSG_274, "Lost temp file" ));
        unlink(tmpname);
# endif

#if     defined (LINT) || defined (CFLOW)
        fclose(tmplint);        /* unlinked by lint script */
#endif
        return( r );

        }

/* -------------------- dexit -------------------- */

dexit( v ) {
        unlink(tmpname);
        exit(2);
        }

/* -------------------- fpe_catch -------------------- */

#ifdef AIXV2
fpe_catch( f ) {
#else
fpe_catch( int f ) {
#endif /* AIXV2 */
        /* catch floating point exception probably caused
           during constant folding */
        /* "floating point exception detected" */
        UERROR( ALWAYS, MESSAGE(161) );
#ifdef  TEMPFILE
        dexit();
#endif
}

/* -------------------- retgen -------------------- */

retgen( type, label )
        register TPTR type;
        register int label;
{
# ifdef ONEPASS
        branch( label );
# else
        /* At present, pass 2 does not look at the number of registers
         * returned.  If it ever does, this will have to be calculated
         * as
         *      ( (type == TVOID) ? 0 : (ISFLOAT(type) ? 2 : 1 ))
         *
         * For now, just print a 0 here
         */
#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
        printf( "%c%d\t%d\t\n", IRETURN, label, 0);
#endif
# endif
}

/* -------------------- EmitVolatile -------------------- */

EmitVolatile()
{
        if( adebug )
                printf("\t.copt\tvolatile\n");
}
