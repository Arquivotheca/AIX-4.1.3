static char sccsid[] = "@(#)05  1.2  src/bos/kernext/lft/swkbd/diac88591.c, lftdd, bos411, 9428A410j 10/25/93 15:28:36";
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
 *
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


#define LFNUMDIACRITIC  0x3c

#include <lftcode.h>
#include <kks.h>                        /* structure definitions */


/*-----------
   FUNCTION: Composition table for dead keys

   This table is set up so that the first entry of each diacritic "range"
   is the space character, which will result in an output of the
   diacritic itself.
------------*/

struct diacritic diactbl88591 = {
        {'8','8','5','9','-','1','\0','\0'},    /*charset encoding*/
        0x3c,                                   /* number of entries in table*/
        0,
        {
/*0x00*/ {0xB4, IC_SP , 0xB4},          /* acute */
/*0x01*/ {0xB4, IC_LCE, 0xE9},
/*0x02*/ {0xB4, IC_LCA, 0xE1},
/*0x03*/ {0xB4, IC_LCI, 0xED},
/*0x04*/ {0xB4, IC_LCO, 0xF3},
/*0x05*/ {0xB4, IC_LCU, 0xFA},
/*0x06*/ {0xB4, IC_LCY, 0xFD},
/*0x07*/ {0xB4, IC_UCE, 0xC9},
/*0x08*/ {0xB4, IC_UCA, 0xC1},
/*0x09*/ {0xB4, IC_UCI, 0xCD},
/*0x0a*/ {0xB4, IC_UCO, 0xD3},
/*0x0b*/ {0xB4, IC_UCU, 0xDA},
/*0x0c*/ {0xB4, IC_UCY, 0xDD},

/*0x0d*/ {0x60, IC_SP , 0x60},          /* grave */
/*0x0e*/ {0x60, IC_LCE, 0xE8},
/*0x0f*/ {0x60, IC_LCA, 0xE0},
/*0x10*/ {0x60, IC_LCI, 0xEC},
/*0x11*/ {0x60, IC_LCO, 0xF2},
/*0x12*/ {0x60, IC_LCU, 0xF9},
/*0x13*/ {0x60, IC_UCE, 0xC8},
/*0x14*/ {0x60, IC_UCA, 0xC0},
/*0x15*/ {0x60, IC_UCI, 0xCC},
/*0x16*/ {0x60, IC_UCO, 0xD2},
/*0x17*/ {0x60, IC_UCU, 0xD9},

/*0x18*/ {0x5E, IC_SP , 0x5E},          /* circumflex */
/*0x19*/ {0x5E, IC_LCE, 0xEA},
/*0x1a*/ {0x5E, IC_LCA, 0xE2},
/*0x1b*/ {0x5E, IC_LCI, 0xEE},
/*0x1c*/ {0x5E, IC_LCO, 0xF4},
/*0x1d*/ {0x5E, IC_LCU, 0xFB},
/*0x1e*/ {0x5E, IC_UCE, 0xCA},
/*0x1f*/ {0x5E, IC_UCA, 0xC2},
/*0x20*/ {0x5E, IC_UCI, 0xCE},
/*0x21*/ {0x5E, IC_UCO, 0xD4},
/*0x22*/ {0x5E, IC_UCU, 0xDB},

/*0x23*/ {0xA8, IC_SP , 0xA8},          /* diacresis (umlaut) */
/*0x24*/ {0xA8, IC_LCU, 0xFC},
/*0x25*/ {0xA8, IC_LCA, 0xE4},
/*0x26*/ {0xA8, IC_LCI, 0xEF},
/*0x27*/ {0xA8, IC_LCO, 0xF6},
/*0x28*/ {0xA8, IC_LCE, 0xEB},
/*0x29*/ {0xA8, IC_LCY, 0xFF},
/*0x2a*/ {0xA8, IC_UCU, 0xDC},
/*0x2b*/ {0xA8, IC_UCA, 0xC4},
/*0x2c*/ {0xA8, IC_UCI, 0xCF},
/*0x2d*/ {0xA8, IC_UCO, 0xD6},
/*0x2e*/ {0xA8, IC_UCE, 0xCB},

/*0x2f*/ {0x7E, IC_SP , 0x7E},          /* tilde */
/*0x30*/ {0x7E, IC_LCN, 0xF1},
/*0x31*/ {0x7E, IC_LCA, 0xE3},
/*0x32*/ {0x7E, IC_LCO, 0xF5},
/*0x33*/ {0x7E, IC_UCN, 0xD1},
/*0x34*/ {0x7E, IC_UCA, 0xC3},
/*0x35*/ {0x7E, IC_UCO, 0xD5},

/*0x36*/ {0xB0, IC_SP , 0xB0},          /* overcircle */
/*0x37*/ {0xB0, IC_LCA, 0xE5},
/*0x38*/ {0xB0, IC_UCA, 0xC5},

/*0x39*/ {0xB8, IC_SP , 0xB8},          /* cedilla */
/*0x3a*/ {0xB8, IC_LCC, 0xE7},
/*0x3b*/ {0xB8, IC_UCC, 0xC7}
        }
};

