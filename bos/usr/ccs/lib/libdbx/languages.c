static char sccsid[] = "@(#)52	1.8.2.6  src/bos/usr/ccs/lib/libdbx/languages.c, libdbx, bos411, 9428A410j 1/12/94 16:18:55";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: findlanguage, language_define, language_init, language_op,
 *	      language_setop, stampfindlang
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * Language management.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "languages.h"
#include "cplusplus.h"
#include <sys/debug.h>
#include <storclass.h>

public Language primlang;

/* language stamps used by 'ld' to indicated start and end */
/* entries of symbol table, these entries are also used to */
/* store stabstrings compaction level of the object.       */
public Language tb_front;
public Language tb_back;

private Language head;

/*
 * Initialize language information.
 *
 * The last language initialized will be the default one
 * for otherwise indistinguised symbols.
 */

public language_init()
{
    primlang = language_define("$builtin symbols", ".?");
    tb_front = language_define("$start symbols", ".?");
    tb_back = language_define("$end symbols", ".?");
    c_init();
    cpp_init(); 				 
    fortran_init();
    pascal_init();
    cobol_init();
    asm_init();
}


/*
 * NAME: stampfindlang
 *
 * FUNCTION: Utilitize language stamp placed into the symbol 
 *           table to determine the language of a binary object (.o).
 *	     Also return values of special Lang ID from 'ld' containing
 *	     stabstring compaction level.
 *
 * PARAMETERS:
 *   stamps     - two byte short containing:
 * o Source Language ID (Byte 1)
 *   Language stamp with values as defined for Exception Section.
 *   This field is valid only when CPU ID contain an valid (non-zero)
 *   CPU ID.
 *   See also /usr/include/storclass.h for language stamp values.
 * o CPU Version ID (Byte 2)
 *   CPU stamp which identifies the target cpu hardward version
 *   of the binary object.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Language pointer to language of object.
 *	    Or pointer to assembler language if language stamp
 * 	    is undefined or unsupported.
 *	    Or special symbol table start or end value (for 
 *          stabstring compaction level) from 'ld'.
 *          Or nil if cpu stamp is invalid.
 */
public Language stampfindlang(stamps)
unsigned short stamps;
{
   unsigned short lang_stamp = 
		      (unsigned short) ((stamps & 0xff00) >> 8); /* byte 1 */
   unsigned short cpu_stamp = 
		      (unsigned short) (stamps & 0x00ff);        /* byte 2 */
   Language lang;

   if (cpu_stamp) {
       /* if cpu_stamp is ok, use lang_stamp to determine language */
       switch (lang_stamp) {
            case TB_C:              lang = cLang; break;
            case TB_FORTRAN:        lang = fLang; break;
            case TB_PASCAL:         lang = pascalLang; break;
            case TB_COBOL:          lang = cobLang; break;
            case TB_CPLUSPLUS:      lang = cppLang; break;
            case TB_ASM:            lang = asmLang; break;
#ifdef TB_FRONT
            /* special lang ID's for start and end of symbol table */
            case TB_FRONT:	    lang = tb_front; break;
            case TB_BACK:	    lang = tb_back; break;
#endif
            /* unsupported dbx languages get asmLang as default */
            case TB_ADA:
            case TB_PL1:
            case TB_BASIC:
            case TB_LISP:
            case TB_MODULA2:
            case TB_RPG:
            case TB_PL8:
            default:		    lang = asmLang; break;
       }
   } else {
      /* if cpu_stamp is no good, return nil */
      lang = (Language) nil;
   }
   return lang;
}

public Language findlanguage(suffix)
String suffix;
{
    Language lang;
    Boolean found = false;
    struct SUFLIST *sl;

    lang = head;
    if (suffix != nil) {
	while (lang != nil) {
	    for (sl = lang->suflist; sl != nil; sl = sl->next) {
	        if (streq(sl->suffix, suffix)) {
		    found = true;
		    break;
		}
	    }
	    if (found)
	        break;
	    else
	        lang = lang->next;
	}
	if (lang == nil) {
	    lang = head;
	}
    }
    return lang;
}

private Language find_language_name(name)
String name;
{
    Language lang;

    lang = head;
    if (name != nil) {
	while (lang != nil and not streq(lang->name, name))
	    lang = lang->next;
    } else
        lang = nil;
    return lang;
}

public Language language_define(name, suffix)
String name;
String suffix;
{
    Language p;
    struct SUFLIST *sl;

    p = find_language_name(name);
    if (p ==  nil) {
        p = new(Language);
	p->name = name;
	p->suflist = (struct SUFLIST *)malloc(sizeof(struct SUFLIST));
	p->suflist->suffix = suffix;
	p->suflist->next = nil;
	p->next = head;
	head = p;
    }
    else {
        sl = (struct SUFLIST *)malloc(sizeof(struct SUFLIST));
	sl->suffix = suffix;
	sl->next = p->suflist;
	p->suflist = sl;
    }
    return p;
}

public language_setop(lang, op, operation)
Language lang;
LanguageOp op;
LanguageOperation operation;
{
    checkref(lang);
    assert(ord(op) < ord(L_ENDOP));
    lang->op[ord(op)] = operation;
}

public LanguageOperation language_op(lang, op)
Language lang;
LanguageOp op;
{
    LanguageOperation o;

    checkref(lang);
    o = lang->op[ord(op)];
    checkref(o);
    return o;
}
