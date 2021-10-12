static char sccsid[] = "@(#)93	1.11  src/bos/usr/bin/uucp/uuencode.c, cmduucp, bos41B, 9505A 1/25/95 14:04:06";
/* 
 * COMPONENT_NAME: UUCP uuencode.c
 * 
 * FUNCTIONS: ENC, MSGSTR, Muuencode, encode, fr, outdec 
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
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <nl_types.h>
#include	"uuencode_msg.h"
#include	<locale.h>
nl_catd catd;
#define	MSGSTR(n,s)	catgets(catd,MS_UUENCODE,n,s)

void usage();

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

main(argc, argv)
char **argv;
{
	FILE *in;
	struct stat sbuf;
	int mode;
	int num_arguments, ac, c, dashdash=0, i=1;
	int index[2];
	char *remotefile;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUENCODE,NL_CAT_LOCALE);

	/* 
	 * Do not allow any flags before the first operand.
	 * (Flags following the first operand are treated as operands.)
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
		/* Only save the first two operands for uuencode. */
		else if ((ac > 0) && (i < 3)) {
			index[i] = ac;
			i++;
		}
	}
	num_arguments = argc - dashdash;
	/* Quit if no valid operands or if more than 3 arguments. */
	if ((num_arguments == 1) || (num_arguments > 3))
		usage();

	/*
	 * The source file operand is optional.  If this operand is not
	 * specified, then set the input file to stdin and the remote file to
	 * the first operand.
	 */
	if (num_arguments > 2) {
		if ((in = fopen(argv[index[1]], "r")) == NULL) {
			perror(argv[index[1]]);
			exit(1);
		}
		remotefile = argv[index[2]];
	} else {
		in = stdin;
		remotefile = argv[index[1]];
	}

	/* figure out the input file mode */
	fstat(fileno(in), &sbuf);
	mode = sbuf.st_mode & 0777;
	fprintf(stdout, MSGSTR(BEGIN, "begin %o %s\n"), mode, remotefile);
	/* printf("begin %o %s\n", mode, remotefile);*/

	encode(in, stdout);

	fprintf(stdout, MSGSTR(END, "end\n"));
	/* printf("end\n"); */
	exit(0);
}

void
usage()
{
	fprintf(stderr, MSGSTR(USAGE, "Usage: uuencode [infile] remotefile\n"));
	exit(2);
}

/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	int i, n;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fr(in, buf, 45);
		putc(ENC(n), out);

		for (i=0; i<n; i += 3)
			outdec(&buf[i], out);

		putc('\n', out);
		if (n <= 0)
			break;
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec(p, f)
char *p;
FILE *f;
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	putc(ENC(c3), f);
	putc(ENC(c4), f);
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
