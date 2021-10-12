static char sccsid[] = "@(#)12  1.14  src/bos/usr/bin/mirror/mirrord.c, cmdmirror, bos41B, 412_41B_sync 12/8/94 08:10:47";
/*
 * COMPONENT_NAME: CMDMIRROR: Console mirroring
 * 
 * FUNCTIONS:
 * 
 * ORIGINS: 83 
 * 
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information
 */



#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <sys/stropts.h>
#include <sys/mdio.h>
#include <sys/iocc.h>
#include <nl_types.h>
#include <stdarg.h>
#include <locale.h>
#define _KERNSYS
#define _RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef _RS6K_SMP_MCA
#undef _KERNSYS
#include <mirror.h>
#include "modem.h"
#include "mirrord_msg.h"


/* defines for MDIOGET ioctl to get flags from machine device driver */
#define CONTRACT_FLAGS 1
#define CONTRACT_FLAGS_SIZE 12
#define DIAG_FLAGS 2
#define DIAG_FLAGS_SIZE 28

#define SIG_KEY SIGUSR1		/* signal send when the key position changed */
#define MACHINE_DRIVER "/dev/nvram"
#define OFF 0
#define ON !OFF
#define GC_ERROR 2	/* get_carrier function return code error */
#define OK 1
#define ERRVAL 0

/* return codes for modem dialog */
#define TIMEOUT_ERROR 2
#define ABS_ERROR 3
#define IO_ERROR 4

/* defines for scan characters from modem */
#define STR_FOUND 1
#define STR_CONT 2
#define STR_NOT_CONT 3

#define PORT_S1 "s1"
#define PORT_S2 "s2"
#define MIRROR_MODULE "mirror"

/* modem script names */
#define SC_DISCONNECT "disconnect"
#define SC_CONDIN "condin"
#define SC_CONDWAIT "condwait"
#define SC_WAITCALL "waitcall"

#define TMPDIR "/tmp/"
#define LOCK_FILE "/etc/locks/LCK.."	/* indicate than mirroring is active */
#define MAX_MSG_LENGTH 255
#define MAX_USER_NAME 16
#define MAX_MODULES 8
#define BEGIN_WHO_TTY 12	/* column of the tty name in the who command */

#define MAX_DEVICE_NAME 30
#define MAX_BUFFER  256

static char log_separator [] = "----------------------------------------\n";
static char modem_parameters_path [] = "/usr/share/modems/";
static char *modem_parameters_name;
static int service_key = OFF;	/* ON if key is on service position */
static int key_change = OFF;	/* ON if key position changed */
static int mirror_mode = OFF;	/* ON if remote maintenance is active */
static int wait_conn = OFF;	/* indicate "wait connection" state */
static int receive_hangup = OFF;
static int hangup_from;
static int mirror_terminate = OFF;
static int display_st = OFF;	/* indicate display state (SIGUSR2) */

static int service_contract_validity_flag = OFF;
static int remote_service_support_flag = OFF;
static int remote_authorization_flag = OFF;

static int fd_sync [2];		/* pipe between father and child daemon */
static int ok_sync;		/* value 1 if fd_sync is ok */

static int fds1;    		/* file descriptor of tty in port S1 */
static int fds2;     		/* file descriptor of tty in port S2 */

static char *path_usr;	/* home directory of the user  */
static long begin_history_mirror;	/* offset int the .sh_history */

static nl_catd fd_cat;


static char *prnt_str (char *str);
static void echo_error (int num, const char *msg, ... )                          ;
static int get_carrier (int fd);
static int load_modem_parameters ();
static void free_modem_parameters ();
static int exist_script (char *name);
static int scan_char (char c, strlst_t *strlst, int abs);
static int scan_result (int abs, strlst_t *strlst, int timeout);
static int exec_commands (command_t *cmd);
static int exec_script (char *name);
static int modem_disconnect ();
static int wait_connection ();
static void get_hangup_message ();
static int lock_file (int cmd);
static char *find_device (char *n_port);
static char *find_home_directory (char *tty_find);
static int cfg_tty_s2 ();
static int begin_log ();
static void end_log ();
static int load_mirror_module ();
static int unload_mirror_module ();
static int install_mirror_module (int fd);
static int uninstall_mirror_module (int fd);
static int interchange_queues ();
static int active_mirror_modules ();
static int unactive_mirror_modules ();
static int contract_flag ();
static int bull_flag ();
static int user_flag ();
static int get_flags ();
static int key_service_position ();
static void display_state ();
static void signal_handler (int sig);
static int init_signal ();
static void mirroring_active (int from_boot);
static void mirroring_unactive ();
static void child_daemon ();


/*************************************************************************
 * NAME		: prnt_str
 * DESCRIPTION	: 
 * PARAMETERS	: 
 * RETURN VALUE	: 
 ************************************************************************/

char *prnt_str (char *str)
{
	char buffer [256];
	char *p;

	p = buffer;
	while (*str) {
		if (*str == '\n') {
			*p = '\\';
			p++;
			*p = 'n';
			p++;
		}
		else if (*str == '\r') {
			*p = '\\';
			p++;
			*p = 'r';
			p++;
		}
		else {
			*p = *str;
			p++;
		}
		str++;
	}
	*p = '\0';
	return buffer;
}	/* prnt_str */


/*************************************************************************
 * NAME		: echo_error   
 * DESCRIPTION	: Print an error message on console
 * PARAMETERS	: <num> is the number of error message in catalog file.
 *		<msg> is a pointer on default error message.
 *		<...> are more parameters like printf's parameters
 * RETURN VALUE	: None
 ************************************************************************/

void echo_error (int num, const char *msg, ... )                          
{
	int fd;
	char *dev_name;
	char s [MAX_MSG_LENGTH];
	FILE siop;
	va_list ap;
	char *tmp;

	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)s;
	siop._flag = (_IOWRT|_IONOFD);
	va_start (ap,msg);
	tmp = catgets (fd_cat, 1, num, msg);
	_doprnt (catgets (fd_cat, 1, num, msg), ap, &siop);
	va_end (ap);
	*siop._ptr = '\0'; /* plant terminating null character */
	if (fds1 == 0) {
		if ((dev_name = find_device (PORT_S1)) == NULL) {
			return;
		}
		if ((fd = open (dev_name, O_RDWR | O_NDELAY)) == -1) {
			return;
		}
		write (fd, "\n", strlen ("\n"));
		write (fd, s, strlen (s));
		close (fd);
		setpgrp ();
	}
	else {
		write (fds1, "\n", strlen ("\n"));
		write (fds1, s, strlen (s));
	}
}	/* echo_error */

/*************************************************************************
 * NAME		: get_carrier
 * DESCRIPTION	: test if the terminal is connected (carrier detect)
 * PARAMETERS	: <fd> is the file descriptor of the tty
 * RETURN VALUE	: ON if carrier detect, OFF else
 *		GC_ERROR if error.
 ************************************************************************/

int get_carrier (int fd)
{
	int arg;
	struct strioctl ioctl_data;

	ioctl_data.ic_dp = (char *)&arg;
	ioctl_data.ic_len = sizeof (arg);
	ioctl_data.ic_timout = 0;
	ioctl_data.ic_cmd = TIOCMGET;
	if (ioctl (fd, I_STR, &ioctl_data) <0)
	{
		echo_error (CAT_CANNOT_GET_MODEM,  "mirrord: Cannot get information from the modem : %s\n", strerror (errno));
		return GC_ERROR;
	}
	if (arg & TIOCM_CAR) {
		return ON;
	}
	else {
		return OFF;
	}
}	/* get_carrier */


/*************************************************************************
 * NAME		: yyerror
 * DESCRIPTION	: This function is call when an error is detected by yyparse
 * PARAMETERS	: <msg> is the error message from yyparse 
 * RETURN VALUE	: none
 ************************************************************************/

yyerror (char *msg)
{
	echo_error (CAT_YYERROR, "mirrord: %s: %s: Line %d\n", modem_parameters_name, msg, line_number);
}	/* yyerror */

/*************************************************************************
 * NAME		: load_modem_parameters
 * DESCRIPTION	: 
 * PARAMETERS	: 
 * RETURN VALUE	: 
 ************************************************************************/

int load_modem_parameters ()
{
	FILE *fd;

	if (( fd = fopen (modem_parameters_name, "r")) == NULL) {
		echo_error (CAT_OPEN_MPF, "mirrord: Cannot open modem parameters file %s : %s\n", modem_parameters_name, strerror (errno));
		return ERRVAL;
	}
	init_parser (fd);
	if (yyparse () != 0) {
		fclose (fd);
		return ERRVAL;
	}
	fclose (fd);
	if (icdelay == -1) {
		echo_error (CAT_ICDELAY, "mirrord: ICDelay variable must be initialized\n");
		return ERRVAL;
	}
	if (defaultto == 0) {
		echo_error (CAT_DEFAULTTO, "mirrord: DefaultTO variable must be initialized\n");
		return ERRVAL;
	}
	if ( !exist_script (SC_DISCONNECT)) {
		echo_error (CAT_SCRIPT, "mirrord: The script <%s> does not exist\n", SC_DISCONNECT);
		return ERRVAL;
	}
	if ( !exist_script (SC_CONDWAIT)) {
		echo_error (CAT_SCRIPT, "mirrord: The script <%s> does not exist\n", SC_CONDWAIT);
		return ERRVAL;
	}
	if ( !exist_script (SC_WAITCALL)) {
		echo_error (CAT_SCRIPT, "mirrord: The script <%s> does not exist\n", SC_WAITCALL);
		return ERRVAL;
	}
	return OK;
}	/* load_modem_parameters */

/*************************************************************************
 * NAME		: free_modem_parameters
 * DESCRIPTION	: 
 * PARAMETERS	: 
 * RETURN VALUE	: 
 ************************************************************************/

void free_modem_parameters ()
{
	free_all_memory ();
	icdelay = -1;
	defaultto = 0;
	line_number = 1;
}	/* free_modem_parameters */

/*************************************************************************
 * NAME		: exist_script
 * DESCRIPTION	: Check if a script name exist in the list of all scripts 
 * PARAMETERS	: <name> : name of the script
 * RETURN VALUE	: On if the script exist, OFF else
 ************************************************************************/

int exist_script (char *name)
{
	script_t *sc;

	sc = first_script;
	while (sc != NULL) {
		if ( ! strcmp (sc->name, name)) {
			return ON;
		}
		sc = sc->next;
	}
	return OFF;
}	/* exist_script */

/*************************************************************************
 * NAME		: scan_char
 * DESCRIPTION	: Find a charactere on the liste of possible results 
 * PARAMETERS	:  <c> is the character to find
 *		<strlst> is the list of possible results 
 *		<abs>  indicate if it's possible to restart at the begin
 * RETURN VALUE	: STR8FOUND if a result string is found
 *		STR_CONT if it's possible to continue to find   (abs mode)
 *		STR_NOCONT if it's imposbile to continue to find a result (abs mode)
 ************************************************************************/

int scan_char (char c, strlst_t *strlst, int abs)
{
	int found = 0;
	int cont = 0;
	strlst_t *p;

	p = strlst;
	while (p != NULL) {
		if ( !abs || p->cur_char != 0) {
			if (p->str[p->cur_char] == c) {
				p->cur_char ++;
				cont = 1;
				if (p->str[p->cur_char] == '\0') {
					found = 1;
					p->cur_char = 0;
				}
			}
			else if (p->str[0] == c) {
				p->cur_char = 1;
				cont = 1;
			}
			else {
				p->cur_char = 0;
			}
		}
		p = p->next;
	}
	if (found) {
		return STR_FOUND;
	}
	else if (cont) {
		return STR_CONT;
	}
	return STR_NOT_CONT;
}	/* scan_char */


/*************************************************************************
 * NAME		: scan_result
 * DESCRIPTION	: Find if a result from the modem correspond  to a string
 *		in the list of possible result strings 
 * PARAMETERS	: <abs> value 1 if it bust be a absolute result
 *		<strlst> is the list of possible result strings
 *		<timeout> is the value of timeout
 * RETURN VALUE	: OK if a result from the modem is found
 *		TIMEOUT_ERROR if no result is found before timeout
 *		ABS_ERROR if no absolute result is found
 *		IO_ERROR if the read subroutine failed
 ************************************************************************/

int scan_result (int abs, strlst_t *strlst, int timeout)
{
	int i;
	int first_char = 1;
	int res;
	char c;
	int ret_rd;

	i = timeout;
	if (timeout == DEFAULT_VALUE || timeout == N_VALUE)  {
		i = defaultto;
	}
	else if (timeout == NONE_VALUE) {
		i = 1;
	}
	while (i > 0) {
		if (key_change || mirror_terminate ) {
			echo_error (CAT_WAIT_STOPPED, "mirrord: Connection waiting stopped\n");
			return ERRVAL;
		}
		while ((ret_rd = read (fds2, &c, 1)) == 1) {
			if (first_char) {
				res = scan_char (c, strlst, 0);
				first_char = 0;
			}
			else {
				res = scan_char (c, strlst, abs);
			}
			if (res == STR_FOUND) {
				return OK;
			}
			else if (abs && STR_NOT_CONT) {
				return ABS_ERROR;
			}
		}
		if (ret_rd == -1) {
			return IO_ERROR;
		}
		sleep (1);
		if (timeout != NONE_VALUE) i--;
	}
	return TIMEOUT_ERROR;
}	/* scan_result */

/*************************************************************************
 * NAME		: flush_modem
 * DESCRIPTION	: flush all characters from the modem
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void flush_modem ()
{
	char c;

	while (read (fds2, &c, 1) == 1) ;
}	/* flush_modem */

/*************************************************************************
 * NAME		: exec_commands
 * DESCRIPTION	: Execute the list of modem commands
 * PARAMETERS	: <cmd> is the first command of the list of comands
 * RETURN VALUE	: OK if all commands process successful
 *		TIMEOUT_ERROR if no result is found before timeout
 *		ABS_ERROR if no absolute result is found
 *		IO_ERROR if the read or write subroutines failed
 ************************************************************************/

int exec_commands (command_t *cmd)
{
	strlst_t *strlst;
	int l;
	int ret;

	while (cmd != NULL) {
		switch (cmd->type) {
		case CMD_SEND : 
			sleep (icdelay);
			l = strlen (cmd->str);
			if (write (fds2, cmd->str, l) != l) {
				return IO_ERROR;
			}
			break;
		case CMD_EXPECT :
			strlst = cmd->strlst;
			while (strlst != NULL) {
				strlst = strlst->next;
			}
			ret = scan_result (cmd->abs, cmd->strlst, cmd->val);
			flush_modem ();
			if (ret != OK) {
				return ret;
			}
			break;
		case CMD_IGNORE :
			strlst = cmd->strlst;
			while (strlst != NULL) {
				strlst = strlst->next;
			}
			if (scan_result (0, cmd->strlst, cmd->val) == IO_ERROR) {
				return IO_ERROR;
			}
			flush_modem ();
			break;
		case CMD_DELAY :
			sleep (cmd->val);
			break;
		default : ;
			return ERRVAL;
		}
		cmd = cmd->next;
	}
	return OK;
}	/* exec_commands */

/*************************************************************************
 * NAME		: exec_script
 * DESCRIPTION	: Execute a script to control the modem
 * PARAMETERS	:  <name> is the name of the script
 * RETURN VALUE	: OK if the script proccess successful
 *		TIMEOUT_ERROR if no result is found before timeout
 *		ABS_ERROR if no absolute result is found
 *		IO_ERROR if the read or write subroutines failed
 *		ERRVAL if the script is not found
 ************************************************************************/

int exec_script (char *name)
{
	script_t *sc;
	int res;

	sc = first_script;
	while (sc != NULL) {
		if ( ! strcmp (sc->name, name)) {
			res = exec_commands (sc->cmd);
			switch (res) {
			case OK :
				break;
			case TIMEOUT_ERROR :
				echo_error (CAT_MODEM_TIMEOUT, "mirrord: Modem dialog timeout\n");
				break;
			case ABS_ERROR :
				echo_error (CAT_ABS_ERROR, "mirrord: Modem dialog error\n");
				break;
			case IO_ERROR :
				echo_error (CAT_MODEM_ERROR, "mirrord: Modem dialog error : %s\n", strerror (errno));
				break;
			}
			return res;
		}
		sc = sc->next;
	}
	return ERRVAL;
}	/* exec_script */

/*************************************************************************
 * NAME		: modem_disconnect
 * DESCRIPTION	: Disconnect the modem
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int modem_disconnect ()
{

	if (exec_script ("disconnect") != OK) {
		return ERRVAL;
	}
	else {
		return OK;
	}
}	/* modem_disconnect */

/*************************************************************************
 * NAME		: wait_connection 
 * DESCRIPTION	: Wait a successful connection from the remote maintenance
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successul connection, ERRVAL else
 ************************************************************************/

int wait_connection ()
{
	int res;

	wait_conn = ON;
	if (exec_script (SC_CONDIN) != OK) {
		wait_conn = OFF;
		return ERRVAL;
	}
	echo_error  (CAT_WAIT_CALL, "mirrord: Wait connection ...\n");
	for (;;) {
		if (key_change || mirror_terminate) {
			echo_error (CAT_WAIT_STOPPED, "mirrord: Connection waiting stopped\n");
			wait_conn = OFF;
			return ERRVAL;
		}
		res = get_carrier (fds2);
		if (res == ON) break;
		if (res == GC_ERROR) {
			wait_conn == OFF;
			return ERRVAL;
		}
		sleep (1);
	}
	wait_conn = OFF;
	return OK;
}	/* wait_connection */

/*************************************************************************
 * NAME		: get_hangup_message
 * DESCRIPTION	: get the hangup message from the mirror module
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void get_hangup_message ()
{
	struct pollfd pollfds;
	struct strbuf ctlbuf;
	int flags;
	char line[256];

	pollfds.fd = fds2;
	pollfds.events = POLLPRI;
	if (poll (&pollfds, 1, -1) < 0) {
		echo_error (CAT_POLL, "mirrord: Poll error : %s\n", strerror (errno));
		return;
	}
	if (pollfds.revents != POLLPRI) {
		return;
	}
	ctlbuf.maxlen = strlen (M_HANGUP_S1) + 1;
	ctlbuf.len = 0;
	ctlbuf.buf = line;
	flags = RS_HIPRI;
	if (getmsg (fds2, &ctlbuf, NULL, &flags) == -1) {
		return;
	}
	if (! strcmp (line, M_HANGUP_S1)) {
		receive_hangup = ON;
		hangup_from = 1;
	}
	else if (! strcmp (line, M_HANGUP_S2)) {
		receive_hangup = ON;
		hangup_from = 2;
	}
	return;
}	/* get_hangup_message */

/*************************************************************************
 * NAME		: lock_file
 * DESCRIPTION	: set or remove the lock file "mirror" in /etc/locks directory
 * PARAMETERS	: <cmd> value ON to set the file or OFF to remove the file
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int lock_file (int cmd)
{
	int fd;
	char *dev_name;
	char buffer [256];

	if ((dev_name = find_device (PORT_S2)) == NULL) {
		return ERRVAL;
	}
	if (cmd == ON) {
		sprintf (buffer, "%s%s", LOCK_FILE, dev_name+5);
		if ((fd = open (buffer, O_RDWR | O_CREAT | O_TRUNC, 0660)) == -1) {
			echo_error (CAT_CANNOT_OPEN_LOCK_FILE, "mirrord: Cannot open lock file %s : %s\n", buffer, strerror (errno));
			return ERRVAL;
		}
		sprintf (buffer, "    %d", getpid ());
		write (fd, buffer, strlen (buffer));
		close (fd);
	}
	else {	/* cmd == OFF */
		sprintf (buffer, "rm -f %s%s", LOCK_FILE, dev_name+5);
		system (buffer);
	}
	return OK;
}	/* lock_file */

/*************************************************************************
 * NAME		: find_device
 * DESCRIPTION	: find the name of the device connected to a port 
 * PARAMETERS	: <n_port> is a pointer on the sring name of the port
 * RETURN VALUE	:  - a pointer on the device name
 *		   - NULL if the device is not found or not available
 ************************************************************************/

char *find_device (char *n_port)
{
	static char device_name [MAX_DEVICE_NAME];
	int rc;
	char criteria [256];
	struct CuDv CuDv;

	odm_initialize ();
	sprintf (criteria, "connwhere=%sa AND PdDvLn LIKE adapter/sio/*", n_port);
	rc = (int) odm_get_first(CuDv_CLASS, criteria, &CuDv);
	if (rc == -1) {
		echo_error (CAT_ODM_GET, "mirrord: Cannot get information from ODM : %s\n", strerror (errno));
		odm_terminate ();
		return NULL;
	} 
	sprintf (criteria, "parent=%s", CuDv.name);
	rc = (int) odm_get_first(CuDv_CLASS, criteria, &CuDv);
	if (rc == -1) {
		echo_error (CAT_ODM_GET, "mirrord: Cannot get information from ODM : %s\n", strerror (errno));
		odm_terminate ();
		return NULL;
	}
	strcpy (device_name, "/dev/");
	strcat (device_name, CuDv.name);
	odm_terminate ();
	return device_name;
}	/* find_device */

/*************************************************************************
 * NAME		: find_home_directory
 * DESCRIPTION	: find the home directory of a user working to a terminal
 * PARAMETERS	: <tty_find> is the name of the terminal
 * RETURN VALUE	: A pointer on the name of the home directory
 *		NULL if not found.
 ************************************************************************/

char *find_home_directory (char *tty_find)
{
	FILE *fp;
	FILE *fd;
	static char buffer [MAX_BUFFER];
	char user_name [MAX_USER_NAME];
	int found = 0;
	char *p;
	int i;
	int l;

	if ((fp = popen ("who", "r")) == NULL){
		echo_error (CAT_WHO, "mirrord: Cannot execute the who command : %s\n", strerror (errno));
		return NULL;
	}
	found = 0;
	while (fgets (buffer, MAX_BUFFER, fp) != NULL)  {
		if (strlen (buffer) <= BEGIN_WHO_TTY) continue;
		if ((p = strtok (buffer + BEGIN_WHO_TTY, " ")) == NULL) continue;
		if ( strcmp (p, tty_find)) continue;
		if ((p = strtok (buffer, " ")) == NULL) continue;
		strcpy (user_name, p);
		found = 1;
		break;
	}	/* end while */
	pclose (fp);
	if ( !found) {
		return NULL;
	}
	if ((fd = fopen ("/etc/passwd", "r")) == NULL) {
		echo_error (CAT_PASSWD, "mirrord: Cannot open file /etc/passwd : %s\n", strerror (errno));
		return NULL;
	}
	found = 0;
	while (fgets (buffer, MAX_BUFFER, fd) != NULL) {
		if ((p = strtok (buffer, ":")) == NULL) continue;
		if ( !strcmp (p, user_name)) {
			found = 1;
			break;
		}
	}
	fclose (fd);
	if ( !found) {
		return NULL;
	}
	for (i=1; i<5; i++) {
		if ((p = strtok (NULL, ":")) == NULL) {
			return NULL;
		}
	}
	l = strlen (p);
	if (l != 1) {
		p[l] = '/';
		p[l+1] = '\0';
	}
	return p;
}	/* find_home_directory */


/*************************************************************************
 * NAME		: cfg_tty_s2
 * DESCRIPTION	: configuration of tty(S2) with 
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int cfg_tty_s2 ()
{
	struct termios termios;

	if (tcgetattr (fds2, &termios) == -1) {
		return ERRVAL;
	}
	termios.c_cflag |= CLOCAL;
	termios.c_cflag &= ~HUPCL;
	termios.c_lflag &= ~ECHO;
	termios.c_iflag &= ~ICRNL;
	termios.c_iflag |= IXON;
	termios.c_iflag |= IXOFF;
	termios.c_iflag &= ~IXANY;
	termios.c_lflag &= ~ISIG;
	termios.c_lflag &= ~ICANON;
	termios.c_oflag &= ~OPOST;
	termios.c_cc[VEOF] = (cc_t)1;
	termios.c_cc[VEOL] = (cc_t)1;
	if (tcsetattr (fds2, TCSANOW, &termios) == -1) {
		return ERRVAL;
	}
	return OK;
}	/* cfg_tty_s2 */

/*************************************************************************
 * NAME		: begin_log
 * DESCRIPTION	: begin the trace of shell commands in hitory file
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

int begin_log ()
{
	int fd;
	char buffer [MAX_BUFFER];
	char *tty;
	char *home_directory;

	tty = find_device (PORT_S1);
	tty += 5;	/* cut the "/dev/" prefix */
	if ((home_directory = find_home_directory (tty)) == NULL) {
		return ERRVAL;
	}
	path_usr = strdup (home_directory);
	sprintf (buffer, "%s.sh_history", path_usr);
	if ((fd = open (buffer, O_RDONLY)) == -1) {
		return ERRVAL;
	}
	begin_history_mirror = lseek (fd, 0, SEEK_END);
	close (fd);
	return OK;
}	/* begin_log */

/*************************************************************************
 * NAME		: end_log
 * DESCRIPTION	: terminate trace of shell commands in history file 
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void end_log ()
{
	int fd1;
	int fd2;
	char buffer [MAX_BUFFER];
	int nb_rd;

	sprintf (buffer, "%s.sh_history", path_usr);
	if ((fd1 = open (buffer, O_RDONLY)) == -1) {
		return;
	}
	sprintf (buffer, "%s.sh_history.log", path_usr);
	if ((fd2 = open (buffer, O_RDWR | O_CREAT, 0660)) == -1) {
		close (fd1);
		return;
	}
	lseek (fd1, begin_history_mirror, SEEK_SET);
	lseek (fd2, 0, SEEK_END);
	write (fd2, log_separator, strlen (log_separator));
	while ((nb_rd = read (fd1, buffer, MAX_BUFFER)) == MAX_BUFFER) {
		write (fd2, buffer, nb_rd);
	}
	write (fd2, buffer, nb_rd);
	close (fd2);
	close (fd1);
}	/* end_log */


/*************************************************************************
 * NAME		: load_mirror_module
 * DESCRIPTION	: Load the mirror module
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int load_mirror_module ()
{
	if (system ("strload -q -m " MIRROR_MODULE "| grep yes >/dev/null") == 0) {
		return OK;
	}
	if (system ("strload -m " MIRROR_MODULE) != 0) {
		echo_error (CAT_CANNOT_STRLOAD, "mirrord: Cannot load mirror module \n");
		return ERRVAL;
	}
	return OK;
}	/* load_mirror_module */

/*************************************************************************
 * NAME		: unload_mirror_module
 * DESCRIPTION	: Unload the mirror module 
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int unload_mirror_module ()
{
	if (system ("strload -u -m " MIRROR_MODULE " >/dev/null") != 0) {
		echo_error (CAT_CANNOT_STRUNLOAD, "mirrord: Cannot unload mirror module\n");
		return ERRVAL;
	}
	return OK;
}	/* unload_mirror_module */

/*************************************************************************
 * NAME		: install_mirror_module
 * DESCRIPTION	: push the mirror module just above the driver of the stream
 * PARAMETERS	: <fd> is the file descriptor of the stream 
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int install_mirror_module (int fd)
{
	int i;
	struct termios termios;
	struct str_list lst;
	int n_modules;
	int retval = OK;

	if (tcgetattr (fd, &termios) == -1) {
		echo_error (CAT_GETATTR, "mirrord: Cannot get attributs : %s\n", strerror (errno));
		return ERRVAL;
	}
	if (ioctl (fd, I_FIND, MIRROR_MODULE) == 1)
	{
		return OK;
	}
	lst.sl_nmods = MAX_MODULES;
	lst.sl_modlist = (struct str_mlist*) malloc (MAX_MODULES * 
		sizeof (struct str_mlist));
	if (lst.sl_modlist == NULL)
	{
		echo_error (CAT_NO_MEMORY, "mirrord: No place for memory\n");
		return ERRVAL; 
	}
	if (( n_modules = ioctl (fd, I_LIST, &lst)) <0)
	{
		echo_error (CAT_CANNOT_GET_LIST, "mirrord: Cannot get list of modules : %s\n",
			strerror (errno)); 
		return ERRVAL;
	}
	for (i=1; i<n_modules; i++)  /* pop all modules in the stream */
	{
		if (ioctl (fd, I_POP, 0) <0)
		{
			echo_error (CAT_CANNOT_POP, "mirrord: Cannot pop module : %s\n",
				strerror (errno)); 
			return ERRVAL;
		}
	}
	if (ioctl (fd, I_PUSH, MIRROR_MODULE) <0)
	{
		echo_error (CAT_CANNOT_PUSH_MIRROR, "mirrord: Cannot push mirror module : %s\n",
			strerror (errno));
		/* try to push poped modules before return */
		retval = ERRVAL;
	}
	for (i=n_modules-2; i>=0; i--)  /* push all modules poped */
	{
		if (ioctl (fd, I_PUSH, lst.sl_modlist[i].l_name) <0)
		{
			echo_error (CAT_CANNOT_PUSH_POPED, "mirrord: Cannot push poped modules : %s\n",
				strerror (errno));
			return ERRVAL;
		}
	}
	if (tcsetattr (fd, TCSANOW, &termios) == -1) {
		echo_error (CAT_SETATTR, "mirrord: Cannot set attributs : %s\n", strerror (errno));
		return ERRVAL;
	}
	return retval;
}	/* install_mirror_module */

/*************************************************************************
 * NAME		: uninstall_mirror_module
 * DESCRIPTION	: pop the mirror module from a stream 
 * PARAMETERS	: <fd> is the file descriptor of the stream 
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int uninstall_mirror_module (int fd)
{
	int i;
	struct str_list lst;
	int n_modules;
	struct termios termios;

	if (tcgetattr (fd, &termios) == -1) {
		echo_error (CAT_GETATTR, "mirrord: Cannot get attributs : %s\n", strerror (errno));
		return ERRVAL;
	}
	lst.sl_nmods = MAX_MODULES;
	lst.sl_modlist = (struct str_mlist*) malloc (MAX_MODULES * 
		sizeof (struct str_mlist));
	if (lst.sl_modlist == NULL)
	{
		echo_error (CAT_NO_MEMORY, "mirrord: No place for memory\n");
		return ERRVAL; 
	}
	if (( n_modules = ioctl (fd, I_LIST, &lst)) <0)
	{
		echo_error (CAT_CANNOT_GET_LIST, "mirrord: Cannot get list of modules : %s\n",
			strerror (errno)); 
		return ERRVAL;
	}
	if (strcmp (MIRROR_MODULE, lst.sl_modlist[n_modules - 2].l_name)) {
		return OK;
	}
	for (i=1; i<n_modules; i++)
	{
		if (ioctl (fd, I_POP, 0) <0)
		{
			echo_error (CAT_CANNOT_POP, "mirrord: Cannot pop module : %s\n",
				strerror (errno));
			return ERRVAL;
		}
	}
	for (i=n_modules- 3; i>=0; i--)
	{
		if (ioctl (fd, I_PUSH, lst.sl_modlist[i].l_name) <0)
		{
			echo_error (CAT_CANNOT_PUSH_POPED, "mirrord: Cannot push poped modules : %s\n",
				strerror (errno));
			return ERRVAL;
		}
	}
	if (tcsetattr (fd, TCSANOW, &termios) == -1) {
		echo_error (CAT_SETATTR, "mirrord: Cannot set attributs : %s\n", strerror (errno));
		return ERRVAL;
	}
	return OK;
}	/* uninstall_mirror_module */

/*************************************************************************
 * NAME		: interchange_queues
 * DESCRIPTION	: Active interchange of write queues of two mirror modules 
 * PARAMETERS	: none
 * RETURN VALUE	: OK if sccessful, ERRVAL else
 ************************************************************************/

int interchange_queues ()
{
	struct strioctl ioctl_data;

	ioctl_data.ic_dp = NULL;
	ioctl_data.ic_len = 0;
	ioctl_data.ic_timout = 0;
	ioctl_data.ic_cmd = MIR_PUT_WQ1;
	if (ioctl (fds1, I_STR, &ioctl_data) <0)
	{
		echo_error (CAT_CANNOT_INTERCHANGE, "mirrord: Cannot interchange write queues : %s\n", strerror (errno));
		return ERRVAL;
	}
	ioctl_data.ic_cmd = MIR_PUT_WQ2;
	if (ioctl (fds2, I_STR, &ioctl_data) <0)
	{
		echo_error (CAT_CANNOT_INTERCHANGE, "mirrord: Cannot interchange write queues : %s\n", strerror (errno));
		return ERRVAL;
	}
	return OK;
}	/* interchange_queues */

/*************************************************************************
 * NAME		: active_mirror_modules 
 * DESCRIPTION	: Active mirroring with ioctl MIR_ON_ECHO;
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int active_mirror_modules ()
{
	struct strioctl ioctl_data;

	ioctl_data.ic_len = 0;
	ioctl_data.ic_dp = NULL;
	ioctl_data.ic_timout = 0;
	ioctl_data.ic_cmd = MIR_ON_ECHO;
	if (ioctl (fds1, I_STR, &ioctl_data) <0)
	{
		echo_error (CAT_CANNOT_ACTIVATE, "mirrord: Cannot activate mirror module : %s\n",
			strerror (errno));
		return ERRVAL;
	}
	return OK;
}	/* active_mirror_modules */

/*************************************************************************
 * NAME		: unactive_mirror_modules
 * DESCRIPTION	: send Unactive mirroring with ioctl MIR_OFF_ECHO
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int unactive_mirror_modules ()
{
	struct strioctl ioctl_data;

	ioctl_data.ic_len = 0;
	ioctl_data.ic_dp = NULL;
	ioctl_data.ic_timout = 0;
	ioctl_data.ic_cmd = MIR_OFF_ECHO;
	if (ioctl (fds2, I_STR, &ioctl_data) <0)
	{
		echo_error (CAT_CANNOT_UNACTIVATE, "mirrord: Cannot unactivate mirror module : %s\n",
			strerror (errno));
		return ERRVAL;
	}
	return OK;
}	/* unactive_mirror_modules */


/*************************************************************************
 * NAME		: contract_flag
 * DESCRIPTION	: return the position of maintenance contract flag
 * PARAMETERS	: none
 * RETURN VALUE	: ON if flag is set, OFF else 
 ************************************************************************/

int contract_flag ()
{
	int fd;
	fd = open ("/usr/mnt/contract_flag", O_RDWR);
	close (fd);
	if (fd == -1) return OFF;
	return ON;
}	/* contract_flag */

/*************************************************************************
 * NAME		: bull_flag
 * DESCRIPTION	: return the position of bull remote maintenance flag
 * PARAMETERS	: none
 * RETURN VALUE	: ON if flag is set, OFF else
 ************************************************************************/

int bull_flag ()
{
	int fd;

	fd = open ("/usr/mnt/bull_flag", O_RDWR);
	close (fd);
	if (fd == -1) return OFF;
	return ON;
}	/* bull_flag */

/*************************************************************************
 * NAME		: user_flag
 * DESCRIPTION	: return the position of user flag
 * PARAMETERS	: none
 * RETURN VALUE	: ON if the flag is set, OFF else
 ************************************************************************/

int user_flag ()
{
	int fd;
	if (( fd = open ("/usr/mnt/user_flag", O_RDWR)) == -1)
		return OFF;
	close (fd);
	return ON;
}	/* user_flag */


/*************************************************************************
 * NAME		: get_flags
 * DESCRIPTION	: Get the value of the flags from nvram 
 * PARAMETERS	: none
 * RETURN VALUE	: OK if success, ERRVAL else
 ************************************************************************/

int get_flags ()
{
	MACH_DD_IO mdd;
	int f;
	char buffer [MAX_BUFFER];
	ulong tranfered;
	char rssf;	/* remote service suport flag */
	short cvf;	/* contract validation flag */
	char raf;	/* remote authorization flag */

	remote_service_support_flag = OFF;
	service_contract_validity_flag = OFF;
	remote_authorization_flag = OFF;
	if ((f = open (MACHINE_DRIVER, O_RDWR)) == -1) {
		echo_error (CAT_GET_FLAGS, "mirrord: Cannot get the value of contract flags : %s\n", strerror (errno));
		return ERRVAL;
	}
	mdd.md_data = buffer;
	mdd.md_size = CONTRACT_FLAGS_SIZE;
	mdd.md_incr = MV_BYTE;
	mdd.md_type = CONTRACT_FLAGS;
	mdd.md_length = &tranfered;
	if (ioctl (f, MDINFOGET, &mdd) == -1) {
		echo_error (CAT_GET_FLAGS, "mirrord: Cannot get the value of contract flags : %s\n", strerror (errno));
		close (f);
		return ERRVAL;
	}
	rssf = *(buffer + 1);
	cvf = *((short *) (buffer + 2));
	mdd.md_type = DIAG_FLAGS;
	mdd.md_size = DIAG_FLAGS_SIZE;
	if (ioctl (f, MDINFOGET, &mdd) == -1) {
		echo_error (CAT_GET_FLAGS, "mirrord: Cannot get the value of contract flags : %s\n", strerror (errno));
		close (f);
		return ERRVAL;
	}
	raf = *buffer;
	close (f);
	if (rssf & 1) remote_service_support_flag = ON;
	if (cvf >= 0) service_contract_validity_flag = ON;
	if (raf & 1) remote_authorization_flag = ON;
	return OK;
}	/* get_flags */

/*************************************************************************
 * NAME		: key_service_position
 * DESCRIPTION	: check if the key is on "service" position  
 * PARAMETERS	: none
 * RETURN VALUE	: ON if key is on "service" position, OFF else  
 ************************************************************************/

int key_service_position ()
{
	MACH_DD_IO mdd;
	int key_pos = 0;
	int f;

	if ((f = open (MACHINE_DRIVER, O_RDWR)) == -1) {
		echo_error (CAT_CANNOT_GET_KEY, "mirrord: Cannot get the key position : %s\n", strerror (errno));
		return OFF;
	}
	mdd.md_size = 1;
	mdd.md_incr = MV_WORD;
	mdd.md_data = (char *)&key_pos;
	if (ioctl (f, MIOGETKEY, &mdd) != 0) {
		echo_error (CAT_CANNOT_GET_KEY, "mirrord: Cannot get the key position : %s\n", strerror (errno));
		close (f);
		return OFF;
	}
	close (f);
	if ((key_pos & KEY_POS_MASK) == KEY_POS_SERVICE) {
		return ON;
	}
	return OFF;
}	/* key_service_position */

/*************************************************************************
 * NAME		: display_state
 * DESCRIPTION	: display on the console the current state of the daemon
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void display_state ()
{
	if (mirror_mode) {
		echo_error(CAT_MIRROR_ON, "mirrord: Remote user connected, mirroring active.\n");
	}
	else if (wait_conn == ON) {
		echo_error  (CAT_WAIT_CALL, "mirrord: Wait connection ...\n");
	}
	else {
		echo_error(CAT_MIRROR_OFF, "mirrord: Mirroring is stopped.\n");
	}
}	/* display_state */

/*************************************************************************
 * NAME		: signal_handler
 * DESCRIPTION	: this function is the signal handler for the SIG_KEY signal
 * PARAMETERS	: <sig> is the signal receive
 * RETURN VALUE	: none
 ************************************************************************/

void signal_handler (int sig)
{
	switch (sig) {
	case SIG_KEY :
		signal (SIG_KEY, signal_handler);
		sighold (SIG_KEY);
		service_key = key_service_position ();
		key_change = ON;
		sigrelse (SIG_KEY);
		break;
	case SIGPOLL :
		signal (SIGPOLL, signal_handler);
		sighold (SIGPOLL);
		get_hangup_message ();
		sigrelse (SIGPOLL);
		break;
	case SIGHUP :
		signal (SIGHUP, signal_handler);
		break;
	case SIGUSR2 : 
		display_state ();
		display_st = ON;
		signal (SIGUSR2, signal_handler);
		break;
	case SIGTERM : 
		signal (SIGTERM, signal_handler);
		mirror_terminate = ON;
		break;
	}
}	/* signal_handler */

/*************************************************************************
 * NAME		: init_signal
 * DESCRIPTION	: initialize the reception of SIG_KEY and SIGPOLL signals
 * PARAMETERS	: none
 * RETURN VALUE	: OK if successful, ERRVAL else
 ************************************************************************/

int init_signal ()
{
	int f;

	signal (SIGUSR2, signal_handler);
	signal (SIGTERM, signal_handler);
	signal (SIG_KEY, signal_handler);
	signal (SIGPOLL, signal_handler); 
	signal (SIGHUP, signal_handler); 
	if ((f = open (MACHINE_DRIVER, O_RDWR)) == -1) {
		echo_error (CAT_CANNOT_CONNECT, "mirrord: Cannot connect to the key : %s\n", strerror (errno));
		return ERRVAL;
	}
	if (ioctl (f, MIOKEYCONNECT, SIG_KEY) != 0) {
		if (errno == 22) {  /* it's not a PEGASUS machine */
			exit (1);
		}
		echo_error (CAT_CANNOT_CONNECT, "mirrord: Cannot connect to the key : %s\n", strerror (errno));
		close (f);
		return ERRVAL;
	}
	close (f);
	return OK;
}	/* init_signal */


/*************************************************************************
 * NAME		: login_s2
 * DESCRIPTION	:  enable or disable the login on tty(s2)
 * PARAMETERS	: <cmd> = ON/OFF
 * RETURN VALUE	: none
 ************************************************************************/

void login_s2 (int cmd)
{
	FILE *fd;
	char *p;
	static int respawn = OFF;
	static share = OFF;
	char *dev_name;
	char buffer [MAX_BUFFER];

	if ((dev_name = find_device (PORT_S2)) == NULL) {
		return;
	}
	switch (cmd) {
	case OFF :
		if ((fd = fopen ("/etc/inittab", "r")) == NULL) {
			return;
		}
		while (fgets (buffer, MAX_BUFFER, fd) != NULL) {
			if ((p = strtok (buffer, ":")) == NULL) continue;
			if (strcmp (p, dev_name+5)) continue;
			if ((p = strtok (NULL, ":")) == NULL) continue;
			if ((p = strtok (NULL, ":")) == NULL) continue;
			if (strcmp (p, "respawn")) continue;
			respawn = ON;
			if ((p = strtok (NULL, ":")) != NULL) {
				if (strstr (p, "-u") != NULL) {
					share = ON;
					lock_file (ON);
				}
			}
			sprintf (buffer, "pdisable %s >/dev/null", dev_name+5);
			system (buffer);
			break;
		}
		break;
	case ON :
		if (respawn == ON) {
			if (share == ON) {
				sprintf (buffer, "pshare %s >/dev/null", dev_name+5);
				lock_file (OFF);
			}
			else {
				sprintf (buffer, "penable %s >/dev/null", dev_name+5);
			}
			system (buffer);
		}
	}
}	/* login_s2 */

/*************************************************************************
 * NAME		: mirroring_active
 * DESCRIPTION	: start the console mirroring
 * PARAMETERS	: <from_boot> is set to ON if mirroring start on boot 
 * RETURN VALUE	: none
 ************************************************************************/

void mirroring_active (int from_boot)
{
	char *dev_name;

	get_flags ();
	if ( ! (service_contract_validity_flag && remote_service_support_flag )) {
		echo_error (CAT_CONTRACT_FLAG, "mirrord: Remote maintenance contract is no more valid, mirrord exited \n");
		exit (1);
	}
	if ( !remote_authorization_flag)  {
		echo_error (CAT_USER_FLAG, "mirrord: Flag user not set, mirroring remote maintenance failed\n"); 
		return;
	}
	if (load_modem_parameters () != OK) {
		goto finish_0;
	}
	login_s2 (OFF);
	if ((dev_name = find_device (PORT_S2)) == NULL) {
		goto finish_1;
	}
	if ((fds2 = open (dev_name, O_RDWR | O_NDELAY)) == -1) {
		echo_error (CAT_CANNOT_OPEN_S2, "mirrord: Cannot open %s : %s\n",
			dev_name, strerror (errno));
		goto finish_1;
	}
	ioctl (fds2, TIOCEXCL, 0) ;
	if ((dev_name = find_device (PORT_S1)) == NULL) {
		goto finish_2;
	}
	if ((fds1 = open (dev_name, O_RDWR | O_NDELAY)) == -1) {
		echo_error (CAT_CANNOT_OPEN_CONSOLE, "mirrord: Cannot open %s : %s\n",
			dev_name, strerror (errno));
		goto finish_2;
	}
	if ( get_carrier (fds1) != ON) {
		goto finish_3;
	}
	if (frevoke (fds2) == -1) {
		echo_error (CAT_FREVOKE, "mirrord: Cannot revoke process on %s : %s\n",
			dev_name, strerror (errno));
		goto finish_3;
	}
	if (cfg_tty_s2 () != OK) 
		goto finish_3;
	if (ioctl (fds2, I_SETSIG, S_HIPRI) == -1) {
		echo_error (CAT_SIGPOLL, "mirrord: Cannot set SIGPOLL signal : %s\n", strerror (errno));
		goto finish_3;
	}
	if ( ! (from_boot && get_carrier (fds2)) == ON) {
		if (modem_disconnect () != OK) {
			goto finish_3;
		}
		if (wait_connection () != OK)  {
			goto finish_3;
		}
	}	/* if  ! from_boot }} get_carrier */
	if (load_mirror_module () != OK) {
		/* goto finish_4; */
	}
	if (install_mirror_module (fds1) != OK) {
		goto finish_4;
	}
	if (install_mirror_module (fds2) != OK) {
		goto finish_5;
	}
	if (interchange_queues () != OK) {
		goto finish_6;
	}
	if (active_mirror_modules () != OK) {
		goto finish_6;
	}
	if (begin_log ()!= OK) {
		echo_error (CAT_LOG,"mirrord: Warning, no log during the mirroring \n");
	}
	echo_error(CAT_MIRROR_ON, "mirrord: Remote user connected, mirroring active.\n");
	mirror_mode = ON;
	if (ok_sync) {
		write (fd_sync[1], " ", 1);
	}
	close (fds1);
	fds1 = 0;
	setpgrp ();
	return;
finish_6:
	uninstall_mirror_module (fds2);
finish_5:
	uninstall_mirror_module (fds1);
finish_4:
	unload_mirror_module ();
finish_3:
	close (fds1);
finish_2:
	close (fds2);
	setpgrp ();
finish_1:
	free_modem_parameters ();
	login_s2 (ON);
finish_0:
	mirror_mode = OFF;
	fds1 = 0;
}	/* mirroring_active */

/*************************************************************************
 * NAME		: mirroring_unactive
 * DESCRIPTION	: terminate the console mirroring
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void mirroring_unactive ()
{
	char *dev_name;
	char buffer[MAX_BUFFER];

	unactive_mirror_modules ();
	end_log ();
	if ((dev_name = find_device (PORT_S1)) == NULL) {
		return;
	}
	if ((fds1 = open (dev_name, O_RDWR | O_NDELAY)) == -1) {
		echo_error (CAT_CANNOT_OPEN_CONSOLE, "mirrord: Cannot open %s : %s\n",
			dev_name, strerror (errno));
		return;
	}
	ioctl (fds2, TIOCFLUSH, 0);
	uninstall_mirror_module (fds1);
	uninstall_mirror_module (fds2);
	if ( !(mirror_terminate || (receive_hangup && (hangup_from == 2)))) {
		modem_disconnect ();
	}
	free_modem_parameters ();
	close (fds2);
	close (fds1);
	fds1 = 0;
	setpgrp ();
	unload_mirror_module ();
	login_s2 (ON);
	echo_error(CAT_MIRROR_OFF, "mirrord: Mirroring is stopped.\n");
	mirror_mode = OFF;
}	/* mirroring_unactive */

/*************************************************************************
 * NAME		: child_daemon
 * DESCRIPTION	: main function of the process daemon
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void child_daemon ()
{
	char buffer[256];

	if (setpgrp () == -1) {
		echo_error (CAT_SETPGRP, "mirrord: Setpgrp error : %s\n", strerror (errno));
		exit (1);
	}
	if (init_signal () != OK)
	{
		exit (1);
	}
	key_change = OFF;
	mirror_mode = OFF;
	sighold (SIGUSR1);
	if (key_service_position ()) 
	{
		service_key = ON;
	}
	sigrelse (SIGUSR1);
	if (service_key)
	{
		key_change = OFF;
		mirroring_active (ON);
	}

	for (;;)
	{
		if ( !(key_change || receive_hangup || mirror_terminate))
		{
			pause ();
			if (display_st == ON) {
				display_st = OFF;
				continue;
			}
		}
		key_change = OFF;
		if (mirror_terminate) {
			if (mirror_mode) {
				mirroring_unactive ();
			}
			exit (0);
		}
		if (receive_hangup && mirror_mode) {
			receive_hangup = OFF;
			mirroring_unactive ();
			/* and restart mirroring for the next connection */
			mirroring_active (OFF);
		}
		else if (service_key && !mirror_mode) {
			mirroring_active (OFF);
		}
		else if (!service_key && mirror_mode) {
			mirroring_unactive ();
		}
		receive_hangup = OFF;
	}
}	/* child_daemon */

/*************************************************************************
 * NAME		: main
 * DESCRIPTION	: check maintenance flags and create a child process
 * PARAMETERS	: Parameter on command line
 * RETURN VALUE	: 0 if successful, 1 else
 ************************************************************************/

int main (int argc, char **argv)
{
	pid_t pid;
	char buffer[256];

	if ( ! __rs6k_smp_mca ()) exit (0);

	setlocale(LC_ALL, "");
	fd_cat = catopen (MF_MIRRORD, NL_CAT_LOCALE);

	get_flags ();
	if ( !(service_contract_validity_flag && remote_service_support_flag))  {
		exit (1);
	}

	sprintf(buffer,"ps -eko \"pid comm\" | grep mirrord | grep -v %d >/dev/null", getpid());
	if (system (buffer) == 0) {
		echo_error (CAT_ACTIVE, "mirrord: mirror daemon already active \n");
		exit (1);
	}

	if (argc < 2) {
		echo_error (CAT_NO_PARAMETER, "mirrord: You must specify the name of the modem parameters file\n");
		exit (1);
	}
	if (argc > 2) {
		echo_error (CAT_MANY_PARAMETERS, "mirrord: Too many parameters, you must only specify the name of the modem parameters file\n");
		exit (1);
	}
	if (*argv[1] == '/') {
		modem_parameters_name = argv[1];
	}
	else {
		modem_parameters_name = malloc (strlen (modem_parameters_path) + strlen (argv[1]) + 1);
		strcpy (modem_parameters_name, modem_parameters_path);
		strcat (modem_parameters_name, argv[1]);
	}

	if (pipe (fd_sync) != -1) {
		if (fcntl (fd_sync[0], F_SETFL, O_RDONLY | O_NDELAY) != -1) {
			ok_sync = 1;
		}
	}
	pid = fork ();
	switch (pid) {
	case 0 :
		child_daemon ();
		break;
	case -1 :
		perror ("fork error ");
		exit (1);
	default :
		if (ok_sync && (key_service_position () == ON)) {
			int i;
			for (i=0; i<6; i++) {
				if (read (fd_sync[0], &buffer, 1) == 1) break;
				sleep (1);
			}
		}
		exit (0);
	}
}	/* main */

