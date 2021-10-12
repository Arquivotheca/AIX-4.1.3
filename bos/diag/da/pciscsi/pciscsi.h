/* @(#)20	1.2  src/bos/diag/da/pciscsi/pciscsi.h, dapciscsi, bos41J, 9518A_all 5/1/95 15:20:57 */
/*
 *   COMPONENT_NAME: DAPCISCSI
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <stdio.h>
#include        <locale.h>
#include        <signal.h>
#include        <sys/errids.h>
#include        <sys/scsi.h>
#include        <sys/diagex.h>
#include        <diag/diag.h>
#include        <diag/diago.h>
#include        <diag/tm_input.h>
#include        <diag/tmdefs.h>
#include 	<diag/diagresid.h>
#include        <diag/diag_trace.h>
#include        <diag/da.h>
#include        <diag/da_rc.h>
#include        <diag/diag_exit.h>
#include        "dpciscsi_msg.h"
#include        "bloomtu.h"

/* Structure for DA variables. */
struct da_struct {
	long flags;
	struct tm_input tm_input;
	nl_catd catd;
	int ffc;
	tucb_t tucb;
	int tu_rc;
	int srn_rcode;
};

/* Stucture for SRN reason info. */
struct reason_info {
	int rcode;		/* SRN Reason code. */
	int rmsg;		/* SRN Reason message for this rcode. */
	int frub_index;		/* Index into FRU bucket for this rcode. */
};

/* Device FFC's. */
#define FFC_SE		0x746
#define FFC_DIFF	0x747
#define FFC_INT		0x868

/* Flag defines used with da.flags. */
#define DA_ODM_INIT		0x01
#define DA_ASL_INIT  		0x02
#define DA_CAT_OPEN  		0x04
#define DA_DEV_OPEN  		0x08
#define DETECT_DEV_TYPE 	0x10
#define DETECT_DEV_ERROR 	0x20

/* ELA defines */
		/* EN = Error Number from the Error Log Detail Data */
#define MAX_EN_110	5
#define MAX_EN_115	5
#define MAX_EN_127	5
#define MAX_EN_128	5
#define MAX_EN_160	5
#define MAX_EN_170	5
#define MAX_ERRID_10	50
#define MAX_PTC		1 	/* From the detail data AH Stat */
#define ERR_NUM_OFFSET  8 	/* Offset in the detail data buffer. */
#define AH_STAT_OFFSET  3 	/* Offset in the detail data buffer. */

/* Misc. defines */
#define BLOOM_DEVICE_BUSY	99
#define SW_ERROR		0x803
#define FAILED_POST		0x225
#define ERROR_INITIAL_STATE	0x999

