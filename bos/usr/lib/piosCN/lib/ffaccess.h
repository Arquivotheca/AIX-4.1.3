/* @(#)80	1.1  src/bos/usr/lib/piosCN/lib/ffaccess.h, ils-zh_CN, bos41J, 9507B 1/26/95 09:56:04  */
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
 
typedef struct _UdfInfo {
	int          width;       /* width of the character               */
	int          height;      /* height of the character              */
	int          bytes;       /* length of the character image data   */
	unsigned int code;        /* the character code                   */
	char         image[256];  /* printable character image data       */
} UdfInfoRec;


typedef struct _fontimg{
	int	w;
#define	SEND_CODEVALUE	(-2)
	int	h;
	int	bytes_in_line;
	int	baseline;
	unsigned char	img[4096];
} Fontimgrec, *Fontimg;

#define	MAXFONTSIZE	128
#define	CSREGISTRY	"CHARSET_REGISTRY"
#define	CSENCODING	"CHARSET_ENCODING"


extern void	init_Fontpath( char* fontpath );
extern void	init_Fontlist( char* fontlist, FONTinfo fip );
extern Fontimg	lc_get_Font( unsigned char* chars, CURcodeset ccdp );
extern Fontimg	get_Font( unsigned char* chars, unsigned long pFont, int wid );
extern void	C_Fontimg( Fontimg f_in, Fontimg f_out );
extern void	C_Fontimg_us( Fontimg f_in, Fontimg f_out );
