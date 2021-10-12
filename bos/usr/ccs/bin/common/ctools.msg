$ @(#)86        1.11  src/bos/usr/ccs/bin/common/ctools.msg, cmdprog, bos411, 9428A410j 4/29/93 08:10:07
$
$ COMPONENT_NAME: (CMDPROG) ctools.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1991
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$len
$set MS_CTOOLS
$quote "

$  Message numbers have been assigned a range of 500-699.  
$  Do not translate %d, %s, %1$d, %1$s, etc ...  
$  Message 1-195 are written as standard output. They are not error messages.  
$  Talk with your technical representatives before you translate these. Some  
$  customers may have programs that depend on these messages comming out exactly  
$  as they are.  

1               "%s evaluation order undefined"
$  This message comes out where the order of expression evaluation (like   
$  arithmetic operations) are not defined.  
2               "%s may be used before set"
$  Using a variable before it has a value.  
3               "%s redefinition hides earlier one"
$  Definition of %s is used instead of an earlier definition of %s.  
4               "%s set but not used in function %s"
5               "%s undefined"
6               "bad structure offset"
$  Structure offset is a C-Language programming term.  
7               "%s unused in function %s"
8               ""
9               "=<%c illegal"
$  Do not translate =<%c  
10              "=>%c illegal"
$  Do not translate =>%c  
11              "BCD constant exceeds 6 characters"
$   A BCD constant is any 6 or less characters within the ' character.  
$   For example: 'BCD'  
12              ""
13              "ambiguous assignment for non-ansi compilers"
$  Do not translate ansi  
14              "argument %s unused in function %s"
15              "array of functions is illegal"
16              "assignment of different structures"
$  This message occurs when trying to assign a structure of one form to a  
$  structure of a different form.  
17              "bad asm construction"
$  Do not translate.  
18              "bad scalar initialization"
$  Scalar is a mathematical technical term.  
19              ""
20              "cannot initialize extern within a block"
$  Do not translate extern.  
$  A block is a programming concept for a group of statements.  
21              "case not in switch"
$  Do not translate case and switch.  
22              "comparison of unsigned with negative constant"
23              "constant argument to NOT"
$  NOT is referring to the C-Language "not" operator, the (!) character.  
24              "constant expected"
25              "constant in conditional context"
$  This message occurs when using a constant in a condition check.  
26              "constant too big for cross-compiler"
27              "conversion from long may lose accuracy"
$  long is a C-Language type. Do not translate long.  
28              "conversion to long may sign-extend incorrectly"
$  Conversion to the long format may not have propagated the sign bit correctly.  
$  Do not translate long.  
29              "declared argument %s is missing"
30              "default not inside switch"
$  Do not translate default, switch.  
31              "degenerate unsigned comparison"
$  This message occurs when using an unsigned number in a comparision that will  
$  always give the same result. For example, if UNS > 0, where UNS is an   
$  unsigned variable.  
$  Unsigned is a C-Language type.   
32              "division by 0"
33              "division by 0."
34              "duplicate case in switch"
$  Do not translate case, switch.  
35              "duplicate default in switch"
$  Do not translate default, switch.  
36              "empty array declaration"
$  declaration is a C-Language term.  
37              "empty character constant"
38              "enumeration type clash, op %s"
$  Op is operator. Enumeration is a C-Language technical term referring to the   
$  enum type. Clash means they are not compatible types.  
$    
39              "field outside of structure"
$  field refers to a bit field.  
40              "field too big"
$  field refers to a bit field.  
41              "fortran declaration must apply to function"
$  Do not translate.  
42              "fortran function has wrong type"
$  Do not translate.  
43              ""
44              "function %s has return(e); and return;"
$  Do not translate return(e) and return.   
45              ""
46              "function has illegal storage class"
47              "function illegal in structure or union"
$  Structure and union are C-language types. Do not translate.  
48              "function returns illegal type"
49              "gcos BCD constant illegal"
$  Do not translate gcos.  
50              "illegal array size combination, op %s"
$  Op is operator.  
51              "illegal break"
$  break is C-Language term. Do not translate.  
52              "illegal character: %03o (octal)"
53              "illegal class"
$  class is a C-Language technical term.  
54              "illegal combination of pointer and integer, op %s"
55              ""
56              "illegal continue"
$  continue is C-Language term. Do not translate.  
57              "illegal field size"
58              "illegal field type"
59              "illegal function"
60              "illegal hex constant"
61              "illegal indirection"
$  Indirection is a programming technical term that deals with pointers.   
62              "illegal initialization"
63              "illegal lhs of assignment operator"
$  lhs = left hand side.  
64              "illegal member use: %s"
$  Member is the field inside a structure.  
65              ""
66              "illegal member use: perhaps %s.%s"
67              "illegal pointer combination, op %s"
$  op is operator.  
$  pointer is a programming technical term.  
68              "illegal pointer subtraction"
$  Pointer is a programming technical term.  
69              "illegal register declaration"
70              "illegal structure pointer combination, op %s"
$  op is operator.  
71              "illegal type specifier combination"
72              ""
73              "illegal use of field"
74              "illegal type for enumeration constant"
75              "value of enumeration constant is out of range"
76              "loop not entered at top"
77              "member of structure or union required"
$  Do not translate structure, union.  
78              "newline in BCD constant"
$  newline is the newline character ( 
$ ).  
79              "newline in string or char constant"
$  char = character  
$  newline is the newline character ( 
$ ).  
80              ""
81              "non-constant case expression"
$  Do not translate case.  
82              "non-null byte ignored in string initializer"
83              "nonportable character comparison"
$  Making a comparision with a character constant that may port differently to  
$  other platforms  
84              ""
85              "nonunique name demands struct/union or struct/union pointer"
$  Names that are not unique require ...  
$  Do not translate struct/union.  
86              "null dimension"
$  Programming concept that does not do anything.  
87              "null effect"
$  Have a statement that does not do anything.  
88              "old-fashioned assignment operator"
$  An assignment operator that is no longer allowed.   
89              ""
90              "operands of %s have incompatible types"
91              "pointer required"
92              "possible pointer alignment problem, op %s"
$  op is operator.  
$  There may be boundary alignment problems.  
93              "precedence confusion possible: parenthesize!"
94              "precision lost in assignment to (sign-extended?) field"
$  When assigning something to a "signed" field, precision may be lost.  
95              "precision lost in field assignment"
96              ""
97              "redeclaration of %s"
98              "redeclaration of formal parameter, %s"
99              ""
100             ""
101             "statement not reached"
$  This message refers to a statement that will never get executed.  
102             "static %s %s unused"
$  Do not translate static.  
103             "struct/union %s never defined"
$  Do not translate struct/union.  
104             "struct/union or struct/union pointer required"
$  Do not translate struct/union.  
105             "non ansi type specifier long long"
106             "non ansi integer constant suffix ll"
107             ""
108             "too many characters in character constant"
109             "too many initializers"
110             "type clash in conditional"
111             "unacceptable operand of &"
$  The & character is a C-Language operator.  
112             "undeclared initializer name %s"
$  This message occurs when initializing a variable with another variable that  
$  has not yet been initialized itself.  
113             "undefined structure or union"
114             "unexpected EOF"
$  EOF = End of file.  
115             "unknown array size"
116             "unsigned comparison with 0?"
117             "void function %s cannot return value"
$  Void is a C-Language function type. Void function is a function that is   
$  defined as not returnning a value.  
$  Do not translate void.   
118             "void type for %s"
119             "void type illegal in expression"
120             "zero or negative subscript"
121             "zero size field for %s"
122             "zero sized structure"
123             "long long in case or switch statement may be truncated"
124             "long in case or switch statement may be truncated in non-ansi compilers"
$  Do not translate long, case, switch and ansi.  
125             "bad octal digit %c"
126             "illegal bit field type, unsigned assumed"
127             ""
128             "nested comments not supported"
129             "*/ found outside of a comment context"
$  Do not translate */  
130             "unknown escape sequence \\%c"
131             "'string literals' mixed with 'wide string literals'"
$  "string literals" and "wide string literals" are technical terms.  
132             "illegal type qualifier combination"
$  type qualifier is a technical programming concept.  
133             "function returns qualified type"
134             "const lhs of assignment operator"
$  lhs = left hand side.  
$  Do not translate const.  
135             "basic type cannot mix with struct/union/enum/typedef"
$  Basic type refers to the C-Language Base types.  
$  Do not translate struct union enum typedef.  
136             "struct/union lhs of assignment operator has const member"
$  Do not translate struct union const.  
$  lhs = left hand side.  
137             "only one storage class specifier allowed"
$  Storage class is a technical concept.  
138             "mix of old and new style argument declarations"
$  old and new refers to old C-Language and new ansi specifications.  
139             "old style argument declaration"
140             "constant value (0x%x) exceeds (0x%x)"
141             "declaration is missing declarator"
$  Declaration is a C-Language term.  
$  This message means your declare statement is missing the thing you are trying  
$  to declare.  
142             "declaration must have explicit type specifiers"
$  Declaration is a C-Language term.  
143             "extraneous comma"
144             "structure members must be terminated by ';'"
145             "function %s must return a value"
146             "incomplete type for %s has already been completed"
$  The type declaration has already been resolved.  
147             "operator (%s) in a constant expression"
148             "illegal use of void type"
$  Do not translate void.  
149             "illegal pointer qualifier combination, op %s"
$  op is operator.  
150             "cannot take address of register variable"
151             "illegal cast in an integral constant expression"
$  Cast is a C-Language technical concept.  
$  This message means you cannot cast an integral constant in  
$  this situation.  
152             "floating point expression in an integral constant expression"
153             "illegal address constant expression"
154             "use of old-style function definition in presence of prototype"
$  Prototype is ansi's new way of defining functions. Cannot mix prototype and  
$  non-prototype function definitions.  
155             "illegal typedef declaration"
$  Do not translate typedef.  
156             "no name for definition parameter"
$  A function definition parameter is missing a name.   
157             "illegal cast"
158             "wrong number of arguments in function call"
159             "arg list in declaration"
$  arg is parameter.  
$  declaration is a C-Language term.  
160             "mismatched type in function argument"
161             "illegal redeclaration of %s"
162             "floating point exception detected"
163             "static function %s not defined"
$  Do not translate static.  
164             "static function %s not defined or used"
$  Do not translate static.  
165             "%s declared both static and extern"
$  Do not translate static, extern.  
166             "wrong number of arguments in function definition"
167             "cannot initialize function variable"
168             "unknown enumeration"
$  Do not translate enumeration.  
169             "unknown array size for %s"
170             "undefined structure or union for %s"
$  Do not translate structure, union.  
171             "cannot take size of a function"
172             "main() returns random value to invocation environment"
$  Do not translate main()  
173             "incompatible function prototype, op %s"
$  prototype is ansi's new way of defining functions. Cannot mix prototype and  
$  non-prototype function definitions.  
$  op is operator.  
174             "illegal use of ellipsis"
$  ellipsis = ...  
175             "illegal use of label %s"
176             "array not large enough to store terminating null"
177             "illegal structure initialization"
178             "illegal union initialization"
179             "mix of old and new style function declaration"
180             "non-integral controlling expression of a switch"
$  Do not translate switch.  
181             "partially elided initialization"
182             "function prototype not in scope"
$  Scope is a C-Language concept. Do not translate.  
183             "illegal cast in a constant expression"
$  Cast is a C-Language technical concept.  
$  This message means you cannot cast an char constant in  
$  this situation.  
184             "non portable use of cast"
185             "cannot take size of a bit field"
186             "redeclaration of parameter %s inside function"
187             "storage class not the first type specifier"
$  storage class is a technical concept.  
188             "prototype type mismatch of formal parameter %s"
$  prototype is ansi's new way of defining functions. Cannot mix prototype and  
$  non-prototype function definitions.  
189             "operand of %s has illegal type"
190             "bad floating point constant"
191             "struct/union field cannot have storage class"
$  Do not translate struct union.  
$  Storage class is a technical concept.  
192             "cannot declare incomplete static object"
193             "must have at least one declaration in translation unit"
$  declaration is a C-Language term.  
194             "external symbol type clash for %s"
195             "protoype not compatible with non-prototype declaration"
$  Do not translate.  
M_MSG_195       "1286-500 Out of tree space.\n\
\tUse the -Ntx flag with x greater than %d."
$  Do not translate -Ntx  or x
M_MSG_196       "op: %1$d, val: %2$ld\n"
$  Do not translate.  
M_MSG_197       "1286-501 Internal wasted space: %o"
M_MSG_198       "1286-502 Internal error releasing empty tree.\n"
M_MSG_199       "\"%1$s\", line %2$d: error: "
$  You may translate line and error. Please keep it as small as possible.  
$ Do not remove the blank at the end of the message, or the quotes.
$ Do not change the order.
M_MSG_200       "1286-503 There is not enough memory available now.\n"
M_MSG_201       "1286-504 Internal error; code %d is not known."
M_MSG_202       "1286-505 Internal error; Cannot fold nil expression."
$ Do not translate fold.  
M_MSG_203       "1286-506 Structure is causing unexpected result."
M_MSG_204       "1286-507 Internal error in oconvert: %d"
$ Do not translate oconvert.  
M_MSG_205       "1286-508 Internal error in opact operation %d"
$ Do not translate opact.  
M_MSG_206       "sizeof operation will return a 0"
$  sizeof is a C-Language operator. Do not translate sizeof.  
M_MSG_207       "The type table mark has a prototype list that is not resolved."
M_MSG_208       "1286-509 The type table mark is missing."
M_MSG_209       "1286-510 The type table level is not valid."
M_MSG_210       "1286-511 Out of type nodes.\n\
\tUse the -Nlx flag with x greater than %d."
$  Do not translate -Nlx  or x
M_MSG_211       "1286-512 Internal error; Cannot qualify generic structure."
M_MSG_212       "1286-513 There is not enough memory available now.\n"
M_MSG_213       "1286-514 Character %03o (octal) is not recognized."
M_MSG_214       "1286-515 Internal error; The yylex function failed on switch."
$  yylex is the lexical analyzer function. Do not translate switch, yylex.  
M_MSG_215       "1286-516 Internal error; The action for AR_?? is not known."
$  Do not translate AR_??  
M_MSG_216       "1286-517 Internal error because of defid call."
$  Do not translate defid.  
M_MSG_217       "1286-518 The tyreduce function causes an internal error."
$  Do not translate tyreduce.  
M_MSG_218       "1286-519 Internal error was found on paramstk stack table."
$  Do not translate paramstk.  
M_MSG_219       "1286-520 There is not enough memory available for parameter stack.\n"
M_MSG_220       "1286-521 bcsave internal error."
$  Do not translate bcsave.  
M_MSG_221       "1286-522 Internal error; parameter not reset."
M_MSG_222       "1286-523 switch internal error."
$  Do not translate switch.  
M_MSG_223       "Structure name is not known for tagging."
M_MSG_224       "1286-524 structure member is causing unexpected result."
M_MSG_225       "1286-525 parameter is not processed."
M_MSG_226       "1286-526 Size for type 0%o is not known."
M_MSG_227       "1286-527 Internal error; Defining function variable %s"
M_MSG_228       "1286-528 Size for %s type 0%o is not known."
M_MSG_229       "1286-529 Alignment initialization failed."
M_MSG_230       "1286-530 Internal error; inoff."
$  Do not translate inoff.  
M_MSG_231       "Reached the initialization stack limit of 32."
M_MSG_232       "a floating point constant in a NOFLOAT compiler?"
$  Do not translate. This is an obsolete message.  
M_MSG_233       "1286-531 Cannot process more local variables."
M_MSG_234       "1286-532 The structure is too large."
M_MSG_235       "1286-533 The filename cannot exceed 2048 characters."
M_MSG_236       "1286-534 Internal error; The function treewalk failed."
M_MSG_237       "1286-535 Cannot have more than 15 nested structs in a type definition."
$  Do not translate struct.  
M_MSG_238       "1286-536 Internal error in the first parameter of the tymerge function."
$  Do not translate tymerge.  
M_MSG_239       "%d is not a valid class."
$  Class is a C-Language technical term.  
M_MSG_240       "1286-537 Use the -Nnx flag to increase the symbol table size."
$  Do not translate -Nnx 
M_MSG_241       "check error: %s"
$  Do not translate.  This is a debug message only.
M_MSG_242       "%1$s check at level %2$d"
$  Do not translate.  This is a debug message only.
M_MSG_243       "1286-538 There is not enough memory available now.\n"
M_MSG_244       "1286-539 Internal error; No more items on prototype stack."
M_MSG_245       "1286-540 There is not enough memory available now.\n"
M_MSG_246       "1286-541 Internal error; Cannot optimize nil expression."
M_MSG_247       "The & operator is not used correctly."
M_MSG_248       "1286-542 Internal error in a conversion operation."
M_MSG_249       "1286-543 Internal error in the incode function: field > int"
$  Do not translate incode. 
M_MSG_250       "bad storage class %d"
$  Do not translate.  
M_MSG_251       "sdb value error on %s\n"
$  Do not translate.  
M_MSG_252       "1286-544 Use the -Ndx flag to increase the dimension table size."
$  Do not translate -Ndx 
M_MSG_253       "1286-545 Use the -Ndx flag to increase the dimension table size."
$  Do not translate -Ndx 
M_MSG_254       "1286-546 Cannot have more than 300 unnamed variables."
M_MSG_255       "1286-547 Internal error; Cannot identify type of generated tagname."
$  Do not translate type, tagname.  
M_MSG_256       "1286-548 Internal error; Cannot identify type of tagname."
$  Do not translate type, tagname.  
M_MSG_257       "Cannot continue: too many errors found."
M_MSG_267       "%1$s, line %2$d: cflow error: "
$  You may translate line and error. Please keep it small as possible.  
M_MSG_268       "%1$s, line %2$d: cxref error: "
$  You may translate line and error. Please keep it small as possible.  
M_MSG_269       "outdouble: bad arguments\n"
$  Do not translate.  
M_MSG_270       "1286-549 Internal error in the locctr function: STAB is not used."
$  Do not translate locctr and STAB.  
M_MSG_271       "1286-550 The location counter is not valid."
M_MSG_272       "1286-551 parameter is not valid."
M_MSG_273       "1286-552 An option on the -N flag is not correct."
M_MSG_274       "Lost temp file"
$  Do not translate.  
M_MSG_275       "1286-553 There is an error in the function level."
M_MSG_276       "1286-554 There is an error in the parameter level."
M_MSG_277       "\"%1$s\", line %2$d (\"%3$s\"): error: "
$  You may translate line and error. Please keep it small as possible.  
M_MSG_278       "1286-555 There are too many nested compound statements."
M_MSG_279       "1286-556 Internal error on while loop code generation."
$  Do not translate while. 
M_MSG_280       "1286-557 Internal error in the bdty function."
$  Do not translate bdty.  
M_MSG_281       "1286-558 Use the -Ndx flag to increase the dimension table size."
$  Do not translate -Ndx 
M_MSG_282       "1286-559 There are too many nested compound statements."
M_MSG_283       "1286-560 Cannot have more than 2000 case statements per switch."
$  Do not translate switch, case.  
M_MSG_284       "Version: %s\n"
$  Do not translate.  
M_MSG_285       "%1$s type:%2$o qual:%3$o other:%4$o\n"
$  Do not translate.  
M_MSG_286       "1286-561 Internal error; The unhide function failed."
$  Do not translate unhide.  
M_MSG_287       "structure tagname not found"
$  Do not translate.  
M_MSG_288       "%1$s, line %2$d: error: "
$  You may translate line and error. Please keep it small as possible.  
M_MSG_289       "Cannot recover from earlier errors: Exiting.\n"
M_MSG_290       "%1$s, line %2$d: lint error: "
$  You may translate line and error. Please keep it small as possible.  
M_MSG_291       "%1$s, line %2$d: compiler error: "
$  Do not translate.  
M_MSG_292       "\"%1$s\", line %2$d: warning: "
$  You may translate line and warning. Please keep it small as possible.  
M_MSG_293       "\"%1$s\", line %2$d (\"%3$s\"): warning: "
$  You may translate line and warning. Please keep it small as possible.  
M_MSG_294       "%1$s, line %2$d: warning:"
$  You may translate line and warning. Please keep it small as possible.  
M_MSG_299       "ccom: Cannot open %s\n"
$  Do not translate. This message is obsolete.  
M_MSG_300       "cxref: 1286-562 Cannot open %s\n"
M_MSG_301       "lint: 1286-563 Cannot open temporary file.\n"
M_MSG_302       "1286-564 %s is not a valid class."
M_MSG_303       "FIELD[%d]"
$  Do not translate. 
M_MSG_320       "Cannot have more than 29 nested blocks."
M_MSG_321       "Cannot specify more than 13 dimensions."
M_MSG_322       "Function prototypes should not use the lint /*VARARGSn*/"
$  Do not translate lint /*VARARGSn*/ 
M_MSG_323       "Invalid Character\n"
