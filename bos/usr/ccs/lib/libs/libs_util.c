static char sccsid[] = "@(#)21	1.20.1.4  src/bos/usr/ccs/lib/libs/libs_util.c, libs, bos411, 9428A410j 6/21/94 11:23:13";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: chgcolon
 *		chgstanza
 *		cpybool
 *		freeval
 *		getbool
 *		getint
 *		getlist
 *		getname
 *		getquotes
 *		getstr
 *		nextattr
 *		nextentry
 *		nextrec
 *		putbool
 *		putint
 *		putlist
 *		putquotes
 *		putstr
 *		rmaudit
 *		rmrecord
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <usersec.h>
#include <userpw.h>
#include <ctype.h>
#include <sys/errno.h>
#include <string.h>
#include "libs.h"

extern	struct	xlat	xtab[];
extern	struct	eval	*neweval();
extern	struct	ehead	*getableinfo(); /* gets ehead struct for table */
extern	struct	eval	*addeval();
extern	char		*putquotes();	/* puts attributes with quotes */
static	char		*getname();

#define MAXBOOL	16	/* Max length a boolean string may be (eg. "True") */
#define NFIELDS	7	/* Max number of fields in a colon file */

/*
 * Name: is_blank_line
 *
 * Function: Determine if the current line contains only whitespace
 *
 * Returns:  0 -- non-whitespace characters present on line
 *           1 -- only whitespace characters present on line
 */
int
is_blank_line (char *cp)
{
	for (;*cp != '\n' && isspace (*cp);cp++)
		;

	return *cp == '\n';
}

/* 
 * Name : nextentry
 *
 * Function: Get the next entry (stanza) from a specified file 
 *
 * Returns :  0 success
 *	     -1 and errno for error
 */
int
nextentry (void *val, int table)
{
	struct	eval	*eval;
	char		*name;
	struct  ehead	*head;
	struct	attr	*atab;
	int		nattr;
	int		nftab;
	struct	atfile	*f;

	if ((head = getableinfo(table,&nattr,&nftab,&atab,&f)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	if (!f->fhdle)
	{
		/* open or rewind the file */
		if ((f->fhdle = (*(f->open))(f->org, f->fhdle, NULL)) == NULL)
			return (-1);
	}

	/* get a new structure without a name */
	eval = neweval (NULL, nattr);
	if (!eval)
		return (-1);
	
	/* get the next entry and copy in the name */
	if (afnxtrec (f->fhdle) == NULL)
		return (-1);

	/* get the name and add the eval structure to the head */
	name = ((AFILE_t) f->fhdle)->AF_catr->AT_value;
	if ((eval->name = malloc (strlen (name) + 1)) == NULL)
		return (-1);

	strcpy (eval->name, name);
	(void) addeval (head, eval);

	/* name is returned by reference */
	*(char **) val = eval->name;	
	return (0);
}

/*
 * Name : nextattr
 *
 * Function: Positions the pointer to the next attribute in the file.
 *
 * Returns : mp   pointer to the next attribute.
 *	     NULL there is no attribute on this line.
 */
char *
nextattr (char *mp)
{
	NEXTLINE(mp);

	while ((*mp == '\t') || (*mp == ' '))
		mp++;

	if (!(*mp) || *mp == '\n')
		return (NULL);

	while (isspace((int)*mp))
		mp++;

	return (mp);
}

/*
 * Name : nextrec
 *
 * Function: Finds the start of the next recored
 * 
 * Returns: mp  pointer to the next line that does not start with a white
 *	        space or a comment.
 */
char *
nextrec(char *mp)
{
	/*
	 * the start of a record is anything 
	 * other than a white-space or '*' after
	 * a blank line
	 */

	while (*mp)
	{
		NEXTLINE (mp);
		if (!(isspace((int)*mp) || *mp == '*'))
		{
			break;	
		}
	}
	return (mp);
}

/*
 * Name : freeval
 *
 * Function: Remove the "ua1" eval structure from the eval list
 *
 * Returns: 0
 */
int
freeval (struct ehead *head, struct eval *ua1, int nattr)
{
	int	i;
	struct	eval   *eval,*peval;

	/* intialize */
        peval = (struct eval *)NULL;

	/*
	 * go through the linked list searching for the
	 * pointer passed in. if found, remove from the list.
	 * special handling for the case where the first eval
	 * structure is the one we want.
	 */

	for (eval = head->eval; eval; eval = eval->next)
	{
		if (ua1 == eval)
		{
			if (peval)
				peval->next = eval->next;
			else
				head->eval = eval->next;
			break;
		}
		peval = eval;
	}

	/* free and clear the attribute values and their flags */
	for (i = 0; i < nattr; i++) {
		if ((ua1->atvals[i].flgs & DYNAMIC)) {
			bzero (ua1->atvals[i].val,strlen(ua1->atvals[i].val));
			ua1->atvals[i].siz = (short)NULL;
			ua1->atvals[i].flgs = (unsigned short)NULL;
			free ((void *)ua1->atvals[i].val);
		}
	}

	/* free and clear the stanza name */
	if (ua1->name) {
		bzero (ua1->name,strlen(ua1->name));
		free ((void *)ua1->name);
	}

	/* free the atvals structure */
	free ((void *)ua1->atvals);

	/* set the next pointer to NULL */
	ua1->next = (struct eval *)NULL;

	/* free the eval structure */
	free ((void *)ua1);

	return (0);
}

/*
 * NAME: chgstanza
 *                                                                    
 * FUNCTION: change a record in a stanza file
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *                                                                   
 * RETURNS: 0
 *
 */  
int
chgstanza (struct eval *eval, 
	   char *cp, 	/* pointer to current memory buffer */
	   char *np,    /* pointer to new memory buffer     */
	   int  fi, 
	   struct attr *atab, 
	   int nattr)
{
	char		*at;		/* attribute */
	int		changed;
	int		i;
	struct	attr	*t;
	char		attr[BUFSIZ];
	char		*ptr;
	char		*val;
	char		*nxt; /* pointer to the next record */

	/* first, modify the existing attributes with their new values */
	/* copy name */
	CPYNAME (np, cp);
	while ((*cp) && ! is_blank_line (cp))
	{
		/* copy in all the spaces */
		CPYSPACE (np, cp);

		/* separate attribute name from attribute value */
		at = cp;
		ptr = at;
		val = &attr[0];

		while ((*ptr)&&(*ptr != ' ')&&(*ptr != '=')&&(*ptr != '\n'))
			*val++ = *ptr++;

		*val = '\0';

		changed = 0;
		/* see if this attribute has been updated */
		for (i = 0; i < nattr; i++)
		{
			if(!(eval->atvals[i].flgs & UPDATED))
			{
				/* check next attribute */
				continue;
			}

			t = atab+i;
			/* check for the correct file and matching attribute */
			if ((t->fi == fi) && (!strcmp(attr,t->atnam)))
			{
			char	*(*func)();
			struct	atval	*aval = &(eval->atvals[i]);
			unsigned short flags = eval->atvals[i].flgs;

				/* see if this value is being set to NULL */
				if(eval->atvals[i].flgs & NUKED)
				{
					changed = 1;
					SKIPeol(cp);

					/* reset attribute flags */
					aval->flgs &= ~CACHED;
					aval->flgs &= ~NUKED;
					aval->flgs &= ~UPDATED;

					/* this attribute has to be committed */
					aval->flgs |= COMMIT;

					/* nuke the tab */
					np--;
					if (*np != '\t')
						np++;
					break;
				}

				/* 
				 * copy the new value into the buffer
				 * to be written out to the file
				 */
				CPYATTR (np, cp);
				if ((func = xtab[t->xi].put))
				{
					if (t->qwerks & QUOTES)
					   np = putquotes(np,aval->val,&flags);
					else
					   np = (*(func))(np, aval->val,&flags);
				}
				*np++ = '\n';
				changed = 1;

				aval->flgs &= ~UPDATED;

				/* this attribute has to be committed */
				aval->flgs |= COMMIT;


				SKIPeol(cp);
				break;
			}
		}
		if (!changed)
		{
			/* copy attribute and value */
			CPYNAME (np, cp);
		}
	}
	/* add all new attributes */
	for (i = 0; i < nattr; i++)
	{
		/* see if this value is has been updated */
		if (!(eval->atvals[i].flgs & UPDATED))
		{
			/* check next attribute */
			continue;
		}

		/* see if this value is being set to NULL */
		if(eval->atvals[i].flgs & NUKED)
		{
			/* check next attribute */
			continue;
		}

		/* add the rest of the new attribute values */
		t = atab+i;
		if ((t->fi == fi)) 
		{
		char	*(*func)();
		char	*val;
		struct	atval	*aval = &(eval->atvals[i]);
		unsigned short	flags = eval->atvals[i].flgs;

			*np++ = '\t';
			val = t->atnam;
			while (*val)
				*np++ = *val++;
			*np++ = ' ';
			*np++ = '=';
			*np++ = ' ';

			/* put in new value */
			if (func = xtab[t->xi].put)
			{
				if (t->qwerks & QUOTES)
					np = putquotes(np, aval->val,&flags);
				else
					np = (*(func))(np, aval->val,&flags);
			}
			*np++ = '\n';

			/* reset attribute flags */
			aval->flgs &= ~UPDATED;

			/* this attribute has to be committed */
			aval->flgs |= COMMIT;
		}
	}

	/* copy from end of current data to the next record */
	nxt = nextrec (cp);
	while (cp < nxt)
		*np++ = *cp++;

	/* the new buffer is a string */
	*np = '\0';
	return (0);
}

/*
 * NAME: chgcolon
 *                                                                    
 * FUNCTION: change a record in a colon file
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *                                                                   
 * RETURNS: 0
 *
 */  
int
chgcolon (struct eval *eval, 
	  char *cp, 		/* pointer to current memory buffer */
	  char *np, 		/* pointer to new memory buffer     */
	  int fi,   		/* file table index		    */
	  struct attr *atab, 
	  int nattr)
{
	int	xattr[NFIELDS];		/* fields changed */
	int	i, fields;

	/* initialize the fields changed array */
	for (i = 0; i < NFIELDS; i++)
	{
		xattr [i] = 0;
	}
	/* record the fields that have changed so we can update in order */
	for (i = 1; i < nattr; i++)
	{
		if (!(eval->atvals[i].flgs & UPDATED) || (fi != atab[i].fi))
		{
			continue;
		}
		xattr[(atab + i)->field] = i;
	}
	for (fields = 0; fields < NFIELDS; fields++)
	{
		if (i = xattr[fields])
		{
		char	*(*func)();
		struct	atval	*aval = &(eval->atvals[i]);
		unsigned short	flags = eval->atvals[i].flgs; 

			/* putint, putlist , putstr, putbool etc. */
			if (func = xtab [(atab + i)->xi].put)
				np = (*(func))(np,aval->val,&flags);

			/* end new field and free value */
			eval->atvals[i].flgs &= ~UPDATED;

			/* this attribute has to be committed */
			eval->atvals[i].flgs |= COMMIT;

			/* skip field */
			while ((*cp != ':') && (*cp != '\n'))
				cp++;
			if (*cp == ':')
				*np++ = *cp++;
		}
		else
		{
			COPYFIELD (np, cp);
		}
		if (*(cp - 1) == '\n')
			break;
	}
	if (*(np - 1) != '\n')
		*np++ = '\n';
	*np = '\0';
	return (0);
}

/*
 * Name : getint
 *
 * Function: Changes a character string of numbers into an integer value
 *
 * Returns: target  integer value
 *	    NULL    value was not a character string of numbers
 */
char *
getint (char *val, unsigned short *flgs)
{
	char	*target;
	char	*result;

	while (*val == '0' && *(val+1) != '\0')
		val++;

	target = (char *) strtol (val, &result, 0);
	if (result != val)
	{
		*flgs |= CACHED;
		return (target);
	}
	*flgs |= NOT_VALID;
	return (NULL);
}

/*
 * Name : getstr
 *
 * Function: Malloc storage and copy in parameter "val".  Set flgs to 
 * 	     DYNAMIC and CACHED.
 *
 * Returns: target  pointer to new string
 *	    NULL    malloc failed 
 */
char *
getstr (char *val, unsigned short *flgs)
{
	char	*target;

	/* '+2' in case we need to convert this to a 'list' later on */
	if ((target = malloc (strlen (val) + 2)) != NULL)
	{
		strcpy (target, val);
		*(target + strlen (target) + 1) = '\0';
		*flgs |= CACHED | DYNAMIC;
	}
	else
		*flgs |= NOT_FOUND;

	return (target);
}

/*
 * Name : getquotes 
 *
 * Function: Malloc storage for passed in "val" argument.  Prepend 
 *	     and append quotes to value.
 *
 * Returns:  target  Newly quoted string
 *	     NULL    malloc failed
 */
char *
getquotes(char *val, unsigned short *flgs)
{
	char	*target;
	int	len = 0;
	char	*p = val;

	while (*p)
	{
		while (*p++) 
			len++;
		len++;
	}
	len++;
	if ((target = malloc (len + 4)) != NULL)
	{
		strcpy (target,"\"");
		strcat (target,val);
		strcat (target,"\"");
		*flgs |= CACHED | DYNAMIC;
	}
	else
		*flgs |= NOT_FOUND;
	
	return (target);
}

/*
 * Name : getlist
 *
 * Function: Turn a quoted list into a string of strings.
 *
 * Returns: target  pointer to newly created string of strings.
 *	    NULL    malloc failed
 */
char *
getlist (char *val, unsigned short *flgs)
{
	char	*target;
	int	len = 0;
	char	*p = val;

	/* turn the quoted strings into lists */
	if (*flgs & QUOTES)
	{
		char *ptr = val;

		len = strlen (ptr) + 2;
		while (*ptr)
		{
			if (*ptr == ',')
				*ptr = '\0';
			ptr++;
		}
		ptr++;
		*ptr = '\0';
		
	}
			
	else
	{
		while (*p)
		{
			while (*p++) 
				len++;
			len++;
		}
		len++;
	}
	if ((target = malloc (len)) != NULL)
	{
		memcpy (target, val, len);
		*flgs |= CACHED | DYNAMIC;
	}
	else
		*flgs |= NOT_FOUND;
	
	return (target);
}

/*
 * NAME : getbool
 * 
 * Function: Translates the (val) parameter into either a true (1) or 
 *	     false (0) response.  This function first checks the national
 *	     language equivalent of yes/no with the rpmatch() routine.
 *	     There are also additional English equivalents that are 
 *	     accepted such as "true" or "false".
 *
 * Returns:  1  True
 *	     0  False
 */ 
char *
getbool (char *val, unsigned short *flgs)
{
	struct	bools
	{
		char	*str;
		int	result;
	};
	static	struct bools stab[] = 
	{ 
		{ "never", 	0 },
		{ "no", 	0 },
		{ "false", 	0 },
		{ "always",	1 },
		{ "yes",	1 },
		{ "true",	1 }
	};

	register struct	bools  *s;
	char	 buf[MAXBOOL];
	int	 i;
	int      converted_ok = 0;

	/* 
	 * If the value equals either a Yes or No response then return
	 */
	if ((i = rpmatch(val)) != -1)
		return((char *)i);

	/*
	 * Otherwise we should check for the other English equivalents.
	 */
	if (isalpha((int)*val))
	{
		char	*vp = val;

		for (i = 0; *vp && (i < MAXBOOL-1); i++)
		{
			buf[i] = (isupper ((int)*vp)) ? tolower((int)*vp) : *vp;
			vp++;
		}
		buf[i] = '\0';

		/*
	 	 * loop thru the possible values and return true or false
		 */
		for (i = 0; 
		     i < (sizeof(stab) / sizeof(struct bools));
		     i++)
		{
			if (strcmp(buf, stab[i].str) == 0) {
				converted_ok = 1;
				break;
			}
		}
		*flgs |= CACHED;
		if (converted_ok) {
			return ((char *)stab[i].result);
		}
	}
	/*
	 * If it is unrecognizable as true or false, return the appropriate
	 * value (based on the qwerks value in flgs).
	 *    If flgs contains FAILBADVAL,
	 *		we return INT_MIN (/usr/include/sys/limits.h).
	 *			(see defect 153373, grpck).
	 *    Else if flgs contains FAILTRUE,
	 *		we return TRUE.
	 *    Else
	 *		we return FALSE.
	 */
	if (*flgs & FAILBADVAL)
		return((char *)INT_MIN);
	else if (*flgs & FAILTRUE) 
		return ((char*)1);
	else
		return ((char*)0);
}

/*
 * Name : cpybool 
 *
 * Function: Takes user level input of a boolean value and translates
 *	     it to "true" or "false".  The user can give "rlogin=si"
 *	     in Spanish and the database will contain "rlogin=true".
 * 
 * Returns: val  Boolean string
 */
char *
cpybool(char *val, unsigned short *flgs)
{
	char	*target;

	/* 
	 * malloc storage plus additional in case we need a list
	 */
	if ((target = malloc(sizeof("false") + 2)) != NULL)
	{
		if (getbool(val,flgs))
			strcpy(target, "true");
		else
			strcpy(target, "false");

		*flgs |= CACHED | DYNAMIC;
	}
	else
		*flgs |= NOT_FOUND;

	return (target);
}

/*
 * Name : putstr 
 *
 * Function: Copy parameter "val" to parameter "dest"
 *
 * Returns: dest  points to character element after newly copied string
 */
char *
putstr (register char *dest, register char *val, unsigned short *flags)
{
	while (*val)
		*dest++ = *val++;
	return (dest);
}

/*
 * Name : putbool 
 *
 * Function: Translates a numeric "val" parameter into a true or false  
 *	     expression
 *
 * Returns:  pointer to character element after newly copied string
 */
char *
putbool(char *dest, char *val, unsigned short *flags)
{
	char	*newval = val;

	if ((int) val == 0)
		newval = "false";

	else if ((int) val == 1)
		newval = "true";

	return (putstr (dest,newval,flags));
}

/*
 * Name : putquotes
 *
 * Function: Turns a string of strings into a quoted list.
 *
 * Returns: dest  pointer to next character element after newly copied string
 */
char *
putquotes(register char *dest, register char *val, unsigned short *flags)
{
	char	*ptr;

	ptr = dest;
	*dest++ = '"';
	while (*val)
	{
		while (*val) 
			*dest++ = *val++;
		val++;
		if (*val)
			*dest++ = ',';
	}
	*dest++ = '"';

	return (dest);
}

/*
 * Name : putlist
 *
 * Function: Turns a string of strings into a comma separated list.
 * 
 * Returns: dest  pointer to next character element after newly copied string
 */
char *
putlist (register char *dest, register char *val, unsigned short *flags)
{
	while (*val)
	{
		while (*val) 
			*dest++ = *val++;
		if (*(++val))
			*dest++ = ',';
	}
	return (dest);
}

/*
 * Name : putint
 *
 * Function: copies an integer value into a character string
 *
 * Returns: dest pointer to next character element after newly copied string.
 */
char *
putint (register char *dest, register long *val, unsigned short flags)
{
	dest += sprintf (dest, "%ld", val);
	return (dest);
}

/*
 * Name : rmaudit 
 *
 * Function: Remove users audit entry from the audit/config file.
 *
 * Returns:  0 success
 *   	    -1 error
 */
int 
rmaudit (char *nam, char *buf, int *firstpart, char **remainder)
{
	char	*p = buf;
	char 	*prev = (char *)NULL;
	char	name[BUFSIZ];

	while (isspace ((int)*p)) p++;
	while (*p && strncmp ("users", p, 5))
	{
		p = nextrec (p);
	}
	if (!*p)
		return (-1);

        /* points to after users, in case there is a user "users"
         * D31025
         */
        p += 5;

	while (*p)
	{
		getname (name,p);
		if (!strcmp (nam,name))
			break;
		prev = p;
		p = nextattr (p);
	}
	if (!*p)
		return(-1);

	*firstpart = (p - buf);

	/*
	 * We need to special case the circumstance where we are deleting
	 * the last user in the file.  If we do not we will leave an 
	 * ending tab with no newline, and this will screw the file up.
 	 */
	if (!*(p = nextattr(p)))
	{
		SKIPeol(prev);
		*firstpart = (prev - buf);
	}

	*remainder = p;

	return (0);
}

/*
 * Name : rmrecord
 *
 * Function: Removes user record from stanza file
 * 
 * Returns:  0 success
 *          -1 error
 */
int 
rmrecord (char *nam, char *buf, int *firstpart, char **remainder)
{
	char	*p = buf;
	char	name[BUFSIZ];

	while (isspace ((int)*p)) p++;
	while (*p)
	{
		getname(name,p);
	 	if (!strcmp (name,nam))
			break;
		p = nextrec (p);
	}
	if (!*p)
		return (-1);
	*firstpart = (p - buf);
	p = nextrec (p);
	*remainder = p;

	return (0);
}

/*
 * Name : getname
 *
 * Function: Clears "name" and copies the second argument into the first
 * 	     leaving off any ending special characters.
 *
 * Returns:  ptr   character position after the copy
 */
static char *
getname (char *name, char *ptr)
{
	memset (name,0,BUFSIZ);
	while (*ptr && (*ptr != ' ') && (*ptr != '\0') && (*ptr != '\n') && 
	   (*ptr != '\t') && (*ptr != '=') && (*ptr != ':') && (*ptr != '*')
		&& (*ptr != ','))
		*name++ = *ptr++;
	return (ptr);
}
