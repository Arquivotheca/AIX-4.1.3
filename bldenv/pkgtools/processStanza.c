 static char sccsid[] = "@(#)04  1.1     src/bldenv/pkgtools/processStanza.c, pkgtools, bos412, GOLDA411a 5/31/94 15:22:42";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *		main
 *              usage
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#define STANZA    0

extern char *  optarg;
extern char *  MissingOpt;
extern char *  Usage;
char *  commandName = NULL;

/*----------------------------------------------------------
| Error message functions.                                 
-----------------------------------------------------------*/
char    *MissingOpt =
"\n\t%s:The -c<current_stanza_file> and -p <previous_stanza_file> \n\
        options are required.\n\n";

char	*Usage =
"USAGE:  \n\t%s -c <currentstanzaFile> -p <previous_stanza_file> \n";

/*---------------------------------------------------------------
| NAME: usage
|
| DESCRIPTION:Prints usage statement
| 
| PRE CONDITIONS: None
| 
| POST CONDITIONS: None
| 
| PARAMETERS: None
| 
| NOTES: None
| 
| DATA STRUCTURES: None
| 
| RETURNS: None
-----------------------------------------------------------------*/
void
usage()
{
	fprintf(stderr, Usage, commandName);
	exit (-1);
}

/*---------------------------------------------------------------
| NAME: processStanza
|
| DESCRIPTION: This functions reads the each stanza from the 
|	       input stanza file using odm library functions. 
| 	       It deletes comments and embedded blank lines
|              from the stanza and writes them to a file. Stanzas
|              the output file are seperated by blank lines.
| 
| PRE CONDITIONS: None
| 
| POST CONDITIONS: Two files are generated:
|                  prevstanza.processed
|                  curstanza.processed
| 
| PARAMETERS: -c current stanza file
|             -p previous stanza file (history file)
| 
| NOTES: None
| 
| DATA STRUCTURES: None
| 
| RETURNS:None
-----------------------------------------------------------------*/
main (int argc, char **argv)
{
        FILE *prevstanzafp; /* file pointer to input previous stanza file*/
        FILE *curstanzafp; /* file pointer to input current stanza file*/
        /* pointer to the file that contains processed stanzas from input previous file*/
        FILE *pstanzaFp; 
        /* pointer to the file that contains processed stanzas from input current file*/
        FILE *cstanzaFp;
	char prevstanzaFile[MAXPATHLEN+1]; /* previous file name */
	char curstanzaFile[MAXPATHLEN+1];/* current file name */
        char *stanza = NULL;  /*stores the stanza from the input stanza file*/
        int stanza_length = 0;
	int arg;

	/*------------------------------------------------------
	| Initialize the veriables.                            
	------------------------------------------------------*/

	bzero(prevstanzaFile,MAXPATHLEN+1);
	bzero(curstanzaFile,MAXPATHLEN+1);

	commandName = getCommandName( argv[0] );

       /* parse the command line */
	while ( (arg = getopt (argc, argv, "c:p:")) != EOF )
	{
	    switch (arg)
	    {
		case 'c':
			strcpy(curstanzaFile, optarg);
			break;
		case 'p':
			strcpy(prevstanzaFile, optarg);
			break;
		case '?':
		        usage();
	    }/* switch */
	} /* while */

      /*---------------------------------------------------------------
      | verify the command line paremeters                            
      ----------------------------------------------------------------*/
      if ( !strlen (curstanzaFile) || !strlen(prevstanzaFile) )
           fatal(MissingOpt,commandName);
     
     /*---------------------------------------------------------------
     | open the temp file stanzaFile for write access. This file has
     | the stanzas from the prev and the current files that are   
     | passed to the odmupdate program.                          
     ----------------------------------------------------------------*/
      pstanzaFp = openFile ("prevstanza.processed" , "w");
      cstanzaFp = openFile ("curstanza.processed" , "w");

      /*--------------------------------------------------------------
      | open the previous stanza file for read access         
      ---------------------------------------------------------------*/
      prevstanzafp = openFile(prevstanzaFile, "r");
          
      /*--------------------------------------------------------------
      | open the current stanza file for read access    
      --------------------------------------------------------------*/
      curstanzafp = openFile(curstanzaFile, "r");

     /*----------------------------------------------------------------
     | Read the input previous stanza file and write out the processes
     | stanzas to another file.                                      
     ----------------------------------------------------------------*/
     while ( (stanza_length = get_ascii_phrase(prevstanzafp, STANZA, &stanza) ) > 0 )
        {
          fprintf(pstanzaFp,"%s\n", stanza);
        }
     fclose(pstanzaFp);

     /*----------------------------------------------------------------
     | Read the input current stanza file and write out the processes 
     | stanzas to another file.                                     
     ----------------------------------------------------------------*/
     while ( (stanza_length = get_ascii_phrase(curstanzafp, STANZA, &stanza) ) > 0 )
       {
         fprintf(cstanzaFp,"%s\n", stanza);
       }
     fclose(cstanzaFp);

     fclose(prevstanzafp);
     fclose(curstanzafp);
     exit(0);
}/*main*/
