static char sccsid[] = "@(#)59  1.26  src/bos/usr/bin/odmget/odmget.c, cmdodm, bos411, 9434B411a 8/22/94 17:35:51";
/*
 * COMPONENT_NAME: (CMDODM) Object Data Manager library
 *
 * FUNCTIONS:  main (odmget), get_from_odm, print_odm_syntax, odmiget
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "odmcmd_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>    /* access system call constants */
#include "odmcmd.h"
#include <odmi.h>
#include "odmroutine.h"
#include <string.h>
#include <mbstr.h>

#define OODM_LINK 102         /* Needed since odmi.h redefines */
#define OODM_METHOD 103       /* LINK and METHOD               */

#define MAX_FNAME 255

struct Class *odm_mount_class();
extern int errno;
extern int odmerrno;
int cmddebug={0};
#define POPO 42
extern int odmtrace;
extern int odm_read_only;

/*
 * NAME: main (odmget)
 *
 * FUNCTION:
 *
 *      Retrieves objects from an object class and displays them in
 *      an ascii stanza to standard output.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Runs as a command entered from the shell.  The syntax for this
 *      command is:
 *
 *                    odmget [-q criteria] classname [classname . . . ]
 *                    odmget -O [-q criteria]  classname [classname . . . ]
 *
 *                     -O  --  Get data entries from an ODM data class
 *
 * RETURNS:
 *
 *     Exits with 0 if successful, greater than zero if not.
 *
 */
main (argc, argv, envp)
    int argc;
    char *argv[];
    char *envp;
{
  int index;

#ifndef R5A
 setlocale(LC_ALL,"");
 scmc_catd = catopen(MF_CMD, NL_CAT_LOCALE);    /* Defect: 116034 */
#endif

  if (argc == 1)
    {
      print_odm_syntax();
      exit(47);
    } /* endif */

  odm_read_only=1;

  for (index = 1; index < argc; index++)
    {
      if (strcmp(argv[index],"-O") == 0 )
        {
         print_odm_syntax();
         exit(88);
        } /* endif */
    } /* endfor */

      exit(odmiget(argc,argv));


} /* end of main.  odmget */


/****************************************************************************
 * NAME:  PRINT_ODM_SYNTAX
 *
 * FUNCTION:
 *
 *      Displays the correct syntax for the odmget command.
 *
 * RETURNS:
 *
 *     Nothing.
 ****************************************************************************/
int print_odm_syntax()
{
    fprintf(stderr, catgets(scmc_catd, MS_odmget, GET_MSG_1,
            "usage: odmget [-q criteria] classname [classname . . . ]\n") );
}


/****************************************************************************
 * NAME:  PRINT_ODM_STRING
 *
 * FUNCTION:
 *
 *      prints a string in ODMADD stanza format.
 *
 * RETURNS:
 *
 *     Nothing.
 ****************************************************************************/

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

  fprintf(output, "\"");
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

  fprintf (output, "\"");
}


struct Class *odm_mount_class();

/****************************************************************************
 * NAME: ODMIGET
 *
 * FUNCTION:
 *
 *    Retrieves objects from a "new" ODM database.
 *
 * RETURNS:
 *
 *    A 0 if successful, a positive number otherwise.
 *
 ****************************************************************************/

int odmiget(argc,argv)
int argc;         /* argc passed to main() */
char *argv[];     /* argv passed to main() */
{
    struct Class *classp;

    int user_flags;

    char *m,*o;
    int i,ngot;
    char crit[MAX_ODMI_CRIT];
    char *criteria;
    int len;
    int file_format;
    int extra_format;
    char classname[256];
    int inherit_flag;
    int num_of_crit_flags;
    int option_letter;
    extern int optind;
    extern char *optarg;

    int number_descriptors;
    int index;
    FILE *output;
    int returncode;
    char *binary_in_ascii_ptr;  /* Ptr to binary value represented in ascii */
    char *string_ptr;

    /*
     * init various variables
     */

#ifdef _DEBUG
    cmddebug = 1;
#endif
    classname[0] = '\0';
    crit[0]  = '\0';
    criteria = (char *)&crit;
    num_of_crit_flags = 0;

    /*
     * Get the command line arguments.
     * Valid options are o, q, c, s, f, e, i and m.
     */
    while ((option_letter = getopt(argc,argv, "q:O")) != EOF)
    {

        switch ( option_letter )
        {
          case 'O':
                /* This is from the odmget */
            break;

          case 'q':

            if ((len=strlen(optarg)) >= MAX_ODMI_CRIT )
              {
		criteria = (char *)malloc(len + 1);
		if ( criteria == (char *) NULL )
		{
			perror("odmget");
                	return(6);
		}
              }
            num_of_crit_flags++;
            strcpy(criteria,optarg);
            break;
        case '?':
        DEFAULT:
            print_odm_syntax();
            return(7);  /* option error */
        } /* end case */
    }

    /* print error msg and exit if more than one criteria string entered */
    if (num_of_crit_flags > 1)
      {
        print_odm_syntax();
        return (1);
      }

    if (argv[optind][0] == '\0')
    {
        fprintf(stderr, catgets(scmc_catd, MS_odmget, GET_MSG_3,
                                   "odmget:  No class name specified\n") );
        print_odm_syntax();
        return(1);
    }

    output = stdout;

    /*
     * Setup call to odminit
     */

     odm_initialize ();

    /*
     * Open object class
     */
    for (index = optind; index < argc; index++)
      {
           if ( strlen(argv[index]) >= MAX_ODMI_NAME )
           {
              fprintf(stderr, catgets(scmc_catd, MS_odmget, GET_MSG_4,
                            "odmget: Class name too long: %s\n") ,argv[index]);
               return(1); /* SMU Defect 54087 */
           }
           strcpy(classname,argv[index]);

          classp = odm_mount_class(classname);
          if ( (int) classp == -1 || classp == NULL )
          {
             fprintf(stderr, catgets(scmc_catd, MS_odmget, GET_MSG_5,
                                "odmget:  Cannot open class %s\n") ,classname);
             return(15);
          }
          number_descriptors = classp->nelem;

          /*
           * Setup call to odmget.
           */

          /*
           * Setup criteria for search
           */

          returncode = 1;
          m = (char *) malloc (classp->structsize);

          ngot = 0;
          while( returncode > 0 )
          {
              o = (char *) odm_get_obj(classp,criteria,m,!(ngot++));
              if ( !o)
              {
                  break;
              }

              if (o == (char *) -1)
              {
             fprintf(stderr, catgets(scmc_catd, MS_odmget, GET_MSG_6,
                "odmget: Could not retrieve object for %s, odm errno %d\n") ,
                                 classname,odmerrno);
                 break;
              }

              /*
               * Now display the results, if there is any thing to display.
               */


                  /*
                   * loop thru and print each descriptor
                   */
                  for(i=0;i<classp->nelem;i++)
                  {
                       if (i == 0)
                         {
                           fprintf(output,"\n%s:\n",classname);
                         }
                       else
                         {
                           fprintf(output,"\n");
                         } /* endif */

                       fprintf(output,"\t%s = ",(classp->elem)[i].elemname);

                      switch((classp->elem)[i].type)
                      {
                      case ODM_LONG:
                          fprintf(output,
                                "%ld",*(long *)(m + (classp->elem)[i].offset));
                          break;
                      case ODM_ULONG:
                          fprintf(output,
                                "%lu",*(unsigned long *)(m + (classp->elem)[i].offset));
                          break;
                      case ODM_DOUBLE:
                          fprintf(output,
                               "%g",*(double *)(m + (classp->elem)[i].offset));
                          break;
                      case ODM_SHORT:
                          fprintf(output,
                                "%d",*(short *)(m + (classp->elem)[i].offset));
                          break;
                      case ODM_LINK:
                          fprintf(output, "\"%s\"",
                    (char *)(m + (classp->elem)[i].offset + 2*sizeof(char *)));
                              break;
                      case ODM_CHAR:
                      case ODM_LONGCHAR:
                      case ODM_METHOD:
                             string_ptr=(char *)(m+(classp->elem)[i].offset );
                             print_odm_string (string_ptr, output);
                         break;
                      case ODM_VCHAR:
                             string_ptr = *(char **)(m + (classp->elem)[i].offset);
                             print_odm_string (string_ptr, output);
                          break;
                      case ODM_BINARY:
                          binary_in_ascii_ptr =
                             convert_to_hex_ascii(m + (classp->elem)[i].offset,
                                                   (classp->elem)[i].size);

                          if (binary_in_ascii_ptr == NULL)
                            {
                              fprintf(stderr,
                         catgets(scmc_catd, MS_odmget, GET_MSG_7,
                        "odmget: Could not output descriptor %s in class %s\n"),
                                      (classp->elem)[i].elemname,
                                      classp->classname);
                              return(33);

                            } /* endif */

                          fprintf(output,"\"%s\"",binary_in_ascii_ptr);
                          break;
                      default:
                              fprintf(stderr,
                         catgets(scmc_catd, MS_odmget, GET_MSG_8,
                           "odmget: Bad descriptor type for %s in class %s\n"),
                                      (classp->elem)[i].elemname,
                                      classp->classname);
                      }
                  }
                fprintf(output,"\n");

          } /* endwhile */
      } /* endfor index < argc */

    odm_terminate ();
    return(0);   /* successful return */
}

