# @(#)20        1.4.1.2  src/bos/usr/lib/pios/H.ps.S, cmdpios, bos411, 9428A410j 3/3/93 17:23:06
#
# COMPONENT_NAME: (cmdpios) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
%!
/LM 20 def
/ypos 725 def
/lineheight 12 def
/linewidth  546 def
/cr   { LM ypos moveto } def
/crlf { ypos lineheight sub
	/ypos exch def
	cr } def

0 rotate

/Courier findfont 12 scalefont setfont

/wrap_lines
  { /str exch def
    /strlen str length def  
    { str stringwidth pop linewidth gt
      { /lastchar 1 def /lastspace 0 def
	{
	  str 0 lastchar getinterval stringwidth pop linewidth gt
	  {
            str lastspace 1 add strlen lastspace sub 1 sub getinterval
            ( ) search { exch pop exch pop } if 
            stringwidth pop linewidth gt { /lastspace lastchar def } if
	    lastspace 0 ne { /lastchar lastspace def } if
	    str 0 lastchar getinterval show crlf
	    lastspace 0 ne { /lastchar lastchar 1 add def } if
	    /str str lastchar strlen lastchar sub getinterval def
	    /strlen str length def
	    exit
	  }
	  {
	    str lastchar 1 getinterval dup
	    ( ) eq exch (/008) eq or 
	    { /lastspace lastchar def } if
	    /lastchar lastchar 1 add def
	  } ifelse
	} loop 
      }
      { str show crlf exit } ifelse
    } loop
  } def

cr

(*############################################################################*) show crlf
(*############################################################################*) show crlf
(*############################################################################*) show crlf
(*############################################################################*) show crlf
crlf
crlf
crlf
crlf
(******************************************************************************) show crlf
(******************************************************************************) show crlf
crlf
crlf
(%t  %T ) wrap_lines 
crlf
(%p  %P ) wrap_lines 
crlf
(%q  %Q ) wrap_lines 
crlf
(%h  %H ) wrap_lines 
crlf
(%s  %S ) wrap_lines 
crlf
(%d  =====> %D <===== ) wrap_lines 
crlf
(******************************************************************************) show crlf
(******************************************************************************) show crlf
crlf
crlf
(%a ) wrap_lines 
(%A ) wrap_lines 
crlf
crlf
(******************************************************************************) show crlf
(******************************************************************************) show crlf
showpage
