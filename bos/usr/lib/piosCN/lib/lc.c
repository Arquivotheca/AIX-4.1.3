static char sccsid[] = "@(#)87	1.3  src/bos/usr/lib/piosCN/lib/lc.c, ils-zh_CN, bos41J, 9516A_all 4/17/95 15:24:06";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: JISconv_init
 *		fix_JPkana
 *		get_iFont
 *		get_pFont
 *		get_wid
 *		init_CSinfo
 *		is_FONT_P
 *		jis78topc
 *		jis78topc_fast
 *		jis83topc
 *		jis83topc_fast
 *		jis90topc
 *		jis90topc_fast
 *		lc_ch_cs_font
 *		lc_fin
 *		lc_init
 *		lc_init0
 *		lc_p_code_change
 *		open_iconvs
 *		pctojis83
 *		pctojis90
 *		pctosjis
 *		show_CSinfo
 *		show_CURcodeset
 *		show_FONTinfo
 *		sjistopc
 *		sjistopc_fast
 *		sp_JIS_check
 *		sp_alias_to_name
 *		sp_lc_init
 *		sp_open_iconv
 *		xcsid
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include	"lc.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<locale.h>
#include	<langinfo.h>
#include	<iconv.h>

/*---------------- locale information fix to use with JP-printer -----------
	xcsid
	fix_JPkana
	sp_lc_init
	sp_open_iconv
--------------------------------------------------------------------------*/

int	xcsid( unsigned char* chars, CURcodeset ccsp )
{
	int	l;
	int	csi;
	static b[6];


	if( ccsp->pre_csid ){
		strncpy( b, chars, 6 );
		chars = (*ccsp->pre_csid)( b, ccsp->state );
	}

	l = mblen( chars, MB_CUR_MAX );
	if( l < 0 || l > strlen( chars ) ) return( -1 );
	csi = csid( chars );

        if( csi == 3 && ccsp && ccsp->cdi && ccsp->cdi->codesetname ){
                /* Japanese User defined charcters area */
                if( strcmp( ccsp->cdi->codesetname, "IBM-eucJP" ) == 0 ){
                        if( chars[1] > 0xf4 || chars[2] == 0 ) csi = 4;
                }else if( strcmp( ccsp->cdi->codesetname, "IBM-932" ) == 0 ){
                        if( chars[0] >= 0xf0 && chars[0] <= 0xf9 ) csi = 4;
                }
        }
	return( csi );
}


void	fix_JPkana( CODEinfo cdip, FONTinfo fip )
{
	FONTinfo	fipnew, fipcur;

	for( fipnew = fipcur = 0; fip && fip->cs_registry; fip++ ){
		if( strcmp( fip->cs_registry, "jisx0201.1976" ) == 0 ){
			if( strcmp( fip->cs_encoding, "0" ) == 0 ){
				if( fip->flags == FOUND ) fipcur = fip;
			}else if( strcmp( fip->cs_encoding, "1" ) == 0 ){
				if( fip->flags == FOUND ) fipnew = fip;
			}
		}
	}

	if( !fipnew && !fipcur ) return;

	for( ; cdip && cdip->CSnum; cdip++ ){
		if( strcmp( cdip->codesetname, "IBM-932" ) == 0 ||
		    strcmp( cdip->codesetname, "IBM-eucJP" ) == 0 ){
			if( fipcur ){
				cdip->CSptr[2].iconvname = "JISX0201.1976-0";
				cdip->CSptr[2].FTptr[0].FONTptr = fipcur;
			}else if( fipnew ){
				cdip->CSptr[2].iconvname = "JISX0201.1976-GL";
				cdip->CSptr[2].FTptr[0].FONTptr = fipnew;
			}
		}
	}
}

#if defined( SJIS90 )
static unsigned short	CP932toSJIS90[][2] = {
	{0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, {0x8a61, 0xe579}, {0x8a68, 0x9d98}, 
	{0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, {0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, 
	{0x8c7a, 0xe8f2}, {0x8d56, 0xfad0}, {0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, 
	{0x9078, 0xe8d5}, {0x9147, 0xe6cb}, {0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, 
	{0x938e, 0x9e8d}, {0x9393, 0x9fb7}, {0x93f4, 0xe78e}, {0x9488, 0xe5a2}, 
	{0x954f, 0x9e77}, {0x968a, 0xeaa0}, {0x9699, 0x98d4}, {0x96f7, 0xe54d}, 
	{0x9779, 0xeaa1}, {0x9855, 0xe2c4}, {0x98d4, 0x9699}, {0x9ae2, 0x92d9}, 
	{0x9d98, 0x8a68}, {0x9e77, 0x954f}, {0x9e8d, 0x938e}, {0x9fb7, 0x9393}, 
	{0x9ff3, 0x8ac1}, {0xe086, 0xeaa4}, {0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, 
	{0xe1e8, 0x9376}, {0xe27d, 0x8a96}, {0xe2c4, 0x9855}, {0xe541, 0x8ec7}, 
	{0xe54d, 0x96f7}, {0xe579, 0x8a61}, {0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, 
	{0xe6cb, 0x9147}, {0xe78e, 0x93f4}, {0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, 
	{0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, {0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, 
	{0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}, {0xeaa4, 0xe086}, {0xfa54, 0x81ca}, 
	{0xfa5b, 0x81e6}, {0xfad0, 0x8d56}, 
	};

static unsigned short	SJIS90toCP932[][2] = {
	{0x81ca, 0xfa54}, {0x81e6, 0xfa5b}, {0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, 
	{0x8a61, 0xe579}, {0x8a68, 0x9d98}, {0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, 
	{0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, {0x8c7a, 0xe8f2}, {0x8d56, 0xfad0}, 
	{0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, {0x9078, 0xe8d5}, {0x9147, 0xe6cb}, 
	{0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, {0x938e, 0x9e8d}, {0x9393, 0x9fb7}, 
	{0x93f4, 0xe78e}, {0x9488, 0xe5a2}, {0x954f, 0x9e77}, {0x968a, 0xeaa0}, 
	{0x9699, 0x98d4}, {0x96f7, 0xe54d}, {0x9779, 0xeaa1}, {0x9855, 0xe2c4}, 
	{0x98d4, 0x9699}, {0x9ae2, 0x92d9}, {0x9d98, 0x8a68}, {0x9e77, 0x954f}, 
	{0x9e8d, 0x938e}, {0x9fb7, 0x9393}, {0x9ff3, 0x8ac1}, {0xe086, 0xeaa4}, 
	{0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, {0xe1e8, 0x9376}, {0xe27d, 0x8a96}, 
	{0xe2c4, 0x9855}, {0xe541, 0x8ec7}, {0xe54d, 0x96f7}, {0xe579, 0x8a61}, 
	{0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, {0xe6cb, 0x9147}, {0xe78e, 0x93f4}, 
	{0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, {0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, 
	{0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, {0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}, 
	{0xeaa4, 0xe086}, {0xfad0, 0x8d56}, 
	};
#endif /* SJIS90 */
static unsigned short	CP932toSJIS83[][2] = {
	{0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, {0x8a61, 0xe579}, {0x8a68, 0x9d98}, 
	{0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, {0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, 
	{0x8c7a, 0xe8f2}, {0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, 
	{0x9078, 0xe8d5}, {0x9147, 0xe6cb}, {0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, 
	{0x938e, 0x9e8d}, {0x9393, 0x9fb7}, {0x93f4, 0xe78e}, {0x9488, 0xe5a2}, 
	{0x954f, 0x9e77}, {0x968a, 0xeaa0}, {0x9699, 0x98d4}, {0x96f7, 0xe54d}, 
	{0x9779, 0xeaa1}, {0x9855, 0xe2c4}, {0x98d4, 0x9699}, {0x9ae2, 0x92d9}, 
	{0x9d98, 0x8a68}, {0x9e77, 0x954f}, {0x9e8d, 0x938e}, {0x9fb7, 0x9393}, 
	{0x9ff3, 0x8ac1}, {0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, 
	{0xe1e8, 0x9376}, {0xe27d, 0x8a96}, {0xe2c4, 0x9855}, {0xe541, 0x8ec7}, 
	{0xe54d, 0x96f7}, {0xe579, 0x8a61}, {0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, 
	{0xe6cb, 0x9147}, {0xe78e, 0x93f4}, {0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, 
	{0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, {0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, 
	{0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}, {0xfa54, 0x81ca}, 
	{0xfa5b, 0x81e6},
	};

static unsigned short	SJIS83toCP932[][2] = {
	{0x81ca, 0xfa54}, {0x81e6, 0xfa5b}, {0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, 
	{0x8a61, 0xe579}, {0x8a68, 0x9d98}, {0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, 
	{0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, {0x8c7a, 0xe8f2},
	{0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, {0x9078, 0xe8d5}, {0x9147, 0xe6cb}, 
	{0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, {0x938e, 0x9e8d}, {0x9393, 0x9fb7}, 
	{0x93f4, 0xe78e}, {0x9488, 0xe5a2}, {0x954f, 0x9e77}, {0x968a, 0xeaa0}, 
	{0x9699, 0x98d4}, {0x96f7, 0xe54d}, {0x9779, 0xeaa1}, {0x9855, 0xe2c4}, 
	{0x98d4, 0x9699}, {0x9ae2, 0x92d9}, {0x9d98, 0x8a68}, {0x9e77, 0x954f}, 
	{0x9e8d, 0x938e}, {0x9fb7, 0x9393}, {0x9ff3, 0x8ac1},
	{0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, {0xe1e8, 0x9376}, {0xe27d, 0x8a96}, 
	{0xe2c4, 0x9855}, {0xe541, 0x8ec7}, {0xe54d, 0x96f7}, {0xe579, 0x8a61}, 
	{0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, {0xe6cb, 0x9147}, {0xe78e, 0x93f4}, 
	{0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, {0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, 
	{0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, {0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}, 
	};

unsigned char*	pctosjis( unsigned char* p )
{
	unsigned int	pc;
	int	ulim, llim, cur;	/* upper/lower limit for b-search */


	if( p == 0 || p[0] == 0 || p[1] == 0 || p[2] != 0 )	return( p );
	pc = ( (unsigned)p[0] << 8 ) + (unsigned)p[1];
	/*
	 *	convert 932 to JISX0208-1983 order.
	 */
	llim = 0;
	ulim = sizeof (CP932toSJIS83) / sizeof (CP932toSJIS83[0]);
	while (llim <= ulim) {
		cur = llim + ulim >> 1;
		if (pc < CP932toSJIS83[cur][0])
			ulim = cur - 1;
		else if (pc > CP932toSJIS83[cur][0])
			llim = cur + 1;
		else {
			pc = CP932toSJIS83[cur][1];
			p[0] = ( pc >> 8 ) & 0xff;
			p[1] = pc & 0xff;
			break;
		}
	}

	return( p );
}

unsigned char*	sjistopc( unsigned char* p )
{
	unsigned int	sjis;
	int	ulim, llim, cur;	/* upper/lower limit for b-search */

	if( p == 0 || p[0] == 0 || p[1] == 0 || p[2] != 0 )	return( p );
	sjis = ( (unsigned)p[0] << 8 ) + (unsigned)p[1];
	/*
	 *	convert JISX0208-1983 order to 932.
	 */
	llim = 0;
	ulim = sizeof (SJIS83toCP932) / sizeof (SJIS83toCP932[0]);
	while (llim <= ulim) {
		cur = llim + ulim >> 1;
		if (sjis < SJIS83toCP932[cur][0])
			ulim = cur - 1;
		else if (sjis > SJIS83toCP932[cur][0])
			llim = cur + 1;
		else {
			sjis = SJIS83toCP932[cur][1];
			p[0] = ( sjis >> 8 ) & 0xff;
			p[1] = sjis & 0xff;
			break;
		}
	}

	return( p );
}

unsigned char* sjistopc_fast( unsigned char* p )
{
	/* to get correct csid value */
	if( p && p[0] == 0x81 && p[2] == 0 ){
		if( p[1] == 0xca ){
			p[0] = 0xfa;
			p[1] = 0x54;
		}else if( p[1] == 0xe6 ){
			p[0] = 0xfa;
			p[1] = 0x5b;
		}
	}

	return( p );
}

static iconv_t	cd_j78, cd_j90, cd_j90r;

static void JISconv_init()
{
	if( !cd_j78 ){
		cd_j78 = iconv_open( "IBM-932", "JISX0208.1978-GL" );
	}
	if( !cd_j90 ){
		cd_j90 = iconv_open( "IBM-932", "JISX0208.1983-GL" );
	}
	if( !cd_j90r ){
		cd_j90r = iconv_open( "JISX0208.1983-GL", "IBM-932" );
	}
}

static unsigned char* jis90topc( unsigned char* chars, int st )
{
	int	rc, n = strlen( chars ), i = 10;
	static unsigned char	buff[10];
	unsigned char*	s = chars;
	unsigned char*	p = buff;

	if( st == ST_KI ){
		rc = iconv( cd_j90, (const unsigned char**)&s, (unsigned long*)&n, &p, (unsigned long*)&i );
		if( rc == 0 ){
			*p = 0;
			strcpy( chars, buff );
		}else{
			chars[0] = 0;
		}
	}

	return( chars );
}

static unsigned char* jis90topc_fast( unsigned char* chars, int st )
{
	if( st == ST_KI && chars && chars[1] == 0 ){
		chars[0] = 0x81;
		return( chars );
	}

	return( jis90topc( chars, st ) );
}

static unsigned char* pctojis90( unsigned char* chars )
{
	int	rc, n = strlen( chars ), i = 10;
	static unsigned char	buff[10];
	unsigned char*	s = chars;
	unsigned char*	p = buff;

	rc = iconv( cd_j90r, (const unsigned char**)&s, (unsigned long*)&n, &p, (unsigned long*)&i );
	if( rc == 0 ){
		*p = 0;
		strcpy( chars, buff );
	}else{
		chars[0] = 0;
	}

	return( chars );
}

static unsigned char* jis83topc( unsigned char* chars, int st )
{
	if( chars && st == ST_KI && chars[2] == 0 ){
		if( chars[0] == 0x74 && chars[1] == 0x26 ){
			chars[0] = 0x5f;
			chars[1] = 0x66;
		}else if( chars[0] == 0x5f && chars[1] == 0x66 ){
			chars[0] = 0x74;
			chars[1] = 0x26;
		}else if( chars[0] == 0x39 && chars[1] == 0x37 ){
			/* pc code direct */
			chars[0] = 0x8d;
			chars[1] = 0x56;
			return( chars );
		}
	}

	return( jis90topc( chars, st ) );
}

static unsigned char* jis83topc_fast( unsigned char* chars, int st )
{
	if( st == ST_KI && chars && chars[1] == 0 ){
		chars[0] = 0x81;
		return( chars );
	}

	return( jis83topc( chars, st ) );
}

static unsigned char* pctojis83( unsigned char* chars )
{
	if( chars && chars[2] == 0 ){
		if( chars[0] == 0x8d && chars[1] == 0x56 ){
			chars[0] = 0x39;
			chars[1] = 0x37;
			return( chars );
		}

		chars = pctojis90( chars );
		/* jis90 to jis83 */
		if( chars[0] == 0x74 && chars[1] == 0x26 ){
			chars[0] = 0x5f;
			chars[1] = 0x66;
		}else if( chars[0] == 0x5f && chars[1] == 0x66 ){
			chars[0] = 0x74;
			chars[1] = 0x26;
		}else if( chars[0] == 0x39 && chars[1] == 0x37 ){
			/* double wide space */
			/* chars[0] = chars[1] = 0x21; */
			;
		}
	}

	return( chars );
}

static unsigned char* jis78topc( unsigned char* chars, int st )
{
	int	rc, n = strlen( chars ), i = 10;
	static unsigned char	buff[10];
	unsigned char*	s = chars;
	unsigned char*	p = buff;

	if( st == ST_KI ){
		rc = iconv( cd_j78, (const unsigned char**)&s, (unsigned long*)&n, &p, (unsigned long*)&i );
		if( rc == 0 ){
			*p = 0;
			strcpy( chars, buff );
		}else{
			chars[0] = 0;
		}
	}

	return( chars );
}

static unsigned char* jis78topc_fast( unsigned char* chars, int st )
{
	if( st == ST_KI && chars && chars[1] == 0 ){
		chars[0] = 0x81;
		return( chars );
	}

	return( jis78topc( chars, st ) );
}


static struct sp_cd_info{
	char*	name;
	char*	prcodeset;
	int	init_state;
	unsigned char*	(*pre_csid)();
	unsigned char*	(*pre_conv)();
	unsigned char*	(*post_conv)();
} Sp_code_info[5] = {
	{ "SJIS83",  "Ja_JP", ST_NONE, sjistopc_fast,  sjistopc,  pctosjis  },
	/* as for 932 to jis90,78, iconv converter should exist. */
	{ "JISX0208.1983-GL", "Ja_JP", ST_KO, jis90topc_fast, jis90topc, 0 },
	{ "jis83", "Ja_JP", ST_KO, jis83topc_fast, jis83topc, pctojis83 },
	{ "JISX0208.1978-GL", "Ja_JP", ST_KO, jis78topc_fast, jis78topc, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

static struct sp_cd_alias{
	char*	aliasname;
	char*	realname;
} Sp_cd_alias[7] = {
	{ "sjis", "SJIS83" },
	{ "sjis83", "SJIS83" },
	{ "jis90", "JISX0208.1983-GL" },
	{ "JISX0208.1983-0", "JISX0208.1983-GL" },
	{ "jis78", "JISX0208.1978-GL" },
	{ "JISX0208.1978-0", "JISX0208.1978-GL" },
	{ 0, 0 }
};

static char*	sp_alias_to_name( char* alias )
{
	struct sp_cd_alias*	x;


	for( x = Sp_cd_alias; x->aliasname; x++ ){
		if( strcmp( x->aliasname, alias ) == 0 ){
			return( x->realname );
		}
	}

	return( alias );
}


static CURcodeset	sp_lc_init( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname )
{
	struct sp_cd_info*	x;
	char*	n;
	int	i;
	CURcodeset	ccsp;
	static CURcodeset	lc_init0( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname, char* realname );


	codesetname = sp_alias_to_name( codesetname );

	for( x = Sp_code_info; x->name; x++ ){
		if( strcmp( x->name, codesetname ) == 0 ){
			i = strlen( codesetname ) + 1;

			if( ( n = calloc( sizeof( char ), i ) ) == 0 ){
				return( 0 );
			}

			strcpy( n, codesetname );
			ccsp = lc_init0( lcip, cdip, csip, x->prcodeset, n );
			ccsp->pre_csid = x->pre_csid;
			ccsp->state = x->init_state;
			if( !sp_JIS_check(ccsp) ) ccsp->pre_conv = x->pre_conv;
			return( ccsp );
		}
	}

	return( 0 );
}

static void	sp_open_iconv( CURcodeset ccsp, int csid, char* fromname, char* toname )
{
	struct sp_cd_info*	x;

	if( strcmp( ccsp->realname, sp_alias_to_name( toname ) ) == 0 ){
		ccsp->pre_conv = 0;
		return;
	}

	for( x = Sp_code_info; x->name; x++ ){
		if( strcmp( x->name, toname ) == 0 ){
			if( strcmp( x->prcodeset, fromname ) ){
				ccsp->iconvs[csid] = (unsigned long)iconv_open(
					x->prcodeset, fromname );
				if( ccsp->iconvs[csid] == 0 ){
					/* if iconv_open returns -1 */
					ccsp->iconvs[csid] = 0;
					return;
				}
			}

			ccsp->post_conv[csid] = x->post_conv;
			return;
		}
	}
}

static int	sp_JIS_check( CURcodeset ccsp )
{
	char* s;
	char* x;


	if( !ccsp || !ccsp->cdi || ccsp->cdi->CSnum < 2 ) return( 0 );
	s = sp_alias_to_name( ccsp->cdi->CSptr[1].iconvname );
	x = sp_alias_to_name( ccsp->realname );

	if( strcmp( x, "JISX0208.1983-GL" ) == 0 ||
	    strcmp( x, "jis83" ) == 0 || strcmp( s, "jis83" ) == 0 ||
	    strcmp( x, "JISX0208.1978-GL" ) == 0 ){
		JISconv_init();
		if( strcmp( s, x ) == 0 ){
			ccsp->pre_conv = 0;
			return( 1 );
		}
	}

	return( 0 );
}

/*---------------- locale information fix to use with JP-printer end -------*/

/* codesetname can be codesetname like IBM-850 or localename like en_US */
CURcodeset	lc_init( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname )
{
	static CURcodeset	lc_init0( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname, char* realname );

	return( lc_init0( lcip, cdip, csip, codesetname, codesetname ) );
}


static CURcodeset	lc_init0( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname, char* realname )
{
	CODEinfo	p;
	LOCinfo	l;
	CURcodeset	ccsp;
	static void	init_CSinfo( CSinfo csip );
	static void	open_iconvs( CURcodeset ccsp );

	if( codesetname && codesetname[0] == '-' && codesetname[1] == 0 ){
		realname = codesetname = setlocale( LC_CTYPE, 0 );
	}

	for( l = lcip; l->localename; l++ ){
		if( strcmp( codesetname, l->localename ) == 0 ){
			if( ( ccsp = calloc( sizeof( CURcodesetrec ), 1 ) ) == 0 ){
				return( 0 );
			}
			setlocale( LC_ALL, l->localename );
			ccsp->lci = l;
			ccsp->cdi = l->Cptr;
			if( strcmp( realname, l->localename ) == 0 ){
				ccsp->realname = ccsp->cdi->codesetname;
			}else{
				ccsp->realname = realname;
			}
			init_CSinfo( csip );
			open_iconvs( ccsp );
			return( ccsp );
		}
	}

	for( p = cdip; p->codesetname; p++ ){
		if( strcmp( codesetname, p->codesetname ) == 0 ){
			return( lc_init0( lcip, cdip, csip, p->localename, realname ) );
		}
	}

/* special code set for JP */
	return( sp_lc_init( lcip, cdip, csip, codesetname ) );
}


void	lc_fin( CURcodeset ccsp )
{
	int	i;


	if( !ccsp || !ccsp->cdi || ccsp->cdi->CSnum <= 0 )	return;

	for( i = 0; i < ccsp->cdi->CSnum; i++ ){
		if( ccsp->iconvs[i] )	iconv_close( (iconv_t)ccsp->iconvs[i] );
	}
}


static void	open_iconvs( CURcodeset ccsp )
{
	int	i;
	unsigned long	x;
	CODEinfo	cdip;
	CSinfo	csip;


	if( !ccsp || !ccsp->cdi || ccsp->cdi->CSnum <= 0 )	return;
	cdip = ccsp->cdi;
	if( ( ccsp->iconvs = calloc( sizeof( iconv_t ), cdip->CSnum ) ) == 0 ||
	    ( ccsp->post_conv = calloc( sizeof( unsigned char* (*)() ), cdip->CSnum ) ) == 0 ){
		return;
	}

	if( sp_JIS_check( ccsp ) ) return;

	for( i = 0, csip = cdip->CSptr; i < cdip->CSnum; i++, csip++ ){
		if( strcmp( csip->iconvname, cdip->codesetname ) ){
			x = (unsigned long)iconv_open(
					csip->iconvname, cdip->codesetname );
			ccsp->iconvs[i] = x;
			if( ccsp->iconvs[i] == 0 ){
				sp_open_iconv( ccsp, i, cdip->codesetname, csip->iconvname );
			}
		}
	}
}


static void	init_CSinfo( CSinfo csip )
{
	FONTtbl	ftp;
	int	i, j;

	for( ; csip && csip->n; csip++ ){
		if( csip->flags == FONT_X ){
			ftp = csip->FTptr; 
			for( i = 0; i < csip->n; i++, ftp++ ){
				if( ftp && ftp->FONTptr && ftp->FONTptr->flags == FOUND ){
					csip->pFont = ftp->FONTptr->pFont;
					break;
				}
			}

			if( i >= csip->n ) csip->flags = NO_FONT;
		}
	}
}

void	lc_ch_cs_font( CODEinfo cdip, char* codesetname, int csid, int flags )
{
	if( cdip && csid >= 0 ){
		for( ; cdip->CSnum; cdip++ ){
			if( strcmp( codesetname, cdip->codesetname ) == 0 ){
				if( cdip->CSptr && csid < cdip->CSnum ){
					switch( flags ){
					case FONT_X:
					case FONT_P:
					case NO_FONT:
						cdip->CSptr[csid].flags = flags;
						break;
					default:
						break;
					}
				}
			}
		}
	}
}


void    lc_p_code_change( CSinfo csip, char* p_codesetname )
{
	char*	s;
	int	i = strlen( p_codesetname ) + 1;


	if( !csip || ( s = calloc( sizeof( char ), i ) ) == 0 ){
		return;
	}

	strcpy( s, p_codesetname );

	for( ; csip->n; csip++ ){
		if( csip->flags == FONT_P ){
			csip->iconvname = s;
		}
	}
}




int	is_FONT_P( int csid, CURcodeset ccsp )
{
	if( !ccsp || !ccsp->cdi || csid >= ccsp->cdi->CSnum || csid < 0 ){
		return( 0 );
	}

	return( ccsp->cdi->CSptr && ccsp->cdi->CSptr[csid].flags == FONT_P );
}

int	get_wid( int csid, CURcodeset ccsp )
{
	if( !ccsp || !ccsp->cdi || csid >= ccsp->cdi->CSnum || csid < 0 || ccsp->cdi->CSptr == 0 ){
		return( 0 );
	}

	return( ccsp->cdi->CSptr[csid].wid );
}

unsigned char*	get_iFont( unsigned char* chars, CURcodeset ccsp, int csid )
{
#define	SBSIZE	34
	static unsigned char	buff[SBSIZE];
	unsigned char*	p = buff;
	int	i = SBSIZE;
	int	n = strlen( chars );
	int	rc;


	if( !ccsp || !ccsp->cdi || csid >= ccsp->cdi->CSnum || csid < 0 ){
		return( 0 );
	}

	if( chars[0] == 0 ){
		buff[0] = buff[1] = 0;
		return( buff );
	}

	if( ccsp->pre_conv ) chars = (*ccsp->pre_conv)( chars, ccsp->state );

	if( ccsp->cdi->CSptr[csid].flags == NO_FONT ){
		for( i = 0; i < ccsp->cdi->CSptr[csid].wid; i++ ){
			buff[i] = ' ';
		}
		buff[i] = 0;
	}else if( ccsp->iconvs[csid] ){
		rc = iconv( (iconv_t)ccsp->iconvs[csid], (const unsigned char**)&chars, (unsigned long*)&n, &p, (unsigned long*)&i );
		if( rc == 0 ){
			*p = 0;
			if( ccsp->post_conv[csid] ) (*ccsp->post_conv[csid])( buff );
		}
	}else{
		if( ccsp->post_conv[csid] ) (*ccsp->post_conv[csid] )( chars );
		strncpy( buff, chars, SBSIZE - 2 );
	}

	return( buff );
}

unsigned long	get_pFont( CURcodeset ccsp, int csid )
{
	CODEinfo	cdip;
	CSinfo	csip;

	if( !ccsp || !ccsp->cdi || csid < 0 ) return( 0 );
	cdip = ccsp->cdi;
	if( csid >= cdip->CSnum ) return( 0 );
	csip = cdip->CSptr + csid;
	if( csip && csip->flags == FONT_X )	return( csip->pFont );
	return( 0 );
}


void	show_FONTinfo( FONTinfo fip )
{
	for( ; fip->cs_registry || fip->cs_encoding; fip++ ){
		fprintf( stderr, " flags = %d\n", fip->flags );
		fprintf( stderr, "pFont = %x\n", fip->pFont );
		fprintf( stderr, "cs_registry = %s\n", fip->cs_registry );
		fprintf( stderr, "cs_encoding = %s\n", fip->cs_encoding );
	}
}

void	show_CSinfo( CSinfo csip )
{
	for( ; csip->n; csip++ ){
		fprintf( stderr, " flags = %d\n", csip->flags );
		fprintf( stderr, "pFont = %x\n", csip->pFont );
		fprintf( stderr, "iconvname = %s\n", csip->iconvname );
		fprintf( stderr, "n = %d\n", csip->n );
		fprintf( stderr, "FTptr = %x\n", csip->FTptr );
	}
}

void	show_CURcodeset( CURcodeset ccsp )
{
	if( ccsp ){
		int	i;
		fprintf( stderr, "realname = %s\n", ccsp->realname );
		fprintf( stderr, "codeset = %s\n", ccsp->cdi->codesetname );
		fprintf( stderr, "state = %d\n", ccsp->state );

		for( i = 0; i < ccsp->cdi->CSnum; i++ ){
			fprintf( stderr, "iconv[%d] = %x\n", i, ccsp->iconvs[i] );
		}

		fprintf( stderr, "pre_csid = %x\n", ccsp->pre_csid );
		fprintf( stderr, "pre_conv = %x\n", ccsp->pre_conv );

		for( i = 0; i < ccsp->cdi->CSnum; i++ ){
			fprintf( stderr, "post_conv[%d] = %x\n", i, ccsp->post_conv[i] );
		}
	}
}
