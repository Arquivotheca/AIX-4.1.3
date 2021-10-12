static char sccsid[] = "@(#)73	1.11  src/bos/usr/bin/xsend/xsend.c, cmdmailx, bos411, 9428A410j 11/15/93 14:24:32";
/* 
 * COMPONENT_NAME: CMDMAILX xsend.c
 * 
 * FUNCTIONS: MSGSTR, Mxsend, encipher, init, mkcd, run 
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
#include <pwd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>
#include <ctype.h>


#include "xsend_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(n,s) catgets(catd,MS_xsend,n,s) 

extern int errno;
struct stat stbuf;
int uid, destuid;
char *myname, *dest, keyfile[128], line[128];
struct direct *dbuf;
char *maildir = "/usr/spool/secretmail/";
FILE *kf, *mf;
DIR *df;
MINT *a[42], *cd[6][128];
MINT *msg;
char buf[256], eof;
int dbg;
extern char *malloc(), *getlogin();

main(argc, argv) char **argv;
{	int i, nmax, len;
	char *p;
	long now;
            
        setlocale(LC_ALL,"");
	catd = catopen(MF_XSEND,NL_CAT_LOCALE);
	if (argc < 2)
		xfatal(MSGSTR(M_SUSAGE, "usage: xsend user_name"));
	if (argc > 2)
		xfatal(MSGSTR(M_MSG_2, "you can only mail to one person") );
	uid = getuid();
	p =getlogin();
	if(p == NULL)
		p = getpwuid(uid)->pw_name;
	myname = malloc(strlen(p)+1);
	strcpy(myname, p);
	dest = argv[1];
	strcpy(keyfile, maildir);
	strcat(keyfile, dest);
	strcat(keyfile, ".key");
	if(stat(keyfile, &stbuf) <0)
		if (errno == ENOENT)
		    xfatal(MSGSTR(M_MSG_3,
"specified recipient is not enrolled (they must use the enroll command)") );
		else {
		    perror(MSGSTR(M_MSG_5,
			"error accessing recipient's key file") );
		    exit(1);
		}
	destuid = getpwnam(dest)->pw_uid;
	if(destuid != stbuf.st_uid)
		fprintf(stderr,  MSGSTR(M_MSG_4,
		    "warning: recipient's key file may be subverted\n") );
	errno = 0;
	kf = fopen(keyfile, "r");
	if(kf == NULL) {
		perror(MSGSTR(M_MSG_5, "error accessing recipient's key file"));
		exit(1);
	}
	df = opendir(maildir);
	if(df == NULL)
	{	perror(maildir);
		exit(1);
	}
	strcpy(line, dest);
	strcat(line, ".%d");
	nmax = -1;
	while ((dbuf=readdir(df))!=NULL)
	{	if(sscanf(dbuf->d_name, line, &i) != 1)
			continue;
		if(i>nmax) nmax = i;
	}
	nmax ++;
	for(i=0; i<10; i++)
	{	sprintf(line, "%s%s.%d", maildir, dest, nmax+i);
		if(creat(line, 0666) >= 0) break;
	}
	if(i==10) {
	    perror(MSGSTR(M_MSG_7, "cannot create mail file"));
	    exit(1);
	}
	mf = fopen(line, "w");
	init();
	time(&now);
	sprintf(buf, "From %s %s", myname, ctime(&now) );
#ifdef DBG
	dbg = 1;
#endif
	run();
	{
		char	hostname[32];
		FILE	*nf;
		struct	passwd	*passp;

		sprintf(buf, "/bin/bellmail %s", dest);
		if ((nf = popen(buf, "w")) == NULL) {
			perror(MSGSTR(M_MSG_10,
			    "cannot pipe to /bin/bellmail") );
			exit(1);
		}
		passp = getpwuid(getuid());
		if (passp == 0){
			pclose(nf);
			xfatal(MSGSTR(M_MSG_11, "cannot get your username") );
		}
		gethostname(hostname, sizeof(hostname));
		fprintf(nf, MSGSTR(M_MSG_12,
		    "Subject: secret mail from %s@%s\n"),
			passp->pw_name, hostname);
		fprintf(nf, MSGSTR(M_MSG_13,
"Your secret mail can be read on host %s using the \"xget\" command.\n"),
			hostname);
		pclose(nf);
	}
	exit(0);
}
mkcd()
{	int i, j, k, n;
	for(i=0; i<42; i++)
		nin(a[i], kf);
	fclose(kf);
	for(i=0; i<6; i++)
	for(j=0; j<128; j++)
		for(k=j, n=0; k>0 && n<7; n++, k>>=1)
			if(k&01) madd(cd[i][j], a[7*i+n], cd[i][j]);
}
encipher(s) char s[6];
{	int i;
	msub(msg, msg, msg);
	for(i=0; i<6; i++)
		madd(msg, cd[i][toascii(s[i]&0377)], msg);
}
init()
{	int i, j;
	msg = itom(0);
	for(i=0; i<42; i++)
		a[i] = itom(0);
	for(i=0; i<6; i++)
	for(j=0; j<128; j++)
		cd[i][j] = itom(0);
	mkcd();
}
run()
{	char *p;
	int i, len, eof = 0;
	for(;;)
	{	len = strlen(buf);
		for(i=0; i<len/6; i++)
		{
			encipher(buf+6*i);
			nout(msg, mf);
		}
		p = buf;
		for(i *= 6; i<len; i++)
			*p++ = buf[i];
		if(eof) return;
		fgets(p, sizeof(buf)-6, stdin);
		if(strcmp(p, ".\n") == 0 || feof(stdin))
		{	for(i=0; i<6; i++) *p++ = ' ';
			*p = 0;
			eof = 1;
		}
	}
}
