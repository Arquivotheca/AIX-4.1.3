static char sccsid[] = "@(#)70  1.5  src/bos/usr/bin/chusrprof/chusrprof.c, rcs, bos411, 9428A410j 11/21/93 15:22:40";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *		usage
 *		validate
 *		colon_check
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
#include <chusrprof_msg.h> 
#include <locale.h>
#include <nl_types.h>

#define ODMDIR          "/etc/objrepos"
#define PWFILE          "/etc/passwd"
#define FAIL   255
#define SUCCESS 0
#define ROOT 0

/********************************************************/
/* prototypes						*/ 
/********************************************************/ 
void usage(void);
int validate(int uid, char *name);
int main(int argc, char *argv[]);
void colon_check(char *);

extern int    optind;
extern char    *optarg;

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

int	lock_id;    	/* lock id used by odm_lock and odm_unlock  */
int	link_flag = 0;  /* check to see if we get the right parms passed */
int	support_flag = 0; /* check to see if we get the right parms passed */

void usage(void)
{
    fprintf(stderr, catgets(scmc_catd, MS_chusrprof, M_MSG_0, 
	"Usage: chusrprof -m l\n\t\t"
        "-n Name\n\t\t"
        "[ -t Connection_Type ] \n\t\t"
        "[ -d Connection_Description ] \n\t\t"
        "[ -e Emulator ]\n\t\t"
        "[ -h HCON_Sessions ]\n\t\t"
        "-r RecordType { 3|4|5 }\n"
	"Usage: chusrprof -m s\n\t\t"
        "-n Name\n\t\t"
        "[ -i { motif | ascii } ]\n\t\t"
        "[ -s Server_Name ]\n\t\t"
        "[ -t Connection_Type ] \n\t\t"
        "[ -d Connection_Description ] \n\t\t"
        "[ -u UserID ]\n\t\t"
        "[ -c CustomerID ]\n\t\t"   
        "[ -p Password ]\n\t\t"   
        "-r RecordType { 1|2 }\n") );
	catclose(scmc_catd);
        exit(FAIL);
}


int validate(int uid, char *name)
{
	struct passwd *user;
        int	got_a_match = 0;

	if ((user = getpwnam(name)) == NULL)
	{
		fprintf(stderr, catgets(scmc_catd, MS_chusrprof, 
			M_MSG_3, "The user name was not found in "
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

/*
*  function main processes input using getopt
*/
int main(int argc, char *argv[])
{
    char *error_message;
    int c;             	/* used for command line processing 		*/
    int update_rtn;    	/* number returned by changing the odm database	*/
    int return_status; 	/* status return - success or error		*/
    int err_return;    	/* error message number				*/
    int unlock_err;     /* unlock id returned by odm_unlock  		*/
			/* flags to let me know if we have these 	*/
			/* command line options sent to this pgm 	*/	
    int n_flag = 0;	/* n_flag = name	*/ 
    int i_flag = 0; 	/* i_flag = interface 	*/	
    int c_flag = 0;	/* c_flag = custid	*/
    int d_flag = 0;	/* d_flag = conn_desc	*/
    int u_flag = 0;	/* u_flag = userid	*/
    int p_flag = 0;	/* p_flag = passwd	*/
    int r_flag = 0;	/* s_flag = record type	*/
    int s_flag = 0;	/* s_flag = support	*/
    int t_flag = 0;	/* t_flag = conn_type	*/
    int e_flag = 0;	/* e_flag = emulator	*/
    int h_flag = 0;	/* h_flag = hostname	*/
    int m_flag = 0;	/* m_flag = mode 	*/
			/* (ibmlink or support)	*/

    int interface_str_is_ok = 0;
    int mode_is_ok = 0;
    int record_type = 0; 
    int uid;        	/* user id					*/    
    int updatable;    	/* is the user able to update the user id	*/    


    struct userprofiles user_profile;

    struct userprofiles my_usr_prof;

    struct userprofiles *usr_prof_ptr;

    char    name_str[9];          /* arrays to hold strings from the command */
    char    conn_type_str[7];     /* line  -  names are descriptive */
    char    connection_desc[65]; 
    char    emulator_str[7];
    char    hostname_str[256];
    char    server_str[128];
    char    interface_str[6];
    char    type_str[7];
    char    custid_str[9];
    char    userid_str[9];
    char    passwd_str[33];
    char    srchstr[128];
    char    mode_str[2];
    char    sessions_str[20];

    bzero(name_str,9);         /* make sure all arrays contain nulls */
    bzero(conn_type_str,7);
    bzero(connection_desc,65);
    bzero(emulator_str,7);
    bzero(hostname_str,256);
    bzero(server_str,128);
    bzero(interface_str,6);
    bzero(custid_str,9);
    bzero(userid_str,9);
    bzero(passwd_str,33);
    bzero(type_str,7);
    bzero(srchstr,128);
    bzero(mode_str,2);
    bzero(sessions_str,20);

	(void) setlocale(LC_ALL,"");

	scmc_catd = catopen("chusrprof.cat",NL_CAT_LOCALE);

  /* not even close to enough args */ 
	if (argc < 2) 
   	 	usage();

	uid = getuid();

	odm_initialize();

	odm_set_path(ODMDIR);

    /* parse the command line and get options */

    while ((c = getopt(argc,argv,"c:d:e:h:i:m:n:p:r:s:t:u:")) != EOF)
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
                    strncpy(connection_desc,optarg,64);
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
					    if (strcmp(interface_str,"motif") 
							== 0)
							interface_str_is_ok++;
					    if (strcmp(interface_str,"ascii") 
							== 0)
							interface_str_is_ok++;
						if (!interface_str_is_ok)
							usage();
            break;

            case 'm':
		colon_check(optarg);
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
			if (record_type < 1 || record_type > 5)
				usage();
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

	if (!mode_is_ok)
		usage();

	if (support_flag && !n_flag || !r_flag) 
	{
		usage();
	}

	if (link_flag && !n_flag || !r_flag) 
	{
		usage();
	}

	/* validate users ablility 	*/
	/* to update this profile  	*/
	if((updatable = validate(uid,name_str)) != 0) 
	{
       		 fprintf(stderr, catgets(scmc_catd, MS_chusrprof, M_MSG_4,
			"You are not allowed to update this profile.\n") );
			catclose(scmc_catd);
        		exit(FAIL);
	}
	
	if (n_flag && link_flag) 
	{
        	sprintf(srchstr,"UserName = %s and RecordType = %d",
					name_str,record_type);

	/* look up the desired user profile */

		usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
				srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr == -1) 
  		{
                        return_status = odm_err_msg(odmerrno,
                                &error_message);

                        if (return_status < 0)
                                fprintf(stderr, catgets(scmc_catd, 
					MS_chusrprof, M_MSG_2, 
				"Retrieval of error message failed.\n") );
                        else
                                fprintf(stderr,error_message);

			catclose(scmc_catd);
                        exit(FAIL);
		}


		/* copy to the record only if we have an argument */
		/* for each of the following elements - we have a name */

		strcpy(usr_prof_ptr->UserName,name_str);

		if (t_flag) 
		{
			if (conn_type_str[0] != NULL)
				strcpy(usr_prof_ptr->ConnType,
					conn_type_str);
		}

		if (d_flag) 
		{
			strcpy(usr_prof_ptr->ConnDesc,connection_desc);
		}

		if (e_flag) 
		{
			if (emulator_str[0] != NULL)
			strcpy(usr_prof_ptr->Emulator,emulator_str);
		}

		if (h_flag) 
		{
			if (sessions_str[0] != NULL)
			strcpy(usr_prof_ptr->Sessions,sessions_str);
		}

		if (r_flag) 
		{
			if (record_type > 2 || record_type < 6)
				usr_prof_ptr->RecordType = record_type;
			else
				usage();
		}

		if (s_flag) 
		{
			if (server_str[0] != NULL)
			strcpy(usr_prof_ptr->Server,server_str);
		}

                if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) == -1 )
                {
                        return_status = odm_err_msg(odmerrno,
                                &error_message);

                        if (return_status < 0)
                                fprintf(stderr, catgets(scmc_catd, 
					MS_chusrprof, M_MSG_2,
					"Retrieval of error "
					"message failed.\n") );
                        else
                                fprintf(stderr,error_message);

			catclose(scmc_catd);
                        exit(FAIL);
                }
		else 
		{	 /* got a lock so update the user profile */

 		  	update_rtn = odm_change_obj(userprofiles_CLASS,
							usr_prof_ptr); 
			if (update_rtn == -1)
			{
  				return_status = odm_err_msg(odmerrno,
                               				 &error_message);

                        	if (return_status < 0)
                                	fprintf(stderr, catgets(scmc_catd, 
						MS_chusrprof, M_MSG_2,
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
        	sprintf(srchstr,"UserName=%s and RecordType = %d",
				name_str,record_type);

		/* look up the desired user profile */
			usr_prof_ptr = odm_get_obj(userprofiles_CLASS,
					srchstr,&my_usr_prof,TRUE); 

		if (usr_prof_ptr == -1) 
  		{
                        return_status = odm_err_msg(odmerrno,
                                &error_message);

                        if (return_status < 0)
                                fprintf(stderr, catgets(scmc_catd, 
					MS_chusrprof, M_MSG_2, 
					"Retrieval of error "
					"message failed.\n") );
                        else
                                fprintf(stderr,error_message);

                        catclose(scmc_catd);
                        exit(FAIL);
		}

		/* copy to the record only if we have an argument */
		/* for each of the following elements */

		if (d_flag) 
		{
			strcpy(usr_prof_ptr->ConnDesc,connection_desc);
		}
		if (c_flag) 
		{
			if (custid_str[0] != NULL)
			strcpy(usr_prof_ptr->Custid,custid_str);
		}

		if (n_flag) 
		{
			if (name_str[0] != NULL)
			strcpy(usr_prof_ptr->UserName,name_str);
		}

		if (i_flag) 
		{
			if (interface_str[0] != NULL)
			strcpy(usr_prof_ptr->Interface,interface_str);
		}

		if (p_flag) 
		{
			if (passwd_str[0] != NULL)
			strcpy(usr_prof_ptr->Password,passwd_str);
		}

		if (r_flag) 
		{
			if (record_type != 0)
			if (record_type > 3)
				usage();
			usr_prof_ptr->RecordType = record_type;
		}

		if (s_flag) 
		{
			if (server_str[0] != NULL)
			strcpy(usr_prof_ptr->Server,server_str);
		}

		if (t_flag) 
		{
			if (conn_type_str[0] != NULL)
			strcpy(usr_prof_ptr->ConnType,conn_type_str);
		}

		if (u_flag) 
		{
			if (userid_str[0] != NULL)
			strcpy(usr_prof_ptr->Userid,userid_str);
		}

		if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) == -1)
		{
			return_status = odm_err_msg(odmerrno,
					&error_message);

			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd, 
					MS_chusrprof, M_MSG_2,
					"Retrieval of error "
					"message failed.\n") );
			else
				fprintf(stderr,error_message);

                       	catclose(scmc_catd);
			exit(FAIL);
		}

		/* got a lock so update the user profile */
		/* if update error is = -1 then change can't be made */

		update_rtn = odm_change_obj(userprofiles_CLASS,
					usr_prof_ptr); 
		if (update_rtn == -1) 
		{
       	         	return_status = odm_err_msg(odmerrno,
       	                 	&error_message);

       	               if (return_status < 0)
       	               		fprintf(stderr, catgets(scmc_catd, 
					MS_chusrprof, M_MSG_2, 
					"Retrieval of error "
					"message failed.\n") );
			else
       	                 	fprintf(stderr,error_message);

			catclose(scmc_catd);
			exit(FAIL);
		}
		/* release the odm database */
 		odm_unlock(lock_id);
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
		fprintf(stderr, catgets(scmc_catd,MS_chusrprof, M_MSG_5, 
			"Error changing %s;\nCharacter : is invalid.\n"),
			optarg_string);
		exit(FAIL);
	}
}
