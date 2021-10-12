/* @(#)33       1.3  src/bos/kernel/sys/jctype1.h, libcnls, bos411, 9428A410j 6/16/90 00:30:12 */
/*
 *  COMPONENT_NAME: (INCSYS) definition of _jctype1_ Kanji table
 *
 *  FUNCTIONS: _jctype1_
 *
 *  ORIGINS: 10
 *
 *  (C) COPYRIGHT International Business Machines Corp.  1986, 1989
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#ifndef _H_JCTYPE1
#define _H_JCTYPE1
        /* Second-level tables:  First index is from _jctype0_, second
         * from second byte of character. */

/* Shortened definitions (since we're not worried about ident clashes),
 * to make the tables easier to read. */
#define k       _Jk             /* katakana */
#define H       _JH             /* hiragana */
#define K       _JK             /* kanji */
#define D       _JD             /* digit */
#define A       _JA             /* upper alpha (non-hex) */
#define a       _Ja             /* lower alpha (non-hex) */
#define X       _JB             /* upper hex alpha */
#define x       _Jb             /* lower hex alpha */
#define P       _JP             /* punctuation */
#define G       _JG             /* any other graphic character */
#define r       _JR             /* any valid unused position */

unsigned char _jctype1_[13][256] = {
        /* Table 0: invalid upper byte */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 4x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 5x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 6x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 7x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 8x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 9x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* ax */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* bx */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* cx */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* dx */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* ex */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },   /* fx */
        /* Table 1: single bytes (upper byte == 0) */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        G,P,P,P, P,P,P,P, P,P,P,P, P,P,P,P,     /* 2x */
        D,D,D,D, D,D,D,D, D,D,P,P, P,P,P,P,     /* 3x */
        P,X,X,X, X,X,X,A, A,A,A,A, A,A,A,A,     /* 4x */
        A,A,A,A, A,A,A,A, A,A,A,P, P,P,P,P,     /* 5x */
        P,x,x,x, x,x,x,a, a,a,a,a, a,a,a,a,     /* 6x */
        a,a,a,a, a,a,a,a, a,a,a,P, P,P,P,0,     /* 7x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 8x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 9x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* ax */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* bx */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* cx */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* dx */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* ex */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },   /* fx */
        /* Table 2: 0x81 upper byte */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        G,P,P,P, P,P,P,P, P,P,P,P, P,P,P,P,     /* 4x */
        P,P,G,G, G,G,G,G, G,G,P,P, P,P,P,P,     /* 5x */
        P,P,P,P, P,P,P,P, P,P,P,P, P,P,P,P,     /* 6x */
        P,P,P,P, P,P,P,P, P,P,P,P, P,P,P,0,     /* 7x */
        P,P,P,P, P,P,P,P, P,P,P,P, P,P,P,P,     /* 8x */
        P,P,P,P, P,P,P,P, P,G,G,G, G,G,G,G,     /* 9x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,0,0,0,     /* ax */
        r,r,r,r, r,r,r,r, G,G,G,G, G,G,G,G,     /* bx */
        r,r,r,r, r,r,r,r, G,G,r,G, G,G,G,r,     /* cx */
        r,r,r,r, r,r,r,r, r,r,G,G, G,G,G,G,     /* dx */
        G,G,G,G, G,G,r,G, G,r,r,r, r,r,r,r,     /* ex */
        G,G,G,G, G,P,P,P, r,r,r,r, G,0,0,0 },   /* fx */
        /* Table 3: 0x82 upper byte */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,D,     /* 4x */
        D,D,D,D, D,D,D,D, D,r,r,r, r,r,r,r,     /* 5x */
        X,X,X,X, X,X,A,A, A,A,A,A, A,A,A,A,     /* 6x */
        A,A,A,A, A,A,A,A, A,A,0,r, r,r,r,0,     /* 7x */
        r,x,x,x, x,x,x,a, a,a,a,a, a,a,a,a,     /* 8x */
        a,a,a,a, a,a,a,a, a,a,a,r, r,r,r,H,     /* 9x */
        H,H,H,H, H,H,H,H, H,H,H,H, H,H,H,H,     /* ax */
        H,H,H,H, H,H,H,H, H,H,H,H, H,H,H,H,     /* bx */
        H,H,H,H, H,H,H,H, H,H,H,H, H,H,H,H,     /* cx */
        H,H,H,H, H,H,H,H, H,H,H,H, H,H,H,H,     /* dx */
        H,H,H,H, H,H,H,H, H,H,H,H, H,H,H,H,     /* ex */
        H,H,r,r, r,r,r,r, r,r,r,r, r,0,0,0 },   /* fx */
        /* Table 4: 0x83 upper byte */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* 4x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* 5x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* 6x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,0,     /* 7x */
        k,k,k,k, k,k,k,k, k,k,k,k, k,k,k,k,     /* 8x */
        k,k,k,k, k,k,k,r, r,r,r,r, r,r,r,G,     /* 9x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* ax */
        G,G,G,G, G,G,G,r, r,r,r,r, r,r,r,G,     /* bx */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* cx */
        G,G,G,G, G,G,G,r, r,r,r,r, r,r,r,r,     /* dx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ex */
        r,r,r,r, r,r,r,r, r,r,r,r, r,0,0,0 },   /* fx */
        /* Table 5: 0x84 upper byte */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 4x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 5x */
        G,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 6x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,0,     /* 7x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 8x */
        G,G,r,r, r,r,r,r, r,r,r,r, r,r,r,G,     /* 9x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* ax */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,0,     /* bx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* cx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* dx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ex */
        r,r,r,r, r,r,r,r, r,r,r,r, r,0,0,0 },   /* fx */
        /* Table 6: 0x88 upper byte - start kanji level 1 */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 4x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 5x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 6x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,0,     /* 7x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 8x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,K,     /* 9x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ax */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* bx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* cx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* dx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ex */
        K,K,K,K, K,K,K,K, K,K,K,K, K,0,0,0 },   /* fx */
        /* Table 7: kanji ward, no end conditions */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 4x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 5x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 6x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,0,     /* 7x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 8x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 9x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ax */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* bx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* cx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* dx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ex */
        K,K,K,K, K,K,K,K, K,K,K,K, K,0,0,0 },   /* fx */
        /* Table 8: 0x98 upper byte - kanji level 1 / 2 interstice */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 4x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 5x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 6x */
        K,K,K,r, r,r,r,r, r,r,r,r, r,r,r,0,     /* 7x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 8x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,K,     /* 9x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ax */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* bx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* cx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* dx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ex */
        K,K,K,K, K,K,K,K, K,K,K,K, K,0,0,0 },   /* fx */
        /* Table 9: 0xea upper byte - end kanji level 2 */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 4x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 5x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 6x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,0,     /* 7x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 8x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 9x */
        K,K,K,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ax */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* bx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* cx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* dx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ex */
        r,r,r,r, r,r,r,r, r,r,r,r, r,0,0,0 },   /* fx */
        /* Table 10: 0xfa upper byte - IBM non-kanji, start kanji */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 4x */
        G,G,G,G, G,G,G,G, G,G,G,G, K,K,K,K,     /* 5x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 6x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,0,     /* 7x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 8x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* 9x */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ax */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* bx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* cx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* dx */
        K,K,K,K, K,K,K,K, K,K,K,K, K,K,K,K,     /* ex */
        K,K,K,K, K,K,K,K, K,K,K,K, K,0,0,0 },   /* fx */
        /* Table 11: 0xfc upper byte - end IBM-defined kanji */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        K,K,K,K, K,K,K,K, K,K,K,K, r,r,r,r,     /* 4x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 5x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 6x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,0,     /* 7x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 8x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* 9x */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ax */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* bx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* cx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* dx */
        r,r,r,r, r,r,r,r, r,r,r,r, r,r,r,r,     /* ex */
        r,r,r,r, r,r,r,r, r,r,r,r, r,0,0,0 },   /* fx */
        /* Table 12: various upper bytes - unused but valid ward */
{       0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 0x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 1x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 2x */
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,     /* 3x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 4x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 5x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 6x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,0,     /* 7x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 8x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* 9x */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* ax */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* bx */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* cx */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* dx */
        G,G,G,G, G,G,G,G, G,G,G,G, G,G,G,G,     /* ex */
        G,G,G,G, G,G,G,G, G,G,G,G, G,0,0,0 },   /* fx */
};

/* undefine all shortened definitions */

#undef  k
#undef  H
#undef  K
#undef  D
#undef  A
#undef  a
#undef  X
#undef  x
#undef  P
#undef  G
#undef  r

#endif
