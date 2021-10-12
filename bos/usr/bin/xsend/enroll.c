static char sccsid[] = "@(#)69	1.8  src/bos/usr/bin/xsend/enroll.c, cmdmailx, bos411, 9428A410j 11/12/93 11:33:23";
/* 
 * COMPONENT_NAME: CMDMAILX enroll.c
 * 
 * FUNCTIONS: MSGSTR, Menroll 
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
#include "pwd.h"
#include "sys/types.h"

#include "xsend_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(n,s) catgets(catd,MS_xsend,n,s) 

MINT *a[42], *x, *b, *one, *c64, *t45, *z, *q, *r, *two, *t15;
char buf[256];
char maildir[] = { "/usr/spool/secretmail"};
main()
{
	int uid, i;
	FILE *fd;
	char *myname, fname[128];

        setlocale(LC_ALL,"");
	catd = catopen(MF_XSEND,NL_CAT_LOCALE);
	uid = getuid();
	myname = (char *) getlogin();
	if(myname == NULL)
		myname = getpwuid(uid)->pw_name;
	sprintf(fname, "%s/%s.key", maildir, myname);
	comminit();
	setup(getpass(MSGSTR(M_MSG_8, "Enter encryption key: ")));
	mkb();
	mkx();
#ifdef debug
	omout(b);
	omout(x);
#endif
	mka();
	i = creat(fname, 0644);
	if(i<0)
	{	perror(fname);
		exit(1);
	}
	close(i);
	fd = fopen(fname, "w");
	for(i=0; i<42; i++)
		nout(a[i], fd);
	exit(0);
}
