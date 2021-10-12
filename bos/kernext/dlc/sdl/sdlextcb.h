/* @(#) 54    1.3  6/18/90  16:22:34 */

/*
 * COMPONENT_NAME: (sysxdlcs) Synchronous Data Link Control Device Driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1987, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SDLEXTCB
#define _H_SDLEXTCB

#include <sys/types.h>

/*
**	These fields make up the sdlc product specific data for
**	the ioctl call with the DLC_START_LS option
*/
struct	sdl_start_psd
{
	uchar_t	duplex;		/* link station xmit/receive capability	*/
	uchar_t	secladd;	/* secondary station local address	*/
	uchar_t	prirpth;	/* primary repoll timeout threshold	*/
	uchar_t	priilto;	/* primary idle list timeout		*/
	uchar_t	prislto;	/* currently not supported		*/
	uchar_t	retxct;		/* retransmit count ceiling		*/
	uchar_t	retxth;		/* retransmit count threshold		*/
	uchar_t	reserved;	/* currently not used			*/
};

#endif	/* _H_SDLEXTCB */
