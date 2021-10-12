static char sccsid[] = "@(#)04	1.23.1.6  src/bos/usr/lib/methods/chginet/chginet.c, cmdnet, bos411, 9428A410j 3/21/94 17:50:39";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, chginet, usage, routeargs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
  chginet -- reconfigures tcpip (inet) code.

  - checks to see if inet instance has been defined.
  - puts given attributes into ODM.
  - calls hostname to (re)define hostname.
  - calls route to (re)define routes.
 */

#include "tcpcfg.h"
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

char progname[128];		/* = argv[0] at run time */

#include "chginet_msg.h"
#define MSG_SET MS_CHGINET
#include "msg_hdr.h"
nl_catd catd;

int dflag=0;
static char *replacecommas();

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(int argc, char **argv)
{
	char *cp;
	int c, aflag = 0, phase = 0;
	char attrbuf[1024];

	attrbuf[0] = '\0';

	setlocale(LC_ALL, "");
	catd = catopen(MF_CHGINET, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'a':
			strcat(attrbuf, " ");
			strcat(attrbuf, optarg);
			aflag++;
			break;
		case 'd':
			dflag++;
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

	if (!aflag)  usage(0);
	chginet(attrbuf);

	odm_terminate();

	return(0);
}


struct cfg_load load;
struct cfg_kmod kmod;

/*
 * NAME: chginet
 *                                                                    
 * FUNCTION: changes attributes of inet instance.
 *                                                                    
 * NOTES:
 *    if dflag is set then changes only take place in the configuration
 *    database.  otherwise hostname or route is called to change the
 *    values of the currently running inet instance.
 *
 * RETURNS:  nothing.
 */
chginet(char *attr)
{
	int rc, nattr;
	CLASS_SYMBOL cuat_hndl;
	struct CuDv *cudvp;
	struct PdAt *pdatp;
	struct CuAt *cuatp, newattr;
	struct listinfo cudv_info, cuat_info, pdat_info;
	char crit[128], cmd[MAXDNAME*2+40], *newval, *type, *dest, *gate, *args;
	char *user_dest, *user_gate;

	cuat_hndl = odm_open_class(CuAt_CLASS);
	if ((int) cuat_hndl == -1) {
		ERR(Msg(OPENCUAT, "0821-213 chginet: Cannot open CuAt.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * check to make sure that the inet instance is defined.
	 */

	cudvp = get_CuDv_list(CuDv_CLASS, "name = 'inet0'", &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		ERR(Msg(GETCUDV,
	"0821-208 chginet: Cannot get records from CuDv for inet0.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	if (cudv_info.num != 1) {
		ERR(Msg(NOTONE,
  "0821-212 chginet: Expected one inet0 instance, retrieved %d from CuDv.\n"),
		    cudv_info.num);
		CFGEXIT(E_ODMGET);
	}

	/*
	 * we check to see if any new values have been
	 * specified (using getval).  for each attribute with a new 
	 * value, we modify the CuAt record.
	 */

	if ((newval = getval("hostname", attr)) != 0) {

		DBGMSG((stderr, "hostname specified: %s\n", newval));
		cuatp = tcp_getattr("inet0", "hostname", 0, &nattr);
		if (cuatp == 0) {
			ERR(Msg(GETHOSTNAME,
  "0821-210 chginet: Cannot get hostname attribute record from CuAt.\n"));
			CFGEXIT(E_NOATTR);
		}

		if (!dflag) {
			/*
			 * call hostname to set hostname.
			 */
			strcpy(cmd, "/bin/hostname ");
			strcat(cmd, newval);
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			if (shell(cmd)){
				CFGEXIT (E_LAST_ERROR);
			}
		}

		if (strcmp(cuatp->value, newval) != 0) {
			/*
			 * change hostname in config database.
			 */
			DBGMSG((stderr, "Adding new hostname %s to database\n",
				newval));
			strncpy(cuatp->value, newval, sizeof(cuatp->value));
			if (putattr(cuatp) == -1) {
				ERR(Msg(BADADDCUAT,
		"0821-206 chginet: Cannot add customized attribute %s.\n"),
				    newval);
				CFGEXIT(E_ODMADD);
			}
		}
	}

	/*
	 * now check for default gateway attribute.  call route command
	 * to set the default gateway.
	 */

	if ((newval = getval("gateway", attr)) != 0) {
		
		DBGMSG((stderr, "gateway specified: %s\n", newval));
		cuatp = tcp_getattr("inet0", "gateway", 0, &nattr);
		if (cuatp == 0) {
			ERR(Msg(GETGATEWAY,
	"0821-209 chginet: Cannot get attribute record for gateway.\n"));
			CFGEXIT(E_NOATTR);
		}

		if (!dflag) {
			/*
			 * Call route command to set up route.
			 */
			strcpy(cmd, "/usr/sbin/route add 0 ");
			strcat(cmd, newval);
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			if (shell(cmd) != 0 ) {
				CFGEXIT(E_LAST_ERROR);
			}
		}
		if (strcmp(cuatp->value, newval) != 0) {
			DBGMSG((stderr, "Adding new gateway %s to database\n",
				newval));
			/*
			 * change gateway in config database.
			 */
			strncpy(cuatp->value, newval, sizeof(cuatp->value));
			if (putattr(cuatp) == -1) {
				ERR(Msg(BADADDCUAT,
		"0821-206 chginet: Cannot add customized attribute %s.\n"),
				    newval);
				CFGEXIT(E_ODMADD);
			}
		}
	}

	/*
	 * for each route specified by user for deletion (with delroute)
	 * call route to delete the route and delete from ODM.
	 * 
	 * example route spec (delroute=type,destination,gateway):
	 *
	 * -a "delroute=host,192.9.200.5,129.35.20.41 delroute=net,0,bcroom"
	 */
	
	while ((newval = getval("delroute", attr)) != 0) {

		DBGMSG((stderr, "route specified for deletion: %s\n", newval));

		/* convert symbolic net/host names into IP addresses */
		if (!routeargs(newval, &type, &args, &dest, &gate,
			       &user_dest, &user_gate)) {
			CFGEXIT(E_BADATTR);
		}

		/*
		 * delete this route from the database.
		 * try all three possible formats of each
		 * of the two possible hostnames / IP
		 * addresses.
		 */

		for(;;)
		{
		    /* try the grandest case */

		    sprintf(crit,
	    "name='inet0' and attribute='route' and value like '%s,*,%s,%s'",
			type, dest, gate);
		    DBGMSG((stderr, "deleting %s", crit));
		    if (odm_rm_obj(cuat_hndl, crit) > 0)
			break;

		    /* try without args */

		    sprintf(crit,
		    "name='inet0' and attribute='route' and value='%s,%s,%s'",
			type, dest, gate);
		    DBGMSG((stderr, "deleting %s", crit));
		    if (odm_rm_obj(cuat_hndl, crit) > 0)
			break;
			
		    /* try no type */

		    sprintf(crit,
		    "name='inet0' and attribute='route' and value='%s,%s'",
			dest, gate);
		    DBGMSG((stderr, "deleting %s", crit));
		    if (odm_rm_obj(cuat_hndl, crit) > 0)
			break;

		    /* try user-given hostnames */

		    if(dest != user_dest || gate != user_gate)
		    {
			dest = user_dest;
			gate = user_gate;
			continue;
		    }

		    /* route not found */

		    sprintf(cmd, "%s,%s,%s", type, dest, gate);
		    ERR(Msg(BADRTDEL,
		    "0821-216 chginet: Cannot delete route (%s) from CuAt.\n"),
			cmd);
		    CFGEXIT(E_ODMDELETE);
		    break;	/* just in case */
		}
		if (!dflag) {
			/*
			 * Call route command to delete route. Include args.
			 */
			sprintf(cmd, "/usr/sbin/route delete -%s %s %s %s",
				type, replacecommas(args), dest, gate);
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			if (shell(cmd)) {
				CFGEXIT( E_LAST_ERROR);
			}
		}
	}

	/*
	 * for each route specified by user, enter it into config database
	 * and call route to set up the route.  check for duplicate entries.
	 * 
	 * example route spec (route=type,[args,]destination,gateway):
	 *
	 * -a "route=host,-interface,192.9.200.5,129.35.20.41
	 *     route=net,-hopcount,2,0,bcroom"
	 */
	
	while ((newval = getval("route", attr)) != 0) {

		DBGMSG((stderr, "route specified: %s\n", newval));

		/* convert symbolic net/host names into IP addresses */
		if (!routeargs(newval, &type, &args, &dest, &gate,
			       &user_dest, &user_gate)) {
			CFGEXIT(E_BADATTR);
		}

		/*
		 * check to see if record already exists.  Normally
		 * attributes are checked with tcp_getattr, but if any routes
		 * exist at all, they would be in the CuAt file.
		 */
		sprintf(crit,
		"name='inet0' and attribute='route' and value='%s,%s,%s,%s'",
			type, args, dest, gate);
		DBGMSG((stderr, "Querying CuAt for: %s", crit));
		cuatp = get_CuAt_list(CuAt_CLASS, crit, &cuat_info, 1, 1);
		if ((int) cuatp == -1) {
			ERR(Msg(GETROUTE,
		"0821-211 chginet: Cannot get route attribute from CuAt.\n"));
			CFGEXIT(E_ODMGET);
		}

		if (!dflag) {
			/*
			 * Call route command to set up route.
			 */
			sprintf(cmd, "/usr/sbin/route add -%s %s %s %s",
				type, replacecommas(args), dest, gate);
			DBGMSG((stderr, "Calling:  \"%s\"", cmd));
			if (shell(cmd) != 0 ) {
				CFGEXIT(E_LAST_ERROR);
			}
		}
		if (cuat_info.num == 0) {
			/*
			 * drop a new route record into CuAt.
			 */
			strcpy(newattr.name, "inet0");
			strcpy(newattr.attribute, "route");
			sprintf(newattr.value, "%s,%s,%s,%s",
			    type, args, dest, gate);
			strcpy(newattr.type, "R");
			strcpy(newattr.generic, "DU");
			strcpy(newattr.rep, "s");
			newattr.nls_index = 0;
			DBGMSG((stderr, "Adding new route %s to database\n",
				newattr.value));
			if (odm_add_obj(cuat_hndl, &newattr) == -1) {
				ERR(Msg(BADADDROUTE,
			"0821-207 chginet: Cannot add route record to CuAt.\n"));
				CFGEXIT(E_ODMADD);
			}
		}
	}

	if ((newval = getval("bootup_option", attr)) != 0) {

		DBGMSG((stderr, "bootup_option specified: %s\n", newval));
		cuatp = tcp_getattr("inet0", "bootup_option", 0, &nattr);
		if (cuatp == 0) {
			ERR(Msg(GETBOOTOPTN,
  "0821-210 chginet: Cannot get bootup_option attribute record from CuAt.\n"));
			CFGEXIT(E_NOATTR);
		}

		if (strcmp(cuatp->value, newval) != 0) {
			/*
			 * change bootup_option in config database.
			 */
			DBGMSG((stderr, "Adding new bootup_option %s to database\n",
				newval));
			strncpy(cuatp->value, newval, sizeof(cuatp->value));
			if (putattr(cuatp) == -1) {
				ERR(Msg(BADADDCUAT,
		"0821-206 chginet: Cannot add customized attribute %s.\n"),
				    newval);
				CFGEXIT(E_ODMADD);
			}
		}
	}

	odm_close_class(cuat_hndl);
}


/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
usage(char *a, char *b, char *c, char *d)
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr, Msg(USAGE1,
			    "\nusage:\t%s  [-d] [-a \"attr=val ...\"] [-D]\n"),
		progname);
	CFGEXIT(E_ARGS);
}

/*
 * resolve possibly symbolic name into IP address; leave name pointing to
 * result, use res as storage for result, host is true if name is a host,
 * false if a net; return zero on error, and give error message.
 */

static
getip(name, res, host)
char **name;
char *res;
int host;
{
	struct hostent *hp;
	struct netent *np;
	u_long addr;
	char *cp;

	if (host) {
	    if ((addr = inet_addr(*name)) == -1) {  /* it's symbolic */
		if (!(hp = gethostbyname(*name))) {
		    herror(*name);
		    return(0);
		}
		bcopy(hp->h_addr, &addr, sizeof(addr));
	    }
	    *name = strcpy(res, inet_ntoa(*(struct in_addr *)&addr));
	} else {  /* net */
	    if ((addr = inet_network(*name)) == -1) {  /* it's symbolic */
		if (!(np = getnetbyname(*name))) {
		    h_errno = HOST_NOT_FOUND;
		    herror(*name);
		    return(0);
		}
		addr = np->n_net;
	    }
	    cp = inet_ntoa(*(struct in_addr *)&addr);
	    while (cp[0] == '0' && cp[1] == '.')  /* skip leading zeroes */
		cp += 2;
	    *name = strcpy(res, cp);
	}
	return(1);
}

static char *
replacecommas(str)
char *str;
{
	char *cp, *cp2, *newstr;

	if ((newstr = malloc(strlen(str) + 1)) == NULL)
	    return(str);  /* punt */
	for (cp = str, cp2 = newstr; *cp; ++cp, ++cp2) {
	    *cp2 = (*cp == ',') ? ' ' : *cp;
	}
	*cp2 = '\0';
	return(newstr);
}

/*
 * NAME: routeargs
 *
 * FUNCTION: parses comma-separated arg into bits; tries to resolve symbolic
 *	     net and host names into IP addresses
 *	     must handle following formats of database entries:
 *		[type,[arg1,...]]dest,gate
 *
 * RETURNS:  assigns pointers to static strings that are the parsed result;
 *           gives error message and returns 0 on error, returns 1 on success
 */
static
routeargs(arg, atype, aargs, adest, agate, auser_dest, auser_gate)
char *arg;
char **atype;
char **aargs;
char **adest;
char **agate;
char **auser_dest;
char **auser_gate;
{

	static char dest[16], gate[16], comma[] = ",";
	char *cp, *cp2;
	int dots;

	/*
	* parse dest and gate from end
	*/
	if ((cp = strrchr(arg, ',')) == NULL)
	    return(0);  /* must have at least one comma */
	*agate = cp + 1;
	*cp = '\0';

	/*
	 * parse type from beginning
	 */
	if ((cp = strrchr(arg, ',')) == NULL) {  /* old entry with no type */
	    *adest = arg;
	    *aargs = "";  /* no args either */
	    /*
	     * total hack to deal with database entries in the old format.
	     * since they don't have the net/host keyword we have to supply
	     * one based on the faulty assumption that three dots means
	     * it's a host route, less means a net route.  of course this
	     * doesn't take account of subnets but if they haven't noticed
	     * by now then it's probably ok.
	     */
	    for (dots = 0, cp = *adest; *cp; ++cp) {
		if (*cp == '.')  /* count the dots */
		    ++dots;
	    }
	    *atype = (dots < 3 ? "net" : "host");

	} else {
	    *adest = cp + 1;
	    *cp = '\0';
	    *atype = arg;
	    /*
	     * parse optional args
	     */
	    if ((cp2 = strchr(arg, ',')) == NULL) {
		*aargs = "";
	    } else {
		*aargs = cp2 + 1;
		*cp2 = '\0';
	    }
	}

	/*
	 * resolve symbolic net/host names into IP addrs
	 */
	if (!strcmp(*adest, "default"))  /* handle special default cases */
	    *adest = "0";
	*auser_dest = *adest;
	*auser_gate = *agate;
	if (strcmp(*adest, "0") && !getip(adest, dest, **atype == 'h'))
	    return(0);
	return(getip(agate, gate, 1));
}
