static char sccsid[] = "@(#)00  1.2  src/bos/usr/bin/errlg/convert_errlog/main.c, cmderrlg, bos411, 9428A410j 4/22/94 13:39:09";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main
 *            
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/mdio.h>
#include <locale.h>
#include <signal.h>
#include <memory.h>
#include <errno.h>
#include <sys/erec.h>
#include <errlg.h>
#include "convert_errlog.h"

int	tmpltflg;		/* true then template repository available */
int Errorfd;      /* /dev/error */

/* v4 stuff */
char *v4log = NULL;
int Logfd;
struct log_hdr   log_hdr;

/* v3 stuff */
char *v3log = NULL;
char *v3log_temp;
int v3Logfd;
struct v3_log_hdr v3_log_hdr;
int v3_Log_outoff;


char  *Progname;					/* the program name, argv[0] */
int   Threshold;					/* value, in bytes, for the logfile to wrap at */


static	void	convert_log(void);
static	void	v3_to_v4_entry(struct v3_log_entry *v3_le, char *v4_le);
static	int		pack_dynamic_ent(int length,char* buf,int* p);
static	off_t	enough_space(void);
static	int		open_v3_log(void);
static	int		get_v3_hdr(void);
static	int		open_v4_log(void);
static	int		set_v4_hdr(void);
static	void	close_v3_log(void);
static	void	close_v4_log(void);

extern	int		v3_logget();
extern	int     limit_range(int min,int val,int max);
/*
 * NAME:     main
 * FUNCTION: 
 */

main(int argc, char *argv[])
{
	off_t rs;						/* required space */
	int c;
	int rv;
	char * bos_level;
	char mv_cmdline[80];

	setprogname();					/* set program name to basename of argv[0] */
	setlocale(LC_ALL,"");			/* This sets the environment  */

	while ((c=getopt(argc,argv,"i:o:")) != EOF )
	{
		switch(c) {
		case 'i':
			v3log = stracpy(optarg);
			break;
		case 'o':
			v4log = stracpy(optarg);
			break;
		default:
			usage();
		}
	}

	if (v3log == 0)
		v3log = ERRLOG_DFLT;
	if (v4log == 0)
		v4log = ERRLOG_DFLT;

	bos_level = getenv("BOSLEVEL");
	if ( (bos_level != NULL) && (strcmp(bos_level,"3.2") == 0))
	{ /* migration from 3.2 to 4.1 */
		if(!open_v3_log())
			exit(1);
		else if(!open_v4_log()) {
			fprintf(stderr,"%s: unable to open the version 4 errlog: %s\n",Progname,v4log);
			exit(1);
		}

		convert_log();

		unlink(v3log_temp);

		exit(0);
	}
	else /* migration from 4.1 to 4.1 */
	{
		sprintf(mv_cmdline,"mv %s %s 1>/dev/null 2>/dev/null",v3log, v4log);
		rv = system(mv_cmdline);
		exit(rv);
	}
}


/*
 * NAME:     open_v3_log
 * FUNCTION: Move the existing /var/adm/ras/errlog to /var/adm/ras/errlog.v3 and then open 
 *			 the version 3 error log /var/adm/ras/errlog.v3 and verify the header.
 * RETURN:	 1 - success
 *			 0 - failure
 */

static int
open_v3_log(void)
{
	int rc = 0;		/* failure */
	int saverr;
	struct stat statbuf;

	if ( (v3log_temp = malloc(strlen(v3log) + 4))  == NULL)
		perror("open_v3_log:malloc");
	else
	{
		strcpy(v3log_temp,v3log);
		if((strcat(v3log_temp,".v3")) == NULL) 		/* v3 log will be /var/adm/ras/errlog.v3 */
			fprintf(stderr,"%s: strcat(%s,\".v3\") returned NULL)\n",Progname,v3log);

		else if(stat(v3log_temp,&statbuf) != -1) {	/* version 3 logfile already exists */
			fprintf(stderr,"The file %s exists.  The %s program would destroy the contents.\n",
			    v3log_temp,Progname);
			fprintf(stderr,"Move the %s file and rerun the %s command.\n",v3log_temp,Progname);
			exit(1);
		}
		else if(rename(v3log,v3log_temp) == -1) {		/* move v3 log to backup */
			saverr = errno;
			fprintf(stderr,"%s:  open_v3_log: ",Progname);
			errno = saverr;
			perror("rename: ");
		}
		else if((v3Logfd=open(v3log_temp,O_RDONLY)) == -1) {
			saverr = errno;
			fprintf(stderr,"%s:  open_v3_log: ",Progname);
			errno = saverr;
			perror("open(v3logs_temp,O_RDONLY): ");
		}
		else if(!get_v3_hdr())
			;
		else
			rc = 1;		/* success */

		return(rc);
	}
}

/*
 * NAME:     get_v3_hdr
 * FUNCTION: get the header from the version 3 errlog at /var/adm/ras/errlog.v3
 * RETURN:	 1 - success, good header.
 *			 0 - failure
 */

static int
get_v3_hdr(void)
{
	int saverr;
	int rv = 0;

	if(lseek(v3Logfd,0,0) == -1) {
		saverr = errno;
		fprintf(stderr,"%s: get_v3_hdr: ",Progname);
		errno = saverr;
		perror("lseek: ");
	}
	else if((rv=read(v3Logfd,&v3_log_hdr,sizeof(v3_log_hdr))) == -1) {
		saverr = errno;
		fprintf(stderr,"%s: get_v3_hdr: ",Progname);
		errno = saverr;
		perror("read: ");
	}
	else if(rv != sizeof(v3_log_hdr))
		fprintf(stderr,"%s: get_v3_hdr: invalid header: %d bytes read\n",Progname,rv);
	else if(strncmp(v3_log_hdr.lh_magic,v3_LH_MAGIC,8))
		fprintf(stderr,"%s: get_v3_hdr: invalid header: lh_magic = %s\n", Progname,v3_log_hdr.lh_magic);
	else
		rv = 1;

	return(rv);
}

/*
 * NAME:     open_v4_log
 * FUNCTION: open and create the version 4 errlog at /var/adm/ras/errlog.
 * RETURN:	 1 - success
 *			 0 - failure
 */

static int
open_v4_log(void)
{
	int rc = 0;		/* failure */

	if(loginit() == 0) {		/* create v4 errlog */
		set_v4_hdr();
		rc = 1;		/* success */
	}

	return(rc);
}


/*
 * NAME:     set_v4_hdr
 * FUNCTION: set the header in the version 4 errlog at /var/adm/ras/errlog
 *			 from the v3 header already read in.
 * RETURN:	 1 - success, good header.
 *			 0 - failure
 */

static int
set_v4_hdr(void)
{
	int saverr;
	int rv = 0;

	Threshold = v3_log_hdr.lh_maxsize;
	log_hdr.lh_sequence = v3_log_hdr.lh_sequence;
	log_hdr.lh_maxsize = v3_log_hdr.lh_maxsize;
	log_hdr.lh_staleoff = v3_log_hdr.lh_maxsize;
	if(logwrite(&log_hdr,sizeof(log_hdr),0) == 0) /* success */
		rv = 1;

	return(rv);
}


/*
 * NAME:     convert_log
 * FUNCTION: Convert the version 3 errorlog to a version 4 errlog.
 * RETURN:	 none
 */

static void
convert_log(void)
{

	char pack_buf[LE_MAX_SIZE];			/* will hold v4 log entry */
	struct v3_log_entry v3_log_entry;	/* will hold v3 log entry */

	v3_Log_outoff = v3_log_hdr.lh_outoff;

	bzero(&v3_log_entry,sizeof(v3_log_entry));
	while(v3_logget(&v3_log_entry) > 0) {
		bzero(&pack_buf,LE_MAX_SIZE);
		v3_to_v4_entry(&v3_log_entry,&pack_buf);
		logappend(&pack_buf);
	}
	close_v3_log();
	close_v4_log();
}

/*
 * NAME:     v3_to_v4_entry
 * FUNCTION: Convert a version 3 errorlog entry to a version 4 errlog entry.
 * RETURN:	 none
 */

static void
v3_to_v4_entry(struct v3_log_entry *v3_le, char *v4_le)
{
	struct obj_errlog v3_obj_errlog, *v3_oe;
	int	used = 0;		/* space used for the entry */
	char *p;

	p = v4_le;
	v3_oe = &v3_obj_errlog;

	/*
	 * Copy the obj_errlog portion of the v3 entry to an obj_errlog
	 * to facilitate the manipulations for conversion to v4.
	 */
	memcpy(v3_oe,&v3_le->le_data[0],sizeof(struct obj_errlog));

	/*
	 *	Copy necessary data to the v4 error log structure from the v3 
	 *	errlog entry.
	 */

	p += sizeof(int);		 /* save the entry length until last. */
	*(int *)p = LE_MAGIC;
	p += sizeof(int);
	*(int *)p = v3_oe->el_sequence;
	p += sizeof(int);
	*(int *)p = v3_oe->el_timestamp;
	p += sizeof(int);
	*(int *)p = v3_oe->el_crcid;
	p += sizeof(int);


	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_machineid)+1,LE_MACHINE_ID_MAX),
	    &v3_oe->el_machineid,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_nodeid)+1,LE_NODE_ID_MAX),
	    &v3_oe->el_nodeid,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_resource)+1,LE_RESOURCE_MAX),
	    &v3_oe->el_resource,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_vpd_ibm)+1,LE_VPD_MAX),
	    &v3_oe->el_vpd_ibm,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_vpd_user)+1,LE_VPD_MAX),
	    &v3_oe->el_vpd_user,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_in)+1,LE_IN_MAX),
	    &v3_oe->el_in,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_connwhere)+1,LE_CONN_MAX),
	    &v3_oe->el_connwhere,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_rclass)+1,LE_RCLASS_MAX),
	    &v3_oe->el_rclass,p);

	p += pack_dynamic_ent(limit_range(0,strlen(&v3_oe->el_rtype)+1,LE_RTYPE_MAX),
	    &v3_oe->el_rtype,p);

	p += pack_dynamic_ent(limit_range(0,v3_oe->el_detail_length,LE_DETAIL_MAX),
	    &v3_oe->el_detail_data,p);
	p += pack_dynamic_ent(0,0,p);

	p += sizeof(int);
	used = p - v4_le;					/* true total size of entry */
	*(int *)v4_le  = used;				/* le_length */
	*(int *)(p - sizeof(int)) = used;	/* le_length2 */
}


/*
  * NAME:      pack_dynamic_ent()
  * FUNCTION:  Pack supplied dynamic entry data into the supplied
  *            buffer.
  * RETURNS:   amount of space used to pack the entry
  */

static int
pack_dynamic_ent(
int	length,		/* length of supplied entry */
char* data,		/* dynamic entry data */
int* p)			/* address of buffer to pack in */
{
	int used = sizeof(int);		/* the length entry will always be packed */

	memcpy(p, &length,sizeof(int));
	p += 1;		/* one sizeof(int) = 4 bytes  in ptr arithmetic */
	if (length > 0) {
		memcpy(p,data,length);
		used += length;
	}
	return(used);
}


static void
close_v3_log(void)
{
	close(v3Logfd);
}


static void
close_v4_log(void)
{
	close(Logfd);
}

static usage()
{
	cat_eprint(CAT_CONVERT_USAGE,
	    "Usage:   convert_errlog -i Filename -o Filename\n\
\n\
-i Filename     Specifies the name of the V3 error log file.\n\
-o Filename     Specifies the name of the V4 error log file.\n\
\n\
If no option is specified, the default for input and output\n\
files is /var/adm/ras/errlog\n");

	exit(1);
}


