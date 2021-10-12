static char sccsid[] = "@(#)94	1.2  src/bos/usr/bin/chsec/chsec.c, cmdsuser, bos411, 9428A410j 12/15/93 17:01:38";
/*
 * COMPONENT_NAME: (CMDSUSER) security: user commands
 *
 * FUNCTIONS: chgattr, main
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
static int chgattr(struct fileattr *fileptr, char *stanzaname,
		   char *attrval);

/*
 *
 * NAME: chsec
 *                                                                    
 * FUNCTION: changes attributes in the security configuration files
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *	command
 *
 * PARAMETERS: 
 *
 *	USAGE: chsec -f file -s stanza -a attr=value ...
 *	       where:
 *		"file"    : is the name of the config file
 *		"stanza"  : is the name of the stanza in the config file
 *		"attr"    : is a valid attribute for the config file
 *		"value"   : is the new value for the attribute.
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
	char **attr;
	char *attrs;
	int i, nattr = 0;

	/*
	 * NLS gorp.
	 */

	(void ) setlocale(LC_ALL,"");
	catd = catopen ("tcbauth.cat", 0);

	/*
	 * Allocate an array of pointers for the attribute names.
	 */

	if(!(attr = (char **)malloc(sizeof(char *) * ((argc >> 1) + 1))))
	{
		fprintf(stderr, "%s.\n", MALLOC);
		return(1);
	}

	/*
	 * Parse the command line.
	 */

	while((i = getopt(argc, argv, "f:s:a:")) != EOF)
	{
		switch(i)
		{
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
				if(!strchr(optarg, '='))
				{
					fprintf(stderr, CHSECUSAGE);
					return(1);
				}
				attr[nattr++] = optarg;
				break;
			default:
				fprintf(stderr, CHSECUSAGE);
				return(1);
		}
	}

	/*
	 * Check for correct usage.
	 */

	if((optind < argc) || !fileptr || !fileptr->filename || !stanzaname ||
	   (nattr == 0))
	{
		fprintf(stderr, CHSECUSAGE);
		return(1);
	}

	/* 
	 * suspend auditing for this process
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/*
	 * Change the attributes.
	 */
	for(i = 0; i < nattr; i++)
	{
		/*
		 * Build the audit tail.
		 */

		if(i == 0)
		{
			if(!(attrs = malloc(strlen(attr[0]) + 1)))
			{
				fprintf(stderr, "%s.\n", MALLOC);
				return(1);
			}
			strcpy(attrs, attr[0]);
		}
		else
		{
			if(!(attrs = realloc(attrs, strlen(attrs) +
						    strlen(attr[i]) + 2)))
			{
				fprintf(stderr, "%s.\n", MALLOC);
				return(1);
			}
			strcat(attrs, " ");
			strcat(attrs, attr[i]);
		}
		if(chgattr(fileptr, stanzaname, attr[i]))
		{
			exitax(fileptr->auditname, 1, stanzaname, attrs, 0);
		}
	}
	exitax(fileptr->auditname, 0, stanzaname, attrs, 0);
}

/*
 * NAME: chgattr
 *
 * FUNCTION: changes the value of an attribute
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS: 0 if the change was successful, 1 otherwise.
 */
int
chgattr(struct fileattr *fileptr, char *stanzaname, char *attrval)
{
	struct stanzaattr *ptr;
	char *attr, *val, *ret;

	/*
	 * Split the attribute and value.  Strip leading and trailing
	 * whitespace from the attribute name.
	 */

	while(isspace(*attrval))
		attrval++;
	attr = attrval;
	attrval = val = strchr(attrval, '=');
	*val++ = '\0';
	attrval--;
	while((attrval > attr) && isspace(*attrval))
		*attrval-- = '\0';

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
					return(1);
				}
			}

			/*
			 * See if this is a pseudo-attribute.  If so, then
			 * handle it as such.
			 */
			if(ptr->qwerks & FAKE)
			{
				if((*(ptr->checkval))(stanzaname, val))
				{
					fprintf(stderr, CHGTOERR, attr, val);
					fprintf(stderr, ERBADVAL);
					return(1);
				}
				return(0);
			}

			/*
			 * If the value is null, then delete the attribute
			 * from the stanza.
			 */

			if(!*val)
			{
				if(putgenericattr(fileptr->filename, stanzaname,
						  attr, NULL, SEC_DELETE))
				{
					fprintf(stderr, COMMIT, stanzaname);
					fprintf(stderr, ".\n");
					return(1);
				}
				return(0);
			}

			/*
			 * If there is a routine to validate the value, then
			 * call it.
			 */

			if(ptr->checkval)
			{
				if((*(ptr->checkval))(val, &ret))
				{
					fprintf(stderr, CHGTOERR, attr, val);
					fprintf(stderr, ERBADVAL);
					return(1);
				}
			}
			else
				ret = val;

			/*
			 * If this value should be quoted in the database, then
			 * quote it before we put it.
			 */

			if(ptr->qwerks & QUOTE)
			{
				char *tmp;

				if(!(tmp = malloc(strlen(ret) + 4)))
				{
					fprintf(stderr, "%s.\n", MALLOC);
					return(1);
				}
				strcpy(tmp, "\"");
				strcat(tmp, ret);
				strcat(tmp, "\"");
				tmp[strlen(tmp) + 1] = '\0';
				ret = tmp;
			}

			/*
			 * Write the new value.
			 */

			if(putgenericattr(fileptr->filename, stanzaname, attr,
					  ret, ptr->type))
			{
				fprintf(stderr, COMMIT, stanzaname);
				fprintf(stderr, ".\n");
				return(1);
			}
			return(0);
		}
	}

	/*
	 * The attribute is not valid.  Print an error message and return an
	 * error.
	 */

	fprintf(stderr, CHGTOERR, attr, val);
	fprintf(stderr, ERBADATTR);
	return(1);
}

