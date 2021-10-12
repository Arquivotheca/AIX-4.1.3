/* "@(#)93	1.1  src/bos/usr/bin/errlg/convert_errlog/convert_errlog.h, cmderrlg, bos411, 9428A410j 1/11/94 19:25:12" */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: common header file for CMDERRLG
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct v3_log_hdr {
	char lh_magic[8];       /* "jerrlogj" */
	int  lh_version;        /* some type of version info */
	int  lh_fill1;          /* ? */
	int  lh_startoff;       /* start of log entries */
	int  lh_inoff;          /* offset to where next input will go */
	int  lh_outoff;         /* offset to next entry for reading out */
	int  lh_fill2;          /* ? */
	int  lh_currsize;       /* current size of this logfile */
	int  lh_maxsize;        /* maximum size of this logfile */
	int  lh_sequence;       /* current sequence number */
	int  lh_fill3[5];       /* ? */
};

struct v3_log_entry {
	int le_magic;          /* magic number to help identify */
	int le_length;         /* length of variable-length log entry */
	int le_sequence;       /* sequence number for this entry */
	int le_datalength;     /* length of the data */
	char le_data[sizeof(struct obj_errlog)];	/* actual error record */
	int  le_length2;		/* length for reverse reading */
};

#define v3_LH_MAGIC   "jerrlogj"
#define v3_LE_MAGIC   0x0C3DF420
