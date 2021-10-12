$  static char sccs_id[] = "src/bos/usr/ccs/lib/libtli/libtli.msg, libtli, bos411, 9428A410j 93/11/09 18:50:03";
$  COMPONENT_NAME:  LIBTLI
$
$  FUNCTIONS:
$
$  ORIGINS: 18, 27
$
$  This module contains IBM CONFIDENTIAL code. -- (IBM
$  Confidential Restricted when combined with the aggregated
$  modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp., 1993
$  All Rights Reserved
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2

$set MS_LIBTLI
$quote "
$ The following  are from libtli/terrnolst.c
1		"Incorrect address format"
2		"Incorrect option format"
3		"Incorrect permissions"
$ Do not translate "fd"
4		"Illegal transport fd"
5		"Could not allocate address"
6		"Out of state"
7		"Bad call sequence number"
8		"System error"
9		"Event requires attention"
10		"Illegal amount of data"
11		"Buffer not large enough"
12		"Flow control"
13		"No data"
14		"No disconnect indication available"
$ Do not translate "unitdata"
15		"No unitdata error indication available"
16		"Bad flags"
17		"No orderly release indication available"
18		"Primitive not supported"
19		"State is in process of changing"
$ Do not translate "struct_type"
20		"Unsupported struct_type requested"
21		"Invalid transport provider name"
$ Do not translate "qlen"
22		"qlen is zero"
23		"Address in use"
$ XPG4 additions - terror and tstrerror shift by 3
24		"Outstanding connection indications"
25		"Transport provider mismatch"
$ Do not translate "resfd", "fd" and "qlen"
26		"resfd specified to accept with qlen > 0"
27		"resfd not bound to same address as fd"
28		"Incoming connection queue is full"
$ Do not translate "XTI"
29		"XTI protocol error"
$ gencat allocates 30 31 32 for these
$ Do not translate "t_error"
TERROR_MSG	"t_error"
TERROR_ERR	"%1$s: Error %2$d occurred\n"
TERROR_STRERR	"%d: error unknown"
