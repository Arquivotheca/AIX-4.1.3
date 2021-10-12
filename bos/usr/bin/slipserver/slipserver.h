/* @(#)76  1.3  src/bos/usr/bin/slipserver/slipserver.h, rcs, bos411, 9428A410j 6/30/93 18:10:48					*/
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: none
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

#define ERRRORLOG "/tmp/errorlog"

#include <sys/termio.h>
#define  CAT_CMD         	"/bin/cat"
#define  SED_CMD         	"/bin/sed"
#define  RM_CMD         	"/bin/rm"
#define  SLIPCOMM_CMD  		"/usr/bin/slipcomm"
#define  LSUSRPROF_CMD  	"/usr/bin/lsusrprof"
#define  LSSLIP_CMD  		"/usr/bin/lsslip"
#define  GREP_CMD       	"/usr/bin/grep"
#define  STTY_CMD       	"/usr/bin/stty"
#define  MKDEV_CMD      	"/usr/sbin/mkdev"
#define  RMDEV_CMD      	"/usr/sbin/rmdev"
#define  IFCONFIG_CMD   	"/usr/sbin/ifconfig"
#define  SLATTACH_CMD   	"/usr/sbin/slattach"
#define  ROUTE_CMD      	"/etc/route"
#define  ETC_DEVICES_TMP	"/etc/uucp/Devices.tmp"
#define  ETC_DEVICES    	"/etc/uucp/Devices"
#define  SERVER_FIFO    	"/var/tmp/SLIPSERVER"
#define  SERVER_PID_FILE 	"/var/tmp/SLIPSERVER.PID"
#define  CLIENT_FIFO    	"/tmp/SLIPCLIENT"
#define  ROOT 0
#define  MINUS_ONE	-1 
#define  SUCCESS 0
#define  PID_LEN 8
#define	 FAIL 255 /* generic failure return code */
#define	 UNKNOWN_ERROR 255 /* ??? */
#define  MAX_LINE 80
#define  USER_COMMAND_LEN  128
#define  MAX_ROUTE_CMD 1024
#define  ERRORLOG "/u/elaird/smit/src/errorlog"
#define  RETURN_CODE 'r'
#define  OUTPUT_DATA 'o'
#define  CONNECTED  "0"		/* response code 0 connected slip ok	*/
#define  TTY_LOCK_FAIL "1"	/* response code 1 port in use		*/
#define  MKDEV_FAIL "2"		/* response code 2 mkdev slx failed	*/
#define  ALREADY_CONNECTED  "3"	/* response code 3 already connected	*/
#define  SLIP_CONFIG_FAIL "4"	/* response code 4 ifconfig slx failed	*/
#define  ADD_ROUTE_FAIL "5"	/* response code 5 route add failed	*/
#define  SLATTACH_FAIL "6"	/* response code 6 route add failed	*/
#define  MODEM_FAIL "7"		/* response code 7 stty  add failed	*/
#define  FINISHED "8"		/* response code 8 Finished request OK	*/
#define  KILLED_SLIP "8"	/* response code 8 killed slip xxxxx |	*/
#define  END_OF_LIST "8"	/* we have reached the end of the link	*/
                            	/* list, this causes slipcomm to exit(0)*/
#define  SLIP_DISCONNECTED "8"	/* response code 8                    	*/
#define  NO_SLIPS   "9"		/* response code 9 no active slips	*/
#define  RECONFIG_SLIP_FAIL  "10"
				/* response code 10 the reconfig failed	*/
#define  BADLEN 		 1
#define  MIN_PID		 2
#define  BADCOMMAND		 3
#define  CLIENT_FIFO_WRITE_ERR	 4
#define  MKFIFO_SERVER_ERR	 5
#define  UNKNOWN_SLIP_ERR	 6 	/* couldn't find the slip name */
#define	 DEVICES_APPEND_ERR	 7
#define	 CREATE_ERR		 8
#define  OPEN_TTY_LOCK_FILE_FAIL 9
#define  OPEN_CLIENT_FIFO_ERR 	10
#define  CONFIG_DOWN_FAIL	11
#define  DEV_READ_FAIL		12
#define  DEVTMP_WRITE_FAIL	13
#define  ETC_DEV_RENAME_FAIL	14
#define  MODEM_PROBLEM		15 
#define  ETC_DEV_UNLINK_FAIL	16
#define  SLATTACH_FAILED	17 
#define  CHOWN_DEVICES_FAIL	18
#define  DEL_RTS_FAIL		19
#define  REMDEV_SLIP_FAIL	20
#define  SLATTACH_CMD_FAIL	21
				/* modem didn't respond, isn't turned on? */
#define  MKDEV_CMD_FAIL		22
				/* timed out, wrong parms, slx in use? */
#define	 TTYPORT_FAIL		23
#define	 MKDEVSLIP_FAIL		24
#define	 ROUTEADD_FAIL		25
#define	 IFCONFIG_FAIL		26
#define	 TXGETLD_IOCTL_FAIL	27
#define	 SLATTACH_INFO_FORK_FAIL 28 /* child process fork fail */
#define  ZERO_SLIPS		29
#define	 RECONFIG_FAIL		30
#define  DEVICES_RMLINE_FAIL	31
#define	 LOCKFILE_CREAT_ERR	32
#define  ACTIVE 'a'
#define  QUERY 'q'
#define  KILL 'k' 
#define  DISCONNECT 'd'
#define  CONNECT 'c'
#define  RECONFIG 'r'
#define  SET_TIMEOUT 't'
#define  CURRENT_USER 'u'
#define  ALL 'a'

struct    CLIENT {
    int     pid;		/* the process id 		*/
    int    userid;		/* the effective userid		*/
    struct CLIENT *nextclient;	/* pointer to the next client	*/
};

struct    REMOTE_HOST {
		int route_added;
		char remote_host[256];
		struct REMOTE_HOST *next_host;
};

struct    LINK {
	int   devices_line_added;
	int   added_rts_cts;
	int   user_count;
	char  *slip_number;	/* output from mkdev	*/
	char  *slip_name;	/* slip name            */
	char  *slip_desc;	/* slip description     */
	char  *slip_lhost;	/* slip local host      */
				/* gateway address/name	*/
	char  *slip_gateway;	
	char  *slip_mask;	/* slip network mask    */
	char  *slip_ttyport;	/* slip tty port        */
	char  *slip_baud;	/* slip baudrate       	*/
	char  *slip_dialstr;    /* slip dial string    	*/
	/* int   number_of_routes;  number of routes	*/
	/*	struct route *routes; an array of routes	*/
				/* link list of routes	*/
	struct REMOTE_HOST *remote_hosts;
	struct  CLIENT  *client;/* link list of clients */
	struct LINK *nextlink;	/* next slip in the list*/
	struct termios *savebuffer; /* info on the tty	*/
};


