static char sccsid[] = "@(#)37	1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMRegister.c, libKJI, bos411, 9428A410j 5/31/93 20:53:30";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMRegister
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <X11/keysym.h>
#include "_Kcmap.h"
#include "_Kcmcb.h"
#include "imjimP.h"		/* JIM private header			*/
#include "JIMRegister.h"	/* JIMRegister header			*/
#include "JIMRegisterMsg.h"	/* JIMRegister message header		*/

/* the first byte of SJIS double byte character */
#define FIRST_SJIS(c)					\
	((0x80 <= (c) && (c) <= 0x9f) || (0xe0 <= (c) && (c) <= 0xfc))


/*----------------------------------------------------------------------*
 *	set a registration window message to aux area
 *----------------------------------------------------------------------*/
static	void	RegAuxSetMessage
(
IMAuxInfo	*aux,		/* a pointer to an aux structure	*/
JIMRegister	*reg,		/* JIMRegister pointer			*/
int		line		/* the line to set the message		*/
)
{
	char	*p;

	if(reg->msgno == reg->next_msgno){
		return;
	}

	switch(reg->next_msgno){
	case JIM_NO_MESSAGE	: p = JIM_REG_FOOTER;		break;
	case JIM_ACCESS_ERR	: p = JIM_ACCESS_ERR_MSG;	break;
	case JIM_EQ_YOMI_GOKU	: p = JIM_EQ_YOMI_GOKU_MSG;	break;
	case JIM_INVAL_YOMI	: p = JIM_INVAL_YOMI_MSG;	break;
	case JIM_INVAL_GOKU	: p = JIM_INVAL_GOKU_MSG;	break;
	case JIM_INVAL_MIX	: p = JIM_INVAL_MIX_MSG;	break;
	case JIM_TOO_LONG	: p = JIM_TOO_LONG_MSG;		break;
	case JIM_USR_EXIST	: p = JIM_USR_EXIST_MSG;	break;
	case JIM_SYS_EXIST	: p = JIM_SYS_EXIST_MSG;	break;
	case JIM_DICT_FULL	: p = JIM_DICT_FULL_MSG;	break;
	case JIM_YOMI_FULL	: p = JIM_YOMI_FULL_MSG;	break;
	case JIM_INPUT_YOMI	: p = JIM_INPUT_YOMI_MSG;	break;
	case JIM_INPUT_GOKU	: p = JIM_INPUT_GOKU_MSG;	break;
	case JIM_INVAL_CHAR	: p = JIM_INVAL_CHAR_MSG;	break;
	case JIM_SELECT_ONE	: p = JIM_SELECT_ONE_MSG;	break;
	case JIM_TOUROKU_OK	: p = JIM_TOUROKU_OK_MSG;	break;
	}

	if(reg->cd == NULL){ /* SJIS */
		memcpy(aux->message.text[line].str, p, strlen(p));
		aux->message.text[line].len = strlen(p);
		memset(aux->message.text[line].att, 0x00, strlen(p));
	}else{ /* EUC */
		aux->message.text[line].len =
			SjisToEuc(reg->cd, p, aux->message.text[line].str,
						strlen(p), NULL, NULL, NULL);
		memset(aux->message.text[line].att, 0x00, strlen(p));
	}
	reg->msgno = reg->next_msgno;
}


/*----------------------------------------------------------------------*
 *	change input line in registration area
 *----------------------------------------------------------------------*/
static	void	JIMRegisterChangeLine
(
JIMRegister	*reg		/* registration area			*/
)
{
	if(reg->row == 1){
		reg->row = 2;
		reg->cur = reg->yomilen;
	}else{
		reg->row = 1;
		reg->cur = reg->gokulen;
	}
}


/*----------------------------------------------------------------------*
 *	write a yomi/goku pair to user dictionary
 *----------------------------------------------------------------------*/
static	int	JIMRegisterWrite	/* success -> 0			*/
				/* error -> 1				*/
(
JIMRegister	*reg,		/* JIMRegister pointer			*/
JIMed		*jedinfo	/* to access KKC			*/
)
{
	int	ret, yomilen;
	char	yomi[64];

	if(reg->gokulen == 0){
		if(reg->row != 1){
			JIMRegisterChangeLine(reg);
		}
		reg->next_msgno = JIM_INPUT_GOKU;
		return 1;
	}
	if(reg->yomilen == 0){
		if(reg->row != 2){
			JIMRegisterChangeLine(reg);
		}
		reg->next_msgno = JIM_INPUT_YOMI;
		return 1;
	}
		
	if(reg->gokulen == reg->yomilen){
		if(memcmp(reg->goku, reg->yomi, reg->gokulen) == 0){
			reg->next_msgno = JIM_EQ_YOMI_GOKU;
			return 1;
		}
	}

	if((ret = SjisTo7bit(reg->yomi, reg->yomilen, yomi, &yomilen)) != 0){
		if(ret == 1){
			reg->next_msgno = JIM_INVAL_MIX;
		}else{
			reg->next_msgno = JIM_INVAL_YOMI;
		}
		return 1;
	}
/* 7 bit code for alpha-num shift */
#define  SHIFT_ALNUM   ( 0x1d )
	if(*yomi == SHIFT_ALNUM && yomilen > JIM_REG_YOMI_MAX / 2){
		reg->next_msgno = JIM_TOO_LONG;
		return 1;
	}

	/* write to user dictionary */
	yomi[yomilen] = '\0';
	if((ret = JIMWrite(jedinfo, yomi, yomilen,
					reg->goku, reg->gokulen)) == 0){
		reg->next_msgno = JIM_TOUROKU_OK;
		return 0;
	}else{
		switch(ret){
		case 0x0110	: reg->next_msgno = JIM_SYS_EXIST; break;
		case 0x0210	: reg->next_msgno = JIM_USR_EXIST; break;
		case 0x0310	: reg->next_msgno = JIM_DICT_FULL; break;
		case 0x0410	: reg->next_msgno = JIM_YOMI_FULL; break;
		default		: reg->next_msgno = JIM_ACCESS_ERR;
		}
		return 1;
	}
}


/*----------------------------------------------------------------------*
 *	initialize runtime registration information structure
 *----------------------------------------------------------------------*/
static	void	JIMRegisterInit
(
JIMRegister	*reg,		/* JIMRegister pointer			*/
JIMobj		*obj		/* JIM object pointer (for cd)		*/
)
{
	int	codeset = ((JIMfep *)(obj->imobject.imfep))->codeset;

	reg->gokulen = 0;
	reg->yomilen = 0;
	reg->goku[0] = '\0';
	reg->yomi[0] = '\0';
	reg->row = 1;
	reg->cur = 0;
	if (codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
		reg->cd = NULL;
	}else{
		reg->cd = ((JIMfep *)(obj->imobject.imfep))->cd;
	}
	reg->other_item = 0;
	reg->msgno = JIM_NO_MESSAGE;
	reg->next_msgno = JIM_NO_MESSAGE;
}

/*----------------------------------------------------------------------*
 *	create and initialize runtime registration information structure
 *----------------------------------------------------------------------*/
JIMRegister	*JIMRegisterCreate
			/* success -> pointer to a new JIMRegister	*/
			/* error -> NULL				*/
(
JIMobj		*obj		/* JIM object pointer */
				/* only the codeset and cd is used */
)
{
	JIMRegister	*reg = (JIMRegister *)malloc(sizeof(JIMRegister));

	if(reg == NULL){
		return NULL;
	}

	JIMRegisterInit(reg, obj);

	return reg;
}


/*----------------------------------------------------------------------*
 *	destroy runtime registration information structure
 *----------------------------------------------------------------------*/
static	void	JIMRegisterDestroy
(
JIMRegister	**reg		/* a pointer to a JIMRegister		*/
)
{
	if(*reg != NULL){
		free(*reg);
		*reg = NULL;
	}
}

/*----------------------------------------------------------------------*
 *	initialize aux area for runtime registration
 *----------------------------------------------------------------------*/
static	void	RegAuxInit
(
IMAuxInfo	*aux,		/* a pointer to a aux structure		*/
JIMRegister	*reg,		/* JIMRegister pointer (for cd)		*/
unsigned char	*udictname	/* user dictionary name			*/
)
{
	int	i, j;

	aux->title.len = 0;		/* no title */
	aux->title.str = NULL;
	aux->selection.panel_row = 0;	/* no panel at initialization */
	aux->selection.panel_col = 0;
	aux->selection.panel = NULL;
	aux->button = IM_NONE;		/* no button */
	aux->hint = IM_AtTheEvent;	/* displayed at the event */
	aux->message.maxwidth = JIM_REG_WIN_WMAX;
	aux->message.nline = JIM_REG_WIN_HEIGHT;
	aux->message.cursor = TRUE;
	aux->message.cur_row = 1;	/* Cursor is at the top of goku */
	aux->message.cur_col = JIM_REG_IN_START;	/* input cursor */

	/* set initial string */
	if(reg->cd == NULL){ /* SJIS */
		memcpy(aux->message.text[0].str,
				JIM_REG_HEADER, strlen(JIM_REG_HEADER));
		memcpy(aux->message.text[1].str,
				JIM_REG_GOKU, strlen(JIM_REG_GOKU));
		memcpy(aux->message.text[2].str,
				JIM_REG_YOMI, strlen(JIM_REG_YOMI));
		memcpy(aux->message.text[3].str,
				JIM_REG_FOOTER, strlen(JIM_REG_FOOTER));
		aux->message.text[0].len = strlen(JIM_REG_HEADER);
		aux->message.text[1].len = strlen(JIM_REG_GOKU);
		aux->message.text[2].len = strlen(JIM_REG_YOMI);
		aux->message.text[3].len = strlen(JIM_REG_FOOTER);
	}else{ /* EUC */
		aux->message.text[0].len =
			SjisToEuc(reg->cd, JIM_REG_HEADER,
				aux->message.text[0].str,
				strlen(JIM_REG_HEADER), NULL, NULL, NULL);
		aux->message.text[1].len =
			SjisToEuc(reg->cd, JIM_REG_GOKU,
				aux->message.text[1].str,
				strlen(JIM_REG_GOKU), NULL, NULL, NULL);
		aux->message.text[2].len =
			SjisToEuc(reg->cd, JIM_REG_YOMI,
				aux->message.text[2].str,
				strlen(JIM_REG_YOMI), NULL, NULL, NULL);
		aux->message.text[3].len =
			SjisToEuc(reg->cd, JIM_REG_FOOTER,
				aux->message.text[3].str,
				strlen(JIM_REG_FOOTER), NULL, NULL, NULL);
	}

	/* set user dictionary name */
	i = strlen(udictname);
	if(i > JIM_REG_WIN_WMAX - JIM_REG_UDNAME_START){
		/* cut the tail of udict name and put '-' at the head */
		for(j = 0; j < i;){
			if(FIRST_SJIS(udictname[j])){
				j += 2;
				if(j > JIM_REG_WIN_WMAX -
						JIM_REG_UDNAME_START - 1){
					j -= 2;
					break;
				}
			}else{
				j++;
				if(j > JIM_REG_WIN_WMAX -
						JIM_REG_UDNAME_START - 1){
					j--;
					break;
				}
			}
		}
		aux->message.text[0].str[JIM_REG_UDNAME_START] = '-';
		memcpy(&(aux->message.text[0].str[JIM_REG_UDNAME_START + 1]),
							udictname, j);
	}else{
		memcpy(&(aux->message.text[0].str[JIM_REG_UDNAME_START]),
							udictname, i);
	}

	/* initialize attribute strings */
	for(i = 0; i < JIM_REG_WIN_HEIGHT; i++){
		memset(aux->message.text[i].att, 0x00, JIM_REG_WIN_WMAX);
	}
}


/*----------------------------------------------------------------------*
 *	start runtime registration and display the aux window
 *----------------------------------------------------------------------*/
int	StartRegistration		/* success -> IMNoError		*/
					/* error -> IMError		*/
(
JIMobj	*obj
)
{
	IMCallback	*cb;

	/* initialize aux area */
	RegAuxInit(&(obj->auxinfo), obj->registration,
		((JIMfep *)(obj->imobject.imfep))->udictinfo.udictname);

	/* create aux window */
	obj->textauxstate = IMAuxiliaryOn;
	cb = obj->imobject.cb;
	if(cb->auxcreate && (*cb->auxcreate)(obj, &obj->auxid,
					obj->imobject.udata) == IMError){
			obj->imobject.imfep->imerrno = IMCallbackError;
			return(IMError);
	}

	/* draw aux window */
	obj->auxidflag = TRUE;
	obj->auxstate = JIM_AUXNOW;
	obj->auxinfo.status = IMAuxShowing;

	if(cb->auxdraw && (*cb->auxdraw)(obj, obj->auxid, &obj->auxinfo,
			obj->imobject.udata) == IMError){
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	}

	return IMNoError;
}


/*----------------------------------------------------------------------*
 *	add committed string into registration area
 *----------------------------------------------------------------------*/
static	void	JIMRegisterPutString
(
JIMRegister	*reg,		/* registration area			*/
unsigned char	*str,		/* string (SJIS) to be added		*/
int		len		/* string length			*/
)
{
	int	i, slen;
	char	buf[256];

	/* goku string */
	if(reg->row == 1){
		if(CheckGoku(str, len, buf, &slen)){
			reg->next_msgno = JIM_INVAL_CHAR;
		}else{
			reg->next_msgno = JIM_NO_MESSAGE;
		}
		if(reg->gokulen + slen > JIM_REG_GOKU_MAX){
			slen = JIM_REG_GOKU_MAX - reg->gokulen;
		}
		if(reg->cur < reg->gokulen){
			for(i = reg->gokulen - 1; i >= reg->cur; i--){
				reg->goku[i + slen] = reg->goku[i];
			}
		}
		memcpy(&(reg->goku[reg->cur]), buf, slen);
		reg->gokulen += slen;

	/* yomi string */
	}else{
		if(CheckYomi(str, len, buf, &slen)){
			reg->next_msgno = JIM_INVAL_CHAR;
		}else{
			reg->next_msgno = JIM_NO_MESSAGE;
		}
		if(reg->yomilen + slen > JIM_REG_YOMI_MAX){
			slen = JIM_REG_YOMI_MAX - reg->yomilen;
		}
		if(reg->cur < reg->yomilen){
			for(i = reg->yomilen - 1; i >= reg->cur; i--){
				reg->yomi[i + slen] = reg->yomi[i];
			}
		}
		memcpy(&(reg->yomi[reg->cur]), buf, slen);
		reg->yomilen += slen;
	}
	reg->cur += slen;
}


/*----------------------------------------------------------------------*
 *	move cursor to left
 *----------------------------------------------------------------------*/
static	void	JIMRegisterLeft
(
JIMRegister	*reg		/* registration area			*/
)
{
	unsigned char	*p;
	int		i;

	if(reg->row == 1){
		p = reg->goku;
	}else{
		p = reg->yomi;
	}

	if(reg->cur >= 2){
		reg->cur -= 2;
		for(i = 0; i < reg->cur;){
			if(FIRST_SJIS(p[i])){
				i += 2;
			}else{
				i++;
			}
		}
		reg->cur = i;
	}else if(reg->cur == 1){
		reg->cur = 0;
	}

	return;
}


/*----------------------------------------------------------------------*
 *	move cursor to right
 *----------------------------------------------------------------------*/
static	void	JIMRegisterRight
(
JIMRegister	*reg		/* registration area			*/
)
{
	unsigned char	*p;

	if(reg->row == 1){
		if(reg->cur >= reg->gokulen){
			return;
		}
		p = reg->goku;
	}else{
		if(reg->cur >= reg->yomilen){
			return;
		}
		p = reg->yomi;
	}

	if(FIRST_SJIS(p[reg->cur])){
		reg->cur +=2;
	}else{
		reg->cur++;
	}
}


/*----------------------------------------------------------------------*
 *	delete one character (without changing cursor position)
 *----------------------------------------------------------------------*/
static	void	JIMRegisterDelete
(
JIMRegister	*reg		/* registration area			*/
)
{
	int	clen, i;

	if(reg->row == 1){
		if(FIRST_SJIS(reg->goku[reg->cur])){
			clen = 2;
		}else{
			clen = 1;
		}
		if(reg->gokulen >= clen && reg->cur < reg->gokulen){
			for(i = reg->cur + clen; i < reg->gokulen; i++){
				reg->goku[i - clen] = reg->goku[i];
			}
			reg->gokulen -= clen;
		}
	}else{
		if(FIRST_SJIS(reg->yomi[reg->cur])){
			clen = 2;
		}else{
			clen = 1;
		}
		if(reg->yomilen >= clen && reg->cur < reg->yomilen){
			for(i = reg->cur + clen; i < reg->yomilen; i++){
				reg->yomi[i - clen] = reg->yomi[i];
			}
			reg->yomilen -= clen;
		}
	}
}


/*----------------------------------------------------------------------*
 *	make aux window information with listbox
 *----------------------------------------------------------------------*/
void	RegAuxListbox
(
IMAuxInfo	*aux,		/* a pointer to a aux structure		*/
JIMRegister	*reg,		/* JIMRegister pointer			*/
JIMed		*jedinfo	/* get pre-edit sting and attribute	*/
)
{
	aux->message.cur_col = JIM_REG_IN_START + reg->cur + 
					jedGetCurPos(jedinfo->jeid);
	aux->message.cur_row = reg->row;
	aux->message.nline = JIM_REG_WIN_HEIGHT;

	/* remove title */
	if(aux->title.str != NULL){
		free(aux->title.str);
		aux->title.str = NULL;
	}
	aux->title.len = 0;

	/* set message */
	reg->msgno = -1;
	reg->next_msgno = JIM_SELECT_ONE;
	RegAuxSetMessage(aux, reg, JIM_REG_WIN_HEIGHT - 1);
}


/*----------------------------------------------------------------------*
 *	make string on registration aux window
 *----------------------------------------------------------------------*/
static	void	RegAuxMakeString
(
IMAuxInfo	*aux,		/* a pointer to a aux structure		*/
JIMRegister	*reg,		/* JIMRegister pointer			*/
JIMed		*jedinfo	/* get pre-edit sting and attribute	*/
)
{
	int	pre_len;	/* pre-edit string length		*/
	int	pre_cur;	/* cursor position in pre-edit string	*/
	int	com_len;	/* committed string length		*/
	int	com_cur;	/* committed string cursor position	*/
	int	row = reg->row;	/* input row				*/
	int	max;		/* max input characters			*/
	unsigned char	*str;	/* committed string			*/
	unsigned char	*reg_str;/* the head of the input area string	*/
	unsigned char	*reg_att;/* the head of the input area attribute*/
	int	i;

	if(row == 1){
		str = reg->goku;
		com_len = reg->gokulen;
		max = JIM_REG_GOKU_MAX;
	}else{
		str = reg->yomi;
		com_len = reg->yomilen;
		max = JIM_REG_YOMI_MAX;
	}
	com_cur = reg->cur;
	reg_str = &(aux->message.text[row].str[JIM_REG_IN_START]);
	reg_att = &(aux->message.text[row].att[JIM_REG_IN_START]);

	/* copy commited string from JIMRegister to IMAuxInfo */
	if(reg->cd == NULL){ /* SJIS */
		memcpy(reg_str, str, com_len);
	}else{ /* EUC */
		com_len = SjisToEuc(reg->cd, str, reg_str,
					com_len, &com_cur, NULL, NULL);
	}
	memset(reg_att, 0x00, com_len);

	/* pre-edit string */ 
	pre_cur =jedGetCurPos(jedinfo->jeid);
	pre_len = jedGetEchoBufLen(jedinfo->jeid);

	/* pre-edit sting is too long. cut it */
	if(com_cur + pre_len > max + 2){
		pre_len = max + 2 - com_cur;
		if(com_cur + pre_cur > max + 1){
			pre_cur = max + 1 - com_cur;
		}
	}

	/* copy pre-edit string from JIMed to IMAuxInfo */ 
	if(reg->cd == NULL){ /* SJIS */
		memcpy(&(reg_str[com_cur]), jedinfo->echobufs, pre_len);
		for(i = 0; i < pre_len; i++){
			if(jedinfo->echobufa[i] & KP_HL_REVERSE){
				reg_att[com_cur + i] =
					IMAttHighlight | IMAttAttention;
			}else{
				reg_att[com_cur + i] = IMAttHighlight;
			}
		}
	}else{ /* EUC */
		pre_len = SjisToEucAtt(reg->cd,
			jedinfo->echobufs, jedinfo->echobufa,
			&(reg_str[com_cur]), &(reg_att[com_cur]),
			pre_len, &pre_cur, NULL, NULL);
		for(i = 0; i < pre_len; i++){
			if(reg_att[com_cur + i] & KP_HL_REVERSE){
				reg_att[com_cur + i] =
					IMAttHighlight | IMAttAttention;
			}else{
				reg_att[com_cur + i] = IMAttHighlight;
			}
		}
	}
	pre_len += com_cur;
	if(pre_len < com_len){
		pre_len = com_len;
	}
	for(i = pre_len; i < max; i++){
		reg_str[i] = 0x20;
		reg_att[i] = 0x00;
	}
	if(i == max){
		reg_str[i] = 0x5d; reg_str[i + 1] = 0x20;	/* ] */
		reg_att[i] = 0x00; reg_att[i + 1] = 0x00;
	}
	for(i += 2; i < JIM_REG_WIN_WMAX; i++){
		reg_str[i] = 0x20;
		reg_att[i] = 0x00;
	}
	aux->message.cur_col = JIM_REG_IN_START + com_cur + pre_cur;
	aux->message.cur_row = row;

	RegAuxSetMessage(aux, reg, JIM_REG_WIN_HEIGHT - 1);
}


/*----------------------------------------------------------------------*
 *	make aux information with cnadidate list or number input area
 *----------------------------------------------------------------------*/
void	RegAuxMake
(
IMAuxInfo	*aux,		/* a pointer to a aux structure		*/
JIMRegister	*reg,		/* JIMRegister pointer			*/
JIMed		*jedinfo	/* get aux information			*/
)
{
	AuxSize		auxsize, jedGetAuxSize();
	AuxCurPos	auxcurpos, jedGetAuxCurPos();
	int		auxtype;
	int		i, j;
	caddr_t		*from, *froma;
	unsigned int	curcol;

	/**********************************************/
	/* ask editor aux size and cursor information */
	/**********************************************/
	auxsize = jedGetAuxSize(jedinfo->jeid);
	auxcurpos = jedGetAuxCurPos(jedinfo->jeid);
	curcol = auxcurpos.colpos;

	/* aux has been removed */
	if(auxsize.itemnum == 0){
		aux->title.len = 0;		/* no title */
		aux->title.str = NULL;
		aux->selection.panel_row = 0;	/* no panel */
		aux->selection.panel_col = 0;
		aux->selection.panel = NULL;
		aux->button = IM_NONE;		/* no button */
		aux->hint = IM_AtTheEvent;	/* displayed at the event */
		aux->message.maxwidth = JIM_REG_WIN_WMAX;
		aux->message.nline = JIM_REG_WIN_HEIGHT;
		aux->message.cursor = TRUE;
		aux->message.cur_row = reg->row;
		aux->message.cur_col = JIM_REG_IN_START + reg->cur + 
						jedGetCurPos(jedinfo->jeid);

		reg->other_item = 0;
		reg->msgno = -1;
		reg->next_msgno = JIM_NO_MESSAGE;
		RegAuxSetMessage(aux, reg, auxsize.itemnum + 3);

		return;
	}

	/* copy string and attribute */
	if(reg->cd == NULL){ /* SJIS */
		from = jedinfo->auxbufs;
		for(i = 0; i < auxsize.itemnum; i++){
			memcpy(aux->message.text[i + 3].str,
					*from++, auxsize.itemsize);
		}

		/* assuming there is only reverse attribute within aux */
		from = jedinfo->auxbufa;
		for(i = 0; i < auxsize.itemnum; i++){
			aux->message.text[i + 3].len = auxsize.itemsize;
			for(j = 0; j < auxsize.itemsize; j++){
				if((*from)[j] & KP_HL_REVERSE){
					aux->message.text[i + 3].att[j] =
							IMAttAttention;
				}else{
					aux->message.text[i + 3].att[j] =
							IMAttNone;
				}
			}
			from++;
		}

	}else{ /* EUC */
		from = jedinfo->auxbufs;
		froma = jedinfo->auxbufa;
		for(i = 0; i < auxsize.itemnum; i++, from++, froma++){
			if(i == auxcurpos.rowpos){
				aux->message.text[i + 3].len =
					SjisToEucAtt(reg->cd, *from, *froma,
						aux->message.text[i + 3].str,
						aux->message.text[i + 3].att,
						auxsize.itemsize,
						&curcol, NULL, NULL);
			}else{
				aux->message.text[i + 3].len =
					SjisToEucAtt(reg->cd, *from, *froma,
						aux->message.text[i + 3].str,
						aux->message.text[i + 3].att,
						auxsize.itemsize,
						NULL, NULL, NULL);
			}
		}

		/* assuming there is only reverse attribute within aux */
		for(i = 0; i < auxsize.itemnum; i++){
			for(j = 0; j < aux->message.text[i].len; j++){
				if (aux->message.text[i + 3].att[j] &
								KP_HL_REVERSE){
					aux->message.text[i + 3].att[j] =
								IMAttAttention;
				}else{
					aux->message.text[i].att[j] =
								IMAttNone;
				}
			}
		}
	}

	reg->msgno = -1;
	if(auxsize.itemnum > 1){
		reg->next_msgno = JIM_SELECT_ONE;
	}else{
		reg->next_msgno = JIM_NO_MESSAGE;
	}
	RegAuxSetMessage(aux, reg, auxsize.itemnum + 3);

	if(auxcurpos.rowpos >= 0 && auxcurpos.colpos >= 0){
		aux->message.cur_row = auxcurpos.rowpos + 3;
		aux->message.cur_col = curcol;
	}
	aux->message.nline = auxsize.itemnum + JIM_REG_WIN_HEIGHT;

	reg->other_item = auxsize.itemnum;
}


/*----------------------------------------------------------------------*
 *	registration process
 *----------------------------------------------------------------------*/
int	Registration	/* return value is same as jimfilter		*/
			/* success -> IMInputUsed/IMInputNotUsed	*/
			/* error -> IMError				*/
(
JIMobj		*obj,	/* JIM object					*/
unsigned int	key,	/* input key (keysym)				*/
unsigned int	shift,	/* input shift state				*/
IMBuffer	*imb	/* internal buffer				*/
)
{
	extern EchoBufChanged	jedIsEchoBufChanged();
	extern AuxSize	jedGetAuxSize();
	extern InputMode	jedGetInputMode();
	extern int	jedIsAuxBufChanged();
	extern int	jedControl();
	extern int	jedIsBeepRequested();
	extern int	jedIsCurPosChanged();

	AuxSize	auxsize;
	int	ret;
	int	textstate;
	int	direction;
	int	offset;
	IMCallback	*cb;
	IMFep	imfep;
	int	jeid;

        /* while selection panel is displayed, no input is handled */
        if(obj->auxinfo.selection.panel != NULL){
                if(key != XK_Escape) {
                        return(IMInputNotUsed);
                }
        }

	imfep = obj->imobject.imfep;
	cb = obj->imobject.cb;
	jeid = obj->jedinfo.jeid;
	/*********************/
	/* initialize states */
	/*********************/
	obj->textauxstate = IMTextAndAuxiliaryOff;
	if (jedGetEchoBufLen(jeid) == 0)
		textstate = JIM_NOTEXT;
	else
		textstate = JIM_TEXTON;
	inittextinfo(obj);

	/******************************/
	/* call IMED process function */
	/******************************/
	/* terminate registration */
	if(key == XK_Escape){
		jimclear(obj, 1);		/* clear aux window */
		JIMRegisterDestroy(&(obj->registration));
		return IMInputUsed;

	/* normal registration process */
	}else{
		ret = jedFilter(jeid, ((JIMFEP)imfep)->immap, key, shift, imb);
	}

	/**********************************************/
	/* put committed string into regstration area */
	/**********************************************/
	if(obj->output.len != 0){
		JIMRegisterPutString(obj->registration,
			obj->output.data, obj->output.len);
		obj->output.len = 0;
	}

	/*****************************/
	/* real-edit or registration */
	/*****************************/
	if(ret == KP_NOTUSED){

		switch(key){
		case XK_Up :
		case XK_Down :
		case XK_Tab :
			JIMRegisterChangeLine(obj->registration); break;
		case XK_BackSpace :
			JIMRegisterLeft(obj->registration);
			JIMRegisterDelete(obj->registration); break;
		case XK_Delete :
			JIMRegisterDelete(obj->registration); break;
		case XK_Left :
			JIMRegisterLeft(obj->registration); break;
		case XK_Right :
			JIMRegisterRight(obj->registration); break;
		case XK_Return :
			if(JIMRegisterWrite(obj->registration,
							&(obj->jedinfo)) == 0){
				JIMRegisterInit(obj->registration, obj);
				obj->registration->next_msgno = JIM_TOUROKU_OK;
				RegAuxInit(&(obj->auxinfo), obj->registration,
					((JIMfep *)(obj->imobject.imfep))
						->udictinfo.udictname);
			}
			break;
		default :
			if(cb->beep && (*cb->beep)(obj, JIM_BEEPPER,
					obj->imobject.udata) == IMError){
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
			return IMInputUsed;
		}

		/* re-draw aux window */
		RegAuxMakeString(&(obj->auxinfo),
			obj->registration, &(obj->jedinfo));
		if(cb->auxdraw && (*cb->auxdraw)(obj, obj->auxid, &obj->auxinfo,
					 obj->imobject.udata) == IMError){
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}
		return IMInputUsed;
	}

	/****************************/
	/* process text information */
	/****************************/
	if(!(jedIsAuxBufChanged(jeid) || jedIsAuxCurPosChanged(jeid))){
		if(jedIsEchoBufChanged(jeid).flag || jedIsCurPosChanged(jeid)){
			RegAuxMakeString(&(obj->auxinfo),
					obj->registration, &(obj->jedinfo));
			if(cb->auxdraw && (*cb->auxdraw)(obj, obj->auxid,
				&obj->auxinfo, obj->imobject.udata) == IMError){
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
		}

	/***************************/
	/* process aux information */
	/***************************/
	}else{

		/* aux window size must be changed */
		auxsize = jedGetAuxSize(jeid);
		if(auxsize.itemnum != obj->registration->other_item){

			/* remove old aux window */
			if(cb->auxhide && (*cb->auxhide)(obj,
				obj->auxid, obj->imobject.udata) == IMError){
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
			if(cb->auxdestroy && (*cb->auxdestroy)(obj,
				obj->auxid, obj->imobject.udata) == IMError){
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}

			/* create new aux window */
			if(cb->auxcreate && (*cb->auxcreate)(obj,
				&obj->auxid, obj->imobject.udata) == IMError){
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
			obj->auxidflag = TRUE;
		}

		/* make new aux information */
		if(jedHasSelection(obj->jedinfo.jeid)){
			makelistbox(obj);
			RegAuxListbox(&(obj->auxinfo),
				obj->registration, &(obj->jedinfo));
		}else{
			RegAuxMake(&(obj->auxinfo), obj->registration,
							&(obj->jedinfo));
		}

		/* make new text */
		if(jedIsEchoBufChanged(jeid).flag || jedIsCurPosChanged(jeid)){
			RegAuxMakeString(&(obj->auxinfo),
					obj->registration, &(obj->jedinfo));
		}

		/* draw new aux window */
		if(cb->auxdraw && (*cb->auxdraw)(obj, obj->auxid,
				&obj->auxinfo, obj->imobject.udata) == IMError){
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}

	} /* end of aux processing */

	/*********************************/
	/* process indicator information */
	/*********************************/
	if (jedIsInputModeChanged(jeid)) {
		makeindinfo(obj);
		makequerystate(obj);
		if (jedGetInputMode(jeid).ind3 == KP_NORMALMODE &&
			cb->indicatordraw && (*cb->indicatordraw)(obj,
			&obj->indinfo, obj->imobject.udata) == IMError) {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}
	}

	/********/
	/* beep */
	/********/
	if (jedIsBeepRequested(jeid) && cb->beep &&
		(*cb->beep)(obj, JIM_BEEPPER, obj->imobject.udata) == IMError) {
			imfep->imerrno = IMCallbackError;
			return(IMError);
	}
	return ret == KP_USED ? IMInputUsed : IMInputNotUsed;
}


/*----------------------------------------------------------------------*
 *	get MCB (in KKC) pointer from jfep
 *----------------------------------------------------------------------*/
struct MCB	*jfepGetMCB(JIMed *jedinfo)
{
	return (struct MCB *)jmntGetKKCB(
					jexmGetKJCBLK(
						jedGetKCB(jedinfo->jeid)));
}


/*----------------------------------------------------------------------*
 *	get KCB (in KKC) pointer from jkkc
 *----------------------------------------------------------------------*/
struct KCB	*jkkcGetKCB(struct MCB *mp)
{
	return mp->kcbaddr;
}


/*----------------------------------------------------------------------*
 *	write a yomi/goku pair to the user dictionary
 *----------------------------------------------------------------------*/
int	JIMWrite		/* success -> 0				*/
				/* error -> 1				*/
(
JIMed		*jedinfo,	/* to access KKC			*/
unsigned char	*yomi,
int		yomilen,
unsigned char	*goku,
int		gokulen
)
{
	struct MCB	*mp;
	struct KCB	*kp;
	int		ret;

	/* get MCB & KCB (in KKC) pointer */
	mp = jfepGetMCB(jedinfo);
	kp = (struct KCB *)jkkcGetKCB(mp);

	/* re-load the current user dictionary */ 
	WatchUdict(mp);

	/* set yomi */
	memcpy(kp->ymiaddr, yomi, yomilen);
	kp->ymill1 = kp->ymill2 = yomilen;
	kp->mode = 0;

	/* set goku */
	memcpy(kp->seiaddr->kj, goku, gokulen);
	kp->seiaddr->ll[0] = 0x00;
        kp->seiaddr->ll[1] = kp->seill = gokulen + 2;

	/* register yomi/goku to user dictionary */
	ret = _Kc0dicr(kp);

	/* reset KCB work area */
	_Kcxinia(kp);
	_Kcrinit(kp);

	return ret;
}
