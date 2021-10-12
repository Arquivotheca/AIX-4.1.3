static char sccsid[] = "@(#)34	1.10  src/bos/usr/ccs/lib/libdiag/diag_errlg.c, libdiag, bos411, 9428A410j 2/23/93 08:25:54";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	error_log_get	
 *		errlogread_init
 *		errlogread_term
 *		errlogread
 *		nametoelp
 *		streq
 *		copy_struct
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/stat.h>
#include <sys/errids.h>
#include "diag/diag.h"

unsigned el_sequence;
unsigned el_timestamp;
unsigned el_crcid;
char     el_machineid[100];
char     el_nodeid[100];
char     el_class[2];
char     el_type[5];
char     el_resource[NAMESIZE];
char     el_rclass[CLASSIZE];
char     el_rtype[TYPESIZE];
char     el_vpd[VPDSIZE];
char     el_connwhere[LOCSIZE];
char     el_in[LOCSIZE];
unsigned el_detail_length;
char     el_detail_data[256];
char     el_alpha_data[256];
unsigned et_logflg;
unsigned et_alertflg;
unsigned et_reportflg;

#define ELTY_NUM  1
#define ELTY_BOOL 2
#define ELTY_CHAR 3
#define ELTY_BIN  4

#define SNUM(NAME)  { "NAME", ELTY_NUM,  (char *)&NAME,    0 }
#define SCHAR(NAME) { "NAME", ELTY_CHAR, (char *)&NAME[0], sizeof(NAME) }
#define SBOOL(NAME) { "NAME", ELTY_BOOL, (char *)&NAME,    0 }
#define SBIN(NAME)  { "NAME", ELTY_BIN,  (char *)&NAME[0], sizeof(NAME) }

FILE *Efp;
static struct el {
	char *el_name;
	int   el_type;
	char *el_data;
	int   el_size;
} el[] = {
	SNUM(el_sequence),
	SNUM(el_timestamp),
	SNUM(el_crcid),
	SCHAR(el_machineid),
	SCHAR(el_nodeid),
	SCHAR(el_class),
	SCHAR(el_type),
	SCHAR(el_resource),
	SCHAR(el_rclass),
	SCHAR(el_rtype),
	SCHAR(el_vpd),
	SCHAR(el_connwhere),
	SCHAR(el_in),
	SNUM(el_detail_length),
	SBIN(el_detail_data),
	SCHAR(el_alpha_data),
	SBOOL(et_logflg),
	SBOOL(et_alertflg),
	SBOOL(et_reportflg),
};
#define N_EL (sizeof(el) / sizeof(el[0]))

/* FUNCTION PROTOTYPES */
int error_log_get(int, char *, struct errdata *);
int errlogread_init(char *);
int errlogread_term(void);
int streq( register char *, register char *);
int errlogread(void);
struct el *nametoelp(char *);
int copy_struct(struct errdata *err_data);

/* EXTERNAL FUNCTIONS */
extern char *strstr(char *, char *);

/*
 * convert ascii to int binary equivalent
 */
#define ATOB(n) \
	( 'a' <= n && n <= 'f' ? n - 'a' + 0x0a : \
	  'A' <= n && n <= 'F' ? n - 'A' + 0x0a : \
	  n - '0' )

#define ISSPACE(c) ( (c) == ' ' || (c) == '\t' )

/*  */
/*
 * NAME: error_log_get 
 *                                                                    
 * FUNCTION: Interface to errlog functions. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 
 *		INIT: 		0 - no error
 *		                1 - error log entry available
 *		               -1 - error initializing errpt
 *		SUBSEQ:		0 - no more entries
 *		                1 - error log entry available
 *		TERMI: 		0 - terminate successful
 */  

int error_log_get(
	int	op_flg,
	char	*crit,
	struct	errdata	*err_data)
{
	int	rc;
	char	buffer[256];
	int	dev_flg;		/* TRUE = errpt on specific device */

	switch( op_flg ) {
		case INIT:
			dev_flg = (strstr(crit, "-N") == (char *)NULL) ? 0 : 1;
			sprintf(buffer, "errpt -g %s 2>/dev/null", crit);
			if ((rc = errlogread_init(buffer)) != -1) {
				if (rc = errlogread()) {
					copy_struct(err_data);
					if ((err_data->err_id == 
					    (unsigned )ERRID_REPLACED_FRU)
					     && dev_flg) return(0);
				}
			}
			/* if error running errpt - return 0 */
			else 
				return(0);
			break;
		case SUBSEQ:
			if (rc = errlogread()) {
				copy_struct(err_data);
				if ((err_data->err_id ==
				    (unsigned )ERRID_REPLACED_FRU)
				     && dev_flg) return(0);
			}
			break;
		case TERMI:
			 rc = errlogread_term();
	}
	return(rc);
}

/*  */
/*
 * NAME: errlogread_init 
 *                                                                    
 * FUNCTION: Execute the 'errpt' cmd and open a pipe to read the data
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 
 *		0 : no error
 *             -1 : error executing a pipe open call on 'errpt'
 */  

int errlogread_init(char *cmd)
{

	int	rc;
	struct	stat	buf;

	if (( rc = stat(ERRPT, &buf)) != 0 )
		return(-1);

	if(cmd == 0)
		cmd = "errpt -g";
	if(Efp)
		pclose(Efp);
	if((Efp = popen(cmd,"r")) == 0) {
		perror(cmd);
		return(-1);
	}
	return(0);
}

/*  */
/*
 * NAME: errlogread_term  
 *                                                                    
 * FUNCTION: Close the open pipe on 'errpt'
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 0
 */  

int errlogread_term(void)
{
	int rc;

	/* If pipe was opened read all errlog	*/
	/* entries until pipe is empty.		*/
	if (Efp)
	  {
	  while (rc = errlogread()) ;
	  pclose(Efp);
	  }
	return(0);
}

/*
 * NAME:      streq
 *
 * FUNCTION:  determine if two strings have the same value
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 * 	this routine is written for expressions like:
 *    		if(streq(optarg,"stdin"))
 *           		...
 * 	which is closer to the way it is thought of than:
 *    		if(strcmp(optarg,"stdin") == 0)
 *           		...
 *
 * RETURNS:   1 if equal (non-zero)
 *            0 if not equal
 *
 */

int streq(
	register char *s1,
	register char *s2)
{
	register char c;

	while(c = *s1++)
		if(c != *s2++)
			return(0);
	return(*s2 == '\0' ? 1 : 0);
}

/*  */
/*
 * NAME: errlogread
 *                                                                    
 * FUNCTION: Reads the next error log entry.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 
 *		0 : no more entries
 *             >0 : additional entries in error log
 */  

int errlogread(void)
{
	char *name;
	char *data;
	struct el *elp;
	register char *cp;
	int i;
	int eflg;
	char line[512];

	if (Efp == 0)
		return(0);
	for (i = 0; i < N_EL; i++) {
		switch(el[i].el_type) {
		case ELTY_NUM:
		case ELTY_BOOL:
			*(unsigned *)el[i].el_data = 0;
			break;
		case ELTY_CHAR:
			el[i].el_data[0] = '\0';
			break;
		default:
			continue;
		}
	}

	eflg = 0;
	while (fgets(line, sizeof(line), Efp)) {
		switch(line[0]) {
		case '\0':
		case '\n':
		case ' ':
		case '\t':
		case '#':
		case '@':
			if(line[1] == '@')
				return(eflg ? 1 : 0);
			continue;
		}
		cp = line;
		while(!ISSPACE(*cp)) {
			if(*cp == '\0' || *cp == '\n')
				goto skip;
			cp++;
		}
		*cp++ = '\0';
		name = line;
		while(ISSPACE(*cp)) {
			if(*cp == '\0' || *cp == '\n') {
				*cp == '\0';
				break;
			}
			cp++;
		}

		data = cp;
		if((elp = nametoelp(name)) == 0)
			continue;
		switch(elp->el_type) {
		case ELTY_BIN:
			for(i=0, cp=data; i < elp->el_size, *cp; i++, cp += 2)
				elp->el_data[i] = 
					(ATOB(cp[0]) << 4) + ATOB(cp[1]);
			break;
		case ELTY_BOOL:
			*(unsigned *)elp->el_data = streq(data,"FALSE") ? 0 : 1;
			break;
		case ELTY_NUM:
			*(unsigned *)elp->el_data = strtoul(data,0,0);
			break;
		case ELTY_CHAR:
			strncpy(elp->el_data,data,elp->el_size);
			elp->el_data[elp->el_size-1] = '\0';
			break;
		default:
			continue;
		}
		eflg++;
skip:
		;
	}
	return(0);
}

/*  */
/*
 * NAME: nametoelp 
 *                                                                    
 * FUNCTION: Returns pointer to element entry matching 'name'
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS: 
 *		0 : element not found
 *		address of element
 */  

struct el *nametoelp(char *name)
{
	int i;

	for(i = 0; i < N_EL; i++)
		if(streq(el[i].el_name,name))
			return(&el[i]);
	return(0);
}


/*  */
/*
 * NAME: copy_struct    
 *                                                                    
 * FUNCTION: Copies error log data to user's structure 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * RETURNS:  0
 */  
int copy_struct(struct errdata *err_data)
{
	err_data->sequence = el_sequence;
	err_data->time_stamp = el_timestamp;
	err_data->err_id = el_crcid;
	err_data->machine_id = el_machineid;
	err_data->node_id = el_nodeid;
	err_data->class = el_class;
	err_data->type = el_type;
	err_data->resource = el_resource;
	err_data->vpd_data = el_vpd;
	err_data->conn_where = el_connwhere;
	err_data->location = el_in;
	err_data->detail_data_len = el_detail_length;
	err_data->detail_data = el_detail_data;

	return(0);
}
