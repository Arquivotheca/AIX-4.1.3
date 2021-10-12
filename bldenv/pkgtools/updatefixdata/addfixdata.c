static char sccsid[] = "@(#)31  1.2     src/bldenv/pkgtools/updatefixdata/addfixdata.c, pkgtools, bos412, GOLDA411a 10/4/94 16:43:36";
/*
*   COMPONENT_NAME: PKGTOOLS
*
*   FUNCTIONS: addfixdata
*              matchline
*
*   ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when 20
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
*
* (C) COPYRIGHT International Business Machines Corp. 1994
* All Rights Reserved
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

/* NAME: addfixdata
*
* DESCRIPTION: Filter fixdata stanzas.  
*
* PRECONDITIONS: fixdataDB exists (may be empty, but must be
*		 present).  This is to ensure we update what we
*		 intend to update. The format of fixdata stanza is:
*
*                fix:
*	               name = IX12345
*	               abstract = Apar abstract text
*	               type = f
*	               filesets = "foo.bar.fileset 04.01.0001.0003\n\
*                              foo.bar.second_fileset 04.01.0002.0001\n\
*                              "
*	               symptom = "Some symptom string which can be arbitrarily\n\
*                              short or long or absent.\n\
*                              "
*
* POSTCONDITIONS: output: writes stanzas that do not already exist in the
*		  database to stdout.
*
* INPUT PARAMETERS: stdin is fixdata stanzas
*	            argv[1] is the name of the database file
*
*                   The input is the output of buildfixdata
*	            (a set of fix database stanzas such as:)
*
* DATA STRUCTURES: None
*
* RETURNS:
*/

#include <stdio.h>
#include <sys/param.h>
#define BUFSIZE 1024
#define NAMEEQUAL "name ="

/*------------------------------------------------------------------------
| global variables 
------------------------------------------------------------------------*/
char * commandName = NULL;
char *Usage;

main (int argc, char **argv)
{
    FILE    *infile; /* input file pointer */
    FILE    *datafile; /* pointer to database file */
    char    *datafilename[MAXPATHLEN+1]; /* name of database file name */
    char    *line[BUFSIZE];/* buffer to read in lines from input file */
    char    *buf[BUFSIZE]; 
    char    *ptr; /* temporary pointer */
    char    *apar; /* apar number */

    /*-------------------------------------------------------------------
    | get the command name
    --------------------------------------------------------------------*/
    commandName = getCommandName (argv[0]);

    /*---------------------------------------------------------------------
    | get the database name from the command line 
    ----------------------------------------------------------------------*/
    strcpy (datafilename,argv[1]);
    
    /*--------------------------------------------------------------------
    | open the database file (stream) 
    ----------------------------------------------------------------------*/
    
    datafile = openFile (datafilename, "r");

    /*--------------------------------------------------------------------
    | read stanzas from stdin 
    ---------------------------------------------------------------------*/
    infile = stdin;
    while ((fgets(line,BUFSIZE,infile)) != NULL)
    {
        /*-----------------------------------------------------------------
	| look for pattern 'name =' 
	------------------------------------------------------------------*/
	if (strstr(line,NAMEEQUAL))
	{
	    /*--------------------------------------------------------------
	    | check for matching stanza in fixdatabase 
	    | if no match, print the stanza 
	    ---------------------------------------------------------------*/
	    if ((matchline(datafile,line)) == 1)
	    {
		printf("fix:\n");
		printf("%s",line);
		while ((fgets(line,BUFSIZE,infile)) != NULL)
		{
		    strcpy(buf,line);
		    ptr=strtok(line," \n");
		    if (strstr(ptr,"fix:"))
			break;
		    printf("%s",buf);
		}
	    }
	}
    }
    exit(0);
}

/* NAME: matchline
*
* DESCRIPTION: This functions checks to see if a fixdata stanza exists in
*              the database file for a given apar
*
* PRE CONDITIONS: expects top/HISTORY/database file to exist
*
* POST CONDITIONS: None
*
* PARAMETERS: apar number and database file pointer
*
* DATA STRUCTURES: None
*
* NOTES: The performance will be better if the stanza if found at the top of
*        the fixdata database.
*
* RETURNS: Returns 0 if a fixdata stanza was not found in the database file
*          for an apar else returns 1
*/

int matchline(fp,line)
FILE *fp;
char *line;
{
    char *buf[BUFSIZE];

    rewind(fp);

    while ((fgets(buf,BUFSIZE,fp)) != NULL)
    {
	if (strstr(buf,line))
	    return(0);
    }

    return(1);
}
