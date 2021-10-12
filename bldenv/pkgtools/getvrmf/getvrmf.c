static char sccsid[] = "@(#)42  1.9     src/bldenv/pkgtools/getvrmf/getvrmf.c, pkgtools, bos412, 9443A412a 10/21/94 14:03:53";
 /*   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *		generateVRMF
 *		stripComments
 *		bumplevel
 *		incrstring
 *		main
 *		usage
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

#define  INTERNALTABLE		"/HISTORY/internalvrmfTable"
#define  PTFOPTFILE		"/HISTORY/ptfoptions"
#define  PTFREQSLIST		"/HISTORY/ptfreqsFile"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include "ptf.h"

extern char * optarg;
char *commandName = NULL;
int 	debug = 0 ;	     /* Flag to turn on print statements. */

/*--------------------------------------------------------------
| This is the usage function. All the command line parameters  |
| are optional.                                                |
--------------------------------------------------------------*/
void
usage()
{
  fprintf(stderr, "USAGE: %s -o [outputfile]\n", commandName);
  fprintf(stderr, "Where outputfile name is optional\n");
  exit (-1);
}


main (int argc, char **argv)
{
	FILE 	*lppListFp; /* file pointer to lpplist.BLDCYCLE file*/
	FILE 	*ptfpkgFp; /* file pointer to ptf_pkg.BLDCYCLE file.*/
	FILE 	*ptfoptFp; /* file pointer to ptfoptions file.*/
	FILE 	*internalTableFp;/* file pointer to internal vrmf Table.*/
	/* file pointer to ptf requisites file. This is a temp file*/
	FILE 	*ptfreqslistFp;
	char    lppListFile[MAXPATHLEN+1];/* lpplist file name*/
	char    ptfpkgFile[MAXPATHLEN+1];/*ptf_pkg file name*/
	char    ptfoptFile[MAXPATHLEN+1];/*ptfoptions file name*/
	char    ptfreqslistFile[MAXPATHLEN+1];/*requisites file name*/
	char    internalTableFile[MAXPATHLEN+1];/* internal vrmf table file*/
	char    specialPtfType[MAXPATHLEN+1];/* flag to mark special ptf type*/
	/* variable to store the file name from the third field in ptf_pkg file*/
	char    fileName[MAXPATHLEN+1];
	/* variable to read each lppname from lpplist file*/
	char    lppName[BUFSIZE];
	char    buf[BUFSIZE];/* buffer to read in each line from ptf_pkg file*/
	char    ptfbuf[BUFSIZE];/* buffer to read in each line from ptfoptions file*/
	/* variable to read in the fileset:vrmf field from the ptf_pkg
	file. This is the fourth field in each line and may or may not have
	":vrmf" in it. */
	char    filesetvrmf[BUFSIZE];
	char    *fileset;
	char 	ptfName[PTFLEN];
	char 	oldptfName[PTFLEN];/* to store teh previous ptf number*/
	char 	*buildCycle;/* variable to read the environment var BLDCYCLE*/
	char 	*top;/* variable to read the environment var TOP*/
	char 	*ptr; 
	char	*apar ;
	vrmftype   ptfvrmf; /* to store the vrmf entry into each of its components*/
	vrmftype   newvrmfentry; /* to store the vrmf entry into each of its components*/
	char    * vrmfNum; /* to store the vrmf number as a string.*/
	char    ptfreqslist[BUFSIZE];/* to store requisites for ptf*/
	char    aparslist[BUFSIZE];/* to store apars for ptf*/
	char    *ptfnumber; /* to store ptf's number from ptfoptions file */
	char    *ptfopt; /* to store ptf's fileset from ptfoptions file */
	char    *ptfoptvrmf; /* to store ptf's vrmf from ptfoptions file */
	int     match;
	int     arg;

	/*-----------------------------------------------------------
	| initialize all the variables.                             |
	------------------------------------------------------------*/
	specialPtfType[0] = NULL;
	filesetvrmf[0] = NULL;
	fileName[0] = NULL;
	lppName[0] = NULL;
	buf[0] = NULL;
	ptfbuf[0] = NULL;
	ptfName[0] = NULL;
	oldptfName[0] = NULL;
	lppListFile[0] = NULL;
	ptfpkgFile[0] = NULL;
	internalTableFile[0] = NULL;
	ptfreqslistFile[0] = NULL;
	ptfreqslist[0] = NULL;
	aparslist[0] = NULL;

	commandName = (char*)getCommandName( argv[0] );

	if ((ptr = (char*)getEnvVar("VRMF_DEBUG")) != (char*)NULL)
	{
	    debug = 1 ;
	    free(ptr) ;
	}

	/*-----------------------------------------------------
	| Get the command line parameters.                    |
	-----------------------------------------------------*/
	while (( arg = getopt ( argc, argv, "o:")) != EOF)
	{
	    switch (arg)
	    {
		case 'o':
			strcpy(internalTableFile, optarg);
			break;
                case '?':
			usage();
			break;
             }
         }

	/*-----------------------------------------------------
	| get the environment variable bldcycle and top. Give |
	| a fatal error only if TOP is  not set.              |
	-----------------------------------------------------*/
	buildCycle = (char*)getEnvVar ("BLDCYCLE");
	if ( !(top = (char*)getEnvVar ("TOP")) )
		fatal (SelfixTopNotSet);

	/*---------------------------------------------------------
	| open the internaltable for write access. We generate    |
	|  a new internal vrmf table for each build cycle.        |
	---------------------------------------------------------*/
	if (strlen(internalTableFile))
	    internalTableFp = openFile(internalTableFile, "w");
	else
	{
	    sprintf(internalTableFile, "%s/%s.%s", top, INTERNALTABLE, buildCycle);
	    internalTableFp = openFile(internalTableFile, "w");
	}

	if (debug)
	{
	    fprintf(stderr, "dbg: Opened internal table at \"%s\"\n", 
		    internalTableFile) ;
	}
	/*---------------------------------------------------------
	| open the ptfreqslistFile for write access. We generate  |
	|  a new requisites file for each build cycle.            |
	| This file will be used by ptfpkg command later. If a    |
	| ptf could not be packaged successfully then ptf_pkg     |
	| identifies the ptfs that have the failed ptf as a       |
	| requisite and gives a warning message with ptf numbers. |
	| All these ptf will have to be repackaged.               |
	| The format of this file is:                             |
	| PTF# IFREQS|COREQS|PREREQS|                             |
	---------------------------------------------------------*/
	sprintf(ptfreqslistFile, "%s/%s.%s", top, PTFREQSLIST, buildCycle);
	ptfreqslistFp = openFile(ptfreqslistFile, "w");
	if (debug)
	{
	    fprintf(stderr, "dbg: Opened requisite list file at \"%s\"\n", 
		    ptfreqslistFile) ;
	}

	/*--------------------------------------------------------
	| Generate the full path name for lpplist.bldcycle file.|
	| lpplist.bldcycle file has all the lpp names that      |
	| are being updated in this build cycle.                |
	| Open lpplist.bldcycle file for read access.           |
	--------------------------------------------------------*/
	sprintf(lppListFile, "%s/%s/%s.%s", top, "UPDATE", 
			"lpplist", buildCycle);
	lppListFp = openFile (lppListFile, "r");
	if (debug)
	{
	    fprintf(stderr, "dbg: Using lpplist file at \"%s\"\n", 
		    lppListFile) ;
	}


	/*--------------------------------------------------------
	| For each lpp in the lpplist.bldcycle file              |
	--------------------------------------------------------*/
	while (stripComments (lppListFp, lppName) != EOF)
	{
	    if (debug)
	    {	
		fprintf(stderr, "dbg: working with LPP '%s'\n",
			lppName) ;
	    }
	    /*----------------------------------------------------------
       	    | generate the ptf_pkg file name.                          |
	    | open the ptf_pkg.buildcycle file for read access.        |
 	    ----------------------------------------------------------*/
	    sprintf(ptfpkgFile, "%s/%s/%s/%s.%s", top, "UPDATE",
	    			lppName, "ptf_pkg", buildCycle);
	    ptfpkgFp = openFile (ptfpkgFile, "r");
	    if (debug)
	    {
		fprintf(stderr, "dbg: working on ptf_pkg file \"%s\"\n", 
			ptfpkgFile) ;
	    }


      	    /*--------------------------------------------------------------
	    | The format of each line in ptf_pkg.bldcycle file is:         |
	    | PTF#|APARS|FILENAME|OPTION.SUBSYS:VRMF|IFREQS|COREQS|PREREQS |
	    | For each line in ptf_pkg.BLDCYCLE file                       |
       	    ---------------------------------------------------------------*/
 	    while (stripComments (ptfpkgFp, buf) != EOF)
	    {
		if (debug)
		{
		    fprintf(stderr,
			    "dbg: processing ptf_pkg record:\n\"%s\"\n", 
			    buf) ;
		}
   	        /*------------------------------------------------------
		| the first field is the ptf number                    |
		| Check to see if this ptf has been processed brfore   |
		| for vrmf. If yes then read the next line as we have  |
		| already calculated a vrmf number for this ptf.       |
		| If no then get a new vrmf number.                    |
		| The ptf_pkg.buildcycle file has been sorted by ptf   |
		| numbers prior to calling this program.               |
		-------------------------------------------------------*/
		ptr = strtok (buf, "|");
		strcpy (ptfName, ptr);
		if (debug)
		{
		    fprintf(stderr,"dbg: extracted PTF ID : \"%s\"\n", 
			    ptfName) ;
		}

		if (strcmp( ptfName, oldptfName) != 0 )
		{
		   /* 
		    *--------------------------------------------------
		    * Need to tack the aparslist on the previous line
		    * when a new PTF number is discovered.  The
		    * strlen check handle the case of the 1st 
		    * record (no newline should be genereated then).
		    *--------------------------------------------------
		    */ 
		    if (strlen(oldptfName) > 0)
		    {
			fprintf(internalTableFp, " %s\n", aparslist) ;
			if (debug)
			{
			    fprintf(stderr,
			       "dbg: Detected new ptf, wrote aparslist (%s) for '%s'\n",
				    aparslist, oldptfName) ;
		        }
		    }
		    strcpy(oldptfName, ptfName);
		}
		else
		{
		   /*
		    *-------------------------------------------
		    * Need to get the APARs field from this
		    * record because it could have different
		    * APAR numbers.
		    * 
		    * Due to problems with the list of APARs
		    * getting MUCH too large, we need to
		    * make sure we don't duplicate APAR
		    * numbers in the aparslist.
		    * 
		    * Then the rest of the processing for
		    * the record can be skipped.
		    *-------------------------------------------
		    */
		    apar = strtok (NULL, "|");
		    while (strlen(apar) != 0)
		    {
		       /* 
			*---------------------------------------------
			* Find the end of this APAR (a blank) and
			* turn it into a "standalone" string.  
			* Set a temp pointer to the beginning of
			* the next APAR so we know how to get to
			* it later.
			*---------------------------------------------
			*/
			if ((ptr = strchr(apar, ' ')) != (char*)NULL)
			{
			    ptr[0] = '\0' ;
			    ptr++ ;
			    if (debug)
			    {
				fprintf(stderr, 
				 "dbg: Found apar '%s', rest of list = '%s'\n",
					apar, ptr) ;
			    }
			}
		       /* 
			*----------------------------------------------
			* Check to see if this APAR is already in
			* the list.  If it is not, add it if there
			* is room in the buffer for it.
			*----------------------------------------------
			*/
			if (strstr(aparslist, apar) == (char*)NULL) 
			{	
			    if (strlen(aparslist) + strlen(apar) + 1 < BUFSIZE)
			    {
				if (debug)
				{
				    fprintf(stderr,
					    "dbg: adding apar '%s' to list\n",
					    apar) ;
				}
				sprintf(aparslist, "%s %s", aparslist, apar);
			    }
			    else	/* No room in buffer, log the error */
			    {
				fprintf(stderr, 
			    "getvrmf: ERROR, aparslist overflow!  Size > %d\n",
					BUFSIZE) ;
				fprintf(stderr, 
					"         ERROR, list = %s\n",
					aparslist) ;
				fprintf(stderr, 
					"         ERROR, need to add '%s'\n",
					apar) ;
			    }
			}
			apar = ptr ;	/* Move to next APAR in list. */
		    }
		    continue;
		}

		/*-------------------------------------------
		| Get the apars field.
		-------------------------------------------*/
		ptr = strtok (NULL, "|");
		strcpy(aparslist,ptr);
		if (debug)
		{
		    fprintf(stderr, 
			    "dbg: Got following APARs from ptf_pkg; %s\n",
			    aparslist) ;
		}

		/*-----------------------------------------------------------
		| Set flag if special case cum_ptf. Else copy the filename  |
		-----------------------------------------------------------*/
		ptr = strtok (NULL, "|");
		if ( strstr(ptr, "cum_ptf") )
			strcpy (specialPtfType, ptr);
		else
			strcpy (fileName, ptr);
		if (debug)
		{
		    fprintf(stderr, 
		    "dbg: Got following from filename field of  ptf_pkg\n%s\n",
			    ptr) ;
		}

		/*----------------------------------------------------------
		| get the fileset:vrmf . It is the 4th field in the line.   
		| The 4th field will have :VRMF only if we want to override 
		| the default mechanism for calculating the new vrmf number.
		------------------------------------------------------------*/
		ptr = strtok (NULL, "|");
		strcpy(filesetvrmf, ptr);
		if (debug)
		{
		    fprintf(stderr, "dbg: Working PTF for '%s'\n", 
			    filesetvrmf) ;
		}


		ptr = strtok (NULL, "\n");
		strcpy(ptfreqslist, ptr);
		fprintf(ptfreqslistFp, "%s %s\n", ptfName, ptfreqslist);
		if (debug)
		{
		    fprintf(stderr, "dbg: Detected following reqs '%s'\n", 
			    ptfreqslist) ;
		}

		/*----------------------------------------------------------
		| if there are multiple fileset names then                  |
		| exit fatally as we do not want to proceed for            |
		| special type ptfs that have multiple fileset names.       |
 		----------------------------------------------------------*/ 
		if ( stripBlanks (filesetvrmf) ) 
			fatal (MultipleFilesetsForFile, fileName, ptfpkgFile);

		vrmfNum = NULL;
		/*----------------------------------------------------------
		| If vrmf number was given in the fileset field then        |
		|  use that vrmf number for internal table. Also break     |
		|  the field into fileset and vrmf number.                  |
		-----------------------------------------------------------*/
		if ((ptr = strchr(filesetvrmf, ':')) != (char*)NULL)
		{
			fileset = strtok(filesetvrmf, ":");
			vrmfNum = strtok(NULL, ":");
			if (debug)
			{
			    fprintf(stderr, 
				    "dbg: Override VRMF detected; '%s'\n", 
				    vrmfNum) ;
			}
		}
		else
			fileset = filesetvrmf;

		if (debug)
		{
		    fprintf(stderr, "dbg: Using fileset name of '%s'\n",
			    fileset) ;
		}

		/*------------------------------------------------------------------
		| If the new vrmf number was not specified in the ptf_pkg.bldcycle |
		| file then check to see if an entry for this ptf exists in        |
		| ptfoptions file. If yes then we are repackaging this ptf and     |
		| use that vrmf number for internal table.                         |
		------------------------------------------------------------------*/
		if (vrmfNum == (char*)NULL)
		{
		    if (debug)
		    {
			fprintf(stderr, "dbg: contents of vrmfNum = 0x%x\n",
				(long)vrmfNum) ;
		    }
			sprintf(ptfoptFile, "%s/%s", top, PTFOPTFILE);
                	ptfoptFp = openFile( ptfoptFile, "r");
			if (debug)
			{
			    fprintf(stderr, 
				    "dbg: Using ptfoptions file at '%s'\n",
				    ptfoptFile) ;
			}
			while (stripComments(ptfoptFp, ptfbuf) != EOF)
			{
				ptfnumber = strtok(ptfbuf, " ");
				ptfopt = strtok(NULL, " ");
				ptfoptvrmf = strtok(NULL, " ");
				if (! strcmp(ptfnumber, ptfName))
				{
					match = 1;
					if (debug)
					{
					    fprintf(stderr, 
 "dbg: Found PTF '%s' in ptfoptions file for fileset '%s' with vrmf '%s'\n",
						    ptfnumber, 
						    ptfopt, 
						    ptfoptvrmf) ;
					}
					break;
				}
				else
					match = 0;
			}
			if (match && (ptfoptvrmf != (char*)NULL))
			{
			    vrmfNum = ptfoptvrmf;
			}
		}

		/*----------------------------------------------------------
		| If the vrmf number was given in the ptf_pkg.bldcycle     |
		| file then use that to write the entry in internalTable.  |
		| The Format of each line in internalvrmfTable is :        |
		| ptf# opt1 opt2 ... version.release.mod.fix               |
		-----------------------------------------------------------*/
		if ( vrmfNum != (char*)NULL )
		{
		    if (debug)
		    {
			fprintf(stderr, "dbg: Got an override vrmf of '%s'\n",
				vrmfNum) ;
			fprintf(stderr, "dbg: contents of vrmfNum = 0x%x\n",
				(long)vrmfNum) ;
		    }
			splitVRMF(fileset, vrmfNum, &newvrmfentry);
			fprintf(internalTableFp, "%s %s %s.%s.%s.%s", 
				ptfName,
				fileset,
				skipLeadingZeros(newvrmfentry.version),
				skipLeadingZeros(newvrmfentry.release),
				skipLeadingZeros(newvrmfentry.mod),
				skipLeadingZeros(newvrmfentry.fix)) ;
			if (debug)
			{
			    fprintf(stderr, 
"dbg: wrote override vrmf entry to internal table for '%s', %s %s %s %s\n",
				    ptfName, 
				    skipLeadingZeros(newvrmfentry.version),
				    skipLeadingZeros(newvrmfentry.release),
				    skipLeadingZeros(newvrmfentry.mod),
				    skipLeadingZeros(newvrmfentry.fix)) ;
			}
		}
		else
                {
		/*---------------------------------------------------------
		| if the vrmf number was not given in the fileset          |
		| field then calculate the new vrmf number for the ptf.   |
		| The default mechamism to calculate the vrmf is:         |
		| 1. bump the fix level by one if it is not a cum ptf.    |
		| 2. bump the mod level and reset the fix level for a     |
		|    cum_ptf.                                             |
		----------------------------------------------------------*/

		generateVRMF(fileset, buildCycle, 
				&ptfvrmf, top, specialPtfType);

		fprintf(internalTableFp, "%s %s %s.%s.%s.%s", 
			ptfName, 
			fileset, 
			skipLeadingZeros(ptfvrmf.version),
			skipLeadingZeros(ptfvrmf.release),
			skipLeadingZeros(ptfvrmf.mod),
			skipLeadingZeros(ptfvrmf.fix)) ;
			if (debug)
			{
			    fprintf(stderr, 
"dbg: wrote generated vrmf entry to internal table for '%s', %s %s %s %s\n",
				    ptfName, 
				    skipLeadingZeros(newvrmfentry.version),
				    skipLeadingZeros(newvrmfentry.release),
				    skipLeadingZeros(newvrmfentry.mod),
				    skipLeadingZeros(newvrmfentry.fix)) ;
			}
		} /* else*/

	    }/*while ptf_pkg*/

	}/* while lpplist */
 /* 
  *---------------------------------------------------------
  * Need to add aparslist to the last PTF record
  * generated in the internal table before closing it.
  *---------------------------------------------------------
  */
  fprintf(internalTableFp, " %s\n", aparslist) ;

  fclose (lppListFp);
  fclose (internalTableFp);
  fclose (ptfreqslistFp);
}/*main*/

/*------------------------------------------------------------------
| This function calculates the vrmf number for a given ptf and     |
| fileset.                                                          |
-------------------------------------------------------------------*/
void
generateVRMF(char * fileset, char *bldcycle, vrmftype * ptfvrmf, char * top, 
             char *specialTypePtf)
{
	FILE *vrmfFp;/* file pointer to vrmfFile.*/
	char vrmf[VRMFSIZE]; /* to store the vrmf as string*/
	char vrmfFile[MAXPATHLEN+1];/* to store the vrmf file name*/
        /* to store the full path to vrmf file in UPDATE tree*/
	char vrmffilePath[MAXPATHLEN+1];
	vrmftype vrmfentry;

	/*----------------------------------------------------------
	| get the directory path to vrmf file. vrmf file is in the |
	| same directory as the ptfsList and cumsList files.       |
	| Open the vrmfFile for read access. It should always exist|
	----------------------------------------------------------*/
	getDirName(fileset, top, vrmffilePath);
	sprintf(vrmfFile, "%s/%s", vrmffilePath, "vrmfFile");
	vrmfFp = openFile(vrmfFile, "r");
	stripComments (vrmfFp, vrmf); 
	if (debug)
	{
	    fprintf(stderr, "dbg: Using vrmfFile at '%s'\n", vrmfFile) ;
	    fprintf(stderr, "dbg: Got following record from vrmfFile '%s'\n",
		    vrmf) ;
	}

	/*--------------------------------------------------------------------
	| split vrmf number into individual version, release mod and fix     |
	| values.                                                            |
	--------------------------------------------------------------------*/
	splitVRMF(fileset, vrmf, &vrmfentry);

	bumplevel(vrmfentry, ptfvrmf, specialTypePtf);
	fclose (vrmfFp);
}
 
/*--------------------------------------------------------------------------
| This function calculates the new vrmf from the old value. It             |
| increases the fix level by one if the ptf is not a cum_ptf,              |
| else it increases the mod level by one and resets the fix level.         |
--------------------------------------------------------------------------*/
void
bumplevel( vrmftype vrmfentry, vrmftype * ptfvrmf, char *specialPtfType)
{

	/*-------------------------------------------------------------------------
	| If it is not a maintainace (cum_ptf) type ptf then bump the fix level   |
	--------------------------------------------------------------------------*/
 	if ( ! strlen(specialPtfType) ) 
 	{
		*ptfvrmf = vrmfentry;
		incrstring(ptfvrmf->fix);
		if (debug)
		{
		    fprintf(stderr, "dbg: 'normal' PTF; new fix level = %s\n",
			    ptfvrmf->fix) ;
		}
	}
 	else
	{
	/*------------------------------------------------------------------------
	| else bump the mod level and reset the fix level                        |
	-------------------------------------------------------------------------*/
		*ptfvrmf = vrmfentry;
		incrstring(ptfvrmf->mod);
		strcpy(ptfvrmf->fix, "0000");
		if (debug)
		{
		    fprintf(stderr, 
		       "dbg: 'special ' PTF; mod level =%s; fix level = %s\n",
			    ptfvrmf->mod, ptfvrmf->fix) ;
		}
	}

}

/*--------------------------------------------------------------------------
| This function takes the vrmf string and converts it into an integer.     |
| Then it increments the integer and converts it back into a string.       |
---------------------------------------------------------------------------*/
void
incrstring(char * str)
{
	int i,j,k;
 
	/*--------------------------------------------------------------
	| convert the string into an integer and increment by 1        |
	---------------------------------------------------------------*/
	i = atoi(str);
	i++;

	/*--------------------------------------------------------------
	| convert the integer back into the string                     |
	---------------------------------------------------------------*/
	str[MODFIXLEN] = 0;
	for (j=1; j<=MODFIXLEN; j++) 
	{
		k = i % 10;
		i = i/10;
		str[MODFIXLEN-j] = '0' + k;
	}
}

   
/*---------------------------------------
| Strip comment lines as we go.         |
---------------------------------------*/
int
stripComments (FILE *insfp, char *line)
{
	char *num;
	char *ptr;

	while   ( (num = fgets(line,BUFSIZE,insfp)) != NULL )
	{
		/*-------------------------------------------------------
		| Position line[i] at first non-whitespace character.	|
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

