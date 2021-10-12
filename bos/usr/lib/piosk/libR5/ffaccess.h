/* @(#)88       1.3 9/14/93 00:50:02 */

/*
 *   COMPONENT_NAME: CMDPIOSK
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "lc.h"
 
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

#define NO_PAD		0 /* for cmdtext */
#define GG_PAD		2 /* for ibm55xx printers */
#define PR_PAD		1 /* for other printers */

extern void	init_Fontpath( char* fontpath );
extern void	init_Fontlist( char* fontlist, FONTinfo fip );
extern Fontimg	lc_get_Font( unsigned char* chars, CURcodeset ccdp );
extern Fontimg	get_Font( unsigned char* chars, unsigned long pFont, int wid ,int pad);
extern void	C_Fontimg( Fontimg f_in, Fontimg f_out );
extern void	C_Fontimg_us( Fontimg f_in, Fontimg f_out );
