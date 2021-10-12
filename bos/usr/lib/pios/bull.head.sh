# @(#)34 1.1 src/bos/usr/lib/pios/bull.head.sh, cmdpios, bos411, 9428A410j 4/28/94 07:53:23
#
# COMPONENT_NAME: (CMDPIOS) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
COLUMNS=$1
ARG1=$PIOJOBNUM
ARG2=$PIOTO
ARG3=$PIOTITLE
CMD0=$PIOQNAME

echo "\n\n\n\n\n"
if [ -x /bin/banner ]; then
    banner=/bin/banner
else
    banner=/bin/echo
fi
if [ "$COLUMNS" -gt 85 ]; then
    $banner "$ARG2" "$ARG3"
else
    $banner "$ARG2" 
fi
echo "\nRequest id: $ARG1    Printer: `basename $CMD0`\n\n`date`\n\f\c"
exit 0
