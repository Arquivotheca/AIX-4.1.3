#!/bin/bsh
# @(#)12        1.22.2.10  src/bos/usr/ccs/bin/lint/lint.sh, cmdprog, bos411, 9428A410j 7/8/94 17:52:06
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS: 
#
# ORIGINS: 3 10 27 32
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Lint shell script.  Interacts with options from cc(1).
#
LLDIR=/usr/lib                  # location of lint executables
LCPP=/lib/cpp
LINT1=$LLDIR/lint1
LINT2=$LLDIR/lint2
TMP=/usr/tmp/tlint.$$           # preprocessor scratch file
TOUT=/usr/tmp/olint.$$          # combined input for second pass
LIBA=$LLDIR/llib-lansi.ln       # standard ANSI library
LIBE=$LLDIR/llib-lc.ln          # default EXTD library
ERRFLAG=0
DASHFLAG=0

# options for the preprocessor
POPT="-E -C -U__STR__ -U__MATH__ -Dlint -DLINT -D_AIX -D_IBMR2 -D_POWER"

# C++ support
CPPCOMP=/usr/bin/xlC                            # C++ compiler (pass1)
CPPPOPT="-U__STR__ -U__MATH__ -Dlint -DLINT"    # options for the C++ compiler
CPPLLDIR=/usr/lpp/xlC/lib                       # location of lint libraries
CPPLIB=$CPPLLDIR/llib-lC.ln                     # standard C++ library
CPPLIBA="$CPPLIB $CPPLLDIR/llib-lansi.ln"       # standard C++ ANSI C library
CPPLIBE="$CPPLIB $CPPLLDIR/llib-lc.ln"          # default C++ C library

L1OPT=                          # options for the lint passes
L2OPT=                          # options for lint pass2 only
CFILES=                         # the *.c files in order
CPPFILES=                       # the *.C files in order
LFILES=                         # the *.ln files in order
CPPLFILES=                      # the *.ln files in order (for C++)
LIBFILES=                       # the library files in order
CPPLIBFILES=                    # the library files in order (for C++)
LLIB=                           # lint library file to create
WARNS="-wA"                     # lint warning level
CONLY=                          # set for ``compile only''
MANSI=                          # set if ANSI mode
LOOK=                           # set for echo commands only
RC=0                            # Value of 2 to signal failure of lint1
CF=0                            # C files specified on input line
CPPLIBS=0                       # Use C++ lint libraries
ISWARN=0

trap "rm -f $TMP $TOUT; exit 2" 1 2 3 15

#
# Process each of the arguments, building lists of options.
#

while [ "" != "$*" ] && [ $DASHFLAG -ne 1 ]

do
        OPT=$1
        case "$OPT" in
        *.c)    CFILES="$CFILES $OPT";;
        *.C)    CPPFILES="$CPPFILES $OPT";;
        *.ln)   LFILES="$LFILES $OPT"
                CPPLFILES="$CPPLFILES $OPT";;
        --)     DASHFLAG=1;;
        -*)     OPT=`echo X$OPT | sed s/X-//`
                while [ "" != "$OPT" ]
                do
                        O=`echo $OPT | sed 's/\\(.\\).*/\\1/'`  # option letter
                        OPT=`echo $OPT | sed s/.//`     # option argument
                        case $O in
                        \#)     LOOK=1;;                # echo commands only
                        c)      CONLY=1;;       # lint1 only, make .ln files
                        p)      L2OPT="$L2OPT -$O";
                                # extreme portability
                                LIBE="$LLDIR/llib-port.ln";
                                CPPLIBE="$CPPLIB $CPPLLDIR/llib-port.ln";;
                        n)      LIBA=                   # no libraries
                                LIBE=
                                CPPLIBA=
                                CPPLIBE=;;
                        v)      L1OPT="$L1OPT -v"       # parameter usage check
                                ;;
                        a)      WARNS="${WARNS}l"     # warning message options
                                ;;
                        w)      ISWARN=`echo $OPT | grep -e "[^acdhklnoprsuACDOPR]"`
                                if [ "$OPT" = "" ] || [ "$ISWARN" != "" ]
                                then
                                        dspmsg lint.cat 110 "Incorrect usage of option: %s\n" $O >&2
                                        exit 1
                                fi 
                                WARNS="$WARNS$OPT"    # warning message options
                                break;;
                        b)      WARNS="${WARNS}R"    # warning message options
                                ;;
                        [hu])   WARNS="$WARNS$O"    # warning message options
                                ;;
                        x)      WARNS="${WARNS}D"    # warning message options
                                ;;
                        [N])   L1OPT="$L1OPT -$O$OPT"  # valid cc(1) options
                                break;;
                        C)      CPPLIBS=1               # use C++ libraries
                                break;;
                        M)      if [ "$OPT" != "A" ]
                                then
                                        dspmsg lint.cat 107 "lint: %s option is not recognized\n" "$O$OPT" >&2
                                        exit 1
                                fi
                                L1OPT="$L1OPT -$O$OPT"  # valid cc(1) options
                                POPT="$POPT -qlanglvl=ansi -D_ANSI_C_SOURCE"  #invoke cpp in ansi mode
                                MANSI=1
                                break;;
                        l)      if [ "" != "$OPT" ]     # include a lint library
                                then
                                    LIBFILES="$LLDIR/llib-l$OPT.ln $LIBFILES"
                                    CPPLIBFILES="$CPPLLDIR/llib-l$OPT.ln $CPPLIBFILES"
                                else
                                    dspmsg lint.cat 110 "Incorrect usage of option: %s\n" $O >&2
                                    exit 1
                                fi
                                break;;
                        o)      if [ "" != "$OPT" ]             # make a lint library
                                then
                                        OPT=`basename $OPT`
                                        LLIB="llib-l$OPT.ln"
                                else if [ "" != "$2" ] # pick up next arg as lib
                                     then
                                        OPT=`basename $2`
                                        LLIB="llib-l$OPT.ln"
                                        shift
                                     else
                                        # situation of -o with no more args
                                        dspmsg lint.cat 110 "Incorrect usage of option: %s\n" $O >&2
                                        exit 1
                                     fi
                                fi
                                break;;
                        g)
                                break;;
                        O)      
                                break;;
                        [IDUq]) if [ "" != "$OPT" ]             # preprocessor options
                                then
                                        POPT="$POPT -$O$OPT"
                                        CPPPOPT="$CPPPOPT -$O$OPT"
                                else if [ "" != "$2" ]
                                     then
                                        POPT="$POPT -$O$2"
                                        CPPPOPT="$CPPPOPT -$O$2"
                                        shift
                                     else
                                        dspmsg lint.cat 110 "Incorrect usage of option: %s\n" $O >&2
                                        exit 1
                                     fi
                                fi
                                break;;
                        C)      CPPLIBS=1;;
                        *)      dspmsg lint.cat 107 "lint: %s option is not recognized\n" $O >&2
                                exit 1;;
                        esac
                done;;
        *)      dspmsg lint.cat 106 "lint: File %s must have a .c, .C or .ln extension. It is ignored.\n" $OPT >&2
                ERRFLAG=1;;
        esac
        shift
done

if [ $DASHFLAG -eq 1 ]
then
        while [ "" != "$*" ]
        do
                OPT=$1
                case "$OPT" in
                *.c)    CFILES="$CFILES $OPT";;
                *.C)    CPPFILES="$CPPFILES $OPT";;
                *.ln)   LFILES="$LFILES $OPT"
                        CPPLFILES="$CPPLFILES $OPT";;
                *)      dspmsg lint.cat 106 "lint: File %s must have a .c, .C or .ln extension. It is ignored.\n" $OPT >&2
                        ERRFLAG=1;;
                esac
                shift
        done
fi
        
if [ "" = "$CFILES$CPPFILES$LFILES" ]
then
        dspmsg lint.cat 108 "lint: there are no files to process.\n" >&2
        exit 1
fi

#
# Check for full ANSI library.
#
if [ "" != "$MANSI" ]
then
        LIBFILES="$LIBA $LIBFILES"          # standard ANSI library
        CPPLIBFILES="$CPPLIBA $CPPLIBFILES"
else
        LIBFILES="$LIBFILES $LIBE"          # standard EXTD library
        CPPLIBFILES="$CPPLIBFILES $CPPLIBE"
        POPT="$POPT -D_LONG_LONG"
fi

#
# Use C++ libraries?
#
if [ $CPPLIBS != 0 ]
then
        LFILES="$CPPLFILES"
        LIBFILES="$CPPLIBFILES"
fi


#
# Is there any need for a C++ compiler, but no compiler?
#
if [ $CPPLIBS != 0 -a ! -d $CPPLLDIR ]
then
        dspmsg lint.cat 104 "-C specified, but no C++ lint libraries present\n" >&2
        exit 1
fi

if [ "$CPPFILES" != "" -a ! -x $CPPCOMP ]
then
        dspmsg lint.cat 105 ".C files specified, but xlC C++ compiler not present\n" >&2
        exit 1
fi

#
# Run the file through lint1 (lint2).
#
if [ "" != "$CONLY" ]           # run lint1 on *.[cC]'s only producing *.ln's
then
        for i in $CFILES
        do
                FILENAME=`basename $i`
                T=`basename $i .c`.ln
                rm -f $TMP $T
                if [ "" != "$LOOK" ]
                then    
                        echo "( $LCPP $POPT -- $i 2>&1 1>$TMP"
                        echo "$LINT1 $WARNS -f$FILENAME $L1OPT $TMP -L$T )"
                else
                        if [ ! -r $i ]
                        then
                                dspmsg lint.cat 109 "Cannot open file %s.\n" $i >&2
                                ERRFLAG=1
                                continue
                        fi
                        ( $LCPP $POPT -- $i 2>&1 1>$TMP
                        $LINT1 $WARNS -f$FILENAME $L1OPT $TMP -L$T )
                        RC=$?
                        if [ $RC -eq 2 ]
                        then
                                exit 2
                        fi
                fi
        done
        for i in $CPPFILES
        do
                T=`basename $i .C`.ln
                rm -f $TMP $T
                if [ "" != "$LOOK" ]
                then    
                        echo "$CPPCOMP -c -qlint:$WARNS:$T $CPPPOPT -- $i"
                else
                        if [ ! -r $i ]
                        then
                                dspmsg lint.cat 109 "Cannot open file %s.\n" $i >&2
                                ERRFLAG=1
                                continue
                        fi
                        $CPPCOMP -c -qlint:$WARNS:$T $CPPPOPT -- $i
                        RC=$?
                        if [ $RC -gt 1 ]
                        then
                                exit 2
                        fi
                fi
        done
else                    # send all *.[cC]'s through lint1 run all through lint2
        rm -f $TOUT; touch $TOUT
        for i in $CFILES
        do      
                FILENAME=`basename $i`
                rm -f $TMP
                if [ "" != "$LOOK" ]
                then    
                        echo "( $LCPP $POPT -- $i 2>&1 1>$TMP"
                        echo "$LINT1 $WARNS -f$FILENAME $L1OPT $TMP -L$TOUT )"
                else
                        if [ ! -r $i ]
                        then
                                dspmsg lint.cat 109 "Cannot open file %s.\n" $i >&2
                                ERRFLAG=1
                                continue
                        fi
                        ( $LCPP $POPT -- $i 2>&1 1>$TMP
                        $LINT1 $WARNS -f$FILENAME $L1OPT $TMP -L$TOUT )
                        RC=$?
                        if [ $RC -eq 2 ]
                        then
                                rm -f $TMP $TOUT
                                exit $RC;
                        fi
                fi
        done
        for i in $CPPFILES
        do      
                rm -f $TMP
                if [ "" != "$LOOK" ]
                then    
                        echo "$CPPCOMP -c -qlint:$WARNS:$TOUT $CPPPOPT -- $i"
                else
                        if [ ! -r $i ]
                        then
                                dspmsg lint.cat 109 "Cannot open file %s.\n" $i >&2
                                ERRFLAG=1
                                continue
                        fi
                        $CPPCOMP -c -qlint:$WARNS:$TOUT $CPPPOPT -- $i
                        RC=$?
                        if [ $RC -gt 1 ]
                        then
                                rm -f $TMP $TOUT
                                exit 2
                        fi
                fi
        done

        if [ "" != "$LOOK" ]
        then    
                echo "$LINT2 $WARNS $L1OPT $L2OPT -- $LFILES $TOUT $LIBFILES"
        else
                if [ ! -s $TOUT ]
                then
                        rm -f $TOUT
                        TOUT=""
                fi
                if [ "" != "$TOUT" ] || [ "" != "$LFILES" ]
                then
                        $LINT2 $WARNS $L1OPT $L2OPT -- $LFILES $TOUT $LIBFILES
                        if [ $? -eq 2 ]
                        then
                                rm -f $TMP $TOUT
                                exit 2
                        fi
                        
                        if [ "" != "$LLIB" ]    # make a library of lint1 results
                        then
                                if [ "" != "$TOUT" ]
                                then
                                        mv $TOUT $LLIB
                                fi
                        fi
                fi
        fi
fi
rm -f $TMP $TOUT
exit $ERRFLAG
