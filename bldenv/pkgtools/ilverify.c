static char sccsid[] = "@(#)06  1.13  src/bldenv/pkgtools/ilverify.c, pkgtools, bos412, GOLDA411a 8/30/94 15:15:33";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: createPrqPaths
 *              createIlpaths
 *              find_inslist
 *              getOrderedList
 *              getPrereqs
 *              il_array
 *		main
 *              prereqMatch
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

#include <sys/stat.h>
#include "ins.h"
#include "ade.h"

extern char *optarg;
extern char *commandName;

main(int ac,char **av)
{
    char *searchDir = '\0';			/* -p option */
    char *dbFileOut = '\0';			/* -d option */
    char *dbFileIn = '\0';			/* -n option */
    char exceptList[MAXPATHLEN+1];		/* -e option */
    char *idTableFile = '\0';			/* -t option */
    char *ptr;
    char insname[MAXPATHLEN+1];
    char buf[ADE_BUFSIZE];
    int dFlag=0, iFlag=0;
    int arg, i=0, num_ilpaths=0;
    char insLists[INS_MAXPATHS][MAXPATHLEN+1];
    int rc=INS_SUCCESS, readrc=0;
    InsEntry insentry;
    struct stat statbuf;

    FILE *ilpathsfp = NULL;
    FILE *insfp = NULL;
    FILE *id_ptr = NULL;
    FILE *except_ptr = NULL;
    FILE *dbout_ptr = NULL;
    FILE *dbin_ptr = NULL;

    /*-----------------------------------------------------------
    | Remove old copies of ilpaths file and prqpaths file.	|
    -----------------------------------------------------------*/
    if ( !stat ("ilpaths", &statbuf) )
	unlink ("ilpaths");
    if ( !stat ("prqpaths", &statbuf) )
	unlink ("prqpaths");

    while (( arg = getopt (ac, av, "p:e:i:t:d:n:")) != EOF) 
    {
	switch (arg) 
	{
	    case 'i':
		iFlag++;
		ilpathsfp = openFile("ilpaths","w");
		ptr = strtok(optarg,":");
		do {
		    sprintf(insname,"%s\n",ptr);
		    fputs(insname,ilpathsfp);
		    ptr = strtok(NULL,":");
		}
		while (ptr != (char *)NULL);
                fclose(ilpathsfp);   
		break;
	    case 'p':
		searchDir = optarg;
		break;
	    case 't':
		idTableFile = optarg;
		break;
	    case 'e':
	        strcpy (exceptList, optarg);
		break;
	    case 'd':
		dFlag++;
		dbFileOut = optarg;
		break;
	    case 'n':
		dbFileIn = optarg;
		break;
	    default:
		usage();
		break;
	}
    }

    /*-------------------------------------------
    | -d and -n are mutually exclusive.		|
    -------------------------------------------*/
    if (*dbFileIn && *dbFileOut)
       usage();

    /*-------------------------------------------
    | Open optional id table file.		|
    -------------------------------------------*/
    if ( *idTableFile )
       id_ptr = openFile(idTableFile,"r");

    /*-------------------------------------------
    | Open optional exception list file.	|
    -------------------------------------------*/
    if ( !(*exceptList) )
	if ( (ptr = getenv(EXCEPTION_LIST)) != NULL )
	    strcpy (exceptList, ptr);
    if (*exceptList)
       except_ptr = openFile(exceptList,"r");

    /*-------------------------------------------
    | Open optional output database file.	|
    -------------------------------------------*/
    if ( dFlag )
	dbout_ptr = openFile(dbFileOut,"w");

    /*-------------------------------------------
    | Open optional input database file.	|
    -------------------------------------------*/
    if ( *dbFileIn )
	dbin_ptr = openFile(dbFileIn,"r");
    else
	if ( dbFileIn = getenv(DB_FILE_IN) )
	    if ( dFlag )
		fatal (dbFileError);
	    else
		dbin_ptr = openFile(dbFileIn,"r");

    num_ilpaths = createIlpaths (searchDir, iFlag, insLists, dbFileIn);

    /*-------------------------------------------------------------------
    | If we are verifying against a dabatase file we won't do the	|
    | requisite directory checking because we won't have all of the	|
    | prereq files available.						|
    -------------------------------------------------------------------*/
    if ( *dbFileIn )
    {
	fprintf (stdout, "%s:  Verification against an input database file\n",commandName);
	fprintf (stdout, "\twill not include requisite directory checks.\n\n");
    }
    /*-------------------------------------------------------------------
    | Otherwise, rewrite the ilpaths file from the ordered inslist	|
    | array so that dirCheck will process them in the correct order.	|
    -------------------------------------------------------------------*/
    else
	ilpathsfp = openFile ("ilpaths", "w");
	
    /*------------------------------------------------------------------- 
    | Read each inslist in array and verify each inslist entry and	|
    | rewrite the ilpaths file if applicable.				|
    -------------------------------------------------------------------*/
    fprintf (stdout, "\nPerforming syntax check and link verification:\n\n");
    fflush (stdout);
    for (i = 0; i < num_ilpaths; i++)
    {
	if ( (insfp = fopen (insLists[i], "r")) )
	{
	    if ( !strlen(dbFileIn) )
		fprintf (ilpathsfp, "%s\n", insLists[i]);
	    fprintf (stdout, "\n%s:\n",insLists[i]);
	    fflush (stdout);
	    /*-----------------------------------------------------------
	    | Write inslist header line to database output file.	|
	    -----------------------------------------------------------*/
	    if ( dFlag )
	    {
		sprintf (buf, "!!%s\n",insLists[i]);
		fputs (buf, dbout_ptr);
	    }
	    /*-----------------------------------------------------------
	    | A non-zero value in the 3rd parm used to cause extra	|
	    | verification checks for hard link entries, however this   |
	    | is no longer necessary (so 3rd parm is always 0).         |
	    -----------------------------------------------------------*/
	    while ( (readrc = readList (insfp, &insentry, 0)) != EOF )
	    {
		rc |= readrc;
		rc |= idTable (&insentry, id_ptr);
	        rc |= crossFileSys (&insentry);
                
		if ( dFlag && (readrc == 0) )
		    writeDb (dbout_ptr, &insentry);
		fflush (dbout_ptr);
	    } 
	    fclose(insfp);
	}
	else
	    warning (insOpenFailed, insLists[i]);
    }

    if ( !strlen(dbFileIn) )
	fclose (ilpathsfp);

    /*-----------------------------------------------------------
    | Build perl command.  The format of the command is		|
    | dirCheck <ilfile> <exceptList> <inputDBFile>		|
    | where exceptList and inputDBFile are N if they do not	|
    | apply.							|
    -----------------------------------------------------------*/

    if ( (ptr = getenv(ODE_TOOLS)) != NULL )
	sprintf (buf, "%s/usr/bin/dirCheck ilpaths ", ptr);
    else
	sprintf (buf, "dirCheck ilpaths ");

    if ( except_ptr )
	strcat (buf, exceptList);
    else
	strcat (buf, "N");
    strcat (buf, " ");

    if ( dbin_ptr )
	strcat (buf, dbFileIn);
    else
	strcat (buf, "N");

    rc |= system (buf);
    unlink("ilpaths");

    if ( dFlag )
	fclose (dbout_ptr);
    if ( dbin_ptr )
       fclose (dbin_ptr);

    exit(rc);
}

/*-----------------------------------------------------------------------
| Create an inslist array.  If dbFlag is set the array is not ordered.	|
| Otherwise, check the prereq files for each inslist recursively and	|
| list all prereq inslist files first.  The prqpaths file has a list	|
| of all of the prereq files.						|
-----------------------------------------------------------------------*/

il_array(char insLists[][MAXPATHLEN+1], int dbFlag)
{
    int inscount = 0;
    int i;
    int prqListIndex=0;			/* Index to the lppOpt:prereq list  */
    int prqCount=0;			/* Index to the ordered prereq list */
					/*    which contains individual lpp */
					/*    option names ordered by       */
					/*    the prereq.S files	    */
    FILE *pfilefp, *insfp;
    char prqFileName[MAXPATHLEN+1];
    char *prereqName;
    char *ptr;
    char prereqList[INS_MAXPATHS][2*MAXPATHLEN+2];    /* lppOpt:prereq list */
    char orderedPrereqList[INS_MAXPATHS][MAXPATHLEN+1];  /* see prqCount    */
   
    /*-------------------------------------------------------------------
    | Create an array of all the lpp option names and their prereqs.	| 
    | The prereq file naming convention is lppOptionName.prereq.S.	|
    | The prereq file contains the name(s) of prereq lpps or lpp	|
    | options.  The format of the array entry is:			|
    | 			lppOptName:prereqlppOptName			|
    -------------------------------------------------------------------*/
    if ( !dbFlag )
    {
	if ( pfilefp = fopen("prqpaths", "r") )
	{
	    while ( (fgets(prqFileName, ADE_BUFSIZE, pfilefp)) != NULL)
	    {
		if ( ptr = strchr (prqFileName, '\n') )
		    *ptr = '\0';
		getPrereqs (prqFileName, prereqList, &prqListIndex);
	    }
            fclose(pfilefp);
            unlink("prqpaths");

	    /*-----------------------------------------------------------
	    | Create an ordered array of prereqs, eliminating the dups.	| 
	    | This is done by recursively searching the prereq list 	|
	    | until the end of the prereq chain is found.		|
	    -----------------------------------------------------------*/
	    for (i=0; i < prqListIndex; i++)
	    {
		prereqName = strchr(prereqList[i],':');
		prereqName++;
		prereqMatch (prereqName, &prqCount, orderedPrereqList, prereqList, prqListIndex);
	    }
	}
    }

    insfp = openFile("ilpaths", "r");

    /*-----------------------------------------------------------
    | If a database input file was specified the inslist do	|
    | not have to be ordered.					|
    -----------------------------------------------------------*/
    if ( dbFlag )
    {
        while ( fgets(insLists[inscount], MAXPATHLEN+1, insfp) != NULL)
        {
            if ( ptr = strchr (insLists[inscount], '\n') )
                *ptr = '\0';
            inscount++;
            if (inscount == INS_MAXPATHS)
                fatal (numInslistsExceededLimit, INS_MAXPATHS); 
        }
        return (inscount);
    }
    else
	inscount = getOrderedList (insLists, insfp, prqCount, orderedPrereqList, prereqList, prqListIndex);

   fclose(insfp);
   return (inscount);
}

/*-----------------------------------------------------------------------
| Recursively search prereqList to get the prereq chain for an lpp	|
| option.  When the prereq chain stops write the lpp names to the	|
| ordered prereq list.							|
-----------------------------------------------------------------------*/

void
prereqMatch (char *prereqName, int *prqCount,
	char orderedPrereqList[][MAXPATHLEN+1], char prereqList[][2*MAXPATHLEN+2],
	int prqListIndex)
{
    int i, index;
    char *ptr;
    char lppOptName[ADE_BUFSIZE];

    for (i=0; i < prqListIndex; i++)
    {
	strcpy (lppOptName, prereqList[i]);
	ptr = strchr(lppOptName,':');
	*ptr = '\0';
	ptr++;
	if ( !strcmp(prereqName, lppOptName) )
	    prereqMatch(ptr,prqCount,orderedPrereqList,prereqList, prqListIndex);
    }

    /*-----------------------------------------------------------
    | Check for dups as we add to the orderedPrereqList.	|
    -----------------------------------------------------------*/
    for ( index=0; index < *prqCount; index++ )
	if ( !strcmp (prereqName, orderedPrereqList[index]) )
            return; 

    if (*prqCount < INS_MAXPATHS)
    {
        strcpy (orderedPrereqList[*prqCount], prereqName);
        *prqCount = (*prqCount) + 1;
    }
    else
        fatal (numOptionsExceededLimit, INS_MAXPATHS); 
}

/*-----------------------------------------------------------------------
| Search the inslist array for a particular inslist.  Return the index	|
| if a match is found, otherwise -1.					|
-----------------------------------------------------------------------*/

find_inslist(char iLists[][MAXPATHLEN+1], int inscount, char *ilname)
{
    int i = 0;
    char *tmpname;

    while ( i < inscount )
    {
	if ( (tmpname = strrchr(iLists[i],'/')) != '\0' )  
	    ++tmpname;
	else
	    tmpname = iLists[i];
	if ( !strcmp(tmpname,ilname) )
	    return (i);
	i++;
   }
   return (-1);
}

/*-----------------------------------------------------------------------
| Generate the prereq file name for inslists specified with the -i	|
| option.  If the prereq file exists then add it to prqpaths.		|
-----------------------------------------------------------------------*/

void createPrqPaths()
{
    char fName[ADE_BUFSIZE];
    char *ptr;
    struct stat statbuf;
    FILE *ilpathfp;
    FILE *prqpathfp;

    ilpathfp = openFile("ilpaths","r");
    prqpathfp = openFile("prqpaths","w");
    while ( fgets(fName, ADE_BUFSIZE, ilpathfp) != NULL )
    {
	if ( (ptr = strrchr(fName,'.')) != NULL )
	    *ptr = '\0';
	strcat (fName, "prereq.S");
	if ( !stat (fName, &statbuf) )
	{
		fputs (fName, prqpathfp);
		fputs ("\n", prqpathfp);
	}
    }
    fclose(ilpathfp);
    fclose(prqpathfp); 
}

/*-----------------------------------------------------------------------
| Create ilpaths file and the prqpaths file. The ilpaths file is a list	|
| of the inslist files (*.il) and the prqpaths file is a list of the	|
| prereq files (*.prereq.S).  The prereq files may not be located in	|
| the same directory as the associated inslist.				|
|									|
| If -p and -i were not used on the command line, look for the		|
| files in the current directory.					|
-----------------------------------------------------------------------*/
createIlpaths ( char *searchDir, int iFlag, char insLists[][MAXPATHLEN+1],
	char *dbFileIn )
{
    int num_ilpaths, dbFlag=0;
    char cmd[ADE_BUFSIZE];
    char cmdPath[MAXPATHLEN+1];
    struct stat statbuf;
    char *ptr;

    /*-----------------------------------------
    | Set path name for find and ls commands. |
    -----------------------------------------*/

    strcpy (cmdPath, "/bin");

    /*-----------------------------------------------------------
    | If an input database file was specified there will	|
    | be no prerequisite directory checking.			|
    -----------------------------------------------------------*/
    if ( strlen(dbFileIn) )
	dbFlag++;

    if ( (!*searchDir) && (!iFlag) )
    {
	sprintf(cmd, "%s/ls *.il > ilpaths 2>/dev/null", cmdPath);
	system(cmd);
	if ( stat ("ilpaths", &statbuf) )
	    fatal (statFailed);
	else
		if ( !statbuf.st_size )
			fatal (noInsCurDir); 

        if ( !dbFlag )
	{
	    sprintf(cmd, "%s/ls *.prereq.S > prqpaths 2>/dev/null", cmdPath);
	    system(cmd);
	}
	num_ilpaths = il_array(insLists, dbFlag);
    }
    else {
	if ( iFlag && !dbFlag )
	   createPrqPaths();

	/*---------------------------------------------------------------
	| Get recursive list of inslist files from search directory.	|
	---------------------------------------------------------------*/
	if ( *searchDir )
	{
            fprintf (stdout, "\nPerforming find command to get list of inslist files:\n\n");
            fflush (stdout);
	    sprintf(cmd,"%s/find %s -name \"*.il\" -print >> ilpaths 2>/dev/null", cmdPath, searchDir);
	    system(cmd);
	    if ( !dbFlag )
	    {
                fprintf (stdout, "\nPerforming find command to get list of prereq files:\n\n");
                fflush (stdout);
		sprintf(cmd,"%s/find %s -name \"*.prereq.S\" -print >> prqpaths 2>/dev/null", cmdPath, searchDir);
		system(cmd);
	    }
	}

	if ( !(num_ilpaths = il_array(insLists, dbFlag)) )
		fatal (noInsSearchDir, searchDir);
    }
    return (num_ilpaths);
}

/*---------------------------------------------------------------------
| NAME: getPrereqs
|
| DESCRIPTION: Get the prereqs from the prqFileName and add them to the
|              prereq list
|
| PRE CONDITIONS: none
|
| POST CONDITIONS: none
|
| PARAMETERS:
|   input:    prqFileName - name for the file containing prereq infor-
|                           mation
|   output:   prereqList  - array of prereqs with the following format
|                           option:prereq
|   output:   prqIndex    - number of prereqs in the array
|
| NOTES: The prereqs in the file have the word prereq preceeding the name
|
| DATA STRUCTURES: none
|
| RETURNS:  void
-----------------------------------------------------------------------*/

getPrereqs (char *prqFileName, char prereqList[][2*MAXPATHLEN+2], int *prqIndex)
{
	char tmpFileName[MAXPATHLEN+1];
	char prereqLine[ADE_BUFSIZE];
	char lppOptName[ADE_BUFSIZE];
	char *ptr;
	FILE *prqfp;

	strcpy(tmpFileName,prqFileName);

	/*-----------------------------------------------
	| Strip off leading path and .prereq.S suffix	|
	| to get lpp name from the prereq file path.	|
	-----------------------------------------------*/
	ptr = strstr(tmpFileName,".prereq.S");
	*ptr = '\0';
	ptr = strrchr(tmpFileName,'/');
	if ( ptr )
	    ptr++;
	strcpy (lppOptName, ptr);

        prqfp = fopen(prqFileName, "r");

        if ( !prqfp )
        {
            warning (OpenError, prqFileName, 'r');
            return;
        }

	/*-------------------------------------------------------
	| Skip over the word prereq if it is there.  We don't	|
	| need to keep track of coreqs, ifreqs or instreqs.	|
	-------------------------------------------------------*/
	while (stripComments(prqfp, prereqLine) != EOF)
	{
 		if  (!(ptr = strstr(prereqLine, "prereq")))
 			continue;

                /* parce up to the option name */
                ptr = ptr + strlen("prereq") + 1;
                while (isspace(*ptr))
                {
                   ptr++;
                }

                /* get just the option name ( the option name may have junk
                   after it) */
                ptr = strtok(ptr," \t");

                if (*prqIndex < INS_MAXPATHS)
                {
		    sprintf(prereqList[*prqIndex],"%s:%s",lppOptName,ptr);
		    *prqIndex = (*prqIndex)+1;
                }
                else
                    fatal (numOptionsExceededLimit, INS_MAXPATHS); 
	}
	fclose (prqfp);
}

/*---------------------------------------------------------------
| NAME: getOrderedList
|
| DESCRIPTION: Order the *.il files according to the order in the 
|              prereq list.  The bos.rte.il file should be first,
|              followed by the inslists for the prereqs, followed
|              by everything else.  tmpLists is used to store the
|              inslists in memory to speed up searching.
|
| PRE CONDITIONS: none 
|
| POST CONDITIONS: none
|
| PARAMETERS: 
|     input:  orderedPrereqList -  list of options in prereq order
|     input:  prqCount - number if entries in orderedPrereqList
|     input:  prereqList - the list containing the option and its
|                          prereq information (format option:prereq)
|     input:  prqListIndex - number of entries in prereqList
|     input:  inspf - pointer to file containing list of *.il file 
|                     names
|     output: insLists - list of ordered *.il files
|
| NOTES:
|
| DATA STRUCTURES: none
|
| RETURNS: insindex - the number of entries (*.il files) in insLists
-----------------------------------------------------------------*/

static int
getOrderedList ( char insLists[][MAXPATHLEN+1], FILE *insfp,
	int prqCount, char orderedPrereqList[][MAXPATHLEN+1],
        char prereqList[][2*MAXPATHLEN+2],
        int prqListIndex)
{
    char tmpLists[INS_MAXPATHS][MAXPATHLEN+1];
    int inscount=0, insindex = 0, i, j;
    char ilname[MAXPATHLEN+1];
    char *ptr;
    int prqInsIndex;
    char lppOptName[ADE_BUFSIZE];

    while ( fgets(tmpLists[inscount], MAXPATHLEN+1, insfp) != NULL)
    {
        if ( ptr = strchr (tmpLists[inscount], '\n') )
            *ptr = '\0';
        inscount++;
        if (inscount == INS_MAXPATHS)
            fatal (numInslistsExceededLimit, INS_MAXPATHS); 
    }

    if ( (prqInsIndex = find_inslist(tmpLists,inscount,"bos.rte.il")) != -1 )
	strcpy(insLists[insindex++],tmpLists[prqInsIndex]);
    else
	inserror(BosNotFound);

    /*-------------------------------------------------------------------
    | Get the prereq inslists, skipping bos.rte since we've already	|
    | made sure it is there.						|
    -------------------------------------------------------------------*/
    for ( i=0; i < prqCount; i++)
    {
	if ( !strcmp(orderedPrereqList[i], "bos.rte") )
	    break;
	sprintf (ilname, "%s.il", orderedPrereqList[i]);
	if ( (prqInsIndex = find_inslist(tmpLists,inscount,ilname)) != -1 )
	    strcpy(insLists[insindex++],tmpLists[prqInsIndex]);
	else
            for (j=0; j < prqListIndex; j++)
            {
                strcpy(lppOptName,prereqList[j]);
                ptr = strchr(lppOptName,':');
                *ptr = '\0';
                ptr++;
                if( !strcmp(ptr,orderedPrereqList[i]))
                    inserror (prereqInslistNotFound, orderedPrereqList[i], lppOptName);
            }
    }

    /*----------------------------
    | Get the remaining inslists |
    ----------------------------*/
    for ( i=0; i < inscount; i++)
    {
	if ( (ptr = strrchr(tmpLists[i],'/')) != NULL)
	{
	    ++ptr;
	    strcpy (ilname, ptr);
	}
	else
	    strcpy (ilname, tmpLists[i]);
	if ( find_inslist(insLists,inscount,ilname) == -1 )
	    strcpy(insLists[insindex++],tmpLists[i]);
    }
    return (insindex);
}

