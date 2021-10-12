$ @(#)56	1.11.1.3  src/bos/usr/ccs/bin/gprof/gprof.msg, cmdstat, bos411, 9428A410j 12/16/93 11:13:42
$
$ COMPONENT_NAME: (CMDSTAT) gprof
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


$
$ messages for gprof
$
$ gprof is used primarily by technical people for performance purposes.
$quote "
$set MS_GPROF
$ If %1$s is at the start of the message, it is replaced with gprof:
$ Do not move it from the beginning.

$ In the following comments, SNH means Should Never Happen and
$ the error message won't be printed.  If it does, then thousands
$ of error messages will be occuring, because the system as a whole
$ is totally messed up...

TICKS           "gprof 0551-034 Time is in ticks, not seconds.\n"
$ could not find the hertz value for this machine....SNH
$ Do not translate ticks

BADFORM         "%1$s: 0551-035 %2$s: Format is not correct.\n"
$ This message is no longer used. Do not translate.
$ Object profiled is not in the correct executable format

NOSTRTBL        "%1$s: 0551-036 %2$s: No string table (old format?).\n"
$ This message is no longer used. Do not translate.
$ Object profiled is not in the correct executable format

NOROOM          "%1$s: 0551-037 %2$s: No room for %3$d bytes of string table."
$ Profiled object has too many symbols, need a smaller program.

READERR_ST      "%1$s: 0551-038 %2$s: Error while reading string table.\n"
$ ....SNH

NOROOM2         "%1$s: 0551-039 No room for %2$d bytes of symbol table.\n"
$ Profiled object has too many symbols, need a smaller program.

NOROOM3         "%1$s: 0551-040 Ran out room for %2$d bytes of text space:  "
$ .....SNH

CANTDO_C        "gprof: 0551-041 The -c flag is not supported.\n"
$ option is no longer supported.

NOREADTXT       "%1$s: 0551-042 Cannot read text space:  "
$ This message is no longer used. Do not translate.
$ .....SNH

FILECONFLICT    "%1$s: 0551-043 Not compatible with first gmon file.\n"
$ Do not translate gmon
$ Trying to run gprof with multiple gmon.out files.  They all must
$ be created from the same executable

NOROOM_PC       "%1$s: 0551-044 No room for %2$d sample place counters.\n"
$ Sample range too large.  Can't malloc enought space

BADEOF          "%1$s: 0551-045 Reached unexpected end of file after reading %2$d/%3$d samples.\n"
$ corrupted gmon.out file

NOMEMORY        "gprof: 0551-046 doarcs ran out of memory for topo sorting.\n"
$ Do not translate topo, doarcs
$ to many functions called.  Cannot sort the calls

NOSORTMEM       "%s: 0551-047 There is not enough memory available for sorting.\n"
$ to many functions called.  Cannot sort the calls

BADPROPAGATE    "gprof: 0551-048 propagate: toporder error.\n"
$ Do not translate propogate, toporder
$ ...SNH

NOROOM_CH       "%1$s: 0551-049 There is not enough space for %2$d bytes of cycle headers.\n"
$ Ran out of memory...smaller program or more memory advised

NOTRELADDR      "gprof: 0551-050 reladdr: Not relative address.\n"
$ Do not translate reladdr
$ ....SNH

OVERFLOW        "gprof: 0551-051 dfn: There is a dfn_stack overflow.\n"
$ Do not translate dfn, dfn_stack
$ ....SNH

NOCYCLEHEAD     "gprof: 0551-052 dfn_findcycle: Cannot find head of cycle.\n"
$ Do not translate dfn_findcycle
$ ....SNH

DFNBUSY         "gprof: 0551-053 dfn_busy: glommed, but not to cyclehead\n"
$ ....SNH
$ glommed is a Unix technical term. Do not translate
$ Do not translate cyclehead, dfn_busy.


BADSEARCH       "gprof: 0551-054 nllookup: The binary search failed.\n"
$ Do not translate nllookup
$ Probably a bad gmon.out file.

ARCLOOKUP       "gprof: 0551-055 arclookup: parentp == 0 || childp == 0\n"
$ Do not translate.
$ ...SNH

NOMOREMEM       "gprof: 0551-056 printprof: There is not enough memory for time sorting.\n"
$ Do not translate printprof

NOROOM_PRLST    "gprof: 0551-057 There is not enough space for printlist.\n"
$ do not translate printlist

INVOPT          "gprof: 0551-070 The specified flag is not valid.\n"

OPT1            "Usage: gprof [-b][-c][-s][-z][-e Name]"

OPT2            "\t     [-E Name][-f Name][-F Name][-L PathName] [gmon.out ...] \n"

BADDATA         "gprof: 0551-058 lookup failed. The data may be corrupt.\n"
$ Probably a bad gmon.out file.
$ Do not translate lookup

BADMON          "gprof: 0551-059 The mon.out %d file doesn't match.\n"
$ Do not translate mon.out

STR2BIG         "gprof: 0551-060 The path names are too big in the loader table.\n"
$ The pathname to an archive or loadable object used in the executable
$ overran internal string space.

FILEHEAD        "gprof: 0551-061 Cannot read file header.\n"
$ ...bad object

NOACHIVE        "gprof: 0551-062 ldahread failed. Not an archive.\n"
$ ...symbol object (like libc.a) is expect to be an archive when it wasn't
$ Do not translate ldahread

SEEKER          "gprof: 0551-063 The ldtbseek function failed.\n"
$ Do not translate ldtbseek
$ ....SNH

NOSYM           "gprof: 0551-064 Cannot read symbol: %d\n"

STR2LONG        "gprof: 0551-065 The file name string is too long.\n"
$ The pathname to an archive or loadable object used in the executable
$ overran internal string space.

MEMBER          "gprof: 0551-066 The member string is too long.\n"
$ The pathname to loadable object used in the executable
$ overran internal string space.

EARLYEND        "%1$s: 0551-067 %2$s: %3$s: Reached end of file.\n"
$ The 2nd and 3rd %s's are file name and a message.

GRAN            "ngranularity: Each sample hit covers %d bytes."
$ Do not translate ngranularity

FORSECS         " Time: %.2f seconds\n\n"
$ Leave a space at the beginning of this message.
$ Either this or NOACC or NOTTIME follows GRAN on the same line,

NOACC           " No time has accumulated.\n\n"
$ Leave a space at the beginning of this message.
$ Either this or FORSECS follows GRAN on the same line,

GRAN2           "ngranularity: Each sample hit covers %d bytes."
$ Do not translate ngranularity

NOTTIME         " There is no time propagated.\n\n"
$ Leave a space at the beginning of this message.
$ Either this or FORSECS or NOACC follows GRAN on the same line,

SPON            "6.6s %5.5s %7.7s %11.11s %7.7s %7.7s     <spontaneous>\n"
$ Do not translate

CYCLE           " <cycle %d>"
$ Do not translate

WHOLECYLCE      " <cycle %d as a whole>\t[%d]\n"
$ Do not translate cycle


INDEX           "\f\nIndex by function name\n\n"
$ Do not translate \f

SCN             "gprof: 0551-068 ldnshread failed.\n"
$ Do not translate ldnshread
NOCNTS1         "Warning: mon.out file has no call counts.  Program\n"
NOCNTS2         "    possibly not compiled with profiled libraries.\n"
$ The file and libraries had no calls to mcount...

NOSUCH          "gprof: 0511-069 No such profile header.\n"


FLATHEAD1 "  %   cumulative   self              self     total           \n"
FLATHEAD2 " time   seconds   seconds    calls  ms/call  ms/call  name    \n"
$         "##### ########## ######## ######## ######## ########  ########\n"
$ The line with the "#" characters is a template.  All text in FLATHEAD1 and
$ FLATHEAD2 must be aligned with the template.
$ Note to translators:  These header entries must match up with those
$ found in the "Flat Profile Header" below.

CALLGHEAD1 "                                  called/total       parents \n"
CALLGHEAD2 "index  %time    self descendents  called+self    name    \tindex\n"
CALLGHEAD3 "                                  called/total       children\n"
$          "                                 #######/#######     ########\n"
$          "###### ##### ####### ########### #######+####### ########\t#####\n"
$          "                                 #######/#######     ########\n"
$ The lines with the "#" characters is a template.  All text in CALLGHEAD1,
$ CALLGHEAD2, and CALLGHEAD3 must fit exactly in the space indicated by the
$ "#" characters.  Do not translate, change or move the "/", "+", "\t" or "\n"
$ characters.
$ Note to translators:  These header entries must match up with those
$ found in the "Callg Profile Header" below.

$ ***********************
$ * Flat Profile Header *
$ ***********************

$set MS_FLAT

MF1   ""
MF2   "flat profile:"
MF3   ""
MF4   " %         the percentage of the total running time of the"
MF5   "time       program used by this function."
MF6   ""
MF7   "cumulative a running sum of the number of seconds accounted"
MF8   " seconds   for by this function and those listed above it."
MF9   ""
MF10  " self      the number of seconds accounted for by this"
MF11  "seconds    function alone.  This is the major sort for this"
MF12  "           listing."
MF13  ""
MF14  "calls      the number of times this function was invoked, if"
MF15  "           this function is profiled, else blank."
MF16  ""
MF17  " self      the average number of milliseconds spent in this"
MF18  "ms/call    function per call, if this function is profiled,"
MF19  "           else blank."
MF20  ""
MF21  " total     the average number of milliseconds spent in this"
MF22  "ms/call    function and its descendents per call, if this "
MF23  "           function is profiled, else blank."
MF24  ""
MF25  "name       the name of the function.  This is the minor sort"
MF26  "           for this listing. The index shows the location of"
MF27  "           the function in the gprof listing. If the index is"
MF28  "           in parenthesis it shows where it would appear in"
MF29  "           the gprof listing if it were to be printed."

$ ************************
$ * Callg Profile Header *
$ ************************

$set MS_CALLG

MC1   ""
MC2   "call graph profile:"
MC3   "          The sum of self and descendents is the major sort"
MC4   "          for this listing."
MC5   ""
MC6   "          function entries:"
MC7   ""
MC8   "index     the index of the function in the call graph"
MC9   "          listing, as an aid to locating it (see below)."
MC10  ""
MC11  "%time     the percentage of the total time of the program"
MC12  "          accounted for by this function and its"
MC13  "          descendents."
MC14  ""
MC15  "self      the number of seconds spent in this function"
MC16  "          itself."
MC17  ""
MC18  "descendents"
MC19  "          the number of seconds spent in the descendents of"
MC20  "          this function on behalf of this function."
MC21  ""
MC22  "called    the number of times this function is called (other"
MC23  "          than recursive calls)."
MC24  ""
MC25  "self      the number of times this function calls itself"
MC26  "          recursively."
MC27  ""
MC28  "name      the name of the function, with an indication of"
MC29  "          its membership in a cycle, if any."
MC30  ""
MC31  "index     the index of the function in the call graph"
MC32  "          listing, as an aid to locating it."
MC33  ""
MC34  ""
MC35  ""
MC36  "          parent listings:"
MC37  ""
MC38  "self*     the number of seconds of this function's self time"
MC39  "          which is due to calls from this parent."
MC40  ""
MC41  "descendents*"
MC42  "          the number of seconds of this function's"
MC43  "          descendent time which is due to calls from this"
MC44  "          parent."
MC45  ""
MC46  "called**  the number of times this function is called by"
MC47  "          this parent.  This is the numerator of the"
MC48  "          fraction which divides up the function's time to"
MC49  "          its parents."
MC50  ""
MC51  "total*    the number of times this function was called by"
MC52  "          all of its parents.  This is the denominator of"
MC53  "          the propagation fraction."
MC54  ""
MC55  "parents   the name of this parent, with an indication of the"
MC56  "          parent's membership in a cycle, if any."
MC57  ""
MC58  "index     the index of this parent in the call graph"
MC59  "          listing, as an aid in locating it."
MC60  ""
MC61  ""
MC62  ""
MC63  "          children listings:"
MC64  ""
MC65  "self*     the number of seconds of this child's self time"
MC66  "          which is due to being called by this function."
MC67  ""
MC68  "descendent*"
MC69  "          the number of seconds of this child's descendent's"
MC70  "          time which is due to being called by this"
MC71  "          function."
MC72  ""
MC73  "called**  the number of times this child is called by this"
MC74  "          function.  This is the numerator of the"
MC75  "          propagation fraction for this child."
MC76  ""
MC77  "total*    the number of times this child is called by all"
MC78  "          functions.  This is the denominator of the"
MC79  "          propagation fraction."
MC80  ""
MC81  "children  the name of this child, and an indication of its"
MC82  "          membership in a cycle, if any."
MC83  ""
MC84  "index     the index of this child in the call graph listing,"
MC85  "          as an aid to locating it."
MC86  ""
MC87  ""
MC88  ""
MC89  "          * these fields are omitted for parents (or"
MC90  "          children) in the same cycle as the function.  If"
MC91  "          the function (or child) is a member of a cycle,"
MC92  "          the propagated times and propagation denominator"
MC93  "          represent the self time and descendent time of the"
MC94  "          cycle as a whole."
MC95  ""
MC96  "          ** static-only parents and children are indicated"
MC97  "          by a call count of 0."
MC98  ""
MC99  ""
MC100 ""
MC101 "          cycle listings:"
MC102 "          the cycle as a whole is listed with the same"
MC103 "          fields as a function entry.  Below it are listed"
MC104 "          the members of the cycle, and their contributions"
MC105 "          to the time and call counts of the cycle."
