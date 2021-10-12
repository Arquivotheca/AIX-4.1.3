/* @(#) 64     1.7  2/5/90  07:02:47 */

/*
 * COMPONENT_NAME: (sysxdlcs) SDLC Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
**	File Name      : 64
**
**	Version Number : 1.7
**	Date Created   : 90/02/05      
**	Time Created   : 07:02:47       
*/


/************************************************************************/
/*                                                                      */
/*      IIIIIIII  NN    NN   TTTTTTT  EEEEEEEE  RRRRRR    NN    NN      */
/*         II     NNN   NN     TT     EE        RR    RR  NNN   NN      */
/*         II     NNNN  NN     TT     EE        RR    RR  NNNN  NN      */
/*         II     NN NN NN     TT     EEEEE     RRRRRR    NN NN NN      */
/*         II     NN  NNNN     TT     EE        RR  RR    NN  NNNN      */
/*         II     NN   NNN     TT     EE        RR   RR   NN   NNN      */
/*      IIIIIIII  NN    NN     TT     EEEEEEEE  RR    RR  NN    NN      */
/*                                                                      */
/************************************************************************/

#ifndef H_SDLDEFS
#define H_SDLDEFS

/*
*****************************************************************
*	C O N S T A N T S					*
*****************************************************************
*/

/*
**	misc constants
*/
#define	MOD			8	/* modulo 8 arithmetic	*/
#define	RC_GOOD			0x00
#define ELEMENT_LENGTH		16
#define CMD_HEADER_LENGTH	24
#define BROADCAST		0xFF
#define RETRIEVE		0xFF
#define	REACHED_LIMIT		-1		/* counter overflow	*/
#define	CLOSE_PL		0xDEADBEEF	/* BAD error, close lnk	*/
#define	BUF_SENT		0x0A0B0C0D
#define	ENDFRAME_BUF_SENT	0xA0B0C00D
#define	NULL_POINTER		-1
#define	NOT_FOUND		-2
#define NORMAL			0

#define	NON_ALERT		0		/* non alertable error	*/
#define	ALERT			1		/* alertable error	*/
#define	FN			__FILE__
#define	LN			__LINE__
#define	DLC_SET_LBUSY		0x02
#define	USER_SET_LBUSY		0x04
#define	MAX_NON_PROD_RR		3

/*
**	i frame and buffer maximums
*/
#define	MAX_I_FRAME		4096	/* 4k maximum i frame size	*/


/*
**	station poll modes 
*/
#define ACTIVE			0x41
#define	SLOW			0x53
#define IDLE			0x49

/*
**	link status
*/
#define CLOSED			0x01
#define OPENING			0x02
#define OPENED			0x03
#define CLOSING			0x04
#define ABORTED			0x05

/*
**	station type
*/
#define SECONDARY	0x01
#define PRIMARY		0x02

/*
**	offsets into the data buffer
*/
#define	ADDRESS		0		/* station address		*/
#define	CONTROL		1		/* control byte			*/
#define	DATA		2		/* data area of buffer		*/

					/* the following are used for	*/
					/* frame reject responses	*/
#define LAST_CONTROL	2		/* last control byte received	*/
#define COUNTS		3		/* current NR and NS counts	*/
#define	REASON		4		/* frame reject reason		*/
 
/*
**	monitor trace information     
*/
#define	WRITE_COMMAND		"WCMD"
#define	WRITE_XID		"WXID"
#define	WRITE_NETWORK		"WNTK"
#define	WRITE_DATAGRAM		"WDTG"
#define	WRITE_I_FRAME		"WIFR"
#define	WRITE_UNNUM_RESP	"WUNR"
#define WRITE_UNNUM_CMD		"WUNC"
#define	WRITE_RNR		"WRNR"
#define	WRITE_RR		"W RR"
#define	RECEIVED_XID	 	"RXID"
#define RECEIVED_TEST		"RTST"
#define RECEIVED_DISC		"RDSC"
#define	RECEIVED_IN_NDM		"RNDM"
#define	RECEIVED_I_FRAME	"RIFR"
#define SEND_FRAME_REJECT	"FREJ"
#define	START_LINK_STATION	"STLS"
#define	CHANGE_PARAMETER	"ALTR"
#define	OPEN_DEVICE		"ODEV"
#define CLOSE_DEVICE		"CDEV"
#define START_DEVICE		"SDEV"
#define	HALT_DEVICE		"HDEV"
#define	PRIMARY_RCV		"PRCV"
#define	SECONDARY_RCV		"SRCV"
#define PR_IOCTL		"IOCT"
#define	RX_TIMEOUT		"RXTO"
#define	PROC_STATUS_BLOCK	"STAT"
#define	INVALID_STATUS_BLOCK	"IVSB"
#define	INVALID_ASYNC_OP	"IVAO"
#define	USER_ERROR		"UERR"
#define	START_AUTO_RESP		"S_AR"
#define	END_AUTO_RESP		"E_AR"

/*
**	error thresholds
*/
#define	TRANSMIT_PERCENT	0
#define	RECEIVE_PERCENT		0

/*
**	types of timeouts
*/
#define IDLE_POLL_TYPE		0x01
#define ABORT_TYPE		0x02
#define INACT_TYPE		0x03

/*
**	station mode
*/
#define QUIESCE			0x01
#define NDM			0x01
#define NRM			0x02

/*
**	e_post definitions
*/
#define	WAIT_MASK		0xFF000000
#define	CLR_MASK		0xFF000000
#define	SDL_RINGQ_POST		0x80000000	/* data avail on ringq	*/
#define	SDL_TIMER_POST		0x40000000	/* ticker timer		*/
#define SDL_BAD_RQPUT_DATA	0x20000000	/* rqput failed      	*/
#define SDL_BAD_RQPUT_STATUS	0x10000000	/* rqput failed      	*/

/*
**	generic dlc definitions
*/
#define	DLC_RCV_DATA	0xA1A0000


/*
**	ringq return codes and maximums
*/
#define	DLC_RQ_DEPTH		30		/* ring queue depth	*/
#define	DLC_RINGQ_EMPTY		-1
#define	DLC_RINGQ_FULL		-1


/*
**	length
*/
#define TX_Q_LEN		8
#define WOFFSET			0
#define HEADER_OFFSET		1

/*
**	thresholds and max counts
*/
#define REPOLL_THRESHOLD	15
#define RE_XMIT_BURST_THRESHOLD	10
#define MAX_COUNT		0xFFFFFFFF
#define MAX_LIMIT		50


/*
**	read with no buffer allocation
*/
#define HEADER_LENGTH		2		/*data link header length*/
#define FRMR_LENGTH     	5		/*frame reject     length*/

/*
**	frame type
*/
#define UNNUMBERED_FRAME   	0x3		/* 00000011b		*/
#define S_FRAME   		0x1		/* 00000001b		*/
#define I_FRAME   		0x0		/* 00000000b		*/

/*
**	extended frame type
*/
#define	COMMAND_FRAME		0x01		/* outgoing command	*/
#define	RESPONSE_FRAME		0x02		/* outgoing response	*/
#define	INFORMATION_FRAME	0x03		/* outgoing i frame	*/
#define SUPERVISORY_FRAME	0x04		/* outgoing s frame	*/

/*
**	commands and response
*/
#define SNRM              	0x93
#define DISC              	0x53
#define TEST              	0xF3
#define XID               	0xBF
#define XID_NO_POLL       	0xAF
#define FRMR              	0x97
#define UA                	0x73
#define DM                	0x1F
#define SEND_RR           	0x11		/* 00010001b		*/
#define SEND_RNR          	0x15		/* 00010101b		*/
#define REC_RR            	0x00		/* 00000000b		*/
#define REC_RNR           	0x04		/* 00000100b		*/

/*
**	function masks (for retry purposes)
*/
#define	XID_FUNCTION		0x80000000	/* retry xid data func	*/
#define	I_FUNCTION		0x40000000	/* retry info data func	*/
#define	NET_FUNCTION		0x20000000	/* retry ntwk data func	*/

/*
**	frame reject reasons
*/
#define INVALID_COMMAND   	0x01		/* 00000001b		*/
#define INVALID_I_FIELD   	0x03		/* 00000011b		*/
#define I_FIELD_OVERFLOW  	0x04		/* 00000100b		*/
#define COUNT_INVALID     	0x08		/* 00001000b		*/
#define EMPTY_NO		-1

/*
**	link trace information
*/

#define	TRACE_LONG_SIZE		224
#define	TRACE_SHORT_SIZE	80 
 
#define TRACE_OPEN       	0		 
#define TRACE_CLOSE      	1

/*
**	command pending / response pending flags
*/
#define	XID_PENDING		0x8000
#define	SNRM_PENDING		0x4000
#define	DISC_PENDING		0x2000
#define	TEST_PENDING		0x1000

#endif	/* ifndef H_SDLDEFS */
