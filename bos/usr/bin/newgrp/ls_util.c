static char sccsid[] = "@(#)02	1.4.1.2  src/bos/usr/bin/newgrp/ls_util.c, cmdsuser, bos411, 9428A410j 10/7/93 12:12:03";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: getnewstring
 *		printcolon
 *		printstanza
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

#include "tcbauth.h"

struct	pattrs
	{
		char	*name;
		char	*val;
	};

/*
 * FUNCTION:	printstanza
 *
 * DESCRIPTION:	takes a delimiter separated string and
 *		prints it in stanza format.
 *
 * RETURNS:	none.
 *
 */

void
printstanza(char *delim,char *string)
{
char	*ptr;		/* temporary pointer */
char	*attr;		/* temporary pointer */
int	name = 1;	/* flag for the name */

	if(!string)
		return;

	ptr = attr = string;

	while (ptr && *ptr)
	{
		ptr = strchr(ptr, *delim);
		if (ptr)
			*ptr++ = '\0';
		if (name)
		{
			name = 0;
			printf("%s:\n",attr);
		}
		else
			printf("\t%s\n",attr);

		attr = ptr;
	}
	printf("\n");
}
				

/*
 * FUNCTION:	printcolon
 *
 * DESCRIPTION:	takes a delimiter separated string and
 *		prints it in colon format.
 *
 * RETURNS:	none.
 *
 */

void
printcolon(char *delim,char *string)
{
char		*ptr;			/* temporary pointer		    */
char		*attr;			/* temporary pointer		    */
register int	i = 0;			/* counter 			    */
register int	c = 0;			/* counter 			    */
struct	pattrs	attrs[MAXATTRS];	/* array of structures		    */
struct	pattrs	*attrp;			/* pointer to structure		    */

	if(!string)
		return;

	attr = string;
	attrp = attrs;

	if ((ptr = strchr(string, *delim)) == NULL)
		return;

	*ptr++ = '\0';

	attrp->name = "#name";
	attrp->val = attr;

	attrp++;

	attr = ptr;

	while(ptr && *ptr)
	{
		ptr = strchr(ptr, '=');
		if(!ptr)
			break;

		*ptr++ = '\0';

		if(*ptr == *delim)
		{
			ptr++;
			attr = ptr;
			continue;
		}

		attrp->name = attr;
		attrp->val = ptr;
		attrp++;
		i++;

		ptr = strchr(ptr, *delim);
		if(ptr)
		{
			*ptr++ = '\0';
			attr = ptr;
		}
	}

	attrp = attrs;
	for (c=0;c<=i;c++)
	{
		printf ("%s",attrp->name);
		if (c != i)
			printf(":");
		attrp++;
	}
	printf ("\n");

	attrp = attrs;
	for (c=0;c<=i;c++)
	{
		printf ("%s",attrp->val);
		if (c != i)
			printf(":");
		attrp++;
	}
	printf ("\n");
}

/*
 * FUNCTION:	getnewstring
 *
 * DESCRIPTION:	reallocates space and copies in new string.
 *
 * RETURNS:	0 or error.
 *
 */

char *
getnewstring(char *string,char *attr,char *delim,int type,void *val)
{
register int siz;
char	*ret;
char	intbuf[16];

	if (type == SEC_INT)
		siz = strlen(string) +  strlen(attr) + 
		      strlen(delim)  + sprintf(intbuf, "%ld", val) + 2;
	else
		siz = strlen(string) + strlen(attr) + 
		      strlen(delim)  + strlen(val)  + 2;

	if ((ret = (char *)realloc(string,siz)) == NULL)
	{
		fprintf (stderr, MALLOC);
		exitx (errno);
	}

	strcat(ret, attr);
	strcat(ret, "=");
	if (type == SEC_INT)
		strcat(ret, intbuf);
	else 
		if (val)
			strcat(ret, val);
	strcat(ret, delim);

	return ret;
}
