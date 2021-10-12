# @(#)37 1.1 src/bos/usr/lib/pios/bullT.ps.S, cmdpios, bos411, 9428A410j 4/28/94 07:54:50
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
/ypos 200 def
/lineheight 12 def
/cr   { LM ypos moveto } def
/crlf { ypos lineheight sub
	/ypos exch def
	cr } def

0 rotate


/Helvetica-Bold findfont 25 scalefont setfont
.35 setgray 108 700 moveto (%e) show
/Helvetica-Bold findfont 60 scalefont setfont
.0 setgray 108 600 moveto (%D) show
showpage
