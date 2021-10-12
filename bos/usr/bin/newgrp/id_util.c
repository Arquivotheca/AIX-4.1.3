static char sccsid[] = "@(#)01	1.11  src/bos/usr/bin/newgrp/id_util.c, cmdsuser, bos411, 9428A410j 3/23/94 12:57:15";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: checkgids
 *		checkuids
 *		compare
 *		getgids
 *		getids
 *		getlowest
 *		getnewids
 *		getuids
 *		inkids
 *		lockfile
 *		sortem
 *		unlockfile
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>		/* for O_RDWR etc. 	*/
#include <sys/stat.h>		/* for stat call 	*/
#include <pwd.h>		/* passwd struct 	*/
#include <grp.h>		/* group struct 	*/
#include "tcbauth.h"

static	uid_t	*sortem(struct ids *,uid_t);
static	int	compare(uid_t *,uid_t *);
static	int	getlowest(uid_t *ids,uid_t,uid_t,uid_t);
static	int	lockfile(int,int);
static	void	unlockfile(int);
static	int	checkgids(gid_t,gid_t);
static	int	checkuids(uid_t,uid_t);
static	int	getnewids(gid_t *,gid_t *,gid_t *,gid_t *,char *);
static	int	getgids(struct ids *,char *);
static	int	getuids(struct ids *,char *);

/*
 * FUNCTION:	compare
 *
 * DESCRIPTION:	compare routine used by qsort()
 *
 * RETURNS:	int.
 */
static int
compare(uid_t *a, uid_t *b)
{
	int x, y;

	x = *a;
	y = *b;
	return(x - y);
}

/*
 * FUNCTION:	sortem
 *
 * DESCRIPTION:	sorts the id list by size.
 *
 * RETURNS:	sorted id's array
 *
 */
static	uid_t	*
sortem(struct ids *array,uid_t c)
{
uid_t		i=0;	/* counter */
uid_t		*ids;
struct	ids	*arrayp;

	arrayp = array;
	if ((ids = (uid_t *)calloc(c,sizeof(uid_t)+1)) == (uid_t *)NULL) 
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	while (arrayp->next)
	{
		ids[i++] = arrayp->id;
		arrayp = arrayp->next;
	}
	
	ids[i] = arrayp->id;

	qsort(ids, i+1, sizeof(uid_t), compare);

	return(ids);
}

/*
 * FUNCTION:	getlowest
 *
 * DESCRIPTION:	gets the lowest numbered id not in use.
 *
 * PASSED:	the id list, where to start (admin: count = 0 or
 *		regular: count = 200), the max id to be allotted,
 *		the number of ids to search.
 *
 * RETURNS:	the id.
 *
 *
 *	uid_t	*ids;		 the id list
 *	int	lower;		 where to start the search
 *	int	max;		 the max id to be allotted
 *	int	c;		 the number of ids
 */

static	int
getlowest(uid_t *ids,uid_t lower,uid_t max,uid_t c)
{
uid_t		i;	/* counter */
uid_t		newid;

	/*
	 *  We are checking a sorted array of id's, and max is
	 *  the last element in the array.  Therefore, if count
	 *  is greater than max, then it must be unused, and
	 *  we return that value.
	 */
	if (lower > max)
		return(lower);
		
	for(i=0;i<c;i++)
	{
		newid=ids[i]+1;
		/*
		 * We are checking for a 'hole' in the sorted array (ids).
		 * There are three possible cases:
		 *    1. current id == next id 
		 *    2. current is 1 less than next id (sequential), or
		 *    3. current and next id are not sequential (i.e. 
	         *	 next - current > 1)
		 * newid is valid only in case 3.
		 */
		if ((newid>=lower) && (newid<=max) && (ids[i+1]-ids[i]>1))
			return(newid);
	}
	/*
	 * Didn't find a gap if we get here
	 */
	return(-1);
}


/*
 * FUNCTION:	inkids
 *
 * DESCRIPTION:	opens file and updates the values.
 *
 * PASSED:	the flag of id (admin or not), the id's.
 *
 * RETURNS:	None.
 *
 * PASSED:
 *	int	flag;	 group or user
 *	int	type;	 admin or not
 *	uid_t	auid;	 the admin id
 *	uid_t	uid;	 the ordinary id
 *	gid_t	agid;	 the admin id
 *	gid_t	gid;	 the ordinary id
 *	int	f;	 file descriptor
 *	char	*event;	 auditing event
 */

void
inkids(int flag,int type,uid_t auid,uid_t uid,
		gid_t agid,gid_t gid,int f)
{
char	mbuf[BUFSIZ];
int	siz;

	/*
	 * depending on whether we're creating a user or group, increment the
	 * appropriate id based on the flag variable (admin or ordinary) before
	 * the update.
	 */

	/* if creating a user */
	if(flag == 0)
	{	
		/* if admin user */
		if (type)
			auid++;
		/* if ordinary user */
		else
			uid++;
	}
	/* if creating a group, flag == 1 */
	else
	{
		/* if admin group */
		if (type)
			agid++;
		/* if ordinary group */
		else
			gid++;
	}	


	/*
	 *  Let's build the list of new id's
	 */

	sprintf(mbuf,"%lu %lu %lu %lu",auid,uid,agid,gid);
	siz = strlen(mbuf) + 1;

	if (ftruncate(f, 0) < 0)
	{
		close (f);
		return;
	}
	lseek (f, 0, 0);

	/*
	 * we don't care about return code here
	 */
        write(f,mbuf,siz);
	unlockfile(f);
	close(f);
}

/*
 * FUNCTION:	lockfile
 *
 * DESCRIPTION:	puts lock on a file.
 *
 * RETURNS:	0 or -1 for error.
 *
 */

static	int
lockfile(int f,int time)
{
register int	n = 0;
static struct flock flk = { F_WRLCK, 0, 0, 0, 0, 0 };

		/* lock the file */
	while (fcntl(f,F_SETLK,&flk) < 0)
	{
		/* try for time # of seconds */
		n++;
		if (n == time)
			return (-1);

		/* sleep and try again */
		sleep (1);
	}
	return (0);
}

/*
 * FUNCTION:	unlockfile
 *
 * DESCRIPTION:	takes the lock off /etc/security/.ids.
 *
 * RETURNS:	none.
 *
 */

static	void
unlockfile(int f)
{
static struct flock filelock = { F_UNLCK, 0, 0, 0, 0, 0 };

	fcntl(f, F_SETLK, &filelock);
}

/*
 * FUNCTION:	getids
 *
 * DESCRIPTION:	read the ids from /etc/security/.ids,
 *		if not successful call getnewids to read /etc/group and 
 *		regenerate /etc/security/.ids.
 *
 * PASSED:	the flag specifying user or group creation, the name of the 
 *		user/group to be added, the type (admin or not),
 *		places to put the new ids;
 *
 * RETURNS:	the file descriptor exits on error.
 *
 * PASSED:
 *	char	*name;		 the name of the user/group to add
 *	uid_t	*auid;
 *	uid_t	*uid;
 *	gid_t	*agid;
 *	gid_t	*gid;
 *	char	*event;		 the auditing event
 */

int
getids(char *name,uid_t *auid,uid_t *uid,gid_t *agid,gid_t *gid,char *event)
{
int		f;
struct	stat 	sbuf;           /* stat buffer 		   */
unsigned int 	orgsize;        /* size of org file 	   */
char		*mbuf = NULL;	/* string of ids from file */
register int	create = 0;
int		end;
int		rootid = 0;
int		securityid = 0;

	/* open /etc/security/.ids */
	if ((f = open(DOTIDFILE, O_RDWR)) < 0) 
	{
		/* doesn't exist, create it from scratch */
		if ((f = open (DOTIDFILE, O_CREAT | O_RDWR, 0600)) < 0)
		{
			fprintf(stderr,OPENERR,DOTIDFILE);
			exitax(event,errno,name,NULL,PRINT);
		}

		/*
		 *  Set the owner and group to "root" and "security".
		 *  If it fails, it's ok.
		 */
		getgroupattr(SECURITY,S_ID,&securityid,SEC_INT);
		fchown(f,rootid,securityid);

		create = 1;
	}

	/* lock the file. try for 120 seconds */
	if (lockfile(f,120))
	{
		close(f);
		fprintf(stderr,ERRLOCK,DOTIDFILE);
		exitax(event,errno,name,NULL,PRINT);
	}

	if (!create)
	{
		struct stat tmpbuf;

		/* get the size of the original */
		if (fstat(f, &sbuf) < 0)
		{
			unlockfile(f);
			close (f);
			fprintf(stderr,IDINFO);
			exitax(event,errno,name,NULL,PRINT);
		}

		orgsize = sbuf.st_size;
		/* Allocate memory for read */
		if ((mbuf = (char *)malloc(orgsize + 3)) == NULL)
		{
			unlockfile(f);
			close (f);
			fprintf(stderr,MALLOC);
			exitax(event,errno,name,NULL,PRINT);
		}
		if (read(f,mbuf,orgsize) != orgsize)
		{
			free(mbuf);
			unlockfile(f);
			close (f);
			fprintf(stderr,IDINFO);
			exitax(event,errno,name,NULL,PRINT);
		}


		/* NULL terminate string */
		*(mbuf + orgsize - 1) = ' ';
		*(mbuf + orgsize)     = '0';
		*(mbuf + orgsize + 1) = '\0';
	}

	/*	
	 * check .ids string for validity
	 */
	if ((sscanf(mbuf,"%lu %lu %lu %lu %u",auid,uid,agid,gid,&end) != 5) ||
					(end != 0)) 
	{
 		if (getnewids(auid,uid,agid,gid,event))
 		{
 			unlockfile(f);
 			close(f);
 			if (mbuf)
 				free(mbuf);
 			fprintf(stderr,IDINFO);
 			exitax(event,EINVAL,name,NULL,PRINT);
 		}
	}
		
	/* see if ids are already used */
	if (checkgids(*agid,*gid) != 0)
	{
		if (getnewids(auid,uid,agid,gid,event))
		{
			unlockfile(f);
			close(f);
			if (mbuf)
				free(mbuf);
			fprintf(stderr,IDINFO);
			exitax(event,EINVAL,name,NULL,PRINT);
		}
	}

	if(checkuids(*auid,*uid) != 0)
	{
		if (getnewids(auid,uid,agid,gid,event))
		{
			unlockfile(f);
			close(f);
			if (mbuf)
				free(mbuf);
			fprintf(stderr,IDINFO);
			exitax(event,EINVAL,name,NULL,PRINT);
		}
	}

	/*
	 *  File is not unlocked so inkids() can
	 *  update the file with new ids
	 */
	return(f);
}

/*
 * FUNCTION:	checkgids
 *
 * DESCRIPTION:	checks the new gids for already existing.
 *
 * PASSED:	the type of group to be added, and places to store the ids.
 *
 * RETURNS:	0 for success and -1 for error.
 *
 */

static	int
checkgids(gid_t agid,gid_t gid)
{
	/*
	 * does id already exist?
	 */
	if(IDtogroup(agid) != NULL)
		return(-1);

	if(IDtogroup(gid) != NULL)
		return(-1);

	return(0);
}

/*
 * FUNCTION:	checkuids
 *
 * DESCRIPTION:	checks the new uids for already existing.
 *
 * PASSED:	the type of user to be added, and places to store the ids.
 *
 * RETURNS:	0 for success and -1 for error.
 *
 */

static	int
checkuids(uid_t auid,uid_t uid)
{
	/*
	 * does id already exist?
	 */
	if(IDtouser(auid) != NULL)
		return(-1);

	if(IDtouser(uid) != NULL)
		return(-1);

	return(0);
}

/*
 * FUNCTION:	getnewids
 *
 * DESCRIPTION:	checks the new ids for already existing.
 *		makes sure that the normal group/user id is not < 200.
 *		(the admin id is not checked because the id may be > 200).
 *
 * PASSED:	the type of group to be added, and places to store the ids.
 *
 * RETURNS:	0 for success and -1 for error.
 *
 */

static	int
getnewids(gid_t *auid,gid_t *uid,gid_t *agid,gid_t *gid,char *event)
{
gid_t	*grps;
uid_t	*usrs;
uid_t	c1=0, c2=0;
struct	ids	garray;		/* array for gids */
struct	ids	uarray;		/* array for uids */
	
	/*
	 * get ids from /etc/group
	 */
	c1 = getgids(&garray,event);

	grps = sortem(&garray,c1);

	/*
	 * gets lowest unused id >= 0 and less than the current largest
	 * gid + 1
	 */
	if ((*agid = getlowest(grps,0,(grps[c1-1] + 1),c1)) == -1)
		return(-1);

	/*
	 * gets lowest unused id >= 200 and less than the current 
	 * largest gid + 1
	 */
	if ((*gid = getlowest(grps,200,(grps[c1-1] + 1),c1)) == -1)
		return(-1);
	
	/*
	 * get ids from /etc/passwd
	 */
	c2 = getuids(&uarray,event);

	usrs = sortem(&uarray,c2);

	/* 
	 * gets lowest unused id >= 0 and less than the current largest
	 * uid + 1 
	 */
	if ((*auid = getlowest(usrs,0,(usrs[c2-1] + 1),c2)) == -1)
		return(-1);

	/*
	 * gets lowest unused id >= 200 & less than the current largest
	 * uid + 1
	 */
	if ((*uid = getlowest(usrs,200,(usrs[c2-1] + 1),c2)) == -1)
		return(-1);

	return (0);
}

/*
 * FUNCTION:	getgids
 *
 * DESCRIPTION:	gets all the system gids and builds a linked list.
 *
 * RETURNS:	the number of users found.
 */

static	int
getgids(struct ids *array,char *event)
{
struct	ids	*arrayp;	/* temporary pointer		*/
struct	ids	*arraypn;	/* temporary pointer		*/
struct	group	*grp;		/* return from getgrent		*/
register int	i = 0;		/* the number of users returned	*/

	arrayp = array;

	setgrent();
	while ((grp = (struct group *)getgrent()) != NULL)
	{
		if (i == 0)
			arrayp->id = grp->gr_gid;
		else 
		{
			if ((arraypn = (struct ids *)calloc(1,
					sizeof(struct ids)+1)) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitax(event,errno,NULL,NULL,PRINT);
			}
			arrayp->next = arraypn;
			arrayp = arraypn;
			arrayp->id = grp->gr_gid;
		}
		arrayp->next = NULL;
		i++;
	}

	endgrent();

	return(i);
}

/*
 * FUNCTION:	getuids
 *
 * DESCRIPTION:	gets all the system uids and builds a linked list.
 *
 * RETURNS:	the number of users found.
 *
 */

static	int
getuids(struct ids *array,char *event)
{
struct	ids	*arrayp;	/* temporary pointer		*/
struct	ids	*arraypn;	/* temporary pointer		*/
struct	passwd	*pwd;		/* return from getpwent		*/
register int	i = 0;		/* the number of users returned	*/

	arrayp = array;

	setpwent();
	while ((pwd = (struct passwd *)getpwent()) != NULL)
	{
		if (i == 0)
			arrayp->id = pwd->pw_uid;
		else 
		{
			if ((arraypn = (struct ids *)calloc(1,
					sizeof(struct ids)+1)) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitax(event,errno,NULL,NULL,PRINT);
			}
			arrayp->next = arraypn;
			arrayp = arraypn;
			arrayp->id = pwd->pw_uid;
		}
		arrayp->next = NULL;
		i++;
	}

	endpwent();

	return(i);
}
