#!/bin/ksh
# @(#)22        1.1  src/bos/usr/lib/pios/bullps.trailer.sh, cmdpios, bos411, 9428A410j 4/28/94 19:25:43
#
#   COMPONENT_NAME: CMDPIOS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,61
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# BULL SOURCE FILE
#
#
# produces the "END banner for the trailer for Bull print outs
#
if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi


echo "%!
/LM 20 def
/ypos 200 def
/lineheight 12 def
/linewidth  546 def
/cr   { LM ypos moveto } def
/crlf { ypos lineheight sub
	/ypos exch def
	cr } def

0 rotate

/Courier findfont 12 scalefont setfont

cr
crlf
crlf
crlf
crlf"
$banner "END" | sed -e 's/^/(/' -e 's/$/) show crlf/'

echo "showpage"

exit 0
