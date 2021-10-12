/* @(#)04	1.2 6/4/91 10:08:20 */

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcalp.t
 *
 * DESCRIPTIVE NAME:
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/

static unsigned short dbalph[96] = {
/*                 0      1      2      3      4      5      6      7   */
/*                 8      9      A      B      C      D      E      F   */
/*                        !      "      #      $      %      &      '   */
/*  20  */    0x0000,0x8149,0x818d,0x8194,0x8190,0x8193,0x8195,0x818c,
/*                 (      )      *      +      ,      -      .      /   */
/*  28  */    0x8169,0x816a,0x8196,0x817b,0x8143,0x815c,0x8144,0x815e,
/*                 0      1      2      3      4      5      6      7   */
/*  30  */    0x824f,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,
/*                 8      9      :      ;      <      =      >      ?   */
/*  38  */    0x8257,0x8258,0x8246,0x8247,0x8171,0x8181,0x8172,0x8148,
/*                 @      A      B      C      D      E      F      G   */
/*  40  */    0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,
/*                 H      I      J      K      L      M      N      O   */
/*  48  */    0x8267,0x8268,0x8269,0x826a,0x826b,0x826c,0x826d,0x826e,
/*                 P      Q      R      S      T      U      V      W   */
/*  50  */    0x826f,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,
/*                 X      Y      Z      [      \      ]      ^      _   */
/*  58  */    0x8277,0x8278,0x8279,0x816d,0x818f,0x816e,0x814f,0x8151,
/*                 `      a      b      c      d      e      f      g   */
/*  60  */    0x814d,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,
/*                 h      i      j      k      l      m      n      o   */
/*  68  */    0x8288,0x8289,0x828a,0x828b,0x828c,0x828d,0x828e,0x828f,
/*                 p      q      r      s      t      u      v      w   */
/*  70  */    0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,
/*                 x      y      z      {      |      }      ~          */
/*  78  */    0x8298,0x8299,0x829a,0x816f,0x8162,0x8170,0x8150
};
