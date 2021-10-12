static char sccsid[] = "@(#)71  1.8  src/bos/usr/bin/lsusrprof/lsusrprof.c, rcs, bos411, 9428A410j 11/21/93 15:21:46";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *		usage
 *		validate_user
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
#include <pwd.h>
#include <userprofiles.h>
/*              include file for message texts          */
#include <lsusrprof_msg.h> 
#include <locale.h>
#include <nl_types.h>

#define NOT_ALLOWED  2
#define FAIL  255
#define SUCCESS  0
#define ROOT  0
#define ODMDIR  "/etc/objrepos"
#define NO_ODM_LOCK  5

#define NAME_LEN 9   
#define CONN_LEN 64     
#define SERVER_LEN 256
#define EMULATOR_LEN 7
#define HOSTNAME_LEN 256
#define SESSIONS_LEN 16 
#define INTERFACE_LEN 6
#define TYPE_LEN 2
#define SRCHSTR_LEN 32

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/********************************************************/
/* prototypes						*/
/********************************************************/
void usage(void);
int validate_user(int uid, char *name);
int main(int argc, char *argv[]);

/********************************************************/ 
/*	declare external variables for getopt		*/
/********************************************************/
extern int	optind;
extern char	*optarg;

/********************************************************/ 
/* void usage()  returns usage message if errors are	*/
/* encountered on the command line, or if getopt 	*/
/* happens to miss a null 				*/
/********************************************************/ 
void usage(void)
{
	fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, M_MSG_0, 
		"Usage: lsusrprof -a |\n"
		"lsusrprof -s [ -c ] -n Name [ -t { 1 | 2 } ] |\n"
		"lsusrprof -l [ -c ] -n Name [ -t { 3 | 4 | 5 } ]\n") );
		catclose(scmc_catd);
		exit(FAIL);
}

/********************************************************/ 
int main(int argc, char *argv[])
{
	char *error_message;	/* to be used with odm_err_msg function	*/
	int c; 		/* used for command line processing	 	*/
	int return_status;	/* return from odm_err_msg		*/
	int update_err;	/* error message number if change		*/
			/* can't be made to database		 	*/
	int err_return;	/* error message number		 		*/
	               	/* lock id returned by odm  			*/
	               	/* flags to let me know if we have these	*/
	               	/* command line options sent to this pgm	*/
	int uid; 	/* user id of the calling user		 	*/
	int showpass = 0 ; 	/* showpass is set to 1 if uid equals 	*/
			/* the name string on command line		*/
	int lock_id; 	/* lock id returned by odm  		 	*/
	int n_flag = 0;	/* flags to let me know if we have these	*/
	               	/* command line options sent to this pgm	*/

	int support_users = 1;	/* this flag will get switched if	*/
				/* lookup Support/6000 profiles	= NULL	*/
	int ibmlink_users = 1;	/* this flag will get switched if	*/
				/* lookup IBMLink profiles	= NULL	*/
	int support_flag = 0;	/* lookup Support/6000 profiles		*/
	int link_flag = 0;	/* lookup IBMLink profiles		*/
	int all_flag = 0;	/* lookup all profiles			*/
	int t_flag = 0;		/* user profile type flag		*/
	int header_flag = 0;	/* colon separated output if == 1	*/
	int number_type = 0;	/* user profile type from 1 to 5	*/


	/* declare a struct of user_profile */
	struct userprofiles user_profile;

	/* declare my structure of user_profiles */
	struct userprofiles my_usr_prof;

	/* declare a pointer to user_profile */
	struct userprofiles *usr_prof_ptr;

	/* arrays to hold strings from the command */
	/* line  -  names are descriptive */

	char	name_str[NAME_LEN];	
	char	conn_type_str[CONN_LEN]; 	
	char	server_str[SERVER_LEN];
	char	emulator_str[EMULATOR_LEN];
	char	hostname_str[HOSTNAME_LEN];
	char	sessions_str[SESSIONS_LEN];
	char	interface_str[INTERFACE_LEN];
	char	srchstr[SRCHSTR_LEN];

		/* make sure all arrays contain nulls */
	bzero(name_str,NAME_LEN);	
	bzero(conn_type_str,CONN_LEN); 	
	bzero(server_str,SERVER_LEN);
	bzero(emulator_str,EMULATOR_LEN);
	bzero(hostname_str,HOSTNAME_LEN);
	bzero(sessions_str,SESSIONS_LEN);
	bzero(interface_str,INTERFACE_LEN);
	bzero(srchstr,SRCHSTR_LEN);

	(void) setlocale(LC_ALL,"");

	scmc_catd = catopen("lsusrprof.cat",NL_CAT_LOCALE);

	/* if we don't have any args then call usage */
	if (argc < 2)
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, 
		M_MSG_1, "This program needs at least one argument.\n") );
		usage();
	}

	/* setup the odm database manager environment	*/
	odm_initialize();
	odm_set_path(ODMDIR);

	/* parse the command line and get options */
	while ((c = getopt(argc,argv,"acln:st:")) != EOF)
		switch (c) 
		{
			/* set flag if we get the option  	*/
			/* copy the option into the array 	*/	
			/* do this for each case below		*/

			/* -a the all flag */
			case 'a':
				all_flag++;
			break;

			/* user wants a colon separated output	*/
			case 'c':
				header_flag++;
			break;

			/* user wants an IBMLink user profile 	*/
			case 'l':
				link_flag++;
			break;
				
			/* user wants a specific user profile */
			case 'n':
				n_flag++;
				strncpy(name_str,optarg,NAME_LEN-1);
			break;

			/* user wants a Support/6000 user profile */
			case 's':
				support_flag++;
			break;

			case 't':
				t_flag++;
				number_type = atoi(optarg);
				/* check here to see if we have a 	*/
				/* number in the range of 1 to 5	*/
				if (number_type < 1 || number_type > 5)
					usage();
			break;
				
			default:  /* unknown option - give usage */
				usage();
		}

	if (support_flag && link_flag)
		usage();

	if (support_flag && all_flag)
		usage();

	if (link_flag && all_flag)
		usage();

	uid = getuid(); 
	/* validate users ablility to view this profile 	*/
	/* based on the user id (uid)				*/
	if (!all_flag)
		{
		if(validate_user(uid,name_str) != 0) 
		{
       		 	fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, 
				M_MSG_5, "You are not allowed to "
					"view this profile.\n") );
				catclose(scmc_catd);
        			exit(NOT_ALLOWED);
		}
		else
		{
			showpass = 1;
		}
	}
	
	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0)
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, 
		M_MSG_2, "Unable to obtain a lock on the odm database.\n") );
		catclose(scmc_catd);
		exit(NO_ODM_LOCK);
	}


	if (n_flag && support_flag)
	{
		if (name_str[0] == '\0')
		{
			fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, 
			M_MSG_3, "The -n option needs a name.\n") );
			usage();

		}

		if (t_flag)
		{
		/* should not have a request for a Support/6000	*/
		/* profile and type > 2				*/
			if (number_type > 2)
				usage();

		/* we want only one specific profile type */
			sprintf(srchstr,"UserName = %s AND RecordType = %d",
				name_str,number_type);
		}
		else
		/* we want all Support/6000 profile types for this user */
			sprintf(srchstr,"UserName = %s and RecordType < 3",
							name_str);

		/* look up the desired user profile */
		usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
					srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr == NULL) 
		{
			catclose(scmc_catd);
			exit(FAIL);
		}

		if (usr_prof_ptr == -1) 
		{
			return_status = odm_err_msg(odmerrno,
				&error_message);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd,
					MS_lsusrprof,M_MSG_4,
					"Retrieval of error "
					"message failed.\n") );
			else
				fprintf(stderr,error_message);

			catclose(scmc_catd);
			exit(FAIL);
		}

		if (header_flag && showpass) 
		{ 
			printf("#UserName:ConnType:ConnDesc:"
				"Userid:Server:Interface:"
					"RecordType:Custid:Password\n");
		}

		if (header_flag && !showpass) 
		{ 
			printf("#UserName:ConnType:ConnDesc:"
				"Userid:Server:Interface:"
					"RecordType\n");
		}

		if (showpass)
		{
   			printf("%s:%s:%s:%s:%s:%s:%u:%s:%s\n",
				usr_prof_ptr->UserName,
				usr_prof_ptr->ConnType,
				usr_prof_ptr->ConnDesc,
				usr_prof_ptr->Userid,
				usr_prof_ptr->Server,
				usr_prof_ptr->Interface,
				usr_prof_ptr->RecordType,
				usr_prof_ptr->Custid,
				usr_prof_ptr->Password);
		}
		else
		{
			printf("%s:%s:%s:%s:%s:%s:%u\n",
				usr_prof_ptr->UserName,
				usr_prof_ptr->ConnType,
				usr_prof_ptr->ConnDesc,
				usr_prof_ptr->Userid,
				usr_prof_ptr->Server,
				usr_prof_ptr->Interface,
				usr_prof_ptr->RecordType);
		}

 		while((usr_prof_ptr = 
			odm_get_obj(userprofiles_CLASS,
			srchstr,&my_usr_prof,FALSE)) != NULL)  
		{
			if (showpass)
			{
   				printf("%s:%s:%s:%s:%s:%s:%u:%s:%s\n",
					usr_prof_ptr->UserName,
					usr_prof_ptr->ConnType,
					usr_prof_ptr->ConnDesc,
					usr_prof_ptr->Userid,
					usr_prof_ptr->Server,
					usr_prof_ptr->Interface,
					usr_prof_ptr->RecordType,
					usr_prof_ptr->Custid,
					usr_prof_ptr->Password);
			}
			else
			{
				printf("%s:%s:%s:%s:%s:%s:%u\n",
					usr_prof_ptr->UserName,
					usr_prof_ptr->ConnType,
					usr_prof_ptr->ConnDesc,
					usr_prof_ptr->Userid,
					usr_prof_ptr->Server,
					usr_prof_ptr->Interface,
					usr_prof_ptr->RecordType);
			}
		}
	} /* end of support and name */

	if (all_flag) 
	{

	/* in this section of the code we list all 	*/
	/* Support/6000 profiles, but we don't show 	*/
	/* unneeded passwords or customer accounts	*/

	/* we can't use a #field_name: output	*/
	/* because it would not be useful 	*/

	if (header_flag)
			usage();

		/* create the lookup string */
		sprintf(srchstr,"UserName != NULL and RecordType < 3");
		usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
		srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr == NULL) 
		{
			support_users = 0;
		}

		if (usr_prof_ptr == -1) 
		{
			return_status = odm_err_msg(odmerrno,
				&error_message);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd,
					MS_lsusrprof,M_MSG_4,
					"Retrieval of error "
					"message failed.\n") );
			else
				fprintf(stderr,error_message);

			catclose(scmc_catd);
			exit(FAIL);
		}

		if (support_users)
		{
			printf("%s:%s:%s:%s:%s:%s:%u\n",
				usr_prof_ptr->UserName,
				usr_prof_ptr->ConnType,
				usr_prof_ptr->ConnDesc,
				usr_prof_ptr->Userid,
				usr_prof_ptr->Server,
				usr_prof_ptr->Interface,
				usr_prof_ptr->RecordType);

			while((usr_prof_ptr = 
				odm_get_obj(userprofiles_CLASS,
				srchstr,&my_usr_prof,FALSE)) != NULL)  
			{
				printf("%s:%s:%s:%s:%s:%s:%u\n",
					usr_prof_ptr->UserName,
					usr_prof_ptr->ConnType,
					usr_prof_ptr->ConnDesc,
					usr_prof_ptr->Userid,
					usr_prof_ptr->Server,
					usr_prof_ptr->Interface,
					usr_prof_ptr->RecordType);
			}
		}
	} /* end of all flag  (support profiles) */

	/* this section of the code list all IBMLink profiles */
	if (all_flag) 
	{ 
		/* we can't use a colon separted output	*/
		/* because it would not be useful 	*/
		if (header_flag)
			usage();

		/* create the lookup string */
		sprintf(srchstr,"UserName != NULL and RecordType > 2");

		/* get the profile(s)	*/
		usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
				srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr == NULL) 
		{
			ibmlink_users = 0;
		}

		if (usr_prof_ptr == -1) 
		{
			return_status = odm_err_msg(odmerrno,&error_message);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd,
					MS_lsusrprof,M_MSG_4,
					"Retrieval of error "
					"message failed.\n") );
			else
				fprintf(stderr,error_message);

			catclose(scmc_catd);
			exit(FAIL);
		}

		if (ibmlink_users)
		{
			printf("%s:%s:%s:%s:%s:%s:%u\n",
				usr_prof_ptr->UserName,
				usr_prof_ptr->ConnType,
				usr_prof_ptr->ConnDesc,
				usr_prof_ptr->Emulator,
				usr_prof_ptr->Server,
				usr_prof_ptr->Sessions,
				usr_prof_ptr->RecordType);
	
			while((usr_prof_ptr = odm_get_obj
				(userprofiles_CLASS,
				srchstr,&my_usr_prof,FALSE)) != NULL)  
			{
	   			printf("%s:%s:%s:%s:%s:%s:%u\n",
					usr_prof_ptr->UserName,
					usr_prof_ptr->ConnType,
					usr_prof_ptr->ConnDesc,
					usr_prof_ptr->Emulator,
					usr_prof_ptr->Server,
					usr_prof_ptr->Sessions,
					usr_prof_ptr->RecordType);
			}
		}
	} /* end of all flag  (ibmlink profiles) */

	if (!support_users && !ibmlink_users)
	{
		catclose(scmc_catd);
		exit(FAIL);
	}

	if (link_flag && n_flag) 
	{	 
		if (name_str[0] == '\0')
		{
			fprintf(stderr, catgets(scmc_catd, MS_lsusrprof,
			 M_MSG_3, "The -n option needs a name.\n") );
			usage();

		}

		if (t_flag)
		{
			if (number_type < 3)
				usage();

			sprintf(srchstr,"UserName = %s AND RecordType = %d",
				name_str,number_type);
		}
		else
			sprintf(srchstr,"UserName = %s and RecordType > 2",
						name_str);

		usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
				srchstr,&my_usr_prof,TRUE); 
	
		if (usr_prof_ptr == -1) 
		{
			return_status = odm_err_msg(odmerrno,
				&error_message);
			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd,
					MS_lsusrprof,M_MSG_4,
					"Retrieval of error "
					"message failed.\n") );
			else
				fprintf(stderr,error_message);

			catclose(scmc_catd);
			exit(FAIL);
		}

		if (usr_prof_ptr == NULL) 
		{
			catclose(scmc_catd);
			exit(FAIL);
		}

		if (header_flag) 
		{ 
			printf("#UserName:ConnType:ConnDesc:Emulator:"
				"Server:Sessions:RecordType\n");
		}
		printf("%s:%s:%s:%s:%s:%s:%u\n",
			usr_prof_ptr->UserName,
			usr_prof_ptr->ConnType,
			usr_prof_ptr->ConnDesc,
			usr_prof_ptr->Emulator,
			usr_prof_ptr->Server,
			usr_prof_ptr->Sessions,
			usr_prof_ptr->RecordType);

		while((usr_prof_ptr = odm_get_obj
			(userprofiles_CLASS,srchstr,
				&my_usr_prof,FALSE)) != NULL)  
		{
   			printf("%s:%s:%s:%s:%s:%s:%u\n",
				usr_prof_ptr->UserName,
				usr_prof_ptr->ConnType,
				usr_prof_ptr->ConnDesc,
				usr_prof_ptr->Emulator,
				usr_prof_ptr->Server,
				usr_prof_ptr->Sessions,
				usr_prof_ptr->RecordType);
		}
	} /* end of ibmlink and name flag */

	odm_unlock(lock_id);
	catclose(scmc_catd);
	exit(SUCCESS);
}

int validate_user(int uid, char *name)
{
	struct passwd *user;
        int	got_a_match = 0;

	if ((user = getpwnam(name)) == NULL)
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsusrprof, 
			M_MSG_6, "The user name was not found in "
				"the password file.\n") );
		catclose(scmc_catd);
		exit(FAIL);
	}

	if (user->pw_uid == uid)
		got_a_match++;

	if (got_a_match || (uid == ROOT))
                return(0);
        else 
                return(FAIL);
}

