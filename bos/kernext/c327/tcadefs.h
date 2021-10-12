/* @(#)35	1.13  src/bos/kernext/c327/tcadefs.h, sysxc327, bos411, 9428A410j 6/15/90 18:17:49 */
#ifndef	_H_TCADEFS
#define	_H_TCADEFS

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca defines
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

/*
** tcadefs.h
*/
#include <sys/errids.h>

#define MAXPROC		32
#define CLEAR		0x00
#define SET		0x01
#define RCOL		010000000	/* select read collision flag */
#define WCOL		020000000	/* select write collision flag */
#define ECOL		040000000	/* select exception collision flag */

/************** Used by ioctl IOCINFO ***************/

#define	CONTROLLER_TYPE1 \
             cd_ddsPtr->dds_dev_section.cu_info.cu.cont_id_1
#define	CONTROLLER_TYPE2 \
             cd_ddsPtr->dds_dev_section.cu_info.cu.cont_id_2


#define	ATTACHMENT_TYPE  cd_ddsPtr->dds_dev_section.cuat_dds

/********* Module IDs ***************/

#define	Open			1
#define	Read			3
#define	Write			4
#define	Send_Stat_Ioctl		6 
#define	Power_On_Reset_Ioctl	7 

#define SUCCESS			0	/* function succeded */
#define FAILURE			1	/* function failed */

/*
** MDEP return codes from machine dependent routines
*/
#define MDEP_SUCCESS		1	/* routine succeeded */
#define MDEP_FAILURE		0	/* routine failed */

/*
** Link State Defines
*/
#define LS_LINKDOWN		(uint)0x0000001	/* link is down              */
#define LS_NORMAL		(uint)0x0000002	/* normal (home) state       */
#define LS_DATA_AVAIL		(uint)0x0000004	/* data available            */
#define LS_RD_OF_WRT_INPROG	(uint)0x0000008	/* read of write in progress */
#define LS_RD_OF_RD_INPROG	(uint)0x0000010	/* read of read in progress  */
#define LS_WAIT_SYSWRITE	(uint)0x0000020	/* waiting for system write()*/
#define LS_W_WAIT_READACK	(uint)0x0000040	/* write waiting for read ack*/
#define LS_EA_WRITE		(uint)0x0000080	/* e.a. write initial state  */
#define LS_EA_WAIT_SYSREAD	(uint)0x0000100	/* e.a. wait for AIX read    */
#define LS_EA_WAIT_READACK	(uint)0x0000200	/* e.a. wait for remote ack. */
#define LS_DPE			(uint)0x0000400	/* data pending enter state  */
#define LS_DPE_WRITE		(uint)0x0000800	/* d.p.e. write initial state*/
#define LS_DPE_WAIT_SYSREAD	(uint)0x0001000	/* d.p.e. wait for AIX read  */
#define LS_DPE_WAIT_READACK	(uint)0x0002000	/* d.p.e. wait for remote ack*/
#define LS_RE			(uint)0x0004000	/* retry enter state         */
#define LS_CLOSING		(uint)0x0008000	/* closing link              */
#define LS_RESETTING		(uint)0x0010000	/* reseting link             */
#define LS_AA_RD_OF_WRT_INPROG	(uint)0x0020000	/* autoack RD_OF_WRT_INPROG  */
#define LS_AA_INPROG		(uint)0x0040000	/* autoack in progress       */
#define LS_START_INPROG		(uint)0x0080000	/* start in progress         */
#define LS_RM_WRITE		(uint)0x0100000 /* read modified executed */
#define LS_UNDEFINED		(uint)0xfffffff	/* undefined due to null laP */

/*
** Combination of useful states
*/
/* READ_STATES - all states where a read can occur */
#define READ_STATES	(LS_DATA_AVAIL | LS_RD_OF_WRT_INPROG | \
			LS_RD_OF_RD_INPROG | LS_AA_RD_OF_WRT_INPROG | \
			LS_LINKDOWN)

#define WRITE_STATES        (LS_EA_WRITE | LS_DPE_WRITE | LS_CLOSING)
/*
** macros to manipulate link state field in link address structure
** (getLinkState returns undefined link state when laP is null)
*/
#define getLinkState(laP)	((laP ? laP->link_state : LS_UNDEFINED))

typedef unsigned int	state;

#define isCardUp(_d)		(tca_data[(_d)].tca_card_status == TRUE)
#define isCardDown(_d)		(tca_data[(_d)].tca_card_status == FALSE)
#define setCardUp(_d)		(tca_data[(_d)].tca_card_status = TRUE)
#define setCardDown(_d)		(tca_data[(_d)].tca_card_status = FALSE)

#endif	/* _H_TCADEFS */   
