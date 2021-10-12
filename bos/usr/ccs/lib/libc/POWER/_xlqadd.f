* @(#)70	1.1  src/bos/usr/ccs/lib/libc/POWER/_xlqadd.f, libccnv, bos411, 9428A410j 12/13/90 19:55:49
*
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _xlqadd
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1990
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

@process mixed debug(callby)

* NAME: _xlqadd
*                                                                    
* FUNCTION: add two quad precision numbers
*
* RETURNS: a quad precision number containing the sum
*
* NOTE:  Variable names are case-sensitive, must have
*        the @process mixed to work right.

        double complex function _xlqadd(%val(X),%val(x),%val(Y),%val(y))
        implicit real*8 (A-H,O-Z)
        logical i, j
C       Computes (Z,z) = (X,x) + (Y,y), where
C       the notation (p,q) denotes a quad precision number, in which
C       p is the high order word (as an IEEE double precision word),
C       and q is the low order word (as an IEEE double precision word.)
C       The fundamental operation is to combine two double precision words
C       to form a quad precision word.  The underlying operation to compute
C         (A,a) = x + y is as follows (in C-style coding):
C            A = x + y;
C            a = (abs(x) > abs(y)? x - A + y : y - A + x;
C         To carry out (Z,z) = (X,x) + (Y,y) do the following:
C            (S,s) = X + Y  (as in the above description)
C            (T,t) = x + y  (as in the above description)
C            (H,h) = S + (s + T)  (as in the above description)
C            (Z,z) = H + (t + h)  (as in the above description)
C         It is believed that the last two operations can be performed
C         more simply with only conventional double precision arithmetic
C         (i.e. not testins which of S and s+T is the larger).
C         In the code that follows below, in order to recover some of the
C         time due to branching, computations are performed speculatively.
C         For example sx represents the value of s in the event that X > Y,
C         and sy represents the value of s otherwise.  Likewise tx represents
C         t when x > y, and ty represents t otherwise.


        i = (abs(X) .gt. abs(Y))            !Clue to the scheduler
        S = X + Y                           !Begin computation of (S,s)
        j = (abs(x) .gt. abs(y))            !Scheduling clue
        sx = X - S + Y                      !Computation of s as described
        sy = Y - S + X                      !above
C        (S, sx) = X + Y when abs(X) > abs(Y)
C        (S, sy) = X + Y when abs(X) <= abs(Y)
        T = x + y                           !Begin computation of (T,t)
        tx = x - T + y                      !Computation of t as described
        ty = y - T + x                      !above.
        if (abs(X) .gt. abs(Y)) then
           ht = sx + T                      !Path where |X|>|Y|. Use sx for s
           H = S + ht
           h = S - H + ht                   !Computation of (H,h) = S + (s + T)
           if (abs(x) .gt. abs(y)) then
              zt = tx + h                   !Path where |x|>|y|.  Use tx for t
              Z = H + zt
              z = H - Z + zt                !Computation of (Z,z) = H + (t + h)
           else
              zt = ty + h                   !Path where |x|<=|y|. Use ty for t
              Z = H + zt
              z = H - Z + zt                !Computation of (Z,z) = H + (t + h)
           endif
        else
          ht = sy + T                       !Path where |X|<=|Y|. Use sy for s
          H = S + ht
          h = S - H + ht                    !Computation of (H,h) = S + (s + T)
          if (abs(x) .gt. abs(y)) then
             zt = tx + h                    !Path where |x|>|y|.  Use tx for t
             Z = H + zt
             z = H - Z + zt                 !Computation of (Z,z) = H + (t + h)
          else
             zt = ty + h                    !Path where |x|<=|y|.  Use ty for t
             Z = H + zt
             z = H - Z + zt                 !Computation of (Z,z) = H + (t + h)
          endif
       endif
       _xlqadd = dcmplx(Z,z)                !Combine high and low order parts
       return                               !into one 128-bit entity which is
       end                                  !returned in fl. pt. registers
