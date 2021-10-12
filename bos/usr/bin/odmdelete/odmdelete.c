static char sccsid[] = "@(#)64  1.16.1.3  src/bos/usr/bin/odmdelete/odmdelete.c, cmdodm, bos411, 9434B411a 8/22/94 17:35:29";
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

#define  MF_ODMCMD    "odmcmd.cat"
#define  DEL_MSG_SET  4

#include <locale.h>
#include <nl_types.h>
#define MSGSTR(num,str) catgets(catd,DEL_MSG_SET,num-5000,str) /* MSG */
nl_catd catd;

#define DEL_STRLEN     5001
#define DEL_USAGE      5002
#define DEL_NONAME     5003
#define DEL_INIT_ERR   5004
#define DEL_OPEN_ERR   5005
#define DEL_FAILED     5006
#define DEL_NUM_OBJS   5007
#define DEL_NAMLEN     5008


/*
 * NAME: odmdelete (main)
 *
 * FUNCTION: Deletes entries from an ODM object class based on criteria
 *      entered by the user
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is a command executed at the shell
 *
 * NOTES:
 *      The syntax for the command is as follows:
 *
 *      odmdelete -o classname [-q criteria]
 *
 *      where   classname is the object class name that contains the object.
 *
 *              criteria is the SQL format of the search criteria
 *
 *              -D is for debug mode
 *              -T is for trace mode
 *
 *
 * RETURNS: a 0 is returned if the delete was successful, otherwise
 *      a non 0 value is returned that is unique for the exit call.
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
    struct Class *class_symbol_ptr, *odm_mount_class ();

    char crit[MAX_ODMI_CRIT + 1], *slash, *startpath;
    char *criteria;
    char classname[MAXNAMLEN], tmpclassname[MAXNAMLEN], reppath[MAX_ODM_PATH + 1];
    long filesize;

    int option_letter;
    extern int optind;
    extern char *optarg;

    int returncode, lockid;
    int argcnt, i;
    int len;

    char *odm_set_path ();

    /*
     * init various variables
     */

#ifdef _DEBUG
    cmddebug = 1;
#endif

#ifndef R5A
    (void) setlocale (LC_ALL, "");
    catd = catopen (MF_ODMCMD, NL_CAT_LOCALE);    /* Defect 116034 */
#endif

    classname[0] = '\0';
    crit[0]  = '\0';
    criteria = (char *)&crit;
    tmpclassname [0] = '\0';
    reppath [0] = '\0';
    argcnt = 0;


    /* Get the system file size to be used for all name length comparisons */

    filesize = legal_size (classname);

    /*
     * Get the command line arguments.
     * Valid options are o, q, T and D.
     */
    while ((option_letter = getopt(argc,argv, "o:q:TD")) != EOF)
    {
        switch (option_letter)
        {
        case 'o':
            /* Get the object class name */
            if (strlen(optarg) >= filesize)
            {
                (void) fprintf (stderr, MSGSTR(DEL_NAMLEN,
                        "odmdelete:class name too long %d\n"), filesize);
                (void) exit (ODMCMDOPTERR);
            }
            (void) strcpy (classname, optarg);
            argcnt++;
            break;

        case 'q':
            /* Get the user supplied criteria for deletion */
            if ( (len = strlen(optarg)) >= MAX_ODMI_CRIT - 1 ) /* SMU Defect 48903 */
            {
		criteria = (char *)malloc(len + 1);
		if ( criteria == (char *)NULL)
		{
			perror("odmdelete");
                	(void) exit (ODMCMDOPTERR);
		}
            }
            (void) strcpy (criteria, optarg);
            break;

        case 'T':
            /* Hidden flag specifies traces to be printed out of library */
            trace = 1;
            break;

        case 'D':
            /* Hidden flag specifies debug from the command be printed */
            cmddebug = 1;
            break;

        case '?':
        default:
            /* Invalid option specified */
            exit(ODMCMDOPTERR);  /* option error */
        } /* end case */
    }

    if (!argcnt)
    {
        (void) fprintf (stderr, MSGSTR(DEL_USAGE,
                "usage: odmdelete -o object_class [-q criteria]\n"));
        (void) exit (ODMCMDOPTERR);
    }

    if (classname[0] == '\0')
    {
        (void) fprintf (stderr, MSGSTR(DEL_NONAME,
                "odmdelete: no object class name specified\n"));
        (void) exit (ODMCMDNOCLASS);
    }

    /*
     * Setup call to odminit
     */

    returncode = odm_initialize ();
    if (returncode < 0)
    {
        (void) fprintf (stderr, MSGSTR(DEL_INIT_ERR,
                "odmdelete: init error\n"));
        (void) exit (ODMCMDODMERR);
    }

    /* Set the path to the object so that deletion takes place in the */
    /* correct directory                                              */

    slash = strrchr (classname, '/');
    if (slash != NULL)
    {
        (void) strcpy (tmpclassname, slash + 1);
        *slash = '\0';
        (void) strcpy (reppath, classname);
    }
    else
    {
        (void) strcpy (tmpclassname, classname);
        reppath [0] = '\0';
    }

    /*
     * Open object class
     */

    class_symbol_ptr = odm_mount_class (tmpclassname);

    if ( (int) class_symbol_ptr < 0 )
    {
        (void) fprintf (stderr, MSGSTR(DEL_OPEN_ERR,
                "odmdelete: error opening class '%s'\n"), tmpclassname);
        (void) odm_terminate ();
        (void) exit (ODMCMDODMERR);
    }

    /*************************************************************/
    /* Setup call to odmdelete.                                  */
    /*************************************************************/

    /*
     * Setup criteria for search
     */

        returncode = odm_rm_obj (class_symbol_ptr, criteria);

    if (returncode < 0)
    {
        (void) fprintf (stderr, MSGSTR(DEL_FAILED,
                "odmdelete: delete failed\n"));
        (void) odm_terminate ();
        (void) exit (ODMCMDODMERR);
    }
    else
    {
        (void) fprintf (stdout, MSGSTR(DEL_NUM_OBJS, "%d objects deleted\n"),
                returncode);
    }

    (void) odm_terminate ();

    (void) exit(0);   /* successful exit */
}
