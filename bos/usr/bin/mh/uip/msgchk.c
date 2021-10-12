static char sccsid[] = "@(#)64	1.8  src/bos/usr/bin/mh/uip/msgchk.c, cmdmh, bos411, 9428A410j 11/9/93 09:42:43";
/* 
 * COMPONENT_NAME: CMDMH msgchk.c
 * 
 * FUNCTIONS: MSGSTR, Mmsgchk, checkmail, remotemail 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "msgchk.c	7.1 87/10/13 17:30:09"; */

/* msgchk.c - check for mail */

#include "mh.h"
#include <stdio.h>
#include "mts.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <locale.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	HELPSW	0
    "help", 4,

#ifdef	POP
#define	HOSTSW	1
    "host host", 0,
#define	USERSW	2
    "user user", 0,

#define	RPOPSW	3
    "rpop", 0,
#define	NRPOPSW	4
    "norpop", 0,
#endif	POP


    NULL, (int)NULL
};

/*  */

#define	NONEOK	0x0
#define	UUCPOLD	0x1
#define	UUCPNEW	0x2
#define	UUCPOK	(UUCPOLD | UUCPNEW)
#define	MMDFOLD	0x4
#define	MMDFNEW	0x8
#define	MMDFOK	(MMDFOLD | MMDFNEW)


#ifdef	POP
int	snoop = 0;
#endif	POP


#ifdef	SYS5
struct passwd	*getpwuid(), *getpwnam();
#endif	SYS5

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     vecp = 0;
#ifdef	POP
    int	    rpop = 1;
#endif	POP
    char   *cp,
#ifdef	POP
           *host = NULL,
#endif	POP
            buf[80],
	  **ap,
          **argp,
	   *arguments[MAXARGS],
           *vec[50];
    struct passwd  *pw;

        setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
#ifdef	POP
    if (pophost && *pophost)
	host = pophost;
    if ((cp = getenv ("MHPOPDEBUG")) && *cp)
	snoop++;
#endif	POP
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(USEUSERS, "%s [switches] [users ...]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

#ifdef	POP
		case HOSTSW: 
		    if (!(host = *argp++) || *host == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case USERSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    vec[vecp++] = cp;
		    continue;
		case RPOPSW: 
		    rpop++;
		    continue;
		case NRPOPSW: 
		    rpop = 0;
		    continue;
#endif	POP
	    }
	vec[vecp++] = cp;
    }

/*  */

#ifdef	POP
    if (!*host)
	host = NULL;
    if (!host || !rpop)
	(void) setuid (getuid ());
#endif	POP
    if (vecp == 0) {
#ifdef	POP
	if (host)
	    remotemail (host, NULLCP, rpop, 1);
	else
#endif	POP
	    if ((pw = getpwuid (getuid ())) == NULL)
		adios (NULLCP, MSGSTR(YOULOSE, "you lose")); /*MSG*/
	    else
		checkmail (pw, 1);
    }
    else {
	vec[vecp] = NULL;

	for (vecp = 0; cp = vec[vecp]; vecp++)
#ifdef	POP
	    if (host)
		remotemail (host, cp, rpop, 0);
	    else
#endif	POP
		if (pw = getpwnam (cp))
		    checkmail (pw, 0);
		else
		    advise (NULLCP, MSGSTR(NOUSER, "no such user as %s"), cp); /*MSG*/
    }

    done (0);
}

/*  */

checkmail (pw, personal)
register struct passwd  *pw;
int	personal;
{
    int     mf;
    char    buffer[BUFSIZ];
    struct stat st;

    (void) sprintf (buffer, "%s/%s",
	    mmdfldir[0] ? mmdfldir : pw -> pw_dir,
	    mmdflfil[0] ? mmdflfil : pw -> pw_name);
    mf = (stat (buffer, &st) == NOTOK || st.st_size == 0) ? NONEOK
	: st.st_atime <= st.st_mtime ? MMDFNEW : MMDFOLD;

#ifdef	MF
    if (umincproc != NULL && *umincproc != NULL) {
	(void) sprintf (buffer, "%s/%s",
		uucpldir[0] ? uucpldir : pw -> pw_dir,
		uucplfil[0] ? uucplfil : pw -> pw_name);
	mf |= (stat (buffer, &st) == NOTOK || st.st_size == 0) ? NONEOK
	    : st.st_atime <= st.st_mtime ? UUCPNEW : UUCPOLD;
    }
#endif	MF

    if ((mf & UUCPOK) || (mf & MMDFOK)) {
        if (personal) {
            if (mf & UUCPOK)
                if (mf & MMDFOK)
                    if (mf & UUCPOLD)
                        printf ((mf & MMDFOLD) ?
                        MSGSTR(MSG_3,"You have old old-style bell and old Internet mail waiting\n") :
                        MSGSTR(MSG_4,"You have old old-style bell and new Internet mail waiting\n"));
                    else
                        printf ((mf & MMDFOLD) ?
                        MSGSTR(MSG_5,"You have new old-style bell and old Internet mail waiting\n") :
                        MSGSTR(MSG_6,"You have new old-style bell and new Internet mail waiting\n"));
                else
                    printf ((mf & UUCPOLD) ?
                    MSGSTR(MSG_1,"You have old old-style bell mail waiting\n") :
                    MSGSTR(MSG_2,"You have new old-style bell mail waiting\n"));
            else if (mf & MMDFOK)
                printf ((mf & UUCPOLD) ?
                MSGSTR(MSG_7,"You have old Internet mail waiting\n") :
                MSGSTR(MSG_8,"You have new Internet mail waiting\n"));
        }
        else    /* !personal */
            if (mf & UUCPOK)
                if (mf & MMDFOK)
                    if (mf & UUCPOLD)
                        printf ((mf & MMDFOLD) ?
                        MSGSTR(MSG_11,"%s has old old-style bell and old Internet mail waiting\n"), pw->pw_name :
                        MSGSTR(MSG_12,"%s has old old-style bell and new Internet mail waiting\n"), pw->pw_name);
                    else
                        printf ((mf & MMDFOLD) ?
                        MSGSTR(MSG_13,"%s has new old-style bell and old Internet mail waiting\n"), pw->pw_name :
                        MSGSTR(MSG_14,"%s has new old-style bell and new Internet mail waiting\n"), pw->pw_name);
                else
                    printf ((mf & UUCPOLD) ?
                    MSGSTR(MSG_9,"%s has old old-style bell mail waiting\n"), pw->pw_name :
                    MSGSTR(MSG_10,"%s has new old-style bell mail waiting\n"), pw->pw_name);
            else if (mf & MMDFOK)
                printf ((mf & UUCPOLD) ?
                MSGSTR(MSG_15,"%s has old Internet mail waiting\n"), pw->pw_name :
                MSGSTR(MSG_16,"%s has new Internet mail waiting\n"), pw->pw_name);
        
    }
    else
            printf (personal ? 
            MSGSTR(MSG_17,"You don't have any mail waiting\n") :
            MSGSTR(MSG_18,"%s doesn't have any mail waiting\n"),pw->pw_name);
}

/*  */

#ifdef	POP
extern	char response[];


remotemail (host, user, rpop, personal)
register char   *host;
char   *user;
int	rpop,
	personal;
{
    int     nmsgs,
            nbytes;
    char   *pass;

    if (rpop) {
	if (user == NULL)
	    user = getusr ();
	pass = getusr ();
    }
    else
	ruserpass (host, &user, &pass);

    if (pop_init (host, user, pass, snoop, rpop) == NOTOK
	    || pop_stat (&nmsgs, &nbytes) == NOTOK
	    || pop_quit () == NOTOK) {
	advise (NULLCP, "%s", response);
	return;
    }

    if (nmsgs) {
	if (personal)
	    (nmsgs > 1) ?
		printf(MSGSTR(MSG_23,
		    "You have %d messages (%d bytes) on %s\n"),
		    nmsgs, nbytes, host) :
		printf(MSGSTR(MSG_24,"You have 1 message (%d bytes) on %s\n"),
		    host);
	else
	    (nmsgs > 1) ?
		printf(MSGSTR(MSG_20,"%s has %d messages (%d bytes) on %s\n"),
		    user, nmsgs, nbytes, host) :
		printf(MSGSTR(MSG_21,"%s has 1 message (%d bytes) on %s\n"),
		   user, nbytes, host);
    }
    else
	(personal) ?
	    printf(MSGSTR(MSG_22,"You don't have any mail waiting on %s\n"),
	       host) :
	    printf(MSGSTR(MSG_19,"%s doesn't have any mail waiting on %s\n"),
	       user, host);
}
#endif	POP
