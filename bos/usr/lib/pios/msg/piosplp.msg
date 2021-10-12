$ @(#)66        1.3.1.1  src/bos/usr/lib/pios/msg/piosplp.msg, cmdpios, bos411, 9428A410j 5/16/94 10:05:27
$
$ COMPONENT_NAME: (CMDPIOS) Printer Backend
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_SPLP
$
CANTGETSTAT \
0782-900 Cannot get the current line printer status for %1$s.\n\
\tThe errno (error number) is %2$d\n
$
CANTSETSTAT \
0782-901 Cannot set the line printer status.\n\
\tThe errno (error number) is %d\n
$
CANTSETSERIAL \
0782-902 Cannot set the serial port characteristics.\n\
\tThe errno (error number) is %d\n
$
NOTCNFIGD \
0782-903 %s is not configured in this kernel.\n
$
PERMISSION \
0782-904 Permission denied to access device %1$s.\n
$
DEVNOEXIST \
0782-905 Cannot open device %1$s.\n\
\tThe errno (error number) is %2$d.\n
$
ONEOPERAND \
0782-906 Cannot specify more than one operand (device name).\n
$
DEVBUSY \
0782-907 Device %1$s is busy.\n
$
NOTPRINTER \
0782-908 %1$s is not a printer.\n
$
BAD_TIMEOUT \
0782-909 Timeout value must be 1 second or more.\n
$
FONT_PLUS \
0782-910 The only valid parameter for the -F flag is !\n
$
HELPMSG \
0782-911 Unexpected flag option entered.\n\
\t Usage:\tsplp  [DeviceName]                    for status report\n\
\t\tsplp  FlagOptions  [DeviceName]       to change parameters\n\
\t where DeviceName is assumed to be lp0 if not specified\n\n\
\t Example: splp -l 88 -w 132 -i 5  lp2\n\n
$
BADARG_ONOFF \
0782-912 Flag parameter value is not valid. Must be + or !\n
$
SPLP_PARMS \
CURRENT FORMATTING PARAMETERS (ignored by qprt, lpr, and lp commands)|\
Note: -p + causes the other formatting parameters to be ignored.|\
CURRENT ERROR PROCESSING PARAMETERS|CURRENT SERIAL INTERFACE PARAMETERS|\
yes|no|device|pass-through?|send carriage returns?|\
page length (lines)|send line feeds?|\
page width (columns)|carriage rtn after line feed?|\
indentation (columns)|suppress tab expansion?|\
wrap long lines?|send backspaces?|\
convert to upper case?|send form feeds?|\
timeout value (seconds)|return on error?|\
baud rate|character size (bits)|\
enable parity?|two stop bits?|\
odd parity?|
$
$   This message is actually several messages collapsed into one.
$   This reduces the number of NLgetamsg calls to one for this string.
$
$   The vertical bars in this message represent message string separators
$                 !!!!!!!     DO NOT remove them.      !!!!!!!!
$
$   A maximum of 30 characters will be displayed for each of the component
$   substrings.  A substring may be longer than that, but the excess text
$   will be lost.
$
FLAGDISCONTD \
0782-913 %1$s flag is no longer supported.\n\
\t Run "smit chgprt" or "chdev -l %2$s -a %3$s='%4$s'"\n
$
