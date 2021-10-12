static char sccsid[] = "@(#)30        1.33  src/bos/usr/lib/methods/chgif/chgif.c, cmdnet, bos411, 9433B411a 8/16/94 16:13:06";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, nextattr, chgif, usage
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
  chgif -- change the attributes of an interface.

  - verifies if is at least defined.
  - checks new value for attributes against possible values field.
  - updates if in the config database to reflect change.
  - calls ifconfig to change that attribute.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>

#include "sys/stat.h"
#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "chgif_msg.h"
#define MSG_SET MS_CHGIF
#include "msg_hdr.h"
nl_catd catd;

int dflag=0;

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
/*  no longer supported in 3.2
{ "metric", 0, ""}, 
{ "security", 0, "" },
{ "authority", 0, "" },
*/
};

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc,char **argv)
{
	char *cp, *ifname;
	int c, aflag=0;
	char attrbuf[1024];
	int phase = 0;

	attrbuf[0] = '\0';

	setlocale(LC_ALL, "");
	catd = catopen(MF_CHGIF, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'P':
		case 'd':
			dflag++;
			break;
		case 'a':
			strcat(attrbuf, " ");
			strcat(attrbuf, optarg);
			aflag++;
			break;
		case 'l':
			ifname = optarg;
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
	cfg_init_db(phase);

	/* if we are dataless then set dflag */
	if (!dflag) {
		char    *vfs_name = "/usr";
		struct stat    finfo;

		/* if we have a remotely mounted filesystem then only
		 * change the database information.  This will avoid a
		 * customer that changes the interface that has mounted /usr.
		 */
		if ((stat(vfs_name, &finfo)) != -1) {
			/* check if NFS (2) mounted or AFS (4) mounted */
			if (finfo.st_vfstype == 2 || finfo.st_vfstype == 4) {
				dflag = 1;
				fprintf(stderr,"These changes will take affect ONLY in the DATABASE.\n");
			}
		} else {
			perror("stat(/usr)");
		}
	}
	if (!aflag)  usage(0);
	chgif(ifname, attrbuf);

	odm_terminate();
	exit(0);
}


/*
 * NAME: chgif
 *                                                                    
 * FUNCTION: changes the attributes of an interface
 *                                                                    
 * NOTES:
 *    if dflag is set then changes only take place in the configuration
 *    database.  otherwise ifconfig is called to change the values of 
 *    the currently running interface.
 *
 * RETURNS:  nothing.
 */
chgif(char *ifname, char *attr)
{
	int rc=0, i;
	CLASS_SYMBOL cudv_hndl;
	struct CuDv *cudvp;
	struct CuAt *cuatp, *c;
	struct listinfo cudv_info, cuat_info;
 	char crit[128], cmd[256], *chp, *parameterize();
	struct attrvalpair avp[64], *ap;
	int changed = 0;	/* if interface is changed */
	int nattr;
	char *badattr;		/* return pointer for attrval */
	struct cmdtab *cp = cmdtab;
	char *netmask = (char *)0, *ipaddr = (char *)0;

	DBGMSG((stderr, "changing %s: %s", ifname, attr));
	/*
	 * verify that the if in question has been defined.
	 */

	strcpy(crit, "name = '");
	strcat(crit, ifname);
	strcat(crit, "'");
	DBGMSG((stderr, "Querying CuDv for: %s", crit));
	cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
			"0821-226 chgif: Cannot get records from CuDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num != 1) {
		ERR(Msg(NOTONEIF,
		"0821-224 chgif: Expected one if instance, retrieved %d.\n"),
		    cudv_info.num);
		CFGEXIT(E_ODMGET);
	}

	/******************************************
	  Validate attributes
	 *******************************************/
	DBGMSG((stderr, "chgif: Attributes to validate: %s\n", attr));
	if (attrval(cudvp->PdDvLn_Lvalue,attr,&badattr) > 0) {
		ERR(Msg(BADATTR,"0821-228 chgif: Bad attribute(s) or attribute value(s): %s\n"),
			badattr);
		CFGEXIT(E_ATTRVAL);
	}

	/*
	 * for each specified attribute change it in the config database.
	 */

	nattr = 0;		/* number of attribute */
	while ((ap = nextattr(attr)) != 0) {
		/*
		 * lookup current attribute.
		 */
		DBGMSG((stderr, "Looking up attribute: %s for %s\n",
		       ap->attr, ifname));
		cuatp = tcp_getattr(ifname, ap->attr, 0, &i);
		if ((int) cuatp == 0) {
			ERR(Msg(GETCUAT,
			"0821-223 chgif: Cannot get records from CuAt.\n"));
			CFGEXIT(E_ODMGET);
		}
		
		DBGMSG((stderr, "calling putattr to add \"%s\"", ap->val));
		strcpy(cuatp->value, ap->val);
		if (putattr(cuatp) == -1) {
			ERR(Msg(BADADDCUAT,
			"0821-222 chgif: Cannot add customized attribute.\n"));
			CFGEXIT(E_ODMADD);
		}
		/* save attribute and value */
		strcpy ( avp[nattr].attr, ap->attr);
		strcpy ( avp[nattr].val, ap->val);
		nattr++;
		changed = 1;
	}

	DBGMSG((stderr,"no. of attributes = %d\n", nattr));
	/****************************************************************
	 * handle special parameter passing through enviroment variables 
	 *   the dial string consists of blank and \ etc. which the shell
	 *   does not handle those parameters well.
	 ****************************************************************
	 */
	if ( (chp = getenv ( "SLIP_DIAL_STRING")) != NULL ) {
		strcpy ( crit, "dialstring");	/* attribute */
		DBGMSG((stderr, "Looking up attribute: %s for %s\n",
		       crit, ifname));
		cuatp = tcp_getattr(ifname, crit, 0, &i);
		if ((int) cuatp == 0) {
			ERR(Msg(GETCUAT,
			"0821-223 chgif: Cannot get records from CuAt.\n"));
			CFGEXIT(E_ODMGET);
		}
		
		DBGMSG((stderr, "calling putattr to add \"%s\"", chp));
		strcpy(cuatp->value, chp);
		if (putattr(cuatp) == -1) {
			ERR(Msg(BADADDCUAT,
			"0821-222 chgif: Cannot add customized attribute.\n"));
			CFGEXIT(E_ODMADD);
		}
	}

	/*
	 * if we are not just changing the database (dflag), and
	 * we have just made a change in the database, then we call ifconfig
	 */
	if (!dflag && changed) {
		/*
                 * for the interface, we want to look up all customized
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
		strcat(cmd, " inet");
		for (cp=cmdtab; cp->attr; cp++) {
			if (strcmp(cp->attr, (cp+1)->attr) == 0)
				continue;
			for (c=cuatp, i=0; i<nattr; i++, c++) {
				if (strcmp(cp->attr, c->attribute) == 0) {
					DBGMSG((stderr, "put in ifconfig cmd - attr: %s; val = %s\n", c->attribute, c->value));
					strcat(cmd, " ");
					strcat(cmd,
					  parameterize(c->attribute, c->value));
					if (strcmp(c->attribute,  "netaddr") 
						== 0) {
						ipaddr = c->value;
						DBGMSG((stderr, "ipddr = %s\n", 							ipaddr));
					}
					if (strcmp(c->attribute,  "netmask") 
						== 0)
						netmask = c->value;
					break;
				}
			}
		}

		/*
		 * open customized device class for change
		 */

		cudv_hndl = odm_open_class(CuDv_CLASS);
		if ((int) cudv_hndl == -1) {
			ERR(Msg(OPENCUDV,
			    "0821-230 chgif: Cannot open CuDv.\n"));
			CFGEXIT(E_ODMOPEN);
		}

		/*
		 * If this is the Serial Optical Interface, 
		 * then change it special.
		 */
		if ((strncmp(ifname, "so", 2) == 0) &&
		    (ipaddr != (char *)0)) {
			DBGMSG((stderr, "about to call cfgso: ipddr = %s\n", 
				ipaddr));
			if (rc = cfgso(ipaddr, netmask, ifname) != 0) {
				DBGMSG((stderr, "cfgso failed with rc = %d\n", 
					rc));
				CFGEXIT(E_LAST_ERROR);
			}
		}

		
		DBGMSG((stderr, "Calling:  \"%s\"", cmd));
		if ( (rc = shell(cmd)) != 0)  {
			ERR(Msg(IFCFGFAIL,
				"0821-229 chgif: ifconfig command failed.\nThe status of\"%s\" Interface in the current running system is uncertain.\n"), ifname);
			DBGMSG((stderr, "non-zero exit code from ifconfig %d\n", rc));
			/*
			 * change the state of the interface to down
			 */
			cudvp->status = STOPPED;
			odm_change_obj(cudv_hndl, cudvp);
	
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
			CFGEXIT (E_LAST_ERROR);
		}

		/*
		 * check the resultant netmask to see if the kernel changed
		 * it.  if so, update the database and report the fact.
		 */
		if (netmask) {  /* XXX error if not? */
		    int s;
		    struct ifreq ifr;
		    struct sockaddr_in mask, *sin;
		    extern struct in_addr inet_makeaddr();

		    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("chgif: socket");
			CFGEXIT(E_SYSTEM);
		    }
		    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
		    /*
		     * The user may have caused the device to be
		     * detached, at which point we should not
		     * be concerned with what the netmask has
		     * become.
		     */
		    if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifr) < 0) {
			if (errno != ENXIO) {
			    perror("chgif: ioctl(SIOCGIFNETMASK)");
			    CFGEXIT(E_SYSTEM);
			}
		    } else {
			if (getaddr(netmask, &mask.sin_addr) < 0) {
			    CFGEXIT(E_SYSTEM);
			}
			sin = (struct sockaddr_in *)&ifr.ifr_addr;
			if (mask.sin_addr.s_addr != sin->sin_addr.s_addr) {
			    cuatp = tcp_getattr(ifname, "netmask", 0, &i);
			    if (cuatp == NULL || i != 1) {
				ERR(Msg(GETMASK,
				"chgif: error getting netmask attribute for %s\n"),
				    ifname);
				CFGEXIT(E_ODMGET);
			    }
			    strcpy(cuatp->value, inet_ntoa(sin->sin_addr));
			    if (putattr(cuatp) == -1) {
				ERR(Msg(PUTMASK,
				"chgif: error saving changed netmask value\n"));
				CFGEXIT(E_ODMADD);
			    }
			    printf(Msg(MASKCHANGED,
				"chgif: invalid netmask value %s changed to %s\n"),
				inet_ntoa(mask.sin_addr), cuatp->value);
			}
		    }
		    close(s);
		}

		/*
		 * now mark the staus of the interface.
		 */
		if (strstr(cmd, " down")) {
			/* The ifconfig command with the netaddress will always
			 * set the interface up. If the interface is marked
			 * down, then issue an ifconfig command to set it to
			 * down state.
			 */
			strcpy(cmd, "/usr/sbin/ifconfig ");
			strcat(cmd, ifname);
			strcat(cmd, " down");
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			shell(cmd);
			cudvp->status = STOPPED;
		}
		else
			cudvp->status = AVAILABLE;
		odm_change_obj(cudv_hndl, cudvp);
	} else {
		/*
		 * open customized device class for change
		 */
		cudv_hndl = odm_open_class(CuDv_CLASS);
		if ((int) cudv_hndl == -1) {
			ERR(Msg(OPENCUDV,
			    "0821-230 chgif: Cannot open CuDv.\n"));
			CFGEXIT(E_ODMOPEN);
		}
		strcpy ( crit, "state");	/* attribute */
		DBGMSG((stderr, "Looking up attribute: %s for %s\n", crit, ifname));
		cuatp = tcp_getattr(ifname, crit, 0, &i);
		/* trying to change the state to "up", ignore any error */
		if ((int) cuatp != 0) {
			 if (!strcmp(cuatp->value, "up")) {
				cudvp->status = AVAILABLE;
				DBGMSG((stderr, "calling odm_change_obj() 2\n"));
				if (odm_change_obj(cudv_hndl, cudvp) != 0) {
					DBGMSG((stderr, "odm_change_obj() failed odmerrno=%d\n", odmerrno));
				}
			} else {
				cudvp->status = DEFINED;
				DBGMSG((stderr, "calling odm_change_obj() 3\n"));
				if (odm_change_obj(cudv_hndl, cudvp) != 0) {
					DBGMSG((stderr, "odm_change_obj() failed odmerrno=%d\n", odmerrno));
				}
			}
		}
	}
	return (0);
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
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
usage(a, b, c, d)
	char *a, *b, *c, *d;
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		Msg(USAGE1,
	    "\nusage:\t%s  [-d] -l interface [-a \"attr=val ...\"] ...\n"),
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
			error = 1;
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

/*
 * NAME: getaddr
 *
 * FUNCTION: adapted from cmd/net/ifconfig/ifconfig.c.  returns 
 *		value of string as internet addr, either symbolic,
 *		hex, dotted decimal, etc.
 *
 * RETURNS:  if successful, 0 and addr; else -1 and gives error message
 */
static int
getaddr(s, addr)
char *s;
struct in_addr *addr;
{
	struct hostent *hp;
	struct netent *np;
	int val;

	/* see if it's a name or an address */
	if (!isinet_addr(s)) {
		if (hp = gethostbyname(s)) {
			bcopy(hp->h_addr, (char *)addr, hp->h_length);
			return(0);
		} else if (np = getnetbyname(s)) {
			*addr = inet_makeaddr(np->n_net, INADDR_ANY);
			return(0);
		} else {
			herror(s);
			return(-1);
		}
        } else if ((val = inet_addr(s)) != -1) {
		addr->s_addr = val;
		return(0);
	} else {
		fprintf(stderr, Msg(BADATTR,"chgif: bad value: %s\n"), s);
		return(-1);
	}
}
