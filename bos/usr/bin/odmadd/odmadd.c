static char sccsid[] = "@(#)60	1.20.1.3  src/bos/usr/bin/odmadd/odmadd.c, cmdodm, bos411, 9428A410j 12/7/93 11:34:25";
/*
 * COMPONENT_NAME: (CMDODM) Object Data Manager
 *
 * FUNCTIONS: main, print_syntax, open_input_file, populate_from_file
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
nl_catd  scmc_catd;      /* Cat descriptor for scmc conversion */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <odmi.h>
#include "odmtrace.h"
#include "odmroutine.h"

#define MAX_STANZA_NAME 255

CLASS_SYMBOL odm_mount_class();
extern int stanza_line_number;
FILE *open_input_file();

/*
 * NAME: main() of odmadd
 *
 * FUNCTION:
 *
 *        Adds objects to existing object classes.
 *
 * EXECUTION ENVIRONMENT:
 *
 *        The command runs as a command from the shell.  It takes objects
 *        in the form of a stanza which looks like:
 *
 *           <class name1>:
 *                     <descriptor name1> = <value1>
 *                     <descriptor name2> = <value2>
 *
 *           <class name2>:
 *                     <descriptor name3> = <value3>
 *                     <descriptor name4> = <value4>
 *
 *            etc.
 *
 *         The objects can either come from a file or from standard input
 *
 * RETURNS:                                                              .
 *
 *        Returns a 0 if successful, a number greater than 0 if not
 */
main (argc, argv, envp)
int argc;
char *argv[];
char *envp;
{
  int index;

#ifndef R5A
 setlocale(LC_ALL,"");
 scmc_catd = catopen(MF_CMD, NL_CAT_LOCALE);         /* Defect 116034 */
#endif

   if (argc > 1 && argv[1][0] == '-')
   {
      /*--------------------------------------------------------------*/
      /* Since odmadd does not take any commands, print the correct   */
      /* syntax.                                                      */
      /*--------------------------------------------------------------*/
      print_syntax();
      exit(66);
   } 

   odm_initialize ();

   if (argc == 1)
   {
      /*------------------------------------------------------------------*/
      /* The populate will take stanzas from standard input if a filename */
      /* is not specified.                                                */
      /*------------------------------------------------------------------*/
      populate_from_file(NULL);
   }
   else
   {
      for (index = 1 ; index < argc; index++)
      {
         populate_from_file(argv[index]);
      } 
   } 

   exit(0);
}  /* main */


/**************************************************************************
 *  NAME:     POPULATE_FROM_FILE
 *
 *  FUNCTION: Read objects in the form of stanzas from an input file and
 *            put them in the appropriate object class.
 *
 *  RETURNS:  Returns a 0 if successful, exits otherwise with a positive
 *            number.
 **************************************************************************/

int populate_from_file(stanza_input_file)
char *stanza_input_file;   /* Name of the input file */
{
   FILE *file_ptr;
   int stanza_length;
   char *stanza;
   char terminating_char;
   char *class_name;
   char *descrip_name;
   char *descrip_value;
   int skip_spaces;
   int number_of_values;
   register int descriptor_index;
   char *new_entry;
   char *descriptor_offset;
   CLASS_SYMBOL Class;
   int returnstatus;
   char **vchar_location;          /* ptr to vchar location in the structure */
   char *first_err;
   CLASS_SYMBOL return_class;

   stanza_line_number = 0;

   if (stanza_input_file)
      file_ptr = open_input_file(stanza_input_file);
   else
   {
      TRC("odmadd","getting info from stdin","","","");
      file_ptr = stdin;
   } /* endif */


   while ( (stanza_length = get_ascii_phrase(file_ptr,STANZA,&stanza)) > 0)
   {
      TRC("odmadd:","stanza length is %d",stanza_length,"","");
      skip_spaces = FALSE;

      /*--------------------------------------------------*/
      /* Get the first value from the phrase.  Subsequent */
      /* calls will be made with the value 'NULL' for     */
      /* 'ascii_string' since the 'get_value' routine     */
      /* keeps track of where it was last.                */
      /*--------------------------------------------------*/

      /* Find the class name */
      class_name = get_value_from_string(stanza,":\n",skip_spaces,
                                         &terminating_char);

      if (class_name == (char *) NULL || terminating_char != ':')
      {
         /*--------------------------------*/
         /* Could not find the class name. */
         /*--------------------------------*/
         TRC("odmadd","could not find class name!","","","");
         fprintf(stderr,
            catgets(scmc_catd, MS_odmadd, ADD_MSG_0,
            "odmadd:  Cannot find class name in stanza, stanza line: %d  %s\n"),
                      stanza_line_number,stanza_input_file);
         exit(107);
      } /* endif */

      TRC("odmadd","class name %s",class_name,"","");

      if ((Class=odm_mount_class(class_name))==NULL ||
           Class == (CLASS_SYMBOL) -1  )
      {
         TRC("odmadd","could not open class %s",class_name,"","");
         fprintf(stderr,
                 catgets(scmc_catd, MS_odmadd, ADD_MSG_1,
                 "odmadd:  Cannot open class:  %s, stanza line: %d  %s\n"),
                           class_name,stanza_line_number,stanza_input_file);
         exit(117);
      }
      else
      {
         skip_spaces = TRUE;
         return_class = (CLASS_SYMBOL) odm_open_class(Class);
         if ((int) return_class == -1)
         {
            TRC("odmadd","Could not open class! err %d",odmerrno,"","");
            fprintf(stderr,
                    catgets(scmc_catd, MS_odmadd, ADD_MSG_1,
                    "odmadd:  Cannot open class:  %s, stanza line: %d  %s\n"),
                              class_name,stanza_line_number,stanza_input_file);
            exit(211);
         } /* endif */

         new_entry = (char *) malloc(Class->structsize);
         if (new_entry == (char *) NULL)
         {
            TRC("odmadd","new_entry malloc failed! %d",Class->structsize,"","");
            fprintf(stderr,
                    catgets(scmc_catd, MS_odmadd, ADD_MSG_2,
                    "odmadd:  Malloc failed, stanza line: %d  %s\n") ,
                             stanza_line_number,stanza_input_file);
            exit(130);
         } /* endif */

         bzero(new_entry,Class->structsize);

         /*------------------------------------------------------------*/
         /* Since the variable length char (vchars) are stored in a    */
         /* separate buffer rather than in the structure itself,       */
         /* go through the class info and malloc space for the vchars  */
         /*------------------------------------------------------------*/

         for (descriptor_index = 0; descriptor_index < Class->nelem;
                                                  descriptor_index++)
         {
            if ((Class->elem)[descriptor_index].type == ODM_VCHAR)
            {
               vchar_location =
                    (char **)
                      (new_entry + (Class->elem)[descriptor_index].offset);

               *vchar_location = (char *) NULL;
            } 
         } /* endfor */

         while (TRUE)
         {

           /*-------------------------------------------*/
           /* Find the name of the next descriptor      */
           /*-------------------------------------------*/

           TRC("obj_find_string","looking for stanza descrip NAME","","","");
           descrip_name = get_value_from_string((char *) NULL,"=\n",
                                            skip_spaces,&terminating_char);


           if (descrip_name == (char *) NULL ||
               (*descrip_name == '\0' && terminating_char == '\0'))
           {
              /*---------------------------*/
              /* Found the end-of-stanza.  */
              /*---------------------------*/
              TRC("obj_find_string","found_stanza_end!!","","","");
              break;
           }

           else if (terminating_char != '=')
           {
              /*---------------------*/
              /* free(descrip_name); */
              /*---------------------*/
              TRC("odmadd","could not find name!!","","","");
              fprintf(stderr,
                      catgets(scmc_catd, MS_odmadd, ADD_MSG_3,
              "odmadd:  Could not find descriptor name, stanza line: %d  %s\n"),
                     stanza_line_number,stanza_input_file);

              /*------------------------*/
              /* odmerrno = ODM_PHRASE; */
              /*------------------------*/
              exit(76);
            }

            else
            {
               TRC("odmadd","descriptor NAME is %s",descrip_name,"","");
            } 


            /*-------------------------------------------*/
            /* Determine the offset for this descriptor. */
            /*-------------------------------------------*/

            for (descriptor_index = 0; descriptor_index < Class->nelem;
                                                    descriptor_index++)
            {
               if (strcmp((Class->elem)[descriptor_index].elemname,
                                                 descrip_name) == 0)
               {
                  /*-------------*/
                  /* Found name! */
                  /*-------------*/
                  TRC("odmadd","found table name %d",descriptor_index,"","");
                  break;
               }
            } /* endfor */

            if (descriptor_index >= Class->nelem)
            {
               TRC("odmadd","could not find name %s",descrip_name,"","");
               fprintf(stderr,
                       catgets(scmc_catd, MS_odmadd, ADD_MSG_4,
                   "odmadd:  Invalid descriptor: %s  stanza line:  %d  %s\n"),
                          descrip_name,stanza_line_number,stanza_input_file );

               exit(210);
            } /* endif */

            /*-----------------------------------*/
            /* Get the value for this descriptor */
            /*-----------------------------------*/

            descrip_value = get_value_from_string((char *) NULL,"\n",
                                       skip_spaces, &terminating_char);

            if (descrip_value == (char *) NULL)
            {
               /*-----------------------------------------------*/
               /* Could not find the value for this descriptor. */
               /*-----------------------------------------------*/
               fprintf(stderr,
                       catgets(scmc_catd, MS_odmadd, ADD_MSG_5,
                       "odmadd:  Invalid value, stanza line:  %d  %s\n") ,
                                 stanza_line_number,stanza_input_file );
               exit(33);
            } /* endif */

            TRC("odmadd","-->   Descriptor value is %s",descrip_value,"","");

            /*-------------------------------------------------------*/
            /* We now have the descriptor name and its value.  Store */
            /* it in the structure.                                  */
            /*-------------------------------------------------------*/

            descriptor_offset = new_entry +
                                Class->elem[descriptor_index].offset;

            switch ((Class->elem)[descriptor_index].type)
            {
               case ODM_LINK:

                     /* skip pointers */
                     descriptor_offset = 
                              descriptor_offset + 2 * sizeof (char *);
                     /*-----------------------------------*/
                     /* Fall through and save the string. */
                     /*-----------------------------------*/

               case ODM_CHAR:
               case ODM_LONGCHAR:
               case ODM_METHOD:
                     *descriptor_offset = '\0';
                     strncat(descriptor_offset,descrip_value,
                             (Class->elem)[descriptor_index].size - 1);

                     TRC("odmadd","storing character %s", 
                                  descriptor_offset,"","");
                     break;

               case ODM_VCHAR:
                     /*------------------------------------------------*/
                     /* Since the vchars are not put directly into the */
                     /* structure, we need to save the string in the   */
                     /* buffer pointed to by descriptor_offset.        */
                     /*------------------------------------------------*/

                     vchar_location =  (char **) descriptor_offset;
                     if (*vchar_location != NULL)
                     {
                        /*-----------------------------------------------*/
                        /* When *vchar_location != NULL occurs, this     */
                        /* means that the user has two lines in the      */
                        /* stanza for the same value.  Free the previous */
                        /* buffer and malloc a new buffer so we only     */
                        /* keep the last value.                          */
                        /*-----------------------------------------------*/
                        free(*vchar_location);
                        *vchar_location = NULL;
                     } /* endif */

                     *vchar_location =
                            (char *) malloc(strlen(descrip_value) + 1);

                     if (*vchar_location == NULL)
                     {
                        TRC("odmadd","vchar malloc failed! %d",
                               strlen(descrip_value) + 1,"","");

                        fprintf(stderr,
                           catgets(scmc_catd, MS_odmadd, ADD_MSG_6,
                 "odmadd:  Could not malloc for vchar, stanza line: %d  %s\n"),
                                         stanza_line_number,stanza_input_file);

                        exit(76);
                     } /* endif */

                     strcpy(*vchar_location,descrip_value);

                     TRC("odmadd","storing character %s",descrip_value,"","");
                     break;

               case ODM_BINARY:
                     TRC("odmadd","converting hex %s",descrip_value,"","");
 
                     returnstatus = convert_to_binary(descrip_value,
                                        descriptor_offset,
                                        (Class->elem)[descriptor_index].size);

                     if (returnstatus < 0)
                     {
                         TRC("odmadd","could not convert hex","","","");
                         fprintf(stderr,
                            catgets(scmc_catd, MS_odmadd, ADD_MSG_7,
                       "odmadd:  Invalid binary value, stanza line:  %d  %s\n"),
                                        stanza_line_number,stanza_input_file );

                         exit(73);
                     } /* endif */
                     break;

               case ODM_LONG:
                     *(long *)descriptor_offset=
                                  strtol(descrip_value,&first_err,NULL);
                     if (*first_err)
                     {
                        TRC("odmadd","bad long value %s",descrip_value,"","");
                        fprintf(stderr,
                                catgets(scmc_catd, MS_odmadd, ADD_MSG_8,
                        "odmadd:  Invalid long value, stanza line:  %d  %s\n"),
                                        stanza_line_number,stanza_input_file );
                        exit(53);
                     } /* endif */

                     TRC("odmadd","storing long %ld",
                               *(long *)descriptor_offset,"","");
                     break;

               case ODM_ULONG:
                     *(unsigned long *)descriptor_offset=
                                    strtoul(descrip_value,&first_err,NULL);
                     if (*first_err)
                     {
                        TRC("odmadd","bad ulong value %s",descrip_value,"","");
                        fprintf(stderr,
                                catgets(scmc_catd, MS_odmadd, ADD_MSG_8,
                        "odmadd:  Invalid ulong value, stanza line:  %d  %s\n"),
                                         stanza_line_number,stanza_input_file );
                        exit(53);
                     } /* endif */

                     TRC("odmadd","storing ulong %ld",
                               *(unsigned long *)descriptor_offset,"","");
                     break;

               case ODM_DOUBLE:
                     *(double *)descriptor_offset=
                                     strtod(descrip_value,&first_err);
                     if (*first_err)
                     {
                        TRC("odmadd","bad double value %s",descrip_value,"","");
                        fprintf(stderr,
                                catgets(scmc_catd, MS_odmadd, ADD_MSG_9,
                            "odmadd:  Invalid double, stanza line:  %d  %s\n"),
                                         stanza_line_number,stanza_input_file );
                         exit(68);
                     } /* endif */

                     TRC("odmadd","storing long %ld",
                               *(long *)descriptor_offset,"","");
                     break;

               case ODM_SHORT:
                     *(short *)descriptor_offset= (short)
                                     strtol(descrip_value,&first_err,NULL);
                     if (*first_err)
                     {
                        TRC("odmadd","bad short value %s",descrip_value,"","");
                        fprintf(stderr,
                                catgets(scmc_catd, MS_odmadd, ADD_MSG_10,
                       "odmadd:  Invalid short value, stanza line:  %d  %s\n"),
                                        stanza_line_number,stanza_input_file );
                        exit(93);
                     } /* endif */

                     TRC("odmadd","storing short %d",
                                *(short *)descriptor_offset,"","");
                     break;

              default:
                     TRC("odmadd","bad type!! %s",
                               (Class->elem)[descriptor_index].elemname,
                               "type %d",
                               (Class->elem)[descriptor_index].type);

                     fprintf(stderr,
                             catgets(scmc_catd, MS_odmadd, ADD_MSG_11,
       "odmadd: unknown element type for descriptor %s, stanza line: %d  %s\n"),
                             (Class->elem)[descriptor_index].elemname,
                             stanza_line_number,
                             stanza_input_file);
                     exit(21);
            }

            number_of_values++;

            /*----------------------------------------*/
            /*           if (descrip_value)           */
            /*                   free(descrip_value); */
            /*                                        */
            /*           if (descrip_name)            */
            /*                   free(descrip_name);  */
            /*----------------------------------------*/

         } /* endwhile */

         if ((returnstatus = odm_add_obj(Class,new_entry)) < 0)
         {
            char *msg;
            if (odm_err_msg(odmerrno, &msg) == 0)
               fprintf(stderr, msg);
            else
               fprintf(stderr,
         	    catgets(scmc_catd, MS_odmadd, ADD_MSG_12,
			    "odmadd: Could not add to class, "
			    "error: %d, stanza line: %d  %s\n"),
			    odmerrno, stanza_line_number, stanza_input_file);

            exit(-1);
         }

         free(new_entry);

         /*------------------------------------------------------------*/
         /* Since the variable length char (vchars) are stored in a    */
         /* separate buffer rather than in the structure itself,       */
         /* go through the class info and free   space for the vchars */
         /*------------------------------------------------------------*/

         for (descriptor_index = 0; descriptor_index < Class->nelem;
                                                  descriptor_index++)
         {
            if ((Class->elem)[descriptor_index].type == ODM_VCHAR)
            {
               vchar_location = (char **)
                     (new_entry + (Class->elem)[descriptor_index].offset);

               if (*vchar_location != NULL)
               {
                  free(*vchar_location);
                  *vchar_location = NULL;
               }
            } /* endif */
         } /* endfor */
      } /* if OpenClass */

      /*--------------------------------*/
      /* if (stanza != (char *) NULL)   */
      /*                  free(stanza); */
      /*--------------------------------*/


   } /* endwhile */

   /*------------------------------------*/
   /* returnstatus = close_class(Class); */
   /*------------------------------------*/

   if (stanza_length < 0)
   {
      TRC("odmadd:","Stanza length is negative!! %d",stanza_length,"","");
      fprintf(stderr, catgets(scmc_catd, MS_odmadd, ADD_MSG_13,
                      "odmadd:  Invalid stanza, stanza line:  %d  %s\n"),
                                     stanza_line_number,stanza_input_file);
      exit(62);
   } /* endif */

   return(0);
}  /* main */

/****************************************************************************
 * NAME: PRINT_SYNTAX
 *
 * FUNCTION:
 *
 *      Prints the correct syntax for the odmadd commmand.
 *
 * RETURNS:
 *        No value is returned.
 ***************************************************************************/
int print_syntax()
{
        fprintf(stderr, catgets(scmc_catd, MS_odmadd, ADD_MSG_14,
"usage: odmadd [stanza file]\n\
\tAdds objects to an object class.\n\n\
\tThe <stanza file> should be in the form:\n\n\
\t\t<class name 1>:\n\
\t\t\t<descriptor name A> = <descriptor value>\n\
\t\t\t<descriptor name B> = <descriptor value>\n\
\t\t<class name 2>:\n\
\t\t\t<descriptor name C> = <descriptor value>\n\
\t\t\t<descriptor name D> = <descriptor value>\n\n\
\t\t . . .\n"));
} /* end of print_syntax */

/****************************************************************************
 * NAME:      OPEN_INPUT_FILE  
 *
 * FUNCTION:  Attempt to open a stanza file.  This routine will add a .add
 *            extension to the file name and attempt to open the file. If
 *            that fails then the original file name is opened.
 *
 * RETURNS:   A pointer to a FILE structure of the opened file if successful,
 *            -1 otherwise.
 ***************************************************************************/

FILE *open_input_file(original_file_name)
char *original_file_name; /* Name of the input file.  May have .add extension */
{

  FILE *file_ptr;
  char new_file_name[MAX_STANZA_NAME];
  char *extension_location;

  TRC("open_input_file","filename is %s",original_file_name,"","");

  if (strlen(original_file_name) > MAX_STANZA_NAME - 5)
  {
     TRC("open_input_file","Name too long","","","");
     fprintf(stderr, catgets(scmc_catd, MS_odmadd, ADD_MSG_15,
             "odmadd:  Name of inputfile is too long: %s") ,
             original_file_name,MAX_STANZA_NAME - 5);
     exit(28);
  } /* endif */

  strcpy(new_file_name,original_file_name);

  /*-----------------------------------------------------------*/
  /* Determine if the user already added the ".add" extension. */
  /*-----------------------------------------------------------*/
  extension_location = strrchr(original_file_name,'.');

  if (!extension_location ||
      strcmp(extension_location,".add") != 0)
  {
     /*-------------------------------------------------------*/
     /* User has not entered stanza_name.add as the filename. */
     /* We need to append the .add extension.                 */
     /*-------------------------------------------------------*/
     strcat(new_file_name,".add");

  } /* endif */

  TRC("open_input_file","Trying to open file %s",new_file_name,"","");

  file_ptr = fopen(new_file_name,"r");
  if (file_ptr == (FILE *) NULL)
  {
     /*-----------------------------------------------------------*/
     /* We could not open the file with  the .add extension.  Try */
     /* the original filename.                                    */
     /*-----------------------------------------------------------*/
     TRC("open_input_file","cannot open file with .add!","","","");

     TRC("open_input_file",
         "Trying to open file %s",original_file_name,"","");

     file_ptr = fopen(original_file_name,"r");

     if (file_ptr == (FILE *) NULL)
     {
        TRC("open_input_file","cannot open file!","","","");

        fprintf(stderr,
                catgets(scmc_catd, MS_odmadd, ADD_MSG_16,
                "odmadd:  Cannot open file:  %s\n") ,new_file_name);
        print_syntax();
        exit(5);
     } /* endif */

  } /* endif */

  return(file_ptr);

} /* end of open_input_file */
