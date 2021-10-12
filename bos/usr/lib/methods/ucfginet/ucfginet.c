static char sccsid[] = "@(#)22	1.21  src/bos/usr/lib/methods/ucfginet/ucfginet.c, cmdnet, bos411, 9428A410j 11/12/93 13:43:24";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, ucfginet, unload, usage
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
  ucfginet -- unloads the TCP/IP code.

  - makes sure that TCP/IP is stopped.
  - calls sysconfig to unload the TCP/IP code from kernel.
 */

#include "tcpcfg.h"
#include <errno.h>
#include <nlist.h>
#include <sys/device.h>
#include <sys/sysconfig.h>

char progname[128];		/* = argv[0] at run time */

#include "ucfginet_msg.h"
#define MSG_SET MS_UCFGINET
#include "msg_hdr.h"
nl_catd catd;
void usage(char *, char *, char *, char *);

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	int i, phase = 0, c;
	char *cp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_UCFGINET, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
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

	unload();
	ucfginet();

	odm_terminate();

	return(0);
}


/*
 * NAME: ucfginet
 *                                                                    
 * FUNCTION: 
 * Mark status = DEFINED inet0 record
 * in customized database .
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
ucfginet()
{
	int i;
	CLASS_SYMBOL cudv_hndl;
	struct CuDv *cudvp;
	struct CuDep *cudep;
	struct listinfo cudv_info, cudep_info;
	char cmd[128];

	DBGMSG((stderr, "unconfiguring inet"));
	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-260 ucfginet: Cannot open CuDv.\n"));
		return;
	}

	/*
	 * check to see that inet0 is in the STOPPED state.
	 */
	cudvp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info, 1, 1);
	if ((int) cudvp == -1 || cudv_info.num == 0) {
		ERR(Msg(GETCUDV,
			"0821-258 ucfginet: Cannot get records from CuDv.\n"));
		return;
	}

	/*
	 * make sure that TCP/IP is stopped, stop it if it isn't already.
	 */
	if (cudvp->status != STOPPED) {
		sprintf(cmd, "%s%s", METDIR, "stpinet");
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		shell(cmd);
	}

	/*
	 * now we set the status of inet0 to DEFINED.
	 */
	cudvp->status = DEFINED;
	odm_change_obj(cudv_hndl, cudvp);

}


/*
 * NAME: unload
 *
 * FUNCTION: use sysconfig to unload inet code.
 *
 * RETURNS: nothing.
 */
unload()
{
#ifdef UNLOAD
	char path[256];
	struct cfg_load load;
	struct cfg_kmod kmod;

	bzero(&load, sizeof(load));
	bzero(&kmod, sizeof(kmod));
	load.path = path;
	sprintf(path, "%s%s", NETDIR, NETINET);

	/*
	 * get the kernel module id of the driver for the inet code
	 * so we can unload it.
	 */
	if (sysconfig(SYS_QUERYLOAD, &load, sizeof(load)) ==
	    CONF_FAIL) {
		ERR(Msg(BADQUERY,
		"0821-259 ucfginet: error querying kernel extension "));
		perror(load.path);
		CFGEXIT(E_UNLOADEXT);
	}
	if (load.kmid == 0)
		return;
	
	kmod.kmid = load.kmid;
	kmod.cmd = CFG_TERM;
	
	/* 
	 * call sysconfig() to call driver entry point with CFG_TERM.
	 */
	if (sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod)) == CONF_FAIL) {
		ERR(Msg(BADUNCFG,
	"0821-256 ucfginet: The sysconfig (SYS_CFGKMOD) system call failed.  "));
		perror(load.path);
		CFGEXIT(E_CFGTERM);
	}

	/*
	 * call sysconfig() to unload the kernel module.
	 */
	if (sysconfig(SYS_KULOAD, &load, sizeof(load)) == CONF_FAIL) {
		ERR(Msg(BADUNLOAD,
	"0821-257 ucfginet: The sysconfig (SYS_KULOAD) system call failed.  "));
		perror(load.path);
		CFGEXIT(E_UNLOADEXT);
	}
	/* 
	 * call srcdelinet() to inform SRC daemon inet sockets are gone.
	 */
	(void) srcdelinet();
#endif /* UNLOAD */
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
void usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s\n"), progname);
	CFGEXIT(E_ARGS);
}
