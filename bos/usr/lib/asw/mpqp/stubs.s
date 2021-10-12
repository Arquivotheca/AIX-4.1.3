	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: All temporary
;
;  ORIGINS: 27
;
;  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
;  combined with the aggregated modules for this product)
;                   SOURCE MATERIALS
;  (C) COPYRIGHT International Business Machines Corp. 1988, 1989
;  All Rights Reserved
;
;  US Government Users Restricted Rights - Use, duplication or
;  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;
;  FUNCTION: Any ASW function which remains unwritten but is called
;	     is probably here.  Certain error logging and trace mech-
;	     anisms are examples.
;
;**********************************************************************
;
	TITLE	stubs
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)00	1.3  src/bos/usr/lib/asw/mpqp/stubs.s, ucodmpqp, bos411, 9428A410j 5/17/94 09:00:12'
	db	0
	PAGE
	
	public	_f_txproc

_f_txproc	proc	near
		ret
_f_txproc	endp

	public	not_used,signal,_null_fn

not_used proc	near
	 ret
not_used endp
_null_fn proc	near
	 ret
_null_fn endp
signal	proc	near
	ret
signal	endp

	public	_log_error
_log_error proc near
	   ret
_log_error endp

	public	__acrtused
__acrtused proc	near
	xor	ax,ax
	ret
__acrtused endp

_TEXT	ends
	end
