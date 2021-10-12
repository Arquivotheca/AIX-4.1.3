/* @(#)53	1.6  src/bos/usr/ccs/lib/libdbx/languages.h, libdbx, bos411, 9428A410j 1/12/94 17:00:08 */
#ifndef _h_languages
#define _h_languages
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) language_name
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

#include <dbxstclass.h>

typedef int (*LanguageOperation)();
typedef struct Language *Language;

#include "tree.h"

struct SUFLIST {
    String suffix;
    struct SUFLIST *next;
};
struct Language {
    String name;
    struct SUFLIST *suflist;
    LanguageOperation op[20];
    Language next;
};


typedef enum {
    L_PRINTDECL, L_PRINTVAL, L_TYPEMATCH, L_BUILDAREF, L_EVALAREF,
    L_MODINIT, L_HASMODULES, L_PASSADDR, L_FOLDNAMES,
    L_ENDOP
} LanguageOp;


#define language_name(lang)  ((lang == nil) ? "(nil)" : lang->name)

extern Language asmLang;
extern Language cLang;
extern Language cobLang;
extern Language fLang;
extern Language pascalLang;

extern Language primlang;
extern Language tb_front;
extern Language tb_back;

extern language_init(/*  */);
extern Language findlanguage(/* suffix */);
extern Language language_define(/* name, suffix */);
extern language_setop(/* lang, op, operation */);
extern LanguageOperation language_op(/* lang, op */);

/*  prototypes for asm.c  */
void asm_init ();
Boolean asm_typematch (Symbol, Symbol);
void asm_printdecl (Symbol);
void asm_printval (Symbol);
Node asm_buildaref (Node, Node);
void asm_evalaref (Symbol, Address, long);
void asm_modinit ();
boolean asm_hasmodules ();
boolean asm_passaddr (Symbol, Symbol);
cases asm_foldnames ();

/*  prototypes for c.c  */
void c_init ();
Boolean c_typematch (Symbol, Symbol);
void c_printdecl (Symbol);
void c_printdef (Symbol, int);
void C_printtype (Symbol, Symbol, int);
void c_listparams (Symbol);
void ansic_listparams(Symbol);
void c_printval(Symbol, int);
String c_classname(Symbol);
void c_evalaref(Symbol, Address, long);
void c_modinit ();
boolean c_hasmodules ();
boolean c_passaddr (Symbol, Symbol);
cases c_foldnames ();
void printunion (Symbol);

/*  prototypes for cobol.c  */
void cobol_init ();
Boolean cobol_typematch (Symbol, Symbol);
void cobol_printdecl (Symbol);
void cobol_printval (Symbol);
String cobol_classname (Symbol);
Node cobol_buildaref (Node, Node);
void cobol_evalaref (Symbol, Address, long);
void cobol_modinit ();
boolean cobol_hasmodules ();
boolean cobol_passaddr (Symbol, Symbol);
cases cobol_foldnames ();
void fixCobolIntArg (Node);
void fixCobolFloatArg (Node);
void cobolassign (Node, Node);
Boolean islang_cobol (Language);

/*  prototypes for fortran.c  */
void fortran_init ();
Boolean fortran_typematch (Symbol, Symbol);
void fortran_printdecl (Symbol);
void fortran_listparams (Symbol);
void fortran_printval (Symbol);
String fortran_classname (Symbol);
void fortran_evalaref (Symbol, Address, long);
void fortran_modinit ();
boolean fortran_hasmodules ();
boolean fortran_passaddr (Symbol, Symbol);
cases fortran_foldnames ();

/*  prototypes for pascal.c  */
void pascal_init();
boolean pascal_typematch (Symbol, Symbol);
void pascal_printdecl (Symbol);
void pascal_printval (Symbol);
void pascal_evalaref (Symbol, Address, long);
void pascal_modinit ();
boolean pascal_hasmodules ();
boolean pascal_passaddr (Symbol, Symbol);
cases pascal_foldnames ();
void printSet (Symbol); 
void printPstring(Symbol, int);
void printVrecord (Symbol);
Address pop_pas_ptr ();

#endif /* _h_languages */
