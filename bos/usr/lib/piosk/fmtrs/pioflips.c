static char sccsid[] = "@(#)91  1.6.1.8  src/bos/usr/lib/piosk/fmtrs/pioflips.c, cmdpiosk, bos411, 9428A410j 4/6/94 04:03:36";
/*
 *   COMPONENT_NAME: (CMDPIOSK)
 *
 *   FUNCTIONS: setup, initialize, lineout, passthru, restore
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Print Formatter For
 *				1: Canon Laser Shot LIPS II+
 *				2: Canon Laser Shot LIPS III
 */
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>

#define NLCHARMAX	257

#include "piostruct.h"
#include "jfmtrs_msg.h"
#include "lc.h"
#include "ffaccess.h"

CURcodeset	Ccs;
int	Ccsid;

# define IS_EUC(x) ( (x) == 2 || (x) == 128 || (x) == 129 )
# define IS_SJIS(x) ( (x) == 127 || (x) == 130 )

/********
Lookup Tables
********/
struct	lktable	lkup_rest[] = {
	"!",	0,
	"+",	1,
	"++",	2,
	NULL,	NULL
};

struct	lktable	lkup_emph[] = {
	"1",	1,
	"2",	2,
	"3",	3,
	"4",	4,
	"5",	5,
	"6",	6,
	"7",	7,
	"8",	8,
	"9",	9,
	"10",	10,
	"11",	11,
	"12",	12,
	"13",	13,
	"14",	14,
	"15",	15,
	"-------",	1,
	"------",	2,
	"-----",	3,
	"----",	4,
	"---",	5,
	"--",	6,
	"-",	7,
	"!",	8,
	"+",	9,
	"++",	10,
	"+++",	11,
	"++++",	12,
	"+++++",	13,
	"++++++",	14,
	"+++++++",	15,
	NULL,	NULL
};

struct	lktable	lkup_bool[] = {
	"+",	1,
	"!",	0,
	NULL,	NULL
};

struct	lktable lkup_fnt[] = {
	"linePrinter",	0,
	"pica",	1,
	"elite",	2,
	"courier",	3,
	"swiss",	4,
	"dutch",	5,
	"swissNarrow",	12,
	"zapfCalligraphic",	15,
	"garland",	16,
	"humanist",	17,
	"century",	23,
	"avantGarde",	31,
	"zapfChancery",	43,
	"zapfDingbats",	45,
	"bookman",	47,
	"Mincho",	80,
	"Kakugothic",	81,
	"Gothic",	81,
	"Marugothic",	82,
	"Kyokasho",	83,
	"Kaisho",	84,
	"Gyosho",	85,
	"Sosho",	86,
	"Reisho",	87,
	"Socho",	88,
	"Kanteiryu",	89,
	"Jinteiryu",	89,
	"Higemoji",	90,
	"Symbol",	137,
	"Keisen1",	192,
	"Keisen2",	193,
	NULL,	NULL
};

struct	lktable	lkup_src[] = {
	"1",	1,
	"2",	2,
	NULL,	NULL
};

struct	lktable lkup_page[] = {
	"0",	14,
	"1",	15,
	"2",	14,
	"3",	15,
	"A4",	14,
	"A4R",	15,
	"A5",	16,
	"A5R",	17,
	"A6",	18,
	"A6R",	19,
	"B4",	24,
	"B4R",	25,
	"B5",	26,
	"B5R",	27,
	NULL,	NULL
};

# include	"pioflips3.h"

struct shar_vars	*setup( int argc, char **argv, int passthru )
{
	extern void	errorexit( int m, char *p );
	extern void	pputs( char *p );

	if( piogetvals( attrtable, NULL ) < 0 )	pioexit( PIOEXITBAD );
	if( piogetopt( argc, argv, OPTSTRING, NULL ) < 0 )	pioexit( PIOEXITBAD );

	codeset_init();
	set_codeset_byname( Codepagename.ptr );
	if( passthru )	return( NULL );

	option_validation();
	margin_init();
	tab_const();
	stored_pos_const();
	setsharevars();
	return( &sharevars );
}


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


extern void	set_codeset_byval( int val, int gr_val );
extern void	errorexit( int m, char *p );

extern int	kanji_in( void );
extern int	kanji_out( void );
extern int	get_a_mbcx( int *ri, char *bx, int maxbx, int udc_chk );
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
#define	EOL	(-4)
#define	NORMAL	(-5)



#endif	/* ESCSEQ_DEF */


#include	<stdio.h>


extern int	Job_in;

int	charset[4];
int	forient[4];
int	graset[4];
int	charwidth[4];	/* inverse of char pitch */
int	realwidth[4];
int	charsize[4];
int	realsize[4];
int	charstyle[4];
int	__Stroke[4];
int	ftype[4];
int	ftype_sub[4];
int	Pair[4][4];
int	vertical[4];
int	charrotate;



int	nop()
{
	return( NORMAL );
}


int	parameter_reset_operation()
{
	return( NORMAL );
}

int	soft_reset_operation()
{
	parameter_reset_operation();
	return( NORMAL );
}


int	text_mode_start()
{
	Vector_mode = 0;
	Emulation_mode = 0;
	Job_in = 1;
	set_codeset_byval( 1, -1 );/* jis78 or 83 */
	cmdout( TEXT_START_CMD );
	return( NORMAL );
}

int	vector_mode_in()
{
	Vector_mode = 1;
	/* ***** */
	Vector_mode = 0;
	return( nop() );
}

int	emulation_in()
{
	Emulation_mode = 1;
	/* ****** */
	Emulation_mode = 0;
	return( nop() );
}

int	sjis_start()
{
	set_codeset_byval( 127, -1 );	/* shift JIS */
	cmdout( SJIS_START_CMD );
	return( NORMAL );
}

int	job_start()
{
	int	n = R_n();
	extern void	pputf();


	if( n == 4 && strcmp( R_st( 0 ), "?" ) ){
		if( ( n = atoi( R_st( 0 ) ) ) != 31 && n != 21 ) return( nop() );

		if( Job_in ){
			soft_reset_operation();
			cmdout( JOB_END_CMD );
		}

		Command_level = n;
		Dpi = ( n == 31 ) ?	300 : 240;
		if( Codepageval == 127 )	cmdout( TEXT_START_CMD );

		switch( atoi( R_st( 2 ) ) ){
		case 1:
		case 4:
		case 5:
			set_codeset_byval( 1, -1 );
			cmdout( JOB_START_CMD );
			Job_in = 1;
			break;
		case 2:
			set_codeset_byval( 2, -1 );
			cmdout( JOB_START_CMD );
			Job_in = 1;
			break;
		}
	}

	return( NORMAL );
}

int	job_end()
{
	if( R_n() >= 1 && !strcmp( R_st( 0 ), "0" ) && Job_in ){
		soft_reset_operation();
		cmdout( JOB_END_CMD );
		Job_in = 0;
		return( ff_cntl() );
	}
	return( NORMAL );
}

int	hard_reset()
{
	return( nop() );
}

int	soft_reset()
{
	cmdout( SOFT_RESET_CMD );
	soft_reset_operation();
	return( ff_cntl() );
}

int	parameter_reset()
{
	cmdout( PARA_RESET_CMD );
	parameter_reset_operation();
	return( ff_cntl() );
}

int	memory_free()
{
	cmdout( MEM_FREE_CMD );
	return( NORMAL );
}

int	cr_cmd()
{
	CAP_x = Lmargin;
	CAP_nomove = 0;
	PS_offset = 0;
	/* secret = 0, char_rotate = 0 */
}

int	cr_cntl()
{
	cr_cmd();
	cmdout( CR_CNTL_CMD );
	return( CRaddLF ? EOL1 : NORMAL );
}

int	lf_cntl()
{
	if( CAP_nomove && !LFaddCR ){
		CAP_y += FixVMI;
		sharevars._vincr = FixVMI;
	}else{
		CAP_y += VMI;
	}
	if( LFaddCR )	cr_cmd();
	return( EOL2 );
}

int	ff_cntl()
{
	CAP_y = Pagelength * 2;
	if( FFaddCR )	cr_cmd();
	return( EOL2 );
}

int	sp_cntl()
{
	CAP_x += CAP_nomove ? FixHMI: Hmi[GL];
	cmdout( SP_CNTL_CMD );
	return( NORMAL );
}

int	bs_cntl()
{
	CAP_x -= CAP_nomove ? FixHMI : Hmi[GL];
	if( CAP_x < 0 )	CAP_x = 0;
	cmdout( BS_CNTL_CMD );
	return( NORMAL );
}

int	crlf_cntl()
{
	cr_cmd();
	sharevars._vincr_cmd = NEL_CNTL_CMD;
	return( EOL1 );
}

int	half_lf_cntl()
{
	CAP_y += Int0 = VMI / 2;
	sharevars._vincr_cmd = PLD_CNTL_CMD;
	sharevars._vincr = &Int0;
	return( EOL2 );
}

int	reverse_lf_cntl()
{
	if( CAP_nomove ){
		CAP_y -= FixVMI;
		sharevars._vdecr = &FixVMI;
	}else{
		CAP_y -= VMI;
		sharevars._vdecr = &VMI;
	}

	sharevars._vdecr_cmd = RI_CNTL_CMD;
	return( EOL2 );
}

int	reverse_half_lf_cntl()
{
	CAP_y -= Int0 = VMI / 2;
	sharevars._vdecr = &Int0;
	sharevars._vdecr_cmd = PLU_CNTL_CMD;
	return( EOL2 );
}

int	size_unit_select()
{
	if( R_n() == 1 ){
		switch( atoi( R_st( 0 ) ) ){
		case 2:
			Userunit = 10; Userunit_div = 1;
			break;
		case 6:
			Userunit = 2834; Userunit_div = 10000;
			break;
		case 7:
			Userunit = Command_level == 31 ? 24 : 30;
			Userunit_div = 1;
			break;
		}
	}else if( R_n() == 2 &&
			!strcmp( R_st( 0 ), "?" ) && !strcmp( R_st( 1 ), "6" ) ){
		Userunit = 2834;
		Userunit_div = 1000;
	}

	return( nop() );
}

int	page_format_select()
{
	int	n, mod;
	static struct pfs_tbl{
		int	arg;
		int	l;
		int	w;
	} *p, tbl[6] =	{
		{ 14,  81300, 56640 },
		{ 16,  56640, 39210 },
		{ 18,  39210, 25464 },
		{ 24, 100296, 69960 },
		{ 26,  69960, 48696 },
		{ -1, 0, 0 }
	};
	int	max_l = Maxpagelength, max_w = Maxpagewidth;
	int	min_l = Minpagewidth, min_w = Minpagewidth;
	int	l, w;

/*
	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		n = atoi( R_st( 0 ) );
		mod = n % 2;
		n -= mod;
		if( n == 0 ) n = 14;

		for( p = tbl; p->arg != -1; p++ ){
			if( p->arg == n ){
				Pagelength = mod ? p->w : p->l;
				Pagewidth  = mod ? p->l : p->w;
				T_pageend = Dot_offset * Vres / Dpi + VMI * ( 1 +
					( Pagelength - ( Dot_offset + 20 ) * Vres / Dpi ) / VMI );
				Pagesize = n;
				
				return( NORMAL );

 tab, margin clear 
			}
		}

		if( n >= 40 && n < 50 && R_n() >= 3 ){
			l = atoi( R_st( 1 ) ) * Userunit / Userunit_div;
			w = atoi( R_st( 2 ) ) * Userunit / Userunit_div;
			if( l >= min_l && l <= max_l && w >= min_w && w >= max_w ){
				Pagelength = ( mod ? w : l ) - 2880;
				Pagewidth  = ( mod ? l : w ) - 2880;
				T_pageend = Dot_offset * Vres / Dpi + VMI * ( 1 +
					( Pagelength - ( Dot_offset + 20 ) * Vres / Dpi ) / VMI );
				return( NORMAL );
			}
		}
	}
*/
	return( nop() );
}

int	page_rotation()
{
	int	r;
/*
	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		if( ( r = atoi( R_st( 0 ) ) ) >= 0 ){
			Pagerotate = ( r + 4 - Pageorient ) & 3;
		}
	}
*/
	return( NORMAL );
}


int	multi_func_sub( int f )
{
	int	i, n, x;
	struct func_flags	*p;
	static struct func_flags{
		int	code;
		int	*p;
		char	*cmd;
	} func_flags_tbl[16] = {
		{ 1, &_Wrap, WRAP_MODE_CMD },
		{ 2, &_FF_explicit, AUTO_FF_CMD },
		{ 3, &_CAP_nomove, CAP_MODE_CMD },
		{ 4, &_CRaddLF, CR_MODE_CMD },
		{ 5, &_LFaddCR, LF_MODE_CMD },
		{ 6, &_FFaddCR, FF_MODE_CMD },
		{ 7, &_VTaddCR, VT_MODE_CMD },
		{ 8, &_No_PSpacing, NO_PS_CMD },
		{14, &_Goto_tmargin, TM_MODE_CMD },
		{23, &_HMI_explicit, HMI_MODE_CMD },
		{24, &_Mesh_overwrite, MESH_MODE_CMD },
		{25, &_Auto_emphasize, EMPH_MODE_CMD },
		{27, &_Restore_charattr, RCHAR_MODE_CMD },
		{28, &_String_expand, EXPAND_MODE_CMD },
		{29, &_Vertical_char_tr, VERTICAL_MODE_CMD },
		{-1, NULL }
	};


	if( ( n = R_n() ) > 0 ){
		if( strcmp( R_st( 0 ), "?" ) ){
			for( i = 0; i < n; i++ ){
				x = atoi( R_st( i ) );
				for( p = func_flags_tbl; p->code != -1 ; p++ ){
					if( p->code == x ){
						*( p->p + piomode ) = f;
						cmdout( p->cmd );
					}
				}
			}
		}else{
			for( i = 1; i < n; i++ ){
				if( !strcmp( R_st( i ), "11" ) ){
					Sizeunitformove = f;
/*					cmdout( SIZE_MODE_CMD );*/
				}
			}
		}
	}

	return( NORMAL );
}

int	multi_func_on()
{
	return( multi_func_sub( 1 ) );
}

int	multi_func_off()
{
	return( multi_func_sub( 0 ) );
}


int	margin_set()
{
	int	i, n;

	if( ( n = R_n() ) == 0 ){
		Lmargin = CAP_x;
		Int0 = 0;
		cmdout( MARGIN_SET_CMD );
	}else if( strcmp( R_st( 0 ), "?" ) ){
		for( i = 0; i < n; i++ ){
			switch( Int0 = atoi( R_st( i ) ) ){
			case 0:
				if( CAP_x <= Rmargin )	Lmargin = CAP_x;
				break;
			case 1:
				if( CAP_x >= Lmargin ) Rmargin = CAP_x;
				break;
			case 2:
				if( CAP_y <= Bmargin ) Tmargin = CAP_y;
				break;
			case 3:
				if( CAP_y >= Tmargin ) Bmargin = CAP_y;
				break;
			case 9:
				Pageend = CAP_y;
				break;
			default:
				continue;
			}
			cmdout( MARGIN_SET_CMD );
		}
	}
}

void	LR_margin_clear()
{
	Lmargin = 0;
	Rmargin = Pagewidth;
}

void	TB_margin_clear()
{
	Tmargin = Dot_offset * Vres / Dpi;
	Bmargin = Pageend > 0 ? Pageend : T_pageend;
}


void	all_margin_clear()
{
	LR_margin_clear();
	TB_margin_clear();
}

int	margin_clear()
{
	int	i, n;

	if( ( n = R_n() ) == 0 ){
		LR_margin_clear();
		Int0 = 0;
		cmdout( MARGIN_CLR_CMD );
	}else if( strcmp( R_st( 0 ), "?" ) ){
		for( i = 0; i < n; i++ ){
			switch( Int0 = atoi( R_st( i ) ) ){
			case 0:
				LR_margin_clear();
				break;
			case 1:
				TB_margin_clear();
				break;
			default:
				continue;
			}
			cmdout( MARGIN_CLR_CMD );
		}
	}

	return( NORMAL );
}

int	line_pitch_select()
{
	int	x;
	static struct lpis_tbl{
		int	code;
		int	vmi;
	} *p, tbl[6] = {
		{ 0, 1200 },
		{ 1, 1800 },
		{ 2, 2400 },
		{ 3,  600 },
		{ 4,  900 },
		{-1,    0 }
	};

	if( R_n() == 0 ){
		VMI = tbl[0].vmi;
		T_pageend = ( Pagelength );
		Int0 = 0;
		cmdout( LPI_SELECT_CMD );
	}else if( strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
		for( p = tbl; p->code != -1; p++ ){
			if( p->code == x ){
				VMI = p->vmi;
				T_pageend = ( Pagelength );
				Int0 = x;
				cmdout( LPI_SELECT_CMD );
			}
		}
	}

	return( NORMAL );
}

int	spacing_value()
{
	static int	max_vmi = 100800;
	static int	max_hmi = 100800;
	int	x;


	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		Str0.len = Str1.len = 0;

		if( *(char *)R_st( 0 ) != 0 ){
			x = atoi( R_st( 0 ) ) * Userunit / Userunit_div;
			if( x >= 0 && x <= max_vmi ){
				VMI = x;
				T_pageend = ( Pagelength );
				sprintf( Str0.ptr, VMI / ( Vres / 720 ) );
			}
		}

		if( R_n() >= 2 && *(char *)R_st( 1 ) ){
			x = atoi( R_st( 1 ) ) * Userunit / Userunit_div;
			if( x >= 0 && x <= max_hmi ){
				Hmi[GL] = x;
				sprintf( Str1.ptr, Hmi[GL] / ( Hres / 720 ) );
			}
		}

		if( Str0.len || Str1.len )	cmdout( SPACING_CMD );
	}

	return( NORMAL );
}

int	proportional_offset()
{
	int	x;
	static int	max_ps_offset = 3780;

	if( R_n() == 0 ){
		PS_offset = 0;
	}else if( strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) ) * Userunit / Userunit_div;
		if( x >= 0 && x <= max_ps_offset ) PS_offset = x;

		if( R_n() >= 2 && atoi( R_st( 1 ) ) == 1 )	PS_offset = -PS_offset;
	}

	Int0 = PS_offset;
	cmdout( P_OFFSET_CMD );
	return( NORMAL );
}

int	print_point_offset()
{
	static int	max_pre_offset = 50400;
	static int	max_post_offset = 50400;
	static int	max_v_offset = 50400;
	int	x;

	Pre_offset = Post_offset = V_offset = 0;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) ) * Userunit / Userunit_div;
		if( x >= 0 && x <= max_pre_offset ) Pre_offset = x;

		if( R_n() >= 2 ){
			x = atoi( R_st( 1 ) ) * Userunit / Userunit_div;
			if( x >= 0 && x <= max_post_offset ) Post_offset = x;

			if( R_n() >= 3 ){
				x = atoi( R_st( 2 ) ) * Userunit / Userunit_div;
				if( x >= 0 && x <= max_v_offset ) V_offset = x;

				if( R_n() >= 4 ){
					x = atoi( R_st( 3 ) );
					if( x & 4 ) Pre_offset = -Pre_offset;
					if( x & 2 ) Post_offset = -Post_offset;
					if( x & 1 ) V_offset = -V_offset;
				}
			}
		}
	}

	Int0 = Pre_offset;
	Int1 = Post_offset;
	Int2 = V_offset;
	cmdout( OFFSET_CMD );
	return( NORMAL );
}

int	abs_v_move_sub( int x )
{
	static int	max_vmove = 518400;
	static int	logical_dot_offset = 63;

	if( x == 0 ) x = 1;

	if( Sizeunitformove ){
		x = x * Userunit / Userunit_div;
	}else{
		x = x * VMI;
	}

	x += logical_dot_offset * Vres / Dpi;

	if( x >= 0 && x <= max_vmove ){
		return( x );
	}

	return( -1 );
}

int	abs_h_move_sub( int x )
{
	static int	max_hmove = 518400;

	if( x == 0 ) x = 1;

	if( Sizeunitformove ){
		x = x * Userunit / Userunit_div;
	}else{
		x = x * Hmi[GL];
	}

	if( x >= 0 && x <= max_hmove ){
		return( x );
	}

	return( -1 );
}

int	abs_vh_move()
{
	int	x = 1, y = 1;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		y = atoi( R_st( 0 ) );
		if( R_n() >= 2 )	x = atoi( R_st( 1 ) );
	}

	x = abs_h_move_sub( x );
	y = abs_v_move_sub( y );

	if( y != -1 && x != -1 ){
		if( CAP_y > y ){
			Int9 = CAP_y - y;
			sharevars._vdecr = &Int9;
			sharevars._vdecr_cmd = ABS_XYMOVE_CMD;
		}else if( CAP_y < y ){
			Int9 = y - CAP_y;
			sharevars._vincr = &Int9;	
			sharevars._vincr_cmd = ABS_XYMOVE_CMD;
		}else{
			Int1 = CAP_x = x;
			Int0 = CAP_y = y;
			cmdout( ABS_XYMOVE_CMD );
			return( NORMAL );
		}

		Int1 = CAP_x = x;
		Int0 = CAP_y = y;
		return( EOL2 );
	}

	return( NORMAL );
}

int	abs_v_move()
{
	int	y = 1;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		y = atoi( R_st( 0 ) );
	}

	y = abs_v_move_sub( y );
	if( y != -1 ){
		Int0 = y;

		if( Int0 > CAP_y ){
			Int9 = Int0 - CAP_y;
			sharevars._vincr = &Int9;
			sharevars._vincr_cmd = ABS_YMOVE_CMD;
		}else if( Int0 < CAP_y ){
			Int9 = CAP_y - Int0;
			sharevars._vdecr = &Int9;
			sharevars._vincr_cmd = ABS_YMOVE_CMD;
		}else{
			return( NORMAL );
		}

		CAP_y = Int0;
		return( EOL2 );
	}
	return( NORMAL );
}

int	abs_h_move()
{
	int	x = 1;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
	}

	x = abs_h_move_sub( x );
	if( x != -1 ){
		CAP_x = x;
		cmdout( ABS_XMOVE_CMD );
	}

	return( NORMAL );
}

int	rel_v_move( int x0 )
{
	int	x;
	static int	max_vmove = 518400;


	if( Sizeunitformove ){
		x0 = (int)( (double)Userunit * x0 / (double)Userunit_div );
		x = CAP_y + x0;
	}else{
		x = CAP_y + ( x0 *= VMI );
	}

	if( x >= 0 && x <= max_vmove ){
		if( x0 > 0 ){
			Int0 = x0;
			Int9 = x0;
			sharevars._vincr = &Int9;
			sharevars._vincr_cmd = REL_YMOVE_CMD;
		}else if( x0 < 0 ){
			Int0 = x0;
			Int9 = -x0;
			sharevars._vdecr = &Int9;
			sharevars._vdecr_cmd = REL_YMOVE_CMD;
		}

		if( x0 != 0 ){
			CAP_y = x;
			return( EOL2 );
		}
	}

	return( NORMAL );
}

int	rel_h_move( int x )
{
	static int	max_hmove = 518400;


	if( Sizeunitformove ){
		x = CAP_x + x * Userunit / Userunit_div;
	}else{
		x = CAP_x + x * Hmi[GL];
	}

	if( x >= 0 && x <= max_hmove ){
		CAP_x = x;
	}

	return( NORMAL );
}

int	up_move()
{
	int	x;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
		if( x == 0 ) x = 1;
		return( rel_v_move( -x ) );
	}

	return( NORMAL );
}

int	down_move()
{
	int	x;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
		if( x == 0 ) x = 1;
		return( rel_v_move( x ) );
	}

	return( NORMAL );
}

int	left_move()
{
	int	x;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
		if( x == 0 ) x = 1;
		return( rel_h_move( -x ) );
	}

	return( NORMAL );
}

int	right_move()
{
	int	x;

	if( R_n() >= 1 && strcmp( R_st( 0 ), "?" ) ){
		x = atoi( R_st( 0 ) );
		if( x == 0 ) x = 1;
		return( rel_h_move( x ) );
	}

	return( NORMAL );
}

int	pos_store_restore()
{
	int	act = 0, pos = 0, dir = 0;
	int	sy = CAP_y;

	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		act = atoi( R_st( 0 ) );
		if( R_n() >= 2 )	pos = atoi( R_st( 1 ) );
		if( R_n() >= 3 )	dir = atoi( R_st( 2 ) );
	}

	if( act >= 0 && act <= 1 && pos >= 0 && pos < 50 && dir >= 0 && dir <= 2 ){
		if( act ){
			if( dir & 1 == 0 ) Spos_y[pos] = CAP_y;
			if( dir & 2 == 0 ) Spos_x[pos] = CAP_x;
		}else{
			if( dir & 2 == 0 ) CAP_x = Spos_x[pos];

			if( dir & 1 == 0 && CAP_y != Spos_y[pos] ){
				Int1 = CAP_x;
				Int0 = Spos_y[pos];
				if( CAP_y > Spos_y[pos] ){
					Int9 = CAP_y - Spos_y[pos];
					sharevars._vdecr = &Int9;
					sharevars._vdecr_cmd = ABS_XYMOVE_CMD;
				}else{
					Int9 = Spos_y[pos] - CAP_y;
					sharevars._vincr = &Int9;
					sharevars._vincr_cmd = ABS_XYMOVE_CMD;
				}
				CAP_y = Spos_y[pos];
				return( EOL2 );
			}
		}
	}

	return( NORMAL );
}

int	set_htab()
{
	int	i, j;

	if( Htab_l >= Max_htabs ) return( NORMAL );

	for( i = 0; i < Htab_l; i++ ){
		if( Htabs[i] == CAP_x ){
			return( NORMAL );
		}else if( Htabs[i] > CAP_x ){
			for( j = Htab_l; j > i; j-- )	Htabs[j] = Htabs[j-1];
			Htab_l++;
			break;
		}
	}

	Htabs[i] = CAP_x;
	cmdout( HTS_CNTL_CMD );
	return( NORMAL );
}

int	ht_cntl()
{
	int	i;


	for( i = 0; i < Htab_l; i++ ){
		if( CAP_x < Htabs[i] ){
			CAP_x = Htabs[i];
			break;
		}
	}

	cmdout( HT_CNTL_CMD );
	return( NORMAL );
}

int	set_vtab()
{
	int	i, j;

	if( Vtab_l >= Max_vtabs ) return( NORMAL );

	for( i = 0; i < Vtab_l; i++ ){
		if( Vtabs[i] == CAP_y ){
			return( NORMAL );
		}else if( Vtabs[i] > CAP_y ){
			for( j = Vtab_l; j > i; j-- )	Vtabs[j] = Vtabs[j-1];
			Vtab_l++;
			break;
		}
	}

	Vtabs[i] = CAP_y;
	return( NORMAL );
}

int	vt_cntl()
{
	return( nop() );
}

int	tab_clr()
{
	int	i, n;

	if( ( n = R_n() ) && strcmp( R_st( 0 ), "?" ) ){
		for( i = 0; i < n; i++ ){
			switch( atoi( R_st( i ) ) ){
			case 3:
				Htab_l = 0;
				break;
			case 4:
				Vtab_l = 0;
				break;
			}
		}
	}

	return( NORMAL );
}

int	si_cntl()
{
	if( Save_GL < 0 ){
		GL = 0;
	}else{
		Save_GL = 0;
	}

	cmdout( SI_CNTL_CMD );
	return( NORMAL );
}

int	so_cntl()
{
	if( Save_GL < 0 ){
		GL = 1;
	}else{
		Save_GL = 1;
	}

	cmdout( SO_CNTL_CMD );
	return( NORMAL );
}

int	ls2_cntl()
{
	if( Save_GL < 0 ){
		GL = 2;
	}else{
		Save_GL = 2;
	}

	cmdout( LS2_CNTL_CMD );
	return( NORMAL );
}

int	ls3_cntl()
{
	if( Save_GL < 0 ){
		GL = 3;
	}else{
		Save_GL = 3;
	}

	cmdout( LS3_CNTL_CMD );
	return( NORMAL );
}

int	ss2_cntl()
{
	if( Save_GL < 0 ){
		Save_GL = GL;
		GL = 2;
	}

	if( IS_EUC( Incodepageval ) ){
		;
	}else{
		cmdout( SS2_CNTL_CMD );
	}

	return( NORMAL );
}

int	ss3_cntl()
{
	if( Save_GL < 0 ){
		Save_GL = GL;
		GL = 3;
	}

	if( IS_EUC( Incodepageval ) ){
		;
	}else{
		cmdout( SS3_CNTL_CMD );
	}

	return( NORMAL );
}

int	ls1r_cntl()
{
	GR = 1;
	cmdout( LS1R_CNTL_CMD );
	return( NORMAL );
}

int	ls2r_cntl()
{
	GR = 2;
	cmdout( LS2R_CNTL_CMD );
	return( NORMAL );
}

int	ls3r_cntl()
{
	GR = 3;
	cmdout( LS3R_CNTL_CMD );
	return( NORMAL );
}

int	pair_mode()
{
	int	i, j, k, l, m, n, x, p[4];


	if( ( n = R_n() ) && strcmp( R_st( 0 ), "?" ) ){
		for( i = 0; i < n; i++ ){
			x = atoi( R_st( i ) );

			if( x == 0 ){
				for( j = 0; j < 4; j++ ){
					for( k = 0; k < 4; k++ ){
						Pair[j][k] = 0;
					}
				}
			}else if( x >= 1 && x <= 6 ){
				p[0] = x <= 3 ? 0 : ( x <= 5 ? 1 : 2 );
				p[1] = x == 1 ? 1 : ( ( x == 2 || x == 4 ) ? 2 : 3 );
				j = 2;

				for( k = 0; k < 2; k++ ){
					for( l = 0; l < 4; l++ ){
						for( m = 0; m < j; m++ )	if( p[m] == l )	break;

						if( m == j && Pair[p[k]][l] )	p[j++] = l;
					}
				}

				for( k = 0; k < j; k++ ){
					for( l = 0; l < j; l++ ){
						if( k != l )	Pair[p[k]][p[l]] = 1;
					}
				}
			}
		}
	}

	return( NORMAL );
}

static int	read_gr_num()
{
	int	c;

	Unlock_buffer();

	do{
		c = Read_buffer();
	}while( c >= 0x20 && c < 0x30 );

	return( -1 );
}

int	g0_gr()
{
    if( Ccs->state != ST_NONE ){
	Unread_buffer();
	Unread_buffer();
	return(0);
    }
	read_gr_num();
	return( nop() );
}

int	g1_gr()
{
	read_gr_num();
	return( nop() );
}

int	g2_gr()
{
	read_gr_num();
	return( nop() );
}

int	g3_gr()
{
	read_gr_num();
	return( nop() );
}

static int	read_gr_num2()
{
	int	c;

	Unlock_buffer();

	do{
		c = Read_buffer();
	}while( c >= 0x20 && c < 0x30 );

	return( -1 );
}

int	g0_gr2()
{
	read_gr_num2();
	return( nop() );
}

int	g1_gr2()
{
	read_gr_num2();
	return( nop() );
}

int	g2_gr2()
{
	read_gr_num2();
	return( nop() );
}

int	g3_gr2()
{
	read_gr_num2();
	return( nop() );
}

int	set_char_attr( int *attrp, int val )
{
	int	i, j, k;

	attrp[GL] = val;

	for( i = 0; i < 4; i++ ){
		if( i != GL && Pair[GL][i] )	attrp[i] = val;
	}
}

int	cpi_select()
{
	int	cpi = 1000;
	int	x;

	if( R_n() ){
		if( strcmp( R_st( 0 ), "?" ) ){
			switch( atoi( R_st( 0 ) ) ){
			case 1:
				cpi = 1200;
				break;
			case 2:
				cpi = 1500;
				break;
			case 3:
				cpi = 600;
				break;
			}
		}else{
			if( ( x = atoi( R_st( 1 ) ) ) ){
				cpi = x;
			}
		}
	}

	x = Dpi * 100 / cpi;

	if( x >= 1 && x <= Max_csize ){
		set_char_attr( Charwidth, Hres * 100 / cpi );
	}

	return( NORMAL );
}

int	siz_select()
{
	int	x, sz;

	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		sz = atoi( R_st( 0 ) ) * Userunit / Userunit_div;
		x = sz * Dpi / Vres;

		if( x >= 1 && x <= Max_csize ){
			set_char_attr( Charheight, sz );
		}
	}

	return( NORMAL );
}

int	attr_chg()
{
	static struct atch_tbl{
		int	code;
		int	*attr;
		int	val;
	} *p, tbl[20] = {
		{  4, &Underline, 1 },
		{ 21, &Underline, 2 },
		{ 24, &Underline, 0 },
		{  5, &Mesh,      1 },
		{ 25, &Mesh,      0 },
		{  7, &Reverse,   1 },
		{ 27, &Reverse,   0 },
		{  8, &Secret,    1 },
		{ 28, &Secret,    0 },
		{ -5, &Pattfill,  1 },
		{-25, &Pattfill,  0 },
		{ -6, &Shadow,    1 },
		{-26, &Shadow,    0 },
		{ -7, &Outline,   1 },
		{-27, &Outline,   0 },
		{ 0, NULL, 0 }
	};
	int	stroke = 0;
	int	i, j, n, x;

	if( ( n = R_n() ) ){
		if( strcmp( R_st( 0 ), "?" ) ){
			for( i = 0; i < n; i++ ){
				x = atoi( R_st( i ) );
				for( p = tbl; p->code != 0; p++ ){
					if( p->code == x )	*p->attr = p->val;
				}

				switch( x ){
/* 10 - 19 ? */
				case 3:
					set_char_attr( charstyle, 1 );
					break;
				case 23:
					set_char_attr( charstyle, 0 );
					break;
				case 1:
					if( stroke == 0 ){
						stroke = 9;
					}else if( stroke < 15 ){
						stroke++;
					}
					break;
				case 2:
					if( stroke == 0 ){
						stroke = 7;
					}else if( stroke > 1 ){
						stroke--;
					}
					break;
				case 22:
					stroke = 8;
					break;
				
				}
			}

			if( stroke )	set_char_attr( __Stroke, stroke );
		}else{
			for( i = 1; i< n; i++ ){
				x = - atoi( R_st( i ) );
				for( p = tbl; p->code != 0; p++ ){
					if( p->code == x )	*p->attr = p->val;
				}
			}
		}
	}

	return( NORMAL );
}

int	ctype_select()
{
	int	ft = 0, ftsub;

	ftsub = ( Command_level == 31 ) ? 10 : 0;

	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		ft = atoi( R_st( 0 ) );

		if( R_n() > 1 )	ftsub = atoi( R_st( 1 ) );

		set_char_attr( ftype, ft );
		set_char_attr( ftype_sub, ftsub );
	}

	return( NORMAL );
}

int	char_set_name_select()
{
	char	*p;
	int	n;

	if( ( n = R_n() ) && *(char *)R_st( n - 1 ) ){
	}

	return( nop() );
}

int	charset_assign_number()
{
	return( nop() );
}

int	charset_assign_1()
{
	return( nop() );
}

int	Dumread( int n )
{
	int	i;

	Unlock_buffer();

	for( i = 0; i < n; i++ ){
		Read_buffer();
	}
}

int	charset_record()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) );
	}

	return( NORMAL );
}

int	charset_record_hex()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) * 2 );
	}

	return( NORMAL );
}

int	onechar_record()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) );
	}

	return( NORMAL );
}

int	onechar_record_hex()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) * 2 );
	}

	return( NORMAL );
}

int	charset_copy()
{
	return( nop() );
}

int	onechar_copy()
{
	return( nop() );
}

int	charset_record_help()
{
	return( nop() );
}

int	charset_delete()
{
	return( nop() );
}

int	ctrl_print()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) );
	}

	return( NORMAL );
}

int	ctrl_print_hex()
{
	if( R_n() && strcmp( R_st( 0 ), "?" ) ){
		Dumread( atoi( R_st( 0 ) ) * 2 );
	}

	return( NORMAL );
}

int	tate_yoko()
{
	int	x = atoi( R_st( 0 ) );

	vertical[0] = x & 1 ? 1 : 0;
	vertical[1] = x & 2 ? 1 : 0;
	vertical[2] = x & 4 ? 1 : 0;
	vertical[3] = x & 8 ? 1 : 0;
	return( NORMAL );
}

int	composed_print()
{
	int	i, j, n = 0, k;
	char	bx[ NLCHARMAX + 1 ];

	Unlock_buffer();

	if( R_n() ){
		if( strcmp( R_st( 0 ), "?" ) ){
			n = atoi( R_st( 0 ) );
		}
	}else{
		n = 2;
	}

	for( k = 0; k < n; k++ ){
		j = get_a_mbcx( &i, bx, NLCHARMAX, 1 );
	}

	return( NORMAL );
}


int	underline_attribute()
{
	return( NORMAL );
}

int	decolated_char_attr()
{
	return( NORMAL );
}

int	mesh_pattern()
{
	return( NORMAL );
}

int	expand_char()
{
	return( NORMAL );
}

int	char_rotation()
{
	return( NORMAL );
}

int	frame_mesh_start()
{
	return( NORMAL );
}

int	frame_mesh_end()
{
	return( NORMAL );
}

int	mesh_pattern_record()
{
	return( NORMAL );
}

int	mesh_pattern_record_hex()
{
	return( NORMAL );
}

int	gridline_start()
{
	return( NORMAL );
}

int	gridline_end()
{
	return( NORMAL );
}

int	decolated_gridline_start()
{
	return( NORMAL );
}

int	decolated_gridline_end()
{
	return( NORMAL );
}

int	decolated_gridline_pattern_record()
{
	return( NORMAL );
}

int	decolated_gridline_pattern_record_hex()
{
	return( NORMAL );
}

int	raster_image_draw()
{
	return( NORMAL );
}

int	raster_image_draw_hex()
{
	return( NORMAL );
}

int	allocate_image_area()
{
	return( NORMAL );
}

int	wiredot_image_draw1()
{
	return( NORMAL );
}

int	wiredot_image_draw1_hex()
{
	return( NORMAL );
}

int	wiredot_image_draw2()
{
	return( NORMAL );
}

int	wiredot_image_draw2_hex()
{
	return( NORMAL );
}

int	overlaypage_record()
{
	return( NORMAL );
}

int	overlay_print()
{
	return( NORMAL );
}

int	macro_record()
{
	return( NORMAL );
}

int	macro_record_hex()
{
	return( NORMAL );
}

int	macro_execute()
{
	return( NORMAL );
}

int	startup_macro()
{
	return( NORMAL );
}

int	backup_environment()
{
	return( NORMAL );
}

int	restore_environment()
{
	return( NORMAL );
}

int	undo()
{
	return( NORMAL );
}

int	paper_supply()
{
	return( NORMAL );
}

int	copy_number()
{
	return( NORMAL );
}

int	comment_display()
{
	return( NORMAL );
}

int	comment_display3()
{
	return( NORMAL );
}

int	bel_cntl()
{
	return( NORMAL );
}

int	status_request()
{
	return( NORMAL );
}

int	charset_search()
{
	return( NORMAL );
}

int	char_width_request()
{
	return( NORMAL );
}

int	nul_cntl()
{
	return( NORMAL );
}

int	etx_cntl()
{
	return( NORMAL );
}

int	ack_cntl()
{
	return( NORMAL );
}

int	xon_cntl()
{
	return( NORMAL );
}

int	xof_cntl()
{
	return( NORMAL );
}

int	del_cntl()
{
	return( nop() );
}


ESCSEQ_DATA	Ttdata[280] = {
	{ FIX_VALUE, 0x7f, Ttdata + 2, Ttdata + 1 },
	{ FUNC_LEAF, del_cntl, NULL, NULL },
	{ FIX_VALUE, ' ', Ttdata + 4, Ttdata + 3 },
	{ FUNC_LEAF, sp_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1d, Ttdata + 6, Ttdata + 5 },
	{ FUNC_LEAF, ss3_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1b, Ttdata + 250, Ttdata + 7 },
	{ FIX_VALUE, '~', Ttdata + 9, Ttdata + 8 },
	{ FUNC_LEAF, ls1r_cntl, NULL, NULL },
	{ FIX_VALUE, '}', Ttdata + 11, Ttdata + 10 },
	{ FUNC_LEAF, ls2r_cntl, NULL, NULL },
	{ FIX_VALUE, '|', Ttdata + 13, Ttdata + 12 },
	{ FUNC_LEAF, ls3r_cntl, NULL, NULL },
	{ FIX_VALUE, 'o', Ttdata + 15, Ttdata + 14 },
	{ FUNC_LEAF, ls3_cntl, NULL, NULL },
	{ FIX_VALUE, 'n', Ttdata + 17, Ttdata + 16 },
	{ FUNC_LEAF, ls2_cntl, NULL, NULL },
	{ FIX_VALUE, 'c', Ttdata + 19, Ttdata + 18 },
	{ FUNC_LEAF, hard_reset, NULL, NULL },
	{ FIX_VALUE, '[', Ttdata + 177, Ttdata + 20 },
	{ ASCII_ARGS, ';', Ttdata + 19, Ttdata + 21 },
	{ FIX_VALUE, '}', Ttdata + 23, Ttdata + 22 },
	{ FUNC_LEAF, gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', Ttdata + 25, Ttdata + 24 },
	{ FUNC_LEAF, gridline_start, NULL, NULL },
	{ FIX_VALUE, 'z', Ttdata + 27, Ttdata + 26 },
	{ FUNC_LEAF, memory_free, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 29, Ttdata + 28 },
	{ FUNC_LEAF, ctype_select, NULL, NULL },
	{ FIX_VALUE, 'x', Ttdata + 31, Ttdata + 30 },
	{ FUNC_LEAF, pos_store_restore, NULL, NULL },
	{ FIX_VALUE, 'w', Ttdata + 33, Ttdata + 32 },
	{ FUNC_LEAF, proportional_offset, NULL, NULL },
	{ FIX_VALUE, 'v', Ttdata + 35, Ttdata + 34 },
	{ FUNC_LEAF, copy_number, NULL, NULL },
	{ FIX_VALUE, 'u', Ttdata + 37, Ttdata + 36 },
	{ FUNC_LEAF, margin_clear, NULL, NULL },
	{ FIX_VALUE, 't', Ttdata + 39, Ttdata + 38 },
	{ FUNC_LEAF, margin_set, NULL, NULL },
	{ FIX_VALUE, 's', Ttdata + 41, Ttdata + 40 },
	{ FUNC_LEAF, frame_mesh_start, NULL, NULL },
	{ FIX_VALUE, 'r', Ttdata + 43, Ttdata + 42 },
	{ FUNC_LEAF, frame_mesh_end, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 45, Ttdata + 44 },
	{ FUNC_LEAF, paper_supply, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 47, Ttdata + 46 },
	{ FUNC_LEAF, page_format_select, NULL, NULL },
	{ FIX_VALUE, 'n', Ttdata + 49, Ttdata + 48 },
	{ FUNC_LEAF, status_request, NULL, NULL },
	{ FIX_VALUE, 'm', Ttdata + 51, Ttdata + 50 },
	{ FUNC_LEAF, attr_chg, NULL, NULL },
	{ FIX_VALUE, 'l', Ttdata + 53, Ttdata + 52 },
	{ FUNC_LEAF, multi_func_off, NULL, NULL },
	{ FIX_VALUE, 'k', Ttdata + 55, Ttdata + 54 },
	{ FUNC_LEAF, up_move, NULL, NULL },
	{ FIX_VALUE, 'j', Ttdata + 57, Ttdata + 56 },
	{ FUNC_LEAF, left_move, NULL, NULL },
	{ FIX_VALUE, 'h', Ttdata + 59, Ttdata + 58 },
	{ FUNC_LEAF, multi_func_on, NULL, NULL },
	{ FIX_VALUE, 'g', Ttdata + 61, Ttdata + 60 },
	{ FUNC_LEAF, tab_clr, NULL, NULL },
	{ FIX_VALUE, 'f', Ttdata + 63, Ttdata + 62 },
	{ FUNC_LEAF, abs_vh_move, NULL, NULL },
	{ FIX_VALUE, 'e', Ttdata + 65, Ttdata + 64 },
	{ FUNC_LEAF, down_move, NULL, NULL },
	{ FIX_VALUE, 'd', Ttdata + 67, Ttdata + 66 },
	{ FUNC_LEAF, abs_v_move, NULL, NULL },
	{ FIX_VALUE, 'a', Ttdata + 69, Ttdata + 68 },
	{ FUNC_LEAF, right_move, NULL, NULL },
	{ FIX_VALUE, '`', Ttdata + 71, Ttdata + 70 },
	{ FUNC_LEAF, abs_h_move, NULL, NULL },
	{ FIX_VALUE, '/', Ttdata + 90, Ttdata + 72 },
	{ FIX_VALUE, '~', Ttdata + 74, Ttdata + 73 },
	{ FUNC_LEAF, decolated_gridline_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'z', Ttdata + 76, Ttdata + 75 },
	{ FUNC_LEAF, macro_record_hex, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 78, Ttdata + 77 },
	{ FUNC_LEAF, mesh_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'v', Ttdata + 80, Ttdata + 79 },
	{ FUNC_LEAF, ctrl_print_hex, NULL, NULL },
	{ FIX_VALUE, 't', Ttdata + 82, Ttdata + 81 },
	{ FUNC_LEAF, wiredot_image_draw2_hex, NULL, NULL },
	{ FIX_VALUE, 's', Ttdata + 84, Ttdata + 83 },
	{ FUNC_LEAF, wiredot_image_draw1_hex, NULL, NULL },
	{ FIX_VALUE, 'r', Ttdata + 86, Ttdata + 85 },
	{ FUNC_LEAF, raster_image_draw_hex, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 88, Ttdata + 87 },
	{ FUNC_LEAF, onechar_record_hex, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 71, Ttdata + 89 },
	{ FUNC_LEAF, charset_record_hex, NULL, NULL },
	{ FIX_VALUE, '.', Ttdata + 109, Ttdata + 91 },
	{ FIX_VALUE, '~', Ttdata + 93, Ttdata + 92 },
	{ FUNC_LEAF, decolated_gridline_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'z', Ttdata + 95, Ttdata + 94 },
	{ FUNC_LEAF, macro_record, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 97, Ttdata + 96 },
	{ FUNC_LEAF, mesh_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'v', Ttdata + 99, Ttdata + 98 },
	{ FUNC_LEAF, ctrl_print, NULL, NULL },
	{ FIX_VALUE, 't', Ttdata + 101, Ttdata + 100 },
	{ FUNC_LEAF, wiredot_image_draw2, NULL, NULL },
	{ FIX_VALUE, 's', Ttdata + 103, Ttdata + 102 },
	{ FUNC_LEAF, wiredot_image_draw1, NULL, NULL },
	{ FIX_VALUE, 'r', Ttdata + 105, Ttdata + 104 },
	{ FUNC_LEAF, raster_image_draw, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 107, Ttdata + 106 },
	{ FUNC_LEAF, onechar_record, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 90, Ttdata + 108 },
	{ FUNC_LEAF, charset_record, NULL, NULL },
	{ FIX_VALUE, '&', Ttdata + 126, Ttdata + 110 },
	{ FIX_VALUE, '}', Ttdata + 112, Ttdata + 111 },
	{ FUNC_LEAF, vector_mode_in, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 114, Ttdata + 113 },
	{ FUNC_LEAF, comment_display3, NULL, NULL },
	{ FIX_VALUE, 'w', Ttdata + 116, Ttdata + 115 },
	{ FUNC_LEAF, overlay_print, NULL, NULL },
	{ FIX_VALUE, 'u', Ttdata + 118, Ttdata + 117 },
	{ FUNC_LEAF, allocate_image_area, NULL, NULL },
	{ FIX_VALUE, 't', Ttdata + 120, Ttdata + 119 },
	{ FUNC_LEAF, char_rotation, NULL, NULL },
	{ FIX_VALUE, 's', Ttdata + 122, Ttdata + 121 },
	{ FUNC_LEAF, overlaypage_record, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 124, Ttdata + 123 },
	{ FUNC_LEAF, onechar_copy, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 109, Ttdata + 125 },
	{ FUNC_LEAF, charset_delete, NULL, NULL },
	{ FIX_VALUE, '%', Ttdata + 147, Ttdata + 127 },
	{ FIX_VALUE, '}', Ttdata + 129, Ttdata + 128 },
	{ FUNC_LEAF, decolated_char_attr, NULL, NULL },
	{ FIX_VALUE, 'z', Ttdata + 131, Ttdata + 130 },
	{ FUNC_LEAF, restore_environment, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 133, Ttdata + 132 },
	{ FUNC_LEAF, backup_environment, NULL, NULL },
	{ FIX_VALUE, 'x', Ttdata + 135, Ttdata + 134 },
	{ FUNC_LEAF, macro_execute, NULL, NULL },
	{ FIX_VALUE, 'w', Ttdata + 137, Ttdata + 136 },
	{ FUNC_LEAF, print_point_offset, NULL, NULL },
	{ FIX_VALUE, 'v', Ttdata + 139, Ttdata + 138 },
	{ FUNC_LEAF, charset_assign_1, NULL, NULL },
	{ FIX_VALUE, 'u', Ttdata + 141, Ttdata + 140 },
	{ FUNC_LEAF, underline_attribute, NULL, NULL },
	{ FIX_VALUE, 't', Ttdata + 143, Ttdata + 142 },
	{ FUNC_LEAF, charset_search, NULL, NULL },
	{ FIX_VALUE, 'r', Ttdata + 145, Ttdata + 144 },
	{ FUNC_LEAF, page_rotation, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 126, Ttdata + 146 },
	{ FUNC_LEAF, startup_macro, NULL, NULL },
	{ FIX_VALUE, '$', Ttdata + 160, Ttdata + 148 },
	{ FIX_VALUE, '}', Ttdata + 150, Ttdata + 149 },
	{ FUNC_LEAF, decolated_gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', Ttdata + 152, Ttdata + 151 },
	{ FUNC_LEAF, decolated_gridline_start, NULL, NULL },
	{ FIX_VALUE, 'u', Ttdata + 154, Ttdata + 153 },
	{ FUNC_LEAF, undo, NULL, NULL },
	{ FIX_VALUE, 's', Ttdata + 156, Ttdata + 155 },
	{ FUNC_LEAF, mesh_pattern, NULL, NULL },
	{ FIX_VALUE, 'q', Ttdata + 158, Ttdata + 157 },
	{ FUNC_LEAF, composed_print, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 147, Ttdata + 159 },
	{ FUNC_LEAF, pair_mode, NULL, NULL },
	{ FIX_VALUE, ' ', Ttdata + 20, Ttdata + 161 },
	{ FIX_VALUE, 'S', Ttdata + 163, Ttdata + 162 },
	{ FUNC_LEAF, tate_yoko, NULL, NULL },
	{ FIX_VALUE, 'L', Ttdata + 165, Ttdata + 164 },
	{ FUNC_LEAF, line_pitch_select, NULL, NULL },
	{ FIX_VALUE, 'K', Ttdata + 167, Ttdata + 166 },
	{ FUNC_LEAF, cpi_select, NULL, NULL },
	{ FIX_VALUE, 'I', Ttdata + 169, Ttdata + 168 },
	{ FUNC_LEAF, size_unit_select, NULL, NULL },
	{ FIX_VALUE, 'G', Ttdata + 171, Ttdata + 170 },
	{ FUNC_LEAF, spacing_value, NULL, NULL },
	{ FIX_VALUE, 'D', Ttdata + 173, Ttdata + 172 },
	{ FUNC_LEAF, charset_assign_number, NULL, NULL },
	{ FIX_VALUE, 'C', Ttdata + 175, Ttdata + 174 },
	{ FUNC_LEAF, siz_select, NULL, NULL },
	{ FIX_VALUE, 'B', Ttdata + 160, Ttdata + 176 },
	{ FUNC_LEAF, expand_char, NULL, NULL },
	{ FIX_VALUE, 'P', Ttdata + 204, Ttdata + 178 },
	{ ASCII_ARGS, ';', Ttdata + 177, Ttdata + 179 },
	{ FIX_VALUE, 'z', Ttdata + 184, Ttdata + 180 },
	{ VAR_SIZE_TIL, 0, Ttdata + 179, Ttdata + 181 },
	{ FIX_VALUE, 0x1b, Ttdata + 180, Ttdata + 182 },
	{ FIX_VALUE, '\\', Ttdata + 181, Ttdata + 183 },
	{ FUNC_LEAF, char_set_name_select, NULL, NULL },
	{ FIX_VALUE, 'y', Ttdata + 189, Ttdata + 185 },
	{ VAR_SIZE_TIL, 0, Ttdata + 184, Ttdata + 186 },
	{ FIX_VALUE, 0x1b, Ttdata + 185, Ttdata + 187 },
	{ FIX_VALUE, '\\', Ttdata + 186, Ttdata + 188 },
	{ FUNC_LEAF, comment_display, NULL, NULL },
	{ FIX_VALUE, 'x', Ttdata + 194, Ttdata + 190 },
	{ VAR_SIZE_TIL, 0, Ttdata + 189, Ttdata + 191 },
	{ FIX_VALUE, 0x1b, Ttdata + 190, Ttdata + 192 },
	{ FIX_VALUE, '\\', Ttdata + 191, Ttdata + 193 },
	{ FUNC_LEAF, charset_record_help, NULL, NULL },
	{ FIX_VALUE, 'p', Ttdata + 199, Ttdata + 195 },
	{ VAR_SIZE_TIL, 0, Ttdata + 194, Ttdata + 196 },
	{ FIX_VALUE, 0x1b, Ttdata + 195, Ttdata + 197 },
	{ FIX_VALUE, '\\', Ttdata + 196, Ttdata + 198 },
	{ FUNC_LEAF, charset_copy, NULL, NULL },
	{ FIX_VALUE, 'J', Ttdata + 178, Ttdata + 200 },
	{ VAR_SIZE_TIL, 0, Ttdata + 199, Ttdata + 201 },
	{ FIX_VALUE, 0x1b, Ttdata + 200, Ttdata + 202 },
	{ FIX_VALUE, '\\', Ttdata + 201, Ttdata + 203 },
	{ FUNC_LEAF, job_start, NULL, NULL },
	{ FIX_VALUE, 'O', Ttdata + 206, Ttdata + 205 },
	{ FUNC_LEAF, ss3_cntl, NULL, NULL },
	{ FIX_VALUE, 'N', Ttdata + 208, Ttdata + 207 },
	{ FUNC_LEAF, ss2_cntl, NULL, NULL },
	{ FIX_VALUE, 'M', Ttdata + 210, Ttdata + 209 },
	{ FUNC_LEAF, reverse_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'L', Ttdata + 212, Ttdata + 211 },
	{ FUNC_LEAF, reverse_half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'K', Ttdata + 214, Ttdata + 213 },
	{ FUNC_LEAF, half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'J', Ttdata + 216, Ttdata + 215 },
	{ FUNC_LEAF, set_vtab, NULL, NULL },
	{ FIX_VALUE, 'H', Ttdata + 218, Ttdata + 217 },
	{ FUNC_LEAF, set_htab, NULL, NULL },
	{ FIX_VALUE, 'E', Ttdata + 220, Ttdata + 219 },
	{ FUNC_LEAF, crlf_cntl, NULL, NULL },
	{ FIX_VALUE, '?', Ttdata + 222, Ttdata + 221 },
	{ FUNC_LEAF, char_width_request, NULL, NULL },
	{ FIX_VALUE, '=', Ttdata + 224, Ttdata + 223 },
	{ FUNC_LEAF, parameter_reset, NULL, NULL },
	{ FIX_VALUE, '<', Ttdata + 226, Ttdata + 225 },
	{ FUNC_LEAF, soft_reset, NULL, NULL },
	{ FIX_VALUE, '+', Ttdata + 228, Ttdata + 227 },
	{ FUNC_LEAF, g3_gr, NULL, NULL },
	{ FIX_VALUE, '*', Ttdata + 230, Ttdata + 229 },
	{ FUNC_LEAF, g2_gr, NULL, NULL },
	{ FIX_VALUE, ')', Ttdata + 232, Ttdata + 231 },
	{ FUNC_LEAF, g1_gr, NULL, NULL },
	{ FIX_VALUE, '(', Ttdata + 234, Ttdata + 233 },
	{ FUNC_LEAF, g0_gr, NULL, NULL },
	{ FIX_VALUE, '%', Ttdata + 241, Ttdata + 235 },
	{ FIX_VALUE, '@', Ttdata + 237, Ttdata + 236 },
	{ FUNC_LEAF, text_mode_start, NULL, NULL },
	{ FIX_VALUE, '1', Ttdata + 239, Ttdata + 238 },
	{ FUNC_LEAF, emulation_in, NULL, NULL },
	{ FIX_VALUE, '0', Ttdata + 234, Ttdata + 240 },
	{ FUNC_LEAF, sjis_start, NULL, NULL },
	{ FIX_VALUE, '$', Ttdata + 6, Ttdata + 242 },
	{ FIX_VALUE, '+', Ttdata + 244, Ttdata + 243 },
	{ FUNC_LEAF, g3_gr2, NULL, NULL },
	{ FIX_VALUE, '*', Ttdata + 246, Ttdata + 245 },
	{ FUNC_LEAF, g2_gr2, NULL, NULL },
	{ FIX_VALUE, ')', Ttdata + 248, Ttdata + 247 },
	{ FUNC_LEAF, g1_gr2, NULL, NULL },
	{ FIX_VALUE, '(', Ttdata + 241, Ttdata + 249 },
	{ FUNC_LEAF, g0_gr2, NULL, NULL },
	{ FIX_VALUE, 0x19, Ttdata + 252, Ttdata + 251 },
	{ FUNC_LEAF, ss2_cntl, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 254, Ttdata + 253 },
	{ FUNC_LEAF, xof_cntl, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 256, Ttdata + 255 },
	{ FUNC_LEAF, xon_cntl, NULL, NULL },
	{ FIX_VALUE, 0xf, Ttdata + 258, Ttdata + 257 },
	{ FUNC_LEAF, si_cntl, NULL, NULL },
	{ FIX_VALUE, 0xe, Ttdata + 260, Ttdata + 259 },
	{ FUNC_LEAF, so_cntl, NULL, NULL },
	{ FIX_VALUE, 0xd, Ttdata + 262, Ttdata + 261 },
	{ FUNC_LEAF, cr_cntl, NULL, NULL },
	{ FIX_VALUE, 0xc, Ttdata + 264, Ttdata + 263 },
	{ FUNC_LEAF, ff_cntl, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 266, Ttdata + 265 },
	{ FUNC_LEAF, vt_cntl, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 268, Ttdata + 267 },
	{ FUNC_LEAF, lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x9, Ttdata + 270, Ttdata + 269 },
	{ FUNC_LEAF, ht_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 272, Ttdata + 271 },
	{ FUNC_LEAF, bs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x7, Ttdata + 274, Ttdata + 273 },
	{ FUNC_LEAF, bel_cntl, NULL, NULL },
	{ FIX_VALUE, 0x6, Ttdata + 276, Ttdata + 275 },
	{ FUNC_LEAF, ack_cntl, NULL, NULL },
	{ FIX_VALUE, 0x3, Ttdata + 278, Ttdata + 277 },
	{ FUNC_LEAF, etx_cntl, NULL, NULL },
	{ FIX_VALUE, 0x0, NULL, Ttdata + 279 },
	{ FUNC_LEAF, nul_cntl, NULL, NULL }
};




ESCSEQ_DATA	Nonsjis_data[198] = {
	{ FIX_VALUE, 0x9b, Nonsjis_data + 158, Nonsjis_data + 1 },
	{ ASCII_ARGS, ';', Nonsjis_data + 0, Nonsjis_data + 2 },
	{ FIX_VALUE, '}', Nonsjis_data + 4, Nonsjis_data + 3 },
	{ FUNC_LEAF, gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', Nonsjis_data + 6, Nonsjis_data + 5 },
	{ FUNC_LEAF, gridline_start, NULL, NULL },
	{ FIX_VALUE, 'z', Nonsjis_data + 8, Nonsjis_data + 7 },
	{ FUNC_LEAF, memory_free, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 10, Nonsjis_data + 9 },
	{ FUNC_LEAF, ctype_select, NULL, NULL },
	{ FIX_VALUE, 'x', Nonsjis_data + 12, Nonsjis_data + 11 },
	{ FUNC_LEAF, pos_store_restore, NULL, NULL },
	{ FIX_VALUE, 'w', Nonsjis_data + 14, Nonsjis_data + 13 },
	{ FUNC_LEAF, proportional_offset, NULL, NULL },
	{ FIX_VALUE, 'v', Nonsjis_data + 16, Nonsjis_data + 15 },
	{ FUNC_LEAF, copy_number, NULL, NULL },
	{ FIX_VALUE, 'u', Nonsjis_data + 18, Nonsjis_data + 17 },
	{ FUNC_LEAF, margin_clear, NULL, NULL },
	{ FIX_VALUE, 't', Nonsjis_data + 20, Nonsjis_data + 19 },
	{ FUNC_LEAF, margin_set, NULL, NULL },
	{ FIX_VALUE, 's', Nonsjis_data + 22, Nonsjis_data + 21 },
	{ FUNC_LEAF, frame_mesh_start, NULL, NULL },
	{ FIX_VALUE, 'r', Nonsjis_data + 24, Nonsjis_data + 23 },
	{ FUNC_LEAF, frame_mesh_end, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 26, Nonsjis_data + 25 },
	{ FUNC_LEAF, paper_supply, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 28, Nonsjis_data + 27 },
	{ FUNC_LEAF, page_format_select, NULL, NULL },
	{ FIX_VALUE, 'n', Nonsjis_data + 30, Nonsjis_data + 29 },
	{ FUNC_LEAF, status_request, NULL, NULL },
	{ FIX_VALUE, 'm', Nonsjis_data + 32, Nonsjis_data + 31 },
	{ FUNC_LEAF, attr_chg, NULL, NULL },
	{ FIX_VALUE, 'l', Nonsjis_data + 34, Nonsjis_data + 33 },
	{ FUNC_LEAF, multi_func_off, NULL, NULL },
	{ FIX_VALUE, 'k', Nonsjis_data + 36, Nonsjis_data + 35 },
	{ FUNC_LEAF, up_move, NULL, NULL },
	{ FIX_VALUE, 'j', Nonsjis_data + 38, Nonsjis_data + 37 },
	{ FUNC_LEAF, left_move, NULL, NULL },
	{ FIX_VALUE, 'h', Nonsjis_data + 40, Nonsjis_data + 39 },
	{ FUNC_LEAF, multi_func_on, NULL, NULL },
	{ FIX_VALUE, 'g', Nonsjis_data + 42, Nonsjis_data + 41 },
	{ FUNC_LEAF, tab_clr, NULL, NULL },
	{ FIX_VALUE, 'f', Nonsjis_data + 44, Nonsjis_data + 43 },
	{ FUNC_LEAF, abs_vh_move, NULL, NULL },
	{ FIX_VALUE, 'e', Nonsjis_data + 46, Nonsjis_data + 45 },
	{ FUNC_LEAF, down_move, NULL, NULL },
	{ FIX_VALUE, 'd', Nonsjis_data + 48, Nonsjis_data + 47 },
	{ FUNC_LEAF, abs_v_move, NULL, NULL },
	{ FIX_VALUE, 'a', Nonsjis_data + 50, Nonsjis_data + 49 },
	{ FUNC_LEAF, right_move, NULL, NULL },
	{ FIX_VALUE, '`', Nonsjis_data + 52, Nonsjis_data + 51 },
	{ FUNC_LEAF, abs_h_move, NULL, NULL },
	{ FIX_VALUE, '/', Nonsjis_data + 71, Nonsjis_data + 53 },
	{ FIX_VALUE, '~', Nonsjis_data + 55, Nonsjis_data + 54 },
	{ FUNC_LEAF, decolated_gridline_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'z', Nonsjis_data + 57, Nonsjis_data + 56 },
	{ FUNC_LEAF, macro_record_hex, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 59, Nonsjis_data + 58 },
	{ FUNC_LEAF, mesh_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'v', Nonsjis_data + 61, Nonsjis_data + 60 },
	{ FUNC_LEAF, ctrl_print_hex, NULL, NULL },
	{ FIX_VALUE, 't', Nonsjis_data + 63, Nonsjis_data + 62 },
	{ FUNC_LEAF, wiredot_image_draw2_hex, NULL, NULL },
	{ FIX_VALUE, 's', Nonsjis_data + 65, Nonsjis_data + 64 },
	{ FUNC_LEAF, wiredot_image_draw1_hex, NULL, NULL },
	{ FIX_VALUE, 'r', Nonsjis_data + 67, Nonsjis_data + 66 },
	{ FUNC_LEAF, raster_image_draw_hex, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 69, Nonsjis_data + 68 },
	{ FUNC_LEAF, onechar_record_hex, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 52, Nonsjis_data + 70 },
	{ FUNC_LEAF, charset_record_hex, NULL, NULL },
	{ FIX_VALUE, '.', Nonsjis_data + 90, Nonsjis_data + 72 },
	{ FIX_VALUE, '~', Nonsjis_data + 74, Nonsjis_data + 73 },
	{ FUNC_LEAF, decolated_gridline_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'z', Nonsjis_data + 76, Nonsjis_data + 75 },
	{ FUNC_LEAF, macro_record, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 78, Nonsjis_data + 77 },
	{ FUNC_LEAF, mesh_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'v', Nonsjis_data + 80, Nonsjis_data + 79 },
	{ FUNC_LEAF, ctrl_print, NULL, NULL },
	{ FIX_VALUE, 't', Nonsjis_data + 82, Nonsjis_data + 81 },
	{ FUNC_LEAF, wiredot_image_draw2, NULL, NULL },
	{ FIX_VALUE, 's', Nonsjis_data + 84, Nonsjis_data + 83 },
	{ FUNC_LEAF, wiredot_image_draw1, NULL, NULL },
	{ FIX_VALUE, 'r', Nonsjis_data + 86, Nonsjis_data + 85 },
	{ FUNC_LEAF, raster_image_draw, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 88, Nonsjis_data + 87 },
	{ FUNC_LEAF, onechar_record, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 71, Nonsjis_data + 89 },
	{ FUNC_LEAF, charset_record, NULL, NULL },
	{ FIX_VALUE, '&', Nonsjis_data + 107, Nonsjis_data + 91 },
	{ FIX_VALUE, '}', Nonsjis_data + 93, Nonsjis_data + 92 },
	{ FUNC_LEAF, vector_mode_in, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 95, Nonsjis_data + 94 },
	{ FUNC_LEAF, comment_display3, NULL, NULL },
	{ FIX_VALUE, 'w', Nonsjis_data + 97, Nonsjis_data + 96 },
	{ FUNC_LEAF, overlay_print, NULL, NULL },
	{ FIX_VALUE, 'u', Nonsjis_data + 99, Nonsjis_data + 98 },
	{ FUNC_LEAF, allocate_image_area, NULL, NULL },
	{ FIX_VALUE, 't', Nonsjis_data + 101, Nonsjis_data + 100 },
	{ FUNC_LEAF, char_rotation, NULL, NULL },
	{ FIX_VALUE, 's', Nonsjis_data + 103, Nonsjis_data + 102 },
	{ FUNC_LEAF, overlaypage_record, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 105, Nonsjis_data + 104 },
	{ FUNC_LEAF, onechar_copy, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 90, Nonsjis_data + 106 },
	{ FUNC_LEAF, charset_delete, NULL, NULL },
	{ FIX_VALUE, '%', Nonsjis_data + 128, Nonsjis_data + 108 },
	{ FIX_VALUE, '}', Nonsjis_data + 110, Nonsjis_data + 109 },
	{ FUNC_LEAF, decolated_char_attr, NULL, NULL },
	{ FIX_VALUE, 'z', Nonsjis_data + 112, Nonsjis_data + 111 },
	{ FUNC_LEAF, restore_environment, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 114, Nonsjis_data + 113 },
	{ FUNC_LEAF, backup_environment, NULL, NULL },
	{ FIX_VALUE, 'x', Nonsjis_data + 116, Nonsjis_data + 115 },
	{ FUNC_LEAF, macro_execute, NULL, NULL },
	{ FIX_VALUE, 'w', Nonsjis_data + 118, Nonsjis_data + 117 },
	{ FUNC_LEAF, print_point_offset, NULL, NULL },
	{ FIX_VALUE, 'v', Nonsjis_data + 120, Nonsjis_data + 119 },
	{ FUNC_LEAF, charset_assign_1, NULL, NULL },
	{ FIX_VALUE, 'u', Nonsjis_data + 122, Nonsjis_data + 121 },
	{ FUNC_LEAF, underline_attribute, NULL, NULL },
	{ FIX_VALUE, 't', Nonsjis_data + 124, Nonsjis_data + 123 },
	{ FUNC_LEAF, charset_search, NULL, NULL },
	{ FIX_VALUE, 'r', Nonsjis_data + 126, Nonsjis_data + 125 },
	{ FUNC_LEAF, page_rotation, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 107, Nonsjis_data + 127 },
	{ FUNC_LEAF, startup_macro, NULL, NULL },
	{ FIX_VALUE, '$', Nonsjis_data + 141, Nonsjis_data + 129 },
	{ FIX_VALUE, '}', Nonsjis_data + 131, Nonsjis_data + 130 },
	{ FUNC_LEAF, decolated_gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', Nonsjis_data + 133, Nonsjis_data + 132 },
	{ FUNC_LEAF, decolated_gridline_start, NULL, NULL },
	{ FIX_VALUE, 'u', Nonsjis_data + 135, Nonsjis_data + 134 },
	{ FUNC_LEAF, undo, NULL, NULL },
	{ FIX_VALUE, 's', Nonsjis_data + 137, Nonsjis_data + 136 },
	{ FUNC_LEAF, mesh_pattern, NULL, NULL },
	{ FIX_VALUE, 'q', Nonsjis_data + 139, Nonsjis_data + 138 },
	{ FUNC_LEAF, composed_print, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 128, Nonsjis_data + 140 },
	{ FUNC_LEAF, pair_mode, NULL, NULL },
	{ FIX_VALUE, ' ', Nonsjis_data + 1, Nonsjis_data + 142 },
	{ FIX_VALUE, 'S', Nonsjis_data + 144, Nonsjis_data + 143 },
	{ FUNC_LEAF, tate_yoko, NULL, NULL },
	{ FIX_VALUE, 'L', Nonsjis_data + 146, Nonsjis_data + 145 },
	{ FUNC_LEAF, line_pitch_select, NULL, NULL },
	{ FIX_VALUE, 'K', Nonsjis_data + 148, Nonsjis_data + 147 },
	{ FUNC_LEAF, cpi_select, NULL, NULL },
	{ FIX_VALUE, 'I', Nonsjis_data + 150, Nonsjis_data + 149 },
	{ FUNC_LEAF, size_unit_select, NULL, NULL },
	{ FIX_VALUE, 'G', Nonsjis_data + 152, Nonsjis_data + 151 },
	{ FUNC_LEAF, spacing_value, NULL, NULL },
	{ FIX_VALUE, 'D', Nonsjis_data + 154, Nonsjis_data + 153 },
	{ FUNC_LEAF, charset_assign_number, NULL, NULL },
	{ FIX_VALUE, 'C', Nonsjis_data + 156, Nonsjis_data + 155 },
	{ FUNC_LEAF, siz_select, NULL, NULL },
	{ FIX_VALUE, 'B', Nonsjis_data + 141, Nonsjis_data + 157 },
	{ FUNC_LEAF, expand_char, NULL, NULL },
	{ FIX_VALUE, 0x90, Nonsjis_data + 180, Nonsjis_data + 159 },
	{ ASCII_ARGS, ';', Nonsjis_data + 158, Nonsjis_data + 160 },
	{ FIX_VALUE, 'z', Nonsjis_data + 164, Nonsjis_data + 161 },
	{ VAR_SIZE_TIL, 0, Nonsjis_data + 160, Nonsjis_data + 162 },
	{ FIX_VALUE, 0x9c, Nonsjis_data + 161, Nonsjis_data + 163 },
	{ FUNC_LEAF, char_set_name_select, NULL, NULL },
	{ FIX_VALUE, 'y', Nonsjis_data + 168, Nonsjis_data + 165 },
	{ VAR_SIZE_TIL, 0, Nonsjis_data + 164, Nonsjis_data + 166 },
	{ FIX_VALUE, 0x9c, Nonsjis_data + 165, Nonsjis_data + 167 },
	{ FUNC_LEAF, comment_display, NULL, NULL },
	{ FIX_VALUE, 'x', Nonsjis_data + 172, Nonsjis_data + 169 },
	{ VAR_SIZE_TIL, 0, Nonsjis_data + 168, Nonsjis_data + 170 },
	{ FIX_VALUE, 0x9c, Nonsjis_data + 169, Nonsjis_data + 171 },
	{ FUNC_LEAF, charset_record_help, NULL, NULL },
	{ FIX_VALUE, 'p', Nonsjis_data + 176, Nonsjis_data + 173 },
	{ VAR_SIZE_TIL, 0, Nonsjis_data + 172, Nonsjis_data + 174 },
	{ FIX_VALUE, 0x9c, Nonsjis_data + 173, Nonsjis_data + 175 },
	{ FUNC_LEAF, charset_copy, NULL, NULL },
	{ FIX_VALUE, 'J', Nonsjis_data + 159, Nonsjis_data + 177 },
	{ VAR_SIZE_TIL, 0, Nonsjis_data + 176, Nonsjis_data + 178 },
	{ FIX_VALUE, 0x9c, Nonsjis_data + 177, Nonsjis_data + 179 },
	{ FUNC_LEAF, job_start, NULL, NULL },
	{ FIX_VALUE, 0x8f, Nonsjis_data + 182, Nonsjis_data + 181 },
	{ FUNC_LEAF, ss3_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8e, Nonsjis_data + 184, Nonsjis_data + 183 },
	{ FUNC_LEAF, ss2_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8d, Nonsjis_data + 186, Nonsjis_data + 185 },
	{ FUNC_LEAF, reverse_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8c, Nonsjis_data + 188, Nonsjis_data + 187 },
	{ FUNC_LEAF, reverse_half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8b, Nonsjis_data + 190, Nonsjis_data + 189 },
	{ FUNC_LEAF, half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8a, Nonsjis_data + 192, Nonsjis_data + 191 },
	{ FUNC_LEAF, set_vtab, NULL, NULL },
	{ FIX_VALUE, 0x88, Nonsjis_data + 194, Nonsjis_data + 193 },
	{ FUNC_LEAF, set_htab, NULL, NULL },
	{ FIX_VALUE, 0x85, Nonsjis_data + 196, Nonsjis_data + 195 },
	{ FUNC_LEAF, crlf_cntl, NULL, NULL },
	{ FIX_VALUE, ' ', NULL, Nonsjis_data + 197 },
	{ FUNC_LEAF, sp_cntl, NULL, NULL }
};





ESCSEQ_DATA	testjis_data[280] = {
	{ FIX_VALUE, 0x7f, testjis_data + 2, testjis_data + 1 },
	{ FUNC_LEAF, del_cntl, NULL, NULL },
	{ FIX_VALUE, ' ', testjis_data + 4, testjis_data + 3 },
	{ FUNC_LEAF, sp_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1d, testjis_data + 6, testjis_data + 5 },
	{ FUNC_LEAF, ss3_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1b, testjis_data + 250, testjis_data + 7 },
	{ FIX_VALUE, '~', testjis_data + 9, testjis_data + 8 },
	{ FUNC_LEAF, ls1r_cntl, NULL, NULL },
	{ FIX_VALUE, '}', testjis_data + 11, testjis_data + 10 },
	{ FUNC_LEAF, ls2r_cntl, NULL, NULL },
	{ FIX_VALUE, '|', testjis_data + 13, testjis_data + 12 },
	{ FUNC_LEAF, ls3r_cntl, NULL, NULL },
	{ FIX_VALUE, 'o', testjis_data + 15, testjis_data + 14 },
	{ FUNC_LEAF, ls3_cntl, NULL, NULL },
	{ FIX_VALUE, 'n', testjis_data + 17, testjis_data + 16 },
	{ FUNC_LEAF, ls2_cntl, NULL, NULL },
	{ FIX_VALUE, 'c', testjis_data + 19, testjis_data + 18 },
	{ FUNC_LEAF, hard_reset, NULL, NULL },
	{ FIX_VALUE, '[', testjis_data + 177, testjis_data + 20 },
	{ ASCII_ARGS, ';', testjis_data + 19, testjis_data + 21 },
	{ FIX_VALUE, '}', testjis_data + 23, testjis_data + 22 },
	{ FUNC_LEAF, gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', testjis_data + 25, testjis_data + 24 },
	{ FUNC_LEAF, gridline_start, NULL, NULL },
	{ FIX_VALUE, 'z', testjis_data + 27, testjis_data + 26 },
	{ FUNC_LEAF, memory_free, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 29, testjis_data + 28 },
	{ FUNC_LEAF, ctype_select, NULL, NULL },
	{ FIX_VALUE, 'x', testjis_data + 31, testjis_data + 30 },
	{ FUNC_LEAF, pos_store_restore, NULL, NULL },
	{ FIX_VALUE, 'w', testjis_data + 33, testjis_data + 32 },
	{ FUNC_LEAF, proportional_offset, NULL, NULL },
	{ FIX_VALUE, 'v', testjis_data + 35, testjis_data + 34 },
	{ FUNC_LEAF, copy_number, NULL, NULL },
	{ FIX_VALUE, 'u', testjis_data + 37, testjis_data + 36 },
	{ FUNC_LEAF, margin_clear, NULL, NULL },
	{ FIX_VALUE, 't', testjis_data + 39, testjis_data + 38 },
	{ FUNC_LEAF, margin_set, NULL, NULL },
	{ FIX_VALUE, 's', testjis_data + 41, testjis_data + 40 },
	{ FUNC_LEAF, frame_mesh_start, NULL, NULL },
	{ FIX_VALUE, 'r', testjis_data + 43, testjis_data + 42 },
	{ FUNC_LEAF, frame_mesh_end, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 45, testjis_data + 44 },
	{ FUNC_LEAF, paper_supply, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 47, testjis_data + 46 },
	{ FUNC_LEAF, page_format_select, NULL, NULL },
	{ FIX_VALUE, 'n', testjis_data + 49, testjis_data + 48 },
	{ FUNC_LEAF, status_request, NULL, NULL },
	{ FIX_VALUE, 'm', testjis_data + 51, testjis_data + 50 },
	{ FUNC_LEAF, attr_chg, NULL, NULL },
	{ FIX_VALUE, 'l', testjis_data + 53, testjis_data + 52 },
	{ FUNC_LEAF, multi_func_off, NULL, NULL },
	{ FIX_VALUE, 'k', testjis_data + 55, testjis_data + 54 },
	{ FUNC_LEAF, up_move, NULL, NULL },
	{ FIX_VALUE, 'j', testjis_data + 57, testjis_data + 56 },
	{ FUNC_LEAF, left_move, NULL, NULL },
	{ FIX_VALUE, 'h', testjis_data + 59, testjis_data + 58 },
	{ FUNC_LEAF, multi_func_on, NULL, NULL },
	{ FIX_VALUE, 'g', testjis_data + 61, testjis_data + 60 },
	{ FUNC_LEAF, tab_clr, NULL, NULL },
	{ FIX_VALUE, 'f', testjis_data + 63, testjis_data + 62 },
	{ FUNC_LEAF, abs_vh_move, NULL, NULL },
	{ FIX_VALUE, 'e', testjis_data + 65, testjis_data + 64 },
	{ FUNC_LEAF, down_move, NULL, NULL },
	{ FIX_VALUE, 'd', testjis_data + 67, testjis_data + 66 },
	{ FUNC_LEAF, abs_v_move, NULL, NULL },
	{ FIX_VALUE, 'a', testjis_data + 69, testjis_data + 68 },
	{ FUNC_LEAF, right_move, NULL, NULL },
	{ FIX_VALUE, '`', testjis_data + 71, testjis_data + 70 },
	{ FUNC_LEAF, abs_h_move, NULL, NULL },
	{ FIX_VALUE, '/', testjis_data + 90, testjis_data + 72 },
	{ FIX_VALUE, '~', testjis_data + 74, testjis_data + 73 },
	{ FUNC_LEAF, decolated_gridline_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'z', testjis_data + 76, testjis_data + 75 },
	{ FUNC_LEAF, macro_record_hex, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 78, testjis_data + 77 },
	{ FUNC_LEAF, mesh_pattern_record_hex, NULL, NULL },
	{ FIX_VALUE, 'v', testjis_data + 80, testjis_data + 79 },
	{ FUNC_LEAF, ctrl_print_hex, NULL, NULL },
	{ FIX_VALUE, 't', testjis_data + 82, testjis_data + 81 },
	{ FUNC_LEAF, wiredot_image_draw2_hex, NULL, NULL },
	{ FIX_VALUE, 's', testjis_data + 84, testjis_data + 83 },
	{ FUNC_LEAF, wiredot_image_draw1_hex, NULL, NULL },
	{ FIX_VALUE, 'r', testjis_data + 86, testjis_data + 85 },
	{ FUNC_LEAF, raster_image_draw_hex, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 88, testjis_data + 87 },
	{ FUNC_LEAF, onechar_record_hex, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 71, testjis_data + 89 },
	{ FUNC_LEAF, charset_record_hex, NULL, NULL },
	{ FIX_VALUE, '.', testjis_data + 109, testjis_data + 91 },
	{ FIX_VALUE, '~', testjis_data + 93, testjis_data + 92 },
	{ FUNC_LEAF, decolated_gridline_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'z', testjis_data + 95, testjis_data + 94 },
	{ FUNC_LEAF, macro_record, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 97, testjis_data + 96 },
	{ FUNC_LEAF, mesh_pattern_record, NULL, NULL },
	{ FIX_VALUE, 'v', testjis_data + 99, testjis_data + 98 },
	{ FUNC_LEAF, ctrl_print, NULL, NULL },
	{ FIX_VALUE, 't', testjis_data + 101, testjis_data + 100 },
	{ FUNC_LEAF, wiredot_image_draw2, NULL, NULL },
	{ FIX_VALUE, 's', testjis_data + 103, testjis_data + 102 },
	{ FUNC_LEAF, wiredot_image_draw1, NULL, NULL },
	{ FIX_VALUE, 'r', testjis_data + 105, testjis_data + 104 },
	{ FUNC_LEAF, raster_image_draw, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 107, testjis_data + 106 },
	{ FUNC_LEAF, onechar_record, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 90, testjis_data + 108 },
	{ FUNC_LEAF, charset_record, NULL, NULL },
	{ FIX_VALUE, '&', testjis_data + 126, testjis_data + 110 },
	{ FIX_VALUE, '}', testjis_data + 112, testjis_data + 111 },
	{ FUNC_LEAF, vector_mode_in, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 114, testjis_data + 113 },
	{ FUNC_LEAF, comment_display3, NULL, NULL },
	{ FIX_VALUE, 'w', testjis_data + 116, testjis_data + 115 },
	{ FUNC_LEAF, overlay_print, NULL, NULL },
	{ FIX_VALUE, 'u', testjis_data + 118, testjis_data + 117 },
	{ FUNC_LEAF, allocate_image_area, NULL, NULL },
	{ FIX_VALUE, 't', testjis_data + 120, testjis_data + 119 },
	{ FUNC_LEAF, char_rotation, NULL, NULL },
	{ FIX_VALUE, 's', testjis_data + 122, testjis_data + 121 },
	{ FUNC_LEAF, overlaypage_record, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 124, testjis_data + 123 },
	{ FUNC_LEAF, onechar_copy, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 109, testjis_data + 125 },
	{ FUNC_LEAF, charset_delete, NULL, NULL },
	{ FIX_VALUE, '%', testjis_data + 147, testjis_data + 127 },
	{ FIX_VALUE, '}', testjis_data + 129, testjis_data + 128 },
	{ FUNC_LEAF, decolated_char_attr, NULL, NULL },
	{ FIX_VALUE, 'z', testjis_data + 131, testjis_data + 130 },
	{ FUNC_LEAF, restore_environment, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 133, testjis_data + 132 },
	{ FUNC_LEAF, backup_environment, NULL, NULL },
	{ FIX_VALUE, 'x', testjis_data + 135, testjis_data + 134 },
	{ FUNC_LEAF, macro_execute, NULL, NULL },
	{ FIX_VALUE, 'w', testjis_data + 137, testjis_data + 136 },
	{ FUNC_LEAF, print_point_offset, NULL, NULL },
	{ FIX_VALUE, 'v', testjis_data + 139, testjis_data + 138 },
	{ FUNC_LEAF, charset_assign_1, NULL, NULL },
	{ FIX_VALUE, 'u', testjis_data + 141, testjis_data + 140 },
	{ FUNC_LEAF, underline_attribute, NULL, NULL },
	{ FIX_VALUE, 't', testjis_data + 143, testjis_data + 142 },
	{ FUNC_LEAF, charset_search, NULL, NULL },
	{ FIX_VALUE, 'r', testjis_data + 145, testjis_data + 144 },
	{ FUNC_LEAF, page_rotation, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 126, testjis_data + 146 },
	{ FUNC_LEAF, startup_macro, NULL, NULL },
	{ FIX_VALUE, '$', testjis_data + 160, testjis_data + 148 },
	{ FIX_VALUE, '}', testjis_data + 150, testjis_data + 149 },
	{ FUNC_LEAF, decolated_gridline_end, NULL, NULL },
	{ FIX_VALUE, '{', testjis_data + 152, testjis_data + 151 },
	{ FUNC_LEAF, decolated_gridline_start, NULL, NULL },
	{ FIX_VALUE, 'u', testjis_data + 154, testjis_data + 153 },
	{ FUNC_LEAF, undo, NULL, NULL },
	{ FIX_VALUE, 's', testjis_data + 156, testjis_data + 155 },
	{ FUNC_LEAF, mesh_pattern, NULL, NULL },
	{ FIX_VALUE, 'q', testjis_data + 158, testjis_data + 157 },
	{ FUNC_LEAF, composed_print, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 147, testjis_data + 159 },
	{ FUNC_LEAF, pair_mode, NULL, NULL },
	{ FIX_VALUE, ' ', testjis_data + 20, testjis_data + 161 },
	{ FIX_VALUE, 'S', testjis_data + 163, testjis_data + 162 },
	{ FUNC_LEAF, tate_yoko, NULL, NULL },
	{ FIX_VALUE, 'L', testjis_data + 165, testjis_data + 164 },
	{ FUNC_LEAF, line_pitch_select, NULL, NULL },
	{ FIX_VALUE, 'K', testjis_data + 167, testjis_data + 166 },
	{ FUNC_LEAF, cpi_select, NULL, NULL },
	{ FIX_VALUE, 'I', testjis_data + 169, testjis_data + 168 },
	{ FUNC_LEAF, size_unit_select, NULL, NULL },
	{ FIX_VALUE, 'G', testjis_data + 171, testjis_data + 170 },
	{ FUNC_LEAF, spacing_value, NULL, NULL },
	{ FIX_VALUE, 'D', testjis_data + 173, testjis_data + 172 },
	{ FUNC_LEAF, charset_assign_number, NULL, NULL },
	{ FIX_VALUE, 'C', testjis_data + 175, testjis_data + 174 },
	{ FUNC_LEAF, siz_select, NULL, NULL },
	{ FIX_VALUE, 'B', testjis_data + 160, testjis_data + 176 },
	{ FUNC_LEAF, expand_char, NULL, NULL },
	{ FIX_VALUE, 'P', testjis_data + 204, testjis_data + 178 },
	{ ASCII_ARGS, ';', testjis_data + 177, testjis_data + 179 },
	{ FIX_VALUE, 'z', testjis_data + 184, testjis_data + 180 },
	{ VAR_SIZE_TIL, 0, testjis_data + 179, testjis_data + 181 },
	{ FIX_VALUE, 0x1b, testjis_data + 180, testjis_data + 182 },
	{ FIX_VALUE, '\\', testjis_data + 181, testjis_data + 183 },
	{ FUNC_LEAF, char_set_name_select, NULL, NULL },
	{ FIX_VALUE, 'y', testjis_data + 189, testjis_data + 185 },
	{ VAR_SIZE_TIL, 0, testjis_data + 184, testjis_data + 186 },
	{ FIX_VALUE, 0x1b, testjis_data + 185, testjis_data + 187 },
	{ FIX_VALUE, '\\', testjis_data + 186, testjis_data + 188 },
	{ FUNC_LEAF, comment_display, NULL, NULL },
	{ FIX_VALUE, 'x', testjis_data + 194, testjis_data + 190 },
	{ VAR_SIZE_TIL, 0, testjis_data + 189, testjis_data + 191 },
	{ FIX_VALUE, 0x1b, testjis_data + 190, testjis_data + 192 },
	{ FIX_VALUE, '\\', testjis_data + 191, testjis_data + 193 },
	{ FUNC_LEAF, charset_record_help, NULL, NULL },
	{ FIX_VALUE, 'p', testjis_data + 199, testjis_data + 195 },
	{ VAR_SIZE_TIL, 0, testjis_data + 194, testjis_data + 196 },
	{ FIX_VALUE, 0x1b, testjis_data + 195, testjis_data + 197 },
	{ FIX_VALUE, '\\', testjis_data + 196, testjis_data + 198 },
	{ FUNC_LEAF, charset_copy, NULL, NULL },
	{ FIX_VALUE, 'J', testjis_data + 178, testjis_data + 200 },
	{ VAR_SIZE_TIL, 0, testjis_data + 199, testjis_data + 201 },
	{ FIX_VALUE, 0x1b, testjis_data + 200, testjis_data + 202 },
	{ FIX_VALUE, '\\', testjis_data + 201, testjis_data + 203 },
	{ FUNC_LEAF, job_start, NULL, NULL },
	{ FIX_VALUE, 'O', testjis_data + 206, testjis_data + 205 },
	{ FUNC_LEAF, ss3_cntl, NULL, NULL },
	{ FIX_VALUE, 'N', testjis_data + 208, testjis_data + 207 },
	{ FUNC_LEAF, ss2_cntl, NULL, NULL },
	{ FIX_VALUE, 'M', testjis_data + 210, testjis_data + 209 },
	{ FUNC_LEAF, reverse_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'L', testjis_data + 212, testjis_data + 211 },
	{ FUNC_LEAF, reverse_half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'K', testjis_data + 214, testjis_data + 213 },
	{ FUNC_LEAF, half_lf_cntl, NULL, NULL },
	{ FIX_VALUE, 'J', testjis_data + 216, testjis_data + 215 },
	{ FUNC_LEAF, set_vtab, NULL, NULL },
	{ FIX_VALUE, 'H', testjis_data + 218, testjis_data + 217 },
	{ FUNC_LEAF, set_htab, NULL, NULL },
	{ FIX_VALUE, 'E', testjis_data + 220, testjis_data + 219 },
	{ FUNC_LEAF, crlf_cntl, NULL, NULL },
	{ FIX_VALUE, '?', testjis_data + 222, testjis_data + 221 },
	{ FUNC_LEAF, char_width_request, NULL, NULL },
	{ FIX_VALUE, '=', testjis_data + 224, testjis_data + 223 },
	{ FUNC_LEAF, parameter_reset, NULL, NULL },
	{ FIX_VALUE, '<', testjis_data + 226, testjis_data + 225 },
	{ FUNC_LEAF, soft_reset, NULL, NULL },
	{ FIX_VALUE, '+', testjis_data + 228, testjis_data + 227 },
	{ FUNC_LEAF, g3_gr, NULL, NULL },
	{ FIX_VALUE, '*', testjis_data + 230, testjis_data + 229 },
	{ FUNC_LEAF, g2_gr, NULL, NULL },
	{ FIX_VALUE, ')', testjis_data + 232, testjis_data + 231 },
	{ FUNC_LEAF, g1_gr, NULL, NULL },
	{ FIX_VALUE, '(', testjis_data + 234, testjis_data + 233 },
	{ FUNC_LEAF, g0_gr, NULL, NULL },
	{ FIX_VALUE, '%', testjis_data + 241, testjis_data + 235 },
	{ FIX_VALUE, '@', testjis_data + 237, testjis_data + 236 },
	{ FUNC_LEAF, text_mode_start, NULL, NULL },
	{ FIX_VALUE, '1', testjis_data + 239, testjis_data + 238 },
	{ FUNC_LEAF, emulation_in, NULL, NULL },
	{ FIX_VALUE, '0', testjis_data + 234, testjis_data + 240 },
	{ FUNC_LEAF, sjis_start, NULL, NULL },
	{ FIX_VALUE, '$', testjis_data + 6, testjis_data + 242 },
	{ FIX_VALUE, '+', testjis_data + 244, testjis_data + 243 },
	{ FUNC_LEAF, g3_gr2, NULL, NULL },
	{ FIX_VALUE, '*', testjis_data + 246, testjis_data + 245 },
	{ FUNC_LEAF, g2_gr2, NULL, NULL },
	{ FIX_VALUE, ')', testjis_data + 248, testjis_data + 247 },
	{ FUNC_LEAF, g1_gr2, NULL, NULL },
	{ FIX_VALUE, '(', testjis_data + 241, testjis_data + 249 },
	{ FUNC_LEAF, g0_gr2, NULL, NULL },
	{ FIX_VALUE, 0x19, testjis_data + 252, testjis_data + 251 },
	{ FUNC_LEAF, ss2_cntl, NULL, NULL },
	{ FIX_VALUE, 0x13, testjis_data + 254, testjis_data + 253 },
	{ FUNC_LEAF, xof_cntl, NULL, NULL },
	{ FIX_VALUE, 0x11, testjis_data + 256, testjis_data + 255 },
	{ FUNC_LEAF, xon_cntl, NULL, NULL },
	{ FIX_VALUE, 0xf, testjis_data + 258, testjis_data + 257 },
	{ FUNC_LEAF, si_cntl, NULL, NULL },
	{ FIX_VALUE, 0xe, testjis_data + 260, testjis_data + 259 },
	{ FUNC_LEAF, so_cntl, NULL, NULL },
	{ FIX_VALUE, 0xd, testjis_data + 262, testjis_data + 261 },
	{ FUNC_LEAF, cr_cntl, NULL, NULL },
	{ FIX_VALUE, 0xc, testjis_data + 264, testjis_data + 263 },
	{ FUNC_LEAF, ff_cntl, NULL, NULL },
	{ FIX_VALUE, 0xb, testjis_data + 266, testjis_data + 265 },
	{ FUNC_LEAF, vt_cntl, NULL, NULL },
	{ FIX_VALUE, 0xa, testjis_data + 268, testjis_data + 267 },
	{ FUNC_LEAF, lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x9, testjis_data + 270, testjis_data + 269 },
	{ FUNC_LEAF, ht_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8, testjis_data + 272, testjis_data + 271 },
	{ FUNC_LEAF, bs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x7, testjis_data + 274, testjis_data + 273 },
	{ FUNC_LEAF, bel_cntl, NULL, NULL },
	{ FIX_VALUE, 0x6, testjis_data + 276, testjis_data + 275 },
	{ FUNC_LEAF, ack_cntl, NULL, NULL },
	{ FIX_VALUE, 0x3, testjis_data + 278, testjis_data + 277 },
	{ FUNC_LEAF, etx_cntl, NULL, NULL },
	{ FIX_VALUE, 0x0, NULL, testjis_data + 279 },
	{ FUNC_LEAF, nul_cntl, NULL, NULL }
};







#define	MAXCODENUM	16

static struct	codeset_table{
	char	*codesetname;
	char	*tmpcodeset;
	char	*codesetout;
	char	*localename;
	int	in_val;
	int	lbpval;
	int	gr_val;
	int	initstate;
} Codeset_table[MAXCODENUM] = {
	/* default values */
	{ "IBM-932", "IBM-932", "IBM-932", "Ja_JP", 127, 127, 1, -1},
	{ NULL, NULL, NULL, NULL, 0, 0, 0, 0 }
};

static struct	codeval_table{
	char	*codesetname;
	int	value;
	int	gr_value;
} Codeval_table[] ={
	{ "IBM-932", 127, 1 },
	{ "JISX0208.1983-GL", 1, 1 },
	{ "JISX0208.1978-GL", 1, 0 },
	{ "IBM-eucJP", 2, 1 },
	{ "IBM-eucKR", 128, 1 },
	{ "IBM-eucTW", 129, 1 },
	{ "IBM-850",   130, 1 },
	{ "ISO8859-1", 131, 1 },
	{ "ISO8859-7", 132, 1 },
	{ "ISO8859-9", 133, 1 },
	{ NULL, 0 }
};


ESCSEQ_DATAP	Kanjimode_data;

int	codeset_init()
{
	int	i, ic, il, iv, io, is, it;
	int	j;
	struct codeval_table	*p;

	for( i = ic = il = iv = io = is = it = 0;
	  ic < Iconvnames.len && il < Localenames.len && is < Statevals.len &&
	  iv < Iconvindex.len && io < Iconvouts.len && it < Tmpcode.len &&
	  i < MAXCODENUM; i++ ){
		Codeset_table[i].codesetname = Iconvnames.ptr + ic;
		Codeset_table[i].tmpcodeset = Tmpcode.ptr + it;
		Codeset_table[i].codesetout  = Iconvouts.ptr + io;
		Codeset_table[i].localename  = Localenames.ptr + il;
		Codeset_table[i].lbpval = atoi( Iconvindex.ptr + iv );
		Codeset_table[i].initstate = atoi( Statevals.ptr + is );

		for( p = Codeval_table; p->codesetname; p++ ){
			if( !strcmp( p->codesetname, Iconvnames.ptr + ic ) ){
				Codeset_table[i].in_val = p->value;
				Codeset_table[i].gr_val = p->gr_value;
				break;
			}
		}

		if( p->codesetname == NULL ){
			Codeset_table[i].in_val = Codeval_table[0].value;
			Codeset_table[i].gr_val = Codeval_table[0].gr_value;
		}

		for( ; Iconvnames.ptr[ic]; ic++ ); ic++;
		for( ; Tmpcode.ptr[it]; it++ ); it++;
		for( ; Iconvouts.ptr[io]; io++ ); io++;
		for( ; Localenames.ptr[il]; il++ ); il++;
		for( ; Iconvindex.ptr[iv]; iv++ ); iv++;
		for( ; Statevals.ptr[is]; is++ ); is++;
	}

	for( i = 0; i < KO_code.len; ){
		Kanjimode_data = ftbl_const( Kanjimode_data, KO_code.ptr+i, kanji_out );
		for( ; KO_code.ptr[i]; i++ ); i++;
	}

	for( i = 0; i < KI_code.len; ){
		Kanjimode_data = ftbl_const( Kanjimode_data, KI_code.ptr+i, kanji_in );
		for( ; KI_code.ptr[i]; i++ ); i++;
	}
}


int	set_codeset_byname( char *name )
{
	int	i;


	if( ( Ccs = lc_init( Codepagename.ptr, Trans_in.ptr, Trans_out.ptr, Fontlist.ptr, Font_path.ptr ) ) == 0 ){
	    errorexit( MSG_BADCODEPAGE, "" );
	}

	for( i = 0; i < MAXCODENUM && Codeset_table[i].codesetname; i++ ){
		if( !strcmp( Codeset_table[i].codesetname, Ccs->proccodeset ) )	break;
	}

	if( i >= MAXCODENUM || Codeset_table[i].codesetname == NULL ){
		i = 0;
	}

	strcpy( Codepagename.ptr, Codeset_table[i].codesetname );
	Codepagename.len = strlen( Codeset_table[i].codesetname );
	Incodepageval = Codeset_table[i].in_val;
	Codepageval = Codeset_table[i].lbpval;
	GRsetval = Codeset_table[i].gr_val;
	
	Ccs->state = Codeset_table[i].initstate;
/*
	for( i = 0; i < MAXCODENUM && Codeset_table[i].codesetname; i++ ){
	    if( !strcmp( Codeset_table[i].codesetname, Ccs->proccodeset ) )
	    else
		Ccs->state = ST_NONE;
	}
*/
/* show_FONTinfo( Ccs->FIP );*/
/* show_CSinfo( Ccs->CSP );*/
/* show_CURcodeset( Ccs );*/

	page_init();
}

void	set_codeset_byval( int val, int gr_val )
{
	int	i;

	for( i = 0; i < MAXCODENUM && Codeset_table[i].codesetname; i++ ){
		if( Codeset_table[i].lbpval == val &&
		  gr_val > 0 && Codeset_table[i].gr_val == gr_val ){
			break;
		}
	}

	if( i >= MAXCODENUM || Codeset_table[i].codesetname == NULL ){
		i = 0;
	}

	strcpy( Codepagename.ptr, Codeset_table[i].codesetname );
	Codepagename.len = strlen( Codeset_table[i].codesetname );
	Incodepageval = Codeset_table[i].in_val;
	Codepageval = Codeset_table[i].lbpval;
	if( gr_val ) GRsetval = Codeset_table[i].gr_val;

	if( ( Ccs = lc_init( Codepagename.ptr, Trans_in.ptr, Trans_out.ptr, Fontlist.ptr, Font_path.ptr) ) == 0 ){
	    errorexit( MSG_BADCODEPAGE, "" );
	}
	Ccs->state = Codeset_table[i].initstate;
/*
# if xx
 show_FONTinfo( FI );
 show_CSinfo( CSI );
 show_CURcodeset( Ccs );
# endif *//* xx */

	page_init();
}


void	pputf( char *s, int a )
{
	static char	buff[512];

	sprintf( buff, s, a );
	pputs( buff );
}

int	option_validation()
{
/*
	pputf( "pagewidth = %d\n", Pagewidth );
	pputf( "pagelength = %d\n", Pagelength );
	pputf( "pageorient = %d\n", Pageorient );
	pputf( "lmargin = %d\n", Lmargin );
	pputf( "rmargin = %d\n", Rmargin );
	pputf( "tmargin = %d\n", Tmargin );
	pputf( "bmargin = %d\n", Bmargin );
	pputf( "pageend = %d\n", Pageend );
	pputf( "VMI = %d\n", VMI );
	pputf( "HMI = %d\n", HMI );
	pputf( "charw = %d\n", Charwidth );
	pputf( "charh = %d\n", Charheight );
	pputf( "emph = %d\n", Emphasize );
*/
}

int	page_init()
{
	int	i, j;

	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j++ ){
			Pair[i][j] = 0;
		}
	}

	if( Codepageval == 1 || Codepageval == 127 ){
		/* sjis or jis */
		Pair[0][1] = Pair[1][0] = 1;
	}

	Hmi[0] = HMI;
	Hmi[1] = Codepageval == 2 ? 2 * HMI : HMI;
	Hmi[2] = Codepageval == 2 ? HMI : 2 * HMI;
	Hmi[3] = Codepageval == 2 ? 2 * HMI : HMI;
}

int	margin_init()
{
	CAP_x = Lmargin;
	CAP_y = Tmargin;
}

int	tab_init()
{
	char	i;

	if( Htabval ){
		for( i = 0; i < Max_htabs && i < Pagewidth / Hmi[GL] / Htabval; i++ ){
			Htabs[i] = Lmargin + i * Htabval * Hmi[GL];
		}
		Htab_l = i;
	}else{
		Htab_l = 0;
	}

	if( Vtabval ){
		for( i = 0; i < Max_vtabs && i < Pagewidth / VMI / Vtabval; i++ ){
			Vtabs[i] = Tmargin + i * Vtabval * VMI;
		}
		Vtab_l = i;
	}else{
		Vtab_l = 0;
	}
}

int	tab_const()
{
	if( ( Htabs = calloc( sizeof( int ), Max_htabs + 2 ) ) == NULL ||
	   ( Vtabs = calloc( sizeof( int ), Max_vtabs + 2 ) ) == NULL ){
		errorexit( MSG_MEMALLOCERR, "" );
	}

	tab_init();
}

int	stored_pos_init()
{
	int	i;

	Spos_x[0] = Spos_x[1] = Spos_x[3] = 0;
	Spos_x[2] = Spos_x[4] = Pagewidth;
	Spos_y[0] = Dot_offset * Vres;
	Spos_y[1] = Spos_y[2] = 0;
	Spos_y[3] = Spos_y[4] = Pagelength;

	for( i = 5; i < SPosN; i++ ){
		Spos_x[i] = Spos_x[0];
		Spos_y[i] = Spos_y[0];
	}
}

int	stored_pos_const()
{
	if( SPosN < 5 )	SPosN = 5;

	if( ( Spos_x = calloc( sizeof( int ), SPosN ) ) == NULL ||
	   ( Spos_y = calloc( sizeof( int ), SPosN ) ) == NULL ){
		errorexit( MSG_MEMALLOCERR, "" );
	}

	stored_pos_init();
}

setsh_for_FF()
{
	sharevars._vincr = &Tmargin_dummy;
	sharevars._vincr_cmd = NUL_CNTL_CMD;
}

setsh_for_LF()
{
	sharevars._vincr = &VMI;
	sharevars._vincr_cmd = LF_CNTL_CMD;
}

setsharevars()
{
	Rvmi = - VMI;
	CAP_y_dummy = 0;/* CAP_y_dummy = CAP_y - Tmargin */
	Bmargin_dummy = ( Bmargin - Tmargin ) * 6 / 5;
	sharevars._pl = &Bmargin_dummy;
	sharevars._tmarg = &Zeroval;
	sharevars._bmarg = &Zeroval;
	sharevars._vpos = &CAP_y_dummy;
	sharevars._vtab_base = &Zeroval;
	setsh_for_FF();
	sharevars._vdecr = &Rvmi;
	sharevars._vdecr_cmd = RI_CNTL_CMD;
	sharevars._ff_cmd = FF_CNTL_CMD;
	sharevars._set_cmd = SWITCHMODE_CMD;
	sharevars._ff_at_eof = &Do_formfeed;
}

/*
void	setpage()
{
}

void	settabs()
{
}
*/
int	initialize()
{
	if( Init_printer ){
		cmdout( INIT_CMD );
		Job_in = 1;
		Init_printer = 1;	/* second time should be soft reset */
/*
		setpage();
		settabs();
*/
	}

	return( 0 );
}


#include	<stdio.h>
#define	NLCHARMAX	257

int	count;
int	trunc;

# include	<iconv.h>

static int	Convert_a_char( char *dest, char *src, int dn, int *snr, int udc_chk )
{
    char*   p;
    int     l;

    Ccsid = get_iFont( src, dest, Ccs );
    if( Ccsid < 0 ) 
	return( Ccsid );
    l = strlen( dest );
    if( l == 0 ) l++;
    return( l );
}


int	get_a_mbcx( int *ri, char *bx, int bxsize, int udc_chk )
{
	int	j, i, c, si;
	char	b[NLCHARMAX+1];

	if( IS_EUC( Incodepageval ) && Save_GL >= 0 ){
		switch( GL ){
		case 2:
			Back_buffer( SS2_VALUE );
			break;
		case 3:
			Back_buffer( SS3_VALUE );
			break;
		}
	}

	for( i = 0, bx[0] = 0; i < NLCHARMAX - 1; ){
		c = Read_buffer();

		if( c == EOF ){
			j = 0;
			break;
		}else{
			b[i] = c;
			i++;
			b[i] = 0;
			si = i;

			if( ( j = Convert_a_char( bx, b, bxsize, &i, udc_chk ) ) > 0 ){
				for( ; si > i; si-- )	Unread_buffer();
				bx[j] = 0;
				break;
			}else if( j == -1 && i < NLCHARMAX - 1 ){
				continue;
			}else if( ( j == -2 || j == -3 ) && i < NLCHARMAX - 1 ){
			        i = 0;
				bx[0] = 0;
				continue;
			}else{
				/* the first i bytes are invalid */
				/* abandon first i bytes */
				if( i > 0 ){
					for( ; i < si; i++ )	Unread_buffer();
					i = 0;
				}else{
					/* error */
					errorexit( MSG_FATAL, "" );
				}
			}
		}
	}
	*ri = i;
	return( j );
}

;
#define BACK_BUFFER_KANJI_IN_J78  {Back_buffer('@'); Back_buffer('$'); Back_buffer('\033');}
#define BACK_BUFFER_KANJI_OUT_J78 {Back_buffer('J'); Back_buffer('('); Back_buffer('\033');}
#define BACK_BUFFER_KANJI_IN_J83  {Back_buffer('B'); Back_buffer('$'); Back_buffer('\033');}
#define BACK_BUFFER_KANJI_OUT_J83 {Back_buffer('B'); Back_buffer('('); Back_buffer('\033');}

int	kanji_in()
{
	if( Ccs->state == ST_KO )
	{
		cmdout( LS2_CNTL_CMD );

		if( Save_GL < 0 ){
			GL = 2;
		}else{
			Save_GL = 2;
		}
		Ccs->state = ST_KI;
		if(!strcmp(Ccs->inputcodeset, "jis78")){
		    BACK_BUFFER_KANJI_IN_J78;
		}
		else{
		    BACK_BUFFER_KANJI_IN_J83;
		}
	}
}

int	kanji_out()
{

	if( Ccs->state == ST_KI )
	{
		cmdout( SI_CNTL_CMD );

		if( Save_GL < 0 ){
			GL = 0;
		}else{
			Save_GL = 0;
		}
		Ccs->state = ST_KO;
		if(!strcmp(Ccs->inputcodeset, "jis78")){
		    BACK_BUFFER_KANJI_OUT_J78;
		}
		else{
		    BACK_BUFFER_KANJI_OUT_J83;
		}

	}
}

lineout( FILE *fileptr )
{
	int	i, j, rc;
	char	wc[100];
	char	bx[NLCHARMAX+1];
	int	GX;
	Fontimg	f;
	void	putfont_lbp( Fontimg f, int hmi, int ch );


	startline();
	Setfp_buffer( fileptr );

	for( count = 0; !Eof_buffer(); ){
		rc = Ccs->state != ST_NONE ? tbl_search( Kanjimode_data, 0 ):0;
/*	        tbl_search( Kanjimode_data, 0 );*/
		i = tbl_search( Ttdata, 1 );
		if( i == 0 && !IS_SJIS( Incodepageval ) ) i = tbl_search( Nonsjis_data, 1 );

		switch( i ){
		case 0:
/*			if( rc ){
				Seek_buffer( rc );
				break;
			}
*/			/* 1 character conversion */
			j = get_a_mbcx( &i, bx, NLCHARMAX, 0 );

			if( j <= 0 ){ /* i >= NLCHARMAX or c == EOF */
/*				for( j = 0; j < i - 1; j++ ) Unread_buffer();
				if( i > 0 ) pput1( b[0] );*/
				return( count );
			}

			if( !trunc ){
				if( get_wid( Ccsid, Ccs ) == 1 ){
					GX = 0;
				}else{
					GX = Codepageval == 2 ? 1 : 2;
				}
				if( !CAP_nomove && CAP_x + Hmi[GX] > Rmargin ){
					/* Wrap */
					if( Wrap ){
						for( j = 0; j < i; j++ )	Unread_buffer();
						CAP_x = Lmargin;
						CAP_nomove = 0;
						PS_offset = 0;
						/* secret = 0, char_rotate = 0 */
						CAP_y += VMI;
						endline();
						return( count );
					}else{
						trunc = 1;
					}
				}else{
					CAP_x += Hmi[GX];
					f = get_Font( bx,get_pFont( Ccs, Ccsid ), get_wid( Ccsid, Ccs ), PR_PAD);
					if( f == 0 ){
						errorexit( MSG_FATAL, "" );
					}else if( f->w == SEND_CODEVALUE ){
						pputn( j, bx );
					}else{
						putfont_lbp( f, Hmi[GX],
							Charheight );
					}
				}
			}
			if( Save_GL >= 0 ){
				GL = Save_GL;
				Save_GL = -1;
			}
			break;
		case EOL1:
			CAP_y += VMI;
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


void putfont_lbp( Fontimg f, int hmi, int ch )
{
	int	h, i;
	int	dpi;


if( f != 0 ){
	dpi = (int)( f->h * Vres / ch );
	if( dpi < 60 ) dpi = 60;
	if( dpi > 600 ) dpi = 600;
	if( f->baseline < 0 ) f->baseline = 0;
	f->baseline++;	/* to balance with laser shot internal font */
	h = ch /*+ ( f->baseline * ch / f->h )*/;

	if( h != 0 ){
		Int0 = -h;
		cmdout( REL_YMOVE_CMD );
	}

	sprintf( Str0.ptr, "%d;%d;%d;0", f->bytes_in_line * f->h,
		f->bytes_in_line, dpi );
	Str0.len = strlen( Str0.ptr );
	cmdout( RAS_DRAW_B_CMD );
	pputn( f->bytes_in_line * f->h, f->img );

	if( h != 0 ){
		Int0 = h;
		cmdout( REL_YMOVE_CMD );
	}
}
	if( hmi != 0 ){
		Int0 = hmi;
		cmdout( REL_XMOVE_CMD );
	}
}


startline()
{
	setsh_for_LF();
	trunc = 0;
	if( CAP_x > Rmargin )	trunc = 1;
}

void endline()
{
	CAP_y_dummy = CAP_y - Tmargin;

	if( CAP_y >= Bmargin ){
		CAP_y_dummy = ( Bmargin - Tmargin ) * 2;
		CAP_y = Tmargin;
		if( FFaddCR ){
			CAP_x = Lmargin;
			CAP_nomove = 0;
			PS_offset = 0;
			/* secret = 0, char_rotate = 0 */
		}

		setsh_for_FF();
	}
}


extern int	count;


#if !defined( pioputchar )
void	pioputchar( int c )
{
	putchar( c );
}
#endif

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

void	cmdout( char *attr )
{
	piocmdout( attr, NULL, 0, NULL );
}


static int	Data_match( ESCSEQ_DATAP p );
static int	Elm_match( int c, ESCSEQ_DATAP p );
static int	Fixval_match( int c, int val );
static int	Tilsiz_match( int c, ESCSEQ_DATAP p );
static int	Fixsiz_match( int c, int siz, ESCSEQ_DATAP p );
static int	Varsiz_match( int c, OP_TOKENP p, ESCSEQ_DATAP q );
static int	Op_eval( OP_TOKENP p );
static int	Op_operate( char *op, int lval, int rval );

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
/*extern int	Dump_buffer( void );*/

static int	Clr_stack( void );
static int	Add_stack( int siz, char *p, ESCSEQ_DATAP q );
static int	Rem_stack( ESCSEQ_DATAP p );
static int	Get_escdata( ESCSEQ_DATAP p );
static int	Get_sizdata( ESCSEQ_VARP p );


static ESCSEQ_VARP	Var_root = NULL;
static ESCSEQ_VARP	*Var_cur = &Var_root;


static ESCSEQ_DATAP	Esc_from_str( char *seq, int (*func)() )
{
	int	i, n;
	ESCSEQ_DATAP	q;


	n = strlen( seq ) + 1;

	if( ( q = calloc( sizeof( ESCSEQ_DATA ), n ) ) == NULL ){
		
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


/*******
	flag == 1 : read buffer if matched
	flag == 0 : not read buffer even if matched
*******/
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
			if( flag == 0 )	rc = Seek_buffer_to_lockpoint();
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
	case ASCII_ARGS:
		return( Aarg_match( c, p->value, p ) );
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

static int	Aarg_match( int c, int sep, ESCSEQ_DATAP p )
{
	int	i, j;
	char	*x;
	ESCSEQ_DATAP	q;


	if( p->down == NULL || p->down->type != FIX_VALUE ) return( 0 );

	Seek_buffer( -1 );
	x = Current_buffer_p();

	for( i = j = 0;; i++ ){
		c = Read_buffer();

		for( q = p->down; q->type == FIX_VALUE && q != p; q = q->next ){
			if( c == q->value ){
				Unread_buffer();
				Add_stack( j, x, p );
				return( 1 );
			}
		}

		if( c >= '0' && c <= '9' ){
			j++;
		}else if( i == 0 && c == '?' ){
			Add_stack( 1, x, p );
			x = Current_buffer_p();
		}else if( c == sep ){
			Add_stack( j, x, p );
			j = 0;
			x = Current_buffer_p();
		}else{
			for( ; i > 0; i-- )	Unread_buffer();
			Rem_stack( p );
			return( 0 );
		}
	}
}

static int	Tilsiz_match( int c, ESCSEQ_DATAP p )
{
	int	i;
	char	*x;
	ESCSEQ_DATAP	q;


	if( p->down == NULL || p->down->type != FIX_VALUE ) return( 0 );

	Seek_buffer( -1 );
	x = Current_buffer_p();

	for( i = 0;; i++ ){
		c = Read_buffer();

		for( q = p->down; q->type == FIX_VALUE && q != p; q = q->next ){
			if( c == q->value ){
				Unread_buffer();
				Add_stack( i, x, p );
				return( 1 );
			}
		}

		if( c == EOF ){
			for( ; i > 0; i-- )	Unread_buffer();
			Rem_stack( p );
			return( 0 );
		}
	}
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
int	Access_buffer( char *rp, int offset )
{
	if( rp >= Ibuffer && rp < Ibuffer + IBUFF_SIZE ){
		offset += ( rp - Ibuffer ) + IBUFF_SIZE;
		return( Ibuffer[ offset % IBUFF_SIZE ] );
	}else{
		return( EOF );
	}
}

char	*Current_buffer_p()
{
	return( Ibcp );
}

/* check only Ibbp ( No Ibcp check ) */
int	Advance_Ibbp()
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
		return( 0 );
		errorexit( MSG_IBUFFULL, "" );
	}
}

int	Seek_buffer( int offset )
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

int	Seek_buffer_to_lockpoint()
{
	int	x;

	if( Iblp != NULL ){
		x = ( Ibcp - Iblp + IBUFF_SIZE ) % IBUFF_SIZE;
		Ibcp = Iblp;
		return( x );
	}else{
		return( 0 );
	}
}

int	Clr_buffer()
{
	Ibcp = Ibbp = Ibuffer;
	Iblp = NULL;
	Ibfp = stdin;
	return( 1 );
}

int	Setfp_buffer( FILE *fp )
{
	Ibfp = fp;
	return( 1 );
}

int	Lock_buffer_at_cp()
{
	Iblp = Ibcp;
	return( 1 );
}

int	Lock_buffer_at_bp()
{
	Iblp = Ibbp;
	return( 1 );
}

int	Unlock_buffer()
{
	Iblp = NULL;
	return( 1 );
}

int	Eof_buffer()
{
	return( Ibcp == Ibbp && feof( Ibfp ) );
}

int	Read_buffer()
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

int	Ref_buffer()
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

int	Unread_buffer()
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

int	Back_buffer( int c )
{
	if( Unread_buffer() ){
		*Ibcp = c;
		return( 1 );
	}else{
		return( 0 );
	}
}

int	Copy_buffer( char *dest, char *src_b, int bsiz )
{
	int	i;


	for( i = 0; i < bsiz; i++ ){
		dest[i] = Access_buffer( src_b, i );
	}

	dest[i] = 0;
	return( 1 );
}


/**********************************/

static int	Clr_stack()
{
	ESCSEQ_VARP	p, q;


	for( p = Var_root; p != NULL; ){
		q = p->next;
		free( p->p );
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

	if( ( (*Var_cur)->p = calloc( sizeof( char ), siz + 1 ) ) == NULL ){
		errorexit( MSG_MEMALLOCERR, "" );
	}

	Copy_buffer( (*Var_cur)->p, p, siz );
	(*Var_cur)->sz  = siz;
	(*Var_cur)->ref = q;
	Var_cur = &((*Var_cur)->next);
	return( 1 );
}


static int	R_n()
{
	ESCSEQ_VARP	p;
	int	i;


	for( i = 0, p = Var_root; p != NULL; i++ )	p = p->next;
	return( i );
}


static int	R_st( int i )
{
	ESCSEQ_VARP	p;

	for( p = Var_root; i > 0 && p != NULL; i-- )	p = p->next;

	if( i < 0 || p == NULL )	return( 0 );

	return( Get_sizdata( p ) );
}


static int	Rem_stack( ESCSEQ_DATAP p )
{
	ESCSEQ_VARP	x, xold, y, s;


	if( p != NULL && Var_root != NULL ){
		switch( p->type ){
		case	FIX_SIZE:
		case	VAR_SIZE:
		case	VAR_SIZE_TIL:
		case	ASCII_ARGS:
			for( x = Var_root; x->next != NULL; xold = x, x = x->next );
			if( x->ref == p ){
				for( y = x; y != NULL; ){
					s = y->next;
					free( y->p );
					free( y );
					y = s;
				}
				if( x == Var_root ){
					Var_root = NULL;
					Var_cur = &Var_root;
				}else{
					xold->next = NULL;
					Var_cur = &(xold->next);
				}

				return( 1 );
			}
		}
	}

	return( 0 );
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
	if( x == NULL || x->ref == NULL ) return( 0 );

	if( x->ref->type == ASCII_ARGS || x->ref->type == VAR_SIZE_TIL ){
		return( (int)x->p );
	}

	switch( x->sz ){
	case 0:
		return( 0 );
	case 1:
		return( (int)*(char *)x->p );
	case 2:
	case 3:
		return( (int)*(short *)x->p );
    defalut:
		return( (int)*(long *)x->p );
	}
}

int	passthru()
{
	int	c;

	while( ( c = piogetchar() ) != EOF )	pioputchar( c );
	if( Do_formfeed )	cmdout( FF_CNTL_CMD );
	return( 0 );
}

int	restore()
{
	if( Restoreprinter )	cmdout( REST_CMD );
	Job_in = 0;
	return( 0 );
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
#define PIOMSGCAT "jfmtrs.cat"
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
	piocmdout (FF_CNTL_CMD, NULL, 0, NULL);
    pioexit (PIOEXITBAD);
}

#endif /* AIX32 */
