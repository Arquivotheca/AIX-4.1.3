static char sccsid[] = "@(#)98	1.2  src/bos/usr/bin/lssec/lssec.c, cmdsuser, bos411, 9428A410j 12/10/93 17:06:29";
/*
 * COMPONENT_NAME: (CMDSUSER) security: user commands
 *
 * FUNCTIONS: getattr, main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/access.h>
#include <usersec.h>
#include <userconf.h>
#include <userpw.h>
#include <sys/id.h>
#include <locale.h>
#include "chsec.h"
#include "tcbauth.h"

/* local defines */
char *getattr(struct fileattr *fileptr, char *stanzaname,
	      char *attrval);

/*
 * NAME: lssec
 *
 * FUNCTION: lists attributes in the security configuration files
 *
 * EXECUTION ENVIRONMENT:
 *	command
 *
 * PARAMETERS:
 *
 *	USAGE: lssec [-c] -f file -s stanza -a attr ...
 *	       where:
 *		"file"    : is the name of the config file
 *		"stanza"  : is the name of the stanza in the config file
 *		"attr" 	  : is a valid attribute for the config file
 *
 * PASSED:	argc = the number of attributes entered.
 *		argv[] = the attributes entered.
 *
 * RETURNS: 0 if successful, otherwise 1.
 */

main(int argc,char *argv[])
{
	struct fileattr *fileptr = NULL;
	char *stanzaname = NULL;
	char **attr, **val;
	int cflag = 0, i, nattr = 0;

	/*
	 * NLS gorp.
	 */

	(void ) setlocale(LC_ALL,"");
	catd = catopen ("tcbauth.cat", 0);

	/*
	 * Allocate an array of pointers for the attribute names and the values
	 * of those attributes.
	 */

	attr = (char **)malloc(sizeof(char *) * ((argc >> 1) + 1));
	val = (char **)malloc(sizeof(char *) * ((argc >> 1) + 1));

	if(!attr || !val)
	{
		fprintf(stderr, "%s.\n", MALLOC);
		return(1);
	}

	/*
	 * Parse the command line.
	 */

	while((i = getopt(argc, argv, "cf:s:a:")) != EOF)
	{
		switch(i)
		{
			case 'c':
				cflag = 1;
				break;
			case 'f':
				if(fileptr)
				{
					fprintf(stderr, FILENAME);
					return(1);
				}
				for(fileptr = filetable; fileptr->filename;
				    fileptr++)
					if(!strcmp(fileptr->filename, optarg))
						break;
				if(!fileptr->filename)
				{
					fprintf(stderr, BADFILENAME, optarg);
					return(1);
				}
				break;
			case 's':
				if(stanzaname)
				{
					fprintf(stderr, STANZA);
					return(1);
				}
				stanzaname = optarg;
				break;
			case 'a':
				attr[nattr++] = optarg;
				break;
			default:
				fprintf(stderr, LSSECUSAGE);
				return(1);
		}
	}

	/*
	 * Check for correct usage.
	 */

	if((optind < argc) || !fileptr || !fileptr->filename || !stanzaname ||
	   (nattr == 0))
	{
		fprintf(stderr, LSSECUSAGE);
		return(1);
	}

	/* 
	 * suspend auditing for this process
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/*
	 * Fetch the attributes.
	 */
	for(i = 0; i < nattr; i++)
	{
		if(!(val[i] = getattr(fileptr, stanzaname, attr[i])))
		{
			/*
			 * getattr() will print an error message, so we just
			 * exit.
			 */
			return(1);
		}
	}

	/*
	 * Print out the list of attributes.
	 */
	if(cflag)
	{
		/*
		 * Print in colon format.
		 */
		printf("#name");
		for(i = 0; i < nattr; i++)
			printf(":%s", attr[i]);
		printf("\n%s", stanzaname);
		for(i = 0; i < nattr; i++)
			printf(":%s", val[i]);
		printf("\n");
	}
	else
	{
		/*
		 * Print in normal format.
		 */
		printf("%s");
		for(i = 0; i < nattr; i++)
			printf(" %s=%s", attr[i], val[i]);
		printf("\n");
	}
	return(0);
}

/*
 * NAME: getattr
 *
 * FUNCTION: fetches the value of an attribute
 *
 * EXECUTION ENVIRONMENT: 
 *	user process
 *
 * RETURNS: A pointer to a string containing the value if successful, NULL if
 *	not successful.
 */

char *
getattr(struct fileattr *fileptr, char *stanzaname, char *attr)
{
	struct stanzaattr *ptr;
	char *val, *ret;
	int index;

	/*
	 * Strip leading and trailing whitespace from the attribute name.
	 */

	while(isspace(*attr))
		attr++;
	index = strlen(attr) - 1;
	while((index > 0) && isspace(attr[index]))
		attr[index--] = '\0';

	/*
	 * Find the attribute.
	 */

	for(ptr = attrtable; ptr->attr; ptr++)
	{
		if((fileptr->filenum == ptr->filenum) &&
		   !strcmp(attr, ptr->attr))
		{
			/*
			 * We've found the attribute in the appropriate file;
			 * now we check to see if the stanza name is correct.
			 */

			if(ptr->checkstanza)
			{
				if(!(*(ptr->checkstanza))(stanzaname))
				{
					fprintf(stderr, BADSTANZA, stanzaname);
					return(0);
				}
			}

			/*
			 * If the attribute is a pseudo-attribute, then "fetch"
			 * its value.
			 */

			if(ptr->qwerks & FAKE)
			{
				return((*(ptr->changeval))(stanzaname, val));
			}

			/*
			 * Now, fetch the value of the attribute.  If there
			 * is no value or an error fetching the value, return
			 * an empty string as the value.
			 */

			if(getgenericattr(fileptr->filename, stanzaname, attr,
					  &val, ptr->type))
			{
				return("");
			}

			/*
			 * If there is a routine to massage the database value
			 * into something human-palatable, then call it.
			 */

			if(ptr->changeval)
			{
				ret = (*(ptr->changeval))(stanzaname, val);
				if(!ret)
					ret = "";
			}
			else
			{
				ret = val;
			}

			/*
			 * If this is a value that should be quoted, then quote
			 * it.
			 */

			if(ptr->qwerks & QUOTE)
			{
				char *tmp;

				if(!(tmp = malloc(strlen(ret) + 3)))
				{
					fprintf(stderr, "%s.\n", MALLOC);
					return(0);
				}
				strcpy(tmp, "\"");
				strcat(tmp, ret);
				strcat(tmp, "\"");
				ret = tmp;
			}

			/*
			 * Return the value to the caller.
			 */

			return(ret);
		}
	}

	/*
	 * The attribute is not valid.  Print an error message and return an
	 * error.
	 */

	fprintf(stderr, LISTERR, attr);
	return(0);
}

