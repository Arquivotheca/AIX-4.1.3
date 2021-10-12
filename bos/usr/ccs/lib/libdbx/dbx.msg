$ @(#)42	1.24.7.2  src/bos/usr/ccs/lib/libdbx/dbx.msg, libdbx, bos41J, 9508A 1/25/95 16:28:26
$
$ COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
$
$ FUNCTIONS: 
$
$ ORIGINS: 26, 27, 83
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
$ Copyright (c) 1982 Regents of the University of California
$
$
$ LEVEL 1,  5 Years Bull Confidential Information
$

$quote "         define quote character.

$ Notes to translators:
$ Throughout this message catalog, there are occurrences of \" at the
$ beginning and ending of messages.  These are places where dbx is 
$ constructing messages by parsing evaluation trees, and the quotes 
$ and the position must be left intact.  This could not be avoided
$ due to the nature of dbx.  When there is a %s substitution character
$ in between the quotes, then the quote-%s-quote triplet may be moved
$ as a unit where necessary or desirable.
$
$ Also, dbx is an interactive program debugger.  Most of the messages
$ in this catalog are responses to commands which the user has typed.
$ The context is usually supplied by the command which was just entered
$ by the user; verbose messages returned by the debugger would likely
$ not be appreciated.  
$
$ The only messages which have the dbx component information (1283-XXX)
$ are those which come from unrecoverable errors.  All other messages are
$ part of the expected output of dbx, and should not have component id
$ information.
$
$ dbx fatal errors have messsage numbers 001-199
$ dbx panic messages have messsage numbers 200-399

$set MS_steps        Define initial set#

MSG_405              "cannot step through protected code at 0x%x\n"
$ Attempt to single-step program into privileged address space
MSG_407              "cannot step through breakpoint at 0x%x\n"
$ Attempt to single-step through an instruction which is same as a breakpoint
MSG_409              "1283-240 error %d trying to step process"
$ Attempt to continue process resulted in an error
MSG_413              "program exited\n"
$ The process being debugged has completed with return code 0
MSG_414              "program exited with code %d\n"
$ The process being debugged has completed with a non-zero return code

$set MS_frame        Redefine set#

MSG_383              "could not locate trace table from starting address 0x%x"
$ dbx was unable to locate a proper procedure-end table from this location
MSG_388              ""
MSG_395		     ""
MSG_399              ""
MSG_400              ""
MSG_403              "[internal error: nil function containing parameter %s]"
$ dbx encountered an unusual condition in the symbol table where a parameter
$ had no associated subprogram.
MSG_404            "[internal error: nil paramlist for function containing %s]"
$ dbx encountered an unusual condition in the symbol table where a subprogram
$ had no associated parameters.
MSG_406              "[internal error: bad size %d in pushretval]"
$ dbx encountered an internal error attempting to display the return value
$ of a function.

$set MS_decode        Redefine set#

MSG_386              "    Invalid opcode."
$ The word does not translate into a valid machine instruction
MSG_478              "[bad size in extractField -- word assumed]\n"
$ A bitfield has been specified to have a length greater than the word size


$set MS_tree        Redefine set#

MSG_379              "cannot take address of register"
$ Attempt to get address of variable which resides in a register
MSG_381              "expected variable, found \""
$ dbx expected a variable in a command and got something else instead
MSG_425              "1283-245 Cannot display evaluation tree for unknown operation %d"
$ The operation to be evaluated is not a valid dbx operation
MSG_447              "too many arguments to type rename"
$ Too many operands were given for a type-rename (casting) operation
MSG_622              "cannot take address of an enumeration constant"
$ Attempt to get address of an enumeration constant

$set MS_symbols        Redefine set#

MSG_361              "1283-241 delete of non-symbol '%s'"
$ dbx attempted to remove a non-existent symbol from its internal symbol table
MSG_366              ""
MSG_367              "\"%s\" is not currently defined"
$ Attempt to use a non-global variable from a subprogram not currently in use.
MSG_368              "[internal error: unexpected nil frame for \"%s\"]"
$ dbx was unable to process the stack while attempting to get the value of a
$ local variable.
MSG_369              "1283-242 address: bad symbol \"%s\""
$ dbx was unable to determine the address of a parameter due to bad information
$ regarding the storage type of the parameter.
MSG_370              "could not find link to type reference"
$ dbx was unable to determine the type information associated with a pointer 
MSG_371              "could not resolve reference"
$ dbx was unable to determine the type information associated with a pointer 
MSG_372              "dynamic bounds not currently available"
$ Attempt to process an array which has limits which cannot be determined yet
MSG_374              "[internal error: unexpected array bound type]"
$ dbx was unable to process the bounds information for an array
MSG_375              "1283-243 missing size information for record"
$ The size information for a record type was omitted by the compiler
MSG_376              "expected range for set base type"
$ dbx was unable to process the base type for a Pascal set type
MSG_377              "1283-244 size: bad class (%d)"
$ dbx was unable to determine the size of an unexpected object type
MSG_378              "cannot compute size of a %s\n"
$ There is no size information for the specified object type
MSG_380              "\" is not a pointer"
$ User attempted to use dereference a variable which is not a pointer
MSG_382              "\" is improper type"
$ User attempted to negate or complement a non-numeric variable
MSG_385              "operation not defined on \""
$ User attempted an arithmetic or logical operation on a expression for
$ which the operation is undefined.
MSG_389              "non-numeric operand for division or exponentiation"
$ User attempted a division or exponentiation operation on a non-numeric
$ operand.
MSG_390              "non-integer operand for div or mod"
$ User attempted a division or modulus operation on a non-numeric operand.
MSG_391              "\"%s\" is not defined in "
$ The user specified a symbol which does not exist within the specified block.
MSG_394              "Too many subscripts."
$ The user specified more dimensions than exist for an array variable.
MSG_396              "\" is not an array"
$ The user specified dimensions for a symbol which is not an array or pointer.
MSG_397              "Too many subscripts for \""
$ The user specified more dimensions than exist for an array variable.
MSG_398              "Not enough subscripts for \""
$ The user specified less dimensions than exist for an array variable.
MSG_401              ", expected boolean expression"
$ The user used a logical operation on a non-boolean expression
MSG_402              "\"%s\" is not defined"
$ The user specified a symbol which does not exist.
MSG_410              ""
MSG_416              "elements of a set must be of the same base type"
$ dbx encountered an error in a set vaariable description because the 
$ members of the set did not all have the same type.
MSG_417       "elements of a set must be of type char, int, or scalar constant"
$ dbx encountered an error in a set vaariable description because the 
$ members of the set were not specified correctly.
MSG_419		"Operation not defined on edited or alpha data types."
MSG_620              "%1$s\"%2$s\" is not defined in "
$ Part of message indicating that given name with the given type is not defined
MSG_621              "%1$s\"%2$s\" is not defined"
$ Indicates that given name with the given type is not defined
MSG_627              "\"%1$s\" is not %2$sfield in "
$ The given name is not a field of the given type in the following name
MSG_635              "missing size information for class"
$ dbx cannot find the size information for a class.

$set MS_stabstring        Redefine set#

MSG_270              "1283-228 expected char '%c', found '%s'"
$ The compiler has produced an improper debug information string.
MSG_271              ""
MSG_341              "[internal error: unknown constant type '%c']"
$ The compiler has produced a debug string for a constant with unknown type.
MSG_345              "1283-229 bad input on type \"%s\" at \"%s\""
$ The compiler has referenced a non-existent type in a debug string.
MSG_346              "1283-230 too many types in file \"%s\""
$ The compiler has exceeded the limit for number of types generated for 1 file.
MSG_347              "1283-231 bad input on tag \"%s\" at \"%s\""
$ The compiler has referenced a non-existent tag in a debug string.
MSG_350              "missing ';' after type attributes"
$ The compiler has produced an improper debug information string.
MSG_354              "1283-232 index(\"%s\", ':') failed"
$ The compiler has produced an improper debug information string.
MSG_362              ""
MSG_363              "1283-233 bad import symbol entry '%s'"
$ The modula-2 compiler has produced an improper debug information string.
MSG_364              "1283-234 bad opaque symbol entry '%s'"
$ The modula-2 compiler has produced an improper debug information string.
MSG_528              "1283-235 consPic: unknown cobol storage type '%c'"
$ The COBOL compiler has produced an improper debug information string.
MSG_529              "1283-236 consPic: cobol picture definition had no internal size"
$ The COBOL compiler has produced an improper debug information string.
MSG_530              "1283-237 consPic: expected edit description"
$ The COBOL compiler has produced an improper debug information string.
MSG_531              "1283-238 consPic: wrong size edit description"
$ The COBOL compiler has produced an improper debug information string.
MSG_532              "1283-014 consPic: malloc error!"
$ dbx was unable to allocate memory to process a COBOL information string.
MSG_533              "1283-239 consPic: expected cobol decimal alignment"
$ The COBOL compiler has produced an improper debug information string.
MSG_631              "expected access type 'i', 'o', or 'u', saw '%1$s'"
$ The C++ compiler has produced an improper debug information string.
MSG_632              "expected member function types, found '%1$s'"
$ The C++ compiler has produced an improper debug information string.
MSG_633              "expected member function attribute, found '%1$s'"
$ The C++ compiler has produced an improper debug information string.
MSG_634              "expected member attribute 's', 'p', 'r' or 'b', found '%s'"
$ The C++ compiler has produced an improper debug information string.
MSG_423              "1283-016 type limit (%d) exceeded in file \"%s\".\n\
Use the -t option to increase type limit."
$ The number of type identifiers found in the object file symbol table
$ has exceeded the limit.  The -t option may be used to increase the limit.
MSG_426              "Unexpected end of string for \"%s\""
$ Compiler has produced \" at end of string.
MSG_641              "1283-246 string_alloc: malloc error"
$ dbx was unable to allocate memory to process a name string.

$set MS_source        Redefine set#

MSG_272              "could not read \"%s\"\n"
$ dbx was unable to read the named source file
MSG_273              "line number must be positive\n"
$ The specified line number was a negative number.
MSG_327              "\"%s\" has only 1 line\n"
$ The specified line number was greater than the number of lines in the file.
MSG_328              "\"%s\" has only %d lines\n"
$ The specified line number was greater than the number of lines in the file.
MSG_340              "second number must be greater than first\n"
$ The ending specified line number was smaller than the beginning line number.
MSG_348              ""
MSG_352              "cannot read \"%s\""
$ dbx was unable to read the named source file
MSG_353              "no source for \"%s\""
$ The specified source file was not compiled to support symbolic debugging.
MSG_356              "no source file\n"
$ The user attempted to search through a non-existent file for a pattern.
MSG_357              ""
MSG_358              "invalid regular expression\n"
$ The user specified a pattern which could not be compiled into a regular expression.
MSG_359              "no match\n"
$ The user specified a pattern which did not match anything in the 
$ current source file.
MSG_360              "warning: could not read command file \"%s\", action ignored.\n"
$ The user attempted to execute commands through a non-existent file.

$set MS_scanner        Redefine set#

MSG_274              "alias expansion too large"
$ The expanded version of the aliased command exceeds the length of
$ the dbx command buffer (10240 characters).
MSG_310              "missing ')' in macro call"
$ User did not close the parameter list to an aliased command with a '('.
MSG_313              "not enough parameters in macro call"
$ User did not specify enough parameters for an aliased command.
MSG_317              "too many parameters in macro call"
$ User specified too many parameters for an aliased command.
MSG_318              "missing actuals for macro"
$ User did not specify parameters for an aliased command requiring parameters.
MSG_329              "^ unrecognized command"
$ User specified a command which is not a dbx command or user alias.
$ The '^' must be left at the beginning of the message.
MSG_334              "^ syntax error"
$ User specified improper syntax for a dbx command or user alias.
$ The '^' must be left at the beginning of the message.
MSG_335              "non-terminated string"
$ User did not close a character string with a quote character.
MSG_337              "cannot open %s"
$ dbx was unable to read a file which is supposed to contain dbx commands.
MSG_339              "unreasonable input nesting on \"%s\""
$ The nesting of command files has exceeded the dbx limit of ten.

$set MS_runtime        Redefine set#

MSG_262              "Frame information in error at 0x%x"
$ dbx was unable to process the stack information associated with the address.
MSG_265              "program is not active"
$ User program has completed or has not begun.
MSG_269              "Could not determine current function"
$ dbx was unable to determine the currently active function.
MSG_276              "no current function"
$ dbx has no value for the currently active function.
MSG_298              "not that many levels"
$ User has attempted to traverse past the end of the stack.
MSG_300              "Frame information in error, %d levels up."
$ dbx was unable to process the stack information %d levels up.
MSG_307              "cannot find _exit"
$ dbx was unable to set a breakpoint at the address of _exit.
MSG_308              "Nesting of \"call\" commands is illegal."
$ User is attempting to run a call subcommand before a previous call subcommand
$ has completed.
MSG_309              ""
MSG_314              "\"%s\" is not a procedure or function"
$ User is attempting to run a call subcommand of a symbol which is not a
$ subprogram.
MSG_315              "too many parameters"
$ User is attempting to pass too many parameters to a subprogram from via
$ call subcommand.
MSG_316              "type mismatch for %s"
$ User is attempting to pass the wrong type of parameter to a subprogram 
$ via the call subcommand.
MSG_319              ""
MSG_321              "not enough parameters to %s"
$ User is attempting to pass too few parameters to a subprogram via the
$ call subcommand.
MSG_322              " returns successfully\n"
$ User has successfully invoked a subprogram via the call subcommand.
MSG_323              "\"%s\" has no return value. Use call instead of print."
$ User has attemped to invoke a subprogram which has no return value via
$ the print subcommand rather than the call subcommand.
MSG_618              "\"this\" pointer evaluates to nil."
$ User is attempting to invoke a C++ member function when a nil "this" ptr.
MSG_619              "\"this\" object not required!"
$ User is attempting to invoke a C++ non-member function with "this" ptr.
MSG_636              "Cannot call a pure virtual function."
$ User has managed to call a pure virtual by explicitly qualifying its name.

$set MS_process        Redefine set#

MSG_267              "Core file is older than current program (core ignored)"
$ Core file specified or found has a date which precedes the date of the
$ program to be debugged.  The core file will not be processed.
MSG_277              "1283-013 could not attach to pid %d\n"
$ User was not allowed to debug the process running with process id %d.
MSG_278              "Waiting to attach to process %d ...\n"
$ dbx is waiting for the system's response to request to debug the process
$ running with process id %d.
MSG_279              "Attaching to process from exec...\n"
$ dbx is waiting for the system's response to request to debug the process
$ which has performed an exec() while multiprocess debug mode is enabled.
MSG_280              "1283-225 cannot fork"
$ dbx was unable to start a process from which to run user's program
MSG_281              "warning: cannot execute %s\n"
$ dbx was unable to run the user's program
MSG_282              "program is not active"
$ User program has completed or has not begun.
MSG_285              "1283-226 error %d trying to continue process"
$ dbx was unable to continue the user's program due to a system error.
MSG_289              ""
MSG_290              ""
MSG_291              "Unreadable instruction at address 0x%x"
$ dbx was unable to read the instruction at address %x in user's program.
MSG_292              ""
MSG_293              ""
MSG_294              "store: could not write instruction at 0x%x"
$ dbx was unable to write an instruction at address %x in user's program.
MSG_295              "store: could not write data at 0x%x"
$ dbx was unable to write data at address %x in user's program.
MSG_296              ""
MSG_297              ""
MSG_536              ""
MSG_537	 "Core file program (%s) does not match current program (core ignored)"
$ Core file specified or found has a program name which differs from the name
$ of the program to be debugged.  The core file will not be processed.
MSG_538		     "%s is not a valid core file (core ignored)"
$ Core file specified or found is not a valid AIX Version 3 core file.  The
$ core file will not be processed.
MSG_550		     "%s does not contain a stack (core ignored)"
$ Core file specified or found does not contain the stack.  The
$ information to make a core file useful (the stack) is not here.  The
$ core file will not be processed.
MSG_301  "Invalid trace table.  Unreadable instruction at address 0x%x"
$ an invalid trace table value resulted in an attempt to read an
$ instruction at address %x in the user's program.
$ dbx was unable to read the instruction at address %x in user's program.

$set MS_printsym        Redefine set#

MSG_261              "\n\
entering %s "
$ This is trace information which tells the user that the subprogram which
$ they have asked to trace has been called.
MSG_264              "leaving %s "
$ This is trace information which tells the user that the subprogram which
$ they have asked to trace has completed.
MSG_284              "*** expression too large ***"
$ The expression to evaluate is too large for the dbx expression evaluation
$ stack.
MSG_311              ""
MSG_312              ""
MSG_320              "record has no fields"
$ dbx is unable to evaluate the specified record because it has no member
$ fields.
MSG_325   "1283-224 unable to evaluate floating point variable of length %d bytes"
$ dbx is unable to evaluate the specified floating point variable because it
$ has a format which dbx does not know how to interpret.

$set MS_pascal        Redefine set#

MSG_338              "1283-223 unexpected class %d for parameter"
$ The Pascal compiler has produced an improper debug information string.
MSG_342        "internal error: expected range or enumerated base type for set"
$ The Pascal compiler has produced an improper debug information string.
MSG_343              "subscript %d out of range [%d..%d]"
$ User has specified a subscript to an array which is not within the bounds
$ of the array.
MSG_344              ""
MSG_415              "set member out of range"
$ User is attempting to assign a value to a Pascal set, but at least one member
$ is not within the allowed values for the set.
MSG_418		     "must specify index when printing space type"
$ User must specify a subscript when printing a Pascal space variable.

$set MS_object        Redefine set#

MSG_117              "1283-011 nesting depth limit (%d) exceeded.\n\
Use the -d option to increase nesting depth limit."
$ The number of nested scoping blocks found in the object file symbol table
$ has exceeded the limit.  The -d option may be used to increase the limit.
MSG_160              ""
MSG_167		     "1283-010 cannot open member %s in file %s"
$ dbx was unable to process a shared object from within an archive file.
MSG_169	             "%1$s is newer than %2$s" 
$ The source file for a compiled object is newer than the program.
MSG_173              "1283-221 nesting depth underflow (%d)"
$ dbx has encountered an error processing nested scoping blocks found in the
$ object file symbol table.
MSG_174              ""
MSG_219              "1283-012 cannot open %s"
$ dbx was unable to process an object file associated with the user program.
MSG_228              "no source compiled with -g"
$ The user program was not compiled with option to support symbolic debugging.
MSG_259              "no source compiled with -g\n"
$ The user program was not compiled with option to support symbolic debugging.
MSG_260              "warning:  stab entry unrecognized: "
$ The user program symbol table contains an improper debug information string.
MSG_263              "1283-222 chain not nil in deffunc"
$ dbx encountered an internal error when defining a function symbol
MSG_266              "global %s ignored"
$ dbx is ignoring an occurrence of a previously defined external symbol
MSG_424              "1283-017 invalid value for include file %s"
$ dbx has encountered an error in processing the object file symbol
$ table.  There is an invalid value field in the symbol table for the
$ include file.
MSG_640              "unsupported binder stabstring compaction level %d, resetting to 1"
$ dbx has encountered an unsupported binder stabstring compaction level in 
$ the symbol table.

$set MS_modula        Redefine set#

MSG_154              ""
MSG_302              "1283-218 unexpected class %d for parameter"
$ The modula-2 compiler has produced an improper debug information string.
MSG_324              "1283-219 expected range or enumerated base type for set"
$ The modula-2 compiler has produced an improper debug information string.
MSG_330              "subscript value %d out of range [%d..%d]"
$ User has specified a subscript to an array which is not within the bounds
$ of the array.
MSG_331              "subscript %d out of range [%d..%d]"
$ User has specified a subscript to an array which is not within the bounds
$ of the array.
MSG_332              "[internal error: expected array in evalaref]"
MSG_333              "1283-220 initial Modula-2 type number too large for '%s'"

$set MS_mappings        Redefine set#

MSG_111              "source file \"%s\" not compiled with -g"
$ The source file was not compiled with option to support symbolic debugging.

$set MS_main        Redefine set#

MSG_89               "dbx is initializing ...\n"
MSG_99               "reading symbolic information ..."
$ Heading telling user that dbx is processing the symbolic information 
$ associated with the user program. 
MSG_136              "[using memory image in %s]\n"
$ Heading telling user that dbx is processing a core file associated
$ with the user program. 
MSG_229              "1283-004 missing directory for -I"
$ User did not specify a directory to be added to the file search list.
MSG_230              "1283-005 missing command file name for -c"
$ User did not specify a command file to be processed.
MSG_231              "1283-006 missing proc id number for -a"
$ User did not specify a running process id to debug.
MSG_236              ""
MSG_237              ""
MSG_249              "1283-008 extraneous argument %s"
$ User improperly placed options after the name of the user program. 
MSG_250              "Successfully attached to %s.\n"
$ Header notifying user that dbx is able to debug the specified running
$ process.
MSG_251              "Could not determine object file name"
$ dbx was unable to determine the name of the object file associated
$ with the specified running process.
MSG_252              "enter object file name (default is `%s', ^D to exit): "
$ dbx was unable to access the specified user program.  dbx then prompts the
$ user to submit another name for the user program or exit dbx by typing
$ control-D.
MSG_254        "1283-002 exiting due to failure to determine object file"
$ dbx was unable to access the specified user program.
MSG_255              "cannot read %s"
$ dbx was unable to read the specified user program.
MSG_256              "1283-206 cannot open /dev/mem"
$ dbx was unable to read the memory device.
MSG_258              "1283-009 unknown option '%c'"
$ User specified an unknown option to dbx.
MSG_100		     "Type 'help' for help.\n"
$ Message telling user how to get help from dbx.
MSG_544		     "Directory containing %s could not be determined.\n\
Apply 'use' command to initialize source path.\n"
$ dbx was unable to determing the directory where the object file associated
$ with a running process exists.  The user must use the dbx use subcommand
$ to specify the directory where the source files may be found.
MSG_248	 "1283-007 Nesting depth limit was not specified after -d option"
$ User did not specify a limit to the depth of nested scoping blocks.
MSG_253		     "Specified nesting limit could not be interpreted.\n\
Nesting limit used will be 25."
$ User specified a non-numeric limit to the depth of nested scoping blocks.
MSG_257	     "Specified nesting limit is greater than maximum of 32767.\n\
Nesting limit used will be 32767."
$ User specified too large of a limit to the depth of nested scoping blocks.
MSG_420	 "1283-015 Type limit was not specified after -t option"
$ User did not specify a limit to the number of type identifiers. 
MSG_421		     "Specified type limit could not be interpreted.\n\
Type limit used will be 5000."
$ User specified a non-numeric limit to the number of type identifiers.
MSG_422	     "Specified type limit is greater than maximum of 32767.\n\
Type limit used will be 32767."
$ User specified too large of a limit to the number of type identifiers.
MSG_275              "enter object file name (default is `%s', %s to exit): "
$ dbx was unable to access the specified user program.  dbx then prompts the
$ user to submit another name for the user program or exit dbx by typing
$ control-D.

$set MS_keywords        Redefine set#

MSG_171              "%s is not aliased"
$ User asked for the expansion of a command which is not a recognized alias.
MSG_172              "\"%s\" is a program symbol -- use assign"
$ User used the 'set' command to change a program variable.
MSG_223              "%s is not set"
$ User attempted to remove a debugger variable which was not set.
MSG_783		     "Valid values are \"function\", \"module\", and \"none\"\n"
$ User attempted to set $stepignore to something other than
$ "function", "module", or "none".
MSG_784		     "Invalid value\n\
\tUsage: set $instructionset = {\"pwr\", \"pwrx\", \"ppc\", \"601\",\n\
\t\t\"603\", \"604\", \"com\", \"any\", \"default\"}\n"
$ User attempted to set $instructionset to something other than valid values
MSG_785		     "Invalid value\n\
\tUsage: set $mnemonics = {\"pwr\", \"ppc\", \"default\"}\n"
$ User attempted to set $mnemonics to something other than valid values

$set MS_fortran        Redefine set#

MSG_80               "Error: element size is %d characters, not %d\n"
$ User attempted to assign an incorrect number of characters to a FORTRAN
$ character* variable.
MSG_232              "source file \"%s.f\""
$ This a response to user's query about a symbol which is a source file name.
MSG_233              "executable file \"%s\""
$ This a response to user's query about a symbol which is a program name.
MSG_235              "class %s in fortran_printdecl"
$ dbx cannot evaluate a symbol because the symbol is not a FORTRAN expression.
MSG_239              "1283-215 unexpected class %d for parameter"
$ The FORTRAN compiler has produced an improper debug information string.
MSG_240              "1283-216 cannot evaluate \"%d\" byte logical, complex,  or real"
$ dbx cannot evaluate a FORTRAN logical, complex, or real because the length
$ is unexpected.
MSG_241              "1283-217 printval: bad class %d"
$ dbx cannot evaluate the FORTRAN expression because of unexpected debug
$ information.
MSG_242              "do not know how to print a %s"
$ dbx cannot evaluate the FORTRAN expression because of unexpected debug
$ information.
MSG_243              "unknown type in fortran printint"
$ dbx cannot evaluate the FORTRAN expression because of unexpected debug
$ information.
MSG_244              "dynamic bounds not currently available"
$ Attempt to process an array which has limits which cannot be determined yet
MSG_246              "subscript out of range"
$ User has specified a subscript to an array which is not within the bounds
$ of the array.
MSG_234              "data type \"%s\""
$ User has asked for the declaration of a FORTRAN symbol which is a data type
$ specification.

$set MS_execute        Redefine set#

MSG_103              "Attempt to resolve \"wildcards\" in run or rerun failed."
$ dbx was unable to interpret the parameters to a run or rerun subcommand.
MSG_105              "too many arguments"
$ dbx was unable to pass all of the parameters to the user program because
$ there were too many.  The limit is 1000 parameters.
MSG_109              "multiple input redirects"
$ User has specified more than one file to redirect input to the user program.
$ Only one is allowed, and only the first file specified would be used.
MSG_110              "cannot continue execution"
$ dbx cannot continue running the user program because the user program is
$ not known to be in a stopped state.
MSG_113              "program terminated by signal %d"
$ The user program ended because it received a %d signal.
MSG_114              "program exited"
$ The user program exited with a return code of 0.
MSG_115              "program exited with code %d"
$ The user program exited with a return code of %d.
MSG_153              "%s is not active"
$ The user program is not running.
MSG_155              "no place to return to"
$ The return subcommand could not be completed because dbx could not find a
$ return address for the named fucntion.
MSG_161              ""
MSG_162              "Failed establishing calling point (libg.a not linked)"
$ The call subcommand could not be completed because the user program did not
$ include the dbx call-support library libg.a.
MSG_163              "\n\
Entering debugger ...\n"
$ Heading produced when user has invoked dbx with the -r option and the user
$ program has ended unexpectedly.
MSG_175              "no process to write to"
$ User is attempting to alter the contents of memory of a non-running program.
MSG_215              "cannot write to process (address 0x%x)"
$ dbx was not allowed by the system to alter the contents of a memory location.
MSG_216  "application forked, child pid=%d, process stopped, awaiting input \n"
$ Heading produced for a user program which has used a fork() system call
$ while debugging in multiprocess debugging mode.
MSG_217          "1283-003 Could not reattach to child process on fork"
$ dbx encountered a system error attempting to debug the child process of a
$ user program in multirprocess debugging mode.
MSG_218            "debugging child, pid=%d, process stopped, waiting input \n"
$ Heading produced for debugging the child process of a user program which has
$ used a fork() system call in multiprocess debugging mode.
MSG_220              ""
MSG_225              "1283-001 cannot read \"%s\""
$ dbx was unable to read the specified user program.
MSG_226              "Address out of bounds for text section."
$ User specified an address which was not a valid text address for the program.
MSG_227              "Address out of bounds for data section."
$ User specified an address which was not a valid data address for the program.
MSG_539		     "Unable to locate linkage symbol for out-of-module call"
$ dbx is unable to complete a call to a subprogram which is not in the 
$ current load module because there is insufficient information in the user
$ program for a call to the named subprogram.
MSG_623		     "signal \"%1$s\" unknown"

$set MS_examine        Redefine set#

MSG_104              "non-positive repetition count"
$ User specified a non-positive number of machine instructions to display.
MSG_112              "bad print format \"%s\""
$ User specified an invalid memory display format.
MSG_151              "first address larger than second"
$ User specified an invalid range of memory to display.  The starting address
$ was after the ending address.
MSG_165              "\"%s\" terminated normally\n"
$ The user program exited with an exit code of 0.
MSG_166              "\"%s\" terminated abnormally (exit code %d)\n"
$ The user program exited with a non-zero exit code.
MSG_176              "\n\
execution completed (exit code %d)\n"
$ The user program completed with a non-zero exit code.
MSG_177              "\n\
execution completed\n"
$ The user program completed with an exit code of 0.
MSG_178		"Instruction is valid on: %s\n"
$ Informing user which instruction sets the executed instruction is valid on

$set MS_events        Redefine set#

MSG_78            "1283-213 cannot find symbol for program counter ($pc) "
$ An internal error within dbx indicating that the internal symbol which
$ corresponds to the program counter of the user's program could not be
$ located.  This will make some stop and trace commands have unpredicatable
$ behavior.
MSG_79               ""
MSG_101              "no executable code at line "
$ The user attempted to place a breakpoint at a source line number which
$ no runnable code associated with it in the user program.  This line
$ is followed by an interpretation of the specified source-line.
MSG_102              "1283-214 bad ancestry for \"%s\""
$ dbx was unable to determine the lowest nesting scope of a user expression.
MSG_133              ""
MSG_542              "no breakpoints found at line "
$ The user attempted to remove all of the breakpoints at a source
$ line where no breakpoints could be found.  This message is followed 
$ by an interpretation of the specified source-line.
MSG_543              "no breakpoints found at address "
$ The user attempted to remove all of the breakpoints at an address
$ where no breakpoints could be found.  This message is followed 
$ by an interpretation of the specified address.

$set MS_eval        Redefine set#

MSG_77               "1283-207 stack underflow"
$ This is an internal error in dbx.  The dbx expression evaluation
$ stack has been corrupted due to an earlier error in evaluation.
MSG_82               "\"%s\" is not active"
$ The user has specified a local variable which is defined within a
$ subprogram which is not in use.  Such a variable has no address or
$ value in that situation.
MSG_83               "cannot evaluate a %s"
$ The user has specified a symbol which cannot be evaluated because there
$ is no method to evaluate a symbol of this type.
MSG_84               "reference through nil pointer"
$ The symbol which is being evaluated is supposed to contain an address
$ of an object, but contains zero instead.  Evaluation cannot continue.
MSG_85               "error: division by 0"
$ User has specified an expression containing a division by zero.
MSG_88               "error: mod by 0"
$ User has specified an expression containing a modulus by zero.
MSG_90               "cannot read \"%s\""
$ dbx was unable to read the named source file
MSG_95               "Symbols are not folded (mixed).\n"
$ dbx is not changing the case of user-typed symbol names
MSG_96               "Symbols are folded to lower case.\n"
$ dbx is changing the case of user-typed symbol names to lower case
MSG_97               "Symbols are folded to upper case.\n"
$ dbx is changing the case of user-typed symbol names to upper case
MSG_98               "Symbols are folded based upon current language.\n"
$ dbx is changing the case of user-typed symbol names based upon the
$ programming language currently associated with the user program.
MSG_106              "run or rerun allowed only on initial process"
$ User has attempted to restart a process which dbx cannot restart.
MSG_107              "unknown event %ld"
$ User has attempted to delete a stop or trace event which does not match
$ any known stop or trace events.
MSG_121              ""
MSG_124              "1283-208 Could not alter multi-processing mode."
$ dbx has encountered a system error when trying to change the multiprocess
$ debugging mode.
MSG_125              "1283-209 Could not detach from process.  Use quit."
$ dbx has encountered a system error when trying to detach from the user
$ program.  The user must use the quit subcommand in order to end the
$ the debugging session.
MSG_126              "1283-210 eval: bad op %d"
$ This is an internal error in dbx.  The dbx expression tree
$ contains an operation which is unknown to dbx.
MSG_128              "expression too large to evaluate"
$ A user expression is too large for the dbx evaluation stack.  It will not
$ be evaluated.
MSG_129              "1283-211 bad size %d in pushsmall"
$ dbx has encountered an internal error in evaluating an expression.
MSG_130              "[internal error: nil frame for %s]"
$ dbx has encountered an error in the user stack information when attempting
$ to evaluate a machine register.
MSG_131              "[internal error: size %d in popsmall]"
$ dbx has encountered an internal error in evaluating an expression.
MSG_132              "expression too large to trace (limit is %d bytes)"
$ User is attempting to trace an expression which is too large for the
$ internal trace evaluation buffer.  This limit is 512 bytes.
MSG_138              "%s is an unusual operand for examining modification"
$ User is attempting to trace modification to a memory location which is
$ not normally modifiable, such as a subprogram entry point.
MSG_139              "Unable to allocate memory for character pointer"
$ dbx has encountered a system error when attempting to allocate storage in
$ the user's program for an assigned character string.
MSG_140              "$frame must be an address"
$ User has attempted to set the $frame debugger variable to a value which
$ is not a proper address.
MSG_141              "no source lines for \"%s\""
$ User has attempted to list the source lines to a subprogram which has
$ no associated line number information in the user program.
MSG_143              "line numbers must be positive\n"
$ User has specified a non-positive line number to start listing lines from.
MSG_144              "second number must be greater than first\n"
$ User has specified a larger beginning than ending line number to list.
MSG_145              "No file to move within\n"
$ User has issued a move subcommand when there is no current file.
MSG_146              "Line specified is not within range of %s\n"
$ User has attempted to move the current line number to be listed to a
$ line number which is outside of the range of the current source file.
MSG_147              ""
MSG_148              ""
MSG_149              "No assembly code for that source line number"
$ User has attempted to list the machine instructions for a source line
$ number which has no associated machine instructions in the user program.
MSG_180              "%s is not a procedure or function"
$ User has attempted to change the current function to a symbol which is
$ not a subprogram.
MSG_181              "program is not executable"
$ User has attempted to change the program counter of a program which 
$ cannot be run.
MSG_182 	     "Goto address is not within current function. \
(set $unsafegoto to override)"
$ User has specified to change the program counter of the user program
$ to an address which is outside of the current function.  This is 
$ not allowed unless the debugger variable $unsafegoto has been set by
$ the user.
MSG_190              "run                    - begin execution of the program"
$ dbx help synopsis for the run subcommand.
MSG_191            "print <exp>            - print the value of the expression"
$ dbx help synopsis for the print subcommand.
MSG_192            "where                  - print currently active procedures"
$ dbx help synopsis for the where subcommand.
MSG_193              "stop at <line>         - suspend execution at the line"
$ dbx help synopsis for the 'stop at' subcommand.
MSG_194      "stop in <proc>         - suspend execution when <proc> is called"
$ dbx help synopsis for the 'stop in' subcommand.
MSG_195              "cont                   - continue execution"
$ dbx help synopsis for the cont subcommand.
MSG_196              "step                   - single step one line"
$ dbx help synopsis for the step subcommand.
MSG_197       "next                   - step to next line (skip over calls)"
$ dbx help synopsis for the next subcommand.
MSG_198              "trace <line#>          - trace execution of the line"
$ dbx help synopsis for the 'trace line' subcommand.
MSG_199              "trace <proc>           - trace calls to the procedure"
$ dbx help synopsis for the 'trace subprogram' subcommand.
MSG_200              "trace <var>            - trace changes to the variable"
$ dbx help synopsis for the 'trace variable' subcommand.
MSG_201       "trace <exp> at <line#> - print <exp> when <line> is reached"
$ dbx help synopsis for the 'trace expression at source line' subcommand.
MSG_202       "status                 - print trace/stop's in effect"
$ dbx help synopsis for the status subcommand.
MSG_203       "delete <number>        - remove trace or stop of given number"
$ dbx help synopsis for the delete subcommand.
MSG_204       "screen                 - switch dbx to another virtual terminal"
$ dbx help synopsis for the screen subcommand.
MSG_205              "call <proc>            - call a procedure in program"
$ dbx help synopsis for the call subcommand.
MSG_206       "whatis <name>          - print the declaration of the name"
$ dbx help synopsis for the whatis subcommand.
MSG_207              "list <line>, <line>    - list source lines"
$ dbx help synopsis for the list subcommand.
MSG_208              "registers              - display register set"
$ dbx help synopsis for the registers subcommand.
MSG_209              "quit                   - exit dbx"
$ dbx help synopsis for the quit subcommand.
MSG_210              "%s: file already exists, use \">!\""
$ User has attempted to redirect output of a dbx subcommand to a file which
$ already exists without specifying to overwrite the existing file.
MSG_211              "cannot create %s"
$ dbx has encountered a error attempting to redirect subcommand output to the
$ user-specified output file.
MSG_212              "1283-212 standard out dup failed"
$ dbx has encountered a error attempting to restore standard output.
MSG_545              "\t[unset $noflregs to view floating point registers]"
$ Do not translate 'unset $noflregs'.  This is an informational message to
$ the user that the floating point registers are not included with the
$ output from the registers subcommand because the $noflregs debugger variable
$ is set.
MSG_116              "Subarray of pointer to array not supported."
$ User has attempted to print a subarray of a pointer to an array, which is
$ not allowed.
MSG_600              "Cannot set a breakpoint in a pure virtual function."
$ User has attempted to set a breakpoint in a pure virtual function.
MSG_601              "Cannot assign a value to a const variable."
$ User has attempted assign to a const without $unsafeassign set.
MSG_602              "Cannot assign to a pointer to member variable."
$ User has attempted assign to a pointer to member.
MSG_637              "Cannot trace an uninitialized reference variable."
$ User has attempted trace an uninitialized reference.
MSG_782              "Goto address is not within current function or block. \
(set $unsafegoto to override)"
$ MSG_782 replaces MSG_782
MSG_731            "Usage: \"thread [run | ready | susp | term]\" takes no id\n"
$ User has attempted to supply a thread identifier to thread options
$ "run", "ready", "susp", or "term".
MSG_732              "Usage: \"thread current\" takes zero or one id\n"
$ User has attempted to supply more than one thread identifiers to 
$ the thread "current" option
MSG_733              "Usage: \"thread run_next\" takes one id\n"
$ User has attempted to supply none or more than one thread identifiers to 
$ the thread "run_next" option
MSG_734              "Usage: \"condition { wait | nowait }\" takes no id\n"
$ User has attempted to supply conditon identifiers to the 
$ condition "wait" or "nowait" options
MSG_735              "Usage: \"mutex { wait | nowait | lock | unlock }\" \
takes no id \n"
$ User has attempted to supply mutex identifiers to the 
$ mutex "wait", "nowait", "lock", or "unlock" options
MSG_736              "thread                 - list existing threads"
$ dbx help synopsis for the thread subcommand.
MSG_737              "mutex                  - list existing mutexes"
$ dbx help synopsis for the mutex subcommand.
MSG_738             "condition              - list existing condition variables"
$ dbx help synopsis for the condition subcommand.
MSG_739              "attribute              - list existing attributes"
$ dbx help synopsis for the attribute subcommand.
MSG_213              "quit                   - exit dbx (program terminated)"
$ dbx help synopsis for the quit subcommand.
$ MSG_213 replaces MSG_209
MSG_214              "detach                 - exit dbx without terminating \
program" 
$ dbx help synopsis for the detach subcommand.
MSG_638              "Cannot assign to a subarray of variables."
$ User has attempted assign to a range.
MSG_639              "Cannot trace or stop a %s."
$ User has attempted a trace or stop on untraceable symbol.
MSG_711            "dbx multproc fails. dbx continued with multproc disabled\n"
MSG_712            "dbx subcommand screen fails. dbx continued\n"
MSG_713 "screen subcommand can only be invoked from the originating process.\n"
MSG_714            "screen subcommand can only be invoked once.\n"

$set MS_cplusplus        Redefine set#

MSG_35               "Memory allocation error in mangle_member_name()"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information for a user C++ program.
MSG_55               "Memory allocation error in buildFuncList()"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information for a user C++ program.
MSG_59               "Memory allocation error in SymList insertsl()"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information for a user C++ program.
MSG_61               "Memory allocation error in fix_mf_arg_list()"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information for a user C++ program.
MSG_76               "Memory allocation error in printClassMembers()"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information for a user C++ program.
MSG_87               ""
MSG_91               "unexpected missing type information"
$ The type information associated with a C++ tag was unexpectedly missing
$ from the debug information for a user C++ program.
MSG_92               ""
MSG_93               "source file \"%s.C\""
$ This a response to user's query about a symbol which is a source file name.
MSG_94              "executable file \"%s\""
$ This a response to user's query about a symbol which is a program name.
MSG_629		     "static data member %1$s not defined; removing"
$ The given static data member has been tossed by the loader 
MSG_630		     "member function %1$s not defined; removing"
$ The given member function has been tossed by the loader 

$set MS_coredump        Redefine set#

MSG_9                ""
MSG_534              "Unable to access address 0x%x from core""
$ dbx was unable to read the specified user address from the core file.
MSG_535		     "%s is not a valid core file (core ignored)"
$ The user has specified a core file which is not a valid AIX Version 3
$ core file.
MSG_551              "The core file is truncated.  You may need to increase\
the ulimit\nfor file and coredump, or free some space on the filesystem."
$ One or more sections are missing from the core file. 

$set MS_check        Redefine set#

MSG_0                "incompatible sizes"
$ The user has attempted to a assign a value to a variable, but the size
$ of the expression is not the same as the size of the variable, so the
$ assignment is not performed.
MSG_1                "incompatible types"
$ The user has attempted to a assign a value to a variable, but the type
$ of the expression is not the same as the type of the variable, so the
$ assignment is not performed.
MSG_3                "invalid signal number"
$ The signal which the user has specified is not a known signal name or
$ valid signal number.  The subcommand is not performed.
MSG_5                "\"%s\" is not a block"
$ The user has used a symbol which is not a block in a context where only
$ a block is allowed.
MSG_8                "expected a symbol, found \""
$ The user has used an expression where only a symbol name is allowed.
MSG_10               "\"%s\" is not a procedure or function"
$ The user has used a symbol which is not a subprogram name where only a
$ subprogram name is allowed.
MSG_13               "\"%s\" is empty"
$ User has attempted to access source lines to a subprogram which has
$ no associated line number information in the user program.
MSG_15               "\" not call-able"
$ User is attempting to run a call subcommand of a symbol which is not a
$ subprogram.
MSG_16               "symbol not defined"
$ User is attempting to find all instances of a symbol name for which there
$ no instances.
MSG_17               "unexpected \"at\" or \"in\""
$ User has specified invalid syntax for a stop or trace subcommand.
MSG_20               "cannot trace expressions"
$ User has specified an expression to trace.  Only variables can be traced.
MSG_22               "missing variable to watch"
$ User omitted the variable to be traced.
MSG_23               "expected variable to watch, found "
$ User has specified an expression to trace.  Only variables can be traced.
MSG_26               "expected variable, found "
$ User has specified an invalid expression to a stop subcommand in a position
$ where only a variable is allowed.
MSG_27               "expected subprogram, found "
$ User has specified an invalid expression to a stop subcommand in a position
$ where only a subprogram name is allowed.
MSG_28               "\"%s\" is not a subprogram"
$ The user has used a symbol which is not a subprogram in a context where only
$ a subprogram is allowed.
MSG_30               "missing line"
$ The user has omitted the line number in a stop or trace subcommand.
MSG_31               "expected source line number, found \"%t\""
$ The user has incorrectly specified the line number in a stop or trace
$ subcommand.
MSG_32               "missing address"
$ The user has omitted the address in a subcommand.
MSG_33               "expected address, found \""
$ The user has incorrectly specified the address in a subcommand.

$set MS_c        Redefine set#

MSG_2                ""
MSG_6                "unexpected missing type information"
$ The type information associated with a C tag was unexpectedly missing
$ from the debug information from the user program.
MSG_7                ""
MSG_11               "source file \"%s.c\""
$ This a response to user's query about a symbol which is a source file name.
MSG_12               "executable file \"%s\""
$ This a response to user's query about a symbol which is a program name.
MSG_18               ""
MSG_19               ""
MSG_21               "1283-201 printtype: class %s"
$ dbx has encountered an internal error in the subprogram which prints the
$ declaration of C objects.
MSG_34               "%ld byte real"
$ The type of the symbol which the user has asked about is a floating-point
$ type which is %ld bytes in length.
MSG_40               "%d byte real "
$ The type of the symbol which the user has asked about is a floating-point
$ type which is an unexpected %ld bytes in length.
MSG_51               "unresolved tag %s"
$ The type information associated with a C tag was unexpectedly missing
$ from the debug information from the user program.
MSG_53               "(class %d)"
$ dbx cannot display the type of a C variable because of unexpected debug
$ information.
MSG_57               "1283-202 unexpected class %d for parameter"
$ The C compiler has produced an improper debug information string.
MSG_58               ""
MSG_62               "subscript must be integer-compatible"
$ User has specified a subscript to an array which is not an integral type.
MSG_64               ""
MSG_66               "\" is the wrong type"
$ User has specified a subscript to an array which is not an integral type.
MSG_67               ""
MSG_68               ""
MSG_70               "subscript out of range"
$ User has specified a subscript to an array which is not compatible bounds
$ of the array.

$set MS_cobol        Redefine set#

MSG_500              ""
MSG_501              ""
MSG_502              ""
MSG_503              ""
MSG_504              ""
MSG_505              "source file \"%s.cbl\""
$ This a response to user's query about a symbol which is a source file name.
MSG_506              "executable file \"%s\""
$ This a response to user's query about a symbol which is a program name.
MSG_507              ""
MSG_508              ""
MSG_509              ""
MSG_510              "no range for an OCCURS"
$ The COBOL compiler has produced an improper debug information string.
MSG_511              "TYPEREF in cobol"
MSG_512              ""
$ The COBOL compiler has produced unexpected debug information.
MSG_513              "(class %d)"
$ dbx cannot display the type of a COBOL variable because of unexpected debug
$ information.
MSG_514              "out of memory \n"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information from a COBOL program.
MSG_515              "no more memory while creating a cobol level record"
$ dbx has encountered a system error when attempting to allocate storage in
$ processing debug information from a COBOL program.
MSG_516              "1283-203 printtype: class %s"
$ dbx has encountered an internal error in the subprogram which prints the
$ declaration of COBOL objects.
MSG_517              "1283-204 unexpected class %d for parameter"
$ The COBOL compiler has produced an improper debug information string.
MSG_518              "1283-205 printval: bad class %d"
$ dbx cannot evaluate the COBOL expression because of unexpected debug
$ information.
MSG_519              ""
MSG_520              "subscript out of range"
$ User has specified a subscript to an array which is not compatible bounds
$ of the array.
MSG_521              ""
MSG_522              ""
MSG_523              ""
MSG_524              ""
MSG_525              ""
MSG_526              ""
MSG_527              ""
MSG_540  "%1$cD %2$s Record Size %3$d.\nOrganization is %4$s, Access is %5$s."
$ Response to user's query about the declaration of a COBOL file descriptor
$ variable.  FD means file descriptor, SD means sorted access file descriptor.
MSG_541              ""
MSG_546		     " requires %1$d or %2$d subscripts.\n"
MSG_547 	     " requires %d subscript(s).\n"
MSG_548 	     "Subarrays in COBOL not currently supported.\n"
MSG_549	"Operation results in an integer overflow or underflow condition."

$set MS_overload        Redefine set#

MSG_603		     "Unexpected type name as function argument."
$ User has typed a type name as the name of a function argument.
MSG_604		     "Failed to successfully read window size."
$ During overload resolution, the size of the user's window could not be read,
$ disallowing overload selection
MSG_605		     "Window too small to select C++ objects. Enlarge."
$ During overload resolution, the user's window is 0 or one lines long, 
$ disallowing overload selection
MSG_606		     "Too many function calls in expression."
$ To many function calls specified in an expression to successfully resolve
$ overloaded functions
MSG_607		     "Function(s) contain compiler generated parameters."
$ User is calling a function, no match was found for the given arguments, and
$ at least one function was seen with compiler generated parameters, possibly
$ indicating the reason for failure.
MSG_608		 "Given argument list fails to match any definition of %1$s."
$ The user is calling a function, but supplied arguments fail to match any
$ definition of the function.

$set MS_resolve         Redefine set#

MSG_609              "Ambiguous base class \"%1$s\"; please qualify."
$ Multiple base classes with the same name accessible from the given class.
MSG_610              "Please qualify ambiguous reference to \"%1$s\"."
$ Multiple members with the same name accessible from the given class.
MSG_611              " does not contain a definition of class \"%1$s\"."
$ A given block doesn't contain a definition of the given.
MSG_612              "\"%1$s\" is not a nested class of \"%2$s\"."
$ The right class is not a nested class of the left class in a qualification
MSG_628              "\" is not a class."
$ The preceding name is not a class name
MSG_613              "\"%1$s\" is not a class."
$ The given name is not a class name
MSG_614              "\"%1$s\" is not defined"
$ The given name is not a class name
MSG_615              "Too few arguments to type rename."
$ Argument missing to the type rename operator.
MSG_616              "Too many arguments to type rename."
$ Too many arguments to the type rename operator.
MSG_617              "The current module is not a C++ module."
$ Attempt to use the global scope operator :: when the current module is not
$ a C++ module.
MSG_624              "'s type is not \"%1$s\" or derived from \"%2$s\"."
$ An class instance's class is not the same as nor derived from another class.
MSG_625              "\" is not a pointer to a member."
$ The given name is not a pointer to member
MSG_626              "\" does not point to member of class \"%1$s\"."
$ A name is not a pointer to member to the given class


$ Messages for subject sensitive help
$set MS_help        Redefine set#

MSG_646              "\n\nType \"help <command>\" or \"help <topic>\" \
for help on a command or topic.\n"

MSG_647              "Execution subcommands:\n\
run          - begin execution of the program\n\
rerun        - begin execution of program with previous arguments\n\
cont         - continue execution\n\
step         - single step one line\n\
next         - step to next line (skip over calls)\n\
return       - continue until a return to specified procedure is reached\n\
skip         - continue execution ignoring next breakpoint\n\
goto         - change execution to specified source line\n\
stepi        - single step one instruction\n\
nexti        - step to next instruction (skip over calls)\n\
gotoi        - change execution to specified address\n\
up           - move current function up the stack\n\
down         - move current function down the stack\n\
where        - print currently active procedures\n\
call         - execute a procedure in program\n\
print        - execute a procedure and print return code\n\
catch        - trap the signal before it is sent to program\n\
ignore       - stop trapping the signal before it is sent to program\n\
detach       - exit dbx without terminating program\n\
quit         - exit dbx (program terminated)\n"

MSG_648              "When displaying variables and expressions, \
dbx resolves names\n\
first using the static scope of the current function. The\n\
dynamic scope is used if the name is not defined in the first\n\
scope. If static and dynamic searches do not yield a result\n\
an arbitrary symbol is chosen and dbx prints the message\n\
[using <module.variable>]. The <module.variable> is the name\n\
of an identifier qualified with a block name. Override the name\n\
resolution procedure by qualifying an identifier with a block\n\
name. Source files are treated as modules named by the file name\n\
without the language suffix (such as, the .c suffix on a C\n\
language program)\n"

MSG_649                    "Expressions are specified with a subset \
of C and Pascal syntax.\n\
A prefix * or a postfix ^ denotes indirection. Use [ ] or\n\
( ) to enclose array subscripts. Use the field reference\n\
operator . (period) with pointers and records.\n\
This makes the C operator \"->\" unnecessary (although it is\n\
supported).\n\
Types of expressions are checked; the type of an expression\n\
may be overridden by using \"type-name(expression)\".\n\
When there is no corresponding named type use the special\n\
construct &type-name to represent a pointer to the named type.\n\
\"$type-name\" and \"$$tag-name\" can be used to represent a\n\
pointer to a named type or enum, struct or union tag.\n\
\nThe following operators are valid in expressions:\n\
\nAlgebraic    +, -, *, / (float), div (integral), mod, exp\n\
Bitwise      -, |, bitand, xor, ~, <<, >>\n\
Logical      or, and, not, ||, &&\n\
Comparison   <, >, <=, >=, <>, !=, =, ==\n\
Other        sizeof\n"

MSG_650              "File subcommands:\n\
use          - set directories to be searched for source file\n\
list         - list lines of current source file\n\
file         - change or display current source file\n\
func         - change or display current function\n\
search(/?)   - search forward or backward in current file for a pattern\n\
edit         - invoke an editor on specifed file or function\n\
move         - change next line to be displayed by list command\n"

MSG_651              "Machine level subcommands:\n\
display(/)   - display contents of memory\n\
cleari       - remove all stop/trace points at given address\n\
listi        - list instruction from application\n\
stopi        - set a breakpoint at a specified address\n\
tracei       - turn on instruction tracing\n\
stepi        - single step one instruction\n\
nexti        - step to next instruction (skip over calls)\n\
registers    - display register set\n\
gotoi        - change execution to specified address\n\
map          - display address maps and loader information\n"

MSG_652              "Basic dbx command usage:\n\n\
run                    - begin execution of the program\n\
print <exp>            - print the value of the expression\n\
where                  - print currently active procedures\n\
stop at <line>         - suspend execution at the line\n\
stop in <proc>         - suspend execution when <proc> is called\n\
cont                   - continue execution\n\
step                   - single step one line\n\
next                   - step to next line (skip over calls)\n\
trace <line#>          - trace execution of the line\n\
trace <proc>           - trace calls to the procedure\n\
trace <var>            - trace changes to the variable\n\
trace <exp> at <line#> - print <exp> when <line> is reached\n\
status                 - print trace/stop's in effect\n\
delete <number>        - remove trace or stop of given number\n\
screen                 - switch dbx to another virtual terminal\n\
call <proc>            - call a procedure in program\n\
whatis <name>          - print the declaration of the name\n\
list <line>, <line>    - list source lines\n\
registers              - display register set\n\
quit                   - exit dbx\n"

MSG_653              "\tThe following \"set\" variables have special \
meanings:\n\n\
\t$catchbp\n\
\t\tWhen set, dbx catches breakpoints during execution\n\
\t\tof the next command.\n\
\n\
\t$frame\n\
\t\tSetting this variable to an address alters dbx's\n\
\t\tidea of the current stack frame.\n\
\n\
\t$expandunions\n\
\t\tCauses dbx to display values of each part of variant\n\
\t\trecords or unions.\n\
\n\
\t$hexin $octin\n\
\t\tWhen set, dbx interprets addresses in hexadecimal or octal.\n\
\n\
\t$hexchars $hexints $hexstrings\n\
\t\tWhen set, dbx prints characters, integers or\n\
\t\tcharacter pointers in hexadecimal.\n\
\n\
\t$noargs\n\
\t\tWhen set, dbx omits arguments from subcommands, such\n\
\t\tas where, up, down, and dump.\n\
\n\
\t$noflregs\n\
\t\tWhen set, dbx omits the display of floating-point\n\
\t\tregisters from the registers subcommand.\n\
\n\
\t$octints\n\
\t\tWhen set, dbx prints integers in octal.\n\
\n\
\t$ignoreload\n\
\t\tWhen set, dbx ignores load(), unload(), or loadbind()\n\
\t\tsubroutines performed by your program.\n\
\n\
\t$instructionset\n\
\t\tSpecifies the default disassembly mode. To change the\n\
\t\tvalue, the user should type 'set $instructionset = \"<value>\"'.\n\
\t\tIf the $instructionset variable is unset, it will behave\n\
\t\tas if the value was \"default\". The following are the valid\n\
\t\tvalues for $instructionset:\n\
\t\t\"default\" -  Disassemble using the instruction set for the\n\
\t\t             hardware architecture dbx is running on.\n\
\t\t\"com\"     -  Disassemble using the instruction set that is\n\
\t\t             common between Power and PowerPC architectures.\n\
\t\t             The mnemonics will default to PowerPC mnemonics.\n\
\t\t\"pwr\"     -  Disassemble using the instruction set for the\n\
\t\t             RS1 implementation of the Power architecture.\n\
\t\t             The mnemonics will default to Power mnemonics.\n\
\t\t\"pwrx\"    -  Disassemble using the instruction set for the\n\
\t\t             RS2 implementation of Power architecture. The\n\
\t\t             mnemonics will default to Power mnemonics.\n\
\t\t\"ppc\"     -  Disassemble using the instruction set for the\n\
\t\t             PowerPC architecture. The mnemonics will\n\
\t\t             default to PowerPC mnemonics.\n\
\t\t\"601\"     -  Disassemble using the instruction set for the\n\
\t\t             601 implementation of PowerPC architecture. The\n\
\t\t             mnemonics will default to PowerPC mnemonics.\n\
\t\t\"603\"     -  Disassemble using the instruction set for the\n\
\t\t             603 implementation of PowerPC architecture. The\n\
\t\t             mnemonics will default to PowerPC mnemonics.\n\
\t\t\"604\"     -  Disassemble using the instruction set for the\n\
\t\t             604 implementation of PowerPC architecture. The\n\
\t\t             mnemonics will default to PowerPC mnemonics.\n\
\t\t\"any\"     -  Disassemble using any valid instruction from\n\
\t\t             either Power or PowerPC architecture. The\n\
\t\t             mnemonics will default to PowerPC mnemonics\n\
\n\
\t$listwindow\n\
\t\tSpecifies the number of lines to list and listi commands.\n\
\n\
\t$mapaddrs\n\
\t\tWhen set, dbx starts address mapping, useful for kernel\n\
\t\tdebugging.\n\
\n\
\t$menuwindow\n\
\t\tSpecifies the number of lines to list when prompting users\n\
\t\tfor choices.\n\
\n\
\t$mnemonics\n\
\t\tSpecifies the set of mnemonics used for disassembly.\n\
\t\tTo change the value, the user should type 'set $mnemonics\n\
\t\t= \"<value>\"'. If the $mnemonics variable is unset, it will\n\
\t\tbehave as if the value was \"default\". The following are the\n\
\t\tvalid values for $mnemonics:\n\
\t\t\"default\" - Disassemble using the mnemonics which most\n\
\t\t            closely match the specified instruction set.\n\
\t\t\"pwr\"     - Disassemble using the mnemonics for the Power\n\
\t\t            architecture.\n\
\t\t\"ppc\"     - Disassemble using the mnemonics for the PowerPC\n\
\t\t            architecture.\n\
\n\
\t$repeat\n\
\t\tWhen set, dbx repeats the previous command if no command\n\
\t\twas entered.\n\
\n\
\t$sigblock\n\
\t\tWhen set, dbx blocks signals to your program.\n\
\n\
\t$showbases\n\
\t\tWhen set, dbx displays base classes of C++ class types.\n\
\n\
\t$stepignore\n\
\t\tSpecifies the behavior of the \"step\" subcommand on a source\n\
\t\tline calling another routine with no debugging information.\n\
\t\tTo change the value, the user should type 'set $stepignore\n\
\t\t= \"<value>\"'. If the $stepignore variable is unset, it will\n\
\t\tbehave as if the value was \"function\". The following are the\n\
\t\tvalid values for $stepignore:\n\
\t\t\"function\" - dbx will behave as if subcommand \"next\"\n\
\t\t             was issued.\n\
\t\t\"module\"   - if function is in load module with no debug\n\
\t\t             information (such as a system library), dbx\n\
\t\t             will behave as if subcommand \"next\" was issued.\n\
\t\t\"none\"     - dbx will \"stepi\" until it reaches an\n\
\t\t             instruction with source information.\n\
\n\
\t$unsafecall $unsafeassign $unsafegoto $unsafebounds\n\
\t\tTurn off type checking for calls, assignments,\n\
\t\tgoto and array bounds checking.\n\
\n\
\t$vardim\n\
\t\tSpecifies the dimension length to use when printing arrays\n\
\t\twith unknown bounds\n"

MSG_654              "%s is not a known help topic or command\n"

MSG_655              "alias\n\
alias <name>\n\
alias <name> <name>\n\
alias <name> \"<string>\"\n\
alias <name> ( <parameters> ) \"<string>\"\n\
\tWhen commands are processed, dbx first checks to see if\n\
\tthe word is an alias for either a command or a string.\n\
\tIf it is an alias, then dbx treats the input as though\n\
\tthe corresponding string (with values substituted for \n\
\tany parameters) had been entered.\n\
\tAlias with no arguments prints the alias definition list.\n"

MSG_656              "assign <variable> = <expression>\n\
\tAssign the value of the expression to the variable.\n"

MSG_657              "call <procedure> ( <parameters> )\n\
\tExecute the object code associated with the named\n\
\tprocedure or function\n\
\tSee also: print\n"

MSG_658              "case\n\
case mixed\n\
case lower\n\
case upper\n\
case default\n\
\tSet character case sensitivity.\n\
\tCase with no arguments prints the current case sensitivity.\n"

MSG_659              "catch\n\
catch <signal-number>\n\
catch <signal-name>\n\
\tCatch with no arguments prints all signals currently\n\
\tbeing caught. If a signal is specified, dbx\n\
\tstart trapping the signal before it is sent to\n\
\tthe program. This is useful when a program being\n\
\tdebugged has signal handlers. A signal\n\
\tmay be specified by number or name.\n\
\tSignal names are by default case insensitive and the \"SIG\"\n\
\tprefix is optional. By default all signals are caught\n\
\texcept SIGHUP, SIGCHLD, SIGALRM and SIGKILL.\n"

MSG_660              "ignore\n\
ignore <signal-number>\n\
ignore <signal-name>\n\
\tIgnore with no arguments prints all signals currently\n\
\tbeing ignored. If a signal is specified, dbx\n\
\tstops trapping the signal before it is sent to\n\
\tthe program. A signal may be specified by number or name.\n\
\tSignal names are by default case insensitive and the \"SIG\"\n\
\tprefix is optional. By default all signals are trapped\n\
\texcept SIGHUP, SIGCHLD, SIGALRM and SIGKILL.\n"

MSG_661              "cont\n\
cont <signal-number>\n\
cont <signal-name>\n\
\tContinue execution from where it stopped. If a signal\n\
\tis specified, the process continues as though it\n\
\treceived the signal. Otherwise, the process is continued\n\
\tas though it had not been stopped.\n"

MSG_662              "delete <status-number>\n\
delete all\n\
\tThe traces or stops corresponding to the given numbers\n\
\tare removed. The numbers associated with traces and stops\n\
\tcan be printed with the \"status\" command.\n"

MSG_663              "up\n\
up <count>\n\
down\n\
down <count>\n\
\tMove the current function, which is used for resolving\n\
\tnames, up or down the stack <count> levels. The default\n\
\t<count> is one.\n"

MSG_664              "dump               [ > <filename> ]\n\
dump .             [ > <filename> ]\n\
dump <module name> [ > <filename> ]\n\
dump <procedure>   [ > <filename> ]\n\
\tPrint the names and values of variables in the given\n\
\tprocedure, or the current one if none is specified. If\n\
\tthe procedure given is \'.\', then all active variables\n\
\tare dumped. If a module name is given, all variables\n\
\tin the module are dumped.\n"

MSG_665              "edit\n\
edit <filename>\n\
edit <procedure>\n\
\tInvoke an editor on <filename> or the current source file\n\
\tif none is specified. If a procedure or function name\n\
\tis specified the editor is invoked on the file that\n\
\tcontains it. The default editor is the vi editor.\n\
\tThe default can be overridden by setting the environment\n\
\tvariable EDITOR to the name of the desired editor.\n"

MSG_666              "file\n\
file <filename>\n\
\tChange the current source file name to <filename>. If\n\
\tnone is specified then the current source file name is\n\
\tprinted.\n"

MSG_667              "func\n\
func <procedure>\n\
\tChange the current function. If none is specified then\n\
\tprint the current function. Changing the current function\n\
\timplicitly changes the current source file to the\n\
\tone that contains the function; it also changes the\n\
\tcurrent scope used for name resolution.\n"

MSG_668              "clear <line-number>\n\
\tRemove all breakpoints and traces at a given line number.\n\
\tSee also: delete\n"

MSG_669              "cleari <address>\n\
\tRemove all breakpoints and traces at a given address.\n\
\tSee also: delete\n"

MSG_670              "goto <line-number>\n\
goto \"<filename>\" : <line-number>\n\
gotoi <address>\n\
\tChange the program counter to <address> or an address\n\
\tnear <line-number>. The variable $unsafegoto must be set\n\
\tif a goto out of the current function is desired.\n"

MSG_671              "help           [> <filename>]\n\
help <command> [> <filename>]\n\
help <topic>   [> <filename>]\n\
\tPrint information about command or topic.\n\
\t(The string describing a topic may be abbreviated.)\n\
\tLong messages can be paged using the default \"pg\" alias.\n\
\tFor example: \"pg (help set_variables)\".\n"

MSG_672              "list [ <source-line-number> [, <source-line-number> \
]]\n\
list <procedure>\n\
\tList the lines in the current source file from the\n\
\tfirst line number to the second inclusive. If no lines\n\
\tare specified, the next 10 lines are listed. If the\n\
\tname of a procedure or function is given lines n-k to\n\
\tn+k are listed where n is the first statement in the\n\
\tprocedure or function and k is defined by $listwindow\n"

MSG_673              "listi\n\
listi [ <address> [ ,<address> ]]\n\
listi at <source-line-number>\n\
listi <procedure>\n\
\tList the instructions from the current program counter\n\
\tlocation or given address, line number or procedure.\n\
\tThe number of instructions printed is controlled by\n\
\tthe dbx internal variable $listwindow.\n"

MSG_674              "next\n\
next <count>\n\
\tExecute up to the next source line. If a count is supplied\n\
\texecute the next count source lines. The difference between\n\
\tthis and \"step\" is that if the line contains a call\n\
\tto a procedure or function the \"step\" command will\n\
\tstop at the beginning of that block, while the \"next\"\n\
\tcommand will not.\n"

MSG_675              "nexti\n\
nexti <count>\n\
\tSingle step as in \"next\", but do a single instruction\n\
\trather than source line. If a count is supplied\n\
\texecute the nexti count instructions.\n"

MSG_676              "print <expression> [, <expression>]\n\
print <procedure> (<parameters>)\n\
\tPrints the value of specified expression. Values of general\n\
\tpurpose registers and floating point registers can also be\n\
\tprinted using this command.\n\
\t\"print <procedure> (<parameters>)\" executes the object code\n\
\tassociated with the procedure and prints the return value.\n\
\n\
\tNames are resolved first using the static scope of the\n\
\tcurrent function, then using the dynamic scope if the name\n\
\tis not defined in the static scope. If static and dynamic\n\
\tsearches do not yield a result, an arbitrary symbol is\n\
\tchosen and the message \"[using <qualified-name>]\" is\n\
\tprinted. The name resolution procedure may be overridden by\n\
\tqualifying an identifier with a block name, e.g.,\n\
\t\"module.variable\". For C, source files are treated as\n\
\tmodules named by the file name without \".c\"\n"

MSG_677              "prompt\n\
prompt \"string\"\n\
\tDisplays the dbx prompt, or changes prompt to \"string\".\n"

MSG_679              "quit\n\
\tExit dbx (program terminated).\n"

MSG_680              "registers\n\
\tPrint the contents of all general purpose registers,\n\
\tsystem control registers, floating-point registers,\n\
\tand the current instruction register. To display\n\
\tfloating-point registers, use the \"unset $noflregs\"\n\
\tdbx subcommand.\n\
\t\n\
\tRegisters can be displayed or assigned to individually\n\
\tby using the following predefined register names:\n\
\t$r0 through $r31 for the general purpose registers\n\
\t$fr0 through $fr31 for the floating point registers\n\
\t$sp, $iar, $cr, $link for, respectively, the stack pointer,\n\
\tthe program counter, condition register, and the link\n\
\tregister.\n"

MSG_681             "run [<arguments>] [< <filename>] [> <filename>] \n\
                    [>> <filename>] [>! <filename>] \n\
                    [2> <filename>] [2>> <filename>] \n\
                    [>& <filename>] [>>& <filename>] \n\
rerun [<arguments>] [< <filename>] [> <filename>] \n\
                    [>> <filename>] [>! <filename>] \n\
                    [2> <filename>] [2>> <filename>] \n\
                    [>& <filename>] [>>& <filename>] \n\
\tStart executing the object file, passing arguments as\n\
\tcommand line arguments; < or > can be used to redirect\n\
\tinput or output in a shell-like manner. When \"rerun\"\n\
\tis used without any arguments the previous argument list\n\
\tis passed to the program; otherwise it is identical to run.\n"

MSG_682              "return\n\
return <procedure>\n\
\tContinue until a return to <procedure> is executed, or\n\
\tuntil the current procedure returns if none is specified.\n"

MSG_683              "set <name>\n\
set <name> = <expression>\n\
\tThe set command defines values for dbx variables.\n\
\tThe names of these variables cannot conflict with names\n\
\tin the program being debugged, and are expanded to the\n\
\tcorresponding expression within other commands.\n\
\tUse \"unset\" to remove a set variable definition.\n\
\tSee also \"help set_variables\" for definitions of predefined\n\
\tset variables.\n"

MSG_684              "sh\n\
sh <command-line>\n\
\tPass the command line to the shell for execution.\n\
\tThe SHELL environment variable determines which shell is used.\n\
\tThe default is the sh shell. If no argument is specified,\n\
\tcontrol is transferred to the shell.\n"

MSG_685              "source <filename>\n\
\tRead dbx commands from the given file.\n"

MSG_686              "status [> <filename>]\n\
\tDisplay the currently active trace and stop commands.\n"

MSG_687              "step\n\
step <count>\n\
\tExecute one source line.\n\
\tIf a count is supplied, execute the next count source lines.\n\
\tThe difference between this and \"next\" is that if the line\n\
\tcontains a call to a procedure or function the \"step\"\n\
\tcommand will enter that procedure or function, while the \n\
\t\"next\" command will not.\n\
\tSee also: set_variables $stepignore\n"

MSG_688              "stepi\n\
stepi <count>\n\
\tExecute a single instruction.\n\
\tIf a count is supplied, execute the next count instructions.\n"

MSG_689              "stop if <condition>\n\
stop at <line-number>            [if <condition>]\n\
stop in <procedure>              [if <condition>]\n\
stop <variable>                  [if <condition>]\n\
stop <variable> at <line-number> [if <condition>]\n\
stop <variable> in <procedure>   [if <condition>]\n\
\tStop execution when the given line is reached,\n\
\tprocedure or function entered, variable changed,\n\
\tor condition true.\n"

MSG_690              "stopi if <condition>\n\
stopi <address>                [if <condition>]\n\
stopi at <address>             [if <condition>]\n\
stopi in <procedure>           [if <condition>]\n\
stopi <address> in <procedure> [if <condition>]\n\
\t\"stopi at <address>\" stops execution when the given\n\
\t<address> is reached.\n\
\t\"stopi <address>\" stops execution when the value located\n\
\tat the given <address> changes.\n"

MSG_691              "unalias <name>\n\
\tRemove the alias for <name>.\n"

MSG_692              "unset <name>\n\
\tRemove the definition for <name>.\n"

MSG_693              "use\n\
use <directory-list>\n\
\tSet the list of directories to be searched when looking\n\
\tfor source files. If no argument is specified, the current\n\
\tlist if directories to be searched is displayed.\n"

MSG_694              "whatis <name>\n\
\tPrint the declaration of the given name.\n"

MSG_695              "where [> <filename>]\n\
\tPrint out a stack trace of the currently active procedures\n\
\tand functions.\n"

MSG_696              "whereis <name>\n\
\tPrint the full qualification of all symbols whose name\n\
\tmatches <name>.\n"

MSG_697              "which <name>\n\
\tPrint the full qualification of the given <name>.\n\
\tFull qualification of a symbol is its name plus names\n\
\tof its outer blocks.\n"

MSG_698              "trace [in <procedure>]              [if <condition>]\n\
trace <line-number>                 [if <condition>]\n\
trace <procedure> [in <procedure>]  [if <condition>]\n\
trace <variable>  [in <procedure>]  [if <condition>]\n\
trace <expression> at <line-number> [if <condition>]\n\
\tHave tracing information printed when the program is\n\
\texecuted. A number is associated with the command so\n\
\tthat tracing can be turned off with the delete command.\n"

MSG_699              "tracei [<address>] [if <condition>]\n\
tracei [<variable>] [at <address>] [if <condition>]\n\
\tTurn on tracing using a machine instruction address.\n\
\tSee also: trace\n"

MSG_701              "screen\n\
\tSwitch dbx to another virtual terminal. Program continues\n\
\tto operate in the window in which it originated.\n"

MSG_703              "no help available for %s\n"

MSG_706              "detach\n\
detach <sig_num>\n\
detach <sig_name>\n\
\tContinues execution from where it stopped without debugger\n\
\tcontrol. If a signal is specified, the process continues\n\
\tas though it received a signal. Otherwise, the debugger will\n\
\texit, but the debugged process shall continue.\n"

MSG_707              "move <source_line_number>\n\
\tChange the next line to be displayed by the list command\n\
\tto source_line_number\n"

MSG_708                     "skip [<num>]\n\
\tContinues execution of the program where it stopped,\n\
\tignoring the next breakpoint.\n\
\tIf \"num\" is supplied, ignore the next \"num\" breakpoints.\n"

MSG_802              "/<regular-expression>[/]\n\
?<regular-expression>[?]\n\
\tSearch (respectively) forward or backward in the current\n\
\tsource file for the given <regular-expression>.\n\
\tBoth forms of search wrap around.\n\
\tThe previous regular expression is used if no\n\
\tregular expression is given to the current command.\n"

MSG_803              "<address> , <address> / [<mode>] [> <filename>]\n\
<address> / [<count>] [<mode>]   [> <filename>]\n\
\tDisplay contents of memory starting at the first\n\
\t<address> up to the second <address> or until <count>\n\
\titems are printed. If the address is \".\", the address\n\
\tfollowing the one most recently printed is used. The mode\n\
\tspecifies how memory is to be printed; if it is omitted the\n\
\tprevious mode specified is used. The initial mode is \"X\".\n\
\tThe following modes are supported:\n\
\ti    print the machine instruction\n\
\td    print a short word in decimal\n\
\tD    print a long word in decimal\n\
\to    print a short word in octal\n\
\tO    print a long word in octal\n\
\tx    print a short word in hexadecimal\n\
\tX    print a long word in hexadecimal\n\
\tb    print a byte in octal\n\
\tc    print a byte as a character\n\
\th    print a byte in hexadecimal\n\
\ts    print a string (terminated by a null byte)\n\
\tf    print a single precision real number\n\
\tg    print a double precision real number\n\
\tq    print a quad precision real number\n\
\tlld  print an 8 byte signed decimal number\n\
\tllu  print an 8 byte unsigned decimal number\n\
\tllx  print an 8 byte unsigned hexadecimal number\n\
\tllo  print an 8 byte unsigned octal number\n"

MSG_804              "DBX Startup Options:\n\
\n\
dbx [-a ProcessID] [-c CommandFile] [-d NestingDepth] [-I Directory]\n\
[-k] [-u] [-x] [-F] [-L] [-r] [ObjectFile [CoreFile]]\n\
\n\
\t-a ProcessID        Attach to specified process\n\
\t-c CommandFile      Run dbx subcommands in specified file first\n\
\t-d NestingDepth     Set limit for nesting of program blocks\n\
\t-I Directory        Include Directory in list of directories\n\
\t                    searched for source files\n\
\t-k                  Map memory addresses\n\
\t-u                  Prepend file name symbols with an '@'\n\
\t-x                  Strip postfix '_' from FORTRAN symbols\n\
\t-F                  Read all symbols at start-up time\n\
\t-L                  Keep linkage symbols\n\
\t-r                  Run object file immediately\n"

MSG_805              "Breakpoint/Trace subcommands:\n\
stop         - set breakpoint in program\n\
trace        - set trace in program\n\
status       - print active breakpoints and traces\n\
delete       - remove traces or breakpoints of given numbers\n\
clear        - remove all breakpoints and traces at given line\n\
stopi        - set a breakpoint at a specified address\n\
tracei       - turn on instruction tracing\n\
cleari       - remove all breakpoints and traces at given address\n"

MSG_806              "Data/Variable subcommands:\n\
print        - print the value of the expression\n\
display(/)   - display contents of memory\n\
dump         - display names and values of variables in procedure\n\
assign       - assign a value to a variable\n\
whatis       - print declaration of specified name\n\
which        - print full qualification of specified name\n\
whereis      - print full qualification of all symbols with specified name\n\
set          - define a value to a non-program variable\n\
unset        - delete a non-program variable\n"

MSG_814              "Environment subcommands:\n\
alias        - display and assign aliases for dbx subcommands\n\
unalias      - remove an alias\n\
prompt       - change dbx prompt to specified string\n\
screen       - open virtual terminal for dbx command interaction\n\
case         - change the way in which dbx interprets symbols\n\
help         - display help for specified subcommand or topic\n\
multproc     - enable and disable multiprocess debugging\n\
sh           - pass a command to shell for execution\n\
source       - read dbx commands from a file\n\
set          - change a value to a dbx environment set variable\n\
unset        - delete a dbx environment set variable\n\
use          - set directories to be searched for source file\n\
catch        - trap the signal before it is sent to program\n\
ignore       - stop trapping the signal before it is sent to program\n"

MSG_807              "Debugging application program with threads:\n\
No special option is needed for debugging application program\n\
with threads; however, a full core file is required for\n\
thread related subcommands when debugging a core file.\n\
\n\
DBX provides four commands (\"thread\", \"mutex\", \"condition\",\n\
and \"attribute\") for listing thread related objects and\n\
attributes.\n\
\n\
Users can also reference individual thread related objects\n\
using their DBX names in other normal dbx subcommands. For\n\
example:\n\
\t(dbx) whatis $t3\n\
\t(dbx) print $t3.state\n\
\t(dbx) assign $m2.type = fast\n\
\n\
Other normal dbx subcommands behave as usual, when debugging\n\
application program with threads, but in the context of\n\
the current thread (indicated by \">\" in the thread list).\n\
\n\
The running thread is defaulted to be the dbx current thread,\n\
but users can specify the current thread using the \"thread\n\
current <number>\" command.\n\
\n\
Normal dbx breakpoints are not specific to any one thread.\n\
If any one thread hits a breakpoint, all threads will stop.\n\
However, conditional breakpoints can be used to specify \n\
breakpoints for any one particular thread by checking the\n\
execution state of the threads. For example:\n\
\t(dbx) stop at 42 if $t3.state == run\n\
\n\
Thread subcommands:\n\
attribute    - list existing attributes\n\
condition    - list existing condition variables\n\
mutex        - list existing mutexes\n\
thread       - list existing threads\n");

MSG_808              "map [> <filename>]\n\
\tDisplays characteristics for each loaded portion of\n\
\tthe application. This information includes the name, text\n\
\torigin, text length, data origin, and data length for\n\
\teach loaded module.\n"

MSG_809              "multproc\n\
multproc on\n\
multproc parent\n\
multproc child\n\
multproc off\n\
\tSpecifies the behavior of the dbx debug program when forked\n\
\tand execed processes are created.  The \"on\" flag is used\n\
\tto specify that a new dbx session will be created to debug\n\
\tthe child path of a fork.  The original dbx will continue\n\
\tto debug the parent path.  The \"parent\" and \"child\" flags\n\
\tare used to specify a single path of a fork to follow.  All\n\
\tflags except \"off\" enable dbx to follow an execed process.\n\
\tThe \"off\" flag disables multiprocess debugging.  If no flags\n\
\tare specified, the multproc subcommand returns the current\n\
\tstatus of multiprocess debugging.\n"

MSG_810              "thread                                [> <filename>]\n\
thread [<number> ...]                 [> <filename>]\n\
thread [info] [<number> ...]          [> <filename>]\n\
thread [run | ready | susp | term]    [> <filename>]\n\
thread [hold | unhold] [<number> ...] [> <filename>]\n\
thread [current] [<number>]           [> <filename>]\n\
thread [run_next] <number>            [> <filename>]\n\
\tPrint the current status of all known threads in the process.\n\
\tThreads to be listed can be specified through the <number>\n\
\tparameters, or all threads will be listed. Threads can also be\n\
\tselected by states using the run, ready, susp, term, or current options.\n\
\tThe info option can be used to display full information about a\n\
\tthread, and threads can be held or unheld with the hold or unhold\n\
\toption.\n\
\tThe current thread is defaulted to be the running thread and is\n\
\tused by dbx as the context for normal dbx subcommand, such as\n\
\twhere or registers. The current option can be used to switch\n\
\tthe dbx current thread. If users wish to run any one thread\n\
\tnext, the run_next option can be used.\n"

MSG_811              "mutex                                 [> <filename>]\n\
mutex [<number> ...]                  [> <filename>]\n\
mutex [wait | nowait | lock | unlock] [> <filename>]\n\
\tPrint the current status of all known mutexes in the process.\n\
\tMutexes to be listed can be specified through the <number>\n\
\tparameters, or all mutexes will be listed.\n\
\tUsers can also select only locked or unlocked mutexes, or mutexes\n\
\twith or without waiters by using the lock, unlock, wait, or\n\
\tnowait options.\n"

MSG_812              "condition                 [> <filename>]\n\
condition [<number> ...]  [> <filename>]\n\
condition [wait | nowait] [> <filename>]\n\
\tPrint the current status of all known condition in the process.\n\
\tCondition variables to be listed can be specified through the\n\
\t<number> parameters, or all condition variables will be listed.\n\
\tUsers can also select only condition variables with or\n\
\twithout waiters by using the wait or nowait options\n"

MSG_813              "attribute                [> <filename>]\n\
attribute [<number> ...] [> <filename>]\n\
\tPrint the current status of all known attributes in the process.\n\
\tAttributes to be listed can be specified through the <number>\n\
\tparameters, or all attributes will be listed.\n"

MSG_815              "thread                                [> <filename>]\n\
thread [<number> ...]                 [> <filename>]\n\
thread [info] [<number> ...]          [> <filename>]\n\
thread [run | wait | susp | term]     [> <filename>]\n\
thread [hold | unhold] [<number> ...] [> <filename>]\n\
thread [current] [<number>]           [> <filename>]\n\
\tPrint the current status of all known threads in the process.\n\
\tThreads to be listed can be specified through the <number>\n\
\tparameters, or all threads will be listed. Threads can also be\n\
\tselected by states using the run, wait, susp, term, or current options.\n\
\tThe info option can be used to display full information about a\n\
\tthread, and threads can be held or unheld with the hold or unhold\n\
\toption.\n\
\tThe current thread defaults to be the running thread and is\n\
\tused by dbx as the context for normal dbx subcommands, such as\n\
\twhere or registers. The current option can be used to switch\n\
\tthe dbx current thread.\n"

MSG_816              "mutex                                 [> <filename>]\n\
mutex [<number> ...]                  [> <filename>]\n\
mutex [ lock | unlock] [> <filename>]\n\
\tPrint the current status of all known mutexes in the process.\n\
\tMutexes to be listed can be specified through the <number>\n\
\tparameters, or all mutexes will be listed.\n\
\tUsers can also select only locked or unlocked mutexes, or mutexes\n\
\tby using the lock, unlock options.\n"

MSG_817              "Debugging application program with threads:\n\
No special option is needed for debugging application program\n\
with threads; however, a full core file is required for\n\
thread related subcommands when debugging a core file.\n\
\n\
DBX provides four commands (\"thread\", \"mutex\", \"condition\",\n\
and \"attribute\") for listing thread related objects and\n\
attributes.\n\
\n\
Users can also reference individual thread related objects\n\
using their DBX names in other normal dbx subcommands. For\n\
example:\n\
\t(dbx) whatis $t3\n\
\t(dbx) print $t3.state\n\
\t(dbx) assign $m2.islock = 1\n\
\n\
Other normal dbx subcommands behave as usual, when debugging\n\
application program with threads, but in the context of\n\
the current thread (indicated by \">\" in the thread list).\n\
The running thread (indicated by \"*\" in the thread list) is the \n\
thread responsible of stopping process.\n\
\n\
The running thread defaults to be the dbx current thread,\n\
but users can specify the current thread using the \"thread\n\
current <number>\" command.\n\
\n\
Normal dbx breakpoints are not specific to any one thread.\n\
If any one thread hits a breakpoint, all threads will stop.\n\
However, conditional breakpoints can be used to specify \n\
breakpoints for any one particular thread by checking the\n\
execution state of the threads.\n\
Two  aliases are given to help the user :\n\
\tblth(f,x)\tstop at f if ($running_thread == x)\n\
\tbfth(f,x)\tstopi at \&f if ($running_thread == x)\n\
\n\
For example:\n\
\t(dbx) stop at 42 if ($running_thread == 3)\n\
\t(dbx) blth (42,3)\n\
\t(dbx) bfth (thread1,3)\n\
\n\
Thread subcommands:\n\
attribute    - list existing attributes\n\
condition    - list existing condition variables\n\
mutex        - list existing mutexes\n\
thread       - list existing threads\n"));

$ help messages for subject sensitive help end


$set MS_command          Redefine set#

MSG_710               "run and rerun may be used only on originating process.\n"
MSG_720               "Nested subarrays are illegal."
MSG_730               "Illegal use of subarrays."

$set MS_cma_thread       Redefine set#	

MSG_740             "Usage: mutex [wait | nowait | lock | unlock | <mutex#>] \n"
$ Usage info for the "mutex" subcommand
MSG_741             "Usage: condition [wait | nowait | <condition#>] \n"
$ Usage info for the "condition" subcommand
MSG_742             "Usage: attribute [<attribute#>] \n"
$ Usage info for the "attribute" subcommand
MSG_743             "Usage: thread [ hold | unhold  | \n"
MSG_744             "                info | current | run_next |\n"
MSG_745             "                 run | ready   | susp     | term ] [thread#]\n"
$ Usage info for the "thread" subcommand
MSG_746		    "\'$t%d\' is not an existing thread."
$ The user has attempted to switch to or run_next a non-existing thread
MSG_747		    "no thread available"
$ The user has attempted to operate on thread when thread library is not loaded

$set MS_pthread       Redefine set#

MSG_760             "Usage: mutex [ lock | unlock | <mutex#>] \n"
$ Usage info for the "mutex" subcommand
MSG_761             "Usage: condition [wait | nowait | <condition#>] \n"
$ Usage info for the "condition" subcommand
MSG_762             "Usage: attribute [<attribute#>] \n"
$ Usage info for the "attribute" subcommand
MSG_763             "Usage: thread [ hold | unhold  |  \n"
MSG_764             "                info | current | wait \n"
MSG_765             "                run | susp | term ] [thread#]\n"
$ Usage info for the "thread" subcommand
MSG_766             "\'$t%d\' is not an existing thread."
$ The user has attempted to switch to or to display  a non-existing thread
MSG_767             "no thread available"
$ The user has attempted to operate on thread when thread library is not loaded
MSG_768             "kernel thread id=%d not found"
$ The thread id is not found in the corefile
MSG_769             "all threads held"
$ All the threads are held, dbx cannnot continue the execution
MSG_770             "1283-246 could not access to procsinfo errno %d"
$ Error on getprocs or malloc error (for structure procsinfo)
MSG_771             "1283-247 could not access to thrdsinfo errno %d"
$ Error on getthrds or malloc error (for structure procsinfo)
MSG_772             "1283-248 thread with signal not found"
$ The function gettid has not found the thread responsible
MSG_773             "1283-249 malloc error"
$ Error on malloc
MSG_774     "Usage: \"thread [run | wait | susp | term]\" takes no id\n"
$ User has attempted to supply a thread identifier to thread options
MSG_775              "Usage: \"mutex { lock | unlock }\" \ takes no id \n"
$ User has attempted to supply a thread identifier to mutex options
MSG_776             "\'$a%d\' is not an existing attribute."
$ The user has attempted to display a non-existing attribute
MSG_777             "\'$m%d\' is not an existing mutex."
$ The user has attempted to display a non-existing mutex
MSG_778             "\'$c%d\' is not an existing condition."
$ The user has attempted to display a non-existing condition

$set MS_library Define initial set#

MSG_851	   "warning"
$ Non recoverable user error
MSG_852    "fatal error"
$ Indicates an internal program error
MSG_853    "internal error"

