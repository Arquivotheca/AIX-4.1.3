/* @(#)34	1.16.1.1  src/bos/kernext/c327/tcadecls.h, sysxc327, bos411, 9428A410j 6/12/92 21:05:50 */
#ifndef	_H_TCADECLS
#define	_H_TCADECLS

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca declarations
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/err_rec.h>
#include "tcadefs.h"
/*
** tcadecls.h
*/

/*
** link_addr:
**	Link address structure
*/
typedef struct link_addr {
	/* Flags we sleep on */
	char	io_in_prog; 	       /* global lock on link address */
	char	waiting_on_io_in_prog; /* actual where sleep for io_in_prog */
	char	waiting_for_data;      /* sleep here when no data for read() */
	char	mlocked;		/* */
	char	waiting_for_unmlocked;	/* */
	char    waiting_for_WSFdb;      /* waiting for la_WSFdbP to be free */
	char    api_cmdWSF;             /* Current la_WSFdbP is api cmdWSF */
	char	la_ackIncKeyun;		/* read ack. includes keyb. unlock */
	char	la_waitClear;		/* waiting for broadcast clear */
	char	la_autoAck;		/* if {TRUE}, then do fxfer autoack */
	boolean	write_discarded;        /* if {TRUE}, write cmd discarded */
    
	/* Flags system sleeps on */
	int	sleep_link_state;
	int	sleep_waiting_on_io_in_prog;
	int	sleep_la_waitClear;
	int	sleep_waiting_for_data;
	int	sleep_waiting_for_unmlocked;
	int     sleep_waiting_for_write_buf;

	/* I/O Processing Information */
	char	la_sess_type;		/* terminal, printer */
	char	la_printerAddr;		/* if printer, this is printer addr */
	short	dd_corr_num;		/* link address correlation number */
	Data_Buffer	*la_recvDbP;	/* read buffer */

        boolean         writeBuffer_used;
        Data_Buffer     *writeBuffer;   /* inbound write buffer */
        int             read_mod;

        Data_Buffer	*la_WSFdbP;	/* Extra buffer for auto ack */

	NETWORK_ID	la_netID;
	uchar		BSC_mode;	/* Transparency Mode */
    
	/* Select Information */
	int    dev_selr;	       /* pid of proc. selecting on read */
	int    dev_sele;	       /* pid of proc. selecting on exception*/
	int    dev_flags;	       /* variable holding collision flags */
    
	/* Process Information */
	int	num_processes;		/* # of procs that have opened link */
    
	/* Status Of Last I/O */
	uint	io_flags;		/* link information flag for io_tca */
	uint	io_status;		/* 5xx, 6xx, 7xx msg codes    */
    uint    io_extra;       /* extra info. in this case, SNA 
                               ACTLU/DACTLU data. */                             

	/* Link State */
	uint	link_state;

    /* sna or non_sna */
    boolean non_sna;        /* lets tca code know if we are an SNA session */

	/* return code from device entry point */
	int	rc;
} linkAddr;

/*
** errmsg:
**	error logger structure
*/
typedef struct errmsg {
	struct  err_rec0 err;
        int     results;
        int     address;
} errmsg;

/*
** card_data:
*/
typedef struct card_data {
	char	waiting_to_open;
	char	open_link_address;
	DDS_DATA	*cd_ddsPtr;
	char	snoozer;
	char	open_first_time;
	char	lower_link_address;
	char	upper_link_address;
	char	INIT_COMPLETE;
	int	tca_card_status;
	int	num_times_init;
	linkAddr	*mlnk_ptrs[MAX_SESSIONS+1];
	int 	sleep_open_link_address;	/* flag system sleeps on */
} cardData;

#endif	/* _H_TCADECLS */
