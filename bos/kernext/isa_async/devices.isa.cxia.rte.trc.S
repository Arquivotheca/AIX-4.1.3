* /etc/trcfmt
# static char sccsid[] = "@(#)93	1.2  src/bos/kernext/isa_async/devices.isa.cxia.rte.trc.S, sysxs128, bos41J, 174392.bos 3/27/95 13:50:47";
#
#   COMPONENT_NAME: SYSXTTY
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

+ 408 1.0 "@STTY CXMA" \
{{ \
scxma_cmdw(cmd) \
	$cmd, \
 	0080 "SRLOW ", \
	0081 "SRHIGH ", \
	0082 "FLUSHTX ", \
	0083 "PAUSETX ", \
	0084 "RESUMETX ", \
	0085 "SMINT ", \
	0086 "SAFLOWC ", \
	0087 "", \
	0088 "SBREAK ", \
	0089 "SMODEM ", \
	008A "SIFLAG ", \
	008B "SFLOWC ", \
	008C "STLOW ", \
	008E "RPAUSE ", \
	008F "RRESUME ", \
	0092 "BUFSETALL ", \
	0093 "SOFLAG ", \
	0094 "SHFLOW ", \
	0095 "SCFLAG ", \
	0096 "SVNEXT " \
}} \
        {{ $maj }} {{ $min }} {{ $how }} {{ $which }} \
        {{ $name = "cxma" }} \
	$411 \
	$which, \
	0009 "ioctl ", \
	000E "add ", \
	000F "del ", \
	0010 "initbd ", \
	0011 "wmove ", \
	0012 "param ", \
	0013 "xtimeo ", \
	0014 "xpoffl ", \
	0015 "event ", \
	0016 "download ", \
	0017 "fail ", \
	0018 "cmd ", \
	0019 "dl_open ", \
	001A "dl_close ", \
	001B "dl_ioctl ", \
	001C "dl_kme ", \
	001D "dl_reset ", \
	001E "dl_diag " \
	$how, \
	0000 { $which, \
	    0009 { "ptr" $D2%X4 "cmd" ioctlname($D3) }, \
            000E { "type" $D3, \
		0x1 "DG_MCXE", \
		0x2 "DG_CXMA" }, \
            000F { "type" $D3, \
		0x1 "DG_MCXE", \
		0x2 "DG_CXMA" }, \
            0010 { "type" $D3, \
		0x1 "DG_MCXE", \
		0x2 "DG_CXMA" }, \
            0011 { "bytes" $D3 }, \
            0015 { "type" $D3, \
		0x1 "DG_MCXE", \
		0x2 "DG_CXMA" }, \
            0018 { "cmd" scxma_cmdw($D3) "word" $D4%X1 "ncmd" $D5%X1 }, \
            001B { "cmd" $D3 } \
	    }, \
	0001 { $which, \
	    0002 { $D3, \
	        0000 "calling openDisc_open ", \
	        0001 "waiting for close to complete ", \
	        0002 "first open for Xprint device ", \
	        0003 "sleeping on IFTEM event ", \
	        0004 "sleeping on allocb() " \
		    "un_flags " $D4%X2 "ch_flags " $D5%X2 }, \
	    0003 { $D3, \
	        0000 "calling openDisc_close ", \
	        0002 "close for Xprint device ", \
	        0003 "sleeping on IFTEM event " \
		    "un_flags " $D4%X2 "ch_flags " $D5%X2 }, \
	    0006 { $D3, \
	        0000 "un_flags " $D4%X2 "flags " $D5%X2 }, \
	    0011 { "writing" $D3, \
	        0000 "chars to wrap ", \
	        0001 "remaining chars " \
		    $D4%X2 }, \
	    0015 { $D3, \
	        0000 "port " $D4%X2 "event " $D5%X2 } \
 	}, \
	0002 { $which, \
		\* { stty_ret($which) } }
