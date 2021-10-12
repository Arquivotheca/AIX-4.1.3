static char sccsid[] = "@(#)44	1.13  src/bos/usr/ccs/lib/libc/getut.c, libcadm, bos411, 9428A410j 4/20/94 17:49:13";
/*
 *   COMPONENT_NAME: LIBCADM
 *
 *   FUNCTIONS: ENDUTENT, GETUTENT, GETUTID, GET_READLOCK,
 *		GET_WRITELOCK, REL_READLOCK, REL_WRITELOCK, SETUTENT,
 *		UT_FOUND, append_wtmp, endutent, endutent_r,
 *		gdebug, getutent, getutent_r, getutid,
 *		getutid_r, getutline, getutline_r, openent,
 *		pututline, pututline_r, setutent, setutent_r,
 *		utmpname, utmpname_r
 * 
 *   ORIGINS: 3,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION: Routines to read and write the /etc/utmp file. 
 *
 * RETURN VALUE DESCRIPTIONS: - struct utmp on success - NULL on failure or EOF 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "utmp.h"
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "ts_supp.h"
#include "push_pop.h"

/*
 * The current convention is to use lockfx and lock the first record
 * of the file for read when reading and for write when writing.  The
 * individual records in the utmp are not locked.
 */

/*
 * Define DO_READLOCK if the utmp file should be locked for reading as
 * well as for writing.
 */

/* #define DO_READLOCK */

/* Argument for write locks */
static struct flock write_lock = {
  F_WRLCK,
  SEEK_SET,
  0,
  sizeof(struct utmp),
};

/* Argument for unlocks */
static struct flock unlock = {
  F_UNLCK,
  SEEK_SET,
  0,
  sizeof(struct utmp),
};

#ifdef DO_READLOCK
/* Argument for read locks */
static struct flock read_lock = {
  F_RDLCK,
  SEEK_SET,
  0,
  sizeof(struct utmp),
};

#define GET_READLOCK(fd) (lockfx(fd, F_SETLKW, &read_lock))
#define REL_READLOCK(fd) (lockfx(fd, F_SETLKW, &unlock))
#define GET_WRITELOCK(fd) (lockfx(fd, F_SETLKW, &write_lock))
#define REL_WRITELOCK(fd) (lockfx(fd, F_SETLKW, &unlock))
#else
#define GET_READLOCK(fd) (0)
#define REL_READLOCK(fd) (0)
#define GET_WRITELOCK(fd) (lockfx(fd, F_SETLKW, &write_lock))
#define REL_WRITELOCK(fd) (lockfx(fd, F_SETLKW, &unlock))
#endif

static int openent();

#define FAILURE -1
#define MAXFILE PATH_MAX + 1	/* max utmp filename length (inc. NULL) */

#ifdef _THREAD_SAFE

#include "rec_mutex.h"

extern struct rec_mutex _utmp_rmutex;

#define	SETUTENT()	setutent_r(utmp_data)
#define	ENDUTENT()	endutent_r(utmp_data)
#define	GETUTENT()	getutent_r(utmp, utmp_data)
#define	GETUTID(u)	getutid_r(u, utmp, utmp_data)
#define	UT_FOUND(u)	(*utmp = u, TS_SUCCESS)

#define	UT_FD		(utmp_data->ut_fd)
#define	LOC_UTMP	(utmp_data->loc_utmp)
#define	UBUF		(utmp_data->ubuf)
#define UTMPFILE	(utmp_data->name)
#define UTMP_DATA	utmp_data

#define POP_N_LEAVE(x)	{ rc = x; goto pop_n_leave; }

#else

#define POP_N_LEAVE(x)	return(x)

#define	SETUTENT()	setutent()
#define	ENDUTENT()	endutent()
#define	GETUTENT()	getutent()
#define	GETUTID(u)	getutid(u)
#define	UT_FOUND(u)	(u)

#define	UT_FD		fd
#define	LOC_UTMP	loc_utmp
#define	UBUF		ubuf
#define UTMPFILE	utmpfile
#define UTMP_DATA

static int		fd = FAILURE;		/* fd for the utmp file. */
static long		loc_utmp;		/* offset in utmp of ubuf */
static struct utmp	ubuf;			/* last entry read in */
static char	utmpfile[MAXFILE]= UTMP_FILE;	/* Current utmp file */

#endif	/* _THREAD_SAFE */

#ifdef DEBUGX

#undef UTMP_FILE
#define UTMP_FILE "utmp"

#endif

/* "getutent" gets the next entry in the utmp file. */

#ifdef _THREAD_SAFE
int 
getutent_r(struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutent(void)
#endif	/* _THREAD_SAFE */
{
    register char *u;
    register int i;

    TS_EINVAL((utmp == 0) || (utmp_data == 0));

    if (openent(UTMP_DATA) < 0)
	return (TS_FAILURE);

    /*
     * Try to read in the next entry from the utmp file. If the read fails,
     * return NULL. 
     */

    if (read(UT_FD, (char *) &UBUF, (unsigned) sizeof(UBUF)) != sizeof(UBUF)) {
	bzero((char *)&UBUF, sizeof(UBUF)); /* Make sure ubuf is zeroed. */
	LOC_UTMP = 0;
	return (TS_FAILURE);
    }
    /* Save the location in the file where this entry was found. */

    LOC_UTMP = lseek(UT_FD, 0L, SEEK_CUR) - (long) (sizeof(struct utmp));
    return (UT_FOUND(&UBUF));
}

/*
 * "getutid" finds the specified entry in the utmp file. If it can't find it,
 * it returns NULL. 
 */

#ifdef _THREAD_SAFE
int 
getutid_r(const struct utmp *utent,
	  struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutid(const struct utmp *utent)
#endif	/* _THREAD_SAFE */
{
    register short type;

    TS_EINVAL((utmp == 0) || (utmp_data == 0));

    /*
     * Start looking for entry. Look in our current buffer before reading in
     * new entries. 
     */

    do {
	/* If there is no entry in "ubuf", skip to the read. */

	if (UBUF.ut_type != EMPTY) {
	    switch (utent->ut_type) {

    	    case EMPTY:
    			/* Do not look for an entry if the user sent us
			 * an EMPTY entry.
			 */
			TS_ERROR(EINVAL);
			return (TS_FAILURE);

		/*
		 * For RUN_LVL, BOOT_TIME, OLD_TIME, and NEW_TIME entries,
		 * only the types have to match. If they do, return the
		 * address of internal buffer. 
		 */

	    case RUN_LVL:
	    case BOOT_TIME:
	    case OLD_TIME:
	    case NEW_TIME:
		if (utent->ut_type == UBUF.ut_type)
		    return (UT_FOUND(&UBUF));
		break;

		/*
		 * For INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, and
		 * DEAD_PROCESS the type of the entry in "ubuf", must be one
		 * of the above and id's must match. 
		 */

	    case INIT_PROCESS:
	    case LOGIN_PROCESS:
	    case USER_PROCESS:
	    case DEAD_PROCESS:
		if (((type = UBUF.ut_type) == INIT_PROCESS ||
		     type == LOGIN_PROCESS ||
		     type == USER_PROCESS ||
		     type == DEAD_PROCESS) &&
		    (strcmp(UBUF.ut_id, utent->ut_id) == 0)) {
		    return (UT_FOUND(&UBUF));
		}
		break;

		/* Do not search for illegal types of entry. */
	    default:
		TS_ERROR(EINVAL);
		return (TS_FAILURE);
	    }
	}
    } while (GETUTENT() != TS_FAILURE);

    /* Return NULL since the proper entry wasn't found. */

    return (TS_NOTFOUND);
}

/*
 * "getutline" searches the "utmp" file for a LOGIN_PROCESS or USER_PROCESS
 * with the same "line" as the specified "entry". 
 */

#ifdef _THREAD_SAFE
int 
getutline_r(const struct utmp *utent,
	    struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutline(const struct utmp *utent)
#endif	/* _THREAD_SAFE */
{
     TS_EINVAL((utmp == 0) || (utmp_data == 0));

    /*
     * Start by using the entry currently incore. This prevents doing reads
     * that aren't necessary. 
     */
    do {
	if (!strncmp(utent->ut_line, UBUF.ut_line, sizeof(UBUF.ut_line)))
	    return (UT_FOUND(&UBUF));
    } while (GETUTENT() != TS_FAILURE);

    /* Since entry wasn't found, return NULL. */
    return (TS_NOTFOUND);
}

/*
 * "pututline" writes the structure sent into the utmp file. If there is
 * already an entry with the same id, then it is overwritten, otherwise a new
 * entry is made at the end of the utmp file. 
 */
#ifdef _THREAD_SAFE
int 
pututline_r(const struct utmp *utmp, struct utmp_data *utmp_data)
#else
struct utmp *
pututline(const struct utmp *utmp)
#endif	/* _THREAD_SAFE */
{
    struct utmp *answer;
    struct utmp tmpbuf;
    int err;

    /* First open the file if necessary */
    if (openent(UTMP_DATA) < 0)
	return (NULL);

    /*
     * To avoid the "possible deadlock" error, we give up the read
     * lock before we get the write lock.
     */
    if (REL_READLOCK(UT_FD) < 0 || GET_WRITELOCK(UT_FD) < 0)
	return (NULL);

    /*
     * Copy the user supplied entry into our temporary buffer to avoid the
     * possibility that the user is actually passing us the address of
     * "ubuf". 
     */

    tmpbuf = *utmp;

    /*
     * Find the proper entry in the utmp file. Start at the current location.
     * If it isn't found from here to the end of the file, then reset to the
     * beginning of the file and try again. If it still isn't found, then
     * write a new entry at the end of the file. (Making sure the location is
     * an integral number of utmp structures into the file incase the file is
     * scribbled.) 
     */

    if (GETUTID(&tmpbuf) == TS_FAILURE) {
#ifdef ERRDEBUG
	gdebug("First getutid() failed. fd: %d", UT_FD);
#endif
	SETUTENT();
	if (GETUTID(&tmpbuf) == TS_FAILURE) {
	    LOC_UTMP = lseek(UT_FD, 0L, SEEK_CUR);
	    LOC_UTMP -= LOC_UTMP % sizeof(struct utmp);
#ifdef ERRDEBUG
	    gdebug("Second getutid() failed. fd: %d loc_utmp: %ld\n",
		   UT_FD, LOC_UTMP);
#endif
	}
    }

    /* Seek to the proper place on the file descriptor for writing. */
    lseek(UT_FD, LOC_UTMP, SEEK_SET);

    /*
     * Write out the user supplied structure. If the write fails, then the
     * user probably doesn't have permission to write the utmp file. 
     */
    if (write(UT_FD, (char *)&tmpbuf, (unsigned) sizeof(tmpbuf)) !=
	sizeof(tmpbuf)) {
#ifdef ERRDEBUG
	gdebug("pututline failed: write-%d\n", errno);
#endif
#ifdef _THREAD_SAFE
	err = -1;
#else
	answer = (struct utmp *) NULL;
#endif
    } else {
	/*
	 * Copy the user structure into ubuf so that it will be up to date in
	 * the future. 
	 */
	UBUF = tmpbuf;
#ifdef _THREAD_SAFE
	err = 0;
#else
	answer = &UBUF;
#endif
#ifdef ERRDEBUG
	gdebug("id: %c%c loc: %x\n", ubuf.ut_id[0], ubuf.ut_id[1],
	       ubuf.ut_id[2], ubuf.ut_id[3], loc_utmp);
#endif
    }

    /* Don't know what to do even if it does error off */
    (void) REL_WRITELOCK(UT_FD);
    (void) GET_READLOCK(UT_FD);

#ifdef _THREAD_SAFE
	return (err);
#else
	return (answer);
#endif
}

/*
 * If the "utmp" file is not open, attempt to open it for reading. If
 * there is no file, attempt to create one. If both attempts fail,
 * return NULL. If the file exists, but isn't readable and writeable,
 * do not attempt to create.
 */
#ifdef _THREAD_SAFE
static int
openent(struct utmp_data *utmp_data)
#else
static int
openent()
#endif
{
    int err;
    int rc = 0;

    if (UT_FD == FAILURE) {
	TS_LOCK(&_utmp_rmutex);
	TS_PUSH_CLNUP(&_utmp_rmutex);

#ifdef _THREAD_SAFE
	if (utmp_data->name == NULL) {
		if ((utmp_data->name = (char *)malloc(MAXFILE + 1)) == (char *)NULL) {
			errno = ENOMEM;
			POP_N_LEAVE(-1);
		}
		strcpy (utmp_data->name, UTMP_FILE);
	}
#endif	
	if ((UT_FD = open(UTMPFILE, O_RDWR|O_CREAT, 0644)) == FAILURE) {
	    /*
	     * If the open failed for permissions, try opening it only
	     * for reading. All "pututline()" later will fail the
	     * writes.
	     */
	    if ((err = errno) != EACCES ||
		(UT_FD = open(UTMPFILE, O_RDONLY)) == FAILURE) {
		    POP_N_LEAVE(-1);
	    }
	}

	/*
	 * We want to close this on exec and we also want the read
	 * lock (in the case we get read locks).
	 */
	if (fcntl(UT_FD, F_SETFL, 1) == -1 || /* why should this fail? */
	    GET_READLOCK(UT_FD) < 0) {	/* Lock the file for reading */
	    close(UT_FD);
	    UT_FD = FAILURE;
	    POP_N_LEAVE(-1);
	}

pop_n_leave:
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_utmp_rmutex);
    }
    return(rc);		/* rc is 0 or set by POP_N_LEAVE(x) to the value x */
}

/* "setutent" just resets the utmp file back to the beginning. */

#ifdef _THREAD_SAFE
void 
setutent_r(struct utmp_data *utmp_data)
#else
void 
setutent(void)
#endif  /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (utmp_data == 0) return;
#endif
    if (UT_FD != FAILURE)
	lseek(UT_FD, 0L, SEEK_SET);

    /*
     * Zero the stored copy of the last entry read, since we are resetting to
     * the beginning of the file. 
     */

    bzero(&UBUF, sizeof(UBUF));
    LOC_UTMP = 0L;
}

/* "endutent" closes the utmp file. */

#ifdef _THREAD_SAFE
void 
endutent_r(struct utmp_data *utmp_data)
#else
void 
endutent(void)
#endif  /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (utmp_data == 0) return;
#endif
    if (UT_FD != FAILURE)
	close(UT_FD);
    UT_FD = FAILURE;
    LOC_UTMP = 0L;
    bzero((char *)&UBUF, sizeof(UBUF));
}

/*
 * "utmpname" allows the user to read a file other than the normal "utmp"
 * file. 
 */

#ifdef _THREAD_SAFE
utmpname_r(char *newfile, struct utmp_data *utmp_data)
#else /* _THREAD_SAFE */
utmpname(char *newfile)
#endif /* _THREAD_SAFE */
{
    /* Determine if the new filename will fit. If not, return FALSE. */
    if (strlen(newfile) > MAXFILE)
	return (FALSE);

    /* copy in the new file name. */
    strcpy(UTMPFILE, newfile);

    ENDUTENT();				/* reset to the beginning */
    return (TRUE);
}

int append_wtmp(char *s, struct utmp *u)
{
    int f;
    long pos;
    int err = 0;

    if ((f = open(s, O_RDWR)) < 0)
	return -1;

    if (GET_WRITELOCK(f) < 0 ||			/* get write lock */
	(pos = lseek(f, 0L, SEEK_END)) < 0 ||	/* seed to end */
	((pos %= sizeof(*u)) &&			/* seek to rec boundry */
	 lseek(f, -pos, SEEK_END) < 0) ||	/* backup to rec boundry */
	write(f, u, sizeof(*u)) != sizeof(*u))	/* write out record */
	err = -1;

    (void)close(f);				/* close, ignore errs */
    return err;
}

#ifdef ERRDEBUG
#include <stdio.h>

gdebug(format, arg1, arg2, arg3, arg4, arg5, arg6)
char *format;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
    register FILE *fp;
    register int errnum;

    if ((fp = fopen("/etc/dbg.getut", "a+")) == NULL)
	return;

    fprintf(fp, format, arg1, arg2, arg3, arg4, arg5, arg6);
    fclose(fp);
}

#endif
