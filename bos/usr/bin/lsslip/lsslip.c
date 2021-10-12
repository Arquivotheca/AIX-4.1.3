static char sccsid[] = "@(#)66  1.3  src/bos/usr/bin/lsslip/lsslip.c, rcs, bos411, 9428A410j 11/21/93 15:22:50";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
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
#include <odmi.h>
#include <slips.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <nl_types.h>
#include <lsslip_msg.h> 

#define ROOT 0
#define FAIL 255
#define SUCCESS 0
#define NAME_LEN 40
#define ODMDIR "/etc/objrepos"

extern int	optind;
extern char	*optarg;
/*              include file for message texts          */
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/* prototypes */
void usage(void);
int main(int argc , char *argv[]);

void usage(void)
{
	fprintf(stderr, catgets(scmc_catd, MS_lsslip, M_MSG_0,
		 "Usage: lsslip -a [ -c ] | -n Name [ -c ]\n") );
	catclose(scmc_catd);
	exit(FAIL);
}

int main(int argc, char *argv[])
{
	int c;
	int uid;
	int lock_id;
	int return_status;
	int a_flag = 0;
	int n_flag = 0;
	int header_flag = 0;
	int show_dial_string = 0;

	char *error_msg;

	struct slips *slip_ptr;
	struct slips myslip;
	struct passwd *usr_grp_info;
	struct group *grp_info;

	char   name_str[NAME_LEN+1];
	char   srchstr[128];

	bzero(name_str,NAME_LEN+1);
	bzero(srchstr,128);

	(void) setlocale(LC_ALL,"");

	 scmc_catd = catopen("lsslip.cat",NL_CAT_LOCALE);
 
	if (argc < 2) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsslip, M_MSG_1, 
			"This program needs at least 1 argument.\n") );
		usage();
	}

        odm_initialize();

	odm_set_path(ODMDIR);

	if ((uid = getuid()) == ROOT) 
	{
		show_dial_string = 1;
	}
	else 
	{
		grp_info = getgrnam("system");
		usr_grp_info = getpwuid(uid);
		if (grp_info->gr_gid == usr_grp_info->pw_gid)
			show_dial_string = 1;
	}

	while ((c = getopt(argc,argv,"acn:")) != EOF)
		switch (c) 
		{
		          /* list all connections */
			case 'a': 
				a_flag++;
			break;

		          /* generates a colon separted header desc */
			case 'c':
				header_flag++;
			break;

		          /* specifies a specific connection name */
			case 'n': 
				n_flag++;
        			strncpy(name_str,optarg,NAME_LEN);
			break;

			default:
				usage();
		}

	if (a_flag && n_flag) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsslip, M_MSG_2, 
				"The -a and -n options are mutually "
				"exclusive.\n") );
		usage();
	}

	if ((header_flag && !a_flag) && (header_flag && !n_flag)) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsslip, M_MSG_3, 
				"The -c option requires a (-a) "
				"or a (-n) component.\n") );
		usage();
	}

	/* open the odm database - blocking mode */
	if ((lock_id = odm_lock(ODMDIR)) < 0)
	{
		fprintf(stderr, catgets(scmc_catd, MS_lsslip,
			M_MSG_4, "Unable to obtain a lock on "
				"the odm database.\n") );

		catclose(scmc_catd);
		exit(FAIL);
	}



	if (a_flag) 
	{
		strcpy(srchstr,"ConnName!=NULL");

		slip_ptr = odm_get_obj(slips_CLASS,srchstr,&myslip,TRUE); 
	
		if (slip_ptr == -1)
		{
			return_status = odm_err_msg(odmerrno, &error_msg);

			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd, 
					MS_lsslip, M_MSG_5,
				 "Retrieval of error message failed.\n") );
			else
				fprintf(stderr,error_msg);

			catclose(scmc_catd);
	     	  	odm_unlock(lock_id);
			exit(FAIL);
		}

		if ( slip_ptr == NULL ) 
		{
		  fprintf(stderr, catgets(scmc_catd, MS_lsslip, M_MSG_6, 
			"\nThere are no SLIP connections defined.\n") );
		  catclose(scmc_catd);
	     	  odm_unlock(lock_id);
		  exit(FAIL);
		}

		if (header_flag) 
		{	
				/* the user wants the		*/ 
				/* colon separated output 	*/
				/* header over the list   	*/
			printf("#ConnName:ConnDesc:"
				"RemoteHost:LocalHost:"
				"DialedHost:NetMask:TTY_Port:"
				"BaudRate:DialString:Timeout\n");
		}

		if(show_dial_string) 
		{
		  printf("%s:%s:%s:%s:%s:%s:%s:%d:%s:%d\n",
			slip_ptr->ConnName,slip_ptr->ConnDesc,
			slip_ptr->RemoteHost,slip_ptr->LocalHost,
			slip_ptr->DialedHost,slip_ptr->NetMask,
			slip_ptr->TTY_Port,slip_ptr->BaudRate,
			slip_ptr->DialString,slip_ptr->Timeout);
		}
		else
		{
		  printf("%s:%s:%s:%s:%s:%s:%s:%d::%d\n",
			slip_ptr->ConnName,slip_ptr->ConnDesc,
			slip_ptr->RemoteHost,slip_ptr->LocalHost,
			slip_ptr->DialedHost,slip_ptr->NetMask,
			slip_ptr->TTY_Port,slip_ptr->BaudRate,
			slip_ptr->Timeout);
		}

		while((slip_ptr=odm_get_obj(slips_CLASS,
			srchstr, &myslip,FALSE)) != NULL) 
		{
			if(show_dial_string) 
			{
		  		printf("%s:%s:%s:%s:%s:%s:%s:%d:%s:%d\n",
					slip_ptr->ConnName,
					slip_ptr->ConnDesc,
					slip_ptr->RemoteHost,
					slip_ptr->LocalHost,
					slip_ptr->DialedHost,
					slip_ptr->NetMask,
					slip_ptr->TTY_Port,
					slip_ptr->BaudRate,
					slip_ptr->DialString,
					slip_ptr->Timeout);
			}
			else
			{
		  		printf("%s:%s:%s:%s:%s:%s:%s:%d::%d\n",
					slip_ptr->ConnName,
					slip_ptr->ConnDesc,
					slip_ptr->RemoteHost,
					slip_ptr->LocalHost,
					slip_ptr->DialedHost,
					slip_ptr->NetMask,
					slip_ptr->TTY_Port,
					slip_ptr->BaudRate,
					slip_ptr->Timeout);
			}
	  	}

		if (slip_ptr == -1)
		{
			return_status = odm_err_msg(odmerrno, 
				&error_msg);

			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd, MS_lsslip,
					 M_MSG_5, "Retrieval of error "
						"message failed.\n") );
			else
				fprintf(stderr,error_msg);

			catclose(scmc_catd);
	     	  	odm_unlock(lock_id);
			exit(FAIL);
		}
		

	catclose(scmc_catd);
	odm_unlock(lock_id);
	exit(SUCCESS);
	} /*  end of if a_flag */

	/* lookup connection name and make sure at 	*/
	/* some point that we only get one record back */

	if (n_flag) 
	{
		sprintf(srchstr,"ConnName='%s'",name_str);

		slip_ptr = odm_get_obj(slips_CLASS,srchstr,&myslip,TRUE); 

		if (slip_ptr == -1)
		{
			return_status = odm_err_msg(odmerrno, 
				&error_msg);

			if (return_status < 0)
				fprintf(stderr, catgets(scmc_catd, MS_lsslip, 
					M_MSG_5, "Retrieval of error "
						"message failed.\n") );
			else
				fprintf(stderr,error_msg);

			catclose(scmc_catd);
			odm_unlock(lock_id);
			exit(FAIL);
		}

		if (slip_ptr == NULL) 
		{
		    fprintf(stderr, catgets(scmc_catd, MS_lsslip, 
				M_MSG_7, "SLIP connection not found.\n") );
			catclose(scmc_catd);
			odm_unlock(lock_id);
		   	exit(FAIL);
		}

		if (header_flag)  
			printf("#ConnName:ConnDesc:RemoteHost:LocalHost:"
			"DialedHost:NetMask:TTY_Port:BaudRate:"
				"DialString:Timeout\n");

		if(show_dial_string) 
		{
		  	printf("%s:%s:%s:%s:%s:%s:%s:%d:%s:%d\n",
				slip_ptr->ConnName,slip_ptr->ConnDesc,
				slip_ptr->RemoteHost,slip_ptr->LocalHost,
				slip_ptr->DialedHost,slip_ptr->NetMask,
				slip_ptr->TTY_Port,slip_ptr->BaudRate,
				slip_ptr->DialString,slip_ptr->Timeout);
		}
		else
		{
		  	printf("%s:%s:%s:%s:%s:%s:%s:%d::%d\n",
				slip_ptr->ConnName,slip_ptr->ConnDesc,
				slip_ptr->RemoteHost,slip_ptr->LocalHost,
				slip_ptr->DialedHost,slip_ptr->NetMask,
				slip_ptr->TTY_Port,slip_ptr->BaudRate,
				slip_ptr->Timeout);
		}
	   catclose(scmc_catd);
	   exit(SUCCESS);
	   odm_unlock(lock_id);
	} /* end if n_flag */
	
}

