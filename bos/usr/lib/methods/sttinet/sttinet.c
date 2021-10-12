static char sccsid[] = "@(#)14	1.16  src/bos/usr/lib/methods/sttinet/sttinet.c, cmdnet, bos411, 9428A410j 3/25/94 09:32:50";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, start_all_ifs, start_one_if, usage
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
  sttinet -- starts the inet instance

  - calls ifconfig "up" for each configured if instance.
  - sets status flag for inet instance to AVAILABLE.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "sttinet_msg.h"
#define MSG_SET MS_STTINET
#include "msg_hdr.h"
nl_catd catd;

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	int c, lflag = 0, i, phase = 0;
	char *ifname[69], *cp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STTINET, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);
	
	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'l':
			if (strcmp(optarg, "inet0") != 0)
				ifname[lflag++] = optarg;
			break;
		case '2':
			phase = 2;
			break;
		case 'h':
		case '?':
			usage(0);
		}
	}

	/* set odm path, initialize and lock the config database. */
	cfg_init_db();

	if (lflag)
		for (i=0; i<lflag; i++) 
			start_one_if(ifname[i]);
	else
		start_all_ifs();

	odm_terminate();
	return(0);
}

start_all_ifs()
{
	int i, j;
	CLASS_SYMBOL cudv_hndl;
	struct CuDv *cudvp, *cd, *inetp;
	struct listinfo cudv_info, inet_info;

	/*
	 * check CuDv to verify that inet is in STOPPED state.
	 */

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-095 sttinet: Cannot open CuDv. \n"));
		CFGEXIT(E_ODMOPEN);
	}
	
	inetp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &inet_info, 1, 1);
	if ((int) inetp == -1) {
		ERR(Msg(GETCUDV,
			"0821-090 sttinet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (inet_info.num != 1) {
		ERR(Msg(NOTONEINET,
	"0821-093 sttinet: Expected one inet0 instance, retrieved %d.\n"),
		    inet_info.num);
		CFGEXIT(E_ODMGET);
	}

	if (inetp->status == DEFINED) {
		ERR(Msg(NOTDEFINED,
			"0821-092 sttinet: TCP/IP is not configured.\n"));
		CFGEXIT(E_DEVSTATE);
	}

	/*
	 * we find all if's in CuDv.  then call ifconfig "up" for each
	 * defined if.
	 */

	cudvp = get_CuDv_list(CuDv_CLASS, "parent = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-090 sttinet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	if (cudv_info.num == 0)
		return; /* no instances were found */
	
	/*
	 * for each if instance we ifconfig it up
	 */
	for (i=0, cd=cudvp; i<cudv_info.num; i++, cd++) {
		/*
		 * but first we check to see if the sucker is loaded.
		 */
		if (cd->status != DEFINED)
			start_one_if(cd->name);
		else {
			ERR(Msg(IFNOTCFG,
				"0821-091 sttinet: %s is not configured.\n"),
			    cd->name);
		}
	}
	
	/*
	 * set status of inet0 to available.
	 */

	inetp->status = AVAILABLE;
	odm_change_obj(cudv_hndl, inetp);

	odm_close_class(cudv_hndl);
}


/*
 * NAME: start_one_if
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
start_one_if(char *ifname)
{
	char cmd[256];
	
	strcpy(cmd, "/usr/sbin/ifconfig ");
	strcat(cmd, ifname);
	strcat(cmd, " up");
	DBGMSG((stderr, "Calling:  \"%s\"", cmd));
	shell(cmd);
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 *:  nothing, exits.
 */
usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s [-l interface ] ...\n"),
		progname);
	CFGEXIT(E_ARGS);
}


