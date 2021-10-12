/* @(#)08	1.1  src/bos/kernel/db/POWER/dbnet.h, sysdb, bos411, 9428A410j 6/20/94 13:53:05 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct typeent {
	char *name;
	int val;
};

struct typeent tcptimers[] = {
	{ "TCPT_REXMT", TCPT_REXMT },
	{ "TCPT_PERSIST", TCPT_PERSIST },
	{ "TCPT_KEEP", TCPT_KEEP },
	{ "TCPT_2MSL", TCPT_2MSL },
	{ 0 }
};

struct typeent mbuftypes[] = {
	{ "FREE", MT_FREE },
	{ "DATA", MT_DATA },
	{ "HEADER", MT_HEADER },
	{ "SOCKET", MT_SOCKET },
	{ "PCB", MT_PCB },
	{ "RTABLE", MT_RTABLE },
	{ "ATABLE", MT_ATABLE },
	{ "SONAME", MT_SONAME },
	{ "SOOPTS", MT_SOOPTS },
	{ "FTABLE", MT_FTABLE },
	{ "RIGHTS", MT_RIGHTS },
	{ "IFADDR", MT_IFADDR },
	{ 0 }
};

struct typeent socktypes[] = {
	{ "STREAM", SOCK_STREAM },
	{ "DGRAM", SOCK_DGRAM },
	{ "RAW", SOCK_RAW },
	{ "RDM", SOCK_RDM },
	{ "SEQPACKET", SOCK_SEQPACKET },
	{ 0 }
};

struct typeent tcpstates[] = {
	{ "CLOSED", TCPS_CLOSED },
	{ "LISTEN", TCPS_LISTEN },
	{ "SYN_SENT", TCPS_SYN_SENT },
	{ "SYN_RECEIVED", TCPS_SYN_RECEIVED },
	{ "ESTABLISHED", TCPS_ESTABLISHED },
	{ "CLOSE_WAIT", TCPS_CLOSE_WAIT },
	{ "FIN_WAIT_1", TCPS_FIN_WAIT_1 },
	{ "CLOSING", TCPS_CLOSING },
	{ "LAST_ACK", TCPS_LAST_ACK },
	{ "FIN_WAIT_2", TCPS_FIN_WAIT_2 },
	{ "TIME_WAIT", TCPS_TIME_WAIT },
	{ 0 }
};

struct typeent oobflags[] = {
	{ "HAVEDATA", TCPOOB_HAVEDATA },
	{ "HADDATA", TCPOOB_HADDATA },
	{ 0 }
};

struct flagent {
	char *name;
	int bit;
};

struct flagent sockstates[] = {
	{ "NOFDREF", SS_NOFDREF },
	{ "ISCONNECTED", SS_ISCONNECTED },
	{ "ISCONNECTING", SS_ISCONNECTING },
	{ "ISDISCONNECTING", SS_ISDISCONNECTING },
	{ "CANTSENDMORE", SS_CANTSENDMORE },
	{ "CANTRCVMORE", SS_CANTRCVMORE },
	{ "RCVATMARK", SS_RCVATMARK },
	{ "PRIV", SS_PRIV },
	{ "NBIO", SS_NBIO },
	{ "ASYNC", SS_ASYNC },
	{ "SS_ISCONFIRMING", SS_ISCONFIRMING },
        { 0 }
};

struct flagent ddflags[] = {
	{ "UP", NDD_UP },
	{ "BROADCAST", NDD_BROADCAST },
	{ "DEBUG", NDD_DEBUG },
	{ "RUNNING", NDD_RUNNING },
	{ "NOECHO", NDD_SIMPLEX },
	{ "DEAD", NDD_DEAD },
	{ "LIMBO", NDD_LIMBO },
	{ "PROMISCUOUS", NDD_PROMISC },
	{ "ALT ADDRS", NDD_ALTADDRS },
	{ "MULTICAST", NDD_MULTICAST },
        { 0 }
};

struct flagent tcpcbflags[] = {
	{ "ACKNOW", TF_ACKNOW },
	{ "DELACK", TF_DELACK },
	{ "NODELAY", TF_NODELAY },
	{ "NOOPT", TF_NOOPT },
	{ "SENTFIN", TF_SENTFIN },
        { 0 },
};

struct flagent sbflags[] = {
	{ "LOCK", SB_LOCK },
	{ "WANT", SB_WANT },
	{ "WAIT", SB_WAIT },
	{ "SEL", SB_SEL },
	{ "ASYNC", SB_ASYNC },
	{ "COLL", SB_COLL },
	{ "NOINTR", SB_NOINTR },
	{ "SB_WAKEONE", SB_WAKEONE },
	{ "SB_WAITING", SB_WAITING },
	{ "KIODONE", SB_KIODONE },
        { 0 },
};

struct flagent sockoptflags[] = {
	{ "DEBUG", SO_DEBUG },
	{ "ACCEPTCONN", SO_ACCEPTCONN },
	{ "REUSEADDR", SO_REUSEADDR },
	{ "KEEPALIVE", SO_KEEPALIVE },
	{ "DONTROUTE", SO_DONTROUTE },
	{ "BROADCAST", SO_BROADCAST },
	{ "USELOOPBACK", SO_USELOOPBACK },
	{ "LINGER", SO_LINGER },
	{ "OOBINLINE", SO_OOBINLINE },
        { 0 }
};

struct flagent mbufflags[] = {
	{ "M_EXT", M_EXT },
	{ "M_PKTHDR", M_PKTHDR },
	{ "M_EOR", M_EOR },
	{ "M_BCAST", M_BCAST },
	{ "M_MCAST", M_MCAST },
        { 0 }
};


struct typeent dmxtypes[] = {
        { "Zero?", 0},
        { "Other", NDD_OTHER },
        { "X.25 DDN", NDD_X25DDN },
        { "X.25", NDD_X25 },
        { "Ethernet", NDD_ETHER },
        { "802.3 Ethernet", NDD_ISO88023 },
        { "Token Ring", NDD_ISO88025 },
        { "FDDI", NDD_FDDI },
        { 0 }
};
