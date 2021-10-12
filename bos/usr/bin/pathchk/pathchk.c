static char sccsid[] = "@(#)76	1.7  src/bos/usr/bin/pathchk/pathchk.c, cmdposix, bos41B, 9504A 12/19/94 12:25:42";
/*
 * COMPONENT_NAME: (CMDPOSIX) commands required by Posix 1003.2
 *
 * FUNCTIONS: pathchk
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdlib.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/access.h>
#include <nl_types.h>
#include "pathchk_msg.h"

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PATHCHK, Num, Str) 

extern char *optarg;
extern int optind;
static int exit_status = 0;

main(argc, argv)
int argc;
char **argv;
{
	int j = 1;
	char *pathname;
	register c;
	int pflag = 0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_PATHCHK, NL_CAT_LOCALE);

	while ( (c = getopt(argc, argv, "p") ) != EOF) {
		switch(c) {
		case 'p':
			pflag++;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR(USAGE, "usage pathchk [-p] pathname\n"));
			exit_status = 1;
			}
		}

	if (pflag) {
		while (optind < argc) {
			check_posix(argv[optind]);
			optind++;
			}
		}
	else {
		while ( j < argc )
			check_path(argv[j++]);
		}

	exit(exit_status);
}

/*
 * Function: check_posix(pathname)
 *
 * Description: checks pathname length and component length
 *      against limits as defined by Posix.  It also checks for
 *      illegal characters in the portable filename character set,
 *      defined by Posix.
 */

static check_posix(pathname)
char *pathname;
{
	char *ptr, *ptr2, *s3;
	static char buf[PATH_MAX];
	int mbcnt;
	wchar_t wc_ptr;
	char mbchar[10];

	if (strlen(pathname) > _POSIX_PATH_MAX) {
		fprintf(stderr, MSGSTR(POSIX_PATH, "pathname %s exceeds _POSIX_PATH_MAX %d\n"), pathname, _POSIX_PATH_MAX);
		exit_status++;
		}
	

	strcpy(buf, pathname);
	ptr = buf;

	while (*ptr != '\0') {
		if (*ptr == '/') ptr++;
		if ( (ptr2 = strchr(ptr, '/')) != NULL) {
			*ptr2 = '\0';
			if (strlen(ptr) > _POSIX_NAME_MAX) {
				fprintf(stderr, MSGSTR(POSIX_NAME, "name %s exceeds _POSIX_NAME_MAX %d\n"), ptr, _POSIX_NAME_MAX);
				exit_status++;
				}
			}
		else {
			if (strlen(ptr) > _POSIX_NAME_MAX) {
				fprintf(stderr, MSGSTR(POSIX_NAME, "name %s exceeds _POSIX_NAME_MAX %d\n"), ptr, _POSIX_NAME_MAX);
				exit_status++;
				}
			break;
			}	
		ptr = ptr2;
		ptr++;
		}

	s3 = pathname;
	for (; *s3 != '\0'; s3++) { 
		if ( (( (*s3 < 0x61 ) && (*s3 != 0x5f)) || ( *s3 > 0x7a)) &&
		 (( *s3 < 0x41) || (*s3 > 0x5a)) &&
		 (( *s3 < 0x30) || (*s3 > 0x39)) &&
		 (( *s3 < 0x2d) || (*s3 > 0x2f)) ) {
           		if ( (mbcnt = mblen(s3, MB_CUR_MAX)) > 0)  {
				mbtowc(&wc_ptr, s3, MB_CUR_MAX);
                     		if (iswprint(wc_ptr)) {
					strncpy(mbchar, s3, mbcnt);
					mbchar[mbcnt]='\0';
                          		fprintf(stderr, "%s ", mbchar);
					s3 += mbcnt - 1;
                        	}
                 		else
                     			fprintf(stderr, "0x%2.2x ", *s3);
			}
                 	else
                     		fprintf(stderr, "0x%2.2x ", *s3);

			fprintf(stderr, MSGSTR(CHAR_SET, "is not in portable filename character set for %s\n"), pathname);
                        exit_status++;
			}
		}
	return;
}

/*
 * check_path is the default path of the pathchk function, this
 * checks for pathnames not to exceed the filesystems maximum
 * and for components not to exceed filesystems maximum.  It also
 * checks for search permission on that directory/file.
 */

static check_path(pathname)
char *pathname;
{
	char *ptr, *ptr2, *s3;
	char file_access[PATH_MAX];
	long namemax;
	static char buf[PATH_MAX];
	static char tbuf[PATH_MAX];
	int createable, absolute;

	if (strlen(pathname) > PATH_MAX) {
		fprintf( stderr, MSGSTR(PATHN_MAX, "pathname %s exceeds PATH_MAX %d\n"), pathname, PATH_MAX);
		exit_status++;
		}

	/*
	 * NAME_MAX is not defined in limits.h
	 * must call pathconf to gets value, since
	 * pathconf returns -1 if file doesn't exist
	 * we need to keep going until we get a valid number
	 * for namemax
	 */
	
	strcpy(buf, pathname);
	absolute = (*pathname == '/');

	/* If the pathname is a relative path name then we must */
	/* append the current directory so that pathconf will   */
	/* succeed.                                             */

	if (!absolute) {
		if (!getcwd(tbuf,PATH_MAX))
			perror("getcwd");
		strcat(tbuf,"/");
		strcat(tbuf,buf);
		strcpy(buf,tbuf);
	}

	while ( (namemax = pathconf(buf, _PC_NAME_MAX) )== -1 ) {
		if ( (s3 = strrchr(buf, '/')) == NULL )
			break;
		if (s3 == buf)
			buf[1] = '\0';
		else
			*s3 = '\0'; 
		}

	*file_access = '\0';

	createable = (access(absolute ? "/" : ".", W_ACC|X_ACC) == 0);
	strcpy(buf, pathname);
	ptr = buf;
	while (*ptr != '\0') {
		if (*ptr == '/') ptr++;
		if ( ( ptr2 = strchr(ptr, '/')) != NULL)
			*ptr2 = '\0';
		if (strlen(ptr) > namemax) {
			fprintf(stderr, MSGSTR(NAME_MAX,
				"name %s exceeds NAME_MAX %d\n"), ptr, namemax);
			exit_status++;
		}
		/* check for search permission,
		 * note the file does not have to
		 * exist, we can ignore ENOENT
		 */
		if (*file_access != '\0' || absolute)
			strcat(file_access, "/");
		strcat(file_access, ptr);
		if (access(file_access, E_ACC) == -1) {
			if (errno == ENOENT && createable)
				/* ENOENT errno is ok */
				;
			else {
				fprintf(stderr, "pathchk: %s ", file_access);
				perror("access");
				exit_status++;
			}
		} else {
			createable = (access(file_access, W_ACC|X_ACC) == 0);
		}
		if (ptr2 == NULL)
			break;
		ptr = ptr2;
		ptr++;
	}
	return;
}
