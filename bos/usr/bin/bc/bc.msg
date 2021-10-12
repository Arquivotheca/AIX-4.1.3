$ @(#)56	1.8  src/bos/usr/bin/bc/bc.msg, cmdcalc, bos411, 9428A410j 2/27/94 13:05:47
$
$ COMPONENT_NAME: (CMDCALC) calculators
$
$ FUNCTIONS: 
$
$ ORIGINS: 27, 71
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
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ OSF/1 1.1
$
$quote "
$set MS_BC
BUND "bundle %d elements at %o\n"
ROUT "routput(%o)\n"
INVINP "%s on line %d %s"
$ Do not put a new line.  yyerror() adds it when printing out the msg.
NOBUNDLE "0705-001: bundling space exceeded"
$ Do not put a new line.  Bc automatically includes it when printing out msg.
NOINFILE "0705-002: cannot open input file"
$ Do not put a new line character at the end. INVINP will follow it.
NOSTRSPC "0705-003: string space exceeded"
$ Do not put a new line character at the end. INVINP will follow it.
TOOBIG "0705-004: program too big"
$ Do not put a new line character at the end. INVINP will follow it.
USAGE "usage: bc [-cl] [files...]\n"
BADIBASE "0705-005: Input base value exceeds the maximum limit of 16"
$ Do not put a new line character at the end. INVINP will follow it.
MINIBASE "0705-006: Input base value cannot be less than 2"
$ Do not put a new line character at the end. INVINP will follow it.
MINOBASE "0705-007: Output base value cannot be less than 2"
$ Do not put a new line character at the end. INVINP will follow it.
