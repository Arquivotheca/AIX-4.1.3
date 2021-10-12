/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: append_arg
 *		create_arglist
 *		evalarg
 *		find_arg
 *		find_field
 *		free_args
 *		free_field
 *		free_rc_file
 *		get_rc_value
 *		init_rc_contents
 *		lookupvar
 *		parse_rc_file
 *		parse_single_rc_file
 *		rci_expand_include
 *		rci_include
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * HISTORY
 * $Log: par_rc_file.c,v $
 * Revision 1.9.8.1  1993/11/10  16:56:58  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:56:01  root]
 *
 * Revision 1.9.4.6  1993/04/28  20:58:49  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:58:32  damon]
 * 
 * Revision 1.9.4.5  1993/04/08  21:45:24  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:44:55  damon]
 * 
 * Revision 1.9.4.4  1993/02/01  21:59:09  damon
 * 	CR 417. replace keyword will not complain if var is not set
 * 	[1993/02/01  21:58:52  damon]
 * 
 * Revision 1.9.4.3  1993/01/28  23:27:20  damon
 * 	CR 419. Replace rc_file_field with get_rc_value
 * 	[1993/01/28  23:26:27  damon]
 * 
 * Revision 1.9.4.2  1993/01/13  16:43:54  damon
 * 	CR 382. Removed FALSE TRUE STATIC
 * 	[1993/01/05  21:09:14  damon]
 * 
 * Revision 1.9.2.10  1992/12/03  17:21:25  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:32  damon]
 * 
 * Revision 1.9.2.9  1992/11/09  15:15:02  damon
 * 	CR 329. Removed return check on unsetenv
 * 	[1992/11/09  15:14:04  damon]
 * 
 * Revision 1.9.2.8  1992/11/06  19:00:54  damon
 * 	CR 329. Added include of stdlib
 * 	[1992/11/06  19:00:42  damon]
 * 
 * Revision 1.9.2.7  1992/09/24  19:02:02  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:21  gm]
 * 
 * Revision 1.9.2.6  1992/08/07  15:38:41  damon
 * 	CR 266. Changed salloc to strdup
 * 	[1992/08/07  15:37:14  damon]
 * 
 * Revision 1.9.2.5  1992/08/06  16:10:05  damon
 * 	Added init_rc_contents
 * 	[1992/07/31  18:50:50  damon]
 * 
 * Revision 1.9.2.4  1992/07/26  17:30:37  gm
 * 	Fixed to remove warnings when compiling under OSF/1 R1.1.
 * 	[1992/07/14  17:16:54  gm]
 * 
 * Revision 1.9.2.3  1992/03/02  22:33:00  damon
 * 	Fixed file opened but not closed bug
 * 	[1992/03/02  22:32:24  damon]
 * 
 * Revision 1.9.2.2  1992/02/18  22:04:31  damon
 * 	Changes for LBE removal
 * 	[1992/02/18  21:57:11  damon]
 * 
 * Revision 1.9  1991/12/05  21:05:28  devrcs
 * 	Added nul to cnfg_ptr in rci_expand_include
 * 	[1991/11/16  15:23:42  damon]
 * 
 * 	Shared sandbox and comment leader NONE support
 * 	[91/08/07  12:27:53  damon]
 * 
 * 	Moved get_rc_value from def_build to par_rc_file
 * 	[91/04/12  15:47:05  damon]
 * 
 * 	Added changes to support RIOS and aix
 * 	[91/01/22  13:00:29  mckeen]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:32  randyb]
 * 
 * Revision 1.7  90/10/07  20:03:57  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:58  gm]
 * 
 * Revision 1.6  90/09/13  12:30:42  devrcs
 * 	Added TRUE/FALSE definitions.  Updated parser to handle "replace"
 * 	and "on <machine>" in either order.
 * 	[90/08/30  09:36:16  gm]
 * 
 * Revision 1.5  90/08/24  13:47:49  devrcs
 * 	Fixed defect which did not allow an rc variable to be redefined in
 * 	terms of itself.
 * 
 * 	Changed how the setenv key word operates.  It now will not set the
 * 	environment variable if it is already set unless the key word
 * 	"replace" is in front of it.  This matches the syntax of the rc
 * 	variables.
 * 
 * 	When this change hits the street; developers will need to have new
 * 	copies of their local and shared rc files which have "replace" in
 * 	the correct places.
 * 	[90/08/15  14:48:01  randyb]
 * 
 * Revision 1.4  90/08/09  14:23:30  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:32:32  gm]
 * 
 * Revision 1.3  90/07/17  12:36:57  devrcs
 * 	More changes for gcc.
 * 	[90/07/04  22:31:04  gm]
 * 
 * Revision 1.2  90/05/24  23:12:38  devrcs
 * 	Created from old "project_db.c" file.
 * 	[90/05/03  15:04:00  randyb]
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)99  1.1  src/bldenv/sbtools/libode/par_rc_file.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:44";
#endif /* not lint */

#include <unistd.h>
#include <sys/param.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef INC_TIME
#include <time.h>
#endif
#include <ode/parse_rc_file.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/util.h>

#define	INSERT	1
#define	REMOVE	2

/*
 * Prototypes
 */
STATIC void
free_args( struct arg_list *args_p );
int
rci_include( char **cnfg_ptr, char *file );

int
find_field( struct rcfile *rcfile_p, const char *field, struct field **field_pp,
            int flags )
{
    int i;
    const char *p;
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
    if ((field = strdup((char *)field)) == NULL)
	return(1);
    *field_pp = (struct field *) calloc(sizeof(char), sizeof(struct field));
    if (*field_pp == (struct field *)NULL)
	return(1);
    (*field_pp)->name = (char *)field;
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
void
free_field( struct field *field_p )
{
    struct arg_list *args_p;

    while ((args_p = field_p->args) != NULL) {
	field_p->args = args_p->next;
	free_args(args_p);
    }
    free(field_p->name);
    free((char *)field_p);
}

STATIC void
free_args( struct arg_list *args_p )
{
    while (--args_p->ntokens >= 0)
	free(args_p->tokens[args_p->ntokens]);
    free((char *)args_p->tokens);
    free((char *)args_p);
}

int
create_arglist( struct field *field_p, struct arg_list **args_pp )
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

STATIC int
find_arg( struct field *field_p, char *arg, struct arg_list **args_pp )
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

int
append_arg( char *arg, struct arg_list *args_p )
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
    if ((arg = strdup(arg)) == NULL)
	return(1);
    args_p->tokens[args_p->ntokens++] = arg;
    return(0);
}

STATIC int
lookupvar( struct rcfile *rcfile_p, char *name, char **valp )
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

STATIC int
evalarg( struct rcfile *rcfile_p, char **pp, char **ap )
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

STATIC int
parse_single_rc_file( FILE *rcfile, struct rcfile *rcfile_p )
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
	if (strchr("#\n", *buf) != NULL)
	    continue;
	if ((p = strrchr(buf, '\n')) != NULL)
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
	    if (getcwd(curpath, sizeof(curpath)) == NULL) {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
		return(1);
	    }
	    if (chdir(dirpath) < 0) {
		fprintf(stderr, "chdir %s: %s\n", dirpath, strerror(errno));
		return(1);
	    }
	    status = parse_single_rc_file(ifile, rcfile_p);
	    if (status != 0)
		fprintf(stderr, "error parsing rc description file %s\n",
				arg);
	    (void) fclose(ifile);
	    if (chdir(curpath) < 0) {
		fprintf(stderr, "chdir %s: %s\n", curpath, strerror(errno));
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
	    unsetenv(name);
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
	      if (find_field(rcfile_p, field, &field_p, INSERT))
		return(1);
	      tf_p = NULL;
/*
	      fprintf ( stderr, "field %s not found.\n", field );
	      return (1);
*/
	    } else {

	      memset ((char *)&nfield, 0, sizeof (nfield));
	      field_p = &nfield;
            } /* if */
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

int
parse_rc_file( char *rcpath, struct rcfile *rcfile_p )
{
    char curpath[MAXPATHLEN];
    char dirpath[MAXPATHLEN];
    char filepath[MAXPATHLEN];
    FILE *rcfile;
    int status;

    if ((rcfile = fopen (rcpath, "r")) == NULL ) {
      fprintf ( stderr, "fopen %s: %s\n", rcpath, strerror(errno));
      return (1);
    } /* if */
    path(rcpath, dirpath, filepath);
    if (getcwd(curpath, sizeof(curpath)) == NULL) {
	fprintf(stderr, "getcwd: %s\n", strerror(errno));
	return(1);
    }
    if (chdir(dirpath) < 0) {
	fprintf(stderr, "chdir %s: %s\n", dirpath, strerror(errno));
	return(1);
    }
    status = parse_single_rc_file(rcfile, rcfile_p);
    if (status != 0) {
	fprintf(stderr, "Error parsing rc description file %s\n",
		rcpath);
    }
    (void) fclose(rcfile);
    if (chdir(curpath) < 0) {
	fprintf(stderr, "chdir %s: %s\n", curpath, strerror(errno));
	return(1);
    }
    return(status);
}


        /* This function actually gets the keyword and makes sure
           it is legal.  It returns ERROR if not, OK if. */

int
get_rc_value (
    const char * keyword,                                /* word to look for */
    char     ** output,                         /* holds output from keyword */
    struct      rcfile  * contents,             /* holds contents of rc file */
    int         report_missing )                               /* error flag */
{
    struct      field   * fptr;                       /* holds rc field info */
    int         results = ERROR;                             /* return value */

  if ( find_field ( contents, keyword, &fptr, 0 ) ) {
    if ( report_missing)
      ui_print ( VWARN, "%s line missing in sandbox rc file.\n", keyword );
  } /* if */

  else if ( fptr->args->ntokens != 1 )
    ui_print ( VWARN, "impoper syntax in %s field in sandbox rc file.\n",
                      keyword );

  else if ( *fptr->args->tokens == NULL )
    ui_print ( VWARN, "no value for %s field in sandbox rc file %s.\n",
                      keyword );

  else if (( *output = strdup ( *fptr->args->tokens )) == NULL )
    ui_print ( VWARN, "strdup failure on space for %s.\n", *fptr->args->tokens);
  else
    results = OK;

  return ( results );
}                                                            /* get rc value */

void
free_rc_file( struct rcfile *rcfile_p )
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

int
rci_expand_include( char **cnfg_ptr, char *config )
{
    char *ptr;
    char *p, *q;
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
int
rci_include( char **cnfg_ptr, char *file )
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
    if ((p = strchr(buf, '\n')))
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

int
init_rc_contents ( 
    struct      rcfile  * rc_contents,             /* structure to initialze */
    char      * sbrc )                       /* file with source information */

        /* This procedure initializes the rc_contents structure. */


{
  memset ( (char *)rc_contents, 0, sizeof ( *rc_contents ));

  return ( parse_rc_file ( sbrc, rc_contents ) );
}
