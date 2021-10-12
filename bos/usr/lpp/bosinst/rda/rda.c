static char sccsid[] = "@(#)421.6 src/bos/usr/lpp/bosinst/rda/rda.c, bosinst, bos411, 9428A410j 93/01/14 13:12:35";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*********************************************************************
 * NAME:	rda.c                                                *
 *                                                                   *
 * FUNCTION: 	extracts information about old configuration,        *
 *              and creates a shell script containing define, config *
 *              and change methods to make the current configuration *
 *              as close as possible to the old one.                 *
 *********************************************************************/

/* 
 **************************************************************************
 * USAGE :
    rda [-a attr_file_name][-c prefix][-d dvc_file_name][-p object path] \
 	[-s scriptname ][-eRrx]
 *    where
 *      -a : specify name of output attribute stanza file;
 *           default is "/etc/objrepos/old_ca_stanzas".
 *      -c : prefix to search for in the old CuDv;
 *           default is "'*'".
 *      -d : specify name of output device stanza file; 
 *           default is "/etc/objrepos/old_cd_stanzas".
 *      -p : specify directory pathname containing the old object classes;
 *           default is "/etc/objrepos".
 *      -e : only extract from the old database and do not recover device or
 *           configuration information and do not execute the recovery script.
 *      -R : Relax the criteria for determining which device to configure.  If
 *	     this flag is set, the location code will not be checked.
 *      -r : only recover device and configuration information from the stanza
 *           files.  Do not extract information from the old database and
 *           do not execute the recovery script.
 *      -s : name of shell script file to use for preserving data.
 *      -x : Only execute the script containing recovery config methods.
 **************************************************************************
 */


/* header files needed for compilation */
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include <stdio.h>
#include <sys/types.h>
#include <cf.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
#include "dback.h"

/* 
 *-------------------------------------------------------
 * The following strings are the search string formats 
 * used to query the databases.
 *-------------------------------------------------------
 */
char	cuat_sstr[] = "name=%s and generic like *U*" ;
char    cudv_sstr[] = "name like %s" ;

/* 
 *-------------------------------------------------------
 * The following variables are for the two stanza files
 * created by this program (the device file and the 
 * attribute file).  Default file names are given
 * which can be overridden from the cmd line.
 * 
 * Files are opened with the specified type.
 *-------------------------------------------------------
 */
FILE	*dv_file ;			/* File ptr for dvc stanza file	*/
FILE    *at_file ;			/* file ptr for attr stanza file*/
char    def_dv_fname[] = "./CuDv.sav" ;
char    def_at_fname[] = "./CuAt.sav" ;

/* 
 *-------------------------------------------------------
 * The following strings are the defaults for the 
 * database paths.  The old path can be overridden from
 * the command line.  The new path is taken from the
 * ODMDIR environment variable if it is set.
 *-------------------------------------------------------
 */
char	def_obj_path[] = "/etc/objrepos" ;	/* default objrepos path*/

/*
 *-------------------------------------------------------
 * Flags to determine what actions to take.
 *-------------------------------------------------------
 */
int	default_flag = 1;	/* take the default path */
int	ext_flag = 0;		/* Only do the extract step. */
int	rec_flag = 0;		/* Only do the create shell script step. */
int	rlx_flag = 0;		/* Do not check location code. */
int	xec_flag = 0;		/* Only do the execute shell script step. */

/* main function code */
main(
int     argc,
char    *argv[],
char    *envp[])

{

    int     exit_code ;
    int     c ;

    char    *old_obj_path ;
    char    *new_obj_path ;
    char    *scriptname = NULL;		/* shell script with config commands */
    char    *at_fname ;			/* ptr to output attr file name */
    char    *dv_fname ;			/* ptr to output dvc file name  */
    char    criteria[256] ;



/* BEGIN main */

    exit_code = E_OK ;
    at_fname = def_at_fname ;
    dv_fname = def_dv_fname ;
    old_obj_path = def_obj_path ;
    sprintf(criteria, cudv_sstr, "'*'") ;

    DEBUG_0("rda: going into getop loop\n")
    while ((c = getopt(argc,argv,"a:d:p:c:s:erRx")) != EOF)
    {  
	switch (c)
	{   
	  case 'a' : /* attribute output filename */
	    DEBUG_1("rda: setting attr fname to %s\n", optarg) ;
	    at_fname = optarg ;
	    break;
	  case 'd' :
	    DEBUG_1("rda: setting dvc fname to %s\n", optarg) ;
	    dv_fname = optarg ;
	    break ;
	  case 'p' :
	    DEBUG_1("rda: setting old obj path to %s\n", optarg) ;
	    old_obj_path = optarg ;
	    break ;
	  case 'c' :
	    DEBUG_1("rda: using input prefix of %s\n", optarg) ;
	    sprintf(criteria, cudv_sstr, optarg) ;
	    break ;
	  case 's' :
	    DEBUG_1("rda: using script name of %s\n", optarg) ;
	    scriptname = optarg ;
	    break ;
	  case 'e' :
	    DEBUG_0("rda: setting extract flag\n") ;
	    ext_flag = 1;
	    default_flag = 0;
	    break ;
	  case 'R' :
	    DEBUG_0("rda: setting relax flag\n") ;
	    rlx_flag = 1;
	    break ;
	  case 'r' :
	    DEBUG_0("rda: setting recover flag\n") ;
	    rec_flag = 1;
	    default_flag = 0;
	    break ;
	  case 'x' :
	    DEBUG_0("rda: setting execute script flag\n") ;
	    xec_flag = 1;
	    default_flag = 0;
	    break ;
	  default  : /* syntax error */
	    usage (argv[0]);
	    DEBUG_1("rda: invalid arg => %c\n", optopt) ;
	    exit_code = E_ARGS ;
	    break;
	} /* END switch (ch) */
    } /* END while getopt */

    if (default_flag)
    {
	rec_flag = xec_flag = 1;
    }

    if ((new_obj_path = (char *)getenv("ODMDIR")) == (char *)NULL)
    {
	new_obj_path = def_obj_path ;
    }
    if (exit_code == E_OK)
    {

	/* start up odm */
	if (odm_initialize() == -1) 
	{
	    /* initialization failed */
	    DEBUG_0("rda: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	if ( ext_flag && exit_code == E_OK )
		{
		exit_code = extract(old_obj_path, criteria, dv_fname, at_fname);
		}
	if ( rec_flag && exit_code == E_OK )
		{
		exit_code = dback(dv_fname, at_fname,
				    scriptname, rlx_flag );
		}
	if ( xec_flag && exit_code == E_OK )
		{
		exit_code = system(scriptname);
		}
    }

    odm_terminate();
    exit(exit_code);
} /* END main */

usage (char *cmdname)

{
	printf( "usage: %s\t-[e|r|x][-R][-a file][-d file][-p path][-s file][-c prefix]\n\
Where\n\
\t[-a file]\tname of file to contain output from old CuAt \n\
\t[-c prefix]\tprefix to search for in old CuDv \n\
\t[-d file]\tname of file to contain output from old CuDv \n\
\t[-e]\t\tonly extract data from old database \n\
\t[-p path]\tpath of old database \n\
\t[-R]\t\tdo not check the location code\n\
\t[-r]\t\tonly create config methods shell script \n\
\t[-s file ]\tname of script to contain config methods \n\
\t[-x]\t\tonly execute config methods shell script\n",cmdname);
}
