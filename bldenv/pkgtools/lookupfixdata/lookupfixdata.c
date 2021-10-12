static char sccsid[] = "@(#)68  1.2     src/bldenv/pkgtools/lookupfixdata/lookupfixdata.c, pkgtools, bos41B, 412_41B_sync 1/12/95 16:01:06";
/*
*   COMPONENT_NAME: PKGTOOLS
*
*   FUNCTIONS: lookupfixdata
*              matchapar
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

/* NAME: lookupfixdata
*
* DESCRIPTION: This function reads apar numbers from the 
*              standard input and writes fixdata stanza to
*              the standard output. It gets the fixdata stanza
*              from the fixdata database file which is in
*              top/HISTORY/fixdataDB file.
*
* PRECONDITIONS:  stdin.. newline separated list of apars
*                 fixdataDB file should exist
*
* POST CONDITIONS: None
*
* INPUT PARAMETERS: argv[1] contains the name of the fixdata database
*
* DATA STRUCTURES: None
*
* RETURNS: Fixdata stanzas for the apars on stdin.
*/

#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#define BUFSIZE 1024
#define APARSIZE 10
#define NAMEEQUAL "name ="

/*---------------------------------------------------------------
| Global variables
----------------------------------------------------------------*/
char *commandName = NULL; /* command Name */
char *Usage;
extern char    *NoStanza;

/*---------------------------------------------------------------
| Warning messages             
---------------------------------------------------------------*/
char    *NoStanza =
"\n\t%s: No stanza was found in the fixdataDB for apar %s.\n\n";

/*----------------------------------------------------------------
| Main.
-----------------------------------------------------------------*/

main (int argc, char **argv)
{
    FILE    *infile; /* pointer to input stream */
    FILE    *datafilefp; /* database file pointer */
    char    datafilename[MAXPATHLEN+1]; /* name of database file */
    char    line[BUFSIZE]; /* buffer to read in lines from  stdin */
    char    buf[BUFSIZE]; /* buffer to read in lines from abstracts file */
    char    *ptr; /* temporary pointer */
    char    apar[128]; /* apar number */
    int rc =0; /* return code */


    /*-----------------------------------------------------------------
    | set the command name
    -------------------------------------------------------------------*/
    commandName = (char*)getCommandName(argv[0]);

    /*------------------------------------------------------------------
    | get the database filename specified by argv[1] 
    -------------------------------------------------------------------*/
    strcpy (datafilename,argv[1]);
    
    /*------------------------------------------------------------------
    | open the database file (stream) 
    -------------------------------------------------------------------*/
    
    datafilefp = openFile(datafilename, "r");

    /*-------------------------------------------------------------------
    | read apars from stdin 
    --------------------------------------------------------------------*/
    infile = stdin;
    while ((fgets(line,BUFSIZE,infile)) != NULL)
    {
	strtok(line," \n");
	strcpy(apar, line);
        /*----------------------------------------------------------------
	| check for matching stanza in fixdatabase 
	-----------------------------------------------------------------*/
	if ((matchapar(datafilefp,line)) == 0)
	{
    	    /*-------------------------------------------------------------
	    | if match exists, print the stanza 
	    ---------------------------------------------------------------*/
	    printf("fix:\n");
	    
	    printf("\t%s %s\n",NAMEEQUAL,line);
	    while ((fgets(line,BUFSIZE,datafilefp)) != NULL)
	    {
		strcpy(buf,line);
		ptr=strtok(line," \n");
		if (strstr(ptr,"fix:"))
		    break;
		printf("%s",buf);
	    }
	printf("\n",buf);
	}
	else
	{
	    rc = 1;
	    warning(NoStanza,commandName, apar);
        }
    }
    exit(rc);
}

/* NAME: matchapar
*
* DESCRIPTION: This function checks to see if a fixdata stanza for
*              an apar exists in the database file or not
*
* PRE CONDITIONS: expects database file to exist
*
* POST CONDITIONS:
*
* PARAMETERS: apar number and database file pointer
*
* DATA STRUCTURES: None
*
* RETURNS: 0 if a fixdata stanza for the apar is found in the 
*          datebase file else return 1
*/

int matchapar(fp,apar)
FILE *fp;
char *apar;
{
    char buf1[256] ;
    char buf2[256] ;
    char line[BUFSIZE];

    /*--------------------------------------------------------------------
    | set the pointer to the beginning of the database file as we
    | want to start the search from the top of the file
    ----------------------------------------------------------------------*/
    rewind(fp);

    /*--------------------------------------------------------------------
    | construct the pattern name = apar#                    
    ----------------------------------------------------------------------*/
    sprintf(buf1, "%s %s", NAMEEQUAL, apar);
    sprintf(buf2, "%s \"%s\"", NAMEEQUAL, apar);
    /*--------------------------------------------------------------------
    | search for name = apar# in the database file. Return 0 if match is
    | found else return 1
    ---------------------------------------------------------------------*/
    while ((fgets(line,BUFSIZE,fp)) != NULL)
    {
	if (strstr(line, buf1))
	{
	    return(0);
	}
	else if (strstr(line, buf2))
	{
	    return(0) ;
	}
    }

    return(1);
}
