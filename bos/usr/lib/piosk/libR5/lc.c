static char sccsid[] = "@(#)99  1.8  src/bos/usr/lib/piosk/libR5/lc.c, cmdpiosk, bos411, 9428A410j 5/18/94 05:26:30";
/*
 *  COMPONENT_NAME: CMDPIOSK
 *
 *  FUNCTIONS: xcsid, lc_init, get_iFont
 *
 *  ORIGINS: 27
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include	"lc.h"
#include        "lcmb.h"
#include        "jfmtrs_msg.h"
#include	<stdio.h>
#include	<stdlib.h>
#include        <string.h>    
#include	<locale.h>
#include	<langinfo.h>
#include        <iconv.h>
#include        <limits.h>

#define		MB_MAX	20


/*
 * NAME:	lc_init
 *
 * FUNCTION:	main rutine of MBCS printer backend library 
 *		for function setup()
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:	CURcodset
 */

/* codesetname can be codesetname like IBM-850 or localename like en_US */
CURcodeset lc_init( char *codesetname, 
                    char *strTi, 
                    char *strTo, 
                    char *fontlists, 
                    char *fontpathes)
{
    CURcodeset	ccsp;
    int rc;
    char *tmp;
    
    static void	init_CSinfo( CSinfo csip );
    static void	open_iconvs( CURcodeset ccsp );
    
    if( ( ccsp = calloc( sizeof( CURcodesetrec ), 1 ) ) == 0 ){
	errorexit( MSG_MEMALLOCERR, "" );
    }
    
    /*
     *  If the -X option is specified with value '-', 
     *  a codeset corresponding to current locale 
     *  is set as intput codeset.
     */
    if( codesetname && codesetname[0] == '-' && codesetname[1] == 0 ){
	codesetname = setlocale(LC_CTYPE, "");
	DEBUGPR((stderr, "locale_name = %s\n", codesetname));
    }
    /*
     * If process codeset or locale name is specified,
     * function set_loc_and_cs() set corresponding 
     * locale name or codeset name and returns 1,
     * otherwise, returns 0.
     */
    if (!set_loc_and_cs (ccsp, codesetname, sp_alias_to_name(codesetname))){
	/* 
	 *   Specified codesetname is one of the special codeset
	 */
	if((rc = init_pre_conv (ccsp, strTi, codesetname)) <= 0){
	    return (NULL);
	}
    }

    if (init_post_conv( ccsp, strTo, fontlists, fontpathes)){
	return( ccsp );
    }
    else{
	return(NULL);
    }
}


void	lc_fin( CURcodeset ccsp )
{
    int	i;
    
    if( !ccsp || ccsp->CSnum <= 0 )	return;
    for( i = 0; i < ccsp->CSnum; i++ ){
	if( ccsp->post_iconv[i] ) iconv_close((iconv_t)ccsp->post_iconv[i]);
    }
}


int	is_FONT_P( int csid, CURcodeset ccsp )
{
    if( !ccsp || csid >= ccsp->CSnum || csid < 0 ){
	return( 0 );
    }
    
    return( ccsp->CSP && ccsp->CSP[csid].flags == FONT_P );
}

/*
 * NAME: get_wid
 *
 * FUNCTION:	get the character width of specifed CSID
 *
 * RETURNS:     i      ( i > 0 )
 *              0      ( error )
 */

int	get_wid( int csid, CURcodeset ccsp )
{
    if( !ccsp || /*csid >= ccsp->CSnum || */ csid < 0 || ccsp->CSP == 0 ){
	return( 0 );
    }
    return( ccsp->CSP[csid].wid );
}

/*
 * NAME:	get_iFont
 *
 * FUNCTION:	Convert input code point to output code point 
 *		using iconv and translation table if it is required.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:	int 	csid:	( >= 0)
 *			-1:	(iconv error)
 *		    	-2:	(JIS Status Ctl Code)
 *		    	-3:	(Non printable Code)
 *		    	-4:	(Fatal error)
 */

extern	int sudc;
extern	int udc;

int	get_iFont(unsigned char* chars, unsigned char* dest, CURcodeset ccsp)
{
    unsigned char buff[MB_MAX+1];
    unsigned char *ib, *ob;
    int	i, in, on, rc, csi, l, len;
    wchar_t wchar;

    if( !ccsp ) return(-4);

    /*
     * pre-translation using a translation table
     */
    if( ccsp->pre_trans ){
	mb_trans( ccsp->pre_trans, chars, ccsp->pre_trans_num);
    }

    /*
     * pre translation using iconv
     */
    if( ccsp->pre_iconv ){
	ib = chars;
	in = strlen( chars );    
	on = MB_MAX+1;
	ob = buff;
	if((rc = iconv((iconv_t)ccsp->pre_iconv, &ib, &in, &ob, &on)) < 0)
	    return(-1);
	*ob = 0;
	chars = buff;
    }
    
    if ((l = mblen(chars, MB_CUR_MAX)) == 0){
	return(-2);
    }
    if( l < 0 || l > strlen( chars ) ) return( -1 );
    
    csi = csid( chars );

    /*
     * Japanese unique 
     */
    if( udc && csi == sudc && ccsp->proccodeset){
	/* Japanese User defined charcters area */
	if( strcmp( ccsp->proccodeset, "IBM-eucJP" ) == 0){
	    if( chars[1] > 0xf4 || chars[2] == 0 ) csi = udc;
	}else if( strcmp( ccsp->proccodeset, "IBM-932" ) == 0){
	    if( chars[0] >= 0xf0 && chars[0] <= 0xf9 ) csi = udc;
	}
    /*
	}else if( strcmp( ccsp->proccodeset, "UTF-8" ) == 0){
	    if( specify the code area ) csi = udc;
	}
    */
    }
    
    /*
     * Set the width of character set in each CSID
     */
    mbtowc(&wchar, chars, (size_t)l);
    ccsp->CSP[csi].wid = (((len = wcwidth(wchar)) == -1) ? 0 : len);

    if( !ccsp->CSP[csi].wid )
	return(-3);
    
    if( ccsp->CSP[csi].flags == NO_FONT || csi >= ccsp->CSnum ){
	for( i = 0; i < get_wid( csi, ccsp); i++ ){
	    buff[i] = ' ';
	}
	buff[i] = 0;
	chars = buff;
    }else{
	/*
	 * post-translation using iconv
	 */
	if( ccsp->post_iconv[csi] ){
	    ib = chars;
	    in = strlen( chars );
	    on = MB_MAX+1;
	    ob = buff;
	    rc = iconv((iconv_t)ccsp->post_iconv[csi],
			   &ib, &in, &ob, &on);
#if defined (DEBUG)
	    if(rc < 0){
		DEBUGPR((stderr, "something wrong : 0x%x id:%d ( %s )\n",
				 chars[0], csi, chars));
	    }
#endif	/* DEBUG */
	    *ob = 0;
	    chars = buff;
	}
	/*
	 * post-translation using a translation table
	 */
	if( rc >= 0 && ccsp->post_trans[csi] ){
	    mb_trans( ccsp->post_trans[csi], chars, ccsp->post_trans_num[csi]);
	}
    }
    strcpy( dest, chars );
    return( csi );
}

/*
 * NAME: get_pFont
 *
 * FUNCTION: Find a correspoding font file pointer to specified CSID
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: If the font file pointer is NULL and font file name is specifed
 *        call open_Font() to open the font file and get font file pointer.
 *
 * RETURNS: unsigned logn (pFont)
 *          
 */

unsigned long	get_pFont( CURcodeset ccsp, int csid )
{
    if( !ccsp || csid >= ccsp->CSnum || csid < 0 )	
	return( 0 );
    if( ccsp->CSP && ccsp->CSP[csid].flags == FONT_X 
       && ccsp->CSP[csid].pFont && !*ccsp->CSP[csid].pFont ){
	DEBUGPR((stderr, "open_Font(%s): csid:%d\n",
		 ccsp->CSP[csid].filename, csid));
	if (!(*ccsp->CSP[csid].pFont = open_Font(ccsp->CSP[csid].filename))){
	    ccsp->CSP[csid].flags = NO_FONT;
	    ccsp->CSP[csid].pFont = 0;
	}
    }
    return (ccsp->CSP[csid].pFont ? *ccsp->CSP[csid].pFont : 0);
}


/*
 * Following functions shows the internal data struct for debug 
 */

#if defined (DEBUG)
void	show_FONTinfo( FONTinfo fip )
{
    if (fip) {
	fprintf( stderr, "\n+- show_FONTinfo\n");
	for( ; fip->cs_registry || fip->cs_encoding; fip++ ){
	    fprintf( stderr, "+ flags = %d\n", fip->flags );
	    fprintf( stderr, "| --> filename = %x\n",fip->filename);
	    fprintf( stderr, "| --> pFont = %x\n", fip->pFont );
	    fprintf( stderr, "| --> cs_registry = %s\n", fip->cs_registry );
	    fprintf( stderr, "| --> cs_encoding = %s\n", fip->cs_encoding );
	}
	fprintf( stderr, "+-\n");
    }
}

void	show_CSinfo( CSinfo csip , int csnum)
{
    int n;
    fprintf( stderr, "\n+- show_CSinfo\n");
    for(n = 0; n < csnum; n++, csip++){
	fprintf( stderr, "+ flags = %d\n", csip->flags );
	fprintf( stderr, "| --> pFont = %x\n", csip->pFont );
	fprintf( stderr, "| --> iconvname = %s\n", csip->iconvname );
	fprintf( stderr, "| --> FTptr = %x\n", csip->FTptr );
    }
    fprintf( stderr, "+-\n");
}

void	show_CURcodeset( CURcodeset ccsp )
{
    if( ccsp ){
	int	i;
	fprintf( stderr, "realname = %s\n", ccsp->realname );
	fprintf( stderr, "codeset = %s\n", ccsp->codesetname );
	fprintf( stderr, "processcodeset = %s\n", ccsp->proccodeset );
	
	fprintf( stderr, "pre_trans = %x\n", ccsp->pre_trans );
	fprintf( stderr, "pre_iconv = %x\n", ccsp->pre_iconv );
	
	for( i = 0; i < ccsp->CSnum; i++ ){
	    fprintf( stderr, "post_iconv[%d] = %x\n", i, ccsp->post_iconv[i] );
	    fprintf( stderr, "post_trans[%d] = %x\n", i, ccsp->post_trans[i] );
	}
    }
}
#endif /* DEBUG */

