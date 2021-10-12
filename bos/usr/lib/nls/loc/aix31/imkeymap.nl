# @(#)20	1.4  src/bos/usr/lib/nls/loc/aix31/imkeymap.nl, cfgnlskm, bos411, 9428A410j 6/29/91 18:34:27
#
# COMPONENT_NAME: (LIBIM) National Language Support 
#
# FUNCTIONS: Dutch keymap for IM (Input Method) applications
#	     (e.g. aixterm).
#
#	     This represents the PS/2 keyboard.  (Keyboard ID 143)
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1986, 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.




# Declare States to be used
#S 1 2 3 4 5 9 17


# The unique key states are Base, Shift, CapsLock, Shift-CapsLock, Control, 
# Alt, and AltGraphic.	
#
# All other key states are mapped to one of these unique key states.  This 
# duplicate mapping is done at the end of this file to help keycomp 
# performance.



# Keysym
# ------
#
0x20	' ' ' ' ' ' ' ' 	' '	    U U U ' '	      U U U U U U U U
0x21	'!' '!' U U		"\033[054q" U U U "\033[065q" U U U U U U U U
0x22	'"' '"' U U		"\033[050q" U U U "\033[060q" U U U U U U U U
0x23	'#' '#' U U		"\033[112q" U U U "\033[113q" U U U U U U U U
0x24	'$' '$' U U		"\35"	    U U U "\033[085q" U U U U U U U U
0x25	'%' '%' U U		U	    U U U U	      U U U U U U U U
0x26	'&' '&' U U		"\033[049q" U U U "\033[058q" U U U U U U U U
0x27	'\'' '"' '\'' '"'	"\033[098q" U U U "\033[099q" U U U U U U U U
0x28	'(' '(' U U		"\033[052q" U U U "\033[062q" U U U U U U U U
0x29	')' ')' U U		"\033[069q" U U U "\033[070q" U U U U U U U U
0x2a	'*' '|' '|' '*' 	"\033[055q" U U U "\033[085q" U U U U U U U U
0x2b	'+' 'Ò' 'Ò' '+' 	"\033[096q" U U U "\033[097q" U U U U U U U U
0x2c	',' ';' ';' ',' 	"\033[108q" U U U "\033[109q" U U U U U U U U
0x2d	'-' '=' '=' '-' 	"\37"	    U U U "\033[068q" U U U U U U U U
0x2e	'.' ':' ':' '.' 	"\033[110q" U U U "\033[111q" U U U U U U U '˙'
0x2e	'.' '>' '.' '>' 	"\033[110q" U U U "\033[111q" U U U U U U U U
0x2f	'/' '?' '?' '/' 	"\033[112q" U U U "\033[113q" U U U U U U U '\\'
0x30	'0' '\'' '\'' '0'	"\033[056q" U U U "\033[067q" U U U U U U U U
0x31	'1' '!' '!' '1' 	"\033[049q" U U U "\033[058q" U U U U U U U '˚'
0x32	'2' '"' '"' '2' 	"\0"	    U U U "\033[059q" U U U U U U U '˝'
0x33	'3' '#' '#' '3' 	"\033[050q" U U U "\033[060q" U U U U U U U '¸'
0x34	'4' '$' '$' '4' 	"\033[051q" U U U "\033[061q" U U U U U U U '¨'
0x35	'5' '%' '%' '5' 	"\033[052q" U U U "\033[062q" U U U U U U U '´'
0x36	'6' '&' '&' '6' 	"\36"	    U U U "\033[063q" U U U U U U U 'Û'
0x37	'7' '_' '_' '7' 	"\033[053q" U U U "\033[064q" U U U U U U U 'ú'
0x38	'8' '(' '(' '8' 	"\33"	    U U U "\033[065q" U U U U U U U '{'
0x39	'9' ')' ')' '9' 	"\35"	    U U U "\033[066q" U U U U U U U '}'
0x3a	':' ':' U U		"\033[110q" U U U "\033[111q" U U U U U U U U
0x3b	';' ':' ';' ':' 	"\033[096q" U U U "\033[097q" U U U U U U U U
0x3c	'<' '>' '>' '<' 	"\033[057q" U U U "\033[115q" U U U U U U U U
0x3d	'=' '+' '=' '+' 	"\033[069q" U U U "\033[070q" U U U U U U U U
0x3e	'>' '>' U U		U	    U U U U	      U U U U U U U U
0x3f	'?' '?' U U		U	    U U U U	      U U U U U U U U
0x40	'@' 'ı' 'ı' '@' 	U	    U U U "\033[086q" U U U U U U U '™'
0x41	'A' 'A' U U		"\1"	    U U U "\033[087q" U U U U U U U U
0x42	'B' 'B' U U		"\2"	    U U U "\033[105q" U U U U U U U U 
0x43	'C' 'C' U U		"\3"	    U U U "\033[103q" U U U U U U U U
0x44	'D' 'D' U U		"\4"	    U U U "\033[089q" U U U U U U U U
0x45	'E' 'E' U U		"\5"	    U U U "\033[076q" U U U U U U U U
0x46	'F' 'F' U U		"\6"	    U U U "\033[090q" U U U U U U U U
0x47	'G' 'G' U U		"\7"	    U U U "\033[091q" U U U U U U U U
0x48	'H' 'H' U U		"\10"	    U U U "\033[092q" U U U U U U U U
0x49	'I' 'I' U U		"\11"	    U U U "\033[081q" U U U U U U U U
0x4a	'J' 'J' U U		"\12"	    U U U "\033[093q" U U U U U U U U
0x4b	'K' 'K' U U		"\13"	    U U U "\033[094q" U U U U U U U U
0x4c	'L' 'L' U U		"\14"	    U U U "\033[095q" U U U U U U U U
0x4d	'M' 'M' U U		"\15"	    U U U "\033[107q" U U U U U U U U
0x4e	'N' 'N' U U		"\16"	    U U U "\033[106q" U U U U U U U U
0x4f	'O' 'O' U U		"\17"	    U U U "\033[082q" U U U U U U U U
0x50	'P' 'P' U U		"\20"	    U U U "\033[083q" U U U U U U U U
0x51	'Q' 'Q' U U		"\21"	    U U U "\033[074q" U U U U U U U U
0x52	'R' 'R' U U		"\22"	    U U U "\033[077q" U U U U U U U U
0x53	'S' 'S' U U		"\23"	    U U U "\033[088q" U U U U U U U U
0x54	'T' 'T' U U		"\24"	    U U U "\033[078q" U U U U U U U U
0x55	'U' 'U' U U		"\25"	    U U U "\033[080q" U U U U U U U U
0x56	'V' 'V' U U		"\26"	    U U U "\033[104q" U U U U U U U U
0x57	'W' 'W' U U		"\27"	    U U U "\033[075q" U U U U U U U U
0x58	'X' 'X' U U		"\30"	    U U U "\033[102q" U U U U U U U U
0x59	'Y' 'Y' U U		"\31"	    U U U "\033[079q" U U U U U U U U
0x5a	'Z' 'Z' U U		"\32"	    U U U "\033[101q" U U U U U U U U
0x5b	'[' '{' '[' '{' 	"\33"	    U U U "\033[084q" U U U U U U U U
0x5c	'\\' '|' '\\' '|'	"\34"	    U U U "\033[086q" U U U U U U U U
0x5d	']' '[' '[' ']' 	"\34"	    U U U U	      U U U U U U U '›'
0x5e	'^' '^' U U		"\033[069q" U U U "\033[070q" U U U U U U U U
0x5f	'_' '_' U U		U	    U U U U	      U U U U U U U U
0x60	'`' '~' '`' '~' 	"\033[057q" U U U "\033[115q" U U U U U U U U
0x61	'a' 'A' 'A' 'a' 	"\1"	    U U U "\033[087q" U U U U U U U U
0x62	'b' 'B' 'B' 'b' 	"\2"	    U U U "\033[105q" U U U U U U U U
0x63	'c' 'C' 'C' 'c' 	"\3"	    U U U "\033[103q" U U U U U U U 'Ω'
0x64	'd' 'D' 'D' 'd' 	"\4"	    U U U "\033[089q" U U U U U U U U
0x65	'e' 'E' 'E' 'e' 	"\5"	    U U U "\033[076q" U U U U U U U U
0x66	'f' 'F' 'F' 'f' 	"\6"	    U U U "\033[090q" U U U U U U U U
0x67	'g' 'G' 'G' 'g' 	"\7"	    U U U "\033[091q" U U U U U U U U
0x68	'h' 'H' 'H' 'h' 	"\10"	    U U U "\033[092q" U U U U U U U U
0x69	'i' 'I' 'I' 'i' 	"\11"	    U U U "\033[081q" U U U U U U U U
0x6a	'j' 'J' 'J' 'j' 	"\12"	    U U U "\033[093q" U U U U U U U U
0x6b	'k' 'K' 'K' 'k' 	"\13"	    U U U "\033[094q" U U U U U U U U
0x6c	'l' 'L' 'L' 'l' 	"\14"	    U U U "\033[095q" U U U U U U U U
0x6d	'm' 'M' 'M' 'm' 	"\15"	    U U U "\033[107q" U U U U U U U 'Ê'
0x6e	'n' 'N' 'N' 'n' 	"\16"	    U U U "\033[106q" U U U U U U U U
0x6f	'o' 'O' 'O' 'o' 	"\17"	    U U U "\033[082q" U U U U U U U U
0x70	'p' 'P' 'P' 'p' 	"\20"	    U U U "\033[083q" U U U U U U U U
0x71	'q' 'Q' 'Q' 'q' 	"\21"	    U U U "\033[074q" U U U U U U U U
0x72	'r' 'R' 'R' 'r' 	"\22"	    U U U "\033[077q" U U U U U U U 'Ù'
0x73	's' 'S' 'S' 's' 	"\23"	    U U U "\033[088q" U U U U U U U '·'
0x74	't' 'T' 'T' 't' 	"\24"	    U U U "\033[078q" U U U U U U U U
0x75	'u' 'U' 'U' 'u' 	"\25"	    U U U "\033[080q" U U U U U U U U
0x76	'v' 'V' 'V' 'v' 	"\26"	    U U U "\033[104q" U U U U U U U U
0x77	'w' 'W' 'W' 'w' 	"\27"	    U U U "\033[075q" U U U U U U U U
0x78	'x' 'X' 'X' 'x' 	"\30"	    U U U "\033[102q" U U U U U U U 'Ø'
0x79	'y' 'Y' 'Y' 'y' 	"\31"	    U U U "\033[079q" U U U U U U U U
0x7a	'z' 'Z' 'Z' 'z' 	"\32"	    U U U "\033[101q" U U U U U U U 'Æ'
0x7b	'{' '{' U U		U	    U U U U	      U U U U U U U U
0x7c	'|' '|' U U		"\34"	    U U U "\033[086q" U U U U U U U U
0x7d	'}' '}' U U		U	    U U U U	      U U U U U U U U
0x7e	'~' '~' U U		"\033[057q" U U U "\033[115q" U U U U U U U U
0xa0	' ' ' ' U U		U	    U U U U	      U U U U U U U U
0xa1	'≠' '≠' U U		"\033[069q" U U U "\033[070q" U U U U U U U U
0xa2	'Ω' 'Ω' U U		U	    U U U U	      U U U U U U U U
0xa3	'ú' 'ú' U U		U	    U U U U	      U U U U U U U U
0xa4	'œ' 'œ' U U		U	    U U U U	      U U U U U U U U
0xa5	'æ' 'æ' U U		U	    U U U U	      U U U U U U U U
0xa6	'›' '›' U U		U	    U U U U	      U U U U U U U U
0xa7	'ı' 'ı' U U		"\033[057q" U U U "\033[115q" U U U U U U U U
0xa8	"D04" "D03" "D03" "D04" "\033[054q" U U U "\033[084q" U U U U U U U U
XK_dead_diaeresis	"D04" "D03" "D03" "D04" "\033[054q" U U U "\033[084q" U U U U U U U U
# 0xa8	'˘' '˘' U U		U	    U U U U	      U U U U U U U U
0xa9	'∏' '∏' U U		U	    U U U U	      U U U U U U U U
0xaa	'¶' '¶' U U		U	    U U U U	      U U U U U U U U
0xab	'Æ' 'Æ' U U		"\033[069q" U U U "\033[070q" U U U U U U U U
0xac	'™' '™' U U		"\033[069q" U U U "\033[070q" U U U U U U U U
0xad	'-' '-' U U		U	    U U U U	      U U U U U U U U
0xae	'©' '©' U U		U	    U U U U	      U U U U U U U U
0xaf	'Ó' 'Ó' U U		U	    U U U U	      U U U U U U U U
0xb0	'¯' "D05" "D05" '¯'	"\033[069q" U U U "\033[070q" U U U U U U U "D12"
0xb1	'Ò' 'Ò' U U		U	    U U U U	      U U U U U U U U
0xb2	'˝' '˝' U U		"\033[057q" U U U "\033[115q" U U U U U U U U
0xb3	'¸' '¸' U U		U	    U U U U	      U U U U U U U U
0xb4	"D00" "D02" "D02" "D00" "\033[098q" U U U "\033[099q" U U U U U U U U
XK_dead_acute	"D00" "D02" "D02" "D00" "\033[098q" U U U "\033[099q" U U U U U U U U
# 0xb4	'Ô' 'Ô' U U		U	    U U U U	      U U U U U U U U
0xb5	'Ê' 'Ê' U U		U	    U U U U	      U U U U U U U U
0xb6	'Ù' 'Ù' U U		U	    U U U U	      U U U U U U U U
0xb7	'˙' '˙' U U		U	    U U U U	      U U U U U U U U
0xb8	"D12" "D12" U U 	"\35"	    U U U "\033[085q" U U U U U U U U
XK_dead_cedilla	"D12" "D12" U U 	"\35"	    U U U "\033[085q" U U U U U U U U
# 0xb8	'˜' '˜' U U		U	    U U U U	      U U U U U U U U
0xb9	'˚' '˚' U U		U	    U U U U	      U U U U U U U U
0xba	'ß' 'ß' U U		"\34"	    U U U "\033[086q" U U U U U U U U
0xbb	'Ø' 'Ø' U U		U	    U U U U	      U U U U U U U U
0xbc	'¨' '¨' U U		U	    U U U U	      U U U U U U U U
0xbd	'´' '´' U U		U	    U U U "\033[086q" U U U U U U U U
0xbe	'Û' 'Û' U U		U	    U U U U	      U U U U U U U U
0xbf	'®' '®' U U		U	    U U U U	      U U U U U U U U
0xc0	'∑' '∑' U U		U	    U U U U	      U U U U U U U U
0xc1	'µ' 'µ' U U		U	    U U U U	      U U U U U U U U
0xc2	'∂' '∂' U U		U	    U U U U	      U U U U U U U U
0xc3	'«' '«' U U		U	    U U U U	      U U U U U U U U
0xc4	'é' 'é' U U		U	    U U U U	      U U U U U U U U
0xc5	'è' 'è' U U		U	    U U U U	      U U U U U U U U
0xc6	'í' 'í' U U		U	    U U U U	      U U U U U U U U
0xc7	'Ä' 'Ä' U U		U	    U U U U	      U U U U U U U U
0xc8	'‘' '‘' U U		U	    U U U U	      U U U U U U U U
0xc9	'ê' 'ê' U U		U	    U U U U	      U U U U U U U U
0xca	'“' '“' U U		U	    U U U U	      U U U U U U U U
0xcb	'”' '”' U U		U	    U U U U	      U U U U U U U U
0xcc	'ﬁ' 'ﬁ' U U		U	    U U U U	      U U U U U U U U
0xcd	'÷' '÷' U U		U	    U U U U	      U U U U U U U U
0xce	'◊' '◊' U U		U	    U U U U	      U U U U U U U U
0xcf	'ÿ' 'ÿ' U U		U	    U U U U	      U U U U U U U U
0xd0	'—' '—' U U		U	    U U U U	      U U U U U U U U
0xd1	'•' '•' U U		U	    U U U U	      U U U U U U U U
0xd2	'„' '„' U U		U	    U U U U	      U U U U U U U U
0xd3	'‡' '‡' U U		U	    U U U U	      U U U U U U U U
0xd4	'‚' '‚' U U		U	    U U U U	      U U U U U U U U
0xd5	'Â' 'Â' U U		U	    U U U U	      U U U U U U U U
0xd6	'ô' 'ô' U U		U	    U U U U	      U U U U U U U U
0xd7	'û' 'û' U U		U	    U U U U	      U U U U U U U U
0xd8	'ù' 'ù' U U		U	    U U U U	      U U U U U U U U
0xd9	'Î' 'Î' U U		U	    U U U U	      U U U U U U U U
0xda	'È' 'È' U U		U	    U U U U	      U U U U U U U U
0xdb	'Í' 'Í' U U		U	    U U U U	      U U U U U U U U
0xdc	'ö' 'ö' U U		U	    U U U U	      U U U U U U U U
0xdd	'Ì' 'Ì' U U		U	    U U U U	      U U U U U U U U
0xde	'Ë' 'Ë' U U		U	    U U U U	      U U U U U U U U
0xdf	'·' '·' U U		"\34"	    U U U "\033[086q" U U U U U U U U
0xe0	'Ö' 'Ö' U U		"\033[098q" U U U "\033[099q" U U U U U U U U
0xe1	'†' '†' U U		U	    U U U U	      U U U U U U U U
0xe2	'É' 'É' U U		U	    U U U U	      U U U U U U U U
0xe3	'∆' '∆' U U		U	    U U U U	      U U U U U U U U
0xe4	'Ñ' 'Ñ' U U		"\033[098q" U U U "\033[099q" U U U U U U U U
0xe5	'Ü' 'Ü' U U		"\033[054q" U U U "\033[084q" U U U U U U U U
0xe6	'ë' 'ë' U U		"\033[096q" U U U "\033[097q" U U U U U U U U
0xe7	'á' 'á' U U		"\033[057q" U U U "\033[115q" U U U U U U U U
0xe8	'ä' 'ä' U U		"\033[054q" U U U "\033[084q" U U U U U U U U
0xe9	'Ç' 'Ç' U U		"\033[112q" U U U "\033[113q" U U U U U U U U
0xea	'à' 'à' U U		U	    U U U U	      U U U U U U U U
0xeb	'â' 'â' U U		U	    U U U U	      U U U U U U U U
0xec	'ç' 'ç' U U		"\36"	    U U U "\033[070q" U U U U U U U U
0xed	'°' '°' U U		U	    U U U U	      U U U U U U U U
0xee	'å' 'å' U U		U	    U U U U	      U U U U U U U U
0xef	'ã' 'ã' U U		U	    U U U U	      U U U U U U U U
0xf0	'–' '–' U U		U	    U U U U	      U U U U U U U U
0xf1	'§' '§' U U		"\033[096q" U U U "\033[097q" U U U U U U U U
0xf2	'ï' 'ï' U U		"\033[096q" U U U "\033[097q" U U U U U U U U
0xf3	'¢' '¢' U U		U	    U U U U	      U U U U U U U U
0xf4	'ì' 'ì' U U		U	    U U U U	      U U U U U U U U
0xf5	'‰' '‰' U U		U	    U U U U	      U U U U U U U U
0xf6	'î' 'î' U U		"\033[096q" U U U "\033[097q" U U U U U U U U
0xf7	'ˆ' 'ˆ' U U		U	    U U U U	      U U U U U U U U
0xf8	'õ' 'õ' U U		"\033[096q" U U U "\033[097q" U U U U U U U U
0xf9	'ó' 'ó' U U		"\033[098q" U U U "\033[099q" U U U U U U U U
0xfa	'£' '£' U U		U	    U U U U	      U U U U U U U U
0xfb	'ñ' 'ñ' U U		U	    U U U U	      U U U U U U U U
0xfc	'Å' 'Å' U U		"\033[054q" U U U "\033[084q" U U U U U U U U
0xfd	'Ï' 'Ï' U U		U	    U U U U	      U U U U U U U U
0xfe	'Á' 'Á' U U		U	    U U U U	      U U U U U U U U
0xff	'ò' 'ò' U U		U	    U U U U	      U U U U U U U U

# 0x9df ' ' ' ' U U		U	    U U U U	      U U U U U U U U
# 0x9e0  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e1  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e2  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e3  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e4  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e5  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e6  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e7  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e8  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9e9  U   U	U U		U	    U U U U	      U U U U U U U U
# 0x9ea 'Ÿ' 'Ÿ' U U		"\033[194q" U U U U	      U U U U U U U U
# 0x9eb 'ø' 'ø' U U		"\033[190q" U U U U	      U U U U U U U U
# 0x9ec '⁄' '⁄' U U		"\033[172q" U U U U	      U U U U U U U U
# 0x9ed '¿' '¿' U U		"\033[176q" U U U U	      U U U U U U U U
# 0x9ee '≈' '≈' U U		"\033[184q" U U U U	      U U U U U U U U
# 0x9ef 'ƒ' 'ƒ' U U		"\033[196q" U U U "\033[197q" U U U U U U U U
# 0x9f0 'ƒ' 'ƒ' U U		"\033[196q" U U U "\033[197q" U U U U U U U U
# 0x9f1 'ƒ' 'ƒ' U U		"\033[196q" U U U "\033[197q" U U U U U U U U
# 0x9f2 'ƒ' 'ƒ' U U		"\033[196q" U U U "\033[197q" U U U U U U U U
# 0x9f3 'ƒ' 'ƒ' U U		"\033[196q" U U U "\033[197q" U U U U U U U U
# 0x9f4 '√' '√' U U		"\033[174q" U U U U	      U U U U U U U U
# 0x9f5 '¥' '¥' U U		"\033[192q" U U U U	      U U U U U U U U
# 0x9f6 '¡' '¡' U U		"\033[186q" U U U U	      U U U U U U U U
# 0x9f7 '¬' '¬' U U		"\033[182q" U U U U	      U U U U U U U U
# 0x9f8 '≥' '≥' U U		"\033[178q" U U U U	      U U U U U U U U

# The following are diacritical keysyms for AIX 3.2

XK_dead_grave "D02" "D02"    U        U     "\033[098q" U U U "\033[099q" U U U U U U U U
XK_dead_circumflex "D03" "D03"    U        U     "\33"	 U U U "\033[084q" U U U U U U U U
XK_dead_tilde "D05" "D05"    U        U     "\033[057q" U U U "\033[115q" U U U U U U U U
XK_dead_degree "D09" "D09"    U        U     U		 U U U U	   U U U U U U U U

# The following are diacritical keysysms used in AIX 3.1

0xff00 "D02" "D02"    U        U     "\033[098q" U U U "\033[099q" U U U U U U U U
0xff01 "D03" "D03"    U        U     "\33"	 U U U "\033[084q" U U U U U U U U
0xff02 "D05" "D05"    U        U     "\033[057q" U U U "\033[115q" U U U U U U U U
0xff03 "D09" "D09"    U        U     U		 U U U U	   U U U U U U U U
0xff08 "\10" "\10"    "\10"    "\10" "\177"	 U U U "\033[071q" U U U U U U U U
0xff09 "\11" "\033[Z" "\11" "\033[Z" "\033[072q" U U U "\033[073q" U U U U U U U U
0xff0a "\12" "\12"    "\12"    "\12" "\12"	 U U U "\12"	   U U U U U U U U
0xff0b U     U	      U        U     U		 U U U U	   U U U U U U U U
0xff0d "\15" "\15" "\15" "\15"	     "\15"	 U U U "\033[100q" U U U U U U U U
0xff13 "\033[217q" "\033[218q" "\033[217q" "\033[218q" "\177"	   U U U "\177"      U U U U U U U U
0xff1b "\33"	   "\033[120q" "\33"	   "\033[120q" "\033[121q" U U U "\033[122q" U U U U U U U U
0xff20 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff21 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff50 "\033[H"    "\033[143q" "\033[H"    "\033[143q" "\033[144q" U U U "\033[145q" U U U U U U U U
0xff51 "\033[D"    "\033[158q" "\033[D"    "\033[158q" "\033[159q" U U U "\033[160q" U U U U U U U U
0xff52 "\033[A"    "\033[161q" "\033[A"    "\033[161q" "\033[162q" U U U "\033[163q" U U U U U U U U
0xff53 "\033[C"    "\033[167q" "\033[C"    "\033[167q" "\033[168q" U U U "\033[169q" U U U U U U U U
0xff54 "\033[B"    "\033[164q" "\033[B"    "\033[164q" "\033[165q" U U U "\033[166q" U U U U U U U U
0xff55 "\033[150q" "\033[151q" "\033[150q" "\033[151q" "\033[152q" U U U "\033[153q" U U U U U U U U
0xff56 "\033[154q" "\033[155q" "\033[154q" "\033[155q" "\033[156q" U U U "\033[157q" U U U U U U U U
0xff57 "\033[146q" "\033[147q" "\033[146q" "\033[147q" "\033[148q" U U U "\033[149q" U U U U U U U U
0xff58 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff60 "\033[146q" "\033[147q" "\033[146q" "\033[147q" "\033[148q" U U U "\033[149q" U U U U U U U U
0xff61 "\033[209q" "\033[210q" "\033[209q" "\033[210q" "\033[211q" U U U "\033[212q" U U U U U U U U
0xff62 "\033[114q" U	       "\033[114q" U	       U	   U U U U	     U U U U U U U U
0xff63 "\033[139q" "\033[139q" "\033[139q" "\033[139q" "\033[140q" U U U "\033[141q" U U U U U U U U
0xff65 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff66 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff67 "\033[114q" U	       "\033[114q" U	       U	   U U U U	     U U U U U U U U
0xff68 U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff69 "\033[213q" "\033[214q" "\033[213q" "\033[214q" "\033[215q" U U U "\033[216q" U U U U U U U U
0xff6a "\033[001q" "\033[013q" "\033[001q" "\033[013q" "\033[025q" U U U "\033[037q" U U U U U U U U
0xff6b "\177"	   "\177"      "\177"	   "\177"      "\177"	   U U U "\177"      U U U U U U U U
0xff7e U	   U	       U	   U	       U	   U U U U	     U U U U U U U U
0xff7f U	   U	       U	   U	       "\23"	   U U U "\033[170q" U U U U U U U U
0xff80 ' '	   ' '	       ' '	   ' '	       ' '	   U U U ' '	     U U U U U U U U
0xff89 "\10"	   "\10"       "\10"	   "\10"       "\10"	   U U U "\10"	     U U U U U U U U
0xff8d "\15"	   "\15"       "\15"	   "\15"       "\15"	   U U U "\033[100q" U U U U U U U U
0xff91 "\033[001q" "\033[013q" "\033[001q" "\033[013q" "\033[025q" U U U "\033[037q" U U U U U U U U
0xff92 "\033[002q" "\033[014q" "\033[002q" "\033[014q" "\033[026q" U U U "\033[038q" U U U U U U U U
0xff93 "\033[003q" "\033[015q" "\033[003q" "\033[015q" "\033[027q" U U U "\033[039q" U U U U U U U U
0xff94 "\033[004q" "\033[016q" "\033[004q" "\033[016q" "\033[028q" U U U "\033[040q" U U U U U U U U
0xffaa '*' '*' '*' '*'	       "\033[187q" U U U "\033[188q" U U U U U U U U
0xffab '+' '+' '+' '+'	       "\033[200q" U U U "\033[201q" U U U U U U U U
0xffac ',' ',' ',' ','	       U	   U U U U	     U U U U U U U U
0xffad '-' '-' '-' '-'	       "\033[198q" U U U "\033[199q" U U U U U U U U
0xffae 'ƒ' '.' 'ƒ' '.'	       "\033[196q" U U U "\033[197q" U U U U U U U U
0xffaf '/' '/' '/' '/'	       "\033[179q" U U U "\033[180q" U U U U U U U U
0xffb0 '≥' '0' '≥' '0'	       "\033[178q" U U U U	     U U U U U U U U
0xffb1 '¿' '1' '¿' '1'	       "\033[176q" U U U U	     U U U U U U U U
0xffb2 '¡' '2' '¡' '2'	       "\033[186q" U U U U	     U U U U U U U U
0xffb3 'Ÿ' '3' 'Ÿ' '3'	       "\033[194q" U U U U	     U U U U U U U U
0xffb4 '√' '4' '√' '4'	       "\033[174q" U U U U	     U U U U U U U U
0xffb5 '≈' '5' '≈' '5'	       "\033[184q" U U U U	     U U U U U U U U
0xffb6 '¥' '6' '¥' '6'	       "\033[192q" U U U U	     U U U U U U U U
0xffb7 '⁄' '7' '⁄' '7'	       "\033[172q" U U U U	     U U U U U U U U
0xffb8 '¬' '8' '¬' '8'	       "\033[182q" U U U U	     U U U U U U U U
0xffb9 'ø' '9' 'ø' '9'	       "\033[190q" U U U U	     U U U U U U U U
0xffbe "\033[001q" "\033[013q" "\033[001q" "\033[013q" "\033[025q" U U U "\033[037q" U U U U U U U U
0xffbf "\033[002q" "\033[014q" "\033[002q" "\033[014q" "\033[026q" U U U "\033[038q" U U U U U U U U
0xffc0 "\033[003q" "\033[015q" "\033[003q" "\033[015q" "\033[027q" U U U "\033[039q" U U U U U U U U
0xffc1 "\033[004q" "\033[016q" "\033[004q" "\033[016q" "\033[028q" U U U "\033[040q" U U U U U U U U
0xffc2 "\033[005q" "\033[017q" "\033[005q" "\033[017q" "\033[029q" U U U "\033[041q" U U U U U U U U
0xffc3 "\033[006q" "\033[018q" "\033[006q" "\033[018q" "\033[030q" U U U "\033[042q" U U U U U U U U
0xffc4 "\033[007q" "\033[019q" "\033[007q" "\033[019q" "\033[031q" U U U "\033[043q" U U U U U U U U
0xffc5 "\033[008q" "\033[020q" "\033[008q" "\033[020q" "\033[032q" U U U "\033[044q" U U U U U U U U
0xffc6 "\033[009q" "\033[021q" "\033[009q" "\033[021q" "\033[033q" U U U "\033[045q" U U U U U U U U
0xffc7 "\033[010q" "\033[022q" "\033[010q" "\033[022q" "\033[034q" U U U "\033[046q" U U U U U U U U
0xffc8 "\033[011q" "\033[023q" "\033[011q" "\033[023q" "\033[035q" U U U "\033[047q" U U U U U U U U
0xffc9 "\033[012q" "\033[024q" "\033[012q" "\033[024q" "\033[036q" U U U "\033[048q" U U U U U U U U
0xffca "\033[013q" "\033[025q" "\033[013q" "\033[025q" "\033[037q" U U U "\033[049q" U U U U U U U U
0xffcb "\033[014q" "\033[026q" "\033[014q" "\033[026q" "\033[038q" U U U "\033[050q" U U U U U U U U
0xffcc "\033[015q" "\033[027q" "\033[015q" "\033[027q" "\033[039q" U U U "\033[051q" U U U U U U U U
0xffcd "\033[016q" "\033[028q" "\033[016q" "\033[028q" "\033[040q" U U U "\033[052q" U U U U U U U U
0xffce "\033[017q" "\033[029q" "\033[017q" "\033[029q" "\033[041q" U U U "\033[053q" U U U U U U U U
0xffcf "\033[209q" "\033[210q" "\033[209q" "\033[210q" "\033[211q" U U U "\033[212q" U U U U U U U U
0xffd0 "\033[213q" "\033[214q" "\033[213q" "\033[214q" "\033[215q" U U U "\033[216q" U U U U U U U U
0xffd1 "\033[217q" "\033[218q" "\033[217q" "\033[218q" "\177"      U U U "\177"      U U U U U U U U
0xffd2 "\033[021q" "\033[033q" "\033[021q" "\033[033q" "\033[045q" U U U "\033[057q" U U U U U U U U
0xffd3 "\033[022q" "\033[034q" "\033[022q" "\033[034q" "\033[046q" U U U "\033[058q" U U U U U U U U
0xffd4 "\033[023q" "\033[035q" "\033[023q" "\033[035q" "\033[047q" U U U "\033[059q" U U U U U U U U
0xffd5 "\033[024q" "\033[036q" "\033[024q" "\033[036q" "\033[048q" U U U "\033[060q" U U U U U U U U
0xffd6 "\033[025q" "\033[037q" "\033[025q" "\033[037q" "\033[049q" U U U "\033[061q" U U U U U U U U
0xffd7 "\033[026q" "\033[038q" "\033[026q" "\033[038q" "\033[050q" U U U "\033[062q" U U U U U U U U
0xffd8 "\033[027q" "\033[039q" "\033[027q" "\033[039q" "\033[051q" U U U "\033[063q" U U U U U U U U
0xffd9 "\033[028q" "\033[040q" "\033[028q" "\033[040q" "\033[052q" U U U "\033[064q" U U U U U U U U
0xffda "\033[029q" "\033[041q" "\033[029q" "\033[041q" "\033[053q" U U U "\033[065q" U U U U U U U U
0xffdb "\033[030q" "\033[042q" "\033[030q" "\033[042q" "\033[054q" U U U "\033[066q" U U U U U U U U
0xffdc "\033[031q" "\033[043q" "\033[031q" "\033[043q" "\033[055q" U U U "\033[067q" U U U U U U U U
0xffdd "\033[032q" "\033[044q" "\033[032q" "\033[044q" "\033[056q" U U U "\033[068q" U U U U U U U U
0xffde "\033[033q" "\033[045q" "\033[033q" "\033[045q" "\033[057q" U U U "\033[069q" U U U U U U U U
0xffdf "\033[034q" "\033[046q" "\033[034q" "\033[046q" "\033[058q" U U U "\033[070q" U U U U U U U U
0xffe0 "\033[035q" "\033[047q" "\033[035q" "\033[047q" "\033[059q" U U U "\033[071q" U U U U U U U U
0xffe1 U U U U U U U U U U U U U U U U U
0xffe2 U U U U U U U U U U U U U U U U U
0xffe3 U U U U U U U U U U U U U U U U U
0xffe3 U U U U U U U U U U U U U U U U U
0xffe4 U U U U U U U U U U U U U U U U U
0xffe5 U U U U U U U U U U U U U U U U U
0xffe6 U U U U U U U U U U U U U U U U U
0xffe7 U U U U U U U U U U U U U U U U U
0xffe8 U U U U U U U U U U U U U U U U U
0xffe9 U U U U U U U U U U U U U U U U U
0xffea U U U U U U U U U U U U U U U U U
0xffeb U U U U U U U U U U U U U U U U U
0xffec U U U U U U U U U U U U U U U U U
0xffed U U U U U U U U U U U U U U U U U
0xffee U U U U U U U U U U U U U U U U U
0xffff "\033[P"    "\033[P"    "\033[P"	   "\033[P"    "\033[142q" U U U "\033[M"    U U U U U U U U


# Map other states to the unique default states.
# This mapping is done at the end of the file to help keycomp performance.
#M 5  6 7 8 13 14 15 16 21 22 23 24 29 30 31 32
#M 9  10 11 12 25 26 27 28
#M 17 18 19 20
