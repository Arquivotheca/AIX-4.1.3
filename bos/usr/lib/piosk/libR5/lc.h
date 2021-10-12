/* @(#)97	1.4 9/16/93 22:26:11 */

/*
 *   COMPONENT_NAME: (CMDPIOSK) Japanese Printer Backend
 *
 *   FUNCTIONS: lc.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	_LC_H
#define	_LC_H

typedef struct _FONTinfo {
	int	flags;
#define	NO_QUERY	0
#define	FOUND		1
#define	NOT_FOUND	2
	unsigned long	pFont;	/* FontPtr */
	char*	cs_registry;
	char*	cs_encoding;
	char* 	filename;
} *FONTinfo, FONTinforec;

typedef struct _FONTtbl {
	int	flags;
	FONTinfo	FONTptr;
} *FONTtbl, FONTtblrec;

struct reg_list {
    char *registry;
    char *encoding;
    struct reg_list *next;
} *reg_l;

typedef struct _CSinfo{
	int	flags;
#define	NO_FONT	0
#define	FONT_P	1
#define	FONT_X	2
	int	wid;
	unsigned long	*pFont;	/* FontPtr */
	char*	iconvname;
	char* 	filename;
	FONTtbl	FTptr;
	struct  reg_list *reg;
} *CSinfo, CSinforec;



typedef struct	_CURcodeset{
/* the followings are for special code sets */
	int	state;
#define ST_NONE	0
#define	ST_KO	1
#define	ST_KI	2
#define JIS     3
	FONTinfo FIP;
	int	 CSnum;
	CSinfo   CSP;
	char*	 realname;
	char*    localename;
	char*    inputcodeset;
	char*    proccodeset;
	char*    codesetname;
	unsigned long	pre_iconv;
	unsigned long*	post_iconv;
	struct _Trans_data *pre_trans;
	int      pre_trans_num;
	struct _Trans_data **post_trans;
	int      *post_trans_num;
} *CURcodeset, CURcodesetrec;



/*
CURcodeset	lc_init( char* codesetname, char* strti, char* strTo );
int     is_FONT_P( int csid, CURcodeset ccsp );
int	get_wid( int csid, CURcodeset ccsp );
int	get_iFont( unsigned char* chars, CURcodeset ccsp, int csid );
unsigned long	get_pFont( CURcodeset ccsp, int csid );
*/

#ifdef DEBUG
#define DEBUGPR( msg ) {fprintf	msg;}
#else  /* DEBUG */
#define DEBUGPR( msg )
#endif /* DEBUG */

#endif /* _LC_H */
