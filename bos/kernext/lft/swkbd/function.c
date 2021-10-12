static char sccsid[] = "@(#)03  1.6  src/bos/kernext/lft/swkbd/function.c, lftdd, bos411, 9428A410j 10/25/93 15:53:31";
/*
 * COMPONENT_NAME: LFTDD
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
 * This table is used to expand a key which has been maps to a
 * predefined function.  It is indexed by code point.
 *
 *
 *      0x000-0x00FE = programmable function keys 1 (0x0000)
 *                      through 255 (0x00FE)
 *      0x0101 = CUU, cursor up                                 esc [ A
 *      0x0102 = CUD, cursor down                               esc [ B
 *      0x0103 = CUF, cursor forward                            esc [ C
 *      0x0104 = CUB, cursor back                               esc [ D
 *      0x0105 = CBT, cursor back tab                           esc [ Z
 *      0x0106 = CHT, cursor horizontal tab                     esc [ I
 *      0x0107 = CVT, cursor vertical tab                       esc [ Y
 *      0x0108 = HVP, home                                      esc [ f
 *      0x0109 = LL, line last, first char
 *      0x010A = END, last line, last char
 *      0x010B = CPL, cursor preceding line, first char         esc [ F
 *      0x010C = CNL, cursor next line, first char              esc [ E
 *      0x0151 = DCH = delete char                              esc [ P
 *      0x0152 = IL, insert line                                esc [ L
 *      0x0153 = DL, delete line                                esc [ M
 *      0x0154 = EL, erase to end of line                       esc [ K
 *      0x0155 = EF, erase to end of field (next tab stop)      esc [ F
 *      0x0156 = ED, erase display                              esc [ 2 J
 *      0x0157 = RIS, restore initial state of VT               esc c
 *      0x0162 = RI, reverse index                              esc L
 *      0x0163 = IND, index                                     esc D
 *      0x01FF = IGNORE, send no info when key pressed
 *
 *  User types a key.  Keyboard device driver returns position code and
 *  shift flag, which are used to index the software keyboard table.  From
 *  the keyboard table, we get a 16-bit function id (filled into the
 *  code page and code point character variables).
 */
#include <sys/types.h>
#include <lft_swkbd.h>
#include <lftcode.h>

uchar kdbase[0x1ff - 0x101 + 1] =  {
IC_CUU,IC_CUD,IC_CUF,IC_CUB,IC_CBT,   IC_CHT,   IC_CVT,   IC_CUP,/*101 thru 108*/
IC_CUP,IC_CUP,IC_CPL,IC_CNL,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,/*109 thru 110*/
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 111 thru 120 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 121 thru 130 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 131 thru 140 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 141 thru 150 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_DCH,IC_IL,IC_DL,IC_EL,IC_EF,IC_ED,IC_RIS,IC_IGNORE,       /* 151 thru 158 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,           /* 159 thru 15F */
IC_IGNORE,IC_IGNORE,
IC_NEL,IC_NEL,IC_RI,IC_IND,IC_PLU,IC_PLD,IC_IGNORE,IC_IGNORE,/* 160 thru 167 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,           /* 168 thru 170 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 171 thru 180 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 181 thru 190 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 191 thru 1A0 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 1A1 thru 1B0 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 1B1 thru 1C0 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 1C1 thru 1D0 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE, /* 1D1 thru 1E0 */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,           /* 1F1 thru 1FF */
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,
IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE,IC_IGNORE
};

/*----------
  Table used to determine whether or not a function id is valid
  ----------*/
#define FID_TABLE_INVALID_ID    0xFF

uchar fid_table[0x1ff - 0x101 + 1] =  {
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CUU    0x0101  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CUD    0x0102  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CUF    0x0103  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CUB    0x0104  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CBT    0x0105  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CHT    0x0106  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CVT    0x0107  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#HOM    0x0108  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#LL     0x0109  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#END    0x010A  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CPL    0x010B  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_CURSOR,  /* KF#CNL    0x010C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x010D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x010E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x010F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0110  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0111  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0112  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0113  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0114  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0115  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0116  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0117  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0118  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0119  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x011F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0120  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0121  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0122  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0123  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0124  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0125  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0126  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0127  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0128  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0129  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x012F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0130  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0131  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0132  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0133  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0134  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0135  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0136  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0137  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0138  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0139  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x013F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0140  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0141  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0142  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0143  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0144  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0145  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0146  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0147  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0148  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0149  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x014F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0150  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#DCH    0x0151  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#IL     0x0152  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#DL     0x0153  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#EEOL   0x0154  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#EEOF   0x0155  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#CLR    0x0156  */
        FLAG_CNTL_FUNCTION << 4 | CODE_STAT_NONE,    /* KF#INIT   0x0157  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0158  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0159  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x015F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0160  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0161  */
        FLAG_ESC_FUNCTION << 4 | CODE_STAT_CURSOR,   /* KF#RI     0x0162  */
        FLAG_ESC_FUNCTION << 4 | CODE_STAT_CURSOR,   /* KF#IND    0x0163  */
        FLAG_ESC_FUNCTION << 4 | CODE_STAT_NONE,     /* KF#PLU    0x0164  */
        FLAG_ESC_FUNCTION << 4 | CODE_STAT_NONE,     /* KF#PLD    0x0165  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0166  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0167  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0168  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0169  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x016F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0170  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0171  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0172  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0173  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0174  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0175  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0176  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0177  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0178  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0179  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x017F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0180  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0181  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0182  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0183  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0184  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0185  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0186  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0187  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0188  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0189  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x018F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x0190  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0191  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0192  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0193  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0194  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0195  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0196  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0197  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0198  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x0199  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019A  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019B  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019C  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019D  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019E  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x019F  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01A0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01A9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AD  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AE  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01AF  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01B0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01B9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BD  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BE  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01BF  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01C0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01C9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CD  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CE  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01CF  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01D0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01D9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DD  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DE  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01DF  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01E0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01E9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01EA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01EB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01EC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01ED  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01EE  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01EF  */

        FID_TABLE_INVALID_ID,                   /* invalid   0x01F0  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F1  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F2  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F3  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F4  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F5  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F6  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F7  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F8  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01F9  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01FA  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01FB  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01FC  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01FD  */
        FID_TABLE_INVALID_ID,                   /* invalid   0x01FE  */
        FLAG_ESC_FUNCTION << 4 | CODE_STAT_NONE      /* KF#IGNORE 0x01FF  */
};
