/* @(#)88	1.1  src/bos/usr/lib/piosCN/lib/lc.h, ils-zh_CN, bos41J, 9507B 1/26/95 09:56:18  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
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

typedef struct _FONTinfo {
	int	flags;
#define	NO_QUERY	0
#define	FOUND		1
#define	NOT_FOUND	2
	unsigned long	pFont;	/* FontPtr */
	char*	cs_registry;
	char*	cs_encoding;
} *FONTinfo, FONTinforec;

typedef struct _FONTtbl {
	int	flags;
	FONTinfo	FONTptr;
} *FONTtbl, FONTtblrec;

typedef struct _CSinfo{
	int	flags;
#define	NO_FONT	0
#define	FONT_P	1
#define	FONT_X	2
	int	wid;
	unsigned long	pFont;	/* FontPtr */
	char*	iconvname;
	int	n;
	FONTtbl	FTptr;
} *CSinfo, CSinforec;

typedef struct _CODEinfo{
	char*	codesetname;
	char*	localename;
	int	CSnum;
	CSinfo	CSptr;
} *CODEinfo, CODEinforec;

typedef struct _LOCinfo{
	char*	localename;
	CODEinfo	Cptr;
} *LOCinfo, LOCinforec;

typedef struct	_CURcodeset{
	LOCinfo	lci;
	CODEinfo	cdi;
	unsigned long*	iconvs;
/* the followings are for special code sets */
	int	state;
#define ST_NONE	0
#define	ST_KO	1
#define	ST_KI	2
	char*	realname;
	unsigned char*	(*pre_csid)();
	unsigned char*	(*pre_conv)();
	unsigned char*	(**post_conv)();
} *CURcodeset, CURcodesetrec;



CURcodeset	lc_init( LOCinfo lcip, CODEinfo cdip, CSinfo csip, char* codesetname );
void	fix_JPkana( CODEinfo cdip, FONTinfo fip );
void    lc_ch_cs_font( CODEinfo cdip, char* codesetnme, int csid, int flags );
void	lc_p_code_change( CSinfo csip, char* p_codesetname );
int	xcsid( unsigned char* chars, CURcodeset ccsp );
int     is_FONT_P( int csid, CURcodeset ccsp );
int	get_wid( int csid, CURcodeset ccsp );
unsigned char*	get_iFont( unsigned char* chars, CURcodeset ccsp, int csid );
unsigned long	get_pFont( CURcodeset ccsp, int csid );
