static char sccsid[] = "@(#)34	1.19  src/bos/usr/bin/errlg/errpt/rpt.c, cmderrlg, bos411, 9439B411a 9/28/94 11:19:48";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: rpt, lpr
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/err_rec.h>		/* ERRSET_DESCRIPTION */
#include <errlg.h>
#include <fcntl.h>

char le[LE_MAX_SIZE];	/* buffer to put error log entry into */

extern uid_t real, saved;

/*
 * NAME:     rpt
 * FUNCTION: Batch mode report
 *           Records are output in reverse sequence_id order.
 * INPUT:    NONE
 * RETURNS:  NONE
 */

rpt()
{
	int rv;
	struct le_bad le_bad;			/* contain code for bad log entry */

	if(logopen(O_RDONLY)) {
		genexit(1);
	}
	loggetinit();		/* initialize the sequence id, etc. */

	for(;;) {				/* for all records in the log */
		rv = udb_logget_rv(&le,&le_bad);	/* get entry & template, reverse order */
		if(rv == 0)			/* no more log entries */
			break;
		/* if reportflg == false, don't print the entry */
		if (T_errtmplt.et_reportflg == 0)
			continue;
		if(lst_chk()) {
			if(le_bad.code)
				le_bad_msg(&le_bad);
			else
				pr_logentry();
		}
	}

}


static int Sequenceno;

/*
 * NAME:     rpt_c
 * FUNCTION: Concurrent mode report
 *           Records are output in sequence_id order.
 *			 Note:
 *				To process the 'new' entry, all previous entries are re-processed.
 *				This is due to knowing the 'new' entries only by remembering the
 *				sequence id of the entries already processed.  This also assumes
 *				that the sequence ids are in ascending order.  This code will
 *				NOT work if the sequence ids are not ascending.
 * INPUT:    NONE
 * RETURNS:  NONE
 *
 */

rpt_c()
{
	int rv;
	int firstflg;
	int curr_sequenceno;
	struct le_bad le_bad;			/* contain information for bad log entry */

	if(logopen(O_RDONLY)) {
		genexit(1);
	}

	Sequenceno = -1;
	seteuid(saved);
	notifyadd();
	seteuid(real);
	for(;;) {	/* forever */
		curr_sequenceno = Sequenceno;
		for(firstflg = 1; ; firstflg = 0) { /* for all entries in the log */
			if (firstflg)
				loggetinit();
			rv = udb_logget(&le,&le_bad);
			if(rv == 0)
				break;
			if((int)T_errlog.el_sequence > Sequenceno)
				Sequenceno = T_errlog.el_sequence;
			if((int)T_errlog.el_sequence <= curr_sequenceno)
				continue;
			/* if reportflg == false, don't print the entry */
			if (T_errtmplt.et_reportflg == 0)
				continue;
			if(lst_chk()) { 
				if(le_bad.code)
					le_bad_msg(&le_bad);
				else
					pr_logentry();
			}
		}
		notifypause();		/* wait for next logged entry */
	}
}


/*
 * copy from a to b. null terminate b
 */
#define VCPY(a,b) \
	memcpy(b,a,MIN(sizeof(a),sizeof(b))); ((char *)(b))[sizeof(b)-1] = '\0';


/*
 * NAME:     rpt_tmplt
 * FUNCTION: Report templates.  Display error templates in either summary
 *			 or detailed mode.
 * INPUT:    NONE
 * RETURNS:  NONE
 *
 */

rpt_tmplt()
{
	int rv;
	int firstflg;

	for(firstflg = 1; ; firstflg = 0) {
		rv = tmpltdump(firstflg);
		if(rv <= 0)
			return;
		if((rv=invalid_tmplt(&T_errtmplt))) 		/* annunciate bad template */
			ann_bad_tmplt(rv,&T_errtmplt);
		T_errlog.el_crcid = T_errtmplt.et_crcid;
		VCPY(T_errtmplt.et_class,T_errlog.el_class);
		VCPY(T_errtmplt.et_type, T_errlog.el_type);
		if(lst_chk())
			pr_tmpltentry();
	}
}

struct nt {
	char *nt_value;
	char *nt_comment;
};

static struct nt classlist[] = {
	{ "H", 0 },
	{ "S", 0 },
	{ "O", 0 },
	{ "U", 0 },
	0
};

static struct nt typelist[] = {
	{ "PERM", 0 },
	{ "TEMP", 0 },
	{ "PERF", 0 },
	{ "PEND", 0 },
	{ "INFO", 0 },
	{ "UNKN", 0 },
	0
};

lpr(str)
char *str;
{

	if(streq(str,"-N")) {
		lpr_resource();
		exit(0);
	}
	if(streq(str,"-T")) {
		lpr_type();
		exit(0);
	}
	if(streq(str,"-R")) {
		lpr_rtype();
		exit(0);
	}
	if(streq(str,"-d")) {
		lpr_class();
		exit(0);
	}
	if(streq(str,"-S")) {
		lpr_rclass();
		exit(0);
	}
}

/*
 * look up resource names in devices object class
 */
static lpr_resource()
{
	int rv;
	int firstflg;

	udb_init();
	for(firstflg = 1; ; firstflg = 0) {
		rv = CuDv_dump(firstflg);
		if(rv <= 0)
			break;
		printf("%s\n",CuDv_name);
	}
}

/*
 * look up resource class in devices object class
 * class, subclass, type
 */
static lpr_rclass()
{
	char *cp;
	int rv;
	int firstflg;

	udb_init();
	for(firstflg = 1; ; firstflg = 0) {
		rv = CuDv_dump(firstflg);
		if(rv <= 0)
			break;
		cp = strtok(CuDv_rclass,", \t");
		if(cp && *cp)
			printf("%s\n",cp);
	}
}

/*
 * look up resource types in devices object class
 */
static lpr_rtype()
{
	char *cp;
	int rv;
	int firstflg;

	udb_init();
	for(firstflg = 1; ; firstflg = 0) {
		rv = CuDv_dump(firstflg);
		if(rv <= 0)
			break;
		cp = strtok(CuDv_rclass,", \t");
		cp = strtok(0,", \t");
		cp = strtok(0,", \t");
		if(cp && *cp)
			printf("%s\n",cp);
	}
}

static lpr_class()
{

	ilpr(classlist);
}

static lpr_type()
{

	ilpr(typelist);
}

static ilpr(ntp0)
struct nt *ntp0;
{
	struct nt *ntp;

	for(ntp = ntp0; ntp->nt_value; ntp++) {
		if(ntp->nt_comment)
			printf("%-10s %s\n",ntp->nt_value,ntp->nt_comment);
		else
			printf("%s\n",ntp->nt_value);
	}
}
