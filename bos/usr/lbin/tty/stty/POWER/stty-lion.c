static char sccsid[] = "@(#)87  1.4  src/bos/usr/lbin/tty/stty/POWER/stty-lion.c, cmdtty, bos411, 9428A410j 12/21/93 01:41:02";

/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (stty-lion.c)
 *
 * ORIGINS: 26, 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <string.h>
#include <sys/li.h>

#include        <locale.h>
#include        <nl_types.h>
#include        "stty-lion_msg.h"

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_STTY_LION,num,str)

void error(char *s);
int (*search(register char *cmd))();
int dogt1(char **argv);
int dogt2(char **argv);
int doinx(char **argv);
int dolvx(char **argv);
int doprx(char **argv);
int dosc1(char **argv);
int dosc2(char **argv);
int doshw(char **argv);
int dotbc(char **argv);

struct cmds {
    char *name;
    int (*func)(char **argv);
} cmds[] = {
    "goto1", dogt1,
    "goto2", dogt2,
    "in_xpar", doinx,
    "lv_xpar", dolvx,
    "priority", doprx,
    "screen1", dosc1,
    "screen2", dosc2,
    "show", doshw,
    "tbc", dotbc,
};
#define CMDS (sizeof(cmds) / sizeof(struct cmds))

char *progname;
struct vter_parms vter;
struct xpar_parms xpar;
int tbc;
int ch_vter, ch_xpar, ch_tbc;

main(int argc, char *argv[])
{
    int (*f)();

    setlocale(LC_ALL,"") ;
    catd = catopen(MF_STTY_LION, NL_CAT_LOCALE);

    if (progname = strrchr(argv[0], '/'))
	++progname;
    else
	progname = argv[0];
    ++argv;

    if (ioctl(0, LI_GETVT, &vter))
	error("LI_GETVT");
    if (ioctl(0, LI_GETXP, &xpar))
	error("LI_GETXP");
    if (ioctl(0, LI_GETTBC, &tbc))
	error("LI_GETTBC");

    while (*argv) {
	if (!(f = search(*argv))) {
	    fprintf(stderr, MSGSTR(UNKNOWN, "%s: %s unknown\n"), progname,
		    *argv);
	    ++argv;
	    continue;
	}
	argv += (*f)(argv);
    }
    
    if (ch_vter && ioctl(0, LI_SETVT, &vter))
	error("LI_SETVT");
    if (ch_xpar && ioctl(0, LI_SETXP, &xpar))
	error("LI_SETXP");
    if (ch_tbc && ioctl(0, LI_SETTBC, &tbc))
	error("LI_SETTBC");
}

void error(char *s)
{
    fprintf(stderr, "%s: ", progname);
    perror(s);
    exit(1);
}

/* 
 * Binary search of command.  Return true for a hit and sets f to the 
 * function to call.  hi starts at the number of commands minus 2 so 
 * lo can only be as big as the number of commands minus 1 which is 
 * the index to the last command.
 */
int (*search(register char *cmd))()
{
    register int lo = 0, hi = CMDS - 2;
    register int mid;
    register struct cmds *p;

    /* 
     * Binary search loop.  By the time we are done, lo points to the 
     * only possible match.
     */
    while (lo <= hi)
	if (strcmp(cmd, (p = cmds + (mid = (lo + hi) >> 1))->name) <= 0)
	    hi = mid - 1;
	else
	    lo = mid + 1;

    p = cmds + lo;
    return (strcmp(cmd, p->name)) ? 0 : p->func;
}

int dotbc(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(TBCUSAGE, "Usage: %s tbc level\n"));
	return 1;
    }
    ch_tbc = 1;
    tbc = atoi(*argv);
    return 2;
}

dogt1(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(GT1USAGE, "Usage: %s goto1 string\n"));
	return 1;
    }
    ch_vter = 1;
    rdstr(vter.goto1, *argv, 4);
    return 2;
}

dogt2(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(GT2USAGE, "Usage: %s goto2 string\n"));
	return 1;
    }
    ch_vter = 1;
    rdstr(vter.goto2, *argv, 4);
    return 2;
}

dosc1(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(SC1USAGE, "Usage: %s screen1 string\n"));
	return 1;
    }
    ch_vter = 1;
    rdstr(vter.screen1, *argv, 10);
    return 2;
}

dosc2(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(SC2USAGE, "Usage: %s screen2 string\n"));
	return 1;
    }
    ch_vter = 1;
    rdstr(vter.screen2, *argv, 10);
    return 2;
}

doinx(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(INXUSAGE, "Usage: %s in_xpar string\n"));
	return 1;
    }
    ch_xpar = 1;
    rdstr(xpar.in_xpar, *argv, 10);
    return 2;
}

dolvx(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(LVXUSAGE, "Usage: %s lv_xpar string\n"));
	return 1;
    }
    ch_xpar = 1;
    rdstr(xpar.lv_xpar, *argv, 10);
    return 2;
}

int doprx(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(PRXUSAGE, "Usage: %s priority level\n"));
	return 1;
    }
    ch_xpar = 1;
    xpar.priority = atoi(*argv);
    return 2;
}

rdstr(char *outstr, char *instr, int len)
{
    register i, c;

    for (i=0 ; i<len && (c = *instr++); i++) {
	if (c == '^') {
	    c = *instr++;
	    if (c == '?')
		c = 0x7f;
	    else if (c == '\0') {
		*outstr++ = '^';
		break;
	    } else
		c &= 0x1f;
	}
	*outstr++ = c;
    }
    while (i++ < len+1)
	*outstr++ = 0xff;
}

outstr(char *what, char *str)
{
    register c;

    printf("%s = '", what);
    while ((c = *str++) != 0xff) {
	if (c < ' ') {
	    printf("^");
	    c |= '@';
	}
	printf("%c", c);
    }
    printf("'\n");
}

doshw(char **argv)
{
    printf("tbc = %d\n", tbc);
    outstr("goto1", vter.goto1);
    outstr("goto2", vter.goto2);
    outstr("screen1", vter.screen1);
    outstr("screen2", vter.screen2);
    outstr("in_xpar", xpar.in_xpar);
    outstr("lv_xpar", xpar.lv_xpar);
    printf("priority = %d\n", xpar.priority);
    return 1;
}
