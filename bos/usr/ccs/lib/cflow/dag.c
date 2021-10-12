static char sccsid[] = "@(#)38  1.9.1.7  src/bos/usr/ccs/lib/cflow/dag.c, cmdprog, bos411, 9428A410j 4/14/94 12:32:59";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, addlink, copy, dfs, getnode, getnum, getst, newlink,
              newnode, reader
 *
 * ORIGINS: 3 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*      MSG is used to define whether or not you want the
            NLS messages.  Needs to be set when you want the
            messages.
*/

#define _ILS_MACROS
#include "stdio.h"
#include <locale.h>
#include <nl_types.h>
#include <sys/localedef.h>
#include <ctype.h>

#include        "cflow_msg.h"

#define         MSGSTR(Num, Str) catgets(catd, MS_DAG, Num, Str)

#define NLINK   8
#define NODEBLK 200     /* node allocation size */
#define LINKBLK 100     /* link allocation size */

typedef struct  node {
        struct  node *n_next;
        struct  node *n_left;
        struct  node *n_right;
        char    *n_name;
        char    *n_data;
        int     n_rcnt;
        int     n_visit;
        int     n_lcnt;
        struct  link *n_link;
} node;

node    *first = NULL;
node    *last = NULL;
node    *root = NULL;

typedef struct  link {
        struct  link *l_next;
        struct  node *l_node[NLINK];
} link;

#define isnamec(c)      (c != '\0' && !isspace(c))
#define NUL     '\0'
#define BIGINT  32767

node    *looknode(char *);
node    *getnode(char *);
char    *copy();
int     lines, nodes, links, chars;
int     lineno = 0;
int     lvmax = BIGINT;
char    stdbuf[BUFSIZ];

nl_catd catd;

main(argc, argv)
int argc;
char *argv[];
{
        extern int optind;
        extern char *optarg;
        register node *np;
        node *getnode();
        int c;
        void reader(), dfs();

        setlocale(LC_ALL, "");

        catd = catopen(MF_CFLOW, NL_CAT_LOCALE);

        setbuf(stdout, stdbuf);
        while ((c = getopt(argc, argv, "d:")) != EOF)
                if (c == 'd')
                        {
                        if ((lvmax = getnum(optarg, 10)) == 0)
                                {
                                lvmax = BIGINT;
                                goto argerr;
                                }
                        }
                else
                argerr:
                        (void)fprintf(stderr, MSGSTR(DBOPT,
                                "dag: bad option %c ignored\n"), c);
        reader(stdin);
        argc -= optind + 1;
        if (argc <= 1)
                for (np = first; np != NULL; np = np->n_next)
                        {
                        if (np->n_rcnt == 0)
                                dfs(np, 0);
                        }
        else
                while (--argc > 0)
                        dfs(getnode(*++argv), 0);
        return(0);
}

getnum(p, base)
        register int base;
        register char *p;
        {
        register int n;

        n = 0;
        while (isdigit(*p))
                n = n * base + (*p++ - '0');
        return(n);
        }

void reader(fp)
register FILE *fp;
{
        register char *p1, *p2;
        int     c;
        node    *np, *getnode();
        char    line[BUFSIZ], *copy();

        while (getst(line, fp)) {
                ++lines;
                p1 = line;
                while (isspace(*p1))
                        ++p1;
                if (*p1 == NUL)
                        continue;
                p2 = p1;
		for(;;) {
		    if (isnamec(*p2))
			/* normal name character */
                        ++p2;
		    else if (*p2 == ' ' && isalpha(p2[1]))
			/* C++ name with embedded blanks */
			++p2;
		    else
			break;
		}
                do {
                        c = *p2;
                        *p2++ = NUL;
                } while (isspace(c));
                switch (c) {

/* PTM 45503 -> funcational change to add proto-control within the ".g"
                and make sure proto-types only enter the data structure
                if there is a function previously defined to avoid adding
                a function call in the output that does not belong. This
                happens when a _name () is calling a proto-typed function
                and the -i_ flag is on.
        JRW 13/07/90
*/
                case '#':
                        /* only add proto-type if is for an already defined
                           function */
                        if((np = looknode(p1)) != NULL) {
                                while (isspace(*p2))
                                        ++p2;
                                if(np->n_data == NULL)
                                        np->n_data = copy(p2);
                                }
                        continue;
                case '=':
                        np = getnode(p1);
                        while (isspace(*p2))
                                ++p2;
                        /* Redefinitions expected with prototypes. */
                        if (np->n_data != NULL && strcmp(np->n_data, p2))
                                (void)cfree(np->n_data);
                        np->n_data = copy(p2);
                        continue;
                case ':':
                        np = getnode(p1);
                        while (*(p1 = p2) != NUL) {
                                while (isspace(*p1))
                                        ++p1;
                                p2 = p1;
                                while (isnamec(*p2))
                                        ++p2;
                                (void)addlink(np, getnode(p1));
                                if (*p2 != NUL)
                                        *p2++ = NUL;
                        }
                        continue;
                }
        }
}

getst(s, fp)
char *s;
FILE *fp;
{
        register i, c;

        i = 0;
        while ((c = getc(fp)) != EOF) {
                if (c == '\n') {
                        *s = NUL;
                        return 1;
                }
                if (i++ < BUFSIZ)
                        *s++ = c;
        }
        return 0;
}

char *
copy(s)
char    *s;
{
        char    *p, *strcpy(), *calloc();
        void    exit();

        p = calloc(sizeof(char), (unsigned)(strlen(s)+2));
        if (p == NULL) {
                (void)fprintf(stderr, MSGSTR(TMCHS, "too many characters\n"));
                exit(1);
        }
        (void)strcpy(p, s);
        chars += strlen(p);
        return p;
}

node *
looknode(name)
char    *name;
{
        register i;
        register node *np, **pp;

        pp = &root;
        while ((np = *pp) != NULL) {
                i = strcmp(name, np->n_name);
                if (i > 0)
                        pp = &np->n_right;
                else if (i < 0)
                        pp = &np->n_left;
                else
                        return np;      /* only successful conclusion */
        }
        return(NULL); /* search failed */
}


node *
getnode(name)
char    *name;
{
        register i;
        register node *np, **pp;
        node *newnode();
        char *copy();

        pp = &root;
        while ((np = *pp) != NULL) {
                i = strcmp(name, np->n_name);
                if (i > 0)
                        pp = &np->n_right;
                else if (i < 0)
                        pp = &np->n_left;
                else
                        return np;
        }
        *pp = newnode();
        np = *pp;
        ++nodes;
        np->n_name = copy(name);
        np->n_data = NULL;
        if (first == NULL)
                first = np;
        if (last != NULL)
                last->n_next = np;
        last = np;
        np->n_next = NULL;
        np->n_left = np->n_right = NULL;
        np->n_rcnt = np->n_lcnt = np->n_visit = 0;
        np->n_link = NULL;
        return np;
}

addlink(np, rp)
node    *np, *rp;
{
        register i, j;
        link    *lp, **pp;
        link *newlink();

        i = j = 0;
        pp = &np->n_link;
        while ((lp = *pp) != NULL && i < np->n_lcnt) {
                if (rp == lp->l_node[j])
                        return 0;
                if (++j >= NLINK) {
                        j = 0;
                        pp = &lp->l_next;
                }
                ++i;
        }
        if (lp == NULL) {
                *pp = newlink();
                lp = *pp;
                ++links;
                lp->l_next = NULL;
        }
        lp->l_node[j] = rp;
        if (np != rp)
        ++rp->n_rcnt;
        ++np->n_lcnt;
        return 1;
}

void dfs(np, lv)
register
node *np;
{
        register i, j;
        link *lp;

        if (np == NULL || lv > lvmax)
                return;
        i = 0;
        (void)printf("%d\t",++lineno);
        while (i++ < lv)
                (void)putchar('\t');
        (void)printf("%s: ", np->n_name);
        if (np->n_visit > 0) {
                (void)printf("%d\n", np->n_visit);
                return; 
        }
        if (np->n_data != NULL)
                (void)printf("%s\n", np->n_data);
        else
                (void)printf("<>\n");
/* 
Changed back for XPG4
                (void)printf("() int, <>\n");
                              ^^^^^^^^
PTM 45502: Added the text above since that is the only function definition
           possible if the function is used and not defined previously.
        JRW 13/07/90
*/
        np->n_visit = lineno;
        i = j = 0;
        lp = np->n_link;
        while (i < np->n_lcnt && lp != NULL) {
                dfs(lp->l_node[j], lv+1);
                if (++j >= NLINK) {
                        j = 0;
                        lp = lp->l_next;
                }
                ++i;
        }
}

node *
newnode()
{
        static int bunchsize = 0;
        static node *bunch;
        char *calloc();
        void exit();

        if (bunchsize == 0) {
                bunch = (node *)calloc(sizeof(node), NODEBLK);
                if (bunch == NULL) {
                        (void)fprintf(stderr, MSGSTR(TMNDS, "too many nodes"));
                        exit(1);
                }
                bunchsize = NODEBLK;
        }
        return (&bunch[--bunchsize]);
}

link *
newlink()
{
        static int bunchsize = 0;
        static link *bunch;
        char *calloc();
        void exit();

        if (bunchsize == 0) {
                bunch = (link *)calloc(sizeof(link), LINKBLK);
                if (bunch == NULL) {
                        (void)fprintf(stderr, MSGSTR(TMLNKS,
                                "too many links\n"));
                        exit(1);
                }
                bunchsize = LINKBLK;
        }
        return (&bunch[--bunchsize]);
}
