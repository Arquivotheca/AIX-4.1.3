$ @(#)00        1.2  src/bos/usr/bin/patch/patch.msg, cmdposix, bos411, 9428A410j 3/14/94 09:12:42
$
$ COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
$
$ FUNCTIONS: None
$
$ ORIGINS: 27, 85
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$ OSF/1 1.2
$
$ @(#)$RCSfile: patch.msg,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1992/07/06 18:32:41 $
$
$ messages for the patch program
$ Message number 001 - 050 reserved for this file.
$set MS_PATCH
$quote "
$
$ message for inp.c
$
PLAN	"Patching file %s using Plan %s...\n"
CREAT	"(Creating file %s...)\n"
TRYRCS	"patch: 3016-001 Cannot find %s--attempting to check it out from RCS.\n"
NOCHECK	"patch: 3016-002 Cannot check out %s.\n"
TRYSCCS	"patch: 3016-003 Cannot find %s--attempting to get it from SCCS.\n"
NOGET	"patch: 3016-004 Cannot get %s.\n"
NOFIND	"patch: 3016-005 Cannot find %s.\n"
NOTNORM	"patch: 3016-006 %s is not a normal file--cannot patch.\n"
NOOPEN	"patch: 3016-007 Cannot open file %s\n"
VERS1	"patch: 3016-008 This file does not appear to be the %s version--patching anyway.\n"
VERS2	"This file does not appear to be the %s version--patch anyway? [%s] "
$ the %s in the brackets is replaced with the locales no string
$
ABORTED	"Aborted.\n"
VERS3	"Good.  This file appears to be the %s version.\n"
NOMEM	"patch: 3016-009 Cannot allocate enough memory.\n"
TMPWR	"patch: 3016-010 Cannot write temp file.\n"
NOREOPEN	"patch: 3016-011 Cannot reopen file %s\n"
TMPRD	"patch: 3016-012 Error reading tmp file %s.\n"
$
$ messages for patch.c
$ Hunk is a technical term which should not be translated
$
UNSWAP	"Not enough memory to try swapped hunk!  Assuming unswapped.\n"
ALLOCE	"Lost hunk on alloc error!\n"
REVERSE	"Reversed (or previously applied) patch detected!  Assume -R? [%s] "
UNREVERSE "Unreversed (or previously applied) patch detected!  Ignore -R? [%s] "
ANYWAY	"Apply anyway? [%s] "
IGNORE	"Ignoring previously applied (or reversed) patch.\n"
HIGNORE	"Hunk #%d ignored at %ld.\n"
HFAIL	"Hunk #%d failed at %ld.\n"
HGOOD	"Hunk #%d succeeded at %ld"
$ Do not translate fuzz
FUZZ	" with fuzz %ld"
OFFSET1	" (offset %ld line)"
OFFSET2	" (offset %ld lines)"
TRYAGAIN	"\n\nRan out of memory using Plan A--trying again...\n\n"
REJECT	"%d out of %d hunks ignored--saving rejects to %s\n"
REJECT2	"%d out of %d hunks failed--saving rejects to %s\n"
TOOMANY	"patch: 3016-013 Too many file arguments.\n"
CANTCD	"patch: 3016-014 Cannot cd to %s.\n"
BADD	"patch: 3016-015 Argument to -D is not an identifier.\n"
BADN	"patch: 3016-016 Argument to -%c is not a number.\n"
USAGE  "Usage: patch [-flsvNR] [-b [-B prefix]] [-c | -e | -n] [-d directory]\n\
              [-D define] [-F number] [-i patchfile] [-o outfile]\n\
              [-p number] [-r rejectfile] [-x number] [file]\n"
FHUNK	"patch: 3016-017 Fatal internal error in abort_hunk().\n"
BADSYNC	"Out-of-sync patch, lines %ld,%ld--corrupted text or line numbers, maybe?\n"
NOCREAT	"patch: 3016-018 Cannot create %s.\n"
BADORDER	"patch: 3016-019 Misordered hunks! Output will be garbled.\n"
$
$ messages for pch.c
$
NOTFOUND	"patch: 3016-020 Patch file %s not found\n"
GNOMEM	"patch: 3016-021 Out of memory\n"
DONE	"done\n"
HUMM	"Processing..."
$ printed while patch is determining what to do, ie "Thinking..."
TRAIL	"  Ignoring the trailing text.\ndone\n"
NOPATCH	"  I cannot find a patch in there anywhere.\n"
TYPE1	"  Looks like a context diff...\n"
TYPE2	"  The next patch looks like a context diff...\n"
TYPE3	"  Looks like a normal diff...\n"
TYPE4	"  The next patch looks like a normal diff...\n"
TYPE5	"  Looks like an ed script...\n"
TYPE6	"  The next patch looks like an ed script...\n"
TYPE7	"  Looks like a new-style context diff...\n"
TYPE8	"  The next patch looks like a new-style context diff...\n"
INDENT1	"(Patch is indented 1 space.)\n"
INDENT2	"(Patch is indented %d spaces.)\n"
NOFILE	"No file to patch.  Skipping...\n"
ASKFILE	"File to patch: "
SKIPTHIS	"No file found--skip this patch? [%s] "
SKIPPING	"Skipping patch...\n"
LEADT	"The text leading up to this was:\n"
EXPEOF	"patch: 3016-022 Unexpected end of file in patch.\n"
EXPHUNK	"patch: 3016-023 Unexpected end of hunk at line %ld.\n"
EXPSTAR	"patch: 3016-024 Unexpected *** at line %ld: %s"
DUPDASH	"patch: 3016-025 Duplicate \"---\" at line %ld--check line numbers at line %ld.\n"
PREDASH	"patch: 3016-026 Premature \"---\" at line %ld--check line numbers at line %ld.\n"
DUEDASH	"patch: 3016-027 Overdue \"---\" at line %ld--check line numbers at line %ld.\n"
TOLARGE	"patch: 3016-028 Hunk is too large (%ld lines) at line %ld: %s"
NODASH	"patch: 3016-029 No --- found in patch at line %ld\n"
NOVEL	"patch: 3016-030 (This is really a new-style context diff but without\n\
the telltale extra asterisks on the *** line that usually indicate\n\
the new style...)\n"
MANGLED	"patch: 3016-031 Replacement text or line numbers corrupted in hunk at line %ld\n"
EXPEOF2	"patch: 3016-032 Unexpected end of file in patch at line %ld.\n"
EXPLESS	"patch: 3016-033 < expected at line %ld of patch.\n"
EXPDASH	"patch: 3016-034 --- expected at line %ld of patch.\n"
EXPGTR	"patch: 3016-035 > expected at line %ld of patch.\n"
CANTSWAP	"patch: 3016-036 Not enough memory to swap next hunk!\n"
MALFORMED	"patch: 3016-037 Malformed patch at line %ld: %s"
$
$ messages for util.c
$
NOREOPEN2	"patch: 3016-038 Internal error, cannot reopen %s\n"
WRFAIL	"patch: 3016-039 Write failed\n"
NOBACKUP	"patch: 3016-040 Cannot backup %s, output is in %s\n"
CANTCR	"patch: 3016-041 Cannot create %s, output is in %s.\n"
WRFAIL2	"patch: 3016-042 Write (%s) failed\n"
SSNOMEM	"patch: 3016-043 Out of memory (savestr)\n"
$
$ message for version.c
$
AIXVERSION	"AIX - based on:\n"
VERSION		"%s\nPatch level: %d\n"
