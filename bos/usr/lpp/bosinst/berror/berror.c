static char sccsid[] = "@(#) 25 1.33.2.18 src/bos/usr/lpp/bosinst/berror/berror.c, bosinst, bos412, 9445C412b 94/11/10 08:29:59";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: berror, display_options, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:    berror.c
 *
 * FUNCTION:     displays and logs error message text for the BOS
 *        installation process
 *
 * EXECUTION ENVIRONMENT:
 *
 *    The berror program is a command that is executed from
 *    a shell environment.
 *
 *    Flags:    -f function    Required - specifies name of failing routine
 *        -e errcode    Required - specifies the index in the error
 *                    message array of the error message
 *                    to be displayed
 *        -a action    Required - 1 to display error and return
 *                       2 to display error, prompt user for
 *                        continue, maintenance, reboot
 *        -r retcode    Optional - return code from the failing routine
 *
 *    Syntax:    berror [-f function ] -e errcode -a action [ -r retcode ]
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/termio.h>
#include <nl_types.h>

#define ERR_LINE_LEN 160
#define APPEND "a"

#define INVALID_ERROR_CODE 19
#define FUNC_NAME_REQUIRED 20
#define INVALID_ACTION_CODE 21
#define MENU 22
#define USAGE 23

nl_catd catfd;                                /* BosMneus message catalog fd   */
char *lang;
char *boottype;

/* char errbuf1[ERR_LINE_LEN]; */
char *errbuf1;

char *err_tab[] = {
/* 0 is a place holder, since NLS message numbers cannot begin with 0 */
"",
/* 1 */
"%s: Could not initialize %s.\nYou may want to run diagnostics on the listed disk(s).\n",
/* 2 */
"%s: Could not create the jfs log.\n",
/* 3 */
"%s: No paging devices could be found on the following disks:\n%s\n",
/* 4 */
"%s: Could not replay the jfs log.\n",
/* 5 */
"%s: Can not preserve customized object classes of previous system.\nContinuing.\n",
/* 6 */
"%s: After saving all the data from the previous system into /tmp, it\nwas discovered that there will not be enough free space in /tmp to make the\nboot image.  Please reboot in normal mode and increase the size of /tmp or\nreduce the number of files to save as listed in the \"/etc/preserve.list\" file.\n",
/* 7 */
"%s: Could not remove the %s logical volume.  Can not continue.\n",
/* 8 */
"%s: You chose to create logical volumes mapped exactly as they\nwere on the previous disks, but there are no map files specified in the\nimage.data file.\n",
/* 9 */
"%s: Could not create the %s logical volume.\n",
/* 10 */
"%s: Could not create the %s file system.\n",
/* 11 */
"%s: Could not create the root volume group.\n",
/* 12 */
"%s: Although the root volume group was created successfully, it could\nnot be accessed.\n",
/* 13 */
"%s: Restore of Base Operating System from CDROM failed.\n",
/* 14 */
"%s: Restore of Base Operating System from %s failed.\n",
/* 15 */
"%s: Could not populate devices directory.\n",
/* 16 */
"%s: Could not copy volume group information to the disk.\n",
/* 17 */
"%s: Could not copy customized device object classes to disk.\n",
/* 18 */
"%s: Could not create boot image.\n",
/* 19 */
"%s: Invalid error code %d\n",
/* 20 */
"%s:  function name required.\n",
/* 21 */
"\n%s:  invalid action code %d\n",
/* 22 */
"   ID#        OPTION\n     1        Continue\n     2        Perform System Maintenance and Then Continue\n   Enter ID number: ",
/* 23 */
"\nusage:  %s -f function  -e error code -a action [ -r return code ]\n",
/* 24 */
"\n                               Running Customization\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
/* 25 */
"\n                        Installing Base Operating System\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n",
/* 26 */
"\n\n\n\n\n        Base Operating System installation is complete.\n\n        Please perform the following steps to activate\n        the changes made during this installation.\n\n",
/* 27 */
"        1. Turn the system key to the NORMAL position.\n        2. Press the ENTER key to restart (reboot) the system.\n\n\n\n",
/* 28 */
"%s:  Could not install the %s package.  This package provides\n  the /unix kernel.\n",
/* 29 */
"%s:  Interrupted by user.\n",
/* 30 */
"\n                        Installing Base Operating System\n\n  Turn the system key to the NORMAL position any time before \nthe installation ends.\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n",
/* 31 */
"Recovering volume group information for %s onto disk %s.\n",
/* 32 */
"Could not import volume group %s.  Continuing post-install processing.\n",
/* 33 */
"Error in merging the saved and new files.  The running of the system should\nnot be impaired.  For further information, check the log located in\n/var/adm/ras/bosinstlog.\n",
/* 34 */
"Error in merging the saved and new files.  A successful BOS installation\ncannot be completed with the new version of the file left in place.  For more\ninformation, see the log in /var/adm/ras/bi.log.\n",
/* 35 */
"\n        1.Remove the installation media.\n        2. Press the ENTER key to restart (reboot) the system.\n\n\n\n\n\n",
/* 36 */
"Migration menu preparation in progress.\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n",
/* 37 */
"Saving system configuration files in /tmp/bos...",
/* 38 */
"\nRemoving obsolete filesets, directories, and files...",
/* 39 */
"Basic operating system support could not be installed.\n\
System administrator should see /var/adm/ras/devinst.log for further\n\
information.  Probable cause of failure is insufficient free disk space.\n\
Type '2' to perform system maintenance to correct the problem, then type\n\
'exit' to continue the installation, or restart the installation with\n\
different installation options.\n"
};

static int maxerr = sizeof(err_tab) / sizeof(char *);

/*
 * main:  display error message depending on input from command line.
 */
main(int argc, char *argv[])
{
  int  errcode = -1, action = -1, c;
  char funcname[80], retcode[100], catfilepath[128];
  extern int optind;
  extern char *optarg;

    lang = getenv("LANG");
    boottype = getenv("BOOTTYPE");
    funcname[0] = '\0';
    retcode[0] = '\0';

    sprintf(catfilepath,"/usr/lib/nls/msg/%s/BosMenus.cat", lang);
    catfd = catopen(catfilepath, NL_CAT_LOCALE);

    /* Parse flags. */
    while ((c = getopt(argc, argv, "f:e:a:r:")) != EOF)
        switch(c)
        {
        /* name of failing routine -- required */
        case 'f':
            strcpy (funcname,optarg);
            break;
        /* index into array containing err codes -- required */
        case 'e':
            if ( ! isdigit(optarg[0]) )
            {
                fprintf(stderr,
        catgets(catfd, 11, INVALID_ERROR_CODE, err_tab[INVALID_ERROR_CODE]),
                        funcname, atoi(optarg)
                );
                usage();
                return (1);
            }
            else
            {
                errcode = atoi(optarg);
            } /* if then else */
            break;
        /* proper action code -- required */
        case 'a':
            action = atoi(optarg);
            break;
        case 'r':
            strcpy(retcode, optarg);
            break;
        default:
            usage();
            return (1);
        } /* switch */
    /* end of while */

    if (funcname[0] == '\0')
    {
        fprintf(
            stderr,
            catgets(catfd, 11, FUNC_NAME_REQUIRED, err_tab[FUNC_NAME_REQUIRED]),
            "berror"
        );
        usage();
        return (1);
    }
    if (errcode < 0 || errcode >= maxerr)
    {
        fprintf(
            stderr,
            catgets(catfd, 11, INVALID_ERROR_CODE, err_tab[INVALID_ERROR_CODE]),
            funcname, errcode
        );
        usage();
        return (1);
    }
    else
    {
        errbuf1 = catgets(catfd, 11, errcode, err_tab[errcode]);
    } /* if else */

    switch (action)
    {
        /* output error and return */
        case 1 : fprintf(stderr, errbuf1, funcname, retcode);
             break;
        /* output error and prompt for next action */
        case 2 : display_options(errbuf1, funcname, retcode);
             break;
        /* invalid action code */
        default:
            fprintf(
                stderr,
        catgets(catfd, 11, INVALID_ACTION_CODE, err_tab[INVALID_ACTION_CODE]),
                funcname, action
            );
    } /* switch */

    catclose(catfd);
    return (0);
} /* main */

/*
 * display_options:  Output appropriate error message and wait for user's
 *                   decision on next action to take.
 */

display_options(char *errbuf1, char funcname[], char retcode[])
{
   int id, in, out, err;
   char input[4];
	FILE *fp;

    /*
     * disable all input from standard in so that if berror is called from
     * inside a loop, this menu whill not take the loop's input as it's own.
     */
    close (0);
    in = open ("/dev/console", O_RDWR);
	close (1);
    out = open ("/dev/console", O_RDWR);
	close (2);
    err = open ("/dev/console", O_RDWR);
	fp = fopen("/dev/console","w");
    do {
        fprintf(fp, errbuf1, funcname, retcode);
	fprintf( fp, catgets(catfd, 11, MENU, err_tab[MENU]) );
	read(in, input, 3);
	id = atoi(input);
    } while (id < 1 || id > 2);

    switch ( id )
    {
        case 1 : break;
        case 2 : execl("/bin/ksh", "ksh", 0);
             break;
    } /* switch */
    close (in);
    close (out);
    close (err);
    fclose (fp);
} /* display_options() */

/*
 * Usage:  display usage of berror.
 */
usage()
{
    fprintf( stderr, catgets(catfd, 11, USAGE, err_tab[USAGE]), "berror" );
}
