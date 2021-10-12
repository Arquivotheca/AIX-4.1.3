static char sccsid[] = "@(#)96  1.17  src/bldenv/pkgtools/readlist.c, pkgtools, bos412, GOLDA411a 6/3/94 13:29:23";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: checkHardLink
 *		cmpObjectNames 
 *		endEntry 
 *   		getEntryInfo
 *		getHardLinkInfo
 *		isaLink
 *		parseLine
 *		readList
 *		stripComments
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ade.h"
#include <stdio.h>

extern int errno;

/*--------------------------------------------------------------------
| Read an entry from the inslist and return data in InsEntry struct. |
| If an error is detected, ADE_ERROR is returned.  Otherwise,        |
| ADE_SUCCESS or EOF is returned.                                    |
--------------------------------------------------------------------*/

int readList(FILE *fd, InsEntry *inslist, int lFlag)
{
	int rc=ADE_SUCCESS;

	inslist->tcbflag = 0;
	inslist->subsystem_name[0] = '\0';
	inslist->numHardLinks = 0;

	if ( feof(fd) )
		return EOF;

	do
	{
		rc |= getEntryInfo (fd, inslist);
		if ( rc != 0)
			break;
	}
	while ( !endEntry(fd) );

	fflush (stderr);
	return rc;
}

/*---------------------------------------------------------------
| Read the next inslist entry line and fill in the InsEntry 	|
| struct.   Each element of tokens contains one field of the	|
| current line of the inslist entry.				|
---------------------------------------------------------------*/

getEntryInfo (FILE *insfp, InsEntry *inslist)
{
	char entryLine[ADE_BUFSIZE], saveLine[ADE_BUFSIZE];
	char *tokens[ADE_TOKENS], c;
        int  rc=ADE_SUCCESS;
	int  numLinks,isLink,numTokens;

	if ( stripComments (insfp, entryLine) == EOF )
		return EOF;

	/*---------------------------------------------------------------
	| Save a copy of line before strtok changes it during parsing.	|
	---------------------------------------------------------------*/

	strcpy (saveLine, entryLine);
	numTokens = parseLine (entryLine, tokens);

	c = tokens[0][0];

	switch(c) {
		case 'F':
		case 'f':
		case 'D':
		case 'd':
		case 'B':
		case 'b':
		case 'C':
		case 'c':
		case 'I':
		case 'i':
		case 'N':
		case 'n':
		case 'A':
		case 'a':
		case 'V':
		case 'v':
		case 'S':
		case 's':
			isLink = isaLink (c);
			if ( (numTokens < 5) && !isLink )
			{
			    fprintf (stderr, "ERROR:  Invalid format on inslist entry line\n");
			    fprintf (stderr, "\t%s\n\n", saveLine);
			    fprintf (stderr, "\tFile entries should have the following format:\n");
			    fprintf (stderr, "\ttype uid gid mode fileName subsystemName\n\n");
			    fprintf (stderr, "\tContinuing to process.\n");
			    return(ADE_ERROR);
			}
			if ( (numTokens < 6) && isLink )
			{
			    fprintf (stderr, "ERROR:  Invalid format on inslist entry line\n");
			    fprintf (stderr, "\t%s\n\n", saveLine);
			    fprintf (stderr, "\tLink entries should have the following format:\n");
			    fprintf (stderr, "\ttype uid gid mode linkName targetFile subsystemName\n\n");
			    fprintf (stderr, "\tContinuing to process.\n");
			    return(ADE_ERROR);
			}
			inslist->type = c;
			if ((tokens[0][1] == 'T') || (tokens[0][1] == 't'))
				inslist->tcbflag = 'Y';
			else {
				if (tokens[0][1] != NULL)
				{
				    fprintf (stderr, "ERROR:  Invalid format on inslist entry line\n");
			    	    fprintf (stderr, "\t%s\n\n", saveLine);
				    fprintf (stderr, "\tThe second character of the inslist type field must be a T\n");
				    fprintf (stderr, "\tor t.  Continuing to process.\n");
				    return(ADE_ERROR);
				}
			}
			inslist->uid = atoi(tokens[1]);
			inslist->gid = atoi(tokens[2]);

			sscanf(tokens[3],"%o", &inslist->mode);

			strcpy(inslist->object_name, tokens[4]);

			/*-----------------------------------------------
			| Link entries have an extra target field	|
			| before the subsystem name.			|
			-----------------------------------------------*/
			if ( isLink )
			{
				strcpy ( inslist->target, tokens[5] );
				strcpy(inslist->subsystem_name, tokens[6]);
			}
			else
				strcpy(inslist->subsystem_name, tokens[5]);
			break;
		case 'H':
		case 'h':
			if ( numTokens < 6 ) 
			{
			    fprintf (stderr, "ERROR:  Invalid format on inslist entry line\n");
			    fprintf (stderr, "\t%s\n\n", saveLine);
			    fprintf (stderr, "\tLink entries should have the following format:\n");
			    fprintf (stderr, "\ttype uid gid mode linkName targetFile subsystemName\n\n");
			    fprintf (stderr, "\tContinuing to process.\n");
			    return(ADE_ERROR);
			}

			if ( cmpObjectNames(tokens[5],inslist->object_name) )
				return(ADE_ERROR);
				
			numLinks = inslist->numHardLinks;
			if ( numLinks < ADE_MAXLINKS )
			{
				if (inslist->hardLinks[numLinks])
				    free(inslist->hardLinks[numLinks],strlen(inslist->hardLinks[numLinks]));
				inslist->hardLinks[numLinks] = malloc (strlen(tokens[4])+1);
				strcpy(inslist->hardLinks[numLinks],tokens[4]);
				inslist->numHardLinks++;
			}
			else
			{
  				fprintf (stderr, "ERROR:  The number of hard links for entry %s\n",inslist->object_name);
				fprintf (stderr,"\thas exceeded the maximum of %d\n",ADE_MAXLINKS);
				rc=ADE_ERROR;
				break;
			}
			break;
  		default:
			fprintf (stderr, "ERROR:  Unrecognized file type %c in string %s\n\n", c, saveLine);
			fprintf (stderr, "\tContinuing to process.\n");
			return(ADE_ERROR);
	}
	return rc;
}
	

/*---------------------------------------------------------------
| Compare 2 object names, display error message if not a match. |
---------------------------------------------------------------*/

cmpObjectNames (char *name1, char *name2)
{
	if ( strcmp (name1, name2) )
	{
		fprintf (stderr, "ERROR: object name %s on link line and inslist\n",name1);
		fprintf (stderr, "\tentry name %s do not match.\n",name2);
		fprintf (stderr, "\tHard link entries must immediately follow their\n");
		fprintf (stderr, "\tcorresponding target file entry.\n");
		fprintf (stderr, "\tThe link entry will be ignored.\n\n");
		return 1;
	}
	else
		return 0;
}

/*---------------------------------------------------------------
| If next line contains a valid object identifier then it	|
| begins a new inslist entry.					|
---------------------------------------------------------------*/

int
endEntry ( FILE *fd )
{
	int c;
	char line[ADE_BUFSIZE+1];

	c = getc (fd);

	while ( (c == '#') || (c == '\n') ) 
	{
		if ( c == '#' )
			fgets(line,ADE_BUFSIZE,fd);
		c = getc (fd);
	}

	if ( feof(fd) )
		return 1;

	switch (c) {
		case 'F':
		case 'f':
		case 'D':
		case 'd':
		case 'B':
		case 'b':
		case 'C':
		case 'c':
		case 'I':
		case 'i':
		case 'N':
		case 'n':
		case 'A':
		case 'a':
		case 'V':
		case 'v':
		case 'S':
		case 's':
			ungetc (c,fd);
			return 1;
		case 'H':
		case 'h':
			ungetc (c,fd);
			return 0;
		default:
			ungetc (c,fd);
			return(-1);
	}
}

/*---------------------------------------------------------------
| Read a line from the inslist and put each field in tokens.	|
| Fields are delimited by spaces.  Return the number of tokens. |
---------------------------------------------------------------*/

parseLine (char *line, char **tokens)
{
	int i=0;

	/* pick out tokens separated by whitespace */

	tokens[i++] = strtok(line, " \t");

	while (((tokens[i] = strtok(NULL, " \t")) != (char *) NULL))
        {
          i++; 
          if ( i == ADE_TOKENS )
          {
	    fprintf (stderr, "WARNING:\n\n\tThe number of fields in the following line of the\n");
	    fprintf (stderr, "\tinslist file has exceeded the maximum number of %d fields:\n", ADE_TOKENS);
	    fprintf (stderr, "\t%s\n");
	    break;
          }
        }
	return i;
}


/*---------------------------------------
| Strip comment lines as we go.         |
---------------------------------------*/
int
stripComments (FILE *insfp, char *line)
{
        char *num;
        char *ptr;

        while   ( (num = fgets(line,ADE_BUFSIZE,insfp)) != NULL )
        {
                /*-------------------------------------------------------
                | Position line[i] at first non-whitespace character.   |
                -------------------------------------------------------*/
                for ( ptr=line; isspace (*ptr); ptr++ )
                {
                        if ( (*ptr == '\n') )
                            break;
                }
                if ( (*ptr != '#') && (*ptr != '\n') )
                        break;
        }

        if (num == (char *) NULL)
                return (EOF);

        if ( num = strchr (line, '\n') )
                *num = (char *) NULL;
        return 0;
}


/*-----------------------------------------------------------------------
| Return true if type is S or s.  These types of entries are links.	|
| Types H and h are not included here since they are handled separately.|
-----------------------------------------------------------------------*/
isaLink (char type)
{
	if ( (type == 's') || (type == 'S') )
			return (1);
	else
		return (0);
}


