static char sccsid[] = "@(#)73	1.9  src/bos/usr/ccs/lib/libc/NLflattab.c, libcnls, bos411, 9428A410j 6/11/91 09:46:29";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLflattab
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

/*
 * NAME: _NLflattab 
 *
 * NOTE: Table to control translation from NLchar to mnemonic for 
 *       appearance preserving (i.e., flatttening) ASCII conversion.
 *       Table is indexed by NLchar value.
 *	 _NLflattsize = sizeof (_NLflattab)	 
 */
/* (#)NLflattab.c	7.1 - 87/06/16 - 01:08:44
 *
 *  Table to control translation from NLchar to mnemonic for appearance-
 *  preserving (i.e., flattening) ASCII conversion.  Table is indexed
 *  by NLchar value.
 */

unsigned char _NLflattab[] = {
'C',     /* P0B+0x80,    LC42       c cedilla capital              */
'u',     /* P0B+0x81,    LU17/LU37  u umlaut small                 */
'e',     /* P0B+0x82,    LE11       e acute small                  */
'a',     /* P0B+0x83,    LA15       a circumflex small             */
'a',     /* P0B+0x84,    LA17/LA37  a umlaut small                 */
'a',     /* P0B+0x85,    LA13       a grave small                  */
'a',     /* P0B+0x86,    LA27       a overcircle small             */
'c',     /* P0B+0x87,    LC41       c cedilla small                */
'e',     /* P0B+0x88,    LE15       e circumflex small             */
'e',     /* P0B+0x89,    LE17/LE37  e umlaut small                 */
'e',     /* P0B+0x8a,    LE13       e grave small                  */
'i',     /* P0B+0x8b,    LI17       i umlaut small                 */
'i',     /* P0B+0x8c,    LI15       i circumflex small             */
'i',     /* P0B+0x8d,    LI13       i grave small                  */
'A',     /* P0B+0x8e,    LA18/LA38  a umlaut capital               */
'A',     /* P0B+0x8f,    LA28       a overcircle capital           */
'E',     /* P0B+0x90,    LE12       e acute capital                */
'a',     /* P0B+0x91,    LA51       ae diphthong small             */
'A',     /* P0B+0x92,    LA52       ae diphthong capital           */
'o',     /* P0B+0x93,    LO15       o circumflex small             */
'o',     /* P0B+0x94,    LO17/LO37  o umlaut small                 */
'o',     /* P0B+0x95,    LO13       o grave small                  */
'u',     /* P0B+0x96,    LU15       u circumflex small             */
'u',     /* P0B+0x97,    LU13       u grave small                  */
'y',     /* P0B+0x98,    LY17/LY37  y umlaut small                 */
'O',     /* P0B+0x99,    LO18/LO38  o umlaut capital               */
'U',     /* P0B+0x9a,    LU18/LU38  u umlaut capital               */
'o',     /* P0B+0x9b,    LO61       o slash small                  */
'#',     /* P0B+0x9c,    SC02       English pound sign             */
'O',     /* P0B+0x9d,    LO62       o slash capital                */
'x',     /* P0B+0x9e,    SA07       Multiply sign                  */
'f',     /* P0B+0x9f,    SC07       Florin sign                    */
'a',     /* P0B+0xa0,    LA11       a acute small                  */
'i',     /* P0B+0xa1,    LI11       i acute small                  */
'o',     /* P0B+0xa2,    LO11       o acute small                  */
'u',     /* P0B+0xa3,    LU11       u acute small                  */
'n',     /* P0B+0xa4,    LN19       n tilde small                  */
'N',     /* P0B+0xa5,    LN20       n tilde capital                */
'x',     /* P0B+0xa6,    SM21     ? Feminine sign                  */
'o',     /* P0B+0xa7,    SM20     ? Masculine sign                 */
'?',     /* P0B+0xa8,    SP16       Spanish question mark          */
'r',     /* P0B+0xa9,    SM53       registered trademark symbol    */
'~',     /* P0B+0xaa,    SM66       Logical not                    */
'?',     /* P0B+0xab,    NF01       One half                       */
'?',     /* P0B+0xac,    NF04       One quarter                    */
'!',     /* P0B+0xad,    SP03       Spanish exclamation sign       */
'<',     /* P0B+0xae,    SP17       Left angle quotes              */
'>',     /* P0B+0xaf,    SP18       Right angle quotes             */
'#',     /* P0B+0xb0,    SF14       Quarter hashed                 */
'#',     /* P0B+0xb1,    SF15       Half hashed                    */
'#',     /* P0B+0xb2,    SF16       Full hashed                    */
'|',     /* P0B+0xb3,    SF11/SM13  Vertical bar                   */
'|',     /* P0B+0xb4,    SF09       Right side middle              */
'A',     /* P0B+0xb5,    LA12       a acute capital                */
'A',     /* P0B+0xb6,    LA16       a circumflex capital           */
'A',     /* P0B+0xb7,    LA14       a grave capital                */
'c',     /* P0B+0xb8,    SM52       Copyright symbol               */
'|',     /* P0B+0xb9,    SF23       double right side middle       */
'|',     /* P0B+0xba,    SF24       double vertical bar            */
'+',     /* P0B+0xbb,    SF25       double upper right corner box  */
'+',     /* P0B+0xbc,    SF26       double lower right corner box  */
'c',     /* P0B+0xbd,    SC04       Cent sign                      */
'Y',     /* P0B+0xbe,    SC05       Yen sign                       */
'+',     /* P0B+0xbf,    SF03       Upper right corner box         */
'+',     /* P0B+0xc0,    SF02       Lower left corner box          */
'-',     /* P0B+0xc1,    SF07       Bottom side middle             */
'-',     /* P0B+0xc2,    SF06       Top side middle                */
'|',     /* P0B+0xc3,    SF08       Left side middle               */
'-',     /* P0B+0xc4,    SF10/SM12  Center box bar                 */
'+',     /* P0B+0xc5,    SF05       Intersection                   */
'a',     /* P0B+0xc6,    LA19       a tilde small                  */
'A',     /* P0B+0xc7,    LA20       a tilde capital                */
'+',     /* P0B+0xc8,    SF38       double lower left corner box   */
'+',     /* P0B+0xc9,    SF39       double upper left corner box   */
'-',     /* P0B+0xca,    SF40       double bottom side middle      */
'-',     /* P0B+0xcb,    SF41       double top side middle         */
'|',     /* P0B+0xcc,    SF42       double left side middle        */
'-',     /* P0B+0xcd,    SF43       double center box bar          */
'+',     /* P0B+0xce,    SF44       double intersection            */
'*',     /* P0B+0xcf,    SC01       international currency symbol  */
'd',     /* P0B+0xd0,    LD63       eth icelandic small            */
'D',     /* P0B+0xd1,    LD62       eth icelandic capital          */
'E',     /* P0B+0xd2,    LE16       e circumflex capital           */
'E',     /* P0B+0xd3,    LE18       e umlaut capital               */
'E',     /* P0B+0xd4,    LE14       e grave capital                */
'i',     /* P0B+0xd5,    LI61       small i dotless                */
'I',     /* P0B+0xd6,    LI12       i acute capital                */
'I',     /* P0B+0xd7,    LI16       i circumflex capital           */
'I',     /* P0B+0xd8,    LI18       i umlaut capital               */
'+',     /* P0B+0xd9,    SF04       Lower right corner box         */
'+',     /* P0B+0xda,    SF01       Upper left corner box          */
'#',     /* P0B+0xdb,    SF61       bright cell                    */
'#',     /* P0B+0xdc,    SF57       bright cell - lower half       */
'|',     /* P0B+0xdd,    SM65       broken vertical bar            */
'I',     /* P0B+0xde,    LI14       i grave capital                */
'#',     /* P0B+0xdf,    SF60       bright cell - upper half       */
'O',     /* P0B+0xe0,    LO12       o acute capital                */
'B',     /* P0B+0xe1,    LS61       s sharp small                  */
'O',     /* P0B+0xe2,    LO16       o circumflex capital           */
'O',     /* P0B+0xe3,    LO14       o grave capital                */
'o',     /* P0B+0xe4,    LO19       o tilde small                  */
'O',     /* P0B+0xe5,    LO20       o tilde capital                */
'm',     /* P0B+0xe6,    GM01/SM17  Mu small (Micro)               */
'I',     /* P0B+0xe7,    LT63       thorn icelandic small          */
'I',     /* P0B+0xe8,    LT64       thorn icelandic capital        */
'U',     /* P0B+0xe9,    LU12       u acute capital                */
'U',     /* P0B+0xea,    LU16       u circumflex capital           */
'U',     /* P0B+0xeb,    LU14       u grave capital                */
'y',     /* P0B+0xec,    LY11       y acute small                  */
'Y',     /* P0B+0xed,    LY12       y acute capital                */
'?',     /* P0B+0xee,    SM15       overbar                        */
'\'',    /* P0B+0xef,    SD11       acute accent                   */
'-',     /* P0B+0xf0,    SP32       syllable hyphen (SP10)         */
'?',     /* P0B+0xf1,    SA02       Plus or minus                  */
'_',     /* P0B+0xf2,    SM10       double underscore              */
'?',     /* P0B+0xf3,    NF05       three quarters                 */
'?',     /* P0B+0xf4,    SM25       Paragraph                      */
'?',     /* P0B+0xf5,    SM24       Section                        */
'%',     /* P0B+0xf6,    SA06       Divide                         */
',',     /* P0B+0xf7,    SD41       cedilla accent                 */
'o',     /* P0B+0xf8,    SM19       Degree (Overcircle)            */
'\'',    /* P0B+0xf9,    SD17       umlaut accent                  */
'.',     /* P0B+0xfa,    SM26       Middle dot (Product dot) (aka SD63)            */
'1',     /* P0B+0xfb,    NS01       Superscript one                */
'3',     /* P0B+0xfc,    ND031      superscript three              */
'2',     /* P0B+0xfd,    NS02       Superscript two (aka ND021)    */
'#',     /* P0B+0xfe,    SV18       Vertical solid rectangle (aka SM47)            */
'#',     /* P0B+0xff,               All ones                       */
'?',     /* P1A+0x00,                                              */
'?',     /* P1A+0x01,                                              */
'?',     /* P1A+0x02,                                              */
'?',     /* P1A+0x03,                                              */
'?',     /* P1A+0x04,                                              */
'?',     /* P1A+0x05,                                              */
'?',     /* P1A+0x06,                                              */
'?',     /* P1A+0x07,                                              */
'?',     /* P1A+0x08,                                              */
'?',     /* P1A+0x09,                                              */
'?',     /* P1A+0x0a,                                              */
'?',     /* P1A+0x0b,                                              */
'?',     /* P1A+0x0c,                                              */
'?',     /* P1A+0x0d,                                              */
'?',     /* P1A+0x0e,                                              */
'?',     /* P1A+0x0f,                                              */
'?',     /* P1A+0x10,                                              */
'?',     /* P1A+0x11,                                              */
'?',     /* P1A+0x12,                                              */
'?',     /* P1A+0x13,                                              */
'?',     /* P1A+0x14,                                              */
'?',     /* P1A+0x15,                                              */
'?',     /* P1A+0x16,                                              */
'?',     /* P1A+0x17,                                              */
'?',     /* P1A+0x18,                                              */
'?',     /* P1A+0x19,                                              */
'?',     /* P1A+0x1a,                                              */
'?',     /* P1A+0x1b,                                              */
'?',     /* P1A+0x1c,                                              */
'?',     /* P1A+0x1d,                                              */
'?',     /* P1A+0x1e,                                              */
'?',     /* P1A+0x1f,                                              */
'.',     /* P1A+0x20,    SD63       Spanish middle dot             */
'?',     /* P1A+0x21,    SS00       Smiling Face                   */
'?',     /* P1A+0x22,    SS01       Dark Smiling Face              */
'H',     /* P1A+0x23,    SS02       Heart                          */
'D',     /* P1A+0x24,    SM61       Diamond (aka SS03)             */
'C',     /* P1A+0x25,    SS04       Club                           */
'S',     /* P1A+0x26,    SS05       Spade                          */
'@',     /* P1A+0x27,    SM58       Bullet (aka SM57)              */
'@',     /* P1A+0x28,    SM62       Reverse Video Bullet (aka SM570001)            */
'O',     /* P1A+0x29,    SM75       Circle                         */
'O',     /* P1A+0x2a,    SM72       Reverse Video Circle (aka SM750002)            */
'?',     /* P1A+0x2b,    SM28       Male Symbol                    */
'?',     /* P1A+0x2c,    SM29       Female Symbol                  */
'?',     /* P1A+0x2d,    SM93       Eighth Note                    */
'?',     /* P1A+0x2e,    SM91       Sixteenth Note                 */
'*',     /* P1A+0x2f,    SM69       Sun                            */
'>',     /* P1A+0x30,    SM59       Right Solid Triangle           */
'<',     /* P1A+0x31,    SM63       Left Solid Triangle            */
'^',     /* P1A+0x32,    SM36       Bidirectional Vertical Arrow (aka SM76)        */
'!',     /* P1A+0x33,    SP33       Double Exclamation Point       */
'?',     /* P1A+0x34,    SM25       Paragraph                      */
'?',     /* P1A+0x35,    SM24       Section                        */
'#',     /* P1A+0x36,    SM70       Horizontal Solid Rectangle     */
'-',     /* P1A+0x37,    SM37       Underlined Bidirect. Vert. Arrow (a. SM77)     */
'^',     /* P1A+0x38,    SM32       Upward Arrow                   */
'v',     /* P1A+0x39,    SM33       Downward Arrow                 */
'>',     /* P1A+0x3a,    SM31       Right Arrow                    */
'<',     /* P1A+0x3b,    SM30       Left Arrow                     */
'L',     /* P1A+0x3c,    SA42       Diagonally Flipped Logical Not (new)           */
'-',     /* P1A+0x3d,    SM38       Bidirectional Horizontal Arrow (aka SM78)      */
'^',     /* P1A+0x3e,    SM60       Solid Upward Triangle          */
'v',     /* P1A+0x3f,    SV04       Solid Downward Triangle        */
'a',     /* P1A+0x40,    LA19       a tilde small                  */
'B',     /* P1A+0x41,    LS61       s sharp small                  */
'A',     /* P1A+0x42,    LA16       a circumflex capital           */
'A',     /* P1A+0x43,    LA14       a grave capital                */
'A',     /* P1A+0x44,    LA12       a acute capital                */
'A',     /* P1A+0x45,    LA20       a tilde capital                */
'o',     /* P1A+0x46,    LO61       o slash small                  */
'E',     /* P1A+0x47,    LE16       e circumflex capital           */
'E',     /* P1A+0x48,    LE18       e umlaut capital               */
'E',     /* P1A+0x49,    LE14       e grave capital                */
'I',     /* P1A+0x4a,    LI12       i acute capital                */
'I',     /* P1A+0x4b,    LI16       i circumflex capital           */
'I',     /* P1A+0x4c,    LI18       i umlaut capital               */
'I',     /* P1A+0x4d,    LI14       i grave capital                */
'O',     /* P1A+0x4e,    LO62       slashed o capital              */
'd',     /* P1A+0x4f,    LD63       eth icelandic small            */
'y',     /* P1A+0x50,    LY11       y acute small                  */
'I',     /* P1A+0x51,    LT63       thorn icelandic small          */
',',     /* P1A+0x52,    SD41       cedilla accent                 */
'*',     /* P1A+0x53,    SC01       international currency symbol  */
'D',     /* P1A+0x54,    LD62       eth icelandic capital          */
'Y',     /* P1A+0x55,    LY12       y acute capital                */
'I',     /* P1A+0x56,    LT64       thorn icelandic capital        */
'r',     /* P1A+0x57,    SM53       registered trademark symbol    */
'?',     /* P1A+0x58,    NF05       three quarters                 */
'?',     /* P1A+0x59,    SM15       overbar                        */
'\'',    /* P1A+0x5a,    SD17       umlaut accent                  */
'\'',    /* P1A+0x5b,    SD11       acute accent                   */
'_',     /* P1A+0x5c,    SM10       double underscore              */
'o',     /* P1A+0x5d,    LO19       o tilde small                  */
'i',     /* P1A+0x5e,    LI61       small i dotless                */
'O',     /* P1A+0x5f,    LO16       o circumflex capital           */
'O',     /* P1A+0x60,    LO14       o grave capital                */
'O',     /* P1A+0x61,    LO12       o acute capital                */
'O',     /* P1A+0x62,    LO20       o tilde capital                */
'3',     /* P1A+0x63,    ND031      superscript three              */
'U',     /* P1A+0x64,    LU16       u circumflex capital           */
'U',     /* P1A+0x65,    LU14       u grave capital                */
'U',     /* P1A+0x66,    LU12       u acute capital                */
'a',     /* P1A+0x67,    LA43       a ogonek small                 */
'e',     /* P1A+0x68,    LE21       e caron small                  */
'c',     /* P1A+0x69,    LC21       c caron small                  */
'c',     /* P1A+0x6a,    LC11       c acute small                  */
'e',     /* P1A+0x6b,    LE43       e ogonek small                 */
'u',     /* P1A+0x6c,    LU27       u overcircle small             */
'd',     /* P1A+0x6d,    LD21       d caron small                  */
'l',     /* P1A+0x6e,    LL11       l acute small                  */
'A',     /* P1A+0x6f,    LA44       a ogonek capital               */
'E',     /* P1A+0x70,    LE22       e caron capital                */
'C',     /* P1A+0x71,    LC22       c caron capital                */
'C',     /* P1A+0x72,    LC12       c acute capital                */
'v',     /* P1A+0x73,    SD21       caron accent                   */
'E',     /* P1A+0x74,    LE44       e ogonek capital               */
'U',     /* P1A+0x75,    LU28       u overcircle capital           */
'D',     /* P1A+0x76,    LD22       d caron capital                */
'L',     /* P1A+0x77,    LL12       l acute capital                */
'l',     /* P1A+0x78,    LL21       l caron small                  */
'n',     /* P1A+0x79,    LN21       n caron small                  */
'd',     /* P1A+0x7a,    LD61       d stroke small                 */
'r',     /* P1A+0x7b,    LR21       r caron small                  */
's',     /* P1A+0x7c,    LS11       s acute small                  */
'o',     /* P1A+0x7d,    SD27       overcircle accent              */
'l',     /* P1A+0x7e,    LL61       l stroke small                 */
'n',     /* P1A+0x7f,    LN11       n acute small                  */
's',     /* P1B+0x80,    LS21       s caron small                  */
'L',     /* P1B+0x81,    LL22       l caron capital                */
'N',     /* P1B+0x82,    LN22       n caron capital                */
'R',     /* P1B+0x83,    LR22       r caron capital                */
'S',     /* P1B+0x84,    LS12       s acute capital                */
'.',     /* P1B+0x85,    SD29       overdot accent                 */
'z',     /* P1B+0x86,    LZ29       z overdot small                */
'<',     /* P1B+0x87,    SD43       ogonek accent                  */
'Z',     /* P1B+0x88,    LZ30       z overdot capital              */
'z',     /* P1B+0x89,    LZ21       z caron small                  */
'z',     /* P1B+0x8a,    LZ11       z acute small                  */
'Z',     /* P1B+0x8b,    LZ22       z caron capital                */
'Z',     /* P1B+0x8c,    LZ12       z acute capital                */
'L',     /* P1B+0x8d,    LL62       l stroke capital               */
'N',     /* P1B+0x8e,    LN12       n acute capital                */
'S',     /* P1B+0x8f,    LS22       s caron capital                */
't',     /* P1B+0x90,    LT21       t caron small                  */
'r',     /* P1B+0x91,    LR11       r acute small                  */
'o',     /* P1B+0x92,    LO25       o double acute small           */
'u',     /* P1B+0x93,    LU25       u double acute small           */
'T',     /* P1B+0x94,    LT22       t caron capital                */
'R',     /* P1B+0x95,    LR12       r acute capital                */
'O',     /* P1B+0x96,    LO26       o double acute capital         */
'U',     /* P1B+0x97,    LU26       u double acute capital         */
'a',     /* P1B+0x98,    LA23       a breve small                  */
'g',     /* P1B+0x99,    LG23       g breve small                  */
'I',     /* P1B+0x9a,    LI30       i overdot capital              */
'A',     /* P1B+0x9b,    LA24       a breve capital                */
'G',     /* P1B+0x9c,    LG24       g breve capital                */
'u',     /* P1B+0x9d,    SD23       breve accent                   */
'=',     /* P1B+0x9e,    SD25       double acute accent            */
's',     /* P1B+0x9f,    LS41       s cedilla small                */
'l',     /* P1B+0xa0,    SM16       liter symbol                   */
'n',     /* P1B+0xa1,    LN63       high comma n small             */
'S',     /* P1B+0xa2,    LS42       s cedilla capital              */
'-',     /* P1B+0xa3,    SD31       macron accent                  */
't',     /* P1B+0xa4,    LT41       t cedilla small                */
'T',     /* P1B+0xa5,    LT42       t cedilla capital              */
'a',     /* P1B+0xa6,    LA31       a macron small                 */
'A',     /* P1B+0xa7,    LA32       a macron capital               */
'c',     /* P1B+0xa8,    LC15       c circumflex small             */
'C',     /* P1B+0xa9,    LC16       c circumflex capital           */
'?',     /* P1B+0xaa,    SM94       High reverse solidus           */
'c',     /* P1B+0xab,    LC29       c overdot small                */
'C',     /* P1B+0xac,    LC30       c overdot capital              */
'e',     /* P1B+0xad,    LE29       e overdot small                */
'E',     /* P1B+0xae,    LE30       e overdot capital              */
'e',     /* P1B+0xaf,    LE31       e macron small                 */
'E',     /* P1B+0xb0,    LE32       e macron capital               */
'g',     /* P1B+0xb1,    LG11       g acute small                  */
'g',     /* P1B+0xb2,    LG15       g circumflex small             */
'G',     /* P1B+0xb3,    LG16       g circumflex capital           */
'g',     /* P1B+0xb4,    LG29       g overdot small                */
'G',     /* P1B+0xb5,    LG30       g overdot capital              */
'G',     /* P1B+0xb6,    LG42       g cedilla capital              */
'h',     /* P1B+0xb7,    LH15       h circumflex small             */
'H',     /* P1B+0xb8,    LH16       h circumflex capital           */
'h',     /* P1B+0xb9,    LH61       h stroke small                 */
'H',     /* P1B+0xba,    LH62       h stroke capital               */
'i',     /* P1B+0xbb,    LI19       i tilde small                  */
'I',     /* P1B+0xbc,    LI20       i tilde capital                */
'i',     /* P1B+0xbd,    LI31       i macron small                 */
'I',     /* P1B+0xbe,    LI32       i macron capital               */
'i',     /* P1B+0xbf,    LI43       i ogonek small                 */
'I',     /* P1B+0xc0,    LI44       i ogonek capital               */
'i',     /* P1B+0xc1,    LI51       ij ligature small              */
'I',     /* P1B+0xc2,    LI52       ij ligature capital            */
'j',     /* P1B+0xc3,    LJ15       j circumflex small             */
'J',     /* P1B+0xc4,    LJ16       j circumflex capital           */
'k',     /* P1B+0xc5,    LK41       k cedilla small                */
'K',     /* P1B+0xc6,    LK42       k cedilla capital              */
'k',     /* P1B+0xc7,    LK61       k greenlandic small            */
'l',     /* P1B+0xc8,    LL41       l cedilla small                */
'L',     /* P1B+0xc9,    LL42       l cedilla capital              */
'l',     /* P1B+0xca,    LL63       l middle dot small             */
'L',     /* P1B+0xcb,    LL64       l middle dot capital           */
'n',     /* P1B+0xcc,    LN41       n cedilla small                */
'N',     /* P1B+0xcd,    LN42       n cedilla capital              */
'n',     /* P1B+0xce,    LN61       n eng lapp small               */
'N',     /* P1B+0xcf,    LN62       n eng lapp capital             */
'o',     /* P1B+0xd0,    LO31       o macron small                 */
'O',     /* P1B+0xd1,    LO32       o macron capital               */
'o',     /* P1B+0xd2,    LO51       oe ligature small              */
'O',     /* P1B+0xd3,    LO52       oe ligature capital            */
'r',     /* P1B+0xd4,    LR41       r cedilla small                */
'R',     /* P1B+0xd5,    LR42       r cedilla capital              */
's',     /* P1B+0xd6,    LS15       s circumflex small             */
'S',     /* P1B+0xd7,    LS16       s circumflex capital           */
't',     /* P1B+0xd8,    LT61       t stroke small                 */
'T',     /* P1B+0xd9,    LT62       t stroke capital               */
'u',     /* P1B+0xda,    LU19       u tilde small                  */
'U',     /* P1B+0xdb,    LU20       u tilde capital                */
'u',     /* P1B+0xdc,    LU23       u breve small                  */
'U',     /* P1B+0xdd,    LU24       u breve capital                */
'u',     /* P1B+0xde,    LU31       u macron small                 */
'U',     /* P1B+0xdf,    LU32       u macron capital               */
'u',     /* P1B+0xe0,    LU43       u ogonek small                 */
'U',     /* P1B+0xe1,    LU44       u ogonek capital               */
'w',     /* P1B+0xe2,    LW15       w circumflex small             */
'W',     /* P1B+0xe3,    LW16       w circumflex capital           */
'y',     /* P1B+0xe4,    LY15       y circumflex small             */
'Y',     /* P1B+0xe5,    LY16       y circumflex capital           */
'Y',     /* P1B+0xe6,    LY18       y umlaut capital               */
'c',     /* P1B+0xe7,    SM52       Copyright symbol               */
'1',     /* P1B+0xe8,    NS01       Superscript one                */
'?',     /* P1B+0xe9,    SM54       Trademark symbol               */
'?',     /* P1B+0xea,    NF18       One eighth                     */
'?',     /* P1B+0xeb,    NF19       Three eighths                  */
'?',     /* P1B+0xec,    NF20       Five eighths                   */
'?',     /* P1B+0xed,    NF21       Seven eighths                  */
'x',     /* P1B+0xee,    SA07       Multiply sign                  */
'`',     /* P1B+0xef,    SP20       Right single quote             */
'\'',    /* P1B+0xf0,    SP21       Left double quote              */
'\'',    /* P1B+0xf1,    SP22       Right double quote             */
'=',     /* P1B+0xf2,    SA04-04    equal sign superscript         */
'-',     /* P1B+0xf3,    SP10-04    minus sign superscript         */
'+',     /* P1B+0xf4,    SA01-04    plus sign superscript          */
'8',     /* P1B+0xf5,    SA45-04    infinity symbol superscript    */
'?',     /* P1B+0xf6,    GP01-04    pi superscript                 */
'?',     /* P1B+0xf7,    SM73-04    delta symbol superscript       */
'>',     /* P1B+0xf8,    SM31-04    right arrow superscript        */
'/',     /* P1B+0xf9,    SP12-04    slash superscript              */
'+',     /* P1B+0xfa,    SM34       dagger                         */
'<',     /* P1B+0xfb,    SD18-04    left angle superscript         */
'>',     /* P1B+0xfc,    SD19-04    right angle superscript        */
'?',     /* P1B+0xfd,    SM55       prescription symbol            */
'?',     /* P1B+0xfe,    SA36       'is not an element',symbol     */
'?',     /* P1B+0xff,    SA37       'therefore',symbol             */
'?',     /* P2A+0x00,                                              */
'?',     /* P2A+0x01,                                              */
'?',     /* P2A+0x02,                                              */
'?',     /* P2A+0x03,                                              */
'?',     /* P2A+0x04,                                              */
'?',     /* P2A+0x05,                                              */
'?',     /* P2A+0x06,                                              */
'?',     /* P2A+0x07,                                              */
'?',     /* P2A+0x08,                                              */
'?',     /* P2A+0x09,                                              */
'?',     /* P2A+0x0a,                                              */
'?',     /* P2A+0x0b,                                              */
'?',     /* P2A+0x0c,                                              */
'?',     /* P2A+0x0d,                                              */
'?',     /* P2A+0x0e,                                              */
'?',     /* P2A+0x0f,                                              */
'?',     /* P2A+0x10,                                              */
'?',     /* P2A+0x11,                                              */
'?',     /* P2A+0x12,                                              */
'?',     /* P2A+0x13,                                              */
'?',     /* P2A+0x14,                                              */
'?',     /* P2A+0x15,                                              */
'?',     /* P2A+0x16,                                              */
'?',     /* P2A+0x17,                                              */
'?',     /* P2A+0x18,                                              */
'?',     /* P2A+0x19,                                              */
'?',     /* P2A+0x1a,                                              */
'?',     /* P2A+0x1b,                                              */
'?',     /* P2A+0x1c,                                              */
'?',     /* P2A+0x1d,                                              */
'?',     /* P2A+0x1e,                                              */
'?',     /* P2A+0x1f,                                              */
'>',     /* P2A+0x20,    SM95       increase                       */
'<',     /* P2A+0x21,    SM99       decrease                       */
'+',     /* P2A+0x22,    SM35       double dagger                  */
'#',     /* P2A+0x23,    SA54       not equal sign                 */
'v',     /* P2A+0x24,    SA32       or                             */
'^',     /* P2A+0x25,    SA33       and                            */
'|',     /* P2A+0x26,    SA34       parallel                       */
'?',     /* P2A+0x27,    SA35       angle symbol                   */
'<',     /* P2A+0x28,    SA18       left angle bracket             */
'>',     /* P2A+0x29,    SA19       right angle bracket            */
'?',     /* P2A+0x2a,    SA17       minus or plus sign             */
'%',     /* P2A+0x2b,    SM49       lozenge                        */
'\'',    /* P2A+0x2c,    SM50       minutes symbol                 */
'?',     /* P2A+0x2d,    SA51       integral symbol                */
'u',     /* P2A+0x2e,    SA39       union                          */
'?',     /* P2A+0x2f,    SA40       'is included in',symbol        */
'?',     /* P2A+0x30,    SA41       'includes',symbol              */
'+',     /* P2A+0x31,    SA55       circle plus/closed sum         */
'L',     /* P2A+0x32,    SA42       right angle symbol             */
'x',     /* P2A+0x33,    SA56       circle multiply                */
'\'',    /* P2A+0x34,    SM51       seconds symbol                 */
'=',     /* P2A+0x35,    SM64       double overline                */
'y',     /* P2A+0x36,    GP61       psi small                      */
'e',     /* P2A+0x37,    GE01       epsilon small                  */
'l',     /* P2A+0x38,    GL01       lambda small                   */
'h',     /* P2A+0x39,    GE31       eta small                      */
'i',     /* P2A+0x3a,    GI01       iota small                     */
'(',     /* P2A+0x3b,    SS20       upper left parenthesis sect.   */
'(',     /* P2A+0x3c,    SS21       lower left parenthesis sect.   */
'%',     /* P2A+0x3d,    SM56       permille symbol                */
'&',     /* P2A+0x3e,    GT63       theta small                    */
'k',     /* P2A+0x3f,    GK01       kappa small                    */
'w',     /* P2A+0x40,    GO31       omega small                    */
'n',     /* P2A+0x41,    GN01       nu small                       */
'o',     /* P2A+0x42,    GO01       omicron small                  */
'r',     /* P2A+0x43,    GR01       rho small                      */
'g',     /* P2A+0x44,    GG01       gamma small                    */
'q',     /* P2A+0x45,    GT61       theta small                    */
')',     /* P2A+0x46,    SS22       upper right parenthesis sect.  */
')',     /* P2A+0x47,    SS23       lower right parenthesis sect.  */
'~',     /* P2A+0x48,    SA43       'congruent to',symbol          */
'x',     /* P2A+0x49,    GX01       xi small                       */
'c',     /* P2A+0x4a,    GH01       chi small                      */
'u',     /* P2A+0x4b,    GU01       upsilon small                  */
'z',     /* P2A+0x4c,    GZ01       zeta small                     */
'|',     /* P2A+0x4d,    SS25       lower right/upper left brace section           */
'|',     /* P2A+0x4e,    SS24       upper right/lower left brace section           */
'0',     /* P2A+0x4f,    NS10       zero subscript                 */
'1',     /* P2A+0x50,    NS11       one subscript                  */
'2',     /* P2A+0x51,    NS12       two subscript                  */
'3',     /* P2A+0x52,    NS13       three subscript                */
'4',     /* P2A+0x53,    NS14       four subscript                 */
'5',     /* P2A+0x54,    NS15       five subscript                 */
'6',     /* P2A+0x55,    NS16       six subscript                  */
'7',     /* P2A+0x56,    NS17       seven subscript                */
'8',     /* P2A+0x57,    NS18       eight subscript                */
'9',     /* P2A+0x58,    NS19       nine subscript                 */
'|',     /* P2A+0x59,    SM90/SA78  perpendicular                  */
'?',     /* P2A+0x5a,    SS62       total symbol                   */
'Y',     /* P2A+0x5b,    GP62       psi capital                    */
'P',     /* P2A+0x5c,    GP02       pi capital                     */
'L',     /* P2A+0x5d,    GL02       lambda capital                 */
'?',     /* P2A+0x5e,    SS63       bottle symbol                  */
'b',     /* P2A+0x5f,    SM67       substitute blank               */
'?',     /* P2A+0x60,    SA49       partial differential symbol    */
'~',     /* P2A+0x61,    SA30       sine symbol                    */
'?',     /* P2A+0x62,    SM45       open square                    */
'?',     /* P2A+0x63,    SM47       solid square                   */
'?',     /* P2A+0x64,    SM48       slash square                   */
'?',     /* P2A+0x65,    SS28       upper summation section        */
'?',     /* P2A+0x66,    SS29       lower summation section        */
'X',     /* P2A+0x67,    GX02       xi capital                     */
'?',     /* P2A+0x68,    SA47       'proportional to',symbol       */
'D',     /* P2A+0x69,    GD02       delta capital                  */
'U',     /* P2A+0x6a,    GU02       upsilon capital                */
'~',     /* P2A+0x6b,    SA44       'approximately equal to',symb. */
'~',     /* P2A+0x6c,    SA16       cycle/'equivalent to',symbol   */
'0',     /* P2A+0x6d,    NS00       zero superscript               */
'4',     /* P2A+0x6e,    NS04       four superscript               */
'5',     /* P2A+0x6f,    NS05       five superscript               */
'6',     /* P2A+0x70,    NS06       six superscript                */
'7',     /* P2A+0x71,    NS07       seven superscript              */
'8',     /* P2A+0x72,    NS08       eight superscript              */
'9',     /* P2A+0x73,    NS09       nine superscript               */
'0',     /* P2A+0x74,    SS45       zero slash                     */
'P',     /* P2A+0x75,    SC06       Peseta sign                    */
'~',     /* P2A+0x76,    SM68       Flipped logical not            */
'|',     /* P2A+0x77,    SF19       Right side middle - double horizontal          */
'|',     /* P2A+0x78,    SF20       Right side middle - double vertical            */
'+',     /* P2A+0x79,    SF21       Upper right corner box - double vertical       */
'+',     /* P2A+0x7a,    SF22       Upper right corner box - double horizontal     */
'+',     /* P2A+0x7b,    SF27       Lower right corner box - double vertical       */
'+',     /* P2A+0x7c,    SF28       Lower right corner box - double horizontal     */
'|',     /* P2A+0x7d,    SF36       Left side middle - double horizontal           */
'|',     /* P2A+0x7e,    SF37       Left side middle - double vertical             */
'-',     /* P2A+0x7f,    SF45       Bottom side middle - double horizontal         */
'-',     /* P2B+0x80,    SF46       Bottom side middle - double vertical           */
'-',     /* P2B+0x81,    SF47       Top side middle - double horizontal            */
'+',     /* P2B+0x82,    SF49       Lower left corner box - double vertical        */
'+',     /* P2B+0x83,    SF50       Lower left corner box - double horizontal      */
'+',     /* P2B+0x84,    SF51       Upper left corner box - double horizontal      */
'+',     /* P2B+0x85,    SF52       Upper left corner box - double vertical        */
'+',     /* P2B+0x86,    SF53       Intersection - double vertical                 */
'+',     /* P2B+0x87,    SF54       Intersection - double horizontal               */
'#',     /* P2B+0x88,    SF58       bright cell - left half        */
'#',     /* P2B+0x89,    SF59       bright cell - right half       */
'a',     /* P2B+0x8a,    GA01       Alpha small                    */
'b',     /* P2B+0x8b,    GB01       Beta small                     */
'G',     /* P2B+0x8c,    GG02       Gamma capital                  */
'?',     /* P2B+0x8d,    GP01       Pi small                       */
'S',     /* P2B+0x8e,    GS02/SS40  Sigma capital/Summation symbol */
's',     /* P2B+0x8f,    GS01       Sigma small                    */
't',     /* P2B+0x90,    GT01       Tau small                      */
'F',     /* P2B+0x91,    GF02       Phi capital                    */
'Q',     /* P2B+0x92,    GT62       Theta capital                  */
'O',     /* P2B+0x93,    GO32/SM18  Omega (Ohm)                    */
'd',     /* P2B+0x94,    GD01       Delta small                    */
'8',     /* P2B+0x95,    SA45       Infinity                       */
'f',     /* P2B+0x96,    GF01       Phi small                      */
'E',     /* P2B+0x97,    SA67       Is an element of               */
'n',     /* P2B+0x98,    SA38       Intersection                   */
'=',     /* P2B+0x99,    SA48       Identity symbol                */
'?',     /* P2B+0x9a,    SA53       Greater than or equal to       */
'?',     /* P2B+0x9b,    SA52       Less than or equal to          */
'?',     /* P2B+0x9c,    SS26       Upper integral section         */
'?',     /* P2B+0x9d,    SS27       Lower integral section         */
'=',     /* P2B+0x9e,    SA70       Double equivalent              */
'o',     /* P2B+0x9f,    SA79       Solid overcircle               */
'?',     /* P2B+0xa0,    SM23       Radical symbol (square root)   */
'-',     /* P2B+0xa1,    SF48       Top side middle - double vertical              */
'n',     /* P2B+0xa2,    LN011      Superscript n                  */
'?',     /* P2B+0xa3,    SP31       Numeric Space                  */
'-',     /* P2B+0xa4,    SV08       Center Line                    */
'?',     /* P2B+0xa5,    SV38       Counter Bore                   */
'u',     /* P2B+0xa6,    SV39       Counter Sink                   */
'v',     /* P2B+0xa7,    SV40       Depth                          */
'o',     /* P2B+0xa8,    SV41       Diameter                       */
};

int _NLflattsize = sizeof (_NLflattab);
