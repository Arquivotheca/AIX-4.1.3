static char sccsid[] = "@(#)63  1.25.1.17  src/bos/usr/bin/odmcreate/odmcreate.c, cmdodm, bos411, 9428A410j 5/23/94 09:59:13";

/*
 *   COMPONENT_NAME: CMDODM
 *
 *   FUNCTIONS: MSGSTR
 *		array_error
 *		create_header
 *		error
 *		error1
 *		error2
 *		error3
 *		error4
 *		fclean
 *		find_class_desc
 *		get_descrip_info1149
 *		main
 *		parse_link
 *		parse_simple
 *		read_class_defs
 *		warning
 *		warning1
 *		write_class
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <ctype.h>
#include <mbstr.h>
#include <sys/dir.h>
#include <unistd.h>
#include <odmi.h>

#define MF_ODMCMD       "odmcmd.cat"
#define ODM_MSG_SET     1 /* "NLS SET" */
#define ODM_NCHAR             10
#define ODM_NOTYPE_ERR        5001
#define ODM_NOTARRAY_ERR      5002
#define ODM_ARRAY_ERR         5003
#define ODM_UNKNOWNTYPE_ERR   5004
#define ODM_COLNOTFOUND_ERR   5005
#define ODM_MAXDESC_ERR       5006
#define ODM_INTERNAL_ERR      5007
#define ODM_ARG_ERR           5008
#define ODM_SYNTAX_ERR        5009
#define ODM_BAD_DIM_ERR       5010
#define ODM_NAMELEN_ERR       5011
#define ODM_HDRFILE_ERR       5012
#define ODM_DESCFILE_ERR      5013
#define ODM_APPEND_ERR        5014
#define ODM_CLASSNOTFOUND_ERR 5015
#define ODM_CPP_ERR           5016
#define ODM_INIT_ERR          5017
#define ODM_SRCFILE_ERR       5018
#define ODM_MALLOC_ERR        5019
#define ODM_TERM_ERR          5020
#define ODM_CREATE_FAIL       5022
#define ODM_REMOVE_FAIL       5023
#define ODM_PREREQ_ERR        5024
#define ODM_MAX_CLASSES       5025
#define ODM_CLONE_ERR         5026

#define VLVALUESIZE     256
#define VLCLASSSIZE     16
#define LVALUESIZE      256
#define METHODSIZE      256

#define INVALID -1
#define FALSE 0
#define TRUE 1

#define MAX_DESC 128
#define LINELEN 1024

#define error(a,b) \
        {(void)fprintf(stderr, MSGSTR(a,b));fclean();(void)exit(-1);}
#define error1(a,b,c) \
        {(void)fprintf(stderr, MSGSTR(a,b),c);fclean();(void)exit(-1);}
#define error2(a,b,c,d) \
        {(void)fprintf(stderr, MSGSTR(a,b),c,d);fclean();(void)exit(-1);}
#define error3(a,b,c,d,e) \
        {(void)fprintf(stderr, MSGSTR(a,b),c,d,e);fclean();(void)exit(-1);}
#define error4(a,b,c,d,e,f) \
        {(void)fprintf(stderr, MSGSTR(a,b),c,d,e,f);fclean();(void)exit(-1);}
#define warning(a,b) {(void)fprintf(stderr, MSGSTR(a,b));}
#define warning1(a,b,c) {(void)fprintf(stderr, MSGSTR(a,b),c);}

#include <locale.h>
#include <nl_types.h>
#define MSGSTR(num,str) catgets(catd,ODM_MSG_SET,num-5000,str) /* MSG */
nl_catd catd;

char *malloc();
char get_descrip_info();
/* 
char *strrchr (), *strchr (), *mbschr (); 
*/

static int elems;
int lcl;                /* link-to (std) class */
int ncl;                /* number of classes */
int clone;              /* make exact dup of existing class */
int lineno;             /* line number in input deck, for error message */
int hflag;              /* header only option - dont read or create classes */
int pflag;              /* cpp option - allowing includes and macros */
int cflag;              /* compile only option */
long filesize;          /* Underlying filesize name maximum */
int lastline;           /* boolean - set by a number of indications we
                           are out of input e.g. EOF, null input, etc */

struct Class Classes[MAX_CLASSES];
static struct ClassElem elem[MAX_DESC];
static char junk[128];  /* for holding garbage read in by scanf */
static char sname[MAXNAMLEN]; /* input clc source file name */
static char hname[MAXNAMLEN]; /* output header file name */
static char cname[MAXNAMLEN]; /* output C source file name */
static char include[MAXNAMLEN]; /* character string to hold the .h file name */
static char currentfile[MAXNAMLEN]; /* input file name for errors */
char command[MAXNAMLEN];        /* for commands to system or popen */
char message[MAXNAMLEN];        /* for generating messages for output */
char *created_classes [MAX_CLASSES]; /* list of generated object classes */
FILE *fs = NULL; /* file descriptors for .oclc, .h, .c files */
FILE *fh = NULL;
FILE *fc = NULL;
/* 
FILE *fopen(),*popen(); 
*/

/*
 * NAME: odmcreate (main)
 *
 * FUNCTION: Parse input file and create the necessary ODM object classes
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Independent program with command line arguments,
 *      usually invoked by a user from a shell, makefile, etc.
 *
 * NOTES: The odmcreate command takes a description of a set of object
 *      classes in a C-like form and from it creates the object
 *      classes and a header file to be included in C source programs.
 *      The classes it creates include the specific information
 *      data which libodm needs.
 *
 *      parse command line
 *      open input file and C header file
 *      parse input file
 *      *       special treatment for links  - find
 *              the  column type of the linked to column
 *      *       special treatment for imbed and clone - copy
 *              descriptor information from the imbedded class
 *      for each complete class description
 *      *       determine the structure offsets of each descriptor
 *      *       create the class
 *      close input file
 *
 * RETURNS: 0 if command completes successfully, a non-negative value
 *      is returned based on which error odmcreate encounters while
 *      parsing the input file.
 *
 */

main (argc, argv)
int argc;
char *argv[];
{
   extern int optind;
   extern char *optarg;

   char base[MAX_ODMI_NAME];
   char *period, *slash;
   int returncode, i;
   int option_letter;
   int free_index;
   char tempname[20] = "odmcreateXXXXXX";
#ifdef _BLD
   char * ode_path;
   char * inc_dirs;
   char cc_path[MAXNAMLEN];
#endif


#ifndef R5A
        (void) setlocale (LC_ALL, "");
	catd = catopen (MF_ODMCMD, NL_CAT_LOCALE);     /* Defect 116034 */
#endif

   cflag = FALSE;
   hflag = FALSE;
   pflag = FALSE;

   /* Check to see if any arguments were entered on the command */
   /* line. If not, print error and exit */

   if (argc == 1)
   {
       error(ODM_ARG_ERR,"usage: odmcreate [-h][-p] <basename>\n");
   }


   /* Determine the maximum filename for underlying file system */

   filesize = legal_size (cname);

   returncode = odm_initialize ();
   if (returncode < 0)
   {
      error(ODM_INIT_ERR,"odmcreate: Error initializing ODM\n");
   }

   bzero ((char *) Classes, MAX_CLASSES * sizeof (struct Class));

   /*****************************************************************/
   /* Get the command line arguments.  The valid options are:       */
   /*             h - create headers only                           */
   /*             p - invoke the C preprocessor before processing   */
   /*                 input file                                    */
   /*             c - create classes only (no headers)              */
   /*****************************************************************/
   while ((option_letter = getopt (argc, argv, "hpc")) != EOF)
   {
      switch (option_letter)
      {
         case 'h':
            /* Set the header only flag */
            hflag++;
            break;

         case 'p':
            /* Set the C preprocessor flag */
#ifdef _BLD

	    ode_path = getenv("ODE_TOOLS");
	    strcpy(cc_path,ode_path);
	    strcat(cc_path,"/usr/bin/cc");
            if (access (cc_path, F_OK) != 0)
#else
            if (access ("/usr/bin/cc", F_OK) != 0)
#endif
            {
               error(ODM_PREREQ_ERR,"C compiler necessary for '-p' option\n");
            }
            else
               pflag++;
            break;

         case 'c':
            /* Set the create only flag */
            cflag++;
            break;

         case '?':
         default:
            /* Error parsing input, generate usage error */
            error(ODM_ARG_ERR,"usage: odmcreate [-c][-h][-p] <basename>\n");
      }
   }
   
   /* Check to see if user has asked for both headers and create    */
   /* objects using the -c and -h flags.  Since this is the default */
   /* reset flags to 0.                                             */

   if (cflag && hflag)
   {
      cflag = FALSE;
      hflag = FALSE;
   }

   /* Check to see if the object class read in on the command */
   /* line is longer than the filesystem supports             */
   ncl = 0;
   while (argv[optind] != NULL)
   {
      /* Check to see if the class collection pointer (clxnp) is not NULL. */
      /* If it is not NULL, free it and the clxnp->clxnname                */

      for (free_index = 0; free_index < ncl; free_index++)
      {
         if (Classes[free_index].clxnp)
       	 {
	    if (Classes[free_index].clxnp->clxnname)
	       free(Classes[free_index].clxnp->clxnname);
 
	    free(Classes[free_index].clxnp);
	    Classes[free_index].clxnp = NULL;
	 }
      } 	
 
      sname[0] = '\0';
      hname[0] = '\0';
      cname[0] = '\0';
      include[0] = '\0';

      /* Create new list of object classes for each input file */
      for (i = 0; i < MAX_CLASSES; i++)
         created_classes [i] = NULL;

      if (strlen (argv[optind]) > filesize)
         error3(ODM_NAMELEN_ERR,"odmcreate: class %s name to long\n",
                                              0,argv[optind],filesize);

      /* See if object class definition specified by user has an */
      /* extension on it already                                 */

      period = (char *)strrchr (argv[optind], '.');
      if (period != (char *) NULL)
      {
         /* Found a period, now need to determine if its a */
         /* pathname or extension                          */

         slash = (char *)strrchr (argv[optind], '/');
         if (slash > period)
         {
            (void) strcpy (sname, argv[optind]);

            /* Get the filename from pathname and make */
            /* the include name by appending .h        */
            (void) strcpy (include, slash+1);
            (void) strcat (include, ".h");

            /* Append default extension                */
            (void) strcat (sname, ".oclc");

            /* Determine if file exists                */
            if (access (sname, F_OK) != 0)
            {
               /* File doesn't exist, append other */
               /* default extension                */
               (void) strcpy (sname, argv[optind]);
               (void) strcat (sname,".cre");
            }
         }

         else
         {
            /* File has user extension, so see if it */
            /* exists before using it                */
            (void) strcpy (sname, argv[optind]);
            if (access (sname, F_OK) !=0)
            {
               error1(ODM_DESCFILE_ERR,"odmcreate: file %s not found\n",sname);
            }

            *period = '\0';
            (void) strcpy (include, argv[optind]);
            (void) strcat (include, ".h");
         }
      }
      else
      {
         /* No period found, append default extension      */
         (void) strcpy (sname, argv[optind]);
         (void) strcat (sname, ".oclc");
         (void) strcpy (include, argv[optind]);
         (void) strcat (include, ".h");

         /* Determine if file exists                       */
         if (access (sname, F_OK) != 0)
         {
            /* File doesn't exist, append other       */
            /* extension                              */
            (void) strcpy (sname, argv[optind]);
            (void) strcat (sname, ".cre");
         }
      }

      /* Build the header file file names (.c and .h extensions) */

      (void) strcpy (hname, argv[optind]);
      (void) strcat (hname, ".h");
      (void) strcpy (cname, argv[optind]);
      (void) strcat (cname, ".c");

      if (!sname[0])
         error(ODM_ARG_ERR,"usage: odmcreate [-c][-h][-p] <basename>\n");

      (void) strcpy (currentfile, sname);

      if (pflag)
      {
         if (access (sname, F_OK) != 0)
         {
            error1(ODM_DESCFILE_ERR,"odmcreate: file %s not found\n",sname);
         }
	 mktemp(tempname);
#ifdef _BLD
	/* this is to make sure that the include header file from the build env */
         inc_dirs = getenv("_CC_GENINC");
         (void) sprintf (command, "%s %s -E %s > %s", cc_path,inc_dirs, sname,tempname);
#else
         (void) sprintf (command, "/usr/bin/cc -E %s > %s", sname,tempname);
#endif
	 
         if ( system(command) )
         {
	     unlink(tempname);
	     exit(-1);
         }
	 else
	 {
	    fs = fopen(tempname,"r");
	    unlink(tempname);
            if (!fs)
            {
              error1(ODM_DESCFILE_ERR,
                   "odmcreate: cant open class spec file '%s'\n",
                   tempname);
            }
	 } 
      }

      else
      {
         fs = fopen (sname,"r");
         if (!fs)
         {
            error1(ODM_DESCFILE_ERR,
                   "odmcreate: cant open class spec file '%s'\n",
                   sname);
         }
      }

      if (!cflag)
      {
         /* initialize the new header file */
         fh = fopen (hname, "w");
         if (!fh)
         {
            error1(ODM_HDRFILE_ERR,
                   "odmcreate: cant open header file '%s'\n",hname);
         }

         (void) fprintf (fh, "\n#include <odmi.h>\n");
         (void) fprintf (fh, "\n#ifndef _H_%s\n#define _H_%s\n\n",
                              argv[optind],argv[optind]);
         (void) fclose (fh);
         fh = NULL;

         /* initialize the new C file */
         fc = fopen (cname, "w");
         if (!fc)
         {
            error1(ODM_SRCFILE_ERR,
                   "odmcreate: cant open c source file '%s'\n",cname);
         }

         (void) fprintf (fc, "\n#include \"%s\"\n",include);
         (void) fclose (fc);
         fc = NULL;
      }

      /* process the input .oclc file */
      (void) read_class_defs (fs);

      /* clean up and go home */
      (void) fclose (fs);
      fs = NULL;
      optind++;
   }

   if (!cflag)                     /* Defect 90732 */
   {
      fh = fopen (hname,"a");
      if (!fh)
      {
         error1(ODM_APPEND_ERR,"odmcreate: Cant append to file '%s'\n", hname);
      }

      strncpy(base ,hname, strlen(hname) - 2);

      if (lastline)                 /* Defect 91718 */
         (void) fprintf (fh, "\n#endif /* _H_%s */\n\n\n",base);

      (void) fclose (fh);
      fh = NULL;
   }

   (void) exit (0);
}

/******************************************************************************
 * NAME: READ_CLASS_DEFS 
 *
 * FUNCTION: read the description of each class, and when each
 *      complete description is read, call for the creation
 *      of the class.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called by the odmcreate command.
 *
 * RETURNS: NONE
 *
 *****************************************************************************/

read_class_defs (fd)
FILE *fd;               /* file descriptor for the input */
{
   int i;                  /* scratch integer */
   int size;
   int brace;              /* brace counter */
   int linktype;
   int array;              /* current descriptor is of type array */
   char *cp;               /* scratch character pointer */
   char linebuf[LINELEN];  /* buffer to receive lines from the file */
   char type[128];         /* string to put descriptor type */
   char tmp[128];          /* scratch buffer */
   char classname[2*MAX_ODMI_NAME];        /* name of the current class */

   lastline = FALSE;
   lineno = 0;
   brace = 0;
   while (TRUE)
   {
      array = FALSE;
      bzero (linebuf, LINELEN);
      (void) fgets (linebuf, LINELEN, fd);
      lineno++;

      if (feof(fd) || ferror(fd))
         lastline = TRUE;

      if (!strlen (linebuf))
         break;

      /* if cpp stuff, get lineno */
      if (linebuf[0] == '#')
      {
         if ((sscanf (&linebuf[1],"%d %s", &i, tmp) == 2)
                                       && (tmp[0] == '"'))
         {
            tmp[strlen(tmp)-1] = '\0';
            (void) strcpy (currentfile, &tmp[1]);
            lineno = i - 1;
         }
         continue;
      }

      /* if comment at end of line, blow it away */
      if (cp = (char *)strchr (linebuf, '/'))
         *cp = '\0';

      /* if comma in the line, blow it away.  We dont
         allow multiple elem defs on a line as in a C struct
         but if the source includes one and we don't
         separate  the words, we may not catch the error */

      if (cp = (char *)strchr (linebuf, ','))
         *cp = ' ';

      /* left brace indicates beginning of a class */
      if (cp = mbschr (linebuf, (wchar_t)'{'))
      {
         brace++;
         clone = FALSE;
         *cp = '\0';

         if ((sscanf (linebuf, "%s %s %s",tmp, classname, junk) != 2) ||
            ((strcmp (tmp,"struct")) &&
            (strcmp (tmp,"class"))))
         {
            error2(ODM_SYNTAX_ERR,
              "odmcreate: class description syntax error, line %d in file %s\n",
                          lineno, currentfile);
         }

         if (strlen(classname) > (filesize-1))
         {
            error3(ODM_NAMELEN_ERR,
                   "odmcreate: classname error, line %d in file %s\n",
                               lineno, currentfile,filesize);
         }

         elems = 0;
         bzero (elem, MAX_DESC * sizeof (struct ClassElem));
         Classes[ncl].classname = malloc (strlen(classname) + 1);

         if (Classes[ncl].classname == NULL)
         {
            error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
         }

         Classes[ncl].begin_magic = ODMI_MAGIC;
         Classes[ncl].end_magic =  -ODMI_MAGIC;
         (void) strcpy (Classes[ncl].classname, classname);
      }

                   /* right brace indicates end of a class */
      else if (cp = mbschr (linebuf, (wchar_t)'}'))
      {
         brace--;
         if (brace < 0)
         {
            error2(ODM_SYNTAX_ERR,
             "odmcreate: class description syntax error, line %d in file %s\n",
                                        lineno, currentfile);
          }

          (void) write_class (classname);
       }

                   /* semicolon indicates a descriptor */
       else if (cp = (char *)strchr (linebuf, ';'))
       {
          if (clone)
          {
             error2(ODM_CLONE_ERR,
                    "odmcreate: cant use clone to imbed, line %d in file %s\n",
                                lineno, currentfile);
          }

          *cp = ' ';
          if (elems > MAX_DESC)
          {
             error3(ODM_MAXDESC_ERR,
                "odmcreate: too many elems in class '%s', line %d in file %s\n",
                                classname, lineno, currentfile);
          }

          elem[elems].type = 0;
          /* brace tells us is of type array -
             so we need to get dimension */
  	  if (cp = mbschr (linebuf, (wchar_t)']'))
          {
  	      *cp = ' ';

	      if (cp = mbschr (linebuf, (wchar_t)'['))
	      {
		 *cp = ' ';
		 if (sscanf(cp+1,"%d",&elem[elems].size) != 1)
		 {
		    error2(ODM_BAD_DIM_ERR,
         "odmcreate: bad dimension, line %d in file %s\n", lineno, currentfile);
        	 }
  	      }

	      else
	      {           /* couldn't find begin brace to match end */
 		 error2(ODM_BAD_DIM_ERR,
         "odmcreate: bad dimension, line %d in file %s\n", lineno, currentfile);
	      }

              array = TRUE;
	      *(cp+1) = '\0';
	  }

  	  else
             elem[elems].size = 0;

          if (sscanf (linebuf, "%s", type) != 1)
          {
             error2(ODM_NOTYPE_ERR,
                    "odmcreate: no type, line %d in file %s\n",
                                lineno,currentfile);
          }

          elem[elems].elemname = malloc(filesize);
          if (elem[elems].elemname == NULL)
          {
             error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
          }

          /* now, deal with each type line */
          if (!strcmp (type,"char") ||
              !strcmp (type,"ODM_CHAR"))
          {
             elem[elems].type = ODM_CHAR;
             (void) parse_simple (linebuf);
             if (!array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"longchar") ||
                   !strcmp (type, "ODM_LONGCHAR"))
          {
              elem[elems].type = ODM_LONGCHAR;
              (void) parse_simple (linebuf);
              if (!array)
                 (void) array_error (array, elems);
          }

          else if (!strcmp (type,"vchar") ||
                   !strcmp (type, "ODM_VCHAR") ||
                   !strcmp (type, "nchar") ||
                   !strcmp (type, "ODM_NCHAR"))
          {
	     /* Indicate nchar type by setting reserved field and elem type */
	     if ((strcmp(type,"nchar") == 0) ||
	         (strcmp(type,"ODM_NCHAR") == 0)) 
             {
                 /* Defect 110204: set type to ODM_CHAR */
                 elem[elems].type = ODM_CHAR;
		 elem[elems].reserved = 1;
	     }

       	     else 
             {
                elem[elems].type = ODM_VCHAR;
		elem[elems].reserved = 0;
             }

             (void) parse_simple (linebuf);
             if (!array)
                (void) array_error (array, elems);

             if (!Classes[ncl].clxnp)
             {
                Classes[ncl].clxnp = (struct StringClxn *)
                                     malloc(sizeof (struct StringClxn));

                if (Classes[ncl].clxnp == NULL)
                {
                   error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
                }

                bzero(Classes[ncl].clxnp,sizeof(struct StringClxn));
                Classes[ncl].clxnp->clxnname =
                                   (char *) malloc (strlen(classname) + 4);

                if (Classes[ncl].clxnp->clxnname <= (char *) 4)
                {
                   error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
                }

                (void) sprintf(Classes[ncl].clxnp->clxnname,
                               "%s.vc",Classes[ncl].classname);
             }
          }

          else if (!strcmp(type,"binary") ||
                   !strcmp (type, "ODM_BINARY"))
          {
             elem[elems].type = ODM_BINARY;
             (void) parse_simple (linebuf);
             if (!array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"short") ||
                   !strcmp (type, "ODM_SHORT"))
          {
             elem[elems].size = sizeof(short);
             elem[elems].type = ODM_SHORT;
             (void) parse_simple (linebuf);
             if (array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"long") ||
                   !strcmp (type, "ODM_LONG"))
          {
             elem[elems].size = sizeof(long);
             elem[elems].type = ODM_LONG;
             (void) parse_simple (linebuf);
             if (array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"ulong") ||
                   !strcmp (type, "ODM_ULONG"))
          {
             elem[elems].size = sizeof(unsigned long);
             elem[elems].type = ODM_ULONG;
             (void) parse_simple (linebuf);
             if (array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"double") ||
                   !strcmp (type, "ODM_DOUBLE"))
          {
             elem[elems].size = sizeof(double);
             elem[elems].type = ODM_DOUBLE;
             (void) parse_simple (linebuf);
             if (array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"int"))
          {
             elem[elems].size = sizeof(long);
             elem[elems].type = ODM_LONG;
             (void) parse_simple (linebuf);
             if (array)
                (void) array_error (array, elems);
          }

          else if (!strcmp (type,"link") ||
                   !strcmp (type, "ODM_LINK"))
          {
             elem[elems].col = malloc (filesize);
             if (elem[elems].col == NULL)
             {
                error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
             }
             elem[elems].type = ODM_LINK;
             (void) parse_link (linebuf);

             linktype = get_descrip_info (elem[elems].link,
                                          elem[elems].col, &size);
             elem[elems].linktype = linktype;

             switch ( linktype )
             {

                case ODM_CHAR:
                       elem[elems].size = size;
                       break;

                case ODM_LONGCHAR:
                       elem[elems].size = size;
                       break;

                case ODM_BINARY:
		       --ncl;
                       error2(ODM_UNKNOWNTYPE_ERR,
                        "odmcreate: invalid link type, line %d in file %s\n",
                                        lineno, currentfile);
                       break;

                case ODM_SHORT:
                       elem[elems].size = 6;
                       break;

                case ODM_LONG:
                       elem[elems].size = 11;
                       break;

                case ODM_LINK:
		       --ncl;
                       error2(ODM_UNKNOWNTYPE_ERR,
                        "odmcreate: invalid link type, line %d in file %s\n",
                                        lineno, currentfile);
                       break;

                case ODM_METHOD:
		       --ncl;
                       error2(ODM_UNKNOWNTYPE_ERR,
                        "odmcreate: invalid link type, line %d in file %s\n",
                                        lineno, currentfile);
                       break;

                case ODM_VCHAR:
                       /* Defect 93885 */
                       elem[elems].size = size;
                       break;

                case ODM_DOUBLE:
                       elem[elems].size = 24;
                       break;

                case ODM_ULONG:
                       elem[elems].size = 11;
                       break;

                default:
                       error2(ODM_UNKNOWNTYPE_ERR,
                        "odmcreate: unknown element type, line %d in file %s\n",
                                        lineno, currentfile);
                       break;
             }
          }

          else if (!strcmp (type, "method") ||
                   !strcmp (type, "ODM_METHOD"))
          {
              elem[elems].type = ODM_METHOD;
              (void) parse_simple (linebuf);
              if (!elem[elems].size)
                 elem[elems].size = METHODSIZE;
          }

          else
          {
             error2(ODM_UNKNOWNTYPE_ERR,
                    "odmcreate: unknown element type, line %d in file %s\n",
                                lineno,currentfile);
          }

          elems++;
       }

       if (lastline)
          break;
   }

   if (brace != 0)
   {
       error2(ODM_SYNTAX_ERR,
             "odmcreate: class description syntax error line %d in file %s\n",
                        lineno, currentfile);
   }
}

/*****************************************************************************
 * NAME: ARRAY_ERROR
 *
 * FUNCTION: report descriptors which should not be arrays, but are
 *      or should be arrays, but are not
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called by read_class_defs
 *
 * RETURNS: generates appropriate error message and exits
 *
 ****************************************************************************/

int array_error (array, i)
int array;      /* boolean: erroneous desc was an array */
int i;          /* index of the descriptor */
{

   if (!array)
   {
      error3( ODM_NOTARRAY_ERR,
           "odmcreate: descriptor '%s' must be an array, line %d in file %s\n",
                 elem[i].elemname, lineno, currentfile);
   }
   else
   {
      error3( ODM_ARRAY_ERR,
        "odmcreate: descriptor '%s' must not be an array, line %d in file %s\n",
                elem[i].elemname, lineno, currentfile);
   }
}

/*****************************************************************************
 * NAME: PARSE_SIMPLE
 *
 * FUNCTION: parse the simple descriptor input lines
 *      (short, long,...).  All we need to get
 *      point is the descriptor name - we know the type.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called by read_class_defs
 *
 * RETURNS: upon success, no value is returned.  On error, prints
 *      appropriate error message and then exits
 *
 ****************************************************************************/

int parse_simple (line)
char line[];
{

   char tmp[LINELEN], string [LINELEN], newline [LINELEN];
   int number;

   (void) strcpy (newline, line);
   number = sscanf (newline, "%s %s %s", tmp, string, tmp);
   if (number != 2)
   {
      error2(ODM_SYNTAX_ERR,
             "odmcreate: class description syntax error line %d in file %s\n",
                        lineno, currentfile);
   }

   if (strlen (string) > (filesize - 1))
   {
      error3(ODM_NAMELEN_ERR,
             "odmcreate: name length error line %d in file %s\n",
                        lineno, currentfile,filesize);
   }

   (void) strcpy (elem[elems].elemname, string);
}

/*****************************************************************************
 * NAME: PARSE_LINK
 *
 * FUNCTION: parse the link descriptor input lines
 *      We need to get the class, stdclass, col, and name.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called from read_class_defs
 *
 * RETURNS: Successful completion has no return value, on failure error
 *      message is generated and then exits back to shell
 *
 ****************************************************************************/

int parse_link (line)
char *line;
{

   char tmp[LINELEN], structname[LINELEN], linkclassname[LINELEN];
   char colname[LINELEN], elemname[LINELEN];

   if (sscanf (line, "%s %s %s %s %s %s", tmp, structname, linkclassname,
               colname, elemname, tmp) != 5)
   {
       error2(ODM_SYNTAX_ERR,
              "odmcreate: class description syntax error line %d in file %s\n",
                        lineno, currentfile);
   }

   if (strlen (linkclassname) > (filesize - 1))
   {
       error3(ODM_NAMELEN_ERR,
              "odmcreate: name length error line %d in file %s\n",
                        lineno, currentfile,filesize);
   }

   if (strlen (structname) > (filesize - 1))
   {
       error3(ODM_NAMELEN_ERR,
              "odmcreate: name length error line %d in file %s\n",
                        lineno, currentfile,filesize);
   }

   if (strlen (elemname) > (filesize - 1))
   {
      error3(ODM_NAMELEN_ERR,
             "odmcreate: name length error line %d in file %s\n",
                        lineno, currentfile,filesize);
   }

   (void) strcpy (elem[elems].elemname, elemname);
   if (strlen (colname) > (filesize - 1))
   {
      error3(ODM_NAMELEN_ERR,
             "odmcreate: name length error line %d in file %s\n",
                        lineno, currentfile,filesize);
   }

   (void) strcpy (elem[elems].col, colname);

   if ((lcl = find_class_desc (linkclassname)) == -1)
   {
      error3(ODM_CLASSNOTFOUND_ERR,
        "odmcreate: class '%s' is not described in the class spec, line %d in file %s\n",
        linkclassname, lineno, currentfile);
   }

   elem[elems].link = &Classes[lcl];
}

/*****************************************************************************
 * NAME: WRITE_CLASS
 *
 * FUNCTION: calls routines to get struct info for the class
 *      and create it in the odm.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called from read_class_defs
 *
 * RETURNS: NONE
 *
 ****************************************************************************/

int write_class (classname)
char *classname;
{
   int retcode;

   if (elems > 0)
   {
      Classes[ncl].elem =  (struct ClassElem *)
      malloc (elems * sizeof(struct ClassElem));
      if (Classes[ncl].elem == NULL)
         error(ODM_MALLOC_ERR,"odmcreate: malloc failure\n");
   }

   Classes[ncl].nelem =  elems;

   bcopy (elem, Classes[ncl].elem,elems * sizeof(struct ClassElem));
   (void) get_offsets (&Classes[ncl]);
   if (!cflag)
      (void) create_header (classname);

   if (!hflag)
   {
      (void) odm_rm_class (&Classes[ncl]);
      retcode = odm_create_class (&Classes[ncl]);
      if (retcode < 0)
      {
         error1(ODM_CREATE_FAIL,
                "odmcreate: cannot create object class '%s'\n",classname);
      }
   }

   created_classes[ncl] = malloc (strlen(classname) * sizeof(char) + 1);
   if (created_classes[ncl] == NULL)
   {
       error(ODM_MALLOC_ERR, "odmcreate: malloc failure\n");
   }

   (void) strcpy (created_classes[ncl], classname);
   ncl++;

   /* MAX_CLASS test       */
   if (ncl >= MAX_CLASSES)
   {
      error1(ODM_MAX_CLASSES,
             "odmcreate: maximum number of classes for file '%s' exceeded\n",
                         sname);
   }

   (void) fprintf (stdout, "%s\n", classname);
}


/*****************************************************************************
 * NAME: CREATE_HEADER
 *
 * FUNCTION: Append the struct for this class to the .h file and create
 *      the entries in the .c file that initialize the appropriate header
 *      structure
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called from write_header
 *
 * NOTES: Creates the .h and the .c files for the defined object class in
 *      the specified directory, the actual ODM object class is created in
 *      the current directory
 *
 * RETURNS: On successful completion, no returned value.  On failure, the
 *      appropriate error message is generated and exit back to the shell
 *
 ****************************************************************************/

int create_header (classname)
char *classname; /* the class name */
{

   char stringname[MAX_ODMI_NAME];
   int i;          /* scratch integer */
   struct ClassElem *elemp;
   int current_offset;    /* current offset to be written to .c file */
   int next_offset;       /* next offset    */

   elemp =  Classes[ncl].elem;

   fh = fopen (hname,"a");
   if (!fh)
   {
      error1(ODM_APPEND_ERR,
             "odmcreate: Cant append to file '%s'\n", hname);
   }

   (void) fprintf (fh,"\nstruct %s {\n",classname);
   (void) fprintf (fh,"\tlong _id;\n");
   (void) fprintf (fh,"\tlong _reserved;\n");
   (void) fprintf (fh,"\tlong _scratch;\n");
   for (i = 0; i < elems; i++)
   {
       switch (elemp[i].type)
       {
          case ODM_SHORT:
                (void) fprintf (fh,"\tshort %s;\n", elemp[i].elemname);
                break;

          case ODM_DOUBLE:
                (void) fprintf (fh,"\tdouble %s;\n", elemp[i].elemname);
                break;

          case ODM_LONG:
                (void) fprintf (fh,"\tlong %s;\n", elemp[i].elemname);
                break;

          case ODM_ULONG:
                (void) fprintf (fh,"\tulong %s;\n", elemp[i].elemname);
                break;

          case ODM_VCHAR:
                /* Defect 110204: since the element type is set to
                   ODM_CHAR when nchar is used. no need to check for
                   nchar here       */
                /* if (elemp[i].reserved)        
                      (void) fprintf (fh,
                             "\tchar %s[%d];\n", elemp[i].elemname,
                                        elemp[i].size);
                else  */


                (void) fprintf (fh,"\tchar *%s;\n", elemp[i].elemname);
                break;

          case ODM_CHAR:
          case ODM_LONGCHAR:
          case ODM_BINARY:
                (void) fprintf (fh,"\tchar %s[%d];\n", elemp[i].elemname,
                                   elemp[i].size);
                break;

          case ODM_METHOD:
                (void) fprintf (fh,"\tchar %s[%d];\t/* method */\n",
                                    elemp[i].elemname, elemp[i].size);
                break;

          case ODM_LINK:
                (void) fprintf (fh,"\tstruct %s *%s;\t/* link */\n",
                                   elemp[i].link->classname,elemp[i].elemname);

                 (void) fprintf (fh,"\tstruct listinfo *%s_info;\t/* link */\n",
                                    elemp[i].elemname);

                 (void) fprintf (fh,"\tchar %s_Lvalue[%d];\t/* link */\n",
                                    elemp[i].elemname, elemp[i].size);
                 break;

          default:
                error2(ODM_UNKNOWNTYPE_ERR,
                       "odmcreate: unknown element type, line %d in file %s\n",
                                lineno, currentfile);
                break;
       }
   }

   (void) fprintf (fh, "\t};\n");
   (void) fprintf (fh,"#define %s_Descs %d\n\n", classname, elems);
   (void) fprintf (fh, "extern struct Class %s_CLASS[];\n", classname);
   (void) fprintf (fh,
       "#define get_%s_list(a,b,c,d,e) (struct %s * )odm_get_list(a,b,c,d,e)\n",
                classname, classname);

   (void) fclose (fh);
   fh = NULL;

   fc = fopen (cname,"a");
   if (!fc)
   {
      error1(ODM_APPEND_ERR,"odmcreate: Cant append to file '%s'\n", cname);
      /* defect 91265: change hname to cname when calling error1 */
   }

   (void) fprintf (fc, "static struct ClassElem %s_ClassElem[] =  {\n",
                        classname);

   next_offset = 3 * sizeof(long);       /* setting offset  */

   for (i = 0; i < elems; i++)
   {

      /* Defect 105129: if nchar type is used, the offset is
         saved as 4 bytes (char pointer) instead of the
         actual size (static char), when we write the 
         offset to the .c file, the size needs to be
         the static char size.                                */

      /* Defect 109547: Need to keep track of current offset 
         and next offset when writing offset to the .c file.  
         The calculation of rounding off to the boundary is  
         neccessary. Defect 105129 didn't take care of this. 
         The logic of rounding off to the boundary is copied 
         from get_offset routine.                             */

      current_offset = next_offset;      /* beginning offset  */

      switch (elemp[i].type)
      {
         case ODM_SHORT:
               /* rounding the address up to the next short
                  word boundary by adding one and masking off
                  the last bit.                                */

               current_offset = (current_offset + 1) & ~1;
               next_offset = current_offset + 2;
               (void) fprintf (fc,
                     " { \"%s\",ODM_SHORT, %d, %d, NULL,NULL,0,NULL ,-1,0},\n",
                       elemp[i].elemname, current_offset, elemp[i].size);
               break;

         case ODM_LONG:
               /* rounding the address up to the next longword
                  boundary by adding three and masking off the
                  last two bits.                                */

               current_offset = (current_offset + 3) & ~3;
               next_offset = current_offset + 4;
               (void) fprintf (fc,
                       " { \"%s\",ODM_LONG, %d, %d, NULL,NULL,0,NULL ,-1,0},\n",
                              elemp[i].elemname, current_offset, elemp[i].size);

               break;

         case ODM_ULONG:
               /* rounding the address up to the next longword
                  boundary by adding three and masking off the
                  last two bits.                                 */

               current_offset = (current_offset + 3) & ~3;
               next_offset = current_offset + 4;
               (void) fprintf (fc,
                    " { \"%s\",ODM_ULONG, %d, %d, NULL,NULL,0,NULL ,-1,0},\n",
                            elemp[i].elemname, current_offset, elemp[i].size);
               break;

         case ODM_DOUBLE:
               /* rounding the address up to the next longword
                  boundary by adding three and masking off the
                  last two bits.                                */

               current_offset = (current_offset + 3) & ~3;
               next_offset = current_offset + sizeof(double);
               (void) fprintf (fc,
                    " { \"%s\",ODM_DOUBLE, %d, %d, NULL,NULL,0,NULL ,-1,0},\n",
                             elemp[i].elemname, current_offset, elemp[i].size);
               break;

         case ODM_CHAR:
               /* char arrays, no rounding  */

               next_offset = current_offset + elem[i].size;
               (void) fprintf (fc,
                    " { \"%s\",ODM_CHAR, %d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                           elemp[i].elemname, current_offset, elemp[i].size);
               break;

         case ODM_VCHAR:
               /* Defect 110204: since the element type
                  is set to ODM_CHAR when nchar is used. 
                  no need to check for nchar here       */
               /* if (elemp[i].reserved)         
               {

                  next_offset = current_offset + elem[i].size;
                  (void) fprintf (fc,
                    " { \"%s\",ODM_CHAR, %d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                        elemp[i].elemname, current_offset, elemp[i].size);
                  break;
               }
               else   */                        

               /* rounding the address up to the next longword
                  boundary by adding three and masking off the
                  last two bits.                                */

               current_offset = (current_offset + 3) & ~3;
               next_offset = current_offset + 4;
 
               (void) fprintf (fc,
                    " { \"%s\",ODM_VCHAR, %d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                           elemp[i].elemname, current_offset, elemp[i].size);

               break;

         case ODM_LONGCHAR:
               /* no rounding */
  
               next_offset = current_offset + elem[i].size;
               (void) fprintf (fc,
                   " { \"%s\",ODM_LONGCHAR, %d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                             elemp[i].elemname, current_offset, elemp[i].size);
                break;

          case ODM_BINARY:
                /* no rouding requires */

                next_offset = current_offset + elem[i].size;
                (void) fprintf (fc,
                    " { \"%s\",ODM_BINARY,%d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                           elemp[i].elemname, current_offset, elemp[i].size);
                break;

          case ODM_METHOD:
                /* method is a char arrays, no rounding */

                next_offset = current_offset + elem[i].size;
                (void) fprintf (fc,
                    " { \"%s\",ODM_METHOD, %d,%d, NULL,NULL,0,NULL ,-1,0},\n",
                            elemp[i].elemname, current_offset, elemp[i].size);
                break;

          case ODM_LINK:
                /* links begin with pointers, so we are
                   rounding the address up to the next longword
                   boundary by adding three and masking off the
                   last two bits.                                */

                current_offset = (current_offset + 3) & ~3;
                next_offset = current_offset + elem[i].size + 8; 
                (void) fprintf (fc,
                " { \"%s\",ODM_LINK, %d ,%d, %s_CLASS,\"%s\",%d,NULL ,-1,0},\n",
                       elemp[i].elemname, current_offset, elemp[i].size,
                       elemp[i].link->classname, elemp[i].col,
                       elemp[i].linktype);
                break;

          default:
                error2(ODM_UNKNOWNTYPE_ERR,
                    "odmcreate: unknown element type, line %d in file %s\n",
                    lineno, currentfile);
                break;
      }
   }

   (void) fprintf (fc," };\n");
   if (Classes[ncl].clxnp)
   {
      (void) sprintf (stringname, "%s_STRINGS", classname);
      (void) fprintf (fc,"struct StringClxn %s[] = {\n",stringname);
      (void) fprintf (fc, " \"%s\", 0,NULL,NULL,0,0,0\n",
                           Classes[ncl].clxnp->clxnname);
      (void) fprintf (fc," };\n");
   }

   else
      (void) sprintf (stringname, "NULL", classname);

   (void) fprintf (fc, "struct Class %s_CLASS[] = {\n", classname);
   (void) fprintf (fc, " ODMI_MAGIC, \"%s\", sizeof(struct %s),",
                       classname, classname);
   (void) fprintf (fc, " %s_Descs, %s_ClassElem, %s,FALSE,NULL,",
                       classname, classname, stringname);
   (void) fprintf (fc, "NULL,0,0,NULL,0,\"\", 0,-ODMI_MAGIC\n");
   (void) fprintf (fc," };\n");
   (void) fclose (fc);
   fc = NULL;
}

/*****************************************************************************
 * NAME: GET_DESCRIP_INFO  
 *
 * FUNCTION: get the type and size of a column in a given class
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Function used in create_header
 *
 * NOTES: This is necessary because the implicit query criterion for a link
 *        is different depending on whether the column it points to is string
 *        or numeric.   For a string the value is enclosed in single quotes.
 *
 * RETURNS: Successful completion returns the type of the field in question.
 *      On error, the appropriate error message is generated and exits to the
 *      shell
 *
 ****************************************************************************/

char get_descrip_info (classp, col, size)
struct Class *classp;   /* name of the class */
char *col;              /* name of the column */
int *size;              /* returned size of the field */
{

   int i;  /* scratch integer */
   char v; /* return value */

   for (i = 0; i < classp->nelem; i++)
      if (!strcmp (col, classp->elem[i].elemname))
      {
         v = classp->elem[i].type;
         *size = classp->elem[i].size;
         return (v);
      }

   error4( ODM_COLNOTFOUND_ERR,
          "odmcreate: couldn't find col '%s' in linked to class '%s', line %d in file %s\n",
          col, classp->classname, lineno, currentfile);
   return ((char) -1);
}

/*****************************************************************************
 * NAME: FIND_CLASS_DESC
 *
 * FUNCTION: look through the classes we already have seen to see if we
 *      already have a description of a given class
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Function called bye read_class_defs
 *
 * RETURNS: Successful completion returns the index position of the class
 *      requested.  On failure, returns a -1 (class not found)
 *
 ****************************************************************************/

int find_class_desc (class)
char *class;
{
   int i;

   for (i = 0; i < ncl; i++)
      if (!strcmp (Classes[i].classname, class))
         return (i);
   return (-1);
}

/*****************************************************************************
 * NAME: FCLEAN
 *
 * FUNCTION: On error exit, closes all open files and deletes the created
 *      .c and .h so that user doesn't think that the odmcreate was
 *      successful.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine is call on ALL error exits to clean up files that were
 *      created during execution
 *
 * RETURNS: NONE
 *
 ****************************************************************************/

fclean()
{
   int i, retcode;
   struct Class *class_symbol_ptr;
   struct Class *odm_mount_class ();

   if (fc)
      (void) fclose(fc);

   if (fh)
      (void) fclose(fh);

   (void) unlink (cname);
   (void) unlink (hname);
   for (i = 0; i < ncl; i++)
   {
      class_symbol_ptr = odm_mount_class (created_classes [i]);
      if (class_symbol_ptr == (struct Class *) NULL)
      {
         (void) fprintf (stdout, catgets(catd,
                         ODM_MSG_SET, ODM_TERM_ERR,
                         "odmcreate: error on exit\n"));
      }

      retcode = odm_rm_class (class_symbol_ptr);
      if (retcode < 0)
      {
         (void) fprintf (stdout, catgets(catd,
                         ODM_MSG_SET, ODM_TERM_ERR,
                         "odmcreate: unable to delete %s\n"),
                         created_classes[i]);
      }
   }
   (void) odm_terminate ();
}
