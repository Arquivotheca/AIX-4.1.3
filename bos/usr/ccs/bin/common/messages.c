static char sccsid[] = "@(#)47  1.7  src/bos/usr/ccs/bin/common/messages.c, cmdprog, bos411, 9428A410j 4/29/93 08:10:57";
/*
 * COMPONENT_NAME: (CMDPROG) messages.c
 *
 * FUNCTIONS:                                                                
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 * @OSF_COPYRIGHT@
 */
/* AIWS C compiler */
#include        "messages.h"

/* msgtext is an array of pointers to printf format strings used in calls to
 * error message printing functions
 * the index into msgtext of each string is "hardwired" hence when adding new
 * messages they should be appended to the end of msgtext and when deleting
 * messages, they should be replaced by empty strings
 *
 * NUMMSGS is defined to be the number of entries is msgtext and it too is a
 * hardwired constant, defined in messages.h
 */

char    *msgtext[] = {
/* [0] */       "%s evaluation order undefined",        /* werror::name */
/* [1] */       "%s may be used before set",    /* werror::name */
/* [2] */       "%s redefinition hides earlier one",    /* uerror::name */
/* [3] */       "%s set but not used in function %s",   /* werror::name,name */
/* [4] */       "%s undefined", /* uerror::name */
/* [5] */       "bad structure offset", /* uerror */
/* [6] */       "%s unused in function %s",     /* werror::name,name */
/* [7] */       "",
/* [8] */       "=<%c illegal", /* uerror::character */
/* [9] */       "=>%c illegal", /* uerror::character */
/* [10] */      "BCD constant exceeds 6 characters",    /* uerror */
/* [11] */      "",
/* [12] */      "ambiguous assignment for non-ansi compilers",  /* warning */
/* [13] */      "argument %s unused in function %s",    /* werror::name,name */
/* [14] */      "array of functions is illegal",        /* uerror */
/* [15] */      "assignment of different structures",   /* uerror */
/* [16] */      "bad asm construction", /* uerror */
/* [17] */      "bad scalar initialization",    /* uerror */
/* [18] */      "",
/* [19] */      "cannot initialize extern within a block",      /* uerror */
/* [20] */      "case not in switch",   /* uerror */
/* [21] */      "comparison of unsigned with negative constant",        /* werror */
/* [22] */      "constant argument to NOT",     /* werror */
/* [23] */      "constant expected",    /* uerror */
/* [24] */      "constant in conditional context",      /* werror */
/* [25] */      "constant too big for cross-compiler",  /* uerror */
/* [26] */      "conversion from long may lose accuracy",       /* werror */
/* [27] */      "conversion to long may sign-extend incorrectly",       /* werror */
/* [28] */      "declared argument %s is missing",      /* uerror::name */
/* [29] */      "default not inside switch",    /* uerror */
/* [30] */      "degenerate unsigned comparison",       /* werror */
/* [31] */      "division by 0",        /* uerror */
/* [32] */      "division by 0.",       /* uerror */
/* [33] */      "duplicate case in switch, %d", /* uerror::number */
/* [34] */      "duplicate default in switch",  /* uerror */
/* [35] */      "empty array declaration",      /* werror */
/* [36] */      "empty character constant",     /* uerror */
/* [37] */      "enumeration type clash, op %s",        /* uerror::operator */
/* [38] */      "field outside of structure",   /* uerror */
/* [39] */      "field too big",        /* uerror */
/* [40] */      "fortran declaration must apply to function",   /* uerror */
/* [41] */      "fortran function has wrong type",      /* uerror */
/* [42] */      "",
/* [43] */      "function %s has return(e); and return;",       /* werror::name */
/* [44] */      "",
/* [45] */      "function has illegal storage class",   /* uerror */
/* [46] */      "function illegal in structure or union",       /* uerror */
/* [47] */      "function returns illegal type",        /* uerror */
/* [48] */      "gcos BCD constant illegal",    /* uerror */
/* [49] */      "illegal array size combination, op %s",        /* werror::operator */
/* [50] */      "illegal break",        /* uerror */
/* [51] */      "illegal character: %03o (octal)",      /* uerror::number */
/* [52] */      "illegal class",        /* uerror */
/* [53] */      "illegal combination of pointer and integer, op %s",    /* werror::operator */
/* [54] */      "",
/* [55] */      "illegal continue",     /* uerror */
/* [56] */      "illegal field size",   /* uerror */
/* [57] */      "illegal field type",   /* uerror */
/* [58] */      "illegal function",     /* uerror */
/* [59] */      "illegal hex constant", /* uerror */
/* [60] */      "illegal indirection",  /* uerror */
/* [61] */      "illegal initialization",       /* uerror */
/* [62] */      "illegal lhs of assignment operator",   /* uerror */
/* [63] */      "illegal member use: %s",       /* uerror::name */
/* [64] */      "",
/* [65] */      "illegal member use: perhaps %s.%s",    /* werror::name,name */
/* [66] */      "illegal pointer combination, op %s",   /* werror::operator */
/* [67] */      "illegal pointer subtraction",  /* uerror */
/* [68] */      "illegal register declaration", /* uerror */
/* [69] */      "illegal structure pointer combination, op %s", /* werror::operator */
/* [70] */      "illegal type specifier combination",   /* uerror */
/* [71] */      "",
/* [72] */      "illegal use of field", /* uerror */
/* [73] */      "illegal type for enumeration constant", /* uerror */
/* [74] */      "value of enumeration constant is out of range", /* uerror */
/* [75] */      "loop not entered at top",      /* werror */
/* [76] */      "member of structure or union required",        /* uerror */
/* [77] */      "newline in BCD constant",      /* uerror */
/* [78] */      "newline in string or char constant",   /* uerror */
/* [79] */      "",
/* [80] */      "non-constant case expression", /* uerror */
/* [81] */      "non-null byte ignored in string initializer",  /* werror */
/* [82] */      "nonportable character comparison",     /* werror */
/* [83] */      "",
/* [84] */      "nonunique name demands struct/union or struct/union pointer",  /* uerror */
/* [85] */      "null dimension",       /* uerror */
/* [86] */      "null effect",  /* werror */
/* [87] */      "old-fashioned assignment operator",    /* werror */
/* [88] */      "",
/* [89] */      "operands of %s have incompatible types",       /* uerror::operator */
/* [90] */      "pointer required",     /* uerror */
/* [91] */      "possible pointer alignment problem, op %s",    /* werror::operator */
/* [92] */      "precedence confusion possible: parenthesize!", /* werror */
/* [93] */      "precision lost in assignment to (sign-extended?) field",       /* werror */
/* [94] */      "precision lost in field assignment",   /* werror */
/* [95] */      "",
/* [96] */      "redeclaration of %s",  /* werror::name */
/* [97] */      "redeclaration of formal parameter, %s",        /* uerror::name */
/* [98] */      "",
/* [99] */      "",
/* [100] */     "statement not reached",        /* werror */
/* [101] */     "static %s %s unused",  /* werror::thing,name */
/* [102] */     "struct/union %s never defined",        /* werror::name */
/* [103] */     "struct/union or struct/union pointer required",        /* werror */
/* [104] */     "non ansi type specifier long long", /* warning */
/* [105] */     "non ansi integer constant suffix ll", /* warning */
/* [106] */     "",
/* [107] */     "too many characters in character constant",    /* uerror */
/* [108] */     "too many initializers",        /* uerror */
/* [109] */     "type clash in conditional",    /* uerror */
/* [110] */     "unacceptable operand of &",    /* uerror */
/* [111] */     "undeclared initializer name %s",       /* werror::name */
/* [112] */     "undefined structure or union", /* uerror */
/* [113] */     "unexpected EOF",       /* uerror */
/* [114] */     "unknown array size",   /* uerror */
/* [115] */     "unsigned comparison with 0?",  /* werror */
/* [116] */     "void function %s cannot return value", /* uerror::name */
/* [117] */     "void type for %s",     /* uerror::name */
/* [118] */     "void type illegal in expression",      /* uerror */
/* [119] */     "zero or negative subscript",   /* werror */
/* [120] */     "zero size field for %s",       /* uerror::name */
/* [121] */     "zero sized structure", /* uerror */
/* [122] */     "long long in case or switch statement may be truncated", /* warning */
/* [123] */     "long in case or switch statement may be truncated in non-ansi compilers",      /* warning */
/* [124] */     "bad octal digit %c",   /* werror */
/* [125] */     "illegal bit field type, unsigned assumed",     /* werror */
/* [126] */     "",
/* [127] */     "nested comments not supported",        /* uerror */
/* [128] */     "*/ found outside of a comment context",        /* werror */
/* [129] */     "unknown escape sequence \\%c", /* werror */
/* [130] */     "'string literals' mixed with 'wide string literals'",  /* werror */
/* [131] */     "illegal type qualifier combination",   /* uerror */
/* [132] */     "function returns qualified type",      /* werror */
/* [133] */     "const lhs of assignment operator",     /* uerror */
/* [134] */     "basic type cannot mix with struct/union/enum/typedef", /* uerror */
/* [135] */     "struct/union lhs of assignment operator has const member",     /* uerror */
/* [136] */     "only one storage class specifier allowed",     /* uerror */
/* [137] */     "mix of old and new style argument declarations",       /* uerror */
/* [138] */     "old style argument declaration",       /* werror */
/* [139] */     "constant value (0x%x) exceeds (0x%x)", /* werror */
/* [140] */     "declaration is missing declarator",    /* uerror */
/* [141] */     "declaration must have explicit type specifiers",       /* uerror */
/* [142] */     "extraneous comma",     /* uerror */
/* [143] */     "structure members must be terminated by ';'",  /* uerror */
/* [144] */     "function %s must return a value",      /* werror::name */
/* [145] */     "incomplete type for %s has already been completed",    /* werror::name */
/* [146] */     "operator (%s) in a constant expression",       /* uerror */
/* [147] */     "illegal use of void type",     /* uerror */
/* [148] */     "illegal pointer qualifier combination, op %s", /* werror::operator */
/* [149] */     "cannot take address of register variable",     /* uerror */
/* [150] */     "illegal cast in an integral constant expression",      /* uerror::name */
/* [151] */     "floating point expression in an integral constant expression", /* uerror::name */
/* [152] */     "illegal address constant expression",  /* uerror::name */
/* [153] */     "use of old-style function definition in presence of prototype",        /* werror */
/* [154] */     "illegal typedef declaration",  /* uerror */
/* [155] */     "no name for definition parameter",     /* uerror */
/* [156] */     "illegal cast", /* uerror */
/* [157] */     "wrong number of arguments in function call",   /* uerror */
/* [158] */     "arg list in declaration",      /* uerror */
/* [159] */     "mismatched type in function argument", /* werror */
/* [160] */     "illegal redeclaration of %s",  /* uerror::name */
/* [161] */     "floating point exception detected",    /* uerror */
/* [162] */     "static function %s not defined",       /* uerror */
/* [163] */     "static function %s not defined or used",       /* werror */
/* [164] */     "%s declared both static and extern",   /* uerror */
/* [165] */     "wrong number of arguments in function definition",     /* uerror */
/* [166] */     "cannot initialize function variable",  /* uerror */
/* [167] */     "unknown enumeration",  /* uerror */
/* [168] */     "unknown array size for %s",    /* uerror::name */
/* [169] */     "undefined structure or union for %s",  /* uerror::name */
/* [170] */     "cannot take size of a function",       /* uerror */
/* [171] */     "main() returns random value to invocation environment",        /* werror */
/* [172] */     "incompatible function prototype, op %s",       /* uerror::operation */
/* [173] */     "illegal use of ellipsis",      /* uerror */
/* [174] */     "illegal use of label %s",      /* uerror */
/* [175] */     "array not large enough to store terminating null",     /* werror */
/* [176] */     "illegal structure initialization",     /* uerror */
/* [177] */     "illegal union initialization", /* uerror */
/* [178] */     "mix of old and new style function declaration",        /* warning */
/* [179] */     "non-integral controlling expression of a switch",      /* uerror */
/* [180] */     "partially elided initialization",      /* warning */
/* [181] */     "function prototype not in scope",      /* warning */
/* [182] */     "illegal cast in a constant expression",
/* [183] */     "non portable use of cast", /* warning */
/* [184] */     "cannot take size of a bit field",      /* uerror */
/* [185] */     "redeclaration of parameter %s inside function",        /* uerror::name */
/* [186] */     "storage class not the first type specifier",   /* warning */
/* [187] */     "prototype type mismatch of formal parameter %s",       /* uerror::name */
/* [188] */     "operand of %s has illegal type",       /* uerror::operator */
/* [189] */     "bad floating point constant",  /* uerror */
/* [190] */     "struct/union field cannot have storage class", /* werror */
/* [191] */     "cannot declare incomplete static object",      /* werror */
/* [192] */     "must have at least one declaration in translation unit",       /* werror */
/* [193] */     "external symbol type clash for %s",    /* uerror::name */
/* [194] */     "prototype not compatible with non-prototype declaration",      /* werror */
};
