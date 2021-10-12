/* @(#)63       1.3  src/bldenv/bdftosnf/misc.h, sysxdisp, bos412, GOLDA411a 2/21/94 15:26:22 */
/*
 *   COMPONENT_NAME: sysxdisp
 *
 *   FUNCTIONS: GLYPHWIDTHBYTES
 *		GLYPHWIDTHBYTESPADDED
 *		GLYPHWIDTHPIXELS
 *		PADGLYPHWIDTHBYTES
 *		n1dChars
 *		n2dChars
 *
 *   ORIGINS: 27,18,40,42,16
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1987,1990
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1989, OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
 * (c) Copyright 1987, 1988, 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY
 */
#ifndef MISC_H
#define MISC_H

typedef int Bool;
typedef int BOOL;
typedef int INT32;
typedef unsigned int CARD32;

#define FontLeftToRight 0
#define FontRightToLeft 1

#define LSBFirst 0
#define MSBFirst 1

#define FALSE 0
#define TRUE  1

#define MAXSHORT 32767
#define MINSHORT (-MAXSHORT)

#define n1dChars(pfi) ((pfi)->lastCol - (pfi)->firstCol + 1)

#define n2dChars(pfi)   (((pfi)->lastCol - (pfi)->firstCol + 1) * \
                         ((pfi)->lastRow - (pfi)->firstRow + 1))

#define GLYPHWIDTHBYTES(pci)    (((GLYPHWIDTHPIXELS(pci))+7) >> 3)
#define GLYPHWIDTHPIXELS(pci)   (pci->rightSideBearing \
                                    - pci->leftSideBearing)

#define GLYPHWIDTHBYTESPADDED(pci)      (GLYPHWIDTHBYTES(pci))
#define PADGLYPHWIDTHBYTES(w)           (((w)+7)>>3)

#endif  MISC_H
