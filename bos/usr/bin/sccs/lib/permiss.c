static char sccsid[] = "@(#)90 1.9 src/bos/usr/bin/sccs/lib/permiss.c, cmdsccs, bos411, 9428A410j 2/6/94 10:36:04";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: finduser, doflags, permiss, ck_lock
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

# include       "defines.h"
# include       <ctype.h>
# include	<grp.h>


finduser(pkt)
register struct packet *pkt;
{
	register char *p;
	char *user, *logname(), *fmalloc();
	char *strend(), *getline();
	int agid, i, ok_user, ngroups, none;
	int *groups = (int *)0;

	none = 1;
	user = logname();
	while ((p = getline(pkt)) != NULL && *p != CTLCHAR) {
		none = 0;
		ok_user = 1;
		repl(p,'\n','\0');	/* this is done for equal test below */
		if(*p == '!') {
			++p;
			ok_user = 0;
			}
		if (!pkt->p_user)
			if (equal(user,p))
				pkt->p_user = ok_user;
			else if (isdigit((int)(*p))) { /* if a group-id number */ 
				int i, agid;
				agid=atoi(p);
				if (!groups) {
					groups = (int *)fmalloc(NGROUPS*sizeof(int));
					ngroups = getgroups(NGROUPS, groups);
				}
				for (i=0; i<ngroups; i++)
					if (agid == groups[i]) {
						pkt->p_user = ok_user;
						break;
					}
			}
		*(strend(p)) = '\n';	/* repl \0 end of line w/ \n again */
	}
	if (groups)
		ffree((char*)groups);
	if (none)
		pkt->p_user = 1;
	if (p == NULL || p[1] != EUSERNAM)
		fmterr(pkt);
}


char	*Sflags[NFLAGS];

doflags(pkt)
struct packet *pkt;
{
	register char *p;
	register int k;
	char *getline(), *fmalloc();

	for (k = 0; k < NFLAGS; k++)
		Sflags[k] = 0;
	while ((p = getline(pkt)) != NULL && *p++ == CTLCHAR && *p++ == FLAG) {
		NONBLANK(p);
		k = *p++ - 'a';
		NONBLANK(p);
		Sflags[k] = fmalloc(size(p));
		copy(p,Sflags[k]);
		for (p = Sflags[k]; *p++ != '\n'; )
			;
		*--p = 0;
	}
}


permiss(pkt)
register struct packet *pkt;
{
	extern char *Sflags[];
	register char *p;
	register int n;
	char ErrMsg[512];

	if (!pkt->p_user)
		fatal(MSGCO(NOTAUTH, "\nYou are not authorized to make deltas.\n\
\tUse local procedures to place your user name\n\
\tor group number on the list of users who can make deltas.  (co14)\n"));  /* MSG */
	if (p = Sflags[FLORFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) < (n = patoi(p))) {
			sprintf(ErrMsg,MSGCO(RLSLTFLR, "Release %1$u is less than the lowest allowed release %2$u\n\
\t(the floor).\n\
\tThe specified release must be greater than or equal to the lowest\n\
\tallowed release.  (co15)\n"), pkt->p_reqsid.s_rel,n);  /* MSG */
			fatal(ErrMsg);
		}
	}
	if (p = Sflags[CEILFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) > (n = patoi(p))) {
			sprintf(ErrMsg,MSGCO(RLSGTCLNG, "Release %1$u is greater than highest allowed release %2$u\n\
\t(the ceiling).\n\
\tThe specified release must be less than or equal to the highest\n\
\tallowed release.  (co16)\n"), pkt->p_reqsid.s_rel,n);  /* MSG */
			fatal(ErrMsg);
		}
	}
	/*
	check to see if the file or any particular release is
	locked against editing. (Only if the `l' flag is set)
	*/
	if ((p = Sflags[LOCKFLAG - 'a']))
		ck_lock(p,pkt);
}

char l_str[NL_TEXTMAX];

ck_lock(p,pkt)
register char *p;
register struct packet *pkt;
{
	int l_rel;
	int locked;

	sprintf(l_str,MSGCO(FILLCKD, "The SCCS file is locked against editing (co23)\n"));
	
	locked = 0;
	if (*p == 'a')
		locked++;
	else while(*p) {
		p = satoi(p,&l_rel);
		++p;
		if (l_rel == pkt->p_gotsid.s_rel || l_rel == pkt->p_reqsid.s_rel) {
			locked++;
			sprintf(l_str,MSGCO(RLSLCKD, "Release %d is locked against editing. (co23)\n"), l_rel);  /* MSG */
			break;
		}
	}
	if (locked)
		fatal(l_str);
}
