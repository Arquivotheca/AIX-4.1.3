/* @(#)89       1.5  src/bos/usr/lib/pios/pioattred.h, cmdpios, bos411, 9428A410j 7/23/93 16:23:19 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stat.h>
#include "piobe_msg.h"

#define WIDTH               80
#define MSG_INDENT          15
/* Due to the way line_print() is written, ATTR_INDENT can't be less than one */
#define ATTR_INDENT         1
#define IND_INCR            5

#define DFLT_ATTR_CAT_NUM   1
#define OP_CATALOG          "piobe.cat"
#define MSG_CATALOG         "pioattr1.cat"
#define NO_CAT              "Cannot open message catalog"
#define NO_MSG              "Not a valid message number"
#define NO_ATTR             "Attribute not found"

#define PIOCNVT             "/usr/sbin/piocnvt"
#define CHVIRPRT            "chvirprt"
#define DEFVARDIR           "/var/spool/lpd/pio/@local"
#define DEFBASEDIR          "/usr/lib/lpd/pio"

#define NO_FIELD            1
#define STR_FIELD           2
#define CHR_FIELD           3
#define NUM_FIELD           4

#define BIG                 1
#define SMALL               0
#define UNDEFINED           -1

/* Define macros for non-modifiable attributes */
#define ATTRNMLEN	2
#define ATTR_DEF_STATE	"zD"
#define ATTR_CUR_STATE	"zS"
#define PROHIBIT_ATTRLIST_DEFN	char *prohibit_attrs[] = \
			{ \
			   ATTR_DEF_STATE, \
			   ATTR_CUR_STATE, \
			   NULL \
			}

/*
**  The original input and unput macros read from a file, asually stdin,
**  and, in the case of input, recognize EOL and EOF and act accordingly.
**  These replacement macros assume that a pointer to a string in memory
**  is given as 'inptr'.  This pointer is set to point at the beginning
**  of the string to be parsed when yyparse() is called.  It will point
**  at the null string terminator when it gets to the end of the string
**  and so behave normally.  Neither EOL nor EOF will be seen.
**
**  Even though the unput macro is fairly simple, it is still probably
**  more complicated than it needs to be.  The simple approach would be
**  to assume that characters to be stuffed back into the input data
**  stream came from the input data stream in the order they're being
**  stuffed back in.  If that assumption could hold, them all the unput
**  macro would have to do would be to decrement 'inptr' each time it
**  was called.  I was unwilling to make that assumption.  In the dark
**  black world of lex and yacc, it seemes possible that, in order to
**  induce specific behavior, oddball characters could be stuffed back
**  into the data stream.  Therefore, I made unbfr[100], whose first
**  byte is a null, and 'unptr' which initially points at that null.
**  The unput macro first increments 'unptr' then writes its byte into
**  unbfr.  That byte is then read by the input macro the next time around.
*/

#   undef   input
#   undef   unput
#   define  input()         (yytchar = *unptr ? *unptr-- : *inptr++)
#   define  unput(c)        (*(++unptr) = c)


/*  =====================
**  Function declarations
*/

int             format(),
                unformat(),
                fgetln();

char            *find_attr(),
                *get_desc();

void            add_attr(),
                read_edit(),
                line_print(),
                check_opts(),
                error_exit();

struct line     *get_file();

char            *find_desc(char *atbt);
void            show_op(int msg_num, char *oper, int type, char *str, int chr);

/*  ============================
**  Global variable declarations
*/

char            tok_name[200],
                attributes[5000],
                *this,
                guts[1000],
                operator[1000],
                unbfr[1000],
                *inptr,
                *unptr,
                *cat_name_mD,
                attr_cat[50];

int             disp, indent, if_while, synt_err, disperronly, parseerr;
int		colonf_input = FALSE;

struct line     *file;

FILE            *out;
