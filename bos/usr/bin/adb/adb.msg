$ "@(#)82	1.9.2.1  src/bos/usr/bin/adb/adb.msg, cmdadb, bos411, 9428A410j 10/6/93 15:12:10"
$
$ COMPONENT_NAME: (CMDADB) assembler debugger
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$
$
$ scmc msg file created Fri Aug 18 16:21:08 1989
$
$ Component ID for adb is 1281.
$
$ 

$quote "         define quote character.

$ len 
$set MS_steps        Define initial set#

E_MSG_56             "1281-013: cannot set breakpoint: "
M_MSG_59             "1281-014: can't single step through protected code, use continue"
E_MSG_67             "1281-015: can't read instruction"
E_MSG_68             "1281-016: cannot step through breakpoint instruction (tgte r2,r2)"
M_MSG_73             "1281-018: cannot continue the process execution\n"

$set MS_machdep		Redefine set#

M_MSG_74	     "1281-57: can't unwind the stack\n"
M_MSG_75	     "1281-58: internal error:  cannot recover from stack frames\n"
M_MSG_76	     "1281-59: adb could not find the trace back table\n"

$set MS_setup        Redefine set#

M_MSG_38             "1281-022: core file is outdated or doesn't match current program"
M_MSG_41             "1281-024: can't open %s\n"
M_MSG_42             "1281-025: can't open member %s in file %s\n"
M_MSG_43             "1281-026: bad corefile\n"
M_MSG_44             "1281-047: data section is missing from the core file\n"
M_MSG_45             "1281-048: user stack is missing from the core file\n"
M_MSG_46             "1281-049: ublock is missing from the corefile\n"
M_MSG_47             "1281-050: core is truncated\n"
M_MSG_48             "1281-051: error reading header of string table\n"
M_MSG_49             "1281-052: not enough space for the string table\n"
M_MSG_50             "1281-053: string table is missing or the object is stripped\n"
M_MSG_51             "1281-054: not enough space for the .debug section\n"
M_MSG_52             "1281-055: can't read .debug section\n"


$set MS_runpcs        Redefine set#

M_MSG_32             "1281-031: cannot  exec the process, probable cause - bad executable\n"
E_MSG_37             "1281-035: error in reading the general purpose registers"
E_MSG_38             "1281-036: error in reading the floating point registers"

$set MS_print        Redefine set#

E_MSG_22             "1281-037: no process\n"
E_MSG_23      "1281-038: Invalid instruction set.\nChoices are pwr, pwrx,ppc, 601, com, any, default"
E_MSG_24      "1281-039: Invalid mnemonic selection.\nChoices are pwr, ppc, default"
E_MSG_25      "Current instruction set is %s.\n"
E_MSG_26      "Current mnemonic selection is %s.\n"


$set MS_main        Redefine set#

M_MSG_77	     "1281-060: warning cannot open the catalogue file\n"
M_MSG_78	     "1281-061: Usage: adb [ -w ] [ -Idir ] [ -k ] [ -pPROMPT ] [ a.out [ core ]]\n"

$set MS_expr        Redefine set#

E_MSG_1              "1281-044: cannot unwind stack"
E_MSG_2              "1281-045: no previous frame"

$set MS_access        Redefine set#

E_MSG_14             "1281-046: not in write mode"

$set MS_extern        Redefine set#

E_MSG_80             "1281-080: address wrap around"
E_MSG_81             "1281-081: bad command"
E_MSG_82             "1281-082: data address not found"
E_MSG_83             "1281-083: unexpected `='"
E_MSG_84             "1281-084: bad file format"
E_MSG_85             "1281-085: unexpected ')'"
E_MSG_86             "1281-086: automatic variable not found"
E_MSG_87             "1281-087: bad core magic number"
E_MSG_88             "1281-088: bad modifier"
E_MSG_89             "1281-089: not enough space for symbols"
E_MSG_90             "1281-090: must have 2<= radix <= 16"
E_MSG_91             "1281-091: symbol not found"
E_MSG_92             "1281-092: syntax error"
E_MSG_93             "1281-093: text address not found"
E_MSG_94             "1281-094: bad variable"
E_MSG_95             "1281-095: wait error: process disappeared!"
E_MSG_96             "1281-096: process terminated"
E_MSG_97             "1281-097: too many breakpoints"
E_MSG_98             "1281-098: filename too long"
E_MSG_99             "1281-099: address expected"
E_MSG_100            "1281-100: no breakpoint set"
E_MSG_101            "1281-101: c routine not found"
E_MSG_102            "1281-102: newline expected"
E_MSG_103            "1281-103: try again"
E_MSG_104            "1281-104: cannot locate value"
E_MSG_105            "1281-105: no process"
E_MSG_106            "1281-106: cannot open"
E_MSG_107            "1281-107: bkpt: command too long"

$set MS_opcode        Redefine set#

E_MSG_150             "    Invalid Opcode."

