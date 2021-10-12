/* @(#)81	1.2  src/bos/diag/da/sdisk/dh2srn.h, dasdisk, bos411, 9428A410j 12/11/92 09:31:30 */
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DH2SRN
#define _H_DH2SRN

/* Definitions of controller Failure. These constants are used in CREATE_FRUB */

#define	INVALID_ADAP_STATUS	0x004fe
#define	ADAPTER_CONFIG_ERR	0x0c021 
#define	CONTROLLER_CONFIG_ERR	0x0c013
#define	DASD_CONFIG_ERR		0x0c005
#define	ADAPTER_POST_FAIL	0x0d001
#define	ADAPTER_OPENX_FAIL	0x0d002
#define	CONTROLLER_OPENX_FAIL	0x0d003
#define	DISK_OPENX_FAIL		0x0d004
#define	CONTROLLER_POST_FAIL	0x0d301
#define	CHECK_READING_SENSE	0x0d310
#define	INVALID_Q_FULL		0x0d320
#define	INVALID_RESERVATION	0x0d330
#define	CNTRL_FAULT_RSV_UNRESET	0x0d340
#define	INVALID_SENSE_DATA	0x0d350
#define	CNTRL_QUIESCE_FAILS  	0x0d360
#define	CNTRL_FAULT_QUIESCE	0x0d370
#define	ADAPTER_TIMEOUT		0x0d510
#define	INVALID_SCSI_STATUS	0x0d570
#define	BUS_ERROR		0x0d580
#define PIO_ERROR		0x0d601
#define	INVALID_PARAMS		0x0dffe
#define	INVALID_RETURN_CODE	0x0dfff

#define HARDWARE_FAILURE	0x0d600 /* HW status and Alert Register byte */
#define ADAPTER_FAILURE         0x0d400 /* need to be added to form SRN.    */
#define	ELA_FOUND_SOFT_ERRS	0x0282  /* Soft errors found during ELA.     */
#endif
