static char sccsid[] = "@(#)92	1.12  src/bos/usr/bin/uucp/uudecode.c, cmduucp, bos41B, 9505A 1/25/95 14:04:04";
/* 
 * COMPONENT_NAME: UUCP uudecode.c
 * 
 * FUNCTIONS: DEC, MSGSTR, Muudecode, decode, fr, index, outdec 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * uudecode [input]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <nl_types.h>
#include	"uudecode_msg.h"
#include	<locale.h>
nl_catd catd;
#define	MSGSTR(n,s)	catgets(catd,MS_UUDECODE,n,s)

void usage();

/* single character decode */
#define DEC(c)	(((c) - ' ') & 077)

main(argc, argv)
char **argv;
{
	FILE *in, *out;
	int mode;
	char dest[128];
	char buf[80];
	int num_arguments, ac, c, dashdash=0, i=1;
	int index=0;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUDECODE,NL_CAT_LOCALE);

	/*
	 * Do not allow any flags before the first operand.
	 */
	while ((c = getopt(argc, argv, "")) != EOF) {
		switch (c) {
			default: usage();
		}
	}

	/*
	 * Have to handle "--" for XPG/4 compliance.
	 * This "--" can be anywhere on the command line following the
	 * command.
	 */
	for (ac=0; ac<argc; ac++) {
		if (strcmp(argv[ac], "--") == 0) {
			dashdash++;
		}
		/* Only save the first operand for uudecode. */
		else if ((ac > 0) && (i == 1)) {
			index = ac;
			i++;
		}
	}
	num_arguments = argc - dashdash;

	/*
	 * The input file operand is optional.  If this operand is not
	 * specified, then set the input file to stdin.
	 */
	if (num_arguments > 1) {
		if ((in = fopen(argv[index], "r")) == NULL) {
			perror(argv[index]);
			exit(1);
		}
	} else
		in = stdin;

	/* Only one operand allowed! */
	if (num_arguments > 2)
		usage();

	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULL) {
			fprintf(stderr, MSGSTR(NO_BEGIN,"No begin line\n"));
			/* fprintf(stderr, "No begin line\n"); */
			exit(3);
		}
		if (strncmp(buf, "begin ", 6) == 0)
			break;
	}
	sscanf(buf, "begin %o %s", &mode, dest);
	mode &= 0777;

	/* handle ~user/file format */
	if (dest[0] == '~') {
		char *sl;
		char *index();
		struct passwd *user;
		char dnbuf[100];

		sl = index(dest, '/');
		if (sl == NULL) {
			fprintf(stderr, MSGSTR(BAD_USER,"Illegal ~user\n"));
			/* fprintf(stderr, "Illegal ~user\n"); */
			exit(3);
		}
		*sl++ = 0;
		user = getpwnam(dest+1);
		if (user == NULL) {
			fprintf(stderr, MSGSTR(NO_USER, 
				"No such user as %s\n"), dest); 
			/* fprintf(stderr, "No such user as %s\n", dest); */
			exit(4);
		}
		strcpy(dnbuf, user->pw_dir);
		strcat(dnbuf, "/");
		strcat(dnbuf, sl);
		strcpy(dest, dnbuf);
	}

	/* create output file */
	out = fopen(dest, "w");
	if (out == NULL) {
		perror(dest);
		exit(4);
	}
	chmod(dest, mode);

	decode(in, out);

	if (fgets(buf, sizeof buf, in) == NULL || strcmp(buf, "end\n")) {
		fprintf(stderr, MSGSTR(NO_END,"No end line\n"));
		/* fprintf(stderr, "No end line\n"); */
		exit(5);
	}
	exit(0);
}

void
usage()
{
	fprintf(stderr, MSGSTR(USAGE,"Usage: uudecode [infile]\n"));
	/* printf("Usage: uudecode [infile]\n"); */
	exit(2);
}

/*
 * copy from in to out, decoding as you go along.
 */
decode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	char *bp;
	int n;

	for (;;) {
		/* for each input line */
		if (fgets(buf, sizeof buf, in) == NULL) {
			fprintf(stderr, MSGSTR(TOOSHORT,"Short file\n"));
			/* printf("Short file\n"); */
			exit(10);
		}
		n = DEC(buf[0]);
		if (n <= 0)
			break;

		bp = &buf[1];
		while (n > 0) {
			outdec(bp, out, n);
			bp += 4;
			n -= 3;
		}
	}
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
outdec(p, f, n)
char *p;
FILE *f;
{
	int c1, c2, c3;

	c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
	c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
	c3 = DEC(p[2]) << 6 | DEC(p[3]);
	if (n >= 1)
		putc(c1, f);
	if (n >= 2)
		putc(c2, f);
	if (n >= 3)
		putc(c3, f);
}


/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

/* #define	NULL	0 */

char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}
