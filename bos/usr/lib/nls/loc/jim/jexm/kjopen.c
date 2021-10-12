static char sccsid[] = "@(#)92	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jexm/kjopen.c, libKJI, bos411, 9428A410j 7/23/92 01:56:26";
/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.3		11/11/88
 *	exkjopen(kjcblk,csid,extinf)
 *	KJCBLK	**kjcblk;						ptr to ptr to KJCBLK
 *	short	csid;							character set id
 *	EXTINF	*extinf;						ptr to EXTINF
 *
 *	 Allocate KJCBLK, KJSVPT, HLATST, ISTRING, IHLATST, MSIMAGE, MHIMAGE,
 *	SDTTBL, SDTWORK and MSTRING.
 *	 Open the Monitor.
 *
 *  11/18/87 first write.
 *  05/23/88 add '$' supress flag.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

int	exerr;									/* error no.					*/

exkjopen(kjcblk,csid,extinf)
KJCBLK	**kjcblk;							/* ptr to ptr to KJCBLK			*/
short	csid;								/* character set id				*/
EXTINF	*extinf;							/* ptr to EXTINF				*/
{
	/*
	 *	temporary variable
	 */

	KJSVPT	*kjsvpt;						/* ptr to KJSVPT				*/
	BUFFER	*buffer;						/* ptr to BUFFER				*/
	EXTINF	*mextinf;						/* ptr to MEXTINF				*/
	int		i;								/* loop counter					*/
	char	*p;								/* general pointer				*/
	int		*ptr[10];						/* temporaly ptr save area		*/
	unsigned int	size[10];				/* temporaly size save area		*/
	int		maxstc;							/* temporaly maxstc save area	*/
	int		err;							/* error flag					*/
	int		monret;							/* Monitor return code			*/

	/*
	 *	allocation of contorol blocks and buffers
	 */

	maxstc = extinf->maxstc;
	i = 0;
	size[i] = (unsigned int)(sizeof(KJCBLK));				/* i = 0 KJCBLK	*/
	i++;
	size[i] = (unsigned int)(sizeof(KJSVPT));				/* i = 1 KJSVPT	*/
	i++;
	size[i] = (unsigned int)maxstc + 1;						/* i = 2 HLATST	*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 3 ISTRING*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 4 IHLATST*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 5 MSIMAGE*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 6 MHIMAGE*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 7 SDTTBL	*/
	i++;
	size[i] = (unsigned int)(maxstc + MAXLENOFKJ + 2);		/* i = 8 SDTWORK*/
	i++;
	size[i] = (unsigned int)(2 * maxstc + MAXLENOFKJ + 2);	/* i = 9 MSTRING*/

	for(i = 0,err = FALSE;i < 10 && !err;i++){
		ptr[i] = (int *)(int)malloc(size[i]);
		if(ptr[i] == NULL)
			err = TRUE;
	}
	if(err){
		while(--i) free((char *)ptr[i - 1]);
		return EXMONALLOCERR;
	}
	kjsvpt = (KJSVPT *)ptr[1];				/* ptr to KJSVPT				*/
	buffer = &kjsvpt->buffer;
	
	/*
	 *	initialization of control blocks
	 */

	for(i = 0, p = (char *)ptr[0];i < sizeof(KJCBLK);i++, *p++ = NULL);
	for(i = 0, p = (char *)ptr[1];i < sizeof(KJSVPT);i++, *p++ = NULL);
	
	/*
	 *	Open the Monitor.
	 */

	kjsvpt->extinf = *extinf;				/* copy EXTINF into KJSVPT		*/
	mextinf = &kjsvpt->mextinf;				/* ptr to MEXTINF				*/
	*mextinf = *extinf;						/* copy EXTINF to MEXTINF		*/
	mextinf->maxstc = 2 * maxstc + MAXLENOFKJ + 2;

	exerr = monret = _Jopen(&kjsvpt->mkjcblk,csid,mextinf);
											/* open the Monitor				*/
	if(monret != 0){
		for(i = 0;i < 10;i++) free((char *)ptr[i]);
		return monret;
	}

	/*
	 *	set parameters in KJCBLK and KJSVPT
	 */

	*kjcblk = (KJCBLK *)ptr[0];				/* ptr to KJCBLK				*/
	(*kjcblk)->kjsvpt = (KJSVPT *)ptr[1];	/* ptr to KJSVPT				*/
	(*kjcblk)->hlatst = (char *)ptr[2];		/* ptr to HLATST				*/
	buffer->hlatst    = (char *)ptr[2];		/* ptr to HLATST				*/
	buffer->istring   = (char *)ptr[3];		/* ptr to ISTRING				*/
	buffer->ihlatst   = (char *)ptr[4];		/* ptr to IHLATST				*/
	buffer->maxis     = size[3];			/* max size of ISTRING			*/
	buffer->msimage   = (char *)ptr[5];		/* ptr to MSIMAGE				*/
	buffer->mhimage   = (char *)ptr[6];		/* ptr to MHIMAGE				*/
	buffer->maxim     = size[5];			/* max size of MSIMAGE			*/
	buffer->sdttbl    = (char *)ptr[7];		/* ptr to SDTTBL				*/
	buffer->sdtwork   = (char *)ptr[8];		/* ptr to SDTWORK				*/
	buffer->maxsd     = size[7];			/* max size of SDTTBL			*/
	kjsvpt->mkjcblk->string = (char *)ptr[9];/* ptr to MSTRING				*/

	(*kjcblk)->length = sizeof(KJCBLK);
	(*kjcblk)->id = kjsvpt->mkjcblk->id;
	(*kjcblk)->tracep = kjsvpt->mkjcblk->tracep;
	(*kjcblk)->aux1 = kjsvpt->mkjcblk->aux1;
	(*kjcblk)->hlata1 = kjsvpt->mkjcblk->hlata1;
	(*kjcblk)->maxa1c = kjsvpt->mkjcblk->maxa1c;
	(*kjcblk)->maxa1r = kjsvpt->mkjcblk->maxa1r;
	(*kjcblk)->csid = kjsvpt->mkjcblk->csid;

	kjsvpt->initflg = NOTINITIALIZED;

	/*
	 *	Always TRUE:  The dollar suppress function no longer exists.
	 */
	kjsvpt->dollar = TRUE;

	return	EXMONNOERR;
}
