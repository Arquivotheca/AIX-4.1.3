static char sccsid[] = "@(#)99	1.2  src/bos/usr/lib/nls/loc/jim/jexm/rkcutil.c, libKJI, bos411, 9428A410j 6/6/91 11:22:50";

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
 *	rkcinit(phase1)
 *	PHASE1	*phase1;
 *
 *	rkcpush(mode, phase1,c)
 *	int		mode;
 *	PHASE1	*phase1;
 *	char	c;
 *
 *  02/17/88  version 5.
 */

#include <exmdefs.h>
#include <exmctrl.h>


void	rkcinit(phase1)
PHASE1	*phase1;							/* ptr to PHASE1 struct			*/
{
	/*
	 *	decleration of temp variables
	 */

	int		i;								/* loop counter					*/

	/*
	 *	clear the RKC rest buffer
	 */

	for(i = 0;i < P1BUFSIZE;i++)			/* clear phase1->rest buffer	*/
		phase1->rest[i] = 0;
	phase1->rlen  = 0;						/* clear ROMAJI length			*/
	phase1->klen  = 0;						/* clear KANA length			*/
	phase1->rslen = 0;						/* clear REST length			*/
}


void	rkcpush(mode, phase1, c)
int		mode;								/* Katakana or Hiragana         */
PHASE1	*phase1;							/* ptr to PHASE1 struct			*/
char	c;									/* input char to rkc processor	*/
{
	int		i;								/* loop counter					*/
	short	rlen;							/* temp romaji length			*/
	short	klen;							/* temp kanji length			*/
	short	rslen;							/* temp rest length				*/

	/*
	 *	push the input into RKC
	 */

	for (i = 0; i < phase1->rslen; i++)		/* move from rest to romaji		*/
		phase1->romaji[i] = phase1->rest[i];
	phase1->romaji[i] = c;					/* append char c to romaji		*/
	rlen = i + 1;							/* set romaji length			*/
	klen = rslen = 0;						/* clear klen and rslen			*/
	for (i = 0; i < P1BUFSIZE; i++) {		/* clear rest and kana buffer	*/
		phase1->kana[i] = 0;
		phase1->rest[i] = 0;
	}
	_Rkc(SGLKATA, phase1->romaji, rlen, phase1->kana, &klen,
		phase1->rest, &rslen);				/* RKC call						*/
	phase1->klen  = klen;					/* set kanji length in PHASE1	*/
	if (mode == DBLHIRA) {
		char	*ptr;
		char	kana[2 * P1BUFSIZE];
		char	dummy[2 * P1BUFSIZE];
		short	dummylen;
		char	sc;
		if (phase1->klen == 2 &&
			phase1->kana[0] == 0xb3 && phase1->kana[1] == 0xde)
				phase1->kana[0] = 0xcc;
		else {
			_Rkc(DBLHIRA, phase1->romaji, rlen, kana, &klen, dummy, &dummylen);
			sc = 0;
			ptr = kana;
			while (klen > 0) {
				if (ptr[0] == 0x82) {
					if (ptr[1] == 0xec)
						sc = HIRAGANA_XWA;
					else if (ptr[1] == 0xee)
						sc = HIRAGANA_WI;
					else if (ptr[1] == 0xef)
						sc = HIRAGANA_WE;
				}
				klen -= 2;
				ptr += 2;
			}
			if (sc) {
				ptr = phase1->kana;
				for (i = 0; i < phase1->klen; i++, ptr++)
					if (*ptr == 0xdc || *ptr == 0xb2 || *ptr == 0xb4) {
						*ptr = sc;
						break;
					}
			}
		}
	}
	else if (mode == DBLKATA) {
		char	*ptr;
		char	kana[2 * P1BUFSIZE];
		char	dummy[2 * P1BUFSIZE];
		short	dummylen;
		char	sc;
		_Rkc(DBLKATA, phase1->romaji, rlen, kana, &klen, dummy, &dummylen);
		sc = 0;
		ptr = kana;
		while (klen > 0) {
			if (ptr[0] == 0x83) {
				if (ptr[1] == 0x8e)
					sc = KATAKANA_XWA;
				else if (ptr[1] == 0x90)
					sc = KATAKANA_WI;
				else if (ptr[1] == 0x91)
					sc = KATAKANA_WE;
				else if (ptr[1] == 0x94)
					sc = KATAKANA_VU;
				else if (ptr[1] == 0x95)
					sc = KATAKANA_XKA;
				else if (ptr[1] == 0x96)
					sc = KATAKANA_XKE;
			}
			klen -= 2;
			ptr += 2;
		}
		if (sc) {
			ptr = phase1->kana;
			for (i = 0; i < phase1->klen; i++, ptr++)
				if (*ptr == 0xdc || *ptr == 0xb2 || *ptr == 0xb4 ||
					*ptr == 0xb6 || *ptr == 0xb9) {
					*ptr = sc;
					break;
				}
				else if (*ptr == 0xcc) {
					*ptr++ = sc;
					phase1->klen--;
					for (; i < phase1->klen; i++, ptr++)
						*ptr = *(ptr + 1);
					break;
				}
		}
	}
	phase1->kana[phase1->klen] = 0;			/* make it NULL terminate str	*/
	phase1->rslen = rslen;					/* set rest length in PHASE1	*/
}
