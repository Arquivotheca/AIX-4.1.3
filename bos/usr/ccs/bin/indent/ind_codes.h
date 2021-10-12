/* @(#)02       1.3  src/bos/usr/ccs/bin/indent/ind_codes.h, cmdprog, bos411, 9428A410j 6/21/91 15:15:45 */
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26; 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/*

                          Copyright (C) 1976
                                by the
                          Board of Trustees
                                of the
                        University of Illinois

                         All rights reserved
*/

/* 
FILE NAME:
        indent_codes.h

PURPOSE:
        This include file contains defines for codes used within indent.  They
        are here so that codes passed between and within routines can be
        referenced symbolically.

GLOBALS:
        No global variables, just a bunch of defines

FUNCTIONS:
        None
*/

#define newline         1
#define lparen          2
#define rparen          3
#define unary_op        4
#define binary_op       5
#define postop          6
#define question        7
#define casestmt        8
#define colon           9
#define semicolon       10
#define lbrace          11
#define rbrace          12
#define ident           13
#define comma           14
#define comment         15
#define swstmt          16
#define preesc          17
#define form_feed       18
#define decl            19
#define sp_paren        20
#define sp_nparen       21
#define ifstmt          22
#define whilestmt       23
#define forstmt         24
#define stmt            25
#define stmtl           26
#define elselit         27
#define dolit           28
#define dohead          29
#define ifhead          30
#define elsehead        31
#define period          32
#define lstring         33
