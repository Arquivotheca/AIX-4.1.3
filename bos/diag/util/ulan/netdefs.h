/* @(#)45	1.2  src/bos/diag/util/ulan/netdefs.h, dsalan, bos411, 9428A410j 1/5/93 14:11:59 */
/*
 *   COMPONENT_NAME: DSALAN
 *
 *   FUNCTIONS: 
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


/* ************************************************************************** */
/*  Shorthand defines for control block entries                               */
/* ************************************************************************** */
/* ************************************************************************** */
/*  Padding prefix for token ring.                                            */
/* ************************************************************************** */


#define LLC_K1          0xaa    /*      == K1 == 0xaa == 170 */

#define LLC_CONTROL     0x3     /*      == 3 == Unnumbered Information */

#define LLC_K2          0x0     /*  all three bytes zero (== K2) for TCP/IP */

#define TR_PAD          0x40

typedef unsigned long IP_ADDR;            

typedef unsigned char HW_ADDR[6];

typedef struct 
{
  IP_ADDR ip;
  HW_ADDR hw;
} IP_HW_ADDRS;

typedef struct 
{
  unsigned short rcf;
  unsigned short seg[8];
} TOK_ROUTE;

typedef struct 
{
  IP_ADDR ip;
  HW_ADDR hw;
  TOK_ROUTE route;
} ARP_ENTRY;


typedef struct 
{
       unsigned char ac;
       unsigned char fc;
       HW_ADDR dest;
       HW_ADDR source;
} t_802_5;

typedef struct 
{
	unsigned char	rs1;	
	unsigned char	rs2;	
	unsigned char	rs3;	
       unsigned char fc;
       HW_ADDR dest;
       HW_ADDR source;
} t_fddi;

typedef struct 
{
       HW_ADDR dest;
       HW_ADDR source;
       unsigned short length;
} t_802_3;

typedef struct 
{
       unsigned char dsap;
       unsigned char ssap;
       unsigned char control;
       unsigned char prot_id[3];
       unsigned short type;
} t_802_2;

#define RC_LTH        0x1f00    /* Length Field (even valued) */
#define RC_DIR_MASK   0x0080    /* Direction Mask             */
#define RC_BC_MASK    0x1FFF    /* Broadcast Mask             */
#define RC_PAD        18        /* Routing control pad        */



/* ************************************************************************** */
/* Ethernet stuff                                                             */
/* ************************************************************************** */
#define ET_IP           0x0800
#define ET_ARP          0x0806
#define ET_RARP          0x8035
#define ET_MINLEN       64
#define ET_MAXLEN       1514

typedef unsigned char t_etaddr;

typedef struct
{
        t_etaddr dst[6];
        t_etaddr src[6];
        unsigned short type;
} t_ether;

/* ************************************************************************** */
/* ARP Stuff                                                                  */
/* ************************************************************************** */

typedef struct 
{
        unsigned short hardware;        /* Ether HW types - 1=ether, 6=802  */
        unsigned short protocol;        /* Ether Protocol types, assume IP  */
        unsigned char  hlen;            /* Assume Ether or TR, each = 6     */
        unsigned char  plen;            /* Assume IP, =4                    */
        unsigned short opcode;          /* 1 = request, 2 = response        */
        unsigned char  sender_hw[6];    /*                                  */
        unsigned char  sender_ip[4];    /* Have to declare IP address this  */
        unsigned char  target_hw[6];    /* way because compiler lines up    */
        unsigned char  target_ip[4];    /* ints on word boundaries          */
} t_arp;

#define ARP_REQ         1
#define ARP_REP         2
#define RARP_REQ   	3 
#define RARP_REP        4
#define ARP_SIZE        28

/* ************************************************************************** */
/* IP Stuff                                                                   */
/* ************************************************************************** */
#define IP_PAD          40      /* RFC 791 says max header is 60 bytes        */
                                /* RFC 815 say 64, but it was wrong on other  */
                                /* stuff anyway. */
#define IP_MINLEN       0
#define IP_MAXLEN       1480    /* 1514 - ETHERSIZE - IPSIZE                  */
#define IP_VERIHL       0x45    /* Internet version (4) and header len (5)    */
#define IP_SERVICE      0       /* default type of service                    */
#define IP_TIME         30      /* maximum time to live                       */
#define IP_VERS_NUM     0xF0    /* Mask for version bits                      */
#define IP_HEAD_LEN     0x0F    /* Mask for length bits                       */
#define IP_MORE_FRAG    0xE000  /* Mask for frag flag bits                    */
#define IP_LAST_FRAG    0x2000  /* Mask for last frag flag bit                */
#define IP_FRAG_OFFSET  0x1FFF  /* Mask for frag offset bits                  */
#define IP_TCP          6       /* TCP protocol number                        */
#define IP_UDP          17      /* UDP protocol number                        */
#define IP_ICMP         1       /* ICMP protocol number                       */
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQ   8
#define ICMP_REDIRECT   5

/* typedef unsigned long t_ipaddr; */

typedef struct 
{
        unsigned char ver_IHL;  /* version and header length                  */
        unsigned char service;  /* Type of service                            */
        unsigned short len;     /* Total packet length including header       */
        unsigned short id;      /* ID for fragmentation                       */
        unsigned short fragment;/* fragment and flags                         */
        unsigned char time;     /* Time to live (secs)                        */
        unsigned char proto;    /* protocol                                   */
        unsigned short csum;    /* Header checksum                            */
        IP_ADDR source;        /* Source name                                */
        IP_ADDR dest;          /* Destination name                           */
} t_ip;

typedef struct 
{
        unsigned char type;
        unsigned char code;
        unsigned short checksum;
} t_icmp;


#define HARDWARE_ETHER   	1
#define HARDWARE_TOKEN		6



typedef struct 
{
	unsigned char	ac;
	unsigned char	fc;
	HW_ADDR 	dest;
	HW_ADDR 	source;
	short		major_vector_length;
	unsigned char	major_vector_class;
	unsigned char	major_vector_command;
	unsigned char	subvector_length;
	unsigned char	subvector_id;
	unsigned char	data[512];
} t_mac_report_station;

typedef struct 
{
	unsigned char	ac;
	unsigned char	fc;
	HW_ADDR 	dest;
	HW_ADDR 	source;
	unsigned char	dsap;
	unsigned char 	ssap;
	unsigned char	command_byte;
} t_send_stations_test ;

typedef struct
{
	unsigned char	network_address[6];
	long		ip;
} t_station_id;

t_station_id	station_ids[256];
