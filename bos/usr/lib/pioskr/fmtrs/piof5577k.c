static char sccsid[] = "@(#)77  1.4  src/bos/usr/lib/pioskr/fmtrs/piof5577k.c, cmdpioskr, bos411, 9428A410j 6/3/94 12:59:26";
/******************************************************************************
*								              *
* COMPONEMT_NAME: CMDPIOSKR						      *
*									      *
* Print Formatter For 			                                      *
*	    		1: 	IBM 5575 Model B02/F02, 4208 model 502	      *
*			2:	IBM 5577 Model B02/F02/H02/FU2, 5579 model H02*
*			3:	IBM 5587 Model G01			      *
*			4:	IBM 5587 Model H01     			      *
*			5:	IBM 4216 Model 510			      *
*			6:	IBM 5327 Model 011     			      *
*			7:	IBM 5572				      *
*									      *
* FUNCTIONS: setup, initialize, lineout, passthru, restore, 		      *
*									      *
* ORIGINS: 27 								      *
*									      *	
* IBM Confidential -- (IBM Confidential Restricted when 		      *
* combined with the aggregated modules for this product) 		      *
*                  SOURCE MATERIALS					      *
* (C) COPYRIGHT International Business Machines Corp. 1989,1991		      *
* All Rights Reserved							      *
* Licensed Material - Property of IBM					      *
* 									      *
* US Government Users Restricted Rights - Use, duplication or		      *
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.	      *
*									      *
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>

#if defined( AIX32 )
# define	NLCHARMAX 257
# include	<nl_types.h>
# include	<langinfo.h>
# include	<iconv.h>
# undef _ICONV_INTERNAL
#endif

#include "piostruct.h"

#include "kfmtrs_msg.h"
#if defined( X11R5BOS )
#include "lc.h"
#include "ffaccess.h"
#else /* X11R5BOS */
#include "snfaccess.h"
#endif /* X11R5BOS */

/******************************************************************************
* Global Variables                                                            *
******************************************************************************/
#if defined( X11R5BOS )
CURcodeset	Ccs;
int	Ccsid;					/* current csid */
#endif /* X11R5BOS */
int count;					/* Character Counter */ 
int hpos;					/* Hor. Position */
int vpos;					/* Ver. Position */
int trunc;					/* Truncate Flag */
int vtab_base;					/* Ver. Tab Base */
int cr_indent;					/* Indent by CR Dot Position */
struct shar_vars sharevars;
/******************************************************************************
* External Variables For User-Defined Font                                    *
*******************************************************************************/
int u_des;					/* File Desc. For User Font */
int uuse;					/* User Font Available Flag */
#if !defined( X11R5BOS )
char *u_font = (char *)0;			/* User-defined Font File */
char *u_ptr = (char *)0;			/* ptr to UDF header */
snffontRec snffont;				/* structure for snf font */
#endif /* X11R5BOS */
UdfInfoRec uinfo;				/* structure for user font */
/******************************************************************************
* Macro Definitions                                                           *
******************************************************************************/
int pglen;
#define SET_Pglen(lines)	{Pglen = lines;\
				 pglen = lines * Vincr;}
int tmarg;
#define SET_Tmarg(lines)	{Tmarg = lines;\
				 tmarg = lines * Vincr;}
int bmarg;
#define SET_Bmarg(lines)	{Bmarg = lines;\
				 bmarg = lines * Vincr;}
#define SET_Lmarg(chars)	{Indent = chars;\
				 lmarg = (chars * Hres * 10)/\
					      (Condensed ? 180:(Pitch * 2));}

#define SET_Lpi(lpi)		{Lpi = lpi;\
				 Vincr = Vdecr = (Vres * 10) / Lpi;}

				/* lpi_10 = (actual line density) x 10   */
				/* For example, if "6 lpi" is specified, */
				/* lpi_10 is 0x3c (6 lpi x 10)		 */  

#define SET_Pgwidth(chars)	{Pgwidth = chars;\
			 	 pgwidth = (chars * Hres * 10)/\
						(Condensed ? 180:(Pitch * 2));} 
	
#define SET_Pitch(cpi)		{Pitch = cpi;\
			 Hincr = (Hres * 10)/(Condensed ? 180:(Pitch*2));\
			 Hincr *= (Doublewide ? 2 : 1); }

				/* cpi_10 = (actual hankaku pitch) x 10 % 2  */
				/* For example, if "10 cpi" is specified,    */
				/* cpi_10 is 0x32 (10 lpi x 10 % 2)	     */ 

/******************************************************************************
* Lookup Tables                                                               *
*******************************************************************************/
struct lktable lkup_bool[] = {			/* BOOLEAN */
	"+"	,1	,
	"!"	,0  	,	
	NULL	,NULL
};

struct lktable lkup_lpi[] = {			/* LPI */
	"2"	,0x14	,
	"3"	,0x1e	,
	"4"	,0x28	,
	"5"	,0x32	,
	"6"	,0x3c	,
	"7.5"	,0x4b	,
	"8"	,0x50	,
	NULL	,NULL	
};

struct lktable lkup_cpi[] = {			/* PITCH (CPI) */
	"10"	,0x32	,
	"12"	,0x3c	,
	"13.4"	,0x43	,
	"15"	,0x4b	,
	NULL	,NULL
};

struct lktable lkup_fnt[] = {			/* FONT STYLE */
	"gothic"	,0x01	,
	"elite"		,0x06	,
	"courier"	,0x07	,
	"m12"		,0x08	,
#if !defined( IBM5587G ) && !defined( IBM5587H ) && !defined( IBM4216K )
	"m10"		,0x09 	,	  
#endif
#if defined( IBM5587G ) || defined( IBM5587H )
	"ocr"		,0x11	,
	"orator"	,0x12	,
#endif
	NULL	 	,NULL
};
#if defined( IBM5587G ) || defined( IBM5587H )
struct lktable lkup_src[] = {
	"1"		,1	,
	"2"		,2	,
# if defined( IBM5587H )
	"0"		,0xfd	,
	"t"		,0xfe	,
	"d"		,0xff	,
# endif
	NULL		,NULL
};
#endif
#if defined( IBM5587G ) || defined( IBM5587H ) || defined( IBM4216K )
struct lktable lkup_rot[] = {
	"0"		,0	,
# if defined( IBM5587H )
	"1"		,1	,
	"2"		,2	,
# endif
	"3"		,3	,
	NULL		,NULL
};
#endif
struct lktable lkup_crlf[] = {
	"0"		,0	,
	"1"		,1	,
	"2"		,2	,
	NULL		,NULL	
};
#if defined( IBM5577 ) || defined( IBM5579 )
struct lktable lkup_phd[] = {
	"1"		,1 	,
	"2"		,2	,
	"3" 		,3	,
	NULL 		,NULL
};
#endif
/*****************************************************************************
* Selction of piof header file						     *
******************************************************************************/
#if defined( IBM4208K ) || defined( IBM5572 ) || defined( IBM5575 )
#define DRIVER 5575
#include "piof5575k.h"				/* For Native Mode */
#endif

#if defined( IBM5577 ) || defined( IBM5579 )
#define DRIVER 5577
#include "piof5577d.h"
#endif

#if defined( IBM4216K ) || defined( IBM5587G )
#define DRIVER 5587
#include "piof5587d.h"
#endif 

#if defined( IBM5587H )
#define DRIVER 5587
#include "piof5587Hd.h"
#endif

#if defined( IBM5327 )
#define DRIVER 5327
#include "piof5327d.h"  
#endif


#if !defined( ESCSEQ_DEF )
# define ESCSEQ_DEF

typedef struct escseq_var {
	int	sz;
	char	*p;
	struct escseq_data	*ref;
	struct escseq_var	*next;
} ESCSEQ_VAR, *ESCSEQ_VARP;


typedef enum escdata_type0 {
	FIX_VALUE, FIX_SIZE, VAR_SIZE, VAR_SIZE_TIL, ASCII_ARGS, FUNC_LEAF
} escdata_type;

typedef struct	escseq_data {
	escdata_type	type;
	int	value;
	struct escseq_data	*next;
	struct escseq_data	*down;
#if defined( PREPRO )
	int	number;
	char	*name;
#endif	/* PREPRO */
} ESCSEQ_DATA, *ESCSEQ_DATAP;


typedef enum optoken_type0
    { OPERATOR, VALUE, ESCDATA } optoken_type;

typedef struct	op_token {
	optoken_type	type;
	int	value;
	struct op_token	*lop;
	struct op_token	*rop;
#if defined( PREPRO )
	int	number;
#endif /* PREPRO */
} OP_TOKEN, *OP_TOKENP;


extern void	set_codeset_byval( int val );
extern void	errorexit( int m, char *p );

extern int	kanji_in( void );
extern int	kanji_out( void );
extern int	get_a_mbcx( int *ri, char *bx, int maxbx );
extern ESCSEQ_DATAP 	ftbl_const( ESCSEQ_DATAP root, char *seq, int (*func)() );
extern int	tbl_search( ESCSEQ_DATAP tblp, int flag );
extern void	linefeed( void );
extern void	endline( void );
extern void	cmdout( char *attr );
extern void	pput1( int c );
extern void	pput2( int c );
extern void	pput4( int c );
extern void	pputn( int n, char *p );
extern void	pputs( char *p );
extern int	Eof_buffer( void );
extern int	Clr_buffer( void );
extern int	Setfp_buffer( FILE *fp );
extern int	Access_buffer( char *rp, int offset );
extern char	*Current_buffer_p( void );
extern int	Seek_buffer( int offset );
extern int	Seek_buffer_to_lockpoint( void );
extern int	Lock_buffer_at_cp( void );
extern int	Lock_buffer_at_bp( void );
extern int	Unlock_buffer( void );
extern int	Unread_buffer( void );
extern int	Read_buffer( void );
extern int	Ref_buffer( void );
extern int	Back_buffer( int c );
extern int	Copy_buffer( char *dest, char *src_b, int bsiz );


#define	EOL1	(-2)
#define	EOL2	(-3)
#define	EOL		(-4)
#define	NORMAL	(-5)

#endif	/* ESCSEQ_DEF */


/********************
Action function def.
********************/

int	nop()
{
/*	fprintf( stderr, "matched\n" );*/
	return( NORMAL );
}

/* commonly called by action functions */
static void	Data_through( int n )
{
	int	i, c;

	Unlock_buffer();

	for( i = 0; i < n; i++ ){
		c = Read_buffer();
		pput1( c );
	}
}

/* commonly called by action functions */
static void	Data_through_dum( int n )
{
	int	i;

	Unlock_buffer();

	for( i = 0; i < n; i++ ) Read_buffer();
}

/* commonly called by action functions */
static void	Data_after_esc_dum()
{
	Data_through_dum( R_st( 0 ) );
}


/* commonly called by action functions */
/* attr ch cl xx xx ... */
static void	Data_after_esc( char *attr )
{
	int	i, c, n = R_st( 0 );

	cmdout( attr );
	pput2( n );
	Data_through( n );
}


int	null_cntl()
{
	cmdout( NULL_CMD );
	return( NORMAL );
}

int	bel_cntl()
{
	cmdout( BEL_CMD );
	return( NORMAL );
}

int	bs_cntl()
{
	if( hpos == lmarg ) return( NORMAL );
	hpos -= Hincr;
	if( ( hpos < Pgwidth ) && trunc ) trunc--;
	cmdout( BS_CMD );
	return( NORMAL );
}

int	ht_cntl()
{
	int	i;
	int	tabval;
	char	*tabs;

	tabs = (&Horz_tabs)->ptr;
	for( i = 0; tabs[i] != 0; i++ ){
		tabval = ((int)tabs[i] - 1 ) * Hincr + lmarg;
		if( tabval > hpos ) break;
	}

	if( tabval > pgwidth ) tabval = pgwidth;
	cmdout( COLUMN_SKIP );
	pput1( 1 );
	pput1( (tabval - hpos ) / Hincr );
	while( hpos < tabval ) hpos += Hincr;
	return( NORMAL );
}

int	lf_cntl()
{
	if( Auto_crlf == 2 ){
		Hpos_from_prev = 0;
	}else{
		Hpos_from_prev = hpos;
	}

	linefeed();
	return( EOL );
}

int	vt_cntl()
{
	int	i;
	int	tabval;
	char	*tabs;


	tabs = (&Vert_tabs)->ptr;
	for( i = 0; tabs[i] != 0; i++ ){
		tabval = (tabs[i] - 1 ) * Vincr;
		if( tabval > vpos - vtab_base ) break;
	}

	if( tabs[i] != 0 ){
		vpos = tabval + vtab_base;
	}else{
		vpos += Vincr;
	}
	if( Auto_crlf != 2 ){
		Hpos_from_prev = hpos;
	}else{
		Hpos_from_prev = 0;
	}
	endline();
	return( EOL );
}

int	ff_cntl()
{
	vpos = pglen;
	endline();
	return( EOL );
}

int	cr_cntl()
{
	cmdout( CR_CMD );
	Hpos_from_prev = 0;
	if( Auto_crlf == 1 ){
		linefeed();
	}else{
		endline();
	}
	return( EOL );
}

int	fs_cntl()
{
	int	i;

	cmdout( IMG_HEIGHT_CMD );
	cmdout( IMG_TRSMIT_CMD );
	pput2( img_w );
	Data_through( img_h * img_w );
	i = img_h;
	img_h = 3;
	cmdout( IMG_HEIGHT_CMD );
	img_h = i;
	return( NORMAL );
}


/* this is called by image_dataset() and ex_image_dataset() */
static int	Img_data_set_sub( char *attr, int m )
{
	int	i, c;

	img_w = R_st( 0 );
	cmdout( IMG_HEIGHT_CMD );
	cmdout( attr );
	pput2( img_w );
	Data_through( img_w * img_h );
#if defined( IBM5587G ) || defined( IBM5587H )
	if( img_h == 2 )	m *= 2;
#endif
	hpos += m * img_w;
	i = img_h;
	img_h = 3;
	cmdout( IMG_HEIGHT_CMD );
	img_h = i;
	return( NORMAL );
}

int	image_data_set()
{
	return( Img_data_set_sub( IMG_TRSMIT_CMD, 8 ) );
}

int	ex_image_data_set()
{
	return( Img_data_set_sub( IMG_TRSENL_CMD, 16 ) );
}

int	fw_hskip()
{
	int	i;

	cmdout( FRD_HORSKP_CMD );
	pput2( i = R_st( 0 ) );
	hpos += i * 8;
	return( NORMAL );
}

int	bw_hskip()
{
	int	i;

	cmdout( BWD_HORSKP_CMD );
	pput2( i = R_st( 0 ) );

	if( hpos - i * 8 < lmarg ){
		hpos = lmarg;
	}else{
		hpos -= i * 8;
	}

	if( hpos < pgwidth && trunc ) trunc--;
	return( NORMAL );
}

int	fw_vskip()
{
	int	i = R_st( 0 );

	if( i >= 1 && i <= 0xff ){
		vpos += i * 12;
		Workint = 12;
		Hpos_from_prev = hpos;
		sharevars._vincr = &Workint;
		sharevars._vincr_cmd = (&Vincr_cmd2)->ptr;
		return( EOL2 );
	}else{
		return( NORMAL );
	}
}

int	cr_pset()
{
	cmdout( CR_CMD );
	cr_indent = R_st( 0 ) * 8;
	return( EOL2 );
}

int	bw_vskip()
{
#if !defined( IBM5327 )
	int	i = R_st( 0 );

	if( i >= 1 && i <= 0x28 ){
		vpos -= i * 12;
		if( vpos < 0 ) vpos = 0;
		Workint = 12;
		Hpos_from_prev = hpos;
		sharevars._vdecr = &Workint;
		sharevars._vdecr = (&Vdecr_cmd2)->ptr;
		return( EOL2 );
	}
#endif
	return( NORMAL );
}

int	def_lspacing()
{
	int	i = R_st( 0 );

	if( i >= 1 && i <= 0x3c ){
		cmdout( SET_LININC_CMD );
		pput2( i );
		Vincr = 12 * i;
	}

	return( NORMAL );
}

int	bidir_mode()
{
#if DRIVER != 5587 && DRIVER != 5327
	Unidirect = FALSE;
	cmdout( SET_DIR_CMD );
#endif
	return( NORMAL );
}

int	unidir_mode()
{
#if DRIVER != 5587 && DRIVER != 5327
	Unidirect = TRUE;
	cmdout( SET_DIR_CMD );
#endif
	return( NORMAL );
}

int	set3tr()
{
	img_h = 3;
	return( NORMAL );
}

int	set2tr()
{
	img_h = 2;
	return( NORMAL );
}

int	def_pglen()
{
	return( nop() );
}

int	hspeed_mode()
{
#if DRIVER != 5587
	Highspeed = TRUE;
	cmdout( SET_SPD_CMD );
#endif
	return( NORMAL );
}

int	lspeed_mode()
{
#if DRIVER != 5587
	Highspeed = FALSE;
	cmdout( SET_SPD_CMD );
#endif
	return( NORMAL );
}

int	cutform_in()
{
	vpos = pglen;
	return( EOL1 );
}

int	cutform_out()
{
	return( cutform_in() );
}

int	large_in()
{
	Doublewide = TRUE;
	Doublehigh = FALSE;
	SET_Pitch( Pitch );
	cmdout( SET_ENLG_CMD );
	return( NORMAL );
}

int	large_out()
{
	Doublewide = FALSE;
	Doublehigh = FALSE;
	SET_Pitch( Pitch );
	cmdout( SET_ENLG_CMD );
	return( NORMAL );
}

int	read_pr()
{
	return( nop() );
}

int	init_pr()
{
	return( nop() );
}

int	char_pitch()
{
	int	i = R_st( 0 );

	if( i == 0x32 || i == 0x3c || i == 0x43 || i == 0x4b ){
		set_Pitch( i );
		cmdout( CHG_CPI_CMD );
	}

	return( NORMAL );
}

int	line_pitch()
{
	int	i = R_st( 0 );

	if( i == 0x14 || i == 0x1e || i == 0x28 || i == 0x32 ||
	   i == 0x3c || i == 0x4b || i == 0x50 ){
		set_Lpi( i );
		cmdout( CHG_LPI_CMD );
	}

	return( NORMAL );
}

int	ch_pglen_6()
{
	return( nop() );
}

int	ch_pglen_l()
{
	return( nop() );
}

int	ch_pglen_i()
{
	return( nop() );
}

int	font_style()
{
	int	i = R_st( 0 );

	if( i == 0 || i == 1 || ( i >= 6 && i <= 8 )
#if !defined( IBM5587G ) && !defined( IBM5587H ) && !defined( IBM4216K )
	   || i == 9
#endif
#if defined( IBM5587G ) || defined( IBM5587H )
	   || i == 11 || i == 12
#endif
	){
		Typestyle = i;
		cmdout( CHG_FONT_CMD );
	}

	return( NORMAL );
}

int	tr_print()
{
	int	i, j, k, n = R_st( 0 );
	char	bx[NLCHARMAX+1];


	Unlock_buffer();

	for( k = 0; k < n; ){
		j = get_a_mbcx( &i, bx, NLCHARMAX );

		if( k + i >= n ){
			for( j = 0; j < k + i - n; j++ ) Unread_buffer();
			break;
		}else{
			k += i;
		}

		if( j <= 0 ){
			if( Eof_buffer() ){
				return( EOF );
			}else{
				continue;
			}
		}else{
			if( trunc ) continue;

			if( hspace( Hincr * mbwidth( bx ) ) ){
				if( j == 1 && bx[0] == 0 ) continue;
#if defined( X11R5BOS )
				if( is_FONT_P( Ccsid, Ccs ) ){
#endif /* X11R5BOS */
					cmdout( PRTALL_CMD );
					pput1( j );
					pputn( j, bx );
					hpos += Hincr;
#if defined( X11R5BOS )
				}
#endif /* X11R5BOS */
				continue;
			}else{
				if( Wrap ){
					Back_buffer( ( n - k ) & 0xff );
					Back_buffer( ( n - k ) >> 8 );
					Back_buffer( 8 );
					Back_buffer( 0x7e );
					Back_buffer( 0x1b );
					return( EOL1 );
				}else{
					trunc++;
				}
			}
		}
	}

	return( NORMAL );
}

int	ch_hmove()
{
	return( nop() );
}

int	ch_vmove()
{
	return( nop() );
}

int	condense_in()
{
	Condensed = TRUE;
	set_Pitch( Pitch );
	cmdout( SET_CND_CMD );
	return( NORMAL );
}

int	condense_out()
{
	Condensed = FALSE;
	set_Pitch( Pitch );
	cmdout( SET_CND_CMD );
	return( NORMAL );
}

int	vprint_in()
{
	Vertical = TRUE;
	cmdout( SET_VERT_CMD );
	return( NORMAL );
}

int	vprint_out()
{
	Vertical = FALSE;
	cmdout( SET_VERT_CMD );
	return( NORMAL );
}

int	superscr_in()
{
	Superscr_mode = TRUE;
	Subscr_mode = FALSE;
	cmdout( SET_SCRIPT_CMD );
	return( NORMAL );
}

int	subscr_in()
{
	Superscr_mode = FALSE;
	Subscr_mode = TRUE;
	cmdout( SET_SCRIPT_CMD );
	return( NORMAL );
}

int	script_out()
{
	Superscr_mode = FALSE;
	Subscr_mode = FALSE;
	cmdout( SET_SCRIPT_CMD );
	return( NORMAL );
}

int	errdetect_in()
{
	return( nop() );
}

int	errdetect_out()
{
	return( nop() );
}

int	halfline_up()
{
	Workint = Vincr / 2;
	vpos -= Workint;
	if( vpos < 0 ) vpos = 0;
	Hpos_from_prev = hpos;
	sharevars._vdecr = &Workint;
	sharevars._vdecr_cmd = (&Vdecr_cmd3)->ptr;
	return( EOL2 );
}

int	halfline_down()
{
	Workint = Vincr / 2;
	vpos += Workint;
	if( vpos > pglen ) vpos = pglen;
	Hpos_from_prev = hpos;
	sharevars._vincr = &Workint;
	sharevars._vincr_cmd = (&Vincr_cmd3)->ptr;
	return( EOL2 );
}

int	emphasize_in()
{
	Emphasized = TRUE;
	cmdout( SET_ENPH_CMD );
	return( NORMAL );
}

int	emphasize_out()
{
	Emphasized = FALSE;
	cmdout( SET_ENPH_CMD );
	return( NORMAL );
}

int	dstrike_in()
{
#if DRIVER != 5587 && DRIVER != 5327
	Doublestrike = TRUE;
	cmdout( SET_DBLSTR_CMD );
#endif
	return( NORMAL );
}

int	dstrike_out()
{
#if DRIVER != 5587 && DRIVER != 5327
	Doublestrike = FALSE;
	cmdout( SET_DBLSTR_CMD );
#endif
	return( NORMAL );
}

int	pr_complete()
{
	return( NORMAL );
}

int	ch_form_hdl()
{
	return( NORMAL );
}

int	underline_mode()
{
#if defined( IBM5587H )
	Cont_undrscore = R_st( 0 ) & 7;
#else
	Cont_undrscore = R_st( 0 ) & 3;
#endif
	cmdout( UNDERSCORE_CMD );
}

int	ch_datastream()
{
	return( nop() );
}

int	overstrike_cntl()
{
	int	n = R_st( 0 );
	int	osf = R_st( 1 );
	int	c = R_st( 3 );
	int	j, i;
	char	bx[NLCHARMAX+1];


	switch( n ){
	case 1:
		if( osf == 0 ){
			Ovrstrike_flag = n;
			Ovrstrike_mode = osf;
			Ovrstrike_code = 0;
			cmdout( OVERSTRIKE_CMD );
		}
		break;
	case 4:
	case 3:
		Seek_buffer( 2 - n );
		Ovrstrike_mode = osf;
/* here will come the code set conversion */
		if( ( j = get_a_mbcx( &i, bx, NLCHARMAX ) ) <= 0 ){
			if( Eof_buffer() ){
				return( EOF );
			}else{
				break;
			}
		}
		Ovrstrike_flag = 2 + j;
		Ovrstrike_code = *(long *)bx;
#if defined( X11R5BOS )
		if( is_FONT_P( Ccsid, Ccs ) ){
#endif /* X11R5BOS */
			cmdout( OVERSTRIKE_CMD );
			pputn( j, bx );
#if defined( X11R5BOS )
		}
#endif /* X11R5BOS */
		break;
	}

	return( NORMAL );
}

int	gridline()
{
#if DRIVER != 5575
	Data_after_esc( GRID_LINE_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	clr_htab()
{
	char	*tabs = (&Horz_tabs)->ptr;

	tabs[0] = 0;
	return( NORMAL );
}

int	default_htab()
{
	int	i;
	char	*tabs = (&Horz_tabs)->ptr;

	for( i = 0; i < Max_htabs; i++ ){
		tabs[i] = 8 * i + 1;
	}

	tabs[Max_htabs] = 0;
	return( NORMAL );
}

/* this is called by set_htab() and set_vtab() */
static int	Set_tabs( char *tabs, int maxn )
{
	int	i, j, c, c0, n = R_st( 0 );


	Unlock_buffer();

	for( i = 0, j = 0, c0 = 0; i < n; i++, c0 = c ){
		c = Read_buffer();
		if( j < maxn && c > c0 )	tabs[ j++ ] =  c;
	}

	tabs[j] = 0;
	return( NORMAL );
}

int	set_htab()
{
	char	*tabs = (&Horz_tabs)->ptr;

	return( Set_tabs( tabs, Max_htabs ) );
}

int	clr_vtab()
{
	char	*tabs = (&Vert_tabs)->ptr;

	tabs[0] = 0;
	return( NORMAL );
}

int	set_vtab()
{	
	char	*tabs = (&Vert_tabs)->ptr;

	return( Set_tabs( tabs, Max_vtabs ) );
}

int	set_hmargin()
{
	int	lm = R_st( 0 );
	int	rm = R_st( 1 );

	if( lm > 0 && rm >= lm + 4 ){
		lm--;
		set_Lmarg( lm );
		set_Pgwidth( rm );
	}else{
		return( NORMAL );
	}
}

int	set_skip_perforation()
{
	int	n = R_st( 0 );

	if( n > Pglen ){
		Tmarg = Bmarg = 0;
	}else if( n != 0 ){
		Tmarg = Perf_percent * n / 100;
		Bmarg = n - Tmarg;
	}

	return( NORMAL );
}

int	h_colskip0()
{
	cmdout( CR_CMD );
	Hpos_from_prev = lmarg + Hincr * R_st( 0 );
	if( Hpos_from_prev > pgwidth )	Hpos_from_prev = pgwidth;
	return( EOL2 );
}

int	h_colskip1()
{
	cmdout( CR_CMD );
	Hpos_from_prev = hpos + Hincr * R_st( 0 );
	if( Hpos_from_prev > pgwidth )	Hpos_from_prev = pgwidth;
	return( EOL2 );
}

int	h_colskip2()
{
	cmdout( CR_CMD );
	Hpos_from_prev = hpos - Hincr * R_st( 0 );
	if( Hpos_from_prev < lmarg ) Hpos_from_prev = lmarg;
	if( Hpos_from_prev < pgwidth && trunc ) trunc--;
	return( EOL2 );
}

int	v_lineskip0()
{
	return( nop() );
}

int	v_lineskip1()
{
	vpos += Vincr * R_st( 0 );
	if( pglen < vpos ) vpos = pglen;
	Hpos_from_prev = hpos;
	return( EOL2 );
}

int	v_lineskip2()
{
	return( nop() );
}

int	char_pitch_1440()
{
	return( nop() );
}

int	line_pitch_1440()
{
	return( nop() );
}

int	char_scaling()
{
	int	n1 = R_st( 0 );
	int	n2 = R_st( 1 );

	if(
#if DRIVER==5587
		( n1 == 8 || n1 == 0x10 ) && ( n2 == 8 || n2 == 0x10 )
#else
		( n1 == 8 && n2 == 8 ) || ( n1 == 0x10 && n2 == 0x10 )
#endif
	){
		Doublewide = Doublehigh = FALSE;
	}else if(
#if defined( IBM4216K )
		( n1 == 8 || n1 == 0x10 ) && ( n2 == 0x20 || n2 == 0x40 )
#else
# if defined( IBM5587H )
		( n1 == 8 || n1 == 0x10 ) &&
		( n2 == 0x14 || n2 == 0x18 || n2 == 0x20 )
# else
		n1 == 0x10 && n2 == 0x20
# endif
#endif
){
		Doublewide = FALSE;
		Doublehigh = TRUE;
	}else if(
#if defined( IBM4216K )
		( n2 == 8 || n2 == 0x10 ) && ( n1 == 0x20 || n1 == 0x40 )
#else
# if defined( IBM5587H )
		( n2 == 8 || n2 == 0x10 ) &&
		( n1 == 0x14 || n1 == 0x18 || n1 == 0x20 )
# else
		n1 == 0x20 && n2 == 0x10
# endif
#endif
	){
		Doublewide = TRUE;
		Doublehigh = FALSE;
	}else if(
#if defined( IBM5587G ) || defined( IBM4216K )
		( n1 == 0x20 || n1 == 0x40 ) && ( n2 == 0x20 || n2 == 0x40 )
#else
# if defined( IBM5587H )
		( n1==0x14 || n1==0x18 || n1==0x20 || n1==0x30 || n1==0x40 )&&
		( n2==0x14 || n2==0x18 || n2==0x20 || n2==0x30 || n2==0x40 )&&
		!( ( n1==0x14 || n1==0x18 ) && n2==0x40 ) &&
		!( n1==0x14 && n2==0x40 ) &&
		!( ( n2==0x14 || n2==0x18 ) && n1==0x40 ) &&
		!( n2==0x14 && n1==0x40 )
# else
		n1 == 0x20 && n2 == 0x20
# endif
#endif
	){
		Doublewide = Doublehigh = TRUE;
	}

	cmdout( CHG_SCALE_CMD );
	pput1( n1 );
	pput1( n2 );
	pput1( 2 );
	return( NORMAL );
}

int	gaij_load()
{
#if DRIVER != 5575
	Data_after_esc( GAIJI_LOAD_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	wr_img_cntl()
{
#if DRIVER == 5587
	Data_after_esc( WIMG_CNTL_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	wr_img()
{
#if DRIVER == 5587
	Data_after_esc( WRITE_IMG_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	load_font_cntl()
{
#if DRIVER == 5587
	cmdout( LDFNT_CNTL_CMD );
	pput1( R_st( 0 ) );
	pput2( R_st( 1 ) );
	pput2( R_st( 2 ) );
	pput1( R_st( 3 ) );
	pput1( R_st( 4 ) );
#endif
	return( NORMAL );
}

int	load_font_pattern()
{
#if DRIVER == 5587
	Data_after_esc( LDFNT_PAT_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	font_local()
{
#if DRIVER == 5587
	cmdout( FONT_LOCAL_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	del_local_font()
{
#if DRIVER == 5587
	Data_after_esc( DELLOC_FNT_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	char_rotation()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( CHAR_ROTAT_CMD );
	pput2( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	char_bline_offset()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( BASE_OFSET_CMD );
	pput2( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	text_rotation()
{
	int	i = R_st( 0 );
	int	b = R_st( 1 );

#if DRIVER == 5587
	if( i == 0x0000 && b == 0x2d00 ){
		Rotation = 0;
# if defined( IBM5587H )
	}else if( i == 0x2d00 && b == 0x5a00 ){
		Rotation = 1;
	}else if( i == 0x5a00 && b == 0x8700 ){
		Rotation = 2;
# endif
	}else if( i == 0x8700 && b == 0x0000 ){
		Rotation = 3;
	}

	cmdout( TEXT_ROTAT_CMD );
#endif
	return( NORMAL );
}

int	cp_number()
{
#if DRIVER == 5587
	cmdout( SET_COPY_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	logical_page()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( SET_LOGPG_CMD );
	pput2( R_st( 0 ) );
	pput2( R_st( 1 ) );
	pput2( R_st( 2 ) );
	pput2( R_st( 3 ) );
	pput1( 0 );
#endif
	return( NORMAL );
}

int	line_type()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( SET_LNTYPE_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	line_width()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( SET_LNWIDE_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

#if defined( IBM5587H )
int	draw_box_sub( char *attr )
{
	int	i, c, n = R_st( 0 );

	Escarglen = n;
	cmdout( attr );
	Data_through( n - 1 );
	return( NORMAL );
}
#endif

int	draw_box()
{
#if defined( IBM5587H )
	return( draw_box_sub( DRAW_BOX_CMD ) );
#else
	int	n = R_st( 0 );

# if defined( IBM5587G )
	cmdout( DRAW_BOX_CMD );
	pput2( n );
	pput1( 0xc1 );
# endif
# if defined( IBM5587G )
	Data_through( n - 1 );
# else
	Data_through_dum( n - 1 );
# endif
	return( NORMAL );
#endif
}

int	draw_box2()
{
#if defined( IBM5587H )
	return( draw_box_sub( DRAW_BOX2_CMD ) );
#else
	int	n = R_st( 0 );

	Data_through_dum( n - 1 );
	return( NORMAL );
#endif
}

int	draw_box3()
{
#if defined( IBM5587H )
	return( draw_box_sub( DRAW_BOX3_CMD ) );
#else
	int	n = R_st( 0 );

	Data_through_dum( n - 1 );
	return( NORMAL );
#endif
}

int	relative_line()
{
#if defined( IBM5587H )
	return( draw_box_sub( REL_LINE_CMD ) );
#else
	int	n = R_st( 0 );

	Data_through_dum( n - 1 );
	return( NORMAL );
#endif
}

int	paper_source()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	int	n = R_st( 0 );

	if( n == 0 ) Paper_src = 2;
	if( n == 1 ) Paper_src = 1;
# if defined( IBM5587H )
	if( n >= 0xfd && n <= 0xff ) Paper_src = n;
# endif
	cmdout( PAPER_SRC_CMD );
#endif
	return( NORMAL );
}

int	overlay_cntl()
{
#if defined( IBM5587G ) || defined( IBM5587H )
	cmdout( OVERLAY_CMD );
	pput2( R_st( 0 ) );
	pput2( R_st( 1 ) );
#endif
	return( NORMAL );
}

int	esc_segment()
{
#if DRIVER==5587
	cmdout( SEGMENT_CMD );
	pput2( R_st( 0 ) );
	pput2( R_st( 1 ) );
#endif
	return( NORMAL );
}

int	esc_pmp()
{
#if DRIVER==5587
	Data_after_esc( PMP_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	media_size()
{
#if defined( IBM5587H )
	cmdout( MEDIA_SIZE_CMD );
	pput1( R_st( 0 ) );
	pput2( R_st( 1 ) );
	pput2( R_st( 2 ) );
	pput2( R_st( 3 ) );
#endif
	return( NORMAL );
}

int	media_axis()
{
#if defined( IBM5587H )
	cmdout( MEDIA_AXIS_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	page_format()
{
#if defined( IBM5587H )
	cmdout( PAGE_FMT_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	page_descripter()
{
#if defined( IBM5587H )
	Data_after_esc( PAGE_DSCRPT_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	logical_zero()
{
#if defined( IBM5587H )
	cmdout( LOGICAL_ZERO_CMD );
	pput4( R_st( 0 ) );
	pput4( R_st( 1 ) );
#endif
	return( NORMAL );
}

int	composed_char()
{
#if defined( IBM5587H )
	int	n = R_st( 0 ), m = R_st( 1 );

	if( n - 1 == m ){
		cmdout( COMPOSE_C_CMD );

		switch( m ){
		case 1:
			pput1( m );
			pput1( R_st( 2 ) );
			break;
		case 2:
			pput1( m );
			pput2( R_st( 2 ) );
			break;
		default:
			pput1( 0 );
		}
	}
#endif
	return( NORMAL );
}

int	mesh_cntl()
{
#if defined( IBM5587H )
	Data_after_esc( MESH_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	mesh_reference()
{
#if defined( IBM5587H )
	cmdout( MESH_REF_CMD );
	pput1( R_st( 0 ) );
	pput4( R_st( 1 ) );
	pput4( R_st( 2 ) );
#endif
	return( NORMAL );
}

int	mesh_clear()
{
#if defined( IBM5587H )
	cmdout( MESH_CLR_CMD );
	pput1( R_st( 0 ) );
#endif
	return( NORMAL );
}

int	mesh_record()
{
	int	i, c, n = R_st( 0 );

#if defined( IBM5587H )
	cmdout( MESH_REC_CMD );
	pput2( n );
	pput1( R_st( 1 ) );
	pput2( R_st( 2 ) );
	pput1( R_st( 3 ) );
	pput1( R_st( 4 ) );
#endif
#if defined( IBM5587H )
	Data_through( n - 5 );
#else
	Data_through_dum( n - 5 );
#endif
	return( NORMAL );
}

int	graphics_cntl()
{
#if defined( IBM5587H )
	Data_after_esc( GRPH_CNTL_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

int	graphics_draw()
{
#if defined( IBM5587H )
	Data_after_esc( GRPH_DRAW_CMD );
#else
	Data_after_esc_dum();
#endif
	return( NORMAL );
}

extern ESCSEQ_DATA	Ttdata[];

static OP_TOKEN	Ttok[4] = {
	{ ESCDATA, (int)(Ttdata + 40), NULL, NULL },
	{ OPERATOR, (int)"+", Ttok + 2, Ttok + 3 },
	{ VALUE, -2, NULL, NULL },
	{ ESCDATA, (int)(Ttdata + 229), NULL, NULL }
};

ESCSEQ_DATA	Ttdata[407] = {
	{ FIX_VALUE, 0x1c, Ttdata + 2, Ttdata + 1 },
	{ FUNC_LEAF, fs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1b, Ttdata + 385, Ttdata + 3 },
	{ FIX_VALUE, '~', Ttdata + 337, Ttdata + 4 },
	{ FIX_VALUE, 0xa1, Ttdata + 7, Ttdata + 5 },
	{ FIX_SIZE, 2, Ttdata + 4, Ttdata + 6 },
	{ FUNC_LEAF, graphics_draw, NULL, NULL },
	{ FIX_VALUE, 0xa0, Ttdata + 10, Ttdata + 8 },
	{ FIX_SIZE, 2, Ttdata + 7, Ttdata + 9 },
	{ FUNC_LEAF, graphics_cntl, NULL, NULL },
	{ FIX_VALUE, 0x90, Ttdata + 13, Ttdata + 11 },
	{ FIX_SIZE, 2, Ttdata + 10, Ttdata + 12 },
	{ FUNC_LEAF, load_font_pattern, NULL, NULL },
	{ FIX_VALUE, 0x82, Ttdata + 20, Ttdata + 14 },
	{ FIX_SIZE, 2, Ttdata + 13, Ttdata + 15 },
	{ FIX_SIZE, 1, Ttdata + 14, Ttdata + 16 },
	{ FIX_SIZE, 2, Ttdata + 15, Ttdata + 17 },
	{ FIX_SIZE, 1, Ttdata + 16, Ttdata + 18 },
	{ FIX_SIZE, 1, Ttdata + 17, Ttdata + 19 },
	{ FUNC_LEAF, mesh_record, NULL, NULL },
	{ FIX_VALUE, 0x81, Ttdata + 23, Ttdata + 21 },
	{ FIX_SIZE, 2, Ttdata + 20, Ttdata + 22 },
	{ FUNC_LEAF, gaij_load, NULL, NULL },
	{ FIX_VALUE, '{', Ttdata + 28, Ttdata + 24 },
	{ FIX_VALUE, 0x0, Ttdata + 23, Ttdata + 25 },
	{ FIX_VALUE, 0x1, Ttdata + 24, Ttdata + 26 },
	{ FIX_SIZE, 1, Ttdata + 25, Ttdata + 27 },
	{ FUNC_LEAF, mesh_clear, NULL, NULL },
	{ FIX_VALUE, 'z', Ttdata + 35, Ttdata + 29 },
	{ FIX_VALUE, 0x0, Ttdata + 28, Ttdata + 30 },
	{ FIX_VALUE, 0x9, Ttdata + 29, Ttdata + 31 },
	{ FIX_SIZE, 1, Ttdata + 30, Ttdata + 32 },
	{ FIX_SIZE, 4, Ttdata + 31, Ttdata + 33 },
	{ FIX_SIZE, 4, Ttdata + 32, Ttdata + 34 },
	{ FUNC_LEAF, mesh_reference, NULL, NULL },
	{ FIX_VALUE, 'w', Ttdata + 38, Ttdata + 36 },
	{ FIX_SIZE, 2, Ttdata + 35, Ttdata + 37 },
	{ FUNC_LEAF, mesh_cntl, NULL, NULL },
	{ FIX_VALUE, 'u', Ttdata + 43, Ttdata + 39 },
	{ FIX_SIZE, 2, Ttdata + 38, Ttdata + 40 },
	{ FIX_SIZE, 1, Ttdata + 39, Ttdata + 41 },
	{ VAR_SIZE, (int)(Ttok + 0), Ttdata + 40, Ttdata + 42 },
	{ FUNC_LEAF, composed_char, NULL, NULL },
	{ FIX_VALUE, 'b', Ttdata + 47, Ttdata + 44 },
	{ FIX_SIZE, 2, Ttdata + 43, Ttdata + 45 },
	{ FIX_SIZE, 2, Ttdata + 44, Ttdata + 46 },
	{ FUNC_LEAF, overlay_cntl, NULL, NULL },
	{ FIX_VALUE, 'a', Ttdata + 51, Ttdata + 48 },
	{ FIX_SIZE, 2, Ttdata + 47, Ttdata + 49 },
	{ FIX_SIZE, 2, Ttdata + 48, Ttdata + 50 },
	{ FUNC_LEAF, esc_segment, NULL, NULL },
	{ FIX_VALUE, '[', Ttdata + 54, Ttdata + 52 },
	{ FIX_SIZE, 2, Ttdata + 51, Ttdata + 53 },
	{ FUNC_LEAF, del_local_font, NULL, NULL },
	{ FIX_VALUE, 'S', Ttdata + 62, Ttdata + 55 },
	{ FIX_VALUE, 0x0, Ttdata + 54, Ttdata + 56 },
	{ FIX_VALUE, 0xa, Ttdata + 55, Ttdata + 57 },
	{ FIX_SIZE, 4, Ttdata + 56, Ttdata + 58 },
	{ FIX_SIZE, 4, Ttdata + 57, Ttdata + 59 },
	{ FIX_VALUE, 0x0, Ttdata + 58, Ttdata + 60 },
	{ FIX_VALUE, 0x0, Ttdata + 59, Ttdata + 61 },
	{ FUNC_LEAF, logical_zero, NULL, NULL },
	{ FIX_VALUE, 'R', Ttdata + 65, Ttdata + 63 },
	{ FIX_SIZE, 2, Ttdata + 62, Ttdata + 64 },
	{ FUNC_LEAF, page_descripter, NULL, NULL },
	{ FIX_VALUE, 'Q', Ttdata + 70, Ttdata + 66 },
	{ FIX_VALUE, 0x0, Ttdata + 65, Ttdata + 67 },
	{ FIX_VALUE, 0x1, Ttdata + 66, Ttdata + 68 },
	{ FIX_SIZE, 1, Ttdata + 67, Ttdata + 69 },
	{ FUNC_LEAF, page_format, NULL, NULL },
	{ FIX_VALUE, 'P', Ttdata + 75, Ttdata + 71 },
	{ FIX_VALUE, 0x0, Ttdata + 70, Ttdata + 72 },
	{ FIX_VALUE, 0x1, Ttdata + 71, Ttdata + 73 },
	{ FIX_SIZE, 1, Ttdata + 72, Ttdata + 74 },
	{ FUNC_LEAF, media_axis, NULL, NULL },
	{ FIX_VALUE, 'F', Ttdata + 84, Ttdata + 76 },
	{ FIX_VALUE, 0x0, Ttdata + 75, Ttdata + 77 },
	{ FIX_VALUE, 0x5, Ttdata + 76, Ttdata + 78 },
	{ FIX_VALUE, 0x0, Ttdata + 77, Ttdata + 79 },
	{ FIX_VALUE, 0x0, Ttdata + 78, Ttdata + 80 },
	{ FIX_SIZE, 1, Ttdata + 79, Ttdata + 81 },
	{ FIX_VALUE, 0x0, Ttdata + 80, Ttdata + 82 },
	{ FIX_VALUE, 0x0, Ttdata + 81, Ttdata + 83 },
	{ FUNC_LEAF, paper_source, NULL, NULL },
	{ FIX_VALUE, 'C', Ttdata + 87, Ttdata + 85 },
	{ FIX_SIZE, 2, Ttdata + 84, Ttdata + 86 },
	{ FUNC_LEAF, esc_pmp, NULL, NULL },
	{ FIX_VALUE, '8', Ttdata + 96, Ttdata + 88 },
	{ FIX_VALUE, 0x0, Ttdata + 87, Ttdata + 89 },
	{ FIX_VALUE, 0x9, Ttdata + 88, Ttdata + 90 },
	{ FIX_SIZE, 2, Ttdata + 89, Ttdata + 91 },
	{ FIX_SIZE, 2, Ttdata + 90, Ttdata + 92 },
	{ FIX_SIZE, 2, Ttdata + 91, Ttdata + 93 },
	{ FIX_SIZE, 2, Ttdata + 92, Ttdata + 94 },
	{ FIX_VALUE, 0x0, Ttdata + 93, Ttdata + 95 },
	{ FUNC_LEAF, logical_page, NULL, NULL },
	{ FIX_VALUE, '7', Ttdata + 101, Ttdata + 97 },
	{ FIX_VALUE, 0x0, Ttdata + 96, Ttdata + 98 },
	{ FIX_VALUE, 0x1, Ttdata + 97, Ttdata + 99 },
	{ FIX_SIZE, 1, Ttdata + 98, Ttdata + 100 },
	{ FUNC_LEAF, font_local, NULL, NULL },
	{ FIX_VALUE, '6', Ttdata + 110, Ttdata + 102 },
	{ FIX_VALUE, 0x0, Ttdata + 101, Ttdata + 103 },
	{ FIX_VALUE, 0x7, Ttdata + 102, Ttdata + 104 },
	{ FIX_SIZE, 1, Ttdata + 103, Ttdata + 105 },
	{ FIX_SIZE, 2, Ttdata + 104, Ttdata + 106 },
	{ FIX_SIZE, 2, Ttdata + 105, Ttdata + 107 },
	{ FIX_SIZE, 1, Ttdata + 106, Ttdata + 108 },
	{ FIX_SIZE, 1, Ttdata + 107, Ttdata + 109 },
	{ FUNC_LEAF, load_font_cntl, NULL, NULL },
	{ FIX_VALUE, '3', Ttdata + 116, Ttdata + 111 },
	{ FIX_VALUE, 0x0, Ttdata + 110, Ttdata + 112 },
	{ FIX_VALUE, 0x2, Ttdata + 111, Ttdata + 113 },
	{ FIX_VALUE, 0x1, Ttdata + 112, Ttdata + 114 },
	{ FIX_SIZE, 1, Ttdata + 113, Ttdata + 115 },
	{ FUNC_LEAF, cp_number, NULL, NULL },
	{ FIX_VALUE, '2', Ttdata + 134, Ttdata + 117 },
	{ FIX_VALUE, 0x0, Ttdata + 125, Ttdata + 118 },
	{ FIX_VALUE, 0x2, Ttdata + 117, Ttdata + 119 },
	{ FIX_VALUE, 0x19, Ttdata + 122, Ttdata + 120 },
	{ FIX_SIZE, 1, Ttdata + 119, Ttdata + 121 },
	{ FUNC_LEAF, line_width, NULL, NULL },
	{ FIX_VALUE, 0x17, Ttdata + 118, Ttdata + 123 },
	{ FIX_SIZE, 1, Ttdata + 122, Ttdata + 124 },
	{ FUNC_LEAF, line_type, NULL, NULL },
	{ FIX_SIZE, 2, Ttdata + 116, Ttdata + 126 },
	{ FIX_VALUE, 0xe1, Ttdata + 128, Ttdata + 127 },
	{ FUNC_LEAF, relative_line, NULL, NULL },
	{ FIX_VALUE, 0xc1, Ttdata + 130, Ttdata + 129 },
	{ FUNC_LEAF, draw_box, NULL, NULL },
	{ FIX_VALUE, 0xc0, Ttdata + 132, Ttdata + 131 },
	{ FUNC_LEAF, draw_box3, NULL, NULL },
	{ FIX_VALUE, 0x80, Ttdata + 125, Ttdata + 133 },
	{ FUNC_LEAF, draw_box2, NULL, NULL },
	{ FIX_VALUE, '1', Ttdata + 140, Ttdata + 135 },
	{ FIX_VALUE, 0x0, Ttdata + 134, Ttdata + 136 },
	{ FIX_VALUE, 0x4, Ttdata + 135, Ttdata + 137 },
	{ FIX_SIZE, 2, Ttdata + 136, Ttdata + 138 },
	{ FIX_SIZE, 2, Ttdata + 137, Ttdata + 139 },
	{ FUNC_LEAF, text_rotation, NULL, NULL },
	{ FIX_VALUE, '/', Ttdata + 148, Ttdata + 141 },
	{ FIX_VALUE, 0x0, Ttdata + 140, Ttdata + 142 },
	{ FIX_VALUE, 0x7, Ttdata + 141, Ttdata + 143 },
	{ FIX_SIZE, 1, Ttdata + 142, Ttdata + 144 },
	{ FIX_SIZE, 2, Ttdata + 143, Ttdata + 145 },
	{ FIX_SIZE, 2, Ttdata + 144, Ttdata + 146 },
	{ FIX_SIZE, 2, Ttdata + 145, Ttdata + 147 },
	{ FUNC_LEAF, media_size, NULL, NULL },
	{ FIX_VALUE, ')', Ttdata + 151, Ttdata + 149 },
	{ FIX_SIZE, 2, Ttdata + 148, Ttdata + 150 },
	{ FUNC_LEAF, wr_img, NULL, NULL },
	{ FIX_VALUE, '(', Ttdata + 154, Ttdata + 152 },
	{ FIX_SIZE, 2, Ttdata + 151, Ttdata + 153 },
	{ FUNC_LEAF, wr_img_cntl, NULL, NULL },
	{ FIX_VALUE, '"', Ttdata + 159, Ttdata + 155 },
	{ FIX_VALUE, 0x0, Ttdata + 154, Ttdata + 156 },
	{ FIX_VALUE, 0x2, Ttdata + 155, Ttdata + 157 },
	{ FIX_SIZE, 2, Ttdata + 156, Ttdata + 158 },
	{ FUNC_LEAF, char_bline_offset, NULL, NULL },
	{ FIX_VALUE, '!', Ttdata + 164, Ttdata + 160 },
	{ FIX_VALUE, 0x0, Ttdata + 159, Ttdata + 161 },
	{ FIX_VALUE, 0x2, Ttdata + 160, Ttdata + 162 },
	{ FIX_SIZE, 2, Ttdata + 161, Ttdata + 163 },
	{ FUNC_LEAF, char_rotation, NULL, NULL },
	{ FIX_VALUE, ' ', Ttdata + 171, Ttdata + 165 },
	{ FIX_VALUE, 0x0, Ttdata + 164, Ttdata + 166 },
	{ FIX_VALUE, 0x3, Ttdata + 165, Ttdata + 167 },
	{ FIX_SIZE, 1, Ttdata + 166, Ttdata + 168 },
	{ FIX_SIZE, 1, Ttdata + 167, Ttdata + 169 },
	{ FIX_VALUE, 0x2, Ttdata + 168, Ttdata + 170 },
	{ FUNC_LEAF, char_scaling, NULL, NULL },
	{ FIX_VALUE, 0x1f, Ttdata + 173, Ttdata + 172 },
	{ FUNC_LEAF, line_pitch_1440, NULL, NULL },
	{ FIX_VALUE, 0x1e, Ttdata + 175, Ttdata + 174 },
	{ FUNC_LEAF, char_pitch_1440, NULL, NULL },
	{ FIX_VALUE, 0x1d, Ttdata + 187, Ttdata + 176 },
	{ FIX_VALUE, 0x0, Ttdata + 175, Ttdata + 177 },
	{ FIX_VALUE, 0x2, Ttdata + 176, Ttdata + 178 },
	{ FIX_VALUE, 0x2, Ttdata + 181, Ttdata + 179 },
	{ FIX_SIZE, 1, Ttdata + 178, Ttdata + 180 },
	{ FUNC_LEAF, v_lineskip2, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 184, Ttdata + 182 },
	{ FIX_SIZE, 1, Ttdata + 181, Ttdata + 183 },
	{ FUNC_LEAF, v_lineskip1, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 177, Ttdata + 185 },
	{ FIX_SIZE, 1, Ttdata + 184, Ttdata + 186 },
	{ FUNC_LEAF, v_lineskip0, NULL, NULL },
	{ FIX_VALUE, 0x1c, Ttdata + 199, Ttdata + 188 },
	{ FIX_VALUE, 0x0, Ttdata + 187, Ttdata + 189 },
	{ FIX_VALUE, 0x2, Ttdata + 188, Ttdata + 190 },
	{ FIX_VALUE, 0x2, Ttdata + 193, Ttdata + 191 },
	{ FIX_SIZE, 1, Ttdata + 190, Ttdata + 192 },
	{ FUNC_LEAF, h_colskip2, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 196, Ttdata + 194 },
	{ FIX_SIZE, 1, Ttdata + 193, Ttdata + 195 },
	{ FUNC_LEAF, h_colskip1, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 189, Ttdata + 197 },
	{ FIX_SIZE, 1, Ttdata + 196, Ttdata + 198 },
	{ FUNC_LEAF, h_colskip0, NULL, NULL },
	{ FIX_VALUE, 0x1b, Ttdata + 204, Ttdata + 200 },
	{ FIX_VALUE, 0x0, Ttdata + 199, Ttdata + 201 },
	{ FIX_VALUE, 0x1, Ttdata + 200, Ttdata + 202 },
	{ FIX_SIZE, 1, Ttdata + 201, Ttdata + 203 },
	{ FUNC_LEAF, set_skip_perforation, NULL, NULL },
	{ FIX_VALUE, 0x1a, Ttdata + 210, Ttdata + 205 },
	{ FIX_VALUE, 0x0, Ttdata + 204, Ttdata + 206 },
	{ FIX_VALUE, 0x2, Ttdata + 205, Ttdata + 207 },
	{ FIX_SIZE, 1, Ttdata + 206, Ttdata + 208 },
	{ FIX_SIZE, 1, Ttdata + 207, Ttdata + 209 },
	{ FUNC_LEAF, set_hmargin, NULL, NULL },
	{ FIX_VALUE, 0x19, Ttdata + 216, Ttdata + 211 },
	{ FIX_VALUE, 0x0, Ttdata + 214, Ttdata + 212 },
	{ FIX_VALUE, 0x0, Ttdata + 211, Ttdata + 213 },
	{ FUNC_LEAF, clr_vtab, NULL, NULL },
	{ FIX_SIZE, 2, Ttdata + 210, Ttdata + 215 },
	{ FUNC_LEAF, set_vtab, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 225, Ttdata + 217 },
	{ FIX_VALUE, 0x0, Ttdata + 223, Ttdata + 218 },
	{ FIX_VALUE, 0x1, Ttdata + 221, Ttdata + 219 },
	{ FIX_VALUE, 0x0, Ttdata + 218, Ttdata + 220 },
	{ FUNC_LEAF, default_htab, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 217, Ttdata + 222 },
	{ FUNC_LEAF, clr_htab, NULL, NULL },
	{ FIX_SIZE, 2, Ttdata + 216, Ttdata + 224 },
	{ FUNC_LEAF, set_htab, NULL, NULL },
	{ FIX_VALUE, 0x16, Ttdata + 228, Ttdata + 226 },
	{ FIX_SIZE, 2, Ttdata + 225, Ttdata + 227 },
	{ FUNC_LEAF, gridline, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 234, Ttdata + 229 },
	{ FIX_SIZE, 2, Ttdata + 228, Ttdata + 230 },
	{ FIX_SIZE, 1, Ttdata + 229, Ttdata + 231 },
	{ FIX_SIZE, 1, Ttdata + 230, Ttdata + 232 },
	{ VAR_SIZE, (int)(Ttok + 1), Ttdata + 231, Ttdata + 233 },
	{ FUNC_LEAF, overstrike_cntl, NULL, NULL },
	{ FIX_VALUE, 0x12, Ttdata + 237, Ttdata + 235 },
	{ FIX_SIZE, 3, Ttdata + 234, Ttdata + 236 },
	{ FUNC_LEAF, ch_datastream, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 242, Ttdata + 238 },
	{ FIX_VALUE, 0x0, Ttdata + 237, Ttdata + 239 },
	{ FIX_VALUE, 0x1, Ttdata + 238, Ttdata + 240 },
	{ FIX_SIZE, 1, Ttdata + 239, Ttdata + 241 },
	{ FUNC_LEAF, underline_mode, NULL, NULL },
	{ FIX_VALUE, 0x10, Ttdata + 245, Ttdata + 243 },
	{ FIX_SIZE, 3, Ttdata + 242, Ttdata + 244 },
	{ FUNC_LEAF, ch_form_hdl, NULL, NULL },
	{ FIX_VALUE, 0xe, Ttdata + 296, Ttdata + 246 },
	{ FIX_VALUE, 0x0, Ttdata + 245, Ttdata + 247 },
	{ FIX_VALUE, 0x1, Ttdata + 246, Ttdata + 248 },
	{ FIX_VALUE, '$', Ttdata + 250, Ttdata + 249 },
	{ FUNC_LEAF, pr_complete, NULL, NULL },
	{ FIX_VALUE, 0x1a, Ttdata + 252, Ttdata + 251 },
	{ FUNC_LEAF, dstrike_out, NULL, NULL },
	{ FIX_VALUE, 0x19, Ttdata + 254, Ttdata + 253 },
	{ FUNC_LEAF, dstrike_in, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 256, Ttdata + 255 },
	{ FUNC_LEAF, emphasize_out, NULL, NULL },
	{ FIX_VALUE, 0x17, Ttdata + 258, Ttdata + 257 },
	{ FUNC_LEAF, emphasize_in, NULL, NULL },
	{ FIX_VALUE, 0x16, Ttdata + 260, Ttdata + 259 },
	{ FUNC_LEAF, set2tr, NULL, NULL },
	{ FIX_VALUE, 0x15, Ttdata + 262, Ttdata + 261 },
	{ FUNC_LEAF, set3tr, NULL, NULL },
	{ FIX_VALUE, 0x14, Ttdata + 264, Ttdata + 263 },
	{ FUNC_LEAF, halfline_down, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 266, Ttdata + 265 },
	{ FUNC_LEAF, halfline_up, NULL, NULL },
	{ FIX_VALUE, 0x12, Ttdata + 268, Ttdata + 267 },
	{ FUNC_LEAF, errdetect_out, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 270, Ttdata + 269 },
	{ FUNC_LEAF, errdetect_in, NULL, NULL },
	{ FIX_VALUE, 0xf, Ttdata + 272, Ttdata + 271 },
	{ FUNC_LEAF, script_out, NULL, NULL },
	{ FIX_VALUE, 0xe, Ttdata + 274, Ttdata + 273 },
	{ FUNC_LEAF, subscr_in, NULL, NULL },
	{ FIX_VALUE, 0xd, Ttdata + 276, Ttdata + 275 },
	{ FUNC_LEAF, superscr_in, NULL, NULL },
	{ FIX_VALUE, 0xc, Ttdata + 278, Ttdata + 277 },
	{ FUNC_LEAF, vprint_out, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 280, Ttdata + 279 },
	{ FUNC_LEAF, vprint_in, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 282, Ttdata + 281 },
	{ FUNC_LEAF, large_out, NULL, NULL },
	{ FIX_VALUE, 0x9, Ttdata + 284, Ttdata + 283 },
	{ FUNC_LEAF, large_in, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 286, Ttdata + 285 },
	{ FUNC_LEAF, condense_out, NULL, NULL },
	{ FIX_VALUE, 0x7, Ttdata + 288, Ttdata + 287 },
	{ FUNC_LEAF, condense_in, NULL, NULL },
	{ FIX_VALUE, 0x6, Ttdata + 290, Ttdata + 289 },
	{ FUNC_LEAF, cutform_out, NULL, NULL },
	{ FIX_VALUE, 0x5, Ttdata + 292, Ttdata + 291 },
	{ FUNC_LEAF, cutform_in, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 294, Ttdata + 293 },
	{ FUNC_LEAF, lspeed_mode, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 247, Ttdata + 295 },
	{ FUNC_LEAF, hspeed_mode, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 299, Ttdata + 297 },
	{ FIX_SIZE, 4, Ttdata + 296, Ttdata + 298 },
	{ FUNC_LEAF, ch_vmove, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 302, Ttdata + 300 },
	{ FIX_SIZE, 4, Ttdata + 299, Ttdata + 301 },
	{ FUNC_LEAF, ch_hmove, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 305, Ttdata + 303 },
	{ FIX_SIZE, 2, Ttdata + 302, Ttdata + 304 },
	{ FUNC_LEAF, tr_print, NULL, NULL },
	{ FIX_VALUE, 0x6, Ttdata + 310, Ttdata + 306 },
	{ FIX_VALUE, 0x0, Ttdata + 305, Ttdata + 307 },
	{ FIX_VALUE, 0x1, Ttdata + 306, Ttdata + 308 },
	{ FIX_SIZE, 1, Ttdata + 307, Ttdata + 309 },
	{ FUNC_LEAF, font_style, NULL, NULL },
	{ FIX_VALUE, 0x4, Ttdata + 320, Ttdata + 311 },
	{ FIX_VALUE, 0x0, Ttdata + 310, Ttdata + 312 },
	{ FIX_VALUE, 0x3, Ttdata + 316, Ttdata + 313 },
	{ FIX_VALUE, 0x0, Ttdata + 312, Ttdata + 314 },
	{ FIX_SIZE, 2, Ttdata + 313, Ttdata + 315 },
	{ FUNC_LEAF, ch_pglen_6, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 311, Ttdata + 317 },
	{ FIX_VALUE, 0x1, Ttdata + 316, Ttdata + 318 },
	{ FIX_SIZE, 1, Ttdata + 317, Ttdata + 319 },
	{ FUNC_LEAF, ch_pglen_l, NULL, NULL },
	{ FIX_VALUE, 0x3, Ttdata + 325, Ttdata + 321 },
	{ FIX_VALUE, 0x0, Ttdata + 320, Ttdata + 322 },
	{ FIX_VALUE, 0x1, Ttdata + 321, Ttdata + 323 },
	{ FIX_SIZE, 1, Ttdata + 322, Ttdata + 324 },
	{ FUNC_LEAF, line_pitch, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 330, Ttdata + 326 },
	{ FIX_VALUE, 0x0, Ttdata + 325, Ttdata + 327 },
	{ FIX_VALUE, 0x1, Ttdata + 326, Ttdata + 328 },
	{ FIX_SIZE, 1, Ttdata + 327, Ttdata + 329 },
	{ FUNC_LEAF, char_pitch, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 334, Ttdata + 331 },
	{ FIX_VALUE, 0x0, Ttdata + 330, Ttdata + 332 },
	{ FIX_VALUE, 0x0, Ttdata + 331, Ttdata + 333 },
	{ FUNC_LEAF, init_pr, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 3, Ttdata + 335 },
	{ FIX_SIZE, 2, Ttdata + 334, Ttdata + 336 },
	{ FUNC_LEAF, read_pr, NULL, NULL },
	{ FIX_VALUE, ']', Ttdata + 339, Ttdata + 338 },
	{ FUNC_LEAF, large_out, NULL, NULL },
	{ FIX_VALUE, '[', Ttdata + 341, Ttdata + 340 },
	{ FUNC_LEAF, large_in, NULL, NULL },
	{ FIX_VALUE, 'V', Ttdata + 343, Ttdata + 342 },
	{ FUNC_LEAF, cutform_out, NULL, NULL },
	{ FIX_VALUE, 'S', Ttdata + 345, Ttdata + 344 },
	{ FUNC_LEAF, cutform_in, NULL, NULL },
	{ FIX_VALUE, 'P', Ttdata + 347, Ttdata + 346 },
	{ FUNC_LEAF, lspeed_mode, NULL, NULL },
	{ FIX_VALUE, 'O', Ttdata + 349, Ttdata + 348 },
	{ FUNC_LEAF, hspeed_mode, NULL, NULL },
	{ FIX_VALUE, 'F', Ttdata + 352, Ttdata + 350 },
	{ FIX_SIZE, 2, Ttdata + 349, Ttdata + 351 },
	{ FUNC_LEAF, def_pglen, NULL, NULL },
	{ FIX_VALUE, ')', Ttdata + 354, Ttdata + 353 },
	{ FUNC_LEAF, set2tr, NULL, NULL },
	{ FIX_VALUE, '(', Ttdata + 356, Ttdata + 355 },
	{ FUNC_LEAF, set3tr, NULL, NULL },
	{ FIX_VALUE, '%', Ttdata + 2, Ttdata + 357 },
	{ FIX_VALUE, 'U', Ttdata + 359, Ttdata + 358 },
	{ FUNC_LEAF, unidir_mode, NULL, NULL },
	{ FIX_VALUE, 'B', Ttdata + 361, Ttdata + 360 },
	{ FUNC_LEAF, bidir_mode, NULL, NULL },
	{ FIX_VALUE, '9', Ttdata + 364, Ttdata + 362 },
	{ FIX_SIZE, 2, Ttdata + 361, Ttdata + 363 },
	{ FUNC_LEAF, def_lspacing, NULL, NULL },
	{ FIX_VALUE, '8', Ttdata + 367, Ttdata + 365 },
	{ FIX_SIZE, 2, Ttdata + 364, Ttdata + 366 },
	{ FUNC_LEAF, bw_vskip, NULL, NULL },
	{ FIX_VALUE, '6', Ttdata + 370, Ttdata + 368 },
	{ FIX_SIZE, 2, Ttdata + 367, Ttdata + 369 },
	{ FUNC_LEAF, cr_pset, NULL, NULL },
	{ FIX_VALUE, '5', Ttdata + 373, Ttdata + 371 },
	{ FIX_SIZE, 2, Ttdata + 370, Ttdata + 372 },
	{ FUNC_LEAF, fw_vskip, NULL, NULL },
	{ FIX_VALUE, '4', Ttdata + 376, Ttdata + 374 },
	{ FIX_SIZE, 2, Ttdata + 373, Ttdata + 375 },
	{ FUNC_LEAF, bw_hskip, NULL, NULL },
	{ FIX_VALUE, '3', Ttdata + 379, Ttdata + 377 },
	{ FIX_SIZE, 2, Ttdata + 376, Ttdata + 378 },
	{ FUNC_LEAF, fw_hskip, NULL, NULL },
	{ FIX_VALUE, '2', Ttdata + 382, Ttdata + 380 },
	{ FIX_SIZE, 2, Ttdata + 379, Ttdata + 381 },
	{ FUNC_LEAF, ex_image_data_set, NULL, NULL },
	{ FIX_VALUE, '1', Ttdata + 356, Ttdata + 383 },
	{ FIX_SIZE, 2, Ttdata + 382, Ttdata + 384 },
	{ FUNC_LEAF, image_data_set, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 387, Ttdata + 386 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 389, Ttdata + 388 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 391, Ttdata + 390 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0xd, Ttdata + 393, Ttdata + 392 },
	{ FUNC_LEAF, cr_cntl, NULL, NULL },
	{ FIX_VALUE, 0xc, Ttdata + 395, Ttdata + 394 },
	{ FUNC_LEAF, ff_cntl, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 397, Ttdata + 396 },
	{ FUNC_LEAF, vt_cntl, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 399, Ttdata + 398 },
	{ FUNC_LEAF, lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x9, Ttdata + 401, Ttdata + 400 },
	{ FUNC_LEAF, ht_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 403, Ttdata + 402 },
	{ FUNC_LEAF, bs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x7, Ttdata + 405, Ttdata + 404 },
	{ FUNC_LEAF, bel_cntl, NULL, NULL },
	{ FIX_VALUE, 0x0, NULL, Ttdata + 406 },
	{ FUNC_LEAF, null_cntl, NULL, NULL }
};


#if defined( X11R5BOS )
ESCSEQ_DATAP	Kanjimode_data;

int	kanji_in()
{
	if( Ccs->state == ST_KO ){
		Ccs->state = ST_KI;
	}
}


int	kanji_out()
{
	if( Ccs->state == ST_KI ){
		Ccs->state = ST_KO;
	}
}
#endif /* X11R5BOS */


/* 
 * NAME: setup
 *                
 * FUNCTION: Performs setup processing for the print formatter
 * 
 * EXECUTION ENVIRONMENT: called by Formatter Driver
 *
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	Upon successful completion, the setup function should return either,
 *	
 *	(1) a pointer to a shar_vars structure that contains pointers to 
 *	    initialized vertical spacing variables that will be shared with
 *	    the formatter driver, who will provide vertical page movement, or
 *	   
 *	(2) a NULL pointer, whitch indicates that the formatter will handle
 *	    its own vertical page movement (i.e. top and bottom margins,
 *	    new pages, initial pages to be skipped, future top and bottom
 *	    security labels, and progress reports to qdaemon), or that the
 *	    input data stream is to be passed through without modification.  
 */

struct shar_vars *setup (argc, argv, passthru)
unsigned int argc;
char *argv[];
int passthru;
{
    struct stat buf;				/* Buffer for stat */
    void free (); 
    void errorexit ();
    extern int piogetvals ();
    extern int piogetopt ();
    extern void pioexit ();
#if defined( X11R5BOS )
    extern LOCinforec	LCI[];
    extern CODEinforec	CDI[];
    extern CSinforec	CSI[];
    extern FONTinforec	FI[];
#endif /* X11R5BOS */


    /* tell Formatter Driver about Data Base variables */
    if (piogetvals (attrtable, NULL) < 0)
        pioexit (PIOEXITBAD);

# if defined( AIX32 )
#  if defined( X11R5BOS )
    /* process command line flags */
    if (piogetopt (argc, argv, OPTSTRING, NULL) < 0)
        pioexit (PIOEXITBAD);

	if (User_font.len) {			/* User-defined font name */ 
		init_Fontlist( User_font.ptr, FI );
		fix_JPkana( CDI, FI );
		uuse++;
	}

        if( strcmp( Codepagename.ptr, "ISO8859-1" ) == 0 ){
                strcpy( Codepagename.ptr, "IBM-eucKR" );
        }

	lc_ch_cs_font( CDI, "IBM-932", 1, FONT_P );	/* kanji */
	lc_ch_cs_font( CDI, "IBM-932", 2, FONT_P );	/* kana */
	lc_ch_cs_font( CDI, "IBM-932", 3, FONT_P );	/* jp ibm selected */
	lc_p_code_change( CSI, "IBM-932" );	/* printer codeset is ibm-932*/
	if( ( Ccs = lc_init( LCI, CDI, CSI, Codepagename.ptr ) ) == 0 ){
		errorexit( MSG_BADCODEPAGE, "" );
	}

	Kanjimode_data = ftbl_const( Kanjimode_data, "\033(B", kanji_out );
	Kanjimode_data = ftbl_const( Kanjimode_data, "\033(J", kanji_out );
	Kanjimode_data = ftbl_const( Kanjimode_data, "\033$B", kanji_in );
	Kanjimode_data = ftbl_const( Kanjimode_data, "\033$@", kanji_in );
#if xx
show_FONTinfo( FI );
show_CSinfo( CSI );
show_CURcodeset( Ccs );
#endif
#  else /* X11R5BOS */
	if( !strcmp( Codepagename.ptr, "IBM-850" ) ||
	    !strcmp( Codepagename.ptr, "ISO8859-1" ) ){
		Codepagename.len = 0;
		Codepagename.ptr[0] = 0;
	}

	setlocale( LC_CTYPE, "Ja_JP" );
#  endif /* X11R5BOS */
# else
	setlocale( LC_CTYPE, "Jp_JP" );
# endif
#if !defined( X11R5BOS )
    /* process command line flags */
    if (piogetopt (argc, argv, OPTSTRING, NULL) < 0)
        pioexit (PIOEXITBAD);
#endif /* X11R5BOS */

    /* if passthru mode, that's all that needs to be done for setup() */
    if (passthru)
       return (NULL);

    /* initialize shadow variables */
    SET_Pglen (Pglen);
    SET_Tmarg (Tmarg);
    SET_Bmarg (Bmarg);
    SET_Lmarg (Indent);

    /* Validation */
    if (Emphasized & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "e");
    if (Beginpg <= 0)			errorexit(MSG_BADFORMFLAG2, "g");
    if (Init_printer & 0xfffffffe) 	errorexit(MSG_BADFORMFLAG1, "j");
    if (Pitch != 0x32 &&
	Pitch != 0x3c &&
	Pitch != 0x43 &&
	Pitch != 0x4b)			errorexit(MSG_BADPITCH, "p");
    if (Lpi != 0x14 &&
	Lpi != 0x1e &&
	Lpi != 0x28 &&
	Lpi != 0x32 &&
	Lpi != 0x3c &&
	Lpi != 0x4b &&
	Lpi != 0x50)			errorexit(MSG_BADLPI, "v");

    if (Typestyle != 0x01 
	&& Typestyle != 0x06 
	&& Typestyle != 0x07
	&& Typestyle != 0x08 
	&& Typestyle != 0x09 
#if DRIVER == 5587
	&& Typestyle != 0x11
	&& Typestyle != 0x12
#endif
       )
#if DRIVER != 5587
					errorexit(MSG_BADFONTSTYLE1, "s");
#endif
#if DRIVER == 5587
					errorexit(MSG_BADFONTSTYLE2, "s");
#endif
    if (Auto_crlf < 0 || Auto_crlf > 2)	errorexit(MSG_BADFORMFLAG7, "x"); 
#if DRIVER != 5587 && DRIVER != 5327 
    if (Doublestrike & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "y");
#endif
    if (Doublewide & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "W");
    if (Doublehigh & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "E");
    if (Restoreprinter & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "J");
    if (Condensed & 0xfffffffe)		errorexit(MSG_BADFORMFLAG1, "K");
    if (Wrap & 0xfffffffe)		errorexit(MSG_BADFORMFLAG1, "L");
#if DRIVER != 5587
    if (Highspeed & 0xfffffffe)		errorexit(MSG_BADFORMFLAG1, "S");
#endif
#if DRIVER != 5587 && DRIVER != 5327 
    if (Unidirect & 0xfffffffe)		errorexit(MSG_BADFORMFLAG1, "U"); 
#endif
    if (Vertical & 0xfffffffe)		errorexit(MSG_BADFORMFLAG1, "V");
    if (Do_formfeed & 0xfffffffe)	errorexit(MSG_BADFORMFLAG1, "Z");

#if DRIVER == 5577
    if (Paper_handle < 1 || Paper_handle > 3)
					errorexit(MSG_BADPAPERFLAG, "O");
#endif
#if defined( IBM5587G ) || defined( IBM5587H )
    if ( ( Paper_src < 1 || Paper_src > 2 )
# if defined( IBM5587H )
	&& ( Paper_src < 0xfd || Paper_src > 0xff )
# endif
    )
					errorexit(MSG_BADPAPERSRC,  "u");
#endif
#if DRIVER==5587
    if (Rotation != 0 && Rotation != 3
# if defined( IBM5587H )
	&& Rotation != 1 && Rotation != 2
# endif
    )
					errorexit(MSG_BADROTATION,  "z");
#endif
    if (Pgwidth < 1)			errorexit(MSG_BADFORMFLAG2, "w");
    if (Indent < 0)			errorexit(MSG_BADFORMFLAG3, "i");
    if (Indent >= Pgwidth)		errorexit(MSG_BADFORMFLAG8, "i");


    /* arrange for pages to be skipped (if any) */
    piopgskip = Beginpg - 1;

#if !defined( X11R5BOS )
    /* check user-defined font (if any) */
    u_font = User_font.ptr; 


    if (User_font.len) {			/* User-defined font name */ 
	init_Fontlist( u_font, "" );
	if (stat (u_font, &buf) < 0) {
	    errorexit(MSG_BADUSERFONT1, u_font);
	}
	if (!(buf.st_mode & 0x8000)) {
	    errorexit(MSG_BADUSERFONT2, u_font);
	}
	u_ptr = malloc ((unsigned int)(buf.st_size));
	if (!u_ptr) {
	    errorexit(MSG_BADUSERFONT3, u_font);
	}
	if ((u_des = open (u_font, O_RDONLY)) == -1) {
	    errorexit(MSG_BADUSERFONT4, u_font);
	}
	if (read (u_des, u_ptr, (unsigned int)(buf.st_size)) == -1) {
	    free (u_ptr);
	    errorexit(MSG_BADUSERFONT5, u_font);
	}
	if (init_snf (u_ptr) < 0) {
	    free (u_ptr);
	    errorexit(MSG_BADUSERFONT6, u_font);

	}
        uuse++;					/* User Font Available */
    } 						/* End of process for u_font */
#endif /* X11R5BOS */

    /* Initialize pointers to vertical spacing variables shared with  */      
    /* Formatter Driver (except for command names, all values pointed */
    /* to are in Vres unit)                                           */
    sharevars._pl	= &pglen;	/* page length */
    sharevars._tmarg	= &tmarg;	/* top margin */
    sharevars._bmarg	= &bmarg;	/* bottom margin */
    sharevars._vpos	= &vpos;	/* vertical position of next line */
    sharevars._vtab_base= &vtab_base;	/* base for vertical tab stop */
    sharevars._vincr	= &Vincr;	/* vertical increment of Vincr cmd */
    sharevars._vincr_cmd= (&Vincr_cmd)->ptr;/* name of vertical increment cmd*/
    sharevars._vdecr	= &Vdecr;	/* vertical decrement of Vdecr cmd */
    sharevars._vdecr_cmd= (&Vdecr_cmd)->ptr;/* name of vertical decrement cmd*/
    sharevars._ff_cmd  	= (&Ff_cmd)->ptr;/* name of form feed command */
    sharevars._set_cmd  = SWITCHMODE_CMD;
    sharevars._ff_at_eof = &Do_formfeed;
 	

    /* Return */
    return (&sharevars);
} 
                     
/* 
 * NAME: initialize
 * 
 * FUNCTION: Initializes the printer
 *
 * EXECUTION ENVIRONMENT: Called by Formatter Driver
 * 
 * (NOTE:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	Upon successful completion, the initialize function should return
 *	a value of zero.
 */
   

int initialize ()
{
    if (!Init_printer) return (0);
    cmdout (INIT_CMD);
    return (0);
}   

/* 
 * NAME: passthru
 *      
 * FUNCTION: Passes through the input data stream without modification
 *
 * EXECUTION ENVIRONMENT: Called by Formatter Driver
 * 
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	Upon successful completion, the passthru function should return a 
 *	value of zero, indicating that printing has completed successfully.
 */

int passthru ()
{
    int c;

    while ((c = piogetchar()) != EOF)
 	pioputchar (c);

    if (Do_formfeed)
	piocmdout (FF_CMD, NULL, 0, NULL);

    return (0);
} 
/* 
 * NAME: restore
 *          
 * FUNCTION: Restores the printer to its default state        
 *
 * EXECUTION ENVIRONMENT: Called by Formatter Driver
 * 
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 * 
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	Upon successful completion, the restore function should return a 
 *	value of zero.
 */

int restore ()
{
    /* output the command string to restore the printer */
    if (Restoreprinter)
	cmdout (REST_CMD);

    return (0);
}



#if !defined( X11R5BOS )
static iconv_t	Icd = NULL;
static char	*Processcodeset = "IBM-932";
static char	*Currenticonvcodeset = NULL;
#endif /* X11R5BOS */

static int	Convert_a_char( char *dest, char *src, int dn, int sn )
{
#if defined( X11R5BOS )
	char*	p;
	int	l;


	Ccsid = xcsid( src, Ccs );
	if( Ccsid < 0 )	return( -1 );
	p = get_iFont( src, Ccs, Ccsid );
	strncpy( dest, p, dn );
	l = strlen( dest );
	if( l == 0 ) l++;
	return( l );
#else /* X11R5BOS */
	char	*dest0 = dest, *src0 = src;
	int	dn0 = dn, sn0 = sn;
	int	rc;
	int	n;
	char	buff[256];


	if( Codepagename.len && strcmp( Processcodeset, Codepagename.ptr ) ){
		if( strcmp( Currenticonvcodeset, Codepagename.ptr ) ){
			if( Icd ){
				iconv_close( Icd );
				free( Currenticonvcodeset );
			}

			if( ( Icd = iconv_open( Processcodeset, Codepagename.ptr ) ) == NULL ){
				errorexit( MSG_BADCODEPAGE, "" );
			}
			n = strlen( Codepagename.ptr ) + 1;
			if( ( Currenticonvcodeset = calloc( sizeof( char ), n ) ) == NULL ){
				errorexit( MSG_MEMALLOCERR, "" );
			}
			strncpy( Currenticonvcodeset, Codepagename.ptr, n );
		}
		n = dn;
		rc = iconv( Icd, &src, &sn, &dest, &dn );
pputs( "" );/* dummy ( because of waiting or xlc bug ) */
		return( rc == ICONV_DONE ? n - dn : -1 );
	}else{
		if( src != NULL && dest != NULL && sn > 0 && dn > 0 ){
			n = mblen( src, NLCHARMAX );
			if( n == 0 ) n++;

			if( n > 0 && dn >= n ){
				strncpy( dest, src, n );
				dest[n] = 0;
			}

			return( n );
		}else{
			errorexit( MSG_FATAL, "" );
		}
	}
#endif /* X11R5BOS */
}


int	get_a_mbcx( int *ri, char *bx, int bxsize )
{
	int	j, i, c;
	char	b[NLCHARMAX+1];


	for( i = 0, bx[0] = 0; i < NLCHARMAX - 1; ){
		c = Read_buffer();

		if( c == EOF ){
			j = 0;
			break;
		}else{
			b[i] = c;
			i++;
			b[i] = 0;

			if( ( j = Convert_a_char( bx, b, bxsize, i ) ) > 0 ){
				bx[j] = 0;
				break;
			}else if( j < 0 ){
				continue;
			}else{
				/* error */
				errorexit( MSG_FATAL, "" );
				j = 0;
				break;
			}
		}
	}

	*ri = i;
	return( j );
}

#if defined( AIX32 )
int	mbwidth( char *p )
{
#if defined( X11R5BOS )
	return( get_wid( Ccsid, Ccs ) );
#else /* X11R5BOS */
	wchar_t	wc;
	int len;


	mbtowc( &wc, p, strlen( p ) );
	return( (((len = wcwidth( wc )) == -1) ? 1 : len) );
#endif /* X11R5BOS */
}
#else /* AIX32 */
int	mbwidth( char *p )
{
	int	i = strlen( p );
	return( i < 2 ? 1 : 2 );
}
#endif /* AIX32 */


lineout( FILE *fileptr )
{
	int	i, j, rc;
	char	wc[100];
	char	bx[NLCHARMAX+1];


	trunc = 0;

	sharevars._vincr = &Vincr;
	sharevars._vdecr = &Vdecr;
	sharevars._vincr_cmd = (&Vincr_cmd)->ptr;
	sharevars._vdecr_cmd = (&Vdecr_cmd)->ptr;

	if( hpos = Hpos_from_prev ){
		Hpos_from_prev = 0;
	}else{
		hpos = lmarg + cr_indent;
		cr_indent = 0;
	}

	if( Workint = hpos ) cmdout( HORZ_SPACE_CMD );
	if( Cont_undrscore ) cmdout( UNDERSCORE_CMD );
	if( Ovrstrike_flag == 3 || Ovrstrike_flag == 4 || Ovrstrike_flag == 5){
		cmdout( OVERSTRIKE_CMD );
		pputn( Ovrstrike_flag - 2, (char *)&Ovrstrike_code );
	}

	if( hpos > pgwidth ) trunc++;
	Setfp_buffer( fileptr );

	for( count = 0; !Eof_buffer(); ){
#if defined( X11R5BOS )
		if( Ccs->state != ST_NONE && tbl_search( Kanjimode_data, 1 ) ){
			continue;
		}
#endif /* X11R5BOS */
		switch( tbl_search( Ttdata, 1 ) ){
		case 0:
			/* 1 character conversion */
			j = get_a_mbcx( &i, bx, NLCHARMAX );

			if( j <= 0 ){ /* i >= NLCHARMAX or c == EOF */
/*				for( j = 0; j < i - 1; j++ ) Unread_buffer();
				if( i > 0 ) pput1( b[0] );*/
				return( count );
			}

			if( !trunc ){
				if( !hspace( Hincr * mbwidth( bx ) ) ){
					if( Wrap ){
						for( j = 0; j < i; j++ ) Unread_buffer();
						Hpos_from_prev = 0;
						linefeed();
						return( count );
					}else{
						trunc++;
					}
				}else{
#if defined( X11R5BOS )
					Fontimg	f = get_Font( bx, get_pFont( Ccs, Ccsid ), get_wid( Ccsid, Ccs ) );
					if( f == 0 ){
						errorexit( MSG_FATAL, "" );
					}else if( f->w == SEND_CODEVALUE ){
						pputn( j, bx );
					}else{
						fontout( f );
					}
#else /* X11R5BOS */
					if( uuse && j == 2 && mbwidth( bx ) ==2
					  && getfont( bx ) == 0 ){
						fontout();
					}else{
						pputn( j, bx );
					}
#endif /* X11R5BOS */
					hpos += Hincr * mbwidth( bx );
				}
			}
			break;
		case EOL1:
			Hpos_from_prev = 0;
			linefeed();
			return( count );
		case EOL2:
			endline();
		case EOL:
		case EOF:
			return( count );
		default:
			break;
		}
	}

	return( count );
}

#if defined( X11R5BOS )
int	fontout( Fontimg f )
{
	static struct pt2sz_tbl{
		int	pitch;
		int	ls[3];
		int	ls_at_lm[3];
		int	rs[3];
	} *p, pt2sz_table[5] = {
		{ 0x32, { 13, 9, 4 }, { 12, 6, 3 }, { 11, 3, 2 } },
		{ 0x3c, {  7, 4, 2 }, {  6, 3, 2 }, {  5, 2, 1 } },
		{ 0x43, {  4, 2, 1 }, {  3, 2, 1 }, {  2, 1, 0 } },
		{ 0x4b, {  0, 0, 0 }, {  0, 0, 0 }, {  0, 0, 0 } },
		{ 0,    {  0, 0, 0 }, {  0, 0, 0 }, {  0, 0, 0 } }
	};
	int	i = Doublewide ? 0 : 1;
	int	n;
	int	s = img_h;
	static	Fontimgrec	fo;

/*
	if( f->h != 24 || f->w != 24 ) return( 0 );
	c_image( f->img, imgb, 3, 3, f->bytes_in_line );
*/
	img_h = 3;
	cmdout( IMG_HEIGHT_CMD );

	fo.bytes_in_line = img_h;
	fo.w = 8 * img_h;
	fo.h = get_wid( Ccsid, Ccs ) * 12;
	if( fo.h == 12 ) i++;
	C_Fontimg( f, &fo );

	for( p = pt2sz_table; p->pitch != 0; p++ ){
		if( p->pitch == Pitch ){
			n = hpos == lmarg ? p->ls_at_lm[i] : p->ls[i];
			if( n > 0 ){
				cmdout( FRD_HORSKP_CMD );
				pput2( n );
			}
			cmdout( Doublewide ? IMG_TRSENL_CMD : IMG_TRSMIT_CMD );
			pput2( fo.h );
			pputn( fo.h * img_h, fo.img );
			if( p->rs[i] > 0 ){
				cmdout( FRD_HORSKP_CMD );
				pput2( p->rs[i] );
			}
		}
	}

	img_h = s;
}
#else /* X11R5BOS */
int	fontout()
{
	static struct pt2sz_tbl{
		int	pitch;
		int	ls[2];
		int	ls_at_lm[2];
		int	rs[2];
	} *p, pt2sz_table[5] = {
		{ 0x32, { 13, 9 }, { 12, 6 }, { 11, 3 } },
		{ 0x3c, {  7, 4 }, {  6, 3 }, {  5, 2 } },
		{ 0x43, {  4, 2 }, {  3, 2 }, {  2, 1 } },
		{ 0x4b, {  0, 0 }, {  0, 0 }, {  0, 0 } },
		{ 0,    {  0, 0 }, {  0, 0 }, {  0, 0 } }
	};
	int	i = Doublewide ? 0 : 1;
	int	n;
	int	s = img_h;

	img_h = 3;
	cmdout( IMG_HEIGHT_CMD );

	for( p = pt2sz_table; p->pitch != 0; p++ ){
		if( p->pitch == Pitch ){
			n = hpos == lmarg ? p->ls_at_lm[i] : p->ls[i];
			if( n > 0 ){
				cmdout( FRD_HORSKP_CMD );
				pput2( n );
			}
			cmdout( Doublewide ? IMG_TRSENL_CMD : IMG_TRSMIT_CMD );
			pput2( 24 );
			pputn( 72, uinfo.image );
			if( p->rs[i] > 0 ){
				cmdout( FRD_HORSKP_CMD );
				pput2( p->rs[i] );
			}
		}
	}

	img_h = s;
}
#endif /* X11R5BOS */

void	linefeed()
{
	vpos += Vincr * Linespacing;
	endline();
}

void	endline()
{
	int	s, s2, s3;

	if( Cont_undrscore ){
		s = Cont_undrscore;
		Cont_undrscore = FALSE;
		cmdout( UNDERSCORE_CMD );
		Cont_undrscore = s;
	}
	if( Ovrstrike_flag != 1 ){
		s = Ovrstrike_flag;
		s2= Ovrstrike_code;
		s3= Ovrstrike_mode;
		Ovrstrike_flag = 1;
		Ovrstrike_code = 0;
		Ovrstrike_mode = 0;
		cmdout( OVERSTRIKE_CMD );
		Ovrstrike_flag = s;
		Ovrstrike_code = s2;
		Ovrstrike_mode = s3;
	}
}

int set_Pitch( int cpi )
{
	Pitch = cpi;
	Hincr = (Hres * 10)/(Condensed ? 180:(Pitch*2));
	Hincr *= (Doublewide ? 2 : 1);
	/* cpi_10 = (actual hankaku pitch) x 10 % 2  */
	/* For example, if "10 cpi" is specified,    */
	/* cpi_10 is 0x32 (10 lpi x 10 % 2)	     */ 
}

int	set_Lpi( int lpi )
{
	Lpi = lpi;
	Vincr = Vdecr = (Vres * 10) / Lpi;
	/* lpi_10 = (actual line density) x 10   */
	/* For example, if "6 lpi" is specified, */
	/* lpi_10 is 0x3c (6 lpi x 10)		 */  
}

int	set_Pgwidth( int rm )
{
	Pgwidth = rm;
 	pgwidth = rm * Hres * 10 /	( Condensed ? 180 : Pitch * 2 );
}

int	set_Lmarg( int lm )
{
	Indent = lm;
	lmarg = lm * Hres * 10 / ( Condensed ? 180 : Pitch * 2 );
}

void	cmdout( char *attr )
{
	piocmdout( attr, NULL, 0, NULL );
}

void	pput1( int c )
{
	pioputchar( c );
}

void	pput2( int c )
{
	unsigned short	s;

	s = (unsigned short)c;
	pioputchar( s >> 8 );	/* high byte */
	pioputchar( s & 0xff );	/* low byte */
}

void	pput4( int c )
{
	char	*p = (char *)&c;

	pioputchar( p[0] );
	pioputchar( p[1] );
	pioputchar( p[2] );
	pioputchar( p[3] );
}

void	pputn( int n, char *p )
{
	int	i;

	for( i = 0; i < n; i++ ){
		pioputchar( p[i] );
	}
}

void	pputs( char *p )
{
	if( p ){
		pputn( strlen( p ), p );
	}
}


static int	Data_match( ESCSEQ_DATAP p );
static int	Elm_match( int c, ESCSEQ_DATAP p );
static int	Fixval_match( int c, int val );
static int	Tilsiz_match( int c, ESCSEQ_DATAP p );
static int	Fixsiz_match( int c, int siz, ESCSEQ_DATAP p );
static int	Varsiz_match( int c, OP_TOKENP p, ESCSEQ_DATAP q );
static int	Op_eval( OP_TOKENP p );
static int	Op_operate( char *op, int lval, int rval );

static int	Eof_buffer( void );
static int	Clr_buffer( void );
static int	Setfp_buffer( FILE *fp );
static int	Access_buffer( char *rp, int offset );
static char	*Current_buffer_p( void );
static int	Seek_buffer( int offset );
static int	Seek_buffer_to_lockpoint( void );
static int	Lock_buffer_at_cp( void );
static int	Lock_buffer_at_bp( void );
static int	Unlock_buffer( void );
static int	Unread_buffer( void );
static int	Read_buffer( void );
static int	Ref_buffer( void );
static int	Back_buffer( int c );
/*static int	Dump_buffer( void );*/

static int	Clr_stack( void );
static int	Add_stack( int siz, char *p, ESCSEQ_DATAP q );
static int	Rem_stack( ESCSEQ_DATAP p );
static int	Get_escdata( ESCSEQ_DATAP p );
static int	Get_sizdata( ESCSEQ_VARP p );


static ESCSEQ_VARP	Var_root = NULL;
static ESCSEQ_VARP	*Var_cur = &Var_root;

#if defined( X11R5BOS )
static ESCSEQ_DATAP	Esc_from_str( char *seq, int (*func)() )
{
	int	i, n;
	ESCSEQ_DATAP	q;


	n = strlen( seq ) + 1;

	if( ( q = calloc( sizeof( ESCSEQ_DATA ), n ) ) == NULL ){
		fprintf( stderr, "cant allocate memory\n" );
		exit( 2 );
	}

	for( i = 0; seq[i]; i++ ){
		q[i].type = FIX_VALUE;
		q[i].value = seq[i];
		q[i].down = q + i + 1;
		if( i > 0 )	q[i].next = q + i - 1;
	}

	q[i].type = FUNC_LEAF;
	q[i].value = (int)func;
	return( q );
}

static int	Elm_cmp( ESCSEQ_DATAP p, ESCSEQ_DATAP q )
{
	if( p == NULL || p->type == FUNC_LEAF ){
		return( q == NULL || q->type == FUNC_LEAF ? 0 : -1 );
	}else if( q == NULL || q->type == FUNC_LEAF ){
		return( 1 );
	}else{	/* FIX_VALUE */
		return( p->value - q->value );
	}
}

static int	Esc_match( ESCSEQ_DATAP p, ESCSEQ_DATAP seq )
{
	int	x;
	ESCSEQ_DATAP	*r;

	if( ( x = Elm_cmp( p, seq ) ) == 0 ){
		if( p == NULL || seq == NULL )	return( 0 );
/*		seq->next = p;*/
		for( r = &p->down; *r != p; r = &(*r)->next ){
			if( ( x = Esc_match( *r, seq->down ) ) <= 0 )	break;
		}

		if( x != 0 && seq->down != NULL ){
			seq->down->next = *r;
			*r = seq->down;
		}

		return( 0 );
	}else{
		return( x );
	}
}

ESCSEQ_DATAP	ftbl_const( ESCSEQ_DATAP root, char *seq, int (*func)() )
{
	ESCSEQ_DATAP	p, q, s;
	int	x;


	p = Esc_from_str( seq, func );

	if( root == NULL ){
		return( p );
	}else{
		for( q = root, s = NULL; q != NULL; s = q, q = q->next ){
			if( ( x = Esc_match( q, p ) ) <= 0 )	break;
		}

		if( x != 0 ){
			p->next = q;
			if( s != NULL ){
				s->next = p;
			}else{
				root = p;
			}
		}

		return( root );
	}
}
#endif /* X11R5BOS */

int	tbl_search( ESCSEQ_DATAP Esc_root, int flag )
{
	ESCSEQ_DATAP	p, q;
	int	i;
	int	(*f)();
	int	rc;


	Lock_buffer_at_cp();

	for( p = Esc_root; p != NULL; p = p->next ){
		if( f = (int (*)())Data_match( p ) ){
			/* matched */
			rc = (*f)();
			Unlock_buffer();
			Clr_stack();
			return( rc );
		}
	}

	/* not matched */
	Unlock_buffer();
	Clr_stack();
	return( 0 );
}



/* p->type never be FUNC_LEAF */
static int	Data_match( ESCSEQ_DATAP p )
{
	int	c = Read_buffer();


	if( Eof_buffer() ) return( 0 );

	if( Elm_match( c, p ) ){
		ESCSEQ_DATAP	q = p->down;

		if( q->type == FUNC_LEAF )	return( q->value );

		for( ; q != p; q = q->next ){
			int	rc;

			if( rc = Data_match( q ) )	return( rc );
		}
	}

	/* not matched */
	Unread_buffer();
	Rem_stack( p );
	return( 0 );
}


static int	Elm_match( int c, ESCSEQ_DATAP p )
{
	switch( p->type ){
	case FIX_VALUE:
		return( Fixval_match( c, p->value ) );
	case FIX_SIZE:
		return( Fixsiz_match( c, p->value, p ) );
	case VAR_SIZE:
		return( Varsiz_match( c, (OP_TOKEN *)p->value, p ) );
	case VAR_SIZE_TIL:
		return( Tilsiz_match( c, p ) );
	default:
		return( 0 );
	}
}


static int	Fixval_match( int c, int val )
{
	if( c == val ){
		return( 1 );
	}else{
		return( 0 );
	}
}


static int	Tilsiz_match( int c, ESCSEQ_DATAP p )
{
	int	i;
	char	*x;


	if( p->down == NULL || p->down->type != FIX_VALUE ) return( 0 );

	Seek_buffer( -1 );
	x = Current_buffer_p();
	Seek_buffer( 1 );

	for( i = 1; c != p->down->value; i++ ){
		c = Read_buffer();
		if( Eof_buffer() ){
			for( ; i > 1; i-- )	Unread_buffer();
			return( 0 );
		}
	}

	Unread_buffer();
	Add_stack( i, x, p );
	return( 1 );
}


static int	Fixsiz_match( int c, int siz, ESCSEQ_DATAP p )
{
	int	i;
	char	*x;


	Seek_buffer( -1 );
	x = Current_buffer_p();
	Seek_buffer( 1 );

	for( i = 1; i < siz; i++ ){
		c = Read_buffer();
		if( Eof_buffer() ){
			for( ; i > 1; i-- ) Unread_buffer();
			return( 0 );
		}
	}

	Add_stack( siz, x, p );
	return( 1 );
}


static int	Varsiz_match( int c, OP_TOKENP p, ESCSEQ_DATAP q )
{
	return( Fixsiz_match( c, Op_eval( p ), q ) );
}


static int	Op_eval( OP_TOKENP p )
{
	if( p == NULL ) return( 0 );

	switch( p->type ){
	case OPERATOR:
		return(
		 Op_operate( (char *)p->value, Op_eval(p->lop), Op_eval(p->rop) ) );
	case VALUE:
		return( p->value );
	case ESCDATA:
		return( Get_escdata( (ESCSEQ_DATA *)(p->value) ) );
	}
}


static int	Op_operate( char *op, int lval, int rval )
{
	if( !strcmp( op, "+" ) ){
		return( lval + rval );
	}else if( !strcmp( op, "-" ) ){
		return( lval - rval );
	}else if( !strcmp( op, "*" ) ){
		return( lval * rval );
	}else if( !strcmp( op, "/" ) ){
		return( lval / rval );
	}else if( !strcmp( op, "&" ) ){
		return( lval & rval );
	}else if( !strcmp( op, "|" ) ){	
		return( lval | rval );
	}else if( !strcmp( op, "^" ) ){
		return( lval ^ rval );
	}else{
		return( 0 );
	}
}


/*****************************************/
#define	IBUFF_SIZE	512

static char	Ibuffer[IBUFF_SIZE];
static char	*Ibbp = Ibuffer;
static char	*Ibcp = Ibuffer;
static char	*Iblp = NULL;
static FILE	*Ibfp = stdin;


/* no check for Ibbp, Iblp, Ibcp */
static int	Access_buffer( char *rp, int offset )
{
	if( rp >= Ibuffer && rp < Ibuffer + IBUFF_SIZE ){
		offset += ( rp - Ibuffer ) + IBUFF_SIZE;
		return( Ibuffer[ offset % IBUFF_SIZE ] );
	}else{
		return( EOF );
	}
}

static char	*Current_buffer_p()
{
	return( Ibcp );
}

/* check only Ibbp ( No Ibcp check ) */
static int	Advance_Ibbp()
{
	int	c;
	char	*p = Ibuffer + ( ( Ibbp - Ibuffer + 1 ) % IBUFF_SIZE );

	if( p != Iblp ){
		if( ( c = fgetc( Ibfp ) ) == EOF ){
			return( 0 );
		}
		*Ibbp = c;
		Ibbp = p;
		return( 1 );
	}else{
		errorexit( MSG_IBUFFULL, "" );
	}
}

static int	Seek_buffer( int offset )
{
	if( Iblp == NULL ){
		Ibcp = Ibuffer + ( ( Ibcp - Ibuffer + IBUFF_SIZE + offset ) % IBUFF_SIZE );
		return( 1 );
	}else{
		int	n = ( ( Ibcp - Iblp ) + IBUFF_SIZE ) % IBUFF_SIZE;

		if( n + offset >= 0 && n + offset < IBUFF_SIZE ){
			Ibcp = Iblp + n + offset;
			if( Ibcp >= Ibuffer + IBUFF_SIZE ) Ibcp -= IBUFF_SIZE;
			return( 1 );
		}else{
			return( 0 );
		}
	}
}

static int	Seek_buffer_to_lockpoint()
{
	if( Iblp != NULL ){
		Ibcp = Iblp;
		return( 1 );
	}else{
		return( 0 );
	}
}

static int	Clr_buffer()
{
	Ibcp = Ibbp = Ibuffer;
	Iblp = NULL;
	Ibfp = stdin;
	return( 1 );
}

static int	Setfp_buffer( FILE *fp )
{
	Ibfp = fp;
	return( 1 );
}

static int	Lock_buffer_at_cp()
{
	Iblp = Ibcp;
	return( 1 );
}

static int	Lock_buffer_at_bp()
{
	Iblp = Ibbp;
	return( 1 );
}

static int	Unlock_buffer()
{
	Iblp = NULL;
	return( 1 );
}

static int	Eof_buffer()
{
	return( Ibcp == Ibbp && feof( Ibfp ) );
}

static int	Read_buffer()
{
	char	*p;

	if( !Eof_buffer() ){
		if( Ibcp == Ibbp ){
			if( !Advance_Ibbp() ){
				return( EOF );
/*				errorexit( MSG_IBUFFULL, "" );*/
			}else{
				p = Ibcp;
				Ibcp = Ibbp;
				count++;
				return( *p );
			}
		}else{
			p = Ibcp;
			Seek_buffer( 1 );
			count++;
			return( *p );
		}
	}else{
		return( EOF );
	}
}

static int	Ref_buffer()
{
	if( !Eof_buffer() ){
		if( Ibcp == Ibbp && !Advance_Ibbp() ){
			return( EOF );
		}else{
			return( *Ibcp );
		}
	}else{
		return( EOF );
	}
}

static int	Unread_buffer()
{
	int	n = ( Ibbp - Ibcp + IBUFF_SIZE ) % IBUFF_SIZE;

	if( n + 1 < IBUFF_SIZE && Seek_buffer( -1 ) ){
		count--;
		return( 1 );
	}else{
		errorexit( MSG_IBUFFATAL, "" );
		return( 0 );
	}
}

static int	Back_buffer( int c )
{
	if( Unread_buffer() ){
		*Ibcp = c;
		return( 1 );
	}else{
		return( 0 );
	}
}

/**********************************/

static int	Clr_stack()
{
	ESCSEQ_VARP	p, q;


	for( p = Var_root; p != NULL; ){
		q = p->next;
		free( p );
		p = q;
	}

	Var_root = NULL;
	Var_cur = &Var_root;
	return( 1 );
}


static int	Add_stack( int siz, char *p, ESCSEQ_DATAP q )
{
	if( ( *Var_cur = calloc( sizeof( ESCSEQ_VAR ), 1 ) ) == NULL ){
		errorexit( MSG_MEMALLOCERR, "" );
	}

	(*Var_cur)->sz  = siz;
	(*Var_cur)->p   = p;
	(*Var_cur)->ref = q;
	Var_cur = &((*Var_cur)->next);
	return( 1 );
}


static int	R_st( int i )
{
ESCSEQ_VARP	p;
	char	buff[6];

	for( p = Var_root; i > 0 && p != NULL; i-- )	p = p->next;

	if( i < 0 || p == NULL )	return( 0 );

	return( Get_sizdata( p ) );
}


static int	Rem_stack( ESCSEQ_DATAP p )
{
	ESCSEQ_VARP	x, xold;


	if( p != NULL && Var_root != NULL ){
		switch( p->type ){
			case	FIX_SIZE:
			case	VAR_SIZE:
			case	VAR_SIZE_TIL:
				for( x = Var_root; x->next != NULL; xold = x, x = x->next );
				if( x->ref == p && &x->next == Var_cur ){
					free( x );
					if( x == Var_root ){
						Var_root = NULL;
						Var_cur = &Var_root;
					}else{
						xold->next = NULL;
						Var_cur = &(xold->next);
					}
				}else{
					errorexit( MSG_FATAL, "" );
				}
		}
	}
}


static int	Get_escdata( ESCSEQ_DATAP p )
{
	ESCSEQ_VARP	x;


	switch( p->type ){
	case FIX_VALUE:
		return( p->value );
	case VAR_SIZE:
	case FIX_SIZE:
		for( x = Var_root; x != NULL; x = x->next ){
			if( x->ref == p ) break;
		}

		return( Get_sizdata( x ) );
	}

	return( 0 );
}


static int	Get_sizdata( ESCSEQ_VARP x )
{
	char buff[5];
	int	i, rc;

	if( x == NULL ) return( 0 );

	switch( x->sz ){
	case 0:
		return( 0 );
	case 1:
		if( ( rc = Access_buffer( x->p, 0 ) ) == EOF ){
			errorexit( MSG_FATAL, "" );
		}else{
			return( rc );
		}
	case 2:
	case 3:
		for( i = 0; i < 2; i++ ){
			if( ( rc = Access_buffer( x->p, i ) ) != EOF ){
				buff[i] = rc;
			}else{
				errorexit( MSG_FATAL, "" );
			}
		}
		buff[2] = 0;
		return( *(short *)buff );
    defalut:
		for( i = 0; i < 4; i++ ){
			if( ( rc = Access_buffer( x->p, i ) ) != EOF ){
				buff[i] = rc;
			}else{
				errorexit( MSG_FATAL, "" );
			}
		}
		buff[4] = 0;
		return( *(long *)buff );
	}
	return( 0 );
}




/* 
 * NAME: hspace
 *
 * FUNCTION: Checks if there's a space for character output   
 * 
 * EXECUTION ENVIRONMENT: Called by Formatter Driver
 * 
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:      
 *	Upon successful completion, the hspace should return a value of 1,  
 *	indicating that there's enough space in the line for the character,
 *	or 0 indicating that there's not enough space in the line for the
 *	character.
 */
int hspace (cw)
int cw;						/* character width */
{ 
    if ((pgwidth - hpos) >= cw) return (1);	/* Space */
    return (0);					/* No space */
}

#if defined( AIX32 )
# include	"defmsg.c"
#else /* AIX32 */
/* 
 * NAME: errorexit
 * 
 * FUNCTION: displays error messages 
 * 
 * EXECUTION ENVIRONMENT: Called by setup ()
 *
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	NONE 
 */
#define PIOMSGCAT "kfmtrs.cat"
#define PIOMSGSET 1

void errorexit (msgnum, stringval)
int msgnum;
char *stringval;
{
    char defmsg[500];
    char msgbuf[1000];

    (void) sprintf (defmsg, PIODEFMSG, msgnum, PIOMSGCAT);
    (void) sprintf ( msgbuf, 
		     NLgetamsg (PIOMSGCAT, PIOMSGSET, msgnum, defmsg),
		     stringval
		   );
    (void) piomsgout (msgbuf);
    if (piodatasent && Do_formfeed)
	piocmdout (FF_CMD, NULL, 0, NULL);
    pioexit (PIOEXITBAD);
}
#endif /* AIX32 */
#if !defined( X11R5BOS )
/*
 * NAME: init_snf                                                           
 *                                                                         
 * FUNCTION: Check the font file, and initialize the snffont structure        
 *                                                                           
 * EXECUTION ENVIRONMENT: Called from DBCS backend filter                    
 *                                                                           
 * (NOTE:) pointer to the snf font file to be given                          
 *                                                                            
 * (RECOVERY OPERATION:)
 *
 * RETURNS:   0 when the file is correct snf file.                             
 *           -1 when the file is not correct snf file.                         
 */
int init_snf(udf_ptr)
char *udf_ptr;
{
	FontPropPtr  ptr;
	char         *name, *value;
	int          rc1, rc2, i;

/* check version1 to see if this is an snf file of correct version */
	if (*(unsigned int *)u_ptr != FONT_FILE_VERSION) return(-1);

/* initialize and set the snffont structure */
	snffont.pFI = (FontInfoPtr)udf_ptr;
	snffont.pCI = (CharInfoPtr)(ADDRCharInfoRec(snffont.pFI));
	snffont.pGlyphs = (char *)(ADDRCHARGLYPHS(snffont.pFI));
	snffont.pFP = (FontPropPtr)(ADDRXFONTPROPS(snffont.pFI));
	snffont.strings = (char *)(ADDRSTRINGTAB(snffont.pFI));

/* double check if this is an snf file of correct version */
	if (snffont.pFI->version2 != FONT_FILE_VERSION) return(-1);

/* check the keyword to see whether it is the UDF file */
	for(i=0, ptr=snffont.pFP; i<snffont.pFI->nProps; i++, ptr++) {
	   name = snffont.strings + ptr->name;
	   value = snffont.strings + ptr->value;
	   rc1 = strncmp(KYWDTITLE, name,  strlen(name ));
	   rc2 = strncmp(KYWDVALUE, value, strlen(value)) *
	         strncmp(KYWDVALUE2, value, strlen(value));
	   if ( !rc1 && !rc2 ) return(0); /* O.K. */
	}
	return(-1);
}


/*
 * NAME: getfont                                                               
 *                                                                             
 * FUNCTION: Gets font image data from snf file, and change it to printable    
 *           data.                                                             
 *           SJIS to JIS conversion is also done in this routine.              
 *                                                                             
 * EXECUTION ENVIRONMENT: Called from DBCS backend filter                      
 *                                                                             
 * (NOTE:) SJIS code must be set in the UdfInfoRec structure when called   
 *                                                                             
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  0 when the image data is set to the UdfInfoRec structure correctly
 *          -1 when the char specified does not exits or incorrect.            
 */

#define	IS_UDC(hi)	(0x7f <= (hi) && (hi) < 0x7f + 20)

int getfont( char *sjisbuff )
{
	CharInfoRec   cinfo;
	unsigned char *glyph;
	unsigned int  code_row, code_col, code_npr;
	int           bwidth, bheight;
	static char   image_buf[256];
	unsigned int  pc2jis();
	void          rmpad(), c_image();

	uinfo.code = pc2jis( (unsigned)*(unsigned short *)sjisbuff );
	if (uinfo.code == 0) return (-1);
	if (!IS_UDC(uinfo.code >> 8))
		return -1;
	uinfo.code -= 0x1a00;

/* check if the char for the JIS code exists */
	code_row = (uinfo.code >> 8) & 0xff;
	code_col = uinfo.code & 0xff;
	if ((code_row < snffont.pFI->firstRow) ||
	    (code_row > snffont.pFI->lastRow ) ||
	    (code_col < snffont.pFI->firstCol) ||
	    (code_col > snffont.pFI->lastCol )) return(-1);

/* get the CharInfoRec structure for the JIS code */
	code_row -= snffont.pFI->firstRow;
	code_col -= snffont.pFI->firstCol;
	code_npr  = snffont.pFI->lastCol - snffont.pFI->firstCol + 1;
	cinfo = *(snffont.pCI + code_col + code_row * code_npr);
	if (!cinfo.exists) return(-1);

/* get character width */
	uinfo.width  = cinfo.metrics.rightSideBearing -
		       cinfo.metrics.leftSideBearing - 2;
	     /* there is one blank col at both side of the char box. */
	if (uinfo.width <= 0) return(-1);

/* get character height */
	uinfo.height = cinfo.metrics.ascent +
		       cinfo.metrics.descent - 3;
	     /* there are two blank lines at the bottom, one at the  */
	     /* top of the char box.                                 */
	if (uinfo.height <= 0) return(-1);

/* get the length of the image data */
	bwidth = (uinfo.width+7) >> 3;        /* char width in byte  */
	bheight = (uinfo.height+7) >> 3;      /* char height in byte */
	uinfo.bytes = bwidth * (uinfo.height);
	if (uinfo.bytes <= 0) return(-1);
	/* uinfo.width = 24                                            */
	/* uinfo.height = 24                                           */
	/* uinfo.byte = 72                                             */
	/* bwidth = 3                                                  */
	/* bheight =3          ,when 24x24 font is used                */
	if ((uinfo.width != 24) || (uinfo.height != 24)) return(-1);

/* get the addr of the glyph data */
	glyph = snffont.pGlyphs + cinfo.byteOffset;

	rmpad(glyph, bwidth, image_buf);
	c_image(image_buf, uinfo.image, bwidth, bheight);

	return(0);

}  /* end of getfont() */

/*
 * NAME: pctojis.c
 *
 * FUNCTION: Converts PC Code (IBM-932) code to jis code.
 *
 * EXECUTION ENVIRONMENT: Called from getfont().
 *
 * (NOTE:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: jis code
 *
 */

/*
 *	borrow from iconv.
 *	expected to be replaced by iconv.
 */

static unsigned short	CP932toSJIS[][2] = {
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

unsigned int	pc2jis(pc)
unsigned int	pc;
{
	unsigned int	hi,low;
	int	ulim, llim, cur;	/* upper/lower limit for b-search */

	/*
	 *	convert 932 to JISX0208-1983 order.
	 */
	llim = 0;
	ulim = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
	while (llim <= ulim) {
		cur = llim + ulim >> 1;
		if (pc < CP932toSJIS[cur][0])
			ulim = cur - 1;
		else if (pc > CP932toSJIS[cur][0])
			llim = cur + 1;
		else {
			pc = CP932toSJIS[cur][1];
			break;
		}
	}

	hi = (pc >> 8) & 0xff;
	low = pc & 0xff;

	/*
	 *	validity check for the 2nd byte.
	 */
	if (low < 0x40 || low == 0x7f || 0xfd <= low && low <= 0xff)
		return 0;

	/*
	 *	convert to JISX0208(+)
	 */
	hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
	hi = hi * 2 + 1;

	if (low > 0x7f)
		low--;

	if (low >= 0x9e) {
		low -= 0x7d;
		hi++;
	}
	else
		low -= 0x1f;

	return (hi << 8) | low;
}
/*
 * NAME: rmpad                                                             
 *                                                                       
 * FUNCTION: Remove unnecessary padded 0's surrounding the font   
 *                                                               
 * EXECUTION ENVIRONMENT: Called from getfont().               
 *                                                          
 * (NOTE:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: NONE                                        
 *
 */
void rmpad(glyph, bwidth, image_buf)
char *glyph;
int  bwidth;
char image_buf[];
{
	int i, j, k, bpr, index;
	int from, to;
	register char image;

	bpr = GLWIDTHBYTESPADDED(uinfo.width+2, DEFAULTGLPAD);
	/* bytes per row of snf glyph data                              */
	/* assume default padding(=1) was used when snftobdf convering. */
	/* bpr = 4 when 24x24 is used

/* remove one line at the top, two lines at the bottom of the snf glyph */
/* data, and store the new data into image_buf[].                       */
	from = bpr;
	to   = uinfo.height*bpr+bpr-1;  /* maybe from 4 to 99, when 24x24 */
	i = 0;
	for (j=from; j<=to ; j++) {
	   image_buf[i] = glyph[j];
	   i++;
	}

/* remove left edge colum, and shift left 1 bit */
	i = 0;
	if (!(uinfo.width%8)) {
	   for (j=0; j<uinfo.height; j++) {
	      for (k=0; k<bwidth; k++) {
		 index = k + j * bpr;
		 image =  image_buf[index];
		 image <<= 1;
		 if (image_buf[index+1] & 0x80) image |= 0x01;
		 image_buf[i] = image;
		 i++;
	      }
	   }
	}
	else {      /* rarely comes here */
	   for (j=0; j<uinfo.height; j++) {
	      for (k=0; k<bwidth-1; k++) {
		 index = k + j * bpr;
		 image =  image_buf[index];
		 image <<= 1;
		 if (image_buf[index+1] & 0x80) image |= 0x01;
		 image_buf[i] = image;
		 i++;
	      }
	      image_buf[i] = image_buf[index+1];
	   }
	}
}  /* end of rmpad() */

/*
 * NAME: c_image
 *                                                                             
 * FUNCTION: Convert the snf glyph data to printable data                      
 *                                                                             
 * EXECUTION ENVIRONMENT: Called from getfont().                               
 *                                                                             
 * (NOTE:) indata must be pure font data, i.e. data w/o any additional          
 *         surrounding bits.                                                    
 *
 * (RECOVERY OPERATION:)
 * 
 * (DATA STRUCTURES:)
 *                                                                             
 * RETURNS: NONE                                                               
 *                                                                             
 */
void c_image(indata, outdata, bwidth, bheight)
char indata[], outdata[];
int  bwidth, bheight;
{
	int i, j, m, n, indexin, indexout;
	register char in, out;

	for (m=0; m<bheight; m++) {
	   for (n=0; n<bwidth; n++) {
	      for (i=0; i<8; i++) {
		 out = 0x00;
		 for (j=0; j<8; j++) {
		    indexin = m*bwidth*8 + n + j*bwidth;
		    in = indata[indexin];
		    out |= (((in<<i) & 0x80) >> j);
		 }
		 indexout = n*bheight*8 + m + i*bheight;
		 outdata[indexout] = out;
	      }
	   }
	}
}  /* end of c_image() */
#endif /* X11R5BOS */
