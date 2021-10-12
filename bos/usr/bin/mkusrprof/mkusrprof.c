static char sccsid[] = "@(#)72  1.5  src/bos/usr/bin/mkusrprof/mkusrprof.c, rcs, bos411, 9428A410j 11/21/93 15:22:42";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: if
 *		main
 *		usage
 *		validate
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
#include <mkusrprof_msg.h>
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include <locale.h>
#include <nl_types.h>


#define FAIL       -1
#define SUCCESS     0
#define ROOT     0
#define ODMDIR     "/etc/objrepos"
#define PWFILE     "/etc/passwd"

extern int    optind;
extern char    *optarg;

/* prototypes */
void colon_check(char *);
void usage();
int main(int, char*arg[]);
int validate(int, char *);

int	lock_id;   	    /* lock id used by odm_lock and odm_unlock  */
int	link_flag = 0;      /* check to see if we get the right parms passed */
int	support_flag = 0;   /* check to see if we get the right parms passed */

void usage()
{
	fprintf(stderr, catgets(scmc_catd, MS_mkusrprof, M_MSG_1, 
	"Usage: mkusrprof -m l\n\t\t"
        "-n Name\n\t\t"
        "-t Connection_Type\n\t\t"
        "-d Connection_Description\n\t\t"
        "-e telnet { -s Server_Name } | -e hcon { -h HCON_Sessions } |\n\t\t"
        "-r RecordType|\n"
        "Usage: mkusrprof -m s\n\t\t"
        "-n Name\n\t\t"
        "-i { motif | ascii }\n\t\t"
        "-t Connection_Type\n\t\t"
        "-d Connection_Description\n\t\t"
        "-u UserID\n\t\t"
        "-s Server_Name\n\t\t"
        "-c [ CustomerID ]\n\t\t"
        "-p [ Password ]\n\t\t"
        "-r RecordType\n") );

	catclose(scmc_catd);
        exit(FAIL);
}

int validate(int uid, char *name)
{
		struct passwd *user;
		int     got_a_match = 0;

		if ((user = getpwnam(name)) == NULL)
		{
			fprintf(stderr, catgets(scmc_catd, MS_mkusrprof,M_MSG_2,			 "User name was not found in the password file.\n") );
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


int main(int argc, char *argv[])
{
	char *error_message; /* error message pointer for odm errors	*/
	int c;         	/* used for command line processing 	*/
	int add_rtn;	/* return from adding to the odm database 	*/ 
	int return_status; /* return from retrieving odm error message	*/ 
	int n_flag = 0;	/* flags to let me know if we have these 	*/
	int i_flag = 0;	/* command line options sent to this pgm 	*/
	int r_flag = 0;	/* n_flag = name, i_flag = interface, s_flag	*/
	int s_flag = 0;	/* n_flag = name, i_flag = interface, s_flag	*/
	int t_flag = 0;	/* = support, t_flag = connection type		*/
	int d_flag = 0;	/* d_flag = connection description		*/
	int e_flag = 0;	/* e_flag = emulator, h_flag = hostname		*/
	int h_flag = 0;	/* m_flag = mode (link or support) 		*/
	int m_flag = 0;	/* r_flag = record type 			*/
	int c_flag = 0; /* customer flag  				*/
	int u_flag = 0; /* userid flag    				*/
	int p_flag = 0; /* password flag  				*/
	int z_flag = 0; /* zero field flag				*/
	int record_type;
	int interface_str_is_ok = 0;
	int mode_is_ok = 0;
	int uid;        /*    user id   */    
	int updatable;  /*   is the user able to update the user id   */    

	struct userprofiles user_profile;
	struct userprofiles my_usr_prof;
	struct userprofiles *usr_prof_ptr;

	char    name_str[9];  /* arrays to hold strings from the command */
	char    conn_type_str[7];      /* line  -  names are descriptive */
	char    conn_desc_str[65];
	char    emulator_str[7];
	char    hostname_str[256];
	char    server_str[128];
	char    interface_str[6];
	char    type_str[7];
	char    custid_str[9];
	char    passwd_str[33];
	char    userid_str[9];
	char    srchstr[128];
	char    mode_str[2];
	char    sessions_str[20];

	/* make sure all arrays contain nulls */
	bzero(custid_str,9);
	bzero(passwd_str,33);
	bzero(userid_str,9);
	bzero(name_str,9);         
	bzero(conn_type_str,7);
	bzero(conn_desc_str,65);
	bzero(emulator_str,7);
	bzero(hostname_str,256);
	bzero(server_str,128);
	bzero(interface_str,6);
	bzero(type_str,7);
	bzero(srchstr,128);
	bzero(mode_str,2);
	bzero(sessions_str,20);

	bzero(user_profile,sizeof(struct userprofiles));
	bzero(my_usr_prof,sizeof(struct userprofiles));

	(void) setlocale(LC_ALL,"");
	
	 scmc_catd = catopen("mkusrprof.cat",NL_CAT_LOCALE);

  /* not even close to enough args */ 
	if (argc < 2) 
   	 	usage();

    uid = getuid();

	odm_initialize();

	odm_set_path(ODMDIR);

    /* parse the command line and get options */
    while ((c = getopt(argc,argv,"c:d:e:h:i:m:n:p:r:s:t:u:z")) != EOF)
        switch (c) 
		{
            /* set flag if we get the option  */
            /* copy the option into the array */    
            /* do this for each case below    */

            case 'c':
		colon_check(optarg);
                c_flag++;
                    strncpy(custid_str,optarg,8);
            break;

            case 'd':
		colon_check(optarg);
                d_flag++;
                    strncpy(conn_desc_str,optarg,64);
            break;

            case 'e':
		colon_check(optarg);
                e_flag++;
                    strncpy(emulator_str,optarg,6);
            break;

            case 'h': /* hcon sessions */
		colon_check(optarg);
                h_flag++;
                    strncpy(sessions_str,optarg,19);
            break;

            case 'i':
		colon_check(optarg);
		i_flag++;
		strncpy(interface_str,optarg,5);
		if (strcmp(interface_str,"motif") == 0)
			interface_str_is_ok++;
		if (strcmp(interface_str,"ascii") == 0)
			interface_str_is_ok++;
		if (!interface_str_is_ok) 
		{
			usage();
		}
	   break;

           case 'm':
		colon_check(optarg);
                m_flag++;
		if (optarg[0] == 'l')
		{
			link_flag++;
			mode_is_ok++;
		}

		if (optarg[0] == 's')
		{
			support_flag++;
			mode_is_ok++;
		}
            break;

            case 'n':
		colon_check(optarg);
                n_flag++;
                    strncpy(name_str,optarg,8);
            break;

            case 'p':
		colon_check(optarg);
                p_flag++;
                strncpy(passwd_str,optarg,32);
            break;

            case 'r':
		colon_check(optarg);
                r_flag++;
                    record_type = atoi(optarg);
            break;

            case 's':
		colon_check(optarg);
                s_flag++;
                    strncpy(server_str,optarg,127);
            break;

            case 't':
		colon_check(optarg);
                t_flag++;
                    strncpy(conn_type_str,optarg,6);
            break;

            case 'u':
		colon_check(optarg);
                u_flag++;
                    strncpy(userid_str,optarg,8);
            break;

            default:  /* unknown option - give usage */
                usage();
			break; 
	} 	

	/* validate users ablility 	*/
	/* to update this profile  	*/
	if((updatable = validate(uid,name_str)) != 0) 
	{
        	fprintf(stderr, catgets(scmc_catd, MS_mkusrprof, M_MSG_3, 
			"You are not permitted to create this profile,"
			" check ownership\n") );
		usage();
	}

	if (!mode_is_ok)
	{
		usage();
	}

	if (support_flag && !n_flag) 
	{
		usage();
	}

	if (link_flag && !n_flag) 
	{
		usage();
	}

	if (n_flag && link_flag) 
	{

		if (!t_flag || !e_flag || !r_flag)  
		{
			usage();
		}

			/* copy to the record, we have an argument */
			/* for each of the following elements */

			sprintf(srchstr,"UserName = %s and Emulator = %s"
					" and ConnType = %s",
				name_str,emulator_str,conn_type_str);


			usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
			srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr != NULL) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_mkusrprof,M_MSG_5,			 "This user type profile has already been defined.\n"));
			catclose(scmc_catd);
			exit(FAIL);
		}

			strcpy(user_profile.UserName,name_str);
			strcpy(user_profile.ConnType,conn_type_str);
			strcpy(user_profile.ConnDesc,conn_desc_str);
			strcpy(user_profile.Emulator,emulator_str);
			strcpy(user_profile.Server,server_str);
			user_profile.RecordType = record_type;

			if (h_flag)
				strcpy(user_profile.Sessions,sessions_str);

	 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_mkusrprof,M_MSG_6,			 "Unable to obtain a lock on the odm database.\n") );
			catclose(scmc_catd);
			exit(FAIL);
		}
		else 
		{
			/* got a lock so add the user profile */
			add_rtn = odm_add_obj(userprofiles_CLASS,&user_profile); 
			/* check the return code from the add for failure */
			if (add_rtn == -1)
			{
				return_status = odm_err_msg(odmerrno,
					&error_message);
				if (return_status < 0)
					fprintf(stderr, catgets(scmc_catd,
						MS_mkusrprof,M_MSG_7,
						"Retrieval of error "
						"message failed.\n") );
				else
					fprintf(stderr,error_message);

				catclose(scmc_catd);
				exit(FAIL);
			}
		  	odm_unlock(lock_id); 
		}

	catclose(scmc_catd);
	exit(SUCCESS);
	} /* end of link and name */
	
	if (n_flag && support_flag) 
	{

		if (!t_flag || !s_flag || !i_flag || !r_flag) 
		{
			usage();
		}

			/* copy to the record, we have an argument */
			/* for each of the following elements */

		sprintf(srchstr,"UserName = %s and ConnType = %s"
				" and Interface = %s",
				name_str,conn_type_str,interface_str);

		usr_prof_ptr = odm_get_obj
			(userprofiles_CLASS,srchstr, 
					&my_usr_prof,TRUE); 

		if (usr_prof_ptr != NULL) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_mkusrprof,M_MSG_5,
			"This user type profile has already been defined.\n") );
			catclose(scmc_catd);
			exit(FAIL);
		}
			strcpy(user_profile.UserName,name_str);
			strcpy(user_profile.Userid,userid_str);
			strcpy(user_profile.ConnType,conn_type_str);
			strcpy(user_profile.ConnDesc,conn_desc_str);
			strcpy(user_profile.Server,server_str);
			strcpy(user_profile.Interface,interface_str);
			user_profile.RecordType = record_type;
			strcpy(user_profile.Custid,custid_str);
			strcpy(user_profile.Password,passwd_str);

		if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_mkusrprof,M_MSG_6,			 "Unable to obtain a lock on the odm database.\n") );
			catclose(scmc_catd);
			exit(FAIL);
		}
		else 
		{ 
			/* got a lock so update the user profile */
			add_rtn = odm_add_obj(userprofiles_CLASS,&user_profile); 
			if (add_rtn == -1)
			{
				return_status = odm_err_msg(odmerrno,
					&error_message);
				if (return_status < 0)
					fprintf(stderr, catgets(scmc_catd,
						MS_mkusrprof,M_MSG_7,
						"Retrieval of error "
						"message failed.\n") );
				else
					fprintf(stderr,error_message);

				catclose(scmc_catd);
				exit(FAIL);
			}
			/* release the odm database */
		  	odm_unlock(lock_id); 
		}
	
	catclose(scmc_catd);
	exit(SUCCESS);
	} /* end of support and name */
}

/****************************************/
/* this function checks for an imbedded	*/
/* colon in the optarg or optarg string	*/
/****************************************/
void colon_check(char *optarg_string)
{
	if (strchr(optarg_string,':') != NULL)
	{
		fprintf(stderr, catgets(scmc_catd,MS_mkusrprof, M_MSG_8, 
			"Error adding %s;\nCharacter : is invalid.\n"),
			optarg_string);
		exit(FAIL);
	}
}
