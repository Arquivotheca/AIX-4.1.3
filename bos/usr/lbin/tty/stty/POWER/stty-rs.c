#ifndef lint
static char sccsid[] = "@(#)72  1.6  src/bos/usr/lbin/tty/stty/POWER/stty-rs.c, cmdtty, bos411, 9428A410j 12/21/93 01:42:08";
#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 26,27, 83
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
#include <sys/rs.h>

#include        <locale.h>
#include        <nl_types.h>
#include        "stty-rs_msg.h"

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_STTY_RS,num,str)

void error(char *s);
int (*search(register char *cmd))();
int doshow(char **argv);
int dodma(char **argv);
int dotrig(char **argv);
int dotbc(char **argv);
int domil(char **argv);

struct cmds {
    char *name;
    int (*func)(char **argv);
} cmds[] = {
    "dma", dodma,
    "mil", domil,
    "rtrig", dotrig,
    "show", doshow,
    "tbc", dotbc,
};
#define CMDS (sizeof(cmds) / sizeof(struct cmds))

char *progname;
struct rs_info rs;
int changed;

main(int argc, char *argv[])
{
    int (*f)();

    setlocale(LC_ALL,"") ;
    catd = catopen(MF_STTY_RS, NL_CAT_LOCALE);

    if (progname = strrchr(argv[0], '/'))
	++progname;
    else
	progname = argv[0];
    ++argv;

    if (ioctl(0, RS_GETA, &rs))
	error("RS_GETA");

    while (*argv) {
	if (!(f = search(*argv))) {
	    fprintf(stderr, MSGSTR(UNKNOWN, "%s: %s unknown\n"), progname,
		    *argv);
	    ++argv;
	    continue;
	}
	argv += (*f)(argv);
    }
    
    if (changed && ioctl(0, RS_SETA, &rs))
	error("RS_SETA");
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

int doshow(char **argv)
{
    printf("dma = %d, rtrig = %d, tbc = %d, mil = %d\n",
	   rs.rs_dma, rs.rs_rtrig, rs.rs_tbc, rs.rs_mil);
    return 1;
}

int dodma(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(DMAUSAGE, "Usage %s dma level\n"));
	return 1;
    }
    changed = 1;
    rs.rs_dma = atoi(*argv);
    return 2;
}

int dotrig(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(RTRIGUSAGE, "Usage %s rtrig level\n"));
	return 1;
    }
    changed = 1;
    rs.rs_rtrig = atoi(*argv);
    return 2;
}

int dotbc(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(TBCUSAGE, "Usage %s tbc level\n"));
	return 1;
    }
    changed = 1;
    rs.rs_tbc = atoi(*argv);
    return 2;
}

int domil(char **argv)
{
    if (!*++argv) {
	fprintf(stderr, MSGSTR(MILUSAGE, "Usage %s mil level\n"));
	return 1;
    }
    changed = 1;
    rs.rs_mil = atoi(*argv);
    return 2;
}

