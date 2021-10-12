#!/bin/ksh
# @(#)46	1.3  src/bos/usr/lib/pios/pioqms100.sh, cmdpios, bos411, 9428A410j 10/11/90 07:35:24

# COMPONENT_NAME: CMDPIOS
#
# FUNCTIONS: error_exit
# 
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 

#
#  NAME: pioqms100 - Initializes the QMS ColorScript 100 color printer.
#
#  FUNCTION:
#  - Loads HPGL emulator files from diskette (-Q flag)
#  - Maintains a file in /usr/lpd/pio/flags that determines the
#    current state of the printer (see below).
#
#  NOTES: 
#   This script should only be invoked:
#   - From the command line (-Q flag) to load HPGL emulator files from diskette
#   - By piobe as a part of the pipeline defined for the QMS printer. In this
#     case, all these requisite files must exist or the script will terminate
#     with a return code indicating failure:
#
#       HPGLCL.EMU          (the gl emulator)
#       GO_HPGL.[SER|PAR]   (part a of the emulator start up sequence)
#       HPGL.CMD            (part b of the emulator start up sequence)
#       END_HPGL.[SER|PAR]  (terminates the emulator)
#
#   Invocation:
#       pioqms100 -Q  (to install HPGL emulator files from diskette)
#   OR
#       pioqms100 -s stream -i mode -f filename -d delay  pioout_flags
#
#   where:
#       stream = s (PostScript) or g (HPGL)
#       mode = 0 (ordinary file), 1 (parallel), 2 (serial), or 3 (tty)
#       filename is the name of the flags file to update
#       delay is the delay (seconds) between jobs of differing data stream type
#       pioout_flags is a list of flags to be passed to the pioout command
#
#  RETURNS:
#   A value of zero is returned on successful completion of the script.
#   99 is returned in the event of any type of failure.
#

error_exit()
{
MSG_NUM=`echo $1 | cut -f1 -d' '`
DFLT_MSG=$DEFAULT_MSG\\n`echo $1 | cut -f2- -d' '`
$PIOBASEDIR/etc/piomsg "`dspmsg piobe.cat -s 4 $MSG_NUM \"$DFLT_MSG\" 2>&1`"
exit 99
}

STREAM=
MODE=
FILE=
PIOOUT=
DELAY=
PIO_opts=
REAL_DEVICE=true
INSTALL=false
if [ -z "$PIOBASEDIR" ]
    then PIOBASEDIR=/usr/lpd/pio
fi
DIR=$PIOBASEDIR/etc
DEFAULT_MSG="Message catalog not found"

BAD_DATASTREAM="1 Data stream not properly selected\\\n"
BAD_MODE="2 Communication mode not properly selected\\\n"
BAD_CALL="4 Improper command invocation\\\n\\\n\
\\\tUsage:  pioqms100 -s stream -i mode -f filename -d delay pioout\\\n\
\\\tor      pioqms100 -Q  (used only to install HPGL emulator files)\\\n\\\n"
FILES_MISSING="5 Not all HPGL emulation control files are present.\\\n"
EMUDNLD_FAIL="6 Downloading the HPGL emulator failed.\\\n"
GL_ACTIVATE_A="7 HPGL emulator activation Part A failed\\\n"
GL_ACTIVATE_B="8 HPGL emulator activation Part B failed\\\n"
GL_TERM="9 HPGL termination failed\\\n"
NOT_ROOT="10 Must have root permissions to perform installation.\\\n"

K_MSG="state may be changed"
L_MSG="PrinterError"

###############################################################################
#        check for valid flags
###############################################################################

while getopts :s:i:f:d:QC:D:F:I:N:R: FLAG
    do  case $FLAG in
            s)  case $OPTARG in
                    s) STREAM=post_script;;
                    g) STREAM=hpgl;;
                    *) error_exit $BAD_DATASTREAM;;
                esac ;;
            i)  case $OPTARG in
                    0) MODE=PAR; REAL_DEVICE=false;;
                    1) MODE=PAR;;
                    2|3) MODE=SER;;
                    *) error_exit $BAD_MODE
                esac ;;
            f)  FILE=$OPTARG;;
            d)  DELAY=$OPTARG;;
            Q)  INSTALL=true;;

            C|D|F|I|N|R)    PIO_opts="$PIO_opts -$FLAG $OPTARG";;
            \?) ;;
        esac
    done

if [ $INSTALL = false ]
    then    PIOOUT="$PIOBASEDIR/etc/pioout $PIO_opts"
fi

###############################################################################
#        options can be -Q (to install files)
#              OR       -s [stream] -i [interface] -f [filename] -d [delay]
#        but not a combination of the two
#
#        if a combination of the flags is found to have been
#        on the command line then error exit and return
###############################################################################

if [ $INSTALL = false ]
    then if [ -z "$STREAM" ] || [ -z "$MODE" ] || [ -z "$FILE" ] || \
            [ -z "$PIOOUT" ] || [ -z "$DELAY" ] || [ $DELAY -lt 0 ]
            then    error_exit $BAD_CALL
        fi
    else if [ -n "$STREAM" ] || [ -n "$MODE" ] || [ -n "$FILE" ] || \
            [ -n "$PIOOUT" ] || [ -n "$DELAY" ]
            then    error_exit $BAD_CALL
        fi
fi

###############################################################################
#        copy all necessary files from DOS 3.5" diskette
###############################################################################

if [ $INSTALL = true ]
    then    if [ -z "`id|fgrep uid=0\(root\)`" ]
                then  MSG_NUM=`echo $NOT_ROOT | cut -f1 -d' '`
                      DFLT_MSG=$DEFAULT_MSG\\n`echo $NOT_ROOT | cut -f2- -d' '`
                        dspmsg piobe.cat -s 4 $MSG_NUM "$DFLT_MSG" 2>&1
                        exit 99
            fi
            
            dosread -a HPGLCL.EMU $DIR/HPGLCL.EMU
            dosread -a GO_HPGL.SER $DIR/GO_HPGL.SER
            dosread -a GO_HPGL.PAR $DIR/GO_HPGL.PAR
            dosread -a HPGL.CMD $DIR/HPGL.CMD
            dosread -a END_HPGL.SER $DIR/END_HPGL.SER
            dosread -a END_HPGL.PAR $DIR/END_HPGL.PAR
            exit
fi

###############################################################################
#        make sure all necessary files exist
#           (only if data stream is hpgl)
###############################################################################

if [ $STREAM = hpgl ]
    then    if  [ -f $DIR/HPGLCL.EMU ] && \
                [ -f $DIR/GO_HPGL.$MODE ] && \
                [ -f $DIR/HPGL.CMD ] && \
                [ -f $DIR/END_HPGL.$MODE ]
                    then :
                    else error_exit $FILES_MISSING
            fi
fi

###############################################################################
#        what is the current state of the printer
#
#        if a comparison of the current state and the desired state
#        indicates that the state will be changed, then wait an
#        arbitrary length of time from the printing of the last job
#        to be completed
#
#        if $FILE does not exist, the current state is assumed
#        to be EMULATOR=not_loaded and LAST_TIME=0
#
#        if $FILE exists, the first line contains the state of the emulator
#        the second line contains the time when the last pipeline finished
###############################################################################

# set -x     # turn on tracing

set `date +"%j %H %M %S"`
CURRENT_TIME=`expr $1 \* 86400 + $2 \* 3600 + $3 \* 60 + $4`

if [ -f $FILE ] && [ `wc -l $FILE|awk '{print $1}'` = 2 ]
    then    exec 3<$FILE
            read -u3 EMULATOR
            read -u3 DAY HOUR MIN SEC
            exec 3<&-
            LAST_TIME=`expr $DAY \* 86400 + $HOUR \* 3600 + $MIN \* 60 + $SEC`

    else    EMULATOR=not_loaded
            LAST_TIME=0
fi

REMAINDER=0
if [ $STREAM = post_script ] && [ $EMULATOR = enabled ]
    then    REMAINDER=`expr $LAST_TIME + $DELAY - $CURRENT_TIME`
fi

if [ $STREAM = hpgl ] && [ $EMULATOR != enabled ]
    then    REMAINDER=`expr $LAST_TIME + $DELAY - $CURRENT_TIME`
fi

if [ $REMAINDER -gt 0 ] && [ "$REAL_DEVICE" = "true" ]
    then sleep `expr $REMAINDER + 10`
fi

###############################################################################
#       download, enable, or disable the HPGL emulator as needed
###############################################################################

if [ $STREAM = post_script ]
    then    if [ $EMULATOR = enabled ]
                then    cat $DIR/END_HPGL.$MODE \
                        | $PIOOUT -E LPST_NOSLCT -K "$K_MSG" -L "$L_MSG"
                        sleep 5
                        EC=$?
                        if [ $EC = 0 ] || [ $EC = 102 ]
                            then    sleep 10
                            else    error_exit $GL_TERM
                        fi
                        EMULATOR=loaded
            fi

    else    if [ $EMULATOR = not_loaded ]
                then    cat $DIR/HPGLCL.EMU \
                        | $PIOOUT -E LPST_NOSLCT -K "$K_MSG" -L "$L_MSG"
                        EC=$?
                        if [ $EC != 0 ]
                            then    error_exit $EMUDNLD_FAIL
                        fi
                        EMULATOR=loaded
            fi
            if [ $EMULATOR = loaded ]
                then    cat $DIR/GO_HPGL.$MODE \
                        | $PIOOUT -E LPST_NOSLCT -K "$K_MSG" -L "$L_MSG"
                        EC=$?
                        if [ $EC = 0 ]
                            then    sleep 5
                                    cat $DIR/HPGL.CMD \
                                    | $PIOOUT -E LPST_NOSLCT \
                                    -K "$K_MSG" -L "$L_MSG"
                                    EC=$?
                                    if [ $EC != 0 ]
                                        then    error_exit $GL_ACTIVATE_B
                                    fi
                            else    error_exit $GL_ACTIVATE_A
                        fi
                        EMULATOR=enabled
            fi
fi

echo $EMULATOR >$FILE
exit 0
