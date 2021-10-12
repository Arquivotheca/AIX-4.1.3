static char sccsid[] = "@(#)75  1.10  src/bos/usr/bin/slipserver/slipserver.c, rcs, bos411, 9428A410j 7/1/94 18:22:27";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: add_client
 *		add_line_devices
 *		add_link
 *		add_rts_cts
 *		ck_devices_file
 *		cleanup
 *		config_addrs
 *		config_slip_down
 *              connection_timeout
 *		config_slip_up
 *		connect_slip
 *		del_rts_cts
 *		delete_client
 *              demon_init
 *		disconnect_slip
 *		find_link
 *		kill_one_or_all_slips
 *		kill_slip
 *		log_error
 *		lookup_user
 *		main
 *		mkdev_slip
 *		mkdevices
 *		process_request
 *		query_slips
 *		read_pipe
 *		remove_line
 *		rmdev_slip
 *		route_cmd
 *		sig_handler
 *		slattach_timer
 *		slip_attach
 *		terminate_server
 *		write_client
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

/************************************************************************/
/* Major revisions were made to this code in 4.1 			*/
/* Any lines of code that used ioctl calls to check for slip line	*/
/* dicipline were removed since in 4.1 streams are used and there are	*/
/* functions in /usr/sbin to check for slip. ex. /usr/sbin/strconf	*/ 
/************************************************************************/

/*  to get debug output uncomment the next few lines  */
/*
*   #define DEBUG
*/

#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <signal.h>
#include <termio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/erec.h>
#include <rcs.err.h>
#include <slipserver.h>
/*              include file for message texts          */
#include "slipserver_msg.h" 
#include <locale.h>
#include <nl_types.h>

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/************************************************************************/
/* prototype declarations	                                        */
/************************************************************************/
struct LINK *find_link(char *);
int demon_init(void);
void cleanup(char);
void log_error(int error);
void kill_one_or_all_slips(char *);
void kill_slip(char *);
int config_addrs(char *buf);
int ck_devices_file(char *, char *);
int add_line_devices(char *, char *);
int slip_attach(char *, char *, char *);
void slattach_timer(char *);
int route_cmd(char *, char *, char *);
int config_slip_down(char *);
int config_slip_up(char *, char *, char *, char *);
int del_rts_cts(char *, struct termios *);
int add_rts_cts(char *);
int mkdevices(char *, char *);
int mkdev_slip(char *, char *);
int rmdev_slip(char *);
int terminate_server(int);
void sig_handler(int n);
int delete_client(char *, int);
int add_link(struct LINK *);
int main(void);
void process_request(char *);
int connect_slip(char *);
int query_slips();
struct LINK *lookup_user(char *, int);
void write_client(char, char *);
void add_client(char *, struct CLIENT *);

/************************************************************************/
/* global declarations - available to all functions			*/
/************************************************************************/
struct LINK *global_link = NULL;/* global link struct to hold 	*/
				/* my active slip links 	*/

char	slattach_port[MAX_LINE+1];
				/* the current /dev/ttyx port	*/

int clean = 0;			/* flag lets me know if the 	*/
				/* the client finished  the	*/
				/* connection successfully	*/
				/* through all steps		*/

int     current_userid;         /* current user processing req  */

char    current_slip[21];       /* current slip in this request */

char    current_slip_number[6]; /* current slip# of this req	*/

static int tty_connected = 0;	/* flag lets me know if the 	*/
				/* modem connect was successful	*/

char	client_pipe_path[MAX_LINE+1];
				/* the current client fifo 	*/
				/* the demon writes to - a 	*/
				/* global so any function can 	*/
				/* access it 			*/

int client_pipe_fd = -1; 	/* client pipe file descriptor  */
				/* (current user) - this is 	*/
				/* where responses are sent     */

int G_timeout = 85;		/* slattach timeout in seconds	*/

int got_sig_pipe = 0;		/* flag to let us know if it's	*/
				/* ok to write to the client	*/

struct termios FDbufsave;	/* struct to hold the tty parms	*/

int 	slattach_fork_rtn;      /* pid from child process       */

extern int errno;

void sig_handler(int n)
{
	switch(n)
	{
	case SIGTERM:
		terminate_server(SIGTERM);
		break;

	case SIGDANGER:
		terminate_server(SIGDANGER);
		break;

	case SIGUSR1:
		exit(SUCCESS);
		break;

	case SIGUSR2:
		write_client(RETURN_CODE,SLATTACH_FAIL);
		exit(FAIL);
		break;

	case SIGQUIT:
		terminate_server(SIGQUIT);
		break;

	case SIGINT:
		terminate_server(SIGINT);
		break;

	case SIGHUP:
		break;

	case SIGPIPE:

#ifdef DEBUG
printf("got a sigpipe, setting cleanup in cleanup('u')?\n");
#endif

		got_sig_pipe = 1;
		cleanup(CURRENT_USER);
		break;

	default:
		cleanup(ALL);
		terminate_server(UNKNOWN_ERROR);
		break;
	}
}

int demon_init(void)
{
	int server_pid;

	if ((server_pid = fork()) < 0)
		return(FAIL);
	else if (server_pid != 0)
		exit(SUCCESS); /* parent exits */

	setsid();
	
	chdir("/");

	/************************************************/
	/* making sure all mask bits are set to 0 	*/
	/* before starting prevents any default      	*/
	/* mask settings from interferring with the 	*/
	/* FIFO creation to our desired mode	       	*/
	/************************************************/
	umask(0);

	return(SUCCESS);
}

int main(void)
{
	int     i;
	int     c;
	int     fd_index;		/* used as fd[fd_index] for slipserver */
	int     fd[2];			/* file descriptors for slipserver  */
	int 	server_pid;		/* pid file */
	int 	server_pid_fd;		/* pid file file descriptor */
	int     cmd_len;
	int     bytes_read;
	char	*user_command;
	char    buf[USER_COMMAND_LEN];
	struct  flock   lock;
	FILE    *fd_out;
	/************************************************/
	/* set all the signals so the signal handler 	*/
	/* can take care of the signals   */
	/* we want to handle (our way) as opposed to	*/
	/* letting the defaults occur  */
	/************************************************/
	sigset(SIGQUIT,sig_handler);
	sigset(SIGINT,sig_handler);
	sigset(SIGHUP,sig_handler);
	sigset(SIGTERM,sig_handler);
	sigset(SIGDANGER,sig_handler);
	sigset(SIGPIPE,sig_handler);
	sigset(SIGUSR1,sig_handler);
	sigset(SIGUSR2,sig_handler);

	(void) setlocale(LC_ALL,""); /* nls stuff */

	scmc_catd = catopen("slipserver.cat",NL_CAT_LOCALE);

	/************************************************/
	/* start up the demon                       	*/
	/************************************************/
	if (demon_init() != 0)
	{
		fprintf(stderr,catgets(scmc_catd, MS_slipserver, M_MSG_1,
			"Unable to create a child process\n"));
			exit(FAIL);
	}

	/************************************************/
	/* be sure the slipserver in not running 	*/
	/************************************************/
	if ((server_pid_fd = open(SERVER_PID_FILE, O_WRONLY | O_CREAT, S_IWUSR))
			< 0)
		log_error(LOCKFILE_CREAT_ERR);

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = 0;
	lock.l_len = 0;

	fcntl(server_pid_fd, F_SETLK, &lock);

	if (errno == EACCES || errno == EAGAIN)
	{
		fprintf(stderr,catgets(scmc_catd, MS_slipserver, M_MSG_2,
			"Multiple instances of "
			"the SLIP Connection Subsystem are not "
			"supported.\n"));
		exit(FAIL);
	}

	/* close stdin, stdout, & stderr */
	close(0); 
	close(1);
	close(2);

	/************************************************/
	/* delete the fifo if it exists, unconditionally*/
	/************************************************/
	unlink(SERVER_FIFO);

	/************************************************/
	/* create the fifo it should not exist, and 	*/
	/* since this is a background process we will 	*/
	/* have to send any error messages to an error 	*/
	/* file, create the file containing the server	*/
	/* pid for acces by slipcomm 			*/
	/************************************************/
	if (mkfifo(SERVER_FIFO,0622) != 0) 
	{
		log_error(MKFIFO_SERVER_ERR);
		exit(FAIL);
	}

	/************************************************/
	/* open the well known fifo read only 		*/
	/* blocking until we get a request, enter an	*/
	/* infinite loop to read from the pipe, if we	*/
	/* get a 0, then reopen the pipe again to 	*/
	/* block until the kernel signals us. Error to	*/
	/* errlog if we can't open the fifo, 		*/
	/* set initial fd[fd_index] to 0		*/
	/************************************************/
	fd_index = 0;
	if ((fd[fd_index] = open(SERVER_FIFO,O_RDONLY)) < 0) 
	{
		log_error(MKFIFO_SERVER_ERR);
		exit(FAIL);
	}

	while (1) 
	{
	/************************************************/
	/* clear the buffers ( stuff them with nulls )	*/
	/************************************************/
		bzero(buf,USER_COMMAND_LEN);

		bytes_read = read(fd[fd_index],buf,USER_COMMAND_LEN);

		if (bytes_read > 0) 
		{
			user_command = buf;
	/************************************************/
	/* use atoi and check the length of the buffer	*/
	/* send an error to the system log and shut 	*/
	/* down the server if the length does not match */
	/* the number of bytes passed to us by slipcomm	*/
	/************************************************/
			cmd_len = atoi(user_command);
			if (cmd_len != strlen(user_command)) 
			{
#ifdef DEBUG
printf("bad length, user_command = %s, len = %d\n",user_command,cmd_len);
#endif
				terminate_server(BADLEN);
			}
#ifdef DEBUG
printf("going to process request = %s\n",user_command);
#endif
			process_request(user_command);
		}
		else 
		{
			fd[fd_index ^ 1] = open(SERVER_FIFO,O_RDONLY);
			close(fd[fd_index]);
			fd_index = fd_index ^ 1;
		}
	}
}

/********************************************************/
/* the length has been checked so skip past		*/
/* the first two bytes and check the command type	*/
/********************************************************/
void process_request(char *buf)
{
	int 	i;
	int 	pid;
	int	n;
	char    c;
	char    *current_field;
	char    command_buffer[USER_COMMAND_LEN];
	char    cpipe_buff[USER_COMMAND_LEN];
	char    message[MAX_LINE+1];

	/************************************************/
	/* clear out buffers ( set to nulls )		*/
	/************************************************/
	bzero(command_buffer,USER_COMMAND_LEN);
	bzero(cpipe_buff,USER_COMMAND_LEN);
	bzero(message,MAX_LINE+1);

	/************************************************/
	/* copy contents of buf into a temp buffer, and	*/
	/* then use that buffer to break up the line	*/
	/* into many fields, also grab the first field	*/
	/* it must be a valid control character c,d,k,q */
	/* or r, & can't be NULL.  The first two	*/
	/* characters the show the command length and	*/
	/* we don't need them, they're already checked	*/
	/************************************************/
	strcpy(command_buffer,buf);

#ifdef DEBUG
printf("processing command < %s >\n",command_buffer);
#endif

	if ((current_field = strtok(command_buffer," ")) == NULL) 
	{
		return;
	}
	current_field+=2;
	/************************************************/
	/* data must be in the buffer to get this far	*/
	/* so continue processing and establish what	*/
	/* kind of command was sent  (c,d,k,r or q)	*/
	/************************************************/
	c = *current_field;
	/************************************************/
	/* determine client's pipe name (using pid) to 	*/
	/* write any info messages   			*/
	/************************************************/
	current_field = strtok(NULL," ");

	sprintf(cpipe_buff,"%s.%s",CLIENT_FIFO,current_field);

	bzero(client_pipe_path,MAX_LINE+1);

	strncpy(client_pipe_path,cpipe_buff,strlen(cpipe_buff));

	/* a new request so reset the 	*/
	/* client file descriptor and	*/
	/* clean/sigpipe flags and 	*/
	/* reopen the new client pipe	*/

	if (client_pipe_fd >= 0)
		close(client_pipe_fd);

	clean = 0;
	got_sig_pipe = 0;

	if ((client_pipe_fd = open(client_pipe_path,O_WRONLY)) < 0) 
	{
		clean = 1;
		log_error(CLIENT_FIFO_WRITE_ERR);
		return;
	}

	switch (c)
	{
		case ACTIVE:
			query_slips(buf);
		break;

		case QUERY:
			query_slips(buf);
		break;
	
		case CONNECT:
			connect_slip(buf);
		break;
	
		case DISCONNECT:
			disconnect_slip(buf);
			write_client(RETURN_CODE,FINISHED);
#ifdef DEBUG
printf("returned from disconnect slips\n");
#endif
		break;
	
		case KILL:
			kill_slip(buf);
			write_client(RETURN_CODE,FINISHED);
		break;
	
		case RECONFIG:
			
			if ((config_addrs(buf)) == SUCCESS)
			{
			 	write_client(RETURN_CODE,FINISHED);
			}
			else
			{
				write_client(RETURN_CODE,RECONFIG_SLIP_FAIL); 
			}
		
		break;
	
		default:
			/* should have been filtered */
			/* by the slipcomm program   */
			log_error(UNKNOWN_SLIP_ERR);
			return;				
		break;
	}
}

/*********************************/
/* reconfigure a slip connection */
/*********************************/
int config_addrs(char *buf)
{
	struct LINK *slipptr;
	struct REMOTE_HOST *host;
	int	slipinfo_fp;
	int 	i = 0;
	int 	return_code = 0;
	int 	pid = 0;
	int 	userid = 0;
	char 	config_slip_string[MAX_LINE+1];
	char 	send_config_string[MAX_LINE+1];
	char 	command_buffer[MAX_LINE+1];
	char 	read_buffer[MAX_LINE+1];
	char	slip[21];
	char	slip_number[6];
	char	gateway_addr[16];
	char	local_addr[16];
	char	lookup_string[MAX_LINE+1];
	char	*p;

	bzero(command_buffer,MAX_LINE+1);
	bzero(read_buffer,MAX_LINE+1);
	bzero(config_slip_string,MAX_LINE+1);
	bzero(send_config_string,MAX_LINE+1);
	bzero(lookup_string,MAX_LINE+1);
	bzero(slip,21);
	bzero(slip_number,6);
	bzero(gateway_addr,16);
	bzero(local_addr,16);

	strcpy(command_buffer,buf);

#ifdef DEBUG
printf("command to process in reconfig = %s\n",command_buffer);
#endif


	/* cmd len and cmd char example = 44r */
	p = strtok(command_buffer," ");

	/* pid */
	p = strtok(NULL," ");
	pid = atoi(p);

	/* slip name */
	p = strtok(NULL," ");
	strncpy(slip,p,strlen(p));

	/* user name */
	p = strtok(NULL," ");
	userid = atoi(p);

	/* local host address */
	p = strtok(NULL," ");
	strcpy(local_addr,p);

#ifdef DEBUG
printf("local addr = %s\n",local_addr);
#endif


	/* gateway address */
	p = strtok(NULL," ");
	strcpy(gateway_addr,p);

#ifdef DEBUG
printf("gateway addr = %s\n",gateway_addr);
#endif


	return_code = FAIL;

	if ((slipptr = lookup_user(slip,userid)) != NULL)
	{

#ifdef DEBUG
printf("slx returned = %s\n",slipptr->slip_number);
#endif

		strncpy(slip_number,slipptr->slip_number,5);

		/* delete_old_routes */

		host = slipptr->remote_hosts;

		while(host)
		{
			if (host->route_added)
				route_cmd("delete",host->remote_host,
						slipptr->slip_gateway);
			host = host->next_host;
		}

		sprintf(config_slip_string,"%s %s %s %s 2>&1\n",
			IFCONFIG_CMD,slip_number,local_addr,gateway_addr);

		strncpy(send_config_string,config_slip_string,76);

		write_client(OUTPUT_DATA,send_config_string);

#ifdef DEBUG
printf("reconfig string  =  %s\n",config_slip_string);
#endif

 		return_code = system(config_slip_string); 

		/* add new routes */

		host = slipptr->remote_hosts;

		while(host)
		{
			host->route_added = 1;
			route_cmd("add",host->remote_host,
					gateway_addr);
			host = host->next_host;
		}

	}
return(return_code);
}

/************************************************/
/* make the slip connection                     */
/* gets the command buffer                      */
/* example 22c usrname slip1                    */
/************************************************/
int connect_slip(char *command_buf)
{
	/* variable declarations */
	int slipinfo_fp;
	int c;
	int i;
	int j;
	int k;
	int n;
	int pid;
	int rts_cts_added;
	int rts_cts_delete;
	int control_type;
	int return_code;
	int added_a_line;
	int stty_output;
	int userid;
	int add_rts_cts_rtn;
	int route_counter = 0;
	int added_this_route = 0;
	int dial_string_changed = 0;
	int skip_two = 0;

	char slip_created[6];
	char *command_buffer;
	char *client_pipe;
	char *current_field;
	char *slipinfo;
	char *slip_desc;
	char *slip_tty;
	char *current_slip_name;
	char *slip_name;
	char *slip_baudrate;
	char *slip_netmask;
	char *slip_dialedhost;
	char *slip_localhost;
	char *slip_remotehost;
	char *slip_dialstring;
	char *route_field;
	char lookup_string[64];
	char read_buffer[BUFSIZ];
	char field[10][257];

	char *p, *q;	/* used to manipulate the dial string 	*/
			/* if it contains embedded colons	*/

	char new_dialstring[257]; 
				/* dial string stripped of 	*/
				/* embedded colons		*/
	struct LINK *newlink;
	struct CLIENT *cl;
	struct REMOTE_HOST *remotehosts;
	struct REMOTE_HOST *tmp_remote_host;

	/* variable declarations end */

	/************************************************/
	/* clear out buffers ( set to nulls )		*/
	/************************************************/
	bzero(read_buffer,BUFSIZ);
	bzero(lookup_string,64);

	command_buffer = strdup(command_buf);

#ifdef DEBUG
printf("in connect slip with buf = %s\n",command_buffer);
#endif

	/************************************************/
	/* after strtok the current field should have 	*/
	/* the form of [length][cmd]			*/
	/* example 22c, but since the command has been 	*/
	/* previously determined we we will just strtok	*/
	/* the buffer again to get the next field	*/
	/************************************************/
	if ((current_field = strtok(command_buffer," ")) == NULL) 
	{
#ifdef DEBUG
printf("command length = NULL\n");
#endif
		/* points to len + cmd and we should 	*/
		/* not get a null so error to client if */
		/* it happens - not sure what error??	*/
		return;
	}

	/************************************************/
	/* this strtok should set the current field to 	*/
	/* process ID					*/
	/************************************************/
	if ((current_field = strtok(NULL," ")) == NULL) 
	{
#ifdef DEBUG
printf("prcess id = NULL\n");
#endif
		/* return some kind of error if we get 	*/
		/* a null - again not sure what error??	*/
		return;
	}
	/************************************************/
	/* use the atoi function to save the process id */
	/* in the pid variable,	check for a valid pid &	*/
	/* for a user (not an init = 1) and if the pid	*/
	/* is still ALIVE? by using kill(pid,0)		*/
	/* pid is used later for user lookup 		*/
	/************************************************/
	pid = atoi(current_field);
	if (pid < MIN_PID || kill(pid,0) != 0) 
	{
		cleanup(CURRENT_USER);
		return;
	}

	/************************************************/
	/* The next strtok should set the current field */
	/* to slip name. Determine if the slip is 	*/
	/* active by checking link info. Look in the 	*/
	/* list for this slip link - if found add the	*/
	/* client ( be sure not to add the user twice ) */
	/* otherwise make room for a new link and	*/
	/* update the contents appropriately. We have 	*/
	/* to look up the configuration on this slip to	*/
	/* determine what tty and baud rate we need 	*/
	/************************************************/
	if ((current_field = strtok(NULL," ")) == NULL) 
	{
#ifdef DEBUG
printf("slipname = NULL\n");
#endif
		log_error(UNKNOWN_SLIP_ERR);
		/* return some kind of error if we get */
		/* a null because it should point to 	*/
		/* the slip name */
		return;
	}
	current_slip_name = strdup(current_field);

	/************************************************/
	/* save the user name before looking up the 	*/
	/* link using the slip name			*/
	/************************************************/
	if ((current_field = strtok(NULL," ")) == NULL) 
	{
#ifdef DEBUG
printf("username = NULL\n");
#endif
		log_error(UNKNOWN_SLIP_ERR);
		/* return error if we get a null 	*/
		/* because it should point to a user id	*/
		return;
	}
	userid = atoi(current_field);
	current_userid = userid;

	/************************************************/
	/* determine if this is a new connection	*/
	/* or if we just need to add the user 		*/
	/* to an existing slip connection - NULL if new	*/
	/************************************************/
#ifdef DEBUG
printf("going to find_link\n");
#endif
	if ((find_link(current_slip_name)) == NULL)  /* new */
	{
	/************************************************/
	/* Add if we are here - make a place for this	*/
	/* client and subsequent clients.		*/
	/* Memory is malloc'd for this client and sets 	*/
	/* the client pid equal to the pid passed to	*/
	/* this function, we can use this to kill the	*/
	/* process and cleanup if we get a kill or 	*/
	/* disconnect, also the demon should ++ the 	*/
	/* user count. If everything is ok we write a 	*/
	/* 0 to the client fifo to indicate success	*/
	/************************************************/

#ifdef DEBUG
printf("back from find_link and current_slip_name == NULL\n");
#endif

		sprintf(lookup_string,"%s -n %s",LSSLIP_CMD,current_slip_name);

		slipinfo_fp = popen(lookup_string,"r");

		fgets(read_buffer,sizeof(read_buffer),slipinfo_fp);

		pclose(slipinfo_fp);

		for (i = 0; i <= 9; i++)
		{
			bzero(field[i],257);
		}

		i = j = k = 0;

		while (read_buffer[k]) 
		{
			if (read_buffer[k] == ':' && !skip_two) 
			{
				j++;
				k++;
				i=0;
			}

			if (read_buffer[k] == '#' && read_buffer[k+1] == '!' 
						&& read_buffer[k+2] == ':')
				skip_two = 1;

			if ( (j == 8) && (skip_two) )  
			{
				if (read_buffer[k] == ':')
				{
					skip_two = 0;
					field[j][i] = read_buffer[k];
					i++;
					k++;
				}
				else
				{
					k++;
				}
				continue;
			}
			else
			{
				field[j][i] = read_buffer[k];
				i++;
				k++;
			}

			if (read_buffer[k] == ':') 
			{
				j++;
				k++;
				i=0;
			}
		}

		slip_name = strdup(field[0]);       /* 1nd field */
		/* Field #2 (description) - not used internally  */
		slip_remotehost = strdup(field[2]); /* 3rd field */
		slip_localhost = strdup(field[3]);  /* 4th field */
		slip_dialedhost = strdup(field[4]); /* 5th field */
		slip_netmask = strdup(field[5]);    /* 6th field */
		slip_tty = strdup(field[6]);        /* 7th field */
		slip_baudrate = strdup(field[7]);   /* 8th field */
		slip_dialstring = strdup(field[8]); /* 9th field */
		G_timeout = atoi(field[9]);         /* 10th field */

		if (strchr(slip_dialstring,':') != NULL)
		{
			dial_string_changed++;
			p = slip_dialstring;
			q = new_dialstring;
			while (*p)
			{
				if (*p == '#' && *p+1 == '!' && *p+2 == ':')
				{
					p+=2;
					*q++ = *p++;
				}
				else
				{
					*q++ = *p++;
				}
			}
		}


#ifdef DEBUG
printf("Global timeout == %d\n",G_timeout);
#endif

		/************************************************/
		/* check to see if we can add rts - also	*/
		/* check to see if modem is on			*/
		/************************************************/

#ifdef DEBUG
printf("going to add rts with = %s\n",slip_tty);
#endif

		add_rts_cts_rtn = add_rts_cts(slip_tty);
#ifdef DEBUG
printf("inside connect slip and add rts return = %d\n",add_rts_cts_rtn);
#endif

		switch(add_rts_cts_rtn) 
		{
		case 255:
			write_client(RETURN_CODE,MODEM_FAIL);
			return;
			break;

		case 0:
			stty_output = 1; /* rts added ok */
			break;

		case -1:
			stty_output = 0; /* rts already added */
			break;
		}
		rts_cts_added = stty_output;
		/****************************************/
		/* check for use of the tty port by 	*/
		/* uucp or by other programs		*/
		/****************************************/
		if ((i = ttylocked(slip_tty)) != 0)
		{
			write_client(RETURN_CODE,TTY_LOCK_FAIL);
			return;
		}
		/************************************************/
		/* run the mkdev command on the tty port we 	*/
		/* want to use as a SLIP connection		*/
		/************************************************/

		bzero(slip_created,6); /* zero out the buffer */

#ifdef DEBUG
printf("going to mkdev_slip\n");
#endif
		
		if ((return_code = mkdev_slip(slip_tty,slip_created)) != 0) 
		{
			write_client(RETURN_CODE,MKDEV_FAIL);
			return;
		}
		/************************************************/
		/* see if we need to add a line to the devices 	*/
		/* file in /etc/uucp  				*/
		/************************************************/
		added_a_line = ck_devices_file(slip_tty,slip_baudrate);

		/************************************************/
		/* try to if config the slip up, add the route 	*/
		/* and slip attach, but copy the slip created	*/
		/* and slip name to the globals for cleanup use	*/
		/************************************************/
		bzero(current_slip,21); /* zero out the buffers */
		bzero(current_slip_number,6);
		strncpy(current_slip_number,slip_created,5);
		strncpy(current_slip,current_slip_name,20);

#ifdef DEBUG
printf("going to ifconfig subroutine\n");
#endif

		if (config_slip_up(slip_created,slip_localhost,
			slip_dialedhost, slip_netmask) != 0) 
		{
			write_client(RETURN_CODE,SLIP_CONFIG_FAIL);
			return;
		}



#ifdef DEBUG
printf("going to slip_attach subroutine\n");
#endif

		if (dial_string_changed)
		{
			/* use the new dial string that has #! stripped */
			slip_attach(slip_tty,slip_baudrate,new_dialstring);
			dial_string_changed = 0; /* reset the dial str flag */
		}
		else
		{
			slip_attach(slip_tty,slip_baudrate,slip_dialstring);
		}

		if (!tty_connected)
		{

#ifdef DEBUG
printf("slip_attach subroutine returned failure\n");
#endif

			if (rts_cts_added)
				del_rts_cts(slip_tty,&FDbufsave);


#ifdef DEBUG
printf("going to rmdev_slip subroutine from if !tty_connected\n");
#endif
			rmdev_slip(slip_created);

			write_client(RETURN_CODE,SLATTACH_FAIL);

			return;
		}

		/* error handling is inside function	*/
		/* and in the child process		*/


#ifdef DEBUG
printf("going to route add\n");
#endif

		route_counter = 0; /* reset the route counter */

		route_field = strtok(slip_remotehost," ");

		if (route_cmd("add",route_field,
				slip_dialedhost) != 0)
			added_this_route = 0;	
		else
			added_this_route = 1;

		remotehosts = (struct REMOTE_HOST *) 
			malloc(sizeof(struct REMOTE_HOST));

		remotehosts->route_added = added_this_route;
		strcpy(remotehosts->remote_host,route_field);
		remotehosts->next_host = NULL; 
		/* we only null the first one because all subsequent	*/
		/* routes are inserted into the front of the list	*/

                while ((route_field = strtok(NULL," ")) != NULL)
                {
			if (route_cmd("add",route_field,
					slip_dialedhost) != 0)
				added_this_route = 0;	
			else
				added_this_route = 1;
			
			/* save the remote hosts pointer	*/
			tmp_remote_host = remotehosts;

			remotehosts = (struct REMOTE_HOST *) 
				malloc(sizeof(struct REMOTE_HOST));
	
			/* insert this remote host at the front	*/
			remotehosts->route_added = added_this_route;

			/* save the current remote host info	*/
			strcpy(remotehosts->remote_host,route_field);

			/* point the next host to the old remote list	*/
			remotehosts->next_host = tmp_remote_host;

                }
#ifdef DEBUG
printf("back from route add\n");
#endif


		/************************************************/
		/* copy all the info into the new link struct 	*/
		/* for copying to the linklist. We've already	*/
		/* checked the devices file to see if we needed */
		/* to add an entry for this slip link, so save	*/
		/* the info needed to remove the line when we 	*/
		/* disconnect the slip later			*/
		/************************************************/
		newlink = (struct LINK *)malloc(sizeof(struct LINK));

       /*          newlink->number_of_routes = route_counter; */

		if (rts_cts_added)
			newlink->added_rts_cts = 1;
		else
			newlink->added_rts_cts = 0;

		if (added_a_line)
			newlink->devices_line_added = 1;
		else
			newlink->devices_line_added= 0;

		newlink->nextlink = NULL;
		newlink->user_count = 1;              
		/* set user count to 1 */

		newlink->slip_number = strdup(slip_created); 
		/* slip # returned from mkdev cmd */

#ifdef DEBUG
printf("adding to newlink slip_number = %s\n",newlink->slip_number);
#endif

		/* string duplicate all  */
		/* fields to be saved in */
		/* the link list 	 */


#ifdef DEBUG
printf("adding to newlink slip_name = %s\n",slip_name);
#endif

		/* newlink->routes = route_struct;  */
		newlink->remote_hosts = remotehosts;
		newlink->slip_name = slip_name; 
		newlink->slip_lhost = slip_localhost;
       	   	newlink->slip_gateway = slip_dialedhost;
		newlink->slip_mask = slip_netmask;
		newlink->slip_ttyport = slip_tty;
		newlink->slip_baud = slip_baudrate;
		newlink->slip_dialstr = slip_dialstring;
		newlink->savebuffer = (struct termios *)malloc(sizeof(struct termios));
		memcpy(newlink->savebuffer,&FDbufsave,sizeof(struct termios));
		cl = (struct CLIENT *) malloc(sizeof(struct CLIENT));
		cl->pid = pid;
		cl->userid = userid;
		cl->nextclient = NULL;
		newlink->client = cl;
		add_link(newlink);

	}
	else 
	{
		if ((lookup_user(current_slip_name,userid)) == NULL) 
		{

#ifdef DEBUG
printf("existing slip, but adding %d\n",userid);
#endif

			/* slip exists and we're just adding a client	*/
			/* lookup ensures that we aren't duplicating	*/
			/* so set the connect flag so we don't get	*/
			/* an error sent to the client pipe      	*/
			tty_connected = 1;

			cl = (struct CLIENT *) malloc(sizeof(struct CLIENT));
			cl->pid = pid;
			cl->userid = userid;
			cl->nextclient = NULL;
			add_client(current_slip_name,cl);

#ifdef DEBUG
printf("Client info before going to Add Client\n");
printf(" < Pid = %d, Userid = %d >\n",cl->pid,cl->userid);
#endif

		}
		else
		{

#ifdef DEBUG
printf("slip exists, and %d is already in the list\n",userid);
#endif

			write_client(RETURN_CODE,ALREADY_CONNECTED);
			return;
		}
		
	}

	/************************************************/
	/* sigpipe is set so we can handle the client	*/
	/* going away after she/he starts a dialog with */
	/* the daemon if we get a sigpipe we will use 	*/
	/* the pid of the user and do a little clean up	*/
	/* things like removing client_pipe		*/
	/* if we do not have an active slip the 	*/
	/* following steps will take place 		*/
	/* read the ODM database for the specified 	*/
	/* connection and user profiles 		*/
	/* determine if the connection's tty port is 	*/
	/* currently being used by another program like	*/
	/* UUCP, check /etc/locks/ (proceed if ok)	*/
	/* write a 1 to the client pipe to indicate tty	*/
	/* in use if so add RTS/CTS control to the tty	*/
	/* run the mkdev command to create the slip and	*/
	/* capture stdout write a 2 to the client pipe 	*/
	/* if the command fails. If necessary add the 	*/
	/* appropriate line to the Devices file		*/
	/* if the Devices file does not exist I will 	*/
	/* create it using the a+ for mode		*/
	/* configure the slip up using the ifconfig 	*/
	/* command, add a route for this session based 	*/
	/* on the SLIP profile in the database, if the	*/
	/* route command fails write a 5 to the client 	*/
	/* pipe. Exec the slattach command to establish	*/
	/* the connection and write a 6 to the client 	*/
	/* pipe, if the command fails, otherwise write 	*/
	/* a 0 to the client pipe to indicate that all	*/
	/* operations were successful. Close the client	*/
	/* pipe and go back to reading the pipe		*/
	/************************************************/
	if (tty_connected)
	{
		/* got a successfull connection */
		write_client(RETURN_CODE,CONNECTED);

		/* reset the tty flag for the next try */
		tty_connected = 0;

		if (clean)
		{

#ifdef DEBUG
printf("SIGPIPE? -> to delete client with %s %d\n",current_slip,current_userid);
#endif
			delete_client(current_slip,current_userid);
			if (access(client_pipe_path,F_OK) == 0)
				unlink(client_pipe_path);
			clean = 0;
		}
	}
#ifdef DEBUG
printf("returning to read another request\n");
#endif
return;
	 free(command_buf);
	 free(current_slip_name);
}

/********************************************************/
/* disconnect a slip link - remove from link list       */
/********************************************************/
disconnect_slip(char *command_buf)
{
	char	command_buffer[128];
	char	*current_field;
	char	slip[21];
	int	pid;
	int	userid;

	strcpy(command_buffer,command_buf);

	current_field = strtok(command_buffer," "); 
		/* length and command */

	current_field = strtok(NULL," "); 
	pid = atoi(current_field);
		/* pid */

	current_field = strtok(NULL," "); 
	strcpy(slip,current_field); 
		/* slip name */

	current_field = strtok(NULL," "); 
	userid = atoi(current_field);
		/* userid */

	if ((lookup_user(slip,userid)) != NULL)
		delete_client(slip,userid); 

	/************************************************/
	/* delete_client should do the following  	*/
	/* remove the user from the Link->client 	*/
	/* list and decrement the user count for 	*/
	/* that slip, if the # of users goes to 0 then 	*/
	/* it's time to disconnect the slip from 	*/
	/* service.  Issue the route delete command, 	*/
	/* ifconfig sl#x down, maybe rm the line from 	*/
	/* the Devices file, remove the slip (rmdev) 	*/
	/* (if added) remove rts/cts using stty/ioctl 	*/
	/* close the client pipe return for next cmd	*/
	/************************************************/
return(SUCCESS);
}

/********************************************************/
/* check the link list for a given slip connection      */
/********************************************************/
int query_slips(char *buf)
{
	struct	LINK *Link;
	struct	CLIENT *tmp;
	char	*current_field;
	char	command_buf[MAX_LINE+1];
	char	link_info[MAX_LINE+1];
	char	send_str[MAX_LINE+1];
	int	pid;
	int	userid;

	bzero(command_buf,MAX_LINE+1);

	strncpy(command_buf,buf,strlen(buf));

	/* cmd and length */
	current_field = strtok(command_buf," ");

	/* pid */
	current_field = strtok(NULL," ");
	pid = atoi(current_field);

	/* user name */
	current_field = strtok(NULL," ");
	userid = atoi(current_field);
	Link = global_link;

#ifdef DEBUG
printf("in query slips, uid = %d\n",userid);
#endif

	if (Link == NULL) 
	{
		write_client(RETURN_CODE,NO_SLIPS);
		return;
	}

	/* the root user gets all active slips sent back */
	if (userid == MINUS_ONE) 
	{
		while(Link) 
		{
			bzero(link_info,MAX_LINE+1);
			sprintf(link_info,"%s\n",Link->slip_name);

#ifdef DEBUG
printf("# of users (root request) = %d\n",Link->user_count);
#endif

			if (strlen(link_info) > 77)
				link_info[77] = 0;
			write_client(OUTPUT_DATA,link_info);
			Link = Link->nextlink;
		}
		/* from the first to last Link - look up each client	*/
		/* this is a root user request and s/he wants all the 	*/
		/* slips in use sent to her/his pipe			*/

		/* done to tell the client */
		write_client(RETURN_CODE,FINISHED);
	}
	else
	{
		while(Link)
		{
			/* the average user just gets the slip that	*/
			/* s/he has configured and active		*/

#ifdef DEBUG
printf("Slip Name = %s\t\t# of users (user request) = %d\n",Link->slip_name,Link->user_count);
#endif

			tmp = Link->client;
			while (tmp)
			{

#ifdef DEBUG
printf("uid = %d, Link->client->pid = %d, Link->client->userid = %d\n",
		userid,tmp->pid,tmp->userid);
#endif
				if (userid == tmp->userid)
				{
					bzero(link_info,MAX_LINE+1);
					sprintf(link_info,"%s\n"
			    			,Link->slip_name);
					if (strlen(link_info) > 77)
						link_info[77] = 0;
					write_client(OUTPUT_DATA,link_info);
					break;
				}
				tmp = tmp->nextclient;
			}

		 	 Link = Link->nextlink;
		}
		/* done so tell the client */
		write_client(RETURN_CODE,END_OF_LIST);
	}
}

/************************************************/
/* cleanup gets a CURRENT_USER  ALL	 	*/
/************************************************/
void cleanup(char c)
{
	/* delete pipe(s) and kill client(s) 	*/
	/* depending on CURRENT_USER or ALL parameter	*/

	char rm_string[128];
	bzero(rm_string,128);

	switch(c)
	{
	case ALL:
		rmdev_slip(current_slip_number);
		kill_one_or_all_slips("all"); 
	break;

	case CURRENT_USER:

		if (access(client_pipe_path,F_OK) == 0)
			unlink(client_pipe_path);

#ifdef DEBUG
printf("setting clean = 1 in cleanup()\n");
#endif

		clean = 1;
	break;

	default:
	break;
	}
	return;
}

/********************************************************/
/*							*/
/********************************************************/
terminate_server(int n)
{
	FILE     *fd_out;

	fd_out = fopen(ERRORLOG,"a+");

#ifdef DEBUG
printf("in terminate server with sig = %d\n",n);
#endif

	switch(n)
	{
	case SIGINT:
	case SIGTERM:
		if (slattach_fork_rtn > MIN_PID)
			kill(slattach_fork_rtn,SIGKILL);
		unlink(SERVER_PID_FILE);
		unlink(SERVER_FIFO);
#ifdef DEBUG
printf("got a sigterm\n");
#endif
		cleanup(ALL);
#ifdef DEBUG
printf("back from clean\n");
#endif
		exit(FAIL);
		break;

	case BADCOMMAND:
	case BADLEN:
	case SIGDANGER:
	default:
		log_error(UNKNOWN_SLIP_ERR);
		if (slattach_fork_rtn > MIN_PID)
			kill(slattach_fork_rtn,SIGKILL);
		unlink(SERVER_PID_FILE);
		unlink(SERVER_FIFO);
		cleanup(ALL);
		exit(FAIL);
		break;
	}
}


/********************************************************/
/* Traverse the list until we find an active 		*/
/* Link or return a NULL				*/
/********************************************************/
struct LINK *find_link(char *slip_name)
{
	struct LINK *Link;

	Link = global_link;

	if (Link == NULL)
		return(NULL);

	/************************************************/
	/* link list so look for it below	*/
	/************************************************/

	while (Link) 
	{
		if ((strcmp(Link->slip_name,slip_name)) == 0)
			return(Link);
		else 
			Link = Link->nextlink;
	}
	return(NULL);
}




/********************************************************/
/* add a link to the linked list - add it to the end 	*/
/* of the list						*/
/********************************************************/
int add_link(struct LINK *newlink)
{
int return_code;

	struct LINK *Link;

	Link = global_link;

	return_code = FAIL; 
		/****************************/
		/* if everything goes well  */
		/* the return code will be  */
		/* set to SUCCESS, otherwise*/
		/* the function returns fail*/
		/****************************/

	if (Link == NULL) 
	{
	/************************************************/
	/* must be an empty list so just add the new 	*/
	/* link  and return				*/
	/************************************************/
		global_link = newlink;
		return_code = SUCCESS;
	}
	/************************************************/
	/*  not an empty list so find the end of the 	*/
	/* list and add this new link 			*/
	/************************************************/
	else
	{
		while (Link->nextlink)
			Link = Link->nextlink;

		Link->nextlink = newlink;
		return_code = SUCCESS;
	}
return(return_code);
}


struct LINK *lookup_user(char *slip_name, int userid)
{
	struct CLIENT *tmp;
	struct LINK *Link;

	Link = global_link;

	if (Link == NULL)
		return(NULL);

	while(Link) 
	{
		if ((strcmp(Link->slip_name,slip_name)) == 0)
		{

			tmp = Link->client;
			while(tmp) 
			{
				if (tmp->userid == userid)
				{
#ifdef DEBUG
printf("In Lookup_User - User matched\n"
"Client PID = %d, User Id = %d\nReturning Link %s\n",
tmp->pid,tmp->userid,Link->slip_name);
#endif
					return(Link);
				}
				tmp = tmp->nextclient;
			}
		}
		Link = Link->nextlink;
	}

#ifdef DEBUG
printf("not found - returning NULL\n");
#endif

return(NULL);
}


/************************************************/
/* adds a client to an existing slip connection */
/************************************************/
void add_client(char *slipname, struct CLIENT *cl)
{
	struct CLIENT *tmp;
	struct LINK *Link;

	Link = global_link;
	/************************************************/
	/* find the link that this client belongs to by */
	/* comparing slip name to the link slip name 	*/
	/* and then place the client at the end of the 	*/
	/* list 					*/
	/************************************************/

#ifdef DEBUG
printf("in add_client - Pid = %d and Userid = %d\n",cl->pid,cl->userid);
#endif

	while (Link) 
	{
		if ((strcmp(Link->slip_name,slipname)) == 0) 
		{

#ifdef DEBUG
printf("in add_client - got a match on the slipname\n");
printf("adding %d to %s\n",cl->userid,Link->slip_name);
#endif
                        if (Link->client == NULL)
                        {
			/************************************************/
			/* add the client & increment the user count	*/
			/************************************************/

#ifdef DEBUG
printf("first one - adding client\n");
#endif

				Link->client = cl;
				Link->user_count++;
				return;
			}
			else
			{
				tmp = Link->client;
				Link->client = cl;
				Link->client->nextclient = tmp;
				Link->user_count++;
				return;
			}
		}
		Link = Link->nextlink;
	} /* end of while links */
} /* end of add client */

/************************************************/
/* function to delete clients from the list	*/ 
/* and cleanup connections if this is the  	*/ 
/* last user using this slip connection    	*/ 
/************************************************/
int delete_client(char *slipname, int userid)
{
	struct LINK     *temp;
	struct LINK     *prev_link;
	struct LINK     *Link;
	struct CLIENT	*tmp;
	struct CLIENT	*prev_client;
	struct CLIENT	*client;
	int i;
	int number_of_users = 0;
	int error_rtn;

	Link = global_link; 
	/* set pointer to the start of the link list */

#ifdef DEBUG
printf("in delete client \n");
#endif

	prev_link = Link;
	while (Link) 
	{
		if ((strcmp(Link->slip_name,slipname)) == 0) 
		{
			if (Link->client->userid == userid)
			{

#ifdef DEBUG
printf("matched the userid < %d >\n",Link->client->userid);
printf("Link user count == < %d >\n",Link->user_count);
#endif

				tmp = Link->client;
				number_of_users = Link->user_count;
				if (--number_of_users == 0)
				{
					kill_one_or_all_slips(slipname);
					return(SUCCESS);
				}
				else
				{

#ifdef DEBUG
printf("just removing user < %d >\n",tmp->userid);
#endif

					Link->client = Link->client->nextclient;
					free(tmp);
					--Link->user_count;

#ifdef DEBUG
printf("# of users now == < %d >\n",Link->user_count);
#endif

					return(SUCCESS);
				}
			}
			else 
			{
				prev_client = Link->client;
				client = Link->client->nextclient;
				while(client) 
				{
					if (client->userid == userid)
					{
						tmp = client;
						prev_client->nextclient = 
							client->nextclient;
						free(tmp);
						--Link->user_count;
						return(SUCCESS);
					}
					else 
					{
						prev_client = client;
						client = client->nextclient;
					}
				}
			}
		}
		prev_link = Link;
		Link = Link->nextlink;
	}

#ifdef DEBUG
printf("leaving delete client \n");
#endif

} /* end of delete client */

/************************************************/
/* function to kill a slip from a failed 	*/
/* connection or because we are bring the 	*/
/* slip down 					*/
/************************************************/
void kill_one_or_all_slips(char *slip_name)
{
	char slipname[21];
	struct LINK *link_temp;
	struct LINK *Link;
	struct LINK *prev_link;
	struct CLIENT *client_tmp;
	struct REMOTE_HOST *host_tmp;
	struct REMOTE_HOST *hosts;
	int	pid;
	int	i;
	int	error_rtn;

	bzero(slipname,21);


#ifdef DEBUG
printf("slip in kill_one... = %s\n",slip_name);
#endif

	/* set pointer to the start of the link list */
	Link = global_link; 

	if (Link == NULL) 
	{

#ifdef DEBUG
printf("no slips to kill\n");
#endif

		return;
	}

	if (strcmp("all",slip_name) == 0)
	{

#ifdef DEBUG
printf("killing (  all  ) slips\n");
#endif
		unlink(SERVER_FIFO); 	/* going down so get rid of the	*/
					/* well known fifo		*/

		while (Link) 
		{
			global_link = Link->nextlink;

			hosts = Link->remote_hosts;

			while(hosts) 
			{
				if (hosts->route_added)
				{
#ifdef DEBUG
printf("-> to route delete w/route delete %s %s\n",hosts->remote_host,
						Link->slip_gateway);
#endif
					route_cmd("delete",
						hosts->remote_host,
						Link->slip_gateway);
				}
				hosts = hosts->next_host;
			}

			if (config_slip_down(Link->slip_number) != 0)
				log_error(CONFIG_DOWN_FAIL);

			if (strcmp(Link->slip_number,current_slip_number) != 0)
			{
#ifdef DEBUG
printf("-> to rmdev_slip subroutine (kill all) w/%s\n",Link->slip_number);
#endif
				if (rmdev_slip(Link->slip_number) != 0)
					log_error(REMDEV_SLIP_FAIL);
			}

			if (Link->added_rts_cts)
				if (del_rts_cts(Link->slip_ttyport,
					&Link->savebuffer) < 0)
					log_error(DEL_RTS_FAIL);

			if (Link->devices_line_added)
				if (remove_line(Link->slip_ttyport,
					 Link->slip_baud) != 0)
					log_error(DEVICES_RMLINE_FAIL);

			while(Link->client) 
			{
				client_tmp = Link->client;
				Link->client = Link->client->nextclient;
				free(client_tmp);
			}

			hosts = Link->remote_hosts;
			while(hosts) 
			{
				host_tmp = hosts;
				hosts = hosts->next_host;
				free(host_tmp);
			}

			link_temp = Link;
			Link = Link->nextlink;
			free(link_temp->slip_number);
			free(link_temp->slip_name);
			free(link_temp->slip_lhost);
			free(link_temp->slip_gateway);
			free(link_temp->slip_mask);
			free(link_temp->slip_ttyport);
			free(link_temp->slip_baud);
			free(link_temp->slip_dialstr);
			free(link_temp);
		}
	return;
	}

	/* slip name is not "all" so it must 	*/
	/* be a specific slip, if it's the 	*/
	/* first link it's a special case and	*/
	/* the global link must be repositioned */


#ifdef DEBUG
printf("(first one = %s)\n",Link->slip_name);
#endif

	if ((strcmp(Link->slip_name,slip_name)) == 0)
	{

#ifdef DEBUG
printf("got a match (first one)\n");
#endif

		hosts = Link->remote_hosts;
		while(hosts) 
		{
			if (hosts->route_added)
			{
				route_cmd("delete",
					hosts->remote_host,
					Link->slip_gateway);
			}
			hosts = hosts->next_host;
		}

		if (config_slip_down(Link->slip_number) != 0)
				log_error(CONFIG_DOWN_FAIL);

#ifdef DEBUG
printf("-> to rmdev_slip subroutine (kill one) w/%s\n",Link->slip_number); 
#endif
		if (rmdev_slip(Link->slip_number) != 0)
				log_error(REMDEV_SLIP_FAIL);

		if (Link->added_rts_cts)
			if (del_rts_cts(Link->slip_ttyport,
				&Link->savebuffer) < 0)
				log_error(DEL_RTS_FAIL);

		if (Link->devices_line_added)
			if (remove_line(Link->slip_ttyport,
				 Link->slip_baud) != 0)
				log_error(DEVICES_RMLINE_FAIL);

		while(Link->client) 
		{
			client_tmp = Link->client;
			Link->client = Link->client->nextclient;
			free(client_tmp);
		}

		while(Link->remote_hosts) 
		{
			host_tmp = Link->remote_hosts;
			Link->remote_hosts = Link->remote_hosts->next_host;
			free(host_tmp);
		}

		link_temp = Link;
		free(link_temp->slip_number);
		free(link_temp->slip_name);
		free(link_temp->slip_lhost);
		free(link_temp->slip_gateway);
		free(link_temp->slip_mask);
		free(link_temp->slip_ttyport);
		free(link_temp->slip_baud);
		free(link_temp->slip_dialstr);
		free(link_temp);
		global_link = Link->nextlink;
#ifdef DEBUG
if (global_link == NULL)
printf("global_link is NULL now\n");
printf("Link removed\n");
#endif
		return;
	}
	else
	{
		
		prev_link = Link; /* save the pointer */
		
		Link = Link->nextlink; /* step down the list */

#ifdef DEBUG
printf("next one and slip = %s\n",Link->slip_name);
#endif

		while (Link) 
		{

#ifdef DEBUG
printf("not the first one and slip = %s\n",Link->slip_name);
#endif

			if ((strcmp(Link->slip_name,slip_name)) == 0) 
			{

#ifdef DEBUG
printf("got a match (not the first one)\n");
#endif

				hosts = Link->remote_hosts;

				while(hosts) 
				{
					if (hosts->route_added)
					{
						route_cmd("delete",
							hosts->remote_host,
							Link->slip_gateway);
					}
					hosts = hosts->next_host;
				}

				if (config_slip_down(Link->slip_number) != 0)
 					log_error(CONFIG_DOWN_FAIL);

#ifdef DEBUG
printf("-> to rmdev_slip subroutine (kill one - not 1st) w/%s\n",Link->slip_number);
#endif
				if (rmdev_slip(Link->slip_number) != 0)
					log_error(REMDEV_SLIP_FAIL);

				if (Link->added_rts_cts)
					if (del_rts_cts(Link->slip_ttyport,
						&Link->savebuffer) < 0)
						log_error(DEL_RTS_FAIL);

				if (Link->devices_line_added)
					if (remove_line(Link->slip_ttyport,
						 Link->slip_baud) != 0)
						log_error(DEVICES_RMLINE_FAIL);

				while(Link->client) 
				{
					client_tmp = Link->client;
					Link->client = 
						Link->client->nextclient;
					free(client_tmp);
				}

				while(Link->remote_hosts) 
				{
					host_tmp = Link->remote_hosts;
					Link->remote_hosts = 
						Link->remote_hosts->next_host;
					free(host_tmp);
				}

				link_temp = Link;
				prev_link->nextlink = Link->nextlink;
				free(link_temp->slip_number);
				free(link_temp->slip_name);
				free(link_temp->slip_lhost);
				free(link_temp->slip_gateway);
				free(link_temp->slip_mask);
				free(link_temp->slip_ttyport);
				free(link_temp->slip_baud);
				free(link_temp->slip_dialstr);
				free(link_temp);
				return;
			}

			prev_link = Link; /* save */

			Link = Link->nextlink; /* step */
		}
	}

}

/********************************/
/* function to kill an active	*/
/* slip from slipcomm 		*/
/********************************/
void kill_slip(char *command_buf)
{
	char *command_buffer;
	char slip_name[21];
	char *current_field;
	int	pid;
	int	i;

	command_buffer = strdup(command_buf);

	current_field = strtok(command_buf," "); 
		/* length and command */

	current_field = strtok(NULL," "); /* pid */
	pid = atoi(current_field);

	current_field = strtok(NULL," "); /* slip names	*/

	bzero(slip_name,21); /* null out slip_name	*/

	strcpy(slip_name,current_field);

	current_field = strtok(NULL," "); /* user name */

	if ((atoi(current_field)) != ROOT)  /* send'em home if not root	*/
		return;

	kill_one_or_all_slips(slip_name);

	free(command_buf);

return;
}

/***********************************************/
/*   Sends a message to the client pipe        */
/***********************************************/
void write_client(char type, char *message)
{
	char	send_str[MAX_LINE+1];
	char	output_type[2];

#ifdef DEBUG
/* printf("write_client parms = %c, %s\n",type, message); */
#endif

	if (got_sig_pipe)
		return;
	else
	{
		bzero(output_type,2);

		bzero(send_str,MAX_LINE+1);

		sprintf(send_str,"%02d",strlen(message)+3);

		if (type == 'r')
			output_type[0] = 'r';

		if (type == 'o')
			output_type[0] = 'o';

		strcat(send_str,output_type);

		strcat(send_str,message);


#ifdef DEBUG
/* printf("writing ->  %s\n",send_str); */
#endif

		write(client_pipe_fd,send_str,MAX_LINE);
	}

} /* end of write_client */



/****************************************/
/* sends error codes to an errorlog     */
/****************************************/
void log_error(int error)
{
	struct err_rec e;
	int	rcs_error = 0;

	switch(error)
	{
		case LOCKFILE_CREAT_ERR:
			rcs_error = ERRID_RCS_ERR_MSG_1;
		break;

		case MKFIFO_SERVER_ERR:
			rcs_error = ERRID_RCS_ERR_MSG_2;
		break;

		case CLIENT_FIFO_WRITE_ERR:
			rcs_error = ERRID_RCS_ERR_MSG_3;
		break;

		case UNKNOWN_SLIP_ERR:
			rcs_error = ERRID_RCS_ERR_MSG_4;
		break;

		case TXGETLD_IOCTL_FAIL:
			rcs_error = ERRID_RCS_ERR_MSG_5;
		break;

		case REMDEV_SLIP_FAIL:
			rcs_error = ERRID_RCS_ERR_MSG_6;
		break;

		case CONFIG_DOWN_FAIL:
			rcs_error = ERRID_RCS_ERR_MSG_7;
		break;

		case SLATTACH_INFO_FORK_FAIL: 
			rcs_error = ERRID_RCS_ERR_MSG_8;
		break;
		
		case DEL_RTS_FAIL: 
			rcs_error = ERRID_RCS_ERR_MSG_9;
		break;

		case CREATE_ERR:
		case DEVTMP_WRITE_FAIL:
		case DEVICES_APPEND_ERR:
		case CHOWN_DEVICES_FAIL:
		case ETC_DEV_RENAME_FAIL:
		case ETC_DEV_UNLINK_FAIL: 
		case DEVICES_RMLINE_FAIL:
			rcs_error = ERRID_RCS_FILE_IO_ERR;
		break;

		default:
			rcs_error = ERRID_RCS_ERR_MSG_4;
		break;

	}

	e.error_id = rcs_error;
	strcpy(e.resource_name,"RCS");
	errlog(&e,sizeof(e));
}

/****************************************/
/* checks for existance of entries 	*/
/* in /etc/Devices file       		*/
/****************************************/
ck_devices_file(char *ttyport, char *baudrate)
{
	int		grep_output_ptr;
	int		i;
	int		return_code;
	char	read_buffer[81];
	char	grep_str[41];


	return_code = 0; 
	/* set return code to 0 (default)	*/
	/* and lets the function determine	*/ 
	/* if it needs to be otherwise 		*/

	if (access(ETC_DEVICES,F_OK) == 0) 
	{
		bzero(grep_str,41);

		sprintf(grep_str,
		    "%s \"^Direct.*%s\" %s | %s %s",GREP_CMD,
		    ttyport,ETC_DEVICES,GREP_CMD,baudrate);


#ifdef DEBUG
printf("grep parms  = %s, %s\n",ttyport,baudrate);
printf("grep string = %s\n",grep_str);
#endif

		grep_output_ptr = popen(grep_str,"r");

		bzero(read_buffer,81);

		fgets(read_buffer,sizeof(read_buffer),grep_output_ptr);

#ifdef DEBUG
printf("Devices file output from grep = %s\n",read_buffer);
#endif

		pclose(grep_output_ptr);

		if (read_buffer[0] != 'D') 
		{

#ifdef DEBUG
printf("adding a line to Devices\n");
#endif

			add_line_devices(ttyport,baudrate);
			return_code = 1;
		}
	}
	else 
	{
		mkdevices(ttyport,baudrate);
		return_code = 1;
	}

	return(return_code);
}

/****************************************/
/* add a line to the devices file	*/
/* when the one needed doesn't exist   	*/
/****************************************/
add_line_devices(char *ttyport, char *baudrate)
{
	FILE	*devices_ptr;
	char line_to_add[81];

	bzero(line_to_add,81);

	sprintf(line_to_add, "Direct %s - %s direct",
	    ttyport,baudrate);
	if ((devices_ptr = fopen(ETC_DEVICES,"a+")) == NULL) 
	{
		log_error(DEVICES_APPEND_ERR);
		return;
	}
	fprintf(devices_ptr,"%s\n",line_to_add);
	fclose(devices_ptr);
}

/****************************************/
/* creates an /etc/uucp/Devices 	*/
/* file when one doesn't exist       	*/
/****************************************/
mkdevices(char *ttyport, char *baudrate)
{
	FILE	*devices_fd;
	char line_to_add[81];

	bzero(line_to_add,81);

	sprintf(line_to_add, "Direct %s - %s direct",
	    ttyport,baudrate);
	if ((devices_fd = fopen(ETC_DEVICES,"w+")) == NULL) 
	{
		log_error(CREATE_ERR);
		return;
	}
	fprintf(devices_fd,"%s\n",line_to_add);
	fclose(devices_fd);
}

/*************************************************/
/* adds RTS dicipline to /dev/tty(x)		 */
/*************************************************/
int add_rts_cts(char *tty)
{
	int	return_code;
	int fd, fd2, i;
	char ttyport[16];
	struct termios FDbuf;

	bzero(ttyport,16);

	sprintf(ttyport,"/dev/%s",tty);

#ifdef DEBUG
printf("attempt (first) to open port %s\n",ttyport);
#endif

	if ((fd = open(ttyport,O_RDWR | O_NDELAY | O_NOCTTY)) < 0) 
	{
#ifdef DEBUG
printf("(first) failed to open port %s and will return with fail code\n",ttyport);
#endif
		return(FAIL);
	}

#ifdef DEBUG
printf("going to get attr's\n");
#endif
	tcgetattr(fd, &FDbuf);	
	tcgetattr(fd, &FDbufsave);	

#ifdef DEBUG
printf("just got attr's\n");
#endif

	FDbuf.c_cflag = FDbuf.c_cflag | CLOCAL | HUPCL;

	tcsetattr(fd, TCSANOW, &FDbuf);
#ifdef DEBUG
printf("just set tcsetattr");
#endif

	close(fd); 


#ifdef DEBUG
printf("attempt (second) to open port %s\n",ttyport);
#endif

	if ((fd2 = open(ttyport,O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
		return(FAIL);

	return_code = ioctl(fd,TXADDCD,"rts");

#ifdef DEBUG
printf("return from ioctl = %d\n",return_code);
#endif

	close(fd2);

	return(return_code);
}

/************************************************/
/* deletes rts from /dev/ttyx if it was added 	*/ 
/************************************************/
int del_rts_cts(char *tty, struct termios *prev_settings)
{
	int	return_code;
	int fd, fd2, i;
	char ttyport[16];
	struct termios FDbuf;

	bzero(ttyport,16);

	sprintf(ttyport,"/dev/%s",tty);

	return_code = 0;

	if ((fd = open(ttyport,O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
	{
		return(FAIL);
	}
	return_code = ioctl(fd,TXDELCD,"rts");


#ifdef DEBUG
printf("return from ioctl = %d\n",return_code);
#endif

	tcsetattr(fd, TCSANOW, prev_settings);
	close(fd);

	return(return_code);
}

/********************************************/
/* runs the ifconfig command on slx which   */
/* was determined when mkdev was run        */
/********************************************/
int config_slip_up(char *slip, char *local_addr, char *dest_addr, char *netmask)
{
	int	return_code;
	char 	send_config_string[MAX_LINE+1];
	char 	config_slip_string[MAX_LINE+1];

	bzero(send_config_string,MAX_LINE+1);
	bzero(config_slip_string,MAX_LINE+1);

	return_code = 0;

	/* sometimes netmask is a null string */
	if (atoi(netmask) > 0 )
		sprintf(config_slip_string,"%s %s inet %s %s netmask %s up\n",
	    		IFCONFIG_CMD,slip,local_addr,dest_addr,netmask);
	else
		sprintf(config_slip_string,"%s %s %s %s up\n",
	    		IFCONFIG_CMD,slip,local_addr,dest_addr);


#ifdef DEBUG
printf("ifconfig str = %s\n",config_slip_string);
#endif

	strncpy(send_config_string,config_slip_string,76);

	write_client(OUTPUT_DATA,send_config_string);

	return_code = system(config_slip_string);
	
	return(return_code);
}


/************************************************/
/* brings an active slip connection down	*/
/************************************************/
int config_slip_down(char *slip)
{
	int	return_code;
	char 	config_slip_string[MAX_LINE+1];

	return_code = 0;

	bzero(config_slip_string,MAX_LINE+1);

	sprintf(config_slip_string,"%s %s down",IFCONFIG_CMD,slip);

	return_code = system(config_slip_string);

	return(return_code);
}

/************************************************/
/* route command to either add or delete	*/                         
/* typical command = route add 1.1.1.1 1.1.1.2	*/
/* where 1.1.1.1 is the remote host to receive	*/
/* where 1.1.1.2 is the gateway to the remote	*/
/* host that will be receiving the packets	*/
/************************************************/
int route_cmd(char *cmd, char *first_addr, char *second_addr)
{
	int	return_code = 0;
	int	route_info_fp;
	char 	route_cmd_string[MAX_ROUTE_CMD+1];
	char 	client_send_string[MAX_LINE+1];
	char 	read_buffer[MAX_LINE+1];

	bzero(route_cmd_string,MAX_ROUTE_CMD+1);
	bzero(client_send_string,MAX_LINE+1);

		/* command string should look 	*/
		/* something like		*/
		/* route add address address |	*/
		/* route delete address address	*/
	sprintf(route_cmd_string,"%s %s %s %s\n",
				ROUTE_CMD,cmd,first_addr,second_addr);

#ifdef DEBUG
printf("route command = %s\n",route_cmd_string);
#endif

	/* 76 bytes leaves room for strlen + o to client */
	strncpy(client_send_string,route_cmd_string,76);

	write_client(OUTPUT_DATA,client_send_string); 

	route_info_fp = popen(route_cmd_string,"r");

	while ((fgets(read_buffer,sizeof(read_buffer),
			route_info_fp)) != NULL) 
	{
		if (strlen(read_buffer) > 77)
			read_buffer[77] = 0;
		write_client(OUTPUT_DATA,read_buffer); 
	}

	return_code = pclose(route_info_fp);

return(return_code); 
}
	

/************************************************/
/*  slip attach (slattach tty0 9600 atdt ...)	*/
/************************************************/
int slip_attach(char *tty, char *speed, char *dial_string)
{
	int	tty_fd;
	int	i = 0;
	int	wait_rtn;
	int	counter = 0;
	int	num_loops = 0;
	int	one_more = 0;
	int	interrupted;
	int	return_code;
	int	slattach_info_fp;
	char	*p;
	char	slattach_cmd_str[1024];
        char    get_slip_status[MAX_LINE+1];
	char	buf[20];
	char	read_buffer[MAX_LINE+1];
	char	send_str[MAX_LINE+1];
        struct termios FDbuf;

        bzero(get_slip_status,MAX_LINE+1);

	bzero(buf,20); /* holds tty line descipline */

	bzero(slattach_cmd_str,1024);

	bzero(read_buffer,MAX_LINE+1);

	bzero(send_str,MAX_LINE+1);

	bzero(slattach_port,MAX_LINE+1);

	sprintf(slattach_port,"/dev/%s",tty);

	/* set the modem lines off (baud rate = 0) */
	if ((tty_fd = open(slattach_port,O_RDWR | O_NDELAY | O_NOCTTY)) < 0)
        {
                return(FAIL);
        }

	tcgetattr(tty_fd, &FDbuf);

	if (i = (cfsetospeed(&FDbuf, B0)) < 0)
        {
                return(FAIL);
        }

	tcsetattr(tty_fd, TCSANOW, &FDbuf);

	close(tty_fd);
	/* end of setting  baud == 0 */

       return_code = system("/usr/sbin/strinfo -m | grep \"\'slip\'\" > \
				/dev/null || /usr/sbin/strload -m \
				/usr/lib/drivers/slip");

	if ( return_code == 0)
	{
		tty_connected = 1;
	}

#ifdef DEBUG
printf("returned from the strload function\n");
#endif

	if ((slattach_fork_rtn = fork ()) < 0) /* fork a child process */
		log_error(SLATTACH_INFO_FORK_FAIL);

	if (slattach_fork_rtn == 0)  /* 0 means we are the child */
	{

		sprintf(&slattach_cmd_str[0],"%s %s %s '%s' 9 2>&1\n",
			SLATTACH_CMD,tty,speed,dial_string);

		num_loops = (strlen(slattach_cmd_str)/77);

		if ( (one_more=(strlen(slattach_cmd_str) % 77)) )
			num_loops++;

		for (i = 0; i < num_loops; i++)
		{
			bzero(send_str,80);

			p = &slattach_cmd_str[+77*i];

			strncpy(send_str,p,77);

			write_client(OUTPUT_DATA,send_str);
		}

#ifdef DEBUG
printf("%s\n",slattach_cmd_str);
#endif

		slattach_info_fp = popen(slattach_cmd_str,"r");


		while ((fgets(read_buffer,sizeof(read_buffer),
				slattach_info_fp)) != NULL) 
		{
			if (strlen(read_buffer) > 77)
				read_buffer[77] = 0;
			write_client(OUTPUT_DATA,read_buffer); 
		}

		pclose(slattach_info_fp);

		exit();
	}

#ifdef DEBUG
printf("child pid = %d\n",slattach_fork_rtn);
#endif

	slattach_timer(tty);

	waitpid(slattach_fork_rtn,NULL,0);


#ifdef DEBUG
printf("got past the wait\n");
#endif

#ifdef DEBUG
printf("leaving slip attach subroutine\n");
#endif

        sprintf(get_slip_status,"/usr/sbin/strconf < /dev/%s | grep \"slip\" > /dev/null",tty);

	return_code = system(get_slip_status);

	if (return_code != 0)
	{

#ifdef DEBUG
printf("return code !=0 from slattach - setting tty_connected to 0 \n");
#endif
		tty_connected = 0;
	}
	else
	{

#ifdef DEBUG
printf("return code = 0 from /usr/sbin/strconf < /dev/%s - tty_connected\n",tty);
#endif
	}

return(return_code); 
}

/****************************************/
/* used to kill the process that	*/
/* reads the output from slattach  	*/
/****************************************/
void slattach_timer(char *tty)
{
	int timer_counter = 0;
	int return_code = 0;

	char	get_slip_status[MAX_LINE+1];

	bzero(get_slip_status,MAX_LINE+1);

#ifdef DEBUG
printf("inside slattach timer\n");
#endif

	/* give this connection 85 seconds, unless the default	*/
	/* had been reset by the system administrator		*/

	sprintf(get_slip_status,"/usr/sbin/strconf < /dev/%s | grep \"slip\" > /dev/null",tty);

	while (timer_counter < G_timeout) 
	{
		timer_counter = timer_counter + 5;

		sleep(5);

        	return_code = system(get_slip_status);

		if (return_code == 0)
			break;

	}
	if (return_code != 0)
	{
		if (slattach_fork_rtn > MIN_PID)
		{

#ifdef DEBUG
printf("inside slattach timer (Failed slip test) and killing process %d with SIGUSR2\n",slattach_fork_rtn);
#endif

			kill(slattach_fork_rtn,SIGUSR2);
		}
	}
	else
	{
		if (slattach_fork_rtn > MIN_PID)
		{

#ifdef DEBUG
printf("inside slattach timer and killing process %d with SIGUSR1\n",slattach_fork_rtn);
#endif

			kill(slattach_fork_rtn,SIGUSR1);
		}
		if (slattach_fork_rtn == 0)
			exit(SUCCESS);
	}
return;
} /* end of timer */


/************************************************/
/*  remove device slx - cleanup routine		*/ 
/************************************************/
int rmdev_slip(char *slip)
{
	int	rmdev_info_fp ;
	int	return_code ;
	int	slattach_pid_fd;
	int	slip_number = 0;
	int	slattach_pid_to_kill;

	char	rmdev_string[MAX_LINE+1];
	char	send_client_string[MAX_LINE+1];
	char	read_buffer[MAX_LINE+1];
	char	get_pid_str[MAX_LINE+1];
	char	slip_tmp[6];
	char	slattach_pid_str[MAX_LINE+1];
	char	*slip_ptr;

	bzero(send_client_string,MAX_LINE+1);
	bzero(rmdev_string,MAX_LINE+1);
	bzero(read_buffer,MAX_LINE+1);
	bzero(get_pid_str,MAX_LINE+1);
	bzero(slip_tmp,6);
	bzero(slattach_pid_str,MAX_LINE+1);

	strcpy(slip_tmp,slip);

	slip_ptr = slip_tmp;

	slip_ptr = slip_ptr + 2;

	slip_number = atoi(slip_ptr);

	sprintf(rmdev_string,"%s -l %s -d\n",RMDEV_CMD,slip);

	/* string looks something like		*/
	/* rmdev -l sl0 -d or slx (x < 512)	*/ 
	/* using strncpy will limit the length	*/
	/* to 76 bytes for sure (including \n)	*/
	strncpy(send_client_string,rmdev_string,76);

	write_client(OUTPUT_DATA,send_client_string);

	/* return_code = system(rmdev_string); */

	rmdev_info_fp = popen(rmdev_string,"r");

	while ((fgets(read_buffer,sizeof(read_buffer),
			rmdev_info_fp)) != NULL) 
	{
		/* output from the rmdev command should	*/
		/* be short - like "sl0 deleted", but	*/
		/* just in case we'll throw in code	*/
		/* that will protect the length limit	*/
		/* of 76 bytes sent to write_client	*/

		if (strlen(read_buffer) > 77)
			read_buffer[77] = 0;
		write_client(OUTPUT_DATA,read_buffer); 
	}

	return_code = pclose(rmdev_info_fp);

	sprintf(get_pid_str,"ps -ef | /usr/bin/grep slattach | /usr/bin/grep tty%d | /usr/bin/awk '{ print $2 }'\n",slip_number);

	if (return_code == 0)
	{
		slattach_pid_fd = popen(get_pid_str,"r");

		fgets(read_buffer,sizeof(read_buffer),slattach_pid_fd);

		pclose(slattach_pid_fd);

		strcpy(slattach_pid_str,read_buffer);

		slattach_pid_to_kill = atoi(slattach_pid_str);

		if (slattach_pid_to_kill > MIN_PID)
			kill(slattach_pid_to_kill,SIGKILL);
	}

	if (return_code != 0)
		return_code = REMDEV_SLIP_FAIL;
	else
		write_client(RETURN_CODE,KILLED_SLIP);

	return(return_code);
}

/************************************************/
/* makes the slip interface using the mkdev	*/
/* command, and returns a pointer to the 	*/
/* slip name e.g. sl0, or sl1      		*/
/************************************************/
int mkdev_slip(char *tty, char *slip_number)
{
	int	return_code;
	int	buf_ptr;
	char	mkdev_string[MAX_LINE+1];
	char	send_mkdev_string[MAX_LINE+1];
	char	read_buffer[MAX_LINE+1];
	char	slip_interface_num[4];
	char	slip_created[7];
	char	*slip_ptr;

	bzero(slip_interface_num,4);

	bzero(mkdev_string,MAX_LINE+1);

	bzero(send_mkdev_string,MAX_LINE+1);

	strcpy(slip_interface_num,tty+3);

 	sprintf(mkdev_string,"%s -c if -s SL -t sl -w sl%s -a ttyport=%s\n",
 	    			MKDEV_CMD,slip_interface_num,tty);

	return_code = 0;

	/* leave room for strlen string + 'o'	*/
	strncpy(send_mkdev_string,mkdev_string,76);

	write_client(OUTPUT_DATA,send_mkdev_string);

#ifdef DEBUG
printf("mkdev command = %s\n",mkdev_string);
#endif

	buf_ptr = popen(mkdev_string,"r");

	if (buf_ptr == NULL)
		return_code = (MKDEV_CMD_FAIL);

	fgets(read_buffer,sizeof(read_buffer),buf_ptr);

	slip_ptr = strtok(read_buffer," \n");

	sprintf(slip_created,"%s\n",slip_ptr);	/* sent to client pipe	*/

	sprintf(slip_number,"%s",slip_ptr);	/* stored in link list	*/

	if (strlen(slip_created) > 77)
		slip_created[77] = 0;
	write_client(OUTPUT_DATA,slip_created); 

	return_code = pclose(buf_ptr);

	return(return_code);
}



/*************************************************/
/* Removes a line from the /etc/uucp/Devices file*/
/*************************************************/
remove_line(char *ttyport, char *baudrate)
{
	int	return_code = 0;
	int	devices_fd;
	struct passwd	*uucp_info;
	struct flock	lock;
	char	remline_cmd[128];

	bzero(remline_cmd,128);
/*
lock the file first! then copy all lines excluding the following
Direct tty2 - 1200 direct  
*/
	sprintf(remline_cmd,"%s %s | %s '/^Direct.*%s.*%s/d' > %s",
		CAT_CMD,ETC_DEVICES,SED_CMD,ttyport,baudrate,ETC_DEVICES_TMP);

	if ((devices_fd = open(ETC_DEVICES, O_WRONLY | S_IWUSR)) < 0)
		exit(FAIL);

        lock.l_type = F_WRLCK;
        lock.l_start = 0;
        lock.l_whence = 0;
        lock.l_len = 0;

        fcntl(devices_fd, F_SETLK, &lock);

        if (errno == EACCES || errno == EAGAIN)
	{
		log_error(DEVICES_RMLINE_FAIL);
		exit(FAIL);
	}

	return_code = system(remline_cmd);

	if (return_code != 0)
	{
		log_error(DEVTMP_WRITE_FAIL);
		return(return_code);
	}

	if (unlink(ETC_DEVICES) < 0)
		log_error(ETC_DEV_UNLINK_FAIL);

	if (rename(ETC_DEVICES_TMP,ETC_DEVICES) < 0)
		log_error(ETC_DEV_RENAME_FAIL);


	uucp_info = getpwnam("uucp");

	if (chown(ETC_DEVICES,uucp_info->pw_uid,uucp_info->pw_gid) < 0)
		log_error(CHOWN_DEVICES_FAIL);

	umask(S_IRWXU | S_IRWXG| S_IRWXO);

	chmod(ETC_DEVICES,S_IRUSR | S_IRGRP | S_IROTH);

return(return_code);
}

/* end of all functions */

