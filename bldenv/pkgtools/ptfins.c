static char sccsid[] = "@(#)21  1.17     src/bldenv/pkgtools/ptfins.c, pkgtools, bos41J, 9524E 6/8/95 15:46:16";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: main
 *              addToTable
 *              entryAlreadyInInslist
 *              libcase
 *              usage
 *              printInslist
 *              processLine
 *              invEntry
 *              printLine
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <string.h>
#include <varargs.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "ptfins.h"

#define UPDATE	"UPDATE"
#define INV_U	"inv_u"

/* 
 *---------------------------------------------------
 * External Function Declarations
 * (needed to cleanup compiler warnings).
 *---------------------------------------------------
 */
extern char* getCommandName(char* fullName) ;
extern FILE* openFile(char* fileName, char* mode) ;
extern char* replace_char(char* str, char fromChar, char toChar) ;

/*
 *-------------------------------
 * Global variables.
 *-------------------------------
 */
extern char *  optarg;
int firsttime = TRUE;
char *commandName = NULL;

/*
 *-----------------------------------------
 * Module level variables.
 * The 1st group is for managing the list of
 * entry names used to prevent duplicate 
 * entries being placed in the inslist.  This 
 * is needed for lib names and hard links.
 *-----------------------------------------
 */
static char**	entryTable  = NULL ;	/* ptr to table of lib and hardlink entries */
static int	currEntryLimit = 0 ;	/* Nmbr slots in entryTable	*/
static int	currEntryCnt   = 0 ;	/* Cnt of entries in entryTable	*/

static int	debug        = 0 ;	/* 0 no debug msgs.		*/

/*------------------------------------------------------------
| This is the usage function. All the command line parameters |
| are required.                                               |
-------------------------------------------------------------*/
void
usage()
{
	fprintf(stderr, Usage, commandName);
	exit(-1);
}


/*
 *********************************************************************
 * NAME:   Add Entry to Table
 *                                                                    
 * DESCRIPTION: 
 *      Adds the input entry to the entry table. 
 * PRE CONDITIONS: 
 *      none.
 * POST CONDITIONS: 
 *      Module level data will be modified.
 *      (see DATA STRUCTURES below).
 * PARAMETERS: 
 *      INPUT:
 *	   eName - ptr to string containing name of library or
 *			hard link entry to check.
 *      OUTPUT:
 *         none.
 * NOTES: 
 *      This function manages a table of entries that 
 *	have been added to the inslist.  The table is a 
 *	simple array of pointers.  'currEntryLimit' is the
 *      current size of the table.  'currEntryMax' is the
 *      current number of entries in the table.  The
 *      table will be expanded if more slots are needed.
 * DATA STRUCTURES: 
 *      entryTable - new entry added.  If no space for
 *                  the entry, more memory will be obtained.
 *      currCnt   - will be incremented by 1.
 *      currLimit - may be incremented if more space is
 *      	    needed in the table to hold the input
 *      	    entry.
 * RETURNS: 
 *      nothing.
 *********************************************************************
 */
static void
addToTable(char * eName)
{
    int	  foundIt = 0 ;
    int   i       = 0 ;


    if (debug)
    {
	fprintf(stderr, "%s: DBG= adding '%s' to entryTable;\n",
		commandName, eName) ;
	fprintf(stderr, "%s: DBG=\tcurrCnt = %d\tsize = %d\n",
		commandName, currEntryCnt, currEntryLimit) ;
    }
    if (currEntryLimit == 0)
    {
	currEntryLimit = 16 ;
	entryTable = malloc(sizeof(char *) * currEntryLimit) ;
	if (debug)
	{
	    fprintf(stderr, 
		    "%s:    DBG= got initial space for table; size = %d\n",
		    commandName, currEntryLimit) ;
	    fprintf(stderr, 
		    "%s:    DBG=     address of table = %X\n",
		    commandName, entryTable) ;
	}
    }
    else if (currEntryCnt == currEntryLimit)
    {
	currEntryLimit += 16 ;
	entryTable = realloc(entryTable, 
			       sizeof(char *) * currEntryLimit) ;
	if (debug)
	{
	    fprintf(stderr, 
		    "%s:    DBG= got more space for table; new size = %d\n",
		    commandName, currEntryLimit) ;
	    fprintf(stderr, 
		    "%s:    DBG=     address of table = %X\n",
		    commandName, entryTable) ;
	}
    }

    entryTable[currEntryCnt++] = strdup(eName) ;
} /* END addToTable */


/*
 *********************************************************************
 * NAME:   Entry Already in Inlist
 *                                                                    
 * DESCRIPTION: 
 *      Checks to see if the input entry has already been
 *      added to the ptf's inslist.
 * PRE CONDITIONS: 
 *      none.
 * POST CONDITIONS: 
 *      Module level data may be modified by called functions 
 * PARAMETERS: 
 *      INPUT:
 *	   eName - ptr to string containing name of library
 *		     or hard link entry to check.
 * NOTES: 
 *      A table is kept of each library and hard link entry added 
 *	to the inslist.  If the input entry is not in the table, 
 *	it will be added.
 * DATA STRUCTURES: 
 *      Module level data may be modified by called functions.
 * RETURNS: 
 *      0 = not currently in inslist.
 *	1 = entry is already in the inslist
 *********************************************************************
 */
static int
entryAlreadyInInslist(char * eName)
{
    int	  foundIt = 0 ;
    int   i       = 0 ;


    if (debug)
    {
	fprintf(stderr, "%s: DBG= checking for '%s' in table\n",
		commandName, eName) ;
    }
    while (i < currEntryCnt && !foundIt)
    {
	if (debug)
	{
	    fprintf(stderr, "%s:\tDBG= i = %d, value = '%s'\n",
		    commandName, i, entryTable[i]) ;
	}
	if (strcmp(eName, entryTable[i]) == 0)
	{
	    foundIt = 1 ;
	    if (debug)
	    {
		fprintf(stderr, "%s:DBG= found match!\n",
			commandName) ;
	    }
	}
	i++ ;
    }
    
    if (!foundIt)
    {
	addToTable(eName) ;
    }
    return(foundIt) ;
} /* END entryAlreadyInInslist */


/*---------------------------------------------------------------
| This function writes each line to the output file.            |
| The fourth parameter to this function will always be NULL     |
| except for library updates. In case of a library update       |
| we want to print an apply only entry for the library member.  |
| This entry should have the filename from the filenamesList    |
| file with "inst_updt" in it. For rest all types we print the  |
| object_name from the inslist file.                            |
---------------------------------------------------------------*/
void
printLine(InsEntry *insentry, char typechar, FILE *outFp, char *fileName)
{
	char linktype[3];
	int  i;

	if( isupper(typechar))
		if (insentry->tcbflag == 'Y')
			fprintf(outFp, "%cT %d %d %o	",
					typechar,
					insentry->uid,
					insentry->gid,
					insentry->mode);
		else
			fprintf(outFp, "%c %d %d %o	",
					typechar,
					insentry->uid,
					insentry->gid,
					insentry->mode);
	else /* lower case entry */
		if (insentry->tcbflag == 'Y')
			fprintf(outFp, "%ct %d %d %o	",
					typechar,
					insentry->uid,
					insentry->gid,
					insentry->mode);
		else
			fprintf(outFp, "%c %d %d %o	",
					typechar,
					insentry->uid,
					insentry->gid,
					insentry->mode);
	if( isaLink(insentry->type) )
		fprintf(outFp, "%s %s\n", insentry->object_name,
						insentry->target);
	else
		if (fileName == NULL)
		{
			fprintf(outFp, "%s\n", insentry->object_name);
			if (insentry->numHardLinks>0) 
			    if ( isupper(typechar) )
			      	 if (insentry->tcbflag == 'Y')
				     strcpy(linktype,"HT");
				 else
				     strcpy(linktype,"H");
			    else	
			      	 if (insentry->tcbflag == 'Y')
				     strcpy(linktype,"ht");
				 else
				     strcpy(linktype,"h");
			for ( i=0; i < insentry->numHardLinks; i++ )
				 fprintf(outFp, "%s %d %d %o	%s %s\n",
					linktype,
					insentry->uid,
					insentry->gid,
					insentry->mode,
					insentry->hardLinks[i],
					insentry->object_name);
		}
				
		else
			fprintf(outFp, "%s\n", fileName);
	fflush(outFp);
}

/*-----------------------------------------------------------------------------
| This function checks to see if option.inv_u file exists in the              |
| $TOP/UPDATE/lpp/opt dir. This file contains a set of regular                |
| inslist entries for the files that are not shipped on the media.            |
| All entries in the inv_u are added to the output inslist file if            |
| an entry does not already exist.                                            |
------------------------------------------------------------------------------*/
void
invEntry(char *top, char *option, char *bldcycle, char *insfile)
{
	FILE *inv_ptf; 		/* file pointer to inv_u file*/
	FILE *outfilefp; 	/* file pointer to output file */
	FILE *insfp; 		/* pointer to inslist file */
	InsEntry insentry;	/* struct to store each inslist entry info. */
	int i=0; 		/* loop var */
	int found=0; 		/* to mark if an entry has already been processed*/
	char newFile[BUFSIZE]; 	/* to store the full path name to the inv_u file */
	char outFile[BUFSIZE];	/* stores the name of the output file*/
	char entryLine[BUFSIZE];
	char saveLine[BUFSIZE];
	char entryName[BUFSIZE];
	char *dirname = NULL;
	char *tempname = NULL;
	char newchar = NULL;	/* to store the type char*/
	char *object_name;
	char *target;
	char *fptr = NULL;

	/*-------------------------------------------------------------
	|  generate the output file name.                             |
	--------------------------------------------------------------*/
	sprintf(outFile, "%s.%s", option, "il");
	
	/*---------------------------------------------------------------
	| if output file already exists then open it for append         |
	| else open for write                                           |
	----------------------------------------------------------------*/
	outfilefp = fopen(outFile,"a");
	
	/*---------------------------------------------------------------
	|  generate the full path name to the inv_u file                |
	|  First save the option name for later use as replace char     |
	|  modifies the original string                                 |
	----------------------------------------------------------------*/
	tempname = strdup(option);
	dirname = replace_char(tempname,'.','/');
	sprintf(newFile, "%s/%s/%s/%s.%s.%s", 
			top, UPDATE, dirname, option, INV_U, bldcycle);

	/*--------------------------------------------------------------
	| Open the option.inv_u.bldcycle for read.                     |
	| The file may or may not exist.                               |
	| If the file does not exist then look for fileset.inu_u file  |
	| and do the rest of the processing.                           |
	| If both the files could not be found then return.
	--------------------------------------------------------------*/
	if ( !(inv_ptf = fopen(newFile, "r")) )
	{
	       fptr = strrchr(newFile, '.');
	       *fptr = NULL;
	       if ( !(inv_ptf = fopen(newFile, "r")) )
	    	   return;
        }
	/*---------------------------------------------------------------
	| For each line in option.inv_u file, we check to see if        |
	| this entry is in the filenamesList.  If yes then we have      |
	| already put this into the output file and we return. We       |
	| should not repeat the entries as syschk will give an error    |
	| if it finds an entry twice.                     		|
	---------------------------------------------------------------*/
	while( stripComments(inv_ptf, entryLine) != EOF )
 	{
	        /*-----------------------------------------------------------
		| Save a copy of the entryLine before parsing it with 	    |
		| strtok.						    |
		| Skip over 1st four fields to get to the object_name.      |
		| If this is a hard link, also get the target (6th field).  |
		-----------------------------------------------------------*/
		strcpy(saveLine, entryLine);
		object_name = strtok(entryLine, " \t");
		object_name = strtok(NULL, " \t");
		object_name = strtok(NULL, " \t");
		object_name = strtok(NULL, " \t");
		object_name = strtok(NULL, " \t");
                if ( (saveLine[0] == 'h') || (saveLine[0] == 'H') )
			target = strtok(NULL, " \t");

		/*-----------------------------------------------------------
		| Change the type of this entry if it is F(f), V(v), (B)b,  |
		| or C(c).                                                  |
		-----------------------------------------------------------*/
                newchar=getNewType(saveLine[0]);
		saveLine[0] = newchar;
          
		/*------------------------------------------------------------
		| If the entry has not been processed then write it to the   |
		| output option.il file.                                     |
		| Make sure we do not put multiple entries in the output     | 
		| option.il file. 					     |
		------------------------------------------------------------*/
		if ( (newchar != 'H') && (newchar != 'h') )
		{
			if (!entryAlreadyInInslist(object_name))
		   	   fprintf(outfilefp,"%s\n",saveLine);
		}
		else
		{
		       /*
			*-----------------------------------------------------
			* For hard links, we need to check the target of
			* the link.  If it is already in the table, then
			* all of the hard links will also be in the .il
			* file.   They are placed there by printLine.
			*-----------------------------------------------------
			*/
			strcpy(entryName, target);
			if (!entryAlreadyInInslist(entryName))
			{
		   	   found = 0;
		   	   insfp = openFile (insfile, "r");
		   	   while ( (!found) && ( i = readList(insfp, &insentry, FALSE) != EOF) )
		   	      if (!strcmp(insentry.object_name, target)) 
			   	  found = 1;
		   	   if (found)
			   {
                	      newchar=getNewType(insentry.type);
		   	      printLine(&insentry, newchar, outfilefp, NULL);
			   }
		   	   fclose(insfp);
			}
		}

	}/*while*/
	fclose(outfilefp);
}

/*---------------------------------------------------------------
| NAME: getNewType
|
| DESCRIPTION:
| Changes the type character for an entry from the inv_u file.
| Type F(f) entries from inv_u are changed to type N(n), meaning              
| non_volatile inventory only (non_shipped, fixed size).                     
| Type V(v) B(b) and C(c) are changed to I(i) type, meaning                 
| volatile inventory only (non_shipped, variable size).                    
| Directory D(d) entries are copied as is.                                
| 
| PRE CONDITIONS: none
| 
| POST CONDITIONS: none
| 
| PARAMETERS: type character
| 
| NOTES:
| 
| DATA STRUCTURES: none
| 
| RETURNS: new type character
-----------------------------------------------------------------*/
char
getNewType(char typeChar)
{
	char newC;

	switch (typeChar) 
	{
	case 'D': case 'd':
		newC = 'd';
		break;
	case 'F': case 'f':
		newC = 'n';
		break;
	case 'S': case 's':
		newC = 's';
		break;
	case 'H': case 'h':
		newC = 'h';
		break;
	case 'V': case 'v': case 'B': case 'b': case 'C': case 'c':
		newC = 'i';
		break;
	}
	/*---------------------------------------------------------
	| If the insentry type is upper case then change the new  |
	| char to uppercase.                                      |
	---------------------------------------------------------*/
        if (isupper(typeChar))
		newC = toupper(newC);
	return(newC);
}

/*
 *********************************************************************
 * NAME: printInslist
 *
 * DESCRIPTION:
 *      print an inslist entry to the output inslist file.  If the
 *	entry is for a library update then two entries must be
 *	added, one for the member being updated and one for the
 *	library.  The rules for library updates follow:
 *	1)  If library is VOLATILE then the library entry should
 *		be type I(i), inventory only.
 *	2)  If library is not VOLATILE then the library entry should
 *		be type N(n), non-volatile inventory only.
 *	Otherwise the inslist entry should be copied as it exists in
 *	the original inslist file.
 *
 * PRE CONDITIONS:
 *      finsentry contains the inslist data for the entry being written
 *	to the output file.  flibptr is a flag that will only be set if
 *	this is a library update.
 *
 * POST CONDITIONS:
 *	Appropriate inslist information has been written to the output
 *	inslist file.
 *
 * PARAMETERS:
 *	fileptr - pointer into the output inslist file being written to
 *	flibptr - Non-NULL means this entry is for a library update
 *	finsentry - pointer to inslist entry struct from the "big" inslist
 *	fname - pointer to member name for library update, otherwise NULL
 *
 * RETURNS:
 *      void
 *********************************************************************
 */

void 
printInslist(FILE *fileptr, char *flibptr, InsEntry *finsentry, char *fname)
{
	int firstc1;
	char firstc2;

	/*-------------------------------------------------------------
	| if not library case then print the entry as in inslist file |
	--------------------------------------------------------------*/
	if(flibptr == NULL)
		printLine(finsentry,finsentry->type, fileptr, NULL);
	else /* if special lib case */
	{
		if ( (finsentry->type == 'V') || (finsentry->type == 'v') )
			firstc1 = 'i';
		else
			firstc1 = 'n';
		firstc2 = 'a';
		if (isupper(finsentry->type))
		{
			firstc1 = toupper( firstc1 );
			firstc2 = 'A';
		}
	       /*
		*-------------------------------------------------------- 
		* Only put the library entry in the inslist if
		* it has not already been done.  Duplicate entries
		* get duplicated in the files installp sees and
		* installp gets confused.
		*--------------------------------------------------------
		*/
		if (!entryAlreadyInInslist(flibptr))
		{
		    printLine(finsentry, firstc1, fileptr, NULL);
		}
		/*----------------------------------------------------------
		| print for the library member an apply only entry         |
		-----------------------------------------------------------*/
		printLine(finsentry, firstc2, fileptr, fname);
         } /* else special library case*/
}
            
/*-----------------------------------------------------------------------------
| This function looks for "inst_updt" in the file name. "inst_updt" indicates |
| a library member update.  For these updates the inslist file will contain   |
| an entry for the library itself and not the member so we must search for    |
| the library name and not the member name.                                   |
| An example :                                                                |
| Entry in filenamesList: /usr/lib/inst_updt/liblvm.a/synclp.o                |
| In the above example /usr/lib/liblvm.a is the library name 		      |
| and synclp.o  is a member of the library /usr/lib/liblvm.a .                |
-----------------------------------------------------------------------------*/
char *
libcase(char *fileName)
{
	char buf[BUFSIZE];/* to store the processed string */
	char *p = NULL;/* to track the beginning of "inst_updt" string*/
	char *ptr = NULL; /* pointer to "inst_updt" string in the filename*/
	/* to duplicate the filename since we do not want to change the orig*/
	char *fptr = NULL;
	char *libp = NULL;
	int x = 0;
	int y = 0;

	buf[0] = 0;

	fptr = strdup(fileName);
	/*--------------------------------------------------------
	| get everything before the inst_updt string.            |
	---------------------------------------------------------*/
	if ( (ptr = strstr(fptr, "inst_updt")) != NULL)
	{
	x = strlen(fptr);
	y = strlen(ptr);
	strncpy(buf, fptr, (x-y)); 
	buf[x-y] = NULL;
	}
	
	/*--------------------------------------------------------
	| strip off the member name                              |
	---------------------------------------------------------*/
	p = strrchr(fptr, '/');
	*p = NULL;
	
	/*-------------------------------------------------------
	| append the rest of the string after inst_updt         |
	--------------------------------------------------------*/
	ptr = ptr+(strlen("inst_updt/"));
	strcat(buf, ptr);
	
	/*-------------------------------------------------------
	| return the processed string.                          |
	--------------------------------------------------------*/
	libp = strdup(buf);
	return(libp);
}

/*----------------------------------------------------------------------------- 
| This function processes each line from the filenamesList file.              |
| An Example of the format of a line in filenamesList file is:                |
| /usr/bin/sccs                                                               | 
| For each file name in filenamesList file it generates an inslist entry      |
| in the output file.                                                         | 
-----------------------------------------------------------------------------*/
processLine(char *fname, char *insfile, char *option_name)
{
	FILE *outfp; 		/* pointer to output file */
	FILE *insl_ptr;		/* pointer to lpp_option.il file*/
	InsEntry insentry; 	/* structure to store inslist entries*/
	int i = 0;
	int found ;
	char *libptr = NULL;	/* for library entries*/
	char outputFile[BUFSIZE]; /* to store the output file name*/
	char *p, *ptr, *tmp;
	char compcode; 		/* comparison code */
	char line[BUFSIZE]; 	/* to read each line from filenamesList file*/
	char entryName[BUFSIZE]; /* */
	FILE *inputFilefp;	/* input file pointer*/

 	/*-------------------------------------------------------------
	| open the input filenamesList file for read only             |
	--------------------------------------------------------------*/
 	inputFilefp = openFile(fname, "r");

	/*------------------------------------------------------------
	| For each non comment and non blank line in filenamesList   |
	------------------------------------------------------------*/
 	while( fgets(line, BUFSIZE, inputFilefp) != NULL )
	{
	   /*------------------------------------------------------- 
	   | ignore comments and blank lines                       |
	   -------------------------------------------------------*/
	   if( (line[0] == '#') || (line[0] == '\n') )
		continue;

	   /*-------------------------------------------------------
	   | convert the last nextline char to null                |
	   -------------------------------------------------------*/
	   p = strchr(line,'\n');
	   *p = NULL;

	   /*-------------------------------------------------------
	   | generate the output file name                         |
	   --------------------------------------------------------*/
	   sprintf(outputFile, "%s.%s", option_name, "il");

	   /*-------------------------------------------------------
	   | open the new inslist file for write access            |
	   | but we want to do this only for the first time.       |
	   --------------------------------------------------------*/ 
	   if (firsttime)
	   {
		outfp = openFile(outputFile, "w");
		firsttime = 0;
	   }

	   /*------------------------------------------------------------
	   | check for the following patterns in the file name field.   |
	   | liblpp.a is not in inslist so skip it.                     |
	   ------------------------------------------------------------*/
	
	   if ((strstr(line,"liblpp.a") != NULL)) 
		continue;
	
	   found = 0; 
	   /*------------------------------------------------------------
	   | process the library entries                                |
	   ------------------------------------------------------------*/
	   if ((tmp = strstr(line, "inst_updt")) != NULL)
		libptr = libcase(line);
	   else
		libptr = NULL;
	   /*------------------------------------------------------------
	   | For each line in input inslist file.                       |
	   ------------------------------------------------------------*/
	   if ( (insl_ptr = openFile(insfile, "r")) != NULL)
	   {
		i=readList(insl_ptr, &insentry, FALSE); 
		while ( i != EOF)
		{
			/*--------------------------------------------------
			| If this is a library entry then look for a match |
			| with the returned string from libptr call else   |
			| look for a match with the line as is.            |
			--------------------------------------------------*/
			if (libptr == NULL) 
				compcode = strcmp(insentry.object_name, line);
			else 
				compcode = strcmp(insentry.object_name,libptr);
			if( !compcode) /* if a match was found */
			{
				/*---------------------------------------------
				| We will write all filenamesList entries to  |
				| the entry Table, to prevent any duplicate   |
				| entries in the output inslist.  Duplicate   |
				| inslist entries causes duplicate stanzas in |
				| the inventory file, which in turn causes    |
				| syschk to choke.			      |
				---------------------------------------------*/

				found = 1;

				/*---------------------------------------------
				| If the filenamesList file is a symlink then |
				| include the entry as such in the output     |
				| inslist file. Target file is not included   |
				| at this point.                    	      |
				---------------------------------------------*/
				if ( isaLink(insentry.type))
				{
				    addToTable(insentry.object_name);
				    printLine(&insentry, insentry.type, outfp, NULL);
				    break;
				}
			       /*
				*--------------------------------------------
				* For all other insentry types include them
				* as such.
				* For hard links, concatenate the link name
				* and target name before adding to the entry 
				* table.
				*--------------------------------------------
                                */
				if (insentry.type == 'h' || insentry.type == 'H')
				{
				    strcpy(entryName, insentry.object_name);
				    strcat(entryName, insentry.target);
				    addToTable(entryName);
				}
				else
				{
				    addToTable(insentry.object_name);
				}
				printInslist(outfp, libptr ,&insentry, line);
				i=readList(insl_ptr, &insentry, FALSE);
				break;
			} /* if compcode */
			else 
				i=readList(insl_ptr, &insentry, FALSE); 
		} /* while (i != EOF) */
		/*------------------------------------------------------------
		| If a match for the filenamesList file was not found in the |
		| inslist file then give an error and return non-zero        |
		------------------------------------------------------------*/
		if (! found)
		{
			if (libptr == NULL)
				inserror(noMatch, line);
			else
				inserror(noMatch, libptr);
			return (1);
		}
	   }
	}/* while fgets */
	return (0);
}
/*-----------------------------------------------------------------------------
| Mainbody: ptfins                                                            |
|                                                                             |
| To execute this command:                                                    |
| ptfins -f filenameList -i inslist -o lpp_option                             |
|                                                                             |
| Input -                                                                     |
| filenamesList: File containing the file name.                               |
|                                                                             |
| Output -                                                                    | 
| option_name.il: This file contains an inslist entry for every file in the   |
|                  filenameList file and inv_u file if one exists.            | 
-----------------------------------------------------------------------------*/
main (int argc, char **argv)
{
	int arg;
	char top[BUFSIZE];	/* environment var; sets top of the tree structure*/
	char buildcycle[BUFSIZE];/* environment var for build cycle*/
	char inputfilename[MAXPATHLEN+1];
	char inslistfile[MAXPATHLEN+1];
	char option[MAXPATHLEN+1];
	char *ptr; 		/* pointer to the env variable TOP*/
	char *bldptr; 		/* pointer to the env variable BLDCYCLE*/
	int rc=0;
	
	/*-----------------------------------------------
	| Initialize the variables.                     |
	------------------------------------------------*/
	top[0] = NULL;
	buildcycle[0] = NULL;
	inputfilename[0] = NULL;
	inslistfile[0] = NULL;
	option[0] = NULL;

	commandName = getCommandName( argv[0] );

	/*--------------------------------------------------------------------
	|  parse the command line                                            |
	---------------------------------------------------------------------*/
	while ( (arg = getopt (argc, argv, "f:o:i:")) != EOF )
	{
		switch (arg)
		{
			case 'f':
				strcpy(inputfilename,optarg);
				break;
			case 'o':
				strcpy(option,optarg);
				break;
			case 'i':
				strcpy(inslistfile,optarg);
				break;
			case '?':
				usage();
		}
	} /* while */

	/*------------------------------------------------------------------
	| Verify the command line parameters. All are required.      	    | 
	-------------------------------------------------------------------*/
	if ( !strlen(inputfilename) || (!strlen(option)) || 
					(!strlen(inslistfile)) )
	{
		fprintf(stderr,Missing_Opt, commandName);
		usage();
	}
	
       /*
        *--------------------------------------------
	* Get environment variables.
	* Must get $TOP, others are optional.
	* DEBUG_PTFINS is away to turn on debug
	* messages in the code.
	*--------------------------------------------
        */
	if ((ptr = getenv("DEBUG_PTFINS")) != NULL)
	{
	    debug = 1 ;
	    free(ptr) ;
	}

	if( (bldptr = getenv(BLDCYCLE)) != NULL)
		strcpy(buildcycle, bldptr);

	if( (ptr = getenv(TOP)) != NULL)
		strcpy(top, ptr);
	else
		fatal(No_Env_Var_TOP);

	/*--------------------------------------------------------------------
	| process the filenamesList file entries                             |
	-------------------------------------------------------------------- */
	rc = processLine(inputfilename, inslistfile, option);

	/*-------------------------------------------------------------------
	| process the inv_u file entries                                    |
	--------------------------------------------------------------------*/
	invEntry(top, option, buildcycle, inslistfile);

	exit(rc);
}/*main*/
