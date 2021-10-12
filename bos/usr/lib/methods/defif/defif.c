static char sccsid[] = "@(#)06	1.30  src/bos/usr/lib/methods/defif/defif.c, cmdnet, bos41J, 9519A_all 5/4/95 14:33:58";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, defif_by_type, defif, usage
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
  defif -- defines interfaces in the configuration database.

  - derives logical name of if instance.
  - create customized if record in config database.
  - for each predefined attribute, a customized attribute is defined, 
    if not already defined.
  - updates customized connection class to reflect if dependencies.
  - sets status flag of if instance to DEFINED.
 */

#include "tcpcfg.h"

char progname[128];		/* = argv[0] at run time */

#include "defif_msg.h"
#define MSG_SET MS_DEFIF
#include "msg_hdr.h"
nl_catd catd;

char *get_val();
int defif_by_name( char *, char *, char *, char *);
int defif_by_type(char *,  char *);
int defif(char *);
void usage(char *, char *, char *, char *);	
/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */

main(int argc, char **argv)
{
	int c, iflag, cflag, phase = 0, rc;
	char *p, *cp, *class = NULL;
	char *subclass = NULL, *type = NULL;
	char *if_name = NULL;	/* conn. where ptr-just to receive parameter
				 * from mkdev to designate a particular if
				 */

	setlocale(LC_ALL, "");
	catd = catopen(MF_DEFIF, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = tcp_getopt(argc, argv)) != EOF) {
		switch (c) {
		case 'c':
			if (strcmp(optarg, "if") != 0) {
				ERR(Msg(BADCLASS,
			"0821-016 defif: The class specified must be if."));
				exit(1);
			}
			class = optarg;
			break;
		case 's':
			subclass = optarg;
			break;
		case 't':
			type = optarg;
			break;
		case '2':
			phase = 2;
			break;
		case 'w':
			if_name = optarg;
			break;
		case 'h':
		case '?':
		default:
			usage(0,0,0,0);
		}
	}
	
	/* set odm path, initialize and lock the config database. */
	cfg_init_db();

	/* if the new interface name is passed from the command line
	 *	just do it
	 */
	if ( if_name != NULL ) 
		rc = defif_by_name(class, subclass, type, if_name);
	else
		rc = defif_by_type(subclass, type);
	
	odm_terminate();
	return(rc);
}

/*
 * NAME: defif_by_name
 *                                                                    
 * FUNCTION: defines an interface for a given name which is passed to
 * 	command by using the -w options.  All the class, subclass and
 *	type should also be passed.
 *                                                                    
 * NOTES:
 *	this command is intented to be used with smit in which all
 *	the parameters are set.
 *
 * RETURNS:  0 if something was defined.
 *	     -1 if nothing defined.
 */
int
defif_by_name( char *class,	/* class */
	char *subclass,		/* subclass of IF */
	char *type,		/* type of IF */
	char if_name[])		/* if_name to be defined */
{
	char crit[128];
	struct PdAt pdat;
	struct PdDv pddv, *pddvp;
	struct CuDv cudv;
	struct PdCn *pdcnp;
	struct listinfo pddv_info, pdcn_info;
	char adp_name[NAMESIZE];	/* adaptor name */
	int	i;

	/* ensure all the parameters are defined */
	if ( class == NULL || subclass == NULL ||
	    type == NULL || if_name == NULL )
		usage(NULL,NULL,NULL,NULL);

	sprintf (crit,"uniquetype = '%s/%s/%s' and attribute = 'if_keyword'",
	    class, subclass, type);
	DBGMSG((stderr, "Querying PdAt for: %s", crit));
	if ( ((int)odm_get_first(PdAt_CLASS, crit, &pdat)) <= 0 ) {
		DBGMSG((stderr, "no if_keyword found\n",
			adp_name));
		ERR(Msg(CANTGETPDAT, "0821-033 defif: Cannot get record for if_keyword attribute from PdAt.\nuniquetype = %s/%s/%s\n"),
			class, subclass, type );
		CFGEXIT(E_ODMGET);
	}

	/* check the internet interface without a matching adapter,
	 * e.g. SLIP, loop back
	 */
	if ( !strcmp ( pdat.deflt, "none" ) ) {
		defif(if_name);
		return(0);
	}

	/* get prefix from the data base */
	strcpy(crit, "connkey = '");
	strcat(crit, pdat.deflt);
	strcat(crit, "'");
	DBGMSG((stderr, "Querying PdCn for: %s", crit));
	pdcnp = get_PdCn_list(PdCn_CLASS, crit, &pdcn_info, 1, 1);
	if ((int) pdcnp == -1) {
		ERR(Msg(GETADAPT,
		"0821-020 defif: Cannot get records from PdCn.\n"));
		CFGEXIT(E_ODMGET);
	}

	/*
	 * find the predefined adaptor.
	 */
	sprintf(crit, "uniquetype = '%s'", pdcnp->uniquetype);
	DBGMSG((stderr, "looking for Pd adaptor: %s", crit));
	pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info,
			      1, 1);
	if ((int) pddvp == -1) {
		ERR(Msg(GETPDADAP,
	"0821-022 defif: Cannot get predefined adaptor.\n"));
		CFGEXIT(E_ODMGET);
	}
	if (pddv_info.num != 1) {
		ERR(Msg(PDADAPNUM,
	"0821-029 defif: Expected one predefined adaptor, retreived %d.\n"),
			pddv_info.num);
		CFGEXIT(E_ODMGET);
	}


	strcpy( adp_name, pddvp->prefix);	/* get the prefix of adaptor */

	/* find the hex digits appended to the end of the logical name
	 * look for prefix in the predefined data base
	 */
	sprintf (crit,"uniquetype = '%s/%s/%s'", class, subclass, type);
	DBGMSG((stderr, "Querying PdDv for: %s", crit));
	if ( ((int)odm_get_first(PdDv_CLASS, crit, &pddv)) <= 0 ) {
		DBGMSG((stderr, "no PdDv record found for adapter %s\n",
			adp_name));
		ERR(Msg(CANTGETPDDV, "0821-034 defif: Cannot find predefined device with uniquetype = %s/%s/%s\n"),
			class, subclass, type );
		CFGEXIT(E_ODMGET);
	}

	i = strlen(pddv.prefix);
	strcpy ( &adp_name [strlen(adp_name)], &if_name[i]);

	/* get the appropriate adaptor name */

	/*
	 * If this is the Serial Optical driver, then it may either be in
	 * the AVAILABLE -OR- DEFINED states.  Either state is OK to define
	 * a so interface...
	 */
	if (strncmp(adp_name, "ops", 3) == 0) 
		sprintf (crit, "name = '%s' ", adp_name);
	else
		sprintf (crit, "name = '%s' and status = %d ", adp_name, 
			AVAILABLE);
	DBGMSG((stderr, "defif_by_name: search criteria -- %s", crit));
	if ( ((int)odm_get_first(CuDv_CLASS, crit, &cudv)) <= 0 ) {
		DBGMSG((stderr, "no customized adaptors (%s) found\n",
			adp_name));
		ERR(Msg(CANTGETCUDV, "0821-032 defif: Cannot get record for adaptor %s from CuDv.\n"),adp_name );
		return(1);
	}
	DBGMSG((stderr, "defif_by_name: available adaptor found\n"));
	defif(if_name);		/* define the if_name in the system */
	return (0);
}
	 
/*
 * NAME: defif_by_type
 *                                                                    
 * FUNCTION: defines an interface for given subclass and type, if not
 * already defined.
 *                                                                    
 * NOTES:
 *  for each predefined interface in PdDv,
 * look up the adaptor keyword we are to look for in PdAt.
 * look up adaptor keyword in PdCn to get uniquetype for adaptor
 * for each adaptor in CuDv, configure the interface.
 *
 * RETURNS:  0 if something was defined.
 *	     -1 if nothing defined.
 */
int
defif_by_type(char *subclass,	/* subclass of IF */
	      char *type)	/* type of IF */
{
	int i, j, n, t, define_one, ndef=0;
	struct PdCn *pdcnp, *k;
	struct PdDv *pddvp;
	struct PdAt *pdatp, *atp;
	struct CuDv *cudvp, *d;
	struct listinfo pddv_info, cudv_info, pdat_info, pdcn_info;
	char crit[128];
	char ifname[64];
	int	seqno;		/* the no. next in sequence */

	define_one =  (type && subclass);
	DBGMSG((stderr, "define_one is %s", (define_one)?"TRUE":"FALSE"));
	/*
	 * get a list of the adaptor keywords.
	 */
	sprintf(crit,
		"uniquetype like 'if/%s/%s' and attribute = 'if_keyword'",
		(subclass) ? subclass : "*",
		(type) ? type : "*");
	
	DBGMSG((stderr, "looking for keywords: %s", crit));
	pdatp = get_PdAt_list(PdAt_CLASS, crit, &pdat_info, 1, 1);
	if ((int) pdatp == -1) {
		ERR(Msg(GETPDAT,
			"0821-023 defif: Cannot get records from PdAt.\n"));
		CFGEXIT(E_ODMGET);
	}

	DBGMSG((stderr, "number of predefined attr 'if_keywords' found = %d",
		pdat_info.num));
	/*
	 * for each keyword of a predefined if, we look it up in PdCn
	 * to find the corresponding adaptor.
	 */
	for (i=0, atp=pdatp; i<pdat_info.num; i++,atp++) {

		/*
		 * for IF's that don't have adaptors (i.e. loopback) 
		 * we automatically configure them.
		 */
		
		DBGMSG((stderr, "if_keyword: %s", atp->deflt));
		
		if (strcmp(atp->deflt, "none") == 0) {
			sprintf(crit, "uniquetype = '%s'", atp->uniquetype);
			DBGMSG((stderr, "Querying PdDv for: %s", crit));
			pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info,
					      1, 1);
			if ((int) pddvp == -1) {
				ERR(Msg(GETPDDV,
			"0821-024 defif: Cannot get records from PdDv.\n"));
				CFGEXIT(E_ODMGET);
			}
			if (pddv_info.num < 1) {
				ERR(Msg(NOPDIF,
			"0821-031 defif: Cannot find interface: %s\n"),
				    atp->uniquetype);
				CFGEXIT(E_NOPdDv);
			}

			/* check if loopback -- if not, don't define any
			 *	if not explicitly passed with type and
			 *	subclass information
			 */
			if ( strcmp ( pddvp->prefix, "lo" )) {
			    if (define_one) {
	 			/* generate logical name for device
				 * without matching adapter
				 */
				if ((seqno=genseq(pddvp->prefix))<0) {
					/* error making logical name */
					ERR( Msg ( CANTMKLGNM,
			    		"0821-035 defif: failure in making logical name\n"));
					CFGEXIT(1);
				}
				sprintf ( ifname, "%s%d", pddvp->prefix, seqno);
				defif(ifname);
				return(0);
			    }
			    else
				continue;
			}

			sprintf(ifname, "%s0", pddvp->prefix);
			if ((ndef = defif(ifname)) && define_one)
				return(0);
			continue;
		}
			
		strcpy(crit, "connkey = '");
		strcat(crit, atp->deflt);
		strcat(crit, "'");
		DBGMSG((stderr, "Querying PdCn for: %s", crit));
		pdcnp = get_PdCn_list(PdCn_CLASS, crit, &pdcn_info, 1, 1);
		if ((int) pdcnp == -1) {
			ERR(Msg(GETADAPT,
			"0821-020 defif: Cannot get records from PdCn.\n"));
			CFGEXIT(E_ODMGET);
		}

		/*
		 * if adaptors are found, the uniquetype is the adaptor,
		 * and the connkey is the keyword for that adaptor which 
		 * specifies the type of adaptor it is: e.g. "token",
		 * "ent".
		 * the connwhere field in PdCn is the interface device
		 * subclass (EN, TR, ...)
		 */
		if (pdcn_info.num == 0) continue;

		/*
		 * for each adaptor, look up in
		 * CuDv to see if an adaptor instance is defined.  
		 */
		for (k=pdcnp, j=0; j<pdcn_info.num; j++, k++) {

			/*
			 * find the predefined adaptor.
			 */
			sprintf(crit, "uniquetype = '%s'", k->uniquetype);
			DBGMSG((stderr, "looking for Pd adaptor: %s", crit));
			pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info,
					      1, 1);
			if ((int) pddvp == -1) {
				ERR(Msg(GETPDADAP,
			"0821-022 defif: Cannot get predefined adaptor.\n"));
				CFGEXIT(E_ODMGET);
			}
			if (pddv_info.num != 1) {
				ERR(Msg(PDADAPNUM,
	"0821-029 defif: Expected one predefined adaptor, retreived %d.\n"),
					pddv_info.num);
				CFGEXIT(E_ODMGET);
			}
			

			/*
			 * find the customized adaptor, if present.
			 *
			 * If this is the Serial Optical driver, then 
			 * it may either be in the AVAILABLE -OR- DEFINED 
			 * states.  Either state 
			 * is OK to define a so interface...
			 */
			if (strcmp(pddvp->prefix, "ops") == 0) 
				sprintf(crit, "name like '%s[0-9]*'", 
					pddvp->prefix);
			else
				sprintf(crit, 
					"name like '%s[0-9]*' and status = %d",
					pddvp->prefix, AVAILABLE);
			DBGMSG((stderr, "Querying CuDv for: %s", crit));
			cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info,
					      1, 1);

			if ((int) cudvp == -1 || cudv_info.num == 0) {
			DBGMSG((stderr, "no customized adaptors (%s) found",
			       pddvp->prefix));
				continue; /* no instances were found */
			}

			/*
			 * since we have found instance(s) for this adaptor,
			 * we now need to define an interface for EACH adaptor
			 * instance.
			 * first we look up the prefix name of the INTERFACE
			 * we want to define in PdDv.
			 */
			strcpy(crit, "uniquetype = '");
			strcat(crit, atp->uniquetype);
			strcat(crit, "'");
			DBGMSG((stderr, "Querying PdDv for: %s", crit));
			pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info,
					      1, 1);
			if ((int) pddvp == -1) {
				ERR(Msg(GETPDDV,
			"0821-024 defif: Cannot get records from PdDv.\n"));
				CFGEXIT(E_ODMGET);
			}
			if (pddv_info.num < 1) {
				ERR(Msg(NOPDIF,
			"0821-031 defif: Cannot find interface: %s\n"),
				    atp->uniquetype);
				CFGEXIT(E_ODMGET);
			}
			/*
			 * then for each adaptor instance we found, we define
			 * an interface.  
			 * but if we are only defining one new IF, then
			 * stop after one is defined.
			 */
			for (n=0, d=cudvp; n<cudv_info.num; n++, d++) {
				register char *cp;
				register int i;
        
				/* construct the ifname for the defif call */
				strcpy(ifname, pddvp->prefix);
        
				for (cp=d->name; *cp; cp++)
					if (isdigit(*cp))
						break;
				t = cp - d->name;
				i = strlen(ifname);
				strcpy(ifname+i, (d->name)+t);
        
				if ((ndef = defif(ifname)) && define_one)
					return(0);
				DBGMSG((stderr, "ndef = %d", ndef));
			}
		}
	}

	return( (ndef) ? 0 : -1 );
}


/*
 * NAME: defif
 *                                                                    
 * FUNCTION: defines one given interface
 *                                                                    
 * NOTES:
 *
 * RETURNS:  0 if nothing is defined (already defined).
 *	     1 if a new IF gets defined.
 */
int
defif(char *ifname)		/* name of if to define */
{
	CLASS_SYMBOL cudv_hndl, cudep_hndl;
	struct PdDv *pddvp;
	struct CuDv cudv;
	struct PdAt *pdatp;
	struct CuDep cudep;
	struct listinfo cudv_info, pddv_info, pdat_info, cudep_info;
	char crit[128];

	DBGMSG((stderr, "defining interface: %s", ifname));
	/*
	 * first check to see if it already has been defined.
	 */
	
	sprintf(crit, "name = '%s'", ifname);
	DBGMSG((stderr, "Querying CuDv for: %s", crit));
	if( ((int)odm_get_first(CuDv_CLASS, crit, &cudv)) > 0 ) {
		return(0);
	}
	
	cudep_hndl = odm_open_class(CuDep_CLASS);
	if ((int) cudep_hndl == -1) {
		ERR(Msg(OPENCUDP, "0821-025 defif: Cannot open CuDep.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	cudv_hndl = odm_open_class(CuDv_CLASS);
	if ((int) cudv_hndl == -1) {
		ERR(Msg(OPENCUDV, "0821-026 defif: Cannot open CuDv.\n"));
		CFGEXIT(E_ODMOPEN);
	}

	/*
	 * from the ifname we get the prefix.  we look up the prefix 
	 * in the PdDv class to find the predefined info for this
	 * interface.  Take the predefined info, create an if instance 
	 * record for CuDv, and drop it into the database.
	 */

	strcpy(crit, "prefix = '");
	strncat(crit, ifname, 2);
	strcat(crit, "'");
	DBGMSG((stderr, "Querying PdDv for: %s", crit));
	pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info, 1, 1);
	if ((int) pddvp == -1) {
		ERR(Msg(GETPDDV,
			"0821-024 defif: Cannot get records from PdDv.\n"));
		CFGEXIT(E_ODMGET);
	}
	
	/* create a CuDv record for adding */
	
	memset(&cudv, 0, sizeof(struct CuDv));
	strcpy(cudv.name, ifname);
	cudv.status = DEFINED;
	cudv.chgstatus = 1;	/* DONT_CARE */
	strcpy(cudv.ddins, pddvp->DvDr);
	*(cudv.location) = 0;	/* field not used */
	strcpy(cudv.parent, "inet0");
	*(cudv.connwhere) = 0; /* field not used */
	cudv.PdDvLn = pddvp;
	cudv.PdDvLn_info = &pddv_info;
	strcpy(cudv.PdDvLn_Lvalue, pddvp->uniquetype);

	/*
	 * stick the customized interface record into ODM.
	 */
	DBGMSG((stderr, "Adding the customized if record for: %s",
		cudv.name));
	if (odm_add_obj(cudv_hndl, &cudv) == -1) {
		ERR(Msg(BADADD,
	"0821-014 defif: Cannot add a customized interface record.\n"));
		CFGEXIT(E_ODMADD);
	}
	/*
	 * now we write the name of the defined device to stdout.
	 * so cfgmgr and hi-level commands work.
	 */
	printf("%s\n", cudv.name);

	/*
	 *  update the the Customized Dependancy info.
	 */
	memcpy(&cudep, 0, sizeof(cudep));
	strncpy(cudep.name, ifname, sizeof(cudep.name));
	strcpy(cudep.dependency, "inet0");

	/*
	 * stick the customized dependency record into ODM.
	 */
	DBGMSG((stderr, "Adding the customized dependency record for: %s",
		cudv.name));
	if (odm_add_obj(cudep_hndl, &cudep) == -1) {
		ERR(Msg(BADADDDEP,
	"0821-015 defif: Cannot add a customized dependency record.\n"));
		CFGEXIT(E_ODMADD);
	}

	odm_close_class(cudv_hndl);
	odm_close_class(cudep_hndl);
	return(1);
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
		Msg(USAGE1, "\nusage:\t%s  -c class -s subclass -t type\n"),
		progname);
	fprintf(stderr,
		Msg(USAGE2, "\t%s  -h\n"),
		progname);
	odm_terminate();
	CFGEXIT(E_ARGS);
}
