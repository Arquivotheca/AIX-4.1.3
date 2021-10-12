static char sccsid[] = "@(#)00        1.19.1.4  src/bos/usr/lib/methods/cfginet/cfginet.c, cmdnet, bos411, 9428A410j 3/21/94 18:05:39";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, cfginet, load_inet, usage
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
  cfginet -- loads and configures tcpip (inet) code.

  - checks to see if inet instance has been defined.
  - loads the inet code via sysconfig.
  - calls hostname to define hostname.
  - calls route to define routes.
  - sets status flag of inet instance to AVAILABLE.
 */

#include "tcpcfg.h"
#include <errno.h>
#include <sys/device.h>
#include <sys/sysconfig.h>

char progname[128];		/* = argv[0] at run time */
extern char *stats[];

#include "cfginet_msg.h"
#define MSG_SET MS_CFGINET
#include "msg_hdr.h"
nl_catd catd;

int phase = 0;
char *replacecommas(char *);
void usage(char *);
/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	int i, c;
	char *cp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_CFGINET, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process cmd line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case '2':
			phase = 2;
			break;
		case 'h':
		case '?':
			usage(0);
		}
	}

	/* set odm path, initialize and lock the config database. */
	cfg_init_db(phase);

	load_inet();
	cfginet();

	odm_terminate();
	return(0);
}



/*
 * deal with old format database entries by assuming that three dots means
 * it's a host route, less means a net route.  of course this doesn't take
 * account of subnets but if they haven't noticed by now then it's ok.
 */
static char *
hackit(cp)
char *cp;
{
	int dots;

	for (dots = 0; *cp && *cp != ','; ++cp)
	    if (*cp == '.')  /* count the dots */
		++dots;
	return(dots < 3 ? "-net " : "-host ");
}

/*
 * NAME: cfginet
 *                                                                    
 * FUNCTION: configures the inet instance - loads the netinet kernel
 *           extension via sysconfig().
 *                                                                    
 * RETURNS:  nothing.
 */
cfginet()
{
	int i;
	int rc;
	CLASS_SYMBOL cudv_hndl, pddv_hndl;
	struct CuDv *cudvp;
	struct CuAt *cuatp, *c;
	struct PdDv *pddvp;
	struct listinfo cudv_info, cuat_info, pddv_info;
	char cmd[256];
	int nattr;
	char module_symbol[128];

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-198 cfginet: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * check to make sure that the inet instance is defined and in the
	 * DEFINED state.
	 */

	cudvp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-194 cfginet: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num != 1) {
		ERR(Msg(NOTONE,
	"0821-196 cfginet: Expected one inet0 instance, retrieved %d.\n"),
		    cudv_info.num);
		CFGEXIT(E_ODMGET);
	}
/*
	if (cudvp->status != DEFINED) {
		ERR(Msg(BADSTATE,
	"0821-191 cfginet: Expected status = DEFINED. Got status = %s.\n"),
		    stats[cudvp->status]);
		CFGEXIT(E_DEVSTATE);
	}
*/

	/*
	 * look up hostname in CuAt for inet0.
	 */

	cuatp = tcp_getattr("inet0", "hostname", 0, &nattr);
	if ((int) cuatp == 0) {
		ERR(Msg(GETCUAT,
			"0821-193 cfginet: Cannot get attribute record.\n"));
		CFGEXIT(E_NOATTR);
	}
	
	DBGMSG((stderr, "hostname = %s", cuatp->value));
	if (nattr == 1) {
		/*
		 * call hostname to set hostname.
		 */
		
		strcpy(cmd, "/bin/hostname ");
		strcat(cmd, cuatp->value);
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		shell(cmd);
	}

	/*
	 * for each route in CuAt, call route to set up that route.
	 */
	
	cuatp = tcp_getattr("inet0", "route", 0, &nattr);
	if (cuatp == 0) {
		ERR(Msg(GETCUAT,
			"0821-193 cfginet: Cannot get attribute record.\n"));
		CFGEXIT(E_NOATTR);
	}
	
	for (i=0, c=cuatp; i<nattr; i++, c++) {
		/*
		 * if there's no "net" or "host" keyword then we're dealing
		 * with an old format database entry, and we need to add a
		 * best-guess keyword using the old faulty logic.
		 */
		sprintf(cmd, "/usr/sbin/route add %s%s",
		    strncmp(c->value, "net,", 4) &&
		    strncmp(c->value, "host,", 5) ? hackit(c->value) : "-",
		    replacecommas(c->value));
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		shell(cmd);
	}

	/*
	 * set status of inet0 to 'AVAILABLE'.
	 */

	cudvp->status = AVAILABLE;
	odm_change_obj(cudv_hndl, cudvp);

	/* start TCPIP only if in 2nd phase of boot process */
	if (phase == 2) {

		pddvp = get_PdDv_list(PdDv_CLASS,
				      "uniquetype = 'tcpip/TCPIP/inet'",
				      &pddv_info, 1, 1);
		if ((int) pddvp == -1) {
			ERR(Msg(GETPDDV,
			"0821-195 cfginet: Cannot get records from PdDv.\n"));
			CFGEXIT(E_ODMGET);
		}
		if (pddv_info.num != 1) {
			ERR(Msg(NOTONEINET,
		"0821-197 cfginet: expected one INET record, retrieved %d.\n"),
			    pddv_info.num);
			CFGEXIT(E_ODMGET);
		}
		/* run the start method */
		DBGMSG((stderr, "Running start method: %s\n", pddvp->Start));
		EXIT_ON_ERROR(shell(pddvp->Start));
	}

	odm_close_class(cudv_hndl);
}


/*
 * NAME: load_inet
 *
 * FUNCTION: loads the netinet code into the kernel via sysconfig().
 *
 * RETURNS: nothing.
 */
load_inet()
{
	char path[256];
	struct cfg_load load;
	struct cfg_kmod kmod;

	bzero(&load, sizeof(load));
	bzero(&kmod, sizeof(kmod));
	load.path = path;
	sprintf(path, "%s%s", NETDIR, NETINET);

	/*
	 * check the kernel module id of the driver for the inet code
	 * to make sure it isn't loaded.
	 */
	if (sysconfig(SYS_QUERYLOAD, &load, sizeof(load)) ==
	    CONF_FAIL) {
		ERR(Msg(BADQUERY,
			"0821-200 cfginet: error querying kernel extension "));
		perror(load.path);
		CFGEXIT(E_LOADEXT);
	}

	if (load.kmid != 0)
		return;

	/*
	 * call sysconfig() to load the kernel module.
	 */
	
	if (sysconfig(SYS_KLOAD, &load, sizeof(load)) == CONF_FAIL) {
		ERR(Msg(BADLOAD,
	"0821-190 cfginet: The sysconfig (SYS_KLOAD) system call failed.  "));
		perror(load.path);
		CFGEXIT(E_LOADEXT);
	}
	kmod.cmd = CFG_INIT; /* we are always doing init now */
	kmod.kmid = load.kmid; /* move module id over */
	
	/* we want to pass the kmid to the kernel, first time only */
	kmod.mdiptr = (caddr_t)&kmod.kmid;
	kmod.mdilen = sizeof(kmod.kmid);
	
	/* 
	 * call sysconfig() to call driver entry point with CFG_INIT.
	 */
	if (sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod)) == CONF_FAIL) {
		ERR(Msg(BADCFG,
	"0821-189 cfginet: The sysconfig (SYS_CFGKMOD) system call failed.  "));
		perror(load.path);
		CFGEXIT(E_CFGINIT);
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
usage(char *a)
{
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s\n"), progname);
	fprintf(stderr, Msg(USAGE2, "\t%s  -h\n"), progname);
	CFGEXIT(E_ARGS);
}


char *
replacecommas(char *str)
{
	static char newstr[128];
	char *cp;

	strcpy(newstr, str);
	while (cp = strchr(newstr, ','))
		*cp = ' ';
	return(newstr);
}
