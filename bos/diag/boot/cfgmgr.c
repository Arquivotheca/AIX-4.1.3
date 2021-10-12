static char sccsid[] = "@(#)78	1.3  src/bos/diag/boot/cfgmgr.c, diagboot, bos41J, 9516B_all 4/20/95 16:42:37";
/*
 * COMPONENT_NAME: DIAGBOOT
 *
 * FUNCTIONS: cfgmgr for diagnostics boot
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <odmi.h>
#include <sys/cfgdb.h>
#include <sys/priv.h>	/* privilege */
#include <sys/signal.h>
#include <sys/cfgodm.h> /* classes structure dcls */
#include <cf.h>

#define MSGSTR(num)		msgstr[num]
#define ERRSTR(num)		errstr[num]
#define METHERR(num)		meth_err_msg[num]

/*---------------------------- message strings ------------------------------*/
#define MSG_OUT_INFO	1
#define MSG_NO_OUT	2
#define MSG_ERR_INFO	3
#define MSG_NO_ERR	4
#define MSG_PHASE1	5
#define MSG_PHASE2	6
#define MSG_PHASE2MAINT	7
#define MSG_BAD_PHASE	8
#define MSG_TOP_LEVEL	9
#define MSG_RC		10
#define MSG_SAVEBASE	11
#define MSG_CONFIG_DEV	12
#define MSG_INVOKING	13
#define MSG_METHERR	14
#define MSG_UNCFG_DEV	15

char *msgstr[] =
{
"",
"****************** stdout ***********\n%s\n",
"****************** no stdout ***********\n",
"****************** stderr ***********\n%s\n",
"****************** no stderr ***********\n",
"cfgmgr is running in PHASE1\n",
"cfgmgr is running in PHASE2\n",
"cfgmgr is running in PHASE2MAINT\n",
"cfgmgr : unknown phase = %d\n",
"invoking top level program -- \'%s\'\n",
"return code = %d\n",
"calling savebase\n",
"attempting to configure device '%s'\n",
"invoking %s %s\n",
"Method error (%s):\n",
"attempting to UNconfigure device '%s'\n",
};

/*------------------------------ error messages ----------------------------*/
/* WARNING - these error numbers correspond to led values which are REGISTERED*/
/*    in some documents, so they cannot be changed!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define ERR_SYNTAX	1	/* syntax error in command line */
#define ERR_CONFLICT	2	/* option conflict */
#define ERR_ODM_INIT	3	/* can't initialize ODM */
#define ERR_GET_RULES	4	/* can't access config rules object */
#define ERR_GET_CUDV	5	/* can't get data from customized device obj. */
#define ERR_GET_CUDVDR	6	/* can't get data from customized device
				   driver object */
#define ERR_PHASE1_ILLEGAL 7	/* phase1 illegal at this point */
#define ERR_NO_SEQ	8	/* can't find sequence rule */
#define ERR_UPDATE_ODM	9	/* can't update odm data base */
#define ERR_SAVEBASE	10	/* savebase failure */
#define ERR_GET_PDAT	11	/* can't access the PdAt class */
#define ERR_NO_MEMORY	12	/* can't allocate system memory */
#define ERR_CONFIGMTH	13	/* no config method for device */
#define ERR_LOCK_ACC	14	/* lock acquire error */
#define ERR_UNUSED	15	/* currently unused */
#define ERR_OBSOLETE1	16	/* obsolete error code */
#define ERR_OBSOLETE2	17	/* obsolete error code */
#define ERR_PROGRAM	18	/* no error - just used to set leds to show */
				/*   that a program is being invoked */
#define ERR_RETURN	19	/* no error - just used to set leds to show */
				/*   that we have returned from odm_run_method*/
#define ERR_UNCFGMTH	20	/* no unconfig method for device */


char *errstr[] =
{
"",
"cfgmgr: 0514-600 Usage error : %s\nUsage: cfgmgr [-f|-s] [-v]\n",
"cfgmgr: 0514-601 Flag conflict : %s\nUsage: cfgmgr [-f|-s] [-v]\n",
"cfgmgr: 0514-602 Cannot initialize the ODM data base.\n",
"cfgmgr: 0514-603 Cannot access the Config_Rules object class in the\n\
\tdevice configuration database.\n",
"cfgmgr: 0514-604 Cannot access the CuDv object class in the\n\
\tdevice configuration database.\n",
"cfgmgr: 0514-605 Cannot access the CuDvDr object class in the\n\
\tdevice configuration database.\n",
"cfgmgr: 0514-606 Running PHASE1 at this point is not valid.\n",
"cfgmgr: 0514-607 There are no sequence rules to execute for the\n\
\tspecified phase.\n",
"cfgmgr: 0514-608 Cannot update an object in the device\n\
\tconfiguration database.\n",
"cfgmgr: 0514-609 Unable to save the base customized information\n\
\ton /dev/ipldevice.\n%s\n",
"cfgmgr: 0514-610 Cannot access the PdAt object class in the\n\
\tdevice configuration database.\n",
"cfgmgr: 0514-611 There is not enough memory available now.\n",
"cfgmgr: 0514-612 Unable to configure the %s device because\n\
\tit either does not exist in the customized device database or it does not\n\
\thave a corresponding entry in the predefined device database.\n",
"cfgmgr: 0514-613 Unable to acquire the device configuration database\n\
\tlock. The errno is %d.\n",
"currently unused",
"obsolete error",
"obsolete error",
"going to run a program",
"return from running a program",
"cfgmgr: 0514-620 Unable to acquire the device UNconfiguration database\n\
\tlock. The errno is %d.\n",
};

/*---------------------------------- exit status ---------------------------*/
#define EXIT_NOT	-1
#define EXIT_OK		0
#define EXIT_FATAL	1

/*---------------------------------- leds ----------------------------------*/
#define LEDS_BASE	0x520	/* first cfgmgr led value */
#define LEDS_PROGRAM	0x538	/* led value to signify a program being run */
#define LEDS_RETURN	0x539	/* led value to signify a program being run */
#define LEDS(num)	{if (!runtime) setleds(num);}

/*------------------------------------ misc --------------------------------*/
#define SEPAR		','	/* node/device name separator */
#define QUOTE		'\"'	/* quote */

/*--------------------------------- global vars -----------------------------*/
int 		verbose;		/* >0 if verbose output desired */
short		phase;			/* current phase */
short		phase3=0;		/* unconfigure devices recursively */
int		runtime;		/* >0 if cfgmgr called without flags */
extern	char	*malloc();		/* memory allocation routine */
extern	int	errno;			/* errno */

/* FUNCTION PROTOTYPES */
int init_dev_data(void);
int configure_node(void);
int configure(char **, int, int);
int configure_dev(char *, char **);
int unconfigure_dev(char *);
int showerr(int, char *, int);
int noctrlch(char *);
void privilege(int);
struct Config_Rules **sort_seq(struct Config_Rules [], int);

/*
* NAME:  cfgmgr
*
* FUNCTION:
*	Configuration manager is responsible for device configuration
*	during IPL.  The config manager is divided into two phases.  The
*	first phase is involved when the kernel is brought into the system and
*	only the boot file system is active.  The boot file system is usually
*	a subset of a full functional root file system.  The responsibility
*	of the config manager in phase I is to configure base devices so
*	that the root device(s) can be configured and be ready for operation.
*	After the root device(s) is/are configured, the init program switches
*	the root directory to the new device(s).  The second phase of config
*	manager is to configure the rest of the devices in the system
*	after the root file system is up and running.  The config manager
*	is a simple routine which is mainly driven by the config rules
*	object class in the data base.
*	The cfgmgr can also be invoked during run time to configure any
*	detectable devices which are not being configured during the IPL time.
*	Those devices may be turned off at IPL time or newly added devices.
*
*	Added 4/05/90
*	A third phase that configures in a depth first fashion and unconfigures
*	the devices after it is through with each branch.
*
*
* EXECUTION ENVIRONMENT:
*
* EXECUTION ENVIRONMENT:
*
*
* DATA STRUCTURES: config rule object class
*		    device predefined and customized object classes
*
* INPUT PARAMETERS:
*			argv -- input parameters
*				1st parm -- "-f"	first phase
*					 or "-s"	second phase
*					 or "-t"	third phase
*				"-n"			no initialization
*			     -- none -- run time option
*
* RETURN VALUE DESCRIPTION:  0	-- no errors 
*			      -1 -- fatal error 
*
* EXTERNAL PROCEDURES CALLED:
*
*/
int main(
	int	argc,
	char	*argv[])	/* 1st parm if "-f" -- first phase  */
				/*	       "-s" -- second phase */
				/*	       "-t" -- third phase  */
{
	int	rc;		/* return code */
	char	*cp;		/* temporary character pointer */
	int	c;		/* option character */
	struct PdAt pdat;	/* PdAt object */
	extern char *optarg;	/* getopt */
	extern int optind;	/* getopt */
   	struct sigaction action;/* parameters of sigaction */
	struct sigaction oldaction;
	struct	CuDv *CuDv;
	struct	listinfo info;
	int 	i;

	/* ignore these signals */
	memset(&action, '\0', sizeof (struct sigaction));
	action.sa_handler = SIG_IGN;
	sigaction (SIGQUIT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGINT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGTERM, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGHUP, &action, &oldaction);

	/* initialize internal state */
	privilege(PRIV_LAPSE);

	/* initialize values */
	verbose = phase = runtime = FALSE;	

	/* initialize the ODM */
	if ( odm_initialize() ) 
	   showerr (ERR_ODM_INIT, NULL,EXIT_FATAL);

	/* parse the command line */
	while (( c = getopt(argc, argv, "stfvd")) != EOF )
	   switch (c)
	   {  case 's' : if ( phase == PHASE1 )
			    showerr( ERR_CONFLICT,"-s",EXIT_FATAL );
			 phase = PHASE2;
			 break;
	      case 'f' : if ( phase == PHASE2 )
			    showerr( ERR_CONFLICT,"-f",EXIT_FATAL );
			 phase = PHASE1;
			 break;
	      case 't' : if (( phase == PHASE2 ) || ( phase == PHASE1 ))
			    showerr( ERR_CONFLICT,"-t",EXIT_FATAL );
			 phase = PHASE2;
			 phase3 = 1;
			 break;
	      case 'd' :
	      case 'v' : verbose = TRUE;
			 break;
	      default  : showerr( ERR_SYNTAX, NULL,EXIT_FATAL );
			 break;
	   }/*end switch*/

	/* check for other args */
	if (optind < argc)
	   showerr( ERR_SYNTAX, argv[optind],EXIT_FATAL );

	/* if no phase specified, use PHASE2 as the default */
	/* when no phase is specified, do not show leds */
	if (phase == 0)
	{  phase = PHASE2;
	   runtime = TRUE;
	}

	if ( phase == PHASE1 )
	{  /* when BOS is installed on a system, the database
	   in /etc/objrepos is already populated.  One of
	   the objects in PdAt is for use here - that's on
	   the real file system.  When we're in phase 1 of
	   IPL, we're running out of the RAM file system -
	   the database in /etc/objrepos is different because
	   it uses the database in /etc/objrepos/boot, which isn't
	   populated with the PdAt object we're looking for.
	   Therefore, if this PdAt object is there, we're using
	   the real file system, which means that phase 1 has already
	   been run, and is NOT legal at this point */
	   if ((rc = (int)odm_get_first(PdAt_CLASS,PHASE1_DISALLOWED,
				   &pdat)) == -1)
	      showerr(ERR_GET_PDAT,NULL,EXIT_FATAL);
	   else if (rc != (int) NULL)
	      showerr(ERR_PHASE1_ILLEGAL,NULL,EXIT_FATAL);

	   /* this is phase 1 IPL */

	   /* reset some of the database info */
	   init_dev_data();

	}/* phase1 */

	/* config all dev according the sequence rules */
	configure_node();

	/* security */
	privilege(PRIV_ACQUIRE);

	/*
	 *  - mark all customized device objects as new
	 */
	if ((struct CuDv *) -1 == 
	    (CuDv = get_CuDv_list(CuDv_CLASS,"",&info,100,1)) ) 
	   showerr( ERR_GET_CUDV, NULL, EXIT_FATAL );

	for(i=0;i<info.num;i++)
	{
		if ( (CuDv[i].chgstatus != SAME) &&
		     (CuDv[i].chgstatus != DONT_CARE) ){
			CuDv[i].chgstatus = NEW;
			if (odm_change_obj(CuDv_CLASS,&CuDv[i]) == -1)
			   showerr( ERR_UPDATE_ODM, NULL, EXIT_FATAL );
		}
	}
	odm_free_list(CuDv,&info);

	/* security */
	privilege(PRIV_LAPSE);

	odm_terminate();

	/* blank out leds */
	setleds( 0xfff );

	exit( EXIT_OK );

}  /* end cfgmgr */

/*
*  NAME:  init_dev_data
*
*  FUNCTION: initialize device variables during IPL.
*		Some device variables have to be initialized during IPL --
*			e.g. device driver use count, device state, etc..
*
*  RETURN VALUE:
*		0  -- successful
*		non-zero -- failed
*
*/
int init_dev_data(void)
{
	int	num, i, rc;
	short	int	**config_ct;		/* driver use count */
	short	int	status; 	/* device status */
	short	int	chgstatus;	/* change status */
	char	value2[4];	/* all value should be stored as string */
	struct	CuDv *CuDv;
	struct	CuDvDr *CuDvDr;
	struct	listinfo info;

	/* security */
	privilege(PRIV_ACQUIRE);

	/*
	 *  - mark all customized device objects as missing
	 *  - move all current device state to previous state and marked
	 *	current device state to defined.
	 */
	if ((struct CuDv *) -1 == 
	    (CuDv = get_CuDv_list(CuDv_CLASS,"",&info,100,1)) ) 
	   showerr( ERR_GET_CUDV, NULL, EXIT_FATAL );

	for(i=0;i<info.num;i++)
	{
		CuDv[i].status = DEFINED;

		if (CuDv[i].chgstatus != DONT_CARE)
			CuDv[i].chgstatus = MISSING;
		if (odm_change_obj(CuDv_CLASS,&CuDv[i]) == -1)
		   showerr( ERR_UPDATE_ODM, NULL, EXIT_FATAL );
	}
	odm_free_list(CuDv,&info);


	if ((CuDvDr = (struct CuDvDr *)
	     odm_get_list(CuDvDr_CLASS,
			"resource = 'dd'",&info,100,1)) == (struct CuDvDr *)-1)
	   showerr( ERR_GET_CUDVDR, NULL, EXIT_FATAL );

	for(i=0;i<info.num;i++)
	{
		strcpy(CuDvDr[i].value2, "0");		/* config use count */
		if (odm_change_obj(CuDvDr_CLASS,&CuDvDr[i]) == -1)
		   showerr( ERR_UPDATE_ODM, NULL, EXIT_FATAL );
	}
	odm_free_list(CuDvDr,&info);

	/* security */
	privilege(PRIV_LAPSE);
}


/*
*  NAME:  configure_node
*
*  FUNCTION:  configure devices under a node in the sequence rule
*
*  RETURN VALUE:
*		0  -- successful
*		non-zero -- failed
*
*/
int configure_node(void)
{
  struct Config_Rules *rules;
  struct listinfo me_info;
  char criteria[MAX_CRITELEM_LEN];
  char **name_list;
  char *name;
  char *out_ptr;
  char *err_ptr;
  int len;
  int seq;
  int i,j;
  int rc;
  int entries;
  struct Config_Rules **list;

	/* if phase 2, check front panel key position in maintenance mode */
	/* key pos -- 1 secure,2 service,3 normal*/
	if ( phase == PHASE2 )
	   phase = PHASE2MAINT;

	/* initialize the criteria */
	sprintf(criteria,"phase = %d", phase);

	if (verbose)
	{  printf("---------------------------------------------------------\
--------------\n");
	   if (phase == PHASE1)
	      printf(MSGSTR(MSG_PHASE1));
	   else if (phase == PHASE2)
	      printf(MSGSTR(MSG_PHASE2));
	   else if (phase == PHASE2MAINT)
	      printf(MSGSTR(MSG_PHASE2MAINT));
	   else
	      printf(MSGSTR(MSG_BAD_PHASE),phase);
	}

	/* get all the rules for this phase */
	if ((rules =(struct Config_Rules *)
	    odm_get_list(Config_Rules_CLASS,criteria,
			&me_info,1,1))==(struct Config_Rules *)-1)
	   showerr( ERR_GET_RULES, NULL, EXIT_FATAL );

	/* if there is no sequence rule, return */
	if ( me_info.num == 0 )
	   showerr( ERR_NO_SEQ , NULL, EXIT_FATAL );

	/* sort the rules by me.seq value */
	list = sort_seq( rules, me_info.num );

	/* execute each rule */
	for (seq=0; seq < me_info.num; seq++)
	{  if (verbose)
	   {  printf("---------------------------------------------------------\
--------------\n");
	      printf(MSGSTR(MSG_TOP_LEVEL),list[seq]->rule);
	   }

	   /* show that a program is being executed */
	   LEDS( LEDS_PROGRAM )

	   /* execute this program */
	   rc = odm_run_method( list[seq]->rule, "", &out_ptr, &err_ptr );

	   /* show that we have returned from the program */
	   LEDS( LEDS_RETURN )

	   if (verbose)
	   {   printf(MSGSTR(MSG_RC),rc);
	       if (noctrlch(out_ptr))
	          printf(MSGSTR(MSG_OUT_INFO),out_ptr);
	       else
	          printf(MSGSTR(MSG_NO_OUT));
	       if (noctrlch(err_ptr))
	          printf(MSGSTR(MSG_ERR_INFO),err_ptr);
	       else
	          printf(MSGSTR(MSG_NO_ERR));
	    }

	    /* check for an error */
	    rc = ((rc < 0) || (rc > E_LAST_ERROR)) ? E_LAST_ERROR : rc;
	    if (rc)
	    {  fprintf(stderr,MSGSTR(MSG_METHERR),list[seq]->rule);
	       fprintf(stderr,METHERR(rc));
	       if (err_ptr != NULL)
	       {  /* print whatever the method wrote */
		  if ((*err_ptr != '\0') && (*err_ptr != '\n'))
	             fprintf(stderr,"%s\n",err_ptr);
		  free( err_ptr );
	       }
	    }

	    /* if nothing returned, nothing else to configure */
	    if( (out_ptr == (char *) 0) || (strlen(out_ptr) == 0) )
		continue;

	   /* malloc space for name_list, which will store the addresses */
	   /*   of the out_ptr buffers which will be returned from the   */
	   /*   configure methods */
	   if ( (name_list = (char **) malloc ( 15 * sizeof (char *))) == 0 )
	      showerr(ERR_NO_MEMORY, NULL, EXIT_FATAL);

	   /* initialize the name_list */
	   j = 0;	
	   entries = 1;
	   name_list[0] = out_ptr;

	   configure( name_list, entries, j );		

	   /* free up the name_list */
	   free( (void *)name_list );

	}/* end for (seq < me_info.num) */

	odm_free_list(rules,&me_info);
	free(list);
	return (0);
}


/*
* NAME:  configure
*
* FUNCTION:
*	Processes the output of the sequence rule, calling config methods
*	and stacking their output in name_list.  
*
* EXECUTION ENVIRONMENT:
*
* DATA STRUCTURES:
*
* INPUT PARAMETERS:
*	name_list -- The array of entries.
*	entries   -- The total number of entries
*	j	  -- current entry in name_list
*
* RETURN VALUE DESCRIPTION:  0 -- successful
*			      non-zero --  failed
*
* EXTERNAL PROCEDURES CALLED:
*	
*/
int configure(
	char **name_list,
	int entries,
	int j)
{
	int 	i, len, rc;
	char 	*name, *out_ptr;

	/* process each list of names of devices to be configured */
	for (; j < entries; j++ )
	{  /* seperate the names */
	   len = strlen( name_list[j] );
	   i = 0;

	   while (i < len)
	   {  /* point to the current name */
	      name = &(name_list[j][i]);

	      /* look for a seperator char */
	      for (; i < len; i++)
		 if ( (name_list[j][i] == ' ') ||
		      (name_list[j][i] == ',') ||
		      (name_list[j][i] == '\n') ||
		      (name_list[j][i] == '\t') )
		 {  /* take everything up to this point as a program */
		     name_list[j][i] = '\0';

		     /* increment past this point for next loop */
		     i++;
		     break;
		 }

	      /* check for null name - just in case */
	      if (name[0] == '\0')
	         continue;

	      out_ptr = NULL;		

	      /* configure this device */
	      rc = configure_dev( name, &out_ptr );

	      /* any output from that configure method??? */
	      if ((out_ptr != NULL) && (strlen(out_ptr) != 0) )
	      {  /* add this to the names list */
	         name_list[entries++] = out_ptr;

	         if ( (entries%15) == 0 )
		 {  /* allocate more memory for the name_list */
		    if ( (name_list = (char **) 
		          realloc(name_list,(entries+15) * sizeof (char *)))
			     == (char **) NULL )
		       showerr(ERR_NO_MEMORY, NULL,EXIT_FATAL);
		 }

		 if ( phase3 )
		    configure( name_list, entries, j+1 );	
	      }

	      if ( phase3 && !rc)
	         unconfigure_dev( name );

	   }/* end while (i < len) */

	   /* done with this list, set it to null, decrement index */
	   if ( strlen( name_list[j] ) ){
	      name_list[j][0] = '\0';
	      if ( phase3 )	
			entries--;
	   }

	}/* end for (j < entries) */
	
	return(0);

}


/*
* NAME:  configure_dev
*
* FUNCTION:
*	To invoke the configure method of the device.
*
* EXECUTION ENVIRONMENT:
*
* DATA STRUCTURES:
*
* INPUT PARAMETERS:
*	device -- device logical name
*	names-ret -- device name ptr for the list of device names
*			to be returned to be configured
*
* RETURN VALUE DESCRIPTION:  0 -- successful
*			      non-zero --  failed
*
* EXTERNAL PROCEDURES CALLED:
*		individual device config method
*/
int configure_dev(
	char *device,
	char **out_ptr)
{
  char criteria[MAX_CRITELEM_LEN];
  struct CuDv *cudv;
  struct listinfo cudv_info;
  char *err_ptr;
  int rc;
  int lock_id;
  char params[100];

	*out_ptr = 0;

	if (verbose)
	{  printf("---------------------------------------------------------\
--------------\n");
	   printf (MSGSTR(MSG_CONFIG_DEV),device);
	}

	/* get the PdDv info for this device */
	sprintf (criteria, "name = '%s'",device);
	if ((cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,criteria,
				&cudv_info,1,2)) == (struct CuDv *) -1)
	{  showerr( ERR_GET_CUDV, device, EXIT_NOT );
	   printf("\n");
	   return( -1 );
	}

	if ( (cudv_info.num != 1) || (!cudv[0].PdDvLn) )
	{  showerr ( ERR_CONFIGMTH , device, EXIT_NOT);
	   printf("\n");
	   return( -1 );
	}

	/* initialize the method parameter string */
	if (phase == PHASE1)
	   sprintf( params, "-1 -l %s", device );
	else if (!runtime)
	   sprintf( params, "-2 -l %s", device );
	else
	   sprintf( params, "-l %s", device );

	/* security */
	privilege(PRIV_ACQUIRE);

	if (verbose)
	   printf(MSGSTR(MSG_INVOKING), cudv[0].PdDvLn->Configure, params);

	/* acquire data base lock */
	if ((lock_id = odm_lock( "/etc/objrepos/config_lock", ODM_WAIT)) == -1)
	   showerr(ERR_LOCK_ACC, (char *)errno, EXIT_NOT);

	/* show that a program is being executed */
	LEDS( LEDS_PROGRAM )

	/* invoke the configure method */
 	rc = odm_run_method(cudv[0].PdDvLn->Configure,params,out_ptr,&err_ptr);

	/* show that we have returned from the program */
	LEDS( LEDS_RETURN )

	if (verbose)
	{  printf(MSGSTR(MSG_RC),rc);
	   if (noctrlch(*out_ptr))
	      printf(MSGSTR(MSG_OUT_INFO),*out_ptr);
	   else
	      printf(MSGSTR(MSG_NO_OUT));
	   if (noctrlch(err_ptr))
	      printf(MSGSTR(MSG_ERR_INFO),err_ptr);
	   else
	      printf(MSGSTR(MSG_NO_ERR));
	}

	/* check for an error */
	rc = ((rc < 0) || (rc > E_LAST_ERROR)) ? E_LAST_ERROR : rc;
	if (rc)
	  {
	  fprintf(stderr,MSGSTR(MSG_METHERR),cudv[0].PdDvLn->Configure);
	  fprintf(stderr,METHERR(rc));
	  }

	/* unlock the database */
	odm_unlock(lock_id);

	/* out-security */
	privilege(PRIV_LAPSE);

	/* print stderr stuff */
	if (err_ptr != NULL)
	{  if ((*err_ptr != '\0') && (*err_ptr != '\n'))
	      fprintf(stderr,"%s\n",err_ptr);
	   free( err_ptr );
	}
	odm_free_list(cudv,&cudv_info);

	return( rc );
}


/*
* NAME:  unconfigure_dev
*
* FUNCTION:
*	To invoke the unconfigure method of the device.
*
* EXECUTION ENVIRONMENT:
*
* DATA STRUCTURES:
*
* INPUT PARAMETERS:
*	device -- device logical name
*
* RETURN VALUE DESCRIPTION:  0 -- successful
*			      non-zero --  failed
*
* EXTERNAL PROCEDURES CALLED:
*		individual device unconfig method
*/
int unconfigure_dev(char *device)
{
  char criteria[MAX_CRITELEM_LEN];
  struct CuDv *cudv;
  struct listinfo cudv_info;
  char *err_ptr;
  int rc;
  int lock_id;
  char params[100];
  char *out_ptr;	

	out_ptr = 0;

	if (verbose)
	{  printf("---------------------------------------------------------\
--------------\n");
	   printf (MSGSTR(MSG_UNCFG_DEV),device);
	}

	/* get the PdDv info for this device */
	sprintf (criteria, "name = '%s'",device);
	if ((cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,criteria,
				&cudv_info,1,2)) == (struct CuDv *) -1)
	{  showerr( ERR_GET_CUDV, device, EXIT_NOT );
	   printf("\n");
	   return( -1 );
	}

	if ( (cudv_info.num != 1) || (!cudv[0].PdDvLn) )
	{  showerr ( ERR_UNCFGMTH , device, EXIT_NOT);
	   printf("\n");
	   return( -1 );
	}
	/* Return if there is no Unconfigure methods */

	if(cudv[0].PdDvLn->Unconfigure[0] == '\0')
	   return( 0 );

	/* Return if this device needs to stay configured at all times. */
	/* For example, Disk arrays controller needs to be configured   */
	/* otherwise the Config Rule for the pseudo device (dar) will   */
	/* fail, and the physical disks will not be configured.	        */
	
	if (!strcmp(cudv[0].PdDvLn->type, "dac7135") &&
	    !strcmp(cudv[0].PdDvLn->class, "array") &&
	    !strcmp(cudv[0].PdDvLn->subclass, "scsi"))
		    return(0);

	/* initialize the method parameter string */
	sprintf( params, "-l %s", device );

	/* security */
	privilege(PRIV_ACQUIRE);

	if (verbose)
	   printf(MSGSTR(MSG_INVOKING), cudv[0].PdDvLn->Unconfigure, params);

	/* acquire data base lock */
	if ( (lock_id=odm_lock( "/etc/objrepos/config_lock", ODM_WAIT)) == -1)
	   showerr(ERR_LOCK_ACC, (char *)errno, EXIT_NOT);

	/* show that a program is being executed */
	LEDS( LEDS_PROGRAM )

	/* invoke the configure method */
 	rc=odm_run_method(cudv[0].PdDvLn->Unconfigure,params,&out_ptr,&err_ptr);

	/* show that we have returned from the program */
	LEDS( LEDS_RETURN )

	if (verbose)
	{  printf(MSGSTR(MSG_RC),rc);
	   if (noctrlch(out_ptr))
	      printf(MSGSTR(MSG_OUT_INFO),*out_ptr);
	   else
	      printf(MSGSTR(MSG_NO_OUT));
	   if (noctrlch(err_ptr))
	      printf(MSGSTR(MSG_ERR_INFO),err_ptr);
	   else
	      printf(MSGSTR(MSG_NO_ERR));
	}

	/* check for an error */
	rc = ((rc < 0) || (rc > E_LAST_ERROR)) ? E_LAST_ERROR : rc;
	if (rc)
	  {
	  fprintf(stderr,MSGSTR(MSG_METHERR),cudv[0].PdDvLn->Unconfigure);
	  fprintf(stderr,METHERR(rc));
	  if ((out_ptr != NULL) && (strlen( out_ptr )))
	    free( out_ptr );
	  }

	/* unlock the database */
	odm_unlock(lock_id);

	/* out-security */
	privilege(PRIV_LAPSE);

	/* print stderr stuff */
	if (err_ptr != NULL)
	{  if ((*err_ptr != '\0') && (*err_ptr != '\n'))
	      fprintf(stderr,"%s\n",err_ptr);
	   free( err_ptr );
	}

	return( rc );
}


/*
* NAME : showerr
*
* FUNCTION : Display error message on console or front panel led's display
*		depending whether the cfgmgr is invoked at IPL time or
*		run time.
*
* PARAMETERS :
*	arg -- character arg to be displayed
*
* RETURNS VALUES:
*
* GLOBAL VARIABLES REFERENCED : none
*/
int showerr(
	int errnum,		/* error number */
	char *arg,		/* character to be displayed */
	int exit_status)	/* exit status code */
{
	/* show the errnum */
	LEDS( (errnum/10*6) + errnum + LEDS_BASE )

	/* write an error message */
	fprintf(stderr, ERRSTR(errnum), arg);

	if (exit_status == EXIT_NOT)
	   return( 0 );
	else
	{  /* assuming fatal error */
	   odm_terminate();

	   /* turn off the leds */
	   setleds( 0xfff );

	   /* bye-bye */
	   exit( exit_status );
	}
}

int noctrlch(char *str)
{
  int i;

	if (str[0] == '\0')
	   return( 0 );

	for (i=0; str[i] != '\0'; i++)
	   if ((!isprint(str[i])) && (str[i] != '\n'))
	      return( 0 );

	return( 1 );
}

/*  */

struct Config_Rules **sort_seq(
	struct Config_Rules rules[],
	int max_rules)
{
  int i,j;
  struct Config_Rules *tmp_me;
  struct Config_Rules **list;

	/* malloc the appropriate space for the ordered list */
	if ((list = (struct Config_Rules **)
	     malloc( max_rules * sizeof(struct Config_Rules *) )) == 0)
	   showerr( ERR_NO_MEMORY, NULL,EXIT_FATAL );

	/* initialize the ptr list */
	for (i=0; i < max_rules; i++)
	   list[i] = &(rules[i]);

	/* prioritize the list based on me.seq */
	/* put zeros at the end */
	for (i=0; i < (max_rules-1); i++)
	  for (j=i+1; j < max_rules; j++)
	  {  if (list[j]->seq == 0)
		continue;
	     if ( (list[j]->seq < list[i]->seq) || (list[i]->seq == 0) )
	     {  /* swap 'em */
		tmp_me = list[i];
		list[i] = list[j];
		list[j] = tmp_me;
	     }
	   }

	return( list );
}


/*
* NAME : privilege
*
* FUNCTION : Duplicating privilege function call from libs to avoid having
*		to have libs around for the cfgmgr call.
*
* RETURNS VALUES:
*
* GLOBAL VARIABLES REFERENCED : none
*/

void privilege(cmd)
int cmd;	/* drop/regain privilege indicator flag */
{
	priv_t	priv;	/* privilege set buffer */

	switch(cmd)
	{
		case PRIV_LAPSE:			 /* lapse privilege */
			getpriv(PRIV_INHERITED, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
			break;

		case PRIV_ACQUIRE:			 /* reacquire privilege */
			getpriv(PRIV_MAXIMUM, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
			break;

		case PRIV_DROP:				 /* drop privilege */
			getpriv(PRIV_INHERITED, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH|PRIV_MAXIMUM, &priv, sizeof(priv_t));
	}
}
