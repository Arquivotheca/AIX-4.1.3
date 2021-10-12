#!/bin/bsh
# @(#)37        1.16.1.11  src/bos/usr/ccs/bin/cflow/cflow.sh, cmdprog, bos411, 9428A410j 7/8/94 17:51:48
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS: 
#
# ORIGINS: 3 10 27 32 65 71
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 1.1
#


o=
INVFLG=
DFLAG=
IFLAG=
MFLAG=
DASHFLAG=
ERRFLAG=0
RC=0
ISNUM=
MANSI=0

DIR=/usr/lib
FILES=
CFLAGS="-E -U__STR__ -U__MATH__ -D_AIX -D_IBMR2 -D_POWER"
CC=
XLCC=/usr/bin/xlC
XLCFLAGS="-U__STR__ -U__MATH__"
USAGE="Usage: cflow [-d Number] [-i _] [-i p] [-i x] [-MA] [-r]\n\
\t[[-D Name[=Definition]] [-U Name] [-I Directory] [-qOption]] ...\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
OPTUSE="cflow: Incorrect usage of option: %s\n"
OPTREC="cflow: Option: %s is not recognized.\n"
FILEXT="cflow: File %s must have a .o, .s, .i, .c, .C, .l, or .y extension.\n"

TMP=/usr/tmp/cf.$$
TMPG=$TMP.g
trap "rm -f $TMP.?; kill $$" 1 2 3

base=`basename $0`
case $base in
        cflow | cflow.sh)       CC=/lib/cpp;;
        *cflow*)                CC=`echo $base | sed s/cflow/cpp/`;;
        *)                      CC=/lib/cpp;;
esac

if [ $# -eq 0 ]
then
        dspmsg cflow.cat 106 "$USAGE" >&2
        exit 1
fi

echo "" >$TMP.g

# parse the arguments


while [ "$1" != "" ]
do
        case "$1" in
        --)
                DASHFLAG=1
                shift
                break;;
        -d)
                ISNEGNUM=`echo $2 | grep -x -e "-[0123456789]*"`
                ISNUM=`echo $2 | grep -x -e "[0123456789]*"`
                if [ "" = "$2" ] || ( [ "$ISNUM" = "" ] && [ "$ISNEGNUM" = "" ] )
                then
                        dspmsg cflow.cat 108 "$OPTUSE" $1 >&2
                        dspmsg cflow.cat 106 "$USAGE" >&2
                        rm -f $TMP.?
                        exit 1
                fi
                if [ "$ISNUM" != "" ]
                then
                        DFLAG="-d$2"
                fi
                shift
                ;;
        -d*)
                ISNEGNUM=`echo $1 | grep -x -e "-d-[0123456789]*"`
                ISNUM=`echo $1 | grep -x -e "-d[0123456789]*"`
                if [ "$ISNUM" = "" ] && [ "$ISNEGNUM" = "" ]
                then
                        dspmsg cflow.cat 108 "$OPTUSE" "-d" >&2
                        dspmsg cflow.cat 106 "$USAGE" >&2
                        rm -f $TMP.?
                        exit 1
                fi
                if [ "$ISNUM" != "" ]
                then
                        DFLAG=$1
                fi
                ;;
        -i)
                if [ "" = "$2" ] || [ "$2" != "p" ] && [ "$2" != "_" ] && [ "$2" != "x" ]
                then
                        dspmsg cflow.cat 108 "$OPTUSE" $1 >&2
                        dspmsg cflow.cat 106 "$USAGE" >&2
                        rm -f $TMP.?
                        exit 1
                fi
                IFLAG="$IFLAG -i$2"
                shift
                ;;
        -i*)
                if [ "$1" != "-ip" ] && [ "$1" != "-i_" ] && [ "$1" != "-ix" ]
                then
                        dspmsg cflow.cat 108 "$OPTUSE" "-i" >&2
                        dspmsg cflow.cat 106 "$USAGE" >&2
                        rm -f $TMP.?
                        exit 1
                fi
                IFLAG="$IFLAG $1"
                ;;
        -MA)
                MFLAG="$MFLAG $1"
                CFLAGS="$CFLAGS -D_ANSI_C_SOURCE"
                MANSI=1
                ;;
        -f)
                cat $2 </dev/null >>$TMPG
                shift
                ;;
        -g)
                TMPG=$2
                if [ "$TMPG" = "" ]
                then
                        TMPG=$TMP.g
                fi
                shift
                ;;
        -N*)
                MFLAG="$MFLAG $1"
                ;;
        -[IDUq])
                if [ "" = "$2" ]
                then
                        dspmsg cflow.cat 108 "$OPTUSE" $1 >&2
                        dspmsg cflow.cat 106 "$USAGE" >&2
                        rm -f $TMP.?
                        exit 1
                fi
                o="$o $1$2"
                shift
                ;;
        -[IDUq]*)
                o="$o $1"
                ;;
        -[r]*)
                OPT=`echo X$1 | sed s/X-//`
                while [ "" != "$OPT" ]
                do
                        O=`echo $OPT | sed 's/\\(.\\).*/\\1/'`
                        OPT=`echo $OPT | sed s/.//`
                        case $O in
                        r)
                                INVFLG=1
                                ;;
                        *)
                               dspmsg cflow.cat 109 "$OPTREC" $O >&2
                               dspmsg cflow.cat 106 "$USAGE" >&2
                               rm -f $TMP.?
                               exit 1
                               ;;
                        esac
                done;;
        -*)
                dspmsg cflow.cat 109 "$OPTREC" $1 >&2
                dspmsg cflow.cat 106 "$USAGE" >&2
                rm -f $TMP.?
                exit 1
                ;;
        *.[ylcisoC])
                FILES="$FILES $1"               # Keep a list of files to process
                ;;
        *)
                dspmsg cflow.cat 104 "$FILEXT" $1 >&2
                ERRFLAG=1
                ;;
        esac
        shift
done

if [ $DASHFLAG ]
then
        while [ "$1" != "" ]
        do
                case "$1" in
                *.[ylcisoC])
                        FILES="$FILES $1"               # Keep a list of files to process
                        ;;
                *)
                        dspmsg cflow.cat 104 "$FILEXT" $1 >&2
                        ERRFLAG=1
                        ;;
                esac
                shift
        done
fi

if [ "" = "$FILES" ]
then
      dspmsg cflow.cat 107 "cflow: There are no files to process.\n" >&2
      rm -f $TMP.?
      exit 1
fi

if [ MANSI -eq 0 ]
then
      CFLAGS="$CFLAGS -D_LONG_LONG"
fi

# Now process the specified files
for file in $FILES; do
        if [ $DASHFLAG ]
        then
                filed="-- $file"
        else
                filed=$file
        fi
        if [ ! -r $file ]
        then
                dspmsg cflow.cat 105 "Cannot open file %s.\n" $file >&2
                ERRFLAG=1
                continue
        fi
        FILENAME=`basename $file`
        case $file in
                *.y)
                        yacc $filed || { ERRFLAG=1 ; continue ; }
                        sed -e "/^# line/d" y.tab.c > $file.c
                        $CC $CFLAGS $o $file.c > $TMP.i
                        $DIR/cflow1 -f$FILENAME $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                 rm $TMP.? y.tab.c $file.c
                                 exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        RC=$?
                        rm y.tab.c $file.c
                        ;;
                *.l)
                        lex $filed || { ERRFLAG=1 ; continue ; }
                        sed -e "/^# line/d" lex.yy.c > $file.c
                        $CC $CFLAGS $o $file.c > $TMP.i
                        $DIR/cflow1 -f$FILENAME $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                rm $TMP.? lex.yy.c $file.c
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        RC=$?
                        rm lex.yy.c $file.c
                        ;;
                *.c)
                        $CC $CFLAGS $o $filed > $TMP.i
                        $DIR/cflow1 -f$FILENAME $MFLAG -L$TMP.j $TMP.i 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then
                                rm $TMP.?
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        RC=$?
                        ;;
                *.C)
                        if [ ! -x $XLCC ]
                        then
                                dspmsg cflow.cat 103 "%s not present -- file %s skipped\n" $XLCC $file >&2
                                ERRFLAG=1
                                continue;
                        fi
                        $XLCC $XLCFLAGS $o -c -qcflow:$TMP.j $MFLAG $filed
                        if [ $? -ge 3 ]         # Unrecoverable failure in xlC
                        then
                                rm $TMP.?
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        RC=$?
                        ;;
                *.i)
                        $DIR/cflow1 -f$FILENAME $MFLAG -L$TMP.j $filed 
                        if [ $? -eq 2 ]         # Unrecoverable failure in cflow1
                        then 
                                rm $TMP.?
                                exit 1;
                        fi
                        $DIR/lpfx $IFLAG $TMP.j >> $TMPG
                        RC=$?
                        ;;
                *.s)
                        DASH=`echo $file | grep -e "^-"`
                        if [ "$DASH" != "" ]
                        then
                             filed="./${file}"
                        fi
                        a=`basename $file .s`
                        as -o $TMP.o $filed || ERRFLAG=1
                        nm -vhe $TMP.o | $DIR/nmf $a ${a}.s >>$TMPG || ERRFLAG=1
                        ;;
                *.o)
                        a=`basename $file .o`
                        nm -vhe $filed | $DIR/nmf $a ${a}.o >>$TMPG || ERRFLAG=1
                        ;;
        
        esac
        if [ $RC -gt 0 ]  # lpfx failed
        then
                rm -f $TMP.?
                exit 1
        fi
done

if [ "$INVFLG" != "" ]
then
        grep "=" $TMPG >$TMP.q
        grep ":" $TMPG | $DIR/flip >>$TMP.q
        sort <$TMP.q >$TMPG || ERRFLAG=1
        rm $TMP.q
fi

$DIR/dag $DFLAG < $TMPG || ERRFLAG=1
rm -f $TMP.?

exit $ERRFLAG
