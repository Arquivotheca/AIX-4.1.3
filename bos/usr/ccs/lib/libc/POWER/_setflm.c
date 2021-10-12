static char sccsid[] = "@(#)84	1.1  src/bos/usr/ccs/lib/libc/POWER/_setflm.c, libccnv, bos411, 9428A410j 12/13/90 20:32:22";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _setflm
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*
 * NAME: _setflm
 *                                                                    
 * FUNCTION: small function to give fortran access to the 
 *           C compiler's _setflm function.
 *
 * RETURNS: contents of FPSCR, in the same format as
 *          the C language _setflm compiler built-in.
 */

 double _setflm(double a)
   {
   double b;
    b = __setflm(a);
    return(b);
   } 
