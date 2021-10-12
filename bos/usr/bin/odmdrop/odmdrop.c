static char sccsid[] = "@(#)65	1.14.1.3  src/bos/usr/bin/odmdrop/odmdrop.c, cmdodm, bos411, 9428A410j 12/2/93 11:40:51";
/*
 *   COMPONENT_NAME: CMDODM
 *
 *   FUNCTIONS: MSGSTR
 *		main
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
#include <string.h>
#include <odmi.h>
#include "odmcmd.h"
#include <unistd.h>

#define  ODM_CMDS_CAT  "odmcmd.cat"
#define  DROP_MSG_SET  2

#define  DROP_ACCESS   5001
#define  DROP_NAMLEN   5002
#define  DROP_NONAME   5003
#define  ODM_INIT_ERR  5004
#define  DROP_FAILED   5005
#define  DROP_USAGE    5006

#include <locale.h>
#include <nl_types.h>
#define MSGSTR(num,str) catgets(catd,DROP_MSG_SET,num-5000,str)
nl_catd catd;

/*
 * NAME: odmdrop (main)
 *
 * FUNCTION: This command physically deletes the entire object class from
 *      the disk
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is a command at the shell level with various options
 *
 * NOTES:
 *      The syntax for the command is as follows:
 *
 *      odmdrop -o classname -T -D
 *
 *      where   classname is the object class name to be deleted.
 *
 *              -T is for trace mode
 *              -D is for debug mode
 *
 * RETURNS: a 0 is returned if the operation was successful, otherwise
 *      a non 0 value is returned that indicates the type of error.
 *
 */

extern int errno;
int odmerrno;
int trace;
int cmddebug = {0};

main (argc, argv)
int argc;
char *argv[];
{
    struct Class *class_symbol_ptr;

    char classname [MAXNAMLEN], tmpclassname [MAXNAMLEN], reppath [MAXNAMLEN];
    char *slash, *startpath;
    long filesize;

    int option_letter;
    extern int optind;
    extern char *optarg;

    int returncode, lockid;
    int argcnt;

    char *odm_set_path ();
    struct Class *odm_mount_class ();

#ifdef _DEBUG
    cmddebug = 1;
#endif

#ifndef R5A
    /* Set the NLS locale */
    (void) setlocale (LC_ALL, "");
    catd = catopen (ODM_CMDS_CAT, NL_CAT_LOCALE);    /* Defect 116034 */
#endif

    classname[0] = '\0';
    tmpclassname[0] = '\0';
    reppath[0] = '\0';
    argcnt = 0;

    /* Determine maximum size of file names for underlying file system */

    filesize = legal_size (classname);

    /*
     * Get the command line arguments.
     * The valid option is o, where o has an argument.
     */
    while ((option_letter = getopt (argc, argv, "o:DT")) != EOF)
    {
        switch (option_letter)
        {
        case 'o':
            /* Get the object class specified off the command line */
            if (strlen (optarg) >= filesize)
            {
                (void) fprintf (stdout, MSGSTR(DROP_NAMLEN,
                        "odmdrop: class name too long\n"));
                (void) exit (ODMCMDOPTERR);
            }
            (void) strcpy (classname, optarg);
            argcnt++;
            break;

        case 'D':
            /* Internal flag that will specify that the command generate */
            /* debug information                                         */
            cmddebug = 1;
            break;

        case 'T':
            /* Internal flag that will specify that the library generate */
            /* debug information                                         */
            trace = 1;
            break;

        case '?':
        default:
            /* Invalid option selected */
            fprintf (stdout, MSGSTR(DROP_USAGE,
                        "usage: odmdrop -o class_name\n"));
            (void) exit (ODMCMDOPTERR);
        } /* end case */
    }
    if (!argcnt)
    {
        fprintf (stdout, MSGSTR(DROP_USAGE,"usage: odmdrop -o class_name\n"));
        (void) exit (ODMCMDOPTERR);
    }
    if (classname[0] == '\0')
    {
        (void) fprintf (stdout, MSGSTR(DROP_NONAME,
                "odmdrop: no object class name specified\n"));
        (void) exit (ODMCMDNOCLASS);
    }

    returncode = odm_initialize ();
    if (returncode < 0)
    {
        (void) fprintf (stdout, MSGSTR(ODM_INIT_ERR,
                "odmdrop: init error\n"));
        (void) exit (ODMCMDODMERR);
    }


    /* Set the path to the object so that deletion takes place in the */
    /* correct directory                                              */

    slash = strrchr (classname, '/');
    if (slash != NULL)
    {
        (void) strcpy (tmpclassname, slash + 1);
        *slash = '\0';
        startpath = odm_set_path (classname);
    }
    else
    {
        startpath = odm_set_path (NULL);
        (void) strcpy (tmpclassname, classname);
    }

    class_symbol_ptr = odm_mount_class (tmpclassname);
    if (class_symbol_ptr == (struct Class *) -1)
    {
        (void) fprintf (stdout, MSGSTR(DROP_ACCESS,
                        "odmdrop: error accessing class %s\n"), tmpclassname);
        (void) exit (22);
    }

    returncode = odm_rm_class (class_symbol_ptr);
    if (returncode < 0)
    {
        (void) fprintf (stdout, MSGSTR(DROP_FAILED,
                "odmdrop: drop failed for class '%s'\n"), tmpclassname);
        (void) odm_terminate ();
        (void) exit (ODMCMDODMERR);
    }

    free ((void *)startpath);

    (void) odm_terminate ();

    (void) exit (0);   /* successful exit */
}
