static char sccsid[] = "@(#)22	1.15  src/bos/usr/bin/uucp/uupoll.c, cmduucp, bos411, 9433B411a 8/16/94 16:55:52";
/* 
 * COMPONENT_NAME: CMDUUCP uupoll.c
 * 
 * FUNCTIONS: Muupoll, cleanup 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
** uupoll.c
**
*/

#ifdef BSD4_2
#undef BSD4_2
#endif

#include	"uucp.h"

nl_catd catd;
mode_t omask;		/* Not used. Keeps linker happy for utility.c */
								

main(argc, argv, envp)
register int	argc;
register char	**argv;
char **envp;
{
	int	nulljob = 0, mode, i;
	char	grade, lockfile[256], file[256];
	FILE	*fd;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);
	if (argc < 2) {
		fprintf(stderr, MSGSTR(MSG_UUPOLL_1, 
			"Usage: uupoll [-ggrade] [-n] system\n"));
		exit(1);
	}
	Env = envp;
	omask = umask(UMASK);
	(void)atexit(oldmask);

	grade = '\0';
	myName(Myname);
	chdir(SPOOL);

	for (--argc, ++argv; argc > 0; --argc, ++argv) {
		if (strcmp(argv[0], Myname) == SAME) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_2,
				"This *is* %s!\n"), Myname);
			continue;
		}
		if (strncmp(argv[0],"-g",2) == SAME) {
			if (isalnum(argv[0][2]))
				grade = argv[0][2];	/* just one byte */
			else 
				fprintf(stderr, MSGSTR(MSG_CICO23,
					"%s: Ignoring invalid transfer grade of %c\n"),
					"uupoll", argv[0][2]);
			continue;
		}
		if (strncmp(argv[0],"-n") == SAME) {
			nulljob++;
			continue;
		}
	
		if (versys(argv[0])) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_3,
				"%s: unknown system\n"), argv[0]);
			continue;
		}

			/*
			** In order to convince UUCICO to ignore any
			** status (/usr/spool/uucp/.Status/<sys_name>
			** files that might prevent it from calling,
			** we'll just take the bull by the horns and
			** blow away the status file.  If the
			** sysadmin is really needing that info, he
			** can use "uulog"
			*/

		sprintf(lockfile, "%s/%s", STATDIR, argv[0]);
		unlink(lockfile);

			/*
			** Now we have to trick UUCICO into thinking there
			** is some work to do.  So we generate a phony job
			** (after we check to ensure we're not going to
			** accidently erase a valid one!).  We need to create
			** a system_name directory under /usr/spool/uucp.
			** If it already exists, no problem . . . just go
			** ahead and continue.  If it isn't created for
			** some reason, report the problem and close up 
			** shop.
			*/

			sprintf(file, "%s/%s", SPOOL, argv[0]);
			mode =  S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
			 	S_IXGRP | S_IROTH | S_IXOTH;
			if (mkdir(file, mode)) {
				if (errno != EEXIST) {
					fprintf(stderr, MSGSTR(MSG_UUPOLL_4,
					  "could not create directory '%s'\n"),
						file);
					cleanup(1);
				}
			}
					/*
					** we have to find an unused filename
					** inside of /usr/spool/uucp/argv[0]
					*/

			for (i = 0; i < 9999; i++) {
				if (strlen(argv[0]) > 7) {
					sprintf(file, "%s/%s/C.%.7sa%.4d", 
						SPOOL, argv[0], argv[0], i);
				} else {
					sprintf(file, "%s/%s/C.%sa%.4d", 
						SPOOL, argv[0], argv[0], i);
				}

				if (( fd = fopen(file, "r")) != NULL) {
					fclose(fd);
					continue;
				}
				break;
			}

					/*
					** create the phony job
					*/

			if ((fd = fopen(file, "w")) == NULL) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_5,
				"Can't open '%s' for writing!\n"), file);
				cleanup(1);
			}
			fclose(fd);		/*
						** file now exists, length = 0
						*/

			if (nulljob)
				exit(0);

			if (grade != '\0') {
				execle(UUCICO, "uucico", "-r", "1", "-g",
					&grade, "-s", argv[0], 0, Env);
			}
			else {
				execle(UUCICO, "uucico", "-r", "1", 
					"-s", argv[0], 0, Env);
			}
			/*
			** what are we still doing here?
			** the execle() failed.
			*/
			  fprintf(stderr, MSGSTR(MSG_UUPOLL_7,
				"could not exec uucico! Exiting...\n"));
				cleanup(1);
	} 				/* end of for() loop */
}


/*
**	cleanup(value)
**	int	value;
**
**	Exit the program with the error level passed in "value".
**
**	Return code:  We don't return.
*/

cleanup(value)
int	value;
{

	catclose(catd);

	exit(value);
}
