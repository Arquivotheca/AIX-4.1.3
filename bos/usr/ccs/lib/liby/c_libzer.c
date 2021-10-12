static char sccsid[] = "@(#)70 1.1 src/bos/usr/ccs/lib/liby/c_libzer.c, liby, bos411, 9428A410j 2/19/92 16:42:28";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities 
 *
 * FUNCTIONS: yyerror 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with the
 * aggregated modules for this product)                  SOURCE MATERIALS (C)
 * COPYRIGHT International Business Machines Corp. 1985, 1989 All Rights
 * Reserved 
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp. 
 */
/*
 * C++ source:
 * # include <iostream.h> 
 *
 * void yyerror( char *s ) { clog << s << endl; } 
 */
struct ios {
	int             nuser__3ios;
	union ios_user_union *x_user__3ios;
	struct streambuf *bp__3ios;
	int             state__3ios;
	int             ispecial__3ios;
	int             ospecial__3ios;
	int             isfx_special__3ios;
	int             osfx_special__3ios;
	int             delbuf__3ios;
	struct ostream *x_tie__3ios;
	long            x_flags__3ios;
	short           x_precision__3ios;
	char            x_fill__3ios;
	short           x_width__3ios;
	int             assign_private__3ios;
	struct __mptr  *__vptr__3ios;
};

struct ostream {
	struct __mptr  *__vptr__7ostream;
	struct ios     *Pios;
	struct ios      Oios;
};

extern struct ostream clog;
extern struct ostream *__ls__7ostreamFPCc();
extern struct ostream *endl__FR7ostream();

char yyerror__FPc(char *s)
{
    /* this is much simplified */
    endl__FR7ostream (__ls__7ostreamFPCc(&clog, s));
}
