static char sccsid[] = "@(#)30	1.1  src/bos/usr/bin/errlg/errlogger/main.c, cmderrlg, bos411, 9428A410j 3/2/93 16:22:37";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * Log an operator message
 * Create an error log entry containing an operator message up to
 *  ERR_REC_MAX (230) characters in length.
 *
 * usage is:
 * errlogger message_string(s)
 *
 */


#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/erec.h>
#include <sys/errids.h>
#include <locale.h>
#include <errlg.h>
#include <cmderrlg_msg.h>

#define VCPY(T,F) strncpy(T,F,sizeof(T))

extern optind;
extern char *optarg;

/* 
 * errlog call allows only ERR_REC-MAX chars to be logged 
 * so there is no point having the buffer bigger than that.
 */
#define EBUFSIZE ERR_REC_MAX
static char Ebuf[EBUFSIZE+1];
static int Ebufidx;
static int Ebufsize;

static struct {
	struct err_rec0 err_rec;
	char buf[ERR_REC_MAX+1];
} e;


main(argc,argv)
char *argv[];
{

  /*  This sets the language variable  */
	setlocale(LC_ALL,"");
	setprogname();
	catinit(MF_CMDERRLG);		/* open msg catalog */
	if(argc == 1) {
		cat_eprint(CAT_LOGGER_USAGE,"Usage: errlogger message\n");
		exit(1);
	}
	errlogger(argc,argv);
}

static errlogger(argc,argv)
char *argv[];
{
	int i;
	int n;
	int longmsgflg=0;/* set if message to errlogger is > 230 bytes */

	for(i=1; i < argc; i++) {
		/* ok to use strlen to search for NULL, ILS guide p. 28 */
		n = strlen(argv[i]);	 /* count bytes in each argv[i] */
		if(n > EBUFSIZE - 1 - Ebufidx) {
			n = EBUFSIZE - 1 - Ebufidx;
			memcpy(&Ebuf[Ebufidx],argv[i],n);
			Ebufidx += n;
			longmsgflg=1;
			break;
		}
		memcpy(&Ebuf[Ebufidx],argv[i],n);
		Ebufidx += n;
		if(i < argc-1 && Ebufidx < EBUFSIZE)
			Ebuf[Ebufidx++] = ' ';
	}
	/* pad with a null */
	Ebufsize = Ebufidx;
	Ebuf[Ebufsize]='\0';

	/*
	 * Ebuf now contains a null terminated string.
	 *   1. The strlen(Ebuf) is less than or equal to ERR_REC_MAX
	 *   2. Ebuf does not truncate an NLS character in half
	 */
	e.err_rec.error_id = ERRID_OPMSG;
	VCPY(e.err_rec.resource_name,"OPERATOR");
	strncpy(e.buf,Ebuf,Ebufsize);
	if(errlog(&e,sizeof(e.err_rec) + Ebufsize) == -1) {
		perror("errlogger");
		exit(1);
	}
	if (longmsgflg) cat_eprint(CAT_ERRLOGER_MSG_LONG,
		"\nerrlogger:The preceding message was truncated to 230 bytes\n");
	exit(0);
}
