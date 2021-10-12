static char sccsid[] = "@(#)37	1.10  src/bos/usr/ccs/lib/libc/NLunesctab.c, libcnls, bos411, 9428A410j 6/11/91 09:46:36";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLunesctab, _NLunescval
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <NLctype.h>
#include <stdlib.h>
extern int _NLunescval_sb(unsigned char *src, char slen, NLchar *dest);
extern int _NLunescval_932(unsigned char *src, char slen, NLchar *dest);

/*
 * NAME: NLunesctab
 *
 * NOTE: A table to control translation from informtion-preserving ASCII
 *       mnemonic to NLS code point.
 */
/*
 *
 *  Table to control translation from information-preserving ASCII
 *  mnemonic to NLS code point (plus function to access table).
 *  _NLunescval returns the NLchar corresponding to the mnemonic
 *  given as input, or -1 if the mnemonic is unknown.  If a
 *  corresponding NLchar is found, it is written to the input
 *  NLchar pointer as well.
 */


/* 
 *  Code page offsets
 */
#define P0B  0

struct NLescdata _NLunesctab[] = {
    
    /* To work this table MUST BE SORTED by first field */
    /* Beware! backslashes for C's sake make things sort out of order */
    
    "!",      P0B+0xad,    /* SP03       Spanish exclamation sign       */
    "##",     P0B+0xff,    /*            All ones                       */
    "#1",     P0B+0xb0,    /* SF14       Quarter hashed                 */
    "#2",     P0B+0xb1,    /* SF15       Half hashed                    */
    "#3",     P0B+0xb2,    /* SF16       Full hashed                    */
    "&m",     P0B+0xe6,    /* GM01/SM17  Mu small (Micro)               */
    "+-",     P0B+0xf1,    /* SA02       Plus or minus                  */
    "--",     P0B+0xee,    /* SM15       overbar                        */
    "-.",     P0B+0xaa,    /* SM66       Logical not                    */
    "-a",     P0B+0xa6,    /* SM21     ? Feminine sign                  */
    "-o",     P0B+0xa7,    /* SM20     ? Masculine sign                 */
    "..",     P0B+0xfa,    /* SM26       Middle dot (Product dot) (aka SD63) */
    "12",     P0B+0xab,    /* NF01       One half                       */
    "14",     P0B+0xac,    /* NF04       One quarter                    */
    "34",     P0B+0xf3,    /* NF05       three quarters                 */
    ":-",     P0B+0xf6,    /* SA06       Divide                         */
    "?",      P0B+0xa8,    /* SP16       Spanish question mark          */
    "A\"",    P0B+0x8e,    /* LA18/LA38  a umlaut capital               */
    "A'",     P0B+0xb5,    /* LA12       a acute capital                */
    "AE",     P0B+0x92,    /* LA52       ae diphthong capital           */
    "A^",     P0B+0xb6,    /* LA16       a circumflex capital           */
    "A`",     P0B+0xb7,    /* LA14       a grave capital                */
    "Ao",     P0B+0x8f,    /* LA28       a overcircle capital           */
    "A~",     P0B+0xc7,    /* LA20       a tilde capital                */
    "B",      P0B+0xdb,    /* SF61       bright cell                    */
    "B2",     P0B+0xdc,    /* SF57       bright cell - lower half       */
    "B8",     P0B+0xdf,    /* SF60       bright cell - upper half       */
    "BO",     P0B+0xdd,    /* SM65       broken vertical bar            */
    "C,",     P0B+0x80,    /* LC42       c cedilla capital              */
    "D+",     P0B+0xd1,    /* LD62       eth icelandic capital          */
    "D.",     P0B+0xcd,    /* SF43       double center box bar          */
    "D0",     P0B+0xba,    /* SF24       double vertical bar            */
    "D1",     P0B+0xc8,    /* SF38       double lower left corner box   */
    "D2",     P0B+0xca,    /* SF40       double bottom side middle      */
    "D3",     P0B+0xbc,    /* SF26       double lower right corner box  */
    "D4",     P0B+0xcc,    /* SF42       double left side middle        */
    "D5",     P0B+0xce,    /* SF44       double intersection            */
    "D6",     P0B+0xb9,    /* SF23       double right side middle       */
    "D7",     P0B+0xc9,    /* SF39       double upper left corner box   */
    "D8",     P0B+0xcb,    /* SF41       double top side middle         */
    "D9",     P0B+0xbb,    /* SF25       double upper right corner box  */
    "E\"",    P0B+0xd3,    /* LE18       e umlaut capital               */
    "E'",     P0B+0x90,    /* LE12       e acute capital                */
    "E^",     P0B+0xd2,    /* LE16       e circumflex capital           */
    "E`",     P0B+0xd4,    /* LE14       e grave capital                */
    "I\"",    P0B+0xd8,    /* LI18       i umlaut capital               */
    "I'",     P0B+0xd6,    /* LI12       i acute capital                */
    "IP",     P0B+0xe8,    /* LT64       thorn icelandic capital        */
    "I^",     P0B+0xd7,    /* LI16       i circumflex capital           */
    "I`",     P0B+0xde,    /* LI14       i grave capital                */
    "Ip",     P0B+0xe7,    /* LT63       thorn icelandic small          */
    "L=",     P0B+0x9c,    /* SC02       English pound sign             */
    "N~",     P0B+0xa5,    /* LN20       n tilde capital                */
    "O\"",    P0B+0x99,    /* LO18/LO38  o umlaut capital               */
    "O'",     P0B+0xe0,    /* LO12       o acute capital                */
    "O/",     P0B+0x9d,    /* LO62       o slash capital                */
    "O^",     P0B+0xe2,    /* LO16       o circumflex capital           */
    "O`",     P0B+0xe3,    /* LO14       o grave capital                */
    "O~",     P0B+0xe5,    /* LO20       o tilde capital                */
    "S.",     P0B+0xc4,    /* SF10/SM12  Center box bar                 */
    "S0",     P0B+0xb3,    /* SF11/SM13  Vertical bar                   */
    "S1",     P0B+0xc0,    /* SF02       Lower left corner box          */
    "S2",     P0B+0xc1,    /* SF07       Bottom side middle             */
    "S3",     P0B+0xd9,    /* SF04       Lower right corner box         */
    "S4",     P0B+0xc3,    /* SF08       Left side middle               */
    "S5",     P0B+0xc5,    /* SF05       Intersection                   */
    "S6",     P0B+0xb4,    /* SF09       Right side middle              */
    "S7",     P0B+0xda,    /* SF01       Upper left corner box          */
    "S8",     P0B+0xc2,    /* SF06       Top side middle                */
    "S9",     P0B+0xbf,    /* SF03       Upper right corner box         */
    "U\"",    P0B+0x9a,    /* LU18/LU38  u umlaut capital               */
    "U'",     P0B+0xe9,    /* LU12       u acute capital                */
    "U^",     P0B+0xea,    /* LU16       u circumflex capital           */
    "U`",     P0B+0xeb,    /* LU14       u grave capital                */
    "Y'",     P0B+0xed,    /* LY12       y acute capital                */
    "Y=",     P0B+0xbe,    /* SC05       Yen sign                       */
    "[]",     P0B+0xfe,    /* SV18       Vertical solid rectangle (aka SM47) */
    "^-",     P0B+0xf0,    /* SP32       syllable hyphen (SP10)         */
    "^1",     P0B+0xfb,    /* NS01       Superscript one                */
    "^2",     P0B+0xfd,    /* NS02       Superscript two (aka ND021)    */
    "^3",     P0B+0xfc,    /* ND031      superscript three              */
    "_\"",    P0B+0xf9,    /* SD17       umlaut accent                  */
    "_'",     P0B+0xef,    /* SD11       acute accent                   */
    "_,",     P0B+0xf7,    /* SD41       cedilla accent                 */
    "__",     P0B+0xf2,    /* SM10       double underscore              */
    "a\"",    P0B+0x84,    /* LA17/LA37  a umlaut small                 */
    "a'",     P0B+0xa0,    /* LA11       a acute small                  */
    "a^",     P0B+0x83,    /* LA15       a circumflex small             */
    "a`",     P0B+0x85,    /* LA13       a grave small                  */
    "ae",     P0B+0x91,    /* LA51       ae diphthong small             */
    "ao",     P0B+0x86,    /* LA27       a overcircle small             */
    "a~",     P0B+0xc6,    /* LA19       a tilde small                  */
    "c,",     P0B+0x87,    /* LC41       c cedilla small                */
    "c/",     P0B+0xbd,    /* SC04       Cent sign                      */
    "cO",     P0B+0xb8,    /* SM52       Copyright symbol               */
    "d+",     P0B+0xd0,    /* LD63       eth icelandic small            */
    "e\"",    P0B+0x89,    /* LE17/LE37  e umlaut small                 */
    "e'",     P0B+0x82,    /* LE11       e acute small                  */
    "e^",     P0B+0x88,    /* LE15       e circumflex small             */
    "e`",     P0B+0x8a,    /* LE13       e grave small                  */
    "f",      P0B+0x9f,    /* SC07       Florin sign                    */
    "i",      P0B+0xd5,    /* LI61       small i dotless                */
    "i\"",    P0B+0x8b,    /* LI17       i umlaut small                 */
    "i'",     P0B+0xa1,    /* LI11       i acute small                  */
    "i^",     P0B+0x8c,    /* LI15       i circumflex small             */
    "i`",     P0B+0x8d,    /* LI13       i grave small                  */
    "n~",     P0B+0xa4,    /* LN19       n tilde small                  */
    "o",      P0B+0xf8,    /* SM19       Degree (Overcircle)            */
    "o\"",    P0B+0x94,    /* LO17/LO37  o umlaut small                 */
    "o'",     P0B+0xa2,    /* LO11       o acute small                  */
    "o*",     P0B+0xcf,    /* SC01       international currency symbol  */
    "o/",     P0B+0x9b,    /* LO61       o slash small                  */
    "o^",     P0B+0x93,    /* LO15       o circumflex small             */
    "o`",     P0B+0x95,    /* LO13       o grave small                  */
    "o~",     P0B+0xe4,    /* LO19       o tilde small                  */
    "rO",     P0B+0xa9,    /* SM53       registered trademark symbol    */
    "ss",     P0B+0xe1,    /* LS61       s sharp small                  */
    "u\"",    P0B+0x81,    /* LU17/LU37  u umlaut small                 */
    "u'",     P0B+0xa3,    /* LU11       u acute small                  */
    "u^",     P0B+0x96,    /* LU15       u circumflex small             */
    "u`",     P0B+0x97,    /* LU13       u grave small                  */
    "x",      P0B+0x9e,    /* SA07       Multiply sign                  */
    "y\"",    P0B+0x98,    /* LY17/LY37  y umlaut small                 */
    "y'",     P0B+0xec,    /* LY11       y acute small                  */
    "{{",     P0B+0xae,    /* SP17       Left angle quotes              */
    "|P",     P0B+0xf4,    /* SM25       Paragraph                      */
    "|S",     P0B+0xf5,    /* SM24       Section                        */
    "}}",     P0B+0xaf,    /* SP18       Right angle quotes             */
};

unsigned _NLunesctsize = sizeof (_NLunesctab) / sizeof (struct NLescdata);


struct NLescdata _NLunesctab_932[] = {
    
    /* To work this table MUST BE SORTED by first field */
    /* Beware! backslashes for C's sake make things sort out of order */
    
    "j\"",    0xde,
    "j\"\"",  0xdf,
    "j'",     0xa3,
    "j*",     0xa5,
    "j,",     0xa4,
    "j-",     0xb0,
    "j.",     0xa1,
    "jA",     0xb1,
    "jCHI",   0xc1,
    "jE",     0xb4,
    "jHA",    0xca,
    "jHE",    0xcd,
    "jHI",    0xcb,
    "jHO",    0xce,
    "jHU",    0xcc,
    "jI",     0xb2,
    "jKA",    0xb6,
    "jKE",    0xb9,
    "jKI",    0xb7,
    "jKO",    0xba,
    "jKU",    0xb8,
    "jMA",    0xcf,
    "jME",    0xd2,
    "jMI",    0xd0,
    "jMO",    0xd3,
    "jMU",    0xd1,
    "jNA",    0xc5,
    "jNE",    0xc8,
    "jNI",    0xc6,
    "jNN",    0xdd,
    "jNO",    0xc9,
    "jNU",    0xc7,
    "jO",     0xb5,
    "jRA",    0xd7,
    "jRE",    0xda,
    "jRI",    0xd8,
    "jRO",    0xdb,
    "jRU",    0xd9,
    "jSA",    0xbb,
    "jSE",    0xbe,
    "jSI",    0xbc,
    "jSO",    0xbf,
    "jSU",    0xbd,
    "jTA",    0xc0,
    "jTE",    0xc3,
    "jTO",    0xc4,
    "jTSU",   0xc2,
    "jU",     0xb3,
    "jWA",    0xdc,
    "jWO",    0xa6,
    "jYA",    0xd4,
    "jYO",    0xd6,
    "jYU",    0xd5,
    "j`",     0xa2,
    "ja",     0xa7,
    "je",     0xaa,
    "ji",     0xa8,
    "jo",     0xab,
    "jtsu",   0xaf,
    "ju",     0xa9,
    "jya",    0xac,
    "jyo",    0xae,
    "jyu",    0xad,
};

unsigned _NLunesctsize_932 = sizeof (_NLunesctab_932) / sizeof (struct NLescdata);

/*
 * NAME: _NLunescval
 *
 * FUNCTION: Accesses to the table that translates from information
 *	     preserving ASCII to NLS code point, i.e. _NLunesctab.
 *
 * RETURN VALUE DESCRIPTION: -1 if the mnemonic is unknown. If a
 *           corresponding NLchar is found, returns the value of  
 *	     NLchar string. 
 */
int _NLunescval(unsigned char *src, char slen, NLchar *dest)
{
    if (MB_CUR_MAX == 1)
	return(_NLunescval_sb(src, slen, dest));
    else
	return(_NLunescval_932(src, slen, dest));
}

int _NLunescval_sb(unsigned char *src, char slen, NLchar *dest)
{
    register int min, max, n, key;
    register int mne = ((unsigned char)*src << 8) +
	((1 < slen) ? (unsigned char)*(src + 1) : 0);
    int byte0;
    struct NLescdata *x;
    
    min = 0; max = _NLunesctsize;
    while(1) {
	n = min + ((max - min) >> 1);
	x = &_NLunesctab[n];
	key = (x->key[0] << 8) + x->key[1];
	if (key == mne) {
	    byte0 = (x->value >> 8) ? x->value >> 8 : x->value;
	    _NCdec2(byte0, x->value, *dest);
	    return (*dest);
	}
	if (key < mne) {
	    if(n <= min)
		break;
	    min = n;
	} else {
	    if (max <= n)
		break;
	    max = n;
	}
    };
    return(-1);
}

int _NLunescval_932(unsigned char *src, char slen, NLchar *dest)
{	      
    register int min, max, n, key;
    register int mne = ((unsigned char)*src << 24) + 
	((1 < slen) ? ((unsigned char)*(src + 1) << 16)  + 
	 ((2 < slen) ? ((unsigned char)*(src + 2) << 8)  + 
	  ((3 < slen) ? (unsigned char)*(src + 3) : 0) : 0) : 0);
    int byte0;
    struct NLescdata *x;
    
    min = 0; max = _NLunesctsize_932;
    while(1) {
	n = min + ((max - min) >> 1);
	x = &_NLunesctab_932[n];
	key = (x->key[0] << 24) + (x->key[1] << 16) + (x->key[2] << 8) + x->key[3];
	if (key == mne) {
	    byte0 = (x->value >> 8) ? x->value >> 8 : x->value;
	    _NCdec2(byte0, x->value, *dest);
	    return (*dest);
	}
	if (key < mne) {
	    if(n <= min)
		break;
	    min = n;
	} else {
	    if (max <= n)
		break;
	    max = n;
	}
    };
    return(-1);
}


