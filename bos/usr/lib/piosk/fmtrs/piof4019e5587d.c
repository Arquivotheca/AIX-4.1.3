static char sccsid[] = "@(#)99  1.14  src/bos/usr/lib/piosk/fmtrs/piof4019e5587d.c, cmdpiosk, bos411, 9428A410j 4/6/94 04:02:54";
/*
 * COMPONENT_NAME: (CMDPIOSK)
 *
 * FUNCTIONS: setup, initialize, lineout, passthru, restore
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * Print Formatter For
 * [ IBM 5587 Model G01 Emulation with IBM LaserPrinter 4019 ]
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>

# define NLCHARMAX 257
# include       <nl_types.h>
# include       <langinfo.h>

#include "piostruct.h"
#include "jfmtrs_msg.h"
#include "lc.h"
#include "ffaccess.h"

/******************************************************************************
* Global Variables                                                            *
******************************************************************************/
CURcodeset	Ccs;
int	Ccsid;
int count;                                      /* Character Counter */
int hpos;                                       /* Hor. Position */
int vpos;                                       /* Ver. Position */
int trunc;                                      /* Truncate Flag */
int vtab_base;                                  /* Ver. Tab Base */
int cr_indent;                                  /* Indent by CR Dot Position */
struct shar_vars sharevars;
/******************************************************************************
* External Variables For User-Defined Font                                    *
*******************************************************************************/
/*int u_des;                            */      /* File Desc. For User Font */
int uuse = 0;                                   /* User Font Available Flag */
/*char *u_font = (char *)0;             */      /* User-defined Font File */
/*char *u_ptr = (char *)0;              */      /* ptr to UDF header */
/*snffontRec snffont;                   */      /* structure for snf font */
/*UdfInfoRec uinfo;                     */      /* structure for user font */

/******************************************************************************
* External Variables for Fonts                                                *
*******************************************************************************/
/*#define	EMUDEBUG*/			/* When debugging		*/
#if defined(EMUDEBUG)
FILE    *debfp;				 
#endif
/* Kanji */
char *u_font1 = (char *)0;                       /* Font File Name */
char *u_ptr1 = (char *)0;                        /* ptr to font file header */
UdfInfoRec uinfo1;                               /* structure for user font */
/* Kana */
char *u_font2 = (char *)0;                       /* Font File Name */
char *u_ptr2 = (char *)0;                        /* ptr to font file header */
UdfInfoRec uinfo2;                               /* structure for user font */
/* User */
char *u_font3 = (char *)0;                       /* User-defined Font File */
char *u_ptr3 = (char *)0;                        /* ptr to UDF header */
UdfInfoRec uinfo3;                               /* structure for user font */
/******************************************************************************
* Macro Definitions                                                           *
******************************************************************************/
int pglen;
#define SET_Pglen(lines)        {Pglen = lines;\
                                 pglen = lines * Vincr;}
int tmarg;
#define SET_Tmarg(lines)        {Tmarg = lines;\
                                 tmarg = lines * Vincr;}
int bmarg;
#define SET_Bmarg(lines)        {Bmarg = lines;\
                                 bmarg = lines * Vincr;}
#define SET_Lmarg(chars)        {Indent = chars;\
                                 lmarg = (chars * Hres * 10)/\
                                              (Condensed ? 180:(Pitch * 2));}

#define SET_Lpi(lpi)            {Lpi = lpi;\
                                 Vincr = Vdecr = (Vres * 10) / Lpi;}

                                /* lpi_10 = (actual line density) x 10   */
                                /* For example, if "6 lpi" is specified, */
                                /* lpi_10 is 0x3c (6 lpi x 10)           */

#define SET_Pgwidth(chars)      {Pgwidth = chars;\
                                 pgwidth = (chars * Hres * 10)/\
                                                (Condensed ? 180:(Pitch * 2));}

#define SET_Pitch(cpi)          {Pitch = cpi;\
                         Hincr = (Hres * 10)/(Condensed ? 180:(Pitch*2));\
                         Hincr *= (Doublewide ? 2 : 1); }

                                /* cpi_10 = (actual hankaku pitch) x 10 % 2  */
                                /* For example, if "10 cpi" is specified,    */
                                /* cpi_10 is 0x32 (10 lpi x 10 % 2)          */

/******************************************************************************
* Lookup Tables                                                               *
*******************************************************************************/
struct lktable lkup_bool[] = {                  /* BOOLEAN */
        "+"     ,1      ,
        "!"     ,0      ,
        NULL    ,NULL
};

struct lktable lkup_lpi[] = {                   /* LPI */
        "2"     ,0x14   ,
        "3"     ,0x1e   ,
        "4"     ,0x28   ,
        "5"     ,0x32   ,
        "6"     ,0x3c   ,
        "7.5"   ,0x4b   ,
        "8"     ,0x50   ,
        NULL    ,NULL
};

struct lktable lkup_cpi[] = {                   /* PITCH (CPI) */
        "10"    ,0x32   ,
        "12"    ,0x3c   ,
        "13.4"  ,0x43   ,
        "15"    ,0x4b   ,
        NULL    ,NULL
};

struct lktable lkup_fnt[] = {                   /* FONT STYLE */
        "gothic"        ,0x01   ,
        "elite"         ,0x06   ,
        "courier"       ,0x07   ,
        "m12"           ,0x08   ,
#if !defined( IBM5587G ) && !defined( IBM5587H ) && !defined( IBM4216K )
        "m10"           ,0x09   ,
#endif
#if defined( IBM5587G ) || defined( IBM5587H )
        "ocr"           ,0x11   ,
        "orator"        ,0x12   ,
#endif
        NULL            ,NULL
};
/*********************************************************************/
/*#if defined( IBM5587G ) || defined( IBM5587H )
struct lktable lkup_src[] = {
        "1"             ,1      ,
        "2"             ,2      ,
# if defined( IBM5587H )
        "0"             ,0xfd   ,
        "t"             ,0xfe   ,
        "d"             ,0xff   ,
# endif
        NULL            ,NULL
};
#endif*/

struct lktable lkup_src[] = {
        "1"             ,1      ,
        "2"             ,2      ,
        "0"             ,0      ,
        "3"             ,3      ,
        "4"             ,4      ,
        NULL            ,NULL
};
/*********************************************************************/
#if defined( IBM5587G ) || defined( IBM5587H ) || defined( IBM4216K )
struct lktable lkup_rot[] = {
        "0"             ,0      ,
# if defined( IBM5587H )
        "1"             ,1      ,
        "2"             ,2      ,
# endif
        "3"             ,3      ,
        NULL            ,NULL
};
#endif
struct lktable lkup_crlf[] = {
        "0"             ,0      ,
        "1"             ,1      ,
        "2"             ,2      ,
        NULL            ,NULL
};
#if defined( IBM5577 ) || defined( IBM5579 )
struct lktable lkup_phd[] = {
        "1"             ,1      ,
        "2"             ,2      ,
        "3"             ,3      ,
        NULL            ,NULL
};
#endif
/*****************************************************************************
* Selction of piof header file                                               *
******************************************************************************/
#if defined( IBM4208K ) || defined( IBM5572 ) || defined( IBM5575 )
#define DRIVER 5575
#include "piof5575d.h"                          /* For Native Mode */
#endif

#if defined( IBM5577 ) || defined( IBM5579 )
#define DRIVER 5577
#include "piof5577d.h"
#endif

#if defined( IBM4216K ) || defined( IBM5587G )
#define DRIVER 5587
#include "piof4019e5587d.h"
#endif

#if defined( IBM5587H )
#define DRIVER 5587
#endif

#if defined( IBM5327 )
#define DRIVER 5327
#include "piof5327d.h"
#endif
/************************************************/


/*****************************************************************/
#if !defined( ESCSEQ_DEF )
# define ESCSEQ_DEF

typedef struct escseq_var {
        int     sz;
        char    *p;
        struct escseq_data      *ref;
        struct escseq_var       *next;
} ESCSEQ_VAR, *ESCSEQ_VARP;


typedef enum escdata_type0 {
        FIX_VALUE, FIX_SIZE, VAR_SIZE, VAR_SIZE_TIL, FUNC_LEAF } escdata_type;

typedef struct  escseq_data {
        escdata_type    type;
        int     value;
        struct escseq_data      *next;
        struct escseq_data      *down;
#if defined( PREPRO )
        int     number;
        char    *name;
#endif  /* PREPRO */
} ESCSEQ_DATA, *ESCSEQ_DATAP;


typedef enum optoken_type0
    { OPERATOR, VALUE, ESCDATA } optoken_type;

typedef struct  op_token {
        optoken_type    type;
        int     value;
        struct op_token *lop;
        struct op_token *rop;
#if defined( PREPRO )
        int     number;
#endif /* PREPRO */
} OP_TOKEN, *OP_TOKENP;

extern int      kanji_in( void );
extern int      kanji_out( void );
extern ESCSEQ_DATAP     ftbl_const( ESCSEQ_DATAP root, char *seq, int (*func)() );
extern int      get_a_mbcx( int *ri, char *bx, int maxbx );
extern int      tbl_search( ESCSEQ_DATAP esc_root, int flag );
extern void     linefeed( void );
extern void     endline( void );
extern void     cmdout( char *attr );
extern void     pput1( int c );
extern void     pput2( int c );
extern void     pput4( int c );
extern void     pputn( int n, char *p );
extern void     pputs( char *p );


#define EOL1    (-2)
#define EOL2    (-3)
#define EOL             (-4)
#define NORMAL  (-5)

#endif  /* ESCSEQ_DEF */


/********************
Action function def.
********************/

int     nop()
{
/*      fprintf( stderr, "matched\n" );*/
        return( NORMAL );
}

/* commonly called by action functions */
static void     Data_through( int n )
{
        int     i, c;

        Unlock_buffer();

        for( i = 0; i < n; i++ ){
                c = Read_buffer();
                pput1( c );
        }
}

/* commonly called by action functions */
static void     Data_through_dum( int n )
{
        int     i;

        Unlock_buffer();

        for( i = 0; i < n; i++ ) Read_buffer();
}

/* commonly called by action functions */
static void     Data_after_esc_dum()
{
        Data_through_dum( R_st( 0 ) );
}


/* commonly called by action functions */
/* attr ch cl xx xx ... */
static void     Data_after_esc( char *attr )
{
        int     i, c, n = R_st( 0 );

        cmdout( attr );
        pput2( n );
        Data_through( n );
}


int     null_cntl()
{
        cmdout( NULL_CMD );
        return( NORMAL );
}

int     bel_cntl()
{
        cmdout( BEL_CMD );
        return( NORMAL );
}

int     bs_cntl()
{
        if( hpos == lmarg ) return( NORMAL );
        hpos -= Hincr;
        if( ( hpos < Pgwidth ) && trunc ) trunc--;
        cmdout( BS_CMD );
        return( NORMAL );
}

int     ht_cntl()
{
        int     i;
        int     tabval;
        char    *tabs;

        tabs = (&Horz_tabs)->ptr;
        for( i = 0; tabs[i] != 0; i++ ){
                tabval = ((int)tabs[i] - 1 ) * Hincr + lmarg;
                if( tabval > hpos ) break;
        }

        if( tabval > pgwidth ) tabval = pgwidth;
      /*cmdout( COLUMN_SKIP );            */
      /*pput1( 1 );                       */
      /*pput1( (tabval - hpos ) / Hincr );*/
        cmdout( SET_CURPOS_CMD );
        pput2 ( (tabval - hpos ) );
        pput2 ( 0 );
        while( hpos < tabval ) hpos += Hincr;
        return( NORMAL );
}

int     lf_cntl()
{
        if( Auto_crlf == 2 ){
                Hpos_from_prev = 0;
        }else{
                Hpos_from_prev = hpos;
        }

        linefeed();
        return( EOL );
}

int     vt_cntl()
{
        int     i;
        int     tabval;
        char    *tabs;


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

int     ff_cntl()
{
        vpos = pglen;
        endline();
        return( EOL );
}

int     cr_cntl()
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

int     fs_cntl()
{
        int     i;

      /*cmdout( IMG_HEIGHT_CMD );     */
      /*cmdout( IMG_TRSMIT_CMD );     */
      /*pput2( img_w );               */
      /*Data_through( img_h * img_w );*/
	/*fprintf(debfp, "fs_cntl img_h %x img_w %x\n",img_h,img_w);*/
        Data_through_dum( img_h * img_w );
        i = img_h;
        img_h = 3;
      /*cmdout( IMG_HEIGHT_CMD );     */
        img_h = i;
        return( NORMAL );
}


/* this is called by image_dataset() and ex_image_dataset() */
static int      Img_data_set_sub( char *attr, int m )
{
        int     i, c;

        img_w = R_st( 0 );
      /*cmdout( IMG_HEIGHT_CMD );         */
      /*cmdout( attr );                   */
      /*pput2( img_w );                   */
      /*Data_through( img_w * img_h );    */
	if ( Esc4019 ) {
        	Data_through( img_w * img_h );    
	} else {
        	Data_through_dum( img_w * img_h );
	}
#if defined( IBM5587G ) || defined( IBM5587H )
        if( img_h == 2 )        m *= 2;
#endif
        hpos += m * img_w;
        i = img_h;
        img_h = 3;
      /*cmdout( IMG_HEIGHT_CMD );         */
        img_h = i;
        return( NORMAL );
}

int     image_data_set()
{
        return( Img_data_set_sub( IMG_TRSMIT_CMD, 8 ) );
}

int     ex_image_data_set()
{
        return( Img_data_set_sub( IMG_TRSENL_CMD, 16 ) );
}

int     fw_hskip()
{
        int     i;

      /*cmdout( FRD_HORSKP_CMD );*/
      /*pput2( i = R_st( 0 ) );  */
        i = R_st(0);
        cmdout( SET_CURPOS_CMD );
        pput2 ( i*8 );
        pput2 (  0  );
        hpos += i * 8;
        return( NORMAL );
}

int     bw_hskip()
{
        int     i;

      /*cmdout( BWD_HORSKP_CMD );*/
      /*pput2( i = R_st( 0 ) );  */
        i = R_st(0);
        cmdout( SET_CURPOS_CMD );
        pput2 ( -i*8 );
        pput2 (   0  );

        if( hpos - i * 8 < lmarg ){
                hpos = lmarg;
        }else{
                hpos -= i * 8;
        }

        if( hpos < pgwidth && trunc ) trunc--;
        return( NORMAL );
}

int     fw_vskip()
{
        int     i = R_st( 0 );

      /*if( i >= 1 && i <= 0xff ){                        */
      /*        vpos += i * 12;                           */
      /*        Workint = 12;                             */
      /*        Hpos_from_prev = hpos;                    */
      /*        sharevars._vincr = &Workint;              */
      /*        sharevars._vincr_cmd = (&Vincr_cmd2)->ptr;*/
      /*        return( EOL2 );                           */
      /*}else{                                            */
                return( NORMAL );
      /*}                                                 */
}

int     cr_pset()
{
        cmdout( CR_CMD );
        cr_indent = R_st( 0 ) * 8;
        return( EOL2 );
}

int     bw_vskip()
{
#if !defined( IBM5327 )
        int     i = R_st( 0 );

      /*if( i >= 1 && i <= 0x28 ){                    */
      /*        vpos -= i * 12;                       */
      /*        if( vpos < 0 ) vpos = 0;              */
      /*        Workint = 12;                         */
      /*        Hpos_from_prev = hpos;                */
      /*        sharevars._vdecr = &Workint;          */
      /*        sharevars._vdecr = (&Vdecr_cmd2)->ptr;*/
      /*        return( EOL2 );                       */
      /*}                                             */
#endif
        return( NORMAL );
}

int     def_lspacing()
{
        int     i = R_st( 0 );

      /*if( i >= 1 && i <= 0x3c ){       */
      /*        cmdout( SET_LININC_CMD );*/
      /*        pput2( i );              */
      /*        Vincr = 12 * i;          */
      /*}                                */

        return( NORMAL );
}

int     bidir_mode()
{
#if DRIVER != 5587 && DRIVER != 5327
      /*Unidirect = FALSE;    */
      /*cmdout( SET_DIR_CMD );*/
#endif
        return( NORMAL );
}

int     unidir_mode()
{
#if DRIVER != 5587 && DRIVER != 5327
      /*Unidirect = TRUE;     */
      /*cmdout( SET_DIR_CMD );*/
#endif
        return( NORMAL );
}

int     set3tr()
{
    if (Ccs->state == JIS){
	Unread_buffer();
	Unread_buffer();
	return(0);
    }
    else{
        img_h = 3;
        return( NORMAL );
    }
}

int     set2tr()
{
        img_h = 2;
        return( NORMAL );
}

int     def_pglen()
{
        return( nop() );
}

int     hspeed_mode()
{
#if DRIVER != 5587
      /*Highspeed = TRUE;     */
      /*cmdout( SET_SPD_CMD );*/
#endif
        return( NORMAL );
}

int     lspeed_mode()
{
#if DRIVER != 5587
      /*Highspeed = FALSE;    */
      /*cmdout( SET_SPD_CMD );*/
#endif
        return( NORMAL );
}

int     cutform_in()
{
        vpos = pglen;
        return( EOL1 );
}

int     cutform_out()
{
        return( cutform_in() );
}

int     large_in()
{
      /*Doublewide = TRUE;     */
      /*Doublehigh = FALSE;    */
      /*SET_Pitch( Pitch );    */
      /*cmdout( SET_ENLG_CMD );*/
        return( NORMAL );
}

int     large_out()
{
      /*Doublewide = FALSE;    */
      /*Doublehigh = FALSE;    */
      /*SET_Pitch( Pitch );    */
      /*cmdout( SET_ENLG_CMD );*/
        return( NORMAL );
}

int     read_pr()
{
        return( nop() );
}

int     init_pr()
{
        return( nop() );
}

int     char_pitch()
{
        int     i = R_st( 0 );

        if( i == 0x32 || i == 0x3c || i == 0x43 || i == 0x4b ){
              /*set_Pitch( i );       */
              /*cmdout( CHG_CPI_CMD );*/
        }

        return( NORMAL );
}

int     line_pitch()
{
        int     i = R_st( 0 );

      /*if( i == 0x14 || i == 0x1e || i == 0x28 || i == 0x32 ||	*/
      /*   i == 0x3c || i == 0x4b || i == 0x50 ){		*/
      /*        set_Lpi( i );					*/
      /*        cmdout( CHG_LPI_CMD );				*/
      /*}							*/

        return( NORMAL );
}

int     ch_pglen_6()
{
        return( nop() );
}

int     ch_pglen_l()
{
        return( nop() );
}

int     ch_pglen_i()
{
        return( nop() );
}

int     font_style()
{
        int     i = R_st( 0 );

        if( i == 0 || i == 1 || ( i >= 6 && i <= 8 )
#if !defined( IBM5587G ) && !defined( IBM5587H ) && !defined( IBM4216K )
           || i == 9
#endif
#if defined( IBM5587G ) || defined( IBM5587H )
           || i == 11 || i == 12
#endif
        ){
      /*        Typestyle = i;          */
      /*        cmdout( CHG_FONT_CMD ); */
        }

        return( NORMAL );
}

int     tr_print()
{
        int     i, j, k, n = R_st( 0 );
        char    bx[NLCHARMAX+1];


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
                              /*cmdout( PRTALL_CMD );*/
                              /*pput1( j );          */
                              /*pputn( j, bx );      */
                                hpos += Hincr;
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

int     ch_hmove()
{
        return( nop() );
}

int     ch_vmove()
{
        return( nop() );
}

int     condense_in()
{
      /*Condensed = TRUE;     */
      /*set_Pitch( Pitch );   */
      /*cmdout( SET_CND_CMD );*/
        return( NORMAL );
}

int     condense_out()
{
      /*Condensed = FALSE;    */
      /*set_Pitch( Pitch );   */
      /*cmdout( SET_CND_CMD );*/
        return( NORMAL );
}

int     vprint_in()
{
      /*Vertical = TRUE;       */
      /*cmdout( SET_VERT_CMD );*/
        return( NORMAL );
}

int     vprint_out()
{
      /*Vertical = FALSE;      */
      /*cmdout( SET_VERT_CMD );*/
        return( NORMAL );
}

int     superscr_in()
{
      /*Superscr_mode = TRUE;    */
      /*Subscr_mode = FALSE;     */
      /*cmdout( SET_SCRIPT_CMD );*/
        return( NORMAL );
}

int     subscr_in()
{
      /*Superscr_mode = FALSE;   */
      /*Subscr_mode = TRUE;      */
      /*cmdout( SET_SCRIPT_CMD );*/
        return( NORMAL );
}

int     script_out()
{
      /*Superscr_mode = FALSE;   */
      /*Subscr_mode = FALSE;     */
      /*cmdout( SET_SCRIPT_CMD );*/
        return( NORMAL );
}

int     errdetect_in()
{
        return( nop() );
}

int     errdetect_out()
{
        return( nop() );
}

int     halfline_up()
{
      /*Workint = Vincr / 2;                      */
      /*vpos -= Workint;                          */
      /*if( vpos < 0 ) vpos = 0;                  */
      /*Hpos_from_prev = hpos;                    */
      /*sharevars._vdecr = &Workint;              */
      /*sharevars._vdecr_cmd = (&Vdecr_cmd3)->ptr;*/
      /*return( EOL2 );                           */
        return( NORMAL );
}

int     halfline_down()
{
      /*Workint = Vincr / 2;                      */
      /*vpos += Workint;                          */
      /*if( vpos > pglen ) vpos = pglen;          */
      /*Hpos_from_prev = hpos;                    */
      /*sharevars._vincr = &Workint;              */
      /*sharevars._vincr_cmd = (&Vincr_cmd3)->ptr;*/
      /*return( EOL2 );                           */
        return( NORMAL );
}

int     emphasize_in()
{
      /*Emphasized = TRUE;     */
      /*cmdout( SET_ENPH_CMD );*/
        return( NORMAL );
}

int     emphasize_out()
{
      /*Emphasized = FALSE;    */
      /*cmdout( SET_ENPH_CMD );*/
        return( NORMAL );
}

int     dstrike_in()
{
#if DRIVER != 5587 && DRIVER != 5327
      /*Doublestrike = TRUE;     */
      /*cmdout( SET_DBLSTR_CMD );*/
#endif
        return( NORMAL );
}

int     dstrike_out()
{
#if DRIVER != 5587 && DRIVER != 5327
      /*Doublestrike = FALSE;    */
      /*cmdout( SET_DBLSTR_CMD );*/
#endif
        return( NORMAL );
}

int     pr_complete()
{
        return( NORMAL );
}

int     ch_form_hdl()
{
        return( NORMAL );
}

int     underline_mode()
{
#if defined( IBM5587H )
        Cont_undrscore = R_st( 0 ) & 7;
#else
        Cont_undrscore = R_st( 0 ) & 3;
#endif
        cmdout( UNDERSCORE_CMD );
}

int     ch_datastream()
{
        return( nop() );
}

int     overstrike_cntl()
{
        int     n = R_st( 0 );
        int     osf = R_st( 1 );
        int     c = R_st( 3 );
        int     j, i;
        char    bx[NLCHARMAX+1];



        /*switch( n ){*/
        /*case 1:*/
                /*if( osf == 0 ){*/
                        /*Ovrstrike_flag = n;*/
                        /*Ovrstrike_mode = osf;*/
                        /*Ovrstrike_code = 0;*/
                      /*cmdout( OVERSTRIKE_CMD );*/
                /*}*/
                /*break;*/
        /*case 4:*/
        /*case 3:*/
                /*Seek_buffer( 2 - n );*/
                /*Ovrstrike_mode = osf;*/
/* here will come the code set conversion */
                /*if( ( j = get_a_mbcx( &i, bx, NLCHARMAX ) ) <= 0 ){*/
                        /*if( Eof_buffer() ){*/
                                /*return( EOF );*/
                        /*}else{*/
                                /*break;*/
                        /*}*/
                /*}*/
                /*Ovrstrike_flag = 2 + j;*/
                /*Ovrstrike_code = *(long *)bx;*/
              /*cmdout( OVERSTRIKE_CMD );*/
              /*pputn( j, bx );          */
                /*break;*/
        /*}*/

        return( NORMAL );
}

int     gridline()
{
        int     n, c, i;

        n = R_st(0);
        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x78 );                 /* save x,y to 30 */
        for (i=1 ; i<n; i++){
            c = Read_buffer();
	    if ( hspace( Hincr *  i ) ){
                if ( (c & 0x0f) != 0x00 ){
                    vgridout();             /* vertical gridline    */
                }
                if ( (c & 0xf0) != 0x00 ){
                    hgridout();             /* horizontal gridline & next */
                }else{
                    pput1( 0x20 );          /* next position        */
                }
	    }
        }
        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x7b );                 /* restore x,y from 30 */

#if DRIVER != 5575
/*     Data_after_esc( GRID_LINE_CMD ); */
#else
/*      Data_after_esc_dum();           */
#endif
        return( NORMAL );
}

int     clr_htab()
{
        char    *tabs = (&Horz_tabs)->ptr;

        tabs[0] = 0;
        return( NORMAL );
}

int     default_htab()
{
        int     i;
        char    *tabs = (&Horz_tabs)->ptr;

        for( i = 0; i < Max_htabs; i++ ){
                tabs[i] = 8 * i + 1;
        }

        tabs[Max_htabs] = 0;
        return( NORMAL );
}

/* this is called by set_htab() and set_vtab() */
static int      Set_tabs( char *tabs, int maxn )
{
        int     i, j, c, c0, n = R_st( 0 );


        Unlock_buffer();

        for( i = 0, j = 0, c0 = 0; i < n; i++, c0 = c ){
                c = Read_buffer();
                if( j < maxn && c > c0 )        tabs[ j++ ] =  c;
        }

        tabs[j] = 0;
        return( NORMAL );
}

int     set_htab()
{
        char    *tabs = (&Horz_tabs)->ptr;

        return( Set_tabs( tabs, Max_htabs ) );
}

int     clr_vtab()
{
        char    *tabs = (&Vert_tabs)->ptr;

        tabs[0] = 0;
        return( NORMAL );
}

int     set_vtab()
{
        char    *tabs = (&Vert_tabs)->ptr;

        return( Set_tabs( tabs, Max_vtabs ) );
}

int     set_hmargin()
{
        int     lm = R_st( 0 );
        int     rm = R_st( 1 );

        if( lm > 0 && rm >= lm + 4 ){
                lm--;
                set_Lmarg( lm );
                set_Pgwidth( rm );
        }else{
                return( NORMAL );
        }
}

int     set_skip_perforation()
{
        int     n = R_st( 0 );

        if( n > Pglen ){
                Tmarg = Bmarg = 0;
        }else if( n != 0 ){
                Tmarg = Perf_percent * n / 100;
                Bmarg = n - Tmarg;
        }

        return( NORMAL );
}

int     h_colskip0()
{
        cmdout( CR_CMD );
        Hpos_from_prev = lmarg + Hincr * R_st( 0 );
        if( Hpos_from_prev > pgwidth )  Hpos_from_prev = pgwidth;
        return( EOL2 );
}

int     h_colskip1()
{
        cmdout( CR_CMD );
        Hpos_from_prev = hpos + Hincr * R_st( 0 );
        if( Hpos_from_prev > pgwidth )  Hpos_from_prev = pgwidth;
        return( EOL2 );
}

int     h_colskip2()
{
        cmdout( CR_CMD );
        Hpos_from_prev = hpos - Hincr * R_st( 0 );
        if( Hpos_from_prev < lmarg ) Hpos_from_prev = lmarg;
        if( Hpos_from_prev < pgwidth && trunc ) trunc--;
        return( EOL2 );
}

int     h_colskipx()
{
        return( nop() );
}

int     v_lineskip0()
{
        return( nop() );
}

int     v_lineskip1()
{
        vpos += Vincr * R_st( 0 );
        if( pglen < vpos ) vpos = pglen;
        Hpos_from_prev = hpos;
        return( EOL2 );
}

int     v_lineskip2()
{
        return( nop() );
}

int     v_lineskipx()
{
        return( nop() );
}

int     char_pitch_1440()
{
        return( nop() );
}

int     line_pitch_1440()
{
        return( nop() );
}

int     char_scaling()
{
        int     n1 = R_st( 0 );
        int     n2 = R_st( 1 );

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
              /*Doublewide = FALSE;*/
              /*Doublehigh = TRUE; */
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
              /*Doublewide = TRUE; */
              /*Doublehigh = FALSE;*/
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
              /*Doublewide = Doublehigh = TRUE;*/
        }

      /*cmdout( CHG_SCALE_CMD );*/
      /*pput1( n1 );            */
      /*pput1( n2 );            */
      /*pput1( 2 );             */
        return( NORMAL );
}

int     gaij_load()
{
#if DRIVER != 5575
      /*Data_after_esc( GAIJI_LOAD_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     wr_img_cntl()
{
#if DRIVER == 5587
      /*Data_after_esc( WIMG_CNTL_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     wr_img()
{
#if DRIVER == 5587
      /*Data_after_esc( WRITE_IMG_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     load_font_cntl()
{
#if DRIVER == 5587
     /* cmdout( LDFNT_CNTL_CMD );*/
     /* pput1( R_st( 0 ) );      */
     /* pput2( R_st( 1 ) );      */
     /* pput2( R_st( 2 ) );      */
     /* pput1( R_st( 3 ) );      */
     /* pput1( R_st( 4 ) );      */
     R_st(0);R_st(1);R_st(2);R_st(3);R_st(4);
#endif
        return( NORMAL );
}

int     load_font_pattern()
{
#if DRIVER == 5587
      /*Data_after_esc( LDFNT_PAT_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     font_local()
{
#if DRIVER == 5587
      /*cmdout( FONT_LOCAL_CMD );*/
      /*pput1( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

int     del_local_font()
{
#if DRIVER == 5587
      /*Data_after_esc( DELLOC_FNT_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     char_rotation()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( CHAR_ROTAT_CMD );*/
      /*pput2( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

int     char_bline_offset()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( BASE_OFSET_CMD );*/
      /*pput2( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

int     text_rotation()
{
        int     i = R_st( 0 );
        int     b = R_st( 1 );

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

int     cp_number()
{
#if DRIVER == 5587
      /*cmdout( SET_COPY_CMD );*/
      /*pput1( R_st( 0 ) );    */
      R_st(0);
#endif
        return( NORMAL );
}

int     logical_page()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( SET_LOGPG_CMD );*/
      /*pput2( R_st( 0 ) );     */
      /*pput2( R_st( 1 ) );     */
      /*pput2( R_st( 2 ) );     */
      /*pput2( R_st( 3 ) );     */
      /*pput1( 0 );             */
      R_st(0);R_st(1);R_st(2);R_st(3);
#endif
        return( NORMAL );
}

int     line_type()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( SET_LNTYPE_CMD );*/
      /*pput1( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

int     line_width()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( SET_LNWIDE_CMD );*/
      /*pput1( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

#if defined( IBM5587H )
int     draw_box_sub( char *attr )
{
        int     i, c, n = R_st( 0 );

        Escarglen = n;
      /*cmdout( attr );*/
      /*Data_through( n - 1 );*/
        Data_through_dum( n - 1 );
        return( NORMAL );
}
#endif

int     draw_box()
{
#if defined( IBM5587H )
        return( draw_box_sub( DRAW_BOX_CMD ) );
#else
        int     n = R_st( 0 );

# if defined( IBM5587G )
      /*cmdout( DRAW_BOX_CMD ); */
      /*pput2( n );             */
      /*pput1( 0xc1 );          */
# endif
# if defined( IBM5587G )
      /*Data_through( n - 1 );*/
        Data_through_dum( n - 1 );
# else
        Data_through_dum( n - 1 );
# endif
        return( NORMAL );
#endif
}

int     draw_box2()
{
#if defined( IBM5587H )
        return( draw_box_sub( DRAW_BOX2_CMD ) );
#else
        int     n = R_st( 0 );

        Data_through_dum( n - 1 );
        return( NORMAL );
#endif
}

int     draw_box3()
{
#if defined( IBM5587H )
        return( draw_box_sub( DRAW_BOX3_CMD ) );
#else
        int     n = R_st( 0 );

        Data_through_dum( n - 1 );
        return( NORMAL );
#endif
}

int     relative_line()
{
#if defined( IBM5587H )
        return( draw_box_sub( REL_LINE_CMD ) );
#else
        int     n = R_st( 0 );

        Data_through_dum( n - 1 );
        return( NORMAL );
#endif
}

int     paper_source()
{
#if defined( IBM5587G ) || defined( IBM5587H )
        int     n = R_st( 0 );

        if( n == 0 ) Paper_src = 2;
        if( n == 1 ) Paper_src = 1;
# if defined( IBM5587H )
        if( n >= 0xfd && n <= 0xff ) Paper_src = n;
# endif
      /*cmdout( PAPER_SRC_CMD );*/
#endif
        return( NORMAL );
}

int     overlay_cntl()
{
#if defined( IBM5587G ) || defined( IBM5587H )
      /*cmdout( OVERLAY_CMD );*/
      /*pput2( R_st( 0 ) );   */
      /*pput2( R_st( 1 ) );   */
      R_st(0);R_st(1);
#endif
        return( NORMAL );
}

int     esc_segment()
{
#if DRIVER==5587
      /*cmdout( SEGMENT_CMD );*/
      /*pput2( R_st( 0 ) );   */
      /*pput2( R_st( 1 ) );   */
      R_st(0);R_st(1);
#endif
        return( NORMAL );
}

int     esc_pmp()
{
#if DRIVER==5587
      /*Data_after_esc( PMP_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     media_size()
{
#if defined( IBM5587H )
      /*cmdout( MEDIA_SIZE_CMD );*/
      /*pput1( R_st( 0 ) );      */
      /*pput2( R_st( 1 ) );      */
      /*pput2( R_st( 2 ) );      */
      /*pput2( R_st( 3 ) );      */
      R_st(0);R_st(1);R_st(2);R_st(3);
#endif
        return( NORMAL );
}

int     media_axis()
{
#if defined( IBM5587H )
      /*cmdout( MEDIA_AXIS_CMD );*/
      /*pput1( R_st( 0 ) );      */
      R_st(0);
#endif
        return( NORMAL );
}

int     page_format()
{
#if defined( IBM5587H )
      /*cmdout( PAGE_FMT_CMD );*/
      /*pput1( R_st( 0 ) );    */
      R_st(0);
#endif
        return( NORMAL );
}

int     page_descripter()
{
#if defined( IBM5587H )
      /*Data_after_esc( PAGE_DSCRPT_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     logical_zero()
{
#if defined( IBM5587H )
      /*cmdout( LOGICAL_ZERO_CMD );*/
      /*pput4( R_st( 0 ) );        */
      /*pput4( R_st( 1 ) );        */
      R_st(0);R_st(1);
#endif
        return( NORMAL );
}

int     composed_char()
{
#if defined( IBM5587H )
        int     n = R_st( 0 ), m = R_st( 1 );

        if( n - 1 == m ){
              /*cmdout( COMPOSE_C_CMD );*/

                switch( m ){
                case 1:
                      /*pput1( m );         */
                      /*pput1( R_st( 2 ) ); */
                      R_st(2);
                        break;
                case 2:
                      /*pput1( m );         */
                      /*pput2( R_st( 2 ) ); */
                      R_st(2);
                        break;
                default:
                      /*pput1( 0 );         */
                }
        }
#endif
        return( NORMAL );
}

int     mesh_cntl()
{
#if defined( IBM5587H )
      /*Data_after_esc( MESH_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     mesh_reference()
{
#if defined( IBM5587H )
      /*cmdout( MESH_REF_CMD );*/
      /*pput1( R_st( 0 ) );    */
      /*pput4( R_st( 1 ) );    */
      /*pput4( R_st( 2 ) );    */
      R_st(0);R_st(1);R_st(2);
#endif
        return( NORMAL );
}

int     mesh_clear()
{
#if defined( IBM5587H )
      /*cmdout( MESH_CLR_CMD );*/
      /*pput1( R_st( 0 ) );    */
      R_st(0);
#endif
        return( NORMAL );
}

int     mesh_record()
{
        int     i, c, n = R_st( 0 );

#if defined( IBM5587H )
      /*cmdout( MESH_REC_CMD );*/
      /*pput2( n );            */
      /*pput1( R_st( 1 ) );    */
      /*pput2( R_st( 2 ) );    */
      /*pput1( R_st( 3 ) );    */
      /*pput1( R_st( 4 ) );    */
      R_st(0);R_st(1);R_st(2);R_st(3);R_st(4);
#endif
#if defined( IBM5587H )
      /*Data_through( n - 5 );*/
      Data_through_dum( n - 5 );
#else
        Data_through_dum( n - 5 );
#endif
        return( NORMAL );
}

int     graphics_cntl()
{
#if defined( IBM5587H )
      /*Data_after_esc( GRPH_CNTL_CMD );*/
        Data_after_esc_dum();
#else
        Data_after_esc_dum();
#endif
        return( NORMAL );
}

int     graphics_draw()
{
#if defined( IBM5587H )
      /*Data_after_esc( GRPH_DRAW_CMD );*/
        Data_after_esc_dum();
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
	{ ESCDATA, (int)(Ttdata + 262), NULL, NULL }
};

ESCSEQ_DATA	Ttdata[440] = {
	{ FIX_VALUE, 0x1c, Ttdata + 2, Ttdata + 1 },
	{ FUNC_LEAF, fs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x1b, Ttdata + 418, Ttdata + 3 },
	{ FIX_VALUE, '~', Ttdata + 370, Ttdata + 4 },
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
	{ FIX_SIZE, 1, Ttdata + 168, Ttdata + 170 },
	{ FUNC_LEAF, char_scaling, NULL, NULL },
	{ FIX_VALUE, 0x1f, Ttdata + 176, Ttdata + 172 },
	{ FIX_VALUE, 0x0, Ttdata + 171, Ttdata + 173 },
	{ FIX_VALUE, 0x2, Ttdata + 172, Ttdata + 174 },
	{ FIX_SIZE, 2, Ttdata + 173, Ttdata + 175 },
	{ FUNC_LEAF, line_pitch_1440, NULL, NULL },
	{ FIX_VALUE, 0x1e, Ttdata + 181, Ttdata + 177 },
	{ FIX_VALUE, 0x0, Ttdata + 176, Ttdata + 178 },
	{ FIX_VALUE, 0x2, Ttdata + 177, Ttdata + 179 },
	{ FIX_SIZE, 2, Ttdata + 178, Ttdata + 180 },
	{ FUNC_LEAF, char_pitch_1440, NULL, NULL },
	{ FIX_VALUE, 0x1d, Ttdata + 206, Ttdata + 182 },
	{ FIX_VALUE, 0x0, Ttdata + 181, Ttdata + 183 },
	{ FIX_VALUE, 0x3, Ttdata + 196, Ttdata + 184 },
	{ FIX_VALUE, 0x6, Ttdata + 187, Ttdata + 185 },
	{ FIX_SIZE, 2, Ttdata + 184, Ttdata + 186 },
	{ FUNC_LEAF, v_lineskipx, NULL, NULL },
	{ FIX_VALUE, 0x5, Ttdata + 190, Ttdata + 188 },
	{ FIX_SIZE, 2, Ttdata + 187, Ttdata + 189 },
	{ FUNC_LEAF, v_lineskipx, NULL, NULL },
	{ FIX_VALUE, 0x4, Ttdata + 193, Ttdata + 191 },
	{ FIX_SIZE, 2, Ttdata + 190, Ttdata + 192 },
	{ FUNC_LEAF, v_lineskipx, NULL, NULL },
	{ FIX_VALUE, 0x3, Ttdata + 183, Ttdata + 194 },
	{ FIX_SIZE, 2, Ttdata + 193, Ttdata + 195 },
	{ FUNC_LEAF, v_lineskipx, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 182, Ttdata + 197 },
	{ FIX_VALUE, 0x2, Ttdata + 200, Ttdata + 198 },
	{ FIX_SIZE, 1, Ttdata + 197, Ttdata + 199 },
	{ FUNC_LEAF, v_lineskip2, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 203, Ttdata + 201 },
	{ FIX_SIZE, 1, Ttdata + 200, Ttdata + 202 },
	{ FUNC_LEAF, v_lineskip1, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 196, Ttdata + 204 },
	{ FIX_SIZE, 1, Ttdata + 203, Ttdata + 205 },
	{ FUNC_LEAF, v_lineskip0, NULL, NULL },
	{ FIX_VALUE, 0x1c, Ttdata + 231, Ttdata + 207 },
	{ FIX_VALUE, 0x0, Ttdata + 206, Ttdata + 208 },
	{ FIX_VALUE, 0x3, Ttdata + 221, Ttdata + 209 },
	{ FIX_VALUE, 0x6, Ttdata + 212, Ttdata + 210 },
	{ FIX_SIZE, 2, Ttdata + 209, Ttdata + 211 },
	{ FUNC_LEAF, h_colskipx, NULL, NULL },
	{ FIX_VALUE, 0x5, Ttdata + 215, Ttdata + 213 },
	{ FIX_SIZE, 2, Ttdata + 212, Ttdata + 214 },
	{ FUNC_LEAF, h_colskipx, NULL, NULL },
	{ FIX_VALUE, 0x4, Ttdata + 218, Ttdata + 216 },
	{ FIX_SIZE, 2, Ttdata + 215, Ttdata + 217 },
	{ FUNC_LEAF, h_colskipx, NULL, NULL },
	{ FIX_VALUE, 0x3, Ttdata + 208, Ttdata + 219 },
	{ FIX_SIZE, 2, Ttdata + 218, Ttdata + 220 },
	{ FUNC_LEAF, h_colskipx, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 207, Ttdata + 222 },
	{ FIX_VALUE, 0x2, Ttdata + 225, Ttdata + 223 },
	{ FIX_SIZE, 1, Ttdata + 222, Ttdata + 224 },
	{ FUNC_LEAF, h_colskip2, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 228, Ttdata + 226 },
	{ FIX_SIZE, 1, Ttdata + 225, Ttdata + 227 },
	{ FUNC_LEAF, h_colskip1, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 221, Ttdata + 229 },
	{ FIX_SIZE, 1, Ttdata + 228, Ttdata + 230 },
	{ FUNC_LEAF, h_colskip0, NULL, NULL },
	{ FIX_VALUE, 0x1b, Ttdata + 236, Ttdata + 232 },
	{ FIX_VALUE, 0x0, Ttdata + 231, Ttdata + 233 },
	{ FIX_VALUE, 0x1, Ttdata + 232, Ttdata + 234 },
	{ FIX_SIZE, 1, Ttdata + 233, Ttdata + 235 },
	{ FUNC_LEAF, set_skip_perforation, NULL, NULL },
	{ FIX_VALUE, 0x1a, Ttdata + 242, Ttdata + 237 },
	{ FIX_VALUE, 0x0, Ttdata + 236, Ttdata + 238 },
	{ FIX_VALUE, 0x2, Ttdata + 237, Ttdata + 239 },
	{ FIX_SIZE, 1, Ttdata + 238, Ttdata + 240 },
	{ FIX_SIZE, 1, Ttdata + 239, Ttdata + 241 },
	{ FUNC_LEAF, set_hmargin, NULL, NULL },
	{ FIX_VALUE, 0x19, Ttdata + 248, Ttdata + 243 },
	{ FIX_VALUE, 0x0, Ttdata + 246, Ttdata + 244 },
	{ FIX_VALUE, 0x0, Ttdata + 243, Ttdata + 245 },
	{ FUNC_LEAF, clr_vtab, NULL, NULL },
	{ FIX_SIZE, 2, Ttdata + 242, Ttdata + 247 },
	{ FUNC_LEAF, set_vtab, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 257, Ttdata + 249 },
	{ FIX_VALUE, 0x0, Ttdata + 255, Ttdata + 250 },
	{ FIX_VALUE, 0x1, Ttdata + 253, Ttdata + 251 },
	{ FIX_VALUE, 0x0, Ttdata + 250, Ttdata + 252 },
	{ FUNC_LEAF, default_htab, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 249, Ttdata + 254 },
	{ FUNC_LEAF, clr_htab, NULL, NULL },
	{ FIX_SIZE, 2, Ttdata + 248, Ttdata + 256 },
	{ FUNC_LEAF, set_htab, NULL, NULL },
	{ FIX_VALUE, 0x16, Ttdata + 261, Ttdata + 258 },
	{ FIX_SIZE, 2, Ttdata + 257, Ttdata + 259 },
	{ FIX_VALUE, 0x1, Ttdata + 258, Ttdata + 260 },
	{ FUNC_LEAF, gridline, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 267, Ttdata + 262 },
	{ FIX_SIZE, 2, Ttdata + 261, Ttdata + 263 },
	{ FIX_SIZE, 1, Ttdata + 262, Ttdata + 264 },
	{ FIX_SIZE, 1, Ttdata + 263, Ttdata + 265 },
	{ VAR_SIZE, (int)(Ttok + 1), Ttdata + 264, Ttdata + 266 },
	{ FUNC_LEAF, overstrike_cntl, NULL, NULL },
	{ FIX_VALUE, 0x12, Ttdata + 270, Ttdata + 268 },
	{ FIX_SIZE, 3, Ttdata + 267, Ttdata + 269 },
	{ FUNC_LEAF, ch_datastream, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 275, Ttdata + 271 },
	{ FIX_VALUE, 0x0, Ttdata + 270, Ttdata + 272 },
	{ FIX_VALUE, 0x1, Ttdata + 271, Ttdata + 273 },
	{ FIX_SIZE, 1, Ttdata + 272, Ttdata + 274 },
	{ FUNC_LEAF, underline_mode, NULL, NULL },
	{ FIX_VALUE, 0x10, Ttdata + 278, Ttdata + 276 },
	{ FIX_SIZE, 3, Ttdata + 275, Ttdata + 277 },
	{ FUNC_LEAF, ch_form_hdl, NULL, NULL },
	{ FIX_VALUE, 0xe, Ttdata + 329, Ttdata + 279 },
	{ FIX_VALUE, 0x0, Ttdata + 278, Ttdata + 280 },
	{ FIX_VALUE, 0x1, Ttdata + 279, Ttdata + 281 },
	{ FIX_VALUE, '$', Ttdata + 283, Ttdata + 282 },
	{ FUNC_LEAF, pr_complete, NULL, NULL },
	{ FIX_VALUE, 0x1a, Ttdata + 285, Ttdata + 284 },
	{ FUNC_LEAF, dstrike_out, NULL, NULL },
	{ FIX_VALUE, 0x19, Ttdata + 287, Ttdata + 286 },
	{ FUNC_LEAF, dstrike_in, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 289, Ttdata + 288 },
	{ FUNC_LEAF, emphasize_out, NULL, NULL },
	{ FIX_VALUE, 0x17, Ttdata + 291, Ttdata + 290 },
	{ FUNC_LEAF, emphasize_in, NULL, NULL },
	{ FIX_VALUE, 0x16, Ttdata + 293, Ttdata + 292 },
	{ FUNC_LEAF, set2tr, NULL, NULL },
	{ FIX_VALUE, 0x15, Ttdata + 295, Ttdata + 294 },
	{ FUNC_LEAF, set3tr, NULL, NULL },
	{ FIX_VALUE, 0x14, Ttdata + 297, Ttdata + 296 },
	{ FUNC_LEAF, halfline_down, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 299, Ttdata + 298 },
	{ FUNC_LEAF, halfline_up, NULL, NULL },
	{ FIX_VALUE, 0x12, Ttdata + 301, Ttdata + 300 },
	{ FUNC_LEAF, errdetect_out, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 303, Ttdata + 302 },
	{ FUNC_LEAF, errdetect_in, NULL, NULL },
	{ FIX_VALUE, 0xf, Ttdata + 305, Ttdata + 304 },
	{ FUNC_LEAF, script_out, NULL, NULL },
	{ FIX_VALUE, 0xe, Ttdata + 307, Ttdata + 306 },
	{ FUNC_LEAF, subscr_in, NULL, NULL },
	{ FIX_VALUE, 0xd, Ttdata + 309, Ttdata + 308 },
	{ FUNC_LEAF, superscr_in, NULL, NULL },
	{ FIX_VALUE, 0xc, Ttdata + 311, Ttdata + 310 },
	{ FUNC_LEAF, vprint_out, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 313, Ttdata + 312 },
	{ FUNC_LEAF, vprint_in, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 315, Ttdata + 314 },
	{ FUNC_LEAF, large_out, NULL, NULL },
	{ FIX_VALUE, 0x9, Ttdata + 317, Ttdata + 316 },
	{ FUNC_LEAF, large_in, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 319, Ttdata + 318 },
	{ FUNC_LEAF, condense_out, NULL, NULL },
	{ FIX_VALUE, 0x7, Ttdata + 321, Ttdata + 320 },
	{ FUNC_LEAF, condense_in, NULL, NULL },
	{ FIX_VALUE, 0x6, Ttdata + 323, Ttdata + 322 },
	{ FUNC_LEAF, cutform_out, NULL, NULL },
	{ FIX_VALUE, 0x5, Ttdata + 325, Ttdata + 324 },
	{ FUNC_LEAF, cutform_in, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 327, Ttdata + 326 },
	{ FUNC_LEAF, lspeed_mode, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 280, Ttdata + 328 },
	{ FUNC_LEAF, hspeed_mode, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 332, Ttdata + 330 },
	{ FIX_SIZE, 4, Ttdata + 329, Ttdata + 331 },
	{ FUNC_LEAF, ch_vmove, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 335, Ttdata + 333 },
	{ FIX_SIZE, 4, Ttdata + 332, Ttdata + 334 },
	{ FUNC_LEAF, ch_hmove, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 338, Ttdata + 336 },
	{ FIX_SIZE, 2, Ttdata + 335, Ttdata + 337 },
	{ FUNC_LEAF, tr_print, NULL, NULL },
	{ FIX_VALUE, 0x6, Ttdata + 343, Ttdata + 339 },
	{ FIX_VALUE, 0x0, Ttdata + 338, Ttdata + 340 },
	{ FIX_VALUE, 0x1, Ttdata + 339, Ttdata + 341 },
	{ FIX_SIZE, 1, Ttdata + 340, Ttdata + 342 },
	{ FUNC_LEAF, font_style, NULL, NULL },
	{ FIX_VALUE, 0x4, Ttdata + 353, Ttdata + 344 },
	{ FIX_VALUE, 0x0, Ttdata + 343, Ttdata + 345 },
	{ FIX_VALUE, 0x3, Ttdata + 349, Ttdata + 346 },
	{ FIX_VALUE, 0x0, Ttdata + 345, Ttdata + 347 },
	{ FIX_SIZE, 2, Ttdata + 346, Ttdata + 348 },
	{ FUNC_LEAF, ch_pglen_6, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 344, Ttdata + 350 },
	{ FIX_VALUE, 0x1, Ttdata + 349, Ttdata + 351 },
	{ FIX_SIZE, 1, Ttdata + 350, Ttdata + 352 },
	{ FUNC_LEAF, ch_pglen_l, NULL, NULL },
	{ FIX_VALUE, 0x3, Ttdata + 358, Ttdata + 354 },
	{ FIX_VALUE, 0x0, Ttdata + 353, Ttdata + 355 },
	{ FIX_VALUE, 0x1, Ttdata + 354, Ttdata + 356 },
	{ FIX_SIZE, 1, Ttdata + 355, Ttdata + 357 },
	{ FUNC_LEAF, line_pitch, NULL, NULL },
	{ FIX_VALUE, 0x2, Ttdata + 363, Ttdata + 359 },
	{ FIX_VALUE, 0x0, Ttdata + 358, Ttdata + 360 },
	{ FIX_VALUE, 0x1, Ttdata + 359, Ttdata + 361 },
	{ FIX_SIZE, 1, Ttdata + 360, Ttdata + 362 },
	{ FUNC_LEAF, char_pitch, NULL, NULL },
	{ FIX_VALUE, 0x1, Ttdata + 367, Ttdata + 364 },
	{ FIX_VALUE, 0x0, Ttdata + 363, Ttdata + 365 },
	{ FIX_VALUE, 0x0, Ttdata + 364, Ttdata + 366 },
	{ FUNC_LEAF, init_pr, NULL, NULL },
	{ FIX_VALUE, 0x0, Ttdata + 3, Ttdata + 368 },
	{ FIX_SIZE, 2, Ttdata + 367, Ttdata + 369 },
	{ FUNC_LEAF, read_pr, NULL, NULL },
	{ FIX_VALUE, ']', Ttdata + 372, Ttdata + 371 },
	{ FUNC_LEAF, large_out, NULL, NULL },
	{ FIX_VALUE, '[', Ttdata + 374, Ttdata + 373 },
	{ FUNC_LEAF, large_in, NULL, NULL },
	{ FIX_VALUE, 'V', Ttdata + 376, Ttdata + 375 },
	{ FUNC_LEAF, cutform_out, NULL, NULL },
	{ FIX_VALUE, 'S', Ttdata + 378, Ttdata + 377 },
	{ FUNC_LEAF, cutform_in, NULL, NULL },
	{ FIX_VALUE, 'P', Ttdata + 380, Ttdata + 379 },
	{ FUNC_LEAF, lspeed_mode, NULL, NULL },
	{ FIX_VALUE, 'O', Ttdata + 382, Ttdata + 381 },
	{ FUNC_LEAF, hspeed_mode, NULL, NULL },
	{ FIX_VALUE, 'F', Ttdata + 385, Ttdata + 383 },
	{ FIX_SIZE, 2, Ttdata + 382, Ttdata + 384 },
	{ FUNC_LEAF, def_pglen, NULL, NULL },
	{ FIX_VALUE, ')', Ttdata + 387, Ttdata + 386 },
	{ FUNC_LEAF, set2tr, NULL, NULL },
	{ FIX_VALUE, '(', Ttdata + 389, Ttdata + 388 },
	{ FUNC_LEAF, set3tr, NULL, NULL },
	{ FIX_VALUE, '%', Ttdata + 2, Ttdata + 390 },
	{ FIX_VALUE, 'U', Ttdata + 392, Ttdata + 391 },
	{ FUNC_LEAF, unidir_mode, NULL, NULL },
	{ FIX_VALUE, 'B', Ttdata + 394, Ttdata + 393 },
	{ FUNC_LEAF, bidir_mode, NULL, NULL },
	{ FIX_VALUE, '9', Ttdata + 397, Ttdata + 395 },
	{ FIX_SIZE, 2, Ttdata + 394, Ttdata + 396 },
	{ FUNC_LEAF, def_lspacing, NULL, NULL },
	{ FIX_VALUE, '8', Ttdata + 400, Ttdata + 398 },
	{ FIX_SIZE, 2, Ttdata + 397, Ttdata + 399 },
	{ FUNC_LEAF, bw_vskip, NULL, NULL },
	{ FIX_VALUE, '6', Ttdata + 403, Ttdata + 401 },
	{ FIX_SIZE, 2, Ttdata + 400, Ttdata + 402 },
	{ FUNC_LEAF, cr_pset, NULL, NULL },
	{ FIX_VALUE, '5', Ttdata + 406, Ttdata + 404 },
	{ FIX_SIZE, 2, Ttdata + 403, Ttdata + 405 },
	{ FUNC_LEAF, fw_vskip, NULL, NULL },
	{ FIX_VALUE, '4', Ttdata + 409, Ttdata + 407 },
	{ FIX_SIZE, 2, Ttdata + 406, Ttdata + 408 },
	{ FUNC_LEAF, bw_hskip, NULL, NULL },
	{ FIX_VALUE, '3', Ttdata + 412, Ttdata + 410 },
	{ FIX_SIZE, 2, Ttdata + 409, Ttdata + 411 },
	{ FUNC_LEAF, fw_hskip, NULL, NULL },
	{ FIX_VALUE, '2', Ttdata + 415, Ttdata + 413 },
	{ FIX_SIZE, 2, Ttdata + 412, Ttdata + 414 },
	{ FUNC_LEAF, ex_image_data_set, NULL, NULL },
	{ FIX_VALUE, '1', Ttdata + 389, Ttdata + 416 },
	{ FIX_SIZE, 2, Ttdata + 415, Ttdata + 417 },
	{ FUNC_LEAF, image_data_set, NULL, NULL },
	{ FIX_VALUE, 0x18, Ttdata + 420, Ttdata + 419 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0x13, Ttdata + 422, Ttdata + 421 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0x11, Ttdata + 424, Ttdata + 423 },
	{ FUNC_LEAF, nop, NULL, NULL },
	{ FIX_VALUE, 0xd, Ttdata + 426, Ttdata + 425 },
	{ FUNC_LEAF, cr_cntl, NULL, NULL },
	{ FIX_VALUE, 0xc, Ttdata + 428, Ttdata + 427 },
	{ FUNC_LEAF, ff_cntl, NULL, NULL },
	{ FIX_VALUE, 0xb, Ttdata + 430, Ttdata + 429 },
	{ FUNC_LEAF, vt_cntl, NULL, NULL },
	{ FIX_VALUE, 0xa, Ttdata + 432, Ttdata + 431 },
	{ FUNC_LEAF, lf_cntl, NULL, NULL },
	{ FIX_VALUE, 0x9, Ttdata + 434, Ttdata + 433 },
	{ FUNC_LEAF, ht_cntl, NULL, NULL },
	{ FIX_VALUE, 0x8, Ttdata + 436, Ttdata + 435 },
	{ FUNC_LEAF, bs_cntl, NULL, NULL },
	{ FIX_VALUE, 0x7, Ttdata + 438, Ttdata + 437 },
	{ FUNC_LEAF, bel_cntl, NULL, NULL },
	{ FIX_VALUE, 0x0, NULL, Ttdata + 439 },
	{ FUNC_LEAF, null_cntl, NULL, NULL }
};


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


/************************************************/

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
 *      Upon successful completion, the setup function should return either,
 *
 *      (1) a pointer to a shar_vars structure that contains pointers to
 *          initialized vertical spacing variables that will be shared with
 *          the formatter driver, who will provide vertical page movement, or
 *
 *      (2) a NULL pointer, whitch indicates that the formatter will handle
 *          its own vertical page movement (i.e. top and bottom margins,
 *          new pages, initial pages to be skipped, future top and bottom
 *          security labels, and progress reports to qdaemon), or that the
 *          input data stream is to be passed through without modification.
 */

struct shar_vars *setup (argc, argv, passthru)
unsigned int argc;
char *argv[];
int passthru;
{
    /*struct stat buf;          */              /* Buffer for stat */
    /*void free ();             */
    void errorexit ();
    void warnmsg ();
    extern int piogetvals ();
    extern int piogetopt ();
    extern void pioexit ();

/**************************************************/
#if defined (EMUDEBUG)
        debfp=fopen("4019deb.msg","w");			/* file may be opened in /usr/lpd/qdir	*/
#endif
    /* tell Formatter Driver about Data Base variables */
    if (piogetvals (attrtable, NULL) < 0)
        pioexit (PIOEXITBAD);

    /* process command line flags */
    if (piogetopt (argc, argv, OPTSTRING, NULL) < 0)
        pioexit (PIOEXITBAD);

    if( ( Ccs = lc_init( Codepagename.ptr, Trans_in.ptr, Trans_out.ptr, User_font.ptr, Font_path.ptr) ) == 0 ){
	errorexit( MSG_BADCODEPAGE, "" );
    }
    
    /* if passthru mode, that's all that needs to be done for setup() */
    if (passthru)
	return (NULL);

    /* initialize shadow variables */
    SET_Pglen (Pglen);
    SET_Tmarg (Tmarg);
    SET_Bmarg (Bmarg);
    SET_Lmarg (Indent);
    
    /* Validation */
    if (Emphasized & 0xfffffffe)        errorexit(MSG_BADFORMFLAG1, "e");
    if (Beginpg <= 0)                   errorexit(MSG_BADFORMFLAG2, "g");
    if (Init_printer & 0xfffffffe)      errorexit(MSG_BADFORMFLAG1, "j");
    /**************************************/
    /*set_Pitch( 0x32 ) ; *//* 5CPI */
    /*set_Lpi (0x3c );    *//* 6LPI */
    /**************************************/
    if (Pitch != 0x32 &&
        Pitch != 0x3c &&
        Pitch != 0x43 &&
        Pitch != 0x4b)                  errorexit(MSG_BADPITCH, "p");
    if (Lpi != 0x14 &&
        Lpi != 0x1e &&
        Lpi != 0x28 &&
        Lpi != 0x32 &&
        Lpi != 0x3c &&
        Lpi != 0x4b &&
        Lpi != 0x50)                    errorexit(MSG_BADLPI, "v");
    
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
    if (Auto_crlf < 0 || Auto_crlf > 2) errorexit(MSG_BADFORMFLAG7, "x");
#if DRIVER != 5587 && DRIVER != 5327
    if (Doublestrike & 0xfffffffe)      errorexit(MSG_BADFORMFLAG1, "y");
#endif
    if (Doublewide & 0xfffffffe)        errorexit(MSG_BADFORMFLAG1, "W");
    if (Doublehigh & 0xfffffffe)        errorexit(MSG_BADFORMFLAG1, "E");
    if (Restoreprinter & 0xfffffffe)    errorexit(MSG_BADFORMFLAG1, "J");
    if (Condensed & 0xfffffffe)         errorexit(MSG_BADFORMFLAG1, "K");
    if (Wrap & 0xfffffffe)              errorexit(MSG_BADFORMFLAG1, "L");
#if DRIVER != 5587
    if (Highspeed & 0xfffffffe)         errorexit(MSG_BADFORMFLAG1, "S");
#endif
#if DRIVER != 5587 && DRIVER != 5327
    if (Unidirect & 0xfffffffe)         errorexit(MSG_BADFORMFLAG1, "U");
#endif
    if (Vertical & 0xfffffffe)          errorexit(MSG_BADFORMFLAG1, "V");
    if (Do_formfeed & 0xfffffffe)       errorexit(MSG_BADFORMFLAG1, "Z");

#if DRIVER == 5577
    if (Paper_handle < 1 || Paper_handle > 3)
	errorexit(MSG_BADPAPERFLAG, "O");
#endif
/*#if defined( IBM5587G ) || defined( IBM5587H )
  if ( ( Paper_src < 1 || Paper_src > 2 )*/
    /*# if defined( IBM5587H )
      && ( Paper_src < 0xfd || Paper_src > 0xff )
      # endif
      )*/
    if ( ( Paper_src < 0 || Paper_src > 4 ) )
	errorexit(MSG_BADPAPERSRC,  "u");
    /*#endif*/
#if DRIVER==5587
    if (Rotation != 0 && Rotation != 3
# if defined( IBM5587H )
        && Rotation != 1 && Rotation != 2
# endif
	)
	errorexit(MSG_BADROTATION,  "z");
#endif
    if (Pgwidth < 1)                    errorexit(MSG_BADFORMFLAG2, "w");
    if (Indent < 0)                     errorexit(MSG_BADFORMFLAG3, "i");
    if (Indent >= Pgwidth)              errorexit(MSG_BADFORMFLAG8, "i");
    
    
    /* arrange for pages to be skipped (if any) */
    piopgskip = Beginpg - 1;
    
    /* Initialize pointers to vertical spacing variables shared with  */
    /* Formatter Driver (except for command names, all values pointed */
    /* to are in Vres unit)                                           */
    sharevars._pl       = &pglen;       /* page length */
    sharevars._tmarg    = &tmarg;       /* top margin */
    sharevars._bmarg    = &bmarg;       /* bottom margin */
    sharevars._vpos     = &vpos;        /* vertical position of next line */
    sharevars._vtab_base= &vtab_base;   /* base for vertical tab stop */
    sharevars._vincr    = &Vincr;       /* vertical increment of Vincr cmd */
    sharevars._vincr_cmd= (&Vincr_cmd)->ptr;/* name of vertical increment cmd*/
    sharevars._vdecr    = &Vdecr;       /* vertical decrement of Vdecr cmd */
    sharevars._vdecr_cmd= (&Vdecr_cmd)->ptr;/* name of vertical decrement cmd*/
    sharevars._ff_cmd   = (&Ff_cmd)->ptr;/* name of form feed command */
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
 *      Upon successful completion, the initialize function should return
 *      a value of zero.
 */


int initialize ()
{
    if (Init_printer) cmdout (INIT_CMD);
    if (Init_emu)     cmdout (INIT_EMU_CMD);
    return (0);
/*******************************/
/*
        printf(" Pgwidth %x",Pgwidth);
*/
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
 *      Upon successful completion, the passthru function should return a
 *      value of zero, indicating that printing has completed successfully.
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
 *      Upon successful completion, the restore function should return a
 *      value of zero.
 */

int restore ()
{
    /* output the command string to restore the printer */
    if (Restoreprinter)
        cmdout (REST_CMD);

/***********************************/
#if defined(EMUDEBUG)
        fclose(debfp);
#endif
    return (0);
}


# include	<iconv.h>

static int      Convert_a_char( char *dest, char *src, int dn, int sn )
{
    char*	p;
    int	l;

    Ccsid = get_iFont( src, dest, Ccs );
    if( Ccsid < 0 ) 
	return( Ccsid );
    l = strlen( dest );
    if( l == 0 ) l++;
    return( l );
}


int     get_a_mbcx( int *ri, char *bx, int bxsize )
{
        int     j, i, c;
        char    b[NLCHARMAX+1];


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
			}else if( j == -1 ){
				continue;

			}else if( j == -2 ){
			    Ccs->state = JIS;
			    i = 0;
			    bx[0] = 0;
			    continue;

			}else if( j == -3 ){
			    i = 0;
			    bx[0] = 0;
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


int	mbwidth( char *p )
{
	return( get_wid( Ccsid, Ccs ) );
}


/*
 * NAME: lineout
 *
 * FUNCTIONS: Formats a print line
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
 *      Upon successful completion, the lineout function should return the
 *      number of bytes processed from the input data stream. If the value
 *      returned is zero, and there are no more data bytes in the input
 *      data stream, the formatter driver assumes that printing is complete,
 *      and the lineout will not be invoked again.
 */
lineout( FILE *fileptr )
{
        int     i, j;
        char    wc[100];
        char    bx[NLCHARMAX+1];


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
              /*cmdout( OVERSTRIKE_CMD );                            */
              /*pputn( Ovrstrike_flag - 2, (char *)&Ovrstrike_code );*/
        }

        if( hpos > pgwidth ) trunc++;
        Setfp_buffer( fileptr );

        for( count = 0; !Eof_buffer(); ){
                switch( tbl_search( Ttdata, 1 ) ){
                case 0:
                        /* 1 character conversion */
                        j = get_a_mbcx( &i, bx, NLCHARMAX );

                        if( j == 0 ){ /* c == EOF */
/*                              for( j = 0; j < i - 1; j++ ) Unread_buffer();
                                if( i > 0 ) pput1( b[0] );*/
                                return( count );
                        }
			else if (j < 0) {       /* i is invalid multibyte */
			    for( j = 0; j < i - 1; j++ ) Unread_buffer();
				/* first invalid byte is ignored */
				/* no return */
			    break;
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
					Fontimg	f = get_Font( bx, get_pFont( Ccs, Ccsid ), get_wid( Ccsid, Ccs ), PR_PAD);
					if( f == 0 ){
						errorexit( MSG_FATAL, "" );
					}else if( f->w == SEND_CODEVALUE ){
						pputn( j, bx );
					}else{
						fontout( f, mbwidth( bx ) );
					}
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


int     fontout( Fontimg f, int wid )
{
	static char	imgb[256];
        char *pimg, imgbuf[256];
        char *pbicmd;
        int     xs, ys, xd, yd;
        int     dpi, bheight, bwidth;
        int     i;
	void	c_image_us();

        bheight = (f->h+7)>>3;
        bwidth  = (f->w+7)>>3;
	c_image_us( f->img, imgb, bwidth, bheight, f->bytes_in_line );
/*
	i = f->w; f->w = f->h; f->h = i;
	i = bwidth; bwidth = bheight; bheight = i;
*/

        if( f->w <= 16 && f->h <=16 ){
                dpi = 120;
                pbicmd = DDBIT_IMG_CMD;
        } else {
                dpi = 240;
                pbicmd = HDBIT_IMG_CMD;
        }

	if( wid == 1 ){
        	xs = Hincr/2 - f->w*1440/dpi/2;
	}else{
        	xs = Hincr - f->w*1440/dpi/2;
	}
        xd = -( bwidth*8*1440/dpi );
        ys = Vincr/2 - f->h*1440/dpi/2;
        yd = 8*1440/dpi;

        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x78 );                 /* save x,y to 30 */

        cmdout( SET_CURPOS_CMD );
        pput2 ( xs );
        pput2 ( ys );
        cmdout( pbicmd );
        pput1 ( bwidth*8 );
        pput1 (  0 );
        pputn ( bwidth*8, imgb );

        for (i=1; i<bheight; i++){
                cmdout( SET_CURPOS_CMD );
                pput2 ( xd );
                pput2 ( yd );
                cmdout( pbicmd );
                pput1 ( bwidth*8 );
                pput1 (  0 );
                pputn ( bwidth*8, imgb+bwidth*8*i );
        }

        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x7b );                 /* restore x,y from 30 */

	if( wid == 1 ){
	        pput1 ( 0x20 );               /* 1 blank */
	}else{
	        pput2 ( 0x2020 );               /* 2 blank */
	}

        return(0);
}


/**************************************************/
int     vgridout()
{
        int     lheight, bheight, i, xd, yd;
        char    *pbicmd;

        lheight = 2400/Lpi;    /* dpi/(Lpi/10)        */
        bheight = (lheight+7)>>3;
        pbicmd = HDBIT_IMG_CMD;
        xd = 0;
        yd = 8*1440/240;        /* 240 dpi = HDBIT_IMG_CMD      */

        if ( bheight < 1 ) return(0);
        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x7c );                 /* save x,y to 31 */

        if ( bheight >= 2 ){
            cmdout( pbicmd );
            pput1 ( 0x01 );
            pput1 ( 0x00 );
            pput1 ( 0xff );
            cmdout( SR_CURPOS_CMD );
            pput1 ( 0x7f );                 /* restore x,y from 31 */
        }
        if ( bheight >= 3 ){
            for ( i=1; i<bheight-1; i++){
                cmdout( SET_CURPOS_CMD );
                pput2 ( xd );
                pput2 ( yd*i );
                cmdout( pbicmd );
                pput1 ( 0x01 );
                pput1 ( 0x00 );
                pput1 ( 0xff );
                cmdout( SR_CURPOS_CMD );
                pput1 ( 0x7f );              /* restore x,y from 31 */
            }
        }
        cmdout( SET_CURPOS_CMD );
        pput2 ( xd );
        pput2 ( yd*(bheight-1) );
        cmdout( pbicmd );
        pput1 ( 0x01 );
        pput1 ( 0x00 );
        pput1 ( 0xff<<(bheight*8-lheight) );
        cmdout( SR_CURPOS_CMD );
        pput1 ( 0x7f );              /* restore x,y from 31 */
        return(0);
}

int     hgridout()
{
        int     cwidth, i;
        char    *pbicmd;

        cwidth = 1200/Pitch;   /*  dpi/(Pitch/10)/2  */
        pbicmd = HDBIT_IMG_CMD;

        cmdout( pbicmd );
        pput1 ( cwidth&0xff );
        pput1 ( (cwidth&0xff00)>>8 );
        for ( i=0; i<cwidth; i++){
            pput1 ( 0x80 );
        }
        return(0);
}
/**************************************************/

void    linefeed()
{
        vpos += Vincr * Linespacing;
        endline();
}

void    endline()
{
        int     s, s2, s3;

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
              /*cmdout( OVERSTRIKE_CMD );*/
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
        /* cpi_10 is 0x32 (10 lpi x 10 % 2)          */
}

int     set_Lpi( int lpi )
{
        Lpi = lpi;
        Vincr = Vdecr = (Vres * 10) / Lpi;
        /* lpi_10 = (actual line density) x 10   */
        /* For example, if "6 lpi" is specified, */
        /* lpi_10 is 0x3c (6 lpi x 10)           */
}

int     set_Pgwidth( int rm )
{
        Pgwidth = rm;
        pgwidth = rm * Hres * 10 /      ( Condensed ? 180 : Pitch * 2 );
}

int     set_Lmarg( int lm )
{
        Indent = lm;
        lmarg = lm * Hres * 10 / ( Condensed ? 180 : Pitch * 2 );
}

void    cmdout( char *attr )
{
        piocmdout( attr, NULL, 0, NULL );
}

void    pput1( int c )
{
        pioputchar( c );
}

void    pput2( int c )
{
        unsigned short  s;

        s = (unsigned short)c;
        pioputchar( s >> 8 );   /* high byte */
        pioputchar( s & 0xff ); /* low byte */
}

void    pput4( int c )
{
        char    *p = (char *)&c;

        pioputchar( p[0] );
        pioputchar( p[1] );
        pioputchar( p[2] );
        pioputchar( p[3] );
}

void    pputn( int n, char *p )
{
        int     i;

        for( i = 0; i < n; i++ ){
                pioputchar( p[i] );
        }
}

void    pputs( char *p )
{
        if( p ){
                pputn( strlen( p ), p );
        }
}


static int      Data_match( ESCSEQ_DATAP p );
static int      Elm_match( int c, ESCSEQ_DATAP p );
static int      Fixval_match( int c, int val );
static int      Tilsiz_match( int c, ESCSEQ_DATAP p );
static int      Fixsiz_match( int c, int siz, ESCSEQ_DATAP p );
static int      Varsiz_match( int c, OP_TOKENP p, ESCSEQ_DATAP q );
static int      Op_eval( OP_TOKENP p );
static int      Op_operate( char *op, int lval, int rval );

static int      Eof_buffer( void );
static int      Clr_buffer( void );
static int      Setfp_buffer( FILE *fp );
static int      Access_buffer( char *rp, int offset );
static char     *Current_buffer_p( void );
static int      Seek_buffer( int offset );
static int      Seek_buffer_to_lockpoint( void );
static int      Lock_buffer_at_cp( void );
static int      Lock_buffer_at_bp( void );
static int      Unlock_buffer( void );
static int      Unread_buffer( void );
static int      Read_buffer( void );
static int      Ref_buffer( void );
static int      Back_buffer( int c );
/*static int    Dump_buffer( void );*/

static int      Clr_stack( void );
static int      Add_stack( int siz, char *p, ESCSEQ_DATAP q );
static int      Rem_stack( ESCSEQ_DATAP p );
static int      Get_escdata( ESCSEQ_DATAP p );
static int      Get_sizdata( ESCSEQ_VARP p );


static ESCSEQ_VARP      Var_root = NULL;
static ESCSEQ_VARP      *Var_cur = &Var_root;


static ESCSEQ_DATAP	Esc_from_str( char *seq, int (*func)() )
{
	int	i, n;
	ESCSEQ_DATAP	q;


	n = strlen( seq ) + 1;

	if( ( q = calloc( sizeof( ESCSEQ_DATA ), n ) ) == NULL ){
                errorexit( MSG_MEMALLOCERR, "" );
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


int     tbl_search( ESCSEQ_DATAP Esc_root, int flag )
{
        ESCSEQ_DATAP    p, q;
        int     i;
        int     (*f)();
        int     rc;


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
static int      Data_match( ESCSEQ_DATAP p )
{
        int     c = Read_buffer();


        if( Eof_buffer() ) return( 0 );

        if( Elm_match( c, p ) ){
                ESCSEQ_DATAP    q = p->down;

                if( q->type == FUNC_LEAF )      return( q->value );

                for( ; q != p; q = q->next ){
                        int     rc;

                        if( rc = Data_match( q ) )      return( rc );
                }
        }

        /* not matched */
        Unread_buffer();
        Rem_stack( p );
        return( 0 );
}


static int      Elm_match( int c, ESCSEQ_DATAP p )
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


static int      Fixval_match( int c, int val )
{
        if( c == val ){
                return( 1 );
        }else{
                return( 0 );
        }
}


static int      Tilsiz_match( int c, ESCSEQ_DATAP p )
{
        int     i;
        char    *x;


        if( p->down == NULL || p->down->type != FIX_VALUE ) return( 0 );

        Seek_buffer( -1 );
        x = Current_buffer_p();
        Seek_buffer( 1 );

        for( i = 1; c != p->down->value; i++ ){
                c = Read_buffer();
                if( Eof_buffer() ){
                        for( ; i > 1; i-- )     Unread_buffer();
                        return( 0 );
                }
        }

        Unread_buffer();
        Add_stack( i, x, p );
        return( 1 );
}


static int      Fixsiz_match( int c, int siz, ESCSEQ_DATAP p )
{
        int     i;
        char    *x;


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


static int      Varsiz_match( int c, OP_TOKENP p, ESCSEQ_DATAP q )
{
        return( Fixsiz_match( c, Op_eval( p ), q ) );
}


static int      Op_eval( OP_TOKENP p )
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


static int      Op_operate( char *op, int lval, int rval )
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
#define IBUFF_SIZE      512

static char     Ibuffer[IBUFF_SIZE];
static char     *Ibbp = Ibuffer;
static char     *Ibcp = Ibuffer;
static char     *Iblp = NULL;
static FILE     *Ibfp = stdin;


/* no check for Ibbp, Iblp, Ibcp */
static int      Access_buffer( char *rp, int offset )
{
        if( rp >= Ibuffer && rp < Ibuffer + IBUFF_SIZE ){
                offset += ( rp - Ibuffer ) + IBUFF_SIZE;
                return( Ibuffer[ offset % IBUFF_SIZE ] );
        }else{
                return( EOF );
        }
}

static char     *Current_buffer_p()
{
        return( Ibcp );
}

/* check only Ibbp ( No Ibcp check ) */
static int      Advance_Ibbp()
{
        int     c;
        char    *p = Ibuffer + ( ( Ibbp - Ibuffer + 1 ) % IBUFF_SIZE );

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

static int      Seek_buffer( int offset )
{
        if( Iblp == NULL ){
                Ibcp = Ibuffer + ( ( Ibcp - Ibuffer + IBUFF_SIZE + offset ) % IBUFF_SIZE );
                return( 1 );
        }else{
                int     n = ( ( Ibcp - Iblp ) + IBUFF_SIZE ) % IBUFF_SIZE;

                if( n + offset >= 0 && n + offset < IBUFF_SIZE ){
                        Ibcp = Iblp + n + offset;
                        if( Ibcp >= Ibuffer + IBUFF_SIZE ) Ibcp -= IBUFF_SIZE;
                        return( 1 );
                }else{
                        return( 0 );
                }
        }
}
/*
static int      Seek_buffer_to_lockpoint()
{
        if( Iblp != NULL ){
                Ibcp = Iblp;
                return( 1 );
        }else{
                return( 0 );
        }
}
*/
static int      Clr_buffer()
{
        Ibcp = Ibbp = Ibuffer;
        Iblp = NULL;
        Ibfp = stdin;
        return( 1 );
}

static int      Setfp_buffer( FILE *fp )
{
        Ibfp = fp;
        return( 1 );
}

static int      Lock_buffer_at_cp()
{
        Iblp = Ibcp;
        return( 1 );
}

static int      Lock_buffer_at_bp()
{
        Iblp = Ibbp;
        return( 1 );
}

static int      Unlock_buffer()
{
        Iblp = NULL;
        return( 1 );
}

static int      Eof_buffer()
{
        return( Ibcp == Ibbp && feof( Ibfp ) );
}

static int      Read_buffer()
{
        char    *p;

        if( !Eof_buffer() ){
                if( Ibcp == Ibbp ){
                        if( !Advance_Ibbp() ){
                                return( EOF );
/*                              errorexit( MSG_IBUFFULL, "" );*/
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

static int      Ref_buffer()
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

static int      Unread_buffer()
{
        int     n = ( Ibbp - Ibcp + IBUFF_SIZE ) % IBUFF_SIZE;

        if( n + 1 < IBUFF_SIZE && Seek_buffer( -1 ) ){
                count--;
                return( 1 );
        }else{
                errorexit( MSG_IBUFFATAL, "" );
                return( 0 );
        }
}

static int      Back_buffer( int c )
{
        if( Unread_buffer() ){
                *Ibcp = c;
                return( 1 );
        }else{
                return( 0 );
        }
}

/**********************************/

static int      Clr_stack()
{
        ESCSEQ_VARP     p, q;


        for( p = Var_root; p != NULL; ){
                q = p->next;
                free( p );
                p = q;
        }

        Var_root = NULL;
        Var_cur = &Var_root;
        return( 1 );
}


static int      Add_stack( int siz, char *p, ESCSEQ_DATAP q )
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


static int      R_st( int i )
{
ESCSEQ_VARP     p;
        char    buff[6];

        for( p = Var_root; i > 0 && p != NULL; i-- )    p = p->next;

        if( i < 0 || p == NULL )        return( 0 );

        return( Get_sizdata( p ) );
}


static int      Rem_stack( ESCSEQ_DATAP p )
{
        ESCSEQ_VARP     x, xold;


        if( p != NULL && Var_root != NULL ){
                switch( p->type ){
                        case    FIX_SIZE:
                        case    VAR_SIZE:
                        case    VAR_SIZE_TIL:
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


static int      Get_escdata( ESCSEQ_DATAP p )
{
        ESCSEQ_VARP     x;


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


static int      Get_sizdata( ESCSEQ_VARP x )
{
        char buff[5];
        int     i, rc;

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
 *      Upon successful completion, the hspace should return a value of 1,
 *      indicating that there's enough space in the line for the character,
 *      or 0 indicating that there's not enough space in the line for the
 *      character.
 */
int hspace (cw)
int cw;                                         /* character width */
{
    if ((pgwidth - hpos) >= cw) return (1);     /* Space */
    return (0);                                 /* No space */
}

#if defined( AIX32 )
# include	"defmsg.c"
#else
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
 *      NONE
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
        piocmdout (FF_CMD, NULL, 0, NULL);
    pioexit (PIOEXITBAD);
}
#endif	/* AIX32 */

void warnmsg (msgnum, stringval)
int msgnum;
char *stringval;
{
/*
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
*/
}

/***************************************************************************
 * NAME: c_image_us
 * FUNCTION: Convert the snf glyph data to printable data for 4019 Bit Image
 * (NOTE:) indata must be pure font data, i.e. data w/o any additional
 *         surrounding bits.
 */

void c_image_us (indata, outdata, bwidth, bheight,bpl)
char indata[], outdata[];
int  bwidth, bheight,bpl;
{
        int i, j, m, n, indexin, indexout;
        register char in, out;

        for (m=0; m<bheight; m++) {
           for (n=0; n<bwidth; n++) {
              for (i=0; i<8; i++) {
                 out = 0x00;
                 for (j=0; j<8; j++) {
                    indexin = m*bpl*8 + n + j*bpl;
                    in = indata[indexin];
                    out |= (((in<<i) & 0x80) >> j);
                 }
                 indexout = m*bwidth*8 + n*8 + i; /* 4019 Bit Image Order */
                 outdata[indexout] = out;
              }
           }
        }
}  /* end of c_image_us() */
