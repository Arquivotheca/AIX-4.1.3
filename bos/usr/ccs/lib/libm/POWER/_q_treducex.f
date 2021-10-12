* @(#)52	1.2  src/bos/usr/ccs/lib/libm/POWER/_q_treducex.f, libm, bos411, 9428A410j 9/21/93 11:56:43
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: _q_treducex
*
* ORIGINS: 55
*
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990, 1991

@process xflag(callby)

* NAME: _q_treduces
*                                                                    
* FUNCTION: argument reduction for quad precision trig
*           functions
*
* RETURNS: reduced quad precision argument

*******************************************************************
*	     PROGRAM: Quad-Precision SINE			  *
*		      Quad-Precision COSINE			  *
*	     AUTHOR:  ISQUARE, Inc (V. Markstein)		  *
*	     DATE:    4/3/90					  *
*	     MOD:     5/1/90 C --> FORTRAN			  *
*		      10/29/90 port --> RS/6000 		  *
*                     3/20/91 --> increase precision near         *
*                                 multiples of pi/2               *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990, 1991      *
*								  *
*		      The argument (dhead, dtail) is		  *
*			  real*16				  *
*								  *
*		      Results are returned by VALUE and by	  *
*		      means of the referenced parameters,	  *
*		      dleft (extra precision for reduced arg),	  *
*		      and M (quadrant where angle resides).	  *
*								  *
*******************************************************************

       function _q_treducex(%val(dHead),%val(dTail),dleft,M)
       implicit real*8		(a-h,o-y)
       implicit double complex	(z,_)
       integer*4		N(2),M
       logical			l1,l2,l3,l4,l5,l6,l7,l8,l9,l10

       data			pih  /z'3FF921FB54442D18'/   !pi/2 to
       data			pihb /z'3C91A62633145C07'/   !octuple precision
       data			pihc /z'B91F1976B7ED8FBC'/
       data			pihd /z'35B4CF98E804177D'/
       data                     pihe /z'32531D89CD9128A5'/
       data			big  /z'4338000000000000'/
       PARAMETER		(RPIH=0.6366197723675814D0)
       equivalence		(eN,N(1))
*******************************************************************************
*******************************************************************************
*       This argument reduction (mod pi/2) only uses 270 bits of pi/2.        *
*       For x,  such that log2(x)<107, expect argument precise to 107 bits    *
*                                                                             *
*       For 106 < log2(x) < 212, the number of good bits in the  reduced      *
*       argument is 213-log2(x)                                               *
*******************************************************************************
*******************************************************************************
	amid=0.0
	alow=0.0
        ainf=0.0
        easylimit = 0.0
	do while (abs(dHead) > easylimit)    !reduce until argument
           easylimit=0.5*pih
	   q=dHead*rpih 		     !isn't larger than easylimit
	   l10=(abs(q) < 2.0**54)	     !1st of many scheduling directives
	   l8=(abs(q) < 2.0**52)	     !Scheduling directive
	   l9=(q > 0)			     !Scheduling directive
	   if (abs(q) < 2.0**54) then	     !maybe q(mod 4) != 0
	      if (abs(q) < 2.0**52) then     !maybe q isn't even an integer
		 if (q > 0) then
		    q=q+2.0**52-2.0**52      !remove fraction if any exists
		 else
		    q=q-2.0**52+2.0**52
		 end if
	      end if			     !at this point, q is an integer
	      q1=q*0.25 		     !and determine q(mod 4)
	      if (q > 0) then		     !giving the quadrant in which
		 q1=q1+2.0**52-2.0**52	     !the angle resides.
	      else
		 q1=q1-2.0**52+2.0**52
	      end if
	      eN=q-4.0*q1+big		     !right adjust quadrant
	      M=M+N(2)
	   end if
*****************************************************************************
*   Tackling the subtraction of q*(pi/2) from the octuple precision word:   *
*		      (dHead, dTail, amid, alow)			    *
*****************************************************************************
	   high=dHead-q*pih		     !exact (Spectacular use of MAF)
	   b=q*pihb
	   blow=b-q*pihb		     !Yet another use of MAF
	   c=q*pihc
	   clow=c-q*pihc
	   d=q*pihd
	   dlow=d-q*pihd
           e=q*pihe
           elow=e-q*pihe
	   b1=dTail+high
	   l1=(abs(b1) > abs(b))	     !scheduling directive
	   l2=(abs(amid) > abs(blow))	     !scheduling directive
	   if (abs(dtail) > abs(high)) then  !sum and propagate
	      c1=dtail-b1+high		     !carries to the right
	   else
	      c1=high-b1+dtail
	   end if
	   dHead=b1-b
	   if (abs(b1) > abs(b)) then
	      c2=b1-dHead-b+c1
	   else
	      c2=b1-(dHead+b)+c1
	   end if
	   l3=(abs(c2) > abs(c))	     !scheduling directive
	   c3=amid+blow
	   if (abs(amid) > abs(blow)) then
	      d1=amid-c3+blow
	   else
	      d1=blow-c3+amid
	   end if
	   c4=c2-c
	   l4=(abs(c3) > abs(c4))	     !scheduling directive
	   l5=(abs(alow) > abs(clow))	     !scheduling directive
	   if (abs(c2) > abs(c)) then
	      d2=c2-c4 -c+d1
	   else
	      d2=c2-(c4+c)+d1
	   end if
	   dTail=c3+c4
	   if (abs(c3) > abs(c4)) then
	      d3=c3-dTail+c4+d2
	   else
	      d3=c4-dTail+c3+d2
	   end if
	   l6=(abs(d3) > abs(d))	     !scheduling directive
	   d4=alow+clow
	   if (abs(alow) > abs(clow)) then
	      e1=alow-d4+clow
	   else
	      e1=clow-d4+alow
	   end if
	   d5=d3-d
	   l7=(abs(d4) > abs(d5))	     !scheduling directive
           l4=(abs(ainf) > abs(dlow))
	   if (abs(d3) > abs(d)) then
	      e2=d3-d5-d+e1
	   else
	      e2=d3-(d5+d)+e1
	   end if
	   amid=d4+d5
	   if (abs(d4) > abs(d5)) then
	      e3=d4-amid+d5+e2
	   else
	      e3=d5-amid+d4+e2
	   end if
           l5=(abs(e3) > abs(e))
           e4=ainf+dlow
           if (abs(ainf) > abs(dlow)) then
              f4=ainf-e4+dlow+elow
           else
              f4=dlow-e4+ainf+elow
           end if
           e5=e3-e
           l6=(abs(e4) > abs(e5))
           if (abs(e3) > abs(e)) then
              f5=e3-e5-e+f4
           else
              f5=e3-(e5+e)+f4
           end if
           alow=e4+e5
           if (abs(e4) > abs(e5)) then
              ainf=e4-alow+e5+f5
           else
              ainf=e5-alow+e4+f5
           end if
        end do
        do while(dHead .eq. 0)
           if (abs(dHead)+abs(dTail)+abs(amid)+abs(alow)+abs(ainf)
     x          .eq. 0) go to 111
           dHead=dTail
           dTail=amid
           amid=alow
           alow=ainf
           ainf=0
	end do
111     continue
	high=dHead+dTail		     !At long last,
	carry=dHead-high+dTail		     !A final distillation
	dTail=carry+(amid+alow)
	dHead=high
        if (abs(carry) > abs(amid)) then
           dleft=carry-dTail+amid+alow+ainf
        else
           dleft=amid-dTail+carry+alow+ainf
        end if
	_q_treducex=dcmplx(dHead,dTail)
	return
	end
