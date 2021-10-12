/* @(#)54	1.10  src/bos/usr/bin/trcrpt/td.h, cmdtrace, bos411, 9428A410j 6/15/90 23:52:53 */

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: header file for a template tree
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * template/descriptors.
 *   symbol table structure for each piece of a template. Do not confuse with
 *   the symbol table created by lex, in sym.c
 *                                                                    
 */

/*
 * The x_fill members are not really necessary, but make it
 * easier to see the attempt to make pointers word aligned.
 */

struct headd {
	union td *u_next;
	short u_type;
	short u_fill;
	union td *h_next2;
};

struct flagsd {
	union td *u_next;
	short u_type;
	short u_fill;
	struct stringd *fl_field;
	struct stringd *fl_true;
	struct stringd *fl_false;
};
#define fl_mask  fl_false

struct macdefd {
	union td *u_next;
	short u_type;
	short u_fill;
	char m_name;			/* numeric Register number */
	char m_code;			/* 'i', 'r', or '$' */
	struct exprd *m_expr;	/* = X4 */
};

struct functiond {
	union td *u_next;
	short u_type;
	short fn_number;			/* function number */
	union td *fn_arglist;
};

struct exprd {
	union td *u_next;
	short u_type;
	char e_op;				/* = + -  * / */
	char e_fill;
	union td *e_left;		/* left */
	union td *e_right;		/* optional right */
};

struct formatd {
	union td *u_next;
	short u_type;
	char  f_fmtcode;		/* X,D,... */
	char  f_fld1;			/* X4 */
	char  f_fld2;			/* B32.8 */
	char  f_xfmtcode;		/* $rrr%X4 */
	short f_val;
	union td *f_desc;		/* LOOP X4 {   } */
};

struct switchd {
	union td *u_next;
	short u_type;
	short  s_base;
	struct exprd *s_expr;
	struct cased *s_case;
};

struct cased {
	union td *u_next;
	short u_type;
	short u_fill;
	struct stringd *c_value_label;	/* label */
	union td *c_desc;				/* optional descriptor */
};

struct stringd {
	union td *u_next;
	short u_type;
	short s_flags;					/* optimize ' ' delimiters */
	char *s_string;					/* contents */
	int   s_value;					/* numeric equivalent in SWITCH */
};
#define SFLG_NUMERIC 0x0001
#define SFLG_QUOTED  0x0002
#define SFLG_LDELIM  0x0004
#define SFLG_RDELIM  0x0008

/*
 * The td union comprises all the type of descriptors in the
 * parse tree. The beginning of each is the same, with a
 * next element pointer and a type value.
 * The rest is unique to the particular descriptor.
 */
union td {
	struct formatd td_formatd;
	struct switchd td_switchd;
	struct cased   td_cased;
	struct stringd td_stringd;
	struct macdefd td_macdefd;
	struct exprd   td_exprd;
	struct headd   td_headd;
	struct flagsd  td_flagsd;
};
#define td_next  td_headd.u_next
#define td_type  td_headd.u_type
#define td_next2 td_headd.h_next2

char *pr_format();
char *pr_tok();
union td *gettd();

/*
 * declarations between parse.y and parseutil.c
 */
union  td        *ydescriptors();
union  td        *ydescriptor();
struct formatd   *yloop();
struct formatd   *yformat();
struct switchd   *yswitch();
struct switchd   *ybitflags();
struct flagsd    *yflaglist();
struct flagsd    *yflagentry();
struct cased     *ycaselist();
struct cased     *ycase();
struct stringd   *ymatchvalue();
union  td        *ynewdesc();
union  td        *yarg();
struct exprd     *yexpr();
struct macdefd   *ymacdef();
union  td        *yoptlvl();
struct functiond *yfunction();
union td         *yarglist();
union td         *yqdescrp();
struct stringd   *ystringlist();

extern union td   *Tdp0;		/* head of parse tree */

