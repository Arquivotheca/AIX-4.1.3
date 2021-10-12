static char sccsid[] = "@(#)54  1.4.1.9  src/bos/usr/bin/tcbck/tcbck.c, cmdsadm, bos411, 9437B411a 9/13/94 18:45:41";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: main
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/audit.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <odmi.h>
#include <stdlib.h>
#include <usersec.h>
#include "tcbdbio.h"
#include "tcbmsg.h"
#include <sys/fullstat.h>

/*
 * TCBCK - System configuration checker
 *
 *	Define and check proper system installation and configuration.
 *
 * Syntax:
 */

char	*Usage[] = {
	"",
	"Usage: tcbck -a <filename> [ [ <attr> | <attr>=[<value> | ] ] ... ]",
	"             -a -f <filename>",
	"             -a sysck [ treeck_novfs=<dir> ] [ checksum=<program> ]",
	"             -d <filename> | <class> [ <filename> | <class> ] ...",
	"             -d -f <filename>",
	"             -l /dev/<filename> [ /dev/<filename> ] ...  ",
	"             -(p|y|n|t) [ [<filename>|<class>] ... | ALL | tree ]",
	NULL
};

/*
 * Attribute meanings:
 */

/* NOTE:  This line may be appropriate in the future, but is not
   currently a documented attribute name. */
/*	"         pcl                     - PCL of file (computed)",	*/

char	*Attributes[] = {
	"",
	"3001-199 Illegal Attribute, must be one of",
	"         class=<class_name,...>  - classes file is a member of",
	"         type                    - FILE, DIRECTORY, etc. (computed)",
	"         owner                   - owner ID of file (computed)",
	"         group                   - group ID of file (computed)",
	"         acl                     - ACL of file (computed)",
	"         mode                    - Mode bits of file only (computed)",
	"         links=<filename,...>    - list of hard links to file",
	"         checksum                - checksum for file (computed)",
	"         program=\"<program_name>[,<arg,...>]\"",
	"                                 - program to verify file",
	"         source=[<filename>]     - location of original copy of file",
	"         size                    - file size (computed)",
	"         target=[<filename>]     - location of file to link to",
	" ",
	"         Valid in sysck stanza only",
	" ",
	"         treeck_novfs=<mount_point,...>",
	"                                 - list of mount points to ignore",
	"         treeck_nodir=<directory,...>",
	"                                 - list of directories to ignore",
	"         checksum=\"program_name flags\"",
	"                                 - command for computing checksums",
	"         setuids=<user,...>      - list of administrative users",
	"         setgids=<group,...>     - list of administrative groups",
	NULL
};

/*
 * Program names
 *
 * The Checksum program may be overriden by a value in the TCB file
 */

char	*Aclget = "/usr/bin/aclget";
char	*Aclput = "/usr/bin/aclput";
char	*Pclget = "/usr/bin/privget";
char	*Pclput = "/usr/bin/privput";
char	*Checksum = "/usr/bin/sum -r < ";
int	BuiltInSum = 1;

/*
 * Argument flags and optional arguments
 */

int	aflg;		/* add a file or stanza file to sysck.cfg           */
int	dflg;		/* remove a file or class from sysck.cfg            */
int	pflg;		/* test file silently and make fixes                */
int	yflg;		/* test verbosely and make fixes                    */
int	nflg;		/* test verbosely and don't make fixes              */
int	tflg;		/* test verbosely and make fixes interactively      */
int 	lflg;		/* add /dev entries to sysck.cfg file as
			   per the command line			            */
int     sflg;           /* save stanzas from sysck.cfg that are specified   */
int     fflg;           /* f flag specified (for -s flag checking)          */

int	all;		/* all keyword appeared                             */
int	tree;		/* tree keyword appeared                            */
int	verbose = 1;	/* Either of -n or -y were specified.  On initially */
int	fixit = 0;	/* Either of -p or -y were specified                */
int	query = 0;	/* -t was specified                                 */

/*
 * Forward declarations and external functions
 */

/*
 * For getopt()
 */
extern	int	optind;
extern	char	*optarg;

extern	int	make_tcbent();
extern  int     save_tcbents(char *, char *);
extern	void	init_sysck();
extern	void	init_vfs();
extern	void	usage();
extern	void	fatal();

/*
 * Global data
 */

struct	tcbent	**tcb_table;	/* array of structures describing the TCB    */
char	**novfs;		/* list of file systems to ignore            */
char	**nodir;		/* list of directories to ignore             */
char	**vfs;			/* list of file systems to test              */
gid_t	*setgids;		/* group IDs to test for                     */
uid_t	*setuids;		/* user IDs to test for                      */
int	tcb_cnt;		/* number of valid entries in tcb_table      */
int	tcb_max;		/* number of slots in tcb_table              */
char    temp_init[TEMP_SIZE];	/* temporary file name			     */
FILE    *fdtemp;		/* temporary file descriptor		     */


/*
 * NAME:	main
 *
 * FUNCTION:	Process arguments for tcbck command
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Invoked by another user process.  This is a command.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

main (argc, argv)
int	argc;
char	**argv;
{
	int	arg;
	int	rc;
	int	i, j;
	int	new_entries = 0;
	int	errors = 0;
	struct	tcbent	*tcbent;
	char	*file;
	CLASS_SYMBOL classp;
	void	*o;
	char	*m;
        char    stz_list_file[MAXPATHLEN], /* List of stanzas to be saved  */
                stz_save_file[MAXPATHLEN]; /* Where to save the specified  */
                                           /* sysck.cfg stanzas            */


#define TCB_ATTR_NAME "TCB_STATE"
	/*
	 * Turn off auditing for this process.  It will cut its
	 * own audit records.
	 */

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	setlocale (LC_ALL, "");

	/*
	 * Initialize the NLS message catalogs 
	 */

	init_msg ();

	/* Before doing anything else, make sure the TCB was enabled during
	   installation. Set the ODM path to /usr/lib/objrepos so that
           when installing the share part of a lpp; you are looking in the
	   correct place for the class information. */

	odm_initialize();
	odm_set_path("/usr/lib/objrepos");
	classp = odm_mount_class("PdAt");
	if (classp == (CLASS_SYMBOL)-1) {
		odm_err_msg(odmerrno, &m);
		fmsg(m);
		fatal (Open_An_Apar, NULL, ENOTRUST);
	}
	m = xmalloc(classp->structsize);

	o = odm_get_obj(classp, "attribute=" TCB_ATTR_NAME, m, ODM_FIRST);
	if (o == NULL || o == (void *)-1)
		fatal(Corrupted_Machine, TCB_ATTR_NAME, ENOTRUST);
	for (i = 0; i < classp->nelem; i++) {
		if (strcmp("deflt", classp->elem[i].elemname) == 0) {
			if (strcmp("tcb_enabled",
				   &m[classp->elem[i].offset]) == 0)
				break;
			if (strcmp("tcb_disabled",
				   &m[classp->elem[i].offset]) == 0)
				fatal(Not_Trusted_Machine, NULL,
				      ENOTRUST);
			fatal(Corrupted_Machine, TCB_ATTR_NAME, ENOTRUST);
		}
	}
	if (i >= classp->nelem)
		fatal(Corrupted_Machine, TCB_ATTR_NAME, ENOTRUST);
		
	odm_terminate();
	free(m);

	/*
	 * Initialize the sysck stanza values
	 */
	init_sysck ();

	/*
	 * Process each argument in turn.
	 *
	 *	-a - test if next argument is -f, do -f processing if so
	 *	     otherwise create a stanza from the remaining arguments
	 *	     and add it.
	 *	-f - read each stanza from next argument and add.  must
	 *	     occur with -a or -d option.  Also used to specify
         *           the file containing stanzas to save in conjunction
         *           with the -s flag.
	 *	-d - a list of file names or file classes follows.  each
	 *	     of these is to be removed from the TCB file.
	 *	-l - a list of /dev files to be added to the sysck.cfg file.
         *           These files have been deemed secure by the user.
	 *	-p - silently compare the TCB file against the actuall
	 *	     files and correct any discrepancies
	 *	-y - compare the TCB file against the actual files and
	 *	     correct any discrepancies, reporting changes that
	 *	     are made.
	 *	-n - compare the TCB file against the actual files and
	 *	     report any discrepancies.
	 *	-t - compare the TCB file against the actual files and
	 *	     correct any discrepancies interactively.
         *      -s - save any stanzas from sysck.cfg that are listed
         *           in the -f file to the file named in the arg to -s.
	 */

	while ((arg = getopt (argc, argv, "afdpnytls")) != EOF) {
		switch (arg) {

			/*
			 * Add - add a single file, or if followed by
			 * the -f flag, and entire file of stanzas.
			 */

			case 'a':

				/*
				 * An update of the database is going
				 * to happen, so let me tell the library
				 * about it.
				 */

				if (setuserdb (S_READ|S_WRITE))	
					fatal (Open_An_Apar, 0, errno);
				aflg++;
				if (optind >= argc)
					usage (Usage);
				if (argv[optind][0] == '-')
					break;
				rc = make_tcbent (&argv[optind]);
				if (rc == -1)
					exit (EINVAL);
				exit (rc);

			case 's':
				sflg++;
				if ((optind + 1 != argc) || !argv[optind] || argv[optind][0]=='-')
					usage(Usage);
				strcpy(stz_save_file, argv[optind++]);
				break;

			/*
			 * Use a file - the next argument must be a file
			 * name of a file containing stanzas to be added
			 * to or installed in the TCB file.
			 */

			case 'f':
				fflg++;
				/*
				 * Expect one more argument - the name of
				 * a stanza file to read input stanzas from.
				 */

				if (aflg) {
					if (optind + 1 != argc)
						usage (Usage);
					if (rc = add_tcbents (argv[optind]))
						exit (rc);

				/*
				 * Expect one more argument - the name of
				 * a stanza file to read filenames to delete
				 * from the SYSCK database.
				 */

				} else if (dflg) {
					if (optind + 1 != argc)	
						usage (Usage);
					if (rc = del_tcbents (argv[optind], 0))
						exit (rc);
					else
						exit (0);

                                } else {
                                        /* Assume that since -a and -d haven't
                                         * appeared yet-must be a SAVE,i.e.
                                         * tcbck -f inv-file -s stanza-save-file
                                         * We will check this after all of the
                                         * args have been collected (since -s
                                         * will normally appear after the -f).
                                         * We have to save the arg to the -f
                                         * flag so that when we read the -s,
                                         * we know where to read the update
                                         * list from.
                                         */

                                        /*
                                         * NOTE: this should only be used by
                                         * the install/update process
                                         */

					if ( !argv[optind] || argv[optind][0]=='-' )
							usage(Usage);
                                       	strcpy(stz_list_file, argv[optind++]);
                                }
                                if (aflg || dflg) {

                                        /*
                                         * Some collection of stanzas may have been
                                         * modified - update the whole lot at once.
                                         */

                                        for (i = 0;i < tcb_cnt;i++) {
                                                tcbent = tcb_table[i];
                                                if (tcbent->tcb_changed)
                                                        if (rc = puttcbent (tcbent))
                                                                errors++;
                                        }

                                        /*
                                         * Actually commit all of the changes that
                                         * the user has requested.
                                         */

                                        puttcbattr ((char *) 0, (char *) 0,
                                                (void *) 0, SEC_COMMIT);
                                        enduserdb ();
                                        exit (errors ? rc:0);
                                }
                                break;


			/*
			 * Delete - the remaining arguments are the names
			 * of files or classes to be removed from the TCB
			 * file.
			 */

			case 'd':
				/*
				 * An update of the database is going
				 * to happen, so let me tell the library
				 * about it.
				 */

				if (setuserdb (S_READ|S_WRITE))	
					fatal (Open_An_Apar, 0, errno);
				dflg++;
				if (optind >= argc)
					usage (Usage);
				if (argv[optind][0] == '-')
					break;
				rc = del_tcbent (&argv[optind]);
				enduserdb ();
				exit (rc);

                        /*
                         * Add /dev entries - the remaining arguments are the 
                         * names of /dev entries to be added to the sysck.cfg
                         * file.
                         */

                        case 'l':

                                /*
                                 * An update of the database is going
                                 * to happen, so let me tell the library
                                 * about it.
                                 */

                                if (setuserdb (S_READ|S_WRITE))
                                        fatal (Open_An_Apar, 0, errno);
                                lflg++;

                                if ((optind >= argc)||(argv[optind][0]=='-'))
                                        usage (Usage);

				/*
	 			* l flag cannot be used with any other flags
	 			*/ 
				if (lflg)
					if (aflg + dflg + sflg + fflg + yflg + nflg +pflg + tflg > 0)
						usage (Usage);

				/* Build a /tmp file with new /dev entries */
                                if (rc = build_temp_file (&argv[optind]))
					exit (rc);

	 	   		/* Add the /tmp entries to sysck.cfg */
				if (rc = add_tcbents((char *)temp_init))
					exit (rc);

                                /*
                                 * Some collection of stanzas may have been
                                 * modified - update the whole lot at once.
                                 */

                                for (i = 0;i < tcb_cnt;i++) {
                                        tcbent = tcb_table[i];
                                        if (tcbent->tcb_changed)
                                                if (rc = puttcbent (tcbent))
                                                        errors++;
                                }

                                /*
                                 * Actually commit all of the changes that
                                 * the user has requested.
                                 */

                                puttcbattr ((char *) 0, (char *) 0,
                                        (void *) 0, SEC_COMMIT);

                                enduserdb ();
				unlink (temp_init);
                                exit (errors ? rc:0);
				
			case 'p':	/* Fix problems silently */
				pflg++;
				break;
			case 'y':	/* Fix problems verbosely */
				yflg++;
				break;
			case 'n':	/* Report problems, no fixing */
				nflg++;
				break;
			case 't':	/* Fix problems interactively */
				tflg++;
				break;
			default:	/* Unknown flag */
				usage (Usage);
		}
	}


	/*
	 * y, n, p, t flags are mutually exclusive *
	 */ 
	if (yflg + nflg + pflg + tflg > 1)
		usage (Usage);

	/*
	 * a, d flags are mutually exclusive *
	 */ 
	if (aflg + dflg > 1)
		usage (Usage);

	/*
	 * if s flag then the f flag must be used
	 */ 
	if (sflg)
		if (sflg + fflg != 2)
			usage (Usage);

	/*
	 * s flag cannot be used with any other flags except fflg
	 */ 
	if (sflg)
		if (aflg + dflg + lflg + yflg + nflg + pflg + tflg > 0)
			usage (Usage);
		else
                {
                	rc = save_tcbents(stz_list_file,stz_save_file);
                        exit (rc ? rc : 0);
                }
		
	/*
	 * At this point we know aflg + dflg <= 1 and yflg + nflg + pflg + tflg <= 1)
	 * So we are checking that the y,n,p,t cannot be used with a,d flags
	 */ 

	if (aflg + dflg + yflg + nflg + pflg + tflg > 1)
		usage (Usage);

	/*
	 * If -y, -n, -p, or -t are specified, a single argument,
	 * "ALL" or "tree" must be present, or a list of files or
	 * file classes to be tested may be given.
	 */

	if ((yflg + nflg + pflg + tflg == 0) || argc == 1 || optind == argc)
		usage (Usage);

	for (i = optind;i < argc;i++) {
		if (strcmp (argv[i], "ALL") == 0)
			all++;
		else if (strcmp (argv[i], "tree") == 0)
			tree++;
	}

	/*
	 * Only one of "ALL" or "tree" may be specified
	 */

	if (all + tree > 1)
		fatal (All_or_Tree, 0, EINVAL);

	if ((all || tree) && optind + 1 < argc)
		fatal (No_More_Args, 0, EINVAL);

	/*
	 * -p, -y, -n, and -t are mutually exclusive.  Both -y
	 * and -n cause tcbck to output error messages.  Both
	 * -y and -p cause tcbck to fix problems it uncovers.
	 * The -t option reports problems, but queries for an
	 * interactive action.
	 */

	if (pflg + nflg + yflg + tflg != 1)
		fatal (P_N_or_Y, 0, EINVAL);


	/*
	 * It is finally safe to turn off the verbose flag if it was
	 * set.  Set up the last two flags and start checking the system
	 * out.
	 */

	verbose = yflg || nflg || tflg;
	fixit = yflg || pflg;
	query = tflg;

	/*
	 * Check the files in the TCB first, followed by the files in
	 * the remainder of the tree, if requested.  Each tree in the
	 * mount table is checked one after the other.
	 */

	if (query)
		if (setuserdb (S_READ|S_WRITE))	
			fatal (Open_An_Apar, 0, errno);
	if (tree || all)
		optind++;

	if (rc = check_tcb (&argv[optind]))
		errors++;

	if (tree) {
		/*
		 * A list of mounted filesystems is built if the entire tree is
		 * going to be tested.
		 */
		init_vfs ();

		/*
		 * Check every file in all of the mounted file systems.
		 *
		 * TBD: There should be a requirement that EVERY mount
		 * point be in the TCB.  This is to prevent someone
		 * from hiding something under a mount point.
		 */

		for (i = 0;vfs[i];i++) {
			if (treeck (vfs[i])) {
				rc = ENOTRUST;
				errors++;
			}
		}
	}

	/*
	 * Scan the table of entries to see if any of them were changed.
	 * Output the new attributes for the modified entries.
	 */

	for (i = 0;i < tcb_cnt;i++) {
		if (tcb_table[i]->tcb_changed) {
			add_audit_info (tcb_table[i], AUDIT_OK);
			puttcbent (tcb_table[i]);
			new_entries++;
		}
	}
	if (new_entries)
		puttcbattr ((char *) 0, (char *) 0, (void *) 0, SEC_COMMIT);

	if (query)
		enduserdb ();

	exit (errors ? rc:0);

}

