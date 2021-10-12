/* @(#)07	1.2  src/bldenv/pkgtools/mkodmextract.c, pkgtools, bos412, GOLDA411a  2/1/94  10:35:56 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: fatal
 *		getdescriptor
 *		getfield
 *		getlineobj
 *		print_odm_string
 *		main
 *		usage
 *		verify
 *		vnoupdate
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <varargs.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mkodmextract.h"


extern char *  optarg;

/*---------------------------------------------------------------------------
| fatal accepts a variable length argument list which is a message          |
| format string and any arguments to the error message.  The arguments      |
| are combined and displayed to stderr and fatal exits with a -1.           |
|                                                                           |
----------------------------------------------------------------------------*/

void
fatal (va_alist)
{
    va_list ap;
    char *msgFormat;

    fprintf (stderr,"%s:  FATAL ERROR:\n",COMMANDNAME);

    va_start(ap);
    msgFormat = va_arg(ap, char *);
    vfprintf (stderr, msgFormat, ap);
    va_end(ap);

    fprintf (stderr,"\n\tTerminating.\n");
    exit (-1);
 }

/*-------------------------------------------------------------------------------------- 
| This function prints the usage of the mkodmextract command.                          |
--------------------------------------------------------------------------------------*/

void
usage ()
{
 fprintf (stderr,Usage,COMMANDNAME);
 exit (-1);
}

/*--------------------------------------------------------------------------- 
| This function takes an input line and returns the value of descriptor     |
| in buf.                                                                   |
----------------------------------------------------------------------------*/

void 
getdescriptor( char *fline , char * buf)
{
   int i = 0;
   char *first = NULL; /* pointer to get the value of descriptor fron the stanza */
   char *descr = NULL; /* points to the descriptor in the stanza.*/
   
   /* clear the buffer */
    buf[0] = 0;

 /* get the descriptor from the line */
  if ( (descr=strchr(fline, ':')) != NULL )/* get the position of : in the stanza */
   {
     first=fline;
     i = 0;
     while (isspace(*first))  first++; /* skip the leading white spaces*/
     while( first != descr ) /* extract the value of descriptor in buf */
       {
         buf[i] = *first;
         i++;
         first++;
       }/* while*/
        buf[i] = 0;/* end of string */
    }/* if */
}


/*---------------------------------------------------------------------------
| This function takes the descriptor from the stanza and verifies that it is|
| a valid descriptor by looking in the object class table. This function    |
| return 1 if the match for the descriptor is found in the objclassdb, else |
| returns 0.                                                                |
---------------------------------------------------------------------------*/

char 
getlineobj(char* fdescr, char * oline, FILE *objclassdbfp)
{
  char odescr[BUFSIZE];
       while ( fgets(oline, OBJSIZE, objclassdbfp) != NULL )/* get each line from the objclassdb */
        {
	    /* skip any comments in objclassdb */
             if ( strstr(oline, "#") ) 
              continue;
           else
           {
              /* extract the descriptor value fron objclassdb line */
              getdescriptor(oline, odescr); 
              if (!strcmp(fdescr, odescr) ) /* look for match in objclassdb for the descriptor */
                 return(1);
          } /* else */
    }
    /* no match found */
    return(0);
}

/*--------------------------------------------------------------------------
| This function gets the fields and the keys associated with an object     |
| from the lines in the objclassdb. It takes tha field seperator, the line |
| from the objclassdb and writes the fields or keys in fld. It returns     |
| a pointer to the next field or key in the line.                          |
---------------------------------------------------------------------------*/
char *
getfield(char *fline, char sep, char * fld)
{
 char c;
 char *fptr;
 int i = 0;

   fptr = fline; /* get the field value */
   /* skip all the leading white spaces and initial separators*/
   while  (*fptr)
    if (isspace(*fptr) || (*fptr == sep)) fptr++;
    else break;

   i=0;
   /* while next seperator, white space or end of line not found*/
   while((*fptr != sep) && (!isspace(*fptr)) && (*fptr != 0))
   { /* get the value of field */
   fld[i] = *fptr;
   i++;
   fptr++;
   }
   fld[i]=0; /* end of string */
   return (fptr);/* return the pointer*/
}

/*---------------------------------------------------------------------------
| This function verifies the fields or keys from each line in the stanza.   |
| It returns 1 if a match is found else return 0.                           |
----------------------------------------------------------------------------*/

char 
verify(char * fkey, char * farr, char sep)
{
  char ofield[BUFSIZE];

  
       /* get the fields from the fields string in the objclassdb  seperated by sep */
         farr = getfield(farr, sep, ofield); 
         while  (strlen(ofield) != 0) /* while not end of fields string */
          {
            if (!strcmp(ofield, fkey)) /* look for a match in the objclassdb */
               return(1); /* match found*/
         farr = getfield(farr, sep, ofield); /* get the next field */
         /* fprintf(stdout, "the ofield is %s\n", ofield); */
          }/* while*/
       
       return(0);
}

/*---------------------------------------------------------------------------
| This function verifies that the object class can not be updated.          |
----------------------------------------------------------------------------*/
void
vnoupdate( char * fkeyarr, char * fdescr)
{
  int fval = 0;


     fval=atoi(fkeyarr);/* change the value to an integer */
     if ( fval == 99 ) /* check for the object classes that can not be updated*/
      {
      fatal(Noupdate_Object_Class,COMMANDNAME,fdescr,errno);
       }
 }

/*---------------------------------------------------------------------------
| NAME:  PRINT_ODM_STRING                                                   | 
|                                                                           |
| FUNCTION:                                                                 |
|                                                                           |
|      prints a string in ODMADD stanza format.                             |
|                                                                           |
| RETURNS:                                                                  |
|                                                                           |
|     Nothing.                                                              |
-----------------------------------------------------------------------------*/

void print_odm_string (string_ptr, output)
char *string_ptr;
FILE *output;
{

  char *substring_ptr;
  char *delimeter_ptr;
  int END_OF_LINE; /* boolean value delimiting end of string_ptr */


  /* initialize variables */

  END_OF_LINE = FALSE;
  substring_ptr = string_ptr;

  fprintf(output, "\'");
  while (!END_OF_LINE)
    {
      delimeter_ptr = mbspbrk (substring_ptr, "\\\"\n");

      if (delimeter_ptr == NULL)
        {
           fprintf(output, "%s", substring_ptr);
           END_OF_LINE = TRUE;
        }
      else
        {

          switch (*delimeter_ptr)
          {

             case '\\':
                 *delimeter_ptr = '\0';
                 fprintf(output, "%s", substring_ptr);
                 fprintf(output, "\\\\");
                 substring_ptr = delimeter_ptr + 1;
                 break;
             case '\"':
                 *delimeter_ptr = '\0';
                 fprintf(output, "%s", substring_ptr);
                 fprintf(output, "\\\"");
                 substring_ptr = delimeter_ptr + 1;
                 break;
             case '\n':
                 *delimeter_ptr = '\0';
                 fprintf(output, "%s", substring_ptr);
                 fprintf(output, "\\n\\\n");
                 substring_ptr = delimeter_ptr + 1;
                 break;
           } /* endswitch */
        } /* endif */
    } /* endwhile */

  fprintf (output, "\'");
}
/*---------------------------------------------------------------------------
| Mainbody: mkodmextract                                                    |
| Purpose: parses input file one line at a time and writes to stdout        |
| an odmdelete command with appropriate parameters.                         |
|                                                                           |
| To execute this command:                                                  |
| mkodmextract -d objclassdb -f stanzafile                                  |
|                                                                           |
| Input -                                                                   |
| objclassdb: table containing object class descriptors, keys and fields.   |
| stanzafile: the input stanza file                                         |
|                                                                           |
---------------------------------------------------------------------------*/
main (int argc, char **argv)
{
        FILE *stanzafp;
        FILE *objclassdbfp;
	int arg;
	char *stanzaFile = NULL ;
        char *objclassdb = NULL;
        char *sdescr; /* pointer to the value of descriptor in the stanza */
	char line[OBJSIZE];  /* to get the lines from the objclassdb */
        int stanza_length = 0;
        char * stanza;  /*stores the stanza from the input stanza file*/
        char terminating_char;
        int skip_spaces;
        char * key;/* pointer to a key or a field in the stanza*/
	char storekey[BUFSIZE];
        char *value;/* pointer to the value of a key or a field*/
	char * odescr; 
	char * okeyarr; 
	char * ofldarr; 
        int isakey, isafield;
	int firsttime;


       /* parse the command line */
	while ( (arg = getopt (argc, argv, "d:f:")) != EOF )
	{
	    switch (arg)
	    {
		case 'd':
			objclassdb = optarg;
			break;
		case 'f':
			stanzaFile = optarg;
			break;
		case '?':
		        usage();
	    }/* switch */
	} /* while */

      /* verify the command line paremeters */
      if ( !strlen (stanzaFile) || !strlen(objclassdb) )
      {
       fprintf(stderr,Missing_Opt,COMMANDNAME);
       usage();
      }

      /* open the input stanza file for read only */
      if ( (stanzafp = fopen(stanzaFile, "r")) == NULL ) 
          fatal(File_Open_Failed,stanzaFile,errno);

      /* open the object class table for read only */
      if ( (objclassdbfp = fopen(objclassdb, "r")) == NULL ) 
          fatal(File_Open_Failed,objclassdb,errno);
       
       
/* get the stanza from the stanza file one at a time */
while( (stanza_length = get_ascii_phrase(stanzafp,STANZA,&stanza)) > 0 )
{

    skip_spaces = 0;
    /* get the descriptor from the stanza */
    sdescr = get_value_from_string(stanza,":\n",skip_spaces,&terminating_char);    
    if(sdescr == (char *)NULL || terminating_char != ':')
     {
           fatal(Descriptor_Not_Found,COMMANDNAME,stanza);
     }
     
     /* get the line from object class */
     if (!getlineobj(sdescr, line, objclassdbfp))
     {
          fatal(Unrecognized_Object_Class, COMMANDNAME,sdescr,stanza,errno);
	  fflush(stderr);
	  }


     /* get the first occurance of : in line: which has the descriptor */
     odescr  = strtok(line,":"); 

    /* get the second occurence of : which has all the keys */
     okeyarr = strtok(NULL,":"); 

    /* get the remaining line - fields*/
     ofldarr = strtok(NULL,"\n"); 


     /* for object classes that can not be updated */
     vnoupdate(okeyarr, sdescr);

     skip_spaces = 1;
     firsttime = 1; /* for printing the odmdelete command */
     while (TRUE)
     {
        /* get the keys from the stanza*/
        /* the function get_value_from_string goes through an ascii string and find a 
           string that ends with a character specified by the second argument to this
           function. Subsequent calls to this function with first argument = NULL 
           gets the values starting from the last value found in the string. 
           This function returns a pointer to the first string which ends with 
           one of the characters in the second argument to the function. It also
           passes back the single character which ended the string.*/

        key = get_value_from_string((char *)NULL,"=\n",skip_spaces,&terminating_char);
        strcpy(storekey, key);
        if (key == (char *)NULL || (*key == '\0' && terminating_char == '\0'))
        {
         break;
        }
        else if (terminating_char != '=')
          {
           fatal(Keyname_Not_Found,COMMANDNAME,stanza);
           }
        isakey=verify(key,okeyarr,'%');/* to print the odmdelete command*/
        isafield = verify(key, ofldarr,',');/* verify if it is a field */
	if (!isafield)
           { /* illegal field or key, so give ERROR and exit */
           fatal(Invalid_Field,COMMANDNAME,key,stanza,errno);
        }

/* get_value_from_string returns the string in double quotes and we need
   to have them in single quotes. */
        value = get_value_from_string((char *)NULL,"\n",skip_spaces,&terminating_char);

        if (isakey){
        /* the following lines just generate the desired format for printing */
        if (firsttime) { /* if starting the odmdelete command */
         fprintf(stdout, "odmdelete -o %s -q %s", odescr,"\"" );
	 fprintf (stdout, " %s = ", storekey);
	 print_odm_string (value, stdout);
         firsttime = 0;
         }
         else /* to append to the odmdelete command */
          {
	 	fprintf (stdout, " AND %s = ", storekey);
	 	print_odm_string (value, stdout);
          }
       }
     } /* while */
	fprintf (stdout, " \" >/dev/null\n");
   rewind(objclassdbfp); /* close objclassdb for next read to start at the beginning
    of the file */

  }/*while*/
 exit(0);
}/*main*/

