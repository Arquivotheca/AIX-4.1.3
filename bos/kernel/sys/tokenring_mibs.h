/* @(#)65	1.4  src/bos/kernel/sys/tokenring_mibs.h, snmp, bos411, 9428A410j 4/21/94 11:15:31 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/include/sys/tokenring_mibs.h
 */

#ifndef _H_TokenRing_MIBS
#define _H_TokenRing_MIBS

#include <sys/generic_mibs.h>


typedef struct  Dot5Entry  {
#define TOKEN_COMMANDS		30
	unsigned short  commands;	/* dot5Commands */
#define TR_MIB_NO_OP	1
#define TR_MIB_OPEN	2
#define TR_MIB_RESET	3
#define TR_MIB_CLOSE	4

#define TOKEN_RING_STATUS	31
	unsigned long   ring_status;	/* dot5RingStatus */
#define TR_MIB_NOPROBLEM		0
#define TR_MIB_RINGRECOVERY  		32
#define TR_MIB_SINGLESTATION		64
#define TR_MIB_REMOVE_RX		256
#define TR_MIB_RESERVED			512
#define TR_MIB_AUTOREMOVAL_ERR		1024
#define TR_MIB_LOBEWIREFAULT		2048
#define TR_MIB_TX_BEACON		4096
#define TR_MIB_SOFT_ERR			8192
#define TR_MIB_HARD_ERROR		16384
#define TR_MIB_SIGNAL_LOSS		32768
#define TR_MIB_NO_STATUS		131072

#define TOKEN_RING_STATE	32
	unsigned short  ring_state;	/* dot5RingState */
#define TR_MIB_OPENED		1
#define TR_MIB_CLOSED		2
#define TR_MIB_OPENING		3
#define TR_MIB_CLOSING		4
#define TR_MIB_OPENFAILURE	5
#define TR_MIB_RINGFAILURE	6

#define TOKEN_RING_OSTATUS	33
	unsigned short  ring_ostatus;	/* dot5RingOpenStatus */
#define TR_MIB_NOOPEN			1	/* no open attempted */
#define TR_MIB_BADPARAM		2
#define TR_MIB_LOBEFAILED		3
#define TR_MIB_SIGNALLOSS		4
#define TR_MIB_INSERTIONTIMEOUT	5
#define TR_MIB_RINGFAILED		6
#define TR_MIB_BEACONING		7
#define TR_MIB_DUPLICATEMAC		8
#define TR_MIB_REQUESTFAILED		9
#define TR_MIB_REMOVERECEIVED		10
#define TR_MIB_LASTOPEN			11      /* last open successful */

#define TOKEN_RING_SPEED	34
	unsigned short  ring_speed;	/* dot5RingSpeed */
#define TR_MIB_UNKNOWN		1
#define TR_MIB_ONEMEGABIT	2
#define TR_MIB_FOURMEGABIT	3
#define TR_MIB_SIXTEENMEGABIT	4

#define TOKEN_UPSTREAM		35
	unsigned char   upstream[6];	/* dot5UpStream */
	unsigned short  participate;	/* dot5ActMonParticipate */
#define TR_MIB_TRUE	1
#define TR_MIB_FALSE	2

#define TOKEN_FUNCTIONAL	36
	unsigned char   functional[6];	/* dot5Functional */
}  Dot5Entry_t;


/* The Statistics Table */
typedef struct  Dot5StatsEntry  {
#define TOKEN_LINE_ERRS		40
	unsigned long  line_errs;	/* dot5StatsLineErrors */

#define TOKEN_BURST_ERRS	41
	unsigned long  burst_errs;	/* dot5StatsBurstErrors */

#define TOKEN_AC_ERRS		42
	unsigned long  ac_errs;		/* dot5StatsACErrors */

#define TOKEN_ABORT_ERRS	43
	unsigned long  abort_errs;	/* dot5StatsAbortTransErrors */

#define TOKEN_INT_ERRS		44
	unsigned long  int_errs;	/* dot5StatsInternalErrors */

#define TOKEN_LOSTFRAMES	45
	unsigned long  lostframes;	/* dot5StatsLostFrameErrors */

#define TOKEN_RX_CONGESTION	46
	unsigned long  rx_congestion;	/* dot5StatsReceiveCongestions */

#define TOKEN_FRAMECOPIES	47
	unsigned long  framecopies;	/* dot5StatsFrameCopiedErrors */

#define TOKEN_TOKEN_ERRS	48
	unsigned long  token_errs;	/* dot5StatsTokenErrors */

#define TOKEN_SOFT_ERRS		49
	unsigned long  soft_errs;	/* dot5StatsSoftErrors */

#define TOKEN_HARD_ERRS		50
	unsigned long  hard_errs;	/* dot5StatsHardErrors */

#define TOKEN_SIGNAL_LOSS	51
	unsigned long  signal_loss;	/* dot5StatsSignalLoss */

#define TOKEN_TX_BEACONS	52
	unsigned long  tx_beacons;	/* dot5StatsTransmitBeacons */

#define TOKEN_RECOVERYS		53
	unsigned long  recoverys;	/* dot5StatsRecoverys */

#define TOKEN_LOBEWIRES		54
	unsigned long  lobewires;	/* dot5StatsLobeWires */

#define TOKEN_REMOVES		55
	unsigned long  removes;		/* dot5StatsRemoves */

#define TOKEN_SINGLES		56
	unsigned long  singles;		/* dot5StatsSingles */

#define TOKEN_FREQ_ERRS		57
	unsigned long  freq_errs;	/* dot5StatsFreqErrors */
} Dot5StatsEntry_t;


/* The Timer Table */
typedef struct  Dot5TimerEntry {
#define TOKEN_RETURN_REPEAT	60
	unsigned long  return_repeat;	/* dot5TimerReturnRepeat */

#define TOKEN_HOLDING		61
	unsigned long  holding;		/* dot5TimerHolding */

#define TOKEN_QUEUE_PDU		62
	unsigned long  queue_pdu;	/* dot5TimerQueuePDU */

#define TOKEN_VALID_TX		63
	unsigned long  valid_tx;	/* dot5TimerValidTransmit */

#define TOKEN_NO_TOKEN		64
	unsigned long  no_token;	/* dot5TimerNoToken */

#define TOKEN_ACTIVE_MON	65
	unsigned long  active_mon;	/* dot5TimerActiveMon */

#define TOKEN_STANDBY_MON	66
	unsigned long  standby_mon;	/* dot5TimerStandbyMon */

#define TOKEN_ERR_REPORT	67
	unsigned long  err_report;	/* dot5TimerErrorReport */

#define TOKEN_BEACON_TX		68
	unsigned long  beacon_tx;	/* dot5TimerBeaconTransmit */

#define TOKEN_BEACON_RX		69
	unsigned long  beacon_rx;	/* dot5TimerBeaconReceive */
} Dot5TimerEntry_t;

typedef struct token_ring_mib  {
	Dot5Entry_t		Dot5Entry;
	Dot5StatsEntry_t	Dot5StatsEntry;
	Dot5TimerEntry_t	Dot5TimerEntry;
}  token_ring_mib_t;

typedef struct token_ring_all_mib  {
	generic_mib_t		Generic_mib;
	token_ring_mib_t	Token_ring_mib;
} token_ring_all_mib_t;

/* 802.5 Interface Tests */
#define TR_MIB_testInsertFunc    	1

/* 802.5 Hardware Chip Sets -- Dot5ChipSets */
/* IBM 16/4 Mb/s */
#define TR_MIB_IBM16		"\000\000\000\012\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\011\000\000\000\004\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* TI 4Mb/s */
#define TR_MIB_TITMS380		"\000\000\000\012\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\011\000\000\000\004\000\000\000\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

/* TI 16/4 Mb/s */
#define TR_MIB_TITMS380C16 	"\000\000\000\012\000\000\000\001\000\000\000\003\000\000\000\006\000\000\000\001\000\000\000\002\000\000\000\001\000\000\000\012\000\000\000\011\000\000\000\004\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

#endif  /* _H_TokenRing_MIBS */
