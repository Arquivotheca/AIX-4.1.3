static char sccsid[] = "@(#)70  1.3     src/bldenv/pkgtools/updatefixdata/buildfixdata.c, pkgtools, bos41B, 412_41B_sync 1/12/95 15:59:56";
/*
*   COMPONENT_NAME: PKGTOOLS
*
*   FUNCTIONS:
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

/* NAME: buildfixdata
*
* DESCRIPTION : This function reads apar information from standard input.
*               It then searches the abstracts file for the apar number 
*               and writes fixdata stanza to the standard output.
*
* PRECONDITIONS: input (stdin) from updatefixdata is of the form: 
*		     <@>
*		     apar
*		     fileset vrmf
*		     fileset vrmf
*		     ...
*		     <@>
*		     apar	
*		     fileset ...
*		     ...
*		     
*		     abstract file format is:
*
*		     apar# abstract...
*
*		     symptom info
*		     ...
*		     ...
*		     <@>
*
* POST CONDITIONS: writes fixdata stanzas to stdout.
*          The format of each stanza is:
* fix:
*	name = IX12345
*	abstract = Apar abstract text
*	type = f
*	filesets = "foo.bar.fileset 04.01.0001.0003\n\
*       foo.bar.second_fileset 04.01.0002.0001\n\
*                   "
*	symptom = "Some symptom string which can be arbitrarily\n\
*       short or long.\n\
*                 "
*
* INPUTS PARAMETERS: Gets apar/fileset information on stdin.
*	   argv[1] contains the name of the abstract file.
*	
*
* DATA STRUCTURES: None
*
* RETURNS: None
*/

#include <stdio.h>
#include <sys/param.h>
#include <errno.h>
#define BUFSIZE 1024
#define DELIMITER "<@>"

/*-----------------------------------------------------------------
| Function definitions and global variables
- -----------------------------------------------------------------*/
int  getabstract (FILE *,char *,char **);
char * getsymptom (FILE *);
extern char 	*NoDelimiter;
extern char 	*NoAbstract;
char *commandName = NULL;
char 	*Usage;

/*-----------------------------------------------------------------
| Error messages.        
- -----------------------------------------------------------------*/
char    *NoDelimiter =
    "\n\t%s: No delimiter <@> was found in the input. Bad input format\n\n";

char    *NoAbstract =
    "\n\t%s: No Abstract was found for APAR %s in \n\
%s file. \n\
Processing will continue. \n\n";

/*----------------------------------------------------------------
| Main. 
- -----------------------------------------------------------------*/
main (int argc, char **argv)
{
    char    *c;
    char    *ptr;
    char    *top; /* top of the update tree */
    FILE    *infileFp; /* input file pointer, stdin in this case */
    FILE    *abstractfileFp; /* pointer to top/HISTORY/abstracts file */
    char    *absfilename[MAXPATHLEN+1];/* full path name of abstracts file */
    char    *line[BUFSIZE]; /* buffer to read lines from abstracts file */
    char    *apar; /* apar number */
    char    *abstract; /* apar abstract */
    char    *symptomline; /* apar symptom */
    int     rc;	/* return code */

    /*--------------------------------------------------------------
    | Set the command name
    ---------------------------------------------------------------*/
    commandName = getCommandName (argv[0]);

    /*--------------------------------------------------------------
    | get the name of the abstracts file from the command line 
    | and open for read access.
    ---------------------------------------------------------------*/
    strcpy (absfilename,argv[1]);
    abstractfileFp = openFile (absfilename, "r");
	
    /*---------------------------------------------------------------
    | read stanzas from stdin
    | The format of each stanza is :
    | <@>
    | apar#
    | fileset vrmf
    | fileset vrmf
    | ....
    | <@>
    ----------------------------------------------------------------*/
    infileFp = stdin;
    if (fgets(line,BUFSIZE,infileFp) == NULL)
	exit(0);

    /*---------------------------------------------------------------
    | the first line should be a delimiter <@>
    ----------------------------------------------------------------*/
    if (!strstr(line,DELIMITER))
	 fatal (NoDelimiter, commandName);

    /*---------------------------------------------------------------
    | read lines from standard input till another delimiter is found
    ----------------------------------------------------------------*/
    while ((fgets(line,BUFSIZE,infileFp)) != NULL)
    {
	/*-----------------------------------------------------------
	| get the apar number
	------------------------------------------------------------*/
	apar=strtok(line," \n");

	/*-----------------------------------------------------------
	| Look for the apar abstract in the abstracts file
	------------------------------------------------------------*/
	if ((rc=getabstract(abstractfileFp,apar,&abstract)) != 0)
        {
		warning(NoAbstract, commandName, apar, absfilename);

		/* 
		The apar didn't appear in the abstracts file.
		Consume stdin until you get to the next stanza
		*/

		while ((fgets(line,BUFSIZE,infileFp)) != NULL)
			if (strstr(line,DELIMITER))
				break; 
	}
	else 
	{

		/*-----------------------------------------------------------
		| Create a fixdata stanza for the apar.
		| output the header, the name line, the abstract line, 
		| the type field, and the fileset lines. Look in the 
		| function prologs for the format of each stanza
		------------------------------------------------------------*/
		printf("fix:\n");
		printf("\tname = %s\n",apar);
		printf("\tabstract = %s\n",abstract);
		printf("\ttype = f\n");
		printf("\tfilesets = \"");
		if ((fgets(line,BUFSIZE,infileFp)) != NULL)
		    do
		    {
		        if(*line)
			{
			    strtok(line,"\n");
			    printf("%s\\n\\\n",line);
			}
		        c=fgets(line,BUFSIZE,infileFp);	    
	
		    } while(!(strstr(line,DELIMITER)) && c != NULL);
	
	
	        /*--------------------------------------------------------------
		| end the fileset string 
		---------------------------------------------------------------*/
	        printf ("\"\n");
			
		/*-------------------------------------------------------------
		| Add the symptom information to the stanza for the apar
		--------------------------------------------------------------*/
		printf("\tsymptom = \"");
		while ((symptomline = getsymptom(abstractfileFp)) != NULL)
		{
		    printf("%s\\n\\\n",symptomline);
		}
		printf("\"\n");
	
	}
    }

    fclose (abstractfileFp);
    exit(0);

}

/* NAME: getabstract
*
* DESCRIPTION: This function gets the abstract for 
*              an apar from the abstracts file.
*              The format of abstracts file is:
*              <@>
*               apar# abstract
*               start symptom
*               .......
*               end symptom
*              <@>
*
* PRE CONDITIONS: expects top/HISTORY/abstracts file to exist
*
* POST CONDITIONS: none
*
* PARAMETERS: apar number and abstract file pointer
*
* DATA STRUCTURES: none
*
* RETURNS: abstract of an apar if a match was found else NULL
*/

int getabstract(fp,apar,abstract)
    FILE *fp;
    char *apar;
    char **abstract;
{
    static char   *line[BUFSIZE]; /* buffer to read the apar abstract */
    char   *ptr; /* buffer to read the apar abstract */

    /*------------------------------------------------------------
    | rewind the abstract file as we wnat to search from the 
    | beginning.
    -------------------------------------------------------------*/
    rewind(fp);

    /*------------------------------------------------------------
    | do fgets looking for first line or <@> 
    -------------------------------------------------------------*/
    if ((fgets(line,BUFSIZE,fp)) == NULL)
	return(-1);

    /*-------------------------------------------------------------
    | if the first line doesn't contain the apar number 
    --------------------------------------------------------------*/
    if ((strstr(line,apar)) == NULL)
    {
	/*--------------------------------------------------------
	| read another line
	---------------------------------------------------------*/
	while(fgets(line,BUFSIZE,fp))
	{
	    /*-----------------------------------------------------
	    | if the line contains the delimiter 
	    ------------------------------------------------------*/
	    if (strstr(line,DELIMITER))
	    {
		/*-------------------------------------------------
		| see if the next line contains our apar number 
		--------------------------------------------------*/
		if (fgets(line,BUFSIZE,fp))
	 	{
		    if (strstr(line,apar))
		    {
		        strtok(line," \t");

			/*------------------------------------------
			| the abstract starts at the second token 
			| to the new line
			-------------------------------------------*/
			*abstract=strtok(NULL,"\n");
		        return(0);
		    }
		}

		/*--------------------------------------------------
		| if here, we're at end of file 
		---------------------------------------------------*/
		else
		    return (-1);;
	    }

	}
	/*----------------------------------------------------------
	| if here, we're at end of file 
	-----------------------------------------------------------*/
	return(-1);

    }
    /*---------------------------------------------------------------
    | the first line contained the apar number 
    ----------------------------------------------------------------*/
    else
    {
	strtok(line," ");

	/*-----------------------------------------------------------
	| the abstract starts at the second token to the new line
	------------------------------------------------------------*/
	*abstract=strtok(NULL,"\n");
	return(0);
    }

    /*---------------------------------------------------------------
    | should never get here 
    ----------------------------------------------------------------*/
    return(-1);
}


/* NAME: getsymptom
*
* DESCRIPTION: This function gets the symptom for 
*              an apar from the abstracts file.
*              The format of abstracts file is:
*              <@>
*               apar# abstract
*               start symptom
*               .......
*               end symptom
*              <@>
*
* PRE CONDITIONS: None
*
* POST CONDITIONS: None
*
* PARAMETERS: File pointer to abstracts file
*
* DATA STRUCTURES: None
*
* RETURNS: returns the next non-blank symptom line or NULL if none remain
*/

char * getsymptom(fp)
    FILE * fp;
{
    static    char    *line[BUFSIZE];
    char *buf[BUFSIZE];
    char *ptr;
    /*-------------------------------------------------------------
    | read the next line 
    --------------------------------------------------------------*/
    while(fgets(buf,BUFSIZE,fp))
    {
	strcpy(line,buf);
	ptr=strtok(buf," \t\n");
	/*---------------------------------------------------------
	| if blank, get the next line 
	----------------------------------------------------------*/
	if (strcmp(ptr,""))
	{
	    /*-----------------------------------------------------
	    | if delimiter<@> is found, we're done 
	    -------------------------------------------------------*/
	    if (strstr(ptr,DELIMITER))
	        return(NULL);

            /*-------------------------------------------------------
	    | otherwise, just return the line 
	    --------------------------------------------------------*/
	    strtok(line,"\n");
	    return(line);
	}
	else
	    continue;
    }
    /*----------------------------------------------------------------
    | return NULL if there aren't any more lines 
    -----------------------------------------------------------------*/
    return(NULL);
}
