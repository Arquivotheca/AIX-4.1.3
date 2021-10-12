static char sccsid[] = "@(#)08  1.2  src/bos/kernext/lft/swkbd/diac88594.c, lftdd, bos411, 9428A410j 10/25/93 15:29:13";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /*
 * Function:  These tables are used to identify valid diacritics and
 *      then match them up with a valid second character.
 *
 *      The shdiac structure contains a list of all valid diacritics.
 *      When a keystroke is received, it is matched against each symb
 *      in the structure to determine if it is a valid diacritic.  If
 *      it is, the index is retrieved.  The index is a ushort which
 *      contains a start index in the top half and a stop index in the
 *      lower half.  These indexes define a range in the shcomp table
 *      to search.
 *
 *      When the second keystroke is received, it is matched against each
 *      compin field of the the shcomp table from the start index to the
 *      stop index retrieved from the shdiac table in the first step.  If
 *      a match is found, the two characters are replaced by the compout
 *      character, which is the graphic for a valid diacritic.
 */


#define LFNUMDIACRITIC  0x50

#include <lftcode.h>
#include <kks.h>                        /* structure definitions */

/*-----------
   FUNCTION: Composition table for dead keys

   This table is set up so that the first entry of each diacritic "range"
   is the space character, which will result in an output of the
   diacritic itself.
------------*/

struct diacritic diactbl88594 = {
        {'8','8','5','9','-','4','\0','\0'},    /*charset encoding*/
        0x50,                                   /* number of entries in table*/
        0,
        {
/*0x00*/ {0xB4, IC_SP , 0xB4},          /* acute */
/*0x01*/ {0xB4, IC_LCE, 0xE9},
/*0x02*/ {0xB4, IC_LCA, 0xE1},
/*0x03*/ {0xB4, IC_LCI, 0xED},
/*0x04*/ {0xB4, IC_LCU, 0xFA},
/*0x05*/ {0xB4, IC_UCE, 0xC9},
/*0x06*/ {0xB4, IC_UCA, 0xC1},
/*0x07*/ {0xB4, IC_UCI, 0xCD},
/*0x08*/ {0xB4, IC_UCU, 0xDA},

/*0x09*/ {0x5E, IC_SP , 0x5E},          /* circumflex */
/*0x0a*/ {0x5E, IC_LCA, 0xE2},
/*0x0b*/ {0x5E, IC_LCI, 0xEE},
/*0x0c*/ {0x5E, IC_LCO, 0xF4},
/*0x0d*/ {0x5E, IC_UCA, 0xC2},
/*0x0e*/ {0x5E, IC_UCI, 0xCE},
/*0x0f*/ {0x5E, IC_UCO, 0xD4},

/*0x10*/ {0xA8, IC_SP , 0xA8},          /* diacresis (umlaut) */
/*0x11*/ {0xA8, IC_LCU, 0xFC},
/*0x12*/ {0xA8, IC_LCA, 0xE4},
/*0x13*/ {0xA8, IC_LCO, 0xF6},
/*0x14*/ {0xA8, IC_LCE, 0xEB},
/*0x15*/ {0xA8, IC_UCU, 0xDC},
/*0x16*/ {0xA8, IC_UCA, 0xC4},
/*0x17*/ {0xA8, IC_UCO, 0xD6},
/*0x18*/ {0xA8, IC_UCE, 0xCB},

/*0x19*/ {0xB8, IC_SP , 0xB8},          /* cedilla */
/*0x1a*/ {0xB8, IC_LCR, 0xB3},
/*0x1b*/ {0xB8, IC_LCK, 0xF3},
/*0x1c*/ {0xB8, IC_LCN, 0xF1},
/*0x1d*/ {0xB8, IC_UCR, 0xA3},
/*0x1e*/ {0xB8, IC_UCK, 0xD3},
/*0x1f*/ {0xB8, IC_UCN, 0xD1},
/*0x20*/ {0xB8, IC_UCL, 0xB6},
/*0x21*/ {0xB8, IC_UCL, 0xA6},

/*0x21*/ {0xFF, IC_SP , 0xFF},          /* overdot */
/*0x22*/ {0xFF, IC_LCE, 0xEC},
/*0x23*/ {0xFF, IC_LCE, 0xCC},

/*0x24*/ {0xB2, IC_SP , 0xB2},          /* ? */
/*0x25*/ {0xB2, IC_LCA, 0xB1},
/*0x26*/ {0xB2, IC_UCA, 0xA1},
/*0x27*/ {0xB2, IC_LCE, 0xEA},
/*0x28*/ {0xB2, IC_UCE, 0xCA},
/*0x29*/ {0xB2, IC_LCI, 0xE7},
/*0x2a*/ {0xB2, IC_UCI, 0xC7},
/*0x2b*/ {0xB2, IC_LCU, 0xF9},
/*0x2c*/ {0xB2, IC_UCU, 0xD9},

/*0x2d*/ {0xB7, IC_SP , 0xB7},          /* ? */
/*0x2e*/ {0xB7, IC_LCS, 0xB9},
/*0x2f*/ {0xB7, IC_UCS, 0xA9},
/*0x30*/ {0xB7, IC_LCZ, 0xBE},
/*0x31*/ {0xB7, IC_UCZ, 0xAE},
/*0x32*/ {0xB7, IC_LCC, 0xE8},
/*0x33*/ {0xB7, IC_UCC, 0xC8},

/*0x34*/ {0xAF, IC_SP , 0xAF},          /* overbar */
/*0x35*/ {0xAF, IC_UCE, 0xBA},
/*0x36*/ {0xAF, IC_UCE, 0xAA},
/*0x37*/ {0xAF, IC_LCA, 0xE0},
/*0x38*/ {0xAF, IC_UCA, 0xC0},
/*0x39*/ {0xAF, IC_UCI, 0xEF},
/*0x3a*/ {0xAF, IC_UCI, 0xCF},
/*0x3b*/ {0xAF, IC_LCO, 0xF2},
/*0x3c*/ {0xAF, IC_UCO, 0xD2},
/*0x3d*/ {0xAF, IC_LCU, 0xFE},
/*0x3e*/ {0xAF, IC_UCU, 0xDE},

/*0x3f*/ {0x7E, IC_SP , 0x7E},          /* tilde */
/*0x40*/ {0x7E, IC_UCI, 0xB5},
/*0x41*/ {0x7E, IC_UCI, 0xA5},
/*0x42*/ {0x7E, IC_LCA, 0xE3},
/*0x43*/ {0x7E, IC_UCA, 0xC3},
/*0x44*/ {0x7E, IC_UCO, 0xF5},
/*0x45*/ {0x7E, IC_UCO, 0xD5},
/*0x46*/ {0x7E, IC_LCU, 0xFD},
/*0x47*/ {0x7E, IC_UCU, 0xDD},
/*0x48*/ {0x7E, IC_LCU, 0xFE},
/*0x49*/ {0x7E, IC_UCU, 0xDE},
        }
};

