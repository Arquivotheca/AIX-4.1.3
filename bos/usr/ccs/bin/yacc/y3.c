static char sccsid[] = "@(#)24  1.8  src/bos/usr/ccs/bin/yacc/y3.c, cmdlang, bos411, 9428A410j 12/17/93 11:51:46";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: apack, go2gen, go2out, hideprod, output,
                precftn, warray, wdef, wract, wrstate
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "CSfile: y3.c,v Revision: 2.4  (OSF) Date: 90/
10/07 17:48:59 "; */
# include "dextern"
#ifndef _BLD
#include "yacc_msg.h"
extern nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_YACC,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif

        /* important local variables */
int lastred;  /* the number of the last reduction of a state */
int *defact;  /* the default actions of states */

output(){ /* print the output for the states */

        int i, k, c;
        register struct wset *u, *v;

        fprintf( ftable, "yytabelem yyexca[] ={\n\t" );

        SLOOP(i) { /* output the stuff for state i */
                nolook = !(tystate[i]==MUSTLOOKAHEAD);
                closure(i);
                /* output actions */
                nolook = 1;
                aryfil( temp1, ntokens+nnonter+1, 0 );
                WSLOOP(wsets,u){
                        c = *( u->pitem );
                        if( c>1 && c<NTBASE && temp1[c]==0 ) {
                                WSLOOP(u,v){
                                        if( c == *(v->pitem) )
                                                putitem( v->pitem+1,
                                                        (struct looksets *)0 );
                                        }
                                temp1[c] = state(c);
                                }
                        else if( c > NTBASE &&
                                temp1[ (c -= NTBASE) + ntokens ] == 0 ){
                                temp1[ c+ntokens ] = amem[indgo[i]+c];
                                }
                        }

                if( i == 1 ) temp1[1] = ACCEPTCODE;

                /* now, we have the shifts; look at the reductions */

                lastred = 0;
                WSLOOP(wsets,u){
                        c = *( u->pitem );
                        if( c<=0 ){ /* reduction */
                                lastred = -c;
                                TLOOP(k){
                                        if( BIT(u->ws.lset,k) ){
                                                  if( temp1[k] == 0 )
                                                        temp1[k] = c;
                                                  else if( temp1[k]<0 ){
                                                  /* reduce/reduce conflict */
                                                    if( foutput!=NULL )
                                                      fprintf( foutput,
                                                        MSGSTR(REDRED1, "\n%d: reduce/reduce conflict (red'ns %d and %d ) on %s"), 
                                                        i, -temp1[k], lastred,
                                                        symnam(k) );
                                                    if( -temp1[k] > lastred )
                                                        temp1[k] = -lastred;
                                                    ++zzrrconf;
                                                    }
                                                  else {
                                                  /* potential shift/reduce
                                                   * conflict */
                                                        precftn(lastred, k, i);
                                                        }
                                                }
                                        }
                                }
                        }
                wract(i);
                }

        fprintf( ftable, "\t};\n" );

        wdef( "YYNPROD", nprod );

        }

int pkdebug = 0;
apack(p, n ) int *p;{ /* pack state i from temp1 into amem */
        int off;
        register *pp, *qq, *rr;
        int *q, *r;

        /* we don't need to worry about checking because we
           we will only look entries known to be there... */

        /* eliminate leading and trailing 0's */

        q = p+n;
        for( pp=p,off=0 ; *pp==0 && pp<=q; ++pp,--off ) /* VOID */ ;
        if( pp > q ) return(0);  /* no actions */
        p = pp;

        /* now, find a place for the elements from p to q, inclusive */

        r = &amem[actsize-1];
        for( rr=amem; rr<=r; ++rr,++off ){  /* try rr */
                for( qq=rr,pp=p ; pp<=q ; ++pp,++qq){
                        if( *pp != 0 ){
                                if( *pp != *qq && *qq != 0 ) goto nextk;
                                }
                        }

                /* we have found an acceptable k */

                if( pkdebug && foutput!=NULL ) fprintf( foutput, MSGSTR(OFF1,
                        "off = %d, k = %d\n"), off, rr-amem ); /*MSG*/

                for( qq=rr,pp=p; pp<=q; ++pp,++qq ){
                        if( *pp ){
                                if( qq > r ) OUTBUFMSG;
                                if( qq>memp ) memp = qq;
                                *qq = *pp;
                                }
                        }
                if( pkdebug && foutput!=NULL ){
                        for( pp=amem; pp<= memp; pp+=10 ){
                                fprintf( foutput, "\t");
                                for( qq=pp; qq<=pp+9; ++qq )
                                        fprintf( foutput, "%d ", *qq );
                                fprintf( foutput, "\n");
                                }
                        }
                return( off );

                nextk: ;
                }
        OUTBUFMSG;
        /* NOTREACHED */
        }

go2out(){ /* output the gotos for the nontermninals */
        int i, j, k, best, count, cbest, times;

        fprintf( ftemp, "$\n" );  /* mark begining of gotos */

        for( i=1; i<=nnonter; ++i ) {
                go2gen(i);

                /* find the best one to make default */

                best = -1;
                times = 0;

                for( j=0; j<=nstate; ++j ){ /* is j the most frequent */
                        if( tystate[j] == 0 ) continue;
                        if( tystate[j] == best ) continue;

                        /* is tystate[j] the most frequent */

                        count = 0;
                        cbest = tystate[j];

                        for( k=j; k<=nstate; ++k )
                                if( tystate[k]==cbest ) ++count;

                        if( count > times ){
                                best = cbest;
                                times = count;
                                }
                        }

                /* best is now the default entry */

                zzgobest += (times-1);
                for( j=0; j<=nstate; ++j ){
                        if( tystate[j] != 0 && tystate[j]!=best ){
                                fprintf( ftemp, "%d,%d,", j, tystate[j] );
                                zzgoent += 1;
                                }
                        }

                /* now, the default */

                zzgoent += 1;
                fprintf( ftemp, "%d\n", best );

                }



        }

int g2debug = 0;
go2gen(c){ /* output the gotos for nonterminal c */

        int i, work, cc;
        struct item *p, *q;


        /* first, find nonterminals with gotos on c */

        aryfil( temp1, nnonter+1, 0 );
        temp1[c] = 1;

        work = 1;
        while( work ){
                work = 0;
                PLOOP(0,i){
                        if( (cc=prdptr[i][1]-NTBASE) >= 0 ){
                        /* cc is a nonterminal */
                                if( temp1[cc] != 0 ){ /* cc has a goto on c */
                                        cc = *prdptr[i]-NTBASE;
                                        /* thus, the left side of
                                         * production i does too */
                                        if( temp1[cc] == 0 ){
                                                  work = 1;
                                                  temp1[cc] = 1;
                                                  }
                                        }
                                }
                        }
                }

        /* now, we have temp1[c] = 1 if a goto on c in closure of cc */

        if( g2debug && foutput!=NULL ){
                fprintf( foutput, MSGSTR(GOTOON, "%s: gotos on "),
                        nontrst[c].name ); /*MSG*/
                NTLOOP(i) if( temp1[i] ) fprintf( foutput, "%s ",
                                nontrst[i].name);
                fprintf( foutput, "\n");
                }

        /* now, go through and put gotos into tystate */

        aryfil( tystate, nstate, 0 );
        SLOOP(i){
                ITMLOOP(i,p,q){
                        if( (cc= *p->pitem) >= NTBASE ){
                                if( temp1[cc -= NTBASE] ){
                                /* goto on c is possible */
                                        tystate[i] = amem[indgo[i]+c];
                                        break;
                                        }
                                }
                        }
                }
        }

precftn(r,t,s){ /* decide a shift/reduce conflict by precedence.
         * r is a rule number, t a token number
         * the conflict is in state s
         * temp1[t] is changed to reflect the action */

        int lp,lt, action;

        lp = levprd[r];
        lt = toklev[t];
        if( PLEVEL(lt) == 0 || PLEVEL(lp) == 0 ) {
                /* conflict */
                if( foutput != NULL ) fprintf( foutput, MSGSTR(SFTRED1, "\n%d: shift/reduce conflict (shift %d, red'n %d) on %s"), /*MSG*/
                                                s, temp1[t], r, symnam(t) );
                ++zzsrconf;
                return;
                }
        if( PLEVEL(lt) == PLEVEL(lp) ) action = ASSOC(lt);
        else if( PLEVEL(lt) > PLEVEL(lp) ) action = RASC;  /* shift */
        else action = LASC;  /* reduce */

        switch( action ){

        case BASC:  /* error action */
                temp1[t] = ERRCODE;
                return;

        case LASC:  /* reduce */
                temp1[t] = -r;
                return;

                }
        }

wract(i){ /* output state i */
        /* temp1 has the actions, lastred the default */
        int p, p0, p1;
        int ntimes, tred, count, j;
        int flag;

        /* find the best choice for lastred */

        lastred = 0;
        ntimes = 0;
        TLOOP(j){
                if( temp1[j] >= 0 ) continue;
                if( temp1[j]+lastred == 0 ) continue;
                /* count the number of appearances of temp1[j] */
                count = 0;
                tred = -temp1[j];
                levprd[tred] |= REDFLAG;
                TLOOP(p){
                        if( temp1[p]+tred == 0 ) ++count;
                        }
                if( count >ntimes ){
                        lastred = tred;
                        ntimes = count;
                        }
                }

        /* for error recovery, arrange that, if there is a shift on the
         * error recovery token, `error', that the default
         * be the error action */
        if( temp1[1] > 0 ) lastred = 0;

        /* clear out entries in temp1 which equal lastred */
        TLOOP(p) if( temp1[p]+lastred == 0 )temp1[p]=0;

        wrstate(i);
        defact[i] = lastred;

        flag = 0;
        TLOOP(p0){
                if( (p1=temp1[p0])!=0 ) {
                        if( p1 < 0 ){
                                p1 = -p1;
                                goto exc;
                                }
                        else if( p1 == ACCEPTCODE ) {
                                p1 = -1;
                                goto exc;
                                }
                        else if( p1 == ERRCODE ) {
                                p1 = 0;
                                goto exc;
                        exc:
                                if( flag++ == 0 ) fprintf( ftable, "-1, %d,\n",
                                        i );
                                fprintf( ftable, "\t%d, %d,\n",
                                        tokset[p0].value, p1 );
                                ++zzexcp;
                                }
                        else {
                                fprintf( ftemp, "%d,%d,", tokset[p0].value,
                                        p1 );
                                ++zzacent;
                                }
                        }
                }
        if( flag ) {
                defact[i] = -2;
                fprintf( ftable, "\t-2, %d,\n", lastred );
                }
        fprintf( ftemp, "\n" );
        return;
        }

wrstate(i){ /* writes state i */
        register j0,j1;
        register struct item *pp, *qq;
        register struct wset *u;

        if( foutput == NULL ) return;
        fprintf( foutput, "\nstate %d\n",i);
        ITMLOOP(i,pp,qq) fprintf( foutput, "\t%s\n", writem(pp->pitem));
        if( tystate[i] == MUSTLOOKAHEAD ){
                /* print out empty productions in closure */
                WSLOOP( wsets+(pstate[i+1]-pstate[i]), u ){
                        if( *(u->pitem) < 0 ) fprintf( foutput, "\t%s\n",
                                writem(u->pitem) );
                        }
                }

        /* check for state equal to another */

        TLOOP(j0) if( (j1=temp1[j0]) != 0 ){
                fprintf( foutput, "\n\t%s  ", symnam(j0) );
                if( j1>0 ){ /* shift, error, or accept */
                        if( j1 == ACCEPTCODE ) fprintf( foutput,
                                 MSGSTR(ACCEPT, "accept" )); /*MSG*/
                        else if( j1 == ERRCODE ) fprintf( foutput,
                                MSGSTR(MERROR, "error") ); /*MSG*/
                        else fprintf( foutput,  MSGSTR(SHIFT, "shift %d"),
                                j1 ); /*MSG*/
                        }
                else fprintf( foutput, MSGSTR(EREDUCE, "reduce %d"),
                        -j1 ); /*MSG*/
                }

        /* output the final production */

        if( lastred ) fprintf( foutput, MSGSTR(CRREDUCE,
                "\n\t.  reduce %d\n\n"), lastred ); /*MSG*/
        else fprintf( foutput, MSGSTR(CRERROR, "\n\t.  error\n\n") ); /*MSG*/

        /* now, output nonterminal actions */

        j1 = ntokens;
        for( j0 = 1; j0 <= nnonter; ++j0 ){
                if( temp1[++j1] ) fprintf( foutput, "\t%s  goto %d\n",
                        symnam( j0+NTBASE), temp1[j1] );
                }

        }

wdef( s, n ) char *s; { /* output a definition of s to the value n */
        fprintf( ftable, "# define %s %d\n", s, n );
        }

warray( s, v, n ) char *s; int *v, n; {

        register i;

        fprintf( ftable, "yytabelem %s[]={\n", s );
        for( i=0; i<n; ){
                if( i%10 == 0 ) fprintf( ftable, "\n" );
                fprintf( ftable, "%6d", v[i] );
                if( ++i == n ) fprintf( ftable, " };\n" );
                else fprintf( ftable, "," );
                }
        }

hideprod(){
        /* in order to free up the mem and amem arrays for the optimizer,
         * and still be able to output yyr1, etc., after the sizes of
         * the action array is known, we hide the nonterminals
         * derived by productions in levprd.
        */

        register i, j;

        j = 0;
        levprd[0] = 0;
        PLOOP(1,i){
                if( !(levprd[i] & REDFLAG) ){
                        ++j;
                        if( foutput != NULL ){
                                fprintf( foutput, MSGSTR(NOTRED,
                                        "Rule not reduced:   %s\n"),
                                        writem( prdptr[i] ) ); /*MSG*/
                                }
                        }
                levprd[i] = *prdptr[i] - NTBASE;
                }
        if( j ) fprintf( stderr, MSGSTR(NEVRRED, "%d rules never reduced\n"),
                j ); /*MSG*/
        }
