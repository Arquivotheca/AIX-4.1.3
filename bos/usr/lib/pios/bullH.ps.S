# @(#)36 1.1 src/bos/usr/lib/pios/bullH.ps.S, cmdpios, bos411, 9428A410j 4/28/94 07:54:22
#
# COMPONENT_NAME: (cmdpios) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
/LM 20 def
/ypos 725 def
/lineheight 12 def
/cr   { LM ypos moveto } def
/crlf { ypos lineheight sub
	/ypos exch def
	cr } def

0 rotate
/Helvetica-Bold findfont 60 scalefont setfont
108 500 moveto (%D) show
.35 setgray 108 700 moveto (%T) show
/Helvetica-Bold findfont 25 scalefont setfont
108 600 moveto (%P) show 
108 150 moveto (%H) show
showpage
