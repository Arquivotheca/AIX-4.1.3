static char sccsid[] = "@(#)19  1.1  src/bos/kernext/dlpi/globals.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:26";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * globals.c - driver global symbols
 */

#include "include.h"

/* functions in dispatch table */
extern void dl_info(), dl_bind(), dl_unbind();
extern void dl_udatareq(), dl_attach(), dl_detach();
extern void dl_connreq(), dl_connres(), dl_token();
extern void dl_discreq(), dl_subs_unbind();
extern void dl_resetreq(), dl_resetres(), dl_subs_bind();
extern void dl_enabmulti(), dl_disabmulti(), dl_promiscon(), dl_promiscoff();
extern void dl_xidreq(), dl_xidres(), dl_testreq(), dl_testres();
extern void dl_physaddr(), dl_getstats();
extern void eproto(), notsupp();

/* DLPI primitive dispatch table */
#define	e	eproto
#define	ns	notsupp
void (*dl_funcs[])() = {
	dl_info,	/* 0x00 - DL_INFO_REQ */
	dl_bind,	/* 0x01 - DL_BIND_REQ */
	dl_unbind,	/* 0x02 - DL_UNBIND_REQ */
	e,		/* 0x03 - DL_INFO_ACK */
	e,		/* 0x04 - DL_BIND_ACK */
	e,		/* 0x05 - DL_ERROR_ACK */
	e,		/* 0x06 - DL_OK_ACK */
	dl_udatareq,	/* 0x07 - DL_UNITDATA_REQ */
	e,		/* 0x08 - DL_UNITDATA_IND */
	e,		/* 0x09 - DL_UDERROR_IND */
	ns,		/* 0x0a - DL_UDQOS_REQ */
	dl_attach,	/* 0x0b - DL_ATTACH_REQ */
	dl_detach,	/* 0x0c - DL_DETACH_REQ */
	dl_connreq,	/* 0x0d - DL_CONNECT_REQ */
	e,		/* 0x0e - DL_CONNECT_IND */
	dl_connres,	/* 0x0f - DL_CONNECT_RES */
	e,		/* 0x10 - DL_CONNECT_CON */
	dl_token,	/* 0x11 - DL_TOKEN_REQ */
	e,		/* 0x12 - DL_TOKEN_ACK */
	dl_discreq,	/* 0x13 - DL_DISCONNECT_REQ */
	e,		/* 0x14 - DL_DISCONNECT_IND */
	dl_subs_unbind,	/* 0x15 - DL_SUBS_UNBIND_REQ */
	e,		/* 0x16 - not defined */
	dl_resetreq,	/* 0x17 - DL_RESET_REQ */
	e,		/* 0x18 - DL_RESET_IND */
	dl_resetres,	/* 0x19 - DL_RESET_RES */
	e,		/* 0x1a - DL_RESET_CON */
	dl_subs_bind,	/* 0x1b - DL_SUBS_BIND_REQ */
	e,		/* 0x1c - DL_SUBS_BIND_ACK */
	dl_enabmulti,	/* 0x1d - DL_ENABMULTI_REQ */
	dl_disabmulti,	/* 0x1e - DL_DISABMULTI_REQ */
	dl_promiscon,	/* 0x1f - DL_PROMISCON_REQ */
	dl_promiscoff,	/* 0x20 - DL_PROMISCOFF_REQ */
	ns,		/* 0x21 - DL_DATA_ACK_REQ */
	ns,		/* 0x22 - DL_DATA_ACK_IND */
	ns,		/* 0x23 - DL_DATA_ACK_STATUS_IND */
	ns,		/* 0x24 - DL_REPLY_REQ */
	ns,		/* 0x25 - DL_REPLY_IND */
	ns,		/* 0x26 - DL_REPLY_STATUS_IND */
	ns,		/* 0x27 - DL_REPLY_UPDATE_REQ */
	ns,		/* 0x28 - DL_REPLY_UPDATE_STATUS_IND */
	dl_xidreq,	/* 0x29 - DL_XID_REQ */
	e,		/* 0x2a - DL_XID_IND */
	dl_xidres,	/* 0x2b - DL_XID_RES */
	e,		/* 0x2c - DL_XID_CON */
	dl_testreq,	/* 0x2d - DL_TEST_REQ */
	e,		/* 0x2e - DL_TEST_IND */
	dl_testres,	/* 0x2f - DL_TEST_RES */
	e,		/* 0x30 - DL_TEST_CON */
	dl_physaddr,	/* 0x31 - DL_PHYS_ADDR_REQ */
	e,		/* 0x32 - DL_PHYS_ADDR_ACK */
	ns,		/* 0x33 - DL_SET_PHYS_ADDR_REQ */
	dl_getstats,	/* 0x34 - DL_GET_STATISTICS_REQ */
	e,		/* 0x35 - DL_GET_STATISTICS_ACK */
};
#undef e
#undef ns

/* global stats, default broadcast address */
stats_t dl_stats;
unsigned char dl_broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* debugging enable */
int dl_Debug = 1;
int dl_Trace = 1;

/* MP declarations */
int  dl_noni_lock = LOCK_AVAIL;
Simple_lock dl_intr_lock;
