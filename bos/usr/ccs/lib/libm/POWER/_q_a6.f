* @(#)61	1.1  src/bos/usr/ccs/lib/libm/POWER/_q_a6.f, libm, bos411, 9428A410j 1/9/91 07:52:55
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _q_a6
*
* ORIGINS: 55
*
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990

@process xflag(callby)

* NAME: _q_a6
*                                                                    
* FUNCTION: Sextuple-precision addition
*
* RETURNS: high order parts of result are returned
*          explicity as a quad precision number.  low
*          order part is returned in argument 'bottom'.

*******************************************************************
*	     PROGRAM: Sextuple-Precision Addition		  *
*	     AUTHOR:  ISQUARE, Inc (V. Markstein)		  *
*	     DATE:    5/4/90					  *
*	     MOD:     6/18/90 C --> FORTRAN			  *
*		      11/27/90 port --> RS/6000 		  *
*	     NOTICE:  Copyright (c) ISQUARE, Inc. 1990		  *
*								  *
*		      Results are returned by VALUE, and the	  *
*		      last parameter				  *
*								  *
*******************************************************************

      double complex function	_q_a6(%val(a),%val(b),%val(c),
     +				%val(d),%val(e),%val(f),bottom)

      implicit real*8		(a-h,o-z)
      implicit double complex	(_)
      implicit logical*4	(l)

***************************************************************************
*     This routine adds the sextuple precision values (a,b,c) and (d,e,f) *
*     using a distillation procedure.					  *
***************************************************************************

      l1=(abs(a) > abs(d))		!All assignments to logical vars
      l2=(abs(b) > abs(e))		!are scheduling directives.  They can be
      l3=(abs(c) > abs(f))		!removed without changing the semantics
      p=a+d				!of the program, but the execution time
      q=b+e				!could increase significantly.
      r=c+f				!**************** ALIGNMENT MAP ->
      if (abs(a) > abs(d)) then 	!a    b      c
	 s=a-p+d			! d    e      f
      else				!p    s 	  (replaces a and d
	 s=d-p+a
      end if
      if (abs(b) > abs(e)) then
	 t=b-q+e			!     q       t   (replaces b and e
      else
	 t=e-q+b
      end if
      l4=(abs(s) > abs(q))
      l5=(abs(t) > abs(r))
      if (abs(c) > abs(f)) then
	 u=c-r+f			!	      r    u  (replaces c and f
      else
	 u=f-r+c
      end if
      v=s+q
      x=t+r
      l6=(abs(p) > abs(v))
      if (abs(s) > abs(q)) then
	 w=s-v+q			!     v       w     (replaces s and q
      else
	 w=q-v+s
      end if
      l7=(abs(x) > abs(w))
      if (abs(t) > abs(r)) then
	 y=t-x+r			!	      x    y  (replaces t and r
      else
	 y=r-x+t
      end if
      z=x+w
      res=p+v
      if (abs(p) > abs(v)) then
	 carry=p-res+v			!res	carry	     (replaces p and v
      else
	 carry=v-res+p
      end if
      l8=(abs(carry) > abs(z))
      if (abs(x) > abs(w)) then
	 z1=x-z+w			!	       z  z1  (replaces x and w
      else
	 z1=w-z+x
      end if
      z2=u+y+z1 			!		  z2  (replaces u, y, and z1
      resmid=carry+z
      if (abs(carry) > abs(z)) then
	 reslow=carry-resmid+z		!    resmid carry2  (replaces carry, z
      else
	 reslow=z-resmid+carry
      end if

***************************************************************************
*     Here the sum is represented by (reshi, resmid, reslow), but	  *
*     the computation of resmid may cause it to be more than 1/2 ulp	  *
*     of reshi, so one last distillation is performed.			  *
***************************************************************************

      top=res+resmid
      rmid=(res-top+resmid)+reslow
      bottom=(res-top+resmid)-rmid+reslow+z2
      _q_a6=dcmplx (top, rmid)
      return
      end
