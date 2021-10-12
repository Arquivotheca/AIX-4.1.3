static char sccsid[] = "@(#)98        1.31  src/bos/usr/lib/methods/cfgif/cfgif.c, cmdnet, bos411, 9428A410j 3/21/94 17:59:27";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, cfgif_all, parameterize, cfgif, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * cfgif -- configures all defined interfaces.
 *
 * takes the defined info for an interface and translates it into a
 *  cmd line for ifconfig.
 * ifconfig is then called with the constructed command line.
 * ifconfig does the actual loading of the interface driver.
 */

#include "tcpcfg.h"
#include <sys/device.h>
#include <sys/sysconfig.h>

char progname[128];		/* = argv[0] at run time */

#include "cfgif_msg.h"
#define MSG_SET MS_CFGIF
#include "msg_hdr.h"
nl_catd catd;

/*
 * the cmdtab is used to convert the information in the config database
 * into a command line that can be passed to ifconfig.  for each 
 * if attribute in the database, and for each value for that attribute,
 * the corresponding command argument is given.
 *
 * the command line parameter can be one of three types:
 * 1) a switch, which is keyed by attr + val, and has the form in the
 *       cmd field of the struct.  (example: "-arp").
 *       the cmdtab indicates this form when there is an entry for 
 *       both attr and val fields.
 * 2) a keyword value pair, which is keyed by attribute. and has the
 *       form "attribute-name value" where the value is looked up in
 *       the config database.  (example: "netmask 255.255.0.0").
 *       the cmdtab indicates this form when there is an entry for 
 *       the attr field, the value field is NULL, and the cmd field
 *       is non-NULL (i.e. "").
 * 3) a value, retrieved from the config database.  (example for
 *       netaddr -> "192.9.200.13").  
 *       the cmdtab indicates this form when there is an entry for 
 *       the attr field, and both the val and cmd fields are NULL.
 */
struct cmdtab {
	char *attr;		/* ODM style attribute */
	char *val;		/* value in ODM */
	char *cmd;		/* what to put on cmdline */
} cmdtab[] = {
{ "netaddr", 0, 0 },
{ "dest", 0, 0 },
{ "arp", "on", "arp" },
{ "arp", "off", "-arp" },
{ "allcast", "on", "allcast" },
{ "allcast", "off", "-allcast" },
{ "hwloop", "on", "hwloop" },
{ "hwloop", "off", "-hwloop" },
{ "netmask", 0, "" },
{ "mtu", 0, "" },
{ "broadcast", 0, "" },
{ "subchan", 0, "" },
{ "state", "up", "up" },
{ "state", "down", "down" },
{ "state", "detach", "detach" },
{ 0 }
/*  not supported in 3.2
{ "metric", 0, ""}, 
{ "remmtu", 0, "" },
{ "security", 0, "" },
{ "authority", 0, "" },
*/
};

int nconfigured = 0;		/* count how many if's we configure. */
int cfgif(char *);
void usage(char*, char *, char *, char *);
int cfgso(char *, char *, char *);
int getprocessorid(char *, char *);

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(argc,argv)
	int argc;
	char **argv;
{
	int c, lflag = 0;
	char *cp, *ifname;
	int phase = 0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_CFGIF, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process cmd line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case '2':
			phase = 2;
			break;
		case 'l':
			ifname = optarg;
			lflag++;
			break;
		case 'h':
		case '?':
			usage(0,0,0,0);
		}
	}

	/* set odm path, initialize and lock the config database. */
	cfg_init_db(phase);

	if (lflag) {
		if (( c = cfgif(ifname)) != 0 ) {
			DBGMSG((stderr, "cfgif routine error exit code: %d\n", c));
			CFGEXIT(E_LAST_ERROR);
		}
	}
	else
		cfgif_all();

	if (nconfigured == 0) {
		printf(Msg(NOCFG, "no interfaces were configured.\n"),
		       progname);
	}

	odm_terminate();
	exit(0);
}


/*
 * NAME: cfgif_all
 *                                                                    
 * FUNCTION: configures all the interfaces
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
cfgif_all()
{
	int i, j;
	struct CuDv *cudvp, *c, *cloop;
	struct PdDv *pddvp, *p;
	struct listinfo cudv_info, pddv_info;
	char cmd[128];

	/*
	 * we need to find all interfaces that have been defined
	 */

	cudvp = get_CuDv_list(CuDv_CLASS, "parent = 'inet0'", &cudv_info,
			      1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-002 cfgif: Getting records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	DBGMSG((stderr, "number of if's retrieved = %d", cudv_info.num));
	/* Make sure loopback interface is last, Berkeley code assumes
         * primary interface is first in the if list
         */
	for (c=cudvp, j=0; j<cudv_info.num; j++, c++) {
		if(strncmp(c->name,"lo",2)) {
			cfgif(c->name);
		}
		else
			cloop = c;
	}
	cfgif(cloop->name);
}


/*
 * NAME: parameterize
 *                                                                    
 * FUNCTION: 
 * turns an ODM attribute into an ifconfig cmd line parameter via
 * table lookup in cmdtab structure.
 *                                                                    
 * NOTES:
 *
 * RETURNS:
 *    pointer to command line parameter for given attribute.
 *    0 if no match for given attribute is found in cmdtab.
 */
char *
parameterize(char *attr,
	     char *val)
{
	static char cmdbuf[256];
	struct cmdtab *cp = cmdtab;
	
	while (cp->attr) {
		if (strcmp(attr, cp->attr) == 0) {
			if (cp->val) {
				if (strcmp(val, cp->val) == 0)
					/* if here, we have a match on attr
					   , and val */
					return(cp->cmd);
			} else {
				/* if here, we have a match on attr, no val */
				if (cp->cmd) {
					strcpy(cmdbuf, cp->attr);
					strcat(cmdbuf, " ");
					strcat(cmdbuf, val);
					return(cmdbuf);
				} else
					return(val);
			}
		}
		cp++;
	}
	return(0);
}


/*
 * NAME: cfgif
 *                                                                    
 * FUNCTION: calls ifconfig to configure a given interface.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  nothing.
 */
int
cfgif(char *ifname)		/* name of interface to configure */
{
	CLASS_SYMBOL cuat_hndl, cudv_hndl;
	int i, nattr, rc;
	struct CuDv *cudvp;
	struct CuAt *cuatp, *c;
	struct listinfo cudv_info;
	char crit[128];
	char cmd[256];
	struct cmdtab *cp;
	char *ipaddr = (char *)0;
	char *netmask = (char *)0;

	DBGMSG((stderr, "configuring %s", ifname));
	/*
	 * when we configure an IF, we first check to see if it has
	 * an internet address.  if there is no address.  we don't
	 * configure.
	 */
	cuatp = (struct CuAt *) tcp_getattr(ifname, "netaddr", 0, &nattr);
	if ((int) cuatp == -1) {
		ERR(Msg(GETCUAT,
			"0821-001 cfgif: Getting records from CuAt.\n"));
		CFGEXIT(E_ODMGET);
	}
	if (nattr == 0) {
		DBGMSG((stderr, "No address for: %s, not configuring.",
			ifname));
		return (0);
	}

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-006 cfgif: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}
	
	strcpy(crit, "name = '");
	strcat(crit, ifname);
	strcat(crit, "'");
	DBGMSG((stderr, "Querying CuDv for: %s", crit));
	cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-002 cfgif: Getting records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}

	if (cudv_info.num == 0) {
		ERR(Msg(NOCUDV,
	"0821-004 cfgif: There is no customized device record for %s.\n"),
		    ifname);
		CFGEXIT(E_NOCuDv);
	}

	/*
	 * for the specified interface, we want to look up all customized
	 * attributes, make a cmd line for ifconfig, then call ifconfig.
	 */
	cuatp = (struct CuAt *) tcp_getattr(ifname, 0, 1, &nattr);
	if ((int) cuatp == 0) {
		ERR(Msg(GETCUAT,
			"0821-001 cfgif: Getting records from CuAt.\n"));
		CFGEXIT(E_NOATTR);
	}
	/* construct cmd line, we should do it by entries in cmdtab,
	   so that the cmdtab controls the order of the attributes,
	   instead of the database. */
	strcpy(cmd, "/usr/sbin/ifconfig ");
	strcat(cmd, ifname);
	strcat(cmd, " inet ");
	for (cp=cmdtab; cp->attr; cp++) {
		if (strcmp(cp->attr, (cp+1)->attr) == 0)
			continue;
		for (c=cuatp, i=0; i<nattr; i++, c++) {
			if (strcmp(c->attribute, "netaddr") == 0)
				ipaddr = c->value;
			if (strcmp(c->attribute, "netmask") == 0)
				netmask = c->value;
			if (strcmp(cp->attr, c->attribute) == 0) {
				strcat(cmd, " ");
				strcat(cmd,
				       parameterize(c->attribute, c->value));
				break;
			}
		}
	}

        /*
         * If this is the Serial Optical Interface, then config it special.
         */
        if (strncmp(ifname, "so", 2) == 0)
                if (cfgso(ipaddr, netmask, ifname) != 0) {
                        DBGMSG((stderr, "cfgso failed"));
                        return(1);
                }


	if (( rc=shell(cmd)) != 0) {
		DBGMSG((stderr, "shell failed"));
		ERR(Msg(IFCFGFAIL,
		"0821-007 cfgif: ifconfig command failed.\nThe status of\"%s\" Interface in the current running system is uncertain.\n"), ifname);
		/* change the interface to down state */
		strcpy ( crit, "state");	/* attribute */
		DBGMSG((stderr, "Looking up attribute: %s for %s\n",
		       crit, ifname));
		cuatp = tcp_getattr(ifname, crit, 0, &i);
		/* trying to change the state to "down", ignore any error */
		if ((int) cuatp != 0) {
			strcpy(cuatp->value, "down");
			DBGMSG((stderr, "calling putattr to change \"%s\"", cuatp->value));
			putattr(cuatp);
		}
		cudvp->status = STOPPED;
		odm_change_obj(cudv_hndl, cudvp);
		return(1);
	}
		
	/*
	 * now mark the staus of the interface.
	 */
	if (strstr(cmd, " down")) {
		/* The ifconfig command with the netaddress will always
		 * set the interface up. If the interface is marked down,
		 * then issue an ifconfig command to set it to down state.
		 */
		strcpy(cmd, "/usr/sbin/ifconfig ");
		strcat(cmd, ifname);
		strcat(cmd, " down");
		shell(cmd);
		cudvp->status = STOPPED;
	}
	else
		cudvp->status = AVAILABLE;
	odm_change_obj(cudv_hndl, cudvp);
	nconfigured++;

	odm_close_class(cudv_hndl);

	return (0);
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
	fprintf(stderr, Msg(USAGE1, "\nusage:\t%s [-l interface] ...\n"),
		progname);
	fprintf(stderr, Msg(USAGE2, "\t%s -h\n"),
		progname);
	CFGEXIT(E_ARGS);
}

/*
 * NAME: cfgso
 *                                                                    
 * FUNCTION: configs the serial optical interface by doing the following:
 *		Get optical device processor id.  
 *		if id == -1 then configure the device by setting the
 *			processor id to the last octet of the ipaddr 
 *			& ~netmask
 *		else if the id does not match the last octet of the
 *			ipaddr & ~netmask then fail the config.
 *                                                                    
 * NOTES:
 *
 * RETURNS:  0 == okedokey, return codes from the shell() fn, or -1
 */
int
cfgso(char *ipaddr, char *netmask, char *ifname) 
/*
	ipaddr				   ^ to net address from odm
	netmask				   ^ to net mask from odm
	ifname				   ^ to interface name
*/
{

	char cmd[100];			/* tmp buffer for mkdev command */
        struct CuAt *cusatt;            /* customized attribute class ptr */
	int error = 0, kaka;		/* return code vars */
	int procid;			/* procid derived from ipaddr */
	char *cp;			/* tmp ptr */

	DBGMSG((stderr, "cfgso start: ipaddr is %s, netmask is %s\n", ipaddr,
		netmask));
	
	/*
	 * Check ODM for the processor id of the optical driver.
	 */
        cusatt = getattr(DEVNAME,"processor_id",0, &kaka);
        if (cusatt == (struct CuAt *) NULL) {
                ERR(Msg(GETCUAT,
                        "0821-001 cfgif: Getting records from CuAt.\n"));
                CFGEXIT(E_NOATTR);
        }
	
	DBGMSG((stderr, "cfgso: got processor id of optical driver\n"));

	/*
 	 * Determine what the processor id should be based on the last
 	 * octet of the ip addr (with netmask applied).
	 */
	procid = getprocessorid(ipaddr, netmask);

	DBGMSG((stderr, "cfgso: procid is %x\n", procid));

	/*
	 * If ODM tells us the optical driver has NOT been configured, then
	 * we will do it!
	 */
        if (atoi(cusatt->value) == UNDEFINED_PID) {
		DBGMSG((stderr, "cfgso: driver is not configured\n"));
		strcpy(cmd, "/usr/sbin/chdev -l ");
		strcat(cmd, DEVNAME);
		strcat(cmd, " -a processor_id=");
		cp = cmd + strlen(cmd);
		sprintf(cp, "%d", procid);
		DBGMSG((stderr, "cfgso: chdev cmd is %s\n", cmd));
		if ((error = shell(cmd)) != 0) {
			ERR("chdev command failed\n", ifname);
		}
		strcpy(cmd, "/usr/sbin/mkdev -l ");
		strcat(cmd, DEVNAME);
		DBGMSG((stderr, "cfgso: mkdev cmd is %s\n", cmd));
		if ((error = shell(cmd)) != 0) {
			ERR("mkdev command failed\n", ifname);
		}
        }
	else {
	
		DBGMSG((stderr, "cfgso: driver processor id is %s\n", 
			cusatt->value));

		/*
		 * If the optical driver has already been configured, then 
		 * verify that the processor id jives with the last octet 
		 * of the ip address. If it doesn't, then fail the cfg.
		 */
		if (procid != atoi(cusatt->value)) {
			ERR("The device processor id does not match the last octet of the IP address.\nYou must either change the IP address or change the device processor id.\n", ifname);
			error = -1;
		}
	}
	return(error);
}

/*
 * NAME: getprocessorid
 *                                                                    
 * FUNCTION: given then id address and netmask in standard string form,
 *	compute the acceptable processor id and return it.
 *                                                                    
 * NOTES:
 *
 * RETURNS:  ipaddr & ~netmask & 0xff
 */
int
getprocessorid(char *ipaddr, char *netmask)
{
	long	addr = 0;
	long	mask = 0;

	addr = inet_addr(ipaddr);
	if (netmask != (char *)0) 
		mask = inet_addr(netmask);
	return(addr & ~mask & 0xff);
}
