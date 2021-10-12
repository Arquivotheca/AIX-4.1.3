static char sccsid[] = "@(#)03	1.5  src/bos/usr/bin/mkstr/mkstr.c, cmdmsg, bos411, 9428A410j 8/3/92 13:39:48";
/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: main, process, match, copystr, octdigit, inithash, hashit, fgetNUL
 *
 * ORIGINS: 26
 *
 *                  SOURCE MATERIALS
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


#include <stdio.h>

#define	ungetchar(c)	ungetc(c, stdin)

long	ftell();
char	*calloc();
/*
 * NAME:  mkstr - create a string error message file by massaging C source
 *
 * FUNCTION: 
 * Modified March 1978 to hash old messages to be able to recompile
 * without addding messages to the message file (usually)
 *
 * Program to create a string error message file
 * from a group of C programs.  Arguments are the name
 * of the file where the strings are to be placed, the
 * prefix of the new files where the processed source text
 * is to be placed, and the files to be processed.
 *
 * The program looks for 'error("' in the source stream.
 * Whenever it finds this, the following characters from the '"'
 * to a '"' are replaced by 'seekpt' where seekpt is a
 * pointer into the error message file.
 * If the '(' is not immediately followed by a '"' no change occurs.
 *
 * The optional '-' causes strings to be added at the end of the
 * existing error message file for recompilation of single routines.
 */

FILE	*mesgread, *mesgwrite;
char	*progname;
char	usagestr[] =	"usage: %s [ - ] mesgfile newprefix sourcefile ...\n";
char	name[100], *np;

main(argc, argv)
	int argc;
	char *argv[];
{
	char addon = 0;

	argc--; 
	progname = *argv++;
	if (argc > 1 && argv[0][0] == '-')
	{
		addon++; 
		argc--; 
		argv++;
	}
	if (argc < 3)
	{
		fprintf(stderr, usagestr, progname); 
		exit(1);
	}
	mesgwrite = fopen(argv[0], addon ? "a" : "w");
	if (mesgwrite == NULL)
	{
		perror(argv[0]); 
		exit(1);
	}
	mesgread = fopen(argv[0], "r");
	if (mesgread == NULL)
	{
		perror(argv[0]); 
		exit(1);
	}
	inithash();
	argc--; 
	argv++;
	strcpy(name, argv[0]);
	np = name + strlen(name);
	argc--; 
	argv++;
	do {
		strcpy(np, argv[0]);
		if (freopen(name, "w", stdout) == NULL)
			perror(name), exit(1);
		if (freopen(argv[0], "r", stdin) == NULL)
			perror(argv[0]), exit(1);
		process();
		argc--, argv++;
	} while (argc > 0);
	exit(0);
}

/*
 * NAME:  process
 *
 * FUNCTION:  check for error message, if error then move string.
 */
process()
{
	char *cp;
	int c;

	for (;;) {
		c = getchar();
		if (c == EOF)
			return;
		if (c != 'e') {
			putchar(c);
			continue;
		}
		if (match("error(")) {
			printf("error(");
			c = getchar();
			if (c != '"')
				putchar(c);
			else
				copystr();
		}
	}
}

/*
 * NAME: match
 *
 * FUNCTION:  compare two strings.
 */
match(ocp)
	char *ocp;
{
	char *cp;
	int c;

	for (cp = ocp + 1; *cp; cp++) {
		c = getchar();
		if (c != *cp) {
			while (ocp < cp)
				putchar(*ocp++);
			ungetchar(c);
			return (0);
		}
	}
	return (1);
}

/*
 * NAME: copystr
 *
 * FUNCTION:  check for special characters then copy string.
 */
copystr()
{
	int c, ch;
	char buf[512];
	char *cp = buf;

	for (;;) {
		c = getchar();
		if (c == EOF)
			break;
		switch (c) {

		case '"':
			*cp++ = 0;
			goto out;
		case '\\':
			c = getchar();
			switch (c) {

			case 'b':
				c = '\b';
				break;
			case 't':
				c = '\t';
				break;
			case 'r':
				c = '\r';
				break;
			case 'n':
				c = '\n';
				break;
			case '\n':
				continue;
			case 'f':
				c = '\f';
				break;
			case '0':
				c = 0;
				break;
			case '\\':
				break;
			default:
				if (!octdigit(c))
					break;
				c -= '0';
				ch = getchar();
				if (!octdigit(ch))
					break;
				c <<= 7, c += ch - '0';
				ch = getchar();
				if (!octdigit(ch))
					break;
				c <<= 3, c+= ch - '0', ch = -1;
				break;
			}
		}
		*cp++ = c;
	}
out:
	*cp = 0;
	printf("%d", hashit(buf, 1, NULL));
}

/*
 * NAME:  octdigit
 *
 * FUNCTION: return true if character is and octal digit.
 */
octdigit(c)
	char c;
{

	return (c >= '0' && c <= '7');
}

/*
 * NAME: inithash
 *
 * FUNCTION:  the hash function is run on all input strings checking first
 *            for an error.
 */
inithash()
{
	char buf[512];
	int mesgpt = 0;

	rewind(mesgread);
	while (fgetNUL(buf, sizeof buf, mesgread) != NULL) {
		hashit(buf, 0, mesgpt);
		mesgpt += strlen(buf) + 2;
	}
}

#define	NBUCKETS	511

struct	hash {
	long	hval;
	unsigned hpt;
	struct	hash *hnext;
} *bucket[NBUCKETS];

/*
 * NAME: hashit
 *
 * FUNCTION: hash the string by finding all the matching strings and then 
 *           put sting into the correct bucket.
 */
hashit(str, really, fakept)
	char *str;
	char really;
	unsigned fakept;
{
	int i;
	struct hash *hp;
	char buf[512];
	long hashval = 0;
	char *cp;

	if (really)
		fflush(mesgwrite);
	for (cp = str; *cp;)
		hashval = (hashval << 1) + *cp++;
	i = hashval % NBUCKETS;
	if (i < 0)
		i += NBUCKETS;
	if (really != 0)
		for (hp = bucket[i]; hp != 0; hp = hp->hnext)
		if (hp->hval == hashval) {
			fseek(mesgread, (long) hp->hpt, 0);
			fgetNUL(buf, sizeof buf, mesgread);
/*
			fprintf(stderr, "Got (from %d) %s\n", hp->hpt, buf);
*/
			if (strcmp(buf, str) == 0)
				break;
		}
	if (!really || hp == 0) {
		hp = (struct hash *) calloc(1, sizeof *hp);
		hp->hnext = bucket[i];
		hp->hval = hashval;
		hp->hpt = really ? ftell(mesgwrite) : fakept;
		if (really) {
			fwrite(str, sizeof (char), strlen(str) + 1, mesgwrite);
			fwrite("\n", sizeof (char), 1, mesgwrite);
		}
		bucket[i] = hp;
	}
/*
	fprintf(stderr, "%s hashed to %ld at %d\n", str, hp->hval, hp->hpt);
*/
	return (hp->hpt);
}

#include <sys/types.h>
#include <sys/stat.h>

/*
 * NAME: fgetNUL
 *
 * FUNCTION: gets at most rmdr characters and checks for end of file or 
 *           file error.
 */
fgetNUL(obuf, rmdr, file)
	char *obuf;
	int rmdr;
	FILE *file;
{
	int c;
	char *buf = obuf;

	while (--rmdr > 0 && (c = getc(file)) != 0 && c != EOF)
		*buf++ = c;
	*buf++ = 0;
	getc(file);
	return ((feof(file) || ferror(file)) ? (int)NULL : 1);
}
