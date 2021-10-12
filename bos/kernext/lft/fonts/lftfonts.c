static char sccsid[] = "@(#)49  1.3  src/bos/kernext/lft/fonts/lftfonts.c, lftdd, bos411, 9428A410j 1/3/94 15:35:10";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lft_fonts_init
 *		lft_fonts_term
 *		load_font
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ---------------------------------------------------------------------- * 
 * Includes								  * 
 * ---------------------------------------------------------------------- */

#define Bool  unsigned               /* for aixfont.h */

#include <lft.h>       
#include <sys/aixfont.h>
#include <sys/conf.h>       
#include <sys/device.h> 
#include <unistd.h>
#include <sys/uio.h>   
#include <sys/fullstat.h>   
#include <sys/sysmacros.h>   
#include <sys/malloc.h>    
#include <graphics/gs_trace.h>
#include <lft_debug.h>

extern lft_ptr_t	lft_ptr;
extern int	kill_fkproc();

BUGVDEF(db_lftfonts, 99);

GS_MODULE(lftfonts);

/* --------------------------------------------------------------------	*
 * Functions:								*
 *	lft_fonts_init()						*
 *	font_load()							*
 *	lft_fonts_term()						*
 * --------------------------------------------------------------------	*/

/* --------------------------------------------------------------------	*
 * Name:		lft_fonts_init					*
 * Description:		Initilaize the fonts				*
 * Parameters:								*
 *	input		char	*ff_names;				*
 *									*
 * Process:								*
 *	o lft_fonts_init is invoked by lftconfig() during initialization*
 *	o The file names are passed in ff_names				*
 *	o These files are opened					*
 *	o Space is allocated for the fonts and they are read in		*
 *	o The font data structure is initialized with the read data	*
 *	o Number of fonts count in lft_dds is updated with the number	*
 *	  of fonts successfully initialized				*
 *	o If the start_fkproc is set in lft_dds, the font kernel process*
 *	  is started.							*
 *	o The font enqueue and font dequeue pointers are initialized	*
 *									*
 * Return :								*
 *	SUCCESS if successful						*
 *	Otherwise an error is returned					*
 * -------------------------------------------------------------------- */

int
lft_fonts_init(ff_names)
char	*ff_names;
{
    lft_dds_t		*ddsptr;
    uint		fontsz;
    int			rc;
    uint		index;
    int			num_fonts, i;
    struct file		*font_fp;	/* File pointer - font file */
    char		*ffnames;
    extern int		fsp_enq();	/* Font support		    */
    extern int		fsp_deq();
    
    GS_ENTER_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_init, lft_ptr);

    ddsptr = lft_ptr -> dds_ptr;
    fontsz = sizeof( struct font_data) * LFTNUMFONTS;
    lft_ptr->fonts = xmalloc(fontsz ,0, pinned_heap );
    if (lft_ptr->fonts == NULL)
    {
	lfterr(NULL,"LFTDD","lftfonts","xmalloc",0, LFT_ALLOC_FAIL, UNIQUE_1);
	BUGLPR(db_lftfonts, BUGNFO, ("lftfonts: cannot allocate font space\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_init, ENOMEM);
	return(ENOMEM);
    }
    /*
      The font file names are stored in a 2 dim. array.  The length  
      of each element in the array is FILE_NAME_LEN (255).  ff_names is 
      a pointer to this.  We bump the pointer by FILE_NAME_LEN to get to
      the next name in the array.
      */
    bzero(lft_ptr->fonts, fontsz);
    for (num_fonts = 0, i = 0 ; i < ddsptr->number_of_fonts; i++)
    {
	rc = fp_open(ff_names, O_RDONLY, 0, 0, SYS_ADSPACE, &font_fp);
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftfonts","fp_open",rc, LFT_FP_OPEN, UNIQUE_2);	
	    BUGLPR(db_lftfonts, BUGNFO,
			("lftfonts: Could not open font file %s\n",ff_names));
	    ff_names += FILE_NAME_LEN;	/* Position for next file name 	*/
	    continue;
	}
	ff_names += FILE_NAME_LEN;	/* Position for next file name 	*/
	rc = load_font( &lft_ptr->fonts[num_fonts], font_fp, i );
	if(!rc)				/* Success			*/
	{
	    num_fonts++;		/* Next good index		*/
	}
	fp_close( font_fp );
    }
    if (!num_fonts)			/* Did not load any fonts	*/
    {
	lfterr(NULL,"LFTDD","lftfonts",NULL,0,LFT_FONT_INIT, UNIQUE_3);
	BUGLPR(db_lftfonts, BUGNFO, ("lftfonts: did not load any fonts\n"));
	xmfree(lft_ptr->fonts, pinned_heap); 	/* Free the font space	*/
	lft_ptr->fonts = NULL;		    	/* Clear ptr		*/
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_init, EIO);
	return( EIO );
    }
    ddsptr->number_of_fonts = num_fonts;	/* Actual fonts read	*/

    /*
      If the font kernel process needs to be started - do it now and
      initialize the fsp_enq and fsp_deq pointers
      */

    if(lft_ptr->dds_ptr->start_fkproc)
    {
	lft_ptr->lft_fkp.fsp_enq = fsp_enq;
	lft_ptr->lft_fkp.fsp_deq = fsp_deq;
	rc = create_fkproc();
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftfonts","create_fkproc",rc, 
					LFT_CREATE_FKPROC, UNIQUE_4);
	    BUGLPR(db_lftfonts, BUGNFO, ("lftfonts: could not start fkproc\n"));
	    GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_init, rc);
	    return(rc);
	}
    }
    GS_EXIT_TRC0(HKWD_GS_LFT, lftfonts, 1, lft_fonts_init );
    return(SUCCESS);
}

/* --------------------------------------------------------------------	*
 * Name:	load_font						*
 * Description:	Read font into kernel space				*
 * Parameters:								*
 *	input	struct	font_data	*fdptr;				*
 *		int	fd;						*
 *		uint	i;						*
 *									*
 * Process:								*
 *	Read the font file and initialize the font data structure	*
 *									*
 * Return:								*
 *	SUCCESS if successful						*
 *	Otherwise an error is returned					*
 * -------------------------------------------------------------------	*/

int
load_font(fdptr, fp, index)
struct font_data *fdptr;	/* Points to info for this font		*/
struct file	 *fp;		/* file pointer for this font file	*/
uint		 index;		/* index of this font			*/
{
    aixFontInfoPtr	aixfont;
    aixFontPropPtr	aixprop;
    struct fullstat	f_stat;
    int			f_length;
    int			rc;
    int			countp;		/* Used in the fp_read call	*/
    int			i;		/* Used in for loops		*/
    char		*string_pool;

    GS_ENTER_TRC0(HKWD_GS_LFT, lftfonts, 1, load_font);
    rc = fp_fstat(fp, &f_stat, sizeof(struct fullstat));
    if(rc)
    {  
	lfterr(NULL, "LFTDD", "lftfonts", "fp_stat", rc, LFT_FP_FSTAT, UNIQUE_5);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: fstat failed\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, rc);
	return(rc);
    }
    f_length = f_stat.st_size;		/* Length of file in bytes	*/
    if(f_length < sizeof(aixFontInfo))        /* typedef'd in aixfont.h */
    {
	lfterr(NULL, "LFTDD", "lftfonts", "fp_stat", 0, LFT_FP_FSTAT, UNIQUE_6);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: Bad font file size\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, EIO);
	return(EIO);
    }
    /*
      Seek to start of file
    */
    rc = fp_lseek(fp, 0, SEEK_SET);
    if(rc)
    {
	lfterr(NULL, "LFTDD", "lftfonts", "fp_lseek", rc, LFT_FP_LSEEK, UNIQUE_7);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: File seek error\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, EIO);
	return(EIO);
    }
    /*
      Allocate space for font data
    */
    aixfont = xmalloc(f_length, 0, pinned_heap);
    if(aixfont == NULL)
    {
	lfterr(NULL, "LFTDD", "lftfonts", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_8);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: Could not allocate space\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, ENOMEM);
	return(ENOMEM);
    }
    /*
      Read the font file
    */
    rc = fp_read(fp, aixfont, f_length, 0, UIO_SYSSPACE, &countp);
    if(rc || countp < f_length)
    {
	lfterr(NULL, "LFTDD", "lftfonts", "fp_read", rc, LFT_FP_READ, UNIQUE_9);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: fp_read error\n"));
	xmfree(aixfont, pinned_heap);
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, EBADF);
	return(EBADF);
    }
    /*
      Check for variable width fonts - not supported
    */
    if(aixfont->minbounds.characterWidth != 
       aixfont->maxbounds.characterWidth)
    {
	lfterr(NULL, "LFTDD", "lftfonts", NULL, 0, LFT_FONT_INIT, UNIQUE_10);
	BUGLPR(db_lftfonts, BUGNFO, ("load_font: Unsupported variable width font\n"));
	xmfree(aixfont, pinned_heap);
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, load_font, EIO);
	return(EIO);
    }
    /*
      Initialize the font data in this slot.  The font file contains the
      following data structures ( This is a s summary - for details refer
      to aixfont.h)
        The FontInfo Structure
	The CharInfo array
	Character glyphs
	Properties
    */
    fdptr->font_id = index;		/* Passed in the call to func	*/
    /*
      Get to the properties
    */
    aixprop = (aixFontPropPtr) ((char *) aixfont +
				( BYTESOFFONTINFO(aixfont) +
				 BYTESOFCHARINFO(aixfont) +
				 BYTESOFGLYPHINFO(aixfont) ));
    /*
      Set string_pool to point past the font property information
    */
    string_pool = (char *) aixprop + BYTESOFPROPINFO(aixfont);
   
    for(i = aixfont->nProps; i; i--, aixprop++) /* Go through the props */
    {
	if(strcmp(&string_pool[aixprop->name], "WEIGHT_NAME") == 0)
	{
	    strncpy(fdptr->font_weight, &string_pool[aixprop->value], 8);
	    continue;
	}	
	if(strcmp(&string_pool[aixprop->name], "FONT") == 0)
	{
	    int i = 0;
	    char *p;

	    strncpy(fdptr->font_name, &string_pool[aixprop->value], 16);
	    /*
	      Look for end of name so we can get char set of font
	    */

	    p = (char *) &string_pool[aixprop->value] +
		(strlen(&string_pool[aixprop->value]) - 1);
		
	    while (*p != '-') 
	    {
		i++;
		p--;
	    }		
	    
	    do 
	    {
		p--;
	    } while (*p != '-');
	    
	    p++;
	    strncpy(fdptr->font_page, p, 8);
	    /*
	      In case of iso font, include #
	    */
	    if(i == 1)
	    {
		p = (char *) &string_pool[aixprop->value] +
		    (strlen(&string_pool[aixprop->value]) - 1);
		fdptr->font_page[7] = *p;
	    }
	    continue;
	}	
	if (strcmp(&string_pool[aixprop->name], "SLANT") == 0) 
	{
		strncpy(fdptr->font_slant,&string_pool[aixprop->value], 8);
	}

    }  /* end of for loop */

    fdptr->font_width  = aixfont->minbounds.characterWidth;
    fdptr->font_height = aixfont->minbounds.ascent +aixfont->minbounds.descent;
    fdptr->font_ptr = (long *) aixfont;	 
    fdptr->font_size = f_length;	/*length of entire font structure */

    GS_EXIT_TRC0(HKWD_GS_LFT, lftfonts, 1, load_font);
    return(SUCCESS);
}	
	
/* --------------------------------------------------------------------	*
 * Name:	load_fonts_term						*
 * Description:	Clean up the font setup in lft				*
 * Parameters:								*
 *	input	None							*
 *									*
 * Process:								*
 *	Free allocated memory and terminate fkproc (if required)	*
 *									*
 * Return:								*
 *	SUCCESS if successful						*
 *	Otherwise an error is returned					*
 * -------------------------------------------------------------------	*/
int
lft_fonts_term()
{
    uint    index;
    int     rc;

    GS_ENTER_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_term, lft_ptr);

    if (lft_ptr == NULL) 
    {
	lfterr(NULL, "LFTDD", "lftfonts", NULL, 0, LFT_INV_STATE, UNIQUE_11);
	BUGLPR(db_lftfonts, BUGNFO, ("lft_fonts_term: Invalid lft_ptr\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftfonts, 1, lft_fonts_term, EINVAL);
	return(EINVAL);
    }

    /*
	If fkproc was started - kill it
      */

    if(lft_ptr->dds_ptr->start_fkproc)
    {
	kill_fkproc();
    }
	
    if (lft_ptr->fonts != NULL) 
    {
	for (index = 0 ; index < LFTNUMFONTS ; index ++) 
	{
	    if (lft_ptr->fonts[index].font_ptr != NULL) 
	    {
		xmfree( lft_ptr->fonts[index].font_ptr, pinned_heap);
	    }
	}
    }
    xmfree(lft_ptr->fonts, pinned_heap);
    lft_ptr->fonts = NULL;

    GS_EXIT_TRC0(HKWD_GS_LFT, lftfonts, 1, lft_fonts_term);
    return(SUCCESS);
} 	
