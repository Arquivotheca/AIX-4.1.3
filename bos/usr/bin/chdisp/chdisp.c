static char sccsid[] = "@(#)40	1.21  src/bos/usr/bin/chdisp/chdisp.c, cmdlft, bos411, 9428A410j 2/2/94 11:52:12";

/*
 * COMPONENT_NAME: (CMDLFT) Low Function Terminal Commands
 *
 * FUNCTIONS:  main, parse_args
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*------------
  PURPOSE:
  Change the display used by the active virtual terminal or change
  the default terminal for subsequently-opened virtual terminals.

  FUNCTIONS:
  parse_args(argv,argc) - parse the users flags and parameters
  ------------*/

#include <stdio.h>
#include <locale.h>
#include <strings.h>
#include <fcntl.h>
#include "lftcmds.h"
#include "lftcmds_msg.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>
#include <sys/ioctl.h>
#include <sys/lft_ioctl.h>

#define SUCCESS 0
#define FAIL -1

int dflag, pflag;	   /* current, default parse flags */
char dispname[20];


/*------------
  This command changes the default display being used by lft
  The syntax of the command is:

	chdisp {-d | -p} Device Name

	-d : Changes the default display for this session

	-p : Changes the default display in the database and requires
		 the user to have superuser authority. This change will take
		 effect at the next IPL. 

	Device Name: The logical name of the display.  The lsdisp command
	can be used to get this.  Its contained in the first column of
	the lsdisp output.

  ------------*/


main(argc, argv)
int argc;
char *argv[];
{
	int rc=0;			/* return code */
	struct CuDv  	cudv;		/* customized device object */
	struct CuAt 	*cuatobj;	/* customized device object storage */
	char		crit[256];	/* odm search string */
	int		num_disps;	/* number of displays found */
	lft_query_t	lft_query;	/* struct to hold lft query info */
	
	setlocale(LC_ALL,"");

	/*------------
	  Open the message facilities catalog.
	  Retrieve the program name.
	  ------------*/
	catd=catopen("lftcmds.cat", NL_CAT_LOCALE);
	strcpy(program, MSGSTR(CHDISP, CHDISP_PGM, M_CHDISP_PGM));

	/*
		This command has to be run from a lft
	*/
	if ( ioctl(0, LFT_QUERY_LFT, &lft_query) == -1)
	{
		fprintf(stderr,MSGSTR(COMMON, NOT_LFT, M_NOT_LFT),program);
		exit(FAIL);
	}

	/*------------
	  If no arguments are specified, display the command usage and exit.
	  ------------*/
	if (argc == 1) 
	{
		fprintf(stderr,MSGSTR(CHDISP, CHDISP_USAGE, M_CHDISP_USAGE));
		exit(FAIL);
	}

	/*------------
	  parse the command string
	  ------------*/
	rc = parse_args(argc,argv);
	if (rc != 0)
		exit(FAIL);

	/*------------
	Open the odm data base and verify that the device name is valid
	  ------------*/
	if( (rc = odm_initialize()) < 0 )
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_INIT, M_ODM_INIT), program);
		exit(FAIL);
	}
	sprintf(crit, "name = '%s' AND status = '%d'", dispname, AVAILABLE);
	rc = (int)odm_get_obj(CuDv_CLASS,crit,&cudv,ODM_FIRST);
	if(rc == -1)	/* Failed */
	{
		fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_FAIL),
				program, "odm_get_obj()", "CuDv", crit);
		odm_terminate();
		exit(FAIL);
	}
	if(rc == 0)	  /* Nothing found */
	{
		fprintf(stderr,MSGSTR(CHDISP, CHDISP_NO_DISP, M_CHDISP_NO_DISP),
				dispname);
		odm_terminate();
		exit(FAIL);
	}


	/*------------
	Use the input flags to do what the user wants
	  ------------*/
	if (dflag)
	{
		/* 
	   	 Clear screen and move cursor to home
		*/
		rc = write(1,"\033[1;1H",6); 
		if(rc == -1)
		{
	   		fprintf(stderr,MSGSTR(CHDISP, CHDISP_WRITE,
					M_CHDISP_WRITE));
			odm_terminate();
			exit(FAIL);
		}
		rc = write(1,"\033[2J",4);
		if(rc == -1)
		{
			fprintf(stderr,MSGSTR(CHDISP, CHDISP_WRITE,
					M_CHDISP_WRITE));
			odm_terminate();
			exit(FAIL);
		}
		/*
	   		 Set the default display
		*/
		if ( ioctl(1, LFT_SET_DFLT_DISP, dispname) == -1)
		{
			fprintf(stderr,MSGSTR(CHDISP, CHDISP_IOCTL,
					M_CHDISP_IOCTL));
			odm_terminate();
			exit(FAIL);
		}
		/*
	   		Clear screen at the new display and move cursor to home
		*/
		rc = write(1,"\033[1;1H",6); 
		if(rc == -1)
		{
			fprintf(stderr,MSGSTR(CHDISP, CHDISP_WRITE,
					M_CHDISP_WRITE));
			odm_terminate();
			exit(FAIL);
		}
		rc = write(1,"\033[2J",4);
		if(rc == -1)
		{
			fprintf(stderr,MSGSTR(CHDISP, CHDISP_WRITE,
					M_CHDISP_WRITE));
			odm_terminate();
			exit(FAIL);
		}
	} /* end of dflag processing */

	if (pflag)
	{
		cuatobj = getattr("lft0","default_disp",FALSE,&num_disps);
		if(cuatobj == NULL)
		{
			fprintf(stderr,MSGSTR(COMMON, ODM_NOOBJ, M_ODM_NOOBJ),
					program,
					"getattr()", "PdAt", "default_disp");
			odm_terminate();
			exit(FAIL);
		}
		strcpy(cuatobj->value,dispname);
		if((rc = putattr(cuatobj)) == -1)
		{
			fprintf(stderr,MSGSTR(COMMON, ODM_FAIL, M_ODM_NOOBJ),
					program,
					"putattr()", "CuAt", "default_disp");
			odm_terminate();
			exit(FAIL);
		}
	
	}


	/*------------
	  Close the ODM and the message facilities catalog
	  ------------*/
	odm_terminate();
	catclose(catd);
	system("savebase");	/* flush out the changes to ODM */

	exit(SUCCESS);

}   /* end main */


/*------------
  Parse the user's arguments and parameters
  ------------ */
parse_args(argc,argv)
int argc;
char *argv[];
{
	int i;			/* generic index */
	char *argptr;		/* ptr to arg[i] string */

	for (i=1; i<argc;) 
	{
		argptr = argv[i];
		/*------------
		  If argument is a flag, see which one and save the parameter
		  which follows
		  ------------*/
		if (*argptr == '-')	/* if arg is a flag */
		{
			switch (*(argptr+1)) /* switch on first char of flag */
			{
			case 'd':   /* change default for this session only */
				dflag=TRUE; 
				/*------------
				  parameter in same arg
				  (follows flag with no space)
				  ------------*/
				if (*(argptr+2) != '\0')
				{
					argptr = argptr+2;
					strcpy(dispname, argptr);
				}
				/*------------
				  parameter in next arg
				  ------------*/
				else if( (i+1) < argc && *argv[i+1] != '-' )
				{
					strcpy(dispname, argv[i+1]);
					i++;
				}
				i++;
				break;

			case 'p': /* change default in database for next IPL */
				pflag=TRUE;
				/*------------
				  parameter in same arg
				  (follows flag with no space)
				  ------------*/
				if (*(argptr+2) != '\0') 
				{
					argptr = argptr+2;
					strcpy(dispname, argptr);
				}
				/*------------
				  parameter in next arg
				  ------------*/
				else if( (i+1) < argc && *argv[i+1] != '-' )
				{
					strcpy(dispname, argv[i+1]);
					i++;
				}
				i++;
				break;

			default:
				fprintf(stderr,MSGSTR(COMMON, BADFLAG,
						M_BADFLAG), program, argptr);
				fprintf(stderr,MSGSTR(CHDISP, CHDISP_USAGE,
						M_CHDISP_USAGE));
				return(FAIL);
			}  /* end switch */
		}  /* end 'if a flag' */
		/*------------
		  If argument is not a flag, log an error
		  ------------*/
		else  /* not a valid flag */
		{
			fprintf(stderr,MSGSTR(COMMON, BADFLAG, M_BADFLAG),
					program, argv[i]);
			fprintf(stderr,MSGSTR(CHDISP, CHDISP_USAGE,
					M_CHDISP_USAGE));
			return(FAIL);
		}
	}  /* end for loop */
	return(SUCCESS);
}
