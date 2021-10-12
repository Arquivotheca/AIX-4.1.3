$ @(#)58        1.4  src/bos/usr/lib/piosk/msg/jfmtrs.msg, cmdpiosk, bos411, 9428A410j 5/27/93 06:17:55
$
$ COMPONENT_NAME: (CMDPIOS) Printer Backend
$
$ FUNCTIONS: none
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 1
$
$       Messages issued by a formatter
$
120 Dummy Message.
MSG_BADFORMFLAG \
0782-500 The parameter value specified with the -%.1s flag is not valid.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG1 \
0782-501 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be + or !.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG2 \
0782-502 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1 or greater.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG3 \
0782-503 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0 or greater.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG4 \
0782-504 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 10 or 12.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG5 \
0782-505 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0, 1, 2, or 3.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG6 \
0782-506 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 6 or 8.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG7 \
0782-507 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0, 1, or 2.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG8 \
0782-508 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be less than the page width.\n\
\tCheck the parameter value.\n
$
MSG_BADFORMFLAG9 \
0782-509 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1, 2, or 3.\n\
\tCheck the parameter value.\n
$
MSG_NOPITCH \
0782-510 Cannot specify a pitch value directly.\n\
\tThe pitch value is implied by the font identifier.\n\
\tSpecify an appropriate font identifier with the -I flag.\n\
\tNote: To determine the correct font identifier, consult the manual\n\
\t      that came with your printer.\n
$
MSG_BAD4224COLOR \
0782-511 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    black   (or 0)\n\
\t    blue    (or 1)\n\
\t    red     (or 2)\n\
\t    magenta (or 3)\n\
\t    green   (or 4)\n\
\t    cyan    (or 5)\n\
\t    yellow  (or 6)\n\
\t    brown   (or 10)\n\
\t Check the parameter value.\n
$
$	Message For Japanese Printers
$
$
$ The following is a unique message for Japanese Printers Only
MSG_BADQUALITY \
0782-700 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1 or 2.\n\
\tCheck the parameter value.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_BADFONTSTYLE1 \
0782-701 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    m10\n\
\t    m12\n\
\t    gothic\n\
\t    courier\n\
\t    elite\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADFONTSTYLE2 \
0782-702 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    m12\n\
\t    gothic\n\
\t    courier\n\
\t    elite\n\
\t    orator\n\
\t    ocr\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADPITCH \
0782-703 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    10\n\
\t    12\n\
\t    13.4\n\
\t    15\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADLPI \
0782-704 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     2\n\
\t     3\n\
\t     4\n\
\t     5\n\
\t     6\n\
\t     7.5\n\
\t     8\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADPAPERFLAG \
0782-705 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     1: Manual\n\
\t     2: Pinwheel\n\
\t     3: Sheetfeed\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADPAPERSRC \
0782-706 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     1: Upper\n\
\t     2: Lower\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADROTATION \
0782-707 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     0: Portrait\n\
\t     1: Landscape right\n\
\t     2: Portrait Upside-down\n\
\t     3: Landscape left\n\
\t Check the parameter value.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADUSERFONT1 \
0782-708 Cannot access the specified user-defined font file %s.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_BADUSERFONT2 \
0782-709 The file %s specified as a user-defined font file is not \n\
a regular file. \n
$
$ The following is a unique message for Japanese Printers Only
MSG_BADUSERFONT3 \
0782-710 Cannot allocate memory for the user-defined font file %s. \n
$ The following is a unique message for Japanese Printers Only
$
MSG_BADUSERFONT4 \
0782-711 Cannot open the specified user-defined font file %s. \n
$
$ The following is a unique message for Japanese Printers Only
MSG_BADUSERFONT5 \
0782-712 Cannot load the specified user-defined font file %s. \n
$
$ The following is a unique message for Japanese Printers Only
MSG_BADUSERFONT6 \
0782-713 The file %s specified as a user-defined font file is not a \n\
user-defined font file in snf format.\n 
$
$ The following is a unique message for Japanese Printers Only
MSG_BADCODEPAGE \
0782-714 The specified code page is not supported or bad.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_MEMALLOCERR \
0782-715 Memory allocation error has happened.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_FATAL \
0782-716 Fatal error.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_IBUFFULL \
0782-717 Internal buffer full.\n
$
$ The following is a unique message for Japanese Printers Only
MSG_IBUFFATAL \
0782-718 Internal buffer fatal error.\n
$
$ following message for JP/KR/TW Printers
$ 
MSG_BADNLSDIR \
0782-719 Cannot open file %s.\n
$
MSG_BADTOORNLS \
0782-720 Cannot specify which font to use, printer or X font.\n\
\t Check the discription of parameter To in colon file.\n\
\t Check the data base file %s.\n 
$
MSG_BADFORMATF \
0782-721 Data base file %s for printer has incompatible format.\n
$
MSG_BADFORMATP \
0782-722 The discription of parameter %s in colon file is not valid.\n
$
MSG_BADTRASFILE \
0782-723 Cannot open translation file %s.\n\
\t Check the file and /usr/lib/lpd/pio/transJP/trans.dir.\n
