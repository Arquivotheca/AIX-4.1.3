$ @(#)23	1.3  src/bos/usr/bin/keycomp/keycomp.msg, cmdimkc, bos411, 9428A410j 7/8/93 19:55:07
$
$ COMPONENT_NAME : (cmdimkc) AIX Input Method Keymap Compiler
$
$ FUNCTIONS : message file for keycomp
$
$ ORIGINS : 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$	Message Catalog for keycomp command
$
$len
$quote "
$set MS_KEYCOMP
$
$	Do not translate "keycomp"	
$
MN_MEM	"keycomp : cannot allocate memory\n"
$
$	Do not modify "%%M", "#M/#S".  They are keywords.
$
MN_MIX	"keycomp : %%M and #M/#S cannot be mixed\n"
$
MN_LINE	"keycomp : error at line %d\n"
$
$	Two parameters, line number and item number, are passed
$	in this order.
$
MN_ITEM	"keycomp : error at line %1$d item %2$d\n"
$
$	Usage massage -- Do not translate
$
MN_USG	"Usage: keycomp {-c}\n"
$
$	parsing error
$
MN_PARS	"keycomp : %1$s at line %2$d in %3$s\n"
$
$	CPP error -- Do not translate "CPP"
$
MN_CPP	"keycomp : CPP error\n"
$
$	output error
$
MN_OUT	"keycomp : output error\n"
$
$	unknown keysym -- Do not translate "keysym"
$
MN_KSYM	"unknown keysym"
