static char sccsid[] = "@(#)05        1.6  src/bos/usr/bin/errlg/liberrlg/child.c, cmderrlg, bos411, 9428A410j 3/15/94 17:08:39";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: childexit,log,get_CuVPD_resource,childsignal,siginit,log_onoff
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

/*
 * The child specific errdemon & errdead code is here.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/erec.h>
#include <sys/cfgdb.h>		/* get VPDSIZE */
#include <sys/time.h>
#include <sys/rasprobe.h>
#include <errlg.h>

extern char* be_to_le();
extern int tmpltgetbycrcid();
extern char Logfile[PATH_MAX];
extern char *getenv();
extern void udb_tmplt_generic_to_le();


void    log(struct buf_entry *be);
void	get_CuVPD_resource(int *buf, char *resource);
static	void childsignal(int signo);
int     childexit(int exitcode);
void    siginit(void);
void    log_onoff(unsigned crcid);

#define FIRST 1
#define NEXT  0
#define CONSOLE_DFLT "/dev/console"
static char *Console = CONSOLE_DFLT;

/*
 * copy from a to b. null terminate b
 */
#define VCPY(a,b) \
   memcpy(b,a,MIN(sizeof(a),sizeof(b))); ((char *)(b))[sizeof(b)-1] = '\0';

#define ECPY(a,b) \
        strcpy(b,a);
/*       strncpy(b,a,strlen(a));  */

#define SCPY(from,to_array) \
        strncpy(to_array,from,sizeof(to_array))

/* NOTE: VPD data may contain embedded nulls. e.g. token */
#define VPDCPY(from,to_array) \
        bcopy(from,to_array,VPDSIZE)

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
 * NAME:     log()
 * FUNCTION: Build an error log entry from the given buf_entry +
 *           template + ODM data.  Construct a statically allocated
 *           log entry in the order the data occurs for convenience,
 *           except for the length (which must be saved until all
 *           dynamic sizes are known) and the sequence id.  The sequence id
 *           will be added by the logappend routine.
 *           We will attempt to construct the given entry, limiting
 *           dynamic lengths to their proper ranges.
 *
 * RETURNS:	 None
 */


void
log(struct buf_entry *be)
{
	char *le;		/* pointer to packed log_entry returned from be_to_le */


	if((le=be_to_le(be)) != NULL)		/* If good, put in the errlog */
	{
		/* write log entry to the database */
		if (logappend(le) < 0)	/* could not write it... */
		{
			ERR_TRCHKL(ERRDEMON,0,be->err_rec.error_id);
		}
		else
		{
			memcpy(&T_errlog.el_sequence,le+8,sizeof(T_errlog.el_sequence));
			notify();		 /* Notify waiting processes of new entry. */
		}
	}
}

/*
 * NAME:     get_CuVPD_resource
 * FUNCTION: Get the CuVPD object from ODM matching the supplied resource.
 * RETURNS:  None
 */

void
get_CuVPD_resource(int *buf, char *resource)
{
	int	size;

	/* Get the CuVPD object matching el_resource. */
	CuVPD_getbyresource(resource);	 /* IBM's */
	if(strncmp(CuVPD_vpd,"NONE",4) == 0)
		size = 5;
	else
		size = VPDSIZE;

	memcpy(*buf,&size,sizeof(int));	/* vpd_ibm_size */
	*buf += sizeof(int);
	memcpy(*buf,CuVPD_vpd,size);	/* vpd_ibm */
	*buf += size;

	if (CuVPD_vpd) {
		CuVPD_get(""); /* User's */
		if(strncmp(CuVPD_vpd,"NONE",4) == 0)
			size = 5;
		else
			size = VPDSIZE;

		memcpy(*buf,&size,sizeof(int));	/* vpd_user_size */
		*buf += sizeof(int);
		memcpy(*buf,CuVPD_vpd,size);		/* vpd_user */
		*buf += size;

	}
}

/*
 * NAME:     childsignal
 * FUNCTION: Signal handler for errdemon or errdead children.
 * RETURNS:  None
 */

static void childsignal(int signo)
{
	int rv;

	rv = 1;
	switch(signo) {
	case SIGUSR1:
		childexit(rv);
	case SIGQUIT:
	case SIGTERM:
	case SIGHUP:
		childexit(rv);
	default:
		childexit(rv);
	}
	signal(signo,(void(*)(int)) childsignal);
}

/*
 * NAME:     childexit
 * FUNCTION: Exit for errdemon children.
 * RETURNS:  None
 */

int
childexit(int exitcode)
{

	log_onoff(ERRID_ERRLOG_OFF);
	close(Errorfd);
	_exit(exitcode);
}

/*
 * NAME:     siginit
 * FUNCTION: Initialize signal handling for errdemon and errdead children.
 * RETURNS:  None
 */

void
siginit(void)
{

	signal(SIGINT, SIG_IGN);                /* ignore INT. It comm's to the parent */
	signal(SIGQUIT,(void(*)(int)) childsignal);     /* terminate, immediately */
	signal(SIGTERM,(void(*)(int)) childsignal);     /* terminate, immediately */
	signal(SIGUSR1,(void(*)(int)) childsignal);     /* terminate by forcing early EOF */
	signal(SIGHUP, (void(*)(int)) childsignal);     /* terminate, immediately */
	signal(SIGCLD, SIG_IGN);                /* no defuncts from methods */
}

/*
 * NAME:     log_onoff
 * FUNCTION: Log an "on" or "off" record for errdemon children.
 * RETURNS:  None
 */

void
log_onoff(unsigned crcid)
{
	time_t t;
	struct buf_entry e;

	e.err_rec.error_id = crcid;
	time(&e.erec.erec_timestamp);
	SCPY("errdemon",e.err_rec.resource_name);
	e.erec.erec_len = sizeof(struct erec0) + sizeof(struct err_rec0);
        e.erec.erec_rec_len = sizeof(struct err_rec0);
        /* Set the symptom length to 0, since the log on/off */
        /* record does not contain symptom data.             */
        e.erec.erec_symp_len = 0;

	log(&e);
}

