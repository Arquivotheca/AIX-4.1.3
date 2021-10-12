static char sccsid[] = "@(#)20        1.19  src/bos/usr/lib/methods/ucfgif/ucfgif.c, cmdnet, bos411, 9428A410j 3/25/94 09:33:14";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, unconfig_all, ucfgif, usage
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
  ucfgif -- unloads if instances from the kernel.

  - calls sysconfig to unload specified instances from the kernel.
 */

#include "tcpcfg.h"
#include <errno.h>
#include <sys/device.h>
#include <sys/sysconfig.h>

char progname[128];		/* = argv[0] at run time */

#include "ucfgif_msg.h"
#define MSG_SET MS_UCFGIF
#include "msg_hdr.h"
nl_catd catd;
void ucfgif(char *);
void usage(char *, char *, char *, char *);
/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	char *cp, *ifname[69];
	int c, i, lflag=0, phase=0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_UCFGIF, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);
	
	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
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
	cfg_init_db(phase);

	if (lflag)
		for (i=0; i<lflag; i++)
			ucfgif(ifname[i]);
	else
		ucfgif(0);

	odm_terminate();
	return(0);
}


/*
 * NAME: ucfgif
 *
 * FUNCTION: detaches interface instance, and, if the last instance,
 *           unloads it, all via ifconfig.
 *
 * RETURNS: nothing.
 */
void
ucfgif(char *ifname)
{
	CLASS_SYMBOL cudv_hndl;
	int i;
	struct CuDv *cudvp, *c;
	struct listinfo cudv_info;
	char cmd[256], crit[256];

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-151 ucfgif: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * we find all if's in customized database. 
	 */
	if (ifname)
		sprintf(crit, "name = '%s'", ifname);
	else
		strcpy(crit, "parent = 'inet0'");

	DBGMSG((stderr, "Querying CuDv for: %s", crit));
	cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info, 1, 1);
	if ((int) cudvp == -1 ) {
		ERR(Msg(GETCUDV,
			"0821-150 ucfgif: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	DBGMSG((stderr, "retrieved %d customized IF's", cudv_info.num));
	
	/*
	 * for each IF instance we set it to the defined state.
	 */
	for (i=0, c=cudvp; i<cudv_info.num; i++, c++) {
		/* now call ifconfig to detach the IF driver, if appropriate */
		strcpy(cmd, "/usr/sbin/ifconfig ");
		strcat(cmd, c->name);
		strcat(cmd, " detach");
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		shell(cmd);
		
		c->status = DEFINED;
		odm_change_obj(cudv_hndl, c);
	}

	odm_close_class(cudv_hndl);
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
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s [-l interface ] ...\n"),
		progname);
	odm_terminate();
	CFGEXIT(E_ARGS);
}
