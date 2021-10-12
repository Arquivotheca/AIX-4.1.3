/* @(#)02	1.3  src/bos/usr/bin/mh/zotnet/mts.h, cmdmh, bos411, 9428A410j 10/9/90 16:50:56 */
/* 
 * COMPONENT_NAME: CMDMH mts.h
 * 
 * FUNCTIONS: isdlm1, isdlm2 
 *
 * ORIGINS: 26  28  35 
 *
 */
/* mts.h - definitions for the mail system */


/* Local and UUCP Host Name */

char   *LocalName (), *SystemName (), *UucpChan ();


/* Mailboxes */

extern char *mmdfldir,
            *mmdflfil,
            *uucpldir,
            *uucplfil;

#define	MAILDIR	(mmdfldir && *mmdfldir ? mmdfldir : (char *)getenv ("HOME"))
#define	MAILFIL	(mmdflfil && *mmdflfil ? mmdflfil : getusr ())
#define	UUCPDIR	(uucpldir && *uucpldir ? uucpldir : (char *)getenv ("HOME"))
#define	UUCPFIL	(uucplfil && *uucplfil ? uucplfil : getusr ())

char   *getusr (), *getfullname ();


/* Separators */

extern char *mmdlm1,
            *mmdlm2;

#define	isdlm1(s)	(strcmp (s, mmdlm1) == 0)
#define	isdlm2(s)	(strcmp (s, mmdlm2) == 0)


/* Filters */

extern char *umincproc;


/* Locking Directory */

#define	LOK_UNIX	0
#define	LOK_BELL	1
#define	LOK_MMDF	2

#ifndef	MMDFONLY
extern int   lockstyle;
#endif	MMDFONLY
extern char *lockldir;

int	lkopen (), lkclose ();
FILE   *lkfopen ();
int	lkfclose ();

/*  */

/* MTS specific variables */

#ifdef	MHMTS
extern char *Mailqdir;
extern char *TMailqdir;
extern int Syscpy;
extern char *Overseer;
extern char *Mailer;
extern char *Fromtmp;
extern char *Msgtmp;
extern char *Errtmp;
extern int Tmpmode;
extern char *Okhosts;
extern char *Okdests;
#endif	MHMTS

#ifdef	MMDFMTS
#endif	MMDFMTS

#ifdef	SENDMTS
extern char *hostable;
extern char *sendmail;
#endif SENDMTS


/* SMTP/POP stuff */

extern char *servers;
extern char *pophost;


/* BBoards-specific variables */

extern char *bb_domain;


/* POP BBoards-specific variables */

#ifdef	BPOP
extern char *popbbhost;
extern char *popbbuser;
extern char *popbblist;
#endif	BPOP


/* MailDelivery */

extern char *maildelivery;


/* Aliasing Facility (doesn't belong here) */

extern int Everyone;
extern char *NoShell;
