static char sccsid[] = "@(#)09  1.2  src/bos/kernext/lft/swkbd/diac88597.c, lftdd, bos411, 9428A410j 10/25/93 15:29:27";
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


#define LFNUMDIACRITIC  0x17

#include <lftcode.h>
#include <kks.h>                        /* structure definitions */

/*-----------
   FUNCTION: Composition table for dead keys

   This table is set up so that the first entry of each diacritic "range"
   is the space character, which will result in an output of the
   diacritic itself.
------------*/

struct diacritic diactbl88597 = {
        {'8','8','5','9','-','7','\0','\0'},    /*charset encoding*/
        0x17,                                   /* number of entries in table*/
        0,
        {
/*0x00*/ {0xA8, 0x20, 0xA8},          /* umlaut */
/*0x01*/ {0xA8, 0xC9, 0xDA},
/*0x02*/ {0xA8, 0xE9, 0xFA},
/*0x03*/ {0xA8, 0xD5, 0xDB},
/*0x04*/ {0xA8, 0xF5, 0xFB},

/*0x05*/ {0xB4, 0x20, 0xB4},
/*0x06*/ {0xB4, 0xC1, 0xB6},
/*0x07*/ {0xB4, 0xC5, 0xB8},
/*0x08*/ {0xB4, 0xC7, 0xB9},
/*0x09*/ {0xB4, 0xC9, 0xBA},
/*0x0A*/ {0xB4, 0xCF, 0xBC},
/*0x0B*/ {0xB4, 0xD5, 0xBE},
/*0x0C*/ {0xB4, 0xD9, 0xBF},
/*0x0D*/ {0xB4, 0xE1, 0xDC},
/*0x0E*/ {0xB4, 0xE5, 0xDD},
/*0x0F*/ {0xB4, 0xE7, 0xDE},
/*0x10*/ {0xB4, 0xE9, 0xDF},
/*0x11*/ {0xB4, 0xEF, 0xFC},
/*0x12*/ {0xB4, 0xF5, 0xFD},
/*0x13*/ {0xB4, 0xF9, 0xFE},

/*0x14*/ {0xB5, 0x20, 0xB5},
/*0x15*/ {0xB5, 0xE9, 0xC0},
/*0x16*/ {0xB5, 0xF5, 0xE0},

        }
};

