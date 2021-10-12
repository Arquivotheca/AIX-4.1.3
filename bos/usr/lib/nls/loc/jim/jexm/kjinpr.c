static char sccsid[] = "@(#)91	1.3  src/bos/usr/lib/nls/loc/jim/jexm/kjinpr.c, libKJI, bos411, 9428A410j 5/18/93 05:25:32";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method - Ex Monitor
 *
 * FUNCTIONS : kjinpr
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.3		11/11/88
 *	exkjinpr(kjcblk)
 *		argument
 *			KJCBLK	*kjcblk;		pointer to KJCBLK
 *
 *		The ExMon input processing routine.
 *
 *  11/18/87 first write.
 *  01/17/88 version 5.
 *	06/28/88 version 6. add the set cursor as a psuedo code.
 *
 */

#include <exmdefs.h>
#include <exmctrl.h>

exkjinpr(kjcblk)
KJCBLK	*kjcblk;							/* ptr to KJCBLK				*/
{
	/*
	 *	decleration of temp variables
	 */

	KJSVPT	*kjsvpt		= kjcblk->kjsvpt;	/* ptr to KJSVPT				*/
	PHASE1	*phase1		= &kjsvpt->phase1;	/* ptr to PHASE1 struct			*/
	PHASE2	*phase2		= &kjsvpt->phase2;	/* ptr to PHASE2 struct			*/
	int		i,i1,i2;						/* loop counter					*/
	KJCBLK	*mkjcblk	= kjsvpt->mkjcblk;	/* ptr to MKJCBLK				*/
	int		ncodes;							/* number of input codes		*/
	char	type[3];						/* array of input type			*/
	char	code[3];						/* array of input code			*/

	/*
	 *	decleration of externl functions
	 */

	extern	void	rkcinit();				/* rkc init routine				*/
	extern	void	rkcpush();				/* rkc input routine			*/
	extern	char	_Mktec();				/* kana to alphanum thru keymap	*/
	extern	char	ktoan();				/* kana to alphanum thru keymap	*/
	extern	void	m1allcand();			/* allcandidates request		*/	
	extern	void	m1bs();					/* back space					*/
	extern	void	m1char();				/* character input				*/
	extern	void	m1conv();				/* conversion key				*/
	extern	void	m1convsw();				/* conversion mode switch		*/
	extern	void	m1curright();			/* cursor right					*/
	extern	void	m1curleft();			/* cursor left					*/
	extern	void	m1curup();				/* cursor up					*/
	extern	void	m1del();				/* delete						*/
	extern	void	m1diagon();				/* diagnosys on					*/
	extern	void	m1diagoff();			/* diagnosys off				*/
	extern	void	m1enter();				/* enter key					*/
	extern	void	m1ereof();				/* ErEOF						*/
	extern	void	m1erinput();			/* ErInput						*/
	extern	void	m1cchar();				/* confirmation for char input	*/
	extern	void	m1call();				/* confirmation for char input	*/
	extern	void	m1cbs();				/* confirmation for char input	*/
	extern	void	m1chkc();				/* confirmation for char input	*/
	extern	void	m1cdel();				/* confirmation for char input	*/
	extern	void	m1insert();				/* insert toggle				*/
	extern	void	m1kjnum();				/* Kanji Number input request	*/
	extern	void	m1noconv();				/* no conversion				*/
	extern	void	m1nop();				/* no operation					*/
	extern	void	m1precand();			/* previous candidates			*/
	extern	void	m1reset();				/* reset						*/
	extern	void	m1yomi();				/* bunsetsu yomi				*/
	extern	void	m1setcur();				/* set cursor position			*/
	extern	void	m2cmp();				/* compare character			*/
	extern	void	m2cmpmv();				/* compare and move character	*/

/*
 *
 *	phase1 of exkjinpr
 *
 */

	/*
	 *	check whether the ExMon is already opened or not
	 */

	if (kjcblk == 0 || kjcblk->kjsvpt == 0)
		return EXMONNOTOPEN;				/* ERROR return					*/

	/*
	 *	check whether the ExMon is already initialized or not
	 */

	if (kjsvpt->initflg == NOTINITIALIZED)
		return EXMONNOTINIT;				/* ERROR return					*/

	/*
	 *	phase1 INIT step
	 */

	kjcblk->shift  = 0;						/* clear shift change flag		*/
	kjcblk->beep   = 0;						/* clear beep flag				*/
	kjcblk->chpos  = 0;						/* clear changed pos parm		*/
	kjcblk->chlen  = 0;						/* clear changed len parm		*/
	kjcblk->chpsa1 = 0;						/* clear chgd pos parm for AUX1	*/
	kjcblk->chlna1 = 0;						/* clear chgd len parm for AUX1	*/
	kjcblk->discrd = phase2->discrd = DISCARD;

	if (phase1->diaflg == ON) {				/* diagnosis 2-byte sequence.	*/
		if (kjcblk->type == TYPE2 && kjcblk->code == PCURUP)
			if (kjcblk->trace)
				phase1->diaflg = OFF;
			else
				kjcblk->code = PDIAGON;
		else if (kjcblk->type == TYPE2 && kjcblk->code == PCURDOWN)
			if (kjcblk->trace)
				kjcblk->code = PDIAGOFF;
			else
				phase1->diaflg = OFF;
		else
			phase1->diaflg = OFF;
	}
	else {									/* phase1->diaflg == OFF		*/
		if (kjcblk->type == TYPE2 && kjcblk->code == PDIAGNOS)
			phase1->diaflg = ON;
	}

	/*
	 *	'$' supress routine.
	 */

	if (!kjsvpt->dollar && kjcblk->shift1 == ALPHANUM &&
		kjcblk->type == TYPE1 && kjcblk->code == '$') {
		if(kjsvpt->beep)
			kjcblk->beep   = phase2->beep;
		else
			kjcblk->beep   = OFF;
		return EXMONNOERR;
	}

	/*
	 * RKC buffer clear.
	 */

	if (kjcblk->shift2 == RKC_ON && kjcblk->shift1 != ALPHANUM &&
		kjcblk->type == TYPE2) {
		switch (kjcblk->code) {
		case PCONVERT or PNOCONV :
			if (phase1->rslen == 1 && phase1->rest[0] == 'n') {
				type[0] = TYPE1;
				code[0] = KATAKANA_MI;
				type[1] = TYPE1;
				code[1] = KATAKANA_MI;
				type[2] = kjcblk->type;
				code[2] = kjcblk->code;
				ncodes  = 3;
			}
			else {
				type[0] = kjcblk->type;
				code[0] = kjcblk->code;
				ncodes  = 1;
			}
			break;

		case PBACKSPACE :
			if (phase1->rslen > 0)
				ncodes  = 0;
			else {
				type[0] = kjcblk->type;
				code[0] = kjcblk->code;
				ncodes  = 1;
			}
			break;

		default	:
			type[0] = kjcblk->type;
			code[0] = kjcblk->code;
			ncodes  = 1;
			break;
		}									/* end of switch				*/
		rkcinit(phase1);					/* RKC buffer clear				*/
	}
	else {
		type[0] = kjcblk->type;
		code[0] = kjcblk->code;
		ncodes  = 1;
	}

	/*
	 *	shift, rkc and synonymous function
	 */

	for (i1 = i2 = 0; i1 < ncodes; i1++) {
		if (type[i1] == TYPE1) {			/* type[i] == TYPE1				*/
			switch (kjcblk->shift1) {
			case ALPHANUM :
				phase1->type[i2] = TYPE1;
				phase1->code[i2] = code[i1];
				i2++;
				break;

			case KATAKANA :
				if (kjcblk->axuse1 == USE || kjcblk->cnvsts == OVERFLOW) {
					phase1->type[i2] = TYPE1;
					if (kjcblk->shift3 == SINGLE)
						phase1->code[i2] = ktoan(code[i1]);
					else
						phase1->code[i2] = code[i1];
					i2++;
				}
				else {
					if (kjcblk->shift2 == RKC_ON) {
						if (kjcblk->shift3 == SINGLE)
							rkcpush(SGLKATA,phase1,_Mktec(code[i1]));
						else
							rkcpush(DBLKATA,phase1,_Mktec(code[i1]));
						if (phase1->klen > 0) {	/* rkc output				*/
							for (i = 0; i < phase1->klen; i++) {
								phase1->type[i2] = TYPE1;
								phase1->code[i2] = phase1->kana[i];
								i2++;
							}
						}
					}
					else {						/* RKC_OFF					*/
						phase1->type[i2] = TYPE1;
						phase1->code[i2] = code[i1];
						i2++;
					}
				}
				break;

			case HIRAGANA :
				if (kjcblk->axuse1 == USE || kjcblk->cnvsts == OVERFLOW ||
					kjcblk->shift2 == RKC_OFF) {
					phase1->type[i2] = TYPE1;
					phase1->code[i2] = code[i1];
					i2++;
				}
				else {							/* RKC_ON					*/
					rkcpush(DBLHIRA,phase1,_Mktec(code[i1]));
					if (phase1->klen > 0) {		/* rkc output				*/
						for (i = 0; i < phase1->klen; i++) {
							phase1->type[i2] = TYPE1;
							phase1->code[i2] = phase1->kana[i];
							i2++;
						}
					}
				}
				break;

			}	/* end of switch */
		}
		else {								/* type[i] == TYPE2				*/
			switch (code[i1]) {
			case PALPHANUM :				/* AN shift						*/
				if (kjcblk->shift1 == KATAKANA) {
					if (kjcblk->shift3 == DOUBLE)
						inpr2(PALPHANUM)
					kjcblk->shift1 = ALPHANUM;
					kjcblk->shift |= SHIFT1;
				}
				else if (kjcblk->shift1 == HIRAGANA) {
					inpr2(PALPHANUM)
					kjcblk->shift1 = ALPHANUM;
					kjcblk->shift |= SHIFT1;
				}
				break;

			case PKATAKANA :				/* Katakana shift				*/
				if (kjcblk->shift1 == ALPHANUM){
					if (kjcblk->shift3 == DOUBLE)
						inpr2(PKATAKANA)
					kjcblk->shift1 = KATAKANA;
					kjcblk->shift |= SHIFT1;
				}
				else if (kjcblk->shift1 == HIRAGANA) {
					if (kjcblk->shift3 == DOUBLE)
						inpr2(PKATAKANA)
					else
						inpr2(PALPHANUM)
					kjcblk->shift1 = KATAKANA;
					kjcblk->shift |= SHIFT1;
				}
				break;

			case PHIRAGANA :				/* Hiragana shift				*/
				if (kjcblk->shift1 != HIRAGANA) {
					inpr2(PHIRAGANA)
					kjcblk->shift1 = HIRAGANA;
					kjcblk->shift |= SHIFT1;
				}
				break;

			case PRKC :						/* RKC shift					*/
				if (kjcblk->shift2 == RKC_ON)
					kjcblk->shift2 = RKC_OFF;
				else
					kjcblk->shift2 = RKC_ON;
				kjcblk->shift |= SHIFT2;
				break;

			case PSGLDBL :					/* Single/Double shift			*/
				if (kjcblk->shift3 == SINGLE) {
					if(kjcblk->shift1 == KATAKANA)
						inpr2(PKATAKANA)
					kjcblk->shift3 = DOUBLE;
				}
				else{
					if(kjcblk->shift1 == KATAKANA)
						inpr2(PALPHANUM)
					kjcblk->shift3 = SINGLE;
				}
				kjcblk->shift |= SHIFT3;
				break;

			case PCURDLEFT :				/* cursor move double left		*/
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PCURLEFT;
				i2++;
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PCURLEFT;
				i2++;
				break;

			case PCURDRIGHT :				/* cursor move double right		*/
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PCURRIGHT;
				i2++;
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PCURRIGHT;
				i2++;
				break;

			case PENTER or PACTION or PCARRIGE :/* enter, action or CR		*/
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PENTER;
				i2++;
				break;

			case PCURDOWN :					/* cursor up / down				*/
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = PCURUP;
				i2++;
				break;

			default	:						/* other codes					*/
				phase1->type[i2] = TYPE2;
				phase1->code[i2] = code[i1];
				i2++;
				break;
			}
		}
	}										/* end of for					*/
	phase1->ncodes = i2;					/* number of codes				*/

	/*
	 *	preparation for PHASE2
	 */

	for (i1 = i2 = 0; i1 < phase1->ncodes; i1++) {
		if (phase1->type[i1] == TYPE1) {	/* prefixed by PCCHAR			*/
			phase2->type[i2] = TYPE2;
			phase2->code[i2] = PCCHAR;
			i2++;
		}
		else {
			switch (phase1->code[i1]) {
			case PENTER or PKJNUM or PEREOF or PERINPUT or PINSERT or PCONVSW
				 or PDIAGON or PDIAGOFF or PCURUP:
				phase2->type[i2] = TYPE2;
				phase2->code[i2] = PCALL;
				i2++;
				break;

			case PBACKSPACE :
				phase2->type[i2] = TYPE2;
				phase2->code[i2] = PCBS;
				i2++;
				break;

			case PDELETE :
				phase2->type[i2] = TYPE2;
				phase2->code[i2] = PCDEL;
				i2++;
				break;

			case PCURLEFT or PCURRIGHT or PALLCAND or PCONVERT or PPRECAND
				 or PSETCUR :
				phase2->type[i2] = TYPE2;
				phase2->code[i2] = PCHKC;
				i2++;
				break;
			}
		}
		phase2->type[i2] = phase1->type[i1];
		phase2->code[i2] = phase1->code[i1];
		i2++;
	}										/* end of for					*/
	phase2->ncodes = i2;

/*
 *
 *	phase2 of exkjinpr
 *
 */

if (phase2->ncodes > 0)						/* if input for phase2 exists	*/
{											/* start phase2					*/

	BUFFER	*buffer		= &kjsvpt->buffer;	/* ptr to BUFFER struct			*/
	char	*string		= kjcblk->string;	/* ptr to STRING				*/
	char	*hlatst		= kjcblk->hlatst;	/* ptr to HLATST				*/
	char	*istring	= buffer->istring;	/* ptr to ISTRING				*/
	char	*ihlatst	= buffer->ihlatst;	/* ptr to IHLATST				*/
	char	*msimage	= buffer->msimage;	/* ptr to MSIMAGE				*/
	char	*mhimage	= buffer->mhimage;	/* ptr to MHIMAGE				*/
	int		schg;							/* start pos of changed area	*/
	int		echg;							/* end pos of changed area		*/
	int		imax;							/* loop limit					*/
	int		lastch;							/* save last char position		*/

	/*
	 *	definition of static variables
	 */

	static	void	(*m1func[])() = {		/* array of pointer to main1	*/
											/* functions					*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x00 - 0x03	*/
		m1nop,		m1conv,		m1noconv,	m1allcand,		/* 0x04 - 0x07	*/
		m1nop,		m1kjnum,	m1convsw,	m1nop,			/* 0x08 - 0x0b	*/
		m1precand,	m1nop,		m1nop,		m1nop,			/* 0x0c - 0x0f	*/
		m1nop,		m1nop,		m1yomi,		m1nop,			/* 0x10 - 0x13	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x14 - 0x17	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x18 - 0x1b	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x1c - 0x1f	*/
		m1enter,	m1nop,		m1nop,		m1reset,		/* 0x20 - 0x23	*/
		m1curright,	m1curleft,	m1curup,	m1nop,			/* 0x24 - 0x27	*/
		m1nop,		m1nop,		m1ereof,	m1erinput,		/* 0x28 - 0x2b	*/
		m1insert,	m1del,		m1bs,		m1nop,			/* 0x2c - 0x2f	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x30 - 0x33	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x34 - 0x37	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x38 - 0x3b	*/
		m1nop,		m1nop,		m1nop,		m1nop,			/* 0x3c - 0x3f	*/
		m1nop,		m1diagon,	m1diagoff,	m1nop,			/* 0x40 - 0x43	*/
		m1call,		m1cbs,		m1cchar,	m1cdel,			/* 0x44 - 0x47	*/
		m1chkc,		m1nop,		m1nop,		m1nop,			/* 0x48 - 0x4b	*/
		m1setcur,	m1nop,		m1nop,		m1nop,			/* 0x4c - 0x4f	*/
	};

	/*
	 *	phase2 INIT step	initialize of PHASE2 structure
	 */

	if (kjcblk->shift1 == HIRAGANA)
		phase2->shift3 = DOUBLE;
	else									/* shift1 != HIRAGANA			*/
		phase2->shift3 = kjcblk->shift3;
	phase2->repins = kjcblk->repins;
	phase2->beep = OFF;
	phase2->curst = kjcblk->curcol;			/* cursor pos in STRING			*/
	phase2->lastst = kjcblk->lastch;		/* last char pos in STRING		*/
	phase2->moncall = NO;
	phase2->setcsc = kjcblk->setcsc;		/* set cursor position			*/

	/*
	 *	phase2 main1 loop
	 */

	for (i = 0; i < phase2->ncodes; i++) {
#ifdef DEBUG
		printstate("before state = ", phase2->state);
#endif
		if(phase2->type[i] == TYPE1)			/* TYPE1					*/
			m1char(kjsvpt, phase2->code[i]);
		else if(phase2->code[i] < MAXPSEUDO)	/* TYPE2					*/
			(*m1func[phase2->code[i]])(kjsvpt);
		else									/* invalid					*/
			return EXMONNOERR;
#ifdef DEBUG
		printstate("  after state = ", phase2->state);
		printf("\n");
#endif
	}

	/*
	 *	phase2 main2
	 */

	if (phase2->state != OVF) {				/* if not in overflow state		*/
		if (phase2->state == NCS) {			/* NCS							*/
			kjcblk->cnvsts = FINISHED;
			lastch = phase2->lastis;
			if (lastch > buffer->actcol) {	/* OVF condition is detected	*/
				phase2->state = OVF;
				phase2->beep = ON;
				kjcblk->cnvsts = OVERFLOW;
				schg = echg = 0;
			}
			else {							/* didn't overflow				*/
				if (phase2->lastst > lastch) {
					echg = phase2->lastst;
					imax = lastch;
					for (i = imax; i < phase2->lastst; i++)
						string[i] = hlatst[i] = 0;
				}
				else if (phase2->lastst < lastch) {
					echg = lastch;
					imax = lastch;
				}
				else {
					echg = 0;
					for (i = lastch - 1; i >= 0 && echg == 0; )
						m2cmp(istring, ihlatst, i, string, hlatst,
							i, &i, &echg);
					imax = echg;
				}
				for (i = 0, schg = imax; i < imax; ) {
					if(schg == imax)
						m2cmpmv(istring, ihlatst, i, string, hlatst,
							i, &i, &schg);
					else {					/* chg area already found		*/
						string[i] = istring[i];
						hlatst[i] = ihlatst[i];
						i += 1;
					}
				}
				phase2->curst = phase2->curis;
				if ((hlatst[phase2->curst] & KJMASK) == KJ1st)
					kjcblk->curlen = CURATD;
				else
					kjcblk->curlen = CURATS;
				phase2->lastst = lastch;
			}
		}
	/**** insert mode but not NCS ****/
		else if (kjcblk->repins == INSERT) {
			if (phase2->state == HK1 || phase2->state == HK2)
				kjcblk->cnvsts = FINISHED;
			else
				kjcblk->cnvsts = GOINGON;
			lastch = phase2->lastis + phase2->lastim;
			if (lastch > buffer->actcol) {	/* OVF condition is detected	*/
				phase2->state = OVF;
				phase2->beep = ON;
				kjcblk->cnvsts = OVERFLOW;
				schg = echg = 0;
			}
			else {							/* didn't overflow				*/
				if (phase2->lastst > lastch) {
					echg = phase2->lastst;
					imax = lastch;
					for (i = imax; i < phase2->lastst; i++)
						string[i] = hlatst[i] = 0;
				}
				else if (phase2->lastst < lastch) {
					echg = lastch;
					imax = lastch;
				}
				else {
					echg = 0;
					for (i = lastch - 1; i >= 0 && echg == 0; ) {
						if (i >= phase2->topim + phase2->lastim)
							m2cmp(istring, ihlatst, i - phase2->lastim,
								string, hlatst, i, &i, &echg);
						else if (i >= phase2->topim)
							m2cmp(msimage,mhimage,i - phase2->topim,
								string, hlatst, i, &i, &echg);
						else				/* i < phase2->topim			*/
							m2cmp(istring, ihlatst, i, string, hlatst,
								i, &i, &echg);
					}
					imax = echg;
				}
				for (i = 0, schg = imax; i < imax; ){
					if (schg == imax) {
						if (i < phase2->topim)
							m2cmpmv(istring, ihlatst, i, string, hlatst,
								i, &i, &schg);
						else if (i < phase2->topim + phase2->lastim)
							m2cmpmv(msimage, mhimage, i - phase2->topim,
								string, hlatst, i, &i, &schg);
						else				/* i >= phase2->botmstr			*/
							m2cmpmv(istring, ihlatst, i - phase2->lastim,
								string, hlatst, i, &i, &schg);
					}
					else {					/* schg < imax					*/
						if (i < phase2->topim) {
							string[i] = istring[i];
							hlatst[i] = ihlatst[i] & KJMASK;
							i += 1;
						}
						else if (i < phase2->topim + phase2->lastim) {
							string[i] = msimage[i - phase2->topim];
							hlatst[i] = mhimage[i - phase2->topim];
							i += 1;
						}
						else{				/* phase2->botmstr <= i			*/
							string[i] = istring[i - phase2->lastim];
							hlatst[i] = ihlatst[i - phase2->lastim] & KJMASK;
							i += 1;
						}
					}
				}
				if (phase2->curis == -1)
					phase2->curst = phase2->topim + phase2->curim;
				else if (phase2->curis < phase2->topim)
					phase2->curst = phase2->curis;
				else
					phase2->curst = phase2->curis + phase2->lastim;
				if((hlatst[phase2->curst] & KJMASK) == KJ1st)
					kjcblk->curlen = CURATD;
				else
					kjcblk->curlen = CURATS;
				phase2->lastst = lastch;
			}
		}
	/**** replace mode but not NCS ****/
		else {
			if (phase2->state == HK1 || phase2->state == HK2)
				kjcblk->cnvsts = FINISHED;
			else
				kjcblk->cnvsts = GOINGON;
			lastch = MAX(phase2->lastis, phase2->topim + phase2->lastim);
			if (lastch > buffer->actcol) {	/* OVF condition is detected	*/
				phase2->state = OVF;
				phase2->beep = ON;
				kjcblk->cnvsts = OVERFLOW;
				schg = echg = 0;
			}
			else {							/* didn't overflow				*/
				if (phase2->lastst > lastch) {
					echg = phase2->lastst;
					imax = lastch;
					for (i = imax; i < phase2->lastst; i++)
						string[i] = hlatst[i] = 0;
				}
				else if (phase2->lastst < lastch) {
					echg = lastch;
					imax = lastch;
				}
				else {
					echg = 0;
					for (i = lastch - 1; i >= 0 && echg == 0; ) {
						if (i > phase2->topim + phase2->lastim) {
							m2cmp(istring, ihlatst, i, string, hlatst,
								i,&i,&echg);
						}
						else if (i == phase2->topim + phase2->lastim) {
							if ((ihlatst[i] & KJMASK) == KJ2nd) {
								if (string[i] == SPACE &&
								(hlatst[i] & HLMASK) == (ihlatst[i] & HLMASK))
									i -= 1;
								else
									echg = i + 1;
							}
							else {
								if (string[i] == istring[i] &&
									hlatst[i] == ihlatst[i])
									i -= 1;
								else
									echg = i + 1;
							}
						}
						else if (i >= phase2->topim) {
							m2cmp(msimage, mhimage, i - phase2->topim,
								string, hlatst, i, &i, &echg);
						}
						else {				/* i < phase2->topim			*/
							m2cmp(istring, ihlatst, i, string, hlatst,
								i,&i,&echg);
						}
					}
					imax = echg;
				}
				for (i = 0, schg = imax; i < imax; ) {
					if (schg == imax) {
						if (i < phase2->topim) {
							m2cmpmv(istring, ihlatst, i, string, hlatst,
								i, &i, &schg);
						}
						else if (i < phase2->topim + phase2->lastim) {
							m2cmpmv(msimage, mhimage, i - phase2->topim,
								string, hlatst, i, &i, &schg);
						}
						else if (i == phase2->topim + phase2->lastim) {
							if ((ihlatst[i] & KJMASK) == KJ2nd) {
								if (string[i] == SPACE &&
								(hlatst[i] & HLMASK) == (ihlatst[i] & HLMASK)){
									i += 1;
								}
								else {
									schg = i;
									string[i] = SPACE;
									hlatst[i] = JISCII & NOHIGHLIGHT;
									i += 1;
								}
							}
							else {
								m2cmpmv(istring, ihlatst, i,
										string, hlatst, i, &i, &schg);
							}
						}
						else {				/* i > phase2->botmstr			*/
							m2cmpmv(istring, ihlatst, i, string, hlatst,
								i, &i, &schg);
						}
					}
					else {					/* schg < imax					*/
						if (i < phase2->topim ||
							phase2->topim + phase2->lastim < i) {
							string[i] = istring[i];
							hlatst[i] = ihlatst[i] & KJMASK;
							i += 1;
						}
						else if (i < phase2->topim + phase2->lastim) {
							string[i] = msimage[i - phase2->topim];
							hlatst[i] = mhimage[i - phase2->topim];
							i += 1;
						}
						else {				/* i == phase2->botmstr			*/
							if ((ihlatst[i] & KJMASK) == KJ2nd) {
								string[i] = SPACE;
								hlatst[i] = JISCII;
								i += 1;
							}
							else {			/* not KJ2nd					*/
								string[i] = istring[i];
								hlatst[i] = ihlatst[i] & KJMASK;
								i += 1;
							}
						}
					}
				}							/* end of for loop				*/
				if (phase2->curis == -1)
					phase2->curst = phase2->topim + phase2->curim;
				else						/* curis != topim				*/
					phase2->curst = phase2->curis;
				if ((hlatst[phase2->curst] & KJMASK) == KJ1st)
					kjcblk->curlen = CURATD;
				else
					kjcblk->curlen = CURATS;
				phase2->lastst = lastch;
			}
		}
		kjcblk->chpos = schg;
		kjcblk->chlen = echg - schg;
	}
	else									/* current state == OVF			*/
		kjcblk->cnvsts = OVERFLOW;

	if(kjsvpt->beep)
		kjcblk->beep   = phase2->beep;
	else
		kjcblk->beep   = OFF;
	kjcblk->lastch = phase2->lastst;
	kjcblk->curcol = phase2->curst;
	if (kjcblk->cnvsts == OVERFLOW)
		kjcblk->discrd = DISCARD;
	else
		kjcblk->discrd = phase2->discrd;
	kjcblk->repins = phase2->repins;
	kjcblk->axuse1 = phase2->axuse1;
	if (phase2->moncall == YES) {			/* the Monitor has been called	*/
		kjcblk->trace  = mkjcblk->trace;
		kjcblk->ax1col = mkjcblk->ax1col;
		kjcblk->ax1row = mkjcblk->ax1row;
		kjcblk->cura1c = mkjcblk->cura1c;
		if ((kjcblk->cura1r = mkjcblk->cura1r) > 0)
			kjcblk->cura1r = 0;
		kjcblk->chpsa1 = mkjcblk->chpsa1;
		kjcblk->chlna1 = mkjcblk->chlna1;
	}
}											/* end of phase2				*/
	return EXMONNOERR;
}

#ifdef DEBUG
printstate(msg, state)
char	*msg;
int		state;
{
	char	*name;

	switch (state) {
	case NCS : name = "NCS"; break;
	case CYI : name = "CYI"; break;
	case CNV : name = "CNV"; break;
	case EDE : name = "EDE"; break;
	case EDM : name = "EDM"; break;
	case EDC : name = "EDC"; break;
	case HK1 : name = "HK1"; break;
	case HK2 : name = "HK2"; break;
	case ACI : name = "ACI"; break;
	case KNI : name = "KNI"; break;
	case CMS : name = "CMS"; break;
	case DIA : name = "DIA"; break;
	case OVF : name = "OVF"; break;
	default  : name = "UNKNOWN"; break;
	}
	printf("%s%s", msg, name);
}
#endif

/*
 *	return _KJCBLK pointer
 */
struct  _KJCBLK	*jexmGetKJCBLK(KJCBLK *kp)
{
	return kp->kjsvpt->mkjcblk;
}
