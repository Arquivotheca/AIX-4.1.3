static char sccsid[] = "@(#)47  1.1  src/bldenv/sbtools/libsb/par_rc_file.c, bldprocess, bos412, GOLDA411a 4/29/93 12:22:21";
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
 */
/*
 * ODE 2.1.1
 */

/*
**      1) get_rc_value ( char* keyword, char** output,
**                        struct rcfile contents, int report_error ) int
**
**         returns: ERROR if any type of failure else it returns OK.
**
**         usage:
**           The value of the keyword is filled from the rc contents from
**           file and put into new space which *output points to. If the
**           keyword is not found and report_error is TRUE, an error is
**           reported.
**
*/
/*
 * routines to parse rc file into description structure
 */
#include <sys/param.h>
#include <sys/time.h>
#include <stdio.h>
#ifdef INC_TIME
#include <time.h>
#endif
#include <ode/parse_rc_file.h>
# include <ode/odedefs.h>

#define STATIC
#ifndef STATIC
#define STATIC static
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

extern char *salloc();
extern char *concat();
extern char *getenv();
extern char *nxtarg();

#define	INSERT	1
#define	REMOVE	2

STATIC
find_field(rcfile_p, field, field_pp, flags)
struct rcfile *rcfile_p;
char *field;
struct field **field_pp;
int flags;
{
    int i;
    char *p;
    struct hashent **hpp, *hp;

    i = 0;
    for (p = field; *p != '\0'; p++)
	i += *p;
    hpp = &rcfile_p->hashtable[RC_HASH(i)];
    while ((hp = *hpp) != (struct hashent *)NULL) {
	if (strcmp(hp->field->name, field) != 0) {
	    hpp = &(hp->next);
	    continue;
	}
	*field_pp = hp->field;
	if ((flags&REMOVE) == 0)
	    return(0);
	*hpp = hp->next;
	(void) free(hp);
	return(0);
    }
    if ((flags&INSERT) == 0 || (flags&REMOVE) != 0)
	return(1);
    if ((field = salloc(field)) == NULL)
	return(1);
    *field_pp = (struct field *) calloc(sizeof(char), sizeof(struct field));
    if (*field_pp == (struct field *)NULL)
	return(1);
    (*field_pp)->name = field;
    hp = (struct hashent *) calloc(sizeof(char), sizeof(struct hashent));
    if (hp == (struct hashent *)NULL)
	return(1);
    *hpp = hp;
    hp->field = *field_pp;
    if (rcfile_p->last == (struct field *)NULL)
	rcfile_p->list = *field_pp;
    else
	rcfile_p->last->next = *field_pp;
    rcfile_p->last = *field_pp;
    return(0);
}

STATIC
free_field(field_p)
struct field *field_p;
{
    struct arg_list *args_p;

    while ((args_p = field_p->args) != NULL) {
	field_p->args = args_p->next;
	free_args(args_p);
    }
    free(field_p->name);
    free((char *)field_p);
}

STATIC
free_args(args_p)
struct arg_list *args_p;
{
    while (--args_p->ntokens >= 0)
	free(args_p->tokens[args_p->ntokens]);
    free((char *)args_p->tokens);
    free((char *)args_p);
}

STATIC
create_arglist(field_p, args_pp)
struct field *field_p;
struct arg_list **args_pp;
{
    struct arg_list **app;

    app = &field_p->args;
    while (*app != (struct arg_list *)NULL)
	    app = &((*app)->next);
    *app = (struct arg_list *) calloc(sizeof(char), sizeof(struct arg_list));
    if (*app == (struct arg_list *)NULL)
	return(1);
    *args_pp = *app;
    return(0);
}

STATIC
find_arg(field_p, arg, args_pp)
struct field *field_p;
char *arg;
struct arg_list **args_pp;
{
    struct arg_list *ap;

    for (ap = field_p->args; ap != NULL; ap = ap->next) {
	if (ap->ntokens != 1)
	    continue;
	if (strcmp(ap->tokens[0], arg) != 0)
	    continue;
	*args_pp = ap;
	return(0);
    }
    return(1);
}

STATIC
append_arg(arg, args_p)
char *arg;
struct arg_list *args_p;
{
    if (args_p->ntokens == args_p->maxtokens) {
	if (args_p->maxtokens == 0) {
	    args_p->maxtokens = 8;
	    args_p->tokens = (char **) malloc((unsigned) args_p->maxtokens *
					      sizeof(char *));
	} else {
	    args_p->maxtokens <<= 1;
	    args_p->tokens = (char **) realloc((char *) args_p->tokens,
					(unsigned) args_p->maxtokens *
					       sizeof(char *));
	}
	if (args_p->tokens == NULL)
	    return(1);
    }
    if ((arg = salloc(arg)) == NULL)
	return(1);
    args_p->tokens[args_p->ntokens++] = arg;
    return(0);
}

STATIC
lookupvar(rcfile_p, name, valp)
struct rcfile *rcfile_p;
char *name, **valp;
{
    struct field *field_p;
    struct arg_list *args_p;
    char *val;

    if ( find_field ( rcfile_p, name, &field_p, 0 ) != 0 ) {
	if ((val = getenv(name)) == NULL) {
	    fprintf(stderr, "lookup/getenv: %s not found\n", name);
	    return(1);
	}
	*valp = val;
	return(0);
    }

    args_p = field_p->args;

    if (args_p == NULL) {
	fprintf(stderr, "field %s has no value\n", name);
	return(1);
    }
    if (args_p->next != NULL) {
	fprintf(stderr, "field %s has more than one value\n", name);
	return(1);
    }
    if (args_p->ntokens == 0) {
	fprintf(stderr, "field %s has no values\n", name);
	return(1);
    }
    if (args_p->ntokens > 1) {
	fprintf(stderr, "field %s has multiple values\n", name);
	return(1);
    }
    *valp = args_p->tokens[0];
    return(0);
}

STATIC
evalarg(rcfile_p, pp, ap)
struct rcfile *rcfile_p;
char **pp, **ap;
{
    static char outbuf[MAXPATHLEN];
    char *front;
    char *p, *p1, *p2;
    char *q;
    char quotechar;

    q = outbuf;
    front = *pp;
    while (*front == ' ' || *front == '\t')
	front++;
    p = front;
    quotechar = '\0';
    while (*p != '\0') {
	if (*p == '\\') {
	    p++;
	    if (*p == '\0') {
		fprintf(stderr, "missing character after '\\'\n");
		return(1);
	    }
	    *q++ = *p++;
	    continue;
	}
	if (*p == '$') {
	    p++;
	    if (*p != '{') {
		fprintf(stderr, "missing '{' after '$' in description\n");
		return(1);
	    }
	    for (p1 = ++p; *p1 != '}'; p1++)
		if (*p1 == '\0') {
		    fprintf(stderr, "missing '}' after '$' in description\n");
		    return(1);
		}
	    *p1++ = '\0';
	    if (lookupvar(rcfile_p, p, &p2) != 0) {
		fprintf(stderr, "lookupvar %s failed\n", p);
		return(1);
	    }
	    while (*p2 != '\0')
		*q++ = *p2++;
	    p = p1;
	    continue;
	}
	if (*p == '"') {
	    if (quotechar == *p)
		quotechar = '\0';
	    else
		quotechar = *p;
	    p++;
	    continue;
	}
	if (quotechar == '\0' && (*p == ' ' || *p == '\t'))
	    break;
	*q++ = *p++;
    }
    *q = '\0';
    *pp = (*p == '\0') ? p : p + 1;
    *ap = outbuf;
    return(0);
}

STATIC
parse_single_rc_file(rcfile, rcfile_p)
FILE *rcfile;
struct rcfile *rcfile_p;
{
    char buf[BUFSIZ];
    char curpath[MAXPATHLEN];
    char dirpath[MAXPATHLEN];
    char filepath[MAXPATHLEN];
    char *p, *field, *arg, *name;
    FILE *ifile;
    struct field *field_p, *tf_p, nfield;
    struct arg_list *args_p;
    int status;
    int isreplace;
    int skip;

    while (fgets(buf, sizeof(buf)-1, rcfile) != NULL) {
	isreplace = 0;
	if (index("#\n", *buf) != NULL)
	    continue;
	if ((p = rindex(buf, '\n')) != NULL)
	    *p = '\0';
	p = buf;

	skip = FALSE;
	for (;;) {

	    field = nxtarg(&p, " \t");

	    if (*field == '\0') {
		skip = TRUE;
		break;
	    }

	    if ( strcmp ( field, "replace" ) == 0 ) {
		isreplace = 1;
		continue;
	    } /* if */

	    if (strcmp(field, "on") == 0) {
		arg = nxtarg(&p, " \t");
		if (*arg == '\0') {
		    fprintf(stderr, "Missing machine name for on directive\n");
		    skip = TRUE;
		    break;
		}
		if (strcmp(arg, MACHINE) != 0) {
		    skip = TRUE;
		    break;
		}
		continue;
	    }

	    break;
	}

	if (skip)
	    continue;

	if (strcmp(field, "include") == 0) {
	    arg = nxtarg(&p, " \t");
	    if ( isreplace == 1 ) {
		fprintf(stderr, "replace include: not a legal combination.\n");
		return(1);
	    }
	    if (*arg == '\0') {
		fprintf(stderr, "missing file name for include directive\n");
		return(1);
	    }
	    if ((ifile = fopen(arg, "r")) == NULL) {
		fprintf(stderr, "cannot find include file %s\n", arg);
		return(1);
	    }
	    path(arg, dirpath, filepath);
	    if (getwd(curpath) == NULL) {
		fprintf(stderr, "getwd: %s\n", curpath);
		return(1);
	    }
	    if (chdir(dirpath) < 0) {
		fprintf(stderr, "chdir %s: %s\n", dirpath, errmsg(-1));
		return(1);
	    }
	    status = parse_single_rc_file(ifile, rcfile_p);
	    if (status != 0)
		fprintf(stderr, "error parsing rc description file %s\n",
				arg);
	    (void) fclose(ifile);
	    if (chdir(curpath) < 0) {
		fprintf(stderr, "chdir %s: %s\n", curpath, errmsg(-1));
		return(1);
	    }
	    if (status != 0)
		return(status);
	    continue;
	}

	if (strcmp(field, "setenv") == 0) {
	    name = nxtarg(&p, " \t");
	    if (*name == '\0') {
		fprintf(stderr, "missing name for setenv directive\n");
		return(1);
	    }
	    if (evalarg(rcfile_p, &p, &arg) != 0) {
		fprintf(stderr, "error evaluating argument\n");
		return(1);
	    }

	    if (setenv(name, arg, isreplace) != 0) {
		fprintf(stderr, "setenv %s failed\n", name);
		return(1);
	    }

	    if (find_field(rcfile_p, field, &field_p, INSERT))
		return(1);
	    if (find_arg(field_p, name, &args_p) == 0)
		continue;
	    if (create_arglist(field_p, &args_p))
		return(1);
	    if (append_arg(name, args_p))
		return(1);
	    continue;
	}

	if (strcmp(field, "unsetenv") == 0) {
	    name = nxtarg(&p, " \t");
	    if ( isreplace == 1 ) {
		fprintf(stderr, "replace unsetenv: not a legal combination.\n");
		return(1);
	    }
	    if (*name == '\0') {
		fprintf(stderr, "missing name for setenv directive\n");
		return(1);
	    }
	    if (unsetenv(name) != 0) {
		fprintf(stderr, "unsetenv %s failed\n", name);
		return(1);
	    }
	    if (find_field(rcfile_p, field, &field_p, INSERT))
		return(1);
	    if (find_arg(field_p, name, &args_p) == 0)
		continue;
	    if (create_arglist(field_p, &args_p))
		return(1);
	    if (append_arg(name, args_p))
		return(1);
	    continue;
	}

	if ( isreplace == 1 ) {
	    if (*field == '\0')
		continue;

	    if (find_field(rcfile_p, field, &tf_p, 0) != 0) {
	      fprintf ( stderr, "field %s not found.\n", field );
	      return (1);
	    }

	    bzero (&nfield, sizeof (nfield));
	    field_p = &nfield;
	} else {
	    if (find_field(rcfile_p, field, &field_p, INSERT))
		return(1);
	    tf_p = NULL;
	}

	if (create_arglist(field_p, &args_p))
	    return(1);

	for (;;) {
	    if (evalarg(rcfile_p, &p, &arg) != 0) {
		fprintf(stderr, "error evaluating argument\n");
		return(1);
	    }
	    if (*arg == '\0')
		break;
	    if (append_arg(arg, args_p))
		return(1);
	}

	if ( tf_p != NULL ) {		  /* covers replacing self with self */
	    if ( tf_p->args != NULL) {
	        (void) free_args( tf_p->args);
		tf_p->args = field_p-> args;
    	    }
	}
    }
    return(0);
}

parse_rc_file(rcpath, rcfile_p)
char *rcpath;
struct rcfile *rcfile_p;
{
    char *spath;
    char curpath[MAXPATHLEN];
    char relpath[MAXPATHLEN];
    char dirpath[MAXPATHLEN];
    char filepath[MAXPATHLEN];
    FILE *rcfile;
    int status;

    if ((rcfile = fopen (rcpath, "r")) == NULL ) {
      fprintf ( stderr, "fopen %s: %s\n", rcpath, errmsg(-1));
      return (1);
    } /* if */
    path(rcpath, dirpath, filepath);
    if (getwd(curpath) == NULL) {
	fprintf(stderr, "getwd: %s\n", curpath);
	return(1);
    }
    if (chdir(dirpath) < 0) {
	fprintf(stderr, "chdir %s: %s\n", dirpath, errmsg(-1));
	return(1);
    }
    status = parse_single_rc_file(rcfile, rcfile_p);
    if (status != 0) {
	fprintf(stderr, "Error parsing rc description file %s\n",
		rcpath);
    }
    (void) fclose(rcfile);
    if (chdir(curpath) < 0) {
	fprintf(stderr, "chdir %s: %s\n", curpath, errmsg(-1));
	return(1);
    }
    return(status);
}

rc_file_field(rcfile_p, field, field_pp)
struct rcfile *rcfile_p;
char *field;
struct field **field_pp;
{
    return(find_field(rcfile_p, field, field_pp, 0));
}

int get_rc_value ( keyword, output, contents, report_missing )

        /* This function actually gets the keyword and makes sure
           it is legal.  It returns ERROR if not, OK if. */

    char      * keyword,                                 /* word to look for */
             ** output;                         /* holds output from keyword */
    struct      rcfile  * contents;             /* holds contents of rc file */
    int         report_missing;                                /* error flag */
{
    struct      field   * fptr;                       /* holds rc field info */
    int         results = ERROR;                             /* return value */

  if ( rc_file_field ( contents, keyword, &fptr )) {
    if ( report_missing)
      ui_print ( VWARN, "%s line missing in sandbox rc file.\n", keyword );
  } /* if */

  else if ( fptr->args->ntokens != 1 )
    ui_print ( VWARN, "impoper syntax in %s field in sandbox rc file.\n",
                      keyword );

  else if ( *fptr->args->tokens == NULL )
    ui_print ( VWARN, "no value for %s field in sandbox rc file %s.\n",
                      keyword );

  else if (( *output = ( char * ) salloc ( *fptr->args->tokens )) == NULL )
    ui_print ( VWARN, "salloc failure on space for %s.\n", *fptr->args->tokens);
  else
    results = OK;

  return ( results );
}                                                            /* get rc value */

free_rc_file(rcfile_p)
struct rcfile *rcfile_p;
{
    int i;
    struct hashent *hash_p;

    for (i = 0; i < RC_HASHSIZE; i++) {
        while ((hash_p = rcfile_p->hashtable[i]) != NULL) {
            rcfile_p->hashtable[i] = hash_p->next;
            free_field(hash_p->field);
	    free((char *)hash_p);
        }
    }
}

/*
 * NOTE:
 * The functions rci_include and rci_expand_include are temporary functions
 * which are currently only usable to expand the check_out_config rc variable.
 * In future versions they may be changed to work for generic include
 * expansions.
 */
int rci_include();

STATIC
int rci_expand_include(cnfg_ptr, config)
char **cnfg_ptr;
char *config;
{
    char buf[MAXPATHLEN];
    char *ptr;
    char *p, *q;
    FILE *fp;
    char ch;
  int status = OK;

  ptr = *cnfg_ptr;
  p = config;
  for (;;) {
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\0') {
      *cnfg_ptr = ptr;
      status = OK;
      break;
    }
    if (*p == ';') {
      p++;
      continue;
    }
    if (strncmp(p, "include", 7) != 0 ||
      (*(p+7) != ' ' && *(p+7) != '\t')) {
      *ptr++ = ';';
      while (*p != ';' && *p != '\0') {
        *ptr++ = *p++;
      }
      ptr--;
      while (*ptr == ' ' || *ptr == '\t')
        ptr--;
      ptr++;
      *ptr = '\0';
      continue;
    }
    p += 7;
    while (*p == ' ' || *p == '\t')
      p++;
    q = p;
    while (*q != '\0' && *q != ';' && *q != ' ' && *q != '\t')
      q++;
    ch = *q;
    *q = '\0';
    if ( rci_include(&ptr, p) != OK ) {
      status = ERROR;
      break;
    }
    *q = ch;
    if (ch == ';')
      q++;
    p = q;
  }
  return ( status );
}

/*
 * Get config date from CONFIG file at base of backing build
 */
int rci_include(cnfg_ptr, file)
char **cnfg_ptr;
char *file;
{
  char buf[MAXPATHLEN];
  FILE *fp;
  char *p;
  int status = OK;

  ui_print ( VDEBUG, "file is '%s'\n", file );
  if ((fp = fopen(file, "r")) == NULL) {
    ui_print ( VFATAL, "Could not open CONFIG file: %s", file);
    return ( ERROR );
  } /* end if */
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    if (p = index(buf, '\n'))
      *p = '\0';
    /* if */
    if ( rci_expand_include(cnfg_ptr, buf) != OK )
      status = ERROR;
    /* if */
  }
  if (ferror(fp) || fclose(fp) == EOF) {
    ui_print ( VFATAL, "Error while reading %s", file);
    return ( ERROR );
  } /* if */
  return ( status );
}
