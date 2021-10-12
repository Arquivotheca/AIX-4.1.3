static char sccsid[] = "@(#)66  1.17  src/bos/usr/ccs/lib/libc/initgroups.c, libcs, bos41J, 9520A_all 5/11/95 11:03:17";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: initgroups 
 *
 * ORIGINS: 26, 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <sys/param.h>
#include <grp.h>
#include "libc_msg.h"
#include <ctype.h>

#ifdef	_NO_PROTO
struct group *getgrent();
#else
struct group *getgrent(void);
#endif

#define MSGSTR(Num,Str) 	catgets(catd, MS_LIBC, Num, Str)

#ifndef	_THREAD_SAFE
static	nl_catd	catd;
static	int	groups[NGROUPS];
static	int	ngroups;
#endif	/* _THREAD_SAFE */

/*
 * NAME: isagroup
 *
 * FUNCTION: check for existing group membership
 *
 * RETURN VALUE DESCRIPTIONS:
 *	1 - group is already present
 *	0 - group is not already present
 */

static int
#ifndef	_THREAD_SAFE
isagroup (gid_t group)
#else
isagroup_r (gid_t group,int groups[],int ngroups)
#endif	/* _THREAD_SAFE */
{
	int	i;

	for (i = 0;i < ngroups;i++)
		if (groups[i] == group)
			return 1;

	return 0;
}

/*                                                                    
 * NAME: initgroups
 *
 * FUNCTION: initialize group membership list
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - 0 on success
 *	     - 1 on failure
 */

initgroups(uname, agroup)
	char *uname;
	gid_t agroup;
{
	register struct group *grp;
	register int i;
	char *grset;
	char *grstr;
	int grnum;
	long atol();
	char *uname_cpy;

#ifdef	_THREAD_SAFE
	nl_catd	catd;
	int	groups[NGROUPS];
	int	ngroups;
	char	line[BUFSIZ+1];
	struct	group	group;
	FILE	*grf = 0;

	grp = &group;

#endif	/* _THREAD_SAFE */

	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	ngroups = 0;
	if (agroup >= 0)
		groups[ngroups++] = agroup;

#ifndef  OLD_SLOW_WAY
	/*
	 * Make a copy of uname as it might have been obtained from a
	 * getpwuid() etc, might be a pointer to static date.  getgrset()
	 * calls getpwnam() which overwrites the static data (probably
	 * with 'root').
	 */

	uname_cpy = strdup(uname);
	if (uname_cpy && (grset = strdup (getgrset (uname_cpy)))) {
		for (grstr = strtok(grset,",");grstr;grstr = strtok(NULL,",")) {
			grnum = atol(grstr);
			if ((grnum || !strcmp(grstr,"0")) && grnum != agroup) {
				if (ngroups == NGROUPS) {
					fprintf(stderr, MSGSTR(M_INITG,
				"initgroups: %s is in too many groups\n"),
						uname);
					break;
				}
				groups[ngroups++] = grnum;
			}
		}
		free(grset);
	}
	if (uname_cpy)
		free (uname_cpy);
#else
#ifndef	_THREAD_SAFE
	setgrent();

	while (grp = getgrent()) {
		if (grp->gr_gid == agroup || isagroup (grp->gr_gid))
#else
	setgrent_r(&grf);
	while (getgrent_r(grp,line,sizeof(line)-1,&grf) == 0){
		if (grp->gr_gid == agroup || isagroup_r (grp->gr_gid,groups,ngroups))
#endif	/* _THREAD_SAFE */

			continue;
		for (i = 0; grp->gr_mem[i]; i++)
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups == NGROUPS) {
					fprintf(stderr, MSGSTR(M_INITG, "initgroups: %s is in too many groups\n"), uname);
					goto toomany;
				}
				groups[ngroups++] = grp->gr_gid;
			}
	}
toomany:
#ifndef	_THREAD_SAFE
	endgrent();
#else
	endgrent_r(&grf);
#endif	/* _THREAD_SAFE */
#endif	/* OLD_SLOW_WAY */
	if (setgroups(ngroups, groups ) < 0) {
		catclose(catd);
		perror("setgroups");
		return (1);
	}
	catclose(catd);
	return (0);
}
