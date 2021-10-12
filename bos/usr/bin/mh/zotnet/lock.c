static char sccsid[] = "@(#)13	1.11  src/bos/usr/bin/mh/zotnet/lock.c, cmdmh, bos411, 9428A410j 8/19/93 09:35:23";
/* 
 * COMPONENT_NAME: CMDMH lock.c
 * 
 * FUNCTIONS: alrmser, b_lkopen, f_lkopen, lkclose, lkfclose, lkfopen, 
 *            lkopen, lockname, timerOFF, timerON 
 *
 * ORIGINS: 26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* lock.c - universal locking routines */

#ifndef	MMDFONLY
#include "strings.h"
#else	MMDFONLY
#include "strings.h"
#include "mmdfonly.h"
#endif	MMDFONLY
#include <stdio.h>
#include "mts.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/lockf.h>
#include <sys/errno.h>
#include <setjmp.h>

#define	NOTOK	(-1)
#define	OK	0

#define	NULLCP	((char *) 0)

#ifdef	SYS5
#define	index	strchr
#define	rindex	strrchr
#endif	SYS5

static void alrmser(int);
static int b_lkopen();
static lockname();
static timerON();
static timerOFF();

extern int  errno;

/*  */

int	lkopen (file, access)
register char   *file;
register int     access;
{
    mts_init ("mts");
    switch (lockstyle) {
	case LOK_UNIX:
#ifdef	BSD42
	    return f_lkopen (file, access);
#endif	BSD42
	default:
	    return b_lkopen (file, access);
	}
}

/*  */

static int  b_lkopen (file, access)
register char   *file;
register int     access;
{
    register int    i,
                    j,
	      lockfd;
    long    curtime;
    char    curlock[BUFSIZ];
    struct stat st;

    if (stat (file, &st) == NOTOK)
	return NOTOK;
    lockname (curlock, file, (int) st.st_dev, (int) st.st_ino);

    for (i = 0;;) {
        lockfd=open(curlock, O_CREAT | O_NSHARE | O_NDELAY, 0600);
	switch (lockfd) {

	    case NOTOK: 
		if (stat (curlock, &st) == NOTOK) {
		    if (i++ > 5)
			return NOTOK;
		    sleep (5);
		    break;
		}

		i = 0;
		(void) time (&curtime);
		if (curtime < st.st_ctime + 60L)
		    sleep (5);
		else
		    (void) unlink (curlock);
		break;

	    default:  /* OK */
		if ((i = open (file, access)) == NOTOK) {
		    j = errno;
		    (void) unlink (curlock);
		    errno = j;
		}
		timerON (curlock, i);
		return (i);

	}
    }
}


/*  */

static  lockname (curlock, file, dev, ino)
register char   *curlock,
	        *file;
register int     dev,
		 ino;
{
    register char  *bp,
                   *cp;

    bp = curlock;
    if ((cp = rindex (file, '/')) == NULL || *++cp == (char)NULL)
	cp = file;
    if (lockldir == NULL || *lockldir == (char)NULL) {
	if (cp != file) {
	    (void) sprintf (bp, "%.*s", cp - file, file);
	    bp += strlen (bp);
	}
    }
    else {
	(void) sprintf (bp, "%s/", lockldir);
	bp += strlen (bp);
    }

    switch (lockstyle) {
	case LOK_BELL: 
	default: 
	    (void) sprintf (bp, "%s.lock", cp);
	    break;

	case LOK_MMDF: 
	    (void) sprintf (bp, "LCK%05d.%05d", dev, ino);
	    break;
    }

}

/*  */

#ifdef	BSD42

static jmp_buf jmp;

static void timeout()
{
    longjmp(jmp, 1);
}

static int  f_lkopen (file, access)
register char   *file;
register int     access;
{
    register int    fd,
                    i,
		    j;
    void (*oldsig)();

    if ((fd = open (file, access | O_NDELAY)) == NOTOK)
	return NOTOK;

    if (setjmp(jmp)) {  /* timed out */
	signal(SIGALRM, oldsig);
	(void) close (fd);
	errno = EWOULDBLOCK;  /* mbx_Xopen() in dropsbr.c looks for this */
	return(NOTOK);
    }
    alarm(0);  /* just in case */
    oldsig = signal(SIGALRM, timeout);
    alarm(25);  /* originally looped 5 times, sleeping 5 seconds */

#ifdef _AIX  /* lockf() is more flexible than flock() in AIX */
    i = lockf(fd, F_LOCK, 0);
#else
    i = flock (fd, LOCK_EX);
#endif
    j = errno;
    alarm(0);
    signal(SIGALRM, oldsig);
    if (i != NOTOK)  /* success */
	return fd;
    (void) close (fd);
    errno = j;
    return NOTOK;
}
#endif	BSD42

/*  */

/* ARGSUSED */

int     lkclose (fd, file)
register int     fd;
register char   *file;
{
    char    curlock[BUFSIZ];
    struct stat st;

    if (fd == NOTOK)
	return OK;
    switch (lockstyle) {
	case LOK_UNIX: 
#ifdef	BSD42
	    break;
#endif	BSD42

	default: 
	    if (fstat (fd, &st) != NOTOK) {
		lockname (curlock, file, (int) st.st_dev, (int) st.st_ino);
		(void) unlink (curlock);
		timerOFF (fd);
	    }
    }

    return (close (fd));
}


/*  */

FILE	*lkfopen (file, mode)
register char   *file,
 	        *mode;
{
    register int    fd;
    register FILE  *fp;

    if ((fd = lkopen (file, strcmp (mode, "r+w") ? 2 : 0)) == NOTOK)
	return NULL;

    if ((fp = fdopen (fd, mode)) == NULL) {
	(void) close (fd);
	return NULL;
    }

    return fp;
}


/* ARGSUSED */

int	lkfclose (fp, file)
register FILE	*fp;
register char	*file;
{
    char    curlock[BUFSIZ];
    struct stat st;

    if (fp == NULL)
	return OK;

    switch (lockstyle) {
	case LOK_UNIX: 
#ifdef	BSD42
	    break;
#endif	BSD42

	default: 
	    if (fstat (fileno (fp), &st) != NOTOK) {
		lockname (curlock, file, (int) st.st_dev, (int) st.st_ino);
		(void) unlink (curlock);
	    }
    }

    return (fclose (fp));
}

/*  */

#include <signal.h>

#define	NSECS	((unsigned) 20)


struct lock {
    int		 l_fd;
    char	*l_lock;
    struct lock *l_next;
};
#define	NULLP	((struct lock *) 0)

static struct lock *l_top = NULLP;


/* ARGSUSED */

static void alrmser (int sig)
{
    register int    j;
    register char  *cp;
    register struct lock   *lp;

#ifndef	BSD42
    (void) signal (SIGALRM, (void(*)(int)) alrmser);
#endif	BSD42

    for (lp = l_top; lp; lp = lp -> l_next)
	if (*(cp = lp -> l_lock) && (j = creat (cp, 0400)) != NOTOK)
	    (void) close (j);

    (void) alarm (NSECS);
}

/*  */

static timerON (lock, fd)
char   *lock;
int	fd;
{
    register struct lock   *lp;

    if ((lp = (struct lock *) malloc ((unsigned) (sizeof *lp))) == NULLP)
	return;			/* XXX */

    lp -> l_fd = fd;
    if ((lp -> l_lock = malloc ((unsigned) (strlen (lock) + 1))) == NULLCP) {
	free ((char *) lp);
	return;			/* XXX */
    }
    (void) strcpy (lp -> l_lock, lock);
    lp -> l_next = NULLP;

    if (l_top)
	lp -> l_next = l_top -> l_next;
    else {
	(void) signal (SIGALRM, (void(*)(int)) alrmser);/* perhaps SIGT{STP,TIN,TOU} */
	(void) alarm (NSECS);
    }
    l_top = lp;
}


static timerOFF (fd)
int	fd;
{
    register struct lock   *pp,
                           *lp;

    (void) alarm (0);

    if (l_top) {
	for (pp = lp = l_top; lp; pp = lp, lp = lp -> l_next)
	    if (lp -> l_fd == fd)
		break;
	if (lp) {
	    if (lp == l_top)
		l_top = lp -> l_next;
	    else
		pp -> l_next = lp -> l_next;

	    free (lp -> l_lock);
	    free ((char *) lp);
	}
    }

    if (l_top)
	(void) alarm (NSECS);
}
