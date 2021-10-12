static char sccsid[] = "@(#)42  1.10  src/bos/kernext/dlc/sdl/sdlerr.c, sysxdlcs, bos411, 9428A410j 10/19/93 12:44:32";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: sdlerr.c
 *	error_log()
 *	sdlc_alert()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/

/*
**      File Name      : 42
**
**      Version Number : 1.10
**      Date Created   : 93/10/19
**      Time Created   : 12:44:32
*/


#include "sdlc.h"



#define DET_MODULE	0x00A2	/* (00A2) Detecting Module	     */
#define ADD_SUBVECS	0x8001	/* (8001) Additional Subvectors      */

#define SV_LCS_DATA	  0x51	/* Link Connection Subsystem Data SV */
#define SF51_LOCAL_MAC	  0x03	/* Local MAC address Subfield	     */
#define SF51_REMOTE_MAC   0x04	/* Remote MAC address Subfield	     */
#define SF51_ROUTING	  0x04	/* Routing Information Subfield      */

#define SV_LCS_CONFIG	  0x52	/* Link Connection Subsystem Data SV */
#define SF52_REMOTE_ADDR  0x02	/* Remote link address/SAP Subfield  */
#define SF52_LOCAL_ADDR   0x04	/* Local link address/SAP Subfield   */

#define SF52_LS_ATTR	  0x06	/* Link station attributes	     */
#define SF52_PRIMARY	  0x01	/* attribute = primary station		*/
#define SF52_SECONDARY	  0x02	/* attribute = secondary station	*/
#define SF52_NEGOTIABLE   0x03	/* attribute = negotiable station	*/
#define SF52_NODE_TYPE	  0x04	/* node type 2.1 SNA			*/

#define SF52_LINK_ATTR	  0x07	/* Link connection attributes		*/
#define SF52_NON_SW	  0x01	/* attribute = non switched		*/
#define SF52_SWITCHED	  0x02	/* attribute = switched			*/
#define SF52_HDX	  0x01	/* attribute = half duplex		*/
#define SF52_FDX	  0x02	/* attribute = full duplex		*/
#define SF52_SDLC	  0x01	/* attribute = SDLC data link		*/
#define SF52_BSC	  0x02	/* attribute = BSC data link		*/
#define SF52_START_STOP	  0x03	/* attribute = start stop		*/
#define SF52_LAPB	  0x04	/* attribute = LAPB			*/
#define SF52_PT_TO_PT	  0x01	/* link type is point-to-point		*/
#define SF52_MULTI_PT	  0x02	/* mulitpoint link			*/

#define SV_LS_DATA	  0x8C	/* Link Station Data Subfield	     */
#define SF8C_NS_NR	  0x01	/* Current NS/NR counts Subfield     */
#define SF8C_FRAMES_OUT   0x02	/* Outstanding frame count Subfield  */
#define SF8C_CNTL_RCVD	  0x03	/* Last control received Subfield    */
#define SF8C_CNTL_SENT	  0x04	/* Last control sent Subfield	     */
#define SF8C_MODULUS	  0x05	/* Sequence number modulus Subfield  */
#define SF8C_LS_STATE	  0x06	/* Link station state Subfield	     */
#define SF8C_LS_LBUSY	  0x80	/* Link station state = local busy   */
#define SF8C_LS_RBUSY	  0x40	/* Link station state = remote busy  */
#define SF8C_REPOLLS	  0x07	/* Number of repolls Subfield	     */
#define SF8C_RCVD_NR	  0x08	/* Last received Nr Subfield	     */

#define SV_RESOURCE_LIST  0x05	/* Hierarchy/Resource List SV	     */
#define SF05_NAME_LIST	  0x10	/* Name List Subfield		     */
#define SE0510_CMPL_IND   0xC0	/* Complete Indicator Sub Element    */
#define SE0510_FLAGS	  0x00	/* Flags Sub Element		     */

#define COMP_NAME "SYSXDLCS"
#define DLC_TYPE "SDLC DLC        "   /* must be 16 chars */

void	error_log();
void	setup_entry();
void	sdlc_alert();


/************************************************************************/
/*                                                                      */
/* Name:	error_log                                               */
/*                                                                      */
/* Function:	build an SDLC error log entry                           */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	                                                        */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void	error_log(cb, error, alert, plc_code, filenm, line)

register	PORT_CB	*cb;
ulong		error;		/* error id for this log		*/
int		alert;		/* is this error an alert		*/
int		plc_code;	/* physical link error code		*/
char		*filenm;	/* file name of module logging the error*/
ulong		line;		/* line num of module logging the error */

{
	cb->errptr = cb->sdl_error_rec;

	setup_entry(cb, error, alert, filenm, line);

	errsave(cb->sdl_error_rec, cb->errptr - cb->sdl_error_rec);

	/* if there is a plc code, then halt the device and close the SAP */
	if (plc_code)
	{
#ifdef MULT_PU
		sdl_abort(cb);
#endif
		pl_blowout(cb);
	}
}


/************************************************************************/
/*                                                                      */
/* Name:	setup_entry                                             */
/*                                                                      */
/* Function:	set up the detail data information                      */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void	setup_entry(cb, error, alert, filenm, line)

register	PORT_CB	*cb;
ulong		error;		/* error id for this log		*/
int		alert;		/* is this an alert			*/
char		*filenm;	/* file name of module logging the err	*/
ulong		line;		/* line num of module logging the error	*/

{
	struct err_rec *rptr;

	rptr = (struct err_rec *)cb->errptr;

	/* Setup the Error ID field in the error record */
	rptr->error_id = error;

	/* Load the Resource Name into the error record */
	bcopy(COMP_NAME,rptr->resource_name,sizeof(COMP_NAME));

	/* Build the generic part of the detailed data area */
	/* Initialize the Detail Data pointer, vector size counter and
					    total vector size counter */
	cb->errptr = rptr->detail_data;


	/* Data Link Type */
	bcopy (DLC_TYPE, cb->errptr, 16);
	cb->errptr = cb->errptr + 16;	 /* fixed length */

	/* Communications Device Name */
	bcopy("                ", cb->errptr, 16);
	bcopy(cb->dlc.namestr, cb->errptr, strlen(cb->dlc.namestr));
	cb->errptr = cb->errptr + 16;

	/* Detecting Module */
	bcopy("                    ",cb->errptr,20);
	bcopy(filenm, cb->errptr, strlen(filenm));
	cb->errptr = cb->errptr + 20;

	/* Line number */
	bcopy(&line, cb->errptr, 4);
	cb->errptr = cb->errptr + 4;

	/* Sense data */
	bcopy(&cb->sense, cb->errptr, 8);
	cb->errptr = cb->errptr + 4;

	cb->sense = 0x00;

	if (alert)
		sdlc_alert(cb);

} 	/***** setup_entry **********************************************/




/************************************************************************/
/*									*/
/* Name:	sdlc_alert						*/
/*									*/
/* Function:	fill in the sdlc alert data				*/
/*									*/
/* Notes:	this subroutine fills in all the subvector data for	*/
/*		an SDLC alertable errror				*/
/*									*/
/* Data									*/
/* Structures:	sdlc port control block 				*/
/*									*/
/* Returns:	void							*/
/*									*/
/************************************************************************/

void		sdlc_alert(cb)
register	PORT_CB	*cb;

{
	char	*vlptr;			/* vector length field pointer	*/
	char	temp_ns;		/* temporary ns value		*/
	int	tempint;


	/* Builds the generic Detail_Data area for alerts */

	/*
	** (8001) Additional Subvectors
	*/

	tempint = ADD_SUBVECS;
	bcopy (&tempint, cb->errptr, 2);
	cb->errptr = cb->errptr + 2;


	/*
	** (52) LAN Connection Subsystem Config
	*/

	/* save the position of vector length field and bump past */
	vlptr = cb->errptr;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SV_LCS_CONFIG;
	cb->errptr++;

	if (cb->station_type == PRIMARY)
	{
		/* (02) Remote link address */

		/* load the subfield length and bump */
		*cb->errptr = 3;
		cb->errptr++;

		/* load the subvector value and bump */
		*cb->errptr = SF52_REMOTE_ADDR;
		cb->errptr++;

		/* load the remote address value and bump */
		*cb->errptr = cb->active_ls->ll.sls.raddr_name[0];
		cb->errptr++;

		/* (04) Local link address/SAP */

		/* load the subfield length and bump */
		*cb->errptr = 3;
		cb->errptr++;

		/* load the subvector value and bump */
		*cb->errptr = SF52_LOCAL_ADDR;
		cb->errptr++;
		/* load the local address value and bump */
		*cb->errptr = cb->active_ls->ll.sls.raddr_name[0];
	}
	else	/* link is secondary */
	{
		/* (02) Remote link address */

		/* load the subfield length and bump */
		*cb->errptr = 3;
		cb->errptr++;

		/* load the subvector value and bump */
		*cb->errptr = SF52_REMOTE_ADDR;
		cb->errptr++;

		/* load the remote address value and bump */
		*cb->errptr = cb->active_ls->ll.sdl.secladd;
		cb->errptr++;

		/* (04) Local link address/SAP */

		/* load the subfield length and bump */
		*cb->errptr = 3;
		cb->errptr++;

		/* load the subvector value and bump */
		*cb->errptr = SF52_LOCAL_ADDR;
		cb->errptr++;

		/* load the local address value and bump */
		*cb->errptr = cb->active_ls->ll.sdl.secladd;
	}

	cb->errptr++;

	/*
	** (06) Link Station Attributes
	*/

	/* load the subfield length and bump */
	*cb->errptr = 4;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF52_LS_ATTR;
	cb->errptr++;

	/* load the link station role */

	if (cb->active_ls->ll.sls.flags & DLC_SLS_NEGO)
		*cb->errptr = SF52_NEGOTIABLE;
	else if (cb->active_ls->ll.sls.flags & DLC_SLS_STAT)
		*cb->errptr = SF52_PRIMARY;
	else
		*cb->errptr = SF52_SECONDARY;

	cb->errptr++;

	*cb->errptr = SF52_NODE_TYPE;
	cb->errptr++;

	/*
	** (07) Link Attributes
	*/

	/* load the subfield length and bump */
	*cb->errptr = 6;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF52_LINK_ATTR;
	cb->errptr++;

	if (cb->pl.esap.flags & DLC_ESAP_NTWK)
		*cb->errptr = SF52_NON_SW;
	else
		*cb->errptr = SF52_SWITCHED;
	cb->errptr++;

	*cb->errptr = SF52_HDX;
	cb->errptr++;

	*cb->errptr = SF52_SDLC;
	cb->errptr++;

	if (cb->pl.esap.flags & DLC_ESAP_LINK)
		*cb->errptr = SF52_MULTI_PT;
	else
		*cb->errptr = SF52_PT_TO_PT;
	cb->errptr++;


	/* load the length field back at the top of the vector */
	*vlptr = cb->errptr - vlptr;

	/*
	** (8C) Link Station Data
	*/

	/* save the position of vector length field and bump past */
	vlptr = cb->errptr;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SV_LS_DATA;
	cb->errptr++;

	/* (01) Current NS/NR Counts */

	/* load the subfield length and bump */
	*cb->errptr = 4;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_NS_NR;
	cb->errptr++;

	/* load the current NS (sent) count */
	*cb->errptr = cb->active_ls->ns;
	cb->errptr++;

	/* load the current NR (received) count */
	*cb->errptr = cb->active_ls->nr;
	cb->errptr++;

	/* (02) Outstanding Frame Count */

	/* load the subfield length and bump */
	*cb->errptr = 3;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_FRAMES_OUT;
	cb->errptr++;

	/* load the number of sent I-frames outstanding */
	if (cb->active_ls->ns < cb->active_ls->ack_nr)
		temp_ns = cb->active_ls->ns + MOD;
	else 
		temp_ns = cb->active_ls->ns;

	/*
	** the number of buffers outstanding is the number of buffers
	** the station has sent (temp_ns) less the number of buffers
	** the remote station has acknowledged
	*/
	*cb->errptr = temp_ns - cb->active_ls->ack_nr;
	cb->errptr++;

	/* (03) Last Control Received */

	/* load the subfield length and bump */
	*cb->errptr = 4;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_CNTL_RCVD;
	cb->errptr++;

	/* load the last received control byte values */
	*cb->errptr = cb->active_ls->last_sent;
	cb->errptr++;
	*cb->errptr = 0x00;
	cb->errptr++;

	/* (04) Last Control Sent */

	/* load the subfield length and bump */
	*cb->errptr = 4;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_CNTL_SENT;
	cb->errptr++;

	/* load the last sent control byte values */
	*cb->errptr = cb->active_ls->last_rcvd;
	cb->errptr++;
	*cb->errptr = 0x00;
	cb->errptr++;

	/* (05) Sequence Number Modulus */

	/* load the subfield length and bump */
	*cb->errptr = 3;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_MODULUS;
	cb->errptr++;

	/* load the modulus value */
	*cb->errptr = MOD;
	cb->errptr++;

	/* (06) Link Station State */

	/* load the subfield length and bump */
	*cb->errptr = 3;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_LS_STATE;
	cb->errptr++;

	/* default the link station state to NOT busy; */
	*cb->errptr = 0;

	if (cb->active_ls->sub_state & DLC_LOCAL_BUSY)
	{
		*cb->errptr |= SF8C_LS_LBUSY;
	}
	else if (cb->active_ls->sub_state & DLC_REMOTE_BUSY)
	{
		*cb->errptr |= SF8C_LS_RBUSY;
	}
	cb->errptr++;

	/* (07) LLC Reply Timer Expiration Count */

	/* load the subfield length and bump */
	*cb->errptr = 4;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_REPOLLS;
	cb->errptr++;

	/* load the number of repoll timeouts */
	bcopy(&(cb->active_ls->ct.ttl_repolls_sent)+2, cb->errptr, 2);
	cb->errptr = cb->errptr + 2;

	/* (08) Last Received NR Count */

	/* load the subfield length and bump */
	*cb->errptr = 3;
	cb->errptr++;

	/* load the subvector value and bump */
	*cb->errptr = SF8C_RCVD_NR;
	cb->errptr++;

	/* load the link station's recieved ack value */
	*cb->errptr = cb->active_ls->ack_nr;
	cb->errptr++;

	/* load the length field back at the top of the vector */
	*vlptr = cb->errptr - vlptr;

}	/***** sdl_alert ************************************************/
