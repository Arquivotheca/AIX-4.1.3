/* @(#)51	1.5  src/bos/diag/util/ulan/saring.h, dsalan, bos411, 9428A410j 1/3/94 13:32:41 */
/*
 *   COMPONENT_NAME: DSALAN
 *
 *   FUNCTIONS: GOTOXY
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/tokuser.h>
#include <sys/signal.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>

#define		YES			1
#define		NO			2
#define		GOOD			0x0
#define		BAD			-1
#define		TIME_OUT		15000 	/* 1000 for  1 second	 */
#define 	ONEMIN			60000	/* one minute of waiting */
#define		ERROR_DETECT		-1
#define		TOKEN  			0x1
#define		ETHERNET		0x2
#define		FDDI    		0x3
#define		NETWORK_ADDR_LEN 	0x6
#define		NO_RESPONSE		-1
#define		DEBUG			1
#define		SIXTEEN			1
#define		FOUR			2
#define         QUIT                    -1
#define		NO_DATA			33
#define		NO_READ_DATA		34
#define		RESPONSE_STATION_STATE 	0x23
#define		STANDBY_MONITOR_PRESENT 0x06

#define         GOTOXY(_a,_b)           move( _b, _a)

/*----------------------------------------------------------------------*/
/* DISPLAY MESSAGE NUMBER						*/
/*----------------------------------------------------------------------*/

#define		TOKEN_RING_SPEED	1
#define		IP_INPUT		2
#define		LAN_STANDBY		3
#define		ETHERNET_BUSY		110
#define		TOKENRING_BUSY		111
#define		FDDI_BUSY     		112
#define		NAUN			80


int		is_802_3;
unsigned char   network_address[6];
unsigned char   sending_station_network_address[6];
unsigned char   hardware_address[NETWORK_ADDR_LEN];
unsigned char   dest_net_address[NETWORK_ADDR_LEN];
unsigned char   gateway_net_address[NETWORK_ADDR_LEN];
unsigned char   NAUN_network_address[6];
unsigned char   BEACONING_network_address[6];
unsigned long	source_ip; 
unsigned long	NAUN_ip; 
unsigned long	destination_ip;
char		source_ip_decimal[64];
char		destination_ip_decimal[64];
unsigned long   gateway_ip;
char            devname[64];
int		net_type;
unsigned char	remote_route_info[18];	/* remote routing information		*/
int		remote_rcl;		/* remote routing control field length*/
unsigned short	remote_rcf;		/* remote control field			*/
static unsigned char           BUF[4096];
unsigned short	icmp_data_length;
int		alarm_timeout;
nl_catd 	catd;                   /* pointer to message catalog      */
int		ring_speed;
char		resource_name[64];
char		device_location[16];

struct PdAt	pd_at;
struct CuAt	cu_at;
struct CuAt	*cu_at_obj;
char		*device_found[16];
char		*location[16];
char		device_type[16];
int		num_devices_found;
int		database_changed;
int		add_ring_speed_database;
int		ring_speed_database_changed;
int		beacon_mac_database_changed;
int		add_beacon_mac_database;
int		prior_state;
int		monitoring;

ASL_SCR_INFO 	*device_info;

FILE		*fp;
int		number_of_loops;

/* these variable are used for Ring Diagnotics Soft error		*/

int		line_errors; int		internal_errors;
int		burst_errors;
int		ac_errors;
int		abort_del_transmit_errors;
int		lost_frame_errors;
int		receiver_congestion;
int		frame_copied_errors;
int		frequency_errors;
int		token_errors;
struct 		pollfd   poll_struct;
int		number_of_local_stations;
long		minute_run;
long 		minute_remain;
long		second_run;
long		second_remain;

char		*p_tmp;
long		begin_time;
long		current_time;
long		time_allowed;
long		time_run;
long		previous_time;
