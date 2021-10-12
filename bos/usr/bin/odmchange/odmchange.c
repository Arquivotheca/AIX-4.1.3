static char sccsid[] = "@(#)61  1.20.1.4  src/bos/usr/bin/odmchange/odmchange.c, cmdodm, bos411, 9434B411a 8/22/94 17:35:08";
/*
 *   COMPONENT_NAME: CMDODM
 *
 *   FUNCTIONS: MSGSTR
 *		main
 *		update_values
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
#include <fcntl.h>
#include <unistd.h>    /* access system call constants */
#include <string.h>
#include <odmi.h>
#include <math.h>
#include "odmcmd.h"

#define  MF_ODMCMD   "odmcmd.cat"
#define  CHA_MSG_SET 3

#include <locale.h>
#include <nl_types.h>
#define MSGSTR(num,str) catgets(catd,CHA_MSG_SET,num-5000,str) /* MSG */

#define CHANGE_STRLEN             5001
#define CHANGE_USAGE              5002
#define CHANGE_NONAME             5003
#define CHANGE_FNAMLEN            5004
#define CHANGE_ACCESS             5005
#define CHANGE_INIT_ERR           5006
#define CHANGE_OPEN_ERR           5007
#define CHANGE_STANZA_NOTFOUND    5008
#define CHANGE_INVALID_STANZA     5009
#define CHANGE_BAD_CRITERIA       5010
#define CHANGE_BAD_DATA           5011
#define DESC_NOT_FOUND            5012
#define INVALID_DESC              5013
#define INVALID_VALUE             5014
#define INVALID_BINARY            5015
#define INVALID_NUMERIC           5016
#define UNKNOWN_TYPE              5017
#define CHANGE_NAMLEN             5018


nl_catd 	catd;
/*
 * NAME: odmchange (main)
 *
 * FUNCTION: command to change values inside of a specified ODM database
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is a shell command, and executes totally as a separate
 *      process.  Parameters can/must (see notes) be passed to it for
 *      it to function correctly.
 *
 * NOTES:
 *      The syntax for the command is as follows:
 *
 *      odmchange -o classname [-q criteria ] [filename] [-T] [-D]
 *
 *      where   classname is the object class name that holds the object.
 *
 *              criteria is the SQL search criteria
 *
 *              filename is the name of an input file that contains the
 *                      the new values.
 *
 *              -T is for trace mode
 *              -D is for debug mode
 *
 * RETURNS: a 0 is returned if the create was successful, otherwise
 *          a non 0 value is returned that is unique for the exit call.
 *
 */

extern int odmerrno;
extern int stanza_line_number;
int cmddebug = {0};
int trace;


main (argc, argv)
int argc;
char *argv[];
{
    char filename [MAXNAMLEN];
    char classname [MAXNAMLEN];
    int argcnt;
    char crit[MAX_ODMI_CRIT + 1], tmpclassname[MAXNAMLEN + 1],
                                       reppath [MAX_ODM_PATH];
    char *criteria;
    int len;
    struct Class *class_symbol_ptr;
    char *class_structure, *object_ptr, *startpath;
    FILE *file_ptr;
    long filesize;
    char *stanza, *class_name, terminating_char, *slash;
    int stanza_length, skip_spaces, number, counter, lockid;
    struct listinfo list_info;

    int option_letter;
    extern int optind;
    extern char *optarg;
    int returncode;

    /* FILE *fopen (); */
    struct Class *odm_mount_class ();
    char *get_value_from_string (), *odm_set_path ();
    int get_ascii_phrase ();

    /*
     * init various variables
     */

#ifdef _DEBUG
    cmddebug = 1;
#endif

#ifndef R5A
    (void) setlocale (LC_ALL, "");
    catd = catopen(MF_ODMCMD, NL_CAT_LOCALE);       /* Defect 116034 */
#endif

    classname [0] = '\0';
    crit [0]  = '\0';
    criteria = (char *)&crit;
    filename [0]  = '\0';
    tmpclassname [0] = '\0';
    reppath [0] = '\0';
    argcnt = 0;

    filesize = legal_size (classname);

    /* Get the command line arguments.  Valid options are o, q */

    while ((option_letter = getopt (argc,argv, "o:q:TD")) != EOF)
    {
        switch (option_letter)
        {
        case 'o':

            /* Object name specified */

            if (strlen(optarg) >= filesize)
            {
                (void) fprintf (stderr, MSGSTR(CHANGE_NAMLEN,
                        "odmchange: class name too long %d\n"),filesize);
                (void) exit (ODMCMDOPTERR);
            }
            (void) strcpy(classname, optarg);
            argcnt++;
            break;

        case 'q':

            /* Query string specified */

            if ((len=strlen(optarg)) > MAX_ODMI_CRIT - 1)
            {
		criteria = (char *)malloc(len + 1);
		if ( criteria == (char *)NULL)
		{
			perror("odmchange");
                	(void) exit (ODMCMDOPTERR);
		}
            }
            (void) strcpy(criteria, optarg);
            break;

        case 'T':

            /* Hidden flag from users, for tracing in the library */

            trace = 1;
            break;

        case 'D':

            /* Hidden flag from users, for generating debug from the */
            /* command                                               */

            cmddebug = 1;
            break;

        case '?':
        default:

            /* Invalid option selected */

            exit(ODMCMDOPTERR);  /* option error */
        } /* end case */
    }

    /* If no Object class is specified, show syntax */

    if (!argcnt)
    {
        (void) fprintf(stderr, MSGSTR(CHANGE_USAGE,
                "usage: odmchange -o classname [-q criteria] [filename]\n"));
        (void) exit (ODMCMDOPTERR);
    }

    /* If Object class name is NULL, then present error message */

    if (classname[0] == '\0')
    {
        (void) fprintf(stderr, MSGSTR(CHANGE_NONAME,
                "odmchange: no object class name specified\n"));
        (void) exit (ODMCMDOPTERR);
    }

    /* Is there a file name specified on the command line ? */

    if (argv[optind] != NULL )
    {
        /* Check to see if the filename is valid */

        if (strlen(argv[optind]) >= filesize)
        {
           (void) fprintf(stderr, MSGSTR(CHANGE_FNAMLEN,
                "odmchange: file name to long %d\n"),filesize);
           (void) exit(ODMCMDOPTERR); /* SMU Defect 48904 */
        }

        /* Valid filename, copy it into local variable */
        (void) strcpy(filename, argv[optind]);
    }

    /* Set the path to the object so that deletion takes place in the */
    /* correct directory                                              */

    slash = strrchr(classname, '/');
    if (slash != NULL)
    {
        (void) strcpy(tmpclassname, slash + 1);
        *slash = '\0';
        (void) strcpy(reppath, classname);
    }
    else
    {
        (void) strcpy(tmpclassname, classname);
        reppath [0] = '\0';
    }

    startpath = odm_set_path (reppath);

    if (filename[0] != '\0')
    {
        /* check filename */
        if (access (filename, R_OK))
        {
           (void) fprintf(stderr, MSGSTR(CHANGE_ACCESS,
                "odmchange: file '%1$s not accessible or does not exist\n"),
                filename);
           (void) exit (ODMCMDFNAME);
        }
    }

    /* Setup call to odminit */

    returncode = odm_initialize();
    if (returncode < 0)
    {
        (void) fprintf(stderr, MSGSTR(CHANGE_INIT_ERR,
                "odmchange: init error\n"));
        (void) exit (ODMCMDODMERR);
    }

    /* Open object class */

    class_symbol_ptr = odm_mount_class (tmpclassname);
    if ((int) class_symbol_ptr < 0)
    {
        (void) fprintf (stderr, MSGSTR(CHANGE_OPEN_ERR,
                "odmchange: error opening class '%1$s'\n"), tmpclassname);
        (void) odm_terminate ();
        (void) exit (ODMCMDODMERR);
    }

    /* Get data for change */

    if (filename [0] == '\0')
        file_ptr = stdin;
    else
        file_ptr = fopen (filename, "r");

    skip_spaces = FALSE;
    stanza_line_number = 0;
    while ((stanza_length = get_ascii_phrase (file_ptr, STANZA, &stanza)) > 0)
    {
        /* Get the first value from the phrase.  Subsequent calls will be   */
        /* made with the value NULL for search string since the 'get_value' */
        /* routine keeps track of where it was last.                        */

        class_name = get_value_from_string (stanza, ":\n", skip_spaces,
                &terminating_char);
        if ((class_name == (char *) NULL) || (terminating_char != ':'))
        {
            /* Couldn't find a class name in first line of file */

            (void) fprintf(stderr, MSGSTR(CHANGE_STANZA_NOTFOUND,
            "odmchange: cannot find class name in stanza, stanza line: %1$d\n"),
               stanza_line_number);
            (void) exit (107);
        }
        while ((stanza_length > 0) && (strcmp (classname, class_name) != 0))
        {
            /* Find the matching stanza entry for the classname specified */
            /* on the command line by the user                            */

            stanza_length = get_ascii_phrase (file_ptr, STANZA, &stanza);
            class_name = get_value_from_string (stanza, ":\n", skip_spaces,
                &terminating_char);
        }
        if (stanza_length < 0)
        {
            (void) fprintf(stderr, MSGSTR(CHANGE_INVALID_STANZA,
               "odmchange: invalid stanza, stanza line: %1$d\n"),
                stanza_line_number);
            (void) exit (62);
        }

        /* Found correct classname entry in stanza file, ready to proceed */

        number = class_symbol_ptr -> nelem;
        class_structure = (char *) odm_get_list (class_symbol_ptr, criteria, 
                                        &list_info, number, 1);

        if (class_structure < (char *) 0)
        {
            (void) fprintf (stderr, MSGSTR(CHANGE_BAD_CRITERIA,
                "odmchange: retrieval error caused by criteria: %1$s\n"),
                 criteria);
            (void) exit (1000);
        }
        if (class_structure != (char *) NULL)
        {
            /* Change appropriate values */
            returncode = update_values (class_structure, class_symbol_ptr,
                                list_info.num);



            /* Put changed values back into the odm class */
            for (counter = 0; counter < list_info.num; counter++)
            {
                object_ptr = class_structure;
                object_ptr = object_ptr + (counter *
                                        (class_symbol_ptr -> structsize));


                returncode = odm_change_obj (class_symbol_ptr, object_ptr);
                if (returncode < 0)
                {
                    (void) fprintf(stderr, MSGSTR(CHANGE_BAD_DATA,
                      "odmchange: error changing data in object class: %1$s\n"),
                            classname);
                    (void) exit (22);
                }
            }
        }
    }

    if (stanza_length < 0)
    {
        (void) fprintf(stderr,MSGSTR(CHANGE_INVALID_STANZA,
                "odmchange: invalid stanza, stanza line: %d\n"),
                stanza_line_number);
        (void) exit (62);
    }

    /* Normal termination */

    (void) odm_free_list (class_structure, &list_info);
    (void) odm_terminate ();
    (void) exit (0);   /* successful exit */
}

/*
 * NAME: update_values
 *
 * FUNCTION: Parses the stanza line and updates the appropriate fields in
 *      the ODM file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Subroutine called by odmchange.
 *
 * NOTES: This routine makes use of stanza conversion routines and then
 *      loops through all the entries returned by odm_get_list changing
 *      the appropriate value in each record.
 *
 * RETURNS: Routine returns 0 if successful, exits to the shell on error
 *      with a unique error code.
 *
 */

int update_values (class, class_symbol_ptr, num_objects)
struct Class *class_symbol_ptr;         /* CLASS_SYM that contains all the
                                           header information about a class */
char *class;                            /* Pointer to the list of records
                                           meeting search criteria for change */
int num_objects;                        /* Number of records meeting search
                                           criteria for change */
{
     int skip_spaces, offset_index, returnstatus, structsize;
     long tmp_long;
     unsigned long tmp_ulong;
     double tmp_dbl;
     char terminating_char, *retchar;
     char **vchar_location, *new_string;
     char *descriptor_name, *descriptor_value, *descriptor_offset;
     char *index_ptr;
     int loop, retcode;

     skip_spaces = TRUE;

     while (TRUE)
     {
        /* Get the descriptor name of the field to be changed */

        retchar = get_value_from_string ((char *) NULL, "=\n",
                        skip_spaces, &terminating_char);
        if (retchar == (char *) NULL)
                break;
        else if (terminating_char != '=')
        {
                (void) fprintf(stderr,MSGSTR(DESC_NOT_FOUND,
               "odmchange: could not find descriptor name, stanza line: %1$d"),
                        stanza_line_number);
                (void) exit (72);
        }
        descriptor_name = (char *)malloc(strlen(retchar) + 1);
        (void) strcpy(descriptor_name, retchar);

        /* Determine the offset for this descriptor */

        for (offset_index = 0; offset_index < class_symbol_ptr -> nelem;
                                                        offset_index++)
        {
                if (strcmp((class_symbol_ptr -> elem)[offset_index].elemname,
                                descriptor_name) == 0)
                break;
        }

        if (offset_index >= class_symbol_ptr -> nelem)
        {
                (void) fprintf(stderr,MSGSTR(INVALID_DESC,
                   "odmchange: invalid descriptor: %1$s  stanza line: %2$d\n"),
                        descriptor_name, stanza_line_number);
                (void) odm_terminate();
                (void) exit (210);
        }

        /* Get the value for the descriptor */

        descriptor_value = get_value_from_string ((char *) NULL, "\n",
                skip_spaces, &terminating_char);
        if (descriptor_value == (char *) NULL)
        {
                (void) fprintf(stderr,MSGSTR(INVALID_VALUE,
                        "Invalid value, stanza line: %1$d\n"),
                        stanza_line_number);
                (void) odm_terminate();
                (void) exit (33);
        }

        /* We now have the descriptor name and its value.  Store it in */
        /* the structure.                                              */

        descriptor_offset = class + class_symbol_ptr->elem[offset_index].offset;

        /* Determine size of structure, and which type the descriptor is to  */
        /* handle storage correctly                                          */

        structsize = class_symbol_ptr -> structsize;
        switch ((class_symbol_ptr -> elem)[offset_index].type)
        {
                case ODM_LINK:
                        /* skip pointers */
                        descriptor_offset = descriptor_offset + 2 *
                                sizeof (char *);

                        /* Fall through and save the string */
                case ODM_CHAR:
                case ODM_LONGCHAR:
                case ODM_METHOD:

                        index_ptr = descriptor_offset;

                        for (loop = 0; loop < num_objects; loop++)
                        {
                                *index_ptr = '\0';
                                (void) strncat(index_ptr,descriptor_value,
                                (class_symbol_ptr -> elem)[offset_index].size
                                 - 1);
                                index_ptr += structsize;
                        }
                        break;

                case ODM_VCHAR:
                        /* Since the vchars are not put directly into the */
                        /* structure, we need to save the string in the   */
                        /* buffer pointed to by descriptor_offset.        */

                        index_ptr = descriptor_offset;
                        for (loop = 0; loop < num_objects; loop++)
                        {
                                new_string = (char *)malloc(strlen
                                                (descriptor_value) + 1);
                                (void) strcpy(new_string, descriptor_value);
                                (void) free (* (char **) index_ptr);
                                * (char **) index_ptr = new_string;
                                index_ptr += structsize;
                        }
                        break;

                case ODM_BINARY:
                        index_ptr = descriptor_offset;
                        for (loop = 0; loop < num_objects; loop++)
                        {
                                returnstatus =
                                        convert_to_binary (descriptor_value,
                                                 index_ptr,
                                (class_symbol_ptr -> elem)[offset_index].size);
                                if (returnstatus < 0)
                                {
                                        (void) fprintf(stderr,
                                                MSGSTR(INVALID_BINARY,
                       "odmchange: invalid binary value, stanza line: %1$d\n"),
                                        stanza_line_number);
                                        (void) odm_terminate();
                                        (void) exit (73);
                                }
                                index_ptr += structsize;
                        }
                        break;

                case ODM_DOUBLE:
                        /* Use 'strtod' here so that if a bad value is passed */
                        /* in, it can be caught.  Using 'atof' would return 0 */
                        /* on error                                           */
                        tmp_dbl = strtod (descriptor_value, &retchar);
                        if (strcmp(descriptor_value, retchar) == 0)
                        {
                                (void) fprintf(stderr,
                                        MSGSTR(INVALID_NUMERIC,
                      "odmchange: invalid numeric value, stanza line: %1$d\n"),
                                        stanza_line_number);
                                (void) odm_terminate();
                                (void) exit (174);
                        }
                        index_ptr = descriptor_offset;
                        for (loop = 0; loop < num_objects; loop++)
                        {
                                *(double *) index_ptr = tmp_dbl;
                                index_ptr += structsize;
                        }
                        break;

                case ODM_LONG:
                case ODM_SHORT:
                        /* Use 'strtol' here so that if a bad value is passed */
                        /* in, it can be caught.  Using 'atoi' would return 0 */
                        /* on error                                           */
                        tmp_long = strtol (descriptor_value, &retchar, 10);
                        if (strcmp(descriptor_value, retchar) == 0)
                        {
                                (void) fprintf(stderr,
                                        MSGSTR(INVALID_NUMERIC,
                     "odmchange: invalid numeric value, stanza line: %1$d\n"),
                                        stanza_line_number);
                                (void) odm_terminate();
                                (void) exit (174);
                        }
                        index_ptr = descriptor_offset;
                        if ((class_symbol_ptr -> elem)[offset_index].type
                                == ODM_LONG)
                        {
                                for (loop = 0; loop < num_objects; loop++)
                                {
                                        *(long *) index_ptr = tmp_long;
                                        index_ptr += structsize;
                                }
                        }
                        else
                        {
                                for (loop = 0; loop < num_objects; loop++)
                                {
                                        *(short *) index_ptr = tmp_long;
                                        index_ptr += structsize;
                                }
                        }
                        break;

                case ODM_ULONG:
                        tmp_ulong = strtoul (descriptor_value, &retchar, 10);
                        if (strcmp(descriptor_value, retchar) == 0)
                        {
                                (void) fprintf(stderr,
                                        MSGSTR(INVALID_NUMERIC,
                     "odmchange: invalid numeric value, stanza line: %1$d\n"),
                                        stanza_line_number);
                                (void) odm_terminate();
                                (void) exit (174);
                        }
                        index_ptr = descriptor_offset;
                        if ((class_symbol_ptr -> elem)[offset_index].type
                                == ODM_ULONG)
                        {
                                for (loop = 0; loop < num_objects; loop++)
                                {
                                        *(unsigned long *) index_ptr = tmp_ulong;
                                        index_ptr += structsize;
                                }
                        }
                        else
                        {
                                for (loop = 0; loop < num_objects; loop++)
                                {
                                        *(short *) index_ptr = tmp_ulong;
                                        index_ptr += structsize;
                                }
                        }
                        break;

                default:
                        /* Invalid type has been passed through */
                        (void) fprintf(stderr,MSGSTR(UNKNOWN_TYPE,
                                "odmchange: unknown element type for %1$s\n"),
                        (class_symbol_ptr -> elem)[offset_index].elemname);
                        (void) odm_terminate();
                        (void) exit (21);
        }
    }

    return (0);
}
