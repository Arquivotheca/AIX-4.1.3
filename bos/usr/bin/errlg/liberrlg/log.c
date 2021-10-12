static char sccsid[] = "@(#)08        1.16  src/bos/usr/bin/errlg/liberrlg/log.c, cmderrlg, bos41J, 9507C 2/2/95 18:03:56";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: loginit, logtrunc, logappend, loggetinit, logget,
 *            logfsyncmode, logread, logwrite, hdrlock,
 *            hdrunlock, logclose, get_log_entry, get_log_entry_rv, get_log_hdr,
 *            log_hdr_keeping, log_pre_wrap, log_regular_wrap
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
#include <ulimit.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <stdio.h>
#include <sys/cfgodm.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/errids.h>
#include <sys/erec.h>
#include <cmderrlg_msg.h>
#include <errlg.h>
#include <errlg/SWservAt.h>
#include <sys/err_rec.h>

#ifdef DUMPLOG
/********************************************************************/
/*   Global variables to be used in debugging logfile pointer       */
/*   management problems.  To use this, compile with -D option      */
/*   ie. "build OPT_LEVEL=-DDUMPLOG"                                */
/*   Debug information for this errdemon will then go to            */
/*   /tmp/errlog.dump.                                              */

extern int  Global_Wrap;
extern int  Global_Sequence;
extern int  Global_New_Inoffb;

/********************************************************************/
#endif  /* DUMPLOG */

extern void get_CuVPD_resource();
extern int  tmpltflg;
extern Boolean  symptomdataflg;

extern	struct log_hdr log_hdr;
extern	struct log_entry log_entry;

extern	int	Threshold;		/* threshold size of error log file */
extern  char 	Logfile[PATH_MAX];	/* error log file name, set to default in errlg.h
								possibly changes by errpt or errclear */

int	Logfd = -1;
static Log_lockflg;
static Log_wrap;
static Log_outoff;
static Log_descflg;
static Log_prevsequence;
static nofsyncflg;
static Last_entry;
char pack_buf[LE_MAX_SIZE];

int	set_append_pointer();

void 	getlogpath();
void    setlogpath();
int		loginit(int createflg);
static	logcreate(void);
void	logtrunc(void);
static 	logreset(int start);
void	logfsyncmode(int onflg);
void	loggetinit(void);
int		logget(struct obj_errlog *oep, char *buf, struct le_bad *le_bad);
int		logget_rv(struct obj_errlog *oep, char *buf, struct le_bad *le_bad);
int		logread(char *buf, int count, int offset);
int		logwrite(char *buf, int count, int offset);
void	logstat(void);
void	hdrlock(void);
void	hdrunlock(void);
int		logopen(int flag);
void	logclose(void);
int		get_log_entry(char *buf,struct obj_errlog* oep, int offset,struct le_bad *le_bad);
int		get_log_entry_rv(char *buf,struct obj_errlog* oep, int offset,struct le_bad *le_bad);
int		logappend( char *pack_buf);
int		get_log_hdr(struct log_hdr *hdr);
void	log_hdr_keeping(struct log_hdr *lh, int esize);
void	log_pre_wrap(int prev_wrap_size);
void	log_regular_wrap();


/*
 *  An error log entry will look like the following.  Note that if a dynamic
 *  length is zero then there will be no dynamic data present.
 * log_entry:
 *			unsigned int le_length;
 *			unsigned int le_magic;
 *			int			 le_sequence;
 *			time_t		 le_timestamp;
 *			unsigned int le_crcid;
 *			unsigned int le_mach_length;
 *			char*		 le_machine_id;
 *			unsigned int le_node_length;
 *			char*		 le_node_id;
 *			unsigned int le_resource_length;
 *			char*        le_resource;
 *			unsigned int le_vpd_ibm_length;
 *			char*        le_vpd_ibm;
 *			unsigned int le_vpd_user_length;
 *			char*        le_vpd_user;
 *			unsigned int le_in_length;
 *			char*        le_in;
 *			unsigned int le_connwhere_length;
 *			char*        le_connwhere;
 *			unsigned int le_rclass_length;
 *			char*        le_rclass;
 *			unsigned int le_rtype_length;
 *			char*        le_rtype;
 *			unsigned int le_detail_length;
 *			char*        le_detail_data;
 *			unsigned int le_symptom_length;
 *			char* 	     le_symptom_data;
 *			unsigned int le_length2;
 */


/*
 * NAME:      loginit()
 * FUNCTION:  Initialize an error log file by opening and optionally
 *            creating it.
 * RETURNS:   -1 Error
 *            0  Success
 */

int
loginit(int createflg)
{

	if(createflg) {
		if((Logfd = open(Logfile,O_RDWR)) < 0 && logcreate()) {
			perror("loginit: open(Logfile,O_RDWR)");
			return(-1);
		}
	}
	else {
		if((Logfd = open(Logfile,O_RDONLY)) < 0) {
			perror("loginit: open(Logfile,O_RDONLY)");
			return(-1);
		}
	}
	if(logread(&log_hdr,sizeof(log_hdr),0))
		return(-1);
	if(strncmp(log_hdr.lh_magic,LH_MAGIC,8) != 0)
		return(-1);
	return(0);
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
	if((Logfd = open(Logfile,O_RDWR|O_TRUNC|O_CREAT,0644)) < 0)  {
		perror("logcreate: open");
		rc = -1;
	}
	else if(logwrite(&buf,LH_MINSIZE,0)) /* reserve LH_MINSIZE bytes for log file */
		rc = -1;

	logreset(1);
}


/*
 * NAME:      logtrunc()
 * FUNCTION:  Truncate an error log file.  Leave only the header.
 * RETURNS:   NONE
 */

void
logtrunc(void)
{

	logread(&log_hdr,sizeof(log_hdr),0);
	if(ftruncate(Logfd,0) < 0) {
		perror("logtrunc: ftruncate");
		genexit(1);
	}
	logreset(log_hdr.lh_sequence);
}

#define VCPY(a,b) \
   memcpy(b,a,MIN(sizeof(a),sizeof(b))); ((char *)(b))[sizeof(b)-1] = '\0';


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
 * NAME:      logfsyncmode(()
 * FUNCTION:  Set the syncflag for the error log file.
 * RETURNS:   NONE
 */

void
logfsyncmode(int onflg)
{

	if(onflg) {
		fsync(Logfd);
		nofsyncflg = 0;
	} else {
		nofsyncflg = 1;
	}
}


/*
 * NAME:      loggetinit(()
 * FUNCTION:  Initialize error log state variables.
 * RETURNS:   NONE
 */

void
loggetinit(void)
{

	Log_prevsequence = 0;
	Log_outoff = 0;
	Last_entry = 0;
	Log_wrap = 0;
}


 /*
  * NAME:      logget()
  * FUNCTION:  Read an error log entry and format it into struct obj_errlog
  *            for processing.
  * RETURNS:
  *            >= sizeof(struct log_entry)	Good data
  *            0                            No entry
  *            -1                           Error
  */

int
logget(struct obj_errlog *oep, char *buf, struct le_bad *le_bad)
{
	unsigned esize;		/* entry size in bytes */
	int		 rc;

	if(Last_entry)
		rc = 0;

	else if(Logfd < 0)
		rc = 0;
	else {

		if(logread(&log_hdr,sizeof(log_hdr),0))
			rc = -1;

		else if (log_hdr.lh_entries <= 0)
			rc = 0;

		else {

			if (Log_outoff == 0)
				Log_outoff = log_hdr.lh_outoff;
			if (logread(&esize, sizeof(log_entry.le_length), Log_outoff))
				rc = -1;
			else if (logread(buf,esize,Log_outoff))
				rc = -1;
			else {
				Log_outoff += esize;
				if(invalid_le_static(buf,Log_outoff-esize,le_bad))
					rc = le_bad->code;
				else if(unpack_le(buf,oep,esize)) {
					rc = esize;
					if (!Log_wrap) {
						if (((Log_outoff) >= log_hdr.lh_inoff) &&
						    (log_hdr.lh_inoff > log_hdr.lh_outoff))
							Last_entry = 1;
						else if (Log_outoff >= log_hdr.lh_staleoff) {
							Log_outoff = sizeof(log_hdr);
							Log_wrap = 1;
						}
					}
					else if(Log_outoff >= log_hdr.lh_inoff) {
						Last_entry = 1;
					}
				}
				else
					rc = 0;
			}
		}
	}
	return(rc);
}


 /*
  * NAME:      logget_rv()
  * FUNCTION:  Read an error log entry in front of the current position, 
  *            and format it into struct obj_errlog for processing.
  * RETURNS:
  *            >= sizeof(struct log_entry)	Good data
  *            0                            No entry
  *            -1                           Error
  */

int
logget_rv(struct obj_errlog *oep, char *buf, struct le_bad *bad_le)
{
	unsigned esize;		/* entry size in bytes */
	int		 rc;

	if(Last_entry)
		rc = 0;

	else if(Logfd < 0)
		rc = 0;
	else {

		if(logread(&log_hdr,sizeof(log_hdr),0))
			rc = -1;

		else if (log_hdr.lh_entries <= 0)
			rc = 0;

		else {

			if (Log_outoff == 0)
				Log_outoff = log_hdr.lh_inoff;
			if (logread(&esize, sizeof(log_entry.le_length),
			    (Log_outoff-sizeof(log_entry.le_length))))
				rc = -1;
			else if (logread(buf,esize,(Log_outoff-esize)))
				rc = -1;
			else {
				Log_outoff-=esize;
				if(invalid_le_static(buf,Log_outoff,bad_le))
					rc = bad_le->code;
				else if(unpack_le(buf,oep,esize)) {
					rc = esize;
					if (!Log_wrap) {
						if ((Log_outoff <= log_hdr.lh_outoff) &&
						    (log_hdr.lh_inoff > log_hdr.lh_outoff))
							Last_entry = 1;
						else if (Log_outoff <= sizeof(log_hdr)) {
							Log_outoff = log_hdr.lh_staleoff;
							Log_wrap = 1;
						}
					}
					else if((Log_outoff) <= log_hdr.lh_outoff) {
						Last_entry = 1;
					}
				}
				else
					rc = 0;
			}
		}
	}
	return(rc);
}


 /*
  * NAME:      logread()
  * FUNCTION:  Read an error log entry from the supplied offset.
  * RETURNS:
  *             0 Success
  *            -1 Error
  */

int
logread(char *buf, int count, int offset)
{
	int rv;
	int rc;

	if(Logfd < 0) 
		rc = -1;
	else if(lseek(Logfd,offset,0) < 0) {
		perror("logread: lseek");
		rc = -1;
	}
	else if((rv=read(Logfd,buf,count)) < 0) {
		perror("logread: read");
		rc = -1;
	}
	else if(rv != count) {
		cat_print(CAT_LOG_READ_EOF,"logread: unexpected end of file\n");
		cat_print(CAT_LOG_CANT_PROCESS,"Unable to process the error log file %s.\n", Logfile);
		rc = -1;
	}
	else
		rc = 0;
	return(rc);
}


 /*
  * NAME:      logwrite()
  * FUNCTION:  Write an error log entry at the supplied offset.
  * RETURNS:
  *             0 Success
  *            -1 Error
  */

int
logwrite(char *buf, int count, int offset)
{
	int rc;

	if(Logfd < 0)
		return(-1);

	if(lseek(Logfd,offset,0) < 0) {
		perror("logwrite: lseek");
		return(-1);
	}

	if((rc=write(Logfd,buf,count)) < 0) { 
		perror("logwrite: write");
		return(-1);
	}
	else if (rc != count) {
		errno = ENOSPC;
		perror("logwrite: write");
		return(-1);	 	
	}

	return(0);
}


 /*
  * NAME:      logstat()
  * FUNCTION:  Print error log statistics.
  * RETURNS:   None
  */

#define STAT(m) printf("%-15s = %04x\n","m",log_hdr.lh_/**/m)

void
logstat(void)
{

	loginit(0);
	logread(&log_hdr,sizeof(log_hdr),0);
	printf("Logfile is %s\n",Logfile);
	STAT(inoff);
	STAT(outoff);
	STAT(maxsize);
	STAT(sequence);
}


 /*
  * NAME:      hdrlock()
  * FUNCTION:  Lock the error log header.
  * RETURNS:   None
  */

void
hdrlock(void)
{

	if(Log_lockflg)
		return;
	lseek(Logfd,0,0);
	if (lockf(Logfd,F_LOCK,0) == -1) 	/* lock all of file */
		perror("hdrlockf(Logfd,F_LOCK)");
	else
		Log_lockflg = 1;
}


 /*
  * NAME:      hdrunlock()
  * FUNCTION:  Unlock the error log header.
  * RETURNS:   None
  */
void
hdrunlock(void)
{

	if(!Log_lockflg)
		return;
	lseek(Logfd,0,0);
	if (lockf(Logfd,F_ULOCK,0) == -1)	/* unlock all of file */
		perror("hdrunlockf(Logfd,F_ULOCK)");
	else
		Log_lockflg = 0;
}

 /*
  * NAME:      logopen()
  * FUNCTION:  Open an existing error log file in the specified mode.
  *            The opened file will be checked for a valid magic
  *            number denoting an errlog file.
  * RETURNS:
  *            0  Success
  *            -1 Error
  */

int
logopen(int flag)
{
	int	rc;

	if((Logfd=open(Logfile,flag)) < 0){
		perror("logopen: open");
		rc = -1;
	}
	else if(logread(&log_hdr,sizeof(log_hdr),0) ||
	    strncmp(log_hdr.lh_magic,LH_MAGIC,8) != 0 ||
	    !in_range(LH_MINSIZE,log_hdr.lh_maxsize,INT_MAX)) {
		cat_eprint(CAT_LOG_INVALID,"The supplied error log file is not valid: %s.\n",
		    Logfile);
		rc = -1;
	}
	else
		rc = 0;

	return(rc);
}

 /*
  * NAME:      logclose()
  * FUNCTION:  Close an error log file. 
  * RETURNS:   None.
  */

void
logclose(void)
{
	if (close(Logfd) == -1)
		perror("logclose: close");
}

 /*
  * NAME:      logappend()
  * FUNCTION:  Append an entry to an error log.
  * RETURNS:   None.
  */

logappend(char *pack_buf) /* LE_MAX_SIZE bytes max. */
{
	int  esize;
	int  old_wrap_size;
	int  wrap;		/* true when the entry will cause the log to wrap. */

#ifdef DUMPLOG	
	Global_Wrap = FALSE;
#endif  /* DUMPLOG */

	if(Logfd >= 0) {
		hdrlock();
		if(!logread(&log_hdr,sizeof(log_hdr),0)) {

			/*  
			 * The log header was read successfully, so fill in the log_entry
			 * sequence id with the value that was stored in the header.  We
			 * wait until the end to assign the sequence id, in case we don't
			 * use it.
			 */
			memcpy(&pack_buf[8],&log_hdr.lh_sequence,sizeof(log_entry.le_sequence));

#ifdef DUMPLOG
			Global_Sequence = log_hdr.lh_sequence;
#endif  /* DUMPLOG */

			esize = *(int *)pack_buf;		/* size of entry to be logged */
			/*	
			 * The routine set_append_pointer() is called to handle pointer
			 * management by determining if the entry to be logged is going to
			 * cause the logfile to be wrapped and/or data in the file is to be
			 * wrapped (overwritten).  If the file is to be wrapped, the wrap
			 * variable will be set, and lh_inoff (write offset) will have to
			 * be adjusted to point to just past the header.  If data is to
			 * be wrapped, the lh_outoff (read offset) will be adjusted to point
			 * to the next entry not yet overwritten.
			 */
			wrap = set_append_pointer(&log_hdr,esize); 
			if(!logwrite(pack_buf,esize,log_hdr.lh_inoff)) {
			/*
			 * The entry was written successfully.  Call log_hdr_keeping() to 
			 * increment lh_sequence and lh_entries by one and to adjust lh_inoff
			 * to point to just past the entry just logged. 
			 */
				log_hdr_keeping(&log_hdr,esize);
				if (wrap) {
				/*
				 * If the wrap value, returned from set_append_pointer(), is set,
				 * the file was wrapped because it reached its maximum size.  Log
				 * an entry to inform the user that the errlog file has wrapped. 
				 */
#ifdef DUMPLOG
					Global_Wrap = TRUE;
#endif  /* DUMPLOG */
					log_regular_wrap();
				}
			}
			else {	
			/*
			 * The entry was not written successfully.  Assuming the file system
			 * was full, force a file wrap at the current size of the log file.
			 * The set_threshold_at_cur() routine will be called to get the current 
			 * size of the log file for the new lh_maxsize value and to update the 
             * errlg_size attribute in the SWservAt configuration database.
			 */ 
#ifdef DUMPLOG
				Global_Wrap = TRUE;
#endif  /* DUMPLOG */
				
				old_wrap_size = log_hdr.lh_maxsize;
				log_hdr.lh_staleoff = log_hdr.lh_inoff;
				log_hdr.lh_maxsize = set_threshold_at_cur();
				log_hdr.lh_inoff = sizeof(log_hdr);
				/*
				 * The purpose of calling set_append_pointer() at this point is
				 * not to determine if the log file is going to wrap - since that
				 * has just happened - but just to determine if entries are going
				 * to be overwritten and to adjust the lh_outoff (read offset)
				 * accordingly.
				 */
				(void)set_append_pointer(&log_hdr,esize);
				/*
				 * The second write should never fail, since we should have
				 * wrapped to the beginning of the file. Write the entry,
				 * update lh_sequence, lh_entries, and lh_inoff by calling
				 * log_hdr_keeping(), and then log an entry to inform the 
				 * user that the log file has been wrapped prematurely and 
				 * to inform him of the new and old errlog file maxsize values.
				 */
				logwrite(pack_buf,esize,log_hdr.lh_inoff);
				log_hdr_keeping(&log_hdr,esize);
				log_pre_wrap(old_wrap_size);	/* log premature wrap */
			}
		}
		hdrunlock();
		if(!nofsyncflg)
			fsync(Logfd);
	}
}

 /*
  * NAME:      get_log_entry()
  * FUNCTION:  Get the error log entry at the provided file offset.
  * RETURNS:   
  *             0  Bad entry
  *            !0  Good entry
  */

int
get_log_entry(char *buf,struct obj_errlog* oep, int offset,struct le_bad *le_bad)
{
	int			rc;
	unsigned	esize;


	if(logread(&esize,sizeof(log_entry.le_length),offset))
		rc = 0;
	else if(logread(buf,esize,offset))
		rc = 0;
	else {
		if(invalid_le_static(buf,offset,le_bad))
			rc = le_bad->code;
		else if(unpack_le(buf,oep,esize)) {
			rc = esize;
		}
		else
			rc = 0;
	}

	return(rc);
}

 /*
  * NAME:      get_log_entry_rv()
  * FUNCTION:  Get the error log entry in front of the provided file offset.
  * RETURNS:   
  *             0  Bad entry
  *            !0  Good entry
  */

int
get_log_entry_rv(char *buf,struct obj_errlog* oep, int offset,struct le_bad *le_bad)
{
	int			rc;
	unsigned	esize;


	if(logread(&esize,sizeof(log_entry.le_length),offset-sizeof(log_entry.le_length)))
		rc = 0;
	else if(logread(buf,esize,offset-esize))
		rc = 0;
	else {
		if(invalid_le_static(buf,offset,le_bad))
			rc = le_bad->code;
		else if(unpack_le(buf,oep,esize)) {
			rc = esize;
		}
		else
			rc = 0;
	}

	return(rc);
}

 /*
  *	NAME:      get_log_hdr()
  * FUNCTION:  Get the error log header.
  *	RETURNS:
  *            0 - Success
  *            1 - Failure
  */

int
get_log_hdr(struct log_hdr *hdr)
{
	int	rc;

	if(Logfd >= 0) {
		if(logread(hdr,sizeof(log_hdr),0))
			rc = 1;
		else
			rc = 0;
	}
	else
		rc = 1;

	return(rc);
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
	if(lh->lh_sequence == INT_MAX)
		/* cut log entry pertaining to wrapping sequence */
		lh->lh_sequence = 1;
	else
		lh->lh_sequence++;

	if(lh->lh_entries == INT_MAX)
		/* cut log entry pertaining to too many entries. */
		;
	else
		lh->lh_entries++;
	lh->lh_inoff += esize;
#ifdef DUMPLOG
	Global_New_Inoffb = lh->lh_inoff;
#endif  /* DUMPLOG */
	logwrite(lh,sizeof(log_hdr),0);		/* hdr write should never fail */
}

/*
 * NAME:     be_to_le
 * FUNCTION: Convert a buf_entry to a log_entry.  Build an errlog entry from
 *			 the provided buf_entry plus template plus ODM data. 
 *			 Data validation is NOT performed.
 * INPUTS:	 pointer to an error buffer entry.
 * RETURNS:  Pointer to a packed buffer containing the entry to log.
 *			 NULL - Do not log entry; either not loggable or invalid buf entry.
 */

char * 
be_to_le(struct buf_entry* be)
{
	char *p = &pack_buf[4];		/* skip length until sizes known */
	char *class;
	char *subclass;
	char *type;
	int	 tmpltfound;
	int	 length;
	struct utsname uts;
	char *sympptr;
	char *rv;


	if(tmpltflg && tmpltgetbycrcid(be->err_rec.error_id) && !invalid_tmplt(&T_errtmplt)) {
		tmpltfound = 1;
	}
	else {		/* no tmplt found, loggable by default. */
		tmpltfound = 0;
		udb_tmplt_generic_to_le();
		T_errtmplt.et_crcid = be->err_rec.error_id; 
	}
	bzero(&pack_buf,LE_MAX_SIZE);
	if (T_errtmplt.et_logflg){
		*(int *)p = LE_MAGIC;
		p += sizeof(int);

		p += sizeof(int); /* skip sequence until logappend() */

		*(int *)p = be->erec.erec_timestamp;
		p += sizeof(be->erec.erec_timestamp);

		*(int *)p = be->err_rec.error_id;
		p += sizeof(be->err_rec.error_id);

		uname(&uts);
		/* Add machine id to log entry */
		pack_dynamic_ent(strlen(uts.machine)+1,uts.machine,&p);

		/* Add node id to log entry */
		pack_dynamic_ent(strlen(uts.nodename)+1,uts.nodename,&p);

		/* Get the resource name from the buffer_entry. */
		pack_dynamic_ent(strlen(be->err_rec.resource_name)+1,be->err_rec.resource_name,&p);
		/* T_errlog.el_resource will be needed by concurrent notification */
		memcpy(T_errlog.el_resource,be->err_rec.resource_name,strlen(be->err_rec.resource_name)+1);

		if (tmpltfound) {
			if(!strcmp(T_errtmplt.et_class,"H"))
				get_CuVPD_resource(&p,be->err_rec.resource_name);
			else 	/* no vpd available. buf was zero filled so just bump p. */
				p += 2*sizeof(int);		/* ibm_len + user_len */
		}
		else
			get_CuVPD_resource(&p,be->err_rec.resource_name); /* Get the CuVPD object matching resource. */

		/* Get the CuDv object matching 'resource'. */
		CuDv_getbyresource(be->err_rec.resource_name);

		length = strlen(CuDv_location)+1;
		length = limit_range(0,length,LE_IN_MAX);
		pack_dynamic_ent(length,CuDv_location,&p);

		length = strlen(CuDv_connwhere)+1;
		length = limit_range(0,length,LE_CONN_MAX);
		pack_dynamic_ent(length,CuDv_connwhere,&p);

		if (streq(CuDv_rclass,"NONE") ) {
			class    = CuDv_rclass;
			subclass = CuDv_rclass;
			type     = CuDv_rclass;
		}
		else {
			class    = strtok(CuDv_rclass,"/");/* class '/' subclass '/' type */
			subclass = strtok(0,"/");
			type     = strtok(0,"/");
		}

		length = strlen(class)+1;
		length = limit_range(0,length,LE_RCLASS_MAX);
		pack_dynamic_ent(length,class,&p);
		/* T_errlog.el_rclass may be needed by concurrent notification */
		memcpy(T_errlog.el_rclass,class,length);

		length = strlen(type)+1;
		length = limit_range(0,length,LE_RTYPE_MAX);
		pack_dynamic_ent(length,type,&p);
		/* T_errlog.el_rtype may be needed by concurrent notification */
		memcpy(T_errlog.el_rtype,type,length);

		length = be->erec.erec_rec_len - sizeof(struct err_rec0);
		length = limit_range(0,length,LE_DETAIL_MAX);
		pack_dynamic_ent(length,be->detail_data,&p);

		/* The error data length is not fixed therefore  */
		/* need to figure out the end of the data to get */
		/* to the symptom data.                          */
		sympptr = (char *)&be->erec;
		sympptr += be->erec.erec_rec_len + sizeof(struct erec0);

		/* pack symptom data to the error log if any */
		/* exists and the data should be logged.     */
		if ( be->erec.erec_symp_len == 0)
		{
			/* The pointer p points to the data to be */
			/* packed.  At this point we are needing  */
			/* to pack the symptom data, the first    */
			/* item to pack is the length, therefore  */
			/* point p to 0, since no symptom data    */
			/* exists.				     */
			*p = 0;
			p += sizeof(be->erec.erec_symp_len);
			symptomdataflg = FALSE;
		}
		else
		{
			length = be->erec.erec_symp_len;
			pack_dynamic_ent(length, sympptr, &p);
			symptomdataflg = TRUE;
		}

		p += sizeof(unsigned);					/* account for le_length2 */
		length = (int)p - (int)&pack_buf;		/* true length of entry */

		*(int *)pack_buf = length;				 /* le_length */
		*(int *)(p - sizeof(unsigned)) = length; /* le_length2 */

		rv = (char*)&pack_buf;

	}
	else	/* not loggable */
		rv = NULL;
	return(rv);
}

 /*
  * NAME:      init_be_internal()
  * FUNCTION:  Initialize a buf_entry struct for internal error log entry.
  *			   An internal errlog entry is one that the errdemon logs directly,
  *			   rather than throught the errlog() library call.
  *	INPUTS:	   Pointer to a buffer entry.
  *			   A CRCID.
  * RETURNS:   None
  */

void
init_be_internal(struct buf_entry* be, unsigned crcid)
{
	bzero(&be->err_rec.resource_name,ERR_NAMESIZE);
	be->err_rec.error_id = crcid;
	time(&be->erec.erec_timestamp);
	strncpy(be->err_rec.resource_name,ERRDEMON_RESOURCE_NAME,
			strlen(ERRDEMON_RESOURCE_NAME));

	be->erec.erec_len = sizeof(struct erec0) + sizeof(struct err_rec0);
	be->erec.erec_rec_len = sizeof(struct err_rec0);
	be->erec.erec_symp_len = 0;

}

 /*
  * NAME:      log_pre_wrap()
  * FUNCTION:  Log an entry describing the premature log file wrap
  *            that the previous error log record caused.  The entry
  *			   will include the old and new wrap values.
  *			   NOTE:  Do NOT call logappend to log this entry since
  *					  that is where this routine is called from.
  * INPUTS:	   The previous wrap size.
  * RETURNS:   None
  */

void
log_pre_wrap(int prev_wrap_size)
{
	struct buf_entry be;	/* entry to contain wrap record */
	char *le;				/* pointer to packed log entry */
	int esize;				/* entry size in bytes */
	int	det_len;			/* detail data length */

	init_be_internal(&be,ERRID_LOG_PRE_WRAP);

	/* copy old wrap value and new wrap value to detail data.*/

	memcpy(&be.detail_data,&prev_wrap_size,sizeof(log_hdr.lh_maxsize));
	memcpy(&be.detail_data[4],&Threshold,sizeof(log_hdr.lh_maxsize));

	det_len = sizeof(log_hdr.lh_maxsize) + sizeof(log_hdr.lh_maxsize);
	be.erec.erec_len += det_len;
	be.erec.erec_rec_len += det_len;

	/* pack a log_entry from a provided buf_entry */
	if(!valid_be(&be)) { 		/* not valid buffer entry */
		ann_bad_buf_entry(&be);		/* annunciate bad buffer entry */
	}
	else if((le=be_to_le(&be)) != NULL)		/* good log entry built to be logged */
	{
		esize = *(int *)le;
		memcpy(le+8,&log_hdr.lh_sequence,sizeof(log_entry.le_sequence));
		set_append_pointer(&log_hdr,esize);
		logwrite(le,esize,log_hdr.lh_inoff);	/* this will never fail */
		log_hdr_keeping(&log_hdr,esize);
	}


}

 /*
  * NAME:      log_regular_wrap()
  * FUNCTION:  Log an entry describing a regular log file wrap.
  *			   NOTE:  Do NOT call logappend to log this entry since
  *					  that is where this routine is called from.
  * INPUTS:	   NONE
  * RETURNS:   None
  */

void
log_regular_wrap(void)
{
	struct buf_entry be;	/* entry to contain wrap record */
	char* le;				/* pointer to packed log entry */
	int esize;				/* log entry size */

	init_be_internal(&be,ERRID_LOG_REG_WRAP);

	/* pack a log_entry from a provided buf_entry */
	if(!valid_be(&be)) { 		/* not valid buffer entry */
		ann_bad_buf_entry(&be);		/* annunciate bad buffer entry */
	}
	else if((le=be_to_le(&be)) != NULL)		/* good log entry built to be logged */
	{
		esize = *(int *)le;
		memcpy(le+8,&log_hdr.lh_sequence,sizeof(log_entry.le_sequence));
		set_append_pointer(&log_hdr,esize);
		logwrite(le,esize,log_hdr.lh_inoff);	/* this will never fail */
		log_hdr_keeping(&log_hdr,esize);
	}

}

/*
 * NAME:	getlogpath()
 * FUNCTION:    Get the current value of the "errlg_file"
 *		attribute from ODM, and set Logfile to that value.
 * RETURNS:	None
 */

void
getlogpath()
{
	struct SWservAt *swservp;
	int number;

	if (0 > odm_initialize())
	{
		cat_eprint(CAT_ODM_INIT, "Cannot initialize ODM.\n\
Using default value %s for error log file.\n", ERRLOG_DFLT);
		strcpy(Logfile,ERRLOG_DFLT);
		return;
	}
	odm_set_path(ODM_DATABASE);

	swservp = ras_getattr("errlg_file", 0, &number);

	if (swservp == NULL)
	{
		cat_eprint(CAT_PATH_GETATTR,
		    "Unable to get %s attribute from ODM object class SWservAt.\n\
Using default value %s for error log file.\n",
		    "errlg_file", ERRLOG_DFLT);
		strcpy(Logfile,ERRLOG_DFLT);
	}
	else
	{
		strcpy(Logfile,swservp->value);
		free(swservp);
	}

	if (0 > odm_terminate())
		cat_fatal(CAT_ODM_TERM, "Unable to terminate ODM.\n");
}

/*
 * NAME:      do_odm_hdr_size
 * FUNCTION:  Check the ODM errlg_size against the log header lh_maxsize.
 *			  Set ODM to the header value if they are not equal.
 *			  Issue error msgs for invalid value in ODM.
 * INPUTS:	  None
 * RETURNS:   None
 */
  
static void
do_odm_hdr_size(void)
{
	struct SWservAt *swservp;
	int number;
	int lock;
	int size;
	char str[26];

	if (0 > odm_initialize()) 
		cat_eprint(CAT_ODM_INIT, "Cannot initialize ODM.\n");
	else 
	{
		odm_set_path(ODM_DATABASE);
		if((swservp = ras_getattr("errlg_size", 0, &number)) == NULL)
		{
			cat_eprint(CAT_GET_ATTR_FSIZE_USE_HDR,
"Unable to get the error log file size from the ODM object class SWservAt.\n\
Using the value %d from the header of errlog file %s for error log size.\n",
						log_hdr.lh_maxsize,Logfile);
		}
		else 	/* got an attribute value */
		{
			size = atoi(swservp->value);
			if(size == 0 || size == LONG_MAX || size == LONG_MIN)  /* conversion error */
				perror("do_odm_hdr_size: size = atoi(swservp->value)");

			if(!in_range(LH_MINSIZE,size,INT_MAX))
				cat_eprint(CAT_SW_ATTR_BAD,
			"The ODM object class SWservAT attribute %s contained an unacceptable value: %s\n"
					"errlg_size",swservp->value);

			if(log_hdr.lh_maxsize != size)  	/* set ODM to existing file hdr size */
			{
				cat_eprint(CAT_FSIZE_USE_FILE,"\
The supplied errlog header contains a file size different from ODM.\n\
Updating ODM to use the supplied header file size %d.\n",log_hdr.lh_maxsize);

				sprintf(str,"%d",log_hdr.lh_maxsize);
				swservp->value = str;	
				if ((lock = odm_lock("/etc/objrepos/config_lock",5)) == -1)
					cat_eprint(CAT_ODM_LOCK, "Unable to lock ODM.\n");

				else if(ras_putattr(swservp) == -1) 			 /* write the threshold to ODM */
				{
					cat_eprint(CAT_PUT_ATTR,
				"Unable to update the log file maximum size in the ODM object class SWservAt.\n");
				}
				else
					savebase();			/* make ODM persistant across boots   */

				if (odm_unlock(lock) == -1)
					cat_eprint(CAT_ODM_UNLOCK, "Unable to unlock ODM.\n");
			}
			free(swservp);	  	/* free space malloced by ras_getattr */
		}
		if (0 > odm_terminate())
			cat_eprint(CAT_ODM_TERM,"errdemon: Unable to terminate ODM.\n");
	}
}

/*
 * NAME:	 log_odm_error()
 * FUNCTION: Log an entry through the device driver for odm failure 
 *			 while setting a SWservAt attribute.
 * INPUTS:	 Pointer to a string representation of an odm attribute.
 *			 Pointer to a string representation of an odm operation.
 *			 A copy of the errno resulting from the odm operation.
 * RETURNS:	 None
 */

void
log_odm_error(char* attr,	/* odm attribute */
			  char* opstr,	/* operation string */
			  int err		/* errno to log */
			  )
{
	struct odm_err {
		struct err_rec0 err_rec;
		char detail_data[ATTR_SIZE+OPSTR_SIZE];
	} odme;

	odme.err_rec.error_id = ERRID_ODM_CHANGE_SWSERVAT;
	strncpy(odme.err_rec.resource_name,ERRDEMON_RESOURCE_NAME,
			sizeof(ERRDEMON_RESOURCE_NAME));

	memcpy(&odme.detail_data,attr,ATTR_SIZE);
	memcpy(&odme.detail_data[ATTR_SIZE],opstr,OPSTR_SIZE);
	memcpy(&odme.detail_data[ATTR_SIZE+OPSTR_SIZE],&err,sizeof(int));

	if(errlog(&odme,sizeof(struct odm_err)))
		perror("log_odm_error: errlog");
}
/*
 * NAME:	setlogpath()
 * FUNCTION:    Sets the attribute "errlg_file" in ODM to the
 *		current value of Logfile.  This is called only
 *		by errdemon when the -i option is invoked for
 *		the user to input an alternate error log file.
 * RETURNS:	None
 */

void
setlogpath()
{
	struct SWservAt *swservp;
	struct stat statbuf;
	char new_Logfile[256];
	char Logfile_dirname[256];
	int number;
	int lock;
	int	exists=FALSE;		/* true when supplied errlog already exists, and is proper. */
	int sverr;

	/* Check for relative pathnames here. */
	if (strncmp(Logfile,"/",1) != 0)
	{
		/* If the logfile is a directory, just get the current
                   directory.  This will give an error later on. */
		if ((stat(Logfile,&statbuf) == 0) &&
		    (S_ISDIR(statbuf.st_mode)))
			getcwd(Logfile_dirname,BUFSIZ);
		else
		{
			/* The logfile exists. */
			/* Get the current directory,
                           and put it in Logfile_dirname. */
			if (stat(dirname(Logfile),&statbuf) == 0)
			{
				chdir(dirname(Logfile));
				getcwd(Logfile_dirname,BUFSIZ);
			}
			else
			{
				/* The logfile doesn't exist. */
				/* Get the current directory. */
				getcwd(Logfile_dirname,BUFSIZ);
				/* Check to see if the relative pathname includes
                                   directories. */
				if (strcmp(Logfile,dirname(Logfile)) != 0)
				{
					if (strcmp("/",Logfile_dirname) != 0)
						strcat(Logfile_dirname,"/");
					strcat(Logfile_dirname,dirname(Logfile));
				}
			}
		}
		/* Make sure the basename is preceded by a "/". */
		/* Concatenate the dirname with the basename.   */
		/* Logfile now contains the full pathname.      */
		if (strcmp("/",Logfile_dirname) != 0)
			strcat(Logfile_dirname,"/");
		strcat(Logfile_dirname, basename(Logfile));
		strcpy(Logfile, Logfile_dirname);
	}

	/* Check that the error log file given from the user exists.
	   If the full path does not exist, check for the directory. 
	   If the directory exists, then create the file.  Otherwise,
	   call getlogpath() and return with a warning. */

	if (stat(Logfile,&statbuf) != 0) {
		strcpy(new_Logfile,Logfile);
		if (stat(dirname(Logfile),&statbuf) != 0) {
			getlogpath();
			cat_warn(CAT_INVALID_PATH, "\
The path %s does not exist.\n\
Using default logfile %s from ODM.\n",
			    dirname(new_Logfile), Logfile);
			return;
		}
	}
	else
		exists = TRUE;

	/* Make sure the file is a valid error log file. 
	   Don't create or initialize if it exists. */
	if(exists) { 
		if(logopen(O_RDWR) != 0)	/* not a valid errlog file */ 
			exit(1);
		else /* Perform consistency check for hdr_size against ODM_size */
			do_odm_hdr_size();
	}
	else if (loginit(1) < 0) {		/* will create log if it D.N.E. */
		strcpy(new_Logfile,Logfile);
		getlogpath();
		cat_warn(CAT_INVALID_LOG, "\
Cannot initialize error log file %s.\n\
The error log file does not have the correct permissions, or is not a\n\
valid error log file.  Using default error log file %s from ODM.\n",
		    new_Logfile,Logfile);
		return;
	}

/*
	logclose();
*/


	/* The error log file input is valid.  Let's put it in ODM. */
	if (0 > odm_initialize()) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_INIT,sverr);
		cat_fatal(CAT_ODM_INIT, "Cannot initialize ODM.\n");
	}
	odm_set_path(ODM_DATABASE);

	swservp = ras_getattr("errlg_file", 0, &number);

	if (swservp == NULL) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_READ,sverr);
		cat_fatal(CAT_PATH_GETATTR,
		    "Unable to get %s attribute from ODM object class SWservAt.\n", "errlg_file");
	}
	else
		swservp->value = Logfile;

	if ((lock = odm_lock("/etc/objrepos/config_lock",5)) == -1) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_LOCK,sverr);
		cat_fatal(CAT_ODM_LOCK,
		    "Unable to lock ODM.\n");
	}

	if (ras_putattr(swservp) == -1) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_WRITE,sverr);
		cat_fatal(CAT_PATH_PUTATTR,
		    "Unable to update %s attribute in ODM object class SWservAt.\n","errlg_file");
	}

	if (odm_unlock(lock) == -1) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_UNLOCK,sverr);
		cat_fatal(CAT_ODM_UNLOCK,
		    "Unable to unlock ODM.\n");
	}

	if (0 > odm_terminate()) {
		sverr = errno;
		log_odm_error(ERRLG_FILE,ODM_TERM,sverr);
		cat_fatal(CAT_ODM_TERM,"errdemon: Unable to terminate ODM.\n");
	}


	free(swservp);	  	/* free space malloced by ras_getattr */


	savebase();		/* make ODM persistant across boots   */

	errstop();		/* stop the demon		      */

	sleep(3);		/* give the child some time to finish */

}

