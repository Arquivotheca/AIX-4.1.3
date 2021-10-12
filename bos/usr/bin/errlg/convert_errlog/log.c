static char sccsid[] = "@(#)01  1.2  src/bos/usr/bin/errlg/convert_errlog/log.c, cmderrlg, bos411, 9428A410j 4/21/94 21:02:57";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: loginit, logtrunc, logappend, logcreate, logreset, logread, 
 *			  logwrite, log_hdr_keeping
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <sys/lockf.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/erec.h>
#include <cmderrlg_msg.h>
#include <errlg.h>
#include "convert_errlog.h"

extern	int	Logfd;
extern	int v3Logfd;
extern	struct log_hdr log_hdr;
extern	struct v3_log_hdr v3_log_hdr;

extern	int	Threshold;		/* threshold size of error log file */
extern	char *v4log;		/* error log file name */
extern	char *v3log;		/* error log file name */
extern	char *v3log_temp;

int	Logfd = -1;
extern v3_Log_outoff;
static Log_descflg;
static Log_prevsequence;
static v3_Log_prevsequence;

int		loginit(void);
static	logcreate(void);
static 	logreset(int start);
static	int logread(char *buf, int count, int offset);
int		logwrite(char *buf, int count, int offset);
void	logappend(char *pack_buf);
static	void log_hdr_keeping(struct log_hdr *lh, int esize);
int		v3_logget(struct v3_log_entry *le);


/*
 * NAME:      loginit()
 * FUNCTION:  Initialize an error log file by opening and optionally
 *            creating it.
 * RETURNS:   -1 Error
 *            0  Success
 */

int
loginit(void)
{

	int rc = -1;

	if((Logfd = open(v4log,O_RDWR)) < 0 && logcreate())
		perror("loginit: open(v4log,O_RDRW)");
	else if(logread(&log_hdr,sizeof(log_hdr),0) && logcreate())
		;
	else if(strncmp(log_hdr.lh_magic,LH_MAGIC,8) != 0 && logcreate())
		;
	else
		rc = 0;	/* success */

	return(rc);
}


/*
 * NAME:      logcreate()
 * FUNCTION:  Create an error log file and reserve the minimum size.
 * RETURNS:   NONE
 */
static logcreate(void)
{
	char buf[LH_MINSIZE];
	int  rc = 0;

	if(Logfd >= 0)
		close(Logfd);
	if((Logfd = open(v4log,O_RDWR|O_TRUNC|O_CREAT,0644)) < 0)  {
		perror("logcreate: open");
		rc = -1;
	}
	else if(logwrite(&buf,LH_MINSIZE,0)) /* reserve LH_MINSIZE bytes for log file */
		rc = -1;

	logreset(1);
}


/*
 * NAME:      logreset()
 * FUNCTION:  Reset an error log file by initializing the header.
 * RETURNS:   0  Success
 *            -1 Failure
 */

int
static logreset(int start)
{

	memset(&log_hdr,0,sizeof(log_hdr));
	memcpy(log_hdr.lh_magic,LH_MAGIC,8);
	log_hdr.lh_inoff    = sizeof(log_hdr);
	log_hdr.lh_outoff   = sizeof(log_hdr);
	log_hdr.lh_maxsize  = Threshold;
	log_hdr.lh_sequence = start;
	log_hdr.lh_entries  = 0;
	log_hdr.lh_staleoff = Threshold;

	if(logwrite(&log_hdr,sizeof(log_hdr),0))
		return(-1);
	return(0);
}


/*
  * NAME:      logread()
  * FUNCTION:  Read an error log entry from the supplied offset in the v3 errlog.
  * RETURNS:
  *             0 Success
  *            -1 Error
  */

int
logread(char *buf, int count, int offset)
{
	int rv;
	int rc;

	if(v3Logfd < 0)
		rc = -1;
	else if(lseek(v3Logfd,offset,0) < 0) {
		perror("logread: lseek");
		rc = -1;
	}
	else if((rv=read(v3Logfd,buf,count)) < 0) {
		perror("logread: read");
		rc = -1;
	}
	else if(rv != count) {
		cat_eprint(CAT_LOG_READ_EOF,"logread: UNEXPECTED EOF\n");
		rc = -1;
	}
	else
		rc = 0;
	return(rc);
}


/*
  * NAME:      logwrite()
  * FUNCTION:  Write an error log entry at the supplied offset
  *		in the V4 error log.
  * RETURNS:
  *             0 Success
  *            -1 Error
  */

int
logwrite(char *buf, int count, int offset)
{

	if(Logfd < 0)
		return(-1);

	if(lseek(Logfd,offset,0) < 0) {
		perror("logwrite: lseek");
		return(-1);
	}
	if(write(Logfd,buf,count) != count) {
		perror("logwrite: write");
		return(-1);
	}
	return(0);
}


/*
  * NAME:      logappend()
  * FUNCTION:  Append an entry to a V4 error log.
  * RETURNS:   None.
  */

void
logappend(char *pack_buf) /* LE_MAX_SIZE bytes max. */
{
	int  esize;
	int saverr;

	if(Logfd >= 0) {
		esize = *(int *)pack_buf;		/* entry size */
		if(!logwrite(pack_buf,esize,log_hdr.lh_inoff)) {
			log_hdr_keeping(&log_hdr,esize);
		}
		else {	/* write failed, so terminate processing */

			/* delete the v4 log */
			unlink(v4log);

			/* rename the v3 log back to the original name */
			if(rename(v3log_temp,v3log) == -1)
			{       /* move  backup to v3 log */
				saverr = errno;
				fprintf(stderr,"%s:  convert_clean_up: ",Progname);
				errno = saverr;
				perror("rename: ");
			}

			fprintf(stderr,"%s: logappend: unable to write to %s.\n",Progname,v4log);
			exit(1);
		}
		fsync(Logfd);
	}
	else {
		fprintf(stderr,"%s: logappend: Logfd < 0.\n",Progname);
		exit(1);
	}
}


/*
  * NAME:      log_hdr_keeping()
  * FUNCTION:  Perform log_hdr (house)keeping following
  *            the successful addition of a log entry.
  * RETURNS:   None
  */

void
log_hdr_keeping(struct log_hdr *lh, int esize)
{
	lh->lh_inoff += esize;
	lh->lh_entries += 1;
	logwrite(lh,sizeof(log_hdr),0);		/* hdr write should never fail */
}

/*
 * NAME:     v3_logget
 * FUNCTION: Get a version 3 logentry from v3Logfd.
 * RETURN:	 0 - finished (or file is empty)
 *		 -1 - error
 *		 >1 - length of entry
 */

int
v3_logget(struct v3_log_entry *le)
{

	if(v3_log_hdr.lh_currsize == v3_log_hdr.lh_startoff) 	/* a test for empty */
		return(0);
	if(logread(le,3*sizeof(int),v3_Log_outoff)) {
		fprintf(stderr,"%s: logget: unable to read log entry magic, length, and sequence at offset 0x%x\n",Progname,
		    v3_Log_outoff);
		return(-1);
	}
	if(le->le_magic != v3_LE_MAGIC) {
		fprintf(stderr,"%s: logget: bad magic at offset 0x%x\n",Progname,v3_Log_outoff);
		return(-1);
	}
	if(le->le_sequence <= v3_Log_prevsequence) {		/* finished */
		return(0);
	}
	if(logread(le,le->le_length,v3_Log_outoff))  {
		fprintf(stderr,"%s: logget: logread failed at offset 0x%x reading %d bytes\n",Progname,v3_Log_outoff,le->
		    le_length);
		return(-1);
	}
	else if ((v3_Log_outoff += le->le_length) >= v3_log_hdr.lh_currsize)	/* wrap it */
		v3_Log_outoff = v3_log_hdr.lh_startoff;


	v3_Log_prevsequence = le->le_sequence;

	/*
	printf("le:\n\
	\tmagic\t=\t%x\n\
	\tlength\t=\t%d\n\
	\tsequence\t=\t%d\n\
	\tdatalength\t=\t%d\n",le->le_magic,le->le_length,le->le_sequence,le->le_datalength);

	\tlabel\t=\t%s\n\
	\ttime\t=\t%d\n\
	\tcrcid\t=\t%x\n\
	\tmachine\t=\t%s\n\
	\tnode\t=\t%s\n\
	\tclass\t=\t%s\n\
	\ttype\t=\t%s\n\
	\tresource\t=\t%s\n\
	\trclass\t=\t%s\n\
	\trtype\t=\t%s\n\
	\tibm\t=\t%s\n\
	\tuser\t=\t%s\n\
	\tin\t=\t%s\n\
	\tconn\t=\t%s\n\
	\tdetlen\t=\t%d\n\
	\tlength2\t=\t%d\n"
*/



	return(le->le_length);
}

