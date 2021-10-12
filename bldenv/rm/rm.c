static char sccsid[] = "@(#)95 1.24.1.9  src/bos/usr/bin/rm/rm.c, cmdfiles, bos41J, 9508A 2/16/95 13:25:44";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: rm, rmdir
 *
 * ORIGINS: 3, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * OSF/1 Release 1.0
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * static char rcsid[] = "RCSfile: rm.c,v  Revision: 2.10  (OSF) Date: 90/10/07 16:53:20 
 */

#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/limits.h>	/* for PATH_MAX */
#include	<sys/stat.h>
#include	<dirent.h>
#include	<sys/access.h>
#include	<sys/mode.h>
#include	<sys/errno.h>

#include <ctype.h>
#include <sys/types.h>
#include <locale.h>

#include "rm_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_RM,Num,Str)

static int errcode;

static char *cmdname;

static dev_t cwd_st_dev;
static ino_t cwd_st_ino;
static int is_cwd_called = 0;

static int fflg, iflg, rflg, eflg = 0;		/* options for rm command */

/*
 * NAME: writable
 *
 * FUNCTION: Test if file is writable.  Symbolic links are always writable.
 *
 * RETURNS:  1 - yes
 * 	     0 - no
 */

/* use a macro for performance */
#define writable(name,buf) ((((buf)->st_mode&S_IFMT) == S_IFLNK)?1:(access(name,W_OK) == 0))

#if 0
static writable(name, buf)
char *name;
struct stat *buf;
{
    if ((buf->st_mode&S_IFMT) == S_IFLNK)
	return 1;
    return (access(name,W_OK) == 0);
}
#endif


/*
 * NAME: dotname
 * 
 * FUNCTION: Test whether s is "." or ".."
 * 
 * RETURNS:  1 - yes, s is "." or ".."    
 * 	     0 - no
 */

/* use a macro for performance */
#define dotname(s) (s[0] == '.' && (!s[1] || s[1] == '.' && !s[2]))

#if 0
static dotname(s)
char *s;
{
	return(s[0] == '.' && (!s[1] || s[1] == '.' && !s[2]));
}
#endif


/*
 * NAME: {rm | delete} [-fiRre] File ... 
 *       rmdir [-p] Directory ... 
 *
 * FUNCTION: Removes file and directory entries.
 *
 * NOTES:   -f  do not prompt for confirmation
 *	    -i  prompt for confirmation
 *          -R  remove file hierarchies
 *          -r  same as -R
 *          -e  displays a message after deleting each file 
 *
 *          -p  remove all directories in a pathname
 */

main(argc,argv)
int argc;
char **argv;
{
    int rv = 2;

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_RM, NL_CAT_LOCALE);

    /* find our cmdname */
    cmdname = (char *) strrchr(argv[0],'/');
    if (cmdname) cmdname++; else cmdname = argv[0];

    if (!strcmp(cmdname,"rmdir")) rv = rmdir_main(argc,argv);
    else rv = rm_main(argc, argv);	/* assume rm */
    exit(rv);
}

/*
 * NAME: rm_main
 *
 * FUNCTION: main rm routine -- checks options and calls rm
 */

static rm_main(argc, argv)
int argc;
char *argv[];
{
    register char *arg;
    int c;
    int cwdfis = 0; 	/* Current Working Directory found in Subdirectory
			 * Used by recursive rm function to detect a call of
			 * the form "rm -r <ancestor of current working directory>"
			 */

	while ((c=getopt(argc, argv, "fiRre")) != EOF) 
	  switch(c) {
		case 'f':
			fflg++;
			iflg = 0;
			break;
		case 'i':
			iflg++;
			fflg = 0;
			break;
		case 'R':
		case 'r':
			rflg++;
			break;
		case 'e':
			eflg++;
			break;
		default:
		    fprintf(stderr, 
		       MSGSTR(USAGE,"Usage: %s [-firRe] [--] File ...\n"), cmdname);
		    return(2);
		}

    argc -= optind;
    argv=&argv[optind];

    if (argc < 1 && !fflg) {
	fprintf(stderr, MSGSTR(USAGE,"Usage: %s [-firRe] [--] File ...\n"), cmdname);
	return(2);
    }

    while (argc-- > 0) {
        arg = (char *) strrchr(*argv,'/');
        if (arg) arg++; else arg = *argv;
        if (dotname(arg)) {
		fprintf(stderr,MSGSTR(CANTREMOVE,
			"%s: cannot remove '.' or '..'\n"),cmdname);
		++errcode;
	}
        else rm(*argv, *argv, &cwdfis);
	argv++;
    } 

    return(errcode?2:0);
}

/*
 * NAME: rm
 *
 * FUNCTION: (recursive) removes files and directories 
 */

static rm(arg, short_arg, cwdic)
char arg[];	/* the actual path, as seen by user.  May be greater than PATH_MAX */
char short_arg[];/* if arg is greater than PATH_MAX, then this will be a shorter   */
		/* relative path name.  Otherwise, it will be the same as arg      */
int *cwdic;	/* current working directory is a child */
{
    struct	stat	buf;
    struct	dirent	*direct;
    char	*name;
    int		malloc_len = PATH_MAX;/* current ammount of malloced space in name */
    DIR		*d;
    int		cwdfis;		/* current working directory found in subdirectory */

    *cwdic = 0;			/* until proven 1 by finding it directly or by having a
				 * recursive call to rm find it in a subdirectory .
				 */
    /* The following "Steps" are specified by XPG4 */
    /* Step 1 -- If file does not exist */
    if(lstat(short_arg, &buf)) {
	/* Step 1.a */
	/* if '-f', don't print diagnostic msg for non-existent file */
	if (fflg==0) {	
	    fprintf(stderr, "%s: ", cmdname);
	    perror(arg);
	    ++errcode;
	}
	/* Step 1.b */
	return;
    }
    /* Step 2 -- If file is a directory... */
    if ((buf.st_mode&S_IFMT) == S_IFDIR) {
	/* Step 2.a */
	if (!rflg) {			/* directory and rflg not set */
		fprintf(stderr, MSGSTR(ISDIRECT,"%s: cannot remove directory %s\n"), cmdname, arg);
		++errcode;
		return;
	}
	/* Step 2.b */
	if (!fflg && ((!writable(short_arg, &buf) && isatty(fileno(stdin)))
	    || iflg)) {
	    fprintf(stderr,MSGSTR(DIRECTORY,"%s: remove files in directory %s?  Enter y for yes. "),cmdname,arg);
	    if(!yes())
		    return;
	}
	/* Step 2.c */
	if((d=opendir(short_arg)) == NULL) {
	    fprintf(stderr,MSGSTR(CANTREAD,"%s: cannot read %s\n"), cmdname, arg);
	    ++errcode;
	    return;
	}

	if ((name=(char *)malloc(malloc_len))==NULL) {	/* get initial memory */
		perror(cmdname);
		exit(1);				/* fatal error */
		}
	while ((direct = readdir(d)) != NULL) {
	    static int tmp_length;
	    if(!dotname(direct->d_name)) {
		tmp_length=strlen(arg)+strlen(direct->d_name)+2;
		while (tmp_length > malloc_len) {
			malloc_len *= 2;
			if ((name=(char *)realloc(name,malloc_len))==NULL) {
				perror(cmdname);
				exit(1);		/* fatal error */
				}
			}

		sprintf(name, "%s/%s", arg, direct->d_name);
		cwdfis = 0;	
		if (arg==short_arg) {  /* arg is <= PATH_MAX */
		    if (tmp_length <= PATH_MAX)  /* normal path */
			rm(name, name, &cwdfis);
		    else { /* boundry case */
			   /* need to jump all the way down and back */
			char original[PATH_MAX];
			if (getcwd(original,PATH_MAX)==NULL) {
				perror(cmdname);
				exit(1);	/* fatal error */
				}
			if (chdir(arg)!=0) {
				perror(cmdname);
				++errcode;
				}
			rm(name, direct->d_name, &cwdfis);
			if (chdir(original)!=0) {
				perror(cmdname);
				++errcode;
				}
			}
		    }
                 else { /* deep case */
		        /* need to go down and up a single directory */
			if (chdir(short_arg)!=0) {
				perror(cmdname);
				++errcode;
				}
			rm(name, direct->d_name, &cwdfis);
			if (chdir("..")!=0) {
				perror(cmdname);
				++errcode;
				}
			}

		if (cwdfis) (*cwdic)++;
	    }
	}
	free(name);
	closedir(d);
	if (*cwdic)	
	    return;
	/* Step 2.d */
	if (iflg) {
	    fprintf(stderr,MSGSTR(REMPROMPT,"%s: remove %s? "),cmdname,arg);
	    if(!yes())
		    return;
	}

	/* Step 3 */
	/* Nothing to do, this is a directory. */

	/* Step 4 (directory) */
	/* if rmdir fails, print diagnostic message.  */
	if (rmdir(short_arg)) {
	    if (rm_msg(cmdname,arg,short_arg,1) == 1) (*cwdic)++;  /* current working directory is a child */ 
	    errcode++;
	    return;
	}
	else if (eflg)
		 fprintf(stderr, MSGSTR(REMOVEDDIR, "%s: removing directory %s\n"), cmdname, arg);

	return;
    }   /* end of directory logic */

    /* Step 3 */
    if (!fflg && ((!writable(short_arg, &buf) && isatty(fileno(stdin)))
	|| iflg)) {
	fprintf(stderr,MSGSTR(REMPROMPT,"%s: remove %s? "),cmdname,arg);
	if(!yes())
	    return;
    }

    /* Step 4 (not directory) */
    /* if unlink fails, print diagnostic message. */
    if(unlink(short_arg)) {
	if (rm_msg(cmdname,arg,short_arg,0) == 1) (*cwdic)++;
	++errcode;
	return;
    }
    else if (eflg)
	    fprintf(stderr, MSGSTR(FILEREMOVED, "%s: removing %s\n"), cmdname, arg);
    return;
}


/*
 * NAME: rmdir_main
 *
 * FUNCTION: main routine for rmdir command
 */

static rmdir_main(argc,argv)
char **argv;
int argc;
{
    char rc = 0;
    int Errors = 0;
    int c;
    int pflg = 0;

    while ((c = getopt(argc, argv, "p")) != EOF)
    switch(c) {
	case 'p':
		pflg++;
		break;
	default:
		fprintf(stderr, MSGSTR(USAGE_RMDIR,"Usage: rmdir [-p] Directory ...\n"));
		return(2);
		/* break; 	line warning: statement not reached */
	}

    argc -= optind;
    argv = &argv[optind];
		
    if(argc < 1) {
	fprintf(stderr, MSGSTR(USAGE_RMDIR,"Usage: rmdir [-p] Directory ...\n"));
	return(2);
    }

    while(argc--) {
	char *cp = *argv++;
	char *p;
	int i;
	i = strlen(cp) - 1;
	if (cp[i] == '/')
		cp[i] = '\0';    /* remove '/' at end of pathname if any */
	do {
		if ((rc = rmdir(cp)) != 0) {
		    rm_msg("rmdir", cp, cp, 1);
		    ++Errors;
		    break;
		}
		if (p=(char *)strrchr(cp,'/'))  
			*p = '\0';		/* remove last dirname*/
		else
			break;
	    } while(pflg && *cp);
    }
    return(Errors? 2: 0);
}

/*
 * NAME: yes
 *
 * FUNCTION: gets interactive response from user
 */

static yes()
{
	#define RESP_LEN 100
	char response[RESP_LEN];

	fgets(response,RESP_LEN,stdin);
	return(rpmatch(response) == 1);
}

/* 
 * NAME: rm_msg     
 *
 * FUNCTION: determine appropriate message to print from bad return
 *           from rmdir or unlink subroutine call
 * RETURNS:
 *  if (errno = EEXIST or (errno = EBUSY and path is a known directory)
 *  then if (path is the current directory path)
 *       then write "cannot remove current directory", return 1
 *       else write "directory is not empty",          return 2
 *  else write (whatever perror writes),               return 0
 *
 *  path is the path name that should be echoed to the user.  It may be longer than
 *  PATH_MAX.  short_path should be used for actual path access, it will always be
 *  less than PATH_MAX.  These values may  be the same if the path name is shorter
 *  than PATH_MAX.
 */
static int rm_msg(cmd,path,short_path,is_dir)
char *cmd, *path, *short_path;
int is_dir;
{
  int rc=0;
  if ((errno == EEXIST) || ((errno == EBUSY) && (is_dir == 1))) {
       if (is_cwd(short_path)) {
           fprintf(stderr, MSGSTR(CANTRMSELF,"%1$s: Cannot remove the current directory %2$s.\n"),cmd,path);
           rc = 1;
       } 
       else {
           fprintf(stderr, MSGSTR(CANTRMFULL,"%1$s: Directory %2$s is not empty.\n"),cmd,path);
           rc = 2;
       }
  } 
  else {
        fprintf(stderr, MSGSTR(NOTREMOVED,"%1$s: %2$s not removed.\n"),cmd,path);
        perror("");
	rc = 3;
  }
  return(rc);
}

/* 
 * NAME: is_cwd
 *
 * FUNCTION:  determine whether s is current directory path.
 *            this routine is called by rm_msg. 
 *
 * RETURNS:   1 if s is the current directory path.
 */
static int is_cwd(s)
char *s;
{
  struct stat statbuf;

  /* If this is first call, find device and file serial of current directory.*/
  if (!is_cwd_called++) {
      char pwdbuf[PATH_MAX];

      /* Get current directory path.*/
      if ((char *)getcwd(pwdbuf,sizeof(pwdbuf))==NULL)
	  return(0); 	/* Can't prove it is cwd.*/
      if (stat(pwdbuf,&statbuf) != 0) 
	  return(0); 	/* Can't prove it is cwd.*/
      cwd_st_dev = statbuf.st_dev;
      cwd_st_ino = statbuf.st_ino;
  }

  /* Find device and file serial of candidate path.*/
  if (stat(s,&statbuf) != 0)
	return(0); 	/* No valid status, can't be current path.*/

  /* Candidate is current directory if it has same device and file serial.*/
  return( (cwd_st_dev == statbuf.st_dev) && (cwd_st_ino == statbuf.st_ino) );
}
