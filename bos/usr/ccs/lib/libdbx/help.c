static char sccsid[] = "@(#)66    1.12  src/bos/usr/ccs/lib/libdbx/help.c, libdbx, bos41J, 9508A 1/25/95 16:28:18";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: help
 *		help_command
 *		help_help
 *		help_topic
 *		
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"              /* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * include files
 */
#include "defs.h"
#include "envdefs.h"
#include "names.h"
#include "tree.h"
#include "symbols.h"
#include "keywords.h"
#include "process.h"
#include "runtime.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "commands.h"
#if defined (CMA_THREAD) || defined (K_THREADS)
#include "k_thread.h"
#endif /* CMA_THREAD || K_THREADS */



/*
 * SUBCOMPONENT DESIGN PROLOG:
 *
 * SUBCOMPONENT NAME: help subcommand
 *
 * FILES: help.c
 *
 * EXTERNAL FUNCTIONS: help 
 *
 * EXTERNAL DATA: None
 *
 * DEPENDENCIES: None
 *
 * FUNCTION: Handle the dbx 'help' subcommand - display 
 *	     available help messages for dbx subcommands
 *	     and topics.
 *
 * NOTES: Routine help() is the only entry point to this 
 *	  subsystem. When no argument is passed in, routine
 *	  help_help() is called to display a list of all
 *	  possible subcommands and topics. If an argument
 *	  is specified, help_command() is called if the argument
 *	  is a defined dbx subcommand, else help_topic()
 *	  is invoked to handle dbx topics and invalid arguments.
 *
 * DATA STRUCTURES: None
 *
 * TESTING: Testing should be done for all possible and invalid
 *	    options for the 'help' subcommand and under 
 *	    different environments ($LANG) where different message 
 *	    catalogs will be utilitied.
 */

/* 
 * defines 
 */
#define LAST_ITEM 0
#define SCREEN_WIDE 72			/* wide of regular screen   */
#define COMMAND_WIDE 12			/* max wide of command name */
#define TOPIC_WIDE 15			/* max wide of topic name   */

/* list of all subcommands available for detail help */
static String help_commands[] ={
    "alias", "assign", 
#if defined (CMA_THREAD) || defined (K_THREADS)
    "attribute",
#endif /*CMA_THREAD || K_THREADS*/
    "call", "case", "catch", "clear", "cleari", 
#if defined (CMA_THREAD) || defined (K_THREADS)
    "condition",
#endif /*CMA_THREAD || K_THREADS*/
    "cont",
#ifdef KDBX
    "cpu",
#endif /* KDBX */
    "delete", "detach",
    "display(/)", "down", "dump", "edit", 
    "file", "func", "goto", "gotoi", "help", "ignore", 
#ifdef KDBX
    "kload",
#endif /*KDBX*/
    "list", "listi", 
#ifdef KDBX
    "lldb",
#endif /*KDBX*/
    "map", "move", "multproc",
#if defined (CMA_THREAD) || defined (K_THREADS)
    "mutex",
#endif /*CMA_THREAD || K_THREADS*/
    "next", "nexti", "print", "prompt", "quit",
    "registers", "rerun", "return", "run", "screen", 
    "search(/?)", "set", "sh", 
    "skip", "source", "status", "step", "stepi", "stop", "stopi", 
#ifdef KDBX
    "switch",
#endif /* KDBX */
#if defined (CMA_THREAD) || defined (K_THREADS)
    "thread",
#endif /*CMA_THREAD  || K_THREADS*/
    "trace", "tracei", "unalias", "unset", "up", "use",
    "whatis", "where", "whereis", "which", 
#ifdef KDBX
    "xcall",
#endif /* KDBX */
    LAST_ITEM
};

/* list of all topics available for detail help */
static String help_topics[] = {
   "startup",
   "execution", "breakpoints", 
   "files", "data", "machine", 
   "environment", 
#if defined (CMA_THREAD) || defined (K_THREADS)
    "threads",
#endif /*CMA_THREAD || K_THREADS*/
   "expressions", "scope",
   "set_variables", "usage",
   LAST_ITEM
};


/*
 * NAME: help_help
 *
 * FUNCTION: Display list of available subcommands and topics for
 *	     detail help.
 *
 * PARAMETERS: None.
 *
 * NOTES: Used by help subcommand when users provided no argument.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void help_help()
{
   String *p;
   int len, i;

   (*rpt_output)(stdout,"Commands:\n");
   i = len = 0;
   /* display command list in columns */
   for ( p = help_commands; *p; p++ ){
      i = (*rpt_output)(stdout," %s ",*p);
      while ( i++ < COMMAND_WIDE)	
	(*rpt_output)(stdout," ");
      len += COMMAND_WIDE;
      if ( len >= SCREEN_WIDE ){
	 (*rpt_output)(stdout,"\n");
	 len = 0;
      }
   }
   (*rpt_output)(stdout,"\n\nTopics:\n");
   i = len = 0;
   /* display topic list in columns */
   for ( p = help_topics; *p; p++ ){
      i = (*rpt_output)(stdout," %s ",*p);
      while ( i++ < TOPIC_WIDE)	
	(*rpt_output)(stdout," ");
      len += TOPIC_WIDE;
      if ( len >= SCREEN_WIDE ){
	 (*rpt_output)(stdout,"\n");
	 len = 0;
      }
   }
   (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_646,
                   "\n\nType \"help <command>\" or \"help <topic>\" \
for help on a command or topic.\n"));
}


/*
 * NAME: help_topic
 *
 * FUNCTION: Check if users provided a topic as an argument to the
 *	     help subcommands and display detail help messages for
 *	     the specified topic.
 *
 * PARAMETERS:
 *      s    - string typed in as option to the help subcommand
 *
 * NOTES: Used by help subcommand when users provided a topic as 
 *	  an argument.
 *        The "search" and "display" options are not part of 
 *	  dbx subcommand token keyword and therefore are handled
 *        here.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void help_topic(s)
char *s;
{
   int n;

   n = strlen(s);

   if (!strncmp(s,"search",n)) {
	/* This is actully a command, but with no taken value */
        (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_802,
                       "/<regular-expression>[/]\n\
?<regular-expression>[?]\n\
\tSearch (respectively) forward or backward in the current\n\
\tsource file for the given <regular-expression>.\n\
\tBoth forms of search wrap around.\n\
\tThe previous regular expression is used if no\n\
\tregular expression is given to the current command.\n"));
   }
   else if ( !strncmp(s,"display",n) ){
	/* This is actully a command, but with no taken value */
        (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_803,
		       "<address> , <address> / [<mode>] [> <filename>]\n\
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
\tllo  print an 8 byte unsigned octal number\n"));
   } 
   else if (!strncmp(s,"startup",n)) {
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_804,
                     "DBX Startup Options:\n\
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
\t-r                  Run object file immediately\n"));
   } 
   else if (!strncmp(s,"execution",n)) {
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_647,
                     "Execution subcommands:\n\
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
quit         - exit dbx (program terminated)\n"));
   }
   else if ( !strncmp(s,"breakpoints",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_805,
                     "Breakpoint/Trace subcommands:\n\
stop         - set breakpoint in program\n\
trace        - set trace in program\n\
status       - print active breakpoints and traces\n\
delete       - remove traces or breakpoints of given numbers\n\
clear        - remove all breakpoints and traces at given line\n\
stopi        - set a breakpoint at a specified address\n\
tracei       - turn on instruction tracing\n\
cleari       - remove all breakpoints and traces at given address\n"));
   }
   else if ( !strncmp(s,"data",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_806,
                     "Data/Variable subcommands:\n\
print        - print the value of the expression\n\
display(/)   - display contents of memory\n\
dump         - display names and values of variables in procedure\n\
assign       - assign a value to a variable\n\
whatis       - print declaration of specified name\n\
which        - print full qualification of specified name\n\
whereis      - print full qualification of all symbols with specified name\n\
set          - define a value to a non-program variable\n\
unset        - delete a non-program variable\n"));
   }
   else if ( !strncmp(s,"environment",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_814,
                     "Environment subcommands:\n\
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
ignore       - stop trapping the signal before it is sent to program\n"));
   }
   else if ( !strncmp(s,"scope",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_648,
                     "When displaying variables and expressions, \
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
language program)\n"));
   }
   else if ( !strncmp(s,"expressions",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_649,
                     "Expressions are specified with a subset \
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
Other        sizeof\n"));
   }
   else if ( !strncmp(s,"files",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_650,
                     "File subcommands:\n\
use          - set directories to be searched for source file\n\
list         - list lines of current source file\n\
file         - change or display current source file\n\
func         - change or display current function\n\
search(/?)   - search forward or backward in current file for a pattern\n\
edit         - invoke an editor on specifed file or function\n\
move         - change next line to be displayed by list command\n"));
   }
   else if ( !strncmp(s,"machine",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_651,
                     "Machine level subcommands:\n\
display(/)   - display contents of memory\n\
cleari       - remove all stop/trace points at given address\n\
listi        - list instruction from application\n\
stopi        - set a breakpoint at a specified address\n\
tracei       - turn on instruction tracing\n\
stepi        - single step one instruction\n\
nexti        - step to next instruction (skip over calls)\n\
registers    - display register set\n\
gotoi        - change execution to specified address\n\
map          - display address maps and loader information\n"));
   }
#if defined (CMA_THREAD) || defined (K_THREADS)
   else if ( !strncmp(s,"threads",n) ){
           if (lib_type != KERNEL_THREAD) {
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_807,
                     "Debugging application program with threads:\n\
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
thread       - list existing threads\n"));
      } else { /* KERNEL_THREAD */

      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_817,
                     "Debugging application program with threads:\n\
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
\tbfth(f,x)\tstopi at &f if ($running_thread == x)\n\
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
     }

   }
#endif /*CMA_THREAD || K_THREADS*/
   else if ( !strncmp(s,"usage",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_652,
                     "Basic dbx command usage:\n\n\
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
quit                   - exit dbx\n"));
   }
   else if ( !strncmp(s,"set_variables",n) ){
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_653,
                     "\tThe following \"set\" variables have special \
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
\t\twith unknown bounds\n"));       
#ifdef KDBX
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
                   "\n\
\t$loadstop\n\
\t\tWhen set, kdbx stops when a kernel extension is loaded.\n\
\n\
\t$screen\n\
\t\tWhen set, kdbx turns screen on when entering into low\n\
\t\tlevel debugger.\n\
\n\
\t$mst\n\
\t\tSpecifies MST value for traceback for another process.\n");
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
                   "\n\
\t$progress\n\
\t\tif $repeat and $progress are set, entering an empty line\n\
\t\tafter a command of the form : address/n format will display n\n\
\t\tmore units (as if the ./n format subcommand had been entered).\n\
\n\
\t$showregs\n\
\t\twhen stepping (stepi, nexti) at the machine language level,\n\
\t\tthe values of the registers implied in the instruction\n\
\t\twill be automatically displayed.\n\
\n\
\t$where_thru_exc\n\
\t\tthe where command will perform a stack trace for all \n\
\t\tstacked exceptions, starting from the current mst\n\
\n\
\t$where_thru_sc\n\
\t\tthe where command will go down to the user mode, (using\n\
\t\tthe traceback information to retrieve the function names)\n\
\n\
\t$no_local_bp\n\
\t\twhen stepping (step, next) at the source level, do not use\n\
\t\tlocal breakpoints from the kernel debugger.\n\
\n\
\t$prompt_cpu\n\
\t\tWhen set, output the current cpu id in the kdbx promtp. \n\
\t$no_bininst\n\
\t\tWhen set, the disassembly of an instruction does not output\n\
\t\tthe binary form of that instruction\n\
");
#endif /* KDBX */
   }
   else         (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_654,
                   "%s is not a known help topic or command\n"),s);
}


/*
 * NAME: help_command
 *
 * FUNCTION: Check if users provided a subcommand as an argument to the
 *	     help subcommands and display detail help messages for
 *	     the specified subcommand.
 *
 * PARAMETERS:
 *      op   - keyword token value associated with argument string 
 *      cmd  - string typed in as option to the help subcommand
 *
 * NOTES: Used by help subcommand when users provided a subcommand as 
 *	  an argument.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void help_command(op,cmd)
int op;
char *cmd;
{
   switch ( op ){

    case ALIAS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_655,
                     "alias\n\
alias <name>\n\
alias <name> <name>\n\
alias <name> \"<string>\"\n\
alias <name> ( <parameters> ) \"<string>\"\n\
\tWhen commands are processed, dbx first checks to see if\n\
\tthe word is an alias for either a command or a string.\n\
\tIf it is an alias, then dbx treats the input as though\n\
\tthe corresponding string (with values substituted for \n\
\tany parameters) had been entered.\n\
\tAlias with no arguments prints the alias definition list.\n"));
      break;

    case ASSIGN:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_656,
                      "assign <variable> = <expression>\n\
\tAssign the value of the expression to the variable.\n"));
      break;

    case CALL:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_657,
                     "call <procedure> ( <parameters> )\n\
\tExecute the object code associated with the named\n\
\tprocedure or function\n\
\tSee also: print\n"));
      break;

    case CASE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_658,
                     "case\n\
case mixed\n\
case lower\n\
case upper\n\
case default\n\
\tSet character case sensitivity.\n\
\tCase with no arguments prints the current case sensitivity.\n"));
      break;

    case CATCH:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_659,
                     "catch\n\
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
\texcept SIGHUP, SIGCHLD, SIGALRM and SIGKILL.\n"));
      break;

    case CLEAR:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_668,
                     "clear <line-number>\n\
\tRemove all breakpoints and traces at a given line number.\n\
\tSee also: delete\n"));
      break;

    case CLEARI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_669,
                     "cleari <address>\n\
\tRemove all breakpoints and traces at a given address.\n\
\tSee also: delete\n"));
      break;

    case CONT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_661,
                     "cont\n\
cont <signal-number>\n\
cont <signal-name>\n\
\tContinue execution from where it stopped. If a signal\n\
\tis specified, the process continues as though it\n\
\treceived the signal. Otherwise, the process is continued\n\
\tas though it had not been stopped.\n"));
      break;

    case DELETE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_662,
                     "delete <status-number>\n\
delete all\n\
\tThe traces or stops corresponding to the given numbers\n\
\tare removed. The numbers associated with traces and stops\n\
\tcan be printed with the \"status\" command.\n"));
      break;

    case DETACH:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_706,
                     "detach\n\
detach <sig_num>\n\
detach <sig_name>\n\
\tContinues execution from where it stopped without debugger\n\
\tcontrol. If a signal is specified, the process continues\n\
\tas though it received a signal. Otherwise, the debugger will\n\
\texit, but the debugged process shall continue.\n"));
      break;

    case DOWN:
    case UP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_663,
                     "up\n\
up <count>\n\
down\n\
down <count>\n\
\tMove the current function, which is used for resolving\n\
\tnames, up or down the stack <count> levels. The default\n\
\t<count> is one.\n"));
      break;

    case DUMP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_664,
		     "dump               [ > <filename> ]\n\
dump .             [ > <filename> ]\n\
dump <module name> [ > <filename> ]\n\
dump <procedure>   [ > <filename> ]\n\
\tPrint the names and values of variables in the given\n\
\tprocedure, or the current one if none is specified. If\n\
\tthe procedure given is \'.\', then all active variables\n\
\tare dumped. If a module name is given, all variables\n\
\tin the module are dumped.\n"));
      break;

    case EDIT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_665,
                     "edit\n\
edit <filename>\n\
edit <procedure>\n\
\tInvoke an editor on <filename> or the current source file\n\
\tif none is specified. If a procedure or function name\n\
\tis specified the editor is invoked on the file that\n\
\tcontains it. The default editor is the vi editor.\n\
\tThe default can be overridden by setting the environment\n\
\tvariable EDITOR to the name of the desired editor.\n"));
      break;

    case DBXFILE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_666,
                     "file\n\
file <filename>\n\
\tChange the current source file name to <filename>. If\n\
\tnone is specified then the current source file name is\n\
\tprinted.\n"));
      break;

    case FUNC:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_667,
                     "func\n\
func <procedure>\n\
\tChange the current function. If none is specified then\n\
\tprint the current function. Changing the current function\n\
\timplicitly changes the current source file to the\n\
\tone that contains the function; it also changes the\n\
\tcurrent scope used for name resolution.\n"));
      break;

    case GOTOI:
    case GOTO:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_670,
                     "goto <line-number>\n\
goto \"<filename>\" : <line-number>\n\
gotoi <address>\n\
\tChange the program counter to <address> or an address\n\
\tnear <line-number>. The variable $unsafegoto must be set\n\
\tif a goto out of the current function is desired.\n"));
      break;

    case HELP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_671,
                     "help           [> <filename>]\n\
help <command> [> <filename>]\n\
help <topic>   [> <filename>]\n\
\tPrint information about command or topic.\n\
\t(The string describing a topic may be abbreviated.)\n\
\tLong messages can be paged using the default \"pg\" alias.\n\
\tFor example: \"pg (help set_variables)\".\n"));
      break;

    case IGNORE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_660,
                     "ignore\n\
ignore <signal-number>\n\
ignore <signal-name>\n\
\tIgnore with no arguments prints all signals currently\n\
\tbeing ignored. If a signal is specified, dbx\n\
\tstops trapping the signal before it is sent to\n\
\tthe program. A signal may be specified by number or name.\n\
\tSignal names are by default case insensitive and the \"SIG\"\n\
\tprefix is optional. By default all signals are trapped\n\
\texcept SIGHUP, SIGCHLD, SIGALRM and SIGKILL.\n"));
      break;

#ifdef KDBX
    case KLOAD:
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
		     "kload\n\
\tUpdate information about loaded kernel extensions.\n\
\tSee also: set_variables $loadstop\n");
      break;
#endif /*KDBX*/

    case LIST:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_672,
                     "list [ <source-line-number> [, <source-line-number> \
]]\n\
list <procedure>\n\
\tList the lines in the current source file from the\n\
\tfirst line number to the second inclusive. If no lines\n\
\tare specified, the next 10 lines are listed. If the\n\
\tname of a procedure or function is given lines n-k to\n\
\tn+k are listed where n is the first statement in the\n\
\tprocedure or function and k is defined by $listwindow\n"));
      break;

    case LISTI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_673,
                     "listi\n\
listi [ <address> [ ,<address> ]]\n\
listi at <source-line-number>\n\
listi <procedure>\n\
\tList the instructions from the current program counter\n\
\tlocation or given address, line number or procedure.\n\
\tThe number of instructions printed is controlled by\n\
\tthe dbx internal variable $listwindow.\n"));
      break;

#ifdef KDBX
    case LLDB:
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
		     "lldb\n\
lldb <command-line>\n\
\tPass the command line to the low level debugger for execution.\n");
      break;
#endif /*KDBX*/

    case MAP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_808,
		"map [> <filename>]\n\
\tDisplays characteristics for each loaded portion of\n\
\tthe application. This information includes the name, text\n\
\torigin, text length, data origin, and data length for\n\
\teach loaded module.\n"));
      break;

    case MOVE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_707,
                     "move <source_line_number>\n\
\tChange the next line to be displayed by the list command\n\
\tto source_line_number\n"));
      break;

    case MULTPROC:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_809,
                     "multproc\n\
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
\tstatus of multiprocess debugging.\n"));
      break;

    case NEXT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_674,
                     "next\n\
next <count>\n\
\tExecute up to the next source line. If a count is supplied\n\
\texecute the next count source lines. The difference between\n\
\tthis and \"step\" is that if the line contains a call\n\
\tto a procedure or function the \"step\" command will\n\
\tstop at the beginning of that block, while the \"next\"\n\
\tcommand will not.\n"));
      break;

    case NEXTI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_675,
                     "nexti\n\
nexti <count>\n\
\tSingle step as in \"next\", but do a single instruction\n\
\trather than source line. If a count is supplied\n\
\texecute the nexti count instructions.\n"));
      break;
                      
    case PRINT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_676,
                     "print <expression> [, <expression>]\n\
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
\tmodules named by the file name without \".c\"\n"));
      break;

    case PROMPT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_677,
                     "prompt\n\
prompt \"string\"\n\
\tDisplays the dbx prompt, or changes prompt to \"string\".\n"));
      break;

    case QUIT:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_679,
                     "quit\n\
\tExit dbx (program terminated).\n"));
      break;

    case REGISTERS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_680,
                     "registers\n\
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
\tregister.\n"));
      break;

    case RERUN:
    case RUN:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_681,
                     "run [<arguments>] [< <filename>] [> <filename>] \n\
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
\tis passed to the program; otherwise it is identical to run.\n"));
      break;

    case RETURN:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_682,
                     "return\n\
return <procedure>\n\
\tContinue until a return to <procedure> is executed, or\n\
\tuntil the current procedure returns if none is specified.\n"));
      break;

    case SCREEN:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_701,
                     "screen\n\
\tSwitch dbx to another virtual terminal. Program continues\n\
\tto operate in the window in which it originated.\n"));
      break;

    case SET:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_683,
                     "set <name>\n\
set <name> = <expression>\n\
\tThe set command defines values for dbx variables.\n\
\tThe names of these variables cannot conflict with names\n\
\tin the program being debugged, and are expanded to the\n\
\tcorresponding expression within other commands.\n\
\tUse \"unset\" to remove a set variable definition.\n\
\tSee also \"help set_variables\" for definitions of predefined\n\
\tset variables.\n"));
      break;

    case SH:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_684,
                     "sh\n\
sh <command-line>\n\
\tPass the command line to the shell for execution.\n\
\tThe SHELL environment variable determines which shell is used.\n\
\tThe default is the sh shell. If no argument is specified,\n\
\tcontrol is transferred to the shell.\n"));

      break;

    case SKIP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_708,
                     "skip [<num>]\n\
\tContinues execution of the program where it stopped,\n\
\tignoring the next breakpoint.\n\
\tIf \"num\" is supplied, ignore the next \"num\" breakpoints.\n"));
      break;

    case SOURCE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_685,
                     "source <filename>\n\
\tRead dbx commands from the given file.\n"));
      break;

    case STATUS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_686,
                     "status [> <filename>]\n\
\tDisplay the currently active trace and stop commands.\n"));
      break;

    case STEP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_687,
                     "step\n\
step <count>\n\
\tExecute one source line.\n\
\tIf a count is supplied, execute the next count source lines.\n\
\tThe difference between this and \"next\" is that if the line\n\
\tcontains a call to a procedure or function the \"step\"\n\
\tcommand will enter that procedure or function, while the \n\
\t\"next\" command will not.\n\
\tSee also: set_variables $stepignore\n"));
      break;

    case STEPI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_688,
                     "stepi\n\
stepi <count>\n\
\tExecute a single instruction.\n\
\tIf a count is supplied, execute the next count instructions.\n"));
      break;

    case STOP:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_689,
                     "stop if <condition>\n\
stop at <line-number>            [if <condition>]\n\
stop in <procedure>              [if <condition>]\n\
stop <variable>                  [if <condition>]\n\
stop <variable> at <line-number> [if <condition>]\n\
stop <variable> in <procedure>   [if <condition>]\n\
\tStop execution when the given line is reached,\n\
\tprocedure or function entered, variable changed,\n\
\tor condition true.\n"));
      break;

    case STOPI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_690,
		     "stopi if <condition>\n\
stopi <address>                [if <condition>]\n\
stopi at <address>             [if <condition>]\n\
stopi in <procedure>           [if <condition>]\n\
stopi <address> in <procedure> [if <condition>]\n\
\t\"stopi at <address>\" stops execution when the given\n\
\t<address> is reached.\n\
\t\"stopi <address>\" stops execution when the value located\n\
\tat the given <address> changes.\n"));
      break;

    case TRACE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_698,
                     "trace [in <procedure>]              [if <condition>]\n\
trace <line-number>                 [if <condition>]\n\
trace <procedure> [in <procedure>]  [if <condition>]\n\
trace <variable>  [in <procedure>]  [if <condition>]\n\
trace <expression> at <line-number> [if <condition>]\n\
\tHave tracing information printed when the program is\n\
\texecuted. A number is associated with the command so\n\
\tthat tracing can be turned off with the delete command.\n"));
      break;

    case TRACEI:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_699,
                     "tracei [<address>] [if <condition>]\n\
tracei [<variable>] [at <address>] [if <condition>]\n\
\tTurn on tracing using a machine instruction address.\n\
\tSee also: trace\n"));
      break;

    case UNALIAS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_691,
                     "unalias <name>\n\
\tRemove the alias for <name>.\n"));
      break;

    case UNSET:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_692,
                     "unset <name>\n\
\tRemove the definition for <name>.\n"));
      break;

    case USE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_693,
                     "use\n\
use <directory-list>\n\
\tSet the list of directories to be searched when looking\n\
\tfor source files. If no argument is specified, the current\n\
\tlist if directories to be searched is displayed.\n"));
      break;

    case WHATIS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_694,
                     "whatis <name>\n\
\tPrint the declaration of the given name.\n"));
      break;

    case WHERE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_695,
                     "where [> <filename>]\n\
\tPrint out a stack trace of the currently active procedures\n\
\tand functions.\n"));
      break;

    case WHEREIS:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_696,
                     "whereis <name>\n\
\tPrint the full qualification of all symbols whose name\n\
\tmatches <name>.\n"));
      break;

    case WHICH:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_697,
                     "which <name>\n\
\tPrint the full qualification of the given <name>.\n\
\tFull qualification of a symbol is its name plus names\n\
\tof its outer blocks.\n"));
      break;

#ifdef KDBX
    case CPU:
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
		     "cpu	[ cpu_number ]\n\
\tswitch the kernel debugger on an other processor.\n\
\tcpu without argument displays the status of the processors\n");
      break;

    case SWITCH:
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
		     "switch	[ thread-slot ]\n\
\tswitch to the context of the thread given by its slot number.\n\
\t(The lldb \"th\" command may be used to dump the thread table)\n\
\tswitch without argument returns to the thread running on the current\n\
\tprocessor\n");
      break;

    case XCALL:
      /* kdbx messages are currently not placed in message catalog */
      (*rpt_output)(stdout,
		     "xcall debug_function (arg,...)\n\
\tdynamically loads the debug_function and calls it.\n\
\tdebug_function is the name of an executable file whose entry point is\n\
\tthe function that will actually be called.\n\
\tThe file shall be under the current directory or in a directory whose\n\
\tpath will be stored in an environment variable named KDBX_FUNCS\n\
\tThe LIBPATH environment variable must be set to /lib:/usr/lib:kdbx_dir\n\
\twhere kdbx_dir is the place where kdbx is actually installed.\n\
\tThe arguments are passed to the function (max allowed = 4 arguments)\n\
\tThe function is unloaded after being executed\n");
      break;
#endif /* KDBX */

#if defined (CMA_THREAD) || defined (K_THREADS)
    case THREAD:
      if( lib_type != KERNEL_THREAD) { /* not  the same argument */
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_810,
                     "thread                                [> <filename>]\n\
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
\tnext, the run_next option can be used.\n"));
      break;
      } else {  /* KERNEL_THREAD  */

      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_815,
                     "thread                                [> <filename>]\n\
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
\tthe dbx current thread.\n"));
      break;
      }

    case MUTEX:
      if( lib_type != KERNEL_THREAD) { /* not  the same argument */
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_811,
                     "mutex                                 [> <filename>]\n\
mutex [<number> ...]                  [> <filename>]\n\
mutex [wait | nowait | lock | unlock] [> <filename>]\n\
\tPrint the current status of all known mutexes in the process.\n\
\tMutexes to be listed can be specified through the <number>\n\
\tparameters, or all mutexes will be listed.\n\
\tUsers can also select only locked or unlocked mutexes, or mutexes\n\
\twith or without waiters by using the lock, unlock, wait, or\n\
\tnowait options.\n"));
      break;
      } else {  /* KERNEL_THREAD  */
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_816,
                     "mutex                                 [> <filename>]\n\
mutex [<number> ...]                  [> <filename>]\n\
mutex [ lock | unlock] [> <filename>]\n\
\tPrint the current status of all known mutexes in the process.\n\
\tMutexes to be listed can be specified through the <number>\n\
\tparameters, or all mutexes will be listed.\n\
\tUsers can also select only locked or unlocked mutexes, or mutexes\n\
\tby using the lock, unlock options.\n"));

      break;
     }

    case CONDITION:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_812,
                     "condition                 [> <filename>]\n\
condition [<number> ...]  [> <filename>]\n\
condition [wait | nowait] [> <filename>]\n\
\tPrint the current status of all known condition in the process.\n\
\tCondition variables to be listed can be specified through the\n\
\t<number> parameters, or all condition variables will be listed.\n\
\tUsers can also select only condition variables with or\n\
\twithout waiters by using the wait or nowait options\n"));
      break;

    case ATTRIBUTE:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_813,
                     "attribute                [> <filename>]\n\
attribute [<number> ...] [> <filename>]\n\
\tPrint the current status of all known attributes in the process.\n\
\tAttributes to be listed can be specified through the <number>\n\
\tparameters, or all attributes will be listed.\n"));
      break;
#endif /*CMA_THREAD || K_THREADS*/

    default:
      (*rpt_output)(stdout,catgets(scmc_catd, MS_help, MSG_703,
                     "no help available for %s\n"),cmd);
   }
}


/*
 * NAME: help
 *
 * FUNCTION: Display help messages for dbx subcommands and topics
 *
 * PARAMETERS:
 *      n    - Node containing string users provided as argument.
 *
 * NOTES: Used when users entered the help subcommand.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void help(n)
Node n;
{
   Node name;
   int op;

   name = n->value.arg[0];

   /* if users provided no arguments,           */
   /* displays a list of subcommands and topics */
   if (name == nil) {
      help_help();
      return;
   }

   /* Try to obtain keyword token values for subcommands */
   op = findkeyword(name->value.name,0);
     
   if (op)
      /* if we have token value, probably a subcommand */
      help_command(op,ident(name->value.name));
   else 
      /* check and see if users typed in a topic name */
      help_topic(ident(name->value.name));
}
