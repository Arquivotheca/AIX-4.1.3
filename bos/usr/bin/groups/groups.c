static char sccsid[] = "@(#)22	1.19  src/bos/usr/bin/groups/groups.c, cmdsauth, bos411, 9428A410j 4/13/94 13:58:07";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/limits.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <locale.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <userpw.h> /* for PW_NAMELEN */

#include <nl_types.h>
#include "groups_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_groups,num,str) 

extern struct group *getgrent(); /* get the next group struct in the file */
extern getgroup();          /* get group membership list for user */

gid_t	groups[NGROUPS_MAX];


#define  NAME_TYPE      1
#define  GID_TYPE       2


/*
 * NAME: groups [user]
 *                                                                    
 * FUNCTION: Write to standard out user group membership.
 *           Default is current user.
 */  
main(int argc, char **argv)
{
int ngroups, i;
struct group *gr;

	(void ) setlocale (LC_ALL,"");       /* set up table for current lang */
	catd = catopen(MF_GROUPS, NL_CAT_LOCALE);

	if (argc > 2) {		/* too many arguments specified */
		fprintf(stderr,MSGSTR(M_USAGE, "usage: groups [user]\n"));
		exit(1);
	}
	if (argc > 1)
		showgroups(argv[1]);
	ngroups = getgroups(NGROUPS_MAX, groups); /* get list of current */
	                                          /* user's group membership */
	for (i = 0; i < ngroups; i++) {
		gr = getgrgid(groups[i]);             /* get data on group */
		if (gr == NULL)
			displayGroup( groups[i], "", GID_TYPE ) ;
		else
			displayGroup( groups[i], gr->gr_name, NAME_TYPE ) ;
	}
	printf("\n");
	catclose (catd);
	exit(0);
}

/*
 * NAME: showgroup
 *                                                                    
 * FUNCTION: Display group membership for user.
 */  
showgroups( char *user )
{
	struct group   *gr;
	struct passwd  *pw;
	char           **cp,
	               visit = 0 ;

	if ((pw = getpwnam(user)) == NULL) {       /* get data on user */
		fprintf(stderr,MSGSTR(M_MSG_3, "No such user\n"));
		exit(ENOENT);
	}
	while (gr = getgrent()) {          /* search group file for user */
		visit = 1 ;
		if (pw->pw_gid == gr->gr_gid) {
			static	int	once;

			/* 
			 * Display the primary group name, but only once,
			 * regardless of how many entries have the same
			 * GID.  This prevents duplicates when the group
			 * has been split into multiple lines due to
			 * length.
			 */

			if (once)
				continue;
			else
				once = TRUE;

			displayGroup( gr->gr_gid, gr->gr_name, NAME_TYPE ) ;
			continue;
		}	
		for (cp = gr->gr_mem; cp && *cp; cp++)
			if (strcmp(*cp, user) == 0) {
				displayGroup( gr->gr_gid, gr->gr_name, NAME_TYPE ) ;
				break;
			}
	}
	if ( ! visit )
	{
		fprintf(stderr,MSGSTR(M_MSG_4,
				"Could not get \"group\" information.\n"));
		exit(1);
	}
	printf("\n");
	exit(0);
}


typedef struct {
	gid_t gid ;
	char  *name ;
} MEMORY ;


displayGroup( gid_t gid, char *groupName, int type )
{
	static MEMORY  *memory     = NULL ;
	static int     memoryIndex = 0,
	               memorySize  = NGROUPS_MAX ;
	static char    *sep = "";
	int            i ;

	/* skip bad entry */
	if ((strlen(groupName) + 1) > PW_NAMELEN)
		return ;

	if ( (char *) memory == NULL )
		if ( ! ( memory = (MEMORY *)malloc((size_t)(memorySize * sizeof(MEMORY)) ) ) )
			exit( ENOMEM );

	for ( i = 0 ; i < memoryIndex && i < memorySize ; i++ )
		if ( gid == memory[ i ].gid &&
		     ! strncmp( groupName, memory[ i ].name, PW_NAMELEN ) )
			break ;

	/* skip duplicate entry */
	if ( i < memoryIndex )
		return ;

	if ( i == memorySize )
	{
		memorySize += NGROUPS_MAX ;
		if ( !( memory = (MEMORY *)realloc((void *)memory,
		     (size_t) (memorySize * sizeof(MEMORY)) )) )
			exit( ENOMEM );
	}

	if ( ! (memory[ memoryIndex ].name = (char *)calloc( sizeof(char),
	       PW_NAMELEN + 1 )) )
		exit( ENOMEM );
	memory[ memoryIndex ].gid = gid ;
	strncpy( memory[ memoryIndex++ ].name, groupName, PW_NAMELEN ) ;

	if ( type == GID_TYPE )
		printf("%s%d", sep, gid);
	else
		printf("%s%s", sep, groupName);
	sep = " ";
}
