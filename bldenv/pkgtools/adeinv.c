static char sccsid[] = "@(#)83  1.46  src/bldenv/pkgtools/adeinv.c, pkgtools, bos41J, 9516A_all 4/18/95 12:06:36";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *		checkSize
 *		chktype
 *		getDirectoryName
 *		getInstDirs
 *		getyp
 *		main
 *		openoutfiles
 *		permTable
 *		printLine
 *		printMode
 *		print_links
 *		proclst
 *		readlp
 *		tableread
 *		writeInvTcb
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

#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <sys/dir.h>
#include <errno.h>
#include "ade.h"
#include "adeinventory.h"
#include "acfList.h"

void		openoutfiles(char *, char *, char *, char *,
			char *, char *, char *);
int		readlp(char *, char *);
void		printLine (char *, int);
void		getDirectoryName (char *, char *, char);
void		getInstDirs (int, int);
void		print_links (InsEntry *, int);
void		printMode (InsEntry *, int);
char		*getyp(InsEntry *);

/*
 *----------------------------------------------------
 * External variable accessed in this module.
 *----------------------------------------------------
 */
extern char	*optarg;		/* For cmdline option processing  */
extern char     **verbose_msgs ;	/* Ptr to verbose message set	  */
extern char     **default_msgs ;	/* Ptr to default (short) msg set */
extern int	errno;

/*
 *-----------------------------------------------------
 * Global data available to other modules in adeinv.
 *-----------------------------------------------------
 */
char	**msgArray ;			/* Ptr to msg set to use  	  */
char	*commandName  = NULL;

/*
 *-----------------------------------------------------
 * Module level data.  Some of this data could
 * reside inside main, but to allow error messages
 * access the information, it is at module level.
 *-----------------------------------------------------
 */
static	char 	insfilename[MAXPATHLEN+1];
static	char 	tablename[MAXPATHLEN+1];

/*
 *=====================================================
 * Function : Main
 * Purpose  : 
 *=====================================================
 */
main(int ac,char **av)
{
    char lppFileName[MAXPATHLEN+1];
    char lppOption[LPPNAMELEN+1];
    char lppname[LPPNAMELEN+1];
    char tcbFile[MAXPATHLEN+1];				/* Output file */
    char invFile[MAXPATHLEN+1];				/* Output file */
    char applyFile[MAXPATHLEN+1];			/* Output file */
    char sizeFile[MAXPATHLEN+1];			/* Output file */
    char xrefFile[MAXPATHLEN+1];			/* Output file */
    char *outputDirectory;
    int  rFlag=0,sFlag=0,DFlag=0,lFlag=0, updtFlag=0;
    int YFlag=0, bootFlag=0;
    int aFlag=0 ;
    int arg,i;
    FILE *insl_ptr;
    InsEntry insentry;
    int rc=ADE_SUCCESS;
    char *vrmf = NULL;

    msgArray = (char**)&default_msgs ;			/* default messages @ 1st */
    bzero (insfilename,MAXPATHLEN+1);
    bzero (lppFileName,MAXPATHLEN+1);
    bzero (lppname,LPPNAMELEN+1);
    outputDirectory = NULL;

    commandName = getCommandName( av[0] );
    acfInit() ;

    while (( arg = getopt (ac, av, "a:d:t:s:i:l:rDLu:U:vY")) != EOF) 
    {
	switch (arg) 
	{
	    case 'a':
	        if (acfLoadFile(optarg) == ADE_ERROR)
		{
		   /*
		    *---------------------------------------------
		    * Inform user of error and give a usage
		    * before aborting.
		    *---------------------------------------------
		    */
		    inserror(msgArray[ACFloadError], optarg) ;
		    usage() ;
		}
		aFlag++ ;
		break ;
	    case 's':
		sFlag++;
		i = 0;
		shipPaths[i++] = strtok(optarg,":");
		while ((shipPaths[i++] = strtok(NULL,":")) != (char *)NULL);
	        num_paths = --i;
		break;
	    case 'i':
       		strcpy(insfilename,optarg);
       		break;
    	    case 't':
		strcpy(tablename,optarg);
          	permtable++;
		break;
	    case 'l':
		strcpy (lppname, optarg);
       		break;
	    case 'u':
		strcpy (lppFileName, optarg);
		bootFlag=readlp(lppFileName,lppOption);
		break;
	    case 'r':
		rFlag++;
		break;
	    case 'D':
		DFlag++;
		break;
	    case 'L':
		lFlag++;
		break;
	    case 'U':
		updtFlag++;
		initHashTable();
                vrmf = optarg;
		break;
	    case 'd':
		outputDirectory = optarg;
		break;
	    case 'v':
		msgArray = (char**)&verbose_msgs ;
		break ;
	    case 'Y':
		YFlag++;
		break;
	    case '?':
		usage();
		break;
      }
    }

    /*-------------------------------------------------------------------
    | Verify that all required flags were provided on command line.	|
    -------------------------------------------------------------------*/
    
    if ( !sFlag || !insfilename[0] || !lppFileName[0] || !lppname[0] )
	usage();

    if ( rFlag && DFlag )
        usage();

    getInstDirs(rFlag,DFlag);

    dirList = listNew();
    listInit (dirList);

    openoutfiles(lppOption,
		tcbFile,
		invFile,
		applyFile,
		sizeFile,
		xrefFile,
		outputDirectory);

   /*-----------------------------------------------------------
    * Don't write an xref file if this is for an update (vrmf
    * contains a string) or if processing the root part.
    * The xref file is written during usr processing.
    *-----------------------------------------------------------
    */
    if ( strlen(vrmf) || rFlag )
    {
	fclose (opxref_ptr);
	opxref_ptr = NULL;
    }

    if ( permtable )
    {
	perm_ptr=fopen(tablename,"r");
	if ( !perm_ptr )
	{
	    warning (msgArray[CantOpenTableFile], tablename);
	    permtable=0;
	}
    }
      
    /*-------------------------------------------------------------------
    | Read each inslist entry and write to apply list, inventory file,	|
    | tcb file ,size file and xref file .  If -r flag specified, only	|
    | process root files, which have a lower case type.			|
    -------------------------------------------------------------------*/

    fprintf (stderr,"\n");
    insl_ptr = openFile(insfilename, "r");
    while ( (i=readList (insl_ptr, &insentry, lFlag)) != EOF )
    {
	if ( !i && opxref_ptr && !isaLink (insentry.type) )
	    fprintf(opxref_ptr,".%s %s\n", insentry.object_name,lppOption);

	if ( rFlag && isupper((int)insentry.type) )
		continue;
	if ( !i )
		rc |= proclst(&insentry,
			lppOption,
			lppname,
			rFlag,
			lFlag,
			updtFlag,
			vrmf,
			YFlag,
			aFlag);
	else
	    rc |= i;
    }

    getInSize(lppOption, rFlag, DFlag, lppname, outputDirectory);		 

    /*-----------------------------------------------------------
    | Be sure tcb file is flushed before calling outSize.	|
    | It will do a stat on the tcb file.			|
    -----------------------------------------------------------*/
    fflush (optcb_ptr);
    outSize(updtFlag, tcbFile, lppOption, bootFlag, rFlag);

    if (updtFlag)
    {
	if (acfWriteACF(TRUE) == ADE_ERROR)
	{
	    inserror(msgArray[lppacfError]) ;
	    rc |= ADE_ERROR ;
	}
    }

    fclose(opinv_ptr);
    fclose(opal_ptr);
    fclose(opsize_ptr);
    fclose(optcb_ptr);
    fclose(opxref_ptr);

    rc |= checkSize (invFile);
    rc |= checkSize (applyFile);
    rc |= checkSize (tcbFile);
    rc |= checkSize (sizeFile);

    exit(rc);
}

/*-----------------------------------------------------------------------
| Open the inventory, tcb, size, and apply list files for this lpp	|
| option.								|
-----------------------------------------------------------------------*/

void openoutfiles (char *lppOption, char *tcbFile, char *invFile,
	char *applyFile, char *sizeFile, char *xrefFile, char *outDir)
{
	int dirFlag = 0;
	if ( strlen(outDir) )
		dirFlag++;

	if ( dirFlag )
	{
		strcpy (invFile,outDir);
		strcat (invFile, "/");
		strcat (invFile, lppOption);
	}
	else
		strcpy(invFile,lppOption);
	strcat(invFile,".inventory");
	opinv_ptr = openFile(invFile,"w");

	if ( dirFlag )
	{
		strcpy (applyFile,outDir);
		strcat (applyFile, "/");
		strcat (applyFile, lppOption);
	}
	else
		strcpy(applyFile,lppOption);
	strcat(applyFile,".al");
	opal_ptr = openFile(applyFile,"w");

	if ( dirFlag )
	{
		strcpy (sizeFile,outDir);
		strcat (sizeFile, "/");
		strcat (sizeFile, lppOption);
	}
	else
		strcpy(sizeFile,lppOption);
	strcat(sizeFile,".size");
	opsize_ptr = openFile(sizeFile,"w");

	if ( dirFlag )
	{
		strcpy (tcbFile,outDir);
		strcat (tcbFile, "/");
		strcat (tcbFile, lppOption);
	}
	else
		strcpy(tcbFile,lppOption);
	strcat(tcbFile,".tcb");
	optcb_ptr = openFile(tcbFile,"w");

	if ( dirFlag )
	{
		strcpy (xrefFile,outDir);
		strcat (xrefFile, "/");
		strcat (xrefFile, lppOption);
	}
	else
		strcpy(xrefFile,lppOption);
	strcat(xrefFile,".xref");
	
	if ( strlen (xrefFile) )
	    opxref_ptr = openFile(xrefFile,"w");
	else
	    opxref_ptr = NULL;
}

/*---------------------------------------------------------------
| NAME:  proclst
|
| DESCRIPTION:  Main processing function for each inslist entry.
|    This function drives the generation of the output files
|    based on the inslist entry and command line parameters.
|
| PRE CONDITIONS:  Command line checking has been performed.
|    insentry contains the inslist data from a syntactically valid
|    inslist file entry.
|
| POST CONDITIONS:  The entry is written to the apply list,
|    inventory and tcb files as applicable.  The size structures
|    are updated for each entry.
|
| PARAMETERS:  insentry - pointer to inslist entry structure
|     lppOption - current option or fileset name
|     rFlag     - set if -r flag was used on command line for
|	root processing
|     lFlag     -  set if -L flag was used on command line for
|	writing links to apply list
|     updtFlag  - set if -U option was used on command line for
|	update mode
|     vrmf      - set to current version.release.mod.fix level if
|	update mode was specified
|     YFlag     - set if adeinv should ignore entries not found
|	in ship trees and continue processing
|     aFlag     - set if the -a option was used to specify a user-
|	provided lpp.acf file
|
| NOTES:
|
| DATA STRUCTURES:  insentry - structure containing inslist entry
|     data for an entry in the inslist file.
|
| RETURNS:  0 for success
|	    non-zero for error
-----------------------------------------------------------------*/

proclst(InsEntry  *insentry, 
	char	*lppOption, 
	char	*lppname,
	int	rFlag, 
	int	lFlag, 
	int	updtFlag, 
	char	*vrmf,
	int	YFlag,
	int	aFlag)
{
    Fileinfo fileinfo;
    char cksum[512];
    int tcb_sw=0;
    off_t filesize;
    char DirName[MAXPATHLEN+1];
    int rc = 0;
    int i = 0;
    
    /*---------------------------------------------------------------
    | Write a tcb file if the tcbflag is set or if the file is
    | SUID or SGID.
    ----------------------------------------------------------------*/

    if ( (insentry->tcbflag == 'Y') ||
	 (insentry->mode & S_ISUID) ||
	 (insentry->mode & S_ISGID) )
	tcb_sw++;

    fileinfo.file_fd = 0;
    if ( !chktype(insentry) &&
	((insentry->type != 'd') && (insentry->type != 'D')) )
    {
	    strcpy(fileinfo.filename, insentry->object_name);
	    if (findfile(fileinfo.filename, shipPaths, num_paths,
		fileinfo.ship_name, &fileinfo.f_st.st, cksum) != 0)
	    {
                warning(msgArray[FileNotFound], fileinfo.filename) ;
		bzero (cksum, 512);
		fileinfo.f_st.st.st_size=0;
		if ( YFlag )
		    return (0);
		else
		    return (1);
	    }
    }
    filesize=fileinfo.f_st.st.st_size;

   /*
    *-----------------------------------------------------------------
    * If no -r flag (processing usr part), all lower case object
    * names should be preceded by /usr/lpp/<lppname>/inst_root before
    * any entries are written to the output files.
    * If -U flag is set (vrmf is not 0 length), then we want to 
    * change the path to include the vrmf_level and option in the
    * pathname, /usr/lpp/<lppname>/<option>/<vrmf_level>/inst_root,
    * before we write any entries to the output files.
    *-----------------------------------------------------------------
    */

    if ( !rFlag && islower((int)insentry->type) )
    {
        if ( ! strlen(vrmf))
        {
	    sprintf(DirName,"/usr/lpp/%s/inst_root%s",
		lppname,
		insentry->object_name);
	    strcpy(insentry->object_name, DirName);
            if ( lFlag )
	    	for ( i=0; i < insentry->numHardLinks; i++ )
		    {
	    	    sprintf(DirName,"/usr/lpp/%s/inst_root%s",
			lppname,
			insentry->hardLinks[i]);
	    	    strcpy(insentry->hardLinks[i], DirName);
		    }
        }
        else
        {
	    sprintf(DirName,"/usr/lpp/%s/%s/%s/inst_root%s",
		lppname,
		lppOption,
		vrmf,
		insentry->object_name);
	    strcpy(insentry->object_name, DirName);
            if ( lFlag )
	    	for ( i=0; i < insentry->numHardLinks; i++ )
		    {
	    	    sprintf(DirName,"/usr/lpp/%s/%s/%s/inst_root%s",
			lppname,
			lppOption,
			vrmf,
			insentry->hardLinks[i]);
	    	    strcpy(insentry->hardLinks[i], DirName);
		    }
        }
    }

    /*-------------------------------------------------------------------
    | Write to apply list. If the -L option is given then we want to	|
    | include links in the apply list.                                  |
    -------------------------------------------------------------------*/


    if ( !chktype(insentry) )
    {
    	fprintf(opal_ptr,".%s\n",insentry->object_name); 
        if ( lFlag )
	    for ( i=0; i < insentry->numHardLinks; i++ )
    		 fprintf(opal_ptr, ".%s\n", insentry->hardLinks[i]);
    }

    if ( isaLink (insentry->type) && lFlag )
    	fprintf(opal_ptr,".%s\n",insentry->object_name); 

    calculateSize (insentry, filesize, updtFlag, aFlag);

    if ( (insentry->type == 'A') || (insentry->type == 'a') )
        return rc;

    /*-----------------------------------------------------------
    | If no -r flag (processing usr part) do not write lower	|
    | case types to the inventory file.  They are written	|
    | when -r is used.						|
    -----------------------------------------------------------*/
    if ( rFlag || (!rFlag && isupper((int)insentry->type)) )
    {
	/*-------------------------------------------------------
	| Repository directory should get 170 for regular files	|
	| and length of link name for links.			|
	-------------------------------------------------------*/
	if ( !isaLink(insentry->type) )
	{
	    updateSize (reposDir, 170, NULL, 0);
	    if ( tcb_sw )
    		updateSize (secDir, 170, NULL, 0);
	}
	else
	{
	    updateSize (reposDir, strlen (insentry->object_name), NULL, 0);
	    if ( tcb_sw )
    		updateSize (secDir, strlen (insentry->object_name), NULL, 0);
	}
	rc = writeInvTcb(insentry,
			tcb_sw,
			lppOption,
			cksum,
			filesize,
			rFlag);
    }
    return rc;
}

/*-----------------------------------------------------------------------
| Write the inventory file entry and tcb entry if applicable.		|
-----------------------------------------------------------------------*/
writeInvTcb(InsEntry *insentry,int tcb_sw,char *lppOption,
	char *cksum,off_t size,int rFlag)
{
   char p_name[10];
   char g_name[10];
   char invTcbLine[ADE_BUFSIZE];
   int rc = 0;

    bzero(invTcbLine,ADE_BUFSIZE);
     
    rc = permTable(insentry,permtable,p_name,g_name);

    sprintf(invTcbLine,"%s:\n",insentry->object_name);  
    printLine(invTcbLine, tcb_sw);

    sprintf(invTcbLine,"          owner = %s\n",p_name); 
    printLine (invTcbLine, tcb_sw);
    sprintf(invTcbLine,"          group = %s\n",g_name); 
    printLine (invTcbLine, tcb_sw);
    printMode(insentry, tcb_sw);

    sprintf(invTcbLine,"          type = %s\n",getyp(insentry));
    printLine (invTcbLine, tcb_sw);

    if (insentry->numHardLinks)
	print_links(insentry, tcb_sw);

    if ( isaLink (insentry->type) )
    {
	sprintf (invTcbLine, "          target = %s\n",insentry->target);
	printLine (invTcbLine, tcb_sw);
    }

    if ( (insentry->type != 'I') && (insentry->type != 'i') &&
	 (insentry->type != 'N') && (insentry->type != 'n') )
    {
	sprintf(invTcbLine,"          class = apply,inventory,%s\n",lppOption); 
	printLine (invTcbLine, tcb_sw);

    }
    else
    {
	sprintf(invTcbLine,"          class = inventory,%s\n",lppOption); 
	printLine (invTcbLine, tcb_sw);
    }

    switch (insentry->type) {
	case 'I':
	case 'i':
	case 'V':
	case 'v':
	    sprintf(invTcbLine,"          size = VOLATILE\n"); 
	    printLine (invTcbLine, tcb_sw);
	    sprintf(invTcbLine,"          checksum = VOLATILE\n\n");
	    printLine (invTcbLine, tcb_sw);
	    break;
	case 'F':
	case 'f':
	    if ( size )
	    {
		sprintf(invTcbLine,"          size = %d\n",size);
		printLine (invTcbLine, tcb_sw);
	    }
	   /*
            *-----------------------------------------------------------
	    * NOTE: 
	    *    If the size is zero (file was not found in the 
            *    ship tree), no size attribute will be generated. 
	    *-----------------------------------------------------------
            */
	    sprintf(invTcbLine,"          checksum = %s\n\n",cksum); 
	    printLine (invTcbLine, tcb_sw);
	    break;
	case 'N':
	case 'n':
	   /*
            *-----------------------------------------------------------
	    * NOTE: 
	    *    Empty attributes is causing installp problems (175091).
            *    Therefore, this case acts the same as the default now.
	    *-----------------------------------------------------------
            */
	default:			/* no size or checksum lines */
	    sprintf(invTcbLine,"\n");
	    printLine (invTcbLine, tcb_sw);
	    break;
    }
    return rc;
}

void
printLine (char *line, int tcb_sw)
{
	fprintf (opinv_ptr,"%s",line);

	if ( tcb_sw )
		fprintf (optcb_ptr,"%s",line);
}

/*------------------------------------------------------------------------
| If the -t<tablename> exist (optional) the file gid and uid is obtained  |
| from a user supplied table <tablename>.                           	  |
 ------------------------------------------------------------------------*/
permTable (InsEntry *insentry,int permtable, char *p_name, char *g_name)
{
   struct Table_Nam *tab_ptr;
   static struct passwd *pass_ptr;
   static struct group *group_ptr;
   int rc = 0;
   int uidFound=0, gidFound=0;
   bzero(p_name,10);
   bzero(g_name,10);
   if ( permtable ) 
   {
      tab_ptr = (struct Table_Nam *) malloc (sizeof (struct Table_Nam));
      tableread(insentry,tab_ptr,&uidFound,&gidFound);
   }
   if (!uidFound)
   {
      if (pass_ptr=getpwuid(insentry->uid))
         strcpy(p_name,pass_ptr->pw_name);
      else
      {    
	    inserror (msgArray[CantResolveUid], insentry->uid, 
		      insfilename) ;
	    rc = ADE_ERROR;
      }
   }
   else
      strcpy(p_name,tab_ptr->idname);
    
   if (!gidFound)
   {
      if (group_ptr=getgrgid(insentry->gid))
         strcpy(g_name,group_ptr->gr_name);
      else
      {
	    inserror (msgArray[CantResolveGid], insentry->gid, 
		      insfilename) ;
	    rc = ADE_ERROR;
      }
   }
   else
      strcpy(g_name,tab_ptr->groupname);
    
   free(tab_ptr);
   return rc;
}

/*---------------------------------------------------------------
| NAME:  printMode
|
| DESCRIPTION:  Print file mode to inventory and/or tcb file.
|    Check for suid and sgid bits and print them out
|    as suid,sgid,mode if there.  Exclusive or (^) is used to clear
|    suid and sgid bits out of the mode before printing it.
|
| PRE CONDITIONS:  Inventory file and Tcb file are opened for
|    write access.
|
| POST CONDITIONS:  The "mode = " attribute line is printed to
|    the inventory and tcb files.
|
| PARAMETERS:  insentry - ptr to structure containing inslist entry
|    info
|              tcb_sw - set if the mode should be printed to the
|    tcb file as well as the inventory file.
|
| NOTES:  tcb_sw is sometimes set to write an entry to the .tcb
|    file because it is an SUID or SGID file and not because it
|    is a tcb file.  Therefore before writing TCB on the mode
|    line check the inslist entry tcbFlag and not just tcb_sw.
|
| DATA STRUCTURES:  InsEntry structure
|
| RETURNS:  void
-----------------------------------------------------------------*/

void printMode(InsEntry *insentry, int tcb_sw)
{
    char invTcbLine[ADE_BUFSIZE];
    char tmpLine[ADE_BUFSIZE];
    int newMode = insentry->mode;

    /*-----------------------------------------------------------
    | Don't use tcb_sw for adding TCB to mode line because	|
    | some entries in the .tcb file are there because they	|
    | are SUID or SGID programs and not because they are TCB	|
    | files.							|
    -----------------------------------------------------------*/
    if (insentry->tcbflag == 'Y')
	strcpy(tmpLine,"          mode = TCB,");
    else
	strcpy(tmpLine,"          mode = ");

    if ( insentry->mode & S_ISUID )
    {
	strcat(tmpLine,"SUID,");
	newMode ^= S_ISUID;
    }
	
    if ( insentry->mode & S_ISGID )
    {
	strcat(tmpLine,"SGID,");
	newMode ^= S_ISGID;
    }

    sprintf(invTcbLine,"%s%o\n",tmpLine,newMode); 
    printLine (invTcbLine, tcb_sw);
}

/*-----------------------------------------------------------------------
| Return the inventory file type string given the character type	|
| from the inslist.							|
-----------------------------------------------------------------------*/
char *getyp(InsEntry *insentry)
{
    switch(insentry->type)
    {
	case 'F': case 'A': case 'I': case 'V': case 'N':
        	return ("FILE");
	case 'f': case 'a': case 'i': case 'v': case 'n':
		return ("FILE");
	case 'D': case 'd':
		return ("DIRECTORY");
	case 'B': case 'b':
		return ("BLK_DEV");
	case 'C': case 'c':
		return ("CHAR_DEV");
	case 'S': case 's':
		return ("SYMLINK");
   }
   return (NULL);
}

/*-----------------------------------------------------------------------
| Read the lpp file for the lpp option name.  The first non-comment,	|
| non-blank line of the file should contain the option name.		|
| The return value indicates whether the lpp is bootable.  If the       |
| second field (quiesce char) is b or B the lpp is bootable.            |
-----------------------------------------------------------------------*/

int readlp(char *lppFile, char *lppOption)
{
	FILE *lppPtr;
	char lp_rec[ADE_BUFSIZE];
	char *ptr;
	char *bflag;

	lp_rec[0] = NULL;
	lppPtr = openFile(lppFile,"r");
	stripComments (lppPtr,lp_rec);
      	if ((ptr=strtok(lp_rec," \t")) != NULL)
	{
		bflag=strtok(NULL, " \t");
        	strcpy(lppOption,ptr);
	}
	else
		fatal (msgArray[CantGetOption], lppFile);

	fclose(lppPtr);
	if ( bflag[0] == 'b' || bflag[0] == 'B' )
		return(1);
	else
		return(0);
}

int tableread(InsEntry *insentry, struct Table_Nam *t_ptr, int *uidFound, int *gidFound)
{
   int idnum;
   char idName[IDSIZE+1];
   int groupnum;
   char groupName[IDSIZE+1];
   char entryLine[ADE_BUFSIZE];

   bzero(idName,IDSIZE+1);
   bzero(groupName,IDSIZE+1);
   rewind(perm_ptr);
   while (stripComments(perm_ptr,entryLine) != EOF)
   {
      if (sscanf(entryLine,"%d %s %d %s",&idnum,idName,&groupnum,groupName) == 4)
      {
         if ((idnum == insentry->uid) && (*uidFound))
		warning (msgArray[MultipleUid], insentry->uid, insfilename,
			 tablename);
         if ((idnum == insentry->uid) && (!*uidFound))
         {
            *uidFound = ++*uidFound;
            strncpy(t_ptr->idname,idName,IDSIZE+1);
         }
         if ((groupnum == insentry->gid) && (*gidFound))
		warning (msgArray[MultipleGid], insentry->gid, insfilename,
			 tablename);
         if ((groupnum == insentry->gid) && (!*gidFound))
         {
            *gidFound = ++*gidFound;
            strncpy(t_ptr->groupname,groupName,IDSIZE+1);
         }

         if (*uidFound && *gidFound)
            return;

         bzero(idName,IDSIZE+1);
         bzero(groupName,IDSIZE+1);
         continue;
      }
      else
      {
	    warning (msgArray[InvalidTableEnt], tablename, entryLine);
	    continue;
      } 
   }

   return;
}

/*-----------------------------------------------------------------------
| chktype returns 0 if the entry is not an inventory only file.		|
| Inventory only (type I(i)) files are not found in the ship trees	|
| so they are not backed up to the media and should not be in the	|
| apply list.								|
-----------------------------------------------------------------------*/
int chktype(InsEntry *insentry)
{
	switch(insentry->type)
	{
	    case 'F': case 'D': case 'A': case 'V':
	    case 'f': case 'd': case 'a': case 'v':
        	return (0);
		break;
	}
	return (-1);
}


/*-----------------------------------------------------------------------
| Determine object repository and savespace directories from command	|
| line flags.								|
-----------------------------------------------------------------------*/

void getInstDirs (int rFlag, int DFlag)
{
	*reposDir = NULL;

	if (rFlag)	
	{
	    strcpy(reposDir, "/etc/objrepos");  
	    strcpy(savespaceDir, "/lpp/SAVESPACE");
	}

	if (DFlag)	
	{
	    strcpy(reposDir, "/usr/share/lib/objrepos"); 
	    strcpy (savespaceDir, "/usr/share/lpp/SAVESPACE");
	}
	    
	if ( *reposDir == NULL )
	{
	    strcpy(reposDir, "/usr/lib/objrepos");
	    strcpy (savespaceDir, "/usr/lpp/SAVESPACE");
	}
}

/*-----------------------------------------------------------------------
| Get the directory name for file entries.  The size is calculated by	|
| directory.								|
-----------------------------------------------------------------------*/
void
getDirectoryName (char *DirName, char *objectName, char type)
{
    char *ptr;

    strcpy (DirName, objectName);

    if ( type == 'D' || type == 'd' )
	return;

    ptr = strrchr(DirName,'/');
    if ( ptr != DirName )
	*ptr = NULL;
    else
	*(++ptr) = NULL;
}

/*---------------------------------------------------------------
| Check the size of the file.  If it is zero unlink it.		|
---------------------------------------------------------------*/
checkSize ( char * fileName )
{
    struct stat statbuf;

    if ( stat (fileName, &statbuf) == -1 )
    {
	inserror (msgArray[StatFailed], fileName, errno);
	return (ADE_ERROR);
    }

    if ( statbuf.st_size == 0 )
	unlink (fileName);

    return 0;
}
		
/*-----------------------------------------------------------------------
| Print links to file and update size info.  Also update              	|
| the repository and security directories to include the link.		|
-----------------------------------------------------------------------*/
void print_links(InsEntry *insentry, int tcb_sw)
{
	int i, link_len=0;
	char DirName[MAXPATHLEN+1];
	char invTcbLine[ADE_BUFSIZE];

	sprintf(invTcbLine,"          links = ");
	printLine(invTcbLine, tcb_sw);
	for (i=0; i < insentry->numHardLinks; i++)
	{
		link_len=strlen(insentry->hardLinks[i]);
		sprintf(invTcbLine,"%s",insentry->hardLinks[i]);
		printLine(invTcbLine, tcb_sw);
		getDirectoryName (DirName, insentry->hardLinks[i], insentry->type);
		updateSize(DirName, link_len, 'H', 0);
		updateSize(reposDir, link_len, NULL, 0);
		if (tcb_sw)
   	 	   updateSize(secDir, link_len, NULL, 0);
		if (i != (insentry->numHardLinks-1) )
		{
			sprintf(invTcbLine,",");
			printLine(invTcbLine, tcb_sw);
		}
	}
	sprintf(invTcbLine,"\n");
	printLine(invTcbLine, tcb_sw);
}

