static char sccsid[] = "@(#)16	1.1  src/bos/usr/lpp/kls/dictutil/hufout.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:34";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hufout.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hufout.c
 *
 *  Description:  output data of user dictionary to file.
 *
 *  Functions:    hufout()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory package.                              */
#include <string.h>     /* String package.      */
#include <fcntl.h>      /* File contorol package.       */
#include "hut.h"        /* Dictionary Utility Define File.   */


int     hufout ( fd , topptr )

int     fd;             /* file discripter      */
UDCS    *topptr;        /* pointer to top of UDCS.      */

{
        UDCS    *curptr;        /* pointer to current of UDCS.          */
	UDCS    *scp;           /* work pointer to UDCS */


	uchar   *wkey;
	uchar   wkeylen;
	int     wrc;
	int     i;              /* work variable.                       */

	/* bracket data */
	static uchar *bracket[2] = { "{", "}" };

	/* head label data.     */
	uchar    headarea[70];
	static  uchar    *hl3d_d = "User Dictionary list\n";

#define HEAD_X ((U_KEY_MX + U_CAN_MX + 4 - strlen(hl3d_d)) / 2)

	struct  outarea {
		char    keylbracket[1];
		char    key[U_KEY_MX];
		char    keyrbracket[1];
		char    filler[2];	/* column break */
		char    candlbracket[1];
		char    cand[U_CAN_MX];
		char    candrbracket[1];
		char    cr;		/* line break */
	} outarea;


/* 0.
*     print header
*/

	memset(headarea,0x20,HEAD_X);
	headarea[HEAD_X] = NULL;
	strcat(headarea,hl3d_d);

	wrc = write(fd,headarea,strlen(headarea));
	if( wrc < 0 )
	{
		return( IUFAIL );
	};

	/* write blank line     */
	headarea[0] = 0x0a;
	wrc = write(fd,headarea,1);

/* 1.
*     print data
*/
	/* set out area */
	memcpy(outarea.candlbracket,bracket[0],1);
	memcpy(outarea.candrbracket,bracket[1],1);
	memset(outarea.filler,0x20,2);
	outarea.cr = 0x0a;

	/* set work key & keylen      */
	wkey = NULL;
	wkeylen = 0;
	scp = topptr;

	/* prcess loop. (loopend mark is '!!!!')        */
	while(TRUE)
	{
		if(scp == NULL)
		{
			break;
		};

	/* check this data is valid. (endif mark is '####')        */
		if((scp->status == U_S_INIT)  ||
			(scp->status == U_S_KEYA)  ||
				(scp->status == U_S_CANU) )
		{

		/* clear data field   */
			memset(outarea.key,0x20,U_KEY_MX);
			memset(outarea.cand,0x20,U_CAN_MX);
			memset(outarea.keylbracket,0x20,1);
			memset(outarea.keyrbracket,0x20,1);
	
		/* check need to print key        */
			if(    (scp->keylen != wkeylen)
				|| (memcmp(scp->key,wkey,wkeylen) != 0) )
			{
			/* case : need to key      */
	
			/*  keep current key data  */
				wkey = scp->key;
				wkeylen = scp->keylen;
	
			/* set data to outarea      */
				memcpy(outarea.keylbracket,bracket[0],1);
				memcpy(outarea.keyrbracket,bracket[1],1);
				memcpy(outarea.key,wkey,(int)(wkeylen));
			};
	
		/* set cand   */
			memcpy(outarea.cand,scp->cand,(int)(scp->candlen));
	
		/* print out */
			wrc = write(fd,&outarea,sizeof(outarea));
			if(wrc < 0)
			{
				return( IUFAIL );
			};
		};  /* endif ####     */

	/* set next data      */
		scp = scp->nx_pos;
	}; /* endloop !!!!      */

	return(IUSUCC);

};
