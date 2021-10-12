static char sccsid[] = "@(#)36	1.1  src/bos/usr/lib/nls/loc/jim/jfep/JIMCheckString.c, libKJI, bos411, 9428A410j 5/23/93 20:44:30";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMCheckString
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

/* second byte of DBCS character */
/* 0x81xx */
#define  U_2COLN       ( 0x46 )  /* DBCS :    */
#define  U_2SEMCOLN    ( 0x47 )  /* DBCS ;    */
#define  U_2QUES       ( 0x48 )  /* DBCS ?    */
#define  U_2BIKKU      ( 0x49 )  /* DBCS !    */
#define  U_2SURA       ( 0x5e )  /* DBCS /    */
#define  U_2MLKA       ( 0x69 )  /* DBCS (    */
#define  U_2MRKA       ( 0x6a )  /* DBCS )    */
#define  U_2LKUCHI     ( 0x83 )  /* DBCS <    */
#define  U_2RKUCHI     ( 0x84 )  /* DBCS >    */
#define  U_2PLUS       ( 0x7b )  /* DBCS +    */
#define  U_2MAI        ( 0x7c )  /* DBCS -    */
#define  U_2IQU        ( 0x81 )  /* DBCS =    */
#define  U_2SPOCHI     ( 0x8c )  /* DBCS '    */
#define  U_2DPOCHI     ( 0x8d )  /* DBCS "    */
#define  U_2PER        ( 0x93 )  /* DBCS %    */
#define  U_2SHAR       ( 0x94 )  /* DBCS #    */
#define  U_2AND        ( 0x95 )  /* DBCS &    */
#define  U_2HOSHI      ( 0x96 )  /* DBCS *    */
#define  U_2TANKA      ( 0x97 )  /* DBCS @    */

#define  U_2CHOUON     ( 0x5b )  /* DBCS -    */
#define  U_2DOLLAR     ( 0x90 )  /* DBCS $    */

/* 0x82xx */
#define  U_CHK_NL      ( 0x4f )  /*  DBCS Numeric  check Low  code  */
#define  U_CHK_NH      ( 0x58 )  /*  DBCS Numeric  check High code  */
#define  U_CHK_ALL     ( 0x60 )  /*  DBCS ALPHABET check Low  code  */
#define  U_CHK_ALH     ( 0x79 )  /*  DBCS ALPHABET check High code  */
#define  U_CHK_ASL     ( 0x81 )  /*  DBCS alphabet check Low  code  */
#define  U_CHK_ASH     ( 0x9a )  /*  DBCS alphabet check High code  */
#define  U_CHK_HL      ( 0x9f )  /*  DBCS Hiragana check Low  code  */
#define  U_CHK_HH      ( 0xf1 )  /*  DBCS Hiragana check High code  */

/* 0x83xx */
#define  U_CHK_KL      ( 0x40 )  /*  DBCS Katakana check Low  code  */
#define  U_CHK_KH      ( 0x96 )  /*  DBCS Katakana check High code  */

/* 7 bit code for alpha-num shift */
#define  SHIFT_ALNUM   ( 0x1d )

/*----------------------------------------------------------------------*
 *	check yomi string
 *				SBCS     -> removed
 *				Katakana -> convert into Hiragana
 *				lowor alpha -> convert into upper
 *----------------------------------------------------------------------*/
int	CheckYomi		/* no character is removed -> 0		*/
				/* some characters are removed -> 1	*/
(
unsigned char	*in,		/* yomi string to be checked		*/
int		inlen,		/* yomi (in) string length		*/
unsigned char	*out,		/* valid yomi string			*/
int		*outlen		/* valid yomi (out) string length	*/
)
{
	int	i, j, ret = 0;

	*outlen = 0;
	for(j = 0; j < inlen;){

		/* remove SBCS */
		if(*in <= 0x80 || (*in >= 0xa0 && *in <= 0xdf) ||
							*in >= 0xfd){
			in++;
			j++;
			ret = 1;
			continue;
		}

		/* DBCS */
		j += 2;
		i = *(in+1);
		if(*in == 0x81){

			/* copy one special character */
			if((i == U_2CHOUON )
				|| (i == U_2COLN)
				|| (i == U_2SEMCOLN) || (i == U_2QUES   )
				|| (i == U_2BIKKU  ) || (i == U_2SURA   )
				|| (i == U_2MLKA   ) || (i == U_2MRKA   )
				|| (i == U_2LKUCHI ) || (i == U_2RKUCHI )
				|| (i == U_2PLUS   ) || (i == U_2MAI    )
				|| (i == U_2IQU    )
				|| (i == U_2PER    )
				|| (i == U_2SHAR   ) || (i == U_2AND    )
				|| (i == U_2HOSHI  ) || (i == U_2TANKA  )){
				*out++ = *in++;
				*out++ = *in++;
				(*outlen) += 2;
				continue;
			}

		}else if(*in == 0x82){

			/* copy a character */
			if(((U_CHK_NL  <= i) && (i <= U_CHK_NH ))
				|| ((U_CHK_ALL <= i) && (i <= U_CHK_ALH))
				|| ((U_CHK_HL  <= i) && (i <= U_CHK_HH ))){
				*out++ = *in++;
				*out++ = *in++;
				(*outlen) += 2;
				continue;

			/* convert lower alphabet into upper */
			}else if((U_CHK_ASL <= i) && (i <= U_CHK_ASH)){
				*out++ = *in++;
				*out++ = (*in++) - (U_CHK_ASL - U_CHK_ALL);
				(*outlen) += 2;
				continue;
			}

		}else if(*in == 0x83){

			/* convert Katakanta into Hiragan */
			if((U_CHK_KL <= i) && (i <= 0x7e)){
				*out++ = 0x82;
				in++;
				*out++ = (*in++) + (U_CHK_HL - U_CHK_KL);
				(*outlen) += 2;
				continue;

			}else if((0x7e <= i) && (i <= U_CHK_KH)){
				*out++ = 0x82;
				in++;
				*out++ = (*in++) + (U_CHK_HL - U_CHK_KL) - 1;
				(*outlen) += 2;
				continue;
			}
		}

		/* invalid DBCS character */
		in += 2;
		ret = 1;
	}
	return ret;
}


/*----------------------------------------------------------------------*
 *	check goku string
 *				SBCS     -> removed
 *				DBCS '$' -> removed
 *----------------------------------------------------------------------*/
int	CheckGoku		/* no character is removed -> 0		*/
				/* some characters are removed -> 1	*/
(
unsigned char	*in,		/* yomi string to be checked		*/
int		inlen,		/* yomi (in) string length		*/
unsigned char	*out,		/* valid yomi string			*/
int		*outlen		/* valid yomi (out) string length	*/
)
{
	int	j, ret = 0;

	*outlen = 0;
	for(j = 0; j < inlen;){

		/* remove SBCS */
		if(*in <= 0x80 || (*in >= 0xa0 && *in <= 0xdf) ||
							*in >= 0xfd){
			in++;
			j++;
			ret = 1;
			continue;
		}

		/* DBCS */
		j += 2;
		/* remove DBCS '$' */
		if(*in == 0x81 && *(in+1) == U_2DOLLAR){
			ret = 1;
			in += 2;

		/* copy a character */
		}else{
			*out++ = *in++;
			*out++ = *in++;
			(*outlen) += 2;
		}
	}
	return ret;
}


/*----------------------------------------------------------------------*
 *	convert a SJIS character into 7 bit code (for user dictionary)
 *----------------------------------------------------------------------*/
int	To7bit			/* return 7 bit code			*/
				/* error -> -1				*/
(
unsigned char	chh,		/* SJIS character high byte		*/
unsigned char	chl,		/* SJIS character low byte		*/
int		*type		/* return page of 7 bit code (1/2)	*/
)
{
	if(chh == 0x82){

		/* hiragana */
		if(chl >= U_CHK_HL && chl <= U_CHK_HH){
			*type = 1;
			return(chl & 0x7f);

		/* numeric 0 - 9 */
		}else if(chl >= U_CHK_NL && chl <= U_CHK_NH){
			*type = 2;
			return('0' + (chl - U_CHK_NL));

		/* alpha A - Z */
		}else if(chl >= U_CHK_ALL && chl <= U_CHK_ALH){
			*type = 2;
			return('A' + (chl - U_CHK_ALL));
		}

	}else if(chh == 0x81){

		/* special character */
		switch(chl){
		case U_2CHOUON	: *type = 1; return 0x74;
		case U_2BIKKU	: *type = 2; return 0x21;
		case U_2SHAR	: *type = 2; return 0x23;
		case U_2PER	: *type = 2; return 0x25;
		case U_2AND	: *type = 2; return 0x26;
		case U_2MLKA	: *type = 2; return 0x28;
		case U_2MRKA	: *type = 2; return 0x29;
		case U_2HOSHI	: *type = 2; return 0x2a;
		case U_2PLUS	: *type = 2; return 0x2b;
		case U_2MAI	: *type = 2; return 0x2d;
		case U_2SURA	: *type = 2; return 0x2f;
		case U_2COLN	: *type = 2; return 0x3a;
		case U_2SEMCOLN	: *type = 2; return 0x3b;
		case U_2LKUCHI	: *type = 2; return 0x3c;
		case U_2IQU	: *type = 2; return 0x3d;
		case U_2RKUCHI	: *type = 2; return 0x3e;
		case U_2QUES	: *type = 2; return 0x3f;
		case U_2TANKA	: *type = 2; return 0x40;
		}

	}else if(chh == 0x83){

		/* vu (katakana) */
		if(chl == 0x94){
			*type = 1;
			return 0x1a;
		}
	}

	/* invalid character */
	return -1;
}


/*----------------------------------------------------------------------*
 *	convert from SJIS into 7 bit code
 *----------------------------------------------------------------------*/
int	SjisTo7bit	/* success -> 0					*/
			/* error (hiragana & alnum ara mixed) -> 1	*/
			/* error (invalid character) -> 2		*/
(
unsigned char	*in,		/* SJIS string to be checked		*/
int		inlen,		/* SJIS (in) string length		*/
unsigned char	*out,		/* 7 bit code string			*/
int		*outlen		/* 7 bit code (out) string length	*/
)
{
	int	i, j, t, type;

	*outlen = 0;
	type = 0;
	for(j = 0; j < inlen; j += 2){

		/* SBCS -> invalid */
		if(*in <= 0x80 || (*in >= 0xa0 && *in <= 0xdf) ||
							*in >= 0xfd){
			return 2;
		}

		if((i = To7bit(*in, *(in+1), &t)) >= 0){
			if(type == 0){
				type = t;
				if(t == 2){
					*out++ = SHIFT_ALNUM;
					(*outlen)++;
				}
			}else if(type != t){
				return 1;
			}
		}else{
				return 2;
		}
		in += 2;
		*out++ = i;
		(*outlen)++;
	}

	return 0;
}


/*----------------------------------------------------------------------*
 *	convert the first byte of a SJIS character into Modified SJIS
 *----------------------------------------------------------------------*/
int	ToMsjis			/* return Modified SJIS			*/
(
unsigned char	ch		/* SJIS character high byte		*/
)
{
	unsigned char	low, high;

	low = 0x0f & ch;
	high = 0xf0 & ch;

	switch(high){
		case 0x80:	high = 0x00;	break;
		case 0x90:	high = 0x10;	break;
		case 0xe0:	high = 0x20;	break;
		case 0xf0:	high = 0x30;	break;
	}
	return(high + low);
}


/*----------------------------------------------------------------------*
 *	convert from SJIS into Modified SJIS (for dictionary)
 *----------------------------------------------------------------------*/
int	SjisToMsjis	/* success -> 0					*/
			/* error (invalid character) -> 2		*/
(
unsigned char	*in,		/* SJIS string to be checked		*/
int		inlen,		/* SJIS (in) string length		*/
unsigned char	*out,		/* Modufied SJIS string			*/
int		*outlen		/* Modufied SJIS (out) string length	*/
)
{
	int	i, j;

	*outlen = 0;
	for(j = 0; j < inlen - 2; j += 2){

		/* SBCS -> invalid */
		if(*in <= 0x80 || (*in >= 0xa0 && *in <= 0xdf) ||
							*in >= 0xfd){
			return 2;
		}

		*out++ = ToMsjis(*in++);
		*out++ = *in++;
		(*outlen) += 2;
	}
	*out++ = *in++;
	*out++ = *in++;
	(*outlen) += 2;

	return 0;
}
