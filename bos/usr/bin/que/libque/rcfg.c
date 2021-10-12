static char sccsid[] = "@(#)67	1.16.1.4  src/bos/usr/bin/que/libque/rcfg.c, cmdque, bos411, 9428A410j 6/15/94 13:44:37";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: needtodigest, readconfig, rcfg, doread
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <IN/standard.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <IN/DRdefs.h>
#include <sys/stat.h>
#include "common.h"
#include <ctype.h>

#include "libq_msg.h"

struct q *rcfg();

extern char *ruser;
extern long renq_nid;

#define MAXRETRIES	4
#define UNCONDITIONAL   1

/*---Qconfig.bin file control object definition */

void    resetQCBFile();
void    setQCBFileOpen();
boolean isQCBFileOpen();
time_t  readQCBTime();
void    openPacQCBFile(char *);
boolean openQCBFile(boolean);
void    closeQCBFile();
size_t  readQCBFile(void *,size_t);

static time_t       qcbtime;
static boolean      qcbisopen;
static int          qcbfd;
static struct flock qcb_flock;
/*---end Qconfig.bin file control object definition */


/*---Reset the qconfig.bin file (or equiv.) control data structure */
void resetQCBFile()
{
        qcbisopen = FALSE;
        qcbtime = (time_t)0;
        qcbfd = 0;
}

/*---Set flag to indicate qconfig.bin (or equiv.) is open now */
void setQCBFileOpen()
{
        qcbisopen = TRUE;
}

/*---Return status indicating if qconfig.bin file (or equiv.) is open or not */
boolean isQCBFileOpen()
{
        return (qcbisopen);
}

/*---Retrieve the qconfig.bin file (or equiv.) creation time */
time_t readQCBTime()
{
        return(qcbtime);
}

/*---Open a named, digested version of the /etc/qconfig file and
     read the creation time, if not already done.  DO NOT FLOCK */
void openPacQCBFile(char *pacfile)
{
        if (isQCBFileOpen() != TRUE)
        {
                errno = 0;
                resetQCBFile();
                if ((qcbfd = open(pacfile, O_RDONLY)) == -1)
                        syserr((int)EXITFATAL,GETMESG(MSGNOPN,"Cannot open %s for reading."), pacfile);

                setQCBFileOpen();
                readQCBFile(&qcbtime,sizeof(time_t));
        };
}

/*---Open /etc/qconfig.bin, lock it, and read the time of creation,
     if not already done */
boolean openQCBFile(boolean openFailOK)
{
        if (isQCBFileOpen() == TRUE)	/* if already open then */
                return(TRUE);		/*  succeed = TRUE, so compare dates */
        errno = 0;
        resetQCBFile();
        if ((qcbfd = open(BCONFIG, O_RDONLY)) == -1)
	{
		if (openFailOK == TRUE)
			return(FALSE);	/* succeed = FALSE (not there) , we needtodigest */
                else
                	syserr((int)EXITFATAL,GETMESG(MSGNOPN,"Cannot open %s for reading."), BCONFIG);
	};

        qcb_flock.l_type = F_RDLCK;
        qcb_flock.l_whence = 0;
        qcb_flock.l_start = 0;
        qcb_flock.l_len = 0;
        if ( -1 == fcntl(qcbfd, F_SETLKW, &qcb_flock ) )
            syserr((int)EXITFILE,GETMESG(MSGNLCK,"cannot lock %s for reading."), BCONFIG);

        setQCBFileOpen();
        readQCBFile(&qcbtime,sizeof(time_t));
	return(TRUE);			/* succeed == TRUE (file opened), so compare dates */
}

/*---Read a sized record from /etc/qconfig.bin (or equiv.) */
size_t readQCBFile(void *buffer, size_t size)
{
        size_t readsize;

        errno = 0;
        readsize = read(qcbfd,buffer,size);
        if (readsize != size &&
            readsize != 0)
                syserr((int)EXITFATAL,GETMESG(MSGRERR,"Read error %d on %s."),errno,BCONFIG);
        return(readsize);
}

/*---Close the qconfig.bin file (or equiv.) */
void closeQCBFile()
{
        if (isQCBFileOpen() == TRUE)
                close(qcbfd);
        resetQCBFile();
}

/* read the config file.  return a list of q's with all the info */
/* plugged in.  preserve order in config, so default is first. */
/* flag selects when digestion (qconfig[ascii] -> qconfig.bin[bin]) occurs */

/*==== Compare the date contained in qconfig.bin of the qconfig it was digested from
       with the actual date of the qconfig file to see if digestion is needed.
*/
boolean needtodigest()
{
	struct stat ascii;
	int i = 0;

	/*----Read the actual date of the qconfig file */
	while (stat(CONFIG,&ascii) == -1) {
		if (i == MAXRETRIES) 
			syserr((int)EXITFATAL,GETMESG(MSGCFGF,"Cannot find config file %s."),CONFIG);
		sleep(1);
		i++;
	}
	/*----Open and read the qconfig last modification date from qconfig.bin file */
	if (openQCBFile(TRUE) == FALSE)		/* open (fail ok), if succeed == FALSE (not exist) */
		return(TRUE);			/*   then we needtodigest */
	if (readQCBTime() == ascii.st_mtime)
		return(FALSE);
	return(TRUE);
}


/* 1: unconditional force; 0: if qconfig.bin not there or older than ascii */
struct q * readconfig(flag)
{
	register struct q *qlist = NULL;

	if( (flag == UNCONDITIONAL) || needtodigest())
	{
		closeQCBFile();
		redigest();    /* redigest local qconfig */
	}
	openQCBFile(FALSE);	/* open failure NOT OK */
	qlist = rcfg(BCONFIG);
/*	if (qlist == NULL)
		syserr((int)EXITFATAL,GETMESG(MSGNADA,"Nothing read from %s.  Errno = %d."),BCONFIG,errno); */
	return(qlist);
}


/*
* rcfg - read in a qconfig.bin file and return a pointer to the qlist.
*	Called by readconfig to read a local qconfig.bin file.
*/

struct q * rcfg(name)
char *name;		/* error reporting only */
{
	register struct q *lastq = NULL;
	register struct d *lastd = NULL;
	register struct q *qbuf;
	register struct d *dbuf;
	struct q *qlist = NULL;
	register int devcount;
	int  belength;
	int numread;
	char dname_save[DNAME +1];
	char bak_end[BENAME+1];

	/*----Read in the queue-device structure */
	while (1)
	{

		/* read in a queue */
		qbuf = Qalloc(sizeof(struct q));
		numread = readQCBFile(qbuf,sizeof(struct q));
		if (numread == 0)       /* no more queues */
		{
			free((void *)qbuf);
			break;
		}

#ifdef	DEBUG
		if( getenv("RCFG") )
			qdbg(qbuf);
#endif

		if (lastq == NULL)
			qlist = lastq = qbuf;
		else
		{
			lastq->q_next = qbuf;
			lastq = lastq->q_next;
		}
		lastd = NULL;
		lastq->q_entlist = NULL;
		lastq->q_devlist = NULL;
		lastq->q_next = NULL;
		lastq->q_entcount = 0;
		lastq->q_top = FALSE;
		devcount = lastq->q_devcount;

#ifdef	DEBUG
	prtq(stderr,lastq);
#endif

		/* now read in each device */
		while (devcount)
		{
			devcount--;
			dbuf = Qalloc( sizeof(struct d));
			readQCBFile(dbuf,sizeof(struct d));
			if (lastd == NULL)
			{
				lastd = dbuf;
				lastq->q_devlist = lastd;
			}
			else
			{
				lastd->d_next = dbuf;
				lastd = lastd->d_next;
			}
			lastd->d_e = NULL;
			lastd->d_next = NULL;
			lastd->d_q = lastq;
#ifdef	DEBUG
	prtdev(stderr,lastd);
#endif

			/* and its backend */

			readQCBFile(&belength,sizeof(belength));

			if( belength <= 0 || belength > MAXLINE )
			  syserr((int)EXITFATAL,GETMESG(MSGCORU,"Corrupted %s, belength = %d."),BCONFIG,belength);

			lastd->d_backend = Qalloc(belength);
			readQCBFile(lastd->d_backend,belength);
		}


		/*
		  A surrogate queue may still have the device info.  It just
		  won't be used at all.  Digest still put it in the qconfig.bin
		  since that file may be overmounted in an SSI system.  I.e. 
		  digest doesn't know which machine will read the qconfig.bin
		  file.
		  Since surrogate queues don't need the specified device info,
		  we'll read it in, (which was done above,) and then chuck it.
		  If a surrogate queue doesn't have any device info, we still
		  need to create a dummy device structure for it because the
		  remote backend still has to be run.
		*/
		if (remote(lastq->q_hostname))  /* must have been remote */
		{
			struct d *dptr,*save;

			/*----Save the name of the first device, if any, for qstatus display purposes */
			if (NULL != lastq->q_devlist)
				strcpy(dname_save, lastq->q_devlist->d_name);

			/*----Dump all devices attached to remote queue */
			for (dptr=lastq->q_devlist; dptr; )
			{
				save=dptr->d_next;	/* get next */
				free((void *)dptr);
				dptr=save;
			}

			/*----Reallocate, reconnect and rename a single device */
			/* D43769, save the backend name with args */
			strncpy(bak_end,lastd->d_backend,BENAME);
			dbuf = Qalloc( sizeof(struct d));
			lastq->q_devlist = dbuf;
			dbuf->d_q = lastq;
			strcpy(dbuf->d_name,dname_save);
			lastq->q_devlist->d_backend= (char *)scopy(bak_end);
		}
	}
	closeQCBFile();
	return(qlist);
}

