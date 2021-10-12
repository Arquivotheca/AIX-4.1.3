static char sccsid[] = "@(#)04  1.2  src/bos/kernext/lft/swkbd/diac850.c, lftdd, bos411, 9428A410j 10/25/93 15:28:21";
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

struct diacritic diactbl850 = {
        {'i','b','m','-','8','5','0','\0'},     /*charset encoding*/
        0x3c,                                   /* number of entries in table*/
        0,
        {
/*0x00*/        {IC_ACUTE, IC_SP , IC_ACUTE},
/*0x01*/        {IC_ACUTE, IC_LCE, IC_LCEACC},
/*0x02*/        {IC_ACUTE, IC_LCA, IC_LCAACC},
/*0x03*/        {IC_ACUTE, IC_LCI, IC_LCIACC},
/*0x04*/        {IC_ACUTE, IC_LCO, IC_LCOACC},
/*0x05*/        {IC_ACUTE, IC_LCU, IC_LCUACC},
/*0x06*/        {IC_ACUTE, IC_LCY, IC_LCYACC},
/*0x07*/        {IC_ACUTE, IC_UCE, IC_UCEACC},
/*0x08*/        {IC_ACUTE, IC_UCA, IC_UCAACC},
/*0x09*/        {IC_ACUTE, IC_UCI, IC_UCIACC},
/*0x0a*/        {IC_ACUTE, IC_UCO, IC_UCOACC},
/*0x0b*/        {IC_ACUTE, IC_UCU, IC_UCUACC},
/*0x0c*/        {IC_ACUTE, IC_UCY, IC_UCYACC},

/*0x0d*/        {IC_LQUOT, IC_SP ,  IC_LQUOT},
/*0x0e*/        {IC_LQUOT, IC_LCE,  IC_LCEGRV},
/*0x0f*/        {IC_LQUOT, IC_LCA,  IC_LCAGRV},
/*0x10*/        {IC_LQUOT, IC_LCI,  IC_LCIGRV},
/*0x11*/        {IC_LQUOT, IC_LCO,  IC_LCOGRV},
/*0x12*/        {IC_LQUOT, IC_LCU,  IC_LCUGRV},
/*0x13*/        {IC_LQUOT, IC_UCE,  IC_UCEGRV},
/*0x14*/        {IC_LQUOT, IC_UCA,  IC_UCAGRV},
/*0x15*/        {IC_LQUOT, IC_UCI,  IC_UCIGRV},
/*0x16*/        {IC_LQUOT, IC_UCO,  IC_UCOGRV},
/*0x17*/        {IC_LQUOT, IC_UCU,  IC_UCUGRV},

/*0x18*/        {IC_AND, IC_SP ,  IC_AND},
/*0x19*/        {IC_AND, IC_LCE,  IC_LCECRF},
/*0x1a*/        {IC_AND, IC_LCA,  IC_LCACRF},
/*0x1b*/        {IC_AND, IC_LCI,  IC_LCICRF},
/*0x1c*/        {IC_AND, IC_LCO,  IC_LCOCRF},
/*0x1d*/        {IC_AND, IC_LCU,  IC_LCUCRF},
/*0x1e*/        {IC_AND, IC_UCE,  IC_UCECRF},
/*0x1f*/        {IC_AND, IC_UCA,  IC_UCACRF},
/*0x20*/        {IC_AND, IC_UCI,  IC_UCICRF},
/*0x21*/        {IC_AND, IC_UCO,  IC_UCOCRF},
/*0x22*/        {IC_AND, IC_UCU,  IC_UCUCRF},

/*0x23*/        {IC_UMLAUT, IC_SP ,  IC_UMLAUT},
/*0x24*/        {IC_UMLAUT, IC_LCU,  IC_LCUUML},
/*0x25*/        {IC_UMLAUT, IC_LCA,  IC_LCAUML},
/*0x26*/        {IC_UMLAUT, IC_LCI,  IC_LCIUML},
/*0x27*/        {IC_UMLAUT, IC_LCO,  IC_LCOUML},
/*0x28*/        {IC_UMLAUT, IC_LCE,  IC_LCEUML},
/*0x29*/        {IC_UMLAUT, IC_LCY,  IC_LCYUML},
/*0x2a*/        {IC_UMLAUT, IC_UCU,  IC_UCUUML},
/*0x2b*/        {IC_UMLAUT, IC_UCA,  IC_UCAUML},
/*0x2c*/        {IC_UMLAUT, IC_UCI,  IC_UCIUML},
/*0x2d*/        {IC_UMLAUT, IC_UCO,  IC_UCOUML},
/*0x2e*/        {IC_UMLAUT, IC_UCE,  IC_UCEUML},

/*0x2f*/        {IC_APPROX, IC_SP ,  IC_APPROX},
/*0x30*/        {IC_APPROX, IC_LCN,  IC_LCNTIL},
/*0x31*/        {IC_APPROX, IC_LCA,  IC_LCATIL},
/*0x32*/        {IC_APPROX, IC_LCO,  IC_LCOTIL},
/*0x33*/        {IC_APPROX, IC_UCN,  IC_UCNTIL},
/*0x34*/        {IC_APPROX, IC_UCA,  IC_UCATIL},
/*0x35*/        {IC_APPROX, IC_UCO,  IC_UCOTIL},

/*0x36*/        {IC_DEGREE, IC_SP ,  IC_DEGREE},
/*0x37*/        {IC_DEGREE, IC_LCA,  IC_LCAOVC},
/*0x38*/        {IC_DEGREE, IC_UCA,  IC_UCAOVC},

/*0x39*/        {IC_CEDILLA, IC_SP ,  IC_CEDILLA},
/*0x3a*/        {IC_CEDILLA, IC_LCC,  IC_LCCCED},
/*0x3b*/        {IC_CEDILLA, IC_UCC,  IC_UCCCED}
        }
};

