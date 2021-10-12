static char sccsid[] = "@(#)12        1.16  src/bos/usr/lib/methods/stpinet/stpinet.c, cmdnet, bos411, 9428A410j 3/25/94 09:32:31";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, stop_all_ifs, stop_one_if, check_avail, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * PROGRAM: stpinet -- disables the inet instance
 *
 * if stpinet is started with a list of if's to take down, then it only
 * stops those if's.  otherwise it informs users of the impending demise
 * of TCP/IP via wall.  calls ifconfig "down" for each configured if.
 * and only if no if's were specified on command line will the status
 * flag of inet instance be set to DEFINED.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */


#include "stpinet_msg.h"
#define MSG_SET MS_STPINET
#include "msg_hdr.h"
nl_catd catd;

void stop_one_if(char *);
void usage(char *, char *, char *, char *);
/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */

main(int argc, char **argv)
{
	int c, lflag = 0, tflag = 0, time, i, phase = 0;
	char *ifname[69];
	char cmd[256], *cp;

	setlocale(LC_ALL, "");
   	catd = catopen(MF_STPINET, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);
	
	/* check to see if inet is available. */
	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 't':
			time = atoi(optarg);
			tflag++;
			break;
		case 'l':
			ifname[lflag++] = optarg;
			break;
		case '2':
			phase = 2;
			break;
		case 'h':
		case '?':
			usage(0,0,0,0);
		}
	}

	/* set odm path, initialize and lock the config database. */
	cfg_init_db();

	check_avail();

	/*
	 * inform users that TCP/IP will be going away in 60 seconds,
	 * then wait a minute.  if we just take down specified if(s),
	 * then don't warn user.
	 */
	
	if (!lflag) {
		if (tflag) {
			sprintf(cmd, Msg(TIMEWARN,
	    "/bin/echo 'WARNING - TCP/IP will shut down in %d minute' | wall"),
				time);
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			shell(cmd);
			sleep(time*60);
		}
		strcpy(cmd, Msg(NOWARNING,
	"/bin/echo 'WARNING - TCP/IP will shut down immediately!' | wall"));
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		shell(cmd);
		stop_all_ifs();
	} else
		for (i=0; i<lflag; i++)  stop_one_if(ifname[i]);

	odm_terminate();
	return(0);
}


/*
 * NAME: stop_all_ifs
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
stop_all_ifs()
{
	int i;
	CLASS_SYMBOL cudv_hndl;
	struct CuDv *cudvp, *cd, *inetp;
	struct listinfo cudv_info;

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-082 stpinet: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}
	
	/*
	 * we find all if's in CuDv.  then call ifconfig "down" for each
	 * defined if.
	 */

	cudvp = get_CuDv_list(CuDv_CLASS, "parent = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-079 stpinet: Cannot get records from CuDv. \n"));
		CFGEXIT(E_ODMGET);
	}
	if (cudv_info.num == 0)
		return; /* no instances were found */
	
	/*
	 * for each if instance we ifconfig it down
	 */
	for (i=0, cd=cudvp; i<cudv_info.num; i++, cd++)
		stop_one_if(cd->name);
	
	/*
	 * set status of inet0 to DEFINED.
	 */

	inetp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info, 1, 1);
	if ((int) inetp == -1) {
		ERR(Msg(GETCUDV,
			"0821-079 stpinet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	inetp->status = STOPPED;
	odm_change_obj(cudv_hndl, inetp);

	odm_close_class(cudv_hndl);

}


/*
 * NAME: stop_one_if
 *                                                                    
 * FUNCTION: stops an if by issuing an 'ifconfig if down' command.
 *                                                                    
 * NOTES:
 *
 * RETURNS:  nothing.
 */
void
stop_one_if(char *ifname)
{
	char cmd[256];

	strcpy(cmd, "/usr/sbin/ifconfig ");
	strcat(cmd, ifname);
	strcat(cmd, " down");
	DBGMSG((stderr, "Calling:  \"%s\"", cmd));
	shell(cmd);
}


/*
 * NAME: check_avail
 *                                                                    
 * FUNCTION: checks to make sure that inet instance is in AVAILABLE state.
 *                                                                    
 * NOTES:
 *
 * RETURNS:  nothing.
 */
check_avail()
{
	struct CuDv *inetp;
	struct listinfo cudv_info;

	/*
	 * check CuDv to verify that inet is in AVAILABLE state.
	 */

	inetp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info, 1, 1);
	if ((int) inetp == -1) {
		ERR(Msg(GETCUDV,
			"0821-079 stpinet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num == 0) {
		ERR(Msg(NOTCFGD,
	"0821-080 stpinet: TCP/IP is not configured yet.\n"));
		CFGEXIT(E_ODMGET);
	}
	
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
void
usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		Msg(USAGE1,
		    "\nusage:\t%s [-l interface] ... [-t minutes]\n"),
		progname);
	CFGEXIT(E_ARGS);
}

