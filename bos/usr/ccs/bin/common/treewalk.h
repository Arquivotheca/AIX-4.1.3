/* @(#)58       1.4.2.1  src/bos/usr/ccs/bin/common/treewalk.h, cmdprog, bos411, 9428A410j 4/29/93 08:13:58 */
/*
 * COMPONENT_NAME: (CMDPROG) treewalk.h
 *
 * FUNCTIONS: fwalk, tprint, walkf                                           
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
/* -------------------- fwalk -------------------- */

fwalk( t, f, down ) register NODE *t; int (*f)(); {

        int down1, down2;

        more:
        down1 = down2 = 0;

        (*f)( t, down, &down1, &down2 );

        switch( optype( t->in.op ) ){

        case BITYPE:
                fwalk( t->in.left, f, down1 );
                t = t->in.right;
                down = down2;
                goto more;

        case UTYPE:
                t = t->in.left;
                down = down1;
                goto more;

                }
        }

/* -------------------- walkf -------------------- */

walkf( t, f ) register NODE *t;  int (*f)(); {
        register opty;

        opty = optype(t->in.op);

        if( opty == UTYPE || opty == BITYPE) walkf( t->in.left, f );
        if( opty == BITYPE ) walkf( t->in.right, f );
        (*f)( t );
        }


/* -------------------- tprint -------------------- */
#ifndef PASS_TWO
# ifndef BUG4
tprint( t )
TPTR t;
{ /* output a nice description of the type of t */
extern const char * tnames[];

        PPTR p;
        TWORD bt;

        for( ;; t = DECREF(t) ){

                if( ISCONST(t) ) printf( "const " );
                if( ISVOLATILE(t) ) printf( "volatile " );

                if( ISPTR(t) ) printf( "PTR " );
                else if( ISREF(t) ) printf( "REF " );
                else if( ISMEMPTR(t) ) printf( "MEMPTR " );
                else if( ISFTN(t) ){
                        printf( "FTN (" );
                        if( ( p = t->ftn_parm ) != PNIL ){
                                for( ;; ){
                                        tprint( p->type );
                                        if( ( p = p->next ) == PNIL ) break;
                                        printf( ", " );
                                }
                        }
                        printf( ") " );
                }
                else if( ISARY(t) ) printf( "ARY[%.0d] ", t->ary_size );
                else {
                        if( ISTSIGNED(t) ) printf( "<signed> " );
                        if( HASCONST(t) ) printf( "<HASCONST> " );
                        if( HASVOLATILE(t) ) printf( "<HASVOLATILE> " );
                        printf( tnames[bt = TOPTYPE(t)] );
                        if( t->typ_size != bt ) printf( "(0%o)", t->typ_size );
                        return;
                }
        }
}
# endif /* BUG4 */
# endif /* PASS_TWO */
