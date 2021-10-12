/* @(#)03       1.6.1.6  src/bos/usr/ccs/bin/indent/ind_globs.h, cmdprog, bos411, 9428A410j 4/14/94 12:42:52 */
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/*-

                          Copyright (C) 1976
                                by the
                          Board of Trustees
                                of the
                        University of Illinois

                         All rights reserved
*/

/*

FILE NAME:
        indent_globs.h

PURPOSE:
        This include file contains the declarations for all global variables
        used in indent.

GLOBALS:
        The names of all of the variables will not be repeated here.  The
        declarations start on the next page.

FUNCTIONS:
        None
*/

#define _ILS_MACROS    /* 139729 - use macros for better performance */
#include <stdio.h>
#include <nl_types.h>
#include <sys/localedef.h>
#include <ctype.h>

#define PROCOM 1 /* Profile comments allowed */

#define BACKSLASH '\\'
#define bufsize 1800 /* size of internal buffers */
#define inp_bufs 600 /* size of input buffer */
#define sc_size 5000 /* size of save_com buffer */
#define label_offset 2 /* number of levels a label is placed to left of code */

#define tabsize 8  /* the size of a tab */
#define tabmask 0177770    /* mask used when figuring length of lines with tabs */


#define false 0
#define true  1

#define MSGSTR(n,s) catgets(catd,MS_INDENT,n,s)

nl_catd catd;                                   /* message catalog descriptor */
int     indent_mb_cur_max = 0;                  /* local copy of MB_CUR_MAX */

FILE   *input;                                  /* the fid for the input file */
FILE   *output;                                 /* the output file */

char    labbuf[bufsize];                        /* buffer for label */
char   *s_lab;                                  /* start ... */
char   *e_lab;                                  /* .. and end of stored label */

char    codebuf[bufsize];                       /* buffer for code section */
char   *s_code;                                 /* start ... */
char   *e_code;                                 /* .. and end of stored code */

char    combuf[bufsize];                        /* buffer for comments */
char   *s_com;                                  /* start ... */
char   *e_com;                                  /* ... and end of stored comments */

char    in_buffer[inp_bufs];                    /* input buffer */
char   *buf_ptr;                                /* ptr to next character to be taken from in_buffer */
char   *buf_end;                                /* ptr to first after last char in in_buffer */
char    win_buffer[16];                         /* storage space for MB chars truncated
                                                 * by in_buffers boundary */

char    save_com[sc_size];      /* input text is saved here when looking for the brace
                      after an if, while, etc */
char   *sc_end;    /* pointer into save_com buffer */

char   *bp_save;   /* saved value of buf_ptr when taking input from
                      save_com */
char   *be_save;   /* similarly saved value of buf_end */

char    token[bufsize]; /* the last token scanned */



int     blanklines_after_declarations;
int     blanklines_before_blockcomments;
int     blanklines_after_procs;
int     swallow_optional_blanklines;
int     n_real_blanklines;
int     prefix_blankline_requested;
int     postfix_blankline_requested;
int     break_comma;    /* when true and not in parens, break after a comma */
int     btype_2;   /* when true, brace should be on same line as if,
                      while, etc */
float   case_ind;  /* indentation level to be used for a "case n:" */
int     code_lines;/* count of lines with code */
int     had_eof;   /* set to true when input is exhausted */
int     line_no;   /* the current line number. */
int     max_col;   /* the maximum allowable line length */
int     pointer_as_binop;       /* when true, "->" is treated as a binary
                                   operator (giving, e.g., "p -> f") */
int     verbose;   /* when true, non-essential error messages are printed
                   */
int     cuddle_else;            /* true if else should cuddle up to '}' */
int     star_comment_cont;      /* true iff comment continuation lines should
                                   have stars at the beginning of each line.
                                   */
int     comment_delimiter_on_blankline;
int     troff;                  /* true iff were generating troff input */
int     procnames_start_line;   /* if true, the names of procedures being
                                   defined get placed in column 1 (ie. a
                                   newline is placed between the type of the
                                   procedure and its name) */
int     proc_calls_space;       /* If true, procedure calls look like:
                                   foo(bar) rather than foo (bar) */
int     format_col1_comments;   /* If comments which start in column 1 are to
                                   be magically reformatted (just like comments that
                                   begin in later columns) */
int     inhibit_formatting;     /* true if INDENT OFF is in effect */
int     suppress_blanklines;    /* set iff following blanklines should be
                                   suppressed */
int     continuation_indent;    /* set to the indentation between the edge of
                                   code and continuation lines */
int     lineup_to_parens;       /* if true, continued code within parens will
                                   be lined up to the open paren */
int     block_comment_max_col;
int     single_line_bc;         /* true if single line block comment wanted */
int     new_func_proto;
int     flip_assign_oper;       /* true if the user want to flip eg. a=+b to */
                                /* a+=b i.e. 'old' C to 'new' C */
int     exit_code;              /* exit status */


struct parser_state {
    int         last_token;
    int         p_stack[50];    /* this is the parsers stack */
    int         il[50];         /* this stack stores indentation levels */
    float       cstk[50];       /* used to store case stmt indentation
                                 * levels */
    int         box_com;        /* set to true when we are in a "boxed"
                                 * comment. In that case, the first
                                 * non-blank char should be lined up with
                                 * the / in '/' '*'  */
    int         comment_delta,
                n_comment_delta;
    int         cast_mask;      /* indicates which close parens close off
                                 * casts */
    int         sizeof_mask;    /* indicates which close parens close off
                                   sizeof''s */
    int         block_init;     /* true iff inside a block initialization */
    int         last_nl;        /* this is true if the last thing scanned
                                 * was a newline */
    int         in_or_st;       /* Will be true iff there has been a
                                 * declarator (e.g. int or char) and no
                                 * left paren since the last semicolon.
                                 * When true, a '{' is starting a
                                 * structure definition or an
                                 * initialization list */
    int         bl_line;        /* set to 1 by dump_line if the line is
                                 * blank */
    int         col_1;          /* set to true if the last token started
                                 * in column 1 */
    int         com_col;        /* this is the column in which the current
                                 * coment should start */
    int         com_ind;        /* the column in which comments to the
                                 * right of code should start */
    int         com_lines;      /* the number of lines with comments, set
                                 * by dump_line */
    int         dec_nest;       /* current nesting level for structure or
                                 * init */
    int         decl_com_ind;   /* the column in which comments after
                                 * declarations should be put */
    int         decl_on_line;   /* set to true if this line of code has
                                 * part of a declaration on it */
    int         i_l_follow;     /* the level to which ind_level should be
                                 * set after the current line is printed */
    int         in_decl;        /* set to true when we are in a
                                 * declaration stmt.  The processing of
                                 * braces is then slightly different */
    int         in_stmt;        /* set to 1 while in a stmt */
    int         ind_level;      /* the current indentation level */
    int         ind_size;       /* the size of one indentation level */
    int         ind_stmt;       /* set to 1 if next line should have an
                                 * extra indentation level because we are
                                 * in the middle of a stmt */
    int         last_u_d;       /* set to true after scanning a token
                                 * which forces a following operator to be
                                 * unary */
    int         leave_comma;    /* if true, never break declarations after
                                 * commas */
    int         ljust_decl;     /* true if declarations should be left
                                 * justified */
    int         out_coms;       /* the number of comments processed, set
                                 * by pr_comment */
    int         out_lines;      /* the number of lines written, set by
                                 * dump_line */
    int         p_l_follow;     /* used to remember how to indent
                                 * following statement */
    int         paren_level;    /* parenthesization level. used to indent
                                 * within stmts */
    short       paren_indents[20];      /* column positions of each paren */
    int         pcase;          /* set to 1 if the current line label is a
                                 * case.  It is printed differently from
                                 * a regular label */
    int         search_brace;   /* set to true by parse when it is
                                 * necessary to buffer up all info up to
                                 * the start of a stmt after an if, while,
                                 * etc */
    int         unindent_displace;      /* comments not to the right of
                                         * code will be placed this many
                                         * indentation levels to the left
                                         * of code */
    int         use_ff;         /* set to one if the current line should
                                 * be terminated with a form feed */
    int         want_blank;     /* set to true when the following token
                                 * should be prefixed by a blank. (Said
                                 * prefixing is ignored in some cases.) */
    int         else_if;        /* True iff else if pairs should be
                                 * handled specially */
    int         decl_indent;    /* column to indent declared identifiers
                                 * to */
    int         its_a_keyword;
    int         sizeof_keyword;
    int         dumped_decl_indent;
    float       case_indent;    /* The distance to indent case labels from
                                 * the switch statement */
    int         in_parameter_declaration;
    int         indent_parameters;
    int         tos;            /* pointer to top of stack */
    char        procname[100];  /* The name of the current procedure */
    int         just_saw_decl;
    int         enum_decl;      /* true if in an enum declaration */
}           ps;

int ifdef_level;
/*A15282 ANSI states that at least 8 levels of nesting must be supported */
struct parser_state state_stack[8];
struct parser_state match_state[8];

/*
 * Function prototypes
 */
char    *nextmbc();
char    *prevmbc();
void     putmbc();
int      cpymbc();
char    *rtrimmbc();
int      widthmbc();
