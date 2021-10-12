#!/bin/bsh
# @(#)81        1.1  src/bldenv/lint/lint.sh, ade_build, bos412, GOLDA411a 3/3/93 11:31:25
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS: 
#
# ORIGINS: 00 03 10 27 32
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
LLDIR=${ODE_TOOLS}/usr/lib                  # location of lint executables
LCPP=${ODE_TOOLS}/usr/lib/cpp
LINT1=$LLDIR/lint1
LINT2=$LLDIR/lint2
TMP=/usr/tmp/tlint.$$           # preprocessor scratch file
TOUT=                           # combined input for second pass
LIBA=$LLDIR/llib-lansi.ln       # standard ANSI library
LIBE=$LLDIR/llib-lc.ln          # default EXTD library

# options for the preprocessor
POPT="-E -C -U__STR__ -U__MATH__ -Dlint -DLINT"

# C++ support
CPPCOMP=${ODE_TOOLS}/usr/bin/xlC                            # C++ compiler (pass1)
CPPPOPT="-U__STR__ -U__MATH__ -Dlint -DLINT"    # options for the C++ compiler
CPPLLDIR=${ODE_TOOLS}/usr/lpp/xlC/lib                       # location of lint libraries
CPPLIB=$CPPLLDIR/llib-lC.ln                     # standard C++ library
CPPLIBA="$CPPLIB $CPPLLDIR/llib-lansi.ln"       # standard C++ ANSI C library
CPPLIBE="$CPPLIB $CPPLLDIR/llib-lc.ln"          # default C++ C library

L1OPT=                          # options for the lint passes
L2OPT=                          # options for lint pass2 only
CFILES=                         # the *.c files in order
CPPFILES=                       # the *.C files in order
LFILES=                         # the *.ln files in order
CPPLFILES=                      # the *.ln files in order (for C++)
LLIB=                           # lint library file to create
WARNS="-wA"                     # lint warning level
CONLY=                          # set for ``compile only''
MANSI=                          # set if ANSI mode
LOOK=                           # set for echo commands only
RC=0                            # Value of 2 to signal failure of lint1
CF=0                            # C files specified on input line
CPPLIBS=0                       # Use C++ lint libraries
                                # GH 09/14/90
trap "${ODE_TOOLS}/usr/bin/rm -f $TMP $TOUT; exit 2" 1 2 3 15

#
# Process each of the arguments, building lists of options.
#
# jrw  16/05/90 modified control loop from for to while to allow shift
#               usage on -o option so that -o filename and -ofilename are
#               valid syntax. Meets X/Open standard (Vol1 Version 3).
#                  PTM 40555

# GH - 09/21/90 modified while loop to explicitly check for "" in order         
# that the test function does not treat bad flag to lint as its own and give    
# error messages. P48838 
while [ "" != "$*" ] 

do
        OPT=$1  # kludge to avoid changing the usage of OPT
        case "$OPT" in
        *.c)    CFILES="$CFILES $OPT";
                TOUT=/usr/tmp/olint.$$;;        # combined input for second pass
        *.C)    CPPFILES="$CPPFILES $OPT";
                TOUT=/usr/tmp/olint.$$;;        # combined input for second pass
        *.ln)   LFILES="$LFILES $OPT";
                CPPLFILES="$CPPLFILES $OPT";;
        -*)     OPT=`echo X$OPT | ${ODE_TOOLS}/usr/bin/sed s/X-//`
                while [ "" != "$OPT" ] # GH fix
                do
                        O=`echo $OPT | ${ODE_TOOLS}/usr/bin/sed 's/\\(.\\).*/\\1/'`  # option letter
                        OPT=`echo $OPT | ${ODE_TOOLS}/usr/bin/sed s/.//`     # option argument
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
                        w)      WARNS="$WARNS$OPT"    # warning message options
                                break;;
                        b)      WARNS="${WARNS}R"    # warning message options
                                ;;
                        [hu])   WARNS="$WARNS$O"    # warning message options
                                ;;
                        x)      WARNS="${WARNS}D"    # warning message options
                                ;;
                        [NX])   L1OPT="$L1OPT -$O$OPT"  # valid cc(1) options
                                break;;
                        C)      CPPLIBS=1               # use C++ libraries
                                break;;
                        M)      L1OPT="$L1OPT -$O$OPT"  # valid cc(1) options
                                MANSI=1
                                break;;
                        l)      if [ "" != "$OPT" ]     # include a lint library
                                then
                                    LFILES="$LLDIR/llib-l$OPT.ln $LFILES"
                                    CPPLFILES="$LLDIR/llib-l$OPT.ln $CPPLFILES"
                                else
                                    $ODE_TOOLS/usr/bin/dspmsg lint.cat 1 "improper usage of option: %s\n" $O
                                fi
                                break;;
                        o)      if [ "" != "$OPT" ]             # make a lint library
                                then
                                        OPT=`basename $OPT`
                                        LLIB="llib-l$OPT.ln" # GH fix
                                else if [ "" != "$2" ] # pick up next arg as lib
                                     then
                                        OPT=`basename $2`
                                        LLIB="llib-l$OPT.ln" # GH fix
                                        shift
                                     else
                                        # situation of -o with no more args
                                        $ODE_TOOLS/usr/bin/dspmsg lint.cat 1 "improper usage of option: %s\n" $O
                                     fi
                                fi
                                break;;
                        g)
                                break;;
                        O)      
                                break;;
                        q)      if [ dbcs = "$OPT" ]
                                then
                                                POPT="$POPT -$O$OPT"
                                else
                                        $ODE_TOOLS/usr/bin/dspmsg lint.cat 1 "improper usage of option: %s\n" $O
                                fi
                                break;;
                        [IDU])  if [ "" != "$OPT" ]             # preprocessor options
                                then
                                        POPT="$POPT -$O$OPT"
                                else
                                        $ODE_TOOLS/usr/bin/dspmsg lint.cat 1 "improper usage of option: %s\n" $O
                                fi
                                break;;
                        C)      CPPLIBS=1;;
                        *)      $ODE_TOOLS/usr/bin/dspmsg lint.cat 2 "lint: bad option ignored: %s\n" $O;;
                        esac
                done;;
        *)      $ODE_TOOLS/usr/bin/dspmsg lint.cat 106 "lint: file with unknown suffix ignored: %s\n" $OPT;;
        esac
        shift
done
#
# Check for full ANSI library.
#
if [ "" != "$MANSI" ]
then
        LFILES="$LIBA $LFILES"          # standard ANSI library
        CPPLFILES="$LIBA $CPPLFILES"
else
        LFILES="$LFILES $LIBE"          # standard EXTD library
        CPPLFILES="$CPPLFILES $CPPLIBE"
fi

#
# Use C++ libraries?
#
if [ $CPPLIBS != 0 ]
then
        LFILES="$CPPLFILES"
fi

#
# Is there any need for a C++ compiler, but no compiler?
#
if [ $CPPLIBS != 0 -a ! -d $CPPLLDIR ]
then
        $ODE_TOOLS/usr/bin/dspmsg lint.cat 104 "-C specified, but no C++ lint libraries present\n"
        exit 1
fi

if [ "$CPPFILES" != "" -a ! -x $CPPCOMP ]
then
        $ODE_TOOLS/usr/bin/dspmsg lint.cat 105 ".C files specified, but xlC C++ compiler not present\n"
        exit 1
fi

#
# Run the file through lint1 (lint2).
#
if [ "" != "$CONLY" ]           # run lint1 on *.[cC]'s only producing *.ln's
then
        for i in $CFILES
        do
                T=`basename $i .c`.ln
                ${ODE_TOOLS}/usr/bin/rm -f $TMP $T
                if [ "" != "$LOOK" ]
                then    
                        echo "( $LCPP $POPT $i > $TMP"
                        echo "$LINT1 $WARNS $L1OPT $TMP -L$T ) 2>&1"
                else
                        ( $LCPP $POPT $i > $TMP
                        $LINT1 $WARNS $L1OPT $TMP -L$T ) 2>&1
                fi
        done
        for i in $CPPFILES
        do
                T=`basename $i .C`.ln
                ${ODE_TOOLS}/usr/bin/rm -f $TMP $T
                if [ "" != "$LOOK" ]
                then    
                        echo "$CPPCOMP -c -qlint:$WARNS:$T $CPPPOPT $i"
                else
                        $CPPCOMP -c -qlint:$WARNS:$T $CPPPOPT $i
                fi
        done
else                    # send all *.[cC]'s through lint1 run all through lint2
        if [ "" != "$CFILES$CPPFILES" ]
        then
                CF=1
                ${ODE_TOOLS}/usr/bin/rm -f $TOUT; ${ODE_TOOLS}/usr/bin/touch $TOUT
        fi
        for i in $CFILES
        do      
                if [ $RC -eq 2 ]
                then
                        ${ODE_TOOLS}/usr/bin/rm -f $TMP $TOUT;
                        exit $RC;
                fi
                ${ODE_TOOLS}/usr/bin/rm -f $TMP
                if [ "" != "$LOOK" ]
                then    
                        echo "( $LCPP $POPT $i > $TMP"
                        echo "$LINT1 $WARNS $L1OPT $TMP -L$TOUT ) 2>&1"
                else
                        ( $LCPP $POPT $i > $TMP
                        $LINT1 $WARNS $L1OPT $TMP -L$TOUT ) 2>&1
                        RC=$?
                fi
        done
        for i in $CPPFILES
        do      
                if [ $RC -eq 2 ]
                then
                        ${ODE_TOOLS}/usr/bin/rm -f $TMP $TOUT;
                        exit $RC;
                fi
                ${ODE_TOOLS}/usr/bin/rm -f $TMP
                if [ "" != "$LOOK" ]
                then    
                        echo "$CPPCOMP -c -qlint:$WARNS:$TOUT $CPPPOPT $i"
                else
                        $CPPCOMP -c -qlint:$WARNS:$TOUT $CPPPOPT $i
                        RC=$?
                        if [ $RC -gt 1 ]
                        then
                            RC=2
                        fi
                fi
        done

        if [ ! $RC -eq 2 ]      # 2 means lint1 failed without recovering
        then
                if [ "" != "$LOOK" ]
                then    
                        echo "$LINT2 $WARNS $L1OPT $L2OPT $TOUT $LFILES"
                else
                        $LINT2 $WARNS $L1OPT $L2OPT $TOUT $LFILES
                fi

                # GH 09/21/90 P48838, making sure mv is not called 
                # unless C files were supplied
                if [ "" != "$LLIB" ]    # make a library of lint1 results
                then
                        if [ $CF -eq 1 ] # GH fix
                        then
                                mv $TOUT $LLIB
                        fi
                fi
        else
                ${ODE_TOOLS}/usr/bin/rm -f $TMP $TOUT;
                exit $RC;
        fi
fi
${ODE_TOOLS}/usr/bin/rm -f $TMP $TOUT
