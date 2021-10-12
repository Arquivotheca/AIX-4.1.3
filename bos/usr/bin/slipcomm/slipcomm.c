static char sccsid[] = "@(#)74  1.6  src/bos/usr/bin/slipcomm/slipcomm.c, rcs, bos411, 9428A410j 11/21/93 15:23:00";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *		sig_handler
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


#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <slipserver.h>
#include <locale.h>
#include <nl_types.h>
#define WHOAMI_CMD "/usr/bin/whoami"

/*              include file for message texts          */
#include "slipcomm_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

extern int      optind;
extern char     *optarg;

/********************************/
/*      prototypes		*/
/********************************/
int main(int argc, char *argv[]);
void usage(void);
int read_response(void);
int exit_program(int);
int read_pipe(char *, int);
void sig_handler(int);

/********************************/
/*      globals			*/
/********************************/

/* client pipe /tmp/SLIPCLIENT.$$  */
char client_pipe[80];

/* slipserver pipe file descriptor */
int server_fd; 


static int exit_loop = 0;

/********************************/
/*      functions begin		*/
/********************************/

void sig_handler(int sig)
{
	switch (sig)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGHUP:
		case SIGTERM:
		case SIGDANGER:
		case SIGPIPE:
				remove(client_pipe);
				exit(FAIL);
		break;
	}
}

/********************************/
/* usage gets called when the	*/
/* user should be notified of	*/
/* a command line error or of 	*/
/* an omission of some kind	*/
/********************************/
void usage(void)
{
	fprintf(stderr,
	 catgets(scmc_catd, MS_slipcomm, M_MSG_1, 
	"\nUsage:  slipcomm -p Process_id -l { -c | -d } -n SlipName |\n"
	"\tslipcomm -p Process_id -s { -c | -d } -n SlipName |\n"
	"\tslipcomm -p Process_id -k -n SlipName |\n"
	"\tslipcomm -p Process_id -q |\n" 
	"\tslipcomm -p Process_id -a |\n" 
	"\tslipcomm -p Process_id -r -D Dialed_Host -L Local_Host\n") );
	exit_program(FAIL);
}

/********************************/
/* main starts the command line */
/* processing using getopt 	*/
/********************************/
int main(int argc, char *argv[])
{
	/********************************/
	/* initialize and turn off all	*/
	/* the flags and switches 	*/
	/********************************/
	int    c; 		/* used for getopt */
	int    valid  = 0;
	int    a_flag = 0;
	int    c_flag = 0;
	int    D_flag = 0;
	int    d_flag = 0;
	int    n_flag = 0;
	int    k_flag = 0;
	int    L_flag = 0;
	int    l_flag = 0;
	int    p_flag = 0;
	int    q_flag = 0;
	int    r_flag = 0;
	int    s_flag = 0;
	int    timeout = 0;
	int    euid   = 0;

        /* login type IBMLink or support/6000 */
	char    type[2]; 

	/* holds the process id */	
	char    pid[6]; 

	/* command if c,d,k,q,r */
	char    command[2]; 

	/* holds the SLIP name */
	char    slipname[33]; 

	/* the command string to be 	*/
	/* combined with the send str	*/
	/* to be sent to the slipserver */
	char    command_string[USER_COMMAND_LEN];
	
	char    send_str[USER_COMMAND_LEN];
	
	char    mkclient_pipe[MAX_LINE];
	
	char    chmod_pipe[MAX_LINE];
	
	char    profile_type[MAX_LINE];
	
	char    local_host[16];
	
	char    dialed_host[16];
	
	char    whatuser[33];
	
	char    check_profile[MAX_LINE];
	
	struct passwd *userstruct;

	/********************************/
	/* null all arrays using the 	*/
	/* bzero function		*/ 
	/********************************/
	bzero(type,2); 
	bzero(pid,6);
	bzero(command,2);
	bzero(slipname,33);
	bzero(command_string,USER_COMMAND_LEN);
	bzero(send_str,USER_COMMAND_LEN);
	bzero(mkclient_pipe,MAX_LINE);
	bzero(chmod_pipe,MAX_LINE);
	bzero(profile_type,MAX_LINE);
	bzero(local_host,16);
	bzero(dialed_host,16);
	bzero(whatuser,33);
	bzero(check_profile,MAX_LINE);

	(void) setlocale(LC_ALL,"");

	scmc_catd = catopen("slipcomm.cat",NL_CAT_LOCALE);

	if (argc < 2) 
	{
		usage();
	}
	/********************************/
	/* loop through the command 	*/
	/* line arguments and set the	*/
	/* appropriate flags		*/
	/********************************/
	while ((c = getopt(argc,argv,"acdkln:qp:rs?D:L:")) != EOF)
		switch(c) 
		{
		case '?':
			usage();
		break;

		case 'a':
			a_flag++;
			command[0] = ACTIVE;
			valid++;
		break;

		case 'c':
			c_flag++;
			command[0] = CONNECT;
			valid++;
		break;

		case 'd':
			d_flag++;
			command[0] = DISCONNECT;
			valid++;
		break;

		case 'q':
			q_flag++;
			command[0] = QUERY;
			valid++;
		break;

		case 'k':
			if (getuid() == 0) 
			{
				command[0] = KILL;
				valid++;
				k_flag++;
			}
			else 
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_2, 
						"Root privilages are "
					"required to kill a slip.\n") );
		break;

		case 'D':
			D_flag++;
			strncpy(dialed_host,optarg,15);
		break;

		case 'L':
			L_flag++;
			strncpy(local_host,optarg,15);
		break;

		case 'l':
			l_flag++;
			sprintf(profile_type,"%s","IBMLink");
			strcpy(type,"l");
		break;

		case 'n':
			n_flag++;
			strncpy(slipname,optarg,32);
		break;

		case 'p':
			p_flag++;
			strncpy(pid,optarg,5);
		break;

		case 'r':
			r_flag++;
			command[0] = 'r';
			valid++;
		break;

		break;

		case 's':
			s_flag++;
			sprintf(profile_type,"%s","Support/6000");
			strncpy(type,"s",2);
		break;

		default:
			usage();
		}
	/*      end of get command line opts		*/

	/********************************************************************/
	/* we must have a process ID for this program to be successful      */
	/********************************************************************/
	if (!p_flag)
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_3, "You must specify a process id.\n") );
		usage();
	}

	/********************************/
	/* we must have a valid request	*/
	/********************************/
	if (!valid)
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_4,
			 "Requests must be one of [ c | d | k | q | r ].\n") );
		usage();
	}

	/********************************/
	/* check for mixed flags - only	*/
	/* (c,d,q,k,or r) are allowed	*/
	/********************************/
	if (c_flag && d_flag || c_flag && 
		k_flag || c_flag && q_flag)
		usage();

	if(d_flag && k_flag || d_flag 
		&& q_flag)
		usage();

	if (a_flag && k_flag)
		usage();

	if (q_flag && k_flag)
		usage();

	/********************************/
	/* check for mixed flags  l & s	*/
	/********************************/
	if (l_flag && s_flag)
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_5, 
			"The -l and -s parameters are mutually exclusive.\n") );
		usage();
	}

	/********************************/
	/* check for connect and the 	*/
	/* slip name is missing     	*/
	/********************************/
	    if (c_flag && !n_flag)
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_6, 
			"The -n parameter is missing.\n") );
		usage();
	}

	/********************************/
	/* check for reconfig of slip 	*/
	/* without Dialed and Local   	*/
	/********************************/
	if (r_flag && !L_flag) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_7, 
			"The -L parameter is missing.\n") );
		usage();
	}

	/********************************/
	/* name is missing            	*/
	/********************************/
	if (r_flag && !n_flag) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_6, 
			"The -n parameter is missing.\n") );
		usage();
	}

	if (r_flag && !D_flag) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, M_MSG_8, 
			"The -D parameter is missing.\n") );
		usage();
	}

	/********************************/
	/* check to see if the 		*/
	/* slipserver is running before */
	/* proceeding 			*/
	/********************************/
		if ((server_fd = open(SERVER_FIFO,O_WRONLY|O_NDELAY)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_9, "The SLIP Connection Subsystem is "
					"not running.\n" 
				"Please call your system administrator.\n") );
			exit_program(FAIL);
		}
	/********************************/
	/* get the effective user id
	/********************************/
		euid = geteuid();
		userstruct = getpwuid(euid);
	/********************************/
	/* create client fifo for the 	*/
	/* server to talk to		*/
	/********************************/
	/* set a signal handler to catch ctrl c etc. to cleanup pipe */
	signal(SIGINT,sig_handler);
	signal(SIGQUIT,sig_handler);
	signal(SIGINT,sig_handler);
	signal(SIGHUP,sig_handler);
	signal(SIGTERM,sig_handler);
	signal(SIGDANGER,sig_handler);
	signal(SIGPIPE,sig_handler);

	bzero(client_pipe,80);
	sprintf(client_pipe,"%s.%s",CLIENT_FIFO,pid);

	/********************************/
	/* set umask to 0 so client 	*/
	/* will have 622 permissions	*/
	/********************************/
	umask(0);
	unlink(client_pipe);
	if (mkfifo(client_pipe,0622) < 0)
	{
		fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_10, "Unable to create the client FIFO.\n") );
		exit_program(FAIL);
	}

	/********************************/
	/* the root user wants to query	*/
	/* all active slips  		*/
	/********************************/
	if (a_flag) 
	{
	/********************************/
	/* the query is for a specific	*/
	/* user and the user name is	*/
	/* determined by the geteuid	*/
	/********************************/

		if ((server_fd = open(SERVER_FIFO,O_WRONLY)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_9, "The SLIP Connection Subsystem is "
					"not running.\n" 
				"Please call your system administrator.\n") );
			exit_program(FAIL);
		}

		sprintf(command_string,"%s %s %d",command,pid,MINUS_ONE);

		sprintf(send_str,"%02d",strlen(command_string)+2);
		/* + 2 add on for the 2 bytes used to send the length */

		strcat(send_str,command_string);
		/* concatenate the two strings together */

		    write(server_fd,send_str,USER_COMMAND_LEN);

		    exit_program(read_response());

	} /* if a_flag */

	if (q_flag) 
	{
	/********************************/
	/* the query is for a specific	*/
	/* user and the user name is	*/
	/* determined by the geteuid	*/
	/********************************/

		if ((server_fd = open(SERVER_FIFO,O_WRONLY)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_9, "The SLIP Connection Subsystem is "
					"not running.\n" 
				"Please call your system administrator.\n") );
			exit_program(FAIL);
		}

		sprintf(command_string,"%s %s %d",command,pid,euid);

		sprintf(send_str,"%02d",strlen(command_string)+2);
		/* + 2 add on for the 2 bytes used to send the length */

		strcat(send_str,command_string);
		/* concatenate the two strings together */

		    write(server_fd,send_str,USER_COMMAND_LEN);

		    exit_program(read_response());

	} /* if q_flag */

	if (k_flag) 
	{

		if ((server_fd = open(SERVER_FIFO,O_WRONLY)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_9, "The SLIP Connection Subsystem is "
					"not running.\n" 
				"Please call your system administrator.\n") );
			exit_program(FAIL);
		}

		sprintf(command_string,"%s %s %s %d",command,pid,
			slipname,euid);

		sprintf(send_str,"%02d",strlen(command_string)+2); 
		    /* + 2 add on for the 2 bytes used to send the length */

		strcat(send_str,command_string);
		    /* concatenate the two strings together */

		write(server_fd,send_str,USER_COMMAND_LEN);

		exit_program(read_response());

	} /* if k_flag */

	if (r_flag) 
	{
		sprintf(command_string,"%s %s %s %d %s %s",command,pid,
				slipname,euid,local_host,dialed_host);

		server_fd = open(SERVER_FIFO,O_WRONLY);

		    /* + 2 add on for the 2 bytes used to send the length */
		sprintf(send_str,"%02d",strlen(command_string)+2); 

		    /* concatenate the two strings together */
		strcat(send_str,command_string);

		write(server_fd,send_str,USER_COMMAND_LEN);

		exit_program(read_response());

	}

	if (c_flag && n_flag) 
	{
		sprintf(command_string,"%s %s %s %d",command,pid,
		    slipname,euid);
		    server_fd = open(SERVER_FIFO,O_WRONLY);

		    /* + 2 add on for the 2 bytes used to send the length */
		sprintf(send_str,"%02d",strlen(command_string)+2); 

		    /* concatenate the two strings together */
		strcat(send_str,command_string);

		write(server_fd,send_str,USER_COMMAND_LEN);

		exit_program(read_response());

	}
	if (d_flag) 
	{
		sprintf(command_string,"%s %s %s %d",command,pid,
		    slipname,euid);
		    server_fd = open(SERVER_FIFO,O_WRONLY);

		    /* + 2 add on for the 2 bytes used to send the length */
		sprintf(send_str,"%02d",strlen(command_string)+2); 

		    /* concatenate the two strings together */
		strcat(send_str,command_string);

		write(server_fd,send_str,USER_COMMAND_LEN);

		exit_program(read_response());
	}
}

int read_response(void)
{
	int	fd[2];
	int	i = 0;
	int	index = 0;
	int	bytes_read = 0;
	int	bytes_left = 0;
	int	return_code = 0;
	int	cmd_length = 0;
	char	read_buf[MAX_LINE+1];
	char	*p;

	exit_loop = 0;

	while (!exit_loop)
	{
		if ((fd[index] = open(client_pipe,O_RDONLY)) < 0) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_slipcomm, 
			M_MSG_11, "Unable to open the client pipe %s.\n") ,
					client_pipe);
			exit_program(FAIL);
		}
	
	
			bzero(read_buf,MAX_LINE+1);
	
       	         	bytes_read = read(fd[index],read_buf,MAX_LINE);
	
       	         if (bytes_read != 0)
       	         {

			p = read_buf;

			cmd_length = atoi(p);

			if (cmd_length != strlen(p))
       	                {
				fprintf(stderr,"Internal system error, "
				"exiting SLIPCOMM_CMD\n");
		 		exit_program(FAIL); 
       	                }
	
       	                return_code = print_response(read_buf);
	
			bzero(read_buf,MAX_LINE+1);
	
       	         }
       	         else
       	         {
       	                 fd[index ^ 1] = open(client_pipe,O_RDONLY);
       	                 close(fd[index]);
       	                 index = index ^ 1;
       	         }
	}
return(return_code);
}


int print_response(char *read_buf)
{
	char *p;
	int return_code = 0;

	switch (read_buf[2])
		{
		  case 'o':
			p = read_buf;
			p+=3;
			fprintf(stderr,"%s",p);
		  break;

		  case 'r':
			p = read_buf;
			p+=3;
	 		switch (atoi(p)) 
			{
			case 0:
				fprintf(stderr, catgets(scmc_catd,
					 MS_slipcomm, M_MSG_12, 
				"SLIP Link connected successfully!\n") );
				return_code = SUCCESS;
				exit_loop = 1;
			break;

			case 1:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_13, 
					"SLIP Link not connected.\n"
					"The TTY port is in use by "
					"another program.\n") );
				return_code = TTYPORT_FAIL;
				exit_loop = 1;
				
			break;

			case 2:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_14, 
					"SLIP Link not connected.\n"
					"The mkdev command failed.\n") );
				return_code = MKDEVSLIP_FAIL;
				exit_loop = 1;
			break;


			case 3:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_15, 
					"This SLIP Link is "
					"already connected!\n") );
				return_code = FAIL;
				exit_loop = 1;
			break;

			case 4:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_16, 
					"SLIP Link not connected.\n"
					"The ifconfig command failed.\n") );
				return_code = IFCONFIG_FAIL;
				exit_loop = 1;
				
			break;

			case 5:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_17, 
					"SLIP Link not connected properly.\n"
					"One or more route add commands "
								"failed.\n") );
				return_code = ROUTEADD_FAIL;
				exit_loop = 1;
			break;

			case 6:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_18, 
					"SLIP Link not connected.\n"
					"The slattach command failed.\n") );
				return_code = SLATTACH_FAILED; 
				exit_loop = 1;
			break;

			case 7:
			 	fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_19, 
					"SLIP Link not connected.\n"
					"The modem failed to respond.\n") );
				return_code = MODEM_PROBLEM;
				exit_loop = 1;
			break;

			case 8:
				return_code = SUCCESS;
				exit_loop = 1;
			break;

			case 9:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_20, 
					"SLIP Link not connected.\n"
					"You don't have any SLIPS "
						"connected.\n") );
				return_code = ZERO_SLIPS;
				exit_loop = 1;
			break;

			case 10:
				fprintf(stderr, catgets(scmc_catd, 
					MS_slipcomm, M_MSG_21, 
					"RECONFIGURE of the SLIP Failed.\n") );
				return_code = RECONFIG_FAIL;
				exit_loop = 1;
			break;

			default: 
			break;
			}
			/* end of switch readbuf[2] on case r */
		  break;

		  default:
		  break;
	} /* end of switch r and o */

  return(return_code);
}

/************************************************/  
/* exit the program with the return code      	*/  
/************************************************/  
int exit_program(int return_code)
{
	/* server_fd is a global 	*/
	/* available to all functions	*/
	if (server_fd >= 0)
		close(server_fd);

	/* catd (catalog fd) is a global*/
	/* available to all functions	*/
	if (scmc_catd >= 0)
		catclose(scmc_catd);

	unlink(client_pipe);
	exit(return_code);

}
