static char sccsid[] = "@(#)43 1.15 src/bldenv/pkgtools/adecopyright.c, pkgtools, bos41J, 9519A_all 5/5/95 10:10:31";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: create_cr
 *              main
 *              proc_cr_master
 *              proc_subst_data
 *              replace_PROG
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

/**********************************************************************
* The adecopyright command will use the .cr files specified on the    *
* command line to generate a temporary composite file of unique       *
* sorted keys.  (Except for the IBM copyright key which must be first *
* in the list.)  The unique keys will be expanded into copyright text *
* using the copyright master file *.ma specified on the command line. *
* The resulting file of copyright text will be written to stdout.     *
**********************************************************************/

#include "adecopyright.h"
#include "ade.h"
#include <errno.h>

char* replace_PROG(char*, char*);

extern int optind;
extern int errno;
extern char *optarg;

/*
 *-------------------------------------------------------------
 * Module level Variable Definitions
 *-------------------------------------------------------------
 */
char fileline[MAXSUBLINE+1];
char blank[MAXSUBLINE+1];
char masterfile[MAXPATHLEN+1];

static char compidsline[MAXIDLINE+1];
static int  noIdAction = 0 ;		/* Flag for action to take if	*/
                                        /* missing a compid (0=abort)   */

static int  IBM_versions = 1;		/* Use IBM Headers & Footers?   */
static int  OEM_versions = 0;		/* Use OEM Headers & Footers?   */

main(int argc, char **argv)
{
   FILE *key_output;
   FILE *keys_unique;
   char cr_keys[MAXPATHLEN+1];
   char uncr_keys[MAXPATHLEN+1];
   char sort_keys[MAXNUMKEYS][MAXKEYSIZE+1];
   int c, count = 0, upcount = 0;
   int pid, i;
   char *lpp_name;
   char compidsfile[MAXPATHLEN+1];
   char *key;
   char ibmkey[MAXKEYSIZE+1];
   char *ptr;

   ibmkey[0] == '\0';
   masterfile[0] == '\0';
   lpp_name[0] == '\0';
   compidsfile[0] == '\0';
   
   while ((c=getopt(argc, argv, "aocf:l:t:")) != EOF) {
      switch(c) {
        case 'c':
          /* continue processing if missing a compid */
	  noIdAction = 1 ;		
	  break ;
	case 'f':
	  strcpy(masterfile,optarg);
	  break;
        case 'l':
	  lpp_name = optarg;
	  break;
        case 't':
	  strcpy(compidsfile,optarg);
	  break;
	case 'o':
	  IBM_versions = 0;		
	  OEM_versions = 1;		
	  break ;
	case 'a':
	  IBM_versions = 1;		
	  OEM_versions = 1;		
	  break ;
	default:
	  usage();
      }
   }
   /*---------------------------------------------------------------
   | Verify that all required flags were provided on command line. |
   ---------------------------------------------------------------*/

   if (!masterfile[0] || !lpp_name[0] || !compidsfile[0])
      usage();

   if (optind == argc)      /* check for file list */
        usage();

   bzero(cr_keys,MAXPATHLEN+1);
   bzero(uncr_keys,MAXPATHLEN+1);
   pid = getpid();
   sprintf(cr_keys,"cr_keys.%u",pid);     /* cr_keys are a concatination of all .cr files */ 
   sprintf(uncr_keys,"uncr_keys.%u",pid); /* uncr_keys are a file of unique keys */ 

   key_output = openFile(cr_keys, "+");

   keys_unique = openFile(uncr_keys, "+"); 

   /*******************************************************
   * Process each .cr file in the command line file list. *
   *******************************************************/

   i = optind;
   while (i < argc) {
	if (create_cr(argv[i], cr_keys, key_output))
	   exit(1);
	i++;
   }

   /**********************************************************
   * Put all keys in an array for sorting and sort the keys. *
   **********************************************************/

   rewind (key_output);
   while (fgets(sort_keys[count], MAXKEYSIZE, key_output) != NULL) {
      count = count + 1;
   }
   
   qsort ( (char *) sort_keys, count, (MAXKEYSIZE + 1), compare);

   /****************************************
   * Write only the unique keys to a file  *
   * excluding the IBM key which is saved. *
   ****************************************/

   key = strtok(sort_keys[upcount++], "\0");
   while ( (count--) > 0) {
      /* compare keys excluding the \n and the last character of */
      /* the key which should be unique, such as a, b, c...      */
      if (strncmp(key, sort_keys[upcount++], ((strlen(key) -2)) ) == 0 )
         continue;
      else {
         if (strncmp(key, KEYIBM, strlen(KEYIBM)) != 0 )
            {
               if ((fputs(key, keys_unique)) == EOF)
                  fatal(keyWriteErr, uncr_keys);
            }
         else {
            if (ibmkey[0] == '\0')
               strcpy(ibmkey, key); 
            else {
               if ( (strcmp(key, ibmkey)) < 0)
                  strcpy(ibmkey, key); 
            }
         }  
         key = strtok(sort_keys[upcount -1], "\0");
         continue;
      }
   }

   /*************************************
   * Put all unique keys into an array. *
   *************************************/

   rewind(keys_unique);
   count = 0;
   upcount = 0;
   while (fgets(sort_keys[count], MAXKEYSIZE, keys_unique) != NULL) {
      count = count+1;
   }

   /**********************************************************
   * Write the unique keys to a file with the IBM key first. *
   **********************************************************/

   rewind(keys_unique);
   if ((fputs(ibmkey, keys_unique)) == EOF)
      fatal(keyWriteErr, uncr_keys);
   while (upcount != count) {
      if ((fputs(sort_keys[upcount++], keys_unique)) == EOF)
         fatal(keyWriteErr, uncr_keys);
      continue;
   }
   proc_cr_master(keys_unique, lpp_name, compidsfile);
    
   fclose(key_output);
   fclose(keys_unique);
   unlink(cr_keys);
   unlink(uncr_keys);
   exit(0);
}

/*****************************************************************
* Name: create_cr                                                * 
*                                                                *
* Function: creates a composite file of keys from the .cr files. *
*****************************************************************/

create_cr(char *arg, char cr_keys[MAXPATHLEN+1], FILE *key_output)
{
   FILE *cr_file;

   cr_file = openFile(arg, "r");

   while (fgets(fileline,MAXSUBLINE,cr_file) != NULL) {

        /*************************
        * Check .cr file entries *
        *************************/

	if (fileline[0] == '#') /* Allow for comment lines */
		continue;

	if (sscanf(fileline,"%s",blank) != 1) /* Allow for blank lines */
		continue;

        if (!(strstr(fileline,KEYPATTERN)))
           fatal(keyPatternErr, arg, fileline, KEYPATTERN);
        if ((fputs(fileline,key_output)) == EOF)
           fatal(keyWriteErr, cr_keys);
   }
   if ((ferror(cr_file) != 0))
      fatal(crReadErr, arg); 

   fclose(cr_file);
   return 0;
}

/*********************************************************************
* proc_cr_master                                                     *
*                                                                    * 
* Function: process copyright.ma file                                *
*                                                                    *
* Process the substitution data.  The substitution data file is      * 
* expected to consist of a comments section, a header section, pairs *
* of substitution keywords and substitution strings, and a footer    *
* section.  The keywords are expected to begin with a designated key *
* pattern, and the substitution strings are double-quoted.  Anything *
* within the outer double-quotes is taken as the replacement text.   *
*                                                                    *
*  ASSUMPTIONS:							     *
*    - The .ma file can have keys in any order			     *
*    - There is no guarantee that the HEADER will be 1st, then the   *
*      CR keys, and finally the FOOTER -- basically, the master file *
*      contols the order that things will be put into the copyright  *
*      file; if the master file order is wrong, the copyright file   *
*      order will be wrong.  					     *
*    - proc_subst_data will leave the file location of the master    *
*      file after the KEYS keyword (if it does not, you could get    *
*      stuck in an infinite loop).  				     *
*    - Only PROG#'s inside active headers will be substituted        *
*    - Duplicate PROG# lines are not used			     *
*********************************************************************/
/* The following is a sample copyright master file.                  *
**********************************************************************
* #                                                                  *
* # comment lines                                                    *
* #                                                                  *
* HEADER                                                             * 
*                                                                    * 
*  header lines                                                      *
*                                                                    *
* KEYS                                                               *
* %%_IBMa  "   (C) Copyright International Business Machines 1992."  *
* %%_ATTa  "   (C) Copyright AT&T 1984, 1985, 1986, 1987, 1989."     *
* %%_KEYa  "   (C) Copyright  Key Corporation, Ltd. 1991."           * 
* FOOTER                                                             *
*                                                                    * 
*  footer lines                                                      *
*                                                                    *
*********************************************************************/

proc_cr_master(FILE *keys_unique, char *lpp_name, char compidsfile[MAXPATHLEN+1])
{
   FILE *ma_file;
   char copyright[MAXPATHLEN +1];
   char *progline;
   char *putline;
   char repline[MAXIDLINE+1];
   char   token[MAXIDLINE+1];
   int end = 0;
   int in_active_header = 0;
   int printed_progno = 0;

   ma_file = openFile(masterfile, "r");

   /*******************************************************
   * Process copyright.ma file and create copyright file. *
   *******************************************************/

   if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
   while(!(feof(ma_file))) { /* Read the entire ma_file */

	if(is_token(token)) { /* Got a token, handle it */

	  if (!(strcmp(KEYOEMHEADER,token))) {
	    sprintf(token,"");
   	    if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    while(!(feof(ma_file))&&(!(is_token(token)))) {
	      if (OEM_versions) {
		in_active_header = 1;
		printf("%s\n",fileline);
	      }
   	      if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    }
	    if(strcmp(KEYPROG,token)) in_active_header=0;
	  }

	  if (!(strcmp(KEYHEADER,token))) {
	    sprintf(token,"");
   	    if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    while(!(feof(ma_file))&&(!(is_token(token)))) {
	      if (IBM_versions) {
		in_active_header = 1;
		printf("%s\n",fileline);
	      }
   	      if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    }
	    if(strcmp(KEYPROG,token)) in_active_header=0;
	  }

	  if (!(strcmp(KEYOEMFOOTER,token))) {
	    sprintf(token,"");
   	    if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    while(!(feof(ma_file))&&(!(is_token(token)))) {
	      if (OEM_versions) printf("%s\n",fileline);
   	      if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    }
	  }

	  if (!(strcmp(KEYFOOTER,token))) {
	    sprintf(token,"");
   	    if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    while(!(feof(ma_file))&&(!(is_token(token)))) {
	      if (IBM_versions) printf("%s\n",fileline);
   	      if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
	    }
	  }

	  if (!(strcmp(KEYKEYS,token))) {
            in_active_header = 0;
	    sprintf(token,"");
            proc_subst_data(ma_file, keys_unique);
	  }

	  if (!(strcmp(KEYPROG,token))) {
	    sprintf(token,"");
   	    if (scan_line(ma_file,fileline)) sscanf(fileline,"%s",token);
            if ((in_active_header)&&(!(printed_progno)))
	    {
              if (progline = replace_PROG(lpp_name,compidsfile))
	      {
		  printf(" %s\n",progline);
	      } else {
		  printf(" PROG#\n",progline);
	      }
	      printed_progno = 1;
	    }
	    in_active_header=0;
	  }

	} else { /* Not a token, probably comments to eat up at beginning */
   	  scan_line(ma_file,fileline);
   	  sscanf(fileline,"%s",token);
	}
   }
   fclose(ma_file);
   return 0;
}
int
is_token(char *token)
{
	if((strcmp(KEYOEMHEADER,token))
	  &&(strcmp(KEYHEADER,token))
	  &&(strcmp(KEYOEMFOOTER,token))
	  &&(strcmp(KEYFOOTER,token))
	  &&(strcmp(KEYKEYS,token))
	  &&(strcmp(KEYPROG,token))) {
	    return(0);
	  } else {
	    return(1);
	  }
}

#ifdef GENDEF
/********************************************************************
* This function opens a defect against the component pkg, sev 2     *
* if an entry for the package  was not found in the compids table.  *
********************************************************************/
void
create_defect(char *productid)
{
   int rc = 0;
   char buf[MAXBUFSIZE+1];
   struct stat statbuf;

   errno = 0;
   stat("/usr/local/cmvc/bin", &statbuf);
   if (errno == ENOENT )
     sprintf(buf, "/afs/austin/local/bin/Defect -open -component pkg -severity 2 -phaseFound code -remarks \" An entry for %s was not found in the compids table. Add this entry to the compids table.\" -verbose >& 2", productid);
   else
     sprintf(buf, "/usr/local/cmvc/bin/Defect -open -component pkg -severity 2 -phaseFound code -remarks \" An entry for %s was not found in the compids table. Add this entry to the compids table.\" -verbose >& 2", productid);
   rc = system(buf);
   if (rc != 0)
   fprintf(stderr, "\nDefect open failed. A defect needs to be opened manually against component pkg, severity 2, to add an entry for %s to the compids table.", productid);
   fprintf(stderr, "\n");
}
#endif


/**********************************************************************
*  replace_PROG - Replace PROG# with component ID from compids.tble   *
*                                                                     *
* Searches the compids.table file for the component ID associated     *
* with the LPP name.  This ID is placed in the copyright file.        *
* If no match is found in compids.table then return NULL.	      *
**********************************************************************/
char*
replace_PROG(char *lpp_name, char compidsfile[MAXPATHLEN+1])
{
   FILE *compids_ptr;
   char *productName;
   char *productID;
   int endofile = 0;

   /*------------------------------------------------------------
   | Open compids.table file and search for product entry.	|
   | compids file has some comment lines beginning with '%' and	|
   | '*' so ignore those lines when looking for product names.	|
   ------------------------------------------------------------*/
   compids_ptr = openFile(compidsfile,"r"); 
   
   while ((endofile = (stripComments(compids_ptr, compidsline))) != EOF)
   {
      if ( (compidsline[0] == '*') || (compidsline[0] == '%') )
	continue;
      productName = strtok(compidsline,":");
      if (!strcmp(productName,lpp_name))
      {
         break;      
      }
      continue;
   }
   fclose(compids_ptr);

  /*
   *--------------------------------------------------------
   * If at EOF, then missing compid.
   * Action to take depends upon the flags at invocation.
   * Action defined in the noIdAction variable.
   *--------------------------------------------------------
   */
   if (endofile == EOF)
   {
      if (noIdAction == 0)
      {
#ifdef GENDEF
	  create_defect(lpp_name);
#endif
	  fatal(noIDinTable, lpp_name); 
	 /*
	  *------------------------------------------------
	  * NOTE: as long as fatal is used above, the
	  *       following lines are not needed.  However,
	  *       if fatal is changed to error, then we
	  *       need to return something.
	  *------------------------------------------------
	  */
	  compidsline[0] = '\0' ;
	  return(compidsline) ;
      }
      else
      {
	  warning(noIDinTable, lpp_name) ;
	  return (NULL);
      }
  } /* END missing compid special handling */

 /*--------------------------------------------------------------
  | Break the compids entry out like normal and return.		|
  -------------------------------------------------------------*/
  productID = strtok(NULL,":");
  return(productID) ;
}

/*---------------------------------------------------------------
| NAME: proc_subst_data
|
| DESCRIPTION: For each key listed in the unique key file, search
|	the master copyright file for a match.  Write to stdout
|	all text for the matching key.  The text is delimited by
|	quotes "" and may be on one line or it may span multiple
|	lines.
|
| PRE CONDITIONS:  The file pointed to by the keys_unique parameter
|	contains a set of unique keys to be expanded (duplicates
|	have been eliminated).
|
| POST CONDITIONS:  This function will write to stdout the text
| 	for each key in the unique key set.
|
| PARAMETERS:
|	keys_unique - file pointer to set of unique keys
|	ma_file - file pointer to master copyright file.  This
|		is the file that contains the expansion text.
|
| NOTES:
|	fileline is a global array.
|	See comments below for sample substitution file.
|
| DATA STRUCTURES:  none
|
| RETURNS:  0 if all keys were found.  Fatally exits if a key is not
|	found in the master file or if the key text is incomplete
|	(i.e. unexpected end of file).
-----------------------------------------------------------------*/

/**********************************************************************
*  The following is a sample substitution file.                       *
***********************************************************************
* #                                                                   *
* # comment lines                                                     *
* #                                                                   *
* HEADER                                                              * 
*                                                                     * 
*  header lines                                                       *
*                                                                     *
* KEYS                                                                *
* %%_IBMa  "   (C) Copyright International Business Machines 1992."   *
* %%_ATTa  "   (C) Copyright AT&T 1984, 1985, 1986, 1987, 1989."      *
* %%_KEYa  "   (C) Copyright  Key Corporation, Ltd. 1991."            * 
* FOOTER                                                              *
*                                                                     * 
*  footer lines                                                       *
*                                                                     *
***********************************************************************
* The following is a sample key file.                                 *
***********************************************************************
* #                                                                   *
* # comment lines                                                     *
* #                                                                   *
*                                                                     * 
* %%_IBMa                                                             *
* %%_KEYa                                                             *
**********************************************************************/

proc_subst_data(FILE *ma_file, FILE *keys_unique)
{
   char *begquote;
   char *endquote;
   int end;
   char keyline[MAXKEYSIZE+1];

   rewind (keys_unique);

   /*****************************************************
   * Match keys in master copyright file and substitute *
   * copyright strings.                                 *
   *****************************************************/

    while ( (stripComments (keys_unique, keyline)) != EOF )
    {
	rewind (ma_file);
	end = 0;
	while ( (stripComments(ma_file, fileline)) != EOF )
	{
	    /***********************************************
	    * Search for FOOTER (end of substitution data) *
	    ***********************************************/

	    if (strstr(fileline,KEYFOOTER))
		fatal(noKeyCrFile, keyline, masterfile);

	    /****************************
	    * Check master file entries *
	    ****************************/

	    if (!(strstr(fileline,KEYPATTERN)))
		continue;
                  
	    /*-------------------------------
	    | No match.  Continue.		|
	    -------------------------------*/
	    if ( strncmp(fileline, keyline, strlen(keyline)) )
		continue;

	    /*---------------------------------------------------
	    | Write the copyright text to stdout.  Some text	|
	    | will span multiple lines.				|
	    ---------------------------------------------------*/

	    if ( !(begquote = strchr(fileline,'"')) )
		fatal(quoteError1, masterfile, fileline);    
	    endquote = strrchr(fileline,'"');

	    /*---------------------------
	    | Single line copyright.    |
	    ---------------------------*/
	    if (begquote != endquote)
	    {
		*endquote = NULL;
		end++;
	    }

	    begquote++;
	    puts(begquote);		/* Puts adds a newline */

	    /*---------------------------
	    | Multiline copyright.	|
	    ---------------------------*/
	    while ( !end )
    	    {
		if ( stripComments(ma_file, fileline) != EOF )
		{
		    if (strstr(fileline, KEYPATTERN))
			fatal(quoteError2, masterfile, keyline); 
		    if ( (endquote = strchr(fileline,'"')) ) 
		    {
			*endquote = NULL;
			end++;
		    }
		    puts (fileline);	/* Puts adds a newline */
		}
		else
		    fatal(quoteError2, masterfile, keyline); 
	    }
	    break;
	}
    }

    /*---------------------------------------------------
    | If end is not set we did not find a match.	|
    ---------------------------------------------------*/
    if (!end)
	fatal(noKeyCrFile, keyline, masterfile);

   return 0;
}

/*---------------------------------------------------------------
| NAME:  stripComments
|
| DESCRIPTION:  Read a line from an input file and return
|	the first non-comment, non-blank line.  Comments are
|	considered to begin with #.	
|
| PRE CONDITIONS:  Calling program has allocated space for
|	line buffer.
|
| POST CONDITIONS:  line parameter contains text of the first
|	line from the input file that is not a comment or blank
|	line.
|
| PARAMETERS:
|	insfp - valid file pointer into input file.
|	line - buffer for reading text from input file.
|
| NOTES:  requires ADE_BUFSIZE set in ade.h file.
|
| DATA STRUCTURES:  none
|
| RETURNS:  EOF for end of file, otherwise 0.
-----------------------------------------------------------------*/
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
		*num = NULL;
	return 0;
}

/*------------------------------
| compare subroutine for qsort |
------------------------------*/
int
compare (char *a, char *b)
{
   return(strcmp(a,b));
}
int
scan_line(FILE *fp,char fileline[MAXSUBLINE+1])
{
	int i=0;
	char c;

	c=fgetc(fp);
	while((!feof(fp))&&('\n'!=c)) {
	  fileline[i++]=c;
	  c=fgetc(fp);
	}
	fileline[i]='\0';
	return(i);
}
