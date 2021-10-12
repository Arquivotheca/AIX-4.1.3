#!/bin/ksh
# @(#)21        1.1  src/bos/usr/lib/pios/bullps.header.sh, cmdpios, bos411, 9428A410j 4/28/94 19:25:36
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
# PostScript header page generator
#
#
COLUMNS=$1
ARG1=$PIOJOBNUM
ARG2=$PIOTO
CMD0=$PIOQNAME

if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi


echo "%!
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

cr

crlf
crlf
crlf
crlf"

$banner "$ARG2" | sed -e 's/^/(/' -e 's/$/) show crlf/'

echo "crlf"
echo "(Request id: $ARG1    Printer: $CMD0 ) show crlf"
echo "crlf"
echo "(`date` ) show crlf"



echo "showpage"

exit 0
