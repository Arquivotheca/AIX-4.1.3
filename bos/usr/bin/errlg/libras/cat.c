static char sccsid[] = "@(#)83	1.2  src/bos/usr/bin/errlg/libras/cat.c, cmderrlg, bos411, 9428A410j 11/12/93 16:53:10";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: jcatopen, jcatgets, jcatprint
 *
 * ORIGINS: 27
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
 * Interface to message catalog services.
 *
 * NAME:     jcatopen
 * FUNCTION: interface to catopen
 * INPUTS:   'catalog' catalog name
 * RETURNS:  >= 0 0-based 'slot' value, like a file descriptor.
 *           -1   error in open of 'catalog'
 *
 * Each successful call to jcatopen returns a 0-based int.
 * This is the index into an internal table of nl_catd values.
 * The open routine will look in the directories (see 'pathlist')
 *   /usr/lpp/msg/english and
 *     .
 * if it can't find the catalog through catopen/NLPATH.
 *
 *
 * NAME:     jcatgets
 * FUNCTION: interface to NLcatgets
 * INPUTS:   'catslot'  slot number as returned from jcatopen
 *           'set'      message set number (usually 1)
 *           'mcn'      message number
 *           'str'      alternate string value if catgets fails.
 * RETURNS:  character pointer to the selected message, or to
 *           'str' if catgets failed.
 */

#include <stdio.h>
#include <limits.h>
#include <nl_types.h>
#include <libras.h>

extern char *catgets();
extern char *getenv();

#define NDESC 10
static nl_catd Desc[NDESC];
static Ndesc;

jcatopen(catalog)
char *catalog;
{
	int i;
	static initflg;

	if(catalog == 0)
		return(-1);
	if(!initflg) {
		initflg++;
		for(i = 0; i < NDESC; i++)
			Desc[i] = (nl_catd)CATD_ERR;
	}
	return(icatopen(catalog));
}

char *jcatgets(catslot,mcn)
{
	char *cp;
	char *str;

	if((unsigned)catslot >= Ndesc)
		return(0);
	str = "-";
	cp = catgets(Desc[catslot],1,mcn,str);
	if(cp == str || cp[0] == str[0] && cp[1] == '\0')
		return(0);
	return(cp);
}

static icatopen(catalog)
char *catalog;
{
	nl_catd catd;
	int i;
	int n;

	Debug("icatopen(%s)\n",catalog);

	if((catd = catopen(catalog,NL_CAT_LOCALE)) != (nl_catd)CATD_ERR){
		n = getslot();
		Desc[n] = catd;
		Debug("icatopen: slot=%d catd=%x\n",n,catd);
		return(n);
	}
	else {
		Debug("open of %s failed\n",catalog);
		return(-1);
	}
}

static getslot()
{
	int i;

	for(i = 0; i < Ndesc; i++)
		if(Desc[i] == (nl_catd)CATD_ERR)
			break;
	if(i == Ndesc) {
		if(i >= NDESC) {
			Debug("getslot: no more message catalog slots");
			genexit(1);
		}
		return(Ndesc++);
	}
	return(i);
}

jcatprint_chk(argc,argv,catalog)
char *argv[];
char *catalog;
{

	if(argc == 2 && streq(argv[1],"-NLSTEST-")) {
		jcatprint(catalog);
		exit(0);
	}
}

static jcatprint(catalog)
char *catalog;
{
	int i,n;
	char *cp;

	if((n = jcatopen(catalog)) < 0)
		return;
	for(i = 0; i < 1024; i++)
		if(cp = jcatgets(n,i))
			printf("%d\n%s\n",i,cp);
}
