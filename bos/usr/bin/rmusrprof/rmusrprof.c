static char sccsid[] = "@(#)73  1.3  src/bos/usr/bin/rmusrprof/rmusrprof.c, rcs, bos411, 9428A410j 11/21/93 15:22:45";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: if
 *		main
 *		process_names
 *		remove_user
 *		usage
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

				
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <userprofiles.h>
/*              include file for message texts          */
#include <rmusrprof_msg.h> 
#include <locale.h>
#include <nl_types.h>


#define ODMDIR          "/etc/objrepos"
#define PWFILE            "/etc/passwd"
#define FAIL 255
#define SUCCESS  0

/* globals */
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern int    optind;
extern char    *optarg;
int lock_id;	/* lock id used by odm_lock and odm_unlock       */
int link_flag;  /* check to see if we get the right parms passed */
int support_flag;/* we must have a support or link flag or a -a  */
int n_flag;    	/* flag to us know if we have a name	         */
int s_flag;    	/* flag to us know if we have a service name	 */
int t_flag;    	/* flag to us know if we have a connection type	 */



/**************************************/
/* function prototypes                */
/**************************************/
int main(int argc, char *argv[]);
int process_name(char *, char *);
int remove_user(char *, char *);
void usage(void);

void usage(void)
{
    fprintf(stderr, catgets(scmc_catd, MS_rmusrprof, M_MSG_1,
	"Usage: rmusrprof -m l -t { TCP/IP | SLIP | SNA } -n Name |\n"
           "       rmusrprof -m s -t { TCP/IP | SLIP } -n Name |\n"
	   "       rmusrprof -a -m { l | s }\n") );
	catclose(scmc_catd);
        exit(FAIL);
}


int main(int argc, char *argv[])
{
    int		 c;	/* used for command line processing 		*/
    int 	return_status;	/* status of retrieving error message	*/
    char 	*error_msg;
    int		 return_code;	/* return_code of odm_rm_obj   		*/
            		/* -1 if changes can't be made to database	*/ 
			/* flags to let me know if we have these 	*/
   		        /* command line options sent to this pgm 	*/
                   	/* m_flag = mode (link or supp)			*/
    int a_flag = 0;
    int m_flag = 0;
    int mode_is_ok = 0;
    int chars_to_cpy;

    struct userprofiles *rm_usr_prof_ptr;

    char    mode_str[1];    
    char    names_str[1024];   	/* arrays to hold strings from the command */
    char    user_to_remove[128]; /* line  -  names are descriptive */
    char    type_str[128];

    bzero(names_str,1024);     /* make sure all arrays contain nulls */
    bzero(user_to_remove,128);
    bzero(type_str,128);


	(void) setlocale(LC_ALL,"");

	 scmc_catd = catopen("rmusrprof.cat",NL_CAT_LOCALE);

  /* not even close to enough args */ 
	if (argc < 2) 
   	 	usage();

	odm_initialize();

	odm_set_path(ODMDIR);

	/* set all flags to 0 */
	link_flag = 0;
	support_flag = 0;
	n_flag = 0;  
	s_flag = 0; 
	t_flag = 0; 

    /* parse the command line and get options */
    while ((c = getopt(argc,argv,"am:n:t:")) != EOF)
        switch (c) 
	{

            /* set flag if we get the option  */
            /* copy the option into the array */    
            /* do this for each case below    */

            case 'a':
                a_flag++;

            case 'm':
                m_flag++;
                    strncpy(mode_str,optarg,1);

			if (strcmp(mode_str,"l") == 0)
			 mode_is_ok++;

			if (strcmp(mode_str,"s") == 0)
			 mode_is_ok++;

			if(*mode_str == 's')
				support_flag++;

			if(*mode_str == 'l')
				link_flag++;
            break;

            case 'n':
                n_flag++;
                    strncpy(names_str,optarg,strlen(optarg));
            break;

            break;

            case 't':
                t_flag++;
                    strncpy(type_str,optarg,strlen(optarg));
            break;

            default:  /* unknown option - give usage */
                usage();
        }
	
	if (!mode_is_ok)
		usage();

	if ((support_flag && !n_flag) && (support_flag && !a_flag))
			usage();

	if ((link_flag && !n_flag) && (link_flag && !a_flag))
		usage();

	if ((n_flag && support_flag) || (n_flag && link_flag))
		process_names(names_str,type_str);


	if (a_flag && link_flag) 
	{
 		if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_rmusrprof,M_MSG_2,
			"Unable to obtain a lock on odm the database.\n") );
			catclose(scmc_catd);
			exit(FAIL);
		}
		else 
		{
		 /* got a lock so remove all the ibmlink user profiles */
			return_code = odm_rm_obj(userprofiles_CLASS,
				"RecordType > 2"); 
			if (return_code == -1)
			{
				return_status=odm_err_msg(odmerrno,&error_msg);
				if (return_status < 0)
					fprintf(stderr, catgets(scmc_catd,
					MS_rmusrprof, M_MSG_3,
					"Retrieval of error message "
						"failed.\n"));
				else
					fprintf(stderr,error_msg);
			}

		/* release the odm database */
		odm_unlock(lock_id);
		}
	} /* end of link and all */


	if (a_flag && support_flag) 
		{
 		if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0) {
			fprintf(stderr, catgets(scmc_catd, MS_rmusrprof,M_MSG_2,
			"Unable to obtain a lock on odm the database.\n") );
				catclose(scmc_catd);
				exit(FAIL);
		}
		else 
		{
		 	/* got a lock so remove all the user profiles */
			return_code = odm_rm_obj(userprofiles_CLASS,
				"RecordType < 3"); 
			if (return_code == -1)
			{
				return_status=odm_err_msg(odmerrno,&error_msg);
				if (return_status < 0)
					fprintf(stderr, catgets(scmc_catd,
						MS_rmusrprof, M_MSG_3,
					"Retrieval of error message failed.\n"));
				else
					fprintf(stderr,error_msg);
			}
		/* release the odm database */
		odm_unlock(lock_id);
		}
	} /* end of support and all */
		
	if (link_flag && n_flag)
		process_names(names_str,type_str);
	
	if (support_flag && n_flag)
		process_names(names_str,type_str);
	
	catclose(scmc_catd);
	exit(SUCCESS);
}
	
process_names(char *namestring,char *type_str)
{
	char usernames[1024];
	char *p, *q;
	char	name[10];
	char	type[10];
	memset(name,0,10);
	memset(usernames,0,1024);

	strcpy(usernames,namestring);
	strcpy(type,type_str);

	if((p = strtok(usernames," ,")) == NULL) 
	{
		strncpy(name,p,9);
		remove_user(name,type);
		return(0);
	}
	else 
	{
		strncpy(name,p,9);
		remove_user(name,type);
	}
	while ((p = strtok(NULL,", ")) != NULL) 
	{
		strncpy(name,p,9);
		remove_user(name,type);
	}
	return(0);
}


remove_user(char *name,char *type)
{
	int	return_code;
    	int 	return_status;	/* status of retrieving error message	*/
    	char 	*error_msg;
	char 	remove_criteria[128];

	bzero(remove_criteria,128);

	sprintf(remove_criteria,"UserName = %s and ConnType = %s",name,type);

 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_rmusrprof, M_MSG_2,
		"Unable to obtain a lock on odm the database.\n") );
			catclose(scmc_catd);
			exit(FAIL);
	}

	if (link_flag) 
	{
	/* look up the desired ibmlink user profile and remove  */
		strcat(remove_criteria," and RecordType > 2");
		return_code = odm_rm_obj(userprofiles_CLASS,remove_criteria); 
		if (return_code == -1)
		{
			return_status = odm_err_msg(odmerrno, &error_msg);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd, MS_rmusrprof,
				M_MSG_3,"Retrieval of error message "
					"failed.\n"));
			else
				fprintf(stderr,error_msg);
		}

		/* release the odm database */
		odm_unlock(lock_id);
	}

	if (support_flag) 
	{
	/* look up the desired support user profile and remove  */
		strcat(remove_criteria," and RecordType < 3");
		return_code = odm_rm_obj(userprofiles_CLASS,remove_criteria); 
		if (return_code == -1)
		{
			return_status = odm_err_msg(odmerrno, &error_msg);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd,MS_rmusrprof,
				M_MSG_3,"Retrieval of error message "
					"failed.\n"));
			else
				fprintf(stderr,error_msg);
		}

		/* release the odm database */
	  	odm_unlock(lock_id);
	}
return(return_code);
}
