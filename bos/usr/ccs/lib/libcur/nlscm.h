/* @(#)35       1.1  src/bos/usr/ccs/lib/libcur/nlscm.h, libcur, bos411, 9428A410j 12/14/89 17:30:55 */
#ifndef _H_NLSCM
#define _H_NLSCM

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: nlscm.h
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                nlscm.h
 *
 * FUNCTION:    ecflin character-mask tables for NLS
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Bit mask tables for ecflin input field patterns - each mask may
 *	contain 128 bytes = 1024 bits.  (Trailing zeros may be omitted.)
 *	Bits are assigned from high order to low order--e.g., 0x80
 *	in first byte corresponds to NUL.  The following code assignments
 *	(hex) are used:
 *		00-7f	standard ASCII
 *		80-ff	PC graphics
 *		100-17f	keypad special keys (cursor, etc) + reserved
 *		180-1ff	function keys + reserved
 *		200-2ff	P1 characters
 *		300-3ff P2 characters
 */

/* D - default mask, all characters and field input controls            */

static
unsigned char  cm_D[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  060-067    068-06F    070-077    078-07F                          */
/*  `abcdefg   hijklmno   pqrstuvw   xyz{|}~   7F is del              */
    0xFF,      0xFF,      0xFF,      0xFE,

/*  080-087    088-08F    090-097    098-09F                          */
/*                                             PC graphics            */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  0A0-0A7    0A8-0AF    0B0-0B7    0B8-0BF                          */
/*                                             PC graphics            */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  0C0-0C7    0C8-0CF    0D0-0D7    0D8-0DF                          */
/*                                             PC graphics            */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  0E0-0E7    0E8-0EF    0F0-0F7    0F8-0FF                          */
/*                                             PC graphics            */
    0xFF,      0xFF,      0xFF,      0xFF,     /* end of P0           */

/*     KEY_BREAK      0x101              break - unreliable           */
/*     KEY_DOWN       0x102             +cursor down                  */
/*     KEY_UP         0x103             +cursor up                    */
/*     KEY_LEFT       0x104             +cursor left                  */
/*     KEY_RIGHT      0x105             +cursor right                 */
/*     KEY_HOME       0x106              home - top left              */
/*     KEY_BACKSPACE  0x107             +backspace - unreliable       */
/*  100-107    -------    -------    -------                          */
/*                                                                    */
    0x3D,

/*     KEY_DL         0x108              delete line                  */
/*     KEY_IL         0x109              insert line                  */
/*     KEY_DC         0x10a             +delete character             */
/*     KEY_IC         0x10b             +insert character mode start  */
/*     KEY_EIC        0x10c             +exit insert character mode   */
/*     KEY_CLEAR      0x10d              clear screen                 */
/*     KEY_EOS        0x10e             +clear to end of screen       */
/*     KEY_EOL        0x10f              clear to end of line         */
/*  -------    108-10F    -------    -------                          */
/*                                                                    */
	       0x3A,

/*     KEY_SF         0x110             +scroll forward toward end    */
/*     KEY_SR         0x111             +scroll backward toward start */
/*     KEY_NPAGE      0x112              next page                    */
/*     KEY_PPAGE      0x113              previous page                */
/*     KEY_STAB       0x114              set tab stop                 */
/*     KEY_CTAB       0x115              clear tab stop               */
/*     KEY_CATAB      0x116              clear all tab stops          */
/*     KEY_ENTER      0x117              enter key - unreliable       */
/*  -------    -------    110-117    -------                          */
/*                                                                    */
			  0xc0,

/*     KEY_SRESET     0x118              soft reset key - unreliable  */
/*     KEY_RESET      0x119              hard reset key - unreliable  */
/*     KEY_PRINT      0x11a              print or copy                */
/*     KEY_LL         0x11b              lower left (last line)       */
/*     KEY_A1         0x11c              pad upper left               */
/*     KEY_A3         0x11d              pad upper right              */
/*     KEY_B2         0x11e              pad center                   */
/*     KEY_C1         0x11f              pad lower left               */
/*  -------    -------    -------    118-11F                          */
/*                                                                    */
				     0x00,

/*     KEY_C3         0x120              pad lower right              */
/*     KEY_DO         0x121              DO key                       */
/*     KEY_QUIT       0x122              QUIT key                     */
/*     KEY_CMD        0x123              Command key                  */
/*     KEY_PCMD       0x124              Previous command key         */
/*     KEY_NPN        0x125              Next pane key                */
/*     KEY_PPN        0x126              previous pane key            */
/*     KEY_CPN        0x127              command pane key             */
/*  120-127    -------    -------    -------                          */
/*                                                                    */
    0x00,

/*     KEY_END        0x128              end key                      */
/*     KEY_HLP        0x129              help key                     */
/*     KEY_SEL        0x12a              select key                   */
/*     KEY_SCR        0x12b             +scroll right key             */
/*     KEY_SCL        0x12c             +scroll left key              */
/*     KEY_TAB        0x12d              tab key                      */
/*     KEY_BTAB       0x12e             +back tab key                 */
/*     KEY_NEWL       0x12f             +new line key                 */
/*  -------    128-12F    130-137    138-13F                          */
/*                                             130-13f reserved       */
	       0x1b,      0x00,      0x00,

/*  140-147    148-14F    150-157    158-15F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  160-167    168-16F    170-177    178-17F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  180-187    188-18F    190-197    198-19F                          */
/*                                             function keys          */
    0x00,      0x00,      0x00,      0x00,

/*  1A0-1A7    1A8-1AF    1B0-1B7    1B8-1BF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1C0-1C7    1C8-1CF    1D0-1D7    1D8-1DF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1E0-1E7    1E8-1EF    1F0-1F7    1F8-1FF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  200-207    208-20F    210-217    218-21F    0x1f80 - 0x1f9f       */
/*                                              single byte cntrls    */
    0x00,      0x00,      0x00,      0x00,

/*  220-227    228-22F    230-237    238-23F    0x1fa0 - 0x1fbf       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  240-247    248-24F    250-257    258-25F    0x1fc0 - 0x1fdf       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  260-267    267-26F    270-277    278-27F    0x1fe0 - 0x1fff       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  280-287    288-28F    290-297    298-29F    0x1e80 - 0x1e9f       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  2A0-2A7    2A8-2AF    2B0-2B7    2B8-2BF    0x1ea0 - 0x1ebf       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  2C0-2C7    2C8-2CF    2D0-2D7    2D8-2DF    0x1ec0 - 1edf         */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  2E0-2E7    2E8-2EF    2F0-2F7    2F8-2FF    0x1ee0 - 0x1eff       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  300-307    308-30F    310-317    318-31F    0x1d80 - 0x1d9f       */
/*                                              single byte ctrls     */
    0x00,      0x00,      0x00,      0x00,

/*  320-327    328-32F    330-337    338-33F    0x1da0 - 0x1dbf       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  340-347    348-34F    350-357    358-35F    0x1dc0 - 0x1ddf       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  360-367    368-36F    370-377    378-37F    0x1de0 - 0x1dff       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  380-387    388-38F    390-397    398-39F    0x1c80 - 0x1c9f       */
/*                                                                    */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  3A0-3A7    3A8-3AF    3B0-3B8    3B9-3BF    0x1ca0 - 0x1cbf       */
/*                                              up to 0x1ca8; rest of */
    0xFF,      0x80,      0x00,      0x00,      /* P0 is reserved     */

/*  3C0-3C7    3C8-3CF    3D0-3D8    3D9-3DF    0x1cc0 - 0x1cdf       */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  3E0-3E7    3E8-3EF    3F0-3F8    3F9-3FF    0x1ce0 - 0x1cff       */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00

		     }  ;


/* U - Upper case alphabetic characters only                          */

static
unsigned char  cm_U[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x00,      0x00,      0x00,      0x00,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0x7F,      0xFF,      0xFF,      0xE0,

/*  060-067    068-06F    070-077    078-07F                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*              0x80            c cedilla capital                     */
/*              0x81                                                  */
/*              0x82                                                  */
/*              0x83                                                  */
/*              0x84                                                  */
/*              0x85                                                  */
/*              0x86                                                  */
/*              0x87                                                  */
/*  080-087    --------   --------   --------                         */
/*                                                                    */
    0x80,

/*              0x88                                                  */
/*              0x89                                                  */
/*              0x8a                                                  */
/*              0x8b                                                  */
/*              0x8c                                                  */
/*              0x8d                                                  */
/*              0x8e            a umlaut capital                      */
/*              0x8f            a overcircle capital                  */
/*  --------   088-08F    --------   --------                         */
/*                                                                    */
	       0x03,

/*              0x90            e acute capital                       */
/*              0x91                                                  */
/*              0x92            ae dipthong capital                   */
/*              0x93                                                  */
/*              0x94                                                  */
/*              0x95                                                  */
/*              0x96                                                  */
/*              0x97                                                  */
/*  --------   --------   090-097    --------                         */
/*                                                                    */
			  0xa0,

/*              0x98                                                  */
/*              0x99            o umlaut capital                      */
/*              0x9a            u umlaut capital                      */
/*              0x9b                                                  */
/*              0x9c                                                  */
/*              0x9d            o slash capital                       */
/*              0x9e                                                  */
/*              0x9f                                                  */
/*  -------    --------   --------   098-09F                          */
/*                                                                    */
				     0x64,

/*              0xa0                                                  */
/*              0xa1                                                  */
/*              0xa2                                                  */
/*              0xa3                                                  */
/*              0xa4                                                  */
/*              0xa5            n tilde capital                       */
/*              0xa6                                                  */
/*              0xa7                                                  */
/*  0a0-0a7    --------   --------   --------                         */
/*                                                                    */
    0x04,

/*  --------   0a8-0aF    --------   --------                         */
/*                                                                    */
	       0x00,

/*              0xb0                                                  */
/*              0xb1                                                  */
/*              0xb2                                                  */
/*              0xb3                                                  */
/*              0xb4                                                  */
/*              0xb5            a acute capital                       */
/*              0xb6            a circumflex capital                  */
/*              0xb7            a grave capital                       */
/*  --------   --------   0b0-0b7    --------                         */
/*                                                                    */
			  0x07,

/*  --------   --------   --------   0b8-0bF                          */
/*                                                                    */
				     0x00,

/*              0xc0                                                  */
/*              0xc1                                                  */
/*              0xc2                                                  */
/*              0xc3                                                  */
/*              0xc4                                                  */
/*              0xc5                                                  */
/*              0xc6                                                  */
/*              0xc7            a tilde capital                       */
/*  0c0-0c7    --------   --------   --------                         */
/*                                                                    */
    0x01,

/*  --------   0c8-0cF    --------   --------                         */
/*                                                                    */
	       0x00,

/*              0xd0                                                  */
/*              0xd1            eth icelandic capital                 */
/*              0xd2            e circumflex capital                  */
/*              0xd3            e umlaut capital                      */
/*              0xd4            e grave capital                       */
/*              0xd5                                                  */
/*              0xd6            i acute capital                       */
/*              0xd7            i circumflex capital                  */
/*  --------   --------   0d0-0d7    --------                         */
/*                                                                    */
			  0x7b,

/*              0xd8            i umlaut capital                      */
/*              0xd9                                                  */
/*              0xda                                                  */
/*              0xdb                                                  */
/*              0xdc                                                  */
/*              0xdd                                                  */
/*              0xde            i grave capital                       */
/*              0xdf                                                  */
/*  --------   --------   --------   0d8-0dF                          */
/*                                                                    */
				     0x82,

/*              0xe0            o acute capital                       */
/*              0xe1                                                  */
/*              0xe2            o circumflex capital                  */
/*              0xe3            o grave capital                       */
/*              0xe4                                                  */
/*              0xe5            o tilde capital                       */
/*              0xe6                                                  */
/*              0xe7                                                  */
/*  0e0-0e7    --------   --------   --------                         */
/*                                                                    */
    0xb4,

/*              0xe8            thorn icelandic capital               */
/*              0xe9            u acute capital                       */
/*              0xea            u circumflex capital                  */
/*              0xeb            u grave capital                       */
/*              0xec                                                  */
/*              0xed            y acute capital                       */
/*              0xee                                                  */
/*              0xef                                                  */
/*  --------   0e8-0eF    --------   --------                         */
/*                                                                    */
	       0xf4,

/*  --------   --------   0f0-0f7    --------                         */
/*                                                                    */
			  0x00,

/*  --------   --------   --------   0f8-0fF                          */
/*                                                                    */
				     0x00,

/*  100-107    108-10F    110-117    118-11F                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  120-127    128-12F    130-137    138-13F                          */
/*                                             reserved               */
    0x00,      0x1b,      0x00,      0x00,

/*  140-147    148-14F    150-157    158-15F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  160-167    168-16F    170-177    178-17F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  180-187    188-18F    190-197    198-19F                          */
/*                                             function keys          */
    0x00,      0x00,      0x00,      0x00,

/*  1A0-1A7    1A8-1AF    1B0-1B7    1B8-1BF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1C0-1C7    1C8-1CF    1D0-1D7    1D8-1DF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1E0-1E7    1E8-1EF    1F0-1F7    1F8-1FF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  200-207    208-20f    210-217    218-21f    single byte controls  */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  220-227    228-22f    230-237    238-23f                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*              0x240                                                 */
/*              0x241                                                 */
/*              0x242           1fc2 - a circumflex capital           */
/*              0x243           1fc3 - a grave capital                */
/*              0x244           1fc4 - a acute capital                */
/*              0x245           1fc5 - a tilde capital                */
/*              0x246                                                 */
/*              0x247           1fc7 - e circumflex capital           */
/*  240-247    --------   --------   --------                         */
/*                                                                    */
    0x3d,

/*              0x248           1fc8 - e umlaut capital               */
/*              0x249           1fc9 - e grave capital                */
/*              0x24a           1fca - i acute capital                */
/*              0x24b           1fcb - i circumflex capital           */
/*              0x84c           1fcc - i umlaut capital               */
/*              0x24d           1fcd - i grave capital                */
/*              0x24e           1fce - slashed o capital              */
/*              0x24f                                                 */
/*  --------   248-24f    --------   --------                         */
/*                                                                    */
	       0xfe,

/*              0x250                                                 */
/*              0x251                                                 */
/*              0x252                                                 */
/*              0x253                                                 */
/*              0x254           1fd4 - eth icelandic capital          */
/*              0x255           1fd5 - y acute capital                */
/*              0x256           1fd6 - thorn icelandic capital        */
/*              0x257                                                 */
/*  --------   --------   250-257    --------                         */
/*                                                                    */
			  0x0e,

/*              0x258                                                 */
/*              0x259                                                 */
/*              0x25a                                                 */
/*              0x25b                                                 */
/*              0x25c                                                 */
/*              0x25d                                                 */
/*              0x25e                                                 */
/*              0x25f           1fdf - o circumflex capital           */
/*  --------   --------   --------   258-25f                          */
/*                                                                    */
				     0x01,

/*              0x260           1fe0 - o grave capital                */
/*              0x261           1fe1 - o acute capital                */
/*              0x262           1fe2 - o tilde capital                */
/*              0x263                                                 */
/*              0x264           1fe4 - u circumflex capital           */
/*              0x265           1fe5 - u grave capital                */
/*              0x266           1fe6 - u acute capital                */
/*              0x267                                                 */
/*  260-267    --------   --------   --------                         */
/*                                                                    */
    0xee,

/*              0x268                                                 */
/*              0x269                                                 */
/*              0x26a                                                 */
/*              0x26b                                                 */
/*              0x86c                                                 */
/*              0x26d                                                 */
/*              0x26e                                                 */
/*              0x26f           1fef - a ogonek capital               */
/*  --------   268-26f    --------   --------                         */
/*                                                                    */
	       0x01,

/*              0x270           1ff0 - e caron capital                */
/*              0x271           1ff1 - c caron capital                */
/*              0x272           1ff2 - c acute capital                */
/*              0x273                                                 */
/*              0x274           1ff4 - e ogonek capital               */
/*              0x275           1ff5 - u overcircle capital           */
/*              0x276           1ff6 - d caron capital                */
/*              0x277           1ff7 - l acute capital                */
/*  --------   --------   270-277    --------                         */
/*                                                                    */
			  0xef,

/*  --------   --------   --------   278-27f                          */
/*                                                                    */
				     0x00,

/*              0x280                                                 */
/*              0x281           1e81 - l caron capital                */
/*              0x282           1e82 - n caron capital                */
/*              0x283           1e83 - r caron capital                */
/*              0x284           1e84 - s acute capital                */
/*              0x285                                                 */
/*              0x286                                                 */
/*              0x287                                                 */
/*  280-287    --------   --------   --------                         */
/*                                                                    */
    0x78,

/*              0x288           1e88 - z overdot capital              */
/*              0x289                                                 */
/*              0x28a                                                 */
/*              0x28b           1e8b - z caron capital                */
/*              0x88c           1e8c - z acute capital                */
/*              0x28d           1e8d - l stroke capital               */
/*              0x28e           1e8e - n acute capital                */
/*              0x28f           1e8f - s caron capital                */
/*  --------   288-28f    --------   --------                         */
/*                                                                    */
	       0x9f,

/*              0x290                                                 */
/*              0x291                                                 */
/*              0x292                                                 */
/*              0x293                                                 */
/*              0x294           1e94 - t caron capital                */
/*              0x295           1e95 - r acute capital                */
/*              0x296           1e96 - o double acute capital         */
/*              0x297           1e97 - u double acute capital         */
/*  --------   --------   290-297    --------                         */
/*                                                                    */
			  0x0f,

/*              0x298                                                 */
/*              0x299                                                 */
/*              0x29a           1e9a - i overdot capital              */
/*              0x29b           1e9b - a breve capital                */
/*              0x29c           1e9c - g breve capital                */
/*              0x29d                                                 */
/*              0x29e                                                 */
/*              0x29f                                                 */
/*  --------   --------   --------   298-29f                          */
/*                                                                    */
				     0x38,

/*              0x2a0                                                 */
/*              0x2a1                                                 */
/*              0x2a2           1ea2 - s cedilla capital              */
/*              0x2a3                                                 */
/*              0x2a4                                                 */
/*              0x2a5           1ea5 - t cedilla capital              */
/*              0x2a6                                                 */
/*              0x2a7           1ea7 - a macron capital               */
/*  2a0-2a7    --------   --------   --------                         */
/*                                                                    */
    0x25,

/*              0x2a8                                                 */
/*              0x2a9           1ea9 - c circumflex capital           */
/*              0x2aa                                                 */
/*              0x2ab                                                 */
/*              0x8ac           1eac - c overdoct capital             */
/*              0x2ad                                                 */
/*              0x2ae           1eae - e overdot capital              */
/*              0x2af                                                 */
/*  --------   2a8-2af    --------   --------                         */
/*                                                                    */
	       0x4a,

/*              0x2b0           1eb0 - e macron capital               */
/*              0x2b1                                                 */
/*              0x2b2                                                 */
/*              0x2b3           1eb3 - g circumflex capital           */
/*              0x2b4                                                 */
/*              0x2b5           1eb5 - g overdot capital              */
/*              0x2b6           1eb6 - g cedilla capital              */
/*              0x2b7                                                 */
/*  --------   --------   2b0-2b7    --------                         */
/*                                                                    */
			  0x96,

/*              0x2b8           1eb8 - h circumflex capital           */
/*              0x2b9                                                 */
/*              0x2ba           1eba - h stroke capital               */
/*              0x2bb                                                 */
/*              0x2bc           1ebc - i tilde capital                */
/*              0x2bd                                                 */
/*              0x2be           1ebe - i macron capital               */
/*              0x2bf                                                 */
/*  --------   --------   --------   2b8-2bf                          */
/*                                                                    */
				     0xaa,

/*              0x2c0           1ec0 - i ogonek capital               */
/*              0x2c1                                                 */
/*              0x2c2           1ec2 - ij ligature capital            */
/*              0x2c3                                                 */
/*              0x2c4           1ec4 - j circumflex capital           */
/*              0x2c5                                                 */
/*              0x2c6           1ec6 - k cedilla capital              */
/*              0x2c7                                                 */
/*  2c0-2c7    --------   --------   --------                         */
/*                                                                    */
    0xaa,

/*              0x2c8                                                 */
/*              0x2c9           1ec9 - l cedilla capital              */
/*              0x2ca                                                 */
/*              0x2cb           1ecb - l middle dot capital           */
/*              0x8cc                                                 */
/*              0x2cd           1ecd - n cedilla capital              */
/*              0x2ce                                                 */
/*              0x2cf           1ecf - n eng lapp capital             */
/*  --------   2c8-2cf    --------   --------                         */
/*                                                                    */
	       0x55,

/*              0x2d0                                                 */
/*              0x2d1           1ed1 - o macro capital                */
/*              0x2d2                                                 */
/*              0x2d3           1ed3 - oe ligature capital            */
/*              0x2d4                                                 */
/*              0x2d5           1ed5 - r cedilla capital              */
/*              0x2d6                                                 */
/*              0x2d7           1ed7 - s circumflex capital           */
/*  --------   --------   2d0-2d7    --------                         */
/*                                                                    */
			  0x55,

/*              0x2d8                                                 */
/*              0x2d9           1ed9 - t stroke capital               */
/*              0x2da                                                 */
/*              0x2db           1edb - u tilde capital                */
/*              0x2dc                                                 */
/*              0x2dd           1edd - u breve capital                */
/*              0x2de                                                 */
/*              0x2df           1edf - u macron capital               */
/*  --------   --------   --------   2d8-2df                          */
/*                                                                    */
				     0x55,

/*              0x2e0                                                 */
/*              0x2e1           1ee1 - u ogonek capital               */
/*              0x2e2                                                 */
/*              0x2e3           1ee3 - w circumflex capital           */
/*              0x2e4                                                 */
/*              0x2e5           1ee5 - y circumflex capital           */
/*              0x2e6           1ee6 - y umlaut capital               */
/*              0x2e7                                                 */
/*  200-207    --------   --------   --------                         */
/*                                                                    */
    0x56

		     }  ;               /* end cm_U                   */



/* L - Lower case alphabetic characters only                          */

static
unsigned char  cm_L[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x00,      0x00,      0x00,      0x00,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0x00,      0x00,      0x00,      0x00,

/*  060-067    068-06F    070-077    078-07F                          */
/*  `abcdefg   hijklmno   pqrstuvw   xyz{|}~   7F is del              */
    0x7F,      0xFF,      0xFF,      0xE0,

/*              0x80            c cedilla capital                     */
/*              0x81            small                                 */
/*              0x82            "                                     */
/*              0x83            "                                     */
/*              0x84            "                                     */
/*              0x85            "                                     */
/*              0x86            "                                     */
/*              0x87            "                                     */
/*  080-087    --------   --------   --------                         */
/*                                                                    */
    0x7f,

/*              0x88            small                                 */
/*              0x89            "                                     */
/*              0x8a            "                                     */
/*              0x8b            "                                     */
/*              0x8c            "                                     */
/*              0x8d            "                                     */
/*              0x8e            a umlaut capital                      */
/*              0x8f            a overcircle capital                  */
/*  --------   088-08F    --------   --------                         */
/*                                                                    */
	       0xfc,

/*              0x90            e acute capital                       */
/*              0x91            small                                 */
/*              0x92            ae dipthong capital                   */
/*              0x93            small                                 */
/*              0x94            "                                     */
/*              0x95            "                                     */
/*              0x96            "                                     */
/*              0x97            "                                     */
/*  --------   --------   090-097    --------                         */
/*                                                                    */
			  0x5f,

/*              0x98            small                                 */
/*              0x99            o umlaut capital                      */
/*              0x9a            u umlaut capital                      */
/*              0x9b            small                                 */
/*              0x9c                                                  */
/*              0x9d            o slash capital                       */
/*              0x9e                                                  */
/*              0x9f                                                  */
/*  -------    --------   --------   098-09F                          */
/*                                                                    */
				     0x90,

/*              0xa0            small                                 */
/*              0xa1            "                                     */
/*              0xa2            "                                     */
/*              0xa3            "                                     */
/*              0xa4            "                                     */
/*              0xa5            n tilde capital                       */
/*              0xa6                                                  */
/*              0xa7                                                  */
/*  0a0-0a7    --------   --------   --------                         */
/*                                                                    */
    0xf8,

/*  --------   0a8-0aF    --------   --------                         */
/*                                                                    */
	       0x00,

/*  --------   --------   0b0-0b7    --------                         */
/*                                                                    */
			  0x00,

/*  --------   --------   --------   0b8-0bF                          */
/*                                                                    */
				     0x00,

/*              0xc0                                                  */
/*              0xc1                                                  */
/*              0xc2                                                  */
/*              0xc3                                                  */
/*              0xc4                                                  */
/*              0xc5                                                  */
/*              0xc6            small                                 */
/*              0xc7            a tilde capital                       */
/*  0c0-0c7    --------   --------   --------                         */
/*                                                                    */
    0x02,

/*  --------   0c8-0cF    --------   --------                         */
/*                                                                    */
	       0x00,

/*              0xd0            small                                 */
/*              0xd1            eth icelandic capital                 */
/*              0xd2            e circumflex capital                  */
/*              0xd3            e umlaut capital                      */
/*              0xd4            e grave capital                       */
/*              0xd5            small i dotless                       */
/*              0xd6            i acute capital                       */
/*              0xd7            i circumflex capital                  */
/*  --------   --------   0d0-0d7    --------                         */
/*                                                                    */
			  0x84,

/*  --------   --------   --------   0d8-0dF                          */
/*                                                                    */
				     0x00,

/*              0xe0            o acute capital                       */
/*              0xe1            small                                 */
/*              0xe2            o circumflex capital                  */
/*              0xe3            o grave capital                       */
/*              0xe4            small                                 */
/*              0xe5            o tilde capital                       */
/*              0xe6                                                  */
/*              0xe7            small                                 */
/*  0e0-0e7    --------   --------   --------                         */
/*                                                                    */
    0x49,

/*              0xe8            thorn icelandic capital               */
/*              0xe9            u acute capital                       */
/*              0xea            u circumflex capital                  */
/*              0xeb            u grave capital                       */
/*              0xec            small                                 */
/*              0xed            y acute capital                       */
/*              0xee                                                  */
/*              0xef                                                  */
/*  --------   0e8-0eF    --------   --------                         */
/*                                                                    */
	       0x08,

/*  --------   --------   0f0-0f7    --------                         */
/*                                                                    */
			  0x00,

/*  --------   --------   --------   0f8-0fF                          */
/*                                                                    */
				     0x00,

/*  100-107    108-10F    110-117    118-11F                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  120-127    128-12F    130-137    138-13F                          */
/*                                             reserved               */
    0x00,      0x1b,      0x00,      0x00,

/*  140-147    148-14F    150-157    158-15F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  160-167    168-16F    170-177    178-17F                          */
/*                                             reserved               */
    0x00,      0x00,      0x00,      0x00,

/*  180-187    188-18F    190-197    198-19F                          */
/*                                             function keys          */
    0x00,      0x00,      0x00,      0x00,

/*  1A0-1A7    1A8-1AF    1B0-1B7    1B8-1BF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1C0-1C7    1C8-1CF    1D0-1D7    1D8-1DF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  1E0-1E7    1E8-1EF    1F0-1F7    1F8-1FF                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  200-207    208-20f    210-217    218-21f    single byte controls  */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*  220-227    228-22f    230-237    238-23f                          */
/*                                                                    */
    0x00,      0x00,      0x00,      0x00,

/*              0x240           1fc0 - small                          */
/*              0x241           1fc1 - "                              */
/*              0x242           1fc2 - a circumflex capital           */
/*              0x243           1fc3 - a grave capital                */
/*              0x244           1fc4 - a acute capital                */
/*              0x245           1fc5 - a tilde capital                */
/*              0x246           1fc6 - small                          */
/*              0x247           1fc7 - e circumflex capital           */
/*  240-247    --------   --------   --------                         */
/*                                                                    */
    0xc2,

/*              0x248           1fc8 - e umlaut capital               */
/*              0x249           1fc9 - e grave capital                */
/*              0x24a           1fca - i acute capital                */
/*              0x24b           1fcb - i circumflex capital           */
/*              0x84c           1fcc - i umlaut capital               */
/*              0x24d           1fcd - i grave capital                */
/*              0x24e           1fce - slashed o capital              */
/*              0x24f           1fcf - small                          */
/*  --------   248-24f    --------   --------                         */
/*                                                                    */
	       0x01,

/*              0x250           1fd0 - small                          */
/*              0x251           1fd1 - "                              */
/*              0x252                                                 */
/*              0x253                                                 */
/*              0x254           1fd4 - eth icelandic capital          */
/*              0x255           1fd5 - y acute capital                */
/*              0x256           1fd6 - thorn icelandic capital        */
/*              0x257                                                 */
/*  --------   --------   250-257    --------                         */
/*                                                                    */
			  0xc0,

/*              0x258                                                 */
/*              0x259                                                 */
/*              0x25a                                                 */
/*              0x25b                                                 */
/*              0x25c                                                 */
/*              0x25d           1fdd - small                          */
/*              0x25e           1fde - small i dotless                */
/*              0x25f           1fdf - o circumflex capital           */
/*  --------   --------   --------   258-25f                          */
/*                                                                    */
				     0x06,

/*              0x260           1fe0 - o grave capital                */
/*              0x261           1fe1 - o acute capital                */
/*              0x262           1fe2 - o tilde capital                */
/*              0x263                                                 */
/*              0x264           1fe4 - u circumflex capital           */
/*              0x265           1fe5 - u grave capital                */
/*              0x266           1fe6 - u acute capital                */
/*              0x267           1fe7 - small                          */
/*  260-267    --------   --------   --------                         */
/*                                                                    */
    0x01,

/*              0x268           1fe8 - small                          */
/*              0x269           1fe9 - "                              */
/*              0x26a           1fea - "                              */
/*              0x26b           1feb - "                              */
/*              0x86c           1fec - "                              */
/*              0x26d           1fee - "                              */
/*              0x26e           1fed - "                              */
/*              0x26f           1fef - a ogonek capital               */
/*  --------   268-26f    --------   --------                         */
/*                                                                    */
	       0xfe,

/*  --------   --------   270-277    --------                         */
/*                                                                    */
			  0x00,

/*              0x278           1ff8 - small                          */
/*              0x279           1ff9 - "                              */
/*              0x27a           1ffa - "                              */
/*              0x27b           1ffb - "                              */
/*              0x27c           1ffc - "                              */
/*              0x27d                                                 */
/*              0x27e           1ffe - "                              */
/*              0x27f           1fff - "                              */

/*  --------   --------   --------   278-27f                          */
/*                                                                    */
				     0xfb,

/*              0x280           1e80 - small                          */
/*              0x281           1e81 - l caron capital                */
/*              0x282           1e82 - n caron capital                */
/*              0x283           1e83 - r caron capital                */
/*              0x284           1e84 - s acute capital                */
/*              0x285                                                 */
/*              0x286           1e86 - small                          */
/*              0x287                                                 */
/*  280-287    --------   --------   --------                         */
/*                                                                    */
    0x82,

/*              0x288           1e88 - z overdot capital              */
/*              0x289           1e89 - small                          */
/*              0x28a           1e8a - "                              */
/*              0x28b           1e8b - z caron capital                */
/*              0x88c           1e8c - z acute capital                */
/*              0x28d           1e8d - l stroke capital               */
/*              0x28e           1e8e - n acute capital                */
/*              0x28f           1e8f - s caron capital                */
/*  --------   288-28f    --------   --------                         */
/*                                                                    */
	       0x60,

/*              0x290           1e90 - small                          */
/*              0x291           1e91 - "                              */
/*              0x292           1e92 - "                              */
/*              0x293           1e93 - "                              */
/*              0x294           1e94 - t caron capital                */
/*              0x295           1e95 - r acute capital                */
/*              0x296           1e96 - o double acute capital         */
/*              0x297           1e97 - u double acute capital         */
/*  --------   --------   290-297    --------                         */
/*                                                                    */
			  0xf0,

/*              0x298           1e98 - small                          */
/*              0x299           1e99 - "                              */
/*              0x29a           1e9a - i overdot capital              */
/*              0x29b           1e9b - a breve capital                */
/*              0x29c           1e9c - g breve capital                */
/*              0x29d                                                 */
/*              0x29e                                                 */
/*              0x29f           1e9f - small                          */
/*  --------   --------   --------   298-29f                          */
/*                                                                    */
				     0xc1,

/*              0x2a0                                                 */
/*              0x2a1           1ea1 - high comma n small             */
/*              0x2a2           1ea2 - s cedilla capital              */
/*              0x2a3                                                 */
/*              0x2a4           1ea4 - small                          */
/*              0x2a5           1ea5 - t cedilla capital              */
/*              0x2a6           1ea6 - small                          */
/*              0x2a7           1ea7 - a macron capital               */
/*  2a0-2a7    --------   --------   --------                         */
/*                                                                    */
    0x4a,

/*              0x2a8           1ea8 - small                          */
/*              0x2a9           1ea9 - c circumflex capital           */
/*              0x2aa                                                 */
/*              0x2ab           1eab - small                          */
/*              0x8ac           1eac - c overdoct capital             */
/*              0x2ad           1ead - small                          */
/*              0x2ae           1eae - e overdot capital              */
/*              0x2af           1eaf - small                          */
/*  --------   2a8-2af    --------   --------                         */
/*                                                                    */
	       0x95,

/*              0x2b0           1eb0 - e macron capital               */
/*              0x2b1           1eb1 - small                          */
/*              0x2b2           1eb2 - "                              */
/*              0x2b3           1eb3 - g circumflex capital           */
/*              0x2b4           1eb4 - small                          */
/*              0x2b5           1eb5 - g overdot capital              */
/*              0x2b6           1eb6 - g cedilla capital              */
/*              0x2b7           1eb7 - small                          */
/*  --------   --------   2b0-2b7    --------                         */
/*                                                                    */
			  0x69,

/*              0x2b8           1eb8 - h circumflex capital           */
/*              0x2b9           1eb9 - small                          */
/*              0x2ba           1eba - h stroke capital               */
/*              0x2bb           1ebb - small                          */
/*              0x2bc           1ebc - i tilde capital                */
/*              0x2bd           1ebb - small                          */
/*              0x2be           1ebe - i macron capital               */
/*              0x2bf           1ebf - small                          */
/*  --------   --------   --------   2b8-2bf                          */
/*                                                                    */
				     0x55,

/*              0x2c0           1ec0 - i ogonek capital               */
/*              0x2c1           1ec1 - small                          */
/*              0x2c2           1ec2 - ij ligature capital            */
/*              0x2c3           1ec3 - small                          */
/*              0x2c4           1ec4 - j circumflex capital           */
/*              0x2c5           1ec5 - small                          */
/*              0x2c6           1ec6 - k cedilla capital              */
/*              0x2c7           1ec7 - small                          */
/*  2c0-2c7    --------   --------   --------                         */
/*                                                                    */
    0x55,

/*              0x2c8           1ec8 - small                          */
/*              0x2c9           1ec9 - l cedilla capital              */
/*              0x2ca           1eca - small                          */
/*              0x2cb           1ecb - l middle dot capital           */
/*              0x8cc           1ecc - small                          */
/*              0x2cd           1ecd - n cedilla capital              */
/*              0x2ce           1ece - small                          */
/*              0x2cf           1ecf - n eng lapp capital             */
/*  --------   2c8-2cf    --------   --------                         */
/*                                                                    */
	       0xaa,

/*              0x2d0           1ed0 - small                          */
/*              0x2d1           1ed1 - o macro capital                */
/*              0x2d2           1ed2 - small                          */
/*              0x2d3           1ed3 - oe ligature capital            */
/*              0x2d4           1ed4 - small                          */
/*              0x2d5           1ed5 - r cedilla capital              */
/*              0x2d6           1ed6 - small                          */
/*              0x2d7           1ed7 - s circumflex capital           */
/*  --------   --------   2d0-2d7    --------                         */
/*                                                                    */
			  0xaa,

/*              0x2d8           1ed8 - small                          */
/*              0x2d9           1ed9 - t stroke capital               */
/*              0x2da           1eda - small                          */
/*              0x2db           1edb - u tilde capital                */
/*              0x2dc           1edc - small                          */
/*              0x2dd           1edd - u breve capital                */
/*              0x2de           1ede - small                          */
/*              0x2df           1edf - u macron capital               */
/*  --------   --------   --------   2d8-2df                          */
/*                                                                    */
				     0xaa,

/*              0x2e0           1ee0 - small                          */
/*              0x2e1           1ee1 - u ogonek capital               */
/*              0x2e2           1ee2 - small                          */
/*              0x2e3           1ee3 - w circumflex capital           */
/*              0x2e4           1ee4 - small                          */
/*              0x2e5           1ee5 - y circumflex capital           */
/*              0x2e6           1ee6 - y umlaut capital               */
/*              0x2e7                                                 */
/*  200-207    --------   --------   --------                         */
/*                                                                    */
    0xa8

		     }  ;               /* end cm_L                   */


/* N - Numeric characters only                                        */

static
unsigned char  cm_N[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x00,      0x00,      0xFF,      0xC0

		     }  ;               /* end cm_N                   */


/* X - Hexadecimal characters only [0-9] [A-F] or [a-f]               */

static
unsigned char  cm_X[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x00,      0x00,      0xFF,      0xC0,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0x7E,      0x00,      0x00,      0x00,

/*  060-067    068-06F    070-077    078-07F                          */
/*  `abcdefg   hijklmno   pqrstuvw   xyz{|}~   7F is del              */
    0x7E,      0x00,      0x00,      0x00

		     }  ;               /* end cm_X                   */

/* B - Space character only                                             */

static
unsigned char  cm_B[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x80,      0x00,      0x00,      0x00

		     }  ;               /* end cm_B                   */

/* G - Graphic characters in normal ASCII                               */

static
unsigned char  cm_G[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x7F,      0xFF,      0xFF,      0xFF,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0xFF,      0xFF,      0xFF,      0xFF,

/*  060-067    068-06F    070-077    078-07F                          */
/*  `abcdefg   hijklmno   pqrstuvw   xyz{|}~   7F is del              */
    0xFF,      0xFF,      0xFF,      0xFE

		     }  ;               /* end cm_G                   */


/* C - Cursor movement keys and other control keys in field input       */

static
unsigned char  cm_C[]  = {

/*  000-007    008-00F    010-017    018-01F                          */
/*                                             single byte controls   */
    0x00,      0x00,      0x00,      0x00,

/*  020-027    028-02F    030-037    038-03F                          */
/*   !"#$%&'   ()*+,-./   01234567   89:;<=>?  20 is space            */
    0x00,      0x00,      0x00,      0x00,

/*  040-047    048-04F    050-057    058-05F                          */
/*  @ABCDEFG   HIJKLMNO   PQRSTUVW   XYZ[\]^_                         */
    0x00,      0x00,      0x00,      0x00,

/*  060-067    068-06F    070-077    078-07F                          */
/*  `abcdefg   hijklmno   pqrstuvw   xyz{|}~   7F is del              */
    0x00,      0x00,      0x00,      0x00,

/*  080-087    088-08F    090-097    098-09F                          */
/*                                             PC graphics            */
    0x00,      0x00,      0x00,      0x00,

/*  0A0-0A7    0A8-0AF    0B0-0B7    0B8-0BF                          */
/*                                             PC graphics            */
    0x00,      0x00,      0x00,      0x00,

/*  0C0-0C7    0C8-0CF    0D0-0D7    0D8-0DF                          */
/*                                             PC graphics            */
    0x00,      0x00,      0x00,      0x00,

/*  0E0-0E7    0E8-0EF    0F0-0F7    0F8-0FF                          */
/*                                             PC graphics            */
    0x00,      0x00,      0x00,      0x00,

/*     KEY_BREAK      0x101              break - unreliable           */
/*     KEY_DOWN       0x102             +cursor down                  */
/*     KEY_UP         0x103             +cursor up                    */
/*     KEY_LEFT       0x104             +cursor left                  */
/*     KEY_RIGHT      0x105             +cursor right                 */
/*     KEY_HOME       0x106              home - top left              */
/*     KEY_BACKSPACE  0x107             +backspace - unreliable       */
/*  100-107    -------    -------    -------                          */
/*                                                                    */
    0x3D,

/*     KEY_DL         0x108              delete line                  */
/*     KEY_IL         0x109              insert line                  */
/*     KEY_DC         0x10a             +delete character             */
/*     KEY_IC         0x10b             +insert character mode start  */
/*     KEY_EIC        0x10c             +exit insert character mode   */
/*     KEY_CLEAR      0x10d              clear screen                 */
/*     KEY_EOS        0x10e             +clear to end of screen       */
/*     KEY_EOL        0x10f              clear to end of line         */
/*  -------    108-10F    -------    -------                          */
/*                                                                    */
	       0x3A,

/*     KEY_SF         0x110             +scroll forward toward end    */
/*     KEY_SR         0x111             +scroll backward toward start */
/*     KEY_NPAGE      0x112              next page                    */
/*     KEY_PPAGE      0x113              previous page                */
/*     KEY_STAB       0x114              set tab stop                 */
/*     KEY_CTAB       0x115              clear tab stop               */
/*     KEY_CATAB      0x116              clear all tab stops          */
/*     KEY_ENTER      0x117              enter key - unreliable       */
/*  -------    -------    110-117    -------                          */
/*                                                                    */
			  0xc0,

/*     KEY_SRESET     0x118              soft reset key - unreliable  */
/*     KEY_RESET      0x119              hard reset key - unreliable  */
/*     KEY_PRINT      0x11a              print or copy                */
/*     KEY_LL         0x11b              lower left (last line)       */
/*     KEY_A1         0x11c              pad upper left               */
/*     KEY_A3         0x11d              pad upper right              */
/*     KEY_B2         0x11e              pad center                   */
/*     KEY_C1         0x11f              pad lower left               */
/*  -------    -------    -------    118-11F                          */
/*                                                                    */
				     0x00,

/*     KEY_C3         0x120              pad lower right              */
/*     KEY_DO         0x121              DO key                       */
/*     KEY_QUIT       0x122              QUIT key                     */
/*     KEY_CMD        0x123              Command key                  */
/*     KEY_PCMD       0x124              Previous command key         */
/*     KEY_NPN        0x125              Next pane key                */
/*     KEY_PPN        0x126              previous pane key            */
/*     KEY_CPN        0x127              command pane key             */
/*  120-127    -------    -------    -------                          */
/*                                                                    */
    0x00,

/*     KEY_END        0x128              end key                      */
/*     KEY_HLP        0x129              help key                     */
/*     KEY_SEL        0x12a              select key                   */
/*     KEY_SCR        0x12b             +scroll right key             */
/*     KEY_SCL        0x12c             +scroll left key              */
/*     KEY_TAB        0x12d              tab key                      */
/*     KEY_BTAB       0x12e             +back tab key                 */
/*     KEY_NEWL       0x12f             +new line key                 */
/*  -------    128-12F    130-137    138-13F                          */
/*                                             130-13f reserved       */
	       0x1b,      0x00,      0x00

		     }  ;               /* end initialization values  */
#endif                          /* _H_NLSCM */
