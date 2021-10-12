static char sccsid[] = "@(#)15  1.5  src/bos/usr/bin/errlg/liberrlg/udbutil.c, cmderrlg, bos411, 9428A410j 6/17/94 15:51:45";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: udb_logget, udb_init, udb_loginsert, udb_logread, udb_tmpltget,
 *            udb_tmplt_to_le, udb_tmplt_generic_to_le,  udb_logget_rv,
 *            udb_get_log_entry, udb_get_log_entry_rv            
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/erec.h>
#include <errlg.h>

extern int tmpltflg;
struct obj_errlog T_errlog;
struct obj_errtmplt T_errtmplt;
char* Progname;
extern  char    Logfile[PATH_MAX];


/*
 * NAME:      udb_init
 * FUNCTION:  Open an error template file, initialize concurrent
 *            notification, and open error log file
 * RETURNS:   None, exit on unrecoverable error.
 */

void
udb_init()
{

	if(tmpltinit(0) < 0) {	/* open errtmplt */
		cat_error(M(CAT_NO_TMPLT_FILE),
			"The error template repository file %s is either unreadable, or\ndoes not exist.\n",
				 gettmpltfile());
		tmpltflg = 0;
	}
	else
		tmpltflg = 1;

	notifyinit();			/* open errnotify and customized devices */
	if(loginit(1) < 0) {  	/* open errlog */
		cat_eprint(0,"%s:\n",Progname);
		cat_eprint(CAT_DEM_DATABASE,"\
Failure to open the logfile '%s' for writing.\n\
Possible causes are:\n\
1. The file exists but the invoking process does not have write\n\
   permission. \n\
2. The file exists but the directory '%s' does not have write\n\
   permission.\n\
3. The file exists but is not a valid error logfile.  Remove the file\n\
   and restart the errdemon.\n\
4. The file does exist and the directory '%s' does not have enough\n\
   space available. The minimum logfile size is %d bytes.\n",
Logfile,dirname(Logfile),dirname(Logfile),LH_MINSIZE);
		strcpy(Logfile,ERRLOG_DFLT);
		if(loginit(1) < 0)
			exit(1);
		else {
			cat_eprint(CAT_PATH_GETATTR,
"Using default value %s for error log file.\n",ERRLOG_DFLT);
		setlogpath();
		}	
	}
	return;
}

/*
 * append entry to log
 */
udb_loginsert()
{
	int rv;
	int esize;

	esize = OFFC(obj_errlog,el_detail_data) + T_errlog.el_detail_length;
	rv = logappend(&T_errlog,esize,&T_errlog.el_sequence);
	return(rv);

}


/*
 * NAME:      udb_tmplt_to_le()
 * FUNCTION:  Copy the label, type, and class from a given error template to
 *            to a given errlog entry.
 * RETURNS:   None
 */

static void
udb_tmplt_to_le()
{

	memcpy(T_errlog.el_label,T_errtmplt.et_label,LE_LABEL_MAX);
	memcpy(T_errlog.el_type,T_errtmplt.et_type,LE_TYPE_MAX);
	memcpy(T_errlog.el_class,T_errtmplt.et_class,LE_CLASS_MAX);
}


/*
 * NAME:      udb_tmplt_generic_to_le()
 * FUNCTION:  Set T_errlog and T_errtmplt members to defaults
 *            when no template was found.
 * RETURNS:   None
 */

void
udb_tmplt_generic_to_le()
{
	int i;

	memcpy(T_errlog.el_label,"NONE",strlen("NONE"));
	memcpy(T_errlog.el_class,"U",strlen("U"));			/* UNDETERMINED */
	memcpy(T_errlog.el_type,"NONE",strlen("NONE"));
	memcpy(T_errlog.el_vpd_ibm,"NONE",strlen("NONE"));
	memcpy(T_errlog.el_vpd_user,"NONE",strlen("NONE"));

	memcpy(T_errtmplt.et_label,"NONE",strlen("NONE"));
	memcpy(T_errtmplt.et_class,"U",strlen("U"));			/* UNDETERMINED */
	memcpy(T_errtmplt.et_type,"NONE",strlen("NONE"));

	for (i=0; i<4; i++) {
		T_errtmplt.et_probcauses[i] = 0xffff;
		T_errtmplt.et_usercauses[i] = 0xffff;
		T_errtmplt.et_useraction[i] = 0xffff;
		T_errtmplt.et_instcauses[i] = 0xffff;
		T_errtmplt.et_instaction[i] = 0xffff;
		T_errtmplt.et_failcauses[i] = 0xffff;
		T_errtmplt.et_failaction[i] = 0xffff;
		T_errtmplt.et_detail_length[i]   = 0xffff;
		T_errtmplt.et_detail_length[4+i] = 0xffff;
		T_errtmplt.et_detail_descid[i]   = 0xffff;
		T_errtmplt.et_detail_descid[4+i] = 0xffff;
	}
	T_errtmplt.et_logflg = 1;			/* loggable */
	T_errtmplt.et_reportflg = 1;		/* reportable */
	T_errtmplt.et_alertflg = 0;			/* not alertable */
}


/*
 * NAME:      udb_logget
 * FUNCTION:  read an error log entry, and format it into the structure
 *            T_errlog.
 * RETURNS:    (sizeof(struct T_errlog))	success.
 *             0                            EOF.
 */

udb_logget(char *buf,struct le_bad *le_bad)
{
	int rv;

	rv = logget(&T_errlog,buf,le_bad);	/* rv = size of T_errlog */
	if(rv <= 0)
		rv = 0;
	else if(le_bad->code)
		;		/* bad log entry */
	else {
			/* tmplate repository available */
		if (tmpltflg && tmpltgetbycrcid(T_errlog.el_crcid))
			udb_tmplt_to_le();
		else {	/* no tmplt available */
			udb_tmplt_generic_to_le();
			T_errtmplt.et_crcid = T_errlog.el_crcid;
		}
	}

 	return(rv);
}


/*
 * NAME:      udb_logget_rv
 * FUNCTION:  read an error log entry 'errlog' into
 *            the structure 'T_errlog'. This read operation
 *			  is begun at the bottom of the error log, and
 *			  the circular error log pointer management assumes
 *			  a reverse read. 
 * RETURNS:    (sizeof(struct T_errlog))	success.
 *             0    no more error log objects, EOF.
 */

udb_logget_rv(char *buf,struct le_bad *le_bad)
{
	int rv;

	rv = logget_rv(&T_errlog,buf,le_bad);	/* rv = size of T_errlog */
	if(rv <= 0)
		rv = 0;
	else if(le_bad->code)
		;			/* bad log entry */
	else {
			/* tmplate repository available */
		if (tmpltflg && tmpltgetbycrcid(T_errlog.el_crcid))
			udb_tmplt_to_le();
		else {	/* no tmplt available */
			udb_tmplt_generic_to_le();
			T_errtmplt.et_crcid = T_errlog.el_crcid;
		}
	}

 	return(rv);
}


/*
 * NAME:      udb_get_log_entry()
 * FUNCTION:  Fetch the log entry at the provided offset in the
 *            error log file, and load it into T_errlg;
 *	Returns:
 *		0 - successful.
 *		1 - failure, no valid log entry at that offset.
 */

int
udb_get_log_entry(char *buf, int offset,struct le_bad *le_bad)
{
	int rc;
	if((rc=get_log_entry(buf,&T_errlog,offset,le_bad)) && !le_bad->code) {
		if(tmpltflg && tmpltgetbycrcid(T_errlog.el_crcid))
			 udb_tmplt_to_le();
		else {	/* no tmplt available */
			udb_tmplt_generic_to_le();
			T_errtmplt.et_crcid = T_errlog.el_crcid;
		}
	}
	
	return(rc);
}


/*
 * NAME:     udb_get_log_entry_rv()
 * FUNCTION: Fetch the log entry in front of the provided offset
 *           in the error log file, and load it into T_errlg;
 * RETURNS:  0 - successful.
 *           1 - failure, no valid log entry at that offset.
 */

int
udb_get_log_entry_rv(char *buf, int offset,struct le_bad *le_bad)
{
	int rc;
	if((rc=get_log_entry_rv(buf,&T_errlog,offset,le_bad)) && !le_bad->code) {
		if(tmpltflg && tmpltgetbycrcid(T_errlog.el_crcid))
			 udb_tmplt_to_le();
		else {	/* no tmplt available */
			udb_tmplt_generic_to_le();
			T_errtmplt.et_crcid = T_errlog.el_crcid;
		}
	}
	
	return(rc);
}
