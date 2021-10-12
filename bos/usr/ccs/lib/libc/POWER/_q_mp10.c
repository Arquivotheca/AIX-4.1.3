static char sccsid[] = "@(#)76	1.1  src/bos/usr/ccs/lib/libc/POWER/_q_mp10.c, libccnv, bos411, 9428A410j 12/13/90 19:59:03";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _q_mp10, _q_mp10a, _q_pow10
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/* Copyright(c) ISquare - 1990 */

/****************************************************************************/
/*                                                                          */
/*   Conversion routine:  Auxilliary function mp10                          */
/*                                                                          */
/*   _q_mp10(int ndx, double a, double b, double c, double result[3])       */
/*                                                                          */
/*        ndx gives the power of 10 to be multiplied by the triple word     */
/*          number represented by (a, b, c).                                */
/*        result is the triple word array where the high order part of the  */
/*          product is returned                                             */
/*                                                                          */
/***************************************************************************/

#include <float.h>
#define RS6000
/*Note: to compile for a machine other than the RS/6000, undefine the
  above RS6000 variable.  You must then provide routines to carry out
  the maf and msf subroutines to perform the same functions as the
  corresponding RS6000 macros which follow below.
*/

/*macro to multiply two numbers and add a third with only one rounding error*/
#ifdef RS6000
#define maf(i,j,k)  i*j + k
#define msf(i,j,k)  i*j - k

#else
 double maf(double i, double j, double k);
 double msf(double i, double j, double k);
#endif
/* macro to multiply (a,b,c) by working precision number extracted
   from the i-th position of a table.
*/
#define M31(tbl, i)  ptbl = ((double *)tbl + 3 * (i)); f1 = *ptbl; \
   u = c*f1; \
   x=b*f1; y=msf(b, f1, x);   \
   c = y + u; \
   z = a*f1; w = msf(a,f1,z); u = x; (__fabs(w) > __fabs(u)); \
   b = u + w; c+= (__fabs(w) > __fabs(u)) ? ((w-b)+u):((u-b)+w); a=z;

/* macro to multiply (a,0.0,0.0) by working precision number extracted
   from the i-th position of a table.
*/
#define M11(tbl, i) ptbl = ((double *)tbl + 3*(i)); f1 = *ptbl; \
   z = a*f1; b = msf(a,f1,z); c = 0.0; a = z;

/* macro to multiply (a,b,c) by triple working precision number extracted
   from the i-th position of a table.
*/
#define M33(tbl, i) ptbl = ((double *)tbl + 3*i); f1 = *ptbl; \
   f2 = *(ptbl + 1); f3 = *(ptbl + 2); u = a*f3 + b*f2 + c*f1; \
   v = a*f2; w = msf(a, f2, v); x=b*f1; y=msf(b, f1, x);   \
   (__fabs(x) > __fabs(v)); c = w + y + u; \
   z = a*f1; W = msf(a,f1,z); U = v + x; (__fabs(W) > __fabs(U)); \
   c += (__fabs(x) > __fabs(v))?((x-U)+v):((v-U)+x); \
   b = U + W; c+= (__fabs(W) > __fabs(U)) ? ((W-b)+U):((U-b)+W); a=z;


/* macro to multiply (a,0.0,0.0) by triple working precision number extracted
   from the i-th position of a table.
*/
#define M13(tbl, i) ptbl = ((double *)tbl + 3*i); f1 = *ptbl; \
   f2 = *(ptbl + 1); f3 = *(ptbl + 2); u = a*f3; \
   v = a*f2; w = msf(a,f2,v); c = w + u; \
   z = a*f1; w = msf(a,f1,z); b = w + v; a = z;\
   c += (__fabs(w) > __fabs(v))? ((w-b)+v): ((v-b)+w);


   static int _q_ip10[40][6] = {     /*positive powers of ten*/
 0x3FF00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   0*/
 0x40240000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   1*/
 0x40590000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   2*/
 0x408F4000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   3*/
 0x40C38800, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   4*/
 0x40F86A00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   5*/
 0x412E8480, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   6*/
 0x416312D0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   7*/
 0x4197D784, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   8*/
 0x41CDCD65, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   9*/
 0x4202A05F, 0x20000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  10*/
 0x42374876, 0xE8000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  11*/
 0x426D1A94, 0xA2000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  12*/
 0x42A2309C, 0xE5400000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  13*/
 0x42D6BCC4, 0x1E900000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  14*/
 0x430C6BF5, 0x26340000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  15*/
 0x4341C379, 0x37E08000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  16*/
 0x43763457, 0x85D8A000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  17*/
 0x43ABC16D, 0x674EC800, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  18*/
 0x43E158E4, 0x60913D00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  19*/
 0x4415AF1D, 0x78B58C40, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  20*/
 0x444B1AE4, 0xD6E2EF50, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  21*/
 0x4480F0CF, 0x064DD592, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*  22*/
 0x44B52D02, 0xC7E14AF6, 0x41600000, 0x00000000, 0x00000000, 0x00000000, /*  23*/
 0x44EA7843, 0x79D99DB4, 0x41700000, 0x00000000, 0x00000000, 0x00000000, /*  24*/
 0x45208B2A, 0x2C280290, 0x41D28000, 0x00000000, 0x00000000, 0x00000000, /*  25*/
 0x4554ADF4, 0xB7320334, 0x42072000, 0x00000000, 0x00000000, 0x00000000, /*  26*/
 0x4589D971, 0xE4FE8401, 0x423CE800, 0x00000000, 0x00000000, 0x00000000, /*  27*/
 0x45C027E7, 0x2F1F1281, 0x42584400, 0x00000000, 0x00000000, 0x00000000, /*  28*/
 0x45F431E0, 0xFAE6D721, 0x429F2A80, 0x00000000, 0x00000000, 0x00000000, /*  29*/
 0x46293E59, 0x39A08CE9, 0x42DB7A90, 0x00000000, 0x00000000, 0x00000000, /*  30*/
 0x465F8DEF, 0x8808B024, 0x42F4B268, 0x00000000, 0x00000000, 0x00000000, /*  31*/
 0x4693B8B5, 0xB5056E16, 0x434677C0, 0x80000000, 0x00000000, 0x00000000, /*  32*/
 0x4D384F03, 0xE93FF9F4, 0x49EB54F2, 0xFDADC71D, 0x469592FD, 0xA87C0400, /*  64*/
 0x53DDF675, 0x62D8B362, 0x50828B17, 0x3FA53BCF, 0x4D35080F, 0x1C46D01B, /*  96*/
 0x5A827748, 0xF9301D31, 0x57337F19, 0xBCCDB0DA, 0x53E8809B, 0x811A79FF, /* 128*/
 0x6126C2D4, 0x256FFCC2, 0x5DDEA95D, 0xEE61ACC5, 0x5A6AC012, 0x47039F0B, /* 160*/
 0x67CC0E1E, 0xF1A724EA, 0x6475AA16, 0xEF894FD1, 0x612D8A0B, 0x8788C514, /* 192*/
 0x6E714A52, 0xDFFC6799, 0x6B02F82B, 0xD6B70D99, 0x67B554DF, 0x78218B8C, /* 224*/
 0x75154FDD, 0x7F73BF3B, 0x71CA3776, 0xEE406E63, 0x6E7F755B, 0xC28F2660} /* 256*/;
   static int _q_inp10[40][6] = {    /*negative powers of ten*/
 0x3FF00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, /*   0*/
 0x3FB99999, 0x99999999, 0x3C633333, 0x33333333, 0x38F99999, 0x9999999A, /*  -1*/
 0x3F847AE1, 0x47AE147A, 0x3C3C28F5, 0xC28F5C28, 0x38EEB851, 0xEB851EB8, /*  -2*/
 0x3F50624D, 0xD2F1A9FB, 0x3C0CED91, 0x6872B020, 0x38B89374, 0xBC6A7EFA, /*  -3*/
 0x3F1A36E2, 0xEB1C432C, 0x3BC4AF4F, 0x0D844D01, 0x385D4951, 0x82A9930C, /*  -4*/
 0x3EE4F8B5, 0x88E368F0, 0x3B908C3F, 0x3E0370CD, 0x38490EA9, 0xE6EEB702, /*  -5*/
 0x3EB0C6F7, 0xA0B5ED8D, 0x3B4B5A63, 0xF9A49C2C, 0x37CB10FD, 0x7E45803D, /*  -6*/
 0x3E7AD7F2, 0x9ABCAF48, 0x3B15E1E9, 0x9483B023, 0x37B23699, 0x194119A6, /*  -7*/
 0x3E45798E, 0xE2308C39, 0x3AFBF3F7, 0x0834ACDA, 0x37AD3E1E, 0x9EA69EBB, /*  -8*/
 0x3E112E0B, 0xE826D694, 0x3AC65CC5, 0xA02A23E2, 0x37653030, 0xFDD7645E, /*  -9*/
 0x3DDB7CDF, 0xD9D7BDBA, 0x3A86FAD5, 0xCD10396A, 0x37109A36, 0x5F7E0DFA, /* -10*/
 0x3DA5FD7F, 0xE1796495, 0x3A47F7BC, 0x7B4D28A9, 0x36F9D748, 0xF2FF38CA, /* -11*/
 0x3D719799, 0x812DEA11, 0x39F97F27, 0xF0F6E885, 0x36A9174F, 0xD663E8EE, /* -12*/
 0x3D3C25C2, 0x68497681, 0x39E84CA1, 0x9697C81A, 0x369837DC, 0xC47A61C9, /* -13*/
 0x3D06849B, 0x86A12B9B, 0x394EA709, 0x09833DE7, 0x35C928DB, 0x2138E9FA, /* -14*/
 0x3CD203AF, 0x9EE75615, 0x3983643E, 0x74DC052F, 0x363B0508, 0x2BD371C8, /* -15*/
 0x3C9CD2B2, 0x97D889BC, 0x3925B4C2, 0xEBE68798, 0x35D35367, 0x7EE2D836, /* -16*/
 0x3C670EF5, 0x4646D496, 0x39112426, 0xFBFAE7EB, 0x35B487C2, 0xFF8DF015, /* -17*/
 0x3C32725D, 0xD1D243AB, 0x38E41CEB, 0xFCC8B989, 0x355CFE79, 0x96BF9A23, /* -18*/
 0x3BFD83C9, 0x4FB6D2AC, 0x388A52B3, 0x1E9E3D06, 0x353865CA, 0x3C4CA40E, /* -19*/
 0x3BC79CA1, 0x0C924223, 0x38675447, 0xA5D8E535, 0x351CF584, 0x181EA806, /* -20*/
 0x3B92E3B4, 0x0A0E9B4F, 0x383F769F, 0xB7E0B75E, 0x34D4BC06, 0x8CFDD9A3, /* -21*/
 0x3B5E3920, 0x10175EE5, 0x3802C54C, 0x931A2C4B, 0x34AD6338, 0x70CB1482, /* -22*/
 0x3B282DB3, 0x4012B251, 0x37C13BAD, 0xB829E078, 0x34778293, 0x8D6F439B, /* -23*/
 0x3AF357C2, 0x99A88EA7, 0x379A9624, 0x9354B393, 0x34493542, 0xD78C3616, /* -24*/
 0x3ABEF2D0, 0xF5DA7DD8, 0x376544EA, 0x0F76F60F, 0x341A9102, 0x4609C4DE, /* -25*/
 0x3A88C240, 0xC4AECB13, 0x37376A54, 0xD92BF80C, 0x33E540CE, 0x9E6E3718, /* -26*/
 0x3A53CE9A, 0x36F23C0F, 0x370921DD, 0x7A89933D, 0x33A5347D, 0xCA49F1C0, /* -27*/
 0x3A1FB0F6, 0xBE506019, 0x36B06C5E, 0x54EB70C4, 0x3350A7F8, 0xEDB96C01, /* -28*/
 0x39E95A5E, 0xFEA6B347, 0x3689F04B, 0x7722C09D, 0x32D0CC17, 0xC5BE001A, /* -29*/
 0x39B4484B, 0xFEEBC29F, 0x3660C684, 0x960DE6A5, 0x32FA051A, 0x31BE599A, /* -30*/
 0x398039D6, 0x6589687F, 0x3633D203, 0xAB3E521D, 0x32E8676B, 0xA38C7852, /* -31*/
 0x3949F623, 0xD5A8A732, 0x35F2E99F, 0x7863B696, 0x3254AF20, 0xB5B1AA03, /* -32*/
 0x32A50FFD, 0x44F4A73D, 0x2F3A53F2, 0x398D747B, 0x2BCB1121, 0x50FF7207, /* -64*/
 0x2C011680, 0x5EFFAEAA, 0x28ACD88E, 0xDE5810C7, 0x25476828, 0xD2181B64, /* -96*/
 0x255BBA08, 0xCF8C979C, 0x220282B1, 0xF2CFDB41, 0x1EADDBDE, 0xE26CA606, /*-128*/
 0x1EB67E9C, 0x127B6E74, 0x1B326B3D, 0xA42CECAD, 0x17C0F568, 0xFE5B452E, /*-160*/
 0x18123FF0, 0x6EEA8479, 0x14C019ED, 0x8C1A8D18, 0x1173EEE7, 0x8E2D2049, /*-192*/
 0x116D9CA7, 0x9D894629, 0x0E1AF693, 0xE2FD58D4, 0x0AC23214, 0x2304D62E, /*-224*/
 0x0AC80628, 0x64AC6F43, 0x07539FA9, 0x11155FEF, 0x0406A611, 0x447C5DA5} /*-256*/;

/*
 * NAME: _q_mp10
 *                                                                    
 * FUNCTION: 
 *
 */

_q_mp10(int ndx, double a, double b, double c, double result[3])
  {
    double u, v, w, x, y, z, U, W, f1, f2, f3;
    double *ptbl, *p;
    int    i, j;
    (ndx > 319);
    (ndx < -383);
    if (ndx == 0)
      {
        result[0] = a;         /*easy case --  10^0 = 1.0 */
        result[1] = b;
        result[2] = c;
        return;
      }
    if (ndx > 319)ndx = 319;
    else if(ndx < -383) ndx = -383;
    j = ndx;
    if (ndx > 0) p = ((double *)&_q_ip10);
    else {p = ((double *)&_q_inp10); j = -j; }
    i = j & 0x1f;                 /*get index into lower part of table*/
    (ndx < 23);    /*"compiler directive"*/
    if (i)
       if ((ndx > 0) && (ndx < 23)) {M31(p, (i));} else {M33(p, (i));}
    if (j&0x100)      /*treat power of 256 as a special case*/
      {
        M33(p,(39));
        j -= 256;   /*and remove 256 from the exponent*/
      }
    if (i = ((j & 0x1e0)>>5))
      {
         M33(p,(i+31));
      }
    result[0] = a + b;
    u = (a - result[0]) +b;
    result[1] = u + c;
    result[2] = (u - result[1]) + c;
    return;
  }

/*
 * NAME: _q_mp10a
 *                                                                    
 * FUNCTION: 
 *
 */

_q_mp10a(int ndx, double a, double result[3])
  {
    double u, v, w, x, y, z, U, W, f1, f2, f3, b, c;
    double *ptbl, *p;
    int    i, j;
    (ndx > 319);
    (ndx < -383);
    if (ndx == 0)
      {
        result[0] = a;         /*easy case --  10^0 = 1.0 */
        result[1] = 0.0;
        result[2] = 0.0;
        return;
      }
    if (ndx > 319)ndx = 319;
    else if(ndx < -383) ndx = -383;
    j = ndx;
    if (ndx > 0) p = ((double *)&_q_ip10);
    else {p = ((double *)&_q_inp10); j = -j;}
    i = j & 0x1f;                 /*get index into lower part of table*/
    (ndx < 23);    /*"compiler directive"*/
    if (i)
      if ((ndx > 0) && (ndx < 23)) {M11(p, (i));} else {M13(p, (i));}
    else b = (c = 0.0);
    if (j&0x100)
      {                 /*treat 10^256 as a special case*/
        M33(p,(39));
        j -= 256;       /*and remove 256 from the exponent*/
      }
    if (i = ((j & 0x1e0)>>5))
      {
         M33(p,(i+31));
      }
    result[0] = a + b;
    u = (a - result[0]) +b;
    result[1] = u + c;
    result[2] = (u - result[1]) + c;
    return;
  }
/*
 * NAME: _q_pow10
 *                                                                    
 * FUNCTION: compute 10**x into sextuple precision number result
 *           using power of ten table
 *
 */

_q_pow10(int ndx, double result[3])    /*routine loads into vector result,     */
                                      /* 10**ndx                              */

{
  int i, j;
  double *p, a, b, c;

  j = ndx;
  if (ndx > 0) p = ((double *)&_q_ip10);
  else {p = ((double *)&_q_inp10); j = -j;}
  i = j & 0xf;                        /*low order bits of exponent            */
  a = *(p + 3*i);
  b = *(p + 3*i + 1);
  c = *(p + 3*i + 2);
  _q_mp10(ndx & 0xfffffff0, a, b, c, result);
}
