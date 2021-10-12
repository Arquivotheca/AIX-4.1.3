static char sccsid[] = "@(#)03  1.12  src/bldenv/pkgtools/common/util.c, pkgtools, bos41J, 9516B_all 4/20/95 10:45:02";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: fatal
 *		getEnvVar
 *		inserror
 *		openFile
 *		ptfstrtok
 *		replace_char
 *		usage
 *		warning
 *		xmalloc
 *		getDirName
 *		stripBlanks
 *		skipLeadingZeros
 *		getCommandName
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <stdio.h>
#include <varargs.h>
#include <string.h>

extern int errno;
extern char *Usage;
extern char *commandName;

char 	*Malloc_Error =
"\tmalloc failed, errno = %d.\n";

char	*Open_Error =
"Could not open file \"%s\" for access '%c'.\n";

/*-----------------------------------------------
| Display usage message and exit.		|
-----------------------------------------------*/

void
usage()
{
	fprintf (stderr, Usage, commandName);
	exit (-1);
}

/*-----------------------------------------------------------------------
| fatal accepts a variable length argument list which is a message	|
| format string and any arguments to the error message.  The arguments	|
| are combined and displayed to stderr and fatal exits with a -1.	|
-----------------------------------------------------------------------*/

void
fatal (va_alist)
{
	va_list ap;
	char *msgFormat;

	fprintf (stderr,"%s:  FATAL ERROR: ",commandName);

	va_start(ap);
	msgFormat = va_arg(ap, char *);
	vfprintf (stderr, msgFormat, ap);
	va_end(ap);

	exit (-1);
}

/*-----------------------------------------------------------------------
| inserror accepts a variable length argument list which is a message	|
| format string and any arguments to the warning message.  The		|
| arguments are combined and displayed to stderr.  This is essentially	|
| the same function as warning but displays the word ERROR.		|
-----------------------------------------------------------------------*/

void
inserror (va_alist)
{
	va_list ap; char *msgFormat; int argno = 0;
	fprintf (stderr,"%s:  ERROR: ",commandName);

	va_start(ap);
	msgFormat = va_arg(ap, char *);
	vfprintf (stderr, msgFormat, ap);
	va_end(ap);
}

/*-----------------------------------------------------------------------
| warning accepts a variable length argument list which is a message	|
| format string and any arguments to the warning message.  The		|
| arguments are combined and displayed to stderr.			|
-----------------------------------------------------------------------*/

void
warning (va_alist)
{
	va_list ap; char *msgFormat; int argno = 0;
	fprintf (stderr,"%s:  WARNING: ",commandName);

	va_start(ap);
	msgFormat = va_arg(ap, char *);
	vfprintf (stderr, msgFormat, ap);
	va_end(ap);
}

/*---------------------------------------
| Malloc with error checking.           |
---------------------------------------*/

char *
xmalloc (int len)
{
	char    *cp;

	if (cp = malloc ((unsigned) len))
	{
		memset ((void *) cp, 0, len);
		return (cp);
	}
	else
		fatal (Malloc_Error, errno);
}

/*-----------------------------------------------
| Open a file for the specified access.		|
-----------------------------------------------*/

FILE *
openFile (char *fileName, char *c)
{
	FILE *fp;
	int mode;

	mode = *c;
	switch (mode) {
	    case 'r':
		fp = fopen (fileName, "r");
		break;
	    case 'w':
		unlink(fileName);
		fp = fopen (fileName, "w");
		break;
	    case '+':
		fp = fopen (fileName, "w+");
		break;
	    case 'a':
		fp = fopen (fileName, "a");
		break;
	}

	if ( !fp )
		fatal (Open_Error, fileName, *c);

	return (fp);
}

/*---------------------------------------------------------------
| Allocate space for an environment variable and return.	|
---------------------------------------------------------------*/
char *
getEnvVar ( char *variable )
{
	char    *ptr;
	char    *value;

	if ( !(ptr = getenv (variable)) )
		return NULL;
	value = xmalloc ( strlen(ptr)+1 );
	strcpy (value, ptr);
	return value;
}

/*-----------------------------------------------------------------------
| This function takes a string and changes every occurence of fromchar	|
| to tochar.								|
-----------------------------------------------------------------------*/
char *
 replace_char(char * str,char fromchar, char tochar)
{
char * p;
     p = strchr(str, fromchar);
    while( p != NULL)
    {
    *p = tochar;
    p = strchr(str,fromchar);
    }
 return(str);
}

/*-----------------------------------------------------------------------
| This version of strtok does not skip leading separators.  That means	|
| that an empty string is returned if a field is empty.  The "real"	|
| strtok would skip over empty fields.					|
-----------------------------------------------------------------------*/
ptfstrtok(char *s1, const char *s2)
{
	char    *p, *q, *r;
	static char     *savept;

	/*first or subsequent call*/
	p = (s1 == NULL)? savept: s1;

	if(p == 0)              /* return if no tokens remaining */
		return(NULL);

/*	q = p + strspn(p, s2);*/  /* skip leading separators */

	if(*p == '\0')          /* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(p, s2)) == NULL)        /* move past token */
		savept = 0;     /* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(p);
}


/*-----------------------------------------------------------------------
| Get the pathname to the fileset directory.  The path is:	        |
|		$TOP/UPDATE/fileset                                     |
| where the fileset may be several levels deep.  For example, if the	|
| fileset name is bos.adt.cls.sccs then the path is:                    |
|     $TOP/UPDATE/bos/adt/cls/sccs.					|
-----------------------------------------------------------------------*/
void
getDirName (char *fileset, char *top, char *filesetPath)
{
	char	*optPtr;

	strcpy (filesetPath, top);
	strcat (filesetPath, "/UPDATE/");
	strcat (filesetPath, fileset);

	/*-------------------------------------------------------
	| Position optPtr at the fileset name itself and then	|
	| change all "." to "/" to get the path name.  If we	|
	| don't then any "." in the path name before the fileset|
	| name will get replaced too.				|
	-------------------------------------------------------*/
	optPtr = strstr (filesetPath, fileset);
	replace_char (optPtr, '.', '/');

}

/*-----------------------------------------------------------------------
| stripBlanks will strip off extra blank characters from the end of a	|
| string.  If there any non-whitespace characters following the blanks,	|
| stripBlanks will return a ptr to the characters and nothing will be	|
| stripped.								|
-----------------------------------------------------------------------*/
char *
stripBlanks (char *buf)
{
	char	*ptr, *nextCh;

	if ( !(ptr = strchr (buf, ' ')) && !(ptr = strchr (buf, '\t')) )
		return ( (char *) NULL );

	nextCh = ptr + 1;
	while ( *nextCh && isspace (*nextCh) )
		nextCh++;

	if ( *nextCh )
		return (nextCh);
	else
	{
		*ptr = NULL;
                fprintf(stderr, "processPtf: WARNING: There are extra spaces in the \n");
                fprintf(stderr, "\tsubsystem field. The blanks will be stripped before \n");
                fprintf(stderr, "\tthe value is calculated.\n");
		return ( (char *) NULL );
	}
}
   

/*-----------------------------------------------------------
| This function skips leading zeros from a string.          |
-----------------------------------------------------------*/
char * 
skipLeadingZeros(char * str)
{
	int i;
	char * ptr = NULL;

	i = strlen(str);
	ptr = str;
	/*---------------------------------------------------
	| need at least one digit. If it 0000 then return 0 |
	| and not a null string.                            |
	---------------------------------------------------*/
	while (i>1) 
	{
		if (*ptr != '0') break;
		ptr++;
		i--;
	}

	return ptr;
}

/*-----------------------------------------------------------------
 * NAME: getCommandName
 *
 * FUNCTION:  This function sets the commandName for error and 
 *            warning messages.
 *
 * EXECUTION ENVIRONMENT:
 *	Normal user process
 *
 * NOTES:  Looks for a "/" and if found then gets the
 *         last occurance of it in the name. Everything after
 *         the last "/" is the commandname.
 *         The calling program is responsible for freeing allocated
 *         space.
 *
 * RECOVERY OPERATION: none
 *
 * PARAMETERS: fullname 
 *
 * RETURNS:  command name
 *
 ----------------------------------------------------------*/

char *getCommandName( char *fullname )
{
    char *ptr;
    char *commandName;

    if ( ptr = strchr(fullname, '/') )
	ptr = strrchr(fullname, '/') + 1;
    else ptr = fullname;
    commandName = xmalloc( strlen( ptr ) + 1 );
    strcpy( commandName, ptr );
    return( commandName );
}
