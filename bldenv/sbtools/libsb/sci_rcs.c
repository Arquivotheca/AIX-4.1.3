static char sccsid[] = "@(#)57  1.1  src/bldenv/sbtools/libsb/sci_rcs.c, bldprocess, bos412, GOLDA411a 4/29/93 12:23:51";
/*
 * WARNING WARNING WARNING WARNING
 * WARNING WARNING WARNING WARNING
 * WARNING WARNING WARNING WARNING
 *
 * This file is only intended for use with bsubmit!!
 * The functions contained herein are not fully functional and will not work
 * properly with other programs, such as bci, bco, etc. !!
 * Note that many of the routines in this module duplicate the functionality
 * of other code and are declared statically here.
 *
 * It is expected that these functions will be generalized in a future release.
 *
 */

/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */
/*
 * This module provides the following functions:
 *
 * sci_init
 * sci_set_source_info
 * sci_lookup_leader_list
 * sci_all_list
 * sci_lookup_user_rev_list
 * sci_lookup_latest_rev_list
 * sci_ancestor_rev_list
 * sci_locked_list
 * sci_is_branch
 * sci_check_in_list
 * sci_lock_list
 * sci_update_build_list
 * sci_outdate_list
 * sci_add_to_list
 * sci_trackfile
 * sci_config_lookup_list
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: sci_rcs.c,v $ $Revision: 1.3.3.2 $ (OSF) $Date: 92/03/09 19:14:22 $";
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ode/parse_rc_file.h>
#include <ode/odedefs.h>
#include <stdio.h>
#include <varargs.h>
#include <ode/sci_rcs.h>
#include <sys/time.h>
#ifdef INC_SIGNAL
#include <sys/signal.h>
#endif
#include <string.h>
#ifdef NO_UTIMES
#include <utime.h>
#endif


/* extern char *sindex(), *index(), *getwd(); */
extern char *sindex(), *getwd();


#define TMPMODE         0777    /* mode for temporary working file */
#define LOGMODE         0644    /* mode for temporary message file */
#define LOGPROMPT       "<<<replace with log message"
#define LOGMSGSIZE      4096    /* -m<msg> buffer size */
#define IBR_MARKER      "*** Initial Branch Revision ***"
#define MISSING         1       /* positive return value - string not found */
/* comment syntax for C files */
#define BEGINCMT        "/*"
#define CONTCMT         " * "
#define ENDCMT          "*/"
#define CR_VAR          "copyright_list"        /* rc_files variable */
#define CR              "COPYRIGHT"
#define HIST            "HISTORY"
#define BEGINLOG        "$Log"
#define ENDLOG          "$EndLog$"
#define DEF_TMPDIR	"/tmp"

#define COPYRIGHT_MARKERS "OSF_COPYRIGHT;OSF_FREE_COPYRIGHT"


#ifndef DEF_EDITOR
#    define DEF_EDITOR  "vi"    /* default editor */
#endif


#define ATOI(n, p) \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        return(1); \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

/*
 * GLOBAL VARIABLES:
 */

int buf_lines;

char *copyright_markers;

/* linked list element for copyright markers */
struct copy_elem {
  char *cr_name;                /* copyright marker string */
  struct copy_elem *cr_next;    /* pointer to next marker */
};

struct copy_elem *copy_list;    /* head of linked list */

STATIC
char * EDIT_PROG;

STATIC
char temp1[MAXPATHLEN], temp2[MAXPATHLEN], temp3[MAXPATHLEN];
STATIC
char temp4[MAXPATHLEN], temp5[MAXPATHLEN];
STATIC
int temp_merge_setup = FALSE;

STATIC
char mesgfile[MAXPATHLEN];      /* temporary file for log message */

STATIC
sigset_t sig_block_set_base;
STATIC
sigset_t *sig_block_set;

STATIC
char *rcfile_source_base;
STATIC
char *rcfile_source_testdir;
STATIC
char *rcfile_source_host;
STATIC
char *rcfile_source_owner;
STATIC
char *rcfile_source_cover;
STATIC
char *rcfile_rcs_base;
STATIC
char *rcfile_rcs_host;
STATIC
char *rcfile_rcs_owner;
STATIC
char *rcfile_rcs_cover;

STATIC
char *RCSCOVER;
STATIC
char *BCSBBASE;                 /* R/W */
STATIC
char *SRCCOVER;                 /* R */
STATIC
char *BCSTEMP;                  /* R */
STATIC
char *BCSSET_NAME;              /* R/W */
STATIC
char *USER;                     /* R */

STATIC
char bcslock[MAXPATHLEN];       /* bcs lock file */
STATIC
char bcsconfig[MAXPATHLEN];     /* bcs config file */
STATIC
char bcsset[MAXPATHLEN];        /* bcs set file */
STATIC
char bcslog[MAXPATHLEN];        /* bcs log file */
STATIC
char bcspath[MAXPATHLEN];       /* bcs path file */
STATIC
char bcstempbuf[MAXPATHLEN];    /* buffer for generated BCSTEMP */
STATIC
char trackfile[MAXPATHLEN];     /* path and name of tracking file */

STATIC
char *curdir;
STATIC
char curdirbuf[MAXPATHLEN];

STATIC
char working_file[MAXPATHLEN];
STATIC
char working_file_dir[MAXPATHLEN];
STATIC
char working_file_tail[MAXPATHLEN];
STATIC
char canon_working_file[MAXPATHLEN];
STATIC
char temp_working_file[MAXPATHLEN];

STATIC
int usetrunk;                   /* using trunk directly */
STATIC
char setrev[MAXPATHLEN];        /* set revision */

struct table {
    int maxa;                   /* max argc before realloc of argv */
    int argc;                   /* current argument count */
    char **argv;                /* arguments */
};

STATIC
struct table currel;

struct logmsg {
    struct logmsg *next;
    char *revhdr;
    short nlines;
    short maxlines;
    char **body;
    char *stamp;
    int seenlog;
};

/*
 * FORWARD DECLARATIONS
 */

STATIC
src_ctl_ancestor ();

STATIC
src_ctl_set_leader ();

STATIC
src_ctl_set_remove ();

STATIC
set_cleanup ();

/*
 * FUNCTIONS/PROCEDURES
 */

# define MAX_FUNCTION_DEPTH 100

STATIC
int current_depth = 0;
char * current_function [ MAX_FUNCTION_DEPTH ];


enter ( func_name )
char * func_name;
{
  if ( current_depth == MAX_FUNCTION_DEPTH ) {
    ui_print ( VALWAYS, "\n*** INTERNAL ERROR 0002 ***\n\n" );
    ui_print ( VFATAL, "Exceeded limit set on function depth.\n" );
    ui_print ( VCONT, "Limit set at: %d\n", MAX_FUNCTION_DEPTH );
    ui_print ( VALWAYS, "\n" );
  } else
    current_function [ current_depth ] = func_name;
  /* end if */
  ui_print ( VDEBUG, "Depth: %d. Entering '%s'.\n", current_depth, func_name );
  current_depth++;
} /* end enter */


leave ( )
{
  if ( current_depth < MAX_FUNCTION_DEPTH ) {
    current_depth--;
    ui_print ( VDEBUG, "Depth: %d. Leaving '%s'.\n", current_depth,
                       current_function [ current_depth ] );
    if ( current_depth > 0 ) 
      ui_print ( VCONT, "Returning to '%s'\n",
                         current_function [ current_depth - 1 ] );
    /* end if */
  } else
    ui_print ( VDEBUG, "Depth: %d. Leaving unknown function.\n",
                       --current_depth );
  /* end if */
} /* end leave */


report_current_function ( )
{
  if ( current_depth >= MAX_FUNCTION_DEPTH )
    ui_print ( VCONT, "Current function: unknown!\n" );
  else
    ui_print ( VCONT, "Current function: '%s'\n",
                      current_function [ current_depth ] );
  /* end if */
} /* end report_current_function */


/*
 * Generate the full path and name for a given set
 */
int full_set_name ( fsn, set_name )
char ** fsn;
char * set_name;
{
  char      * env_ptr, *ptr;                        /* point to env string */
  char        tmp_name1 [ PATH_LEN ];                        /* misc string */
  char        tmp_name2 [ PATH_LEN ];                        /* misc string */

/*
 * Not sure what this next statement does
  if ((( *set_name < 'A' ) || ( *set_name > 'Z' )) &&
      ( strncmp ( tmp_name, set_name, ptr - tmp_name )))
*/
/*
 * Not sure what this next statement does
  else
    strcpy ( set_file_name, set_name );
 */
  /* end if */

  concat ( tmp_name1, NAME_LEN, BCS_SET, set_name, NULL );
  *fsn = salloc ( tmp_name1 );
  if ( *fsn == NULL)
    return ( ERROR );
  else
    return ( OK );
  /* end if */
}


STATIC
int rcs_child_init(ap)
va_list ap;
{
    char *dir;
    int closefd;
    int outfd;
    int fd;

    dir = va_arg(ap, char *);
    closefd = va_arg(ap, int);
    outfd = va_arg(ap, int);

    (void) setgid(getgid());
    (void) setuid(getuid());

    if (setenv("AUTHCOVER_HOST", rcfile_rcs_host, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_HOST setenv failed" );
      return ( ERROR );
    }
    if (setenv("AUTHCOVER_USER", rcfile_rcs_owner, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_USER setenv failed" );
      return ( ERROR );
    }
    if (setenv("AUTHCOVER_TESTDIR", rcfile_rcs_base, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_TESTDIR setenv failed" );
      return ( ERROR );
    }
    if (dir != NULL && chdir(dir) < 0) {
        ui_print ( VFATAL, "%s chdir failed", dir);
        return ( ERROR );
    }

    if (closefd >= 0)
        (void) close(closefd);

    if (outfd >= 0 && outfd != 1) {
        (void) dup2(outfd, 1);
        (void) close(outfd);
    }

    if ((fd = open("/dev/null", O_RDONLY, 0)) > 0) {
        (void) dup2(fd, 0);
        (void) close(fd);
    }
    return ( OK );
}


STATIC
char * alloc_switch(swtch, value)
char swtch;
char *value;
{
    int len;
    char *buf;

    if (value == NULL)
        len = 0;
    else
        len = strlen(value);
    buf = (char *) malloc(len + 3);
    if (buf == NULL) {
      ui_print ( VFATAL, "alloc failed\n" );
      return(NULL);
    }
    buf[0] = '-';
    buf[1] = swtch;
    if (len)
        bcopy(value, buf + 2, len);
    buf[len + 2] = '\0';
    return(buf);
}


STATIC
int rcsfullstat( file_name, rev_str, revision )
char *file_name;
char *rev_str;
char *revision;
{
    char buffer[MAXPATHLEN];
    FILE *inf;
    int p[2];
    int pid;
    char *ptr;
    int status;
    int i;
    char *av[16];

#ifndef NO_DEBUG
  enter ( "rcsfullstat" );
#endif
  rev_str = alloc_switch ( 'r', rev_str );
  if ( rev_str == NULL ) {
    ui_print ( VFATAL, "Revision alloc switch failed" );
    leave ( );
    return ( ERROR );
  } /* end if */
  if (revision == NULL) {
	i = 0;
	av[i++] = "authcover";
	av[i++] = "rcsstat";
	av[i++] = "-q";
	av[i++] = rev_str;
	av[i++] = file_name;
	av[i++] = NULL;
	pid = full_runcmdv(RCSCOVER, av, FALSE,
			   rcs_child_init, (char *)NULL, -1, -1);
	if (pid == -1) {
          leave ( );
	  return( ERROR );
        }
        leave ( );
	return(endcmd(pid));
    }
    if (pipe(p) < 0) {
	ui_init ( VFATAL, "rcsstat pipe failed" );
        leave ( );
	return( ERROR );
    }
    i = 0;
    av[i++] = "authcover";
    av[i++] = "rcsstat";
    av[i++] = "-q";
/*
    if (defunctp)
*/
    av[i++] = "-D";
    av[i++] = "-V";
    av[i++] = rev_str;
    av[i++] = file_name;
    av[i++] = NULL;
    pid = full_runcmdv(RCSCOVER, av, FALSE,
		       rcs_child_init, (char *)NULL, p[0], p[1]);
    *revision = '\0';
/*
    if (defunctp)
	*defunctp = FALSE;
*/
    if (pid == -1) {
      leave ( );
      return( ERROR );
    }
    (void) close(p[1]);
    if ((inf = fdopen(p[0], "r")) == NULL) {
	ui_print ( VWARN, "rcsstat fdopen failed");
	(void) endcmd(pid);
        leave ( );
	return( ERROR );
    }
    if (fgets(buffer, sizeof(buffer), inf) != NULL) {
	if ((ptr = index(buffer, '\n')) != NULL)
	    *ptr = '\0';
	(void) strcpy(revision, buffer);
    }
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_init ( VWARN, "error reading rcsstat pipe");
    (void) endcmd(pid);
    leave ( );
    return( ERROR );
  }
  status = endcmd(pid);
  if (status == 0 && *revision == '\0')
    status = ERROR;
  ui_print ( VDEBUG, "rcsstat returned revision %s\n", revision );
/*
  if (status == 0 && defunctp) {
    ptr = revision + strlen( revision ) - (sizeof(" (defunct)") - 1);
    if (ptr > revision && *ptr == ' ' && strcmp(ptr, " (defunct)") == 0) {
      *ptr = '\0';
      *defunctp = TRUE;
    }
  }
*/
  leave ( );
  return(status);
}


STATIC
int rcs_cmp_func(str1, str2, len, skipped)
char *str1, *str2;
int len, *skipped;
{
    int cmp;

    cmp = strncmp(str1, str2, len);
    if (cmp == 0) {
        if (strcmp(str1 + len, str2 + len) == 0)
            *skipped = TRUE;
    }
    return(cmp);
} /* rcs_cmp_func */


STATIC
int insert_line_in_sorted_file(filename, line, cmp_func, cmp_arg)
char *filename, *line;
int (*cmp_func)();
int cmp_arg;
{
    char buf[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    FILE *inf, *outf;
    int didit, skipped;
    int cmp;

    if (ui_ver_level() >= VDEBUG) {
        if (access(filename, R_OK) < 0)
            ui_print ( VDETAIL, "creating %s\n", filename);
        else
            ui_print ( VDETAIL, "updating %s\n", filename);
    }
    (void) concat(buf, sizeof(buf), filename, ".tmp", NULL);
    (void) unlink(buf);
    if ((outf = fopen(buf, "w")) == NULL) {
      ui_print ("unable to open %s for write", buf);
      return( ERROR );
    }
    didit = FALSE;
    skipped = FALSE;
    if ((inf = fopen(filename, "r")) != NULL) {
        while (fgets(buffer, sizeof(buffer), inf) != NULL) {
            cmp = (*cmp_func)(buffer, line, cmp_arg, &skipped);
            if (cmp > 0) {
                (void) fputs(line, outf);
                didit = TRUE;
                break;
            }
            if (cmp < 0)
                (void) fputs(buffer, outf);
        }
        if (didit)
            do {
                (void) fputs(buffer, outf);
            } while (fgets(buffer, sizeof(buffer), inf) != NULL);
        if (ferror(inf) || fclose(inf) == EOF) {
            ui_print ( VFATAL, "Error reading %s\n", filename);
            (void) fclose(outf);
            (void) unlink(buf);
            return( ERROR );
        }
    }
    if (!didit) {
        (void) fputs(line, outf);
    }
    if (ferror(outf) || fclose(outf) == EOF) {
        ui_print ( VFATAL, "error writing %s\n", buf);
        (void) unlink(buf);
        return( ERROR );
    }
    if (rename(buf, filename) < 0) {
        ui_print ( VFATAL, "rename %s to %s failed\n", buf, filename);
        (void) unlink(buf);
        return( ERROR );
    }
    return( OK );
}


STATIC
int src_ctl_config_insert( file_name, rev )
char * file_name;
char *rev;
{
    char cwbuf[MAXPATHLEN];
    char *ptr;
    int len;

    if (rev == NULL)
        rev = "1.1";
    ptr = concat(cwbuf, sizeof(cwbuf), file_name , ",v\t", NULL);
    len = ptr - cwbuf;
    (void) concat(ptr, sizeof(cwbuf) - len, rev, "\n", NULL);
    return(insert_line_in_sorted_file(bcsconfig, cwbuf, rcs_cmp_func, len));
} /* src_ctl_config_insert */


STATIC
int sci_config_update( sci_ptr, rev1, rev2, rev3)
SCI_ELEM sci_ptr;
char *rev1, *rev2, *rev3;
{
  char cmprev[32];
  int status;

  if (*rev3 == '\0') {
    status = src_ctl_ancestor( sci_ptr, rev1, rev2, &rev3, (int *)NULL);
    if (status != 0)
      return(status);
  }
  strcpy ( cmprev, sci_ptr -> ver_config );
  if (strcmp(rev3, cmprev) == 0) {
    status = src_ctl_config_insert( sci_ptr -> name, rev2 );
    if ( status !=  OK )
      return(status);
  }
  return( OK );
} /* sci_config_update */


STATIC
int src_child_init(ap)
va_list ap;
{
    char *dir;
    int fd;

    dir = va_arg(ap, char *);

    (void) setgid(getgid());
    (void) setuid(getuid());

    if (setenv("AUTHCOVER_HOST", rcfile_source_host, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_HOST setenv failed\n");
      return ( ERROR );
    }
    if (setenv("AUTHCOVER_USER", rcfile_source_owner, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_USER setenv failed\n");
      return ( ERROR );
    }
    if (setenv("AUTHCOVER_TESTDIR", rcfile_source_testdir, TRUE) < 0) {
      ui_print ( VFATAL, "AUTHCOVER_TESTDIR setenv failed\n");
      return ( ERROR );
    }

    if (dir != NULL && chdir(dir) < 0) {
      ui_print ( VFATAL, "%s chdir failed\n", dir);
      return ( ERROR );
    }

    if ((fd = open("/dev/null", O_RDONLY, 0)) > 0) {
        (void) dup2(fd, 0);
        (void) close(fd);
    }
  return ( OK );
} /* end src_child_init */


STATIC
int makedir(dir)
char *dir;
{
  char dbuf[MAXPATHLEN];
  char *ptr;

  ptr = concat(dbuf, sizeof(dbuf), dir, NULL);
  if (*(ptr-1) != '/')
    *ptr++ = '/';
  *ptr++ = '.';
  *ptr++ = '\0';
  return(makepath(dbuf, NULL, TRUE, TRUE));
} /* end makedir */


STATIC
int simple_cmp_func(str1, str2, arg, skipped)
char *str1, *str2;
int arg;
int *skipped;
{
    int cmp;
    cmp = strcmp(str1, str2);
    if (cmp == 0)
        *skipped = TRUE;
    return(cmp);
} /* end simple_cmp_func */


/*
 * This is rather archaic, and should be re-done, but it
 * is okay for now.
 */
STATIC
int check_path(w)
char *w;
{
  char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];
  char prompt[MAXPATHLEN];
  struct stat statb;
  char *ptr;

  /*
   * first we check the working file
   */
  enter ( "check_path" );
  path(w, dbuf, fbuf);
  (void) strcpy(working_file_dir, dbuf);
  if (*fbuf == '.' && *(fbuf+1) == '\0') {
    ui_print ( VFATAL, ".: invalid directory\n");
    return( ERROR );
  }
  if (*dbuf == '.' && *(dbuf+1) == '\0')
    *dbuf = '\0';
  else {
    if (*dbuf == '.' && *(dbuf+1) == '/')
        (void) strcpy(dbuf, dbuf+2);
    ptr = dbuf + strlen(dbuf);
    *ptr++ = '/';
    *ptr = '\0';
  }
  (void) strcpy(working_file, w);
  (void) strcpy(working_file_tail, fbuf);
  (void) concat(canon_working_file, sizeof(canon_working_file),
                "./", dbuf, fbuf, NULL);
  (void) concat(temp_working_file, sizeof(temp_working_file),
                BCSTEMP, "/", fbuf, NULL);
  leave ( "check_path" );
  return( OK );
} /* check_path */


STATIC
int set_insert()
{
    char cwbuf[MAXPATHLEN];

    (void) concat(cwbuf, sizeof(cwbuf), canon_working_file, "\n", NULL);
    return(insert_line_in_sorted_file(bcsset, cwbuf, simple_cmp_func, 0));
} /* end set_insert */


STATIC
int remove_working_file()
{
    int pid;
    int i;
    int status;
    char *av[16];

    if (SRCCOVER == NULL) {
        (void) unlink(working_file);
        return( OK );
    }
    i = 0;
    av[i++] = "authcover";
    av[i++] = "rm";
    av[i++] = "-f";
    av[i++] = canon_working_file;
    av[i++] = NULL;
    pid = full_runcmdv(SRCCOVER, av, FALSE,
                       src_child_init, (char *)NULL);
    if (pid == -1)
        return( ERROR );
    status = endcmd(pid);
    if (status != 0)
        return( ERROR );
    return( OK );
} /* end remove_working_file */


STATIC
int copy_file (src, dst, local )
char *src, *dst;
BOOLEAN local; /* local file copy? If FALSE, dst is on another machine */
{
    char tmp[MAXPATHLEN];
    int sfd;
    int tfd;
#ifdef NO_UTIMES
    struct utimbuf tv;
#else
    struct timeval tv[2];
#endif
    struct stat st, statb;

  ui_print ( VDEBUG, "Entering copy_file\n" );
  ui_print ( VDEBUG, "src :%s:\n", src );
  ui_print ( VDEBUG, "dst :%s:\n", dst );
  if (stat(src, &st) < 0) {
    ui_print ( VFATAL, "stat %s \n", src);
    return( ERROR );
  }
  if ( ! local ) {
    int pid;
      int status;
      int i;
      char *av[16];

      if (remove_working_file() == ERROR )
          return( ERROR );
      i = 0;
      av[i++] = "authcover";
      av[i++] = "-t1";
      av[i++] = "1";
      av[i++] = "cp";
      av[i++] = src;
      av[i++] = canon_working_file;
      av[i++] = NULL;
      pid = full_runcmdv(SRCCOVER, av, FALSE, src_child_init, (char *)NULL);
      if (pid == -1)
        return( ERROR );
      status = endcmd(pid);
      if (status != 0)
        return( ERROR );
      return( OK );
    }
    sfd = open(src, O_RDONLY, 0);
    if (sfd < 0) {
        ui_print ( VFATAL, "open %s\n", src);
        return( ERROR );
    }
    (void) concat(tmp, sizeof(tmp), dst, ".tmp", NULL);
    (void) unlink(tmp);
    tfd = open(tmp, O_WRONLY|O_CREAT|O_EXCL, 0600);
    if (tfd < 0 && dst == working_file) {
        if (stat(working_file_dir, &statb) < 0) {
            if (makedir(working_file_dir) != 0)
                return( ERROR );
        } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
            ui_print ( VFATAL, "%s: not a directory\n", working_file_dir);
            return( ERROR );
        }
        tfd = open(tmp, O_WRONLY|O_CREAT|O_EXCL, 0600);
    }
    if (tfd < 0) {
        ui_print ( VFATAL, "open %s\n", tmp);
        (void) close(sfd);
        return( ERROR );
    }
    if (filecopy(sfd, tfd) < 0) {
        ui_print ( VFATAL, "filecopy %s to %s failed\n", src, tmp);
        (void) close(sfd);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (close(sfd) < 0) {
        ui_print ( VFATAL, "close %s\n", src);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (close(tfd) < 0) {
        ui_print ( VFATAL, "close %s\n", tmp);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (chmod(tmp, (int)st.st_mode&0777) < 0) {
        ui_print ( VFATAL, "chmod %s\n", tmp);
        (void) unlink(tmp);
        return( ERROR );
    }
#ifdef NO_UTIMES
    tv.actime = st.st_atime;
    tv.modtime = st.st_mtime;
    if (utime(tmp, &tv) < 0) {
        ui_print ( VFATAL, "utime %s\n", tmp);
        (void) unlink(tmp);
        return( ERROR );
    }
#else
    tv[0].tv_sec = st.st_atime;
    tv[1].tv_sec = st.st_mtime;
    tv[0].tv_usec = tv[1].tv_usec = 0;
    if (utimes(tmp, tv) < 0) {
        ui_print ( VFATAL, "utimes %s\n", tmp);
        (void) unlink(tmp);
        return( ERROR );
    }
#endif
    if (rename(tmp, dst) < 0) {
        ui_print ( VFATAL, "rename %s to %s failed\n", tmp, dst);
        return( ERROR );
    }
  ui_print ( VDEBUG, "Leaving copy_file\n" );
    return( OK );
} /* end copy_file */


STATIC
int src_ctl_check_out_with_fd( file_name, rev, fd, leader)
char *rev;
int fd;
char *leader;
{
  int status;
  int pid;
  int i;
  char *av[16];
  char rcs_file_name [MAXPATHLEN];

  ui_print ( VDEBUG, "Entering src_ctl_check_out_with_fd\n" );
    if ((rev = alloc_switch('p', rev)) == NULL) {
      ui_print ( VFATAL, "revision alloc switch failed\n");
      return ( ERROR );
    }
    i = 0;
    concat ( rcs_file_name, sizeof (rcs_file_name), file_name, ",v", NULL );
    av[i++] = "authcover";
    av[i++] = "co";
    av[i++] = "-q";
/*
    if (locked)
        av[i++] = "-l";
*/
    av[i++] = rev;
    if (strcmp(leader, "BIN") == 0)   /* test if file is binary */
      av[i++] = "-ko";
    av[i++] = rcs_file_name;
    av[i++] = NULL;
    pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, BCSTEMP, -1, fd);
    if (pid == -1)
      return( ERROR );
    status = endcmd(pid);
  if (status != 0)
    ui_print ( VFATAL, "co command failed\n");
  else
    status = OK;
  /* end if */
  ui_print ( VDEBUG, "Leaving src_ctl_check_out_with_fd\n" );
  return(status);
} /* end src_ctl_check_out_with_fd */


STATIC
int src_ctl_check_out ( file_name, rev, leader )
char * file_name;
char * rev;
char * leader;
{
  int pid;
  int i;
  char *av[16];
  char ch2[2];
  char rcs_file_name [MAXPATHLEN];

  if ((rev = alloc_switch('r', rev)) == NULL) {
    ui_print ( VDEBUG, "Revision alloc switch failed\n");
    return ( ERROR );
  }

  concat ( rcs_file_name, sizeof ( rcs_file_name ), file_name, ",v", NULL );
  i = 0;
  av[i++] = "authcover";
/*
  av[i++] = "-t0";
*/
  av[i++] = "-t2";
  av[i++] = ch2;
  av[i++] = "co";
  av[i++] = rev;
  if (strcmp(leader, "BIN") == 0)   /* test if file is binary */
    av[i++] = "-ko";
  ch2[0] = '0' + i - 3; ch2[1] = '\0';
  av[i++] = working_file_tail;
  av[i++] = rcs_file_name;
  av[i++] = NULL;
  pid = full_runcmdv(RCSCOVER, av, FALSE, rcs_child_init, BCSTEMP, -1, -1);
  if (pid == -1)
    return( ERROR );
  ui_print ( VDEBUG, "Leaving src_ctl_check_out\n" );
  return(endcmd(pid));
} /* end src_ctl_check_out */


STATIC
int src_ctl_config_remove( sci_ptr )
SCI_ELEM sci_ptr;
{
  char buf[MAXPATHLEN];
  char buffer[MAXPATHLEN];
  char cwbuf[MAXPATHLEN];
  FILE *inf, *outf;
  int wcnt;
  char *ptr;
  int len;

  ui_print ( VDEBUG, "Entering src_ctl_config_remove\n" );
  ui_print ( VDETAIL, "Removing from ./.BCSconfig:\n");
  ui_print ( VCONT, "%s\n", sci_ptr -> name );
  (void) concat(buf, sizeof(buf), bcsconfig, ".tmp", NULL);
  (void) unlink(buf);
  if ((outf = fopen(buf, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", buf);
    return( ERROR );
  }
  ptr = concat(cwbuf, sizeof(cwbuf), sci_ptr -> name, ",v\t", NULL);
  len = ptr - cwbuf;
  wcnt = 0;
  if ((inf = fopen(bcsconfig, "r")) != NULL) {
    while (fgets(buffer, sizeof(buffer), inf) != NULL) {

      if (strncmp(buffer, cwbuf, len) == 0) {
        ui_print ( VDETAIL, "%s\n", buffer);
        continue;
      }
      (void) fputs(buffer, outf);
      wcnt++;
    }
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", bcsconfig);
      (void) fclose(outf);
      (void) unlink(buf);
      return( ERROR );
    }
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", buf);
    (void) unlink(buf);
    return( ERROR );
  }
  if (rename(buf, bcsconfig) < 0) {
    ui_print ( VFATAL, "Rename %s to %s failed\n", buf, bcsconfig);
    return ( ERROR );
  }

  if (wcnt == 0 && unlink(bcsconfig) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSconfig\n" );
  return( OK );
} /* end src_ctl_remove_config */


STATIC
int track_insert ( file_name )
char * file_name;
{
  char cwbuf[MAXPATHLEN];

  (void) concat(cwbuf, sizeof(cwbuf), file_name, "\n", NULL);
  return(insert_line_in_sorted_file(trackfile, cwbuf, simple_cmp_func, 0));
}


begin_atomic ( )
{
  sigprocmask ( SIG_BLOCK, sig_block_set, NULL );
  ui_print ( VDEBUG, "begin_atomic\n" );
} /* end begin_atomic */


end_atomic ( )
{
  ui_print ( VDEBUG, "end_atomic\n" );
  sigprocmask ( SIG_UNBLOCK, sig_block_set, NULL );
} /* end end_atomic */


/*
 * log_error and is_in_error are temporary place holders
 * for more comprehensive error handling routines.
 * They are only necessary for a few routines.
 */

int global_status = OK;

log_error ( )
{
  global_status = ERROR;
} /* end log_error */


BOOLEAN is_in_error ( )
{
  return ( global_status == ERROR );
} /* end is_in_error */


STATIC
int sci_read_leader( rcs_file, leader)
char * rcs_file;
char *leader;
{
    char temp[MAXPATHLEN];
    FILE *inf;
    char buf[MAXPATHLEN];
    char *ptr;
    char *lptr;
    int found;
    int fd;
    int pid;
    int status;
    int i;
    char *av[16];

    (void) concat(temp, sizeof(temp), BCSTEMP, "/_HEADERS_", NULL);
    (void) unlink(temp);
    if ((fd = open(temp, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
        ui_print ( VFATAL, "Unable to open %s for write\n", temp);
        (void) unlink(temp);
        return( ERROR );
    }
    i = 0;

    av[i++] = "authcover";
    av[i++] = "rlog";
    av[i++] = "-h";
    av[i++] = rcs_file;
    av[i++] = NULL;
    pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, BCSTEMP, -1, fd);
    if (pid == -1) {
        (void) close(fd);
        (void) unlink(temp);
        return( ERROR );
    }
    (void) close(fd);
    status = endcmd(pid);
    if (status != 0) {
        ui_print ( VFATAL, "rlog command failed\n");
        (void) unlink(temp);
        return( ERROR );
    }
    if ((inf = fopen(temp, "r")) == NULL) {
        ui_print ( VFATAL, "fopen %s", temp);
        (void) fclose(inf);
        (void) unlink(temp);
        return( ERROR );
    }
    found = FALSE;
    while (fgets(buf, sizeof(buf), inf) != NULL) {
        if (strncmp(buf, "comment leader:", 15) == 0) {
            found = TRUE;
            break;
        }
    }
    if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VFATAL, "error reading %s", temp);
        (void) fclose(inf);
        (void) unlink(temp);
        return( ERROR );
    }
    (void) unlink(temp);
    if (!found) {
        ui_print ( VFATAL, "Missing \"comment leader\" header in %s", rcs_file);
        return( ERROR );
    }
    if ((ptr = index(buf, '\n')) == NULL) {
        ui_print ( VFATAL, "Bad comment leader for %s", rcs_file);

        return( ERROR );
    }
    *ptr-- = '\0';
    if (*ptr != '"') {
        ui_print ( VFATAL, "missing '\"' after comment for %s", rcs_file);
        return( ERROR );
    }
    lptr = index(buf, *ptr);
    if (lptr++ == ptr) {
        ui_print ( VFATAL, "bad comment leader for %s", rcs_file);
        return( ERROR );
    }
    bcopy(lptr, leader, ptr - lptr);
    leader[ptr-lptr] = '\0';
    ui_print ( VDEBUG, "In sci_read_leader, leader is '%s'\n", leader );
    return( OK );
} /* end sci_read_leader */


STATIC
numfields(r)
register char *r;
{
    int n[2];
    int c = 0;

    for (;;) {
        ATOI(n[0], r)
        if (*r++ != '.')
            return( ERROR );
        c++;
        ATOI(n[1], r)
        if (*r != '.' && *r != '\0' && *r != ' ')
            return( ERROR );
        c++;
        if (*r++ == '.')
            continue;
        return(c);
    }
}


STATIC
char *months[] = {
    "Jan",    "Feb",    "Mar",    "Apr",    "May",    "Jun",
    "Jul",    "Aug",    "Sep",    "Oct",    "Nov",    "Dec",
    NULL
};


STATIC
month_num(mptr, bptr)
char *mptr, *bptr;
{
    char **mp, *p;

    for (mp = months; (p = *mp) != NULL; mp++)
        if (*p == *mptr && *(p+1) == *(mptr+1) && *(p+2) == *(mptr+2))
            break;
    if (p == NULL) {
        *bptr++ = '?';
        *bptr = '?';
    } else {
        *bptr++ = ((mp-months)+1)/10 + '0';
        *bptr = ((mp-months)+1)%10 + '0';
    }
} /* end month_num */


STATIC
is_whist_header(buf)
char *buf;
{
    char tmpbuf[MAXPATHLEN];
    char *bp, *p, *q;
    int i;

    q = buf + 1;
    while ((q = index(q + 1, '-')) != NULL) {
        if (*(q + 4) != '-' || *(q + 7) != ' ' || *(q + 8) != ' ' ||
            (p = sindex(q + 9, ") at ")) == NULL)
            continue;
        p--;
        while (*p != '(' && p > q)
            p--;
        if (*p++ != '(')
            continue;
        q -= 2;
        bp = tmpbuf;
        *bp++ = *(q+7);
        *bp++ = *(q+8);
        *bp++ = '/';
        month_num(q+3, bp);
        bp += 2;
        *bp++ = '/';
        *bp++ = (*q == ' ') ? '0' : *q;
        *bp++ = *(q+1);
        for (i = 0; i < 12; i++)
            *bp++ = ' ';
        while (*p != ')')
            *bp++ = *p++;
        *bp++ = '\0';
        (void) bcopy(tmpbuf, buf, bp - tmpbuf);
        return( TRUE );
    }
    return( FALSE );
}


STATIC
struct logmsg *
logalloc(logtemp)
struct logmsg *logtemp;
{
    struct logmsg *newlog;

    newlog = (struct logmsg *)calloc(1, sizeof(struct logmsg));
    if (newlog == NULL)
        return(NULL);
    newlog->revhdr = logtemp->revhdr;
    newlog->nlines = logtemp->nlines;
    newlog->maxlines = logtemp->maxlines;
    newlog->body = logtemp->body;
    newlog->stamp = logtemp->stamp;
    newlog->seenlog = logtemp->seenlog;
    bzero((char *)logtemp, sizeof(struct logmsg));
    return(newlog);
}


STATIC
savelog(headptr, tailptr, logtemp)
struct logmsg **headptr;
struct logmsg **tailptr;
struct logmsg *logtemp;
{
  if (*headptr == NULL) {
    *headptr = logalloc(logtemp);
    if (*headptr == NULL) {
      ui_print ( VFATAL, "Logalloc failed\n");
      return( ERROR );
    }
    *tailptr = *headptr;
    return( OK );
  }
  (*tailptr)->next = logalloc(logtemp);
  if ((*tailptr)->next == NULL) {
    ui_print ( VFATAL, "Logalloc failed\n");
    return( ERROR );
  }
  *tailptr = (*tailptr)->next;
  return( OK );
}


STATIC
addtobody(lm, buffer)
struct logmsg *lm;
char *buffer;
{
    char *p;
    int l;

    if (lm->nlines >= lm->maxlines) {
        if (lm->maxlines == 0) {
            lm->maxlines = 4;
            lm->body =  (char **) malloc(lm->maxlines * sizeof(char *));
        } else {
            lm->maxlines += 4;
            lm->body =  (char **) realloc(lm->body,
                                          lm->maxlines * sizeof(char *));
        }
        if (lm->body == NULL) {
            ui_print ( VFATAL, "Body alloc failed\n");
            return( ERROR );
        }
    }
    if ((p = index(buffer, '\n')) == NULL)
        p = buffer + strlen(buffer);
    while (p > buffer && (*(p-1) == ' ' || *(p-1) == '\t'))
        p--;
    if (p == buffer)
        p = NULL;
    else {
        l = p - buffer;
        if ((p = (char *) malloc(l + 1)) == NULL) {
            ui_print ( VFATAL, "Body malloc failed\n");
            return( ERROR );
        }
        bcopy(buffer, p, l);
        p[l] = '\0';
    }
    lm->body[lm->nlines++] = p;
    return( OK );
}


STATIC
remove_ibr_markers(lm)
struct logmsg *lm;
{
    char *p, *q, *lastq;
    char **body = lm->body;
    int nlines = lm->nlines;
    int i, j, k;

    i = 0;
    for (;;) {
        if (i == nlines)
            break;
        if (body[i] == NULL) {
            i++;
            continue;
        }
        p = sindex(body[i], IBR_MARKER);
        if (p == NULL) {
            i++;
            continue;
        }
        q = p + sizeof(IBR_MARKER) - 1;
        while (p > body[i] && (*(p-1) == ' ' || *(p-1) == '\t'))
            p--;
        if (p > body[i]) {
            i++;
            continue;
        }
        while (*q == ' ' || *q == '\t')
            q++;
        if (*q != '\0') {
            i++;
            continue;
        }
        j = i + 1;
        if (j == nlines) {
            break;
        }
        if ((q = body[j]) != NULL && *q == '[') {
            q += strlen(q) - 1;
            if (*q != ']') {
                ui_print ( VWARN, "Please remove \"%s\" message\n", IBR_MARKER);
                return( ERROR );
            }
            q = NULL;
        }
        while (q == NULL) {
            j++;
            if (j == nlines)
                break;
            q = body[j];
        }
        if (j == nlines) {
            break;
        }
        k = i;
        while (j < nlines) {
            if (body[k] != NULL)
                (void) free(body[k]);
            body[k++] = body[j];
            body[j++] = NULL;
        }
        nlines = k;
    }
    while (i > 0 && body[i-1] == NULL)
        i--;
    lm->nlines = i;
    return( OK );
}


STATIC
save_stamp_match(lm, ptr)
struct logmsg *lm;
char *ptr;
{
    char buffer[MAXPATHLEN];
    char *tptr;

    while (*ptr == ' ')
        ptr++;
    if (*(ptr+4) != '/' || *(ptr+7) != '/' || *(ptr+10) != ' ')
        return( OK );
    tptr = ptr;
    ptr += 10;
    while (*ptr == ' ')
        ptr++;
    if (*(ptr+2) != ':' || *(ptr+5) != ':' || *(ptr+8) != ' ')
        return( OK );
    lm->stamp = salloc(tptr);
    if (lm->stamp == NULL) {
        ui_print ( VFATAL, "Stamp salloc failed\n");
        return( ERROR );
    }
    return( OK );
}


STATIC
BOOLEAN check_branches(lm, branches, nrev)
struct logmsg *lm;
char **branches;
int nrev;
{
    int i;
    char *ptr;
    char *bptr;
    char *bufptr;

    if (branches) {
        for (i = 0; branches[i] != NULL; i++) {
            ptr = lm->revhdr;
            bptr = branches[i];
            while (*bptr != '\0' && *ptr == *bptr) {
                ptr++;
                bptr++;
            }
            if (*bptr != '\0')
                continue;
            while (*ptr >= '0' && *ptr <= '9')
                ptr++;
            if (*ptr != ' ')

                continue;
            bptr = lm->body[lm->nlines-1];
            if (*bptr != '[')
                bptr = NULL;
            else {
                bptr += strlen(bptr) - 1;
                if (*bptr != ']')
                    bptr = NULL;
            }
            if (bptr == NULL && save_stamp_match(lm, ptr) != OK ) {
              ui_print ( VFATAL, "Unknown error in check_branches 1\n" );
              return( ERROR );
            }
            return( FALSE );
        }
    }
    if (nrev == 0) {
        return( TRUE );
    }
    i = 0;
    ptr = lm->revhdr;
    for (;;) {
        while (*ptr >= '0' && *ptr <= '9')
            ptr++;
        i++;
        if (*ptr != '.')

            break;
        ptr++;
    }
    if (*ptr != ' ' || i <= nrev)
        return( TRUE );
    bptr = lm->body[lm->nlines-1];
    if (*bptr != '[')
        bptr = NULL;
    else {
        bptr += strlen(bptr) - 1;
        if (*bptr != ']')
            bptr = NULL;
    }
    if (bptr == NULL && save_stamp_match(lm, ptr) != 0) {
        ui_print ( VFATAL, "Unknown error in check_branches 3\n" );
        return( ERROR );
    }
    return( TRUE );
}


STATIC
process_log_messages(loghead, leader, outf, mesgf, branches, nrev)
struct logmsg *loghead;
char *leader;
FILE *outf, *mesgf;
char **branches;
int nrev;
{
    char *p, *q;
    struct logmsg *lm;
    FILE *fp;
    int i;
    int status;
    int donemsg = FALSE;

  enter ( "process_log_messages" );
  ui_print ( VDEBUG, "leader '%s'\n", leader );
  ui_print ( VDEBUG, "nrev '%d'\n", nrev );
    for (lm = loghead; lm != NULL; lm = lm->next) {
        if (lm->nlines == 0) {
            ui_print ( VDETAIL, "Removed empty RCS log message\n");
            continue;
        }
        if (remove_ibr_markers(lm) != OK ) {
          leave ( );
          return( ERROR );
        } /* end if */
        if (lm->nlines == 0) {
            ui_print ( VDETAIL, "Removed empty RCS log message\n");

            continue;
        }
        if (lm->seenlog == FALSE)
            fp = mesgf;
        else if (lm->revhdr == NULL)
            fp = outf;
        else {
            status = check_branches(lm, branches, nrev);
            if (status < 0) {
              leave ( );
              return( ERROR );
            }
            if (status == 0)
                fp = mesgf;
            else
                fp = outf;
        }
        if (fp == mesgf) {
            if (lm->stamp == NULL && lm->revhdr != NULL) {
                p = lm->body[lm->nlines-1];
                if (*p != '[')
                    p = NULL;
                else {
                    p += strlen(p) - 1;
                    if (*p != ']')
                        p = NULL;
                }
                if (p == NULL) {
                    p = lm->revhdr;
                    while (*p != ' ')
                        p++;
                    if (save_stamp_match(lm, p) != 0) {
                      leave ( );
                      return( ERROR );
                    }
                }
            }
            if (donemsg)
                fprintf(fp, "\n");
            for (i = 0; i < lm->nlines; i++) {
                p = lm->body[i];
                if (p == NULL)
                    fprintf(fp, "\n");
                else
                    fprintf(fp, "%s\n", p);
            }
            if (lm->stamp != NULL) {
                fprintf(fp, "[%s]\n", lm->stamp);
            }
            donemsg = TRUE;
            continue;
        }
        fprintf(fp, "%sRevision ", leader);
        if (lm->revhdr == NULL) {
            if (lm->stamp == NULL)
                fprintf(fp, "0.0  00/00/00  00:00:00  unknown\n");
            else
                fprintf(fp, "0.0  %s\n", lm->stamp);
        } else
            fprintf(fp, "%s\n", lm->revhdr);
        for (i = 0; i < lm->nlines; i++) {
            p = lm->body[i];
            if (p == NULL)
                fprintf(fp, "%s\n", leader);
            else
                fprintf(fp, "%s\t%s\n", leader, p);
        }
        if (lm->stamp != NULL)
            fprintf(fp, "%s\t[%s]\n", leader, lm->stamp);
        fprintf(fp, "%s\n", leader);
    }
    leave ( );
    return( OK );
}


STATIC
int src_ctl_extract_history(mesgf, leader, branches, rev)
FILE *mesgf;
char *leader;
char **branches;
char *rev;
{
    char newtemp[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char stamp[MAXPATHLEN];
    int havestamp;
    int owenewline;
    char *ptr, *bptr;
    char *bufptr;
    int leaderlen, wsleaderlen;
    int i;
    int seenhdr;
    int wanthdr;
    int iswhist;
    FILE *inf, *outf;
    int seenmarker;
    struct logmsg *loghead;
    struct logmsg *logtail;
    struct logmsg logtemp;
    int nrev;
    int savelog_status;
    int pl_status;
    int no_ws_leaderlen;
 
    enter ( "src_ctl_extract_history" );
    ui_print ( VDEBUG, "rev is '%s'\n", rev );
    if (rev == NULL)
	nrev = 0;
    else {
	nrev = numfields(rev);
	if (nrev == -1) {
	    ui_print ( VFATAL, "Numfields failed\n");
            leave ( );
	    return( ERROR );
	}
    }
    if (leader == '\0') {
	ui_print ( VFATAL, "Empty leader.\n");
        leave ( );
	return( ERROR );
    }
    if (branches)
	    ui_print ( VDETAIL, "Scanning for HISTORY and $Log...$ messages\n");
	else
	    ui_print ( VDETAIL, "Scanning for HISTORY\n" );
    for (ptr = leader; *ptr != '\0'; ptr++)
	if (*ptr != ' ' && *ptr != '\t')
	    leaderlen = (ptr - leader) + 1;
    wsleaderlen = ptr - leader;
    no_ws_leaderlen = wsleaderlen - 1;
    havestamp = FALSE;
    owenewline = FALSE;
    seenmarker = FALSE;
    loghead = NULL;
    logtail = NULL;
    bzero((char *)&logtemp, sizeof(logtemp));
    if ((inf = fopen(temp_working_file, "r")) == NULL) {
	ui_print ( VFATAL, "fopen %s\n", temp_working_file);
        leave ( );
	return( ERROR );
    }
    (void) concat(newtemp, sizeof(newtemp), temp_working_file, ".new", NULL);
    (void) unlink(newtemp);
    if ((outf = fopen(newtemp, "w")) == NULL) {
	ui_print ( VFATAL, "Fopen %s\n", newtemp);
	(void) fclose(inf);
	(void) unlink(newtemp);
        leave ( );
	return( ERROR );
    }
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
	(void) fputs(bufptr, outf);
	if (sindex(bufptr, "HISTORY") != NULL) {
	    seenmarker = TRUE;
	    break;
	}
    }
    if (!seenmarker) {
	(void) fclose(inf);
	(void) fclose(outf);
	(void) unlink(newtemp);
        ui_print ( VFATAL, "Missing HISTORY marker.\n" );
        leave ( );
	return( ERROR );
    }
    seenmarker = FALSE;
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
	if ((ptr = sindex(bufptr, "$Log")) != NULL &&
	    (*(ptr+4) == '$' || *(ptr+4) == ':')) {
	    (void) fputs(bufptr, outf);
	    seenmarker = TRUE;
	    break;
	}
	if (is_whist_header(bufptr)) {
	    owenewline = FALSE;
	    if (havestamp) {
		if (savelog(&loghead, &logtail, &logtemp) != 0)
		    break;
	    }
	    logtemp.stamp = salloc(bufptr);
	    if (logtemp.stamp == NULL) {
		ui_print ( VFATAL, "Stamp salloc failed\n");
		break;
	    }
	    havestamp = TRUE;
	    continue;
	}
	if (strncmp(bufptr, leader, leaderlen) != 0)
	    break;
	bufptr += leaderlen;
	for (i = leaderlen; i < wsleaderlen; i++) {
	    if (*bufptr != leader[i])
		break;
	    bufptr++;
	}
	for (ptr = bufptr; *ptr != '\0' && *ptr != '\n'; ptr++)
	    if (*ptr != ' ' && *ptr != '\t')
		break;
	if (*ptr == '\0' || *ptr == '\n') {
	    owenewline = TRUE;
	    continue;
	}
	if (*bufptr == '\t')
	    bufptr++;
	if (owenewline) {
	    if (addtobody(&logtemp, "\n") != 0)
		break;
	    owenewline = FALSE;
	}
	if (addtobody(&logtemp, bufptr) != 0)
	    break;
	havestamp = TRUE;
    }
    if (!seenmarker) {
	(void) fclose(inf);
	(void) fclose(outf);
	(void) unlink(newtemp);
        ui_print ( VFATAL, "Missing Log Marker\n" );
        leave ( );
	return( ERROR );
    }
    seenmarker = FALSE;
    seenhdr = FALSE;
    wanthdr = TRUE;
    iswhist = FALSE;
    if (!havestamp)
	logtemp.seenlog = TRUE;
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
	if (strncmp(bufptr, leader, wsleaderlen) == 0 &&
	    *(bufptr + wsleaderlen) == '$' &&
	    *(bufptr + wsleaderlen + 7) == '$' &&
	    strncmp(bufptr + wsleaderlen + 1, "EndLog", 6) == 0) {
	    seenmarker = TRUE;
	    break;
	}
	if (wanthdr) {
	    if (!iswhist && strncmp(bufptr, leader, wsleaderlen) == 0 &&
		strncmp(bufptr + wsleaderlen, "Revision ", 9) == 0) {
		bufptr += wsleaderlen + 9;
		if (havestamp) {
		    if (savelog(&loghead, &logtail, &logtemp) != 0) {
			break;
                    }
		    logtemp.seenlog = TRUE;
		}
		ptr = bufptr + strlen(bufptr) - 1;
		if (*ptr == '\n')
		    *ptr = '\0';
		logtemp.revhdr = salloc(bufptr);
		if (logtemp.revhdr == NULL) {
		    ui_print ( VFATAL, "Revhdr salloc failed\n");
		    break;
		}
		havestamp = TRUE;
		seenhdr = TRUE;
		wanthdr = FALSE;
		continue;
	    }
	    if (is_whist_header(bufptr)) {
		if (havestamp) {
		    if (savelog(&loghead, &logtail, &logtemp) != 0) {
			break;
                    }
		    logtemp.seenlog = TRUE;
		}
		logtemp.stamp = salloc(bufptr);
		if (logtemp.stamp == NULL) {
		    ui_print ( VFATAL, "Stamp salloc failed\n");
		    break;
		}
		havestamp = TRUE;
		seenhdr = TRUE;
		wanthdr = FALSE;
		iswhist = TRUE;
		continue;
	    }
	}
	if (!seenhdr) {
	    break;
        }
	if (!iswhist) {
	    if (strncmp(bufptr, leader, no_ws_leaderlen ) != 0) {
		break;
            }
            bufptr += wsleaderlen;
	} else {
	    if (strncmp(bufptr, leader, no_ws_leaderlen) != 0) {
		break;
            }
            bufptr += wsleaderlen;
	    if (index(" \t\n", *bufptr) == NULL) {
		break;
            }
	    for (i = leaderlen; i < wsleaderlen; i++) {
		if (*bufptr != leader[i])
		    break;
		bufptr++;
	    }
	}
	if (*bufptr == '\t')
	    bufptr++;
	for (ptr = bufptr; *ptr != '\0' && *ptr != '\n'; ptr++)
	    if (*ptr != ' ' && *ptr != '\t')
		break;
	if (*ptr == '\0' || *ptr == '\n') {
	    if (!wanthdr)
		wanthdr = TRUE;
	    continue;
	}
	if (wanthdr && addtobody(&logtemp, "\n") != 0) {
	    break;
        }
	if (addtobody(&logtemp, bufptr) != 0) {
	    break;
        }
	if (wanthdr)
	    wanthdr = FALSE;
    }
    if (!seenmarker ||
	(havestamp && ( savelog_status = savelog(&loghead, &logtail, &logtemp))
        != 0) ||
	( pl_status = process_log_messages(loghead, leader, outf, mesgf,
        branches, nrev)) != OK )
    {
        ui_print ( VFATAL, "unknown error in src_ctl_xtract_history\n" );
	(void) fclose(inf);
	(void) fclose(outf);
	(void) unlink(newtemp);
        leave ( );
	return( ERROR );
    }
	    (void) fputs(bufptr, outf);
    (void) ffilecopy(inf, outf);
  if (ferror(inf) || fclose(inf) == EOF) {
	ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
	(void) fclose(inf);
	(void) fclose(outf);
	(void) unlink(newtemp);
        leave ( );
	return( ERROR );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
	ui_print ( VFATAL, "Error writing %s\n", newtemp);
	(void) fclose(outf);
	(void) unlink(newtemp);
        leave ( );
	return( ERROR );
  }
  if (rename(newtemp, temp_working_file) < 0) {
    ui_print ( VFATAL, "Rename %s to %s\n", newtemp, temp_working_file);
    (void) fclose(inf);
    (void) unlink(newtemp);
    leave ( );
    return( ERROR );
  }
  leave ( );
  return( OK );
}


/*
 * This function currently only handles one revision in xlog as opposed
 * to the multiple ones in the previous version ( from if_rcs.c ).
 */
STATIC
int src_ctl_lookup_logmsg ( file_name, rev, leader, xlog, nxlog )
char *file_name;
char *rev;
char *leader;
char *xlog;
int nxlog;
{
    char buf[MAXPATHLEN];
    char brbuf[MAXPATHLEN];
    char **branches;
    int nbr;
    int maxbr;
    char xlogrev[MAXPATHLEN];
    FILE *mesgf;
    int status;
    char *ptr, *bptr;
    int i;

  enter ( "src_ctl_lookup_logmsg" );
  ui_print ( VDEBUG, "rev is '%s'\n", rev );
      rev = NULL;
      nbr = 0;
      maxbr = 0;
      bptr = brbuf;

      branches = (char **)malloc((nxlog + 1) * sizeof(char *));
      if (branches == NULL) {
        ui_print ( VFATAL, "Branches malloc failed\n");
        leave ( );
        return ( ERROR );
      }
      branches[nxlog] = NULL;
      ptr = xlogrev;
#ifdef notdef
      *ptr++ = '-';
      *ptr++ = 'r';
#endif
      for (i = 0; i < nxlog; i++) {

#ifdef notdef
            if (*xlog[i] >= '0' && *xlog[i] <= '9')

	    (void) strcpy(xlogrev+2, xlog[i]);
            (void) strcpy(xlogrev, xlog[i]);

            if (src_ctl_lookup_revision( file_name, xlogrev, buf) != 0)
#endif
            strcpy ( buf, xlog );
            if (*buf != NUL) {
                ptr = buf + strlen(buf) - 1;
                while (*ptr >= '0' && *ptr <= '9')
                    ptr--;
                if (*ptr++ == '.')
                    *ptr = NUL;
                else
                    *buf = NUL;
            }

          if (*buf == NUL) {
            ui_print ( VWARN, "No branch named '%s' - ignored\n", xlog);
          } else {
              ptr = concat(bptr, brbuf+sizeof(brbuf)-bptr,
                           buf, NULL);
              if (ptr == NULL) {
                  ui_print ( VWARN, "Too many branches - '%s' ignored.\n", xlog);
                  bptr = ptr;
              } else {
                  branches[nbr++] = bptr;
                  bptr = ++ptr;
              }
          }
      }
  ui_print ( VDEBUG, "In src_ctl_lookup_logmsg mesgfile is '%s'\n", mesgfile );
  if ((mesgf = fopen(mesgfile, "w")) == NULL) {
      ui_print ( VFATAL, "fopen %s\n", mesgfile);
      (void) unlink(mesgfile);
      leave ( );
      return( ERROR );
  }
  status = src_ctl_extract_history(mesgf, leader, branches, rev);
  if (ferror(mesgf) || fclose(mesgf) == EOF) {
      if (status ==  OK )
          ui_print ( VFATAL, "Error writing %s\n", mesgfile);
      (void) fclose(mesgf);
      (void) unlink(mesgfile);
      leave ( );
      return( ERROR );
  }
  leave ( );
  return(status);
}


STATIC
int sci_xtract_log ( sci_ptr, leader )
SCI_ELEM sci_ptr;
char * leader;
{
  int status;
  struct stat st;
  FILE *inf;
  char buf[MAXPATHLEN];
  char *def;
  char *ptr;
  int fd;

  enter ( "sci_xtract_log" );
  (void) unlink(mesgfile);
  status = src_ctl_lookup_logmsg( sci_ptr -> name, sci_ptr -> ver_user, leader,
                                  sci_ptr -> ver_user, 1 );
  if (status == OK && stat(mesgfile, &st) == 0 && st.st_size > 0) {
    if ((inf = fopen(mesgfile, "r")) == NULL) {
      ui_print ( VFATAL, "fopen %s\n", mesgfile);
      leave ( );
      return ( ERROR );
    } else {
      while (fgets(buf, sizeof(buf), inf) != NULL) {
        (void) fputs(leader, stdout);
        if (*buf != NUL && *buf != '\n') {
          (void) putchar('\t');
        }
        (void) fputs(buf, stdout);
      }
      if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VFATAL, "Error reading %s\n", mesgfile);
        (void) fclose(inf);
        leave ( );
        return ( ERROR );
      } else {
        leave ( );
        return ( OK );
      }
    }
  } else {
    if ( status == OK ) {
      ui_print ( VWARN, "Empty or non-existant mesgfile.\n" );
      ui_print ( VCONT, "File: '%s'\n", mesgfile );
      ui_print ( VCONT, "No log information was recorded!!\n" );
      leave ( );
      return ( OK );
    } else {
      leave ( );
      return ( ERROR );
    } /* if */
  } /* if */
} /* end sci_xtract_log */


STATIC
int save_log_message(mesgfile)
char *mesgfile;
{
  FILE *inf;
  FILE *outf;

  ui_print ( VDEBUG, "Entering save_log_message\n" );
  ui_print ( VDETAIL, "Updating ./.BCSlog-%s", BCSSET_NAME);
  if ((inf = fopen(mesgfile, "r")) == NULL) {
    ui_print ( VFATAL, "fopen %s\n", mesgfile);
    return( ERROR );
  }
  if ((outf = fopen(bcslog, "a")) == NULL) {
    ui_print ( VFATAL, "fopen %s\n", bcslog);
    (void) fclose(inf);
    return( ERROR );
  }
  fprintf(outf, "[ %s ]\n", canon_working_file);
  (void) ffilecopy(inf, outf);
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_print ( VFATAL, "Error reading %s\n", mesgfile);
    (void) fclose(inf);
    (void) fclose(outf);

    return( ERROR );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", bcslog);
    (void) fclose(outf);
    return( ERROR );
  }
  ui_print ( VDEBUG, "Leaving save_log_message\n" );
  return( OK );
} /* end save_log_message */



/*
 * check contents of log message file, must not contain end-of-comment,
 * must differ from default prompt and can't exceed length limit
 * returns: OK - valid log message
 *          ERROR - on error
 */
STATIC
int okmesg(leader, logbuf)
char *leader, *logbuf;
{
    char buf[MAXPATHLEN];
    FILE *inf;
    int cfile;
    char *ptr;

    enter ( "okmesg" );
    ptr = logbuf;
    if ((inf = fopen(mesgfile, "r")) == NULL) {
        ui_print ( VFATAL, "fopen %s\n", mesgfile);
        leave ( );
        return ( ERROR );
    }
    cfile = (strcmp(leader, CONTCMT) == 0);
    while (fgets(buf, sizeof(buf), inf) != NULL) {
        if (sindex(buf, LOGPROMPT) != NULL) {
            ui_print ( VFATAL, "Found %s ...>>> marker\n", LOGPROMPT);
            (void) fclose(inf);
            leave ( );
            return ( ERROR );
        }
        if (cfile && sindex(buf, ENDCMT) != NULL) {
            ui_print ( VFATAL, "Comment terminator in log message\n");
            (void) fclose(inf);
            leave ( );
            return ( ERROR );
        }
        if (*buf != NUL && *buf != '\n')
            *ptr++ = '\t';
        ptr = concat(ptr, logbuf+LOGMSGSIZE-ptr-1, buf, NULL);
        if (ptr == NULL)
            break;
    }
    if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VFATAL, "Error reading %s\n", mesgfile);
        (void) fclose(inf);
        leave ( );
        return ( ERROR );
    }
    leave ( );
    return ( OK );
}


STATIC
int shorter(r1, r2)
register char *r1, *r2;
{
    int n1[2], n2[2];

    for (;;) {
        ATOI(n1[0], r1)
        ATOI(n2[0], r2)
        if (*r1++ != '.')
            return( ERROR );
        if (*r2++ != '.')
            return( ERROR );
        ATOI(n1[1], r1)
        ATOI(n2[1], r2)
        if (*r1 != '.' && *r1 != '\0')
            return( ERROR );
        if (*r2 != '.' && *r2 != '\0')
            return( ERROR );
        if (*r1 == '\0')
            return(*r2 == '\0' ? 0 : 1);
        if (*r2 == '\0')
            return( OK );
        r1++;

        r2++;
    }
} /* end shorter */


STATIC
int extract_log( sci_ptr, inf, outf, mesgf, rev)
SCI_ELEM sci_ptr;
FILE *inf, *outf, *mesgf;
char *rev;
{
    char buffer[MAXPATHLEN];
    char revbuf[MAXPATHLEN];
    char *ptr, *bptr;
    char *bufptr;
    int leaderlen;
    int inmatch;
    int rlogok;
    int rlogged;
    int whistok;
    int whisted;

    ptr = concat(revbuf, sizeof(revbuf), rev, NULL);
    do {
        ptr--;
    } while (*ptr >= '0' && *ptr <= '9');
    if (*ptr++ != '.') {
      ui_print ( VFATAL, "invalid release %s", rev);
      return( ERROR );
    }
    *ptr = '\0';
    leaderlen = strlen( sci_ptr -> leader );
    whisted = FALSE;
    rlogged = FALSE;
    whistok = FALSE;
    rlogok = FALSE;
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
        if (sindex(bufptr, "HISTORY") != NULL) {
            whisted = TRUE;
            break;
        }
        if ((ptr = sindex(bufptr, "$Log")) != NULL &&
            (*(ptr+4) == '$' || *(ptr+4) == ':')) {
            rlogged = TRUE;
            break;
        }
        (void) fputs(bufptr, outf);
    }
    if (whisted) {
        (void) fputs(bufptr, outf);
        while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
            if ((ptr = sindex(bufptr, "$Log")) != NULL &&
                (*(ptr+4) == '$' || *(ptr+4) == ':')) {
                whistok = TRUE;
                rlogged = TRUE;
                break;
            }
            (void) fputs(bufptr, outf);
        }
    }
    if (rlogged) {
        (void) fputs(bufptr, outf);
        while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
            if (strncmp(bufptr, sci_ptr -> leader, leaderlen) == 0 &&
                *(bufptr + leaderlen) == '$' &&
                *(bufptr + leaderlen + 7) == '$' &&
                strncmp(bufptr + leaderlen + 1, "EndLog", 6) == 0) {
                rlogok = TRUE;
                (void) fputs(bufptr, outf);
                (void) ffilecopy(inf, outf);
                bufptr = NULL;
                break;
            }
            if (strncmp(bufptr, sci_ptr -> leader, leaderlen) == 0 &&
                strncmp(bufptr + leaderlen, "Revision ", 9) == 0) {

                ptr = bufptr + leaderlen + 9;
                bptr = revbuf;
                while (*bptr != '\0' && *ptr == *bptr) {
                    ptr++;
                    bptr++;
                }
                if (*bptr == '\0') {
                    while (*ptr >= '0' && *ptr <= '9')
                        ptr++;
                }
                if (*bptr == '\0' && *ptr == ' ') {
                    inmatch = TRUE;
                    (void) fputs(bufptr, mesgf);
                } else {
                    inmatch = FALSE;
                    (void) fputs(bufptr, outf);
                }
                continue;
            }
            /* looking for pattern "..-...-..  \(.*\) at " */
            ptr = bufptr + 1;
            while ((ptr = index(ptr+1, '-')) != NULL) {

                if (*(ptr+4) != '-' || *(ptr+7) != ' ' || *(ptr+8) != ' ' ||
                    sindex(ptr+9, ") at ") == NULL)
                    continue;
                break;
            }
            if (ptr != NULL || strncmp(bufptr, sci_ptr -> leader, leaderlen) != 0) {
                rlogok = TRUE;
                (void) fputs(bufptr, outf);
                (void) ffilecopy(inf, outf);
                bufptr = NULL;
                break;
            }
            if (!inmatch) {
                (void) fputs(bufptr, outf);
            } else {
                (void) fputs(bufptr, mesgf);
            }
        }
    }
  if ((rlogged && !rlogok) || (whisted && !whistok)) {
    ui_print ( VFATAL, "Inconsistent HISTORY or $Log...$ markers");
    return( ERROR );
  }
  return( OK );
} /* extract log */


STATIC
int extract_log_messages( sci_ptr, mesgfile, rev1, rev2)
SCI_ELEM sci_ptr;
char *mesgfile, *rev1, *rev2;
{
  FILE *inf, *outf, *mesgf;
  int status;
  struct stat st;

  ui_print ( VDEBUG, "Entering extract_log_messages\n" );
  ui_print ( VDEBUG, "Leader is '%s'\n", sci_ptr -> leader );
  if ( *(sci_ptr -> leader) == '\0' || strcmp( sci_ptr -> leader, "NONE") == 0
				    || strcmp (sci_ptr -> leader, "BIN") ) {
    ui_print ( VDEBUG, "No leader\n" );
    return( OK );
  } /* end if */
  if ((inf = fopen(temp1, "r")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for read\n", temp1);
    return( ERROR );
  } /* end if */
  (void) unlink(temp3);
  if ((outf = fopen(temp3, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp3);
    (void) fclose(inf);
    (void) unlink(temp3);
    return( ERROR );
  } /* end if */
  if ((mesgf = fopen(mesgfile, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", mesgfile);
    (void) fclose(inf);
    (void) fclose(outf);
    (void) unlink(temp3);
    (void) unlink(mesgfile);
    return( ERROR );
  }
  if (extract_log( sci_ptr, inf, outf, mesgf, rev1) != 0) {
    ui_print ( VFATAL, "extract_log failed\n" );
    (void) fclose(inf);
    (void) fclose(outf);
    (void) fclose(mesgf);
    (void) unlink(mesgfile);
    (void) unlink(temp3);
    return( ERROR );
  }
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_print ( VFATAL, "Error reading %s\n", temp1);
    (void) fclose(inf);
    (void) fclose(outf);
    (void) fclose(mesgf);
    (void) unlink(mesgfile);
    (void) unlink(temp3);
    return( ERROR );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", temp3);
    (void) fclose(outf);
    (void) fclose(mesgf);
    (void) unlink(mesgfile);
    (void) unlink(temp3);
    return( ERROR );
  }
  if (rename(temp3, temp1) < 0) {
      ui_print ( VFATAL, "Rename %s to %s failed\n", temp3, temp1);
      (void) fclose(mesgf);
      (void) unlink(mesgfile);
      (void) unlink(temp3);
      return( ERROR );
  }
  ui_print ( VDEBUG, " rev1, rev2: '%s' '%s'\n", rev1, rev2 );
  if ((status = shorter(rev2, rev1)) == ERROR ) {
      ui_print ( VFATAL, "Unknown error in extract log messages (1).\n" );
      (void) fclose(mesgf);
      (void) unlink(mesgfile);
      return( ERROR );
  }
  if (status == 0) {
      if ((inf = fopen(temp2, "r")) == NULL) {
          ui_print ( VFATAL, "Unable to open %s for read\n", temp1);
          (void) fclose(mesgf);
          (void) unlink(mesgfile);
          return( ERROR );
      }
      (void) unlink(temp3);
      if ((outf = fopen(temp3, "w")) == NULL) {
          ui_print ( VFATAL, "Unable to open %s for write\n", temp3);
          (void) fclose(inf);
          (void) fclose(mesgf);
          (void) unlink(mesgfile);
          (void) unlink(temp1);
          (void) unlink(temp2);
          (void) unlink(temp3);
          return( ERROR );
      }
      if (extract_log( sci_ptr, inf, outf, mesgf, rev2 ) != 0) {
        ui_print ( VFATAL, "Unknown error in extract log messages (2).\n" );
        (void) fclose(inf);
        (void) fclose(outf);
        (void) fclose(mesgf);
        (void) unlink(mesgfile);
        (void) unlink(temp3);
        return( ERROR );
      }
      if (ferror(inf) || fclose(inf) == EOF) {
          ui_print ( VFATAL, "error reading %s\n", temp2);
          (void) fclose(inf);
          (void) fclose(outf);
          (void) fclose(mesgf);
          (void) unlink(mesgfile);
          (void) unlink(temp3);
          return( ERROR );
      }
      if (ferror(outf) || fclose(outf) == EOF) {
        ui_print ( VFATAL, "Error writing %s\n", temp3);
        (void) fclose(outf);
        (void) fclose(mesgf);
        (void) unlink(mesgfile);
        (void) unlink(temp3);
        return( ERROR );
      }
      if (rename(temp3, temp2) < 0) {
        ui_print ( VFATAL, "Rename %s to %s failed\n", temp3, temp2);
        (void) fclose(mesgf);
        (void) unlink(mesgfile);
        (void) unlink(temp3);
        return( ERROR );
      }
  }
  if (ferror(mesgf) || fclose(mesgf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", mesgfile);
    (void) fclose(mesgf);
    (void) unlink(mesgfile);
    return( ERROR );
  }
  if (stat(mesgfile, &st) == 0 && st.st_size > 0) {
    if ((inf = fopen(mesgfile, "r")) == NULL) {
      ui_print ( VFATAL, "fopen %s\n", mesgfile);
      (void) unlink(mesgfile);
      return( ERROR );
    }
    (void) ffilecopy(inf, stdout);
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", mesgfile);
      (void) fclose(inf);
      (void) unlink(mesgfile);
      return( ERROR );
    }
  } else
    (void) unlink(mesgfile);
  return( OK );
}


STATIC
int reinsert_log_messages(mesgfile)
char *mesgfile;
{
  char leader[MAXPATHLEN];
  FILE *inf, *outf, *mesgf;
  int status;
  char buffer[MAXPATHLEN];
  char *ptr;

  if ((inf = fopen(temp_working_file, "r")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for read\n", temp_working_file);
    return( ERROR );
  }
  if ((mesgf = fopen(mesgfile, "r")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for read\n", mesgfile);
    (void) fclose(inf);
    return( ERROR );
  }
  (void) unlink(temp1);
  if ((outf = fopen(temp1, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp1);
    (void) fclose(inf);
    (void) fclose(mesgf);
    (void) unlink(temp1);
    return( ERROR );
  }
  status = ERROR;
  while ((fgets(buffer, sizeof(buffer), inf)) != NULL) {
      (void) fputs(buffer, outf);
      if ((ptr = sindex(buffer, "$Log")) != NULL &&
          (*(ptr+4) == '$' || *(ptr+4) == ':')) {
          (void) ffilecopy(mesgf, outf);
          (void) ffilecopy(inf, outf);
          status = OK;
          break;
      }
  }
  if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
      (void) fclose(inf);
      (void) fclose(outf);
      (void) fclose(mesgf);
      (void) unlink(temp1);
      return( ERROR );
  }
  if (ferror(mesgf) || fclose(mesgf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", mesgfile);
      (void) fclose(outf);
      (void) fclose(mesgf);
      (void) unlink(temp1);
      return( ERROR );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", temp1);
    (void) fclose(outf);
    (void) unlink(temp1);
    return( ERROR );
  }
  if (status == ERROR) {
    ui_print ( VFATAL, "Could not find HISTORY marker again\n");
    (void) unlink(temp1);
    return( ERROR );
  }
  if (rename(temp1, temp_working_file) < 0) {
    ui_print ( VFATAL, "Rename %s to %s failed\n", temp1, temp_working_file);
    (void) unlink(temp1);
    return( ERROR );
  }
  (void) unlink(mesgfile);
  return( OK );
}



STATIC
int src_ctl_diff_rev_with_file(rev, filename, rcs_file, fd)
char * rev;
char * filename;
char * rcs_file;
int fd;
{
    int pid;
    int i;
    char *av[16];
    char comma_v_file [ MAXPATHLEN ];

    if ((rev = alloc_switch('r', rev)) == NULL) {
      ui_print ( VFATAL, "Revision alloc switch failed\n" );
      return ( ERROR );
    }

    concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
    i = 0;
    av[i++] = "authcover";
    av[i++] = "-t1";
    av[i++] = "2";
    av[i++] = "rcsdiff";
    av[i++] = rev;
    av[i++] = filename;
    av[i++] = comma_v_file;
    av[i++] = NULL;
    /* previously piped through more */
    pid = full_runcmdv(RCSCOVER, av, FALSE,

                       rcs_child_init, (char *)NULL, -1, fd);
    if (pid == -1)
        return( ERROR );
    if (endcmd(pid) == -1)
        return( ERROR );
    return( OK );
}


STATIC
echo(va_alist)
va_dcl
{
    va_list ap;

    va_start(ap);
    echov(ap);
    va_end(ap);
}


/*
 * checks RCS environment, returns OK or ERROR
 */
STATIC
int src_ctl_check_env ( rcfile_p )
struct rcfile *rcfile_p;
{
    struct field *field_p;
    struct arg_list *args_p;
    char buf[MAXPATHLEN];
    char *ptr;

    ui_print ( VDEBUG, "Entering src_ctl_check_env\n" );
    /*
     * top of rcs tree for bcs commands
     */
    if (rc_file_field(rcfile_p, "rcs_base", &field_p) != 0) {
      ui_print ( VFATAL, "rcs_base not defined");
      return ( ERROR );
    }
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
      ui_print ( VFATAL, "improper rcs_base");
      return ( ERROR );
    }
    rcfile_rcs_base = args_p->tokens[0];
    if (*rcfile_rcs_base != '/') {
      ui_print ( VFATAL, "rcs_base is not an absolute path");
      return ( ERROR );
    }

    /*
     * authentication cover for rcs commands
     */
    if (rc_file_field(rcfile_p, "rcs_owner", &field_p) != 0) {
	ui_print ( VFATAL, "rcs_owner not defined");
      return ( ERROR );
      }
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
	ui_print ( VFATAL, "improper rcs_owner");
      return ( ERROR );
      }
    rcfile_rcs_owner = args_p->tokens[0];
    if (rc_file_field(rcfile_p, "rcs_host", &field_p) != 0) {
	ui_print ( VFATAL, "rcs_host not defined");
      return ( ERROR );
      }
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
	ui_print ( VFATAL, "improper rcs_host");
      return ( ERROR );
      }
    rcfile_rcs_host = args_p->tokens[0];
    if (rc_file_field(rcfile_p, "rcs_cover", &field_p) != 0) {
	ui_print ( VFATAL, "rcs_cover not defined");
      return ( ERROR );
      }
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
	ui_print ( VFATAL, "improper rcs_cover");
      return ( ERROR );
      }
    rcfile_rcs_cover = args_p->tokens[0];
    if (*rcfile_rcs_cover != '/') {
	ui_print ( VFATAL, "rcs_cover is not an absolute path");
      return ( ERROR );
      }
    if (access(rcfile_rcs_cover, X_OK) < 0) {
	ui_print ( VFATAL, "rcs_cover %s not executable", rcfile_rcs_cover);
      return ( ERROR );
      }
    RCSCOVER = rcfile_rcs_cover;
    ui_print ( VDEBUG, "Leaving src_ctl_check_env\n" );
    return (OK);
}


/*
 * Provide the revision of a particular file in a particular set
 */
STATIC
int src_ctl_lookup_revision ( name, rev_str, revision )
char * name;
char * rev_str; /* input */
char * revision; /* output */
{
  char tmp_str [PATH_LEN];
  int status;

  ui_print ( VDEBUG, "Entering src_ctl_lookup_revision\n" );
    strcpy ( tmp_str, rev_str );
  /* end if */
  status = rcsfullstat ( name, tmp_str, revision );
  ui_print ( VDEBUG, "Leaving src_ctl_lookup_revision\n" );
  return ( status );
}

STATIC
src_ctl_setup_merge()
{
    ui_print ( VDEBUG, "Entering src_ctl_setup_merge\n" );
    (void) concat(temp1, sizeof(temp1), BCSTEMP, "/_BMERGE_1", NULL);
    (void) concat(temp2, sizeof(temp2), BCSTEMP, "/_BMERGE_2", NULL);
    (void) concat(temp3, sizeof(temp3), BCSTEMP, "/_BMERGE_3", NULL);
    (void) concat(temp4, sizeof(temp4), BCSTEMP, "/_BMERGE_4", NULL);
    (void) concat(temp5, sizeof(temp5), BCSTEMP, "/_BMERGE_5", NULL);
    temp_merge_setup = TRUE;
    ui_print ( VDEBUG, "Leaving src_ctl_setup_merge\n" );
} /* end src_ctl_setup_merge */


int src_ctl_prep_merge(rev1, rev2, rev3, sci_ptr )
char *rev1, *rev2, *rev3;
SCI_ELEM sci_ptr;       /* expecting only an element, not an entire list */
{
  char mesgfile[MAXPATHLEN];
  int status;
  int fd;
  int pid;
  FILE *inf, *outf, *mesgf;
  int i;
  char *av[16];
  char *cmd;
  struct stat st;
  int called_getancestor;

 /*
  * REMOVE next two lines
  */
  int locked;

  locked = FALSE;

  ui_print ( VDEBUG, "Entering src_ctl_prep_merge\n" );

  if ((rev1 = alloc_switch('p', rev1)) == NULL) {
    ui_print ( VFATAL, "Revision alloc switch failed\n" );
    return ( ERROR );
  }
  if ((rev2 = alloc_switch('p', rev2)) == NULL) {
    ui_print ( VFATAL, "Revision alloc switch failed\n");
    return ( ERROR );
  }
  if ((rev3 = alloc_switch('p', rev3)) == NULL) {
    ui_print ( VFATAL, "Revision alloc switch failed\n" );
    return ( ERROR );
  }
  ui_print ( VDEBUG, "before. R1 :%s: R2 :%s: R3 :%s\n" , rev1, rev2, rev3 );

  /*
   * determine revision number for "common" revision
   */
  called_getancestor = FALSE;
  if (rev3[2] != '\0') {
    called_getancestor = FALSE;
    sci_ptr -> called_getancestor = called_getancestor;
  } else {
    status = src_ctl_ancestor( sci_ptr, rev1+2, rev2+2, &rev3,
                               &called_getancestor );
    sci_ptr -> called_getancestor = called_getancestor;
    ui_print ( VDEBUG, "after.  R1 :%s: R2 :%s: R3 :%s\n" , rev1, rev2, rev3 );
    if (status != OK)
      return( ERROR );
    if ((rev3 = alloc_switch('p', rev3)) == NULL) {
      ui_print ( VFATAL, "Revision alloc switch failed");
      return ( ERROR );
    }
  }

  /*
   * retrieve files
   */
  ui_print ( VDEBUG, "rev1,2,3: '%s' '%s' '%s'\n", rev1, rev2, rev3 );
  sci_ptr -> same23 = (strcmp(rev2, rev3) == 0);
  sci_ptr -> same13 = (strcmp(rev1, rev3) == 0);
  if ( sci_ptr -> same23 || ! sci_ptr -> same13  ) {
/*
    (void) unlink(temp1);
  if ((fd = open(temp1, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
	    ui_print ( VFATAL, "Unable to open %s for write 1\n", temp1);
	    (void) unlink(temp1);
	    return( ERROR );
  }
  ui_print ( VDIAG, "Retrieving revision %s\n", rev1+2);
*/
/*
 * Needed?
 *
  status = src_ctl_check_out_with_fd(rev1+2, FALSE, fd, sci_ptr -> leader);
	(void) close(fd);
  if (status != 0) {
    (void) unlink(temp1);
    return( ERROR );
  }

  */
	if ( sci_ptr -> same23) {
#ifdef notdef
	    if ( FALSE ) {
/*
		    ui_print ( VDIAG, "locking revision %s\n", rev2+2);
*/
/*
 * Needed ?
 *
		status = src_ctl_lock_revision(rev2+2);
 */
		if (status != 0) {
		    (void) unlink(temp1);
		    return( ERROR );
		}
	    } else
#endif
	    ui_print ( VALWAYS, "base revision %s\n", rev2+2);
	    ui_print ( VALWAYS, "common ancestor %s; will just use revision %s\n",
		       rev3+2, rev1+2);
            ui_print ( VALWAYS, "No merge required\n", rev3 );
            sci_ptr -> ver_ancestor = salloc ( rev3+2 );
            if ( sci_ptr -> ver_ancestor == NULL ) {
                  ui_print ( VFATAL, "salloc failed\n" );
                  return ( ERROR );
            }
/*
 * Needed?
	    if (rename(temp1, temp_working_file) < 0) {
		ui_print ( VFATAL, "Unable to rename %s to %s\n",
                temp1, temp_working_file);
		(void) unlink(temp1);
		return( ERROR );
	    }
 */
	    return( OK );
	}
    }
#ifdef notdef
    (void) unlink(temp2);
    if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
	ui_print ( VFATAL, "Unable to open %s for write 2\n", temp2);
	(void) unlink(temp1);
	(void) unlink(temp2);
	return( ERROR );
    }

    ui_print ( VDIAG, "Retrieving%s revision %s\n",
	       locked ? " (and locking)" : "", rev2+2);

/*
 * Probably don't need
 *
    status = src_ctl_check_out_with_fd(rev2+2, locked, fd, sci_ptr -> leader);
    (void) close(fd);
    if (status != 0) {
	(void) unlink(temp1);
	(void) unlink(temp2);
	return( ERROR );
    }
 */
#endif
    if ( sci_ptr -> same13) {
	ui_print ( VDETAIL, "Common ancestor %s; will just use revision %s\n",
		   rev3+2, rev2+2);
        ui_print ( VDETAIL, "No merge required\n", rev3 );
        sci_ptr -> ver_ancestor = salloc ( rev3+2 );
	(void) unlink(temp1);
/*
 * Needed?
 *
	if (rename(temp2, temp_working_file) < 0) {
	    ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2,
            temp_working_file);
	    (void) unlink(temp2);
	    return( ERROR );
	}
 */
    return( OK );
  }
/*
 * Test for binary file, if isn't allow merges.
 */
  if (strcmp(sci_ptr -> leader, "BIN") != 0)  {
    ui_print ( VALWAYS, "Common ancestor %s, merge required\n", rev3 );
    sci_ptr -> need_merge = TRUE;
  }
  sci_ptr -> ver_ancestor = salloc ( rev3+2 );
  if ( sci_ptr -> ver_ancestor == NULL ) {
    ui_print ( VFATAL, "salloc failed\n" );
    return ( ERROR );
  }
  ui_print ( VDEBUG, "Leaving src_ctl_prep_merge\n" );
  return ( OK );
} /* src_ctl_prep_merge */


/*
 * returns zero iff r2 is a proper ancestor of r1
 */
/*
 * FIXME
 * This should be changed to return TRUE or FALSE.
 */
STATIC
checkancestor(r1, r2)
char *r1, *r2;
{
    register char *s1, *s2;
    int n1[2], n2[2];

    s1 = r1;
    s2 = r2;
    for (;;) {
	ATOI(n1[0], s1)
	ATOI(n2[0], s2)
	if (*s1++ != '.')
	    return(1);
	if (*s2++ != '.')
	    return(1);
	ATOI(n1[1], s1)
	ATOI(n2[1], s2)
	if (*s1 != '.' && *s1 != '\0')
	    return(1);
	if (*s2 != '.' && *s2 != '\0')
	    return(1);
	if (n1[0] != n2[0] || n1[1] != n2[1] || *s1 == '\0' || *s2 == '\0')
	    break;
	s1++;
	s2++;
    }
    if (n1[0] != n2[0])
	return(n1[0] < n2[0] || (*s1 == '\0' && *s2 != '\0'));
    if (n1[1] != n2[1])
	return(n1[1] < n2[1] || (*s1 == '\0' && *s2 != '\0'));
    return(*s1 == '\0' && *s2 != '\0');
}


/*
 * FIXME
 * This should be changed to return TRUE or FALSE.
 */
STATIC
getancestor(r1, r2, r3)
char *r1, *r2, *r3;
{
    register char *s1, *s2, *s3;
    register int l3;
    char *p1, *p2;
    int n1[2], n2[2];

    s1 = r1;
    s2 = r2;
    p1 = NULL;
    p2 = NULL;
    for (;;) {
	ATOI(n1[0], s1)
	ATOI(n2[0], s2)
	if (*s1++ != '.')
	    return(1);
	if (*s2++ != '.')
	    return(1);
	ATOI(n1[1], s1)
	ATOI(n2[1], s2)
	if (*s1 != '.' && *s1 != '\0')
	    return(1);
	if (*s2 != '.' && *s2 != '\0')
	    return(1);
	if (n1[0] != n2[0] || n1[1] != n2[1] || *s1 == '\0' || *s2 == '\0')
	    break;
	p1 = s1++;
	p2 = s2++;
    }
    if (p1 == NULL) {
	p1 = s1;
	p2 = s2;
    }
    if (n1[0] < n2[0]) {
	l3 = p1 - r1;
	s3 = r1;
    } else if (n1[0] > n2[0]) {
	l3 = p2 - r2;
	s3 = r2;
    } else if (n1[1] < n2[1]) {
	l3 = s1 - r1;
	s3 = r1;
    } else if (n1[1] > n2[1]) {
	l3 = s2 - r2;
	s3 = r2;
    } else if (*s1 == '\0') {
	l3 = s1 - r1;
	s3 = r1;
    } else if (*s2 == '\0') {
	l3 = s2 - r2;
	s3 = r2;
    } else
	return(1);
    (void) strncpy(r3, s3, l3);
    r3[l3] = '\0';
    return(0);
}

STATIC
int src_ctl_config_lookup( sci_ptr, rev )
SCI_ELEM sci_ptr;
char * rev;
{
  char buffer[MAXPATHLEN];
  char cwbuf[MAXPATHLEN];
  FILE *inf;
  char *ptr, *p;
  int len;

  ui_print ( VDEBUG, "Entering src_ctl_config_lookup\n" );
  ptr = concat(cwbuf, sizeof(cwbuf), sci_ptr -> name , ",v\t", NULL);
  len = ptr - cwbuf;
  if ((inf = fopen(bcsconfig, "r")) == NULL) {
    ui_print ( VFATAL, "Can't open ancestor version file.\n" );
    ui_print ( VCONT, "This file is created/updated as files are checked out.\n"
               );
    ui_print ( VCONT, "File: '%s'\n", bcsconfig );
    return( ERROR );
  } /* end if */
  *rev = '\0';
  while (fgets(buffer, sizeof(buffer), inf) != NULL) {
    rm_newline ( buffer) ;
    if (strncmp(buffer, cwbuf, len) != 0)
      continue;
    ptr = buffer + len;
    while (*ptr == ' ' || *ptr == '\t')
      ptr++;
    if (*ptr < '0' || *ptr > '9') {
      if (strcmp(ptr, "defunct") == 0) {
        strcpy(rev, "defunct");
        sci_ptr -> defunct = TRUE;
        break;
      }
      ui_print ( VFATAL, "Invalid configuration line\n");
      (void) fputs(buffer, stderr);
      (void) fclose(inf);
      return( ERROR );
    } else if ( sci_ptr -> defunct) {
      ui_print ( VFATAL, "Contradiction between .BCSconfig file and\n" );
      ui_print ( VCONT, "information in source control system\n" );
      ui_print ( VCONT, "regarding defunct status.\n" );
      ui_print ( VCONT, "For file: '%s'\n", sci_ptr -> name );
      return ( ERROR );
    } /* end if */
    for (p = rev; (*p = *ptr) != '\0'; p++, ptr++)
        if (*p != '.' && (*p < '0' || *p > '9'))
            break;
    *p = '\0';
    break;
  }
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_print ( VFATAL, "Error reading %s\n", bcsconfig);
    return( ERROR );
  } /* end if */
  ui_print ( VDEBUG, "Config is '%s'\n", rev );
  ui_print ( VDEBUG, "Leaving src_ctl_config_lookup\n" );
  if ( *rev == '\0' ) {
    ui_print ( VFATAL, "No information for file in .BCSconfig file.\n" );
    ui_print ( VCONT, "File: '%s'\n", sci_ptr -> name );
    return ( ERROR );
  } else
    return ( OK );
  /* end if */
}

STATIC
src_ctl_ancestor( sci_ptr, rev1, rev2, rev3p, called_getancestor )
SCI_ELEM sci_ptr;
char *rev1, *rev2, **rev3p;
int *called_getancestor;
{
    char rev3[32];
    char rev3a[32];
    int status;

  ui_print ( VDEBUG, "Entering src_ctl_ancestor\n" );
  strcpy ( rev3, sci_ptr -> ver_config );
  ui_print ( VDEBUG, "config 'b' is '%s'\n", rev3 );
  ui_print ( VDEBUG, "comparison yields '%d'\n", strcmp ( rev3, rev3a ) );
  ui_print ( VDEBUG, "rev3 (1) is :%s:\n", rev3 );
  status = checkancestor(rev2, rev3);
  if (status != 0) {
      if (called_getancestor)
          *called_getancestor = TRUE;
      if (getancestor(rev1, rev2, rev3) != 0) {
          ui_print ( VWARN, "Unable to locate common ancestor of %s and %s\n", rev1, rev2);
          return( ERROR );
      }
  }
  ui_print ( VDEBUG, "rev3 (2) is :%s:\n", rev3 );
  *rev3p = salloc(rev3);
  if (*rev3p == NULL) {
    ui_print ( VFATAL, "Ancestor revision salloc failed\n");
    return ( ERROR );
  }
  ui_print ( VDEBUG, "Leaving src_ctl_ancestor\n" );
  return( OK );
} /* src_ctl_ancestor */


STATIC
int src_ctl_check_in ( sci_ptr, rev, logmsg, state )
SCI_ELEM sci_ptr;
char * rev;
char * logmsg;
char * state;
{
  char buf[MAXPATHLEN];
  int pid;
  int i;
  char *p, *lastdot;
  char *av[16];
  char ch2[2];
  char statebuf[32];
  char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];

#ifndef NO_DEBUG
  enter ( "src_ctl_check_in" );
#endif
  if ((rev = alloc_switch('u', rev)) == NULL) {
    ui_print ( VFATAL, "Revision alloc switch failed");
#ifndef NO_DEBUG
    leave ( );
#endif
    return ( ERROR );
  }
/*
 * FIXME
 *
 * This part of defuncting is in the side-effect ballpark. Yuck.
 */
  if ((logmsg = alloc_switch('m', logmsg)) == NULL) {
    ui_print ( VFATAL, "logmsg alloc switch failed");
    leave ( );
    return ( ERROR );
  } /* end if */
  if ((state = alloc_switch('s', state)) == NULL) {
    ui_print ( VFATAL, "State alloc switch failed");
    leave ( );
    return ( ERROR );
  } /* end if */

  path ( sci_ptr -> name, dbuf, fbuf );
  concat ( dbuf, sizeof ( dbuf ), sci_ptr -> name , ",v", NULL );
  i = 0;
  av[i++] = "authcover";
  av[i++] = "-t2";
  av[i++] = ch2;
  av[i++] = "ci";
  av[i++] = "-f";
  av[i++] = state;
  av[i++] = rev;
  lastdot = NULL;
  for (p = rev+2; *p != '\0'; p++)
    if (*p == '.')
      lastdot = p;
  if (lastdot != NULL)
    *lastdot = '\0';
  av[i++] = logmsg;
  ch2[0] = '0' + i - 3; ch2[1] = '\0';
  av[i++] = fbuf;
  av[i++] = dbuf;
  av[i++] = NULL;
  pid = full_runcmdv(RCSCOVER, av, FALSE, rcs_child_init, BCSTEMP, -1, -1);
  if (pid == -1) {
    leave ( );
    return( ERROR );
  }
  leave ( );
  return(endcmd(pid));
} /* end src_ctl_check_in */

STATIC
int src_ctl_lock_revision(rev, file_name )
char * rev;
char * file_name;
{
  int status;
  int pid;
  int i;
  char *av[16];

  ui_print ( VDEBUG, "Entering src_ctl_lock_revision\n" );
  if ((rev = alloc_switch('l', rev)) == NULL) {
    ui_print ( VFATAL, "Revision alloc switch failed");
    return ( ERROR );
  }

  i = 0;
  av[i++] = "authcover";
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = rev;
  av[i++] = file_name;
  av[i++] = NULL;
  pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, BCSTEMP, -1, -1);
  ui_print ( VDEBUG, "Leaving src_ctl_lock_revision\n" );
  if (pid == -1)
    return( ERROR );
  status = endcmd(pid);
  if (status != 0) {
    ui_print ( VFATAL, "rcs -l command failed\n" );
    return ( ERROR );
  }
  return(status);
} /* end src_ctl_lock_revision */


STATIC
char common_config[MAXPATHLEN];

STATIC
char *bmerge_action[] = {
    "abort",
#define MA_ABORT        0
    "ok",
#define MA_OK           1
    "edit",
#define MA_EDIT         2
    "merge",
#define MA_MERGE        3
    "co",
#define MA_CO           4
    "rco",
#define MA_RCO          5
    "diff",
#define MA_DIFF         6
    "rdiff",
#define MA_RDIFF        7
    "help",
#define MA_LEADER	8
    "leader",
#define MA_HELP         9
    NULL
};


STATIC
get_stab(prompt, table, deflt)
char *prompt, **table, *deflt;
{
/*
  if (fast || (autoflag && !noauto)) {
    ui_print ( VALWAYS, "%s  [%s]\n", prompt, deflt);
    return(stablk(deflt, table));
  }
*/
  return(getstab(prompt, table, deflt));
}


STATIC
int src_ctl_merge ( sci_ptr )
SCI_ELEM sci_ptr;
{
  int fd;
  int status;
  int pid;

  ui_print ( VDEBUG, "Entering src_ctl_merge\n" );
  if ( sci_ptr -> same23 || ! sci_ptr -> same13) {
    ui_print ( VDEBUG, "same13 :%d: same23 :%d:\n", sci_ptr -> same13,
                                                    sci_ptr -> same23 );
    (void) unlink(temp1);
    if ((fd = open(temp1, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
      ui_print ( VFATAL, "Unable to open %s for write\n", temp1);
      (void) unlink(temp1);
      return( ERROR );
    }
    ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_merge );
    status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_merge, fd, sci_ptr -> leader);
    (void) close(fd);
    if (status != 0) {
      ui_print ( VFATAL, "Check out failed.\n" );
      (void) unlink(temp1);
      return( ERROR );
    }
    if ( sci_ptr -> same23 ) {
      ui_print ( VDETAIL, "Base revision %s\n", sci_ptr -> ver_user );
      ui_print ( VDETAIL, "Common ancestor %s; will just use revision %s\n",
                 sci_ptr -> ver_ancestor, sci_ptr -> ver_merge );
      if (rename(temp1, temp_working_file) < 0) {
        ui_print ( VFATAL, "Unable to rename %s to %s\n", temp1, temp_working_file);
        (void) unlink(temp1);
        return( ERROR );
      }
      return( OK );
    }
  }
  (void) unlink(temp2);
  if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write", temp2);
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_user );
  status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_user, fd, sci_ptr -> leader);
  (void) close(fd);
  if (status !=  OK ) {
    ui_print ( VFATAL, "Check out failed.\n", sci_ptr -> ver_user );
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  if ( sci_ptr -> same13) {
    ui_print ( VDETAIL, "Common ancestor %s; will just use revision %s\n",
               sci_ptr -> ver_ancestor, sci_ptr -> ver_user );
    (void) unlink(temp1);
    if (rename(temp2, temp_working_file) < 0) {
      ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2, temp_working_file);
      (void) unlink(temp2);
      return( ERROR );
    }
    return( OK );
  }
  if ( sci_ptr -> called_getancestor ) {
    ui_print ( VALWAYS, "\n");
    ui_print ( VALWAYS, "*** WARNING -- calculated common ancestor ***\n");
    ui_print ( VALWAYS, "*** Check the merge differences carefully ***\n");
    ui_print ( VALWAYS, "\n");
  }
  (void) concat(mesgfile, sizeof(mesgfile), BCSTEMP, "/_MESG_", NULL);
  (void) unlink(mesgfile);
  if (extract_log_messages( sci_ptr, mesgfile, sci_ptr -> ver_merge,
                            sci_ptr -> ver_user) != 0) {
    ui_print ( VFATAL, "Failed to extract log messages\n");
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  (void) unlink(temp3);
  if ((fd = open(temp3, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp3);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Retrieving common ancestor %s\n",
             sci_ptr -> ver_ancestor );
  status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_ancestor,fd, sci_ptr -> leader);
  (void) close(fd);
  if ( status != OK ) {
    ui_print ( VFATAL, "Failed to check out %s\n", sci_ptr -> ver_ancestor );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Merging differences between %s and %s to %s\n",
             sci_ptr -> ver_merge, sci_ptr -> ver_user, temp_working_file);
  (void) unlink(temp4);
  if ((fd = open(temp4, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp4);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  pid = fd_runcmd("diff", BCSTEMP, TRUE, -1, fd,
                  "diff", temp1, temp3, NULL);
  (void) close(fd);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  status = endcmd(pid);
  if (status != 0 && status != 1) {
    ui_print ( VFATAL, "Diff command failed with status %d\n", status);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  (void) unlink(temp5);
  if ((fd = open(temp5, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp5);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  pid = fd_runcmd("diff", BCSTEMP, TRUE, -1, fd,
                  "diff", temp2, temp3, NULL);
  (void) close(fd);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  status = endcmd(pid);
  if (status != 0 && status != 1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  fd = open(temp_working_file, O_WRONLY|O_TRUNC|O_CREAT, 0600);
  if (fd < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp_working_file);
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  pid = fd_runcmd("rcsdiff3", BCSTEMP, TRUE, -1, fd,
                  "rcsdiff3", "-r", temp4, temp5, temp1, temp2, temp3,
                  sci_ptr -> ver_merge, sci_ptr -> ver_user, NULL);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of rcsdiff3 failed.\n" );
    (void) close(fd);
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  (void) close(fd);
  status = endcmd(pid);
  if (status == -1) {
    ui_print ( VFATAL, "rcsdiff3 failed\n");
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  if (status == 255) {
    ui_print ( VWARN, "Merge failed\n");
    (void) unlink(temp_working_file);
  } else if (status != 0)
    ui_print ( VALWAYS, "Warning: %d overlaps during merge.\n", status);
  if (status == 0)
    ui_print ( VALWAYS, "Merge successful.\n");
  (void) unlink(temp1);
  (void) unlink(temp2);
  (void) unlink(temp3);
  (void) unlink(temp4);
  (void) unlink(temp5);
  if (access(mesgfile, R_OK) == 0 && reinsert_log_messages(mesgfile) != OK) {
    ui_print ( VFATAL, "Failed to extract log messages\n");
    (void) unlink(mesgfile);
    (void) unlink(temp_working_file);
    return( ERROR );
  }
  ui_print ( VDEBUG, "Leaving src_ctl_merge\n" );
  return(status ? ERROR : 1);
} /* end src_ctl_merge */


STATIC
do_pause ( a )
int a;
{
  char answer [10];

  ui_print ( VALWAYS, "pause: %d\n", a );
  gets (answer);
}

STATIC
get_str(prompt, deflt, buf)
char *prompt, *deflt, *buf;
{
  (void) getstr(prompt, deflt, buf);
} /* get_str */


/*
 * function to check for permissible copyright marker
 */
STATIC
int legal_copyright (str)
char *str;
{
  struct copy_elem *copy_ptr;

  copy_ptr = copy_list;
  while ( copy_ptr != NULL) {
    ui_print ( VDEBUG, "checking: %s\n", copy_ptr->cr_name );
    /* go through list of valid markers checking for match */
    if ( sindex (str, copy_ptr->cr_name) != NULL ) {
      ui_print ( VDEBUG, "Found legal copyright marker: %s\n", copy_ptr->cr_name
 );
      return (TRUE);
    } /* if */
    copy_ptr = copy_ptr->cr_next;
  }
  ui_print ( VDEBUG, "Couldn't find matching copyright marker for: %s", str );
  return (FALSE);
} /* end legal_copyright */



/*
 * get a line from the file pointed to by fp
 * (or reuse the line previously read)
 * side-effect: increments global buf_lines
 */
STATIC
char *
getline(buf, bufsiz, fp, sameline)
char *buf;
int bufsiz;
FILE *fp;
int sameline;
{
  enter ( "getline" );
  if (!sameline) {
    if (fgets(buf, bufsiz, fp) == NULL) {
      ui_print(VDEBUG, "reached EOF reading input file\n");
      leave ( );
      return NULL;        /* EOF or error reading */
    }
    ++buf_lines;
    ui_print(VDEBUG, "read '%s'", buf);
  }
  ui_print(VCONT, "buf_lines = %d\n", buf_lines);
  leave ( );
  return buf;
} /* getline */


/*
 * Search for a particular string and optionally check for good leaders on
 * each line as you go.
 *
 * modes: 1 - require good leader
 *        2 - good leader not required
 *        3 - only check this line
 *
 * returns: OK - if string is found
 *          MISSING - if not found
 *          EOF - on end of file
 */
STATIC
int search_for(buf, size, inf, search, mode, leader, lead_len, good_leader, ptr, reuse)
  char  * buf;          /* buffer for getline */
  int     size;         /* length of buffer */
  FILE  * inf;          /* file ptr from which to read */
  char  * search;       /* string to look for */
  int     mode;                   /* flag: good leader required on each line */
  char  * leader;       /* comment leader */
  int     lead_len;     /* length of comment leader */
  int   * good_leader;                 /* does the string have a good leader */

  char ** ptr;          /* pointer returned by sindex() */
  int     reuse;        /* use existing line already in buffer */
{
  char * res;

  enter ( "search for" );
  ui_print ( VDEBUG, "Mode: %d\n", mode );
  *good_leader = FALSE;
  switch(mode) {
  case 1:
   /*
    *  Good leader required
    */
    while ((res = getline(buf, size, inf, reuse)) != NULL) {
      reuse = FALSE;
      *good_leader = (strncmp(buf, leader, lead_len) == 0);
      if (*good_leader) {
        if ((*ptr = sindex(buf, search)) != NULL) {
          leave ( );
          return (OK);  /* found match */
        } /* endif */
      } else {
        leave ( );
        return (MISSING);
      } /* endif */
    } /* endwhile */
    break;

   /*
    *  Good leader not required
    */
  case 2:
    while ((res = getline(buf, size, inf, reuse)) != NULL) {
      reuse = FALSE;
      if ((*ptr = sindex(buf, search)) != NULL) {
        *good_leader = (strncmp(buf, leader, lead_len) == 0);
        leave ( );
        return (OK);    /* strings matched */
      } /* endif */
    }/* endwhile */
    break;
   /*
    * Look on immediate line only
    */
  case 3:
    if ((res = getline(buf, size, inf, reuse)) != NULL) {
      reuse = FALSE;
      *good_leader = (strncmp(buf, leader, lead_len) == 0);
      if ((*ptr = sindex(buf, search)) != NULL) {
        leave ( );
        return (OK);    /* found match */
      } else {
        leave ( );
        return (MISSING);
      } /* if */
    } else {
      leave ( );
      return (EOF);
    } /* endif */

  } /* endswitch */

  if (res == NULL) {
    leave ( );
    return (EOF);
  } else {
    leave ( );
    return (MISSING);
  } /* endif */

} /* end search_for */


/*
 * returns: OK - if everything is found as expected (or no log)
 *          ERROR - on error
 */
STATIC
checklogstate(leader, improper )
char *leader;   /* comment leader (if any) */
BOOLEAN * improper; /* bad copyright, leader line, etc. */
{
  char buf[MAXPATHLEN];
  char ldr[MAXPATHLEN];		/* comment leader without trailing whitespace */
  char *ptr, *ptr2;
  FILE *inf;
  int lead_len;			/* length of leader (for small optimization) */
  int good_leader;		/* flag indicating good leader */
  int result = 0;
  int reuse;			/* read new line into input buffer */
  register int i;

  buf_lines = 0;
  reuse = FALSE;
  *improper = FALSE;
 /*
  * If there is no leader, there is no point in continuing.
  */
  if ( leader == NULL || *leader == NUL || strcmp ( leader, "NONE") == 0 || strcmp ( leader, "BIN" ) == 0 )
    return ( OK );
  /* endif */

  strcpy(ldr, leader);
  for (i = strlen(leader)-1; i >= 0; i--)
      if (isspace(ldr[i]))
	  ldr[i] = '\0';	/* eliminate any trailing whitespace */
      else
	  break;
  lead_len = strlen(ldr);

 /*
  *  Open a temporary copy of the file to check for correct log info.
  */
  if ((inf = fopen(temp_working_file, "r")) == NULL) {
    ui_print ( VFATAL, "fopen %s for reading\n", temp_working_file);
    return ( ERROR );
  } /* endif */

  if (strcmp(leader, CONTCMT) == 0)     /* is this a C source file? */
    do {
      result = search_for(buf, sizeof(buf), inf, BEGINCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, reuse);
      reuse = FALSE;
      ui_print(VDEBUG, "search_for (1) '%s' returned %d, line %d\n",
               BEGINCMT, result, buf_lines);
      if (result == EOF)
        break;
      else if (result == MISSING)
        continue;
      /* if */
      /* otherwise found start of comment */
      result = search_for(buf, sizeof(buf), inf, CR, 3, ldr,
                          lead_len, &good_leader, &ptr, 0);
      ui_print(VDEBUG, "search_for (2) '%s' returned %d, line %d\n",
               CR, result, buf_lines);
      if (result == OK && good_leader ) {
        if ( ! legal_copyright(buf) ) {
          continue;
        }/* endif */
      } /* if */
      if (result == EOF)
        break;
      else if (result == MISSING || ! good_leader ) {
        reuse = TRUE;      /* start scanning again from this line */
        continue;
      } /* if */
      result = search_for(buf, sizeof(buf), inf, ENDCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, 0);
      ui_print(VDEBUG, "search_for (3) '%s' returned %d, line %d\n",
               ENDCMT, result, buf_lines);
      if (result != MISSING)
        break;
      /* if */
      reuse = TRUE;        /* repeat scan starting with this line */
    } while (result != EOF);    /* end do */
  else {
    do {
      result = search_for(buf, sizeof(buf), inf, CR, 1, ldr,
                          lead_len, &good_leader, &ptr, 0);
      ui_print(VDEBUG, "search_for '%s' returned %d, line %d\n",
               CR, result, buf_lines);
      if ( result == OK )
        if ( legal_copyright ( buf ) )
          break;
        /* if */
      /* if */
    } while (result != EOF);    /* end do */
  } /* if */
  if ( result != OK ) {
    ui_print ( VWARN, "Missing valid copyright marker in %s\n",
         working_file_tail);
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
      (void) fclose(inf);
      return ( ERROR );
    } /* if */

    return ( OK );
  } /* endif */

  reuse = FALSE;
  if (strcmp(leader, CONTCMT) == 0)     /* is this a C source file? */
    do {
      result = search_for(buf, sizeof(buf), inf, BEGINCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, reuse);
      reuse = FALSE;
      if (result == EOF)
        break;
      else if (result == MISSING)
        continue;
      /* endif */
      result = search_for(buf, sizeof(buf), inf, HIST, 3, ldr, lead_len,
                          &good_leader, &ptr, 0);
      if (result != MISSING)
        break;
      reuse = TRUE;    /* couldn't find HISTORY, repeat scan from this line */
      /* endif */
    } while (result != EOF);
  else
    result = search_for(buf, sizeof(buf), inf, HIST, 2, ldr, lead_len,
                        &good_leader, &ptr, 0);
  /* if */
  if (result != OK || ! good_leader ) {
    if (!good_leader) {
      ui_print ( VWARN, "Improper leader line between copyright marker and %s marker in %s\n", HIST, working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } else
      ui_print ( VALWAYS, "Missing %s marker in %s\n", HIST,
                          working_file_tail);
    /* if */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
      (void) fclose(inf);
      return ( ERROR );
    } /* if */

    return ( OK );
  } /* endif */

  result = search_for(buf, sizeof(buf), inf, BEGINLOG, 1, ldr, lead_len,
                      &good_leader, &ptr, 0);
  if (result != OK) {
    if (result == EOF)
      ui_print ( VALWAYS, "Missing %s...$ marker in %s\n", BEGINLOG,
                          working_file_tail);
    else {
      ui_print ( VWARN, "Improper leader line between %s marker and %s marker.\n" );
      ui_print ( VCONT, "File: '%s'\n", HIST, BEGINLOG, working_file_tail );
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } /* endif */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
      (void) fclose(inf);
      return ( ERROR );
    } /* if */

    return ( OK );
  } else {
    ptr2 = ptr+4;
    if (*ptr2 != '$' && (*ptr2 != ':' || sindex(ptr2, "$") < ptr2)) {
      ui_print ( VALWAYS, "Bad %s...$ marker in %s", BEGINLOG,
                          working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
      *improper = TRUE;

     /*
      * Check for file errors.
      */
      if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
        (void) fclose(inf);
        return ( ERROR );
      } /* if */

      return ( OK );
    } /* endif */
  } /* endif */

  result = search_for(buf, sizeof(buf), inf, ENDLOG, 1, ldr, lead_len,
                      &good_leader, &ptr, 0);
  if (result != OK) {
    if (result == EOF) {
      ui_print ( VALWAYS, "Missing %s marker in %s\n", ENDLOG,
                          working_file_tail);
    } else if (!good_leader) {
      ui_print ( VWARN, "Improper leader line between %s marker and %s marker in %s\n", BEGINLOG, ENDLOG, working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } /* if */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
      (void) fclose(inf);
      return ( ERROR );
    } /* if */

    return ( OK );
  } /* if */
  if (strcmp(leader, CONTCMT) == 0) {
    result = search_for(buf, sizeof(buf), inf, ENDCMT, 3, ldr, lead_len,
                        &good_leader, &ptr, 0);
    if (result != OK) {
      ui_print ( VALWAYS, "Missing comment end after %s marker.\n", ENDLOG,
            working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
      *improper = TRUE;

     /*
      * Check for file errors.
      */
      if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
        (void) fclose(inf);
        return ( ERROR );
      } /* if */

      return ( OK );
    } /* endif */
  } /* endif */

 /*
  * Check for file errors.
  */
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_print ( VFATAL, "Error reading %s\n", temp_working_file);
    (void) fclose(inf);
    return ( ERROR );
  } /* if */

  return ( OK );
} /* end checklogstate */


STATIC
int get_bmerge_action( sci_ptr, initial_key, no_log )
SCI_ELEM sci_ptr;
int initial_key;
BOOLEAN no_log;
{
  char *def;
  int key;
  struct stat st;
  int pid;
  int status;
  int improper = FALSE;
  char leader [ MAXPATHLEN ];
  char prompt[MAXPATHLEN];
  char deflt[MAXPATHLEN];

  key = initial_key;
  for (;;) {
    switch (key) {
    case MA_ABORT:
      (void) unlink(temp_working_file);
      return ( ABORT );
    case MA_OK:
      if (stat(temp_working_file, &st) < 0) {
        def = "merge";
        break;
      }
      if ( ! no_log )
        if ( (status = checklogstate ( sci_ptr -> leader, &improper )) != OK)
          return ( status );
        else if ( improper ) {
          ui_print ( VDEBUG, "Improper was true.\n" );
          def = "edit";
          break;
        } /* if */
      /* if */
      return( OK );
    case MA_EDIT:
            if (stat(temp_working_file, &st) < 0) {
              break;
            }
            if ( ui_ver_level() >= VDEBUG )
                echo(EDIT_PROG, temp_working_file, NULL);
            (void) runp(EDIT_PROG, EDIT_PROG, temp_working_file, NULL);
            def = "rdiff";
            break;
        case MA_MERGE:
            (void) unlink(temp_working_file);
            status = src_ctl_merge( sci_ptr );
            if (status == 0) {
                def = "ok";
                break;
            }
            if (stat(temp_working_file, &st) < 0)
              def = "merge";
            else if (status == 1)
              def = "rdiff";
            else
              def = "edit";
            break;
        case MA_RCO:
            (void) unlink(temp_working_file);
            if ( src_ctl_check_out( sci_ptr -> name, sci_ptr -> ver_user, sci_ptr -> leader )
                 == 0) {
                if (stat(temp_working_file, &st) < 0) {
                  ui_print ( VFATAL, "stat %s", temp_working_file);
                  return ( ERROR );
                } else if (chmod(temp_working_file,
                               (int)(st.st_mode|S_IWRITE)&0777) < 0) {
                  ui_print ( VFATAL, "chmod %s", temp_working_file);
                  return ( ERROR );
                }
                def = "diff";
            }
            break;
        case MA_CO:
            (void) unlink(temp_working_file);
            if ( src_ctl_check_out( sci_ptr -> name, sci_ptr -> ver_merge, sci_ptr -> leader )
                 == 0) {

                if (stat(temp_working_file, &st) < 0) {
                  ui_print ( VFATAL, "stat %s", temp_working_file);
                  return ( ERROR );
                } else if (chmod(temp_working_file,
                               (int)(st.st_mode|S_IWRITE)&0777) < 0) {
                  ui_print ( VFATAL, "chmod %s", temp_working_file);
                }
                def = "ok";
            }
            break;
        case MA_RDIFF:
          if (stat(temp_working_file, &st) < 0) {
            def = "merge";
            break;
          }
          if (src_ctl_diff_rev_with_file( sci_ptr -> ver_user,
                                          temp_working_file, sci_ptr -> name,
                                          -1) != 0)
            break;
          def = "diff";
          break;
        case MA_DIFF:
          if (stat(temp_working_file, &st) < 0) {
            def = "merge";
            break;
          }
          if (src_ctl_diff_rev_with_file( sci_ptr -> ver_merge,
                                          temp_working_file, sci_ptr -> name,
                                          -1) != 0)
              break;
          def = "ok";
          break;
        case MA_LEADER:
            strcpy ( leader, sci_ptr -> leader );
            (void) concat(prompt, sizeof(prompt),
                          "Comment leader for ", sci_ptr -> name, NULL);
            (void) strcpy(deflt, ( *leader == NUL) ? "NONE" : leader);
            for (;;) {
                get_str(prompt, deflt, leader);
                if (strcmp(leader, deflt) == 0)
                    break;
                (void) strcpy(deflt, leader);
            }
            sci_ptr -> leader = salloc ( leader );
            src_ctl_set_leader ( sci_ptr -> name, sci_ptr -> leader );
        case MA_HELP:
          ui_print ( VALWAYS, "One of the following:\n\n");
          ui_print ( VALWAYS, " abort - abort merge for %s\n", canon_working_file);
          ui_print ( VALWAYS, " ok    - done with merged %s; do next file\n",
                 canon_working_file);
          ui_print ( VALWAYS, " edit  - edit merged %s\n", canon_working_file);
          ui_print ( VALWAYS, " merge - merge source version of set '%s' into %s\n",
                 BCSSET_NAME, canon_working_file);
          ui_print ( VALWAYS, " co    - check-out %s from destination branch alone\n",
                 canon_working_file);
          ui_print ( VALWAYS, " rco   - check-out %s from source configuration alone\n",
                 canon_working_file);
          ui_print ( VALWAYS, " diff  - compare merged %s with destination branch\n",
                 canon_working_file);
          ui_print ( VALWAYS, " rdiff - compare merged %s with source configuration\n",
                 canon_working_file);
          ui_print ( VALWAYS, " leader - set the comment leader\n" );
          ui_print ( VALWAYS, "\n");

        }
/*
 * This is rather messy, but it works.
 * Test if file is binary, if it isn't, allow the merge.
 * If no real merge is required, key is set to MA_OK
 * and the MA_OK action is taken.
 */
        if ( (sci_ptr -> need_merge || improper) && (strcmp(sci_ptr -> leader, "BIN") != 0) )
          key = get_stab("Abort, ok, edit, merge, [r]co, [r]diff",
                         bmerge_action, def);
        else
          key = MA_OK;
    }
} /* end get_bmerge_action */


STATIC
merge_elem ( sci_ptr, no_log )
SCI_ELEM sci_ptr;
BOOLEAN no_log;
{
  int fd;
  int status = OK;
  int bmerge_status;
  int improper;

  ui_print ( VDEBUG, "Entering merge_elem\n" );
  check_path ( sci_ptr -> name );
/*
  (void) unlink(temp2);
*/
/*
  if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "unable to open %s for write\n", temp2);
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_user );
  status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_user, fd, sci_ptr -> leader);
  (void) close(fd);
  if (status != OK ) {
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  if (rename(temp2, temp_working_file) < 0) {
    ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2, temp_working_file);
    (void) unlink(temp2);
    return( ERROR );
  }
*/
  bmerge_status = get_bmerge_action ( sci_ptr, MA_MERGE, no_log );
  if ( bmerge_status != OK )
    status = bmerge_status;
  /* end if */
  ui_print ( VDEBUG, "Leaving merge_elem\n" );
  return ( status );
} /* end merge_elem */


STATIC
int src_ctl_create_branch( rcs_file, rev, set_name )
char * rcs_file;
char * rev;
char * set_name;
{
  char buffer[MAXPATHLEN];
  int pid;
  int i;
  char *av[16];
  char *symbol;
  char comma_v_file [MAXPATHLEN];
  char * ibr_str;
  char tempfile[MAXPATHLEN];
  int infd;
  int outfd;
  char * bufptr;
  int status;

  if (rev == NULL)
    rev = "1.1";
  /* end if */
  if ((rev = alloc_switch('r', rev)) == NULL)
    return ( ERROR );
  /* end if */
  if ((symbol = alloc_switch('N', set_name)) == NULL)
    return ( ERROR );
  /* end if */

  if ((ibr_str = alloc_switch('m', IBR_MARKER )) == NULL)
    return ( ERROR );
  /* end if */

  concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
  i = 0;
  av[i++] = "authcover";
  av[i++] = "-t1";       /* -t1 indicates that the file shouldn't be removed */
  av[i++] = "3";
  av[i++] = "branch_ci";
  av[i++] = rev;
  av[i++] = symbol;
  av[i++] = working_file_tail;
  av[i++] = comma_v_file;
  av[i++] = NULL;
  pid = full_runcmdv(RCSCOVER, av, FALSE, rcs_child_init, BCSTEMP, -1, -1);
  if (pid == -1)
    return( ERROR );


  status = endcmd(pid);
  return ( status );
} /* end src_ctl_create_branch */


/*
 * use rlog to obtain log messages
 * returns OK or ERROR
 */
int create_leaderless_log ( sci_ptr, user_set, logmsg, mesgfile )
SCI_ELEM sci_ptr;
char * user_set;
char * logmsg;
char * mesgfile;
{
  int p[2];
  int pid;
  char * rev;
  char * av [6];
  char buffer[MAXPATHLEN];
  char stat_line[MAXPATHLEN];
  char * ptr;
  FILE *inf;
  int test;
  char tmp_logmsg [LOGMSGSIZE];
  FILE *mesgf;
  BOOLEAN found_desc_line;
  int status = OK;

  enter ( "create_leaderless_log" );
  if ( ( rev = alloc_switch('r', user_set ) ) == NULL ) {
    ui_print ( VFATAL, "Revision alloc switch failed ");
    leave ( );
    return ( ERROR );
  } /* end if */
  av[0] = "authcover";
  av[1] = "rlog";
  av[2] = rev;
  av[4] = NULL;
  if (pipe(p) < 0) {
    ui_print ( VWARN, "Pipe failed" );
    leave ( );
    return ( ERROR );
  } /* end if */
  av[3] = sci_ptr -> name;
  pid = full_runcmdv(RCSCOVER, av, FALSE,
                     rcs_child_init, (char *)NULL, p[0], p[1] );
  if (pid == -1) {
    leave ( );
    return ( ERROR );
  } /* end if */
  (void) close(p[1]);
  if ((inf = fdopen(p[0], "r")) == NULL) {
    ui_print ( VWARN, "fdopen failed" );
    (void) endcmd(pid);
    leave ( );
    return ( ERROR );
  }
  if ((mesgf = fopen(mesgfile, "w")) == NULL) {
    ui_print ( VFATAL, "fopen %s\n", mesgfile);
    (void) unlink(mesgfile);
    leave ( );
    return( ERROR );
  }
  found_desc_line = FALSE;
  while ( fgets(buffer, sizeof(buffer), inf) != NULL) {
    if ( strncmp ( "description", buffer, 11 ) == 0 ) {
      found_desc_line = TRUE;
      break;
    } /* if */
  } /* while */
  if ( found_desc_line ) {
    /*
       '-----' line
    */
    if ( fgets(buffer, sizeof(buffer), inf) == NULL) {
      ui_print ( VFATAL, "Unexpected EOF 1\n" );
      status = ERROR;
    } else {
      do {
        /*
           revision line
        */
        if ( fgets(buffer, sizeof(buffer), inf) != NULL) {
          if ( buffer [ strlen ( buffer ) -2 ] == '1' )
            break;
          /* if */
        } else {
          ui_print ( VFATAL, "Unexpected EOF 2\n" );
          status = ERROR;
          break;
        }
        /*
           info line
        */
        if ( fgets(buffer, sizeof(buffer), inf) != NULL) {
        } else {
          ui_print ( VFATAL, "Unexpected EOF 3\n" );
          status = ERROR;
          break;
        } /* if */
        /*
           Actual log information
        */
        do {
          if ( fgets(buffer, sizeof(buffer), inf) != NULL) {
            if ( buffer [ 0 ] != '\t' ) {
              break;
            } /* if */
          } else {
            ui_print ( VFATAL, "Unexpected EOF 4\n" );
            status = ERROR;
            break;
          } /* if */
          fprintf ( mesgf, "%s", buffer );
          concat ( tmp_logmsg, sizeof ( tmp_logmsg ), logmsg, buffer, NULL );
          strcpy ( logmsg, tmp_logmsg );
        } while ( TRUE );
      } while ( TRUE );
    } /* if */
  } else {
    ui_print ( VFATAL, "Unable to find log information !?\n" );
    status = ERROR;
  } /* if */
  if ( ferror ( inf ) || fclose ( inf ) == EOF ) {
    (void) fclose(mesgf);
    ui_print ( VWARN, "Error reading pipe");
    (void) endcmd(pid);
    leave ( );
    return ( ERROR );
  } /* end if */
  (void) fclose(mesgf);
  (void) endcmd(pid);
  leave ( );
  return ( status );
} /* end create_leaderless_log */


STATIC
int setup_bcstemp()
{
  struct stat st;
  int changes = FALSE;

  if (stat(BCSTEMP, &st) < 0 || (st.st_mode&S_IFMT) != S_IFDIR) {
    ui_print ( VDETAIL, "Creating %s\n", BCSTEMP);
    (void) unlink(BCSTEMP);
    if (makedir(BCSTEMP) != 0) {
      ui_print ( VFATAL, "Unable to create %s directory\n", BCSTEMP);
      return ( ERROR );
    } /* end if */
    if (stat(BCSTEMP, &st) < 0) {
      ui_print ("Unable to stat %s directory\n", BCSTEMP);
      return ( ERROR );
    } /* end if */
    changes = TRUE;
  }
  if (changes) {
    if ( ui_ver_level() >= VDETAIL ) {
      ui_print ( VDEBUG, "ls -lgd %s\n", BCSTEMP );
      (void) runp("ls", "ls", "-lgd", BCSTEMP, NULL);
    } /* end if */
  } /* end if */
  return ( OK );
} /* end setup_bcstemp */


/*
 * canonicalize path - similar to abspath
 */
STATIC
canonicalize(base, relpath, outbuf, outbuf_size)
char *base;
char *relpath;
char *outbuf;
int outbuf_size;
{
    char *from;
    char *to;
    char *slash;
    char *peek;

    /*
     * concatenate parts of path into buffer
     */
    if (concat(outbuf, outbuf_size, base, "/", relpath, "/", NULL) == NULL) {
      ui_print ( VFATAL, "Path length exceeds buffer size\n" );
      return ( ERROR );
    } /* end if */

    /*
     * remember position of first slash
     */

    slash = index(outbuf, '/');
    from = to = slash + 1;

    /*
     * canonicalize the path
     */
    while (*from != '\0') {
        if ((*to++ = *from++) != '/')
            continue;
        peek = to-2;
        if (*peek == '/') {
            /*
             * found "//", back up one
             */
            to--;
            continue;
        }
        if (*peek != '.')
            continue;
        peek--;
        if (*peek == '/') {
            /*
             * found "/./", back up two
             */
            to -= 2;
            continue;
        }
        if (*peek != '.')
            continue;
        peek--;
        if (*peek != '/')
            continue;
        /*
         * found "/../", try to remove preceding token
         */
        if (peek == slash) {
            /*
             * hit the "first" slash, update to not remove any more tokens
             */
            slash = to-1;
            continue;
        }
        /*
         * backup one token
         */
        while (*--peek != '/')
            ;
        to = peek+1;
    }
    *to-- = '\0';
    if (to > outbuf && *to == '/')
        *to = '\0';
  return ( OK );
} /* end canonicalize */


/*
 * read copyright markers from file and add to copy_list
 * returns: ERROR - on error
 *          OK - normally
 */
STATIC
int copyrights_from_file (copyrights_rc)
  char copyrights_rc[MAXPATHLEN];
{
  char buf[MAXPATHLEN];
  FILE *copy_f;
  int has_entries;
  struct copy_elem *copy_ptr;
  int lines;

  ui_print ( VDEBUG, "Looking for copyright markers in file: '%s'.\n",
             copyrights_rc );
  lines = 0;
 /*
  * Read in list of legal copyright markers.
  */
  if ( ( copy_f = fopen(copyrights_rc, "r" ) ) == NULL ) {
    ui_print ( VWARN, "Couldn't open copyrights file for reading (missing?).\n");
    ui_print ( VCONT, "This file was specified in the rc file variable '%s'.\n",CR_VAR);
    ui_print ( VCONT, "It should exist and contain the list of legal copyright markers. \n" );
    ui_print ( VCONT, "File: '%s'.\n", copyrights_rc );
    return (ERROR);
  }
  has_entries = FALSE;
  while ( fgets ( buf, sizeof ( buf ), copy_f ) != NULL ) {
    char tmpbuf[MAXPATHLEN];

    lines += 1;
    if (buf[0] == '#')  /* comment to end of line */
    {
      ui_print ( VDEBUG, "Skipping comment line %d\n", lines );
      continue;
    }
    ui_print ( VDEBUG, "Line %d: %s\n", lines, buf );
    copy_ptr = (struct copy_elem *)malloc( sizeof ( struct copy_elem ) );
    has_entries = TRUE;
    if ((strlen(buf) > 0) && (buf[strlen(buf)-1] == '\n') ) {
      buf[strlen(buf)-1] = NUL;
    }
    if ( ( buf[0] == NUL) || ( buf[0] == '\n') ) {
      ui_print ( VWARN, "Empty copyright marker in copyright file.\n" );
      ui_print ( VCONT, "Line: %d.\n", lines );
      ui_print ( VCONT, "File: '%s'.\n", copyrights_rc );
      return (ERROR);
    }
    strcpy(tmpbuf, buf);
    if ( sindex ( buf, CR ) == NULL) {
      ui_print ( VWARN, "All copyright markers must contain the string '%s'.\n",
 CR);
      ui_print ( VCONT, "The marker '%s' does not. \n", buf );
      ui_print ( VCONT, "Line: %d.\n", lines );
      ui_print ( VCONT, "File: '%s'.\n", copyrights_rc );
      return (ERROR);
    } /* while */
    concat(buf, sizeof(buf), "@", tmpbuf, "@", NULL);
    copy_ptr->cr_name = strdup(buf);
    copy_ptr->cr_next = copy_list;
    copy_list = copy_ptr;
  }
  if ( ! has_entries) {
    ui_print ( VWARN, "There are no entries in the copyright file.\n");
    ui_print ( VCONT, "File: '%s'.\n", copyrights_rc );
    return (ERROR);
  } /* if */
  return (OK);
} /* end copyrights_from_file */


/*
 * parse string for valid copyright markers and add to copy_list
 * returns: OK - string is parsed successfully
 *          ERROR - otherwise
 */
STATIC
int copyrights_from_string( copyrights_str )
  char *copyrights_str;
{
  char buf[MAXPATHLEN];
  int entries;
  int has_entries;
  struct copy_elem *copy_ptr;
  char * str_ptr;
  char * next_str_ptr;

  ui_print ( VDEBUG, "Looking for copyright markers in string:\n" );
  ui_print ( VCONT, "%s \n", copyrights_str );
  has_entries = FALSE;
  entries = 0;
  while ( *(str_ptr = nxtarg ( &copyrights_str, "; " ) ) != NUL ) {
    char tmpbuf[MAXPATHLEN];


    if ( strcmp ( str_ptr, "include" ) == 0 ) {
      str_ptr = nxtarg ( &copyrights_str, ";" );
      if ( str_ptr == NUL )
        return (ERROR);
      else if ( copyrights_from_file ( str_ptr ) == ERROR )
        return (ERROR);
      /* end if */
      has_entries = TRUE;
      continue;
    } /* end if */
    entries += 1;
    ui_print ( VDEBUG, "Entry %d: %s\n", entries, str_ptr );
    copy_ptr = (struct copy_elem *)malloc( sizeof ( struct copy_elem ) );
    has_entries = TRUE;
    strcpy(tmpbuf, str_ptr);
    if ( sindex (str_ptr, CR ) == NULL) {
      ui_print ( VWARN, "All copyright markers must contain the string '%s'.\n",
 CR );
      ui_print ( VCONT, "The marker '%s' does not. \n", str_ptr );
      ui_print ( VCONT, "Entry: %d.\n", entries );
      return (ERROR);
    }
    concat(buf, sizeof(buf), "@", tmpbuf, "@", NULL);
    copy_ptr->cr_name = strdup(buf);
    copy_ptr->cr_next = copy_list;
    copy_list = copy_ptr;
  } /* end while */
  if ( ! has_entries) {
    ui_print ( VWARN, "There are no entries in the copyright string provided.\n"
);
    return (ERROR);
  } else
    return (OK);
  /* end if */
} /* end copyrights_from_string */



STATIC
int read_legal_copyrights ( contents )
struct rcfile * contents;
{
  char *copyrights_rc;
  int status;

  copy_list = NULL;
  ui_print ( VDEBUG, "Composing list of legal copyright markers.\n" );

  if ( get_rc_value (CR_VAR, &copyrights_rc, contents, FALSE ) == OK )
  {
    ui_print ( VDEBUG, "copyright rc variable: (starts on next line)\n" );
    ui_print ( VCONT, "%s \n", copyrights_rc ) ;
    status = copyrights_from_string( copyrights_rc );
  } else {
    ui_print ( VDEBUG, "Using default copyright markers.\n" );
    copyright_markers = salloc ( COPYRIGHT_MARKERS );
    status = copyrights_from_string( copyright_markers );
  } /* if */
  return ( status );
} /* end read_legal_copyrights */

STATIC
src_ctl_set_leader( rcs_file, leader)
char * rcs_file;
char * leader;
{
  char buf[MAXPATHLEN];
  int pid;
  int i;
  char *av[16];

  ui_print ( VDEBUG, "Setting leader to '%s'\n", leader);

  (void) concat(buf, sizeof(buf), "-c", leader, NULL);
  i = 0;
  av[i++] = "authcover";
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = buf;
  if (strcmp(leader, "BIN") == 0)
    av[i++] = "-ko";
  av[i++] = rcs_file;
  av[i++] = NULL;
  pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, (char *)NULL, -1, -1);
  if (pid == -1)
    return( ERROR );
  return(endcmd(pid));
} /* src_ctl_set_leader */

STATIC
src_ctl_set_remove( sci_ptr )
SCI_ELEM sci_ptr;
{
  char buf[MAXPATHLEN];
  char buffer[MAXPATHLEN];
  char cwbuf[MAXPATHLEN];
  FILE *inf, *outf;
  int wcnt;

  ui_print ( VDEBUG, "[ updating ./.BCSset-%s ]\n", BCSSET_NAME);
  (void) concat(buf, sizeof(buf), bcsset, ".tmp", NULL);
  (void) unlink(buf);
  if ((outf = fopen(buf, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", buf);
    return( ERROR );
  }
  (void) concat(cwbuf, sizeof(cwbuf), sci_ptr -> name, "\n", NULL);
  wcnt = 0;
  if ((inf = fopen(bcsset, "r")) != NULL) {
    while (fgets(buffer, sizeof(buffer), inf) != NULL) {
      if (strcmp(buffer, cwbuf) == 0) {
        if ( ui_ver_level () >= VQUIET )
          fprintf(stderr, "< %s", cwbuf);
        continue;
      }
      (void) fputs(buffer, outf);
      wcnt++;
    }
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", bcsset);
      (void) fclose(outf);
      (void) unlink(buf);
      return( ERROR );
    }
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", buf);
    (void) unlink(buf);
    return( ERROR );
  }
  if (rename(buf, bcsset) < 0) {
    ui_print ( VFATAL, "Rename %s to %s failed\n", buf, bcsset);
    return ( ERROR );
  }
  if (wcnt == 0)
    set_cleanup();
  return( OK );
}

STATIC
set_cleanup()
{
  struct stat st;

  if (stat(bcsset, &st) == 0 && st.st_size > 0)
    return;
  if (unlink(bcsset) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSset-%s\n", BCSSET_NAME);
  if (unlink(bcslog) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSlog-%s\n", BCSSET_NAME);
  if (unlink(bcspath) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSpath-%s\n", BCSSET_NAME);
}

/*
 * END OF SUPPORTING FUNCTIONS
 */

/*
 * FUNCTION sci_set_source_info
 * FIXME
 * sci_set_source_info is a pseudo-kludge. It is only necessary because
 * of the way src_ctl_check_env determines how to set SRCCOVER. This will
 * be done differently in the future. source_* should not be in the
 * rc_file/local in the backing build!!!.
 */
int sci_set_source_info ( usr_rcfile, sb )
char * usr_rcfile;
char * sb;
{
  char *basedir = NULL;
  char *sb_rcfile = NULL;
  struct rcfile rcfile;

  if (current_sb(&sb, &basedir, &sb_rcfile, &usr_rcfile) != 0) {
    ui_print ( VFATAL, "Unable to parse home sandbox rc\n");
    return ( ERROR ); /* unable to continue */
  } /* end if */
  bzero ( &rcfile, sizeof ( rcfile ) );
  if (parse_rc_file(sb_rcfile, &rcfile) != 0) {
    ui_print ( VFATAL, "Unable to parse sandbox");
    return ( ERROR ); /* unable to continue */
  } /* end if */
  get_rc_value ( "source_cover", &rcfile_source_cover, &rcfile, TRUE );
  get_rc_value ( "source_host", &rcfile_source_host, &rcfile, TRUE );
  get_rc_value ( "source_owner", &rcfile_source_owner, &rcfile, TRUE );
  get_rc_value ( "source_testdir", &rcfile_source_testdir, &rcfile, TRUE );
  SRCCOVER = rcfile_source_cover;
  return ( OK );
} /* end sci_set_source_info */

/*
 * FUNCTION sci_init
 *
 * This function needs to be called at the beginning of
 * a source control sesion, before any other sci_* calls are
 * made, to get setup information such as which server to use, etc.
 */
int sci_init ( contents, set_name )
struct rcfile * contents;
char * set_name;
{
  char temp[MAXPATHLEN];
  struct field *field_p;
  struct arg_list *args_p;
  int i;
  char buf[MAXPATHLEN];
  char *ptr;
  char *tmpdir;
  char *setname = NULL;
  int status;
  int status_tmp;

  ui_print ( VDEBUG, "Entering sci_init\n" );

  sig_block_set = &sig_block_set_base;
  sigemptyset ( sig_block_set );
  sigaddset ( sig_block_set, SIGINT );

  if ((tmpdir = getenv("TMPDIR")) == NULL)
    tmpdir = DEF_TMPDIR;
  /* if */

  status = OK;
  if (getuid() != geteuid() || getgid() != getegid())  {
    ui_print ( VFATAL, "Branch rcs commands should not run setuid\n" );
    status = ERROR;
  } /* if */
  if ((USER = getenv("USER")) == NULL) {
    ui_print ( VFATAL, "USER not defined in environment\n" );
    status = ERROR;
  } /* if */
  if ((EDIT_PROG = getenv(EDITOR)) == NULL)
    EDIT_PROG = DEF_EDITOR;
  /* if */

  /*
   * check sandbox source directory
   */
  if (rc_file_field(contents, "source_base", &field_p) != 0) {
    ui_print ( VFATAL, "Source_base not defined");
    status = ERROR;
  } /* if */
  args_p = field_p->args;
  if (args_p->ntokens != 1) {
    ui_print ("Improper source_base");
    status = ERROR;
  } /* if */
  rcfile_source_base = args_p->tokens[0];
  if (*rcfile_source_base != '/') {
    ui_print ( VFATAL, "source_base is not an absolute path");
    return ( ERROR ); /* unable to continue */
  } /* if */
  /*
   * BCSBBASE - top of working tree for source control operations
   */

/*
 * FIXME
 * the following error messages are rather scanty!
 */
  BCSBBASE = rcfile_source_base;
  if (curdir != NULL) {
    if (*curdir == '/') {
	    if (chdir(BCSBBASE) < 0) {
	      ui_print ( VFATAL, "chdir %s \n", BCSBBASE);
              return( ERROR );
            }	/* if */
	    (void) concat(curdirbuf, sizeof(curdirbuf), ".", curdir, NULL);
	    curdir = curdirbuf;
	}	/* if */
	if (chdir(curdir) < 0) {
	      ui_print ( VFATAL, "chdir %s \n", BCSBBASE);
              return( ERROR );
            }	/* if */
    }	/* if */
    if ((curdir = getwd(curdirbuf)) == NULL) {
	      ui_print ( VFATAL, "getwd %s \n", BCSBBASE);
              return( ERROR );
    } /* if */
    ptr = curdir + strlen(curdir);
    *ptr++ = '/';
    *ptr = '\0';

    if (chdir(BCSBBASE) < 0) {
      ui_print ( VFATAL, "chdir %s \n", BCSBBASE);
      return( ERROR );
    } /* if */
    if (getwd(buf) == NULL) {
	      ui_print ( VFATAL, "getwd %s \n", BCSBBASE);
              return( ERROR );
    } /* if */
    BCSBBASE = salloc(buf);
    if (BCSBBASE == NULL) {
      ui_print ( VFATAL, "source_base salloc failed\n" );
      return ( ERROR );
    } /* if */
    if (access(BCSBBASE, R_OK) < 0) {
      ui_print ( VFATAL, "%s not accessable\n", BCSBBASE);
      return ( ERROR );
    } /* if */
    ptr = concat(buf, sizeof(buf), BCSBBASE, "/", NULL);
    if (bcmp(buf, curdir, ptr - buf) != 0) {
	if (currel.argc > 0 ) {
          ui_print ( VFATAL, "current directory not in %s", BCSBBASE);
          return ( ERROR );
        }	/* if */
      (void) strcpy(curdirbuf, buf);
    }	/* if */
    curdir += ptr - buf;
    while (*curdir == '/')
	curdir++;
    if (*curdir != '\0' && chdir(curdir) < 0) {
      ui_print ( VFATAL, "chdir %s \n", curdir);
      return ( ERROR );
    }	/* if */
    curdir--;

    /*
     * authentication cover information for source tree
     */
/*
 * FIXME
 * The following is the old way to get rc fields!
 */

  if (rc_file_field(contents, "source_owner", &field_p) != 0) {
    SRCCOVER = NULL;
  } else {
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
      status = ERROR;
      ui_print ( VFATAL, "improper source_owner\n");
    }
    rcfile_source_owner = args_p->tokens[0];
    if (rc_file_field(contents, "source_host", &field_p) != 0) {
      ui_print ( VFATAL, "source_host not defined\n");
      return ( ERROR );
    }
    args_p = field_p->args;
    if (args_p->ntokens != 1) {
     ui_print ( VFATAL, "improper source_host\n");
     status = ERROR;
    }	/* if */
    rcfile_source_host = args_p->tokens[0];
    if (rc_file_field(contents, "source_testdir", &field_p) != 0) {
      ui_print ( VFATAL, "source_testdir not defined");
      status = ERROR;
    }	/* if */
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    ui_print ( VFATAL, "improper source_testdir");
      return ( ERROR );
      }	/* if */
	rcfile_source_testdir = args_p->tokens[0];
	if (*rcfile_source_testdir != '/') {
	    ui_print ( VFATAL, "source_testdir is not an absolute path");
      return ( ERROR );
      }
	if (rc_file_field(contents, "source_cover", &field_p) != 0) {
	    ui_print ( VFATAL, "source_cover not defined");
      return ( ERROR );
      }
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    ui_print ( VFATAL, "improper source_cover");
      return ( ERROR );
      }
	rcfile_source_cover = args_p->tokens[0];
	if (*rcfile_source_cover != '/') {
	    ui_print ( VFATAL, "source_cover is not an absolute path");
      return ( ERROR );
      }
	if (access(rcfile_source_cover, X_OK) < 0) {
	    ui_print ( VFATAL, "source_cover %s not executable", rcfile_source_cover);
      return ( ERROR );
      }
	SRCCOVER = rcfile_source_cover;
  }

  /*
   * source control environment checks
   */
  if ( src_ctl_check_env(contents) == ERROR )
    return ( ERROR );
  /* end if */


  /*
   * BCSTEMP - temporary directory for bcs commands
   */
  BCSTEMP = getenv("BCSTEMP");
  if (BCSTEMP == NULL) {
    if (rc_file_field(contents, "bcstemp", &field_p) != 0) {
	    (void) concat(bcstempbuf, sizeof(bcstempbuf),
			  tmpdir, "/b-", USER, NULL);
	    BCSTEMP = bcstempbuf;
	} else {
	    args_p = field_p->args;
	    if (args_p->ntokens != 1) {
		ui_print ( VFATAL, "improper bcstemp");
      return ( ERROR );
      }
	    BCSTEMP = args_p->tokens[0];
	}
	if (setenv("BCSTEMP", BCSTEMP, TRUE) < 0) {
	    ui_print ( VFATAL, "BCSTEMP setenv failed");
      return ( ERROR );
      }
	BCSTEMP = getenv("BCSTEMP");
	if (BCSTEMP == NULL) {
	    ui_print ( VFATAL, "BCSTEMP not defined");
      return ( ERROR );
      }
    }
    status = setup_bcstemp ( );
    if ( status != OK )
      return ( status );
    /* end if */

    /*
     * BCSSET_NAME - set name to use for bcs commands
     */
    BCSSET_NAME = set_name;
    if (strcmp(BCSSET_NAME, "TRUNK") == 0) {
	usetrunk = TRUE;
	(void) strcpy(setrev, "-r");
    } else {
	if (BCSSET_NAME[0] >= '0' && BCSSET_NAME[0] <= '9') {
	    usetrunk = TRUE;
	} else {
	    usetrunk = FALSE;
	    if (strncmp(BCSSET_NAME, USER, strlen(USER)) != 0 &&
		(BCSSET_NAME[0] < 'A' || BCSSET_NAME[0] > 'Z')) {
		(void) concat(setrev, sizeof(setrev),
			      USER, "_", BCSSET_NAME, NULL);
		(void) strcpy(buf, setrev);
		BCSSET_NAME = buf;
	    }
	}
	(void) concat(setrev, sizeof(setrev), "-r", BCSSET_NAME, NULL);
    }
    if (setenv(BCSSET, BCSSET_NAME, TRUE) < 0) {
	ui_print ( VFATAL, "BCSSET setenv failed");
      return ( ERROR );
      }
    BCSSET_NAME = getenv(BCSSET);
    if (BCSSET_NAME == NULL) {
	ui_print ( VFATAL, "BCSSET not defined");
      return ( ERROR );
      }

  /*
   * generate set dependent information
   */
  (void) concat(bcslock, sizeof(bcslock),
		  BCSBBASE, "/.BCSlock", NULL);
  (void) concat(bcsconfig, sizeof(bcsconfig),
		  BCSBBASE, "/.BCSconfig", NULL);
  (void) concat(bcsset, sizeof(bcsset),
		  BCSBBASE, "/.BCSset-", BCSSET_NAME, NULL);
  (void) concat(bcslog, sizeof(bcslog),
		  BCSBBASE, "/.BCSlog-", BCSSET_NAME, NULL);
  (void) concat(bcspath, sizeof(bcspath),
	  BCSBBASE, "/.BCSpath-", BCSSET_NAME, NULL);

  if ( (status_tmp = read_legal_copyrights ( contents ) ) != OK )
    status = status_tmp;
  /* if */
  ui_print ( VDEBUG, "Leaving sci_init\n" );
  return ( status );
} /* end sci_init */


/*
 * The following variables are utilized by sci_new_list and confirm_alloc.
 */

#define MAX_SCI_LIST 100

STATIC
int serial_num = 0;

STATIC
SCI_LIST serial_list [ MAX_SCI_LIST ];

/*
 * FUNCTION confirm_alloc
 *
 * Confirm that the given object was actually allocated.
 */
BOOLEAN confirm_alloc ( sl )
SCI_LIST sl;
{
  if ( sl != NULL && sl -> serial_num >= 0 &&
       sl -> serial_num <= ( MAX_SCI_LIST - 1) &&
       serial_list [ sl -> serial_num ] == sl )
    return ( TRUE );
  else {
    ui_print ( VALWAYS, "\n*** INTERNAL ERROR 0001 ***\n\n" );
    ui_print ( VFATAL, "Unallocated object detected.\n" );
    ui_print ( VCONT, "Report failure immediately!!\n" );
    report_current_function ( );
    ui_print ( VALWAYS, "\n" );
    log_error ( );
    return ( FALSE );
  } /* end if */
} /* end confirm_alloc */


/*
 * FUNCTION sci_new_list
 *
 * Create a new list. Must be called before any calls to sci_add_to_list
 * are made.
 */
int sci_new_list ( sl )
SCI_LIST * sl;
{
  BOOLEAN status;

  ui_print ( VDEBUG, "Entering sci_new_list\n" );
  *sl = ( SCI_LIST ) malloc ( sizeof ( struct sci_list ) );
  if ( *sl == NULL) {
    ui_print ( VFATAL, "Unable to allocate %d bytes of memory!\n" );
    ui_print ( VFATAL, "Function: sci_new_list.\n" );
    status = ERROR;
  } else {
    if ( serial_num == MAX_SCI_LIST ) {
      ui_print ( VFATAL, "Exceeded limit set on number of\n" );
      ui_print ( VCONT, "SCI_LIST objects that may be allocated.\n" );
      ui_print ( VCONT, "Limit currently set at: %d.\n", MAX_SCI_LIST );
      status = ERROR;
    } else {
      serial_list [ serial_num ] = *sl;
      ( *sl ) -> serial_num = serial_num++;
      ( *sl ) -> head = NULL;
      ( *sl ) -> tail = NULL;
      ( *sl ) -> elem_num = 0;
      status = OK;
    } /* end if */
  } /* end if */
  ui_print ( VDEBUG, "Leaving sci_new_list\n" );
  return ( status );
} /* end sci_new_list */


/*
 * FUNCTION sci_add_to_list
 */
int sci_add_to_list ( sl, file_name, org_dir )
SCI_LIST sl;
char * file_name;
char * org_dir;
{
  SCI_ELEM sci_ptr;
  char buf [MAXPATHLEN];
  char canonical_name [MAXPATHLEN];

#ifndef NO_DEBUG
  enter ( "sci_add_to_list" );
#endif
  if ( ! confirm_alloc ( sl ) ) {
#ifndef NO_DEBUG
    leave ( );
#endif
    return ( ERROR );
  } /* end if */
  ui_print ( VDEBUG, "Adding %s to a list.\n", file_name );
  sci_ptr = ( SCI_ELEM ) malloc ( sizeof ( struct sci_elem ) );
  if ( sci_ptr == NULL ) {
    ui_print ( VFATAL, "alloc of sci_ptr failed.\n" );
    return ( ERROR );
  } /* end if */
  if (( *file_name == SLASH ) ||               /* absolute path; ./ beginning */
      (( *file_name == PERIOD ) && (*( file_name + 1 ) == SLASH )) ||
      ( *org_dir == NUL )) {                   /* not below src */
    strcpy ( buf, file_name );
  } else {                       /* relative path so use ./org_dir/field */
    concat ( buf, sizeof ( buf ), org_dir, "/", file_name,
             NULL );
  } /* else */
/*
 * This is messy, but it works for now.
 */
  if ( canonicalize ( ".", buf, canonical_name, sizeof (canonical_name) )
       != OK )
    return ( ERROR );
  /* end if */
  sci_ptr -> name = salloc ( canonical_name );
  sci_ptr -> ver_user = NULL;
  sci_ptr -> ver_ancestor = NULL;
  sci_ptr -> ver_config = NULL;
  sci_ptr -> ver_merge = NULL;
  sci_ptr -> ver_latest = NULL;
  sci_ptr -> leader = NULL;
  sci_ptr -> skip = FALSE;
  sci_ptr -> locked = FALSE;
  sci_ptr -> need_merge = FALSE;
  sci_ptr -> merged_up = FALSE;
  sci_ptr -> called_getancestor = FALSE;
  sci_ptr -> defunct = FALSE;
  sci_ptr -> status = OK;
  sci_ptr -> same13 = FALSE;
  sci_ptr -> same23 = FALSE;
  sci_ptr -> has_user_branch = FALSE;
  sci_ptr -> has_merge_branch = FALSE;
  sci_ptr -> next = NULL;
  if ( sl -> tail == NULL )
    sl -> head = sci_ptr;
  else
    ( sl -> tail ) -> next = sci_ptr;
  /* end if */
  sl -> tail = sci_ptr;
  sl -> elem_num += 1;
#ifndef NO_DEBUG
  leave ( );
#endif
  return ( OK );
} /* end sci_add_to_list */


/*
 * FUNCTION sci_first
 *
 * Give the first element in a given list
 */
SCI_ELEM sci_first ( sl )
SCI_LIST sl;
{
  if ( ! confirm_alloc ( sl ) ) {
    log_error ( );
    return ( NULL );
  } /* end if */
  return ( sl -> head );
} /* end sci_first */


/*
 * FUNCTION sci_next
 *
 * Give the next element in a list from the element given.
 */
SCI_ELEM sci_next ( se )
SCI_ELEM se;
{
  return ( se -> next );
} /* end sci_next */


/*
 * FUNCTION sci_lookup_leader_list
 */
int sci_lookup_leader_list ( sl )
SCI_LIST sl;
{
  SCI_ELEM sci_ptr;
  char leader [MAXPATHLEN];

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_read_leader( sci_ptr -> name, leader ) == ERROR )
      return( ERROR );
    /* end if */
    ui_print ( VDEBUG, "In sci_lookup_leader_list, leader is '%s'\n", leader );
    sci_ptr -> leader = salloc ( leader );
    ui_print ( VDEBUG, "In sci_lookup_leader_list, leader is '%s'\n",
                       sci_ptr -> leader );
  } /* end for */
  return( OK );
} /* sci_lookup_leader_list */


/*
 * In the old bsubmit, the code equivalent to sci_init was invoked multiple
 * times. Once for each invokation of bci, bco, etc. Thus, bcsset
 * was set multiple times. The following routine is a temporary
 * kludge to accomplish the same thing for the two variables that require
 * being set a second time.
 */

set_and_log_kludge ( set_name )
char * set_name;
{
  (void) concat(bcsset, sizeof(bcsset),
		  BCSBBASE, "/.BCSset-", set_name, NULL);
  (void) concat(bcslog, sizeof(bcslog),
		  BCSBBASE, "/.BCSlog-", set_name, NULL);
}


/*
 * FUNCTION sci_all_list
 *
 * Create a list of all files in a given set. The new list will
 * be allocated. A new list does not need to be passed in, and
 * will in fact be overwritten.
 */
int sci_all_list ( sl , set_name )
SCI_LIST * sl;
char *set_name;
{
  FILE      * fp;
  char        pathbuf [ PATH_LEN ],                         /* misc string */
              filebuf [ PATH_LEN ],                         /* misc string */
              tmp_buf [ PATH_LEN ],                         /* misc string */
            * ptr_buff,                                /* misc ptr to char */
            * set_file_name;                           /* name of set file */

  ui_print ( VDEBUG, "In sci_all_list\n" );
  sci_new_list ( sl );
  if ( full_set_name ( &set_file_name, set_name ) == ERROR ) {
    ui_print ( VDEBUG, "Leaving sci_all_list\n" );
    return ( ERROR );
  }
  if (( fp = fopen ( set_file_name, READ )) == NULL ) {
    ui_print ( VFATAL, "Unable to open set file.\n" );
    ui_print ( VCONT, "The set file contains the list of files in a set.\n" );
    ui_print ( VCONT, "The abscence of a set file may indicate that no\n" );
    ui_print ( VCONT, "files belong to the set, the set name was\n" );
    ui_print ( VCONT, "misspelled, or the file has been removed.\n" );
    ui_print ( VCONT, "Set %s\n", set_name );
    ui_print ( VCONT, "Set file: %s\n", set_file_name );
    return ( ERROR );
  }
  while ( fgets ( filebuf, PATH_LEN, fp ) != NULL ) {
    rm_newline ( filebuf );
/*
    concat ( tmp_buf, sizeof ( tmp_buf ), filebuf, ",v", NULL );
*/
    strcpy ( tmp_buf, filebuf );
    if (( ptr_buff = salloc ( tmp_buf )) == NULL ) {
      ui_print ( VFATAL, "No room for salloc of: %s\n", tmp_buf );
      ui_print ( VDEBUG, "Leaving sci_all_list\n" );
      return ( ERROR );
    }
    if ( sci_add_to_list ( *sl, ptr_buff ) != OK ) {
      return ( ERROR );
    } /* end if */
  } /* while */
  fclose ( fp );
  ui_print ( VDEBUG, "Leaving sci_all_list\n" );
  return ( OK );
}


/*
 * FUNCTION sci_lookup_user_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int sci_lookup_user_rev_list ( sl , set_name , missing_revs )
SCI_LIST sl;
char * set_name;
int * missing_revs;
{
  char rev_str [MAXPATHLEN];
  int status;
  char buf [PATH_LEN];
  char * buf_ptr;
  char * ver_user;
  char * def_str;
  SCI_ELEM sci_ptr;

  ui_print ( VDEBUG, "Entering sci_lookup_user_rev_list.\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  confirm_alloc ( sl );
  status = OK;
  * missing_revs = FALSE;
  for (sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user != NULL)
      if (strcmp ( sci_ptr -> ver_user, "outdated" ) == 0 )
        continue;
      /* end if */
    /* end if */
    if ( src_ctl_lookup_revision ( sci_ptr -> name, set_name, buf ) != OK ) {
      status = ERROR;
      continue;
    }
    if ( buf == NULL || *buf == '\0' ) {
      sci_ptr -> ver_user = NULL;
      * missing_revs = TRUE;
    } else {
      buf_ptr = buf;
      ver_user = nxtarg ( &buf_ptr, WHITESPACE );
      def_str = nxtarg ( &buf_ptr, WHITESPACE );
      ui_print ( VDEBUG, "ver_user, def_str '%s', '%s'\n", ver_user, def_str );
      if ( strcmp ( def_str, "(defunct)" ) == 0 ) {
        ui_print ( VDEBUG, "version is defunct\n" );
        sci_ptr -> defunct = TRUE;
      } else if ( sci_ptr -> defunct ) {
        ui_print ( VFATAL, "Contradiction between .BCSconfig file and\n" );
        ui_print ( VCONT, "information in source control system\n" );
        ui_print ( VCONT, "regarding defunct status.\n" );
        ui_print ( VCONT, "For file: '%s'\n", sci_ptr -> name );
        status = ERROR;
      }
      sci_ptr -> ver_user = salloc ( ver_user );
      if ( sci_ptr -> ver_user == NULL ) {
        ui_print ( VFATAL, "salloc of ver_user failed\n" );
        status = ERROR;
        break;
      } /* end if */
    } /* end if */
  } /* end for */
  ui_print ( VDEBUG, "Leaving sci_lookup_user_rev_list.\n" );
  return ( status );
} /* sci_lookup_user_rev_list */


/*
 * FUNCTION sci_lookup_latest_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int sci_lookup_latest_rev_list ( sl , set_name , missing_revs )
SCI_LIST sl;
char * set_name;
int * missing_revs;
{
  int status = OK;
  char buf [PATH_LEN];
  SCI_ELEM sci_ptr;

  ui_print ( VDEBUG, "Entering sci_lookup_latest_rev_list.\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */
  * missing_revs = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next) {
    if ( sci_ptr -> defunct )
      continue;
    /* if */
    src_ctl_lookup_revision ( sci_ptr -> name, set_name, buf );
    if ( buf == NULL || *buf == '\0' ) {
      sci_ptr -> ver_latest = NULL;
      * missing_revs = TRUE;
    } else {
      sci_ptr -> ver_latest = salloc ( buf );
      if ( sci_ptr -> ver_latest == NULL ) {
        ui_print ( VFATAL, "salloc of sci_ptr -> ver_latest failed.\n" );
        status = ERROR;
        break;
      } /* if */
    } /* if */
  } /* for */
  ui_print ( VDEBUG, "Leaving sci_lookup_latest_rev_list.\n" );
  return ( status );
} /* sci_lookup_latest_rev_list */


/*
 * FUNCTION sci_lookup_merge_rev_list
 *
 * Find the correct version to merge against ( whether an actual merge
 * is necessary or not) .
 */
int sci_lookup_merge_rev_list ( sl , rev_str, config_str )
SCI_LIST sl;
char * rev_str;
char * config_str;
{
  SCI_ELEM sci_ptr;
  char rev1[32];
  int status;

 /*
  * determine revision number for version to be merged with.
  */
  status = OK;
  enter ( "sci_lookup_merge_rev_list" );
  ui_print ( VDEBUG, "rev_str '%s'\n", rev_str );
  ui_print ( VDEBUG, "config_str '%s'\n", config_str );
  for ( sci_ptr = sci_first ( sl ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) ) {
    if ( src_ctl_lookup_revision( sci_ptr -> name, rev_str, rev1) != OK ) {
/*
      ui_print ( VFATAL, "Revision %s not found\n", rev_str );
      status = ERROR;
      sci_ptr -> status = status;
*/
      sci_ptr -> has_merge_branch = FALSE;
      if ( src_ctl_lookup_revision ( sci_ptr -> name, config_str, rev1 )
           != OK ) {
        ui_print ( VFATAL, "Revision %s not found\n", rev_str );
        status = ERROR;
        sci_ptr -> status = status;
        continue;
      } /* end if */
    } else
      sci_ptr -> has_merge_branch = TRUE;
    /* end if */
    ui_print ( VDEBUG, "Setting ver_merge to '%s'\n", rev1 );
    sci_ptr -> ver_merge = salloc ( rev1 );
  } /* end for */
 /*
  * Error from sci_first or sci_next ?
  */
  if ( is_in_error ( ) )
    status = ERROR;
  /* end */
  leave ( );
  return ( status );
} /* end sci_lookup_merge_rev_list */


/*
 * FUNCTION sci_ancestor_rev_list
 *
 * Find the ancestors for the files given and determine if
 * a merge is necessary
 */
int sci_ancestor_list ( sl )
SCI_LIST sl;
{
  char revision[32], rev1[32], rev2[32], rev3[32];
  char *rev;
  int branch_exists;
  int status;
  int pid;
  SCI_ELEM sci_ptr;

  ui_print ( VDEBUG, "Entering sci_ancestor_list\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  if (!temp_merge_setup)
    src_ctl_setup_merge();
  /* end if */
  status = OK;
  for ( sci_ptr = sl -> head; sci_ptr != NULL;
        sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user == NULL ) {
      sci_ptr -> status = ERROR;
      continue;
    } /* end if */
    if ( sci_ptr -> defunct ) {
      continue;
    } /* end if */
    /*
     * check if working file is writable
     */
/*
 * NEEDED?
 *
    if (access( sci_ptr -> name, W_OK) == 0) {
      ui_print ( VFATAL, "The file %s is already writeable; branch merge aborted\n", sci_ptr -> name );
      status = ERROR;
      sci_ptr -> status = status;
      continue;
    }
 */

    /*
     * remove temp file
     */
/*
 * Not needed?
 *
    (void) unlink(temp_working_file);
 */

    /*
     * determine revision number for "our" revision
     */

/*
 * Experimental changes:
    if (src_ctl_lookup_revision( sci_ptr -> name, setrev+2, rev2) == 0)
      branch_exists = TRUE;
    else {
      if (usetrunk) {
	ui_print ( VFATAL, "Trunk revision does not exist\n");
        status = ERROR;
        sci_ptr -> status = status;
        continue;
      }
      branch_exists = FALSE;
      if ( src_ctl_lookup_revision( sci_ptr -> name, (char *)NULL, rev2) == ERROR ) {
        ui_print ( VFATAL, "bmerge config revision not found\n");
        status = ERROR;
        sci_ptr -> status = status;
        continue;
      }
    }
*/

    strcpy ( rev1, sci_ptr -> ver_merge );
    strcpy ( rev2, sci_ptr -> ver_user ); /* replaces code from above */
    /*
     * determine revision number for "common" revision
     */
    if (*common_config != '\0') {
      if ( src_ctl_lookup_revision( sci_ptr -> name, common_config+2, rev3) == ERROR ) {
        status = ERROR;
        sci_ptr -> status = status;
        continue;
      }
    }
    else
      rev3[0] = '\0';

    if ( src_ctl_prep_merge ( rev1, rev2, rev3, sci_ptr ) != OK ) {
      status = ERROR;
      sci_ptr -> status = status;
      continue;
    }
  } /* end while */
  return ( status );
} /* end sci_ancestor_list */


/*
 * FUNCTION sci_locked_list
 *
 * Determine if the user has locks on the files, or if a lock
 * exists on the ver_merge version of the file.
 */
sci_locked_list ( sl , rev_str, locks_set )
SCI_LIST sl;
char * rev_str;
int * locks_set;
{
  int p[2];
  int pid;
  SCI_ELEM sci_ptr;
  char * rev;
  char * av [6];
  char buffer[MAXPATHLEN];
  char * ptr;
  FILE *inf;
  int test;
  int status;
  BOOLEAN found_lock_line;

  enter ( "sci_locked_list" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  if ( ( rev = alloc_switch('r', rev_str ) ) == NULL ) {
    ui_print ( VFATAL, "Revision alloc switch failed ");
    leave ( );
    return ( ERROR );
  }

  *locks_set = FALSE;
  av[0] = "authcover";
  av[1] = "rlog";
  av[2] = "-h";
  av[3] = rev;
  av[5] = NULL;
  status = OK;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user == NULL ) {
      sci_ptr -> status = ERROR;
      continue;
    }
    if (pipe(p) < 0) {
      ui_print ( VFATAL, "Pipe failed" );
      status = ERROR;
      sci_ptr -> status = ERROR;
      continue;
    }
    av[4] = sci_ptr -> name;
    pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, (char *)NULL, p[0], p[1] );
    if (pid == -1) {
      status = ERROR;
      continue;
    } /* end if */
    (void) close(p[1]);
    if ((inf = fdopen(p[0], "r")) == NULL) {
      ui_print ( VFATAL, "fdopen failed" );
      (void) endcmd(pid);
      status = ERROR;
      sci_ptr -> status = ERROR;
      continue;
    }
    found_lock_line = FALSE; 
    while (fgets(buffer, sizeof(buffer), inf) != NULL) {
      rm_newline ( buffer);
      if ( strncmp ( buffer, "locks:", 5) == 0 ) {
        found_lock_line = TRUE;
        continue;
      } /* if */
      if ( found_lock_line) 
        if ( *buffer != '\t' )
          break;
        else {
          if ( strstr ( buffer, USER ) != NULL) {
            ui_print ( VDEBUG,"User has lock\n" );
            sci_ptr -> locked = TRUE;
            *locks_set = TRUE;
          } /* end if */
          /*
             If there is no branch to merge against, it will be created later
             and cannot possibly exist yet. Therefore, it cannot possibly
             be locked! So, only check for locks on the merge branch if
             there is one.
          */
          if ( sci_ptr -> has_merge_branch)
            if ( strstr ( buffer, sci_ptr -> ver_merge ) != NULL) {
              ui_print ( VDEBUG,"Some one has a lock on '%s'.\n",
                                sci_ptr -> ver_merge );
              sci_ptr -> locked = TRUE;
              *locks_set = TRUE;
            } /* if */
          /* if */
        } /* if */
      /* if */
    } /* end while */
    if ( ferror ( inf ) || fclose ( inf ) == EOF ) {
      ui_print ( VFATAL, "Error reading pipe");
      (void) endcmd(pid);
      status = sci_ptr -> status = ERROR;
      continue;
    } /* end if */
    (void) endcmd(pid);
    if ( ! found_lock_line ) {
      ui_print ( VFATAL, "Could not find lock line in source control file.\n" );
      status = sci_ptr -> status = ERROR;
    } /* end if */
  } /* end for */
  leave ( );
  return ( status );
} /* end sci_locked_list */


/*
 * FUNCTION sci_is_branch
 *
 * Determine if the given revisions represent branches
 */
int sci_is_branch ( sl , bad_branches)
SCI_LIST sl;
int * bad_branches;
{
  SCI_ELEM sci_ptr;
  char * p;
  int i;
  int revision;

  enter ( "sci_is_branch" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* end if */
  *bad_branches = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user == NULL)
      continue;
    /* end if */
    i = 0;
    for ( p = sci_ptr -> ver_user; *p != NUL ; p++ ) {
      if ( *p == '.' ) {
        i++;
        if ( i == 3 ) {
          p++;
          break;
        }
        /* if */
      } /* if */
    } /* for */
    if ( i == 3 ) {
      ATOI ( revision, p );
    } else
      return ( ERROR );
    /* if */
    if ( revision > 1 )
      sci_ptr -> has_user_branch = TRUE;
    else
      *bad_branches = TRUE;
    /* if */
  } /* for */
  leave ( );
  return ( OK );
} /* end sci_is_branch */


/*
 * FUNCTION sci_check_in_list
 *
 * Check in given files.
 * Current version does not (and does not need to ) handle log messages.
 */
int sci_check_in_list ( sl , build_set, user_set, state, no_log )
SCI_LIST sl;
char * build_set;
char * user_set;
char * state;
BOOLEAN no_log;
{
  SCI_ELEM sci_ptr;
  char leader [MAXPATHLEN];
  char logmsg [LOGMSGSIZE];
  int status = OK;
  char branch_rev[32];
  char * b_ptr;
  BOOLEAN done;

  enter ( "sci_check_in_list" );
  logmsg [0] = '\0';
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* end if */
  (void) concat(mesgfile, sizeof(mesgfile), BCSTEMP, "/_LOG_", NULL);
  set_and_log_kludge ( build_set );
  ui_print ( VDEBUG, "mesgfile is '%s'\n", mesgfile );
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    ui_print ( VALWAYS, "%s\n", sci_ptr -> name );
    check_path ( sci_ptr -> name );
    begin_atomic ( );
    if ( copy_file ( sci_ptr -> name, temp_working_file, TRUE ) != 0) {
      status = ERROR;
      end_atomic ( );
      continue;
    } /* end if */
    if ( (strcmp ( sci_ptr -> leader, "NONE" )) == 0 ||
	 (strcmp ( sci_ptr -> leader, "BIN" ) == 0 )) {
      ui_print ( VDEBUG, "mesgfile is '%s'\n", mesgfile );
      if ( create_leaderless_log ( sci_ptr, user_set, logmsg, mesgfile ) !=
           OK) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* if */
    } else if ( ! sci_ptr -> defunct && ! no_log ) {
      if ( sci_xtract_log ( sci_ptr, sci_ptr -> leader ) != OK ) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
      ui_print ( VDEBUG, "logmsg is '%s'\n", logmsg );
      if ( okmesg ( sci_ptr -> leader, logmsg ) != OK ) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
    } /* end if */
    ui_print ( VDEBUG, "logmsg is '%s'\n", logmsg );
    ui_print ( VDEBUG, "Before calling src_ctl_check_in, ver_merge is '%s'\n",
               sci_ptr -> ver_merge );
   /*
    * If a branch already exists for the set we are submitting to, then
    * we can just bump up the revision number. E.g., W.X.Y.Z goes to
    * W.X.Y.Z+1. Otherwise, we need to create W.X.new branch.1, where
    * NEWBRANCH is determined by src_ctl_create_branch. Then we can bump
    * up the revision number as in the first case.
    */
    if ( ! sci_ptr -> has_merge_branch ) {
      done = FALSE;
      strcpy ( branch_rev, sci_ptr -> ver_merge );
      for ( b_ptr = branch_rev; *b_ptr != '\0'; b_ptr++) {
        if (*b_ptr != '.')
          continue;
        /* end if */
        if ( done ) {
          *b_ptr = '\0';
          break;
        } /* end if */
        done = TRUE;
      } /* end for */
      if ( src_ctl_create_branch ( sci_ptr -> name, branch_rev, build_set )
           != OK) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
     /*
      * NOTE: branch_rev is an 'out' parameter. The old value from
      * above is overwritten.
      */
      if ( src_ctl_lookup_revision( sci_ptr -> name, build_set, branch_rev)
           != OK ) {
        ui_print ( VFATAL, "Could not find newly created branch\n" );
        ui_print ( VCONT, "for set: '%s'.\n", build_set );
        status = ERROR;
        end_atomic ( );
        continue;
      } else
        sci_ptr -> ver_merge = salloc ( branch_rev );
      /* end if */
    } /* end if */

/*
 * The braces are necessary for this if statement. Do not remove them!
 */
    if ( sci_ptr -> defunct ) {
      if ( src_ctl_check_in ( sci_ptr, sci_ptr -> ver_merge,
                              "File is defunct", "Defunct" )
           != OK ) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
    } else {
      if ( src_ctl_check_in ( sci_ptr, sci_ptr -> ver_merge, logmsg , state )
                            != OK ) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
      if ( ! no_log )
        if ( save_log_message( mesgfile ) != OK) {
          status = ERROR;
          end_atomic ( );
          continue;
        } /* if */
      /* if */
      (void) unlink(mesgfile);
      (void) unlink ( temp_working_file );
    }/* if */
    track_insert ( sci_ptr -> name );
    end_atomic ( );
  } /* end for */
  leave ( );
  return ( status );
} /* sci_check_in_list */


/*
 * FUNCTION sci_lock_list
 *
 * Lock given files
 * Currently hard-wired to lock ver_merge.
 * Will not do a lock if sl -> has_merge_branch is FALSE.
 */
sci_lock_list ( sl )
SCI_LIST sl;
{
  SCI_ELEM sci_ptr;
  char tmp_buf [MAXPATHLEN];
  int status = OK;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip ) {
      sci_ptr -> skip = FALSE;
      continue;
    } /* end if */
    concat ( tmp_buf, sizeof (tmp_buf), sci_ptr -> name, ",v" , NULL );
    begin_atomic ( );
    if ( sci_ptr -> has_merge_branch )
      if ( src_ctl_lock_revision ( sci_ptr -> ver_merge, tmp_buf )
                != OK )
        status = ERROR;
      /* end if */
    else
      track_insert ( sci_ptr -> name );
    /* end if */
    end_atomic ( );
  } /* end for */
  return ( status );
} /* end sci_lock_list */


/*
 * FUNCTION sci_lock_list
 *
 * This function and merge_elem are temporary kludges. 
 * They should be replaced with other functions such that
 * the merging can be controlled externally.
 */
sci_merge_list ( sl, no_log )
SCI_LIST sl;
BOOLEAN no_log;
{
  SCI_ELEM sci_ptr;
  int status = OK;
  int status2;
  int m_e_status;
  int fd;

  char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];

  enter ( "sci_merge_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* end if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip ) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    ui_print ( VALWAYS, "%s\n", sci_ptr -> name );
    check_path ( sci_ptr -> name );
    if ( sci_ptr -> defunct ) {
      (void) unlink(temp2);
      if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
        ui_print ( VFATAL, "Unable to open %s for write", temp2);
        (void) unlink(temp1);
        (void) unlink(temp2);
        return( ERROR );
      }
      ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_merge );
      status2 = OK;
      if ( src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_merge, fd, sci_ptr -> leader) == ERROR)
        status2 = ERROR;
      /* end if */
      (void) close(fd);
      if ( status2 != OK ) {
        ui_print ( VFATAL, "Check out failed.\n", sci_ptr -> ver_user );
        (void) unlink(temp1);
        (void) unlink(temp2);
        status = ERROR;
        continue;
      }
      if (rename(temp2, temp_working_file) < 0) {
        ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2,
                   temp_working_file);
        (void) unlink(temp2);
        status = ERROR;
        continue;
      }
    } else {
      m_e_status = merge_elem ( sci_ptr, no_log );
      if ( m_e_status != OK ) {
        status = m_e_status;
        if ( status == ABORT )
          break;
        else
          continue;
        /* end if */
      } /* end if */
      sci_ptr -> merged_up = TRUE;
    /*
     * If we merged onto an existing branch, only need to update our set file
     */
/*
    if (branch_exists) { 
*/
/*
 * It is temporarily assumed that branch_exists is TRUE.
 */

/*
 * As it turns out this is probably no longer needed.
 *
    if ( TRUE ) {
      if ( sci_config_update(rev1, rev2, rev3) != OK ) {
        status = ERROR;
        continue;
      }
      if ( set_insert() != OK ) {
        status = ERROR;
        continue;
      }
    }
 */
    }
    if ( copy_file ( temp_working_file, sci_ptr -> name, TRUE ) != 0) {
      status = ERROR;
      continue;
    } /* end if */
    unlink ( temp_working_file );
    track_insert ( sci_ptr -> name );
  } /* end for */
  leave ( );
  return ( status );
} /* end sci_merge_list */


/*
 * FUNCTION sci_update_build_list
 *
 * Update the backing build copies of the files. Either check them out
 * or remove (defunct) them.
 */
int sci_update_build_list ( sl )
SCI_LIST sl;
{
  SCI_ELEM sci_ptr;
  int status = OK;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    ui_print ( VALWAYS, "%s\n", sci_ptr -> name );
    check_path ( sci_ptr -> name );
    begin_atomic ( );
    if ( sci_ptr -> defunct ) {
      if ( remove_working_file ( ) != OK ) {
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
    } else {
      (void) unlink(temp_working_file);
      src_ctl_check_out ( sci_ptr -> name, sci_ptr -> ver_latest, sci_ptr -> leader );
      if ( copy_file ( temp_working_file, sci_ptr -> name, FALSE ) !=  OK ) {
        (void) unlink(temp_working_file);
        status = ERROR;
        end_atomic ( );
        continue;
      } /* end if */
    } /* end if */
    unlink ( temp_working_file );
    track_insert ( sci_ptr -> name );
    end_atomic ( );
  } /* end for */
  return ( status );
} /* end sci_update_build_list */


/*
 * FUNCTION sci_outdate_list
 */
int sci_outdate_list ( sl , set_name )
SCI_LIST sl;
char * set_name;
{
  SCI_ELEM sci_ptr;
  int status = OK;
  char o_string[MAXPATHLEN];
  int pid;
  char *ptr;
  int i;
  char *av[16];
  char rcs_file_name [MAXPATHLEN];
  char symbolic_name_switch [MAXPATHLEN];

  ui_print ( VDEBUG, "Entering sci_outdate_list\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  set_and_log_kludge ( set_name );
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    concat ( o_string, sizeof ( o_string ), "-o-", sci_ptr -> ver_user, NULL );
    i = 0;
    concat ( rcs_file_name, sizeof ( rcs_file_name ), sci_ptr -> name,
             ",v", NULL );
    concat ( symbolic_name_switch, sizeof ( symbolic_name_switch ), "-n",
             set_name, NULL );
    av[i++] = "authcover";
    av[i++] = "rcs";
    av[i++] = o_string;
    av[i++] = symbolic_name_switch;
    av[i++] = rcs_file_name;
    av[i++] = NULL;

    begin_atomic ( );
    pid = full_runcmdv(RCSCOVER, av, FALSE,
                       rcs_child_init, (char *)NULL, -1, -1);
    if (pid == -1) {
      status = ERROR;
      end_atomic;
      continue;
    } /* end if */
    status = endcmd(pid);
    if ( src_ctl_config_remove ( sci_ptr ) != OK ) {
      status = ERROR;
      end_atomic;
      continue;
    } /* end if */
    if ( src_ctl_set_remove ( sci_ptr ) != OK ) {
      status = ERROR;
      end_atomic;
      continue;
    }
    if ( unlink ( sci_ptr -> name ) == 0 )
      ui_print ( VDETAIL, "rm: removing %s\n", sci_ptr -> name);
    /* end if */
    track_insert ( sci_ptr -> name );
    end_atomic ( );
  } /* end for */
  ui_print ( VDEBUG, "Leaving sci_outdate_list\n" );
  return ( status );
} /* end sci_outdate_list */


/*
 * FUNCTION sci_trackfile
 *
 * The following function is a temporary kludge.
 * externally to set the track file.
 */
int sci_trackfile ( file_name )
char * file_name;
{
  ui_print ( VDEBUG, "Entering sci_trackfile\n" );
  strcpy ( trackfile, file_name );
  ui_print ( VDEBUG, "Leaving sci_trackfile\n" );
}


/*
 * FUNCTION sci_config_lookup_list
 */
sci_config_lookup_list ( sl )
SCI_LIST sl;
{
  SCI_ELEM sci_ptr;
  char config_rev [32];
  int status;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  status = OK;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( src_ctl_config_lookup ( sci_ptr, config_rev ) != OK ) {
      status = ERROR;
      continue;
    }
    sci_ptr -> ver_config = salloc ( config_rev );
    ui_print ( VDEBUG, "config_rev is '%s'\n", config_rev );
  } /* end if */
  return ( status );
} /* end sci_config_lookup_list */
