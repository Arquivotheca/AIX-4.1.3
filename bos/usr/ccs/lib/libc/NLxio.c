static char sccsid[] = "@(#)14	1.4  src/bos/usr/ccs/lib/libc/NLxio.c, libcnls, bos411, 9428A410j 6/16/90 01:27:27";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLxio
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>

#include <NLxio.h>

#define ERR -1
static struct NLxtbl *NLxload(char *);
/*
	Default input table
*/

struct NLxtbl *_NLxotbl;
struct NLxtbl *_NLxitbl;

static struct NLxtbl def_in = {
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* input name */
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* output name */
		0x00,	/*	NUL	*/
		0x01,	/*	SOH	*/
		0x02,	/*	STX	*/
		0x03,	/*	ETX	*/
		0xDC,	/*	SEL	*/
		0x09,	/*	HT	*/
		0xC3,	/*	RNL	*/
		0x7F,	/*	DEL	*/
		0xCA,	/*	GE	*/
		0xB2,	/*	SPS	*/
		0xD5,	/*	RPT	*/
		0x0B,	/*	VT	*/
		0x0C,	/*	FF	*/
		0x0D,	/*	CR	*/
		0x0E,	/*	SO	*/
		0x0F,	/*	SI	*/
		0x10,	/*	DLE	*/
		0x11,	/*	DC1	*/
		0x12,	/*	DC2	*/
		0x13,	/*	DC3	*/
		0xDB,	/*	RES, ENP	*/
		0xDA,	/*	NL	*/
		0x08,	/*	BS	*/
		0xC1,	/*	POC	*/
		0x18,	/*	CAN	*/
		0x19,	/*	EM	*/
		0xC8,	/*	UBS	*/
		0xF2,	/*	CU1	*/
		0x1C,	/*	IFS	*/
		0x1D,	/*	IGS	*/
		0x1E,	/*	IRS	*/
		0x1F,	/*	IUS, ITB	*/
		0xC4,	/*	DS	*/
		0xB3,	/*	SOS	*/
		0xC0,	/*	FS	*/
		0xD9,	/*	WUS	*/
		0xBF,	/*	BYP, INP	*/
		0x0A,	/*	LF	*/
		0x17,	/*	ETB	*/
		0x1B,	/*	ESC	*/
		0xB4,	/*	SA	*/
		0xC2,	/*	SFE	*/
		0xC5,	/*	SM, SW	*/
		0xB0,	/*	CSP	*/
		0xB1,	/*	MFA	*/
		0x05,	/*	ENQ	*/
		0x06,	/*	ACK	*/
		0x07,	/*	BEL	*/
		0xCD,	/*	Reserved	*/
		0xBA,	/*	Reserved	*/
		0x16,	/*	SYN	*/
		0xBC,	/*	IR	*/
		0xBB,	/*	PP	*/
		0xC9,	/*	TRN	*/
		0xCC,	/*	NBS	*/
		0x04,	/*	EOT	*/
		0xB9,	/*	SBS	*/
		0xCB,	/*	IT	*/
		0xCE,	/*	RFF	*/
		0xDF,	/*	CU3	*/
		0x14,	/*	DC4	*/
		0x15,	/*	NAK	*/
		0xFE,	/*	Reserved	*/
		0x1A,	/*	SUB	*/
		0x20,	/*	Space	*/
		0xFF,	/*	Required Space	*/
		0x83,	/*	a circumflex small	*/
		0x84,	/*	a umlaut small	*/
		0x85,	/*	a grave small	*/
		0xA0,	/*	a acute small	*/
		0xC6,	/*	a tilde small	*/
		0x86,	/*	a overcircle small	*/
		0x87,	/*	c cedilla small	*/
		0xA4,	/*	n tilde small	*/
		0x5B,	/*	Left Bracket	*/
		0x2E,	/*	Period	*/
		0x3C,	/*	Less Than Sign	*/
		0x28,	/*	Left Parenthesis	*/
		0x2B,	/*	Plus Sign	*/
		0x21,	/*	Exclamation Point	*/
		0x26,	/*	Ampersand	*/
		0x82,	/*	e acute small	*/
		0x88,	/*	e circumflex small	*/
		0x89,	/*	e umlaut small	*/
		0x8A,	/*	e grave small	*/
		0xA1,	/*	i acute small	*/
		0x8C,	/*	i circumflex small	*/
		0x8B,	/*	i umlaut small	*/
		0x8D,	/*	i grave small	*/
		0xE1,	/*	s sharp small	*/
		0x5D,	/*	Right Bracket	*/
		0x24,	/*	Dollar Sign	*/
		0x2A,	/*	Asterisk	*/
		0x29,	/*	Right Parenthesis	*/
		0x3B,	/*	Semicolon	*/
		0x5E,	/*	Circumflex	*/
		0x2D,	/*	Hyphen	*/
		0x2F,	/*	Slash	*/
		0xB6,	/*	a circumflex capital	*/
		0x8E,	/*	a umlaut capital	*/
		0xB7,	/*	a grave capital	*/
		0xB5,	/*	a acute capital	*/
		0xC7,	/*	a tilde capital	*/
		0x8F,	/*	a overcircle capital	*/
		0x80,	/*	c cedilla capital	*/
		0xA5,	/*	n tilde capital	*/
		0xDD,	/*	Vertical Line Broken	*/
		0x2C,	/*	Comma	*/
		0x25,	/*	Percent Sign	*/
		0x5F,	/*	Underline	*/
		0x3E,	/*	Greater Than Sign	*/
		0x3F,	/*	Question Mark	*/
		0x9B,	/*	o slash small	*/
		0x90,	/*	e acute capital	*/
		0xD2,	/*	e circumflex capital	*/
		0xD3,	/*	e umlaut capital	*/
		0xD4,	/*	e grave capital	*/
		0xD6,	/*	i acute capital	*/
		0xD7,	/*	i circumflex capital	*/
		0xD8,	/*	i umlaut capital	*/
		0xDE,	/*	i grave capital	*/
		0x60,	/*	Grave Accent	*/
		0x3A,	/*	Colon	*/
		0x23,	/*	Number Sign	*/
		0x40,	/*	At Sign	*/
		0x27,	/*	Apostrophe	*/
		0x3D,	/*	Equal Sign	*/
		0x22,	/*	Double Quote	*/
		0x9D,	/*	o slash capital	*/
		0x61,	/*	a	*/
		0x62,	/*	b	*/
		0x63,	/*	c	*/
		0x64,	/*	d	*/
		0x65,	/*	e	*/
		0x66,	/*	f	*/
		0x67,	/*	g	*/
		0x68,	/*	h	*/
		0x69,	/*	i	*/
		0xAE,	/*	Left angle quotes	*/
		0xAF,	/*	Right angle quotes	*/
		0xD0,	/*	eth icelandic small	*/
		0xEC,	/*	y acute small	*/
		0xE7,	/*	thorn icelandic small	*/
		0xF1,	/*	Plus or minus	*/
		0xF8,	/*	Degree (Overcircle)	*/
		0x6A,	/*	j	*/
		0x6B,	/*	k	*/
		0x6C,	/*	l	*/
		0x6D,	/*	m	*/
		0x6E,	/*	n	*/
		0x6F,	/*	o	*/
		0x70,	/*	p	*/
		0x71,	/*	q	*/
		0x72,	/*	r	*/
		0xA6,	/*	Feminine Sign	*/
		0xA7,	/*	Masculine Sign	*/
		0x91,	/*	ae diphthong small	*/
		0xF7,	/*	cedilla accent	*/
		0x92,	/*	ae diphthong capital	*/
		0xCF,	/*	international currency symbol	*/
		0xE6,	/*	Mu small (Micro)	*/
		0x7E,	/*	Tilde Accent	*/
		0x73,	/*	s	*/
		0x74,	/*	t	*/
		0x75,	/*	u	*/
		0x76,	/*	v	*/
		0x77,	/*	w	*/
		0x78,	/*	x	*/
		0x79,	/*	y	*/
		0x7A,	/*	z	*/
		0xAD,	/*	Spanish exclamation Sign	*/
		0xA8,	/*	Spanish Question Mark	*/
		0xD1,	/*	eth icelandic capital	*/
		0xED,	/*	y acute capital	*/
		0xE8,	/*	thorn icelandic capital	*/
		0xA9,	/*	registered trademark	*/
		0xBD,	/*	Cent Sign	*/
		0x9C,	/*	English Pound Sign	*/
		0xBE,	/*	Yen Sign	*/
		0xFA,	/*	Middle dot (Product dot)	*/
		0xB8,	/*	Florin Sign	*/
		0xF5,	/*	Section	*/
		0xF4,	/*	Paragraph	*/
		0xAC,	/*	One quarter	*/
		0xAB,	/*	One half	*/
		0xF3,	/*	three quarters	*/
		0xAA,	/*	Logical not	*/
		0x7C,	/*	Logical OR	*/
		0xEE,	/*	overbar	*/
		0xF9,	/*	umlaut accent	*/
		0xEF,	/*	acute accent	*/
		0x9E,	/*	double underscore	*/
		0x7B,	/*	Left Brace	*/
		0x41,	/*	A	*/
		0x42,	/*	B	*/
		0x43,	/*	C	*/
		0x44,	/*	D	*/
		0x45,	/*	E	*/
		0x46,	/*	F	*/
		0x47,	/*	G	*/
		0x48,	/*	H	*/
		0x49,	/*	I	*/
		0xF0,	/*	syllable hyphen	*/
		0x93,	/*	o circumflex small	*/
		0x94,	/*	o umlaut small	*/
		0x95,	/*	o grave small	*/
		0xA2,	/*	o acute small	*/
		0xE4,	/*	o tilde small	*/
		0x7D,	/*	Right Brace	*/
		0x4A,	/*	J	*/
		0x4B,	/*	K	*/
		0x4C,	/*	L	*/
		0x4D,	/*	M	*/
		0x4E,	/*	N	*/
		0x4F,	/*	O	*/
		0x50,	/*	P	*/
		0x51,	/*	Q	*/
		0x52,	/*	R	*/
		0xFB,	/*	small i dotless	*/
		0x96,	/*	u circumflex small	*/
		0x81,	/*	u umlaut small	*/
		0x97,	/*	u grave small	*/
		0xA3,	/*	u acute small	*/
		0x98,	/*	y umlaut small	*/
		0x5C,	/*	Reverse Slash	*/
		0xF6,	/*	Numeric Space	*/
		0x53,	/*	S	*/
		0x54,	/*	T	*/
		0x55,	/*	U	*/
		0x56,	/*	V	*/
		0x57,	/*	W	*/
		0x58,	/*	X	*/
		0x59,	/*	Y	*/
		0x5A,	/*	Z	*/
		0xFD,	/*	Superscript two	*/
		0xE2,	/*	o circumflex capital	*/
		0x99,	/*	o umlaut capital	*/
		0xE3,	/*	o grave capital	*/
		0xE0,	/*	o acute capital	*/
		0xE5,	/*	o tilde capital	*/
		0x30,	/*	0	*/
		0x31,	/*	1	*/
		0x32,	/*	2	*/
		0x33,	/*	3	*/
		0x34,	/*	4	*/
		0x35,	/*	5	*/
		0x36,	/*	6	*/
		0x37,	/*	7	*/
		0x38,	/*	8	*/
		0x39,	/*	9	*/
		0xFC,	/*	Superscript three	*/
		0xEA,	/*	u circumflex capital	*/
		0x9A,	/*	u umlaut capital	*/
		0xEB,	/*	u grave capital	*/
		0xE9,	/*	u acute capital	*/
		0x9F,	/*	All ones	*/ 
} ;

/*
	default output table
*/

static struct NLxtbl def_out = {
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* input name */
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* output name */
		0x00,	/*	NUL	*/
		0x01,	/*	SOH	*/
		0x02,	/*	STX	*/
		0x03,	/*	ETX	*/
		0x37,	/*	EOT	*/
		0x2D,	/*	ENQ	*/
		0x2E,	/*	ACK	*/
		0x2F,	/*	BEL	*/
		0x16,	/*	BS	*/
		0x05,	/*	HT	*/
		0x25,	/*	LF	*/
		0x0B,	/*	VT	*/
		0x0C,	/*	FF	*/
		0x0D,	/*	CR	*/
		0x0E,	/*	SO	*/
		0x0F,	/*	SI	*/
		0x10,	/*	DLE	*/
		0x11,	/*	DC1	*/
		0x12,	/*	DC2	*/
		0x13,	/*	DC3	*/
		0x3C,	/*	DC4	*/
		0x3D,	/*	NAK	*/
		0x32,	/*	SYN	*/
		0x26,	/*	ETB	*/
		0x18,	/*	CAN	*/
		0x19,	/*	EM	*/
		0x3F,	/*	SUB	*/
		0x27,	/*	ESC	*/
		0x1C,	/*	SS4	*/
		0x1D,	/*	SS3	*/
		0x1E,	/*	SS2	*/
		0x1F,	/*	SS1	*/
		0x40,	/*	Space	*/
		0x4F,	/*	Exclamation Point	*/
		0x7F,	/*	Double Quote	*/
		0x7B,	/*	Number Sign	*/
		0x5B,	/*	Dollar Sign	*/
		0x6C,	/*	Percent Sign	*/
		0x50,	/*	Ampersand	*/
		0x7D,	/*	Apostrophe	*/
		0x4D,	/*	Left Parenthesis	*/
		0x5D,	/*	Right Parenthesis	*/
		0x5C,	/*	Asterisk	*/
		0x4E,	/*	Plus Sign	*/
		0x6B,	/*	Comma	*/
		0x60,	/*	Hyphen	*/
		0x4B,	/*	Period	*/
		0x61,	/*	Slash	*/
		0xF0,	/*	Zero	*/
		0xF1,	/*	One	*/
		0xF2,	/*	Two	*/
		0xF3,	/*	Three	*/
		0xF4,	/*	Four	*/
		0xF5,	/*	Five	*/
		0xF6,	/*	Six	*/
		0xF7,	/*	Seven	*/
		0xF8,	/*	Eight	*/
		0xF9,	/*	Nine	*/
		0x7A,	/*	Colon	*/
		0x5E,	/*	Semicolon	*/
		0x4C,	/*	Less Than Sign	*/
		0x7E,	/*	Equal Sign	*/
		0x6E,	/*	Greater Than Sign	*/
		0x6F,	/*	Question Mark	*/
		0x7C,	/*	At Sign	*/
		0xC1,	/*	A	*/
		0xC2,	/*	B	*/
		0xC3,	/*	C	*/
		0xC4,	/*	D	*/
		0xC5,	/*	E	*/
		0xC6,	/*	F	*/
		0xC7,	/*	G	*/
		0xC8,	/*	H	*/
		0xC9,	/*	I	*/
		0xD1,	/*	J	*/
		0xD2,	/*	K	*/
		0xD3,	/*	L	*/
		0xD4,	/*	M	*/
		0xD5,	/*	N	*/
		0xD6,	/*	O	*/
		0xD7,	/*	P	*/
		0xD8,	/*	Q	*/
		0xD9,	/*	R	*/
		0xE2,	/*	S	*/
		0xE3,	/*	T	*/
		0xE4,	/*	U	*/
		0xE5,	/*	V	*/
		0xE6,	/*	W	*/
		0xE7,	/*	X	*/
		0xE8,	/*	Y	*/
		0xE9,	/*	Z	*/
		0x4A,	/*	Left Bracket	*/
		0xE0,	/*	Reverse Slash	*/
		0x5A,	/*	Right Bracket	*/
		0x5F,	/*	Circumflex	*/
		0x6D,	/*	Underline	*/
		0x79,	/*	Grave Accent	*/
		0x81,	/*	a	*/
		0x82,	/*	b	*/
		0x83,	/*	c	*/
		0x84,	/*	d	*/
		0x85,	/*	e	*/
		0x86,	/*	f	*/
		0x87,	/*	g	*/
		0x88,	/*	h	*/
		0x89,	/*	i	*/
		0x91,	/*	j	*/
		0x92,	/*	k	*/
		0x93,	/*	l	*/
		0x94,	/*	m	*/
		0x95,	/*	n	*/
		0x96,	/*	o	*/
		0x97,	/*	p	*/
		0x98,	/*	q	*/
		0x99,	/*	r	*/
		0xA2,	/*	s	*/
		0xA3,	/*	t	*/
		0xA4,	/*	u	*/
		0xA5,	/*	v	*/
		0xA6,	/*	w	*/
		0xA7,	/*	x	*/
		0xA8,	/*	y	*/
		0xA9,	/*	z	*/
		0xC0,	/*	Left Brace	*/
		0xBB,	/*	Logical OR	*/
		0xD0,	/*	Right Brace	*/
		0xA1,	/*	Tilde Accent	*/
		0x07,	/*	DEL	*/
		0x68,	/*	c cedilla capital	*/
		0xDC,	/*	u umlaut small	*/
		0x51,	/*	e acute small	*/
		0x42,	/*	a circumflex small	*/
		0x43,	/*	a umlaut small	*/
		0x44,	/*	a grave small	*/
		0x47,	/*	a overcircle small	*/
		0x48,	/*	c cedilla small	*/
		0x52,	/*	e circumflex small	*/
		0x53,	/*	e umlaut small	*/
		0x54,	/*	e grave small	*/
		0x57,	/*	i umlaut small	*/
		0x56,	/*	i circumflex small	*/
		0x58,	/*	i grave small	*/
		0x63,	/*	a umlaut capital	*/
		0x67,	/*	a overcircle capital	*/
		0x71,	/*	e acute capital	*/
		0x9C,	/*	ae diphthong small	*/
		0x9E,	/*	ae diphthong capital	*/
		0xCB,	/*	o circumflex small	*/
		0xCC,	/*	o umlaut small	*/
		0xCD,	/*	o grave small	*/
		0xDB,	/*	u circumflex small	*/
		0xDD,	/*	u grave small	*/
		0xDF,	/*	y umlaut small	*/
		0xEC,	/*	o umlaut capital	*/
		0xFC,	/*	u umlaut capital	*/
		0x70,	/*	o slash small	*/
		0xB1,	/*	English pound sign	*/
		0x80,	/*	o slash capital	*/
		0xBF,	/*	Multiply sign	*/
		0xFF,	/*	Florin sign	*/
		0x45,	/*	a acute small	*/
		0x55,	/*	i acute small	*/
		0xCE,	/*	o acute small	*/
		0xDE,	/*	u acute small	*/
		0x49,	/*	n tilde small	*/
		0x69,	/*	n tilde capital	*/
		0x9A,	/*	Feminine sign	*/
		0x9B,	/*	Masculine sign	*/
		0xAB,	/*	Spanish question mark	*/
		0xAF,	/*	registered trademark	*/
		0xBA,	/*	Logical not	*/
		0xB8,	/*	One half	*/
		0xB7,	/*	One quarter	*/
		0xAA,	/*	Spanish exclamation sign	*/
		0x8A,	/*	Left angle quotes	*/
		0x8B,	/*	Right angle quotes	*/
		0x2B,	/*	Quarter hashed	*/
		0x2C,	/*	Half hashed	*/
		0x09,	/*	Full hashed	*/
		0x21,	/*	Center Box Vertical bar	*/
		0x28,	/*	Right side middle	*/
		0x65,	/*	a acute capital	*/
		0x62,	/*	a circumflex capital	*/
		0x64,	/*	a grave capital	*/
		0xB4,	/*	Copyright symbol	*/
		0x38,	/*	Double right side middle	*/
		0x31,	/*	Double vertical bar	*/
		0x34,	/*	Double upper right corner box	*/
		0x33,	/*	Double lower right corner box	*/
		0xB0,	/*	Cent sign	*/
		0xB2,	/*	Yen sign	*/
		0x24,	/*	Upper right corner box	*/
		0x22,	/*	Lower left corner box	*/
		0x17,	/*	Bottom side middle	*/
		0x29,	/*	Top side middle	*/
		0x06,	/*	Left side middle	*/
		0x20,	/*	Center box bar	*/
		0x2A,	/*	Intersection	*/
		0x46,	/*	a tilde small	*/
		0x66,	/*	a tilde capital	*/
		0x1A,	/*	Double lower left corner box	*/
		0x35,	/*	Double upper left corner box	*/
		0x08,	/*	Double bottom side middle	*/
		0x39,	/*	Double top side middle	*/
		0x36,	/*	Double left side middle	*/
		0x30,	/*	Double center box bar	*/
		0x3A,	/*	Double intersection	*/
		0x9F,	/*	international currency symbol	*/
		0x8C,	/*	eth icelandic small	*/
		0xAC,	/*	eth icelandic capital	*/
		0x72,	/*	e circumflex capital	*/
		0x73,	/*	e umlaut capital	*/
		0x74,	/*	e grave capital	*/
		0x0A,	/*	small i dotless	*/
		0x75,	/*	i acute capital	*/
		0x76,	/*	i circumflex capital	*/
		0x77,	/*	i umlaut capital	*/
		0x23,	/*	Lower right corner box	*/
		0x15,	/*	Upper left corner box	*/
		0x14,	/*	Bright character cell	*/
		0x04,	/*	Bright character cell - lower half	*/
		0x6A,	/*	Vertical Line Broken	*/
		0x78,	/*	i grave capital	*/
		0x3B,	/*	Bright character cell - upper half	*/
		0xEE,	/*	o acute capital	*/
		0x59,	/*	s sharp small	*/
		0xEB,	/*	o circumflex capital	*/
		0xED,	/*	o grave capital	*/
		0xCF,	/*	o tilde small	*/
		0xEF,	/*	o tilde capital	*/
		0xA0,	/*	Mu small (Micro)	*/
		0x8E,	/*	thorn icelandic small	*/
		0xAE,	/*	thorn icelandic capital	*/
		0xFE,	/*	u acute capital	*/
		0xFB,	/*	u circumflex capital	*/
		0xFD,	/*	u grave capital	*/
		0x8D,	/*	y acute small	*/
		0xAD,	/*	y acute capital	*/
		0xBC,	/*	overbar	*/
		0xBE,	/*	acute accent	*/
		0xCA,	/*	syllable hyphen	*/
		0x8F,	/*	Plus or minus	*/
		0x1B,	/*	double underscore	*/
		0xB9,	/*	three quarters	*/
		0xB6,	/*	Paragraph	*/
		0xB5,	/*	Section	*/
		0xE1,	/*	Divide	*/
		0x9D,	/*	cedilla accent	*/
		0x90,	/*	Degree (Overcircle)	*/
		0xBD,	/*	umlaut accent	*/
		0xB3,	/*	Middle dot (Product dot)	*/
		0xDA,	/*	Superscript one	*/
		0xFA,	/*	Superscript three	*/
		0xEA,	/*	Superscript two	*/
		0x3E,	/*	Vertical solid rectangle	*/
		0x41	/*	All ones (Required space)	*/
};

/*
 * NAME: NLxin
 *	
 * FUNCTION: Translate EBCDIC to ASCII through a translation
 * table NLxin() queries the NLIN environment variable
 * for the pathname of the translation table.  NLIN is only 
 * checked on the first invocation of NLxin().
 *
 * RETURN VALUE DESCRIPTION: NLxin returns the number of 
 * characters translated.  That number will be the lesser of 
 * strlen(s) and n.  
 */  

NLxin(char *t,char *s,int n) 
/*
char *t;		 Target string  (Ascii) 
char *s;		 Source string  (EBSDIC) 
int n;			 Maximum number of chars to be 
				translated 
*/
{
	char *tblname;
	extern char *getenv();
	
	int i;

	if (!_NLxitbl) {
		if (!(tblname = getenv("NLIN")))
			_NLxitbl = &def_in;
		else {
			if (!(_NLxitbl = NLxload(tblname)))
				_NLxitbl = &def_in;
		}
	}
	for (i = 0 ; i < n && s[i] ; i++) 
		t[i] = _NLxitbl->chrs[s[i]];
	if (i < n)
		t[i] = (char)NULL;
	return(i);
}

/*
 * NAME: NLxout
 *	
 * FUNCTION: Translate ASCII to EBCDIC through a translation
 * table.  The routine queries the NLOUT environment variable
 * for the pathname to the translation table.  NLOUT is only
 * checked on the first invocation of the routine.
 *
 * RETURN VALUE DESCRIPTION: The routine returns the number of 
 * characters translated.
 */  

NLxout(char *t,char *s,int n) 
/*
char *t;		 Target string  (EBSDIC) 
char *s;		 Source string  (Ascii) 
int n;			 Maximum number of chars to be translated 
*/
{
	char *tblname;
	extern char *getenv();
	int i;

	if (!_NLxotbl) {	/* if the table has not been loaded .. */
		if (!(tblname = getenv("NLOUT")))
			_NLxotbl = &def_out;
		else {
			if (!(_NLxotbl = NLxload(tblname)))
				_NLxotbl = &def_out;
		}
	}

	for (i = 0 ; i < n && s[i] ; i++) 
		t[i] = _NLxotbl->chrs[s[i]];
	if (i < n)
		t[i] = (char)NULL;
	return(i);
}

/*
 *
 * NAME: NLxload
 *	
 * FUNCTION:  Read a translation table into memory and return 
 * a pointer to it.
 *
 * NOTES:  This routine is not designed to be called except by
 * NLxin() and NLxout (hence the double underscore prefix).
 *
 * RETURN VALUE DESCRIPTION: NLxload returns the address
 * of the translation table. If there is an error the return
 * value is NULL.
 *
 */  

static struct NLxtbl *NLxload(char *name)
/*
char *name;		 path name to the table to be loaded 		
*/
{
	int fd;		/* file descriptor for the table file 		*/
	struct NLxtbl *tm;  		/* pointer to a table 	*/


	if ((fd = open(name,O_RDONLY)) == ERR) {
		return(NULL);			/*  Unable to open file	*/
	}

	if (!(tm = (struct NLxtbl *) malloc(sizeof(struct NLxtbl)))) {
		close(fd);
		return(NULL);			/*  Unable to get memory*/
	}

	if (read(fd,tm,sizeof(*tm)) != sizeof(*tm)) {
		free(tm);
		close(fd);
		return(NULL);	/* Unable to read from file */
	}
	close(fd);
	return(tm);				/* Normal return 	*/
}

/*
 *
 * NAME: NLxstart
 *	
 * FUNCTION:  Read a translation table into memory and return 
 * a pointer to it.
 *
 * RETURN VALUE DESCRIPTION: NLxload returns the address
 * of the translation table. If there is an error the return
 * value is NULL.
 *
 */  

void NLxstart()
{
	char *tblname;
	
	if (_NLxotbl && _NLxotbl != &def_out)
		free(_NLxotbl);

	if (_NLxitbl && _NLxitbl != &def_in)
		free(_NLxitbl);
		
	if (!(tblname = getenv("NLOUT")))
		_NLxotbl = &def_out;
	else {
		if (!(_NLxotbl = NLxload(tblname)))
			_NLxotbl = &def_out;
	}

	if (!(tblname = getenv("NLIN")))
		_NLxitbl = &def_in;
	else {
		if (!(_NLxitbl = NLxload(tblname)))
			_NLxitbl = &def_out;
	}
}
