static char sccsid[] = "@(#)71	1.11  src/bos/usr/bin/xsend/xget.c, cmdmailx, bos411, 9428A410j 11/15/93 14:22:56";
/* 
 * COMPONENT_NAME: CMDMAILX xget.c
 * 
 * FUNCTIONS: MSGSTR, ck_key, decipher, files, icmp, newstr
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "xmail.h"
#include <sys/types.h>
#include <sys/dir.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/syspest.h>
#include <sysexits.h>

/* define BUGLPR variable */

BUGVDEF(mailbug, 0)

#include "xsend_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(n,s) catgets(catd,MS_xsend,n,s) 

char *myname;
int uid;
struct direct *dbuf;
char *maildir = "/usr/spool/secretmail/";
FILE *kf, *mf;
DIR *df;
MINT *a[42];
MINT *x, *b, *one, *t45, *z, *q, *r;
MINT *two, *t15, *mbuf;
char buf[256], line[128];
#define MXF 100
int fnum[MXF], fcnt;
struct stat stbuf;
static int ck_key(char *, MINT **);
main()
{	int i;
	char *p;
	struct passwd *pw;
	extern char *newstr();
	char *homedir;

        setlocale(LC_ALL,"");
	catd = catopen(MF_XSEND,NL_CAT_LOCALE);
	uid = getuid();
	myname = (char *)getlogin();
	if(myname == NULL)
		myname = getpwuid(uid)->pw_name;
	comminit();
	mbuf = itom(0);
	files();
	setup(getpass(MSGSTR(M_MSG_6, "Enter encryption key: ")));
	mkb();
	mkx();

	/* check if they entered the right key */
	mka();
	if (! ck_key(myname, a))
		exit(1);

#ifndef debug
	invert(x, b, x);
#else
	invert(x, b, z);
	mult(x, z, z);
	mdiv(z, b, q, z);
	omout(z);
	invert(x, b, x);
#endif
	for(i=0; i<fcnt; i++)
	{	sprintf(line, "%s%s.%d", maildir, myname, fnum[i]);
		if(stat(line, &stbuf)<0)
		{	perror(line);
			continue;
		}
		if(stbuf.st_size == 0)
		{	printf(MSGSTR(M_MSG_9, "zero length mail file\n"));
			unlink(line);
			continue;
		}
		if((mf = fopen(line, "r"))==NULL)
		{	perror(line);
			continue;
		}
		decipher(mf, stdout);
	cmnd:
		printf("\n? ");
		fgets(buf, sizeof(buf), stdin);
		if(feof(stdin)) exit(0);
		switch(buf[0])
		{
		case 'q':
			exit(0);
		case 'n':
		case 'd':
		case '\n':
			fclose(mf);
			unlink(line);
			break;
		case '!':
			system(buf+1);
			printf("!\n");
			goto cmnd;
		case 's':
		case 'w':
			rewind(mf);
			if(buf[1] == '\n' || buf[1] == '\0') {
				/* find user's home directory */
				pw = getpwnam(myname);
				if (pw == NULL) {
					fprintf(stderr, MSGSTR(M_MSG_31, "Unknown user %s. Can't create $HOME/mbox.\n"), myname);
					exit(EX_NOUSER);
				}
				homedir = newstr(pw->pw_dir);
				(void)strcpy(buf, "s ");
				(void)strcat(buf, homedir);
				(void)strcat(buf, "/mbox\n");
			}
			for(p = buf+1; isspace(*p); p++);
			p[strlen(p)-1] = 0;
			kf = fopen(p, "a");
			if(kf == NULL)
			{	perror(p);
				goto cmnd;
			}
			decipher(mf, kf);
			fclose(mf);
			fclose(kf);
			unlink(line);
			break;
		default:
			printf(MSGSTR(M_MSG_17, "Commands are:\n"));
			printf(MSGSTR(M_MSG_18, "q	quit, leaving unread messages\n"));
			printf(MSGSTR(M_MSG_19, "n	delete current message and display next\n"));
			printf(MSGSTR(M_MSG_20, "d	same as above\n"));
			printf(MSGSTR(M_MSG_21, "<CR>	same as above\n"));
			printf(MSGSTR(M_MSG_22, "!	execute shell command\n"));
			printf(MSGSTR(M_MSG_23, "s	save message in the named file or mbox\n"));
			printf(MSGSTR(M_MSG_24, "w	same as above\n"));
			printf(MSGSTR(M_MSG_25, "?	print this list\n"));
			goto cmnd;
		}
	}
	exit(0);
}
icmp(a, b) int *a, *b;
{
	return(*a - *b);
}
files()
{	int i;
	if((df = opendir(maildir)) == NULL)
	{	perror(maildir);
		exit(1);
	}
	strcpy(line, myname);
	strcat(line, ".%d");
	while ((dbuf = readdir(df)) != NULL) 
	{
		if(sscanf(dbuf->d_name, line, &i) != 1)
			continue;
		if(fcnt >= MXF)
			break;
		fnum[fcnt++] = i;
	}
	closedir(df);
	if(fcnt == 0)
	{	printf(MSGSTR(M_MSG_26, "No secret mail for %s\n"), myname);
		exit(0);
	}
	qsort(fnum, fcnt, sizeof(int), icmp);
}
decipher(u, w) FILE *u, *w;
{	int i;
	short a;
	for(;;)
	{	nin(mbuf, u);
		if(feof(u)) break;
		mult(mbuf, x, mbuf);
		mdiv(mbuf, b, q, mbuf);
		for(i=1; i<=3; i++)
		{	a = mbuf->val[i];
			putc(a&0177, w);
			a >>= 8;
			putc(a&0177, w);
		}
	}
}

static int ck_key(char *user, MINT **a)

/* this checks the user's key, as stored in the user.key file, against
   the entered key, as massaged into the a array, and returns non-zero
   iff they are identical */

{

#define A_SIZE (42)  /* number of elements in a */

MINT a_in;
char fname[128];
int i;
FILE *fp;


    /* open the user's key file */

    sprintf(fname, "%s%s.key", maildir, user);
    if (! (fp = fopen(fname, "r"))) {
	perror(fname);
	return(0);
    }

    /* read in the file, checking each record against each element of the
       a array */
    
    for (i = 0; i < A_SIZE; ++i) {
	if (! fread(&a_in.len, sizeof(int), 1, fp))  /* get the data length */
	    break;
	    
	BUGLPR(mailbug, 5,
	    ("ck_key: a[%d]->len = %d, a_in.len = %d\n",
	    i, a[i]->len, a_in.len));
	    
	if (a_in.len != a[i]->len)  /* lengths don't match */
	    break;
	if (! (a_in.val = (short *)malloc(a_in.len))) {  /* alloc for data */
	    perror(MSGSTR(M_MSG_28, "memory allocation error"));
	    i = -1;
	    break;
	}
	if (fread(a_in.val, sizeof(short), a_in.len, fp) != a_in.len) {
	    free(a_in.val);
	    break;
	}
	if (memcmp(a_in.val, a[i]->val, a_in.len)) {  /* data doesn't match */

	    BUGLPR(mailbug, BUGACT,
		("ck_key: data mismatch at element %d\n", i));
	    free(a_in.val);
	    break;
	}
	free(a_in.val);
    }

    BUGLPR(mailbug, 5,
	("ck_key: after read loop: i = %d\n", i));

    if (i < A_SIZE) {  /* error */
	if (i != -1)  /* bad key */
	    fprintf(stderr,
		MSGSTR(M_MSG_29, "you have entered the wrong encryption key for user %s\n"),
		user);
	else if (ferror(fp))
	    perror(MSGSTR(M_MSG_30, "error reading key file"));
	fclose(fp);
	return(0);
    }

    /* success */
    fclose(fp);
    return(1);
}

char *
newstr(s)
	char *s;
{
	char *p;
	
	p = malloc(strlen(s) + 1);
	if (p == NULL) {
		fprintf(stderr, 
			MSGSTR(M_MSG_28, "newstr: could not malloc memory"));
		exit(EX_OSERR);
	}
	(void)strcpy(p, s);
	return(p);
}
	
