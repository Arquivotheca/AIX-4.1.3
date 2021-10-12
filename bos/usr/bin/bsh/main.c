static char sccsid[] = "@(#)06	1.38.1.15  src/bos/usr/bin/bsh/main.c, cmdbsh, bos411, 9428A410j 4/22/94 18:13:06";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: main exfile chkpr settmp Ldup chkmail setmail
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.38.1.2  com/cmd/sh/sh/main.c, cmdsh, bos320, 9132320b 8/1/91 08:52:18
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include <sys/types.h>
#include <nl_types.h>
#include <unistd.h>
#include "defs.h"
#include "sym.h"
#include "timeout.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>    
#include <termios.h>
#include <locale.h>

/* tmpout is 7 for /tmp/sh, 6 for mktemp, up to 4 for serial (2000 possible),
 * up to 10 for pid (2 Gig possible), 1 for ending null = total possible 27 */
uchar_t         tmpout[27] = "/tmp/sh-";

static BOOL     beenhere = FALSE;
struct fileblk  stdfile;
struct fileblk	*standin = &stdfile;
int		mailchk = 0;
int		timeout = TIMEOUT;
int		timecroak;

static uchar_t	*mailp;
static time_t	*mod_time = 0;
static int	exfile();
extern uchar_t	*simple();
extern int	_open_max;

#ifndef _SHRLIB /* NOT shared library */
extern void     setup_mem();
#endif /* NOT _SHRLIB */

nl_catd		catd;           /* message catalog descriptor */

main(int c, uchar_t *v[], uchar_t *e[])
{
    register uchar_t	*shname;
    register int	rflag = ttyflg;
    int			rsflag = 1;     /* local restricted flag */
    struct namnod	*n;
    int			fd_tty;            /* fd of /dev/tty    */
    extern void		stdsigs();
    extern int		setsig();

    if(**v == '-')
        stdsigs();
    else
        if((c == 1) || (strcmp(v[1], "-c") != 0))
            specsigs();

    setlocale (LC_ALL, "");

#ifndef _SHRLIB /* NOT shared library */

    /* initialize storage allocation */
    setup_mem();

#endif /* NOT _SHRLIB */

    catd = catopen(MF_BSH, NL_CAT_LOCALE);

    /* setup for max number of file pointers */
    if( (_open_max = sysconf(_SC_OPEN_MAX)) == -1) {
        perror(MSGSTR(M_SYSCONF,"bsh: sysconf error"));
        exit(ERROR);
    }

    if( (fdmap = (struct fdsave *)malloc( 
        _open_max * (sizeof(struct fdsave)))) == NULL) {
        perror(MSGSTR(M_NOMEM,"bsh: no memory"));
        exit(ERROR);
    }

    /* set names from userenv (ignoring environment passed in) */
    setup_env();

    /*
     * 'rsflag' is non-zero if SHELL variable is set in the
     *  environment and has a simple name of 'Rsh' or '-Rsh'.
     *  'rsh' or '-rsh' also allowed for historical
     *  compatability (this is now the AIX remote shell)
     */
    if ((n = findnam("SHELL")) && n->namval != NULL) {
        if (*(shname = simple(n->namval)) == '-')
            ++shname;
        if ((strcmp(shname,"rsh") == 0) ||
            (strcmp(shname,"Rsh") == 0))
            rsflag = 0;
    }

    /*
     * a shell is also restricted if argv(0) has
     * a simple name of 'Rsh' or '-Rsh'.
     *  'rsh' or '-rsh' also allowed for historical
     *  compatability (this is now the AIX remote shell)
     */
    if (c > 0) {
        if (*(shname = simple(*v)) == '-')
            ++shname;
        if ((strcmp(shname,"rsh") == 0) ||
            (strcmp(shname,"Rsh") == 0))
            rflag = 0;
    }

    hcreate();
    set_dotpath();

    /*
     * look for options
     * dolc is $#
     */
    eflag = 0;    /* fail on error flag */
    dolc = options(c, v);

    if (dolc < 2) {
        flags |= stdflg;
        {
            register uchar_t	*flagc = flagadr;

            while (*flagc)
                flagc++;
            *flagc++ = STDFLG;
            *flagc = 0;
        }
    }
    if ((flags & stdflg) == 0)
        dolc--;

    dolv = v + c - dolc;
    dolc--;

    /*
     * return here for shell file execution
     * but not for parenthesis subshells
     */
    setjmp(subshell);

    /* number of positional parameters */
    replace(&cmdadr, dolv[0]);      /* cmdadr is $0 */

    /* set pidname '$$' */
    assnum(&pidadr, getpid());

    /* set up temp file names */
    serial = 0; /* initialize check number for tmpname */
    (void)settmp();

    /* default internal field separators - $IFS (space, tab, newline) */
    dfault(&ifsnod, sptbnl);

    /* default check mail time of 10 minutes */
    dfault(&mchknod, MAILCHECK);
    mailchk = stoi(mchknod.namval);

    /* shell timeout value, a value of zero indicates no limit */
    if(timenod.namval)
        timeout = stoi(timenod.namval);

    /* catch longjmps that are not caught elsewhere */
    if (setjmp(errshell)) {
        exit(ERROR);
    }

    /* initialize OPTIND for getopt */
    n = lookup("OPTIND");
    assign(n, "1");

    if ((beenhere++) == FALSE) {    /* ? profile */
        if (*(simple(cmdadr)) == '-') { /* system profile */
            if ((input = pathopen(nullstr, sysprofile)) >= 0)
                exfile(rflag);          /* file exists */

            if ((input = pathopen(nullstr, profile)) >= 0) {
                exfile(rflag);
                flags &= ~ttyflg;
            }
        }

        if (rsflag == 0 || rflag == 0)
            flags |= rshflg;

        /* open input file if specified */
        if (comdiv) {
            estabf(comdiv);
            input = -1;
        }
        else {
            /* Set cmdadr to argv[0] so if chkopen fails
             * it will print "sh: file: not found"
            */
            uchar_t	*oldcmd = cmdadr;
            cmdadr = v[0];
            input = ((flags & stdflg) ? 0 : chkopen(oldcmd));

#ifdef ACCT
            if (input != 0)
                preacct(oldcmd);
#endif
            cmdadr = oldcmd;
            comdiv--;
        }
    }

    exfile(0);
    done();
}

long			mailtime;       /* last time mail was checked */

static int
exfile(BOOL prof)
{
    jmp_buf		savebuf;
    time_t		curtime = 0;
    register int	userid;
    int			needtoexit = 0;

    /* preserve the previous longjmp context */
    memcpy(savebuf,errshell,sizeof(savebuf));

    /* move input to be next available file descriptor dynamically */
    if (input > 0)
        input = Ldup(input, INIO);

    userid = geteuid();

    /* decide whether interactive */
    if ((flags & intflg) ||
        ((flags&oneflg) == 0 &&
        isatty(output) &&
        isatty(input)) ) {

        uchar_t		*stdprompt, *supprompt;
        
        stdprompt = shstdprompt;
        supprompt = shsupprompt;
        
        dfault(&ps1nod, (userid ? stdprompt : supprompt));
        dfault(&ps2nod, readmsg);
        flags |= ttyflg | prompt;
        ignsig(SIGTERM);

        if (mailpnod.namflg != N_DEFAULT)
            setmail(mailpnod.namval);
        else
            setmail(mailnod.namval);
    }
    else {
        flags |= prof;
        flags &= ~prompt;
    }

    if (setjmp(errshell) && prof) {
        memcpy(errshell,savebuf,sizeof(errshell));
        close(input);
        return;
    }
    /* error return here */

    loopcnt = peekc = peekn = 0;
    fshift = 0;
    fndef = 0;
    nohash = 0;
    iopend = 0;

    if (input >= 0)
        initf(input);
    /* command loop */
    for (;;) {
        tdystak(0);
        stakchk();      /* may reduce sbrk */
        exitset();

        if ((flags & prompt) && standin->fstak == 0 && !eof) {
            timecroak = 0;
            if (mailp || timeout) {
                time(&curtime);
                if(timeout)
                    timecroak = curtime + 60*timeout;

                if (mailp && (curtime - mailtime) >= mailchk) {
                    (void)chkmail();
                    mailtime = curtime;
                }
            }

            prs(ps1nod.namval);

            if(timeout || mailp && mailchk)
                alarm((mailchk && (mailchk < 60 * timeout)) ? 
                       mailchk : 60 * timeout);

            flags |= waiting;
        }

        trapnote = 0;
        peekc = readc();
        if (eof)
            break;

        alarm(0);

        flags &= ~waiting;

        /*
         * If "oneflg" is set, then we will need to exit.  This is
         * done before calling "execute()" because the user could set
         * "oneflg" (with 'set -t'), in which case one more command
         * needs to be executed before we exit.
         */
        if (flags & oneflg)
            needtoexit++;

        execute(cmd(NL, MTFLG), 0, eflag);
        if (needtoexit)
            eof++;
    }
    /* restore the previous longjmp context */
    memcpy(errshell,savebuf,sizeof(errshell));
}

void
chkpr()
{
    if ((flags & prompt) && standin->fstak == 0)
        prs(ps2nod.namval);
}

int
settmp()
{
    /* Changed manner in which tmp file name is generated 
     * - old method used pid, then used mktemp, now uses
     * /tmp/sh followed by mktemp name followed by unique
     *  check digit followed by pid id of shell
    */
    strcpy ( numbuf , "XXXXXX" );    /*  init numbuf for call to mktemp  */
    mktemp ( numbuf );

    /* add mktemp output beyond the /tmp/sh portion of tmpname */
    tempname = movstr(numbuf, &tmpout[TMPNAM]);
}

int
Ldup(int fa, int fb)
{
    if (fa >= 0) {
        fb = fcntl(fa, F_DUPFD, fb);          /* normal dup */

        if (fb < 0) {            /* no available fd from 63 to 1999 */
            fb = fcntl(fa, F_DUPFD, 10);     /* try from 10; */

            if (fb < 0) {               /* shell reserves 0-9 */
                perror("bsh: fcntl");
                exitsh(ERROR);
            }

        }

        close(fa);
        fcntl(fb, F_SETFD, FD_CLOEXEC);       /* autoclose for fb */
        return(fb);
    }

}


int
chkmail()
{
    register uchar_t	*s = mailp;
    register uchar_t	*save;

    time_t		*ptr = mod_time;
    uchar_t		*start;
    BOOL		flg;
    struct stat		statb;

    while (s && *s) {
        start = s;
        save = 0;
        flg = 0;

        while (*s) {
            if (*s != COLON) {
                if (*s == '%' && save == 0)
                    save = s;

                s++;
            }
            else {
                flg = 1;
                *s = 0;
            }
        }

        if (save)
            *save = 0;

        if (*start && stat((char *)start, &statb) >= 0) {
            if(statb.st_size && *ptr && statb.st_mtime != *ptr) {
                if (save) {
                    prs(save+1);
                    newline();
                }
                else {
                    if (mailmnod.namflg != N_DEFAULT) {
                        prs(mailmnod.namval);
                        newline();
                    }
                    else
                        prs(MSGSTR(M_MAILMSG,(char *)mailmsg));
                }
            }
            *ptr = statb.st_mtime;
        }
        else if (*ptr == 0)
            *ptr = 1;

        if (save)
            *save = '%';

        if (flg)
            *s++ = COLON;

        ptr++;
    }
}

int
setmail(uchar_t *mailpath)
{
    register uchar_t	*s = mailpath;
    register int	cnt = 1;

    time_t		*ptr;

    free((void *)mod_time);
    if (mailp = mailpath) {
        while (*s) {
            if (*s == COLON)
                cnt += 1;

            s++;
        }

        ptr = mod_time = (time_t *)malloc(sizeof(long) * cnt);

        while (cnt) {
            *ptr = 0;
            ptr++;
            cnt--;
        }
    }
}
