static char sccsid[] = "@(#)13        1.10.1.1  src/bos/usr/bin/errlg/liberrlg/threshold.c, cmderrlg, bos41J, 9507C 2/6/95 16:12:59";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: ch_threshold, show_threshold, get_threshold, set_threshold_at_cur
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file contains functions to change, show, get, and set the threshold
 * value for the error log file.  The threshold value is stored in ODM.
 */

#include <fcntl.h>
#include <sys/lockf.h>
#include <sys/param.h>
#include <ulimit.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include <errno.h>
#include <sys/erec.h>
#include <cmderrlg_msg.h>
#include <errlg.h>
#include <errlg/SWservAt.h>

int		ch_threshold(char *str);
void	show_threshold(void);
int		get_threshold(void);
int		set_threshold_at_cur(void);

extern logread();
extern hdrlock();
extern logwrite();
extern void log_odm_error();

extern	char Logfile[PATH_MAX];
extern	int Logfd;
extern	struct log_hdr   log_hdr;

struct	SWservAt*	swservp; /* odm record for errlog file size */
extern  int Threshold;


/*
 * NAME:      ch_threshold
 * FUNCTION:  Change the error log file threshold value.
 * RETURNS:   0 - bad threshold value supplied.
 *            1 - threshold value changed ok.
 *            2 - supplied threshold == odm.
 *            Exit on unrecoverable errors.
 */

int
ch_threshold(char *str)
{
	struct	stat	sbuf;
	int 		cnt;
	int		lock;
	int		tmp;
	int		size;
	off_t	limit;		/* ulimit value, multiplied by UBSIZE giving bytes */
	int		 sverr;
	char	        *old_Logfile;

	limit = ulimit(GET_FSIZE,0) * UBSIZE;
	if((tmp=atoi(str)) < LH_MINSIZE || tmp > (int)limit) /* range: min <=> ulimit */
		return(0);

	if(Threshold == tmp) {	 /* no action necessary */
		odm_terminate();
		return(2);
	}
	else
		Threshold = tmp;

	if ((swservp=ras_getattr("errlg_size", 0, &cnt)) == NULL) {
		sverr = errno;
		log_odm_error(ERRLG_SIZE,ODM_READ,sverr);
		cat_fatal(CAT_GET_ATTR_FSIZE, 
		"Unable to get the log file size from the ODM object class SWservAt.\n");
	}
		

	if((lock=odm_lock("/etc/objrepos/config_lock",5)) == -1) {
		sverr = errno;
		log_odm_error(ERRLG_SIZE,ODM_LOCK,sverr);
		odm_terminate();
		cat_fatal(CAT_ODM_LOCK,
			"Unable to lock ODM after waiting 5 seconds.\n\tTry again later.");
	}

	swservp->value = str;
	if(ras_putattr(swservp) == -1) {     /* write the threshold to ODM */
		sverr = errno;
		log_odm_error(ERRLG_SIZE,ODM_WRITE,sverr);
		odm_terminate();
		cat_fatal(CAT_PUT_ATTR,
			"Unable to update the log file size in ODM object class SWservAt.\n");
	}

	if(odm_unlock(lock) == -1) {
		sverr = errno;
		log_odm_error(ERRLG_SIZE,ODM_UNLOCK,sverr);
		odm_terminate();
		cat_fatal(CAT_ODM_UNLOCK,
			"Unable to unlock ODM.\n");
	}

	free(swservp);

	savebase();			/* output to boot record so new threshold
						   is persistent across boots */

	errstop();							/* stop the demon.child */
	sleep(3);							/* give 'child' a chance to finish */


	if(stat(Logfile,&sbuf) == -1)		/* doesn't exist */
		loginit(1);					/* create and initialize logfile */

	else if(sbuf.st_size > Threshold) {		/* make a copy, so no loss of data */
		size = strlen(Logfile) + 5;
		if ((old_Logfile = malloc(size)) == 0)
			{
			perror("malloc: old_Logfile");
			odm_terminate();
			exit(1);
			}
		strcpy(old_Logfile,Logfile);
		strcat(old_Logfile,".old");
		if (rename(Logfile,old_Logfile) == -1) {
			perror("rename(Logfile,old_Logfile)");
			odm_terminate();
			exit(1);
		}
		cat_print(CAT_LOG_CPY,
			"Decreasing the error log file maximum size caused %s\n\tto be moved to %s.\n"
			,Logfile,old_Logfile);
		loginit(1);					/* create and initialize logfile */
	}
	else {			/* since current size is less than requested,
					 * just change threshold value for header, ODM updated above. */
		if((Logfd = open(Logfile,O_RDWR)) < 0) {
			cat_fatal(CAT_DEM_DATABASE,
"Failure to open the logfile '%s' for writing.\n\
Possible causes are:\n\
1. The file exists but the invoking process does not have write\n\
   permission. \n\
2. The file exists but the directory '%s' does not have write\n\
   permission.\n\
3. The file exists but is not a valid error logfile.  Remove it and rerun\n\
   the errdemon.\n\
4. The file exists but the directory %s does not have enough space\n\
   available.  The minimum logfile size is %d bytes.\n", Logfile, dirname(Logfile), dirname(Logfile), LH_MINSIZE);

			return(0);
		}

		logread(&log_hdr,sizeof(log_hdr),0);		/* get current header info */
		log_hdr.lh_maxsize = Threshold;
		hdrlock();
		if(logwrite(&log_hdr,sizeof(log_hdr),0)) {
			cat_fatal(CAT_LOG_HDR_WRITE,
				"errdemon: Unable to update errlog header for error log file %s.\n", Logfile);
		}
		hdrunlock();
	}
	
	free(old_Logfile);
	close(Logfd);
	return(1);
}

/*
 *  get_threshold()
 *	Get the threshold value for the error log.
 * 	This sets the global variable 'Threshold' to the value stored
 *	in ODM for 'errlg_size'.  If ODM is not accessible, we use the
 *	default threshold value.  
 */

int
get_threshold(void)
{
	int	cnt;		/* count of items returned by ras_getattr(); */
	char *str;
			 /* get ODM ready */
	if(odm_initialize() < 0) 
		{
		cat_eprint(CAT_ODM_INIT, "Unable to initialize ODM.\n\
Using default value %d for error log size.\n", THRESHOLD_DFLT);
		Threshold = THRESHOLD_DFLT;
		return;
		}
	odm_set_path(ODM_DATABASE);

			/* get threshold from ODM */
	if ((swservp=ras_getattr("errlg_size", 0, &cnt)) == NULL)
		{
		cat_eprint(CAT_GET_ATTR_FSIZE,
			"Unable to get the log file size from the ODM object class SWservAt.\n");
		Threshold = THRESHOLD_DFLT;
		}
	else
		{
		str = stracpy(swservp->value);
		if (!(Threshold = atoi(str)))
			Threshold = THRESHOLD_DFLT;	
		jfree(str);
		free(swservp);
		}

	if (odm_terminate() < 0)
		cat_eprint(CAT_ODM_TERM, "Unable to terminate ODM.\n");
}


/*
 * NAME:      set_threshold_at_cur()
 * FUNCTION:  Set the errlog threshold at the current error log size.
 * RETURNS:
 *            < 0 - error
 *            >=0 - The current file size to begin wrapping at.
 */

int
set_threshold_at_cur(void)
{
	struct	stat	sbuf;
	int		rc;
	int		lock;
	int 		cnt;
	char	str[26];	/* 25 digits max in log file max. */

	if(stat(Logfile,&sbuf) == -1)		/* doesn't exist */
		rc = -1;
	else if(odm_initialize() == -1) {
		cat_fatal(CAT_ODM_INIT,"Unable to initialize ODM.\n");
		rc = -1;
	}
	else if( odm_set_path(ODM_DATABASE) && 
		((swservp=ras_getattr("errlg_size", 0, &cnt)) == NULL) ) {
		odm_terminate();
		cat_fatal(CAT_GET_ATTR_FSIZE,
		"Unable to get the log file size from the ODM object class SWservAt.\n");
	}
	else if((lock=odm_lock("/etc/objrepos/config_lock",5)) == -1) {
			odm_terminate();
			cat_fatal(CAT_ODM_LOCK,
			"Unable to lock ODM after waiting 5 seconds.\n\tTry again later.");
			rc = -1;
	}
	else {
		sprintf(str,"%d",sbuf.st_size);
		swservp->value = str;
		if(ras_putattr(swservp) == -1) {			 /* write the threshold to ODM */
			odm_terminate();
			cat_fatal(CAT_PUT_ATTR,
				"Unable to update the log file maximum size in the ODM object class SWservAt.\n");
			rc = -1;
		}
		else if(odm_unlock(lock) == -1) {
			odm_terminate();
			cat_fatal(CAT_ODM_UNLOCK,
				"errdemon: Unable to unlock ODM.\n");
			rc = -1;
		}
		else {
			Threshold = sbuf.st_size;
			savebase();		/* output to boot record so new threshold
								   is persistent across boots */
			rc = sbuf.st_size;
		}
	}
	odm_terminate();
	return(rc);
}
