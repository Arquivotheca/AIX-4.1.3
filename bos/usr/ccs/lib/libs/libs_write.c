static char sccsid[] = "@(#)20	1.14.1.7  src/bos/usr/ccs/lib/libs/libs_write.c, libs, bos411, 9428A410j 10/12/93 09:22:11";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions
 *
 * FUNCTIONS: wrpass, wrgrps, wraudit, wrname, write, updatefile
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <usersec.h>
#include "libs.h"


extern	struct eval	*findeval ();	/* find name attribute value struct */
extern	char	*nextrec ();		/* parse next record */
extern	char	*nextattr ();		/* parse to next attribute */
extern	char	*putlist ();		/* put a list into the cache */
char		*wrname ();		/* write to a file where  */

char		*wrgrps();		/* write to group file */
char		*wrpass();		/* write to passwd file */
char		*wraudit();		/* write to audit file */
int		commit();		/* interface to updatefile */

static int	rmfilelocks();		/* remove locks on files */
static int	setfilelocks();		/* put locks on files */
static int	writefiles();		/* write out files */
static int	lckfile();
static	char	*getname (char *name,char *ptr);
static int	updatethefiles();

char *
wrpass (struct 	eval 	*eval, 
	char 	*mbuf,
	int    	fi, 
	char 	**remainder, 
	int 	*firstpart, 
	struct 	attr 	*atab, 
	struct	atfile 	*ftab, 
	int	nattr)
{
	struct	atval	*aval = &(eval->atvals[IDX_PGRP]);
	gid_t	gid;			/* Numeric group id	   	     */
	char	convert[64];		/* Temp storage for sprintf()	     */
	char	*tval = (char *)NULL;	/* Temp storage for "val" 	     */
	char	*buf;			/* Return storage from wrname()	     */

	/* 
	 * Change the primary group from a string name to a group id
	 * for the database write.
	 */
	if (aval->val && (grouptoID(aval->val, &gid) == 1))
	{
		/* Convert gid into a character string */
		sprintf(convert, "%d", gid);

		/* Store the current string name */ 
		tval = strdup(aval->val);

		/* If not enough space in "val" then realloc */
		if (strlen(convert) > strlen(aval->val))
			aval->val = realloc(aval->val, strlen(convert)+1);

		strcpy(aval->val, convert);
	}
	buf = wrname(eval, mbuf, fi, remainder, firstpart, atab, ftab, nattr);

	/*
	 * Now return the eval struct to it's original state.  This will
	 * return the group to it's proper name instead of a group id.
	 */
	if (tval && aval->val)
	{
		free(aval->val);
		aval->val = tval;
	}
	return(buf);
}

char *
wrgrps (eval, mp, fi, remainder, firstpart, atab, ftab, nattr)
char		*mp;
struct	eval	*eval;
int		fi;
char		**remainder;
int		*firstpart;
struct	attr	*atab;
struct	atfile	*ftab;
int		nattr;
{
char		*nbuf;		/* new buffer */
char		*np;		/* new buffer pointer */
char		*user;		/* the user */
int		ulen;		/* user strlen */
int		add;		/* boolean indicating user added to group */
struct	attr	*a;		/* table attribute for groups */
char		*newgroups;	/* value of the new groups for this user */
char		name[BUFSIZ];	/* place to hold group name */

	/* need memory of original size + the number of groups adding user to */
	/* assuming we are not adding to more than BUFSIZ/strlen(user) groups */
	if ((nbuf = malloc (strlen (mp) + BUFSIZ)) == NULL)
		return NULL;

	a = atab + IDX_GROUPS;
	newgroups = (eval->atvals[a->ai].val);
	np = nbuf;
	user = eval->name;
	ulen = strlen (user);

	/* find first group */
	while (isspace ((int)*mp))
		*np++ = *mp++;
	/*
	 * For each group: 
	 *	1. see if we add/remove the user name to/from  this group 
	 *	2. search the member list of the group 
	 *	3. keep, add, or delete the user
	 */
	while (*mp)
	{
		/* do we add to this group or take out? */
		{
		char	*val = newgroups;

			add = 0;
			while (*val)
			{
				getname (name,mp);
				if (strcmp (val,name) == 0)
				{
					add = 1;
					break;
				}
				while (*val++);
			}
		}
		COPYFIELD(np, mp);	/* group name */
		COPYFIELD(np, mp);	/* group passwd */
		COPYFIELD(np, mp);	/* group id */

		/* search this field (member list) for name */
		while ((*mp) && (*mp != '\n'))
		{
			getname (name,mp);
			if (strcmp (user,name) == 0)
			{
				break;
			}

			/* next member name */
			while ((*mp) && (*mp != '\n'))
			{
				if (*mp == ',')
				{
					*np++ = *mp++;
					break;
				}
				*np++ = *mp++;
			}

		}
		/* if at end then add it */
		if (*mp == '\n')
		{
			if (add)
			{
			char	*u = user;

				mp--;
				if (*mp != ':')
					*np++ = ',';
				mp++;
				while (*u)
					*np++ = *u++;
			}
			*np++ = *mp++;
			continue;
		}
		/* found user in member list do we keep or delete? */
		if (add)
		{
			/* keep member name and rest of field */
			COPYFIELD(np, mp);
		}
		else
		{
			/* delete member name */
			while ((*mp) && (*mp != ',') && (*mp != '\n'))
			{
				mp++;
			}
			/* don't leave a dangling comma in middle or on end */
			if (*(np - 1) == ',')
			{
				np--;
			}
			/* don't start member list with a comma */
			if (*mp == ',' && *(np - 1) == ':')
			{
				mp++;
			}
			/* put the rest of the entry into the list */
			while ((*mp) && (*mp != '\n'))
			{
				*np++ = *mp++;
			}
			if (*mp == '\0')
				break;

			/* don't forget to add the newline */
			*np++ = *mp++;
		}
	}
	/* end the new buffer */
	*np = '\0';

	return (nbuf);
}

char *
wraudit (eval, mbuf, fi, remainder, firstpart, atab, ftab, nattr)
char		*mbuf;
struct	eval	*eval;
int		fi;
char		**remainder;
int		*firstpart;
struct	attr	*atab;
struct	atfile	*ftab;
int		nattr;
{
char	*nbuf;
char	*np;	/* runner for nbuf */
char	*mp;	/* runner for mbuf */
char	*user = eval->name;
int	ulen = strlen (user);
char	name[BUFSIZ];

	mp = mbuf;
	nbuf = NULL;
	/* find first record */
	if (!isalnum ((int)*mp))
	{
		mp = nextrec (mp);
	}
	while (*mp)
	{
		/* find the "users" stanza */
		if (strncmp (mp, "users:", 6) == 0)
		{
		char	*nextp;

			/* find the attribute of the user's name */
			while (nextp = nextattr (mp))
			{
				getname (name,nextp);
				if (strcmp (user,name) == 0)
				{
					break;
				}
				mp = nextp;
			}
			/* build an attribute name for this user */
			if ((nbuf = malloc (BUFSIZ)) == NULL)
			{
				return (NULL);
			}
			np = nbuf;

			/* adding attribute or replacing existing value? */
			if (!nextp)
			{
				/* add user as an attribute */
				SKIPeol (mp);
				*firstpart = mp - mbuf;
			}
			else
			{
				/* replace user */
				SKIPeol (mp);
				*firstpart = mp - mbuf;
				SKIPeol (mp);
			}
			*remainder = mp;

			{
			struct atval	*aval = &(eval->atvals[IDX_AUDIT]);
			unsigned short	flags = eval->atvals[IDX_AUDIT].flgs;

				if (aval->flgs & NUKED)
				{
					aval->flgs &= ~CACHED;
					aval->flgs &= ~NUKED;
					*np = '\0';
					*remainder = mp;
					return (nbuf);
				}
				
				*np++ = '\t';
				while (*user)
					*np++ = *user++;
				*np++ = ' ';
				*np++ = '=';
				*np++ = ' ';

				np = putlist (np, aval->val,flags);
				aval->flgs &= ~CACHED;
			}
			*np++ = '\n';
			*np = '\0';
			return (nbuf);
		}
		mp = nextrec (mp);
	}
	return (NULL);
}

char *
wrname (eval, mbuf, fi, remainder, firstpart, atab, ftab, nattr)
char		*mbuf;		/* holds content of file */
struct	eval	*eval;
int		fi;
char		**remainder;
int		*firstpart;
struct	attr	*atab;
struct	atfile	*ftab;
int		nattr;
{
int	found = 0;
char	*nbuf;
char	*mp;
char	*user = eval->name;
int	ulen = strlen (user);
char	*new;
char	*save_nis_loc = NULL;	/* location of first '+' */

	mp = mbuf;
	nbuf = NULL;
	/* advance to the first record */
	if (isspace ((int)*mp) || *mp == '*')
		mp = nextrec (mp);

	/* get some space to hold new record */
	if ((nbuf = malloc (strlen(mbuf) + BUFSIZ)) == NULL)
	{
		return (NULL);
	}

	while (*mp)
	{
		if (mp && *mp == '+')
		{
			if (save_nis_loc == NULL)
				/*
				 * save location of first '+', but keep
				 * searching for the name in case it's
				 * after the NIS entry...
				 */
				save_nis_loc = mp;
		}
		else

		/* find the stanza to be changed */
		if (strncmp (mp, user, ulen) == 0 && mp[ulen] == ':')
		{
			found = 1;
			/* 
			 * mark size of the firstpart by subtracting pointers 
			 * mp is where the string we want is 
			 * mbuf is the beginning  of the original file string
			 */
			*firstpart = mp - mbuf;
			/* creat new record : chgcolon is called here */
			if ((*((ftab + fi)->new))(eval,mp,nbuf,fi,atab,nattr))
			{
				return (NULL);
			}
			*remainder = nextrec (mp);
			break;
		}
		mp = nextrec (mp);
	}

	if (!found)
	{
		/*
		 * restore mp back to where the '+' is
		 */
		if (save_nis_loc)
			mp = save_nis_loc;

		/* get enough space for new entry */
		if ((new = malloc(BUFSIZ)) == NULL)
			return (NULL);

		if (!strcmp((ftab + fi)->org,PWD_FILENAME))
			sprintf(new,"%s:*:::::\n",user);

		else if (!strcmp((ftab + fi)->org,GROUP_FILENAME))
			sprintf(new,"%s:!::\n",user);

		else
		{
			/* if there isn't two newlines add one */
			if (*(mp - 1) != '\n' || *(mp - 2) != '\n')
				sprintf(new,"\n%s%c\n",user,':');
			else
				sprintf(new,"%s%c\n",user,':');
		}
		
		*firstpart = mp - mbuf;

		/* creat new record : chgcolon is called here */
		if ((*((ftab + fi)->new))(eval,new,nbuf,fi,atab,nattr))
			return (NULL);

		free (new);

		*remainder = mp;
	}

	return (nbuf);
}

/*
 * NAME: commit()
 *                                                                    
 * FUNCTION: call file locking routine, increment through table
 *		and update attributes that have changed.
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *	
 * RETURNS: 0 if successful -1 if fail.
 *
 */  

int
commit (eval, atab, ftab, nattr, nftab, head, name)
struct	eval	*eval;	/* pointer to attribute linked list */
struct	attr	*atab;	/* pointer to attribute table */
struct	atfile	*ftab;	/* pointer to file table */
int		nattr;	/* number of attributes in data base */
int		nftab;	/* number of files in data base */
char		*name;
struct	ehead	*head;
{
register int	i;
register int	saved = 0;

	/* if name is not specified update all the accounts in cach */
	if (!name)
	{
		/* lock all the files that have been updated */
		for (eval = head->eval; eval; eval = eval->next)
		{
			if (setfilelocks(eval,atab,ftab,nattr,0,nftab))
				return(-1);
		}

		/* for each account update the file buffer */
		for (eval = head->eval; eval; eval = eval->next)
		{
			if (updatethefiles (atab,ftab,nftab,eval,nattr))
				return (-1);
		}
	}
	else
	{
		/* find specified name attribute value structure */
		if ((eval = findeval(head, name)) == NULL)
		{
			errno = ENOENT;
			return (-1);
		}

		/* lock all the files that have been updated */
		if (setfilelocks(eval,atab,ftab,nattr,0,nftab))
			return(-1);

		/* for the named account update the file buffer */
		if (updatethefiles (atab,ftab,nftab,eval,nattr))
			return (-1);
	}

	/* write out all file buffers */
	if (writefiles(ftab,nftab))
		return(-1);

	/* remove all locks and close all files */
	if (rmfilelocks(ftab,nftab))
		return(-1);

	return (0);
}

/*
 * NAME: updatethfiles()
 *                                                                    
 * FUNCTION: walk through attribute table and update all file
 *		buffers that have been marked UPDATED
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *	
 * RETURNS: 0 if successful -1 if fail.
 *
 */  
static int
updatethefiles (atab,ftab,nftab,eval,nattr)
struct	attr	*atab;	/* pointer to attribute table */
struct	atfile	*ftab;	/* attribute file table */
int		nftab;	/* number of files in table */
struct	eval	*eval;	/* pointer to attribute linked list */
int		nattr;	/* number of attributes in data base */
{
int	i;
int	saved;

	/* update files that have the UPDATED bit set */
	for (i = 0; i < nattr; i++)
	{
		if (eval->atvals[i].flgs & UPDATED)
		{
			if (updatefile (NULL,atab,ftab,eval,i,nattr,0,0))
			{
				/* need to save original errno */
				saved = errno;
				rmfilelocks(ftab,nftab);
				errno = saved;
				return (-1);
			}
		}
	}
	return (0);
}


/*
 * NAME: writefiles()
 *                                                                    
 * FUNCTION: walk through file table and write out all
 *		data that has been marked DYNAMIC
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *	
 * RETURNS: 0 if successful -1 if fail.
 *
 */  

static int
writefiles(ftab,nftab)
struct	atfile	*ftab;	/* attribute file table */
int		nftab;	/* number of files in table */
{
register int	i;	/* counter */
register int	siz;	/* size of data to write out to file */
struct	atfile	*f;	/* the file entry we are interested in */

	/*
	 * go through file table and see which files have
	 * data that is DYNAMIC and ready to be written
	 * out to the file. then write it.
	 */

	for (i = 0; i < nftab; i++)
	{
		/* find the file table entry */
		f = ftab + i;

		/* if file is stored in dynamic buffer write it out */
		if (f->flags & DYNAMIC)
		{
			lseek (f->fd, 0, 0);
			/* write and truncate */
			siz = strlen(f->buf);
			if (write(f->fd,f->buf,siz) < 0)
			{
				fprintf(stderr,
				  (char *)MSGSTR(M_WRITE,DEF_WRITE),f->org);
				perror(" ");
				fflush(stderr);
				rmfilelocks(ftab,nftab);
				return (-1);
			}
			ftruncate (f->fd, siz);
			f->flags &= ~DYNAMIC;
			free (f->buf);
			f->buf = '\0';
		}
	}
	return (0);
}

/*
 * NAME: setfilelocks
 *                                                                    
 * FUNCTION: set locks on all files to be updated.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *	
 * RETURNS: 0 if successful else -1 is returned
 *
 */  

static	int
setfilelocks(eval, atab, ftab, nattr, rmflag, nftab)
struct	eval 	*eval;	/* linked list of new attribute values */
struct	attr	*atab;	/* attribute table */
struct	atfile	*ftab;	/* file table */
int		nattr;	/* number of attributes */
int		rmflag;	/* if we are removing a stanza */
int		nftab;	/* number of files in file table */
{
register int	fi;	/* index into file table */
register int	i;	/* counter */
struct	atfile	*f;	/* the file entry we are interested in */
	
	/*
	 * if rmflag is set we are locking all files
	 * in the file table.
	 */

	if (rmflag)
	{
		for (i = 0; i < nftab; i++)
		{
			f = ftab + i;
			if (lckfile(f,ftab,nftab))
				return (-1);
		}
	}

	/*
	 * go through linked list and find the files that
	 * have been updated and need to be locked
	 */

	else
	{
		for (i = 0; i < nattr; i++)
		{
			if (eval->atvals[i].flgs & UPDATED)
			{
				/* find the file table entry */
				fi = (atab + i)->fi;
				f = ftab + fi;
				if (lckfile(f,ftab,nftab))
					return (-1);
			}	
		}
	}
	return (0);
}
		
/*
 * NAME: lckfile
 *                                                                    
 * FUNCTION: puts a lock on the file specified by f.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *	
 * RETURNS: 0 if successful else -1 is returned
 *
 */  

static int
lckfile(f,ftab,nftab)
struct	atfile	*f;	/* the file entry */
struct	atfile	*ftab;	/* the file table */
int		nftab;	/* number of files in file table */
{
register int	n = 0;	/* number of seconds waited */
register int flags;

static struct flock flk = { F_WRLCK, 0, 0, 0, 0, 0 };

	
	/*
	 * see if the file is locked
	 * if not set the lock.
	 */

	if (!(f->fd) && !(f->flags & LOCKED))
	{
		/* open the file */
		if ((f->fd = open (f->org , O_RDWR)) < 0)
		{
			fprintf(stderr,(char *)MSGSTR(M_OPEN,DEF_OPEN),f->org);
			perror(" ");
			fflush(stderr);
			rmfilelocks(ftab,nftab);
			return (-1);
		}

		/* open these files with a close on exec */
		flags = fcntl (f->fd,F_GETFD,0);
		flags |= FD_CLOEXEC;
		if (fcntl (f->fd,F_SETFD,flags))
			return (-1);

		/* lock the file */
		while (fcntl(f->fd,F_SETLK,&flk) < 0)
		{
			/* try for 2 minutes */
			n++;
			if (n == 120)
			{
				fprintf(stderr,
				  (char *)MSGSTR (M_LOCK,DEF_LOCK),f->org);
				rmfilelocks(ftab,nftab);
				return (-1);
			}
			/* sleep and try again */
			sleep (1);
		}
		f->flags |= LOCKED;
	}
	return (0);
}

/*
 * NAME: rmfilelocks
 *                                                                    
 * FUNCTION: remove locks on all files that have them.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *	
 * RETURNS: 0
 *
 */  

static int
rmfilelocks(ftab,nftab)
struct	atfile	*ftab;	/* pointer to attribute file table */
int		nftab;	/* number of attributes in the table */
{
int		i;	/* counter */
struct	atfile	*f;	/* the file entry we are interested in */

static struct flock filelock = { F_UNLCK, 0, 0, 0, 0, 0 };

	/*
	 * look through the file table and see which 
	 * files have locks. if they do remove the locks
	 * and close the file.
	 */

	for (i = 0; i < nftab; i++)
	{
		f = ftab + i;
		if (f->flags & LOCKED)
		{
        		/* unlock the file */
			fcntl(f->fd, F_SETLK, &filelock);
			close(f->fd);
			f->fd = 0;
			f->flags &= ~LOCKED;
		}
	}
        return (0);
}

/*
 * NAME: updatefile
 *                                                                    
 * FUNCTION: update an attribute file
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *	
 *	Open the original file and copy contents to 'old'.
 *	Read original file into memory.
 *	Change file contents in memory with file specific routine (f->write)().
 *	Close all files (original file is locked throughout this routine).
 *                                                                   
 * RETURNS: 0 if successful else errno is returned
 *
 */  

int
updatefile (name,atab, ftab, eval, attr, nattr, rmflag, fi)
char	*name;			/* stanza name */
struct	attr	*atab;		/* attribute table */
struct	atfile	*ftab;		/* attribute file table */
struct	eval 	*eval;		/* linked list of new attribute values */
int		attr;		/* the attribute we are working on */
int		nattr;		/* number of attributes in the table */
int		rmflag;		/* flag to indicate rmoving entry */
int		fi;		/* index into file table */
{
int 		orgsize;        /* size of org file             */
int		newsize;	/* size of org file after update */
char    	*mbuf;          /* memory location for file     */
char		*newbuf;	/* memory for new contents of file */
char    	*nrec;          /* memory location for new record */
struct stat	sbuf;           /* stat buffer                  */
int		fdorg;		/* original */
int		fdold;		/* old file */
char		*remainder;	/* ptr to unchanged records following 
					the changed record */
int		leftover;	/* strlen of remainder before the realloc */
int		firstpart;	/* length of unchanged records 
					preceding the changed record */
struct	atfile	*f;		/* the file entry we are interested in */
char		*aclp;		/* pointer to acl info */

	/*
	 * find the file table entry.
	 * if we are not removing the entry 
	 * use the index into the file table
	 * to find the file information.
	 */

	if (!rmflag)
		fi = (atab + attr)->fi;

	f = ftab + fi;

	/* set original file descriptor */
	if (!(fdorg = f->fd))
	{
		errno = ENOENT;
		return (-1);
	}

	/* see if we have already read the file */
	if (!(f->flags & DYNAMIC))
	{
		/* check and make sure the write function exists 
		   if it's not there, that's ok. 
		*/
		if (!f->write)
		{
			return (0);
		}

		/* get acl of original file */
		if ((aclp = acl_fget(fdorg)) == NULL)
	                return(-1);

		/* get the size of the original */
        	if (fstat(fdorg, &sbuf) < 0)
        	        return(-1);
        	orgsize = sbuf.st_size;

		/* open and truncate or create the 'old' file */
		if ((fdold = open (f->old , O_WRONLY)) < 0)
		{
			if ((fdold = open (f->old, O_CREAT | O_RDWR, 000)) < 0)
				return (-1);
		}
		else
		{
	        	if (ftruncate(fdold, 0) < 0)
       			{
				close (fdold);
       		        	return(-1);
       			}
		}

		/* set acl of saved to that of the original file */
		if (acl_fput(fdold,aclp,1))
       		{
			close (fdold);
       	        	return(-1);
       		}

		/* set owner and group of saved file to that of original */
		if (fchown (fdold,sbuf.st_uid,sbuf.st_gid))
       		{
			close (fdold);
       	        	return(-1);
       		}

		/* read original contents of file into memory */
        	if ((mbuf = (char *) malloc(orgsize + 1)) == NULL)
        	{
			close (fdold);
                	return(-1);
        	}

		/* set buffer = to allocated space */
		f->buf = mbuf;
                f->flags |= DYNAMIC;

        	if (read(fdorg,mbuf,orgsize) != orgsize)
        	{
			close (fdold);
                	return(-1);
        	}
		/* write original to old */
        	if (write(fdold,mbuf,orgsize) != orgsize)
        	{
			close (fdold);
                	return(-1);
        	}
		close (fdold);

		*(mbuf + orgsize) = '\0';	/* initialize */
	}
	/* the file string is already in cached memory */
	else
	{
		mbuf = f->buf;
		orgsize = strlen(mbuf);
	}

	/*
 	* This file specific function does the following:
 	*	1. calculates the first part of the file that is 
 	*		unchanged - returned in firstpart
 	*	2. makes a new record with the updated 
	*		values - returned in nrec
 	*	3. finds the remainder of the original file that is 
 	*		unchanged - returned in remainder
 	*/
	
	firstpart = 0;			/* initialize */
	remainder = mbuf + orgsize;	/* initialize */

	/* if we are removing record */
	if (rmflag)
	{
		nrec = (char *)NULL;
		if ((*(f->del))(name, mbuf, &firstpart, &remainder))
		{
			/* if record wasn't found don't flag it */
			f->flags &= ~DYNAMIC;
			return (0);
		}
	}
	else
	{
		if (!(nrec = (*(f->write))(eval, mbuf, fi, &remainder, 
					&firstpart, atab, ftab, nattr)))
		{
			return (-1);
		}
	}

	leftover = strlen (remainder);

	/* calculate the new size of the original file */
	newsize = firstpart + strlen (nrec) + leftover;
	if ((newbuf = malloc (newsize + 2)) == NULL)
	{
		/* no change can be made */
		if (!rmflag)
			free (nrec);
		return (-1);
	}

	/* make a new file */
	memcpy (newbuf, mbuf, firstpart);
	newbuf[firstpart] = '\0';
	strcat (newbuf, nrec);
	if (leftover)
		strcat (newbuf, remainder);

	/* if pointer contains old data free it */
	if (f->flags & DYNAMIC)
	{
		free (f->buf);
	}

	/* set pointer to new record's malloc'd memory */
	f->buf = newbuf;

	/* set flags to DYNAMIC */
	f->flags |= DYNAMIC;

	if (!rmflag)
		free (nrec);

	return (0);
}

/*
 * NAME: deletefile
 *                                                                    
 * FUNCTION: deletes the user or group from data base.
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
deletefile (name,flag,nftab,atab,ftab,nattr,table)
char	*name;		/* data file to remove */
int	flag;		/* flag to not take entry out of /etc/security/passwd */
int	nftab;		/* # of files in file table */
struct	attr	*atab;	/* the attribute table */
struct	atfile	*ftab;	/* the file table */
int		nattr;	/* # of attributes in attribute table */
int		table;	/* USER_TABLE, etc. */
{
int	i;		/* counter for the table */
struct	atfile	*f;	/* pointer to a file table entry */

	/* put locks on all the files in the file table */
	if (setfilelocks(NULL,atab,ftab,nattr,1,nftab))
		return(-1);

	/* for all the files in the file table delete the entry */
	for (i = 0; i < nftab; i++)
	{
		f = ftab + i;

		/* do not change the group files if deleting user */
		if ((table == USER_TABLE) && 
			((!strcmp(f->org,GROUP_FILENAME)) ||
			!strcmp(f->org,SGROUP_FILENAME)))
			continue;

		/* if flag is 0 don't delete from /etc/security/passwd */
		if (!flag && !strcmp(f->org,SPWD_FILENAME))
			continue;

		updatefile (name,atab,ftab,NULL,0,nattr,1,i);
	}

	/* write out all the files from cach to the file */
	if (writefiles(ftab,nftab))
		return(-1);

	/* remove all locks put on earlier */
	if (rmfilelocks(ftab,nftab))
		return(-1);

	/* now remove the password history for user if flag is set */
	if (flag && _delete_PWDHistory( name ))
		return (-1);


	return (0);
}

static char *
getname (char *name,char *ptr)
{
	memset (name,0,BUFSIZ);
	while (*ptr && (*ptr != ' ') && (*ptr != '\0') && (*ptr != '\n') && 
	   (*ptr != '\t') && (*ptr != '=') && (*ptr != ':') && (*ptr != '*')
		&& (*ptr != ',')) 
		*name++ = *ptr++;
	return (ptr);
}

