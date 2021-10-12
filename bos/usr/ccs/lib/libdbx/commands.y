/* @(#)34    1.26.3.26  src/bos/usr/ccs/lib/libdbx/commands.y, libdbx, bos411, 9434B411a 8/20/94 16:17:32 */
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 26,27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


%{

/*
 * Yacc grammar for debugger commands.
 */

/*              include file for message texts          */
#include "dbx_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "defs.h"
#include "symbols.h"
#include "operators.h"
#include "tree.h"
#include "process.h"
#include "source.h"
#include "scanner.h"
#include "keywords.h"
#include "eval.h"
#include "names.h"
#include "lists.h"
#include "envdefs.h"
#include "cma_thread.h"
#if defined (CMA_THREAD) || defined (K_THREADS)
#include "k_thread.h"
#endif /* CMA_THREAD || K_THREADS */
#include <fcntl.h>  
extern boolean ScrUsed;
extern int ScrPid;
extern    Ttyinfo ttyinfo;
char *rootdbx_ttyname; 
private String curformat = "X";
public int rerunning = 0;
extern boolean norun;
int star_counter = 0;
int num_stars = 0;

public enum folding { OFF, ON, PAREN, POSSIBLE } 
       case_inhibit = OFF;
public enum redirect { CREATE, APPEND, OVERWRITE } 
       stdout_mode = CREATE;
public enum errordirect { STANDARD, SAME_AS_OUT, CREAT, APPND } 
       stderr_mode = STANDARD;

extern Symbol curmodule;

/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
	};
struct subdim *subdim_head = nil;
struct subdim *subdim_tail = nil;
Symbol array_sym = nil;
boolean subarray_seen = false;			/* parsing a subarray */
boolean ptr_to_subarray = false;
boolean no_more_subarray = false;
Address array_addr = 0;
boolean instlevel = false;
boolean isfunc = false;
long bp_skips = 0;
long stepcount = 1;
private Node parm1;

%}

%term
    ALIAS ALL AND ASSIGN AT ATTRIBUTE BAND CALL CASE CATCH CLEAR CLEARI 
    CONDITION CONT CPU DBXDEBUG DELETE DETACH DIV DOWN DUMP EDIT EXP DBXFILE FUNC 
    GOTO GOTOI HELP IF IGNORE IN KLOAD LIST LISTI LLDB MAP MOD MOVE MULTPROC 
    MUTEX NEXT NEXTI NIL NOT OBJECT OR PRINT PROMPT PSYM QUIT REGISTERS RERUN 
    RETURN RUN SCREEN SET SH SIZEOF SKIP SOURCE STATUS STEP STEPI STOP STOPI 
    SWITCH SYMTYPE THREAD TRACE TRACEI UNALIAS 
    UNSET UP USE WATCH WHATIS WHEN WHERE WHEREIS WHICH WINS WINI XCALL XOR 
    WHATCLASS WHATSTRUCT WHATUNION WHATTYPEDEF WHATENUM WHATTYPE LINESYM 
    DOLLAR INT UINT LONGLONG ULONGLONG CHAR REAL QUAD NAME STRING ERRTOOUT
    REDIRECT ERRDIRECT EQUAL
    EQUAL_EQUAL NOT_EQUAL LESS_GREATER LESS_EQUAL GREATER_EQUAL LEFT_SHIFT
    RIGHT_SHIFT UNARYSIGN DOTDOT BRACKETS ARROWSTAR DOTSTAR ARROW SCOPE
    EXPLICITSYM  TYPECAST 

%right  INT DOLLAR LINESYM ERRTOOUT
%binary AT REDIRECT ERRDIRECT
%binary IN
%left   '=' EQUAL EQUAL_EQUAL '!' NOT_EQUAL LESS_GREATER
%left   '<' LESS_EQUAL '>' GREATER_EQUAL
%left   LEFT_SHIFT RIGHT_SHIFT
%left   '+' '-' '~' OR XOR
%left   '*' '/' DIV MOD BAND AND '|' EXP '&'
%left   UNARYSIGN
%left   '\\' ']' DOTDOT BRACKETS
%left   ARROWSTAR DOTSTAR
%right  SIZEOF
%left   NOT '(' '[' '.' '^' ARROW SCOPE '%'

%union {
    Name y_name;
    Symbol y_sym;
    Node y_node;
    Integer y_int;
    LongLong y_longlong;
    Operator y_op;
    long y_long;
    char y_char;
    double y_real;
    quadf y_quad;
    String y_string;
    Boolean y_bool;
    Cmdlist y_cmdlist;
    List y_list;
};

%type <y_op>	    trace stop aliascmd unaliascmd setcmd unsetcmd
%type <y_sym>	    EXPLICITSYM
%type <y_long>	    INT UINT count enable chgcase line_number_symbol
%type <y_long>      threadoption conditionoption mutexoption attributeoption
%type <y_longlong>  LONGLONG ULONGLONG
%type <y_char>	    CHAR
%type <y_real>	    REAL real_exp
%type <y_quad>	    QUAD quad_exp
%type <y_string>    STRING redirectout filename opt_filename mode
%type <y_name>	    ALIAS AND ASSIGN ALL AT BAND CALL CASE 
%type <y_name>	    CATCH CLEAR CLEARI CONT
%type <y_name>      DBXDEBUG DELETE DIV DOWN DUMP DETACH
%type <y_name>	    EDIT EXP DBXFILE FUNC GOTO GOTOI HELP IF IGNORE IN KLOAD LIST
%type <y_name>	    LISTI LLDB
%type <y_name>      MAP MOD MOVE MULTPROC NEXT NEXTI NIL NOT OBJECT OR PRINT
%type <y_name>      PROMPT PSYM QUIT REGISTERS RERUN RETURN RUN SCREEN SET
%type <y_name>      SH SKIP SOURCE STATUS STEP STEPI STOP STOPI SYMTYPE 
%type <y_name>      THREAD ATTRIBUTE CONDITION MUTEX
%type <y_name>	    TRACE TRACEI UNALIAS UNSET UP USE
%type <y_name>	    WATCH WHATIS WHEN WHERE WHEREIS WHICH WINS WINI XOR
%type <y_name>	    WHATCLASS WHATSTRUCT WHATUNION WHATTYPEDEF WHATENUM
%type <y_name>	    WHATTYPE
%type <y_name>      XCALL
%type <y_name>      SWITCH   CPU
%type <y_name>	    name NAME cpp_name keyword opt_star_list TYPECAST
%type <y_node>      opt_qual_symbol symbol free_class_symbol class_symbol 
%type <y_node>	    command rcommand step what where examine signal
%type <y_node>	    event opt_exp_list opt_cond class_name qualifier
%type <y_node>	    exp_list exp_or_subexp exp term boolean_exp constant
%type <y_node>	    address listi_command list_command
%type <y_node>      thread_cmd condition_cmd mutex_cmd attribute_cmd
%type <y_node>	    integer_list alias_command unalias_command line_number
%type <y_node>	    set_command unset_command typecast
%type <y_list>      sourcepath name_list

%%

input:
	input command_nl
      | /* empty */
      ;

command_nl:
	command_line '\n'
      | command_line ';' { 
	    chkalias = true;
	    endshellmode();
	}
      | '\n'
      ;

command_line:
	command { 
	    if ($1 != nil) { 
		topeval($1); 
	    } 
	}
      | rcommand redirectout {
	    if ($1 != nil) {
	        if ($2 != nil) {
		    setout($2);
		    topeval($1);
		    unsetout();
	  	} else {
		    topeval($1);
	  	}
	    }
	}
      ;

redirectout:
	'>' shellmode NAME {
	    $$ = ident($3);
	    stdout_mode = CREATE;
	}
      | RIGHT_SHIFT shellmode NAME {
	    $$ = ident($3);
	    stdout_mode = APPEND;
	}
      | '>' shellmode '!' NAME {
	    $$ = ident($4);
	    stdout_mode = OVERWRITE;
	} 
      | /* empty */ {
	    $$ = nil;
	}
      ;

/*
 * Non-redirectable commands.
 */
command:
	alias_command {
	    $$ = $1;
	}
      | ASSIGN term '=' exp {
	    $$ = cons(O_ASSIGN, $2, $4);
	}
      | ASSIGN term '(' exp_list ')' '=' exp {
	    $$ = cons(O_ASSIGN, cons(O_UNRVAL, cons(O_INDEX, $2, $4)), $7);
	}
      | ASSIGN INT '=' exp {
	    $$ = cons(O_ASSIGN, cons(O_LCON, $2), $4);
	}
      | CASE {
	    $$ = cons(O_CASE, -1);
	}
      | CASE chgcase {
	    $$ = cons(O_CASE, $2);
	}
      | CATCH signal {
	    $$ = cons(O_CATCH, $2);
	}
      | CATCH {
	    $$ = cons(O_CATCH, cons(O_LCON, (long) 0));
	}
      | CLEAR line_number {
	    $$ = cons(O_CLEAR, cons(O_SCON, strdup(cursource), 0), $2);
	}
      | CLEARI address {
	    $$ = cons(O_CLEARI, $2);
	}
      | CLEAR STRING ':' line_number {
	    $$ = cons(O_CLEAR, cons(O_SCON, $2, 0), $4);
	}
      | CONT {
	    $$ = cons(O_CONT, cons(O_LCON, (long) DEFSIG));
	}
      | CONT signal {
	    $$ = cons(O_CONT, $2);
	}
      | DELETE integer_list {
	    $$ = cons(O_DELETE, $2);
	}
      | DELETE ALL {
	    $$ = cons(O_DELALL);
	}
      | DETACH {
	    $$ = cons(O_DETACH, cons(O_LCON, (long) 0));
	}
      | DETACH signal {
	    $$ = cons(O_DETACH, $2);
	}
      | DOWN {
	    $$ = cons(O_DOWN, cons(O_LCON, (long) 1));
	}
      | DOWN INT {
	    $$ = cons(O_DOWN, cons(O_LCON, (long) $2));
	}
      | EDIT shellmode opt_filename {
	    $$ = cons(O_EDIT, $3);
	}
      | DBXFILE shellmode opt_filename {
	    $$ = cons(O_CHFILE, $3);
	}
      | FUNC {
	    $$ = cons(O_FUNC, nil);
	}
      | FUNC opt_qual_symbol {
	    $$ = cons(O_FUNC, $2);
	}
      | GOTO line_number {
	    if (!instlevel)
		$$ = cons(O_GOTO, cons(O_QLINE, cons(O_SCON, strdup(cursource), 0), $2));
	    else
		$$ = cons(O_GOTO, cons(O_UNRVAL, $2));
	}
      | GOTO STRING ':' line_number {
	    $$ = cons(O_GOTO, cons(O_QLINE, cons(O_SCON, $2, 0), $4));
	}
      | GOTOI address {
	    $$ = cons(O_GOTO, $2);
	}
      | IGNORE signal {
	    $$ = cons(O_IGNORE, $2);
	}
      | IGNORE {
	    $$ = cons(O_IGNORE, cons(O_LCON, (long) 0));
	}
      | KLOAD {
	    $$ = cons(O_KLOAD);
	}
      | LLDB {
	    $$ = cons(O_LLDB, nil);
	}
      | LLDB STRING {
	    $$ = cons(O_LLDB,cons(O_SCON, $2, 0));
	}
      | listi_command {
	    $$ = $1;
	}
      | list_command {
	    $$ = $1;
	}
      | MOVE line_number {
	    $$ = cons(O_MOVE, $2, $2);
	}
      | MOVE opt_qual_symbol {
	    $$ = cons(O_MOVE, $2, nil);
	}
      | MULTPROC {
	    $$ = cons(O_MULTPROC, cons(O_LCON, 0));
	}
      | MULTPROC enable {
	    $$ = cons(O_MULTPROC, cons(O_LCON, $2));
	}
      | OBJECT filename {
	    $$ = cons(O_OBJECT, $2);
	}
      | PROMPT STRING {
	    $$ = cons(O_PROMPT, cons(O_SCON, $2, 0));
	}
      | PROMPT {
	    $$ = cons(O_PROMPT, nil);
	}
      | PSYM exp {
	    $$ = cons(O_PSYM, cons(O_UNRVAL, $2));
	}
      | SYMTYPE exp {
	    $$ = cons(O_SYMTYPE, cons(O_UNRVAL, $2));
	}
      | QUIT {
            if(!norun && ScrUsed) {
               kill(ScrPid, 9);
            }
            if(!norun) { 
               /* if parent dbx quits, restore the originating tty  */
               int fd;
               fd = open(rootdbx_ttyname, O_RDWR);
               ioctl(fd, TCSETA, &(ttyinfo.ttyinfo));
               (void) fcntl(fd, F_SETFL, ttyinfo.fcflags);
            }
	    if (not popinput()) {
		quit(0);
	    } else {
		$$ = nil;
	    }
	}
      | RETURN {
	    $$ = cons(O_RETURN, nil);
	}
      | RETURN opt_qual_symbol {
	    $$ = cons(O_RETURN, $2);
	}
      | runcommand {
#ifdef KDBX
	    (*rpt_error) (stderr, "This command cannot be used under kdbx\n");
#else /* KDBX */

	    if (norun) {
	        (*rpt_error)(stderr,catgets(scmc_catd, MS_command, MSG_710,
                    "run and rerun may be used only on originating process.\n"));
	    }
	    else
	        run();
#endif /* KDBX */
	    $$ = nil; 
	}
      | SCREEN {
	    $$ = cons(O_SCREEN);
	}
      | set_command { 
	    $$ = $1; 
	}
      | SH {
	    shellline();
	    $$ = nil;
	}
      | SKIP {
	    bp_skips = 1;
	    $$ = cons(O_CONT, cons(O_LCON, (long) DEFSIG));
	}
      | SKIP INT {
	    bp_skips = $2;
	    $$ = cons(O_CONT, cons(O_LCON, (long) DEFSIG));
	}
      | SOURCE shellmode filename { 
	    $$ = cons(O_SOURCE, $3); 
	}
      | step { 
	    stepcount = 1;
	    $$ = $1; 
	}
      | step INT {
	    stepcount = $2;
	    $$ = $1;
	}
/* We're commenting out all of the stop commands that use "actions" as it
   is not supported later on.
 
      | stop where opt_cond '{' actions {
	    $$ = cons($1, nil, $2, $3, $5);
	}
      | stop what where opt_cond '{' actions { 
	    $$ = cons($1, $2, $3, $4, $6); 
	}
      | stop what opt_cond '{' actions {
	    $$ = cons($1, $2, nil, $3, $5); 
	}
      | stop IF boolean_exp '{' actions {
	    $$ = cons($1, nil, nil, $3, $5); 
	}
*/
      | stop what where opt_cond { 
	    $$ = cons($1, $2, $3, $4, nil); 
	}
      | stop where opt_cond {
	    $$ = cons($1, nil, $2, $3, nil);
	}
      | stop what opt_cond {
	    $$ = cons($1, $2, nil, $3, nil); 
        } 
      | stop IF boolean_exp {
	    $$ = cons($1, nil, nil, $3, nil); 
	}
      | trace what where opt_cond {
	    $$ = cons($1, $2, $3, $4); 
	}
      | trace where opt_cond {
	    $$ = cons($1, nil, $2, $3); 
	}
      | trace what opt_cond {
	    $$ = cons($1, $2, nil, $3); 
        }
      | trace opt_cond {
	    $$ = cons($1, nil, nil, $2);
	}
      | WATCH exp where opt_cond {
	    $$ = cons(O_WATCH, $2, $3, $4);
	}
      | WATCH exp opt_cond {
	    $$ = cons(O_WATCH, $2, nil, $3);
	}
      | unalias_command {
	    $$ = $1;
	}
      | unset_command {
	    $$ = $1;
	}
      | UP {
	    $$ = cons(O_UP, cons(O_LCON, (long) 1));
	}
      | UP INT {
	    $$ = cons(O_UP, cons(O_LCON, (long) $2));
	}
      | CPU {
	    $$ = cons(O_CPU, cons(O_LCON, (long) -1));
	}
      | CPU INT {
	    $$ = cons(O_CPU, cons(O_LCON, (long) $2));
	}
      | USE shellmode sourcepath { 
	    String dir;
	    $$ = nil;
	    if (list_size($3) == 0) {
		foreach (String, dir, sourcepath)
		    (*rpt_output)( stdout, "%s ", dir);
		endfor
		(*rpt_output)( stdout, "\n");
	    } else {
		if (sourcepath)
		   foreach (String, dir, sourcepath)
		       list_delete(list_curitem(sourcepath), sourcepath);
		   endfor
		sourcepath = $3;
		action_mask |= LISTING; 
		action_mask |= CONFIGURATION;
	        update_source();
	    }
	}
      | WHATCLASS opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WCLASS);
	}
      | WHATSTRUCT opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WSTRUCT);
	}
      | WHATUNION opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WUNION);
	}
      | WHATTYPEDEF opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WTYPEDEF);
	}
      | WHATENUM opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WENUM);
	}
      | WHATTYPE opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WTYPE);
	}
      | WHATIS opt_qual_symbol {
	    $$ = cons(O_WHATIS, $2, WANY);
	}
/* not documented ---
      | WHEN event '{' actions {
	    $$ = cons(O_ADDEVENT, $2, $4);
	}
*/
      | WHEREIS name {
	    $$ = cons(O_WHEREIS, cons(O_NAME, $2));
	}
      | WHICH symbol {
	    $$ = cons(O_WHICH, $2);
	}
      | WINS INT {
	    $$ = cons(O_WINS, cons(O_LCON, (long) $2));
	}
      | WINI INT {
	    $$ = cons(O_WINI, cons(O_LCON, (long) $2));
	}
      | XCALL name '(' exp_list ')' {
	    $$ = cons(O_XCALL, cons(O_NAME, $2), $4);
	}
      | SWITCH {
	    $$ = cons(O_SWITCH, cons(O_LCON, (long) -1));
	}
      | SWITCH INT {
	    $$ = cons(O_SWITCH, cons(O_LCON, (long) $2));
	}
      | '/' {
	    $$ = cons(O_SEARCH,
	               cons(O_LCON, (long) '/'),
	               cons(O_SCON, strdup(scanner_linebuf), 0));
	    gobble();
	    insertinput("\n");
	}
      | '?' {
	    $$ = cons(O_SEARCH, cons(O_LCON, (long) '?'),
	               cons(O_SCON, strdup(scanner_linebuf), 0));
	    gobble();
	    insertinput("\n");
	}
      ;

signal:
	INT {
	    $$ = cons(O_LCON, (long) $1);
	}
      | name {
	    $$ = cons(O_LCON, (long) siglookup(ident($1)));
	}
      ;

chgcase:
	name {
	    $$ = getcase(ident($1));
	}
      ;

enable:
      name {
	    $$ = chkenable(ident($1));
	}
      ;

runcommand:
	run arglist
      | run
      | rerun { rerunning = 0;
	}
      ;

run:
	RUN resetout shellmode {
	    arginit();
	    fflush(stdout);
	}
      ;

rerun:
	RERUN resetout shellmode rerunmode
	{
	    arginit();
	    fflush(stdout);
	}
      | RERUN resetout shellmode rerunmode arglist { fflush(stdout);
	}
      ;

rerunmode:
	/* empty */
	{
	    /*
	     * Clear the redirection file names
	     */
	    clear_filenames();
	    rerunning = 1;
	}
      ;

arglist:
	arglist arg
      | arg
      ;

arg:
	NAME {
	    if (rerunning) {
		rerunning = 0;
		arginit();
	    }
	    newarg(ident($1), false);
	}
      | CHAR {
	    char str[2];
	    if (rerunning) {
		rerunning = 0;
		arginit();
	    }
	    str[0] = $1;
	    str[1] = 0;
	    newarg(str, true);
	}
      | STRING {
	    if (rerunning) {
		rerunning = 0;
		arginit();
	    }
	    newarg($1, true);
	}
      | '<' NAME {
	    inarg(ident($2));
	}
      | '>' NAME {
	    outarg(ident($2));
	    stdout_mode = CREATE;
	}
      | '>' '&' NAME {
	    outarg(ident($3));
	    errarg(ident($3));
	    stdout_mode = CREATE;
	    stderr_mode = SAME_AS_OUT;
	}
      | RIGHT_SHIFT '&' NAME {
	    outarg(ident($3));
	    errarg(ident($3));
	    stdout_mode = APPEND;
	    stderr_mode = SAME_AS_OUT;
	}
      | RIGHT_SHIFT NAME {
	    outarg(ident($2));
	    stdout_mode = APPEND;
	}
      | ERRDIRECT NAME {
	    errarg(ident($2));
	    stderr_mode = CREAT;
	}
      | ERRDIRECT '>' NAME {
	    errarg(ident($3));
	    stderr_mode = APPND;
	}
      | ERRDIRECT ERRTOOUT {
	    stderr_mode = SAME_AS_OUT;
	}
      ;

step:
	STEP {
	    $$ = cons(O_STEP, true, false);
	}
      | STEPI {
	    $$ = cons(O_STEP, false, false);
	}
      | NEXT {
	    $$ = cons(O_STEP, true, true);
	}
      | NEXTI {
	    $$ = cons(O_STEP, false, true);
	}
      ;

shellmode:
	/* empty */ { beginshellmode();
	}
      ;

resetout:
	/* empty */ {
	    stdout_mode= CREATE;
	    stderr_mode= STANDARD;
	}
      ;

caseon:
	/* empty */ { case_inhibit = OFF;
	}
      ;

caseoff:
	/* empty */ { case_inhibit = ON;
	}
      ;

sourcepath:
	sourcepath NAME {
	    $$ = $1;
	    list_append(list_item(ident($2)), nil, $$);
	}
      | /* empty */ {
	    $$ = list_alloc();
	}
      ;

event:
	where
      | exp
      ;

/* not used any more ---
actions:
	actions cmd '}' {
	    $$ = $1;
	    cmdlist_append($2, $$);
	}
      | cmd ';' {
	    $$ = list_alloc();
	    cmdlist_append($1, $$);
	}
      | cmd '}' {
	    $$ = list_alloc();
	    cmdlist_append($1, $$);
	}
      ;

cmd:
	command
      | rcommand
      ;
*/

/*
 * Redirectable commands.
 */
rcommand:
	PRINT exp_list {
	    $$ = cons(O_PRINT, $2);
	}
      | attribute_cmd {
            $$ = cons(O_ATTRIBUTE, cons(O_LCON, (long) $1), nil);
        }
      | attribute_cmd integer_list {
            $$ = cons(O_ATTRIBUTE, cons(O_LCON, (long) $1), $2);
        }
      | condition_cmd {
            $$ = cons(O_CONDITION, cons(O_LCON, (long) $1), nil);
        }
      | condition_cmd integer_list {
            $$ = cons(O_CONDITION, cons(O_LCON, (long) $1), $2);
        }
      | mutex_cmd {
            $$ = cons(O_MUTEX, cons(O_LCON, (long) $1), nil);
        }
      | mutex_cmd integer_list {
            $$ = cons(O_MUTEX, cons(O_LCON, (long) $1), $2);
        }
      | thread_cmd {
            $$ = cons(O_THREAD, cons(O_LCON, (long) $1), nil);
        }
      | thread_cmd integer_list {
            $$ = cons(O_THREAD, cons(O_LCON, (long) $1), $2);
        }
      | WHERE {
	    $$ = cons(O_WHERE);
	}
      | examine {
	    $$ = $1;
	}
      | CALL term '(' opt_exp_list ')' {
	    $$ = cons(O_CALLPROC, $2, $4);
	}
      | DBXDEBUG INT {
	    $$ = cons(O_DEBUG, $2);
	}
      | DBXDEBUG '-' INT {
	    $$ = cons(O_DEBUG, -$3);
	}
      | DUMP opt_qual_symbol {
	    $$ = cons(O_DUMP, $2);
	}
      | DUMP '.' {
	    $$ = cons(O_DUMP, nil);
	}
      | DUMP {
	    $$ = cons(O_DUMP, cons(O_SYM, curfunc));
	}
      | HELP name {
            $$ = cons(O_HELP, build(O_NAME,$2));
        }
      | HELP {
            $$ = cons(O_HELP, nil);
        }
      | MAP {
	    $$ = cons(O_MAP);
	}
      | REGISTERS {
	    $$ = cons(O_REGS, nil);
	}
      | STATUS {
	    $$ = cons(O_STATUS);
	}
      ;

alias_command:
	aliascmd name name { 
	    $$ = cons(O_ALIAS, cons(O_NAME, $2), cons(O_NAME, $3));
	}
      | aliascmd name STRING {
	    $$ = cons(O_ALIAS, cons(O_NAME, $2), cons(O_SCON, $3, 0));
	}
      | aliascmd name '(' name_list ')' STRING {
	    $$ = cons(O_ALIAS, cons(O_COMMA, cons(O_NAME, $2), 
                                    cons(O_NAMELIST, $4)),
	              cons(O_SCON, $6, 0));
	}
      | aliascmd name {
	    $$ = cons(O_ALIAS, cons(O_NAME, $2), nil);
	}
      | aliascmd {
	    $$ = cons(O_ALIAS, nil, nil);
	}
      ;

aliascmd:
	ALIAS {
	    case_inhibit = ON;
	    $$ = O_ALIAS;
	}
      ;

unalias_command:
	unaliascmd name {
	    $$ = cons(O_UNALIAS, cons(O_NAME, $2));
	}
      ;

unaliascmd:
	UNALIAS {
	    case_inhibit = ON;
	    $$ = O_UNALIAS;
	}
      ;

set_command:
	setcmd name '=' caseon exp {
	    $$ = cons(O_SET, cons(O_NAME, $2), $5);
	}
      | setcmd name caseon {
	    $$ = cons(O_SET, cons(O_NAME, $2), nil);
	}
      | setcmd caseon {
	    $$ = cons(O_SET, nil, nil);
	}
      ;

setcmd:
	SET {
	    case_inhibit = ON;
	    $$ = O_SET;
	}
      ;

unset_command:
	unsetcmd name caseon {
	    $$ = cons(O_UNSET, cons(O_NAME, $2));
	}
      ;

unsetcmd:
	UNSET {
	    case_inhibit = ON;
	    $$ = O_UNSET;
	}
      ;

thread_cmd:
        THREAD {
            $$ = nil;
        }
      | THREAD threadoption {
            $$ = $2;
        }
      ;

attribute_cmd:
        ATTRIBUTE {
            $$ = nil;
        }
      | ATTRIBUTE attributeoption {
            $$ = $2;
        }
      ;

condition_cmd:
        CONDITION {
            $$ = nil;
        }
      | CONDITION conditionoption {
            $$ = $2;
        }
      ;

mutex_cmd:
        MUTEX {
            $$ = nil;
        }
      | MUTEX mutexoption {
            $$ = $2;
        }
      ;

threadoption:
        name {
#if defined (CMA_THREAD) || defined (K_THREADS)
            if (lib_type != KERNEL_THREAD)
               $$ = getThreadOpt(ident($1));
            else
               $$ = getThreadOpt_k(ident($1));
#endif /* CMA_THREAD || K_THREADS */
        }
      ;

attributeoption:
        name {
#if defined (CMA_THREAD) || defined (K_THREADS)
            if (lib_type != KERNEL_THREAD)
               $$ = getAttributeOpt(ident($1));
            else
               $$ = getAttributeOpt_k(ident($1));
#endif /* CMA_THREAD || K_THREADS */
        }
      ;

conditionoption:
        name {
#if defined (CMA_THREAD) || defined (K_THREADS)
            if (lib_type != KERNEL_THREAD)
               $$ = getConditionOpt(ident($1));
            else
               $$ = getConditionOpt_k(ident($1));
#endif /* CMA_THREAD || K_THREADS */
        }
      ;

mutexoption:
        name {
#if defined (CMA_THREAD) || defined (K_THREADS)
            if (lib_type != KERNEL_THREAD)
               $$ = getMutexOpt(ident($1));
            else
               $$ = getMutexOpt_k(ident($1));
#endif /* CMA_THREAD || K_THREADS */
        }
      ;

name_list:
	name_list ',' name {
	    $$ = $1;
	    list_append(list_item($3), nil, $$);
	}
      | name {
	    $$ = list_alloc();
	    list_append(list_item($1), nil, $$);
	}
      ;

trace:
	TRACE {
	    instlevel = false;
	    $$ = O_TRACE;
	}
      | TRACEI {
	    instlevel = true;
	    $$ = O_TRACEI;
	}
      ;

stop:
	STOP {
	    instlevel = false;
	    $$ = O_STOP;
	}
      | STOPI {
	    instlevel = true;
	    $$ = O_STOPI;
	}
      ;

what:
	exp {
	    $$ = $1;
	}
      | STRING ':' line_number { 
            char *filename_ptr;
            Boolean is_ambig;

            filename_ptr = resolveFilename ($1, ($3)->value.lcon, NULL,
                                            NULL, &is_ambig, NULL);

            if (filename_ptr && is_ambig)
	      $$ = cons(O_QLINE, cons(O_SCON, filename_ptr, 0), $3);
            else {
	      char	*basename = basefile($1);

	      if( basename != $1 ) {
		$$ = cons(O_QLINE, cons(O_SCON, strdup(basename), 0), $3);
	      } else {
		$$ = cons(O_QLINE, cons(O_SCON, basename, 0), $3);
	      }
	    }
	}
      ;

where:
	IN exp {
	    $$ = cons(O_UNRVAL, $2);
	}
      | AT address {
	    if (!instlevel)
            {
              char *filename_ptr;
              Boolean is_ambig;

              filename_ptr = resolveFilename (cursource,
                                              ($2)->value.lcon, NULL,
                                              NULL, &is_ambig, NULL);

              if (filename_ptr && is_ambig)
		$$ = cons(O_QLINE, cons(O_SCON, filename_ptr, 0), $2);
              else
		$$ = cons(O_QLINE, cons(O_SCON,
                              strdup(basefile(cursource)), 0), $2);
            }
	    else
		$$ = cons(O_UNRVAL, $2);
	}
      | AT STRING ':' line_number {
            char *filename_ptr;
            Boolean is_ambig;

            filename_ptr = resolveFilename ($2, ($4)->value.lcon,
                                            NULL,
                                            NULL, &is_ambig, NULL);

            if (filename_ptr && is_ambig)
	      $$ = cons(O_QLINE, cons(O_SCON, filename_ptr, 0), $4);
            else {
	      char	*basename = basefile($2);

	      if( basename != $2 ) {
		$$ = cons(O_QLINE, cons(O_SCON, strdup(basename), 0), $4);
	      } else {
		$$ = cons(O_QLINE, cons(O_SCON, basename, 0), $4);
	      }
	    }
	}
      ;

opt_star_list:
        /* empty */ {
            $$ = nil;
        }
      | '*' opt_star_list {
            star_counter++;
            $$ = $2;
        }
      ;

filename:
	NAME {
	    $$ = ident($1);
	}
      ;

opt_filename:
	/* empty */ {
	    $$ = nil;
	}
      | filename {
	    $$ = $1;
	}
      ;

opt_exp_list:
	exp_list {
	    $$ = $1;
	}
      | /* empty */ {
	    $$ = nil;
	}
      ;

listi_command:
	LISTI {
	    $$ = cons(O_LISTI, cons(O_LCON, (long) 0), 
		       cons(O_LCON, (long) 0));
	}
      | LISTI address {
	    $$ = cons(O_LISTI, $2, cons(O_LCON, (long) 0));
	}
      | LISTI address ',' address {
	    $$ = cons(O_LISTI, $2, $4);
	}
      | LISTI opt_qual_symbol {
	    $$ = cons(O_LISTI, $2, nil);
	}
      | LISTI AT line_number {
	    $$ = cons(O_LISTI, cons(O_LCON, (long) 0), $3);
	}
      ;

list_command:
	LIST {
	    $$ = cons(O_LIST, cons(O_LCON, (long)0), cons(O_LCON, (long)0));
	}
      | LIST line_number {
	    $$ = cons(O_LIST, $2, cons(O_LCON, (long) 0));
	}
      | LIST line_number ',' line_number {
	    $$ = cons(O_LIST, $2, $4);
	}
      | LIST opt_qual_symbol {
	    $$ = cons(O_LIST, $2, nil);
	}
      ;

integer_list:
	INT { 
	    $$ = cons(O_LCON, $1); 
	}
      | INT integer_list { 
      	    $$ = cons(O_COMMA, cons(O_LCON, $1), $2); 
	}
      | INT ',' integer_list { 
	    $$ = cons(O_COMMA, cons(O_LCON, $1), $3);
	}
      ;

line_number:
	line_number_symbol {
	    $$ = cons(O_LCON, (long) $1);
	}
      | line_number_symbol '+' INT {
	    $$ = cons(O_LCON, (long) ($1 + $3));
	}
      | line_number_symbol '-' INT {
	    int topline;

	    topline = ($1 > $3) ? ($1 - $3) : 1;
	    $$ = cons(O_LCON, topline);
	}
      ;

examine:
	address '/' caseoff count mode caseon {
	    $$ = cons(O_EXAMINE, $5, $1, nil, $4);
	}
      | address ',' caseoff address '/' mode caseon {
	    $$ = cons(O_EXAMINE, $6, $1, $4, 0);
	}
      | address '=' caseoff mode caseon {
	    $$ = cons(O_EXAMINE, $4, $1, nil, 0);
	}
      ;

address:
	INT {
	    $$ = cons(O_LCON, $1);
	}
      | '.' {
	    $$ = cons(O_LCON, (long) prtaddr);
	}
      | '&' exp %prec NOT {
	    $$ = cons(O_ADDR, $2);
	}
      | address '+' address {
	    $$ = cons(O_ADD, $1, $3);
	}
      | address '-' address {
	    $$ = cons(O_SUB, $1, $3);
	}
      | address '*' address {
	    $$ = cons(O_MUL, $1, $3);
	}
      | '*' address %prec UNARYSIGN {
	    $$ = cons(O_INDIRA, $2);
	}
      | '-' address %prec UNARYSIGN {
	    $$ = cons(O_NEG, $2);
	}
      | '(' exp ')' {
	    $$ = $2;
	}
      ;

term:
	symbol {
	    $$ = $1;
	}
      | term DOTSTAR term {
	    $$ = cons(O_UNRVAL, cons(O_DOTSTAR, cons(O_RVAL, $1), 
                               cons(O_RVAL, $3)));
	}
      | term ARROWSTAR term {
	    $$ = cons(O_UNRVAL, cons(O_DOTSTAR, cons(O_RVAL, $1), 
                               cons(O_RVAL, $3)));
	}
      | term '%' class_symbol {
            $$ = cons(O_UNRVAL, cons(O_DOT, cons(O_RVAL, $1), $3));
        }
      | term '.' class_symbol {
	    $$ = cons(O_UNRVAL, cons(O_DOT, cons(O_RVAL, $1), $3));
	}
      | term ARROW class_symbol {
	    $$ = cons(O_UNRVAL, cons(O_DOT, cons(O_RVAL, $1), $3));
	}
      | '(' term ')' {
	    $$ = $2;
	}
      | term '\\' opt_qual_symbol {
	    $$ = cons(O_UNRVAL, cons(O_TYPERENAME, cons(O_UNRVAL, $1), $3));
	}
      | '*' term %prec UNARYSIGN {
	    $$ = cons(O_INDIRA, $2);
	}
      | term '^' %prec UNARYSIGN {
	    $$ = cons(O_INDIRA, $1);
	}
      | term '[' exp_list ']' {
	    $$ = cons(O_UNRVAL, cons(O_INDEX, $1, $3));
	}
      ;

count:
	/* empty */ {
	    $$ = 1;
	}
      | INT {
	    $$ = $1;
	}
      ;

mode:
	name {
	    $$ = ident($1);
	    curformat = $$;
	}
      | /* empty */ {
	    $$ = curformat;
	}
      ;

opt_cond:
	/* empty */ {
	    $$ = nil;
	}
      | IF boolean_exp {
	    $$ = $2;
	}
      ;

exp_list:
	exp_or_subexp {
	    $$ = cons(O_COMMA, $1, nil);
	}
      | exp_or_subexp ',' exp_list {
	    $$ = cons(O_COMMA, $1, $3);
	}
      | exp_or_subexp BRACKETS exp_list { /* BRACKETS are "][" */
	    $$ = cons(O_COMMA, $1, $3);
	}
      ;

exp_or_subexp:
	exp {
	    $$ = $1;
	}
      | exp DOTDOT exp { /* DOTDOT is ".." */
	    subarray_seen = true;
	    $$ = cons(O_DOTDOT, $1, $3);
	}
      ;

exp:
	symbol {
	    $$ = cons(O_RVAL, $1);
	}
      | '[' ']' {
	    $$ = cons(O_BSET, nil);
	}
      | '[' exp_list ']' { 
	    $$ = cons(O_BSET, $2);
	}
      | exp '[' exp_list ']' {
	    if (no_more_subarray && subarray_seen)
	    {
	        beginerrmsg();
	        (*rpt_error)(stderr, catgets(scmc_catd, MS_command, MSG_720,
                                    "Nested subarrays are illegal."));
	        enderrmsg();
	    }
	    if (subarray_seen)
	        no_more_subarray = true;
	    $$ = cons(O_INDEX, cons(O_UNRVAL, $1), $3);
	}
      | SIZEOF exp {
	    $$ = cons(O_SIZEOF, cons(O_UNRVAL, $2));
	}
      | typecast '(' exp ')' %prec SIZEOF {
            $$ = cons(O_TYPERENAME, $3, $1);
        }
      | '(' typecast ')' exp %prec SIZEOF {
            $$ = cons(O_TYPERENAME, $4, $2);
        }
      | '(' typecast '&' ')' exp %prec SIZEOF {

            $$ = cons(O_REFRENAME, $5, $2);
        }
      | '(' typecast '*' opt_star_list ')' exp %prec SIZEOF {
          num_stars = star_counter;
          star_counter = 0;

          $$ = cons(O_PTRRENAME, $6, $2);

          while (num_stars > 0)
          {
            $$ = cons (O_PTRRENAME, $$, copynode($$));
            num_stars--;
          }
        }
      | SIZEOF '(' typecast ')' %prec SIZEOF {
	    $$ = cons(O_SIZEOF, cons(O_UNRVAL, $3));
	}
      | exp '%' class_symbol {
            $$ = cons(O_DOT, $1, $3);
        }
      | exp '.' class_symbol {
	    $$ = cons(O_DOT, $1, $3);
	}
      | exp ARROW class_symbol {
	    $$ = cons(O_DOT, $1, $3);
	}
      | exp ARROWSTAR exp {
            $$ = cons(O_DOTSTAR, $1, $3);
	}
      | exp DOTSTAR exp {
            $$ = cons(O_DOTSTAR, $1, $3);
	}
      | '*' exp %prec UNARYSIGN {
	    if (subarray_seen)
	        ptr_to_subarray = true;
	    $$ = cons(O_INDIR, $2);
	}
      | exp '^' %prec UNARYSIGN {
	    if (subarray_seen)
		ptr_to_subarray = true;
	    $$ = cons(O_INDIR, $1);
	}
      | exp '\\' opt_qual_symbol {
	    $$ = cons(O_TYPERENAME, $1, $3);
	}
      | exp '\\' '&' opt_qual_symbol %prec '\\' {
	    $$ = cons(O_PTRRENAME, $1, $4);
	}
      | exp '(' opt_exp_list ')' %prec '(' {
            $$ = cons(O_INDEX_OR_CALL, cons(O_UNRVAL, $1), $3);
        }
      | constant {
	    $$ = $1;
	}
      | '+' exp %prec UNARYSIGN {
	    $$ = $2;
	}
      | '-' exp %prec UNARYSIGN {
	    $$ = cons(O_NEG, $2);
	}
      | '&' exp %prec UNARYSIGN {
	    if (subarray_seen)
	    {
	       beginerrmsg();
	       (*rpt_error)(stderr, catgets(scmc_catd, MS_command, MSG_730,
                                   "Illegal use of subarrays."));
	       enderrmsg();
	    }
	    $$ = cons(O_ADDR, $2);
	}
      | '~' exp {
	    $$ = cons(O_COMP, $2);
	}
      | exp '+' exp {
	    $$ = cons(O_PADD, $1, $3); /* may be pointer arithmetic */
	}
      | exp '-' exp {
	    $$ = cons(O_PSUB, $1, $3); /* may be pointer arithmetic */
	}
      | exp '*' exp {
	    $$ = cons(O_MUL, $1, $3);
	}
      | exp '/' exp {
	    $$ = cons(O_DIVF, $1, $3);
	}
      | exp DIV exp {
	    $$ = cons(O_DIV, $1, $3);
	}
      | exp MOD exp {
	    $$ = cons(O_MOD, $1, $3);
	}
      | exp AND exp {
	    $$ = cons(O_AND, $1, $3);
	}
      | exp OR exp {
	    $$ = cons(O_OR, $1, $3);
	}
      | exp BAND exp {
	    $$ = cons(O_BAND, $1, $3);
	}
      | exp '&' exp {
	    $$ = cons(O_BAND, $1, $3);
	}
      | exp EXP exp {
	    $$ = cons(O_EXP, $1, $3);
	}
      | exp '|' exp {
	    $$ = cons(O_BOR, $1, $3);
	}
      | exp XOR exp {
	    $$ = cons(O_BXOR, $1, $3);
	}
      | exp '<' exp {
	    $$ = cons(O_LT, $1, $3);
	}
      | exp LESS_EQUAL exp {
	    $$ = cons(O_LE, $1, $3);
	}
      | exp '>' exp {
	    $$ = cons(O_GT, $1, $3);
	}
      | exp GREATER_EQUAL exp {
	    $$ = cons(O_GE, $1, $3);
	}
      | exp '=' exp {
	    $$ = cons(O_EQ, $1, $3);
	}
      | exp EQUAL_EQUAL exp {
	    $$ = cons(O_EQ, $1, $3);
	}
      | exp LEFT_SHIFT exp {
	    $$ = cons(O_SL, $1, $3);
	}
      | exp RIGHT_SHIFT exp {
	    $$ = cons(O_SR, $1, $3);
	}
      | exp LESS_GREATER exp {
	    $$ = cons(O_NE, $1, $3);
	}
      | exp NOT_EQUAL exp {
	    $$ = cons(O_NE, $1, $3);
	}
      | NOT exp {
	    $$ = cons(O_NOT, $2);
	}
      | '!' exp %prec NOT {
	    $$ = cons(O_NOT, $2);
	}
      | '(' exp ')' {
	    $$ = $2;
	}
      | '(' real_exp ',' real_exp ')' {
	    $$ = cons(O_KCON, $2, $4); 
	}
      | '(' quad_exp ',' quad_exp ')' {
	    $$ = cons(O_QKCON, $2, $4); 
	}
      ;

boolean_exp:
	exp {
	    $$ = $1;
	}
      ;

quad_exp:
	QUAD {
	    $$ = $1;
	}
      | '-' quad_exp {
            quadf temp_quad = $2;
            temp_quad.val[0] = -temp_quad.val[0];
            temp_quad.val[1] = -temp_quad.val[1];
            $$ = temp_quad;
	}
      ;

real_exp:
	REAL {
	    $$ = $1;
	}
      | '-' real_exp {
	    $$ = -1.0 * $2;
	}
      | INT {
	    $$ = 1.0 * $1;
	}
      ;

constant:
	INT {
	    $$ = cons(O_LCON, $1);
	}
      | UINT {
	    $$ = cons(O_ULCON, $1);
	}
      | LONGLONG {
	    $$ = cons(O_LLCON, $1);
	}
      | ULONGLONG {
	    $$ = cons(O_ULLCON, $1);
	}
      | CHAR {
	    $$ = cons(O_CCON, $1);
	}
      | REAL {
	    $$ = cons(O_FCON, $1);
	}
      | QUAD {
	    $$ = cons(O_QCON, $1);
	}
      | STRING {
	    $$ = cons(O_SCON, $1, 0);
	}
      ;

opt_qual_symbol:
	symbol {
	    $$ = $1;
	}
      | typecast {
            $$ = $1;
        }
      | opt_qual_symbol '%' class_symbol {
            $$ = cons(O_DOT, $1, $3);
        }
      | opt_qual_symbol '.' class_symbol {
	    $$ = cons(O_DOT, $1, $3);
	}
      | opt_qual_symbol ARROW class_symbol {
	    $$ = cons(O_DOT, $1, $3);
	}
      ;

symbol:
	free_class_symbol {
	    $$ = $1;
	}
      | '.' class_symbol {
	    $$ = cons(O_DOT, cons(O_SYM, program), $2);
	}
      | SCOPE class_symbol { 
	    $$ = cons(O_SCOPE, nil, $2);
	}
      | EXPLICITSYM {
	    Symbol s = $1;

	    if (rtype(s)->class == CPPREF)
		$$ = cons(O_CPPREF, cons(O_SYM,$1));
	    else
		$$ = cons(O_SYM, $1);
	}
      ;

free_class_symbol:
	name {
	    $$ = cons(O_FREE, cons(O_NAME, $1));
	}
      | qualifier SCOPE class_name {
	    $$ = cons(O_UNRES, $1, $3);
	}
      ;

class_symbol:
	cpp_name {
	    $$ = cons(O_NAME, $1);
	}
      | qualifier SCOPE class_name {
	    $$ = cons(O_UNRES, $1, $3);
	}
      ;

qualifier:
	cpp_name {
	    $$ = cons(O_NAME, $1);
	}
      | qualifier SCOPE cpp_name {
	    $$ = cons(O_SCOPE, $1, cons(O_NAME, $3));
	}
      ;

class_name:
	cpp_name {
	    $$ = cons(O_NAME, $1);
	}
      | '~' cpp_name { 
	    char *name = malloc(strlen(ident($2)) + 2);
            name[0] = '~';
	    (void)strcpy(&name[1], ident($2));
	    $$ = cons(O_NAME, identname(name), true);
	}
      ;

cpp_name :
        name {
            $$ = $1;
        }
      | TYPECAST {
            $$ = $1;
        }
      ;

name:
	NAME {
	    $$ = $1;
	}
      | keyword {
	    $$ = $1;
	}
      ;

typecast:
        TYPECAST {
            $$ = cons(O_FREE, cons(O_NAME, $1));  
        }
      ;

line_number_symbol:
	INT {
	    $$ = $1;
	}
      | DOLLAR {
	    getsrcpos();
	    $$ = curline;
	}
      | LINESYM {
	    $$ = cursrcline;
	}
      ;

keyword:
	ALIAS | ALL | AND | ASSIGN | AT | ATTRIBUTE | BAND | CALL | CASE | 
	CATCH | CLEAR | CLEARI | CONDITION | CONT | CPU |  DBXDEBUG | DELETE | 
	DETACH | DIV | DOWN | DUMP | EDIT | EXP | DBXFILE | FUNC | GOTO | 
	GOTOI | HELP | IGNORE | IN | KLOAD | LIST | LISTI | LLDB | MAP |
        MOD | MOVE | MULTPROC | MUTEX | NEXT | NEXTI | NIL | NOT | OBJECT | OR |
        PRINT | PROMPT | PSYM | QUIT | REGISTERS | RERUN | RETURN | RUN |
        SCREEN | SET | SH | SKIP | SOURCE | STATUS | STEP | STEPI | STOP | 
	STOPI | SWITCH |
	SYMTYPE | THREAD | TRACE | TRACEI | UNALIAS | UNSET | UP | USE |
        WATCH | WHATIS | WHEN | WHERE | WHEREIS | WHICH | WINS | WINI | 
	XCALL | XOR |
        WHATCLASS | WHATSTRUCT | WHATUNION | WHATTYPE | WHATENUM | WHATTYPEDEF
      ;
