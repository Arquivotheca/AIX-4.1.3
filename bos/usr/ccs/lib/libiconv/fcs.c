static char sccsid[] = "@(#)11	1.6.1.9  src/bos/usr/ccs/lib/libiconv/fcs.c, libiconv, bos41J, 9520A_all 5/9/95 12:46:57";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993, 1994, 1995
 *   All Rights	Reserved
 *   US	Government Users Restricted Rights - Use, duplication or
 *   disclosure	restricted by GSA ADP Schedule Contract	with IBM Corp.
 */

#include <stdlib.h>
#include <fcs.h>
#include <iconvP.h>
#include <iconv932.h>


static	char	iconv_ASCII[]			= {0x1b, 0x28, 0x42};
static	char	iconv_ASCII_GR[]		= {0x1b, 0x29, 0x42};
static	char	iconv_ISO8859_1_GL[]		= {0x1b, 0x2e, 0x41};
static	char	iconv_ISO8859_1_GR[]		= {0x1b, 0x2d, 0x41};
static	char	iconv_ISO8859_2_GL[]		= {0x1b, 0x2e, 0x42};
static	char	iconv_ISO8859_2_GR[]		= {0x1b, 0x2d, 0x42};
static	char	iconv_ISO8859_3_GL[]		= {0x1b, 0x2e, 0x43};
static	char	iconv_ISO8859_3_GR[]		= {0x1b, 0x2d, 0x43};
static	char	iconv_ISO8859_4_GL[]		= {0x1b, 0x2e, 0x44};
static	char	iconv_ISO8859_4_GR[]		= {0x1b, 0x2d, 0x44};
static	char	iconv_ISO8859_5_GL[]		= {0x1b, 0x2e, 0x4c};
static	char	iconv_ISO8859_5_GR[]		= {0x1b, 0x2d, 0x4c};
static	char	iconv_ISO8859_6_GL[]		= {0x1b, 0x2e, 0x47};
static	char	iconv_ISO8859_6_GR[]		= {0x1b, 0x2d, 0x47};
static	char	iconv_ISO8859_7_GL[]		= {0x1b, 0x2e, 0x46};
static	char	iconv_ISO8859_7_GR[]		= {0x1b, 0x2d, 0x46};
static	char	iconv_ISO8859_8_GL[]		= {0x1b, 0x2e, 0x48};
static	char	iconv_ISO8859_8_GR[]		= {0x1b, 0x2d, 0x48};
static	char	iconv_ISO8859_9_GL[]		= {0x1b, 0x2e, 0x4d};
static	char	iconv_ISO8859_9_GR[]		= {0x1b, 0x2d, 0x4d};
static	char	iconv_JISX0201_1976[]		= {0x1b, 0x28, 0x4a};
static	char	iconv_JISX0201_1976_GL[]	= {0x1b, 0x28, 0x49};
static	char	iconv_JISX0201_1976_GR[]	= {0x1b, 0x29, 0x49};
static	char	iconv_JISX0208_1978_GL[]	= {0x1b, 0x24, 0x28, 0x40};
static	char	iconv_JISX0208_1978_GL_MAIL[]	= {0x1b, 0x24, 0x40};
static	char	iconv_JISX0208_1978_GR[]	= {0x1b, 0x24, 0x29, 0x40};
static	char	iconv_JISX0208_1983_GL[]	= {0x1b, 0x24, 0x28, 0x42};
static	char	iconv_JISX0208_1983_GL_MAIL[]	= {0x1b, 0x24, 0x42};
static	char	iconv_JISX0208_1983_GR[]	= {0x1b, 0x24, 0x29, 0x42};
static	char	iconv_KSC5601_1987_GR[]		= {0x1b, 0x24, 0x29, 0x43};
static	char	iconv_KSC5601_1987_GL[]		= {0x1b, 0x24, 0x28, 0x43};
static	char	iconv_CNS11643_GR[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_CNS11643_1986_1_GL[]	= {0x1b, 0x24, 0x29, 0x30};
static	char	iconv_CNS11643_1986_1_GL_MAIL[]	= {0x1b, 0x24, 0x30};
static	char	iconv_CNS11643_1986_1_GR[]	= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_CNS11643_1986_2_GL[]	= {0x1b, 0x24, 0x2a, 0x31};
static	char	iconv_CNS11643_1986_2_GL_MAIL[]	= {0x1b, 0x24, 0x31};
static	char	iconv_CNS11643_1986_2_GR[]	= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_CNS11643_1992_3_GL[]	= {0x1b, 0x24, 0x2b, 0x32};
static	char	iconv_CNS11643_1992_3_GL_MAIL[]	= {0x1b, 0x24, 0x32};
static	char	iconv_CNS11643_1992_3_GR[]	= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_CNS11643_1992_4_GL[]	= {0x1b, 0x24, 0x2b, 0x33};
static	char	iconv_CNS11643_1992_4_GL_MAIL[]	= {0x1b, 0x24, 0x33};
static	char	iconv_CNS11643_1992_4_GR[]	= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_udcJP_GL[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_udcJP_GR[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_udcTW_GL[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_udcTW_GR[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_sbdTW_GL[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_sbdTW_GR[]		= {0x1b, 0x25, 0x2f, 0x32};
static	char	iconv_IBM_850_GL[]		= {0x1b, 0x25, 0x2f, 0x31};
static	char	iconv_IBM_850_GR[]		= {0x1b, 0x25, 0x2f, 0x31};
/* Added items for EUC S/C */
static char     iconv_GB2312_1980_0_GL[]	= { 0x1b, 0x24, 0x28, 0x41 };
static char     iconv_GB2312_1980_0_GR[]	= { 0x1b, 0x24, 0x29, 0x41 };
static char     iconv_GB2312_1980_GR[]		= { 0x1b, 0x25, 0x2f, 0x32 };
static char     iconv_UTF_8_GR[]	        = { 0x1b, 0x25, 0x2f, 0x30 };
static char     iconv_UTF_8_GL[]	        = { 0x1b, 0x25, 0x2f, 0x30 };

/* End addition for EUC S/C */

static	char	iconv_CNS11643_GR_SEG[]		=
	{'C','N','S','1','1','6','4','3',0x02};
static	char	iconv_CNS11643_1986_1_GR_SEG[]	=
	{'C','N','S','1','1','6','4','3','.','1','9','8','6','-','1',0x02};
static	char	iconv_CNS11643_1986_2_GR_SEG[]	=
	{'C','N','S','1','1','6','4','3','.','1','9','8','6','-','2',0x02};
static	char	iconv_CNS11643_1992_3_GR_SEG[]	=
	{'C','N','S','1','1','6','4','3','.','1','9','9','2','-','3',0x02};
static	char	iconv_CNS11643_1992_4_GR_SEG[]	=
	{'C','N','S','1','1','6','4','3','.','1','9','9','2','-','4',0x02};
static	char	iconv_IBM_udcJP_GL_SEG[]	=
	{'i','b','m','-','u','d','c','J','P',0x02};
static	char	iconv_IBM_udcJP_GR_SEG[]	=
	{'I','B','M','-','u','d','c','J','P',0x02};
static	char	iconv_IBM_udcTW_GL_SEG[]	=
	{'i','b','m','-','u','d','c','T','W',0x02};
static	char	iconv_IBM_udcTW_GR_SEG[]	=
	{'I','B','M','-','u','d','c','T','W',0x02};
static	char	iconv_IBM_sbdTW_GL_SEG[]	=
	{'i','b','m','-','s','b','d','T','W',0x02};
static	char	iconv_IBM_sbdTW_GR_SEG[]	=
	{'I','B','M','-','s','b','d','T','W',0x02};
static	char	iconv_IBM_850_GL_SEG[]		=
	{'i','b','m','-','8','5','0',0x02};
static	char	iconv_IBM_850_GR_SEG[]		=
	{'I','B','M','-','8','5','0',0x02};
/* Added entrys for UTF S/C */
static	char	iconv_UTF_8_GR_SEG[]		=
   	{'U','T','F','-','8',0x02};
static	char	iconv_UTF_8_GL_SEG[]		=
   	{'U','T','F','-','7',0x02};

/* End addition for UTF S/C */

/*
 *	Macro definitions for entries of the escape sequence table.
 *
 *	NOTE: Element of the table (defined in fcs.h)
 *
 *	typedef	struct		_EscTbl	{
 *		uchar_t		*name;	* code set name		       *
 *		uchar_t		*str;	* escape sequence	       *
 *		int		len;	* escape sequence length       *
 *		uchar_t		*seg;	* extended segment name	       *
 *		int		seglen;	* extended segment name	length *
 *		int		gl;	* flag for GL/GR	       *
 *	} EscTbl;
 */

#define	ICONV_ASCII		{ NULL,	&iconv_ASCII[0],		\
				sizeof (iconv_ASCII),			\
				NULL, 0, True }
#define	ICONV_ASCII_GR		{ "ASCII-GR",				\
				&iconv_ASCII_GR[0],			\
				sizeof (iconv_ASCII_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_1_GL	{ "ISO8859-1-GL",			\
				&iconv_ISO8859_1_GL[0],			\
				sizeof (iconv_ISO8859_1_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_1_GR	{ "ISO8859-1-GR",			\
				&iconv_ISO8859_1_GR[0],			\
				sizeof (iconv_ISO8859_1_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_2_GL	{ "ISO8859-2-GL",			\
				&iconv_ISO8859_2_GL[0],			\
				sizeof (iconv_ISO8859_2_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_2_GR	{ "ISO8859-2-GR",			\
				&iconv_ISO8859_2_GR[0],			\
				sizeof (iconv_ISO8859_2_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_3_GL	{ "ISO8859-3-GL",			\
				&iconv_ISO8859_3_GL[0],			\
				sizeof (iconv_ISO8859_3_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_3_GR	{ "ISO8859-3-GR",			\
				&iconv_ISO8859_3_GR[0],			\
				sizeof (iconv_ISO8859_3_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_4_GL	{ "ISO8859-4-GL",			\
				&iconv_ISO8859_4_GL[0],			\
				sizeof (iconv_ISO8859_4_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_4_GR	{ "ISO8859-4-GR",			\
				&iconv_ISO8859_4_GR[0],			\
				sizeof (iconv_ISO8859_4_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_5_GL	{ "ISO8859-5-GL",			\
				&iconv_ISO8859_5_GL[0],			\
				sizeof (iconv_ISO8859_5_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_5_GR	{ "ISO8859-5-GR",			\
				&iconv_ISO8859_5_GR[0],			\
				sizeof (iconv_ISO8859_5_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_6_GL	{ "ISO8859-6-GL",			\
				&iconv_ISO8859_6_GL[0],			\
				sizeof (iconv_ISO8859_6_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_6_GR	{ "ISO8859-6-GR",			\
				&iconv_ISO8859_6_GR[0],			\
				sizeof (iconv_ISO8859_6_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_7_GL	{ "ISO8859-7-GL",			\
				&iconv_ISO8859_7_GL[0],			\
				sizeof (iconv_ISO8859_7_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_7_GR	{ "ISO8859-7-GR",			\
				&iconv_ISO8859_7_GR[0],			\
				sizeof (iconv_ISO8859_7_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_8_GL	{ "ISO8859-8-GL",			\
				&iconv_ISO8859_8_GL[0],			\
				sizeof (iconv_ISO8859_8_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_8_GR	{ "ISO8859-8-GR",			\
				&iconv_ISO8859_8_GR[0],			\
				sizeof (iconv_ISO8859_8_GR),		\
				NULL, 0, False }
#define	ICONV_ISO8859_9_GL	{ "ISO8859-9-GL",			\
				&iconv_ISO8859_9_GL[0],			\
				sizeof (iconv_ISO8859_9_GL),		\
				NULL, 0, True }
#define	ICONV_ISO8859_9_GR	{ "ISO8859-9-GR",			\
				&iconv_ISO8859_9_GR[0],			\
				sizeof (iconv_ISO8859_9_GR),		\
				NULL, 0, False }
#define	ICONV_JISX0201_1976	{ NULL,	&iconv_JISX0201_1976[0],	\
				sizeof (iconv_JISX0201_1976),		\
				NULL, 0, True }
#define	ICONV_JISX0201_1976_GL	{ "JISX0201.1976-GL",			\
				&iconv_JISX0201_1976_GL[0],		\
				sizeof (iconv_JISX0201_1976_GL),	\
				NULL, 0, True }
#define	ICONV_JISX0201_1976_GR	{ "JISX0201.1976-GR",			\
				&iconv_JISX0201_1976_GR[0],		\
				sizeof (iconv_JISX0201_1976_GR),	\
				NULL, 0, False }
#define	ICONV_JISX0208_1978_GL	{ "JISX0208.1978-GL",			\
				&iconv_JISX0208_1978_GL[0],		\
				sizeof (iconv_JISX0208_1978_GL),	\
				NULL, 0, True }
#define	ICONV_JISX0208_1978_GL_MAIL					\
				{ "JISX0208.1978-GL",			\
				&iconv_JISX0208_1978_GL_MAIL[0],	\
				sizeof (iconv_JISX0208_1978_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_JISX0208_1978_GR	{ "JISX0208.1978-GR",			\
				&iconv_JISX0208_1978_GR[0],		\
				sizeof (iconv_JISX0208_1978_GR),	\
				NULL, 0, False }
#define	ICONV_JISX0208_1983_GL	{ "JISX0208.1983-GL",			\
				&iconv_JISX0208_1983_GL[0],		\
				sizeof (iconv_JISX0208_1983_GL),	\
				NULL, 0, True }
#define	ICONV_JISX0208_1983_GL_MAIL					\
				{ "JISX0208.1983-GL",			\
				&iconv_JISX0208_1983_GL_MAIL[0],	\
				sizeof (iconv_JISX0208_1983_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_JISX0208_1983_GR	{ "JISX0208.1983-GR",			\
				&iconv_JISX0208_1983_GR[0],		\
				sizeof (iconv_JISX0208_1983_GR),	\
				NULL, 0, False }
#define	ICONV_KSC5601_1987_GR	{ "KSC5601.1987-GR",			\
				&iconv_KSC5601_1987_GR[0],		\
				sizeof (iconv_KSC5601_1987_GR),		\
				NULL, 0, False }
#define	ICONV_KSC5601_1987_GL	{ "KSC5601.1987-GL",			\
				&iconv_KSC5601_1987_GL[0],		\
				sizeof (iconv_KSC5601_1987_GL),		\
				NULL, 0, True }
#define	ICONV_CNS11643_CT_GR	{ "CNS11643.CT-GR",			\
				&iconv_CNS11643_GR[0],			\
				sizeof (iconv_CNS11643_GR),		\
				&iconv_CNS11643_GR_SEG[0],		\
				sizeof (iconv_CNS11643_GR_SEG),	True }
#define	ICONV_CNS11643_1986_1_GL { "CNS11643.1986-1-GL",		\
				&iconv_CNS11643_1986_1_GL[0],		\
				sizeof (iconv_CNS11643_1986_1_GL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1986_1_GL_MAIL { "CNS11643.1986-1-GL",		\
				&iconv_CNS11643_1986_1_GL_MAIL[0],	\
				sizeof (iconv_CNS11643_1986_1_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1986_1_GR { "CNS11643.1986-1-GR",		\
				&iconv_CNS11643_1986_1_GR[0],		\
				sizeof (iconv_CNS11643_1986_1_GR),	\
				&iconv_CNS11643_1986_1_GR_SEG[0],	\
				sizeof (iconv_CNS11643_1986_1_GR_SEG), False }
#define	ICONV_CNS11643_1986_2_GL { "CNS11643.1986-2-GL",		\
				&iconv_CNS11643_1986_2_GL[0],		\
				sizeof (iconv_CNS11643_1986_2_GL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1986_2_GL_MAIL { "CNS11643.1986-2-GL",		\
				&iconv_CNS11643_1986_2_GL_MAIL[0],	\
				sizeof (iconv_CNS11643_1986_2_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1986_2_GR { "CNS11643.1986-2-GR",		\
				&iconv_CNS11643_1986_2_GR[0],		\
				sizeof (iconv_CNS11643_1986_2_GR),	\
				&iconv_CNS11643_1986_2_GR_SEG[0],	\
				sizeof (iconv_CNS11643_1986_2_GR_SEG), False }
#define	ICONV_CNS11643_1992_3_GL { "CNS11643.1992-3-GL",		\
				&iconv_CNS11643_1992_3_GL[0],		\
				sizeof (iconv_CNS11643_1992_3_GL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1992_3_GL_MAIL { "CNS11643.1992-3-GL",		\
				&iconv_CNS11643_1992_3_GL_MAIL[0],	\
				sizeof (iconv_CNS11643_1992_3_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1992_3_GR { "CNS11643.1992-3-GR",		\
				&iconv_CNS11643_1992_3_GR[0],		\
				sizeof (iconv_CNS11643_1992_3_GR),	\
				&iconv_CNS11643_1992_3_GR_SEG[0],	\
				sizeof (iconv_CNS11643_1992_3_GR_SEG), False }
#define	ICONV_CNS11643_1992_4_GL { "CNS11643.1992-4-GL",		\
				&iconv_CNS11643_1992_4_GL[0],		\
				sizeof (iconv_CNS11643_1992_4_GL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1992_4_GL_MAIL { "CNS11643.1992-4-GL",		\
				&iconv_CNS11643_1992_4_GL_MAIL[0],	\
				sizeof (iconv_CNS11643_1992_4_GL_MAIL),	\
				NULL, 0, True }
#define	ICONV_CNS11643_1992_4_GR { "CNS11643.1992-4-GR",		\
				&iconv_CNS11643_1992_4_GR[0],		\
				sizeof (iconv_CNS11643_1992_4_GR),	\
				&iconv_CNS11643_1992_4_GR_SEG[0],	\
				sizeof (iconv_CNS11643_1992_4_GR_SEG), False }
#define	ICONV_IBM_udcJP_GL	{ "IBM-udcJP-GL",			\
				&iconv_IBM_udcJP_GL[0],			\
				sizeof (iconv_IBM_udcJP_GL),		\
				&iconv_IBM_udcJP_GL_SEG[0],		\
				sizeof (iconv_IBM_udcJP_GL_SEG), True }
#define	ICONV_IBM_udcJP_GR	{ "IBM-udcJP-GR",			\
				&iconv_IBM_udcJP_GR[0],			\
				sizeof (iconv_IBM_udcJP_GR),		\
				&iconv_IBM_udcJP_GR_SEG[0],		\
				sizeof (iconv_IBM_udcJP_GR_SEG), False }
#define	ICONV_IBM_udcTW_GL	{ "IBM-udcTW-GL",			\
				&iconv_IBM_udcTW_GL[0],			\
				sizeof (iconv_IBM_udcTW_GL),		\
				&iconv_IBM_udcTW_GL_SEG[0],		\
				sizeof (iconv_IBM_udcTW_GL_SEG), True }
#define	ICONV_IBM_udcTW_GR	{ "IBM-udcTW-GR",			\
				&iconv_IBM_udcTW_GR[0],			\
				sizeof (iconv_IBM_udcTW_GR),		\
				&iconv_IBM_udcTW_GR_SEG[0],		\
				sizeof (iconv_IBM_udcTW_GR_SEG), False }
#define	ICONV_IBM_udcTW_CT_GR	{ "IBM-udcTW.CT-GR",			\
				&iconv_IBM_udcTW_GR[0],			\
				sizeof (iconv_IBM_udcTW_GR),		\
				&iconv_IBM_udcTW_GR_SEG[0],		\
				sizeof (iconv_IBM_udcTW_GR_SEG), False }
#define	ICONV_IBM_sbdTW_GL	{ "IBM-sbdTW-GL",			\
				&iconv_IBM_sbdTW_GL[0],			\
				sizeof (iconv_IBM_sbdTW_GL),		\
				&iconv_IBM_sbdTW_GL_SEG[0],		\
				sizeof (iconv_IBM_sbdTW_GL_SEG), True }
#define	ICONV_IBM_sbdTW_GR	{ "IBM-sbdTW-GR",			\
				&iconv_IBM_sbdTW_GR[0],			\
				sizeof (iconv_IBM_sbdTW_GR),		\
				&iconv_IBM_sbdTW_GR_SEG[0],		\
				sizeof (iconv_IBM_sbdTW_GR_SEG), False }
#define	ICONV_IBM_sbdTW_CT_GR	{ "IBM-sbdTW.CT-GR",			\
				&iconv_IBM_sbdTW_GR[0],			\
				sizeof (iconv_IBM_sbdTW_GR),		\
				&iconv_IBM_sbdTW_GR_SEG[0],		\
				sizeof (iconv_IBM_sbdTW_GR_SEG), False }
#define	ICONV_IBM_850_GL	{ "IBM-850-GL",				\
				&iconv_IBM_850_GL[0],			\
				sizeof (iconv_IBM_850_GL),		\
				&iconv_IBM_850_GL_SEG[0],		\
				sizeof (iconv_IBM_850_GL_SEG), True }
#define	ICONV_IBM_850_GR	{ "IBM-850-GR",				\
				&iconv_IBM_850_GR[0],			\
				sizeof (iconv_IBM_850_GR),		\
				&iconv_IBM_850_GR_SEG[0],		\
				sizeof (iconv_IBM_850_GR_SEG), False }
/* Added items for EUC S/C */
#define ICONV_GB2312_1980_0_GL  { "GB2312.1980-0-GL", \
                                        &iconv_GB2312_1980_0_GL[0], \
                                        sizeof (iconv_GB2312_1980_0_GL), \
                                        NULL, 0, True }

#define ICONV_GB2312_1980_0_GR  { "GB2312.1980-0-GR", \
                                        &iconv_GB2312_1980_0_GR[0], \
                                        sizeof (iconv_GB2312_1980_0_GR), \
                                        NULL, 0, \
					False }

#define ICONV_GB2312_1980_GR  { "GB2312.1980-0-GR", \
                                        &iconv_GB2312_1980_GR[0], \
                                        sizeof (iconv_GB2312_1980_GR), \
                                        NULL, 0, False }

#define ICONV_UTF_8  { "UTF-8", \
                                        &iconv_UTF_8_GR[0], \
                                        sizeof (iconv_UTF_8_GR), \
                                        &iconv_UTF_8_GR_SEG[0], \
                                        sizeof (iconv_UTF_8_GR_SEG), True }

#define ICONV_UTF_8_GL  { "UTF-8-GL", \
                                        &iconv_UTF_8_GL[0], \
                                        sizeof (iconv_UTF_8_GL), \
                                        &iconv_UTF_8_GL_SEG[0], \
                                        sizeof (iconv_UTF_8_GL_SEG), True }

/* End addition for EUC S/C */


/*
 *	Array of EscTbl, that lists possible ISO2022 escape sequences.
 */

static	EscTbl	etbl_ct_IBM_850[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_ct_IBM_850 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 4,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 2, 2, 2,	2, 4, 4, 4, 4, 2, 2, 4,

	4, 4, 4, 4, 4, 4, 2, 2,	4, 4, 4, 4, 4, 4, 4, 2,
	2, 2, 2, 2, 2, 4, 2, 2,	2, 4, 4, 4, 4, 2, 2, 4,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 4, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 4, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_IBM_856[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_ct_IBM_856 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, X, 2, X, 2, X,
	X, X, X, X, X, X, X, X,	X, 2, 2, 2, 2, X, 2, 2,
	4, 4, 4, 4, 4, X, X, X,	2, 4, 4, 4, 4, 2, 2, 4,

	4, 4, 4, 4, 4, 4, X, X,	4, 4, 4, 4, 4, 4, 4, 2,
	X, X, X, X, X, X, X, X,	X, 4, 4, 4, 4, 2, X, 4,
	X, X, X, X, X, X, 2, X,	X, X, X, X, X, X, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 4, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_IBM_1046[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL
};

static	uchar_t	csidx_ct_IBM_1046 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 4, 4, 2, 2, 2, 2, 2,	X, 4, 4, 4, 4, 4, 4, 4,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 0, X,	X, X, X, 2, 2, 2, 2, X,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GR,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_ct_IBM_932 (uchar_t p[], size_t l) {

	uchar_t		c = p[0];
	ushort_t	dbcs;
	int		low, mid, high;

	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		else	return INVALIDCSID;
	}
	if (c <= 0x7f)	return 0;
	if (c == 0x80)	return INVALIDCSID;
	if (c == 0xa0)	return INVALIDCSID;
	if ((0xa1 <= c) && (c <= 0xdf))
			return 2;
	if (c <= 0xfc) {
		if (l <	2)
			return NEEDMORE;
		if ((p[1] < 0x40) || (0xfc < p[1]))
			return INVALIDCSID;
		dbcs = ((ushort_t)c << 8 & 0xff00) + ((ushort_t)p[1] & 0xff);
		low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if	(dbcs >	CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
				break;
			}
		}
		c = (uchar_t)(dbcs >> 8 & 0xff);
		if (c <= 0xef)
			return 8;
		else	return 10;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_ct_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GR,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_ct_IBM_eucJP (uchar_t p[], size_t l) {

	uchar_t	  c = p[0];

	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		else	return INVALIDCSID;
	}
	if (c <= 0x7f)	return 0;
	if (c <= 0x8d)	return INVALIDCSID;
	if (c == 0x8e) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 2;
		else	return INVALIDCSID;
	}
	if (c == 0x8f) {
		if (l <	3)
			return NEEDMORE;
		if (((0xa1 <= p[1]) && (p[1] <= 0xfe)) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)))
			return 10;
		else	return INVALIDCSID;
	}
	if (c <= 0x9a)	return INVALIDCSID;
	if (c == 0x9b)	return CONTROLCSID;
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xf4) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 8;
		else	return INVALIDCSID;
	}
	if (c <= 0xfe) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 10;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_ct_IBM_eucKR[] = {
	ICONV_ASCII,
	ICONV_KSC5601_1987_GR,
	ICONV_KSC5601_1987_GL,
};

static	uchar_t	csidx_ct_IBM_eucKR (uchar_t p[], size_t l) {

	uchar_t	  c = p[0];

	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		else	return INVALIDCSID;
	}
	if (c <= 0x7f)	return 0;
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xac) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	if (c <= 0xaf)	return INVALIDCSID;
	if (c <= 0xc8) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	if (c == 0xc9)	return INVALIDCSID;
	if (c <= 0xfd) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_ct_IBM_eucTW[] = {
	ICONV_ASCII,
	ICONV_CNS11643_CT_GR,
	ICONV_IBM_udcTW_GL,
	ICONV_IBM_udcTW_CT_GR,
	ICONV_IBM_sbdTW_GL,
	ICONV_IBM_sbdTW_CT_GR,
	ICONV_CNS11643_1986_1_GL_MAIL,
	ICONV_CNS11643_1986_2_GL_MAIL,
	ICONV_CNS11643_1992_3_GL_MAIL,
	ICONV_CNS11643_1992_4_GL_MAIL,
	ICONV_CNS11643_1986_1_GL,
	ICONV_CNS11643_1986_1_GR,
	ICONV_CNS11643_1986_2_GL,
	ICONV_CNS11643_1986_2_GR,
	ICONV_CNS11643_1992_3_GL,
	ICONV_CNS11643_1992_3_GR,
	ICONV_CNS11643_1992_4_GL,
	ICONV_CNS11643_1992_4_GR,
	ICONV_GB2312_1980_0_GR
};

static	uchar_t	csidx_ct_IBM_eucTW (uchar_t p[], size_t l) {

	uchar_t	  c = p[0];

	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		else	return INVALIDCSID;
	}
	if (c <= 0x7f)	return 0;
	if (c <= 0x8d)	return INVALIDCSID;
	if (c == 0x8e) {
		if (l <	4)
			return NEEDMORE;
		if (((p[1] == 0xa2) || (p[1] == 0xa3) || (p[1] == 0xa4)) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 1;
		if ((p[1] == 0xac) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 3;
		if ((p[1] == 0xad) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 5;
		return INVALIDCSID;
	}
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xfe) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_ct_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_ASCII_GR,
};

static EscTbl   etbl_ct_IBM_eucCN[] = {
        ICONV_ASCII,
        ICONV_CNS11643_CT_GR,
        ICONV_GB2312_1980_0_GR,
        ICONV_GB2312_1980_0_GL,
};


static unsigned char    csidx_ct_IBM_eucCN(unsigned char *p, size_t l)
{
        unsigned char   c;

        c = *p;
        if (c < 0x20) {
                if (c == '\t' || c == '\n')
                        return CONTROLCSID;
                return INVALIDCSID;
        }
        if (c <= 0x7f)
                return 0;

        if (c <= 0xa0)
                return INVALIDCSID;

        if ((c <= 0xfe && p[1] <= 0xfe)) {
                if (l < 2)
                    return NEEDMORE;
                return 2;
            }
       return INVALIDCSID;
};


static	uchar_t	csidx_ct_ISO8859_1 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GR,
	ICONV_ISO8859_2_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_2 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GR,
	ICONV_ISO8859_3_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_3 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GR,
	ICONV_ISO8859_4_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_4 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GR,
	ICONV_ISO8859_5_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_5 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_6 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GR,
	ICONV_ISO8859_7_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_7 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_8 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_ct_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GR,
	ICONV_ISO8859_9_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_ct_ISO8859_9 (uchar_t *p,	size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X,	X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_IBM_850[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GL,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_fold7_IBM_850 (uchar_t *p, size_t	l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 3,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 2, 2, 2,	2, 3, 3, 3, 3, 2, 2, 3,

	3, 3, 3, 3, 3, 3, 2, 2,	3, 3, 3, 3, 3, 3, 3, 2,
	2, 2, 2, 2, 2, 3, 2, 2,	2, 3, 3, 3, 3, 2, 2, 3,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 3, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 3, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_IBM_856[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GL,
	ICONV_IBM_850_GL,
};

static uchar_t	csidx_fold7_IBM_856 (uchar_t *p, size_t	l) {
static uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, X, 2, X, 2, X,
	X, X, X, X, X, X, X, X,	X, 2, 2, 2, 2, X, 2, 2,
	3, 3, 3, 3, 3, X, X, X,	2, 3, 3, 3, 3, 2, 2, 3,

	3, 3, 3, 3, 3, 3, X, X,	3, 3, 3, 3, 3, 3, 3, 2,
	X, X, X, X, X, X, X, X,	X, 3, 3, 3, 3, 2, X, 3,
	X, X, X, X, X, X, 2, X,	X, X, X, X, X, X, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 3, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_IBM_1046[] =	{
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GL,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_fold7_IBM_1046 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 3, 3, 2, 2, 2, 2, 2,	X, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 0, X,	X, X, X, 2, 2, 2, 2, X,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_fold7_IBM_932 (uchar_t p[], size_t l) {

	uchar_t		c = p[0];
	ushort_t	dbcs;
	int		low, mid, high;

	if (c <= 0x1f)	return CONTROLCSID;
	if (c <= 0x7f)	return 0;
	if (c == 0x80)	return INVALIDCSID;
	if (c == 0xa0)	return INVALIDCSID;
	if ((0xa1 <= c) && (c <= 0xdf))
			return 2;
	if (c <= 0xfc) {
		if (l <	2)
			return NEEDMORE;
		if ((p[1] < 0x40) || (0xfc < p[1]))
			return INVALIDCSID;
		dbcs = ((ushort_t)c << 8 & 0xff00) + ((ushort_t)p[1] & 0xff);
		low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if	(dbcs >	CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
				break;
			}
		}
		c = (uchar_t)(dbcs >> 8 & 0xff);
		if (c <= 0xef)
			return 4;
		else	return 7;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold7_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_fold7_IBM_eucJP (uchar_t p[], size_t l) {

	uchar_t	  c = p[0];

	if (c <= 0x1f)	return CONTROLCSID;
	if (c <= 0x7f)	return 0;
	if (c <= 0x8d)	return INVALIDCSID;
	if (c == 0x8e) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 2;
		else	return INVALIDCSID;
	}
	if (c == 0x8f) {
		if (l <	3)
			return NEEDMORE;
		if (((0xa1 <= p[1]) && (p[1] <= 0xfe)) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)))
			return 7;
		else	return INVALIDCSID;
	}
	if (c <= 0x9a)	return INVALIDCSID;
	if (c == 0x9b)	return CONTROLCSID;
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xf4) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 4;
		else	return INVALIDCSID;
	}
	if (c <= 0xfe) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 7;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold7_IBM_eucKR[] = {
	ICONV_ASCII,
	ICONV_KSC5601_1987_GL,
};

static	uchar_t	csidx_fold7_IBM_eucKR (uchar_t p[], size_t l) {
/*
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, X, X, X,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1,	1, X, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, X, X,
#undef	C
#undef	X
	};
	return csidx[*p];
*/
	uchar_t		c = p[0];

	if (c <= 0x1f)  return CONTROLCSID;
	if (c <= 0x7f)  return 0;
	if (c <= 0xa0)  return INVALIDCSID;
	if (c <= 0xac) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	if (c <= 0xaf)  return INVALIDCSID;
	if (c <= 0xc8) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	if (c == 0xc9)  return INVALIDCSID;
	if (c <= 0xfd) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold7_IBM_eucTW[] = {
	ICONV_ASCII,
	ICONV_CNS11643_1986_1_GL,
	ICONV_CNS11643_1986_2_GL,
	ICONV_CNS11643_1992_3_GL,
	ICONV_CNS11643_1992_4_GL,
	ICONV_CNS11643_1986_1_GL_MAIL,
	ICONV_CNS11643_1986_2_GL_MAIL,
	ICONV_CNS11643_1992_3_GL_MAIL,
	ICONV_CNS11643_1992_4_GL_MAIL,
	ICONV_IBM_udcTW_GL,
	ICONV_IBM_sbdTW_GL,
        ICONV_GB2312_1980_0_GL
};

static	uchar_t	csidx_fold7_IBM_eucTW (uchar_t p[], size_t l) {

	uchar_t		c = p[0];

	if (c <= 0x1f)	return CONTROLCSID;
	if (c <= 0x7f)	return 0;			/* ASCII */
	if (c <= 0x8d)	return INVALIDCSID;
	if (c == 0x8e) {
		if (l <	4)
			return NEEDMORE;
		if ((p[1] == 0xa2) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 2;			/* CNS11643_1986_2_GL */
		if ((p[1] == 0xa3) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 3;			/* CNS11643_1992_3_GL */
		if ((p[1] == 0xa4) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 4;			/* CNS11643_1992_4_GL */
		if ((p[1] == 0xac) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 9;			/* IBM_udcTW_GL */
		if ((p[1] == 0xad) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 10;			/* IBM_sbdTW_GL */
		return INVALIDCSID;
	}
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xfe) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;			/* CNS11643_1986_1_GL */
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static EscTbl   etbl_fold7_IBM_eucCN[] = {
        ICONV_ASCII,
        ICONV_GB2312_1980_0_GL,
        ICONV_CNS11643_1986_1_GL,
        ICONV_CNS11643_1986_2_GL
};

static unsigned char    csidx_fold7_IBM_eucCN(unsigned char *p, size_t l)
{
        unsigned char   c;

        c = *p;
        if (c < 0x20)
                return CONTROLCSID;

        if (c <= 0x7f)
                return 0;

        if (c <= 0xa0)
                return INVALIDCSID;

        if (c <= 0xfe) {
                if (l < 2)
                    return NEEDMORE;
                return 1;
        }
	return INVALIDCSID;
};

static	EscTbl	etbl_fold7_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GL,
};

static	uchar_t	csidx_fold7_ISO8859_1 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GL,
};

static	uchar_t	csidx_fold7_ISO8859_2 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GL,
};

static	uchar_t	csidx_fold7_ISO8859_3 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GL,
};

static	uchar_t	csidx_fold7_ISO8859_4 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}
static	EscTbl	etbl_fold7_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GL,
};

static	uchar_t	csidx_fold7_ISO8859_5 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GL,
};

static	uchar_t	csidx_fold7_ISO8859_6 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GL,
};

static	uchar_t	csidx_fold7_ISO8859_7 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GL,
};

static	uchar_t	csidx_fold7_ISO8859_8 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold7_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GL,
};

static	uchar_t	csidx_fold7_ISO8859_9 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X,	X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}
static	EscTbl	etbl_fold8_IBM_850[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_ASCII_GR,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_fold8_IBM_850 (uchar_t *p, size_t	l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 5,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	5, 5, 5, 5, 5, 2, 2, 2,	2, 5, 5, 5, 5, 2, 2, 5,

	5, 5, 5, 5, 5, 5, 2, 2,	5, 5, 5, 5, 5, 5, 5, 2,
	2, 2, 2, 2, 2, 5, 2, 2,	2, 5, 5, 5, 5, 2, 2, 5,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 5, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 5, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_IBM_856[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_ASCII_GR,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_fold8_IBM_856 (uchar_t *p, size_t	l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, X, 2, X, 2, X,
	X, X, X, X, X, X, X, X,	X, 2, 2, 2, 2, X, 2, 2,
	5, 5, 5, 5, 5, X, X, X,	2, 5, 5, 5, 5, 2, 2, 5,

	5, 5, 5, 5, 5, 5, X, X,	5, 5, 5, 5, 5, 5, 5, 2,
	X, X, X, X, X, X, X, X,	X, 5, 5, 5, 5, 2, X, 5,
	X, X, X, X, X, X, 2, X,	X, X, X, X, X, X, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 5, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_IBM_1046[] =	{
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_ASCII_GR,
	ICONV_IBM_850_GR,
	ICONV_IBM_850_GL,
};

static	uchar_t	csidx_fold8_IBM_1046 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,	
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 5, 5, 2, 2, 2, 2, 2,	X, 5, 5, 5, 5, 5, 5, 5,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 0, X,	X, X, X, 2, 2, 2, 2, X,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GR,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_fold8_IBM_932 (uchar_t p[], size_t l) {

	uchar_t		c = p[0];
	ushort_t	dbcs;
	int		low, mid, high;

	if (c <= 0x1f)	return CONTROLCSID;
	if (c <= 0x7f)	return 0;
	if (c == 0x80)	return INVALIDCSID;
	if (c == 0xa0)	return INVALIDCSID;
	if (0xa1 <= c && c <= 0xdf)
			return 2;
	if (c <= 0xfc) {
		if (l <	2)
			return NEEDMORE;
		if ((p[1] < 0x40) || (0xfc < p[1]))
			return INVALIDCSID;
		dbcs = ((ushort_t)c << 8 & 0xff00) + ((ushort_t)p[1] & 0xff);
		low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if	(dbcs >	CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
				break;
			}
		}
		c = (uchar_t)(dbcs >> 8 & 0xff);
		if (c <= 0xef)
			return 8;
		else	return 10;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold8_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
	ICONV_IBM_udcJP_GR,
	ICONV_IBM_udcJP_GL,
};

static	uchar_t	csidx_fold8_IBM_eucJP (uchar_t p[], size_t l) {

	uchar_t	  c = p[0];

	if (c <= 0x1f)	return CONTROLCSID;
	if (c <= 0x7f)	return 0;
	if (c <= 0x8d)	return INVALIDCSID;
	if (c == 0x8e) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 2;
		else	return INVALIDCSID;
	}
	if (c == 0x8f) {
		if (l <	3)
			return NEEDMORE;
		if (((0xa1 <= p[1]) && (p[1] <= 0xfe)) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)))
			return 10;
		else	return INVALIDCSID;
	}
	if (c <= 0x9a)	return INVALIDCSID;
	if (c == 0x9b)	return CONTROLCSID;
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xf4) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 8;
		else	return INVALIDCSID;
	}
	if (c <= 0xfe) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 10;
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold8_IBM_eucKR[] = {
	ICONV_ASCII,
	ICONV_KSC5601_1987_GR,
	ICONV_KSC5601_1987_GL,
};

static	uchar_t	csidx_fold8_IBM_eucKR (uchar_t p[], size_t l) {
/*
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	X, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, X, X, X,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1,	1, X, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, X, X,
#undef	C
#undef	X
	};
	return csidx[*p];
*/
	uchar_t		c = p[0];

	if (c <= 0x1f)  return CONTROLCSID;
	if (c <= 0x7f)  return 0;		/* ASCII */
	if (c <= 0xa0)  return INVALIDCSID;
	if (c <= 0xac) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;		/* KSC5601_1987_GR */
		else	return INVALIDCSID;
	}
	if (c <= 0xaf)  return INVALIDCSID;
	if (c <= 0xc8) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;		/* KSC5601_1987_GR */
		else	return INVALIDCSID;
	}
	if (c == 0xc9)  return INVALIDCSID;
	if (c <= 0xfd) {
		if (l < 2)
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 1;		/* KSC5601_1987_GR */
		else	return INVALIDCSID;
	}
	return INVALIDCSID;
}

static	EscTbl	etbl_fold8_IBM_eucTW[] = {
	ICONV_ASCII,
	ICONV_IBM_udcTW_GL,
	ICONV_IBM_udcTW_GR,
	ICONV_IBM_sbdTW_GL,
	ICONV_IBM_sbdTW_GR,
	ICONV_CNS11643_1986_1_GL_MAIL,
	ICONV_CNS11643_1986_2_GL_MAIL,
	ICONV_CNS11643_1992_3_GL_MAIL,
	ICONV_CNS11643_1992_4_GL_MAIL,
	ICONV_CNS11643_1986_1_GL,
	ICONV_CNS11643_1986_1_GR,
	ICONV_CNS11643_1986_2_GL,
	ICONV_CNS11643_1986_2_GR,
	ICONV_CNS11643_1992_3_GL,
	ICONV_CNS11643_1992_3_GR,
	ICONV_CNS11643_1992_4_GL,
	ICONV_CNS11643_1992_4_GR,
        ICONV_GB2312_1980_0_GR
};

static	uchar_t	csidx_fold8_IBM_eucTW (uchar_t p[], size_t l) {

	uchar_t		c = p[0];

	if (c <= 0x1f) return	CONTROLCSID;
	if (c <= 0x7f) return 0;
	if (c <= 0x8d) return INVALIDCSID;
	if (c == 0x8e) {
		if (l <	4)
			return NEEDMORE;
		if ((p[1] == 0xa2) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 12;
		if ((p[1] == 0xa3) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 14;
		if ((p[1] == 0xa4) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 16;
		if ((p[1] == 0xac) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 2;
		if ((p[1] == 0xad) &&
		    ((0xa1 <= p[2]) && (p[2] <= 0xfe)) &&
		    ((0xa1 <= p[3]) && (p[3] <= 0xfe)))
			return 4;
		return INVALIDCSID;
	}
	if (c <= 0xa0)	return INVALIDCSID;
	if (c <= 0xfe) {
		if (l < 2) 
			return NEEDMORE;
		if ((0xa1 <= p[1]) && (p[1] <= 0xfe))
			return 10;
		else	return INVALIDCSID;
	}
}

static EscTbl   etbl_fold8_IBM_eucCN[] = {
        ICONV_ASCII,
        ICONV_GB2312_1980_0_GR,
        ICONV_GB2312_1980_0_GL,
        ICONV_CNS11643_1986_1_GR,
        ICONV_CNS11643_1986_2_GR
};

static unsigned char    csidx_fold8_IBM_eucCN(unsigned char *p, size_t l)
{
        unsigned char   c;

        c = *p;
        if (c < 0x20)
                 return CONTROLCSID;

        if (c <= 0x7f)
                return 0;

        if (c <= 0xa0)
                return INVALIDCSID;

        if ((c <= 0xfe && p[1] <= 0xfe)){
                if (l < 2)
                    return NEEDMORE;
                return 1;
            }
	return INVALIDCSID;

};

static	EscTbl	etbl_fold8_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_1 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GR,
	ICONV_ISO8859_2_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_2 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}


static	EscTbl	etbl_fold8_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GR,
	ICONV_ISO8859_3_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_3 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GR,
	ICONV_ISO8859_4_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_4 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GR,
	ICONV_ISO8859_5_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_5 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_6 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GR,
	ICONV_ISO8859_7_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_7 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_8 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_fold8_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GR,
	ICONV_ISO8859_9_GL,
	ICONV_ASCII_GR,
};

static	uchar_t	csidx_fold8_ISO8859_9 (uchar_t *p, size_t l) {
static	uchar_t	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C,	C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
	};
	return csidx[*p];
}

static	EscTbl	etbl_UTF_8[] = {
	ICONV_UTF_8
};

static	EscTbl	etbl_UTF_8_GL[] = {
	ICONV_UTF_8_GL
};

/*
 *	Control character map
 *	0	- Not control character
 *	1	- Valid control character
 *	2	- Invalid control character
 */

static	uchar_t	ct_isctl[256] =	{
	2, 2, 2, 2, 2, 2, 2, 2,	2, 1, 1, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 1, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
};

static	uchar_t	fold7_isctl[256] = {
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
};

static	uchar_t	fold8_isctl[256] = {
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
};


/*
 *	Table of vectors to ISO2022 escape sequence table.
 *
 *	NOTE: Element of the table (defined in fcs.h)
 *
 *      typedef struct	_EscTblTbl {
 *		uchar_t	*name;		* code set name                       *
 *		int	netbl;		* # of entries of escape sequence tbl *
 *		int	defgl;		* index of default code set for GL    *
 *		int	defgr;		* index of default code set for GR    *
 *		EscTbl	*etbl;		* pointer to escape sequence table    *
 *		uchar_t	(*csidx)();	* pointer to CS index function        *
 *		uchar_t	*isctl;		* pointer to control character map    *
 *	} EscTblTbl;
 */

#define	SIZEOF(name)	(sizeof	(name) / sizeof	(name[0]))

EscTblTbl	_iconv_ct_ett[]	= {
	{
		"IBM-850",
		SIZEOF(etbl_ct_IBM_850),
		0, 2,
		etbl_ct_IBM_850,
		csidx_ct_IBM_850,
		ct_isctl,
	},
	{
		"IBM-856",
		SIZEOF(etbl_ct_IBM_856),
		0, 2,
		etbl_ct_IBM_856,
		csidx_ct_IBM_856,
		ct_isctl,
	},
	{
		"IBM-1046",
		SIZEOF(etbl_ct_IBM_1046),
		0, 2,
		etbl_ct_IBM_1046,
		csidx_ct_IBM_1046,
		ct_isctl,
	},
	{
		"IBM-932",
		SIZEOF(etbl_ct_IBM_932),
		0, 0,
		etbl_ct_IBM_932,
		csidx_ct_IBM_932,
		ct_isctl,
	},
	{
		"IBM-eucJP",
		SIZEOF(etbl_ct_IBM_eucJP),
		0, 0,
		etbl_ct_IBM_eucJP,
		csidx_ct_IBM_eucJP,
		ct_isctl,
	},
	{
		"IBM-eucKR",
		SIZEOF(etbl_ct_IBM_eucKR),
		0, 0,
		etbl_ct_IBM_eucKR,
		csidx_ct_IBM_eucKR,
		ct_isctl,
	},
	{
		"IBM-eucTW",
		SIZEOF(etbl_ct_IBM_eucTW),
		0, 0,
		etbl_ct_IBM_eucTW,
		csidx_ct_IBM_eucTW,
		ct_isctl,
	},
	{
                "IBM-eucCN",
                SIZEOF(etbl_ct_IBM_eucCN),
                0, 0,
                etbl_ct_IBM_eucCN,
                csidx_ct_IBM_eucCN,
                ct_isctl,
        },
	{
		"ISO8859-1",
		SIZEOF(etbl_ct_ISO8859_1),
		0, 2,
		etbl_ct_ISO8859_1,
		csidx_ct_ISO8859_1,
		ct_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_ct_ISO8859_2),
		0, 2,
		etbl_ct_ISO8859_2,
		csidx_ct_ISO8859_2,
		ct_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_ct_ISO8859_3),
		0, 2,
		etbl_ct_ISO8859_3,
		csidx_ct_ISO8859_3,
		ct_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_ct_ISO8859_4),
		0, 2,
		etbl_ct_ISO8859_4,
		csidx_ct_ISO8859_4,
		ct_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_ct_ISO8859_5),
		0, 2,
		etbl_ct_ISO8859_5,
		csidx_ct_ISO8859_5,
		ct_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_ct_ISO8859_6),
		0, 2,
		etbl_ct_ISO8859_6,
		csidx_ct_ISO8859_6,
		ct_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_ct_ISO8859_7),
		0, 2,
		etbl_ct_ISO8859_7,
		csidx_ct_ISO8859_7,
		ct_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_ct_ISO8859_8),
		0, 2,
		etbl_ct_ISO8859_8,
		csidx_ct_ISO8859_8,
		ct_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_ct_ISO8859_9),
		0, 2,
		etbl_ct_ISO8859_9,
		csidx_ct_ISO8859_9,
		ct_isctl,
	},
					/* 
					 *The following is a dummy that
					 * used by the UTF/ct conversion.
					 * for UTF extended segments.
					 */ 
	{				
		"UTF-8",
		SIZEOF(etbl_UTF_8),
		0, 0,
		etbl_UTF_8,
		csidx_ct_ISO8859_1,
		ct_isctl,
	},
	{ NULL,	0, 0, 0, NULL, NULL, NULL }
};

EscTblTbl	_iconv_fold7_ett[] = {
	{
		"IBM-850",
		SIZEOF(etbl_fold7_IBM_850),
		0, 0,
		etbl_fold7_IBM_850,
		csidx_fold7_IBM_850,
		fold7_isctl,
	},
	{
		"IBM-856",
		SIZEOF(etbl_fold7_IBM_856),
		0, 0,
		etbl_fold7_IBM_856,
		csidx_fold7_IBM_856,
		fold7_isctl,
	},
	{
		"IBM-1046",
		SIZEOF(etbl_fold7_IBM_1046),
		0, 0,
		etbl_fold7_IBM_1046,
		csidx_fold7_IBM_1046,
		fold7_isctl,
	},
	{
		"IBM-932",
		SIZEOF(etbl_fold7_IBM_932),
		0, 0,
		etbl_fold7_IBM_932,
		csidx_fold7_IBM_932,
		fold7_isctl,
	},
	{
		"IBM-eucJP",
		SIZEOF(etbl_fold7_IBM_eucJP),
		0, 0,
		etbl_fold7_IBM_eucJP,
		csidx_fold7_IBM_eucJP,
		fold7_isctl,
	},
	{
		"IBM-eucKR",
		SIZEOF(etbl_fold7_IBM_eucKR),
		0, 0,
		etbl_fold7_IBM_eucKR,
		csidx_fold7_IBM_eucKR,
		fold7_isctl,
	},
	{
		"IBM-eucTW",
		SIZEOF(etbl_fold7_IBM_eucTW),
		0, 0,
		etbl_fold7_IBM_eucTW,
		csidx_fold7_IBM_eucTW,
		fold7_isctl,
	},
        {
                "IBM-eucCN",
                SIZEOF(etbl_fold7_IBM_eucCN),
                0, 0,
                etbl_fold7_IBM_eucCN,
                csidx_fold7_IBM_eucCN,
                fold7_isctl,
        },
	{
		"ISO8859-1",
		SIZEOF(etbl_fold7_ISO8859_1),
		0, 0,
		etbl_fold7_ISO8859_1,
		csidx_fold7_ISO8859_1,
		fold7_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_fold7_ISO8859_2),
		0, 0,
		etbl_fold7_ISO8859_2,
		csidx_fold7_ISO8859_2,
		fold7_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_fold7_ISO8859_3),
		0, 0,
		etbl_fold7_ISO8859_3,
		csidx_fold7_ISO8859_3,
		fold7_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_fold7_ISO8859_4),
		0, 0,
		etbl_fold7_ISO8859_4,
		csidx_fold7_ISO8859_4,
		fold7_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_fold7_ISO8859_5),
		0, 0,
		etbl_fold7_ISO8859_5,
		csidx_fold7_ISO8859_5,
		fold7_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_fold7_ISO8859_6),
		0, 0,
		etbl_fold7_ISO8859_6,
		csidx_fold7_ISO8859_6,
		fold7_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_fold7_ISO8859_7),
		0, 0,
		etbl_fold7_ISO8859_7,
		csidx_fold7_ISO8859_7,
		fold7_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_fold7_ISO8859_8),
		0, 0,
		etbl_fold7_ISO8859_8,
		csidx_fold7_ISO8859_8,
		fold7_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_fold7_ISO8859_9),
		0, 0,
		etbl_fold7_ISO8859_9,
		csidx_fold7_ISO8859_9,
		fold7_isctl,
	},
					/* 
					 *The following is a dummy that
					 * used by the UTF/ct conversion.
					 * for UTF extended segments.
					 */ 
	{				
		"UTF-8",
		SIZEOF(etbl_UTF_8_GL),
		0, 0,
		etbl_UTF_8_GL,
		csidx_fold7_ISO8859_1,
		fold7_isctl,
	},
	{ NULL,	0, 0, 0, NULL, NULL, NULL }
};

EscTblTbl	_iconv_fold8_ett[] = {
	{
		"IBM-850",
		SIZEOF(etbl_fold8_IBM_850),
		0, 2,
		etbl_fold8_IBM_850,
		csidx_fold8_IBM_850,
		fold8_isctl,
	},
	{
		"IBM-856",
		SIZEOF(etbl_fold8_IBM_856),
		0, 2,
		etbl_fold8_IBM_856,
		csidx_fold8_IBM_856,
		fold8_isctl,
	},
	{
		"IBM-1046",
		SIZEOF(etbl_fold8_IBM_1046),
		0, 2,
		etbl_fold8_IBM_1046,
		csidx_fold8_IBM_1046,
		fold8_isctl,
	},
	{
		"IBM-932",
		SIZEOF(etbl_fold8_IBM_932),
		0, 0,
		etbl_fold8_IBM_932,
		csidx_fold8_IBM_932,
		fold8_isctl,
	},
	{
		"IBM-eucJP",
		SIZEOF(etbl_fold8_IBM_eucJP),
		0, 0,
		etbl_fold8_IBM_eucJP,
		csidx_fold8_IBM_eucJP,
		fold8_isctl,
	},
	{
		"IBM-eucKR",
		SIZEOF(etbl_fold8_IBM_eucKR),
		0, 0,
		etbl_fold8_IBM_eucKR,
		csidx_fold8_IBM_eucKR,
		fold8_isctl,
	},
	{
		"IBM-eucTW",
		SIZEOF(etbl_fold8_IBM_eucTW),
		0, 0,
		etbl_fold8_IBM_eucTW,
		csidx_fold8_IBM_eucTW,
		fold8_isctl,
	},
        {
                "IBM-eucCN",
                SIZEOF(etbl_fold8_IBM_eucCN),
                0, 0,
                etbl_fold8_IBM_eucCN,
                csidx_fold8_IBM_eucCN,
                fold8_isctl,
        },
	{
		"ISO8859-1",
		SIZEOF(etbl_fold8_ISO8859_1),
		0, 2,
		etbl_fold8_ISO8859_1,
		csidx_fold8_ISO8859_1,
		fold8_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_fold8_ISO8859_2),
		0, 2,
		etbl_fold8_ISO8859_2,
		csidx_fold8_ISO8859_2,
		fold8_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_fold8_ISO8859_3),
		0, 2,
		etbl_fold8_ISO8859_3,
		csidx_fold8_ISO8859_3,
		fold8_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_fold8_ISO8859_4),
		0, 2,
		etbl_fold8_ISO8859_4,
		csidx_fold8_ISO8859_4,
		fold8_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_fold8_ISO8859_5),
		0, 2,
		etbl_fold8_ISO8859_5,
		csidx_fold8_ISO8859_5,
		fold8_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_fold8_ISO8859_6),
		0, 2,
		etbl_fold8_ISO8859_6,
		csidx_fold8_ISO8859_6,
		fold8_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_fold8_ISO8859_7),
		0, 2,
		etbl_fold8_ISO8859_7,
		csidx_fold8_ISO8859_7,
		fold8_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_fold8_ISO8859_8),
		0, 2,
		etbl_fold8_ISO8859_8,
		csidx_fold8_ISO8859_8,
		fold8_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_fold8_ISO8859_9),
		0, 2,
		etbl_fold8_ISO8859_9,
		csidx_fold8_ISO8859_9,
		fold8_isctl,
	},
					/* 
					 *The following is a dummy that
					 * used by the UTF/ct conversion.
					 * for UTF extended segments.
					 */ 
	{				
		"UTF-8",
		SIZEOF(etbl_UTF_8),
		0, 0,
		etbl_UTF_8,
		csidx_fold8_ISO8859_1,
		fold8_isctl,
	},
	{ NULL,	0, 0, 0, NULL, NULL, NULL }
};
