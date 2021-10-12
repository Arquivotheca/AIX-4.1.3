static char sccsid[] = "@(#)97  1.2  src/bldenv/genpath/genpath.c, bldprocess, bos412, GOLDA411a 2/3/93 16:34:42";

/*
 *   COMPONENT_NAME: BOSBUILD
 *
 *   FUNCTIONS: canonicalize
 *		concat
 *		defined
 *		expand_flag
 *		main
 *		path_relative_to
 *		print_revision
 *		print_usage
 *		salloc
 *		vconcat
 *		
 *
 *   ORIGINS: 71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * @OSF_FREE_COPYRIGHT@
 * 
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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: genpath.c,v $ $Revision: 1.7 $ (OSF) $Date: 1991/12/05 20:41:20 $";
#endif
/*
 * program to generate command flags for sandboxes
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifndef _BLD
#include <ode/parse_rc_file.h>
#endif

extern int errno;

extern char *index();
extern char *rindex();
extern char *malloc();
extern char *salloc();
extern char *getenv();

#if __STDC__
char *concat(char *buf, int buflen, ...);
#else
char *concat();
#endif
char *vconcat();

char **expand_flag();
char *path_relative_to();

char *progname;		/* program name */

print_usage()
{
    fprintf(stderr, "usage: %s [ options ] [ switches ... ]\n", progname);
    fprintf(stderr, "options include:\n");
    fprintf(stderr, "    -rc <rc_file>\n");
    fprintf(stderr, "    -sb <sandbox_name>\n");
    fprintf(stderr, "    -sb_rc <sandbox_rc_file>\n");
    fprintf(stderr, "    -usage | -version\n");
    fprintf(stderr, "    -verbose\n");
    exit(1);
}


/* show the revision of this program */
print_revision()
{
    printf("%s $Revision: 1.7 $ $Date: 1991/12/05 20:41:20 $\n", progname);
}


main(argc, argv)
    int argc;
    char *argv[];
{
    char *rcfile_source_base;
    char *rcfile_object_base;
    char *sandbox = NULL;
    char *basedir = NULL;
    char *sb_rcfile = NULL;
    char *usr_rcfile = NULL;
    char search_path[1024*2];
    char source_base[1024];
    char sourcebase[1024];
    char sourcedir[1024];
    char curdir[1024];
    int curdir_len;
    char *relpath;
    int i, j, k;
    char ch,*b,*p,*p2,*p3,**nargv;
    int nargc;
    char buf[1024+2];
    char ibuf[1024+2];
    int verbose = 0;
    char *space;
#ifndef _BLD
    struct rcfile rcfile;
    struct field *field_p;
    struct arg_list *args_p;
#endif
    int read_rcfile;
    int in_objdir;

    if (argc > 0) {
	if ((progname = rindex(argv[0], '/')) != NULL)
	    progname++;
	else
	    progname = argv[0];
	argc--; argv++;
    } else
	progname = "genpath";

#ifndef _BLD
    bzero(&rcfile, sizeof(struct rcfile));
#endif
    read_rcfile = 0;

    while (argc > 0) {
	if (strcmp(argv[0], "-version") == 0)
	{
	    print_revision();
	    exit(0);
	}
	if (strncmp(argv[0], "-v", strlen("-v")) == 0) {
	    verbose++;
	    argv++;
	    argc--;
	    continue;
	}
	if (strcmp(argv[0], "-sb") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			progname, argv[0]);
		print_usage();
	    }
	    argc--;
	    argv++;
	    sandbox = argv[0];
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-sb_rc") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			progname, argv[0]);
		print_usage();
	    }
	    argc--;
	    argv++;
	    sb_rcfile = argv[0];
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-rc") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			progname, argv[0]);
		print_usage();
	    }
	    argc--;
	    argv++;
	    usr_rcfile = argv[0];
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-usage") == 0)
	    print_usage();
	break;
    }

    if (argc == 0)
	exit(0);

    for (i = 0; i < argc; i++) {
	if (argv[i][0] != '-') {
	    fprintf(stderr, "%s: argument %s is not a switch\n",
		    progname, argv[i]);
	    exit(1);
	}
	if (argv[i][1] != 'I' && argv[i][1] != 'L') {
	    fprintf(stderr, "%s: switch %s not recognized\n",
		    progname, argv[i]);
	    exit(1);
	}
    }

    if (read_rcfile ||
	(b = getenv("SOURCEBASE")) == NULL ||
	(p = getenv("SOURCEDIR")) == NULL) {
#ifdef _BLD
	fprintf(stderr, "%s: SOURCEBASE or SOURCEDIR not defined\n", progname);
	exit(1);
#else	/* _BLD */
	read_rcfile = 1;
	if (current_sb (&sandbox, &basedir, &sb_rcfile, &usr_rcfile) != 0) {
	    fprintf(stderr, "%s: unable to get sandbox basedir\n", progname);
	    exit(1);
	}
	if (parse_rc_file(sb_rcfile, &rcfile) != 0) {
	    fprintf(stderr, "%s: unable to parse %s sandbox description\n",
		    progname, sandbox);
	    exit(1);
	}
	if (rc_file_field(&rcfile, "source_base", &field_p) != 0) {
	    fprintf(stderr, "%s: source_base not defined\n", progname);
	    exit(1);
	}
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    fprintf(stderr, "%s: improper source_base\n", progname);
	    exit(1);
	}
	rcfile_source_base = args_p->tokens[0];
	if (*rcfile_source_base != '/') {
	    fprintf(stderr, "%s: source_base is not an absolute path\n", progname);
	    exit(1);
	}
	if (rc_file_field(&rcfile, "object_base", &field_p) != 0) {
	    fprintf(stderr, "%s: object_base not defined\n", progname);
	    exit(1);
	}
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    fprintf(stderr, "%s: improper object_base\n", progname);
	    exit(1);
	}
	rcfile_object_base = args_p->tokens[0];
	if (*rcfile_object_base != '/') {
	    fprintf(stderr, "%s: object_base is not an absolute path\n", progname);
	    exit(1);
	}
	if ((b = getenv("SOURCEBASE")) == NULL) {
	    fprintf(stderr, "%s: SOURCEBASE is not defined\n", progname);
	    exit(1);
	}
	if ((p = getenv("SOURCEDIR")) == NULL) {
	    fprintf(stderr, "%s: SOURCEDIR is not defined\n", progname);
	    exit(1);
	}
#endif	/* _BLD */
    }
    (void) strcpy(source_base, b);
    if (verbose)
	fprintf(stderr, "[ SOURCEBASE %s ]\n", b);
    (void) strcpy(sourcedir, p);
    if (verbose)
	fprintf(stderr, "[ SOURCEDIR %s ]\n", p);

    if (getwd(curdir) == NULL) {
	fprintf(stderr, "%s: getwd .: %s\n", progname, curdir);
	exit(1);
    }
    curdir_len = strlen(curdir);
    if (curdir_len == 0 || curdir[0] != '/') {
	fprintf(stderr, "%s: getwd returned bad directory \"%s\"\n",
		progname, curdir);
	exit(1);
    }
    if (verbose)
	fprintf(stderr, "[ curdir %s ]\n", curdir);

    if (read_rcfile || (p = getenv("OBJECTDIR")) != NULL) {
	if (read_rcfile)
	    p = rcfile_object_base;
	if (verbose)
	    fprintf(stderr, "[ OBJECTDIR %s ]\n", p);
	if (*p != '/') {
	    canonicalize(source_base, p, buf, sizeof(buf));
	    p = buf;
	}
	if (verbose)
	    fprintf(stderr, "[ object_base %s ]\n", p);
	relpath = path_relative_to(p, curdir, curdir_len);
	in_objdir = (relpath != NULL);
    } else
	in_objdir = 0;
    if (!in_objdir)
	relpath = path_relative_to(source_base, curdir, curdir_len);
    if (relpath == NULL) {
	fprintf(stderr, "%s: unable to find path within sandbox\n", progname);
	return(1);
    }
    if (verbose)
	fprintf(stderr, "[ relative path %s ]\n", relpath);

    b = source_base;
    p = sourcedir;
    if (*p == '\0')
	(void) concat(search_path, sizeof(search_path), ":", b, relpath, NULL);
    else if (*relpath == '\0')
	(void) concat(search_path, sizeof(search_path), ":", b, ":", p, NULL);
    else {
	p3 = concat(search_path, sizeof(search_path), ":", b, relpath, NULL);
	for (;;) {
	    p2 = p;
	    while (*p && *p != ':')
		p++;
	    ch = *p;
	    *p = '\0';
	    if (*p2 != '/') {
		fprintf(stderr, "%s: SOURCEDIR contains a relative path\n",
			progname);
		exit(1);
	    }
	    p3 = concat(p3, search_path + sizeof(search_path) - p3,
			":", p2, relpath, NULL);
	    if (ch == '\0')
		break;
	    *p++ = ch;
	}
    }
    if (verbose)
	fprintf(stderr, "[ search_path %s ]\n", search_path);

    nargc = argc;
    nargv = (char **)malloc(nargc*sizeof(char *));
    j = 0;	/* argument counter for new arg list */

    for (i = 0; i < argc; i++) {
	if (argv[i][2] == '/') {
	    nargv[j++] = argv[i];
	    continue;
	}
	nargv = expand_flag(&j, &nargc, nargv, argv[i], search_path);
    }

    space = "";
    for (i = 0; i < j; i++) {
	printf("%s%s", space, nargv[i]);
	space = " ";
    }
    if (j > 0)
	printf("\n");
    exit(0);
}

/*
 * canonicalize path - similar to abspath
 */
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
	fprintf(stderr, "%s: path length exceeds buffer size\n");
	return(-1);
    }

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
}

char **
expand_flag(j, nargc, nargv, rel, search_path)
    int *j;
    int *nargc;
    char **nargv;
    char *rel;
    char *search_path;
{
    char ibuf[1024+2];
    char ch;
    char *p;
    char *p2;
    char *p3;

    ibuf[0] = *rel++;
    ibuf[1] = *rel++;
    p = search_path;
    for (;;) {
	p2 = p;
	while (*p && *p != ':')
	    p++;
	ch = *p;
	*p = '\0';
	p3 = NULL;
	if (*p2 == '\0' || (*p2 == '.' && *(p2+1) == '\0'))
	    p3 = rel;
	else if (*rel == '\0' || (*rel == '.' && *(rel+1) == '\0'))
	    p3 = p2;
	if (p3 == NULL)
	    canonicalize(p2, rel, ibuf+2, sizeof(ibuf)-2);
	else if (*p3 == '/')
	    canonicalize("", p3, ibuf+2, sizeof(ibuf)-2);
	else {
	    canonicalize("", p3, ibuf+1, sizeof(ibuf)-1);
	    ibuf[1] = *(rel-1);
	    if (ibuf[2] == '\0') {
		ibuf[2] = '.';
		ibuf[3] = '\0';
	    }
	}
	(*nargc)++;
	nargv = (char **)realloc((char *)nargv, (*nargc)*sizeof(char *));
	nargv[(*j)++] = salloc(ibuf);
	if (ch == '\0')
	    break;
	*p++ = ch;
    }
    return(nargv);
}

/*
 * If we are within the directory subtree of base_dir, return the path
 * from there to the current directory.  Otherwise, return NULL.
 */
char *
path_relative_to(base_dir, curdir, curdir_len)
char *base_dir, *curdir;
int curdir_len;
{
    char errbuf[BUFSIZ];
    int save_errno;
    char basedir[1024];
    char *path;
    int len;

    if (chdir(base_dir) < 0) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: chdir %s", progname, base_dir);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    if (getwd(basedir) == NULL) {
	fprintf(stderr, "%s: getwd %s: %s\n", progname, base_dir, basedir);
	exit(1);
    }
    if (chdir(curdir) < 0) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: chdir %s", progname, base_dir);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    len = strlen(basedir);
    if (len == 0 || basedir[0] != '/') {
	fprintf(stderr, "%s: getwd returned bad base directory \"%s\"\n",
		progname, basedir);
	exit(1);
    }
    if (curdir_len < len)
	return(NULL);
    if (bcmp(basedir, curdir, len) != 0)
	return(NULL);
    if (curdir[len] != '\0' && curdir[len] != '/')
	return(NULL);
    if ((path = salloc(curdir+len)) == NULL) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: salloc relative path", progname);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    return(path);
}

#ifdef _BLD
char *salloc(p)
char *p;
{
	register char *q;
	register int l;

	q = malloc(l = strlen(p) + 1);
	if (q != 0)
		bcopy(p, q, l);
	return(q);
}

/*VARARGS2*/
char *
#if __STDC__
concat(char *buf, int buflen, ...)
#else
concat(va_alist)
va_dcl
#endif
{
#if !__STDC__
    char *buf;
    int buflen;
#endif
    va_list ap;
    char *ptr;

#if __STDC__
    va_start(ap, buflen);
#else
    va_start(ap);
    buf = va_arg(ap, char *);
    buflen = va_arg(ap, int);
#endif
    ptr = vconcat(buf, buflen, ap);
    va_end(ap);
    return(ptr);
}

char *
vconcat(buf, buflen, ap)
char *buf;
int buflen;
va_list ap;
{
    register char *arg, *ptr, *ep;

    if (buf == NULL)
	return(NULL);
    if (buflen <= 0)
	return(NULL);
    ptr = buf;
    *ptr = '\0';
    ep = buf + buflen;
    while (ptr != NULL && (arg = va_arg(ap, char *)) != NULL)
	while (*ptr = *arg++)
	    if (++ptr == ep) {
		ptr = NULL;
		break;
	    }
    return(ptr);
}
#endif	/* _BLD */
